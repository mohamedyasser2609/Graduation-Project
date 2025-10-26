/*
 * @file PWM_Cfg.h
 * @brief PWM Driver Configuration Header for TM4C123GH6PM
 * @details This file contains compile-time configuration parameters for the PWM driver
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

#ifndef MCAL_PWM_PWM_CFG_H_
#define MCAL_PWM_PWM_CFG_H_

/* ===================[Configuration Parameters]=================== */

/**
 * @brief Enable/Disable PWM Development Error Detection
 * @details Set to TRUE to enable DET error reporting
 */
#define PWM_DEV_ERROR_DETECT            (FALSE)

/**
 * @brief Enable/Disable PWM Version Info API
 * @details Set to TRUE to enable Pwm_GetVersionInfo() API
 */
#define PWM_VERSION_INFO_API            (TRUE)

/**
 * @brief Enable/Disable PWM DeInit API
 * @details Set to TRUE to enable Pwm_DeInit() API
 */
#define PWM_DE_INIT_API                 (TRUE)

/**
 * @brief Enable/Disable PWM Set Duty Cycle API
 * @details Set to TRUE to enable Pwm_SetDutyCycle() API
 */
#define PWM_SET_DUTY_CYCLE_API          (TRUE)

/**
 * @brief Enable/Disable PWM Set Period And Duty API
 * @details Set to TRUE to enable Pwm_SetPeriodAndDuty() API
 */
#define PWM_SET_PERIOD_AND_DUTY_API     (TRUE)

/**
 * @brief Enable/Disable PWM Set Output To Idle API
 * @details Set to TRUE to enable Pwm_SetOutputToIdle() API
 */
#define PWM_SET_OUTPUT_TO_IDLE_API      (TRUE)

/**
 * @brief Enable/Disable PWM Get Output State API
 * @details Set to TRUE to enable Pwm_GetOutputState() API
 */
#define PWM_GET_OUTPUT_STATE_API        (TRUE)

/**
 * @brief Enable/Disable PWM Notification Support
 * @details Set to TRUE to enable edge notification callbacks
 */
#define PWM_NOTIFICATION_SUPPORTED      (FALSE)

/**
 * @brief Default PWM Frequency (Hz)
 * @details Default frequency for PWM signals (1 kHz for LED dimming)
 */
#define PWM_DEFAULT_FREQUENCY_HZ        (1000u)

/**
 * @brief PWM Clock Divider
 * @details System clock divider for PWM module
 *          Options: 2, 4, 8, 16, 32, 64
 */
#define PWM_CLOCK_DIVIDER               (64u)

/**
 * @brief Maximum number of PWM channels
 * @details Total number of PWM channels configured in the system
 */
#define PWM_MAX_CHANNELS                (8u)

#endif /* MCAL_PWM_PWM_CFG_H_ */
