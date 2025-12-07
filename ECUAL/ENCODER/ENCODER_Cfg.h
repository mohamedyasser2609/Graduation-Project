/**
 * @file ENCODER_Cfg.h
 * @brief Encoder Driver Configuration Header
 * @details Compile-time configuration for the encoder driver
 *
 * @author Mohamed Yasser
 * @date Nov 5, 2025
 * @version 1.0.0
 */

#ifndef ECUAL_ENCODER_ENCODER_CFG_H_
#define ECUAL_ENCODER_ENCODER_CFG_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"
#include "ENCODER_Types.h"

/* ===================[Pre-Compile Options]=================== */
#define ENCODER_DEV_ERROR_DETECT            (STD_ON)
#define ENCODER_VERSION_INFO_API            (STD_ON)

/* ===================[External Configuration]=================== */
extern const Encoder_ConfigType Encoder_Config;

#endif /* ECUAL_ENCODER_ENCODER_CFG_H_ */
