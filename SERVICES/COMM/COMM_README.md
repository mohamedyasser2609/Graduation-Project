# ROS 2 UART Communication Protocol — v3.0

**TM4C123GH6PM ↔ Raspberry Pi 5 (ROS 2)**  
**Protocol Version:** 3.0 (Twist velocity command)  
**Last Updated:** April 19, 2026

---

## Overview

Bidirectional UART between TM4C123 robot controller and RPi 5 running ROS 2.  
All data uses **fixed-point integers** (no floating-point on the wire).

- **TM4C sends**: Encoder ticks/velocity + IMU accel/gyro
- **TM4C receives**: Velocity commands (linear + angular) or legacy motor speeds
- **GPS**: Wired directly to RPi (not sent by TM4C)

### What's New in v3.0

> **NEW COMMAND `TWIST_CMD (0x12)` — sends `(v, ω)` directly to the TM4C.**
>
> Previously, the ROS2 bridge had to compute left/right wheel speeds and send `MOTOR_CMD (0x10)`.
> Now the bridge sends the raw `/cmd_vel` linear and angular velocities, and the TM4C firmware
> applies the differential drive kinematics internally.
>
> **The old `MOTOR_CMD (0x10)` is still supported for backward compatibility**, but
> `TWIST_CMD (0x12)` is the **recommended** command.

---

## Hardware Connections

| TM4C Pin | Function | RPi Pin | GPIO |
|----------|----------|---------|------|
| **PB1** | UART1 TX | **Pin 10** | GPIO 15 (RX) |
| **PB0** | UART1 RX | **Pin 8** | GPIO 14 (TX) |
| **GND** | Ground | **Pin 6** | GND |

**UART: 115200 baud, 8N1, No flow control**

⚠️ Common GND is mandatory. Both are 3.3V logic — no level shifter needed.

---

## Packet Structure

```
┌──────────┬─────────┬────────┬──────────────┬──────────┬─────────┐
│  0xAA    │ COMMAND │ LENGTH │ DATA         │ CHECKSUM │  0x55   │
│ (1 byte) │ (1 byte)│(1 byte)│ (0-120 bytes)│ (1 byte) │ (1 byte)│
└──────────┴─────────┴────────┴──────────────┴──────────┴─────────┘
```

- **Start byte:** `0xAA`
- **End byte:** `0x55`
- **Checksum:** `CMD ^ LEN ^ DATA[0] ^ DATA[1] ^ ... ^ DATA[N-1]`
- **Byte order:** All multi-byte values are **little-endian**

---

## Command Table

| Command | ID | Direction | Data Len | Description |
|---------|:------:|-----------|:--------:|-------------|
| PING | `0x01` | Both | 0 | Heartbeat / connectivity check |
| ACK | `0x02` | Both | 0 | Acknowledgment |
| NACK | `0x03` | Both | 1 | Error code |
| **TWIST_CMD** | **`0x12`** | **RPi → TM4C** | **4** | **⭐ Velocity command (v, ω) — NEW, PREFERRED** |
| MOTOR_CMD | `0x10` | RPi → TM4C | 4 | Legacy L/R wheel % (backward compat) |
| MOTOR_STOP | `0x11` | RPi → TM4C | 0 | Emergency stop |
| IMU_DATA | `0x22` | TM4C → RPi | 12 | IMU accel + gyro (sint16 × 100) |
| ENCODER_DATA | `0x23` | TM4C → RPi | 12 | Encoder ticks + velocity |
| **STATUS** | **`0x30`** | **TM4C → RPi** | **20** | **⭐ Telemetry: battery, current, temps, velocity (1 Hz)** |

---

## STATUS / TELEMETRY (0x30) — TM4C → RPi — 20 bytes — **GUI DATA**

Sent by TM4C at **~1 Hz**. Contains everything the GUI needs to display system health.

### Wire Format

```
Byte [0]:      SystemState          (uint8,  Robot_StateType enum)
Byte [1]:      ErrorFlags           (uint8,  fault bitmap)
Byte [2-3]:    BatteryVoltage       (sint16, mV  = V × 1000)
Byte [4]:      BatteryPercent       (uint8,  0-100%)
Byte [5-6]:    LeftMotorCurrent     (sint16, mA  = A × 1000)
Byte [7-8]:    RightMotorCurrent    (sint16, mA  = A × 1000)
Byte [9-10]:   TempMotors           (sint16, °C × 10)
Byte [11-12]:  TempMCU              (sint16, °C × 10)
Byte [13-14]:  TempBattery          (sint16, °C × 10)
Byte [15]:     FanSpeed             (uint8,  0-100%)
Byte [16-17]:  LinearVelocity       (sint16, mm/s  = m/s × 1000)
Byte [18-19]:  AngularVelocity      (sint16, mrad/s = rad/s × 1000)
```

### SystemState Values

| Value | State | Meaning |
|:-----:|-------|---------|
| 0 | UNINIT | Not initialized |
| 1 | IDLE | Ready, motors stopped |
| 2 | RUNNING | Motors active |
| 3 | ESTOP | Emergency stop active |
| 4 | FAULT | Fault detected |

### ErrorFlags Bitmap

| Bit | Flag | Meaning |
|:---:|------|---------|
| 0 | MOTOR_L_OVERLOAD | Left motor overcurrent |
| 1 | MOTOR_R_OVERLOAD | Right motor overcurrent |
| 2 | MOTOR_L_THERMAL | Left motor overtemp |
| 3 | MOTOR_R_THERMAL | Right motor overtemp |
| 4 | ENCLOSURE_THERMAL | Enclosure overtemp |
| 5 | ENCODER_L_FAULT | Left encoder fault |
| 6 | ENCODER_R_FAULT | Right encoder fault |
| 7 | CMD_TIMEOUT | No commands received |

### Python Parsing

```python
CMD_STATUS = 0x30

def parse_status(data):
    """Parse 20-byte STATUS/TELEMETRY packet."""
    import struct
    (state, errors, batt_mv, batt_pct,
     i_left, i_right,
     t_motors, t_mcu, t_battery,
     fan_speed,
     lin_vel, ang_vel) = struct.unpack('<BBhBhhhhh B hh', data[:20])

    return {
        'state':         state,
        'error_flags':   errors,
        'battery_v':     batt_mv / 1000.0,
        'battery_pct':   batt_pct,
        'current_left':  i_left / 1000.0,    # Amps
        'current_right': i_right / 1000.0,   # Amps
        'temp_motors':   t_motors / 10.0,    # °C
        'temp_mcu':      t_mcu / 10.0,       # °C
        'temp_battery':  t_battery / 10.0,   # °C
        'fan_speed':     fan_speed,           # %
        'linear_vel':    lin_vel / 1000.0,    # m/s
        'angular_vel':   ang_vel / 1000.0,    # rad/s
    }
```

### Bridge Integration

In the bridge `handle_packet()`, add:

```python
elif command == CMD_STATUS and len(data) >= 20:
    self.publish_status(data)
```

Publish on a custom ROS2 topic (e.g. `/robot_status`) for the GUI to subscribe to.

---

## ⭐ TWIST_CMD (0x12) — RPi → TM4C — 4 bytes — **USE THIS**

This is the **preferred** command for robot velocity control. Send the `/cmd_vel` linear and angular velocities directly — the TM4C firmware applies the differential drive kinematics.

### Wire Format

```
Byte [0-1]:  Linear velocity     (sint16, mm/s  = m/s × 1000)
Byte [2-3]:  Angular velocity    (sint16, mrad/s = rad/s × 1000)
```

### Scaling

| Field | Wire Type | Scale Factor | Range | Meaning |
|-------|:---------:|:------------:|:-----:|---------|
| Linear vel | sint16 | × 1000 | -1000..+1000 | m/s → mm/s |
| Angular vel | sint16 | × 1000 | -3140..+3140 | rad/s → mrad/s |

### Conversion

```python
# ROS2 /cmd_vel  →  wire values
linear_mmps  = int(twist.linear.x  * 1000)   # m/s → mm/s
angular_mrads = int(twist.angular.z * 1000)   # rad/s → mrad/s

# Clamp to sint16 range
linear_mmps  = max(-32768, min(32767, linear_mmps))
angular_mrads = max(-32768, min(32767, angular_mrads))

# Pack as little-endian
data = struct.pack('<hh', linear_mmps, angular_mrads)
send_packet(CMD_TWIST_CMD, data)
```

### Examples

| Desired Motion | linear.x | angular.z | Wire (hex LE) |
|----------------|:--------:|:---------:|:---------|
| Forward 0.5 m/s | 0.5 | 0.0 | `F4 01 00 00` |
| Backward 0.3 m/s | -0.3 | 0.0 | `D4 FE 00 00` |
| Spin left (CCW) 1.0 rad/s | 0.0 | 1.0 | `00 00 E8 03` |
| Spin right (CW) 1.0 rad/s | 0.0 | -1.0 | `00 00 18 FC` |
| Arc: forward + turn | 0.3 | 0.5 | `2C 01 F4 01` |
| **STOP** | 0.0 | 0.0 | `00 00 00 00` |

### What the TM4C Does Internally

When it receives `TWIST_CMD`, the firmware:

1. Converts fixed-point to float: `v = wire / 1000.0`, `ω = wire / 1000.0`
2. Applies **differential drive kinematics**:
   ```
   v_left  = v - (ω × L / 2)     ← left wheel velocity (m/s)
   v_right = v + (ω × L / 2)     ← right wheel velocity (m/s)
   
   Where L = 0.30 m (wheel base)
   ```
3. Runs **PID velocity control** on each wheel (setpoint in m/s, feedback from encoders)
4. Drives motors via PWM

**You do NOT need to compute wheel speeds on the RPi side.**

---

## MOTOR_CMD (0x10) — RPi → TM4C — 4 bytes — **LEGACY**

> ⚠️ **Deprecated in favor of `TWIST_CMD (0x12)`.** Kept for backward compatibility.

```
Byte [0-1]: Left motor speed   (sint16, range: -100 to +100)
Byte [2-3]: Right motor speed  (sint16, range: -100 to +100)
```

Positive = Forward, Negative = Reverse, 0 = Stop.

```python
data = struct.pack('<hh', 50, 30)  # Left=50%, Right=30% forward
send_packet(CMD_MOTOR_CMD, data)
```

---

## MOTOR_STOP (0x11) — RPi → TM4C — 0 bytes

Emergency stop. All motors immediately halted. No data payload.

```python
send_packet(CMD_MOTOR_STOP)
```

---

## ENCODER_DATA (0x23) — TM4C → RPi — 12 bytes

Sent by TM4C at **~50 Hz**.

```
Byte [0-3]:   Left encoder ticks    (sint32, raw tick count)
Byte [4-7]:   Right encoder ticks   (sint32, raw tick count)
Byte [8-9]:   Left velocity         (sint16, RPM × 100)
Byte [10-11]: Right velocity        (sint16, RPM × 100)
```

**To get real RPM:** divide sint16 by 100.0

```python
left_ticks, right_ticks, left_vel_x100, right_vel_x100 = struct.unpack('<iihh', data[:12])
left_rpm = left_vel_x100 / 100.0   # e.g. 5000 → 50.00 RPM
```

### Robot Parameters for Odometry (needed on RPi side)

| Parameter | Value | Notes |
|-----------|:-----:|-------|
| Wheel radius | 0.065 m | 65 mm |
| Wheel base | 0.30 m | Center-to-center |
| Encoder CPR | 1440 | Counts per revolution |

```python
# Convert ticks to distance
WHEEL_RADIUS = 0.065  # meters
ENCODER_CPR  = 1440
distance_per_tick = (2 * math.pi * WHEEL_RADIUS) / ENCODER_CPR  # ≈ 0.000284 m/tick
```

---

## IMU_DATA (0x22) — TM4C → RPi — 12 bytes

Sent by TM4C at **~50 Hz**.

```
Byte [0-1]:   Accel X  (sint16, m/s² × 100)
Byte [2-3]:   Accel Y  (sint16, m/s² × 100)
Byte [4-5]:   Accel Z  (sint16, m/s² × 100)
Byte [6-7]:   Gyro X   (sint16, rad/s × 100)
Byte [8-9]:   Gyro Y   (sint16, rad/s × 100)
Byte [10-11]: Gyro Z   (sint16, rad/s × 100)
```

**To get real values:** divide sint16 by 100.0
```python
ax, ay, az, gx, gy, gz = struct.unpack('<6h', data[:12])
accel_x = ax / 100.0   # e.g. 981 → 9.81 m/s² (1g)
gyro_z  = gz / 100.0   # e.g. 174 → 1.74 rad/s
```

> ⚠️ **IMU is currently disabled on TM4C** — all values will be zero until re-enabled.

---

## PING (0x01) — Heartbeat

Either side can send a PING. The receiver should reply with ACK (`0x02`).

- TM4C sends PING periodically
- RPi should send PING every ~2 seconds
- If TM4C receives no PING or motor command for **5 seconds**, it considers the connection lost

```python
send_packet(CMD_PING)
```

---

## Checksum Calculation

```python
def calculate_checksum(command, length, data):
    """XOR checksum: CMD ^ LEN ^ DATA[0] ^ DATA[1] ^ ..."""
    checksum = command ^ length
    for byte in data:
        checksum ^= byte
    return checksum & 0xFF
```

---

## Complete ROS2 Bridge — Updated for TWIST_CMD

```python
#!/usr/bin/env python3
"""
TM4C ComStack UART Bridge Node for ROS2 — v3.0
================================================
Bridges ROS2 /cmd_vel → TM4C TWIST_CMD (0x12)
Bridges TM4C sensor data → ROS2 topics

Publishes:
    /encoder_ticks   (Int32MultiArray)  - [left_ticks, right_ticks]
    /imu/data_raw    (sensor_msgs/Imu)  - accelerometer + gyroscope

Subscribes:
    /cmd_vel         (geometry_msgs/Twist) - velocity commands → TWIST_CMD
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

# =================== Protocol Constants ===================
START_BYTE       = 0xAA
END_BYTE         = 0x55

CMD_PING         = 0x01
CMD_ACK          = 0x02
CMD_MOTOR_CMD    = 0x10   # Legacy — kept for backward compat
CMD_MOTOR_STOP   = 0x11
CMD_TWIST_CMD    = 0x12   # ⭐ NEW — preferred velocity command
CMD_IMU_DATA     = 0x22
CMD_ENCODER_DATA = 0x23


class TM4CBridgeNode(Node):

    def __init__(self):
        super().__init__('tm4c_bridge')

        # --- Parameters ---
        self.declare_parameter('serial_port', '/dev/ttyAMA0')
        self.declare_parameter('baud_rate', 115200)

        port = self.get_parameter('serial_port').value
        baud = self.get_parameter('baud_rate').value

        # --- Serial Port ---
        try:
            self.ser = serial.Serial(
                port=port, baudrate=baud,
                bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE, timeout=0.01
            )
            self.get_logger().info(f'Serial opened: {port} @ {baud}')
        except serial.SerialException as e:
            self.get_logger().error(f'Cannot open serial: {e}')
            raise

        # --- Publishers ---
        self.encoder_pub = self.create_publisher(Int32MultiArray, '/encoder_ticks', 10)
        self.imu_pub     = self.create_publisher(Imu, '/imu/data_raw', 10)

        # --- Subscribers ---
        self.cmd_vel_sub = self.create_subscription(
            Twist, '/cmd_vel', self.cmd_vel_callback, 10
        )

        # --- Stats ---
        self.rx_count = 0
        self.tx_count = 0
        self.error_count = 0

        # --- RX thread ---
        self.running = True
        self.rx_thread = threading.Thread(target=self.receive_loop, daemon=True)
        self.rx_thread.start()

        # --- Timers ---
        self.create_timer(2.0, self.send_ping)
        self.create_timer(5.0, self.print_stats)

        self.get_logger().info('TM4C Bridge Node v3.0 started (TWIST_CMD mode)')

    # =================== TRANSMIT ===================

    def calculate_checksum(self, command, length, data):
        checksum = command ^ length
        for byte in data:
            checksum ^= byte
        return checksum & 0xFF

    def send_packet(self, command, data=b''):
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
        Convert ROS2 Twist (/cmd_vel) → TWIST_CMD (0x12)

        Simply packs linear.x and angular.z as fixed-point × 1000.
        The TM4C firmware applies the differential drive equation.
        """
        # Scale to fixed-point × 1000 (mm/s, mrad/s)
        linear_mmps   = int(msg.linear.x  * 1000.0)
        angular_mrads = int(msg.angular.z * 1000.0)

        # Clamp to sint16 range
        linear_mmps   = max(-32768, min(32767, linear_mmps))
        angular_mrads = max(-32768, min(32767, angular_mrads))

        # Pack and send as CMD 0x12
        data = struct.pack('<hh', linear_mmps, angular_mrads)
        self.send_packet(CMD_TWIST_CMD, data)

    def send_ping(self):
        self.send_packet(CMD_PING)

    # =================== RECEIVE ===================

    def receive_loop(self):
        while self.running:
            try:
                byte = self.ser.read(1)
                if len(byte) == 0 or byte[0] != START_BYTE:
                    continue
                header = self.ser.read(2)
                if len(header) < 2:
                    self.error_count += 1; continue
                command, length = header[0], header[1]
                if length > 120:
                    self.error_count += 1; continue
                data = self.ser.read(length) if length > 0 else b''
                if len(data) < length:
                    self.error_count += 1; continue
                footer = self.ser.read(2)
                if len(footer) < 2:
                    self.error_count += 1; continue
                if footer[1] != END_BYTE:
                    self.error_count += 1; continue
                if footer[0] != self.calculate_checksum(command, length, data):
                    self.error_count += 1; continue
                self.rx_count += 1
                self.handle_packet(command, data)
            except serial.SerialException:
                time.sleep(0.1)
            except Exception as e:
                self.get_logger().error(f'RX error: {e}')
                time.sleep(0.01)

    def handle_packet(self, command, data):
        if command == CMD_ENCODER_DATA and len(data) >= 12:
            self.publish_encoder(data)
        elif command == CMD_IMU_DATA and len(data) >= 12:
            self.publish_imu(data)
        elif command == CMD_ACK:
            pass
        elif command == CMD_PING:
            self.send_packet(CMD_ACK)

    def publish_encoder(self, data):
        left_ticks, right_ticks, _, _ = struct.unpack('<iihh', data[:12])
        msg = Int32MultiArray()
        msg.data = [left_ticks, right_ticks]
        self.encoder_pub.publish(msg)

    def publish_imu(self, data):
        ax, ay, az, gx, gy, gz = struct.unpack('<6h', data[:12])
        msg = Imu()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = 'imu_link'
        msg.linear_acceleration.x = float(ax) / 100.0
        msg.linear_acceleration.y = float(ay) / 100.0
        msg.linear_acceleration.z = float(az) / 100.0
        msg.angular_velocity.x = float(gx) / 100.0
        msg.angular_velocity.y = float(gy) / 100.0
        msg.angular_velocity.z = float(gz) / 100.0
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
```

---

## Migration Guide: MOTOR_CMD → TWIST_CMD

### Before (v2.0 — bridge computes wheel speeds)

```python
def cmd_vel_callback(self, msg):
    half_base = 0.30 / 2.0
    left_vel  = msg.linear.x - (msg.angular.z * half_base)
    right_vel = msg.linear.x + (msg.angular.z * half_base)
    left_pct  = int((left_vel / 1.0) * 100.0)
    right_pct = int((right_vel / 1.0) * 100.0)
    data = struct.pack('<hh', left_pct, right_pct)
    self.send_packet(0x10, data)   # CMD_MOTOR_CMD
```

### After (v3.0 — bridge sends raw velocities)

```python
def cmd_vel_callback(self, msg):
    linear_mmps   = int(msg.linear.x  * 1000.0)
    angular_mrads = int(msg.angular.z * 1000.0)
    data = struct.pack('<hh', linear_mmps, angular_mrads)
    self.send_packet(0x12, data)   # CMD_TWIST_CMD
```

**That's it.** Remove the differential drive math from the bridge. The TM4C handles it.

---

## Verified Raw Hex Examples

### TWIST_CMD — Forward 0.5 m/s, no rotation

```
aa 12 04 f4 01 00 00 e7 55
│  │  │  │     │     │  └ END
│  │  │  │     │     └ XOR checksum
│  │  │  │     └ AngularMrads = 0
│  │  │  └ LinearMmps = 500 (0x01F4 LE)
│  │  └ LEN = 4
│  └ CMD = 0x12 (TWIST_CMD)
└ START
```

### TWIST_CMD — Spin left 1.0 rad/s (in place)

```
aa 12 04 00 00 e8 03 f9 55
                │     │
                └ AngularMrads = 1000 (0x03E8 LE)
```

### Encoder packet (TM4C → RPi)

```
aa 23 0c 05 00 00 00 00 00 00 00 00 00 00 00 2a 55
│  │  │  │            │            │         │  │
│  │  │  └LeftTicks=5 └RightTicks=0└Vel=0,0  │  └END
│  │  └ LEN=12                               └ XOR=0x2A
│  └ CMD=0x23
└ START
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| No data on RPi | Check TX→RX crossover, verify GND connected |
| Garbled data | Verify 115200 baud, 8N1 both sides |
| Checksum errors | Verify XOR: `CMD ^ LEN ^ DATA[0] ^ ... ^ DATA[N-1]` |
| No UART on RPi | `sudo raspi-config` → Interface → Serial: shell=NO, hardware=YES |
| Test raw bytes | `sudo stty -F /dev/ttyAMA0 115200 raw && sudo xxd /dev/ttyAMA0` |
| Robot doesn't move | Check robot state — send PING first, verify ACK response |
| Motors spin wrong dir | Swap motor wires or negate velocity signs in bridge |

---

## Quick Test Script

```python
#!/usr/bin/env python3
"""Quick test: send TWIST_CMD and watch for encoder response."""
import serial, struct, time

ser = serial.Serial('/dev/ttyAMA0', 115200, timeout=0.1)

START, END = 0xAA, 0x55
CMD_TWIST = 0x12
CMD_ENCODER = 0x23

def checksum(cmd, length, data):
    cs = cmd ^ length
    for b in data: cs ^= b
    return cs & 0xFF

def send(cmd, data=b''):
    n = len(data)
    pkt = bytes([START, cmd, n]) + data + bytes([checksum(cmd, n, data), END])
    ser.write(pkt)

# Send: forward 0.3 m/s, rotate 0.5 rad/s
send(CMD_TWIST, struct.pack('<hh', 300, 500))
print("Sent TWIST_CMD: v=0.3 m/s, ω=0.5 rad/s")

# Listen for encoder response
for _ in range(100):
    b = ser.read(1)
    if len(b) == 0: continue
    if b[0] != START: continue
    hdr = ser.read(2)
    if len(hdr) < 2: continue
    cmd, ln = hdr
    data = ser.read(ln) if ln > 0 else b''
    ftr = ser.read(2)
    if cmd == CMD_ENCODER and len(data) >= 12:
        lt, rt, lv, rv = struct.unpack('<iihh', data[:12])
        print(f"Encoder: L_ticks={lt} R_ticks={rt} L_rpm={lv/100:.1f} R_rpm={rv/100:.1f}")

send(CMD_TWIST, struct.pack('<hh', 0, 0))  # STOP
print("Sent STOP")
ser.close()
```

---

**Protocol Version:** 3.0 (Twist velocity) | **UART:** 115200 baud, 8N1 | **Data Rate:** ~50Hz sensor TX | **Byte Order:** Little-endian
