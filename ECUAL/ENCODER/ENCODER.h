/**
 * @file ENCODER.h
 * @brief Encoder Driver API for EMG49 Motor Encoder
 * @details AUTOSAR-compliant API for encoder position and velocity measurement
 *
 * @author Mohamed Yasser
 * @date Nov 5, 2025
 * @version 1.0.0
 */

#ifndef ECUAL_ENCODER_ENCODER_H_
#define ECUAL_ENCODER_ENCODER_H_

/* ===================[Includes]=================== */
#include <Std_types.h>
#include "../../CONFIG/Compiler.h"
#include "../../CONFIG/Det.h"
#include "ENCODER_Types.h"
#include "ENCODER_Cfg.h"

/* ===================[Version Information]=================== */
#define ENCODER_VENDOR_ID                    (0x1234u)
#define ENCODER_MODULE_ID                    (200u)
#define ENCODER_INSTANCE_ID                  (0u)

#define ENCODER_SW_MAJOR_VERSION             (1u)
#define ENCODER_SW_MINOR_VERSION             (0u)
#define ENCODER_SW_PATCH_VERSION             (0u)

#define ENCODER_AR_RELEASE_MAJOR_VERSION     (4u)
#define ENCODER_AR_RELEASE_MINOR_VERSION     (4u)
#define ENCODER_AR_RELEASE_REVISION_VERSION  (0u)

/* ===================[Service IDs]=================== */
#define ENCODER_INIT_SID                     (0x00u)
#define ENCODER_DEINIT_SID                   (0x01u)
#define ENCODER_GET_POSITION_SID             (0x02u)
#define ENCODER_GET_VELOCITY_SID             (0x03u)
#define ENCODER_GET_DIRECTION_SID            (0x04u)
#define ENCODER_GET_STATUS_SID               (0x05u)
#define ENCODER_RESET_POSITION_SID           (0x06u)
#define ENCODER_GET_DATA_SID                 (0x07u)

/* ===================[Error Codes]=================== */
#define ENCODER_E_PARAM_POINTER              (0x01u)
#define ENCODER_E_UNINIT                     (0x02u)
#define ENCODER_E_PARAM_UNIT                 (0x03u)

/* ===================[API Function Prototypes]=================== */

/**
 * @brief Initialize the encoder driver
 * @param ConfigPtr Pointer to encoder configuration structure (channels array)
 */
void Encoder_Init(const Encoder_ConfigType* ConfigPtr);

/**
 * @brief De-initialize the encoder driver
 */
void Encoder_DeInit(void);

/**
 * @brief Get current position in specified unit
 * @param Channel Encoder channel
 * @param Unit Position unit (COUNTS, REVOLUTIONS, or DEGREES)
 * @return Position value as float (counts are promoted to float)
 */
float Encoder_GetPosition(Encoder_ChannelType Channel, Encoder_UnitType Unit);

/**
 * @brief Get current absolute position in quadrature counts (64-bit)
 * @param Channel Encoder channel
 * @return Position in counts
 */
int64_t Encoder_GetPositionCounts(const Encoder_ChannelType Channel);

/**
 * @brief Get current position in revolutions
 * @param Channel Encoder channel
 * @return Position in revolutions (float)
 */
float Encoder_GetPositionRevolutions(const Encoder_ChannelType Channel);

/**
 * @brief Get current position in degrees (normalized to 0-360)
 * @param Channel Encoder channel
 * @return Position in degrees
 */
float Encoder_GetPositionDegrees(const Encoder_ChannelType Channel);

/**
 * @brief Get current velocity in specified unit
 * @param Channel Encoder channel
 * @param Unit Velocity unit (COUNTS_PER_SEC or RPM)
 * @return Velocity value (signed)
 */
float Encoder_GetVelocity(Encoder_ChannelType Channel, Encoder_VelocityUnitType Unit);

/**
 * @brief Get current velocity in counts per second (signed)
 * @param Channel Encoder channel
 * @return Velocity in counts/second
 */
int32_t Encoder_GetVelocityCountsPerSec(const Encoder_ChannelType Channel);

/**
 * @brief Get current velocity in RPM (signed)
 * @param Channel Encoder channel
 * @return Velocity in revolutions per minute
 */
float Encoder_GetVelocityRPM(const Encoder_ChannelType Channel);

/**
 * @brief Get current rotation direction
 * @param Channel Encoder channel
 * @return Encoder_DirectionType (FORWARD or REVERSE)
 */
Encoder_DirectionType Encoder_GetDirection(const Encoder_ChannelType Channel);

/**
 * @brief Get encoder status
 * @param Channel Encoder channel
 * @return Encoder_StatusType status
 */
Encoder_StatusType Encoder_GetStatus(const Encoder_ChannelType Channel);

/**
 * @brief Reset encoder position and velocity state to zero
 * @param Channel Encoder channel
 */
void Encoder_ResetPosition(const Encoder_ChannelType Channel);

/**
 * @brief Set encoder position to a specific value
 * @param Channel Encoder channel
 * @param Position New position value in counts
 */
void Encoder_SetPosition(const Encoder_ChannelType Channel, uint32 Position);

/**
 * @brief Get all encoder data (position, velocity, direction)
 * @param Channel Encoder channel
 * @param DataPtr Pointer to Encoder_DataType structure to fill
 * @return Std_ReturnType (E_OK or E_NOT_OK)
 */
Std_ReturnType Encoder_GetData(const Encoder_ChannelType Channel, Encoder_DataType* DataPtr);

/**
 * @brief Update encoder data (call periodically for filtered velocity)
 * @param Channel Encoder channel
 * @note MUST be called at a fixed period (e.g., 1ms FreeRTOS task).
 *       The filtering assumes a consistent update interval.
 */
void Encoder_Update(const Encoder_ChannelType Channel);

/**
 * @brief Convenience helper to update all configured channels.
 */
void Encoder_UpdateAll(void);

#if (ENCODER_VERSION_INFO_API == STD_ON)
/**
 * @brief Retrieve version information for the encoder driver
 * @param versionInfoPtr Pointer to version info structure
 */
void Encoder_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
#endif

#endif /* ECUAL_ENCODER_ENCODER_H_ */
