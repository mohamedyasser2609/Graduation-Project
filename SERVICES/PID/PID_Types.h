/**
 * @file PID_Types.h
 * @brief PID Controller Type Definitions
 * @details Type definitions for a generic PID controller with anti-windup and
 *          derivative filtering. Intended for motor speed/position control.
 *
 * @author
 * @date Dec 2025
 * @version 1.0.0
 */

#ifndef SERVICES_PID_PID_TYPES_H_
#define SERVICES_PID_PID_TYPES_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[PID Configuration]=================== */

/**
 * @brief PID controller configuration structure
 */
typedef struct
{
    float32 Kp;                    /**< Proportional gain */
    float32 Ki;                    /**< Integral gain (per second) */
    float32 Kd;                    /**< Derivative gain (per second) */

    float32 SampleTimeSec;         /**< Sample time in seconds (dt) */

    float32 OutMin;                /**< Output minimum clamp */
    float32 OutMax;                /**< Output maximum clamp */

    float32 IntegratorMin;         /**< Integrator minimum clamp */
    float32 IntegratorMax;         /**< Integrator maximum clamp */

    boolean DerivativeOnMeasurement;  /**< TRUE: use -d(meas)/dt, FALSE: d(error)/dt */
    float32 DerivativeFilterAlpha;    /**< Derivative low-pass filter alpha (0-1, higher = less filtering) */
} PID_ConfigType;

/**
 * @brief PID controller runtime state
 */
typedef struct
{
    float32 Integrator;         /**< Accumulated integral term */
    float32 PrevError;          /**< Previous error sample */
    float32 PrevMeasurement;    /**< Previous measurement sample */
    float32 PrevDerivative;     /**< Filtered derivative previous value */
    float32 PrevOutput;         /**< Last output */
    boolean Initialized;        /**< State initialized flag */
} PID_StateType;

#endif /* SERVICES_PID_PID_TYPES_H_ */

