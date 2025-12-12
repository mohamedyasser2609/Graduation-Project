/**
 * @file I2C_Types.h
 * @brief I2C Driver Type Definitions for TM4C123GH6PM
 * @details Type definitions used by the AUTOSAR-compliant I2C driver
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_I2C_I2C_TYPES_H_
#define MCAL_I2C_I2C_TYPES_H_

#include "../../CONFIG/Std_Types.h"

/* ===================[I2C Module Selection]=================== */
/**
 * @brief I2C module identifiers
 */
typedef enum {
    I2C_MODULE_0 = 0,  /**< I2C module 0 */
    I2C_MODULE_1 = 1,  /**< I2C module 1 */
    I2C_MODULE_2 = 2,  /**< I2C module 2 */
    I2C_MODULE_3 = 3   /**< I2C module 3 */
} I2C_ModuleType;

/* ===================[I2C Mode]=================== */
/**
 * @brief I2C operation mode
 */
typedef enum {
    I2C_MODE_MASTER = 0,  /**< Master mode */
    I2C_MODE_SLAVE = 1     /**< Slave mode */
} I2C_ModeType;

/* ===================[I2C Speed]=================== */
/**
 * @brief I2C bus speed selection
 */
typedef enum {
    I2C_SPEED_STANDARD = 0,    /**< Standard speed: 100 kHz */
    I2C_SPEED_FAST = 1          /**< Fast speed: 400 kHz */
} I2C_SpeedType;

/* ===================[I2C Slave Configuration]=================== */
/**
 * @brief I2C slave mode configuration structure
 */
typedef struct {
    uint8 OwnAddress;              /**< Slave's own 7-bit address */
    boolean DualAddressEnable;     /**< Enable secondary address */
    uint8 SecondaryAddress;        /**< Secondary 7-bit address (if enabled) */
} I2C_SlaveConfigType;

/* ===================[I2C Configuration]=================== */
/**
 * @brief I2C module configuration structure
 */
typedef struct {
    I2C_ModuleType Module;         /**< I2C module identifier */
    I2C_ModeType Mode;             /**< Operation mode (master/slave) */
    I2C_SpeedType Speed;           /**< Bus speed selection */
    I2C_SlaveConfigType* SlaveConfig;  /**< Slave configuration (NULL for master mode) */
} I2C_ConfigType;

/* ===================[I2C Status]=================== */
/**
 * @brief I2C bus status enumeration
 */
typedef enum {
    I2C_STATUS_IDLE = 0,              /**< Bus is idle */
    I2C_STATUS_BUSY,                  /**< Bus is busy with transaction */
    I2C_STATUS_ERROR,                 /**< General error occurred */
    I2C_STATUS_ARBITRATION_LOST,      /**< Arbitration lost */
    I2C_STATUS_DATA_NACK,             /**< Data byte not acknowledged */
    I2C_STATUS_ADDR_NACK              /**< Address not acknowledged */
} I2C_StatusType;

#endif /* MCAL_I2C_I2C_TYPES_H_ */
