/*
 * @file Uart.h
 * @brief UART Driver API for TM4C123GH6PM
 * @details This file contains the public API for the UART driver.
 *          Supports serial communication with configurable baud rates,
 *          data bits, parity, and stop bits.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_UART_UART_H_
#define MCAL_UART_UART_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"
#include "../../CONFIG/Det.h"
#include "Uart_Types.h"
#include "Uart_Cfg.h"

/* ===================[API Function Declarations]=================== */

/**
 * @brief Initialize UART driver
 * @details Initializes the specified UART module with the provided configuration.
 *          This function must be called before any other UART APIs.
 *
 * @param[in] ConfigPtr Pointer to UART configuration structure
 *
 * @pre UART module must not be initialized
 * @post UART is configured and ready for communication
 *
 * @error UART_E_PARAM_POINTER: ConfigPtr is NULL
 * @error UART_E_PARAM_CONFIG: Invalid configuration parameters
 * @error UART_E_ALREADY_INITIALIZED: UART already initialized
 *
 * @example
 * @code
 * const Uart_ConfigType uartConfig = {
 *     .Module = UART_MODULE_0,
 *     .BaudRate = UART_BAUD_115200,
 *     .DataBits = UART_DATA_BITS_8,
 *     .Parity = UART_PARITY_NONE,
 *     .StopBits = UART_STOP_BITS_1,
 *     .FifoEnable = TRUE
 * };
 * Uart_Init(&uartConfig);
 * @endcode
 */
void Uart_Init(const Uart_ConfigType* ConfigPtr);

/**
 * @brief De-initialize UART driver
 * @details Disables the specified UART module and clears its configuration.
 *
 * @param[in] Module UART module (UART_MODULE_0 to UART_MODULE_7)
 *
 * @pre UART must be initialized
 * @post UART is disabled and de-configured
 *
 * @error UART_E_PARAM_MODULE: Invalid UART module
 * @error UART_E_UNINIT: UART not initialized
 */
void Uart_DeInit(Uart_ModuleType Module);

/**
 * @brief Send a single byte via UART
 * @details Transmits one byte of data. Blocks until transmission is complete.
 *
 * @param[in] Module UART module (UART_MODULE_0 to UART_MODULE_7)
 * @param[in] Data Byte to transmit
 *
 * @return Std_ReturnType
 *         - E_OK: Transmission successful
 *         - E_NOT_OK: Transmission failed (timeout or error)
 *
 * @pre UART must be initialized
 *
 * @error UART_E_PARAM_MODULE: Invalid UART module
 * @error UART_E_UNINIT: UART not initialized
 */
Std_ReturnType Uart_SendByte(Uart_ModuleType Module, uint8 Data);

/**
 * @brief Receive a single byte via UART
 * @details Receives one byte of data. Blocks until data is available or timeout.
 *
 * @param[in] Module UART module (UART_MODULE_0 to UART_MODULE_7)
 * @param[out] DataPtr Pointer to store received byte
 *
 * @return Std_ReturnType
 *         - E_OK: Reception successful
 *         - E_NOT_OK: Reception failed (timeout or error)
 *
 * @pre UART must be initialized
 *
 * @error UART_E_PARAM_MODULE: Invalid UART module
 * @error UART_E_PARAM_POINTER: DataPtr is NULL
 * @error UART_E_UNINIT: UART not initialized
 */
Std_ReturnType Uart_ReceiveByte(Uart_ModuleType Module, uint8* DataPtr);

/**
 * @brief Send a null-terminated string via UART
 * @details Transmits a string until null terminator is reached.
 *
 * @param[in] Module UART module (UART_MODULE_0 to UART_MODULE_7)
 * @param[in] StringPtr Pointer to null-terminated string
 *
 * @return Std_ReturnType
 *         - E_OK: Transmission successful
 *         - E_NOT_OK: Transmission failed
 *
 * @pre UART must be initialized
 *
 * @error UART_E_PARAM_MODULE: Invalid UART module
 * @error UART_E_PARAM_POINTER: StringPtr is NULL
 * @error UART_E_UNINIT: UART not initialized
 */
Std_ReturnType Uart_SendString(Uart_ModuleType Module, const uint8* StringPtr);

/**
 * @brief Send a buffer via UART
 * @details Transmits specified number of bytes from buffer.
 *
 * @param[in] Module UART module (UART_MODULE_0 to UART_MODULE_7)
 * @param[in] BufferPtr Pointer to data buffer
 * @param[in] Length Number of bytes to transmit
 *
 * @return Std_ReturnType
 *         - E_OK: Transmission successful
 *         - E_NOT_OK: Transmission failed
 *
 * @pre UART must be initialized
 *
 * @error UART_E_PARAM_MODULE: Invalid UART module
 * @error UART_E_PARAM_POINTER: BufferPtr is NULL
 * @error UART_E_PARAM_VALUE: Length is 0
 * @error UART_E_UNINIT: UART not initialized
 */
Std_ReturnType Uart_SendBuffer(Uart_ModuleType Module, const uint8* BufferPtr, uint16 Length);

/**
 * @brief Get UART status
 * @details Returns the current status of the UART module.
 *
 * @param[in] Module UART module (UART_MODULE_0 to UART_MODULE_7)
 *
 * @return Uart_StateType Current UART state
 *
 * @pre UART must be initialized
 *
 * @error UART_E_PARAM_MODULE: Invalid UART module
 * @error UART_E_UNINIT: UART not initialized
 */
Uart_StateType Uart_GetStatus(Uart_ModuleType Module);

/**
 * @brief Check if transmit FIFO is empty
 * @details Useful for determining if all data has been transmitted.
 *
 * @param[in] Module UART module (UART_MODULE_0 to UART_MODULE_7)
 *
 * @return boolean
 *         - TRUE: TX FIFO is empty
 *         - FALSE: TX FIFO has data
 *
 * @pre UART must be initialized
 */
boolean Uart_IsTxFifoEmpty(Uart_ModuleType Module);

/**
 * @brief Check if receive FIFO has data
 * @details Useful for polling mode reception.
 *
 * @param[in] Module UART module (UART_MODULE_0 to UART_MODULE_7)
 *
 * @return boolean
 *         - TRUE: RX FIFO has data
 *         - FALSE: RX FIFO is empty
 *
 * @pre UART must be initialized
 */
boolean Uart_IsRxDataAvailable(Uart_ModuleType Module);

#if (UART_VERSION_INFO_API == STD_ON)
/**
 * @brief Get UART driver version information
 * @details Returns version information about the UART driver.
 *
 * @param[out] VersionInfo Pointer to store version information
 *
 * @pre None
 * @post Version information is stored in VersionInfo
 *
 * @error UART_E_PARAM_POINTER: VersionInfo is NULL
 */
void Uart_GetVersionInfo(Std_VersionInfoType* VersionInfo);
#endif

#endif /* MCAL_UART_UART_H_ */
