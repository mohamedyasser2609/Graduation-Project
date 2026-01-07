/**
 * @file ACS712.c
 * @brief ACS712 Current Sensor Driver Implementation
 * @details AUTOSAR-compliant driver for ACS712 Hall-effect current sensors
 *
 * Circuit Connection:
 * - VCC: 5V supply
 * - GND: Ground
 * - OUT: Analog output to TM4C123 ADC pin
 * - IP+/IP-: Current path (in series with load)
 *
 * Output Characteristics (30A model):
 * - Zero current output: VCC/2 = 2.5V
 * - Sensitivity: 66mV/A
 * - Current = (Vout - 2.5V) / 0.066
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0, MISRA C:2012
 */

#include "ACS712.h"
#include "../../MCAL/ADC/Adc.h"

#if (ACS712_DEV_ERROR_DETECT == STD_ON)
#include "../../CONFIG/Det.h"
#endif

/* ===================[Private Macros]=================== */
#define ACS712_ADC_TO_VOLTAGE(raw, vref)    (((float32)(raw) * (vref)) / (float32)ACS712_ADC_MAX_VALUE)

/* ===================[Private Types]=================== */
typedef struct
{
    float32                   ZeroOffset;       /**< Calibrated zero-current offset */
    float32                   LastCurrent;      /**< Last measured current */
    uint32                    SampleAccumulator; /**< For averaging */
    uint8                     SampleCount;      /**< Current sample count */
    ACS712_ChannelStatusType  Status;           /**< Channel status */
} ACS712_ChannelStateType;

/* ===================[Private Variables]=================== */
static const ACS712_ConfigType* ACS712_ConfigPtr = NULL_PTR;
static ACS712_StatusType ACS712_ModuleStatus = ACS712_STATUS_UNINIT;
static ACS712_ChannelStateType ACS712_ChannelStates[ACS712_MAX_CHANNELS];

#if (ACS712_OVERLOAD_CALLBACK_API == STD_ON)
static ACS712_OverloadCallbackType ACS712_OverloadCallback = NULL_PTR;
#endif

/* ===================[Private Function Declarations]=================== */
static boolean ACS712_IsChannelValid(ACS712_ChannelType Channel);
static float32 ACS712_GetSensitivity(ACS712_ModelType Model);
static Std_ReturnType ACS712_ReadAdcChannel(uint8 AdcChannel, uint16* Value);
static float32 ACS712_CalculateCurrent(uint16 RawAdc, const ACS712_ChannelConfigType* ChannelCfg,
                                        float32 ZeroOffset);
static void ACS712_CheckOverload(ACS712_ChannelType Channel, float32 Current,
                                  const ACS712_ChannelConfigType* ChannelCfg);

/* ===================[Private Function Implementations]=================== */

/**
 * @brief Check if channel ID is valid
 */
static boolean ACS712_IsChannelValid(ACS712_ChannelType Channel)
{
    boolean isValid = FALSE;
    
    if (ACS712_ConfigPtr != NULL_PTR)
    {
        if (Channel < ACS712_ConfigPtr->NumChannels)
        {
            isValid = TRUE;
        }
    }
    
    return isValid;
}

/**
 * @brief Get sensitivity value for sensor model
 */
static float32 ACS712_GetSensitivity(ACS712_ModelType Model)
{
    float32 sensitivity;
    
    switch (Model)
    {
        case ACS712_MODEL_5A:
            sensitivity = ACS712_SENSITIVITY_5A;
            break;
        case ACS712_MODEL_20A:
            sensitivity = ACS712_SENSITIVITY_20A;
            break;
        case ACS712_MODEL_30A:
        default:
            sensitivity = ACS712_SENSITIVITY_30A;
            break;
    }
    
    return sensitivity;
}

/**
 * @brief Read raw ADC value from channel with averaging
 */
static Std_ReturnType ACS712_ReadAdcChannel(uint8 AdcChannel, uint16* Value)
{
    Std_ReturnType status;
    uint16 adcValue = 0u;
    
    /* Use the ADC driver to read the value */
    status = Adc_ReadChannel(AdcChannel, &adcValue);
    
    if (status == E_OK)
    {
        *Value = adcValue;
    }
    
    return status;
}

/**
 * @brief Calculate current from raw ADC value
 */
static float32 ACS712_CalculateCurrent(uint16 RawAdc, const ACS712_ChannelConfigType* ChannelCfg,
                                        float32 ZeroOffset)
{
    float32 voltage;
    float32 current;
    float32 sensitivity;
    float32 zeroVoltage;
    
    /* Convert ADC value to voltage */
    voltage = ACS712_ADC_TO_VOLTAGE(RawAdc, ChannelCfg->VrefV);
    
    /* Get sensitivity for this model */
    sensitivity = ACS712_GetSensitivity(ChannelCfg->Model);
    
    /* Calculate zero-current reference (with calibration offset) */
    zeroVoltage = ChannelCfg->ZeroCurrentVoltage + ZeroOffset;
    
    /* Calculate current: I = (Vout - Vzero) / Sensitivity */
    current = (voltage - zeroVoltage) / sensitivity;
    
    return current;
}

/**
 * @brief Check for overload condition and trigger callback
 */
static void ACS712_CheckOverload(ACS712_ChannelType Channel, float32 Current,
                                  const ACS712_ChannelConfigType* ChannelCfg)
{
    float32 absCurrent;
    
    /* Get absolute current value */
    absCurrent = (Current >= 0.0f) ? Current : -Current;
    
    if (absCurrent > ChannelCfg->OverloadThreshold)
    {
        ACS712_ChannelStates[Channel].Status = ACS712_CHANNEL_OVERLOAD;
        
        #if (ACS712_OVERLOAD_CALLBACK_API == STD_ON)
        if (ACS712_OverloadCallback != NULL_PTR)
        {
            ACS712_OverloadCallback(Channel, Current);
        }
        #endif
    }
    else
    {
        if (ACS712_ChannelStates[Channel].Status == ACS712_CHANNEL_OVERLOAD)
        {
            /* Clear overload if current is below threshold */
            ACS712_ChannelStates[Channel].Status = ACS712_CHANNEL_OK;
        }
    }
}

/* ===================[Public Function Implementations]=================== */

/**
 * @brief Initialize the ACS712 driver
 */
void ACS712_Init(const ACS712_ConfigType* ConfigPtr)
{
    uint8 channelIdx;
    
#if (ACS712_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_INIT_SID, ACS712_E_PARAM_POINTER);
        return;
    }
    
    if (ACS712_ModuleStatus != ACS712_STATUS_UNINIT)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_INIT_SID, ACS712_E_ALREADY_INIT);
        return;
    }
    
    if (ConfigPtr->NumChannels > ACS712_MAX_CHANNELS)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_INIT_SID, ACS712_E_PARAM_CONFIG);
        return;
    }
    
    if (ConfigPtr->Channels == NULL_PTR)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_INIT_SID, ACS712_E_PARAM_POINTER);
        return;
    }
#endif
    
    /* Store configuration pointer */
    ACS712_ConfigPtr = ConfigPtr;
    
    /* Initialize channel states */
    for (channelIdx = 0u; channelIdx < ConfigPtr->NumChannels; channelIdx++)
    {
        ACS712_ChannelStates[channelIdx].ZeroOffset = 0.0f;
        ACS712_ChannelStates[channelIdx].LastCurrent = 0.0f;
        ACS712_ChannelStates[channelIdx].SampleAccumulator = 0u;
        ACS712_ChannelStates[channelIdx].SampleCount = 0u;
        ACS712_ChannelStates[channelIdx].Status = ACS712_CHANNEL_OK;
    }
    
    /* Set module status to idle */
    ACS712_ModuleStatus = ACS712_STATUS_IDLE;
}

#if (ACS712_DEINIT_API == STD_ON)
/**
 * @brief De-initialize the ACS712 driver
 */
void ACS712_DeInit(void)
{
    uint8 channelIdx;
    
#if (ACS712_DEV_ERROR_DETECT == STD_ON)
    if (ACS712_ModuleStatus == ACS712_STATUS_UNINIT)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_DEINIT_SID, ACS712_E_UNINIT);
        return;
    }
#endif
    
    /* Reset all channel states */
    for (channelIdx = 0u; channelIdx < ACS712_MAX_CHANNELS; channelIdx++)
    {
        ACS712_ChannelStates[channelIdx].ZeroOffset = 0.0f;
        ACS712_ChannelStates[channelIdx].LastCurrent = 0.0f;
        ACS712_ChannelStates[channelIdx].SampleAccumulator = 0u;
        ACS712_ChannelStates[channelIdx].SampleCount = 0u;
        ACS712_ChannelStates[channelIdx].Status = ACS712_CHANNEL_OK;
    }
    
    #if (ACS712_OVERLOAD_CALLBACK_API == STD_ON)
    ACS712_OverloadCallback = NULL_PTR;
    #endif
    
    ACS712_ConfigPtr = NULL_PTR;
    ACS712_ModuleStatus = ACS712_STATUS_UNINIT;
}
#endif

/**
 * @brief Read current from a specific channel
 */
Std_ReturnType ACS712_ReadCurrent(ACS712_ChannelType Channel, ACS712_DataType* DataPtr)
{
    Std_ReturnType status = E_NOT_OK;
    uint16 rawAdc = 0u;
    uint32 accumulator = 0u;
    uint8 sampleIdx;
    const ACS712_ChannelConfigType* channelCfg;
    
#if (ACS712_DEV_ERROR_DETECT == STD_ON)
    if (ACS712_ModuleStatus == ACS712_STATUS_UNINIT)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_READ_CURRENT_SID, ACS712_E_UNINIT);
        return E_NOT_OK;
    }
    
    if (DataPtr == NULL_PTR)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_READ_CURRENT_SID, ACS712_E_PARAM_POINTER);
        return E_NOT_OK;
    }
    
    if (!ACS712_IsChannelValid(Channel))
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_READ_CURRENT_SID, ACS712_E_PARAM_CHANNEL);
        return E_NOT_OK;
    }
#endif
    
    ACS712_ModuleStatus = ACS712_STATUS_BUSY;
    
    channelCfg = &ACS712_ConfigPtr->Channels[Channel];
    
    /* Read multiple samples for averaging */
    for (sampleIdx = 0u; sampleIdx < channelCfg->FilterSamples; sampleIdx++)
    {
        if (ACS712_ReadAdcChannel(channelCfg->AdcChannel, &rawAdc) == E_OK)
        {
            accumulator += rawAdc;
        }
        else
        {
            ACS712_ChannelStates[Channel].Status = ACS712_CHANNEL_FAULT;
            ACS712_ModuleStatus = ACS712_STATUS_ERROR;
            return E_NOT_OK;
        }
    }
    
    /* Calculate averaged ADC value */
    rawAdc = (uint16)(accumulator / channelCfg->FilterSamples);
    
    /* Calculate current */
    DataPtr->RawAdcValue = rawAdc;
    DataPtr->VoltageV = ACS712_ADC_TO_VOLTAGE(rawAdc, channelCfg->VrefV);
    DataPtr->CurrentAmps = ACS712_CalculateCurrent(rawAdc, channelCfg,
                                                    ACS712_ChannelStates[Channel].ZeroOffset);
    
    /* Store last reading */
    ACS712_ChannelStates[Channel].LastCurrent = DataPtr->CurrentAmps;
    
    /* Check for overload */
    ACS712_CheckOverload(Channel, DataPtr->CurrentAmps, channelCfg);
    
    DataPtr->Status = ACS712_ChannelStates[Channel].Status;
    
    ACS712_ModuleStatus = ACS712_STATUS_OK;
    status = E_OK;
    
    return status;
}

/**
 * @brief Read all configured channels
 */
Std_ReturnType ACS712_ReadAllChannels(ACS712_DataType* DataArray)
{
    Std_ReturnType status = E_OK;
    uint8 channelIdx;
    
#if (ACS712_DEV_ERROR_DETECT == STD_ON)
    if (ACS712_ModuleStatus == ACS712_STATUS_UNINIT)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_READ_CURRENT_SID, ACS712_E_UNINIT);
        return E_NOT_OK;
    }
    
    if (DataArray == NULL_PTR)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_READ_CURRENT_SID, ACS712_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif
    
    for (channelIdx = 0u; channelIdx < ACS712_ConfigPtr->NumChannels; channelIdx++)
    {
        if (ACS712_ReadCurrent(channelIdx, &DataArray[channelIdx]) != E_OK)
        {
            status = E_NOT_OK;
        }
    }
    
    return status;
}

/**
 * @brief Get raw ADC value from channel
 */
Std_ReturnType ACS712_GetRawValue(ACS712_ChannelType Channel, uint16* RawValue)
{
    Std_ReturnType status;
    const ACS712_ChannelConfigType* channelCfg;
    
#if (ACS712_DEV_ERROR_DETECT == STD_ON)
    if (ACS712_ModuleStatus == ACS712_STATUS_UNINIT)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_READ_CURRENT_SID, ACS712_E_UNINIT);
        return E_NOT_OK;
    }
    
    if (RawValue == NULL_PTR)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_READ_CURRENT_SID, ACS712_E_PARAM_POINTER);
        return E_NOT_OK;
    }
    
    if (!ACS712_IsChannelValid(Channel))
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_READ_CURRENT_SID, ACS712_E_PARAM_CHANNEL);
        return E_NOT_OK;
    }
#endif
    
    channelCfg = &ACS712_ConfigPtr->Channels[Channel];
    status = ACS712_ReadAdcChannel(channelCfg->AdcChannel, RawValue);
    
    return status;
}

/**
 * @brief Check if channel is in overload condition
 */
boolean ACS712_IsOverload(ACS712_ChannelType Channel)
{
    boolean isOverload = FALSE;
    
#if (ACS712_DEV_ERROR_DETECT == STD_ON)
    if (ACS712_ModuleStatus == ACS712_STATUS_UNINIT)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_GET_STATUS_SID, ACS712_E_UNINIT);
        return FALSE;
    }
    
    if (!ACS712_IsChannelValid(Channel))
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_GET_STATUS_SID, ACS712_E_PARAM_CHANNEL);
        return FALSE;
    }
#endif
    
    if (ACS712_ChannelStates[Channel].Status == ACS712_CHANNEL_OVERLOAD)
    {
        isOverload = TRUE;
    }
    
    return isOverload;
}

/**
 * @brief Get channel status
 */
ACS712_ChannelStatusType ACS712_GetChannelStatus(ACS712_ChannelType Channel)
{
    ACS712_ChannelStatusType status = ACS712_CHANNEL_FAULT;
    
#if (ACS712_DEV_ERROR_DETECT == STD_ON)
    if (ACS712_ModuleStatus == ACS712_STATUS_UNINIT)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_GET_STATUS_SID, ACS712_E_UNINIT);
        return ACS712_CHANNEL_FAULT;
    }
    
    if (!ACS712_IsChannelValid(Channel))
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_GET_STATUS_SID, ACS712_E_PARAM_CHANNEL);
        return ACS712_CHANNEL_FAULT;
    }
#endif
    
    status = ACS712_ChannelStates[Channel].Status;
    
    return status;
}

/**
 * @brief Get driver status
 */
ACS712_StatusType ACS712_GetStatus(void)
{
    return ACS712_ModuleStatus;
}

/**
 * @brief Calibrate zero-current offset for a channel
 */
Std_ReturnType ACS712_CalibrateZero(ACS712_ChannelType Channel)
{
    Std_ReturnType status = E_NOT_OK;
    uint32 accumulator = 0u;
    uint16 rawAdc = 0u;
    uint16 avgAdc;
    uint8 sampleIdx;
    float32 measuredVoltage;
    const ACS712_ChannelConfigType* channelCfg;
    const uint8 calibrationSamples = 16u;
    
#if (ACS712_DEV_ERROR_DETECT == STD_ON)
    if (ACS712_ModuleStatus == ACS712_STATUS_UNINIT)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_INIT_SID, ACS712_E_UNINIT);
        return E_NOT_OK;
    }
    
    if (!ACS712_IsChannelValid(Channel))
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_INIT_SID, ACS712_E_PARAM_CHANNEL);
        return E_NOT_OK;
    }
#endif
    
    channelCfg = &ACS712_ConfigPtr->Channels[Channel];
    
    /* Read multiple samples for averaging */
    for (sampleIdx = 0u; sampleIdx < calibrationSamples; sampleIdx++)
    {
        if (ACS712_ReadAdcChannel(channelCfg->AdcChannel, &rawAdc) == E_OK)
        {
            accumulator += rawAdc;
        }
        else
        {
            return E_NOT_OK;
        }
    }
    
    avgAdc = (uint16)(accumulator / calibrationSamples);
    measuredVoltage = ACS712_ADC_TO_VOLTAGE(avgAdc, channelCfg->VrefV);
    
    /* Calculate offset from expected zero-current voltage */
    ACS712_ChannelStates[Channel].ZeroOffset = measuredVoltage - channelCfg->ZeroCurrentVoltage;
    
    status = E_OK;
    
    return status;
}

#if (ACS712_OVERLOAD_CALLBACK_API == STD_ON)
/**
 * @brief Set overload notification callback
 */
void ACS712_SetOverloadCallback(ACS712_OverloadCallbackType Callback)
{
    ACS712_OverloadCallback = Callback;
}
#endif

#if (ACS712_VERSION_INFO_API == STD_ON)
/**
 * @brief Get driver version information
 */
void ACS712_GetVersionInfo(Std_VersionInfoType* versionInfoPtr)
{
#if (ACS712_DEV_ERROR_DETECT == STD_ON)
    if (versionInfoPtr == NULL_PTR)
    {
        Det_ReportError(ACS712_MODULE_ID, ACS712_INSTANCE_ID, ACS712_GET_VERSION_SID, ACS712_E_PARAM_POINTER);
        return;
    }
#endif
    
    versionInfoPtr->vendorID = ACS712_VENDOR_ID;
    versionInfoPtr->moduleID = ACS712_MODULE_ID;
    versionInfoPtr->sw_major_version = ACS712_SW_MAJOR_VERSION;
    versionInfoPtr->sw_minor_version = ACS712_SW_MINOR_VERSION;
    versionInfoPtr->sw_patch_version = ACS712_SW_PATCH_VERSION;
}
#endif
