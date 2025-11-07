/**
 * @file MCU.c
 * @brief MCU Driver Implementation for TM4C123GH6PM
 * @details AUTOSAR-compliant MCU driver with integrated PLL configuration
 *
 * @author Mohamed Yasser
 * @date Nov 1, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#include "Mcu.h"
#include "Mcu_Cfg.h"
#include "tm4c123gh6pm.h"

/* ===================[Register Helpers]=================== */
#define MCU_SCB_SCR_REG              (*((volatile uint32*)0xE000ED10UL))
#define MCU_SCB_SCR_SLEEPDEEP_MASK   (0x4UL)

/* ===================[Private Variables]=================== */
static boolean Mcu_ModuleInitialized = FALSE;    /**< @brief MCU module initialization status */
static uint32 Mcu_SystemClockFrequency = 0UL;    /**< @brief Current system clock frequency */
static uint32 Mcu_TargetClockFrequency = 0UL;    /**< @brief Target clock frequency (after PLL distribution) */
static boolean Mcu_PllEnabled = FALSE;           /**< @brief PLL enable status */

/* ===================[Private Function Prototypes]=================== */
static void Mcu_SetSystemClock(Mcu_ClockType ClockSetting);
static void Mcu_ConfigurePll(Mcu_ClockType ClockSetting);
static void Mcu_Delay(uint32 Count);

/* ===================[Function Implementations]=================== */

/**
 * @brief Initialize MCU driver
 * @param ConfigPtr Pointer to MCU configuration
 */
void Mcu_Init(const Mcu_ConfigType* ConfigPtr)
{
    if (ConfigPtr == NULL_PTR)
    {
        return;
    }

    /* Store configuration */
    Mcu_PllEnabled = ConfigPtr->PllEnabled;

    /* Set default system clock frequency */
    Mcu_SystemClockFrequency = MCU_CLOCK_FREQ_MOSC_16MHZ;

    /* Set module as initialized */
    Mcu_ModuleInitialized = TRUE;
}

/**
 * @brief Initialize clock (including PLL if configured)
 * @param ClockSetting Clock configuration to use
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType Mcu_InitClock(Mcu_ClockType ClockSetting)
{
    if (Mcu_ModuleInitialized == FALSE)
    {
        return E_NOT_OK;
    }
    
    if (ClockSetting > MCU_CLOCK_80MHZ)
    {
        return E_NOT_OK;
    }

    /* Configure system clock */
    Mcu_SetSystemClock(ClockSetting);

    return E_OK;
}

/**
 * @brief Activate PLL clock (switch system clock to PLL)
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType Mcu_DistributePllClock(void)
{
    if (Mcu_ModuleInitialized == FALSE || Mcu_PllEnabled == FALSE)
    {
        return E_NOT_OK;
    }

    /* Check if PLL is locked */
    if (Mcu_GetPllStatus() != MCU_PLL_LOCKED)
    {
        return E_NOT_OK;
    }

    /* Clear BYPASS2 to enable PLL (RCC2 overrides RCC when USERCC2 is set) */
    SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;
    
    /* Hardware delay - wait a few cycles for clock to stabilize */
    __asm("    nop\n"
          "    nop\n"
          "    nop\n"
          "    nop\n");
    
    /* NOW update the actual system clock frequency */
    Mcu_SystemClockFrequency = Mcu_TargetClockFrequency;

    return E_OK;
}

/**
 * @brief Get PLL status
 * @return PLL lock status
 */
Mcu_PllStatusType Mcu_GetPllStatus(void)
{
    if (Mcu_ModuleInitialized == FALSE)
    {
        return MCU_PLL_STATUS_UNDEFINED;
    }

    /* Check PLL lock status */
    if ((SYSCTL_PLLSTAT_R & 0x01) != 0UL)
    {
        return MCU_PLL_LOCKED;
    }
    else
    {
        return MCU_PLL_UNLOCKED;
    }
}

/**
 * @brief Get reset reason
 * @return Reset reason
 */
Mcu_ResetType Mcu_GetResetReason(void)
{
    uint32 ResetCause = SYSCTL_RESC_R;
    
    if (Mcu_ModuleInitialized == FALSE)
    {
        return MCU_RESET_UNDEFINED;
    }

    /* Determine reset cause */
    if ((ResetCause & SYSCTL_RESC_POR) != 0UL)
    {
        return MCU_POWER_ON_RESET;
    }
    else if ((ResetCause & SYSCTL_RESC_EXT) != 0UL)
    {
        return MCU_EXTERNAL_RESET;
    }
    else if ((ResetCause & SYSCTL_RESC_BOR) != 0UL)
    {
        return MCU_BROWN_OUT_RESET;
    }
    else if (((ResetCause & SYSCTL_RESC_WDT0) != 0UL) || 
             ((ResetCause & SYSCTL_RESC_WDT1) != 0UL))
    {
        return MCU_WATCHDOG_RESET;
    }
    else if ((ResetCause & SYSCTL_RESC_SW) != 0UL)
    {
        return MCU_SOFTWARE_RESET;
    }
    else
    {
        return MCU_RESET_UNDEFINED;
    }
}

/**
 * @brief Get reset raw value
 * @return Raw reset register value
 */
uint32 Mcu_GetResetRawValue(void)
{
    if (Mcu_ModuleInitialized == FALSE)
    {
        return 0UL;
    }

    return SYSCTL_RESC_R;
}

/**
 * @brief Perform microcontroller reset
 */
void Mcu_PerformReset(void)
{
    /* Trigger software reset via APINT register */
    NVIC_APINT_R = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
    
    /* Wait for reset */
    while(1)
    {
        /* Infinite loop - reset will occur */
    }
}

/**
 * @brief Set MCU power mode
 * @param McuMode Power mode to enter
 */
void Mcu_SetMode(Mcu_ModeType McuMode)
{
    if (Mcu_ModuleInitialized == FALSE || McuMode > MCU_MODE_DEEP_SLEEP)
    {
        return;
    }

    switch (McuMode)
    {
        case MCU_MODE_NORMAL:
            /* Normal mode - do nothing */
            break;
            
        case MCU_MODE_SLEEP:
            /* Ensure deep sleep bit is cleared and enter sleep */
            MCU_SCB_SCR_REG &= ~MCU_SCB_SCR_SLEEPDEEP_MASK;
            __asm("    WFI\n");  /* Wait for interrupt */
            break;
            
        case MCU_MODE_DEEP_SLEEP:
            /* Set deep sleep bit and wait for interrupt */
            MCU_SCB_SCR_REG |= MCU_SCB_SCR_SLEEPDEEP_MASK;
            __asm("    WFI\n");
            MCU_SCB_SCR_REG &= ~MCU_SCB_SCR_SLEEPDEEP_MASK;  /* Restore configuration */
            break;
            
        default:
            break;
    }
}

/**
 * @brief Get current system clock frequency
 * @return System clock frequency in Hz
 */
uint32 Mcu_GetSystemClock(void)
{
    if (Mcu_ModuleInitialized == FALSE)
    {
        return 0UL;
    }

    return Mcu_SystemClockFrequency;
}

/**
 * @brief Enable peripheral clock
 * @param Peripheral Peripheral identifier (use SYSCTL_RCGC bits)
 */
void Mcu_EnablePeripheralClock(uint32 Peripheral)
{
    if (Mcu_ModuleInitialized == FALSE)
    {
        return;
    }

    uint32 offset = MCU_PERIPHERAL_GET_OFFSET(Peripheral);
    uint32 mask = MCU_PERIPHERAL_GET_MASK(Peripheral);

    if (mask == 0UL)
    {
        return;
    }

    volatile uint32* reg = (volatile uint32*)(MCU_SYSCTL_BASE_ADDR + MCU_RCGC_OFFSET + offset);
    *reg |= mask;

    /* Small delay to allow clock to stabilize */
    Mcu_Delay(3);
}

/**
 * @brief Disable peripheral clock
 * @param Peripheral Peripheral identifier (use SYSCTL_RCGC bits)
 */
void Mcu_DisablePeripheralClock(uint32 Peripheral)
{
    if (Mcu_ModuleInitialized == FALSE)
    {
        return;
    }

    uint32 offset = MCU_PERIPHERAL_GET_OFFSET(Peripheral);
    uint32 mask = MCU_PERIPHERAL_GET_MASK(Peripheral);

    if (mask == 0UL)
    {
        return;
    }

    volatile uint32* reg = (volatile uint32*)(MCU_SYSCTL_BASE_ADDR + MCU_RCGC_OFFSET + offset);
    *reg &= ~mask;
}

/* ===================[Private Functions]=================== */

/**
 * @brief Set system clock based on configuration
 * @param ClockSetting Clock configuration to use
 */
static void Mcu_SetSystemClock(Mcu_ClockType ClockSetting)
{
    switch (ClockSetting)
    {
        case MCU_CLOCK_16MHZ:
            /* Configure for 16MHz main oscillator (no PLL) */
            /* Bypass PLL to use main oscillator directly */
            SYSCTL_RCC2_R |= 0x00000800;  /* BYPASS2 = 1 */
            SYSCTL_RCC_R |= 0x00000800;   /* BYPASS = 1 */
            /* Power down PLL to save power */
            SYSCTL_RCC2_R |= 0x00002000;  /* PWRDN2 = 1 */
            /* Clear DIV400 and SYSDIV to use raw oscillator */
            SYSCTL_RCC2_R &= ~0x40000000; /* DIV400 = 0 */
            /* Update software clock frequency */
            Mcu_SystemClockFrequency = 16000000UL;
            Mcu_TargetClockFrequency = 16000000UL;
            Mcu_PllEnabled = FALSE;
            break;
            
        case MCU_CLOCK_20MHZ:
            /* Configure PLL for 20MHz */
            Mcu_ConfigurePll(MCU_CLOCK_20MHZ);
            Mcu_TargetClockFrequency = 20000000UL;
            Mcu_PllEnabled = TRUE;
            break;
            
        case MCU_CLOCK_25MHZ:
            /* Configure PLL for 25MHz */
            Mcu_ConfigurePll(MCU_CLOCK_25MHZ);
            Mcu_TargetClockFrequency = MCU_CLOCK_FREQ_PLL_25MHZ;
            Mcu_PllEnabled = TRUE;
            break;
            
        case MCU_CLOCK_40MHZ:
            /* Configure PLL for 40MHz */
            Mcu_ConfigurePll(MCU_CLOCK_40MHZ);
            Mcu_TargetClockFrequency = MCU_CLOCK_FREQ_PLL_40MHZ;
            Mcu_PllEnabled = TRUE;
            break;
            
        case MCU_CLOCK_50MHZ:
            /* Configure PLL for 50MHz */
            Mcu_ConfigurePll(MCU_CLOCK_50MHZ);
            Mcu_TargetClockFrequency = MCU_CLOCK_FREQ_PLL_50MHZ;
            Mcu_PllEnabled = TRUE;
            break;
            
        case MCU_CLOCK_80MHZ:
            /* Configure PLL for 80MHz */
            Mcu_ConfigurePll(MCU_CLOCK_80MHZ);
            Mcu_TargetClockFrequency = MCU_CLOCK_FREQ_PLL_80MHZ;
            Mcu_PllEnabled = TRUE;
            break;
            
        default:
            /* Default to 16MHz main oscillator */
            Mcu_SystemClockFrequency = 16000000UL;
            Mcu_TargetClockFrequency = 16000000UL;
            Mcu_PllEnabled = FALSE;
            break;
    }
    
}

/**
 * @brief Configure PLL for specific frequency
 * @param ClockSetting Target clock setting
 */
static void Mcu_ConfigurePll(Mcu_ClockType ClockSetting)
{
    uint32 sysdiv2;
    
    /* Calculate SYSDIV2 value: (400MHz / target_freq) - 1 */
    switch (ClockSetting)
    {
        case MCU_CLOCK_80MHZ:  sysdiv2 = 4;  break;  /* 400/(4+1) = 80MHz */
        case MCU_CLOCK_50MHZ:  sysdiv2 = 7;  break;  /* 400/(7+1) = 50MHz */
        case MCU_CLOCK_40MHZ:  sysdiv2 = 9;  break;  /* 400/(9+1) = 40MHz */
        case MCU_CLOCK_25MHZ:  sysdiv2 = 15; break;  /* 400/(15+1) = 25MHz */
        case MCU_CLOCK_20MHZ:  sysdiv2 = 19; break;  /* 400/(19+1) = 20MHz */
        default: sysdiv2 = 4; break;
    }
    
    /* TI-recommended PLL configuration sequence for TM4C123 */
    
    /* Step 1: Bypass PLL while initializing */
    SYSCTL_RCC2_R |= 0x80000000;  /* USERCC2 */
    SYSCTL_RCC2_R |= 0x00000800;  /* BYPASS2 */
    
    /* Step 2: Select 16MHz external crystal */
    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x000007C0) | 0x00000540;  /* XTAL = 16MHz */
    
    /* Step 3: Select main oscillator as source */
    SYSCTL_RCC2_R &= ~0x00000070;  /* OSCSRC2 = MOSC */
    
    /* Step 4: Clear PWRDN to activate PLL */
    SYSCTL_RCC2_R &= ~0x00002000;  /* PWRDN2 = 0 */
    
    /* Step 5: Set DIV400 to use 400MHz PLL */
    SYSCTL_RCC2_R |= 0x40000000;  /* DIV400 */
    
    /* Step 6: Set SYSDIV2 for desired frequency */
    SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) | (sysdiv2 << 22);
    
    /* Step 7: Wait for PLL to lock */
    while ((SYSCTL_PLLSTAT_R & 0x01) == 0) {
        /* Busy wait for PLL lock */
    }
    
    /* Step 8: Clear BYPASS2 to use PLL (done in DistributePllClock) */
}

/**
 * @brief Simple delay function
 * @param Count Number of iterations to delay
 */
static void Mcu_Delay(uint32 Count)
{
    volatile uint32 Delay = Count;
    while (Delay > 0UL)
    {
        Delay--;
    }

}

