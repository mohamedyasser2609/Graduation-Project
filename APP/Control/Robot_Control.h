/**
 * @file Robot_Control.h
 * @brief Robot Controller Application Interface
 * @details High-level robot control interface for ROS2 integration
 *
 * This module provides:
 * - Differential drive kinematics
 * - Velocity command processing
 * - Odometry calculation
 * - System state management
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#ifndef APP_ROBOT_CONTROL_H_
#define APP_ROBOT_CONTROL_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[Macros]=================== */
/** @brief Robot physical parameters */
#define ROBOT_WHEEL_RADIUS_M        (0.065f)    /**< Wheel radius in meters (65mm) */
#define ROBOT_WHEEL_BASE_M          (0.50f)     /**< Distance between wheels in meters */
#define ROBOT_ENCODER_CPR           (980u)      /**< Encoder counts per revolution (245 PPR × 4 quadrature) */

/** @brief Control limits */
#define ROBOT_MAX_LINEAR_VEL        (0.68f)     /**< Max linear velocity (m/s) matches 100 RPM */
#define ROBOT_MAX_ANGULAR_VEL       (3.14f)     /**< Max angular velocity (rad/s) */
#define ROBOT_MAX_WHEEL_RPM         (100.0f)    /**< Max wheel RPM */

/* ===================[Type Definitions]=================== */

/**
 * @brief Robot operating state
 */
typedef enum
{
    ROBOT_STATE_INIT        = 0u,   /**< Initializing */
    ROBOT_STATE_IDLE        = 1u,   /**< Ready, motors off */
    ROBOT_STATE_RUNNING     = 2u,   /**< Operating normally */
    ROBOT_STATE_ESTOP       = 3u,   /**< Emergency stopped */
    ROBOT_STATE_FAULT       = 4u    /**< Fault condition */
} Robot_StateType;

/**
 * @brief Twist command (ROS-style velocity)
 */
typedef struct
{
    float32     LinearX;        /**< Forward velocity (m/s) */
    float32     LinearY;        /**< Lateral velocity (m/s) - not used for diff drive */
    float32     AngularZ;       /**< Rotational velocity (rad/s) */
} Robot_TwistType;

/**
 * @brief Odometry data
 */
typedef struct
{
    float32     X;              /**< X position (meters) */
    float32     Y;              /**< Y position (meters) */
    float32     Theta;          /**< Heading angle (radians) */
    float32     LinearVel;      /**< Current linear velocity (m/s) */
    float32     AngularVel;     /**< Current angular velocity (rad/s) */
} Robot_OdometryType;

/**
 * @brief Wheel velocities
 */
typedef struct
{
    float32     LeftRPM;        /**< Left wheel RPM */
    float32     RightRPM;       /**< Right wheel RPM */
    float32     LeftMps;        /**< Left wheel velocity (m/s) */
    float32     RightMps;       /**< Right wheel velocity (m/s) */
} Robot_WheelVelType;

/**
 * @brief Robot status summary
 */
typedef struct
{
    Robot_StateType     State;
    float32             BatteryVoltage;
    float32             LeftCurrent;
    float32             RightCurrent;
    float32             MaxTemperature;
    uint8               ErrorFlags;
} Robot_StatusType;

/* ===================[API Declarations]=================== */

/**
 * @brief Initialize robot controller
 */
void Robot_Init(void);

/**
 * @brief Process velocity command from ROS2
 * @param[in] Cmd Twist velocity command
 * @return E_OK if command accepted
 */
Std_ReturnType Robot_SetVelocity(const Robot_TwistType* Cmd);

/**
 * @brief Emergency stop
 */
void Robot_EmergencyStop(void);

/**
 * @brief Resume from emergency stop
 * @return E_OK if resumed, E_NOT_OK if faults present
 */
Std_ReturnType Robot_Resume(void);

/**
 * @brief Get current odometry
 * @param[out] OdomPtr Pointer to store odometry data
 * @return E_OK on success
 */
Std_ReturnType Robot_GetOdometry(Robot_OdometryType* OdomPtr);

/**
 * @brief Reset odometry to zero
 */
void Robot_ResetOdometry(void);

/**
 * @brief Get wheel velocities
 * @param[out] WheelPtr Pointer to store wheel velocities
 * @return E_OK on success
 */
Std_ReturnType Robot_GetWheelVelocities(Robot_WheelVelType* WheelPtr);

/**
 * @brief Get robot status
 * @param[out] StatusPtr Pointer to store status
 * @return E_OK on success
 */
Std_ReturnType Robot_GetStatus(Robot_StatusType* StatusPtr);

/**
 * @brief Get current state
 * @return Robot state
 */
Robot_StateType Robot_GetState(void);

/**
 * @brief Update control loop (call periodically)
 * @details Should be called by Control Task at 100Hz
 */
void Robot_UpdateControl(void);

/**
 * @brief Update odometry (call periodically)
 * @details Should be called by Sensor Task at 50Hz
 */
void Robot_UpdateOdometry(void);

#endif /* APP_ROBOT_CONTROL_H_ */
