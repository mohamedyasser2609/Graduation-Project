/**
 * @file ComStack_Handler.c
 * @brief Command Handler for Communication Stack
 * @details Processes received commands and dispatches to appropriate handlers
 *
 * This module provides:
 * - Motor command parsing and forwarding
 * - Sensor data collection and transmission
 * - System status reporting
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#include "ComStack.h"

/* ===================[External Declarations]=================== */
/* These should be provided by the respective drivers */
/* Motor control (from ECUAL/MOTOR) */
extern Std_ReturnType Motor_SetSpeedAndDirection(uint8 Channel, uint8 Speed, uint8 Direction);
extern void Motor_StopAll(void);

/* ===================[Type Definitions]=================== */

/**
 * @brief Motor command data (from ROS2)
 */
typedef struct
{
    sint16  LeftSpeed;      /**< Left motor speed (-100 to +100) */
    sint16  RightSpeed;     /**< Right motor speed (-100 to +100) */
} ComStack_MotorCmdDataType;

/* ===================[Public Functions]=================== */

/**
 * @brief Process a motor command packet
 * @param[in] Packet Received packet containing motor command
 * @return E_OK if command processed, E_NOT_OK on error
 */
Std_ReturnType ComStack_ProcessMotorCommand(const ComStack_PacketType* Packet)
{
    sint16 leftSpeed;
    sint16 rightSpeed;
    uint8 leftDir;
    uint8 rightDir;
    uint8 leftAbsSpeed;
    uint8 rightAbsSpeed;
    
    if (Packet == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    if (Packet->Length < 4u)
    {
        return E_NOT_OK;
    }
    
    /* Parse motor speeds (2 bytes each, signed) */
    leftSpeed = (sint16)(((uint16)Packet->Data[1] << 8u) | (uint16)Packet->Data[0]);
    rightSpeed = (sint16)(((uint16)Packet->Data[3] << 8u) | (uint16)Packet->Data[2]);
    
    /* Determine direction and absolute speed */
    if (leftSpeed >= 0)
    {
        leftDir = 1u;  /* Forward */
        leftAbsSpeed = (uint8)((leftSpeed > 100) ? 100 : leftSpeed);
    }
    else
    {
        leftDir = 0u;  /* Reverse */
        leftAbsSpeed = (uint8)((leftSpeed < -100) ? 100 : (uint8)(-leftSpeed));
    }
    
    if (rightSpeed >= 0)
    {
        rightDir = 1u;  /* Forward */
        rightAbsSpeed = (uint8)((rightSpeed > 100) ? 100 : rightSpeed);
    }
    else
    {
        rightDir = 0u;  /* Reverse */
        rightAbsSpeed = (uint8)((rightSpeed < -100) ? 100 : (uint8)(-rightSpeed));
    }
    
    /* Apply motor commands */
    /* Note: Motor channels 0 = Left, 1 = Right */
    (void)Motor_SetSpeedAndDirection(0u, leftAbsSpeed, leftDir);
    (void)Motor_SetSpeedAndDirection(1u, rightAbsSpeed, rightDir);
    
    return E_OK;
}

/**
 * @brief Process a motor stop command
 * @return E_OK if command processed
 */
Std_ReturnType ComStack_ProcessMotorStop(void)
{
    Motor_StopAll();
    return E_OK;
}

/**
 * @brief Handle the received packet based on command type
 * @param[in] Packet Received packet to process
 * @return E_OK if handled, E_NOT_OK if unknown command
 */
Std_ReturnType ComStack_HandleCommand(const ComStack_PacketType* Packet)
{
    Std_ReturnType status = E_OK;
    
    if (Packet == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    switch (Packet->Command)
    {
        case COMSTACK_CMD_PING:
            /* Respond with ACK */
            (void)ComStack_SendAck();
            break;
            
        case COMSTACK_CMD_ACK:
            /* Acknowledgment received - nothing to do */
            break;
            
        case COMSTACK_CMD_NACK:
            /* Negative acknowledgment - log error */
            break;
            
        case COMSTACK_CMD_MOTOR_CMD:
            status = ComStack_ProcessMotorCommand(Packet);
            break;
            
        case COMSTACK_CMD_MOTOR_STOP:
            status = ComStack_ProcessMotorStop();
            break;
            
        case COMSTACK_CMD_STATUS:
            /* Status request - will be handled by App task */
            break;
            
        case COMSTACK_CMD_CONFIG:
            /* Configuration command - parse and apply */
            break;
            
        case COMSTACK_CMD_CALIBRATE:
            /* Calibration command - trigger calibration */
            break;
            
        default:
            /* Unknown command */
            (void)ComStack_SendNack(0x01u);  /* Unknown command error */
            status = E_NOT_OK;
            break;
    }
    
    return status;
}
