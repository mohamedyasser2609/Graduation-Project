# AM2320 Temperature & Humidity Sensor Driver

## Overview
AUTOSAR-compliant driver for ASAIR AM2320 calibrated digital temperature and humidity sensors, used for robot thermal monitoring and cooling system control.

## Supported Hardware
- **AM2320** Digital Temperature & Humidity Sensor (ASAIR)

## Specifications
- Temperature Range: -40°C to +80°C (±0.5°C accuracy)
- Humidity Range: 0-99.9% RH (±3% accuracy)
- Interface: I2C (address 0x5C)
- Supply: 3.1V - 5.5V

## Features
- Multi-sensor support (up to 4 sensors)
- CRC-16 Modbus validation
- Temperature alarm thresholds with callback
- Average/maximum temperature calculation
- Retry mechanism for communication errors
- MISRA C:2012 compliant

## Hardware Connection

```
TM4C123GH6PM               AM2320
    PB2 (I2C0 SCL) <------> SCL
    PB3 (I2C0 SDA) <------> SDA
    3.3V <------------------> VDD
    GND <-------------------> GND
```

> **Note:** All three sensors share the same I2C address. Use an I2C multiplexer (TCA9548A) or connect sensors to different I2C modules (I2C0, I2C1, I2C2).

## API Reference

| Function | Description |
|----------|-------------|
| `AM2320_Init()` | Initialize driver |
| `AM2320_Read()` | Read temp/humidity from sensor |
| `AM2320_ReadAllSensors()` | Read all configured sensors |
| `AM2320_ReadTemperature()` | Read temperature only |
| `AM2320_GetAverageTemperature()` | Calculate average temp |
| `AM2320_GetMaxTemperature()` | Get hottest sensor reading |
| `AM2320_SetAlarmCallback()` | Register temp alarm handler |

## Usage Example

```c
#include "ECUAL/TEMP_SENSOR/AM2320.h"

extern const AM2320_ConfigType AM2320_Config;

void Temp_AlarmHandler(AM2320_SensorType Sensor, float32 Temp, boolean IsHigh)
{
    if (IsHigh) {
        Fan_SetSpeed(100);  /* Maximum cooling */
    }
}

void main(void)
{
    AM2320_DataType tempData[3];
    float32 avgTemp;
    
    I2c_Init(&I2c_Config);
    AM2320_Init(&AM2320_Config);
    AM2320_SetAlarmCallback(Temp_AlarmHandler);
    
    while (1)
    {
        /* Read all sensors */
        if (AM2320_ReadAllSensors(tempData) == E_OK)
        {
            /* Process readings */
        }
        
        /* Get average for cooling control */
        if (AM2320_GetAverageTemperature(&avgTemp) == E_OK)
        {
            /* Adjust fan speed based on avgTemp */
        }
    }
}
```

## Communication Protocol
1. Wake sensor: I2C start + address (no ACK expected)
2. Wait 800µs
3. Send read command: [0x03, 0x00, 0x04]
4. Wait 1.5ms for measurement
5. Read 8 bytes: [func, len, hum_H, hum_L, temp_H, temp_L, crc_L, crc_H]

## Configuration
Edit `AM2320_PBCfg.c` to modify:
- I2C module assignments
- Temperature alarm thresholds
- Calibration offsets
