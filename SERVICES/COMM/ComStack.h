/**
 * @file ComStack.h
 * @brief Communication Stack Service API
 * @details UART-based ROS2 communication protocol handler
 *
 * Protocol: Simple framed packet protocol
 * | START (0xAA) | CMD | LEN | DATA[0..LEN-1] | CHECKSUM | END (0x55) |
 *
 * Features:
 * - Bidirectional communication with Raspberry Pi 5
 * - Packet framing with checksum validation
 * - Support for sensor data transmission and motor commands
 * - Connection monitoring with heartbeat
 * - Receive/transmit callbacks
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#ifndef COMSTACK_H
#define COMSTACK_H

/* ===================[Includes]=================== */
#include "ComStack_Types.h"
#include "ComStack_Cfg.h"

/* ===================[API Declarations]=================== */

/**
 * @brief Initialize the Communication Stack
 * @param[in] ConfigPtr Pointer to configuration structure
 * @return None
 * @pre UART driver must be initialized
 */
void ComStack_Init(const ComStack_ConfigType* ConfigPtr);

#if (COMSTACK_DEINIT_API == STD_ON)
/**
 * @brief De-initialize the Communication Stack
 * @return None
 */
void ComStack_DeInit(void);
#endif

/**
 * @brief Main function - call periodically for RX/TX processing
 * @return None
 * @note Should be called frequently (every 1-10ms)
 */
void ComStack_MainFunction(void);

/**
 * @brief Send a packet to ROS2
 * @param[in] Command Command ID
 * @param[in] Data Pointer to payload data
 * @param[in] Length Payload length (0-120)
 * @return TX result status
 */
ComStack_TxResultType ComStack_SendPacket(ComStack_CommandType Command, 
                                          const uint8* Data, 
                                          uint8 Length);

/**
 * @brief Send an acknowledgment packet
 * @return TX result status
 */
ComStack_TxResultType ComStack_SendAck(void);

/**
 * @brief Send a negative acknowledgment packet
 * @param[in] ErrorCode Error code to send
 * @return TX result status
 */
ComStack_TxResultType ComStack_SendNack(uint8 ErrorCode);

/**
 * @brief Send a ping/heartbeat packet
 * @return TX result status
 */
ComStack_TxResultType ComStack_SendPing(void);

/**
 * @brief Check if a packet is available
 * @return TRUE if packet available, FALSE otherwise
 */
boolean ComStack_IsPacketAvailable(void);

/**
 * @brief Get the next received packet
 * @param[out] PacketPtr Pointer to store received packet
 * @return RX result status
 */
ComStack_RxResultType ComStack_GetPacket(ComStack_PacketType* PacketPtr);

/**
 * @brief Send motor command data
 * @param[in] MotorData Pointer to motor command data
 * @return TX result status
 */
ComStack_TxResultType ComStack_SendMotorResponse(const ComStack_MotorCmdType* MotorData);

#if (FEATURE_GPS_ENABLED == 1u)
/**
 * @brief Send GPS data
 * @param[in] GpsData Pointer to GPS data
 * @return TX result status
 */
ComStack_TxResultType ComStack_SendGpsData(const ComStack_GpsDataType* GpsData);
#endif

/**
 * @brief Send IMU data
 * @param[in] ImuData Pointer to IMU data
 * @return TX result status
 */
ComStack_TxResultType ComStack_SendImuData(const ComStack_ImuDataType* ImuData);

/**
 * @brief Send encoder data
 * @param[in] EncoderData Pointer to encoder data
 * @return TX result status
 */
ComStack_TxResultType ComStack_SendEncoderData(const ComStack_EncoderDataType* EncoderData);

/**
 * @brief Send system status
 * @param[in] StatusData Pointer to status data
 * @return TX result status
 */
ComStack_TxResultType ComStack_SendStatus(const ComStack_StatusDataType* StatusData);

/**
 * @brief Get driver status
 * @return Current status
 */
ComStack_StatusType ComStack_GetStatus(void);

/**
 * @brief Check if connected to ROS2
 * @return TRUE if connected, FALSE otherwise
 */
boolean ComStack_IsConnected(void);

/**
 * @brief Get connection uptime in milliseconds
 * @return Uptime in ms
 */
uint32 ComStack_GetUptime(void);

/**
 * @brief Get statistics
 * @param[out] TxCount Pointer to store TX packet count
 * @param[out] RxCount Pointer to store RX packet count
 * @param[out] ErrorCount Pointer to store error count
 * @return None
 */
void ComStack_GetStatistics(uint32* TxCount, uint32* RxCount, uint32* ErrorCount);

#if (COMSTACK_RX_CALLBACK_API == STD_ON)
/**
 * @brief Set receive callback
 * @param[in] Callback Function to call when packet received
 */
void ComStack_SetRxCallback(ComStack_RxCallbackType Callback);
#endif

#if (COMSTACK_TX_CALLBACK_API == STD_ON)
/**
 * @brief Set transmit complete callback
 * @param[in] Callback Function to call when transmission complete
 */
void ComStack_SetTxCallback(ComStack_TxCallbackType Callback);
#endif

#if (COMSTACK_VERSION_INFO_API == STD_ON)
/**
 * @brief Get service version information
 * @param[out] versionInfoPtr Pointer to version info structure
 */
void ComStack_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
#endif

#endif /* COMSTACK_H */
