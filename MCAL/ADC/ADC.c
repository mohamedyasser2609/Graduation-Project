#include "Adc.h"
#include "Adc_Regs.h"
#include "../MCU/MCU.h"
#include "../NVIC/NVIC.h"

typedef struct {
    Adc_StatusType Status;
    Adc_ValueGroupType* ResultBuffer;
    uint16 ResultLength;
    uint16 StreamIndex;
    boolean NotificationEnabled;
} Adc_GroupRuntimeType;

static const Adc_ConfigType* Adc_ConfigPtr = NULL_PTR;
static boolean Adc_Initialized = FALSE;
static Adc_GroupRuntimeType Adc_GroupRuntime[ADC_CONFIGURED_GROUPS];

static uint32 Adc_GetUnitBase(Adc_HwUnitType Unit)
{
    return (Unit == ADC_HW_UNIT_0) ? ADC0_BASE_ADDRESS : ADC1_BASE_ADDRESS;
}

static uint32 Adc_GetUnitClockMask(Adc_HwUnitType Unit)
{
    return (Unit == ADC_HW_UNIT_0) ? SYSCTL_RCGCADC_R0 : SYSCTL_RCGCADC_R1;
}

static uint32 Adc_GetUnitReadyMask(Adc_HwUnitType Unit)
{
    return (Unit == ADC_HW_UNIT_0) ? SYSCTL_PRADC_R0 : SYSCTL_PRADC_R1;
}

static uint8 Adc_GetSequencerMaxSteps(Adc_SequencerType Sequencer)
{
    switch (Sequencer) {
        case ADC_SEQUENCER_0: return 8u;
        case ADC_SEQUENCER_1: return 4u;
        case ADC_SEQUENCER_2: return 4u;
        case ADC_SEQUENCER_3: return 1u;
        default: return 0u;
    }
}

static void Adc_EnableUnitClock(Adc_HwUnitType Unit)
{
    SYSCTL_RCGCADC_R |= Adc_GetUnitClockMask(Unit);
    while ((SYSCTL_PRADC_R & Adc_GetUnitReadyMask(Unit)) == 0u) {
    }
}

static boolean Adc_IsGroupValid(Adc_GroupType Group)
{
    if (Adc_ConfigPtr == NULL_PTR) {
        return FALSE;
    }
    return (Group < (Adc_GroupType)Adc_ConfigPtr->GroupCount);
}

static const Adc_GroupConfigType* Adc_GetGroupCfg(Adc_GroupType Group)
{
    return &Adc_ConfigPtr->Groups[Group];
}

static void Adc_SetEmuxTrigger(uint32 base, Adc_SequencerType Sequencer, Adc_TriggerSourceType Trigger)
{
    uint32 shift = ((uint32)Sequencer * 4u);
    uint32 value = ADC_REG(base, ADC_EMUX_OFFSET);
    value &= ~(0xFu << shift);
    value |= (((uint32)Trigger & 0xFu) << shift);
    ADC_REG(base, ADC_EMUX_OFFSET) = value;
}

static void Adc_SetSequencerPriority(uint32 base, Adc_SequencerType Sequencer, uint8 Priority)
{
    uint32 shift = ((uint32)Sequencer * 2u);
    uint32 value = ADC_REG(base, ADC_SSPRI_OFFSET);
    value &= ~(0x3u << shift);
    value |= (((uint32)Priority & 0x3u) << shift);
    ADC_REG(base, ADC_SSPRI_OFFSET) = value;
}

static void Adc_ConfigureSequencer(const Adc_GroupConfigType* GroupCfg)
{
    uint32 base = Adc_GetUnitBase(GroupCfg->Unit);
    uint32 ssBit = (1u << (uint32)GroupCfg->Sequencer);

    uint8 maxSteps = Adc_GetSequencerMaxSteps(GroupCfg->Sequencer);
    uint8 chCount = GroupCfg->ChannelCount;
    if ((chCount == 0u) || (chCount > maxSteps)) {
        return;
    }

    ADC_REG(base, ADC_ACTSS_OFFSET) &= ~ssBit;

    Adc_SetSequencerPriority(base, GroupCfg->Sequencer, GroupCfg->Priority);
    Adc_SetEmuxTrigger(base, GroupCfg->Sequencer, GroupCfg->TriggerSource);

    ADC_REG(base, ADC_SAC_OFFSET) = (uint32)GroupCfg->SampleAveraging;

    uint32 muxOffset = ADC_SSMUX0_OFFSET + ((uint32)GroupCfg->Sequencer * ADC_SSMUX_STRIDE);
    uint32 ctlOffset = ADC_SSCTL0_OFFSET + ((uint32)GroupCfg->Sequencer * ADC_SSMUX_STRIDE);

    uint32 mux = 0u;
    uint32 ctl = 0u;
    uint8 i;

    for (i = 0u; i < chCount; i++) {
        Adc_ChannelType ch = GroupCfg->Channels[i];
        uint32 stepShift = ((uint32)i * 4u);

        if (ch == ADC_CHANNEL_TEMP) {
            ctl |= (1u << (stepShift + 3u));
        } else if (ch <= 15u) {
            mux |= ((uint32)ch & 0xFu) << stepShift;
        } else {
            return;
        }

        if (i == (uint8)(chCount - 1u)) {
            ctl |= (1u << (stepShift + 1u));
            ctl |= (1u << (stepShift + 2u));
        }
    }

    ADC_REG(base, muxOffset) = mux;
    ADC_REG(base, ctlOffset) = ctl;

    ADC_REG(base, ADC_ISC_OFFSET) = ssBit;
    ADC_REG(base, ADC_IM_OFFSET) &= ~ssBit;

    ADC_REG(base, ADC_ACTSS_OFFSET) |= ssBit;
}

static void Adc_ReadFifoToBuffer(Adc_GroupType Group, const Adc_GroupConfigType* GroupCfg)
{
    uint32 base = Adc_GetUnitBase(GroupCfg->Unit);
    uint32 fifoOffset = ADC_SSFIFO0_OFFSET + ((uint32)GroupCfg->Sequencer * ADC_SSMUX_STRIDE);

    Adc_GroupRuntimeType* rt = &Adc_GroupRuntime[Group];
    uint16 channelCount = (uint16)GroupCfg->ChannelCount;

    if (rt->ResultBuffer == NULL_PTR) {
        return;
    }

    uint16 writeIndex = 0u;
    if (GroupCfg->AccessMode == ADC_ACCESS_MODE_STREAMING) {
        uint16 sampleIndex = rt->StreamIndex;
        uint16 totalSamples = (GroupCfg->StreamNumSamples == 0u) ? 1u : GroupCfg->StreamNumSamples;
        if (GroupCfg->StreamBufferMode == ADC_STREAM_BUFFER_CIRCULAR) {
            sampleIndex = (uint16)(sampleIndex % totalSamples);
        } else if (sampleIndex >= totalSamples) {
            sampleIndex = (uint16)(totalSamples - 1u);
        }
        writeIndex = (uint16)(sampleIndex * channelCount);
    }

    uint16 i;
    for (i = 0u; i < channelCount; i++) {
        uint32 raw = ADC_REG(base, fifoOffset);
        rt->ResultBuffer[writeIndex + i] = (Adc_ValueGroupType)(raw & ADC_FIFO_DATA_MASK);
    }
}

static void Adc_HandleGroupCompletion(Adc_GroupType Group)
{
    const Adc_GroupConfigType* GroupCfg = Adc_GetGroupCfg(Group);
    uint32 base = Adc_GetUnitBase(GroupCfg->Unit);
    uint32 ssBit = (1u << (uint32)GroupCfg->Sequencer);

    if ((ADC_REG(base, ADC_RIS_OFFSET) & ssBit) == 0u) {
        return;
    }

    /* Clear interrupt flag */
    ADC_REG(base, ADC_ISC_OFFSET) = ssBit;

    Adc_ReadFifoToBuffer(Group, GroupCfg);

    Adc_GroupRuntimeType* rt = &Adc_GroupRuntime[Group];
    if (GroupCfg->AccessMode == ADC_ACCESS_MODE_STREAMING) {
        rt->StreamIndex++;
        if ((GroupCfg->StreamNumSamples != 0u) && (rt->StreamIndex >= GroupCfg->StreamNumSamples)) {
            rt->Status = ADC_STREAM_COMPLETED;
            if (GroupCfg->ConvMode == ADC_CONV_MODE_ONESHOT) {
                ADC_REG(base, ADC_ACTSS_OFFSET) &= ~ssBit;
            }
        } else {
            rt->Status = ADC_BUSY;
            if (GroupCfg->ConvMode == ADC_CONV_MODE_ONESHOT) {
                ADC_REG(base, ADC_PSSI_OFFSET) = ssBit;
            }
        }
    } else {
        rt->Status = ADC_COMPLETED;
        if (GroupCfg->ConvMode == ADC_CONV_MODE_CONTINUOUS) {
            ADC_REG(base, ADC_PSSI_OFFSET) = ssBit;
            rt->Status = ADC_BUSY;
        } else {
            ADC_REG(base, ADC_ACTSS_OFFSET) &= ~ssBit;
        }
    }

    if ((rt->NotificationEnabled == TRUE) && (GroupCfg->Notification != NULL_PTR)) {
        GroupCfg->Notification(Group);
    }
}

void Adc_Init(const Adc_ConfigType* ConfigPtr)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_INIT_SID, ADC_E_PARAM_POINTER);
        return;
    }
    if (Adc_Initialized == TRUE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_INIT_SID, ADC_E_ALREADY_INITIALIZED);
        return;
    }
    if ((ConfigPtr->Groups == NULL_PTR) || (ConfigPtr->GroupCount == 0u) || (ConfigPtr->GroupCount > ADC_CONFIGURED_GROUPS)) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_INIT_SID, ADC_E_PARAM_CONFIG);
        return;
    }
#endif

    Adc_ConfigPtr = ConfigPtr;

    uint16 i;
    for (i = 0u; i < ADC_CONFIGURED_GROUPS; i++) {
        Adc_GroupRuntime[i].Status = ADC_IDLE;
        Adc_GroupRuntime[i].ResultBuffer = NULL_PTR;
        Adc_GroupRuntime[i].ResultLength = 0u;
        Adc_GroupRuntime[i].StreamIndex = 0u;
        Adc_GroupRuntime[i].NotificationEnabled = FALSE;
    }

    for (i = 0u; i < ConfigPtr->GroupCount; i++) {
        const Adc_GroupConfigType* grp = &ConfigPtr->Groups[i];
        Adc_EnableUnitClock(grp->Unit);
        Adc_ConfigureSequencer(grp);
        
        /* Enable interrupt in NVIC */
        NVIC_InterruptVectorType irq;
        if (grp->Unit == ADC_HW_UNIT_0) {
            irq = 14u + (uint8)grp->Sequencer;
        } else {
            irq = 48u + (uint8)grp->Sequencer;
        }
        NVIC_EnableIRQ(irq);
        NVIC_SetPriority(irq, 3u); /* Default priority */
    }

    Adc_Initialized = TRUE;
}

void Adc_DeInit(void)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (Adc_Initialized == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_DEINIT_SID, ADC_E_UNINIT);
        return;
    }
#endif

    uint16 i;
    for (i = 0u; i < Adc_ConfigPtr->GroupCount; i++) {
        const Adc_GroupConfigType* grp = Adc_GetGroupCfg((Adc_GroupType)i);
        uint32 base = Adc_GetUnitBase(grp->Unit);
        uint32 ssBit = (1u << (uint32)grp->Sequencer);
        ADC_REG(base, ADC_ACTSS_OFFSET) &= ~ssBit;
        ADC_REG(base, ADC_IM_OFFSET) &= ~ssBit;
        ADC_REG(base, ADC_ISC_OFFSET) = ssBit;
    }

    Adc_ConfigPtr = NULL_PTR;
    Adc_Initialized = FALSE;

    for (i = 0u; i < ADC_CONFIGURED_GROUPS; i++) {
        Adc_GroupRuntime[i].Status = ADC_IDLE;
        Adc_GroupRuntime[i].ResultBuffer = NULL_PTR;
        Adc_GroupRuntime[i].ResultLength = 0u;
        Adc_GroupRuntime[i].StreamIndex = 0u;
        Adc_GroupRuntime[i].NotificationEnabled = FALSE;
    }
}

Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (Adc_Initialized == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_SETUP_RESULT_BUFFER_SID, ADC_E_UNINIT);
        return E_NOT_OK;
    }
    if (DataBufferPtr == NULL_PTR) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_SETUP_RESULT_BUFFER_SID, ADC_E_PARAM_POINTER);
        return E_NOT_OK;
    }
    if (Adc_IsGroupValid(Group) == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_SETUP_RESULT_BUFFER_SID, ADC_E_PARAM_GROUP);
        return E_NOT_OK;
    }
#endif

    const Adc_GroupConfigType* cfg = Adc_GetGroupCfg(Group);
    uint16 len = (uint16)cfg->ChannelCount;
    if (cfg->AccessMode == ADC_ACCESS_MODE_STREAMING) {
        uint16 samples = (cfg->StreamNumSamples == 0u) ? 1u : cfg->StreamNumSamples;
        len = (uint16)(len * samples);
    }

    Adc_GroupRuntime[Group].ResultBuffer = DataBufferPtr;
    Adc_GroupRuntime[Group].ResultLength = len;
    Adc_GroupRuntime[Group].StreamIndex = 0u;
    Adc_GroupRuntime[Group].Status = ADC_IDLE;
    return E_OK;
}

void Adc_StartGroupConversion(Adc_GroupType Group)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (Adc_Initialized == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_START_GROUP_CONVERSION_SID, ADC_E_UNINIT);
        return;
    }
    if (Adc_IsGroupValid(Group) == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_START_GROUP_CONVERSION_SID, ADC_E_PARAM_GROUP);
        return;
    }
    if (Adc_GroupRuntime[Group].ResultBuffer == NULL_PTR) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_START_GROUP_CONVERSION_SID, ADC_E_BUFFER_UNINIT);
        return;
    }
    if (Adc_GroupRuntime[Group].Status == ADC_BUSY) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_START_GROUP_CONVERSION_SID, ADC_E_BUSY);
        return;
    }
#endif

    const Adc_GroupConfigType* cfg = Adc_GetGroupCfg(Group);
    uint32 base = Adc_GetUnitBase(cfg->Unit);
    uint32 ssBit = (1u << (uint32)cfg->Sequencer);

    Adc_GroupRuntime[Group].StreamIndex = 0u;
    Adc_GroupRuntime[Group].Status = ADC_BUSY;

    Adc_ConfigureSequencer(cfg);

    ADC_REG(base, ADC_ISC_OFFSET) = ssBit;

    if (cfg->TriggerSource == ADC_TRIGG_SRC_PROCESSOR) {
        ADC_REG(base, ADC_PSSI_OFFSET) = ssBit;
    }
}

void Adc_StopGroupConversion(Adc_GroupType Group)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (Adc_Initialized == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_STOP_GROUP_CONVERSION_SID, ADC_E_UNINIT);
        return;
    }
    if (Adc_IsGroupValid(Group) == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_STOP_GROUP_CONVERSION_SID, ADC_E_PARAM_GROUP);
        return;
    }
#endif

    const Adc_GroupConfigType* cfg = Adc_GetGroupCfg(Group);
    uint32 base = Adc_GetUnitBase(cfg->Unit);
    uint32 ssBit = (1u << (uint32)cfg->Sequencer);

    ADC_REG(base, ADC_ACTSS_OFFSET) &= ~ssBit;
    ADC_REG(base, ADC_IM_OFFSET) &= ~ssBit;
    ADC_REG(base, ADC_ISC_OFFSET) = ssBit;

    Adc_GroupRuntime[Group].Status = ADC_IDLE;
}

Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (Adc_Initialized == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_READ_GROUP_SID, ADC_E_UNINIT);
        return E_NOT_OK;
    }
    if (Adc_IsGroupValid(Group) == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_READ_GROUP_SID, ADC_E_PARAM_GROUP);
        return E_NOT_OK;
    }
    if (DataBufferPtr == NULL_PTR) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_READ_GROUP_SID, ADC_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif

    if ((Adc_GroupRuntime[Group].Status == ADC_BUSY) || (Adc_GroupRuntime[Group].ResultBuffer == NULL_PTR)) {
        return E_NOT_OK;
    }

    uint16 len = Adc_GroupRuntime[Group].ResultLength;
    uint16 i;
    for (i = 0u; i < len; i++) {
        DataBufferPtr[i] = Adc_GroupRuntime[Group].ResultBuffer[i];
    }

    return E_OK;
}

Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (Adc_Initialized == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_GET_GROUP_STATUS_SID, ADC_E_UNINIT);
        return ADC_IDLE;
    }
    if (Adc_IsGroupValid(Group) == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_GET_GROUP_STATUS_SID, ADC_E_PARAM_GROUP);
        return ADC_IDLE;
    }
#endif

    return Adc_GroupRuntime[Group].Status;
}

void Adc_EnableHardwareTrigger(Adc_GroupType Group)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (Adc_Initialized == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_ENABLE_HW_TRIGGER_SID, ADC_E_UNINIT);
        return;
    }
    if (Adc_IsGroupValid(Group) == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_ENABLE_HW_TRIGGER_SID, ADC_E_PARAM_GROUP);
        return;
    }
#endif

    const Adc_GroupConfigType* cfg = Adc_GetGroupCfg(Group);
    uint32 base = Adc_GetUnitBase(cfg->Unit);
    if (cfg->TriggerSource == ADC_TRIGG_SRC_PROCESSOR) {
#if (ADC_DEV_ERROR_DETECT == STD_ON)
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_ENABLE_HW_TRIGGER_SID, ADC_E_PARAM_TRIGGER);
#endif
        return;
    }

    Adc_SetEmuxTrigger(base, cfg->Sequencer, cfg->TriggerSource);
}

void Adc_DisableHardwareTrigger(Adc_GroupType Group)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (Adc_Initialized == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_DISABLE_HW_TRIGGER_SID, ADC_E_UNINIT);
        return;
    }
    if (Adc_IsGroupValid(Group) == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_DISABLE_HW_TRIGGER_SID, ADC_E_PARAM_GROUP);
        return;
    }
#endif

    const Adc_GroupConfigType* cfg = Adc_GetGroupCfg(Group);
    uint32 base = Adc_GetUnitBase(cfg->Unit);
    Adc_SetEmuxTrigger(base, cfg->Sequencer, ADC_TRIGG_SRC_PROCESSOR);
}

void Adc_EnableGroupNotification(Adc_GroupType Group)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (Adc_Initialized == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_ENABLE_NOTIFICATION_SID, ADC_E_UNINIT);
        return;
    }
    if (Adc_IsGroupValid(Group) == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_ENABLE_NOTIFICATION_SID, ADC_E_PARAM_GROUP);
        return;
    }
#endif

    Adc_GroupRuntime[Group].NotificationEnabled = TRUE;
    
    const Adc_GroupConfigType* cfg = Adc_GetGroupCfg(Group);
    uint32 base = Adc_GetUnitBase(cfg->Unit);
    uint32 ssBit = (1u << (uint32)cfg->Sequencer);
    ADC_REG(base, ADC_IM_OFFSET) |= ssBit;
}

void Adc_DisableGroupNotification(Adc_GroupType Group)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (Adc_Initialized == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_DISABLE_NOTIFICATION_SID, ADC_E_UNINIT);
        return;
    }
    if (Adc_IsGroupValid(Group) == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_DISABLE_NOTIFICATION_SID, ADC_E_PARAM_GROUP);
        return;
    }
#endif

    Adc_GroupRuntime[Group].NotificationEnabled = FALSE;
    
    const Adc_GroupConfigType* cfg = Adc_GetGroupCfg(Group);
    uint32 base = Adc_GetUnitBase(cfg->Unit);
    uint32 ssBit = (1u << (uint32)cfg->Sequencer);
    ADC_REG(base, ADC_IM_OFFSET) &= ~ssBit;
}

void Adc_MainFunction_Handling(void)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (Adc_Initialized == FALSE) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_MAIN_FUNCTION_HANDLING_SID, ADC_E_UNINIT);
        return;
    }
#endif

    uint16 i;
    for (i = 0u; i < Adc_ConfigPtr->GroupCount; i++) {
        if (Adc_GroupRuntime[i].Status == ADC_BUSY) {
            Adc_HandleGroupCompletion((Adc_GroupType)i);
        }
    }
}

static void Adc_HandleIsr(Adc_HwUnitType Unit, Adc_SequencerType Sequencer)
{
    if (Adc_ConfigPtr == NULL_PTR) {
        return;
    }

    uint16 i;
    for (i = 0u; i < Adc_ConfigPtr->GroupCount; i++) {
        const Adc_GroupConfigType* cfg = &Adc_ConfigPtr->Groups[i];
        if ((cfg->Unit == Unit) && (cfg->Sequencer == Sequencer)) {
            if (Adc_GroupRuntime[i].Status == ADC_BUSY) {
                Adc_HandleGroupCompletion((Adc_GroupType)i);
            }
        }
    }
}

void Adc_ADC0SS0_ISR(void) { Adc_HandleIsr(ADC_HW_UNIT_0, ADC_SEQUENCER_0); }
void Adc_ADC0SS1_ISR(void) { Adc_HandleIsr(ADC_HW_UNIT_0, ADC_SEQUENCER_1); }
void Adc_ADC0SS2_ISR(void) { Adc_HandleIsr(ADC_HW_UNIT_0, ADC_SEQUENCER_2); }
void Adc_ADC0SS3_ISR(void) { Adc_HandleIsr(ADC_HW_UNIT_0, ADC_SEQUENCER_3); }

void Adc_ADC1SS0_ISR(void) { Adc_HandleIsr(ADC_HW_UNIT_1, ADC_SEQUENCER_0); }
void Adc_ADC1SS1_ISR(void) { Adc_HandleIsr(ADC_HW_UNIT_1, ADC_SEQUENCER_1); }
void Adc_ADC1SS2_ISR(void) { Adc_HandleIsr(ADC_HW_UNIT_1, ADC_SEQUENCER_2); }
void Adc_ADC1SS3_ISR(void) { Adc_HandleIsr(ADC_HW_UNIT_1, ADC_SEQUENCER_3); }

#if (ADC_VERSION_INFO_API == STD_ON)
void Adc_GetVersionInfo(Std_VersionInfoType* VersionInfo)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    if (VersionInfo == NULL_PTR) {
        Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ADC_GET_VERSION_INFO_SID, ADC_E_PARAM_POINTER);
        return;
    }
#endif

    VersionInfo->vendorID = ADC_VENDOR_ID;
    VersionInfo->moduleID = ADC_MODULE_ID;
    VersionInfo->sw_major_version = ADC_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = ADC_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = ADC_SW_PATCH_VERSION;
}
#endif

