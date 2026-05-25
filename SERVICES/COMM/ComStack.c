/**
 * @file ComStack.c
 * @brief Communication Stack Service Implementation
 * @details UART-based ROS2 communication protocol handler
 *
 * Protocol Implementation:
 * - State machine for packet reception
 * - XOR checksum calculation
 * - Packet queuing for TX/RX
 * - Connection monitoring
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#include "ComStack.h"
#include "../../MCAL/UART/Uart.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#if (COMSTACK_DEV_ERROR_DETECT == STD_ON)
#include "../../CONFIG/Det.h"
#endif

/* ===================[Private Types]=================== */
typedef struct
{
    ComStack_PacketType     Packets[COMSTACK_RX_QUEUE_SIZE];
    uint8                   Head;
    uint8                   Tail;
    uint8                   Count;
} ComStack_RxQueueType;

typedef struct
{
    ComStack_RxStateType    State;
    ComStack_PacketType     CurrentPacket;
    uint8                   DataIndex;
    uint32                  LastByteTime;
} ComStack_RxContextType;

/* ===================[Private Variables]=================== */
static const ComStack_ConfigType* ComStack_ConfigPtr = NULL_PTR;
static ComStack_StatusType ComStack_ModuleStatus = COMSTACK_STATUS_UNINIT;
static ComStack_RxQueueType ComStack_RxQueue;
static ComStack_RxContextType ComStack_RxContext;

/* Statistics */
static uint32 ComStack_TxPacketCount = 0u;
static uint32 ComStack_RxPacketCount = 0u;
static uint32 ComStack_ErrorCount = 0u;
static uint32 ComStack_LastRxTime = 0u;
static uint32 ComStack_StartTime = 0u;
static boolean ComStack_Connected = FALSE;

#if (COMSTACK_RX_CALLBACK_API == STD_ON)
static ComStack_RxCallbackType ComStack_RxCallback = NULL_PTR;
#endif

#if (COMSTACK_TX_CALLBACK_API == STD_ON)
static ComStack_TxCallbackType ComStack_TxCallback = NULL_PTR;
#endif

/* ===================[Private Function Declarations]=================== */
static uint8 ComStack_CalculateChecksum(ComStack_CommandType Cmd, uint8 Len, const uint8* Data);
static void ComStack_ProcessRxByte(uint8 Byte);
static void ComStack_QueueRxPacket(const ComStack_PacketType* Packet);
static boolean ComStack_IsRxQueueFull(void);
static boolean ComStack_IsRxQueueEmpty(void);
static void ComStack_TransmitPacket(const ComStack_PacketType* Packet);
static uint32 ComStack_GetTickMs(void);

/* ===================[Private Function Implementations]=================== */

/**
 * @brief Calculate XOR checksum
 */
static uint8 ComStack_CalculateChecksum(ComStack_CommandType Cmd, uint8 Len, const uint8* Data)
{
    uint8 checksum;
    uint8 i;
    
    checksum = Cmd ^ Len;
    
    if (Data != NULL_PTR)
    {
        for (i = 0u; i < Len; i++)
        {
            checksum ^= Data[i];
        }
    }
    
    return checksum;
}

/**
 * @brief Get current tick in milliseconds
 */
static uint32 ComStack_GetTickMs(void)
{
    return (uint32)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/**
 * @brief Process a received byte through state machine
 */
static void ComStack_ProcessRxByte(uint8 Byte)
{
    switch (ComStack_RxContext.State)
    {
        case COMSTACK_RX_WAIT_START:
            if (Byte == COMSTACK_START_BYTE)
            {
                ComStack_RxContext.State = COMSTACK_RX_WAIT_CMD;
                ComStack_RxContext.CurrentPacket.Valid = FALSE;
                ComStack_RxContext.DataIndex = 0u;
            }
            break;
            
        case COMSTACK_RX_WAIT_CMD:
            ComStack_RxContext.CurrentPacket.Command = Byte;
            ComStack_RxContext.State = COMSTACK_RX_WAIT_LEN;
            break;
            
        case COMSTACK_RX_WAIT_LEN:
            ComStack_RxContext.CurrentPacket.Length = Byte;
            if (Byte > COMSTACK_MAX_DATA_LENGTH)
            {
                /* Invalid length, reset */
                ComStack_RxContext.State = COMSTACK_RX_WAIT_START;
                ComStack_ErrorCount++;
                (void)Uart_SendString(UART_MODULE_0, (const uint8*)"[ComStack] Invalid Length!\r\n");
            }
            else if (Byte == 0u)
            {
                /* No data, go to checksum */
                ComStack_RxContext.State = COMSTACK_RX_WAIT_CHECKSUM;
            }
            else
            {
                ComStack_RxContext.DataIndex = 0u;
                ComStack_RxContext.State = COMSTACK_RX_WAIT_DATA;
            }
            break;
            
        case COMSTACK_RX_WAIT_DATA:
            ComStack_RxContext.CurrentPacket.Data[ComStack_RxContext.DataIndex] = Byte;
            ComStack_RxContext.DataIndex++;
            
            if (ComStack_RxContext.DataIndex >= ComStack_RxContext.CurrentPacket.Length)
            {
                ComStack_RxContext.State = COMSTACK_RX_WAIT_CHECKSUM;
            }
            break;
            
        case COMSTACK_RX_WAIT_CHECKSUM:
            ComStack_RxContext.CurrentPacket.Checksum = Byte;
            ComStack_RxContext.State = COMSTACK_RX_WAIT_END;
            break;
            
        case COMSTACK_RX_WAIT_END:
            if (Byte == COMSTACK_END_BYTE)
            {
                /* Validate checksum */
                uint8 calcChecksum = ComStack_CalculateChecksum(
                    ComStack_RxContext.CurrentPacket.Command,
                    ComStack_RxContext.CurrentPacket.Length,
                    ComStack_RxContext.CurrentPacket.Data);
                
                if (calcChecksum == ComStack_RxContext.CurrentPacket.Checksum)
                {
                    /* Valid packet */
                    ComStack_RxContext.CurrentPacket.Valid = TRUE;
                    ComStack_QueueRxPacket(&ComStack_RxContext.CurrentPacket);
                    ComStack_RxPacketCount++;
                    ComStack_LastRxTime = ComStack_GetTickMs();
                    ComStack_Connected = TRUE;
                    if (ComStack_RxContext.CurrentPacket.Command == COMSTACK_CMD_TIME_SYNC)
                    {
                        (void)Uart_SendString(UART_MODULE_0, (const uint8*)"[ComStack] RX TIME_SYNC Packet Valid!\r\n");
                    }
                    
                    #if (COMSTACK_RX_CALLBACK_API == STD_ON)
                    if (ComStack_RxCallback != NULL_PTR)
                    {
                        ComStack_RxCallback(&ComStack_RxContext.CurrentPacket);
                    }
                    #endif
                    
                    #if (COMSTACK_AUTO_ACK == STD_ON)
                    /* Auto-acknowledge if enabled and not already an ACK/PING */
                    if ((ComStack_RxContext.CurrentPacket.Command != COMSTACK_CMD_ACK) &&
                        (ComStack_RxContext.CurrentPacket.Command != COMSTACK_CMD_PING) &&
                        (ComStack_RxContext.CurrentPacket.Command != COMSTACK_CMD_NACK))
                    {
                        (void)ComStack_SendAck();
                    }
                    #endif
                }
                else
                {
                    /* Checksum error */
                    ComStack_ErrorCount++;
                    (void)Uart_SendString(UART_MODULE_0, (const uint8*)"[ComStack] Checksum Error!\r\n");
                }
            }
            else
            {
                /* Invalid end byte */
                ComStack_ErrorCount++;
                (void)Uart_SendString(UART_MODULE_0, (const uint8*)"[ComStack] Invalid End Byte!\r\n");
            }
            
            /* Reset state machine */
            ComStack_RxContext.State = COMSTACK_RX_WAIT_START;
            break;
            
        default:
            ComStack_RxContext.State = COMSTACK_RX_WAIT_START;
            break;
    }
    
    ComStack_RxContext.LastByteTime = ComStack_GetTickMs();
}

/**
 * @brief Queue a received packet
 */
static void ComStack_QueueRxPacket(const ComStack_PacketType* Packet)
{
    if (!ComStack_IsRxQueueFull())
    {
        ComStack_RxQueue.Packets[ComStack_RxQueue.Head] = *Packet;
        ComStack_RxQueue.Head = (ComStack_RxQueue.Head + 1u) % COMSTACK_RX_QUEUE_SIZE;
        ComStack_RxQueue.Count++;
    }
    else
    {
        /* Queue overflow */
        ComStack_ErrorCount++;
    }
}

/**
 * @brief Check if RX queue is full
 */
static boolean ComStack_IsRxQueueFull(void)
{
    return (ComStack_RxQueue.Count >= COMSTACK_RX_QUEUE_SIZE);
}

/**
 * @brief Check if RX queue is empty
 */
static boolean ComStack_IsRxQueueEmpty(void)
{
    return (ComStack_RxQueue.Count == 0u);
}

/**
 * @brief Transmit a packet over UART
 */
static void ComStack_TransmitPacket(const ComStack_PacketType* Packet)
{
    uint8 txBuffer[COMSTACK_MAX_PACKET_SIZE];
    uint8 txLen = 0u;
    uint8 i;
    uint8 checksum;
    
    /* Build packet */
    txBuffer[txLen++] = COMSTACK_START_BYTE;
    txBuffer[txLen++] = Packet->Command;
    txBuffer[txLen++] = Packet->Length;
    
    /* Copy data */
    for (i = 0u; i < Packet->Length; i++)
    {
        txBuffer[txLen++] = Packet->Data[i];
    }
    
    /* Calculate and add checksum */
    checksum = ComStack_CalculateChecksum(Packet->Command, Packet->Length, Packet->Data);
    txBuffer[txLen++] = checksum;
    txBuffer[txLen++] = COMSTACK_END_BYTE;
    
    /* Transmit via UART */
    for (i = 0u; i < txLen; i++)
    {
        Uart_SendByte(ComStack_ConfigPtr->UartModule, txBuffer[i]);
    }
    
    ComStack_TxPacketCount++;
    
    #if (COMSTACK_TX_CALLBACK_API == STD_ON)
    if (ComStack_TxCallback != NULL_PTR)
    {
        ComStack_TxCallback(COMSTACK_TX_OK);
    }
    #endif
}

/* ===================[Public Function Implementations]=================== */

/**
 * @brief Initialize the Communication Stack
 */
void ComStack_Init(const ComStack_ConfigType* ConfigPtr)
{
#if (COMSTACK_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(COMSTACK_MODULE_ID, COMSTACK_INSTANCE_ID, 
                       COMSTACK_INIT_SID, COMSTACK_E_PARAM_POINTER);
        return;
    }
    
    if (ComStack_ModuleStatus != COMSTACK_STATUS_UNINIT)
    {
        Det_ReportError(COMSTACK_MODULE_ID, COMSTACK_INSTANCE_ID, 
                       COMSTACK_INIT_SID, COMSTACK_E_ALREADY_INIT);
        return;
    }
#endif
    
    ComStack_ConfigPtr = ConfigPtr;
    
    /* Initialize RX queue */
    ComStack_RxQueue.Head = 0u;
    ComStack_RxQueue.Tail = 0u;
    ComStack_RxQueue.Count = 0u;
    
    /* Initialize RX context */
    ComStack_RxContext.State = COMSTACK_RX_WAIT_START;
    ComStack_RxContext.DataIndex = 0u;
    ComStack_RxContext.LastByteTime = 0u;
    ComStack_RxContext.CurrentPacket.Valid = FALSE;
    
    /* Reset statistics */
    ComStack_TxPacketCount = 0u;
    ComStack_RxPacketCount = 0u;
    ComStack_ErrorCount = 0u;
    ComStack_LastRxTime = 0u;
    ComStack_StartTime = ComStack_GetTickMs();
    ComStack_Connected = FALSE;
    
    ComStack_ModuleStatus = COMSTACK_STATUS_IDLE;
}

#if (COMSTACK_DEINIT_API == STD_ON)
/**
 * @brief De-initialize the Communication Stack
 */
void ComStack_DeInit(void)
{
#if (COMSTACK_DEV_ERROR_DETECT == STD_ON)
    if (ComStack_ModuleStatus == COMSTACK_STATUS_UNINIT)
    {
        Det_ReportError(COMSTACK_MODULE_ID, COMSTACK_INSTANCE_ID, 
                       COMSTACK_DEINIT_SID, COMSTACK_E_UNINIT);
        return;
    }
#endif
    
    #if (COMSTACK_RX_CALLBACK_API == STD_ON)
    ComStack_RxCallback = NULL_PTR;
    #endif
    
    #if (COMSTACK_TX_CALLBACK_API == STD_ON)
    ComStack_TxCallback = NULL_PTR;
    #endif
    
    ComStack_ConfigPtr = NULL_PTR;
    ComStack_ModuleStatus = COMSTACK_STATUS_UNINIT;
}
#endif

/**
 * @brief Main function - poll for received bytes
 */
void ComStack_MainFunction(void)
{
    uint8 rxByte;
    Std_ReturnType status;
    uint32 currentTime;
    
    if (ComStack_ModuleStatus == COMSTACK_STATUS_UNINIT)
    {
        return;
    }
    
    /* Process all available bytes */
    while (Uart_IsRxDataAvailable(ComStack_ConfigPtr->UartModule))
    {
        status = Uart_ReceiveByte(ComStack_ConfigPtr->UartModule, &rxByte);
        if (status == E_OK)
        {
            ComStack_ProcessRxByte(rxByte);
        }
    }
    
    /* Check for RX timeout (reset state machine if partial packet times out) */
    currentTime = ComStack_GetTickMs();
    if (ComStack_RxContext.State != COMSTACK_RX_WAIT_START)
    {
        if ((currentTime - ComStack_RxContext.LastByteTime) > ComStack_ConfigPtr->RxTimeoutMs)
        {
            /* Timeout, reset state machine */
            ComStack_RxContext.State = COMSTACK_RX_WAIT_START;
            ComStack_ErrorCount++;
            (void)Uart_SendString(UART_MODULE_0, (const uint8*)"[ComStack] RX Timeout!\r\n");
        }
    }
    
    /* Check connection timeout */
    if ((currentTime - ComStack_LastRxTime) > COMSTACK_CONNECTION_TIMEOUT_MS)
    {
        ComStack_Connected = FALSE;
    }
}

/**
 * @brief Send a packet to ROS2
 */
ComStack_TxResultType ComStack_SendPacket(ComStack_CommandType Command, 
                                          const uint8* Data, 
                                          uint8 Length)
{
    ComStack_PacketType packet;
    uint8 i;
    
#if (COMSTACK_DEV_ERROR_DETECT == STD_ON)
    if (ComStack_ModuleStatus == COMSTACK_STATUS_UNINIT)
    {
        return COMSTACK_TX_ERROR;
    }
    
    if ((Length > COMSTACK_MAX_DATA_LENGTH) || ((Length > 0u) && (Data == NULL_PTR)))
    {
        return COMSTACK_TX_ERROR;
    }
#endif
    
    packet.Command = Command;
    packet.Length = Length;
    
    for (i = 0u; i < Length; i++)
    {
        packet.Data[i] = Data[i];
    }
    
    ComStack_TransmitPacket(&packet);
    
    return COMSTACK_TX_OK;
}

/**
 * @brief Send an acknowledgment packet
 */
ComStack_TxResultType ComStack_SendAck(void)
{
    return ComStack_SendPacket(COMSTACK_CMD_ACK, NULL_PTR, 0u);
}

/**
 * @brief Send a negative acknowledgment packet
 */
ComStack_TxResultType ComStack_SendNack(uint8 ErrorCode)
{
    return ComStack_SendPacket(COMSTACK_CMD_NACK, &ErrorCode, 1u);
}

/**
 * @brief Send a ping/heartbeat packet
 */
ComStack_TxResultType ComStack_SendPing(void)
{
    uint32 uptime = ComStack_GetUptime();
    uint8 data[4];
    
    /* Pack uptime into data */
    data[0] = (uint8)(uptime & 0xFFu);
    data[1] = (uint8)((uptime >> 8u) & 0xFFu);
    data[2] = (uint8)((uptime >> 16u) & 0xFFu);
    data[3] = (uint8)((uptime >> 24u) & 0xFFu);
    
    return ComStack_SendPacket(COMSTACK_CMD_PING, data, 4u);
}

/**
 * @brief Check if a packet is available
 */
boolean ComStack_IsPacketAvailable(void)
{
    return !ComStack_IsRxQueueEmpty();
}

/**
 * @brief Get the next received packet
 */
ComStack_RxResultType ComStack_GetPacket(ComStack_PacketType* PacketPtr)
{
#if (COMSTACK_DEV_ERROR_DETECT == STD_ON)
    if (PacketPtr == NULL_PTR)
    {
        return COMSTACK_RX_NO_DATA;
    }
#endif
    
    if (ComStack_IsRxQueueEmpty())
    {
        return COMSTACK_RX_NO_DATA;
    }
    
    *PacketPtr = ComStack_RxQueue.Packets[ComStack_RxQueue.Tail];
    ComStack_RxQueue.Tail = (ComStack_RxQueue.Tail + 1u) % COMSTACK_RX_QUEUE_SIZE;
    ComStack_RxQueue.Count--;
    
    return COMSTACK_RX_OK;
}

#if (FEATURE_GPS_ENABLED == 1u)
/**
 * @brief Send GPS data
 */
ComStack_TxResultType ComStack_SendGpsData(const ComStack_GpsDataType* GpsData)
{
    uint8 data[22];
    
    if (GpsData == NULL_PTR)
    {
        return COMSTACK_TX_ERROR;
    }
    
    /* Pack GPS data */
    (void)memcpy(&data[0], &GpsData->Latitude, 4u);
    (void)memcpy(&data[4], &GpsData->Longitude, 4u);
    (void)memcpy(&data[8], &GpsData->Altitude, 4u);
    data[12] = GpsData->FixType;
    data[13] = GpsData->NumSatellites;
    
    return ComStack_SendPacket(COMSTACK_CMD_GPS_DATA, data, 14u);
}
#endif

/**
 * @brief Send IMU data (Timestamp + fixed-point sint16 × 100)
 */
ComStack_TxResultType ComStack_SendImuData(const ComStack_ImuDataType* ImuData)
{
    uint8 data[20];
    
    if (ImuData == NULL_PTR)
    {
        return COMSTACK_TX_ERROR;
    }
    
    /* Pack Timestamp (2 × uint32 = 8 bytes) */
    (void)memcpy(&data[0], &ImuData->TimestampSec, 4u);
    (void)memcpy(&data[4], &ImuData->TimestampNsec, 4u);

    /* Pack IMU data (6 × sint16 = 12 bytes) */
    (void)memcpy(&data[8],  &ImuData->AccelX, 2u);
    (void)memcpy(&data[10], &ImuData->AccelY, 2u);
    (void)memcpy(&data[12], &ImuData->AccelZ, 2u);
    (void)memcpy(&data[14], &ImuData->GyroX, 2u);
    (void)memcpy(&data[16], &ImuData->GyroY, 2u);
    (void)memcpy(&data[18], &ImuData->GyroZ, 2u);
    
    return ComStack_SendPacket(COMSTACK_CMD_IMU_DATA, data, 20u);
}

/**
 * @brief Send encoder data (Timestamp + ticks + velocity)
 */
ComStack_TxResultType ComStack_SendEncoderData(const ComStack_EncoderDataType* EncoderData)
{
    uint8 data[20];
    
    if (EncoderData == NULL_PTR)
    {
        return COMSTACK_TX_ERROR;
    }
    
    /* Pack Timestamp (2 × uint32 = 8 bytes) */
    (void)memcpy(&data[0], &EncoderData->TimestampSec, 4u);
    (void)memcpy(&data[4], &EncoderData->TimestampNsec, 4u);

    /* Pack encoder data: 2 × sint32 ticks + 2 × sint16 velocity = 12 bytes */
    (void)memcpy(&data[8], &EncoderData->LeftTicks, 4u);
    (void)memcpy(&data[12], &EncoderData->RightTicks, 4u);
    (void)memcpy(&data[16], &EncoderData->LeftVelocity, 2u);
    (void)memcpy(&data[18], &EncoderData->RightVelocity, 2u);
    
    return ComStack_SendPacket(COMSTACK_CMD_ENCODER_DATA, data, 20u);
}

/**
 * @brief Send system telemetry/status (CMD 0x30) — 20 bytes
 * @details Packs all telemetry fields as fixed-point integers.
 */
ComStack_TxResultType ComStack_SendStatus(const ComStack_StatusDataType* StatusData)
{
    uint8 data[20];
    
    if (StatusData == NULL_PTR)
    {
        return COMSTACK_TX_ERROR;
    }
    
    /* Pack telemetry — all little-endian */
    data[0]  = StatusData->SystemState;
    data[1]  = StatusData->ErrorFlags;
    data[2]  = (uint8)(StatusData->BatteryVoltageMv & 0xFFu);
    data[3]  = (uint8)((StatusData->BatteryVoltageMv >> 8u) & 0xFFu);
    data[4]  = StatusData->BatteryPercent;
    data[5]  = (uint8)(StatusData->LeftCurrentMa & 0xFFu);
    data[6]  = (uint8)((StatusData->LeftCurrentMa >> 8u) & 0xFFu);
    data[7]  = (uint8)(StatusData->RightCurrentMa & 0xFFu);
    data[8]  = (uint8)((StatusData->RightCurrentMa >> 8u) & 0xFFu);
    data[9]  = (uint8)(StatusData->TempMotors & 0xFFu);
    data[10] = (uint8)((StatusData->TempMotors >> 8u) & 0xFFu);
    data[11] = (uint8)(StatusData->TempMCU & 0xFFu);
    data[12] = (uint8)((StatusData->TempMCU >> 8u) & 0xFFu);
    data[13] = (uint8)(StatusData->TempBattery & 0xFFu);
    data[14] = (uint8)((StatusData->TempBattery >> 8u) & 0xFFu);
    data[15] = StatusData->FanSpeedPercent;
    data[16] = (uint8)(StatusData->LinearVelMmps & 0xFFu);
    data[17] = (uint8)((StatusData->LinearVelMmps >> 8u) & 0xFFu);
    data[18] = (uint8)(StatusData->AngularVelMrads & 0xFFu);
    data[19] = (uint8)((StatusData->AngularVelMrads >> 8u) & 0xFFu);
    
    return ComStack_SendPacket(COMSTACK_CMD_STATUS, data, 20u);
}

/**
 * @brief Get driver status
 */
ComStack_StatusType ComStack_GetStatus(void)
{
    return ComStack_ModuleStatus;
}

/**
 * @brief Check if connected to ROS2
 */
boolean ComStack_IsConnected(void)
{
    return ComStack_Connected;
}

/**
 * @brief Get connection uptime in milliseconds
 */
uint32 ComStack_GetUptime(void)
{
    return ComStack_GetTickMs() - ComStack_StartTime;
}

/**
 * @brief Get statistics
 */
void ComStack_GetStatistics(uint32* TxCount, uint32* RxCount, uint32* ErrorCount)
{
    if (TxCount != NULL_PTR)
    {
        *TxCount = ComStack_TxPacketCount;
    }
    if (RxCount != NULL_PTR)
    {
        *RxCount = ComStack_RxPacketCount;
    }
    if (ErrorCount != NULL_PTR)
    {
        *ErrorCount = ComStack_ErrorCount;
    }
}

#if (COMSTACK_RX_CALLBACK_API == STD_ON)
/**
 * @brief Set receive callback
 */
void ComStack_SetRxCallback(ComStack_RxCallbackType Callback)
{
    ComStack_RxCallback = Callback;
}
#endif

#if (COMSTACK_TX_CALLBACK_API == STD_ON)
/**
 * @brief Set transmit complete callback
 */
void ComStack_SetTxCallback(ComStack_TxCallbackType Callback)
{
    ComStack_TxCallback = Callback;
}
#endif

#if (COMSTACK_VERSION_INFO_API == STD_ON)
/**
 * @brief Get service version information
 */
void ComStack_GetVersionInfo(Std_VersionInfoType* versionInfoPtr)
{
#if (COMSTACK_DEV_ERROR_DETECT == STD_ON)
    if (versionInfoPtr == NULL_PTR)
    {
        Det_ReportError(COMSTACK_MODULE_ID, COMSTACK_INSTANCE_ID, 
                       COMSTACK_GET_VERSION_SID, COMSTACK_E_PARAM_POINTER);
        return;
    }
#endif
    
    versionInfoPtr->vendorID = COMSTACK_VENDOR_ID;
    versionInfoPtr->moduleID = COMSTACK_MODULE_ID;
    versionInfoPtr->sw_major_version = COMSTACK_SW_MAJOR_VERSION;
    versionInfoPtr->sw_minor_version = COMSTACK_SW_MINOR_VERSION;
    versionInfoPtr->sw_patch_version = COMSTACK_SW_PATCH_VERSION;
}
#endif
