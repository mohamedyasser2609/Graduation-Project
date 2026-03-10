/**
 * @file Diag_PBCfg.c
 * @brief Post-build configuration for Diagnostics Service
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "Diagnostics.h"

/* ===================[Service Configuration]=================== */
const Diag_ConfigType Diag_Config =
{
    .DebugUartModule = UART_MODULE_0,  /* UART0 for debug output */
    .DebugEnabled    = TRUE,    /* Enable debug printing */
    .EventLogEnabled = TRUE     /* Enable event logging */
};
