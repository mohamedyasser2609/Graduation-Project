/**
 * @file Tasks_Init.h
 * @brief FreeRTOS Task Initialization Header
 * @details Public interface for task initialization and queue access
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.1.0
 */

#ifndef TASKS_INIT_H
#define TASKS_INIT_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize all tasks and start the scheduler
 * @note This function never returns
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
 * @brief Get the wheel speed command queue handle
 * @return Queue handle for wheel speed commands (Comm -> Control)
 */
QueueHandle_t Tasks_GetWheelSpeedCmdQueue(void);

/**
 * @brief Get the sensor feedback queue handle
 * @return Queue handle for sensor feedback (Sensor -> Comm)
 */
QueueHandle_t Tasks_GetSensorFeedbackQueue(void);

#endif /* TASKS_INIT_H */
