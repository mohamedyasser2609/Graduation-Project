/**
 * @file ACS712.h
 * @brief ACS712 Current Sensor Driver API
 * @details AUTOSAR-compliant driver for ACS712 Hall-effect current sensors
 *
 * Hardware: ACS712 (5A/20A/30A variants)
 * Interface: Analog output to ADC
 * 
 * Features:
 * - Multi-channel support (up to 4 sensors)
 * - Configurable overload detection
 * - Averaging filter for noise reduction
 * - Per-channel status monitoring
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#ifndef ACS712_H
#define ACS712_H

/* ===================[Includes]=================== */
#include "ACS712_Types.h"
#include "ACS712_Cfg.h"

/* ===================[API Declarations]=================== */

/**
 * @brief Initialize the ACS712 driver
 * @param[in] ConfigPtr Pointer to configuration structure
 * @return None
 * @pre ADC driver must be initialized
 * @post Driver is ready for current readings
 */
void ACS712_Init(const ACS712_ConfigType* ConfigPtr);

#if (ACS712_DEINIT_API == STD_ON)
/**
 * @brief De-initialize the ACS712 driver
 * @return None
 */
void ACS712_DeInit(void);
#endif

/**
 * @brief Read current from a specific channel
 * @param[in] Channel Channel identifier
 * @param[out] DataPtr Pointer to store reading data
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType ACS712_ReadCurrent(ACS712_ChannelType Channel, ACS712_DataType* DataPtr);

/**
 * @brief Read all configured channels
 * @param[out] DataArray Array to store readings (must be sized for NumChannels)
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType ACS712_ReadAllChannels(ACS712_DataType* DataArray);

/**
 * @brief Get raw ADC value from channel
 * @param[in] Channel Channel identifier
 * @param[out] RawValue Pointer to store raw ADC value
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType ACS712_GetRawValue(ACS712_ChannelType Channel, uint16* RawValue);

/**
 * @brief Check if channel is in overload condition
 * @param[in] Channel Channel identifier
 * @return TRUE if overload, FALSE otherwise
 */
boolean ACS712_IsOverload(ACS712_ChannelType Channel);

/**
 * @brief Get channel status
 * @param[in] Channel Channel identifier
 * @return Channel status
 */
ACS712_ChannelStatusType ACS712_GetChannelStatus(ACS712_ChannelType Channel);

/**
 * @brief Get driver status
 * @return Driver status
 */
ACS712_StatusType ACS712_GetStatus(void);

/**
 * @brief Calibrate zero-current offset for a channel
 * @param[in] Channel Channel identifier
 * @return E_OK on success, E_NOT_OK on failure
 * @note Call with no current flowing through sensor
 */
Std_ReturnType ACS712_CalibrateZero(ACS712_ChannelType Channel);

#if (ACS712_OVERLOAD_CALLBACK_API == STD_ON)
/**
 * @brief Set overload notification callback
 * @param[in] Callback Function to call on overload detection
 */
void ACS712_SetOverloadCallback(ACS712_OverloadCallbackType Callback);
#endif

#if (ACS712_VERSION_INFO_API == STD_ON)
/**
 * @brief Get driver version information
 * @param[out] versionInfoPtr Pointer to version info structure
 */
void ACS712_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
#endif

#endif /* ACS712_H */
