#!/usr/bin/env python3
"""
TM4C ComStack UART Bridge Node for ROS2
========================================
This is the BRIDGE between TM4C hardware UART and ROS2 topics.

Data Flow:
    TM4C --[UART binary packets]--> This Bridge --[ROS topics]--> Odometry Node / SLAM
    TM4C <--[UART binary packets]-- This Bridge <--[/cmd_vel]---- Navigation / Teleop

Publishes:
    /encoder_ticks   (Int32MultiArray)  - [left_ticks, right_ticks]
    /imu/data_raw    (sensor_msgs/Imu)  - accelerometer + gyroscope

Subscribes:
    /cmd_vel         (geometry_msgs/Twist) - velocity commands → motor control

Usage:
    1. Connect TM4C UART1 to RPi UART (GPIO14/15 or via USB)
    2. ros2 run your_package tm4c_bridge_node
    3. Verify: ros2 topic echo /encoder_ticks

Protocol (ComStack Binary):
    | 0xAA | CMD(1) | LEN(1) | DATA(0-120) | XOR_CHECKSUM(1) | 0x55 |
"""

import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32MultiArray
from sensor_msgs.msg import Imu
from geometry_msgs.msg import Twist
import serial
import struct
import threading
import time
import math


# =================== ComStack Protocol Constants ===================
START_BYTE       = 0xAA
END_BYTE         = 0x55

# Command IDs (must match ComStack_Types.h on TM4C)
CMD_PING         = 0x01
CMD_ACK          = 0x02
CMD_NACK         = 0x03
CMD_MOTOR_CMD    = 0x10
CMD_MOTOR_STOP   = 0x11
CMD_IMU_DATA     = 0x22
CMD_ENCODER_DATA = 0x23

# Robot parameters (must match Robot_Control.h on TM4C)
ROBOT_MAX_LINEAR_VEL  = 1.0    # m/s
ROBOT_WHEEL_BASE      = 0.30   # meters


class TM4CBridgeNode(Node):

    def __init__(self):
        super().__init__('tm4c_bridge')

        # --- Parameters ---
        self.declare_parameter('serial_port', '/dev/ttyAMA0')  # RPi UART0
        self.declare_parameter('baud_rate', 115200)

        port = self.get_parameter('serial_port').value
        baud = self.get_parameter('baud_rate').value

        # --- Serial Port ---
        try:
            self.ser = serial.Serial(
                port=port,
                baudrate=baud,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.01
            )
            self.get_logger().info(f'Serial opened: {port} @ {baud}')
        except serial.SerialException as e:
            self.get_logger().error(f'Cannot open serial: {e}')
            raise

        # --- Publishers ---
        self.encoder_pub = self.create_publisher(Int32MultiArray, '/encoder_ticks', 10)
        self.imu_pub = self.create_publisher(Imu, '/imu/data_raw', 10)

        # --- Subscribers ---
        self.cmd_vel_sub = self.create_subscription(
            Twist, '/cmd_vel', self.cmd_vel_callback, 10
        )

        # --- Stats ---
        self.rx_count = 0
        self.tx_count = 0
        self.error_count = 0

        # --- Start UART receive thread ---
        self.running = True
        self.rx_thread = threading.Thread(target=self.receive_loop, daemon=True)
        self.rx_thread.start()

        # --- Heartbeat timer (send ping every 2 seconds) ---
        self.create_timer(2.0, self.send_ping)

        # --- Stats timer ---
        self.create_timer(5.0, self.print_stats)

        self.get_logger().info('TM4C Bridge Node started')

    # =================== TRANSMIT (ROS → TM4C) ===================

    def calculate_checksum(self, command, length, data):
        """XOR checksum: CMD ^ LEN ^ DATA[0] ^ DATA[1] ^ ..."""
        checksum = command ^ length
        for byte in data:
            checksum ^= byte
        return checksum & 0xFF

    def send_packet(self, command, data=b''):
        """Send a ComStack binary packet to TM4C via UART"""
        length = len(data)
        checksum = self.calculate_checksum(command, length, data)
        packet = bytes([START_BYTE, command, length]) + data + bytes([checksum, END_BYTE])
        try:
            self.ser.write(packet)
            self.tx_count += 1
        except serial.SerialException as e:
            self.get_logger().error(f'TX error: {e}')

    def cmd_vel_callback(self, msg):
        """
        Convert ROS Twist (/cmd_vel) → ComStack motor command.
        
        Twist.linear.x  = forward velocity (m/s)
        Twist.angular.z  = rotation velocity (rad/s)
        
        Converts to left/right wheel speeds (-100 to +100) using
        differential drive kinematics, then sends as CMD 0x10.
        """
        # Differential drive: convert (v, w) → wheel velocities
        half_base = ROBOT_WHEEL_BASE / 2.0
        left_vel = msg.linear.x - (msg.angular.z * half_base)
        right_vel = msg.linear.x + (msg.angular.z * half_base)

        # Scale to -100..+100 percentage
        left_pct = int((left_vel / ROBOT_MAX_LINEAR_VEL) * 100.0)
        right_pct = int((right_vel / ROBOT_MAX_LINEAR_VEL) * 100.0)

        # Clamp
        left_pct = max(-100, min(100, left_pct))
        right_pct = max(-100, min(100, right_pct))

        # Pack as two sint16 (little-endian)
        data = struct.pack('<hh', left_pct, right_pct)
        self.send_packet(CMD_MOTOR_CMD, data)

    def send_ping(self):
        """Send heartbeat ping to TM4C"""
        self.send_packet(CMD_PING)

    # =================== RECEIVE (TM4C → ROS) ===================

    def receive_loop(self):
        """Background thread: read UART bytes and parse ComStack packets"""
        while self.running:
            try:
                # Look for start byte
                byte = self.ser.read(1)
                if len(byte) == 0:
                    continue
                if byte[0] != START_BYTE:
                    continue

                # Read command + length
                header = self.ser.read(2)
                if len(header) < 2:
                    self.error_count += 1
                    continue

                command = header[0]
                length = header[1]

                # Sanity check length
                if length > 120:
                    self.error_count += 1
                    continue

                # Read data payload
                data = self.ser.read(length) if length > 0 else b''
                if len(data) < length:
                    self.error_count += 1
                    continue

                # Read checksum + end byte
                footer = self.ser.read(2)
                if len(footer) < 2:
                    self.error_count += 1
                    continue

                checksum = footer[0]
                end_byte = footer[1]

                # Validate end byte
                if end_byte != END_BYTE:
                    self.error_count += 1
                    continue

                # Validate checksum
                calc = self.calculate_checksum(command, length, data)
                if checksum != calc:
                    self.error_count += 1
                    continue

                # Valid packet!
                self.rx_count += 1
                self.handle_packet(command, data)

            except serial.SerialException:
                time.sleep(0.1)
            except Exception as e:
                self.get_logger().error(f'RX error: {e}')
                time.sleep(0.01)

    def handle_packet(self, command, data):
        """Route parsed packet to the correct ROS publisher"""
        if command == CMD_ENCODER_DATA and len(data) >= 12:
            self.publish_encoder(data)
        elif command == CMD_IMU_DATA and len(data) >= 12:
            self.publish_imu(data)
        elif command == CMD_ACK:
            pass  # Heartbeat response, ignore
        elif command == CMD_PING:
            self.send_packet(CMD_ACK)  # Reply to TM4C ping

    def publish_encoder(self, data):
        """
        Parse encoder packet and publish to /encoder_ticks
        
        Data format (12 bytes, little-endian, fixed-point):
            [0-3]   Left encoder ticks  (sint32)
            [4-7]   Right encoder ticks (sint32)
            [8-9]   Left velocity RPM   (sint16, RPM × 100)
            [10-11] Right velocity RPM  (sint16, RPM × 100)
        """
        left_ticks, right_ticks, left_vel_x100, right_vel_x100 = struct.unpack('<iihh', data[:12])

        msg = Int32MultiArray()
        msg.data = [left_ticks, right_ticks]
        self.encoder_pub.publish(msg)

    def publish_imu(self, data):
        """
        Parse IMU packet and publish to /imu/data_raw
        
        Data format (12 bytes, little-endian, fixed-point × 100):
            [0-1]   Accel X (sint16, m/s² × 100)
            [2-3]   Accel Y (sint16, m/s² × 100)
            [4-5]   Accel Z (sint16, m/s² × 100)
            [6-7]   Gyro X  (sint16, rad/s × 100)
            [8-9]   Gyro Y  (sint16, rad/s × 100)
            [10-11] Gyro Z  (sint16, rad/s × 100)
        """
        ax, ay, az, gx, gy, gz = struct.unpack('<6h', data[:12])

        msg = Imu()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = 'imu_link'

        # Divide by 100 to recover real values
        msg.linear_acceleration.x = float(ax) / 100.0
        msg.linear_acceleration.y = float(ay) / 100.0
        msg.linear_acceleration.z = float(az) / 100.0

        msg.angular_velocity.x = float(gx) / 100.0
        msg.angular_velocity.y = float(gy) / 100.0
        msg.angular_velocity.z = float(gz) / 100.0

        # No orientation estimate from raw data
        msg.orientation_covariance[0] = -1.0

        self.imu_pub.publish(msg)

    # =================== Diagnostics ===================

    def print_stats(self):
        self.get_logger().info(
            f'Bridge Stats | RX: {self.rx_count} | TX: {self.tx_count} | Errors: {self.error_count}'
        )

    def destroy_node(self):
        self.running = False
        if self.ser.is_open:
            # Send motor stop before shutting down
            self.send_packet(CMD_MOTOR_STOP)
            self.ser.close()
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = TM4CBridgeNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
