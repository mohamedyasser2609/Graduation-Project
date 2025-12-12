/**
 * @file GPS_Cfg.h
 * @brief GPS Driver Configuration for NEO-M8N
 * @details Production-ready configuration header for AUTOSAR-compliant GPS driver
 *          Supports multi-GNSS, SBAS, AssistNow, D-GPS, PPS, Geofencing
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 2.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef ECUAL_GPS_GPS_CFG_H_
#define ECUAL_GPS_GPS_CFG_H_

#include "../../CONFIG/Std_Types.h"

/* ===================[GPS Module Configuration]=================== */

/**
 * @brief GPS module default baud rate
 * @note NEO-M8N default is 9600, can be configured up to 115200
 */
#define GPS_DEFAULT_BAUD_RATE           9600u

/**
 * @brief UBX message buffer size
 */
#define GPS_UBX_BUFFER_SIZE             256u

/**
 * @brief NMEA sentence buffer size
 */
#define GPS_NMEA_BUFFER_SIZE            128u

/**
 * @brief Maximum number of fields in NMEA sentence
 */
#define GPS_MAX_NMEA_FIELDS             20u

/**
 * @brief GPS update rate (Hz)
 * @note NEO-M8N supports 1Hz to 10Hz (18Hz for some versions)
 */
#define GPS_DEFAULT_UPDATE_RATE         1u      /* 1Hz */
#define GPS_MAX_UPDATE_RATE             10u     /* 10Hz max */

/**
 * @brief Maximum geofences supported
 */
#define GPS_MAX_GEOFENCES               4u

/**
 * @brief Maximum number of satellites to track
 */
#define GPS_MAX_SATELLITES              32u

/**
 * @brief GPS timeout in milliseconds
 */
#define GPS_TIMEOUT_MS                  5000u

/* ===================[NMEA Sentence Types]=================== */

/**
 * @brief NMEA sentence identifiers
 */
#define GPS_NMEA_GGA                    "GGA"   /**< Global Positioning System Fix Data */
#define GPS_NMEA_RMC                    "RMC"   /**< Recommended Minimum Navigation Information */
#define GPS_NMEA_GSA                    "GSA"   /**< GPS DOP and Active Satellites */
#define GPS_NMEA_GSV                    "GSV"   /**< GPS Satellites in View */
#define GPS_NMEA_VTG                    "VTG"   /**< Track Made Good and Ground Speed */
#define GPS_NMEA_GLL                    "GLL"   /**< Geographic Position */

/* ===================[UBX Protocol Configuration]=================== */

/**
 * @brief UBX protocol header bytes
 */
#define GPS_UBX_SYNC_CHAR_1             0xB5u
#define GPS_UBX_SYNC_CHAR_2             0x62u

/**
 * @brief UBX message classes
 */
#define GPS_UBX_CLASS_NAV               0x01u   /**< Navigation Results */
#define GPS_UBX_CLASS_RXM               0x02u   /**< Receiver Manager */
#define GPS_UBX_CLASS_INF               0x04u   /**< Information Messages */
#define GPS_UBX_CLASS_ACK               0x05u   /**< Ack/Nack Messages */
#define GPS_UBX_CLASS_CFG               0x06u   /**< Configuration Input */
#define GPS_UBX_CLASS_MON               0x0Au   /**< Monitoring Messages */
#define GPS_UBX_CLASS_AID               0x0Bu   /**< AssistNow Aiding */
#define GPS_UBX_CLASS_TIM               0x0Du   /**< Timing Messages */

/**
 * @brief UBX-CFG message IDs
 */
#define GPS_UBX_CFG_PRT                 0x00u   /**< Port Configuration */
#define GPS_UBX_CFG_MSG                 0x01u   /**< Message Configuration */
#define GPS_UBX_CFG_RATE                0x08u   /**< Navigation/Measurement Rate */
#define GPS_UBX_CFG_CFG                 0x09u   /**< Clear, Save, Load Config */
#define GPS_UBX_CFG_NAV5                0x24u   /**< Navigation Engine Settings */
#define GPS_UBX_CFG_SBAS                0x16u   /**< SBAS Configuration */
#define GPS_UBX_CFG_GNSS                0x3Eu   /**< GNSS System Configuration */
#define GPS_UBX_CFG_TP5                 0x31u   /**< Timepulse Parameters */
#define GPS_UBX_CFG_PM2                 0x3Bu   /**< Extended Power Management */
#define GPS_UBX_CFG_GEOFENCE            0x69u   /**< Geofencing Configuration */
#define GPS_UBX_CFG_RTCM                0x5Eu   /**< RTCM Configuration */

/**
 * @brief UBX-ACK message IDs
 */
#define GPS_UBX_ACK_ACK                 0x01u   /**< Message Acknowledged */
#define GPS_UBX_ACK_NAK                 0x00u   /**< Message Not Acknowledged */

/**
 * @brief UBX-AID message IDs (AssistNow)
 */
#define GPS_UBX_AID_INI                 0x01u   /**< Aiding position, time */
#define GPS_UBX_AID_EPH                 0x31u   /**< GPS Ephemeris Data */
#define GPS_UBX_AID_ALM                 0x30u   /**< GPS Almanac Data */
#define GPS_UBX_AID_AOP                 0x33u   /**< AssistNow Autonomous */

/**
 * @brief UBX-NAV message IDs
 */
#define GPS_UBX_NAV_PVT                 0x07u   /**< Navigation Position Velocity Time */
#define GPS_UBX_NAV_STATUS              0x03u   /**< Receiver Navigation Status */
#define GPS_UBX_NAV_SAT                 0x35u   /**< Satellite Information */
#define GPS_UBX_NAV_TIMEGPS             0x20u   /**< GPS Time Solution */

/* ===================[GPS Fix Quality]=================== */

/**
 * @brief GPS fix quality indicators
 */
#define GPS_FIX_INVALID                 0u      /**< No fix */
#define GPS_FIX_GPS                     1u      /**< GPS fix */
#define GPS_FIX_DGPS                    2u      /**< Differential GPS fix */
#define GPS_FIX_PPS                     3u      /**< PPS fix */
#define GPS_FIX_RTK                     4u      /**< Real Time Kinematic */
#define GPS_FIX_RTK_FLOAT               5u      /**< Float RTK */
#define GPS_FIX_ESTIMATED               6u      /**< Estimated (dead reckoning) */
#define GPS_FIX_MANUAL                  7u      /**< Manual input mode */
#define GPS_FIX_SIMULATION              8u      /**< Simulation mode */

/* ===================[Navigation Modes]=================== */

/**
 * @brief Dynamic platform models for NEO-M8N
 */
#define GPS_DYNAMIC_MODEL_PORTABLE      0u      /**< Portable (default) */
#define GPS_DYNAMIC_MODEL_STATIONARY    2u      /**< Stationary */
#define GPS_DYNAMIC_MODEL_PEDESTRIAN    3u      /**< Pedestrian */
#define GPS_DYNAMIC_MODEL_AUTOMOTIVE    4u      /**< Automotive */
#define GPS_DYNAMIC_MODEL_SEA           5u      /**< Sea */
#define GPS_DYNAMIC_MODEL_AIRBORNE_1G   6u      /**< Airborne < 1g */
#define GPS_DYNAMIC_MODEL_AIRBORNE_2G   7u      /**< Airborne < 2g */
#define GPS_DYNAMIC_MODEL_AIRBORNE_4G   8u      /**< Airborne < 4g */
#define GPS_DYNAMIC_MODEL_WRIST         9u      /**< Wrist-worn watch */
#define GPS_DYNAMIC_MODEL_BIKE          10u     /**< Bike/Motorbike */

/* ===================[GNSS Constellations]=================== */

/**
 * @brief GNSS system identifiers
 */
#define GPS_GNSS_GPS                    0x00u   /**< GPS */
#define GPS_GNSS_SBAS                   0x01u   /**< SBAS */
#define GPS_GNSS_GALILEO                0x02u   /**< Galileo */
#define GPS_GNSS_BEIDOU                 0x03u   /**< BeiDou */
#define GPS_GNSS_QZSS                   0x05u   /**< QZSS */
#define GPS_GNSS_GLONASS                0x06u   /**< GLONASS */

/* ===================[Default Configuration]=================== */

/* ===================[Power Management Modes]=================== */

/**
 * @brief Power management modes
 */
#define GPS_POWER_MODE_FULL             0u      /**< Full power, continuous tracking */
#define GPS_POWER_MODE_BALANCED         1u      /**< Balanced power/performance */
#define GPS_POWER_MODE_INTERVAL         2u      /**< Interval tracking (cyclic) */
#define GPS_POWER_MODE_AGGRESSIVE       3u      /**< Aggressive power saving */

/* ===================[SBAS Systems]=================== */

/**
 * @brief SBAS system identifiers
 */
#define GPS_SBAS_AUTO                   0x00u   /**< Auto-select */
#define GPS_SBAS_WAAS                   0x01u   /**< WAAS (Americas) */
#define GPS_SBAS_EGNOS                  0x02u   /**< EGNOS (Europe) */
#define GPS_SBAS_MSAS                   0x04u   /**< MSAS (Japan) */
#define GPS_SBAS_GAGAN                  0x08u   /**< GAGAN (India) */

/* ===================[AssistNow Modes]=================== */

/**
 * @brief AssistNow aiding modes
 */
#define GPS_ASSISTNOW_OFFLINE           0u      /**< Offline (pre-downloaded) */
#define GPS_ASSISTNOW_ONLINE            1u      /**< Online (real-time) */
#define GPS_ASSISTNOW_AUTONOMOUS        2u      /**< Autonomous (self-generated) */

/* ===================[RTCM Message Types]=================== */

/**
 * @brief RTCM 2.3 message types for D-GPS
 */
#define GPS_RTCM_TYPE_1                 1u      /**< Differential GPS Corrections */
#define GPS_RTCM_TYPE_3                 3u      /**< Reference Station Parameters */
#define GPS_RTCM_TYPE_9                 9u      /**< GPS Partial Correction Set */

/* ===================[Default Configuration]=================== */

/**
 * @brief Default GPS configuration values
 */
#define GPS_DEFAULT_DYNAMIC_MODEL       GPS_DYNAMIC_MODEL_AUTOMOTIVE
#define GPS_DEFAULT_ENABLE_SBAS         STD_ON
#define GPS_DEFAULT_ENABLE_GLONASS      STD_ON
#define GPS_DEFAULT_ENABLE_GALILEO      STD_ON
#define GPS_DEFAULT_ENABLE_BEIDOU       STD_OFF
#define GPS_DEFAULT_MIN_SATELLITES      4u
#define GPS_DEFAULT_POWER_MODE          GPS_POWER_MODE_FULL
#define GPS_DEFAULT_SBAS_SYSTEM         GPS_SBAS_AUTO
#define GPS_DEFAULT_PPS_ENABLED         STD_OFF
#define GPS_DEFAULT_GEOFENCE_ENABLED    STD_OFF

/* ===================[External Configuration Declarations]=================== */

/**
 * @brief External declaration of GPS configuration structures
 * @note These are defined in GPS_PBCfg.c
 */
extern const struct GPS_ConfigType GPS_Config_Default;
extern const struct GPS_ConfigType GPS_Config_Automotive;
extern const struct GPS_ConfigType GPS_Config_Drone;
extern const struct GPS_ConfigType GPS_Config_Marine;
extern const struct GPS_ConfigType* GPS_ConfigPtr;

#endif /* ECUAL_GPS_GPS_CFG_H_ */
