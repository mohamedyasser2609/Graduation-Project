/**
 * @file FAN_Cfg.h
 * @brief Configuration header for Fan Control Driver
 * @details Pre-compile configuration parameters for Delta FFB0812EHE fans
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef FAN_CFG_H
#define FAN_CFG_H

/* ===================[Pre-compile Options]=================== */

/**
 * @brief Enable/disable development error detection
 * @range STD_ON, STD_OFF
 */
#define FAN_DEV_ERROR_DETECT            (STD_ON)

/**
 * @brief Enable/disable version info API
 * @range STD_ON, STD_OFF
 */
#define FAN_VERSION_INFO_API            (STD_ON)

/**
 * @brief Enable/disable de-initialization API
 * @range STD_ON, STD_OFF
 */
#define FAN_DEINIT_API                  (STD_ON)

/**
 * @brief Enable/disable tachometer feedback
 * @range STD_ON, STD_OFF
 */
#define FAN_TACHOMETER_SUPPORT          (STD_OFF)

/**
 * @brief Enable/disable fault callback
 * @range STD_ON, STD_OFF
 */
#define FAN_FAULT_CALLBACK_API          (STD_ON)

/**
 * @brief PWM frequency for fan control (Hz)
 * @note Delta FFB0812EHE supports 25kHz PWM
 */
#define FAN_PWM_FREQUENCY_HZ            (25000u)

/**
 * @brief Minimum startup duty cycle (%)
 * @note Some fans need minimum duty to start spinning
 */
#define FAN_DEFAULT_MIN_DUTY            (20u)

/**
 * @brief Stall detection timeout (ms)
 * @note Time without tach pulses before stall is reported
 */
#define FAN_STALL_TIMEOUT_MS            (2000u)

/**
 * @brief Delta FFB0812EHE specifications
 */
#define FAN_FFB0812EHE_MAX_RPM          (12000u)
#define FAN_FFB0812EHE_PULSES_PER_REV   (2u)
#define FAN_FFB0812EHE_MIN_DUTY         (20u)

#endif /* FAN_CFG_H */
