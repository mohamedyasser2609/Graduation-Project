/*
 * @file Timer_Regs.h
 * @brief General Purpose Timer (GPTM) Register Definitions for TM4C123GH6PM
 * @details This file contains register addresses and bit field definitions
 *          for the 6 GPTM modules (Timer0-Timer5).
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_TIMER_TIMER_REGS_H_
#define MCAL_TIMER_TIMER_REGS_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[Timer Base Addresses]=================== */
#define TIMER0_BASE_ADDRESS        (0x40030000UL)
#define TIMER1_BASE_ADDRESS        (0x40031000UL)
#define TIMER2_BASE_ADDRESS        (0x40032000UL)
#define TIMER3_BASE_ADDRESS        (0x40033000UL)
#define TIMER4_BASE_ADDRESS        (0x40034000UL)
#define TIMER5_BASE_ADDRESS        (0x40035000UL)

/* ===================[Timer Register Offsets]=================== */
#define GPTMCFG_OFFSET             (0x000UL)    /**< Configuration */
#define GPTMTAMR_OFFSET            (0x004UL)    /**< Timer A Mode */
#define GPTMTBMR_OFFSET            (0x008UL)    /**< Timer B Mode */
#define GPTMCTL_OFFSET             (0x00CUL)    /**< Control */
#define GPTMSYNC_OFFSET            (0x010UL)    /**< Synchronize */
#define GPTMIMR_OFFSET             (0x018UL)    /**< Interrupt Mask */
#define GPTMRIS_OFFSET             (0x01CUL)    /**< Raw Interrupt Status */
#define GPTMMIS_OFFSET             (0x020UL)    /**< Masked Interrupt Status */
#define GPTMICR_OFFSET             (0x024UL)    /**< Interrupt Clear */
#define GPTMTAILR_OFFSET           (0x028UL)    /**< Timer A Interval Load */
#define GPTMTBILR_OFFSET           (0x02CUL)    /**< Timer B Interval Load */
#define GPTMTAMATCHR_OFFSET        (0x030UL)    /**< Timer A Match */
#define GPTMTBMATCHR_OFFSET        (0x034UL)    /**< Timer B Match */
#define GPTMTAPR_OFFSET            (0x038UL)    /**< Timer A Prescale */
#define GPTMTBPR_OFFSET            (0x03CUL)    /**< Timer B Prescale */
#define GPTMTAPMR_OFFSET           (0x040UL)    /**< Timer A Prescale Match */
#define GPTMTBPMR_OFFSET           (0x044UL)    /**< Timer B Prescale Match */
#define GPTMTAR_OFFSET             (0x048UL)    /**< Timer A */
#define GPTMTBR_OFFSET             (0x04CUL)    /**< Timer B */
#define GPTMTAV_OFFSET             (0x050UL)    /**< Timer A Value */
#define GPTMTBV_OFFSET             (0x054UL)    /**< Timer B Value */
#define GPTMRTCPD_OFFSET           (0x058UL)    /**< RTC Predivide */
#define GPTMTAPS_OFFSET            (0x05CUL)    /**< Timer A Prescale Snapshot */
#define GPTMTBPS_OFFSET            (0x060UL)    /**< Timer B Prescale Snapshot */
#define GPTMTAPV_OFFSET            (0x064UL)    /**< Timer A Prescale Value */
#define GPTMTBPV_OFFSET            (0x068UL)    /**< Timer B Prescale Value */
#define GPTMPP_OFFSET              (0xFC0UL)    /**< Peripheral Properties */

/* ===================[System Control Registers]=================== */
#define SYSCTL_RCGCTIMER           (*((volatile uint32*)0x400FE604UL))
#define SYSCTL_PRTIMER             (*((volatile uint32*)0x400FEA04UL))

/* ===================[Register Access Macros]=================== */
#define TIMER_REG(base, offset)    (*((volatile uint32*)((base) + (offset))))

/* ===================[GPTMCFG Register Bits]=================== */
#define GPTMCFG_32BIT              (0x00000000UL)  /**< 32-bit timer configuration */
#define GPTMCFG_32BIT_RTC          (0x00000001UL)  /**< 32-bit RTC configuration */
#define GPTMCFG_16BIT              (0x00000004UL)  /**< 16-bit timer configuration */

/* ===================[GPTMTAMR/GPTMTBMR Register Bits]=================== */
#define GPTMTMR_MR_ONESHOT         (0x00000001UL)  /**< One-shot mode */
#define GPTMTMR_MR_PERIODIC        (0x00000002UL)  /**< Periodic mode */
#define GPTMTMR_MR_CAPTURE         (0x00000003UL)  /**< Capture mode */
#define GPTMTMR_CMR                (0x00000004UL)  /**< Capture mode */
#define GPTMTMR_AMS                (0x00000008UL)  /**< PWM mode */
#define GPTMTMR_CDIR               (0x00000010UL)  /**< Count direction */
#define GPTMTMR_MIE                (0x00000020UL)  /**< Match interrupt enable */
#define GPTMTMR_WOT                (0x00000040UL)  /**< Wait on trigger */
#define GPTMTMR_SNAPS              (0x00000080UL)  /**< Snapshot mode */
#define GPTMTMR_ILD                (0x00000100UL)  /**< Interval load mode */
#define GPTMTMR_PWMIE              (0x00000200UL)  /**< PWM interrupt enable */
#define GPTMTMR_MRSU               (0x00000400UL)  /**< Match register update */
#define GPTMTMR_PLO                (0x00000800UL)  /**< PWM legacy operation */

/* ===================[GPTMCTL Register Bits]=================== */
#define GPTMCTL_TAEN               (0x00000001UL)  /**< Timer A enable */
#define GPTMCTL_TASTALL            (0x00000002UL)  /**< Timer A stall enable */
#define GPTMCTL_TAEVENT_POS        (0x00000004UL)  /**< Positive edge */
#define GPTMCTL_TAEVENT_NEG        (0x0000000CUL)  /**< Negative edge */
#define GPTMCTL_TAEVENT_BOTH       (0x00000000UL)  /**< Both edges */
#define GPTMCTL_RTCEN              (0x00000010UL)  /**< RTC enable */
#define GPTMCTL_TAOTE              (0x00000020UL)  /**< Timer A output trigger */
#define GPTMCTL_TAPWML             (0x00000040UL)  /**< Timer A PWM output level */
#define GPTMCTL_TBEN               (0x00000100UL)  /**< Timer B enable */
#define GPTMCTL_TBSTALL            (0x00000200UL)  /**< Timer B stall enable */
#define GPTMCTL_TBEVENT_POS        (0x00000400UL)  /**< Positive edge */
#define GPTMCTL_TBEVENT_NEG        (0x00000C00UL)  /**< Negative edge */
#define GPTMCTL_TBEVENT_BOTH       (0x00000000UL)  /**< Both edges */
#define GPTMCTL_TBOTE              (0x00002000UL)  /**< Timer B output trigger */
#define GPTMCTL_TBPWML             (0x00004000UL)  /**< Timer B PWM output level */

/* ===================[GPTMIMR Register Bits]=================== */
#define GPTMIMR_TATOIM             (0x00000001UL)  /**< Timer A timeout interrupt mask */
#define GPTMIMR_CAMIM              (0x00000002UL)  /**< Capture A match interrupt mask */
#define GPTMIMR_CAEIM              (0x00000004UL)  /**< Capture A event interrupt mask */
#define GPTMIMR_RTCIM              (0x00000008UL)  /**< RTC interrupt mask */
#define GPTMIMR_TAMIM              (0x00000010UL)  /**< Timer A match interrupt mask */
#define GPTMIMR_TBTOIM             (0x00000100UL)  /**< Timer B timeout interrupt mask */
#define GPTMIMR_CBMIM              (0x00000200UL)  /**< Capture B match interrupt mask */
#define GPTMIMR_CBEIM              (0x00000400UL)  /**< Capture B event interrupt mask */
#define GPTMIMR_TBMIM              (0x00001000UL)  /**< Timer B match interrupt mask */
#define GPTMIMR_WUEIM              (0x00010000UL)  /**< Write update error interrupt mask */

/* ===================[GPTMICR Register Bits]=================== */
#define GPTMICR_TATOCINT           (0x00000001UL)  /**< Timer A timeout interrupt clear */
#define GPTMICR_CAMCINT            (0x00000002UL)  /**< Capture A match interrupt clear */
#define GPTMICR_CAECINT            (0x00000004UL)  /**< Capture A event interrupt clear */
#define GPTMICR_RTCCINT            (0x00000008UL)  /**< RTC interrupt clear */
#define GPTMICR_TAMCINT            (0x00000010UL)  /**< Timer A match interrupt clear */
#define GPTMICR_TBTOCINT           (0x00000100UL)  /**< Timer B timeout interrupt clear */
#define GPTMICR_CBMCINT            (0x00000200UL)  /**< Capture B match interrupt clear */
#define GPTMICR_CBECINT            (0x00000400UL)  /**< Capture B event interrupt clear */
#define GPTMICR_TBMCINT            (0x00001000UL)  /**< Timer B match interrupt clear */
#define GPTMICR_WUECINT            (0x00010000UL)  /**< Write update error interrupt clear */

#endif /* MCAL_TIMER_TIMER_REGS_H_ */
