/**
 * @file GPS_PBCfg.c
 * @brief GPS Post-Build Configuration for NEO-M8N
 * @details Production-ready pre-configured GPS setups for common use cases
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 2.0.0
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
    .PowerMode = GPS_POWER_MODE_FULL,
    .SbasSystem = GPS_SBAS_AUTO,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .EnableGalileo = FALSE,
    .EnableBeiDou = FALSE,
    .EnablePPS = FALSE,
    .EnableGeofencing = FALSE,
    .EnableRTCM = FALSE,
    .UpdateRate = 1,
    .AssistNowMode = GPS_ASSISTNOW_AUTONOMOUS
};

/**
 * @brief Automotive configuration
 * @details Optimized for car/vehicle applications with multi-GNSS
 * @note Module DOES accept UBX commands - old test code proved this!
 */
const GPS_ConfigType GPS_Config_Automotive = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_AUTOMOTIVE,
    .PowerMode = GPS_POWER_MODE_FULL,
    .SbasSystem = GPS_SBAS_AUTO,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,   /* ✅ RE-ENABLED - old code got 15 satellites! */
    .EnableGalileo = TRUE,   /* ✅ RE-ENABLED - NEO-M8N supports this */
    .EnableBeiDou = FALSE,   /* Keep disabled - max 3 concurrent */
    .EnablePPS = FALSE,
    .EnableGeofencing = FALSE,
    .EnableRTCM = FALSE,
    .UpdateRate = 1,
    .AssistNowMode = GPS_ASSISTNOW_AUTONOMOUS
};

/**
 * @brief Drone/Quadcopter configuration
 * @details Optimized for aerial vehicles
 */
const GPS_ConfigType GPS_Config_Drone = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_AIRBORNE_1G,
    .PowerMode = GPS_POWER_MODE_FULL,
    .SbasSystem = GPS_SBAS_AUTO,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .EnableGalileo = TRUE,
    .EnableBeiDou = FALSE,
    .EnablePPS = TRUE,  /* PPS for flight controller sync */
    .EnableGeofencing = TRUE,  /* No-fly zones */
    .EnableRTCM = FALSE,
    .UpdateRate = 5,  /* 5Hz for faster updates */
    .AssistNowMode = GPS_ASSISTNOW_AUTONOMOUS
};

/**
 * @brief Marine/Boat configuration
 * @details Optimized for marine applications
 */
const GPS_ConfigType GPS_Config_Marine = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_SEA,
    .PowerMode = GPS_POWER_MODE_FULL,
    .SbasSystem = GPS_SBAS_AUTO,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .EnableGalileo = TRUE,
    .EnableBeiDou = FALSE,
    .EnablePPS = FALSE,
    .EnableGeofencing = FALSE,
    .EnableRTCM = TRUE,  /* D-GPS for navigation */
    .UpdateRate = 1,
    .AssistNowMode = GPS_ASSISTNOW_AUTONOMOUS
};

/**
 * @brief Pedestrian configuration
 * @details Optimized for walking/hiking
 */
const GPS_ConfigType GPS_Config_Pedestrian = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_PEDESTRIAN,
    .PowerMode = GPS_POWER_MODE_BALANCED,
    .SbasSystem = GPS_SBAS_AUTO,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .EnableGalileo = FALSE,
    .EnableBeiDou = FALSE,
    .EnablePPS = FALSE,
    .EnableGeofencing = FALSE,
    .EnableRTCM = FALSE,
    .UpdateRate = 1,
    .AssistNowMode = GPS_ASSISTNOW_AUTONOMOUS
};

/**
 * @brief Stationary configuration
 * @details Optimized for fixed position applications
 */
const GPS_ConfigType GPS_Config_Stationary = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_STATIONARY,
    .PowerMode = GPS_POWER_MODE_FULL,
    .SbasSystem = GPS_SBAS_AUTO,
    .EnableSBAS = TRUE,
    .EnableGLONASS = FALSE,  /* GPS only for stationary */
    .EnableGalileo = FALSE,
    .EnableBeiDou = FALSE,
    .EnablePPS = TRUE,  /* Timing applications */
    .EnableGeofencing = FALSE,
    .EnableRTCM = TRUE,  /* High accuracy */
    .UpdateRate = 1,
    .AssistNowMode = GPS_ASSISTNOW_AUTONOMOUS
};

/**
 * @brief High speed configuration
 * @details For fast-moving applications (racing, aircraft)
 */
const GPS_ConfigType GPS_Config_HighSpeed = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_AIRBORNE_2G,
    .PowerMode = GPS_POWER_MODE_FULL,
    .SbasSystem = GPS_SBAS_AUTO,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .EnableGalileo = TRUE,
    .EnableBeiDou = FALSE,
    .EnablePPS = FALSE,
    .EnableGeofencing = FALSE,
    .EnableRTCM = FALSE,
    .UpdateRate = 10,  /* 10Hz for maximum update rate */
    .AssistNowMode = GPS_ASSISTNOW_AUTONOMOUS
};

/**
 * @brief Low power configuration
 * @details GPS only, 1Hz update for battery-powered applications
 */
const GPS_ConfigType GPS_Config_LowPower = {
    .UartModule = UART_MODULE_1,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_PORTABLE,
    .PowerMode = GPS_POWER_MODE_INTERVAL,  /* Cyclic tracking */
    .SbasSystem = GPS_SBAS_AUTO,
    .EnableSBAS = FALSE,
    .EnableGLONASS = FALSE,
    .EnableGalileo = FALSE,
    .EnableBeiDou = FALSE,
    .EnablePPS = FALSE,
    .EnableGeofencing = FALSE,
    .EnableRTCM = FALSE,
    .UpdateRate = 1,
    .AssistNowMode = GPS_ASSISTNOW_OFFLINE  /* Pre-downloaded */
};

/**
 * @brief UART2 configuration (alternate UART)
 */
const GPS_ConfigType GPS_Config_UART2 = {
    .UartModule = UART_MODULE_2,
    .BaudRate = GPS_DEFAULT_BAUD_RATE,
    .DynamicModel = GPS_DYNAMIC_MODEL_PORTABLE,
    .PowerMode = GPS_POWER_MODE_FULL,
    .SbasSystem = GPS_SBAS_AUTO,
    .EnableSBAS = TRUE,
    .EnableGLONASS = TRUE,
    .EnableGalileo = FALSE,
    .EnableBeiDou = FALSE,
    .EnablePPS = FALSE,
    .EnableGeofencing = FALSE,
    .EnableRTCM = FALSE,
    .UpdateRate = 1,
    .AssistNowMode = GPS_ASSISTNOW_AUTONOMOUS
};

/**
 * @brief Default configuration pointer
 */
const GPS_ConfigType* GPS_ConfigPtr = &GPS_Config_Default;
