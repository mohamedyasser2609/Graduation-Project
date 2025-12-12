/**
 * @file MOTOR.h
 * @brief Motor Driver API for Cytron MDD10A Rev2.0
 * @details AUTOSAR-compliant API for dual-channel motor control
 *
 * Hardware: Cytron MDD10A Rev2.0 Dual Channel Motor Driver
 * - 2 independent motor channels
 * - PWM speed control
 * - GPIO direction control
 * - Supports forward, reverse, brake, and coast modes
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef ECUAL_MOTOR_MOTOR_H_
#define ECUAL_MOTOR_MOTOR_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"
#include "../../CONFIG/Compiler.h"
#include "../../CONFIG/Det.h"
#include "MOTOR_Types.h"
#include "MOTOR_Cfg.h"

/* ===================[Version Information]=================== */
#define MOTOR_VENDOR_ID                    (0x1234u)
#define MOTOR_MODULE_ID                    (300u)
#define MOTOR_INSTANCE_ID                  (0u)

#define MOTOR_SW_MAJOR_VERSION             (1u)
#define MOTOR_SW_MINOR_VERSION             (0u)
#define MOTOR_SW_PATCH_VERSION             (0u)

#define MOTOR_AR_RELEASE_MAJOR_VERSION      (4u)
#define MOTOR_AR_RELEASE_MINOR_VERSION     (4u)
#define MOTOR_AR_RELEASE_REVISION_VERSION  (0u)

/* ===================[Service IDs]=================== */
#define MOTOR_INIT_SID                     (0x00u)
#define MOTOR_DEINIT_SID                   (0x01u)
#define MOTOR_SET_SPEED_SID                (0x02u)
#define MOTOR_SET_DIRECTION_SID             (0x03u)
#define MOTOR_SET_SPEED_AND_DIRECTION_SID  (0x04u)
#define MOTOR_STOP_SID                     (0x05u)
#define MOTOR_GET_STATUS_SID               (0x06u)
#define MOTOR_GET_DATA_SID                 (0x07u)

/* ===================[Error Codes]=================== */
#define MOTOR_E_PARAM_POINTER              (0x01u)
#define MOTOR_E_UNINIT                     (0x02u)
#define MOTOR_E_PARAM_CHANNEL              (0x03u)
#define MOTOR_E_PARAM_SPEED                (0x04u)
#define MOTOR_E_PARAM_DIRECTION            (0x05u)

/* ===================[API Function Prototypes]=================== */

/**
 * @brief Initialize the motor driver
 * @param ConfigPtr Pointer to motor configuration structure
 */
void Motor_Init(const Motor_ConfigType* ConfigPtr);

#if (MOTOR_DE_INIT_API == STD_ON)
/**
 * @brief De-initialize the motor driver
 */
void Motor_DeInit(void);
#endif

/**
 * @brief Set motor speed (0-100%)
 * @param Channel Motor channel identifier
 * @param SpeedPercent Speed percentage (0-100)
 * @return Std_ReturnType (E_OK or E_NOT_OK)
 */
Std_ReturnType Motor_SetSpeed(Motor_ChannelType Channel, Motor_SpeedType SpeedPercent);

/**
 * @brief Set motor direction
 * @param Channel Motor channel identifier
 * @param Direction Motor direction (FORWARD, REVERSE, BRAKE, COAST)
 * @return Std_ReturnType (E_OK or E_NOT_OK)
 */
Std_ReturnType Motor_SetDirection(Motor_ChannelType Channel, Motor_DirectionType Direction);

/**
 * @brief Set motor speed and direction in one call
 * @param Channel Motor channel identifier
 * @param SpeedPercent Speed percentage (0-100)
 * @param Direction Motor direction
 * @return Std_ReturnType (E_OK or E_NOT_OK)
 */
Std_ReturnType Motor_SetSpeedAndDirection(Motor_ChannelType Channel, Motor_SpeedType SpeedPercent, Motor_DirectionType Direction);

/**
 * @brief Stop motor (brake mode)
 * @param Channel Motor channel identifier
 * @return Std_ReturnType (E_OK or E_NOT_OK)
 */
Std_ReturnType Motor_Stop(Motor_ChannelType Channel);

/**
 * @brief Stop all motors (brake mode)
 */
void Motor_StopAll(void);

/**
 * @brief Get motor status
 * @param Channel Motor channel identifier
 * @return Motor_StatusType status
 */
Motor_StatusType Motor_GetStatus(Motor_ChannelType Channel);

/**
 * @brief Get all motor data (status, direction, speed)
 * @param Channel Motor channel identifier
 * @param DataPtr Pointer to Motor_DataType structure to fill
 * @return Std_ReturnType (E_OK or E_NOT_OK)
 */
Std_ReturnType Motor_GetData(Motor_ChannelType Channel, Motor_DataType* DataPtr);

#if (MOTOR_VERSION_INFO_API == STD_ON)
/**
 * @brief Retrieve version information for the motor driver
 * @param versionInfoPtr Pointer to version info structure
 */
void Motor_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
#endif

#endif /* ECUAL_MOTOR_MOTOR_H_ */
