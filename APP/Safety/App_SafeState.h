/**
 * @file App_SafeState.h
 * @brief Safe State Manager for UGV Low-Level Controller
 * @details Defines safe state behavior and privilege separation model
 *
 * Safety Design:
 * - Defines all fault conditions and responses
 * - Provides motor enable gate controlled ONLY by Safety Task
 * - Ensures fail-safe motor behavior
 *
 * @author Mohamed Yasser
 * @date Jan 09, 2026
 * @version 1.0.0
 */

#ifndef APP_SAFESTATE_H
#define APP_SAFESTATE_H

#include "../../CONFIG/Std_Types.h"

/* ===================[Fault Definitions]=================== */

/**
 * @brief Fault reason codes
 */
typedef enum
{
    SAFESTATE_REASON_NONE               = 0x00u,
    SAFESTATE_REASON_MOTOR_LEFT_OVERLOAD = 0x01u,
    SAFESTATE_REASON_MOTOR_RIGHT_OVERLOAD = 0x02u,
    SAFESTATE_REASON_MOTOR_LEFT_THERMAL  = 0x03u,
    SAFESTATE_REASON_MOTOR_RIGHT_THERMAL = 0x04u,
    SAFESTATE_REASON_ENCLOSURE_THERMAL   = 0x05u,
    SAFESTATE_REASON_ENCODER_LEFT_FAULT  = 0x06u,
    SAFESTATE_REASON_ENCODER_RIGHT_FAULT = 0x07u,
    SAFESTATE_REASON_COMMAND_TIMEOUT     = 0x08u,
    SAFESTATE_REASON_HEARTBEAT_TIMEOUT   = 0x09u,
    SAFESTATE_REASON_WATCHDOG_WARNING    = 0x0Au,
    SAFESTATE_REASON_ESTOP_COMMAND       = 0x0Bu,
    SAFESTATE_REASON_SENSOR_FAULT        = 0x0Cu,
    SAFESTATE_REASON_INTERNAL_ERROR      = 0xFFu
} SafeState_ReasonType;

/**
 * @brief Safe state status
 */
typedef enum
{
    SAFESTATE_NORMAL = 0u,      /**< Normal operation */
    SAFESTATE_WARNING = 1u,     /**< Warning - degraded operation */
    SAFESTATE_ACTIVE = 2u,      /**< Safe state active - motors stopped */
    SAFESTATE_LOCKOUT = 3u      /**< Requires manual reset */
} SafeState_StatusType;

/**
 * @brief Complete safe state information
 */
typedef struct
{
    SafeState_StatusType    Status;
    SafeState_ReasonType    LastReason;
    uint32                  FaultFlags;
    uint32                  EntryTimestamp;
    uint8                   FaultCount;
    boolean                 MotorEnableAllowed;
} SafeState_InfoType;

/* ===================[Fault Flag Bits]=================== */
#define FAULT_FLAG_MOTOR_L_OVERLOAD     (1u << 0u)
#define FAULT_FLAG_MOTOR_R_OVERLOAD     (1u << 1u)
#define FAULT_FLAG_MOTOR_L_THERMAL      (1u << 2u)
#define FAULT_FLAG_MOTOR_R_THERMAL      (1u << 3u)
#define FAULT_FLAG_ENCLOSURE_THERMAL    (1u << 4u)
#define FAULT_FLAG_ENCODER_L_FAULT      (1u << 5u)
#define FAULT_FLAG_ENCODER_R_FAULT      (1u << 6u)
#define FAULT_FLAG_CMD_TIMEOUT          (1u << 7u)
#define FAULT_FLAG_HEARTBEAT_TIMEOUT    (1u << 8u)
#define FAULT_FLAG_WDG_WARNING          (1u << 9u)
#define FAULT_FLAG_ESTOP                (1u << 10u)

/* ===================[Public API]=================== */

/**
 * @brief Initialize safe state manager
 */
void SafeState_Init(void);

/**
 * @brief Enter safe state with specified reason
 * @param[in] Reason The fault reason
 * @note This will STOP ALL MOTORS immediately
 */
void SafeState_Enter(SafeState_ReasonType Reason);

/**
 * @brief Clear safe state and allow recovery
 * @return E_OK if recovery allowed, E_NOT_OK if lockout
 */
Std_ReturnType SafeState_Clear(void);

/**
 * @brief Set a fault flag
 * @param[in] FaultFlag Fault flag bit to set
 */
void SafeState_SetFault(uint32 FaultFlag);

/**
 * @brief Clear a fault flag
 * @param[in] FaultFlag Fault flag bit to clear
 */
void SafeState_ClearFault(uint32 FaultFlag);

/**
 * @brief Check if motor enable is allowed
 * @return TRUE if motors may be enabled
 * @note This is the PRIVILEGE GATE - Control Task must check this
 */
boolean SafeState_IsMotorEnableAllowed(void);

/**
 * @brief Request motor enable (called by Control Task)
 * @return TRUE if request granted
 * @note Safety Task evaluates conditions before granting
 */
boolean SafeState_RequestMotorEnable(void);

/**
 * @brief Request motor disable (e-stop request)
 */
void SafeState_RequestMotorDisable(void);

/**
 * @brief Get current safe state status
 * @return Current status
 */
SafeState_StatusType SafeState_GetStatus(void);

/**
 * @brief Get complete safe state information
 * @param[out] InfoPtr Pointer to info structure
 * @return E_OK on success
 */
Std_ReturnType SafeState_GetInfo(SafeState_InfoType* InfoPtr);

/**
 * @brief Get current fault flags
 * @return Fault flag bitmap
 */
uint32 SafeState_GetFaultFlags(void);

/**
 * @brief Check if any critical fault is active
 * @return TRUE if critical fault active
 */
boolean SafeState_HasCriticalFault(void);

/**
 * @brief Update from Safety Task (10ms cycle)
 * @note Called ONLY by Safety Task
 */
void SafeState_Update(void);

#endif /* APP_SAFESTATE_H */
