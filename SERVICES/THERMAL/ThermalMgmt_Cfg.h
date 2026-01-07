/**
 * @file ThermalMgmt_Cfg.h
 * @brief Configuration header for Thermal Management Service
 * @details Pre-compile configuration parameters
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef THERMALMGMT_CFG_H
#define THERMALMGMT_CFG_H

/* ===================[Pre-compile Options]=================== */

/**
 * @brief Enable/disable development error detection
 * @range STD_ON, STD_OFF
 */
#define THERMALMGMT_DEV_ERROR_DETECT        (STD_ON)

/**
 * @brief Enable/disable version info API
 * @range STD_ON, STD_OFF
 */
#define THERMALMGMT_VERSION_INFO_API        (STD_ON)

/**
 * @brief Enable/disable event callback
 * @range STD_ON, STD_OFF
 */
#define THERMALMGMT_EVENT_CALLBACK_API      (STD_ON)

/**
 * @brief Enable/disable thermal shutdown capability
 * @range STD_ON, STD_OFF
 * @note When enabled, system can request power reduction on critical temps
 */
#define THERMALMGMT_SHUTDOWN_SUPPORT        (STD_ON)

/**
 * @brief Default update period (ms)
 */
#define THERMALMGMT_DEFAULT_PERIOD_MS       (1000u)

/**
 * @brief Hysteresis for temperature thresholds (Celsius)
 * @note Prevents fan speed oscillation
 */
#define THERMALMGMT_HYSTERESIS_C            (2.0f)

/**
 * @brief Failed sensor temperature assumption
 * @note Temperature to assume if sensor fails (for safety)
 */
#define THERMALMGMT_FAIL_SAFE_TEMP_C        (70.0f)

/**
 * @brief Default thresholds for motor zone (Celsius)
 */
#define THERMALMGMT_MOTOR_LOW_THRESH        (35.0f)
#define THERMALMGMT_MOTOR_MED_THRESH        (50.0f)
#define THERMALMGMT_MOTOR_HIGH_THRESH       (65.0f)
#define THERMALMGMT_MOTOR_CRIT_THRESH       (80.0f)

/**
 * @brief Default thresholds for MCU zone (Celsius)
 */
#define THERMALMGMT_MCU_LOW_THRESH          (30.0f)
#define THERMALMGMT_MCU_MED_THRESH          (45.0f)
#define THERMALMGMT_MCU_HIGH_THRESH         (60.0f)
#define THERMALMGMT_MCU_CRIT_THRESH         (75.0f)

/**
 * @brief Default thresholds for battery zone (Celsius)
 */
#define THERMALMGMT_BATT_LOW_THRESH         (25.0f)
#define THERMALMGMT_BATT_MED_THRESH         (35.0f)
#define THERMALMGMT_BATT_HIGH_THRESH        (45.0f)
#define THERMALMGMT_BATT_CRIT_THRESH        (55.0f)

/**
 * @brief Default fan speeds (%)
 */
#define THERMALMGMT_FAN_OFF_SPEED           (0u)
#define THERMALMGMT_FAN_MIN_SPEED           (30u)
#define THERMALMGMT_FAN_MED_SPEED           (60u)
#define THERMALMGMT_FAN_MAX_SPEED           (100u)

#endif /* THERMALMGMT_CFG_H */
