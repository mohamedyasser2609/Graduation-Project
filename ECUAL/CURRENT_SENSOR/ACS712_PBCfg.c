/**
 * @file ACS712_PBCfg.c
 * @brief Post-build configuration for ACS712 Current Sensor Driver
 * @details Configuration for 2x ACS712-30A sensors for motor current monitoring
 *
 * Hardware Setup:
 * - Channel 0: Left motor current sensor on ADC0 (PE3/AIN0)
 * - Channel 1: Right motor current sensor on ADC1 (PE2/AIN1)
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "ACS712.h"

/* ===================[Channel Configurations]=================== */
static const ACS712_ChannelConfigType ACS712_ChannelConfigs[] =
{
    /* Channel 0: Left Motor Current Sensor */
    {
        .ChannelId          = 0u,
        .AdcChannel         = 0u,                    /* ADC0 - PE3/AIN0 */
        .Model              = ACS712_MODEL_30A,
        .OverloadThreshold  = 25.0f,                 /* 25A overload threshold */
        .VrefV              = 3.3f,                  /* TM4C123 ADC reference */
        .ZeroCurrentVoltage = 2.5f,                  /* VCC/2 at zero current */
        .FilterSamples      = 8u                     /* 8 samples averaging */
    },
    /* Channel 1: Right Motor Current Sensor */
    {
        .ChannelId          = 1u,
        .AdcChannel         = 1u,                    /* ADC1 - PE2/AIN1 */
        .Model              = ACS712_MODEL_30A,
        .OverloadThreshold  = 25.0f,                 /* 25A overload threshold */
        .VrefV              = 3.3f,
        .ZeroCurrentVoltage = 2.5f,
        .FilterSamples      = 8u
    }
};

/* ===================[Driver Configuration]=================== */
const ACS712_ConfigType ACS712_Config =
{
    .NumChannels = 2u,
    .Channels    = ACS712_ChannelConfigs
};
