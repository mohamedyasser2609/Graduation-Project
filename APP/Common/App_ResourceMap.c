/**
 * @file App_ResourceMap.c
 * @brief Hardware Resource Ownership Implementation
 * @details Manages mutexes and queues for inter-task resource sharing
 *
 * @author Mohamed Yasser
 * @date Jan 09, 2026
 * @version 1.0.0
 */

#include "App_ResourceMap.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"

/* ===================[Private Variables]=================== */
static boolean ResourceMap_Initialized = FALSE;

/* ===================[Mutex Handles]=================== */
SemaphoreHandle_t Mutex_Encoder = NULL;
SemaphoreHandle_t Mutex_I2C0 = NULL;

/* ===================[Queue Handles]=================== */
QueueHandle_t Queue_SafetyStatus = NULL;

/* ===================[Queue Sizes]=================== */
#define QUEUE_SAFETY_STATUS_SIZE    (1u)  /* MUST be 1 for xQueueOverwrite */

/* ===================[Resource Ownership Table]=================== */
static const ResourceOwnerType ResourceOwners[RESOURCE_COUNT] = {
    RESOURCE_OWNER_CONTROL,     /* RESOURCE_MOTOR_LEFT */
    RESOURCE_OWNER_CONTROL,     /* RESOURCE_MOTOR_RIGHT */
    RESOURCE_OWNER_SHARED_RO,   /* RESOURCE_ENCODER_LEFT */
    RESOURCE_OWNER_SHARED_RO,   /* RESOURCE_ENCODER_RIGHT */
    RESOURCE_OWNER_SENSOR,      /* RESOURCE_IMU */
    RESOURCE_OWNER_SENSOR,      /* RESOURCE_GPS */
    RESOURCE_OWNER_SAFETY,      /* RESOURCE_CURRENT_LEFT */
    RESOURCE_OWNER_SAFETY,      /* RESOURCE_CURRENT_RIGHT */
    RESOURCE_OWNER_THERMAL,     /* RESOURCE_TEMP_SENSOR_0 */
    RESOURCE_OWNER_THERMAL,     /* RESOURCE_TEMP_SENSOR_1 */
    RESOURCE_OWNER_THERMAL,     /* RESOURCE_TEMP_SENSOR_2 */
    RESOURCE_OWNER_THERMAL,     /* RESOURCE_FAN_0 */
    RESOURCE_OWNER_THERMAL,     /* RESOURCE_FAN_1 */
    RESOURCE_OWNER_SAFETY,      /* RESOURCE_WATCHDOG */
    RESOURCE_OWNER_COMM,        /* RESOURCE_ROS_UART */
    RESOURCE_OWNER_NONE         /* RESOURCE_DEBUG_UART - shared */
};

/* ===================[Public Functions]=================== */

Std_ReturnType ResourceMap_Init(void)
{
    if (ResourceMap_Initialized)
    {
        return E_OK;
    }
    
    /* Create encoder mutex (shared between Sensor and Control) */
    Mutex_Encoder = xSemaphoreCreateMutex();
    if (Mutex_Encoder == NULL)
    {
        return E_NOT_OK;
    }
    
    /* Create I2C0 mutex (if IMU shares bus) */
    Mutex_I2C0 = xSemaphoreCreateMutex();
    if (Mutex_I2C0 == NULL)
    {
        return E_NOT_OK;
    }
    
    /* Create Safety Status Queue (Safety -> Comm) */
    Queue_SafetyStatus = xQueueCreate(QUEUE_SAFETY_STATUS_SIZE, 
                                       sizeof(SafetyStatusMsgType));
    if (Queue_SafetyStatus == NULL)
    {
        return E_NOT_OK;
    }
    
    ResourceMap_Initialized = TRUE;
    return E_OK;
}

boolean ResourceMap_AcquireEncoder(uint32 TimeoutMs)
{
    TickType_t ticks;
    
    if (Mutex_Encoder == NULL)
    {
        return FALSE;
    }
    
    ticks = pdMS_TO_TICKS(TimeoutMs);
    
    if (xSemaphoreTake(Mutex_Encoder, ticks) == pdTRUE)
    {
        return TRUE;
    }
    
    return FALSE;
}

void ResourceMap_ReleaseEncoder(void)
{
    if (Mutex_Encoder != NULL)
    {
        xSemaphoreGive(Mutex_Encoder);
    }
}

boolean ResourceMap_IsOwner(ResourceType Resource)
{
    TaskHandle_t currentTask;
    const char* taskName;
    ResourceOwnerType owner;
    
    if (Resource >= RESOURCE_COUNT)
    {
        return FALSE;
    }
    
    owner = ResourceOwners[Resource];
    
    /* Shared resources can be accessed by anyone with mutex */
    if (owner == RESOURCE_OWNER_SHARED_RO)
    {
        return TRUE;
    }
    
    /* Get current task name */
    currentTask = xTaskGetCurrentTaskHandle();
    taskName = pcTaskGetName(currentTask);
    
    /* Check ownership based on task name */
    switch (owner)
    {
        case RESOURCE_OWNER_SAFETY:
            return (taskName[0] == 'S' && taskName[1] == 'a');  /* "Safety" */
            
        case RESOURCE_OWNER_CONTROL:
            return (taskName[0] == 'C' && taskName[1] == 'o');  /* "Control" */
            
        case RESOURCE_OWNER_SENSOR:
            return (taskName[0] == 'S' && taskName[1] == 'e');  /* "Sensor" */
            
        case RESOURCE_OWNER_COMM:
            return (taskName[0] == 'C' && taskName[1] == 'o' && 
                    taskName[2] == 'm');  /* "Comm" */
            
        case RESOURCE_OWNER_THERMAL:
            return (taskName[0] == 'T');  /* "Thermal" */
            
        default:
            return FALSE;
    }
}

Std_ReturnType ResourceMap_SendSafetyStatus(const SafetyStatusMsgType* Status)
{
    if ((Status == NULL) || (Queue_SafetyStatus == NULL))
    {
        return E_NOT_OK;
    }
    
    /* Use xQueueOverwrite to always have latest status */
    if (xQueueOverwrite(Queue_SafetyStatus, Status) == pdTRUE)
    {
        return E_OK;
    }
    
    return E_NOT_OK;
}

Std_ReturnType ResourceMap_ReceiveSafetyStatus(SafetyStatusMsgType* Status, 
                                                uint32 TimeoutMs)
{
    TickType_t ticks;
    
    if ((Status == NULL) || (Queue_SafetyStatus == NULL))
    {
        return E_NOT_OK;
    }
    
    ticks = pdMS_TO_TICKS(TimeoutMs);
    
    if (xQueueReceive(Queue_SafetyStatus, Status, ticks) == pdTRUE)
    {
        return E_OK;
    }
    
    return E_NOT_OK;
}
