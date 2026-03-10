/**
 * @file main_slam_test.c
 * @brief SLAM Test Application — Encoder + IMU → ROS2
 * @details Standalone main for testing SLAM mapping with ROS2.
 *
 * What this does:
 * - Sends Encoder data (ticks + velocity) to RPi via ComStack @ 50Hz
 * - Sends IMU data (accel + gyro) to RPi via ComStack @ 50Hz
 * - Receives Motor commands from RPi and applies them
 * - Debug output on UART0
 *
 * What this does NOT do:
 * - No GPS (wired directly to RPi)
 * - No temperature/current monitoring
 * - No FreeRTOS (simple super-loop for stability)
 *
 * Hardware:
 * - UART0 (PA0/PA1): Debug @ 115200
 * - UART1 (PB0/PB1): ROS2 RPi @ 115200
 * - I2C0 (PB2/PB3): MPU-9250 IMU
 * - QEI0 (PD6/PD7): Left Encoder
 * - QEI1 (PC5/PC6): Right Encoder
 * - PWM0 (PA6/PA7): Motors
 *
 * @author Mohamed Yasser
 * @date Mar 10, 2026
 * @version 1.0.0
 */

#if 1 /* Set to 1 to enable this main file */

/* ===================[Includes]=================== */
#include "MCAL/MCU/Mcu.h"
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/UART/Uart.h"
#include "MCAL/I2C/I2C.h"
#include "MCAL/PWM/PWM.h"
#include "MCAL/QEI/Qei.h"

#include "ECUAL/IMU/IMU.h"
#include "ECUAL/ENCODER/Encoder.h"
#include "ECUAL/MOTOR/Motor.h"

#include "SERVICES/COMM/ComStack.h"

#include "CONFIG/Std_Types.h"

/* ===================[External Configurations]=================== */
extern const Mcu_ConfigType* Mcu_ConfigPtr;
extern const Gpio_ConfigType Gpio_Configuration;
extern const Uart_ConfigType Uart0_Config_115200;
extern const Pwm_ConfigType Pwm_Configuration;
extern const I2C_ConfigType I2C0_Master_100kHz;
extern const Qei_ConfigType Qei_Config;
extern const IMU_ConfigType IMU_Config_Default;
extern const Encoder_ConfigType Encoder_Config;
extern const Motor_ConfigType Motor_Config;
extern const ComStack_ConfigType ComStack_Config;

/* ===================[Definitions]=================== */
#define SLAM_LOOP_DELAY         (1600000u)  /* ~20ms at 80MHz → 50Hz */
#define DEBUG_PRINT_INTERVAL    (50u)       /* Print debug every 50 loops (~1s) */

/* ===================[Helper Functions]=================== */

static void SimpleDelay(uint32 count)
{
    volatile uint32 i;
    for (i = 0; i < count; i++);
}

static void PrintString(const char* str)
{
    Uart_SendString(UART_MODULE_0, (const uint8*)str);
}

static void PrintInt32(sint32 val)
{
    char buf[12];
    uint8 i = 0;
    uint32 uval;
    
    if (val < 0) {
        Uart_SendByte(UART_MODULE_0, '-');
        uval = (uint32)(-val);
    } else {
        uval = (uint32)val;
    }
    
    if (uval == 0) {
        Uart_SendByte(UART_MODULE_0, '0');
        return;
    }
    
    while (uval > 0 && i < 11) {
        buf[i++] = '0' + (uval % 10);
        uval /= 10;
    }
    
    while (i > 0) {
        Uart_SendByte(UART_MODULE_0, buf[--i]);
    }
}

/* ===================[Interrupt Handlers]=================== */

/**
 * @brief Timer2A handler (dummy to satisfy linker)
 */
void Timer2A_Handler(void)
{
    while(1);
}

/* ===================[SLAM Data]=================== */

static IMU_SensorDataType      imuRaw;
static Encoder_DataType         encoderLeft;
static Encoder_DataType         encoderRight;
static ComStack_PacketType      rxPacket;
static uint32                   loopCounter = 0u;
static uint32                   txCount = 0u;
static uint32                   rxCmdCount = 0u;

/* ===================[Packet Send Functions]=================== */

/**
 * @brief Send encoder data via ComStack (CMD 0x23)
 */
static void SendEncoderPacket(void)
{
    ComStack_EncoderDataType encData;
    
    encData.LeftTicks = (sint32)encoderLeft.PositionCounts;
    encData.RightTicks = (sint32)encoderRight.PositionCounts;
    encData.LeftVelocity = encoderLeft.VelocityRPM;
    encData.RightVelocity = encoderRight.VelocityRPM;
    
    ComStack_SendEncoderData(&encData);
    txCount++;
}

/**
 * @brief Send IMU data via ComStack (CMD 0x22)
 */
static void SendImuPacket(void)
{
    ComStack_ImuDataType imuData;
    
    /* Convert raw to float (simple scaling, not calibrated) */
    /* Accel: ±2g range → 16384 LSB/g */
    imuData.AccelX = (float32)imuRaw.accel.x / 16384.0f;
    imuData.AccelY = (float32)imuRaw.accel.y / 16384.0f;
    imuData.AccelZ = (float32)imuRaw.accel.z / 16384.0f;
    
    /* Gyro: ±250°/s range → 131 LSB/°/s → convert to rad/s */
    imuData.GyroX = ((float32)imuRaw.gyro.x / 131.0f) * 0.01745329f;
    imuData.GyroY = ((float32)imuRaw.gyro.y / 131.0f) * 0.01745329f;
    imuData.GyroZ = ((float32)imuRaw.gyro.z / 131.0f) * 0.01745329f;
    
    ComStack_SendImuData(&imuData);
    txCount++;
}

/* ===================[Motor Command Processing]=================== */

/**
 * @brief Process received motor commands from ROS2
 */
static void ProcessMotorCommand(const ComStack_PacketType* pkt)
{
    sint16 leftSpeed;
    sint16 rightSpeed;
    Motor_DirectionType leftDir, rightDir;
    uint8 leftAbs, rightAbs;
    
    if (pkt->Length < 4u)
    {
        return;
    }
    
    /* Parse: 2 bytes left speed (signed), 2 bytes right speed (signed) */
    leftSpeed = (sint16)(((uint16)pkt->Data[1] << 8u) | (uint16)pkt->Data[0]);
    rightSpeed = (sint16)(((uint16)pkt->Data[3] << 8u) | (uint16)pkt->Data[2]);
    
    /* Direction and absolute speed */
    leftDir = (leftSpeed >= 0) ? MOTOR_DIRECTION_FORWARD : MOTOR_DIRECTION_REVERSE;
    leftAbs = (uint8)((leftSpeed >= 0) ? leftSpeed : -leftSpeed);
    if (leftAbs > 100u) leftAbs = 100u;
    
    rightDir = (rightSpeed >= 0) ? MOTOR_DIRECTION_FORWARD : MOTOR_DIRECTION_REVERSE;
    rightAbs = (uint8)((rightSpeed >= 0) ? rightSpeed : -rightSpeed);
    if (rightAbs > 100u) rightAbs = 100u;
    
    /* Apply to motors */
    Motor_SetSpeedAndDirection(MOTOR_CHANNEL_LEFT, leftAbs, leftDir);
    Motor_SetSpeedAndDirection(MOTOR_CHANNEL_RIGHT, rightAbs, rightDir);
    
    rxCmdCount++;
}

/* ===================[Main Function]=================== */

int main(void)
{
    Std_ReturnType initResult;
    
    /* ===== 1. MCU Init (80MHz PLL) ===== */
    Mcu_Init(Mcu_ConfigPtr);
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) { }
    Mcu_DistributePllClock();
    SimpleDelay(100000);
    
    /* ===== 2. GPIO ===== */
    Gpio_Init(&Gpio_Configuration);
    
    /* ===== 3. UART0 (Debug) ===== */
    Uart_Init(&Uart0_Config_115200);
    SimpleDelay(50000);
    
    PrintString("\r\n");
    PrintString("========================================\r\n");
    PrintString(" SLAM Test Application v1.0\r\n");
    PrintString(" Encoder + IMU -> ROS2 | Motor <- ROS2\r\n");
    PrintString(" GPS: Excluded (wired to RPi)\r\n");
    PrintString("========================================\r\n");
    
    /* ===== 4. UART1 (ROS2 RPi) ===== */
    /* ComStack will use UART1 as configured in ComStack_PBCfg */
    
    /* ===== 5. I2C (for IMU) ===== */
    PrintString("[INIT] I2C...\r\n");
    I2C_Init(&I2C0_Master_100kHz);
    SimpleDelay(50000);
    
    /* ===== 6. PWM (for motors) ===== */
    PrintString("[INIT] PWM...\r\n");
    Pwm_Init(&Pwm_Configuration);
    
    /* ===== 7. QEI (for encoders) ===== */
    PrintString("[INIT] QEI...\r\n");
    Qei_Init(&Qei_Config);
    
    /* ===== 8. ECUAL Drivers ===== */
    
    /* IMU */
    PrintString("[INIT] IMU...");
    initResult = IMU_Init(&IMU_Config_Default);
    if (initResult == E_OK) {
        PrintString(" OK\r\n");
    } else {
        PrintString(" FAIL (continuing without IMU)\r\n");
    }
    
    /* Encoders */
    PrintString("[INIT] Encoders...\r\n");
    Encoder_Init(&Encoder_Config);
    
    /* Motors */
    PrintString("[INIT] Motors...\r\n");
    Motor_Init(&Motor_Config);
    
    /* ===== 9. ComStack ===== */
    PrintString("[INIT] ComStack...\r\n");
    
    /* UART1 for ROS2 needs to be initialized */
    {
        const Uart_ConfigType ros2Uart = {
            .Module = UART_MODULE_1,
            .BaudRate = UART_BAUD_115200,
            .DataBits = UART_DATA_BITS_8,
            .Parity = UART_PARITY_NONE,
            .StopBits = UART_STOP_BITS_1,
            .FlowControl = UART_FLOW_CONTROL_NONE,
            .FifoEnable = TRUE,
            .RxInterruptEnable = FALSE,
            .TxInterruptEnable = FALSE,
            .RxCallback = NULL_PTR,
            .TxCallback = NULL_PTR
        };
        Uart_Init(&ros2Uart);
    }
    SimpleDelay(50000);
    
    ComStack_Init(&ComStack_Config);
    
    PrintString("[INIT] All systems ready.\r\n");
    PrintString("[LOOP] Starting 50Hz sensor loop...\r\n\r\n");
    
    /* ===== Main Super-Loop ===== */
    while (1)
    {
        /* --- RX: Process incoming bytes from RPi --- */
        ComStack_MainFunction();
        
        /* --- RX: Handle any received packets --- */
        while (ComStack_IsPacketAvailable())
        {
            if (ComStack_GetPacket(&rxPacket) == COMSTACK_RX_OK)
            {
                switch (rxPacket.Command)
                {
                    case COMSTACK_CMD_MOTOR_CMD:
                        ProcessMotorCommand(&rxPacket);
                        break;
                    
                    case COMSTACK_CMD_MOTOR_STOP:
                        Motor_StopAll();
                        rxCmdCount++;
                        break;
                    
                    case COMSTACK_CMD_PING:
                        ComStack_SendAck();
                        break;
                    
                    default:
                        /* Unknown command — ignore */
                        break;
                }
            }
        }
        
        /* --- TX: Read sensors and send to RPi --- */
        
        /* Read IMU (raw) */
        (void)IMU_ReadRawData(&imuRaw);
        
        /* Read Encoders */
        Encoder_UpdateAll();
        (void)Encoder_GetData(ENCODER_CHANNEL_LEFT, &encoderLeft);
        (void)Encoder_GetData(ENCODER_CHANNEL_RIGHT, &encoderRight);
        
        /* Send packets to RPi */
        SendEncoderPacket();
        SendImuPacket();
        
        /* --- Debug Output (every ~1s) --- */
        loopCounter++;
        if (loopCounter % DEBUG_PRINT_INTERVAL == 0u)
        {
            PrintString("[SLAM] TX:");
            PrintInt32((sint32)txCount);
            PrintString(" RX:");
            PrintInt32((sint32)rxCmdCount);
            PrintString(" | L:");
            PrintInt32((sint32)encoderLeft.PositionCounts);
            PrintString(" R:");
            PrintInt32((sint32)encoderRight.PositionCounts);
            PrintString(" | AZ:");
            PrintInt32((sint32)imuRaw.accel.z);
            PrintString(" GZ:");
            PrintInt32((sint32)imuRaw.gyro.z);
            PrintString("\r\n");
        }
        
        /* --- Loop Delay (~20ms → 50Hz) --- */
        SimpleDelay(SLAM_LOOP_DELAY);
    }
}

#endif /* Enable guard */
