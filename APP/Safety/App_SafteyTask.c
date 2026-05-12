/**
 * @file App_SafetyTask.c
 * @brief Safety Monitoring Task Implementation
 * @details Highest priority task for system safety monitoring
 *
 * SAFETY RESPONSIBILITIES:
 * 1. Feed watchdog (ONLY this task feeds WDG)
 * 2. Monitor motor current for overload
 * 3. Check temperature limits
 * 4. Manage motor enable privilege
 * 5. Enter safe state on fault conditions
 *
 * PRIVILEGE MODEL:
 * - This task is the SOLE owner of WDG feed
 * - This task controls motor enable via SafeState module
 * - Control Task must request permission to run motors
 *
 * @author Mohamed Yasser
 * @date Jan 09, 2026
 * @version 2.0.0
 */

#include "../../CONFIG/Std_Types.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"

/* Driver includes */
#include "../../MCAL/WDG/Wdg.h"
#include "../../ECUAL/CURRENT_SENSOR/ACS712.h"
#include "../../ECUAL/MOTOR/Motor.h"
#include "../../SERVICES/THERMAL/ThermalMgmt.h"
#include "../../SERVICES/DIAG/Diagnostics.h"
#include "../../CONFIG/System_FeatureFlags.h"

/* Safe state manager */
#include "App_SafeState.h"

/* Resource management */
#include "../Common/App_ResourceMap.h"

/* ===================[Private Variables]=================== */
static boolean App_SafetyInitialized = FALSE;
static uint32 App_SafetyCycleCount = 0u;

/* Health check flags */
static boolean App_Health_ThermalOk = TRUE;
static boolean App_Health_CurrentOk = TRUE;
static boolean App_Health_CommOk = TRUE;

/* Heartbeat tracking */
static uint32 App_LastHeartbeatTime = 0u;
#define HEARTBEAT_TIMEOUT_TICKS     (500u)  /* 5 seconds at 100Hz tick */
static boolean App_HeartbeatFaultActive = FALSE; /* Prevent repeated SafeState_Enter */

/* Boot grace period: always feed WDG for the first N ticks after boot
 * to prevent reset loop when no ROS2 bridge is connected yet. */
#define BOOT_GRACE_TICKS            (500u)  /* 5 seconds grace period */
static boolean App_BootGraceActive = TRUE;

/* ===================[Private Functions]=================== */

/**
 * @brief Check motor current limits
 * @return TRUE if all currents within limits
 */
static boolean App_Safety_CheckMotorCurrent(void)
{
#if (FEATURE_CURRENT_ENABLED == 0u)
    /* Current sensors not connected — skip check */
    return TRUE;
#else
    ACS712_DataType currentData;
    boolean allOk = TRUE;
    static boolean prevLeftOverload = FALSE;
    static boolean prevRightOverload = FALSE;
    
    /* Check left motor overload */
    if (ACS712_ReadCurrent(0u, &currentData) == E_OK)
    {
        if (currentData.Status == ACS712_CHANNEL_OVERLOAD)
        {
            SafeState_SetFault(FAULT_FLAG_MOTOR_L_OVERLOAD);
            SafeState_Enter(SAFESTATE_REASON_MOTOR_LEFT_OVERLOAD);
            if (!prevLeftOverload)
            {
                Diag_ReportDtc(DIAG_DTC_MOTOR_OVERLOAD, TRUE);
            }
            prevLeftOverload = TRUE;
            allOk = FALSE;
        }
        else
        {
            prevLeftOverload = FALSE;
        }
    }
    
    /* Check right motor overload */
    if (ACS712_ReadCurrent(1u, &currentData) == E_OK)
    {
        if (currentData.Status == ACS712_CHANNEL_OVERLOAD)
        {
            SafeState_SetFault(FAULT_FLAG_MOTOR_R_OVERLOAD);
            SafeState_Enter(SAFESTATE_REASON_MOTOR_RIGHT_OVERLOAD);
            if (!prevRightOverload)
            {
                Diag_ReportDtc(DIAG_DTC_MOTOR_OVERLOAD, TRUE);
            }
            prevRightOverload = TRUE;
            allOk = FALSE;
        }
        else
        {
            prevRightOverload = FALSE;
        }
    }
    
    return allOk;
#endif
}

/**
 * @brief Check thermal limits
 * @return TRUE if all temperatures within limits
 */
static boolean App_Safety_CheckThermal(void)
{
#if (FEATURE_TEMP_ENABLED == 0u)
    /* Temp sensors not connected — skip check */
    return TRUE;
#else
    ThermalMgmt_StatusType thermalStatus;
    boolean allOk = TRUE;
    static ThermalMgmt_StatusType prevThermalStatus = THERMALMGMT_STATUS_NORMAL;
    
    thermalStatus = ThermalMgmt_GetStatus();
    
    if (thermalStatus == THERMALMGMT_STATUS_CRITICAL)
    {
        SafeState_SetFault(FAULT_FLAG_ENCLOSURE_THERMAL);
        if (prevThermalStatus != THERMALMGMT_STATUS_CRITICAL)
        {
            Diag_ReportDtc(DIAG_DTC_THERMAL_CRITICAL, TRUE);
        }
        allOk = FALSE;
        
        /* Force maximum cooling */
        ThermalMgmt_EmergencyCooling();
    }
    
    if (ThermalMgmt_IsShutdownRequired() == TRUE)
    {
        SafeState_Enter(SAFESTATE_REASON_ENCLOSURE_THERMAL);
        if (prevThermalStatus != THERMALMGMT_STATUS_SHUTDOWN)
        {
            Diag_ReportDtc(DIAG_DTC_THERMAL_SHUTDOWN, TRUE);
        }
        allOk = FALSE;
    }
    
    prevThermalStatus = thermalStatus;
    return allOk;
#endif
}

/**
 * @brief Check communication heartbeat
 * @return TRUE if heartbeat is alive
 */
static boolean App_Safety_CheckHeartbeat(void)
{
    uint32 currentTick = xTaskGetTickCount();

    /* Skip heartbeat check during boot grace period — bridge isn't connected yet */
    if (App_BootGraceActive)
    {
        if (currentTick < BOOT_GRACE_TICKS)
        {
            return TRUE;  /* Pretend heartbeat is OK during grace */
        }
        else
        {
            App_BootGraceActive = FALSE;
            /* Reset heartbeat time to NOW so the timeout starts fresh */
            App_LastHeartbeatTime = currentTick;
        }
    }

    if ((currentTick - App_LastHeartbeatTime) > HEARTBEAT_TIMEOUT_TICKS)
    {
        /* Only enter safe state ONCE per timeout event, not every cycle */
        if (!App_HeartbeatFaultActive)
        {
            SafeState_SetFault(FAULT_FLAG_HEARTBEAT_TIMEOUT);
            SafeState_Enter(SAFESTATE_REASON_HEARTBEAT_TIMEOUT);
            App_HeartbeatFaultActive = TRUE;
        }
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Evaluate overall system health
 * @return TRUE if all subsystems healthy
 */
static boolean App_Safety_EvaluateHealth(void)
{
    App_Health_CurrentOk = App_Safety_CheckMotorCurrent();
    App_Health_ThermalOk = App_Safety_CheckThermal();
    App_Health_CommOk = App_Safety_CheckHeartbeat();
    
    return (App_Health_CurrentOk && App_Health_ThermalOk && App_Health_CommOk);
}

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize safety monitoring
 */
void App_SafetyTask_Init(void)
{
    /* Initialize safe state manager */
    SafeState_Init();
    
    /* Initialize heartbeat time */
    App_LastHeartbeatTime = xTaskGetTickCount();
    
    App_SafetyCycleCount = 0u;
    App_SafetyInitialized = TRUE;
    
    Diag_LogEvent(DIAG_SRC_SAFETY, 0x0001u, DIAG_SEVERITY_INFO, NULL_PTR);
}

/**
 * @brief Safety task main function (called by FreeRTOS task)
 * @note This runs every 10ms - highest priority
 */
void App_SafetyTask_Run(void)
{
    boolean systemHealthy;
    
    if (App_SafetyInitialized == FALSE)
    {
        App_SafetyTask_Init();
    }
    
    App_SafetyCycleCount++;
    
    /* During boot grace: feed WDG FIRST before any checks.
     * This ensures the system doesn't reset even if a health-check crashes. */
    if (App_BootGraceActive)
    {
        Wdg_Service();
    }
    
    /* 1. Evaluate all subsystem health */
    systemHealthy = App_Safety_EvaluateHealth();
    
    /* 2. Update safe state manager */
    SafeState_Update();
    
    /* 3. CRITICAL: Feed watchdog */
    /* As long as the FreeRTOS Safety task is executing, we feed the watchdog.
     * Functional faults (thermal, motor overload, heartbeat) are handled by 
     * the SafeState manager which explicitly disables PWM/motors.
     * We DO NOT want to stop feeding the WDG on functional faults, otherwise
     * the MCU boot-loops and we lose telemetry to ROS. */
    Wdg_Service();
    
    /* 4. Handle motor enable requests */
    if (systemHealthy)
    {
        /* Allow motor enable if requested and healthy */
        (void)SafeState_RequestMotorEnable();
    }
    else
    {
        /* Force motor disable on any fault */
        SafeState_RequestMotorDisable();
    }
    
    /* 5. Publish safety status to Comm task for ROS reporting */
    {
        SafetyStatusMsgType statusMsg;
        SafeState_InfoType safeInfo;
        
        SafeState_GetInfo(&safeInfo);
        
        statusMsg.FaultFlags = safeInfo.FaultFlags;
        statusMsg.Timestamp = xTaskGetTickCount();
        statusMsg.SafeStateStatus = (uint8)safeInfo.Status;
        statusMsg.LastFaultReason = (uint8)safeInfo.LastReason;
        statusMsg.MotorEnabled = safeInfo.MotorEnableAllowed;
        statusMsg.RequiresAck = (safeInfo.FaultFlags != 0u);
        
        (void)ResourceMap_SendSafetyStatus(&statusMsg);
    }
}

/**
 * @brief Report heartbeat received from ROS
 * @note Called by Comm Task when heartbeat packet received
 */
void App_SafetyTask_ReportHeartbeat(void)
{
    App_LastHeartbeatTime = xTaskGetTickCount();
    App_HeartbeatFaultActive = FALSE;
    SafeState_ClearFault(FAULT_FLAG_HEARTBEAT_TIMEOUT);
    
    /* If the system was locked out due to heartbeat loss, fully recover.
     * ROS2 is back online, so clear the safe state and allow motors. */
    if (SafeState_GetStatus() >= SAFESTATE_ACTIVE)
    {
        (void)SafeState_Clear();
    }
}

/**
 * @brief Get safety task cycle count
 * @return Number of safety task cycles executed
 */
uint32 App_SafetyTask_GetCycleCount(void)
{
    return App_SafetyCycleCount;
}

/**
 * @brief Check if motor operation is safe
 * @return TRUE if motors can be operated
 */
boolean App_SafetyTask_IsMotorSafe(void)
{
    return SafeState_IsMotorEnableAllowed();
}

/**
 * @brief Request emergency stop
 * @note Can be called from any task
 */
void App_SafetyTask_RequestEmergencyStop(void)
{
    SafeState_Enter(SAFESTATE_REASON_ESTOP_COMMAND);
    Diag_LogEvent(DIAG_SRC_SAFETY, 0x00FFu, DIAG_SEVERITY_WARNING, NULL_PTR);
}
