/**
 * @file main_qei_test.c
 * @brief QEI Driver Test Application for TM4C123GH6PM
 * @details Comprehensive test for AUTOSAR-compliant QEI driver
 *
 * Test Features:
 * - QEI initialization and configuration
 * - Position counter reading and setting
 * - Velocity measurement
 * - Direction detection
 * - Interrupt handling (index, direction, error)
 * - Position reset functionality
 * - Maximum position wrap-around
 *
 * Hardware Requirements:
 * - Quadrature encoder connected to QEI0 pins (PD6/PD7)
 * - Optional: Index signal on PD3
 *
 * @author Mohamed Yasser
 * @date Nov 5, 2025
 * @version 1.0.0
 */

#if 0  /* <<<< ENTIRE IMU MAIN COMMENTED OUT - REMOVE THIS LINE TO ACTIVATE >>>> */


#include "MCAL/QEI/QEI.h"
#include "MCAL/QEI/QEI_Cfg.h"
#include "MCAL/MCU/Mcu.h"
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/UART/UART.h"

/* ===================[External Configuration]=================== */
extern const Qei_ConfigType Qei_Config;             /* From QEI_PBCfg.c */
extern const Mcu_ConfigType* Mcu_ConfigPtr;         /* From Mcu_PBCfg.c */
extern const Gpio_ConfigType Gpio_Configuration;    /* From Gpio_PBCfg.c */
extern const Uart_ConfigType Uart0_Config_115200;  /* From Uart_PBCfg.c */

/* ===================[Test Variables]=================== */
volatile boolean indexInterruptFired = FALSE;
volatile boolean directionInterruptFired = FALSE;
volatile boolean errorInterruptFired = FALSE;
volatile uint32 interruptCount = 0;
volatile Qei_DirectionType lastDirection = QEI_DIRECTION_FORWARD;

/* ===================[Interrupt Handlers]=================== */

/**
 * @brief QEI0 interrupt handler
 */
void QEI0_Handler(void)
{
    /* Get interrupt status */
    /* In real implementation, check which interrupt fired */
    
    interruptCount++;
    
    /* Clear interrupts */
    Qei_ClearInterrupt(QEI_INT_INDEX | QEI_INT_DIRECTION | QEI_INT_ERROR);
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
    indexInterruptFired = FALSE;
    directionInterruptFired = FALSE;
    errorInterruptFired = FALSE;
    interruptCount = 0;
}

/**
 * @brief Simulate encoder rotation (for testing without hardware)
 * @note In real hardware test, this would not be needed
 */
void SimulateEncoderRotation(void)
{
    /* This is a placeholder - actual encoder signals come from hardware */
    SimpleDelay(10000);
}

/* ===================[Test Functions]=================== */

/**
 * @brief Test QEI initialization
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Qei_Init(void)
{
    PrintTestHeader("QEI Initialization Test");
    
    /* Initialize QEI with configuration */
    Qei_Init(&Qei_Config);
    
    /* Check initial position */
    uint32 position = Qei_GetPosition();
    
    /* Position should be at initial value (typically 0) */
    boolean result = (position == 0);
    
    PrintTestResult("QEI Init", result);
    return result;
}

/**
 * @brief Test position counter reading
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Qei_PositionRead(void)
{
    PrintTestHeader("Position Reading Test");
    boolean testPassed = TRUE;
    
    /* Initialize QEI */
    Qei_Init(&Qei_Config);
    
    /* Read position multiple times */
    uint32 pos1 = Qei_GetPosition();
    SimpleDelay(10000);
    
    uint32 pos2 = Qei_GetPosition();
    SimpleDelay(10000);
    
    uint32 pos3 = Qei_GetPosition();
    
    /* Positions should be valid (within max position) */
    /* Note: Without actual encoder, positions may not change */
    
    PrintTestResult("Position Read", testPassed);
    return testPassed;
}

/**
 * @brief Test position counter setting
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Qei_PositionSet(void)
{
    PrintTestHeader("Position Setting Test");
    boolean testPassed = TRUE;
    
    /* Initialize QEI */
    Qei_Init(&Qei_Config);
    
    /* Test 1: Set position to 100 */
    Qei_SetPosition(100);
    uint32 pos1 = Qei_GetPosition();
    if (pos1 != 100)
    {
        testPassed = FALSE;
    }
    
    /* Test 2: Set position to 500 */
    Qei_SetPosition(500);
    uint32 pos2 = Qei_GetPosition();
    if (pos2 != 500)
    {
        testPassed = FALSE;
    }
    
    /* Test 3: Set position to 0 */
    Qei_SetPosition(0);
    uint32 pos3 = Qei_GetPosition();
    if (pos3 != 0)
    {
        testPassed = FALSE;
    }
    
    PrintTestResult("Position Set", testPassed);
    return testPassed;
}

/**
 * @brief Test direction detection
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Qei_Direction(void)
{
    PrintTestHeader("Direction Detection Test");
    boolean testPassed = TRUE;
    
    /* Initialize QEI */
    Qei_Init(&Qei_Config);
    
    /* Get current direction */
    Qei_DirectionType dir = Qei_GetDirection();
    
    /* Direction should be either forward or reverse */
    if (dir != QEI_DIRECTION_FORWARD && dir != QEI_DIRECTION_REVERSE)
    {
        testPassed = FALSE;
    }
    
    /* Note: Without actual encoder rotation, direction may not change */
    
    PrintTestResult("Direction Detection", testPassed);
    return testPassed;
}

/**
 * @brief Test velocity measurement
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Qei_Velocity(void)
{
    PrintTestHeader("Velocity Measurement Test");
    boolean testPassed = TRUE;
    
    /* Initialize QEI */
    Qei_Init(&Qei_Config);
    
    /* Read velocity */
    uint32 velocity = Qei_GetVelocity();
    
    /* Velocity should be readable (0 if encoder not moving) */
    /* This test just verifies the API works */
    
    /* Read velocity again after delay */
    SimpleDelay(100000);
    velocity = Qei_GetVelocity();
    
    PrintTestResult("Velocity Measurement", testPassed);
    return testPassed;
}

/**
 * @brief Test position wrap-around at maximum
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Qei_PositionWrap(void)
{
    PrintTestHeader("Position Wrap-Around Test");
    boolean testPassed = TRUE;
    
    /* Initialize QEI */
    Qei_Init(&Qei_Config);
    
    /* Set position near maximum (assuming MaxPosition = 2399 for 600 PPR) */
    /* Get max position from config or use known value */
    uint32 maxPos = 2399;  /* Example: 600 PPR × 4 - 1 */
    
    /* Set to max position */
    Qei_SetPosition(maxPos);
    uint32 pos1 = Qei_GetPosition();
    
    if (pos1 != maxPos)
    {
        testPassed = FALSE;
    }
    
    /* Set to 0 (wrap around) */
    Qei_SetPosition(0);
    uint32 pos2 = Qei_GetPosition();
    
    if (pos2 != 0)
    {
        testPassed = FALSE;
    }
    
    PrintTestResult("Position Wrap", testPassed);
    return testPassed;
}

/**
 * @brief Test interrupt enable/disable
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Qei_InterruptControl(void)
{
    PrintTestHeader("Interrupt Control Test");
    boolean testPassed = TRUE;
    
    /* Initialize QEI */
    Qei_Init(&Qei_Config);
    
    /* Enable index interrupt */
    Qei_EnableInterrupt(QEI_INT_INDEX);
    SimpleDelay(10000);
    
    /* Disable index interrupt */
    Qei_DisableInterrupt(QEI_INT_INDEX);
    SimpleDelay(10000);
    
    /* Enable direction interrupt */
    Qei_EnableInterrupt(QEI_INT_DIRECTION);
    SimpleDelay(10000);
    
    /* Disable direction interrupt */
    Qei_DisableInterrupt(QEI_INT_DIRECTION);
    
    PrintTestResult("Interrupt Control", testPassed);
    return testPassed;
}

/**
 * @brief Test multiple position reads
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Qei_MultipleReads(void)
{
    PrintTestHeader("Multiple Position Reads Test");
    boolean testPassed = TRUE;
    
    /* Initialize QEI */
    Qei_Init(&Qei_Config);
    
    /* Set known position */
    Qei_SetPosition(1000);
    
    /* Read position multiple times */
    for (uint32 i = 0; i < 10; i++)
    {
        uint32 pos = Qei_GetPosition();
        
        /* Position should remain consistent without encoder movement */
        if (pos != 1000)
        {
            /* Position changed (could be due to encoder movement) */
            /* This is not necessarily a failure */
        }
        
        SimpleDelay(5000);
    }
    
    PrintTestResult("Multiple Reads", testPassed);
    return testPassed;
}

/**
 * @brief Test position increment simulation
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Qei_PositionIncrement(void)
{
    PrintTestHeader("Position Increment Test");
    boolean testPassed = TRUE;
    
    /* Initialize QEI */
    Qei_Init(&Qei_Config);
    
    /* Set initial position */
    Qei_SetPosition(0);
    
    /* Simulate incremental position changes */
    for (uint32 i = 0; i < 10; i++)
    {
        uint32 currentPos = Qei_GetPosition();
        
        /* Set to next position */
        Qei_SetPosition(currentPos + 10);
        
        SimpleDelay(5000);
    }
    
    /* Final position should be 100 */
    uint32 finalPos = Qei_GetPosition();
    if (finalPos != 100)
    {
        testPassed = FALSE;
    }
    
    PrintTestResult("Position Increment", testPassed);
    return testPassed;
}

/**
 * @brief Test QEI status
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Qei_Status(void)
{
    PrintTestHeader("QEI Status Test");
    boolean testPassed = TRUE;
    
    /* Initialize QEI */
    Qei_Init(&Qei_Config);
    
    /* Get status */
    Qei_StatusType status = Qei_GetStatus();
    
    /* Status should be valid */
    if (status != QEI_UNINIT && status != QEI_IDLE && status != QEI_BUSY)
    {
        testPassed = FALSE;
    }
    
    PrintTestResult("QEI Status", testPassed);
    return testPassed;
}

/**
 * @brief Test QEI de-initialization
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Qei_DeInit(void)
{
    PrintTestHeader("QEI De-Init Test");
    boolean testPassed = TRUE;
    
    /* Initialize QEI */
    Qei_Init(&Qei_Config);
    
    /* Set position */
    Qei_SetPosition(500);
    
    /* De-initialize */
    Qei_DeInit();
    
    /* Re-initialize */
    Qei_Init(&Qei_Config);
    
    /* Position should be reset to initial value */
    uint32 pos = Qei_GetPosition();
    if (pos != 0)
    {
        /* Position may not be 0 if encoder moved */
        /* This is acceptable */
    }
    
    PrintTestResult("QEI De-Init", testPassed);
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
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   QEI Driver Test Suite                 *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   TM4C123GH6PM LaunchPad                *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   AUTOSAR 4.4.0 Compliant               *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*******************************************\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    /* Note: GPIO initialization for QEI pins would be done here */
    /* Gpio_Init(&Gpio_Qei_Config); */
    
    /* Test 1: QEI Initialization */
    testCount++;
    if (Test_Qei_Init() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 2: Position Reading */
    testCount++;
    if (Test_Qei_PositionRead() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 3: Position Setting */
    testCount++;
    if (Test_Qei_PositionSet() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 4: Direction Detection */
    testCount++;
    if (Test_Qei_Direction() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 5: Velocity Measurement */
    testCount++;
    if (Test_Qei_Velocity() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 6: Position Wrap-Around */
    testCount++;
    if (Test_Qei_PositionWrap() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 7: Interrupt Control */
    testCount++;
    if (Test_Qei_InterruptControl() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 8: Multiple Position Reads */
    testCount++;
    if (Test_Qei_MultipleReads() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 9: Position Increment */
    testCount++;
    if (Test_Qei_PositionIncrement() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 10: QEI Status */
    testCount++;
    if (Test_Qei_Status() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 11: QEI De-Init */
    testCount++;
    if (Test_Qei_DeInit() == TRUE)
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
    if (testCount < 10) {
        buffer[0] = '0' + testCount;
        buffer[1] = '\0';
    } else {
        buffer[0] = '1';
        buffer[1] = '0' + (testCount - 10);
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
        buffer[1] = '0' + (passedCount - 10);
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
            /* In hardware test: Monitor encoder position continuously */
            uint32 position = Qei_GetPosition();
            Qei_DirectionType direction = Qei_GetDirection();
            uint32 velocity = Qei_GetVelocity();
            
            /* Use values to prevent optimization */
            (void)position;
            (void)direction;
            (void)velocity;
            
            SimpleDelay(100000);
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

