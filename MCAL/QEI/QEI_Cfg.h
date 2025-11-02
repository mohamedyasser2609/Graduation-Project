/**
 * @file QEI_Cfg.h
 * @brief Quadrature Encoder Interface (QEI) Driver Configuration
 */

#ifndef MCAL_QEI_QEI_CFG_H_
#define MCAL_QEI_QEI_CFG_H_

#include "../../CONFIG/Std_Types.h"
#include "QEI_Types.h"

/* ===================[Pre-Compile Options]=================== */
#define QEI_DEV_ERROR_DETECT            (STD_ON)
#define QEI_VERSION_INFO_API            (STD_ON)

/* Debug stall enable */
#define QEI_ALLOW_DEBUG_STALL           (STD_ON)

/* ===================[External Configuration]=================== */
extern const Qei_ConfigType Qei_Config;

#endif /* MCAL_QEI_QEI_CFG_H_ */
