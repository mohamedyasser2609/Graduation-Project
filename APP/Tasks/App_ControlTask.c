/**
 * @file App_ControlTask.c
 * @brief Motor Control Task Implementation — Robot_Control Proxy
 * @details Bridges the FreeRTOS queue interface with Robot_Control module.
 *
 * This task:
 *   1. Reads WheelSpeedCmdType from queue (from Comm Task, binary ComStack)
 *   2. Converts wheel rad/s to Robot_TwistType (linear + angular)
 *   3. Calls Robot_SetVelocity() to set the target
 *   4. Calls Robot_UpdateControl() at 100Hz (PID runs inside Robot_Control)
 *   5. Checks SafeState before allowing motor commands
 *
 * Design decision (P0.5):
 *   Robot_Control.c is the single PID controller (operates in m/s,
 *   handles kinematics + odometry + state management). This task
 *   is intentionally thin — it only adapts the queue interface.
 *
 * @author Mohamed Yasser
 * @date Apr 13, 2026
 * @version 2.0.0 — Rewritten as Robot_Control proxy (was duplicate PID)
 */

#include "../../CONFIG/Std_Types.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "queue.h"

/* APP includes */
#include "../Control/Robot_Control.h"
#include "../Common/App_SharedTypes.h"

/* Safe state manager — PRIVILEGE CHECK */
#include "../Safety/App_SafeState.h"

/* Queue access */
#include "../../SERVICES/RTOS/Tasks_Init.h"

/* Encoder for odometry update */
#include "../../ECUAL/ENCODER/Encoder.h"

/* ===================[Private Variables]=================== */
static boolean App_ControlInitialized = FALSE;

/* Target wheel speeds from Comm Task (rad/s) */
static float32 App_TargetLeftRadS = 0.0f;
static float32 App_TargetRightRadS = 0.0f;

/* Command timeout (ticks) */
#define CMD_TIMEOUT_TICKS   (100u)  /* ~1 second at 100Hz tick rate */
static uint32 App_LastCmdTime = 0u;

/* Queue handle */
static QueueHandle_t App_WheelCmdQueue = NULL;

/* ===================[Private Functions]=================== */

/**
 * @brief Check for new commands from Comm task queue
 * @details Reads WheelSpeedCmdType and applies timeout if no commands
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

    /* Check for command timeout — stop if no commands received */
    if ((xTaskGetTickCount() - App_LastCmdTime) > CMD_TIMEOUT_TICKS)
    {
        App_TargetLeftRadS = 0.0f;
        App_TargetRightRadS = 0.0f;
    }
}

/**
 * @brief Convert wheel rad/s targets to Robot_TwistType and send to Robot_Control
 * @details Uses differential drive forward kinematics:
 *   v_wheel (m/s) = omega (rad/s) * wheel_radius (m)
 *   linear_x = (v_left + v_right) / 2
 *   angular_z = (v_right - v_left) / wheel_base
 */
static void App_ApplyVelocityCommand(void)
{
    Robot_TwistType twist;
    float32 leftVel, rightVel;

    /* Convert wheel angular velocity (rad/s) to linear velocity (m/s) */
    leftVel = App_TargetLeftRadS * ROBOT_WHEEL_RADIUS_M;
    rightVel = App_TargetRightRadS * ROBOT_WHEEL_RADIUS_M;

    /* Convert wheel velocities to Twist (linear.x + angular.z) */
    /* v = (v_left + v_right) / 2 */
    /* w = (v_right - v_left) / L */
    twist.LinearX = (leftVel + rightVel) / 2.0f;
    twist.LinearY = 0.0f;
    twist.AngularZ = (rightVel - leftVel) / ROBOT_WHEEL_BASE_M;

    /* Apply via Robot_Control (clamping + state checks built in) */
    (void)Robot_SetVelocity(&twist);
}

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize control task
 */
void App_ControlTask_Init(void)
{
    /* Get queue handle */
    App_WheelCmdQueue = Tasks_GetWheelSpeedCmdQueue();

    App_TargetLeftRadS = 0.0f;
    App_TargetRightRadS = 0.0f;
    App_LastCmdTime = xTaskGetTickCount();

    App_ControlInitialized = TRUE;
}

/**
 * @brief Control task main function (called by FreeRTOS task @ 100Hz)
 * @details
 *   1. Read wheel commands from queue (from Comm Task)
 *   2. Convert to Twist and set Robot_Control velocity target
 *   3. Update encoders for Robot_Control
 *   4. Run Robot_Control PID loop (only if SafeState allows)
 */
void App_ControlTask_Run(void)
{
    if (App_ControlInitialized == FALSE)
    {
        App_ControlTask_Init();
    }

    /* 1. Check for new commands from Comm task */
    App_CheckCommandQueue();

    /* 2. Convert rad/s targets to Twist and set Robot_Control */
    App_ApplyVelocityCommand();

    /* 3. Update encoder data for Robot_Control */
    Encoder_UpdateAll();

    /* 4. Run PID control loop via Robot_Control */
    if (SafeState_IsMotorEnableAllowed())
    {
        Robot_UpdateControl();
    }
    else
    {
        /* Safety has disabled motors — ensure stopped */
        Robot_EmergencyStop();
    }
}

/**
 * @brief Set wheel speed targets directly (for internal use)
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
 * @brief Emergency stop — delegate to Robot_Control
 */
void App_ControlTask_EmergencyStop(void)
{
    App_TargetLeftRadS = 0.0f;
    App_TargetRightRadS = 0.0f;
    Robot_EmergencyStop();
}
