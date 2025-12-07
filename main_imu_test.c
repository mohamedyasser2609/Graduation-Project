/**
 * @file main_imu_test.c
 * @brief IMU MPU-9250 Test Application for TM4C123GH6PM
 * @details Comprehensive test for MPU-9250 IMU driver
 *
 * Test Features:
 * - I2C initialization and bus scan
 * - MPU-9250 device detection
 * - IMU initialization and configuration
 * - Raw sensor data reading (Accel, Gyro, Mag, Temp)
 * - Calibrated sensor data reading
 * - Self-test functionality
 * - Continuous data streaming via UART
 *
 * Hardware Connections:
 * - I2C0: PB2 (SCL), PB3 (SDA) - with internal pull-up resistors
 * - UART0: PA0 (RX), PA1 (TX) - for debug output
 * - MPU-9250: Connected to I2C0, AD0 pin to GND (address 0x68)
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 */

#if 0  /* <<<< ENTIRE IMU MAIN COMMENTED OUT - REMOVE THIS LINE TO ACTIVATE >>>> */


#include "MCAL/MCU/Mcu.h"
#include "MCAL/MCU/Mcu_Cfg.h"
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/I2C/I2C.h"
#include "MCAL/UART/UART.h"
#include "ECUAL/IMU/IMU.h"
#include "CONFIG/Std_Types.h"

/* ===================[External Configuration]=================== */
extern const Mcu_ConfigType* Mcu_ConfigPtr;
extern const Gpio_ConfigType Gpio_Configuration;
extern const I2C_ConfigType* I2C_ConfigPtr;  /* From I2C_PBCfg.c */
extern const IMU_ConfigType* IMU_ConfigPtr;  /* From IMU_PBCfg.c */
extern const Uart_ConfigType Uart0_Config_115200;  /* From Uart_PBCfg.c */

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
 */
void SimpleDelay(uint32 count)
{
    volatile uint32 i;
    for (i = 0; i < count; i++);
}

/* Uart_SendString is already defined in UART.h, no need to redefine */

/**
 * @brief Convert integer to string and send via UART
 * @note Helper function for printing integers
 */
static void Uart_SendInt(Uart_ModuleType module, sint32 value)
{
    uint8 buffer[12];
    uint8 i = 0;
    boolean negative = FALSE;
    
    if (value < 0)
    {
        negative = TRUE;
        value = -value;
    }
    
    if (value == 0)
    {
        buffer[i++] = '0';
    }
    else
    {
        while (value > 0 && i < 11)
        {
            buffer[i++] = '0' + (value % 10);
            value /= 10;
        }
    }
    
    if (negative)
    {
        buffer[i++] = '-';
    }
    
    /* Reverse and send */
    while (i > 0)
    {
        Uart_SendByte(module, buffer[--i]);
    }
}

/**
 * @brief Send float via UART (2 decimal places)
 * @note Helper function for printing floats
 */
static void Uart_SendFloat(Uart_ModuleType module, float32 value)
{
    sint32 intPart = (sint32)value;
    sint32 fracPart = (sint32)((value - intPart) * 100);
    if (fracPart < 0) fracPart = -fracPart;
    
    Uart_SendInt(module, intPart);
    Uart_SendByte(module, '.');
    if (fracPart < 10) Uart_SendByte(module, '0');
    Uart_SendInt(module, fracPart);
}

/**
 * @brief Print test header
 * @note Helper function for formatting output
 */
static void PrintHeader(const char* title)
{
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"========================================\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)title);
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"========================================\r\n");
}

/* ===================[Test Functions]=================== */

/**
 * @brief Test I2C bus scan
 */
void Test_I2C_Scan(void)
{
    uint8 foundAddresses[16];
    uint8 count;
    uint8 i;
    
    PrintHeader("I2C Bus Scan");
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"Scanning I2C bus...\r\n");
    
    count = I2C_ScanBus(I2C_MODULE_0, foundAddresses, 16);
    
    if (count == 0)
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"No devices found!\r\n");
    }
    else
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"Found devices at addresses: ");
        for (i = 0; i < count; i++)
        {
            Uart_SendString(UART_MODULE_0, (const uint8*)"0x");
            if (foundAddresses[i] < 16) Uart_SendByte(UART_MODULE_0, '0');
            /* Simple hex conversion */
            {
                uint8 high = (foundAddresses[i] >> 4) & 0x0F;
                uint8 low = foundAddresses[i] & 0x0F;
                Uart_SendByte(UART_MODULE_0, (high < 10) ? ('0' + high) : ('A' + high - 10));
                Uart_SendByte(UART_MODULE_0, (low < 10) ? ('0' + low) : ('A' + low - 10));
            }
            if (i < count - 1) Uart_SendString(UART_MODULE_0, (const uint8*)", ");
        }
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    }
    
    SimpleDelay(1000000);
}

/**
 * @brief Test reading WHO_AM_I register directly
 */
void Test_IMU_WhoAmI(void)
{
    uint8 whoAmI = 0;
    Std_ReturnType result;
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"Testing WHO_AM_I register...\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"Expected: 0x71 (MPU-9250) or 0x73 (MPU-9255)\r\n");
    
    /* Try to read WHO_AM_I register directly via I2C */
    result = I2C_ReadRegister(I2C_MODULE_0, 0x68, 0x75, &whoAmI, 1);
    
    if (result == E_OK)
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"WHO_AM_I read successful: 0x");
        {
            uint8 high = (whoAmI >> 4) & 0x0F;
            uint8 low = whoAmI & 0x0F;
            Uart_SendByte(UART_MODULE_0, (high < 10) ? ('0' + high) : ('A' + high - 10));
            Uart_SendByte(UART_MODULE_0, (low < 10) ? ('0' + low) : ('A' + low - 10));
        }
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
        
        if (whoAmI == 0x71)
        {
            Uart_SendString(UART_MODULE_0, (const uint8*)"Device identified as MPU-9250!\r\n");
        }
        else if (whoAmI == 0x73)
        {
            Uart_SendString(UART_MODULE_0, (const uint8*)"Device identified as MPU-9255!\r\n");
        }
        else
        {
            Uart_SendString(UART_MODULE_0, (const uint8*)"WARNING: Unexpected device ID!\r\n");
        }
    }
    else
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"WHO_AM_I read FAILED!\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"Check I2C connections:\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"  - SCL (PB2) connected?\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"  - SDA (PB3) connected?\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"  - Power (3.3V) connected?\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"  - GND connected?\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"  - AD0 connected to GND (address 0x68)?\r\n");
    }
    
    SimpleDelay(2000000);
}

/**
 * @brief Test IMU initialization
 */
boolean Test_IMU_Init(void)
{
    PrintHeader("IMU Initialization Test");
    
    /* First test direct WHO_AM_I read */
    Test_IMU_WhoAmI();
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nInitializing IMU driver...\r\n");
    
    if (IMU_Init(IMU_ConfigPtr) == E_OK)
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"IMU initialized successfully!\r\n");
        
        /* Check device presence */
        if (IMU_IsDevicePresent() == TRUE)
        {
            Uart_SendString(UART_MODULE_0, (const uint8*)"MPU-9250 device detected!\r\n");
        }
        else
        {
            Uart_SendString(UART_MODULE_0, (const uint8*)"WARNING: Device not detected!\r\n");
        }
        
        SimpleDelay(1000000);
        return TRUE;
    }
    else
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"IMU initialization FAILED!\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"Status: ");
        {
            IMU_StatusType status = IMU_GetStatus();
            switch(status)
            {
                case IMU_STATUS_DEVICE_NOT_FOUND:
                    Uart_SendString(UART_MODULE_0, (const uint8*)"Device not found\r\n");
                    break;
                case IMU_STATUS_ERROR:
                    Uart_SendString(UART_MODULE_0, (const uint8*)"Communication error\r\n");
                    break;
                case IMU_STATUS_NOT_INITIALIZED:
                    Uart_SendString(UART_MODULE_0, (const uint8*)"Not initialized\r\n");
                    break;
                default:
                    Uart_SendString(UART_MODULE_0, (const uint8*)"Unknown error\r\n");
                    break;
            }
        }
        SimpleDelay(2000000);
        return FALSE;
    }
}

/**
 * @brief Test IMU self-test
 */
void Test_IMU_SelfTest(void)
{
    PrintHeader("IMU Self-Test");
    
    if (IMU_SelfTest() == E_OK)
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"Self-test PASSED\r\n");
    }
    else
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"Self-test FAILED\r\n");
    }
    
    SimpleDelay(1000000);
}

/**
 * @brief Test reading raw sensor data
 */
void Test_IMU_RawData(void)
{
    IMU_SensorDataType rawData;
    
    PrintHeader("Raw Sensor Data");
    
    if (IMU_ReadRawData(&rawData) == E_OK)
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"Accelerometer (raw):\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"  X: ");
        Uart_SendInt(UART_MODULE_0, rawData.accel.x);
        Uart_SendString(UART_MODULE_0, (const uint8*)"  Y: ");
        Uart_SendInt(UART_MODULE_0, rawData.accel.y);
        Uart_SendString(UART_MODULE_0, (const uint8*)"  Z: ");
        Uart_SendInt(UART_MODULE_0, rawData.accel.z);
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
        
        Uart_SendString(UART_MODULE_0, (const uint8*)"Gyroscope (raw):\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"  X: ");
        Uart_SendInt(UART_MODULE_0, rawData.gyro.x);
        Uart_SendString(UART_MODULE_0, (const uint8*)"  Y: ");
        Uart_SendInt(UART_MODULE_0, rawData.gyro.y);
        Uart_SendString(UART_MODULE_0, (const uint8*)"  Z: ");
        Uart_SendInt(UART_MODULE_0, rawData.gyro.z);
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
        
        Uart_SendString(UART_MODULE_0, (const uint8*)"Magnetometer (raw):\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"  X: ");
        Uart_SendInt(UART_MODULE_0, rawData.mag.x);
        Uart_SendString(UART_MODULE_0, (const uint8*)"  Y: ");
        Uart_SendInt(UART_MODULE_0, rawData.mag.y);
        Uart_SendString(UART_MODULE_0, (const uint8*)"  Z: ");
        Uart_SendInt(UART_MODULE_0, rawData.mag.z);
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
        
        Uart_SendString(UART_MODULE_0, (const uint8*)"Temperature (raw): ");
        Uart_SendInt(UART_MODULE_0, rawData.temperature);
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    }
    else
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"Failed to read raw data!\r\n");
    }
    
    SimpleDelay(2000000);
}

/**
 * @brief Test reading calibrated sensor data
 */
void Test_IMU_CalibratedData(void)
{
    IMU_CalibratedDataType calData;
    
    PrintHeader("Calibrated Sensor Data");
    
    if (IMU_ReadCalibratedData(&calData) == E_OK)
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"Accelerometer (g):\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"  X: ");
        Uart_SendFloat(UART_MODULE_0, calData.accel.x);
        Uart_SendString(UART_MODULE_0, (const uint8*)"  Y: ");
        Uart_SendFloat(UART_MODULE_0, calData.accel.y);
        Uart_SendString(UART_MODULE_0, (const uint8*)"  Z: ");
        Uart_SendFloat(UART_MODULE_0, calData.accel.z);
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
        
        Uart_SendString(UART_MODULE_0, (const uint8*)"Gyroscope (deg/s):\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"  X: ");
        Uart_SendFloat(UART_MODULE_0, calData.gyro.x);
        Uart_SendString(UART_MODULE_0, (const uint8*)"  Y: ");
        Uart_SendFloat(UART_MODULE_0, calData.gyro.y);
        Uart_SendString(UART_MODULE_0, (const uint8*)"  Z: ");
        Uart_SendFloat(UART_MODULE_0, calData.gyro.z);
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
        
        Uart_SendString(UART_MODULE_0, (const uint8*)"Magnetometer (uT):\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"  X: ");
        Uart_SendFloat(UART_MODULE_0, calData.mag.x);
        Uart_SendString(UART_MODULE_0, (const uint8*)"  Y: ");
        Uart_SendFloat(UART_MODULE_0, calData.mag.y);
        Uart_SendString(UART_MODULE_0, (const uint8*)"  Z: ");
        Uart_SendFloat(UART_MODULE_0, calData.mag.z);
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
        
        Uart_SendString(UART_MODULE_0, (const uint8*)"Temperature (C): ");
        Uart_SendFloat(UART_MODULE_0, calData.temperature);
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    }
    else
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"Failed to read calibrated data!\r\n");
    }
    
    SimpleDelay(2000000);
}

/**
 * @brief Continuous data streaming
 */
void Test_IMU_ContinuousStream(void)
{
    IMU_CalibratedDataType calData;
    uint32 count = 0;
    
    PrintHeader("Continuous Data Stream");
    Uart_SendString(UART_MODULE_0, (const uint8*)"Streaming data (press reset to stop)...\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"Format: Count | Accel(g) | Gyro(deg/s) | Mag(uT) | Temp(C)\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"------------------------------------------------------------\r\n");
    
    while (1)
    {
        if (IMU_ReadCalibratedData(&calData) == E_OK)
        {
            /* Print count */
            Uart_SendInt(UART_MODULE_0, count++);
            Uart_SendString(UART_MODULE_0, (const uint8*)" | ");
            
            /* Print accelerometer */
            Uart_SendFloat(UART_MODULE_0, calData.accel.x);
            Uart_SendString(UART_MODULE_0, (const uint8*)",");
            Uart_SendFloat(UART_MODULE_0, calData.accel.y);
            Uart_SendString(UART_MODULE_0, (const uint8*)",");
            Uart_SendFloat(UART_MODULE_0, calData.accel.z);
            Uart_SendString(UART_MODULE_0, (const uint8*)" | ");
            
            /* Print gyroscope */
            Uart_SendFloat(UART_MODULE_0, calData.gyro.x);
            Uart_SendString(UART_MODULE_0, (const uint8*)",");
            Uart_SendFloat(UART_MODULE_0, calData.gyro.y);
            Uart_SendString(UART_MODULE_0, (const uint8*)",");
            Uart_SendFloat(UART_MODULE_0, calData.gyro.z);
            Uart_SendString(UART_MODULE_0, (const uint8*)" | ");
            
            /* Print magnetometer */
            Uart_SendFloat(UART_MODULE_0, calData.mag.x);
            Uart_SendString(UART_MODULE_0, (const uint8*)",");
            Uart_SendFloat(UART_MODULE_0, calData.mag.y);
            Uart_SendString(UART_MODULE_0, (const uint8*)",");
            Uart_SendFloat(UART_MODULE_0, calData.mag.z);
            Uart_SendString(UART_MODULE_0, (const uint8*)" | ");
            
            /* Print temperature */
            Uart_SendFloat(UART_MODULE_0, calData.temperature);
            Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
        }
        else
        {
            Uart_SendString(UART_MODULE_0, (const uint8*)"Read error!\r\n");
        }
        
        /* Delay for ~100ms at 80MHz */
        SimpleDelay(800000);
    }
}

/* ===================[Main Function]=================== */

int main(void)
{
    /* Initialize MCU */
    Mcu_Init(Mcu_ConfigPtr);
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    
    /* Wait for system to stabilize */
    SimpleDelay(100000);
    
    /* Initialize GPIO */
    Gpio_Init(&Gpio_Configuration);
    
    /* Initialize UART for debug output */
    Uart_Init(&Uart0_Config_115200);
    
    /* Wait for UART to be ready */
    SimpleDelay(100000);
    
    /* Print welcome message */
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"========================================\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"MPU-9250 IMU Test Application\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"TM4C123GH6PM - I2C0 Test\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"========================================\r\n");
    
    /* Initialize I2C */
    Uart_SendString(UART_MODULE_0, (const uint8*)"Initializing I2C...\r\n");
    I2C_Init(I2C_ConfigPtr);
    SimpleDelay(100000);
    
    /* Check I2C bus status */
    {
        I2C_StatusType status = I2C_GetStatus(I2C_MODULE_0);
        Uart_SendString(UART_MODULE_0, (const uint8*)"I2C Bus Status: ");
        switch(status)
        {
            case I2C_STATUS_IDLE:
                Uart_SendString(UART_MODULE_0, (const uint8*)"IDLE\r\n");
                break;
            case I2C_STATUS_BUSY:
                Uart_SendString(UART_MODULE_0, (const uint8*)"BUSY\r\n");
                break;
            case I2C_STATUS_ERROR:
                Uart_SendString(UART_MODULE_0, (const uint8*)"ERROR\r\n");
                break;
            default:
                Uart_SendString(UART_MODULE_0, (const uint8*)"UNKNOWN\r\n");
                break;
        }
    }
    
    /* Test I2C bus scan */
    Test_I2C_Scan();
    
    /* Test IMU initialization */
    if (Test_IMU_Init() == FALSE)
    {
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nIMU initialization failed. Check connections!\r\n");
        Uart_SendString(UART_MODULE_0, (const uint8*)"Expected: MPU-9250 at address 0x68\r\n");
        while (1);  /* Halt */
    }
    
    /* Run self-test */
    Test_IMU_SelfTest();
    
    /* Test raw data reading */
    Test_IMU_RawData();
    
    /* Test calibrated data reading */
    Test_IMU_CalibratedData();
    
    /* Start continuous streaming */
    Test_IMU_ContinuousStream();
    
    /* Should never reach here - infinite loop in Test_IMU_ContinuousStream() */
    while (1)
    {
        /* Infinite loop - never returns */
    }
}


#endif  /* <<<< END OF COMMENTED OUT IMU TEST CODE >>>> */

