/**
 * @file IMU.h
 * @brief IMU Driver for MPU-9250 9-Axis Motion Sensor
 * @details AUTOSAR-compliant driver for MPU-9250 (Accelerometer + Gyroscope + Magnetometer)
 *
 * Features:
 * - 3-axis accelerometer (±2g, ±4g, ±8g, ±16g)
 * - 3-axis gyroscope (±250, ±500, ±1000, ±2000 °/s)
 * - 3-axis magnetometer (AK8963)
 * - Temperature sensor
 * - Calibration support
 * - FIFO buffer
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef ECUAL_IMU_IMU_H_
#define ECUAL_IMU_IMU_H_

#include "../../CONFIG/Std_Types.h"
#include "../../MCAL/I2C/I2C.h"
#include "IMU_Cfg.h"

/* ===================[Type Definitions]=================== */

/**
 * @brief IMU sensor data structure
 */
typedef struct {
    sint16 x;    /**< X-axis data */
    sint16 y;    /**< Y-axis data */
    sint16 z;    /**< Z-axis data */
} IMU_AxisDataType;

/**
 * @brief IMU complete sensor data
 */
typedef struct {
    IMU_AxisDataType accel;    /**< Accelerometer data (raw) */
    IMU_AxisDataType gyro;     /**< Gyroscope data (raw) */
    IMU_AxisDataType mag;      /**< Magnetometer data (raw) */
    sint16 temperature;        /**< Temperature (raw) */
} IMU_SensorDataType;

/**
 * @brief IMU calibrated sensor data (floating point)
 */
typedef struct {
    float32 x;    /**< X-axis data */
    float32 y;    /**< Y-axis data */
    float32 z;    /**< Z-axis data */
} IMU_AxisDataFloatType;

/**
 * @brief IMU calibrated complete data
 */
typedef struct {
    IMU_AxisDataFloatType accel;    /**< Accelerometer (g) */
    IMU_AxisDataFloatType gyro;     /**< Gyroscope (°/s) */
    IMU_AxisDataFloatType mag;      /**< Magnetometer (µT) */
    float32 temperature;            /**< Temperature (°C) */
} IMU_CalibratedDataType;

/**
 * @brief IMU configuration structure
 */
typedef struct {
    I2C_ModuleType I2C_Module;          /**< I2C module to use */
    uint8 DeviceAddress;                /**< MPU-9250 I2C address */
    uint8 GyroRange;                    /**< Gyroscope range */
    uint8 AccelRange;                   /**< Accelerometer range */
    uint8 MagMode;                      /**< Magnetometer mode */
    uint8 MagResolution;                /**< Magnetometer resolution */
} IMU_ConfigType;

/**
 * @brief IMU status enumeration
 */
typedef enum {
    IMU_STATUS_OK = 0,
    IMU_STATUS_ERROR,
    IMU_STATUS_NOT_INITIALIZED,
    IMU_STATUS_DEVICE_NOT_FOUND,
    IMU_STATUS_CALIBRATION_ERROR
} IMU_StatusType;

/* ===================[Function Prototypes]=================== */

/**
 * @brief Initialize IMU driver
 * @param ConfigPtr Pointer to IMU configuration
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType IMU_Init(const IMU_ConfigType* ConfigPtr);

/**
 * @brief De-initialize IMU driver
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType IMU_DeInit(void);

/**
 * @brief Read raw sensor data from IMU
 * @param Data Pointer to store sensor data
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType IMU_ReadRawData(IMU_SensorDataType* Data);

/**
 * @brief Read calibrated sensor data from IMU
 * @param Data Pointer to store calibrated data
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType IMU_ReadCalibratedData(IMU_CalibratedDataType* Data);

/**
 * @brief Read accelerometer data only
 * @param Data Pointer to store accelerometer data
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType IMU_ReadAccel(IMU_AxisDataType* Data);

/**
 * @brief Read gyroscope data only
 * @param Data Pointer to store gyroscope data
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType IMU_ReadGyro(IMU_AxisDataType* Data);

/**
 * @brief Read magnetometer data only
 * @param Data Pointer to store magnetometer data
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType IMU_ReadMag(IMU_AxisDataType* Data);

/**
 * @brief Read temperature
 * @param Temperature Pointer to store temperature (°C)
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType IMU_ReadTemperature(float32* Temperature);

/**
 * @brief Calibrate IMU sensors
 * @param Samples Number of samples for calibration
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType IMU_Calibrate(uint16 Samples);

/**
 * @brief Perform self-test
 * @return E_OK if passed, E_NOT_OK if failed
 */
Std_ReturnType IMU_SelfTest(void);

/**
 * @brief Get IMU status
 * @return Current IMU status
 */
IMU_StatusType IMU_GetStatus(void);

/**
 * @brief Reset IMU device
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType IMU_Reset(void);

/**
 * @brief Check if IMU device is present
 * @return TRUE if device found, FALSE otherwise
 */
boolean IMU_IsDevicePresent(void);

#endif /* ECUAL_IMU_IMU_H_ */
