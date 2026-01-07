# ADC Driver Documentation

**TM4C123GH6PM Analog-to-Digital Converter Driver**

---

## Overview

AUTOSAR-compliant ADC driver supporting up to 12 analog input channels with hardware averaging, multiple trigger sources, and interrupt-driven conversions.

### Features
- 2 ADC modules (ADC0, ADC1)
- 4 sequencers per module (SS0-SS3)
- 12 analog input channels (AIN0-AIN11) + temperature sensor
- 12-bit resolution
- Hardware sample averaging (1x-64x)
- Multiple trigger sources (processor, timer, PWM, external)
- One-shot and continuous conversion modes
- Interrupt-driven with notification callbacks

---

## Hardware Configuration

### Pin Mapping

| Channel | Pin | Description |
|---------|-----|-------------|
| AIN0 | PE3 | Analog Input 0 |
| AIN1 | PE2 | Analog Input 1 |
| AIN2 | PE1 | Analog Input 2 |
| AIN3 | PE0 | Analog Input 3 |
| AIN4 | PD3 | Analog Input 4 |
| AIN5 | PD2 | Analog Input 5 |
| AIN6 | PD1 | Analog Input 6 |
| AIN7 | PD0 | Analog Input 7 |
| AIN8 | PE5 | Analog Input 8 |
| AIN9 | PE4 | Analog Input 9 |
| AIN10 | PB4 | Analog Input 10 |
| AIN11 | PB5 | Analog Input 11 |
| TEMP | Internal | Temperature Sensor |

### Sequencer Capabilities

| Sequencer | Max Samples | FIFO Depth |
|-----------|-------------|------------|
| SS0 | 8 | 8 |
| SS1 | 4 | 4 |
| SS2 | 4 | 4 |
| SS3 | 1 | 1 |

---

## API Reference

### Initialization

```c
/* Initialize ADC with configuration */
void Adc_Init(const Adc_ConfigType* ConfigPtr);

/* De-initialize ADC module */
void Adc_DeInit(void);
```

### Conversion Control

```c
/* Setup result buffer for group */
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);

/* Start software-triggered conversion */
void Adc_StartGroupConversion(Adc_GroupType Group);

/* Stop ongoing conversion */
void Adc_StopGroupConversion(Adc_GroupType Group);

/* Read conversion results */
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);
```

### Hardware Trigger Control

```c
/* Enable hardware trigger for group */
void Adc_EnableHardwareTrigger(Adc_GroupType Group);

/* Disable hardware trigger */
void Adc_DisableHardwareTrigger(Adc_GroupType Group);
```

### Status and Notifications

```c
/* Get group conversion status */
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);

/* Enable/Disable conversion complete notification */
void Adc_EnableGroupNotification(Adc_GroupType Group);
void Adc_DisableGroupNotification(Adc_GroupType Group);
```

---

## Usage Example

```c
#include "MCAL/ADC/Adc.h"

/* Buffer for ADC results */
Adc_ValueGroupType adcBuffer[4];

void main(void) {
    /* Initialize ADC */
    Adc_Init(&Adc_Configuration);
    
    /* Setup result buffer */
    Adc_SetupResultBuffer(ADC_GROUP_0, adcBuffer);
    
    /* Start conversion */
    Adc_StartGroupConversion(ADC_GROUP_0);
    
    /* Wait for completion */
    while (Adc_GetGroupStatus(ADC_GROUP_0) != ADC_COMPLETED);
    
    /* Read results */
    Adc_ReadGroup(ADC_GROUP_0, adcBuffer);
    
    /* Convert to voltage (3.3V reference) */
    float voltage = (adcBuffer[0] / 4096.0f) * 3.3f;
}
```

---

## Trigger Sources

| Trigger | Value | Description |
|---------|-------|-------------|
| `ADC_TRIGG_SRC_PROCESSOR` | 0 | Software trigger |
| `ADC_TRIGG_SRC_TIMER` | 5 | Timer trigger |
| `ADC_TRIGG_SRC_PWM0-3` | 6-9 | PWM module trigger |
| `ADC_TRIGG_SRC_EXTERNAL` | 4 | External GPIO trigger |
| `ADC_TRIGG_SRC_ALWAYS` | 15 | Continuous sampling |

---

## Version Information

| Item | Value |
|------|-------|
| Module ID | ADC |
| Driver Version | 1.0.0 |
| AUTOSAR Version | 4.4.0 |
