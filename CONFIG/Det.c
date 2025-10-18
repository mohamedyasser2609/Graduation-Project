/*
 * @file Det.c
 * @brief Development Error Tracer implementation for ARM Cortex M4F
 * @details This file implements the Development Error Tracer (DET) functionality
 *          for error reporting and debugging during development phase.
 *          Optimized for TM4C123GH6PM platform.
 *
 * @author Mohamed Yasser
 * @date Oct 12, 2025
 * @version 1.0.0
 */

/* Include DET interface */
#include "Det.h"

/* Include platform types for PLATFORM_VENDOR_ID */
#include "Platform_Types.h"

/* Include compiler abstraction */
#include "Compiler.h"

/* Include memory mapping */
#include "MemMap.h"

/* Include standard library for memset */
#include <string.h>

/* ===================[DET Internal Data Types]=================== */
/* DET Internal State */
typedef enum {
    DET_STATE_UNINIT = 0x00u,
    DET_STATE_INIT = 0x01u
} Det_InternalStateType;

/* ===================[DET Internal Variables]=================== */
/* DET Error Buffer */
static Det_ErrorReportType Det_ErrorBuffer[DET_MAX_ERROR_REPORTS];
static uint8 Det_ErrorCount = 0u;
static uint8 Det_ErrorIndex = 0u;
static Det_InternalStateType Det_State = DET_STATE_UNINIT;
static uint32 Det_Timestamp = 0u;

#if (DET_CALLBACK_ENABLED == 1u)
/* DET Callback Function */
static Det_CallbackType Det_Callback = NULL_PTR;
#endif

/* ===================[DET Internal Functions]=================== */

/**
 * @brief Internal function to get current timestamp
 * @details This function returns a simple timestamp counter.
 *          In a real implementation, this could use a system timer.
 * @return uint32 - Current timestamp
 */
static uint32 Det_GetTimestamp(void)
{
    return Det_Timestamp++;
}

/**
 * @brief Internal function to add error report to buffer
 * @details This function adds an error report to the circular buffer.
 * @param[in] ErrorReport - Pointer to error report to add
 * @return void
 */
static void Det_AddErrorReport(const Det_ErrorReportType* ErrorReport)
{
    if (Det_ErrorCount < DET_MAX_ERROR_REPORTS) {
        Det_ErrorBuffer[Det_ErrorCount] = *ErrorReport;
        Det_ErrorCount++;
    } else {
        /* Circular buffer - overwrite oldest entry */
        Det_ErrorBuffer[Det_ErrorIndex] = *ErrorReport;
        Det_ErrorIndex = (Det_ErrorIndex + 1u) % DET_MAX_ERROR_REPORTS;
    }
}

/**
 * @brief Internal function to log error report
 * @details This function logs the error report. In a real implementation,
 *          this could send data via UART, store to flash, etc.
 * @param[in] ErrorReport - Pointer to error report to log
 * @return void
 */
static void Det_LogErrorReport(const Det_ErrorReportType* ErrorReport)
{
    /* In a real implementation, this could:
     * - Send via UART
     * - Store to flash memory
     * - Display on LCD
     * - Send to debugger
     */
    
    /* For now, we'll use a simple approach - the error is already stored in buffer */
    COMPILER_SUPPRESS_UNUSED_WARNING(ErrorReport);
}

/* ===================[DET Public Functions]=================== */

Det_ReturnType Det_Init(void)
{
    Det_ReturnType ReturnValue = DET_NOT_OK;
    
    if (DET_STATE_UNINIT == Det_State) {
        /* Clear error buffer */
        memset(Det_ErrorBuffer, 0x00u, sizeof(Det_ErrorBuffer));
        Det_ErrorCount = 0u;
        Det_ErrorIndex = 0u;
        Det_Timestamp = 0u;
        
        /* Set state to initialized */
        Det_State = DET_STATE_INIT;
        
        ReturnValue = DET_OK;
    }
    
    return ReturnValue;
}

Det_ReturnType Det_DeInit(void)
{
    Det_ReturnType ReturnValue = DET_NOT_OK;
    
    if (DET_STATE_INIT == Det_State) {
        /* Clear error buffer */
        memset(Det_ErrorBuffer, 0x00u, sizeof(Det_ErrorBuffer));
        Det_ErrorCount = 0u;
        Det_ErrorIndex = 0u;
        Det_Timestamp = 0u;
        
#if (DET_CALLBACK_ENABLED == 1u)
        Det_Callback = NULL_PTR;
#endif
        
        /* Set state to uninitialized */
        Det_State = DET_STATE_UNINIT;
        
        ReturnValue = DET_OK;
    }
    
    return ReturnValue;
}

void Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId)
{
#if (DET_ENABLED == 1u)
    Det_ErrorReportType ErrorReport;
    
    /* Fill error report structure */
    ErrorReport.ModuleId = ModuleId;
    ErrorReport.InstanceId = InstanceId;
    ErrorReport.ApiId = ApiId;
    ErrorReport.ErrorId = ErrorId;
    ErrorReport.Timestamp = Det_GetTimestamp();
    
    /* Add to error buffer */
    Det_AddErrorReport(&ErrorReport);
    
    /* Log error report */
    Det_LogErrorReport(&ErrorReport);
    
#if (DET_CALLBACK_ENABLED == 1u)
    /* Call callback function if set */
    if (NULL_PTR != Det_Callback) {
        Det_Callback(&ErrorReport);
    }
#endif

#else
    /* DET disabled - suppress unused parameter warnings */
    COMPILER_SUPPRESS_UNUSED_WARNING(ModuleId);
    COMPILER_SUPPRESS_UNUSED_WARNING(InstanceId);
    COMPILER_SUPPRESS_UNUSED_WARNING(ApiId);
    COMPILER_SUPPRESS_UNUSED_WARNING(ErrorId);
#endif
}

Det_ReturnType Det_GetVersionInfo(Std_VersionInfoType* VersionInfo)
{
    Det_ReturnType ReturnValue = DET_NOT_OK;
    
    if (NULL_PTR != VersionInfo) {
        VersionInfo->vendorID = PLATFORM_VENDOR_ID;
        VersionInfo->moduleID = DET_MODULE_ID_SERVICES;
        VersionInfo->sw_major_version = DET_SW_MAJOR_VERSION;
        VersionInfo->sw_minor_version = DET_SW_MINOR_VERSION;
        VersionInfo->sw_patch_version = DET_SW_PATCH_VERSION;
        
        ReturnValue = DET_OK;
    }
    
    return ReturnValue;
}

Det_ReturnType Det_GetErrorReport(Det_ErrorReportType* ErrorReport, uint8 Index)
{
    Det_ReturnType ReturnValue = DET_NOT_OK;
    
    if ((NULL_PTR != ErrorReport) && (Index < Det_ErrorCount)) {
        if (Det_ErrorCount < DET_MAX_ERROR_REPORTS) {
            /* Linear buffer */
            *ErrorReport = Det_ErrorBuffer[Index];
        } else {
            /* Circular buffer - calculate actual index */
            uint8 ActualIndex = (Det_ErrorIndex + Index) % DET_MAX_ERROR_REPORTS;
            *ErrorReport = Det_ErrorBuffer[ActualIndex];
        }
        
        ReturnValue = DET_OK;
    }
    
    return ReturnValue;
}

Det_ReturnType Det_ClearErrorBuffer(void)
{
    Det_ReturnType ReturnValue = DET_NOT_OK;
    
    if (DET_STATE_INIT == Det_State) {
        /* Clear error buffer */
        memset(Det_ErrorBuffer, 0x00u, sizeof(Det_ErrorBuffer));
        Det_ErrorCount = 0u;
        Det_ErrorIndex = 0u;
        
        ReturnValue = DET_OK;
    }
    
    return ReturnValue;
}

uint8 Det_GetErrorCount(void)
{
    return Det_ErrorCount;
}

#if (DET_CALLBACK_ENABLED == 1u)
Det_ReturnType Det_SetCallback(Det_CallbackType Callback)
{
    Det_ReturnType ReturnValue = DET_NOT_OK;
    
    if (DET_STATE_INIT == Det_State) {
        Det_Callback = Callback;
        ReturnValue = DET_OK;
    }
    
    return ReturnValue;
}

#endif

