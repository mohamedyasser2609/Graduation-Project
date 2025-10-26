/*
 * @file Button_PBCfg.c
 * @brief Button Driver Post-Build Configuration for TM4C123GH6PM
 * @details This file contains example configurations for buttons and switches
 *          connected to the TM4C123GH6PM LaunchPad. These configurations can
 *          be modified based on the specific hardware setup.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

/* ===================[Includes]=================== */
#include "../Button/Button.h"

/* ===================[Button Configurations]=================== */

/**
 * @brief Configuration for SW1 button (PF4)
 * @details SW1 on TM4C123GH6PM LaunchPad is connected to PF4 with pull-up resistor.
 *          Button is active-low (pressed = LOW, released = HIGH).
 */
const Button_ConfigType Button_SW1 = {
    .Port = GPIO_PORT_F,
    .Pin = GPIO_PIN_4,
    .ActiveHigh = FALSE,     /* Active-low (pressed = LOW) */
    .PullUp = TRUE,          /* Internal pull-up resistor */
    .DebounceMs = 20u        /* 20ms debounce time */
};

/**
 * @brief Configuration for SW2 button (PF0)
 * @details SW2 on TM4C123GH6PM LaunchPad is connected to PF0 with pull-up resistor.
 *          Button is active-low (pressed = LOW, released = HIGH).
 *          Note: PF0 requires unlock sequence due to NMI function.
 */
const Button_ConfigType Button_SW2 = {
    .Port = GPIO_PORT_F,
    .Pin = GPIO_PIN_0,
    .ActiveHigh = FALSE,     /* Active-low (pressed = LOW) */
    .PullUp = TRUE,          /* Internal pull-up resistor */
    .DebounceMs = 20u        /* 20ms debounce time */
};

/**
 * @brief Configuration for external button on PA0
 * @details Example configuration for an external push button connected to PA0.
 *          Assumes external pull-up resistor and active-low operation.
 */
const Button_ConfigType Button_External_PA0 = {
    .Port = GPIO_PORT_A,
    .Pin = GPIO_PIN_0,
    .ActiveHigh = FALSE,     /* Active-low (pressed = LOW) */
    .PullUp = FALSE,         /* External pull-up resistor */
    .DebounceMs = 15u        /* 15ms debounce time */
};

/**
 * @brief Configuration for external button on PB1 (Active-High)
 * @details Example configuration for an external push button connected to PB1.
 *          Assumes external pull-down resistor and active-high operation.
 */
const Button_ConfigType Button_External_PB1 = {
    .Port = GPIO_PORT_B,
    .Pin = GPIO_PIN_1,
    .ActiveHigh = TRUE,      /* Active-high (pressed = HIGH) */
    .PullUp = FALSE,         /* External pull-down resistor */
    .DebounceMs = 25u        /* 25ms debounce time */
};

/**
 * @brief Array of all button configurations
 * @details This array contains all button configurations used in the system.
 *          Add or remove configurations as needed for your application.
 */
const Button_ConfigType* Button_Configs[] = {
    &Button_SW1,
    &Button_SW2,
    &Button_External_PA0,
    &Button_External_PB1
};

/**
 * @brief Number of button configurations
 * @details Total number of button configurations in the system
 */
const uint8 Button_ConfigCount = sizeof(Button_Configs) / sizeof(Button_ConfigType*);
