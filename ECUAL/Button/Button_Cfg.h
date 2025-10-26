/*
 * @file Button_Cfg.h
 * @brief Button Driver Configuration for TM4C123GH6PM
 * @details This file contains configuration parameters for the Button driver.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

#ifndef ECUAL_BUTTON_BUTTON_CFG_H_
#define ECUAL_BUTTON_BUTTON_CFG_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/std_types.h"

/* ===================[Configuration Switches]=================== */

/**
 * @brief Enable/Disable Development Error Detection
 * @details When enabled, the driver reports errors through DET
 */
#define BUTTON_DEV_ERROR_DETECT              STD_ON

/**
 * @brief Enable/Disable Version Information API
 * @details When enabled, provides Button_GetVersionInfo() function
 */
#define BUTTON_VERSION_INFO_API              STD_ON

/**
 * @brief Enable/Disable Button State Change Detection
 * @details When enabled, provides Button_HasStateChanged() function
 */
#define BUTTON_STATE_CHANGE_API              STD_ON

/**
 * @brief Default debounce time in milliseconds
 * @details Default debounce time used when not specified in configuration
 */
#define BUTTON_DEFAULT_DEBOUNCE_MS           (20u)

/* ===================[API Service IDs]=================== */

/**
 * @brief Service ID for Button_Init
 */
#define BUTTON_INIT_SID                      (0x01u)

/**
 * @brief Service ID for Button_ReadState
 */
#define BUTTON_READ_STATE_SID                (0x02u)

/**
 * @brief Service ID for Button_ReadStateDebounced
 */
#define BUTTON_READ_STATE_DEBOUNCED_SID      (0x03u)

/**
 * @brief Service ID for Button_HasStateChanged
 */
#define BUTTON_HAS_STATE_CHANGED_SID         (0x04u)

/**
 * @brief Service ID for Button_GetVersionInfo
 */
#define BUTTON_GET_VERSION_INFO_SID          (0x05u)

/* ===================[DET Error Codes]=================== */

/**
 * @brief Invalid parameter (NULL pointer)
 * @details A NULL pointer was passed to an API function
 */
#define BUTTON_E_PARAM_POINTER               (0x01u)

/**
 * @brief Invalid button configuration
 * @details Button configuration contains invalid parameters
 */
#define BUTTON_E_INVALID_CONFIG              (0x02u)

#endif /* ECUAL_BUTTON_BUTTON_CFG_H_ */
