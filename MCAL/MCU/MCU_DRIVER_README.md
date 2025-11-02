# MCU Driver for TM4C123GH6PM

## Overview
This is an AUTOSAR-compliant MCU (Microcontroller Unit) driver for the Texas Instruments TM4C123GH6PM microcontroller. The driver provides a standardized interface for MCU-related functionalities including clock management, PLL configuration, power modes, reset handling, and peripheral clock control.

## Features
- System clock configuration (up to 80MHz)
- PLL configuration and control
- Peripheral clock gating
- Power mode management
- Reset handling
- Clock source selection
- AUTOSAR 4.4.0 compliance
- Development error tracing support

## File Structure
```
MCU/
├── MCU.c              # MCU driver implementation
├── MCU.h              # MCU driver interface
├── MCU_Types.h        # MCU type definitions
├── Mcu_Cfg.h          # MCU configuration parameters
├── Mcu_PBcfg.c        # MCU post-build configuration
└── MCU_DRIVER_README.md # This file
```

## API Functions

### Initialization
- `void Mcu_Init(const Mcu_ConfigType* ConfigPtr)` - Initialize MCU driver
- `Std_ReturnType Mcu_InitClock(Mcu_ClockType ClockSetting)` - Initialize clock

### PLL Control
- `Std_ReturnType Mcu_DistributePllClock(void)` - Activate PLL clock
- `Mcu_PllStatusType Mcu_GetPllStatus(void)` - Get PLL status

### Reset Handling
- `Mcu_ResetType Mcu_GetResetReason(void)` - Get reset reason
- `uint32 Mcu_GetResetRawValue(void)` - Get reset raw value
- `void Mcu_PerformReset(void)` - Perform microcontroller reset

### Power Management
- `void Mcu_SetMode(Mcu_ModeType McuMode)` - Set MCU power mode
- `uint32 Mcu_GetSystemClock(void)` - Get current system clock frequency

### Peripheral Control
- `void Mcu_EnablePeripheralClock(uint32 Peripheral)` - Enable peripheral clock
- `void Mcu_DisablePeripheralClock(uint32 Peripheral)` - Disable peripheral clock

## Configuration Options

### Clock Settings
- `MCU_CLOCK_MOSC_16MHZ` - Main oscillator 16MHz (default)
- `MCU_CLOCK_PLL_80MHZ` - PLL 80MHz
- `MCU_CLOCK_PLL_50MHZ` - PLL 50MHz
- `MCU_CLOCK_PLL_40MHZ` - PLL 40MHz
- `MCU_CLOCK_PLL_25MHZ` - PLL 25MHz
- `MCU_CLOCK_PIOSC_16MHZ` - Precision internal oscillator 16MHz

### Power Modes
- `MCU_MODE_NORMAL` - Normal run mode
- `MCU_MODE_SLEEP` - Sleep mode (CPU halted, peripherals active)
- `MCU_MODE_DEEP_SLEEP` - Deep sleep mode (CPU and most peripherals halted)

## Usage Example

```c
#include "MCU.h"
#include "Mcu_Cfg.h"

int main(void)
{
    /* Initialize MCU with default configuration */
    Mcu_Init(&Mcu_Config);
    
    /* Set system clock to 80MHz using PLL */
    Mcu_InitClock(MCU_CLOCK_PLL_80MHZ);
    
    /* Enable GPIO Port F clock */
    Mcu_EnablePeripheralClock(0x6080020UL);
    
    /* Your application code here */
    
    return 0;
}
```

## Development Error Tracing
The driver supports DET (Development Error Tracer) for debugging:
- `MCU_E_PARAM_CONFIG` - Invalid configuration parameter
- `MCU_E_PARAM_CLOCK` - Invalid clock setting
- `MCU_E_PARAM_MODE` - Invalid power mode
- `MCU_E_PLL_NOT_LOCKED` - PLL not locked when required
- `MCU_E_UNINIT` - Module not initialized
- `MCU_E_PARAM_POINTER` - NULL pointer parameter

## Dependencies
- Std_Types.h - Standard AUTOSAR types
- Det.h - Development Error Tracer
- Platform_Types.h - Platform-specific types

## Hardware Support
- Texas Instruments TM4C123GH6PM microcontroller
- ARM Cortex-M4F processor
- System Control registers
- PLL configuration registers
- Reset control registers
- Peripheral clock gating registers

## Compliance
This driver is compliant with:
- AUTOSAR 4.4.0 specification
- MISRA C:2012 guidelines (where applicable)
- ISO 26262 functional safety standards (where applicable)

## Version Information
- Version: 1.0.0
- Date: Nov 1, 2025
- Author: Mohamed Yasser

## Notes
- The driver assumes a 16MHz external crystal
- PLL configuration values are optimized for specific frequencies
- Power mode transitions may require additional hardware configuration
- Peripheral clock control uses direct register manipulation