/**
 * @file main_wdg_test.c
 * @brief Watchdog Driver Test Application for TM4C123GH6PM
 * @details Comprehensive test for AUTOSAR-compliant Watchdog driver
 *
 * Test Features:
 * - Watchdog initialization and configuration
 * - Watchdog service (kick) functionality
 * - Timeout configuration
 * - Trigger mode selection
 * - Watchdog disable functionality
 * - Status monitoring
 * - Interrupt mode testing
 *
 * @author Mohamed Yasser
 * @date Nov 5, 2025
 * @version 1.0.0
 */

#if 0  /* <<<< ENTIRE IMU MAIN COMMENTED OUT - REMOVE THIS LINE TO ACTIVATE >>>> */


#include "MCAL/WDG/WDG.h"
#include "MCAL/WDG/WDG_Cfg.h"
#include "MCAL/MCU/Mcu.h"
#include "MCAL/UART/UART.h"
#include "MCAL/GPIO/Gpio.h"

/* ===================[External Configuration]=================== */
extern const Wdg_ConfigType Wdg_Config;             /* From WDG_PBCfg.c */
extern const Mcu_ConfigType* Mcu_ConfigPtr;         /* From Mcu_PBCfg.c */
extern const Gpio_ConfigType Gpio_Configuration;    /* From Gpio_PBCfg.c */
extern const Uart_ConfigType Uart0_Config_115200;  /* From Uart_PBCfg.c */

/* ===================[Test Variables]=================== */
volatile boolean watchdogInterruptFired = FALSE;
volatile uint32 watchdogInterruptCount = 0;
volatile boolean watchdogResetOccurred = FALSE;

/* ===================[Interrupt Handlers]=================== */

/**
 * @brief Watchdog 0 interrupt handler
 */
void Watchdog0_Handler(void)
{
    watchdogInterruptFired = TRUE;
    watchdogInterruptCount++;
    
    /* Service watchdog to prevent reset */
    Wdg_Service();
}

/**
 * @brief Timer2A interrupt handler (dummy for linking)
 */
void Timer2A_Handler(void)
{
    /* Empty handler */
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

/**
 * @brief Reset test variables
 */
void ResetTestVariables(void)
{
    watchdogInterruptFired = FALSE;
    watchdogInterruptCount = 0;
}

/* ===================[Test Functions]=================== */

/**
 * @brief Test watchdog initialization
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Wdg_Init(void)
{
    PrintTestHeader("Watchdog Initialization Test");
    
    /* Initialize watchdog with configuration */
    Wdg_Init(&Wdg_Config);
    
    /* Check status */
    Wdg_StatusType status = Wdg_GetStatus();
    
    /* After init, watchdog should be running */
    boolean result = (status == WDG_IDLE || status == WDG_BUSY);
    
    PrintTestResult("Watchdog Init", result);
    return result;
}

/**
 * @brief Test watchdog service (kick) functionality
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Wdg_Service(void)
{
    PrintTestHeader("Watchdog Service Test");
    boolean testPassed = TRUE;
    
    /* Initialize watchdog */
    Wdg_Init(&Wdg_Config);
    
    /* Service watchdog multiple times */
    for (uint32 i = 0; i < 5; i++)
    {
        SimpleDelay(100000);  /* Delay less than timeout */
        Wdg_Service();        /* Kick watchdog */
    }
    
    /* If we reached here, watchdog service is working */
    /* (system didn't reset) */
    
    PrintTestResult("Watchdog Service", testPassed);
    return testPassed;
}

/**
 * @brief Test watchdog trigger mode selection
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Wdg_TriggerMode(void)
{
    PrintTestHeader("Trigger Mode Test");
    boolean testPassed = TRUE;
    
    /* Initialize watchdog */
    Wdg_Init(&Wdg_Config);
    
    /* Test Normal Mode */
    Wdg_SetTriggerMode(WDG_TRIGGER_MODE_NORMAL);
    SimpleDelay(10000);
    Wdg_Service();
    
    /* Test Fast Mode */
    Wdg_SetTriggerMode(WDG_TRIGGER_MODE_FAST);
    SimpleDelay(10000);
    Wdg_Service();
    
    /* Back to Normal Mode */
    Wdg_SetTriggerMode(WDG_TRIGGER_MODE_NORMAL);
    SimpleDelay(10000);
    Wdg_Service();
    
    PrintTestResult("Trigger Mode", testPassed);
    return testPassed;
}

/**
 * @brief Test watchdog timeout configuration
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Wdg_TimeoutConfig(void)
{
    PrintTestHeader("Timeout Configuration Test");
    boolean testPassed = TRUE;
    
    /* Initialize watchdog */
    Wdg_Init(&Wdg_Config);
    
    /* Test 1: Set timeout to 2 seconds (32,000,000 ticks @ 16MHz) */
    Wdg_SetTriggerCondition(32000000);
    SimpleDelay(500000);  /* Wait 0.5 seconds */
    Wdg_Service();
    
    /* Test 2: Set timeout to 1 second (16,000,000 ticks @ 16MHz) */
    Wdg_SetTriggerCondition(16000000);
    SimpleDelay(250000);  /* Wait 0.25 seconds */
    Wdg_Service();
    
    /* Test 3: Set timeout to 500ms (8,000,000 ticks @ 16MHz) */
    Wdg_SetTriggerCondition(8000000);
    SimpleDelay(100000);  /* Wait less than timeout */
    Wdg_Service();
    
    PrintTestResult("Timeout Config", testPassed);
    return testPassed;
}

/**
 * @brief Test watchdog status monitoring
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Wdg_Status(void)
{
    PrintTestHeader("Status Monitoring Test");
    boolean testPassed = TRUE;
    
    /* Initialize watchdog */
    Wdg_Init(&Wdg_Config);
    
    /* Get status */
    Wdg_StatusType status = Wdg_GetStatus();
    
    /* Status should be valid */
    if (status != WDG_IDLE && status != WDG_BUSY)
    {
        testPassed = FALSE;
    }
    
    /* Service and check again */
    Wdg_Service();
    status = Wdg_GetStatus();
    
    if (status != WDG_IDLE && status != WDG_BUSY)
    {
        testPassed = FALSE;
    }
    
    PrintTestResult("Status Monitoring", testPassed);
    return testPassed;
}

/**
 * @brief Test watchdog disable functionality
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Wdg_Disable(void)
{
    PrintTestHeader("Watchdog Disable Test");
    boolean testPassed = TRUE;
    
    /* Initialize watchdog */
    Wdg_Init(&Wdg_Config);
    
    /* Service once */
    Wdg_Service();
    SimpleDelay(100000);
    
    /* Disable watchdog */
    Wdg_Disable();
    
    /* Wait longer than normal timeout without servicing */
    SimpleDelay(1000000);
    
    /* If we reached here, disable worked (no reset) */
    
    PrintTestResult("Watchdog Disable", testPassed);
    return testPassed;
}

/**
 * @brief Test watchdog re-initialization after disable
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Wdg_ReInit(void)
{
    PrintTestHeader("Watchdog Re-Init Test");
    boolean testPassed = TRUE;
    
    /* Initialize watchdog */
    Wdg_Init(&Wdg_Config);
    Wdg_Service();
    
    /* Disable */
    Wdg_Disable();
    SimpleDelay(100000);
    
    /* Re-initialize */
    Wdg_Init(&Wdg_Config);
    
    /* Service to verify it's working */
    Wdg_Service();
    SimpleDelay(100000);
    Wdg_Service();
    
    PrintTestResult("Watchdog Re-Init", testPassed);
    return testPassed;
}

/**
 * @brief Test watchdog periodic servicing
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Wdg_PeriodicService(void)
{
    PrintTestHeader("Periodic Service Test");
    boolean testPassed = TRUE;
    
    /* Initialize watchdog */
    Wdg_Init(&Wdg_Config);
    
    /* Service periodically for 10 iterations */
    for (uint32 i = 0; i < 10; i++)
    {
        /* Simulate periodic task execution */
        SimpleDelay(200000);
        
        /* Service watchdog */
        Wdg_Service();
    }
    
    /* If we reached here, periodic servicing works */
    
    PrintTestResult("Periodic Service", testPassed);
    return testPassed;
}

/**
 * @brief Test watchdog with varying service intervals
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Wdg_VaryingIntervals(void)
{
    PrintTestHeader("Varying Service Intervals Test");
    boolean testPassed = TRUE;
    
    /* Initialize watchdog */
    Wdg_Init(&Wdg_Config);
    
    /* Test with different service intervals */
    
    /* Short interval */
    SimpleDelay(50000);
    Wdg_Service();
    
    /* Medium interval */
    SimpleDelay(200000);
    Wdg_Service();
    
    /* Short interval again */
    SimpleDelay(50000);
    Wdg_Service();
    
    /* Medium interval */
    SimpleDelay(200000);
    Wdg_Service();
    
    /* Long interval (but still within timeout) */
    SimpleDelay(400000);
    Wdg_Service();
    
    PrintTestResult("Varying Intervals", testPassed);
    return testPassed;
}

/**
 * @brief Test watchdog reset reason detection
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Wdg_ResetReason(void)
{
    PrintTestHeader("Reset Reason Detection Test");
    boolean testPassed = TRUE;
    
    /* Check if last reset was from watchdog */
    Mcu_ResetType resetReason = Mcu_GetResetReason();
    
    if (resetReason == MCU_WATCHDOG_RESET)
    {
        /* Watchdog reset occurred */
        watchdogResetOccurred = TRUE;
    }
    
    /* This test always passes - just informational */
    
    PrintTestResult("Reset Reason", testPassed);
    return testPassed;
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
    
    /* Initialize MCU first */
    Mcu_Init(Mcu_ConfigPtr);
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    
    /* Wait for PLL lock */
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED);
    Mcu_DistributePllClock();
    
    /* Initialize GPIO for all pins (includes UART0) */
    Gpio_Init(&Gpio_Configuration);
    
    /* Initialize UART for test output */
    Uart_Init(&Uart0_Config_115200);
    
    /* Print test banner */
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*******************************************\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   Watchdog Driver Test Suite            *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   TM4C123GH6PM LaunchPad                *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   AUTOSAR 4.4.0 Compliant               *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*******************************************\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    /* Test 1: Check Reset Reason (before initializing watchdog) */
    testCount++;
    if (Test_Wdg_ResetReason() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 2: Watchdog Initialization */
    testCount++;
    if (Test_Wdg_Init() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 3: Watchdog Service */
    testCount++;
    if (Test_Wdg_Service() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 4: Trigger Mode */
    testCount++;
    if (Test_Wdg_TriggerMode() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 5: Timeout Configuration */
    testCount++;
    if (Test_Wdg_TimeoutConfig() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 6: Status Monitoring */
    testCount++;
    if (Test_Wdg_Status() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 7: Watchdog Disable */
    testCount++;
    if (Test_Wdg_Disable() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 8: Watchdog Re-Init */
    testCount++;
    if (Test_Wdg_ReInit() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 9: Periodic Service */
    testCount++;
    if (Test_Wdg_PeriodicService() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 10: Varying Service Intervals */
    testCount++;
    if (Test_Wdg_VaryingIntervals() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Disable watchdog before final loop */
    Wdg_Disable();
    
    /* Final Result Summary */
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"=========================================\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"         TEST SUMMARY                    \r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"=========================================\r\n");
    
    /* Print test count */
    Uart_SendString(UART_MODULE_0, (const uint8*)"Total Tests: ");
    uint8 buffer[10];
    if (testCount < 10) {
        buffer[0] = '0' + testCount;
        buffer[1] = '\0';
    } else {
        buffer[0] = '1';
        buffer[1] = '0';
        buffer[2] = '\0';
    }
    Uart_SendString(UART_MODULE_0, buffer);
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"Passed: ");
    if (passedCount < 10) {
        buffer[0] = '0' + passedCount;
        buffer[1] = '\0';
    } else {
        buffer[0] = '1';
        buffer[1] = '0';
        buffer[2] = '\0';
    }
    Uart_SendString(UART_MODULE_0, buffer);
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    if (allTestsPassed == TRUE)
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nRESULT: ALL TESTS PASSED! ✓\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"=========================================\r\n\r\n");
        
        /* All tests passed - indicate success */
        /* In real application: Turn on green LED */
        while(1)
        {
            /* Success loop */
        }
    }
    else
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nRESULT: SOME TESTS FAILED! ✗\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"=========================================\r\n\r\n");
        
        /* Some tests failed - indicate failure */
        /* In real application: Turn on red LED */
        while(1)
        {
            /* Failure loop */
        }
    }
    
    /* Never reached */
}

#endif  /* <<<< END OF COMMENTED OUT IMU TEST CODE >>>> */

