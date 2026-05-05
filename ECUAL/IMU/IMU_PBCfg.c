/**
 * @file IMU_PBCfg.c
 * @brief IMU Post-Build Configuration for MPU-9250
 * @details Pre-configured IMU setups for common use cases
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#include "IMU.h"

/* ===================[Pre-configured IMU Setups]=================== */

/**
 * @brief Default IMU configuration
 * @details ±2g accel, ±250°/s gyro, 100Hz mag, I2C0
 */
const IMU_ConfigType IMU_Config_Default = {
    .I2C_Module = I2C_MODULE_0,
    .DeviceAddress = MPU9250_I2C_ADDR_DEFAULT,
    .GyroRange = IMU_DEFAULT_GYRO_RANGE,
    .AccelRange = IMU_DEFAULT_ACCEL_RANGE
};

/**
 * @brief High sensitivity configuration
 * @details ±2g accel, ±250°/s gyro for precise measurements
 */
const IMU_ConfigType IMU_Config_HighSensitivity = {
    .I2C_Module = I2C_MODULE_0,
    .DeviceAddress = MPU9250_I2C_ADDR_DEFAULT,
    .GyroRange = MPU9250_GYRO_FS_250DPS,
    .AccelRange = MPU9250_ACCEL_FS_2G
};

/**
 * @brief High range configuration
 * @details ±16g accel, ±2000°/s gyro for high dynamic applications
 */
const IMU_ConfigType IMU_Config_HighRange = {
    .I2C_Module = I2C_MODULE_0,
    .DeviceAddress = MPU9250_I2C_ADDR_DEFAULT,
    .GyroRange = MPU9250_GYRO_FS_2000DPS,
    .AccelRange = MPU9250_ACCEL_FS_16G
};

/**
 * @brief Balanced configuration
 * @details ±4g accel, ±500°/s gyro for general purpose
 */
const IMU_ConfigType IMU_Config_Balanced = {
    .I2C_Module = I2C_MODULE_0,
    .DeviceAddress = MPU9250_I2C_ADDR_DEFAULT,
    .GyroRange = MPU9250_GYRO_FS_500DPS,
    .AccelRange = MPU9250_ACCEL_FS_4G
};

/**
 * @brief Drone/Quadcopter configuration
 * @details ±8g accel, ±1000°/s gyro for aerial vehicles
 */
const IMU_ConfigType IMU_Config_Drone = {
    .I2C_Module = I2C_MODULE_0,
    .DeviceAddress = MPU9250_I2C_ADDR_DEFAULT,
    .GyroRange = MPU9250_GYRO_FS_1000DPS,
    .AccelRange = MPU9250_ACCEL_FS_8G
};

/**
 * @brief Low power configuration
 * @details 8Hz magnetometer for battery-powered applications
 */
const IMU_ConfigType IMU_Config_LowPower = {
    .I2C_Module = I2C_MODULE_0,
    .DeviceAddress = MPU9250_I2C_ADDR_DEFAULT,
    .GyroRange = MPU9250_GYRO_FS_250DPS,
    .AccelRange = MPU9250_ACCEL_FS_2G
};

/**
 * @brief I2C1 configuration (alternate I2C bus)
 */
const IMU_ConfigType IMU_Config_I2C1 = {
    .I2C_Module = I2C_MODULE_1,
    .DeviceAddress = MPU9250_I2C_ADDR_DEFAULT,
    .GyroRange = IMU_DEFAULT_GYRO_RANGE,
    .AccelRange = IMU_DEFAULT_ACCEL_RANGE
};

/**
 * @brief Alternate address configuration (AD0 = HIGH)
 */
const IMU_ConfigType IMU_Config_AltAddress = {
    .I2C_Module = I2C_MODULE_0,
    .DeviceAddress = MPU9250_I2C_ADDR_AD0_HIGH,
    .GyroRange = IMU_DEFAULT_GYRO_RANGE,
    .AccelRange = IMU_DEFAULT_ACCEL_RANGE
};

/**
 * @brief Default configuration pointer
 */
const IMU_ConfigType* IMU_ConfigPtr = &IMU_Config_Default;
