/**
 * @file FAN_Types.h
 * @brief Type definitions for Fan Control Driver
 * @details AUTOSAR-compliant types for Delta FFB0812EHE fan control
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#ifndef FAN_TYPES_H
#define FAN_TYPES_H

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[Macros]=================== */
/** @brief Module identification */
#define FAN_MODULE_ID               (0x82u)
#define FAN_VENDOR_ID               (0x00u)
#define FAN_INSTANCE_ID             (0x00u)

/** @brief Software version */
#define FAN_SW_MAJOR_VERSION        (1u)
#define FAN_SW_MINOR_VERSION        (0u)
#define FAN_SW_PATCH_VERSION        (0u)

/** @brief Development error codes */
#define FAN_E_PARAM_POINTER         (0x10u)
#define FAN_E_PARAM_FAN             (0x11u)
#define FAN_E_UNINIT                (0x12u)
#define FAN_E_ALREADY_INIT          (0x13u)
#define FAN_E_PARAM_CONFIG          (0x14u)
#define FAN_E_PARAM_SPEED           (0x15u)

/** @brief API service IDs */
#define FAN_INIT_SID                (0x00u)
#define FAN_DEINIT_SID              (0x01u)
#define FAN_SET_SPEED_SID           (0x02u)
#define FAN_GET_STATUS_SID          (0x03u)
#define FAN_GET_VERSION_SID         (0x04u)

/** @brief Maximum number of fans */
#define FAN_MAX_FANS                (4u)

/** @brief Speed limits */
#define FAN_SPEED_MIN               (0u)
#define FAN_SPEED_MAX               (100u)

/* ===================[Type Definitions]=================== */

/**
 * @brief Fan identifier type
 */
typedef uint8 Fan_IdType;

/**
 * @brief Fan speed type (0-100%)
 */
typedef uint8 Fan_SpeedType;

/**
 * @brief Driver status
 */
typedef enum
{
    FAN_STATUS_UNINIT     = 0u,     /**< Driver not initialized */
    FAN_STATUS_IDLE       = 1u,     /**< Initialized, idle */
    FAN_STATUS_RUNNING    = 2u,     /**< At least one fan running */
    FAN_STATUS_ERROR      = 3u      /**< Error occurred */
} Fan_StatusType;

/**
 * @brief Individual fan status
 */
typedef enum
{
    FAN_STATE_OFF         = 0u,     /**< Fan is off */
    FAN_STATE_RUNNING     = 1u,     /**< Fan is running */
    FAN_STATE_STALLED     = 2u,     /**< Fan stall detected (if tach available) */
    FAN_STATE_ERROR       = 3u      /**< Fan error */
} Fan_StateType;

/**
 * @brief Control mode
 */
typedef enum
{
    FAN_MODE_PWM          = 0u,     /**< PWM speed control */
    FAN_MODE_ONOFF        = 1u      /**< Simple on/off control */
} Fan_ModeType;

/**
 * @brief Fan runtime data
 */
typedef struct
{
    Fan_SpeedType    CurrentSpeed;   /**< Current speed setting (0-100%) */
    uint16           RpmMeasured;    /**< Measured RPM (if tach available) */
    Fan_StateType    State;          /**< Fan state */
} Fan_DataType;

/**
 * @brief Fan configuration structure
 */
typedef struct
{
    Fan_IdType       FanId;          /**< Logical fan identifier */
    Fan_ModeType     Mode;           /**< Control mode */
    uint8            PwmChannel;     /**< PWM channel for speed control */
    uint8            TachChannel;    /**< Timer channel for tachometer (or 0xFF if unused) */
    uint8            MinDutyPercent; /**< Minimum duty to start fan */
    uint16           MaxRpm;         /**< Maximum rated RPM */
    uint8            PulsesPerRev;   /**< Tach pulses per revolution (typically 2) */
} Fan_ConfigItemType;

/**
 * @brief Driver configuration structure
 */
typedef struct
{
    uint8                       NumFans;        /**< Number of configured fans */
    const Fan_ConfigItemType*   Fans;           /**< Array of fan configs */
} Fan_ConfigType;

/**
 * @brief Callback for fan fault notification
 */
typedef void (*Fan_FaultCallbackType)(Fan_IdType FanId, Fan_StateType State);

#endif /* FAN_TYPES_H */
