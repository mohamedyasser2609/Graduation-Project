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
#include "Mcu_Cfg.h"

/* ===================[Clock Settings]=================== */
typedef enum {
    MCU_CLOCK_16MHZ = 0,    /* Main oscillator 16MHz (no PLL) */
    MCU_CLOCK_20MHZ,        /* PLL: 20MHz */
    MCU_CLOCK_25MHZ,        /* PLL: 25MHz */
    MCU_CLOCK_40MHZ,        /* PLL: 40MHz */
    MCU_CLOCK_50MHZ,        /* PLL: 50MHz */
    MCU_CLOCK_80MHZ         /* PLL: 80MHz (maximum) */
} Mcu_ClockType;

/* ===================[PLL Status]=================== */
typedef enum {
    MCU_PLL_LOCKED = 0,     /* PLL is locked and stable */
    MCU_PLL_UNLOCKED,       /* PLL is not locked */
    MCU_PLL_STATUS_UNDEFINED /* PLL status cannot be determined */
} Mcu_PllStatusType;

/* ===================[Reset Reasons]=================== */
typedef enum {
    MCU_POWER_ON_RESET = 0,
    MCU_EXTERNAL_RESET,
    MCU_WATCHDOG_RESET,
    MCU_BROWN_OUT_RESET,
    MCU_SOFTWARE_RESET,
    MCU_RESET_UNDEFINED
} Mcu_ResetType;

/* ===================[Power Modes]=================== */
typedef enum {
    MCU_MODE_NORMAL = 0,    /* Normal run mode */
    MCU_MODE_SLEEP,         /* Sleep mode (CPU halted, peripherals active) */
    MCU_MODE_DEEP_SLEEP     /* Deep sleep mode (CPU and most peripherals halted) */
} Mcu_ModeType;

/* ===================[Clock Source]=================== */
typedef enum {
    MCU_CLOCK_SOURCE_MOSC = 0,  /* Main Oscillator (16MHz crystal) */
    MCU_CLOCK_SOURCE_PIOSC,     /* Precision Internal Oscillator (16MHz) */
    MCU_CLOCK_SOURCE_PIOSC_DIV4,/* PIOSC / 4 (4MHz) */
    MCU_CLOCK_SOURCE_LFIOSC     /* Low-Frequency Internal Oscillator (30kHz) */
} Mcu_ClockSourceType;

/* ===================[MCU Configuration]=================== */
typedef struct {
    Mcu_ClockType DefaultClock;         /* Default clock setting */
    Mcu_ClockSourceType ClockSource;    /* Clock source */
    boolean PllEnabled;                 /* Enable PLL */
    uint8 NumberOfClockSettings;        /* Number of available clock configs */
} Mcu_ConfigType;

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
