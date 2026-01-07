# Thermal Management Service

## Overview
AUTOSAR-compliant service for active cooling system management. Coordinates AM2320 temperature sensors and Delta FFB0812EHE fans to maintain safe operating temperatures.

## Architecture

```
┌──────────────────────────────────────────────────────┐
│              ThermalMgmt Service                     │
│  ┌─────────────────────────────────────────────────┐ │
│  │              ThermalMgmt_MainFunction()         │ │
│  │  1. Read sensors → 2. Update status → 3. Fan   │ │
│  └─────────────────────────────────────────────────┘ │
├────────────────────┬─────────────────────────────────┤
│   AM2320 Driver    │         Fan Driver             │
│   (3 sensors)      │         (2 fans)               │
└────────────────────┴─────────────────────────────────┘
```

## Thermal Zones

| Zone | Location | Low | Med | High | Critical |
|------|----------|-----|-----|------|----------|
| Motors | Motor drivers | 35°C | 50°C | 65°C | 80°C |
| MCU | Main board | 30°C | 45°C | 60°C | 75°C |
| Battery | Battery area | 25°C | 35°C | 45°C | 55°C |

## Operating Modes

| Mode | Behavior |
|------|----------|
| `OFF` | Fans disabled |
| `PASSIVE` | Minimum speed (30%) |
| `AUTO` | Temperature-based control |
| `AGGRESSIVE` | Maximum speed (100%) |
| `MANUAL` | User-controlled speed |

## API Reference

| Function | Description |
|----------|-------------|
| `ThermalMgmt_Init()` | Initialize service |
| `ThermalMgmt_MainFunction()` | Periodic update (call every 1s) |
| `ThermalMgmt_SetMode()` | Set operating mode |
| `ThermalMgmt_GetStatus()` | Get thermal status |
| `ThermalMgmt_GetData()` | Get all thermal data |
| `ThermalMgmt_EmergencyCooling()` | Force max cooling |

## Usage Example

```c
#include "SERVICES/THERMAL/ThermalMgmt.h"

extern const ThermalMgmt_ConfigType ThermalMgmt_Config;

void ThermalEventHandler(ThermalMgmt_StatusType Status,
                         ThermalMgmt_ZoneType Zone, float32 Temp)
{
    if (Status == THERMALMGMT_STATUS_SHUTDOWN) {
        /* Reduce motor power, log event */
    }
}

void main(void)
{
    AM2320_Init(&AM2320_Config);
    Fan_Init(&Fan_Config);
    ThermalMgmt_Init(&ThermalMgmt_Config);
    ThermalMgmt_SetEventCallback(ThermalEventHandler);
    
    /* In FreeRTOS task or timer: */
    while (1) {
        ThermalMgmt_MainFunction();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

## Integration
Called by FreeRTOS task for periodic thermal monitoring and cooling control.
