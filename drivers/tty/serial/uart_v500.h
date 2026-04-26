/*
 * Driver for UART V500 serial ports
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
#ifndef __UART_V500_H_
#define __UART_V500_H_
#include "uart_v500_console.h"

#define A53_CLUSTER0_CPU1                 1
#define FIFO_SIZE_UART_V500               64
#define UART_V500_DR_OE                   (1 << 11)
#define UART_V500_DR_BE                   (1 << 10)
#define UART_V500_DR_PE                   (1 << 9)
#define UART_V500_DR_FE                   (1 << 8)
#define UART_DUMMY_DR_RX                  (1 << 16)
#define UART_TX_WORDS                     0xFF
#define FIFO_MAX_COUNT                    256
#define UART_V500_DR_ERROR                (UART_V500_DR_OE | UART_V500_DR_BE | \
	UART_V500_DR_PE | UART_V500_DR_FE)

#define UART_V500_INT_STAUS               (UART_V500_INT_OES | \
	UART_V500_INT_BES | UART_V500_INT_PES | UART_V500_INT_FES | \
	UART_V500_INT_RTS | UART_V500_INT_RXS)

#define UART_V500_INT_MASK                (UART_V500_INT_RXM | \
	UART_V500_INT_RTM | UART_V500_INT_FEM | UART_V500_INT_PEM | \
	UART_V500_INT_BEM | UART_V500_INT_OEM)

unsigned int uart_v500_reg_to_offset(
	const struct uart_v500_port *uap, unsigned int reg);

static struct vendor_data g_vendor_uart_v500 = {
	.reg_offset = g_uart_v500_std_offsets,
	.fifo_cfg = UART_V500_FIFO_RX_1_4 | UART_V500_FIFO_TX_1_2 |
		UART_V500_FIFO_RTS_7_8,
	.stat_busy = UART_V500_STAT_TX_BUSY,
	.stat_cts = UART_V500_STAT_CTS,
	.access_32b = true,
};

#endif
