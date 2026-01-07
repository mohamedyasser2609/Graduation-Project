/**
 * @file AM2320_PBCfg.c
 * @brief Post-build configuration for AM2320 Temperature & Humidity Sensor
 * @details Configuration for 3x AM2320 sensors for thermal monitoring
 *
 * Sensor Placement:
 * - Sensor 0: Motor driver area
 * - Sensor 1: Main board/MCU area  
 * - Sensor 2: Battery compartment
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "AM2320.h"

/* ===================[Sensor Configurations]=================== */
static const AM2320_SensorConfigType AM2320_SensorConfigs[] =
{
    /* Sensor 0: Motor Driver Area */
    {
        .SensorId          = 0u,
        .I2cModule         = 0u,                /* I2C0 */
        .TempOffsetC       = 0.0f,              /* No offset calibration */
        .HumidityOffset    = 0.0f,
        .TempHighThreshold = 70.0f,             /* 70°C high alarm */
        .TempLowThreshold  = 5.0f               /* 5°C low alarm */
    },
    /* Sensor 1: Main Board/MCU Area */
    {
        .SensorId          = 1u,
        .I2cModule         = 0u,                /* I2C0 (via mux or daisy chain) */
        .TempOffsetC       = 0.0f,
        .HumidityOffset    = 0.0f,
        .TempHighThreshold = 65.0f,             /* MCU more sensitive */
        .TempLowThreshold  = 5.0f
    },
    /* Sensor 2: Battery Compartment */
    {
        .SensorId          = 2u,
        .I2cModule         = 0u,                /* I2C0 */
        .TempOffsetC       = 0.0f,
        .HumidityOffset    = 0.0f,
        .TempHighThreshold = 50.0f,             /* Battery more sensitive */
        .TempLowThreshold  = 0.0f               /* Battery min temp */
    }
};

/* ===================[Driver Configuration]=================== */
const AM2320_ConfigType AM2320_Config =
{
    .NumSensors = 3u,
    .Sensors    = AM2320_SensorConfigs
};
