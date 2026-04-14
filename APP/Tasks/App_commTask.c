/**
 * @file App_CommTask.c
 * @brief Communication Task Implementation — Binary ComStack Protocol
 * @details Handles ROS2 communication via ComStack binary framed protocol.
 *
 * RX (from ROS2 via ComStack binary):
 *   CMD 0x10 MOTOR_CMD:  4 bytes — sint16 leftSpeed, sint16 rightSpeed
 *   CMD 0x11 MOTOR_STOP: 0 bytes — Emergency stop
 *   CMD 0x01 PING:       0 bytes — Heartbeat / connection check
 *
 * TX (to ROS2 via ComStack binary):
 *   CMD 0x23 ENCODER_DATA: sint32 leftTicks, sint32 rightTicks, float32 leftVel, float32 rightVel
 *   CMD 0x22 IMU_DATA:     float32 ax,ay,az, float32 gx,gy,gz
 *
 * @author Mohamed Yasser
 * @date Apr 13, 2026
 * @version 2.0.0 — Rewritten to use ComStack binary protocol (was ASCII)
 */

#include "../../CONFIG/Std_Types.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "queue.h"

/* Service includes */
#include "../../SERVICES/COMM/ComStack.h"
#include "../../SERVICES/RTOS/Tasks_Init.h"

/* ECUAL includes (for sensor data reads) */
#include "../../ECUAL/ENCODER/Encoder.h"
#include "../../ECUAL/IMU/IMU.h"

/* APP includes */
#include "../Common/App_SharedTypes.h"
#include "../Control/Robot_Control.h"

/* ===================[External Safety API]=================== */
extern void App_SafetyTask_ReportHeartbeat(void);

/* ===================[Private Variables]=================== */
static boolean App_CommInitialized = FALSE;

/* Queue handle cache */
static QueueHandle_t App_WheelCmdQueue = NULL;

/* Received packet buffer */
static ComStack_PacketType App_RxPacket;

/* TX statistics */
static uint32 App_TxCount = 0u;
static uint32 App_RxCmdCount = 0u;

/* ===================[Private Functions]=================== */

/**
 * @brief Process a received motor command packet (CMD 0x10)
 * @details Converts signed speed (-100..+100) to WheelSpeedCmdType
 *          for the Control Task queue. Uses differential drive inverse
 *          kinematics to convert wheel speeds to rad/s.
 *
 * @param[in] pkt Pointer to received packet
 */
static void App_ProcessMotorCommand(const ComStack_PacketType* pkt)
{
    sint16 leftSpeed;
    sint16 rightSpeed;
    WheelSpeedCmdType cmd;
    float32 leftVel, rightVel;

    if (pkt->Length < 4u)
    {
        return;
    }

    /* Parse: 2 bytes left speed (sint16 LE), 2 bytes right speed (sint16 LE) */
    leftSpeed = (sint16)(((uint16)pkt->Data[1] << 8u) | (uint16)pkt->Data[0]);
    rightSpeed = (sint16)(((uint16)pkt->Data[3] << 8u) | (uint16)pkt->Data[2]);

    /* Clamp to -100..+100 */
    if (leftSpeed > 100) leftSpeed = 100;
    if (leftSpeed < -100) leftSpeed = -100;
    if (rightSpeed > 100) rightSpeed = 100;
    if (rightSpeed < -100) rightSpeed = -100;

    /* Convert percentage (-100..100) to velocity (m/s) */
    /* Max speed = ROBOT_MAX_LINEAR_VEL (1.0 m/s) */
    leftVel = ((float32)leftSpeed / 100.0f) * ROBOT_MAX_LINEAR_VEL;
    rightVel = ((float32)rightSpeed / 100.0f) * ROBOT_MAX_LINEAR_VEL;

    /* Convert wheel velocities to rad/s for the Control Task queue */
    /* v_wheel (m/s) = omega (rad/s) * wheel_radius (m) */
    /* omega (rad/s) = v_wheel / wheel_radius */
    cmd.LeftRadPerSec = leftVel / ROBOT_WHEEL_RADIUS_M;
    cmd.RightRadPerSec = rightVel / ROBOT_WHEEL_RADIUS_M;
    cmd.Timestamp = xTaskGetTickCount();
    cmd.Valid = TRUE;

    /* Send to Control Task via queue (non-blocking, overwrite old commands) */
    if (App_WheelCmdQueue != NULL)
    {
        (void)xQueueOverwrite(App_WheelCmdQueue, &cmd);
    }

    App_RxCmdCount++;
}

/**
 * @brief Process all incoming ComStack packets
 * @details Calls ComStack_MainFunction() to process UART bytes, then
 *          handles all available received packets.
 */
static void App_ProcessComStackRx(void)
{
    /* Process incoming UART bytes into ComStack packets */
    ComStack_MainFunction();

    /* Handle all available received packets */
    while (ComStack_IsPacketAvailable())
    {
        if (ComStack_GetPacket(&App_RxPacket) == COMSTACK_RX_OK)
        {
            switch (App_RxPacket.Command)
            {
                case COMSTACK_CMD_MOTOR_CMD:
                    App_ProcessMotorCommand(&App_RxPacket);
                    /* Motor commands also prove connectivity */
                    App_SafetyTask_ReportHeartbeat();
                    break;

                case COMSTACK_CMD_MOTOR_STOP:
                    Robot_EmergencyStop();
                    App_RxCmdCount++;
                    break;

                case COMSTACK_CMD_PING:
                    /* Heartbeat — ACK the ping and report to Safety Task */
                    ComStack_SendAck();
                    App_SafetyTask_ReportHeartbeat();
                    break;

                default:
                    /* Unknown command — ignore */
                    break;
            }
        }
    }
}

/**
 * @brief Transmit encoder data via ComStack binary (CMD 0x23)
 */
static void App_TransmitEncoderData(void)
{
    Encoder_DataType encoderLeft;
    Encoder_DataType encoderRight;
    ComStack_EncoderDataType encData;

    (void)Encoder_GetData(ENCODER_CHANNEL_LEFT, &encoderLeft);
    (void)Encoder_GetData(ENCODER_CHANNEL_RIGHT, &encoderRight);

    encData.LeftTicks = (sint32)encoderLeft.PositionCounts;
    encData.RightTicks = (sint32)encoderRight.PositionCounts;
    encData.LeftVelocity = (sint16)(encoderLeft.VelocityRPM * 100.0f);
    encData.RightVelocity = (sint16)(encoderRight.VelocityRPM * 100.0f);

    ComStack_SendEncoderData(&encData);
    App_TxCount++;
}

/**
 * @brief Transmit IMU data via ComStack binary (CMD 0x22)
 * @details Values are sent as scaled integers (* 100).
 */
static void App_TransmitImuData(void)
{
    IMU_SensorDataType imuRaw;
    ComStack_ImuDataType imuData;

    (void)IMU_ReadRawData(&imuRaw);

    /* Accel: +/-2g range → 16384 LSB/g → m/s² * 100 */
    imuData.AccelX = (sint16)((((float32)imuRaw.accel.x / 16384.0f) * 9.81f) * 100.0f);
    imuData.AccelY = (sint16)((((float32)imuRaw.accel.y / 16384.0f) * 9.81f) * 100.0f);
    imuData.AccelZ = (sint16)((((float32)imuRaw.accel.z / 16384.0f) * 9.81f) * 100.0f);

    /* Gyro: +/-250 deg/s range → 131 LSB/deg/s → rad/s * 100 */
    imuData.GyroX = (sint16)((((float32)imuRaw.gyro.x / 131.0f) * 0.0174533f) * 100.0f);
    imuData.GyroY = (sint16)((((float32)imuRaw.gyro.y / 131.0f) * 0.0174533f) * 100.0f);
    imuData.GyroZ = (sint16)((((float32)imuRaw.gyro.z / 131.0f) * 0.0174533f) * 100.0f);

    ComStack_SendImuData(&imuData);
    App_TxCount++;
}

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize communication task
 */
void App_CommTask_Init(void)
{
    /* Get queue handles */
    App_WheelCmdQueue = Tasks_GetWheelSpeedCmdQueue();

    App_TxCount = 0u;
    App_RxCmdCount = 0u;
    App_CommInitialized = TRUE;
}

/**
 * @brief Communication task main function (called by FreeRTOS task @ 50Hz)
 * @details
 *   1. Process incoming ComStack binary packets (motor commands, heartbeat)
 *   2. Transmit encoder data to ROS2 (binary, 50Hz)
 *   3. Transmit IMU data to ROS2 (binary, 50Hz)
 */
void App_CommTask_Run(void)
{
    if (App_CommInitialized == FALSE)
    {
        App_CommTask_Init();
    }

    /* 1. Process incoming binary packets from ROS2 */
    App_ProcessComStackRx();

    /* 2. Transmit sensor data to ROS2 (binary protocol) */
    App_TransmitEncoderData();
    App_TransmitImuData();
}
