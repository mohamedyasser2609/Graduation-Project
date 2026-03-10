/**
 * @file MPU.c
 * @brief Memory Protection Unit (MPU) Driver Implementation
 * @details Configures ARM Cortex-M4 MPU for privilege separation
 *
 * SAFETY DESIGN:
 * - Watchdog registers: Privileged access ONLY
 * - PWM registers: Privileged access ONLY
 * - MemManage fault on violation → logged + safe state
 *
 * @author Mohamed Yasser
 * @date Jan 09, 2026
 * @version 1.0.0
 */

#include "MPU.h"
#include "../../CONFIG/Std_Types.h"

/* System control registers for fault handling */
#define SCB_SHCSR_R         (*((volatile uint32*)0xE000ED24u))
#define SCB_CFSR_R          (*((volatile uint32*)0xE000ED28u))
#define SCB_MMFAR_R         (*((volatile uint32*)0xE000ED34u))

/* SHCSR bits */
#define SHCSR_MEMFAULTENA   (1u << 16u)

/* CFSR MemManage bits */
#define CFSR_MMARVALID      (1u << 7u)
#define CFSR_MSTKERR        (1u << 4u)
#define CFSR_MUNSTKERR      (1u << 3u)
#define CFSR_DACCVIOL       (1u << 1u)
#define CFSR_IACCVIOL       (1u << 0u)

/* ===================[Private Variables]=================== */
static MPU_FaultInfoType MPU_LastFault = {0};
static boolean MPU_Initialized = FALSE;

/* ===================[Private Functions]=================== */

/**
 * @brief Configure a single MPU region
 */
static void MPU_ConfigureRegion(uint8 region, uint32 baseAddr, uint32 size, 
                                 uint8 accessPerm, boolean executable)
{
    uint32 rasrValue;
    
    /* Select region */
    MPU_RNR_R = region;
    
    /* Set base address (must be aligned to region size) */
    MPU_RBAR_R = baseAddr;
    
    /* Build RASR value */
    rasrValue = (size << MPU_RASR_SIZE_SHIFT) |          /* Region size */
                (accessPerm << MPU_RASR_AP_SHIFT) |       /* Access permissions */
                (executable ? 0u : (1u << MPU_RASR_XN_SHIFT)) | /* Execute never */
                (1u << MPU_RASR_S_SHIFT) |                /* Shareable */
                (1u << MPU_RASR_C_SHIFT) |                /* Cacheable */
                (1u << MPU_RASR_B_SHIFT) |                /* Bufferable */
                MPU_RASR_ENABLE;                          /* Enable region */
    
    MPU_RASR_R = rasrValue;
}

/* ===================[Public Functions]=================== */

uint8 MPU_GetRegionCount(void)
{
    /* Read DREGION field from MPU_TYPE register */
    return (uint8)((MPU_TYPE_R >> 8u) & 0xFFu);
}

void MPU_Init(void)
{
    uint8 regionCount;
    
    /* Prevent re-initialization */
    if (MPU_Initialized)
    {
        return;
    }
    
    /* Check if MPU is present */
    regionCount = MPU_GetRegionCount();
    if (regionCount == 0u)
    {
        /* No MPU - cannot continue with protection */
        return;
    }
    
    /* Disable MPU during configuration */
    MPU_CTRL_R = 0u;
    
    /* Enable MemManage fault handler */
    SCB_SHCSR_R |= SHCSR_MEMFAULTENA;
    
    /*
     * REGION 0: Flash (256KB) - Full access, executable
     * Base: 0x00000000, Size: 256KB
     */
    MPU_ConfigureRegion(
        MPU_REGION_FLASH,
        TM4C_FLASH_BASE,
        MPU_SIZE_256KB,
        MPU_AP_FULL_ACCESS,
        TRUE  /* Executable */
    );
    
    /*
     * REGION 1: SRAM (32KB) - Full access, no execute
     * Base: 0x20000000, Size: 32KB
     */
    MPU_ConfigureRegion(
        MPU_REGION_SRAM,
        TM4C_SRAM_BASE,
        MPU_SIZE_32KB,
        MPU_AP_FULL_ACCESS,
        FALSE  /* No execute */
    );
    
    /*
     * REGION 2: Peripherals (1MB) - Default full access
     * Base: 0x40000000, Size: 1MB
     * This is the default - will be overridden by more specific regions
     */
    MPU_ConfigureRegion(
        MPU_REGION_PERIPHERALS,
        TM4C_PERIPH_BASE,
        MPU_SIZE_1MB,
        MPU_AP_FULL_ACCESS,
        FALSE
    );
    
    /*
     * REGION 3: WATCHDOG - PRIVILEGED ACCESS ONLY
     * Base: 0x40000000 (WDT0), Size: 4KB
     * 
     * SAFETY CRITICAL: Only Safety Task (privileged) can access WDG
     * Any unprivileged access will trigger MemManage fault
     */
    MPU_ConfigureRegion(
        MPU_REGION_WATCHDOG,
        TM4C_WDT0_BASE,
        MPU_SIZE_4KB,
        MPU_AP_PRIV_RW,  /* PRIVILEGED READ-WRITE ONLY */
        FALSE
    );
    
    /*
     * REGION 4: PWM - PRIVILEGED ACCESS ONLY
     * Base: 0x40028000 (PWM0), Size: 4KB
     *
     * SAFETY CRITICAL: Only Control Task (privileged) should access PWM
     * Prevents unauthorized motor control
     */
    MPU_ConfigureRegion(
        MPU_REGION_PWM,
        TM4C_PWM0_BASE,
        MPU_SIZE_4KB,
        MPU_AP_PRIV_RW,  /* PRIVILEGED READ-WRITE ONLY */
        FALSE
    );
    
    /*
     * REGION 5: PWM1 - PRIVILEGED ACCESS ONLY
     * Base: 0x40029000 (PWM1), Size: 4KB
     */
    MPU_ConfigureRegion(
        MPU_REGION_MOTOR_GPIO,
        TM4C_PWM1_BASE,
        MPU_SIZE_4KB,
        MPU_AP_PRIV_RW,
        FALSE
    );
    
    /*
     * Enable MPU with:
     * - PRIVDEFENA: Privileged code uses default memory map for undefined regions
     * - ENABLE: Turn on MPU
     *
     * Note: All FreeRTOS tasks run in privileged mode by default,
     * so this protects against future unprivileged mode implementation
     */
    MPU_CTRL_R = MPU_CTRL_PRIVDEFENA | MPU_CTRL_ENABLE;
    
    /* Memory barrier to ensure MPU is configured before continuing */
    __asm("    DSB");
    __asm("    ISB");
    
    MPU_Initialized = TRUE;
}

void MPU_Enable(void)
{
    MPU_CTRL_R |= MPU_CTRL_ENABLE;
    __asm("    DSB");
    __asm("    ISB");
}

void MPU_Disable(void)
{
    MPU_CTRL_R &= ~MPU_CTRL_ENABLE;
    __asm("    DSB");
    __asm("    ISB");
}

Std_ReturnType MPU_GetFaultInfo(MPU_FaultInfoType* FaultInfo)
{
    if (FaultInfo == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    *FaultInfo = MPU_LastFault;
    return E_OK;
}

void MPU_ClearFaultInfo(void)
{
    MPU_LastFault.FaultAddress = 0u;
    MPU_LastFault.FaultCount = 0u;
    MPU_LastFault.LastRegionViolated = 0u;
    MPU_LastFault.IsValid = FALSE;
}

/**
 * @brief MemManage Fault Handler
 * @note Called when MPU violation occurs
 */
void MPU_MemManageFaultHandler(void)
{
    uint32 cfsr = SCB_CFSR_R;
    
    /* Record fault information */
    MPU_LastFault.FaultCount++;
    MPU_LastFault.IsValid = TRUE;
    
    /* Check if fault address is valid */
    if (cfsr & CFSR_MMARVALID)
    {
        MPU_LastFault.FaultAddress = SCB_MMFAR_R;
        
        /* Determine which region was violated */
        if ((MPU_LastFault.FaultAddress >= TM4C_WDT0_BASE) &&
            (MPU_LastFault.FaultAddress < (TM4C_WDT0_BASE + 0x1000u)))
        {
            MPU_LastFault.LastRegionViolated = MPU_REGION_WATCHDOG;
        }
        else if ((MPU_LastFault.FaultAddress >= TM4C_PWM0_BASE) &&
                 (MPU_LastFault.FaultAddress < (TM4C_PWM0_BASE + 0x1000u)))
        {
            MPU_LastFault.LastRegionViolated = MPU_REGION_PWM;
        }
    }
    
    /* Clear the fault status */
    SCB_CFSR_R = cfsr;
    
    /*
     * SAFETY ACTION: Stop all motors and enter safe state
     * We cannot continue safely after an MPU violation
     */
    
    /* Disable interrupts */
    __asm("    CPSID I");
    
    /* Stop motors directly (bypass any abstractions) */
    /* PWM0 generator outputs - set to 0 */
    (*((volatile uint32*)0x40028060u)) = 0u;  /* PWM0_0_CMPA */
    (*((volatile uint32*)0x400280A0u)) = 0u;  /* PWM0_1_CMPA */
    (*((volatile uint32*)0x400280E0u)) = 0u;  /* PWM0_2_CMPA */
    
    /* Infinite loop - requires reset to recover */
    /* In production, could trigger WDG reset instead */
    for (;;)
    {
        /* 
         * Optional: Toggle LED to indicate fault
         * Or wait for WDG reset
         */
    }
}

/**
 * @brief Alias for vector table
 */
void MemManage_Handler(void)
{
    MPU_MemManageFaultHandler();
}
