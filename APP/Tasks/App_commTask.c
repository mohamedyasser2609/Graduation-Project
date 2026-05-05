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
 *   CMD 0x22 IMU_DATA:     sint16 ax,ay,az, sint16 gx,gy,gz (all x100)
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

/* Service includes for telemetry */
#include "../../SERVICES/THERMAL/ThermalMgmt.h"
#include "../../SERVICES/DIAG/Diagnostics.h"
#include "../../SERVICES/TIME/TimeSync.h"

/* ===================[External System API]=================== */
extern void App_SafetyTask_ReportHeartbeat(void);
extern Std_ReturnType App_SensorTask_GetImuData(IMU_CalibratedDataType* DataPtr);

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
 * @brief Process a received twist velocity command (CMD 0x12) — PREFERRED
 * @details Receives linear velocity (v) and angular velocity (ω) directly
 *          as fixed-point ×1000 sint16 values (mm/s and mrad/s).
 *
 *          The TM4C applies the differential drive equation internally:
 *            v_left  = v - (ω × L / 2)
 *            v_right = v + (ω × L / 2)
 *
 * @param[in] pkt Pointer to received packet (4 bytes data)
 */
static void App_ProcessTwistCommand(const ComStack_PacketType* pkt)
{
    sint16 linearMmps;
    sint16 angularMrads;
    Robot_TwistType twist;

    if (pkt->Length < 4u)
    {
        return;
    }

    /* Parse fixed-point ×1000 values (little-endian sint16) */
    linearMmps   = (sint16)(((uint16)pkt->Data[1] << 8u) | (uint16)pkt->Data[0]);
    angularMrads = (sint16)(((uint16)pkt->Data[3] << 8u) | (uint16)pkt->Data[2]);

    /* Convert from fixed-point to float */
    twist.LinearX  = (float32)linearMmps  / 1000.0f;   /* mm/s  → m/s   */
    twist.LinearY  = 0.0f;
    twist.AngularZ = (float32)angularMrads / 1000.0f;   /* mrad/s → rad/s */

    /* Calculate individual wheel speeds (rad/s) for the Control Task queue */
    /* This avoids the Control Task overwriting the direct Robot_SetVelocity call with 0.0 */
    WheelSpeedCmdType cmd;
    float32 v = twist.LinearX;
    float32 w = twist.AngularZ;
    float32 L = ROBOT_WHEEL_BASE_M;
    float32 R = ROBOT_WHEEL_RADIUS_M;

    cmd.LeftRadPerSec  = (v - (w * L / 2.0f)) / R;
    cmd.RightRadPerSec = (v + (w * L / 2.0f)) / R;
    cmd.Valid = TRUE;

    /* Send to Control Task via queue (non-blocking, overwrite old commands) */
    if (App_WheelCmdQueue != NULL)
    {
        (void)xQueueOverwrite(App_WheelCmdQueue, &cmd);
    }

    App_RxCmdCount++;
}

/**
 * @brief Process a received motor command packet (CMD 0x10) — LEGACY
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
 * @brief Process a time synchronization command (CMD 0x05)
 * @param pkt Pointer to received packet (8 bytes: 4s, 4ns)
 */
static void App_ProcessTimeSync(const ComStack_PacketType* pkt)
{
    uint32 sec, nsec;

    if (pkt->Length < 8u)
    {
        return;
    }

    /* Parse 4-byte seconds and 4-byte nanoseconds (little-endian) */
    sec  = (uint32)pkt->Data[0] | ((uint32)pkt->Data[1] << 8u) | ((uint32)pkt->Data[2] << 16u) | ((uint32)pkt->Data[3] << 24u);
    nsec = (uint32)pkt->Data[4] | ((uint32)pkt->Data[5] << 8u) | ((uint32)pkt->Data[6] << 16u) | ((uint32)pkt->Data[7] << 24u);

    TimeSync_Update(sec, nsec);
    Diag_DebugPrint("[COMM] Time synchronized with ROS2\r\n");
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
            Diag_DebugPrintValue("[COMM] RX CMD (Decimal): ", App_RxPacket.Command);
            
            switch (App_RxPacket.Command)
            {
                case COMSTACK_CMD_TWIST_CMD:
                    App_ProcessTwistCommand(&App_RxPacket);
                    App_SafetyTask_ReportHeartbeat();
                    break;

                case COMSTACK_CMD_TIME_SYNC:
                    App_ProcessTimeSync(&App_RxPacket);
                    break;

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
 * @details Values are retrieved from SensorTask cache to avoid I2C race conditions.
 *          Values are sent as scaled integers (real_value * 100).
 */
static void App_TransmitImuData(void)
{
    IMU_CalibratedDataType imuCal;
    ComStack_ImuDataType imuData;

    /* Get calibrated data from SensorTask cache (avoids I2C bus collision) */
    if (App_SensorTask_GetImuData(&imuCal) == E_OK)
    {
        /* Accel: g → m/s² * 100 */
        imuData.AccelX = (sint16)(imuCal.accel.x * 981.0f);
        imuData.AccelY = (sint16)(imuCal.accel.y * 981.0f);
        imuData.AccelZ = (sint16)(imuCal.accel.z * 981.0f);

        /* Gyro: deg/s → rad/s * 100 */
        /* rad/s = deg/s * (PI/180) ≈ deg/s * 0.0174533 */
        /* rad/s * 100 ≈ deg/s * 1.74533 */
        imuData.GyroX = (sint16)(imuCal.gyro.x * 1.74533f);
        imuData.GyroY = (sint16)(imuCal.gyro.y * 1.74533f);
        imuData.GyroZ = (sint16)(imuCal.gyro.z * 1.74533f);

        ComStack_SendImuData(&imuData);
        App_TxCount++;
    }
}

/* ===================[Telemetry]=================== */

/* Telemetry rate divider: send every N calls (50Hz / 50 = 1Hz) */
#define TELEMETRY_DIVIDER   (50u)
static uint8 App_TelemetryCounter = 0u;

/**
 * @brief Transmit system telemetry via ComStack (CMD 0x30, 1Hz)
 * @details Gathers current, temperature, battery, and velocity data
 *          into a single 20-byte STATUS packet for the GUI.
 */
static void App_TransmitTelemetry(void)
{
    ComStack_StatusDataType status;
    Robot_StatusType robotStatus;
    Robot_OdometryType odom;
    float32 tempMotors = 0.0f, tempMCU = 0.0f, tempBattery = 0.0f;

    /* Get robot state (already reads current + thermal internally) */
    (void)Robot_GetStatus(&robotStatus);
    (void)Robot_GetOdometry(&odom);

    status.SystemState      = (uint8)robotStatus.State;
    status.ErrorFlags       = (uint8)robotStatus.ErrorFlags;

    /* Battery — TODO: replace hardcoded 12.0V with real ADC */
    status.BatteryVoltageMv = (sint16)(robotStatus.BatteryVoltage * 1000.0f);
    /* Simple linear estimate: 12.6V=100%, 10.5V=0% */
    if (robotStatus.BatteryVoltage >= 12.6f)
    {
        status.BatteryPercent = 100u;
    }
    else if (robotStatus.BatteryVoltage <= 10.5f)
    {
        status.BatteryPercent = 0u;
    }
    else
    {
        status.BatteryPercent = (uint8)(((robotStatus.BatteryVoltage - 10.5f) / 2.1f) * 100.0f);
    }

    /* Motor currents (already read by Robot_GetStatus) */
    status.LeftCurrentMa  = (sint16)(robotStatus.LeftCurrent * 1000.0f);
    status.RightCurrentMa = (sint16)(robotStatus.RightCurrent * 1000.0f);

    /* Zone temperatures */
    (void)ThermalMgmt_GetZoneTemperature(THERMALMGMT_ZONE_MOTORS, &tempMotors);
    (void)ThermalMgmt_GetZoneTemperature(THERMALMGMT_ZONE_MCU, &tempMCU);
    (void)ThermalMgmt_GetZoneTemperature(THERMALMGMT_ZONE_BATTERY, &tempBattery);
    status.TempMotors  = (sint16)(tempMotors * 10.0f);
    status.TempMCU     = (sint16)(tempMCU * 10.0f);
    status.TempBattery = (sint16)(tempBattery * 10.0f);

    /* Fan speed */
    status.FanSpeedPercent = ThermalMgmt_GetFanSpeed();

    /* Current velocity from odometry */
    status.LinearVelMmps   = (sint16)(odom.LinearVel * 1000.0f);
    status.AngularVelMrads = (sint16)(odom.AngularVel * 1000.0f);

    ComStack_SendStatus(&status);
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
    App_TelemetryCounter = 0u;
    App_CommInitialized = TRUE;
}

/**
 * @brief Communication task main function (called by FreeRTOS task @ 50Hz)
 * @details
 *   1. Process incoming ComStack binary packets (motor commands, heartbeat)
 *   2. Transmit encoder data to ROS2 (binary, 50Hz)
 *   3. Transmit IMU data to ROS2 (binary, 50Hz)
 *   4. Transmit telemetry to ROS2/GUI (binary, 1Hz)
 */
void App_CommTask_Run(void)
{
    if (App_CommInitialized == FALSE)
    {
        App_CommTask_Init();
    }

    /* 1. Process incoming binary packets from ROS2 */
    App_ProcessComStackRx();

    /* 2. Transmit sensor data to ROS2 (binary protocol, 50Hz) */
    App_TransmitEncoderData();
    App_TransmitImuData();

    /* 3. Transmit telemetry at ~1Hz (every 50th call) */
    App_TelemetryCounter++;
    if (App_TelemetryCounter >= TELEMETRY_DIVIDER)
    {
        App_TelemetryCounter = 0u;
        App_TransmitTelemetry();
    }
}
