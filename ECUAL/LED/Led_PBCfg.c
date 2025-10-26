/*
 * @file Led_PBCfg.c
 * @brief LED Driver Post-Build Configuration for TM4C123GH6PM
 * @details This file contains example configurations for LEDs connected to
 *          the TM4C123GH6PM LaunchPad. These configurations can be modified
 *          based on the specific hardware setup.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

/* ===================[Includes]=================== */
#include "../LED/Led.h"

/* ===================[LED Configurations]=================== */

/**
 * @brief Configuration for Red LED (PF1)
 * @details Red LED on TM4C123GH6PM LaunchPad is connected to PF1.
 *          LED is active-high (ON = HIGH, OFF = LOW).
 */
const Led_ConfigType Led_Red = {
    .Port = GPIO_PORT_F,
    .Pin = GPIO_PIN_1,
    .ActiveHigh = TRUE       /* Active-high (ON = HIGH) */
};

/**
 * @brief Configuration for Blue LED (PF2)
 * @details Blue LED on TM4C123GH6PM LaunchPad is connected to PF2.
 *          LED is active-high (ON = HIGH, OFF = LOW).
 */
const Led_ConfigType Led_Blue = {
    .Port = GPIO_PORT_F,
    .Pin = GPIO_PIN_2,
    .ActiveHigh = TRUE       /* Active-high (ON = HIGH) */
};

/**
 * @brief Configuration for Green LED (PF3)
 * @details Green LED on TM4C123GH6PM LaunchPad is connected to PF3.
 *          LED is active-high (ON = HIGH, OFF = LOW).
 */
const Led_ConfigType Led_Green = {
    .Port = GPIO_PORT_F,
    .Pin = GPIO_PIN_3,
    .ActiveHigh = TRUE       /* Active-high (ON = HIGH) */
};

/**
 * @brief Configuration for external LED on PA0 (Active-Low)
 * @details Example configuration for an external LED connected to PA0.
 *          LED is active-low (ON = LOW, OFF = HIGH).
 */
const Led_ConfigType Led_External_PA0 = {
    .Port = GPIO_PORT_A,
    .Pin = GPIO_PIN_0,
    .ActiveHigh = FALSE      /* Active-low (ON = LOW) */
};

/**
 * @brief Configuration for external LED on PB0 (Active-High)
 * @details Example configuration for an external LED connected to PB0.
 *          LED is active-high (ON = HIGH, OFF = LOW).
 */
const Led_ConfigType Led_External_PB0 = {
    .Port = GPIO_PORT_B,
    .Pin = GPIO_PIN_0,
    .ActiveHigh = TRUE       /* Active-high (ON = HIGH) */
};

/**
 * @brief Array of all LED configurations
 * @details This array contains all LED configurations used in the system.
 *          Add or remove configurations as needed for your application.
 */
const Led_ConfigType* Led_Configs[] = {
    &Led_Red,
    &Led_Blue,
    &Led_Green,
    &Led_External_PA0,
    &Led_External_PB0
};

/**
 * @brief Number of LED configurations
 * @details Total number of LED configurations in the system
 */
const uint8 Led_ConfigCount = sizeof(Led_Configs) / sizeof(Led_ConfigType*);
