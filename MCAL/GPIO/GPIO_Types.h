/*
 * @file GPIO_Types.h
 * @brief GPIO Type Definitions for TM4C123GH6PM
 * @details This file contains AUTOSAR-compliant type definitions for GPIO driver
 *          including port/pin definitions, direction, level, and mode types.
 *
 * @author Mohamed Yasser
 * @date Oct 19, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_GPIO_GPIO_TYPES_H_
#define MCAL_GPIO_GPIO_TYPES_H_

/* ===================[Includes]=================== */
#include <Std_types.h>
#include "../../CONFIG/Platform_Types.h"

/* ===================[Version Information]=================== */
#define GPIO_TYPES_VENDOR_ID                    (0x1234u)
#define GPIO_TYPES_MODULE_ID                    (120u)  /* AUTOSAR GPIO Module ID */
#define GPIO_TYPES_SW_MAJOR_VERSION             (1u)
#define GPIO_TYPES_SW_MINOR_VERSION             (0u)
#define GPIO_TYPES_SW_PATCH_VERSION             (0u)
#define GPIO_TYPES_AR_RELEASE_MAJOR_VERSION     (4u)
#define GPIO_TYPES_AR_RELEASE_MINOR_VERSION     (4u)
#define GPIO_TYPES_AR_RELEASE_REVISION_VERSION  (0u)

/* ===================[GPIO Port Definitions]=================== */
/**
 * @brief GPIO Port Type
 * @details Enumeration for GPIO ports available on TM4C123GH6PM
 *          TM4C123GH6PM has 6 GPIO ports: A, B, C, D, E, F
 */
typedef enum {
    GPIO_PORT_A = 0u,  /**< @brief GPIO Port A */
    GPIO_PORT_B = 1u,  /**< @brief GPIO Port B */
    GPIO_PORT_C = 2u,  /**< @brief GPIO Port C */
    GPIO_PORT_D = 3u,  /**< @brief GPIO Port D */
    GPIO_PORT_E = 4u,  /**< @brief GPIO Port E */
    GPIO_PORT_F = 5u,  /**< @brief GPIO Port F */
    GPIO_PORT_MAX = 6u /**< @brief Maximum number of GPIO ports */
} Gpio_PortType;

/* ===================[GPIO Pin Definitions]=================== */
/**
 * @brief GPIO Pin Type
 * @details Enumeration for GPIO pins (0-7) within a port
 */
typedef enum {
    GPIO_PIN_0 = 0u,  /**< @brief GPIO Pin 0 */
    GPIO_PIN_1 = 1u,  /**< @brief GPIO Pin 1 */
    GPIO_PIN_2 = 2u,  /**< @brief GPIO Pin 2 */
    GPIO_PIN_3 = 3u,  /**< @brief GPIO Pin 3 */
    GPIO_PIN_4 = 4u,  /**< @brief GPIO Pin 4 */
    GPIO_PIN_5 = 5u,  /**< @brief GPIO Pin 5 */
    GPIO_PIN_6 = 6u,  /**< @brief GPIO Pin 6 */
    GPIO_PIN_7 = 7u,  /**< @brief GPIO Pin 7 */
    GPIO_PIN_MAX = 8u /**< @brief Maximum number of pins per port */
} Gpio_PinType;

/* ===================[GPIO Channel Type]=================== */
/**
 * @brief GPIO Channel Type
 * @details Type for GPIO channel (combination of port and pin)
 *          Channel = (Port * 8) + Pin
 */
typedef uint8 Gpio_ChannelType;

/* Channel calculation macro */
#define GPIO_CHANNEL(port, pin)  ((Gpio_ChannelType)(((port) * 8u) + (pin)))

/* ===================[GPIO Pin Direction]=================== */
/**
 * @brief GPIO Pin Direction Type
 * @details Enumeration for GPIO pin direction (input/output)
 */
typedef enum {
    GPIO_PIN_IN  = 0u,  /**< @brief Pin configured as input */
    GPIO_PIN_OUT = 1u   /**< @brief Pin configured as output */
} Gpio_PinDirectionType;

/* ===================[GPIO Pin Level]=================== */
/**
 * @brief GPIO Pin Level Type
 * @details Enumeration for GPIO pin level (low/high)
 */
typedef enum {
    GPIO_LEVEL_LOW  = 0u,  /**< @brief Pin level is LOW (0V) */
    GPIO_LEVEL_HIGH = 1u   /**< @brief Pin level is HIGH (3.3V) */
} Gpio_LevelType;

/* ===================[GPIO Pin Mode]=================== */
/**
 * @brief GPIO Pin Mode Type
 * @details Enumeration for GPIO pin modes
 */
typedef enum {
    GPIO_MODE_DIO = 0u,        /**< @brief Digital I/O mode */
    GPIO_MODE_ALT_FUNC = 1u,   /**< @brief Alternate function mode */
    GPIO_MODE_ANALOG = 2u      /**< @brief Analog mode */
} Gpio_PinModeType;

/* ===================[GPIO Internal Resistor]=================== */
/**
 * @brief GPIO Internal Resistor Type
 * @details Enumeration for internal pull-up/pull-down resistors
 */
typedef enum {
    GPIO_RESISTOR_OFF = 0u,       /**< @brief No internal resistor */
    GPIO_RESISTOR_PULL_UP = 1u,   /**< @brief Internal pull-up resistor */
    GPIO_RESISTOR_PULL_DOWN = 2u, /**< @brief Internal pull-down resistor */
    GPIO_RESISTOR_OPEN_DRAIN = 3u /**< @brief Open drain configuration */
} Gpio_InternalResistorType;

/* ===================[GPIO Drive Strength]=================== */
/**
 * @brief GPIO Drive Strength Type
 * @details Enumeration for GPIO output drive strength
 */
typedef enum {
    GPIO_DRIVE_2MA = 0u,   /**< @brief 2mA drive strength */
    GPIO_DRIVE_4MA = 1u,   /**< @brief 4mA drive strength */
    GPIO_DRIVE_8MA = 2u    /**< @brief 8mA drive strength */
} Gpio_DriveStrengthType;

/* ===================[GPIO Slew Rate]=================== */
/**
 * @brief GPIO Slew Rate Type
 * @details Enumeration for GPIO slew rate control
 */
typedef enum {
    GPIO_SLEW_RATE_NORMAL = 0u,  /**< @brief Normal slew rate */
    GPIO_SLEW_RATE_SLOW = 1u     /**< @brief Slow slew rate */
} Gpio_SlewRateType;

/* ===================[GPIO Interrupt Configuration]=================== */
/**
 * @brief GPIO Interrupt Trigger Type
 * @details Enumeration for GPIO interrupt trigger modes
 */
typedef enum {
    GPIO_INT_DISABLED = 0u,       /**< @brief Interrupt disabled */
    GPIO_INT_RISING_EDGE = 1u,    /**< @brief Rising edge trigger */
    GPIO_INT_FALLING_EDGE = 2u,   /**< @brief Falling edge trigger */
    GPIO_INT_BOTH_EDGES = 3u,     /**< @brief Both edges trigger */
    GPIO_INT_LOW_LEVEL = 4u,      /**< @brief Low level trigger */
    GPIO_INT_HIGH_LEVEL = 5u      /**< @brief High level trigger */
} Gpio_IntTriggerType;

/* ===================[GPIO Pin Configuration Structure]=================== */
/**
 * @brief GPIO Pin Configuration Type
 * @details Structure containing all configuration parameters for a GPIO pin
 */
typedef struct {
    Gpio_PortType               Port;              /**< @brief GPIO port */
    Gpio_PinType                Pin;               /**< @brief GPIO pin number */
    Gpio_PinDirectionType       Direction;         /**< @brief Pin direction */
    Gpio_LevelType              InitialLevel;      /**< @brief Initial output level */
    Gpio_PinModeType            Mode;              /**< @brief Pin mode */
    Gpio_InternalResistorType   InternalResistor;  /**< @brief Internal resistor */
    Gpio_DriveStrengthType      DriveStrength;     /**< @brief Drive strength */
    Gpio_SlewRateType           SlewRate;          /**< @brief Slew rate control */
    Gpio_IntTriggerType         IntTrigger;        /**< @brief Interrupt trigger */
    uint8                       AlternateFuncNum;  /**< @brief Alternate function number (0-15) */
    boolean                     DirectionChangeable; /**< @brief Direction changeable at runtime */
} Gpio_PinConfigType;

/* ===================[GPIO Configuration Type]=================== */
/**
 * @brief GPIO Configuration Type
 * @details Main configuration structure for GPIO module
 */
typedef struct {
    const Gpio_PinConfigType*   PinConfigs;        /**< @brief Pointer to pin configurations */
    uint16                      NumberOfPins;      /**< @brief Number of configured pins */
} Gpio_ConfigType;

/* ===================[GPIO Port Group Type]=================== */
/**
 * @brief GPIO Port Group Type
 * @details Type for defining a group of pins within a port
 */
typedef struct {
    Gpio_PortType   Port;      /**< @brief GPIO port */
    uint8           Mask;      /**< @brief Pin mask (bit field) */
    uint8           Offset;    /**< @brief Bit offset for the group */
} Gpio_PortGroupType;

/* ===================[GPIO Module State]=================== */
/**
 * @brief GPIO Module State Type
 * @details Enumeration for GPIO module initialization state
 */
typedef enum {
    GPIO_UNINIT = 0u,      /**< @brief GPIO module not initialized */
    GPIO_INIT = 1u         /**< @brief GPIO module initialized */
} Gpio_StateType;

/* ===================[GPIO Lock Status]=================== */
/**
 * @brief GPIO Lock Status Type
 * @details Enumeration for GPIO pin lock status
 */
typedef enum {
    GPIO_UNLOCKED = 0u,    /**< @brief Pin is unlocked */
    GPIO_LOCKED = 1u       /**< @brief Pin is locked */
} Gpio_LockStatusType;

/* ===================[Common Pin Definitions]=================== */
/* Predefined channels for common pins */
#define GPIO_CHANNEL_PA0    GPIO_CHANNEL(GPIO_PORT_A, GPIO_PIN_0)
#define GPIO_CHANNEL_PA1    GPIO_CHANNEL(GPIO_PORT_A, GPIO_PIN_1)
#define GPIO_CHANNEL_PA2    GPIO_CHANNEL(GPIO_PORT_A, GPIO_PIN_2)
#define GPIO_CHANNEL_PA3    GPIO_CHANNEL(GPIO_PORT_A, GPIO_PIN_3)
#define GPIO_CHANNEL_PA4    GPIO_CHANNEL(GPIO_PORT_A, GPIO_PIN_4)
#define GPIO_CHANNEL_PA5    GPIO_CHANNEL(GPIO_PORT_A, GPIO_PIN_5)
#define GPIO_CHANNEL_PA6    GPIO_CHANNEL(GPIO_PORT_A, GPIO_PIN_6)
#define GPIO_CHANNEL_PA7    GPIO_CHANNEL(GPIO_PORT_A, GPIO_PIN_7)

#define GPIO_CHANNEL_PB0    GPIO_CHANNEL(GPIO_PORT_B, GPIO_PIN_0)
#define GPIO_CHANNEL_PB1    GPIO_CHANNEL(GPIO_PORT_B, GPIO_PIN_1)
#define GPIO_CHANNEL_PB2    GPIO_CHANNEL(GPIO_PORT_B, GPIO_PIN_2)
#define GPIO_CHANNEL_PB3    GPIO_CHANNEL(GPIO_PORT_B, GPIO_PIN_3)
#define GPIO_CHANNEL_PB4    GPIO_CHANNEL(GPIO_PORT_B, GPIO_PIN_4)
#define GPIO_CHANNEL_PB5    GPIO_CHANNEL(GPIO_PORT_B, GPIO_PIN_5)
#define GPIO_CHANNEL_PB6    GPIO_CHANNEL(GPIO_PORT_B, GPIO_PIN_6)
#define GPIO_CHANNEL_PB7    GPIO_CHANNEL(GPIO_PORT_B, GPIO_PIN_7)

#define GPIO_CHANNEL_PC0    GPIO_CHANNEL(GPIO_PORT_C, GPIO_PIN_0)
#define GPIO_CHANNEL_PC1    GPIO_CHANNEL(GPIO_PORT_C, GPIO_PIN_1)
#define GPIO_CHANNEL_PC2    GPIO_CHANNEL(GPIO_PORT_C, GPIO_PIN_2)
#define GPIO_CHANNEL_PC3    GPIO_CHANNEL(GPIO_PORT_C, GPIO_PIN_3)
#define GPIO_CHANNEL_PC4    GPIO_CHANNEL(GPIO_PORT_C, GPIO_PIN_4)
#define GPIO_CHANNEL_PC5    GPIO_CHANNEL(GPIO_PORT_C, GPIO_PIN_5)
#define GPIO_CHANNEL_PC6    GPIO_CHANNEL(GPIO_PORT_C, GPIO_PIN_6)
#define GPIO_CHANNEL_PC7    GPIO_CHANNEL(GPIO_PORT_C, GPIO_PIN_7)

#define GPIO_CHANNEL_PD0    GPIO_CHANNEL(GPIO_PORT_D, GPIO_PIN_0)
#define GPIO_CHANNEL_PD1    GPIO_CHANNEL(GPIO_PORT_D, GPIO_PIN_1)
#define GPIO_CHANNEL_PD2    GPIO_CHANNEL(GPIO_PORT_D, GPIO_PIN_2)
#define GPIO_CHANNEL_PD3    GPIO_CHANNEL(GPIO_PORT_D, GPIO_PIN_3)
#define GPIO_CHANNEL_PD4    GPIO_CHANNEL(GPIO_PORT_D, GPIO_PIN_4)
#define GPIO_CHANNEL_PD5    GPIO_CHANNEL(GPIO_PORT_D, GPIO_PIN_5)
#define GPIO_CHANNEL_PD6    GPIO_CHANNEL(GPIO_PORT_D, GPIO_PIN_6)
#define GPIO_CHANNEL_PD7    GPIO_CHANNEL(GPIO_PORT_D, GPIO_PIN_7)

#define GPIO_CHANNEL_PE0    GPIO_CHANNEL(GPIO_PORT_E, GPIO_PIN_0)
#define GPIO_CHANNEL_PE1    GPIO_CHANNEL(GPIO_PORT_E, GPIO_PIN_1)
#define GPIO_CHANNEL_PE2    GPIO_CHANNEL(GPIO_PORT_E, GPIO_PIN_2)
#define GPIO_CHANNEL_PE3    GPIO_CHANNEL(GPIO_PORT_E, GPIO_PIN_3)
#define GPIO_CHANNEL_PE4    GPIO_CHANNEL(GPIO_PORT_E, GPIO_PIN_4)
#define GPIO_CHANNEL_PE5    GPIO_CHANNEL(GPIO_PORT_E, GPIO_PIN_5)

#define GPIO_CHANNEL_PF0    GPIO_CHANNEL(GPIO_PORT_F, GPIO_PIN_0)
#define GPIO_CHANNEL_PF1    GPIO_CHANNEL(GPIO_PORT_F, GPIO_PIN_1)
#define GPIO_CHANNEL_PF2    GPIO_CHANNEL(GPIO_PORT_F, GPIO_PIN_2)
#define GPIO_CHANNEL_PF3    GPIO_CHANNEL(GPIO_PORT_F, GPIO_PIN_3)
#define GPIO_CHANNEL_PF4    GPIO_CHANNEL(GPIO_PORT_F, GPIO_PIN_4)

#endif /* MCAL_GPIO_GPIO_TYPES_H_ */
