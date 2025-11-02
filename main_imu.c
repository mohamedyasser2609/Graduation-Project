/*
 * ============================================================================
 * THIS FILE IS CURRENTLY COMMENTED OUT - ROS2 BRIDGE IS ACTIVE (main_ros2.c)
 * To use this IMU test version, uncomment this file and comment main_ros2.c
 * ============================================================================
 */

#if 0  /* <<<< ENTIRE IMU MAIN COMMENTED OUT - REMOVE THIS LINE TO ACTIVATE >>>> */

/**
 * @file main_imu.c
 * @brief MPU-9250 IMU Test Program for TM4C123GH6PM
 * @details 9-axis motion tracking: 3-axis gyroscope, 3-axis accelerometer, 3-axis magnetometer
 *
 * Hardware Setup:
 * - TM4C123GH6PM LaunchPad
 * - MPU-9250 IMU Module
 * - I2C Connection:
 *   - SCL → PB2 (I2C0 SCL)
 *   - SDA → PB3 (I2C0 SDA)
 *   - VCC → 3.3V
 *   - GND → GND
 *
 * MPU-9250 Specifications:
 * - Gyroscope: ±250, ±500, ±1000, ±2000 °/s
 * - Accelerometer: ±2g, ±4g, ±8g, ±16g
 * - Magnetometer: ±4800 μT
 * - I2C Address: 0x68 (default) or 0x69 (if AD0 high)
 *
 * @author Mohamed Yasser
 * @date Oct 29, 2025
 * @version 1.0.0
 */

/* ===================[Includes]=================== */
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/UART/Uart.h"
#include "MCAL/I2C/I2C.h"
#include "CONFIG/Std_Types.h"
#include "tm4c123gh6pm.h"

/* Type aliases for compatibility */
typedef sint16 int16;
typedef sint32 int32;

/* ===================[MPU-9250 Definitions]=================== */
#define MPU9250_I2C_ADDR        0x68    /* MPU-9250 I2C address (7-bit) */
#define AK8963_I2C_ADDR         0x0C    /* Magnetometer I2C address */

/* MPU-9250 Register Map */
#define MPU9250_WHO_AM_I        0x75    /* Device ID register */
#define MPU9250_PWR_MGMT_1      0x6B    /* Power management */
#define MPU9250_PWR_MGMT_2      0x6C
#define MPU9250_CONFIG          0x1A    /* Configuration */
#define MPU9250_GYRO_CONFIG     0x1B    /* Gyroscope configuration */
#define MPU9250_ACCEL_CONFIG    0x1C    /* Accelerometer configuration */
#define MPU9250_ACCEL_CONFIG2   0x1D
#define MPU9250_INT_PIN_CFG     0x37    /* Interrupt pin configuration */
#define MPU9250_INT_ENABLE      0x38    /* Interrupt enable */
#define MPU9250_INT_STATUS      0x3A    /* Interrupt status */

/* Data Registers */
#define MPU9250_ACCEL_XOUT_H    0x3B    /* Accelerometer X-axis high byte */
#define MPU9250_ACCEL_XOUT_L    0x3C
#define MPU9250_ACCEL_YOUT_H    0x3D
#define MPU9250_ACCEL_YOUT_L    0x3E
#define MPU9250_ACCEL_ZOUT_H    0x3F
#define MPU9250_ACCEL_ZOUT_L    0x40
#define MPU9250_TEMP_OUT_H      0x41    /* Temperature high byte */
#define MPU9250_TEMP_OUT_L      0x42
#define MPU9250_GYRO_XOUT_H     0x43    /* Gyroscope X-axis high byte */
#define MPU9250_GYRO_XOUT_L     0x44
#define MPU9250_GYRO_YOUT_H     0x45
#define MPU9250_GYRO_YOUT_L     0x46
#define MPU9250_GYRO_ZOUT_H     0x47
#define MPU9250_GYRO_ZOUT_L     0x48

/* Magnetometer Registers (AK8963) */
#define AK8963_WHO_AM_I         0x00    /* Device ID */
#define AK8963_ST1              0x02    /* Status 1 */
#define AK8963_XOUT_L           0x03    /* Magnetometer X-axis low byte */
#define AK8963_XOUT_H           0x04
#define AK8963_YOUT_L           0x05
#define AK8963_YOUT_H           0x06
#define AK8963_ZOUT_L           0x07
#define AK8963_ZOUT_H           0x08
#define AK8963_ST2              0x09    /* Status 2 */
#define AK8963_CNTL1            0x0A    /* Control 1 */
#define AK8963_CNTL2            0x0B    /* Control 2 */

/* Scale Factors */
#define ACCEL_SCALE_2G          16384.0f    /* LSB/g for ±2g */
#define ACCEL_SCALE_4G          8192.0f     /* LSB/g for ±4g */
#define ACCEL_SCALE_8G          4096.0f     /* LSB/g for ±8g */
#define ACCEL_SCALE_16G         2048.0f     /* LSB/g for ±16g */

#define GYRO_SCALE_250DPS       131.0f      /* LSB/(°/s) for ±250°/s */
#define GYRO_SCALE_500DPS       65.5f       /* LSB/(°/s) for ±500°/s */
#define GYRO_SCALE_1000DPS      32.8f       /* LSB/(°/s) for ±1000°/s */
#define GYRO_SCALE_2000DPS      16.4f       /* LSB/(°/s) for ±2000°/s */

#define MAG_SCALE               0.6f        /* μT/LSB for magnetometer */

/* UART Configuration */
#define DEBUG_UART_MODULE       UART_MODULE_0
#define DEBUG_BAUD_RATE         UART_BAUD_115200

/* I2C Configuration */
#define IMU_I2C_MODULE          I2C_MODULE_0

/* ===================[Type Definitions]=================== */
typedef struct {
    int16 accelX;
    int16 accelY;
    int16 accelZ;
    int16 gyroX;
    int16 gyroY;
    int16 gyroZ;
    int16 magX;
    int16 magY;
    int16 magZ;
    int16 temperature;
    float accelX_g;
    float accelY_g;
    float accelZ_g;
    float gyroX_dps;
    float gyroY_dps;
    float gyroZ_dps;
    float magX_uT;
    float magY_uT;
    float magZ_uT;
    float temp_C;
} ImuData_t;

/* ===================[Global Variables]=================== */
static ImuData_t imuData = {0};

/* ===================[Interrupt Handlers]=================== */

/**
 * @brief Timer2A interrupt handler (dummy)
 */
void Timer2A_Handler(void) {
    /* Not used */
}

/* ===================[Helper Functions]=================== */

/**
 * @brief Send string to debug UART
 */
void Debug_Print(const uint8* str) {
    Uart_SendString(DEBUG_UART_MODULE, str);
}

/**
 * @brief Convert sint32 to string
 */
void Int32ToString(sint32 value, uint8* buffer) {
    uint8 i = 0;
    uint8 temp[12];
    uint8 j;
    boolean isNegative = FALSE;
    
    if (value < 0) {
        isNegative = TRUE;
        value = -value;
    }
    
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    j = 0;
    if (isNegative) {
        buffer[j++] = '-';
    }
    
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

/**
 * @brief Convert uint8 to hex string
 */
void Uint8ToHexString(uint8 value, uint8* buffer) {
    const uint8 hexChars[] = "0123456789ABCDEF";
    buffer[0] = hexChars[(value >> 4) & 0x0F];
    buffer[1] = hexChars[value & 0x0F];
    buffer[2] = '\0';
}

/**
 * @brief Convert float to string (2 decimal places)
 */
void FloatToString(float value, uint8* buffer) {
    sint32 intPart;
    sint32 fracPart;
    uint8 intStr[12];
    uint8 fracStr[12];
    uint8 idx = 0;
    
    if (value < 0) {
        buffer[idx++] = '-';
        value = -value;
    }
    
    intPart = (int32)value;
    fracPart = (int32)((value - intPart) * 100);
    
    Int32ToString(intPart, intStr);
    Int32ToString(fracPart, fracStr);
    
    /* Copy integer part */
    uint8 i = 0;
    while (intStr[i] != '\0') {
        buffer[idx++] = intStr[i++];
    }
    
    buffer[idx++] = '.';
    
    /* Pad fraction if needed */
    if (fracPart < 10) {
        buffer[idx++] = '0';
    }
    
    /* Copy fraction part */
    i = 0;
    while (fracStr[i] != '\0' && i < 2) {
        buffer[idx++] = fracStr[i++];
    }
    
    buffer[idx] = '\0';
}

/* ===================[I2C Communication Functions]=================== */

/**
 * @brief Write single byte to MPU-9250 register
 */
Std_ReturnType MPU9250_WriteRegister(uint8 regAddr, uint8 data) {
    uint8 txData[2];
    txData[0] = regAddr;
    txData[1] = data;
    
    return I2C_MasterTransmit(IMU_I2C_MODULE, MPU9250_I2C_ADDR, txData, 2);
}

/**
 * @brief Read single byte from MPU-9250 register
 */
Std_ReturnType MPU9250_ReadRegister(uint8 regAddr, uint8* data) {
    Std_ReturnType status;
    
    /* Write register address */
    status = I2C_MasterTransmit(IMU_I2C_MODULE, MPU9250_I2C_ADDR, &regAddr, 1);
    if (status != E_OK) return status;
    
    /* Read data */
    return I2C_MasterReceive(IMU_I2C_MODULE, MPU9250_I2C_ADDR, data, 1);
}

/**
 * @brief Read multiple bytes from MPU-9250
 */
Std_ReturnType MPU9250_ReadRegisters(uint8 regAddr, uint8* data, uint8 length) {
    Std_ReturnType status;
    
    /* Write register address */
    status = I2C_MasterTransmit(IMU_I2C_MODULE, MPU9250_I2C_ADDR, &regAddr, 1);
    if (status != E_OK) return status;
    
    /* Read data */
    return I2C_MasterReceive(IMU_I2C_MODULE, MPU9250_I2C_ADDR, data, length);
}

/* ===================[MPU-9250 Initialization]=================== */

/**
 * @brief Test I2C GPIO pins
 */
void I2C_GPIOTest(void) {
    Debug_Print((const uint8*)"\r\n--- I2C GPIO Pin Test ---\r\n");
    
    /* Check Port B clock */
    if (SYSCTL_RCGCGPIO_R & 0x02) {
        Debug_Print((const uint8*)"[OK] Port B clock enabled\r\n");
    } else {
        Debug_Print((const uint8*)"[ERROR] Port B clock NOT enabled!\r\n");
    }
    
    /* Check PB2 (SCL) configuration */
    if (GPIO_PORTB_AFSEL_R & 0x04) {
        Debug_Print((const uint8*)"[OK] PB2 alternate function enabled\r\n");
    } else {
        Debug_Print((const uint8*)"[ERROR] PB2 NOT in alternate function mode!\r\n");
    }
    
    /* Check PB3 (SDA) configuration */
    if (GPIO_PORTB_AFSEL_R & 0x08) {
        Debug_Print((const uint8*)"[OK] PB3 alternate function enabled\r\n");
    } else {
        Debug_Print((const uint8*)"[ERROR] PB3 NOT in alternate function mode!\r\n");
    }
    
    /* Check digital enable */
    if ((GPIO_PORTB_DEN_R & 0x0C) == 0x0C) {
        Debug_Print((const uint8*)"[OK] PB2/PB3 digital enable set\r\n");
    } else {
        Debug_Print((const uint8*)"[ERROR] PB2/PB3 digital enable NOT set!\r\n");
    }
    
    /* Check open drain */
    if ((GPIO_PORTB_ODR_R & 0x0C) == 0x0C) {
        Debug_Print((const uint8*)"[OK] PB2/PB3 open drain enabled\r\n");
    } else {
        Debug_Print((const uint8*)"[WARNING] PB2/PB3 open drain NOT enabled\r\n");
    }
    
    /* Check PCTL for I2C function (3) */
    uint32 pctl = GPIO_PORTB_PCTL_R;
    uint8 pb2_func = (pctl >> 8) & 0x0F;
    uint8 pb3_func = (pctl >> 12) & 0x0F;
    
    if (pb2_func == 3) {
        Debug_Print((const uint8*)"[OK] PB2 configured for I2C (function 3)\r\n");
    } else {
        Debug_Print((const uint8*)"[ERROR] PB2 NOT configured for I2C! (function ");
        uint8 buf[4];
        Int32ToString(pb2_func, buf);
        Debug_Print(buf);
        Debug_Print((const uint8*)")\r\n");
    }
    
    if (pb3_func == 3) {
        Debug_Print((const uint8*)"[OK] PB3 configured for I2C (function 3)\r\n");
    } else {
        Debug_Print((const uint8*)"[ERROR] PB3 NOT configured for I2C! (function ");
        uint8 buf[4];
        Int32ToString(pb3_func, buf);
        Debug_Print(buf);
        Debug_Print((const uint8*)")\r\n");
    }
    
    Debug_Print((const uint8*)"\r\n");
}

/**
 * @brief Test I2C hardware registers
 */
void I2C_HardwareTest(void) {
    Debug_Print((const uint8*)"\r\n--- I2C Hardware Test ---\r\n");
    
    /* Check if I2C clock is enabled */
    if (SYSCTL_RCGCI2C_R & 0x01) {
        Debug_Print((const uint8*)"[OK] I2C0 clock enabled\r\n");
    } else {
        Debug_Print((const uint8*)"[ERROR] I2C0 clock NOT enabled!\r\n");
    }
    
    /* Check if I2C master is enabled */
    if (I2C0_MCR_R & I2C_MCR_MFE) {
        Debug_Print((const uint8*)"[OK] I2C0 master function enabled\r\n");
    } else {
        Debug_Print((const uint8*)"[ERROR] I2C0 master NOT enabled!\r\n");
    }
    
    /* Display I2C timer period register */
    Debug_Print((const uint8*)"I2C0 Timer Period (MTPR): ");
    uint8 buf[8];
    Int32ToString(I2C0_MTPR_R, buf);
    Debug_Print(buf);
    Debug_Print((const uint8*)" (should be 7 for 100kHz)\r\n");
    
    /* Check bus status */
    uint32 mcs = I2C0_MCS_R;
    
    if (mcs & I2C_MCS_BUSBSY) {
        Debug_Print((const uint8*)"[ERROR] I2C bus BUSY (stuck!)\r\n");
        Debug_Print((const uint8*)"  -> SCL or SDA stuck LOW\r\n");
        Debug_Print((const uint8*)"  -> Check pull-up resistors!\r\n");
    } else if (mcs & I2C_MCS_IDLE) {
        Debug_Print((const uint8*)"[OK] I2C bus idle\r\n");
    } else {
        Debug_Print((const uint8*)"[WARNING] I2C bus status unclear\r\n");
    }
    
    /* Try to read the actual pin states */
    Debug_Print((const uint8*)"\r\nPin States (reading GPIO_PORTB_DATA_R):\r\n");
    uint32 pinState = GPIO_PORTB_DATA_R & 0x0C;
    Debug_Print((const uint8*)"  PB2 (SCL): ");
    if (pinState & 0x04) {
        Debug_Print((const uint8*)"HIGH\r\n");
    } else {
        Debug_Print((const uint8*)"LOW\r\n");
    }
    
    Debug_Print((const uint8*)"  PB3 (SDA): ");
    if (pinState & 0x08) {
        Debug_Print((const uint8*)"HIGH\r\n");
    } else {
        Debug_Print((const uint8*)"LOW\r\n");
    }
    
    /* Check for pull-up resistors */
    Debug_Print((const uint8*)"\r\n--- Pull-Up Resistor Test ---\r\n");
    Debug_Print((const uint8*)"IMPORTANT: TM4C123 does NOT have internal pull-ups\r\n");
    Debug_Print((const uint8*)"           for I2C pins in alternate function mode!\r\n");
    Debug_Print((const uint8*)"\r\n");
    
    if ((pinState & 0x0C) == 0x0C) {
        Debug_Print((const uint8*)"Both pins are HIGH.\r\n");
        Debug_Print((const uint8*)"This could mean:\r\n");
        Debug_Print((const uint8*)"  1. MPU-9250 module has built-in pull-ups (good!)\r\n");
        Debug_Print((const uint8*)"  2. External pull-ups are connected (good!)\r\n");
        Debug_Print((const uint8*)"  3. Pins are floating (bad - won't work!)\r\n");
        Debug_Print((const uint8*)"\r\n");
        Debug_Print((const uint8*)"To verify, check your MPU-9250 module:\r\n");
        Debug_Print((const uint8*)"  - Look for R1/R2 resistors near SCL/SDA (4.7k or 10k)\r\n");
        Debug_Print((const uint8*)"  - If no resistors visible, add external 4.7k pull-ups!\r\n");
    } else {
        Debug_Print((const uint8*)"[ERROR] One or both pins are LOW!\r\n");
        Debug_Print((const uint8*)"  -> No pull-up resistors present\r\n");
        Debug_Print((const uint8*)"  -> OR device is pulling line LOW (short circuit)\r\n");
        Debug_Print((const uint8*)"\r\n");
        Debug_Print((const uint8*)"ACTION REQUIRED:\r\n");
        Debug_Print((const uint8*)"  Add 4.7k pull-up resistors:\r\n");
        Debug_Print((const uint8*)"    3.3V ---[4.7k]--- SCL (PB2)\r\n");
        Debug_Print((const uint8*)"    3.3V ---[4.7k]--- SDA (PB3)\r\n");
    }
    
    Debug_Print((const uint8*)"\r\n");
}

/**
 * @brief Scan I2C bus for devices
 */
void I2C_ScanBus(void) {
    uint8 addr;
    uint8 dummy;
    Std_ReturnType status;
    uint8 hexBuffer[4];
    uint8 foundCount = 0;
    
    /* First test GPIO configuration */
    I2C_GPIOTest();
    
    /* Then test I2C hardware */
    I2C_HardwareTest();
    
    Debug_Print((const uint8*)"\r\n========================================\r\n");
    Debug_Print((const uint8*)"Scanning I2C Bus (0x08 to 0x77)\r\n");
    Debug_Print((const uint8*)"========================================\r\n");
    
    for (addr = 0x08; addr < 0x78; addr++) {
        /* Try to read from this address */
        status = I2C_MasterReceive(IMU_I2C_MODULE, addr, &dummy, 1);
        
        if (status == E_OK) {
            Debug_Print((const uint8*)"[FOUND] Device at address: 0x");
            Uint8ToHexString(addr, hexBuffer);
            Debug_Print(hexBuffer);
            
            /* Identify common devices */
            if (addr == 0x68 || addr == 0x69) {
                Debug_Print((const uint8*)" (MPU-9250/6050)");
            }
            Debug_Print((const uint8*)"\r\n");
            foundCount++;
        }
    }
    
    Debug_Print((const uint8*)"\r\n");
    if (foundCount == 0) {
        Debug_Print((const uint8*)"[ERROR] No I2C devices found!\r\n");
        Debug_Print((const uint8*)"\r\nTroubleshooting Checklist:\r\n");
        Debug_Print((const uint8*)"  [ ] SCL connected to PB2\r\n");
        Debug_Print((const uint8*)"  [ ] SDA connected to PB3\r\n");
        Debug_Print((const uint8*)"  [ ] VCC connected (3.3V or 5V)\r\n");
        Debug_Print((const uint8*)"  [ ] GND connected\r\n");
        Debug_Print((const uint8*)"  [ ] Pull-up resistors present (4.7k)\r\n");
        Debug_Print((const uint8*)"  [ ] Module powered on\r\n");
    } else {
        Debug_Print((const uint8*)"Total devices found: ");
        uint8 countStr[4];
        Int32ToString(foundCount, countStr);
        Debug_Print(countStr);
        Debug_Print((const uint8*)"\r\n");
    }
    Debug_Print((const uint8*)"========================================\r\n\r\n");
}

/**
 * @brief Initialize MPU-9250 sensor
 */
Std_ReturnType MPU9250_Init(void) {
    uint8 whoAmI;
    Std_ReturnType status;
    uint8 buffer[8];
    
    Debug_Print((const uint8*)"Initializing MPU-9250...\r\n");
    
    /* First, scan I2C bus */
    I2C_ScanBus();
    
    /* Try default address 0x68 */
    Debug_Print((const uint8*)"Trying address 0x68...\r\n");
    status = MPU9250_ReadRegister(MPU9250_WHO_AM_I, &whoAmI);
    
    if (status != E_OK) {
        /* Try alternate address 0x69 */
        Debug_Print((const uint8*)"Failed. Trying address 0x69...\r\n");
        
        /* Change to alternate address */
        #undef MPU9250_I2C_ADDR
        #define MPU9250_I2C_ADDR 0x69
        
        status = MPU9250_ReadRegister(MPU9250_WHO_AM_I, &whoAmI);
        
        if (status != E_OK) {
            Debug_Print((const uint8*)"[ERROR] Failed to read WHO_AM_I at both addresses\r\n");
            Debug_Print((const uint8*)"[ERROR] Check:\r\n");
            Debug_Print((const uint8*)"  1. SCL connected to PB2\r\n");
            Debug_Print((const uint8*)"  2. SDA connected to PB3\r\n");
            Debug_Print((const uint8*)"  3. VCC connected to 3.3V\r\n");
            Debug_Print((const uint8*)"  4. GND connected\r\n");
            Debug_Print((const uint8*)"  5. Pull-up resistors on SCL/SDA (4.7k)\r\n");
            return E_NOT_OK;
        }
    }
    
    Debug_Print((const uint8*)"[OK] WHO_AM_I: 0x");
    Uint8ToHexString(whoAmI, buffer);
    Debug_Print(buffer);
    Debug_Print((const uint8*)"\r\n");
    
    if (whoAmI != 0x71 && whoAmI != 0x73) {  /* 0x71 for MPU-9250, 0x73 for MPU-9255 */
        Debug_Print((const uint8*)"[ERROR] Invalid device ID!\r\n");
        return E_NOT_OK;
    }
    
    /* Reset device */
    MPU9250_WriteRegister(MPU9250_PWR_MGMT_1, 0x80);
    {
        volatile uint32 delay;
        for (delay = 0; delay < 100000; delay++) { }  /* Wait 100ms */
    }
    
    /* Wake up device and select clock source */
    MPU9250_WriteRegister(MPU9250_PWR_MGMT_1, 0x01);  /* Auto select clock */
    
    /* Configure gyroscope: ±250°/s */
    MPU9250_WriteRegister(MPU9250_GYRO_CONFIG, 0x00);
    
    /* Configure accelerometer: ±2g */
    MPU9250_WriteRegister(MPU9250_ACCEL_CONFIG, 0x00);
    
    /* Configure accelerometer low-pass filter */
    MPU9250_WriteRegister(MPU9250_ACCEL_CONFIG2, 0x00);
    
    /* Configure sample rate divider */
    MPU9250_WriteRegister(MPU9250_CONFIG, 0x00);
    
    /* Enable bypass mode to access magnetometer */
    MPU9250_WriteRegister(MPU9250_INT_PIN_CFG, 0x02);
    
    Debug_Print((const uint8*)"[OK] MPU-9250 initialized successfully!\r\n\r\n");
    
    return E_OK;
}

/* ===================[Data Reading Functions]=================== */

/**
 * @brief Read accelerometer and gyroscope data
 */
void MPU9250_ReadAccelGyro(void) {
    uint8 rawData[14];
    
    /* Read all sensor data (14 bytes) */
    if (MPU9250_ReadRegisters(MPU9250_ACCEL_XOUT_H, rawData, 14) == E_OK) {
        /* Combine high and low bytes */
        imuData.accelX = (int16)((rawData[0] << 8) | rawData[1]);
        imuData.accelY = (int16)((rawData[2] << 8) | rawData[3]);
        imuData.accelZ = (int16)((rawData[4] << 8) | rawData[5]);
        
        imuData.temperature = (int16)((rawData[6] << 8) | rawData[7]);
        
        imuData.gyroX = (int16)((rawData[8] << 8) | rawData[9]);
        imuData.gyroY = (int16)((rawData[10] << 8) | rawData[11]);
        imuData.gyroZ = (int16)((rawData[12] << 8) | rawData[13]);
        
        /* Convert to physical units */
        imuData.accelX_g = (float)imuData.accelX / ACCEL_SCALE_2G;
        imuData.accelY_g = (float)imuData.accelY / ACCEL_SCALE_2G;
        imuData.accelZ_g = (float)imuData.accelZ / ACCEL_SCALE_2G;
        
        imuData.gyroX_dps = (float)imuData.gyroX / GYRO_SCALE_250DPS;
        imuData.gyroY_dps = (float)imuData.gyroY / GYRO_SCALE_250DPS;
        imuData.gyroZ_dps = (float)imuData.gyroZ / GYRO_SCALE_250DPS;
        
        /* Convert temperature: ((TEMP_OUT - RoomTemp_Offset)/Temp_Sensitivity) + 21°C */
        imuData.temp_C = ((float)imuData.temperature / 333.87f) + 21.0f;
    }
}

/**
 * @brief Display IMU data on serial monitor
 */
void DisplayImuData(void) {
    uint8 buffer[32];
    
    Debug_Print((const uint8*)"\r\n========================================\r\n");
    Debug_Print((const uint8*)"MPU-9250 IMU Data\r\n");
    Debug_Print((const uint8*)"========================================\r\n");
    
    /* Accelerometer */
    Debug_Print((const uint8*)"Accelerometer (g):\r\n");
    Debug_Print((const uint8*)"  X: ");
    FloatToString(imuData.accelX_g, buffer);
    Debug_Print(buffer);
    Debug_Print((const uint8*)" g\r\n");
    
    Debug_Print((const uint8*)"  Y: ");
    FloatToString(imuData.accelY_g, buffer);
    Debug_Print(buffer);
    Debug_Print((const uint8*)" g\r\n");
    
    Debug_Print((const uint8*)"  Z: ");
    FloatToString(imuData.accelZ_g, buffer);
    Debug_Print(buffer);
    Debug_Print((const uint8*)" g\r\n\r\n");
    
    /* Gyroscope */
    Debug_Print((const uint8*)"Gyroscope (°/s):\r\n");
    Debug_Print((const uint8*)"  X: ");
    FloatToString(imuData.gyroX_dps, buffer);
    Debug_Print(buffer);
    Debug_Print((const uint8*)" °/s\r\n");
    
    Debug_Print((const uint8*)"  Y: ");
    FloatToString(imuData.gyroY_dps, buffer);
    Debug_Print(buffer);
    Debug_Print((const uint8*)" °/s\r\n");
    
    Debug_Print((const uint8*)"  Z: ");
    FloatToString(imuData.gyroZ_dps, buffer);
    Debug_Print(buffer);
    Debug_Print((const uint8*)" °/s\r\n\r\n");
    
    /* Temperature */
    Debug_Print((const uint8*)"Temperature: ");
    FloatToString(imuData.temp_C, buffer);
    Debug_Print(buffer);
    Debug_Print((const uint8*)" °C\r\n");
    
    Debug_Print((const uint8*)"========================================\r\n");
}

/* ===================[Main Function]=================== */
int main(void) {
    uint32 loopCounter = 0;
    
    /* Step 1: Initialize GPIO */
    Gpio_Init(&Gpio_Configuration);
    
    /* Step 2: Configure Debug UART */
    const Uart_ConfigType DebugUart_Config = {
        .Module = DEBUG_UART_MODULE,
        .BaudRate = DEBUG_BAUD_RATE,
        .DataBits = UART_DATA_BITS_8,
        .Parity = UART_PARITY_NONE,
        .StopBits = UART_STOP_BITS_1,
        .FlowControl = UART_FLOW_CONTROL_NONE,
        .FifoEnable = TRUE,
        .RxInterruptEnable = FALSE,
        .TxInterruptEnable = FALSE,
        .RxCallback = NULL_PTR,
        .TxCallback = NULL_PTR
    };
    Uart_Init(&DebugUart_Config);
    
    /* Step 3: Configure I2C for IMU */
    const I2C_ConfigType I2C_Config = {
        .Module = IMU_I2C_MODULE,
        .Mode = I2C_MODE_MASTER,
        .Speed = I2C_SPEED_STANDARD  /* 100 kHz */
    };
    I2C_Init(&I2C_Config);
    
    /* Small delay for peripherals to stabilize */
    {
        volatile uint32 delay;
        for (delay = 0; delay < 100000; delay++) { }
    }
    
    /* Send welcome message */
    Debug_Print((const uint8*)"\r\n\r\n");
    Debug_Print((const uint8*)"========================================\r\n");
    Debug_Print((const uint8*)"  MPU-9250 IMU Test - TM4C123GH6PM    \r\n");
    Debug_Print((const uint8*)"========================================\r\n");
    Debug_Print((const uint8*)"I2C0: PB2 (SCL), PB3 (SDA)\r\n");
    Debug_Print((const uint8*)"Debug UART: UART0 @ 115200 baud\r\n");
    Debug_Print((const uint8*)"\r\n");
    
    /* Step 4: Initialize MPU-9250 */
    if (MPU9250_Init() != E_OK) {
        Debug_Print((const uint8*)"[FATAL] MPU-9250 initialization failed!\r\n");
        Debug_Print((const uint8*)"Check I2C connections and power.\r\n");
        while (1) {
            /* Halt on error */
        }
    }
    
    Debug_Print((const uint8*)"Reading IMU data every 1 second...\r\n\r\n");
    
    /* Main loop */
    while (1) {
        /* Read sensor data */
        MPU9250_ReadAccelGyro();
        
        /* Display data */
        DisplayImuData();
        
        /* Delay ~1 second */
        {
            volatile uint32 delay;
            for (delay = 0; delay < 1000000; delay++) { }
        }
        
        loopCounter++;
    }
    
    /* Never reached */
}

#endif  /* <<<< END OF COMMENTED OUT IMU TEST CODE >>>> */
