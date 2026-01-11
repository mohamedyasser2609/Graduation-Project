# APP Layer - Safety Module

**Version:** 1.0.0  
**Date:** January 10, 2026  
**Layer:** APPLICATION

---

## 📁 Contents

| File | Description |
|------|-------------|
| `App_SafeState.h` | Safe state manager API |
| `App_SafeState.c` | State machine and motor enable control |
| `App_SafteyTask.c` | Safety monitoring task (Priority 4) |
| `SafeState_README.md` | Detailed documentation |

---

## 🎯 Purpose

The Safety module provides:
- **Motor enable privilege control** - Only Safety Task can enable motors
- **Fault management** - Track, escalate, and recover from faults
- **Watchdog feeding** - Health-gated WDG service
- **Safe state enforcement** - Immediate motor stop on fault

---

## 🔗 Dependencies

**Uses:**
- `MCAL/WDG` - Watchdog service
- `ECUAL/CURRENT_SENSOR` - Motor current monitoring
- `SERVICES/THERMAL` - Temperature checking

**Used by:**
- `APP/Tasks/App_ControlTask.c` - Checks `SafeState_IsMotorEnableAllowed()`
- `APP/Common/App_ResourceMap.c` - Publishes safety status

---

## 📊 Task Details

| Property | Value |
|----------|-------|
| Priority | 4 (Highest) |
| Period | 10ms |
| Stack | 128 words |
| Owner | WDG, Current sensors |

---

**See `SafeState_README.md` for detailed documentation.**
