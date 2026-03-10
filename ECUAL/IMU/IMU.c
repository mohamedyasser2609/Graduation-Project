/**
 * @file IMU.c
 * @brief IMU Driver Implementation for MPU-9250
 * @details Complete implementation of MPU-9250 9-axis IMU driver
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#include "IMU.h"
#include <math.h>

/* ===================[Private Variables]=================== */

static IMU_ConfigType IMU_Config;
static boolean IMU_Initialized = FALSE;
static IMU_StatusType IMU_Status = IMU_STATUS_NOT_INITIALIZED;

/* Calibration offsets */
static IMU_AxisDataType IMU_AccelOffset = {0, 0, 0};
static IMU_AxisDataType IMU_GyroOffset = {0, 0, 0};

/* Scale factors */
static float32 IMU_AccelScale = 1.0f;
static float32 IMU_GyroScale = 1.0f;
static float32 IMU_MagScale = 1.0f;

/* ===================[Private Function Prototypes]=================== */

static Std_ReturnType IMU_WriteRegister(uint8 RegAddr, uint8 Data);
static Std_ReturnType IMU_ReadRegister(uint8 RegAddr, uint8* Data);
static Std_ReturnType IMU_ReadRegisters(uint8 RegAddr, uint8* Data, uint8 Length);
static void IMU_CalculateScaleFactors(void);
static Std_ReturnType IMU_ConfigureMagnetometer(void);

/* ===================[Function Implementations]=================== */

/**
 * @brief Initialize IMU driver
 */
Std_ReturnType IMU_Init(const IMU_ConfigType* ConfigPtr) {
    uint8 whoAmI;
    volatile uint32 i;
    
    if (ConfigPtr == NULL_PTR) {
        return E_NOT_OK;
    }
    
    /* Store configuration */
    IMU_Config = *ConfigPtr;
    
    /* Check device ID */
    if (IMU_ReadRegister(MPU9250_WHO_AM_I, &whoAmI) != E_OK) {
        IMU_Status = IMU_STATUS_DEVICE_NOT_FOUND;
        return E_NOT_OK;
    }
    
    if (whoAmI != MPU9250_DEVICE_ID && whoAmI != MPU9255_DEVICE_ID) {
        IMU_Status = IMU_STATUS_DEVICE_NOT_FOUND;
        return E_NOT_OK;
    }
    
    /* Reset device */
    IMU_WriteRegister(MPU9250_PWR_MGMT_1, 0x80);
    for(i = 0; i < 100000; i++);  /* Delay */
    
    /* Wake up device */
    IMU_WriteRegister(MPU9250_PWR_MGMT_1, 0x01);  /* Auto select clock source */
    for(i = 0; i < 10000; i++);
    
    /* Configure gyroscope */
    IMU_WriteRegister(MPU9250_GYRO_CONFIG, IMU_Config.GyroRange);
    
    /* Configure accelerometer */
    IMU_WriteRegister(MPU9250_ACCEL_CONFIG, IMU_Config.AccelRange);
    IMU_WriteRegister(MPU9250_ACCEL_CONFIG2, 0x00);  /* No DLPF */
    
    /* Configure sample rate (1kHz) */
    IMU_WriteRegister(MPU9250_SMPLRT_DIV, 0x00);
    
    /* Configure DLPF */
    IMU_WriteRegister(MPU9250_CONFIG, 0x03);  /* 41Hz bandwidth */
    
    /* Disable I2C master to allow bypass mode for magnetometer */
    IMU_WriteRegister(MPU9250_USER_CTRL, 0x00);
    for(i = 0; i < 10000; i++);
    
    /* Enable bypass mode for direct magnetometer access */
    IMU_WriteRegister(MPU9250_INT_PIN_CFG, 0x02);
    for(i = 0; i < 100000; i++);  /* Longer delay for bypass to take effect */
    
    /* Configure magnetometer (optional - don't fail if not accessible) */
    if (IMU_ConfigureMagnetometer() != E_OK) {
        /* Magnetometer init failed - continue without it */
        /* Log warning but don't fail initialization */
    }
    
    /* Calculate scale factors */
    IMU_CalculateScaleFactors();
    
    IMU_Initialized = TRUE;
    IMU_Status = IMU_STATUS_OK;
    
    return E_OK;
}

/**
 * @brief De-initialize IMU
 */
Std_ReturnType IMU_DeInit(void) {
    /* Put device in sleep mode */
    IMU_WriteRegister(MPU9250_PWR_MGMT_1, 0x40);
    
    IMU_Initialized = FALSE;
    IMU_Status = IMU_STATUS_NOT_INITIALIZED;
    
    return E_OK;
}

/**
 * @brief Read raw sensor data
 */
Std_ReturnType IMU_ReadRawData(IMU_SensorDataType* Data) {
    uint8 buffer[14];
    
    if (!IMU_Initialized || Data == NULL_PTR) {
        return E_NOT_OK;
    }
    
    /* Read all sensor data in one burst */
    if (IMU_ReadRegisters(MPU9250_ACCEL_XOUT_H, buffer, 14) != E_OK) {
        return E_NOT_OK;
    }
    
    /* Parse accelerometer data */
    Data->accel.x = (sint16)((buffer[0] << 8) | buffer[1]);
    Data->accel.y = (sint16)((buffer[2] << 8) | buffer[3]);
    Data->accel.z = (sint16)((buffer[4] << 8) | buffer[5]);
    
    /* Parse temperature */
    Data->temperature = (sint16)((buffer[6] << 8) | buffer[7]);
    
    /* Parse gyroscope data */
    Data->gyro.x = (sint16)((buffer[8] << 8) | buffer[9]);
    Data->gyro.y = (sint16)((buffer[10] << 8) | buffer[11]);
    Data->gyro.z = (sint16)((buffer[12] << 8) | buffer[13]);
    
    /* Read magnetometer data */
    IMU_ReadMag(&Data->mag);
    
    return E_OK;
}

/**
 * @brief Read calibrated data
 */
Std_ReturnType IMU_ReadCalibratedData(IMU_CalibratedDataType* Data) {
    IMU_SensorDataType rawData;
    
    if (!IMU_Initialized || Data == NULL_PTR) {
        return E_NOT_OK;
    }
    
    /* Read raw data */
    if (IMU_ReadRawData(&rawData) != E_OK) {
        return E_NOT_OK;
    }
    
    /* Apply calibration and scaling to accelerometer */
    Data->accel.x = (rawData.accel.x - IMU_AccelOffset.x) * IMU_AccelScale;
    Data->accel.y = (rawData.accel.y - IMU_AccelOffset.y) * IMU_AccelScale;
    Data->accel.z = (rawData.accel.z - IMU_AccelOffset.z) * IMU_AccelScale;
    
    /* Apply calibration and scaling to gyroscope */
    Data->gyro.x = (rawData.gyro.x - IMU_GyroOffset.x) * IMU_GyroScale;
    Data->gyro.y = (rawData.gyro.y - IMU_GyroOffset.y) * IMU_GyroScale;
    Data->gyro.z = (rawData.gyro.z - IMU_GyroOffset.z) * IMU_GyroScale;
    
    /* Apply scaling to magnetometer */
    Data->mag.x = rawData.mag.x * IMU_MagScale;
    Data->mag.y = rawData.mag.y * IMU_MagScale;
    Data->mag.z = rawData.mag.z * IMU_MagScale;
    
    /* Convert temperature to Celsius */
    Data->temperature = (rawData.temperature / 333.87f) + 21.0f;
    
    return E_OK;
}

/**
 * @brief Read accelerometer only
 */
Std_ReturnType IMU_ReadAccel(IMU_AxisDataType* Data) {
    uint8 buffer[6];
    
    if (!IMU_Initialized || Data == NULL_PTR) {
        return E_NOT_OK;
    }
    
    if (IMU_ReadRegisters(MPU9250_ACCEL_XOUT_H, buffer, 6) != E_OK) {
        return E_NOT_OK;
    }
    
    Data->x = (sint16)((buffer[0] << 8) | buffer[1]);
    Data->y = (sint16)((buffer[2] << 8) | buffer[3]);
    Data->z = (sint16)((buffer[4] << 8) | buffer[5]);
    
    return E_OK;
}

/**
 * @brief Read gyroscope only
 */
Std_ReturnType IMU_ReadGyro(IMU_AxisDataType* Data) {
    uint8 buffer[6];
    
    if (!IMU_Initialized || Data == NULL_PTR) {
        return E_NOT_OK;
    }
    
    if (IMU_ReadRegisters(MPU9250_GYRO_XOUT_H, buffer, 6) != E_OK) {
        return E_NOT_OK;
    }
    
    Data->x = (sint16)((buffer[0] << 8) | buffer[1]);
    Data->y = (sint16)((buffer[2] << 8) | buffer[3]);
    Data->z = (sint16)((buffer[4] << 8) | buffer[5]);
    
    return E_OK;
}

/**
 * @brief Read magnetometer only
 */
Std_ReturnType IMU_ReadMag(IMU_AxisDataType* Data) {
    uint8 buffer[7];
    uint8 status;
    
    if (!IMU_Initialized || Data == NULL_PTR) {
        return E_NOT_OK;
    }
    
    /* Read magnetometer via I2C bypass */
    if (I2C_ReadRegister(IMU_Config.I2C_Module, AK8963_I2C_ADDR, AK8963_ST1, &status, 1) != E_OK) {
        return E_NOT_OK;
    }
    
    /* Check if data is ready */
    if ((status & 0x01) == 0) {
        return E_NOT_OK;
    }
    
    /* Read magnetometer data */
    if (I2C_ReadRegister(IMU_Config.I2C_Module, AK8963_I2C_ADDR, AK8963_XOUT_L, buffer, 7) != E_OK) {
        return E_NOT_OK;
    }
    
    /* Check overflow */
    if (buffer[6] & 0x08) {
        return E_NOT_OK;
    }
    
    Data->x = (sint16)((buffer[1] << 8) | buffer[0]);
    Data->y = (sint16)((buffer[3] << 8) | buffer[2]);
    Data->z = (sint16)((buffer[5] << 8) | buffer[4]);
    
    return E_OK;
}

/**
 * @brief Read temperature
 */
Std_ReturnType IMU_ReadTemperature(float32* Temperature) {
    uint8 buffer[2];
    sint16 rawTemp;
    
    if (!IMU_Initialized || Temperature == NULL_PTR) {
        return E_NOT_OK;
    }
    
    if (IMU_ReadRegisters(MPU9250_TEMP_OUT_H, buffer, 2) != E_OK) {
        return E_NOT_OK;
    }
    
    rawTemp = (sint16)((buffer[0] << 8) | buffer[1]);
    *Temperature = (rawTemp / 333.87f) + 21.0f;
    
    return E_OK;
}

/**
 * @brief Calibrate IMU
 */
Std_ReturnType IMU_Calibrate(uint16 Samples) {
    IMU_AxisDataType accelSum = {0, 0, 0};
    IMU_AxisDataType gyroSum = {0, 0, 0};
    IMU_AxisDataType tempData;
    uint16 i;
    volatile uint32 j;
    
    if (!IMU_Initialized || Samples == 0) {
        return E_NOT_OK;
    }
    
    /* Collect samples */
    for (i = 0; i < Samples; i++) {
        if (IMU_ReadAccel(&tempData) == E_OK) {
            accelSum.x += tempData.x;
            accelSum.y += tempData.y;
            accelSum.z += tempData.z;
        }
        
        if (IMU_ReadGyro(&tempData) == E_OK) {
            gyroSum.x += tempData.x;
            gyroSum.y += tempData.y;
            gyroSum.z += tempData.z;
        }
        
        for(j = 0; j < 1000; j++);  /* Small delay */
    }
    
    /* Calculate averages */
    IMU_AccelOffset.x = accelSum.x / Samples;
    IMU_AccelOffset.y = accelSum.y / Samples;
    IMU_AccelOffset.z = (accelSum.z / Samples) - (sint16)(16384);  /* Subtract 1g */
    
    IMU_GyroOffset.x = gyroSum.x / Samples;
    IMU_GyroOffset.y = gyroSum.y / Samples;
    IMU_GyroOffset.z = gyroSum.z / Samples;
    
    return E_OK;
}

/**
 * @brief Self-test
 */
Std_ReturnType IMU_SelfTest(void) {
    uint8 whoAmI;
    
    if (!IMU_Initialized) {
        return E_NOT_OK;
    }
    
    /* Check device ID */
    if (IMU_ReadRegister(MPU9250_WHO_AM_I, &whoAmI) != E_OK) {
        return E_NOT_OK;
    }
    
    if (whoAmI != MPU9250_DEVICE_ID && whoAmI != MPU9255_DEVICE_ID) {
        return E_NOT_OK;
    }
    
    return E_OK;
}

/**
 * @brief Get status
 */
IMU_StatusType IMU_GetStatus(void) {
    return IMU_Status;
}

/**
 * @brief Reset device
 */
Std_ReturnType IMU_Reset(void) {
    volatile uint32 i;
    
    if (!IMU_Initialized) {
        return E_NOT_OK;
    }
    
    /* Software reset */
    IMU_WriteRegister(MPU9250_PWR_MGMT_1, 0x80);
    for(i = 0; i < 100000; i++);
    
    /* Re-initialize */
    return IMU_Init(&IMU_Config);
}

/**
 * @brief Check if device is present
 */
boolean IMU_IsDevicePresent(void) {
    uint8 whoAmI;
    
    if (IMU_ReadRegister(MPU9250_WHO_AM_I, &whoAmI) != E_OK) {
        return FALSE;
    }
    
    return (whoAmI == MPU9250_DEVICE_ID || whoAmI == MPU9255_DEVICE_ID);
}

/* ===================[Private Functions]=================== */

/**
 * @brief Write single register
 */
static Std_ReturnType IMU_WriteRegister(uint8 RegAddr, uint8 Data) {
    return I2C_WriteRegister(IMU_Config.I2C_Module, IMU_Config.DeviceAddress, RegAddr, &Data, 1);
}

/**
 * @brief Read single register
 */
static Std_ReturnType IMU_ReadRegister(uint8 RegAddr, uint8* Data) {
    return I2C_ReadRegister(IMU_Config.I2C_Module, IMU_Config.DeviceAddress, RegAddr, Data, 1);
}

/**
 * @brief Read multiple registers
 */
static Std_ReturnType IMU_ReadRegisters(uint8 RegAddr, uint8* Data, uint8 Length) {
    return I2C_ReadRegister(IMU_Config.I2C_Module, IMU_Config.DeviceAddress, RegAddr, Data, Length);
}

/**
 * @brief Calculate scale factors based on configuration
 */
static void IMU_CalculateScaleFactors(void) {
    /* Accelerometer scale factor (g) */
    switch (IMU_Config.AccelRange) {
        case MPU9250_ACCEL_FS_2G:
            IMU_AccelScale = 2.0f / 32768.0f;
            break;
        case MPU9250_ACCEL_FS_4G:
            IMU_AccelScale = 4.0f / 32768.0f;
            break;
        case MPU9250_ACCEL_FS_8G:
            IMU_AccelScale = 8.0f / 32768.0f;
            break;
        case MPU9250_ACCEL_FS_16G:
            IMU_AccelScale = 16.0f / 32768.0f;
            break;
        default:
            IMU_AccelScale = 2.0f / 32768.0f;
            break;
    }
    
    /* Gyroscope scale factor (°/s) */
    switch (IMU_Config.GyroRange) {
        case MPU9250_GYRO_FS_250DPS:
            IMU_GyroScale = 250.0f / 32768.0f;
            break;
        case MPU9250_GYRO_FS_500DPS:
            IMU_GyroScale = 500.0f / 32768.0f;
            break;
        case MPU9250_GYRO_FS_1000DPS:
            IMU_GyroScale = 1000.0f / 32768.0f;
            break;
        case MPU9250_GYRO_FS_2000DPS:
            IMU_GyroScale = 2000.0f / 32768.0f;
            break;
        default:
            IMU_GyroScale = 250.0f / 32768.0f;
            break;
    }
    
    /* Magnetometer scale factor (µT) */
    if (IMU_Config.MagResolution == AK8963_BIT_16) {
        IMU_MagScale = 4912.0f / 32760.0f;  /* 16-bit: 0.15 µT/LSB */
    } else {
        IMU_MagScale = 4912.0f / 8190.0f;   /* 14-bit: 0.6 µT/LSB */
    }
}

/**
 * @brief Configure magnetometer
 */
static Std_ReturnType IMU_ConfigureMagnetometer(void) {
    uint8 data;
    volatile uint32 i;
    
    /* Check magnetometer ID */
    if (I2C_ReadRegister(IMU_Config.I2C_Module, AK8963_I2C_ADDR, AK8963_WHO_AM_I, &data, 1) != E_OK) {
        return E_NOT_OK;
    }
    
    if (data != AK8963_DEVICE_ID) {
        return E_NOT_OK;
    }
    
    /* Power down magnetometer */
    data = AK8963_MODE_POWER_DOWN;
    I2C_WriteRegister(IMU_Config.I2C_Module, AK8963_I2C_ADDR, AK8963_CNTL1, &data, 1);
    for(i = 0; i < 10000; i++);
    
    /* Set magnetometer mode and resolution */
    data = IMU_Config.MagMode | IMU_Config.MagResolution;
    I2C_WriteRegister(IMU_Config.I2C_Module, AK8963_I2C_ADDR, AK8963_CNTL1, &data, 1);
    for(i = 0; i < 10000; i++);
    
    return E_OK;
}
