/**
 * @file main_fan_debug.c
 * @brief Simple GPIO Fan Test (No PWM)
 * @details Toggles PB6/PB7 as standard GPIO outputs to verify electrical control.
 *          Low (0V) should stop the fan. High (3.3V) should run full speed.
 */

#if 0 /* Set to 1 to enable this debug file */

#include <stdint.h>
#include <stdbool.h>

#define SYSCTL_RCGCUART_R       (*((volatile uint32_t *)0x400FE618))
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define GPIO_PORTA_AFSEL_R      (*((volatile uint32_t *)0x40004420))
#define GPIO_PORTA_PCTL_R       (*((volatile uint32_t *)0x4000452C))
#define GPIO_PORTA_DEN_R        (*((volatile uint32_t *)0x4000451C))

#define GPIO_PORTB_DATA_R       (*((volatile uint32_t *)0x400053FC))
#define GPIO_PORTB_DIR_R        (*((volatile uint32_t *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile uint32_t *)0x40005420))
#define GPIO_PORTB_PUR_R        (*((volatile uint32_t *)0x40005510))
#define GPIO_PORTB_DEN_R        (*((volatile uint32_t *)0x4000551C))
#define GPIO_PORTB_PCTL_R       (*((volatile uint32_t *)0x4000552C))

#define UART0_DR_R              (*((volatile uint32_t *)0x4000C000))
#define UART0_FR_R              (*((volatile uint32_t *)0x4000C018))
#define UART0_IBRD_R            (*((volatile uint32_t *)0x4000C024))
#define UART0_FBRD_R            (*((volatile uint32_t *)0x4000C028))
#define UART0_LCRH_R            (*((volatile uint32_t *)0x4000C02C))
#define UART0_CTL_R             (*((volatile uint32_t *)0x4000C030))

/* Helper Functions */
void SimpleDelay(uint32_t count)
{
    volatile uint32_t i;
    for (i = 0; i < count; i++);
}

void UART0_Init(void)
{
    SYSCTL_RCGCUART_R |= 0x01;
    SYSCTL_RCGCGPIO_R |= 0x01;
    SimpleDelay(100);
    GPIO_PORTA_AFSEL_R |= 0x03;
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFFFFFF00) | 0x00000011;
    GPIO_PORTA_DEN_R |= 0x03;
    UART0_CTL_R &= ~0x01;
    
    /* Baud Rate Calculation for 16 MHz Clock, 115200 Baud */
    /* BRD = 16,000,000 / (16 * 115200) = 8.680555 */
    /* IBRD = 8 */
    /* FBRD = 0.680555 * 64 = 43.555 -> 44 */
    UART0_IBRD_R = 8;
    UART0_FBRD_R = 44;
    
    UART0_LCRH_R = 0x70;
    UART0_CTL_R |= 0x301;
}

void UART0_SendString(const char* str)
{
    while(*str)
    {
        while((UART0_FR_R & 0x20) != 0);
        UART0_DR_R = *str++;
    }
}

/* Dummy handler */
void Timer2A_Handler(void) { while(1); }

/* GPIO Register for Open Drain */
#define GPIO_PORTB_ODR_R        (*((volatile uint32_t *)0x4000550C))

int main(void)
{
    /* Use default 16MHz clock to keep it simple */
    UART0_Init();
    
    UART0_SendString("\r\n========================================\r\n");
    UART0_SendString("[DEBUG] GPIO Fan Hardware Test v2\r\n");
    UART0_SendString("[FIX] UART Fixed + Open Drain Enabled\r\n");
    UART0_SendString("PB6/PB7 toggling Low/High every 5s\r\n");
    
    /* 1. Enable Port B GPIO */
    SYSCTL_RCGCGPIO_R |= 0x02;
    SimpleDelay(100);
    
    /* 2. Configure PB6 & PB7 as GPIO Output with Open Drain */
    GPIO_PORTB_AFSEL_R &= ~0xC0; /* Disable Alt Func */
    GPIO_PORTB_PCTL_R &= ~0xFF000000; /* PCTL=0 (GPIO) */
    GPIO_PORTB_DIR_R |= 0xC0;    /* Output */
    GPIO_PORTB_ODR_R |= 0xC0;    /* Enable Open Drain (Simulates Switch to GND) */
    GPIO_PORTB_DEN_R |= 0xC0;    /* Digital Enable */
    
    /* Note: In Open Drain:
       - Output 0: Drives Low (GND) -> Fan should see 0%
       - Output 1: Floating (Hi-Z)  -> Fan pulls up -> Fan should see 100%
    */
    
    while(1)
    {
        /* State 1: LOW (Active Drive to GND) */
        UART0_SendString("GPIO: LOW (Open Drain Active) -> Expect Fans STOP\r\n");
        GPIO_PORTB_DATA_R &= ~0xC0;
        
        int k;
        for(k=0; k<5; k++) { SimpleDelay(1600000); } /* ~5s delay */
        
        /* State 2: HIGH (Hi-Z / Floating) */
        UART0_SendString("GPIO: HIGH (Open Drain Floating) -> Expect Fans MAX\r\n");
        GPIO_PORTB_DATA_R |= 0xC0;
        
        for(k=0; k<5; k++) { SimpleDelay(1600000); } /* ~5s delay */
    }
}
#endif
