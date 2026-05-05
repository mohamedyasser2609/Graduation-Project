/**
 * @file main_ros2.c
 * @brief Transparent UART Bridge for TM4C123GH6PM
 * @details Simple bidirectional UART bridge between PC and Raspberry Pi
 *
 * Hardware Setup:
 * - TM4C123GH6PM LaunchPad
 * - Raspberry Pi 5 (ROS 2)
 * - UART Connection:
 *   - TM4C TX (PB1) → RPi RX (GPIO 15)
 *   - TM4C RX (PB0) → RPi TX (GPIO 14)
 *   - GND → GND (IMPORTANT!)
 *
 * UART Configuration:
 * - UART1 (PB0/PB1): ROS 2 Communication @ 115200 baud
 * - UART0 (PA0/PA1): Debug/PC @ 115200 baud
 *
 * Operation:
 * - Type in serial monitor → Sent to Raspberry Pi
 * - Data from Raspberry Pi → Displayed in serial monitor
 * - Completely transparent - no protocol processing
 *
 * @author Mohamed Yasser
 * @date Oct 29, 2025
 * @version 2.0.0 (Transparent Bridge Mode)
 */

#if 0  /* <<<< ENTIRE IMU MAIN COMMENTED OUT - REMOVE THIS LINE TO ACTIVATE >>>> */


/* ===================[Includes]=================== */
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/UART/Uart.h"
#include "MCAL/MCU/Mcu.h"
#include "CONFIG/Std_Types.h"

/* ===================[External Configurations]=================== */
extern const Mcu_ConfigType Mcu_Config_80MHz;
extern const Gpio_ConfigType Gpio_Configuration;

/* ===================[Definitions]=================== */
#define ROS2_UART_MODULE        UART_MODULE_1   /* ROS 2 on UART1 (PB0/PB1) */
#define PC_UART_MODULE          UART_MODULE_0   /* PC via USB */
#define ROS2_BAUD_RATE          UART_BAUD_115200
#define PC_BAUD_RATE            UART_BAUD_115200

/* ===================[Global Variables]=================== */
static uint32 bytesFromPC = 0;
static uint32 bytesFromRPi = 0;

/* ===================[Interrupt Handlers]=================== */

/**
 * @brief Timer2A interrupt handler (dummy - not used in ROS 2 application)
 */
void Timer2A_Handler(void) {
    /* Not used in ROS 2 application */
}

/* ===================[Helper Functions]=================== */

/**
 * @brief Send string to PC UART
 */
void PC_Print(const uint8* str) {
    Uart_SendString(PC_UART_MODULE, str);
}

/**
 * @brief Convert uint32 to string
 */
void Uint32ToString(uint32 value, uint8* buffer) {
    uint8 i = 0;
    uint8 temp[12];
    uint8 j;
    
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    for (j = 0; j < i; j++) {
        buffer[j] = temp[i - 1 - j];
    }
    buffer[j] = '\0';
}

/**
 * @brief Forward data from PC to Raspberry Pi
 * @details Anything typed in serial monitor goes to RPi
 */
void ForwardPCToRPi(void) {
    uint8 receivedByte;
    Std_ReturnType status;
    
    while (Uart_IsRxDataAvailable(PC_UART_MODULE)) {
        status = Uart_ReceiveByte(PC_UART_MODULE, &receivedByte);
        if (status == E_OK) {
            /* Echo back to PC with proper line endings */
            if (receivedByte == '\r' || receivedByte == '\n') {
                /* Send CR+LF for proper line ending */
                Uart_SendByte(PC_UART_MODULE, '\r');
                Uart_SendByte(PC_UART_MODULE, '\n');
            } else {
                /* Echo character back to PC */
                Uart_SendByte(PC_UART_MODULE, receivedByte);
            }
            
            /* Send to Raspberry Pi */
            Uart_SendByte(ROS2_UART_MODULE, receivedByte);
            bytesFromPC++;
        }
    }
}

/**
 * @brief Forward data from Raspberry Pi to PC
 * @details Anything from RPi is displayed in serial monitor
 */
void ForwardRPiToPC(void) {
    uint8 receivedByte;
    Std_ReturnType status;
    
    while (Uart_IsRxDataAvailable(ROS2_UART_MODULE)) {
        status = Uart_ReceiveByte(ROS2_UART_MODULE, &receivedByte);
        if (status == E_OK) {
            /* Handle line endings from RPi */
            if (receivedByte == '\n') {
                /* Convert LF to CR+LF for proper display */
                Uart_SendByte(PC_UART_MODULE, '\r');
                Uart_SendByte(PC_UART_MODULE, '\n');
            } else if (receivedByte != '\r') {
                /* Display character (skip standalone CR) */
                Uart_SendByte(PC_UART_MODULE, receivedByte);
            }
            bytesFromRPi++;
        }
    }
}

/* ===================[Main Function]=================== */
int main(void) {
    uint32 loopCounter = 0;
    volatile uint32 delay;
    
    /* Step 1: Initialize MCU at 80MHz */
    Mcu_Init(&Mcu_Config_80MHz);
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    
    /* Wait for PLL to lock */
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) {
        /* Wait */
    }
    Mcu_DistributePllClock();
    
    /* Small delay for clock stabilization */
    for (delay = 0; delay < 50000; delay++);
    
    /* Step 2: Initialize GPIO */
    Gpio_Init(&Gpio_Configuration);
    
    /* Step 3: Configure PC UART (UART0 - USB to PC) */
    const Uart_ConfigType PcUart_Config = {
        .Module = PC_UART_MODULE,
        .BaudRate = PC_BAUD_RATE,
        .DataBits = UART_DATA_BITS_8,
        .Parity = UART_PARITY_NONE,
        .StopBits = UART_STOP_BITS_1,
        .FlowControl = UART_FLOW_CONTROL_NONE,
        .FifoEnable = TRUE,
        .RxInterruptEnable = FALSE,
        .TxInterruptEnable = FALSE,
        .RxCallback = NULL_PTR,
        .TxCallback = NULL_PTR
    };
    Uart_Init(&PcUart_Config);
    
    /* Step 4: Configure ROS 2 UART (UART1 - to Raspberry Pi) */
    const Uart_ConfigType ROS2Uart_Config = {
        .Module = ROS2_UART_MODULE,
        .BaudRate = ROS2_BAUD_RATE,
        .DataBits = UART_DATA_BITS_8,
        .Parity = UART_PARITY_NONE,
        .StopBits = UART_STOP_BITS_1,
        .FlowControl = UART_FLOW_CONTROL_NONE,
        .FifoEnable = TRUE,
        .RxInterruptEnable = FALSE,
        .TxInterruptEnable = FALSE,
        .RxCallback = NULL_PTR,
        .TxCallback = NULL_PTR
    };
    Uart_Init(&ROS2Uart_Config);
    
    /* Small delay for UART stability */
    {
        volatile uint32 delay;
        for (delay = 0; delay < 10000; delay++) { }
    }
    
    /* Send minimal startup message */
    PC_Print((const uint8*)"\r\n[TM4C] UART Bridge Ready\r\n");
    
    /* Main loop - Simple transparent bridge */
    while (1) {
        /* Forward PC → Raspberry Pi */
        ForwardPCToRPi();
        
        /* Forward Raspberry Pi → PC */
        ForwardRPiToPC();
        
        /* Optional: Print statistics every ~60 seconds (disabled by default) */
        #if 0
        if (loopCounter % 60000 == 0 && loopCounter > 0) {
            PC_Print((const uint8*)"\r\n[STATS] TX:");
            Uint32ToString(bytesFromPC, buffer);
            PC_Print(buffer);
            PC_Print((const uint8*)" RX:");
            Uint32ToString(bytesFromRPi, buffer);
            PC_Print(buffer);
            PC_Print((const uint8*)"\r\n");
        }
        #endif
        
        loopCounter++;
        
        /* Small delay to prevent CPU hogging */
        {
            volatile uint32 delay;
            for (delay = 0; delay < 100; delay++) { }
        }
    }
    
    /* Never reached - infinite loop above */
}

#endif  /* <<<< END OF COMMENTED OUT IMU TEST CODE >>>> */


