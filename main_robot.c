/**
 * @file main_robot.c
 * @brief Production Entry Point — FreeRTOS Robot Controller
 * @details Production main for the UGV robot with full safety infrastructure.
 *
 * Boot sequence:
 *   1. MCU Init → 80MHz PLL
 *   2. System_Init() → all MCAL/ECUAL/SERVICES in dependency order
 *   3. Tasks_Init() → FreeRTOS tasks + scheduler (never returns)
 *
 * This replaces main_slam_test.c (bare super-loop) with the full FreeRTOS
 * task architecture including:
 *   - Safety Task (P4): WDG feed, current/thermal monitoring, heartbeat
 *   - Control Task (P3): PID motor control via Robot_Control
 *   - Sensor Task (P2): IMU/Encoder reads, odometry
 *   - Comm Task (P1): ComStack binary protocol ↔ ROS2
 *   - Thermal Task (P1): Fan control, temperature management
 *
 * @author Mohamed Yasser
 * @date Apr 13, 2026
 * @version 1.0.0
 */

#if 1  /* Production main — FreeRTOS ACTIVE */

/* ===================[Includes]=================== */
#include "MCAL/MCU/Mcu.h"
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/UART/Uart.h"

#include "SERVICES/SYSTEM/System_Services.h"
#include "SERVICES/RTOS/Tasks_Init.h"
#include "SERVICES/DIAG/Diagnostics.h"

#include "CONFIG/Std_Types.h"

/* ===================[External Configurations]=================== */
extern const Mcu_ConfigType* Mcu_ConfigPtr;
extern const Gpio_ConfigType Gpio_Configuration;

/* ===================[Timer2A Handler]=================== */
/**
 * @brief Timer2A handler (required by startup vector table)
 * @details In FreeRTOS mode, Timer2A is not used for LED cycling.
 *          This handler is a fault trap — if Timer2A fires unexpectedly
 *          it halts to aid debugging.
 */
void Timer2A_Handler(void)
{
    /* Unexpected interrupt — halt for debug */
    while(1);
}

/* ===================[Helper]=================== */
static void Boot_PrintString(const char* str)
{
    Uart_SendString(UART_MODULE_0, (const uint8*)str);
}

/* ===================[Main Function]=================== */
int main(void)
{
    /* ===== Phase 1: MCU Clock — 80MHz PLL ===== */
    Mcu_Init(Mcu_ConfigPtr);
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) { }
    Mcu_DistributePllClock();

    /* Brief stabilization delay */
    {
        volatile uint32 i;
        for (i = 0u; i < 100000u; i++) { }
    }

    /* ===== Phase 2: Early UART0 for boot diagnostics ===== */
    /* GPIO must be initialized first for UART pins */
    Gpio_Init(&Gpio_Configuration);

    {
        const Uart_ConfigType BootUart0 = {
            .Module          = UART_MODULE_0,
            .BaudRate        = UART_BAUD_115200,
            .DataBits        = UART_DATA_BITS_8,
            .Parity          = UART_PARITY_NONE,
            .StopBits        = UART_STOP_BITS_1,
            .FlowControl     = UART_FLOW_CONTROL_NONE,
            .FifoEnable      = TRUE,
            .RxInterruptEnable = FALSE,
            .TxInterruptEnable = FALSE,
            .RxCallback      = NULL_PTR,
            .TxCallback      = NULL_PTR
        };
        Uart_Init(&BootUart0);
    }

    Boot_PrintString("\r\n");
    Boot_PrintString("========================================\r\n");
    Boot_PrintString(" UGV Robot Controller v1.0\r\n");
    Boot_PrintString(" TM4C123GH6PM + FreeRTOS + ROS2\r\n");
    Boot_PrintString("========================================\r\n");
    Boot_PrintString("[BOOT] MCU @ 80MHz — PLL locked\r\n");

    /* ===== Phase 3: Full system initialization ===== */
    Boot_PrintString("[BOOT] System_Init()...\r\n");
    System_Init();
    Boot_PrintString("[BOOT] System_Init() complete\r\n");

    /* UART1 for ROS2 is initialized inside System_Init() via Uart_Config */

    /* ===== Phase 4: Start FreeRTOS ===== */
    Boot_PrintString("[BOOT] Tasks_Init() — starting FreeRTOS scheduler\r\n");
    Boot_PrintString("[BOOT] Tasks: Safety(P4) Control(P3) Sensor(P2) Comm(P1) Thermal(P1)\r\n");
    Boot_PrintString("========================================\r\n\r\n");

    Tasks_Init();  /* Never returns — scheduler starts here */

    /* ===== Should never reach here ===== */
    Boot_PrintString("[FATAL] Scheduler returned!\r\n");
    for (;;) { }
}

#endif  /* Production main guard */
