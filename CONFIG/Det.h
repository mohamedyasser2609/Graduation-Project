/*
 * @file Det.h
 * @brief Development Error Tracer interface for ARM Cortex M4F
 * @details This file provides the interface for Development Error Tracer (DET)
 *          functionality. DET is used for error reporting and debugging during
 *          development phase. Optimized for TM4C123GH6PM platform.
 *
 * @author Mohamed Yasser
 * @date Oct 12, 2025
 * @version 1.0.0
 */

#ifndef CONFIG_DET_H_
#define CONFIG_DET_H_

/* Include standard types */
#include "std_types.h"

/* ===================[DET Configuration]=================== */
#define DET_SW_MAJOR_VERSION         (1u)
#define DET_SW_MINOR_VERSION         (0u)
#define DET_SW_PATCH_VERSION         (0u)

/* DET enable/disable configuration */
#define DET_ENABLED                  1u  /* 1=Enabled, 0=Disabled */

/* DET buffer configuration */
#define DET_MAX_ERROR_REPORTS        16u  /* Maximum number of error reports to store */
#define DET_ERROR_BUFFER_SIZE        (DET_MAX_ERROR_REPORTS * 8u)  /* 8 bytes per error */

/* DET callback configuration */
#define DET_CALLBACK_ENABLED         1u  /* Enable callback function */

/* ===================[DET Error Codes]=================== */
/* Module IDs - Application specific */
#define DET_MODULE_ID_APP            0x01u
#define DET_MODULE_ID_MCAL           0x02u
#define DET_MODULE_ID_ECUAL          0x03u
#define DET_MODULE_ID_SERVICES       0x04u
#define DET_MODULE_ID_FREERTOS       0x05u

/* Instance IDs */
#define DET_INSTANCE_ID_DEFAULT      0x00u

/* API IDs - Common APIs */
#define DET_API_ID_INIT             0x00u
#define DET_API_ID_DEINIT           0x01u
#define DET_API_ID_GETVERSIONINFO   0x02u
#define DET_API_ID_REPORTERROR      0x03u

/* Error Types */
#define DET_ERROR_TYPE_PARAM_POINTER    0x01u  /* Invalid parameter pointer */
#define DET_ERROR_TYPE_PARAM_VALUE      0x02u  /* Invalid parameter value */
#define DET_ERROR_TYPE_PARAM_OUTOFRANGE 0x03u  /* Parameter out of range */
#define DET_ERROR_TYPE_UNINIT           0x04u  /* Module not initialized */
#define DET_ERROR_TYPE_INIT             0x05u  /* Module already initialized */
#define DET_ERROR_TYPE_STATE            0x06u  /* Invalid state */
#define DET_ERROR_TYPE_CALL             0x07u  /* Invalid function call */
#define DET_ERROR_TYPE_BUFFER           0x08u  /* Buffer error */
#define DET_ERROR_TYPE_TIMEOUT          0x09u  /* Timeout error */
#define DET_ERROR_TYPE_HARDWARE         0x0Au  /* Hardware error */

/* ===================[Type Definitions]=================== */
/* Note: Std_VersionInfoType is defined in std_types.h */

/* DET Error Report Structure */
typedef struct {
    uint16 ModuleId;        /**< @brief Module ID that reported the error */
    uint8  InstanceId;       /**< @brief Instance ID */
    uint8  ApiId;            /**< @brief API ID */
    uint8  ErrorId;         /**< @brief Error ID */
    uint32 Timestamp;        /**< @brief Timestamp when error occurred */
} Det_ErrorReportType;

/* DET Status Type */
typedef uint8 Det_StatusType;
#define DET_UNINIT              0x00u  /**< @brief DET not initialized */
#define DET_INIT                0x01u  /**< @brief DET initialized */

/* DET Return Type */
typedef uint8 Det_ReturnType;
#define DET_OK                  0x00u  /**< @brief Operation successful */
#define DET_NOT_OK              0x01u  /**< @brief Operation failed */

/* ===================[DET Function Prototypes]=================== */

/**
 * @brief Initialize the Development Error Tracer
 * @details This function initializes the DET module and prepares it for
 *          error reporting. Must be called before any other DET function.
 * @return Det_ReturnType - DET_OK if successful, DET_NOT_OK otherwise
 */
Det_ReturnType Det_Init(void);

/**
 * @brief De-initialize the Development Error Tracer
 * @details This function de-initializes the DET module and cleans up
 *          any allocated resources.
 * @return Det_ReturnType - DET_OK if successful, DET_NOT_OK otherwise
 */
Det_ReturnType Det_DeInit(void);

/**
 * @brief Report a development error
 * @details This function reports a development error with the specified
 *          parameters. The error is stored in the error buffer and
 *          optionally logged.
 * @param[in] ModuleId - ID of the module reporting the error
 * @param[in] InstanceId - Instance ID of the module
 * @param[in] ApiId - API ID where the error occurred
 * @param[in] ErrorId - Specific error ID
 * @return void
 */
void Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);

/**
 * @brief Get version information
 * @details This function returns the version information of the DET module.
 * @param[out] VersionInfo - Pointer to version information structure
 * @return Det_ReturnType - DET_OK if successful, DET_NOT_OK otherwise
 */
Det_ReturnType Det_GetVersionInfo(Std_VersionInfoType* VersionInfo);

/**
 * @brief Get error report from buffer
 * @details This function retrieves an error report from the error buffer.
 * @param[out] ErrorReport - Pointer to error report structure
 * @param[in] Index - Index of the error report to retrieve
 * @return Det_ReturnType - DET_OK if successful, DET_NOT_OK otherwise
 */
Det_ReturnType Det_GetErrorReport(Det_ErrorReportType* ErrorReport, uint8 Index);

/**
 * @brief Clear error buffer
 * @details This function clears all error reports from the error buffer.
 * @return Det_ReturnType - DET_OK if successful, DET_NOT_OK otherwise
 */
Det_ReturnType Det_ClearErrorBuffer(void);

/**
 * @brief Get number of error reports
 * @details This function returns the number of error reports currently
 *          stored in the error buffer.
 * @return uint8 - Number of error reports
 */
uint8 Det_GetErrorCount(void);

/* ===================[DET Macro Helpers]=================== */

/* Convenience macros for common error reporting */
#define DET_REPORT_PARAM_POINTER(ModuleId, InstanceId, ApiId) \
    Det_ReportError((ModuleId), (InstanceId), (ApiId), DET_ERROR_TYPE_PARAM_POINTER)

#define DET_REPORT_PARAM_VALUE(ModuleId, InstanceId, ApiId) \
    Det_ReportError((ModuleId), (InstanceId), (ApiId), DET_ERROR_TYPE_PARAM_VALUE)

#define DET_REPORT_PARAM_OUTOFRANGE(ModuleId, InstanceId, ApiId) \
    Det_ReportError((ModuleId), (InstanceId), (ApiId), DET_ERROR_TYPE_PARAM_OUTOFRANGE)

#define DET_REPORT_UNINIT(ModuleId, InstanceId, ApiId) \
    Det_ReportError((ModuleId), (InstanceId), (ApiId), DET_ERROR_TYPE_UNINIT)

#define DET_REPORT_INIT(ModuleId, InstanceId, ApiId) \
    Det_ReportError((ModuleId), (InstanceId), (ApiId), DET_ERROR_TYPE_INIT)

#define DET_REPORT_STATE(ModuleId, InstanceId, ApiId) \
    Det_ReportError((ModuleId), (InstanceId), (ApiId), DET_ERROR_TYPE_STATE)

#define DET_REPORT_CALL(ModuleId, InstanceId, ApiId) \
    Det_ReportError((ModuleId), (InstanceId), (ApiId), DET_ERROR_TYPE_CALL)

#define DET_REPORT_BUFFER(ModuleId, InstanceId, ApiId) \
    Det_ReportError((ModuleId), (InstanceId), (ApiId), DET_ERROR_TYPE_BUFFER)

#define DET_REPORT_TIMEOUT(ModuleId, InstanceId, ApiId) \
    Det_ReportError((ModuleId), (InstanceId), (ApiId), DET_ERROR_TYPE_TIMEOUT)

#define DET_REPORT_HARDWARE(ModuleId, InstanceId, ApiId) \
    Det_ReportError((ModuleId), (InstanceId), (ApiId), DET_ERROR_TYPE_HARDWARE)

/* ===================[DET Callback Function Type]=================== */
#if (DET_CALLBACK_ENABLED == 1u)
/**
 * @brief DET callback function type
 * @details This function is called whenever an error is reported to DET.
 *          It can be used for custom error handling or logging.
 * @param[in] ErrorReport - Pointer to the error report
 * @return void
 */
typedef void (*Det_CallbackType)(const Det_ErrorReportType* ErrorReport);

/**
 * @brief Set DET callback function
 * @details This function sets the callback function that will be called
 *          whenever an error is reported.
 * @param[in] Callback - Pointer to callback function
 * @return Det_ReturnType - DET_OK if successful, DET_NOT_OK otherwise
 */
Det_ReturnType Det_SetCallback(Det_CallbackType Callback);
#endif


#endif /* CONFIG_DET_H_ */
