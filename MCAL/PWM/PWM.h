/*
 * @file PWM.h
 * @brief PWM Driver Interface for TM4C123GH6PM
 * @details AUTOSAR-compliant PWM driver for controlling LED brightness and motor speed
 *
 * Features:
 * - Configurable PWM frequency and duty cycle
 * - Support for multiple PWM channels
 * - Hardware-based PWM generation
 * - Precise timing control
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

#ifndef MCAL_PWM_PWM_H_
#define MCAL_PWM_PWM_H_

/* ===================[Includes]=================== */
#include <Std_types.h>
#include "PWM_Cfg.h"

/* ===================[Type Definitions]=================== */

/**
 * @brief PWM Channel Type
 * @details Numeric identifier for a PWM channel
 */
typedef uint8 Pwm_ChannelType;

/**
 * @brief PWM Period Type
 * @details PWM period value in ticks
 */
typedef uint16 Pwm_PeriodType;

/**
 * @brief PWM Duty Cycle Type
 * @details Duty cycle value: 0x0000 = 0%, 0x8000 = 100%
 */
typedef uint16 Pwm_DutyCycleType;

/**
 * @brief PWM Output State Type
 * @details Current state of PWM output
 */
typedef enum {
    PWM_LOW = 0,      /* PWM output is LOW */
    PWM_HIGH = 1      /* PWM output is HIGH */
} Pwm_OutputStateType;

/**
 * @brief PWM Channel Class Type
 * @details Classification of PWM channel behavior
 */
typedef enum {
    PWM_VARIABLE_PERIOD,    /* Period and duty cycle can be changed */
    PWM_FIXED_PERIOD,       /* Only duty cycle can be changed */
    PWM_FIXED_PERIOD_SHIFTED /* Fixed period with phase shift */
} Pwm_ChannelClassType;

/**
 * @brief PWM Edge Notification Type
 * @details Type of edge that triggers notification
 */
typedef enum {
    PWM_RISING_EDGE = 0,    /* Notification on rising edge */
    PWM_FALLING_EDGE = 1,   /* Notification on falling edge */
    PWM_BOTH_EDGES = 2      /* Notification on both edges */
} Pwm_EdgeNotificationType;

/**
 * @brief PWM Polarity Type
 * @details Output polarity configuration
 */
typedef enum {
    PWM_LOW_POLARITY = 0,   /* Output starts LOW */
    PWM_HIGH_POLARITY = 1   /* Output starts HIGH */
} Pwm_PolarityType;

/**
 * @brief PWM Idle State Type
 * @details State of output when idle
 */
typedef enum {
    PWM_IDLE_LOW = 0,       /* Idle state is LOW */
    PWM_IDLE_HIGH = 1       /* Idle state is HIGH */
} Pwm_IdleStateType;

/**
 * @brief PWM Module Type
 * @details PWM module selection
 */
typedef enum {
    PWM_MODULE_0 = 0,       /* PWM Module 0 */
    PWM_MODULE_1 = 1        /* PWM Module 1 */
} Pwm_ModuleType;

/**
 * @brief PWM Generator Type
 * @details PWM generator within a module
 */
typedef enum {
    PWM_GEN_0 = 0,          /* Generator 0 */
    PWM_GEN_1 = 1,          /* Generator 1 */
    PWM_GEN_2 = 2,          /* Generator 2 */
    PWM_GEN_3 = 3           /* Generator 3 */
} Pwm_GeneratorType;

/**
 * @brief PWM Output Type
 * @details PWM output pin within a generator
 */
typedef enum {
    PWM_OUT_A = 0,          /* Output A */
    PWM_OUT_B = 1           /* Output B */
} Pwm_OutputType;

/**
 * @brief PWM Channel Configuration Type
 * @details Configuration structure for a single PWM channel
 */
typedef struct {
    Pwm_ChannelType Channel;              /* Channel ID */
    Pwm_ModuleType Module;                /* PWM module (0 or 1) */
    Pwm_GeneratorType Generator;          /* Generator number (0-3) */
    Pwm_OutputType Output;                /* Output pin (A or B) */
    Pwm_ChannelClassType ChannelClass;    /* Channel class */
    Pwm_PolarityType Polarity;            /* Output polarity */
    Pwm_IdleStateType IdleState;          /* Idle state */
    Pwm_PeriodType DefaultPeriod;         /* Default period in ticks */
    Pwm_DutyCycleType DefaultDutyCycle;   /* Default duty cycle (0x0000-0x8000) */
    void (*NotificationPtr)(void);        /* Notification callback (NULL if not used) */
} Pwm_ConfigChannelType;

/**
 * @brief PWM Configuration Type
 * @details Main configuration structure containing all PWM channels
 */
typedef struct {
    const Pwm_ConfigChannelType* Channels;  /* Pointer to channel configurations */
    uint8 NumChannels;                      /* Number of configured channels */
} Pwm_ConfigType;

/* ===================[Function Prototypes]=================== */

/**
 * @brief Initialize the PWM driver
 * @details Initializes all configured PWM channels with their default values
 *
 * @param[in] ConfigPtr - Pointer to PWM configuration structure
 *
 * @return void
 *
 * @pre ConfigPtr must not be NULL
 * @post PWM channels are initialized and ready to use
 */
void Pwm_Init(const Pwm_ConfigType* ConfigPtr);

#if (PWM_DE_INIT_API == TRUE)
/**
 * @brief De-initialize the PWM driver
 * @details Disables all PWM channels and resets hardware to default state
 *
 * @return void
 *
 * @pre Pwm_Init() must have been called
 * @post All PWM outputs are set to idle state
 */
void Pwm_DeInit(void);
#endif

#if (PWM_SET_DUTY_CYCLE_API == TRUE)
/**
 * @brief Set the duty cycle of a PWM channel
 * @details Sets the duty cycle while keeping the period unchanged
 *
 * @param[in] ChannelNumber - PWM channel ID
 * @param[in] DutyCycle - Duty cycle value (0x0000 = 0%, 0x8000 = 100%)
 *
 * @return void
 *
 * @pre Pwm_Init() must have been called
 * @post PWM output reflects new duty cycle
 */
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, Pwm_DutyCycleType DutyCycle);
#endif

#if (PWM_SET_PERIOD_AND_DUTY_API == TRUE)
/**
 * @brief Set the period and duty cycle of a PWM channel
 * @details Sets both period and duty cycle (only for VARIABLE_PERIOD channels)
 *
 * @param[in] ChannelNumber - PWM channel ID
 * @param[in] Period - Period value in ticks
 * @param[in] DutyCycle - Duty cycle value (0x0000 = 0%, 0x8000 = 100%)
 *
 * @return void
 *
 * @pre Pwm_Init() must have been called
 * @pre Channel must be VARIABLE_PERIOD type
 * @post PWM output reflects new period and duty cycle
 */
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber, Pwm_PeriodType Period, Pwm_DutyCycleType DutyCycle);
#endif

#if (PWM_SET_OUTPUT_TO_IDLE_API == TRUE)
/**
 * @brief Set PWM output to idle state
 * @details Immediately sets the output to its configured idle state
 *
 * @param[in] ChannelNumber - PWM channel ID
 *
 * @return void
 *
 * @pre Pwm_Init() must have been called
 * @post PWM output is in idle state
 */
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber);
#endif

#if (PWM_GET_OUTPUT_STATE_API == TRUE)
/**
 * @brief Get current PWM output state
 * @details Returns the current logical state of the PWM output
 *
 * @param[in] ChannelNumber - PWM channel ID
 *
 * @return Pwm_OutputStateType
 *         - PWM_LOW: Output is currently LOW
 *         - PWM_HIGH: Output is currently HIGH
 *
 * @pre Pwm_Init() must have been called
 */
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber);
#endif

#if (PWM_NOTIFICATION_SUPPORTED == TRUE)
/**
 * @brief Enable edge notification for a PWM channel
 * @details Enables interrupt-based notification on specified edge
 *
 * @param[in] ChannelNumber - PWM channel ID
 * @param[in] Notification - Edge type for notification
 *
 * @return void
 *
 * @pre Pwm_Init() must have been called
 * @pre Channel must have notification callback configured
 * @post Notifications are enabled for the channel
 */
void Pwm_EnableNotification(Pwm_ChannelType ChannelNumber, Pwm_EdgeNotificationType Notification);

/**
 * @brief Disable edge notification for a PWM channel
 * @details Disables interrupt-based notification
 *
 * @param[in] ChannelNumber - PWM channel ID
 *
 * @return void
 *
 * @pre Pwm_Init() must have been called
 * @post Notifications are disabled for the channel
 */
void Pwm_DisableNotification(Pwm_ChannelType ChannelNumber);
#endif

#if (PWM_VERSION_INFO_API == TRUE)
/**
 * @brief Get PWM driver version information
 * @details Returns version information of the PWM driver
 *
 * @param[out] VersionInfo - Pointer to store version information
 *
 * @return void
 *
 * @pre VersionInfo must not be NULL
 */
void Pwm_GetVersionInfo(Std_VersionInfoType* VersionInfo);
#endif

/* ===================[Helper Functions]=================== */

/**
 * @brief Set PWM duty cycle as percentage
 * @details Convenience function to set duty cycle using percentage (0-100)
 *
 * @param[in] ChannelNumber - PWM channel ID
 * @param[in] Percentage - Duty cycle percentage (0-100)
 *
 * @return void
 */
void Pwm_SetDutyCyclePercent(Pwm_ChannelType ChannelNumber, uint8 Percentage);

/**
 * @brief Set PWM frequency in Hz
 * @details Sets the PWM frequency for variable period channels
 *
 * @param[in] ChannelNumber - PWM channel ID
 * @param[in] FrequencyHz - Frequency in Hz
 *
 * @return void
 */
void Pwm_SetFrequency(Pwm_ChannelType ChannelNumber, uint16 FrequencyHz);

#endif /* MCAL_PWM_PWM_H_ */
