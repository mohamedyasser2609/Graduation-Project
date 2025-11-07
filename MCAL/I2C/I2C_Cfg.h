/**
 * @file I2C_Cfg.h
 * @brief I2C Driver Configuration for TM4C123GH6PM
 * @details Configuration header for AUTOSAR-compliant I2C driver
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_I2C_I2C_CFG_H_
#define MCAL_I2C_I2C_CFG_H_

#include "../../CONFIG/Std_Types.h"

/* ===================[Pre-compile Configuration Parameters]=================== */

/**
 * @brief I2C driver pre-compile configuration switches
 */
#define I2C_DEV_ERROR_DETECT            (STD_ON)    /**< Enable/Disable development error detection */
#define I2C_VERSION_INFO_API            (STD_ON)    /**< Enable/Disable I2C_GetVersionInfo API */
#define I2C_TIMEOUT_ENABLE              (STD_ON)    /**< Enable/Disable timeout protection */

/* ===================[API Service IDs]=================== */

/**
 * @brief Service ID for I2C_Init
 */
#define I2C_INIT_SID                           (0x00u)

/**
 * @brief Service ID for I2C_DeInit
 */
#define I2C_DEINIT_SID                         (0x01u)

/**
 * @brief Service ID for I2C_MasterTransmit
 */
#define I2C_MASTER_TRANSMIT_SID                (0x02u)

/**
 * @brief Service ID for I2C_MasterReceive
 */
#define I2C_MASTER_RECEIVE_SID                 (0x03u)

/**
 * @brief Service ID for I2C_WriteRegister
 */
#define I2C_WRITE_REGISTER_SID                 (0x04u)

/**
 * @brief Service ID for I2C_ReadRegister
 */
#define I2C_READ_REGISTER_SID                  (0x05u)

/**
 * @brief Service ID for I2C_GetStatus
 */
#define I2C_GET_STATUS_SID                     (0x06u)

/**
 * @brief Service ID for I2C_ScanBus
 */
#define I2C_SCAN_BUS_SID                       (0x07u)

/* ===================[DET Error Codes]=================== */

/**
 * @brief API service called with wrong parameter
 */
#define I2C_E_PARAM_CONFIG                     (0x0Au)

/**
 * @brief API service called with invalid module
 */
#define I2C_E_PARAM_MODULE                     (0x0Bu)

/**
 * @brief API service called with NULL pointer
 */
#define I2C_E_PARAM_POINTER                    (0x0Cu)

/**
 * @brief API service called without module initialization
 */
#define I2C_E_UNINIT                           (0x0Du)

/**
 * @brief I2C timeout occurred
 */
#define I2C_E_TIMEOUT                          (0x0Eu)

/**
 * @brief I2C bus error occurred
 */
#define I2C_E_BUS_ERROR                        (0x0Fu)

/* ===================[I2C Timing Parameters]=================== */

/**
 * @brief Default timeout value for I2C operations (in loop iterations)
 */
#define I2C_DEFAULT_TIMEOUT                    (100000UL)

/**
 * @brief System clock frequency for I2C timing calculations
 * @note This should match the MCU system clock frequency
 */
#define I2C_SYSTEM_CLOCK_HZ                    (80000000UL)

/* ===================[Common I2C Device Addresses]=================== */

/* Accelerometer/Gyroscope Sensors */
#define I2C_ADDR_MPU6050                       (0x68u)    /**< MPU6050 IMU default address */
#define I2C_ADDR_MPU6050_ALT                   (0x69u)    /**< MPU6050 IMU alternate address */
#define I2C_ADDR_ADXL345                       (0x53u)    /**< ADXL345 Accelerometer */

/* Pressure/Temperature Sensors */
#define I2C_ADDR_BMP280                        (0x76u)    /**< BMP280 Pressure sensor default */
#define I2C_ADDR_BMP280_ALT                    (0x77u)    /**< BMP280 Pressure sensor alternate */
#define I2C_ADDR_BME280                        (0x76u)    /**< BME280 Environmental sensor */

/* EEPROM */
#define I2C_ADDR_EEPROM_24C02                  (0x50u)    /**< 24C02 EEPROM (2Kbit) */
#define I2C_ADDR_EEPROM_24C32                  (0x50u)    /**< 24C32 EEPROM (32Kbit) */
#define I2C_ADDR_EEPROM_24C256                 (0x50u)    /**< 24C256 EEPROM (256Kbit) */

/* Real-Time Clock */
#define I2C_ADDR_DS1307                        (0x68u)    /**< DS1307 RTC */
#define I2C_ADDR_DS3231                        (0x68u)    /**< DS3231 RTC */

/* LCD Displays */
#define I2C_ADDR_LCD_PCF8574                   (0x27u)    /**< PCF8574 I2C LCD backpack */
#define I2C_ADDR_LCD_PCF8574_ALT               (0x3Fu)    /**< PCF8574 alternate address */

/* IO Expanders */
#define I2C_ADDR_PCF8574                       (0x20u)    /**< PCF8574 8-bit IO expander */
#define I2C_ADDR_MCP23017                      (0x20u)    /**< MCP23017 16-bit IO expander */

/* ===================[External Configuration Declarations]=================== */

/**
 * @brief Note: I2C_ConfigType is defined in I2C.h
 * @note External declarations are provided here for configuration files
 * @note Include I2C.h to use these configurations
 */

#endif /* MCAL_I2C_I2C_CFG_H_ */
