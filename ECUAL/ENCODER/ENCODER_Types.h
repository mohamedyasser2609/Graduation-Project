/**
 * @file ENCODER_Types.h
 * @brief Encoder Driver Type Definitions for EMG49 Motor Encoder
 * @details Type definitions for the AUTOSAR-compliant encoder driver
 *
 * @author Mohamed Yasser
 * @date Nov 5, 2025
 * @version 1.0.0
 */

#ifndef ECUAL_ENCODER_ENCODER_TYPES_H_
#define ECUAL_ENCODER_ENCODER_TYPES_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"
#include "../../MCAL/QEI/QEI_Types.h"

/* ===================[Encoder Specifications]=================== */
/**
 * @brief EMG49 Encoder Specifications
 * @details EMG49 is a 12 PPR (Pulses Per Revolution) encoder
 *          In quadrature mode: 12 × 4 = 48 counts per revolution
 */
#define ENCODER_PPR_EMG49                (12u)      /**< Pulses per revolution */
#define ENCODER_COUNTS_PER_REV_EMG49     (48u)      /**< Quadrature counts per revolution (12 × 4) */
#define ENCODER_DEGREES_PER_COUNT_EMG49  (7.5f)     /**< Degrees per count (360 / 48) */

/* ===================[Type Definitions]=================== */

/**
 * @brief Encoder direction type
 */
typedef enum {
    ENCODER_DIRECTION_FORWARD = 0u,  /**< Forward/Clockwise rotation */
    ENCODER_DIRECTION_REVERSE        /**< Reverse/Counter-clockwise rotation */
} Encoder_DirectionType;

/**
 * @brief Encoder status type
 */
typedef enum {
    ENCODER_STATUS_UNINIT = 0u,  /**< Encoder not initialized */
    ENCODER_STATUS_IDLE,        /**< Encoder idle */
    ENCODER_STATUS_RUNNING      /**< Encoder running */
} Encoder_StatusType;

/**
 * @brief Encoder position unit type
 */
typedef enum {
    ENCODER_UNIT_COUNTS = 0u,    /**< Position in quadrature counts */
    ENCODER_UNIT_REVOLUTIONS,    /**< Position in revolutions */
    ENCODER_UNIT_DEGREES         /**< Position in degrees */
} Encoder_UnitType;

/**
 * @brief Encoder velocity unit type
 */
typedef enum {
    ENCODER_VEL_UNIT_COUNTS_PER_SEC = 0u,  /**< Velocity in counts per second */
    ENCODER_VEL_UNIT_RPM                   /**< Velocity in revolutions per minute */
} Encoder_VelocityUnitType;

/**
 * @brief Encoder configuration structure
 */
typedef struct {
    uint32 PulsesPerRevolution;      /**< Encoder PPR (e.g., 12 for EMG49) */
    uint32 QuadratureCountsPerRev;   /**< Quadrature counts per revolution (PPR × 4) */
    boolean EnableVelocityFilter;     /**< Enable velocity filtering */
    uint8 VelocityFilterAlpha;        /**< Velocity filter coefficient (0-255, higher = less filtering) */
    uint32 VelocityTimerPeriodUs;    /**< Velocity timer period in microseconds */
} Encoder_ConfigType;

/**
 * @brief Encoder data structure
 */
typedef struct {
    uint32 PositionCounts;           /**< Position in quadrature counts */
    float PositionRevolutions;       /**< Position in revolutions */
    float PositionDegrees;           /**< Position in degrees */
    Encoder_DirectionType Direction; /**< Current rotation direction */
    uint32 VelocityCountsPerSec;     /**< Velocity in counts per second */
    float VelocityRPM;               /**< Velocity in RPM */
} Encoder_DataType;

#endif /* ECUAL_ENCODER_ENCODER_TYPES_H_ */

