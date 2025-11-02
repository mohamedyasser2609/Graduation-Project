/**
 * @file Mcu_Cfg.h
 * @brief MCU Driver Configuration for TM4C123GH6PM
 * @details Configuration header for AUTOSAR-compliant MCU driver
 *
 * @author Mohamed Yasser
 * @date Nov 1, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_MCU_MCU_CFG_H_
#define MCAL_MCU_MCU_CFG_H_

#include "../../CONFIG/Std_Types.h"
#include "MCU_Types.h"

/* ===================[Helper Macros]=================== */

/**
 * @brief Encode peripheral clock register offset and bit mask into a single value
 */
#define MCU_PERIPHERAL_ENCODE(offset, mask) \
    ((((uint32)(offset)) & 0xFFFFUL) << 16) | ((uint32)(mask) & 0xFFFFUL)

/**
 * @brief Extract register offset from encoded peripheral value
 */
#define MCU_PERIPHERAL_GET_OFFSET(value)     (((uint32)(value) >> 16) & 0xFFFFUL)

/**
 * @brief Extract bit mask from encoded peripheral value
 */
#define MCU_PERIPHERAL_GET_MASK(value)       ((uint32)(value) & 0xFFFFUL)

/**
 * @brief Helper macro to encode GPIO peripheral clock masks
 */
#define MCU_PERIPHERAL_GPIO(mask)            MCU_PERIPHERAL_ENCODE(MCU_RCGCGPIO_OFFSET, (mask))

/* ===================[Pre-compile Configuration Parameters]=================== */

/**
 * @brief MCU driver pre-compile configuration switches
 */
#define MCU_DEV_ERROR_DETECT            (STD_ON)    /**< @brief Enable/Disable development error detection */
#define MCU_PERFORM_RESET_API           (STD_ON)    /**< @brief Enable/Disable Mcu_PerformReset API */
#define MCU_VERSION_INFO_API            (STD_ON)    /**< @brief Enable/Disable Mcu_GetVersionInfo API */
#define MCU_NO_PLL                      (STD_OFF)   /**< @brief Enable/Disable PLL usage */

/* ===================[API Service IDs]=================== */

/**
 * @brief Service ID for Mcu_Init
 */
#define MCU_INIT_SID                           (0x00u)

/**
 * @brief Service ID for Mcu_InitClock
 */
#define MCU_INIT_CLOCK_SID                     (0x01u)

/**
 * @brief Service ID for Mcu_DistributePllClock
 */
#define MCU_DISTRIBUTE_PLL_CLOCK_SID           (0x02u)

/**
 * @brief Service ID for Mcu_GetPllStatus
 */
#define MCU_GET_PLL_STATUS_SID                 (0x03u)

/**
 * @brief Service ID for Mcu_GetResetReason
 */
#define MCU_GET_RESET_REASON_SID               (0x04u)

/**
 * @brief Service ID for Mcu_GetResetRawValue
 */
#define MCU_GET_RESET_RAW_VALUE_SID            (0x05u)

/**
 * @brief Service ID for Mcu_PerformReset
 */
#define MCU_PERFORM_RESET_SID                  (0x06u)

/**
 * @brief Service ID for Mcu_SetMode
 */
#define MCU_SET_MODE_SID                       (0x07u)

/**
 * @brief Service ID for Mcu_GetSystemClock
 */
#define MCU_GET_SYSTEM_CLOCK_SID               (0x08u)

/**
 * @brief Service ID for Mcu_EnablePeripheralClock
 */
#define MCU_ENABLE_PERIPHERAL_CLOCK_SID        (0x09u)

/**
 * @brief Service ID for Mcu_DisablePeripheralClock
 */
#define MCU_DISABLE_PERIPHERAL_CLOCK_SID       (0x0Au)

/* ===================[DET Error Codes]=================== */

/**
 * @brief API service called with wrong parameter
 * @details Configuration pointer is NULL or invalid
 */
#define MCU_E_PARAM_CONFIG                     (0x0Au)

/**
 * @brief API Mcu_InitClock called with invalid clock setting
 * @details Clock setting is out of valid range
 */
#define MCU_E_PARAM_CLOCK                      (0x0Bu)

/**
 * @brief API Mcu_SetMode called with invalid mode
 * @details Power mode is out of valid range
 */
#define MCU_E_PARAM_MODE                       (0x0Cu)

/**
 * @brief API Mcu_DistributePllClock called before PLL is locked
 * @details PLL must be locked before distributing clock
 */
#define MCU_E_PLL_NOT_LOCKED                   (0x0Du)

/**
 * @brief API service called without module initialization
 * @details Module must be initialized before calling this API
 */
#define MCU_E_UNINIT                           (0x0Eu)

/**
 * @brief API service called with NULL pointer
 * @details A NULL pointer was passed to an API function
 */
#define MCU_E_PARAM_POINTER                    (0x0Fu)

/* ===================[Clock Configuration Parameters]=================== */

/**
 * @brief Crystal frequency in Hz (16MHz external crystal)
 */
#define MCU_CRYSTAL_FREQ                (16000000UL)

/**
 * @brief System clock frequencies for different configurations
 */
#define MCU_CLOCK_FREQ_MOSC_16MHZ       (16000000UL)    /**< @brief Main oscillator 16MHz */
#define MCU_CLOCK_FREQ_PLL_80MHZ        (80000000UL)    /**< @brief PLL 80MHz */
#define MCU_CLOCK_FREQ_PLL_50MHZ        (50000000UL)    /**< @brief PLL 50MHz */
#define MCU_CLOCK_FREQ_PLL_40MHZ        (40000000UL)    /**< @brief PLL 40MHz */
#define MCU_CLOCK_FREQ_PLL_25MHZ        (25000000UL)    /**< @brief PLL 25MHz */
#define MCU_CLOCK_FREQ_PIOSC_16MHZ      (16000000UL)    /**< @brief Precision internal oscillator 16MHz */

/* ===================[PLL Configuration Parameters]=================== */

/**
 * @brief PLL configuration values for different frequencies
 */
#define MCU_PLL_MINT_80MHZ              (4U)        /**< @brief PLL M integer value for 80MHz */
#define MCU_PLL_MFRAC_80MHZ             (0U)        /**< @brief PLL M fractional value for 80MHz */
#define MCU_PLL_N_80MHZ                 (1U)        /**< @brief PLL N value for 80MHz */
#define MCU_PLL_Q_80MHZ                 (2U)        /**< @brief PLL Q value for 80MHz */

#define MCU_PLL_MINT_50MHZ              (25U)       /**< @brief PLL M integer value for 50MHz */
#define MCU_PLL_MFRAC_50MHZ             (0U)        /**< @brief PLL M fractional value for 50MHz */
#define MCU_PLL_N_50MHZ                 (5U)        /**< @brief PLL N value for 50MHz */
#define MCU_PLL_Q_50MHZ                 (2U)        /**< @brief PLL Q value for 50MHz */

#define MCU_PLL_MINT_40MHZ              (25U)       /**< @brief PLL M integer value for 40MHz */
#define MCU_PLL_MFRAC_40MHZ             (0U)        /**< @brief PLL M fractional value for 40MHz */
#define MCU_PLL_N_40MHZ                 (5U)        /**< @brief PLL N value for 40MHz */
#define MCU_PLL_Q_40MHZ                 (4U)        /**< @brief PLL Q value for 40MHz */

#define MCU_PLL_MINT_25MHZ              (25U)       /**< @brief PLL M integer value for 25MHz */
#define MCU_PLL_MFRAC_25MHZ             (0U)        /**< @brief PLL M fractional value for 25MHz */
#define MCU_PLL_N_25MHZ                 (5U)        /**< @brief PLL N value for 25MHz */
#define MCU_PLL_Q_25MHZ                 (8U)        /**< @brief PLL Q value for 25MHz */

/* ===================[System Control Register Addresses]=================== */

/**
 * @brief System Control Register Base Address
 */
#define MCU_SYSCTL_BASE_ADDR            (0x400FE000UL)

/**
 * @brief System Control Register Offsets
 */
#define MCU_RCC_OFFSET                  (0x060UL)   /**< @brief Run-Mode Clock Configuration */
#define MCU_RCC2_OFFSET                 (0x070UL)   /**< @brief Run-Mode Clock Configuration 2 */
#define MCU_PLLFREQ0_OFFSET             (0x160UL)   /**< @brief PLL Frequency 0 */
#define MCU_PLLFREQ1_OFFSET             (0x164UL)   /**< @brief PLL Frequency 1 */
#define MCU_PLLSTAT_OFFSET              (0x168UL)   /**< @brief PLL Status */
#define MCU_RESC_OFFSET                 (0x05CUL)   /**< @brief Reset Cause */
#define MCU_RCGC_OFFSET                 (0x600UL)   /**< @brief Run-Mode Clock Gating Control Base */

/* ===================[Peripheral Clock Gating Offsets]=================== */

/**
 * @brief Peripheral Clock Gating Control Register Offsets
 */
#define MCU_RCGCWD_OFFSET               (0x000UL)   /**< @brief Watchdog Timer */
#define MCU_RCGCTIMER_OFFSET            (0x004UL)   /**< @brief Timer */
#define MCU_RCGCGPIO_OFFSET             (0x008UL)   /**< @brief GPIO */
#define MCU_RCGCDMA_OFFSET              (0x00CUL)   /**< @brief DMA */
#define MCU_RCGCHIB_OFFSET              (0x014UL)   /**< @brief Hibernation */
#define MCU_RCGCUART_OFFSET             (0x018UL)   /**< @brief UART */
#define MCU_RCGCSSI_OFFSET              (0x01CUL)   /**< @brief SSI */
#define MCU_RCGCI2C_OFFSET              (0x020UL)   /**< @brief I2C */
#define MCU_RCGCUSB_OFFSET              (0x028UL)   /**< @brief USB */
#define MCU_RCGCCAN_OFFSET              (0x034UL)   /**< @brief CAN */
#define MCU_RCGCADC_OFFSET              (0x038UL)   /**< @brief ADC */
#define MCU_RCGCACMP_OFFSET             (0x03CUL)   /**< @brief Analog Comparator */
#define MCU_RCGCPWM_OFFSET              (0x040UL)   /**< @brief PWM */
#define MCU_RCGCQEI_OFFSET              (0x044UL)   /**< @brief QEI */
#define MCU_RCGCEEPROM_OFFSET           (0x058UL)   /**< @brief EEPROM */
#define MCU_RCGCWTIMER_OFFSET           (0x05CUL)   /**< @brief Wide Timer */

/* ===================[RCC Register Bit Definitions]=================== */

/**
 * @brief RCC Register Bit Fields
 */
#define MCU_RCC_MOSCDIS_MASK            (0x00000001UL)   /**< @brief Main Oscillator Disable */
#define MCU_RCC_OSCSRC_MASK             (0x00000030UL)   /**< @brief Oscillator Source */
#define MCU_RCC_OSCSRC_MOSC             (0x00000000UL)   /**< @brief Main Oscillator */
#define MCU_RCC_OSCSRC_PIOSC            (0x00000010UL)   /**< @brief Precision Internal Oscillator */
#define MCU_RCC_OSCSRC_PIOSC_D4         (0x00000020UL)   /**< @brief Precision Internal Oscillator/4 */
#define MCU_RCC_OSCSRC_30K              (0x00000030UL)   /**< @brief 30KHz Internal Oscillator */
#define MCU_RCC_XTAL_MASK               (0x000007C0UL)   /**< @brief Crystal Value */
#define MCU_RCC_XTAL_16MHZ              (0x00000540UL)   /**< @brief 16MHz Crystal */
#define MCU_RCC_BYPASS_MASK             (0x00000800UL)   /**< @brief PLL Bypass */
#define MCU_RCC_PWRDN_MASK              (0x00002000UL)   /**< @brief PLL Power Down */
#define MCU_RCC_USESYSDIV_MASK          (0x00400000UL)   /**< @brief Enable System Clock Divider */
#define MCU_RCC_SYSDIV_MASK             (0x07800000UL)   /**< @brief System Clock Divisor */

/* ===================[RCC2 Register Bit Definitions]=================== */

/**
 * @brief RCC2 Register Bit Fields
 */
#define MCU_RCC2_USERCC2_MASK           (0x80000000UL)   /**< @brief Use RCC2 */
#define MCU_RCC2_DIV400_MASK            (0x40000000UL)   /**< @brief Divide PLL as 400MHz */
#define MCU_RCC2_SYSDIV2_MASK           (0x1F800000UL)   /**< @brief System Clock Divisor 2 */
#define MCU_RCC2_SYSDIV2LSB_MASK        (0x00400000UL)   /**< @brief Additional LSB for SYSDIV2 */
#define MCU_RCC2_USBPWRDN_MASK          (0x00004000UL)   /**< @brief Power USB PLL Down */
#define MCU_RCC2_PWRDN2_MASK            (0x00002000UL)   /**< @brief Power Down PLL */
#define MCU_RCC2_BYPASS2_MASK           (0x00000800UL)   /**< @brief PLL Bypass 2 */
#define MCU_RCC2_OSCSRC2_MASK           (0x00000070UL)   /**< @brief Oscillator Source 2 */

/* ===================[PLL Status Register Bit Definitions]=================== */

/**
 * @brief PLL Status Register Bit Fields
 */
#define MCU_PLLSTAT_LOCK_MASK           (0x00000001UL)   /**< @brief PLL Lock */

/* ===================[Reset Cause Register Bit Definitions]=================== */

/**
 * @brief Reset Cause Register Bit Fields
 */
#define MCU_RESC_EXT_MASK               (0x00000001UL)   /**< @brief External Reset */
#define MCU_RESC_POR_MASK               (0x00000002UL)   /**< @brief Power-On Reset */
#define MCU_RESC_BOR_MASK               (0x00000004UL)   /**< @brief Brown-Out Reset */
#define MCU_RESC_WDT0_MASK              (0x00000008UL)   /**< @brief Watchdog Timer 0 Reset */
#define MCU_RESC_SW_MASK                (0x00000010UL)   /**< @brief Software Reset */
#define MCU_RESC_WDT1_MASK              (0x00000020UL)   /**< @brief Watchdog Timer 1 Reset */
#define MCU_RESC_MOSCFAIL_MASK          (0x00010000UL)   /**< @brief MOSC Failure Reset */


#endif /* MCAL_MCU_MCU_CFG_H_ */
