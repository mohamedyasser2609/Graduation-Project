/**
 * @file WDG_PBCfg.c
 * @brief Watchdog Driver Post-Build Configuration for TM4C123GH6PM
 */

#include "WDG.h"

/* ===================[Watchdog Configuration]=================== */
const Wdg_ConfigType Wdg_Config = {
    .Instance = WDG_INSTANCE_0,
    .InitialTimeoutTicks = 16000000u,    /* 1 second @ 16 MHz */
    .MaxTimeoutTicks = 0xFFFFFFFFu,
    .MinTimeoutTicks = 1000u,
    .ResetEnable = TRUE,
    .InterruptEnable = TRUE,
    .NotificationCallback = NULL_PTR
};
