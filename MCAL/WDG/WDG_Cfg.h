/**
 * @file WDG_Cfg.h
 * @brief Watchdog Driver Configuration Header
 */

#ifndef MCAL_WDG_WDG_CFG_H_
#define MCAL_WDG_WDG_CFG_H_

#include "../../CONFIG/Std_Types.h"
#include "WDG_Types.h"

/* ===================[Pre-Compile Options]=================== */
#define WDG_DEV_ERROR_DETECT            (STD_ON)
#define WDG_VERSION_INFO_API            (STD_ON)
#define WDG_ENABLE_WINDOW_MODE          (STD_OFF)
#define WDG_DEFAULT_TRIGGER_MODE        (WDG_TRIGGER_MODE_NORMAL)

/* Watchdog stall option during debug */
#define WDG_ALLOW_DEBUG_STALL           (STD_ON)

/* ===================[External Configuration]=================== */
extern const Wdg_ConfigType Wdg_Config;

#endif /* MCAL_WDG_WDG_CFG_H_ */
