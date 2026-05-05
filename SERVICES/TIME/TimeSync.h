/**
 * @file TimeSync.h
 * @brief System Time Synchronization Service
 * @details Manages the translation between Tiva system ticks and ROS2 epoch time.
 */

#ifndef SERVICES_TIME_TIMESYNC_H_
#define SERVICES_TIME_TIMESYNC_H_

#include "../../CONFIG/Std_Types.h"

/**
 * @brief Structure to hold synchronized time
 */
typedef struct {
    uint32 Seconds;
    uint32 Nanoseconds;
} TimeSync_TimeType;

/**
 * @brief Initialize the time synchronization service
 */
void TimeSync_Init(void);

/**
 * @brief Synchronize system time with an external reference (ROS2)
 * @param Seconds Seconds since Unix Epoch
 * @param Nanoseconds Nanoseconds part of the timestamp
 */
void TimeSync_Update(uint32 Seconds, uint32 Nanoseconds);

/**
 * @brief Get the current synchronized time
 * @param TimePtr Pointer to structure to fill with current time
 */
void TimeSync_GetTime(TimeSync_TimeType* TimePtr);

/**
 * @brief Check if time has been synchronized
 * @return TRUE if synchronized
 */
boolean TimeSync_IsSynchronized(void);

#endif /* SERVICES_TIME_TIMESYNC_H_ */
