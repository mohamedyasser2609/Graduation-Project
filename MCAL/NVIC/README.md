# NVIC Driver (TM4C123GH6PM)

## Overview
The Nested Vectored Interrupt Controller (NVIC) driver offers an AUTOSAR-compliant abstraction for enabling, prioritizing, and managing interrupts on the TM4C123GH6PM. It configures vector priorities, handles pending/active statuses, and supports software-triggered interrupts.

## Included Files
- `NVIC_Types.h` – vector identifiers, priority group definitions, and configuration structures.
- `NVIC_Cfg.h` / `NVIC_PBCfg.c` – compile-time options and post-build interrupt tables.
- `NVIC.h` / `NVIC.c` – public API and implementation with helper utilities.

## Key Features
- Initialization via configuration table (`NVIC_ConfigType`).
- Interrupt enable/disable control per vector.
- Priority configuration respecting priority grouping.
- Pending flag set/clear and active state queries.
- Software interrupt generation for testing or cross-triggering.
- Optional version info API and DET error reporting.

## Configuration Workflow
1. Populate `NVIC_InterruptConfigTable` in `NVIC_PBCfg.c` with vectors, priorities, and enable state.
2. Set desired priority grouping (`NVIC_DEFAULT_PRIORITY_GROUP`) in `NVIC_Cfg.h`.
3. Expose `extern const NVIC_ConfigType NVIC_Config;` for application use.

## Typical Initialization
```c
#include "NVIC.h"
#include "NVIC_PBCfg.h"

NVIC_Init(&NVIC_Config);
```
This loads priority grouping into `APINT`, applies priorities, and toggles enable bits based on the configuration table.

## API Highlights
- `void NVIC_EnableIRQ(NVIC_InterruptVectorType Vector);`
- `void NVIC_DisableIRQ(NVIC_InterruptVectorType Vector);`
- `void NVIC_SetPriority(NVIC_InterruptVectorType Vector, NVIC_PriorityType Priority);`
- `NVIC_PriorityType NVIC_GetPriority(NVIC_InterruptVectorType Vector);`
- `void NVIC_SetPendingIRQ(NVIC_InterruptVectorType Vector);`
- `void NVIC_ClearPendingIRQ(NVIC_InterruptVectorType Vector);`
- `boolean NVIC_IsActiveIRQ(NVIC_InterruptVectorType Vector);`
- `void NVIC_GenerateSoftwareInterrupt(NVIC_InterruptVectorType Vector);`

## Error Handling
When `NVIC_DEV_ERROR_DETECT` is `STD_ON`, DET reports:
- Null configuration pointers.
- Re-initialization attempts.
- Out-of-range vectors or priorities.
- Using APIs prior to `NVIC_Init`.

## Integration Tips
- Ensure corresponding ISR symbols exist to avoid linker errors when enabling vectors.
- Configure NVIC priorities and enables before peripheral configuration that relies on interrupts.
- Use `NVIC_GenerateSoftwareInterrupt` to emulate hardware triggers during integration tests.

## Version Information
Version macros are defined in `NVIC.h` and exposed through `NVIC_GetVersionInfo()` if enabled.
