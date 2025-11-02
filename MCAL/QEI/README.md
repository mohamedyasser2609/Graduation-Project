# Quadrature Encoder Interface (QEI) Driver (TM4C123GH6PM)

## Overview
The QEI driver implements AUTOSAR-style services for configuring and reading the TM4C123GH6PM quadrature encoder modules. It supports both QEI0 and QEI1, including position tracking, velocity capture, and interrupt callbacks.

## Included Files
- `QEI_Types.h` – module identifiers, configuration structures, and auxiliary enums.
- `QEI_Cfg.h` / `QEI_PBCfg.c` – compile-time switches and example post-build configuration.
- `QEI.h` / `QEI.c` – public API and driver logic.

## Key Features
- Initialization of QEI hardware with configurable signal, reset, and filter modes.
- Position counter setup with programmable maximum and initial values.
- Optional velocity capture with selectable pre-divider and timer window.
- Interrupt enable/disable, clear, and callback notification support.
- Direction sensing, error detection, and debug stall capability.

## Configuration Workflow
1. Define a `Qei_ConfigType` instance in `QEI_PBCfg.c` describing the module, signal options, velocity period, and interrupts.
2. Set compile-time flags (`QEI_DEV_ERROR_DETECT`, `QEI_VERSION_INFO_API`, `QEI_ALLOW_DEBUG_STALL`) in `QEI_Cfg.h`.
3. Provide the configuration to the application via `extern const Qei_ConfigType Qei_Config;`.

## Typical Initialization
```c
#include "QEI.h"
#include "QEI_PBCfg.h"

Qei_Init(&Qei_Config);
```
The driver enables module clocks, programs control registers, loads position/velocity registers, applies interrupt masks, and starts the encoder.

## Runtime API Highlights
- `uint32 Qei_GetPosition(void);`
- `void Qei_SetPosition(uint32 Position);`
- `uint32 Qei_GetVelocity(void);` *(requires velocity capture enabled)*
- `Qei_DirectionType Qei_GetDirection(void);`
- `void Qei_EnableInterrupt(Qei_InterruptMaskType mask);`
- `void Qei_DisableInterrupt(Qei_InterruptMaskType mask);`
- `void Qei_ClearInterrupt(Qei_InterruptMaskType mask);`
- `void Qei_DeInit(void);`

Interrupt service routines delegate to `Qei_Qei0Handler` / `Qei_Qei1Handler`, which clear source flags and invoke the user callback if provided.

## Error Handling
With `QEI_DEV_ERROR_DETECT` set to `STD_ON`, DET detects:
- Null configuration pointers.
- Re-initialization attempts.
- Invalid module or position values.
- Velocity queries when velocity capture is disabled.
- API usage prior to `Qei_Init`.

## Integration Tips
- Ensure the MCU driver enables the GPIO peripherals used by QEI signals (PhA/PhB/IDX) and configures pin muxing.
- For absolute positioning, periodically store the position counter or handle index resets appropriately.
- Choose velocity window (`VelocityTimerLoad`) to balance responsiveness and resolution for your encoder counts.

## Version Information
Version macros are defined in `QEI.h` and exposed through `Qei_GetVersionInfo()` when the API is enabled.
