/**
 * @file NVIC_Types.h
 * @brief NVIC Driver Types for TM4C123GH6PM
 * @details Type definitions used by the AUTOSAR-compliant NVIC driver.
 */

#ifndef MCAL_NVIC_NVIC_TYPES_H_
#define MCAL_NVIC_NVIC_TYPES_H_

#include "../../CONFIG/Std_Types.h"

/**
 * @brief Maximum number of NVIC interrupts supported
 */
#define NVIC_NUM_INTERRUPTS              (138u)

/**
 * @brief NVIC priority grouping options (PRIGROUP field)
 */
typedef enum {
    NVIC_PRIGROUP_XXX = 0u,   /**< All pre-emption priorities, no subpriority */
    NVIC_PRIGROUP_XXY = 1u,
    NVIC_PRIGROUP_XYY = 2u,
    NVIC_PRIGROUP_YYY = 3u
} NVIC_PriorityGroupType;

/**
 * @brief Interrupt priority levels (0 highest, 7 lowest)
 */
typedef uint8 NVIC_PriorityType;

/**
 * @brief NVIC interrupt vector identifier
 */
typedef uint8 NVIC_InterruptVectorType;

/**
 * @brief NVIC interrupt configuration
 */
typedef struct {
    NVIC_InterruptVectorType Vector; /**< Interrupt vector number */
    NVIC_PriorityType Priority;      /**< Interrupt priority (0-7) */
    boolean Enable;                  /**< TRUE to enable interrupt */
} NVIC_InterruptConfigType;

/**
 * @brief NVIC driver configuration
 */
typedef struct {
    NVIC_PriorityGroupType PriorityGrouping;   /**< Priority grouping configuration */
    const NVIC_InterruptConfigType* Interrupts;/**< Array of interrupt configs */
    uint16 InterruptCount;                     /**< Number of entries in array */
} NVIC_ConfigType;

#endif /* MCAL_NVIC_NVIC_TYPES_H_ */
