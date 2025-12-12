/*
 * @file Uart.c
 * @brief UART Driver Implementation for TM4C123GH6PM
 * @details Complete implementation of UART driver with polling and interrupt support.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

/* ===================[Includes]=================== */
#include "Uart.h"
#include "Uart_Regs.h"
#include "Uart_Cfg.h"
#include "../../CONFIG/Std_Types.h"
#include "../MCU/Mcu.h"  /* For Mcu_GetSystemClock() */

/* ===================[Local Defines]=================== */
#define UART_INITIALIZED_FLAG     (0xA5u)
#define UART_NOT_INITIALIZED      (0x00u)

/* ===================[Local Types]=================== */
/**
 * @brief UART runtime state structure
 */
typedef struct {
    uint8 InitializedFlag;              /**< Initialization status */
    Uart_StateType State;               /**< Current state */
    Uart_NotificationFuncPtr RxCallback; /**< RX callback */
    Uart_NotificationFuncPtr TxCallback; /**< TX callback */
} Uart_RuntimeStateType;

/* ===================[Local Variables]=================== */
/**
 * @brief Runtime state for all UART modules
 */
static Uart_RuntimeStateType Uart_RuntimeState[8];

/**
 * @brief Base addresses for UART modules
 */
static const uint32 Uart_BaseAddress[8] = {
    UART0_BASE_ADDRESS,
    UART1_BASE_ADDRESS,
    UART2_BASE_ADDRESS,
    UART3_BASE_ADDRESS,
    UART4_BASE_ADDRESS,
    UART5_BASE_ADDRESS,
    UART6_BASE_ADDRESS,
    UART7_BASE_ADDRESS
};

/* ===================[Local Function Prototypes]=================== */
static boolean Uart_IsModuleValid(Uart_ModuleType Module);
static boolean Uart_IsInitialized(Uart_ModuleType Module);
static void Uart_EnableClock(Uart_ModuleType Module);
static uint32 Uart_GetBaseAddress(Uart_ModuleType Module);
static void Uart_ConfigureBaudRate(uint32 baseAddr, Uart_BaudRateType baudRate);

/* ===================[Local Function Implementations]=================== */

/**
 * @brief Validate UART module
 */
static boolean Uart_IsModuleValid(Uart_ModuleType Module) {
    return (Module <= UART_MODULE_7) ? TRUE : FALSE;
}

/**
 * @brief Check if UART is initialized
 */
static boolean Uart_IsInitialized(Uart_ModuleType Module) {
    return (Uart_RuntimeState[Module].InitializedFlag == UART_INITIALIZED_FLAG) ? TRUE : FALSE;
}

/**
 * @brief Enable clock for UART module
 */
static void Uart_EnableClock(Uart_ModuleType Module) {
    SYSCTL_RCGCUART |= (1u << Module);
    
    /* Wait for UART to be ready */
    volatile uint32 delay = SYSCTL_PRUART;
    (void)delay;
}

/**
 * @brief Get base address of UART module
 */
static uint32 Uart_GetBaseAddress(Uart_ModuleType Module) {
    return Uart_BaseAddress[Module];
}

/**
 * @brief Configure baud rate
 * @details Uses formula: BRD = BRDI + BRDF = UARTSysClk / (ClkDiv * Baud Rate)
 *          Where ClkDiv = 16 for standard, 8 for high-speed
 *          Dynamically queries system clock from MCU driver for accuracy
 */
static void Uart_ConfigureBaudRate(uint32 baseAddr, Uart_BaudRateType baudRate) {
    /* Get current system clock frequency from MCU driver */
    uint32 systemClock = Mcu_GetSystemClock();
    
    /* Calculate baud rate divisor */
    /* BRD = SystemClock / (16 * baudRate) */
    uint32 brd = (systemClock * 8) / baudRate;  /* Multiply by 8 for 0.5 precision */
    uint32 ibrd = brd / 128;  /* Integer part */
    uint32 fbrd = ((brd % 128) + 1) / 2;  /* Fractional part (round to nearest) */
    
    /* Write baud rate divisors */
    UART_REG(baseAddr, UARTIBRD_OFFSET) = ibrd;
    UART_REG(baseAddr, UARTFBRD_OFFSET) = fbrd;
}

/* ===================[API Function Implementations]=================== */

void Uart_Init(const Uart_ConfigType* ConfigPtr) {
#if (UART_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_INIT_SID, UART_E_PARAM_POINTER);
        return;
    }
    
    if (Uart_IsModuleValid(ConfigPtr->Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_INIT_SID, UART_E_PARAM_MODULE);
        return;
    }
    
    if (Uart_IsInitialized(ConfigPtr->Module) == TRUE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_INIT_SID, UART_E_ALREADY_INITIALIZED);
        return;
    }
#endif

    /* Enable clock */
    Uart_EnableClock(ConfigPtr->Module);
    
    uint32 baseAddr = Uart_GetBaseAddress(ConfigPtr->Module);
    
    /* Disable UART while configuring */
    UART_REG(baseAddr, UARTCTL_OFFSET) &= ~UARTCTL_UARTEN;
    
    /* Configure baud rate */
    Uart_ConfigureBaudRate(baseAddr, ConfigPtr->BaudRate);
    
    /* Configure line control (data bits, parity, stop bits, FIFO) */
    uint32 lcrh = 0;
    
    /* Data bits */
    lcrh |= (ConfigPtr->DataBits << 5);  /* WLEN field */
    
    /* Parity */
    if (ConfigPtr->Parity != UART_PARITY_NONE) {
        lcrh |= UARTLCRH_PEN;  /* Enable parity */
        if (ConfigPtr->Parity == UART_PARITY_EVEN) {
            lcrh |= UARTLCRH_EPS;  /* Even parity */
        }
        if (ConfigPtr->Parity == UART_PARITY_MARK || ConfigPtr->Parity == UART_PARITY_SPACE) {
            lcrh |= UARTLCRH_SPS;  /* Stick parity */
            if (ConfigPtr->Parity == UART_PARITY_SPACE) {
                lcrh |= UARTLCRH_EPS;
            }
        }
    }
    
    /* Stop bits */
    if (ConfigPtr->StopBits == UART_STOP_BITS_2) {
        lcrh |= UARTLCRH_STP2;
    }
    
    /* FIFO enable */
    if (ConfigPtr->FifoEnable == TRUE) {
        lcrh |= UARTLCRH_FEN;
    }
    
    UART_REG(baseAddr, UARTLCRH_OFFSET) = lcrh;
    
    /* Configure interrupts */
    if (ConfigPtr->RxInterruptEnable == TRUE) {
        UART_REG(baseAddr, UARTIM_OFFSET) |= UARTIM_RXIM;
    }
    if (ConfigPtr->TxInterruptEnable == TRUE) {
        UART_REG(baseAddr, UARTIM_OFFSET) |= UARTIM_TXIM;
    }
    
    /* Enable UART, TX, and RX */
    uint32 ctl = UARTCTL_UARTEN | UARTCTL_TXE | UARTCTL_RXE;
    
    /* Flow control */
    if (ConfigPtr->FlowControl == UART_FLOW_CONTROL_RTS || 
        ConfigPtr->FlowControl == UART_FLOW_CONTROL_RTS_CTS) {
        ctl |= UARTCTL_RTSEN;
    }
    if (ConfigPtr->FlowControl == UART_FLOW_CONTROL_CTS || 
        ConfigPtr->FlowControl == UART_FLOW_CONTROL_RTS_CTS) {
        ctl |= UARTCTL_CTSEN;
    }
    
    UART_REG(baseAddr, UARTCTL_OFFSET) = ctl;
    
    /* Save runtime state */
    Uart_RuntimeState[ConfigPtr->Module].InitializedFlag = UART_INITIALIZED_FLAG;
    Uart_RuntimeState[ConfigPtr->Module].State = UART_STATE_IDLE;
    Uart_RuntimeState[ConfigPtr->Module].RxCallback = ConfigPtr->RxCallback;
    Uart_RuntimeState[ConfigPtr->Module].TxCallback = ConfigPtr->TxCallback;
}

void Uart_DeInit(Uart_ModuleType Module) {
#if (UART_DEV_ERROR_DETECT == STD_ON)
    if (Uart_IsModuleValid(Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_DEINIT_SID, UART_E_PARAM_MODULE);
        return;
    }
    
    if (Uart_IsInitialized(Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_DEINIT_SID, UART_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Uart_GetBaseAddress(Module);
    
    /* Disable UART */
    UART_REG(baseAddr, UARTCTL_OFFSET) &= ~UARTCTL_UARTEN;
    
    /* Clear interrupts */
    UART_REG(baseAddr, UARTIM_OFFSET) = 0;
    
    /* Clear runtime state */
    Uart_RuntimeState[Module].InitializedFlag = UART_NOT_INITIALIZED;
    Uart_RuntimeState[Module].State = UART_STATE_UNINIT;
    Uart_RuntimeState[Module].RxCallback = NULL_PTR;
    Uart_RuntimeState[Module].TxCallback = NULL_PTR;
}

Std_ReturnType Uart_SendByte(Uart_ModuleType Module, uint8 Data) {
#if (UART_DEV_ERROR_DETECT == STD_ON)
    if (Uart_IsModuleValid(Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_SEND_BYTE_SID, UART_E_PARAM_MODULE);
        return E_NOT_OK;
    }
    
    if (Uart_IsInitialized(Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_SEND_BYTE_SID, UART_E_UNINIT);
        return E_NOT_OK;
    }
#endif

    uint32 baseAddr = Uart_GetBaseAddress(Module);
    
    /* Wait until TX FIFO is not full */
    volatile uint32 timeout = UART_TX_TIMEOUT_US;
    while ((UART_REG(baseAddr, UARTFR_OFFSET) & UARTFR_TXFF) && (timeout > 0)) {
        timeout--;
    }
    
    if (timeout == 0) {
        return E_NOT_OK;  /* Timeout */
    }
    
    /* Write data */
    UART_REG(baseAddr, UARTDR_OFFSET) = Data;
    
    return E_OK;
}

Std_ReturnType Uart_ReceiveByte(Uart_ModuleType Module, uint8* DataPtr) {
#if (UART_DEV_ERROR_DETECT == STD_ON)
    if (Uart_IsModuleValid(Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_RECEIVE_BYTE_SID, UART_E_PARAM_MODULE);
        return E_NOT_OK;
    }
    
    if (DataPtr == NULL_PTR) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_RECEIVE_BYTE_SID, UART_E_PARAM_POINTER);
        return E_NOT_OK;
    }
    
    if (Uart_IsInitialized(Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_RECEIVE_BYTE_SID, UART_E_UNINIT);
        return E_NOT_OK;
    }
#endif

    uint32 baseAddr = Uart_GetBaseAddress(Module);
    
    /* Wait until RX FIFO is not empty */
    volatile uint32 timeout = UART_RX_TIMEOUT_US;
    while ((UART_REG(baseAddr, UARTFR_OFFSET) & UARTFR_RXFE) && (timeout > 0)) {
        timeout--;
    }
    
    if (timeout == 0) {
        return E_NOT_OK;  /* Timeout */
    }
    
    /* Read data */
    uint32 data = UART_REG(baseAddr, UARTDR_OFFSET);
    
    /* Check for errors */
    if (data & (UARTDR_OE | UARTDR_BE | UARTDR_PE | UARTDR_FE)) {
        return E_NOT_OK;  /* Error occurred */
    }
    
    *DataPtr = (uint8)(data & UARTDR_DATA_MASK);
    
    return E_OK;
}

Std_ReturnType Uart_SendString(Uart_ModuleType Module, const uint8* StringPtr) {
#if (UART_DEV_ERROR_DETECT == STD_ON)
    if (Uart_IsModuleValid(Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_SEND_STRING_SID, UART_E_PARAM_MODULE);
        return E_NOT_OK;
    }
    
    if (StringPtr == NULL_PTR) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_SEND_STRING_SID, UART_E_PARAM_POINTER);
        return E_NOT_OK;
    }
    
    if (Uart_IsInitialized(Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_SEND_STRING_SID, UART_E_UNINIT);
        return E_NOT_OK;
    }
#endif

    uint16 index = 0;
    
    /* Transmit until null terminator or max length */
    while ((StringPtr[index] != '\0') && (index < UART_MAX_STRING_LENGTH)) {
        if (Uart_SendByte(Module, StringPtr[index]) != E_OK) {
            return E_NOT_OK;
        }
        index++;
    }
    
    return E_OK;
}

Std_ReturnType Uart_SendBuffer(Uart_ModuleType Module, const uint8* BufferPtr, uint16 Length) {
    uint16 i;
    
#if (UART_DEV_ERROR_DETECT == STD_ON)
    if (Uart_IsModuleValid(Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_SEND_BUFFER_SID, UART_E_PARAM_MODULE);
        return E_NOT_OK;
    }
    
    if (BufferPtr == NULL_PTR) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_SEND_BUFFER_SID, UART_E_PARAM_POINTER);
        return E_NOT_OK;
    }
    
    if (Length == 0) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_SEND_BUFFER_SID, UART_E_PARAM_VALUE);
        return E_NOT_OK;
    }
    
    if (Uart_IsInitialized(Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_SEND_BUFFER_SID, UART_E_UNINIT);
        return E_NOT_OK;
    }
#endif

    for (i = 0; i < Length; i++) {
        if (Uart_SendByte(Module, BufferPtr[i]) != E_OK) {
            return E_NOT_OK;
        }
    }
    
    return E_OK;
}

Uart_StateType Uart_GetStatus(Uart_ModuleType Module) {
#if (UART_DEV_ERROR_DETECT == STD_ON)
    if (Uart_IsModuleValid(Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_GET_STATUS_SID, UART_E_PARAM_MODULE);
        return UART_STATE_UNINIT;
    }
    
    if (Uart_IsInitialized(Module) == FALSE) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_GET_STATUS_SID, UART_E_UNINIT);
        return UART_STATE_UNINIT;
    }
#endif

    return Uart_RuntimeState[Module].State;
}

boolean Uart_IsTxFifoEmpty(Uart_ModuleType Module) {
    if (Uart_IsModuleValid(Module) == FALSE || Uart_IsInitialized(Module) == FALSE) {
        return FALSE;
    }
    
    uint32 baseAddr = Uart_GetBaseAddress(Module);
    return ((UART_REG(baseAddr, UARTFR_OFFSET) & UARTFR_TXFE) != 0) ? TRUE : FALSE;
}

boolean Uart_IsRxDataAvailable(Uart_ModuleType Module) {
    if (Uart_IsModuleValid(Module) == FALSE || Uart_IsInitialized(Module) == FALSE) {
        return FALSE;
    }
    
    uint32 baseAddr = Uart_GetBaseAddress(Module);
    return ((UART_REG(baseAddr, UARTFR_OFFSET) & UARTFR_RXFE) == 0) ? TRUE : FALSE;
}

#if (UART_VERSION_INFO_API == STD_ON)
void Uart_GetVersionInfo(Std_VersionInfoType* VersionInfo) {
#if (UART_DEV_ERROR_DETECT == STD_ON)
    if (VersionInfo == NULL_PTR) {
        Det_ReportError(UART_MODULE_ID, UART_INSTANCE_ID, UART_GET_VERSION_INFO_SID, UART_E_PARAM_POINTER);
        return;
    }
#endif

    VersionInfo->vendorID = UART_VENDOR_ID;
    VersionInfo->moduleID = UART_MODULE_ID;
    VersionInfo->sw_major_version = UART_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = UART_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = UART_SW_PATCH_VERSION;
}
#endif




