/**
 * @file App_ControlTask.c
 * @brief Motor Control Task Implementation
 * @details PID-based motor control consuming wheel speed commands from queue
 *
 * Receives: WheelSpeedCmdType from Comm Task via queue
 * Outputs:  PWM commands to motor driver
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.1.0
 */

#include "../../CONFIG/Std_Types.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "queue.h"

/* Driver includes */
#include "../../ECUAL/MOTOR/Motor.h"
#include "../../ECUAL/ENCODER/Encoder.h"
#include "../../SERVICES/PID/PID.h"
#include "../../SERVICES/RTOS/Tasks_Init.h"

/* Safe state manager - PRIVILEGE CHECK */
#include "../Safety/App_SafeState.h"

/* Shared types */
#include "../Common/App_SharedTypes.h"

/* ===================[Private Variables]=================== */
static boolean App_ControlInitialized = FALSE;

/* PID states for left and right motors */
static PID_StateType App_PidStateLeft;
static PID_StateType App_PidStateRight;

/* Target wheel speeds from ROS2 (rad/s) */
static float32 App_TargetLeftRadS = 0.0f;
static float32 App_TargetRightRadS = 0.0f;

/* Command timeout (ticks) */
#define CMD_TIMEOUT_TICKS   (100u)  /* ~1 second at 100Hz */
static uint32 App_LastCmdTime = 0u;

/* Queue handle */
static QueueHandle_t App_WheelCmdQueue = NULL;

/* PID configuration */
static const PID_ConfigType App_PidConfig = {
    .Kp = 2.0f,
    .Ki = 0.8f,
    .Kd = 0.05f,
    .SampleTimeSec = 0.01f,
    .OutMin = -100.0f,
    .OutMax = 100.0f,
    .IntegratorMin = -50.0f,
    .IntegratorMax = 50.0f,
    .DerivativeOnMeasurement = TRUE,
    .DerivativeFilterAlpha = 0.1f
};

/* ===================[Private Functions]=================== */

/**
 * @brief Convert rad/s to RPM
 */
static float32 RadSToRPM(float32 radPerSec)
{
    return radPerSec * RAD_TO_RPM;
}

/**
 * @brief Check for new commands from Comm task
 */
static void App_CheckCommandQueue(void)
{
    WheelSpeedCmdType cmd;
    
    if (App_WheelCmdQueue == NULL)
    {
        return;
    }
    
    /* Non-blocking receive from queue */
    if (xQueueReceive(App_WheelCmdQueue, &cmd, 0) == pdTRUE)
    {
        if (cmd.Valid)
        {
            App_TargetLeftRadS = cmd.LeftRadPerSec;
            App_TargetRightRadS = cmd.RightRadPerSec;
            App_LastCmdTime = xTaskGetTickCount();
        }
    }
    
    /* Check for command timeout - stop if no commands received */
    if ((xTaskGetTickCount() - App_LastCmdTime) > CMD_TIMEOUT_TICKS)
    {
        App_TargetLeftRadS = 0.0f;
        App_TargetRightRadS = 0.0f;
    }
}

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize control task
 */
void App_ControlTask_Init(void)
{
    /* Get queue handle */
    App_WheelCmdQueue = Tasks_GetWheelSpeedCmdQueue();
    
    /* Initialize PID states */
    (void)PID_Init(&App_PidConfig, &App_PidStateLeft);
    (void)PID_Init(&App_PidConfig, &App_PidStateRight);
    
    App_TargetLeftRadS = 0.0f;
    App_TargetRightRadS = 0.0f;
    App_LastCmdTime = xTaskGetTickCount();
    
    App_ControlInitialized = TRUE;
}

/**
 * @brief Control task main function (called by FreeRTOS task)
 */
void App_ControlTask_Run(void)
{
    Encoder_DataType encoderLeft;
    Encoder_DataType encoderRight;
    float32 targetLeftRPM;
    float32 targetRightRPM;
    float32 pidOutputLeft;
    float32 pidOutputRight;
    uint8 leftSpeed;
    uint8 rightSpeed;
    Motor_DirectionType leftDir;
    Motor_DirectionType rightDir;
    
    if (App_ControlInitialized == FALSE)
    {
        App_ControlTask_Init();
    }
    
    /* 1. Check for new commands from Comm task */
    App_CheckCommandQueue();
    
    /* 2. Convert rad/s targets to RPM */
    targetLeftRPM = RadSToRPM(App_TargetLeftRadS);
    targetRightRPM = RadSToRPM(App_TargetRightRadS);
    
    /* 3. Get encoder feedback */
    (void)Encoder_GetData(ENCODER_CHANNEL_LEFT, &encoderLeft);
    (void)Encoder_GetData(ENCODER_CHANNEL_RIGHT, &encoderRight);
    
    /* 4. Compute PID for left motor */
    (void)PID_Compute(&App_PidConfig, &App_PidStateLeft,
                      targetLeftRPM, encoderLeft.VelocityRPM,
                      &pidOutputLeft);
    
    /* 5. Compute PID for right motor */
    (void)PID_Compute(&App_PidConfig, &App_PidStateRight,
                      targetRightRPM, encoderRight.VelocityRPM,
                      &pidOutputRight);
    
    /* 6. Convert PID output to motor commands */
    if (pidOutputLeft >= 0.0f)
    {
        leftDir = MOTOR_DIRECTION_FORWARD;
        leftSpeed = (uint8)((pidOutputLeft > 100.0f) ? 100u : (uint8)pidOutputLeft);
    }
    else
    {
        leftDir = MOTOR_DIRECTION_REVERSE;
        leftSpeed = (uint8)((-pidOutputLeft > 100.0f) ? 100u : (uint8)(-pidOutputLeft));
    }
    
    if (pidOutputRight >= 0.0f)
    {
        rightDir = MOTOR_DIRECTION_FORWARD;
        rightSpeed = (uint8)((pidOutputRight > 100.0f) ? 100u : (uint8)pidOutputRight);
    }
    else
    {
        rightDir = MOTOR_DIRECTION_REVERSE;
        rightSpeed = (uint8)((-pidOutputRight > 100.0f) ? 100u : (uint8)(-pidOutputRight));
    }
    
    /* 7. PRIVILEGE CHECK: Only command motors if SafeState allows */
    if (SafeState_IsMotorEnableAllowed())
    {
        (void)Motor_SetDirection(MOTOR_CHANNEL_LEFT, leftDir);
        (void)Motor_SetSpeed(MOTOR_CHANNEL_LEFT, leftSpeed);
        (void)Motor_SetDirection(MOTOR_CHANNEL_RIGHT, rightDir);
        (void)Motor_SetSpeed(MOTOR_CHANNEL_RIGHT, rightSpeed);
    }
    else
    {
        /* Safety has disabled motors - ensure they are stopped */
        (void)Motor_Stop(MOTOR_CHANNEL_LEFT);
        (void)Motor_Stop(MOTOR_CHANNEL_RIGHT);
    }
}

/**
 * @brief Set wheel speed targets directly (for Robot_Control module)
 */
void App_ControlTask_SetWheelSpeeds(float32 LeftRadS, float32 RightRadS)
{
    App_TargetLeftRadS = LeftRadS;
    App_TargetRightRadS = RightRadS;
    App_LastCmdTime = xTaskGetTickCount();
}

/**
 * @brief Get current targets
 */
void App_ControlTask_GetSetpoints(float32* LeftRadS, float32* RightRadS)
{
    if (LeftRadS != NULL_PTR)
    {
        *LeftRadS = App_TargetLeftRadS;
    }
    if (RightRadS != NULL_PTR)
    {
        *RightRadS = App_TargetRightRadS;
    }
}

/**
 * @brief Emergency stop - reset PID and stop motors
 */
void App_ControlTask_EmergencyStop(void)
{
    App_TargetLeftRadS = 0.0f;
    App_TargetRightRadS = 0.0f;
    
    (void)PID_Reset(&App_PidStateLeft);
    (void)PID_Reset(&App_PidStateRight);
    
    Motor_StopAll();
}
