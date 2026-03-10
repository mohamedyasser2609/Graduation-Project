/**
 * @file main_temp_test.c
 * @brief AM2320 Temperature Sensor Test Application
 * @details Verification test for distributed temperature sensing system
 *
 * Test Features:
 * - Initialization of I2C and AM2320 driver
 * - Periodic polling of 3 sensors (Motor, MCU, Battery)
 * - Raw value and converted temperature display
 * - Error handling checks
 *
 * Hardware Connections:
 * - I2C0: PB2 (SCL), PB3 (SDA)
 * - Sensor 0 (Motor): Address 0x5C
 * - Sensor 1 (MCU): Address 0x5C
 * - Sensor 2 (Battery): Address 0x5C
 * 
 * @author Mohamed Yasser
 * @date Feb 06, 2026
 * @version 1.0.0
 */

#if 0 /* Set to 1 to enable this main file */

#include "MCAL/MCU/Mcu.h"
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/I2C/I2C.h"
#include "MCAL/UART/Uart.h"
#include "ECUAL/TEMP_SENSOR/AM2320.h"
#include "CONFIG/Std_Types.h"

/* ===================[External Configuration]=================== */
extern const Mcu_ConfigType* Mcu_ConfigPtr;
extern const Gpio_ConfigType Gpio_Configuration;
extern const I2C_ConfigType* I2C_ConfigPtr;
extern const Uart_ConfigType Uart0_Config_115200;

/* ===================[Helper Functions]=================== */

void SimpleDelay(uint32 count)
{
    volatile uint32 i;
    for (i = 0; i < count; i++);
}

/**
 * @brief Convert integer to string and send via UART
 */
static void Uart_SendInt(Uart_ModuleType Module, sint32 value)
{
    uint8 buffer[12];
    uint8 i = 0;
    uint8 j = 0;
    uint8 temp[12];
    boolean negative = FALSE;

    if (value < 0) {
        negative = TRUE;
        value = -value;
    }

    if (value == 0) {
        Uart_SendByte(Module, '0');
        return;
    }

    /* Convert to string */
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    /* Reverse */
    if (negative) buffer[j++] = '-';
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';

    Uart_SendString(Module, buffer);
}

/**
 * @brief Convert float to string and send via UART
 */
static void Uart_SendFloat(Uart_ModuleType Module, float32 value)
{
    sint32 intPart = (sint32)value;
    sint32 fracPart = (sint32)((value - intPart) * 10.0f); 
    
    if (fracPart < 0) fracPart = -fracPart;
    
    Uart_SendInt(Module, intPart);
    Uart_SendByte(Module, '.');
    Uart_SendInt(Module, fracPart);
}

static void PrintString(const char* str)
{
    Uart_SendString(UART_MODULE_0, (const uint8*)str);
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
    PrintString("[TEST] AM2320 Temperature Sensor Test Start\r\n");
    PrintString("========================================\r\n");
    
    /* 2. Init I2C */
    PrintString("Initializing I2C...\r\n");
    I2C_Init(I2C_ConfigPtr);
    
    /* 3. Init Sensors */
    /* Note: AM2320_Init typically takes a config pointer, passing NULL if using internal PBCfg */
    PrintString("Initializing AM2320 Driver...\r\n");
    AM2320_Init(NULL_PTR); 
    
    /* 4. Test Loop */
    while (1)
    {
        AM2320_DataType data;
        uint8 i;
        
        PrintString("\r\n--- Reading Sensors ---\r\n");
        
        /* Test 3 Sensors: IDs 0, 1, 2 */
        for (i = 0; i < 3; i++)
        {
            /* Add delay between reads as AM2320 is slow */
            SimpleDelay(2000000); 

            PrintString("Sensor ");
            Uart_SendInt(UART_MODULE_0, i);
            PrintString(": ");
            
            Std_ReturnType status = AM2320_Read(i, &data);
            
            if (status == E_OK)
            {
                PrintString("Temp=");
                Uart_SendFloat(UART_MODULE_0, data.TemperatureC);
                PrintString("C, Hum=");
                Uart_SendFloat(UART_MODULE_0, data.Humidity);
                PrintString("%\r\n");
            }
            else
            {
                PrintString("ERROR (Read Failed)\r\n");
            }
        }
        
        SimpleDelay(8000000); /* ~1s loop delay */
    }
}

#endif
