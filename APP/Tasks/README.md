# APP Layer - Tasks Module

**Version:** 1.0.0  
**Date:** January 10, 2026  
**Layer:** APPLICATION

---

## 📁 Contents

| File | Description |
|------|-------------|
| `App_ControlTask.c` | Motor PID control task (Priority 3) |
| `App_SensorTask.c` | Sensor reading task (Priority 2) |
| `App_commTask.c` | ROS communication task (Priority 1) |
| `App_ThermalTask.c` | Thermal management task (Priority 1) |

---

## 🎯 Purpose

FreeRTOS task implementations for the robot controller:

### Control Task (10ms, Priority 3)
- Consumes wheel speed commands from Comm
- Runs PID control loop
- Commands motor via PWM
- Checks `SafeState_IsMotorEnableAllowed()` before commanding

### Sensor Task (20ms, Priority 2)
- Reads IMU, GPS, Encoders
- Publishes feedback to Comm via queue
- Calculates odometry

### Comm Task (20ms, Priority 1)
- Parses ROS wheel commands (`W,left,right\n`)
- Transmits sensor feedback (`F,lTicks,rTicks,yaw,lat,lon\n`)
- Reports faults from Safety queue

### Thermal Task (1000ms, Priority 1)
- Monitors enclosure temperatures
- Controls cooling fans
- Triggers thermal shutdown if needed

---

## 🔗 Dependencies

**Uses:**
- `APP/Safety/` - SafeState checks
- `APP/Common/` - Shared types, resource map
- `APP/Control/` - Robot_Control for kinematics
- `ECUAL/` - Motor, Encoder, IMU, GPS
- `SERVICES/RTOS/` - Queue handles

---

**See `SERVICES/RTOS/RTOS_README.md` for task architecture.**
