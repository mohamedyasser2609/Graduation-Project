/**
 * @file QEI_Types.h
 * @brief Quadrature Encoder Interface (QEI) Driver Types for TM4C123GH6PM
 * @details Type definitions used by the AUTOSAR-compliant QEI driver.
 */

#ifndef MCAL_QEI_QEI_TYPES_H_
#define MCAL_QEI_QEI_TYPES_H_

#include "../../CONFIG/Std_Types.h"

/**
 * @brief Available QEI modules
 */
typedef enum {
    QEI_MODULE_0 = 0u,
    QEI_MODULE_1
} Qei_ModuleType;

/**
 * @brief QEI signal mode selection
 */
typedef enum {
    QEI_SIGNAL_MODE_QUADRATURE = 0u,   /**< Use PhA/PhB quadrature signals */
    QEI_SIGNAL_MODE_DIRECTION          /**< Use direction + clock inputs */
} Qei_SignalModeType;

/**
 * @brief QEI reset mode selection
 */
typedef enum {
    QEI_RESET_MODE_FREE_RUNNING = 0u,  /**< Position counter free-running */
    QEI_RESET_MODE_ON_INDEX            /**< Reset position on index pulse */
} Qei_ResetModeType;

/**
 * @brief QEI velocity pre-divider
 */
typedef enum {
    QEI_VELOCITY_PREDIV_1 = 0u,
    QEI_VELOCITY_PREDIV_2,
    QEI_VELOCITY_PREDIV_4,
    QEI_VELOCITY_PREDIV_8,
    QEI_VELOCITY_PREDIV_16,
    QEI_VELOCITY_PREDIV_32,
    QEI_VELOCITY_PREDIV_64,
    QEI_VELOCITY_PREDIV_128
} Qei_VelocityPredivType;

/**
 * @brief QEI direction state
 */
typedef enum {
    QEI_DIRECTION_FORWARD = 0u,
    QEI_DIRECTION_REVERSE
} Qei_DirectionType;

/**
 * @brief QEI driver status
 */
typedef enum {
    QEI_STATUS_UNINIT = 0u,
    QEI_STATUS_IDLE,
    QEI_STATUS_RUNNING
} Qei_StatusType;

/**
 * @brief QEI interrupt mask bits
 */
#define QEI_INT_ERROR                   (0x01u)
#define QEI_INT_DIRECTION               (0x02u)
#define QEI_INT_TIMER                   (0x04u)
#define QEI_INT_INDEX                   (0x08u)

typedef uint32 Qei_InterruptMaskType;

/**
 * @brief QEI callback notification type
 * @param[in] interruptFlags Interrupt sources that triggered the callback
 */
typedef void (*Qei_NotificationType)(Qei_InterruptMaskType interruptFlags);

/**
 * @brief QEI configuration structure
 */
typedef struct {
    uint32 VelocityTimerLoad;                /**< Velocity timer load value */
    uint32 MaxPosition;                      /**< Maximum position count */
    uint32 InitialPosition;                  /**< Initial position value */
    Qei_InterruptMaskType InterruptMask;     /**< Interrupt mask */
    Qei_NotificationType NotificationCallback; /**< Optional callback */
    Qei_ModuleType Module;                   /**< QEI hardware module */
    Qei_SignalModeType SignalMode;           /**< Signal mode selection */
    Qei_ResetModeType ResetMode;             /**< Reset behavior */
    Qei_VelocityPredivType VelocityPreDiv;   /**< Velocity pre-divider */
    uint8 FilterCount;                       /**< Filter count (0-15) */
    boolean SwapChannels;                    /**< Swap PhA/PhB inputs */
    boolean InvertChannelA;                  /**< Invert PhA input */
    boolean InvertChannelB;                  /**< Invert PhB input */
    boolean InvertIndex;                     /**< Invert index input */
    boolean EnableVelocityCapture;           /**< Enable velocity capture */
    boolean EnableFilter;                    /**< Enable digital filter */
    boolean DebugStallEnable;                /**< Stall counter during debug */
} Qei_ConfigType;

#endif /* MCAL_QEI_QEI_TYPES_H_ */
