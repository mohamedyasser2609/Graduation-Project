/*
 * @file Gpio_PBCfg.c
 * @brief GPIO Post-Build Configuration for TM4C123GH6PM
 * @details This file contains example post-build configuration for GPIO driver.
 *          Users should modify this file according to their application needs.
 *
 * @author Mohamed Yasser
 * @date Oct 19, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

/* ===================[Includes]=================== */
#include "Gpio.h"
#include "Gpio_Cfg.h"

/* ===================[Example Pin Configurations]=================== */
/**
 * @brief Example GPIO pin configurations
 * @details This array contains example configurations for commonly used pins:
 *          - PF1 (Red LED) - Output, initially LOW
 *          - PF2 (Blue LED) - Output, initially LOW
 *          - PF3 (Green LED) - Output, initially LOW
 *          - PF4 (SW1) - Input with pull-up
 *          - PF0 (SW2) - Input with pull-up (requires unlock)
 *          - PA0-PA1 (UART0) - Alternate function
 *          - PB6-PB7 (I2C0) - Alternate function with open-drain
 */
static const Gpio_PinConfigType Gpio_PinConfigurations[] = {
    /* Switch Pins - Port F */
    {
        .Port = GPIO_PORT_F,
        .Pin = GPIO_PIN_4,
        .Direction = GPIO_PIN_IN,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_DIO,
        .InternalResistor = GPIO_RESISTOR_PULL_UP,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_FALLING_EDGE,
        .AlternateFuncNum = 0u,
        .DirectionChangeable = FALSE
    },
    {
        .Port = GPIO_PORT_F,
        .Pin = GPIO_PIN_0,
        .Direction = GPIO_PIN_IN,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_DIO,
        .InternalResistor = GPIO_RESISTOR_PULL_UP,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_FALLING_EDGE,
        .AlternateFuncNum = 0u,
        .DirectionChangeable = FALSE
    },
    
    /* UART1 Pins for GPS Module - Port B (PB0=RX, PB1=TX) */
    {
        .Port = GPIO_PORT_B,
        .Pin = GPIO_PIN_0,
        .Direction = GPIO_PIN_IN,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_OFF,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 1u,  /* UART1 RX - GPS TX */
        .DirectionChangeable = FALSE
    },
    {
        .Port = GPIO_PORT_B,
        .Pin = GPIO_PIN_1,
        .Direction = GPIO_PIN_OUT,
        .InitialLevel = GPIO_LEVEL_HIGH,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_OFF,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 1u,  /* UART1 TX - GPS RX */
        .DirectionChangeable = FALSE
    },
    
    /* PWM Pins for RGB LEDs - Port F (PF1=Red, PF2=Blue, PF3=Green) */
    {
        .Port = GPIO_PORT_F,
        .Pin = GPIO_PIN_1,
        .Direction = GPIO_PIN_OUT,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_OFF,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 5u,  /* M1PWM5 - Red LED PWM */
        .DirectionChangeable = FALSE
    },
    {
        .Port = GPIO_PORT_F,
        .Pin = GPIO_PIN_2,
        .Direction = GPIO_PIN_OUT,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_OFF,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 5u,  /* M1PWM6 - Blue LED PWM */
        .DirectionChangeable = FALSE
    },
    {
        .Port = GPIO_PORT_F,
        .Pin = GPIO_PIN_3,
        .Direction = GPIO_PIN_OUT,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_OFF,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 5u,  /* M1PWM7 - Green LED PWM */
        .DirectionChangeable = FALSE
    },
    
    /* UART0 Pins - Port A (PA0=RX, PA1=TX) */
    {
        .Port = GPIO_PORT_A,
        .Pin = GPIO_PIN_0,
        .Direction = GPIO_PIN_IN,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_OFF,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 1u,  /* UART0 RX */
        .DirectionChangeable = FALSE
    },
    {
        .Port = GPIO_PORT_A,
        .Pin = GPIO_PIN_1,
        .Direction = GPIO_PIN_OUT,
        .InitialLevel = GPIO_LEVEL_HIGH,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_OFF,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 1u,  /* UART0 TX */
        .DirectionChangeable = FALSE
    },
    
    /* I2C0 Pins - Port B (PB2=SCL, PB3=SDA) */
    /* Note: I2C uses AF3. Special open-drain handling is in Gpio.c */
    {
        .Port = GPIO_PORT_B,
        .Pin = GPIO_PIN_2,
        .Direction = GPIO_PIN_OUT,
        .InitialLevel = GPIO_LEVEL_HIGH,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_OPEN_DRAIN,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 3u,  /* I2C0 SCL */
        .DirectionChangeable = FALSE
    },
    {
        .Port = GPIO_PORT_B,
        .Pin = GPIO_PIN_3,
        .Direction = GPIO_PIN_OUT,
        .InitialLevel = GPIO_LEVEL_HIGH,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_OPEN_DRAIN,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 3u,  /* I2C0 SDA */
        .DirectionChangeable = FALSE
    },

    /* Motor PWM Pins - Port A (PA6=M1PWM2 Left Motor, PA7=M1PWM3 Right Motor) */
    /* Note: M1PWM2 and M1PWM3 use Alternate Function 5 on TM4C123 */
    {
        .Port = GPIO_PORT_A,
        .Pin = GPIO_PIN_6,
        .Direction = GPIO_PIN_OUT,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_OFF,
        .DriveStrength = GPIO_DRIVE_8MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 5u,  /* M1PWM2 - Left Motor PWM */
        .DirectionChangeable = FALSE
    },
    {
        .Port = GPIO_PORT_A,
        .Pin = GPIO_PIN_7,
        .Direction = GPIO_PIN_OUT,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_OFF,
        .DriveStrength = GPIO_DRIVE_8MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 5u,  /* M1PWM3 - Right Motor PWM */
        .DirectionChangeable = FALSE
    },

    /* (PB6/PB7 QEI0 mapping removed - using PD6/PD7) */
    
    /* QEI0 Pins - Port D (PD6=PhA, PD7=PhB, PD3=Index) */
    {
        .Port = GPIO_PORT_D,
        .Pin = GPIO_PIN_6,
        .Direction = GPIO_PIN_IN,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_PULL_UP,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 6u,  /* QEI0 PhA */
        .DirectionChangeable = FALSE
    },
    {
        .Port = GPIO_PORT_D,
        .Pin = GPIO_PIN_7,
        .Direction = GPIO_PIN_IN,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_PULL_UP,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 6u,  /* QEI0 PhB */
        .DirectionChangeable = FALSE
    },
    {
        .Port = GPIO_PORT_D,
        .Pin = GPIO_PIN_3,
        .Direction = GPIO_PIN_IN,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_PULL_UP,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 6u,  /* QEI0 Index */
        .DirectionChangeable = FALSE
    },

    /* QEI1 Pins - Port C (PC5=PhA1, PC6=PhB1, PC4=Index1) */
    {
        .Port = GPIO_PORT_C,
        .Pin = GPIO_PIN_5,
        .Direction = GPIO_PIN_IN,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_PULL_UP,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 6u,  /* QEI1 PhA */
        .DirectionChangeable = FALSE
    },
    {
        .Port = GPIO_PORT_C,
        .Pin = GPIO_PIN_6,
        .Direction = GPIO_PIN_IN,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_PULL_UP,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 6u,  /* QEI1 PhB */
        .DirectionChangeable = FALSE
    },
    {
        .Port = GPIO_PORT_C,
        .Pin = GPIO_PIN_4,
        .Direction = GPIO_PIN_IN,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_ALT_FUNC,
        .InternalResistor = GPIO_RESISTOR_PULL_UP,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 6u,  /* QEI1 Index */
        .DirectionChangeable = FALSE
    },
    
    /* General Purpose Digital I/O - Port E */
    {
        .Port = GPIO_PORT_E,
        .Pin = GPIO_PIN_0,
        .Direction = GPIO_PIN_OUT,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_DIO,
        .InternalResistor = GPIO_RESISTOR_OFF,
        .DriveStrength = GPIO_DRIVE_4MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 0u,
        .DirectionChangeable = TRUE
    },

        {
                .Port = GPIO_PORT_E,
                .Pin = GPIO_PIN_1,  /* Left Motor Direction */
                .Direction = GPIO_PIN_OUT,
                .InitialLevel = GPIO_LEVEL_LOW,
                .Mode = GPIO_MODE_DIO,
                .InternalResistor = GPIO_RESISTOR_OFF,
                .DriveStrength = GPIO_DRIVE_4MA,
                .SlewRate = GPIO_SLEW_RATE_NORMAL,
                .IntTrigger = GPIO_INT_DISABLED,
                .AlternateFuncNum = 0u,
                .DirectionChangeable = FALSE
            },
            {
                .Port = GPIO_PORT_E,
                .Pin = GPIO_PIN_4,  /* Right Motor Direction (Moved from PE2 to avoid AIN1 conflict) */
                .Direction = GPIO_PIN_OUT,
                .InitialLevel = GPIO_LEVEL_LOW,
                .Mode = GPIO_MODE_DIO,
                .InternalResistor = GPIO_RESISTOR_OFF,
                .DriveStrength = GPIO_DRIVE_4MA,
                .SlewRate = GPIO_SLEW_RATE_NORMAL,
                .IntTrigger = GPIO_INT_DISABLED,
                .AlternateFuncNum = 0u,
                .DirectionChangeable = FALSE
            },
            /* Fan PWM Pins - Port B (PB6=M0PWM0 Fan 1, PB7=M0PWM1 Fan 2) */
            {
                .Port = GPIO_PORT_B,
                .Pin = GPIO_PIN_6,
                .Direction = GPIO_PIN_OUT,
                .InitialLevel = GPIO_LEVEL_LOW,
                .Mode = GPIO_MODE_ALT_FUNC,
                .InternalResistor = GPIO_RESISTOR_OFF,
                .DriveStrength = GPIO_DRIVE_8MA,
                .SlewRate = GPIO_SLEW_RATE_NORMAL,
                .IntTrigger = GPIO_INT_DISABLED,
                .AlternateFuncNum = 4u,  /* M0PWM0 - Fan 1 PWM */
                .DirectionChangeable = FALSE
            },
            {
                .Port = GPIO_PORT_B,
                .Pin = GPIO_PIN_7,
                .Direction = GPIO_PIN_OUT,
                .InitialLevel = GPIO_LEVEL_LOW,
                .Mode = GPIO_MODE_ALT_FUNC,
                .InternalResistor = GPIO_RESISTOR_OFF,
                .DriveStrength = GPIO_DRIVE_8MA,
                .SlewRate = GPIO_SLEW_RATE_NORMAL,
                .IntTrigger = GPIO_INT_DISABLED,
                .AlternateFuncNum = 4u,  /* M0PWM1 - Fan 2 PWM */
                .DirectionChangeable = FALSE
            }
};

/* ===================[GPIO Configuration Structure]=================== */
/**
 * @brief Main GPIO configuration structure
 * @details This structure is referenced by the driver during initialization
 */
const Gpio_ConfigType Gpio_Configuration = {
    .PinConfigs = Gpio_PinConfigurations,
    .NumberOfPins = sizeof(Gpio_PinConfigurations) / sizeof(Gpio_PinConfigType)
};
