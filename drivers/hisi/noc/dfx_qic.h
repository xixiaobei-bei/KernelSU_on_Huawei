/*
 *
 * QIC Mntn Module.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __DFX_QIC_H
#define __DFX_QIC_H

#include <linux/platform_device.h>

#define QIC_MODULE_NAME  "DFX_QIC"
#define MAX_QIC_NODE 8
#define MAX_QIC_NODE_NAME_LEN 24
#define MAX_QIC_CLK_NAME_LEN 16
#define MAX_QIC_REG 32
#define IB_DATA_CMD_START 20
#define IB_ADDR_MASK 0xFFFFFFFF00000
#define QTP_RESP_MASK 0x7
#define IB_TYPE_CFG 1
#define IB_TYPE_DATA 2
#define IRQ_CTRL_OFFSET 0x600
#define QIC_CLOCK_NUM  4
#define MAX_QIC_BUS_NUM  3
#define MASK_UNSIGNED_INT 0xFFFFFFFFUL
#define QIC_HIGH_ENABLE_MASK 16
#define MAX_QIC_GIVEUP_IDLE_NUM 5
#define DFX_QIC_CLOCK_REG_DEFAULT 0xFFFFFFFF
#define VOTE_POWER_ON 0ul
#define VOTE_POWER_DOWN 1ul
#define DIVIDED_NUMBER 0x19283746

struct dfx_qic_device {
	struct device *dev;
	void __iomem *pericrg_base;
	void __iomem *pmctrl_base;
	unsigned int qic_irq;
	unsigned int bus_num;
};

struct qic_clk {
	unsigned int clock;
	unsigned int mask;
};

struct qic_pwidle {
	unsigned int idle_req;
	unsigned int idle_status;
};

struct dfx_qic_clk {
	char *bus_name;
	unsigned int bus_id;
	void __iomem *reg_base;
	unsigned int pwidle_bit;
	struct qic_clk crg_clk[QIC_CLOCK_NUM];
	struct qic_clk bus_clk[QIC_CLOCK_NUM];
	struct qic_pwidle pwidle_offset;
	unsigned int dump_reg_nums;
	unsigned int *reg_offset; /* need dump reg offset */
	unsigned int fid_val;
};

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

struct dfx_qic_regs {
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
	unsigned int bus_id;
	unsigned int offset;
	unsigned int irq_num;
	unsigned int irq_mask;
	unsigned int *irq_bit;
	unsigned int *irq_type;
	void __iomem *qic_base[MAX_QIC_REG];
	unsigned int *ib_type;
	unsigned int *cmd_id;
	unsigned int *dst_id;
	unsigned int *qic_coreid;
	struct dfx_qic_clk *qic_clock;
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
	QIC_IB_ERR_IRQ,     /* 0 */
	QIC_TB0_IRQ,		 /* 1 */
	QIC_TB1_IRQ,		 /* 2 */
};

/* NOTE: Must the same order with DTS */
enum QIC_REG_BASE {
	PERI_CRG_BASE,
	PMCTRL_BASE,
	QIC_MAX_BASE,
};

#define QIC_REGISTER_LIST_MAX_LENGTH 512
#define QIC_MODID_EXIST 0x1
#define QIC_MODID_NEGATIVE 0xFFFFFFFF
#define QIC_MODID_NUM_MAX 0x6

struct qic_coreid_modid_trans_s {
	struct list_head s_list;
	unsigned int qic_coreid;
	unsigned int modid;
};

struct dfx_qic_clk **get_qic_clk_info(void);
struct dfx_qic_device *get_qic_dev(void);
int dfx_qic_check_crg_status(struct dfx_qic_clk *qic_clk_s);
int dfx_qic_dump_init(void);
int dfx_qic_vote_power_on(struct dfx_qic_clk *qic_clk_s);
void dfx_qic_vote_power_down(struct dfx_qic_clk *qic_clk_s);
#endif
