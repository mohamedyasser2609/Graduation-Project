# MPU-9250 IMU Test Program

**9-Axis Motion Tracking: Accelerometer + Gyroscope + Magnetometer**

---

## 🎯 Overview

The MPU-9250 is a 9-axis MotionTracking device combining:
- **3-axis Gyroscope** - Measures angular velocity (rotation)
- **3-axis Accelerometer** - Measures linear acceleration (gravity, movement)
- **3-axis Magnetometer** - Measures magnetic field (compass heading)

Perfect for:
- Robot orientation and navigation
- Tilt sensing and leveling
- Motion detection
- Gesture recognition
- Drone stabilization

---

## 📊 Specifications

### **Gyroscope:**
- **Range:** ±250, ±500, ±1000, ±2000 °/s (configurable)
- **Resolution:** 16-bit ADC
- **Current setting:** ±250 °/s

### **Accelerometer:**
- **Range:** ±2g, ±4g, ±8g, ±16g (configurable)
- **Resolution:** 16-bit ADC
- **Current setting:** ±2g

### **Magnetometer (AK8963):**
- **Range:** ±4800 μT
- **Resolution:** 16-bit ADC

### **Temperature Sensor:**
- **Range:** -40°C to +85°C
- **Accuracy:** ±1°C

---

## 🔌 Hardware Connections

### **TM4C123 ↔ MPU-9250**

| TM4C Pin | Function | MPU-9250 Pin | Notes |
|----------|----------|--------------|-------|
| **PB2** | I2C0 SCL | **SCL** | Clock line |
| **PB3** | I2C0 SDA | **SDA** | Data line |
| **3.3V** | Power | **VCC** | 3.3V or 5V |
| **GND** | Ground | **GND** | Common ground |

**⚠️ IMPORTANT:**
- **Pull-up resistors** (4.7kΩ) required on SCL and SDA lines
- Most MPU-9250 modules have built-in pull-ups ✅
- I2C address: **0x68** (default) or **0x69** if AD0 pin is high

### **MPU-9250 Module Pinout:**

```
┌─────────────────┐
│   MPU-9250      │
├─────────────────┤
│ VCC  → 3.3V     │
│ GND  → GND      │
│ SCL  → PB2      │
│ SDA  → PB3      │
│ AD0  → (leave)  │  ← I2C address select
│ INT  → (opt)    │  ← Interrupt (optional)
└─────────────────┘
```

---

## 🚀 Quick Start

### **Step 1: Wire the Hardware**

1. Connect MPU-9250 to TM4C as shown above
2. Verify 3.3V power supply
3. Check GND connection

### **Step 2: Build and Flash**

```
1. Project → Clean
2. Project → Build
3. Run → Debug → Resume
```

### **Step 3: Open Serial Monitor**

- **Baud Rate:** 115200
- **Data Bits:** 8
- **Parity:** None
- **Stop Bits:** 1

---

## 📺 Expected Output

### **Startup:**

```
========================================
  MPU-9250 IMU Test - TM4C123GH6PM    
========================================
I2C0: PB2 (SCL), PB3 (SDA)
Debug UART: UART0 @ 115200 baud

Initializing MPU-9250...
WHO_AM_I: 0x71
[OK] MPU-9250 initialized successfully!

Reading IMU data every 1 second...
```

### **Continuous Data (Every 1 Second):**

```
========================================
MPU-9250 IMU Data
========================================
Accelerometer (g):
  X: 0.02 g
  Y: -0.01 g
  Z: 1.00 g

Gyroscope (°/s):
  X: 0.15 °/s
  Y: -0.32 °/s
  Z: 0.08 °/s

Temperature: 24.50 °C
========================================
```

---

## 📖 Understanding the Data

### **Accelerometer (g-force):**

| Axis | Stationary | Meaning |
|------|------------|---------|
| **X** | ~0.0 g | Horizontal tilt |
| **Y** | ~0.0 g | Horizontal tilt |
| **Z** | ~1.0 g | Gravity (pointing up) |

**When flat on table:**
- Z should read ~1.0g (gravity)
- X and Y should read ~0.0g

**Tilt the board:**
- Values change as gravity vector rotates
- Total magnitude: √(X² + Y² + Z²) ≈ 1.0g

### **Gyroscope (degrees/second):**

| Axis | Stationary | Rotating |
|------|------------|----------|
| **X** | ~0 °/s | Pitch rotation |
| **Y** | ~0 °/s | Roll rotation |
| **Z** | ~0 °/s | Yaw rotation |

**When stationary:**
- All axes should read close to 0 °/s
- Small drift is normal (±1 °/s)

**Rotate the board:**
- Values show rotation speed
- Positive/negative indicates direction

### **Temperature:**
- Chip internal temperature
- Useful for temperature compensation
- Typically 20-30°C at room temperature

---

## 🔧 Troubleshooting

### **Problem: "Failed to read WHO_AM_I"**

**Possible causes:**
1. **I2C not connected** - Check SCL/SDA wiring
2. **Wrong I2C address** - Try 0x69 if AD0 is high
3. **No power** - Check VCC and GND
4. **No pull-ups** - Add 4.7kΩ resistors to SCL/SDA

**Debug steps:**
```c
/* In main_imu.c, try alternate address: */
#define MPU9250_I2C_ADDR  0x69  /* Instead of 0x68 */
```

### **Problem: "Invalid device ID"**

**WHO_AM_I values:**
- **0x71** = MPU-9250 ✅
- **0x73** = MPU-9255 ✅
- **0x68** = MPU-6050 (no magnetometer)
- **Other** = Wrong device or communication error

### **Problem: All readings are zero**

**Check:**
1. Sensor initialized successfully?
2. I2C communication working?
3. Correct register addresses?
4. Power supply stable?

### **Problem: Noisy/erratic readings**

**Solutions:**
1. **Add capacitor** (0.1μF) near VCC/GND
2. **Shorten I2C wires** (< 20cm recommended)
3. **Lower I2C speed** (already at 100kHz)
4. **Enable low-pass filter** in sensor config

---

## 🎓 Calibration

### **Accelerometer Calibration:**

The accelerometer should read:
- **Z-axis:** ~1.0g when flat (gravity)
- **X/Y-axis:** ~0.0g when level

**If readings are off:**
```c
/* Add offset correction in code */
imuData.accelX_g -= 0.05;  /* Example: -0.05g offset */
imuData.accelY_g += 0.02;  /* Example: +0.02g offset */
```

### **Gyroscope Calibration:**

Gyroscope has drift when stationary. To calibrate:

1. **Collect bias samples** (100 readings while stationary)
2. **Calculate average** (this is your bias)
3. **Subtract bias** from all readings

```c
/* Example bias correction */
float gyro_bias_x = 0.5;  /* Measured bias */
imuData.gyroX_dps -= gyro_bias_x;
```

---

## 🔬 Advanced Features

### **1. Change Accelerometer Range:**

```c
/* In MPU9250_Init() function */
/* ±2g  */ MPU9250_WriteRegister(MPU9250_ACCEL_CONFIG, 0x00);
/* ±4g  */ MPU9250_WriteRegister(MPU9250_ACCEL_CONFIG, 0x08);
/* ±8g  */ MPU9250_WriteRegister(MPU9250_ACCEL_CONFIG, 0x10);
/* ±16g */ MPU9250_WriteRegister(MPU9250_ACCEL_CONFIG, 0x18);

/* Update scale factor accordingly */
#define ACCEL_SCALE  ACCEL_SCALE_4G  /* Change to match */
```

### **2. Change Gyroscope Range:**

```c
/* In MPU9250_Init() function */
/* ±250°/s  */ MPU9250_WriteRegister(MPU9250_GYRO_CONFIG, 0x00);
/* ±500°/s  */ MPU9250_WriteRegister(MPU9250_GYRO_CONFIG, 0x08);
/* ±1000°/s */ MPU9250_WriteRegister(MPU9250_GYRO_CONFIG, 0x10);
/* ±2000°/s */ MPU9250_WriteRegister(MPU9250_GYRO_CONFIG, 0x18);

/* Update scale factor accordingly */
#define GYRO_SCALE  GYRO_SCALE_500DPS  /* Change to match */
```

### **3. Enable Low-Pass Filter:**

```c
/* In MPU9250_Init() function */
/* DLPF_CFG = 3: Bandwidth 44Hz, Delay 4.9ms */
MPU9250_WriteRegister(MPU9250_CONFIG, 0x03);
```

### **4. Adjust Sample Rate:**

```c
/* Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV) */
/* For 100Hz: SMPLRT_DIV = 9 (assuming 1kHz gyro rate) */
MPU9250_WriteRegister(0x19, 9);  /* Register 25: SMPLRT_DIV */
```

---

## 🧮 Useful Calculations

### **Calculate Tilt Angles:**

```c
/* Roll (rotation around X-axis) */
float roll = atan2(accelY, accelZ) * 180.0 / 3.14159;

/* Pitch (rotation around Y-axis) */
float pitch = atan2(-accelX, sqrt(accelY*accelY + accelZ*accelZ)) * 180.0 / 3.14159;
```

### **Integrate Gyroscope for Angle:**

```c
/* Angle = previous_angle + (gyro_rate * dt) */
static float angle_x = 0;
float dt = 0.01;  /* 10ms sample time */

angle_x += gyroX_dps * dt;
```

### **Complementary Filter (Sensor Fusion):**

```c
/* Combine accelerometer and gyroscope for stable angle */
float alpha = 0.98;  /* Weight factor */

angle = alpha * (angle + gyroX_dps * dt) + (1 - alpha) * accel_angle;
```

---

## 📚 Register Map Quick Reference

| Register | Address | Function |
|----------|---------|----------|
| WHO_AM_I | 0x75 | Device ID (0x71) |
| PWR_MGMT_1 | 0x6B | Power management |
| GYRO_CONFIG | 0x1B | Gyro range config |
| ACCEL_CONFIG | 0x1C | Accel range config |
| ACCEL_XOUT_H | 0x3B | Accel X high byte |
| TEMP_OUT_H | 0x41 | Temperature high byte |
| GYRO_XOUT_H | 0x43 | Gyro X high byte |

**Full datasheet:** [MPU-9250 Product Specification](https://invensense.tdk.com/products/motion-tracking/9-axis/mpu-9250/)

---

## 🎯 Next Steps

1. ✅ **Test basic readings** (current program)
2. ✅ **Calibrate sensors** (offset correction)
3. ✅ **Add magnetometer** (compass heading)
4. ✅ **Implement sensor fusion** (Kalman/Complementary filter)
5. ✅ **Calculate orientation** (roll, pitch, yaw)
6. ✅ **Integrate with robot** (stabilization, navigation)

---

## 💡 Application Examples

### **Robot Balancing:**
Use accelerometer tilt + gyroscope rate → PID controller → Motor control

### **Drone Stabilization:**
Sensor fusion (accel + gyro + mag) → Attitude estimation → Flight controller

### **Step Counter:**
Detect acceleration peaks → Count steps → Calculate distance

### **Gesture Recognition:**
Pattern matching on accel/gyro data → Recognize gestures → Control actions

---

**Your MPU-9250 IMU is ready to use! Build, flash, and start reading motion data!** 🚀📊🤖
