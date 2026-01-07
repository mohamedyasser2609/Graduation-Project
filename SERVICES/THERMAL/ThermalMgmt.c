/**
 * @file ThermalMgmt.c
 * @brief Thermal Management Service Implementation
 * @details Coordinates temperature sensors and fans for active cooling
 *
 * Control Algorithm:
 * 1. Read all temperature sensors
 * 2. Find maximum temperature across all zones
 * 3. Determine required fan speed based on thresholds
 * 4. Apply fan speed with hysteresis to prevent oscillation
 * 5. Report status changes via callback
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#include "ThermalMgmt.h"
#include "../../ECUAL/TEMP_SENSOR/AM2320.h"
#include "../../ECUAL/FAN/FAN.h"

#if (THERMALMGMT_DEV_ERROR_DETECT == STD_ON)
#include "../../CONFIG/Det.h"
#endif

/* ===================[Private Variables]=================== */
static const ThermalMgmt_ConfigType* ThermalMgmt_ConfigPtr = NULL_PTR;
static ThermalMgmt_StatusType ThermalMgmt_Status = THERMALMGMT_STATUS_UNINIT;
static ThermalMgmt_ModeType ThermalMgmt_CurrentMode = THERMALMGMT_MODE_AUTO;
static ThermalMgmt_DataType ThermalMgmt_CurrentData;
static uint8 ThermalMgmt_CurrentFanSpeed = 0u;
static boolean ThermalMgmt_ShutdownRequired = FALSE;

#if (THERMALMGMT_EVENT_CALLBACK_API == STD_ON)
static ThermalMgmt_EventCallbackType ThermalMgmt_EventCallback = NULL_PTR;
#endif

/* ===================[Private Function Declarations]=================== */
static void ThermalMgmt_ReadAllZones(void);
static uint8 ThermalMgmt_CalculateFanSpeed(float32 MaxTemp);
static void ThermalMgmt_UpdateStatus(float32 MaxTemp, ThermalMgmt_ZoneType HottestZone);
static void ThermalMgmt_ApplyFanSpeed(uint8 Speed);
static void ThermalMgmt_ReportEvent(ThermalMgmt_StatusType Status, 
                                     ThermalMgmt_ZoneType Zone, float32 Temp);

/* ===================[Private Function Implementations]=================== */

/**
 * @brief Read temperature from all configured zones
 */
static void ThermalMgmt_ReadAllZones(void)
{
    uint8 zoneIdx;
    const ThermalMgmt_ZoneConfigType* zoneCfg;
    AM2320_DataType sensorData;
    float32 maxTemp = -100.0f;
    ThermalMgmt_ZoneType hottestZone = THERMALMGMT_ZONE_MOTORS;
    
    for (zoneIdx = 0u; zoneIdx < ThermalMgmt_ConfigPtr->NumZones; zoneIdx++)
    {
        zoneCfg = &ThermalMgmt_ConfigPtr->Zones[zoneIdx];
        
        if (AM2320_Read(zoneCfg->SensorId, &sensorData) == E_OK)
        {
            ThermalMgmt_CurrentData.ZoneTemperatures[zoneCfg->Zone] = sensorData.TemperatureC;
            
            if (sensorData.TemperatureC > maxTemp)
            {
                maxTemp = sensorData.TemperatureC;
                hottestZone = zoneCfg->Zone;
            }
        }
        else
        {
            /* Sensor error - assume fail-safe high temperature */
            ThermalMgmt_CurrentData.ZoneTemperatures[zoneCfg->Zone] = THERMALMGMT_FAIL_SAFE_TEMP_C;
            
            if (THERMALMGMT_FAIL_SAFE_TEMP_C > maxTemp)
            {
                maxTemp = THERMALMGMT_FAIL_SAFE_TEMP_C;
                hottestZone = zoneCfg->Zone;
            }
        }
    }
    
    ThermalMgmt_CurrentData.MaxTemperature = maxTemp;
    ThermalMgmt_CurrentData.HottestZone = hottestZone;
}

/**
 * @brief Calculate required fan speed based on temperature
 */
static uint8 ThermalMgmt_CalculateFanSpeed(float32 MaxTemp)
{
    uint8 zoneIdx;
    uint8 requiredSpeed = THERMALMGMT_FAN_OFF_SPEED;
    const ThermalMgmt_ZoneConfigType* zoneCfg;
    float32 zoneTemp;
    float32 hysteresis = THERMALMGMT_HYSTERESIS_C;
    
    /* Check each zone and determine max required speed */
    for (zoneIdx = 0u; zoneIdx < ThermalMgmt_ConfigPtr->NumZones; zoneIdx++)
    {
        zoneCfg = &ThermalMgmt_ConfigPtr->Zones[zoneIdx];
        zoneTemp = ThermalMgmt_CurrentData.ZoneTemperatures[zoneCfg->Zone];
        
        /* Apply hysteresis based on current speed */
        if (ThermalMgmt_CurrentFanSpeed >= THERMALMGMT_FAN_MAX_SPEED)
        {
            /* Already at max, use lower thresholds to come down */
            hysteresis = THERMALMGMT_HYSTERESIS_C;
        }
        else
        {
            hysteresis = 0.0f;
        }
        
        /* Determine speed based on temperature vs thresholds */
        if (zoneTemp >= (zoneCfg->Thresholds.CriticalThreshold - hysteresis))
        {
            requiredSpeed = THERMALMGMT_FAN_MAX_SPEED;
            break;  /* Critical - max speed immediately */
        }
        else if (zoneTemp >= (zoneCfg->Thresholds.HighThreshold - hysteresis))
        {
            if (THERMALMGMT_FAN_MAX_SPEED > requiredSpeed)
            {
                requiredSpeed = THERMALMGMT_FAN_MAX_SPEED;
            }
        }
        else if (zoneTemp >= (zoneCfg->Thresholds.MediumThreshold - hysteresis))
        {
            if (THERMALMGMT_FAN_MED_SPEED > requiredSpeed)
            {
                requiredSpeed = THERMALMGMT_FAN_MED_SPEED;
            }
        }
        else if (zoneTemp >= (zoneCfg->Thresholds.LowThreshold - hysteresis))
        {
            if (THERMALMGMT_FAN_MIN_SPEED > requiredSpeed)
            {
                requiredSpeed = THERMALMGMT_FAN_MIN_SPEED;
            }
        }
        else
        {
            /* Below low threshold - no cooling needed for this zone */
        }
    }
    
    return requiredSpeed;
}

/**
 * @brief Update thermal status based on temperatures
 */
static void ThermalMgmt_UpdateStatus(float32 MaxTemp, ThermalMgmt_ZoneType HottestZone)
{
    ThermalMgmt_StatusType previousStatus = ThermalMgmt_Status;
    ThermalMgmt_StatusType newStatus = THERMALMGMT_STATUS_NORMAL;
    const ThermalMgmt_ZoneConfigType* zoneCfg = NULL_PTR;
    uint8 zoneIdx;
    
    /* Find the configuration for the hottest zone */
    for (zoneIdx = 0u; zoneIdx < ThermalMgmt_ConfigPtr->NumZones; zoneIdx++)
    {
        if (ThermalMgmt_ConfigPtr->Zones[zoneIdx].Zone == HottestZone)
        {
            zoneCfg = &ThermalMgmt_ConfigPtr->Zones[zoneIdx];
            break;
        }
    }
    
    if (zoneCfg != NULL_PTR)
    {
        if (MaxTemp >= zoneCfg->Thresholds.CriticalThreshold)
        {
            newStatus = THERMALMGMT_STATUS_SHUTDOWN;
            ThermalMgmt_ShutdownRequired = TRUE;
        }
        else if (MaxTemp >= zoneCfg->Thresholds.HighThreshold)
        {
            newStatus = THERMALMGMT_STATUS_CRITICAL;
            ThermalMgmt_ShutdownRequired = FALSE;
        }
        else if (MaxTemp >= zoneCfg->Thresholds.MediumThreshold)
        {
            newStatus = THERMALMGMT_STATUS_WARNING;
            ThermalMgmt_ShutdownRequired = FALSE;
        }
        else
        {
            newStatus = THERMALMGMT_STATUS_NORMAL;
            ThermalMgmt_ShutdownRequired = FALSE;
        }
    }
    
    ThermalMgmt_Status = newStatus;
    ThermalMgmt_CurrentData.OverallStatus = newStatus;
    
    /* Report status change */
    if (newStatus != previousStatus)
    {
        ThermalMgmt_ReportEvent(newStatus, HottestZone, MaxTemp);
    }
}

/**
 * @brief Apply fan speed to all fans
 */
static void ThermalMgmt_ApplyFanSpeed(uint8 Speed)
{
    uint8 fanIdx;
    
    for (fanIdx = 0u; fanIdx < ThermalMgmt_ConfigPtr->NumFans; fanIdx++)
    {
        (void)Fan_SetSpeed(ThermalMgmt_ConfigPtr->Fans[fanIdx].FanId, Speed);
    }
    
    ThermalMgmt_CurrentFanSpeed = Speed;
    ThermalMgmt_CurrentData.CurrentFanSpeed = Speed;
}

/**
 * @brief Report thermal event via callback
 */
static void ThermalMgmt_ReportEvent(ThermalMgmt_StatusType Status, 
                                     ThermalMgmt_ZoneType Zone, float32 Temp)
{
    #if (THERMALMGMT_EVENT_CALLBACK_API == STD_ON)
    if (ThermalMgmt_EventCallback != NULL_PTR)
    {
        ThermalMgmt_EventCallback(Status, Zone, Temp);
    }
    #else
    (void)Status;
    (void)Zone;
    (void)Temp;
    #endif
}

/* ===================[Public Function Implementations]=================== */

/**
 * @brief Initialize the Thermal Management service
 */
void ThermalMgmt_Init(const ThermalMgmt_ConfigType* ConfigPtr)
{
    uint8 zoneIdx;
    
#if (THERMALMGMT_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(THERMALMGMT_MODULE_ID, THERMALMGMT_INSTANCE_ID, 
                       THERMALMGMT_INIT_SID, THERMALMGMT_E_PARAM_POINTER);
        return;
    }
    
    if (ThermalMgmt_Status != THERMALMGMT_STATUS_UNINIT)
    {
        Det_ReportError(THERMALMGMT_MODULE_ID, THERMALMGMT_INSTANCE_ID, 
                       THERMALMGMT_INIT_SID, THERMALMGMT_E_ALREADY_INIT);
        return;
    }
#endif
    
    ThermalMgmt_ConfigPtr = ConfigPtr;
    
    /* Initialize data structure */
    for (zoneIdx = 0u; zoneIdx < (uint8)THERMALMGMT_ZONE_COUNT; zoneIdx++)
    {
        ThermalMgmt_CurrentData.ZoneTemperatures[zoneIdx] = 0.0f;
    }
    ThermalMgmt_CurrentData.MaxTemperature = 0.0f;
    ThermalMgmt_CurrentData.HottestZone = THERMALMGMT_ZONE_MOTORS;
    ThermalMgmt_CurrentData.CurrentFanSpeed = 0u;
    ThermalMgmt_CurrentData.OverallStatus = THERMALMGMT_STATUS_NORMAL;
    
    ThermalMgmt_CurrentFanSpeed = 0u;
    ThermalMgmt_ShutdownRequired = FALSE;
    ThermalMgmt_CurrentMode = THERMALMGMT_MODE_AUTO;
    
    ThermalMgmt_Status = THERMALMGMT_STATUS_NORMAL;
}

/**
 * @brief Main function - call periodically
 */
void ThermalMgmt_MainFunction(void)
{
    uint8 requiredSpeed;
    
    if (ThermalMgmt_Status == THERMALMGMT_STATUS_UNINIT)
    {
        return;
    }
    
    /* Step 1: Read all temperature zones */
    ThermalMgmt_ReadAllZones();
    
    /* Step 2: Update status */
    ThermalMgmt_UpdateStatus(ThermalMgmt_CurrentData.MaxTemperature, 
                             ThermalMgmt_CurrentData.HottestZone);
    
    /* Step 3: Calculate and apply fan speed based on mode */
    switch (ThermalMgmt_CurrentMode)
    {
        case THERMALMGMT_MODE_OFF:
            ThermalMgmt_ApplyFanSpeed(THERMALMGMT_FAN_OFF_SPEED);
            break;
            
        case THERMALMGMT_MODE_PASSIVE:
            ThermalMgmt_ApplyFanSpeed(THERMALMGMT_FAN_MIN_SPEED);
            break;
            
        case THERMALMGMT_MODE_AUTO:
            requiredSpeed = ThermalMgmt_CalculateFanSpeed(ThermalMgmt_CurrentData.MaxTemperature);
            ThermalMgmt_ApplyFanSpeed(requiredSpeed);
            break;
            
        case THERMALMGMT_MODE_AGGRESSIVE:
            ThermalMgmt_ApplyFanSpeed(THERMALMGMT_FAN_MAX_SPEED);
            break;
            
        case THERMALMGMT_MODE_MANUAL:
            /* Speed already set by user, don't change */
            break;
            
        default:
            /* Invalid mode */
            break;
    }
}

/**
 * @brief Set operating mode
 */
Std_ReturnType ThermalMgmt_SetMode(ThermalMgmt_ModeType Mode)
{
#if (THERMALMGMT_DEV_ERROR_DETECT == STD_ON)
    if (ThermalMgmt_Status == THERMALMGMT_STATUS_UNINIT)
    {
        return E_NOT_OK;
    }
#endif
    
    ThermalMgmt_CurrentMode = Mode;
    
    return E_OK;
}

/**
 * @brief Get current operating mode
 */
ThermalMgmt_ModeType ThermalMgmt_GetMode(void)
{
    return ThermalMgmt_CurrentMode;
}

/**
 * @brief Get current thermal status
 */
ThermalMgmt_StatusType ThermalMgmt_GetStatus(void)
{
    return ThermalMgmt_Status;
}

/**
 * @brief Get detailed thermal data
 */
Std_ReturnType ThermalMgmt_GetData(ThermalMgmt_DataType* DataPtr)
{
#if (THERMALMGMT_DEV_ERROR_DETECT == STD_ON)
    if (ThermalMgmt_Status == THERMALMGMT_STATUS_UNINIT)
    {
        return E_NOT_OK;
    }
    
    if (DataPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }
#endif
    
    *DataPtr = ThermalMgmt_CurrentData;
    
    return E_OK;
}

/**
 * @brief Get temperature of specific zone
 */
Std_ReturnType ThermalMgmt_GetZoneTemperature(ThermalMgmt_ZoneType Zone, float32* Temperature)
{
#if (THERMALMGMT_DEV_ERROR_DETECT == STD_ON)
    if (ThermalMgmt_Status == THERMALMGMT_STATUS_UNINIT)
    {
        return E_NOT_OK;
    }
    
    if (Temperature == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    if (Zone >= THERMALMGMT_ZONE_COUNT)
    {
        return E_NOT_OK;
    }
#endif
    
    *Temperature = ThermalMgmt_CurrentData.ZoneTemperatures[Zone];
    
    return E_OK;
}

/**
 * @brief Manually set fan speed
 */
Std_ReturnType ThermalMgmt_SetFanSpeed(uint8 SpeedPercent)
{
#if (THERMALMGMT_DEV_ERROR_DETECT == STD_ON)
    if (ThermalMgmt_Status == THERMALMGMT_STATUS_UNINIT)
    {
        return E_NOT_OK;
    }
    
    if (SpeedPercent > 100u)
    {
        return E_NOT_OK;
    }
#endif
    
    if (ThermalMgmt_CurrentMode != THERMALMGMT_MODE_MANUAL)
    {
        return E_NOT_OK;  /* Can only set speed in manual mode */
    }
    
    ThermalMgmt_ApplyFanSpeed(SpeedPercent);
    
    return E_OK;
}

/**
 * @brief Get current fan speed
 */
uint8 ThermalMgmt_GetFanSpeed(void)
{
    return ThermalMgmt_CurrentFanSpeed;
}

/**
 * @brief Force maximum cooling (emergency)
 */
void ThermalMgmt_EmergencyCooling(void)
{
    if (ThermalMgmt_Status == THERMALMGMT_STATUS_UNINIT)
    {
        return;
    }
    
    ThermalMgmt_CurrentMode = THERMALMGMT_MODE_AGGRESSIVE;
    ThermalMgmt_ApplyFanSpeed(THERMALMGMT_FAN_MAX_SPEED);
}

/**
 * @brief Check if thermal shutdown is required
 */
boolean ThermalMgmt_IsShutdownRequired(void)
{
    return ThermalMgmt_ShutdownRequired;
}

#if (THERMALMGMT_EVENT_CALLBACK_API == STD_ON)
/**
 * @brief Set event callback
 */
void ThermalMgmt_SetEventCallback(ThermalMgmt_EventCallbackType Callback)
{
    ThermalMgmt_EventCallback = Callback;
}
#endif

#if (THERMALMGMT_VERSION_INFO_API == STD_ON)
/**
 * @brief Get service version information
 */
void ThermalMgmt_GetVersionInfo(Std_VersionInfoType* versionInfoPtr)
{
#if (THERMALMGMT_DEV_ERROR_DETECT == STD_ON)
    if (versionInfoPtr == NULL_PTR)
    {
        Det_ReportError(THERMALMGMT_MODULE_ID, THERMALMGMT_INSTANCE_ID, 
                       THERMALMGMT_GET_VERSION_SID, THERMALMGMT_E_PARAM_POINTER);
        return;
    }
#endif
    
    versionInfoPtr->vendorID = THERMALMGMT_VENDOR_ID;
    versionInfoPtr->moduleID = THERMALMGMT_MODULE_ID;
    versionInfoPtr->sw_major_version = THERMALMGMT_SW_MAJOR_VERSION;
    versionInfoPtr->sw_minor_version = THERMALMGMT_SW_MINOR_VERSION;
    versionInfoPtr->sw_patch_version = THERMALMGMT_SW_PATCH_VERSION;
}
#endif
