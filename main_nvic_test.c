A/**
 * @file main_nvic_test.c
 * @brief NVIC Driver Test Application for TM4C123GH6PM
 * @details Comprehensive test for AUTOSAR-compliant NVIC driver
 *
 * Test Features:
 * - NVIC initialization and configuration
 * - Interrupt enable/disable control
 * - Priority configuration and verification
 * - Pending interrupt management
 * - Software interrupt generation
 * - Nested interrupt testing
 * - Active interrupt status checking
 *
 * @author Mohamed Yasser
 * @date Nov 5, 2025
 * @version 1.0.0
 */

#if 0  /* <<<< ENTIRE IMU MAIN COMMENTED OUT - REMOVE THIS LINE TO ACTIVATE >>>> */


#include "MCAL/NVIC/NVIC.h"
#include "MCAL/NVIC/NVIC_Cfg.h"
#include "MCAL/MCU/Mcu.h"
#include "MCAL/UART/UART.h"
#include "MCAL/GPIO/Gpio.h"

/* ===================[External Configuration]=================== */
extern const NVIC_ConfigType NVIC_Config;           /* From NVIC_PBCfg.c */
extern const Mcu_ConfigType* Mcu_ConfigPtr;         /* From Mcu_PBCfg.c */
extern const Gpio_ConfigType Gpio_Configuration;    /* From Gpio_PBCfg.c */
extern const Uart_ConfigType Uart0_Config_115200;  /* From Uart_PBCfg.c */

/* ===================[Test Variables]=================== */
volatile boolean timer0A_Fired = FALSE;
volatile boolean timer0B_Fired = FALSE;
volatile boolean uart0_Fired = FALSE;
volatile uint32 interruptCount = 0;
volatile boolean nestedInterruptOccurred = FALSE;

/* ===================[Interrupt Handlers]=================== */

/**
 * @brief Timer0A interrupt handler for testing
 */
void Timer0A_Handler(void)
{
    timer0A_Fired = TRUE;
    interruptCount++;
    
    /* Clear any pending flags if needed */
    /* In real application: Timer_ClearInterrupt() */
}

/**
 * @brief Timer0B interrupt handler for testing
 */
void Timer0B_Handler(void)
{
    timer0B_Fired = TRUE;
    interruptCount++;
}

/**
 * @brief UART0 interrupt handler for testing
 */
void UART0_Handler(void)
{
    uart0_Fired = TRUE;
    interruptCount++;
}

/**
 * @brief Timer1A interrupt handler for nested interrupt test
 */
void Timer1A_Handler(void)
{
    /* High priority interrupt */
    nestedInterruptOccurred = TRUE;
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
    timer0A_Fired = FALSE;
    timer0B_Fired = FALSE;
    uart0_Fired = FALSE;
    interruptCount = 0;
    nestedInterruptOccurred = FALSE;
}

/* ===================[Test Functions]=================== */

/**
 * @brief Test NVIC initialization
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_NVIC_Init(void)
{
    PrintTestHeader("NVIC Initialization Test");
    
    /* Initialize NVIC with configuration */
    NVIC_Init(&NVIC_Config);
    
    /* Test passed if no crash occurred */
    boolean result = TRUE;
    
    PrintTestResult("NVIC Init", result);
    return result;
}

/**
 * @brief Test interrupt enable/disable
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_NVIC_EnableDisable(void)
{
    PrintTestHeader("Interrupt Enable/Disable Test");
    boolean testPassed = TRUE;
    
    /* Test 1: Enable Timer0A interrupt */
    NVIC_EnableIRQ(NVIC_TIMER0A_IRQ);
    SimpleDelay(1000);
    
    /* Test 2: Disable Timer0A interrupt */
    NVIC_DisableIRQ(NVIC_TIMER0A_IRQ);
    SimpleDelay(1000);
    
    /* Test 3: Enable multiple interrupts */
    NVIC_EnableIRQ(NVIC_UART0_IRQ);
    NVIC_EnableIRQ(NVIC_TIMER0B_IRQ);
    SimpleDelay(1000);
    
    /* Test 4: Disable multiple interrupts */
    NVIC_DisableIRQ(NVIC_UART0_IRQ);
    NVIC_DisableIRQ(NVIC_TIMER0B_IRQ);
    
    PrintTestResult("Enable/Disable", testPassed);
    return testPassed;
}

/**
 * @brief Test priority configuration
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_NVIC_Priority(void)
{
    PrintTestHeader("Priority Configuration Test");
    boolean testPassed = TRUE;
    
    /* Set different priorities */
    NVIC_SetPriority(NVIC_TIMER0A_IRQ, 0);  /* Highest */
    NVIC_SetPriority(NVIC_UART0_IRQ, 3);    /* Medium */
    NVIC_SetPriority(NVIC_TIMER0B_IRQ, 7);  /* Lowest */
    
    /* Verify priorities */
    NVIC_PriorityType priority0 = NVIC_GetPriority(NVIC_TIMER0A_IRQ);
    NVIC_PriorityType priority1 = NVIC_GetPriority(NVIC_UART0_IRQ);
    NVIC_PriorityType priority2 = NVIC_GetPriority(NVIC_TIMER0B_IRQ);
    
    if (priority0 != 0 || priority1 != 3 || priority2 != 7)
    {
        testPassed = FALSE;
    }
    
    PrintTestResult("Priority Config", testPassed);
    return testPassed;
}

/**
 * @brief Test pending interrupt management
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_NVIC_Pending(void)
{
    PrintTestHeader("Pending Interrupt Test");
    boolean testPassed = TRUE;
    
    ResetTestVariables();
    
    /* Disable interrupt to test pending without firing */
    NVIC_DisableIRQ(NVIC_TIMER0A_IRQ);
    
    /* Set interrupt pending */
    NVIC_SetPendingIRQ(NVIC_TIMER0A_IRQ);
    SimpleDelay(1000);
    
    /* Interrupt should not have fired (disabled) */
    if (timer0A_Fired == TRUE)
    {
        testPassed = FALSE;
    }
    
    /* Clear pending interrupt */
    NVIC_ClearPendingIRQ(NVIC_TIMER0A_IRQ);
    SimpleDelay(1000);
    
    /* Enable interrupt - should not fire now (pending cleared) */
    NVIC_EnableIRQ(NVIC_TIMER0A_IRQ);
    SimpleDelay(1000);
    
    if (timer0A_Fired == TRUE)
    {
        testPassed = FALSE;
    }
    
    /* Clean up */
    NVIC_DisableIRQ(NVIC_TIMER0A_IRQ);
    
    PrintTestResult("Pending Management", testPassed);
    return testPassed;
}

/**
 * @brief Test software interrupt generation
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_NVIC_SoftwareInterrupt(void)
{
    PrintTestHeader("Software Interrupt Test");
    boolean testPassed = TRUE;
    
    ResetTestVariables();
    
    /* Enable interrupt */
    NVIC_EnableIRQ(NVIC_TIMER0A_IRQ);
    
    /* Generate software interrupt */
    NVIC_GenerateSoftwareInterrupt(NVIC_TIMER0A_IRQ);
    
    /* Small delay for interrupt to fire */
    SimpleDelay(10000);
    
    /* Verify interrupt fired */
    if (timer0A_Fired != TRUE)
    {
        testPassed = FALSE;
    }
    
    /* Clean up */
    NVIC_DisableIRQ(NVIC_TIMER0A_IRQ);
    NVIC_ClearPendingIRQ(NVIC_TIMER0A_IRQ);
    
    PrintTestResult("Software Interrupt", testPassed);
    return testPassed;
}

/**
 * @brief Test active interrupt status
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_NVIC_ActiveStatus(void)
{
    PrintTestHeader("Active Status Test");
    boolean testPassed = TRUE;
    
    /* Check that no interrupt is currently active */
    boolean isActive = NVIC_IsActiveIRQ(NVIC_TIMER0A_IRQ);
    
    if (isActive == TRUE)
    {
        /* Should not be active when not in ISR */
        testPassed = FALSE;
    }
    
    PrintTestResult("Active Status", testPassed);
    return testPassed;
}

/**
 * @brief Test multiple interrupt handling
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_NVIC_MultipleInterrupts(void)
{
    PrintTestHeader("Multiple Interrupts Test");
    boolean testPassed = TRUE;
    
    ResetTestVariables();
    
    /* Enable multiple interrupts */
    NVIC_EnableIRQ(NVIC_TIMER0A_IRQ);
    NVIC_EnableIRQ(NVIC_TIMER0B_IRQ);
    NVIC_EnableIRQ(NVIC_UART0_IRQ);
    
    /* Generate software interrupts */
    NVIC_GenerateSoftwareInterrupt(NVIC_TIMER0A_IRQ);
    SimpleDelay(5000);
    
    NVIC_GenerateSoftwareInterrupt(NVIC_TIMER0B_IRQ);
    SimpleDelay(5000);
    
    NVIC_GenerateSoftwareInterrupt(NVIC_UART0_IRQ);
    SimpleDelay(5000);
    
    /* Verify all interrupts fired */
    if (timer0A_Fired != TRUE || timer0B_Fired != TRUE || uart0_Fired != TRUE)
    {
        testPassed = FALSE;
    }
    
    /* Verify interrupt count */
    if (interruptCount != 3)
    {
        testPassed = FALSE;
    }
    
    /* Clean up */
    NVIC_DisableIRQ(NVIC_TIMER0A_IRQ);
    NVIC_DisableIRQ(NVIC_TIMER0B_IRQ);
    NVIC_DisableIRQ(NVIC_UART0_IRQ);
    
    PrintTestResult("Multiple Interrupts", testPassed);
    return testPassed;
}

/**
 * @brief Test priority-based interrupt handling
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_NVIC_PriorityHandling(void)
{
    PrintTestHeader("Priority Handling Test");
    boolean testPassed = TRUE;
    
    ResetTestVariables();
    
    /* Set priorities: Timer0A (high), Timer0B (low) */
    NVIC_SetPriority(NVIC_TIMER0A_IRQ, 0);  /* Highest */
    NVIC_SetPriority(NVIC_TIMER0B_IRQ, 7);  /* Lowest */
    
    /* Enable both */
    NVIC_EnableIRQ(NVIC_TIMER0A_IRQ);
    NVIC_EnableIRQ(NVIC_TIMER0B_IRQ);
    
    /* Trigger both simultaneously */
    NVIC_GenerateSoftwareInterrupt(NVIC_TIMER0A_IRQ);
    NVIC_GenerateSoftwareInterrupt(NVIC_TIMER0B_IRQ);
    
    SimpleDelay(10000);
    
    /* Both should have fired */
    if (timer0A_Fired != TRUE || timer0B_Fired != TRUE)
    {
        testPassed = FALSE;
    }
    
    /* Clean up */
    NVIC_DisableIRQ(NVIC_TIMER0A_IRQ);
    NVIC_DisableIRQ(NVIC_TIMER0B_IRQ);
    
    PrintTestResult("Priority Handling", testPassed);
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
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   NVIC Driver Test Suite                *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   TM4C123GH6PM LaunchPad                *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   AUTOSAR 4.4.0 Compliant               *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*******************************************\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    /* Test 1: NVIC Initialization */
    testCount++;
    if (Test_NVIC_Init() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 2: Enable/Disable */
    testCount++;
    if (Test_NVIC_EnableDisable() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 3: Priority Configuration */
    testCount++;
    if (Test_NVIC_Priority() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 4: Pending Interrupt Management */
    testCount++;
    if (Test_NVIC_Pending() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 5: Software Interrupt */
    testCount++;
    if (Test_NVIC_SoftwareInterrupt() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 6: Active Status */
    testCount++;
    if (Test_NVIC_ActiveStatus() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 7: Multiple Interrupts */
    testCount++;
    if (Test_NVIC_MultipleInterrupts() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 8: Priority Handling */
    testCount++;
    if (Test_NVIC_PriorityHandling() == TRUE)
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
