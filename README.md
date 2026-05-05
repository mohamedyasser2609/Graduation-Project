# UGV Low-Level Controller Firmware

> TM4C123GH6PM + FreeRTOS + ROS2 — AUTOSAR-Inspired Differential Drive Robot Controller

## Overview

This firmware is the **low-level controller** for an autonomous Unmanned Ground Vehicle (UGV). It runs on a **TI TM4C123GH6PM** (ARM Cortex-M4F, 80 MHz) and handles real-time motor control, sensor acquisition, and communication with a **Raspberry Pi 5** running the ROS2 navigation stack.

```
┌──────────────────────────────────────────────────────────┐
│                   Raspberry Pi 5 (ROS2)                  │
│   nav2 · slam_toolbox · teleop · ros2_bridge.py          │
└──────────────────────┬───────────────────────────────────┘
                       │ UART1 (115200 baud, binary ComStack)
                       │ PB0 (RX) ←→ RPi TX
                       │ PB1 (TX) ←→ RPi RX
┌──────────────────────┴───────────────────────────────────┐
│              TM4C123GH6PM (This Firmware)                │
│   FreeRTOS · PID · Kinematics · Safety · Diagnostics     │
└──────────────────────────────────────────────────────────┘
         │              │              │            │
    Motors (PWM)   Encoders (QEI)   IMU (I2C)   Sensors
```

---

## Architecture

The project follows an **AUTOSAR-inspired layered architecture**:

```
┌─────────────────────────────────────────────────┐
│  APP Layer                                      │
│  ├── Tasks/     (FreeRTOS task wrappers)         │
│  ├── Control/   (Robot_Control: PID + kinematics)│
│  ├── Safety/    (SafeState + watchdog)           │
│  └── Common/    (SharedTypes, ResourceMap)        │
├─────────────────────────────────────────────────┤
│  SERVICES Layer                                  │
│  ├── RTOS/      (FreeRTOS config, Tasks_Init)    │
│  ├── COMM/      (ComStack binary protocol)       │
│  ├── PID/       (Generic PID controller)         │
│  ├── DIAG/      (Diagnostics + DTC logging)      │
│  ├── THERMAL/   (ThermalMgmt fan control)        │
│  └── SYSTEM/    (System_Init orchestrator)        │
├─────────────────────────────────────────────────┤
│  ECUAL Layer (Device Drivers)                    │
│  ├── MOTOR/     (Cytron MDD10A dual driver)      │
│  ├── ENCODER/   (EMG49 quadrature encoder)       │
│  ├── IMU/       (MPU-9250 accel+gyro)            │
│  ├── CURRENT_SENSOR/ (ACS712 current sense)      │
│  ├── TEMP_SENSOR/    (AM2320 temperature)        │
│  └── FAN/       (PWM fan control)                │
├─────────────────────────────────────────────────┤
│  MCAL Layer (Hardware Abstraction)               │
│  ├── GPIO/  ├── UART/  ├── PWM/  ├── I2C/       │
│  ├── ADC/   ├── QEI/   ├── WDG/  ├── MCU/       │
│  ├── Timer/ └── MPU/                              │
└─────────────────────────────────────────────────┘
```

---

## FreeRTOS Task Architecture

| Task | Priority | Period | Function |
|------|----------|--------|----------|
| **Safety** | 4 (highest) | 10 ms | Watchdog feed, motor current check, thermal check, heartbeat monitor |
| **Control** | 3 | 10 ms | Read command queue, run PID, update motor PWM |
| **Sensor** | 2 | 20 ms | Read IMU, read encoders, update odometry, publish feedback |
| **Comm** | 1 | 20 ms | Process ComStack RX packets, transmit encoder/IMU/telemetry to RPi |
| **Thermal** | 1 | 1000 ms | Temperature zone monitoring, fan speed control |

**Inter-task communication:**
- `Queue_WheelSpeedCmd` — Comm → Control (motor velocity targets)
- `Queue_SensorFeedback` — Sensor → Comm (encoder/IMU/GPS data)

---

## Hardware

### MCU
- **TI TM4C123GH6PM** — ARM Cortex-M4F, 80 MHz, 256 KB Flash, 32 KB SRAM
- **FPU** enabled (hardware floating point for PID and kinematics)
- **FreeRTOS** v10.x with ARM_CM4F port

### Motor Driver
- **Cytron MDD10A Rev2.0** — Dual-channel, 10A continuous per channel
- Interface: PWM (speed) + GPIO (direction) per channel
- PWM frequency: 20 kHz

### Motors + Encoders
- **EMG49** DC geared motors with built-in encoders
- 245 PPR × 4 (quadrature) = **980 counts/revolution**
- TM4C hardware QEI modules with 50 ms velocity measurement window

### IMU
- **MPU-9250** (MPU-6500 core, WHO_AM_I = `0x70`)
- Connected via I2C0 at 100 kHz
- Accelerometer: ±2g, Gyroscope: ±250°/s (configurable)

### Robot Physical Parameters
| Parameter | Value |
|-----------|-------|
| Wheel radius | 65 mm (0.065 m) |
| Wheel base (track width) | 300 mm (0.30 m) |
| Encoder CPR | 980 counts/rev |
| Max linear velocity | 1.0 m/s |
| Max angular velocity | 3.14 rad/s |
| Max wheel RPM | 100 RPM |

---

## Wiring Guide & Pinout

### Complete TM4C123GH6PM Pin Map

```
                    ┌────────────────────┐
                    │   TM4C123GH6PM     │
                    │   (Tiva C LaunchPad)│
                    └────────────────────┘

  PORT A                          PORT B
  ┌──────────────────────┐        ┌──────────────────────┐
  │ PA0 — UART0 RX (DBG) │        │ PB0 — UART1 RX (ROS) │
  │ PA1 — UART0 TX (DBG) │        │ PB1 — UART1 TX (ROS) │
  │ PA2 — (unused)        │        │ PB2 — I2C0 SCL (IMU) │
  │ PA3 — (unused)        │        │ PB3 — I2C0 SDA (IMU) │
  │ PA4 — (unused)        │        │ PB4 — (unused)        │
  │ PA5 — (unused)        │        │ PB5 — (unused)        │
  │ PA6 — M1PWM2 (L Motor)│        │ PB6 — M0PWM0 (Fan 1)  │
  │ PA7 — M1PWM3 (R Motor)│        │ PB7 — M0PWM1 (Fan 2)  │
  └──────────────────────┘        └──────────────────────┘

  PORT C                          PORT D
  ┌──────────────────────┐        ┌──────────────────────┐
  │ PC0 — JTAG TCK       │        │ PD0 — (unused)        │
  │ PC1 — JTAG TMS       │        │ PD1 — (unused)        │
  │ PC2 — JTAG TDI       │        │ PD2 — (unused)        │
  │ PC3 — JTAG TDO       │        │ PD3 — QEI0 IDX (L Enc)│
  │ PC4 — QEI1 IDX (R Enc)│       │ PD4 — (unused)        │
  │ PC5 — QEI1 PhA (R Enc)│       │ PD5 — (unused)        │
  │ PC6 — QEI1 PhB (R Enc)│       │ PD6 — QEI0 PhA (L Enc)│
  │ PC7 — (unused)        │        │ PD7 — QEI0 PhB (L Enc)│
  └──────────────────────┘        └──────────────────────┘

  PORT E                          PORT F
  ┌──────────────────────┐        ┌──────────────────────┐
  │ PE0 — (GPIO reserved) │        │ PF0 — SW2 (input)     │
  │ PE1 — Left Motor DIR  │        │ PF1 — Red LED (PWM)   │
  │ PE2 — ADC AIN1 (R Curr)│       │ PF2 — Blue LED (PWM)  │
  │ PE3 — ADC AIN0 (L Curr)│       │ PF3 — Green LED (PWM) │
  │ PE4 — Right Motor DIR │        │ PF4 — SW1 (input)     │
  │ PE5 — (unused)        │        └──────────────────────┘
  └──────────────────────┘
```

### Master Hardware Connections Table

| Subsystem | Component | TM4C Pin | Function / Notes |
|-----------|-----------|----------|------------------|
| **Motors** | Left Motor PWM | **PA6** | M1PWM2 (20kHz PWM) |
| **Motors** | Left Motor DIR | **PE1** | GPIO Out |
| **Motors** | Right Motor PWM | **PA7** | M1PWM3 (20kHz PWM) |
| **Motors** | Right Motor DIR | **PE4** | GPIO Out **(Moved from PE2)** |
| **Encoders** | Left Encoder PhA | **PD6** | QEI0 PhA |
| **Encoders** | Left Encoder PhB | **PD7** | QEI0 PhB (Locked NMI pin, auto-unlocked) |
| **Encoders** | Left Encoder IDX | **PD3** | QEI0 IDX (Optional index pulse) |
| **Encoders** | Right Encoder PhA | **PC5** | QEI1 PhA |
| **Encoders** | Right Encoder PhB | **PC6** | QEI1 PhB |
| **Encoders** | Right Encoder IDX | **PC4** | QEI1 IDX (Optional index pulse) |
| **Safety** | L Motor Current (ACS712) | **PE3** | ADC0 AIN0 (Analog Input) |
| **Safety** | R Motor Current (ACS712) | **PE2** | ADC1 AIN1 (Analog Input) |
| **Cooling** | Main Fan (PWM) | **PB6** | M0PWM0 (Intake cooling fan) |
| **Cooling** | Exhaust Fan (PWM) | **PB7** | M0PWM1 (Exhaust cooling fan) |
| **Sensors** | IMU (MPU-9250) SCL | **PB2** | I2C0 SCL (100kHz clock) |
| **Sensors** | IMU (MPU-9250) SDA | **PB3** | I2C0 SDA (Pull-up required) |
| **Sensors** | Temp Sensors (AM2320) | **PB2/PB3** | I2C0 (Shares bus with IMU) |
| **Comms** | ROS2 Bridge RX | **PB0** | UART1 RX (Connect to RPi TX) |
| **Comms** | ROS2 Bridge TX | **PB1** | UART1 TX (Connect to RPi RX) |
| **Debug** | PC Serial RX | **PA0** | UART0 RX (USB Virtual COM) |
| **Debug** | PC Serial TX | **PA1** | UART0 TX (USB Virtual COM) |

### Wiring Diagrams

#### Motor Driver (Cytron MDD10A Rev2.0)

```
  TM4C                    Cytron MDD10A
  ─────                   ─────────────
  PA6 (M1PWM2) ────────── CH1 PWM  (Left Motor)
  PE1 (GPIO)   ────────── CH1 DIR  (Left Motor)
  PA7 (M1PWM3) ────────── CH2 PWM  (Right Motor)
  PE4 (GPIO)   ────────── CH2 DIR  (Right Motor)
  GND          ────────── GND
                          VIN ←── Battery (12V)
                          M1A/M1B ←── Left Motor
                          M2A/M2B ←── Right Motor
```

#### IMU (MPU-9250)

```
  TM4C                    MPU-9250
  ─────                   ────────
  PB2 (I2C0 SCL) ──┬───── SCL
  PB3 (I2C0 SDA) ──┼───── SDA
  3.3V           ──┼───── VCC
  GND            ──┼───── GND
                   │      AD0 ───── GND (address 0x68)
                   │
              4.7kΩ pull-up resistors
              (or use internal pull-ups)
```

> **Device ID:** WHO_AM_I = `0x70` (MPU-6500 core inside MPU-9250 package)

#### Encoders (EMG49)

```
  TM4C                    Left Encoder (EMG49)
  ─────                   ────────────────────
  PD6 (QEI0 PhA) ──────── Channel A (Yellow)
  PD7 (QEI0 PhB) ──────── Channel B (Green)
  PD3 (QEI0 IDX) ──────── Index (if available)
  3.3V / 5V      ──────── VCC (Red)
  GND            ──────── GND (Black)

  TM4C                    Right Encoder (EMG49)
  ─────                   ─────────────────────
  PC5 (QEI1 PhA) ──────── Channel A (Yellow)
  PC6 (QEI1 PhB) ──────── Channel B (Green)
  PC4 (QEI1 IDX) ──────── Index (if available)
  3.3V / 5V      ──────── VCC (Red)
  GND            ──────── GND (Black)
```

> **Note:** QEI0 has `SwapChannels = TRUE` in config. If a wheel reports backwards direction, toggle `ReverseDirection` in `ENCODER_PBCfg.c`.

> **Important:** PD7 is a locked NMI pin on TM4C123. The GPIO driver unlocks it automatically during initialization.

#### Raspberry Pi 5 (UART1 — ROS2 Bridge)

```
  TM4C                    Raspberry Pi 5
  ─────                   ──────────────
  PB0 (UART1 RX) ──────── GPIO 14 (TXD) 
  PB1 (UART1 TX) ──────── GPIO 15 (RXD) 
  GND            ──────── GND            

  ⚠ IMPORTANT: TM4C is 3.3V tolerant. RPi GPIO is also 3.3V.
     Direct connection is safe — NO level shifter needed.
     
  ⚠ CRITICAL: Disable Bluetooth on RPi to free /dev/ttyAMA0:
     Add to /boot/firmware/config.txt:
       dtoverlay=disable-bt
     Then use /dev/ttyAMA0 in the ROS2 bridge script.
```

#### Debug UART (UART0)

```
  TM4C                    PC / USB-Serial
  ─────                   ────────────────
  PA0 (UART0 RX) ──────── TX (or built-in LaunchPad USB)
  PA1 (UART0 TX) ──────── RX (or built-in LaunchPad USB)
  
  Settings: 115200 baud, 8N1, No flow control
```

---

## Communication Protocol (ComStack v3.0)

Binary packet format over UART1 at 115200 baud:

```
┌──────┬─────────┬────────┬──────────────┬──────────┐
│ 0xAA │ Command │ Length │ Data[0..N-1] │ Checksum │
│ SYNC │  (1B)   │  (1B)  │   (N bytes)  │   XOR    │
└──────┴─────────┴────────┴──────────────┴──────────┘
```

### Command Table

| CMD | Hex | Direction | Data | Description |
|-----|-----|-----------|------|-------------|
| Ping | `0x01` | RPi → TM4C | 0 bytes | Heartbeat ping |
| ACK | `0x02` | TM4C → RPi | 0 bytes | Acknowledge response |
| Motor CMD | `0x10` | RPi → TM4C | 4B: L_speed(s16) + R_speed(s16) | Direct motor % (-100..100) |
| Twist CMD | `0x12` | RPi → TM4C | 4B: linear_mm/s(s16) + angular_mrad/s(s16) | ROS cmd_vel |
| Motor Stop | `0x11` | RPi → TM4C | 0 bytes | Emergency stop |
| Encoder Data | `0x23` | TM4C → RPi | 8B: L_ticks(s32) + R_ticks(s32) | Encoder feedback (50Hz) |
| IMU Data | `0x24` | TM4C → RPi | 12B: ax,ay,az,gx,gy,gz (s16 ×1000) | IMU feedback (50Hz) |
| Telemetry | `0x30` | TM4C → RPi | Variable | Status/battery/current (1Hz) |

All multi-byte values are **little-endian**. Fixed-point values are scaled ×1000 (mm/s, mrad/s, milli-g, milli-°/s).

---

## Feature Flags

Hardware features can be enabled/disabled at compile time in `CONFIG/System_FeatureFlags.h`:

| Flag | Default | Description |
|------|---------|-------------|
| `ROBOT_IMU_ENABLED` | `STD_ON` | IMU reads in Sensor Task |
| `FEATURE_GPS_ENABLED` | `0` | GPS module (routed to RPi instead) |
| `FEATURE_FAN_ENABLED` | `0` | PWM fan control |
| `FEATURE_TEMP_ENABLED` | `0` | AM2320 temperature sensors |
| `FEATURE_CURRENT_ENABLED` | `0` | ACS712 current sensors |

> Set fans/temp/current to `(1u)` when the hardware is physically connected.

---

## Build & Flash

### Requirements
- **TI Code Composer Studio (CCS)** v12+
- **TivaWare** (for device headers)
- **TI ARM Compiler** v20+ or GCC ARM
- **FreeRTOS** (included in `FreeRTOS/` directory)
- **Portable port:** `CCS/ARM_CM4F` (Cortex-M4 with FPU)

### Steps
1. Open CCS and import the project (`File → Import → CCS Projects`)
2. Ensure `main_robot.c` has `#if 1` (enabled) and all test mains have `#if 0`
3. Build: `Project → Build Project` (Ctrl+B)
4. Connect TM4C LaunchPad via USB
5. Flash: `Run → Debug` (F11), then `Resume` (F8)

### Entry Points
| File | Purpose |
|------|---------|
| `main_robot.c` | **Production** — FreeRTOS scheduler with all tasks |
| `main_slam_test.c` | SLAM testing bare-metal super-loop |
| `testing/main_imu_test.c` | IMU standalone test |
| `testing/main_qei_test.c` | Encoder standalone test |
| `testing/main_i2c_debug.c` | I2C bus scanner |

> **Only one** `main()` can be active. Set `#if 1` / `#if 0` at the top of each file.

---

## PID Tuning

Default PID gains (in `Robot_Control.c`):

| Parameter | Value | Notes |
|-----------|-------|-------|
| Kp | 2.0 | Proportional gain |
| Ki | 0.5 | Integral gain |
| Kd | 0.01 | Derivative gain |
| Sample time | 10 ms | 100 Hz control loop |
| Output range | -100..+100 | Maps to motor duty cycle % |
| Integrator limits | -50..+50 | Anti-windup clamp |
| Deadband kick | 15% | Minimum PWM to overcome static friction |

Each wheel has an **independent PID loop** with encoder feedback. Motor speed mismatches (e.g. one motor 15% faster) are automatically compensated.

---

## Project Structure

```
Graduation_Project/
├── main_robot.c              # Production entry point (FreeRTOS)
├── APP/
│   ├── Tasks/                # FreeRTOS task wrappers
│   │   ├── App_ControlTask.c   # PID proxy → Robot_Control
│   │   ├── App_SensorTask.c    # IMU + Encoder reads + Odometry
│   │   └── App_commTask.c      # ComStack RX/TX ↔ ROS2
│   ├── Control/
│   │   ├── Robot_Control.c     # Diff-drive kinematics + PID
│   │   └── Robot_Control.h
│   ├── Safety/
│   │   ├── App_SafteyTask.c    # Health checks + WDG
│   │   └── App_SafeState.c     # Fault manager + motor gate
│   └── Common/
│       ├── App_SharedTypes.h   # Inter-task data structures
│       └── App_ResourceMap.c   # Mutex-protected resource access
├── SERVICES/
│   ├── RTOS/
│   │   ├── Tasks_Init.c        # Task creation + scheduler start
│   │   └── FreeRTOSConfig.h    # Kernel configuration
│   ├── COMM/
│   │   └── ComStack.c          # Binary UART protocol
│   ├── PID/
│   │   └── PID.c               # Generic PID with filtering
│   ├── DIAG/
│   │   └── Diagnostics.c       # Event logging + DTC
│   ├── THERMAL/
│   │   └── ThermalMgmt.c       # Fan + zone management
│   └── SYSTEM/
│       └── System_Init.c       # Boot-time init orchestrator
├── ECUAL/
│   ├── MOTOR/    # Cytron MDD10A driver
│   ├── ENCODER/  # EMG49 QEI wrapper
│   ├── IMU/      # MPU-9250 driver
│   ├── CURRENT_SENSOR/  # ACS712
│   ├── TEMP_SENSOR/     # AM2320
│   └── FAN/             # PWM fan
├── MCAL/
│   ├── GPIO/ ├── UART/ ├── PWM/ ├── I2C/
│   ├── ADC/  ├── QEI/  ├── WDG/ ├── MCU/
│   ├── Timer/ └── MPU/
├── CONFIG/
│   ├── Std_Types.h
│   └── System_FeatureFlags.h
├── FreeRTOS/               # FreeRTOS kernel source
├── ros2/                   # ROS2 bridge scripts (Python)
└── testing/                # Standalone test applications
```

---

## Author

**Mohamed Yasser** — Graduation Project 2025/2026
