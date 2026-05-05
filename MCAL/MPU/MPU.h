/**
 * @file MPU.h
 * @brief Memory Protection Unit (MPU) Driver for TM4C123GH6PM
 * @details Configures ARM Cortex-M4 MPU for privilege separation
 *
 * Protection Strategy:
 * - Region 0: Flash (code) - execute, read-only
 * - Region 1: SRAM (data) - read-write, no execute
 * - Region 2: Peripherals - privileged access only
 * - Region 3: Watchdog - privileged access only (CRITICAL)
 * - Region 4: PWM - privileged access only (motor safety)
 *
 * @author Mohamed Yasser
 * @date Jan 09, 2026
 * @version 1.0.0
 */

#ifndef MCAL_MPU_MPU_H
#define MCAL_MPU_MPU_H

#include "../../CONFIG/Std_Types.h"

/* ===================[MPU Register Definitions]=================== */
#define MPU_TYPE_R          (*((volatile uint32*)0xE000ED90u))
#define MPU_CTRL_R          (*((volatile uint32*)0xE000ED94u))
#define MPU_RNR_R           (*((volatile uint32*)0xE000ED98u))
#define MPU_RBAR_R          (*((volatile uint32*)0xE000ED9Cu))
#define MPU_RASR_R          (*((volatile uint32*)0xE000EDA0u))

/* MPU_CTRL bits */
#define MPU_CTRL_ENABLE         (1u << 0u)
#define MPU_CTRL_HFNMIENA       (1u << 1u)
#define MPU_CTRL_PRIVDEFENA     (1u << 2u)

/* MPU_RASR bits */
#define MPU_RASR_ENABLE         (1u << 0u)
#define MPU_RASR_SIZE_SHIFT     (1u)
#define MPU_RASR_SRD_SHIFT      (8u)
#define MPU_RASR_B_SHIFT        (16u)
#define MPU_RASR_C_SHIFT        (17u)
#define MPU_RASR_S_SHIFT        (18u)
#define MPU_RASR_TEX_SHIFT      (19u)
#define MPU_RASR_AP_SHIFT       (24u)
#define MPU_RASR_XN_SHIFT       (28u)

/* Access Permission values */
#define MPU_AP_NO_ACCESS        (0u)    /* No access */
#define MPU_AP_PRIV_RW          (1u)    /* Privileged RW only */
#define MPU_AP_PRIV_RW_USER_RO  (2u)    /* Priv RW, User RO */
#define MPU_AP_FULL_ACCESS      (3u)    /* Full access */
#define MPU_AP_PRIV_RO          (5u)    /* Privileged RO only */
#define MPU_AP_RO               (6u)    /* Read-only */

/* Region size encoding (2^(SIZE+1) bytes) */
#define MPU_SIZE_32B            (4u)
#define MPU_SIZE_64B            (5u)
#define MPU_SIZE_128B           (6u)
#define MPU_SIZE_256B           (7u)
#define MPU_SIZE_512B           (8u)
#define MPU_SIZE_1KB            (9u)
#define MPU_SIZE_2KB            (10u)
#define MPU_SIZE_4KB            (11u)
#define MPU_SIZE_8KB            (12u)
#define MPU_SIZE_16KB           (13u)
#define MPU_SIZE_32KB           (14u)
#define MPU_SIZE_64KB           (15u)
#define MPU_SIZE_128KB          (16u)
#define MPU_SIZE_256KB          (17u)
#define MPU_SIZE_512KB          (18u)
#define MPU_SIZE_1MB            (19u)
#define MPU_SIZE_4GB            (31u)

/* ===================[Region Definitions]=================== */
#define MPU_REGION_FLASH        (0u)
#define MPU_REGION_SRAM         (1u)
#define MPU_REGION_PERIPHERALS  (2u)
#define MPU_REGION_WATCHDOG     (3u)
#define MPU_REGION_PWM          (4u)
#define MPU_REGION_MOTOR_GPIO   (5u)

/* TM4C123GH6PM memory map */
#define TM4C_FLASH_BASE         (0x00000000u)
#define TM4C_SRAM_BASE          (0x20000000u)
#define TM4C_PERIPH_BASE        (0x40000000u)
#define TM4C_WDT0_BASE          (0x40000000u)
#define TM4C_WDT1_BASE          (0x40001000u)
#define TM4C_PWM0_BASE          (0x40028000u)
#define TM4C_PWM1_BASE          (0x40029000u)
#define TM4C_GPIOF_BASE         (0x40025000u)  /* Motor control pins */

/* ===================[Fault Status]=================== */
typedef struct
{
    uint32  FaultAddress;
    uint32  FaultCount;
    uint8   LastRegionViolated;
    boolean IsValid;
} MPU_FaultInfoType;

/* ===================[API Functions]=================== */

/**
 * @brief Initialize MPU with safety-critical region protection
 * @note Must be called before FreeRTOS scheduler starts
 */
void MPU_Init(void);

/**
 * @brief Check if MPU is supported on this device
 * @return Number of MPU regions (0 = not supported)
 */
uint8 MPU_GetRegionCount(void);

/**
 * @brief Enable MPU protection
 */
void MPU_Enable(void);

/**
 * @brief Disable MPU protection (for debugging only)
 */
void MPU_Disable(void);

/**
 * @brief Get last MPU fault information
 * @param[out] FaultInfo Pointer to fault info structure
 * @return E_OK if fault info available
 */
Std_ReturnType MPU_GetFaultInfo(MPU_FaultInfoType* FaultInfo);

/**
 * @brief Clear MPU fault info
 */
void MPU_ClearFaultInfo(void);

/**
 * @brief MemManage fault handler (called from vector table)
 */
void MPU_MemManageFaultHandler(void);

#endif /* MCAL_MPU_MPU_H */
