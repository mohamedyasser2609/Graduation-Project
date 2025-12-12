/**
 * @file NVIC.h
 * @brief Nested Vectored Interrupt Controller (NVIC) Driver API
 * @details AUTOSAR-compliant API for configuring and controlling interrupts
 *          on the TM4C123GH6PM microcontroller.
 */

#ifndef MCAL_NVIC_NVIC_H_
#define MCAL_NVIC_NVIC_H_

/* ===================[Includes]=================== */
#include <Std_types.h>
#include "../../CONFIG/Compiler.h"
#include "../../CONFIG/Det.h"
#include "NVIC_Types.h"
#include "NVIC_Cfg.h"

/* ===================[Version Information]=================== */
#define NVIC_VENDOR_ID                    (0x1234u)
#define NVIC_MODULE_ID                    (130u)
#define NVIC_INSTANCE_ID                  (0u)

#define NVIC_SW_MAJOR_VERSION             (1u)
#define NVIC_SW_MINOR_VERSION             (0u)
#define NVIC_SW_PATCH_VERSION             (0u)

#define NVIC_AR_RELEASE_MAJOR_VERSION     (4u)
#define NVIC_AR_RELEASE_MINOR_VERSION     (4u)
#define NVIC_AR_RELEASE_REVISION_VERSION  (0u)

/* ===================[Service IDs]=================== */
#define NVIC_INIT_SID                     (0x00u)
#define NVIC_ENABLE_IRQ_SID               (0x01u)
#define NVIC_DISABLE_IRQ_SID              (0x02u)
#define NVIC_SET_PRIORITY_SID             (0x03u)
#define NVIC_GET_PRIORITY_SID             (0x04u)
#define NVIC_SET_PENDING_IRQ_SID          (0x05u)
#define NVIC_CLEAR_PENDING_IRQ_SID        (0x06u)
#define NVIC_IS_ACTIVE_IRQ_SID            (0x07u)
#define NVIC_GENERATE_SW_IRQ_SID          (0x08u)
#define NVIC_GET_VERSION_INFO_SID         (0x09u)

/* ===================[Error Codes]=================== */
#define NVIC_E_PARAM_POINTER              (0x01u)
#define NVIC_E_PARAM_VECTOR               (0x02u)
#define NVIC_E_UNINIT                     (0x03u)
#define NVIC_E_ALREADY_INITIALIZED        (0x04u)
#define NVIC_E_PARAM_PRIORITY             (0x05u)

/* ===================[API Function Prototypes]=================== */

/**
 * @brief Initialize NVIC driver with provided configuration.
 * @param ConfigPtr Pointer to configuration structure
 */
void NVIC_Init(const NVIC_ConfigType* ConfigPtr);

/**
 * @brief Enable interrupt vector in NVIC.
 * @param Vector Interrupt vector identifier
 */
void NVIC_EnableIRQ(NVIC_InterruptVectorType Vector);

/**
 * @brief Disable interrupt vector in NVIC.
 * @param Vector Interrupt vector identifier
 */
void NVIC_DisableIRQ(NVIC_InterruptVectorType Vector);

/**
 * @brief Set interrupt priority.
 * @param Vector Interrupt vector identifier
 * @param Priority Priority level (0-7, 0 highest)
 */
void NVIC_SetPriority(NVIC_InterruptVectorType Vector, NVIC_PriorityType Priority);

/**
 * @brief Get interrupt priority.
 * @param Vector Interrupt vector identifier
 * @return Priority level (0-7)
 */
NVIC_PriorityType NVIC_GetPriority(NVIC_InterruptVectorType Vector);

/**
 * @brief Set pending flag for interrupt.
 * @param Vector Interrupt vector identifier
 */
void NVIC_SetPendingIRQ(NVIC_InterruptVectorType Vector);

/**
 * @brief Clear pending flag for interrupt.
 * @param Vector Interrupt vector identifier
 */
void NVIC_ClearPendingIRQ(NVIC_InterruptVectorType Vector);

/**
 * @brief Check if interrupt is active.
 * @param Vector Interrupt vector identifier
 * @return TRUE if active, FALSE otherwise
 */
boolean NVIC_IsActiveIRQ(NVIC_InterruptVectorType Vector);

/**
 * @brief Trigger software interrupt.
 * @param Vector Interrupt vector identifier
 */
void NVIC_GenerateSoftwareInterrupt(NVIC_InterruptVectorType Vector);

#if (NVIC_VERSION_INFO_API == STD_ON)
/**
 * @brief Retrieve version information for the NVIC driver.
 * @param versionInfoPtr Pointer to version info structure
 */
void NVIC_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
#endif

#endif /* MCAL_NVIC_NVIC_H_ */
