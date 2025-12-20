#ifndef MCAL_ADC_ADC_REGS_H_
#define MCAL_ADC_ADC_REGS_H_

#include "../../CONFIG/Std_Types.h"

#define ADC0_BASE_ADDRESS                (0x40038000UL)
#define ADC1_BASE_ADDRESS                (0x40039000UL)

#define ADC_ACTSS_OFFSET                 (0x000UL)
#define ADC_RIS_OFFSET                   (0x004UL)
#define ADC_IM_OFFSET                    (0x008UL)
#define ADC_ISC_OFFSET                   (0x00CUL)
#define ADC_EMUX_OFFSET                  (0x014UL)
#define ADC_SSPRI_OFFSET                 (0x020UL)
#define ADC_PSSI_OFFSET                  (0x028UL)
#define ADC_SAC_OFFSET                   (0x030UL)

#define ADC_SSMUX0_OFFSET                (0x040UL)
#define ADC_SSCTL0_OFFSET                (0x044UL)
#define ADC_SSFIFO0_OFFSET               (0x048UL)
#define ADC_SSFSTAT0_OFFSET              (0x04CUL)

#define ADC_SSMUX_STRIDE                 (0x20UL)

#define SYSCTL_RCGCADC_R                 (*((volatile uint32*)0x400FE638UL))
#define SYSCTL_PRADC_R                   (*((volatile uint32*)0x400FEA38UL))

#define SYSCTL_RCGCADC_R0                (0x00000001UL)
#define SYSCTL_RCGCADC_R1                (0x00000002UL)
#define SYSCTL_PRADC_R0                  (0x00000001UL)
#define SYSCTL_PRADC_R1                  (0x00000002UL)

#define ADC_REG(base, offset)            (*((volatile uint32*)((base) + (offset))))

#define ADC_FIFO_DATA_MASK               (0x00000FFFUL)

#endif /* MCAL_ADC_ADC_REGS_H_ */

