/**
 * @file ThermalMgmt.h
 * @brief Thermal Management Service API
 * @details Coordinates temperature sensors and fans for active cooling
 *
 * This service:
 * - Reads temperature from multiple AM2320 sensors
 * - Controls cooling fans based on temperature
 * - Provides thermal status for system-wide decisions
 * - Supports thermal shutdown protection
 *
 * Integration:
 * - Uses AM2320 driver for temperature sensing
 * - Uses Fan driver for PWM fan control
 * - Called periodically by FreeRTOS task or main loop
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#ifndef THERMALMGMT_H
#define THERMALMGMT_H

/* ===================[Includes]=================== */
#include "ThermalMgmt_Types.h"
#include "ThermalMgmt_Cfg.h"

/* ===================[API Declarations]=================== */

/**
 * @brief Initialize the Thermal Management service
 * @param[in] ConfigPtr Pointer to configuration structure
 * @return None
 * @pre AM2320 and Fan drivers must be initialized
 */
void ThermalMgmt_Init(const ThermalMgmt_ConfigType* ConfigPtr);

/**
 * @brief Main function - call periodically
 * @return None
 * @note Should be called at configured interval (e.g., every 1 second)
 */
void ThermalMgmt_MainFunction(void);

/**
 * @brief Set operating mode
 * @param[in] Mode Desired operating mode
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType ThermalMgmt_SetMode(ThermalMgmt_ModeType Mode);

/**
 * @brief Get current operating mode
 * @return Current mode
 */
ThermalMgmt_ModeType ThermalMgmt_GetMode(void);

/**
 * @brief Get current thermal status
 * @return Overall thermal status
 */
ThermalMgmt_StatusType ThermalMgmt_GetStatus(void);

/**
 * @brief Get detailed thermal data
 * @param[out] DataPtr Pointer to store thermal data
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType ThermalMgmt_GetData(ThermalMgmt_DataType* DataPtr);

/**
 * @brief Get temperature of specific zone
 * @param[in] Zone Zone identifier
 * @param[out] Temperature Pointer to store temperature
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType ThermalMgmt_GetZoneTemperature(ThermalMgmt_ZoneType Zone, float32* Temperature);

/**
 * @brief Manually set fan speed (only in MANUAL mode)
 * @param[in] SpeedPercent Fan speed (0-100%)
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType ThermalMgmt_SetFanSpeed(uint8 SpeedPercent);

/**
 * @brief Get current fan speed
 * @return Current fan speed (0-100%)
 */
uint8 ThermalMgmt_GetFanSpeed(void);

/**
 * @brief Force maximum cooling (emergency)
 * @return None
 */
void ThermalMgmt_EmergencyCooling(void);

/**
 * @brief Check if thermal shutdown is required
 * @return TRUE if shutdown recommended, FALSE otherwise
 */
boolean ThermalMgmt_IsShutdownRequired(void);

#if (THERMALMGMT_EVENT_CALLBACK_API == STD_ON)
/**
 * @brief Set event callback
 * @param[in] Callback Function to call on thermal events
 */
void ThermalMgmt_SetEventCallback(ThermalMgmt_EventCallbackType Callback);
#endif

#if (THERMALMGMT_VERSION_INFO_API == STD_ON)
/**
 * @brief Get service version information
 * @param[out] versionInfoPtr Pointer to version info structure
 */
void ThermalMgmt_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
#endif

#endif /* THERMALMGMT_H */
