/**
 * @file ACS712_Types.h
 * @brief Type definitions for ACS712 Current Sensor Driver
 * @details AUTOSAR-compliant types for 30A current sensing
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#ifndef ACS712_TYPES_H
#define ACS712_TYPES_H

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[Macros]=================== */
/** @brief Module identification */
#define ACS712_MODULE_ID            (0x80u)
#define ACS712_VENDOR_ID            (0x00u)
#define ACS712_INSTANCE_ID          (0x00u)

/** @brief Software version */
#define ACS712_SW_MAJOR_VERSION     (1u)
#define ACS712_SW_MINOR_VERSION     (0u)
#define ACS712_SW_PATCH_VERSION     (0u)

/** @brief Development error codes */
#define ACS712_E_PARAM_POINTER      (0x10u)
#define ACS712_E_PARAM_CHANNEL      (0x11u)
#define ACS712_E_UNINIT             (0x12u)
#define ACS712_E_ALREADY_INIT       (0x13u)
#define ACS712_E_PARAM_CONFIG       (0x14u)

/** @brief API service IDs */
#define ACS712_INIT_SID             (0x00u)
#define ACS712_DEINIT_SID           (0x01u)
#define ACS712_READ_CURRENT_SID     (0x02u)
#define ACS712_GET_STATUS_SID       (0x03u)
#define ACS712_GET_VERSION_SID      (0x04u)

/** @brief Maximum number of channels */
#define ACS712_MAX_CHANNELS         (4u)

/* ===================[Type Definitions]=================== */

/**
 * @brief Sensor model variants
 */
typedef enum
{
    ACS712_MODEL_5A  = 0u,      /**< 5A variant, sensitivity 185mV/A */
    ACS712_MODEL_20A = 1u,      /**< 20A variant, sensitivity 100mV/A */
    ACS712_MODEL_30A = 2u       /**< 30A variant, sensitivity 66mV/A */
} ACS712_ModelType;

/**
 * @brief Channel identifier type
 */
typedef uint8 ACS712_ChannelType;

/**
 * @brief Driver status
 */
typedef enum
{
    ACS712_STATUS_UNINIT   = 0u,    /**< Driver not initialized */
    ACS712_STATUS_IDLE     = 1u,    /**< Initialized, idle */
    ACS712_STATUS_BUSY     = 2u,    /**< Conversion in progress */
    ACS712_STATUS_OK       = 3u,    /**< Last reading successful */
    ACS712_STATUS_ERROR    = 4u     /**< Error occurred */
} ACS712_StatusType;

/**
 * @brief Channel status
 */
typedef enum
{
    ACS712_CHANNEL_OK       = 0u,   /**< Normal operation */
    ACS712_CHANNEL_OVERLOAD = 1u,   /**< Current exceeds threshold */
    ACS712_CHANNEL_FAULT    = 2u    /**< Sensor fault detected */
} ACS712_ChannelStatusType;

/**
 * @brief Current reading data structure
 */
typedef struct
{
    float32 CurrentAmps;            /**< Current in Amperes (signed) */
    uint16  RawAdcValue;            /**< Raw ADC reading */
    float32 VoltageV;               /**< Sensor output voltage */
    ACS712_ChannelStatusType Status; /**< Channel status */
} ACS712_DataType;

/**
 * @brief Channel configuration structure
 */
typedef struct
{
    ACS712_ChannelType  ChannelId;       /**< Logical channel ID */
    uint8               AdcChannel;       /**< Physical ADC channel */
    ACS712_ModelType    Model;            /**< Sensor model (5A/20A/30A) */
    float32             OverloadThreshold; /**< Current limit in Amps */
    float32             VrefV;            /**< ADC reference voltage */
    float32             ZeroCurrentVoltage; /**< Output at 0A (typ. 2.5V) */
    uint8               FilterSamples;    /**< Number of samples for averaging */
} ACS712_ChannelConfigType;

/**
 * @brief Driver configuration structure
 */
typedef struct
{
    uint8                           NumChannels;    /**< Number of configured channels */
    const ACS712_ChannelConfigType* Channels;       /**< Array of channel configs */
} ACS712_ConfigType;

/**
 * @brief Callback function type for overload notification
 */
typedef void (*ACS712_OverloadCallbackType)(ACS712_ChannelType Channel, float32 Current);

#endif /* ACS712_TYPES_H */
