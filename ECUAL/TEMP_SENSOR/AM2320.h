/**
 * @file AM2320.h
 * @brief AM2320 Temperature & Humidity Sensor Driver API
 * @details AUTOSAR-compliant driver for ASAIR AM2320 calibrated sensor
 *
 * Hardware: AM2320 Digital Temperature & Humidity Sensor
 * Interface: I2C (address 0x5C)
 * 
 * Features:
 * - Multi-sensor support (up to 4 sensors via I2C mux or separate buses)
 * - Temperature range: -40°C to +80°C (±0.5°C accuracy)
 * - Humidity range: 0-99.9% RH (±3% accuracy)
 * - CRC-16 validation
 * - Temperature alarm thresholds
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#ifndef AM2320_H
#define AM2320_H

/* ===================[Includes]=================== */
#include "AM2320_Types.h"
#include "AM2320_Cfg.h"

/* ===================[API Declarations]=================== */

/**
 * @brief Initialize the AM2320 driver
 * @param[in] ConfigPtr Pointer to configuration structure
 * @return None
 * @pre I2C driver must be initialized
 * @post Driver is ready for sensor readings
 */
void AM2320_Init(const AM2320_ConfigType* ConfigPtr);

#if (AM2320_DEINIT_API == STD_ON)
/**
 * @brief De-initialize the AM2320 driver
 * @return None
 */
void AM2320_DeInit(void);
#endif

/**
 * @brief Read temperature and humidity from sensor
 * @param[in] Sensor Sensor identifier
 * @param[out] DataPtr Pointer to store sensor data
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType AM2320_Read(AM2320_SensorType Sensor, AM2320_DataType* DataPtr);

/**
 * @brief Read all configured sensors
 * @param[out] DataArray Array to store readings (must be sized for NumSensors)
 * @return E_OK on success, E_NOT_OK if any sensor failed
 */
Std_ReturnType AM2320_ReadAllSensors(AM2320_DataType* DataArray);

/**
 * @brief Read temperature only
 * @param[in] Sensor Sensor identifier
 * @param[out] Temperature Pointer to store temperature (Celsius)
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType AM2320_ReadTemperature(AM2320_SensorType Sensor, float32* Temperature);

/**
 * @brief Read humidity only
 * @param[in] Sensor Sensor identifier
 * @param[out] Humidity Pointer to store humidity (% RH)
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType AM2320_ReadHumidity(AM2320_SensorType Sensor, float32* Humidity);

/**
 * @brief Check if sensor is connected
 * @param[in] Sensor Sensor identifier
 * @return TRUE if responding, FALSE otherwise
 */
boolean AM2320_IsSensorPresent(AM2320_SensorType Sensor);

/**
 * @brief Get sensor status
 * @param[in] Sensor Sensor identifier
 * @return Sensor status
 */
AM2320_SensorStatusType AM2320_GetSensorStatus(AM2320_SensorType Sensor);

/**
 * @brief Get driver status
 * @return Driver status
 */
AM2320_StatusType AM2320_GetStatus(void);

/**
 * @brief Get average temperature from all sensors
 * @param[out] AvgTemperature Pointer to store average
 * @return E_OK on success, E_NOT_OK if no valid readings
 */
Std_ReturnType AM2320_GetAverageTemperature(float32* AvgTemperature);

/**
 * @brief Get maximum temperature from all sensors
 * @param[out] MaxTemperature Pointer to store maximum
 * @param[out] SensorId Pointer to store sensor with max temp (optional, can be NULL)
 * @return E_OK on success, E_NOT_OK if no valid readings
 */
Std_ReturnType AM2320_GetMaxTemperature(float32* MaxTemperature, AM2320_SensorType* SensorId);

#if (AM2320_ALARM_CALLBACK_API == STD_ON)
/**
 * @brief Set temperature alarm callback
 * @param[in] Callback Function to call on temperature alarm
 */
void AM2320_SetAlarmCallback(AM2320_AlarmCallbackType Callback);
#endif

#if (AM2320_VERSION_INFO_API == STD_ON)
/**
 * @brief Get driver version information
 * @param[out] versionInfoPtr Pointer to version info structure
 */
void AM2320_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
#endif

#endif /* AM2320_H */
