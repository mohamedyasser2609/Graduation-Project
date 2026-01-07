/**
 * @file AM2320_Types.h
 * @brief Type definitions for AM2320 Temperature & Humidity Sensor Driver
 * @details AUTOSAR-compliant types for ASAIR AM2320 sensor
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#ifndef AM2320_TYPES_H
#define AM2320_TYPES_H

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[Macros]=================== */
/** @brief Module identification */
#define AM2320_MODULE_ID            (0x81u)
#define AM2320_VENDOR_ID            (0x00u)
#define AM2320_INSTANCE_ID          (0x00u)

/** @brief Software version */
#define AM2320_SW_MAJOR_VERSION     (1u)
#define AM2320_SW_MINOR_VERSION     (0u)
#define AM2320_SW_PATCH_VERSION     (0u)

/** @brief Development error codes */
#define AM2320_E_PARAM_POINTER      (0x10u)
#define AM2320_E_PARAM_SENSOR       (0x11u)
#define AM2320_E_UNINIT             (0x12u)
#define AM2320_E_ALREADY_INIT       (0x13u)
#define AM2320_E_PARAM_CONFIG       (0x14u)
#define AM2320_E_COMM_ERROR         (0x15u)
#define AM2320_E_CRC_ERROR          (0x16u)

/** @brief API service IDs */
#define AM2320_INIT_SID             (0x00u)
#define AM2320_DEINIT_SID           (0x01u)
#define AM2320_READ_SID             (0x02u)
#define AM2320_GET_STATUS_SID       (0x03u)
#define AM2320_GET_VERSION_SID      (0x04u)

/** @brief Maximum number of sensors */
#define AM2320_MAX_SENSORS          (4u)

/** @brief I2C Address */
#define AM2320_I2C_ADDRESS          (0x5Cu)

/* ===================[Type Definitions]=================== */

/**
 * @brief Sensor identifier type
 */
typedef uint8 AM2320_SensorType;

/**
 * @brief Driver status
 */
typedef enum
{
    AM2320_STATUS_UNINIT   = 0u,    /**< Driver not initialized */
    AM2320_STATUS_IDLE     = 1u,    /**< Initialized, idle */
    AM2320_STATUS_BUSY     = 2u,    /**< Reading in progress */
    AM2320_STATUS_OK       = 3u,    /**< Last reading successful */
    AM2320_STATUS_ERROR    = 4u     /**< Error occurred */
} AM2320_StatusType;

/**
 * @brief Sensor health status
 */
typedef enum
{
    AM2320_SENSOR_OK           = 0u,   /**< Normal operation */
    AM2320_SENSOR_COMM_ERROR   = 1u,   /**< Communication failure */
    AM2320_SENSOR_CRC_ERROR    = 2u,   /**< CRC check failed */
    AM2320_SENSOR_TIMEOUT      = 3u,   /**< Response timeout */
    AM2320_SENSOR_DISCONNECTED = 4u    /**< Sensor not responding */
} AM2320_SensorStatusType;

/**
 * @brief Temperature and humidity data structure
 */
typedef struct
{
    float32                 TemperatureC;   /**< Temperature in Celsius */
    float32                 Humidity;       /**< Relative humidity in % */
    uint16                  RawTemperature; /**< Raw temperature value */
    uint16                  RawHumidity;    /**< Raw humidity value */
    AM2320_SensorStatusType Status;         /**< Sensor status */
} AM2320_DataType;

/**
 * @brief Sensor configuration structure
 */
typedef struct
{
    AM2320_SensorType   SensorId;           /**< Logical sensor ID */
    uint8               I2cModule;          /**< I2C module to use */
    float32             TempOffsetC;        /**< Temperature offset correction */
    float32             HumidityOffset;     /**< Humidity offset correction */
    float32             TempHighThreshold;  /**< High temp alarm threshold */
    float32             TempLowThreshold;   /**< Low temp alarm threshold */
} AM2320_SensorConfigType;

/**
 * @brief Driver configuration structure
 */
typedef struct
{
    uint8                          NumSensors;    /**< Number of configured sensors */
    const AM2320_SensorConfigType* Sensors;       /**< Array of sensor configs */
} AM2320_ConfigType;

/**
 * @brief Callback function type for temperature alarm
 */
typedef void (*AM2320_AlarmCallbackType)(AM2320_SensorType Sensor, float32 Temperature, boolean IsHigh);

#endif /* AM2320_TYPES_H */
