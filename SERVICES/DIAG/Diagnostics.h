/**
 * @file Diagnostics.h
 * @brief Diagnostic Event Manager Service
 * @details AUTOSAR DEM-like diagnostic event handling and storage
 *
 * Features:
 * - Event logging with timestamps
 * - Error code storage (DTC-like)
 * - Event counter tracking
 * - Debug output via UART
 * - System health reporting
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef SERVICES_DIAG_DIAGNOSTICS_H_
#define SERVICES_DIAG_DIAGNOSTICS_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[Macros]=================== */
/** @brief Module identification */
#define DIAG_MODULE_ID              (0xB0u)
#define DIAG_VENDOR_ID              (0x00u)
#define DIAG_INSTANCE_ID            (0x00u)

/** @brief Software version */
#define DIAG_SW_MAJOR_VERSION       (1u)
#define DIAG_SW_MINOR_VERSION       (0u)
#define DIAG_SW_PATCH_VERSION       (0u)

/** @brief Maximum event log entries */
#define DIAG_MAX_EVENTS             (32u)

/** @brief Maximum DTC entries */
#define DIAG_MAX_DTCS               (16u)

/** @brief Event severity levels */
#define DIAG_SEVERITY_INFO          (0u)
#define DIAG_SEVERITY_WARNING       (1u)
#define DIAG_SEVERITY_ERROR         (2u)
#define DIAG_SEVERITY_FATAL         (3u)

/** @brief Event source IDs */
#define DIAG_SRC_SYSTEM             (0x00u)
#define DIAG_SRC_MOTOR              (0x10u)
#define DIAG_SRC_SENSOR             (0x20u)
#define DIAG_SRC_COMM               (0x30u)
#define DIAG_SRC_THERMAL            (0x40u)
#define DIAG_SRC_SAFETY             (0x50u)

/** @brief Error codes (DTC-like) */
#define DIAG_DTC_NONE               (0x0000u)
#define DIAG_DTC_MOTOR_OVERLOAD     (0x1001u)
#define DIAG_DTC_MOTOR_STALL        (0x1002u)
#define DIAG_DTC_MOTOR_COMM         (0x1003u)
#define DIAG_DTC_IMU_FAIL           (0x2001u)
#define DIAG_DTC_GPS_FAIL           (0x2002u)
#define DIAG_DTC_ENCODER_FAIL       (0x2003u)
#define DIAG_DTC_TEMP_SENSOR_FAIL   (0x2004u)
#define DIAG_DTC_COMM_TIMEOUT       (0x3001u)
#define DIAG_DTC_COMM_CHECKSUM      (0x3002u)
#define DIAG_DTC_THERMAL_CRITICAL   (0x4001u)
#define DIAG_DTC_THERMAL_SHUTDOWN   (0x4002u)
#define DIAG_DTC_WATCHDOG           (0x5001u)
#define DIAG_DTC_STACK_OVERFLOW     (0x5002u)
#define DIAG_DTC_HEAP_EXHAUSTED     (0x5003u)

/* ===================[Type Definitions]=================== */

/**
 * @brief Diagnostic event status
 */
typedef enum
{
    DIAG_STATUS_UNINIT      = 0u,
    DIAG_STATUS_IDLE        = 1u,
    DIAG_STATUS_BUSY        = 2u,
    DIAG_STATUS_ERROR       = 3u
} Diag_StatusType;

/**
 * @brief Event entry structure
 */
typedef struct
{
    uint32      Timestamp;      /**< Event timestamp (ticks) */
    uint16      EventCode;      /**< Event/DTC code */
    uint8       Source;         /**< Event source ID */
    uint8       Severity;       /**< Severity level */
    uint8       Data[4];        /**< Additional data */
} Diag_EventType;

/**
 * @brief DTC entry structure
 */
typedef struct
{
    uint16      DtcCode;        /**< Diagnostic trouble code */
    uint8       Status;         /**< 0=passive, 1=active */
    uint8       OccurrenceCount;/**< Number of occurrences */
    uint32      FirstOccurrence;/**< Timestamp of first occurrence */
    uint32      LastOccurrence; /**< Timestamp of last occurrence */
} Diag_DtcType;

/**
 * @brief System health status
 */
typedef struct
{
    uint8       OverallHealth;  /**< 0-100 health score */
    uint8       ActiveDtcCount; /**< Number of active DTCs */
    uint16      TotalEventCount;/**< Total logged events */
    uint32      UptimeTicks;    /**< System uptime */
    uint8       LastResetReason;/**< Reason for last reset */
} Diag_HealthType;

/**
 * @brief Configuration structure
 */
typedef struct
{
    uint8       DebugUartModule;/**< UART for debug output */
    boolean     DebugEnabled;   /**< Enable debug output */
    boolean     EventLogEnabled;/**< Enable event logging */
} Diag_ConfigType;

/* ===================[API Declarations]=================== */

/**
 * @brief Initialize the Diagnostics service
 * @param[in] ConfigPtr Pointer to configuration
 */
void Diag_Init(const Diag_ConfigType* ConfigPtr);

/**
 * @brief De-initialize the Diagnostics service
 */
void Diag_DeInit(void);

/**
 * @brief Log an event
 * @param[in] Source Event source ID
 * @param[in] EventCode Event code
 * @param[in] Severity Severity level
 * @param[in] Data Optional data (up to 4 bytes, can be NULL)
 */
void Diag_LogEvent(uint8 Source, uint16 EventCode, uint8 Severity, const uint8* Data);

/**
 * @brief Report a DTC
 * @param[in] DtcCode Diagnostic trouble code
 * @param[in] Active TRUE if fault is active, FALSE if cleared
 */
void Diag_ReportDtc(uint16 DtcCode, boolean Active);

/**
 * @brief Clear all DTCs
 */
void Diag_ClearAllDtcs(void);

/**
 * @brief Clear a specific DTC
 * @param[in] DtcCode DTC to clear
 */
void Diag_ClearDtc(uint16 DtcCode);

/**
 * @brief Get number of active DTCs
 * @return Count of active DTCs
 */
uint8 Diag_GetActiveDtcCount(void);

/**
 * @brief Get DTC info
 * @param[in] DtcCode DTC to query
 * @param[out] DtcPtr Pointer to store DTC info
 * @return E_OK if found, E_NOT_OK otherwise
 */
Std_ReturnType Diag_GetDtc(uint16 DtcCode, Diag_DtcType* DtcPtr);

/**
 * @brief Get system health status
 * @param[out] HealthPtr Pointer to store health data
 * @return E_OK on success
 */
Std_ReturnType Diag_GetHealth(Diag_HealthType* HealthPtr);

/**
 * @brief Get the most recent events
 * @param[out] EventArray Array to store events
 * @param[in] MaxCount Maximum events to retrieve
 * @return Number of events retrieved
 */
uint8 Diag_GetRecentEvents(Diag_EventType* EventArray, uint8 MaxCount);

/**
 * @brief Print debug message to UART
 * @param[in] Message Null-terminated string to print
 */
void Diag_DebugPrint(const char* Message);

/**
 * @brief Print formatted debug message with value
 * @param[in] Prefix Message prefix
 * @param[in] Value Integer value to print
 */
void Diag_DebugPrintValue(const char* Prefix, sint32 Value);

/**
 * @brief Get service status
 * @return Current status
 */
Diag_StatusType Diag_GetStatus(void);

#endif /* SERVICES_DIAG_DIAGNOSTICS_H_ */
