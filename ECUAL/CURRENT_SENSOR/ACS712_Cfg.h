/**
 * @file ACS712_Cfg.h
 * @brief Configuration header for ACS712 Current Sensor Driver
 * @details Pre-compile configuration parameters
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef ACS712_CFG_H
#define ACS712_CFG_H

/* ===================[Pre-compile Options]=================== */

/**
 * @brief Enable/disable development error detection
 * @range STD_ON, STD_OFF
 */
#define ACS712_DEV_ERROR_DETECT         (STD_ON)

/**
 * @brief Enable/disable version info API
 * @range STD_ON, STD_OFF
 */
#define ACS712_VERSION_INFO_API         (STD_ON)

/**
 * @brief Enable/disable de-initialization API
 * @range STD_ON, STD_OFF
 */
#define ACS712_DEINIT_API               (STD_ON)

/**
 * @brief Enable/disable overload callback
 * @range STD_ON, STD_OFF
 */
#define ACS712_OVERLOAD_CALLBACK_API    (STD_ON)

/**
 * @brief ADC resolution in bits
 */
#define ACS712_ADC_RESOLUTION_BITS      (12u)

/**
 * @brief ADC maximum value
 */
#define ACS712_ADC_MAX_VALUE            (4095u)

/**
 * @brief Default ADC reference voltage
 */
#define ACS712_DEFAULT_VREF             (3.3f)

/**
 * @brief Default zero-current output voltage (VCC/2)
 */
#define ACS712_DEFAULT_ZERO_VOLTAGE     (2.5f)

/**
 * @brief Sensitivity values in mV/A for each model
 */
#define ACS712_SENSITIVITY_5A           (0.185f)  /* 185 mV/A */
#define ACS712_SENSITIVITY_20A          (0.100f)  /* 100 mV/A */
#define ACS712_SENSITIVITY_30A          (0.066f)  /* 66 mV/A */

/**
 * @brief Default number of samples for filtering
 */
#define ACS712_DEFAULT_FILTER_SAMPLES   (8u)

/**
 * @brief Maximum filter samples allowed
 */
#define ACS712_MAX_FILTER_SAMPLES       (32u)

#endif /* ACS712_CFG_H */
