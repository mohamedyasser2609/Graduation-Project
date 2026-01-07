/**
 * @file GPS.h
 * @brief Production GPS Driver for NEO-M8N Module
 * @details AUTOSAR-compliant driver for u-blox NEO-M8N GPS with full feature set
 *
 * Features:
 * - NMEA sentence parsing (GGA, RMC, GSA, GSV, VTG, GLL)
 * - UBX protocol support with ACK/NAK handling
 * - Multi-GNSS (GPS + GLONASS + Galileo + BeiDou)
 * - SBAS/WAAS/EGNOS support
 * - AssistNow (Online, Offline, Autonomous)
 * - D-GPS / RTCM support
 * - PPS / Timepulse configuration
 * - Geofencing (up to 4 fences)
 * - Power management modes
 * - Configurable dynamic models
 * - Satellite tracking with constellation info
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 2.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef ECUAL_GPS_GPS_H_
#define ECUAL_GPS_GPS_H_

#include <Std_types.h>
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
    uint16 year;                /**< Year (e.g., 2025) */
    uint16 millisecond;         /**< Milliseconds (0-999) */
    uint8 hour;                 /**< UTC hour (0-23) */
    uint8 minute;               /**< UTC minute (0-59) */
    uint8 second;               /**< UTC second (0-59) */
    uint8 day;                  /**< Day of month (1-31) */
    uint8 month;                /**< Month (1-12) */
} GPS_TimeType;

/**
 * @brief GPS satellite information
 */
typedef struct {
    uint16 azimuth;             /**< Azimuth in degrees (0-359) */
    uint8 prn;                  /**< Satellite PRN number */
    uint8 gnssId;               /**< GNSS constellation ID */
    uint8 elevation;            /**< Elevation in degrees (0-90) */
    uint8 snr;                  /**< Signal-to-noise ratio in dB */
    boolean inUse;              /**< TRUE if satellite is used in fix */
    boolean healthy;            /**< TRUE if satellite is healthy */
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
 * @brief GNSS constellation usage
 */
typedef struct {
    uint8 gpsUsed;              /**< GPS satellites used */
    uint8 glonassUsed;          /**< GLONASS satellites used */
    uint8 galileoUsed;          /**< Galileo satellites used */
    uint8 beidouUsed;           /**< BeiDou satellites used */
    boolean sbasUsed;           /**< TRUE if SBAS correction active */
} GPS_ConstellationInfoType;

/**
 * @brief Complete GPS data structure
 */
typedef struct {
    GPS_PositionType position;  /**< Position data */
    GPS_VelocityType velocity;  /**< Velocity data */
    GPS_TimeType time;          /**< Time data */
    GPS_DopType dop;            /**< Dilution of precision */
    GPS_ConstellationInfoType constellations; /**< Constellation usage */
    uint8 fixQuality;           /**< Fix quality (0-8) */
    uint8 fixType;              /**< Fix type (2D/3D) */
    uint8 satellitesUsed;       /**< Number of satellites used */
    uint8 satellitesInView;     /**< Number of satellites visible */
    GPS_SatelliteType satellites[GPS_MAX_SATELLITES]; /**< Satellite info */
    boolean validFix;           /**< TRUE if fix is valid */
    boolean dataUpdated;        /**< TRUE if new data available */
    const char* lastNmeaType;   /**< Last NMEA sentence type processed */
} GPS_DataType;

/**
 * @brief Geofence configuration
 */
typedef struct {
    float32 latitude;           /**< Center latitude */
    float32 longitude;          /**< Center longitude */
    uint32 radius;              /**< Radius in meters */
    boolean enabled;            /**< Fence enabled */
} GPS_GeofenceType;

/**
 * @brief GPS configuration structure
 */
typedef struct GPS_ConfigType {
    uint32 BaudRate;                /**< GPS baud rate */
    Uart_ModuleType UartModule;     /**< UART module for GPS */
    uint8 DynamicModel;             /**< Dynamic platform model */
    uint8 PowerMode;                /**< Power management mode */
    uint8 SbasSystem;               /**< SBAS system selection */
    uint8 UpdateRate;               /**< Update rate in Hz (1-10) */
    uint8 AssistNowMode;            /**< AssistNow mode */
    boolean EnableSBAS;             /**< Enable SBAS/WAAS */
    boolean EnableGLONASS;          /**< Enable GLONASS */
    boolean EnableGalileo;          /**< Enable Galileo */
    boolean EnableBeiDou;           /**< Enable BeiDou */
    boolean EnablePPS;              /**< Enable PPS output */
    boolean EnableGeofencing;       /**< Enable geofencing */
    boolean EnableRTCM;             /**< Enable RTCM input */
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

/* ===================[Advanced Configuration Functions]=================== */

/**
 * @brief Configure multi-GNSS constellations
 * @param enableGPS Enable GPS
 * @param enableGLONASS Enable GLONASS
 * @param enableGalileo Enable Galileo
 * @param enableBeiDou Enable BeiDou
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_ConfigureGNSS(boolean enableGPS, boolean enableGLONASS, 
                                  boolean enableGalileo, boolean enableBeiDou);

/**
 * @brief Enable Galileo constellation
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_EnableGalileo(void);

/**
 * @brief Enable BeiDou constellation
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_EnableBeiDou(void);

/**
 * @brief Configure SBAS system
 * @param sbasSystem SBAS system to use (GPS_SBAS_*)
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_ConfigureSBAS(uint8 sbasSystem);

/**
 * @brief Configure PPS/Timepulse output
 * @param enable Enable PPS output
 * @param freqHz Frequency in Hz (default 1Hz)
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_ConfigurePPS(boolean enable, uint32 freqHz);

/**
 * @brief Set power management mode
 * @param mode Power mode (GPS_POWER_MODE_*)
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_SetPowerMode(uint8 mode);

/**
 * @brief Configure geofence
 * @param fenceId Fence ID (0-3)
 * @param fence Pointer to geofence configuration
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_ConfigureGeofence(uint8 fenceId, const GPS_GeofenceType* fence);

/**
 * @brief Enable RTCM input for D-GPS
 * @param enable Enable RTCM
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_EnableRTCM(boolean enable);

/**
 * @brief Send AssistNow aiding data
 * @param mode AssistNow mode (GPS_ASSISTNOW_*)
 * @param data Pointer to aiding data
 * @param length Data length in bytes
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_SendAssistNowData(uint8 mode, const uint8* data, uint16 length);

/**
 * @brief Save current configuration to flash
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_SaveConfiguration(void);

/**
 * @brief Load configuration from flash
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_LoadConfiguration(void);

/**
 * @brief Get constellation usage information
 * @param info Pointer to store constellation info
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType GPS_GetConstellationInfo(GPS_ConstellationInfoType* info);

/**
 * @brief Wait for UBX ACK/NAK response
 * @param msgClass UBX message class
 * @param msgId UBX message ID
 * @param timeoutMs Timeout in milliseconds
 * @return E_OK if ACK received, E_NOT_OK if NAK or timeout
 */
Std_ReturnType GPS_WaitForAck(uint8 msgClass, uint8 msgId, uint32 timeoutMs);

/**
 * @brief Get last NMEA sentence type processed
 * @return Pointer to NMEA type string (e.g., "GGA", "RMC")
 */
const char* GPS_GetLastNmeaType(void);

#endif /* ECUAL_GPS_GPS_H_ */
