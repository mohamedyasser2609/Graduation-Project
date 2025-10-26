/*
 * @file Button.c
 * @brief Button Driver Implementation for TM4C123GH6PM
 * @details This file contains the implementation of the Button driver functions.
 *          The driver provides reading of push buttons and switches connected to
 *          GPIO pins with support for debouncing and state change detection.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

/* ===================[Includes]=================== */
#include "Button.h"
#include "Button_Cfg.h"
#include "../../CONFIG/Det.h"

/* ===================[Private Variables]=================== */
/**
 * @brief Previous button states for change detection
 * @details Stores the previous state of each button for change detection
 */
static Button_StateType Button_PreviousStates[48] = {BUTTON_RELEASED};  /* 6 ports * 8 pins = 48 max */

/* ===================[Private Function Prototypes]=================== */
static Button_StateType Button_ConvertGpioLevelToState(const Button_ConfigType* Button, Gpio_LevelType GpioLevel);
static uint8 Button_GetChannelIndex(const Button_ConfigType* Button);
static void Button_DebounceDelay(uint16 DelayMs);

/* ===================[Private Function Implementations]=================== */

/**
 * @brief Convert GPIO level to button state
 * @details This function converts GPIO level (HIGH/LOW) to button state (PRESSED/RELEASED)
 *          based on the button's active-high/active-low configuration.
 *
 * @param[in] Button - Pointer to button configuration structure
 * @param[in] GpioLevel - GPIO level (GPIO_LEVEL_HIGH or GPIO_LEVEL_LOW)
 *
 * @return Button_StateType
 *         - BUTTON_PRESSED: Button is pressed
 *         - BUTTON_RELEASED: Button is released
 */
static Button_StateType Button_ConvertGpioLevelToState(const Button_ConfigType* Button, Gpio_LevelType GpioLevel)
{
    if (Button->ActiveHigh) {
        return (GpioLevel == GPIO_LEVEL_HIGH) ? BUTTON_PRESSED : BUTTON_RELEASED;
    } else {
        return (GpioLevel == GPIO_LEVEL_LOW) ? BUTTON_PRESSED : BUTTON_RELEASED;
    }
}

/**
 * @brief Get channel index for state tracking
 * @details This function converts port and pin to a unique channel index
 *          for tracking button states in the state array.
 *
 * @param[in] Button - Pointer to button configuration structure
 *
 * @return uint8 - Channel index (0-47 for 6 ports * 8 pins)
 */
static uint8 Button_GetChannelIndex(const Button_ConfigType* Button)
{
    return (uint8)((Button->Port * 8u) + Button->Pin);
}

/**
 * @brief Simple debounce delay
 * @details This function provides a simple delay-based debounce mechanism.
 *          In a real-time system, consider using timer-based debouncing.
 *
 * @param[in] DelayMs - Delay time in milliseconds
 */
static void Button_DebounceDelay(uint16 DelayMs)
{
    volatile uint32 delay = DelayMs * 1000u;  /* Approximate delay */
    while(delay--) {
        /* Busy wait - in real system use timer */
    }
}

/* ===================[Public Function Implementations]=================== */

void Button_Init(const Button_ConfigType* ConfigPtr)
{
    /* Parameter validation */
    if (ConfigPtr == NULL_PTR) {
#if (BUTTON_DEV_ERROR_DETECT == STD_ON)
        Det_ReportError(BUTTON_MODULE_ID, 0u, BUTTON_INIT_SID, BUTTON_E_PARAM_POINTER);
#endif
        return;
    }

    /* Validate configuration */
    if (ConfigPtr->Port > GPIO_PORT_F || ConfigPtr->Pin > GPIO_PIN_7) {
#if (BUTTON_DEV_ERROR_DETECT == STD_ON)
        Det_ReportError(BUTTON_MODULE_ID, 0u, BUTTON_INIT_SID, BUTTON_E_INVALID_CONFIG);
#endif
        return;
    }

    /* Configure GPIO pin as input */
    Gpio_SetPinDirection(GPIO_CHANNEL(ConfigPtr->Port, ConfigPtr->Pin), GPIO_PIN_IN);

    /* Configure internal resistor if needed */
    if (ConfigPtr->PullUp) {
        /* Use pull-up resistor */
        /* Note: This would require additional GPIO configuration */
        /* For now, assume external pull-up is used */
    }

    /* Initialize previous state */
    uint8 channelIndex = Button_GetChannelIndex(ConfigPtr);
    Button_PreviousStates[channelIndex] = Button_ReadState(ConfigPtr);
}

Button_StateType Button_ReadState(const Button_ConfigType* Button)
{
    /* Parameter validation */
    if (Button == NULL_PTR) {
        return BUTTON_RELEASED;
    }

    /* Read GPIO level */
    Gpio_LevelType gpioLevel = Gpio_ReadChannel(GPIO_CHANNEL(Button->Port, Button->Pin));

    /* Convert GPIO level to button state */
    return Button_ConvertGpioLevelToState(Button, gpioLevel);
}

Button_StateType Button_ReadStateDebounced(const Button_ConfigType* Button)
{
    /* Parameter validation */
    if (Button == NULL_PTR) {
        return BUTTON_RELEASED;
    }

    /* Read initial state */
    Button_StateType initialState = Button_ReadState(Button);
    uint16 debounceTime = (Button->DebounceMs > 0u) ? Button->DebounceMs : BUTTON_DEFAULT_DEBOUNCE_MS;

    /* Wait for debounce time */
    Button_DebounceDelay(debounceTime);

    /* Read state again after debounce */
    Button_StateType finalState = Button_ReadState(Button);

    /* Return state only if both readings are the same */
    return (initialState == finalState) ? initialState : Button_PreviousStates[Button_GetChannelIndex(Button)];
}

boolean Button_HasStateChanged(const Button_ConfigType* Button, Button_StateType* CurrentState)
{
    /* Parameter validation */
    if (Button == NULL_PTR) {
        if (CurrentState != NULL_PTR) {
            *CurrentState = BUTTON_RELEASED;
        }
        return FALSE;
    }

    /* Get current state */
    Button_StateType currentState = Button_ReadState(Button);

    /* Get channel index */
    uint8 channelIndex = Button_GetChannelIndex(Button);

    /* Check if state has changed */
    boolean hasChanged = (currentState != Button_PreviousStates[channelIndex]);

    /* Update previous state */
    Button_PreviousStates[channelIndex] = currentState;

    /* Return current state if requested */
    if (CurrentState != NULL_PTR) {
        *CurrentState = currentState;
    }

    return hasChanged;
}

/* ===================[Version Information API]=================== */

#if (BUTTON_VERSION_INFO_API == STD_ON)

/**
 * @brief Get Button driver version information
 * @details This function returns the version information of the Button driver.
 *
 * @param[in,out] VersionInfo - Pointer to version information structure
 *
 * @return void
 *
 * @pre VersionInfo must not be NULL
 * @post VersionInfo is filled with Button driver version information
 */
void Button_GetVersionInfo(Std_VersionInfoType* VersionInfo)
{
    /* Parameter validation */
    if (VersionInfo == NULL_PTR) {
#if (BUTTON_DEV_ERROR_DETECT == STD_ON)
        Det_ReportError(BUTTON_MODULE_ID, 0u, BUTTON_GET_VERSION_INFO_SID, BUTTON_E_PARAM_POINTER);
#endif
        return;
    }

    /* Fill version information */
    VersionInfo->vendorID = BUTTON_VENDOR_ID;
    VersionInfo->moduleID = BUTTON_MODULE_ID;
    VersionInfo->sw_major_version = BUTTON_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = BUTTON_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = BUTTON_SW_PATCH_VERSION;
}

#endif /* BUTTON_VERSION_INFO_API == STD_ON */
