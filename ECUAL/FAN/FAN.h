/**
 * @file FAN.h
 * @brief Fan Control Driver API
 * @details AUTOSAR-compliant driver for PWM-controlled cooling fans
 *
 * Hardware: Delta FFB0812EHE 80mm High-Speed Fan
 * - 12V DC, 1.35A max
 * - 12000 RPM max
 * - 4-wire: VCC, GND, Tach, PWM
 * - PWM frequency: 25kHz
 *
 * Features:
 * - Multi-fan support (up to 4 fans)
 * - PWM speed control (0-100%)
 * - Optional tachometer RPM feedback
 * - Stall detection (if tach enabled)
 * - Fault notification callback
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#ifndef FAN_H
#define FAN_H

/* ===================[Includes]=================== */
#include "FAN_Types.h"
#include "FAN_Cfg.h"

/* ===================[API Declarations]=================== */

/**
 * @brief Initialize the Fan Control driver
 * @param[in] ConfigPtr Pointer to configuration structure
 * @return None
 * @pre PWM driver must be initialized
 * @post Driver is ready for fan control
 */
void Fan_Init(const Fan_ConfigType* ConfigPtr);

#if (FAN_DEINIT_API == STD_ON)
/**
 * @brief De-initialize the Fan Control driver
 * @return None
 * @post All fans are stopped
 */
void Fan_DeInit(void);
#endif

/**
 * @brief Set fan speed
 * @param[in] FanId Fan identifier
 * @param[in] SpeedPercent Speed (0-100%)
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType Fan_SetSpeed(Fan_IdType FanId, Fan_SpeedType SpeedPercent);

/**
 * @brief Set all fans to same speed
 * @param[in] SpeedPercent Speed (0-100%)
 * @return E_OK if all succeeded, E_NOT_OK if any failed
 */
Std_ReturnType Fan_SetAllSpeed(Fan_SpeedType SpeedPercent);

/**
 * @brief Stop a specific fan
 * @param[in] FanId Fan identifier
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType Fan_Stop(Fan_IdType FanId);

/**
 * @brief Stop all fans (emergency stop)
 * @return None
 */
void Fan_StopAll(void);

/**
 * @brief Get current fan speed setting
 * @param[in] FanId Fan identifier
 * @return Current speed (0-100%), or 0 on error
 */
Fan_SpeedType Fan_GetSpeed(Fan_IdType FanId);

/**
 * @brief Get fan runtime data
 * @param[in] FanId Fan identifier
 * @param[out] DataPtr Pointer to store fan data
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType Fan_GetData(Fan_IdType FanId, Fan_DataType* DataPtr);

/**
 * @brief Get fan state
 * @param[in] FanId Fan identifier
 * @return Fan state
 */
Fan_StateType Fan_GetState(Fan_IdType FanId);

/**
 * @brief Get driver status
 * @return Driver status
 */
Fan_StatusType Fan_GetStatus(void);

#if (FAN_TACHOMETER_SUPPORT == STD_ON)
/**
 * @brief Get measured RPM
 * @param[in] FanId Fan identifier
 * @param[out] Rpm Pointer to store RPM value
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType Fan_GetRpm(Fan_IdType FanId, uint16* Rpm);

/**
 * @brief Update tachometer readings (call periodically)
 * @return None
 * @note Called by FreeRTOS task or timer ISR
 */
void Fan_UpdateTachometer(void);
#endif

#if (FAN_FAULT_CALLBACK_API == STD_ON)
/**
 * @brief Set fault notification callback
 * @param[in] Callback Function to call on fan fault
 */
void Fan_SetFaultCallback(Fan_FaultCallbackType Callback);
#endif

#if (FAN_VERSION_INFO_API == STD_ON)
/**
 * @brief Get driver version information
 * @param[out] versionInfoPtr Pointer to version info structure
 */
void Fan_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
#endif

#endif /* FAN_H */
