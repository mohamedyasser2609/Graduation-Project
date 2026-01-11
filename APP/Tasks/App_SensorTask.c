/**
 * @file App_SensorTask.c
 * @brief Sensor Reading Task Implementation
 * @details Reads sensors and publishes feedback to Comm task via queue
 *
 * Publishes: SensorFeedbackType to Comm Task via queue
 * Format:    Encoder ticks, IMU yaw, GPS coordinates
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.1.0
 */

#include "../../CONFIG/Std_Types.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "queue.h"

/* Driver includes */
#include "../../ECUAL/IMU/IMU.h"
#include "../../ECUAL/GPS/GPS.h"
#include "../../ECUAL/ENCODER/Encoder.h"
#include "../../ECUAL/CURRENT_SENSOR/ACS712.h"
#include "../../ECUAL/TEMP_SENSOR/AM2320.h"
#include "../../SERVICES/RTOS/Tasks_Init.h"

/* Shared types */
#include "../Common/App_SharedTypes.h"

/* ===================[Private Variables]=================== */
static boolean App_SensorInitialized = FALSE;

/* Cached sensor data */
static IMU_CalibratedDataType App_ImuData;
static GPS_DataType App_GpsData;
static Encoder_DataType App_EncoderData[2];
static ACS712_DataType App_MotorCurrent[2];

/* Queue handle */
static QueueHandle_t App_FeedbackQueue = NULL;

/* Feedback publish counter (publish every N calls) */
#define FEEDBACK_PUBLISH_RATE   (2u)    /* Every 40ms at 20ms task period */
static uint8 App_FeedbackCounter = 0u;

/* ===================[Private Functions]=================== */

/**
 * @brief Calculate yaw from IMU gyroscope (simple integration)
 * @note In real application, use proper sensor fusion (Madgwick, Kalman)
 */
static float32 App_GetYawDegrees(void)
{
    static float32 yawAccumulator = 0.0f;
    
    /* Simple integration of gyro Z (deg/s * dt) */
    /* Sample time = 20ms = 0.02s */
    yawAccumulator += App_ImuData.gyro.z * 0.02f;
    
    /* Normalize to 0-360 */
    while (yawAccumulator >= 360.0f) yawAccumulator -= 360.0f;
    while (yawAccumulator < 0.0f) yawAccumulator += 360.0f;
    
    return yawAccumulator;
}

/**
 * @brief Publish sensor feedback to queue
 */
static void App_PublishFeedback(void)
{
    SensorFeedbackType feedback;
    
    if (App_FeedbackQueue == NULL)
    {
        return;
    }
    
    /* Fill feedback structure */
    feedback.LeftEncoderTicks = (sint32)App_EncoderData[0].PositionCounts;
    feedback.RightEncoderTicks = (sint32)App_EncoderData[1].PositionCounts;
    feedback.YawDegrees = App_GetYawDegrees();
    feedback.Latitude = App_GpsData.position.latitude;
    feedback.Longitude = App_GpsData.position.longitude;
    feedback.Timestamp = xTaskGetTickCount();
    feedback.Valid = TRUE;
    
    /* Send to queue (overwrite if full - always send latest) */
    (void)xQueueOverwrite(App_FeedbackQueue, &feedback);
}

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize sensor task
 */
void App_SensorTask_Init(void)
{
    /* Get queue handle */
    App_FeedbackQueue = Tasks_GetSensorFeedbackQueue();
    
    App_FeedbackCounter = 0u;
    App_SensorInitialized = TRUE;
}

/**
 * @brief Sensor task main function (called by FreeRTOS task)
 */
void App_SensorTask_Run(void)
{
    if (App_SensorInitialized == FALSE)
    {
        App_SensorTask_Init();
    }
    
    /* 1. Read IMU data */
    (void)IMU_ReadCalibratedData(&App_ImuData);
    
    /* 2. Read GPS data */
    (void)GPS_GetData(&App_GpsData);
    
    /* 3. Read encoder data */
    Encoder_UpdateAll();  /* Update velocity calculations */
    (void)Encoder_GetData(ENCODER_CHANNEL_LEFT, &App_EncoderData[0]);
    (void)Encoder_GetData(ENCODER_CHANNEL_RIGHT, &App_EncoderData[1]);
    
    /* 4. Read motor current */
    (void)ACS712_ReadCurrent(0u, &App_MotorCurrent[0]);
    (void)ACS712_ReadCurrent(1u, &App_MotorCurrent[1]);
    
    /* 5. Publish feedback at reduced rate */
    App_FeedbackCounter++;
    if (App_FeedbackCounter >= FEEDBACK_PUBLISH_RATE)
    {
        App_FeedbackCounter = 0u;
        App_PublishFeedback();
    }
}

/**
 * @brief Get IMU data
 */
Std_ReturnType App_SensorTask_GetImuData(IMU_CalibratedDataType* DataPtr)
{
    if (DataPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    *DataPtr = App_ImuData;
    return E_OK;
}

/**
 * @brief Get GPS data
 */
Std_ReturnType App_SensorTask_GetGpsData(GPS_DataType* DataPtr)
{
    if (DataPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    *DataPtr = App_GpsData;
    return E_OK;
}

/**
 * @brief Get encoder data
 */
Std_ReturnType App_SensorTask_GetEncoderData(uint8 Channel, Encoder_DataType* DataPtr)
{
    if ((DataPtr == NULL_PTR) || (Channel > 1u))
    {
        return E_NOT_OK;
    }
    
    *DataPtr = App_EncoderData[Channel];
    return E_OK;
}

/**
 * @brief Get motor current
 */
Std_ReturnType App_SensorTask_GetMotorCurrent(uint8 Channel, ACS712_DataType* CurrentPtr)
{
    if ((CurrentPtr == NULL_PTR) || (Channel > 1u))
    {
        return E_NOT_OK;
    }
    
    *CurrentPtr = App_MotorCurrent[Channel];
    return E_OK;
}
