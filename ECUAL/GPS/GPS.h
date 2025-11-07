/**
 * @file GPS.h
 * @brief GPS Driver for NEO-M8M Module
 * @details AUTOSAR-compliant driver for u-blox NEO-M8M GPS with NMEA parsing
 *
 * Features:
 * - NMEA sentence parsing (GGA, RMC, GSA, GSV, VTG, GLL)
 * - UBX protocol support
 * - Multi-GNSS (GPS + GLONASS)
 * - SBAS/WAAS support
 * - Configurable dynamic models
 * - Satellite tracking
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef ECUAL_GPS_GPS_H_
#define ECUAL_GPS_GPS_H_

#include "../../CONFIG/Std_Types.h"
#include "../../MCAL/UART/Uart.h"
#include "GPS_Cfg.h"

/* ===================[Type Definitions]=================== */

/**
 * @brief GPS position data structure
 */
typedef struct {
    float32 latitude;           /**< Latitude in decimal degrees */
    float32 longitude;          /**< Longitude in decimal degrees */
    float32 altitude;           /**< Altitude in meters above sea level */
    float32 geoidHeight;        /**< Geoid height in meters */
} GPS_PositionType;

/**
 * @brief GPS velocity data structure
 */
typedef struct {
    float32 speedKnots;         /**< Speed over ground in knots */
    float32 speedKmh;           /**< Speed over ground in km/h */
    float32 course;             /**< Course over ground in degrees */
} GPS_VelocityType;

/**
 * @brief GPS time data structure
 */
typedef struct {
    uint8 hour;                 /**< UTC hour (0-23) */
    uint8 minute;               /**< UTC minute (0-59) */
    uint8 second;               /**< UTC second (0-59) */
    uint16 millisecond;         /**< Milliseconds (0-999) */
    uint8 day;                  /**< Day of month (1-31) */
    uint8 month;                /**< Month (1-12) */
    uint16 year;                /**< Year (e.g., 2025) */
} GPS_TimeType;

/**
 * @brief GPS satellite information
 */
typedef struct {
    uint8 prn;                  /**< Satellite PRN number */
    uint8 elevation;            /**< Elevation in degrees (0-90) */
    uint16 azimuth;             /**< Azimuth in degrees (0-359) */
    uint8 snr;                  /**< Signal-to-noise ratio in dB */
    boolean inUse;              /**< TRUE if satellite is used in fix */
} GPS_SatelliteType;

/**
 * @brief GPS dilution of precision
 */
typedef struct {
    float32 pdop;               /**< Position DOP */
    float32 hdop;               /**< Horizontal DOP */
    float32 vdop;               /**< Vertical DOP */
} GPS_DopType;

/**
 * @brief Complete GPS data structure
 */
typedef struct {
    GPS_PositionType position;  /**< Position data */
    GPS_VelocityType velocity;  /**< Velocity data */
    GPS_TimeType time;          /**< Time data */
    GPS_DopType dop;            /**< Dilution of precision */
    uint8 fixQuality;           /**< Fix quality (0-8) */
    uint8 satellitesUsed;       /**< Number of satellites used */
    uint8 satellitesInView;     /**< Number of satellites visible */
    GPS_SatelliteType satellites[GPS_MAX_SATELLITES]; /**< Satellite info */
    boolean validFix;           /**< TRUE if fix is valid */
    boolean dataUpdated;        /**< TRUE if new data available */
} GPS_DataType;

/**
 * @brief GPS configuration structure
 */
typedef struct GPS_ConfigType {
    Uart_ModuleType UartModule;     /**< UART module for GPS */
    uint32 BaudRate;                /**< GPS baud rate */
    uint8 DynamicModel;             /**< Dynamic platform model */
    boolean EnableSBAS;             /**< Enable SBAS/WAAS */
    boolean EnableGLONASS;          /**< Enable GLONASS */
    uint8 UpdateRate;               /**< Update rate in Hz (1-10) */
} GPS_ConfigType;

/**
 * @brief GPS status enumeration
 */
typedef enum {
    GPS_STATUS_OK = 0,
    GPS_STATUS_ERROR,
    GPS_STATUS_NOT_INITIALIZED,
    GPS_STATUS_NO_FIX,
    GPS_STATUS_TIMEOUT
} GPS_StatusType;

/* ===================[Function Prototypes]=================== */

/**
 * @brief Initialize GPS driver
 * @param ConfigPtr Pointer to GPS configuration
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_Init(const GPS_ConfigType* ConfigPtr);

/**
 * @brief De-initialize GPS driver
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_DeInit(void);

/**
 * @brief Process incoming GPS data
 * @note Call this periodically to parse NMEA sentences
 * @return E_OK if data processed, E_NOT_OK otherwise
 */
Std_ReturnType GPS_ProcessData(void);

/**
 * @brief Get current GPS data
 * @param Data Pointer to store GPS data
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_GetData(GPS_DataType* Data);

/**
 * @brief Get current position
 * @param Position Pointer to store position
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_GetPosition(GPS_PositionType* Position);

/**
 * @brief Get current velocity
 * @param Velocity Pointer to store velocity
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_GetVelocity(GPS_VelocityType* Velocity);

/**
 * @brief Get current time
 * @param Time Pointer to store time
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_GetTime(GPS_TimeType* Time);

/**
 * @brief Get satellite information
 * @param Satellites Array to store satellite info
 * @param Count Pointer to store number of satellites
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_GetSatellites(GPS_SatelliteType* Satellites, uint8* Count);

/**
 * @brief Check if GPS has valid fix
 * @return TRUE if valid fix, FALSE otherwise
 */
boolean GPS_HasValidFix(void);

/**
 * @brief Get GPS status
 * @return Current GPS status
 */
GPS_StatusType GPS_GetStatus(void);

/**
 * @brief Enable SBAS (WAAS/EGNOS) for improved accuracy
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_EnableSBAS(void);

/**
 * @brief Enable GLONASS constellation
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_EnableGLONASS(void);

/**
 * @brief Set GPS update rate
 * @param RateHz Update rate in Hz (1-10)
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_SetUpdateRate(uint8 RateHz);

/**
 * @brief Set dynamic platform model
 * @param Model Dynamic model (see GPS_DYNAMIC_MODEL_*)
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_SetDynamicModel(uint8 Model);

/**
 * @brief Reset GPS module
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_Reset(void);

#endif /* ECUAL_GPS_GPS_H_ */
