# MCU Driver (TM4C123GH6PM)

## Overview
The MCU driver provides AUTOSAR-compliant initialization and control of the TM4C123GH6PM microcontroller core clocks and reset resources. It orchestrates PLL setup, system clock switching, peripheral clock gating, and reset cause handling to prepare the device for upper-layer services.

## Included Files
- `MCU_Types.h` – type definitions and enumerations used across the driver.
- `MCU_Cfg.h` / `Mcu_PBCfg.c` – configuration parameters and post-build tables.
- `MCU.h` / `MCU.c` – public API and implementation.

## Key Features
- Clock source selection (PIOSC, MOSC, PLL-based).
- PLL configuration and distribution control.
- Peripheral clock gating helpers for SYSCTL RCGC registers.
- Power/low-power mode transitions via `Mcu_SetMode`.
- Reset cause capture and reporting (`Mcu_GetResetReason`).
- System clock frequency queries for timing services.

## Configuration Workflow
1. Populate `Mcu_ConfigType` in `Mcu_PBCfg.c`, selecting desired clock setting and PLL parameters.
2. Define clock settings and mode configurations in `MCU_Cfg.h` (compile-time switches).
3. Expose the configuration pointer (`extern const Mcu_ConfigType* Mcu_ConfigPtr;`).

## Initialization Sequence
```c
#include "MCU.h"

Mcu_Init(Mcu_ConfigPtr);
Mcu_InitClock(MCU_CLOCK_SETTING_0);
(void)Mcu_DistributePllClock();
```
- `Mcu_Init` loads configuration data and prepares the driver state.
- `Mcu_InitClock` programs RCC/RCC2 and waits for PLL lock (if selected).
- `Mcu_DistributePllClock` switches the system clock to the PLL output when ready.

## API Highlights
- `Std_ReturnType Mcu_InitClock(Mcu_ClockType ClockSetting);`
- `Std_ReturnType Mcu_DistributePllClock(void);`
- `Mcu_PllStatusType Mcu_GetPllStatus(void);`
- `Mcu_ResetType Mcu_GetResetReason(void);`
- `uint32 Mcu_GetSystemClock(void);`
- `void Mcu_EnablePeripheralClock(uint32 Peripheral);`
- `void Mcu_DisablePeripheralClock(uint32 Peripheral);`

Refer to `MCU.h` for the full API list and comments.

## Error Handling
Development Error Tracer (DET) checks guard against:
- Null configuration pointers.
- Invalid clock setting requests.
- Calling APIs before initialization.

Ensure `DET` is configured if `MCU_DEV_ERROR_DETECT` is set to `STD_ON` in `MCU_Cfg.h`.

## Integration Tips
- Enable relevant peripheral clocks via `Mcu_EnablePeripheralClock` before configuring module registers.
- If other drivers depend on PLL frequency, call `Mcu_GetSystemClock` during their initialization to derive timing parameters.
- For low-power entry/exit, coordinate `Mcu_SetMode` with OS or application hooks.

## Version Information
Version numbers are defined at the top of `MCU.h` and exposed through `Std_VersionInfoType` if `MCU_VERSION_INFO_API` is enabled.
