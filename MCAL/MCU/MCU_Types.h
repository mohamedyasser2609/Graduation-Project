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
    MCU_CLOCK_16MHZ = 0,    /**< Main oscillator 16MHz (no PLL) */
    MCU_CLOCK_20MHZ,        /**< PLL configured for 20MHz */
    MCU_CLOCK_25MHZ,        /**< PLL configured for 25MHz */
    MCU_CLOCK_40MHZ,        /**< PLL configured for 40MHz */
    MCU_CLOCK_50MHZ,        /**< PLL configured for 50MHz */
    MCU_CLOCK_80MHZ         /**< PLL configured for 80MHz */
} Mcu_ClockType;

/* ===================[PLL Status]=================== */
typedef enum {
    MCU_PLL_LOCKED = 0,     /**< PLL is locked and stable */
    MCU_PLL_UNLOCKED,       /**< PLL is not locked */
    MCU_PLL_STATUS_UNDEFINED /**< PLL status cannot be determined */
} Mcu_PllStatusType;

/* ===================[Reset Reasons]=================== */
typedef enum {
    MCU_POWER_ON_RESET = 0, /**< Power-on reset */
    MCU_EXTERNAL_RESET,     /**< External reset pin */
    MCU_WATCHDOG_RESET,     /**< Watchdog timer reset */
    MCU_BROWN_OUT_RESET,    /**< Brown-out reset */
    MCU_SOFTWARE_RESET,     /**< Software reset */
    MCU_RESET_UNDEFINED     /**< Unknown reset reason */
} Mcu_ResetType;

/* ===================[Power Modes]=================== */
typedef enum {
    MCU_MODE_NORMAL = 0,    /**< Normal run mode */
    MCU_MODE_SLEEP,         /**< Sleep mode (CPU halted, peripherals active) */
    MCU_MODE_DEEP_SLEEP     /**< Deep sleep mode (CPU and most peripherals halted) */
} Mcu_ModeType;

/* ===================[Clock Source]=================== */
typedef enum {
    MCU_CLOCK_SOURCE_MOSC = 0,   /**< Main oscillator (external crystal) */
    MCU_CLOCK_SOURCE_PIOSC,      /**< Precision internal oscillator */
    MCU_CLOCK_SOURCE_PIOSC_DIV4, /**< PIOSC divided by 4 */
    MCU_CLOCK_SOURCE_LFIOSC      /**< Low-frequency internal oscillator */
} Mcu_ClockSourceType;

/* ===================[MCU Configuration]=================== */
typedef struct {
    Mcu_ClockType DefaultClock;       /**< Default clock setting */
    Mcu_ClockSourceType ClockSource;  /**< Clock source selection */
    boolean PllEnabled;               /**< TRUE to enable PLL */
    uint8 NumberOfClockSettings;      /**< Number of available clock configurations */
} Mcu_ConfigType;


#endif /* MCAL_MCU_MCU_TYPES_H_ */
