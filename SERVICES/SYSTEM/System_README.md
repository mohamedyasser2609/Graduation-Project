# System Initialization Documentation

**Version:** 1.0.0  
**Date:** January 9, 2026  
**Module:** SERVICES/SYSTEM  
**Layer:** Services

---

## 📋 Overview

The System module provides centralized initialization of all drivers and services in the correct dependency order. It ensures all layers are properly configured before the FreeRTOS scheduler starts.

---

## 🏗️ Initialization Order

```
┌─────────────────────────────────────────────────────────────────┐
│              SYSTEM INITIALIZATION PHASES                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Phase 0: MPU ──► Configure memory protection (FIRST!)         │
│  Phase 1: MCAL ──► GPIO, UART, PWM, I2C, ADC, QEI               │
│  Phase 2: ECUAL ──► Motor, Encoder, IMU, GPS, Sensors           │
│  Phase 3: DIAG ──► Diagnostics service                          │
│  Phase 4: SERVICES ──► ComStack, ThermalMgmt, PID               │
│  Phase 5: APP ──► Robot_Control                                 │
│  Phase 6: WDG ──► Watchdog (LAST - after all init)              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📁 File Structure

| File | Description |
|------|-------------|
| `System_Services.h` | API declarations |
| `System_Init.c` | Initialization sequence |

---

## 🚀 Usage

```c
#include "System_Services.h"
#include "Tasks_Init.h"

int main(void) {
    /* Initialize all drivers in dependency order */
    System_Init();
    
    /* Create tasks and start scheduler */
    Tasks_Init();  /* Never returns */
    
    for (;;) { }
}
```

---

## 🔧 API Reference

```c
void System_Init(void);           /* Initialize all drivers */
void System_DeInit(void);         /* Stop motors/fans, shutdown */
boolean System_IsInitialized(void); /* Check init status */
```

---

## ⚠️ Important Notes

### MPU Must Be First

MPU_Init() is called **before** any peripheral access to ensure:
- Protected regions are configured
- MemManage handler is ready
- Any early fault is caught

### WDG Must Be Last

Wdg_Init() is called **after** all other initialization:
- All drivers ready before WDG starts
- Immediate reset if any init hangs
- Safety Task feeds WDG after scheduler starts

---

## 📚 Related Modules

- **MPU** - First initialization
- **WDG** - Last initialization
- **Tasks_Init** - Starts scheduler after System_Init

---

**Your system is properly initialized!** ⚡🔧
