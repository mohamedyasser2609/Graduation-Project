//*****************************************************************************
//
// Startup code for use with TI's Code Composer Studio.
//
// Copyright (c) 2011-2014 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
//*****************************************************************************

#include <stdint.h>

//*****************************************************************************
//
// Forward declaration of the default fault handlers.
//
//*****************************************************************************
void ResetISR(void);
static void NmiSR(void);
static void FaultISR(void);
static void IntDefaultHandler(void);

//*****************************************************************************
//
// External declaration for the reset handler that is to be called when the
// processor is started
//
//*****************************************************************************
extern void _c_int00(void);

//*****************************************************************************
//
// Linker variable that marks the top of the stack.
//
//*****************************************************************************
extern uint32_t __STACK_TOP;

//*****************************************************************************
//
// External declarations for the interrupt handlers used by the application.
//
//*****************************************************************************
// Timer interrupt handler for auto-cycling
extern void Timer2A_Handler(void);
// ADC interrupt handlers
extern void Adc_ADC0SS0_ISR(void);
extern void Adc_ADC0SS1_ISR(void);
extern void Adc_ADC0SS2_ISR(void);
extern void Adc_ADC0SS3_ISR(void);
extern void Adc_ADC1SS0_ISR(void);
extern void Adc_ADC1SS1_ISR(void);
extern void Adc_ADC1SS2_ISR(void);
extern void Adc_ADC1SS3_ISR(void);
// MPU MemManage fault handler
extern void MemManage_Handler(void);
// Bus fault handler
static void BusFault_Handler(void);
// Usage fault handler
static void UsageFault_Handler(void);
// Watchdog interrupt handler

extern void Wdg_Watchdog0Handler(void);

//*****************************************************************************
//
// FreeRTOS Cortex-M4 port handlers.
// These MUST be in the vector table for FreeRTOS context switching to work.
// Without them, the first vTaskStartScheduler() call triggers a HardFault.
//
//*****************************************************************************
extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);

//*****************************************************************************
//
// The vector table.  Note that the proper constructs must be placed on this to
// ensure that it ends up at physical address 0x0000.0000 or at the start of
// the program if located at a start address other than 0.
//
//*****************************************************************************
#pragma DATA_SECTION(g_pfnVectors, ".intvecs")
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))0x20008000,             // The initial stack pointer (Hard-coded for 32KB RAM)
    ResetISR,                               // The reset handler

    NmiSR,                                  // The NMI handler
    FaultISR,                               // The hard fault handler
    MemManage_Handler,                      // The MPU fault handler
    BusFault_Handler,                       // The bus fault handler
    UsageFault_Handler,                     // The usage fault handler

    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    vPortSVCHandler,                        // SVCall handler — FreeRTOS
    IntDefaultHandler,                      // Debug monitor handler
    0,                                      // Reserved
    xPortPendSVHandler,                     // The PendSV handler — FreeRTOS
    xPortSysTickHandler,                    // The SysTick handler — FreeRTOS
    IntDefaultHandler,                      // GPIO Port A
    IntDefaultHandler,                      // GPIO Port B
    IntDefaultHandler,                      // GPIO Port C
    IntDefaultHandler,                      // GPIO Port D
    IntDefaultHandler,                      // GPIO Port E
    IntDefaultHandler,                      // UART0 Rx and Tx
    IntDefaultHandler,                      // UART1 Rx and Tx
    IntDefaultHandler,                      // SSI0 Rx and Tx
    IntDefaultHandler,                      // I2C0 Master and Slave
    IntDefaultHandler,                      // PWM Fault
    IntDefaultHandler,                      // PWM Generator 0
    IntDefaultHandler,                      // PWM Generator 1
    IntDefaultHandler,                      // PWM Generator 2
    IntDefaultHandler,                      // Quadrature Encoder 0
    Adc_ADC0SS0_ISR,                        // ADC Sequence 0
    Adc_ADC0SS1_ISR,                        // ADC Sequence 1
    Adc_ADC0SS2_ISR,                        // ADC Sequence 2
    Adc_ADC0SS3_ISR,                        // ADC Sequence 3
    Wdg_Watchdog0Handler,                   // Watchdog timer
    IntDefaultHandler,                      // Timer 0 subtimer A
    IntDefaultHandler,                      // Timer 0 subtimer B
    IntDefaultHandler,                      // Timer 1 subtimer A
    IntDefaultHandler,                      // Timer 1 subtimer B
    Timer2A_Handler,                        // Timer 2 subtimer A
    IntDefaultHandler,                      // Timer 2 subtimer B
    IntDefaultHandler,                      // Analog Comparator 0
    IntDefaultHandler,                      // Analog Comparator 1
    IntDefaultHandler,                      // Analog Comparator 2
    IntDefaultHandler,                      // System Control (PLL, OSC, BO)
    IntDefaultHandler,                      // FLASH Control
    IntDefaultHandler,                      // GPIO Port F
    IntDefaultHandler,                      // GPIO Port G
    IntDefaultHandler,                      // GPIO Port H
    IntDefaultHandler,                      // UART2 Rx and Tx
    IntDefaultHandler,                      // SSI1 Rx and Tx
    IntDefaultHandler,                      // Timer 3 subtimer A
    IntDefaultHandler,                      // Timer 3 subtimer B
    IntDefaultHandler,                      // I2C1 Master and Slave
    IntDefaultHandler,                      // Quadrature Encoder 1
    IntDefaultHandler,                      // CAN0
    IntDefaultHandler,                      // CAN1
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // Hibernate
    IntDefaultHandler,                      // USB0
    IntDefaultHandler,                      // PWM Generator 3
    IntDefaultHandler,                      // uDMA Software Transfer
    IntDefaultHandler,                      // uDMA Error
    Adc_ADC1SS0_ISR,                        // ADC1 Sequence 0
    Adc_ADC1SS1_ISR,                        // ADC1 Sequence 1
    Adc_ADC1SS2_ISR,                        // ADC1 Sequence 2
    Adc_ADC1SS3_ISR,                        // ADC1 Sequence 3
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // GPIO Port J
    IntDefaultHandler,                      // GPIO Port K
    IntDefaultHandler,                      // GPIO Port L
    IntDefaultHandler,                      // SSI2 Rx and Tx
    IntDefaultHandler,                      // SSI3 Rx and Tx
    IntDefaultHandler,                      // UART3 Rx and Tx
    IntDefaultHandler,                      // UART4 Rx and Tx
    IntDefaultHandler,                      // UART5 Rx and Tx
    IntDefaultHandler,                      // UART6 Rx and Tx
    IntDefaultHandler,                      // UART7 Rx and Tx
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // I2C2 Master and Slave
    IntDefaultHandler,                      // I2C3 Master and Slave
    IntDefaultHandler,                      // Timer 4 subtimer A
    IntDefaultHandler,                      // Timer 4 subtimer B
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // Timer 5 subtimer A
    IntDefaultHandler,                      // Timer 5 subtimer B
    IntDefaultHandler,                      // Wide Timer 0 subtimer A
    IntDefaultHandler,                      // Wide Timer 0 subtimer B
    IntDefaultHandler,                      // Wide Timer 1 subtimer A
    IntDefaultHandler,                      // Wide Timer 1 subtimer B
    IntDefaultHandler,                      // Wide Timer 2 subtimer A
    IntDefaultHandler,                      // Wide Timer 2 subtimer B
    IntDefaultHandler,                      // Wide Timer 3 subtimer A
    IntDefaultHandler,                      // Wide Timer 3 subtimer B
    IntDefaultHandler,                      // Wide Timer 4 subtimer A
    IntDefaultHandler,                      // Wide Timer 4 subtimer B
    IntDefaultHandler,                      // Wide Timer 5 subtimer A
    IntDefaultHandler,                      // Wide Timer 5 subtimer B
    IntDefaultHandler,                      // FPU
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // I2C4 Master and Slave
    IntDefaultHandler,                      // I2C5 Master and Slave
    IntDefaultHandler,                      // GPIO Port M
    IntDefaultHandler,                      // GPIO Port N
    IntDefaultHandler,                      // Quadrature Encoder 2
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // GPIO Port P (Summary or P0)
    IntDefaultHandler,                      // GPIO Port P1
    IntDefaultHandler,                      // GPIO Port P2
    IntDefaultHandler,                      // GPIO Port P3
    IntDefaultHandler,                      // GPIO Port P4
    IntDefaultHandler,                      // GPIO Port P5
    IntDefaultHandler,                      // GPIO Port P6
    IntDefaultHandler,                      // GPIO Port P7
    IntDefaultHandler,                      // GPIO Port Q (Summary or Q0)
    IntDefaultHandler,                      // GPIO Port Q1
    IntDefaultHandler,                      // GPIO Port Q2
    IntDefaultHandler,                      // GPIO Port Q3
    IntDefaultHandler,                      // GPIO Port Q4
    IntDefaultHandler,                      // GPIO Port Q5
    IntDefaultHandler,                      // GPIO Port Q6
    IntDefaultHandler,                      // GPIO Port Q7
    IntDefaultHandler,                      // GPIO Port R
    IntDefaultHandler,                      // GPIO Port S
    IntDefaultHandler,                      // PWM 1 Generator 0
    IntDefaultHandler,                      // PWM 1 Generator 1
    IntDefaultHandler,                      // PWM 1 Generator 2
    IntDefaultHandler,                      // PWM 1 Generator 3
    IntDefaultHandler                       // PWM 1 Fault
};

//*****************************************************************************
//
// This is the code that gets called when the processor first starts execution
// following a reset event.  Only the absolutely necessary set is performed,
// after which the application supplied entry() routine is called.  Any fancy
// actions (such as making decisions based on the reset cause register, and
// resetting the bits in that register) are left solely in the hands of the
// application.
//
//*****************************************************************************
void
ResetISR(void)
{
    //
    // Jump to the CCS C initialization routine.  This will enable the
    // floating-point unit as well, so that does not need to be done here.
    //
    __asm("    .global _c_int00\n"
          "    b.w     _c_int00");
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives a NMI.  This
// simply enters an infinite loop, preserving the system state for examination
// by a debugger.
//
//*****************************************************************************
#include "../../APP/Common/App_ResourceMap.h"
#include "../../MCAL/UART/Uart.h"
#include "../../SERVICES/DIAG/Diagnostics.h"
#include "Tasks_Init.h"
static void
NmiSR(void)
{
    //
    // Enter an infinite loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
// Raw UART0 print for crash diagnostics (no driver dependency)
//
//*****************************************************************************
static volatile uint32_t* const UART0_DR = (volatile uint32_t*)0x4000C000;
static volatile uint32_t* const UART0_FR = (volatile uint32_t*)0x4000C018;

static void CrashPrint(const char* msg)
{
    while (*msg != '\0')

    {
        while ((*UART0_FR) & 0x20) {}  // Wait until TX FIFO not full
        *UART0_DR = (uint32_t)*msg;
        msg++;
    }
}


__attribute__((used))
static void
DumpFaultRegisters(uint32_t *stacked_regs)
{
    uint32_t cfsr = *((volatile uint32_t*)0xE000ED28);
    uint32_t bfar = *((volatile uint32_t*)0xE000ED38);
    
    uint32_t pc = stacked_regs[6];
    uint32_t lr = stacked_regs[5];
    int i;
    
    CrashPrint("\r\n--- Fault Context ---\r\n");
    CrashPrint("PC:   0x");
    for(i=7; i>=0; i--) {
        uint32_t nibble = (pc >> (i*4)) & 0xF;
        while ((*UART0_FR) & 0x20);
        *UART0_DR = (nibble < 10) ? (nibble + '0') : (nibble - 10 + 'A');
    }
    CrashPrint("\r\nLR:   0x");
    for(i=7; i>=0; i--) {
        uint32_t nibble = (lr >> (i*4)) & 0xF;
        while ((*UART0_FR) & 0x20);
        *UART0_DR = (nibble < 10) ? (nibble + '0') : (nibble - 10 + 'A');
    }
    CrashPrint("\r\nBFAR: 0x");
    for(i=7; i>=0; i--) {
        uint32_t nibble = (bfar >> (i*4)) & 0xF;
        while ((*UART0_FR) & 0x20);
        *UART0_DR = (nibble < 10) ? (nibble + '0') : (nibble - 10 + 'A');
    }
    CrashPrint("\r\nCFSR: 0x");
    for(i=7; i>=0; i--) {
        uint32_t nibble = (cfsr >> (i*4)) & 0xF;
        while ((*UART0_FR) & 0x20);
        *UART0_DR = (nibble < 10) ? (nibble + '0') : (nibble - 10 + 'A');
    }
    CrashPrint("\r\n--------------------\r\n");
    while(1);
}

void FaultISR(void)
{
    __asm("    tst lr, #4\n"
          "    mrs r0, msp\n"
          "    beq _skip_psp_1\n"
          "    mrs r0, psp\n"
          "_skip_psp_1:\n"
          "    b DumpFaultRegisters");
}

void BusFault_Handler(void)
{
    __asm("    tst lr, #4\n"
          "    mrs r0, msp\n"
          "    beq _skip_psp_2\n"
          "    mrs r0, psp\n"
          "_skip_psp_2:\n"
          "    b DumpFaultRegisters");
}

void UsageFault_Handler(void)
{
    __asm("    tst lr, #4\n"
          "    mrs r0, msp\n"
          "    beq _skip_psp_3\n"
          "    mrs r0, psp\n"
          "_skip_psp_3:\n"
          "    b DumpFaultRegisters");
}






static void
IntDefaultHandler(void)
{
    CrashPrint("\r\n[FAULT] Unexpected IRQ!\r\n");
    while(1) {}
}

