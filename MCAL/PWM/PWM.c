/*
 * @file PWM.c
 * @brief PWM Driver Implementation for TM4C123GH6PM
 * @details Complete AUTOSAR-compliant PWM driver with hardware register access
 *
 * Hardware Details:
 * - TM4C123 has 2 PWM modules (PWM0 and PWM1)
 * - Each module has 4 generators (0-3)
 * - Each generator has 2 outputs (A and B)
 * - Total: 16 PWM outputs available
 *
 * RGB LED Mapping:
 * - PF1 (Red)   → M1PWM5 (Module 1, Gen 2, Output B)
 * - PF2 (Blue)  → M1PWM6 (Module 1, Gen 3, Output A)
 * - PF3 (Green) → M1PWM7 (Module 1, Gen 3, Output B)
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 */

/* ===================[Includes]=================== */
#include "PWM.h"
#include "../GPIO/Gpio.h"

/* ===================[Hardware Register Definitions]=================== */

/* System Control Registers */
#define SYSCTL_RCGCPWM_R        (*((volatile uint32*)0x400FE640UL))  /* PWM Run Mode Clock Gating */
#define SYSCTL_RCC_R            (*((volatile uint32*)0x400FE060UL))  /* Run-Mode Clock Configuration */

/* PWM Module 0 Base Address */
#define PWM0_BASE               (0x40028000UL)

/* PWM Module 1 Base Address */
#define PWM1_BASE               (0x40029000UL)

/* PWM Register Offsets (from module base) */
#define PWM_CTL_OFFSET          (0x000UL)   /* PWM Master Control */
#define PWM_SYNC_OFFSET         (0x004UL)   /* PWM Time Base Sync */
#define PWM_ENABLE_OFFSET       (0x008UL)   /* PWM Output Enable */
#define PWM_INVERT_OFFSET       (0x00CUL)   /* PWM Output Inversion */
#define PWM_FAULT_OFFSET        (0x010UL)   /* PWM Output Fault */
#define PWM_INTEN_OFFSET        (0x014UL)   /* PWM Interrupt Enable */
#define PWM_RIS_OFFSET          (0x018UL)   /* PWM Raw Interrupt Status */
#define PWM_ISC_OFFSET          (0x01CUL)   /* PWM Interrupt Status and Clear */
#define PWM_STATUS_OFFSET       (0x020UL)   /* PWM Status */

/* PWM Generator Register Offsets (add to base + (gen * 0x40)) */
#define PWM_GEN_OFFSET          (0x040UL)   /* Generator base offset */
#define PWM_X_CTL_OFFSET        (0x000UL)   /* Generator Control */
#define PWM_X_INTEN_OFFSET      (0x004UL)   /* Generator Interrupt Enable */
#define PWM_X_RIS_OFFSET        (0x008UL)   /* Generator Raw Interrupt Status */
#define PWM_X_ISC_OFFSET        (0x00CUL)   /* Generator Interrupt Status/Clear */
#define PWM_X_LOAD_OFFSET       (0x010UL)   /* Generator Load */
#define PWM_X_COUNT_OFFSET      (0x014UL)   /* Generator Counter */
#define PWM_X_CMPA_OFFSET       (0x018UL)   /* Generator Compare A */
#define PWM_X_CMPB_OFFSET       (0x01CUL)   /* Generator Compare B */
#define PWM_X_GENA_OFFSET       (0x020UL)   /* Generator A Control */
#define PWM_X_GENB_OFFSET       (0x024UL)   /* Generator B Control */
#define PWM_X_DBCTL_OFFSET      (0x028UL)   /* Generator Dead-Band Control */
#define PWM_X_DBRISE_OFFSET     (0x02CUL)   /* Generator Dead-Band Rising Edge */
#define PWM_X_DBFALL_OFFSET     (0x030UL)   /* Generator Dead-Band Falling Edge */

/* PWM Control Register Bits */
#define PWM_CTL_GLOBALSYNC0     (0x00000001UL)
#define PWM_CTL_GLOBALSYNC1     (0x00000002UL)
#define PWM_CTL_GLOBALSYNC2     (0x00000004UL)
#define PWM_CTL_GLOBALSYNC3     (0x00000008UL)

/* PWM Generator Control Register Bits */
#define PWM_X_CTL_ENABLE        (0x00000001UL)  /* Enable PWM Generator */
#define PWM_X_CTL_MODE          (0x00000002UL)  /* Counter Mode (0=down, 1=up-down) */
#define PWM_X_CTL_DEBUG         (0x00000004UL)  /* Debug Mode */
#define PWM_X_CTL_LOADUPD       (0x00000008UL)  /* Load Register Update Mode */
#define PWM_X_CTL_CMPAUPD       (0x00000010UL)  /* Comparator A Update Mode */
#define PWM_X_CTL_CMPBUPD       (0x00000020UL)  /* Comparator B Update Mode */

/* PWM Generator A/B Control Actions */
#define PWM_GEN_ACT_ZERO        (0x00000001UL)  /* Action on counter=0 */
#define PWM_GEN_ACT_LOAD        (0x00000002UL)  /* Action on counter=LOAD */
#define PWM_GEN_ACT_CMPAUP      (0x00000030UL)  /* Action on CMPA up */
#define PWM_GEN_ACT_CMPADOWN    (0x000000C0UL)  /* Action on CMPA down */
#define PWM_GEN_ACT_CMPBUP      (0x00000300UL)  /* Action on CMPB up */
#define PWM_GEN_ACT_CMPBDOWN    (0x00000C00UL)  /* Action on CMPB down */

/* Action Values */
#define PWM_ACT_NONE            (0x0UL)         /* Do nothing */
#define PWM_ACT_INV             (0x1UL)         /* Invert */
#define PWM_ACT_ZERO            (0x2UL)         /* Set to 0 */
#define PWM_ACT_ONE             (0x3UL)         /* Set to 1 */

/* RCC Register Bits */
#define SYSCTL_RCC_USEPWMDIV    (0x00100000UL)  /* Enable PWM Clock Divisor */
#define SYSCTL_RCC_PWMDIV_M     (0x000E0000UL)  /* PWM Clock Divider Mask */
#define SYSCTL_RCC_PWMDIV_2     (0x00000000UL)  /* Divide by 2 */
#define SYSCTL_RCC_PWMDIV_4     (0x00020000UL)  /* Divide by 4 */
#define SYSCTL_RCC_PWMDIV_8     (0x00040000UL)  /* Divide by 8 */
#define SYSCTL_RCC_PWMDIV_16    (0x00060000UL)  /* Divide by 16 */
#define SYSCTL_RCC_PWMDIV_32    (0x00080000UL)  /* Divide by 32 */
#define SYSCTL_RCC_PWMDIV_64    (0x000A0000UL)  /* Divide by 64 */

/* ===================[Macros]=================== */

/* Get PWM module base address */
#define PWM_GET_BASE(module) \
    ((module) == PWM_MODULE_0 ? PWM0_BASE : PWM1_BASE)

/* Get generator register address */
#define PWM_GET_GEN_REG(base, gen, offset) \
    (*((volatile uint32*)((base) + PWM_GEN_OFFSET + ((gen) * 0x40UL) + (offset))))

/* Get module register address */
#define PWM_GET_MOD_REG(base, offset) \
    (*((volatile uint32*)((base) + (offset))))

/* ===================[Static Variables]=================== */

/* Pointer to current configuration */
static const Pwm_ConfigType* Pwm_ConfigPtr = NULL_PTR;

/* PWM module initialization status */
static boolean Pwm_Initialized = FALSE;

/* ===================[Static Functions]=================== */

/**
 * @brief Enable PWM module clock
 */
static void Pwm_EnableClock(Pwm_ModuleType Module) {
    volatile uint32 delay;
    
    if (Module == PWM_MODULE_0) {
        SYSCTL_RCGCPWM_R |= 0x01;  /* Enable PWM0 clock */
    } else {
        SYSCTL_RCGCPWM_R |= 0x02;  /* Enable PWM1 clock */
    }
    
    /* Wait for clock to stabilize - CRITICAL! */
    /* TM4C123 requires 3 clock cycles minimum */
    delay = SYSCTL_RCGCPWM_R;
    delay = SYSCTL_RCGCPWM_R;
    delay = SYSCTL_RCGCPWM_R;
    (void)delay;
}

/**
 * @brief Configure PWM clock divider
 */
static void Pwm_ConfigureClockDivider(void) {
    uint32 dividerValue;
    
    /* Select divider based on configuration */
    switch(PWM_CLOCK_DIVIDER) {
        case 2:  dividerValue = SYSCTL_RCC_PWMDIV_2;  break;
        case 4:  dividerValue = SYSCTL_RCC_PWMDIV_4;  break;
        case 8:  dividerValue = SYSCTL_RCC_PWMDIV_8;  break;
        case 16: dividerValue = SYSCTL_RCC_PWMDIV_16; break;
        case 32: dividerValue = SYSCTL_RCC_PWMDIV_32; break;
        case 64: dividerValue = SYSCTL_RCC_PWMDIV_64; break;
        default: dividerValue = SYSCTL_RCC_PWMDIV_64; break;
    }
    
    /* Configure RCC register */
    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~SYSCTL_RCC_PWMDIV_M) | dividerValue | SYSCTL_RCC_USEPWMDIV;
}

/**
 * @brief Initialize a single PWM generator
 */
static void Pwm_InitGenerator(const Pwm_ConfigChannelType* ChannelConfig) {
    uint32 baseAddr;
    uint8 gen;
    
    /* Safety check */
    if (ChannelConfig == NULL_PTR) {
        return;
    }
    
    /* Validate generator number */
    if (ChannelConfig->Generator > 3) {
        return;
    }
    
    baseAddr = PWM_GET_BASE(ChannelConfig->Module);
    gen = ChannelConfig->Generator;
    
    /* Disable generator during configuration */
    PWM_GET_GEN_REG(baseAddr, gen, PWM_X_CTL_OFFSET) &= ~PWM_X_CTL_ENABLE;
    
    /* Set load value (period) */
    PWM_GET_GEN_REG(baseAddr, gen, PWM_X_LOAD_OFFSET) = ChannelConfig->DefaultPeriod;
    
    /* Set compare value based on duty cycle */
    uint32 compareValue = (uint32)((ChannelConfig->DefaultDutyCycle * ChannelConfig->DefaultPeriod) / 0x8000UL);
    
    if (ChannelConfig->Output == PWM_OUT_A) {
        PWM_GET_GEN_REG(baseAddr, gen, PWM_X_CMPA_OFFSET) = compareValue;
        
        /* Configure Generator A: Clear on LOAD (start LOW), Set on CMPA down */
        PWM_GET_GEN_REG(baseAddr, gen, PWM_X_GENA_OFFSET) = 
            (PWM_ACT_ZERO << 2) |  /* Clear on LOAD (LED OFF at period start) */
            (PWM_ACT_ONE << 6);    /* Set on CMPA down (LED ON for duty cycle) */
    } else {
        PWM_GET_GEN_REG(baseAddr, gen, PWM_X_CMPB_OFFSET) = compareValue;
        
        /* Configure Generator B: Clear on LOAD (start LOW), Set on CMPB down */
        PWM_GET_GEN_REG(baseAddr, gen, PWM_X_GENB_OFFSET) = 
            (PWM_ACT_ZERO << 2) |  /* Clear on LOAD (LED OFF at period start) */
            (PWM_ACT_ONE << 10);   /* Set on CMPB down (LED ON for duty cycle) */
    }
    
    /* Configure generator control: Down-count mode, immediate updates */
    PWM_GET_GEN_REG(baseAddr, gen, PWM_X_CTL_OFFSET) = PWM_X_CTL_ENABLE;
    
    /* Enable PWM output */
    uint8 outputBit = (gen * 2) + ChannelConfig->Output;
    PWM_GET_MOD_REG(baseAddr, PWM_ENABLE_OFFSET) |= (1 << outputBit);
    
    /* Configure polarity (inversion) */
    if (ChannelConfig->Polarity == PWM_LOW_POLARITY) {
        PWM_GET_MOD_REG(baseAddr, PWM_INVERT_OFFSET) |= (1 << outputBit);
    } else {
        PWM_GET_MOD_REG(baseAddr, PWM_INVERT_OFFSET) &= ~(1 << outputBit);
    }
}

/**
 * @brief Find channel configuration by channel number
 */
static const Pwm_ConfigChannelType* Pwm_FindChannel(Pwm_ChannelType ChannelNumber) {
    uint8 i;
    
    if (Pwm_ConfigPtr == NULL_PTR) {
        return NULL_PTR;
    }
    
    for (i = 0; i < Pwm_ConfigPtr->NumChannels; i++) {
        if (Pwm_ConfigPtr->Channels[i].Channel == ChannelNumber) {
            return &Pwm_ConfigPtr->Channels[i];
        }
    }
    
    return NULL_PTR;
}

/* ===================[Public Functions]=================== */

void Pwm_Init(const Pwm_ConfigType* ConfigPtr) {
    uint8 i;
    
    /* Parameter check */
    if (ConfigPtr == NULL_PTR || ConfigPtr->Channels == NULL_PTR || ConfigPtr->NumChannels == 0) {
        return;
    }
    
    /* Store configuration pointer */
    Pwm_ConfigPtr = ConfigPtr;
    
    /* Configure PWM clock divider */
    Pwm_ConfigureClockDivider();
    
    /* Enable clocks for used modules */
    for (i = 0; i < ConfigPtr->NumChannels; i++) {
        Pwm_EnableClock(ConfigPtr->Channels[i].Module);
    }
    
    /* Additional delay to ensure clock is fully stable */
    {
        volatile uint32 delay;
        for (delay = 0; delay < 100; delay++) {
            /* Wait */
        }
    }
    
    /* Initialize each configured channel */
    for (i = 0; i < ConfigPtr->NumChannels; i++) {
        Pwm_InitGenerator(&ConfigPtr->Channels[i]);
    }
    
    /* Mark as initialized */
    Pwm_Initialized = TRUE;
}

#if (PWM_DE_INIT_API == TRUE)
void Pwm_DeInit(void) {
    uint8 i;
    
    if (Pwm_Initialized == FALSE || Pwm_ConfigPtr == NULL_PTR) {
        return;
    }
    
    /* Disable all configured channels */
    for (i = 0; i < Pwm_ConfigPtr->NumChannels; i++) {
        const Pwm_ConfigChannelType* channel = &Pwm_ConfigPtr->Channels[i];
        uint32 baseAddr = PWM_GET_BASE(channel->Module);
        uint8 gen = channel->Generator;
        uint8 outputBit = (gen * 2) + channel->Output;
        
        /* Disable PWM output */
        PWM_GET_MOD_REG(baseAddr, PWM_ENABLE_OFFSET) &= ~(1 << outputBit);
        
        /* Disable generator */
        PWM_GET_GEN_REG(baseAddr, gen, PWM_X_CTL_OFFSET) &= ~PWM_X_CTL_ENABLE;
    }
    
    /* Clear configuration pointer */
    Pwm_ConfigPtr = NULL_PTR;
    Pwm_Initialized = FALSE;
}
#endif

#if (PWM_SET_DUTY_CYCLE_API == TRUE)
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, Pwm_DutyCycleType DutyCycle) {
    const Pwm_ConfigChannelType* channel;
    uint32 baseAddr;
    uint32 compareValue;
    uint32 period;
    
    if (Pwm_Initialized == FALSE) {
        return;
    }
    
    /* Find channel configuration */
    channel = Pwm_FindChannel(ChannelNumber);
    if (channel == NULL_PTR) {
        return;
    }
    
    /* Get base address and period */
    baseAddr = PWM_GET_BASE(channel->Module);
    period = PWM_GET_GEN_REG(baseAddr, channel->Generator, PWM_X_LOAD_OFFSET);
    
    /* Calculate compare value from duty cycle */
    /* DutyCycle: 0x0000 = 0%, 0x8000 = 100% */
    compareValue = (uint32)((DutyCycle * period) / 0x8000UL);
    
    /* Limit to period */
    if (compareValue > period) {
        compareValue = period;
    }
    
    /* Update compare register */
    if (channel->Output == PWM_OUT_A) {
        PWM_GET_GEN_REG(baseAddr, channel->Generator, PWM_X_CMPA_OFFSET) = compareValue;
    } else {
        PWM_GET_GEN_REG(baseAddr, channel->Generator, PWM_X_CMPB_OFFSET) = compareValue;
    }
}
#endif

#if (PWM_SET_PERIOD_AND_DUTY_API == TRUE)
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber, Pwm_PeriodType Period, Pwm_DutyCycleType DutyCycle) {
    const Pwm_ConfigChannelType* channel;
    uint32 baseAddr;
    uint32 compareValue;
    
    if (Pwm_Initialized == FALSE) {
        return;
    }
    
    /* Find channel configuration */
    channel = Pwm_FindChannel(ChannelNumber);
    if (channel == NULL_PTR) {
        return;
    }
    
    /* Only variable period channels can change period */
    if (channel->ChannelClass != PWM_VARIABLE_PERIOD) {
        return;
    }
    
    /* Get base address */
    baseAddr = PWM_GET_BASE(channel->Module);
    
    /* Set new period */
    PWM_GET_GEN_REG(baseAddr, channel->Generator, PWM_X_LOAD_OFFSET) = Period;
    
    /* Calculate and set compare value */
    compareValue = (uint32)((DutyCycle * Period) / 0x8000UL);
    
    if (channel->Output == PWM_OUT_A) {
        PWM_GET_GEN_REG(baseAddr, channel->Generator, PWM_X_CMPA_OFFSET) = compareValue;
    } else {
        PWM_GET_GEN_REG(baseAddr, channel->Generator, PWM_X_CMPB_OFFSET) = compareValue;
    }
}
#endif

#if (PWM_SET_OUTPUT_TO_IDLE_API == TRUE)
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber) {
    const Pwm_ConfigChannelType* channel;
    uint32 baseAddr;
    uint8 outputBit;
    
    if (Pwm_Initialized == FALSE) {
        return;
    }
    
    /* Find channel configuration */
    channel = Pwm_FindChannel(ChannelNumber);
    if (channel == NULL_PTR) {
        return;
    }
    
    /* Get base address and output bit */
    baseAddr = PWM_GET_BASE(channel->Module);
    outputBit = (channel->Generator * 2) + channel->Output;
    
    /* Disable PWM output (sets to idle state) */
    PWM_GET_MOD_REG(baseAddr, PWM_ENABLE_OFFSET) &= ~(1 << outputBit);
}
#endif

#if (PWM_GET_OUTPUT_STATE_API == TRUE)
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber) {
    const Pwm_ConfigChannelType* channel;
    uint32 baseAddr;
    uint8 outputBit;
    uint32 enableReg;
    
    if (Pwm_Initialized == FALSE) {
        return PWM_LOW;
    }
    
    /* Find channel configuration */
    channel = Pwm_FindChannel(ChannelNumber);
    if (channel == NULL_PTR) {
        return PWM_LOW;
    }
    
    /* Get base address and output bit */
    baseAddr = PWM_GET_BASE(channel->Module);
    outputBit = (channel->Generator * 2) + channel->Output;
    
    /* Read enable register */
    enableReg = PWM_GET_MOD_REG(baseAddr, PWM_ENABLE_OFFSET);
    
    /* Return state */
    return ((enableReg & (1 << outputBit)) != 0) ? PWM_HIGH : PWM_LOW;
}
#endif

#if (PWM_VERSION_INFO_API == TRUE)
void Pwm_GetVersionInfo(Std_VersionInfoType* VersionInfo) {
    if (VersionInfo != NULL_PTR) {
        VersionInfo->vendorID = 0;
        VersionInfo->moduleID = 0;
        VersionInfo->sw_major_version = 1;
        VersionInfo->sw_minor_version = 0;
        VersionInfo->sw_patch_version = 0;
    }
}
#endif

/* ===================[Helper Functions]=================== */

void Pwm_SetDutyCyclePercent(Pwm_ChannelType ChannelNumber, uint8 Percentage) {
    Pwm_DutyCycleType dutyCycle;
    
    /* Limit percentage to 100 */
    if (Percentage > 100) {
        Percentage = 100;
    }
    
    /* Convert percentage to duty cycle value */
    /* 0% = 0x0000, 100% = 0x8000 */
    dutyCycle = (Pwm_DutyCycleType)((Percentage * 0x8000UL) / 100);
    
    /* Set duty cycle */
    Pwm_SetDutyCycle(ChannelNumber, dutyCycle);
}

void Pwm_SetFrequency(Pwm_ChannelType ChannelNumber, uint16 FrequencyHz) {
    const Pwm_ConfigChannelType* channel;
    uint32 pwmClock;
    uint32 period;
    
    if (Pwm_Initialized == FALSE || FrequencyHz == 0) {
        return;
    }
    
    /* Find channel configuration */
    channel = Pwm_FindChannel(ChannelNumber);
    if (channel == NULL_PTR) {
        return;
    }
    
    /* Only variable period channels can change frequency */
    if (channel->ChannelClass != PWM_VARIABLE_PERIOD) {
        return;
    }
    
    /* Calculate PWM clock frequency */
    /* System clock (16 MHz) / PWM divider */
    pwmClock = 16000000UL / PWM_CLOCK_DIVIDER;
    
    /* Calculate period for desired frequency */
    period = pwmClock / FrequencyHz;
    
    /* Limit period to 16-bit value */
    if (period > 0xFFFF) {
        period = 0xFFFF;
    }
    
    /* Get current duty cycle */
    uint32 baseAddr = PWM_GET_BASE(channel->Module);
    uint32 currentCompare;
    uint32 currentPeriod = PWM_GET_GEN_REG(baseAddr, channel->Generator, PWM_X_LOAD_OFFSET);
    
    if (channel->Output == PWM_OUT_A) {
        currentCompare = PWM_GET_GEN_REG(baseAddr, channel->Generator, PWM_X_CMPA_OFFSET);
    } else {
        currentCompare = PWM_GET_GEN_REG(baseAddr, channel->Generator, PWM_X_CMPB_OFFSET);
    }
    
    /* Calculate duty cycle as 0x0000-0x8000 range */
    Pwm_DutyCycleType dutyCycle = (Pwm_DutyCycleType)((currentCompare * 0x8000UL) / currentPeriod);
    
    /* Set new period and maintain duty cycle */
    Pwm_SetPeriodAndDuty(ChannelNumber, (Pwm_PeriodType)period, dutyCycle);
}




