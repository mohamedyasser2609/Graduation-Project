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

#include "../CONFIG/Std_Types.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"

/* Driver includes */
#include "../MCAL/WDG/Wdg.h"
#include "../ECUAL/CURRENT_SENSOR/ACS712.h"
#include "../ECUAL/MOTOR/Motor.h"
#include "../SERVICES/THERMAL/ThermalMgmt.h"
#include "../SERVICES/DIAG/Diagnostics.h"

/* Safe state manager */
#include "App_SafeState.h"

/* ===================[Private Variables]=================== */
static boolean App_SafetyInitialized = FALSE;
static uint32 App_SafetyCycleCount = 0u;

/* Health check flags */
static boolean App_Health_ThermalOk = TRUE;
static boolean App_Health_CurrentOk = TRUE;
static boolean App_Health_CommOk = TRUE;

/* Heartbeat tracking */
static uint32 App_LastHeartbeatTime = 0u;
#define HEARTBEAT_TIMEOUT_TICKS     (200u)  /* 2 seconds at 100Hz tick */

/* ===================[Private Functions]=================== */

/**
 * @brief Check motor current limits
 * @return TRUE if all currents within limits
 */
static boolean App_Safety_CheckMotorCurrent(void)
{
    ACS712_DataType currentData;
    boolean allOk = TRUE;
    
    /* Check left motor overload */
    if (ACS712_ReadCurrent(0u, &currentData) == E_OK)
    {
        if (currentData.Status == ACS712_CHANNEL_OVERLOAD)
        {
            SafeState_SetFault(FAULT_FLAG_MOTOR_L_OVERLOAD);
            SafeState_Enter(SAFESTATE_REASON_MOTOR_LEFT_OVERLOAD);
            Diag_ReportDtc(DIAG_DTC_MOTOR_OVERLOAD, TRUE);
            allOk = FALSE;
        }
    }
    
    /* Check right motor overload */
    if (ACS712_ReadCurrent(1u, &currentData) == E_OK)
    {
        if (currentData.Status == ACS712_CHANNEL_OVERLOAD)
        {
            SafeState_SetFault(FAULT_FLAG_MOTOR_R_OVERLOAD);
            SafeState_Enter(SAFESTATE_REASON_MOTOR_RIGHT_OVERLOAD);
            Diag_ReportDtc(DIAG_DTC_MOTOR_OVERLOAD, TRUE);
            allOk = FALSE;
        }
    }
    
    return allOk;
}

/**
 * @brief Check thermal limits
 * @return TRUE if all temperatures within limits
 */
static boolean App_Safety_CheckThermal(void)
{
    ThermalMgmt_StatusType thermalStatus;
    boolean allOk = TRUE;
    
    thermalStatus = ThermalMgmt_GetStatus();
    
    if (thermalStatus == THERMALMGMT_STATUS_CRITICAL)
    {
        SafeState_SetFault(FAULT_FLAG_ENCLOSURE_THERMAL);
        Diag_ReportDtc(DIAG_DTC_THERMAL_CRITICAL, TRUE);
        allOk = FALSE;
        
        /* Force maximum cooling */
        ThermalMgmt_EmergencyCooling();
    }
    
    if (ThermalMgmt_IsShutdownRequired() == TRUE)
    {
        SafeState_Enter(SAFESTATE_REASON_ENCLOSURE_THERMAL);
        Diag_ReportDtc(DIAG_DTC_THERMAL_SHUTDOWN, TRUE);
        allOk = FALSE;
    }
    
    return allOk;
}

/**
 * @brief Check communication heartbeat
 * @return TRUE if heartbeat is alive
 */
static boolean App_Safety_CheckHeartbeat(void)
{
    uint32 currentTick = xTaskGetTickCount();
    
    if ((currentTick - App_LastHeartbeatTime) > HEARTBEAT_TIMEOUT_TICKS)
    {
        SafeState_SetFault(FAULT_FLAG_HEARTBEAT_TIMEOUT);
        SafeState_Enter(SAFESTATE_REASON_HEARTBEAT_TIMEOUT);
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
    
    /* 1. Evaluate all subsystem health */
    systemHealthy = App_Safety_EvaluateHealth();
    
    /* 2. Update safe state manager */
    SafeState_Update();
    
    /* 3. CRITICAL: Feed watchdog ONLY if system is healthy */
    if (systemHealthy && (SafeState_GetStatus() <= SAFESTATE_WARNING))
    {
        Wdg_Service();
    }
    /* If NOT fed, WDG will timeout in 500ms → MCU reset → motors off */
    
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
}

/**
 * @brief Report heartbeat received from ROS
 * @note Called by Comm Task when heartbeat packet received
 */
void App_SafetyTask_ReportHeartbeat(void)
{
    App_LastHeartbeatTime = xTaskGetTickCount();
    SafeState_ClearFault(FAULT_FLAG_HEARTBEAT_TIMEOUT);
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
