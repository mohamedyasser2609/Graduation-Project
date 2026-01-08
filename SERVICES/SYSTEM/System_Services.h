/**
 * @file System_Services.h
 * @brief System Services Interface
 * @details Common system-level functions and includes
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#ifndef SERVICES_SYSTEM_SYSTEM_SERVICES_H_
#define SERVICES_SYSTEM_SYSTEM_SERVICES_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[API Declarations]=================== */

/**
 * @brief Initialize all system drivers and services
 * @details Must be called at startup before using any drivers
 */
void System_Init(void);

/**
 * @brief De-initialize all system drivers
 * @details Called for controlled shutdown
 */
void System_DeInit(void);

/**
 * @brief Get system initialization status
 * @return TRUE if fully initialized
 */
boolean System_IsInitialized(void);

#endif /* SERVICES_SYSTEM_SYSTEM_SERVICES_H_ */
