/**
 * @file ComStack_Cfg.h
 * @brief Configuration header for Communication Stack Service
 * @details Pre-compile configuration parameters
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef COMSTACK_CFG_H
#define COMSTACK_CFG_H

/* Include UART types for Uart_ModuleType enum */
#include "../../MCAL/UART/Uart_Types.h"

/* ===================[Pre-compile Options]=================== */

/**
 * @brief Enable/disable development error detection
 * @range STD_ON, STD_OFF
 */
#define COMSTACK_DEV_ERROR_DETECT       (STD_ON)

/**
 * @brief Enable/disable version info API
 * @range STD_ON, STD_OFF
 */
#define COMSTACK_VERSION_INFO_API       (STD_ON)

/**
 * @brief Enable/disable de-initialization API
 * @range STD_ON, STD_OFF
 */
#define COMSTACK_DEINIT_API             (STD_ON)

/**
 * @brief Enable/disable receive callback
 * @range STD_ON, STD_OFF
 */
#define COMSTACK_RX_CALLBACK_API        (STD_ON)

/**
 * @brief Enable/disable transmission callback
 * @range STD_ON, STD_OFF
 */
#define COMSTACK_TX_CALLBACK_API        (STD_ON)

/**
 * @brief Enable/disable auto-acknowledge feature
 * @range STD_ON, STD_OFF
 */
#define COMSTACK_AUTO_ACK               (STD_ON)

/**
 * @brief Default UART module for ROS2 communication
 */
#define COMSTACK_DEFAULT_UART_MODULE    UART_MODULE_1    /* UART1: PB0/PB1 */

/**
 * @brief Default receive timeout in milliseconds
 */
#define COMSTACK_DEFAULT_RX_TIMEOUT_MS  (100u)

/**
 * @brief Default number of transmit retries
 */
#define COMSTACK_DEFAULT_TX_RETRIES     (3u)

/**
 * @brief RX buffer size (number of packets)
 */
#define COMSTACK_RX_QUEUE_SIZE          (4u)

/**
 * @brief TX buffer size (number of packets)
 */
#define COMSTACK_TX_QUEUE_SIZE          (4u)

/**
 * @brief Heartbeat/ping interval in milliseconds
 */
#define COMSTACK_HEARTBEAT_INTERVAL_MS  (1000u)

/**
 * @brief Connection timeout in milliseconds
 */
#define COMSTACK_CONNECTION_TIMEOUT_MS  (5000u)

#endif /* COMSTACK_CFG_H */
