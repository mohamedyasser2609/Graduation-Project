# ACS712 Current Sensor Driver

## Overview
AUTOSAR-compliant driver for ACS712 Hall-effect current sensors, designed for motor overload protection on the robot controller.

## Supported Hardware
- ACS712-5A (185 mV/A sensitivity)
- ACS712-20A (100 mV/A sensitivity)
- **ACS712-30A** (66 mV/A sensitivity) - Used in this project

## Features
- Multi-channel support (up to 4 sensors)
- Configurable overload thresholds with callback notification
- Averaging filter for noise reduction
- Zero-current calibration support
- MISRA C:2012 compliant

## Hardware Connection

```
TM4C123GH6PM                ACS712-30A
    PE3 (AIN0) <------------ OUT (Left Motor)
    PE2 (AIN1) <------------ OUT (Right Motor)
    GND <-------------------- GND
                              VCC -- 5V
                              IP+ -- Motor Power+
                              IP- -- Motor Power-
```

## API Reference

| Function | Description |
|----------|-------------|
| `ACS712_Init()` | Initialize driver with configuration |
| `ACS712_ReadCurrent()` | Read current from single channel |
| `ACS712_ReadAllChannels()` | Read all configured channels |
| `ACS712_IsOverload()` | Check overload status |
| `ACS712_CalibrateZero()` | Calibrate zero-current offset |
| `ACS712_SetOverloadCallback()` | Register overload notification |

## Usage Example

```c
#include "ECUAL/CURRENT_SENSOR/ACS712.h"

extern const ACS712_ConfigType ACS712_Config;

void Motor_OverloadHandler(ACS712_ChannelType Channel, float32 Current)
{
    Motor_Stop(Channel);  /* Emergency stop */
}

void main(void)
{
    ACS712_DataType currentData;
    
    Adc_Init(&Adc_Config);
    ACS712_Init(&ACS712_Config);
    ACS712_SetOverloadCallback(Motor_OverloadHandler);
    
    /* Calibrate with no current flowing */
    ACS712_CalibrateZero(0);
    ACS712_CalibrateZero(1);
    
    while (1)
    {
        if (ACS712_ReadCurrent(0, &currentData) == E_OK)
        {
            /* currentData.CurrentAmps contains current in Amps */
        }
    }
}
```

## Current Calculation
```
Current (A) = (Vout - Vzero) / Sensitivity
            = (Vout - 2.5V) / 0.066 V/A   (for 30A model)
```

## Configuration
Edit `ACS712_PBCfg.c` to modify:
- ADC channel assignments
- Overload thresholds
- Filter sample count
- Sensor model selection
