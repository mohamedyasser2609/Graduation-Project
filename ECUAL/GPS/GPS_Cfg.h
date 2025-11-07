/**
 * @file GPS_Cfg.h
 * @brief GPS Driver Configuration for NEO-M8M
 * @details Configuration header for AUTOSAR-compliant GPS driver
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef ECUAL_GPS_GPS_CFG_H_
#define ECUAL_GPS_GPS_CFG_H_

#include "../../CONFIG/Std_Types.h"

/* ===================[GPS Module Configuration]=================== */

/**
 * @brief GPS module default baud rate
 * @note NEO-M8M default is 9600, can be configured up to 115200
 */
#define GPS_DEFAULT_BAUD_RATE           9600u

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
 * @note NEO-M8M supports 1Hz to 10Hz
 */
#define GPS_DEFAULT_UPDATE_RATE         1u      /* 1Hz */

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
#define GPS_UBX_CFG_NAV5                0x24u   /**< Navigation Engine Settings */
#define GPS_UBX_CFG_GNSS                0x3Eu   /**< GNSS System Configuration */
#define GPS_UBX_CFG_SBAS                0x16u   /**< SBAS Configuration */

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
 * @brief Dynamic platform models for NEO-M8M
 */
#define GPS_DYNAMIC_MODEL_PORTABLE      0u      /**< Portable (default) */
#define GPS_DYNAMIC_MODEL_STATIONARY    2u      /**< Stationary */
#define GPS_DYNAMIC_MODEL_PEDESTRIAN    3u      /**< Pedestrian */
#define GPS_DYNAMIC_MODEL_AUTOMOTIVE    4u      /**< Automotive */
#define GPS_DYNAMIC_MODEL_SEA           5u      /**< Sea */
#define GPS_DYNAMIC_MODEL_AIRBORNE_1G   6u      /**< Airborne < 1g */
#define GPS_DYNAMIC_MODEL_AIRBORNE_2G   7u      /**< Airborne < 2g */
#define GPS_DYNAMIC_MODEL_AIRBORNE_4G   8u      /**< Airborne < 4g */

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

/**
 * @brief Default GPS configuration values
 */
#define GPS_DEFAULT_DYNAMIC_MODEL       GPS_DYNAMIC_MODEL_AUTOMOTIVE
#define GPS_DEFAULT_ENABLE_SBAS         STD_ON
#define GPS_DEFAULT_ENABLE_GLONASS      STD_ON
#define GPS_DEFAULT_MIN_SATELLITES      4u

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
