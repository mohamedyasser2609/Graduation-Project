/*
 * @file PWM_PBCfg.c
 * @brief PWM Driver Post-Build Configuration for TM4C123GH6PM
 * @details Configuration for RGB LED PWM channels
 *
 * RGB LED PWM Mapping:
 * - Red LED (PF1)   → M1PWM5 (Module 1, Generator 2, Output B)
 * - Blue LED (PF2)  → M1PWM6 (Module 1, Generator 3, Output A)
 * - Green LED (PF3) → M1PWM7 (Module 1, Generator 3, Output B)
 *
 * PWM Configuration:
 * - Frequency: 1 kHz (1000 Hz)
 * - PWM Clock: 16 MHz / 64 = 250 kHz
 * - Period: 250 ticks (250 kHz / 1 kHz)
 * - Initial Duty Cycle: 0% (LEDs OFF)
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

/* ===================[Includes]=================== */
#include "PWM.h"

/* ===================[PWM Channel Definitions]=================== */

/**
 * @brief PWM Channel IDs
 */
#define PWM_CHANNEL_RED         (0u)    /* Red LED PWM channel */
#define PWM_CHANNEL_BLUE        (1u)    /* Blue LED PWM channel */
#define PWM_CHANNEL_GREEN       (2u)    /* Green LED PWM channel */

/* ===================[PWM Configuration Array]=================== */

/**
 * @brief Array of all PWM channel configurations
 */
static const Pwm_ConfigChannelType Pwm_Channels[] = {
    {   /* Red LED */
        .Channel = PWM_CHANNEL_RED,
        .Module = PWM_MODULE_1,
        .Generator = PWM_GEN_2,
        .Output = PWM_OUT_B,
        .ChannelClass = PWM_FIXED_PERIOD,
        .Polarity = PWM_HIGH_POLARITY,
        .IdleState = PWM_IDLE_LOW,
        .DefaultPeriod = 250u,
        .DefaultDutyCycle = 0x0000,
        .NotificationPtr = NULL_PTR
    },
    {   /* Blue LED */
        .Channel = PWM_CHANNEL_BLUE,
        .Module = PWM_MODULE_1,
        .Generator = PWM_GEN_3,
        .Output = PWM_OUT_A,
        .ChannelClass = PWM_FIXED_PERIOD,
        .Polarity = PWM_HIGH_POLARITY,
        .IdleState = PWM_IDLE_LOW,
        .DefaultPeriod = 250u,
        .DefaultDutyCycle = 0x0000,
        .NotificationPtr = NULL_PTR
    },
    {   /* Green LED */
        .Channel = PWM_CHANNEL_GREEN,
        .Module = PWM_MODULE_1,
        .Generator = PWM_GEN_3,
        .Output = PWM_OUT_B,
        .ChannelClass = PWM_FIXED_PERIOD,
        .Polarity = PWM_HIGH_POLARITY,
        .IdleState = PWM_IDLE_LOW,
        .DefaultPeriod = 250u,
        .DefaultDutyCycle = 0x0000,
        .NotificationPtr = NULL_PTR
    }
};

/**
 * @brief Main PWM Configuration Structure
 * @details Contains all PWM channels for the system
 */
const Pwm_ConfigType Pwm_Configuration = {
    Pwm_Channels,
    3u
};
