/**
 * @file PID.h
 * @brief PID Controller API
 * @details Generic PID controller with anti-windup and derivative filtering.
 */

#ifndef SERVICES_PID_PID_H_
#define SERVICES_PID_PID_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"
#include "../../CONFIG/Compiler.h"
#include "../../CONFIG/Det.h"
#include "PID_Types.h"
#include "PID_Cfg.h"

/* ===================[Version Information]=================== */
#define PID_VENDOR_ID                    (0x1234u)
#define PID_MODULE_ID                    (210u)
#define PID_INSTANCE_ID                  (0u)

#define PID_SW_MAJOR_VERSION             (1u)
#define PID_SW_MINOR_VERSION             (0u)
#define PID_SW_PATCH_VERSION             (0u)

#define PID_AR_RELEASE_MAJOR_VERSION     (4u)
#define PID_AR_RELEASE_MINOR_VERSION     (4u)
#define PID_AR_RELEASE_REVISION_VERSION  (0u)

/* ===================[Service IDs]=================== */
#define PID_INIT_SID                     (0x00u)
#define PID_DEINIT_SID                   (0x01u)
#define PID_COMPUTE_SID                  (0x02u)
#define PID_RESET_SID                    (0x03u)
#define PID_SET_TUNINGS_SID              (0x04u)
#define PID_SET_OUTPUT_LIMITS_SID        (0x05u)
#define PID_SET_SAMPLE_TIME_SID          (0x06u)
#define PID_GET_VERSION_INFO_SID         (0x07u)

/* ===================[Error Codes]=================== */
#define PID_E_PARAM_POINTER              (0x01u)
#define PID_E_UNINIT                     (0x02u)
#define PID_E_PARAM_VALUE                (0x03u)

/* ===================[API Function Prototypes]=================== */

/**
 * @brief Initialize PID state
 * @param ConfigPtr Pointer to PID configuration
 * @param StatePtr  Pointer to PID state
 */
void PID_Init(const PID_ConfigType* ConfigPtr, PID_StateType* StatePtr);

/**
 * @brief De-initialize PID state (clears initialized flag)
 */
void PID_DeInit(PID_StateType* StatePtr);

/**
 * @brief Reset PID internal state (integrator, derivative, previous samples)
 */
void PID_Reset(PID_StateType* StatePtr);

/**
 * @brief Compute PID output
 * @param ConfigPtr   Pointer to PID configuration
 * @param StatePtr    Pointer to PID state
 * @param Setpoint    Desired value
 * @param Measurement Measured value
 * @param OutputPtr   Pointer to store computed output
 * @return Std_ReturnType (E_OK / E_NOT_OK)
 */
Std_ReturnType PID_Compute(const PID_ConfigType* ConfigPtr,
                           PID_StateType* StatePtr,
                           float32 Setpoint,
                           float32 Measurement,
                           float32* OutputPtr);

/**
 * @brief Update PID tunings (Kp, Ki, Kd)
 */
Std_ReturnType PID_SetTunings(PID_ConfigType* ConfigPtr, float32 Kp, float32 Ki, float32 Kd);

/**
 * @brief Update PID output limits
 */
Std_ReturnType PID_SetOutputLimits(PID_ConfigType* ConfigPtr, float32 OutMin, float32 OutMax);

/**
 * @brief Update PID sample time (seconds)
 */
Std_ReturnType PID_SetSampleTime(PID_ConfigType* ConfigPtr, float32 SampleTimeSec);

#if (PID_VERSION_INFO_API == STD_ON)
void PID_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
#endif

#endif /* SERVICES_PID_PID_H_ */

