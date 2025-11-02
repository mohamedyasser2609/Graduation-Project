/**
 * @file WDG.c
 * @brief Watchdog (WDG) Driver Implementation for TM4C123GH6PM
 */

#include "WDG.h"
#include "tm4c123gh6pm.h"

/* ===================[Register Definitions]=================== */
#define WDG0_BASE_ADDR                 (0x40000000u)
#define WDG1_BASE_ADDR                 (0x40001000u)

#define WDG_LOAD_OFFSET                (0x000u)
#define WDG_VALUE_OFFSET               (0x004u)
#define WDG_CTL_OFFSET                 (0x008u)
#define WDG_ICR_OFFSET                 (0x00Cu)
#define WDG_RIS_OFFSET                 (0x010u)
#define WDG_MIS_OFFSET                 (0x014u)
#define WDG_TEST_OFFSET                (0x418u)
#define WDG_LOCK_OFFSET                (0xC00u)

#define WDG_REG(base, offset)          (*((volatile uint32*)(((uint32)(base)) + ((uint32)(offset)))))

#define WDG_CLOCK_BIT(instance)        (1u << (uint32)(instance))

#define WDG_WRITE_TIMEOUT              (1000000u)

/* ===================[Private Variables]=================== */
static const Wdg_ConfigType* Wdg_ConfigPtr = NULL_PTR;
static Wdg_StatusType Wdg_ModuleStatus = WDG_STATUS_UNINIT;

/* ===================[Private Helper Functions]=================== */
static boolean Wdg_IsInstanceValid(Wdg_InstanceType instance)
{
    return (instance == WDG_INSTANCE_0) || (instance == WDG_INSTANCE_1);
}

static uint32 Wdg_GetBaseAddress(Wdg_InstanceType instance)
{
    return (instance == WDG_INSTANCE_0) ? WDG0_BASE_ADDR : WDG1_BASE_ADDR;
}

static void Wdg_EnableClock(Wdg_InstanceType instance)
{
    SYSCTL_RCGCWD_R |= WDG_CLOCK_BIT(instance);
    (void)SYSCTL_RCGCWD_R; /* Dummy read to allow clock to stabilize */
}

static void Wdg_Unlock(uint32 baseAddr)
{
    WDG_REG(baseAddr, WDG_LOCK_OFFSET) = WDT_LOCK_UNLOCK;
}

static void Wdg_Lock(uint32 baseAddr)
{
    WDG_REG(baseAddr, WDG_LOCK_OFFSET) = WDT_LOCK_LOCKED;
}

static void Wdg_WaitForWriteComplete(uint32 baseAddr)
{
    uint32 timeout = WDG_WRITE_TIMEOUT;
    while (((WDG_REG(baseAddr, WDG_CTL_OFFSET) & WDT_CTL_WRC) == 0u) && (timeout > 0u))
    {
        timeout--;
    }
}

static void Wdg_SetLoadRegister(uint32 baseAddr, uint32 timeoutTicks)
{
    WDG_REG(baseAddr, WDG_LOAD_OFFSET) = timeoutTicks;
    Wdg_WaitForWriteComplete(baseAddr);
}

static void Wdg_ConfigureControl(uint32 baseAddr, const Wdg_ConfigType* cfg)
{
    uint32 controlValue = 0u;

    if (cfg->InterruptEnable == TRUE)
    {
        controlValue |= WDT_CTL_INTEN;
    }

    if (cfg->ResetEnable == TRUE)
    {
        controlValue |= WDT_CTL_RESEN;
    }

    WDG_REG(baseAddr, WDG_CTL_OFFSET) = controlValue;
    Wdg_WaitForWriteComplete(baseAddr);
}

static void Wdg_HandleInterrupt(Wdg_InstanceType instance)
{
    uint32 baseAddr = Wdg_GetBaseAddress(instance);

    /* Clear interrupt */
    WDG_REG(baseAddr, WDG_ICR_OFFSET) = WDT_ICR_M;

    if ((Wdg_ConfigPtr != NULL_PTR) && (Wdg_ConfigPtr->Instance == instance))
    {
        if ((Wdg_ConfigPtr->NotificationCallback != NULL_PTR) &&
            (Wdg_ConfigPtr->InterruptEnable == TRUE))
        {
            Wdg_ConfigPtr->NotificationCallback();
        }
    }
}

static boolean Wdg_IsTimeoutValid(const Wdg_ConfigType* cfg, uint32 timeoutTicks)
{
    return (timeoutTicks >= cfg->MinTimeoutTicks) && (timeoutTicks <= cfg->MaxTimeoutTicks);
}

/* ===================[Public API]=================== */
void Wdg_Init(const Wdg_ConfigType* ConfigPtr)
{
#if (WDG_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(WDG_MODULE_ID, WDG_INSTANCE_ID, WDG_INIT_SID, WDG_E_PARAM_POINTER);
        return;
    }

    if (Wdg_ModuleStatus != WDG_STATUS_UNINIT)
    {
        Det_ReportError(WDG_MODULE_ID, WDG_INSTANCE_ID, WDG_INIT_SID, WDG_E_ALREADY_INITIALIZED);
        return;
    }

    if (!Wdg_IsInstanceValid(ConfigPtr->Instance))
    {
        Det_ReportError(WDG_MODULE_ID, WDG_INSTANCE_ID, WDG_INIT_SID, WDG_E_PARAM_INSTANCE);
        return;
    }

    if (!Wdg_IsTimeoutValid(ConfigPtr, ConfigPtr->InitialTimeoutTicks))
    {
        Det_ReportError(WDG_MODULE_ID, WDG_INSTANCE_ID, WDG_INIT_SID, WDG_E_PARAM_TIMEOUT);
        return;
    }
#endif

    uint32 baseAddr = Wdg_GetBaseAddress(ConfigPtr->Instance);

    Wdg_EnableClock(ConfigPtr->Instance);

    Wdg_Unlock(baseAddr);
    Wdg_WaitForWriteComplete(baseAddr);

    Wdg_SetLoadRegister(baseAddr, ConfigPtr->InitialTimeoutTicks);
    Wdg_ConfigureControl(baseAddr, ConfigPtr);

#if (WDG_ALLOW_DEBUG_STALL == STD_ON)
    uint32 testValue = WDG_REG(baseAddr, WDG_TEST_OFFSET);
    testValue |= WDT_TEST_STALL;
    WDG_REG(baseAddr, WDG_TEST_OFFSET) = testValue;
    Wdg_WaitForWriteComplete(baseAddr);
#endif

    Wdg_Lock(baseAddr);

    Wdg_ConfigPtr = ConfigPtr;
    Wdg_ModuleStatus = WDG_STATUS_RUNNING;
}

void Wdg_SetTriggerMode(Wdg_TriggerModeType Mode)
{
#if (WDG_DEV_ERROR_DETECT == STD_ON)
    if (Wdg_ModuleStatus == WDG_STATUS_UNINIT)
    {
        Det_ReportError(WDG_MODULE_ID, WDG_INSTANCE_ID, WDG_SET_MODE_SID, WDG_E_UNINIT);
        return;
    }

    if ((Mode != WDG_TRIGGER_MODE_NORMAL) && (Mode != WDG_TRIGGER_MODE_FAST))
    {
        Det_ReportError(WDG_MODULE_ID, WDG_INSTANCE_ID, WDG_SET_MODE_SID, WDG_E_PARAM_MODE);
        return;
    }
#endif

    if (Mode == WDG_TRIGGER_MODE_FAST)
    {
        Wdg_SetTriggerCondition(Wdg_ConfigPtr->MinTimeoutTicks);
    }
    else
    {
        Wdg_SetTriggerCondition(Wdg_ConfigPtr->InitialTimeoutTicks);
    }
}

void Wdg_SetTriggerCondition(uint32 TimeoutTicks)
{
#if (WDG_DEV_ERROR_DETECT == STD_ON)
    if (Wdg_ModuleStatus == WDG_STATUS_UNINIT)
    {
        Det_ReportError(WDG_MODULE_ID, WDG_INSTANCE_ID, WDG_SET_TRIGGER_COND_SID, WDG_E_UNINIT);
        return;
    }

    if (!Wdg_IsTimeoutValid(Wdg_ConfigPtr, TimeoutTicks))
    {
        Det_ReportError(WDG_MODULE_ID, WDG_INSTANCE_ID, WDG_SET_TRIGGER_COND_SID, WDG_E_PARAM_TIMEOUT);
        return;
    }
#endif

    uint32 baseAddr = Wdg_GetBaseAddress(Wdg_ConfigPtr->Instance);

    Wdg_Unlock(baseAddr);
    Wdg_WaitForWriteComplete(baseAddr);

    Wdg_SetLoadRegister(baseAddr, TimeoutTicks);

    Wdg_Lock(baseAddr);

    Wdg_ModuleStatus = WDG_STATUS_RUNNING;
}

void Wdg_Service(void)
{
#if (WDG_DEV_ERROR_DETECT == STD_ON)
    if (Wdg_ModuleStatus == WDG_STATUS_UNINIT)
    {
        Det_ReportError(WDG_MODULE_ID, WDG_INSTANCE_ID, WDG_SERVICE_SID, WDG_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Wdg_GetBaseAddress(Wdg_ConfigPtr->Instance);
    WDG_REG(baseAddr, WDG_ICR_OFFSET) = WDT_ICR_M;
}

void Wdg_Disable(void)
{
#if (WDG_DEV_ERROR_DETECT == STD_ON)
    if (Wdg_ModuleStatus == WDG_STATUS_UNINIT)
    {
        Det_ReportError(WDG_MODULE_ID, WDG_INSTANCE_ID, WDG_DISABLE_SID, WDG_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Wdg_GetBaseAddress(Wdg_ConfigPtr->Instance);

    Wdg_Unlock(baseAddr);
    Wdg_WaitForWriteComplete(baseAddr);

    uint32 ctl = WDG_REG(baseAddr, WDG_CTL_OFFSET);
    ctl &= ~(WDT_CTL_INTEN | WDT_CTL_RESEN);
    WDG_REG(baseAddr, WDG_CTL_OFFSET) = ctl;
    Wdg_WaitForWriteComplete(baseAddr);

    Wdg_Lock(baseAddr);

    Wdg_ModuleStatus = WDG_STATUS_IDLE;
}

Wdg_StatusType Wdg_GetStatus(void)
{
    return Wdg_ModuleStatus;
}

#if (WDG_VERSION_INFO_API == STD_ON)
void Wdg_GetVersionInfo(Std_VersionInfoType* versionInfoPtr)
{
#if (WDG_DEV_ERROR_DETECT == STD_ON)
    if (versionInfoPtr == NULL_PTR)
    {
        Det_ReportError(WDG_MODULE_ID, WDG_INSTANCE_ID, WDG_GET_VERSION_INFO_SID, WDG_E_PARAM_POINTER);
        return;
    }
#endif

    versionInfoPtr->vendorID = WDG_VENDOR_ID;
    versionInfoPtr->moduleID = WDG_MODULE_ID;
    versionInfoPtr->sw_major_version = WDG_SW_MAJOR_VERSION;
    versionInfoPtr->sw_minor_version = WDG_SW_MINOR_VERSION;
    versionInfoPtr->sw_patch_version = WDG_SW_PATCH_VERSION;
}
#endif

void Wdg_Watchdog0Handler(void)
{
    Wdg_HandleInterrupt(WDG_INSTANCE_0);
}

void Wdg_Watchdog1Handler(void)
{
    Wdg_HandleInterrupt(WDG_INSTANCE_1);
}
