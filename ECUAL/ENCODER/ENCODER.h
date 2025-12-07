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
#include "../../CONFIG/Std_Types.h"
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
 * @param ConfigPtr Pointer to encoder configuration structure
 */
void Encoder_Init(const Encoder_ConfigType* ConfigPtr);

/**
 * @brief De-initialize the encoder driver
 */
void Encoder_DeInit(void);

/**
 * @brief Get current position in specified unit
 * @param Unit Position unit (COUNTS, REVOLUTIONS, or DEGREES)
 * @return Position value (uint32 for COUNTS, float for REVOLUTIONS/DEGREES)
 * @note For COUNTS: returns uint32 cast to float
 * @note For REVOLUTIONS/DEGREES: returns float value
 */
float Encoder_GetPosition(Encoder_UnitType Unit);

/**
 * @brief Get current position in quadrature counts
 * @return Position in counts (0 to MaxPosition)
 */
uint32 Encoder_GetPositionCounts(void);

/**
 * @brief Get current position in revolutions
 * @return Position in revolutions (float)
 */
float Encoder_GetPositionRevolutions(void);

/**
 * @brief Get current position in degrees
 * @return Position in degrees (0.0 to 360.0)
 */
float Encoder_GetPositionDegrees(void);

/**
 * @brief Get current velocity in specified unit
 * @param Unit Velocity unit (COUNTS_PER_SEC or RPM)
 * @return Velocity value (float)
 */
float Encoder_GetVelocity(Encoder_VelocityUnitType Unit);

/**
 * @brief Get current velocity in counts per second
 * @return Velocity in counts/second
 */
uint32 Encoder_GetVelocityCountsPerSec(void);

/**
 * @brief Get current velocity in RPM
 * @return Velocity in revolutions per minute
 */
float Encoder_GetVelocityRPM(void);

/**
 * @brief Get current rotation direction
 * @return Encoder_DirectionType (FORWARD or REVERSE)
 */
Encoder_DirectionType Encoder_GetDirection(void);

/**
 * @brief Get encoder status
 * @return Encoder_StatusType status
 */
Encoder_StatusType Encoder_GetStatus(void);

/**
 * @brief Reset encoder position to zero
 */
void Encoder_ResetPosition(void);

/**
 * @brief Set encoder position to a specific value
 * @param Position New position value in counts
 */
void Encoder_SetPosition(uint32 Position);

/**
 * @brief Get all encoder data (position, velocity, direction)
 * @param DataPtr Pointer to Encoder_DataType structure to fill
 * @return Std_ReturnType (E_OK or E_NOT_OK)
 */
Std_ReturnType Encoder_GetData(Encoder_DataType* DataPtr);

/**
 * @brief Update encoder data (call periodically for filtered velocity)
 * @note This function should be called periodically (e.g., every 10-100ms)
 *       to update filtered velocity calculations
 */
void Encoder_Update(void);

#if (ENCODER_VERSION_INFO_API == STD_ON)
/**
 * @brief Retrieve version information for the encoder driver
 * @param versionInfoPtr Pointer to version info structure
 */
void Encoder_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
#endif

#endif /* ECUAL_ENCODER_ENCODER_H_ */
