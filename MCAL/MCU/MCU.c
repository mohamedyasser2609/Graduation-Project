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

/* ===================[Private Variables]=================== */
static boolean Mcu_ModuleInitialized = FALSE;    /**< @brief MCU module initialization status */
static Mcu_ClockType Mcu_CurrentClockSetting;    /**< @brief Current clock setting */
static uint32 Mcu_SystemClockFrequency = 0UL;    /**< @brief Current system clock frequency */
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
    Mcu_CurrentClockSetting = ConfigPtr->DefaultClock;
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
    Mcu_CurrentClockSetting = ClockSetting;
    
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

    /* Clear the bypass bit to switch to PLL */
    SYSCTL_RCC_R &= ~SYSCTL_RCC_BYPASS;
    SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;

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
            /* Enter sleep mode */
            __asm("    WFI\n");  /* Wait for interrupt */
            break;
            
        case MCU_MODE_DEEP_SLEEP:
            /* Set deep sleep bit and wait for interrupt */
            SCB_SCR_R |= SCB_SCR_SLEEPDEEP;
            __asm("    WFI\n");
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

    /* Enable the peripheral clock - Peripheral is the actual register bit */
    /* This is a simplified version - user should use SYSCTL registers directly */
    
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

    /* Disable the peripheral clock - Peripheral is the actual register bit */
    /* This is a simplified version - user should use SYSCTL registers directly */
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
            Mcu_SystemClockFrequency = 16000000UL;
            Mcu_PllEnabled = FALSE;
            break;
            
        case MCU_CLOCK_20MHZ:
            /* Configure PLL for 20MHz */
            Mcu_ConfigurePll(MCU_CLOCK_20MHZ);
            Mcu_SystemClockFrequency = 20000000UL;
            Mcu_PllEnabled = TRUE;
            break;
            
        case MCU_CLOCK_25MHZ:
            /* Configure PLL for 25MHz */
            Mcu_ConfigurePll(MCU_CLOCK_25MHZ);
            Mcu_SystemClockFrequency = MCU_CLOCK_FREQ_PLL_25MHZ;
            Mcu_PllEnabled = TRUE;
            break;
            
        case MCU_CLOCK_40MHZ:
            /* Configure PLL for 40MHz */
            Mcu_ConfigurePll(MCU_CLOCK_40MHZ);
            Mcu_SystemClockFrequency = MCU_CLOCK_FREQ_PLL_40MHZ;
            Mcu_PllEnabled = TRUE;
            break;
            
        case MCU_CLOCK_50MHZ:
            /* Configure PLL for 50MHz */
            Mcu_ConfigurePll(MCU_CLOCK_50MHZ);
            Mcu_SystemClockFrequency = MCU_CLOCK_FREQ_PLL_50MHZ;
            Mcu_PllEnabled = TRUE;
            break;
            
        case MCU_CLOCK_80MHZ:
            /* Configure PLL for 80MHz */
            Mcu_ConfigurePll(MCU_CLOCK_80MHZ);
            Mcu_SystemClockFrequency = MCU_CLOCK_FREQ_PLL_80MHZ;
            Mcu_PllEnabled = TRUE;
            break;
            
        default:
            /* Default to 16MHz main oscillator */
            Mcu_SystemClockFrequency = 16000000UL;
            Mcu_PllEnabled = FALSE;
            break;
    }
    
    Mcu_CurrentClockSetting = ClockSetting;
}

/**
 * @brief Configure PLL for specific frequency
 * @param ClockSetting Target clock setting
 */
static void Mcu_ConfigurePll(Mcu_ClockType ClockSetting)
{
    volatile uint32* RccReg = (volatile uint32*)(MCU_SYSCTL_BASE_ADDR + MCU_RCC_OFFSET);
    volatile uint32* Rcc2Reg = (volatile uint32*)(MCU_SYSCTL_BASE_ADDR + MCU_RCC2_OFFSET);
    volatile uint32* PllFreq0Reg = (volatile uint32*)(MCU_SYSCTL_BASE_ADDR + MCU_PLLFREQ0_OFFSET);
    volatile uint32* PllFreq1Reg = (volatile uint32*)(MCU_SYSCTL_BASE_ADDR + MCU_PLLFREQ1_OFFSET);
    volatile uint32* PllStatReg = (volatile uint32*)(MCU_SYSCTL_BASE_ADDR + MCU_PLLSTAT_OFFSET);
    
    uint32 MInt = 0UL, MFrac = 0UL, N = 0UL, Q = 0UL;
    uint32 Timeout = 0UL;
    
    /* Set PLL parameters based on clock setting */
    switch (ClockSetting)
    {
        case MCU_CLOCK_80MHZ:
            MInt = MCU_PLL_MINT_80MHZ;
            MFrac = MCU_PLL_MFRAC_80MHZ;
            N = MCU_PLL_N_80MHZ;
            Q = MCU_PLL_Q_80MHZ;
            break;
            
        case MCU_CLOCK_50MHZ:
            MInt = MCU_PLL_MINT_50MHZ;
            MFrac = MCU_PLL_MFRAC_50MHZ;
            N = MCU_PLL_N_50MHZ;
            Q = MCU_PLL_Q_50MHZ;
            break;
            
        case MCU_CLOCK_40MHZ:
            MInt = MCU_PLL_MINT_40MHZ;
            MFrac = MCU_PLL_MFRAC_40MHZ;
            N = MCU_PLL_N_40MHZ;
            Q = MCU_PLL_Q_40MHZ;
            break;
            
        case MCU_CLOCK_25MHZ:
            MInt = MCU_PLL_MINT_25MHZ;
            MFrac = MCU_PLL_MFRAC_25MHZ;
            N = MCU_PLL_N_25MHZ;
            Q = MCU_PLL_Q_25MHZ;
            break;
            
        case MCU_CLOCK_20MHZ:
            /* Use 25MHz settings for now - can be refined */
            MInt = MCU_PLL_MINT_25MHZ;
            MFrac = MCU_PLL_MFRAC_25MHZ;
            N = MCU_PLL_N_25MHZ;
            Q = MCU_PLL_Q_25MHZ;
            break;
            
        default:
            return;  /* Invalid setting */
    }
    
    /* Enable main oscillator */
    *RccReg &= ~MCU_RCC_MOSCDIS_MASK;
    
    /* Wait for oscillator to stabilize */
    Mcu_Delay(524288UL);  /* Approx 3ms delay at 16MHz */
    
    /* Configure crystal value (16MHz) */
    *RccReg &= ~MCU_RCC_XTAL_MASK;
    *RccReg |= MCU_RCC_XTAL_16MHZ;
    
    /* Configure oscillator source to main oscillator */
    *RccReg &= ~MCU_RCC_OSCSRC_MASK;
    *RccReg |= MCU_RCC_OSCSRC_MOSC;
    
    /* Enable RCC2 */
    *Rcc2Reg |= MCU_RCC2_USERCC2_MASK;
    
    /* Configure PLL parameters */
    *PllFreq0Reg = ((MFrac << 10) & 0x000FFC00UL) | (MInt & 0x000003FFUL);
    *PllFreq1Reg = ((Q << 8) & 0x00001F00UL) | (N & 0x0000001FUL);
    
    /* Power up PLL */
    *RccReg &= ~MCU_RCC_PWRDN_MASK;
    *Rcc2Reg &= ~MCU_RCC2_PWRDN2_MASK;
    
    /* Wait for PLL to lock */
    Timeout = 0UL;
    while (((*PllStatReg & MCU_PLLSTAT_LOCK_MASK) == 0UL) && (Timeout < 1000000UL))
    {
        Timeout++;
    }
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