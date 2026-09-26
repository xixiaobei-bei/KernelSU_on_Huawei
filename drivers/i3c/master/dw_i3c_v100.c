/*
 * Synopsys DesignWare I3C adapter driver (master only).
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

#include "dw_i3c_v100.h"
#include <securec.h>

int hs_dw_i3c_clk_cfg(struct dw_i3c_master *master)
{
	u32 i3c_pp_cnt, i3c_pp_lcnt, i3c_pp_hcnt;
	u32 i3c_od_cnt, i3c_od_lcnt, i3c_od_hcnt;
	unsigned long core_rate;

	if ((!master) || (!master->core_clk)) {
		pr_err("i3c: i3c master doesn't be init\n");
		return -EINVAL;
	}

	core_rate = clk_get_rate(master->core_clk);
	if (!core_rate)
		return -EINVAL;

	i3c_pp_cnt = DIV_ROUND_UP(core_rate, I3C_BUS_I2C_FM_PLUS_SCL_RATE) + 1;
	i3c_od_cnt = DIV_ROUND_UP(core_rate, I3C_BUS_I2C_FM_SCL_RATE);

	/* pp_cnt 50% duty */
	i3c_pp_hcnt = i3c_pp_cnt / 2;
	if (i3c_pp_hcnt > I3C_OD_PP_MAX_VAL)
		i3c_pp_hcnt = I3C_OD_PP_MAX_VAL;

	i3c_pp_lcnt = i3c_pp_cnt - i3c_pp_hcnt;
	if (i3c_pp_lcnt > I3C_OD_PP_MAX_VAL)
		i3c_pp_lcnt = I3C_OD_PP_MAX_VAL;
	/* od_cnt 50% duty */
	i3c_od_hcnt = i3c_od_cnt / 2;
	if (i3c_od_hcnt > I3C_OD_PP_MAX_VAL)
		i3c_od_hcnt = I3C_OD_PP_MAX_VAL;

	i3c_od_lcnt = i3c_od_cnt - i3c_od_hcnt;
	if (i3c_od_lcnt > I3C_OD_PP_MAX_VAL)
		i3c_od_lcnt = I3C_OD_PP_MAX_VAL;
	/* PP_HCNT affects  (Push-Pull hcnt) */
	writel((i3c_pp_hcnt << I3C_PP_HCNT_START) |
		(i3c_od_hcnt << I3C_OD_HCNT_START) |
		(readl(master->regs + SCL_HCNT_TIMING)),
			master->regs + SCL_HCNT_TIMING);

	/* PP LCNT affects (Push-Pull lcnt) */
	writel((i3c_pp_lcnt << I3C_PP_LCNT_START) |
		(i3c_od_lcnt << I3C_OD_LCNT_START) |
		(readl(master->regs + SCL_LCNT_TIMING)),
			master->regs + SCL_LCNT_TIMING);

	return 0;
}

int hs_dw_i2c_clk_cfg(struct dw_i3c_master *master)
{
	u32 i2c_od_cnt, i2c_od_lcnt, i2c_od_hcnt, val;
	unsigned long core_rate;

	if ((!master) || (!master->core_clk)) {
		pr_err("i3c: i3c master doesn't be init\n");
		return -EINVAL;
	}

	core_rate = clk_get_rate(master->core_clk);
	if (!core_rate)
		return -EINVAL;

	if (master->i2c_scl_hz == I3C_BUS_I2C_FM_PLUS_SCL_RATE) {
		i2c_od_cnt = DIV_ROUND_UP(core_rate, I3C_BUS_I2C_FM_PLUS_SCL_RATE);
		i2c_od_hcnt = DIV_ROUND_UP((i2c_od_cnt * I3C_BUS_I2C_FMP_THIGH_MIN_NS),
			(I3C_BUS_I2C_FMP_THIGH_MIN_NS + I3C_BUS_I2C_FMP_TLOW_MIN_NS));
	} else {
		i2c_od_cnt = DIV_ROUND_UP(core_rate, I3C_BUS_I2C_FM_SCL_RATE);
		i2c_od_hcnt = DIV_ROUND_UP((i2c_od_cnt * I3C_BUS_I2C_FM_HIGH_MIN_NS),
			(I3C_BUS_I2C_FM_LOW_MIN_NS + I3C_BUS_I2C_FM_HIGH_MIN_NS));
	}

	if (i2c_od_hcnt > I3C_OD_PP_MAX_VAL)
		i2c_od_hcnt = I3C_OD_PP_MAX_VAL;

	i2c_od_lcnt = i2c_od_cnt - i2c_od_hcnt;
	if (i2c_od_lcnt > I3C_OD_LCNT_MAX_VAL)
		i2c_od_lcnt = I3C_OD_LCNT_MAX_VAL;

	/* OD_HCNT affects */
	val = readl(master->regs + SCL_HCNT_TIMING) & I3C_I2C_OD_HCNT_MASK;
	writel(i2c_od_hcnt | val, master->regs + SCL_HCNT_TIMING);
	/* OD LCNT affects */
	val = readl(master->regs + SCL_LCNT_TIMING) & I3C_I2C_OD_LCNT_MASK;
	writel(i2c_od_lcnt | val, master->regs + SCL_LCNT_TIMING);

	return 0;
}

static int hs_dw_i3c_pins_cfg(struct i3c_master_controller *controller,
	const char *name)
{
	struct dw_i3c_master *master = to_dw_i3c_master(controller);
	struct pinctrl_state *s = NULL;
	int ret;

	if (!master) {
		pr_err("i3c: i3c master doesn't be init\n");
		return -1;
	}

	if (!master->pinctrl_flag) {
		master->pinctrl = devm_pinctrl_get(controller->dev.parent);
		if (IS_ERR(master->pinctrl))
			return -1;
		master->pinctrl_flag = 1;
	}

	s = pinctrl_lookup_state(master->pinctrl, name);
	if (IS_ERR(s)) {
		devm_pinctrl_put(master->pinctrl);
		master->pinctrl_flag = 0;
		return -1;
	}

	ret = pinctrl_select_state(master->pinctrl, s);
	if (ret < 0) {
		devm_pinctrl_put(master->pinctrl);
		master->pinctrl_flag = 0;
		return -1;
	}
	return 0;
}

static void hs_i3c_dw_reset_controller(struct dw_i3c_master *master)
{
	struct i3c_priv_data *priv = NULL;
	u32 val = 0;
	int timeout = I3C_WAIT_RESET_STATUS_TIMES;

	priv = &master->priv;
	if (!priv) {
		pr_err("%s:i3c priv doesn't be init\n", __func__);
		return;
	}

	writel(BIT(priv->reset_bit),
		master->reset_reg_base + priv->reset_enable);
	do {
		val = readl(master->reset_reg_base + priv->reset_status);
		val &= BIT(priv->reset_bit);
		udelay(1);
	} while (!val && timeout--);
	if (timeout <= 0)
		pr_err("%s:i3c reset enable timeout\n", __func__);

	timeout = I3C_WAIT_RESET_STATUS_TIMES;

	writel(BIT(priv->reset_bit),
		master->reset_reg_base + priv->reset_disable);
	do {
		val = readl(master->reset_reg_base + priv->reset_status);
		val &= BIT(priv->reset_bit);
		udelay(1);
	} while (val && timeout--);
	if (timeout <= 0)
		pr_err("%s:i3c reset disable timeout\n", __func__);
}

static void hs_i3c_reinit(struct dw_i3c_master *master)
{
	int ret, i;

	for (i = 0; i < I3C_MAX_SLAVE_NUM; i++)
		master->i2c_slave_addr[i] = readl(master->regs +
			I3C_DEV_ADDR_TABLE + i * I3C_REG_LENGTH);

	hs_i3c_dw_reset_controller(master);

	master->i3c_resume_init = I3C_RESUME_INIT;
	ret = dw_i3c_master_bus_init(&master->base);
	if (ret)
		pr_err("dw_i3c_init failed\n");

	for (i = 0; i < I3C_MAX_SLAVE_NUM; i++)
		writel(master->i2c_slave_addr[i], master->regs +
			I3C_DEV_ADDR_TABLE + i * I3C_REG_LENGTH);
}

void reset_i3c_controller(struct dw_i3c_master *master)
{
	if (!master) {
		pr_err("%s:i3c master doesn't be init\n", __func__);
		return;
	}

	disable_irq(master->irq);

	hs_i3c_reinit(master);

	enable_irq(master->irq);
}

static int hs_dw_i3c_pin_check(struct i3c_master_controller *controller)
{
	struct dw_i3c_master *master = to_dw_i3c_master(controller);
	u32 status;

	status = readl(master->regs + I3C_PRESENT_STATE);
	if ((status & I3C_PIN_STATUS_OK) == I3C_PIN_STATUS_OK)
		return 0;

	if (hs_dw_i3c_pins_cfg(controller, PINCTRL_STATE_IDLE))
		pr_err("i3c: set pin idle failed\n");
	hs_i3c_reinit(master);
	if (hs_dw_i3c_pins_cfg(controller, PINCTRL_STATE_DEFAULT))
		pr_err("i3c: set pin default failed\n");
	status = readl(master->regs + I3C_PRESENT_STATE);
	if ((status & I3C_PIN_STATUS_OK) != I3C_PIN_STATUS_OK) {
		pr_err("i3c sda/scl status err 0x%x\n", status);
		return -1;
	}

	return 0;
}

static void hs_dw_i3c_flush(struct dw_i3c_master *master)
{
	u32 len, i;

	len = readl(master->regs + I3C_DATA_BUFFER_STATUS_LEVEL);
	len = I3C_BYTE_MASK &
		(len >> I3C_DATA_BUFF_STATUS_RX_START);
	if (len)
		pr_err("i3c: there has bad data,flush it\n");
	for (i = 0; i < len; i++)
		readl(master->regs + I3C_RX_TX_DATA_PORT);
}

static void hs_dw_i3c_master_wr_tx_fifo(struct dw_i3c_master *master,
	const u8 *bytes, u32 nbytes)
{
	u32 val, i, min_len, left;
	u32 words = nbytes / I3C_BYTES_OF_WORD;
	int times = I3C_TIMEOUT_VALUE;

	if (!bytes)
		return;

	while ((words > 0) && (times > 0)) {
		val =  readl(master->regs + I3C_DATA_BUFFER_STATUS_LEVEL);
		if ((val & I3C_WRITE_BUF_STAT_REG_MASK) > I3C_BYTES_OF_WORD) {
			min_len = val - I3C_BYTES_OF_WORD;
			min_len = (words > min_len ? min_len : words);
			for (i = 0; i < min_len; i++) {
				writesl(master->regs + I3C_RX_TX_DATA_PORT,
					bytes, I3C_TRANSFER_ONE_WORLD);
				bytes += I3C_BYTES_OF_WORD;
			}
			words -= min_len;
		}
		udelay(1);
		times--;
	}
	if (!times)
		pr_err("i3c write to controller timeout\n");

	left = nbytes & I3C_TRANSFER_REMAINDER;
	if (left) {
		u32 tmp = 0;

		if (memcpy_s(&tmp, left, bytes, left) != EOK) {
			pr_err("i3c write memcpy failed!\n");
			return;
		}
		writesl(master->regs + I3C_RX_TX_DATA_PORT, &tmp,
			I3C_TRANSFER_ONE_WORLD);
	}
}

static int dw_i3c_check_read_left_data(struct dw_i3c_master *master,
	u8 *bytes, u32 left)
{
	u32 temp = 0;
	u32 len;
	int times = I3C_TIMEOUT_VALUE;

	if (!bytes)
		return -1;

	do {
		len =  readl(master->regs +
			I3C_DATA_BUFFER_STATUS_LEVEL);
		len =  I3C_BYTE_MASK &
			(len >> I3C_DATA_BUFF_STATUS_RX_START);
		if (len) {
			readsl(master->regs + I3C_RX_TX_DATA_PORT,
				&temp, I3C_TRANSFER_ONE_WORLD);
			if (memcpy_s(bytes, left, &temp, left) != EOK) {
				pr_err("i3c read memcpy failed!\n");
				return -1;
			}
			break;
		}
		udelay(1);
	} while (times--);

	if (!times) {
		pr_err("i3c read to controller timeout\n");
		return -1;
	}

	return 0;
}

static void hs_dw_i3c_master_read_rx_fifo(struct dw_i3c_master *master,
	u8 *bytes, u32 nbytes)
{
	u32 len, i, left;
	u32 words = nbytes / I3C_BYTES_OF_WORD;
	int times = I3C_TIMEOUT_VALUE;

	if (!bytes)
		return;

	while ((words > 0) && (times > 0)) {
		len =  readl(master->regs + I3C_DATA_BUFFER_STATUS_LEVEL);
		len =  I3C_BYTE_MASK &
			(len >> I3C_DATA_BUFF_STATUS_RX_START);
		if (len > 0) {
			len = words > len ? len : words;
			for (i = 0; i < len; i++) {
				readsl(master->regs + I3C_RX_TX_DATA_PORT,
					bytes, I3C_TRANSFER_ONE_WORLD);
				bytes += I3C_BYTES_OF_WORD;
			}
			words -= len;
		}
		udelay(1);
		times--;
	}
	if (!times)
		pr_err("i3c read to controller timeout\n");

	left = nbytes & I3C_TRANSFER_REMAINDER;
	if (left) {
		if (dw_i3c_check_read_left_data(master, bytes, left))
			return;
	}
}

void dw_i3c_master_wr_tx_fifo(struct dw_i3c_master *master)
{
	struct dw_i3c_xfer *xfer = NULL;
	u32 i;

	if (!master) {
		pr_err("i3c: i3c master doesn't be init\n");
		return;
	}

	xfer = master->xferqueue.cur;
	if (!xfer)
		return;
	for (i = 0; i < xfer->ncmds; i++) {
		struct dw_i3c_cmd *cmd = &xfer->cmds[i];

		if (!cmd->tx_len)
			continue;
		hs_dw_i3c_master_wr_tx_fifo(master,
			cmd->tx_buf, cmd->tx_len);
	}
}

void dw_i3c_master_read_rx_fifo(struct dw_i3c_master *master)
{
	struct dw_i3c_xfer *xfer = NULL;
	u32 i;

	if (!master) {
		pr_err("i3c: i3c master doesn't be init\n");
		return;
	}

	xfer = master->xferqueue.cur;
	if (!xfer)
		return;
	for (i = 0; i < xfer->ncmds; i++) {
		struct dw_i3c_cmd *cmd = &xfer->cmds[i];

		if (!cmd->rx_len)
			continue;
		hs_dw_i3c_master_read_rx_fifo(master,
			cmd->rx_buf, cmd->rx_len);
		xfer->cmds[i].read_finish = 1;
	}
}

static void hs_i3c_master_start_irq_xfer(struct dw_i3c_master *master,
	struct dw_i3c_xfer *xfer)
{
	u32 int_enable = 0;
	unsigned int i;

	for (i = 0; i < xfer->ncmds; i++) {
		struct dw_i3c_cmd *cmd = &xfer->cmds[i];

		writel(cmd->cmd_hi, master->regs + I3C_COMMAND_QUEUE_PORT);
		writel(cmd->cmd_lo, master->regs + I3C_COMMAND_QUEUE_PORT);
		/* 4 words tx empty level; 4 words rx full level */
		writel(((I3C_RX_BUF_THLD << I3C_RX_BUF_THLD_START) |
			I3C_TX_EMPTY_BUF_THLD),
			master->regs + I3C_DATA_BUFFER_THLD_CTRL);
		if ((cmd->tx_buf) && (cmd->tx_len))
			int_enable |= I3C_INT_TRANSFER_WRITE;
		if ((cmd->rx_buf) && (cmd->rx_len)) {
			int_enable |= I3C_INT_TRANSFER_READ;
			cmd->read_finish = 0;
		}
	}

	writel(int_enable, master->regs + I3C_INTR_STATUS_EN);
	writel(int_enable, master->regs + I3C_INTR_SIGNAL_EN);
}

static void hs_i3c_master_start_poll_xfer(struct dw_i3c_master *master,
	struct dw_i3c_xfer *xfer)
{
	unsigned int i;

	for (i = 0; i < xfer->ncmds; i++) {
		struct dw_i3c_cmd *cmd = &xfer->cmds[i];

		writel(cmd->cmd_hi, master->regs + I3C_COMMAND_QUEUE_PORT);
		writel(cmd->cmd_lo, master->regs + I3C_COMMAND_QUEUE_PORT);
		if ((cmd->tx_buf) && (cmd->tx_len))
			hs_dw_i3c_master_wr_tx_fifo(master, cmd->tx_buf,
				cmd->tx_len);
		if ((cmd->rx_buf) && (cmd->rx_len))
			hs_dw_i3c_master_read_rx_fifo(master, cmd->rx_buf,
				cmd->rx_len);
	}
}

void dw_i3c_master_start_xfer_locked(struct dw_i3c_master *master)
{
	struct dw_i3c_xfer *xfer = NULL;
	u32 thld_ctrl;

	if (!master) {
		pr_err("i3c: i3c master doesn't be init\n");
		return;
	}
	xfer = master->xferqueue.cur;
	if (!xfer)
		return;

	hs_dw_i3c_flush(master);

	thld_ctrl = readl(master->regs + I3C_QUEUE_THLD_CTRL);
	thld_ctrl &= ~I3C_QUEUE_THLD_CTRL_RESP_BUF_MASK;
	thld_ctrl |= i3c_queue_thld_ctrl_resp_buf(xfer->ncmds);
	writel(thld_ctrl, master->regs + I3C_QUEUE_THLD_CTRL);

	if (master->trans_mode == I3C_TRANSFER_MODE_IRQ)
		hs_i3c_master_start_irq_xfer(master, xfer);
	else
		hs_i3c_master_start_poll_xfer(master, xfer);
}

void hs_dw_i3c_irq_handler(struct dw_i3c_master *master, u32 status)
{
	u32 val;

	if (!master) {
		pr_err("i3c: i3c master doesn't be init\n");
		return;
	}

	val = readl(master->regs + I3C_INTR_SIGNAL_EN);
	if ((status & I3C_INTR_TX_THLD_STAT) && (I3C_INTR_TX_THLD_STAT & val)) {
		val &= ~(I3C_INTR_TX_THLD_STAT);
		writel(val, master->regs + I3C_INTR_SIGNAL_EN);
		dw_i3c_master_wr_tx_fifo(master);
	}

	if ((status & I3C_INTR_RX_THLD_STAT) && (I3C_INTR_RX_THLD_STAT & val)) {
		val &= ~(I3C_INTR_RX_THLD_STAT);
		writel(val, master->regs + I3C_INTR_SIGNAL_EN);
		dw_i3c_master_read_rx_fifo(master);
	}

	if (status & I3C_INTR_MASTER_MASK)
		dw_i3c_master_end_xfer_locked(master, status);
}

/* hs_dw_i3c_hardware_cfg must use with hs_dw_i3c_hardware_disable */
static void hs_dw_i3c_hardware_cfg(struct i3c_master_controller *controller,
	int flag)
{
	struct dw_i3c_master *master = to_dw_i3c_master(controller);

	mutex_lock(&master->lock);
	if (flag) {
		if (hs_dw_i3c_pins_cfg(controller, PINCTRL_STATE_DEFAULT))
			pr_err("i3c: set pin default failed\n");
	}
}

/* hs_dw_i3c_hardware_disable must use with hs_dw_i3c_hardware_cfg */
static void hs_dw_i3c_hardware_disable(struct i3c_master_controller *controller,
	int flag)
{
	struct dw_i3c_master *master = to_dw_i3c_master(controller);

	if (flag) {
		if (hs_dw_i3c_pins_cfg(controller, PINCTRL_STATE_IDLE))
			pr_err("i3c: set pin idle failed\n");
	}
	mutex_unlock(&master->lock);
}

/* for i3c send waveform,must use with send_unprepare */
static int hs_dw_i3c_send_prepare(struct i3c_master_controller *controller)
{
	struct dw_i3c_master *master = NULL;

	master = to_dw_i3c_master(controller);
	if (!master) {
		pr_err("%s get i3c master failed!\n", __func__);
		return -EINVAL;
	}

	hs_dw_i3c_hardware_cfg(controller, I3C_USE_PINCTRL);
	if (hs_dw_i3c_pin_check(controller)) {
		hs_dw_i3c_hardware_disable(controller, I3C_USE_PINCTRL);
		return -EINVAL;
	}

	enable_irq(master->irq);

	return 0;
}

/* for i3c send Waveform,must use with send_prepare */
static void hs_dw_i3c_send_unprepare(struct i3c_master_controller *controller)
{
	struct dw_i3c_master *master = NULL;

	master = to_dw_i3c_master(controller);
	if (!master) {
		pr_err("%s get i3c master failed!\n", __func__);
		return;
	}

	disable_irq(master->irq);
	hs_dw_i3c_hardware_disable(controller, I3C_USE_PINCTRL);
}

void hs_dw_i3c_master_bus_cleanup(struct i3c_master_controller *controller)
{
	if (!controller) {
		pr_err("%s i3c controller is null!\n", __func__);
		return;
	}
	hs_dw_i3c_hardware_cfg(controller, I3C_NOT_USE_PINCTRL);
	dw_i3c_master_bus_cleanup(controller);
	hs_dw_i3c_hardware_disable(controller, I3C_NOT_USE_PINCTRL);
}

int hs_dw_i3c_master_attach_i3c_dev(struct i3c_dev_desc *dev)
{
	struct i3c_master_controller *controller = NULL;
	int ret;

	if (!dev) {
		pr_err("%s i3c dev is null!\n", __func__);
		return -EINVAL;
	}

	controller = i3c_dev_get_master(dev);
	if (!controller) {
		pr_err("%s get i3c master controller failed!\n", __func__);
		return -EINVAL;
	}
	hs_dw_i3c_hardware_cfg(controller, I3C_NOT_USE_PINCTRL);
	ret = dw_i3c_master_attach_i3c_dev(dev);
	hs_dw_i3c_hardware_disable(controller, I3C_NOT_USE_PINCTRL);

	return ret;
}

int hs_dw_i3c_master_reattach_i3c_dev(struct i3c_dev_desc *dev,
	u8 old_dyn_addr)
{
	struct i3c_master_controller *controller = NULL;
	int ret;

	if (!dev) {
		pr_err("%s i3c dev is null!\n", __func__);
		return -EINVAL;
	}

	controller = i3c_dev_get_master(dev);
	if (!controller) {
		pr_err("%s get i3c master controller failed!\n", __func__);
		return -EINVAL;
	}
	hs_dw_i3c_hardware_cfg(controller, I3C_NOT_USE_PINCTRL);
	ret = dw_i3c_master_reattach_i3c_dev(dev, old_dyn_addr);
	hs_dw_i3c_hardware_disable(controller, I3C_NOT_USE_PINCTRL);

	return ret;
}

void hs_dw_i3c_master_detach_i3c_dev(struct i3c_dev_desc *dev)
{
	struct i3c_master_controller *controller = NULL;

	if (!dev) {
		pr_err("%s i3c dev is null!\n", __func__);
		return;
	}

	controller = i3c_dev_get_master(dev);
	if (!controller) {
		pr_err("%s get i3c master controller failed!\n", __func__);
		return;
	}

	hs_dw_i3c_hardware_cfg(controller, I3C_NOT_USE_PINCTRL);
	dw_i3c_master_detach_i3c_dev(dev);
	hs_dw_i3c_hardware_disable(controller, I3C_NOT_USE_PINCTRL);
}

int hs_dw_i3c_master_daa(struct i3c_master_controller *controller)
{
	int ret;

	if (!controller) {
		pr_err("%s i3c controller is null!\n", __func__);
		return -EINVAL;
	}

	if (hs_dw_i3c_send_prepare(controller))
		return -EINVAL;
	ret = dw_i3c_master_daa(controller);
	if (ret < 0)
		pr_err("%s i3c transfer failed, %d\n", __func__, ret);
	hs_dw_i3c_send_unprepare(controller);

	return ret;
}

int hs_dw_i3c_master_send_ccc_cmd(struct i3c_master_controller *controller,
	struct i3c_ccc_cmd *ccc)
{
	int ret;

	if ((!controller) || (!ccc)) {
		pr_err("%s i3c controller or ccc is null!\n", __func__);
		return -EINVAL;
	}

	if (hs_dw_i3c_send_prepare(controller))
		return -EINVAL;
	ret = dw_i3c_master_send_ccc_cmd(controller, ccc);
	if (ret < 0)
		pr_err("%s i3c transfer failed, %d\n", __func__, ret);
	hs_dw_i3c_send_unprepare(controller);

	return ret;
}

int hs_dw_i3c_master_priv_xfers(struct i3c_dev_desc *dev,
	struct i3c_priv_xfer *i3c_xfers, int i3c_nxfers)
{
	struct i3c_master_controller *controller = NULL;
	int ret;

	if (!dev) {
		pr_err("%s i3c dev is null!\n", __func__);
		return -EINVAL;
	}

	controller = i3c_dev_get_master(dev);
	if (!controller) {
		pr_err("%s get i3c master controller failed!\n", __func__);
		return -EINVAL;
	}

	if (hs_dw_i3c_send_prepare(controller))
		return -EINVAL;
	ret = dw_i3c_master_priv_xfers(dev, i3c_xfers, i3c_nxfers);
	if (ret < 0)
		pr_err("%s i3c transfer failed, %d\n", __func__, ret);
	hs_dw_i3c_send_unprepare(controller);

	return ret;
}

int hs_dw_i3c_master_attach_i2c_dev(struct i2c_dev_desc *dev)
{
	struct i3c_master_controller *controller = NULL;
	int ret;

	if (!dev) {
		pr_err("%s i2c dev is null!\n", __func__);
		return -EINVAL;
	}

	controller = i2c_dev_get_master(dev);
	if (!controller) {
		pr_err("%s get i3c master controller failed!\n", __func__);
		return -EINVAL;
	}

	hs_dw_i3c_hardware_cfg(controller, I3C_NOT_USE_PINCTRL);
	ret = dw_i3c_master_attach_i2c_dev(dev);
	hs_dw_i3c_hardware_disable(controller, I3C_NOT_USE_PINCTRL);

	return ret;
}

void hs_dw_i3c_master_detach_i2c_dev(struct i2c_dev_desc *dev)
{
	struct i3c_master_controller *controller = NULL;

	if (!dev) {
		pr_err("%s i2c dev is null!\n", __func__);
		return;
	}

	controller = i2c_dev_get_master(dev);
	if (!controller) {
		pr_err("%s get i3c master controller failed!\n", __func__);
		return;
	}

	hs_dw_i3c_hardware_cfg(controller, I3C_NOT_USE_PINCTRL);
	dw_i3c_master_detach_i2c_dev(dev);
	hs_dw_i3c_hardware_disable(controller, I3C_NOT_USE_PINCTRL);
}

int hs_dw_i3c_master_i2c_xfers(struct i2c_dev_desc *dev,
	const struct i2c_msg *i2c_xfers, int i2c_nxfers)
{
	struct i3c_master_controller *controller = NULL;
	int ret;

	if (!dev) {
		pr_err("%s i2c dev is null!\n", __func__);
		return -EINVAL;
	}

	controller = i2c_dev_get_master(dev);
	if (!controller) {
		pr_err("%s get i3c master controller failed!\n", __func__);
		return -EINVAL;
	}

	if (hs_dw_i3c_send_prepare(controller))
		return -EINVAL;
	ret = dw_i3c_master_i2c_xfers(dev, i2c_xfers, i2c_nxfers);
	if (ret < 0)
		pr_err("%s i3c transfer failed, %d\n", __func__, ret);
	hs_dw_i3c_send_unprepare(controller);

	return ret;
}

static void hs_dw_i3c_read_ctrl_cfg(struct device *dev,
	struct dw_i3c_master *master)
{
	int ret;
	u32 data[I3C_RESET_REG_CNT] = {0};

	ret = of_property_read_u32_array(dev->of_node, "reset-reg-base",
		&data[0], ARRAY_SIZE(data));
	if (ret) {
		pr_err("doesn't have reset-reg-base property!\n");
	} else {
		master->reset_reg_base = devm_ioremap(dev, data[1], data[3]);
		pr_info("i3c reset register phy_addr is: %x\n", data[1]);
	}

	ret = of_property_read_u32_array(dev->of_node, "reset-controller-reg",
		&data[0], ARRAY_SIZE(data));
	if (ret)
		pr_err("doesn't have reset-controller-reg property!\n");

	/* data[0~2] is reset register address, data[3] is reset bit for i2c */
	master->priv.reset_enable = data[0];
	master->priv.reset_disable = data[1];
	master->priv.reset_status = data[2];
	master->priv.reset_bit = data[3];
}

int hs_dw_i3c_init(struct platform_device *pdev,
	struct dw_i3c_master *master)
{
	u64 clk_rate = 0;
	struct device *dev = NULL;
	u32 transfer_mode = 0;
	u32 i2c_speed = 0;
	int ret;

	if (!pdev)
		return -EINVAL;
	if ((!master) || (!master->core_clk))
		return -EINVAL;

	dev = &pdev->dev;
	master->pinctrl_flag = 0;
	master->pinctrl = NULL;

	ret = of_property_read_u64(dev->of_node, "clock-rate", &clk_rate);
	if (ret) {
		pr_err("i3c master doesn't have clock-rate property!\n");
	} else {
		ret = clk_set_rate(master->core_clk, clk_rate);
		if (ret) {
			pr_err("clock rate set failed r[0x%x]\n", ret);
			return -EINVAL;
		}
	}

	ret = of_property_read_u32(dev->of_node, "trans-mode", &transfer_mode);
	if (ret) {
		pr_info("i3c master doesn't have trans-mode, use defult\n");
		master->trans_mode = I3C_TRANSFER_MODE_POLL;
	} else {
		if (transfer_mode == I3C_TRANSFER_MODE_IRQ)
			master->trans_mode = I3C_TRANSFER_MODE_IRQ;
		else
			master->trans_mode = I3C_TRANSFER_MODE_POLL;
	}

	ret = of_property_read_u32(dev->of_node, "i2c-scl-hz", &i2c_speed);
	if (ret)
		master->i2c_scl_hz = I3C_BUS_I2C_FM_SCL_RATE;
	else
		master->i2c_scl_hz = i2c_speed;

	hs_dw_i3c_read_ctrl_cfg(dev, master);

	mutex_init(&master->lock);

	return 0;
}

static int i3c_master_suspend(struct dw_i3c_master *master)
{
	unsigned long flags;
	unsigned int limit = 500;
	int ret = 0;

	spin_lock_irqsave(&master->xferqueue.lock, flags);

	while (!list_empty(&master->xferqueue.list) && limit--) {
		spin_unlock_irqrestore(&master->xferqueue.lock, flags);
		/* sleep 1ms ~ 1.1ms */
		usleep_range(1000, 1100);
		spin_lock_irqsave(&master->xferqueue.lock, flags);
	}

	if (!list_empty(&master->xferqueue.list))
		ret = -EBUSY;
	else
		writel(RESET_CTRL_RX_FIFO | RESET_CTRL_TX_FIFO |
			RESET_CTRL_RESP_QUEUE | RESET_CTRL_CMD_QUEUE,
			master->regs + RESET_CTRL);

	spin_unlock_irqrestore(&master->xferqueue.lock, flags);

	return ret;
}

static int i3c_master_resume(struct dw_i3c_master *master)
{
	int ret, i;

	master->i3c_resume_init = I3C_RESUME_INIT;
	ret = dw_i3c_master_bus_init(&master->base);
	if (ret)
		pr_err("dw_i3c_init failed\n");

	for (i = 0; i < I3C_MAX_SLAVE_NUM; i++)
		writel(master->i2c_slave_addr[i], master->regs +
			I3C_DEV_ADDR_TABLE + i * I3C_REG_LENGTH);

	return ret;
}

int hs_dw_i3c_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct dw_i3c_master *master = platform_get_drvdata(pdev);
	int ret, i;

	if ((!master) || (!master->core_clk)) {
		dev_err(dev, "%s: get drvdata failed\n", __func__);
		return -EINVAL;
	}

	dev_info(dev, "%s: suspend +\n", __func__);

	ret = i3c_master_suspend(master);
	if (ret) {
		dev_err(dev, "cannot suspend master\n");
		return ret;
	}
	for (i = 0; i < I3C_MAX_SLAVE_NUM; i++)
		master->i2c_slave_addr[i] = readl(master->regs +
			I3C_DEV_ADDR_TABLE + i * I3C_REG_LENGTH);
	mutex_lock(&master->lock);
	clk_disable_unprepare(master->core_clk);

	dev_info(dev, "%s: suspend -\n", __func__);
	return 0;
}

int hs_dw_i3c_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct dw_i3c_master *master = platform_get_drvdata(pdev);
	int ret;

	if ((!master) || (!master->core_clk)) {
		dev_err(dev, "%s: get drvdata failed\n", __func__);
		return -EINVAL;
	}

	dev_info(dev, "%s: resume +\n", __func__);

	ret = clk_prepare_enable(master->core_clk);
	if (ret)
		dev_err(dev, "enable clk failed\n");

	i3c_master_resume(master);

	mutex_unlock(&master->lock);
	dev_info(dev, "%s: resume -\n", __func__);

	return 0;
}

int i3c_init_secos(struct i2c_adapter *adap)
{
	struct i3c_master_controller *controller = NULL;
	struct dw_i3c_master *master = NULL;
	int ret;
	unsigned int i;

	controller = container_of(adap,
		struct i3c_master_controller, i2c);
	if (!controller) {
		pr_err("%s get i3c controller failed!\n", __func__);
		return -1;
	}

	master = to_dw_i3c_master(controller);
	if (!master) {
		pr_err("i3c: i3c master doesn't be init\n");
		return -1;
	}

	ret = i3c_master_suspend(master);
	if (ret) {
		pr_err("%s i3c can not in secos\n", __func__);
		return ret;
	}
	for (i = 0; i < I3C_MAX_SLAVE_NUM; i++)
		master->i2c_slave_addr[i] = readl(master->regs +
			I3C_DEV_ADDR_TABLE + i * I3C_REG_LENGTH);

	mutex_lock(&master->lock);

	return 0;
}

int i3c_exit_secos(struct i2c_adapter *adap)
{
	struct i3c_master_controller *controller = NULL;
	struct dw_i3c_master *master = NULL;
	int ret;

	controller = container_of(adap,
		struct i3c_master_controller, i2c);
	if (!controller) {
		pr_err("%s get i3c controller failed!\n", __func__);
		return -1;
	}

	master = to_dw_i3c_master(controller);
	if (!master) {
		pr_err("i3c: i3c master doesn't be init\n");
		return -1;
	}

	ret = i3c_master_resume(master);
	if (ret) {
		pr_err("%s i3c can not out secos\n", __func__);
		return ret;
	}
	mutex_unlock(&master->lock);

	return 0;
}
