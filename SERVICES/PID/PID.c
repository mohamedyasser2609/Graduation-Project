/**
 * @file PID.c
 * @brief PID Controller Implementation
 * @details Generic PID with anti-windup and derivative filtering.
 */

#include "PID.h"
#include <math.h>

/* ===================[Private Helpers]=================== */
static float32 PID_Clamp(float32 value, float32 min, float32 max)
{
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

/* ===================[Public API]=================== */

void PID_Init(const PID_ConfigType* ConfigPtr, PID_StateType* StatePtr)
{
#if (PID_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR || StatePtr == NULL_PTR)
    {
        Det_ReportError(PID_MODULE_ID, PID_INSTANCE_ID, PID_INIT_SID, PID_E_PARAM_POINTER);
        return;
    }
#endif

    StatePtr->Integrator = 0.0f;
    StatePtr->PrevError = 0.0f;
    StatePtr->PrevMeasurement = 0.0f;
    StatePtr->PrevDerivative = 0.0f;
    StatePtr->PrevOutput = 0.0f;
    StatePtr->Initialized = TRUE;
}

void PID_DeInit(PID_StateType* StatePtr)
{
    if (StatePtr == NULL_PTR)
    {
        return;
    }
    StatePtr->Initialized = FALSE;
}

void PID_Reset(PID_StateType* StatePtr)
{
    if (StatePtr == NULL_PTR)
    {
        return;
    }
    StatePtr->Integrator = 0.0f;
    StatePtr->PrevError = 0.0f;
    StatePtr->PrevMeasurement = 0.0f;
    StatePtr->PrevDerivative = 0.0f;
    StatePtr->PrevOutput = 0.0f;
}

Std_ReturnType PID_Compute(const PID_ConfigType* ConfigPtr,
                           PID_StateType* StatePtr,
                           float32 Setpoint,
                           float32 Measurement,
                           float32* OutputPtr)
{
#if (PID_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR || StatePtr == NULL_PTR || OutputPtr == NULL_PTR)
    {
        Det_ReportError(PID_MODULE_ID, PID_INSTANCE_ID, PID_COMPUTE_SID, PID_E_PARAM_POINTER);
        return E_NOT_OK;
    }
    if (StatePtr->Initialized == FALSE)
    {
        Det_ReportError(PID_MODULE_ID, PID_INSTANCE_ID, PID_COMPUTE_SID, PID_E_UNINIT);
        return E_NOT_OK;
    }
#endif

    const float32 dt = ConfigPtr->SampleTimeSec;
    if (dt <= 0.0f)
    {
        return E_NOT_OK;
    }

    /* Error */
    const float32 error = Setpoint - Measurement;

    /* Integral term with clamping and optional freeze if Ki is zero */
    if (ConfigPtr->Ki != 0.0f)
    {
        StatePtr->Integrator += ConfigPtr->Ki * error * dt;
        StatePtr->Integrator = PID_Clamp(StatePtr->Integrator,
                                         ConfigPtr->IntegratorMin,
                                         ConfigPtr->IntegratorMax);
    }

    /* Derivative term */
    float32 derivative;
    if (ConfigPtr->DerivativeOnMeasurement == TRUE)
    {
        derivative = -(Measurement - StatePtr->PrevMeasurement) / dt;
    }
    else
    {
        derivative = (error - StatePtr->PrevError) / dt;
    }

    /* Derivative filtering: y = alpha * d + (1 - alpha) * prev */
    const float32 alpha = PID_Clamp(ConfigPtr->DerivativeFilterAlpha, 0.0f, 1.0f);
    derivative = (alpha * derivative) + ((1.0f - alpha) * StatePtr->PrevDerivative);

    /* PID Output */
    float32 output = (ConfigPtr->Kp * error) +
                     StatePtr->Integrator +
                     (ConfigPtr->Kd * derivative);

    /* Output clamp (anti-windup via clamped output) */
    output = PID_Clamp(output, ConfigPtr->OutMin, ConfigPtr->OutMax);

    /* Save state */
    StatePtr->PrevError = error;
    StatePtr->PrevMeasurement = Measurement;
    StatePtr->PrevDerivative = derivative;
    StatePtr->PrevOutput = output;

    *OutputPtr = output;
    return E_OK;
}

Std_ReturnType PID_SetTunings(PID_ConfigType* ConfigPtr, float32 Kp, float32 Ki, float32 Kd)
{
#if (PID_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(PID_MODULE_ID, PID_INSTANCE_ID, PID_SET_TUNINGS_SID, PID_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif

    ConfigPtr->Kp = Kp;
    ConfigPtr->Ki = Ki;
    ConfigPtr->Kd = Kd;
    return E_OK;
}

Std_ReturnType PID_SetOutputLimits(PID_ConfigType* ConfigPtr, float32 OutMin, float32 OutMax)
{
#if (PID_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(PID_MODULE_ID, PID_INSTANCE_ID, PID_SET_OUTPUT_LIMITS_SID, PID_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif

    if (OutMax < OutMin)
    {
        return E_NOT_OK;
    }

    ConfigPtr->OutMin = OutMin;
    ConfigPtr->OutMax = OutMax;
    return E_OK;
}

Std_ReturnType PID_SetSampleTime(PID_ConfigType* ConfigPtr, float32 SampleTimeSec)
{
#if (PID_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(PID_MODULE_ID, PID_INSTANCE_ID, PID_SET_SAMPLE_TIME_SID, PID_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif

    if (SampleTimeSec <= 0.0f)
    {
        return E_NOT_OK;
    }

    ConfigPtr->SampleTimeSec = SampleTimeSec;
    return E_OK;
}

#if (PID_VERSION_INFO_API == STD_ON)
void PID_GetVersionInfo(Std_VersionInfoType* versionInfoPtr)
{
#if (PID_DEV_ERROR_DETECT == STD_ON)
    if (versionInfoPtr == NULL_PTR)
    {
        Det_ReportError(PID_MODULE_ID, PID_INSTANCE_ID, PID_GET_VERSION_INFO_SID, PID_E_PARAM_POINTER);
        return;
    }
#endif

    versionInfoPtr->vendorID = PID_VENDOR_ID;
    versionInfoPtr->moduleID = PID_MODULE_ID;
    versionInfoPtr->sw_major_version = PID_SW_MAJOR_VERSION;
    versionInfoPtr->sw_minor_version = PID_SW_MINOR_VERSION;
    versionInfoPtr->sw_patch_version = PID_SW_PATCH_VERSION;
}
#endif

