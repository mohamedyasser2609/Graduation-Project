/**
 * @file App_ResourceMap.h
 * @brief Hardware Resource Ownership Map
 * @details Documents which task owns each hardware resource
 *
 * OWNERSHIP RULES:
 * 1. Each resource has ONE owner task
 * 2. Owner has exclusive write access
 * 3. Read-only sharing allowed with mutex protection
 * 4. Inter-task communication via queues only
 *
 * @author Mohamed Yasser
 * @date Jan 09, 2026
 * @version 1.0.0
 */

#ifndef APP_RESOURCEMAP_H
#define APP_RESOURCEMAP_H

#include "../../CONFIG/Std_Types.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

/* ===================[Resource Ownership Definitions]=================== */

/**
 * @brief Resource owner enumeration
 */
typedef enum
{
    RESOURCE_OWNER_NONE = 0u,
    RESOURCE_OWNER_SAFETY,
    RESOURCE_OWNER_CONTROL,
    RESOURCE_OWNER_SENSOR,
    RESOURCE_OWNER_COMM,
    RESOURCE_OWNER_THERMAL,
    RESOURCE_OWNER_SHARED_RO     /* Shared read-only */
} ResourceOwnerType;

/**
 * @brief Resource type enumeration
 */
typedef enum
{
    RESOURCE_MOTOR_LEFT = 0u,
    RESOURCE_MOTOR_RIGHT,
    RESOURCE_ENCODER_LEFT,
    RESOURCE_ENCODER_RIGHT,
    RESOURCE_IMU,
    RESOURCE_GPS,
    RESOURCE_CURRENT_LEFT,
    RESOURCE_CURRENT_RIGHT,
    RESOURCE_TEMP_SENSOR_0,
    RESOURCE_TEMP_SENSOR_1,
    RESOURCE_TEMP_SENSOR_2,
    RESOURCE_FAN_0,
    RESOURCE_FAN_1,
    RESOURCE_WATCHDOG,
    RESOURCE_ROS_UART,
    RESOURCE_DEBUG_UART,
    RESOURCE_COUNT
} ResourceType;

/* ===================[Hardware Resource Map]=================== */

/*
 * RESOURCE OWNERSHIP TABLE
 * ========================
 *
 * | Resource        | Owner      | Access   | Bus     | Notes                    |
 * |-----------------|------------|----------|---------|--------------------------|
 * | Motor Left PWM  | Control    | Write    | PWM0    | Gated by SafeState       |
 * | Motor Right PWM | Control    | Write    | PWM0    | Gated by SafeState       |
 * | Encoder Left    | Sensor+Ctrl| Read     | QEI0    | Mutex protected          |
 * | Encoder Right   | Sensor+Ctrl| Read     | QEI1    | Mutex protected          |
 * | IMU (MPU-9250)  | Sensor     | R/W      | I2C0    | Exclusive                |
 * | GPS (NEO-M8N)   | Sensor     | Read     | UART0   | Exclusive                |
 * | Current Left    | Safety     | Read     | ADC0    | Exclusive                |
 * | Current Right   | Safety     | Read     | ADC1    | Exclusive                |
 * | Temp Sensor 0   | Thermal    | Read     | I2C1    | Exclusive                |
 * | Temp Sensor 1   | Thermal    | Read     | I2C1    | Exclusive                |
 * | Temp Sensor 2   | Thermal    | Read     | I2C1    | Exclusive                |
 * | Fan 0           | Thermal    | Write    | PWM1    | Exclusive                |
 * | Fan 1           | Thermal    | Write    | PWM1    | Exclusive                |
 * | Watchdog        | Safety     | Write    | WDT0    | MPU protected            |
 * | ROS UART        | Comm       | R/W      | UART1   | Exclusive                |
 * | Debug UART      | Diag       | Write    | UART0   | Shared (low priority)    |
 *
 * BUS CONFLICT NOTES:
 * - I2C0: IMU only (Sensor Task)
 * - I2C1: AM2320 sensors (Thermal Task)
 * - UART0: GPS RX + Debug TX (time-sliced)
 * - UART1: ROS communication (Comm Task exclusive)
 */

/* ===================[Shared Resource Mutexes]=================== */

/**
 * @brief Mutex for encoder access (shared between Sensor and Control)
 */
extern SemaphoreHandle_t Mutex_Encoder;

/**
 * @brief Mutex for I2C0 bus (if shared)
 */
extern SemaphoreHandle_t Mutex_I2C0;

/* ===================[Safety Status Queue]=================== */

/**
 * @brief Safety status for fault reporting to Comm task
 */
typedef struct
{
    uint32  FaultFlags;         /**< Active fault bitmap */
    uint32  Timestamp;          /**< Fault detection time */
    uint8   SafeStateStatus;    /**< SafeState_StatusType */
    uint8   LastFaultReason;    /**< SafeState_ReasonType */
    boolean MotorEnabled;       /**< Motor enable status */
    boolean RequiresAck;        /**< Fault requires ROS acknowledgement */
} SafetyStatusMsgType;

/**
 * @brief Queue for safety status (Safety -> Comm)
 */
extern QueueHandle_t Queue_SafetyStatus;

/* ===================[API Functions]=================== */

/**
 * @brief Initialize all resource management structures
 * @return E_OK if successful
 */
Std_ReturnType ResourceMap_Init(void);

/**
 * @brief Acquire mutex for encoder access
 * @param[in] TimeoutMs Maximum wait time
 * @return TRUE if acquired
 */
boolean ResourceMap_AcquireEncoder(uint32 TimeoutMs);

/**
 * @brief Release encoder mutex
 */
void ResourceMap_ReleaseEncoder(void);

/**
 * @brief Check if calling task owns a resource
 * @param[in] Resource Resource to check
 * @return TRUE if caller owns resource
 */
boolean ResourceMap_IsOwner(ResourceType Resource);

/**
 * @brief Send safety status to Comm task
 * @param[in] Status Safety status message
 * @return E_OK if sent
 */
Std_ReturnType ResourceMap_SendSafetyStatus(const SafetyStatusMsgType* Status);

/**
 * @brief Receive safety status in Comm task
 * @param[out] Status Safety status message
 * @param[in] TimeoutMs Maximum wait time
 * @return E_OK if received
 */
Std_ReturnType ResourceMap_ReceiveSafetyStatus(SafetyStatusMsgType* Status, uint32 TimeoutMs);

#endif /* APP_RESOURCEMAP_H */
