/**
 * @file I2C_PBCfg.c
 * @brief I2C Post-Build Configuration for TM4C123GH6PM
 * @details Pre-configured I2C setups for common devices and use cases
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#include "I2C.h"
#include "I2C_Cfg.h"

/* ===================[I2C Module Configurations]=================== */

/**
 * @note Common I2C device addresses are defined in I2C_Cfg.h
 */

/**
 * @brief I2C0 Master Mode - Standard Speed (100kHz)
 * @usage General purpose I2C communication
 */
const I2C_ConfigType I2C0_Master_100kHz = {
    .Module = I2C_MODULE_0,
    .Mode = I2C_MODE_MASTER,
    .Speed = I2C_SPEED_STANDARD,
    .SlaveConfig = NULL_PTR
};

/**
 * @brief I2C0 Master Mode - Fast Speed (400kHz)
 * @usage High-speed I2C communication
 */
const I2C_ConfigType I2C0_Master_400kHz = {
    .Module = I2C_MODULE_0,
    .Mode = I2C_MODE_MASTER,
    .Speed = I2C_SPEED_FAST,
    .SlaveConfig = NULL_PTR
};

/**
 * @brief I2C1 Master Mode - Standard Speed (100kHz)
 */
const I2C_ConfigType I2C1_Master_100kHz = {
    .Module = I2C_MODULE_1,
    .Mode = I2C_MODE_MASTER,
    .Speed = I2C_SPEED_STANDARD,
    .SlaveConfig = NULL_PTR
};

/**
 * @brief I2C1 Master Mode - Fast Speed (400kHz)
 */
const I2C_ConfigType I2C1_Master_400kHz = {
    .Module = I2C_MODULE_1,
    .Mode = I2C_MODE_MASTER,
    .Speed = I2C_SPEED_FAST,
    .SlaveConfig = NULL_PTR
};

/**
 * @brief I2C2 Master Mode - Standard Speed (100kHz)
 */
const I2C_ConfigType I2C2_Master_100kHz = {
    .Module = I2C_MODULE_2,
    .Mode = I2C_MODE_MASTER,
    .Speed = I2C_SPEED_STANDARD,
    .SlaveConfig = NULL_PTR
};

/**
 * @brief I2C3 Master Mode - Standard Speed (100kHz)
 */
const I2C_ConfigType I2C3_Master_100kHz = {
    .Module = I2C_MODULE_3,
    .Mode = I2C_MODE_MASTER,
    .Speed = I2C_SPEED_STANDARD,
    .SlaveConfig = NULL_PTR
};

/* ===================[Slave Mode Configurations]=================== */

/**
 * @brief Example slave configuration
 * @note Modify OwnAddress for your specific application
 */
static I2C_SlaveConfigType I2C0_SlaveConfig_Example = {
    .OwnAddress = 0x42,              /* Example slave address */
    .DualAddressEnable = FALSE,
    .SecondaryAddress = 0x00
};

/**
 * @brief I2C0 Slave Mode Configuration
 */
const I2C_ConfigType I2C0_Slave_Example = {
    .Module = I2C_MODULE_0,
    .Mode = I2C_MODE_SLAVE,
    .Speed = I2C_SPEED_STANDARD,
    .SlaveConfig = &I2C0_SlaveConfig_Example
};

/* ===================[Default Configuration Pointer]=================== */

/**
 * @brief Default I2C configuration pointer
 * @note Points to I2C0 Master 100kHz by default
 */
const I2C_ConfigType* I2C_ConfigPtr = &I2C0_Master_100kHz;
