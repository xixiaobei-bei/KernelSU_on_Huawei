/*
 * The register defination for UART V500 serial ports
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __UART_V500_REG_H_
#define __UART_V500_REG_H_

/* UART V500 IP Register */
#define UART_BAUD_CFG              0x000
#define UART_FRAME_CFG             0x004
#define UART_FIFO_CFG              0x008 /* Interrupt fifo level select. */
#define UART_TX_FIFO               0x00C /* Data written to the interface */
#define UART_RX_FIFO               0x010 /* Data read from the interface */
#define UART_STAT                  0x014
#define UART_INT_CLR               0x018
#define UART_INT_MSK               0x01C
#define UART_INT_RAW               0x020
#define UART_INT_STAT              0x024
#define UART_CONFIG                0x028
#define UART_ID                    0x100

#define UART_BAUD_INT_MASK         0xFFFF
#define UART_BAUD_FRAC_MASK        0x3F

#define UART_V500_CFG_CTS_EN       0x00080008 /* CTS hardware control enable */
#define UART_V500_CFG_RTS_EN       0x00100010 /* RTS hardware control enable */
#define UART_V500_CFG_LOOP_EN      0x02000200 /* loopback enable */
#define UART_V500_CFG_UART_EN      0x00010001 /* UART enable */
#define UART_V500_CFG_RX_EN        0x00040004 /* receive enable */
#define UART_V500_CFG_TX_EN        0x00020002 /* transmit enable */
#define UART_V500_CFG_BRK_EN       0x00800080 /* break request enable */
#define UART_V500_TX_DMA_EN        0x00200020 /* enable transmit dma */
#define UART_V500_RX_DMA_EN        0x00400040 /* enable receive dma */

#define UART_V500_CFG_CTS_DIS      0x00080000 /* CTS hardware control disable */
#define UART_V500_CFG_RTS_DIS      0x00100000 /* RTS hardware control disable */
#define UART_V500_CFG_LOOP_DIS     0x02000000 /* loopback disable */
#define UART_V500_CFG_UART_DIS     0x00010000 /* UART disable */
#define UART_V500_CFG_RX_DIS       0x00040000 /* receive disable */
#define UART_V500_CFG_TX_DIS       0x00020000 /* transmit disable */
#define UART_V500_CFG_BRK_DIS      0x00800000 /* break request disable */
#define UART_V500_TX_DMA_DIS       0x00200000 /* disable transmit dma */
#define UART_V500_RX_DMA_DIS       0x00400000 /* disable receive dma */

#define UART_V500_CFG_DIS_ALL      0xFFFF0000 /* disable all UART config */

#define UART_V500_STAT_TX_BUSY      0x001
#define UART_V500_STAT_RX_BUSY      0x002
#define UART_V500_STAT_TXFE         0x004
#define UART_V500_STAT_TXFF         0x008
#define UART_V500_STAT_RXFE         0x010
#define UART_V500_STAT_RXFF         0x020
#define UART_V500_STAT_CTS          0x040

#define UART_V500_FRAME_LEN_5       0x00
#define UART_V500_FRAME_LEN_6       0x01
#define UART_V500_FRAME_LEN_7       0x02
#define UART_V500_FRAME_LEN_8       0x03
#define UART_FRAME_DATA_BIT         0x03
#define UART_V500_FRAME_STOP        0x20
#define UART_V500_FRAME_PARITY      0x1C
#define UART_V500_FRAME_PES         0x10
#define UART_V500_FRAME_POS         0x14
#define UART_V500_FRAME_NPS         (1 << 4)

#define UART_V500_FIFO_TX_1_8       (0 << 0)
#define UART_V500_FIFO_TX_1_4       (1 << 0)
#define UART_V500_FIFO_TX_1_2       (2 << 0)
#define UART_V500_FIFO_TX_3_4       (3 << 0)
#define UART_V500_FIFO_TX_7_8       (4 << 0)

#define UART_V500_FIFO_RX_1_8       (0 << 3)
#define UART_V500_FIFO_RX_1_4       (1 << 3)
#define UART_V500_FIFO_RX_1_2       (2 << 3)
#define UART_V500_FIFO_RX_3_4       (3 << 3)
#define UART_V500_FIFO_RX_7_8       (4 << 3)

#define UART_V500_FIFO_RTS_1_8      (0 << 6)
#define UART_V500_FIFO_RTS_1_4      (1 << 6)
#define UART_V500_FIFO_RTS_1_2      (2 << 6)
#define UART_V500_FIFO_RTS_3_4      (3 << 6)
#define UART_V500_FIFO_RTS_7_8      (4 << 6)

#define UART_V500_INT_CTSS      (1 << 7) /* cts interrupt status */
#define UART_V500_INT_OES       (1 << 6) /* overrun error interrupt status */
#define UART_V500_INT_BES       (1 << 5) /* break error interrupt status */
#define UART_V500_INT_PES       (1 << 4) /* parity error interrupt status */
#define UART_V500_INT_FES       (1 << 3) /* framing error interrupt status */
#define UART_V500_INT_RTS       (1 << 2) /* receive timeout interrupt status */
#define UART_V500_INT_TXS       (1 << 1) /* transmit interrupt status */
#define UART_V500_INT_RXS       (1 << 0) /* receive interrupt status */

#define UART_V500_INT_CTSM      (1 << 7) /* cts interrupt mask */
#define UART_V500_INT_OEM       (1 << 6) /* overrun error interrupt mask */
#define UART_V500_INT_BEM       (1 << 5) /* break error interrupt mask */
#define UART_V500_INT_PEM       (1 << 4) /* parity error interrupt mask */
#define UART_V500_INT_FEM       (1 << 3) /* framing error interrupt mask */
#define UART_V500_INT_RTM       (1 << 2) /* receive timeout interrupt mask */
#define UART_V500_INT_TXM       (1 << 1) /* transmit interrupt mask */
#define UART_V500_INT_RXM       (1 << 0) /* receive interrupt mask */

#define UART_V500_INT_CTSC      (1 << 7) /* cts interrupt clear */
#define UART_V500_INT_OEC       (1 << 6) /* overrun error interrupt clear */
#define UART_V500_INT_BEC       (1 << 5) /* break error interrupt clear */
#define UART_V500_INT_PEC       (1 << 4) /* parity error interrupt clear */
#define UART_V500_INT_FEC       (1 << 3) /* framing error interrupt clear */
#define UART_V500_INT_RTC       (1 << 2) /* receive timeout interrupt clear */
#define UART_V500_INT_TXC       (1 << 1) /* transmit interrupt clear */
#define UART_V500_INT_RXC       (1 << 0) /* receive interrupt clear */

#define UART_V500_MSK_INT_ALL           0xFFFF
#define UART_V500_CLR_INT_ALL           0xFFFF

enum {
	REG_UART_BAUD,
	REG_UART_FRAME,
	REG_UART_FIFO_CFG,
	REG_UART_TX_FIFO,
	REG_UART_RX_FIFO,
	REG_UART_STAT,
	REG_UART_INT_CLR,
	REG_UART_INT_MSK,
	REG_UART_INT_RAW,
	REG_UART_INT_STAT,
	REG_UART_CONFIG,
	REG_UART_ID,
	REG_ARRAY_SIZE,
};

static unsigned int g_uart_v500_std_offsets[REG_ARRAY_SIZE] = {
	[REG_UART_BAUD]     = UART_BAUD_CFG,
	[REG_UART_FRAME]    = UART_FRAME_CFG,
	[REG_UART_FIFO_CFG] = UART_FIFO_CFG,
	[REG_UART_TX_FIFO]  = UART_TX_FIFO,
	[REG_UART_RX_FIFO]  = UART_RX_FIFO,
	[REG_UART_STAT]     = UART_STAT,
	[REG_UART_INT_CLR]  = UART_INT_CLR,
	[REG_UART_INT_MSK]  = UART_INT_MSK,
	[REG_UART_INT_RAW]  = UART_INT_RAW,
	[REG_UART_INT_STAT] = UART_INT_STAT,
	[REG_UART_CONFIG]   = UART_CONFIG,
	[REG_UART_ID]       = UART_ID,
};

#endif
