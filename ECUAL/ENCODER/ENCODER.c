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

/* ===================[Private Variables]=================== */
static const Encoder_ConfigType* Encoder_ConfigPtr = NULL_PTR;
static Encoder_StatusType Encoder_ModuleStatus = ENCODER_STATUS_UNINIT;
static uint32 Encoder_LastPosition = 0u;
static uint32 Encoder_LastVelocityRaw = 0u;
static uint32 Encoder_FilteredVelocity = 0u;
static uint32 Encoder_LastUpdateTime = 0u;

/* ===================[Private Helper Functions]=================== */

/**
 * @brief Calculate position in revolutions from counts
 */
static float Encoder_CountsToRevolutions(uint32 counts)
{
    if (Encoder_ConfigPtr == NULL_PTR)
    {
        return 0.0f;
    }
    
    return ((float)counts) / ((float)Encoder_ConfigPtr->QuadratureCountsPerRev);
}

/**
 * @brief Calculate position in degrees from counts
 */
static float Encoder_CountsToDegrees(uint32 counts)
{
    if (Encoder_ConfigPtr == NULL_PTR)
    {
        return 0.0f;
    }
    
    float revolutions = Encoder_CountsToRevolutions(counts);
    return revolutions * 360.0f;
}

/**
 * @brief Calculate velocity in RPM from counts per second
 */
static float Encoder_CountsPerSecToRPM(uint32 countsPerSec)
{
    if (Encoder_ConfigPtr == NULL_PTR)
    {
        return 0.0f;
    }
    
    /* RPM = (counts/sec × 60) / (counts per revolution) */
    return ((float)countsPerSec * 60.0f) / ((float)Encoder_ConfigPtr->QuadratureCountsPerRev);
}

/**
 * @brief Apply velocity filtering (exponential moving average)
 */
static uint32 Encoder_FilterVelocity(uint32 rawVelocity)
{
    if (Encoder_ConfigPtr == NULL_PTR || Encoder_ConfigPtr->EnableVelocityFilter == FALSE)
    {
        return rawVelocity;
    }
    
    /* Exponential moving average filter */
    /* filtered = (alpha × new + (256 - alpha) × old) / 256 */
    uint32 alpha = Encoder_ConfigPtr->VelocityFilterAlpha;
    uint32 filtered = ((alpha * rawVelocity) + ((256u - alpha) * Encoder_FilteredVelocity)) / 256u;
    
    return filtered;
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
    
    if (Encoder_ModuleStatus != ENCODER_STATUS_UNINIT)
    {
        /* Already initialized - reinitialize QEI */
        Qei_DeInit();
    }
#endif

    /* Initialize QEI driver with default configuration */
    Qei_Init(&Qei_Config);
    
    /* Store encoder configuration */
    Encoder_ConfigPtr = ConfigPtr;
    
    /* Initialize private variables */
    Encoder_LastPosition = Qei_GetPosition();
    Encoder_LastVelocityRaw = 0u;
    Encoder_FilteredVelocity = 0u;
    Encoder_LastUpdateTime = 0u;
    
    Encoder_ModuleStatus = ENCODER_STATUS_RUNNING;
}

void Encoder_DeInit(void)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (Encoder_ModuleStatus == ENCODER_STATUS_UNINIT)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_DEINIT_SID, ENCODER_E_UNINIT);
        return;
    }
#endif

    Qei_DeInit();
    
    Encoder_ConfigPtr = NULL_PTR;
    Encoder_ModuleStatus = ENCODER_STATUS_UNINIT;
}

float Encoder_GetPosition(Encoder_UnitType Unit)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (Encoder_ModuleStatus == ENCODER_STATUS_UNINIT)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_GET_POSITION_SID, ENCODER_E_UNINIT);
        return 0.0f;
    }
    
    if (Unit > ENCODER_UNIT_DEGREES)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_GET_POSITION_SID, ENCODER_E_PARAM_UNIT);
        return 0.0f;
    }
#endif

    uint32 counts = Qei_GetPosition();
    
    switch (Unit)
    {
        case ENCODER_UNIT_COUNTS:
            return (float)counts;
            
        case ENCODER_UNIT_REVOLUTIONS:
            return Encoder_CountsToRevolutions(counts);
            
        case ENCODER_UNIT_DEGREES:
            return Encoder_CountsToDegrees(counts);
            
        default:
            return 0.0f;
    }
}

uint32 Encoder_GetPositionCounts(void)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (Encoder_ModuleStatus == ENCODER_STATUS_UNINIT)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_GET_POSITION_SID, ENCODER_E_UNINIT);
        return 0u;
    }
#endif

    return Qei_GetPosition();
}

float Encoder_GetPositionRevolutions(void)
{
    return Encoder_GetPosition(ENCODER_UNIT_REVOLUTIONS);
}

float Encoder_GetPositionDegrees(void)
{
    float degrees = Encoder_GetPosition(ENCODER_UNIT_DEGREES);
    
    /* Normalize to 0-360 range */
    while (degrees < 0.0f)
    {
        degrees += 360.0f;
    }
    while (degrees >= 360.0f)
    {
        degrees -= 360.0f;
    }
    
    return degrees;
}

float Encoder_GetVelocity(Encoder_VelocityUnitType Unit)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (Encoder_ModuleStatus == ENCODER_STATUS_UNINIT)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_GET_VELOCITY_SID, ENCODER_E_UNINIT);
        return 0.0f;
    }
    
    if (Unit > ENCODER_VEL_UNIT_RPM)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_GET_VELOCITY_SID, ENCODER_E_PARAM_UNIT);
        return 0.0f;
    }
#endif

    uint32 countsPerSec = Encoder_GetVelocityCountsPerSec();
    
    switch (Unit)
    {
        case ENCODER_VEL_UNIT_COUNTS_PER_SEC:
            return (float)countsPerSec;
            
        case ENCODER_VEL_UNIT_RPM:
            return Encoder_CountsPerSecToRPM(countsPerSec);
            
        default:
            return 0.0f;
    }
}

uint32 Encoder_GetVelocityCountsPerSec(void)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (Encoder_ModuleStatus == ENCODER_STATUS_UNINIT)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_GET_VELOCITY_SID, ENCODER_E_UNINIT);
        return 0u;
    }
#endif

    if (Encoder_ConfigPtr == NULL_PTR)
    {
        return 0u;
    }

    /* Get raw velocity from QEI (counts per timer period) */
    uint32 rawVelocity = Qei_GetVelocity();
    
    /* Convert from counts per timer period to counts per second */
    /* counts/sec = (counts/timer_period) × (1,000,000 / timer_period_us) */
    uint32 velocityCountsPerSec = 0u;
    if (Encoder_ConfigPtr->VelocityTimerPeriodUs > 0u)
    {
        /* Calculate: velocity = rawVelocity × (1,000,000 / timer_period_us) */
        /* Use 64-bit intermediate to avoid overflow */
        uint64 temp = ((uint64)rawVelocity * 1000000u) / ((uint64)Encoder_ConfigPtr->VelocityTimerPeriodUs);
        velocityCountsPerSec = (uint32)temp;
    }
    
    /* Apply filtering if enabled */
    if (Encoder_ConfigPtr->EnableVelocityFilter == TRUE)
    {
        Encoder_FilteredVelocity = Encoder_FilterVelocity(velocityCountsPerSec);
        return Encoder_FilteredVelocity;
    }
    
    return velocityCountsPerSec;
}

float Encoder_GetVelocityRPM(void)
{
    return Encoder_GetVelocity(ENCODER_VEL_UNIT_RPM);
}

Encoder_DirectionType Encoder_GetDirection(void)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (Encoder_ModuleStatus == ENCODER_STATUS_UNINIT)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_GET_DIRECTION_SID, ENCODER_E_UNINIT);
        return ENCODER_DIRECTION_FORWARD;
    }
#endif

    Qei_DirectionType qeiDir = Qei_GetDirection();
    
    if (qeiDir == QEI_DIRECTION_FORWARD)
    {
        return ENCODER_DIRECTION_FORWARD;
    }
    else
    {
        return ENCODER_DIRECTION_REVERSE;
    }
}

Encoder_StatusType Encoder_GetStatus(void)
{
    return Encoder_ModuleStatus;
}

void Encoder_ResetPosition(void)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (Encoder_ModuleStatus == ENCODER_STATUS_UNINIT)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_RESET_POSITION_SID, ENCODER_E_UNINIT);
        return;
    }
#endif

    Qei_SetPosition(0u);
    Encoder_LastPosition = 0u;
}

void Encoder_SetPosition(uint32 Position)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (Encoder_ModuleStatus == ENCODER_STATUS_UNINIT)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_RESET_POSITION_SID, ENCODER_E_UNINIT);
        return;
    }
#endif

    Qei_SetPosition(Position);
    Encoder_LastPosition = Position;
}

Std_ReturnType Encoder_GetData(Encoder_DataType* DataPtr)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (Encoder_ModuleStatus == ENCODER_STATUS_UNINIT)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_GET_DATA_SID, ENCODER_E_UNINIT);
        return E_NOT_OK;
    }
    
    if (DataPtr == NULL_PTR)
    {
        Det_ReportError(ENCODER_MODULE_ID, ENCODER_INSTANCE_ID, ENCODER_GET_DATA_SID, ENCODER_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif

    /* Get all encoder data */
    DataPtr->PositionCounts = Encoder_GetPositionCounts();
    DataPtr->PositionRevolutions = Encoder_GetPositionRevolutions();
    DataPtr->PositionDegrees = Encoder_GetPositionDegrees();
    DataPtr->Direction = Encoder_GetDirection();
    DataPtr->VelocityCountsPerSec = Encoder_GetVelocityCountsPerSec();
    DataPtr->VelocityRPM = Encoder_GetVelocityRPM();
    
    return E_OK;
}

void Encoder_Update(void)
{
#if (ENCODER_DEV_ERROR_DETECT == STD_ON)
    if (Encoder_ModuleStatus == ENCODER_STATUS_UNINIT)
    {
        return;
    }
#endif

    /* Update filtered velocity */
    (void)Encoder_GetVelocityCountsPerSec();
    
    /* Store current position for next update */
    Encoder_LastPosition = Qei_GetPosition();
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
