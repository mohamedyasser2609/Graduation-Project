/**
 * @file QEI_PBCfg.c
 * @brief QEI Driver Post-Build Configuration for TM4C123GH6PM
 */

#include "QEI.h"

/* ===================[QEI Configuration]=================== */
const Qei_ConfigType Qei_ChannelConfigs[QEI_MAX_CONFIGURED_MODULES] = {
    {
        .Module = QEI_MODULE_0,
        .SignalMode = QEI_SIGNAL_MODE_QUADRATURE,
        .ResetMode = QEI_RESET_MODE_FREE_RUNNING,
        .SwapChannels = FALSE,
        .InvertChannelA = FALSE,
        .InvertChannelB = FALSE,
        .InvertIndex = FALSE,
        .EnableVelocityCapture = TRUE,
        .VelocityPreDiv = QEI_VELOCITY_PREDIV_1,
        .VelocityTimerLoad = 160000u,      /* Velocity period window */
        .EnableFilter = TRUE,
        .FilterCount = 4u,
        .DebugStallEnable = TRUE,
        .MaxPosition = 0xFFFFFFFFu,
        .InitialPosition = 0u,
        .InterruptMask = QEI_INT_ERROR | QEI_INT_DIRECTION,
        .NotificationCallback = NULL_PTR
    },
    {
        .Module = QEI_MODULE_1,
        .SignalMode = QEI_SIGNAL_MODE_QUADRATURE,
        .ResetMode = QEI_RESET_MODE_FREE_RUNNING,
        .SwapChannels = FALSE,
        .InvertChannelA = FALSE,
        .InvertChannelB = FALSE,
        .InvertIndex = FALSE,
        .EnableVelocityCapture = TRUE,
        .VelocityPreDiv = QEI_VELOCITY_PREDIV_1,
        .VelocityTimerLoad = 160000u,
        .EnableFilter = TRUE,
        .FilterCount = 4u,
        .DebugStallEnable = TRUE,
        .MaxPosition = 0xFFFFFFFFu,
        .InitialPosition = 0u,
        .InterruptMask = QEI_INT_ERROR | QEI_INT_DIRECTION,
        .NotificationCallback = NULL_PTR
    }
};

/* Backward compatibility single-config alias (module 0) */
const Qei_ConfigType Qei_Config = {
    .Module = QEI_MODULE_0,
    .SignalMode = QEI_SIGNAL_MODE_QUADRATURE,
    .ResetMode = QEI_RESET_MODE_FREE_RUNNING,
    .SwapChannels = FALSE,
    .InvertChannelA = FALSE,
    .InvertChannelB = FALSE,
    .InvertIndex = FALSE,
    .EnableVelocityCapture = TRUE,
    .VelocityPreDiv = QEI_VELOCITY_PREDIV_1,
    .VelocityTimerLoad = 160000u,      /* Velocity period window */
    .EnableFilter = TRUE,
    .FilterCount = 4u,
    .DebugStallEnable = TRUE,
    .MaxPosition = 0xFFFFFFFFu,
    .InitialPosition = 0u,
    .InterruptMask = QEI_INT_ERROR | QEI_INT_DIRECTION,
    .NotificationCallback = NULL_PTR
};
