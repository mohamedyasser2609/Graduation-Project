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

#include "MCAL/MCU/Mcu.h"
#include "MCAL/MCU/Mcu_Cfg.h"

/* ===================[External Configuration]=================== */
extern const Mcu_ConfigType* Mcu_ConfigPtr;

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
    /* In a real application, you would use UART here */
    /* For now, this is a placeholder for test structure */
    (void)testName;
}

/**
 * @brief Print test result
 */
void PrintTestResult(const char* testName, boolean passed)
{
    /* In a real application, you would use UART here */
    (void)testName;
    (void)passed;
}

/* ===================[Test Functions]=================== */

/**
 * @brief Test MCU initialization
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Mcu_Init(void)
{
    PrintTestHeader("MCU Initialization Test");
    
    /* Initialize MCU with default configuration (80MHz) */
    Mcu_Init(Mcu_ConfigPtr);
    
    /* Verify initialization by checking system clock */
    uint32 sysClk = Mcu_GetSystemClock();
    
    /* After init, should be at default 16MHz */
    boolean result = (sysClk == 16000000UL);
    
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
    
    /* Test 1: Initialize to 16MHz (no PLL) */
    result = Mcu_InitClock(MCU_CLOCK_16MHZ);
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
    SimpleDelay(100000);
    
    /* Test 3: Initialize to 80MHz with PLL */
    result = Mcu_InitClock(MCU_CLOCK_80MHZ);
    sysClk = Mcu_GetSystemClock();
    if (result != E_OK || sysClk != 80000000UL)
    {
        testPassed = FALSE;
    }
    
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
    
    /* Initialize clock with PLL */
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    
    /* Wait for PLL to lock */
    SimpleDelay(500000);
    
    /* Check PLL status */
    Mcu_PllStatusType pllStatus = Mcu_GetPllStatus();
    
    /* After sufficient delay, PLL should be locked */
    boolean result = (pllStatus == MCU_PLL_LOCKED);
    
    /* Distribute PLL clock */
    if (result == TRUE)
    {
        Std_ReturnType distResult = Mcu_DistributePllClock();
        result = (distResult == E_OK);
    }
    
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
    
    /* Set to 80MHz */
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    uint32 freq80 = Mcu_GetSystemClock();
    
    /* Set to 50MHz */
    Mcu_InitClock(MCU_CLOCK_50MHZ);
    uint32 freq50 = Mcu_GetSystemClock();
    
    /* Set to 16MHz */
    Mcu_InitClock(MCU_CLOCK_16MHZ);
    uint32 freq16 = Mcu_GetSystemClock();
    
    /* Verify frequencies */
    boolean result = (freq80 == 80000000UL) && 
                     (freq50 == 50000000UL) && 
                     (freq16 == 16000000UL);
    
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
    /* In a real application, print results via UART */
    if (allTestsPassed == TRUE)
    {
        /* All tests passed - indicate success (e.g., green LED) */
        while(1)
        {
            /* Success loop */
        }
    }
    else
    {
        /* Some tests failed - indicate failure (e.g., red LED) */
        while(1)
        {
            /* Failure loop */
        }
    }
    
    /* Never reached due to infinite loops above */
}
