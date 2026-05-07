/**
 * @file ComStack_Types.h
 * @brief Type definitions for Communication Stack Service
 * @details Types for UART-based ROS2 communication protocol
 *
 * Protocol Format:
 * | START (0xAA) | CMD (1B) | LEN (1B) | DATA (0-120B) | CHECKSUM (1B) | END (0x55) |
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#ifndef COMSTACK_TYPES_H
#define COMSTACK_TYPES_H

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"
#include "../../CONFIG/System_FeatureFlags.h"
#include "../../MCAL/UART/Uart_Types.h"

/* ===================[Macros]=================== */
/** @brief Module identification */
#define COMSTACK_MODULE_ID              (0xA0u)
#define COMSTACK_VENDOR_ID              (0x00u)
#define COMSTACK_INSTANCE_ID            (0x00u)

/** @brief Software version */
#define COMSTACK_SW_MAJOR_VERSION       (1u)
#define COMSTACK_SW_MINOR_VERSION       (0u)
#define COMSTACK_SW_PATCH_VERSION       (0u)

/** @brief Development error codes */
#define COMSTACK_E_PARAM_POINTER        (0x10u)
#define COMSTACK_E_UNINIT               (0x12u)
#define COMSTACK_E_ALREADY_INIT         (0x13u)
#define COMSTACK_E_PARAM_CONFIG         (0x14u)
#define COMSTACK_E_TX_BUSY              (0x15u)
#define COMSTACK_E_RX_OVERFLOW          (0x16u)

/** @brief API service IDs */
#define COMSTACK_INIT_SID               (0x00u)
#define COMSTACK_DEINIT_SID             (0x01u)
#define COMSTACK_SEND_SID               (0x02u)
#define COMSTACK_RECEIVE_SID            (0x03u)
#define COMSTACK_MAINFUNCTION_SID       (0x04u)
#define COMSTACK_GET_VERSION_SID        (0x05u)

/** @brief Protocol constants */
#define COMSTACK_START_BYTE             (0xAAu)
#define COMSTACK_END_BYTE               (0x55u)
#define COMSTACK_MAX_DATA_LENGTH        (120u)
#define COMSTACK_HEADER_SIZE            (3u)    /* START + CMD + LEN */
#define COMSTACK_FOOTER_SIZE            (2u)    /* CHECKSUM + END */
#define COMSTACK_MAX_PACKET_SIZE        (COMSTACK_HEADER_SIZE + COMSTACK_MAX_DATA_LENGTH + COMSTACK_FOOTER_SIZE)

/** @brief Command IDs */
#define COMSTACK_CMD_PING               (0x01u)
#define COMSTACK_CMD_ACK                (0x02u)
#define COMSTACK_CMD_NACK               (0x03u)
#define COMSTACK_CMD_MOTOR_CMD          (0x10u)
#define COMSTACK_CMD_MOTOR_STOP         (0x11u)
#define COMSTACK_CMD_TWIST_CMD          (0x12u)  /**< Twist velocity (v, ω) */
#define COMSTACK_CMD_SENSOR_DATA        (0x20u)
#if (FEATURE_GPS_ENABLED == 1u)
#define COMSTACK_CMD_GPS_DATA           (0x21u)  /* GPS wired to RPi — excluded */
#endif
#define COMSTACK_CMD_IMU_DATA           (0x22u)
#define COMSTACK_CMD_ENCODER_DATA       (0x23u)
#define COMSTACK_CMD_CURRENT_DATA       (0x24u)
#define COMSTACK_CMD_TEMP_DATA          (0x25u)
#define COMSTACK_CMD_STATUS             (0x30u)
#define COMSTACK_CMD_CONFIG             (0x31u)
#define COMSTACK_CMD_CALIBRATE          (0x32u)
#define COMSTACK_CMD_TIME_SYNC          (0x05u)
#define COMSTACK_CMD_ERROR              (0xFFu)

/* ===================[Type Definitions]=================== */

/**
 * @brief Time synchronization structure (CMD 0x05)
 * @details Received from ROS2 to sync wall time
 */
typedef struct __attribute__((packed))
{
    uint32      Seconds;            /**< Unix Epoch seconds */
    uint32      Nanoseconds;        /**< Nanoseconds part */
} ComStack_TimeSyncType;

/**
 * @brief Command ID type
 */
typedef uint8 ComStack_CommandType;

/**
 * @brief Driver status
 */
typedef enum
{
    COMSTACK_STATUS_UNINIT      = 0u,   /**< Not initialized */
    COMSTACK_STATUS_IDLE        = 1u,   /**< Idle, ready */
    COMSTACK_STATUS_RECEIVING   = 2u,   /**< Receiving packet */
    COMSTACK_STATUS_TRANSMITTING= 3u,   /**< Transmitting packet */
    COMSTACK_STATUS_ERROR       = 4u    /**< Error occurred */
} ComStack_StatusType;

/**
 * @brief Receive state machine states
 */
typedef enum
{
    COMSTACK_RX_WAIT_START      = 0u,   /**< Waiting for start byte */
    COMSTACK_RX_WAIT_CMD        = 1u,   /**< Waiting for command byte */
    COMSTACK_RX_WAIT_LEN        = 2u,   /**< Waiting for length byte */
    COMSTACK_RX_WAIT_DATA       = 3u,   /**< Receiving data bytes */
    COMSTACK_RX_WAIT_CHECKSUM   = 4u,   /**< Waiting for checksum */
    COMSTACK_RX_WAIT_END        = 5u    /**< Waiting for end byte */
} ComStack_RxStateType;

/**
 * @brief Packet reception status
 */
typedef enum
{
    COMSTACK_RX_OK              = 0u,   /**< Packet received successfully */
    COMSTACK_RX_PENDING         = 1u,   /**< Reception in progress */
    COMSTACK_RX_NO_DATA         = 2u,   /**< No data available */
    COMSTACK_RX_CHECKSUM_ERROR  = 3u,   /**< Checksum mismatch */
    COMSTACK_RX_FRAME_ERROR     = 4u,   /**< Invalid frame format */
    COMSTACK_RX_OVERFLOW        = 5u,   /**< Buffer overflow */
    COMSTACK_RX_TIMEOUT         = 6u    /**< Reception timeout */
} ComStack_RxResultType;

/**
 * @brief Packet transmission status
 */
typedef enum
{
    COMSTACK_TX_OK              = 0u,   /**< Sent successfully */
    COMSTACK_TX_PENDING         = 1u,   /**< Transmission in progress */
    COMSTACK_TX_BUSY            = 2u,   /**< Channel busy */
    COMSTACK_TX_ERROR           = 3u    /**< Transmission error */
} ComStack_TxResultType;

/**
 * @brief Packet structure
 */
typedef struct
{
    ComStack_CommandType    Command;                        /**< Command ID */
    uint8                   Length;                         /**< Data length */
    uint8                   Data[COMSTACK_MAX_DATA_LENGTH]; /**< Payload data */
    uint8                   Checksum;                       /**< Calculated checksum */
    boolean                 Valid;                          /**< Packet validity flag */
} ComStack_PacketType;

/**
 * @brief Motor command data structure
 */
typedef struct __attribute__((packed))
{
    sint16      LeftSpeed;          /**< Left wheel speed (-100 to +100) */
    sint16      RightSpeed;         /**< Right wheel speed (-100 to +100) */
} ComStack_MotorCmdType;

/**
 * @brief Twist velocity command (from ROS2 /cmd_vel)
 * @details Linear and angular velocities as fixed-point × 1000
 *          This is the PREFERRED command — differential drive math
 *          is applied on the TM4C side, not the bridge.
 *
 *  Wire format (4 bytes, little-endian):
 *    [0-1] sint16  linear velocity   (mm/s,  real_m_per_s × 1000)
 *    [2-3] sint16  angular velocity  (mrad/s, real_rad_per_s × 1000)
 *
 *  Example: v=0.5 m/s, ω=1.0 rad/s  →  0x01F4, 0x03E8  (500, 1000)
 */
typedef struct __attribute__((packed))
{
    sint16      LinearVelMmps;      /**< Linear velocity (mm/s = m/s × 1000) */
    sint16      AngularVelMrads;    /**< Angular velocity (mrad/s = rad/s × 1000) */
} ComStack_TwistCmdType;

#if (FEATURE_GPS_ENABLED == 1u)
/**
 * @brief GPS data structure (for transmission)
 * @note GPS is currently wired directly to RPi — this struct is excluded.
 */
typedef struct
{
    float32     Latitude;           /**< Latitude in degrees */
    float32     Longitude;          /**< Longitude in degrees */
    float32     Altitude;           /**< Altitude in meters */
    uint8       FixType;            /**< GPS fix type */
    uint8       NumSatellites;      /**< Number of satellites */
} ComStack_GpsDataType;
#endif

/**
 * @brief IMU data structure (for transmission)
 * @details Values are scaled to avoid floating point overhead
 *          Acceleration: m/s^2 * 100 
 *          Gyroscope: rad/s * 100
 */
typedef struct __attribute__((packed))
{
    uint32      TimestampSec;       /**< Synchronized ROS seconds */
    uint32      TimestampNsec;      /**< Synchronized ROS nanoseconds */
    sint16      AccelX;             /**< Acceleration X (scaled x100) */
    sint16      AccelY;             /**< Acceleration Y (scaled x100) */
    sint16      AccelZ;             /**< Acceleration Z (scaled x100) */
    sint16      GyroX;              /**< Angular rate X (scaled x100) */
    sint16      GyroY;              /**< Angular rate Y (scaled x100) */
    sint16      GyroZ;              /**< Angular rate Z (scaled x100) */
} ComStack_ImuDataType;

/**
 * @brief Encoder data structure (for transmission)
 */
typedef struct __attribute__((packed))
{
    uint32      TimestampSec;       /**< Synchronized ROS seconds */
    uint32      TimestampNsec;      /**< Synchronized ROS nanoseconds */
    sint32      LeftTicks;          /**< Left encoder ticks */
    sint32      RightTicks;         /**< Right encoder ticks */
    sint16      LeftVelocity;       /**< Left velocity (RPM scaled x100) */
    sint16      RightVelocity;      /**< Right velocity (RPM scaled x100) */
} ComStack_EncoderDataType;

/**
 * @brief System telemetry / status structure (CMD 0x30)
 * @details All values are fixed-point integers for wire efficiency.
 *
 *  Wire format (20 bytes, little-endian):
 *    [0]     uint8   SystemState        (Robot_StateType enum)
 *    [1]     uint8   ErrorFlags         (fault bitmap)
 *    [2-3]   sint16  BatteryVoltage     (mV = V × 1000)
 *    [4]     uint8   BatteryPercent     (0-100%)
 *    [5-6]   sint16  LeftMotorCurrent   (mA = A × 1000)
 *    [7-8]   sint16  RightMotorCurrent  (mA = A × 1000)
 *    [9-10]  sint16  TempMotors         (°C × 10)
 *    [11-12] sint16  TempMCU            (°C × 10)
 *    [13-14] sint16  TempBattery        (°C × 10)
 *    [15]    uint8   FanSpeed           (0-100%)
 *    [16-17] sint16  LinearVel          (mm/s = m/s × 1000)
 *    [18-19] sint16  AngularVel         (mrad/s = rad/s × 1000)
 */
typedef struct __attribute__((packed))
{
    uint8       SystemState;        /**< Overall system state */
    uint8       ErrorFlags;         /**< Error flags bitmap */
    sint16      BatteryVoltageMv;   /**< Battery voltage (mV) */
    uint8       BatteryPercent;     /**< Battery SOC (0-100%) */
    sint16      LeftCurrentMa;      /**< Left motor current (mA) */
    sint16      RightCurrentMa;     /**< Right motor current (mA) */
    sint16      TempMotors;         /**< Motor zone temp (°C × 10) */
    sint16      TempMCU;            /**< MCU zone temp (°C × 10) */
    sint16      TempBattery;        /**< Battery zone temp (°C × 10) */
    uint8       FanSpeedPercent;    /**< Fan speed (0-100%) */
    sint16      LinearVelMmps;      /**< Current linear vel (mm/s) */
    sint16      AngularVelMrads;    /**< Current angular vel (mrad/s) */
} ComStack_StatusDataType;

/**
 * @brief Configuration structure
 */
typedef struct
{
    Uart_ModuleType UartModule;     /**< UART module to use */
    uint16      RxTimeoutMs;        /**< Receive timeout in ms */
    uint16      TxRetries;          /**< Number of transmit retries */
    boolean     AutoAck;            /**< Auto-acknowledge received packets */
} ComStack_ConfigType;

/**
 * @brief Callback for received packets
 */
typedef void (*ComStack_RxCallbackType)(const ComStack_PacketType* Packet);

/**
 * @brief Callback for transmission complete
 */
typedef void (*ComStack_TxCallbackType)(ComStack_TxResultType Result);

#endif /* COMSTACK_TYPES_H */
