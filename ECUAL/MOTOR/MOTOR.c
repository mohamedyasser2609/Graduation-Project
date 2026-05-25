/**
 * @file MOTOR.c
 * @brief Motor Driver Implementation for Cytron MDD10A Rev2.0
 * @details AUTOSAR-compliant driver for dual-channel motor control
 *
 * Hardware Interface:
 * - PWM channels for speed control (0-100% duty cycle)
 * - GPIO pins for direction control
 * - Supports forward, reverse, brake, and coast modes
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#include "MOTOR.h"
#include "../../MCAL/PWM/PWM.h"
#include "../../MCAL/GPIO/GPIO.h"

/* ===================[Private Types]=================== */
typedef struct {
    const Motor_ChannelConfigType* Cfg;
    Motor_StatusType Status;
    Motor_DirectionType Direction;
    Motor_SpeedType SpeedPercent;
    Pwm_DutyCycleType CurrentDutyCycle;
} Motor_ChannelStateType;

/* ===================[Private Variables]=================== */
static const Motor_ConfigType* Motor_ConfigPtr = NULL_PTR;
static Motor_ChannelStateType Motor_ChannelStates[MOTOR_MAX_CHANNELS];

/* ===================[Private Helper Functions]=================== */
static boolean Motor_IsChannelValid(Motor_ChannelType channel)
{
    if (Motor_ConfigPtr == NULL_PTR)
    {
        return FALSE;
    }
    return (channel < Motor_ConfigPtr->ChannelCount) && (channel < MOTOR_MAX_CHANNELS);
}

static Motor_ChannelStateType* Motor_GetState(Motor_ChannelType channel)
{
    if (!Motor_IsChannelValid(channel))
    {
        return NULL_PTR;
    }
    return &Motor_ChannelStates[channel];
}

static Pwm_DutyCycleType Motor_ConvertSpeedToDutyCycle(Motor_SpeedType speedPercent, const Motor_ChannelConfigType* cfg)
{
    if (cfg == NULL_PTR)
    {
        return 0u;
    }

    /* Clamp speed to valid range */
    if (speedPercent > MOTOR_MAX_SPEED_PERCENT)
    {
        speedPercent = MOTOR_MAX_SPEED_PERCENT;
    }
    if ((speedPercent > 0u) && (speedPercent < cfg->MinSpeedPercent))
    {
        speedPercent = cfg->MinSpeedPercent;
    }

    /* Convert percentage to PWM duty cycle (0-0x8000 range) */
    /* 0% = 0x0000, 100% = 0x8000 */
    uint32 dutyCycle = ((uint32)speedPercent * 0x8000u) / 100u;
    
    return (Pwm_DutyCycleType)dutyCycle;
}

static void Motor_ApplyDirection(Motor_ChannelType channel, Motor_DirectionType direction, const Motor_ChannelConfigType* cfg)
{
    Gpio_LevelType dirLevel;
    boolean invert = cfg->InvertDirection;

    switch (direction)
    {
        case MOTOR_DIRECTION_FORWARD:
            dirLevel = invert ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH;
            Gpio_WriteChannel(cfg->DirectionPin, dirLevel);
            break;

        case MOTOR_DIRECTION_REVERSE:
            dirLevel = invert ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW;
            Gpio_WriteChannel(cfg->DirectionPin, dirLevel);
            break;

        case MOTOR_DIRECTION_BRAKE:
            /* For brake mode, set direction pin HIGH and disable PWM */
            Gpio_WriteChannel(cfg->DirectionPin, GPIO_LEVEL_HIGH);
            Pwm_SetDutyCycle(cfg->PwmChannel, 0u);
            break;

        case MOTOR_DIRECTION_COAST:
            /* For coast mode, set direction pin LOW and disable PWM */
            Gpio_WriteChannel(cfg->DirectionPin, GPIO_LEVEL_LOW);
            Pwm_SetDutyCycle(cfg->PwmChannel, 0u);
            break;

        default:
            /* Invalid direction - do nothing */
            break;
    }
}

static Std_ReturnType Motor_CheckChannel(const Motor_ChannelType Channel, const uint8 ApiId)
{
#if (MOTOR_DEV_ERROR_DETECT == STD_ON)
    if (Motor_ConfigPtr == NULL_PTR)
    {
        Det_ReportError(MOTOR_MODULE_ID, MOTOR_INSTANCE_ID, ApiId, MOTOR_E_UNINIT);
        return E_NOT_OK;
    }

    if (!Motor_IsChannelValid(Channel))
    {
        Det_ReportError(MOTOR_MODULE_ID, MOTOR_INSTANCE_ID, ApiId, MOTOR_E_PARAM_CHANNEL);
        return E_NOT_OK;
    }
#else
    (void)ApiId;
#endif

    return E_OK;
}

/* ===================[Public API]=================== */
void Motor_Init(const Motor_ConfigType* ConfigPtr)
{
#if (MOTOR_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(MOTOR_MODULE_ID, MOTOR_INSTANCE_ID, MOTOR_INIT_SID, MOTOR_E_PARAM_POINTER);
        return;
    }

    if (ConfigPtr->ChannelCount == 0u || ConfigPtr->ChannelCount > MOTOR_MAX_CHANNELS)
    {
        Det_ReportError(MOTOR_MODULE_ID, MOTOR_INSTANCE_ID, MOTOR_INIT_SID, MOTOR_E_PARAM_CHANNEL);
        return;
    }

    if (ConfigPtr->Channels == NULL_PTR)
    {
        Det_ReportError(MOTOR_MODULE_ID, MOTOR_INSTANCE_ID, MOTOR_INIT_SID, MOTOR_E_PARAM_POINTER);
        return;
    }
#endif

    Motor_ConfigPtr = ConfigPtr;

    /* Initialize all configured channels */
    uint8 idx;
    for (idx = 0u; idx < ConfigPtr->ChannelCount; ++idx)
    {
        const Motor_ChannelConfigType* cfg = &ConfigPtr->Channels[idx];
        Motor_ChannelStateType* state = &Motor_ChannelStates[idx];

        state->Cfg = cfg;
        state->Status = MOTOR_STATUS_IDLE;
        state->Direction = MOTOR_DIRECTION_COAST;
        state->SpeedPercent = 0u;
        state->CurrentDutyCycle = 0u;

        /* Initialize motor to stopped state (coast) */
        Motor_ApplyDirection((Motor_ChannelType)idx, MOTOR_DIRECTION_COAST, cfg);
    }
}

#if (MOTOR_DE_INIT_API == STD_ON)
void Motor_DeInit(void)
{
    if (Motor_ConfigPtr == NULL_PTR)
    {
        return;
    }

    /* Stop all motors */
    Motor_StopAll();

    /* Clear configuration */
    Motor_ConfigPtr = NULL_PTR;

    /* Reset all channel states */
    uint8 idx;
    for (idx = 0u; idx < MOTOR_MAX_CHANNELS; ++idx)
    {
        Motor_ChannelStates[idx].Status = MOTOR_STATUS_UNINIT;
        Motor_ChannelStates[idx].Direction = MOTOR_DIRECTION_COAST;
        Motor_ChannelStates[idx].SpeedPercent = 0u;
        Motor_ChannelStates[idx].CurrentDutyCycle = 0u;
    }
}
#endif

Std_ReturnType Motor_SetSpeed(Motor_ChannelType Channel, Motor_SpeedType SpeedPercent)
{
    if (Motor_CheckChannel(Channel, MOTOR_SET_SPEED_SID) != E_OK)
    {
        return E_NOT_OK;
    }

#if (MOTOR_DEV_ERROR_DETECT == STD_ON)
    if (SpeedPercent > MOTOR_MAX_SPEED_PERCENT)
    {
        Det_ReportError(MOTOR_MODULE_ID, MOTOR_INSTANCE_ID, MOTOR_SET_SPEED_SID, MOTOR_E_PARAM_SPEED);
        return E_NOT_OK;
    }
#endif

    Motor_ChannelStateType* state = Motor_GetState(Channel);
    const Motor_ChannelConfigType* cfg = state->Cfg;

    /* Apply speed limits */
    if (SpeedPercent > cfg->MaxSpeedPercent)
    {
        SpeedPercent = cfg->MaxSpeedPercent;
    }
    else if ((SpeedPercent > 0u) && (SpeedPercent < cfg->MinSpeedPercent))
    {
        SpeedPercent = cfg->MinSpeedPercent;
    }

    /* Convert speed to duty cycle */
    Pwm_DutyCycleType dutyCycle = Motor_ConvertSpeedToDutyCycle(SpeedPercent, cfg);

    /* Set PWM duty cycle */
    Pwm_SetDutyCycle(cfg->PwmChannel, dutyCycle);

    /* Update state */
    state->SpeedPercent = SpeedPercent;
    state->CurrentDutyCycle = dutyCycle;

    /* Update status */
    if (SpeedPercent > 0u && state->Direction != MOTOR_DIRECTION_BRAKE && state->Direction != MOTOR_DIRECTION_COAST)
    {
        state->Status = MOTOR_STATUS_RUNNING;
    }
    else
    {
        state->Status = MOTOR_STATUS_IDLE;
    }

    return E_OK;
}

Std_ReturnType Motor_SetDirection(Motor_ChannelType Channel, Motor_DirectionType Direction)
{
    if (Motor_CheckChannel(Channel, MOTOR_SET_DIRECTION_SID) != E_OK)
    {
        return E_NOT_OK;
    }

#if (MOTOR_DEV_ERROR_DETECT == STD_ON)
    if (Direction > MOTOR_DIRECTION_COAST)
    {
        Det_ReportError(MOTOR_MODULE_ID, MOTOR_INSTANCE_ID, MOTOR_SET_DIRECTION_SID, MOTOR_E_PARAM_DIRECTION);
        return E_NOT_OK;
    }
#endif

    Motor_ChannelStateType* state = Motor_GetState(Channel);
    const Motor_ChannelConfigType* cfg = state->Cfg;

    /* Apply direction */
    Motor_ApplyDirection(Channel, Direction, cfg);

    /* Update state */
    state->Direction = Direction;

    /* Update status */
    if (Direction == MOTOR_DIRECTION_BRAKE || Direction == MOTOR_DIRECTION_COAST)
    {
        state->Status = MOTOR_STATUS_IDLE;
        state->SpeedPercent = 0u;
        state->CurrentDutyCycle = 0u;
    }
    else if (state->SpeedPercent > 0u)
    {
        state->Status = MOTOR_STATUS_RUNNING;
    }
    else
    {
        state->Status = MOTOR_STATUS_IDLE;
    }

    return E_OK;
}

Std_ReturnType Motor_SetSpeedAndDirection(Motor_ChannelType Channel, Motor_SpeedType SpeedPercent, Motor_DirectionType Direction)
{
    if (Motor_CheckChannel(Channel, MOTOR_SET_SPEED_AND_DIRECTION_SID) != E_OK)
    {
        return E_NOT_OK;
    }

    /* Set direction first */
    if (Motor_SetDirection(Channel, Direction) != E_OK)
    {
        return E_NOT_OK;
    }

    /* Then set speed (only if not brake/coast) */
    if (Direction != MOTOR_DIRECTION_BRAKE && Direction != MOTOR_DIRECTION_COAST)
    {
        if (Motor_SetSpeed(Channel, SpeedPercent) != E_OK)
        {
            return E_NOT_OK;
        }
    }

    return E_OK;
}

Std_ReturnType Motor_Stop(Motor_ChannelType Channel)
{
    return Motor_SetDirection(Channel, MOTOR_DIRECTION_BRAKE);
}

void Motor_StopAll(void)
{
    if (Motor_ConfigPtr == NULL_PTR)
    {
        return;
    }

    uint8 idx;
    for (idx = 0u; idx < Motor_ConfigPtr->ChannelCount; ++idx)
    {
        (void)Motor_Stop((Motor_ChannelType)idx);
    }
}

Motor_StatusType Motor_GetStatus(Motor_ChannelType Channel)
{
    if (!Motor_IsChannelValid(Channel))
    {
        return MOTOR_STATUS_UNINIT;
    }

    return Motor_ChannelStates[Channel].Status;
}

Std_ReturnType Motor_GetData(Motor_ChannelType Channel, Motor_DataType* DataPtr)
{
#if (MOTOR_DEV_ERROR_DETECT == STD_ON)
    if (DataPtr == NULL_PTR)
    {
        Det_ReportError(MOTOR_MODULE_ID, MOTOR_INSTANCE_ID, MOTOR_GET_DATA_SID, MOTOR_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif

    if (Motor_CheckChannel(Channel, MOTOR_GET_DATA_SID) != E_OK)
    {
        return E_NOT_OK;
    }

    Motor_ChannelStateType* state = Motor_GetState(Channel);

    DataPtr->Status = state->Status;
    DataPtr->Direction = state->Direction;
    DataPtr->SpeedPercent = state->SpeedPercent;
    DataPtr->PwmDutyCycle = state->CurrentDutyCycle;

    return E_OK;
}

#if (MOTOR_VERSION_INFO_API == STD_ON)
void Motor_GetVersionInfo(Std_VersionInfoType* versionInfoPtr)
{
#if (MOTOR_DEV_ERROR_DETECT == STD_ON)
    if (versionInfoPtr == NULL_PTR)
    {
        Det_ReportError(MOTOR_MODULE_ID, MOTOR_INSTANCE_ID, MOTOR_GET_DATA_SID, MOTOR_E_PARAM_POINTER);
        return;
    }
#endif

    versionInfoPtr->vendorID = MOTOR_VENDOR_ID;
    versionInfoPtr->moduleID = MOTOR_MODULE_ID;
    versionInfoPtr->sw_major_version = MOTOR_SW_MAJOR_VERSION;
    versionInfoPtr->sw_minor_version = MOTOR_SW_MINOR_VERSION;
    versionInfoPtr->sw_patch_version = MOTOR_SW_PATCH_VERSION;
}
#endif
