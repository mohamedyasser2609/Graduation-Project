# Watchdog (WDG) Driver (TM4C123GH6PM)

## Overview
The Watchdog driver delivers an AUTOSAR-aligned abstraction for the TM4C123GH6PM watchdog timers. It supports both watchdog instances, enabling reset and interrupt modes, configurable timeouts, and DET-based error handling.

## Included Files
- `WDG_Types.h` – instance identifiers, trigger modes, status definitions, and configuration structures.
- `WDG_Cfg.h` / `WDG_PBCfg.c` – compile-time options and example post-build configuration.
- `WDG.h` / `WDG.c` – public API and implementation.

## Key Features
- Supports Watchdog 0 and Watchdog 1, including clock gating.
- Configurable reset and/or interrupt response on timeout.
- Adjustable timeout window via `Wdg_SetTriggerCondition`.
- Run-time trigger mode selection (normal vs fast).
- Optional debug stall, interrupt notifications, and version info API.

## Configuration Workflow
1. Populate `Wdg_Config` in `WDG_PBCfg.c` with instance selection, timeout bounds, and behavior flags.
2. Set compile-time switches (`WDG_DEV_ERROR_DETECT`, `WDG_VERSION_INFO_API`, etc.) in `WDG_Cfg.h`.
3. Expose `extern const Wdg_ConfigType Wdg_Config;` for application initialization.

## Typical Initialization
```c
#include "WDG.h"
#include "WDG_PBCfg.h"

Wdg_Init(&Wdg_Config);
```
This unlocks the watchdog, programs load/reset settings, enables reset/interrupt control, and (optionally) stalls during debug sessions.

## Runtime Control
- `void Wdg_SetTriggerMode(Wdg_TriggerModeType Mode);`
- `void Wdg_SetTriggerCondition(uint32 TimeoutTicks);`
- `void Wdg_Service(void);`
- `void Wdg_Disable(void);`
- `Wdg_StatusType Wdg_GetStatus(void);`

Use `Wdg_Service` periodically (before timeout expires) to prevent resets when operating in interrupt or reset mode.

## Interrupt Handling
If `InterruptEnable` is `TRUE`, and a notification callback is provided, the driver clears the interrupt and executes the callback from `Wdg_Watchdog0Handler` / `Wdg_Watchdog1Handler`. Hook these handlers into the vector table or NVIC to receive notifications.

## Error Handling
With `WDG_DEV_ERROR_DETECT` enabled, DET catches:
- Null configuration or version pointers.
- Re-initialization attempts.
- Timeout values outside configured min/max bounds.
- API usage before `Wdg_Init`.

## Integration Tips
- Ensure system clock frequency used to compute timeout ticks matches the currently active MCU clock.
- When using reset mode, confirm system reset handling sequences are prepared for watchdog-triggered resets.
- For debug sessions, enable stall to avoid unwanted resets while halting at breakpoints.

## Version Information
Version macros live in `WDG.h` and can be queried via `Wdg_GetVersionInfo()` when enabled.
