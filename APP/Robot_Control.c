/**
 * @file Robot_Control.c
 * @brief Robot Controller Application Implementation
 * @details Differential drive kinematics and control
 *
 * Kinematics:
 * - Left wheel:  v_l = v - (w * L / 2)
 * - Right wheel: v_r = v + (w * L / 2)
 * Where v = linear velocity, w = angular velocity, L = wheel base
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "Robot_Control.h"
#include "../ECUAL/MOTOR/Motor.h"
#include "../ECUAL/ENCODER/Encoder.h"
#include "../ECUAL/CURRENT_SENSOR/ACS712.h"
#include "../SERVICES/THERMAL/ThermalMgmt.h"
#include "../SERVICES/PID/PID.h"
#include <math.h>

/* ===================[Private Variables]=================== */
static Robot_StateType Robot_CurrentState = ROBOT_STATE_INIT;
static Robot_OdometryType Robot_Odometry = {0};
static Robot_TwistType Robot_TargetVelocity = {0};
static Robot_WheelVelType Robot_WheelVel = {0};

/* PID states for velocity control */
static PID_StateType Robot_PidLeft;
static PID_StateType Robot_PidRight;

static const PID_ConfigType Robot_PidConfig = {
    .Kp = 2.0f,
    .Ki = 0.5f,
    .Kd = 0.05f,
    .SampleTimeSec = 0.01f,
    .OutMin = -100.0f,
    .OutMax = 100.0f,
    .IntegratorMin = -50.0f,
    .IntegratorMax = 50.0f,
    .DerivativeOnMeasurement = TRUE,
    .DerivativeFilterAlpha = 0.1f
};

/* Last encoder values for odometry */
static sint32 Robot_LastLeftTicks = 0;
static sint32 Robot_LastRightTicks = 0;

/* ===================[Private Functions]=================== */

/**
 * @brief Convert linear/angular velocity to wheel velocities
 */
static void Robot_TwistToWheels(const Robot_TwistType* Twist, float32* LeftMps, float32* RightMps)
{
    float32 halfBase = ROBOT_WHEEL_BASE_M / 2.0f;
    
    *LeftMps = Twist->LinearX - (Twist->AngularZ * halfBase);
    *RightMps = Twist->LinearX + (Twist->AngularZ * halfBase);
}

/**
 * @brief Convert wheel velocities to linear/angular velocity
 */
static void Robot_WheelsToTwist(float32 LeftMps, float32 RightMps, float32* LinearX, float32* AngularZ)
{
    *LinearX = (LeftMps + RightMps) / 2.0f;
    *AngularZ = (RightMps - LeftMps) / ROBOT_WHEEL_BASE_M;
}

/**
 * @brief Convert wheel velocity (m/s) to motor command (0-100%)
 */
static uint8 Robot_VelToMotorCmd(float32 VelMps)
{
    float32 maxWheelVel;
    float32 percent;
    
    /* Max wheel velocity in m/s */
    maxWheelVel = (ROBOT_MAX_WHEEL_RPM / 60.0f) * (2.0f * 3.14159f * ROBOT_WHEEL_RADIUS_M);
    
    percent = (VelMps / maxWheelVel) * 100.0f;
    
    if (percent > 100.0f) percent = 100.0f;
    if (percent < -100.0f) percent = -100.0f;
    
    return (uint8)((percent >= 0.0f) ? percent : -percent);
}

/**
 * @brief Convert encoder ticks to meters
 */
static float32 Robot_TicksToMeters(sint32 Ticks)
{
    float32 revolutions;
    
    revolutions = (float32)Ticks / (float32)ROBOT_ENCODER_CPR;
    return revolutions * 2.0f * 3.14159f * ROBOT_WHEEL_RADIUS_M;
}

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize robot controller
 */
void Robot_Init(void)
{
    /* Initialize PID controllers */
    (void)PID_Init(&Robot_PidConfig, &Robot_PidLeft);
    (void)PID_Init(&Robot_PidConfig, &Robot_PidRight);
    
    /* Reset odometry */
    Robot_ResetOdometry();
    
    /* Reset target velocity */
    Robot_TargetVelocity.LinearX = 0.0f;
    Robot_TargetVelocity.LinearY = 0.0f;
    Robot_TargetVelocity.AngularZ = 0.0f;
    
    Robot_CurrentState = ROBOT_STATE_IDLE;
}

/**
 * @brief Process velocity command from ROS2
 */
Std_ReturnType Robot_SetVelocity(const Robot_TwistType* Cmd)
{
    if (Cmd == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    if (Robot_CurrentState == ROBOT_STATE_ESTOP)
    {
        return E_NOT_OK;  /* Cannot move in e-stop */
    }
    
    if (Robot_CurrentState == ROBOT_STATE_FAULT)
    {
        return E_NOT_OK;  /* Cannot move in fault */
    }
    
    /* Clamp velocities to limits */
    Robot_TargetVelocity.LinearX = Cmd->LinearX;
    if (Robot_TargetVelocity.LinearX > ROBOT_MAX_LINEAR_VEL)
    {
        Robot_TargetVelocity.LinearX = ROBOT_MAX_LINEAR_VEL;
    }
    else if (Robot_TargetVelocity.LinearX < -ROBOT_MAX_LINEAR_VEL)
    {
        Robot_TargetVelocity.LinearX = -ROBOT_MAX_LINEAR_VEL;
    }
    
    Robot_TargetVelocity.AngularZ = Cmd->AngularZ;
    if (Robot_TargetVelocity.AngularZ > ROBOT_MAX_ANGULAR_VEL)
    {
        Robot_TargetVelocity.AngularZ = ROBOT_MAX_ANGULAR_VEL;
    }
    else if (Robot_TargetVelocity.AngularZ < -ROBOT_MAX_ANGULAR_VEL)
    {
        Robot_TargetVelocity.AngularZ = -ROBOT_MAX_ANGULAR_VEL;
    }
    
    Robot_CurrentState = ROBOT_STATE_RUNNING;
    
    return E_OK;
}

/**
 * @brief Emergency stop
 */
void Robot_EmergencyStop(void)
{
    Robot_TargetVelocity.LinearX = 0.0f;
    Robot_TargetVelocity.LinearY = 0.0f;
    Robot_TargetVelocity.AngularZ = 0.0f;
    
    (void)PID_Reset(&Robot_PidLeft);
    (void)PID_Reset(&Robot_PidRight);
    
    Motor_StopAll();
    
    Robot_CurrentState = ROBOT_STATE_ESTOP;
}

/**
 * @brief Resume from emergency stop
 */
Std_ReturnType Robot_Resume(void)
{
    if (Robot_CurrentState != ROBOT_STATE_ESTOP)
    {
        return E_NOT_OK;
    }
    
    /* TODO: Check for active faults */
    
    Robot_CurrentState = ROBOT_STATE_IDLE;
    
    return E_OK;
}

/**
 * @brief Get current odometry
 */
Std_ReturnType Robot_GetOdometry(Robot_OdometryType* OdomPtr)
{
    if (OdomPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    *OdomPtr = Robot_Odometry;
    
    return E_OK;
}

/**
 * @brief Reset odometry to zero
 */
void Robot_ResetOdometry(void)
{
    Robot_Odometry.X = 0.0f;
    Robot_Odometry.Y = 0.0f;
    Robot_Odometry.Theta = 0.0f;
    Robot_Odometry.LinearVel = 0.0f;
    Robot_Odometry.AngularVel = 0.0f;
    
    Robot_LastLeftTicks = 0;
    Robot_LastRightTicks = 0;
}

/**
 * @brief Get wheel velocities
 */
Std_ReturnType Robot_GetWheelVelocities(Robot_WheelVelType* WheelPtr)
{
    if (WheelPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    *WheelPtr = Robot_WheelVel;
    
    return E_OK;
}

/**
 * @brief Get robot status
 */
Std_ReturnType Robot_GetStatus(Robot_StatusType* StatusPtr)
{
    float32 leftCurrent, rightCurrent;
    float32 maxTemp;
    
    if (StatusPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    StatusPtr->State = Robot_CurrentState;
    StatusPtr->BatteryVoltage = 12.0f;  /* TODO: Read from ADC */
    
    (void)ACS712_ReadCurrent(0u, &leftCurrent);
    (void)ACS712_ReadCurrent(1u, &rightCurrent);
    StatusPtr->LeftCurrent = leftCurrent;
    StatusPtr->RightCurrent = rightCurrent;
    
    (void)ThermalMgmt_GetMaxTemperature(&maxTemp, NULL_PTR);
    StatusPtr->MaxTemperature = maxTemp;
    
    StatusPtr->ErrorFlags = 0u;  /* TODO: Aggregate from safety task */
    
    return E_OK;
}

/**
 * @brief Get current state
 */
Robot_StateType Robot_GetState(void)
{
    return Robot_CurrentState;
}

/**
 * @brief Update control loop
 */
void Robot_UpdateControl(void)
{
    float32 targetLeftMps, targetRightMps;
    float32 pidOutputLeft, pidOutputRight;
    uint8 leftCmd, rightCmd;
    Motor_DirectionType leftDir, rightDir;
    Encoder_DataType encoderLeft, encoderRight;
    
    if ((Robot_CurrentState != ROBOT_STATE_RUNNING) && 
        (Robot_CurrentState != ROBOT_STATE_IDLE))
    {
        return;
    }
    
    /* Convert twist to wheel velocities */
    Robot_TwistToWheels(&Robot_TargetVelocity, &targetLeftMps, &targetRightMps);
    
    /* Get current wheel velocities from encoders */
    (void)Encoder_GetData(0u, &encoderLeft);
    (void)Encoder_GetData(1u, &encoderRight);
    
    /* TODO: Convert encoder velocity to m/s */
    Robot_WheelVel.LeftMps = encoderLeft.Velocity * 0.001f;  /* Placeholder */
    Robot_WheelVel.RightMps = encoderRight.Velocity * 0.001f;
    
    /* Run PID controllers */
    (void)PID_Compute(&Robot_PidConfig, &Robot_PidLeft,
                      targetLeftMps, Robot_WheelVel.LeftMps,
                      &pidOutputLeft);
    
    (void)PID_Compute(&Robot_PidConfig, &Robot_PidRight,
                      targetRightMps, Robot_WheelVel.RightMps,
                      &pidOutputRight);
    
    /* Convert to motor commands */
    leftCmd = Robot_VelToMotorCmd(pidOutputLeft);
    rightCmd = Robot_VelToMotorCmd(pidOutputRight);
    
    leftDir = (pidOutputLeft >= 0.0f) ? MOTOR_DIRECTION_FORWARD : MOTOR_DIRECTION_REVERSE;
    rightDir = (pidOutputRight >= 0.0f) ? MOTOR_DIRECTION_FORWARD : MOTOR_DIRECTION_REVERSE;
    
    /* Apply to motors */
    Motor_SetDirection(0u, leftDir);
    Motor_SetSpeed(0u, leftCmd);
    Motor_SetDirection(1u, rightDir);
    Motor_SetSpeed(1u, rightCmd);
}

/**
 * @brief Update odometry
 */
void Robot_UpdateOdometry(void)
{
    Encoder_DataType encoderLeft, encoderRight;
    sint32 deltaLeft, deltaRight;
    float32 distLeft, distRight;
    float32 distCenter, deltaTheta;
    
    /* Get current encoder values */
    (void)Encoder_GetData(0u, &encoderLeft);
    (void)Encoder_GetData(1u, &encoderRight);
    
    /* Calculate delta ticks */
    deltaLeft = encoderLeft.Position - Robot_LastLeftTicks;
    deltaRight = encoderRight.Position - Robot_LastRightTicks;
    
    Robot_LastLeftTicks = encoderLeft.Position;
    Robot_LastRightTicks = encoderRight.Position;
    
    /* Convert to distances */
    distLeft = Robot_TicksToMeters(deltaLeft);
    distRight = Robot_TicksToMeters(deltaRight);
    
    /* Calculate center distance and rotation */
    distCenter = (distLeft + distRight) / 2.0f;
    deltaTheta = (distRight - distLeft) / ROBOT_WHEEL_BASE_M;
    
    /* Update position using dead reckoning */
    Robot_Odometry.Theta += deltaTheta;
    
    /* Normalize theta to [-pi, pi] */
    while (Robot_Odometry.Theta > 3.14159f) Robot_Odometry.Theta -= 2.0f * 3.14159f;
    while (Robot_Odometry.Theta < -3.14159f) Robot_Odometry.Theta += 2.0f * 3.14159f;
    
    Robot_Odometry.X += distCenter * cosf(Robot_Odometry.Theta);
    Robot_Odometry.Y += distCenter * sinf(Robot_Odometry.Theta);
    
    /* Calculate current velocities (assuming 20ms period = 50Hz) */
    Robot_WheelsToTwist(Robot_WheelVel.LeftMps, Robot_WheelVel.RightMps,
                        &Robot_Odometry.LinearVel, &Robot_Odometry.AngularVel);
}
