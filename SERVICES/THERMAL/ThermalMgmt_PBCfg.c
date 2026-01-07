/**
 * @file ThermalMgmt_PBCfg.c
 * @brief Post-build configuration for Thermal Management Service
 * @details Configuration for 3 thermal zones and 2 fans
 *
 * Zone Configuration:
 * - Zone 0 (Motors): Sensor 0, highest temp tolerance
 * - Zone 1 (MCU): Sensor 1, moderate tolerance
 * - Zone 2 (Battery): Sensor 2, lowest temp tolerance
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "ThermalMgmt.h"

/* ===================[Zone Configurations]=================== */
static const ThermalMgmt_ZoneConfigType ThermalMgmt_ZoneConfigs[] =
{
    /* Zone 0: Motor Driver Area */
    {
        .Zone       = THERMALMGMT_ZONE_MOTORS,
        .SensorId   = 0u,
        .Thresholds = {
            .LowThreshold       = THERMALMGMT_MOTOR_LOW_THRESH,
            .MediumThreshold    = THERMALMGMT_MOTOR_MED_THRESH,
            .HighThreshold      = THERMALMGMT_MOTOR_HIGH_THRESH,
            .CriticalThreshold  = THERMALMGMT_MOTOR_CRIT_THRESH
        },
        .Priority   = 1u
    },
    /* Zone 1: MCU/Main Board Area */
    {
        .Zone       = THERMALMGMT_ZONE_MCU,
        .SensorId   = 1u,
        .Thresholds = {
            .LowThreshold       = THERMALMGMT_MCU_LOW_THRESH,
            .MediumThreshold    = THERMALMGMT_MCU_MED_THRESH,
            .HighThreshold      = THERMALMGMT_MCU_HIGH_THRESH,
            .CriticalThreshold  = THERMALMGMT_MCU_CRIT_THRESH
        },
        .Priority   = 0u    /* Highest priority - protect MCU */
    },
    /* Zone 2: Battery Compartment */
    {
        .Zone       = THERMALMGMT_ZONE_BATTERY,
        .SensorId   = 2u,
        .Thresholds = {
            .LowThreshold       = THERMALMGMT_BATT_LOW_THRESH,
            .MediumThreshold    = THERMALMGMT_BATT_MED_THRESH,
            .HighThreshold      = THERMALMGMT_BATT_HIGH_THRESH,
            .CriticalThreshold  = THERMALMGMT_BATT_CRIT_THRESH
        },
        .Priority   = 2u
    }
};

/* ===================[Fan Configurations]=================== */
static const ThermalMgmt_FanConfigType ThermalMgmt_FanConfigs[] =
{
    /* Fan 0: Intake */
    {
        .FanId      = 0u,
        .MinSpeed   = THERMALMGMT_FAN_MIN_SPEED,
        .MedSpeed   = THERMALMGMT_FAN_MED_SPEED,
        .MaxSpeed   = THERMALMGMT_FAN_MAX_SPEED
    },
    /* Fan 1: Exhaust */
    {
        .FanId      = 1u,
        .MinSpeed   = THERMALMGMT_FAN_MIN_SPEED,
        .MedSpeed   = THERMALMGMT_FAN_MED_SPEED,
        .MaxSpeed   = THERMALMGMT_FAN_MAX_SPEED
    }
};

/* ===================[Service Configuration]=================== */
const ThermalMgmt_ConfigType ThermalMgmt_Config =
{
    .NumZones       = 3u,
    .Zones          = ThermalMgmt_ZoneConfigs,
    .NumFans        = 2u,
    .Fans           = ThermalMgmt_FanConfigs,
    .UpdatePeriodMs = THERMALMGMT_DEFAULT_PERIOD_MS
};
