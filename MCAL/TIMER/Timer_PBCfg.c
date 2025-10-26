/*
 * @file Timer_PBCfg.c
 * @brief Timer Post-Build Configuration for TM4C123GH6PM
 * @details Example timer configurations for common use cases.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

/* ===================[Includes]=================== */
#include "Timer.h"
#include "Timer_Cfg.h"

/* ===================[Example Configurations]=================== */

/**
 * @brief Example 1: Periodic Timer - 1 second @ 16MHz
 * @details Timer0A configured for 1-second periodic interrupts
 */
const Timer_ConfigType Timer_Config_1Second = {
    .Module = TIMER_MODULE_0,
    .Block = TIMER_BLOCK_A,
    .ConfigMode = TIMER_CONFIG_32BIT,
    .OperationMode = TIMER_MODE_PERIODIC,
    .CountDirection = TIMER_COUNT_DOWN,
    .PwmMode = TIMER_PWM_DISABLED,
    .LoadValue = 16000000u,              /* 1 second @ 16 MHz */
    .MatchValue = 0u,                    /* Not used in periodic mode */
    .Prescaler = 0u,                     /* No prescaler */
    .InterruptEnable = TRUE,             /* Enable timeout interrupt */
    .MatchInterruptEnable = FALSE,
    .TimeoutCallback = NULL_PTR,         /* User provides callback */
    .MatchCallback = NULL_PTR
};

/**
 * @brief Example 2: One-Shot Timer - 100ms @ 16MHz
 * @details Timer1A configured for 100ms one-shot operation
 */
const Timer_ConfigType Timer_Config_100ms_OneShot = {
    .Module = TIMER_MODULE_1,
    .Block = TIMER_BLOCK_A,
    .ConfigMode = TIMER_CONFIG_32BIT,
    .OperationMode = TIMER_MODE_ONESHOT,
    .CountDirection = TIMER_COUNT_DOWN,
    .PwmMode = TIMER_PWM_DISABLED,
    .LoadValue = 1600000u,               /* 100ms @ 16 MHz */
    .MatchValue = 0u,
    .Prescaler = 0u,
    .InterruptEnable = TRUE,
    .MatchInterruptEnable = FALSE,
    .TimeoutCallback = NULL_PTR,
    .MatchCallback = NULL_PTR
};

/**
 * @brief Example 3: PWM Timer - 1kHz, 50% duty cycle @ 16MHz
 * @details Timer2A configured for PWM generation
 */
const Timer_ConfigType Timer_Config_1kHz_PWM = {
    .Module = TIMER_MODULE_2,
    .Block = TIMER_BLOCK_A,
    .ConfigMode = TIMER_CONFIG_32BIT,
    .OperationMode = TIMER_MODE_PERIODIC,
    .CountDirection = TIMER_COUNT_DOWN,
    .PwmMode = TIMER_PWM_ENABLED,
    .LoadValue = 16000u,                 /* 1 kHz @ 16 MHz */
    .MatchValue = 8000u,                 /* 50% duty cycle */
    .Prescaler = 0u,
    .InterruptEnable = FALSE,            /* PWM doesn't need interrupt */
    .MatchInterruptEnable = FALSE,
    .TimeoutCallback = NULL_PTR,
    .MatchCallback = NULL_PTR
};

/**
 * @brief Example 4: Fast Periodic Timer - 1ms @ 16MHz
 * @details Timer3A configured for 1ms tick (for system tick, RTOS, etc.)
 */
const Timer_ConfigType Timer_Config_1ms_Tick = {
    .Module = TIMER_MODULE_3,
    .Block = TIMER_BLOCK_A,
    .ConfigMode = TIMER_CONFIG_32BIT,
    .OperationMode = TIMER_MODE_PERIODIC,
    .CountDirection = TIMER_COUNT_DOWN,
    .PwmMode = TIMER_PWM_DISABLED,
    .LoadValue = 16000u,                 /* 1ms @ 16 MHz */
    .MatchValue = 0u,
    .Prescaler = 0u,
    .InterruptEnable = TRUE,
    .MatchInterruptEnable = FALSE,
    .TimeoutCallback = NULL_PTR,         /* User provides system tick handler */
    .MatchCallback = NULL_PTR
};

/**
 * @brief Example 5: 16-bit Timer B - 10ms @ 16MHz
 * @details Timer0B configured as 16-bit for shorter intervals
 */
const Timer_ConfigType Timer_Config_10ms_16bit = {
    .Module = TIMER_MODULE_0,
    .Block = TIMER_BLOCK_B,
    .ConfigMode = TIMER_CONFIG_16BIT,
    .OperationMode = TIMER_MODE_PERIODIC,
    .CountDirection = TIMER_COUNT_DOWN,
    .PwmMode = TIMER_PWM_DISABLED,
    .LoadValue = 160000u,                /* 10ms @ 16 MHz (fits in 16-bit) */
    .MatchValue = 0u,
    .Prescaler = 0u,
    .InterruptEnable = TRUE,
    .MatchInterruptEnable = FALSE,
    .TimeoutCallback = NULL_PTR,
    .MatchCallback = NULL_PTR
};

/**
 * @brief Example 6: Extended Range Timer with Prescaler - 10 seconds @ 16MHz
 * @details Timer4A with prescaler for very long periods
 */
const Timer_ConfigType Timer_Config_10Second_Prescaled = {
    .Module = TIMER_MODULE_4,
    .Block = TIMER_BLOCK_A,
    .ConfigMode = TIMER_CONFIG_16BIT,
    .OperationMode = TIMER_MODE_PERIODIC,
    .CountDirection = TIMER_COUNT_DOWN,
    .PwmMode = TIMER_PWM_DISABLED,
    .LoadValue = 62500u,                 /* Base value */
    .MatchValue = 0u,
    .Prescaler = 255u,                   /* Prescaler extends range */
    .InterruptEnable = TRUE,
    .MatchInterruptEnable = FALSE,
    .TimeoutCallback = NULL_PTR,
    .MatchCallback = NULL_PTR
};

/* ===================[Usage Notes]=================== */
/*
 * To use these configurations:
 *
 * 1. Initialize the timer:
 *    Timer_Init(&Timer_Config_1Second);
 *
 * 2. Start the timer:
 *    Timer_Start(TIMER_MODULE_0, TIMER_BLOCK_A);
 *
 * 3. Handle interrupt (if enabled):
 *    void Timer0A_Handler(void) {
 *        Timer_ClearInterrupt(TIMER_MODULE_0, TIMER_BLOCK_A, TIMER_INT_TIMEOUT);
 *        // Your code here
 *    }
 *
 * 4. Stop the timer when done:
 *    Timer_Stop(TIMER_MODULE_0, TIMER_BLOCK_A);
 */
