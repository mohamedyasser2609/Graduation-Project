/**
 * @file App_SafeState.c
 * @brief Safe State Manager Implementation
 * @details Manages system safe state and motor enable privilege
 *
 * @author Mohamed Yasser
 * @date Jan 09, 2026
 * @version 1.0.0
 */

#include "App_SafeState.h"
#include "../../ECUAL/MOTOR/Motor.h"

/* FreeRTOS includes for tick count */
#include "FreeRTOS.h"
#include "task.h"

/* ===================[Private Variables]=================== */
static SafeState_InfoType SafeState_Info = {
    .Status = SAFESTATE_NORMAL,
    .LastReason = SAFESTATE_REASON_NONE,
    .FaultFlags = 0u,
    .EntryTimestamp = 0u,
    .FaultCount = 0u,
    .MotorEnableAllowed = FALSE  /* Start disabled until proven safe */
};

static boolean SafeState_Initialized = FALSE;

/* Cooldown period after fault clear (ticks) */
#define SAFESTATE_COOLDOWN_TICKS    (100u)  /* 1 second at 100Hz tick */
static uint32 SafeState_LastFaultClearTime = 0u;

/* ===================[Private Functions]=================== */

/**
 * @brief Stop all motors immediately (brake mode)
 */
static void SafeState_StopMotors(void)
{
    Motor_StopAll();
}

/**
 * @brief Check if cooldown period has elapsed
 */
static boolean SafeState_IsCooldownComplete(void)
{
    uint32 currentTick = xTaskGetTickCount();
    
    if ((currentTick - SafeState_LastFaultClearTime) >= SAFESTATE_COOLDOWN_TICKS)
    {
        return TRUE;
    }
    return FALSE;
}

/* ===================[Public Functions]=================== */

void SafeState_Init(void)
{
    /* Prevent re-initialization */
    if (SafeState_Initialized)
    {
        return;  /* Already initialized */
    }
    
    SafeState_Info.Status = SAFESTATE_NORMAL;
    SafeState_Info.LastReason = SAFESTATE_REASON_NONE;
    SafeState_Info.FaultFlags = 0u;
    SafeState_Info.EntryTimestamp = 0u;
    SafeState_Info.FaultCount = 0u;
    SafeState_Info.MotorEnableAllowed = FALSE;
    
    SafeState_Initialized = TRUE;
}

void SafeState_Enter(SafeState_ReasonType Reason)
{
    /* CRITICAL: Stop motors FIRST */
    SafeState_StopMotors();
    
    /* Update state */
    SafeState_Info.Status = SAFESTATE_ACTIVE;
    SafeState_Info.LastReason = Reason;
    SafeState_Info.EntryTimestamp = xTaskGetTickCount();
    SafeState_Info.FaultCount++;
    SafeState_Info.MotorEnableAllowed = FALSE;
    
    /* Set corresponding fault flag */
    switch (Reason)
    {
        case SAFESTATE_REASON_MOTOR_LEFT_OVERLOAD:
            SafeState_Info.FaultFlags |= FAULT_FLAG_MOTOR_L_OVERLOAD;
            break;
        case SAFESTATE_REASON_MOTOR_RIGHT_OVERLOAD:
            SafeState_Info.FaultFlags |= FAULT_FLAG_MOTOR_R_OVERLOAD;
            break;
        case SAFESTATE_REASON_MOTOR_LEFT_THERMAL:
            SafeState_Info.FaultFlags |= FAULT_FLAG_MOTOR_L_THERMAL;
            break;
        case SAFESTATE_REASON_MOTOR_RIGHT_THERMAL:
            SafeState_Info.FaultFlags |= FAULT_FLAG_MOTOR_R_THERMAL;
            break;
        case SAFESTATE_REASON_COMMAND_TIMEOUT:
            SafeState_Info.FaultFlags |= FAULT_FLAG_CMD_TIMEOUT;
            break;
        case SAFESTATE_REASON_HEARTBEAT_TIMEOUT:
            SafeState_Info.FaultFlags |= FAULT_FLAG_HEARTBEAT_TIMEOUT;
            break;
        case SAFESTATE_REASON_ESTOP_COMMAND:
            SafeState_Info.FaultFlags |= FAULT_FLAG_ESTOP;
            break;
        default:
            break;
    }
    
    /* Check for lockout condition (too many faults) */
    if (SafeState_Info.FaultCount >= 10u)
    {
        SafeState_Info.Status = SAFESTATE_LOCKOUT;
    }
}

Std_ReturnType SafeState_Clear(void)
{
    /* Cannot clear if in lockout */
    if (SafeState_Info.Status == SAFESTATE_LOCKOUT)
    {
        return E_NOT_OK;
    }
    
    /* Clear fault flags */
    SafeState_Info.FaultFlags = 0u;
    SafeState_Info.LastReason = SAFESTATE_REASON_NONE;
    SafeState_Info.Status = SAFESTATE_NORMAL;
    SafeState_LastFaultClearTime = xTaskGetTickCount();
    
    /* Motor enable still requires explicit request after cooldown */
    SafeState_Info.MotorEnableAllowed = FALSE;
    
    return E_OK;
}

void SafeState_SetFault(uint32 FaultFlag)
{
    SafeState_Info.FaultFlags |= FaultFlag;
    
    /* Auto-enter safe state on any fault */
    if (SafeState_Info.Status == SAFESTATE_NORMAL)
    {
        SafeState_Info.Status = SAFESTATE_WARNING;
    }
}

void SafeState_ClearFault(uint32 FaultFlag)
{
    SafeState_Info.FaultFlags &= ~FaultFlag;
    SafeState_LastFaultClearTime = xTaskGetTickCount();
}

boolean SafeState_IsMotorEnableAllowed(void)
{
    return SafeState_Info.MotorEnableAllowed;
}

boolean SafeState_RequestMotorEnable(void)
{
    /* Check all conditions for motor enable */
    
    /* 1. Not in safe state or lockout */
    if (SafeState_Info.Status >= SAFESTATE_ACTIVE)
    {
        return FALSE;
    }
    
    /* 2. No critical faults active */
    if (SafeState_HasCriticalFault())
    {
        return FALSE;
    }
    
    /* 3. Cooldown period elapsed */
    if (!SafeState_IsCooldownComplete())
    {
        return FALSE;
    }
    
    /* All conditions met - allow motor enable */
    SafeState_Info.MotorEnableAllowed = TRUE;
    SafeState_Info.Status = SAFESTATE_NORMAL;
    
    return TRUE;
}

void SafeState_RequestMotorDisable(void)
{
    SafeState_Info.MotorEnableAllowed = FALSE;
    SafeState_StopMotors();
}

SafeState_StatusType SafeState_GetStatus(void)
{
    return SafeState_Info.Status;
}

Std_ReturnType SafeState_GetInfo(SafeState_InfoType* InfoPtr)
{
    if (InfoPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    *InfoPtr = SafeState_Info;
    return E_OK;
}

uint32 SafeState_GetFaultFlags(void)
{
    return SafeState_Info.FaultFlags;
}

boolean SafeState_HasCriticalFault(void)
{
    /* Critical faults that prevent motor operation */
    uint32 criticalMask = FAULT_FLAG_MOTOR_L_OVERLOAD |
                          FAULT_FLAG_MOTOR_R_OVERLOAD |
                          FAULT_FLAG_MOTOR_L_THERMAL |
                          FAULT_FLAG_MOTOR_R_THERMAL |
                          FAULT_FLAG_ENCLOSURE_THERMAL |
                          FAULT_FLAG_ESTOP;
    
    return ((SafeState_Info.FaultFlags & criticalMask) != 0u);
}

void SafeState_Update(void)
{
    /* Called by Safety Task every 10ms */
    
    /* Check for warning-to-active escalation */
    if (SafeState_Info.Status == SAFESTATE_WARNING)
    {
        if (SafeState_HasCriticalFault())
        {
            SafeState_Enter(SafeState_Info.LastReason);
        }
    }
    
    /* Auto-clear non-critical faults after timeout */
    if ((SafeState_Info.Status == SAFESTATE_ACTIVE) &&
        (!SafeState_HasCriticalFault()))
    {
        if (SafeState_IsCooldownComplete())
        {
            SafeState_Info.Status = SAFESTATE_NORMAL;
        }
    }
}
