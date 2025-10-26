/*
 * main.c
 * Simple LED and Button Test Program for TM4C123GH6PM LaunchPad
 *
 * This program tests the GPIO, LED, and Button drivers by cycling through
 * 7 different LED color combinations each time a button is pressed.
 *
 * Hardware Setup:
 * - Red LED: PF1 (active-high)
 * - Blue LED: PF2 (active-high)
 * - Green LED: PF3 (active-high)
 * - SW1 Button: PF4 (active-low, pull-up)
 * - SW2 Button: PF0 (active-low, pull-up)
 *
 * Created: Oct 26, 2025
 * Author: Mohamed Yasser
 */

#include "../../MCAL/GPIO/Gpio.h"
#include "../../ECUAL/LED/Led.h"
#include "../../ECUAL/Button/Button.h"

/* Delay function (simple busy wait) */
void delay_ms(uint32 ms) {
    volatile uint32 count = ms * 40000;  /* Approximate delay for 80MHz */
    while(count--) {
        /* Do nothing */
    }
}

/* LED Color Combinations (7 colors) */
typedef enum {
    COLOR_OFF = 0,
    COLOR_RED,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_YELLOW,    /* Red + Green */
    COLOR_MAGENTA,   /* Red + Blue */
    COLOR_CYAN,      /* Blue + Green */
    COLOR_WHITE      /* Red + Blue + Green */
} LedColor_Type;

int main(void) {
    /* Current color state */
    static LedColor_Type currentColor = COLOR_OFF;

    /* Step 1: Initialize GPIO driver first */
    Gpio_Init(&Gpio_Configuration);

    /* Step 2: Initialize LED driver */
    /* LED Configurations */
    const Led_ConfigType Led_Red = {
        .Port = GPIO_PORT_F,
        .Pin = GPIO_PIN_1,
        .ActiveHigh = TRUE
    };

    const Led_ConfigType Led_Blue = {
        .Port = GPIO_PORT_F,
        .Pin = GPIO_PIN_2,
        .ActiveHigh = TRUE
    };

    const Led_ConfigType Led_Green = {
        .Port = GPIO_PORT_F,
        .Pin = GPIO_PIN_3,
        .ActiveHigh = TRUE
    };

    /* Initialize LEDs */
    Led_Init(&Led_Red);
    Led_Init(&Led_Blue);
    Led_Init(&Led_Green);

    /* Step 3: Initialize Button driver */
    /* Button Configurations */
    const Button_ConfigType Button_SW1 = {
        .Port = GPIO_PORT_F,
        .Pin = GPIO_PIN_4,
        .ActiveHigh = FALSE,  /* Active-low */
        .PullUp = TRUE,       /* Internal pull-up */
        .DebounceMs = 20u
    };

    const Button_ConfigType Button_SW2 = {
        .Port = GPIO_PORT_F,
        .Pin = GPIO_PIN_0,
        .ActiveHigh = FALSE,  /* Active-low */
        .PullUp = TRUE,       /* Internal pull-up */
        .DebounceMs = 20u
    };

    /* Initialize Buttons */
    Button_Init(&Button_SW1);
    Button_Init(&Button_SW2);

    /* Step 4: Main loop - cycle through colors when button pressed */
    while(1) {
        /* Check if either button is pressed */
        Button_StateType sw1State = Button_ReadStateDebounced(&Button_SW1);
        Button_StateType sw2State = Button_ReadStateDebounced(&Button_SW2);

        /* If any button is pressed, cycle to next color */
        if (sw1State == BUTTON_PRESSED || sw2State == BUTTON_PRESSED) {
            /* Move to next color */
            currentColor = (LedColor_Type)((currentColor + 1) % 8);

            /* Set LEDs based on current color */
            switch(currentColor) {
                case COLOR_OFF:
                    /* All LEDs OFF */
                    Led_SetState(&Led_Red, LED_OFF);
                    Led_SetState(&Led_Blue, LED_OFF);
                    Led_SetState(&Led_Green, LED_OFF);
                    break;

                case COLOR_RED:
                    /* Red only */
                    Led_SetState(&Led_Red, LED_ON);
                    Led_SetState(&Led_Blue, LED_OFF);
                    Led_SetState(&Led_Green, LED_OFF);
                    break;

                case COLOR_BLUE:
                    /* Blue only */
                    Led_SetState(&Led_Red, LED_OFF);
                    Led_SetState(&Led_Blue, LED_ON);
                    Led_SetState(&Led_Green, LED_OFF);
                    break;

                case COLOR_GREEN:
                    /* Green only */
                    Led_SetState(&Led_Red, LED_OFF);
                    Led_SetState(&Led_Blue, LED_OFF);
                    Led_SetState(&Led_Green, LED_ON);
                    break;

                case COLOR_YELLOW:
                    /* Red + Green = Yellow */
                    Led_SetState(&Led_Red, LED_ON);
                    Led_SetState(&Led_Blue, LED_OFF);
                    Led_SetState(&Led_Green, LED_ON);
                    break;

                case COLOR_MAGENTA:
                    /* Red + Blue = Magenta */
                    Led_SetState(&Led_Red, LED_ON);
                    Led_SetState(&Led_Blue, LED_ON);
                    Led_SetState(&Led_Green, LED_OFF);
                    break;

                case COLOR_CYAN:
                    /* Blue + Green = Cyan */
                    Led_SetState(&Led_Red, LED_OFF);
                    Led_SetState(&Led_Blue, LED_ON);
                    Led_SetState(&Led_Green, LED_ON);
                    break;

                case COLOR_WHITE:
                    /* All LEDs = White */
                    Led_SetState(&Led_Red, LED_ON);
                    Led_SetState(&Led_Blue, LED_ON);
                    Led_SetState(&Led_Green, LED_ON);
                    break;

                default:
                    /* Should never reach here */
                    currentColor = COLOR_OFF;
                    break;
            }

            /* Wait a bit to avoid multiple triggers from one press */
            delay_ms(50);
        }

        /* Small delay to prevent busy waiting */
        delay_ms(10);
    }


}
