/**
 * @file WDG_PBCfg.c
 * @brief Watchdog Driver Post-Build Configuration for TM4C123GH6PM
 * @details Safety-critical WDG configuration: 500ms timeout at 80MHz
 *
 * Safety Design:
 * - 500ms timeout allows 50 Safety Task cycles (10ms each)
 * - ONLY Safety Task feeds the WDG
 * - Reset enabled on timeout (fail-safe)
 */

#include "WDG.h"

/* ===================[Watchdog Configuration]=================== */
const Wdg_ConfigType Wdg_Config = {
    .Instance = WDG_INSTANCE_0,
    .InitialTimeoutTicks = 40000000u,    /* 500ms @ 80 MHz */
    .MaxTimeoutTicks = 0xFFFFFFFFu,
    .MinTimeoutTicks = 1000u,
    .ResetEnable = TRUE,                 /* CRITICAL: Enable reset on timeout */
    .InterruptEnable = TRUE,             /* Interrupt before reset for logging */
    .NotificationCallback = NULL_PTR     /* TODO: Add pre-reset logging */
};

