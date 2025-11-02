# MCU Driver Usage Guide

## AUTOSAR-Compliant MCU Driver for TM4C123GH6PM

### Features
- ✅ System clock configuration (16MHz to 80MHz)
- ✅ Integrated PLL configuration
- ✅ Power mode management
- ✅ Reset handling
- ✅ Clock source selection
- ✅ Peripheral clock gating

---

## Quick Start

### 1. Basic Initialization (80MHz)

```c
#include "MCAL/MCU/Mcu.h"

int main(void) {
    /* Initialize MCU driver with 80MHz PLL */
    Mcu_Init(&Mcu_Config_80MHz);
    
    /* Initialize clock */
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    
    /* Wait for PLL to lock */
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) {
        /* Wait */
    }
    
    /* Activate PLL clock */
    Mcu_DistributePllClock();
    
    /* Your application code here */
    /* System is now running at 80MHz! */
    
    while(1) {
        /* Main loop */
    }
}
```

### 2. Different Clock Speeds

```c
/* 16MHz (no PLL) */
Mcu_Init(&Mcu_Config_16MHz);
Mcu_InitClock(MCU_CLOCK_16MHZ);

/* 50MHz with PLL */
Mcu_Init(&Mcu_Config_50MHz);
Mcu_InitClock(MCU_CLOCK_50MHZ);
while (Mcu_GetPllStatus() != MCU_PLL_LOCKED);
Mcu_DistributePllClock();
```

### 3. Get System Clock

```c
uint32 systemClock = Mcu_GetSystemClock();
/* Returns: 80000000 (80MHz) */
```

### 4. Power Modes

```c
/* Enter sleep mode (wake on interrupt) */
Mcu_SetMode(MCU_MODE_SLEEP);

/* Enter deep sleep mode */
Mcu_SetMode(MCU_MODE_DEEP_SLEEP);

/* Return to normal mode */
Mcu_SetMode(MCU_MODE_NORMAL);
```

### 5. Reset Functions

```c
/* Get reset reason */
Mcu_ResetType resetCause = Mcu_GetResetReason();

switch (resetCause) {
    case MCU_POWER_ON_RESET:
        /* Power-on reset */
        break;
    case MCU_EXTERNAL_RESET:
        /* External reset button */
        break;
    case MCU_WATCHDOG_RESET:
        /* Watchdog timeout */
        break;
    case MCU_SOFTWARE_RESET:
        /* Software reset */
        break;
}

/* Perform software reset */
Mcu_PerformReset();  /* System will reset */
```

---

## Available Clock Settings

| Setting | Frequency | PLL | Use Case |
|---------|-----------|-----|----------|
| `MCU_CLOCK_16MHZ` | 16 MHz | No | Low power, simple applications |
| `MCU_CLOCK_20MHZ` | 20 MHz | Yes | Moderate performance |
| `MCU_CLOCK_25MHZ` | 25 MHz | Yes | Balanced performance |
| `MCU_CLOCK_40MHZ` | 40 MHz | Yes | Good performance |
| `MCU_CLOCK_50MHZ` | 50 MHz | Yes | High performance |
| `MCU_CLOCK_80MHZ` | 80 MHz | Yes | **Maximum performance** ⚡ |

---

## Configuration Options

### Pre-defined Configurations

```c
/* Available in Mcu_PBCfg.c */
extern const Mcu_ConfigType Mcu_Config_80MHz;  /* Default: 80MHz with PLL */
extern const Mcu_ConfigType Mcu_Config_50MHz;  /* 50MHz with PLL */
extern const Mcu_ConfigType Mcu_Config_16MHz;  /* 16MHz without PLL */
```

### Custom Configuration

```c
const Mcu_ConfigType MyCustomConfig = {
    .DefaultClock = MCU_CLOCK_40MHZ,
    .ClockSource = MCU_CLOCK_SOURCE_MOSC,  /* Main oscillator */
    .PllEnabled = TRUE,
    .NumberOfClockSettings = 6
};

Mcu_Init(&MyCustomConfig);
```

---

## Important Notes

### ⚠️ PLL Usage

When using PLL (any clock > 16MHz):
1. **Always check PLL lock** before distributing clock
2. **Wait for lock** - typically takes a few milliseconds
3. **Don't skip** `Mcu_DistributePllClock()` call

```c
/* CORRECT */
Mcu_InitClock(MCU_CLOCK_80MHZ);
while (Mcu_GetPllStatus() != MCU_PLL_LOCKED);  /* Wait! */
Mcu_DistributePllClock();

/* WRONG - Will fail! */
Mcu_InitClock(MCU_CLOCK_80MHZ);
Mcu_DistributePllClock();  /* PLL not locked yet! */
```

### ⚡ Performance Impact

| Clock | UART Max | I2C Max | PWM Resolution | Power |
|-------|----------|---------|----------------|-------|
| 16MHz | 115200 | 100kHz | Good | Low |
| 40MHz | 460800 | 400kHz | Better | Medium |
| 80MHz | 921600 | 400kHz | **Best** | High |

### 🔧 Updating Existing Drivers

After changing system clock, update baud rate divisors:
- **UART**: Recalculate BRD based on new clock
- **I2C**: Recalculate TPR for desired speed
- **Timers**: Adjust prescalers

---

## API Reference

### Initialization
- `void Mcu_Init(const Mcu_ConfigType* ConfigPtr)`
- `Std_ReturnType Mcu_InitClock(Mcu_ClockType ClockSetting)`

### PLL Control
- `Std_ReturnType Mcu_DistributePllClock(void)`
- `Mcu_PllStatusType Mcu_GetPllStatus(void)`

### System Info
- `uint32 Mcu_GetSystemClock(void)`
- `Mcu_ResetType Mcu_GetResetReason(void)`
- `uint32 Mcu_GetResetRawValue(void)`

### Control
- `void Mcu_PerformReset(void)`
- `void Mcu_SetMode(Mcu_ModeType McuMode)`
- `void Mcu_EnablePeripheralClock(uint32 Peripheral)`
- `void Mcu_DisablePeripheralClock(uint32 Peripheral)`

---

## Troubleshooting

### PLL Won't Lock
- Check crystal is 16MHz
- Verify power supply is stable
- Ensure proper grounding

### System Unstable at 80MHz
- Check power supply can provide enough current
- Verify decoupling capacitors are present
- Try lower frequency (50MHz or 40MHz)

### Reset Loop
- Check `Mcu_GetResetReason()` to identify cause
- Verify watchdog is not enabled unintentionally
- Check for stack overflow or memory corruption

---

## Example: Complete System Initialization

```c
#include "MCAL/MCU/Mcu.h"
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/UART/Uart.h"

int main(void) {
    /* Step 1: Initialize MCU to 80MHz */
    Mcu_Init(&Mcu_Config_80MHz);
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    
    /* Step 2: Wait for PLL lock */
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED);
    
    /* Step 3: Activate PLL */
    Mcu_DistributePllClock();
    
    /* Step 4: Initialize other drivers */
    /* Note: UART baud rates now calculated from 80MHz! */
    Gpio_Init(&Gpio_Configuration);
    Uart_Init(&Uart_Config);
    
    /* Step 5: Check reset reason */
    Mcu_ResetType reset = Mcu_GetResetReason();
    if (reset == MCU_WATCHDOG_RESET) {
        /* Handle watchdog reset */
    }
    
    /* Step 6: Application code */
    while(1) {
        /* Running at 80MHz! */
    }
}
```

---

**Your system is now running at maximum performance!** 🚀⚡
