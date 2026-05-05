/**
 * @file System_FeatureFlags.h
 * @brief Central Feature Flags for Conditional Compilation
 * @details Enable/disable hardware features without deleting code.
 *          Set to 1 to enable, 0 to exclude from build.
 *
 * @author Mohamed Yasser
 * @date Mar 10, 2026
 * @version 1.0.0
 */

#ifndef SYSTEM_FEATUREFLAGS_H
#define SYSTEM_FEATUREFLAGS_H

/* ===================[Hardware Feature Flags]=================== */

/**
 * @brief GPS Module (NEO-M8N)
 * @details Set to 0 when GPS is wired directly to RPi instead of TM4C.
 *          All GPS driver code is preserved but excluded from compilation.
 */
#define FEATURE_GPS_ENABLED         (0u)

/**
 * @brief IMU Module (MPU-9250)
 * @details Set to STD_ON to enable runtime IMU reads in Sensor Task.
 */
#define ROBOT_IMU_ENABLED           STD_ON

/**
 * @brief Fan Control (PWM Fans)
 * @details Set to 0 to exclude fan PWM control (e.g., during bench testing)
 * @note DISABLED — fans not connected for first test run
 */
#define FEATURE_FAN_ENABLED         (0u)

/**
 * @brief Temperature Sensors (AM2320)
 * @details Set to 0 to exclude temperature monitoring
 * @note DISABLED — temp sensors not connected for first test run
 */
#define FEATURE_TEMP_ENABLED        (0u)

/**
 * @brief Current Sensors (ACS712)
 * @details Set to 0 to exclude current monitoring
 * @note DISABLED — current sensors not connected for first test run
 */
#define FEATURE_CURRENT_ENABLED     (0u)

#endif /* SYSTEM_FEATUREFLAGS_H */
