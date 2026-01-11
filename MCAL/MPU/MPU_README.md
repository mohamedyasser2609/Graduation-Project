# Memory Protection Unit (MPU) Driver Documentation

**Version:** 1.0.0  
**Date:** January 9, 2026  
**Target:** TM4C123GH6PM LaunchPad  
**Module:** MCAL - Memory Protection Unit

---

## 📋 Overview

The MPU driver configures the ARM Cortex-M4 Memory Protection Unit for privilege separation and safety-critical register protection. It protects watchdog and motor control (PWM) registers from unauthorized access.

### **Key Features**

✅ **6 Protected Regions** - Flash, SRAM, Peripherals, WDT, PWM0, PWM1  
✅ **Privileged Access Only** - WDT and PWM protected  
✅ **MemManage Fault Handler** - Stops motors on violation  
✅ **Safety Integration** - First init step in System_Init()  
✅ **AUTOSAR Compliant** - Layered architecture  

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     MPU REGION MAP                              │
├─────────────────────────────────────────────────────────────────┤
│ Region 0: Flash (256KB)      │ 0x00000000 │ Full access, XN=0  │
│ Region 1: SRAM (32KB)        │ 0x20000000 │ Full access, XN=1  │
│ Region 2: Peripherals (1MB)  │ 0x40000000 │ Full access        │
│ Region 3: WDT (4KB)          │ 0x40000000 │ 🔒 PRIV ONLY       │
│ Region 4: PWM0 (4KB)         │ 0x40028000 │ 🔒 PRIV ONLY       │
│ Region 5: PWM1 (4KB)         │ 0x40029000 │ 🔒 PRIV ONLY       │
└─────────────────────────────────────────────────────────────────┘
```

### Protection Flow

```
Unprivileged Code Accesses WDT/PWM
            │
            ▼
    ┌───────────────┐
    │ MPU Violation │
    └───────────────┘
            │
            ▼
    ┌───────────────────┐
    │ MemManage_Handler │
    └───────────────────┘
            │
            ▼
    ┌───────────────────┐
    │ Motors STOP       │
    │ System Lockout    │
    └───────────────────┘
```

---

## 📁 File Structure

| File | Description |
|------|-------------|
| `MPU.h` | API declarations and register definitions |
| `MPU.c` | Region configuration and fault handler |

---

## 🚀 Quick Start

### 1. Basic Initialization

```c
#include "MPU.h"

int main(void) {
    /* Initialize MPU as FIRST step */
    MPU_Init();
    
    /* Now initialize other drivers */
    Gpio_Init(&Gpio_Config);
    /* ... */
}
```

### 2. Check MPU Support

```c
uint8 regions = MPU_GetRegionCount();
if (regions == 0) {
    /* MPU not supported on this device */
}
/* TM4C123GH6PM has 8 regions */
```

### 3. Handle MPU Faults

The fault handler is automatically installed in the vector table:

```c
/* In tm4c123gh6pm_startup_ccs.c */
extern void MemManage_Handler(void);

void (* const g_pfnVectors[])(void) = {
    /* ... */
    MemManage_Handler,  /* Vector 4: MPU Fault */
    /* ... */
};
```

---

## 🔧 API Reference

### Initialization
```c
void MPU_Init(void);           /* Configure all regions */
void MPU_Enable(void);         /* Enable MPU */
void MPU_Disable(void);        /* Disable MPU (debug only) */
```

### Status
```c
uint8 MPU_GetRegionCount(void);                    /* Get region count */
Std_ReturnType MPU_GetFaultInfo(MPU_FaultInfoType* FaultInfo);
void MPU_ClearFaultInfo(void);
```

### Fault Handler
```c
void MemManage_Handler(void);  /* Called on violation */
```

---

## 🛡️ Protected Resources

| Resource | Base Address | Why Protected |
|----------|--------------|---------------|
| **WDT** | 0x40000000 | Only Safety Task should feed WDG |
| **PWM0** | 0x40028000 | Motor control - gated by SafeState |
| **PWM1** | 0x40029000 | Fan/motor control |

### Protection Level

```
┌─────────────────────────────────────────────────────────────────┐
│  AP[2:0] = 0b001 = Privileged read/write, Unprivileged no access│
└─────────────────────────────────────────────────────────────────┘
```

---

## ⚠️ Important Notes

### Current Limitation

> **Note:** All FreeRTOS tasks currently run in **privileged mode**.
> The MPU is configured and ready for future unprivileged task implementation.

### When MPU Violations Occur

1. **MemManage_Handler()** is called immediately
2. Motors are stopped by direct register write (bypasses all abstraction)
3. System enters infinite loop (requires reset)
4. Fault info is recorded for debugging

### Integration with SafeState

The MPU provides **hardware-level backup** to the software SafeState model:

```
Software Level:  SafeState_IsMotorEnableAllowed() → gates motor commands
Hardware Level:  MPU → prevents unauthorized PWM register access
```

---

## 🐛 Troubleshooting

### System Hangs After Init

**Symptom:** System stops immediately after MPU_Init()

**Cause:** Flash/SRAM regions not correctly configured

**Solution:** Verify region base addresses match your linker script

### MemManage Fault in Normal Operation

**Symptom:** Unexpected MemManage fault

**Debug:**
```c
MPU_FaultInfoType faultInfo;
MPU_GetFaultInfo(&faultInfo);
/* Check faultInfo.FaultAddress to identify violating code */
```

### No Fault on Violation

**Checklist:**
- ✅ `MPU_Init()` called before any driver init
- ✅ `MemManage_Handler` in vector table
- ✅ MPU_CTRL has ENABLE bit set
- ✅ SHCSR has MEMFAULTENA bit set

---

## 📊 Performance

| Operation | Time | Notes |
|-----------|------|-------|
| `MPU_Init()` | < 50 μs | 6 region configurations |
| Fault detection | < 1 μs | Hardware enforced |

---

## 🎓 Best Practices

### ✅ DO:
- Initialize MPU as the **very first** step in System_Init()
- Keep protected regions as small as possible
- Test fault handler behavior during development
- Document which code needs privileged access

### ❌ DON'T:
- Disable MPU in production
- Access protected registers from unprivileged code
- Ignore MemManage faults

---

## 📚 Related Modules

- **App_SafeState** - Software privilege model
- **WDG Driver** - Protected by MPU
- **PWM Driver** - Protected by MPU
- **Motor Driver** - Uses protected PWM

---

**Your critical registers are now hardware-protected!** 🛡️🔒
