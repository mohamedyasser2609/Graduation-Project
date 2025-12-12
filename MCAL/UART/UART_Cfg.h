/*
 * @file Uart_Cfg.h
 * @brief UART Configuration Parameters for TM4C123GH6PM
 * @details This file contains compile-time configuration parameters for the UART driver.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_UART_UART_CFG_H_
#define MCAL_UART_UART_CFG_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[Module Identification]=================== */
/**
 * @brief UART module and vendor identification
 */
#define UART_VENDOR_ID                      (0x1234u)
#define UART_MODULE_ID                      (122u)
#define UART_INSTANCE_ID                    (0u)

/* ===================[Development Error Detection]=================== */
/**
 * @brief Enable/disable development error detection
 * @details When enabled, API functions perform parameter validation and report errors to DET
 * Values: STD_ON (enabled) / STD_OFF (disabled)
 */
#define UART_DEV_ERROR_DETECT               STD_ON

/* ===================[Version Info API]=================== */
/**
 * @brief Enable/disable version information API
 * @details When enabled, Uart_GetVersionInfo() API is available
 * Values: STD_ON (enabled) / STD_OFF (disabled)
 */
#define UART_VERSION_INFO_API               STD_ON

/* ===================[Version Information]=================== */
/**
 * @brief UART driver version information
 */
#define UART_SW_MAJOR_VERSION               (1u)
#define UART_SW_MINOR_VERSION               (0u)
#define UART_SW_PATCH_VERSION               (0u)

/**
 * @brief AUTOSAR version information
 */
#define UART_AR_RELEASE_MAJOR_VERSION       (4u)
#define UART_AR_RELEASE_MINOR_VERSION       (4u)
#define UART_AR_RELEASE_PATCH_VERSION       (0u)

/* ===================[DET Error Codes]=================== */
/**
 * @brief Development Error Tracer error codes for UART driver
 */
#define UART_E_PARAM_POINTER                (0x01u)  /**< NULL pointer passed to API */
#define UART_E_PARAM_CONFIG                 (0x02u)  /**< Invalid configuration parameter */
#define UART_E_PARAM_MODULE                 (0x03u)  /**< Invalid UART module */
#define UART_E_UNINIT                       (0x04u)  /**< API called before initialization */
#define UART_E_ALREADY_INITIALIZED          (0x05u)  /**< UART already initialized */
#define UART_E_PARAM_VALUE                  (0x06u)  /**< Invalid parameter value */
#define UART_E_BUSY                         (0x07u)  /**< UART is busy */

/* ===================[API Service IDs]=================== */
/**
 * @brief Service IDs for UART driver APIs (for DET reporting)
 */
#define UART_INIT_SID                       (0x00u)  /**< Uart_Init() */
#define UART_DEINIT_SID                     (0x01u)  /**< Uart_DeInit() */
#define UART_SEND_BYTE_SID                  (0x02u)  /**< Uart_SendByte() */
#define UART_RECEIVE_BYTE_SID               (0x03u)  /**< Uart_ReceiveByte() */
#define UART_SEND_STRING_SID                (0x04u)  /**< Uart_SendString() */
#define UART_SEND_BUFFER_SID                (0x05u)  /**< Uart_SendBuffer() */
#define UART_GET_STATUS_SID                 (0x06u)  /**< Uart_GetStatus() */
#define UART_GET_VERSION_INFO_SID           (0x07u)  /**< Uart_GetVersionInfo() */

/* ===================[System Clock Configuration]=================== */
/**
 * @brief System clock frequency (DEPRECATED - No longer used)
 * @details The UART driver now dynamically queries the system clock from
 *          Mcu_GetSystemClock() at initialization time. This ensures the
 *          baud rate is always correct regardless of clock frequency.
 * @note This define is kept for backward compatibility but is NOT used.
 *       The driver automatically adapts to any system clock frequency!
 */
#define UART_SYSTEM_CLOCK_HZ                (80000000UL)  /* DEPRECATED - Not used */

/* ===================[Buffer Sizes]=================== */
/**
 * @brief Maximum string length for Uart_SendString()
 */
#define UART_MAX_STRING_LENGTH              (256u)

/**
 * @brief Transmit timeout in microseconds
 */
#define UART_TX_TIMEOUT_US                  (10000u)  /* 10ms */

/**
 * @brief Receive timeout in microseconds
 */
#define UART_RX_TIMEOUT_US                  (10000u)  /* 10ms */

#endif /* MCAL_UART_UART_CFG_H_ */
