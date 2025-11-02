/**
 * @file QEI.c
 * @brief QEI Driver Implementation for TM4C123GH6PM
 */

#include "QEI.h"
#include "tm4c123gh6pm.h"

/* ===================[Register Definitions]=================== */
#define QEI0_BASE_ADDR                 (0x4002C000u)
#define QEI1_BASE_ADDR                 (0x4002D000u)

#define QEI_CTL_OFFSET                 (0x000u)
#define QEI_STAT_OFFSET                (0x004u)
#define QEI_POS_OFFSET                 (0x008u)
#define QEI_MAXPOS_OFFSET              (0x00Cu)
#define QEI_LOAD_OFFSET                (0x010u)
#define QEI_TIME_OFFSET                (0x014u)
#define QEI_COUNT_OFFSET               (0x018u)
#define QEI_SPEED_OFFSET               (0x01Cu)
#define QEI_INTEN_OFFSET               (0x020u)
#define QEI_RIS_OFFSET                 (0x024u)
#define QEI_ISC_OFFSET                 (0x028u)

#define QEI_REG(base, offset)          (*((volatile uint32*)(((uint32)(base)) + ((uint32)(offset)))))

#define QEI_CLOCK_BIT(module)          (1u << (uint32)(module))

/* ===================[Private Variables]=================== */
static const Qei_ConfigType* Qei_ConfigPtr = NULL_PTR;
static Qei_StatusType Qei_ModuleStatus = QEI_STATUS_UNINIT;

/* ===================[Private Helper Functions]=================== */
static boolean Qei_IsModuleValid(Qei_ModuleType module)
{
    return (module == QEI_MODULE_0) || (module == QEI_MODULE_1);
}

static uint32 Qei_GetBaseAddress(Qei_ModuleType module)
{
    return (module == QEI_MODULE_0) ? QEI0_BASE_ADDR : QEI1_BASE_ADDR;
}

static void Qei_EnableClock(Qei_ModuleType module)
{
    SYSCTL_RCGCQEI_R |= QEI_CLOCK_BIT(module);
    (void)SYSCTL_RCGCQEI_R; /* Dummy read to ensure clock is enabled */
}

static void Qei_DisableModule(uint32 baseAddr)
{
    QEI_REG(baseAddr, QEI_CTL_OFFSET) &= ~QEI_CTL_ENABLE;
}

static void Qei_EnableModule(uint32 baseAddr)
{
    QEI_REG(baseAddr, QEI_CTL_OFFSET) |= QEI_CTL_ENABLE;
}

static void Qei_ConfigureControl(uint32 baseAddr, const Qei_ConfigType* cfg)
{
    uint32 controlValue = 0u;

    if (cfg->SwapChannels == TRUE)
    {
        controlValue |= QEI_CTL_SWAP;
    }

    if (cfg->SignalMode == QEI_SIGNAL_MODE_DIRECTION)
    {
        controlValue |= QEI_CTL_SIGMODE;
    }

    if (cfg->ResetMode == QEI_RESET_MODE_ON_INDEX)
    {
        controlValue |= QEI_CTL_RESMODE;
    }

    if (cfg->EnableVelocityCapture == TRUE)
    {
        controlValue |= QEI_CTL_VELEN;

        switch (cfg->VelocityPreDiv)
        {
            case QEI_VELOCITY_PREDIV_1:   controlValue |= QEI_CTL_VELDIV_1; break;
            case QEI_VELOCITY_PREDIV_2:   controlValue |= QEI_CTL_VELDIV_2; break;
            case QEI_VELOCITY_PREDIV_4:   controlValue |= QEI_CTL_VELDIV_4; break;
            case QEI_VELOCITY_PREDIV_8:   controlValue |= QEI_CTL_VELDIV_8; break;
            case QEI_VELOCITY_PREDIV_16:  controlValue |= QEI_CTL_VELDIV_16; break;
            case QEI_VELOCITY_PREDIV_32:  controlValue |= QEI_CTL_VELDIV_32; break;
            case QEI_VELOCITY_PREDIV_64:  controlValue |= QEI_CTL_VELDIV_64; break;
            case QEI_VELOCITY_PREDIV_128: controlValue |= QEI_CTL_VELDIV_128; break;
            default:
                /* Default handled by AUTOSAR DET check before */
                break;
        }
    }

    if (cfg->SignalMode == QEI_SIGNAL_MODE_QUADRATURE)
    {
        controlValue |= QEI_CTL_CAPMODE; /* Capture both PhA/PhB edges */
    }

    if (cfg->InvertChannelA == TRUE)
    {
        controlValue |= QEI_CTL_INVA;
    }

    if (cfg->InvertChannelB == TRUE)
    {
        controlValue |= QEI_CTL_INVB;
    }

    if (cfg->InvertIndex == TRUE)
    {
        controlValue |= QEI_CTL_INVI;
    }

    if (cfg->EnableFilter == TRUE)
    {
        controlValue |= QEI_CTL_FILTEN;
        uint32 filter = ((uint32)cfg->FilterCount & 0xFu) << QEI_CTL_FILTCNT_S;
        controlValue |= filter;
    }

#if (QEI_ALLOW_DEBUG_STALL == STD_ON)
    if (cfg->DebugStallEnable == TRUE)
    {
        controlValue |= QEI_CTL_STALLEN;
    }
#endif

    QEI_REG(baseAddr, QEI_CTL_OFFSET) = controlValue;
}

static void Qei_ConfigureRegisters(uint32 baseAddr, const Qei_ConfigType* cfg)
{
    QEI_REG(baseAddr, QEI_MAXPOS_OFFSET) = cfg->MaxPosition;
    QEI_REG(baseAddr, QEI_POS_OFFSET) = cfg->InitialPosition;

    if (cfg->EnableVelocityCapture == TRUE)
    {
        QEI_REG(baseAddr, QEI_LOAD_OFFSET) = cfg->VelocityTimerLoad;
    }
    else
    {
        QEI_REG(baseAddr, QEI_LOAD_OFFSET) = cfg->VelocityTimerLoad;
    }

    QEI_REG(baseAddr, QEI_INTEN_OFFSET) = cfg->InterruptMask;
    QEI_REG(baseAddr, QEI_ISC_OFFSET) = 0xFFu; /* Clear any pending interrupts */
}

static void Qei_HandleInterrupt(Qei_ModuleType module)
{
    if ((Qei_ConfigPtr == NULL_PTR) || (Qei_ConfigPtr->Module != module))
    {
        return;
    }

    uint32 baseAddr = Qei_GetBaseAddress(module);
    uint32 flags = QEI_REG(baseAddr, QEI_RIS_OFFSET) & Qei_ConfigPtr->InterruptMask;

    if (flags != 0u)
    {
        QEI_REG(baseAddr, QEI_ISC_OFFSET) = flags;

        if (Qei_ConfigPtr->NotificationCallback != NULL_PTR)
        {
            Qei_ConfigPtr->NotificationCallback(flags);
        }
    }
}

static boolean Qei_IsPositionValid(const Qei_ConfigType* cfg, uint32 position)
{
    return (position <= cfg->MaxPosition);
}

/* ===================[Public API]=================== */
void Qei_Init(const Qei_ConfigType* ConfigPtr)
{
#if (QEI_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_INIT_SID, QEI_E_PARAM_POINTER);
        return;
    }

    if (Qei_ModuleStatus != QEI_STATUS_UNINIT)
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_INIT_SID, QEI_E_ALREADY_INITIALIZED);
        return;
    }

    if (!Qei_IsModuleValid(ConfigPtr->Module))
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_INIT_SID, QEI_E_PARAM_MODULE);
        return;
    }

    if (!Qei_IsPositionValid(ConfigPtr, ConfigPtr->InitialPosition))
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_INIT_SID, QEI_E_PARAM_POSITION);
        return;
    }
#endif

    uint32 baseAddr = Qei_GetBaseAddress(ConfigPtr->Module);

    Qei_EnableClock(ConfigPtr->Module);

    Qei_DisableModule(baseAddr);

    Qei_ConfigureControl(baseAddr, ConfigPtr);
    Qei_ConfigureRegisters(baseAddr, ConfigPtr);

    Qei_EnableModule(baseAddr);

    Qei_ConfigPtr = ConfigPtr;
    Qei_ModuleStatus = QEI_STATUS_RUNNING;
}

void Qei_DeInit(void)
{
#if (QEI_DEV_ERROR_DETECT == STD_ON)
    if (Qei_ModuleStatus == QEI_STATUS_UNINIT)
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_DEINIT_SID, QEI_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Qei_GetBaseAddress(Qei_ConfigPtr->Module);

    Qei_DisableModule(baseAddr);
    QEI_REG(baseAddr, QEI_INTEN_OFFSET) = 0u;
    QEI_REG(baseAddr, QEI_ISC_OFFSET) = 0xFFu;

    Qei_ConfigPtr = NULL_PTR;
    Qei_ModuleStatus = QEI_STATUS_UNINIT;
}

uint32 Qei_GetPosition(void)
{
#if (QEI_DEV_ERROR_DETECT == STD_ON)
    if (Qei_ModuleStatus == QEI_STATUS_UNINIT)
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_GET_POSITION_SID, QEI_E_UNINIT);
        return 0u;
    }
#endif

    uint32 baseAddr = Qei_GetBaseAddress(Qei_ConfigPtr->Module);
    return QEI_REG(baseAddr, QEI_POS_OFFSET);
}

void Qei_SetPosition(uint32 Position)
{
#if (QEI_DEV_ERROR_DETECT == STD_ON)
    if (Qei_ModuleStatus == QEI_STATUS_UNINIT)
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_SET_POSITION_SID, QEI_E_UNINIT);
        return;
    }

    if (!Qei_IsPositionValid(Qei_ConfigPtr, Position))
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_SET_POSITION_SID, QEI_E_PARAM_POSITION);
        return;
    }
#endif

    uint32 baseAddr = Qei_GetBaseAddress(Qei_ConfigPtr->Module);
    QEI_REG(baseAddr, QEI_POS_OFFSET) = Position;
}

uint32 Qei_GetVelocity(void)
{
#if (QEI_DEV_ERROR_DETECT == STD_ON)
    if (Qei_ModuleStatus == QEI_STATUS_UNINIT)
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_GET_VELOCITY_SID, QEI_E_UNINIT);
        return 0u;
    }

    if (Qei_ConfigPtr->EnableVelocityCapture == FALSE)
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_GET_VELOCITY_SID, QEI_E_PARAM_VELOCITY);
        return 0u;
    }
#endif

    uint32 baseAddr = Qei_GetBaseAddress(Qei_ConfigPtr->Module);
    return QEI_REG(baseAddr, QEI_SPEED_OFFSET);
}

void Qei_EnableInterrupt(Qei_InterruptMaskType mask)
{
#if (QEI_DEV_ERROR_DETECT == STD_ON)
    if (Qei_ModuleStatus == QEI_STATUS_UNINIT)
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_ENABLE_INTERRUPT_SID, QEI_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Qei_GetBaseAddress(Qei_ConfigPtr->Module);
    QEI_REG(baseAddr, QEI_INTEN_OFFSET) |= mask;
}

void Qei_DisableInterrupt(Qei_InterruptMaskType mask)
{
#if (QEI_DEV_ERROR_DETECT == STD_ON)
    if (Qei_ModuleStatus == QEI_STATUS_UNINIT)
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_DISABLE_INTERRUPT_SID, QEI_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Qei_GetBaseAddress(Qei_ConfigPtr->Module);
    QEI_REG(baseAddr, QEI_INTEN_OFFSET) &= ~mask;
}

void Qei_ClearInterrupt(Qei_InterruptMaskType mask)
{
#if (QEI_DEV_ERROR_DETECT == STD_ON)
    if (Qei_ModuleStatus == QEI_STATUS_UNINIT)
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_CLEAR_INTERRUPT_SID, QEI_E_UNINIT);
        return;
    }
#endif

    uint32 baseAddr = Qei_GetBaseAddress(Qei_ConfigPtr->Module);
    QEI_REG(baseAddr, QEI_ISC_OFFSET) = mask;
}

Qei_StatusType Qei_GetStatus(void)
{
    return Qei_ModuleStatus;
}

Qei_DirectionType Qei_GetDirection(void)
{
#if (QEI_DEV_ERROR_DETECT == STD_ON)
    if (Qei_ModuleStatus == QEI_STATUS_UNINIT)
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_GET_DIRECTION_SID, QEI_E_UNINIT);
        return QEI_DIRECTION_FORWARD;
    }
#endif

    uint32 baseAddr = Qei_GetBaseAddress(Qei_ConfigPtr->Module);
    uint32 stat = QEI_REG(baseAddr, QEI_STAT_OFFSET);
    return ((stat & QEI_STAT_DIRECTION) != 0u) ? QEI_DIRECTION_REVERSE : QEI_DIRECTION_FORWARD;
}

#if (QEI_VERSION_INFO_API == STD_ON)
void Qei_GetVersionInfo(Std_VersionInfoType* versionInfoPtr)
{
#if (QEI_DEV_ERROR_DETECT == STD_ON)
    if (versionInfoPtr == NULL_PTR)
    {
        Det_ReportError(QEI_MODULE_ID, QEI_INSTANCE_ID, QEI_GET_VERSION_INFO_SID, QEI_E_PARAM_POINTER);
        return;
    }
#endif

    versionInfoPtr->vendorID = QEI_VENDOR_ID;
    versionInfoPtr->moduleID = QEI_MODULE_ID;
    versionInfoPtr->sw_major_version = QEI_SW_MAJOR_VERSION;
    versionInfoPtr->sw_minor_version = QEI_SW_MINOR_VERSION;
    versionInfoPtr->sw_patch_version = QEI_SW_PATCH_VERSION;
}
#endif

void Qei_Qei0Handler(void)
{
    Qei_HandleInterrupt(QEI_MODULE_0);
}

void Qei_Qei1Handler(void)
{
    Qei_HandleInterrupt(QEI_MODULE_1);
}

