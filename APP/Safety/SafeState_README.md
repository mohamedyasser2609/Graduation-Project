# Safe State Manager Documentation

**Version:** 1.0.0  
**Date:** January 9, 2026  
**Module:** APP - Safety System  
**Layer:** Application

---

## 📋 Overview

The Safe State Manager (`App_SafeState`) implements the software privilege separation model for the UGV controller. It controls motor enable/disable privileges and manages fault states to ensure system safety.

### **Key Features**

✅ **Motor Enable Gate** - Control Task must request permission  
✅ **Fault Management** - Track, escalate, and recover from faults  
✅ **Privilege Model** - Only Safety Task can enable motors  
✅ **Cooldown Protection** - Prevents rapid fault recovery  
✅ **Lockout Mode** - Requires power cycle after repeated faults  

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    SAFE STATE MODEL                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  NORMAL ──fault──► WARNING ──critical──► ACTIVE ──10x──► LOCKOUT│
│     ▲                                       │                   │
│     └──────────── clear + cooldown ─────────┘                   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### State Descriptions

| State | Motor Enable | Description |
|-------|:------------:|-------------|
| **NORMAL** | ✅ Allowed | No faults, normal operation |
| **WARNING** | ⚠️ Allowed | Non-critical fault detected |
| **ACTIVE** | ❌ Disabled | Critical fault, motors stopped |
| **LOCKOUT** | ❌ Disabled | Too many faults, requires power cycle |

---

## 📁 File Structure

| File | Description |
|------|-------------|
| `App_SafeState.h` | API declarations, fault definitions |
| `App_SafeState.c` | State machine implementation |

---

## 🚀 Quick Start

### 1. Initialize (Called by Safety Task)

```c
#include "App_SafeState.h"

void App_SafetyTask_Init(void) {
    SafeState_Init();
}
```

### 2. Enter Safe State on Fault

```c
/* On motor overload */
if (currentData.Status == ACS712_CHANNEL_OVERLOAD) {
    SafeState_Enter(SAFESTATE_REASON_MOTOR_LEFT_OVERLOAD);
    /* Motors are IMMEDIATELY stopped */
}
```

### 3. Check Permission in Control Task

```c
void App_ControlTask_Run(void) {
    /* ... compute PID ... */
    
    /* PRIVILEGE CHECK */
    if (SafeState_IsMotorEnableAllowed()) {
        Motor_SetSpeed(0u, leftSpeed);
        Motor_SetSpeed(1u, rightSpeed);
    } else {
        Motor_Stop(0u);
        Motor_Stop(1u);
    }
}
```

### 4. Request Motor Enable

```c
/* In Safety Task after health checks pass */
if (systemHealthy) {
    (void)SafeState_RequestMotorEnable();
}
```

---

## 🔧 API Reference

### Initialization
```c
void SafeState_Init(void);
```

### State Control
```c
void SafeState_Enter(SafeState_ReasonType Reason);  /* Enter safe state */
Std_ReturnType SafeState_Clear(void);               /* Attempt recovery */
```

### Fault Management
```c
void SafeState_SetFault(uint32 FaultFlag);          /* Set fault bit */
void SafeState_ClearFault(uint32 FaultFlag);        /* Clear fault bit */
uint32 SafeState_GetFaultFlags(void);               /* Get active faults */
boolean SafeState_HasCriticalFault(void);           /* Check critical */
```

### Privilege Control
```c
boolean SafeState_IsMotorEnableAllowed(void);       /* Check permission */
boolean SafeState_RequestMotorEnable(void);         /* Request enable */
void SafeState_RequestMotorDisable(void);           /* Force disable */
```

### Status
```c
SafeState_StatusType SafeState_GetStatus(void);
Std_ReturnType SafeState_GetInfo(SafeState_InfoType* InfoPtr);
```

### Update (Safety Task only)
```c
void SafeState_Update(void);  /* Called every 10ms */
```

---

## ⚡ Fault Reasons

| Reason | Description | Critical |
|--------|-------------|:--------:|
| `SAFESTATE_REASON_MOTOR_LEFT_OVERLOAD` | Left motor current exceeded | ✅ |
| `SAFESTATE_REASON_MOTOR_RIGHT_OVERLOAD` | Right motor current exceeded | ✅ |
| `SAFESTATE_REASON_MOTOR_LEFT_THERMAL` | Left motor overheating | ✅ |
| `SAFESTATE_REASON_MOTOR_RIGHT_THERMAL` | Right motor overheating | ✅ |
| `SAFESTATE_REASON_ENCLOSURE_THERMAL` | Enclosure too hot | ✅ |
| `SAFESTATE_REASON_COMMAND_TIMEOUT` | No ROS command received | ❌ |
| `SAFESTATE_REASON_HEARTBEAT_TIMEOUT` | ROS connection lost | ❌ |
| `SAFESTATE_REASON_ESTOP_COMMAND` | Emergency stop requested | ✅ |

---

## 🛡️ Privilege Model

```
┌─────────────────────────────────────────────────────────────────┐
│                    WHO CAN DO WHAT                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  SafeState_Enter()              → Safety Task, Any Task (fault) │
│  SafeState_RequestMotorEnable() → Safety Task ONLY              │
│  SafeState_IsMotorEnableAllowed() → Control Task (read-only)    │
│  SafeState_Update()             → Safety Task ONLY (10ms)       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔄 Recovery Process

```
1. Fault occurs
   └─► SafeState_Enter() → Motors STOP
   
2. Fault condition clears
   └─► SafeState_ClearFault()
   
3. Cooldown period (1 second)
   └─► System waits
   
4. Health checks pass
   └─► SafeState_RequestMotorEnable() → Motors CAN run
   
5. Control Task checks
   └─► SafeState_IsMotorEnableAllowed() = TRUE
```

### Lockout Condition

After **10 faults** in one session:
- State becomes `SAFESTATE_LOCKOUT`
- `SafeState_Clear()` returns `E_NOT_OK`
- **Power cycle required**

---

## 💡 Usage Examples

### Example 1: Integration with Watchdog

```c
void App_SafetyTask_Run(void) {
    boolean systemHealthy = App_Safety_EvaluateHealth();
    
    /* Update safe state */
    SafeState_Update();
    
    /* CRITICAL: Only feed WDG if system healthy */
    if (systemHealthy && (SafeState_GetStatus() <= SAFESTATE_WARNING)) {
        Wdg_Service();
    }
    /* If not fed, WDG resets MCU → motors stop */
}
```

### Example 2: E-Stop from ROS

```c
void App_CommTask_HandleCommand(char* cmd) {
    if (cmd[0] == 'S') {  /* Stop command */
        SafeState_Enter(SAFESTATE_REASON_ESTOP_COMMAND);
        SendAck("STOPPED");
    }
}
```

### Example 3: Recovery After Overload

```c
/* In Safety Task */
if (currentOk && SafeState_GetStatus() == SAFESTATE_ACTIVE) {
    /* Fault cleared, attempt recovery */
    SafeState_ClearFault(FAULT_FLAG_MOTOR_L_OVERLOAD);
    
    /* After cooldown, RequestMotorEnable will succeed */
}
```

---

## ⚠️ Important Notes

### Motor Stop Behavior

When `SafeState_Enter()` is called:
1. `Motor_StopAll()` is called **immediately**
2. `MotorEnableAllowed` is set to `FALSE`
3. Fault flags are updated
4. Timestamp is recorded

### Cooldown Period

- Default: **1 second** (100 ticks at 100Hz)
- Prevents rapid on/off cycling
- Configurable via `SAFESTATE_COOLDOWN_TICKS`

---

## 🐛 Troubleshooting

### Motors Won't Start

**Checklist:**
- ✅ No active faults: `SafeState_GetFaultFlags() == 0`
- ✅ Status is NORMAL: `SafeState_GetStatus() == SAFESTATE_NORMAL`
- ✅ Request granted: `SafeState_RequestMotorEnable() == TRUE`
- ✅ Control Task checks: `SafeState_IsMotorEnableAllowed()`

### Unexpected Motor Stop

**Check fault flags:**
```c
uint32 faults = SafeState_GetFaultFlags();
/* Decode which fault occurred */
if (faults & FAULT_FLAG_MOTOR_L_OVERLOAD) { /* ... */ }
if (faults & FAULT_FLAG_HEARTBEAT_TIMEOUT) { /* ... */ }
```

### System in LOCKOUT

**Cause:** 10+ faults occurred

**Solution:** Power cycle required (by design)

---

## 📚 Related Modules

- **App_SafetyTask** - Calls SafeState_Update() every 10ms
- **App_ControlTask** - Checks SafeState_IsMotorEnableAllowed()
- **App_ResourceMap** - Publishes SafetyStatus to Comm
- **MPU Driver** - Hardware backup protection

---

**Your robot now has layered safety protection!** 🛡️🤖
