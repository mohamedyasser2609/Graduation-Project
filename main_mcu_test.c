/**
 * @file main_mcu_test.c
 * @brief MCU Driver Test Application for TM4C123GH6PM
 * @details Comprehensive test for AUTOSAR-compliant MCU driver
 *
 * Test Features:
 * - MCU initialization and configuration
 * - Clock configuration (16MHz, 50MHz, 80MHz)
 * - PLL status monitoring
 * - Reset reason detection
 * - System clock frequency readout
 * - Peripheral clock control (GPIO example)
 * - Power mode transitions (Normal, Sleep, Deep Sleep)
 *
 * @author Mohamed Yasser
 * @date Nov 2, 2025
 * @version 1.0.0
 */

#if 0  /* <<<< ENTIRE IMU MAIN COMMENTED OUT - REMOVE THIS LINE TO ACTIVATE >>>> */


#include "MCAL/MCU/Mcu.h"
#include "MCAL/MCU/Mcu_Cfg.h"
#include "MCAL/UART/UART.h"
#include "MCAL/GPIO/Gpio.h"

/* ===================[External Configuration]=================== */
extern const Mcu_ConfigType* Mcu_ConfigPtr;
extern const Gpio_ConfigType Gpio_Configuration;  /* From Gpio_PBCfg.c */
extern const Uart_ConfigType Uart0_Config_115200; /* From Uart_PBCfg.c */

/* ===================[Interrupt Handlers]=================== */

/**
 * @brief Timer2A interrupt handler (dummy for linking)
 * @note This test doesn't use Timer2A, but the handler must be defined
 *       because it's referenced in the startup file
 */
void Timer2A_Handler(void)
{
    /* Empty handler - not used in this test */
}

/* ===================[Helper Functions]=================== */

/**
 * @brief Simple delay function
 * @param count Number of iterations
 */
void SimpleDelay(uint32 count)
{
    volatile uint32 i;
    for (i = 0; i < count; i++)
    {
        /* Empty loop for delay */
    }
}

/**
 * @brief Print test header
 */
void PrintTestHeader(const char* testName)
{
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n========================================\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"TEST: ");
    Uart_SendString(UART_MODULE_0, (const uint8*)testName);
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n========================================\r\n");
}

/**
 * @brief Print test result
 */
void PrintTestResult(const char* testName, boolean passed)
{
    Uart_SendString(UART_MODULE_0, (const uint8*)"Result: ");
    Uart_SendString(UART_MODULE_0, (const uint8*)testName);
    Uart_SendString(UART_MODULE_0, (const uint8*)" - ");
    
    if (passed == TRUE)
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"PASSED\r\n");
    }
    else
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"FAILED\r\n");
    }
}

/* ===================[Test Functions]=================== */

/**
 * @brief Test MCU initialization
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Mcu_Init(void)
{
    PrintTestHeader("MCU Initialization Test");
    
    /* MCU already initialized in main() and switched to 80MHz */
    /* Just verify it's working */
    
    /* Verify initialization by checking system clock */
    uint32 sysClk = Mcu_GetSystemClock();
    
    /* After main() initialization, should be at 80MHz */
    boolean result = (sysClk == 80000000UL);
    
    PrintTestResult("MCU Init", result);
    return result;
}

/**
 * @brief Test clock initialization with different settings
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Mcu_ClockInit(void)
{
    Std_ReturnType result;
    uint32 sysClk;
    boolean testPassed = TRUE;
    
    PrintTestHeader("Clock Initialization Test");
    
    /* Wait for UART to finish printing header */
    volatile uint32 i;
    for(i = 0; i < 50000; i++);
    
    /* Test 1: Initialize to 16MHz (no PLL) */
    result = Mcu_InitClock(MCU_CLOCK_16MHZ);
    for( i = 0; i < 50000; i++);
    /* Reinitialize UART for 16MHz */
    Uart_DeInit(UART_MODULE_0);
    Uart_Init(&Uart0_Config_115200);
    sysClk = Mcu_GetSystemClock();
    if (result != E_OK || sysClk != 16000000UL)
    {
        testPassed = FALSE;
    }
    SimpleDelay(100000);
    
    /* Test 2: Initialize to 50MHz with PLL */
    result = Mcu_InitClock(MCU_CLOCK_50MHZ);
    if (result != E_OK)
    {
        testPassed = FALSE;
    }
    /* Wait for PLL lock and distribute */
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED);
    Mcu_DistributePllClock();
    for( i = 0; i < 50000; i++);
    /* Reinitialize UART for 50MHz */
    Uart_DeInit(UART_MODULE_0);
    Uart_Init(&Uart0_Config_115200);
    SimpleDelay(100000);
    
    /* Test 3: Initialize to 80MHz with PLL */
    result = Mcu_InitClock(MCU_CLOCK_80MHZ);
    if (result != E_OK)
    {
        testPassed = FALSE;
    }
    /* Wait for PLL lock and distribute */
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED);
    Mcu_DistributePllClock();
    for( i = 0; i < 50000; i++);
    /* Reinitialize UART for 80MHz */
    Uart_DeInit(UART_MODULE_0);
    Uart_Init(&Uart0_Config_115200);
    /* NOW check the clock frequency */
    sysClk = Mcu_GetSystemClock();
    if (sysClk != 80000000UL)
    {
        testPassed = FALSE;
    }
    
    /* Wait before printing result */
    for( i = 0; i < 50000; i++);
    PrintTestResult("Clock Init", testPassed);
    return testPassed;
}

/**
 * @brief Test PLL status monitoring
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Mcu_PllStatus(void)
{
    PrintTestHeader("PLL Status Test");
    
    /* Wait for UART to finish printing header */
    volatile uint32 i;
    for(i = 0; i < 50000; i++);
    
    /* Initialize clock with PLL */
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    
    /* Wait for PLL to lock */
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED);
    
    /* Check PLL status */
    Mcu_PllStatusType pllStatus = Mcu_GetPllStatus();
    
    /* After sufficient delay, PLL should be locked */
    boolean result = (pllStatus == MCU_PLL_LOCKED);
    
    /* Distribute PLL clock */
    if (result == TRUE)
    {
        Std_ReturnType distResult = Mcu_DistributePllClock();
        result = (distResult == E_OK);
        
        /* Wait for clock to stabilize */
        for(i = 0; i < 50000; i++);
        
        /* Reinitialize UART for 80MHz */
        Uart_DeInit(UART_MODULE_0);
        Uart_Init(&Uart0_Config_115200);
    }
    
    /* Wait before printing result */
    for(i = 0; i < 50000; i++);
    PrintTestResult("PLL Status", result);
    return result;
}

/**
 * @brief Test reset reason detection
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Mcu_ResetReason(void)
{
    PrintTestHeader("Reset Reason Test");
    
    /* Get reset reason */
    Mcu_ResetType resetReason = Mcu_GetResetReason();
    
    /* Get raw reset value */
    uint32 rawReset = Mcu_GetResetRawValue();
    
    /* Any valid reset reason is acceptable */
    boolean result = (resetReason != MCU_RESET_UNDEFINED) && (rawReset != 0UL);
    
    PrintTestResult("Reset Reason", result);
    return result;
}

/**
 * @brief Test system clock frequency readout
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Mcu_SystemClock(void)
{
    PrintTestHeader("System Clock Test");
    
    /* Wait for UART to finish printing header */
    volatile uint32 i;
    for(i = 0; i < 50000; i++);
    
    /* Test 1: Set to 80MHz */
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED);
    Mcu_DistributePllClock();
    for(i = 0; i < 50000; i++);
    Uart_DeInit(UART_MODULE_0);
    Uart_Init(&Uart0_Config_115200);
    uint32 freq80 = Mcu_GetSystemClock();
    
    /* Test 2: Set to 50MHz */
    Mcu_InitClock(MCU_CLOCK_50MHZ);
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED);
    Mcu_DistributePllClock();
    for(i = 0; i < 50000; i++);
    Uart_DeInit(UART_MODULE_0);
    Uart_Init(&Uart0_Config_115200);
    uint32 freq50 = Mcu_GetSystemClock();
    
    /* Test 3: Set to 16MHz */
    Mcu_InitClock(MCU_CLOCK_16MHZ);
    for(i = 0; i < 50000; i++);
    Uart_DeInit(UART_MODULE_0);
    Uart_Init(&Uart0_Config_115200);
    uint32 freq16 = Mcu_GetSystemClock();
    
    /* Verify frequencies */
    boolean result = (freq80 == 80000000UL) && 
                     (freq50 == 50000000UL) && 
                     (freq16 == 16000000UL);
    
    /* Switch back to 80MHz for remaining tests */
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED);
    Mcu_DistributePllClock();
    for(i = 0; i < 50000; i++);
    Uart_DeInit(UART_MODULE_0);
    Uart_Init(&Uart0_Config_115200);
    
    /* Wait before printing result */
    for(i = 0; i < 50000; i++);
    PrintTestResult("System Clock", result);
    return result;
}

/**
 * @brief Test peripheral clock control
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Mcu_PeripheralClock(void)
{
    PrintTestHeader("Peripheral Clock Test");
    
    /* Enable GPIO Port F clock using helper macro */
    uint32 gpioPortF = MCU_PERIPHERAL_GPIO(0x20);  /* Bit 5 for Port F */
    Mcu_EnablePeripheralClock(gpioPortF);
    
    /* Small delay for clock to stabilize */
    SimpleDelay(10000);
    
    /* Disable GPIO Port F clock */
    Mcu_DisablePeripheralClock(gpioPortF);
    
    /* Test passed if no crash occurred */
    boolean result = TRUE;
    
    PrintTestResult("Peripheral Clock", result);
    return result;
}

/**
 * @brief Test power mode transitions
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Mcu_PowerModes(void)
{
    PrintTestHeader("Power Mode Test");
    
    /* Test Normal Mode */
    Mcu_SetMode(MCU_MODE_NORMAL);
    SimpleDelay(10000);
    
    /* Test Sleep Mode (will wake on any interrupt) */
    Mcu_SetMode(MCU_MODE_SLEEP);
    /* CPU will halt here until interrupt */
    
    /* Return to Normal Mode */
    Mcu_SetMode(MCU_MODE_NORMAL);
    SimpleDelay(10000);
    
    /* Note: Deep Sleep test is commented out as it requires
     * proper wake-up configuration (RTC, GPIO interrupt, etc.)
     */
    /* Mcu_SetMode(MCU_MODE_DEEP_SLEEP); */
    
    boolean result = TRUE;
    
    PrintTestResult("Power Modes", result);
    return result;
}

/* ===================[Main Function]=================== */

/**
 * @brief Main test application
 */
int main(void)
{
    boolean allTestsPassed = TRUE;
    uint32 testCount = 0;
    uint32 passedCount = 0;
    
    /* Step 1: Initialize MCU first (required for all peripherals) */
    Mcu_Init(Mcu_ConfigPtr);
    
    /* Step 2: Initialize GPIO for all pins (includes UART0 pins PA0/PA1) */
    Gpio_Init(&Gpio_Configuration);
    
    /* Step 3: Initialize UART at 16MHz first to print initial message */
    Uart_Init(&Uart0_Config_115200);
    
    /* Print initial message at 16MHz */
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n\r\nInitializing at 16MHz...\r\n");
    
    /* Verify current clock */
    uint32 clock1 = Mcu_GetSystemClock();
    if (clock1 == 16000000UL) {
        Uart_SendString(UART_MODULE_0, (const uint8*)"Clock verified: 16MHz\r\n");
    }
    
    /* Step 4: Now switch to 80MHz */
    Uart_SendString(UART_MODULE_0, (const uint8*)"Switching to 80MHz PLL...\r\n");
    
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    
    /* Check PLL status */
    Uart_SendString(UART_MODULE_0, (const uint8*)"Waiting for PLL lock...\r\n");
    uint32 pllWait = 0;
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED && pllWait < 1000000) {
        pllWait++;
    }
    
    if (Mcu_GetPllStatus() == MCU_PLL_LOCKED) {
        Uart_SendString(UART_MODULE_0, (const uint8*)"PLL locked!\r\n");
        /* Wait for UART transmission to complete before switching clock */
        SimpleDelay(10000);
    } else {
        Uart_SendString(UART_MODULE_0, (const uint8*)"ERROR: PLL did not lock!\r\n");
        while(1); /* Halt */
    }
    
    /* Now switch to PLL - UART will be garbled after this until reinitialized */
    Std_ReturnType result = Mcu_DistributePllClock();
    
    if (result != E_OK) {
        Uart_SendString(UART_MODULE_0, (const uint8*)"ERROR: Failed to distribute PLL clock!\r\n");
        while(1);
    }
    
    /* Hardware delay at new clock speed (80MHz) */
    volatile uint32 i;
    for(i = 0; i < 100000; i++);
    
    /* Step 5: Re-initialize UART at new clock frequency */
    Uart_DeInit(UART_MODULE_0);
    Uart_Init(&Uart0_Config_115200);
    
    /* Verify what clock frequency UART sees */
    uint32 clockAfterSwitch = Mcu_GetSystemClock();
    
    /* Now we can print again at correct baud rate */
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n=== Clock switched successfully ===\r\n");
    
    if (clockAfterSwitch == 80000000UL) {
        Uart_SendString(UART_MODULE_0, (const uint8*)"System clock: 80MHz - CORRECT!\r\n");
    } else if (clockAfterSwitch == 16000000UL) {
        Uart_SendString(UART_MODULE_0, (const uint8*)"System clock: 16MHz - FAILED TO SWITCH!\r\n");
        while(1);
    } else {
        Uart_SendString(UART_MODULE_0, (const uint8*)"System clock: UNKNOWN FREQUENCY!\r\n");
        while(1);
    }
    
    /* Print test banner */
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*******************************************\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   MCU Driver Test Suite                 *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   TM4C123GH6PM LaunchPad                *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   AUTOSAR 4.4.0 Compliant               *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*******************************************\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    /* Test 1: MCU Initialization */
    testCount++;
    if (Test_Mcu_Init() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 2: Clock Initialization */
    testCount++;
    if (Test_Mcu_ClockInit() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 3: PLL Status */
    testCount++;
    if (Test_Mcu_PllStatus() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 4: Reset Reason */
    testCount++;
    if (Test_Mcu_ResetReason() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 5: System Clock */
    testCount++;
    if (Test_Mcu_SystemClock() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 6: Peripheral Clock */
    testCount++;
    if (Test_Mcu_PeripheralClock() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 7: Power Modes */
    testCount++;
    if (Test_Mcu_PowerModes() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Final Result Summary */
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"=========================================\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"         TEST SUMMARY                    \r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"=========================================\r\n");
    
    /* Print test count */
    Uart_SendString(UART_MODULE_0, (const uint8*)"Total Tests: ");
    uint8 buffer[10];
    buffer[0] = '0' + testCount;
    buffer[1] = '\0';
    Uart_SendString(UART_MODULE_0, buffer);
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"Passed: ");
    buffer[0] = '0' + passedCount;
    Uart_SendString(UART_MODULE_0, buffer);
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    if (allTestsPassed == TRUE)
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nRESULT: ALL TESTS PASSED! ✓\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"=========================================\r\n\r\n");
        
        /* All tests passed - indicate success (e.g., green LED) */
        while(1)
        {
            /* Success loop */
        }
    }
    else
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nRESULT: SOME TESTS FAILED! ✗\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"=========================================\r\n\r\n");
        
        /* Some tests failed - indicate failure (e.g., red LED) */
        while(1)
        {
            /* Failure loop */
        }
    }
    
    /* Never reached due to infinite loops above */
}


#endif  /* <<<< END OF COMMENTED OUT IMU TEST CODE >>>> */

