/*
 * @file Uart_Regs.h
 * @brief UART Register Definitions for TM4C123GH6PM
 * @details This file contains register addresses and bit field definitions
 *          for the 8 UART modules (UART0-UART7).
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 1.0.0
 * @compliance AUTOSAR 4.4.0
 */

#ifndef MCAL_UART_UART_REGS_H_
#define MCAL_UART_UART_REGS_H_

/* ===================[Includes]=================== */
#include "../../CONFIG/Std_Types.h"

/* ===================[UART Base Addresses]=================== */
#define UART0_BASE_ADDRESS        (0x4000C000UL)
#define UART1_BASE_ADDRESS        (0x4000D000UL)
#define UART2_BASE_ADDRESS        (0x4000E000UL)
#define UART3_BASE_ADDRESS        (0x4000F000UL)
#define UART4_BASE_ADDRESS        (0x40010000UL)
#define UART5_BASE_ADDRESS        (0x40011000UL)
#define UART6_BASE_ADDRESS        (0x40012000UL)
#define UART7_BASE_ADDRESS        (0x40013000UL)

/* ===================[UART Register Offsets]=================== */
#define UARTDR_OFFSET             (0x000UL)    /**< Data Register */
#define UARTRSR_OFFSET            (0x004UL)    /**< Receive Status Register */
#define UARTECR_OFFSET            (0x004UL)    /**< Error Clear Register */
#define UARTFR_OFFSET             (0x018UL)    /**< Flag Register */
#define UARTILPR_OFFSET           (0x020UL)    /**< IrDA Low-Power Register */
#define UARTIBRD_OFFSET           (0x024UL)    /**< Integer Baud Rate Divisor */
#define UARTFBRD_OFFSET           (0x028UL)    /**< Fractional Baud Rate Divisor */
#define UARTLCRH_OFFSET           (0x02CUL)    /**< Line Control Register */
#define UARTCTL_OFFSET            (0x030UL)    /**< Control Register */
#define UARTIFLS_OFFSET           (0x034UL)    /**< Interrupt FIFO Level Select */
#define UARTIM_OFFSET             (0x038UL)    /**< Interrupt Mask Register */
#define UARTRIS_OFFSET            (0x03CUL)    /**< Raw Interrupt Status */
#define UARTMIS_OFFSET            (0x040UL)    /**< Masked Interrupt Status */
#define UARTICR_OFFSET            (0x044UL)    /**< Interrupt Clear Register */
#define UARTDMACTL_OFFSET         (0x048UL)    /**< DMA Control Register */
#define UART9BITADDR_OFFSET       (0x0A4UL)    /**< 9-Bit Address Register */
#define UART9BITAMASK_OFFSET      (0x0A8UL)    /**< 9-Bit Address Mask */
#define UARTPP_OFFSET             (0xFC0UL)    /**< Peripheral Properties */
#define UARTCC_OFFSET             (0xFC8UL)    /**< Clock Configuration */

/* ===================[System Control Registers]=================== */
#define SYSCTL_RCGCUART           (*((volatile uint32*)0x400FE618UL))
#define SYSCTL_PRUART             (*((volatile uint32*)0x400FEA18UL))

/* ===================[Register Access Macros]=================== */
#define UART_REG(base, offset)    (*((volatile uint32*)((base) + (offset))))

/* ===================[UARTFR (Flag Register) Bits]=================== */
#define UARTFR_CTS                (0x00000001UL)  /**< Clear to Send */
#define UARTFR_BUSY               (0x00000008UL)  /**< UART Busy */
#define UARTFR_RXFE               (0x00000010UL)  /**< Receive FIFO Empty */
#define UARTFR_TXFF               (0x00000020UL)  /**< Transmit FIFO Full */
#define UARTFR_RXFF               (0x00000040UL)  /**< Receive FIFO Full */
#define UARTFR_TXFE               (0x00000080UL)  /**< Transmit FIFO Empty */

/* ===================[UARTLCRH (Line Control) Bits]=================== */
#define UARTLCRH_BRK              (0x00000001UL)  /**< Send Break */
#define UARTLCRH_PEN              (0x00000002UL)  /**< Parity Enable */
#define UARTLCRH_EPS              (0x00000004UL)  /**< Even Parity Select */
#define UARTLCRH_STP2             (0x00000008UL)  /**< Two Stop Bits Select */
#define UARTLCRH_FEN              (0x00000010UL)  /**< FIFO Enable */
#define UARTLCRH_WLEN_5           (0x00000000UL)  /**< Word Length 5 bits */
#define UARTLCRH_WLEN_6           (0x00000020UL)  /**< Word Length 6 bits */
#define UARTLCRH_WLEN_7           (0x00000040UL)  /**< Word Length 7 bits */
#define UARTLCRH_WLEN_8           (0x00000060UL)  /**< Word Length 8 bits */
#define UARTLCRH_SPS              (0x00000080UL)  /**< Stick Parity Select */

/* ===================[UARTCTL (Control Register) Bits]=================== */
#define UARTCTL_UARTEN            (0x00000001UL)  /**< UART Enable */
#define UARTCTL_SIREN             (0x00000002UL)  /**< IrDA Enable */
#define UARTCTL_SIRLP             (0x00000004UL)  /**< IrDA Low-Power Mode */
#define UARTCTL_EOT               (0x00000010UL)  /**< End of Transmission */
#define UARTCTL_HSE               (0x00000020UL)  /**< High-Speed Enable */
#define UARTCTL_LBE               (0x00000080UL)  /**< Loopback Enable */
#define UARTCTL_TXE               (0x00000100UL)  /**< Transmit Enable */
#define UARTCTL_RXE               (0x00000200UL)  /**< Receive Enable */
#define UARTCTL_RTS               (0x00000800UL)  /**< Request to Send */
#define UARTCTL_RTSEN             (0x00004000UL)  /**< RTS Hardware Flow Control Enable */
#define UARTCTL_CTSEN             (0x00008000UL)  /**< CTS Hardware Flow Control Enable */

/* ===================[UARTIM (Interrupt Mask) Bits]=================== */
#define UARTIM_RXIM               (0x00000010UL)  /**< Receive Interrupt Mask */
#define UARTIM_TXIM               (0x00000020UL)  /**< Transmit Interrupt Mask */
#define UARTIM_RTIM               (0x00000040UL)  /**< Receive Timeout Interrupt Mask */
#define UARTIM_FEIM               (0x00000080UL)  /**< Framing Error Interrupt Mask */
#define UARTIM_PEIM               (0x00000100UL)  /**< Parity Error Interrupt Mask */
#define UARTIM_BEIM               (0x00000200UL)  /**< Break Error Interrupt Mask */
#define UARTIM_OEIM               (0x00000400UL)  /**< Overrun Error Interrupt Mask */

/* ===================[UARTICR (Interrupt Clear) Bits]=================== */
#define UARTICR_RXIC              (0x00000010UL)  /**< Receive Interrupt Clear */
#define UARTICR_TXIC              (0x00000020UL)  /**< Transmit Interrupt Clear */
#define UARTICR_RTIC              (0x00000040UL)  /**< Receive Timeout Interrupt Clear */
#define UARTICR_FEIC              (0x00000080UL)  /**< Framing Error Interrupt Clear */
#define UARTICR_PEIC              (0x00000100UL)  /**< Parity Error Interrupt Clear */
#define UARTICR_BEIC              (0x00000200UL)  /**< Break Error Interrupt Clear */
#define UARTICR_OEIC              (0x00000400UL)  /**< Overrun Error Interrupt Clear */

/* ===================[UARTDR (Data Register) Bits]=================== */
#define UARTDR_DATA_MASK          (0x000000FFUL)  /**< Data Mask */
#define UARTDR_FE                 (0x00000100UL)  /**< Framing Error */
#define UARTDR_PE                 (0x00000200UL)  /**< Parity Error */
#define UARTDR_BE                 (0x00000400UL)  /**< Break Error */
#define UARTDR_OE                 (0x00000800UL)  /**< Overrun Error */

#endif /* MCAL_UART_UART_REGS_H_ */
