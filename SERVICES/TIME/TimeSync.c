/**
 * @file TimeSync.c
 * @brief System Time Synchronization Service Implementation
 */

#include "TimeSync.h"
#include "FreeRTOS.h"
#include "task.h"

/* ===================[Private Variables]=================== */
static boolean TimeSync_Initialized = FALSE;
static boolean TimeSync_Synchronized = FALSE;

/* The offset between Tiva ticks and ROS wall time */
static uint64_t TimeSync_OffsetNs = 0u;

/* ===================[Public Functions]=================== */

void TimeSync_Init(void)
{
    TimeSync_OffsetNs = 0u;
    TimeSync_Synchronized = FALSE;
    TimeSync_Initialized = TRUE;
}

void TimeSync_Update(uint32 Seconds, uint32 Nanoseconds)
{
    if (TimeSync_Initialized == FALSE)
    {
        return;
    }

    /* Convert incoming ROS time to total nanoseconds */
    uint64_t rosNowNs = ((uint64_t)Seconds * 1000000000ULL) + (uint64_t)Nanoseconds;
    
    /* Get current Tiva time in nanoseconds since boot */
    /* Dynamic scaling: xTaskGetTickCount() * portTICK_PERIOD_MS (10ms for 100Hz) * 1,000,000 ns */
    uint64_t tivaNowNs = (uint64_t)xTaskGetTickCount() * (uint64_t)portTICK_PERIOD_MS * 1000000ULL;
    
    /* Calculate the fixed offset */
    TimeSync_OffsetNs = rosNowNs - tivaNowNs;
    
    TimeSync_Synchronized = TRUE;
}

void TimeSync_GetTime(TimeSync_TimeType* TimePtr)
{
    if ((TimePtr == NULL_PTR) || (TimeSync_Initialized == FALSE))
    {
        return;
    }

    /* Calculate current wall time: TivaNow + Offset */
    uint64_t tivaNowNs = (uint64_t)xTaskGetTickCount() * (uint64_t)portTICK_PERIOD_MS * 1000000ULL;
    uint64_t wallNowNs = tivaNowNs + TimeSync_OffsetNs;
    
    TimePtr->Seconds = (uint32)(wallNowNs / 1000000000ULL);
    TimePtr->Nanoseconds = (uint32)(wallNowNs % 1000000000ULL);
}

boolean TimeSync_IsSynchronized(void)
{
    return TimeSync_Synchronized;
}
