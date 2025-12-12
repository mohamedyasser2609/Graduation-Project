/**
 * @file ENCODER.c
 * @brief Encoder Driver Implementation for EMG49 Motor Encoder
 * @details High-level encoder driver wrapping the QEI driver
 *
 * @author Mohamed Yasser
 * @date Nov 5, 2025
 * @version 1.0.0
 */

#include "ENCODER.h"
#include "../../MCAL/QEI/QEI.h"
#include "../../MCAL/QEI/QEI_Cfg.h"
#include <stdint.h>

/* ===================[Private Types]=================== */
typedef struct
{
    const Encoder_ChannelConfigType* Cfg;
    Encoder_StatusType Status;
    int64_t PositionAccum;         /* 64-bit absolute position */
    uint32 LastHwPosition;         /* Last raw hardware position (wrap tracking) */
    int32_t FilteredVelocity;      /* Signed filtered velocity counts/sec */
    int32_t LastRawVelocity;       /* Last raw velocity counts/sec */
} Encoder_ChannelStateType;

/* ===================[Private Variables]=================== */
static const Encoder_ConfigType* Encoder_ConfigPtr = NULL_PTR;
static Encoder_ChannelStateType Encoder_ChannelStates[ENCODER_MAX_CHANNELS];

/* ===================[Private Helper Functions]=================== */
static boolean Encoder_IsChannelValid(Encoder_ChannelType channel)
{
    if (Encoder_ConfigPtr == NULL_PTR)
    {
        return FALSE;
    }
    return (channel < Encoder_ConfigPtr->ChannelCount) && (channel < ENCODER_MAX_CHANNELS);
}

static Encoder_ChannelStateType* Encoder_GetState(Encoder_ChannelType channel)
{
    if (!Encoder_IsChannelValid(channel))
    {
        return NULL_PTR;
    }
    return &Encoder_ChannelStates[channel];
}

static const Encoder_ChannelConfigType* Encoder_GetConfig(Encoder_ChannelType channel)
{
    if ((Encoder_ConfigPtr == NULL_PTR) || !Encoder_IsChannelValid(channel))
    {
        return NULL_PTR;
    }
    return &Encoder_ConfigPtr->Channels[channel];
}

static uint8 Encoder_GetEffectiveAlpha(const Encoder_FilterConfigType* filterCfg)
{
    uint8 alpha = filterCfg->Alpha;
    if (alpha == 0u)
    {
        alpha = filterCfg->DefaultAlpha;
    }
    if (alpha == 0u)
    {
        alpha = 1u;
    }
    if (alpha > 255u)
    {
        alpha = 255u;
    }
    return alpha;
}

static Encoder_DirectionType Encoder_MapDirection(Encoder_DirectionType hwDir, const Encoder_ChannelConfigType* cfg)
{
    if (cfg->ReverseDirection == TRUE)
    {
        return (hwDir == ENCODER_DIRECTION_FORWARD) ? ENCODER_DIRECTION_REVERSE : ENCODER_DIRECTION_FORWARD;
    }
    return hwDir;
}

static int32_t Encoder_ApplyDeadband(int32_t velocityCountsPerSec, const Encoder_FilterConfigType* filterCfg)
{
    if (filterCfg->DeadbandCountsPerSec == 0u)
    {
        return velocityCountsPerSec;
    }

    if (((velocityCountsPerSec >= 0) ? velocityCountsPerSec : -velocityCountsPerSec) < (int32_t)filterCfg->DeadbandCountsPerSec)
    {
        return 0;
    }
    return velocityCountsPerSec;
}

static int32_t Encoder_ApplyOutlierRejection(int32_t rawVelocity, int32_t lastFiltered, const Encoder_FilterConfigType* filterCfg)
{
    if (filterCfg->SpikeThresholdCountsPerSec == 0u)
    {
        return rawVelocity;
    }

    int32_t delta = rawVelocity - lastFiltered;
    int32_t absDelta = (delta >= 0) ? delta : -delta;
    if (absDelta > (int32_t)filterCfg->SpikeThresholdCountsPerSec)
    {
        /* Reject spike, keep previous filtered value */
        return lastFiltered;
    }
    return rawVelocity;
}

static float Encoder_CountsToRevolutions(int64_t counts, const Encoder_ChannelConfigType* cfg)
{
    if ((cfg == NULL_PTR) || (cfg->QuadratureCountsPerRev == 0u))
    {
        return 0.0f;
    }
    return ((float)counts) / ((float)cfg->QuadratureCountsPerRev);
}

static float Encoder_CountsToDegrees(int64_t counts, const Encoder_ChannelConfigType* cfg)
{
    float rev = Encoder_CountsToRevolutions(counts, cfg);
    return rev * 360.0f;
}

static float Encoder_CountsPerSecToRPM(int32_t countsPerSec, const Encoder_ChannelConfigType* cfg)
{
    if ((cfg == NULL_PTR) || (cfg->QuadratureCountsPerRev == 0u))
    {
        return 0.0f;
    }
    return (((float)countsPerSec) * 60.0f) / ((float)cfg->QuadratureCountsPerRev);
}

static void Encoder_UpdatePositionAccumulator(Encoder_ChannelStateType* state)
{
    const Encoder_ChannelConfigType* cfg = state->Cfg;
    uint32 current = Qei_GetPositionModule(cfg->QeiModule);
    uint32 maxVal = cfg->MaxPosition;
    uint64_t wrap = ((uint64_t)maxVal) + 1u;

    int64_t delta = (int64_t)current - (int64_t)state->LastHwPosition;

    /* Detect wrap-around; use half-range heuristic */
    int64_t halfRange = (int64_t)(wrap >> 1);
    if (delta > halfRange)
    {
        delta -= (int64_t)wrap;
    }
    else if (delta < -halfRange)
    {
        delta += (int64_t)wrap;
    }

    state->PositionAccum += delta;
    state->LastHwPosition = current;
}

static int32_t Encoder_ComputeVelocityCountsPerSec(Encoder_ChannelStateType* state)
{
    const Encoder_ChannelConfigType* cfg = state->Cfg;

    if ((cfg->VelocityTimerPeriodUs == 0u) || (cfg->QeiConfigPtr == NULL_PTR) || (cfg->QeiConfigPtr->EnableVelocityCapture == FALSE))
    {
        return 0;
    }

    uint32 rawCountsPerPeriod = Qei_GetVelocityModule(cfg->QeiModule);
    int32_t sign = (Encoder_MapDirection((Qei_GetDirectionModule(cfg->QeiModule) == QEI_DIRECTION_FORWARD) ? ENCODER_DIRECTION_FORWARD : ENCODER_DIRECTION_REVERSE, cfg) == ENCODER_DIRECTION_FORWARD) ? 1 : -1;

    uint64_t temp = ((uint64_t)rawCountsPerPeriod * 1000000u);
    int32_t countsPerSec = 0;
    if (cfg->VelocityTimerPeriodUs > 0u)
    {
        countsPerSec = (int32_t)(temp / ((uint64_t)cfg->VelocityTimerPeriodUs));
    }

    countsPerSec *= sign;
    return countsPerSec;
}

static int32_t Encoder_FilterVelocity(Encoder_ChannelStateType* state, int32_t rawVelocity)
{
    const Encoder_ChannelConfigType* cfg = state->Cfg;
    const Encoder_FilterConfigType* filterCfg = &cfg->FilterCfg;

    int32_t debounced = Encoder_ApplyDeadband(rawVelocity, filterCfg);
    int32_t candidate = Encoder_ApplyOutlierRejection(debounced, state->FilteredVelocity, filterCfg);

    if (cfg->EnableVelocityFilter == FALSE)
    {
        state->FilteredVelocity = candidate;
        return candidate;
    }

    uint8 alpha = Encoder_GetEffectiveAlpha(filterCfg);
    int64_t filtered = ((int64_t)alpha * (int64_t)candidate) + ((int64_t)(256u - alpha) * (int64_t)state->FilteredVelocity);
    filtered /= 256;
    state->FilteredVelocity = (int32_t)filtered;
    return state->FilteredVelocity;
}

static Std_ReturnType Encoder_CheckChannel(const Encoder_ChannelType Channel, const uint8 ApiId)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (Encoder_ConfigPtr == NULL_PTR)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ApiId, ENCODER_E_UNINIT);
        return E_NOT_OK;
    }

    if (!Encoder_IsChannelValid(Channel))
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ApiId, ENCODER_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#else
    (void)ApiId;
#endif

    return E_OK;
}

static void Encoder_ResetChannelState(Encoder_ChannelStateType* state)
{
    state->PositionAccum = 0u;
    state->LastHwPosition = 0u;
    state->FilteredVelocity = 0;
    state->LastRawVelocity = 0;
}

/* ===================[Public API]=================== */
void Encoder_Init(const Encoder_ConfigType* ConfigPtr)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_INIT_SID, ENCODER_E_PARAM_POINTER);
        return;
    }
#endif

    Encoder_ConfigPtr = ConfigPtr;
    uint8 idx;

    /* Initialize all configured channels */
    for (idx = 0u; idx < ConfigPtr->ChannelCount; ++idx)
    {
        const Encoder_ChannelConfigType* cfg = &ConfigPtr->Channels[idx];
        Encoder_ChannelStateType* state = &Encoder_ChannelStates[idx];

        (void)Qei_InitModule(cfg->QeiConfigPtr);
        state->Cfg = cfg;
        Encoder_ResetChannelState(state);
        state->LastHwPosition = Qei_GetPositionModule(cfg->QeiModule);
        state->Status = ENCODER_STATUS_RUNNING;
    }
}

void Encoder_DeInit(void)
{
    Qei_DeInit();
    uint8 idx;

    for (idx = 0u; idx < ENCODER_MAX_CHANNELS; ++idx)
    {
        Encoder_ChannelStates[idx].Status = ENCODER_STATUS_UNINIT;
    }

    Encoder_ConfigPtr = NULL_PTR;
}

float Encoder_GetPosition(Encoder_ChannelType Channel, Encoder_UnitType Unit)
{
    if (Encoder_CheckChannel(Channel, ENCODER_GET_POSITION_SID) != E_OK)
    {
        return 0.0f;
    }

    Encoder_ChannelStateType* state = Encoder_GetState(Channel);
    const Encoder_ChannelConfigType* cfg = state->Cfg;

    Encoder_UpdatePositionAccumulator(state);

    switch (Unit)
    {
        case ENCODER_UNIT_COUNTS:
            return (float)Encoder_GetPositionCounts(Channel);
        case ENCODER_UNIT_REVOLUTIONS:
            return Encoder_CountsToRevolutions(state->PositionAccum, cfg);
        case ENCODER_UNIT_DEGREES:
            return Encoder_CountsToDegrees(state->PositionAccum, cfg);
        default:
            return 0.0f;
    }
}

int64_t Encoder_GetPositionCounts(const Encoder_ChannelType Channel)
{
    if (Encoder_CheckChannel(Channel, ENCODER_GET_POSITION_SID) != E_OK)
    {
        return 0u;
    }

    Encoder_ChannelStateType* state = Encoder_GetState(Channel);
    Encoder_UpdatePositionAccumulator(state);
    return state->PositionAccum;
}

float Encoder_GetPositionRevolutions(const Encoder_ChannelType Channel)
{
    return Encoder_GetPosition(Channel, ENCODER_UNIT_REVOLUTIONS);
}

float Encoder_GetPositionDegrees(const Encoder_ChannelType Channel)
{
    float deg = Encoder_GetPosition(Channel, ENCODER_UNIT_DEGREES);

    while (deg < 0.0f)
    {
        deg += 360.0f;
    }
    while (deg >= 360.0f)
    {
        deg -= 360.0f;
    }

    return deg;
}

float Encoder_GetVelocity(Encoder_ChannelType Channel, Encoder_VelocityUnitType Unit)
{
    int32_t cps = Encoder_GetVelocityCountsPerSec(Channel);

    switch (Unit)
    {
        case ENCODER_VEL_UNIT_COUNTS_PER_SEC:
            return (float)cps;
        case ENCODER_VEL_UNIT_RPM:
        default:
        {
            const Encoder_ChannelConfigType* cfg = Encoder_GetConfig(Channel);
            return Encoder_CountsPerSecToRPM(cps, cfg);
        }
    }
}

int32_t Encoder_GetVelocityCountsPerSec(const Encoder_ChannelType Channel)
{
    if (Encoder_CheckChannel(Channel, ENCODER_GET_VELOCITY_SID) != E_OK)
    {
        return 0;
    }

    Encoder_ChannelStateType* state = Encoder_GetState(Channel);
    const Encoder_ChannelConfigType* cfg = state->Cfg;
    int32_t rawVelocity = Encoder_ComputeVelocityCountsPerSec(state);

    /* Apply filter pipeline */
    int32_t filtered = Encoder_FilterVelocity(state, rawVelocity);
    state->LastRawVelocity = rawVelocity;

    /* Track direction with ReverseDirection handling */
    (void)Encoder_MapDirection((Qei_GetDirectionModule(cfg->QeiModule) == QEI_DIRECTION_FORWARD) ? ENCODER_DIRECTION_FORWARD : ENCODER_DIRECTION_REVERSE, cfg);

    return filtered;
}

float Encoder_GetVelocityRPM(const Encoder_ChannelType Channel)
{
    return Encoder_GetVelocity(Channel, ENCODER_VEL_UNIT_RPM);
}

Encoder_DirectionType Encoder_GetDirection(const Encoder_ChannelType Channel)
{
    if (Encoder_CheckChannel(Channel, ENCODER_GET_DIRECTION_SID) != E_OK)
    {
        return ENCODER_DIRECTION_FORWARD;
    }

    Encoder_ChannelStateType* state = Encoder_GetState(Channel);
    const Encoder_ChannelConfigType* cfg = state->Cfg;
    Qei_DirectionType qeiDir = Qei_GetDirectionModule(cfg->QeiModule);
    Encoder_DirectionType hwDir = (qeiDir == QEI_DIRECTION_FORWARD) ? ENCODER_DIRECTION_FORWARD : ENCODER_DIRECTION_REVERSE;
    return Encoder_MapDirection(hwDir, cfg);
}

Encoder_StatusType Encoder_GetStatus(const Encoder_ChannelType Channel)
{
    if (!Encoder_IsChannelValid(Channel))
    {
        return ENCODER_STATUS_UNINIT;
    }

    return Encoder_ChannelStates[Channel].Status;
}

void Encoder_ResetPosition(const Encoder_ChannelType Channel)
{
    if (Encoder_CheckChannel(Channel, ENCODER_RESET_POSITION_SID) != E_OK)
    {
        return;
    }

    Encoder_ChannelStateType* state = Encoder_GetState(Channel);
    const Encoder_ChannelConfigType* cfg = state->Cfg;

    Qei_SetPositionModule(cfg->QeiModule, 0u);
    Encoder_ResetChannelState(state);
}

void Encoder_SetPosition(const Encoder_ChannelType Channel, uint32 Position)
{
    if (Encoder_CheckChannel(Channel, ENCODER_RESET_POSITION_SID) != E_OK)
    {
        return;
    }

    Encoder_ChannelStateType* state = Encoder_GetState(Channel);
    const Encoder_ChannelConfigType* cfg = state->Cfg;

    Qei_SetPositionModule(cfg->QeiModule, Position);
    state->PositionAccum = Position;
    state->LastHwPosition = Position;
}

Std_ReturnType Encoder_GetData(const Encoder_ChannelType Channel, Encoder_DataType* DataPtr)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (DataPtr == NULL_PTR)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_GET_DATA_SID, ENCODER_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif

    if (Encoder_CheckChannel(Channel, ENCODER_GET_DATA_SID) != E_OK)
    {
        return E_NOT_OK;
    }

    DataPtr->PositionCounts = Encoder_GetPositionCounts(Channel);
    DataPtr->PositionRevolutions = Encoder_GetPositionRevolutions(Channel);
    DataPtr->PositionDegrees = Encoder_GetPositionDegrees(Channel);
    DataPtr->Direction = Encoder_GetDirection(Channel);
    DataPtr->VelocityCountsPerSec = Encoder_GetVelocityCountsPerSec(Channel);
    DataPtr->VelocityRPM = Encoder_GetVelocityRPM(Channel);

    return E_OK;
}

void Encoder_Update(const Encoder_ChannelType Channel)
{
    if (Encoder_CheckChannel(Channel, ENCODER_GET_DATA_SID) != E_OK)
    {
        return;
    }

    Encoder_ChannelStateType* state = Encoder_GetState(Channel);
    Encoder_UpdatePositionAccumulator(state);
    (void)Encoder_GetVelocityCountsPerSec(Channel);
}

void Encoder_UpdateAll(void)
{
    if (Encoder_ConfigPtr == NULL_PTR)
    {
        return;
    }

    uint8 idx;
    for (idx = 0u; idx < Encoder_ConfigPtr->ChannelCount; ++idx)
    {
        Encoder_Update((Encoder_ChannelType)idx);
    }
}

#if (ENCODER_VERSION_INFO_API == STD_ON)
void Encoder_GetVersionInfo(Std_VersionInfoType* versionInfoPtr)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (versionInfoPtr == NULL_PTR)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_GET_DATA_SID, ENCODER_E_PARAM_POINTER);
        return;
    }
#endif

    versionInfoPtr->vendorID = ENCODER_VENDOR_ID;
    versionInfoPtr->moduleID = ENCODER_MODULE_ID;
    versionInfoPtr->sw_major_version = ENCODER_SW_MAJOR_VERSION;
    versionInfoPtr->sw_minor_version = ENCODER_SW_MINOR_VERSION;
    versionInfoPtr->sw_patch_version = ENCODER_SW_PATCH_VERSION;
}
#endif
