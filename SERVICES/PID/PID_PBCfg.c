/**
 * @file PID_PBCfg.c
 * @brief PID Controller Post-Build Configuration
 * @details Example configuration for motor speed control (EMG49)
 */

#include "PID.h"

/* ===================[PID Configuration]=================== */
/**
 * @brief Example PID configuration for motor speed (EMG49)
 * @details Tunings are placeholders; tune as needed for your plant.
 */
const PID_ConfigType PID_MotorSpeed_Config = {
    .Kp = 0.8f,
    .Ki = 2.0f,
    .Kd = 0.01f,

    .SampleTimeSec = 0.01f,          /* 10 ms loop time */

    .OutMin = -100.0f,               /* Output limits (e.g., PWM duty %) */
    .OutMax =  100.0f,

    .IntegratorMin = -50.0f,         /* Anti-windup clamp */
    .IntegratorMax =  50.0f,

    .DerivativeOnMeasurement = TRUE, /* Reduces derivative kick */
    .DerivativeFilterAlpha = 0.1f    /* Derivative filter (0-1, higher = less filtering) */
};

