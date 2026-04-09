/**
 * @file MOTOR_PBCfg.c
 * @brief Motor Driver Post-Build Configuration for Cytron MDD10A Rev2.0
 * @details Configuration for dual-channel motor control
 *
 * Hardware: Cytron MDD10A Rev2.0 Dual Channel Motor Driver
 * - Motor 1 (Left):  PWM Channel 3, Direction Pin PE1
 * - Motor 2 (Right): PWM Channel 4, Direction Pin PE2
 *
 * PWM Configuration:
 * - Frequency: 20 kHz (typical for motor control)
 * - PWM Clock: 16 MHz / 2 = 8 MHz
 * - Period: 400 ticks (8 MHz / 20 kHz)
 * - Initial Duty Cycle: 0% (motors stopped)
 *
 * Direction Control:
 * - HIGH = Forward (or Reverse if inverted)
 * - LOW = Reverse (or Forward if inverted)
 * - Brake: Direction HIGH + PWM OFF
 * - Coast: Direction LOW + PWM OFF
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

/* ===================[Includes]=================== */
#include "MOTOR.h"
#include "../../MCAL/PWM/PWM.h"
#include "../../MCAL/GPIO/GPIO_Types.h"

/* ===================[Motor Channel Definitions]=================== */

/**
 * @brief PWM Channel IDs for motors
 * @details These should match PWM channel numbers in PWM_PBCfg.c
 */
#define PWM_CHANNEL_MOTOR_LEFT     (3u)    /* Left motor PWM channel */
#define PWM_CHANNEL_MOTOR_RIGHT    (4u)    /* Right motor PWM channel */

/**
 * @brief GPIO Channel IDs for motor direction pins
 * @details Direction control pins for each motor
 */
#define GPIO_CHANNEL_MOTOR_LEFT_DIR    GPIO_CHANNEL(GPIO_PORT_E, GPIO_PIN_1)  /* PE1 - Left motor direction */
#define GPIO_CHANNEL_MOTOR_RIGHT_DIR   GPIO_CHANNEL(GPIO_PORT_E, GPIO_PIN_2)  /* PE2 - Right motor direction */

/* ===================[Motor Channel Configurations]=================== */

/**
 * @brief Array of motor channel configurations
 */
static const Motor_ChannelConfigType Motor_ChannelConfigs[] = {
    {
        /* Left Motor Configuration */
        .ChannelId = MOTOR_CHANNEL_LEFT,
        .PwmChannel = PWM_CHANNEL_MOTOR_LEFT,
        .DirectionPin = GPIO_CHANNEL_MOTOR_LEFT_DIR,
        .InvertDirection = FALSE,              /* Set to TRUE if motor direction is reversed */
        .MaxSpeedPercent = 100u,               /* Maximum speed: 100% */
        .MinSpeedPercent = 0u                  /* Minimum speed: 0% (no deadband) */
    },
    {
        /* Right Motor Configuration */
        .ChannelId = MOTOR_CHANNEL_RIGHT,
        .PwmChannel = PWM_CHANNEL_MOTOR_RIGHT,
        .DirectionPin = GPIO_CHANNEL_MOTOR_RIGHT_DIR,
        .InvertDirection = FALSE,              /* Set to TRUE if motor direction is reversed */
        .MaxSpeedPercent = 100u,               /* Maximum speed: 100% */
        .MinSpeedPercent = 0u                  /* Minimum speed: 0% (no deadband) */
    }
};

/**
 * @brief Main Motor Configuration Structure
 * @details Contains all motor channels for the system
 */
const Motor_ConfigType Motor_Config = {
    .ChannelCount = MOTOR_CHANNEL_COUNT,
    .Channels = Motor_ChannelConfigs
};

