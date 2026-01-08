/**
 * @file App_ControlTask.c
 * @brief Motor Control Task Implementation
 * @details PID-based motor control for differential drive robot
 *
 * Control Loop:
 * - Receives wheel speed setpoints from ROS2 (via Comm task)
 * - Reads encoder feedback
 * - Computes PID output
 * - Commands motor driver
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "../CONFIG/Std_Types.h"

/* Driver includes */
#include "../ECUAL/MOTOR/Motor.h"
#include "../ECUAL/ENCODER/Encoder.h"
#include "../SERVICES/PID/PID.h"

/* ===================[External Configurations]=================== */
extern const Motor_ConfigType Motor_Config;
extern const PID_ConfigType PID_Config;

/* ===================[Private Variables]=================== */
static boolean App_ControlInitialized = FALSE;

/* PID states for left and right motors */
static PID_StateType App_PidStateLeft;
static PID_StateType App_PidStateRight;

/* Setpoints from ROS2 (ticks/second or m/s) */
static float32 App_SetpointLeft = 0.0f;
static float32 App_SetpointRight = 0.0f;

/* PID configuration for both motors */
static const PID_ConfigType App_PidConfig = {
    .Kp = 1.0f,
    .Ki = 0.5f,
    .Kd = 0.1f,
    .SampleTimeSec = 0.01f,     /* 10ms control loop */
    .OutMin = -100.0f,
    .OutMax = 100.0f,
    .IntegratorMin = -50.0f,
    .IntegratorMax = 50.0f,
    .DerivativeOnMeasurement = TRUE,
    .DerivativeFilterAlpha = 0.1f
};

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize control task
 */
void App_ControlTask_Init(void)
{
    /* Initialize PID states */
    (void)PID_Init(&App_PidConfig, &App_PidStateLeft);
    (void)PID_Init(&App_PidConfig, &App_PidStateRight);
    
    App_SetpointLeft = 0.0f;
    App_SetpointRight = 0.0f;
    
    App_ControlInitialized = TRUE;
}

/**
 * @brief Control task main function (called by FreeRTOS task)
 */
void App_ControlTask_Run(void)
{
    Encoder_DataType encoderLeft;
    Encoder_DataType encoderRight;
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
    
    /* 1. Get encoder feedback */
    (void)Encoder_GetData(ENCODER_CHANNEL_LEFT, &encoderLeft);
    (void)Encoder_GetData(ENCODER_CHANNEL_RIGHT, &encoderRight);
    
    /* 2. Compute PID for left motor (using VelocityRPM from Encoder_DataType) */
    (void)PID_Compute(&App_PidConfig, &App_PidStateLeft,
                      App_SetpointLeft, encoderLeft.VelocityRPM,
                      &pidOutputLeft);
    
    /* 3. Compute PID for right motor */
    (void)PID_Compute(&App_PidConfig, &App_PidStateRight,
                      App_SetpointRight, encoderRight.VelocityRPM,
                      &pidOutputRight);
    
    /* 4. Convert PID output to motor commands */
    /* Left motor */
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
    
    /* Right motor */
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
    
    /* 5. Command motors */
    (void)Motor_SetDirection(0u, leftDir);
    (void)Motor_SetSpeed(0u, leftSpeed);
    (void)Motor_SetDirection(1u, rightDir);
    (void)Motor_SetSpeed(1u, rightSpeed);
}

/**
 * @brief Set wheel speed setpoints
 * @param[in] LeftSpeed Left wheel speed setpoint
 * @param[in] RightSpeed Right wheel speed setpoint
 */
void App_ControlTask_SetWheelSpeeds(float32 LeftSpeed, float32 RightSpeed)
{
    App_SetpointLeft = LeftSpeed;
    App_SetpointRight = RightSpeed;
}

/**
 * @brief Get current setpoints
 * @param[out] LeftSpeed Pointer to store left setpoint
 * @param[out] RightSpeed Pointer to store right setpoint
 */
void App_ControlTask_GetSetpoints(float32* LeftSpeed, float32* RightSpeed)
{
    if (LeftSpeed != NULL_PTR)
    {
        *LeftSpeed = App_SetpointLeft;
    }
    if (RightSpeed != NULL_PTR)
    {
        *RightSpeed = App_SetpointRight;
    }
}

/**
 * @brief Emergency stop - reset PID and stop motors
 */
void App_ControlTask_EmergencyStop(void)
{
    App_SetpointLeft = 0.0f;
    App_SetpointRight = 0.0f;
    
    (void)PID_Reset(&App_PidStateLeft);
    (void)PID_Reset(&App_PidStateRight);
    
    Motor_StopAll();
}
