/**
 * @file System_Init.c
 * @brief System Initialization
 * @details Initializes all drivers and services in correct dependency order
 *
 * Initialization Order:
 * 1. MCAL Layer (Hardware drivers)
 * 2. ECUAL Layer (Device drivers)
 * 3. SERVICES Layer (Application services)
 * 4. APP Layer (Application modules)
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "System_Services.h"
#include "../../CONFIG/System_FeatureFlags.h"

/* ===================[MCAL Includes]=================== */
#include "../../MCAL/GPIO/Gpio.h"
#include "../../MCAL/UART/Uart.h"
#include "../../MCAL/Timer/Timer.h"
#include "../../MCAL/PWM/PWM.h"
#include "../../MCAL/I2C/I2C.h"
#include "../../MCAL/ADC/Adc.h"
#include "../../MCAL/QEI/Qei.h"
#include "../../MCAL/WDG/Wdg.h"
#include "../../MCAL/MPU/MPU.h"

/* ===================[ECUAL Includes]=================== */
#include "../../CONFIG/System_FeatureFlags.h"
#include "../../ECUAL/IMU/IMU.h"
#if (FEATURE_GPS_ENABLED == 1u)
#include "../../ECUAL/GPS/GPS.h"
#endif
#include "../../ECUAL/ENCODER/Encoder.h"
#include "../../ECUAL/MOTOR/Motor.h"
#include "../../ECUAL/CURRENT_SENSOR/ACS712.h"
#include "../../ECUAL/TEMP_SENSOR/AM2320.h"
#include "../../ECUAL/FAN/FAN.h"

/* ===================[SERVICES Includes]=================== */
#include "../COMM/ComStack.h"
#include "../THERMAL/ThermalMgmt.h"
#include "../DIAG/Diagnostics.h"
#include "../PID/PID.h"
#include "../TIME/TimeSync.h"

/* ===================[APP Includes]=================== */
#include "../../APP/Control/Robot_Control.h"

/* ===================[External Configurations]=================== */
/* MCAL Configurations */
extern const Gpio_ConfigType Gpio_Configuration;
extern const Uart_ConfigType Uart0_Config_115200;
extern const Pwm_ConfigType Pwm_Configuration;
extern const I2C_ConfigType I2C0_Master_100kHz;
extern const Adc_ConfigType Adc_Config;
extern const Qei_ConfigType Qei_Config;
extern const Wdg_ConfigType Wdg_Config;

/* ECUAL Configurations */
extern const IMU_ConfigType IMU_Config_Default;
extern const Encoder_ConfigType Encoder_Config;
extern const Motor_ConfigType Motor_Config;
extern const ACS712_ConfigType ACS712_Config;
extern const AM2320_ConfigType AM2320_Config;
extern const Fan_ConfigType Fan_Config;

/* SERVICES Configurations */
extern const ComStack_ConfigType ComStack_Config;
extern const ThermalMgmt_ConfigType ThermalMgmt_Config;
extern const Diag_ConfigType Diag_Config;

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize all system drivers and services
 */
void System_Init(void)
{
    /* ===== Phase 0: MPU Configuration (DISABLED FOR DEBUG) ===== */
    /*
     * TEMPORARILY DISABLED: MPU was causing silent resets.
     * Re-enable after system is stable.
     */
    /* MPU_Init(); */
    
    /* ===== Phase 1: MCAL Layer ===== */
    
    /* GPIO and UART0 already initialized by main_robot.c before System_Init() */
    /* Gpio_Init(&Gpio_Configuration); */
    /* Uart_Init(&Uart0_Config_115200); */
    
    /* Initialize UART1 (ROS2 communication via ComStack) */
    {
        const Uart_ConfigType Uart1_ROS2_Config = {
            .Module          = UART_MODULE_1,
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
        Uart_Init(&Uart1_ROS2_Config);
    }
    
    /* Initialize ADC (for current sensors) */
#if (FEATURE_CURRENT_ENABLED == 1u)
    Adc_Init(&Adc_Config);
#endif
    
    /* Initialize I2C (for IMU and temperature sensors) */
    I2C_Init(&I2C0_Master_100kHz);
    
    /* Initialize PWM (for motors and fans) */
    Pwm_Init(&Pwm_Configuration);
    
    /* Initialize QEI (for encoders) */
    Qei_Init(&Qei_Config);
    
    /* ===== Phase 2: Diagnostics (early for logging) ===== */
    Diag_Init(&Diag_Config);
    TimeSync_Init();
    Diag_LogEvent(DIAG_SRC_SYSTEM, 0x0010u, DIAG_SEVERITY_INFO, NULL_PTR);
    
    /* ===== Phase 3: ECUAL Layer ===== */
    
    /* Initialize IMU */
    if (IMU_Init(&IMU_Config_Default) != E_OK)
    {
        Diag_ReportDtc(DIAG_DTC_IMU_FAIL, TRUE);
    }
    
    /* Initialize Encoders */
    Encoder_Init(&Encoder_Config);
    
    /* Initialize Motor driver */
    Motor_Init(&Motor_Config);
    
    /* Initialize Current sensors */
#if (FEATURE_CURRENT_ENABLED == 1u)
    ACS712_Init(&ACS712_Config);
#endif
    
    /* Initialize Temperature sensors */
#if (FEATURE_TEMP_ENABLED == 1u)
    AM2320_Init(&AM2320_Config);
#endif
    
    /* Initialize Fan control */
#if (FEATURE_FAN_ENABLED == 1u)
    Fan_Init(&Fan_Config);
#endif
    
    /* ===== Phase 4: SERVICES Layer ===== */
    
    /* Initialize Communication Stack */
    ComStack_Init(&ComStack_Config);
    
    /* Initialize Thermal Management */
#if (FEATURE_TEMP_ENABLED == 1u)
    ThermalMgmt_Init(&ThermalMgmt_Config);
#endif
    
    /* ===== Phase 5: APP Layer ===== */
    
    /* Initialize Robot Controller */
    Robot_Init();
    
    /* ===== Phase 6: Watchdog (LAST - after all init complete) ===== */
    /*
     * SAFETY CRITICAL:
     * - WDG is initialized LAST so all drivers are ready
     * - ONLY Safety Task will feed the WDG (privilege model)
     * - If Safety Task hangs, WDG resets MCU in 500ms
     */
    Wdg_Init(&Wdg_Config);
    
    /* Enable Usage, Bus, and MemManage faults in SHCSR */
    *((volatile uint32*)0xE000ED24) |= 0x00070000;
    
    Diag_LogEvent(DIAG_SRC_SYSTEM, 0x0011u, DIAG_SEVERITY_INFO, NULL_PTR);
    Diag_DebugPrint("[SYS] Initialization complete - WDG active\r\n");
    Diag_DebugPrint("[BOOT] System_Init() complete\r\n");
}

/**
 * @brief De-initialize all system drivers
 */
void System_DeInit(void)
{
    /* Stop all motors */
    Motor_StopAll();
    
    /* Stop all fans */
    Fan_StopAll();
    
    /* Log shutdown */
    Diag_LogEvent(DIAG_SRC_SYSTEM, 0x00FFu, DIAG_SEVERITY_INFO, NULL_PTR);
}

/**
 * @brief Get system initialization status
 * @return TRUE if fully initialized
 */
boolean System_IsInitialized(void)
{
    /* Check critical services are running */
    return ((Diag_GetStatus() != DIAG_STATUS_UNINIT) &&
            (ComStack_GetStatus() != COMSTACK_STATUS_UNINIT));
}
