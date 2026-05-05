/**
 * @file ComStack_PBCfg.c
 * @brief Post-build configuration for Communication Stack Service
 * @details Configuration for ROS2 UART communication
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "ComStack.h"

/* ===================[Service Configuration]=================== */
const ComStack_ConfigType ComStack_Config =
{
    .UartModule     = COMSTACK_DEFAULT_UART_MODULE,  /* UART1 (PB0/PB1) */
    .RxTimeoutMs    = COMSTACK_DEFAULT_RX_TIMEOUT_MS,
    .TxRetries      = COMSTACK_DEFAULT_TX_RETRIES,
    .AutoAck        = TRUE
};
