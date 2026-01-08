/**
 * @file App_SafetyTask.c
 * @brief Safety Monitoring Task Implementation
 * @details Highest priority task for system safety monitoring
 *
 * Responsibilities:
 * - Feed watchdog timer
 * - Monitor motor current for overload
 * - Check temperature limits
 * - Emergency stop on fault conditions
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "../CONFIG/Std_Types.h"

/* Driver includes */
#include "../MCAL/WDG/Wdg.h"
#include "../ECUAL/CURRENT_SENSOR/ACS712.h"
#include "../ECUAL/MOTOR/Motor.h"
#include "../SERVICES/THERMAL/ThermalMgmt.h"

/* ===================[External Configurations]=================== */
extern const Wdg_ConfigType Wdg_Config;
extern const ACS712_ConfigType ACS712_Config;

/* ===================[Private Variables]=================== */
static boolean App_SafetyInitialized = FALSE;
static uint32 App_SafetyOverloadCount = 0u;
static uint32 App_SafetyFaultFlags = 0u;

/* Fault flag bits */
#define FAULT_MOTOR_LEFT_OVERLOAD   (1u << 0u)
#define FAULT_MOTOR_RIGHT_OVERLOAD  (1u << 1u)
#define FAULT_THERMAL_CRITICAL      (1u << 2u)
#define FAULT_THERMAL_SHUTDOWN      (1u << 3u)
#define FAULT_WATCHDOG              (1u << 4u)

/* ===================[Private Functions]=================== */

/**
 * @brief Check motor current limits
 */
static void App_Safety_CheckMotorCurrent(void)
{
    ACS712_DataType currentData;
    
    /* Check left motor overload */
    if (ACS712_ReadCurrent(0u, &currentData) == E_OK)
    {
        if (currentData.Status == ACS712_CHANNEL_OVERLOAD)
        {
            App_SafetyFaultFlags |= FAULT_MOTOR_LEFT_OVERLOAD;
            App_SafetyOverloadCount++;
            
            /* Stop motor on overload */
            (void)Motor_Stop(0u);
        }
    }
    
    /* Check right motor overload */
    if (ACS712_ReadCurrent(1u, &currentData) == E_OK)
    {
        if (currentData.Status == ACS712_CHANNEL_OVERLOAD)
        {
            App_SafetyFaultFlags |= FAULT_MOTOR_RIGHT_OVERLOAD;
            App_SafetyOverloadCount++;
            
            /* Stop motor on overload */
            (void)Motor_Stop(1u);
        }
    }
}

/**
 * @brief Check thermal limits
 */
static void App_Safety_CheckThermal(void)
{
    ThermalMgmt_StatusType thermalStatus;
    
    thermalStatus = ThermalMgmt_GetStatus();
    
    if (thermalStatus == THERMALMGMT_STATUS_CRITICAL)
    {
        App_SafetyFaultFlags |= FAULT_THERMAL_CRITICAL;
        
        /* Force maximum cooling */
        ThermalMgmt_EmergencyCooling();
    }
    
    if (ThermalMgmt_IsShutdownRequired() == TRUE)
    {
        App_SafetyFaultFlags |= FAULT_THERMAL_SHUTDOWN;
        
        /* Stop motors to reduce heat generation */
        Motor_StopAll();
    }
}

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize safety monitoring
 */
void App_SafetyTask_Init(void)
{
    /* Initialize watchdog (if not already done) */
    /* Wdg_Init(&Wdg_Config); */
    
    App_SafetyFaultFlags = 0u;
    App_SafetyOverloadCount = 0u;
    App_SafetyInitialized = TRUE;
}

/**
 * @brief Safety task main function (called by FreeRTOS task)
 */
void App_SafetyTask_Run(void)
{
    if (App_SafetyInitialized == FALSE)
    {
        App_SafetyTask_Init();
    }
    
    /* 1. Feed watchdog to prevent reset */
    Wdg_Service();
    
    /* 2. Check motor current for overload */
    App_Safety_CheckMotorCurrent();
    
    /* 3. Check thermal conditions */
    App_Safety_CheckThermal();
    
    /* 4. Clear transient faults after checking */
    /* Overload flags are cleared after motor restarts successfully */
}

/**
 * @brief Get current fault flags
 * @return Fault flags bitmap
 */
uint32 App_SafetyTask_GetFaults(void)
{
    return App_SafetyFaultFlags;
}

/**
 * @brief Clear fault flags
 */
void App_SafetyTask_ClearFaults(void)
{
    App_SafetyFaultFlags = 0u;
}

/**
 * @brief Check if emergency stop is active
 * @return TRUE if e-stop active
 */
boolean App_SafetyTask_IsEmergencyStop(void)
{
    return ((App_SafetyFaultFlags & (FAULT_THERMAL_SHUTDOWN | FAULT_MOTOR_LEFT_OVERLOAD | FAULT_MOTOR_RIGHT_OVERLOAD)) != 0u);
}
