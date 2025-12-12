/**
 * @file MOTOR_Cfg.h
 * @brief Motor Driver Configuration Header
 * @details Compile-time configuration for the motor driver
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef ECUAL_MOTOR_MOTOR_CFG_H_
#define ECUAL_MOTOR_MOTOR_CFG_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"
#include "MOTOR_Types.h"

/* ===================[Pre-Compile Options]=================== */
#define MOTOR_DEV_ERROR_DETECT            (STD_ON)    /**< Enable/Disable development error detection */
#define MOTOR_VERSION_INFO_API             (STD_ON)    /**< Enable/Disable version info API */
#define MOTOR_DE_INIT_API                  (STD_ON)    /**< Enable/Disable de-initialization API */

/* ===================[Motor Driver Limits]=================== */
#define MOTOR_MAX_CHANNELS                 (2u)        /**< Maximum number of motor channels */
#define MOTOR_MAX_SPEED_PERCENT            (100u)      /**< Maximum speed percentage */
#define MOTOR_MIN_SPEED_PERCENT            (0u)         /**< Minimum speed percentage */

/* ===================[External Configuration]=================== */
extern const Motor_ConfigType Motor_Config;

#endif /* ECUAL_MOTOR_MOTOR_CFG_H_ */
