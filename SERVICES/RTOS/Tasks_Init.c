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
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "../../CONFIG/Std_Types.h"

/* ===================[Task Priorities]=================== */
/**
 * Task priority definitions (higher number = higher priority)
 * Priority 0 is reserved for idle task
 */
#define TASK_PRIORITY_SAFETY        (4u)    /**< Highest - safety critical */
#define TASK_PRIORITY_CONTROL       (3u)    /**< High - motor control loop */
#define TASK_PRIORITY_SENSOR        (2u)    /**< Medium - sensor reading */
#define TASK_PRIORITY_COMM          (1u)    /**< Lower - communication */
#define TASK_PRIORITY_THERMAL       (1u)    /**< Lowest - thermal management */

/* ===================[Task Stack Sizes]=================== */
/**
 * Stack sizes in words (4 bytes each on ARM Cortex-M4)
 */
#define TASK_STACK_SAFETY           (128u)  /**< 512 bytes */
#define TASK_STACK_CONTROL          (256u)  /**< 1024 bytes */
#define TASK_STACK_SENSOR           (256u)  /**< 1024 bytes */
#define TASK_STACK_COMM             (256u)  /**< 1024 bytes */
#define TASK_STACK_THERMAL          (128u)  /**< 512 bytes */

/* ===================[Task Periods (ms)]=================== */
#define TASK_PERIOD_SAFETY_MS       (10u)   /**< 100 Hz */
#define TASK_PERIOD_CONTROL_MS      (10u)   /**< 100 Hz */
#define TASK_PERIOD_SENSOR_MS       (20u)   /**< 50 Hz */
#define TASK_PERIOD_COMM_MS         (50u)   /**< 20 Hz */
#define TASK_PERIOD_THERMAL_MS      (1000u) /**< 1 Hz */

/* ===================[Task Handles]=================== */
static TaskHandle_t TaskHandle_Safety = NULL;
static TaskHandle_t TaskHandle_Control = NULL;
static TaskHandle_t TaskHandle_Sensor = NULL;
static TaskHandle_t TaskHandle_Comm = NULL;
static TaskHandle_t TaskHandle_Thermal = NULL;

/* ===================[Shared Data Queues]=================== */
static QueueHandle_t Queue_MotorCommands = NULL;
static QueueHandle_t Queue_SensorData = NULL;

/* ===================[External Function Declarations]=================== */
/* These functions should be implemented in the APP layer */
extern void App_SafetyTask_Run(void);
extern void App_ControlTask_Run(void);
extern void App_SensorTask_Run(void);
extern void App_CommTask_Run(void);
extern void App_ThermalTask_Run(void);

/* ===================[Task Functions]=================== */

/**
 * @brief Safety Task - Highest priority
 * @details Monitors system health, feeds watchdog, handles emergency stops
 */
static void Task_Safety(void* pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_PERIOD_SAFETY_MS);
    
    (void)pvParameters;  /* Unused */
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Call the application safety task */
        App_SafetyTask_Run();
        
        /* Wait for next period */
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/**
 * @brief Control Task - Motor PID control
 * @details Runs PID loop for left and right motors
 */
static void Task_Control(void* pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_PERIOD_CONTROL_MS);
    
    (void)pvParameters;
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Call the application control task */
        App_ControlTask_Run();
        
        /* Wait for next period */
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/**
 * @brief Sensor Task - Read all sensors
 * @details Reads IMU, GPS, Encoders, and publishes to queues
 */
static void Task_Sensor(void* pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_PERIOD_SENSOR_MS);
    
    (void)pvParameters;
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Call the application sensor task */
        App_SensorTask_Run();
        
        /* Wait for next period */
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/**
 * @brief Communication Task - Handle ROS2 communication
 * @details Processes incoming commands, sends sensor data to ROS2
 */
static void Task_Comm(void* pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_PERIOD_COMM_MS);
    
    (void)pvParameters;
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Call the application communication task */
        App_CommTask_Run();
        
        /* Wait for next period */
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/**
 * @brief Thermal Task - Manage cooling system
 * @details Reads temperature sensors, controls fans
 */
static void Task_Thermal(void* pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_PERIOD_THERMAL_MS);
    
    (void)pvParameters;
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        /* Call the application thermal task */
        App_ThermalTask_Run();
        
        /* Wait for next period */
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* ===================[Public Functions]=================== */

/**
 * @brief Create all application tasks
 * @return pdPASS if all tasks created, pdFAIL otherwise
 */
BaseType_t Tasks_CreateAll(void)
{
    BaseType_t xResult = pdPASS;
    
    /* Create Safety Task - Highest Priority */
    if (xTaskCreate(Task_Safety, 
                    "Safety",
                    TASK_STACK_SAFETY,
                    NULL,
                    TASK_PRIORITY_SAFETY,
                    &TaskHandle_Safety) != pdPASS)
    {
        xResult = pdFAIL;
    }
    
    /* Create Control Task */
    if (xResult == pdPASS)
    {
        if (xTaskCreate(Task_Control,
                        "Control",
                        TASK_STACK_CONTROL,
                        NULL,
                        TASK_PRIORITY_CONTROL,
                        &TaskHandle_Control) != pdPASS)
        {
            xResult = pdFAIL;
        }
    }
    
    /* Create Sensor Task */
    if (xResult == pdPASS)
    {
        if (xTaskCreate(Task_Sensor,
                        "Sensor",
                        TASK_STACK_SENSOR,
                        NULL,
                        TASK_PRIORITY_SENSOR,
                        &TaskHandle_Sensor) != pdPASS)
        {
            xResult = pdFAIL;
        }
    }
    
    /* Create Communication Task */
    if (xResult == pdPASS)
    {
        if (xTaskCreate(Task_Comm,
                        "Comm",
                        TASK_STACK_COMM,
                        NULL,
                        TASK_PRIORITY_COMM,
                        &TaskHandle_Comm) != pdPASS)
        {
            xResult = pdFAIL;
        }
    }
    
    /* Create Thermal Task */
    if (xResult == pdPASS)
    {
        if (xTaskCreate(Task_Thermal,
                        "Thermal",
                        TASK_STACK_THERMAL,
                        NULL,
                        TASK_PRIORITY_THERMAL,
                        &TaskHandle_Thermal) != pdPASS)
        {
            xResult = pdFAIL;
        }
    }
    
    return xResult;
}

/**
 * @brief Create inter-task communication queues
 * @return pdPASS if all queues created, pdFAIL otherwise
 */
BaseType_t Tasks_CreateQueues(void)
{
    BaseType_t xResult = pdPASS;
    
    /* Motor command queue (from Comm to Control) */
    Queue_MotorCommands = xQueueCreate(4u, sizeof(uint32));
    if (Queue_MotorCommands == NULL)
    {
        xResult = pdFAIL;
    }
    
    /* Sensor data queue (from Sensor to Comm) */
    if (xResult == pdPASS)
    {
        Queue_SensorData = xQueueCreate(4u, sizeof(uint32));
        if (Queue_SensorData == NULL)
        {
            xResult = pdFAIL;
        }
    }
    
    return xResult;
}

/**
 * @brief Initialize all tasks and start the scheduler
 * @return Never returns if successful
 */
void Tasks_Init(void)
{
    /* Create inter-task queues */
    if (Tasks_CreateQueues() != pdPASS)
    {
        /* Queue creation failed - halt */
        for (;;) { }
    }
    
    /* Create all application tasks */
    if (Tasks_CreateAll() != pdPASS)
    {
        /* Task creation failed - halt */
        for (;;) { }
    }
    
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();
    
    /* Should never reach here - scheduler takes over */
    for (;;) { }
}

/**
 * @brief Get handle for a specific task
 * @param[in] taskName Name identifier
 * @return Task handle or NULL if not found
 */
TaskHandle_t Tasks_GetHandle(const char* taskName)
{
    TaskHandle_t handle = NULL;
    
    if (taskName != NULL)
    {
        if (taskName[0] == 'S' && taskName[1] == 'a')
        {
            handle = TaskHandle_Safety;
        }
        else if (taskName[0] == 'C' && taskName[1] == 'o')
        {
            handle = TaskHandle_Control;
        }
        else if (taskName[0] == 'S' && taskName[1] == 'e')
        {
            handle = TaskHandle_Sensor;
        }
        else if (taskName[0] == 'C' && taskName[1] == 'm')
        {
            handle = TaskHandle_Comm;
        }
        else if (taskName[0] == 'T' && taskName[1] == 'h')
        {
            handle = TaskHandle_Thermal;
        }
        else
        {
            /* Unknown task */
        }
    }
    
    return handle;
}

/**
 * @brief Get the motor command queue handle
 * @return Queue handle
 */
QueueHandle_t Tasks_GetMotorCommandQueue(void)
{
    return Queue_MotorCommands;
}

/**
 * @brief Get the sensor data queue handle
 * @return Queue handle
 */
QueueHandle_t Tasks_GetSensorDataQueue(void)
{
    return Queue_SensorData;
}

/**
 * @brief Stack overflow hook (called by FreeRTOS on stack overflow)
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    
    /* Stack overflow - halt system */
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}

/**
 * @brief Malloc failed hook (called by FreeRTOS when heap allocation fails)
 */
void vApplicationMallocFailedHook(void)
{
    /* Heap exhausted - halt system */
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}
