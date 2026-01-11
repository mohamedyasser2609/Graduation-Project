# Diagnostics Service Documentation

**Version:** 1.0.0  
**Date:** January 9, 2026  
**Module:** SERVICES/DIAG  
**Layer:** Services

---

## 📋 Overview

The Diagnostics service provides event logging, Diagnostic Trouble Code (DTC) management, and debug output for the UGV controller. It follows AUTOSAR patterns for error reporting and traceability.

### **Key Features**

✅ **Event Logging** - Timestamped events with severity levels  
✅ **DTC Management** - Fault code tracking and status  
✅ **Debug Output** - UART-based debug printing  
✅ **Circular Buffer** - Fixed-size event storage  
✅ **Health Monitoring** - Integration with SafeState  

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    DIAGNOSTICS FLOW                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Safety Task ──► Diag_ReportDtc() ──► DTC Storage               │
│                                                                 │
│  Any Task ──► Diag_LogEvent() ──► Event Ring Buffer             │
│                                                                 │
│  Debug ──► Diag_DebugPrint() ──► UART Output                    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📁 File Structure

| File | Description |
|------|-------------|
| `Diagnostics.h` | API declarations |
| `Diagnostics.c` | Service implementation |
| `Diag_PBCfg.c` | Configuration |

---

## 🚀 Quick Start

### 1. Initialize

```c
#include "Diagnostics.h"

extern const Diag_ConfigType Diag_Config;

void main(void) {
    Diag_Init(&Diag_Config);
}
```

### 2. Log Events

```c
/* Log system startup */
Diag_LogEvent(DIAG_SRC_SYSTEM, 0x0001, DIAG_SEVERITY_INFO, NULL);

/* Log motor fault */
Diag_LogEvent(DIAG_SRC_MOTOR, 0x0010, DIAG_SEVERITY_ERROR, NULL);
```

### 3. Report DTCs

```c
/* Set fault active */
Diag_ReportDtc(DIAG_DTC_MOTOR_OVERLOAD, TRUE);

/* Clear fault */
Diag_ReportDtc(DIAG_DTC_MOTOR_OVERLOAD, FALSE);
```

### 4. Debug Print

```c
Diag_DebugPrint("[CTRL] PID output: %d\r\n", pidOutput);
```

---

## 🔧 API Reference

### Initialization
```c
void Diag_Init(const Diag_ConfigType* ConfigPtr);
Diag_StatusType Diag_GetStatus(void);
```

### Event Logging
```c
void Diag_LogEvent(Diag_SourceType Source, 
                   uint16 EventId,
                   Diag_SeverityType Severity,
                   const void* DataPtr);
```

### DTC Management
```c
void Diag_ReportDtc(uint16 DtcCode, boolean IsActive);
boolean Diag_IsDtcActive(uint16 DtcCode);
void Diag_ClearAllDtcs(void);
```

### Debug Output
```c
void Diag_DebugPrint(const char* Format, ...);
```

---

## ⚡ Severity Levels

| Level | Value | Use Case |
|-------|:-----:|----------|
| `DIAG_SEVERITY_INFO` | 0 | Normal events |
| `DIAG_SEVERITY_WARNING` | 1 | Non-critical issues |
| `DIAG_SEVERITY_ERROR` | 2 | Recoverable errors |
| `DIAG_SEVERITY_FATAL` | 3 | System-critical errors |

---

## 📊 DTC Codes

| Code | Description |
|------|-------------|
| `DIAG_DTC_MOTOR_OVERLOAD` | Motor current exceeded |
| `DIAG_DTC_THERMAL_CRITICAL` | Temperature warning |
| `DIAG_DTC_THERMAL_SHUTDOWN` | Thermal shutdown |
| `DIAG_DTC_COMM_TIMEOUT` | Communication lost |
| `DIAG_DTC_SENSOR_FAULT` | Sensor malfunction |

---

## 📚 Related Modules

- **App_SafeState** - Reports faults via DTCs
- **App_SafetyTask** - Primary DTC source
- **ComStack** - Debug UART output

---

**Your system events are now tracked!** 📝🔍
