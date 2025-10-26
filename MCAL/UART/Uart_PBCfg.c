/*
 * @file Uart_PBCfg.c
 * @brief UART Post-Build Configuration for TM4C123GH6PM
 * @details Example UART configurations for common use cases.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

/* ===================[Includes]=================== */
#include "Uart.h"
#include "Uart_Cfg.h"

/* ===================[Example Configurations]=================== */

/**
 * @brief Example 1: UART0 - 115200 baud, 8N1, FIFO enabled
 * @details Most common configuration for debugging and serial console
 */
const Uart_ConfigType Uart0_Config_115200 = {
    .Module = UART_MODULE_0,
    .BaudRate = UART_BAUD_115200,
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

/**
 * @brief Example 2: UART1 - 9600 baud, 8N1
 * @details Low-speed communication (GPS, some sensors)
 */
const Uart_ConfigType Uart1_Config_9600 = {
    .Module = UART_MODULE_1,
    .BaudRate = UART_BAUD_9600,
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

/**
 * @brief Example 3: UART0 - 115200 baud, 8E1 (Even parity)
 * @details Configuration with parity checking
 */
const Uart_ConfigType Uart0_Config_Parity = {
    .Module = UART_MODULE_0,
    .BaudRate = UART_BAUD_115200,
    .DataBits = UART_DATA_BITS_8,
    .Parity = UART_PARITY_EVEN,
    .StopBits = UART_STOP_BITS_1,
    .FlowControl = UART_FLOW_CONTROL_NONE,
    .FifoEnable = TRUE,
    .RxInterruptEnable = FALSE,
    .TxInterruptEnable = FALSE,
    .RxCallback = NULL_PTR,
    .TxCallback = NULL_PTR
};

/**
 * @brief Example 4: UART2 - 38400 baud with interrupts
 * @details Configuration for interrupt-driven communication
 */
const Uart_ConfigType Uart2_Config_Interrupt = {
    .Module = UART_MODULE_2,
    .BaudRate = UART_BAUD_38400,
    .DataBits = UART_DATA_BITS_8,
    .Parity = UART_PARITY_NONE,
    .StopBits = UART_STOP_BITS_1,
    .FlowControl = UART_FLOW_CONTROL_NONE,
    .FifoEnable = TRUE,
    .RxInterruptEnable = TRUE,
    .TxInterruptEnable = FALSE,
    .RxCallback = NULL_PTR,  /* User provides callback */
    .TxCallback = NULL_PTR
};

/* ===================[Usage Notes]=================== */
/*
 * To use these configurations:
 *
 * 1. Initialize UART:
 *    Uart_Init(&Uart0_Config_115200);
 *
 * 2. Send a string:
 *    Uart_SendString(UART_MODULE_0, (const uint8*)"Hello World!\r\n");
 *
 * 3. Send a byte:
 *    Uart_SendByte(UART_MODULE_0, 'A');
 *
 * 4. Receive a byte:
 *    uint8 data;
 *    if (Uart_ReceiveByte(UART_MODULE_0, &data) == E_OK) {
 *        // Data received successfully
 *    }
 *
 * 5. Check if data available:
 *    if (Uart_IsRxDataAvailable(UART_MODULE_0)) {
 *        // Read data
 *    }
 */
