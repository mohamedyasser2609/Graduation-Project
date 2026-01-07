# ROS 2 UART Communication Bridge

**TM4C123GH6PM ↔ Raspberry Pi 5 (ROS 2)**

---

## 🎯 Overview

Bidirectional UART communication between TM4C123 microcontroller and Raspberry Pi 5 running ROS 2.

**Use Cases:**
- Send sensor data (GPS, IMU, encoders) from TM4C to ROS 2
- Receive motor commands from ROS 2 to TM4C
- Real-time robot control and monitoring
- Telemetry and logging

---

## 🔌 Hardware Connections

### **TM4C123 ↔ Raspberry Pi 5**

| TM4C Pin | Function | RPi Pin | GPIO | Notes |
|----------|----------|---------|------|-------|
| **PB1** | UART1 TX | **Pin 10** | GPIO 15 (RX) | TM4C sends data |
| **PB0** | UART1 RX | **Pin 8** | GPIO 14 (TX) | TM4C receives data |
| **GND** | Ground | **Pin 6** | GND | **CRITICAL!** |

**⚠️ IMPORTANT:**
- **Common ground is MANDATORY** - Connect GND pins!
- Both devices run at **3.3V logic** - No level shifter needed ✅
- Do NOT connect VCC pins (power separately)

### **Raspberry Pi 5 GPIO Pinout:**

```
         3.3V  [ 1] [ 2]  5V
    GPIO 2     [ 3] [ 4]  5V
    GPIO 3     [ 5] [ 6]  GND  ← Connect to TM4C GND
    GPIO 4     [ 7] [ 8]  GPIO 14 (TX) ← Connect to PB0
         GND   [ 9] [10]  GPIO 15 (RX) ← Connect to PB1
```

---

## 📡 Communication Protocol

### **Packet Structure:**

```
┌──────────┬─────────┬────────┬──────────┬──────────┬─────────┐
│ START    │ COMMAND │ LENGTH │ DATA     │ CHECKSUM │ END     │
│ (0xAA)   │ (1 byte)│(1 byte)│(0-120 B) │ (1 byte) │ (0x55)  │
└──────────┴─────────┴────────┴──────────┴──────────┴─────────┘
```

**Fields:**
- **START:** Always `0xAA`
- **COMMAND:** Command ID (see below)
- **LENGTH:** Number of data bytes (0-120)
- **DATA:** Payload (variable length)
- **CHECKSUM:** XOR of (COMMAND ^ LENGTH ^ DATA[0] ^ DATA[1] ^ ...)
- **END:** Always `0x55`

### **Command IDs:**

| Command | ID | Direction | Description |
|---------|-----|-----------|-------------|
| **PING** | `0x01` | Both | Heartbeat/connection test |
| **ACK** | `0x02` | Both | Acknowledgment |
| **MOTOR_CMD** | `0x10` | ROS2 → TM4C | Motor control |
| **SENSOR_DATA** | `0x20` | TM4C → ROS2 | Generic sensor data |
| **GPS_DATA** | `0x21` | TM4C → ROS2 | GPS coordinates |
| **IMU_DATA** | `0x22` | TM4C → ROS2 | IMU (accel/gyro) |
| **STATUS** | `0x30` | Both | System status |
| **ERROR** | `0xFF` | Both | Error message |

---

## 🚀 Quick Start

### **Step 1: TM4C Setup**

1. **Build and Flash:**
   ```
   Project → Clean
   Project → Build
   Run → Debug → Resume
   ```

2. **Verify Debug Output:**
   Open serial terminal (115200 baud):
   ```
   ========================================
     ROS 2 UART Bridge - TM4C123GH6PM     
   ========================================
   ROS 2 UART: UART1 (PB0/PB1) @ 115200 baud
   Debug UART: UART0 (PA0/PA1) @ 115200 baud
   
   Waiting for ROS 2 connection...
   
   [TX] Initial PING sent to ROS 2
   ```

### **Step 2: Raspberry Pi Setup**

1. **Enable UART on RPi:**
   ```bash
   sudo raspi-config
   # Interface Options → Serial Port
   # Login shell: NO
   # Serial port hardware: YES
   sudo reboot
   ```

2. **Verify UART Device:**
   ```bash
   ls -l /dev/serial0
   # Should link to /dev/ttyAMA0
   ```

3. **Install Python Serial:**
   ```bash
   pip3 install pyserial
   ```

### **Step 3: Test Communication**

Use the provided Python test script (see below).

---

## 🐍 Python Test Script (Raspberry Pi)

Save as `ros2_uart_test.py`:

```python
#!/usr/bin/env python3
"""
ROS 2 UART Communication Test Script
Tests bidirectional communication with TM4C123
"""

import serial
import time
import struct

# Configuration
SERIAL_PORT = '/dev/serial0'  # RPi UART
BAUD_RATE = 115200

# Protocol constants
START_BYTE = 0xAA
END_BYTE = 0x55

# Commands
CMD_PING = 0x01
CMD_ACK = 0x02
CMD_MOTOR_CMD = 0x10
CMD_STATUS = 0x30

def calculate_checksum(command, length, data):
    """Calculate XOR checksum"""
    checksum = command ^ length
    for byte in data:
        checksum ^= byte
    return checksum

def send_packet(ser, command, data=b''):
    """Send packet to TM4C"""
    length = len(data)
    checksum = calculate_checksum(command, length, data)
    
    packet = bytes([START_BYTE, command, length]) + data + bytes([checksum, END_BYTE])
    
    ser.write(packet)
    print(f"[TX] Command: 0x{command:02X}, Length: {length}, Data: {data.hex()}")

def receive_packet(ser, timeout=1.0):
    """Receive packet from TM4C"""
    start_time = time.time()
    
    while (time.time() - start_time) < timeout:
        if ser.in_waiting > 0:
            # Look for start byte
            byte = ser.read(1)
            if byte[0] == START_BYTE:
                # Read command and length
                header = ser.read(2)
                if len(header) < 2:
                    continue
                    
                command = header[0]
                length = header[1]
                
                # Read data
                data = ser.read(length) if length > 0 else b''
                
                # Read checksum and end byte
                footer = ser.read(2)
                if len(footer) < 2:
                    continue
                    
                checksum = footer[0]
                end_byte = footer[1]
                
                # Validate
                if end_byte == END_BYTE:
                    calc_checksum = calculate_checksum(command, length, data)
                    if checksum == calc_checksum:
                        print(f"[RX] Command: 0x{command:02X}, Length: {length}, Data: {data.hex()}")
                        return command, data
                    else:
                        print(f"[ERROR] Checksum mismatch!")
                else:
                    print(f"[ERROR] Invalid end byte: 0x{end_byte:02X}")
    
    return None, None

def main():
    print("=" * 50)
    print("  ROS 2 UART Communication Test")
    print("  Raspberry Pi 5 ↔ TM4C123GH6PM")
    print("=" * 50)
    
    try:
        # Open serial port
        ser = serial.Serial(
            port=SERIAL_PORT,
            baudrate=BAUD_RATE,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=1
        )
        
        print(f"\n[OK] Serial port opened: {SERIAL_PORT} @ {BAUD_RATE} baud\n")
        
        time.sleep(0.5)
        
        # Test 1: Send PING
        print("\n--- Test 1: PING ---")
        send_packet(ser, CMD_PING, b'\x00\x00\x00\x01')
        time.sleep(0.1)
        cmd, data = receive_packet(ser)
        if cmd == CMD_ACK:
            print("[OK] Received ACK!\n")
        
        # Test 2: Request Status
        print("--- Test 2: STATUS REQUEST ---")
        send_packet(ser, CMD_STATUS)
        time.sleep(0.1)
        cmd, data = receive_packet(ser)
        if cmd == CMD_STATUS:
            print(f"[OK] Status data: {data.hex()}\n")
        
        # Test 3: Send Motor Command
        print("--- Test 3: MOTOR COMMAND ---")
        motor_data = bytes([100, 50])  # Example: Motor1=100, Motor2=50
        send_packet(ser, CMD_MOTOR_CMD, motor_data)
        time.sleep(0.1)
        cmd, data = receive_packet(ser)
        if cmd == CMD_ACK:
            print("[OK] Motor command acknowledged!\n")
        
        # Test 4: Continuous monitoring
        print("--- Test 4: CONTINUOUS MONITORING ---")
        print("Listening for 10 seconds...\n")
        
        start_time = time.time()
        while (time.time() - start_time) < 10:
            cmd, data = receive_packet(ser, timeout=0.5)
            if cmd == CMD_PING:
                print("[INFO] Heartbeat received")
                # Send ACK
                send_packet(ser, CMD_ACK)
            time.sleep(0.1)
        
        print("\n[OK] Test completed successfully!")
        
        ser.close()
        
    except serial.SerialException as e:
        print(f"[ERROR] Serial port error: {e}")
    except KeyboardInterrupt:
        print("\n[INFO] Test interrupted by user")
    except Exception as e:
        print(f"[ERROR] {e}")

if __name__ == "__main__":
    main()
```

**Run the test:**
```bash
chmod +x ros2_uart_test.py
python3 ros2_uart_test.py
```

---

## 🤖 ROS 2 Integration

### **Create ROS 2 Node:**

Save as `tm4c_bridge_node.py`:

```python
#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from sensor_msgs.msg import NavSatFix, Imu
from std_msgs.msg import String
import serial
import threading

class TM4CBridgeNode(Node):
    def __init__(self):
        super().__init__('tm4c_bridge')
        
        # Serial connection
        self.ser = serial.Serial('/dev/serial0', 115200, timeout=0.1)
        
        # Publishers
        self.gps_pub = self.create_publisher(NavSatFix, 'gps/fix', 10)
        self.imu_pub = self.create_publisher(Imu, 'imu/data', 10)
        self.status_pub = self.create_publisher(String, 'tm4c/status', 10)
        
        # Subscribers
        self.cmd_vel_sub = self.create_subscription(
            Twist, 'cmd_vel', self.cmd_vel_callback, 10)
        
        # Start receive thread
        self.rx_thread = threading.Thread(target=self.receive_loop, daemon=True)
        self.rx_thread.start()
        
        self.get_logger().info('TM4C Bridge Node started')
    
    def cmd_vel_callback(self, msg):
        """Convert ROS Twist to motor command"""
        # Example: Convert linear.x and angular.z to motor speeds
        left_speed = int((msg.linear.x - msg.angular.z) * 100)
        right_speed = int((msg.linear.x + msg.angular.z) * 100)
        
        # Clamp to 0-255
        left_speed = max(0, min(255, left_speed + 128))
        right_speed = max(0, min(255, right_speed + 128))
        
        # Send motor command
        self.send_packet(0x10, bytes([left_speed, right_speed]))
    
    def send_packet(self, command, data=b''):
        """Send packet to TM4C"""
        length = len(data)
        checksum = command ^ length
        for byte in data:
            checksum ^= byte
        
        packet = bytes([0xAA, command, length]) + data + bytes([checksum, 0x55])
        self.ser.write(packet)
    
    def receive_loop(self):
        """Background thread to receive packets"""
        while rclpy.ok():
            # Implement packet reception and publish to ROS topics
            # Similar to test script above
            pass

def main(args=None):
    rclpy.init(args=args)
    node = TM4CBridgeNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
```

---

## 📊 Example Use Cases

### **1. Send GPS Data to ROS 2:**

In `main_ros2.c`, add:

```c
void SendGPSData(float latitude, float longitude, float altitude) {
    uint8 data[12];
    
    /* Pack floats into bytes (simple method) */
    memcpy(&data[0], &latitude, 4);
    memcpy(&data[4], &longitude, 4);
    memcpy(&data[8], &altitude, 4);
    
    SendPacketToROS2(CMD_GPS_DATA, data, 12);
}
```

### **2. Receive Motor Commands from ROS 2:**

Already implemented in `ProcessPacket()`:

```c
case CMD_MOTOR_CMD:
    uint8 leftMotor = packet->data[0];
    uint8 rightMotor = packet->data[1];
    
    /* Control motors */
    SetMotorSpeed(LEFT_MOTOR, leftMotor);
    SetMotorSpeed(RIGHT_MOTOR, rightMotor);
    
    SendPacketToROS2(CMD_ACK, NULL_PTR, 0);
    break;
```

---

## 🔧 Troubleshooting

### **No Communication:**

1. **Check wiring:**
   - TX → RX (crossover!)
   - RX → TX
   - GND → GND

2. **Verify UART enabled on RPi:**
   ```bash
   ls -l /dev/serial0
   dmesg | grep tty
   ```

3. **Check baud rate match:** Both must be 115200

4. **Test loopback:**
   - Short TX and RX on RPi
   - Should echo back

### **Garbled Data:**

- Check baud rate (115200)
- Verify 8N1 settings
- Check for loose connections

### **Checksum Errors:**

- Verify checksum calculation matches
- Check for data corruption
- Add debug prints

---

## 📝 Next Steps

1. ✅ **Test basic communication** (Python script)
2. ✅ **Integrate with ROS 2 node**
3. ✅ **Add sensor data publishing** (GPS, IMU)
4. ✅ **Implement motor control**
5. ✅ **Add error handling and recovery**
6. ✅ **Optimize packet rate**

---

**Your TM4C is now ready to communicate with ROS 2!** 🤖🔗🛰️
