#include "Adc.h"

static const Adc_ChannelType Adc_Group0_Channels[] = { ADC_CHANNEL_AIN0 };
static const Adc_ChannelType Adc_Group1_Channels[] = { ADC_CHANNEL_AIN1 };

static const Adc_GroupConfigType Adc_Groups[ADC_CONFIGURED_GROUPS] = {
    {
        .Unit = ADC_HW_UNIT_0,
        .Sequencer = ADC_SEQUENCER_3,
        .Priority = 0u,
        .Channels = Adc_Group0_Channels,
        .ChannelCount = 1u,
        .TriggerSource = ADC_TRIGG_SRC_PROCESSOR,
        .ConvMode = ADC_CONV_MODE_ONESHOT,
        .AccessMode = ADC_ACCESS_MODE_SINGLE,
        .StreamBufferMode = ADC_STREAM_BUFFER_LINEAR,
        .StreamNumSamples = 1u,
        .SampleAveraging = ADC_SAMPLE_AVG_1,
        .Notification = NULL_PTR
    },
    {
        .Unit = ADC_HW_UNIT_0,
        .Sequencer = ADC_SEQUENCER_2,
        .Priority = 1u,
        .Channels = Adc_Group1_Channels,
        .ChannelCount = 1u,
        .TriggerSource = ADC_TRIGG_SRC_PROCESSOR,
        .ConvMode = ADC_CONV_MODE_ONESHOT,
        .AccessMode = ADC_ACCESS_MODE_SINGLE,
        .StreamBufferMode = ADC_STREAM_BUFFER_LINEAR,
        .StreamNumSamples = 1u,
        .SampleAveraging = ADC_SAMPLE_AVG_1,
        .Notification = NULL_PTR
    }
};

const Adc_ConfigType Adc_Config = {
    .Groups = Adc_Groups,
    .GroupCount = ADC_CONFIGURED_GROUPS
};

