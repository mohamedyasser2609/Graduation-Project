# ROS 2 UART Communication Protocol

**TM4C123GH6PM ↔ Raspberry Pi 5 (ROS 2)**

---

## Overview

Bidirectional UART between TM4C123 and RPi 5. All sensor data uses **fixed-point integer × 100** (no floating-point on the wire).

- **TM4C sends**: Encoder ticks/velocity + IMU accel/gyro (both as integers)
- **TM4C receives**: Motor speed commands
- **GPS**: Wired directly to RPi (not sent by TM4C)

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

**Checksum** = `CMD ^ LEN ^ DATA[0] ^ DATA[1] ^ ... ^ DATA[N-1]`

All multi-byte values are **little-endian**.

---

## Command Table

| Command | ID | Direction | Data Length | Description |
|---------|------|-----------|-------------|-------------|
| PING | `0x01` | Both | 0 | Heartbeat |
| ACK | `0x02` | Both | 0 | Acknowledgment |
| MOTOR_CMD | `0x10` | RPi → TM4C | 4 | Motor speed control |
| MOTOR_STOP | `0x11` | RPi → TM4C | 0 | Emergency stop |
| IMU_DATA | `0x22` | TM4C → RPi | **12** | IMU accel + gyro (sint16 × 100) |
| ENCODER_DATA | `0x23` | TM4C → RPi | **12** | Encoder ticks + velocity |

---

## Data Format Details

### ENCODER_DATA (0x23) — TM4C → RPi — 12 bytes

```
Byte [0-3]:   Left encoder ticks    (sint32, raw tick count)
Byte [4-7]:   Right encoder ticks   (sint32, raw tick count)
Byte [8-9]:   Left velocity         (sint16, RPM × 100)
Byte [10-11]: Right velocity        (sint16, RPM × 100)
```

**To get real RPM:** divide sint16 by 100.0
```python
left_ticks, right_ticks, left_vel_x100, right_vel_x100 = struct.unpack('<iihh', data)
left_rpm = left_vel_x100 / 100.0   # e.g. 5000 → 50.00 RPM
```

### IMU_DATA (0x22) — TM4C → RPi — 12 bytes

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
ax, ay, az, gx, gy, gz = struct.unpack('<6h', data)
accel_x = ax / 100.0   # e.g. 981 → 9.81 m/s² (1g)
gyro_x  = gx / 100.0   # e.g. 174 → 1.74 rad/s
```

### MOTOR_CMD (0x10) — RPi → TM4C — 4 bytes

```
Byte [0-1]: Left motor speed   (sint16, range: -100 to +100)
Byte [2-3]: Right motor speed  (sint16, range: -100 to +100)
```

Positive = Forward, Negative = Reverse, 0 = Stop.

```python
data = struct.pack('<hh', 50, 30)  # Left=50%, Right=30% forward
```

### MOTOR_STOP (0x11) — RPi → TM4C — 0 bytes

Emergency stop. No data payload.

---

## Verified Raw Hex Example

Captured from TM4C UART on RPi (verified working 2026-04-09):

```
Encoder packet:
aa 23 0c 05 00 00 00 00 00 00 00 00 00 00 00 2a 55
│  │  │  │            │            │         │  │
│  │  │  └LeftTicks=5 └RightTicks=0└Vel=0,0  │  └END
│  │  └ LEN=12                               └ XOR=0x2A
│  └ CMD=0x23
└ START

IMU packet:
aa 22 0c 00 00 00 00 00 00 00 00 00 00 00 00 2e 55
         └── All zeros (IMU not connected) ──┘
```

---

## Complete Python Bridge Implementation

```python
#!/usr/bin/env python3
"""
TM4C ComStack UART Bridge — Receives sensor data, sends motor commands.
"""

import serial
import struct
import time

START_BYTE       = 0xAA
END_BYTE         = 0x55

CMD_PING         = 0x01
CMD_ACK          = 0x02
CMD_MOTOR_CMD    = 0x10
CMD_MOTOR_STOP   = 0x11
CMD_IMU_DATA     = 0x22
CMD_ENCODER_DATA = 0x23

def calculate_checksum(command, length, data):
    checksum = command ^ length
    for byte in data:
        checksum ^= byte
    return checksum & 0xFF

def send_packet(ser, command, data=b''):
    length = len(data)
    checksum = calculate_checksum(command, length, data)
    packet = bytes([START_BYTE, command, length]) + data + bytes([checksum, END_BYTE])
    ser.write(packet)

def receive_packet(ser, timeout=0.1):
    """Returns (command, data) or (None, None)"""
    start_time = time.time()
    while (time.time() - start_time) < timeout:
        byte = ser.read(1)
        if len(byte) == 0:
            continue
        if byte[0] != START_BYTE:
            continue
        header = ser.read(2)
        if len(header) < 2:
            continue
        command, length = header[0], header[1]
        if length > 120:
            continue
        data = ser.read(length) if length > 0 else b''
        if len(data) < length:
            continue
        footer = ser.read(2)
        if len(footer) < 2:
            continue
        if footer[1] != END_BYTE:
            continue
        if footer[0] != calculate_checksum(command, length, data):
            continue
        return command, data
    return None, None

def parse_encoder(data):
    """Parse 12-byte encoder packet → ticks + RPM"""
    lt, rt, lv, rv = struct.unpack('<iihh', data[:12])
    return {'left_ticks': lt, 'right_ticks': rt,
            'left_rpm': lv / 100.0, 'right_rpm': rv / 100.0}

def parse_imu(data):
    """Parse 12-byte IMU packet → m/s² and rad/s"""
    ax, ay, az, gx, gy, gz = struct.unpack('<6h', data[:12])
    return {'accel': (ax/100.0, ay/100.0, az/100.0),
            'gyro':  (gx/100.0, gy/100.0, gz/100.0)}

def send_motor(ser, left_pct, right_pct):
    """Send motor speeds (-100 to +100)"""
    left_pct = max(-100, min(100, left_pct))
    right_pct = max(-100, min(100, right_pct))
    send_packet(ser, CMD_MOTOR_CMD, struct.pack('<hh', left_pct, right_pct))

def send_stop(ser):
    send_packet(ser, CMD_MOTOR_STOP)

# === Example Usage ===
if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyAMA0', 115200, timeout=0.01)
    while True:
        cmd, data = receive_packet(ser)
        if cmd == CMD_ENCODER_DATA:
            enc = parse_encoder(data)
            print(f"Encoder: L={enc['left_ticks']} R={enc['right_ticks']} "
                  f"Lv={enc['left_rpm']:.1f} Rv={enc['right_rpm']:.1f} RPM")
        elif cmd == CMD_IMU_DATA:
            imu = parse_imu(data)
            print(f"IMU: accel={imu['accel']} gyro={imu['gyro']}")
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

---

**Protocol Version:** 2.0 (fixed-point) | **UART:** 115200 baud, 8N1 | **Data Rate:** ~10Hz sensor updates | **Byte Order:** Little-endian
