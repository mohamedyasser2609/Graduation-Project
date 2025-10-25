/*
 * @file Gpio_Regs.h
 * @brief GPIO Hardware Register Definitions for TM4C123GH6PM
 * @details This file contains hardware register definitions and structures
 *          for direct GPIO hardware access on TM4C123GH6PM.
 *
 * @author Mohamed Yasser
 * @date Oct 19, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_GPIO_GPIO_REGS_H_
#define MCAL_GPIO_GPIO_REGS_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/std_types.h"

/* ===================[GPIO Register Structure]=================== */
/**
 * @brief GPIO Port Register Structure
 * @details Structure representing all registers of a GPIO port
 */
typedef struct {
    volatile uint32 DATA_BITS[255];  /**< @brief Data register with bit masking (0x000-0x3FC) */
    volatile uint32 DATA;            /**< @brief Data register (0x3FC) */
    volatile uint32 DIR;             /**< @brief Direction register (0x400) */
    volatile uint32 IS;              /**< @brief Interrupt sense register (0x404) */
    volatile uint32 IBE;             /**< @brief Interrupt both edges register (0x408) */
    volatile uint32 IEV;             /**< @brief Interrupt event register (0x40C) */
    volatile uint32 IM;              /**< @brief Interrupt mask register (0x410) */
    volatile uint32 RIS;             /**< @brief Raw interrupt status register (0x414) */
    volatile uint32 MIS;             /**< @brief Masked interrupt status register (0x418) */
    volatile uint32 ICR;             /**< @brief Interrupt clear register (0x41C) */
    volatile uint32 AFSEL;           /**< @brief Alternate function select register (0x420) */
    volatile uint32 RESERVED1[55];   /**< @brief Reserved (0x424-0x4FC) */
    volatile uint32 DR2R;            /**< @brief 2mA drive select register (0x500) */
    volatile uint32 DR4R;            /**< @brief 4mA drive select register (0x504) */
    volatile uint32 DR8R;            /**< @brief 8mA drive select register (0x508) */
    volatile uint32 ODR;             /**< @brief Open drain select register (0x50C) */
    volatile uint32 PUR;             /**< @brief Pull-up select register (0x510) */
    volatile uint32 PDR;             /**< @brief Pull-down select register (0x514) */
    volatile uint32 SLR;             /**< @brief Slew rate control register (0x518) */
    volatile uint32 DEN;             /**< @brief Digital enable register (0x51C) */
    volatile uint32 LOCK;            /**< @brief Lock register (0x520) */
    volatile uint32 CR;              /**< @brief Commit register (0x524) */
    volatile uint32 AMSEL;           /**< @brief Analog mode select register (0x528) */
    volatile uint32 PCTL;            /**< @brief Port control register (0x52C) */
    volatile uint32 ADCCTL;          /**< @brief ADC control register (0x530) */
    volatile uint32 DMACTL;          /**< @brief DMA control register (0x534) */
} Gpio_RegisterType;

/* ===================[System Control Registers]=================== */
/**
 * @brief System Control Register Structure
 * @details Structure for system control registers relevant to GPIO
 */
typedef struct {
    volatile uint32 RCGCGPIO;        /**< @brief GPIO Run Mode Clock Gating Control */
} Gpio_SysCtlType;

/* ===================[GPIO Port Base Addresses]=================== */
#define GPIO_PORTA_REG    ((Gpio_RegisterType*)0x40004000u)
#define GPIO_PORTB_REG    ((Gpio_RegisterType*)0x40005000u)
#define GPIO_PORTC_REG    ((Gpio_RegisterType*)0x40006000u)
#define GPIO_PORTD_REG    ((Gpio_RegisterType*)0x40007000u)
#define GPIO_PORTE_REG    ((Gpio_RegisterType*)0x40024000u)
#define GPIO_PORTF_REG    ((Gpio_RegisterType*)0x40025000u)

/* ===================[System Control Base Address]=================== */
#define GPIO_SYSCTL_RCGCGPIO_REG  (*((volatile uint32*)0x400FE608u))

/* ===================[GPIO Register Bit Definitions]=================== */

/* GPIO Lock Register Values */
#define GPIO_LOCK_LOCKED          (0x00000001u)  /**< @brief GPIO is locked */
#define GPIO_LOCK_UNLOCKED        (0x00000000u)  /**< @brief GPIO is unlocked */
#define GPIO_LOCK_KEY             (0x4C4F434Bu)  /**< @brief Unlock key value */

/* GPIO Commit Register Bits */
#define GPIO_CR_PIN0              (0x00000001u)  /**< @brief Pin 0 commit */
#define GPIO_CR_PIN1              (0x00000002u)  /**< @brief Pin 1 commit */
#define GPIO_CR_PIN2              (0x00000004u)  /**< @brief Pin 2 commit */
#define GPIO_CR_PIN3              (0x00000008u)  /**< @brief Pin 3 commit */
#define GPIO_CR_PIN4              (0x00000010u)  /**< @brief Pin 4 commit */
#define GPIO_CR_PIN5              (0x00000020u)  /**< @brief Pin 5 commit */
#define GPIO_CR_PIN6              (0x00000040u)  /**< @brief Pin 6 commit */
#define GPIO_CR_PIN7              (0x00000080u)  /**< @brief Pin 7 commit */

/* GPIO Direction Register Bits */
#define GPIO_DIR_INPUT            (0u)           /**< @brief Pin is input */
#define GPIO_DIR_OUTPUT           (1u)           /**< @brief Pin is output */

/* GPIO Digital Enable Register Bits */
#define GPIO_DEN_DISABLE          (0u)           /**< @brief Digital function disabled */
#define GPIO_DEN_ENABLE           (1u)           /**< @brief Digital function enabled */

/* GPIO Alternate Function Select Register Bits */
#define GPIO_AFSEL_GPIO           (0u)           /**< @brief Pin is GPIO */
#define GPIO_AFSEL_ALT            (1u)           /**< @brief Pin is alternate function */

/* GPIO Analog Mode Select Register Bits */
#define GPIO_AMSEL_DIGITAL        (0u)           /**< @brief Pin is digital */
#define GPIO_AMSEL_ANALOG         (1u)           /**< @brief Pin is analog */

/* GPIO Interrupt Sense Register Bits */
#define GPIO_IS_EDGE              (0u)           /**< @brief Edge-sensitive */
#define GPIO_IS_LEVEL             (1u)           /**< @brief Level-sensitive */

/* GPIO Interrupt Both Edges Register Bits */
#define GPIO_IBE_SINGLE           (0u)           /**< @brief Single edge */
#define GPIO_IBE_BOTH             (1u)           /**< @brief Both edges */

/* GPIO Interrupt Event Register Bits */
#define GPIO_IEV_FALLING_LOW      (0u)           /**< @brief Falling edge or low level */
#define GPIO_IEV_RISING_HIGH      (1u)           /**< @brief Rising edge or high level */

/* ===================[Port Control Register Masks]=================== */
#define GPIO_PCTL_PIN0_MASK       (0x0000000Fu)  /**< @brief Pin 0 control mask */
#define GPIO_PCTL_PIN1_MASK       (0x000000F0u)  /**< @brief Pin 1 control mask */
#define GPIO_PCTL_PIN2_MASK       (0x00000F00u)  /**< @brief Pin 2 control mask */
#define GPIO_PCTL_PIN3_MASK       (0x0000F000u)  /**< @brief Pin 3 control mask */
#define GPIO_PCTL_PIN4_MASK       (0x000F0000u)  /**< @brief Pin 4 control mask */
#define GPIO_PCTL_PIN5_MASK       (0x00F00000u)  /**< @brief Pin 5 control mask */
#define GPIO_PCTL_PIN6_MASK       (0x0F000000u)  /**< @brief Pin 6 control mask */
#define GPIO_PCTL_PIN7_MASK       (0xF0000000u)  /**< @brief Pin 7 control mask */

/* ===================[Helper Macros]=================== */

/**
 * @brief Get GPIO port register base address
 * @param[in] port - Port number (0-5)
 * @return Pointer to GPIO port registers
 */
#define GPIO_GET_PORT_REG(port) \
    ((port) == GPIO_PORT_A ? GPIO_PORTA_REG : \
     (port) == GPIO_PORT_B ? GPIO_PORTB_REG : \
     (port) == GPIO_PORT_C ? GPIO_PORTC_REG : \
     (port) == GPIO_PORT_D ? GPIO_PORTD_REG : \
     (port) == GPIO_PORT_E ? GPIO_PORTE_REG : \
     GPIO_PORTF_REG)

/**
 * @brief Get PCTL mask for a specific pin
 * @param[in] pin - Pin number (0-7)
 * @return PCTL mask for the pin
 */
#define GPIO_GET_PCTL_MASK(pin) \
    ((uint32)(0x0000000Fu << ((pin) * 4u)))

/**
 * @brief Get PCTL shift value for a specific pin
 * @param[in] pin - Pin number (0-7)
 * @return PCTL shift value
 */
#define GPIO_GET_PCTL_SHIFT(pin) \
    ((uint8)((pin) * 4u))

/**
 * @brief Set bit in register
 * @param[in] reg - Register address
 * @param[in] bit - Bit position
 */
#define GPIO_SET_BIT(reg, bit)    ((reg) |= (1u << (bit)))

/**
 * @brief Clear bit in register
 * @param[in] reg - Register address
 * @param[in] bit - Bit position
 */
#define GPIO_CLEAR_BIT(reg, bit)  ((reg) &= ~(1u << (bit)))

/**
 * @brief Toggle bit in register
 * @param[in] reg - Register address
 * @param[in] bit - Bit position
 */
#define GPIO_TOGGLE_BIT(reg, bit) ((reg) ^= (1u << (bit)))

/**
 * @brief Read bit from register
 * @param[in] reg - Register address
 * @param[in] bit - Bit position
 * @return Bit value (0 or 1)
 */
#define GPIO_READ_BIT(reg, bit)   (((reg) >> (bit)) & 1u)

/**
 * @brief Write bit to register
 * @param[in] reg - Register address
 * @param[in] bit - Bit position
 * @param[in] val - Value to write (0 or 1)
 */
#define GPIO_WRITE_BIT(reg, bit, val) \
    do { \
        if (val) { \
            GPIO_SET_BIT(reg, bit); \
        } else { \
            GPIO_CLEAR_BIT(reg, bit); \
        } \
    } while(0)

#endif /* MCAL_GPIO_GPIO_REGS_H_ */
