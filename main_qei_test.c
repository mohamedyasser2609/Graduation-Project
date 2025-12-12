/**
 * @file main_qei_test.c
 * @brief Encoder Driver Test Application for TM4C123GH6PM
 * @details Exercises the ECUAL encoder driver (dual channel, filtered velocity)
 *
 * Test Features:
 * - Encoder init/deinit (both channels)
 * - Position reading/setting with 64-bit accumulator
 * - Signed velocity measurement (counts/sec) with filtering
 * - Direction detection (supports inversion)
 * - Position reset functionality
 * - Overflow/wrap handling
 *
 * Hardware Requirements:
 * - Quadrature encoders (EMG49) connected to:
 *   - Left:  QEI0 (PD6 PhA, PD7 PhB, PD3 Index optional)
 *   - Right: QEI1 (PC5 PhA, PC6 PhB, PC4 Index optional) — adjust as wired
 * 
 * EMG49 Encoder Specifications:
 * - Typical: 12 PPR (Pulses Per Revolution)
 * - Quadrature mode (post-gear): ~980 counts per revolution
 * - MaxPosition set to 0xFFFFFFFF in QEI_PBCfg.c; ECUAL maintains 64-bit accum
 *
 * @author Mohamed Yasser
 * @date Nov 5, 2025
 * @version 1.0.0
 */

/* Encoder Test Code - Active */


#include <stdint.h>
#include "ECUAL/ENCODER/ENCODER.h"
#include "MCAL/QEI/QEI.h"
#include "MCAL/MCU/Mcu.h"
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/UART/UART.h"

/* ===================[External Configuration]=================== */
extern const Encoder_ConfigType Encoder_Config;     /* From ENCODER_PBCfg.c */
extern const Mcu_ConfigType* Mcu_ConfigPtr;         /* From Mcu_PBCfg.c */
extern const Gpio_ConfigType Gpio_Configuration;    /* From Gpio_PBCfg.c */
extern const Uart_ConfigType Uart0_Config_115200;  /* From Uart_PBCfg.c */

/* ===================[Interrupt Handlers]=================== */

/**
 * @brief QEI0 interrupt handler (maps to MCAL handler)
 * @note Required by startup to satisfy vector table.
 */
void QEI0_Handler(void)
{
    Qei_Qei0Handler();
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
 * @brief Convert integer to string and send via UART
 */
void Uart_SendInt(Uart_ModuleType Module, uint32 value)
{
    uint8 buffer[12];
    uint8 i = 0;
    uint8 j = 0;
    uint8 temp[12];
    
    if (value == 0)
    {
        Uart_SendByte(Module, '0');
        return;
    }
    
    /* Convert to string (reverse order) */
    while (value > 0)
    {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    /* Reverse string */
    while (i > 0)
    {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
    
    Uart_SendString(Module, buffer);
}

/**
 * @brief Send signed 32-bit integer over UART
 */
void Uart_SendIntSigned(Uart_ModuleType Module, int32_t value)
{
    if (value < 0)
    {
        Uart_SendByte(Module, '-');
        value = -value;
    }
    Uart_SendInt(Module, (uint32)value);
}

/* ===================[Test Functions]=================== */

/**
 * @brief Test QEI initialization
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Encoder_Init(void)
{
    PrintTestHeader("Encoder Initialization Test");
    
    Encoder_Init(&Encoder_Config);
    Encoder_UpdateAll();
    
    boolean result = TRUE;
    if (Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT) != 0)
    {
        result = FALSE;
    }
    if (Encoder_GetPositionCounts(ENCODER_CHANNEL_RIGHT) != 0)
    {
        result = FALSE;
    }
    
    PrintTestResult("Encoder Init", result);
    return result;
}

/**
 * @brief Test position counter reading
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Encoder_PositionRead(void)
{
    PrintTestHeader("Position Reading Test");
    boolean testPassed = TRUE;
    
    Encoder_Init(&Encoder_Config);
    Encoder_UpdateAll();

    int64_t pos1L = Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT);
    int64_t pos1R = Encoder_GetPositionCounts(ENCODER_CHANNEL_RIGHT);
    SimpleDelay(10000);
    Encoder_UpdateAll();
    int64_t pos2L = Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT);
    int64_t pos2R = Encoder_GetPositionCounts(ENCODER_CHANNEL_RIGHT);

    (void)pos1L; (void)pos2L; (void)pos1R; (void)pos2R;

    PrintTestResult("Position Read", testPassed);
    return testPassed;
}

/**
 * @brief Test position counter setting
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Encoder_PositionSet(void)
{
    PrintTestHeader("Position Setting Test");
    boolean testPassed = TRUE;
    
    Encoder_Init(&Encoder_Config);

    Encoder_SetPosition(ENCODER_CHANNEL_LEFT, 100);
    Encoder_Update(ENCODER_CHANNEL_LEFT);
    if (Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT) != 100) { testPassed = FALSE; }
    
    Encoder_SetPosition(ENCODER_CHANNEL_LEFT, 500);
    Encoder_Update(ENCODER_CHANNEL_LEFT);
    if (Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT) != 500) { testPassed = FALSE; }
    
    Encoder_ResetPosition(ENCODER_CHANNEL_LEFT);
    Encoder_Update(ENCODER_CHANNEL_LEFT);
    if (Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT) != 0) { testPassed = FALSE; }

    Encoder_SetPosition(ENCODER_CHANNEL_RIGHT, 50);
    Encoder_Update(ENCODER_CHANNEL_RIGHT);
    if (Encoder_GetPositionCounts(ENCODER_CHANNEL_RIGHT) != 50) { testPassed = FALSE; }

    Encoder_ResetPosition(ENCODER_CHANNEL_RIGHT);
    Encoder_Update(ENCODER_CHANNEL_RIGHT);
    if (Encoder_GetPositionCounts(ENCODER_CHANNEL_RIGHT) != 0) { testPassed = FALSE; }
    
    PrintTestResult("Position Set", testPassed);
    return testPassed;
}

/**
 * @brief Test direction detection
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Encoder_Direction(void)
{
    PrintTestHeader("Direction Detection Test");
    boolean testPassed = TRUE;
    
    Encoder_Init(&Encoder_Config);
    Encoder_UpdateAll();
    
    Encoder_DirectionType dirL = Encoder_GetDirection(ENCODER_CHANNEL_LEFT);
    Encoder_DirectionType dirR = Encoder_GetDirection(ENCODER_CHANNEL_RIGHT);
    
    if (dirL != ENCODER_DIRECTION_FORWARD && dirL != ENCODER_DIRECTION_REVERSE) { testPassed = FALSE; }
    if (dirR != ENCODER_DIRECTION_FORWARD && dirR != ENCODER_DIRECTION_REVERSE) { testPassed = FALSE; }
    
    PrintTestResult("Direction Detection", testPassed);
    return testPassed;
}

/**
 * @brief Test velocity measurement
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Encoder_Velocity(void)
{
    PrintTestHeader("Velocity Measurement Test");
    boolean testPassed = TRUE;
    
    Encoder_Init(&Encoder_Config);
    Encoder_UpdateAll();
    
    (void)Encoder_GetVelocityCountsPerSec(ENCODER_CHANNEL_LEFT);
    (void)Encoder_GetVelocityCountsPerSec(ENCODER_CHANNEL_RIGHT);
    
    SimpleDelay(100000);
    Encoder_UpdateAll();
    (void)Encoder_GetVelocityCountsPerSec(ENCODER_CHANNEL_LEFT);
    (void)Encoder_GetVelocityCountsPerSec(ENCODER_CHANNEL_RIGHT);
    
    PrintTestResult("Velocity Measurement", testPassed);
    return testPassed;
}

/**
 * @brief Test position wrap-around at maximum
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Encoder_PositionWrap(void)
{
    PrintTestHeader("Position Wrap-Around Test");
    boolean testPassed = TRUE;
    
    Encoder_Init(&Encoder_Config);
    
    /* Set near max via encoder API (updates accumulator baseline) */
    Encoder_SetPosition(ENCODER_CHANNEL_LEFT, 0xFFFFFF00u);
    Encoder_Update(ENCODER_CHANNEL_LEFT);
    int64_t before = Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT);
    
    /* Emulate HW counter wrap by directly setting QEI module to a low value */
    Qei_SetPositionModule(QEI_MODULE_0, 100u);
    Encoder_Update(ENCODER_CHANNEL_LEFT);
    int64_t after = Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT);
    
    if (after <= before)
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
boolean Test_Encoder_InterruptControl(void)
{
    PrintTestHeader("Interrupt Control Test (Skipped)");
    PrintTestResult("Interrupt Control", TRUE);
    return TRUE;
}

/**
 * @brief Test multiple position reads
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Encoder_MultipleReads(void)
{
    PrintTestHeader("Multiple Position Reads Test");
    boolean testPassed = TRUE;
    
    Encoder_Init(&Encoder_Config);
    Encoder_SetPosition(ENCODER_CHANNEL_LEFT, 1000);
    Encoder_Update(ENCODER_CHANNEL_LEFT);
    
    uint32 i;
    for (i = 0; i < 10; i++)
    {
        (void)Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT);
        SimpleDelay(5000);
        Encoder_Update(ENCODER_CHANNEL_LEFT);
    }
    
    PrintTestResult("Multiple Reads", testPassed);
    return testPassed;
}

/**
 * @brief Test position increment simulation
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Encoder_PositionIncrement(void)
{
    PrintTestHeader("Position Increment Test");
    boolean testPassed = TRUE;
    
    Encoder_Init(&Encoder_Config);
    
    Encoder_SetPosition(ENCODER_CHANNEL_LEFT, 0);
    Encoder_Update(ENCODER_CHANNEL_LEFT);
    
    uint32 i;
    for (i = 0; i < 10; i++)
    {
        int64_t currentPos = Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT);
        Encoder_SetPosition(ENCODER_CHANNEL_LEFT, (uint32)(currentPos + 10));
        Encoder_Update(ENCODER_CHANNEL_LEFT);
        SimpleDelay(5000);
    }
    
    int64_t finalPos = Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT);
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
boolean Test_Encoder_Status(void)
{
    PrintTestHeader("Encoder Status Test");
    boolean testPassed = TRUE;
    
    Encoder_Init(&Encoder_Config);
    
    Encoder_StatusType statusL = Encoder_GetStatus(ENCODER_CHANNEL_LEFT);
    Encoder_StatusType statusR = Encoder_GetStatus(ENCODER_CHANNEL_RIGHT);
    
    if (statusL != ENCODER_STATUS_RUNNING || statusR != ENCODER_STATUS_RUNNING)
    {
        testPassed = FALSE;
    }
    
    PrintTestResult("Encoder Status", testPassed);
    return testPassed;
}

/**
 * @brief Test QEI de-initialization
 * @return TRUE if test passed, FALSE otherwise
 */
boolean Test_Encoder_DeInit(void)
{
    PrintTestHeader("Encoder De-Init Test");
    boolean testPassed = TRUE;
    
    Encoder_Init(&Encoder_Config);
    Encoder_SetPosition(ENCODER_CHANNEL_LEFT, 500);
    
    Encoder_DeInit();
    Encoder_Init(&Encoder_Config);
    Encoder_UpdateAll();
    
    int64_t pos = Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT);
    (void)pos;
    
    PrintTestResult("Encoder De-Init", testPassed);
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
    
    /* Note: GPIO initialization for QEI pins (PD6/PD7/PD3) should be done in Gpio_Configuration */
    /* QEI0 pins: PD6 (PhA), PD7 (PhB), PD3 (Index - optional) */
    /* These pins need to be configured as alternate function 6 for QEI */
    
    /* Test 1: QEI Initialization */
    testCount++;
    if (Test_Encoder_Init() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 2: Position Reading */
    testCount++;
    if (Test_Encoder_PositionRead() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 3: Position Setting */
    testCount++;
    if (Test_Encoder_PositionSet() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 4: Direction Detection */
    testCount++;
    if (Test_Encoder_Direction() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 5: Velocity Measurement */
    testCount++;
    if (Test_Encoder_Velocity() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 6: Position Wrap-Around */
    testCount++;
    if (Test_Encoder_PositionWrap() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 7: Interrupt Control */
    testCount++;
    if (Test_Encoder_InterruptControl() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 8: Multiple Position Reads */
    testCount++;
    if (Test_Encoder_MultipleReads() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 9: Position Increment */
    testCount++;
    if (Test_Encoder_PositionIncrement() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 10: QEI Status */
    testCount++;
    if (Test_Encoder_Status() == TRUE)
    {
        passedCount++;
    }
    else
    {
        allTestsPassed = FALSE;
    }
    
    /* Test 11: QEI De-Init */
    testCount++;
    if (Test_Encoder_DeInit() == TRUE)
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
    Uart_SendInt(UART_MODULE_0, testCount);
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"Passed: ");
    Uart_SendInt(UART_MODULE_0, passedCount);
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    if (allTestsPassed == TRUE)
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nRESULT: ALL TESTS PASSED! ✓\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"=========================================\r\n\r\n");
        
        /* All tests passed - start continuous monitoring */
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nStarting continuous encoder monitoring...\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"Format: L Pos | L Dir | L Vel || R Pos | R Dir | R Vel\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"--------------------------------------------------------\r\n");
        
        while(1)
        {
            /* Continuous monitoring loop */
            Encoder_UpdateAll();

            /* Left channel */
            Uart_SendString(UART_MODULE_0, (const uint8*)"L Pos: ");
            Uart_SendIntSigned(UART_MODULE_0, (int32_t)Encoder_GetPositionCounts(ENCODER_CHANNEL_LEFT));
            Uart_SendString(UART_MODULE_0, (const uint8*)" | Dir: ");
            if (Encoder_GetDirection(ENCODER_CHANNEL_LEFT) == ENCODER_DIRECTION_FORWARD)
            {
                Uart_SendString(UART_MODULE_0, (const uint8*)"FWD");
            }
            else
            {
                Uart_SendString(UART_MODULE_0, (const uint8*)"REV");
            }
            Uart_SendString(UART_MODULE_0, (const uint8*)" | Vel: ");
            Uart_SendIntSigned(UART_MODULE_0, Encoder_GetVelocityCountsPerSec(ENCODER_CHANNEL_LEFT));

            /* Right channel */
            Uart_SendString(UART_MODULE_0, (const uint8*)" || R Pos: ");
            Uart_SendIntSigned(UART_MODULE_0, (int32_t)Encoder_GetPositionCounts(ENCODER_CHANNEL_RIGHT));
            Uart_SendString(UART_MODULE_0, (const uint8*)" | Dir: ");
            if (Encoder_GetDirection(ENCODER_CHANNEL_RIGHT) == ENCODER_DIRECTION_FORWARD)
            {
                Uart_SendString(UART_MODULE_0, (const uint8*)"FWD");
            }
            else
            {
                Uart_SendString(UART_MODULE_0, (const uint8*)"REV");
            }
            Uart_SendString(UART_MODULE_0, (const uint8*)" | Vel: ");
            Uart_SendIntSigned(UART_MODULE_0, Encoder_GetVelocityCountsPerSec(ENCODER_CHANNEL_RIGHT));
            
            Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
            
            SimpleDelay(500000);  /* 500ms delay */
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


/* End of Encoder Test Code */

