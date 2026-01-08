/**
 * @file App_CommTask.c
 * @brief Communication Task Implementation
 * @details Handles ROS2 communication via UART
 *
 * Responsibilities:
 * - Process incoming packets from ComStack
 * - Forward motor commands to Control task
 * - Transmit sensor data to ROS2
 * - Handle ping/status requests
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "../CONFIG/Std_Types.h"

/* Service includes */
#include "../SERVICES/COMM/ComStack.h"

/* App layer includes */
extern void App_ControlTask_SetWheelSpeeds(float32 LeftSpeed, float32 RightSpeed);
extern Std_ReturnType App_SensorTask_GetImuData(void* DataPtr);
extern uint32 App_SafetyTask_GetFaults(void);

/* ===================[External Configurations]=================== */
extern const ComStack_ConfigType ComStack_Config;

/* ===================[Private Variables]=================== */
static boolean App_CommInitialized = FALSE;
static uint32 App_CommTxCounter = 0u;

/* ===================[Private Functions]=================== */

/**
 * @brief Process a received packet
 */
static void App_Comm_ProcessPacket(const ComStack_PacketType* Packet)
{
    sint16 leftSpeed;
    sint16 rightSpeed;
    
    switch (Packet->Command)
    {
        case COMSTACK_CMD_MOTOR_CMD:
            /* Parse motor command */
            if (Packet->Length >= 4u)
            {
                leftSpeed = (sint16)(((uint16)Packet->Data[1] << 8u) | (uint16)Packet->Data[0]);
                rightSpeed = (sint16)(((uint16)Packet->Data[3] << 8u) | (uint16)Packet->Data[2]);
                
                /* Forward to control task */
                App_ControlTask_SetWheelSpeeds((float32)leftSpeed, (float32)rightSpeed);
            }
            break;
            
        case COMSTACK_CMD_MOTOR_STOP:
            /* Emergency stop */
            App_ControlTask_SetWheelSpeeds(0.0f, 0.0f);
            break;
            
        case COMSTACK_CMD_STATUS:
            /* Status request - send status response */
            {
                ComStack_StatusDataType status;
                status.SystemState = 0u;
                status.ErrorFlags = (uint8)App_SafetyTask_GetFaults();
                status.BatteryVoltage = 12.0f;  /* TODO: Read from ADC */
                status.MaxTemperature = 25.0f;  /* TODO: Get from thermal */
                (void)ComStack_SendStatus(&status);
            }
            break;
            
        case COMSTACK_CMD_PING:
            /* Respond with ACK */
            (void)ComStack_SendAck();
            break;
            
        case COMSTACK_CMD_CONFIG:
            /* Configuration update - TODO */
            break;
            
        default:
            /* Unknown command - NACK */
            (void)ComStack_SendNack(0x01u);
            break;
    }
}

/**
 * @brief Transmit sensor data periodically
 */
static void App_Comm_TransmitSensorData(void)
{
    /* Send IMU data every 2nd call (40ms = 25Hz) */
    if ((App_CommTxCounter % 2u) == 0u)
    {
        /* TODO: Get and send IMU data */
        /* ComStack_SendImuData(&imuData); */
    }
    
    /* Send encoder data every call (50ms = 20Hz) */
    {
        /* TODO: Get and send encoder data */
        /* ComStack_SendEncoderData(&encoderData); */
    }
    
    /* Send GPS data every 20th call (1 second) */
    if ((App_CommTxCounter % 20u) == 0u)
    {
        /* TODO: Get and send GPS data */
        /* ComStack_SendGpsData(&gpsData); */
    }
    
    App_CommTxCounter++;
}

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize communication task
 */
void App_CommTask_Init(void)
{
    /* ComStack should be initialized by System_Init */
    App_CommTxCounter = 0u;
    App_CommInitialized = TRUE;
}

/**
 * @brief Communication task main function (called by FreeRTOS task)
 */
void App_CommTask_Run(void)
{
    ComStack_PacketType packet;
    
    if (!App_CommInitialized)
    {
        App_CommTask_Init();
    }
    
    /* 1. Run ComStack main function to process UART bytes */
    ComStack_MainFunction();
    
    /* 2. Process all received packets */
    while (ComStack_IsPacketAvailable())
    {
        if (ComStack_GetPacket(&packet) == COMSTACK_RX_OK)
        {
            App_Comm_ProcessPacket(&packet);
        }
    }
    
    /* 3. Transmit sensor data */
    App_Comm_TransmitSensorData();
    
    /* 4. Send heartbeat if no recent TX (every 20 calls = 1 second) */
    if ((App_CommTxCounter % 20u) == 0u)
    {
        (void)ComStack_SendPing();
    }
}
