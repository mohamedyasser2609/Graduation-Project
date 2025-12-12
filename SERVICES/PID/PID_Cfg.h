/**
 * @file PID_Cfg.h
 * @brief PID Controller Configuration Header
 * @details Compile-time configuration switches for the PID controller.
 */

#ifndef SERVICES_PID_PID_CFG_H_
#define SERVICES_PID_PID_CFG_H_

#include "../../CONFIG/Std_Types.h"

/* ===================[Pre-Compile Options]=================== */
#define PID_DEV_ERROR_DETECT    (STD_ON)
#define PID_VERSION_INFO_API    (STD_ON)

/* ===================[External Configuration]=================== */
extern const PID_ConfigType PID_MotorSpeed_Config;

#endif /* SERVICES_PID_PID_CFG_H_ */

