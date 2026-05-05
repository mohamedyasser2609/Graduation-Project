/**
 * @file ENCODER_PBCfg.c
 * @brief Encoder Driver Post-Build Configuration for EMG49 Motor Encoder
 * @details Configuration for dual EMG49 encoders (left/right)
 *
 * @author Mohamed Yasser
 * @date Nov 5, 2025
 * @version 1.1.0
 */

#include "ENCODER.h"
#include "../../MCAL/QEI/QEI_Cfg.h"

/* ===================[Encoder Configuration]=================== */
/**
 * @note Velocity Timer Calculation:
 *       QEI VelocityTimerLoad = 4000000 @ 80 MHz -> 50000 us (50ms) window.
 *       QEI speed register reports counts per 50 ms.
 *       Driver scales to counts/second and applies EMA filtering.
 */
const Encoder_ChannelConfigType Encoder_ChannelConfigs[ENCODER_MAX_CHANNELS] = {
    /* Left encoder on QEI0 */
    {
        .ChannelId = ENCODER_CHANNEL_LEFT,
        .QeiModule = QEI_MODULE_0,
        .QeiConfigPtr = &Qei_ChannelConfigs[0],
        .PulsesPerRevolution = 245u,
        .QuadratureCountsPerRev = 980u,
        .VelocityTimerPeriodUs = 50000u,
        .MaxPosition = 0xFFFFFFFFu,
        .ReverseDirection = FALSE,
        .EnableVelocityFilter = TRUE,
        .FilterCfg = {
            .DeadbandCountsPerSec = 30u,
            .SpikeThresholdCountsPerSec = 10000u,
            .Alpha = 200u,                /* ~78% new, 22% old */
            .DefaultAlpha = 128u
        }
    },
    /* Right encoder on QEI1 */
    {
        .ChannelId = ENCODER_CHANNEL_RIGHT,
        .QeiModule = QEI_MODULE_1,
        .QeiConfigPtr = &Qei_ChannelConfigs[1],
        .PulsesPerRevolution = 245u,
        .QuadratureCountsPerRev = 980u,
        .VelocityTimerPeriodUs = 50000u,
        .MaxPosition = 0xFFFFFFFFu,
        .ReverseDirection = FALSE,        /* Toggled: new board wiring */
        .EnableVelocityFilter = TRUE,
        .FilterCfg = {
            .DeadbandCountsPerSec = 30u,
            .SpikeThresholdCountsPerSec = 10000u,
            .Alpha = 200u,
            .DefaultAlpha = 128u
        }
    }
};

const Encoder_ConfigType Encoder_Config = {
    .ChannelCount = ENCODER_MAX_CHANNELS,
    .Channels = Encoder_ChannelConfigs
};

