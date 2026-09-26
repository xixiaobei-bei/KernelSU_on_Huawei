/*
 * Synopsys DesignWare I3C adapter head File(master only).
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

#ifndef DW_I3C_V100_H
#define DW_I3C_V100_H

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/i3c/master.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/iopoll.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/slab.h>
#include <linux/overflow.h>
#include <linux/printk.h>
#include <linux/pinctrl/consumer.h>

#define I3C_COMMAND_QUEUE_PORT 0xC
#define I3C_RX_TX_DATA_PORT 0x14
#define RESET_CTRL 0x34
#define RESET_CTRL_IBI_QUEUE BIT(5)
#define RESET_CTRL_RX_FIFO BIT(4)
#define RESET_CTRL_TX_FIFO BIT(3)
#define RESET_CTRL_RESP_QUEUE BIT(2)
#define RESET_CTRL_CMD_QUEUE BIT(1)
#define RESET_CTRL_SOFT BIT(0)

#define I3C_DATA_BUFFER_STATUS_LEVEL 0x50
#define I3C_PRESENT_STATE 0x54
#define I3C_PIN_STATUS_OK 0x3

#define HISI_MAX_DEVS 32

#define SCL_HCNT_TIMING 0xb4
#define SCL_LCNT_TIMING 0xb8
#define I3C_OD_HCNT_START  8
#define I3C_PP_HCNT_START  16
#define I3C_OD_LCNT_START  16
#define I3C_PP_LCNT_START  24
#define I3C_OD_PP_MAX_VAL 0xFF
#define I3C_OD_LCNT_MAX_VAL 0xFFFF
#define I3C_I2C_OD_HCNT_MASK 0xFFFFFF00
#define I3C_I2C_OD_LCNT_MASK 0xFFFF0000

#define I3C_BUS_I2C_FM_LOW_MIN_NS    1300
#define I3C_BUS_I2C_FM_HIGH_MIN_NS   600
#define I3C_BUS_I2C_FMP_TLOW_MIN_NS  500
#define I3C_BUS_I2C_FMP_THIGH_MIN_NS 260
#define I3C_BUS_I3C_SDR_LOW_MIN_NS 40
#define I3C_BUS_I3C_SDR_HIGH_MIN_NS 41
#define I3C_BUS_I3C_OD_MIN_NS 460
#define I3C_DEV_ADDR_TABLE  0x300
#define I3C_REG_LENGTH  4
#define I3C_MAX_SLAVE_NUM 8
#define I3C_RESUME_INIT 1
#define I3C_USE_PINCTRL 1
#define I3C_NOT_USE_PINCTRL 0

#define I3C_TIMEOUT_VALUE       200000
#define I3C_BYTES_OF_WORD       4
#define I3C_BYTE_MASK           0xFF
#define I3C_WRITE_BUF_STAT_REG_MASK 0x1F
#define I3C_DATA_BUFF_STATUS_RX_START 8
#define I3C_TRANSFER_ONE_WORLD 1
#define I3C_TRANSFER_REMAINDER 3

#define I3C_QUEUE_THLD_CTRL 0x1c
#define I3C_QUEUE_THLD_CTRL_RESP_BUF_MASK GENMASK(15, 8)
#define i3c_queue_thld_ctrl_resp_buf(x) (((x) - 1) << 8)

#define I3C_DATA_BUFFER_THLD_CTRL      0x20
#define I3C_RX_BUF_THLD 0x1
#define I3C_RX_BUF_THLD_START 8
#define I3C_TX_EMPTY_BUF_THLD 0x1

#define I3C_INTR_STATUS                0x3c
#define I3C_INTR_STATUS_EN             0x40
#define I3C_INTR_SIGNAL_EN             0x44
#define I3C_INTR_FORCE                 0x48
#define I3C_INTR_BUSOWNER_UPDATE_STAT  BIT(13)
#define I3C_INTR_IBI_UPDATED_STAT      BIT(12)
#define I3C_INTR_READ_REQ_RECV_STAT    BIT(11)
#define I3C_INTR_DEFSLV_STAT           BIT(10)
#define I3C_INTR_TRANSFER_ERR_STAT     BIT(9)
#define I3C_INTR_DYN_ADDR_ASSGN_STAT   BIT(8)
#define I3C_INTR_CCC_UPDATED_STAT      BIT(6)
#define I3C_INTR_TRANSFER_ABORT_STAT   BIT(5)
#define I3C_INTR_RESP_READY_STAT       BIT(4)
#define I3C_INTR_CMD_QUEUE_READY_STAT  BIT(3)
#define I3C_INTR_IBI_THLD_STAT         BIT(2)
#define I3C_INTR_RX_THLD_STAT          BIT(1)
#define I3C_INTR_TX_THLD_STAT          BIT(0)
#define I3C_INTR_ALL   (I3C_INTR_BUSOWNER_UPDATE_STAT | \
		I3C_INTR_IBI_UPDATED_STAT     | \
		I3C_INTR_READ_REQ_RECV_STAT   | \
		I3C_INTR_DEFSLV_STAT          | \
		I3C_INTR_TRANSFER_ERR_STAT    | \
		I3C_INTR_DYN_ADDR_ASSGN_STAT  | \
		I3C_INTR_CCC_UPDATED_STAT     | \
		I3C_INTR_TRANSFER_ABORT_STAT  | \
		I3C_INTR_RESP_READY_STAT      | \
		I3C_INTR_CMD_QUEUE_READY_STAT | \
		I3C_INTR_IBI_THLD_STAT        | \
		I3C_INTR_TX_THLD_STAT         | \
		I3C_INTR_RX_THLD_STAT)

#define I3C_INTR_MASTER_MASK (I3C_INTR_TRANSFER_ERR_STAT | \
	I3C_INTR_RESP_READY_STAT)

#define I3C_INT_TRANSFER_WRITE (I3C_INTR_RESP_READY_STAT | \
	I3C_INTR_TX_THLD_STAT | I3C_INTR_TRANSFER_ERR_STAT)
#define I3C_INT_TRANSFER_READ (I3C_INTR_TRANSFER_ERR_STAT | \
	I3C_INTR_RESP_READY_STAT | I3C_INTR_RX_THLD_STAT)

#define I3C_TRANSFER_MODE_POLL  0
#define I3C_TRANSFER_MODE_IRQ   1

#define I3C_RESET_REG_CNT 4
#define I3C_WAIT_RESET_STATUS_TIMES 10

struct dw_i3c_master_caps {
	u8 cmdfifodepth;
	u8 datafifodepth;
};

struct dw_i3c_cmd {
	u32 cmd_lo;
	u32 cmd_hi;
	u16 tx_len;
	const void *tx_buf;
	u16 rx_len;
	void *rx_buf;
	u8 error;
	u8 read_finish;
};

struct dw_i3c_xfer {
	struct list_head node;
	struct completion comp;
	int ret;
	unsigned int ncmds;
	struct dw_i3c_cmd cmds[0];
};

struct i3c_priv_data {
	u32 reset_enable;
	u32 reset_disable;
	u32 reset_status;
	u32 reset_bit;
};

struct dw_i3c_master {
	struct i3c_master_controller base;
	u16 maxdevs;
	u16 datstartaddr;
	u32 free_pos;
	struct {
		struct list_head list;
		struct dw_i3c_xfer *cur;
		spinlock_t lock;
	} xferqueue;
	struct dw_i3c_master_caps caps;
	void __iomem *regs;
	struct reset_control *core_rst;
	struct clk *core_clk;
	char version[5];
	char type[5];
	u8 addrs[HISI_MAX_DEVS];
	struct pinctrl  *pinctrl;
	int  pinctrl_flag;
	struct mutex lock;
	int i3c_resume_init;
	int i2c_slave_addr[I3C_MAX_SLAVE_NUM];
	unsigned int trans_mode;
	int irq;
	unsigned int i2c_scl_hz;
	void __iomem  *reset_reg_base;
	struct i3c_priv_data  priv;
};

void dw_i3c_master_start_xfer_locked(struct dw_i3c_master *master);
void dw_i3c_master_end_xfer_locked(struct dw_i3c_master *master, u32 isr);
inline struct dw_i3c_master *
to_dw_i3c_master(struct i3c_master_controller *master);
int dw_i3c_master_bus_init(struct i3c_master_controller *m);
void dw_i3c_master_bus_cleanup(struct i3c_master_controller *m);
int dw_i3c_master_attach_i3c_dev(struct i3c_dev_desc *dev);
int dw_i3c_master_reattach_i3c_dev(struct i3c_dev_desc *dev,
	u8 old_dyn_addr);
void dw_i3c_master_detach_i3c_dev(struct i3c_dev_desc *dev);
int dw_i3c_master_daa(struct i3c_master_controller *m);
int dw_i3c_master_send_ccc_cmd(struct i3c_master_controller *m,
	struct i3c_ccc_cmd *ccc);
int dw_i3c_master_priv_xfers(struct i3c_dev_desc *dev,
	struct i3c_priv_xfer *i3c_xfers, int i3c_nxfers);
int dw_i3c_master_attach_i2c_dev(struct i2c_dev_desc *dev);
void dw_i3c_master_detach_i2c_dev(struct i2c_dev_desc *dev);
int dw_i3c_master_i2c_xfers(struct i2c_dev_desc *dev,
	const struct i2c_msg *i2c_xfers, int i2c_nxfers);

void reset_i3c_controller(struct dw_i3c_master *master);
void dw_i3c_master_wr_tx_fifo(struct dw_i3c_master *master);
void dw_i3c_master_read_rx_fifo(struct dw_i3c_master *master);
void hs_dw_i3c_irq_handler(struct dw_i3c_master *master, u32 status);
void hs_dw_i3c_master_bus_cleanup(struct i3c_master_controller *m);
int hs_dw_i3c_master_attach_i3c_dev(struct i3c_dev_desc *dev);
int hs_dw_i3c_master_reattach_i3c_dev(struct i3c_dev_desc *dev,
	u8 old_dyn_addr);
void hs_dw_i3c_master_detach_i3c_dev(struct i3c_dev_desc *dev);
int hs_dw_i3c_master_daa(struct i3c_master_controller *m);
int hs_dw_i3c_master_send_ccc_cmd(struct i3c_master_controller *m,
	struct i3c_ccc_cmd *ccc);
int hs_dw_i3c_master_priv_xfers(struct i3c_dev_desc *dev,
	struct i3c_priv_xfer *i3c_xfers, int i3c_nxfers);
int hs_dw_i3c_master_attach_i2c_dev(struct i2c_dev_desc *dev);
void hs_dw_i3c_master_detach_i2c_dev(struct i2c_dev_desc *dev);
int hs_dw_i3c_master_i2c_xfers(struct i2c_dev_desc *dev,
	const struct i2c_msg *i2c_xfers, int i2c_nxfers);

int hs_dw_i3c_clk_cfg(struct dw_i3c_master *master);
int hs_dw_i2c_clk_cfg(struct dw_i3c_master *master);
int hs_dw_i3c_init(struct platform_device *pdev,
	struct dw_i3c_master *master);
int hs_dw_i3c_suspend(struct device *dev);
int hs_dw_i3c_resume(struct device *dev);

#endif
