/**
 * @file Mcu_PBcfg.c
 * @brief MCU Driver Post-Build Configuration for TM4C123GH6PM
 * @details Post-build configuration for AUTOSAR-compliant MCU driver
 *
 * @author Mohamed Yasser
 * @date Nov 1, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#include "Mcu.h"
#include "Mcu_Cfg.h"

/* ===================[MCU Configuration]=================== */

/**
 * @brief Default MCU configuration - 80MHz with PLL
 */
const Mcu_ConfigType Mcu_Config_80MHz = {
    .DefaultClock = MCU_CLOCK_80MHZ,
    .ClockSource = MCU_CLOCK_SOURCE_MOSC,
    .PllEnabled = TRUE,
    .NumberOfClockSettings = 6
};

/**
 * @brief MCU configuration - 16MHz without PLL
 */
const Mcu_ConfigType Mcu_Config_16MHz = {
    .DefaultClock = MCU_CLOCK_16MHZ,
    .ClockSource = MCU_CLOCK_SOURCE_MOSC,
    .PllEnabled = FALSE,
    .NumberOfClockSettings = 6
};

/**
 * @brief MCU configuration - 50MHz with PLL
 */
const Mcu_ConfigType Mcu_Config_50MHz = {
    .DefaultClock = MCU_CLOCK_50MHZ,
    .ClockSource = MCU_CLOCK_SOURCE_MOSC,
    .PllEnabled = TRUE,
    .NumberOfClockSettings = 6
};

/**
 * @brief Default configuration pointer (80MHz)
 */
const Mcu_ConfigType* Mcu_ConfigPtr = &Mcu_Config_80MHz;