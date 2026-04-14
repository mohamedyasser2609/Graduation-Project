/**
 * @file App_SharedTypes.h
 * @brief Shared types for inter-task communication
 * @details Defines command and feedback structures passed via FreeRTOS queues
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#ifndef APP_SHAREDTYPES_H
#define APP_SHAREDTYPES_H

#include "../../CONFIG/Std_Types.h"

/* ===================[Command Structures (ROS -> MCU)]=================== */

/**
 * @brief Wheel speed command from ROS2
 * @details Sent from Comm Task to Control Task via queue
 *
 * Protocol: "W,<LeftRadS>,<RightRadS>\n"
 * Example:  "W,5.2,-5.2\n"
 */
typedef struct
{
    float32     LeftRadPerSec;      /**< Left wheel angular velocity (rad/s) */
    float32     RightRadPerSec;     /**< Right wheel angular velocity (rad/s) */
    uint32      Timestamp;          /**< Command timestamp (FreeRTOS ticks) */
    boolean     Valid;              /**< TRUE if command is valid */
} WheelSpeedCmdType;

/**
 * @brief Emergency command type
 */
typedef enum
{
    ECMD_NONE       = 0u,
    ECMD_STOP       = 1u,
    ECMD_RESUME     = 2u,
    ECMD_CALIBRATE  = 3u
} EmergencyCmdType;

/* ===================[Feedback Structures (MCU -> ROS)]=================== */

/**
 * @brief Sensor feedback packet to ROS2
 * @details Sent from Sensor Task via Comm Task to ROS2
 *
 * Protocol: "F,<LTicks>,<RTicks>,<YawDeg>,<Lat>,<Lon>\n"
 * Example:  "F,12345,-12340,45.5,30.0444,31.2357\n"
 */
typedef struct
{
    sint32      LeftEncoderTicks;   /**< Left encoder absolute position */
    sint32      RightEncoderTicks;  /**< Right encoder absolute position */
    float32     YawDegrees;         /**< IMU yaw angle (degrees) */
    float32     Latitude;           /**< GPS latitude */
    float32     Longitude;          /**< GPS longitude */
    uint32      Timestamp;          /**< Sample timestamp */
    boolean     Valid;              /**< TRUE if data is valid */
} SensorFeedbackType;

/**
 * @brief IMU feedback (compact)
 */
typedef struct
{
    float32     Yaw;                /**< Yaw angle (degrees) */
    float32     Pitch;              /**< Pitch angle (degrees) */
    float32     Roll;               /**< Roll angle (degrees) */
    float32     GyroZ;              /**< Angular velocity Z (rad/s) */
} ImuFeedbackType;

/* ===================[Queue Size Definitions]=================== */
#define QUEUE_CMD_SIZE      (4u)    /**< Command queue depth */
#define QUEUE_FEEDBACK_SIZE (4u)    /**< Feedback queue depth */

/* ===================[Protocol Constants]=================== */
#define PROTO_CMD_WHEEL     'W'     /**< Wheel speed command prefix */
#define PROTO_CMD_STOP      'S'     /**< Emergency stop command */
#define PROTO_CMD_RESUME    'R'     /**< Resume command */
#define PROTO_FEEDBACK      'F'     /**< Feedback packet prefix */
#define PROTO_DELIMITER     ','     /**< Field delimiter */
#define PROTO_TERMINATOR    '\n'    /**< Packet terminator */

/* ===================[Conversion Constants]=================== */
#define WHEEL_RADIUS_M          (0.065f)    /**< Wheel radius in meters (65mm) */
#define RAD_TO_RPM              (9.5493f)   /**< rad/s to RPM = 60/(2*PI) */

#endif /* APP_SHAREDTYPES_H */
