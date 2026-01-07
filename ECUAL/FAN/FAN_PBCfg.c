/**
 * @file FAN_PBCfg.c
 * @brief Post-build configuration for Fan Control Driver
 * @details Configuration for 2x Delta FFB0812EHE cooling fans
 *
 * Fan Placement:
 * - Fan 0: Main cooling intake
 * - Fan 1: Exhaust/secondary cooling
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "FAN.h"

/* ===================[Fan Configurations]=================== */
static const Fan_ConfigItemType Fan_Configs[] =
{
    /* Fan 0: Main Cooling (Intake) */
    {
        .FanId          = 0u,
        .Mode           = FAN_MODE_PWM,
        .PwmChannel     = 0u,               /* PWM0 - M0PWM0 (PB6) */
        .TachChannel    = 0xFFu,            /* Tachometer not used */
        .MinDutyPercent = FAN_FFB0812EHE_MIN_DUTY,
        .MaxRpm         = FAN_FFB0812EHE_MAX_RPM,
        .PulsesPerRev   = FAN_FFB0812EHE_PULSES_PER_REV
    },
    /* Fan 1: Exhaust Cooling */
    {
        .FanId          = 1u,
        .Mode           = FAN_MODE_PWM,
        .PwmChannel     = 1u,               /* PWM1 - M0PWM1 (PB7) */
        .TachChannel    = 0xFFu,            /* Tachometer not used */
        .MinDutyPercent = FAN_FFB0812EHE_MIN_DUTY,
        .MaxRpm         = FAN_FFB0812EHE_MAX_RPM,
        .PulsesPerRev   = FAN_FFB0812EHE_PULSES_PER_REV
    }
};

/* ===================[Driver Configuration]=================== */
const Fan_ConfigType Fan_Config =
{
    .NumFans = 2u,
    .Fans    = Fan_Configs
};
