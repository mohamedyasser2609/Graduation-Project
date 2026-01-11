# APP Layer - Control Module

**Version:** 1.0.0  
**Date:** January 10, 2026  
**Layer:** APPLICATION

---

## 📁 Contents

| File | Description |
|------|-------------|
| `Robot_Control.h` | Robot control API and types |
| `Robot_Control.c` | Differential drive kinematics and odometry |

---

## 🎯 Purpose

The Control module provides:
- **Differential drive kinematics** - Convert twist commands to wheel speeds
- **Inverse kinematics** - Convert wheel speeds back to robot velocity
- **Odometry calculation** - Track robot position from encoder feedback
- **Physical parameters** - Wheel radius, track width, gear ratio

---

## 🔧 Key Functions

```c
/* Initialize robot control */
void Robot_Init(void);

/* Set velocity command (linear + angular) */
void Robot_SetTwist(float32 linear_x, float32 angular_z);

/* Get current odometry */
Std_ReturnType Robot_GetOdometry(Robot_OdometryType* Odom);

/* Emergency stop */
void Robot_EmergencyStop(void);

/* Resume operation */
Std_ReturnType Robot_Resume(void);
```

---

## 🔗 Dependencies

**Uses:**
- `ECUAL/ENCODER` - Velocity feedback
- `ECUAL/MOTOR` - Motor commands
- `SERVICES/PID` - Velocity control

**Used by:**
- `APP/Tasks/App_ControlTask.c` - Main control loop

---

## 📐 Robot Parameters

```c
#define ROBOT_WHEEL_RADIUS_M    (0.065f)    /* 65mm wheel */
#define ROBOT_TRACK_WIDTH_M     (0.30f)     /* 300mm track */
#define ROBOT_GEAR_RATIO        (30.0f)     /* 30:1 gearbox */
#define ENCODER_CPR             (48.0f)     /* Counts per rev */
```

---

**See source files for implementation details.**
