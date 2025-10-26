/*
 * Timer_Cfg.h
 * @file Timer_Cfg.h
 * @brief Timer Configuration Parameters for TM4C123GH6PM
 * @details This file contains compile-time configuration parameters for the Timer driver.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_TIMER_TIMER_CFG_H_
#define MCAL_TIMER_TIMER_CFG_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[Module Identification]=================== */
/**
 * @brief Timer module and vendor identification
 */
#define TIMER_VENDOR_ID                     (0x1234u)
#define TIMER_MODULE_ID                     (121u)
#define TIMER_INSTANCE_ID                   (0u)

/* ===================[Development Error Detection]=================== */
/**
 * @brief Enable/disable development error detection
 * @details When enabled, API functions perform parameter validation and report errors to DET
 * Values: STD_ON (enabled) / STD_OFF (disabled)
 */
#define TIMER_DEV_ERROR_DETECT              STD_ON

/* ===================[Version Info API]=================== */
/**
 * @brief Enable/disable version information API
 * @details When enabled, Timer_GetVersionInfo() API is available
 * Values: STD_ON (enabled) / STD_OFF (disabled)
 */
#define TIMER_VERSION_INFO_API              STD_ON

/* ===================[Version Information]=================== */
/**
 * @brief Timer driver version information
 */
#define TIMER_SW_MAJOR_VERSION              (1u)
#define TIMER_SW_MINOR_VERSION              (0u)
#define TIMER_SW_PATCH_VERSION              (0u)

/**
 * @brief AUTOSAR version information
 */
#define TIMER_AR_RELEASE_MAJOR_VERSION      (4u)
#define TIMER_AR_RELEASE_MINOR_VERSION      (4u)
#define TIMER_AR_RELEASE_PATCH_VERSION      (0u)

/* ===================[DET Error Codes]=================== */
/**
 * @brief Development Error Tracer error codes for Timer driver
 */
#define TIMER_E_PARAM_POINTER               (0x01u)  /**< NULL pointer passed to API */
#define TIMER_E_PARAM_CONFIG                (0x02u)  /**< Invalid configuration parameter */
#define TIMER_E_PARAM_MODULE                (0x03u)  /**< Invalid timer module */
#define TIMER_E_PARAM_BLOCK                 (0x04u)  /**< Invalid timer block */
#define TIMER_E_UNINIT                      (0x05u)  /**< API called before initialization */
#define TIMER_E_ALREADY_INITIALIZED         (0x06u)  /**< Timer already initialized */
#define TIMER_E_PARAM_VALUE                 (0x07u)  /**< Invalid parameter value */

/* ===================[API Service IDs]=================== */
/**
 * @brief Service IDs for Timer driver APIs (for DET reporting)
 */
#define TIMER_INIT_SID                      (0x00u)  /**< Timer_Init() */
#define TIMER_DEINIT_SID                    (0x01u)  /**< Timer_DeInit() */
#define TIMER_START_SID                     (0x02u)  /**< Timer_Start() */
#define TIMER_STOP_SID                      (0x03u)  /**< Timer_Stop() */
#define TIMER_GET_ELAPSED_TIME_SID          (0x04u)  /**< Timer_GetElapsedTime() */
#define TIMER_GET_REMAINING_TIME_SID        (0x05u)  /**< Timer_GetRemainingTime() */
#define TIMER_SET_LOAD_VALUE_SID            (0x06u)  /**< Timer_SetLoadValue() */
#define TIMER_GET_STATE_SID                 (0x07u)  /**< Timer_GetState() */
#define TIMER_GET_VERSION_INFO_SID          (0x08u)  /**< Timer_GetVersionInfo() */
#define TIMER_ENABLE_INTERRUPT_SID          (0x09u)  /**< Timer_EnableInterrupt() */
#define TIMER_DISABLE_INTERRUPT_SID         (0x0Au)  /**< Timer_DisableInterrupt() */
#define TIMER_CLEAR_INTERRUPT_SID           (0x0Bu)  /**< Timer_ClearInterrupt() */

/* ===================[System Clock Configuration]=================== */
/**
 * @brief System clock frequency (used for time calculations)
 * @details Default: 16 MHz (without PLL), can be 80 MHz with PLL enabled
 */
#define TIMER_SYSTEM_CLOCK_HZ               (16000000UL)  /* 16 MHz */

/* ===================[Maximum Timers]=================== */
/**
 * @brief Maximum number of timer configurations
 * @details Maximum timers that can be configured (6 modules × 2 blocks = 12)
 */
#define TIMER_MAX_CONFIGURATIONS            (12u)

#endif /* MCAL_TIMER_TIMER_CFG_H_ */
