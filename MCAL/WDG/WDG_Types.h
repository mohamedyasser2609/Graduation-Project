/**
 * @file WDG_Types.h
 * @brief Watchdog (WDG) Driver Types for TM4C123GH6PM
 * @details Type definitions used by the AUTOSAR-compliant Watchdog driver.
 */

#ifndef MCAL_WDG_WDG_TYPES_H_
#define MCAL_WDG_WDG_TYPES_H_

#include "../../CONFIG/Std_Types.h"

/**
 * @brief Watchdog Timer instances
 */
typedef enum {
    WDG_INSTANCE_0 = 0u,   /**< Watchdog 0 */
    WDG_INSTANCE_1        /**< Watchdog 1 */
} Wdg_InstanceType;

/**
 * @brief Watchdog trigger mode
 */
typedef enum {
    WDG_TRIGGER_MODE_NORMAL = 0u, /**< Normal mode (trigger once per cycle) */
    WDG_TRIGGER_MODE_FAST         /**< Fast mode (allows short-time triggers) */
} Wdg_TriggerModeType;

/**
 * @brief Watchdog notification callback type
 */
typedef void (*Wdg_NotificationType)(void);

/**
 * @brief Watchdog driver configuration type
 */
typedef struct {
    Wdg_InstanceType Instance;      /**< Watchdog hardware instance */
    uint32 InitialTimeoutTicks;     /**< Timeout value to load at initialization */
    uint32 MaxTimeoutTicks;         /**< Maximum allowed timeout value */
    uint32 MinTimeoutTicks;         /**< Minimum allowed timeout value */
    boolean ResetEnable;            /**< TRUE to enable system reset on timeout */
    boolean InterruptEnable;        /**< TRUE to enable watchdog interrupt */
    Wdg_NotificationType NotificationCallback; /**< Notification callback */
} Wdg_ConfigType;

/**
 * @brief WDG status type
 */
typedef enum {
    WDG_STATUS_UNINIT = 0u, /**< Watchdog driver not initialized */
    WDG_STATUS_IDLE,        /**< Watchdog initialized but not running */
    WDG_STATUS_RUNNING      /**< Watchdog running */
} Wdg_StatusType;

#endif /* MCAL_WDG_WDG_TYPES_H_ */
