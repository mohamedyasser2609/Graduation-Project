/*
 * @file Gpio.h
 * @brief GPIO Driver API for TM4C123GH6PM
 * @details This file contains AUTOSAR-compliant API function prototypes
 *          for the GPIO driver module.
 *
 * @author Mohamed Yasser
 * @date Oct 19, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_GPIO_GPIO_H_
#define MCAL_GPIO_GPIO_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/std_types.h"
#include "../../CONFIG/Compiler.h"
#include "../../CONFIG/Det.h"
#include "GPIO_Types.h"
#include "Gpio_Cfg.h"

/* ===================[Version Information]=================== */
#define GPIO_VENDOR_ID                    (0x1234u)
#define GPIO_MODULE_ID                    (120u)
#define GPIO_SW_MAJOR_VERSION             (1u)
#define GPIO_SW_MINOR_VERSION             (0u)
#define GPIO_SW_PATCH_VERSION             (0u)
#define GPIO_AR_RELEASE_MAJOR_VERSION     (4u)
#define GPIO_AR_RELEASE_MINOR_VERSION     (4u)
#define GPIO_AR_RELEASE_REVISION_VERSION  (0u)

/* ===================[API Function Prototypes]=================== */

/**
 * @brief Initialize GPIO module
 * @details This function initializes the GPIO module with the provided
 *          configuration. It must be called before any other GPIO function.
 *          Configures all pins according to the configuration structure.
 *
 * @param[in] ConfigPtr - Pointer to configuration structure
 *
 * @return void
 *
 * @pre None
 * @post GPIO module is initialized and ready for use
 *
 * @implements Gpio_Init_Activity
 */
void Gpio_Init(const Gpio_ConfigType* ConfigPtr);

/**
 * @brief Read GPIO channel level
 * @details This function reads the current level of the specified GPIO channel.
 *
 * @param[in] Channel - GPIO channel ID
 *
 * @return Gpio_LevelType
 *         - GPIO_LEVEL_LOW: Channel level is LOW
 *         - GPIO_LEVEL_HIGH: Channel level is HIGH
 *
 * @pre GPIO module must be initialized
 * @post None
 *
 * @implements Gpio_ReadChannel_Activity
 */
Gpio_LevelType Gpio_ReadChannel(Gpio_ChannelType Channel);

/**
 * @brief Write GPIO channel level
 * @details This function writes the specified level to the GPIO channel.
 *
 * @param[in] Channel - GPIO channel ID
 * @param[in] Level - Level to be written (GPIO_LEVEL_LOW or GPIO_LEVEL_HIGH)
 *
 * @return void
 *
 * @pre GPIO module must be initialized
 * @pre Channel must be configured as output
 * @post Channel level is set to the specified value
 *
 * @implements Gpio_WriteChannel_Activity
 */
void Gpio_WriteChannel(Gpio_ChannelType Channel, Gpio_LevelType Level);

/**
 * @brief Read GPIO port level
 * @details This function reads the levels of all pins in the specified port.
 *
 * @param[in] Port - GPIO port ID
 *
 * @return uint8 - Port level (bit field, 8 bits)
 *
 * @pre GPIO module must be initialized
 * @post None
 *
 * @implements Gpio_ReadPort_Activity
 */
uint8 Gpio_ReadPort(Gpio_PortType Port);

/**
 * @brief Write GPIO port level
 * @details This function writes levels to all pins in the specified port.
 *
 * @param[in] Port - GPIO port ID
 * @param[in] Level - Port level to be written (bit field, 8 bits)
 *
 * @return void
 *
 * @pre GPIO module must be initialized
 * @post Port level is set to the specified value
 *
 * @implements Gpio_WritePort_Activity
 */
void Gpio_WritePort(Gpio_PortType Port, uint8 Level);

#if (GPIO_FLIP_CHANNEL_API == STD_ON)
/**
 * @brief Flip (toggle) GPIO channel level
 * @details This function flips the level of the specified GPIO channel.
 *          If the channel is LOW, it becomes HIGH, and vice versa.
 *
 * @param[in] Channel - GPIO channel ID
 *
 * @return Gpio_LevelType - New level after flip
 *
 * @pre GPIO module must be initialized
 * @pre Channel must be configured as output
 * @post Channel level is toggled
 *
 * @implements Gpio_FlipChannel_Activity
 */
Gpio_LevelType Gpio_FlipChannel(Gpio_ChannelType Channel);
#endif

#if (GPIO_VERSION_INFO_API == STD_ON)
/**
 * @brief Get GPIO driver version information
 * @details This function returns the version information of the GPIO driver.
 *
 * @param[out] VersionInfo - Pointer to version information structure
 *
 * @return void
 *
 * @pre None
 * @post Version information is written to the provided structure
 *
 * @implements Gpio_GetVersionInfo_Activity
 */
void Gpio_GetVersionInfo(Std_VersionInfoType* VersionInfo);
#endif

#if (GPIO_SET_PIN_DIRECTION_API == STD_ON)
/**
 * @brief Set GPIO pin direction
 * @details This function sets the direction of the specified GPIO pin at runtime.
 *          Only pins configured as direction-changeable can use this function.
 *
 * @param[in] Channel - GPIO channel ID
 * @param[in] Direction - Pin direction (GPIO_PIN_IN or GPIO_PIN_OUT)
 *
 * @return void
 *
 * @pre GPIO module must be initialized
 * @pre Pin must be configured as direction-changeable
 * @post Pin direction is set to the specified value
 *
 * @implements Gpio_SetPinDirection_Activity
 */
void Gpio_SetPinDirection(Gpio_ChannelType Channel, Gpio_PinDirectionType Direction);
#endif

#if (GPIO_REFRESH_PORT_DIRECTION_API == STD_ON)
/**
 * @brief Refresh port direction
 * @details This function refreshes the direction of all pins in the specified port
 *          to their configured values. This excludes pins configured as
 *          direction-changeable.
 *
 * @param[in] Port - GPIO port ID
 *
 * @return void
 *
 * @pre GPIO module must be initialized
 * @post Port direction is refreshed to configured values
 *
 * @implements Gpio_RefreshPortDirection_Activity
 */
void Gpio_RefreshPortDirection(Gpio_PortType Port);
#endif

#if (GPIO_PORT_GROUP_API == STD_ON)
/**
 * @brief Read GPIO channel group
 * @details This function reads the levels of a group of pins within a port.
 *
 * @param[in] ChannelGroupPtr - Pointer to channel group definition
 *
 * @return uint8 - Channel group level (masked and shifted)
 *
 * @pre GPIO module must be initialized
 * @post None
 *
 * @implements Gpio_ReadChannelGroup_Activity
 */
uint8 Gpio_ReadChannelGroup(const Gpio_PortGroupType* ChannelGroupPtr);

/**
 * @brief Write GPIO channel group
 * @details This function writes levels to a group of pins within a port.
 *
 * @param[in] ChannelGroupPtr - Pointer to channel group definition
 * @param[in] Level - Level to be written to the group
 *
 * @return void
 *
 * @pre GPIO module must be initialized
 * @post Channel group level is set to the specified value
 *
 * @implements Gpio_WriteChannelGroup_Activity
 */
void Gpio_WriteChannelGroup(const Gpio_PortGroupType* ChannelGroupPtr, uint8 Level);
#endif

#if (GPIO_INTERRUPT_SUPPORT == STD_ON)
/**
 * @brief Enable GPIO interrupt
 * @details This function enables interrupt for the specified GPIO channel.
 *
 * @param[in] Channel - GPIO channel ID
 *
 * @return void
 *
 * @pre GPIO module must be initialized
 * @pre Channel must be configured with interrupt trigger
 * @post Interrupt is enabled for the channel
 *
 * @implements Gpio_EnableInterrupt_Activity
 */
void Gpio_EnableInterrupt(Gpio_ChannelType Channel);

/**
 * @brief Disable GPIO interrupt
 * @details This function disables interrupt for the specified GPIO channel.
 *
 * @param[in] Channel - GPIO channel ID
 *
 * @return void
 *
 * @pre GPIO module must be initialized
 * @post Interrupt is disabled for the channel
 *
 * @implements Gpio_DisableInterrupt_Activity
 */
void Gpio_DisableInterrupt(Gpio_ChannelType Channel);

/**
 * @brief Clear GPIO interrupt flag
 * @details This function clears the interrupt flag for the specified GPIO channel.
 *
 * @param[in] Channel - GPIO channel ID
 *
 * @return void
 *
 * @pre GPIO module must be initialized
 * @post Interrupt flag is cleared for the channel
 *
 * @implements Gpio_ClearInterrupt_Activity
 */
void Gpio_ClearInterrupt(Gpio_ChannelType Channel);

/**
 * @brief Get GPIO interrupt status
 * @details This function returns the interrupt status for the specified GPIO channel.
 *
 * @param[in] Channel - GPIO channel ID
 *
 * @return boolean
 *         - TRUE: Interrupt is pending
 *         - FALSE: No interrupt pending
 *
 * @pre GPIO module must be initialized
 * @post None
 *
 * @implements Gpio_GetInterruptStatus_Activity
 */
boolean Gpio_GetInterruptStatus(Gpio_ChannelType Channel);
#endif

/* ===================[Internal Helper Macros]=================== */

/**
 * @brief Extract port from channel
 */
#define GPIO_GET_PORT(channel)    ((Gpio_PortType)((channel) / GPIO_PINS_PER_PORT))

/**
 * @brief Extract pin from channel
 */
#define GPIO_GET_PIN(channel)     ((Gpio_PinType)((channel) % GPIO_PINS_PER_PORT))

/**
 * @brief Create pin mask
 */
#define GPIO_PIN_MASK(pin)        ((uint8)(1u << (pin)))

/**
 * @brief Check if channel is valid
 */
#define GPIO_IS_VALID_CHANNEL(channel)  ((channel) < GPIO_MAX_CHANNELS)

/**
 * @brief Check if port is valid
 */
#define GPIO_IS_VALID_PORT(port)        ((port) < GPIO_NUMBER_OF_PORTS)

#endif /* MCAL_GPIO_GPIO_H_ */
