#include "System_Services.h"
#include "../../MCAL/ADC/Adc.h"

/* Extern configuration from Adc_PBCfg.c */
extern const Adc_ConfigType Adc_Config;

void System_Init(void)
{
    /* Initialize ADC Driver */
    Adc_Init(&Adc_Config);
    
    /* TODO: Add other driver initializations here as needed */
}


