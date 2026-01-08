/**
 * @file Tasks_Init.h
 * @brief FreeRTOS Task Initialization Header
 * @details Public interface for task initialization and management
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#ifndef TASKS_INIT_H
#define TASKS_INIT_H

/* ===================[Includes]=================== */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize all tasks and start the scheduler
 * @details Creates queues, tasks, and starts FreeRTOS scheduler
 * @return Never returns if successful
 */
void Tasks_Init(void);

/**
 * @brief Create all application tasks
 * @return pdPASS if all tasks created, pdFAIL otherwise
 */
BaseType_t Tasks_CreateAll(void);

/**
 * @brief Create inter-task communication queues
 * @return pdPASS if all queues created, pdFAIL otherwise
 */
BaseType_t Tasks_CreateQueues(void);

/**
 * @brief Get handle for a specific task
 * @param[in] taskName Name identifier ("Safety", "Control", etc.)
 * @return Task handle or NULL if not found
 */
TaskHandle_t Tasks_GetHandle(const char* taskName);

/**
 * @brief Get the motor command queue handle
 * @return Queue handle for motor commands
 */
QueueHandle_t Tasks_GetMotorCommandQueue(void);

/**
 * @brief Get the sensor data queue handle
 * @return Queue handle for sensor data
 */
QueueHandle_t Tasks_GetSensorDataQueue(void);

#endif /* TASKS_INIT_H */
