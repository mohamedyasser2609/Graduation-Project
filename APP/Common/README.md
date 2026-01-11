# APP Layer - Common Module

**Version:** 1.0.0  
**Date:** January 10, 2026  
**Layer:** APPLICATION

---

## 📁 Contents

| File | Description |
|------|-------------|
| `App_SharedTypes.h` | Inter-task communication types |
| `App_ResourceMap.h` | Resource ownership and mutex API |
| `App_ResourceMap.c` | Resource management implementation |
| `ResourceMap_README.md` | Detailed documentation |

---

## 🎯 Purpose

The Common module provides shared infrastructure:

### App_SharedTypes.h
- `WheelSpeedCmdType` - Comm → Control commands
- `SensorFeedbackType` - Sensor → Comm feedback
- Protocol constants (terminators, command IDs)

### App_ResourceMap
- Hardware resource ownership table
- `Mutex_Encoder` for shared encoder access
- `Queue_SafetyStatus` for fault reporting
- Thread-safe access functions

---

## 🔧 Key APIs

```c
/* Initialize resources */
Std_ReturnType ResourceMap_Init(void);

/* Encoder mutex */
boolean ResourceMap_AcquireEncoder(uint32 TimeoutMs);
void ResourceMap_ReleaseEncoder(void);

/* Safety status queue */
Std_ReturnType ResourceMap_SendSafetyStatus(const SafetyStatusMsgType* Status);
Std_ReturnType ResourceMap_ReceiveSafetyStatus(SafetyStatusMsgType* Status, uint32 TimeoutMs);
```

---

## 🔗 Used By

- **All APP tasks** - For shared types
- `APP/Safety/` - Sends safety status
- `APP/Tasks/` - Uses mutexes and queues
- `SERVICES/RTOS/Tasks_Init.c` - Initializes resources

---

**See `ResourceMap_README.md` for detailed documentation.**
