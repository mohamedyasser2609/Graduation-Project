/**
 * @file NVIC_Cfg.h
 * @brief NVIC Driver Configuration Header
 */

#ifndef MCAL_NVIC_NVIC_CFG_H_
#define MCAL_NVIC_NVIC_CFG_H_

#include "../../CONFIG/Std_Types.h"
#include "NVIC_Types.h"

/* ===================[Pre-Compile Options]=================== */
#define NVIC_DEV_ERROR_DETECT            (STD_ON)
#define NVIC_VERSION_INFO_API            (STD_ON)

/* Default priority grouping (PRIGROUP value) */
#define NVIC_DEFAULT_PRIORITY_GROUP      (NVIC_PRIGROUP_XYY)

/* Maximum supported priority value */
#define NVIC_MAX_PRIORITY_LEVEL          (7u)

/* ===================[External Configuration]=================== */
extern const NVIC_InterruptConfigType NVIC_InterruptConfigTable[];
extern const NVIC_ConfigType NVIC_Config;

#endif /* MCAL_NVIC_NVIC_CFG_H_ */
