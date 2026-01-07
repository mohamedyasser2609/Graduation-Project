/**
 * @file AM2320_Cfg.h
 * @brief Configuration header for AM2320 Temperature & Humidity Sensor
 * @details Pre-compile configuration parameters
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef AM2320_CFG_H
#define AM2320_CFG_H

/* ===================[Pre-compile Options]=================== */

/**
 * @brief Enable/disable development error detection
 * @range STD_ON, STD_OFF
 */
#define AM2320_DEV_ERROR_DETECT         (STD_ON)

/**
 * @brief Enable/disable version info API
 * @range STD_ON, STD_OFF
 */
#define AM2320_VERSION_INFO_API         (STD_ON)

/**
 * @brief Enable/disable de-initialization API
 * @range STD_ON, STD_OFF
 */
#define AM2320_DEINIT_API               (STD_ON)

/**
 * @brief Enable/disable temperature alarm callback
 * @range STD_ON, STD_OFF
 */
#define AM2320_ALARM_CALLBACK_API       (STD_ON)

/**
 * @brief Wakeup delay in microseconds (>800us required)
 */
#define AM2320_WAKEUP_DELAY_US          (1000u)

/**
 * @brief Measurement delay in microseconds (>1.5ms typical)
 */
#define AM2320_MEASURE_DELAY_US         (2000u)

/**
 * @brief I2C timeout in milliseconds
 */
#define AM2320_I2C_TIMEOUT_MS           (100u)

/**
 * @brief Number of retry attempts on communication failure
 */
#define AM2320_RETRY_COUNT              (3u)

/**
 * @brief Default temperature high threshold (Celsius)
 */
#define AM2320_DEFAULT_TEMP_HIGH        (60.0f)

/**
 * @brief Default temperature low threshold (Celsius)
 */
#define AM2320_DEFAULT_TEMP_LOW         (10.0f)

#endif /* AM2320_CFG_H */
