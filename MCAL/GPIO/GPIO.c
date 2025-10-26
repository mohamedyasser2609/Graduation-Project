/*
 * @file Gpio.c
 * @brief GPIO Driver Implementation for TM4C123GH6PM
 * @details This file contains the complete AUTOSAR-compliant implementation
 *          of the GPIO driver for TM4C123GH6PM microcontroller.
 *
 * @author Mohamed Yasser
 * @date Oct 19, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

/* ===================[Includes]=================== */
#include "Gpio.h"
#include "Gpio_Regs.h"
#include "../../CONFIG/Det.h"

/* Critical section protection macros */
#if (GPIO_CRITICAL_SECTION_PROTECTION == STD_ON)
    /* AUTOSAR Schedule Manager critical section macros */
    #define SchM_Enter_Gpio_GPIO_EXCLUSIVE_AREA_00()  /* Disable interrupts or enter critical section */
    #define SchM_Exit_Gpio_GPIO_EXCLUSIVE_AREA_00()   /* Enable interrupts or exit critical section */
    #define SchM_Enter_Gpio_GPIO_EXCLUSIVE_AREA_01()  /* For unlock sequence */
    #define SchM_Exit_Gpio_GPIO_EXCLUSIVE_AREA_01()
    #define SchM_Enter_Gpio_GPIO_EXCLUSIVE_AREA_02()  /* For flip operation */
    #define SchM_Exit_Gpio_GPIO_EXCLUSIVE_AREA_02()
    #define SchM_Enter_Gpio_GPIO_EXCLUSIVE_AREA_03()  /* For pin configuration */
    #define SchM_Exit_Gpio_GPIO_EXCLUSIVE_AREA_03()
    
    /* Note: In a full AUTOSAR implementation, these would be provided by the RTE/OS.
     * For bare-metal or simple RTOS, implement as:
     * - Disable/enable global interrupts, or
     * - Use mutex/semaphore, or
     * - Use RTOS critical section API */
#endif

/* ===================[Private Variables]=================== */
/**
 * @brief GPIO module initialization state
 */
static Gpio_StateType Gpio_ModuleState = GPIO_UNINIT;

/**
 * @brief Pointer to current configuration
 */
static const Gpio_ConfigType* Gpio_ConfigPtr = NULL_PTR;

/**
 * @brief Port clock enable tracking (bit field for 6 ports)
 * @details Bit 0 = Port A, Bit 1 = Port B, ..., Bit 5 = Port F
 */
static uint8 Gpio_PortClockEnabled = 0u;

/* ===================[Private Function Prototypes]=================== */
static void Gpio_EnablePortClock(Gpio_PortType Port);
static void Gpio_UnlockPin(Gpio_PortType Port, Gpio_PinType Pin);
static void Gpio_ConfigurePin(const Gpio_PinConfigType* PinConfig);
static Gpio_RegisterType* Gpio_GetPortRegister(Gpio_PortType Port);

/* ===================[Private Function Implementations]=================== */

/**
 * @brief Enable clock for GPIO port (once per port)
 * @param[in] Port - GPIO port to enable
 * @details This function enables the clock only once per port to avoid
 *          redundant operations and ensures proper stabilization.
 */
static void Gpio_EnablePortClock(Gpio_PortType Port)
{
    uint32 portMask = 0u;
    uint8 portBit = (1u << Port);
    
    /* Check if clock is already enabled for this port */
    if ((Gpio_PortClockEnabled & portBit) != 0u) {
        return;  /* Clock already enabled */
    }
    
    /* Determine port clock mask */
    switch(Port) {
        case GPIO_PORT_A: portMask = GPIO_RCGC_PORTA; break;
        case GPIO_PORT_B: portMask = GPIO_RCGC_PORTB; break;
        case GPIO_PORT_C: portMask = GPIO_RCGC_PORTC; break;
        case GPIO_PORT_D: portMask = GPIO_RCGC_PORTD; break;
        case GPIO_PORT_E: portMask = GPIO_RCGC_PORTE; break;
        case GPIO_PORT_F: portMask = GPIO_RCGC_PORTF; break;
        default: break;
    }
    
    /* Enable clock with critical section protection */
#if (GPIO_CRITICAL_SECTION_PROTECTION == STD_ON)
    SchM_Enter_Gpio_GPIO_EXCLUSIVE_AREA_00();
#endif
    
    GPIO_SYSCTL_RCGCGPIO_REG |= portMask;
    
#if (GPIO_CRITICAL_SECTION_PROTECTION == STD_ON)
    SchM_Exit_Gpio_GPIO_EXCLUSIVE_AREA_00();
#endif
    
    /* Wait for clock to stabilize - read back to ensure write completed */
    volatile uint32 readback = GPIO_SYSCTL_RCGCGPIO_REG;
    (void)readback;  /* Prevent compiler warning */
    
    /* Additional stabilization delay */
    volatile uint32 delay = GPIO_CLOCK_STABILIZATION_DELAY;
    while(delay--) {
        /* NOP */
    }
    
    /* Mark port clock as enabled */
    Gpio_PortClockEnabled |= portBit;
}

/**
 * @brief Unlock GPIO pin (for special pins like PF0, PD7)
 * @param[in] Port - GPIO port
 * @param[in] Pin - GPIO pin
 * @details Per TM4C123GH6PM datasheet:
 *          - Write 0x4C4F434B to LOCK register to unlock
 *          - Set corresponding bit in CR register to allow changes
 *          - Lock is automatically re-locked after CR write
 *          - JTAG pins (PC0-PC3) are NEVER unlocked to preserve debug capability
 */
static void Gpio_UnlockPin(Gpio_PortType Port, Gpio_PinType Pin)
{
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(Port);
    uint8 pinMask = (1u << Pin);
    
    /* CRITICAL: Never unlock JTAG pins (PC0-PC3) - this would disable debugging! */
    if (Port == GPIO_PORT_C && Pin <= GPIO_PIN_3) {
        return;  /* JTAG pins must remain locked */
    }
    
    /* Check if pin needs unlocking (PF0, PD7 only) */
    if ((Port == GPIO_PORT_F && Pin == GPIO_PIN_0) ||
        (Port == GPIO_PORT_D && Pin == GPIO_PIN_7)) {
        
#if (GPIO_CRITICAL_SECTION_PROTECTION == STD_ON)
        SchM_Enter_Gpio_GPIO_EXCLUSIVE_AREA_01();
#endif
        
        /* Write unlock key to LOCK register */
        portReg->LOCK = GPIO_LOCK_KEY;
        
        /* Verify unlock was successful (LOCK should read as 0) */
        if (portReg->LOCK == GPIO_LOCK_UNLOCKED) {
            /* Set bit in CR register to allow changes to this pin */
            portReg->CR |= pinMask;
        }
        /* Note: LOCK automatically re-locks after CR write per datasheet */
        
#if (GPIO_CRITICAL_SECTION_PROTECTION == STD_ON)
        SchM_Exit_Gpio_GPIO_EXCLUSIVE_AREA_01();
#endif
    }
}

/**
 * @brief Get port register base address
 * @param[in] Port - GPIO port
 * @return Pointer to port registers
 */
static Gpio_RegisterType* Gpio_GetPortRegister(Gpio_PortType Port)
{
    Gpio_RegisterType* portReg = NULL_PTR;
    
    switch(Port) {
        case GPIO_PORT_A: portReg = GPIO_PORTA_REG; break;
        case GPIO_PORT_B: portReg = GPIO_PORTB_REG; break;
        case GPIO_PORT_C: portReg = GPIO_PORTC_REG; break;
        case GPIO_PORT_D: portReg = GPIO_PORTD_REG; break;
        case GPIO_PORT_E: portReg = GPIO_PORTE_REG; break;
        case GPIO_PORT_F: portReg = GPIO_PORTF_REG; break;
        default: break;
    }
    
    return portReg;
}

/**
 * @brief Configure individual GPIO pin
 * @param[in] PinConfig - Pointer to pin configuration
 * @details All RMW register sequences are protected with critical sections
 *          JTAG pins (PC0-PC3) are protected and will not be configured
 */
static void Gpio_ConfigurePin(const Gpio_PinConfigType* PinConfig)
{
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(PinConfig->Port);
    uint8 pinMask = (1u << PinConfig->Pin);
    
    /* CRITICAL: Protect JTAG pins (PC0-PC3) from configuration */
    if (PinConfig->Port == GPIO_PORT_C && PinConfig->Pin <= GPIO_PIN_3) {
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_INIT_SID, GPIO_E_JTAG_PIN_PROTECTED);
#endif
        return;  /* Skip JTAG pin configuration to preserve debug capability */
    }
    
    /* Enable port clock if not already enabled */
    Gpio_EnablePortClock(PinConfig->Port);
    
    /* Unlock pin if necessary */
    Gpio_UnlockPin(PinConfig->Port, PinConfig->Pin);
    
    /* Enter critical section for configuration sequence */
#if (GPIO_CRITICAL_SECTION_PROTECTION == STD_ON)
    SchM_Enter_Gpio_GPIO_EXCLUSIVE_AREA_03();
#endif
    
    /* Configure pin mode */
    if (PinConfig->Mode == GPIO_MODE_ANALOG) {
        /* Analog mode */
        portReg->AMSEL |= pinMask;
        portReg->DEN &= ~pinMask;
        portReg->AFSEL &= ~pinMask;
    }
    else if (PinConfig->Mode == GPIO_MODE_ALT_FUNC) {
        /* Alternate function mode */
        portReg->AMSEL &= ~pinMask;
        portReg->AFSEL |= pinMask;
        portReg->DEN |= pinMask;
        
        /* Configure alternate function number */
        uint32 pctlMask = GPIO_GET_PCTL_MASK(PinConfig->Pin);
        uint8 pctlShift = GPIO_GET_PCTL_SHIFT(PinConfig->Pin);
        portReg->PCTL = (portReg->PCTL & ~pctlMask) | 
                        ((uint32)PinConfig->AlternateFuncNum << pctlShift);
    }
    else {
        /* Digital I/O mode */
        portReg->AMSEL &= ~pinMask;
        portReg->AFSEL &= ~pinMask;
        portReg->DEN |= pinMask;
        portReg->PCTL &= ~GPIO_GET_PCTL_MASK(PinConfig->Pin);
    }
    
    /* Configure direction */
    if (PinConfig->Direction == GPIO_PIN_OUT) {
        portReg->DIR |= pinMask;
        
        /* Set initial level for output pins using atomic masked write */
        if (PinConfig->InitialLevel == GPIO_LEVEL_HIGH) {
            portReg->DATA_BITS[pinMask] = 0xFFu;
        } else {
            portReg->DATA_BITS[pinMask] = 0x00u;
        }
    } else {
        portReg->DIR &= ~pinMask;
    }
    
    /* Configure internal resistor */
    switch(PinConfig->InternalResistor) {
        case GPIO_RESISTOR_PULL_UP:
            portReg->PUR |= pinMask;
            portReg->PDR &= ~pinMask;
            portReg->ODR &= ~pinMask;
            break;
            
        case GPIO_RESISTOR_PULL_DOWN:
            portReg->PDR |= pinMask;
            portReg->PUR &= ~pinMask;
            portReg->ODR &= ~pinMask;
            break;
            
        case GPIO_RESISTOR_OPEN_DRAIN:
            portReg->ODR |= pinMask;
            portReg->PUR &= ~pinMask;
            portReg->PDR &= ~pinMask;
            break;
            
        default: /* GPIO_RESISTOR_OFF */
            portReg->PUR &= ~pinMask;
            portReg->PDR &= ~pinMask;
            portReg->ODR &= ~pinMask;
            break;
    }
    
    /* Configure drive strength */
    switch(PinConfig->DriveStrength) {
        case GPIO_DRIVE_2MA:
            portReg->DR2R |= pinMask;
            portReg->DR4R &= ~pinMask;
            portReg->DR8R &= ~pinMask;
            break;
            
        case GPIO_DRIVE_4MA:
            portReg->DR4R |= pinMask;
            portReg->DR2R &= ~pinMask;
            portReg->DR8R &= ~pinMask;
            break;
            
        case GPIO_DRIVE_8MA:
            portReg->DR8R |= pinMask;
            portReg->DR2R &= ~pinMask;
            portReg->DR4R &= ~pinMask;
            break;
            
        default:
            portReg->DR2R |= pinMask;
            portReg->DR4R &= ~pinMask;
            portReg->DR8R &= ~pinMask;
            break;
    }
    
    /* Configure slew rate */
    if (PinConfig->SlewRate == GPIO_SLEW_RATE_SLOW) {
        portReg->SLR |= pinMask;
    } else {
        portReg->SLR &= ~pinMask;
    }
    
    /* Configure interrupt */
    if (PinConfig->IntTrigger != GPIO_INT_DISABLED) {
        /* Disable interrupt during configuration */
        portReg->IM &= ~pinMask;
        
        switch(PinConfig->IntTrigger) {
            case GPIO_INT_RISING_EDGE:
                portReg->IS &= ~pinMask;
                portReg->IBE &= ~pinMask;
                portReg->IEV |= pinMask;
                break;
                
            case GPIO_INT_FALLING_EDGE:
                portReg->IS &= ~pinMask;
                portReg->IBE &= ~pinMask;
                portReg->IEV &= ~pinMask;
                break;
                
            case GPIO_INT_BOTH_EDGES:
                portReg->IS &= ~pinMask;
                portReg->IBE |= pinMask;
                break;
                
            case GPIO_INT_LOW_LEVEL:
                portReg->IS |= pinMask;
                portReg->IEV &= ~pinMask;
                break;
                
            case GPIO_INT_HIGH_LEVEL:
                portReg->IS |= pinMask;
                portReg->IEV |= pinMask;
                break;
                
            default:
                break;
        }
        
        /* Clear any pending interrupt */
        portReg->ICR |= pinMask;
    }
    
    /* Exit critical section */
#if (GPIO_CRITICAL_SECTION_PROTECTION == STD_ON)
    SchM_Exit_Gpio_GPIO_EXCLUSIVE_AREA_03();
#endif
}

/* ===================[Public Function Implementations]=================== */

/**
 * @brief Initialize GPIO module
 */
void Gpio_Init(const Gpio_ConfigType* ConfigPtr)
{
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    /* Check if already initialized */
    if (Gpio_ModuleState == GPIO_INIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_INIT_SID, GPIO_E_ALREADY_INITIALIZED);
        return;
    }
    
    /* Check for NULL pointer */
    if (ConfigPtr == NULL_PTR) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_INIT_SID, GPIO_E_PARAM_POINTER);
        return;
    }
    
    /* Check for valid configuration */
    if (ConfigPtr->PinConfigs == NULL_PTR || ConfigPtr->NumberOfPins == 0u) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_INIT_SID, GPIO_E_PARAM_CONFIG);
        return;
    }
#endif
    
    /* Store configuration pointer */
    Gpio_ConfigPtr = ConfigPtr;
    
    /* Configure all pins */
    {
        uint16 i;
        for (i = 0u; i < ConfigPtr->NumberOfPins; i++) {
            Gpio_ConfigurePin(&ConfigPtr->PinConfigs[i]);
        }
    }
    
    /* Set module state to initialized */
    Gpio_ModuleState = GPIO_INIT;
}

/**
 * @brief Read GPIO channel level
 * @details Uses atomic masked read via DATA_BITS array for thread-safe access
 */
Gpio_LevelType Gpio_ReadChannel(Gpio_ChannelType Channel)
{
    Gpio_LevelType level = GPIO_LEVEL_LOW;
    
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_READ_CHANNEL_SID, GPIO_E_UNINIT);
        return GPIO_LEVEL_LOW;
    }
    
    if (!GPIO_IS_VALID_CHANNEL(Channel)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_READ_CHANNEL_SID, GPIO_E_PARAM_INVALID_CHANNEL);
        return GPIO_LEVEL_LOW;
    }
#endif
    
    Gpio_PortType port = GPIO_GET_PORT(Channel);
    Gpio_PinType pin = GPIO_GET_PIN(Channel);
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(port);
    
    /* Use atomic masked read via DATA_BITS array
     * Address = Base + 4 * (bit mask)
     * Reading DATA_BITS[mask] returns only the masked bits */
    uint8 pinMask = (1u << pin);
    if (portReg->DATA_BITS[pinMask] != 0u) {
        level = GPIO_LEVEL_HIGH;
    }
    
    return level;
}

/**
 * @brief Write GPIO channel level
 * @details Uses atomic masked write via DATA_BITS array to prevent lost updates.
 *          This is the recommended method per TM4C123GH6PM datasheet section 10.4.
 */
void Gpio_WriteChannel(Gpio_ChannelType Channel, Gpio_LevelType Level)
{
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_WRITE_CHANNEL_SID, GPIO_E_UNINIT);
        return;
    }
    
    if (!GPIO_IS_VALID_CHANNEL(Channel)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_WRITE_CHANNEL_SID, GPIO_E_PARAM_INVALID_CHANNEL);
        return;
    }
#endif
    
    Gpio_PortType port = GPIO_GET_PORT(Channel);
    Gpio_PinType pin = GPIO_GET_PIN(Channel);
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(port);
    
    /* Use atomic masked write via DATA_BITS array
     * Address = Base + 4 * (bit mask)
     * Writing to DATA_BITS[mask] affects only the masked bits
     * This prevents read-modify-write races and lost updates */
    uint8 pinMask = (1u << pin);
    
    if (Level == GPIO_LEVEL_HIGH) {
        portReg->DATA_BITS[pinMask] = 0xFFu;  /* Set masked bit */
    } else {
        portReg->DATA_BITS[pinMask] = 0x00u;  /* Clear masked bit */
    }
}

/**
 * @brief Read GPIO port level
 */
uint8 Gpio_ReadPort(Gpio_PortType Port)
{
    uint8 portLevel = 0u;
    
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_READ_PORT_SID, GPIO_E_UNINIT);
        return 0u;
    }
    
    if (!GPIO_IS_VALID_PORT(Port)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_READ_PORT_SID, GPIO_E_PARAM_INVALID_PORT);
        return 0u;
    }
#endif
    
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(Port);
    portLevel = (uint8)(portReg->DATA & 0xFFu);
    
    return portLevel;
}

/**
 * @brief Write GPIO port level
 */
void Gpio_WritePort(Gpio_PortType Port, uint8 Level)
{
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_WRITE_PORT_SID, GPIO_E_UNINIT);
        return;
    }
    
    if (!GPIO_IS_VALID_PORT(Port)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_WRITE_PORT_SID, GPIO_E_PARAM_INVALID_PORT);
        return;
    }
#endif
    
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(Port);
    portReg->DATA = (uint32)Level;
}

#if (GPIO_FLIP_CHANNEL_API == STD_ON)
/**
 * @brief Flip GPIO channel level
 * @details Uses read-modify-write with atomic masked access and critical section
 */
Gpio_LevelType Gpio_FlipChannel(Gpio_ChannelType Channel)
{
    Gpio_LevelType newLevel = GPIO_LEVEL_LOW;
    
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_FLIP_CHANNEL_SID, GPIO_E_UNINIT);
        return GPIO_LEVEL_LOW;
    }
    
    if (!GPIO_IS_VALID_CHANNEL(Channel)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_FLIP_CHANNEL_SID, GPIO_E_PARAM_INVALID_CHANNEL);
        return GPIO_LEVEL_LOW;
    }
#endif
    
    Gpio_PortType port = GPIO_GET_PORT(Channel);
    Gpio_PinType pin = GPIO_GET_PIN(Channel);
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(port);
    uint8 pinMask = (1u << pin);
    
#if (GPIO_CRITICAL_SECTION_PROTECTION == STD_ON)
    SchM_Enter_Gpio_GPIO_EXCLUSIVE_AREA_02();
#endif
    
    /* Read current level using atomic masked read */
    uint32 currentLevel = portReg->DATA_BITS[pinMask];
    
    /* Toggle and write using atomic masked write */
    if (currentLevel != 0u) {
        portReg->DATA_BITS[pinMask] = 0x00u;  /* Was high, set low */
        newLevel = GPIO_LEVEL_LOW;
    } else {
        portReg->DATA_BITS[pinMask] = 0xFFu;  /* Was low, set high */
        newLevel = GPIO_LEVEL_HIGH;
    }
    
#if (GPIO_CRITICAL_SECTION_PROTECTION == STD_ON)
    SchM_Exit_Gpio_GPIO_EXCLUSIVE_AREA_02();
#endif
    
    return newLevel;
}
#endif

#if (GPIO_VERSION_INFO_API == STD_ON)
/**
 * @brief Get GPIO driver version information
 */
void Gpio_GetVersionInfo(Std_VersionInfoType* VersionInfo)
{
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (VersionInfo == NULL_PTR) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_GET_VERSION_INFO_SID, GPIO_E_PARAM_POINTER);
        return;
    }
#endif
    
    VersionInfo->vendorID = GPIO_VENDOR_ID;
    VersionInfo->moduleID = GPIO_MODULE_ID;
    VersionInfo->sw_major_version = GPIO_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = GPIO_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = GPIO_SW_PATCH_VERSION;
}
#endif

#if (GPIO_SET_PIN_DIRECTION_API == STD_ON)
/**
 * @brief Set GPIO pin direction
 */
void Gpio_SetPinDirection(Gpio_ChannelType Channel, Gpio_PinDirectionType Direction)
{
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_SET_PIN_DIRECTION_SID, GPIO_E_UNINIT);
        return;
    }
    
    if (!GPIO_IS_VALID_CHANNEL(Channel)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_SET_PIN_DIRECTION_SID, GPIO_E_PARAM_INVALID_CHANNEL);
        return;
    }
#endif
    
    Gpio_PortType port = GPIO_GET_PORT(Channel);
    Gpio_PinType pin = GPIO_GET_PIN(Channel);
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(port);
    
    if (Direction == GPIO_PIN_OUT) {
        portReg->DIR |= (1u << pin);
    } else {
        portReg->DIR &= ~(1u << pin);
    }
}
#endif

#if (GPIO_REFRESH_PORT_DIRECTION_API == STD_ON)
/**
 * @brief Refresh port direction
 */
void Gpio_RefreshPortDirection(Gpio_PortType Port)
{
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_REFRESH_PORT_DIRECTION_SID, GPIO_E_UNINIT);
        return;
    }
    
    if (!GPIO_IS_VALID_PORT(Port)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_REFRESH_PORT_DIRECTION_SID, GPIO_E_PARAM_INVALID_PORT);
        return;
    }
#endif
    
    if (Gpio_ConfigPtr != NULL_PTR) {
        uint16 i;
        for (i = 0u; i < Gpio_ConfigPtr->NumberOfPins; i++) {
            const Gpio_PinConfigType* pinCfg = &Gpio_ConfigPtr->PinConfigs[i];
            
            if (pinCfg->Port == Port && !pinCfg->DirectionChangeable) {
                Gpio_RegisterType* portReg = Gpio_GetPortRegister(Port);
                uint8 pinMask = (1u << pinCfg->Pin);
                
                if (pinCfg->Direction == GPIO_PIN_OUT) {
                    portReg->DIR |= pinMask;
                } else {
                    portReg->DIR &= ~pinMask;
                }
            }
        }
    }
}
#endif

#if (GPIO_PORT_GROUP_API == STD_ON)
/**
 * @brief Read GPIO channel group
 */
uint8 Gpio_ReadChannelGroup(const Gpio_PortGroupType* ChannelGroupPtr)
{
    uint8 groupLevel = 0u;
    
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_READ_CHANNEL_GROUP_SID, GPIO_E_UNINIT);
        return 0u;
    }
    
    if (ChannelGroupPtr == NULL_PTR) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_READ_CHANNEL_GROUP_SID, GPIO_E_PARAM_POINTER);
        return 0u;
    }
    
    if (!GPIO_IS_VALID_PORT(ChannelGroupPtr->Port)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_READ_CHANNEL_GROUP_SID, GPIO_E_PARAM_INVALID_GROUP);
        return 0u;
    }
#endif
    
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(ChannelGroupPtr->Port);
    groupLevel = (uint8)((portReg->DATA & ChannelGroupPtr->Mask) >> ChannelGroupPtr->Offset);
    
    return groupLevel;
}

/**
 * @brief Write GPIO channel group
 */
void Gpio_WriteChannelGroup(const Gpio_PortGroupType* ChannelGroupPtr, uint8 Level)
{
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_WRITE_CHANNEL_GROUP_SID, GPIO_E_UNINIT);
        return;
    }
    
    if (ChannelGroupPtr == NULL_PTR) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_WRITE_CHANNEL_GROUP_SID, GPIO_E_PARAM_POINTER);
        return;
    }
    
    if (!GPIO_IS_VALID_PORT(ChannelGroupPtr->Port)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_WRITE_CHANNEL_GROUP_SID, GPIO_E_PARAM_INVALID_GROUP);
        return;
    }
#endif
    
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(ChannelGroupPtr->Port);
    uint32 shiftedLevel = ((uint32)Level << ChannelGroupPtr->Offset) & ChannelGroupPtr->Mask;
    portReg->DATA = (portReg->DATA & ~ChannelGroupPtr->Mask) | shiftedLevel;
}
#endif

#if (GPIO_INTERRUPT_SUPPORT == STD_ON)
/**
 * @brief Enable GPIO interrupt
 */
void Gpio_EnableInterrupt(Gpio_ChannelType Channel)
{
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_ENABLE_INTERRUPT_SID, GPIO_E_UNINIT);
        return;
    }
    
    if (!GPIO_IS_VALID_CHANNEL(Channel)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_ENABLE_INTERRUPT_SID, GPIO_E_PARAM_INVALID_CHANNEL);
        return;
    }
#endif
    
    Gpio_PortType port = GPIO_GET_PORT(Channel);
    Gpio_PinType pin = GPIO_GET_PIN(Channel);
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(port);
    
    portReg->IM |= (1u << pin);
}

/**
 * @brief Disable GPIO interrupt
 */
void Gpio_DisableInterrupt(Gpio_ChannelType Channel)
{
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_DISABLE_INTERRUPT_SID, GPIO_E_UNINIT);
        return;
    }
    
    if (!GPIO_IS_VALID_CHANNEL(Channel)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_DISABLE_INTERRUPT_SID, GPIO_E_PARAM_INVALID_CHANNEL);
        return;
    }
#endif
    
    Gpio_PortType port = GPIO_GET_PORT(Channel);
    Gpio_PinType pin = GPIO_GET_PIN(Channel);
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(port);
    
    portReg->IM &= ~(1u << pin);
}

/**
 * @brief Clear GPIO interrupt flag
 */
void Gpio_ClearInterrupt(Gpio_ChannelType Channel)
{
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_CLEAR_INTERRUPT_SID, GPIO_E_UNINIT);
        return;
    }
    
    if (!GPIO_IS_VALID_CHANNEL(Channel)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_CLEAR_INTERRUPT_SID, GPIO_E_PARAM_INVALID_CHANNEL);
        return;
    }
#endif
    
    Gpio_PortType port = GPIO_GET_PORT(Channel);
    Gpio_PinType pin = GPIO_GET_PIN(Channel);
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(port);
    
    portReg->ICR |= (1u << pin);
}

/**
 * @brief Get GPIO interrupt status
 */
boolean Gpio_GetInterruptStatus(Gpio_ChannelType Channel)
{
    boolean status = FALSE;
    
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
    if (Gpio_ModuleState == GPIO_UNINIT) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_CLEAR_INTERRUPT_SID, GPIO_E_UNINIT);
        return FALSE;
    }
    
    if (!GPIO_IS_VALID_CHANNEL(Channel)) {
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_CLEAR_INTERRUPT_SID, GPIO_E_PARAM_INVALID_CHANNEL);
        return FALSE;
    }
#endif
    
    Gpio_PortType port = GPIO_GET_PORT(Channel);
    Gpio_PinType pin = GPIO_GET_PIN(Channel);
    Gpio_RegisterType* portReg = Gpio_GetPortRegister(port);
    
    if ((portReg->MIS & (1u << pin)) != 0u) {
        status = TRUE;
    }
    
    return status;
}
#endif
