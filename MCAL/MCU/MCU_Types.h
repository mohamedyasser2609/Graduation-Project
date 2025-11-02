/**
 * @file Mcu_Types.h
 * @brief MCU Driver Types for TM4C123GH6PM
 * @details Type definitions for AUTOSAR-compliant MCU driver
 *
 * @author Mohamed Yasser
 * @date Nov 1, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_MCU_MCU_TYPES_H_
#define MCAL_MCU_MCU_TYPES_H_

#include "../../CONFIG/Std_Types.h"

/* ===================[Clock Settings]=================== */
typedef enum {
    MCU_CLOCK_MOSC_16MHZ = 0,    /* Main oscillator 16MHz (default) */
    MCU_CLOCK_PLL_80MHZ,          /* PLL 80MHz */
    MCU_CLOCK_PLL_50MHZ,          /* PLL 50MHz */
    MCU_CLOCK_PLL_40MHZ,          /* PLL 40MHz */
    MCU_CLOCK_PLL_25MHZ,          /* PLL 25MHz */
    MCU_CLOCK_PIOSC_16MHZ         /* Precision internal oscillator 16MHz */
} Mcu_ClockType;

/* ===================[PLL Status]=================== */
typedef enum {
    MCU_PLL_LOCKED = 0,           /* PLL is locked and stable */
    MCU_PLL_UNLOCKED,             /* PLL is not locked */
    MCU_PLL_STATUS_UNDEFINED      /* PLL status cannot be determined */
} Mcu_PllStatusType;

/* ===================[Reset Reason]=================== */
typedef enum {
    MCU_POWER_ON_RESET = 0,       /* Power-on reset */
    MCU_EXTERNAL_RESET,           /* External reset pin */
    MCU_BROWN_OUT_RESET,          /* Brown-out reset */
    MCU_WATCHDOG_RESET,           /* Watchdog timer reset */
    MCU_SOFTWARE_RESET,           /* Software reset */
    MCU_RESET_UNDEFINED           /* Unknown reset reason */
} Mcu_ResetType;

/* ===================[Power Modes]=================== */
typedef enum {
    MCU_MODE_NORMAL = 0,          /* Normal run mode */
    MCU_MODE_SLEEP,               /* Sleep mode (CPU halted, peripherals active) */
    MCU_MODE_DEEP_SLEEP           /* Deep sleep mode (CPU and most peripherals halted) */
} Mcu_ModeType;

/* ===================[Clock Configuration]=================== */
typedef struct {
    Mcu_ClockType ClockSetting;   /* Clock source and frequency */
    boolean PllEnabled;           /* Enable PLL */
    uint32 SysClockFreq;          /* System clock frequency in Hz */
} Mcu_ClockConfigType;

/* ===================[MCU Configuration]=================== */
typedef struct {
    const Mcu_ClockConfigType* ClockConfig;  /* Pointer to clock configuration */
    uint8 NumberOfClockSettings;              /* Number of clock configurations */
} Mcu_ConfigType;

#endif /* MCAL_MCU_MCU_TYPES_H_ */