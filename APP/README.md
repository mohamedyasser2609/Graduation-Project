# APP Layer Documentation

**Version:** 1.0.0  
**Date:** January 10, 2026  
**Layer:** APPLICATION

---

## 📋 Overview

The APP (Application) layer contains the high-level robot control logic, organized into four modules by responsibility.

---

## 📁 Folder Structure

```
APP/
├── Safety/          # Safety monitoring and privilege control
│   ├── App_SafeState.h/c    # Motor enable gate, fault management
│   ├── App_SafteyTask.c     # WDG feeding, current monitoring
│   └── README.md
│
├── Tasks/           # FreeRTOS task implementations
│   ├── App_ControlTask.c    # PID motor control
│   ├── App_SensorTask.c     # IMU/GPS/Encoder reading
│   ├── App_commTask.c       # ROS UART communication
│   ├── App_ThermalTask.c    # Temperature/fan control
│   └── README.md
│
├── Control/         # Robot kinematics and motion control
│   ├── Robot_Control.h/c    # Differential drive, odometry
│   └── README.md
│
├── Common/          # Shared types and resource management
│   ├── App_SharedTypes.h    # Inter-task data structures
│   ├── App_ResourceMap.h/c  # Mutex and queue management
│   └── README.md
│
└── App_Main.c       # Entry point placeholder
```

---

## 🎯 Module Responsibilities

| Module | Responsibility |
|--------|----------------|
| **Safety** | Motor enable privilege, fault handling, WDG |
| **Tasks** | FreeRTOS task implementations |
| **Control** | Differential drive kinematics, odometry |
| **Common** | Shared types, mutexes, queues |

---

## 🔗 Dependencies

```
           ┌────────────────────────────────────────┐
           │            SERVICES Layer             │
           │  RTOS, PID, Thermal, Comm, Diag       │
           └────────────────┬───────────────────────┘
                            │
           ┌────────────────▼───────────────────────┐
           │              APP Layer                │
           ├────────────────────────────────────────┤
           │  Common ◄── Tasks ──► Safety          │
           │              │                        │
           │              ▼                        │
           │           Control                     │
           └────────────────┬───────────────────────┘
                            │
           ┌────────────────▼───────────────────────┐
           │             ECUAL Layer               │
           │  Motor, Encoder, IMU, GPS, Sensors    │
           └────────────────────────────────────────┘
```

---

## 🚀 Task Priority Hierarchy

| Priority | Task | Module | Period |
|:--------:|------|--------|:------:|
| 4 | Safety | APP/Safety/ | 10ms |
| 3 | Control | APP/Tasks/ | 10ms |
| 2 | Sensor | APP/Tasks/ | 20ms |
| 1 | Comm | APP/Tasks/ | 20ms |
| 1 | Thermal | APP/Tasks/ | 1000ms |

---

**See individual folder README files for details.**
