/**
 * @file I2C.c
 * @brief I2C Driver Implementation for TM4C123GH6PM
 * @details Full-featured I2C driver supporting all 4 modules
 *
 * @author Mohamed Yasser
 * @date Nov 1, 2025
 * @version 2.0.0
 */

#include "I2C.h"
#include "tm4c123gh6pm.h"
#include "../MCU/Mcu.h"  /* For Mcu_GetSystemClock() */

/* ===================[I2C Register Structure]=================== */
typedef struct {
    volatile unsigned long* MSA;   /* Master Slave Address */
    volatile unsigned long* MCS;   /* Master Control/Status */
    volatile unsigned long* MDR;   /* Master Data */
    volatile unsigned long* MTPR;  /* Master Timer Period */
    volatile unsigned long* MCR;   /* Master Configuration */
} I2C_RegistersType;

/* ===================[I2C Base Addresses]=================== */
static const I2C_RegistersType I2C_Registers[4] = {
    /* I2C0 */
    {
        .MSA  = &I2C0_MSA_R,
        .MCS  = &I2C0_MCS_R,
        .MDR  = &I2C0_MDR_R,
        .MTPR = &I2C0_MTPR_R,
        .MCR  = &I2C0_MCR_R
    },
    /* I2C1 */
    {
        .MSA  = &I2C1_MSA_R,
        .MCS  = &I2C1_MCS_R,
        .MDR  = &I2C1_MDR_R,
        .MTPR = &I2C1_MTPR_R,
        .MCR  = &I2C1_MCR_R
    },
    /* I2C2 */
    {
        .MSA  = &I2C2_MSA_R,
        .MCS  = &I2C2_MCS_R,
        .MDR  = &I2C2_MDR_R,
        .MTPR = &I2C2_MTPR_R,
        .MCR  = &I2C2_MCR_R
    },
    /* I2C3 */
    {
        .MSA  = &I2C3_MSA_R,
        .MCS  = &I2C3_MCS_R,
        .MDR  = &I2C3_MDR_R,
        .MTPR = &I2C3_MTPR_R,
        .MCR  = &I2C3_MCR_R
    }
};

/* ===================[Static Helper Functions]=================== */

/**
 * @brief Wait for I2C bus to be ready
 */
static Std_ReturnType I2C_WaitBusy(I2C_ModuleType Module) {
    uint32 timeout = 100000;
    const I2C_RegistersType* regs = &I2C_Registers[Module];
    
    while ((*regs->MCS & I2C_MCS_BUSY) && (timeout > 0)) {
        timeout--;
    }
    
    return (timeout > 0) ? E_OK : E_NOT_OK;
}

/**
 * @brief Check for I2C errors
 */
static Std_ReturnType I2C_CheckError(I2C_ModuleType Module) {
    const I2C_RegistersType* regs = &I2C_Registers[Module];
    uint32 status = *regs->MCS;
    
    if (status & I2C_MCS_ERROR) {
        /* Clear error by sending STOP */
        *regs->MCS = I2C_MCS_STOP;
        return E_NOT_OK;
    }
    
    return E_OK;
}

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize I2C module
 */
void I2C_Init(const I2C_ConfigType* ConfigPtr) {
    uint32 tpr;
    volatile uint32 delay;
    const I2C_RegistersType* regs;
    
    if (ConfigPtr == NULL_PTR || ConfigPtr->Module > I2C_MODULE_3) {
        return;
    }
    
    regs = &I2C_Registers[ConfigPtr->Module];
    
    /* Enable I2C clock */
    SYSCTL_RCGCI2C_R |= (1 << ConfigPtr->Module);
    
    /* Wait for peripheral to be ready */
    delay = 100;
    while (delay > 0) delay--;
    
    /* Disable I2C before configuration */
    *regs->MCR = 0;
    
    /* Configure I2C clock speed */
    /* TPR = (System Clock / (2 * (SCL_LP + SCL_HP) * SCL_CLK)) - 1 */
    /* SCL_LP + SCL_HP = 10 (default) */
    /* For 100kHz @ 80MHz: TPR = (80MHz / (2 * 10 * 100kHz)) - 1 = 39 */
    /* For 400kHz @ 80MHz: TPR = (80MHz / (2 * 10 * 400kHz)) - 1 = 9 */
    /* For 100kHz @ 16MHz: TPR = (16MHz / (2 * 10 * 100kHz)) - 1 = 7 */
    /* For 400kHz @ 16MHz: TPR = (16MHz / (2 * 10 * 400kHz)) - 1 = 1 */
    
    /* Get actual system clock from MCU driver */
    uint32 sysClock = Mcu_GetSystemClock();
    
    if (ConfigPtr->Speed == I2C_SPEED_STANDARD) {
        /* 100 kHz */
        tpr = (sysClock / (2 * 10 * 100000)) - 1;
        if (tpr < 1) tpr = 1;  /* Minimum value */
    } else {
        /* 400 kHz */
        tpr = (sysClock / (2 * 10 * 400000)) - 1;
        if (tpr < 1) tpr = 1;  /* Minimum value */
    }
    
    *regs->MTPR = tpr;
    
    /* Enable I2C master */
    *regs->MCR = I2C_MCR_MFE;
}

/**
 * @brief Transmit data to I2C slave
 */
Std_ReturnType I2C_MasterTransmit(I2C_ModuleType Module, uint8 SlaveAddr, const uint8* Data, uint8 Length) {
    uint8 i;
    const I2C_RegistersType* regs;
    
    if (Data == NULL_PTR || Length == 0 || Module > I2C_MODULE_3) {
        return E_NOT_OK;
    }
    
    regs = &I2C_Registers[Module];
    
    /* Wait for bus to be ready */
    if (I2C_WaitBusy(Module) != E_OK) {
        return E_NOT_OK;
    }
    
    /* Set slave address for write */
    *regs->MSA = (SlaveAddr << 1) | 0x00;  /* R/S = 0 for write */
    
    /* Send first byte */
    *regs->MDR = Data[0];
    
    if (Length == 1) {
        /* Single byte transmission */
        *regs->MCS = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;
    } else {
        /* Multi-byte transmission - start */
        *regs->MCS = I2C_MCS_START | I2C_MCS_RUN;
    }
    
    /* Wait and check for errors */
    if (I2C_WaitBusy(Module) != E_OK) {
        return E_NOT_OK;
    }
    if (I2C_CheckError(Module) != E_OK) {
        return E_NOT_OK;
    }
    
    /* Send remaining bytes */
    for (i = 1; i < Length; i++) {
        *regs->MDR = Data[i];
        
        if (i == (Length - 1)) {
            /* Last byte - send STOP */
            *regs->MCS = I2C_MCS_RUN | I2C_MCS_STOP;
        } else {
            /* Continue transmission */
            *regs->MCS = I2C_MCS_RUN;
        }
        
        /* Wait and check for errors */
        if (I2C_WaitBusy(Module) != E_OK) {
            return E_NOT_OK;
        }
        if (I2C_CheckError(Module) != E_OK) {
            return E_NOT_OK;
        }
    }
    
    return E_OK;
}

/**
 * @brief Receive data from I2C slave
 */
Std_ReturnType I2C_MasterReceive(I2C_ModuleType Module, uint8 SlaveAddr, uint8* Data, uint8 Length) {
    uint8 i;
    const I2C_RegistersType* regs;
    
    if (Data == NULL_PTR || Length == 0 || Module > I2C_MODULE_3) {
        return E_NOT_OK;
    }
    
    regs = &I2C_Registers[Module];
    
    /* Wait for bus to be ready */
    if (I2C_WaitBusy(Module) != E_OK) {
        return E_NOT_OK;
    }
    
    /* Set slave address for read */
    *regs->MSA = (SlaveAddr << 1) | 0x01;  /* R/S = 1 for read */
    
    if (Length == 1) {
        /* Single byte reception */
        *regs->MCS = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;
        
        /* Wait and check for errors */
        if (I2C_WaitBusy(Module) != E_OK) {
            return E_NOT_OK;
        }
        if (I2C_CheckError(Module) != E_OK) {
            return E_NOT_OK;
        }
        
        /* Read data */
        Data[0] = (uint8)*regs->MDR;
    } else {
        /* Multi-byte reception - start */
        *regs->MCS = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_ACK;
        
        /* Wait and check for errors */
        if (I2C_WaitBusy(Module) != E_OK) {
            return E_NOT_OK;
        }
        if (I2C_CheckError(Module) != E_OK) {
            return E_NOT_OK;
        }
        
        /* Read first byte */
        Data[0] = (uint8)*regs->MDR;
        
        /* Read middle bytes */
        for (i = 1; i < (Length - 1); i++) {
            *regs->MCS = I2C_MCS_RUN | I2C_MCS_ACK;
            
            /* Wait and check for errors */
            if (I2C_WaitBusy(Module) != E_OK) {
                return E_NOT_OK;
            }
            if (I2C_CheckError(Module) != E_OK) {
                return E_NOT_OK;
            }
            
            Data[i] = (uint8)*regs->MDR;
        }
        
        /* Read last byte with NACK and STOP */
        *regs->MCS = I2C_MCS_RUN | I2C_MCS_STOP;
        
        /* Wait and check for errors */
        if (I2C_WaitBusy(Module) != E_OK) {
            return E_NOT_OK;
        }
        if (I2C_CheckError(Module) != E_OK) {
            return E_NOT_OK;
        }
        
        Data[Length - 1] = (uint8)*regs->MDR;
    }
    
    return E_OK;
}

/**
 * @brief Write to a specific register of an I2C slave device
 */
Std_ReturnType I2C_WriteRegister(I2C_ModuleType Module, uint8 SlaveAddr, uint8 RegAddr, const uint8* Data, uint8 Length) {
    uint8 i;
    const I2C_RegistersType* regs;
    
    if (Data == NULL_PTR || Length == 0 || Module > I2C_MODULE_3) {
        return E_NOT_OK;
    }
    
    regs = &I2C_Registers[Module];
    
    /* Wait for bus to be ready */
    if (I2C_WaitBusy(Module) != E_OK) {
        return E_NOT_OK;
    }
    
    /* Set slave address for write */
    *regs->MSA = (SlaveAddr << 1) | 0x00;
    
    /* Send register address */
    *regs->MDR = RegAddr;
    *regs->MCS = I2C_MCS_START | I2C_MCS_RUN;
    
    if (I2C_WaitBusy(Module) != E_OK) {
        return E_NOT_OK;
    }
    if (I2C_CheckError(Module) != E_OK) {
        return E_NOT_OK;
    }
    
    /* Send data bytes */
    for (i = 0; i < Length; i++) {
        *regs->MDR = Data[i];
        
        if (i == (Length - 1)) {
            *regs->MCS = I2C_MCS_RUN | I2C_MCS_STOP;
        } else {
            *regs->MCS = I2C_MCS_RUN;
        }
        
        if (I2C_WaitBusy(Module) != E_OK) {
            return E_NOT_OK;
        }
        if (I2C_CheckError(Module) != E_OK) {
            return E_NOT_OK;
        }
    }
    
    return E_OK;
}

/**
 * @brief Read from a specific register of an I2C slave device
 * @details Implements proper I2C register read with repeated start
 */
Std_ReturnType I2C_ReadRegister(I2C_ModuleType Module, uint8 SlaveAddr, uint8 RegAddr, uint8* Data, uint8 Length) {
    uint8 i;
    const I2C_RegistersType* regs;
    
    if (Data == NULL_PTR || Length == 0 || Module > I2C_MODULE_3) {
        return E_NOT_OK;
    }
    
    regs = &I2C_Registers[Module];
    
    /* Wait for bus to be ready */
    if (I2C_WaitBusy(Module) != E_OK) {
        return E_NOT_OK;
    }
    
    /* Step 1: Write register address (no STOP for repeated start) */
    *regs->MSA = (SlaveAddr << 1) | 0x00;  /* Write mode */
    *regs->MDR = RegAddr;
    *regs->MCS = I2C_MCS_START | I2C_MCS_RUN;  /* No STOP - will do repeated start */
    
    /* Wait for register address to be sent */
    if (I2C_WaitBusy(Module) != E_OK) {
        return E_NOT_OK;
    }
    if (I2C_CheckError(Module) != E_OK) {
        return E_NOT_OK;
    }
    
    /* Step 2: Repeated start for read (bus should still be in transaction) */
    *regs->MSA = (SlaveAddr << 1) | 0x01;  /* Read mode */
    
    if (Length == 1) {
        /* Single byte read */
        *regs->MCS = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;
        
        if (I2C_WaitBusy(Module) != E_OK) {
            return E_NOT_OK;
        }
        if (I2C_CheckError(Module) != E_OK) {
            return E_NOT_OK;
        }
        
        Data[0] = (uint8)*regs->MDR;
    } else {
        /* Multi-byte read */
        *regs->MCS = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_ACK;
        
        if (I2C_WaitBusy(Module) != E_OK) {
            return E_NOT_OK;
        }
        if (I2C_CheckError(Module) != E_OK) {
            return E_NOT_OK;
        }
        
        /* Read first byte */
        Data[0] = (uint8)*regs->MDR;
        
        /* Read middle bytes */
        for (i = 1; i < (Length - 1); i++) {
            *regs->MCS = I2C_MCS_RUN | I2C_MCS_ACK;
            
            if (I2C_WaitBusy(Module) != E_OK) {
                return E_NOT_OK;
            }
            if (I2C_CheckError(Module) != E_OK) {
                return E_NOT_OK;
            }
            
            Data[i] = (uint8)*regs->MDR;
        }
        
        /* Read last byte with NACK and STOP */
        *regs->MCS = I2C_MCS_RUN | I2C_MCS_STOP;
    
    if (I2C_WaitBusy(Module) != E_OK) {
        return E_NOT_OK;
    }
    if (I2C_CheckError(Module) != E_OK) {
        return E_NOT_OK;
    }
    
        Data[Length - 1] = (uint8)*regs->MDR;
    }
    
    return E_OK;
}

/**
 * @brief Get current I2C bus status
 */
I2C_StatusType I2C_GetStatus(I2C_ModuleType Module) {
    const I2C_RegistersType* regs;
    uint32 status;
    
    if (Module > I2C_MODULE_3) {
        return I2C_STATUS_ERROR;
    }
    
    regs = &I2C_Registers[Module];
    status = *regs->MCS;
    
    if (status & I2C_MCS_BUSBSY) {
        return I2C_STATUS_BUSY;
    }
    if (status & I2C_MCS_ERROR) {
        if (status & I2C_MCS_ARBLST) {
            return I2C_STATUS_ARBITRATION_LOST;
        }
        if (status & I2C_MCS_DATACK) {
            return I2C_STATUS_DATA_NACK;
        }
        if (status & I2C_MCS_ADRACK) {
            return I2C_STATUS_ADDR_NACK;
        }
        return I2C_STATUS_ERROR;
    }
    
    return I2C_STATUS_IDLE;
}

/**
 * @brief Check if I2C bus is busy
 */
boolean I2C_IsBusy(I2C_ModuleType Module) {
    const I2C_RegistersType* regs;
    
    if (Module > I2C_MODULE_3) {
        return FALSE;
    }
    
    regs = &I2C_Registers[Module];
    return (*regs->MCS & I2C_MCS_BUSY) ? TRUE : FALSE;
}

/**
 * @brief Perform I2C bus reset
 */
void I2C_Reset(I2C_ModuleType Module) {
    const I2C_RegistersType* regs;
    
    if (Module > I2C_MODULE_3) {
        return;
    }
    
    regs = &I2C_Registers[Module];
    
    /* Disable and re-enable I2C master */
    *regs->MCR = 0;
    
    /* Small delay */
    {
        volatile uint32 delay = 100;
        while (delay > 0) delay--;
    }
    
    *regs->MCR = I2C_MCR_MFE;
}

/**
 * @brief Scan I2C bus for devices
 * @details Improved scan that properly checks for address acknowledgment
 */
uint8 I2C_ScanBus(I2C_ModuleType Module, uint8* FoundAddresses, uint8 MaxDevices) {
    uint8 addr;
    uint8 count = 0;
    const I2C_RegistersType* regs;
    uint32 status;
    
    if (FoundAddresses == NULL_PTR || MaxDevices == 0 || Module > I2C_MODULE_3) {
        return 0;
    }
    
    regs = &I2C_Registers[Module];
    
    /* Scan addresses 0x08 to 0x77 (valid 7-bit addresses) */
    for (addr = 0x08; addr < 0x78 && count < MaxDevices; addr++) {
        /* Wait for bus to be ready */
        if (I2C_WaitBusy(Module) != E_OK) {
            continue;  /* Skip this address if bus is stuck */
        }
        
        /* Set slave address for read */
        *regs->MSA = (addr << 1) | 0x01;  /* R/S = 1 for read */
        
        /* Try to read one byte */
        *regs->MCS = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;
        
        /* Wait for transaction to complete */
        if (I2C_WaitBusy(Module) != E_OK) {
            /* Clear any error */
            *regs->MCS = I2C_MCS_STOP;
            continue;
        }
        
        /* Check status - device exists if NO errors occurred
         * ADRACK bit is set when slave did NOT acknowledge (device not present)
         * Device found if: no ERROR and no ADRACK (address was ACKed) */
        status = *regs->MCS;
        if ((status & I2C_MCS_ERROR) == 0) {
            /* No error - device acknowledged the address */
            (void)*regs->MDR;  /* Read data to clear (value not used) */
            FoundAddresses[count++] = addr;
        } else {
            /* Error or NACK - no device at this address */
            *regs->MCS = I2C_MCS_STOP;  /* Clear error */
        }
        
        /* Small delay between scans */
        {
            volatile uint32 delay = 1000;
            while (delay > 0) delay--;
        }
    }
    
    return count;
}

/**
 * @brief De-initialize I2C module
 */
void I2C_DeInit(I2C_ModuleType Module) {
    const I2C_RegistersType* regs;
    
    if (Module > I2C_MODULE_3) {
        return;
    }
    
    regs = &I2C_Registers[Module];
    
    /* Disable I2C master */
    *regs->MCR = 0;
    
    /* Disable I2C clock */
    SYSCTL_RCGCI2C_R &= ~(1 << Module);
}
