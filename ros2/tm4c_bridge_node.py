#!/usr/bin/env python3
"""
TM4C ComStack UART Bridge Node for ROS2 — v3.0
================================================
Production bridge between TM4C123GH6PM hardware UART and ROS2 topics.

Protocol v3.0 — Matches ComStack_Types.h on TM4C firmware.

Data Flow:
    ROS2 /cmd_vel  ──►  Bridge  ──►  TWIST_CMD (0x12)  ──►  TM4C
    TM4C  ──►  ENCODER_DATA (0x23)  ──►  Bridge  ──►  /encoder_ticks
    TM4C  ──►  IMU_DATA     (0x22)  ──►  Bridge  ──►  /imu/data_raw
    TM4C  ──►  STATUS       (0x30)  ──►  Bridge  ──►  /robot_status (JSON)
                                                  ──►  /diagnostics

Key Design Decisions (v3.0):
    - TWIST_CMD (0x12) sends (v, ω) directly to TM4C — NO differential
      drive math in the bridge. The TM4C firmware applies kinematics.
    - STATUS (0x30) telemetry: battery, currents, temps, fan, velocity
      parsed and published as JSON + diagnostics at 1 Hz.
    - Legacy MOTOR_CMD (0x10) is NOT sent by this bridge but the TM4C
      still accepts it for backward compatibility.

Wire Protocol:
    | 0xAA | CMD(1) | LEN(1) | DATA(0-120) | XOR_CHECKSUM(1) | 0x55 |

    Checksum = CMD ^ LEN ^ DATA[0] ^ DATA[1] ^ ... ^ DATA[N-1]
    Byte order: little-endian for all multi-byte values.

Usage:
    1. Connect TM4C UART1 (PB0/PB1) to RPi UART (GPIO14/15)
    2. ros2 run ugv_robot tm4c_bridge_node
    3. Verify: ros2 topic echo /encoder_ticks
    4. Drive:  ros2 topic pub /cmd_vel geometry_msgs/msg/Twist \
               "{linear: {x: 0.3}, angular: {z: 0.0}}"

Author:  Mohamed Yasser
Date:    April 19, 2026
Version: 3.0.0
"""

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, DurabilityPolicy
from std_msgs.msg import Int32MultiArray, String
from sensor_msgs.msg import Imu
from geometry_msgs.msg import Twist
from diagnostic_msgs.msg import DiagnosticArray, DiagnosticStatus, KeyValue
import serial
import struct
import threading
import time
import json
import math


# =====================================================================
#  ComStack Protocol Constants  (must match ComStack_Types.h on TM4C)
# =====================================================================

START_BYTE          = 0xAA
END_BYTE            = 0x55

# --- Command IDs ---
CMD_PING            = 0x01      # Heartbeat (both directions)
CMD_ACK             = 0x02      # Acknowledgment
CMD_NACK            = 0x03      # Negative acknowledgment

CMD_MOTOR_CMD       = 0x10      # Legacy L/R wheel % (backward compat)
CMD_MOTOR_STOP      = 0x11      # Emergency stop (0 bytes)
CMD_TWIST_CMD       = 0x12      # ⭐ Velocity (v, ω) — PREFERRED

CMD_IMU_DATA        = 0x22      # IMU accel + gyro (12 bytes)
CMD_ENCODER_DATA    = 0x23      # Encoder ticks + velocity (12 bytes)

CMD_STATUS          = 0x30      # ⭐ System telemetry (20 bytes, 1 Hz)

# --- Robot Parameters (must match Robot_Control.h on TM4C) ---
ROBOT_WHEEL_RADIUS  = 0.065     # meters (65 mm)
ROBOT_WHEEL_BASE    = 0.30      # meters (center-to-center)
ROBOT_ENCODER_CPR   = 1440      # counts per revolution
ROBOT_MAX_LINEAR    = 1.0       # m/s
ROBOT_MAX_ANGULAR   = 3.14      # rad/s

# --- System State Names (Robot_StateType enum) ---
SYSTEM_STATE_NAMES = {
    0: 'UNINIT',
    1: 'IDLE',
    2: 'RUNNING',
    3: 'ESTOP',
    4: 'FAULT',
}

# --- Error Flag Bit Definitions ---
ERROR_FLAG_NAMES = {
    0: 'MOTOR_L_OVERLOAD',
    1: 'MOTOR_R_OVERLOAD',
    2: 'MOTOR_L_THERMAL',
    3: 'MOTOR_R_THERMAL',
    4: 'ENCLOSURE_THERMAL',
    5: 'ENCODER_L_FAULT',
    6: 'ENCODER_R_FAULT',
    7: 'CMD_TIMEOUT',
}


# =====================================================================
#  TM4C Bridge Node
# =====================================================================

class TM4CBridgeNode(Node):
    """
    ROS2 node bridging TM4C123GH6PM firmware ↔ ROS2 ecosystem.

    TX (ROS2 → TM4C):
        /cmd_vel  →  CMD 0x12 TWIST_CMD  (4 bytes: v×1000, ω×1000)
        Heartbeat →  CMD 0x01 PING       (every 2 seconds)

    RX (TM4C → ROS2):
        CMD 0x23  →  /encoder_ticks      (Int32MultiArray, 50 Hz)
        CMD 0x22  →  /imu/data_raw       (sensor_msgs/Imu, 50 Hz)
        CMD 0x30  →  /robot_status       (JSON String, 1 Hz)
                  →  /diagnostics        (DiagnosticArray, 1 Hz)
    """

    def __init__(self):
        super().__init__('tm4c_bridge')

        # ── Parameters ──────────────────────────────────────────────
        self.declare_parameter('serial_port', '/dev/ttyAMA0')
        self.declare_parameter('baud_rate', 115200)
        self.declare_parameter('heartbeat_interval', 2.0)    # seconds
        self.declare_parameter('stats_interval', 5.0)        # seconds
        self.declare_parameter('cmd_vel_timeout', 0.5)       # seconds

        port = self.get_parameter('serial_port').value
        baud = self.get_parameter('baud_rate').value
        hb_interval = self.get_parameter('heartbeat_interval').value
        stats_interval = self.get_parameter('stats_interval').value
        self._cmd_vel_timeout = self.get_parameter('cmd_vel_timeout').value

        # ── Serial Port ─────────────────────────────────────────────
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
            self.get_logger().error(f'Cannot open serial port: {e}')
            raise

        # ── QoS Profiles ────────────────────────────────────────────
        sensor_qos = QoSProfile(
            depth=10,
            reliability=ReliabilityPolicy.BEST_EFFORT,
            durability=DurabilityPolicy.VOLATILE
        )

        # ── Publishers ──────────────────────────────────────────────
        self.encoder_pub = self.create_publisher(
            Int32MultiArray, '/encoder_ticks', sensor_qos)
        self.imu_pub = self.create_publisher(
            Imu, '/imu/data_raw', sensor_qos)
        self.status_pub = self.create_publisher(
            String, '/robot_status', 10)
        self.diag_pub = self.create_publisher(
            DiagnosticArray, '/diagnostics', 10)

        # ── Subscribers ─────────────────────────────────────────────
        self.cmd_vel_sub = self.create_subscription(
            Twist, '/cmd_vel', self.cmd_vel_callback, 10)

        # ── Internal State ──────────────────────────────────────────
        self.rx_count = 0
        self.tx_count = 0
        self.error_count = 0
        self._last_cmd_vel_time = 0.0
        self._last_status = {}            # Most recent parsed telemetry
        self._serial_lock = threading.Lock()

        # ── Timers ──────────────────────────────────────────────────
        self.create_timer(hb_interval, self.send_ping)
        self.create_timer(stats_interval, self.print_stats)
        self.create_timer(self._cmd_vel_timeout, self._check_cmd_vel_timeout)

        # ── RX Thread ───────────────────────────────────────────────
        self.running = True
        self.rx_thread = threading.Thread(
            target=self.receive_loop, daemon=True, name='uart_rx')
        self.rx_thread.start()

        self.get_logger().info(
            'TM4C Bridge Node v3.0 started '
            '(TWIST_CMD mode — diff-drive on TM4C side)')

    # =================================================================
    #  TRANSMIT  (ROS2 → TM4C)
    # =================================================================

    def calculate_checksum(self, command: int, length: int, data: bytes) -> int:
        """XOR checksum: CMD ^ LEN ^ DATA[0] ^ DATA[1] ^ ... ^ DATA[N-1]"""
        checksum = command ^ length
        for byte in data:
            checksum ^= byte
        return checksum & 0xFF

    def send_packet(self, command: int, data: bytes = b'') -> bool:
        """
        Send a ComStack binary packet to TM4C via UART.

        Packet format:
            [0xAA] [CMD] [LEN] [DATA...] [XOR_CHECKSUM] [0x55]

        Returns True on success, False on serial error.
        """
        length = len(data)
        checksum = self.calculate_checksum(command, length, data)
        packet = bytes([START_BYTE, command, length]) + data + \
                 bytes([checksum, END_BYTE])
        try:
            with self._serial_lock:
                self.ser.write(packet)
            self.tx_count += 1
            return True
        except serial.SerialException as e:
            self.get_logger().error(f'TX error: {e}')
            return False

    def cmd_vel_callback(self, msg: Twist):
        """
        Convert ROS2 Twist (/cmd_vel) → TWIST_CMD (0x12).

        Simply packs linear.x and angular.z as fixed-point × 1000
        (mm/s and mrad/s respectively).

        The TM4C firmware applies the differential drive equation:
            v_left  = v - (ω × L / 2)
            v_right = v + (ω × L / 2)
            Where L = 0.30 m (wheel base)

        Then runs PID velocity control on each wheel.
        """
        # Scale to fixed-point × 1000
        linear_mmps = int(msg.linear.x * 1000.0)
        angular_mrads = int(msg.angular.z * 1000.0)

        # Clamp to sint16 range (-32768 .. +32767)
        linear_mmps = max(-32768, min(32767, linear_mmps))
        angular_mrads = max(-32768, min(32767, angular_mrads))

        # Pack as two little-endian sint16 and send CMD 0x12
        data = struct.pack('<hh', linear_mmps, angular_mrads)
        self.send_packet(CMD_TWIST_CMD, data)

        # Track last command time for timeout watchdog
        self._last_cmd_vel_time = time.monotonic()

    def send_ping(self):
        """Send heartbeat ping to TM4C (CMD 0x01)."""
        self.send_packet(CMD_PING)

    def _check_cmd_vel_timeout(self):
        """
        Safety: if no /cmd_vel received for cmd_vel_timeout seconds,
        send a zero-velocity twist to stop the robot.

        This prevents runaway if the navigation stack crashes.
        """
        if self._last_cmd_vel_time == 0.0:
            return  # No command ever received — nothing to timeout

        elapsed = time.monotonic() - self._last_cmd_vel_time
        if elapsed > self._cmd_vel_timeout:
            # Send zero velocity (stop)
            data = struct.pack('<hh', 0, 0)
            self.send_packet(CMD_TWIST_CMD, data)
            # Reset so we don't spam stop commands
            self._last_cmd_vel_time = 0.0

    # =================================================================
    #  RECEIVE  (TM4C → ROS2)
    # =================================================================

    def receive_loop(self):
        """
        Background thread: continuously read UART bytes and parse
        ComStack packets using a simple state machine.

        Packet format:
            [0xAA] [CMD] [LEN] [DATA...] [XOR_CHECKSUM] [0x55]
        """
        while self.running:
            try:
                # Wait for start byte
                byte = self.ser.read(1)
                if len(byte) == 0:
                    continue
                if byte[0] != START_BYTE:
                    continue

                # Read command + length (2 bytes)
                header = self.ser.read(2)
                if len(header) < 2:
                    self.error_count += 1
                    continue

                command = header[0]
                length = header[1]

                # Sanity check length (max 120 bytes per protocol)
                if length > 120:
                    self.error_count += 1
                    continue

                # Read data payload
                data = self.ser.read(length) if length > 0 else b''
                if len(data) < length:
                    self.error_count += 1
                    continue

                # Read checksum + end byte (2 bytes)
                footer = self.ser.read(2)
                if len(footer) < 2:
                    self.error_count += 1
                    continue

                rx_checksum = footer[0]
                end_byte = footer[1]

                # Validate end byte
                if end_byte != END_BYTE:
                    self.error_count += 1
                    continue

                # Validate checksum
                calc_checksum = self.calculate_checksum(command, length, data)
                if rx_checksum != calc_checksum:
                    self.error_count += 1
                    continue

                # ── Valid packet ──
                self.rx_count += 1
                self.handle_packet(command, data)

            except serial.SerialException:
                time.sleep(0.1)
            except Exception as e:
                self.get_logger().error(f'RX error: {e}')
                time.sleep(0.01)

    def handle_packet(self, command: int, data: bytes):
        """Route a validated packet to the correct ROS2 publisher."""

        if command == CMD_ENCODER_DATA and len(data) >= 12:
            self.publish_encoder(data)

        elif command == CMD_IMU_DATA and len(data) >= 12:
            self.publish_imu(data)

        elif command == CMD_STATUS and len(data) >= 20:
            self.publish_status(data)

        elif command == CMD_ACK:
            pass  # Heartbeat ACK — connection confirmed

        elif command == CMD_PING:
            # TM4C sent us a ping — reply with ACK
            self.send_packet(CMD_ACK)

        elif command == CMD_NACK:
            error_code = data[0] if len(data) >= 1 else 0xFF
            self.get_logger().warn(f'TM4C NACK received, error code: 0x{error_code:02X}')

    # -----------------------------------------------------------------
    #  ENCODER_DATA (CMD 0x23) — 12 bytes — 50 Hz
    # -----------------------------------------------------------------

    def publish_encoder(self, data: bytes):
        """
        Parse encoder packet and publish to /encoder_ticks.

        Wire format (12 bytes, little-endian):
            [0-3]   sint32  Left encoder ticks   (raw count)
            [4-7]   sint32  Right encoder ticks  (raw count)
            [8-9]   sint16  Left velocity        (RPM × 100)
            [10-11] sint16  Right velocity       (RPM × 100)

        Publishes:
            /encoder_ticks  →  Int32MultiArray [left_ticks, right_ticks]

        Odometry math (for reference — done in odometry_node):
            distance_per_tick = (2π × 0.065) / 1440 ≈ 0.000284 m/tick
        """
        left_ticks, right_ticks, left_vel_x100, right_vel_x100 = \
            struct.unpack('<iihh', data[:12])

        msg = Int32MultiArray()
        msg.data = [left_ticks, right_ticks]
        self.encoder_pub.publish(msg)

    # -----------------------------------------------------------------
    #  IMU_DATA (CMD 0x22) — 12 bytes — 50 Hz
    # -----------------------------------------------------------------

    def publish_imu(self, data: bytes):
        """
        Parse IMU packet and publish to /imu/data_raw.

        Wire format (12 bytes, little-endian, fixed-point × 100):
            [0-1]   sint16  Accel X  (m/s² × 100)
            [2-3]   sint16  Accel Y  (m/s² × 100)
            [4-5]   sint16  Accel Z  (m/s² × 100)
            [6-7]   sint16  Gyro X   (rad/s × 100)
            [8-9]   sint16  Gyro Y   (rad/s × 100)
            [10-11] sint16  Gyro Z   (rad/s × 100)

        Publishes:
            /imu/data_raw  →  sensor_msgs/Imu (no orientation)
        """
        ax, ay, az, gx, gy, gz = struct.unpack('<6h', data[:12])

        msg = Imu()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = 'imu_link'

        # Recover real values (divide by scale factor 100)
        msg.linear_acceleration.x = float(ax) / 100.0
        msg.linear_acceleration.y = float(ay) / 100.0
        msg.linear_acceleration.z = float(az) / 100.0

        msg.angular_velocity.x = float(gx) / 100.0
        msg.angular_velocity.y = float(gy) / 100.0
        msg.angular_velocity.z = float(gz) / 100.0

        # Providing Virtual Orientation (Identity) for stable SLAM/EKF testing
        msg.orientation.w = 1.0
        msg.orientation.x = 0.0
        msg.orientation.y = 0.0
        msg.orientation.z = 0.0
        
        # Set small covariance to mark orientation as valid (no longer -1)
        msg.orientation_covariance[0] = 0.01

        self.imu_pub.publish(msg)

    # -----------------------------------------------------------------
    #  STATUS / TELEMETRY (CMD 0x30) — 20 bytes — 1 Hz
    # -----------------------------------------------------------------

    def publish_status(self, data: bytes):
        """
        Parse 20-byte STATUS telemetry and publish to:
            /robot_status   →  JSON String (for GUI)
            /diagnostics    →  DiagnosticArray (for ROS2 tooling)

        Wire format (20 bytes, little-endian):
            [0]     uint8   SystemState         (Robot_StateType enum)
            [1]     uint8   ErrorFlags          (fault bitmap)
            [2-3]   sint16  BatteryVoltage      (mV = V × 1000)
            [4]     uint8   BatteryPercent      (0-100%)
            [5-6]   sint16  LeftMotorCurrent    (mA = A × 1000)
            [7-8]   sint16  RightMotorCurrent   (mA = A × 1000)
            [9-10]  sint16  TempMotors          (°C × 10)
            [11-12] sint16  TempMCU             (°C × 10)
            [13-14] sint16  TempBattery         (°C × 10)
            [15]    uint8   FanSpeed            (0-100%)
            [16-17] sint16  LinearVelocity      (mm/s = m/s × 1000)
            [18-19] sint16  AngularVelocity     (mrad/s = rad/s × 1000)
        """
        # Unpack all fields
        (state, errors, batt_mv, batt_pct,
         i_left, i_right,
         t_motors, t_mcu, t_battery,
         fan_speed,
         lin_vel, ang_vel) = struct.unpack('<BBhBhhhhhBhh', data[:20])

        # Build decoded status dict
        state_name = SYSTEM_STATE_NAMES.get(state, f'UNKNOWN({state})')

        # Decode error flags
        active_errors = []
        for bit, name in ERROR_FLAG_NAMES.items():
            if errors & (1 << bit):
                active_errors.append(name)

        status = {
            'state':            state_name,
            'state_id':         state,
            'error_flags':      errors,
            'active_errors':    active_errors,
            'battery_v':        round(batt_mv / 1000.0, 2),
            'battery_pct':      batt_pct,
            'current_left_a':   round(i_left / 1000.0, 3),
            'current_right_a':  round(i_right / 1000.0, 3),
            'temp_motors_c':    round(t_motors / 10.0, 1),
            'temp_mcu_c':       round(t_mcu / 10.0, 1),
            'temp_battery_c':   round(t_battery / 10.0, 1),
            'fan_speed_pct':    fan_speed,
            'linear_vel_mps':   round(lin_vel / 1000.0, 3),
            'angular_vel_rps':  round(ang_vel / 1000.0, 3),
            'timestamp':        time.time(),
        }

        self._last_status = status

        # ── Publish JSON string on /robot_status ──
        json_msg = String()
        json_msg.data = json.dumps(status)
        self.status_pub.publish(json_msg)

        # ── Publish ROS2 DiagnosticArray on /diagnostics ──
        self._publish_diagnostics(status)

    def _publish_diagnostics(self, status: dict):
        """Convert status dict into standard ROS2 DiagnosticArray."""
        diag_msg = DiagnosticArray()
        diag_msg.header.stamp = self.get_clock().now().to_msg()

        # Determine overall level
        if status['error_flags'] != 0 or status['state_id'] >= 3:
            level = DiagnosticStatus.ERROR
        elif status['battery_pct'] < 20 or \
             status['temp_motors_c'] > 60.0 or \
             status['temp_mcu_c'] > 70.0:
            level = DiagnosticStatus.WARN
        else:
            level = DiagnosticStatus.OK

        diag_status = DiagnosticStatus()
        diag_status.level = level
        diag_status.name = 'UGV Robot Controller'
        diag_status.hardware_id = 'TM4C123GH6PM'
        diag_status.message = f"State: {status['state']} | " \
                              f"Battery: {status['battery_pct']}% | " \
                              f"Errors: {status['error_flags']:#04x}"

        diag_status.values = [
            KeyValue(key='system_state',        value=status['state']),
            KeyValue(key='error_flags',         value=f"0x{status['error_flags']:02X}"),
            KeyValue(key='active_errors',       value=', '.join(status['active_errors']) or 'none'),
            KeyValue(key='battery_voltage',     value=f"{status['battery_v']:.2f} V"),
            KeyValue(key='battery_percent',     value=f"{status['battery_pct']}%"),
            KeyValue(key='current_left',        value=f"{status['current_left_a']:.3f} A"),
            KeyValue(key='current_right',       value=f"{status['current_right_a']:.3f} A"),
            KeyValue(key='temp_motors',         value=f"{status['temp_motors_c']:.1f} °C"),
            KeyValue(key='temp_mcu',            value=f"{status['temp_mcu_c']:.1f} °C"),
            KeyValue(key='temp_battery',        value=f"{status['temp_battery_c']:.1f} °C"),
            KeyValue(key='fan_speed',           value=f"{status['fan_speed_pct']}%"),
            KeyValue(key='linear_velocity',     value=f"{status['linear_vel_mps']:.3f} m/s"),
            KeyValue(key='angular_velocity',    value=f"{status['angular_vel_rps']:.3f} rad/s"),
            KeyValue(key='bridge_rx_count',     value=str(self.rx_count)),
            KeyValue(key='bridge_tx_count',     value=str(self.tx_count)),
            KeyValue(key='bridge_error_count',  value=str(self.error_count)),
        ]

        diag_msg.status.append(diag_status)
        self.diag_pub.publish(diag_msg)

    # =================================================================
    #  DIAGNOSTICS & LIFECYCLE
    # =================================================================

    def print_stats(self):
        """Periodic stats log (every stats_interval seconds)."""
        state_str = self._last_status.get('state', 'N/A')
        batt_str = self._last_status.get('battery_pct', '?')
        self.get_logger().info(
            f'Bridge Stats | RX: {self.rx_count} | TX: {self.tx_count} | '
            f'Errors: {self.error_count} | '
            f'Robot: {state_str} | Battery: {batt_str}%'
        )

    def destroy_node(self):
        """Clean shutdown: stop motors, close serial."""
        self.running = False

        if hasattr(self, 'ser') and self.ser.is_open:
            # Send emergency stop before shutting down
            self.get_logger().info('Sending MOTOR_STOP before shutdown...')
            self.send_packet(CMD_MOTOR_STOP)
            time.sleep(0.05)  # Let the byte flush
            self.ser.close()
            self.get_logger().info('Serial port closed.')

        super().destroy_node()


# =====================================================================
#  Entry Point
# =====================================================================

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
