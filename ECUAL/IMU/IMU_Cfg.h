/**
 * @file IMU_Cfg.h
 * @brief IMU Driver Configuration for MPU-9250
 * @details Configuration header for AUTOSAR-compliant IMU driver
 *          All register addresses verified against MPU-9250 datasheet
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef ECUAL_IMU_IMU_CFG_H_
#define ECUAL_IMU_IMU_CFG_H_

#include "../../CONFIG/Std_Types.h"

/* ===================[MPU-9250 I2C Addresses]=================== */

/**
 * @brief MPU-9250 7-bit I2C addresses
 * @note AD0 pin determines address: LOW=0x68, HIGH=0x69
 */
#define MPU9250_I2C_ADDR_AD0_LOW        0x68u    /**< AD0 pin connected to GND */
#define MPU9250_I2C_ADDR_AD0_HIGH       0x69u    /**< AD0 pin connected to VDD */
#define MPU9250_I2C_ADDR_DEFAULT        MPU9250_I2C_ADDR_AD0_LOW


/* ===================[MPU-9250 Register Map]=================== */

/* Configuration Registers */
#define MPU9250_SELF_TEST_X_GYRO        0x00u    /**< Gyro self-test X */
#define MPU9250_SELF_TEST_Y_GYRO        0x01u    /**< Gyro self-test Y */
#define MPU9250_SELF_TEST_Z_GYRO        0x02u    /**< Gyro self-test Z */
#define MPU9250_SELF_TEST_X_ACCEL       0x0Du    /**< Accel self-test X */
#define MPU9250_SELF_TEST_Y_ACCEL       0x0Eu    /**< Accel self-test Y */
#define MPU9250_SELF_TEST_Z_ACCEL       0x0Fu    /**< Accel self-test Z */

#define MPU9250_XG_OFFSET_H             0x13u    /**< Gyro offset X high byte */
#define MPU9250_XG_OFFSET_L             0x14u    /**< Gyro offset X low byte */
#define MPU9250_YG_OFFSET_H             0x15u    /**< Gyro offset Y high byte */
#define MPU9250_YG_OFFSET_L             0x16u    /**< Gyro offset Y low byte */
#define MPU9250_ZG_OFFSET_H             0x17u    /**< Gyro offset Z high byte */
#define MPU9250_ZG_OFFSET_L             0x18u    /**< Gyro offset Z low byte */

#define MPU9250_SMPLRT_DIV              0x19u    /**< Sample rate divider */
#define MPU9250_CONFIG                  0x1Au    /**< Configuration */
#define MPU9250_GYRO_CONFIG             0x1Bu    /**< Gyroscope configuration */
#define MPU9250_ACCEL_CONFIG            0x1Cu    /**< Accelerometer configuration */
#define MPU9250_ACCEL_CONFIG2           0x1Du    /**< Accelerometer configuration 2 */
#define MPU9250_LP_ACCEL_ODR            0x1Eu    /**< Low power accelerometer ODR */
#define MPU9250_WOM_THR                 0x1Fu    /**< Wake-on-motion threshold */

#define MPU9250_FIFO_EN                 0x23u    /**< FIFO enable */

/* I2C Master Control */
#define MPU9250_I2C_MST_CTRL            0x24u    /**< I2C master control */
#define MPU9250_I2C_SLV0_ADDR           0x25u    /**< I2C slave 0 address */
#define MPU9250_I2C_SLV0_REG            0x26u    /**< I2C slave 0 register */
#define MPU9250_I2C_SLV0_CTRL           0x27u    /**< I2C slave 0 control */
#define MPU9250_I2C_SLV1_ADDR           0x28u    /**< I2C slave 1 address */
#define MPU9250_I2C_SLV1_REG            0x29u    /**< I2C slave 1 register */
#define MPU9250_I2C_SLV1_CTRL           0x2Au    /**< I2C slave 1 control */
#define MPU9250_I2C_SLV2_ADDR           0x2Bu    /**< I2C slave 2 address */
#define MPU9250_I2C_SLV2_REG            0x2Cu    /**< I2C slave 2 register */
#define MPU9250_I2C_SLV2_CTRL           0x2Du    /**< I2C slave 2 control */
#define MPU9250_I2C_SLV3_ADDR           0x2Eu    /**< I2C slave 3 address */
#define MPU9250_I2C_SLV3_REG            0x2Fu    /**< I2C slave 3 register */
#define MPU9250_I2C_SLV3_CTRL           0x30u    /**< I2C slave 3 control */
#define MPU9250_I2C_SLV4_ADDR           0x31u    /**< I2C slave 4 address */
#define MPU9250_I2C_SLV4_REG            0x32u    /**< I2C slave 4 register */
#define MPU9250_I2C_SLV4_DO             0x33u    /**< I2C slave 4 data out */
#define MPU9250_I2C_SLV4_CTRL           0x34u    /**< I2C slave 4 control */
#define MPU9250_I2C_SLV4_DI             0x35u    /**< I2C slave 4 data in */
#define MPU9250_I2C_MST_STATUS          0x36u    /**< I2C master status */

/* Interrupt Configuration */
#define MPU9250_INT_PIN_CFG             0x37u    /**< Interrupt pin configuration */
#define MPU9250_INT_ENABLE              0x38u    /**< Interrupt enable */
#define MPU9250_INT_STATUS              0x3Au    /**< Interrupt status */

/* Sensor Data Registers */
#define MPU9250_ACCEL_XOUT_H            0x3Bu    /**< Accelerometer X-axis high byte */
#define MPU9250_ACCEL_XOUT_L            0x3Cu    /**< Accelerometer X-axis low byte */
#define MPU9250_ACCEL_YOUT_H            0x3Du    /**< Accelerometer Y-axis high byte */
#define MPU9250_ACCEL_YOUT_L            0x3Eu    /**< Accelerometer Y-axis low byte */
#define MPU9250_ACCEL_ZOUT_H            0x3Fu    /**< Accelerometer Z-axis high byte */
#define MPU9250_ACCEL_ZOUT_L            0x40u    /**< Accelerometer Z-axis low byte */

#define MPU9250_TEMP_OUT_H              0x41u    /**< Temperature high byte */
#define MPU9250_TEMP_OUT_L              0x42u    /**< Temperature low byte */

#define MPU9250_GYRO_XOUT_H             0x43u    /**< Gyroscope X-axis high byte */
#define MPU9250_GYRO_XOUT_L             0x44u    /**< Gyroscope X-axis low byte */
#define MPU9250_GYRO_YOUT_H             0x45u    /**< Gyroscope Y-axis high byte */
#define MPU9250_GYRO_YOUT_L             0x46u    /**< Gyroscope Y-axis low byte */
#define MPU9250_GYRO_ZOUT_H             0x47u    /**< Gyroscope Z-axis high byte */
#define MPU9250_GYRO_ZOUT_L             0x48u    /**< Gyroscope Z-axis low byte */

/* External Sensor Data */
#define MPU9250_EXT_SENS_DATA_00        0x49u    /**< External sensor data 00 */
#define MPU9250_EXT_SENS_DATA_01        0x4Au    /**< External sensor data 01 */
#define MPU9250_EXT_SENS_DATA_02        0x4Bu    /**< External sensor data 02 */
#define MPU9250_EXT_SENS_DATA_03        0x4Cu    /**< External sensor data 03 */
#define MPU9250_EXT_SENS_DATA_04        0x4Du    /**< External sensor data 04 */
#define MPU9250_EXT_SENS_DATA_05        0x4Eu    /**< External sensor data 05 */
#define MPU9250_EXT_SENS_DATA_06        0x4Fu    /**< External sensor data 06 */
#define MPU9250_EXT_SENS_DATA_07        0x50u    /**< External sensor data 07 */
#define MPU9250_EXT_SENS_DATA_08        0x51u    /**< External sensor data 08 */
#define MPU9250_EXT_SENS_DATA_09        0x52u    /**< External sensor data 09 */
#define MPU9250_EXT_SENS_DATA_10        0x53u    /**< External sensor data 10 */
#define MPU9250_EXT_SENS_DATA_11        0x54u    /**< External sensor data 11 */
#define MPU9250_EXT_SENS_DATA_12        0x55u    /**< External sensor data 12 */
#define MPU9250_EXT_SENS_DATA_13        0x56u    /**< External sensor data 13 */
#define MPU9250_EXT_SENS_DATA_14        0x57u    /**< External sensor data 14 */
#define MPU9250_EXT_SENS_DATA_15        0x58u    /**< External sensor data 15 */
#define MPU9250_EXT_SENS_DATA_16        0x59u    /**< External sensor data 16 */
#define MPU9250_EXT_SENS_DATA_17        0x5Au    /**< External sensor data 17 */
#define MPU9250_EXT_SENS_DATA_18        0x5Bu    /**< External sensor data 18 */
#define MPU9250_EXT_SENS_DATA_19        0x5Cu    /**< External sensor data 19 */
#define MPU9250_EXT_SENS_DATA_20        0x5Du    /**< External sensor data 20 */
#define MPU9250_EXT_SENS_DATA_21        0x5Eu    /**< External sensor data 21 */
#define MPU9250_EXT_SENS_DATA_22        0x5Fu    /**< External sensor data 22 */
#define MPU9250_EXT_SENS_DATA_23        0x60u    /**< External sensor data 23 */

/* I2C Slave Data Out */
#define MPU9250_I2C_SLV0_DO             0x63u    /**< I2C slave 0 data out */
#define MPU9250_I2C_SLV1_DO             0x64u    /**< I2C slave 1 data out */
#define MPU9250_I2C_SLV2_DO             0x65u    /**< I2C slave 2 data out */
#define MPU9250_I2C_SLV3_DO             0x66u    /**< I2C slave 3 data out */

/* I2C Master Delay Control */
#define MPU9250_I2C_MST_DELAY_CTRL      0x67u    /**< I2C master delay control */

/* Signal Path Reset */
#define MPU9250_SIGNAL_PATH_RESET       0x68u    /**< Signal path reset */
#define MPU9250_MOT_DETECT_CTRL         0x69u    /**< Motion detection control */
#define MPU9250_USER_CTRL               0x6Au    /**< User control */

/* Power Management */
#define MPU9250_PWR_MGMT_1              0x6Bu    /**< Power management 1 */
#define MPU9250_PWR_MGMT_2              0x6Cu    /**< Power management 2 */

/* FIFO Registers */
#define MPU9250_FIFO_COUNTH             0x72u    /**< FIFO count high byte */
#define MPU9250_FIFO_COUNTL             0x73u    /**< FIFO count low byte */
#define MPU9250_FIFO_R_W                0x74u    /**< FIFO read/write */

/* Device ID */
#define MPU9250_WHO_AM_I                0x75u    /**< Device ID register */

/* Accelerometer Offset Registers */
#define MPU9250_XA_OFFSET_H             0x77u    /**< Accel offset X high byte */
#define MPU9250_XA_OFFSET_L             0x78u    /**< Accel offset X low byte */
#define MPU9250_YA_OFFSET_H             0x7Au    /**< Accel offset Y high byte */
#define MPU9250_YA_OFFSET_L             0x7Bu    /**< Accel offset Y low byte */
#define MPU9250_ZA_OFFSET_H             0x7Du    /**< Accel offset Z high byte */
#define MPU9250_ZA_OFFSET_L             0x7Eu    /**< Accel offset Z low byte */


/* ===================[Device ID Values]=================== */

#define MPU9250_DEVICE_ID               0x70u    /**< Expected WHO_AM_I value for MPU-9250/6500 */

/* ===================[Configuration Values]=================== */

/* Gyroscope Full Scale Range */
#define MPU9250_GYRO_FS_250DPS          0x00u    /**< ±250 °/s */
#define MPU9250_GYRO_FS_500DPS          0x08u    /**< ±500 °/s */
#define MPU9250_GYRO_FS_1000DPS         0x10u    /**< ±1000 °/s */
#define MPU9250_GYRO_FS_2000DPS         0x18u    /**< ±2000 °/s */

/* Accelerometer Full Scale Range */
#define MPU9250_ACCEL_FS_2G             0x00u    /**< ±2g */
#define MPU9250_ACCEL_FS_4G             0x08u    /**< ±4g */
#define MPU9250_ACCEL_FS_8G             0x10u    /**< ±8g */
#define MPU9250_ACCEL_FS_16G            0x18u    /**< ±16g */


/* ===================[Default Configuration]=================== */

#define IMU_DEFAULT_GYRO_RANGE          MPU9250_GYRO_FS_250DPS
#define IMU_DEFAULT_ACCEL_RANGE         MPU9250_ACCEL_FS_2G

#endif /* ECUAL_IMU_IMU_CFG_H_ */
