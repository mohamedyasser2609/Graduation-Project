/*
 * @file Timer.c
 * @brief Timer Driver Implementation for TM4C123GH6PM
 * @details Complete implementation of General Purpose Timer Module (GPTM) driver.
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

/* ===================[Includes]=================== */
#include "Timer.h"
#include "Timer_Regs.h"
#include "Timer_Cfg.h"
#include "../../CONFIG/Det.h"

/* ===================[Local Defines]=================== */
#define TIMER_INITIALIZED_FLAG    (0xA5u)  /**< Flag to check if timer is initialized */
#define TIMER_NOT_INITIALIZED     (0x00u)  /**< Flag for uninitialized timer */

/* ===================[Local Types]=================== */
/**
 * @brief Timer runtime state structure
 */
typedef struct {
    uint8 InitializedFlag;                  /**< Initialization status */
    Timer_NotificationFuncPtr TimeoutCb;    /**< Timeout callback */
    Timer_NotificationFuncPtr MatchCb;      /**< Match callback */
    Timer_ValueType LoadValue;              /**< Current load value */
    Timer_CountDirectionType CountDir;      /**< Count direction (up/down) */
} Timer_RuntimeStateType;

/* ===================[Local Variables]=================== */
/**
 * @brief Runtime state for all timer blocks
 * @details 6 modules × 2 blocks = 12 possible timers
 */
static Timer_RuntimeStateType Timer_RuntimeState[6][2];

/**
 * @brief Base addresses for timer modules
 */
static const uint32 Timer_BaseAddress[6] = {
    TIMER0_BASE_ADDRESS,
    TIMER1_BASE_ADDRESS,
    TIMER2_BASE_ADDRESS,
    TIMER3_BASE_ADDRESS,
    TIMER4_BASE_ADDRESS,
    TIMER5_BASE_ADDRESS
};

/* ===================[Local Function Prototypes]=================== */
static boolean Timer_IsModuleValid(Timer_ModuleType Module);
static boolean Timer_IsBlockValid(Timer_BlockType Block);
static boolean Timer_IsInitialized(Timer_ModuleType Module, Timer_BlockType Block);
static void Timer_EnableClock(Timer_ModuleType Module);
static uint32 Timer_GetBaseAddress(Timer_ModuleType Module);

/* ===================[Local Function Implementations]=================== */

/**
 * @brief Validate timer module
 */
static boolean Timer_IsModuleValid(Timer_ModuleType Module) {
    return (Module <= TIMER_MODULE_5) ? TRUE : FALSE;
}

/**
 * @brief Validate timer block
 */
static boolean Timer_IsBlockValid(Timer_BlockType Block) {
    return (Block <= TIMER_BLOCK_B) ? TRUE : FALSE;
}

/**
 * @brief Check if timer is initialized
 */
static boolean Timer_IsInitialized(Timer_ModuleType Module, Timer_BlockType Block) {
    return (Timer_RuntimeState[Module][Block].InitializedFlag == TIMER_INITIALIZED_FLAG) ? TRUE : FALSE;
}

/**
 * @brief Enable clock for timer module
 */
static void Timer_EnableClock(Timer_ModuleType Module) {
    /* Enable clock in run mode */
    SYSCTL_RCGCTIMER |= (1u << Module);
    
    /* Wait for timer to be ready (3 clock cycles) */
    volatile uint32 delay = SYSCTL_PRTIMER;
    (void)delay;  /* Suppress unused variable warning */
}

/**
 * @brief Get base address of timer module
 */
static uint32 Timer_GetBaseAddress(Timer_ModuleType Module) {
    return Timer_BaseAddress[Module];
}

/* ===================[API Function Implementations]=================== */

void Timer_Init(const Timer_ConfigType* ConfigPtr) {
#if (TIMER_DEV_ERROR_DETECT == STD_ON)
    /* Check for NULL pointer */
    if (ConfigPtr == NULL_PTR) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_INIT_SID, TIMER_E_PARAM_POINTER);
        return;
    }
    
    /* Validate module */
    if (Timer_IsModuleValid(ConfigPtr->Module) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_INIT_SID, TIMER_E_PARAM_MODULE);
        return;
    }
    
    /* Validate block */
    if (Timer_IsBlockValid(ConfigPtr->Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_INIT_SID, TIMER_E_PARAM_BLOCK);
        return;
    }
    
    /* Check if already initialized */
    if (Timer_IsInitialized(ConfigPtr->Module, ConfigPtr->Block) == TRUE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_INIT_SID, TIMER_E_ALREADY_INITIALIZED);
        return;
    }
#endif

    /* Enable clock for timer module */
    Timer_EnableClock(ConfigPtr->Module);
    
    /* Get base address */
    uint32 baseAddr = Timer_GetBaseAddress(ConfigPtr->Module);
    
    /* Disable timer before configuration */
    TIMER_REG(baseAddr, GPTMCTL_OFFSET) &= ~(ConfigPtr->Block == TIMER_BLOCK_A ? GPTMCTL_TAEN : GPTMCTL_TBEN);
    
    /* Configure timer mode (32-bit or 16-bit) */
    TIMER_REG(baseAddr, GPTMCFG_OFFSET) = ConfigPtr->ConfigMode;
    
    /* Configure operation mode for the selected block */
    uint32 modeReg = 0;
    modeReg |= ConfigPtr->OperationMode;  /* Set mode (one-shot, periodic, capture) */
    
    if (ConfigPtr->PwmMode == TIMER_PWM_ENABLED) {
        modeReg |= GPTMTMR_AMS;  /* Enable PWM mode */
    }
    
    if (ConfigPtr->CountDirection == TIMER_COUNT_UP) {
        modeReg |= GPTMTMR_CDIR;  /* Count up */
    }
    
    /* Write mode register */
    if (ConfigPtr->Block == TIMER_BLOCK_A) {
        TIMER_REG(baseAddr, GPTMTAMR_OFFSET) = modeReg;
    } else {
        TIMER_REG(baseAddr, GPTMTBMR_OFFSET) = modeReg;
    }
    
    /* Set load value */
    if (ConfigPtr->Block == TIMER_BLOCK_A) {
        TIMER_REG(baseAddr, GPTMTAILR_OFFSET) = ConfigPtr->LoadValue;
    } else {
        TIMER_REG(baseAddr, GPTMTBILR_OFFSET) = ConfigPtr->LoadValue;
    }
    
    /* Set match value (for PWM or match interrupts) */
    if (ConfigPtr->MatchValue > 0) {
        if (ConfigPtr->Block == TIMER_BLOCK_A) {
            TIMER_REG(baseAddr, GPTMTAMATCHR_OFFSET) = ConfigPtr->MatchValue;
        } else {
            TIMER_REG(baseAddr, GPTMTBMATCHR_OFFSET) = ConfigPtr->MatchValue;
        }
    }
    
    /* Set prescaler (extends timer range) */
    if (ConfigPtr->Prescaler > 0) {
        if (ConfigPtr->Block == TIMER_BLOCK_A) {
            TIMER_REG(baseAddr, GPTMTAPR_OFFSET) = ConfigPtr->Prescaler;
        } else {
            TIMER_REG(baseAddr, GPTMTBPR_OFFSET) = ConfigPtr->Prescaler;
        }
    }
    
    /* Configure interrupts */
    if (ConfigPtr->InterruptEnable == TRUE) {
        if (ConfigPtr->Block == TIMER_BLOCK_A) {
            TIMER_REG(baseAddr, GPTMIMR_OFFSET) |= GPTMIMR_TATOIM;  /* Enable timeout interrupt */
        } else {
            TIMER_REG(baseAddr, GPTMIMR_OFFSET) |= GPTMIMR_TBTOIM;  /* Enable timeout interrupt */
        }
    }
    
    if (ConfigPtr->MatchInterruptEnable == TRUE) {
        if (ConfigPtr->Block == TIMER_BLOCK_A) {
            TIMER_REG(baseAddr, GPTMIMR_OFFSET) |= GPTMIMR_TAMIM;  /* Enable match interrupt */
        } else {
            TIMER_REG(baseAddr, GPTMIMR_OFFSET) |= GPTMIMR_TBMIM;  /* Enable match interrupt */
        }
    }
    
    /* Save runtime state */
    Timer_RuntimeState[ConfigPtr->Module][ConfigPtr->Block].InitializedFlag = TIMER_INITIALIZED_FLAG;
    Timer_RuntimeState[ConfigPtr->Module][ConfigPtr->Block].TimeoutCb = ConfigPtr->TimeoutCallback;
    Timer_RuntimeState[ConfigPtr->Module][ConfigPtr->Block].MatchCb = ConfigPtr->MatchCallback;
    Timer_RuntimeState[ConfigPtr->Module][ConfigPtr->Block].LoadValue = ConfigPtr->LoadValue;
    Timer_RuntimeState[ConfigPtr->Module][ConfigPtr->Block].CountDir = ConfigPtr->CountDirection;
}

void Timer_DeInit(Timer_ModuleType Module, Timer_BlockType Block) {
#if (TIMER_DEV_ERROR_DETECT == STD_ON)
    if (Timer_IsModuleValid(Module) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_DEINIT_SID, TIMER_E_PARAM_MODULE);
        return;
    }
    if (Timer_IsBlockValid(Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_DEINIT_SID, TIMER_E_PARAM_BLOCK);
        return;
    }
    if (Timer_IsInitialized(Module, Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_DEINIT_SID, TIMER_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Timer_GetBaseAddress(Module);
    
    /* Disable timer */
    TIMER_REG(baseAddr, GPTMCTL_OFFSET) &= ~(Block == TIMER_BLOCK_A ? GPTMCTL_TAEN : GPTMCTL_TBEN);
    
    /* Clear interrupts */
    TIMER_REG(baseAddr, GPTMIMR_OFFSET) &= ~(Block == TIMER_BLOCK_A ? 
                                             (GPTMIMR_TATOIM | GPTMIMR_TAMIM) : 
                                             (GPTMIMR_TBTOIM | GPTMIMR_TBMIM));
    
    /* Clear runtime state */
    Timer_RuntimeState[Module][Block].InitializedFlag = TIMER_NOT_INITIALIZED;
    Timer_RuntimeState[Module][Block].TimeoutCb = NULL_PTR;
    Timer_RuntimeState[Module][Block].MatchCb = NULL_PTR;
    Timer_RuntimeState[Module][Block].LoadValue = 0;
    Timer_RuntimeState[Module][Block].CountDir = TIMER_COUNT_DOWN;
}

void Timer_Start(Timer_ModuleType Module, Timer_BlockType Block) {
#if (TIMER_DEV_ERROR_DETECT == STD_ON)
    if (Timer_IsModuleValid(Module) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_START_SID, TIMER_E_PARAM_MODULE);
        return;
    }
    if (Timer_IsBlockValid(Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_START_SID, TIMER_E_PARAM_BLOCK);
        return;
    }
    if (Timer_IsInitialized(Module, Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_START_SID, TIMER_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Timer_GetBaseAddress(Module);
    
    /* Enable timer */
    TIMER_REG(baseAddr, GPTMCTL_OFFSET) |= (Block == TIMER_BLOCK_A ? GPTMCTL_TAEN : GPTMCTL_TBEN);
}

void Timer_Stop(Timer_ModuleType Module, Timer_BlockType Block) {
#if (TIMER_DEV_ERROR_DETECT == STD_ON)
    if (Timer_IsModuleValid(Module) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_STOP_SID, TIMER_E_PARAM_MODULE);
        return;
    }
    if (Timer_IsBlockValid(Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_STOP_SID, TIMER_E_PARAM_BLOCK);
        return;
    }
    if (Timer_IsInitialized(Module, Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_STOP_SID, TIMER_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Timer_GetBaseAddress(Module);
    
    /* Disable timer */
    TIMER_REG(baseAddr, GPTMCTL_OFFSET) &= ~(Block == TIMER_BLOCK_A ? GPTMCTL_TAEN : GPTMCTL_TBEN);
}

Timer_ValueType Timer_GetElapsedTime(Timer_ModuleType Module, Timer_BlockType Block) {
#if (TIMER_DEV_ERROR_DETECT == STD_ON)
    if (Timer_IsModuleValid(Module) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_GET_ELAPSED_TIME_SID, TIMER_E_PARAM_MODULE);
        return 0;
    }
    if (Timer_IsBlockValid(Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_GET_ELAPSED_TIME_SID, TIMER_E_PARAM_BLOCK);
        return 0;
    }
    if (Timer_IsInitialized(Module, Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_GET_ELAPSED_TIME_SID, TIMER_E_UNINIT);
        return 0;
    }
#endif

    uint32 baseAddr = Timer_GetBaseAddress(Module);
    Timer_ValueType loadValue = Timer_RuntimeState[Module][Block].LoadValue;
    Timer_CountDirectionType countDir = Timer_RuntimeState[Module][Block].CountDir;
    Timer_ValueType currentValue;
    
    /* Read current timer value */
    if (Block == TIMER_BLOCK_A) {
        currentValue = TIMER_REG(baseAddr, GPTMTAR_OFFSET);
    } else {
        currentValue = TIMER_REG(baseAddr, GPTMTBR_OFFSET);
    }
    
    /* Calculate elapsed time based on count direction */
    if (countDir == TIMER_COUNT_DOWN) {
        /* Count-down: elapsed = loadValue - currentValue */
        return (loadValue - currentValue);
    } else {
        /* Count-up: elapsed = currentValue */
        return currentValue;
    }
}

Timer_ValueType Timer_GetRemainingTime(Timer_ModuleType Module, Timer_BlockType Block) {
#if (TIMER_DEV_ERROR_DETECT == STD_ON)
    if (Timer_IsModuleValid(Module) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_GET_REMAINING_TIME_SID, TIMER_E_PARAM_MODULE);
        return 0;
    }
    if (Timer_IsBlockValid(Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_GET_REMAINING_TIME_SID, TIMER_E_PARAM_BLOCK);
        return 0;
    }
    if (Timer_IsInitialized(Module, Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_GET_REMAINING_TIME_SID, TIMER_E_UNINIT);
        return 0;
    }
#endif

    uint32 baseAddr = Timer_GetBaseAddress(Module);
    Timer_ValueType remainingTime;
    
    /* Read current timer value (remaining time) */
    if (Block == TIMER_BLOCK_A) {
        remainingTime = TIMER_REG(baseAddr, GPTMTAR_OFFSET);
    } else {
        remainingTime = TIMER_REG(baseAddr, GPTMTBR_OFFSET);
    }
    
    return remainingTime;
}

void Timer_SetLoadValue(Timer_ModuleType Module, Timer_BlockType Block, Timer_ValueType Value) {
#if (TIMER_DEV_ERROR_DETECT == STD_ON)
    if (Timer_IsModuleValid(Module) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_SET_LOAD_VALUE_SID, TIMER_E_PARAM_MODULE);
        return;
    }
    if (Timer_IsBlockValid(Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_SET_LOAD_VALUE_SID, TIMER_E_PARAM_BLOCK);
        return;
    }
    if (Timer_IsInitialized(Module, Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_SET_LOAD_VALUE_SID, TIMER_E_UNINIT);
        return;
    }
    if (Value == 0) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_SET_LOAD_VALUE_SID, TIMER_E_PARAM_VALUE);
        return;
    }
#endif

    uint32 baseAddr = Timer_GetBaseAddress(Module);
    
    /* Set new load value */
    if (Block == TIMER_BLOCK_A) {
        TIMER_REG(baseAddr, GPTMTAILR_OFFSET) = Value;
    } else {
        TIMER_REG(baseAddr, GPTMTBILR_OFFSET) = Value;
    }
    
    /* Update runtime state */
    Timer_RuntimeState[Module][Block].LoadValue = Value;
}

Timer_StateType Timer_GetState(Timer_ModuleType Module, Timer_BlockType Block) {
#if (TIMER_DEV_ERROR_DETECT == STD_ON)
    if (Timer_IsModuleValid(Module) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_GET_STATE_SID, TIMER_E_PARAM_MODULE);
        return TIMER_STATE_STOPPED;
    }
    if (Timer_IsBlockValid(Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_GET_STATE_SID, TIMER_E_PARAM_BLOCK);
        return TIMER_STATE_STOPPED;
    }
    if (Timer_IsInitialized(Module, Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_GET_STATE_SID, TIMER_E_UNINIT);
        return TIMER_STATE_STOPPED;
    }
#endif

    uint32 baseAddr = Timer_GetBaseAddress(Module);
    uint32 ctlReg = TIMER_REG(baseAddr, GPTMCTL_OFFSET);
    
    /* Check if timer is enabled */
    boolean isEnabled = (Block == TIMER_BLOCK_A) ? 
                        ((ctlReg & GPTMCTL_TAEN) != 0) : 
                        ((ctlReg & GPTMCTL_TBEN) != 0);
    
    return isEnabled ? TIMER_STATE_RUNNING : TIMER_STATE_STOPPED;
}

void Timer_EnableInterrupt(Timer_ModuleType Module, Timer_BlockType Block, Timer_InterruptType IntType) {
#if (TIMER_DEV_ERROR_DETECT == STD_ON)
    if (Timer_IsModuleValid(Module) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_ENABLE_INTERRUPT_SID, TIMER_E_PARAM_MODULE);
        return;
    }
    if (Timer_IsBlockValid(Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_ENABLE_INTERRUPT_SID, TIMER_E_PARAM_BLOCK);
        return;
    }
    if (Timer_IsInitialized(Module, Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_ENABLE_INTERRUPT_SID, TIMER_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Timer_GetBaseAddress(Module);
    
    /* Enable specified interrupt type */
    switch(IntType) {
        case TIMER_INT_TIMEOUT:
            TIMER_REG(baseAddr, GPTMIMR_OFFSET) |= (Block == TIMER_BLOCK_A ? GPTMIMR_TATOIM : GPTMIMR_TBTOIM);
            break;
        case TIMER_INT_MATCH:
            TIMER_REG(baseAddr, GPTMIMR_OFFSET) |= (Block == TIMER_BLOCK_A ? GPTMIMR_TAMIM : GPTMIMR_TBMIM);
            break;
        case TIMER_INT_CAPTURE_MATCH:
            TIMER_REG(baseAddr, GPTMIMR_OFFSET) |= (Block == TIMER_BLOCK_A ? GPTMIMR_CAMIM : GPTMIMR_CBMIM);
            break;
        case TIMER_INT_CAPTURE_EVENT:
            TIMER_REG(baseAddr, GPTMIMR_OFFSET) |= (Block == TIMER_BLOCK_A ? GPTMIMR_CAEIM : GPTMIMR_CBEIM);
            break;
        default:
            break;
    }
}

void Timer_DisableInterrupt(Timer_ModuleType Module, Timer_BlockType Block, Timer_InterruptType IntType) {
#if (TIMER_DEV_ERROR_DETECT == STD_ON)
    if (Timer_IsModuleValid(Module) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_DISABLE_INTERRUPT_SID, TIMER_E_PARAM_MODULE);
        return;
    }
    if (Timer_IsBlockValid(Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_DISABLE_INTERRUPT_SID, TIMER_E_PARAM_BLOCK);
        return;
    }
    if (Timer_IsInitialized(Module, Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_DISABLE_INTERRUPT_SID, TIMER_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Timer_GetBaseAddress(Module);
    
    /* Disable specified interrupt type */
    switch(IntType) {
        case TIMER_INT_TIMEOUT:
            TIMER_REG(baseAddr, GPTMIMR_OFFSET) &= ~(Block == TIMER_BLOCK_A ? GPTMIMR_TATOIM : GPTMIMR_TBTOIM);
            break;
        case TIMER_INT_MATCH:
            TIMER_REG(baseAddr, GPTMIMR_OFFSET) &= ~(Block == TIMER_BLOCK_A ? GPTMIMR_TAMIM : GPTMIMR_TBMIM);
            break;
        case TIMER_INT_CAPTURE_MATCH:
            TIMER_REG(baseAddr, GPTMIMR_OFFSET) &= ~(Block == TIMER_BLOCK_A ? GPTMIMR_CAMIM : GPTMIMR_CBMIM);
            break;
        case TIMER_INT_CAPTURE_EVENT:
            TIMER_REG(baseAddr, GPTMIMR_OFFSET) &= ~(Block == TIMER_BLOCK_A ? GPTMIMR_CAEIM : GPTMIMR_CBEIM);
            break;
        default:
            break;
    }
}

void Timer_ClearInterrupt(Timer_ModuleType Module, Timer_BlockType Block, Timer_InterruptType IntType) {
#if (TIMER_DEV_ERROR_DETECT == STD_ON)
    if (Timer_IsModuleValid(Module) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_CLEAR_INTERRUPT_SID, TIMER_E_PARAM_MODULE);
        return;
    }
    if (Timer_IsBlockValid(Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_CLEAR_INTERRUPT_SID, TIMER_E_PARAM_BLOCK);
        return;
    }
    if (Timer_IsInitialized(Module, Block) == FALSE) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_CLEAR_INTERRUPT_SID, TIMER_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Timer_GetBaseAddress(Module);
    
    /* Clear specified interrupt flag (W1C register - write 1 to clear) */
    switch(IntType) {
        case TIMER_INT_TIMEOUT:
            TIMER_REG(baseAddr, GPTMICR_OFFSET) = (Block == TIMER_BLOCK_A ? GPTMICR_TATOCINT : GPTMICR_TBTOCINT);
            break;
        case TIMER_INT_MATCH:
            TIMER_REG(baseAddr, GPTMICR_OFFSET) = (Block == TIMER_BLOCK_A ? GPTMICR_TAMCINT : GPTMICR_TBMCINT);
            break;
        case TIMER_INT_CAPTURE_MATCH:
            TIMER_REG(baseAddr, GPTMICR_OFFSET) = (Block == TIMER_BLOCK_A ? GPTMICR_CAMCINT : GPTMICR_CBMCINT);
            break;
        case TIMER_INT_CAPTURE_EVENT:
            TIMER_REG(baseAddr, GPTMICR_OFFSET) = (Block == TIMER_BLOCK_A ? GPTMICR_CAECINT : GPTMICR_CBECINT);
            break;
        default:
            break;
    }
}

#if (TIMER_VERSION_INFO_API == STD_ON)
void Timer_GetVersionInfo(Std_VersionInfoType* VersionInfo) {
#if (TIMER_DEV_ERROR_DETECT == STD_ON)
    if (VersionInfo == NULL_PTR) {
        Det_ReportError(TIMER_MODULE_ID, TIMER_INSTANCE_ID, TIMER_GET_VERSION_INFO_SID, TIMER_E_PARAM_POINTER);
        return;
    }
#endif

    VersionInfo->vendorID = 0;
    VersionInfo->moduleID = 0;
    VersionInfo->sw_major_version = TIMER_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = TIMER_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = TIMER_SW_PATCH_VERSION;
}
#endif
