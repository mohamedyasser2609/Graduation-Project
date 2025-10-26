/*
 * @file Timer_Types.h
 * @brief Timer Type Definitions for TM4C123GH6PM
 * @details This file contains type definitions for the Timer driver.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_TIMER_TIMER_TYPES_H_
#define MCAL_TIMER_TIMER_TYPES_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[Timer Module Selection]=================== */
/**
 * @brief Timer module identifiers
 * @details TM4C123GH6PM has 6 general-purpose timer modules (0-5)
 */
typedef enum {
    TIMER_MODULE_0 = 0u,   /**< Timer Module 0 */
    TIMER_MODULE_1 = 1u,   /**< Timer Module 1 */
    TIMER_MODULE_2 = 2u,   /**< Timer Module 2 */
    TIMER_MODULE_3 = 3u,   /**< Timer Module 3 */
    TIMER_MODULE_4 = 4u,   /**< Timer Module 4 */
    TIMER_MODULE_5 = 5u    /**< Timer Module 5 */
} Timer_ModuleType;

/* ===================[Timer Block Selection]=================== */
/**
 * @brief Timer block identifiers (A or B within each module)
 */
typedef enum {
    TIMER_BLOCK_A = 0u,    /**< Timer A block */
    TIMER_BLOCK_B = 1u     /**< Timer B block */
} Timer_BlockType;

/* ===================[Timer Configuration Mode]=================== */
/**
 * @brief Timer configuration mode
 * @details Determines whether timer operates as 32-bit or dual 16-bit
 */
typedef enum {
    TIMER_CONFIG_32BIT = 0u,      /**< 32-bit timer configuration */
    TIMER_CONFIG_32BIT_RTC = 1u,  /**< 32-bit RTC configuration */
    TIMER_CONFIG_16BIT = 4u       /**< Dual 16-bit timer configuration */
} Timer_ConfigModeType;

/* ===================[Timer Operation Mode]=================== */
/**
 * @brief Timer operation modes
 */
typedef enum {
    TIMER_MODE_ONESHOT = 1u,      /**< One-shot timer mode */
    TIMER_MODE_PERIODIC = 2u,     /**< Periodic timer mode */
    TIMER_MODE_CAPTURE = 3u       /**< Input edge-time capture mode */
} Timer_OperationModeType;

/* ===================[Timer Count Direction]=================== */
/**
 * @brief Timer count direction
 */
typedef enum {
    TIMER_COUNT_DOWN = 0u,        /**< Count down */
    TIMER_COUNT_UP = 1u           /**< Count up */
} Timer_CountDirectionType;

/* ===================[Timer PWM Mode]=================== */
/**
 * @brief Timer PWM mode enable/disable
 */
typedef enum {
    TIMER_PWM_DISABLED = 0u,      /**< PWM mode disabled */
    TIMER_PWM_ENABLED = 1u        /**< PWM mode enabled */
} Timer_PwmModeType;

/* ===================[Timer Interrupt Type]=================== */
/**
 * @brief Timer interrupt types
 */
typedef enum {
    TIMER_INT_TIMEOUT = 0u,       /**< Timeout interrupt */
    TIMER_INT_MATCH = 1u,         /**< Match interrupt */
    TIMER_INT_CAPTURE_MATCH = 2u, /**< Capture match interrupt */
    TIMER_INT_CAPTURE_EVENT = 3u  /**< Capture event interrupt */
} Timer_InterruptType;

/* ===================[Timer State]=================== */
/**
 * @brief Timer running state
 */
typedef enum {
    TIMER_STATE_STOPPED = 0u,     /**< Timer is stopped */
    TIMER_STATE_RUNNING = 1u      /**< Timer is running */
} Timer_StateType;

/* ===================[Timer Value Type]=================== */
/**
 * @brief Timer value type (can be 16-bit or 32-bit depending on configuration)
 */
typedef uint32 Timer_ValueType;

/* ===================[Timer Notification Function Pointer]=================== */
/**
 * @brief Function pointer type for timer notification callbacks
 * @details User provides callback function to be called on timer events
 */
typedef void (*Timer_NotificationFuncPtr)(void);

/* ===================[Timer Configuration Structure]=================== */
/**
 * @brief Timer configuration structure
 * @details Contains all configuration parameters for a timer block
 */
typedef struct {
    Timer_ModuleType            Module;             /**< Timer module (0-5) */
    Timer_BlockType             Block;              /**< Timer block (A or B) */
    Timer_ConfigModeType        ConfigMode;         /**< 32-bit or 16-bit configuration */
    Timer_OperationModeType     OperationMode;      /**< One-shot, periodic, capture */
    Timer_CountDirectionType    CountDirection;     /**< Count up or down */
    Timer_PwmModeType           PwmMode;            /**< PWM enabled or disabled */
    Timer_ValueType             LoadValue;          /**< Timer load value */
    Timer_ValueType             MatchValue;         /**< Timer match value (for PWM/match interrupts) */
    uint8                       Prescaler;          /**< Prescaler value (extends timer range) */
    boolean                     InterruptEnable;    /**< Enable timeout interrupt */
    boolean                     MatchInterruptEnable; /**< Enable match interrupt */
    Timer_NotificationFuncPtr   TimeoutCallback;    /**< Callback for timeout events */
    Timer_NotificationFuncPtr   MatchCallback;      /**< Callback for match events */
} Timer_ConfigType;

#endif /* MCAL_TIMER_TIMER_TYPES_H_ */
