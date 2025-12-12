/*
 * @file Led_Cfg.h
 * @brief LED Driver Configuration for TM4C123GH6PM
 * @details This file contains configuration parameters for the LED driver.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

#ifndef ECUAL_LED_LED_CFG_H_
#define ECUAL_LED_LED_CFG_H_

/* ===================[Includes]=================== */
#include <Std_types.h>

/* ===================[Configuration Switches]=================== */

/**
 * @brief Enable/Disable Development Error Detection
 * @details When enabled, the driver reports errors through DET
 */
#define LED_DEV_ERROR_DETECT              STD_ON

/**
 * @brief Enable/Disable Version Information API
 * @details When enabled, provides Led_GetVersionInfo() function
 */
#define LED_VERSION_INFO_API              STD_ON

/* ===================[API Service IDs]=================== */

/**
 * @brief Service ID for Led_Init
 */
#define LED_INIT_SID                      (0x01u)

/**
 * @brief Service ID for Led_SetState
 */
#define LED_SET_STATE_SID                 (0x02u)

/**
 * @brief Service ID for Led_Toggle
 */
#define LED_TOGGLE_SID                    (0x03u)

/**
 * @brief Service ID for Led_GetState
 */
#define LED_GET_STATE_SID                 (0x04u)

/**
 * @brief Service ID for Led_GetVersionInfo
 */
#define LED_GET_VERSION_INFO_SID          (0x05u)

/* ===================[DET Error Codes]=================== */

/**
 * @brief Invalid parameter (NULL pointer)
 * @details A NULL pointer was passed to an API function
 */
#define LED_E_PARAM_POINTER               (0x01u)

#endif /* ECUAL_LED_LED_CFG_H_ */
