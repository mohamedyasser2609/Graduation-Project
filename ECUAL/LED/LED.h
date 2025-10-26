/*
 * @file Led.h
 * @brief LED Driver for TM4C123GH6PM
 * @details This file contains the LED driver implementation for controlling
 *          individual LEDs connected to GPIO pins. The driver supports both
 *          active-high and active-low LED configurations.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

#ifndef ECUAL_LED_LED_H_
#define ECUAL_LED_LED_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/std_types.h"
#include "../../MCAL/GPIO/Gpio.h"

/* ===================[Version Information]=================== */
#define LED_VENDOR_ID                    (0x1234u)
#define LED_MODULE_ID                    (200u)
#define LED_SW_MAJOR_VERSION             (1u)
#define LED_SW_MINOR_VERSION             (0u)
#define LED_SW_PATCH_VERSION             (0u)

/* ===================[LED State Definitions]=================== */
/**
 * @brief LED state enumeration
 * @details Defines the possible states for an LED
 */
typedef enum {
    LED_OFF = 0u,    /**< LED is turned off */
    LED_ON = 1u      /**< LED is turned on */
} Led_StateType;

/* ===================[LED Configuration Structure]=================== */
/**
 * @brief LED configuration structure
 * @details Configuration parameters for an LED connected to a GPIO pin
 */
typedef struct {
    Gpio_PortType   Port;        /**< GPIO port (GPIO_PORT_A to GPIO_PORT_F) */
    Gpio_PinType    Pin;         /**< GPIO pin (GPIO_PIN_0 to GPIO_PIN_7) */
    boolean         ActiveHigh;  /**< TRUE for active-high, FALSE for active-low */
} Led_ConfigType;

/* ===================[API Function Prototypes]=================== */

/**
 * @brief Initialize LED
 * @details This function initializes the LED by setting its initial state to OFF.
 *          The GPIO pin direction is assumed to be configured by the GPIO driver.
 *
 * @param[in] ConfigPtr - Pointer to LED configuration structure
 *
 * @return void
 *
 * @pre GPIO module must be initialized
 * @pre LED pin must be configured as output in GPIO driver
 * @post LED is initialized and set to OFF state
 *
 * @note This function only sets the initial state. GPIO pin configuration
 *       (direction, mode, etc.) must be done separately using GPIO driver.
 */
void Led_Init(const Led_ConfigType* ConfigPtr);

/**
 * @brief Set LED state
 * @details This function sets the LED to the specified state (ON or OFF).
 *          It handles both active-high and active-low LED configurations.
 *
 * @param[in] Led - Pointer to LED configuration structure
 * @param[in] State - Desired state (LED_ON or LED_OFF)
 *
 * @return void
 *
 * @pre LED must be initialized
 * @post LED state is set to the specified value
 */
void Led_SetState(const Led_ConfigType* Led, Led_StateType State);

/**
 * @brief Toggle LED state
 * @details This function toggles the current state of the LED.
 *          If LED is ON, it turns OFF. If LED is OFF, it turns ON.
 *
 * @param[in] Led - Pointer to LED configuration structure
 *
 * @return void
 *
 * @pre LED must be initialized
 * @post LED state is inverted
 */
void Led_Toggle(const Led_ConfigType* Led);

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
void Led_GetVersionInfo(Std_VersionInfoType* VersionInfo);

#endif /* ECUAL_LED_LED_H_ */
