/**
 * @file AM2320.c
 * @brief AM2320 Temperature & Humidity Sensor Driver Implementation
 * @details AUTOSAR-compliant driver for ASAIR AM2320 sensor
 *
 * AM2320 Communication Protocol:
 * 1. Wake up sensor by sending start condition + address (no ACK expected)
 * 2. Wait >800us for sensor to wake
 * 3. Send read command: 0x03 (function code), 0x00 (start addr), 0x04 (length)
 * 4. Wait >1.5ms for measurement
 * 5. Read 8 bytes: func, len, hum_h, hum_l, temp_h, temp_l, crc_l, crc_h
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#include "AM2320.h"
#include "../../MCAL/I2C/I2C.h"

#if (AM2320_DEV_ERROR_DETECT == STD_ON)
#include "../../CONFIG/Det.h"
#endif

/* ===================[Private Macros]=================== */
#define AM2320_FUNC_READ                (0x03u)
#define AM2320_REG_HUM_HIGH             (0x00u)
#define AM2320_REG_TEMP_HIGH            (0x02u)
#define AM2320_REG_COUNT                (0x04u)

#define AM2320_RESPONSE_LENGTH          (8u)
#define AM2320_COMMAND_LENGTH           (3u)

/* CRC-16 Modbus polynomial */
#define AM2320_CRC16_POLY               (0xA001u)

/* ===================[Private Types]=================== */
typedef struct
{
    AM2320_DataType         LastReading;    /**< Last successful reading */
    AM2320_SensorStatusType Status;         /**< Sensor status */
    uint8                   RetryCount;     /**< Current retry count */
} AM2320_SensorStateType;

/* ===================[Private Variables]=================== */
static const AM2320_ConfigType* AM2320_ConfigPtr = NULL_PTR;
static AM2320_StatusType AM2320_ModuleStatus = AM2320_STATUS_UNINIT;
static AM2320_SensorStateType AM2320_SensorStates[AM2320_MAX_SENSORS];

#if (AM2320_ALARM_CALLBACK_API == STD_ON)
static AM2320_AlarmCallbackType AM2320_AlarmCallback = NULL_PTR;
#endif

/* ===================[Private Function Declarations]=================== */
static boolean AM2320_IsSensorValid(AM2320_SensorType Sensor);
static uint16 AM2320_CalculateCRC16(const uint8* Data, uint8 Length);
static void AM2320_DelayUs(uint32 Microseconds);
static Std_ReturnType AM2320_WakeupSensor(uint8 I2cModule);
static Std_ReturnType AM2320_SendReadCommand(uint8 I2cModule);
static Std_ReturnType AM2320_ReadResponse(uint8 I2cModule, uint8* Buffer);
static void AM2320_ParseData(const uint8* Buffer, AM2320_DataType* DataPtr);
static void AM2320_CheckAlarm(AM2320_SensorType Sensor, float32 Temperature,
                               const AM2320_SensorConfigType* SensorCfg);

/* ===================[Private Function Implementations]=================== */

/**
 * @brief Check if sensor ID is valid
 */
static boolean AM2320_IsSensorValid(AM2320_SensorType Sensor)
{
    boolean isValid = FALSE;
    
    if (AM2320_ConfigPtr != NULL_PTR)
    {
        if (Sensor < AM2320_ConfigPtr->NumSensors)
        {
            isValid = TRUE;
        }
    }
    
    return isValid;
}

/**
 * @brief Calculate CRC-16 Modbus checksum
 */
static uint16 AM2320_CalculateCRC16(const uint8* Data, uint8 Length)
{
    uint16 crc = 0xFFFFu;
    uint8 i;
    uint8 j;
    
    for (i = 0u; i < Length; i++)
    {
        crc ^= (uint16)Data[i];
        
        for (j = 0u; j < 8u; j++)
        {
            if ((crc & 0x0001u) != 0u)
            {
                crc = (crc >> 1u) ^ AM2320_CRC16_POLY;
            }
            else
            {
                crc >>= 1u;
            }
        }
    }
    
    return crc;
}

/**
 * @brief Simple delay in microseconds
 */
static void AM2320_DelayUs(uint32 Microseconds)
{
    volatile uint32 count;
    /* Approximate delay for 80MHz clock: ~20 cycles per iteration */
    const uint32 cyclesPerUs = 4u;  /* Adjust based on optimization */
    
    for (count = 0u; count < (Microseconds * cyclesPerUs); count++)
    {
        /* Empty loop for delay */
    }
}

/**
 * @brief Wake up the sensor from sleep mode
 */
static Std_ReturnType AM2320_WakeupSensor(uint8 I2cModule)
{
    Std_ReturnType status;
    
    /* Send wake-up signal: I2C start + address, sensor won't ACK */
    status = I2c_WriteData(I2cModule, AM2320_I2C_ADDRESS, NULL_PTR, 0u);
    
    /* Ignore NACK - sensor is waking up */
    (void)status;
    
    /* Wait for sensor to wake up */
    AM2320_DelayUs(AM2320_WAKEUP_DELAY_US);
    
    return E_OK;
}

/**
 * @brief Send read command to sensor
 */
static Std_ReturnType AM2320_SendReadCommand(uint8 I2cModule)
{
    uint8 command[AM2320_COMMAND_LENGTH];
    Std_ReturnType status;
    
    command[0] = AM2320_FUNC_READ;      /* Read function code */
    command[1] = AM2320_REG_HUM_HIGH;   /* Start at humidity register */
    command[2] = AM2320_REG_COUNT;      /* Read 4 registers */
    
    status = I2c_WriteData(I2cModule, AM2320_I2C_ADDRESS, command, AM2320_COMMAND_LENGTH);
    
    if (status == E_OK)
    {
        /* Wait for measurement to complete */
        AM2320_DelayUs(AM2320_MEASURE_DELAY_US);
    }
    
    return status;
}

/**
 * @brief Read response from sensor
 */
static Std_ReturnType AM2320_ReadResponse(uint8 I2cModule, uint8* Buffer)
{
    Std_ReturnType status;
    
    status = I2c_ReadData(I2cModule, AM2320_I2C_ADDRESS, Buffer, AM2320_RESPONSE_LENGTH);
    
    return status;
}

/**
 * @brief Parse raw data and calculate temperature/humidity
 */
static void AM2320_ParseData(const uint8* Buffer, AM2320_DataType* DataPtr)
{
    uint16 rawHumidity;
    uint16 rawTemperature;
    
    /* Extract raw values (big-endian) */
    rawHumidity = ((uint16)Buffer[2] << 8u) | (uint16)Buffer[3];
    rawTemperature = ((uint16)Buffer[4] << 8u) | (uint16)Buffer[5];
    
    DataPtr->RawHumidity = rawHumidity;
    DataPtr->RawTemperature = rawTemperature;
    
    /* Convert humidity: raw / 10 = % RH */
    DataPtr->Humidity = (float32)rawHumidity / 10.0f;
    
    /* Convert temperature: raw / 10 = °C */
    /* Bit 15 indicates negative temperature */
    if ((rawTemperature & 0x8000u) != 0u)
    {
        /* Negative temperature */
        DataPtr->TemperatureC = -((float32)(rawTemperature & 0x7FFFu) / 10.0f);
    }
    else
    {
        /* Positive temperature */
        DataPtr->TemperatureC = (float32)rawTemperature / 10.0f;
    }
}

/**
 * @brief Check temperature against thresholds and trigger alarm
 */
static void AM2320_CheckAlarm(AM2320_SensorType Sensor, float32 Temperature,
                               const AM2320_SensorConfigType* SensorCfg)
{
    #if (AM2320_ALARM_CALLBACK_API == STD_ON)
    if (AM2320_AlarmCallback != NULL_PTR)
    {
        if (Temperature > SensorCfg->TempHighThreshold)
        {
            AM2320_AlarmCallback(Sensor, Temperature, TRUE);
        }
        else if (Temperature < SensorCfg->TempLowThreshold)
        {
            AM2320_AlarmCallback(Sensor, Temperature, FALSE);
        }
        else
        {
            /* Temperature within normal range */
        }
    }
    #else
    (void)Sensor;
    (void)Temperature;
    (void)SensorCfg;
    #endif
}

/* ===================[Public Function Implementations]=================== */

/**
 * @brief Initialize the AM2320 driver
 */
void AM2320_Init(const AM2320_ConfigType* ConfigPtr)
{
    uint8 sensorIdx;
    
#if (AM2320_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_INIT_SID, AM2320_E_PARAM_POINTER);
        return;
    }
    
    if (AM2320_ModuleStatus != AM2320_STATUS_UNINIT)
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_INIT_SID, AM2320_E_ALREADY_INIT);
        return;
    }
    
    if (ConfigPtr->NumSensors > AM2320_MAX_SENSORS)
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_INIT_SID, AM2320_E_PARAM_CONFIG);
        return;
    }
    
    if (ConfigPtr->Sensors == NULL_PTR)
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_INIT_SID, AM2320_E_PARAM_POINTER);
        return;
    }
#endif
    
    /* Store configuration pointer */
    AM2320_ConfigPtr = ConfigPtr;
    
    /* Initialize sensor states */
    for (sensorIdx = 0u; sensorIdx < ConfigPtr->NumSensors; sensorIdx++)
    {
        AM2320_SensorStates[sensorIdx].LastReading.TemperatureC = 0.0f;
        AM2320_SensorStates[sensorIdx].LastReading.Humidity = 0.0f;
        AM2320_SensorStates[sensorIdx].LastReading.RawTemperature = 0u;
        AM2320_SensorStates[sensorIdx].LastReading.RawHumidity = 0u;
        AM2320_SensorStates[sensorIdx].LastReading.Status = AM2320_SENSOR_OK;
        AM2320_SensorStates[sensorIdx].Status = AM2320_SENSOR_OK;
        AM2320_SensorStates[sensorIdx].RetryCount = 0u;
    }
    
    /* Set module status to idle */
    AM2320_ModuleStatus = AM2320_STATUS_IDLE;
}

#if (AM2320_DEINIT_API == STD_ON)
/**
 * @brief De-initialize the AM2320 driver
 */
void AM2320_DeInit(void)
{
    uint8 sensorIdx;
    
#if (AM2320_DEV_ERROR_DETECT == STD_ON)
    if (AM2320_ModuleStatus == AM2320_STATUS_UNINIT)
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_DEINIT_SID, AM2320_E_UNINIT);
        return;
    }
#endif
    
    /* Reset all sensor states */
    for (sensorIdx = 0u; sensorIdx < AM2320_MAX_SENSORS; sensorIdx++)
    {
        AM2320_SensorStates[sensorIdx].LastReading.TemperatureC = 0.0f;
        AM2320_SensorStates[sensorIdx].LastReading.Humidity = 0.0f;
        AM2320_SensorStates[sensorIdx].Status = AM2320_SENSOR_OK;
    }
    
    #if (AM2320_ALARM_CALLBACK_API == STD_ON)
    AM2320_AlarmCallback = NULL_PTR;
    #endif
    
    AM2320_ConfigPtr = NULL_PTR;
    AM2320_ModuleStatus = AM2320_STATUS_UNINIT;
}
#endif

/**
 * @brief Read temperature and humidity from sensor
 */
Std_ReturnType AM2320_Read(AM2320_SensorType Sensor, AM2320_DataType* DataPtr)
{
    Std_ReturnType status = E_NOT_OK;
    uint8 response[AM2320_RESPONSE_LENGTH];
    uint16 receivedCrc;
    uint16 calculatedCrc;
    uint8 retryCount;
    const AM2320_SensorConfigType* sensorCfg;
    
#if (AM2320_DEV_ERROR_DETECT == STD_ON)
    if (AM2320_ModuleStatus == AM2320_STATUS_UNINIT)
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_READ_SID, AM2320_E_UNINIT);
        return E_NOT_OK;
    }
    
    if (DataPtr == NULL_PTR)
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_READ_SID, AM2320_E_PARAM_POINTER);
        return E_NOT_OK;
    }
    
    if (!AM2320_IsSensorValid(Sensor))
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_READ_SID, AM2320_E_PARAM_SENSOR);
        return E_NOT_OK;
    }
#endif
    
    AM2320_ModuleStatus = AM2320_STATUS_BUSY;
    sensorCfg = &AM2320_ConfigPtr->Sensors[Sensor];
    
    /* Retry loop for communication errors */
    for (retryCount = 0u; retryCount < AM2320_RETRY_COUNT; retryCount++)
    {
        /* Step 1: Wake up sensor */
        (void)AM2320_WakeupSensor(sensorCfg->I2cModule);
        
        /* Step 2: Send read command */
        if (AM2320_SendReadCommand(sensorCfg->I2cModule) != E_OK)
        {
            AM2320_SensorStates[Sensor].Status = AM2320_SENSOR_COMM_ERROR;
            continue;  /* Retry */
        }
        
        /* Step 3: Read response */
        if (AM2320_ReadResponse(sensorCfg->I2cModule, response) != E_OK)
        {
            AM2320_SensorStates[Sensor].Status = AM2320_SENSOR_COMM_ERROR;
            continue;  /* Retry */
        }
        
        /* Step 4: Verify CRC */
        calculatedCrc = AM2320_CalculateCRC16(response, 6u);
        receivedCrc = ((uint16)response[7] << 8u) | (uint16)response[6];
        
        if (calculatedCrc != receivedCrc)
        {
            AM2320_SensorStates[Sensor].Status = AM2320_SENSOR_CRC_ERROR;
            continue;  /* Retry */
        }
        
        /* Step 5: Parse data */
        AM2320_ParseData(response, DataPtr);
        
        /* Apply calibration offsets */
        DataPtr->TemperatureC += sensorCfg->TempOffsetC;
        DataPtr->Humidity += sensorCfg->HumidityOffset;
        
        /* Clamp humidity to valid range */
        if (DataPtr->Humidity < 0.0f)
        {
            DataPtr->Humidity = 0.0f;
        }
        else if (DataPtr->Humidity > 100.0f)
        {
            DataPtr->Humidity = 100.0f;
        }
        else
        {
            /* Humidity in valid range */
        }
        
        DataPtr->Status = AM2320_SENSOR_OK;
        AM2320_SensorStates[Sensor].Status = AM2320_SENSOR_OK;
        AM2320_SensorStates[Sensor].LastReading = *DataPtr;
        
        /* Check temperature alarms */
        AM2320_CheckAlarm(Sensor, DataPtr->TemperatureC, sensorCfg);
        
        AM2320_ModuleStatus = AM2320_STATUS_OK;
        status = E_OK;
        break;  /* Success, exit retry loop */
    }
    
    if (status != E_OK)
    {
        /* Failed after retries */
        DataPtr->Status = AM2320_SensorStates[Sensor].Status;
        AM2320_ModuleStatus = AM2320_STATUS_ERROR;
    }
    
    return status;
}

/**
 * @brief Read all configured sensors
 */
Std_ReturnType AM2320_ReadAllSensors(AM2320_DataType* DataArray)
{
    Std_ReturnType status = E_OK;
    uint8 sensorIdx;
    
#if (AM2320_DEV_ERROR_DETECT == STD_ON)
    if (AM2320_ModuleStatus == AM2320_STATUS_UNINIT)
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_READ_SID, AM2320_E_UNINIT);
        return E_NOT_OK;
    }
    
    if (DataArray == NULL_PTR)
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_READ_SID, AM2320_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif
    
    for (sensorIdx = 0u; sensorIdx < AM2320_ConfigPtr->NumSensors; sensorIdx++)
    {
        if (AM2320_Read(sensorIdx, &DataArray[sensorIdx]) != E_OK)
        {
            status = E_NOT_OK;
        }
    }
    
    return status;
}

/**
 * @brief Read temperature only
 */
Std_ReturnType AM2320_ReadTemperature(AM2320_SensorType Sensor, float32* Temperature)
{
    Std_ReturnType status;
    AM2320_DataType data;
    
#if (AM2320_DEV_ERROR_DETECT == STD_ON)
    if (Temperature == NULL_PTR)
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_READ_SID, AM2320_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif
    
    status = AM2320_Read(Sensor, &data);
    
    if (status == E_OK)
    {
        *Temperature = data.TemperatureC;
    }
    
    return status;
}

/**
 * @brief Read humidity only
 */
Std_ReturnType AM2320_ReadHumidity(AM2320_SensorType Sensor, float32* Humidity)
{
    Std_ReturnType status;
    AM2320_DataType data;
    
#if (AM2320_DEV_ERROR_DETECT == STD_ON)
    if (Humidity == NULL_PTR)
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_READ_SID, AM2320_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif
    
    status = AM2320_Read(Sensor, &data);
    
    if (status == E_OK)
    {
        *Humidity = data.Humidity;
    }
    
    return status;
}

/**
 * @brief Check if sensor is connected
 */
boolean AM2320_IsSensorPresent(AM2320_SensorType Sensor)
{
    const AM2320_SensorConfigType* sensorCfg;
    
#if (AM2320_DEV_ERROR_DETECT == STD_ON)
    if (AM2320_ModuleStatus == AM2320_STATUS_UNINIT)
    {
        return FALSE;
    }
    
    if (!AM2320_IsSensorValid(Sensor))
    {
        return FALSE;
    }
#endif
    
    sensorCfg = &AM2320_ConfigPtr->Sensors[Sensor];
    
    /* Try to wake up sensor */
    (void)AM2320_WakeupSensor(sensorCfg->I2cModule);
    
    /* Try to send read command - if ACK received, sensor is present */
    return (AM2320_SendReadCommand(sensorCfg->I2cModule) == E_OK);
}

/**
 * @brief Get sensor status
 */
AM2320_SensorStatusType AM2320_GetSensorStatus(AM2320_SensorType Sensor)
{
    AM2320_SensorStatusType status = AM2320_SENSOR_DISCONNECTED;
    
#if (AM2320_DEV_ERROR_DETECT == STD_ON)
    if (AM2320_ModuleStatus == AM2320_STATUS_UNINIT)
    {
        return AM2320_SENSOR_DISCONNECTED;
    }
    
    if (!AM2320_IsSensorValid(Sensor))
    {
        return AM2320_SENSOR_DISCONNECTED;
    }
#endif
    
    status = AM2320_SensorStates[Sensor].Status;
    
    return status;
}

/**
 * @brief Get driver status
 */
AM2320_StatusType AM2320_GetStatus(void)
{
    return AM2320_ModuleStatus;
}

/**
 * @brief Get average temperature from all sensors
 */
Std_ReturnType AM2320_GetAverageTemperature(float32* AvgTemperature)
{
    float32 sum = 0.0f;
    uint8 validCount = 0u;
    uint8 sensorIdx;
    
#if (AM2320_DEV_ERROR_DETECT == STD_ON)
    if (AM2320_ModuleStatus == AM2320_STATUS_UNINIT)
    {
        return E_NOT_OK;
    }
    
    if (AvgTemperature == NULL_PTR)
    {
        return E_NOT_OK;
    }
#endif
    
    for (sensorIdx = 0u; sensorIdx < AM2320_ConfigPtr->NumSensors; sensorIdx++)
    {
        if (AM2320_SensorStates[sensorIdx].Status == AM2320_SENSOR_OK)
        {
            sum += AM2320_SensorStates[sensorIdx].LastReading.TemperatureC;
            validCount++;
        }
    }
    
    if (validCount == 0u)
    {
        return E_NOT_OK;
    }
    
    *AvgTemperature = sum / (float32)validCount;
    
    return E_OK;
}

/**
 * @brief Get maximum temperature from all sensors
 */
Std_ReturnType AM2320_GetMaxTemperature(float32* MaxTemperature, AM2320_SensorType* SensorId)
{
    float32 maxTemp = -100.0f;
    AM2320_SensorType maxSensor = 0u;
    boolean foundValid = FALSE;
    uint8 sensorIdx;
    
#if (AM2320_DEV_ERROR_DETECT == STD_ON)
    if (AM2320_ModuleStatus == AM2320_STATUS_UNINIT)
    {
        return E_NOT_OK;
    }
    
    if (MaxTemperature == NULL_PTR)
    {
        return E_NOT_OK;
    }
#endif
    
    for (sensorIdx = 0u; sensorIdx < AM2320_ConfigPtr->NumSensors; sensorIdx++)
    {
        if (AM2320_SensorStates[sensorIdx].Status == AM2320_SENSOR_OK)
        {
            if (AM2320_SensorStates[sensorIdx].LastReading.TemperatureC > maxTemp)
            {
                maxTemp = AM2320_SensorStates[sensorIdx].LastReading.TemperatureC;
                maxSensor = sensorIdx;
                foundValid = TRUE;
            }
        }
    }
    
    if (!foundValid)
    {
        return E_NOT_OK;
    }
    
    *MaxTemperature = maxTemp;
    
    if (SensorId != NULL_PTR)
    {
        *SensorId = maxSensor;
    }
    
    return E_OK;
}

#if (AM2320_ALARM_CALLBACK_API == STD_ON)
/**
 * @brief Set temperature alarm callback
 */
void AM2320_SetAlarmCallback(AM2320_AlarmCallbackType Callback)
{
    AM2320_AlarmCallback = Callback;
}
#endif

#if (AM2320_VERSION_INFO_API == STD_ON)
/**
 * @brief Get driver version information
 */
void AM2320_GetVersionInfo(Std_VersionInfoType* versionInfoPtr)
{
#if (AM2320_DEV_ERROR_DETECT == STD_ON)
    if (versionInfoPtr == NULL_PTR)
    {
        Det_ReportError(AM2320_MODULE_ID, AM2320_INSTANCE_ID, AM2320_GET_VERSION_SID, AM2320_E_PARAM_POINTER);
        return;
    }
#endif
    
    versionInfoPtr->vendorID = AM2320_VENDOR_ID;
    versionInfoPtr->moduleID = AM2320_MODULE_ID;
    versionInfoPtr->sw_major_version = AM2320_SW_MAJOR_VERSION;
    versionInfoPtr->sw_minor_version = AM2320_SW_MINOR_VERSION;
    versionInfoPtr->sw_patch_version = AM2320_SW_PATCH_VERSION;
}
#endif
