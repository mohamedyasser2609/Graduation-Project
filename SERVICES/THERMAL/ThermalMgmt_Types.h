/**
 * @file ThermalMgmt_Types.h
 * @brief Type definitions for Thermal Management Service
 * @details AUTOSAR-compliant types for active cooling system management
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#ifndef THERMALMGMT_TYPES_H
#define THERMALMGMT_TYPES_H

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[Macros]=================== */
/** @brief Module identification */
#define THERMALMGMT_MODULE_ID           (0x90u)
#define THERMALMGMT_VENDOR_ID           (0x00u)
#define THERMALMGMT_INSTANCE_ID         (0x00u)

/** @brief Software version */
#define THERMALMGMT_SW_MAJOR_VERSION    (1u)
#define THERMALMGMT_SW_MINOR_VERSION    (0u)
#define THERMALMGMT_SW_PATCH_VERSION    (0u)

/** @brief Development error codes */
#define THERMALMGMT_E_PARAM_POINTER     (0x10u)
#define THERMALMGMT_E_UNINIT            (0x12u)
#define THERMALMGMT_E_ALREADY_INIT      (0x13u)
#define THERMALMGMT_E_PARAM_CONFIG      (0x14u)

/** @brief API service IDs */
#define THERMALMGMT_INIT_SID            (0x00u)
#define THERMALMGMT_MAINFUNCTION_SID    (0x01u)
#define THERMALMGMT_GET_STATUS_SID      (0x02u)
#define THERMALMGMT_GET_VERSION_SID     (0x03u)

/* ===================[Type Definitions]=================== */

/**
 * @brief Thermal management status
 */
typedef enum
{
    THERMALMGMT_STATUS_UNINIT    = 0u,  /**< Not initialized */
    THERMALMGMT_STATUS_NORMAL    = 1u,  /**< Normal operation, cooling active if needed */
    THERMALMGMT_STATUS_WARNING   = 2u,  /**< Elevated temperature, high cooling */
    THERMALMGMT_STATUS_CRITICAL  = 3u,  /**< Critical temperature, max cooling */
    THERMALMGMT_STATUS_SHUTDOWN  = 4u,  /**< Thermal shutdown required */
    THERMALMGMT_STATUS_ERROR     = 5u   /**< Sensor or fan error */
} ThermalMgmt_StatusType;

/**
 * @brief Cooling mode
 */
typedef enum
{
    THERMALMGMT_MODE_OFF         = 0u,  /**< Cooling disabled */
    THERMALMGMT_MODE_PASSIVE     = 1u,  /**< Minimum cooling (quiet) */
    THERMALMGMT_MODE_AUTO        = 2u,  /**< Automatic temperature-based control */
    THERMALMGMT_MODE_AGGRESSIVE  = 3u,  /**< Maximum cooling */
    THERMALMGMT_MODE_MANUAL      = 4u   /**< Manual fan speed control */
} ThermalMgmt_ModeType;

/**
 * @brief Zone identifier
 */
typedef enum
{
    THERMALMGMT_ZONE_MOTORS     = 0u,   /**< Motor driver zone */
    THERMALMGMT_ZONE_MCU        = 1u,   /**< MCU/main board zone */
    THERMALMGMT_ZONE_BATTERY    = 2u,   /**< Battery zone */
    THERMALMGMT_ZONE_COUNT      = 3u
} ThermalMgmt_ZoneType;

/**
 * @brief Zone temperature thresholds
 */
typedef struct
{
    float32     LowThreshold;       /**< Temperature for minimum cooling */
    float32     MediumThreshold;    /**< Temperature for medium cooling */
    float32     HighThreshold;      /**< Temperature for aggressive cooling */
    float32     CriticalThreshold;  /**< Temperature for shutdown */
} ThermalMgmt_ThresholdsType;

/**
 * @brief Zone configuration
 */
typedef struct
{
    ThermalMgmt_ZoneType        Zone;           /**< Zone identifier */
    uint8                       SensorId;       /**< AM2320 sensor ID for this zone */
    ThermalMgmt_ThresholdsType  Thresholds;     /**< Temperature thresholds */
    uint8                       Priority;       /**< Zone priority (0=highest) */
} ThermalMgmt_ZoneConfigType;

/**
 * @brief Fan control configuration
 */
typedef struct
{
    uint8       FanId;              /**< Fan driver ID */
    uint8       MinSpeed;           /**< Minimum speed for passive mode */
    uint8       MedSpeed;           /**< Medium speed */
    uint8       MaxSpeed;           /**< Maximum speed */
} ThermalMgmt_FanConfigType;

/**
 * @brief Thermal status data
 */
typedef struct
{
    float32                 ZoneTemperatures[THERMALMGMT_ZONE_COUNT]; /**< Current temps */
    float32                 MaxTemperature;                           /**< Highest temp */
    ThermalMgmt_ZoneType    HottestZone;                             /**< Zone with max temp */
    uint8                   CurrentFanSpeed;                          /**< Fan speed (0-100) */
    ThermalMgmt_StatusType  OverallStatus;                           /**< System thermal status */
} ThermalMgmt_DataType;

/**
 * @brief Service configuration
 */
typedef struct
{
    uint8                              NumZones;       /**< Number of thermal zones */
    const ThermalMgmt_ZoneConfigType*  Zones;          /**< Zone configurations */
    uint8                              NumFans;        /**< Number of fans */
    const ThermalMgmt_FanConfigType*   Fans;           /**< Fan configurations */
    uint16                             UpdatePeriodMs; /**< Main function period */
} ThermalMgmt_ConfigType;

/**
 * @brief Callback for thermal events
 */
typedef void (*ThermalMgmt_EventCallbackType)(ThermalMgmt_StatusType Status, 
                                               ThermalMgmt_ZoneType Zone,
                                               float32 Temperature);

#endif /* THERMALMGMT_TYPES_H */
