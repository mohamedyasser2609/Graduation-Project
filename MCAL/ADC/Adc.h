#ifndef MCAL_ADC_ADC_H_
#define MCAL_ADC_ADC_H_

#include "../../CONFIG/Std_Types.h"
#include "../../CONFIG/Det.h"
#include "Adc_Types.h"
#include "Adc_Cfg.h"

void Adc_Init(const Adc_ConfigType* ConfigPtr);
void Adc_DeInit(void);

Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);
void Adc_StartGroupConversion(Adc_GroupType Group);
void Adc_StopGroupConversion(Adc_GroupType Group);
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);

/**
 * @brief Read single ADC channel value
 * @param Channel ADC channel number
 * @param Value Pointer to store ADC value
 * @return E_OK if successful
 */
Std_ReturnType Adc_ReadChannel(uint8 Channel, uint16* Value);

void Adc_EnableHardwareTrigger(Adc_GroupType Group);
void Adc_DisableHardwareTrigger(Adc_GroupType Group);
void Adc_EnableGroupNotification(Adc_GroupType Group);
void Adc_DisableGroupNotification(Adc_GroupType Group);

void Adc_MainFunction_Handling(void);

#if (ADC_VERSION_INFO_API == STD_ON)
void Adc_GetVersionInfo(Std_VersionInfoType* VersionInfo);
#endif

#endif /* MCAL_ADC_ADC_H_ */

