/**
 * @file NVIC_PBCfg.c
 * @brief NVIC Driver Post-Build Configuration for TM4C123GH6PM
 */

#include "NVIC.h"

/* ===================[Interrupt Configuration Table]=================== */
const NVIC_InterruptConfigType NVIC_InterruptConfigTable[] = {
    { .Vector = 23u, .Priority = 3u, .Enable = TRUE },   /* Timer2A interrupt */
    { .Vector = 5u,  .Priority = 2u, .Enable = TRUE }    /* UART0 interrupt */
};

const NVIC_ConfigType NVIC_Config = {
    .PriorityGrouping = NVIC_DEFAULT_PRIORITY_GROUP,
    .Interrupts = NVIC_InterruptConfigTable,
    .InterruptCount = (uint16)(sizeof(NVIC_InterruptConfigTable) / sizeof(NVIC_InterruptConfigType))
};
