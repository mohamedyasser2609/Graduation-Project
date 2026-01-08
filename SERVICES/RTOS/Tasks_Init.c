/**
 * @file Tasks_Init.c
 * @brief FreeRTOS Task Initialization
 * @details Creates and configures all FreeRTOS tasks for the robot controller
 *
 * Task Architecture:
 * - Safety Task:  Highest priority, monitors watchdog and current limits
 * - Control Task: High priority, PID motor control loop
 * - Sensor Task:  Medium priority, reads IMU/GPS/Encoders
 * - Comm Task:    Lower priority, handles ROS2 communication
 * - Thermal Task: Lowest priority, manages cooling system
 *
 * Inter-Task Communication:
 * - Queue_WheelSpeedCmd: Comm -> Control (wheel speed targets)
 * - Queue_SensorFeedback: Sensor -> Comm (encoder/IMU/GPS feedback)
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.1.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "../../CONFIG/Std_Types.h"
#include "../../APP/App_SharedTypes.h"
#include "Tasks_Init.h"

/* ===================[Task Priorities]=================== */
#define TASK_PRIORITY_SAFETY        (4u)
#define TASK_PRIORITY_CONTROL       (3u)
#define TASK_PRIORITY_SENSOR        (2u)
#define TASK_PRIORITY_COMM          (1u)
#define TASK_PRIORITY_THERMAL       (1u)

/* ===================[Task Stack Sizes]=================== */
#define TASK_STACK_SAFETY           (128u)
#define TASK_STACK_CONTROL          (256u)
#define TASK_STACK_SENSOR           (256u)
#define TASK_STACK_COMM             (256u)
#define TASK_STACK_THERMAL          (128u)

/* ===================[Task Periods (ms)]=================== */
#define TASK_PERIOD_SAFETY_MS       (10u)
#define TASK_PERIOD_CONTROL_MS      (10u)
#define TASK_PERIOD_SENSOR_MS       (20u)
#define TASK_PERIOD_COMM_MS         (20u)
#define TASK_PERIOD_THERMAL_MS      (1000u)

/* ===================[Task Handles]=================== */
static TaskHandle_t TaskHandle_Safety = NULL;
static TaskHandle_t TaskHandle_Control = NULL;
static TaskHandle_t TaskHandle_Sensor = NULL;
static TaskHandle_t TaskHandle_Comm = NULL;
static TaskHandle_t TaskHandle_Thermal = NULL;

/* ===================[Inter-Task Queues]=================== */
static QueueHandle_t Queue_WheelSpeedCmd = NULL;    /**< Comm -> Control */
static QueueHandle_t Queue_SensorFeedback = NULL;   /**< Sensor -> Comm */

/* ===================[External Function Declarations]=================== */
extern void App_SafetyTask_Run(void);
extern void App_ControlTask_Run(void);
extern void App_SensorTask_Run(void);
extern void App_CommTask_Run(void);
extern void App_ThermalTask_Run(void);

/* ===================[Task Functions]=================== */

static void Task_Safety(void* pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_PERIOD_SAFETY_MS);
    
    (void)pvParameters;
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        App_SafetyTask_Run();
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

static void Task_Control(void* pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_PERIOD_CONTROL_MS);
    
    (void)pvParameters;
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        App_ControlTask_Run();
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

static void Task_Sensor(void* pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_PERIOD_SENSOR_MS);
    
    (void)pvParameters;
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        App_SensorTask_Run();
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

static void Task_Comm(void* pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_PERIOD_COMM_MS);
    
    (void)pvParameters;
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        App_CommTask_Run();
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

static void Task_Thermal(void* pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_PERIOD_THERMAL_MS);
    
    (void)pvParameters;
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        App_ThermalTask_Run();
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* ===================[Public Functions]=================== */

BaseType_t Tasks_CreateQueues(void)
{
    BaseType_t xResult = pdPASS;
    
    /* Wheel Speed Command Queue: Comm -> Control */
    Queue_WheelSpeedCmd = xQueueCreate(QUEUE_CMD_SIZE, sizeof(WheelSpeedCmdType));
    if (Queue_WheelSpeedCmd == NULL)
    {
        xResult = pdFAIL;
    }
    
    /* Sensor Feedback Queue: Sensor -> Comm */
    if (xResult == pdPASS)
    {
        Queue_SensorFeedback = xQueueCreate(QUEUE_FEEDBACK_SIZE, sizeof(SensorFeedbackType));
        if (Queue_SensorFeedback == NULL)
        {
            xResult = pdFAIL;
        }
    }
    
    return xResult;
}

BaseType_t Tasks_CreateAll(void)
{
    BaseType_t xResult = pdPASS;
    
    if (xTaskCreate(Task_Safety, "Safety", TASK_STACK_SAFETY, NULL, 
                    TASK_PRIORITY_SAFETY, &TaskHandle_Safety) != pdPASS)
    {
        xResult = pdFAIL;
    }
    
    if (xResult == pdPASS)
    {
        if (xTaskCreate(Task_Control, "Control", TASK_STACK_CONTROL, NULL,
                        TASK_PRIORITY_CONTROL, &TaskHandle_Control) != pdPASS)
        {
            xResult = pdFAIL;
        }
    }
    
    if (xResult == pdPASS)
    {
        if (xTaskCreate(Task_Sensor, "Sensor", TASK_STACK_SENSOR, NULL,
                        TASK_PRIORITY_SENSOR, &TaskHandle_Sensor) != pdPASS)
        {
            xResult = pdFAIL;
        }
    }
    
    if (xResult == pdPASS)
    {
        if (xTaskCreate(Task_Comm, "Comm", TASK_STACK_COMM, NULL,
                        TASK_PRIORITY_COMM, &TaskHandle_Comm) != pdPASS)
        {
            xResult = pdFAIL;
        }
    }
    
    if (xResult == pdPASS)
    {
        if (xTaskCreate(Task_Thermal, "Thermal", TASK_STACK_THERMAL, NULL,
                        TASK_PRIORITY_THERMAL, &TaskHandle_Thermal) != pdPASS)
        {
            xResult = pdFAIL;
        }
    }
    
    return xResult;
}

void Tasks_Init(void)
{
    if (Tasks_CreateQueues() != pdPASS)
    {
        for (;;) { }  /* Queue creation failed */
    }
    
    if (Tasks_CreateAll() != pdPASS)
    {
        for (;;) { }  /* Task creation failed */
    }
    
    vTaskStartScheduler();
    
    for (;;) { }  /* Should never reach here */
}

/* ===================[Queue Access Functions]=================== */

QueueHandle_t Tasks_GetWheelSpeedCmdQueue(void)
{
    return Queue_WheelSpeedCmd;
}

QueueHandle_t Tasks_GetSensorFeedbackQueue(void)
{
    return Queue_SensorFeedback;
}

/* ===================[Hook Functions]=================== */

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}
