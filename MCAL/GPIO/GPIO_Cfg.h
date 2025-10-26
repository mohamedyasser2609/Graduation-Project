/*
 * @file Gpio_Cfg.h
 * @brief GPIO Configuration Header for TM4C123GH6PM
 * @details This file contains AUTOSAR-compliant configuration parameters
 *          and compile-time settings for the GPIO driver.
 *
 * @author Mohamed Yasser
 * @date Oct 19, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_GPIO_GPIO_CFG_H_
#define MCAL_GPIO_GPIO_CFG_H_

/* ===================[Includes]=================== */
#include "GPIO_Types.h"

/* ===================[Version Information]=================== */
#define GPIO_CFG_VENDOR_ID                    (0x1234u)
#define GPIO_CFG_MODULE_ID                    (120u)
#define GPIO_CFG_SW_MAJOR_VERSION             (1u)
#define GPIO_CFG_SW_MINOR_VERSION             (0u)
#define GPIO_CFG_SW_PATCH_VERSION             (0u)
#define GPIO_CFG_AR_RELEASE_MAJOR_VERSION     (4u)
#define GPIO_CFG_AR_RELEASE_MINOR_VERSION     (4u)
#define GPIO_CFG_AR_RELEASE_REVISION_VERSION  (0u)

/* ===================[Development Error Detection]=================== */
/**
 * @brief Enable/Disable Development Error Detection
 * @details STD_ON: DET is enabled, STD_OFF: DET is disabled
 */
#define GPIO_DEV_ERROR_DETECT                 STD_ON

/**
 * @brief Enable/Disable Version Info API
 * @details STD_ON: Gpio_GetVersionInfo() is available
 *          STD_OFF: Gpio_GetVersionInfo() is not available
 */
#define GPIO_VERSION_INFO_API                 STD_ON

/**
 * @brief Enable/Disable Flip Channel API
 * @details STD_ON: Gpio_FlipChannel() is available
 *          STD_OFF: Gpio_FlipChannel() is not available
 */
#define GPIO_FLIP_CHANNEL_API                 STD_ON

/**
 * @brief Enable/Disable Port Group APIs
 * @details STD_ON: Port group read/write APIs are available
 *          STD_OFF: Port group APIs are not available
 */
#define GPIO_PORT_GROUP_API                   STD_ON

/* ===================[Module Configuration]=================== */
/**
 * @brief Total number of configured GPIO channels
 * @details This should match the number of pins configured in Gpio_PBCfg.c
 */
#define GPIO_CONFIGURED_CHANNELS              (10u)

/**
 * @brief Maximum number of GPIO channels
 * @details TM4C123GH6PM has 6 ports with 8 pins each = 48 total pins
 *          (Port E has only 6 pins, Port F has only 5 pins = 43 actual pins)
 */
#define GPIO_MAX_CHANNELS                     (48u)

/**
 * @brief Number of GPIO ports
 * @details TM4C123GH6PM has 6 GPIO ports (A-F)
 */
#define GPIO_NUMBER_OF_PORTS                  (6u)

/**
 * @brief Number of pins per port
 * @details Standard GPIO port has 8 pins
 */
#define GPIO_PINS_PER_PORT                    (8u)

/* ===================[API Service IDs]=================== */
/**
 * @brief Service ID for Gpio_Init
 */
#define GPIO_INIT_SID                         (0x00u)

/**
 * @brief Service ID for Gpio_ReadChannel
 */
#define GPIO_READ_CHANNEL_SID                 (0x01u)

/**
 * @brief Service ID for Gpio_WriteChannel
 */
#define GPIO_WRITE_CHANNEL_SID                (0x02u)

/**
 * @brief Service ID for Gpio_ReadPort
 */
#define GPIO_READ_PORT_SID                    (0x03u)

/**
 * @brief Service ID for Gpio_WritePort
 */
#define GPIO_WRITE_PORT_SID                   (0x04u)

/**
 * @brief Service ID for Gpio_FlipChannel
 */
#define GPIO_FLIP_CHANNEL_SID                 (0x05u)

/**
 * @brief Service ID for Gpio_GetVersionInfo
 */
#define GPIO_GET_VERSION_INFO_SID             (0x06u)

/**
 * @brief Service ID for Gpio_SetPinDirection
 */
#define GPIO_SET_PIN_DIRECTION_SID            (0x07u)

/**
 * @brief Service ID for Gpio_RefreshPortDirection
 */
#define GPIO_REFRESH_PORT_DIRECTION_SID       (0x08u)

/**
 * @brief Service ID for Gpio_ReadChannelGroup
 */
#define GPIO_READ_CHANNEL_GROUP_SID           (0x09u)

/**
 * @brief Service ID for Gpio_WriteChannelGroup
 */
#define GPIO_WRITE_CHANNEL_GROUP_SID          (0x0Au)

/**
 * @brief Service ID for Gpio_EnableInterrupt
 */
#define GPIO_ENABLE_INTERRUPT_SID             (0x0Bu)

/**
 * @brief Service ID for Gpio_DisableInterrupt
 */
#define GPIO_DISABLE_INTERRUPT_SID            (0x0Cu)

/**
 * @brief Service ID for Gpio_ClearInterrupt
 */
#define GPIO_CLEAR_INTERRUPT_SID              (0x0Du)

/* ===================[DET Error Codes]=================== */
/**
 * @brief Invalid channel requested
 * @details Channel ID is out of valid range
 */
#define GPIO_E_PARAM_INVALID_CHANNEL          (0x0Au)

/**
 * @brief Invalid port requested
 * @details Port ID is out of valid range
 */
#define GPIO_E_PARAM_INVALID_PORT             (0x0Bu)

/**
 * @brief Invalid channel group
 * @details Channel group definition is invalid
 */
#define GPIO_E_PARAM_INVALID_GROUP            (0x0Cu)

/**
 * @brief API service called with NULL pointer
 * @details A NULL pointer was passed to an API
 */
#define GPIO_E_PARAM_POINTER                  (0x10u)

/**
 * @brief API service used without module initialization
 * @details Gpio_Init() was not called before using other APIs
 */
#define GPIO_E_UNINIT                         (0x0Fu)

/**
 * @brief Invalid pin direction
 * @details Attempted to change direction of a pin that is not changeable
 */
#define GPIO_E_DIRECTION_UNCHANGEABLE         (0x11u)

/**
 * @brief Invalid configuration
 * @details Configuration pointer is NULL or invalid
 */
#define GPIO_E_PARAM_CONFIG                   (0x12u)

/**
 * @brief Module already initialized
 * @details Gpio_Init() called when module is already initialized
 */
#define GPIO_E_ALREADY_INITIALIZED            (0x13u)

/**
 * @brief JTAG pin protection
 * @details Attempted to configure JTAG pins (PC0-PC3) which are protected
 */
#define GPIO_E_JTAG_PIN_PROTECTED             (0x14u)

/* ===================[Hardware Configuration]=================== */
/**
 * @brief GPIO Lock Key Value
 * @details Magic value to unlock GPIO commit register (0x4C4F434B = "LOCK")
 */
#define GPIO_LOCK_KEY                         (0x4C4F434Bu)

/**
 * @brief System Clock Frequency (Hz)
 * @details Default system clock for TM4C123GH6PM (80 MHz)
 */
#define GPIO_SYSTEM_CLOCK_HZ                  (80000000u)

/**
 * @brief GPIO Port Base Addresses
 * @details Base addresses for GPIO ports A-F
 */
#define GPIO_PORTA_BASE_ADDRESS               (0x40004000u)
#define GPIO_PORTB_BASE_ADDRESS               (0x40005000u)
#define GPIO_PORTC_BASE_ADDRESS               (0x40006000u)
#define GPIO_PORTD_BASE_ADDRESS               (0x40007000u)
#define GPIO_PORTE_BASE_ADDRESS               (0x40024000u)
#define GPIO_PORTF_BASE_ADDRESS               (0x40025000u)

/**
 * @brief System Control Base Address
 * @details Base address for system control registers
 */
#define GPIO_SYSCTL_BASE_ADDRESS              (0x400FE000u)

/**
 * @brief GPIO Run Mode Clock Gating Control Register Offset
 */
#define GPIO_SYSCTL_RCGCGPIO_OFFSET           (0x608u)

/* ===================[Port Clock Enable Masks]=================== */
#define GPIO_RCGC_PORTA                       (0x00000001u)  /**< @brief Port A clock enable */
#define GPIO_RCGC_PORTB                       (0x00000002u)  /**< @brief Port B clock enable */
#define GPIO_RCGC_PORTC                       (0x00000004u)  /**< @brief Port C clock enable */
#define GPIO_RCGC_PORTD                       (0x00000008u)  /**< @brief Port D clock enable */
#define GPIO_RCGC_PORTE                       (0x00000010u)  /**< @brief Port E clock enable */
#define GPIO_RCGC_PORTF                       (0x00000020u)  /**< @brief Port F clock enable */

/* ===================[Special Pin Configurations]=================== */
/**
 * @brief JTAG Pins Protection
 * @details PC0-PC3 are JTAG pins and should be handled carefully
 */
#define GPIO_JTAG_PORT                        GPIO_PORT_C
#define GPIO_JTAG_PINS_MASK                   (0x0Fu)  /* PC0-PC3 */

/**
 * @brief NMI Pin Configuration
 * @details PF0 and PD7 can be configured as NMI
 */
#define GPIO_NMI_PIN_PF0                      GPIO_CHANNEL_PF0
#define GPIO_NMI_PIN_PD7                      GPIO_CHANNEL_PD7

/**
 * @brief Locked Pins
 * @details PF0 is locked by default and requires unlock sequence
 */
#define GPIO_LOCKED_PIN_PF0                   GPIO_CHANNEL_PF0

/* ===================[Timing Configuration]=================== */
/**
 * @brief Clock stabilization delay (in cycles)
 * @details Number of NOP cycles to wait after enabling port clock
 */
#define GPIO_CLOCK_STABILIZATION_DELAY        (3u)

/**
 * @brief Unlock sequence timeout (in iterations)
 */
#define GPIO_UNLOCK_TIMEOUT                   (1000u)

/* ===================[Pre-compile Configuration]=================== */
/**
 * @brief Enable/Disable runtime pin direction change
 * @details STD_ON: Pin direction can be changed at runtime
 *          STD_OFF: Pin direction is fixed after initialization
 */
#define GPIO_SET_PIN_DIRECTION_API            STD_ON

/**
 * @brief Enable/Disable port direction refresh
 * @details STD_ON: Gpio_RefreshPortDirection() is available
 *          STD_OFF: Function is not available
 */
#define GPIO_REFRESH_PORT_DIRECTION_API       STD_ON

/**
 * @brief Enable/Disable interrupt support
 * @details STD_ON: GPIO interrupt functions are available
 *          STD_OFF: Interrupt functions are not available
 */
#define GPIO_INTERRUPT_SUPPORT                STD_ON

/**
 * @brief Enable/Disable critical section protection
 * @details STD_ON: Use critical sections for thread safety
 *          STD_OFF: No critical section protection
 */
#define GPIO_CRITICAL_SECTION_PROTECTION      STD_ON

/* ===================[Module Instance ID]=================== */
/**
 * @brief GPIO Module Instance ID
 * @details Used for DET error reporting
 */
#define GPIO_INSTANCE_ID                      (0u)

/* ===================[External Configuration]=================== */
/**
 * @brief External configuration structure declaration
 * @details This structure is defined in Gpio_PBCfg.c
 */
extern const Gpio_ConfigType Gpio_Configuration;

#endif /* MCAL_GPIO_GPIO_CFG_H_ */
