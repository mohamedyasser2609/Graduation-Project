/**
 * @file MOTOR_Types.h
 * @brief Motor Driver Type Definitions for Cytron MDD10A Rev2.0
 * @details Type definitions used by the AUTOSAR-compliant motor driver
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef ECUAL_MOTOR_MOTOR_TYPES_H_
#define ECUAL_MOTOR_MOTOR_TYPES_H_

#include "../../CONFIG/Std_Types.h"
#include "../../MCAL/GPIO/GPIO.h"
#include "../../MCAL/PWM/PWM.h"

/* ===================[Motor Channel Identifiers]=================== */
/**
 * @brief Motor channel identifiers
 */
typedef enum {
    MOTOR_CHANNEL_LEFT = 0u,   /**< Left motor channel */
    MOTOR_CHANNEL_RIGHT,        /**< Right motor channel */
    MOTOR_CHANNEL_COUNT         /**< Number of motor channels */
} Motor_ChannelType;

/* ===================[Motor Direction]=================== */
/**
 * @brief Motor rotation direction
 */
typedef enum {
    MOTOR_DIRECTION_FORWARD = 0u,  /**< Forward rotation */
    MOTOR_DIRECTION_REVERSE,        /**< Reverse rotation */
    MOTOR_DIRECTION_BRAKE,          /**< Brake mode (short circuit) */
    MOTOR_DIRECTION_COAST           /**< Coast mode (high impedance) */
} Motor_DirectionType;

/* ===================[Motor Status]=================== */
/**
 * @brief Motor driver status
 */
typedef enum {
    MOTOR_STATUS_UNINIT = 0u,  /**< Motor not initialized */
    MOTOR_STATUS_IDLE,         /**< Motor idle (stopped) */
    MOTOR_STATUS_RUNNING,      /**< Motor running */
    MOTOR_STATUS_ERROR         /**< Motor error state */
} Motor_StatusType;

/* ===================[Motor Speed Type]=================== */
/**
 * @brief Motor speed representation
 * @details Speed is represented as a percentage (0-100)
 *          0 = stopped, 100 = maximum speed
 */
typedef uint8 Motor_SpeedType;

/* ===================[Motor Configuration]=================== */
/**
 * @brief Per-channel motor configuration structure
 */
typedef struct {
    Motor_ChannelType ChannelId;        /**< Logical channel identifier */
    Pwm_ChannelType PwmChannel;        /**< PWM channel for speed control */
    Gpio_ChannelType DirectionPin;     /**< GPIO pin for direction control */
    boolean InvertDirection;            /**< Invert direction logic (for reversed wiring) */
    Motor_SpeedType MaxSpeedPercent;    /**< Maximum speed limit (0-100) */
    Motor_SpeedType MinSpeedPercent;    /**< Minimum speed threshold (0-100) */
} Motor_ChannelConfigType;

/**
 * @brief Top-level motor configuration structure
 */
typedef struct {
    uint8 ChannelCount;                         /**< Number of configured channels */
    const Motor_ChannelConfigType* Channels;    /**< Pointer to channel configurations */
} Motor_ConfigType;

/**
 * @brief Motor data structure
 */
typedef struct {
    Motor_StatusType Status;           /**< Current motor status */
    Motor_DirectionType Direction;      /**< Current direction */
    Pwm_DutyCycleType PwmDutyCycle;    /**< Current PWM duty cycle */
    Motor_SpeedType SpeedPercent;      /**< Current speed (0-100) */
} Motor_DataType;

#endif /* ECUAL_MOTOR_MOTOR_TYPES_H_ */

