/**
 * @file Mcu_Test.c
 * @brief MCU Driver Test Application for TM4C123GH6PM
 * @details Test application to demonstrate AUTOSAR-compliant MCU driver functionality
 *
 * @author Mohamed Yasser
 * @date Nov 1, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#include "MCU.h"
#include "Mcu_Cfg.h"
#include "../../CONFIG/Det.h"
#include <stdio.h>

/* ===================[Test Functions]=================== */

/**
 * @brief Test MCU initialization
 */
static void Test_Mcu_Init(void)
{
    printf("Testing MCU Initialization...\n");
    
    /* Initialize DET first */
    Det_Init();
    
    /* Initialize MCU with default configuration */
    Mcu_Init(&Mcu_Config);
    
    printf("MCU Initialization completed.\n");
}

/**
 * @brief Test clock initialization
 */
static void Test_Mcu_InitClock(void)
{
    printf("Testing MCU Clock Initialization...\n");
    
    /* Test with different clock settings */
    Std_ReturnType result;
    
    result = Mcu_InitClock(MCU_CLOCK_MOSC_16MHZ);
    printf("MOSC 16MHz initialization: %s\n", (result == E_OK) ? "PASS" : "FAIL");
    
    result = Mcu_InitClock(MCU_CLOCK_PLL_80MHZ);
    printf("PLL 80MHz initialization: %s\n", (result == E_OK) ? "PASS" : "FAIL");
    
    result = Mcu_InitClock(MCU_CLOCK_PLL_50MHZ);
    printf("PLL 50MHz initialization: %s\n", (result == E_OK) ? "PASS" : "FAIL");
}

/**
 * @brief Test PLL status
 */
static void Test_Mcu_GetPllStatus(void)
{
    printf("Testing PLL Status...\n");
    
    Mcu_PllStatusType pllStatus = Mcu_GetPllStatus();
    
    switch (pllStatus)
    {
        case MCU_PLL_LOCKED:
            printf("PLL Status: LOCKED\n");
            break;
        case MCU_PLL_UNLOCKED:
            printf("PLL Status: UNLOCKED\n");
            break;
        case MCU_PLL_STATUS_UNDEFINED:
            printf("PLL Status: UNDEFINED\n");
            break;
        default:
            printf("PLL Status: UNKNOWN\n");
            break;
    }
}

/**
 * @brief Test reset reason
 */
static void Test_Mcu_GetResetReason(void)
{
    printf("Testing Reset Reason...\n");
    
    Mcu_ResetType resetReason = Mcu_GetResetReason();
    
    switch (resetReason)
    {
        case MCU_POWER_ON_RESET:
            printf("Reset Reason: POWER ON RESET\n");
            break;
        case MCU_EXTERNAL_RESET:
            printf("Reset Reason: EXTERNAL RESET\n");
            break;
        case MCU_BROWN_OUT_RESET:
            printf("Reset Reason: BROWN OUT RESET\n");
            break;
        case MCU_WATCHDOG_RESET:
            printf("Reset Reason: WATCHDOG RESET\n");
            break;
        case MCU_SOFTWARE_RESET:
            printf("Reset Reason: SOFTWARE RESET\n");
            break;
        case MCU_RESET_UNDEFINED:
            printf("Reset Reason: UNDEFINED\n");
            break;
        default:
            printf("Reset Reason: UNKNOWN\n");
            break;
    }
}

/**
 * @brief Test system clock frequency
 */
static void Test_Mcu_GetSystemClock(void)
{
    printf("Testing System Clock Frequency...\n");
    
    uint32 clockFreq = Mcu_GetSystemClock();
    printf("System Clock Frequency: %lu Hz\n", clockFreq);
}

/**
 * @brief Test peripheral clock control
 */
static void Test_Mcu_PeripheralClock(void)
{
    printf("Testing Peripheral Clock Control...\n");
    
    /* Enable GPIO Port F clock as an example */
    Mcu_EnablePeripheralClock(0x6080020UL);  /* RCGCGPIO register offset + Port F bit */
    printf("GPIO Port F clock enabled.\n");
    
    /* Disable GPIO Port F clock */
    Mcu_DisablePeripheralClock(0x6080020UL);
    printf("GPIO Port F clock disabled.\n");
}

/**
 * @brief Test power modes
 */
static void Test_Mcu_SetMode(void)
{
    printf("Testing Power Modes...\n");
    
    /* Test normal mode */
    Mcu_SetMode(MCU_MODE_NORMAL);
    printf("Set to NORMAL mode.\n");
    
    /* Test sleep mode */
    Mcu_SetMode(MCU_MODE_SLEEP);
    printf("Set to SLEEP mode.\n");
    
    /* Return to normal mode */
    Mcu_SetMode(MCU_MODE_NORMAL);
    printf("Returned to NORMAL mode.\n");
}

/**
 * @brief Main test function
 */
int main(void)
{
    printf("MCU Driver Test Application\n");
    printf("===========================\n\n");
    
    /* Run all tests */
    Test_Mcu_Init();
    printf("\n");
    
    Test_Mcu_InitClock();
    printf("\n");
    
    Test_Mcu_GetPllStatus();
    printf("\n");
    
    Test_Mcu_GetResetReason();
    printf("\n");
    
    Test_Mcu_GetSystemClock();
    printf("\n");
    
    Test_Mcu_PeripheralClock();
    printf("\n");
    
    Test_Mcu_SetMode();
    printf("\n");
    
    printf("All tests completed.\n");
    
    return 0;
}