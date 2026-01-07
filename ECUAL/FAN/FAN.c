/**
 * @file FAN.c
 * @brief Fan Control Driver Implementation
 * @details AUTOSAR-compliant driver for PWM-controlled cooling fans
 *
 * Delta FFB0812EHE Specifications:
 * - 80mm x 80mm x 38mm dual-ball bearing fan
 * - 12V DC, 1.35A max
 * - 12000 RPM max
 * - 86.7 CFM airflow
 * - 4-pin connector: VCC (Red), GND (Black), Tach (Yellow), PWM (Blue)
 * - PWM: 25kHz, 5V logic level
 * - Tach: 2 pulses per revolution, open-drain output
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#include "FAN.h"
#include "../../MCAL/PWM/PWM.h"

#if (FAN_DEV_ERROR_DETECT == STD_ON)
#include "../../CONFIG/Det.h"
#endif

/* ===================[Private Macros]=================== */
#define FAN_PERCENT_TO_DUTY(percent)    ((uint16)((percent) * 100u))  /* Convert 0-100 to 0-10000 */

/* ===================[Private Types]=================== */
typedef struct
{
    Fan_SpeedType   CurrentSpeed;       /**< Current speed setting */
    Fan_StateType   State;              /**< Fan state */
    uint16          LastRpm;            /**< Last measured RPM */
    uint32          TachPulseCount;     /**< Tachometer pulse counter */
    uint32          LastTachTime;       /**< Timestamp of last tach update */
} Fan_StateDataType;

/* ===================[Private Variables]=================== */
static const Fan_ConfigType* Fan_ConfigPtr = NULL_PTR;
static Fan_StatusType Fan_ModuleStatus = FAN_STATUS_UNINIT;
static Fan_StateDataType Fan_States[FAN_MAX_FANS];

#if (FAN_FAULT_CALLBACK_API == STD_ON)
static Fan_FaultCallbackType Fan_FaultCallback = NULL_PTR;
#endif

/* ===================[Private Function Declarations]=================== */
static boolean Fan_IsIdValid(Fan_IdType FanId);
static Std_ReturnType Fan_ApplySpeed(Fan_IdType FanId, Fan_SpeedType SpeedPercent);
static void Fan_ReportFault(Fan_IdType FanId, Fan_StateType State);

/* ===================[Private Function Implementations]=================== */

/**
 * @brief Check if fan ID is valid
 */
static boolean Fan_IsIdValid(Fan_IdType FanId)
{
    boolean isValid = FALSE;
    
    if (Fan_ConfigPtr != NULL_PTR)
    {
        if (FanId < Fan_ConfigPtr->NumFans)
        {
            isValid = TRUE;
        }
    }
    
    return isValid;
}

/**
 * @brief Apply speed to PWM output
 */
static Std_ReturnType Fan_ApplySpeed(Fan_IdType FanId, Fan_SpeedType SpeedPercent)
{
    const Fan_ConfigItemType* fanCfg;
    uint16 dutyCycle;
    uint8 effectiveSpeed;
    
    fanCfg = &Fan_ConfigPtr->Fans[FanId];
    
    if (SpeedPercent == 0u)
    {
        /* Fan off */
        dutyCycle = 0u;
    }
    else
    {
        /* Apply minimum duty threshold for fan startup */
        if (SpeedPercent < fanCfg->MinDutyPercent)
        {
            effectiveSpeed = fanCfg->MinDutyPercent;
        }
        else
        {
            effectiveSpeed = SpeedPercent;
        }
        
        /* Convert percentage to PWM duty (0-10000 range typically for AUTOSAR) */
        dutyCycle = FAN_PERCENT_TO_DUTY(effectiveSpeed);
    }
    
    /* Set PWM duty cycle - Pwm_SetDutyCycle returns void per AUTOSAR spec */
    Pwm_SetDutyCycle(fanCfg->PwmChannel, dutyCycle);
    
    /* Assume success since PWM driver doesn't return status */
    return E_OK;
}

/**
 * @brief Report fan fault via callback
 */
static void Fan_ReportFault(Fan_IdType FanId, Fan_StateType State)
{
    #if (FAN_FAULT_CALLBACK_API == STD_ON)
    if (Fan_FaultCallback != NULL_PTR)
    {
        Fan_FaultCallback(FanId, State);
    }
    #else
    (void)FanId;
    (void)State;
    #endif
}

/* ===================[Public Function Implementations]=================== */

/**
 * @brief Initialize the Fan Control driver
 */
void Fan_Init(const Fan_ConfigType* ConfigPtr)
{
    uint8 fanIdx;
    
#if (FAN_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_INIT_SID, FAN_E_PARAM_POINTER);
        return;
    }
    
    if (Fan_ModuleStatus != FAN_STATUS_UNINIT)
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_INIT_SID, FAN_E_ALREADY_INIT);
        return;
    }
    
    if (ConfigPtr->NumFans > FAN_MAX_FANS)
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_INIT_SID, FAN_E_PARAM_CONFIG);
        return;
    }
    
    if (ConfigPtr->Fans == NULL_PTR)
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_INIT_SID, FAN_E_PARAM_POINTER);
        return;
    }
#endif
    
    /* Store configuration pointer */
    Fan_ConfigPtr = ConfigPtr;
    
    /* Initialize fan states and set all fans to OFF */
    for (fanIdx = 0u; fanIdx < ConfigPtr->NumFans; fanIdx++)
    {
        Fan_States[fanIdx].CurrentSpeed = 0u;
        Fan_States[fanIdx].State = FAN_STATE_OFF;
        Fan_States[fanIdx].LastRpm = 0u;
        Fan_States[fanIdx].TachPulseCount = 0u;
        Fan_States[fanIdx].LastTachTime = 0u;
        
        /* Ensure fans are off on startup */
        (void)Fan_ApplySpeed(fanIdx, 0u);
    }
    
    /* Set module status to idle */
    Fan_ModuleStatus = FAN_STATUS_IDLE;
}

#if (FAN_DEINIT_API == STD_ON)
/**
 * @brief De-initialize the Fan Control driver
 */
void Fan_DeInit(void)
{
    uint8 fanIdx;
    
#if (FAN_DEV_ERROR_DETECT == STD_ON)
    if (Fan_ModuleStatus == FAN_STATUS_UNINIT)
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_DEINIT_SID, FAN_E_UNINIT);
        return;
    }
#endif
    
    /* Stop all fans */
    for (fanIdx = 0u; fanIdx < Fan_ConfigPtr->NumFans; fanIdx++)
    {
        (void)Fan_ApplySpeed(fanIdx, 0u);
        Fan_States[fanIdx].CurrentSpeed = 0u;
        Fan_States[fanIdx].State = FAN_STATE_OFF;
    }
    
    #if (FAN_FAULT_CALLBACK_API == STD_ON)
    Fan_FaultCallback = NULL_PTR;
    #endif
    
    Fan_ConfigPtr = NULL_PTR;
    Fan_ModuleStatus = FAN_STATUS_UNINIT;
}
#endif

/**
 * @brief Set fan speed
 */
Std_ReturnType Fan_SetSpeed(Fan_IdType FanId, Fan_SpeedType SpeedPercent)
{
    Std_ReturnType status = E_NOT_OK;
    
#if (FAN_DEV_ERROR_DETECT == STD_ON)
    if (Fan_ModuleStatus == FAN_STATUS_UNINIT)
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_SET_SPEED_SID, FAN_E_UNINIT);
        return E_NOT_OK;
    }
    
    if (!Fan_IsIdValid(FanId))
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_SET_SPEED_SID, FAN_E_PARAM_FAN);
        return E_NOT_OK;
    }
    
    if (SpeedPercent > FAN_SPEED_MAX)
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_SET_SPEED_SID, FAN_E_PARAM_SPEED);
        return E_NOT_OK;
    }
#endif
    
    status = Fan_ApplySpeed(FanId, SpeedPercent);
    
    if (status == E_OK)
    {
        Fan_States[FanId].CurrentSpeed = SpeedPercent;
        
        if (SpeedPercent == 0u)
        {
            Fan_States[FanId].State = FAN_STATE_OFF;
        }
        else
        {
            Fan_States[FanId].State = FAN_STATE_RUNNING;
        }
        
        /* Update module status */
        Fan_ModuleStatus = FAN_STATUS_RUNNING;
    }
    else
    {
        Fan_States[FanId].State = FAN_STATE_ERROR;
        Fan_ReportFault(FanId, FAN_STATE_ERROR);
    }
    
    return status;
}

/**
 * @brief Set all fans to same speed
 */
Std_ReturnType Fan_SetAllSpeed(Fan_SpeedType SpeedPercent)
{
    Std_ReturnType status = E_OK;
    uint8 fanIdx;
    
#if (FAN_DEV_ERROR_DETECT == STD_ON)
    if (Fan_ModuleStatus == FAN_STATUS_UNINIT)
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_SET_SPEED_SID, FAN_E_UNINIT);
        return E_NOT_OK;
    }
    
    if (SpeedPercent > FAN_SPEED_MAX)
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_SET_SPEED_SID, FAN_E_PARAM_SPEED);
        return E_NOT_OK;
    }
#endif
    
    for (fanIdx = 0u; fanIdx < Fan_ConfigPtr->NumFans; fanIdx++)
    {
        if (Fan_SetSpeed(fanIdx, SpeedPercent) != E_OK)
        {
            status = E_NOT_OK;
        }
    }
    
    return status;
}

/**
 * @brief Stop a specific fan
 */
Std_ReturnType Fan_Stop(Fan_IdType FanId)
{
    return Fan_SetSpeed(FanId, 0u);
}

/**
 * @brief Stop all fans (emergency stop)
 */
void Fan_StopAll(void)
{
    uint8 fanIdx;
    
    if (Fan_ModuleStatus == FAN_STATUS_UNINIT)
    {
        return;
    }
    
    for (fanIdx = 0u; fanIdx < Fan_ConfigPtr->NumFans; fanIdx++)
    {
        (void)Fan_ApplySpeed(fanIdx, 0u);
        Fan_States[fanIdx].CurrentSpeed = 0u;
        Fan_States[fanIdx].State = FAN_STATE_OFF;
    }
    
    Fan_ModuleStatus = FAN_STATUS_IDLE;
}

/**
 * @brief Get current fan speed setting
 */
Fan_SpeedType Fan_GetSpeed(Fan_IdType FanId)
{
#if (FAN_DEV_ERROR_DETECT == STD_ON)
    if (Fan_ModuleStatus == FAN_STATUS_UNINIT)
    {
        return 0u;
    }
    
    if (!Fan_IsIdValid(FanId))
    {
        return 0u;
    }
#endif
    
    return Fan_States[FanId].CurrentSpeed;
}

/**
 * @brief Get fan runtime data
 */
Std_ReturnType Fan_GetData(Fan_IdType FanId, Fan_DataType* DataPtr)
{
#if (FAN_DEV_ERROR_DETECT == STD_ON)
    if (Fan_ModuleStatus == FAN_STATUS_UNINIT)
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_GET_STATUS_SID, FAN_E_UNINIT);
        return E_NOT_OK;
    }
    
    if (DataPtr == NULL_PTR)
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_GET_STATUS_SID, FAN_E_PARAM_POINTER);
        return E_NOT_OK;
    }
    
    if (!Fan_IsIdValid(FanId))
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_GET_STATUS_SID, FAN_E_PARAM_FAN);
        return E_NOT_OK;
    }
#endif
    
    DataPtr->CurrentSpeed = Fan_States[FanId].CurrentSpeed;
    DataPtr->RpmMeasured = Fan_States[FanId].LastRpm;
    DataPtr->State = Fan_States[FanId].State;
    
    return E_OK;
}

/**
 * @brief Get fan state
 */
Fan_StateType Fan_GetState(Fan_IdType FanId)
{
#if (FAN_DEV_ERROR_DETECT == STD_ON)
    if (Fan_ModuleStatus == FAN_STATUS_UNINIT)
    {
        return FAN_STATE_ERROR;
    }
    
    if (!Fan_IsIdValid(FanId))
    {
        return FAN_STATE_ERROR;
    }
#endif
    
    return Fan_States[FanId].State;
}

/**
 * @brief Get driver status
 */
Fan_StatusType Fan_GetStatus(void)
{
    return Fan_ModuleStatus;
}

#if (FAN_TACHOMETER_SUPPORT == STD_ON)
/**
 * @brief Get measured RPM
 */
Std_ReturnType Fan_GetRpm(Fan_IdType FanId, uint16* Rpm)
{
#if (FAN_DEV_ERROR_DETECT == STD_ON)
    if (Fan_ModuleStatus == FAN_STATUS_UNINIT)
    {
        return E_NOT_OK;
    }
    
    if (Rpm == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    if (!Fan_IsIdValid(FanId))
    {
        return E_NOT_OK;
    }
#endif
    
    *Rpm = Fan_States[FanId].LastRpm;
    
    return E_OK;
}

/**
 * @brief Update tachometer readings
 */
void Fan_UpdateTachometer(void)
{
    uint8 fanIdx;
    const Fan_ConfigItemType* fanCfg;
    uint32 pulseCount;
    uint32 currentTime;
    uint32 elapsedMs;
    uint16 rpm;
    
    if (Fan_ModuleStatus == FAN_STATUS_UNINIT)
    {
        return;
    }
    
    /* Get current timestamp (placeholder - use timer/tick) */
    currentTime = 0u;  /* TODO: Get from timer */
    
    for (fanIdx = 0u; fanIdx < Fan_ConfigPtr->NumFans; fanIdx++)
    {
        fanCfg = &Fan_ConfigPtr->Fans[fanIdx];
        
        if (fanCfg->TachChannel != 0xFFu)
        {
            /* Read pulse count from timer capture */
            pulseCount = 0u;  /* TODO: Read from timer capture */
            
            elapsedMs = currentTime - Fan_States[fanIdx].LastTachTime;
            
            if (elapsedMs > 0u)
            {
                /* RPM = (pulses / pulses_per_rev) * (60000 / elapsed_ms) */
                rpm = (uint16)(((pulseCount - Fan_States[fanIdx].TachPulseCount) * 60000u) / 
                               (fanCfg->PulsesPerRev * elapsedMs));
                
                Fan_States[fanIdx].LastRpm = rpm;
            }
            
            Fan_States[fanIdx].TachPulseCount = pulseCount;
            Fan_States[fanIdx].LastTachTime = currentTime;
            
            /* Check for stall condition */
            if ((Fan_States[fanIdx].CurrentSpeed > 0u) && (rpm == 0u))
            {
                if (elapsedMs > FAN_STALL_TIMEOUT_MS)
                {
                    Fan_States[fanIdx].State = FAN_STATE_STALLED;
                    Fan_ReportFault(fanIdx, FAN_STATE_STALLED);
                }
            }
        }
    }
}
#endif

#if (FAN_FAULT_CALLBACK_API == STD_ON)
/**
 * @brief Set fault notification callback
 */
void Fan_SetFaultCallback(Fan_FaultCallbackType Callback)
{
    Fan_FaultCallback = Callback;
}
#endif

#if (FAN_VERSION_INFO_API == STD_ON)
/**
 * @brief Get driver version information
 */
void Fan_GetVersionInfo(Std_VersionInfoType* versionInfoPtr)
{
#if (FAN_DEV_ERROR_DETECT == STD_ON)
    if (versionInfoPtr == NULL_PTR)
    {
        Det_ReportError(FAN_MODULE_ID, FAN_INSTANCE_ID, FAN_GET_VERSION_SID, FAN_E_PARAM_POINTER);
        return;
    }
#endif
    
    versionInfoPtr->vendorID = FAN_VENDOR_ID;
    versionInfoPtr->moduleID = FAN_MODULE_ID;
    versionInfoPtr->sw_major_version = FAN_SW_MAJOR_VERSION;
    versionInfoPtr->sw_minor_version = FAN_SW_MINOR_VERSION;
    versionInfoPtr->sw_patch_version = FAN_SW_PATCH_VERSION;
}
#endif
