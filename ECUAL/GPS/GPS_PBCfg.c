/**
 * @file GPS_PBCfg.c
 * @brief GPS Post-Build Configuration for NEO-M8M
 * @details Pre-configured GPS setups for common use cases
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#include "GPS.h"

/* ===================[Pre-configured GPS Setups]=================== */

/**
 * @brief Default GPS configuration
 * @details Standard setup with SBAS and GLONASS enabled
 */
const GPS_ConfigType GPS_Config_Default = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_PORTABLE,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .UpdateRate = 1
};

/**
 * @brief Automotive configuration
 * @details Optimized for car/vehicle applications
 */
const GPS_ConfigType GPS_Config_Automotive = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_AUTOMOTIVE,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .UpdateRate = 1
};

/**
 * @brief Drone/Quadcopter configuration
 * @details Optimized for aerial vehicles
 */
const GPS_ConfigType GPS_Config_Drone = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_AIRBORNE_1G,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .UpdateRate = 5  /* 5Hz for faster updates */
};

/**
 * @brief Marine/Boat configuration
 * @details Optimized for marine applications
 */
const GPS_ConfigType GPS_Config_Marine = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_SEA,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .UpdateRate = 1
};

/**
 * @brief Pedestrian configuration
 * @details Optimized for walking/hiking
 */
const GPS_ConfigType GPS_Config_Pedestrian = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_PEDESTRIAN,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .UpdateRate = 1
};

/**
 * @brief Stationary configuration
 * @details Optimized for fixed position applications
 */
const GPS_ConfigType GPS_Config_Stationary = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_STATIONARY,
    .EnableSBAS = TRUE,
    .EnableGLONASS = FALSE,  /* GPS only for stationary */
    .UpdateRate = 1
};

/**
 * @brief High speed configuration
 * @details For fast-moving applications (racing, aircraft)
 */
const GPS_ConfigType GPS_Config_HighSpeed = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_AIRBORNE_2G,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .UpdateRate = 10  /* 10Hz for maximum update rate */
};

/**
 * @brief Low power configuration
 * @details GPS only, 1Hz update for battery-powered applications
 */
const GPS_ConfigType GPS_Config_LowPower = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_PORTABLE,
    .EnableSBAS = FALSE,
    .EnableGLONASS = FALSE,
    .UpdateRate = 1
};

/**
 * @brief UART2 configuration (alternate UART)
 */
const GPS_ConfigType GPS_Config_UART2 = {
    .UartModule = UART_MODULE_2,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_PORTABLE,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .UpdateRate = 1
};

/**
 * @brief Default configuration pointer
 */
const GPS_ConfigType* GPS_ConfigPtr = &GPS_Config_Default;
