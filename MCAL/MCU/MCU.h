/**
 * @file Mcu.h
 * @brief MCU Driver for TM4C123GH6PM (AUTOSAR Compliant)
 * @details Microcontroller Unit driver with integrated PLL configuration
 *
 * Features:
 * - System clock configuration (16MHz to 80MHz)
 * - PLL configuration and control
 * - Peripheral clock gating
 * - Power mode management
 * - Reset handling
 * - Clock source selection
 *
 * AUTOSAR Version: 4.3.1
 * @author Mohamed Yasser
 * @date Nov 2, 2025
 * @version 1.0.0
 */

#ifndef MCAL_MCU_MCU_H_
#define MCAL_MCU_MCU_H_

#include "../../CONFIG/Std_Types.h"
#include "Mcu_Types.h"
#include "Mcu_Cfg.h"

/* ===================[Function Prototypes]=================== */

/**
 * @brief Initialize MCU driver
 * @param ConfigPtr Pointer to MCU configuration
 */
void Mcu_Init(const Mcu_ConfigType* ConfigPtr);

/**
 * @brief Initialize clock with specified setting
 * @param ClockSetting Clock configuration to use
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType Mcu_InitClock(Mcu_ClockType ClockSetting);

/**
 * @brief Activate PLL clock (switch system clock to PLL)
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType Mcu_DistributePllClock(void);

/**
 * @brief Get PLL status
 * @return PLL lock status
 */
Mcu_PllStatusType Mcu_GetPllStatus(void);

/**
 * @brief Get reset reason
 * @return Reset cause
 */
Mcu_ResetType Mcu_GetResetReason(void);

/**
 * @brief Get raw reset reason register value
 * @return Raw reset register value
 */
uint32 Mcu_GetResetRawValue(void);

/**
 * @brief Perform microcontroller reset
 */
void Mcu_PerformReset(void);

/**
 * @brief Set MCU power mode
 * @param McuMode Power mode to enter
 */
void Mcu_SetMode(Mcu_ModeType McuMode);

/**
 * @brief Get current system clock frequency
 * @return System clock frequency in Hz
 */
uint32 Mcu_GetSystemClock(void);

/**
 * @brief Enable peripheral clock
 * @param Peripheral Peripheral identifier (use SYSCTL bits)
 */
void Mcu_EnablePeripheralClock(uint32 Peripheral);

/**
 * @brief Disable peripheral clock
 * @param Peripheral Peripheral identifier (use SYSCTL bits)
 */
void Mcu_DisablePeripheralClock(uint32 Peripheral);

#endif /* MCAL_MCU_MCU_H_ */
