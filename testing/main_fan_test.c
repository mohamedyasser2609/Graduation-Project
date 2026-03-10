/**
 * @file main_fan_test.c
 * @brief PWM Fan Driver Test Application
 * @details Verification test for intake and exhaust cooling fans
 *
 * Test Features:
 * - Initialization of PWM and FAN driver
 * - Cylcing through speeds: 0% -> 50% -> 100%
 * - Manual verification via observation
 *
 * Hardware Connections:
 * - Fan 0 (Intake): PWM Module 1, Generator 0 (matches FAN_PBCfg)
 * - Fan 1 (Exhaust): PWM Module 1, Generator 1
 * 
 * @author Mohamed Yasser
 * @date Feb 06, 2026
 * @version 1.0.0
 */

#if 0 /* Set to 1 to enable this main file */

#include "MCAL/MCU/Mcu.h"
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/PWM/Pwm.h"
#include "MCAL/UART/Uart.h"
#include "ECUAL/FAN/Fan.h"
#include "CONFIG/Std_Types.h"

/* ===================[External Configuration]=================== */
extern const Mcu_ConfigType* Mcu_ConfigPtr;
extern const Gpio_ConfigType Gpio_Configuration;
extern const Pwm_ConfigType Pwm_Configuration;
extern const Uart_ConfigType Uart0_Config_115200;
extern const Fan_ConfigType Fan_Config;

/* ===================[Helper Functions]=================== */

void SimpleDelay(uint32 count)
{
    volatile uint32 i;
    for (i = 0; i < count; i++);
}

static void PrintString(const char* str)
{
    Uart_SendString(UART_MODULE_0, (const uint8*)str);
}

/* Dummy handler to satisfy linker */
void Timer2A_Handler(void)
{
    while(1);
}

/* ===================[Main Function]=================== */

int main(void)
{
    /* 1. Initialize System */
    Mcu_Init(Mcu_ConfigPtr);
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    Mcu_DistributePllClock();
    
    Gpio_Init(&Gpio_Configuration);
    
    Uart_Init(&Uart0_Config_115200);
    SimpleDelay(100000);
    PrintString("\r\n========================================\r\n");
    PrintString("[TEST] PWM Fan Test Application Start\r\n");
    PrintString("========================================\r\n");
    
    /* 2. Init PWM and Fans */
    PrintString("Initializing PWM Driver...\r\n");
    Pwm_Init(&Pwm_Configuration);
    
    PrintString("Initializing Fan Driver...\r\n");
    Fan_Init(&Fan_Config);
    
    /* 3. Test Loop */
    while (1)
    {
        /* --- State 1: OFF (0%) --- */
        PrintString("Fans: OFF (0%)\r\n");
        Fan_SetSpeed(0, 0); /* Fan 0 */
        Fan_SetSpeed(1, 0); /* Fan 1 */
        SimpleDelay(40000000); /* 5s delay */
        
        /* --- State 2: MEDIUM (50%) --- */
        PrintString("Fans: MEDIUM (50%)\r\n");
        Fan_SetSpeed(0, 50);
        Fan_SetSpeed(1, 50);
        SimpleDelay(40000000); /* 5s delay */
        
        /* --- State 3: MAX (100%) --- */
        PrintString("Fans: MAX (100%)\r\n");
        Fan_SetSpeed(0, 100);
        Fan_SetSpeed(1, 100);
        SimpleDelay(40000000); /* 5s delay */
    }
}

#endif
