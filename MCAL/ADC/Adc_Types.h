#ifndef MCAL_ADC_ADC_TYPES_H_
#define MCAL_ADC_ADC_TYPES_H_

#include "../../CONFIG/Std_Types.h"

typedef uint8 Adc_GroupType;
typedef uint16 Adc_ValueGroupType;

typedef enum {
    ADC_HW_UNIT_0 = 0u,
    ADC_HW_UNIT_1 = 1u
} Adc_HwUnitType;

typedef enum {
    ADC_SEQUENCER_0 = 0u,
    ADC_SEQUENCER_1 = 1u,
    ADC_SEQUENCER_2 = 2u,
    ADC_SEQUENCER_3 = 3u
} Adc_SequencerType;

typedef enum {
    ADC_TRIGG_SRC_PROCESSOR = 0u,
    ADC_TRIGG_SRC_COMP0 = 1u,
    ADC_TRIGG_SRC_COMP1 = 2u,
    ADC_TRIGG_SRC_COMP2 = 3u,
    ADC_TRIGG_SRC_EXTERNAL = 4u,
    ADC_TRIGG_SRC_TIMER = 5u,
    ADC_TRIGG_SRC_PWM0 = 6u,
    ADC_TRIGG_SRC_PWM1 = 7u,
    ADC_TRIGG_SRC_PWM2 = 8u,
    ADC_TRIGG_SRC_PWM3 = 9u,
    ADC_TRIGG_SRC_ALWAYS = 15u
} Adc_TriggerSourceType;

typedef enum {
    ADC_CONV_MODE_ONESHOT = 0u,
    ADC_CONV_MODE_CONTINUOUS = 1u
} Adc_GroupConvModeType;

typedef enum {
    ADC_ACCESS_MODE_SINGLE = 0u,
    ADC_ACCESS_MODE_STREAMING = 1u
} Adc_GroupAccessModeType;

typedef enum {
    ADC_STREAM_BUFFER_LINEAR = 0u,
    ADC_STREAM_BUFFER_CIRCULAR = 1u
} Adc_StreamBufferModeType;

typedef enum {
    ADC_SAMPLE_AVG_1 = 0u,
    ADC_SAMPLE_AVG_2 = 1u,
    ADC_SAMPLE_AVG_4 = 2u,
    ADC_SAMPLE_AVG_8 = 3u,
    ADC_SAMPLE_AVG_16 = 4u,
    ADC_SAMPLE_AVG_32 = 5u,
    ADC_SAMPLE_AVG_64 = 6u
} Adc_SampleAveragingType;

typedef enum {
    ADC_IDLE = 0u,
    ADC_BUSY = 1u,
    ADC_COMPLETED = 2u,
    ADC_STREAM_COMPLETED = 3u
} Adc_StatusType;

typedef uint8 Adc_ChannelType;

#define ADC_CHANNEL_AIN0      (0u)
#define ADC_CHANNEL_AIN1      (1u)
#define ADC_CHANNEL_AIN2      (2u)
#define ADC_CHANNEL_AIN3      (3u)
#define ADC_CHANNEL_AIN4      (4u)
#define ADC_CHANNEL_AIN5      (5u)
#define ADC_CHANNEL_AIN6      (6u)
#define ADC_CHANNEL_AIN7      (7u)
#define ADC_CHANNEL_AIN8      (8u)
#define ADC_CHANNEL_AIN9      (9u)
#define ADC_CHANNEL_AIN10     (10u)
#define ADC_CHANNEL_AIN11     (11u)
#define ADC_CHANNEL_TEMP      (16u)

typedef void (*Adc_GroupNotificationType)(Adc_GroupType Group);

typedef struct {
    Adc_HwUnitType Unit;
    Adc_SequencerType Sequencer;
    uint8 Priority;
    const Adc_ChannelType* Channels;
    uint8 ChannelCount;
    Adc_TriggerSourceType TriggerSource;
    Adc_GroupConvModeType ConvMode;
    Adc_GroupAccessModeType AccessMode;
    Adc_StreamBufferModeType StreamBufferMode;
    uint16 StreamNumSamples;
    Adc_SampleAveragingType SampleAveraging;
    Adc_GroupNotificationType Notification;
} Adc_GroupConfigType;

typedef struct {
    const Adc_GroupConfigType* Groups;
    uint16 GroupCount;
} Adc_ConfigType;

#endif /* MCAL_ADC_ADC_TYPES_H_ */

