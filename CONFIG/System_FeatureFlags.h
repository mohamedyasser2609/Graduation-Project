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
 * @brief Fan Control (PWM Fans)
 * @details Set to 0 to exclude fan PWM control (e.g., during bench testing)
 */
#define FEATURE_FAN_ENABLED         (1u)

/**
 * @brief Temperature Sensors (AM2320)
 * @details Set to 0 to exclude temperature monitoring
 */
#define FEATURE_TEMP_ENABLED        (1u)

/**
 * @brief Current Sensors (ACS712)
 * @details Set to 0 to exclude current monitoring
 */
#define FEATURE_CURRENT_ENABLED     (1u)

#endif /* SYSTEM_FEATUREFLAGS_H */
