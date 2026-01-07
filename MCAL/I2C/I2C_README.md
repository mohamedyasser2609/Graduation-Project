# I2C Driver Documentation

**TM4C123GH6PM Inter-Integrated Circuit Driver**

---

## Overview

Full-featured AUTOSAR-compliant I2C driver supporting all 4 I2C modules with master mode operation, multi-byte transfers, and device scanning.

### Features
- 4 I2C modules (I2C0-I2C3)
- Master mode with 7-bit addressing
- Standard (100kHz) and Fast (400kHz) modes
- Multi-byte read/write operations
- Register-level read/write helpers
- Bus scanning for device discovery
- Timeout protection
- Error detection and handling

---

## Hardware Configuration

### Pin Mapping

| Module | SCL Pin | SDA Pin |
|--------|---------|---------|
| I2C0 | PB2 | PB3 |
| I2C1 | PA6 | PA7 |
| I2C2 | PE4 | PE5 |
| I2C3 | PD0 | PD1 |

### Hardware Requirements
- **Pull-up resistors**: 4.7kΩ on SCL and SDA lines
- Most I2C breakout boards include built-in pull-ups

---

## API Reference

### Initialization

```c
/* Initialize I2C module */
void I2C_Init(const I2C_ConfigType* ConfigPtr);

/* De-initialize I2C module */
void I2C_DeInit(I2C_ModuleType Module);
```

### Data Transfer

```c
/* Transmit data to slave */
Std_ReturnType I2C_MasterTransmit(I2C_ModuleType Module, uint8 SlaveAddr, 
                                   const uint8* Data, uint8 Length);

/* Receive data from slave */
Std_ReturnType I2C_MasterReceive(I2C_ModuleType Module, uint8 SlaveAddr, 
                                  uint8* Data, uint8 Length);
```

### Register Operations

```c
/* Write to device register */
Std_ReturnType I2C_WriteRegister(I2C_ModuleType Module, uint8 SlaveAddr, 
                                  uint8 RegAddr, const uint8* Data, uint8 Length);

/* Read from device register */
Std_ReturnType I2C_ReadRegister(I2C_ModuleType Module, uint8 SlaveAddr, 
                                 uint8 RegAddr, uint8* Data, uint8 Length);
```

### Status and Utilities

```c
/* Get I2C bus status */
I2C_StatusType I2C_GetStatus(I2C_ModuleType Module);

/* Check if bus is busy */
boolean I2C_IsBusy(I2C_ModuleType Module);

/* Reset I2C bus */
void I2C_Reset(I2C_ModuleType Module);

/* Scan for connected devices */
uint8 I2C_ScanBus(I2C_ModuleType Module, uint8* FoundAddresses, uint8 MaxDevices);
```

---

## Usage Examples

### Basic Read/Write

```c
#include "MCAL/I2C/I2C.h"

#define MPU6050_ADDR  0x68
#define WHO_AM_I_REG  0x75

void main(void) {
    uint8 deviceId;
    
    /* Initialize I2C0 */
    I2C_Init(&I2C_Configuration);
    
    /* Read device ID register */
    I2C_ReadRegister(I2C_MODULE_0, MPU6050_ADDR, WHO_AM_I_REG, &deviceId, 1);
    
    if (deviceId == 0x68) {
        /* MPU6050 detected */
    }
}
```

### Bus Scanning

```c
uint8 addresses[10];
uint8 count;

/* Scan for all connected I2C devices */
count = I2C_ScanBus(I2C_MODULE_0, addresses, 10);

for (uint8 i = 0; i < count; i++) {
    /* Process found device at addresses[i] */
}
```

---

## Common I2C Device Addresses

| Device | Address | Description |
|--------|---------|-------------|
| MPU6050/MPU9250 | 0x68 (0x69) | IMU sensor |
| BMP280/BME280 | 0x76 (0x77) | Pressure sensor |
| ADXL345 | 0x53 | Accelerometer |
| DS1307/DS3231 | 0x68 | Real-time clock |
| 24C02/24C32/24C256 | 0x50 | EEPROM |
| PCF8574 | 0x27 | LCD backpack |

---

## Troubleshooting

| Issue | Possible Cause | Solution |
|-------|---------------|----------|
| No ACK received | Wrong address | Verify address with `I2C_ScanBus()` |
| Timeout errors | Missing pull-ups | Add 4.7kΩ pull-up resistors |
| Bus stuck | SDA held low | Call `I2C_Reset()` |
| Intermittent failures | Long wires | Keep wires < 20cm |

---

## Version Information

| Item | Value |
|------|-------|
| Module ID | I2C |
| Driver Version | 2.0.0 |
| AUTOSAR Version | 4.4.0 |
