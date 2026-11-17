/*
 * sc200x_protocol.h
 *
 * sc200x protocol header file
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _SC200X_PROTOCOL_H_
#define _SC200X_PROTOCOL_H_

#include "sc200x.h"

struct sc200x_accp_msg {
	unsigned char cmd;
	unsigned char reg_addr;
	unsigned char data_len;
	unsigned char data;
};

struct sc200x_reg_msg {
	unsigned char accp_status;
	unsigned char dev_type;
	unsigned char accp_ack;
	unsigned char data;
};

#define SC200X_ACCP_RETRY_MAX_TIMES     3 /* accp retry max times */
#define SC200X_ACCP_RSP_TIMEOUT         50 /* 50 ms */
#define SC200X_ACCP_DETECT_TIMEOUT      2500 /* 2500 ms */
#define SC200X_ACCP_RST_ADPT_DELAY      10 /* 10 ms */
#define SC200X_ACCP_RST_MSTR_DELAY      12 /* 12 ms */

/* reg=0x00, rw, accp command for transmission */
#define SC200X_REG_ACCP_CMD             0x00

#define SC200X_ACCP_CMD_SBRRD           0x0C
#define SC200X_ACCP_CMD_SBRWR           0x0B

/* reg=0x01, rw, accp address for transmission */
#define SC200X_REG_ACCP_ADDR            0x01

/* reg=0x02, rw, accp data length for transmission */
#define SC200X_REG_ACCP_DATA_LEN        0x02

/* reg=0x03, rw, accp tx data */
#define SC200X_REG_ACCP_TX_DATA         0x03

/* reg=0x10, r, device status */
#define SC200X_REG_DEV_STATUS           0x10

#define SC200X_DVC_MASK                 (BIT(7) | BIT(6))
#define SC200X_DVC_SHIFT                6
#define SC200X_ACCP_RSP_STA_MASK        (BIT(5) | BIT(4) | BIT(3))
#define SC200X_ACCP_RSP_STA_SHIFT       3
#define SC200X_DEVICE_READY_MASK        BIT(0)
#define SC200X_DEVICE_READY_SHIFT       0

#define SC200X_DVC_ACCP_NOT_DETECT      0x00 /* accp detectin not started */
#define SC200X_DVC_NO_VALID_DEV_DET     0x01 /* no vaild device detected */
#define SC200X_DVC_ACCP_DETECTED        0x03 /* accp detected */

#define SC200X_ACCP_ACK                 0x01
#define SC200X_ACCP_NACK                0x02
#define SC200X_ACCP_NORSP               0x03
#define SC200X_ACCP_CRCPAR              0x04

/* reg=0x12, r, accp ack value */
#define SC200X_REG_ACCP_ACK             0x12

/* reg=0x13, r, accp response data */
#define SC200X_REG_ACCP_RX_DATA         0x13

int sc200x_protocol_init(struct sc200x_device_info *di);
int sc200x_accp_alert_handler(struct sc200x_device_info *di, unsigned int alert);

#endif /* _SC200X_PROTOCOL_H_ */
