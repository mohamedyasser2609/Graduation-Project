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

#if 0 /* SLAM TEST ACTIVE */

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
#include "APP/Control/Robot_Control.h"

#include "CONFIG/Std_Types.h"
#include "tm4c123gh6pm.h"

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

static void SimpleDelay(volatile uint32 count)
{
    while (count > 0)
    {
        /* Ensure the loop is not optimized away and provides consistent timing */
        __asm("  nop");
        count--;
    }
}

static void PrintString(const char* str)
{
    Uart_SendString(UART_MODULE_0, (const uint8*)str);
}

static void PrintResetCause(void)
{
    uint32 cause = *((volatile uint32*)(0x400FE05C)); /* RESC register */
    PrintString("[SYS] Reset Cause: ");
    if (cause & 0x01) PrintString("External ");
    if (cause & 0x02) PrintString("Power-On ");
    if (cause & 0x04) PrintString("Brown-Out ");
    if (cause & 0x08) PrintString("Watchdog ");
    if (cause & 0x10) PrintString("Software ");
    PrintString("\r\n");
    *((volatile uint32*)(0x400FE05C)) = 0x1F; /* Clear all */
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

/* ===================[Velocity Command Processing]=================== */

/**
 * @brief Process received twist command from ROS2 (CMD 0x12) — PREFERRED
 * @details Receives linear velocity (v) and angular velocity (ω) directly
 *          as fixed-point ×1000 sint16 values.  The TM4C applies the
 *          differential drive equation internally via Robot_SetVelocity().
 *
 *  Differential drive equations (applied inside Robot_Control):
 *    v_left  = v - (ω × L / 2)
 *    v_right = v + (ω × L / 2)
 *
 *  Wire format (4 bytes, little-endian):
 *    [0-1] sint16  linear velocity   (mm/s  = m/s × 1000)
 *    [2-3] sint16  angular velocity  (mrad/s = rad/s × 1000)
 */
static void ProcessTwistCommand(const ComStack_PacketType* pkt)
{
    sint16 linearMmps;
    sint16 angularMrads;
    Robot_TwistType twist;

    if (pkt->Length < 4u)
    {
        return;
    }

    /* Parse fixed-point ×1000 values (little-endian sint16) */
    linearMmps  = (sint16)(((uint16)pkt->Data[1] << 8u) | (uint16)pkt->Data[0]);
    angularMrads = (sint16)(((uint16)pkt->Data[3] << 8u) | (uint16)pkt->Data[2]);

    /* Convert from fixed-point to float */
    twist.LinearX  = (float32)linearMmps  / 1000.0f;   /* mm/s → m/s  */
    twist.LinearY  = 0.0f;
    twist.AngularZ = (float32)angularMrads / 1000.0f;   /* mrad/s → rad/s */

    /* Debug */
    PrintString("[RX-TWIST] v:");
    PrintInt32((sint32)linearMmps);
    PrintString("mm/s w:");
    PrintInt32((sint32)angularMrads);
    PrintString("mrad/s\r\n");

    /* Apply via Robot_Control — clamping + diff-drive kinematics inside */
    (void)Robot_SetVelocity(&twist);

    rxCmdCount++;
}

/**
 * @brief Process received motor command from ROS2 (CMD 0x10) — LEGACY
 * @details Left/right wheel speed as signed percentage (-100..+100).
 *          Kept for backward compatibility.
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
    leftVel = ((float32)leftSpeed / 100.0f) * ROBOT_MAX_LINEAR_VEL;
    rightVel = ((float32)rightSpeed / 100.0f) * ROBOT_MAX_LINEAR_VEL;

    /* Convert wheel velocities to Twist (linear.x + angular.z) */
    twist.LinearX = (leftVel + rightVel) / 2.0f;
    twist.LinearY = 0.0f;
    twist.AngularZ = (rightVel - leftVel) / ROBOT_WHEEL_BASE_M;

    /* Apply via Robot_Control (clamping + state checks built in) */
    (void)Robot_SetVelocity(&twist);

    rxCmdCount++;
}

static void I2C_BusClear(void)
{
    volatile uint32 bb;
    uint8 clk;
    
    /* Set PB2 (SCL) and PB3 (SDA) as GPIO outputs temporarily */
    GPIO_PORTB_AFSEL_R &= ~((1u << 2) | (1u << 3)); /* Disable AF */
    GPIO_PORTB_ODR_R   |= (1u << 3);                  /* SDA Open Drain */
    GPIO_PORTB_PUR_R   |= (1u << 2) | (1u << 3);      /* Enable Pull-ups */
    GPIO_PORTB_DIR_R   |= (1u << 2) | (1u << 3);      /* Set as output */
    GPIO_PORTB_DEN_R   |= (1u << 2) | (1u << 3);      /* Digital enable */
    
    /* 1. Ensure SDA is high (let go) */
    GPIO_PORTB_DATA_R |= (1u << 3); 
    SimpleDelay(1000);
    
    /* 2. Clock SCL 9 times or until SDA is released */
    GPIO_PORTB_DIR_R |= (1u << 2);   /* SCL Out */
    GPIO_PORTB_DIR_R &= ~(1u << 3);  /* SDA In to check status */
    
    for (clk = 0; clk < 10; clk++) {
        if (GPIO_PORTB_DATA_R & (1u << 3)) break; /* SDA is HIGH, slave released it */
        
        GPIO_PORTB_DATA_R &= ~(1u << 2);  /* SCL LOW */
        SimpleDelay(2000);
        GPIO_PORTB_DATA_R |= (1u << 2);   /* SCL HIGH */
        SimpleDelay(2000);
    }
    
    /* 3. Send STOP equivalent (SDA Low while SCL High, then SDA High) */
    GPIO_PORTB_DIR_R |= (1u << 3);     /* SDA Out */
    GPIO_PORTB_DATA_R &= ~(1u << 3);   /* SDA Low */
    SimpleDelay(2000);
    GPIO_PORTB_DATA_R |= (1u << 3);    /* SDA High (STOP) */
    SimpleDelay(2000);
    
    /* Restore settings - Drive Strength 8mA for noise protection */
    GPIO_PORTB_DR8R_R |= (1u << 2) | (1u << 3);
    GPIO_PORTB_SLR_R  |= (1u << 2) | (1u << 3);
    GPIO_PORTB_AFSEL_R |= (1u << 2) | (1u << 3); /* Restore AF */
}

/* ===================[Main Function]=================== */

int main(void)
{
    Std_ReturnType initResult;
    const I2C_ConfigType I2C0_Slow = {
        .Module = I2C_MODULE_0,
        .Mode = I2C_MODE_MASTER,
        .Speed = 50000u
    };
    
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
    PrintResetCause();
    PrintString("========================================\r\n");
    PrintString(" SLAM Test Application v2.0\r\n");
    PrintString(" Robot Control + Encoder + IMU -> ROS2\r\n");
    PrintString(" Motor Control with PID <- ROS2\r\n");
    PrintString(" GPS: Excluded (wired to RPi)\r\n");
    PrintString("========================================\r\n");
    
#if 0 /* IMU ENABLED - Temporarily disabled for hardware debugging */
    /* ===== 4. I2C (for IMU) ===== */
    PrintString("[INIT] I2C...\r\n");
    
    /* Enable I2C0 clock and reset module to clear hardware lockups */
    *((volatile uint32*)(0x400FE620)) |= (1u << 0); /* RCGC I2C0 */
    SimpleDelay(10000);
    I2C_Reset(I2C_MODULE_0);
    SimpleDelay(100000);
    
    /* Strong I2C Bus Recovery */
    I2C_BusClear();
    
    /* Slow down I2C to 50kHz for noise immunity */
    I2C_Init(&I2C0_Slow);
    *((volatile uint32*)(0x40020020)) |= (1u << 4); /* Glitch Filter */
    SimpleDelay(100000);
    
    /* ===== 5. IMU (init BEFORE PWM to avoid electrical noise) ===== */
    PrintString("[INIT] IMU...");
    {
        uint8 imuRetry;
        initResult = E_NOT_OK;
        for (imuRetry = 0; imuRetry < 3u; imuRetry++) {
            initResult = IMU_Init(&IMU_Config_Default);
            if (initResult == E_OK) {
                break;
            }
            /* Reset I2C bus and try again */
            I2C_Reset(I2C_MODULE_0);
            SimpleDelay(100000);
        }
    }
    if (initResult == E_OK) {
        PrintString(" OK\r\n");
    } else {
        PrintString(" FAIL (continuing without IMU)\r\n");
    }
#endif /* IMU ENABLED */
    
    /* ===== 6. PWM (for motors) - AFTER IMU is safely initialized ===== */
    PrintString("[INIT] PWM...\r\n");
    Pwm_Init(&Pwm_Configuration);
    
    /* ===== 7. QEI (for encoders) ===== */
    PrintString("[INIT] QEI...\r\n");
    Qei_Init(&Qei_Config);
    
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
                    case COMSTACK_CMD_TWIST_CMD:
                        ProcessTwistCommand(&rxPacket);
                        break;

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
#if 0 /* Read IMU data */
        Std_ReturnType imuRet = IMU_ReadRawData(&imuRaw);
        
        /* If SDA is stubbornly stuck low, it simulates valid 0x00 data bytes with valid ACKs.
         * The only way to detect this "silent" lockup is if accel reads exactly 0x0000 on all axes (impossible due to gravity). */
        boolean isDeadZero = (imuRaw.accel.x == 0 && imuRaw.accel.y == 0 && imuRaw.accel.z == 0);
        /* Heuristic: detect garbage data (usually repetitive bytes from framing errors or locked buffers) */
        boolean isGarbage = (imuRaw.accel.x != 0) && (imuRaw.accel.x == imuRaw.accel.y) && (imuRaw.accel.y == imuRaw.accel.z);
        
        if (imuRet != E_OK || isDeadZero || isGarbage) {
            /* I2C bus may be hung - full recovery sequence */
            PrintString("\r\n[WARN] IMU Data Issue. Suppressing motors for recovery...\r\n");
            
            /* Stop motors during recovery to eliminate EMI */
            (void)Motor_SetSpeed(MOTOR_CHANNEL_LEFT, 0);
            (void)Motor_SetSpeed(MOTOR_CHANNEL_RIGHT, 0);
            SimpleDelay(400000); 
            
            I2C_BusClear();
            I2C_Reset(I2C_MODULE_0);
            SimpleDelay(80000); 
            
            I2C_Init(&I2C0_Slow);
            *((volatile uint32*)(0x40020020)) |= (1u << 4); /* Glitch Filter */
            SimpleDelay(80000);
            
            /* Blind Soft-Reset MPU6050 */
            uint8 rst = 0x80;
            (void)I2C_WriteRegister(I2C_MODULE_0, 0x68, 0x6B, &rst, 1);
            (void)I2C_WriteRegister(I2C_MODULE_0, 0x69, 0x6B, &rst, 1);
            SimpleDelay(400000);
            
            /* SigPath Reset */
            uint8 sRst = 0x07;
            (void)I2C_WriteRegister(I2C_MODULE_0, 0x68, 0x68, &sRst, 1);
            (void)I2C_WriteRegister(I2C_MODULE_0, 0x69, 0x68, &sRst, 1);
            
            /* Stabilization Delay (100ms) */
            SimpleDelay(8000000); 
            
            if (IMU_Init(&IMU_Config_Default) != E_OK) {
                PrintString("[ERROR] IMU Recovery Failed!\r\n");
            } else {
                PrintString("[INFO] IMU Recovery Successful. Restoring control.\r\n");
            }
        }
#else
        /* Virtual Dummy IMU Data for ROS2 (Raw values) */
        imuRaw.accel.x = 200;      /* Slight tilt simulator */
        imuRaw.accel.y = 100;
        imuRaw.accel.z = 16384;    /* ~1.0g (assuming 2g range) */
        imuRaw.gyro.x  = 0;
        imuRaw.gyro.y  = 0;
        imuRaw.gyro.z  = -10;      /* Constant slight drift simulator */
#endif /* IMU Read */
        
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
