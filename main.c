/*
 * @file main.c
 * @brief Timer Driver Test Program for TM4C123GH6PM LaunchPad
 * @details This program demonstrates the Timer driver functionality by:
 *          - Timer2A: Cycles through 8 LED colors every 2 seconds
 *          - Color Sequence: OFF → RED → GREEN → BLUE → YELLOW → MAGENTA → CYAN → WHITE
 *
 * Hardware Setup:
 * - Red LED: PF1 (active-high)
 * - Blue LED: PF2 (active-high)
 * - Green LED: PF3 (active-high)
 *
 * Timer Configuration:
 * - System Clock: 16 MHz (default)
 * - Timer2A: 2 seconds periodic (32,000,000 ticks)
 * - Interrupt-driven color cycling
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

#include "../../MCAL/GPIO/Gpio.h"
#include "../../MCAL/TIMER/Timer.h"
#include "../../ECUAL/LED/Led.h"

/* ===================[NVIC Register for Interrupt Enable]=================== */
#define NVIC_EN0_R              (*((volatile uint32*)0xE000E100UL))

/* ===================[External LED Configurations]=================== */
/* LED Configurations from Led_PBCfg.c (defined externally) */
extern const Led_ConfigType Led_Red;
extern const Led_ConfigType Led_Blue;
extern const Led_ConfigType Led_Green;

/* ===================[LED Color Definitions]=================== */
typedef enum {
    COLOR_OFF = 0,      /* All LEDs OFF */
    COLOR_RED,          /* Red only */
    COLOR_GREEN,        /* Green only */
    COLOR_BLUE,         /* Blue only */
    COLOR_YELLOW,       /* Red + Green */
    COLOR_MAGENTA,      /* Red + Blue */
    COLOR_CYAN,         /* Green + Blue */
    COLOR_WHITE         /* All LEDs ON */
} LedColor_Type;

/* Current color for Timer2 (color cycling) */
static volatile LedColor_Type currentColor = COLOR_OFF;

/* ===================[Helper Functions]=================== */
/**
 * @brief Set LED color combination
 * @param color Color to display
 */
void SetLedColor(LedColor_Type color) {
    switch(color) {
        case COLOR_OFF:
            Led_SetState(&Led_Red, LED_OFF);
            Led_SetState(&Led_Green, LED_OFF);
            Led_SetState(&Led_Blue, LED_OFF);
            break;
        case COLOR_RED:
            Led_SetState(&Led_Red, LED_ON);
            Led_SetState(&Led_Green, LED_OFF);
            Led_SetState(&Led_Blue, LED_OFF);
            break;
        case COLOR_GREEN:
            Led_SetState(&Led_Red, LED_OFF);
            Led_SetState(&Led_Green, LED_ON);
            Led_SetState(&Led_Blue, LED_OFF);
            break;
        case COLOR_BLUE:
            Led_SetState(&Led_Red, LED_OFF);
            Led_SetState(&Led_Green, LED_OFF);
            Led_SetState(&Led_Blue, LED_ON);
            break;
        case COLOR_YELLOW:
            Led_SetState(&Led_Red, LED_ON);
            Led_SetState(&Led_Green, LED_ON);
            Led_SetState(&Led_Blue, LED_OFF);
            break;
        case COLOR_MAGENTA:
            Led_SetState(&Led_Red, LED_ON);
            Led_SetState(&Led_Green, LED_OFF);
            Led_SetState(&Led_Blue, LED_ON);
            break;
        case COLOR_CYAN:
            Led_SetState(&Led_Red, LED_OFF);
            Led_SetState(&Led_Green, LED_ON);
            Led_SetState(&Led_Blue, LED_ON);
            break;
        case COLOR_WHITE:
            Led_SetState(&Led_Red, LED_ON);
            Led_SetState(&Led_Green, LED_ON);
            Led_SetState(&Led_Blue, LED_ON);
            break;
        default:
            break;
    }
}

/* ===================[Timer Interrupt Handlers]=================== */
/**
 * @brief Timer2A ISR - Cycles through LED colors every 2 seconds
 */
void Timer2A_Handler(void) {
    /* Clear interrupt flag */
    Timer_ClearInterrupt(TIMER_MODULE_2, TIMER_BLOCK_A, TIMER_INT_TIMEOUT);
    
    /* Cycle to next color */
    currentColor = (LedColor_Type)((currentColor + 1) % 8);
    
    /* Set new color */
    SetLedColor(currentColor);
}

/* ===================[Main Function]=================== */
int main(void) {
    /* Step 1: Initialize GPIO driver */
    Gpio_Init(&Gpio_Configuration);

    /* Step 2: Initialize LED driver */
    Led_Init(&Led_Red);
    Led_Init(&Led_Blue);
    Led_Init(&Led_Green);

    /* Turn off all LEDs initially */
    Led_SetState(&Led_Red, LED_OFF);
    Led_SetState(&Led_Blue, LED_OFF);
    Led_SetState(&Led_Green, LED_OFF);

    /* Step 3: Configure Timer2A - 2 seconds periodic (Color cycling) */
    const Timer_ConfigType Timer2A_Config = {
        .Module = TIMER_MODULE_2,
        .Block = TIMER_BLOCK_A,
        .ConfigMode = TIMER_CONFIG_32BIT,
        .OperationMode = TIMER_MODE_PERIODIC,
        .CountDirection = TIMER_COUNT_DOWN,
        .PwmMode = TIMER_PWM_DISABLED,
        .LoadValue = 32000000u,             /* 2 seconds @ 16MHz */
        .MatchValue = 0u,
        .Prescaler = 0u,
        .InterruptEnable = TRUE,
        .MatchInterruptEnable = FALSE,
        .TimeoutCallback = NULL_PTR,
        .MatchCallback = NULL_PTR
    };

    /* Step 4: Initialize Timer */
    Timer_Init(&Timer2A_Config);

    /* Step 5: Enable Timer2A interrupt in NVIC (IRQ 23) */
    NVIC_EN0_R |= (1 << 23);

    /* Step 6: Start Timer */
    Timer_Start(TIMER_MODULE_2, TIMER_BLOCK_A);

    /* Step 7: Main loop - timer runs in background via interrupts */
    while(1) {
        /* 
         * All LED control is handled by Timer2A interrupt
         * 
         * Observed behavior:
         * - LEDs cycle through 8 colors every 2 seconds:
         *   OFF → RED → GREEN → BLUE → YELLOW → MAGENTA → CYAN → WHITE → (repeat)
         * - Clean color transitions without interference
         */
        
        /* Optional: Add low-power mode or other background tasks here */
        __asm("    WFI");  /* Wait For Interrupt - low power mode */
    }

    /* In embedded systems, main() should never return */
}
