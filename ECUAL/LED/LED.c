/*
 * @file Led.c
 * @brief LED Driver Implementation for TM4C123GH6PM
 * @details This file contains the implementation of the LED driver functions.
 *          The driver provides control over LEDs connected to GPIO pins with
 *          support for both active-high and active-low configurations.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

/* ===================[Includes]=================== */
#include "Led.h"
#include "Led_Cfg.h"
#include "../../CONFIG/Det.h"

/* ===================[Private Variables]=================== */
/* No private variables needed for LED driver */

/* ===================[Private Function Prototypes]=================== */
static Gpio_LevelType Led_ConvertStateToGpioLevel(const Led_ConfigType* Led, Led_StateType State);
static Led_StateType Led_ConvertGpioLevelToState(const Led_ConfigType* Led, Gpio_LevelType GpioLevel);

/* ===================[Private Function Implementations]=================== */

/**
 * @brief Convert LED state to GPIO level
 * @details This function converts LED state (ON/OFF) to GPIO level (HIGH/LOW)
 *          based on the LED's active-high/active-low configuration.
 *
 * @param[in] Led - Pointer to LED configuration structure
 * @param[in] State - LED state (LED_ON or LED_OFF)
 *
 * @return Gpio_LevelType
 *         - GPIO_LEVEL_HIGH: for active-high LEDs when ON
 *         - GPIO_LEVEL_LOW: for active-low LEDs when ON
 */
static Gpio_LevelType Led_ConvertStateToGpioLevel(const Led_ConfigType* Led, Led_StateType State)
{
    if (Led->ActiveHigh) {
        return (State == LED_ON) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW;
    } else {
        return (State == LED_ON) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH;
    }
}

/**
 * @brief Convert GPIO level to LED state
 * @details This function converts GPIO level (HIGH/LOW) to LED state (ON/OFF)
 *          based on the LED's active-high/active-low configuration.
 *
 * @param[in] Led - Pointer to LED configuration structure
 * @param[in] GpioLevel - GPIO level (GPIO_LEVEL_HIGH or GPIO_LEVEL_LOW)
 *
 * @return Led_StateType
 *         - LED_ON: LED is on
 *         - LED_OFF: LED is off
 */
static Led_StateType Led_ConvertGpioLevelToState(const Led_ConfigType* Led, Gpio_LevelType GpioLevel)
{
    if (Led->ActiveHigh) {
        return (GpioLevel == GPIO_LEVEL_HIGH) ? LED_ON : LED_OFF;
    } else {
        return (GpioLevel == GPIO_LEVEL_LOW) ? LED_ON : LED_OFF;
    }
}

/* ===================[Public Function Implementations]=================== */

void Led_Init(const Led_ConfigType* ConfigPtr)
{
    /* Parameter validation */
    if (ConfigPtr == NULL_PTR) {
        /* Report error if DET is enabled */
#if (LED_DEV_ERROR_DETECT == STD_ON)
        Det_ReportError(LED_MODULE_ID, 0u, LED_INIT_SID, LED_E_PARAM_POINTER);
#endif
        return;
    }

    /* Set initial state to OFF */
    Led_SetState(ConfigPtr, LED_OFF);
}

void Led_SetState(const Led_ConfigType* Led, Led_StateType State)
{
    /* Parameter validation */
    if (Led == NULL_PTR) {
        return;
    }

    /* Convert LED state to GPIO level */
    Gpio_LevelType gpioLevel = Led_ConvertStateToGpioLevel(Led, State);

    /* Write to GPIO channel */
    Gpio_WriteChannel(GPIO_CHANNEL(Led->Port, Led->Pin), gpioLevel);
}

void Led_Toggle(const Led_ConfigType* Led)
{
    /* Parameter validation */
    if (Led == NULL_PTR) {
        return;
    }

    /* Toggle GPIO channel */
    Gpio_FlipChannel(GPIO_CHANNEL(Led->Port, Led->Pin));
}

Led_StateType Led_GetState(const Led_ConfigType* Led)
{
    /* Parameter validation */
    if (Led == NULL_PTR) {
        return LED_OFF;
    }

    /* Read GPIO level */
    Gpio_LevelType gpioLevel = Gpio_ReadChannel(GPIO_CHANNEL(Led->Port, Led->Pin));

    /* Convert GPIO level to LED state */
    return Led_ConvertGpioLevelToState(Led, gpioLevel);
}

/* ===================[Version Information API]=================== */

#if (LED_VERSION_INFO_API == STD_ON)

/**
 * @brief Get LED driver version information
 * @details This function returns the version information of the LED driver.
 *
 * @param[in,out] VersionInfo - Pointer to version information structure
 *
 * @return void
 *
 * @pre VersionInfo must not be NULL
 * @post VersionInfo is filled with LED driver version information
 */
void Led_GetVersionInfo(Std_VersionInfoType* VersionInfo)
{
    /* Parameter validation */
    if (VersionInfo == NULL_PTR) {
        /* Report error if DET is enabled */
#if (LED_DEV_ERROR_DETECT == STD_ON)
        Det_ReportError(LED_MODULE_ID, 0u, LED_GET_VERSION_INFO_SID, LED_E_PARAM_POINTER);
#endif
        return;
    }

    /* Fill version information */
    VersionInfo->vendorID = LED_VENDOR_ID;
    VersionInfo->moduleID = LED_MODULE_ID;
    VersionInfo->sw_major_version = LED_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = LED_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = LED_SW_PATCH_VERSION;
}

#endif /* LED_VERSION_INFO_API == STD_ON */
