/*
 * @file Button.h
 * @brief Button Driver for TM4C123GH6PM
 * @details This file contains the Button driver implementation for reading
 *          push buttons and switches connected to GPIO pins. The driver supports
 *          both active-high and active-low button configurations with debouncing.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

#ifndef ECUAL_BUTTON_BUTTON_H_
#define ECUAL_BUTTON_BUTTON_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/std_types.h"
#include "../../MCAL/GPIO/Gpio.h"

/* ===================[Version Information]=================== */
#define BUTTON_VENDOR_ID                    (0x1234u)
#define BUTTON_MODULE_ID                    (201u)
#define BUTTON_SW_MAJOR_VERSION             (1u)
#define BUTTON_SW_MINOR_VERSION             (0u)
#define BUTTON_SW_PATCH_VERSION             (0u)

/* ===================[Button State Definitions]=================== */
/**
 * @brief Button state enumeration
 * @details Defines the possible states for a button
 */
typedef enum {
    BUTTON_RELEASED = 0u,    /**< Button is not pressed */
    BUTTON_PRESSED = 1u      /**< Button is pressed */
} Button_StateType;

/* ===================[Button Configuration Structure]=================== */
/**
 * @brief Button configuration structure
 * @details Configuration parameters for a button connected to a GPIO pin
 */
typedef struct {
    Gpio_PortType   Port;        /**< GPIO port (GPIO_PORT_A to GPIO_PORT_F) */
    Gpio_PinType    Pin;         /**< GPIO pin (GPIO_PIN_0 to GPIO_PIN_7) */
    boolean         ActiveHigh;  /**< TRUE for active-high, FALSE for active-low */
    boolean         PullUp;      /**< TRUE if internal pull-up is used */
    uint16          DebounceMs;  /**< Debounce time in milliseconds */
} Button_ConfigType;

/* ===================[API Function Prototypes]=================== */

/**
 * @brief Initialize Button
 * @details This function initializes the button by configuring the GPIO pin
 *          as input with the specified pull resistor configuration.
 *
 * @param[in] ConfigPtr - Pointer to button configuration structure
 *
 * @return void
 *
 * @pre GPIO module must be initialized
 * @post Button is initialized and ready for use
 *
 * @note This function configures the GPIO pin as input. The GPIO pin configuration
 *       (mode, pull resistors, etc.) is handled automatically.
 */
void Button_Init(const Button_ConfigType* ConfigPtr);

/**
 * @brief Read Button state
 * @details This function reads the current state of the button from the GPIO pin.
 *          It handles both active-high and active-low button configurations.
 *
 * @param[in] Button - Pointer to button configuration structure
 *
 * @return Button_StateType
 *         - BUTTON_PRESSED: Button is currently pressed
 *         - BUTTON_RELEASED: Button is currently released
 *
 * @pre Button must be initialized
 * @post None
 */
Button_StateType Button_ReadState(const Button_ConfigType* Button);

/**
 * @brief Read Button state with debouncing
 * @details This function reads the button state with software debouncing to
 *          eliminate contact bounce effects.
 *
 * @param[in] Button - Pointer to button configuration structure
 *
 * @return Button_StateType
 *         - BUTTON_PRESSED: Button is pressed (debounced)
 *         - BUTTON_RELEASED: Button is released (debounced)
 *
 * @pre Button must be initialized
 * @post None
 *
 * @note Debouncing is implemented using simple delay. For better performance,
 *       consider using timer-based debouncing in a real-time system.
 */
Button_StateType Button_ReadStateDebounced(const Button_ConfigType* Button);

/**
 * @brief Check if button state has changed
 * @details This function checks if the button state has changed since the last call.
 *          Useful for detecting button press and release events.
 *
 * @param[in] Button - Pointer to button configuration structure
 * @param[out] CurrentState - Current button state (can be NULL if not needed)
 *
 * @return boolean
 *         - TRUE: Button state has changed since last call
 *         - FALSE: Button state is the same as last call
 *
 * @pre Button must be initialized
 * @post CurrentState is updated with current button state
 */
boolean Button_HasStateChanged(const Button_ConfigType* Button, Button_StateType* CurrentState);

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
void Button_GetVersionInfo(Std_VersionInfoType* VersionInfo);

#endif /* ECUAL_BUTTON_BUTTON_H_ */
