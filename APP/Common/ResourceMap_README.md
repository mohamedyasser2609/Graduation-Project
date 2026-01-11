# Resource Map Documentation

**Version:** 1.0.0  
**Date:** January 9, 2026  
**Module:** APP - Resource Management  
**Layer:** Application

---

## 📋 Overview

The Resource Map (`App_ResourceMap`) documents hardware resource ownership and provides thread-safe access mechanisms. It prevents resource conflicts and enables fault reporting between tasks.

### **Key Features**

✅ **Ownership Documentation** - Clear resource-to-task mapping  
✅ **Mutex Protection** - Thread-safe encoder access  
✅ **Safety Status Queue** - Fault reporting from Safety to Comm  
✅ **I2C Bus Mutex** - Shared I2C bus support  

---

## 🏗️ Resource Ownership Table

| Resource | Owner Task | Access Type | Bus |
|----------|------------|-------------|-----|
| Motor Left PWM | Control | Write (gated) | PWM0 |
| Motor Right PWM | Control | Write (gated) | PWM0 |
| Encoder Left | **Shared** | Read-only | QEI0 |
| Encoder Right | **Shared** | Read-only | QEI1 |
| IMU (MPU-9250) | Sensor | R/W Exclusive | I2C0 |
| GPS (NEO-M8N) | Sensor | Read Exclusive | UART0 |
| Current Left | Safety | Read Exclusive | ADC0 |
| Current Right | Safety | Read Exclusive | ADC1 |
| Temp Sensors | Thermal | Read Exclusive | I2C1 |
| Fans | Thermal | Write Exclusive | PWM1 |
| Watchdog | Safety | Write (MPU) | WDT0 |
| ROS UART | Comm | R/W Exclusive | UART1 |

---

## 📁 File Structure

| File | Description |
|------|-------------|
| `App_ResourceMap.h` | Ownership table, mutex/queue declarations |
| `App_ResourceMap.c` | Mutex and queue implementation |

---

## 🚀 Quick Start

### 1. Initialize (Before Task Creation)

```c
#include "App_ResourceMap.h"

void Tasks_Init(void) {
    /* Initialize resources FIRST */
    if (ResourceMap_Init() != E_OK) {
        /* Handle error */
    }
    
    /* Then create tasks */
    Tasks_CreateAll();
    vTaskStartScheduler();
}
```

### 2. Acquire Encoder with Mutex

```c
void App_ControlTask_Run(void) {
    Encoder_DataType encoderLeft, encoderRight;
    
    /* Acquire mutex with 10ms timeout */
    if (ResourceMap_AcquireEncoder(10u)) {
        Encoder_GetData(ENCODER_CHANNEL_LEFT, &encoderLeft);
        Encoder_GetData(ENCODER_CHANNEL_RIGHT, &encoderRight);
        ResourceMap_ReleaseEncoder();
    }
    /* ... use encoder data ... */
}
```

### 3. Send Safety Status

```c
void App_SafetyTask_Run(void) {
    SafetyStatusMsgType statusMsg;
    
    statusMsg.FaultFlags = SafeState_GetFaultFlags();
    statusMsg.Timestamp = xTaskGetTickCount();
    statusMsg.MotorEnabled = SafeState_IsMotorEnableAllowed();
    
    ResourceMap_SendSafetyStatus(&statusMsg);
}
```

### 4. Receive Faults in Comm Task

```c
void App_CommTask_Run(void) {
    SafetyStatusMsgType safetyStatus;
    
    if (ResourceMap_ReceiveSafetyStatus(&safetyStatus, 0u) == E_OK) {
        if (safetyStatus.FaultFlags != 0u) {
            /* Send fault packet to ROS */
            SendFaultPacket(&safetyStatus);
        }
    }
}
```

---

## 🔧 API Reference

### Initialization
```c
Std_ReturnType ResourceMap_Init(void);
```

### Encoder Mutex
```c
boolean ResourceMap_AcquireEncoder(uint32 TimeoutMs);
void ResourceMap_ReleaseEncoder(void);
```

### Ownership Check
```c
boolean ResourceMap_IsOwner(ResourceType Resource);
```

### Safety Status Queue
```c
Std_ReturnType ResourceMap_SendSafetyStatus(const SafetyStatusMsgType* Status);
Std_ReturnType ResourceMap_ReceiveSafetyStatus(SafetyStatusMsgType* Status, 
                                                 uint32 TimeoutMs);
```

---

## 📡 Inter-Task Communication

### Queue Summary

| Queue | Direction | Data Type | Depth |
|-------|-----------|-----------|:-----:|
| Queue_WheelSpeedCmd | Comm → Control | WheelSpeedCmdType | 4 |
| Queue_SensorFeedback | Sensor → Comm | SensorFeedbackType | 4 |
| **Queue_SafetyStatus** | Safety → Comm | SafetyStatusMsgType | 2 |

### SafetyStatusMsgType

```c
typedef struct {
    uint32  FaultFlags;         /* Active fault bitmap */
    uint32  Timestamp;          /* Fault detection time */
    uint8   SafeStateStatus;    /* NORMAL/WARNING/ACTIVE/LOCKOUT */
    uint8   LastFaultReason;    /* Last fault reason code */
    boolean MotorEnabled;       /* Motor enable status */
    boolean RequiresAck;        /* Fault requires acknowledgement */
} SafetyStatusMsgType;
```

---

## 🔒 Mutex Details

### Mutex_Encoder

**Purpose:** Protect encoder read operations

**Shared Between:** Sensor Task, Control Task

```c
/* Example: Control Task */
if (ResourceMap_AcquireEncoder(10u)) {
    /* Safe to read encoders */
    Encoder_GetData(0, &enc0);
    Encoder_GetData(1, &enc1);
    ResourceMap_ReleaseEncoder();
}
```

### Mutex_I2C0

**Purpose:** Protect I2C bus access (if shared)

**Currently:** IMU only uses I2C0, but mutex available for future expansion

---

## 🛡️ Ownership Enforcement

The `ResourceMap_IsOwner()` function checks if the calling task owns a resource:

```c
boolean ResourceMap_IsOwner(ResourceType Resource) {
    /* Get current task name */
    const char* taskName = pcTaskGetName(xTaskGetCurrentTaskHandle());
    
    /* Check against ownership table */
    switch (ResourceOwners[Resource]) {
        case RESOURCE_OWNER_SAFETY:
            return (taskName[0] == 'S' && taskName[1] == 'a');
        case RESOURCE_OWNER_CONTROL:
            return (taskName[0] == 'C' && taskName[1] == 'o');
        /* ... */
    }
}
```

---

## 💡 Usage Examples

### Example: Bus Conflict Avoidance

```
I2C0 Bus:
├── IMU (Sensor Task) - Exclusive owner
└── No conflicts

I2C1 Bus:
├── AM2320 Sensor 0 (Thermal Task)
├── AM2320 Sensor 1 (Thermal Task)
└── AM2320 Sensor 2 (Thermal Task) - All same owner, no conflict

UART0:
├── GPS RX (Sensor Task) 
└── Debug TX (Diag) - Time-sliced, low priority
```

### Example: Fault Flow

```
Safety Task detects overload
        │
        ▼
SafetyStatusMsgType created
        │
        ▼
ResourceMap_SendSafetyStatus()
        │
        ▼
Queue_SafetyStatus (xQueueOverwrite)
        │
        ▼
Comm Task receives
        │
        ▼
Send to ROS: "E,0x0001,1234567890\n"
```

---

## ⚠️ Important Notes

### Queue Overwrite Behavior

`Queue_SafetyStatus` uses `xQueueOverwrite()`:
- Always succeeds (never blocks)
- Overwrites oldest item if queue full
- Ensures latest status is always available

### Mutex Timeout

Always use reasonable timeouts:

```c
/* GOOD - 10ms timeout */
if (ResourceMap_AcquireEncoder(10u)) { ... }

/* BAD - infinite wait (can cause deadlock) */
if (ResourceMap_AcquireEncoder(portMAX_DELAY)) { ... }
```

---

## 🐛 Troubleshooting

### ResourceMap_Init() Fails

**Cause:** Insufficient FreeRTOS heap

**Solution:** Increase `configTOTAL_HEAP_SIZE` in FreeRTOSConfig.h

### Mutex Timeout

**Symptom:** `ResourceMap_AcquireEncoder()` returns FALSE

**Checklist:**
- ✅ Other task not holding mutex too long
- ✅ No deadlock between multiple mutexes
- ✅ Timeout is reasonable (>= 10ms)

### Safety Status Not Received

**Checklist:**
- ✅ Safety Task calling `ResourceMap_SendSafetyStatus()`
- ✅ Comm Task calling `ResourceMap_ReceiveSafetyStatus()`
- ✅ Queue created successfully

---

## 📚 Related Modules

- **App_SafeState** - Generates safety status
- **App_SafetyTask** - Sends safety status
- **App_CommTask** - Receives and reports status
- **Tasks_Init** - Calls ResourceMap_Init()

---

**Your task resources are now properly managed!** 📊🔄
