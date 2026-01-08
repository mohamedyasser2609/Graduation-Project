/**
 * @file App_SensorTask.c
 * @brief Sensor Reading Task Implementation
 * @details Reads all sensors and publishes data for other tasks
 *
 * Sensors:
 * - IMU (MPU-9250): Accelerometer, Gyroscope, Magnetometer
 * - GPS: Position, velocity
 * - Encoders: Wheel ticks and velocity
 * - Current sensors: Motor current
 * - Temperature sensors: System temperatures
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "../CONFIG/Std_Types.h"

/* Driver includes */
#include "../ECUAL/IMU/IMU.h"
#include "../ECUAL/GPS/GPS.h"
#include "../ECUAL/ENCODER/Encoder.h"
#include "../ECUAL/CURRENT_SENSOR/ACS712.h"
#include "../ECUAL/TEMP_SENSOR/AM2320.h"

/* ===================[External Configurations]=================== */
extern const IMU_ConfigType IMU_Config;
extern const Encoder_ConfigType Encoder_Config;
extern const ACS712_ConfigType ACS712_Config;
extern const AM2320_ConfigType AM2320_Config;

/* ===================[Public Sensor Data]=================== */
/* These can be read by other tasks */
static IMU_ProcessedDataType App_ImuData;
static GPS_DataType App_GpsData;
static Encoder_DataType App_EncoderData[2];
static float32 App_MotorCurrent[2];
static AM2320_DataType App_TempData[3];

static boolean App_SensorInitialized = FALSE;

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize sensor task
 */
void App_SensorTask_Init(void)
{
    /* Sensors should already be initialized by System_Init */
    App_SensorInitialized = TRUE;
}

/**
 * @brief Sensor task main function (called by FreeRTOS task)
 */
void App_SensorTask_Run(void)
{
    if (!App_SensorInitialized)
    {
        App_SensorTask_Init();
    }
    
    /* 1. Read IMU data */
    (void)IMU_ReadProcessedData(&App_ImuData);
    
    /* 2. Read GPS data */
    (void)GPS_GetData(&App_GpsData);
    
    /* 3. Read encoder data */
    (void)Encoder_GetData(0u, &App_EncoderData[0]);
    (void)Encoder_GetData(1u, &App_EncoderData[1]);
    
    /* 4. Read motor current */
    (void)ACS712_ReadCurrent(0u, &App_MotorCurrent[0]);
    (void)ACS712_ReadCurrent(1u, &App_MotorCurrent[1]);
    
    /* 5. Read temperature sensors (less frequent - every 5th call) */
    static uint8 tempReadCounter = 0u;
    tempReadCounter++;
    if (tempReadCounter >= 5u)
    {
        tempReadCounter = 0u;
        (void)AM2320_ReadAllSensors(App_TempData);
    }
}

/**
 * @brief Get IMU data
 * @param[out] DataPtr Pointer to store IMU data
 * @return E_OK on success
 */
Std_ReturnType App_SensorTask_GetImuData(IMU_ProcessedDataType* DataPtr)
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
 * @param[out] DataPtr Pointer to store GPS data
 * @return E_OK on success
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
 * @param[in] Channel Encoder channel (0=left, 1=right)
 * @param[out] DataPtr Pointer to store encoder data
 * @return E_OK on success
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
 * @param[in] Channel Motor channel (0=left, 1=right)
 * @param[out] CurrentPtr Pointer to store current (Amps)
 * @return E_OK on success
 */
Std_ReturnType App_SensorTask_GetMotorCurrent(uint8 Channel, float32* CurrentPtr)
{
    if ((CurrentPtr == NULL_PTR) || (Channel > 1u))
    {
        return E_NOT_OK;
    }
    
    *CurrentPtr = App_MotorCurrent[Channel];
    return E_OK;
}
