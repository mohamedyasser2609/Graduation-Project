/*
 * @file Uart_Types.h
 * @brief UART Type Definitions for TM4C123GH6PM
 * @details This file contains type definitions for the UART driver.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_UART_UART_TYPES_H_
#define MCAL_UART_UART_TYPES_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[UART Module Selection]=================== */
/**
 * @brief UART module identifiers
 * @details TM4C123GH6PM has 8 UART modules (0-7)
 */
typedef enum {
    UART_MODULE_0 = 0u,   /**< UART Module 0 */
    UART_MODULE_1 = 1u,   /**< UART Module 1 */
    UART_MODULE_2 = 2u,   /**< UART Module 2 */
    UART_MODULE_3 = 3u,   /**< UART Module 3 */
    UART_MODULE_4 = 4u,   /**< UART Module 4 */
    UART_MODULE_5 = 5u,   /**< UART Module 5 */
    UART_MODULE_6 = 6u,   /**< UART Module 6 */
    UART_MODULE_7 = 7u    /**< UART Module 7 */
} Uart_ModuleType;

/* ===================[Baud Rate Selection]=================== */
/**
 * @brief Standard baud rates
 */
typedef enum {
    UART_BAUD_9600    = 9600u,      /**< 9600 bps */
    UART_BAUD_19200   = 19200u,     /**< 19200 bps */
    UART_BAUD_38400   = 38400u,     /**< 38400 bps */
    UART_BAUD_57600   = 57600u,     /**< 57600 bps */
    UART_BAUD_115200  = 115200u,    /**< 115200 bps (most common) */
    UART_BAUD_230400  = 230400u,    /**< 230400 bps */
    UART_BAUD_460800  = 460800u,    /**< 460800 bps */
    UART_BAUD_921600  = 921600u     /**< 921600 bps */
} Uart_BaudRateType;

/* ===================[Data Bits Selection]=================== */
/**
 * @brief Data bits configuration
 */
typedef enum {
    UART_DATA_BITS_5 = 0u,          /**< 5 data bits */
    UART_DATA_BITS_6 = 1u,          /**< 6 data bits */
    UART_DATA_BITS_7 = 2u,          /**< 7 data bits */
    UART_DATA_BITS_8 = 3u           /**< 8 data bits (most common) */
} Uart_DataBitsType;

/* ===================[Parity Selection]=================== */
/**
 * @brief Parity configuration
 */
typedef enum {
    UART_PARITY_NONE = 0u,          /**< No parity */
    UART_PARITY_ODD  = 1u,          /**< Odd parity */
    UART_PARITY_EVEN = 2u,          /**< Even parity */
    UART_PARITY_MARK = 3u,          /**< Mark parity (always 1) */
    UART_PARITY_SPACE = 4u          /**< Space parity (always 0) */
} Uart_ParityType;

/* ===================[Stop Bits Selection]=================== */
/**
 * @brief Stop bits configuration
 */
typedef enum {
    UART_STOP_BITS_1 = 0u,          /**< 1 stop bit */
    UART_STOP_BITS_2 = 1u           /**< 2 stop bits */
} Uart_StopBitsType;

/* ===================[Flow Control]=================== */
/**
 * @brief Hardware flow control
 */
typedef enum {
    UART_FLOW_CONTROL_NONE = 0u,    /**< No flow control */
    UART_FLOW_CONTROL_RTS  = 1u,    /**< RTS only */
    UART_FLOW_CONTROL_CTS  = 2u,    /**< CTS only */
    UART_FLOW_CONTROL_RTS_CTS = 3u  /**< RTS and CTS */
} Uart_FlowControlType;

/* ===================[UART State]=================== */
/**
 * @brief UART driver state
 */
typedef enum {
    UART_STATE_UNINIT = 0u,         /**< UART not initialized */
    UART_STATE_IDLE = 1u,           /**< UART idle, ready for operation */
    UART_STATE_BUSY = 2u            /**< UART busy transmitting/receiving */
} Uart_StateType;

/* ===================[UART Error Codes]=================== */
/**
 * @brief UART error flags
 */
typedef enum {
    UART_ERROR_NONE    = 0x00u,     /**< No error */
    UART_ERROR_FRAMING = 0x01u,     /**< Framing error */
    UART_ERROR_PARITY  = 0x02u,     /**< Parity error */
    UART_ERROR_BREAK   = 0x04u,     /**< Break error */
    UART_ERROR_OVERRUN = 0x08u      /**< Overrun error */
} Uart_ErrorType;

/* ===================[Notification Function Pointer]=================== */
/**
 * @brief Function pointer type for UART notification callbacks
 */
typedef void (*Uart_NotificationFuncPtr)(void);

/* ===================[UART Configuration Structure]=================== */
/**
 * @brief UART configuration structure
 * @details Contains all configuration parameters for a UART module
 */
typedef struct {
    Uart_ModuleType         Module;             /**< UART module (0-7) */
    Uart_BaudRateType       BaudRate;           /**< Baud rate */
    Uart_DataBitsType       DataBits;           /**< Data bits (5-8) */
    Uart_ParityType         Parity;             /**< Parity mode */
    Uart_StopBitsType       StopBits;           /**< Stop bits (1 or 2) */
    Uart_FlowControlType    FlowControl;        /**< Hardware flow control */
    boolean                 FifoEnable;         /**< Enable FIFO */
    boolean                 RxInterruptEnable;  /**< Enable RX interrupt */
    boolean                 TxInterruptEnable;  /**< Enable TX interrupt */
    Uart_NotificationFuncPtr RxCallback;        /**< RX callback function */
    Uart_NotificationFuncPtr TxCallback;        /**< TX callback function */
} Uart_ConfigType;

#endif /* MCAL_UART_UART_TYPES_H_ */
