/**
 * @file main_slam_test.c
 * @brief SLAM Test Application — Full Robot Control + Sensor Feedback
 * @details Standalone main for testing SLAM mapping with ROS2.
 *
 * What this does:
 * - Sends Encoder data (ticks + velocity) to RPi via ComStack @ 50Hz
 * - Sends IMU data (accel + gyro) to RPi via ComStack @ 50Hz
 * - Receives Motor commands from RPi and applies them via Robot_Control
 * - Uses PID velocity control and differential drive kinematics
 * - Computes odometry (dead reckoning)
 * - Debug output on UART0
 *
 * What this does NOT do:
 * - No GPS (wired directly to RPi)
 * - No FreeRTOS (simple super-loop for stability)
 *
 * Hardware:
 * - UART0 (PA0/PA1): Debug @ 115200
 * - UART1 (PB0/PB1): ROS2 RPi @ 115200
 * - I2C0 (PB2/PB3): MPU-6050 IMU
 * - QEI0/QEI1: Left/Right Encoders
 * - PWM0: Left/Right Motors
 *
 * @author Mohamed Yasser
 * @date Mar 12, 2026
 * @version 2.0.0 (Robot_Control integration)
 */

#if 0 /* DISABLED — main_robot.c is the active production entry point */

/* ===================[Includes]=================== */
#include "MCAL/MCU/Mcu.h"w
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/UART/Uart.h"
#include "MCAL/I2C/I2C.h"
#include "MCAL/PWM/PWM.h"
#include "MCAL/QEI/Qei.h"

#include "ECUAL/IMU/IMU.h"
#include "ECUAL/ENCODER/Encoder.h"
#include "ECUAL/MOTOR/Motor.h"

#include "SERVICES/COMM/ComStack.h"
#include "APP/Control/Robot_Control.h"

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
#define SLAM_LOOP_DELAY         (125000u)   /* ~20ms at 80MHz with volatile loop -> ~50Hz */
#define DEBUG_PRINT_INTERVAL    (50u)       /* Print debug every 50 loops (~1s if at 50Hz) */
#define CONTROL_DIVIDER         (2u)        /* Run PID at 100Hz (every 2nd loop) */

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
static Robot_OdometryType       odometry;
static uint32                   loopCounter = 0u;
static uint32                   controlCounter = 0u;
static uint32                   txCount = 0u;
static uint32                   rxCmdCount = 0u;

/* ===================[Packet Send Functions]=================== */

/**
 * @brief Send encoder data via ComStack (CMD 0x23)
 * @details Ticks as sint32, velocity as sint16 RPM×100
 */
static void SendEncoderPacket(void)
{
    ComStack_EncoderDataType encData;
    
    encData.LeftTicks = (sint32)encoderLeft.PositionCounts;
    encData.RightTicks = (sint32)encoderRight.PositionCounts;
    encData.LeftVelocity = (sint16)(encoderLeft.VelocityRPM * 100.0f);
    encData.RightVelocity = (sint16)(encoderRight.VelocityRPM * 100.0f);
    
    ComStack_SendEncoderData(&encData);
    txCount++;
}

/**
 * @brief Send IMU data via ComStack (CMD 0x22)
 * @details All values as sint16, real_value × 100
 */
static void SendImuPacket(void)
{
    ComStack_ImuDataType imuData;
    
    /* Convert raw to physical units, then scale ×100 to sint16 */
    /* Accel: +/-2g range -> 16384 LSB/g -> m/s² -> ×100 */
    imuData.AccelX = (sint16)(((float32)imuRaw.accel.x / 16384.0f) * 981.0f);
    imuData.AccelY = (sint16)(((float32)imuRaw.accel.y / 16384.0f) * 981.0f);
    imuData.AccelZ = (sint16)(((float32)imuRaw.accel.z / 16384.0f) * 981.0f);
    
    /* Gyro: +/-250 deg/s range -> 131 LSB/deg/s -> rad/s -> ×100 */
    imuData.GyroX = (sint16)(((float32)imuRaw.gyro.x / 131.0f) * 1.745329f);
    imuData.GyroY = (sint16)(((float32)imuRaw.gyro.y / 131.0f) * 1.745329f);
    imuData.GyroZ = (sint16)(((float32)imuRaw.gyro.z / 131.0f) * 1.745329f);
    
    ComStack_SendImuData(&imuData);
    txCount++;
}

/* ===================[Motor Command Processing]=================== */

/**
 * @brief Process received motor commands from ROS2
 * @details Converts signed speed (-100 to +100) to Robot_TwistType
 *          using differential drive inverse kinematics.
 */
static void ProcessMotorCommand(const ComStack_PacketType* pkt)
{
    sint16 leftSpeed;
    sint16 rightSpeed;
    Robot_TwistType twist;
    float32 leftVel, rightVel;
    
    if (pkt->Length < 4u)
    {
        return;
    }
    
    /* Parse: 2 bytes left speed (signed), 2 bytes right speed (signed) */
    leftSpeed = (sint16)(((uint16)pkt->Data[1] << 8u) | (uint16)pkt->Data[0]);
    rightSpeed = (sint16)(((uint16)pkt->Data[3] << 8u) | (uint16)pkt->Data[2]);
    
    /* Debug: print received motor command */
    PrintString("[RX-MOT] L:");
    PrintInt32((sint32)leftSpeed);
    PrintString(" R:");
    PrintInt32((sint32)rightSpeed);
    PrintString("\r\n");
    
    /* Clamp to -100..+100 */
    if (leftSpeed > 100) leftSpeed = 100;
    if (leftSpeed < -100) leftSpeed = -100;
    if (rightSpeed > 100) rightSpeed = 100;
    if (rightSpeed < -100) rightSpeed = -100;
    
    /* Convert percentage (-100..100) to velocity (m/s) */
    /* Max speed = ROBOT_MAX_LINEAR_VEL (1.0 m/s) */
    leftVel = ((float32)leftSpeed / 100.0f) * ROBOT_MAX_LINEAR_VEL;
    rightVel = ((float32)rightSpeed / 100.0f) * ROBOT_MAX_LINEAR_VEL;
    
    /* Convert wheel velocities to Twist (linear.x + angular.z) */
    /* v = (v_l + v_r) / 2 */
    /* w = (v_r - v_l) / L */
    twist.LinearX = (leftVel + rightVel) / 2.0f;
    twist.LinearY = 0.0f;
    twist.AngularZ = (rightVel - leftVel) / ROBOT_WHEEL_BASE_M;
    
    /* Apply via Robot_Control (clamping + state checks built in) */
    (void)Robot_SetVelocity(&twist);
    
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
    PrintString(" SLAM Test Application v2.0\r\n");
    PrintString(" Robot Control + Encoder + IMU -> ROS2\r\n");
    PrintString(" Motor Control with PID <- ROS2\r\n");
    PrintString(" GPS: Excluded (wired to RPi)\r\n");
    PrintString("========================================\r\n");
    
    /* ===== 4. I2C (for IMU) ===== */
    PrintString("[INIT] I2C...\r\n");
    I2C_Init(&I2C0_Master_100kHz);
    SimpleDelay(50000);
    
    /* ===== 5. PWM (for motors) ===== */
    PrintString("[INIT] PWM...\r\n");
    Pwm_Init(&Pwm_Configuration);
    
    /* ===== 6. QEI (for encoders) ===== */
    PrintString("[INIT] QEI...\r\n");
    Qei_Init(&Qei_Config);
    
    /* ===== 7. ECUAL Drivers ===== */
    
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
    
    /* ===== 8. Robot Controller (PID + Kinematics) ===== */
    PrintString("[INIT] Robot Controller (PID + Diff Drive)...\r\n");
    Robot_Init();
    
    /* ===== 9. ComStack ===== */
    PrintString("[INIT] ComStack...\r\n");
    
    /* UART1 for ROS2 */
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
    PrintString("[LOOP] Sensor TX @ 50Hz | PID Control @ 100Hz\r\n\r\n");
    
    /* ===== Main Super-Loop (50Hz base rate) ===== */
    while (1)
    {
        /* --- RX: WASD Keyboard Control (UART0) --- */
        if (Uart_IsRxDataAvailable(UART_MODULE_0))
        {
            uint8 key;
            if (Uart_ReceiveByte(UART_MODULE_0, &key) == E_OK)
            {
                Robot_TwistType kbTwist = {0.0f, 0.0f, 0.0f};
                boolean keyValid = TRUE;
                
                switch(key)
                {
                    case 'w': case 'W':
                        kbTwist.LinearX = 0.5f; /* Example: 0.5 m/s forward */
                        PrintString("\r\n[KB] Forward\r\n");
                        break;
                    case 's': case 'S':
                        kbTwist.LinearX = -0.5f; /* 0.5 m/s backward */
                        PrintString("\r\n[KB] Backward\r\n");
                        break;
                    case 'a': case 'A':
                        kbTwist.AngularZ = 1.0f; /* 1.0 rad/s spin left */
                        PrintString("\r\n[KB] Left\r\n");
                        break;
                    case 'd': case 'D':
                        kbTwist.AngularZ = -1.0f; /* 1.0 rad/s spin right */
                        PrintString("\r\n[KB] Right\r\n");
                        break;
                    case ' ': case 'x': case 'X':
                        /* Stop - twist is already 0.0 */
                        PrintString("\r\n[KB] Stopped\r\n");
                        break;
                    default:
                        keyValid = FALSE;
                        break;
                }
                
                if (keyValid)
                {
                    (void)Robot_SetVelocity(&kbTwist);
                }
            }
        }

        /* --- RX: Process incoming bytes from RPi --- */
        ComStack_MainFunction();
        
        /* --- RX: Handle received packets --- */
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
                        Robot_EmergencyStop();
                        rxCmdCount++;
                        break;
                    
                    case COMSTACK_CMD_PING:
                        ComStack_SendAck();
                        break;
                    
                    default:
                        break;
                }
            }
        }
        
        /* --- Sensor Reads (every loop = 50Hz) --- */
        
        /* Read IMU (raw) */
        (void)IMU_ReadRawData(&imuRaw);
        
        /* Read Encoders */
        Encoder_UpdateAll();
        (void)Encoder_GetData(ENCODER_CHANNEL_LEFT, &encoderLeft);
        (void)Encoder_GetData(ENCODER_CHANNEL_RIGHT, &encoderRight);
        
        /* --- Robot Control Update --- */
        
        /* Update odometry every loop (50Hz) */
        Robot_UpdateOdometry();
        
        /* Update PID control loop at 100Hz (every other 10ms half-cycle) */
        controlCounter++;
        if (controlCounter >= CONTROL_DIVIDER)
        {
            controlCounter = 0u;
            Robot_UpdateControl();
        }
        
        /* --- TX: Send sensor data to RPi (50Hz) --- */
        SendEncoderPacket();
        SendImuPacket();
        
        /* --- Debug Output (every ~1s) --- */
        loopCounter++;
        if (loopCounter % DEBUG_PRINT_INTERVAL == 0u)
        {
            (void)Robot_GetOdometry(&odometry);
            
            Motor_DataType mLeft, mRight;
            (void)Motor_GetData(MOTOR_CHANNEL_LEFT, &mLeft);
            (void)Motor_GetData(MOTOR_CHANNEL_RIGHT, &mRight);

            PrintString("[SLAM] TX:");
            PrintInt32((sint32)txCount);
            PrintString(" RX:");
            PrintInt32((sint32)rxCmdCount);
            PrintString(" | L_Enc:");
            PrintInt32((sint32)encoderLeft.PositionCounts);
            PrintString(" R_Enc:");
            PrintInt32((sint32)encoderRight.PositionCounts);
            PrintString(" | L_Pwm%:");
            PrintInt32((sint32)mLeft.SpeedPercent);
            PrintString(" R_Pwm%:");
            PrintInt32((sint32)mRight.SpeedPercent);
            
            /* Recalculate scaled integer IMU data to match what is sent to RPi */
            sint16 ax = (sint16)(((float32)imuRaw.accel.x / 16384.0f) * 981.0f);
            sint16 ay = (sint16)(((float32)imuRaw.accel.y / 16384.0f) * 981.0f);
            sint16 az = (sint16)(((float32)imuRaw.accel.z / 16384.0f) * 981.0f);
            sint16 wz = (sint16)(((float32)imuRaw.gyro.z / 131.0f) * 1.745329f);
            
            PrintString("\r\n       IMU[ ax:");
            PrintInt32((sint32)ax);
            PrintString(" ay:");
            PrintInt32((sint32)ay);
            PrintString(" az:");
            PrintInt32((sint32)az);
            PrintString(" wz:");
            PrintInt32((sint32)wz);
            PrintString(" ]\r\n");
        }
        
        /* --- Loop Delay (~20ms -> 50Hz) --- */
        SimpleDelay(SLAM_LOOP_DELAY);
    }
}

#endif /* Enable guard */
