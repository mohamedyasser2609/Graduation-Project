/*
 * @file Timer.h
 * @brief Timer Driver API for TM4C123GH6PM
 * @details This file contains the public API for the General Purpose Timer Module (GPTM) driver.
 *          Supports one-shot, periodic, PWM, and capture modes with interrupt support.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_TIMER_TIMER_H_
#define MCAL_TIMER_TIMER_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"
#include "../../CONFIG/Det.h"
#include "Timer_Types.h"
#include "Timer_Cfg.h"

/* ===================[API Function Declarations]=================== */

/**
 * @brief Initialize Timer driver
 * @details Initializes the specified timer with the provided configuration.
 *          This function must be called before any other timer APIs.
 *
 * @param[in] ConfigPtr Pointer to timer configuration structure
 *
 * @pre Timer module must be disabled
 * @post Timer is configured and ready to start
 *
 * @error TIMER_E_PARAM_POINTER: ConfigPtr is NULL
 * @error TIMER_E_PARAM_CONFIG: Invalid configuration parameters
 * @error TIMER_E_ALREADY_INITIALIZED: Timer already initialized
 *
 * @example
 * @code
 * const Timer_ConfigType timerConfig = {
 *     .Module = TIMER_MODULE_0,
 *     .Block = TIMER_BLOCK_A,
 *     .ConfigMode = TIMER_CONFIG_32BIT,
 *     .OperationMode = TIMER_MODE_PERIODIC,
 *     .LoadValue = 16000000,  // 1 second @ 16MHz
 *     .InterruptEnable = TRUE
 * };
 * Timer_Init(&timerConfig);
 * @endcode
 */
void Timer_Init(const Timer_ConfigType* ConfigPtr);

/**
 * @brief De-initialize Timer driver
 * @details Disables the specified timer and clears its configuration.
 *
 * @param[in] Module Timer module (TIMER_MODULE_0 to TIMER_MODULE_5)
 * @param[in] Block Timer block (TIMER_BLOCK_A or TIMER_BLOCK_B)
 *
 * @pre Timer must be initialized
 * @post Timer is disabled and de-configured
 *
 * @error TIMER_E_PARAM_MODULE: Invalid timer module
 * @error TIMER_E_PARAM_BLOCK: Invalid timer block
 * @error TIMER_E_UNINIT: Timer not initialized
 */
void Timer_DeInit(Timer_ModuleType Module, Timer_BlockType Block);

/**
 * @brief Start Timer
 * @details Starts the specified timer. Timer will begin counting based on its configuration.
 *
 * @param[in] Module Timer module (TIMER_MODULE_0 to TIMER_MODULE_5)
 * @param[in] Block Timer block (TIMER_BLOCK_A or TIMER_BLOCK_B)
 *
 * @pre Timer must be initialized
 * @post Timer is running
 *
 * @error TIMER_E_PARAM_MODULE: Invalid timer module
 * @error TIMER_E_PARAM_BLOCK: Invalid timer block
 * @error TIMER_E_UNINIT: Timer not initialized
 */
void Timer_Start(Timer_ModuleType Module, Timer_BlockType Block);

/**
 * @brief Stop Timer
 * @details Stops the specified timer. Timer value is retained.
 *
 * @param[in] Module Timer module (TIMER_MODULE_0 to TIMER_MODULE_5)
 * @param[in] Block Timer block (TIMER_BLOCK_A or TIMER_BLOCK_B)
 *
 * @pre Timer must be initialized
 * @post Timer is stopped
 *
 * @error TIMER_E_PARAM_MODULE: Invalid timer module
 * @error TIMER_E_PARAM_BLOCK: Invalid timer block
 * @error TIMER_E_UNINIT: Timer not initialized
 */
void Timer_Stop(Timer_ModuleType Module, Timer_BlockType Block);

/**
 * @brief Get elapsed time
 * @details Returns the elapsed time since timer started (in ticks).
 *
 * @param[in] Module Timer module (TIMER_MODULE_0 to TIMER_MODULE_5)
 * @param[in] Block Timer block (TIMER_BLOCK_A or TIMER_BLOCK_B)
 *
 * @return Timer_ValueType Elapsed time in ticks (0 if error)
 *
 * @pre Timer must be initialized and running
 *
 * @error TIMER_E_PARAM_MODULE: Invalid timer module
 * @error TIMER_E_PARAM_BLOCK: Invalid timer block
 * @error TIMER_E_UNINIT: Timer not initialized
 */
Timer_ValueType Timer_GetElapsedTime(Timer_ModuleType Module, Timer_BlockType Block);

/**
 * @brief Get remaining time
 * @details Returns the remaining time until timeout (in ticks).
 *
 * @param[in] Module Timer module (TIMER_MODULE_0 to TIMER_MODULE_5)
 * @param[in] Block Timer block (TIMER_BLOCK_A or TIMER_BLOCK_B)
 *
 * @return Timer_ValueType Remaining time in ticks (0 if error)
 *
 * @pre Timer must be initialized and running
 *
 * @error TIMER_E_PARAM_MODULE: Invalid timer module
 * @error TIMER_E_PARAM_BLOCK: Invalid timer block
 * @error TIMER_E_UNINIT: Timer not initialized
 */
Timer_ValueType Timer_GetRemainingTime(Timer_ModuleType Module, Timer_BlockType Block);

/**
 * @brief Set timer load value
 * @details Updates the timer load value (interval). Takes effect on next reload.
 *
 * @param[in] Module Timer module (TIMER_MODULE_0 to TIMER_MODULE_5)
 * @param[in] Block Timer block (TIMER_BLOCK_A or TIMER_BLOCK_B)
 * @param[in] Value New load value
 *
 * @pre Timer must be initialized
 * @post Load value is updated
 *
 * @error TIMER_E_PARAM_MODULE: Invalid timer module
 * @error TIMER_E_PARAM_BLOCK: Invalid timer block
 * @error TIMER_E_PARAM_VALUE: Invalid load value
 * @error TIMER_E_UNINIT: Timer not initialized
 */
void Timer_SetLoadValue(Timer_ModuleType Module, Timer_BlockType Block, Timer_ValueType Value);

/**
 * @brief Get timer state
 * @details Returns whether the timer is running or stopped.
 *
 * @param[in] Module Timer module (TIMER_MODULE_0 to TIMER_MODULE_5)
 * @param[in] Block Timer block (TIMER_BLOCK_A or TIMER_BLOCK_B)
 *
 * @return Timer_StateType TIMER_STATE_RUNNING or TIMER_STATE_STOPPED
 *
 * @pre Timer must be initialized
 *
 * @error TIMER_E_PARAM_MODULE: Invalid timer module
 * @error TIMER_E_PARAM_BLOCK: Invalid timer block
 * @error TIMER_E_UNINIT: Timer not initialized
 */
Timer_StateType Timer_GetState(Timer_ModuleType Module, Timer_BlockType Block);

/**
 * @brief Enable timer interrupt
 * @details Enables the specified interrupt type for the timer.
 *
 * @param[in] Module Timer module (TIMER_MODULE_0 to TIMER_MODULE_5)
 * @param[in] Block Timer block (TIMER_BLOCK_A or TIMER_BLOCK_B)
 * @param[in] IntType Interrupt type to enable
 *
 * @pre Timer must be initialized
 * @post Interrupt is enabled
 *
 * @error TIMER_E_PARAM_MODULE: Invalid timer module
 * @error TIMER_E_PARAM_BLOCK: Invalid timer block
 * @error TIMER_E_UNINIT: Timer not initialized
 */
void Timer_EnableInterrupt(Timer_ModuleType Module, Timer_BlockType Block, Timer_InterruptType IntType);

/**
 * @brief Disable timer interrupt
 * @details Disables the specified interrupt type for the timer.
 *
 * @param[in] Module Timer module (TIMER_MODULE_0 to TIMER_MODULE_5)
 * @param[in] Block Timer block (TIMER_BLOCK_A or TIMER_BLOCK_B)
 * @param[in] IntType Interrupt type to disable
 *
 * @pre Timer must be initialized
 * @post Interrupt is disabled
 *
 * @error TIMER_E_PARAM_MODULE: Invalid timer module
 * @error TIMER_E_PARAM_BLOCK: Invalid timer block
 * @error TIMER_E_UNINIT: Timer not initialized
 */
void Timer_DisableInterrupt(Timer_ModuleType Module, Timer_BlockType Block, Timer_InterruptType IntType);

/**
 * @brief Clear timer interrupt
 * @details Clears the specified interrupt flag for the timer.
 *
 * @param[in] Module Timer module (TIMER_MODULE_0 to TIMER_MODULE_5)
 * @param[in] Block Timer block (TIMER_BLOCK_A or TIMER_BLOCK_B)
 * @param[in] IntType Interrupt type to clear
 *
 * @pre Timer must be initialized
 * @post Interrupt flag is cleared
 *
 * @error TIMER_E_PARAM_MODULE: Invalid timer module
 * @error TIMER_E_PARAM_BLOCK: Invalid timer block
 * @error TIMER_E_UNINIT: Timer not initialized
 */
void Timer_ClearInterrupt(Timer_ModuleType Module, Timer_BlockType Block, Timer_InterruptType IntType);

#if (TIMER_VERSION_INFO_API == STD_ON)
/**
 * @brief Get Timer driver version information
 * @details Returns version information about the Timer driver.
 *
 * @param[out] VersionInfo Pointer to store version information
 *
 * @pre None
 * @post Version information is stored in VersionInfo
 *
 * @error TIMER_E_PARAM_POINTER: VersionInfo is NULL
 */
void Timer_GetVersionInfo(Std_VersionInfoType* VersionInfo);
#endif

#endif /* MCAL_TIMER_TIMER_H_ */
