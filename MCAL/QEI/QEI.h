/**
 * @file QEI.h
 * @brief Quadrature Encoder Interface (QEI) Driver API for TM4C123GH6PM
 * @details AUTOSAR-compliant API for configuring and operating the QEI modules.
 */

#ifndef MCAL_QEI_QEI_H_
#define MCAL_QEI_QEI_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"
#include "../../CONFIG/Compiler.h"
#include "../../CONFIG/Det.h"
#include "QEI_Types.h"
#include "QEI_Cfg.h"

/* ===================[Version Information]=================== */
#define QEI_VENDOR_ID                    (0x1234u)
#define QEI_MODULE_ID                    (150u)
#define QEI_INSTANCE_ID                  (0u)

#define QEI_SW_MAJOR_VERSION             (1u)
#define QEI_SW_MINOR_VERSION             (0u)
#define QEI_SW_PATCH_VERSION             (0u)

#define QEI_AR_RELEASE_MAJOR_VERSION     (4u)
#define QEI_AR_RELEASE_MINOR_VERSION     (4u)
#define QEI_AR_RELEASE_REVISION_VERSION  (0u)

/* ===================[Service IDs]=================== */
#define QEI_INIT_SID                     (0x00u)
#define QEI_DEINIT_SID                   (0x01u)
#define QEI_GET_POSITION_SID             (0x02u)
#define QEI_SET_POSITION_SID             (0x03u)
#define QEI_GET_VELOCITY_SID             (0x04u)
#define QEI_ENABLE_INTERRUPT_SID         (0x05u)
#define QEI_DISABLE_INTERRUPT_SID        (0x06u)
#define QEI_CLEAR_INTERRUPT_SID          (0x07u)
#define QEI_GET_STATUS_SID               (0x08u)
#define QEI_GET_DIRECTION_SID            (0x09u)
#define QEI_GET_VERSION_INFO_SID         (0x0Au)

/* ===================[Error Codes]=================== */
#define QEI_E_PARAM_POINTER              (0x01u)
#define QEI_E_ALREADY_INITIALIZED        (0x02u)
#define QEI_E_UNINIT                     (0x03u)
#define QEI_E_PARAM_MODULE               (0x04u)
#define QEI_E_PARAM_POSITION             (0x05u)
#define QEI_E_PARAM_VELOCITY             (0x06u)

/* ===================[API Function Prototypes]=================== */

/**
 * @brief Initialize the QEI driver.
 * @param ConfigPtr Pointer to QEI configuration structure
 */
void Qei_Init(const Qei_ConfigType* ConfigPtr);

/**
 * @brief De-initialize the QEI driver.
 */
void Qei_DeInit(void);

/**
 * @brief Get current position counter value.
 * @return Position count
 */
uint32 Qei_GetPosition(void);

/**
 * @brief Set the position counter value.
 * @param Position New position count
 */
void Qei_SetPosition(uint32 Position);

/**
 * @brief Get current velocity (counts per configured interval).
 * @return Velocity value
 */
uint32 Qei_GetVelocity(void);

/**
 * @brief Enable QEI interrupts specified by mask.
 * @param mask Interrupt mask to enable
 */
void Qei_EnableInterrupt(Qei_InterruptMaskType mask);

/**
 * @brief Disable QEI interrupts specified by mask.
 * @param mask Interrupt mask to disable
 */
void Qei_DisableInterrupt(Qei_InterruptMaskType mask);

/**
 * @brief Clear pending interrupts specified by mask.
 * @param mask Interrupt mask to clear
 */
void Qei_ClearInterrupt(Qei_InterruptMaskType mask);

/**
 * @brief Get current QEI driver status.
 * @return Qei_StatusType status
 */
Qei_StatusType Qei_GetStatus(void);

/**
 * @brief Get current rotation direction.
 * @return Qei_DirectionType direction
 */
Qei_DirectionType Qei_GetDirection(void);

#if (QEI_VERSION_INFO_API == STD_ON)
/**
 * @brief Retrieve version information for the QEI driver.
 * @param versionInfoPtr Pointer to version info structure
 */
void Qei_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
#endif

/**
 * @brief QEI0 interrupt handler wrapper.
 */
void Qei_Qei0Handler(void);

/**
 * @brief QEI1 interrupt handler wrapper.
 */
void Qei_Qei1Handler(void);

#endif /* MCAL_QEI_QEI_H_ */
