/*
 * QIC. (QIC Mntn Module.)
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __HISI_QIC_H
#define __HISI_QIC_H

#define MAX_QIC_NODE 8
#define MAX_QIC_NODE_NAME_LEN 24
#define MAX_QIC_REG 32
#define IB_DATA_CMD_START 20
#define IB_ADDRESS_MASK 0xFFFFFFFF00000
#define QTP_RESP_MASK 0x7
#define IB_TYPE_CFG 1
#define IB_TYPE_DATA 2
#define IRQ_CTRL_OFFSET 0x600

struct qic_ib_reg {
	unsigned int ib_dfx_int_status;
	unsigned int ib_dfx_int_clr;
	unsigned int ib_dfx_err_inf0;
	unsigned int ib_dfx_err_inf1;
	unsigned int ib_dfx_err_inf2;
	unsigned int ib_dfx_err_inf3;
	unsigned int ib_dfx_module;
	unsigned int ib_dfx_bp;
};

struct qic_tb0_reg {
	unsigned int tb_int_status;
	unsigned int dfx_tb_int_clr;
	unsigned int dfx_tb_bp;
	unsigned int dfx_tb_module;
	unsigned int dfx_tb_data_buf;
	unsigned int dfx_tb_fifo;
	unsigned int dfx_tb_tx_crdt;
	unsigned int dfx_tb_lpc_st;
	unsigned int dfx_tb_arb_st;
	unsigned int dfx_tb_id_st0;
	unsigned int dfx_tb_id_st1;
};

struct qic_tb1_reg {
	unsigned int tb1_dfx_stat0;
	unsigned int tb1_fc_in_stat0;
	unsigned int tb1_qtp_dec_stat0;
	unsigned int tb1_qtp_enc_stat0;
	unsigned int tb1_fc_out_stat0;
};

struct hisi_qic_regs {
	struct qic_ib_reg *qic_ib_offset;
	struct qic_tb0_reg *qic_tb0_offset;
	struct qic_tb1_reg *qic_tb1_offset;
	unsigned int qtp_rop_width;
	unsigned int qtp_portnum_width;
	unsigned int qtp_vcnum_width;
};

struct qic_node {
	char *name[MAX_QIC_REG];
	char *bus_name;
	unsigned int offset;
	unsigned int irq_num;
	unsigned int irq_mask;
	unsigned int *irq_bit;
	unsigned int *irq_type;
	void __iomem *qic_base[MAX_QIC_REG];
	unsigned int *ib_type;
	unsigned int *cmd_id;
	unsigned int *dst_id;
};

struct qic_tb0_int_type {
	unsigned int mask;
	unsigned int val;
	char *int_type;
};

struct qic_tb1_dfx_stat {
	unsigned int mask;
	unsigned int val;
	char *dfx_stat;
};

enum QIC_IRQ_TPYE {
	QIC_IB_ERR_IRQ,     /*0*/
	QIC_TB0_IRQ,		/*1*/
	QIC_TB1_IRQ,		/*2*/
};

int hisi_qic_init(void);
unsigned int hisi_qic_interrupt_hanlder(void);
void free_qic_source(void);
#endif
