# Watchdog (WDG) Driver Documentation

**Version:** 1.0.0  
**Date:** November 4, 2025  
**Target:** TM4C123GH6PM LaunchPad  
**Module:** MCAL - Watchdog Driver

---

## 📋 Overview

The Watchdog (WDG) driver provides an AUTOSAR-compliant abstraction for the TM4C123GH6PM watchdog timers. It supports both watchdog instances, enabling reset and interrupt modes, configurable timeouts, and DET-based error handling to protect against system hangs and software failures.

### **Key Features**

✅ **Dual Watchdog Modules** - WDG0 and WDG1 support  
✅ **Reset Protection** - Automatic system reset on timeout  
✅ **Interrupt Mode** - Warning before reset  
✅ **Configurable Timeout** - From milliseconds to minutes  
✅ **Debug Stall** - Pause during debugging  
✅ **AUTOSAR Compliant** - Full AUTOSAR 4.4.0 compliance  

---

## 🏗️ Architecture

```
┌─────────────────────────────────────┐
│      Application Layer              │
│  (periodic servicing, monitoring)   │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│         WDG Driver API              │
│  - Wdg_Init()                       │
│  - Wdg_Service()                    │
│  - Wdg_SetTriggerCondition()        │
│  - Wdg_Disable()                    │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│    Hardware Abstraction Layer       │
│  - WDT register access              │
│  - Lock/unlock mechanism            │
│  - Interrupt handling               │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│         TM4C123 Hardware            │
│  - Watchdog Timer 0                 │
│  - Watchdog Timer 1                 │
│  - System reset controller          │
└─────────────────────────────────────┘
```

---

## 📁 File Structure

| File | Description |
|------|-------------|
| `WDG.h` | Driver interface and API declarations |
| `WDG.c` | Complete driver implementation |
| `WDG_Types.h` | Type definitions and enumerations |
| `WDG_Cfg.h` | Compile-time configuration |
| `WDG_PBCfg.c` | Post-build configuration example |

---

## 🚀 Quick Start

### 1. Basic Watchdog Setup (1 Second Timeout)

```c
#include "WDG.h"
#include "WDG_PBCfg.h"

int main(void) {
    /* Initialize watchdog with 1 second timeout */
    Wdg_Init(&Wdg_Config);
    
    while(1) {
        /* Do work */
        ProcessTask();
        
        /* Service watchdog before timeout */
        Wdg_Service();  /* Reset watchdog timer */
        
        /* More work */
        ProcessAnotherTask();
    }
}
```

### 2. Watchdog with Interrupt Warning

```c
volatile boolean watchdogWarning = FALSE;

/* Watchdog interrupt callback */
void Wdg_TimeoutCallback(void) {
    /* Called before reset - last chance! */
    watchdogWarning = TRUE;
    
    /* Try to recover or log error */
    SaveCriticalData();
    
    /* Service to prevent reset */
    Wdg_Service();
}

/* Configuration with interrupt */
const Wdg_ConfigType Wdg_IntConfig = {
    .Instance = WDG_INSTANCE_0,
    .InitialTimeoutTicks = 16000000,    /* 1 second @ 16MHz */
    .MinTimeoutTicks = 1600000,         /* 100ms min */
    .MaxTimeoutTicks = 80000000,        /* 5 seconds max */
    .ResetEnable = TRUE,
    .InterruptEnable = TRUE,            /* Enable interrupt */
    .DebugStallEnable = TRUE,
    .NotificationCallback = Wdg_TimeoutCallback
};

int main(void) {
    Wdg_Init(&Wdg_IntConfig);
    
    while(1) {
        if (watchdogWarning) {
            /* System recovered from near-timeout */
            LogError("Watchdog warning!");
            watchdogWarning = FALSE;
        }
        
        /* Normal operation */
        Wdg_Service();
        delay_ms(500);  /* Service every 500ms */
    }
}
```

### 3. Adjustable Timeout

```c
int main(void) {
    Wdg_Init(&Wdg_Config);
    
    /* Normal mode - 1 second timeout */
    Wdg_SetTriggerMode(WDG_TRIGGER_MODE_NORMAL);
    Wdg_SetTriggerCondition(16000000);  /* 1 second */
    
    while(1) {
        /* Entering long operation */
        Wdg_SetTriggerCondition(80000000);  /* Extend to 5 seconds */
        
        LongRunningOperation();
        
        /* Back to normal */
        Wdg_SetTriggerCondition(16000000);  /* 1 second */
        
        Wdg_Service();
    }
}
```

---

## ⏱️ Timeout Calculation

### Formula
```
TimeoutTicks = DesiredTimeout_seconds × SystemClock_Hz
```

### Common Timeouts @ 16 MHz

| Desired Timeout | Ticks Required | Configuration |
|-----------------|----------------|---------------|
| **100 ms** | 1,600,000 | Normal |
| **500 ms** | 8,000,000 | Normal |
| **1 second** | 16,000,000 | **Most common** |
| **2 seconds** | 32,000,000 | Normal |
| **5 seconds** | 80,000,000 | Extended |
| **10 seconds** | 160,000,000 | Maximum practical |

### Example Calculation

```c
/* Calculate timeout for 2.5 seconds @ 16MHz */
uint32 timeout = 2.5 * 16000000 = 40000000;

Wdg_SetTriggerCondition(40000000);
```

---

## 💡 Usage Examples

### Example 1: Simple Watchdog Protection

```c
#include "WDG.h"

int main(void) {
    /* Initialize with 1 second timeout */
    Wdg_Init(&Wdg_Config);
    
    while(1) {
        /* Task 1 */
        ReadSensors();
        
        /* Task 2 */
        ProcessData();
        
        /* Task 3 */
        UpdateOutputs();
        
        /* Service watchdog - all tasks completed */
        Wdg_Service();
        
        /* If any task hangs, watchdog will reset system */
    }
}
```

### Example 2: Task Monitoring

```c
#define MAX_TASK_TIME_MS  500

volatile uint32 taskStartTime = 0;

void MonitoredTask(void) {
    taskStartTime = GetSystemTick_ms();
    
    /* Do work */
    ProcessComplexAlgorithm();
    
    /* Check execution time */
    uint32 elapsed = GetSystemTick_ms() - taskStartTime;
    if (elapsed > MAX_TASK_TIME_MS) {
        /* Task took too long - don't service watchdog */
        /* System will reset */
        while(1);  /* Wait for watchdog reset */
    }
    
    /* Task completed in time */
    Wdg_Service();
}
```

### Example 3: Multi-Task Watchdog

```c
typedef struct {
    boolean task1Complete;
    boolean task2Complete;
    boolean task3Complete;
} TaskStatus_t;

TaskStatus_t taskStatus = {FALSE, FALSE, FALSE};

void Task1(void) {
    /* Do work */
    taskStatus.task1Complete = TRUE;
}

void Task2(void) {
    /* Do work */
    taskStatus.task2Complete = TRUE;
}

void Task3(void) {
    /* Do work */
    taskStatus.task3Complete = TRUE;
}

int main(void) {
    Wdg_Init(&Wdg_Config);
    
    while(1) {
        /* Reset status */
        taskStatus.task1Complete = FALSE;
        taskStatus.task2Complete = FALSE;
        taskStatus.task3Complete = FALSE;
        
        /* Execute tasks */
        Task1();
        Task2();
        Task3();
        
        /* Only service if ALL tasks completed */
        if (taskStatus.task1Complete && 
            taskStatus.task2Complete && 
            taskStatus.task3Complete) {
            Wdg_Service();  /* All good */
        }
        /* If any task failed, watchdog will reset */
    }
}
```

### Example 4: Recovery from Watchdog Reset

```c
#include "WDG.h"
#include "MCU.h"

uint32 watchdogResetCount = 0;

int main(void) {
    /* Check if last reset was from watchdog */
    if (Mcu_GetResetReason() == MCU_WATCHDOG_RESET) {
        watchdogResetCount++;
        
        /* Log error */
        LogError("System recovered from watchdog reset");
        LogError("Reset count: %d", watchdogResetCount);
        
        /* Take corrective action */
        if (watchdogResetCount > 3) {
            /* Too many watchdog resets - enter safe mode */
            EnterSafeMode();
        }
    }
    
    /* Initialize watchdog */
    Wdg_Init(&Wdg_Config);
    
    /* Normal operation */
    while(1) {
        Wdg_Service();
        /* ... */
    }
}
```

### Example 5: Conditional Watchdog Disable

```c
int main(void) {
    Wdg_Init(&Wdg_Config);
    
    while(1) {
        /* Normal operation with watchdog */
        Wdg_Service();
        
        /* User requests firmware update */
        if (FirmwareUpdateRequested()) {
            /* Disable watchdog during update */
            Wdg_Disable();
            
            /* Perform update (may take several seconds) */
            UpdateFirmware();
            
            /* Re-enable watchdog */
            Wdg_Init(&Wdg_Config);
        }
    }
}
```

---

## 🔧 API Reference

### Initialization & Control
```c
void Wdg_Init(const Wdg_ConfigType* ConfigPtr);
void Wdg_DeInit(void);
void Wdg_Service(void);
void Wdg_Disable(void);
```

### Configuration
```c
void Wdg_SetTriggerMode(Wdg_TriggerModeType Mode);
void Wdg_SetTriggerCondition(uint32 TimeoutTicks);
```

### Status
```c
Wdg_StatusType Wdg_GetStatus(void);
```

### Version Info (if enabled)
```c
void Wdg_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
```

---

## 🐛 Troubleshooting

### Unexpected System Resets

**Symptoms:** System resets randomly

**Checklist:**
- ✅ `Wdg_Service()` called frequently enough
- ✅ Service interval < timeout period
- ✅ No blocking delays longer than timeout
- ✅ All code paths service watchdog

**Solution:**
```c
/* BAD - delay longer than timeout */
Wdg_Init(&Wdg_Config);  /* 1 second timeout */
while(1) {
    delay_ms(2000);  /* 2 seconds - WILL RESET! */
    Wdg_Service();
}

/* GOOD - service before timeout */
while(1) {
    delay_ms(500);   /* 500ms - safe */
    Wdg_Service();
}
```

### Watchdog Not Resetting System

**Checklist:**
- ✅ `ResetEnable = TRUE` in configuration
- ✅ Watchdog initialized properly
- ✅ Not servicing watchdog (to test)
- ✅ Debug mode not stalling watchdog

**Test:**
```c
/* Test watchdog reset */
Wdg_Init(&Wdg_Config);
/* Don't call Wdg_Service() */
while(1);  /* Should reset after timeout */
```

### Interrupt Not Firing

**Checklist:**
- ✅ `InterruptEnable = TRUE` in config
- ✅ Callback function provided
- ✅ NVIC interrupt enabled for watchdog
- ✅ Interrupt cleared in callback

### Debug Session Resets

**Problem:** Watchdog resets during debugging

**Solution:**
```c
/* Enable debug stall in configuration */
const Wdg_ConfigType Wdg_DebugConfig = {
    .Instance = WDG_INSTANCE_0,
    .InitialTimeoutTicks = 16000000,
    .DebugStallEnable = TRUE,  /* Pause during debug */
    /* ... */
};
```

---

## ⚠️ Important Notes

### Watchdog Lock Mechanism

The watchdog uses a hardware lock to prevent accidental modification:

```c
/* Lock sequence (handled by driver) */
1. Write 0x1ACCE551 to WDT_LOCK
2. Modify registers
3. Lock automatically re-engages
```

### Service Timing

**Best Practice:**
- Service at **50-75%** of timeout period
- Example: 1 second timeout → service every 500-750ms

```c
/* 1 second timeout */
Wdg_Init(&Wdg_Config_1s);

while(1) {
    /* Service every 500ms (50% of timeout) */
    Wdg_Service();
    delay_ms(500);
}
```

### Reset vs Interrupt Mode

| Mode | Behavior | Use Case |
|------|----------|----------|
| **Reset Only** | Immediate reset on timeout | Production systems |
| **Interrupt + Reset** | Warning before reset | Development, logging |
| **Interrupt Only** | No reset, just notification | Monitoring only |

---

## 📊 Configuration Examples

### Production Configuration (Reset Mode)

```c
const Wdg_ConfigType Wdg_Production = {
    .Instance = WDG_INSTANCE_0,
    .InitialTimeoutTicks = 16000000,    /* 1 second */
    .MinTimeoutTicks = 8000000,         /* 500ms min */
    .MaxTimeoutTicks = 32000000,        /* 2 seconds max */
    .ResetEnable = TRUE,                /* Enable reset */
    .InterruptEnable = FALSE,           /* No interrupt */
    .DebugStallEnable = FALSE,          /* Run during debug */
    .NotificationCallback = NULL_PTR
};
```

### Development Configuration (Interrupt Mode)

```c
void Wdg_DevCallback(void) {
    LogWarning("Watchdog timeout warning!");
    Wdg_Service();  /* Prevent reset */
}

const Wdg_ConfigType Wdg_Development = {
    .Instance = WDG_INSTANCE_0,
    .InitialTimeoutTicks = 80000000,    /* 5 seconds */
    .MinTimeoutTicks = 16000000,        /* 1 second min */
    .MaxTimeoutTicks = 160000000,       /* 10 seconds max */
    .ResetEnable = TRUE,
    .InterruptEnable = TRUE,            /* Enable warning */
    .DebugStallEnable = TRUE,           /* Pause during debug */
    .NotificationCallback = Wdg_DevCallback
};
```

---

## 🎓 Best Practices

### ✅ DO:
- Service watchdog regularly and predictably
- Use interrupt mode during development
- Enable debug stall for debugging
- Log watchdog resets for analysis
- Test watchdog reset functionality
- Document service points in code

### ❌ DON'T:
- Service watchdog in interrupt handlers (usually)
- Use very short timeouts (< 100ms)
- Disable watchdog in production without reason
- Forget to service after long operations
- Service watchdog unconditionally in error paths

---

## 📈 Performance

**@ 16 MHz System Clock:**

| Operation | Time | Notes |
|-----------|------|-------|
| `Wdg_Init()` | < 100 μs | One-time initialization |
| `Wdg_Service()` | < 20 μs | Register write + unlock |
| `Wdg_SetTriggerCondition()` | < 30 μs | Includes unlock sequence |
| Interrupt Latency | < 50 μs | From timeout to ISR |

---

## 🔗 Integration with Other Drivers

### With MCU Driver
```c
/* Check reset reason */
if (Mcu_GetResetReason() == MCU_WATCHDOG_RESET) {
    /* Handle watchdog reset */
}
```

### With NVIC Driver
```c
/* Enable watchdog interrupt in NVIC */
NVIC_SetPriority(NVIC_WATCHDOG0_IRQ, 2);
NVIC_EnableIRQ(NVIC_WATCHDOG0_IRQ);
```

### With Timer Driver
```c
/* Use timer for periodic servicing */
void Timer_1Hz_Callback(void) {
    Wdg_Service();  /* Service every second */
}
```

---

## 📚 Related Drivers

- **MCU Driver** - Reset reason detection
- **NVIC Driver** - Interrupt management
- **Timer Driver** - Periodic servicing

---

**Your system is now protected against hangs!** 🛡️⚡
