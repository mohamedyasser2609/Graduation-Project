/**
 * @file WDG.h
 * @brief Watchdog (WDG) Driver API for TM4C123GH6PM
 * @details AUTOSAR-compliant API for configuring and servicing the watchdog timer.
 */

#ifndef MCAL_WDG_WDG_H_
#define MCAL_WDG_WDG_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"
#include "../../CONFIG/Compiler.h"
#include "../../CONFIG/Det.h"
#include "WDG_Types.h"
#include "WDG_Cfg.h"

/* ===================[Version Information]=================== */
#define WDG_VENDOR_ID                    (0x1234u)
#define WDG_MODULE_ID                    (140u)
#define WDG_INSTANCE_ID                  (0u)

#define WDG_SW_MAJOR_VERSION             (1u)
#define WDG_SW_MINOR_VERSION             (0u)
#define WDG_SW_PATCH_VERSION             (0u)

#define WDG_AR_RELEASE_MAJOR_VERSION     (4u)
#define WDG_AR_RELEASE_MINOR_VERSION     (4u)
#define WDG_AR_RELEASE_REVISION_VERSION  (0u)

/* ===================[Service IDs]=================== */
#define WDG_INIT_SID                     (0x00u)
#define WDG_SET_MODE_SID                 (0x01u)
#define WDG_SET_TRIGGER_COND_SID         (0x02u)
#define WDG_SERVICE_SID                  (0x03u)
#define WDG_DISABLE_SID                  (0x04u)
#define WDG_GET_STATUS_SID               (0x05u)
#define WDG_GET_VERSION_INFO_SID         (0x06u)

/* ===================[Error Codes]=================== */
#define WDG_E_PARAM_POINTER              (0x01u)
#define WDG_E_ALREADY_INITIALIZED        (0x02u)
#define WDG_E_PARAM_MODE                 (0x03u)
#define WDG_E_PARAM_TIMEOUT              (0x04u)
#define WDG_E_PARAM_INSTANCE             (0x05u)
#define WDG_E_UNINIT                     (0x06u)

/* ===================[API Function Prototypes]=================== */

/**
 * @brief Initialize the watchdog driver.
 * @param ConfigPtr Pointer to watchdog configuration structure
 */
void Wdg_Init(const Wdg_ConfigType* ConfigPtr);

/**
 * @brief Set watchdog trigger mode (normal or fast).
 * @param Mode Trigger mode selection
 */
void Wdg_SetTriggerMode(Wdg_TriggerModeType Mode);

/**
 * @brief Update watchdog timeout value.
 * @param TimeoutTicks Timeout in timer ticks
 */
void Wdg_SetTriggerCondition(uint32 TimeoutTicks);

/**
 * @brief Service (kick) the watchdog to prevent timeout.
 */
void Wdg_Service(void);

/**
 * @brief Disable the watchdog timer.
 */
void Wdg_Disable(void);

/**
 * @brief Get current watchdog driver status.
 * @return Wdg_StatusType Current driver status
 */
Wdg_StatusType Wdg_GetStatus(void);

#if (WDG_VERSION_INFO_API == STD_ON)
/**
 * @brief Retrieve version information for the watchdog driver.
 * @param versionInfoPtr Pointer to version info structure
 */
void Wdg_GetVersionInfo(Std_VersionInfoType* versionInfoPtr);
#endif

/**
 * @brief Watchdog 0 interrupt handler wrapper.
 */
void Wdg_Watchdog0Handler(void);

/**
 * @brief Watchdog 1 interrupt handler wrapper.
 */
void Wdg_Watchdog1Handler(void);

#endif /* MCAL_WDG_WDG_H_ */
