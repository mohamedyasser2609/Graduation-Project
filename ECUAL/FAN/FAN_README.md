# Fan Control Driver

## Overview
AUTOSAR-compliant driver for PWM-controlled cooling fans, designed for Delta FFB0812EHE high-speed fans used in the robot's active cooling system.

## Supported Hardware
- **Delta FFB0812EHE** 80mm High-Speed Fan
  - 12V DC, 1.35A max
  - 12000 RPM max, 86.7 CFM
  - 4-wire: VCC, GND, Tach, PWM

## Features
- Multi-fan support (up to 4 fans)
- PWM speed control (0-100%)
- Minimum duty enforcement for reliable startup
- Optional tachometer RPM feedback
- Stall detection (if tach enabled)
- Fault notification callback
- MISRA C:2012 compliant

## Hardware Connection

```
TM4C123GH6PM               Delta FFB0812EHE
    PB6 (M0PWM0) <---------> PWM (Blue) - Fan 0
    PB7 (M0PWM1) <---------> PWM (Blue) - Fan 1
    GND <-------------------> GND (Black)
    
External 12V Supply
    12V <-------------------> VCC (Red)
    GND <-------------------> GND (Black)
```

> **Note:** The fan requires 12V power supply. PWM signal is 5V-tolerant. Tach output (Yellow) is open-drain, requires pull-up.

## API Reference

| Function | Description |
|----------|-------------|
| `Fan_Init()` | Initialize driver |
| `Fan_SetSpeed()` | Set individual fan speed (0-100%) |
| `Fan_SetAllSpeed()` | Set all fans to same speed |
| `Fan_Stop()` | Stop a specific fan |
| `Fan_StopAll()` | Emergency stop all fans |
| `Fan_GetSpeed()` | Get current speed setting |
| `Fan_GetState()` | Get fan state |
| `Fan_SetFaultCallback()` | Register fault handler |

## Usage Example

```c
#include "ECUAL/FAN/FAN.h"

extern const Fan_ConfigType Fan_Config;

void Fan_FaultHandler(Fan_IdType FanId, Fan_StateType State)
{
    if (State == FAN_STATE_STALLED) {
        /* Log fault, try restart */
    }
}

void main(void)
{
    Pwm_Init(&Pwm_Config);
    Fan_Init(&Fan_Config);
    Fan_SetFaultCallback(Fan_FaultHandler);
    
    /* Set moderate speed */
    Fan_SetAllSpeed(50);
    
    /* Temperature-based control */
    if (temperature > 60.0f) {
        Fan_SetAllSpeed(100);  /* Full speed */
    } else if (temperature < 40.0f) {
        Fan_SetAllSpeed(30);   /* Quiet mode */
    }
}
```

## PWM Control
- PWM frequency: 25kHz (Delta spec)
- Minimum duty: 20% (for reliable startup)
- Speed range: 0-100% linearly maps to 0-12000 RPM

## Configuration
Edit `FAN_PBCfg.c` to modify:
- PWM channel assignments
- Tachometer channel (if used)
- Minimum duty cycle
- Maximum RPM rating
