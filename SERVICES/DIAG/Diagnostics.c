/**
 * @file Diagnostics.c
 * @brief Diagnostic Event Manager Service Implementation
 * @details Event logging, DTC management, and debug output
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#include "Diagnostics.h"
#include "../../MCAL/UART/Uart.h"
#include <string.h>

/* ===================[Private Variables]=================== */
static const Diag_ConfigType* Diag_ConfigPtr = NULL_PTR;
static Diag_StatusType Diag_ModuleStatus = DIAG_STATUS_UNINIT;

/* Event log (circular buffer) */
static Diag_EventType Diag_EventLog[DIAG_MAX_EVENTS];
static uint8 Diag_EventHead = 0u;
static uint8 Diag_EventCount = 0u;
static uint16 Diag_TotalEventCount = 0u;

/* DTC storage */
static Diag_DtcType Diag_DtcList[DIAG_MAX_DTCS];
static uint8 Diag_DtcCount = 0u;

/* System info */
static uint32 Diag_StartupTicks = 0u;

/* ===================[Private Functions]=================== */

/**
 * @brief Get current tick count (placeholder)
 */
static uint32 Diag_GetTicks(void)
{
    /* TODO: Integrate with FreeRTOS xTaskGetTickCount or Timer */
    static uint32 tickCounter = 0u;
    return tickCounter++;
}

/**
 * @brief Find DTC in list
 */
static sint8 Diag_FindDtc(uint16 DtcCode)
{
    uint8 i;
    
    for (i = 0u; i < Diag_DtcCount; i++)
    {
        if (Diag_DtcList[i].DtcCode == DtcCode)
        {
            return (sint8)i;
        }
    }
    
    return -1;  /* Not found */
}

/**
 * @brief Convert integer to string (simple implementation)
 */
static void Diag_IntToStr(sint32 value, char* buffer)
{
    char temp[12];
    uint8 i = 0u;
    uint8 j;
    boolean negative = FALSE;
    uint32 uvalue;
    
    if (value < 0)
    {
        negative = TRUE;
        uvalue = (uint32)(-value);
    }
    else
    {
        uvalue = (uint32)value;
    }
    
    /* Handle zero */
    if (uvalue == 0u)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    /* Convert digits */
    while (uvalue > 0u)
    {
        temp[i++] = (char)('0' + (uvalue % 10u));
        uvalue /= 10u;
    }
    
    /* Build output */
    j = 0u;
    if (negative)
    {
        buffer[j++] = '-';
    }
    
    while (i > 0u)
    {
        i--;
        buffer[j++] = temp[i];
    }
    buffer[j] = '\0';
}

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize the Diagnostics service
 */
void Diag_Init(const Diag_ConfigType* ConfigPtr)
{
    uint8 i;
    
    if (ConfigPtr == NULL_PTR)
    {
        return;
    }
    
    Diag_ConfigPtr = ConfigPtr;
    
    /* Clear event log */
    for (i = 0u; i < DIAG_MAX_EVENTS; i++)
    {
        Diag_EventLog[i].Timestamp = 0u;
        Diag_EventLog[i].EventCode = 0u;
        Diag_EventLog[i].Source = 0u;
        Diag_EventLog[i].Severity = 0u;
    }
    Diag_EventHead = 0u;
    Diag_EventCount = 0u;
    Diag_TotalEventCount = 0u;
    
    /* Clear DTC list */
    for (i = 0u; i < DIAG_MAX_DTCS; i++)
    {
        Diag_DtcList[i].DtcCode = DIAG_DTC_NONE;
        Diag_DtcList[i].Status = 0u;
        Diag_DtcList[i].OccurrenceCount = 0u;
    }
    Diag_DtcCount = 0u;
    
    Diag_StartupTicks = Diag_GetTicks();
    Diag_ModuleStatus = DIAG_STATUS_IDLE;
    
    /* Log startup event */
    Diag_LogEvent(DIAG_SRC_SYSTEM, 0x0001u, DIAG_SEVERITY_INFO, NULL_PTR);
}

/**
 * @brief De-initialize the Diagnostics service
 */
void Diag_DeInit(void)
{
    Diag_ConfigPtr = NULL_PTR;
    Diag_ModuleStatus = DIAG_STATUS_UNINIT;
}

/**
 * @brief Log an event
 */
void Diag_LogEvent(uint8 Source, uint16 EventCode, uint8 Severity, const uint8* Data)
{
    Diag_EventType* event;
    
    if (Diag_ModuleStatus == DIAG_STATUS_UNINIT)
    {
        return;
    }
    
    if ((Diag_ConfigPtr != NULL_PTR) && (!Diag_ConfigPtr->EventLogEnabled))
    {
        return;
    }
    
    /* Get next slot in circular buffer */
    event = &Diag_EventLog[Diag_EventHead];
    
    event->Timestamp = Diag_GetTicks();
    event->EventCode = EventCode;
    event->Source = Source;
    event->Severity = Severity;
    
    if (Data != NULL_PTR)
    {
        event->Data[0] = Data[0];
        event->Data[1] = Data[1];
        event->Data[2] = Data[2];
        event->Data[3] = Data[3];
    }
    else
    {
        event->Data[0] = 0u;
        event->Data[1] = 0u;
        event->Data[2] = 0u;
        event->Data[3] = 0u;
    }
    
    /* Advance head */
    Diag_EventHead = (Diag_EventHead + 1u) % DIAG_MAX_EVENTS;
    
    if (Diag_EventCount < DIAG_MAX_EVENTS)
    {
        Diag_EventCount++;
    }
    Diag_TotalEventCount++;
    
    /* Debug output for warnings and errors */
    if ((Diag_ConfigPtr != NULL_PTR) && (Diag_ConfigPtr->DebugEnabled))
    {
        if (Severity >= DIAG_SEVERITY_WARNING)
        {
            Diag_DebugPrintValue("[DIAG] Event: ", (sint32)EventCode);
        }
    }
}

/**
 * @brief Report a DTC
 */
void Diag_ReportDtc(uint16 DtcCode, boolean Active)
{
    sint8 index;
    Diag_DtcType* dtc;
    uint32 currentTicks;
    
    if (Diag_ModuleStatus == DIAG_STATUS_UNINIT)
    {
        return;
    }
    
    currentTicks = Diag_GetTicks();
    index = Diag_FindDtc(DtcCode);
    
    if (index >= 0)
    {
        /* Existing DTC */
        dtc = &Diag_DtcList[index];
        dtc->Status = Active ? 1u : 0u;
        dtc->LastOccurrence = currentTicks;
        
        if (Active)
        {
            if (dtc->OccurrenceCount < 255u)
            {
                dtc->OccurrenceCount++;
            }
        }
    }
    else if (Active && (Diag_DtcCount < DIAG_MAX_DTCS))
    {
        /* New DTC */
        dtc = &Diag_DtcList[Diag_DtcCount];
        dtc->DtcCode = DtcCode;
        dtc->Status = 1u;
        dtc->OccurrenceCount = 1u;
        dtc->FirstOccurrence = currentTicks;
        dtc->LastOccurrence = currentTicks;
        Diag_DtcCount++;
    }
    else
    {
        /* DTC list full or clearing non-existent DTC */
    }
    
    /* Log as event too */
    Diag_LogEvent(DIAG_SRC_SYSTEM, DtcCode, Active ? DIAG_SEVERITY_ERROR : DIAG_SEVERITY_INFO, NULL_PTR);
}

/**
 * @brief Clear all DTCs
 */
void Diag_ClearAllDtcs(void)
{
    uint8 i;
    
    for (i = 0u; i < DIAG_MAX_DTCS; i++)
    {
        Diag_DtcList[i].DtcCode = DIAG_DTC_NONE;
        Diag_DtcList[i].Status = 0u;
        Diag_DtcList[i].OccurrenceCount = 0u;
    }
    Diag_DtcCount = 0u;
    
    Diag_LogEvent(DIAG_SRC_SYSTEM, 0x0002u, DIAG_SEVERITY_INFO, NULL_PTR);
}

/**
 * @brief Clear a specific DTC
 */
void Diag_ClearDtc(uint16 DtcCode)
{
    sint8 index;
    uint8 i;
    
    index = Diag_FindDtc(DtcCode);
    
    if (index >= 0)
    {
        /* Remove by shifting array */
        for (i = (uint8)index; i < (Diag_DtcCount - 1u); i++)
        {
            Diag_DtcList[i] = Diag_DtcList[i + 1u];
        }
        Diag_DtcCount--;
        
        /* Clear last entry */
        Diag_DtcList[Diag_DtcCount].DtcCode = DIAG_DTC_NONE;
        Diag_DtcList[Diag_DtcCount].Status = 0u;
    }
}

/**
 * @brief Get number of active DTCs
 */
uint8 Diag_GetActiveDtcCount(void)
{
    uint8 count = 0u;
    uint8 i;
    
    for (i = 0u; i < Diag_DtcCount; i++)
    {
        if (Diag_DtcList[i].Status == 1u)
        {
            count++;
        }
    }
    
    return count;
}

/**
 * @brief Get DTC info
 */
Std_ReturnType Diag_GetDtc(uint16 DtcCode, Diag_DtcType* DtcPtr)
{
    sint8 index;
    
    if (DtcPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    index = Diag_FindDtc(DtcCode);
    
    if (index >= 0)
    {
        *DtcPtr = Diag_DtcList[index];
        return E_OK;
    }
    
    return E_NOT_OK;
}

/**
 * @brief Get system health status
 */
Std_ReturnType Diag_GetHealth(Diag_HealthType* HealthPtr)
{
    uint8 activeDtcs;
    
    if (HealthPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    activeDtcs = Diag_GetActiveDtcCount();
    
    /* Calculate health score (simple algorithm) */
    if (activeDtcs == 0u)
    {
        HealthPtr->OverallHealth = 100u;
    }
    else if (activeDtcs <= 2u)
    {
        HealthPtr->OverallHealth = 75u;
    }
    else if (activeDtcs <= 5u)
    {
        HealthPtr->OverallHealth = 50u;
    }
    else
    {
        HealthPtr->OverallHealth = 25u;
    }
    
    HealthPtr->ActiveDtcCount = activeDtcs;
    HealthPtr->TotalEventCount = Diag_TotalEventCount;
    HealthPtr->UptimeTicks = Diag_GetTicks() - Diag_StartupTicks;
    HealthPtr->LastResetReason = 0u;  /* TODO: Read from reset register */
    
    return E_OK;
}

/**
 * @brief Get the most recent events
 */
uint8 Diag_GetRecentEvents(Diag_EventType* EventArray, uint8 MaxCount)
{
    uint8 count;
    uint8 readIndex;
    uint8 i;
    
    if (EventArray == NULL_PTR)
    {
        return 0u;
    }
    
    count = (MaxCount < Diag_EventCount) ? MaxCount : Diag_EventCount;
    
    /* Read from newest to oldest */
    readIndex = (Diag_EventHead == 0u) ? (DIAG_MAX_EVENTS - 1u) : (Diag_EventHead - 1u);
    
    for (i = 0u; i < count; i++)
    {
        EventArray[i] = Diag_EventLog[readIndex];
        
        if (readIndex == 0u)
        {
            readIndex = DIAG_MAX_EVENTS - 1u;
        }
        else
        {
            readIndex--;
        }
    }
    
    return count;
}

/**
 * @brief Print debug message to UART
 */
void Diag_DebugPrint(const char* Message)
{
    if (Diag_ModuleStatus == DIAG_STATUS_UNINIT)
    {
        return;
    }
    
    if ((Diag_ConfigPtr == NULL_PTR) || (!Diag_ConfigPtr->DebugEnabled))
    {
        return;
    }
    
    if (Message != NULL_PTR)
    {
        while (*Message != '\0')
        {
            Uart_SendByte(Diag_ConfigPtr->DebugUartModule, (uint8)*Message);
            Message++;
        }
    }
}

/**
 * @brief Print formatted debug message with value
 */
void Diag_DebugPrintValue(const char* Prefix, sint32 Value)
{
    char valueStr[12];
    
    if (Diag_ModuleStatus == DIAG_STATUS_UNINIT)
    {
        return;
    }
    
    Diag_DebugPrint(Prefix);
    Diag_IntToStr(Value, valueStr);
    Diag_DebugPrint(valueStr);
    Diag_DebugPrint("\r\n");
}

/**
 * @brief Get service status
 */
Diag_StatusType Diag_GetStatus(void)
{
    return Diag_ModuleStatus;
}
