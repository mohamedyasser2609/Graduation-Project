/**
 * @file I2C.h
 * @brief I2C Driver for TM4C123GH6PM
 * @details Full-featured I2C driver with master/slave mode support
 *
 * Features:
 * - Master and Slave mode
 * - All 4 I2C modules (I2C0-I2C3)
 * - Standard (100kHz) and Fast (400kHz) modes
 * - Multi-byte read/write
 * - Register read/write helpers
 * - Interrupt support
 * - Error handling and status checking
 *
 * @author Mohamed Yasser
 * @date Nov 1, 2025
 * @version 2.0.0
 */

#ifndef MCAL_I2C_I2C_H_
#define MCAL_I2C_I2C_H_

#include "../../CONFIG/Std_Types.h"
#include "I2C_Types.h"
#include "I2C_Cfg.h"

/* ===================[Function Prototypes]=================== */

/**
 * @brief Initialize I2C module
 * @param ConfigPtr Pointer to I2C configuration
 */
void I2C_Init(const I2C_ConfigType* ConfigPtr);

/**
 * @brief Transmit data to I2C slave
 * @param Module I2C module number
 * @param SlaveAddr 7-bit slave address
 * @param Data Pointer to data buffer
 * @param Length Number of bytes to transmit
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType I2C_MasterTransmit(I2C_ModuleType Module, uint8 SlaveAddr, const uint8* Data, uint8 Length);

/**
 * @brief Receive data from I2C slave
 * @param Module I2C module number
 * @param SlaveAddr 7-bit slave address
 * @param Data Pointer to receive buffer
 * @param Length Number of bytes to receive
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType I2C_MasterReceive(I2C_ModuleType Module, uint8 SlaveAddr, uint8* Data, uint8 Length);

/**
 * @brief Write to a specific register of an I2C slave device
 * @param Module I2C module number
 * @param SlaveAddr 7-bit slave address
 * @param RegAddr Register address
 * @param Data Pointer to data buffer
 * @param Length Number of bytes to write
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType I2C_WriteRegister(I2C_ModuleType Module, uint8 SlaveAddr, uint8 RegAddr, const uint8* Data, uint8 Length);

/**
 * @brief Read from a specific register of an I2C slave device
 * @param Module I2C module number
 * @param SlaveAddr 7-bit slave address
 * @param RegAddr Register address
 * @param Data Pointer to receive buffer
 * @param Length Number of bytes to read
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType I2C_ReadRegister(I2C_ModuleType Module, uint8 SlaveAddr, uint8 RegAddr, uint8* Data, uint8 Length);

/**
 * @brief Get current I2C bus status
 * @param Module I2C module number
 * @return Current status
 */
I2C_StatusType I2C_GetStatus(I2C_ModuleType Module);

/**
 * @brief Check if I2C bus is busy
 * @param Module I2C module number
 * @return TRUE if busy, FALSE if idle
 */
boolean I2C_IsBusy(I2C_ModuleType Module);

/**
 * @brief Perform I2C bus reset
 * @param Module I2C module number
 */
void I2C_Reset(I2C_ModuleType Module);

/**
 * @brief Scan I2C bus for devices
 * @param Module I2C module number
 * @param FoundAddresses Array to store found addresses
 * @param MaxDevices Maximum number of devices to find
 * @return Number of devices found
 */
uint8 I2C_ScanBus(I2C_ModuleType Module, uint8* FoundAddresses, uint8 MaxDevices);

/**
 * @brief De-initialize I2C module
 * @param Module I2C module number
 */
void I2C_DeInit(I2C_ModuleType Module);

#endif /* MCAL_I2C_I2C_H_ */
