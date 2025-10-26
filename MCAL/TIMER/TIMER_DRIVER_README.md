# Timer Driver - Complete Documentation

**Version:** 1.0.0  
**Date:** October 26, 2025  
**Author:** Mohamed Yasser  
**Target:** TM4C123GH6PM (Tiva C Series)

---

## Table of Contents

1. [Overview](#overview)
2. [Features](#features)
3. [Hardware Overview](#hardware-overview)
4. [API Reference](#api-reference)
5. [Configuration](#configuration)
6. [Usage Examples](#usage-examples)
7. [Interrupt Handling](#interrupt-handling)
8. [Testing](#testing)

---

## Overview

The Timer driver provides a comprehensive interface for the TM4C123GH6PM's General Purpose Timer Modules (GPTM). It supports periodic timers, one-shot timers, PWM generation, and input capture with full interrupt support.

### Key Highlights

- ✅ **6 Timer Modules** (Timer0-Timer5)
- ✅ **12 Independent Timers** (2 blocks per module: A and B)
- ✅ **32-bit or dual 16-bit** configuration
- ✅ **Multiple Operating Modes** (Periodic, One-shot, Capture, PWM)
- ✅ **Prescaler Support** for extended timing ranges
- ✅ **Interrupt Support** with callbacks
- ✅ **AUTOSAR-Compliant** API structure
- ✅ **DET Integration** for error detection

### Files Structure

```
MCAL/TIMER/
├── Timer.h              (236 lines) - API declarations
├── Timer.c              (501 lines) - Implementation
├── Timer_Types.h        (135 lines) - Type definitions
├── Timer_Cfg.h          (94 lines)  - Configuration
├── Timer_Regs.h         (129 lines) - Register definitions
├── Timer_PBCfg.c        (155 lines) - Example configurations
└── TIMER_DRIVER_README.md (this file)
```

---

## Features

### Operating Modes

| Mode | Description | Use Cases |
|------|-------------|-----------|
| **Periodic** | Timer reloads automatically | System tick, periodic tasks, LED blinking |
| **One-Shot** | Timer stops after timeout | Delays, timeouts, one-time events |
| **PWM** | Generates PWM signals | Motor control, LED dimming, audio |
| **Capture** | Captures input edge timing | Frequency measurement, pulse width |

### Timer Capabilities

- ✅ **32-bit Timers** - Up to 4.29 billion counts (268 seconds @ 16MHz)
- ✅ **16-bit Timers** - Up to 65,535 counts (4ms @ 16MHz)
- ✅ **Prescaler (8-bit)** - Extends range by 256×
- ✅ **Count Up/Down** - Flexible counting direction
- ✅ **Match Interrupts** - Interrupt at specific count
- ✅ **Timeout Interrupts** - Interrupt when timer expires

### Interrupt Support

- ✅ **Timeout Interrupts** - Timer reaches zero/match
- ✅ **Match Interrupts** - Timer matches compare value
- ✅ **Capture Interrupts** - Input edge detected
- ✅ **User Callbacks** - Easy interrupt handling

---

## Hardware Overview

### Timer Modules

| Module | Base Address | Block A IRQ | Block B IRQ |
|--------|-------------|-------------|-------------|
| Timer0 | 0x40030000 | 19 (0x13) | 20 (0x14) |
| Timer1 | 0x40031000 | 21 (0x15) | 22 (0x16) |
| Timer2 | 0x40032000 | 23 (0x17) | 24 (0x18) |
| Timer3 | 0x40033000 | 35 (0x23) | 36 (0x24) |
| Timer4 | 0x40034000 | 70 (0x46) | 71 (0x47) |
| Timer5 | 0x40035000 | 92 (0x5C) | 93 (0x5D) |

### Timing Calculations

**Formula:** `LoadValue = (System_Clock_Hz / Desired_Frequency_Hz) - 1`

**Examples @ 16 MHz:**
- **1 second:** `16,000,000 - 1 = 15,999,999`
- **100 ms:** `1,600,000 - 1 = 1,599,999`
- **10 ms:** `160,000 - 1 = 159,999`
- **1 ms:** `16,000 - 1 = 15,999`
- **100 μs:** `1,600 - 1 = 1,599`
- **10 μs:** `160 - 1 = 159`
- **1 μs:** `16 - 1 = 15`

---

## API Reference

### `Timer_Init()`
```c
void Timer_Init(const Timer_ConfigType* ConfigPtr);
```
Initialize timer with specified configuration.

**Parameters:**
- `ConfigPtr` - Pointer to timer configuration structure

**Example:**
```c
const Timer_ConfigType timerCfg = {
    .Module = TIMER_MODULE_0,
    .Block = TIMER_BLOCK_A,
    .ConfigMode = TIMER_CONFIG_32BIT,
    .OperationMode = TIMER_MODE_PERIODIC,
    .LoadValue = 16000000,  // 1 second @ 16MHz
    .InterruptEnable = TRUE
};
Timer_Init(&timerCfg);
```

---

### `Timer_Start()`
```c
void Timer_Start(Timer_ModuleType Module, Timer_BlockType Block);
```
Start the specified timer.

**Example:**
```c
Timer_Start(TIMER_MODULE_0, TIMER_BLOCK_A);
```

---

### `Timer_Stop()`
```c
void Timer_Stop(Timer_ModuleType Module, Timer_BlockType Block);
```
Stop the specified timer.

**Example:**
```c
Timer_Stop(TIMER_MODULE_0, TIMER_BLOCK_A);
```

---

### `Timer_GetElapsedTime()`
```c
Timer_ValueType Timer_GetElapsedTime(Timer_ModuleType Module, Timer_BlockType Block);
```
Get elapsed time since timer started (in ticks).

**Example:**
```c
uint32 elapsed = Timer_GetElapsedTime(TIMER_MODULE_0, TIMER_BLOCK_A);
```

---

### `Timer_GetRemainingTime()`
```c
Timer_ValueType Timer_GetRemainingTime(Timer_ModuleType Module, Timer_BlockType Block);
```
Get remaining time until timeout (in ticks).

**Example:**
```c
uint32 remaining = Timer_GetRemainingTime(TIMER_MODULE_0, TIMER_BLOCK_A);
```

---

### `Timer_SetLoadValue()`
```c
void Timer_SetLoadValue(Timer_ModuleType Module, Timer_BlockType Block, Timer_ValueType Value);
```
Update timer interval (takes effect on next reload).

**Example:**
```c
Timer_SetLoadValue(TIMER_MODULE_0, TIMER_BLOCK_A, 8000000);  // Change to 0.5s
```

---

## Configuration

### `Timer_ConfigType` Structure

```c
typedef struct {
    Timer_ModuleType            Module;                // Timer module (0-5)
    Timer_BlockType             Block;                 // Block A or B
    Timer_ConfigModeType        ConfigMode;            // 32-bit or 16-bit
    Timer_OperationModeType     OperationMode;         // Periodic, one-shot, capture
    Timer_CountDirectionType    CountDirection;        // Up or down
    Timer_PwmModeType           PwmMode;               // PWM enabled/disabled
    Timer_ValueType             LoadValue;             // Timer interval
    Timer_ValueType             MatchValue;            // PWM match value
    uint8                       Prescaler;             // 0-255 prescaler
    boolean                     InterruptEnable;       // Enable timeout interrupt
    boolean                     MatchInterruptEnable;  // Enable match interrupt
    Timer_NotificationFuncPtr   TimeoutCallback;       // Timeout callback function
    Timer_NotificationFuncPtr   MatchCallback;         // Match callback function
} Timer_ConfigType;
```

---

## Usage Examples

### Example 1: Simple Periodic Timer (LED Blink)

```c
#include "Timer.h"
#include "Led.h"

/* Timer configuration: 500ms periodic */
const Timer_ConfigType Timer_500ms = {
    .Module = TIMER_MODULE_0,
    .Block = TIMER_BLOCK_A,
    .ConfigMode = TIMER_CONFIG_32BIT,
    .OperationMode = TIMER_MODE_PERIODIC,
    .CountDirection = TIMER_COUNT_DOWN,
    .PwmMode = TIMER_PWM_DISABLED,
    .LoadValue = 8000000,        // 500ms @ 16MHz
    .MatchValue = 0,
    .Prescaler = 0,
    .InterruptEnable = TRUE,
    .MatchInterruptEnable = FALSE,
    .TimeoutCallback = NULL_PTR,
    .MatchCallback = NULL_PTR
};

int main(void) {
    /* Initialize drivers */
    Gpio_Init(&Gpio_Configuration);
    Led_Init(&Led_Red);
    
    /* Initialize and start timer */
    Timer_Init(&Timer_500ms);
    Timer_Start(TIMER_MODULE_0, TIMER_BLOCK_A);
    
    while(1) {
        /* Main loop - LED blinks via interrupt */
    }
}

/* Interrupt handler */
void Timer0A_Handler(void) {
    /* Clear interrupt */
    Timer_ClearInterrupt(TIMER_MODULE_0, TIMER_BLOCK_A, TIMER_INT_TIMEOUT);
    
    /* Toggle LED */
    Led_Toggle(&Led_Red);
}
```

---

### Example 2: One-Shot Timer (Delay)

```c
#include "Timer.h"

volatile boolean delayComplete = FALSE;

void DelayCallback(void) {
    delayComplete = TRUE;
}

void Delay_100ms(void) {
    const Timer_ConfigType Timer_Delay = {
        .Module = TIMER_MODULE_1,
        .Block = TIMER_BLOCK_A,
        .ConfigMode = TIMER_CONFIG_32BIT,
        .OperationMode = TIMER_MODE_ONESHOT,
        .LoadValue = 1600000,    // 100ms @ 16MHz
        .InterruptEnable = TRUE,
        .TimeoutCallback = DelayCallback
    };
    
    delayComplete = FALSE;
    Timer_Init(&Timer_Delay);
    Timer_Start(TIMER_MODULE_1, TIMER_BLOCK_A);
    
    /* Wait for delay to complete */
    while(delayComplete == FALSE) {
        /* Wait */
    }
}
```

---

### Example 3: PWM Generation (LED Dimming)

```c
#include "Timer.h"

/* PWM: 1kHz, 25% duty cycle */
const Timer_ConfigType Timer_PWM = {
    .Module = TIMER_MODULE_2,
    .Block = TIMER_BLOCK_A,
    .ConfigMode = TIMER_CONFIG_32BIT,
    .OperationMode = TIMER_MODE_PERIODIC,
    .CountDirection = TIMER_COUNT_DOWN,
    .PwmMode = TIMER_PWM_ENABLED,
    .LoadValue = 16000,          // 1kHz @ 16MHz
    .MatchValue = 12000,         // 25% duty cycle
    .Prescaler = 0,
    .InterruptEnable = FALSE,
    .MatchInterruptEnable = FALSE,
    .TimeoutCallback = NULL_PTR,
    .MatchCallback = NULL_PTR
};

void SetLedBrightness(uint8 brightness) {
    /* brightness: 0-100% */
    uint32 matchValue = (16000 * (100 - brightness)) / 100;
    Timer_SetLoadValue(TIMER_MODULE_2, TIMER_BLOCK_A, 16000);
    Timer_Start(TIMER_MODULE_2, TIMER_BLOCK_A);
}
```

---

### Example 4: System Tick (1ms)

```c
#include "Timer.h"

volatile uint32 systemTick = 0;

void SystemTickCallback(void) {
    systemTick++;
}

const Timer_ConfigType Timer_SysTick = {
    .Module = TIMER_MODULE_3,
    .Block = TIMER_BLOCK_A,
    .ConfigMode = TIMER_CONFIG_32BIT,
    .OperationMode = TIMER_MODE_PERIODIC,
    .LoadValue = 16000,          // 1ms @ 16MHz
    .InterruptEnable = TRUE,
    .TimeoutCallback = SystemTickCallback
};

uint32 GetSystemTick_ms(void) {
    return systemTick;
}

void main(void) {
    Timer_Init(&Timer_SysTick);
    Timer_Start(TIMER_MODULE_3, TIMER_BLOCK_A);
    
    while(1) {
        uint32 currentTime = GetSystemTick_ms();
        /* Use currentTime for timing */
    }
}
```

---

## Interrupt Handling

### Setting Up Interrupts

1. **Enable in NVIC** (if not done by startup code)
2. **Configure Timer** with interrupt enabled
3. **Implement ISR** with proper name
4. **Clear Interrupt** in ISR

### Interrupt Service Routine Names

| Timer | Block A ISR | Block B ISR |
|-------|-------------|-------------|
| Timer0 | `Timer0A_Handler` | `Timer0B_Handler` |
| Timer1 | `Timer1A_Handler` | `Timer1B_Handler` |
| Timer2 | `Timer2A_Handler` | `Timer2B_Handler` |
| Timer3 | `Timer3A_Handler` | `Timer3B_Handler` |
| Timer4 | `Timer4A_Handler` | `Timer4B_Handler` |
| Timer5 | `Timer5A_Handler` | `Timer5B_Handler` |

### ISR Template

```c
void Timer0A_Handler(void) {
    /* Clear interrupt flag */
    Timer_ClearInterrupt(TIMER_MODULE_0, TIMER_BLOCK_A, TIMER_INT_TIMEOUT);
    
    /* Your code here */
    // Process timer event
}
```

---

## Testing

### Test Case 1: Basic Periodic Timer

```c
void Test_PeriodicTimer(void) {
    volatile uint32 count = 0;
    
    const Timer_ConfigType cfg = {
        .Module = TIMER_MODULE_0,
        .Block = TIMER_BLOCK_A,
        .ConfigMode = TIMER_CONFIG_32BIT,
        .OperationMode = TIMER_MODE_PERIODIC,
        .LoadValue = 16000,  // 1ms @ 16MHz
        .InterruptEnable = FALSE
    };
    
    Timer_Init(&cfg);
    Timer_Start(TIMER_MODULE_0, TIMER_BLOCK_A);
    
    /* Wait for timer to expire */
    while(Timer_GetRemainingTime(TIMER_MODULE_0, TIMER_BLOCK_A) > 0) {
        count++;
    }
    
    /* Timer should have elapsed */
    assert(Timer_GetElapsedTime(TIMER_MODULE_0, TIMER_BLOCK_A) >= 16000);
}
```

### Test Case 2: One-Shot Timer

```c
void Test_OneShotTimer(void) {
    const Timer_ConfigType cfg = {
        .Module = TIMER_MODULE_1,
        .Block = TIMER_BLOCK_A,
        .ConfigMode = TIMER_CONFIG_32BIT,
        .OperationMode = TIMER_MODE_ONESHOT,
        .LoadValue = 16000,
        .InterruptEnable = FALSE
    };
    
    Timer_Init(&cfg);
    Timer_Start(TIMER_MODULE_1, TIMER_BLOCK_A);
    
    /* Wait for timeout */
    while(Timer_GetState(TIMER_MODULE_1, TIMER_BLOCK_A) == TIMER_STATE_RUNNING);
    
    /* Timer should be stopped */
    assert(Timer_GetState(TIMER_MODULE_1, TIMER_BLOCK_A) == TIMER_STATE_STOPPED);
}
```

---

## Best Practices

### ✅ DO:
- Initialize timer before starting
- Clear interrupts in ISR
- Use appropriate load values for desired timing
- Consider prescaler for long periods
- Test timing with oscilloscope

### ❌ DON'T:
- Start timer before initialization
- Use load value of 0
- Forget to clear interrupt flags
- Use excessive interrupt rates (>10kHz)
- Assume exact timing without verification

---

## Memory Footprint

- **Code Size:** ~2.5 KB
- **RAM Usage:** ~144 bytes (runtime state for 12 timers)
- **ROM:** ~4 KB (with configurations)

---

## Performance

**At 16 MHz:**
- **Minimum Period:** 1 μs (16 clocks)
- **Maximum Period (32-bit):** 268 seconds
- **Maximum Period (16-bit + prescaler):** 1048 seconds (17.5 minutes)
- **ISR Overhead:** ~50-100 cycles

---

## Version History

### v1.0.0 (October 26, 2025)
- ✅ Complete Timer driver implementation
- ✅ Support for 6 modules, 12 independent timers
- ✅ Periodic, one-shot, PWM, capture modes
- ✅ Interrupt support with callbacks
- ✅ DET integration
- ✅ Comprehensive documentation

---

**End of Documentation**
