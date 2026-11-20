/*
 * I2C Master adapter driver (master only).
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
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

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/arm-smccc.h>
#include "i2c_v400_master.h"

static char *error_sources[] = {
	[ERROR_7B_ADDR_NOACK] =
		"slave address not acknowledged (7bit mode)",
	[ERROR_10ADDR1_NOACK] =
		"first address byte not acknowledged (10bit mode)",
	[ERROR_10ADDR2_NOACK] =
		"second address byte not acknowledged (10bit mode)",
	[ERROR_TXDATA_NOACK] =
		"data not acknowledged",
	[ERROR_GCALL_NOACK] =
		"no acknowledgement for a general call",
	[ERROR_GCALL_READ] =
		"read after general call",
	[ERROR_HS_ACKDET] =
	"the high-speed master code was achnowledged",
	[ERROR_SBYTE_ACKDET] =
		"start byte acknowledged",
	[ERROR_HS_NORSTRT] =
	"the restart is disabled and the user is trying to use the master to transfer data in high-speed mode",
	[ERROR_SBYTE_NORSTRT] =
		"trying to send start byte when restart is disabled",
};

static u32 secure_i2c_v400_readl(u32 base_addr, u32 offset)
{
	struct arm_smccc_res res;

	arm_smccc_1_1_smc(HISI_I2C_READ_REG, base_addr, offset, &res);
	return (u32)res.a1;
}

static void secure_i2c_v400_writel(u32 b, u32 base_addr, u32 offset)
{
	struct arm_smccc_res res;

	arm_smccc_1_1_smc(HISI_I2C_WRITE_REG, b, base_addr, offset, &res);
}

static u32 i2c_v400_readl(struct hs_i2c_dev *dev, u32 offset)
{
	u32 value;

	if (dev->secure_mode)
		value = secure_i2c_v400_readl(dev->reg_base, offset);
	else
		value = readl_relaxed(dev->base + offset);

	return value;
}

static void i2c_v400_writel(struct hs_i2c_dev *dev, u32 b, u32 offset)
{
	if (dev->secure_mode)
		secure_i2c_v400_writel(b, dev->reg_base, offset);
	else
		writel_relaxed(b, dev->base + offset);
}

static u32 hs_i2c_get_clk_rate_khz(struct hs_i2c_dev *dev)
{
	u32 rate;

	rate = clk_get_rate(dev->clk) / I2C_KHZ;
	dev_dbg(dev->dev, "input_clock_khz value is %u\n", rate);

	return rate;
}

static int i2c_pins_ctrl(struct hs_i2c_dev *dev, const char *name)
{
	struct pinctrl_state *s = NULL;
	int ret;

	if (!dev->pinctrl_flag) {
		dev->pinctrl = devm_pinctrl_get(dev->dev);
		if (IS_ERR(dev->pinctrl)) {
			dev_err(dev->dev, "%s: get pinctrl fail\n", __func__);
			return -1;
		}
		dev->pinctrl_flag = 1;
	}

	s = pinctrl_lookup_state(dev->pinctrl, name);
	if (IS_ERR(s)) {
		devm_pinctrl_put(dev->pinctrl);
		dev->pinctrl_flag = 0;
		dev_err(dev->dev, "%s: lookup state fail\n", __func__);
		return -1;
	}

	ret = pinctrl_select_state(dev->pinctrl, s);
	if (ret < 0) {
		devm_pinctrl_put(dev->pinctrl);
		dev->pinctrl_flag = 0;
		dev_err(dev->dev, "%s: select state fail\n", __func__);
		return -1;
	}

	return 0;
}

static void hs_i2c_fifo_clear(struct hs_i2c_dev *dev)
{
	u32 fifo_ctrl;

	fifo_ctrl = i2c_v400_readl(dev, FIFO_CTRL);
	fifo_ctrl = fifo_ctrl | FIFO_CLEAR;
	fifo_ctrl = fifo_ctrl & (~FIFO_CLEAR);
	i2c_v400_writel(dev, fifo_ctrl, FIFO_CTRL);
}

static unsigned long hs_i2c_clk_rate(struct hs_i2c_dev *dev)
{
	/*
	 * Clock is not necessary if we got LCNT/HCNT values directly from
	 * the platform code.
	 */
	if (!dev->get_clk_rate_khz) {
		dev_err(dev->dev, "%s: i2c get clk failed\n", __func__);
		return 0;
	}

	return dev->get_clk_rate_khz(dev);
}
/**
 * hs_i2c_init_master() - Initialize the I2C master hardware
 * @dev: device private data
 *
 * This functions configures and enables the I2C master.
 * This function is called during I2C init function, and in case of timeout at
 * run time.
 */
static int hs_i2c_init_master(struct hs_i2c_dev *dev)
{
	u32 cnt, hcnt, lcnt;
	u32 speed_mode;

	/* set i2c speed */
	speed_mode = dev->master_cfg & HS_I2C_SPEED_MASK;

	if (!dev->fs_spk_len)
		dev->fs_spk_len = i2c_v400_readl(dev, FS_SPK_LEN);
	if (!dev->hs_spk_len)
		dev->hs_spk_len = i2c_v400_readl(dev, HS_SPK_LEN);
	switch (speed_mode) {
	case HS_I2C_SPEED_STD:
		cnt = hs_i2c_clk_rate(dev) / I2C_SS_SPEED;
		hcnt = cnt * I2C_SS_THIGH_MIN_NS /
			I2C_SS_SPEED_HPERIOD - dev->fs_spk_len - 7;
		lcnt = cnt * I2C_SS_TLOW_MIN_NS /
			I2C_SS_SPEED_HPERIOD - 1;
		i2c_v400_writel(dev, hcnt, SS_SCL_HCNT);
		i2c_v400_writel(dev, lcnt, SS_SCL_LCNT);
		break;
	case HS_I2C_SPEED_FAST:
		cnt = hs_i2c_clk_rate(dev) / I2C_FS_SPEED;
		hcnt = cnt * I2C_FS_THIGH_MIN_NS /
			I2C_FS_SPEED_HPERIOD - dev->fs_spk_len - 7;
		lcnt = cnt * I2C_FS_TLOW_MIN_NS /
			I2C_FS_SPEED_HPERIOD - 1;
		i2c_v400_writel(dev, hcnt, FS_SCL_HCNT);
		i2c_v400_writel(dev, lcnt, FS_SCL_LCNT);
		break;
	case HS_I2C_SPEED_HIGH:
		cnt = hs_i2c_clk_rate(dev) / I2C_HS_SPEED;
		hcnt = cnt * I2C_HS_THIGH_MIN_NS /
			I2C_HS_SPEED_HPERIOD - dev->hs_spk_len - 7;
		lcnt = cnt * I2C_HS_TLOW_MIN_NS /
			I2C_HS_SPEED_HPERIOD - 1;
		i2c_v400_writel(dev, hcnt, HS_SCL_HCNT);
		i2c_v400_writel(dev, lcnt, HS_SCL_LCNT);
		break;
	default:
		dev_err(dev->dev, "use Fast-mode default!\n");
		cnt = hs_i2c_clk_rate(dev) / I2C_FS_SPEED;
		hcnt = cnt * I2C_FS_THIGH_MIN_NS /
			I2C_FS_SPEED_HPERIOD - dev->fs_spk_len - 7;
		lcnt = cnt * I2C_FS_TLOW_MIN_NS /
			I2C_FS_SPEED_HPERIOD - 1;
		i2c_v400_writel(dev, hcnt, FS_SCL_HCNT);
		i2c_v400_writel(dev, lcnt, FS_SCL_LCNT);
	}

	/* Configure SDA Hold Time */
	i2c_v400_writel(dev, I2C_DEFULT_HOLD_TIME, SDA_HOLD);

	/* configure the i2c master */
	i2c_v400_writel(dev, dev->master_cfg, FRAME_CTRL);

	/* Configure FIFO */
	hs_i2c_fifo_clear(dev);
	i2c_v400_writel(dev, FIFO_TX_THD | FIFO_RX_THD, FIFO_CTRL);
	return 0;
}

static void hs_i2c_disable_int(struct hs_i2c_dev *dev)
{
	i2c_v400_writel(dev, 0, INT_MASK);
}

static int hs_i2c_reset_controller(struct hs_i2c_dev *dev)
{
	struct hs_i2c_priv_data *priv = NULL;
	u32 val = 0;
	int timeout = I2C_WAIT_STATUS_TIMES;

	if (!dev) {
		pr_err("%s:i2c ctrler doesn't be init\n", __func__);
		return -ENODEV;
	}

	priv = &dev->priv;

	writel(BIT(priv->reset_bit),
		dev->reset_reg_base + priv->reset_enable_off);
	do {
		val = readl(dev->reset_reg_base + priv->reset_status_off);
		val &= BIT(priv->reset_bit);
		udelay(1);
	} while (!val && timeout--);

	if (timeout <= 0) {
		dev_err(dev->dev, "%s:i2c reset enable timeout\n", __func__);
		return -ENODEV;
	}

	timeout = I2C_WAIT_STATUS_TIMES;

	writel(BIT(priv->reset_bit),
		dev->reset_reg_base + priv->reset_disable_off);
	do {
		val = readl(dev->reset_reg_base + priv->reset_status_off);
		val &= BIT(priv->reset_bit);
		udelay(1);
	} while (val && timeout--);

	if (timeout <= 0) {
		dev_err(dev->dev, "%s:i2c reset disable timeout\n", __func__);
		return -ENODEV;
	}

	return 0;
}

static void reset_i2c_controller(struct hs_i2c_dev *dev)
{
	int r;

	disable_irq(dev->irq);

	r = i2c_pins_ctrl(dev, PINCTRL_STATE_IDLE);
	if (r < 0)
		dev_warn(dev->dev, "pins are not configured from the driver\n");

	r = hs_i2c_reset_controller(dev);
	if (r)
		dev_err(dev->dev, "%s:i2c resest controller fail\n", __func__);

	hs_i2c_init_master(dev);
	hs_i2c_disable_int(dev);

	r = i2c_pins_ctrl(dev, PINCTRL_STATE_DEFAULT);
	if (r < 0)
		dev_warn(dev->dev, "pins are not configured from the driver\n");

	enable_irq(dev->irq);
}

#ifdef CONFIG_DMA_ENGINE
static void hs_i2c_dma_probe_initcall(struct hs_i2c_dev *dev)
{
	struct dma_chan *chan = NULL;
	dma_cap_mask_t mask;
	struct dma_slave_config tx_conf = {};
	struct dma_slave_config rx_conf = {};

	/* DMA is the sole user of the platform data right now */
	tx_conf.dst_addr = dev->mapbase + CMD_TXDATA;
	tx_conf.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	tx_conf.direction = DMA_MEM_TO_DEV;
	tx_conf.dst_maxburst = I2C_DMA_MAX_BURST;

	rx_conf.src_addr = dev->mapbase + RXDATA;
	rx_conf.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
	rx_conf.direction = DMA_DEV_TO_MEM;
	rx_conf.src_maxburst = I2C_DMA_MAX_BURST;

	/* Try to acquire a generic DMA engine slave TX channel */
	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);

	chan = dma_request_slave_channel(dev->dev, "tx");
	if (!chan) {
		dev_err(dev->dev, "no TX DMA channel!\n");
		return;
	}

	dmaengine_slave_config(chan, &tx_conf);
	dev->dmatx.chan = chan;

	dev_info(dev->dev, "DMA channel TX %s-%d\n",
		dma_chan_name(dev->dmatx.chan),
		dev->dmatx.chan->chan_id);

	chan = dma_request_slave_channel(dev->dev, "rx");
	if (!chan) {
		dev_err(dev->dev, "no RX DMA channel!\n");
		return;
	}

	dmaengine_slave_config(chan, &rx_conf);
	dev->dmarx.chan = chan;

	dev_info(dev->dev, "DMA channel RX %s-%d\n",
		dma_chan_name(dev->dmarx.chan),
		dev->dmarx.chan->chan_id);
}

static void hs_i2c_dma_probe(struct hs_i2c_dev *dev)
{
	hs_i2c_dma_probe_initcall(dev);
}

static void hs_i2c_dma_remove(struct hs_i2c_dev *dev)
{
	/* remove the initcall if it has not yet executed */
	if (dev->dmatx.chan)
		dma_release_channel(dev->dmatx.chan);
	if (dev->dmarx.chan)
		dma_release_channel(dev->dmarx.chan);
}

/*
 * The current DMA TX buffer has been sent.
 * Try to queue up another DMA buffer.
 */
static void hs_i2c_dma_tx_callback(void *data)
{
	struct hs_i2c_dev *dev = data;

	if (!dev) {
		pr_err("%s:i2c ctrler doesn't be init\n", __func__);
		return;
	}

	dev_dbg(dev->dev, "%s: entry\n", __func__);

	dev->using_tx_dma = false;

	if (!(dev->using_tx_dma) && !(dev->using_rx_dma))
		complete(&dev->dma_complete);
}

static int hs_i2c_dma_tx_refill(struct hs_i2c_dev *dev)
{
	struct hs_i2c_dma_data *dmatx = NULL;
	struct dma_chan *chan = NULL;
	struct dma_device *dma_dev = NULL;
	struct dma_async_tx_descriptor *desc = NULL;

	dmatx = &dev->dmatx;
	chan = dmatx->chan;
	if (!chan) {
		dev_err(dev->dev, "chan is NULL!\n");
		return -EIO;
	}

	dma_dev = chan->device;

	/* Map only 1 entry */
	if (dma_map_sg(dma_dev->dev, &dmatx->sg, 1, DMA_TO_DEVICE) != 1) {
		dev_warn(dev->dev, "unable to map TX DMA\n");
		return -EBUSY;
	}

	/* only 1 mapped entry */
	desc = dmaengine_prep_slave_sg(chan, &dmatx->sg, 1,
		DMA_MEM_TO_DEV,
		DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!desc) {
		dma_unmap_sg(dma_dev->dev, &dmatx->sg, 1, DMA_TO_DEVICE);
		dev_warn(dev->dev, "TX DMA busy\n");
		return -EBUSY;
	}

	/* Some data to go along to the callback */
	desc->callback = hs_i2c_dma_tx_callback;
	desc->callback_param = dev;

	desc->tx_submit(desc);

	/* Fire the DMA transaction */
	dma_dev->device_issue_pending(chan);

	return 0;
}

static void hs_i2c_dma_rx_callback(void *data)
{
	struct hs_i2c_dev *dev = data;
	struct i2c_msg *msgs = NULL;
	struct hs_i2c_dma_data *dmarx = NULL;
	int rx_valid;
	int rd_idx = 0;
	u32 len;
	u8 *buf = NULL;

	if (!dev) {
		pr_err("%s:i2c ctrler doesn't be init\n", __func__);
		return;
	}
	dev_dbg(dev->dev, "%s: entry\n", __func__);

	msgs = dev->msgs;
	dmarx = &dev->dmarx;
	rx_valid = dmarx->sg.length;

	/* Sync in buffer , only 1 mapped entry */
	dma_sync_sg_for_cpu(dev->dev, &dmarx->sg, 1, DMA_FROM_DEVICE);

	for (; dev->msg_read_idx < dev->msgs_num; dev->msg_read_idx++) {
		if (!(msgs[dev->msg_read_idx].flags & I2C_M_RD))
			continue;

		len = msgs[dev->msg_read_idx].len;
		buf = msgs[dev->msg_read_idx].buf;

		for (; len > 0 && rx_valid > 0; len--, rx_valid--)
			*buf++ = dmarx->buf[rd_idx++];
	}

	dev->using_rx_dma = false;

	if (!dev->using_tx_dma && !dev->using_rx_dma)
		complete(&dev->dma_complete);
}

static int hs_i2c_dma_rx_trigger_dma(struct hs_i2c_dev *dev)
{
	struct hs_i2c_dma_data *dmarx = NULL;
	struct dma_chan *rxchan = NULL;
	struct dma_device *dma_dev = NULL;
	struct dma_async_tx_descriptor *desc = NULL;

	dmarx = &dev->dmarx;
	rxchan = dmarx->chan;
	if (!rxchan) {
		dev_err(dev->dev, "rxchan is NULL!\n");
		return -EIO;
	}

	dma_dev = rxchan->device;

	dev_dbg(dev->dev, "%s, %u bytes to read\n",
		__func__, dev->dmarx.sg.length);

	/* Map only 1 entry */
	if (dma_map_sg(dma_dev->dev, &dmarx->sg, 1, DMA_FROM_DEVICE) != 1) {
		dev_warn(dev->dev, "unable to map TX DMA\n");
		return -EBUSY;
	}

	/* Only 1 mapped entry */
	desc = dmaengine_prep_slave_sg(rxchan, &dmarx->sg, 1,
		DMA_DEV_TO_MEM,
		DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!desc) {
		dma_unmap_sg(dma_dev->dev, &dmarx->sg, 1, DMA_FROM_DEVICE);
		dev_warn(dev->dev, "RX DMA busy\n");
		return -EBUSY;
	}

	/* Some data to go along to the callback */
	desc->callback = hs_i2c_dma_rx_callback;
	desc->callback_param = dev;

	/* All errors should happen at prepare time */
	dmaengine_submit(desc);

	/* Fire the DMA transaction */
	dma_async_issue_pending(rxchan);

	return 0;
}

static int hs_i2c_dma_sg_init(struct hs_i2c_dev *dev,
	struct hs_i2c_dma_data *dma_data, unsigned long length)
{
	dma_data->buf = devm_kzalloc(dev->dev, length, GFP_KERNEL);
	if (!dma_data->buf)
		return -ENOMEM;

	sg_init_one(&dma_data->sg, dma_data->buf, length);

	return 0;
}

static void hs_i2c_dma_clear(struct hs_i2c_dev *dev)
{
	if (dev->dmatx.buf) {
		dmaengine_terminate_all(dev->dmatx.chan);
		dma_unmap_sg(dev->dmatx.chan->device->dev,
			&dev->dmatx.sg, 1, DMA_TO_DEVICE);
		devm_kfree(dev->dev, dev->dmatx.buf);
		dev->dmatx.buf = NULL;
		dev->using_tx_dma = false;
	}

	if (dev->dmarx.buf) {
		dmaengine_terminate_all(dev->dmarx.chan);
		dma_unmap_sg(dev->dmarx.chan->device->dev,
			&dev->dmarx.sg, 1, DMA_FROM_DEVICE);
		devm_kfree(dev->dev, dev->dmarx.buf);
		dev->dmarx.buf = NULL;
		dev->using_rx_dma = false;
	}

	dev->using_dma = false;
	i2c_v400_writel(dev, I2C_DMA_DISABLE, DMA_CTRL);
}

#else
/* Blank functions if the DMA engine is not available */
static void hs_i2c_dma_probe(struct hs_i2c_dev *dev)
{
}

static void hs_i2c_dma_remove(struct hs_i2c_dev *dev)
{
}

static int hs_i2c_dma_tx_refill(struct hs_i2c_dev *dev)
{
	return -EIO;
}

static int hs_i2c_dma_rx_trigger_dma(struct hs_i2c_dev *dev)
{
	return -EIO;
}

static int hs_i2c_dma_sg_init(struct hs_i2c_dev *dev,
	struct hs_i2c_dma_data *dma_data, unsigned long length)
{
	return -EIO;
}

static void hs_i2c_dma_clear(struct hs_i2c_dev *dev)
{
}

#endif

/*
 * This routine does i2c bus recovery by using i2c_generic_gpio_recovery
 * which is provided by I2C Bus recovery infrastructure.
 */
static void i2c_prepare_recovery(struct i2c_adapter *adap)
{
	struct hs_i2c_dev *dev = i2c_get_adapdata(adap);
	int ret;

	ret = i2c_pins_ctrl(dev, PINCTRL_STATE_IDLE);
	if (ret < 0)
		dev_warn(dev->dev, "pins are not configured to idle\n");
}

static void i2c_unprepare_recovery(struct i2c_adapter *adap)
{
	struct hs_i2c_dev *dev = i2c_get_adapdata(adap);
	int ret;

	ret = i2c_pins_ctrl(dev, PINCTRL_STATE_DEFAULT);
	if (ret < 0)
		dev_warn(dev->dev, "pins are not configured to default\n");
}

static void i2c_scl_recover_bus(struct i2c_adapter *adap)
{
	struct hs_i2c_dev *dev = i2c_get_adapdata(adap);
	struct i2c_bus_recovery_info *bri = adap->bus_recovery_info;
	int ret;

	// TODO: dependent pinctrl
	return;

	dev_info(dev->dev, "bus prepares recovery ...\n");

	ret = gpio_request_one(bri->sda_gpio, GPIOF_IN, "i2c-sda");
	if (ret)
		dev_warn(dev->dev,
			"Can't get SDA gpio: %d. Not using SDA polling\n",
			bri->sda_gpio);

	/* speed is 100KHz */
	i2c_v400_writel(dev, HISI_I2C_CON_RECOVERY_CFG, FRAME_CTRL);

	/* config slave address to 0x7F */
	i2c_v400_writel(dev, HISI_I2C_TAR_RECOVERY_CFG, SLV_ADDR);

	/* recived data from bus */
	i2c_v400_writel(dev, HISI_I2C_RD_CFG, CMD_TXDATA);

	msleep(I2C_RECOVER_SLEEP_MS);

	gpio_free(bri->sda_gpio);

	ret = i2c_pins_ctrl(dev, PINCTRL_STATE_DEFAULT);
	if (ret < 0)
		dev_warn(dev->dev, "pins are not configured to default\n");

	hs_i2c_init_master(dev);

	dev_info(dev->dev, "bus recovered completely!\n");
}

/*
 * Waiting for bus not busy
 */
static int hs_i2c_wait_bus_not_busy(struct hs_i2c_dev *dev)
{
	int timeout = TIMEOUT;

	while (i2c_v400_readl(dev, TRANS_STATE) & TRANS_STATE_MASK) {
		if (timeout <= 0) {
			dev_warn(dev->dev, "timeout waiting for bus ready\n");
			return -ETIMEDOUT;
		}
		timeout--;
		usleep_range(1000, 1100);
	}

	return 0;
}

static void hs_i2c_xfer_init(struct hs_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	u32 ic_con;

	/* If the slave address is ten bit address, enable 10BITADDR */
	ic_con = i2c_v400_readl(dev, FRAME_CTRL);
	if (msgs[dev->msg_write_idx].flags & I2C_M_TEN)
		ic_con |= I2C_CON_10BITADDR_MASTER;
	else
		ic_con &= ~I2C_CON_10BITADDR_MASTER;

	i2c_v400_writel(dev, ic_con, FRAME_CTRL);
	i2c_v400_writel(dev, msgs[dev->msg_write_idx].addr, SLV_ADDR);

	/* Enforce disabled interrupts (due to HW issues) */
	hs_i2c_disable_int(dev);

	/* Clear and enable interrupts */
	i2c_v400_writel(dev, I2C_INT_CLEAR, INT_CLR);
	i2c_v400_writel(dev, I2C_INTR_DEFAULT_MASK, INT_MASK);
}

static int hs_i2c_msg_check(struct hs_i2c_dev *dev, struct i2c_msg *msgs)
{
	u32 addr = msgs[dev->msg_write_idx].addr;
	int ret = 0;
	/*
	 * If target address has changed, we need to
	 * reprogram the target address in the I2C
	 * adapter when we are done with this transfer.
	 */
	if (msgs[dev->msg_write_idx].addr != addr) {
		dev_err(dev->dev,
			"%s: invalid target address\n", __func__);
		dev->msg_err = -EINVAL;
		return -EINVAL;
	}

	if (msgs[dev->msg_write_idx].len == 0) {
		dev_err(dev->dev,
			"%s: invalid message length\n", __func__);
		dev->msg_err = -EINVAL;
		return -EINVAL;
	}

	return ret;
}
/*
 * Initiate (and continue) low level master read/write transaction.
 * This function is only called from hs_i2c_isr, and pumping i2c_msg
 * messages into the tx buffer.  Even if the size of i2c_msg data is
 * longer than the size of the tx buffer, it handles everything.
 */
static void
hs_i2c_xfer_msg(struct hs_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	u32 intr_mask;
	int tx_limit, rx_limit;
	u32 buf_len = dev->tx_buf_len;
	u8 *buf = dev->tx_buf;
	bool need_restart = false;
	int ret;
	u32 flags = msgs[dev->msg_write_idx].flags;

	intr_mask = I2C_INTR_DEFAULT_MASK;
	for (; dev->msg_write_idx < dev->msgs_num; dev->msg_write_idx++) {
		ret = hs_i2c_msg_check(dev, msgs);
		if (ret)
			break;
		if (!(dev->status & STATUS_WRITE_IN_PROGRESS)) {
			/* new i2c_msg */
			buf = msgs[dev->msg_write_idx].buf;
			buf_len = msgs[dev->msg_write_idx].len;

			/* If both IC_EMPTYFIFO_HOLD_MASTER_EN and
			* IC_RESTART_EN are set, we must manually
			* set restart bit between messages.
			*/
			if ((dev->master_cfg & I2C_RESTART_EN) &&
					(dev->msg_write_idx > 0))
				need_restart = true;
		}
		tx_limit = dev->tx_fifo_depth - buf_len;
		rx_limit = dev->rx_fifo_depth - buf_len;

		while (buf_len > 0 && tx_limit > 0 && rx_limit > 0) {
			u32 cmd = 0;

			/*
			 * If IC_EMPTYFIFO_HOLD_MASTER_EN is set we must
			 * manually set the stop bit. However, it cannot be
			 * detected from the registers so we set it always
			 * when writing/reading the last byte.
			 */

			if (dev->msg_write_idx == dev->msgs_num - 1 &&
				buf_len == 1 && !(flags & I2C_M_RECV_LEN))
				cmd |= BIT(9);

			if (need_restart) {
				cmd |= BIT(10);
				need_restart = false;
			}

			if (i2c_v400_readl(dev, FIFO_STATE) & HISI_I2C_TX_FULL)
				break;
			if (msgs[dev->msg_write_idx].flags & I2C_M_RD) {
				i2c_v400_writel(dev, cmd |
					I2C_READ_CMD, CMD_TXDATA);
				rx_limit--;
				dev->rx_outstanding++;
			} else {
				i2c_v400_writel(dev, cmd | *buf++, CMD_TXDATA);
			}
			buf_len--;
			tx_limit--;
		}

		dev->tx_buf = buf;
		dev->tx_buf_len = buf_len;

		if (buf_len > 0 || flags & I2C_M_RECV_LEN) {
			/* more bytes to be written */
			dev->status |= STATUS_WRITE_IN_PROGRESS;
			break;
		} else {
			dev->status &= ~STATUS_WRITE_IN_PROGRESS;
		}
	}

	if (dev->msg_err)
		intr_mask = 0;

	i2c_v400_writel(dev, intr_mask, INT_MASK);
}

static u8
hs_i2c_recv_len(struct hs_i2c_dev *dev, u8 len)
{
	struct i2c_msg *msgs = dev->msgs;
	u32 flags = msgs[dev->msg_read_idx].flags;

	/*
	 * Adjust the buffer length and mask the flag
	 * after receiving the first byte.
	 */
	len += (flags & I2C_CLIENT_PEC) ? 2 : 1;
	dev->tx_buf_len = len - min_t(u8, len, dev->rx_outstanding);
	msgs[dev->msg_read_idx].len = len;
	msgs[dev->msg_read_idx].flags &= ~I2C_M_RECV_LEN;

	return len;
}

static void
hs_i2c_read(struct hs_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;

	for (; dev->msg_read_idx < dev->msgs_num; dev->msg_read_idx++) {
		u32 len;
		u8 *buf = NULL;

		if (!(msgs[dev->msg_read_idx].flags & I2C_M_RD))
			continue;

		if (!(dev->status & STATUS_READ_IN_PROGRESS)) {
			len = msgs[dev->msg_read_idx].len;
			buf = msgs[dev->msg_read_idx].buf;
		} else {
			len = dev->rx_buf_len;
			buf = dev->rx_buf;
		}

		for (; len > 0; len--) {
			u32 flags = msgs[dev->msg_read_idx].flags;

			if (i2c_v400_readl(dev, FIFO_STATE) & HISI_I2C_RX_EMPTY)
				break;
			*buf = i2c_v400_readl(dev, RXDATA);
			/* Ensure length byte is a valid value */
			if (flags & I2C_M_RECV_LEN)
				len = hs_i2c_recv_len(dev, *buf);
			buf++;
			dev->rx_outstanding--;
		}

		if (len > 0) {
			dev->status |= STATUS_READ_IN_PROGRESS;
			dev->rx_buf_len = len;
			dev->rx_buf = buf;
		} else {
			dev->status &= ~STATUS_READ_IN_PROGRESS;
		}
		return;
	}
}

static void hs_i2c_gen_dmatx_buf(u16 *dma_txbuf, int *o_rx_len,
	struct hs_i2c_dev *dev)
{
	u8 *buf = NULL;
	u32 buf_len;
	u32 i;
	struct i2c_msg *msgs = dev->msgs;
	int rx_len = 0;

	if (!msgs) {
		dev_err(dev->dev, "%s:i2c msgs is NULL\n", __func__);
		return;
	}

	for (; dev->msg_write_idx < dev->msgs_num; dev->msg_write_idx++) {
		u32 cmd = 0;

		buf = msgs[dev->msg_write_idx].buf;
		buf_len = msgs[dev->msg_write_idx].len;

		if (msgs[dev->msg_write_idx].flags & I2C_M_RD) {
			for (i = 0; i < buf_len; i++) {
				if (dev->msg_write_idx == (dev->msgs_num - 1) &&
					i == (buf_len - 1))
					cmd |= I2C_IRQ_STOP;

				*dma_txbuf++ = cmd | I2C_READ_CMD;
				rx_len++;
			}
		} else {
			for (i = 0; i < buf_len; i++) {
				if (dev->msg_write_idx == (dev->msgs_num - 1) &&
					i == (buf_len - 1))
					cmd |= I2C_IRQ_STOP;

				*dma_txbuf++ = cmd | *buf++;
			}
		}
	}

	*o_rx_len = rx_len;
}

static int hs_i2c_xfer_msg_dma_check_ctrl(struct hs_i2c_dev *dev)
{
	if (!dev->dmatx.chan || !dev->dmarx.chan)
		return -EPERM;

	return 0;
}

static int hs_i2c_get_msg_total_len(struct hs_i2c_dev *dev)
{
	int i;
	int total_len = 0;

	for (i = dev->msg_write_idx; i < dev->msgs_num; i++)
		total_len += dev->msgs[i].len;

	dev_dbg(dev->dev, "%s: msg num: %d, total length: %d\n", __func__,
		dev->msgs_num, total_len);

	return total_len;
}

static int hs_i2c_get_tx_len(int *alllen, int *o_tx_len, struct hs_i2c_dev *dev)
{
	int total_len;
	int tx_len;

	/* If total date length less than a fifodepth, not use DMA */
	total_len = hs_i2c_get_msg_total_len(dev);
	if ((u32)total_len < dev->tx_fifo_depth)
		return -EPERM;

	tx_len = total_len * sizeof(unsigned short);
	if (tx_len > I2C_MAX_DMA_BUF_LEN) {
		dev->msg_err = -EINVAL;
		dev_err(dev->dev, "Too long to send with DMA: %d\n", tx_len);
		return -EPERM;
	}

	*o_tx_len = tx_len;

	if (alllen)
		*alllen = total_len;

	return 0;
}

static int hs_i2c_prepare_dma_xfer(struct hs_i2c_dev *dev, int tx_len)
{
	int ret;
	int rx_len = 0;

	reinit_completion(&dev->dma_complete);
	dev->using_dma = true;

	ret = hs_i2c_dma_sg_init(dev, &dev->dmatx, tx_len);
	if (ret < 0) {
		dev_err(dev->dev, "Dma sg init failed. ret=%d\n", ret);
		return ret;
	}

	dev->using_tx_dma = true;
	hs_i2c_gen_dmatx_buf((u16 *)dev->dmatx.buf, &rx_len, dev);
	dev_dbg(dev->dev, "%s: dev->dmatx.sg.length: %u, tx_len: %d\n",
		__func__, dev->dmatx.sg.length, tx_len);

	if (rx_len > 0) {
		ret = hs_i2c_dma_sg_init(dev, &dev->dmarx, rx_len);
		if (ret < 0) {
			dev_err(dev->dev, "Dma sg init failed. ret=%d\n", ret);
			return ret;
		}

		dev->using_rx_dma = true;

		ret = hs_i2c_dma_rx_trigger_dma(dev);
		if (ret < 0) {
			dev_warn(dev->dev, "Dma rx failed\n");
			return ret;
		}
	}

	ret = hs_i2c_dma_tx_refill(dev);
	if (ret < 0) {
		dev_warn(dev->dev, "Dma tx failed\n");
		return ret;
	}

	return 0;
}

static int hs_i2c_xfer_msg_dma(struct hs_i2c_dev *dev, int *alllen)
{
	int tx_len = 0;
	int ret;

	ret = hs_i2c_xfer_msg_dma_check_ctrl(dev);
	if (ret) {
		dev_dbg(dev->dev, "%s check i2c master device fail\n",
			__func__);
		return ret;
	}

	/* If total date length less than a fifodepth, not use DMA */
	ret = hs_i2c_get_tx_len(alllen, &tx_len, dev);
	if (ret) {
		dev_dbg(dev->dev, "%s get tx len fail\n", __func__);
		return ret;
	}

	ret = hs_i2c_prepare_dma_xfer(dev, tx_len);
	if (ret < 0) {
		dev_dbg(dev->dev, "%s prepare xfer fail\n", __func__);
		goto clr_err;
	}

	i2c_v400_writel(dev, I2C_DMA_ENABLE, DMA_CTRL);

	ret = 0;
clr_err:
	if (ret < 0) {
		dev->using_dma = false;
		/* Restore for CPU transfer */
		dev->msg_write_idx = 0;
		dev->msg_read_idx = 0;
		dev->status &= ~STATUS_WRITE_IN_PROGRESS;
		dev_err(dev->dev, "%s return error %d\n", __func__, ret);
	}

	return ret;
}

static int hs_i2c_handle_transfer_error(struct hs_i2c_dev *dev)
{
	unsigned long error_source = dev->error_source;
	unsigned int i;

	if (error_source & (unsigned int)I2C_TRANS_ERROR_NOACK) {
		for_each_set_bit(i, &error_source, ARRAY_SIZE(error_sources))
			dev_dbg(dev->dev,
				"%s: %s\n", __func__, error_sources[i]);
		return -EREMOTEIO;
	}

	for_each_set_bit(i, &error_source, ARRAY_SIZE(error_sources))
		dev_err(dev->dev, "%s: %s\n", __func__, error_sources[i]);

	if (error_source & (unsigned int)I2C_TRANS_ERROR_GCALL_READ)
		return -EINVAL; /* wrong msgs[] data */
	else
		return -EIO;
}

static int hs_i2c_prepare_xfer(struct hs_i2c_dev *dev,
	struct i2c_msg msgs[], int num)
{
	u32 pin_state;
	int r;
	int ret;

	r = clk_enable(dev->clk);
	if (r) {
		dev_warn(dev->dev, "Unable to enable clock!\n");
		return -EINVAL;
	}

	r = i2c_pins_ctrl(dev, PINCTRL_STATE_DEFAULT);
	if (r < 0)
		dev_warn(dev->dev,
			"pins are not configured from the driver\n");

	pin_state = i2c_v400_readl(dev, TRANS_STATE) & PIN_STATE_MASK;
	if (pin_state != I2C_PIN_STATUS) {
		dev_err(dev->dev,
			"pins status are not correct!\n");
		goto err;
	}

	reinit_completion(&dev->cmd_complete);
	dev->msgs = msgs;
	dev->msgs_num = num;
	dev->cmd_err = 0;
	dev->msg_write_idx = 0;
	dev->msg_read_idx = 0;
	dev->msg_err = 0;
	dev->status = STATUS_IDLE;
	dev->error_source = 0;
	dev->rx_outstanding = 0;
	dev->using_tx_dma = false;
	dev->using_rx_dma = false;
	dev->dmarx.buf = NULL;
	dev->dmatx.buf = NULL;
	dev->using_dma = false;

	ret = hs_i2c_wait_bus_not_busy(dev);

	return ret;

err:
	clk_disable(dev->clk);
	r = i2c_pins_ctrl(dev, PINCTRL_STATE_IDLE);
	if (r < 0)
		dev_warn(dev->dev,
			"pins are not configured from the driver\n");
	return -EINVAL;
}

static int hs_i2c_wait_for_completion(struct hs_i2c_dev *dev,
	int totallen)
{
	int ret;

	ret = wait_for_completion_timeout(&dev->cmd_complete,
		WAIT_FOR_COMPLETION);
	if (ret == 0) {
		dev_err(dev->dev, "dev timed out\n");
		i2c_v400_writel(dev, 0, INT_MASK);
		if (dev->reset_controller)
			dev->reset_controller(dev);
		ret = -ETIMEDOUT;
	}
	return ret;
}

static int hs_i2c_wait_for_dma_completion(struct hs_i2c_dev *dev,
	int totallen)
{
	int ret;

	ret = wait_for_completion_timeout(&dev->dma_complete,
		WAIT_FOR_COMPLETION);
	i2c_v400_writel(dev, I2C_DMA_DISABLE, DMA_CTRL);
	if (ret == 0) {
		dev_err(dev->dev,
			"wait for dma complete timed out\n");
		if (dev->reset_controller)
			dev->reset_controller(dev);
		ret = -ETIMEDOUT;
	}
	return ret;
}

static void hs_i2c_bus_recovery(struct i2c_adapter *adap)
{
	struct hs_i2c_dev *dev = i2c_get_adapdata(adap);

	if (dev->recover_bus)
		dev->recover_bus(adap);
	else if (adap->bus_recovery_info &&
		adap->bus_recovery_info->recover_bus)
		adap->bus_recovery_info->recover_bus(adap);
	else
		dev_err(dev->dev, "no recovered routine\n");
}

static int
hs_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	struct hs_i2c_dev *dev = i2c_get_adapdata(adap);
	int totallen = 0;
	int ret;
	int r;

	if (!dev) {
		pr_err("%s:i2c ctrler doesn't be init\n", __func__);
		return -ENODEV;
	}
	mutex_lock(&dev->lock);

	ret = hs_i2c_prepare_xfer(dev, msgs, num);
	if (ret) {
		mutex_unlock(&dev->lock);
		return ret;
	}

	/* start the transfers */
	hs_i2c_xfer_init(dev);

	// TODO: fifo status judgment

	if (hs_i2c_xfer_msg_dma(dev, &totallen) < 0) {
		if (dev->msg_err < 0) {
			ret = dev->msg_err;
			goto done;
		}
	}

	if ((i2c_v400_readl(dev, FIFO_STATE) & I2C_FIFO_TX_EMPTY) &&
		!dev->using_dma)
		hs_i2c_xfer_msg(dev);

	/* wait for tx to complete */
	ret = hs_i2c_wait_for_completion(dev, totallen);
	if (!ret)
		goto done;

	if (dev->msg_err) {
		ret = dev->msg_err;
		goto done;
	}

	if ((!dev->cmd_err) && (dev->using_dma)) {
		ret = hs_i2c_wait_for_dma_completion(dev, totallen);
		if (!ret)
			goto done;
	}

	/* no error */
	if (likely(!dev->cmd_err)) {
		if ((dev->status &
				(STATUS_READ_IN_PROGRESS |
				STATUS_WRITE_IN_PROGRESS)) ||
				(I2C_INT_TX_EMPTY ==
				(i2c_v400_readl(dev, INT_MASK) &
				I2C_INT_TX_EMPTY))) {
			dev_err(dev->dev,
				"write or read is not complete, status 0x%x, len = %d.\n",
				dev->status, totallen);
			ret = -EAGAIN;
		} else {
			ret = num;
			dev->timeout_count = 0;
		}
		goto done;
	}

	/* We have an error */
	if (dev->cmd_err == I2C_INT_TRERR) {
		ret = hs_i2c_handle_transfer_error(dev);
		goto done;
	}
	ret = -EIO;

done:
	hs_i2c_dma_clear(dev);

	if (-ETIMEDOUT == ret) {
		hs_i2c_bus_recovery(adap);
		ret = -EAGAIN;
	}

	clk_disable(dev->clk);

	r = i2c_pins_ctrl(dev, PINCTRL_STATE_IDLE);
	if (r < 0)
		dev_warn(dev->dev,
			"pins are not configured from the driver\n");

	mutex_unlock(&dev->lock);
	if (ret < 0)
		dev_err(dev->dev, "error info %d, slave addr 0x%x.\n",
			ret, msgs[0].addr);

	return ret;
}

static u32 hs_i2c_func(struct i2c_adapter *adap)
{
	struct hs_i2c_dev *dev = i2c_get_adapdata(adap);

	return dev->functionality;
}

static struct i2c_algorithm hs_i2c_algo = {
	.master_xfer = hs_i2c_xfer,
	.functionality = hs_i2c_func,
};

static void hs_i2c_read_speed_mode(u32 *sp_mode, struct device *dev)
{
	int ret;
	u32 speed_mode = 0;

	ret = of_property_read_u32(dev->of_node, "speed-mode", &speed_mode);
	if (ret) {
		dev_info(dev, "no speed-mode property, use default!\n");
		speed_mode = HS_I2C_SPEED_STD;
	} else {
		if ((speed_mode != HS_I2C_SPEED_STD) &&
			(speed_mode != HS_I2C_SPEED_FAST) &&
			(speed_mode != HS_I2C_SPEED_HIGH))
			speed_mode = HS_I2C_SPEED_FAST;

		dev_info(dev, "speed mode is %d!\n", speed_mode);
	}

	*sp_mode = speed_mode;
}

static void hs_i2c_get_secure_slave(struct platform_device *pdev,
	struct hs_i2c_dev *d)
{
	struct device *dev = &pdev->dev;
	u32 secure_slave_num;
	u32 secure_slave_reg[HISI_SEC_SLAVE_MAX_NUM] = {0};
	struct arm_smccc_res res;
	int ret;
	u32 i;

	ret = of_property_read_u32(dev->of_node, "secure-slave-num",
		&secure_slave_num);
	if (ret) {
		secure_slave_num = HISI_DEFAULT_SEC_SLAVE_NUM;
		dev_info(dev, "doesn't have slave-num property, use default\n");
	}
	ret = of_property_read_u32_array(dev->of_node, "secure-slave-reg",
		&secure_slave_reg[0], ARRAY_SIZE(secure_slave_reg));
	if (ret) {
		secure_slave_reg[0] = HISI_DEFAULT_SEC_SLAVE;
		dev_info(dev, "doesn't have slave-reg property, use default\n");
	}

	for (i = 0; i < secure_slave_num; i++)
		arm_smccc_1_1_smc(HISI_SLAVE_WRITE_REG, d->reg_base,
			secure_slave_reg[i], &res);
}

static int hs_i2c_new_dev(struct hs_i2c_dev **i2c_dev,
	struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hs_i2c_dev *d = NULL;
	u32 secure_mode = 0;
	struct resource *iores = NULL;
	int ret;

	d = devm_kzalloc(dev, sizeof(struct hs_i2c_dev), GFP_KERNEL);
	if (!d)
		return -ENOMEM;

	d->dev = get_device(dev);

	iores = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!iores) {
		pr_err("%s get resource fail\n", __func__);
		return -EINVAL;
	}

	ret = of_property_read_u32(dev->of_node, "secure-mode", &secure_mode);
	if (ret)
		secure_mode = 0;
	pr_info("%s secure_mode=%u\n", __func__, secure_mode);
	d->secure_mode = secure_mode;

	d->mapbase = iores->start;
	if (secure_mode) {
		d->reg_base = iores->start;
		hs_i2c_get_secure_slave(pdev, d);
	} else {
		d->base = devm_ioremap(dev, iores->start,
			resource_size(iores));
		if (!d->base) {
			pr_err("%s ioremap fail\n", __func__);
			return -EADDRNOTAVAIL;
		}
	}

	d->pinctrl_flag = 0;
	d->pinctrl = NULL;

	*i2c_dev = d;

	return 0;
}

static int hs_i2c_dev_reset_cfg(struct hs_i2c_dev *idev,
	struct device *dev)
{
	int ret;
	u32 data[I2C_RESET_REG_CNT] = {0};

	ret = of_property_read_u32_array(dev->of_node, "reset-reg-base",
		&data[0], ARRAY_SIZE(data));
	if (ret) {
		dev_err(dev, "doesn't have reset-reg-base property!\n");
		return ret;
	} else {
		idev->reset_reg_base = devm_ioremap(dev, data[1], data[3]);
		dev_info(dev, "i2c reset register phy_addr is: %x\n", data[1]);
	}

	ret = of_property_read_u32(dev->of_node, "delay-off", &idev->delay_off);
	if (ret)
		dev_err(dev, "doesn't have delay-off property!\n");

	ret = of_property_read_u32_array(dev->of_node, "reset-controller-reg",
		&data[0], ARRAY_SIZE(data));
	if (ret) {
		dev_err(dev, "doesn't have reset-controller-reg property!\n");
		return ret;
	}

	/* data[0~2] is reset register address, data[3] is reset bit for i2c */
	idev->priv.reset_enable_off = data[0];
	idev->priv.reset_disable_off = data[1];
	idev->priv.reset_status_off = data[2];
	idev->priv.reset_bit = data[3];
	idev->reset_controller = reset_i2c_controller;
	idev->recover_bus = i2c_scl_recover_bus;
	return ret;
}

static int hs_i2c_init_clk_rate(struct device *dev, struct hs_i2c_dev *d)
{
	u64 clk_rate = 0;
	int ret;
	struct clk *i2c_clk = NULL;

	d->get_clk_rate_khz = hs_i2c_get_clk_rate_khz;
	i2c_clk = devm_clk_get(dev, "clk_i2c");
	if (IS_ERR(i2c_clk)) {
		dev_err(dev, "get clk fail\n");
		return -ENODEV;
	}

	d->clk = i2c_clk;
	ret = of_property_read_u64(dev->of_node, "clock-rate", &clk_rate);
	if (ret) {
		dev_info(dev, "doesn't have clock-rate property!\n");
		return 0;
	}

	dev_info(dev, "clock rate is %llu\n", clk_rate);
	ret = clk_set_rate(d->clk, clk_rate);
	if (ret) {
		dev_err(dev, "clock rate set failed r[0x%x]\n", ret);
		return -EINVAL;
	}

	return 0;
}

static struct i2c_bus_recovery_info *hs_i2c_new_init_rcov_inf(
	struct device *dev)
{
	struct i2c_bus_recovery_info *gpio_recovery_info = NULL;
	int gpio_scl, gpio_sda;

	gpio_recovery_info = devm_kzalloc(
		dev, sizeof(struct i2c_bus_recovery_info), GFP_KERNEL);
	if (!gpio_recovery_info)
		return NULL;

	gpio_scl = of_get_named_gpio(dev->of_node, "cs-gpios", 0);
	gpio_sda = of_get_named_gpio(dev->of_node, "cs-gpios", 1);
	dev_info(dev, "i2c cs-gpios = %d, %d!\n", gpio_scl, gpio_sda);
	if (gpio_scl == -ENOENT || gpio_sda == -ENOENT)
		dev_err(dev, "doesn't have gpio scl/sda property!\n");

	gpio_recovery_info->recover_bus = i2c_generic_gpio_recovery;
	gpio_recovery_info->prepare_recovery = i2c_prepare_recovery;
	gpio_recovery_info->unprepare_recovery = i2c_unprepare_recovery;
	gpio_recovery_info->scl_gpio = gpio_scl;
	gpio_recovery_info->sda_gpio = gpio_sda;

	return gpio_recovery_info;
}

static u32 hs_i2c_read_clear_intrbits(struct hs_i2c_dev *dev)
{
	u32 stat;

	stat = i2c_v400_readl(dev, INT_MSTAT);
	if (stat & I2C_INT_TRCMP)
		i2c_v400_writel(dev, I2C_INT_TRCMP, INT_CLR);
	if (stat & I2C_INT_FIFOERR)
		i2c_v400_writel(dev, I2C_INT_FIFOERR, INT_CLR);
	if (stat & I2C_INT_RX_AFULL)
		i2c_v400_writel(dev, I2C_INT_RX_AFULL, INT_CLR);
	if (stat & I2C_INT_TX_AEMPTY)
		i2c_v400_writel(dev, I2C_INT_TX_AEMPTY, INT_CLR);
	if (stat & I2C_INT_TRERR) {
		dev->error_source = i2c_v400_readl(dev, TRANS_ERR);
		i2c_v400_writel(dev, TRANS_ERR, INT_CLR);
	}

	return stat;
}

static int hs_i2c_irq_handler_master(struct hs_i2c_dev *dev)
{
	u32 stat;

	stat = hs_i2c_read_clear_intrbits(dev);
	if (stat & I2C_INT_TRERR) {
		dev->cmd_err |= I2C_INT_TRERR;
		dev->status = STATUS_IDLE;
		i2c_v400_writel(dev, 0, INT_MASK);
		goto transfer_err;
	}

	if (stat & I2C_INT_RX_AFULL)
		hs_i2c_read(dev);

	if ((stat & I2C_INT_TRCMP)) {
		if (!(dev->using_dma))
			hs_i2c_read(dev);
		goto transfer_err;
	}

transfer_err:
	if ((stat & (I2C_INT_TRERR | I2C_INT_TRCMP)) || dev->msg_err) {
		i2c_v400_writel(dev, 0, INT_MASK);
		complete(&dev->cmd_complete);
	}
	return 0;
}

static irqreturn_t i2c_isr(int this_irq, void *dev_id)
{
	struct hs_i2c_dev *dev = dev_id;

	if (!dev) {
		pr_err("%s: i2c contrller do not be init.\n", __func__);
		return IRQ_HANDLED;
	}
	spin_lock(&dev->spinlock);
	hs_i2c_irq_handler_master(dev);
	spin_unlock(&dev->spinlock);
	return IRQ_HANDLED;
}

static int hs_i2c_fill_i2c_dev(struct hs_i2c_dev *d, struct device *dev)
{
	u32 speed_mode = 0;
	u32 input_clock_khz;
	int ret;

	input_clock_khz = hs_i2c_get_clk_rate_khz(d);
	hs_i2c_read_speed_mode(&speed_mode, dev);

	d->functionality = I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR;

	d->master_cfg = I2C_RESTART_EN | speed_mode;

	ret = hs_i2c_reset_controller(d);
	if (ret) {
		dev_err(dev, "i2c:%s reset contoller fail", __func__);
		return ret;
	}

	d->tx_fifo_depth = I2C_FIFO_TX_DEPTH;
	d->rx_fifo_depth = I2C_FIFO_RX_DEPTH;
	dev_info(dev, "tx_fifo_depth: %u, rx_fifo_depth: %u\n",
		d->tx_fifo_depth, d->rx_fifo_depth);

	return 0;
}

static int hs_i2c_request_irq(
	struct hs_i2c_dev *d, struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;

	d->irq = platform_get_irq(pdev, 0);
	if (d->irq < 0) {
		dev_err(dev, "no irq resource\n");
		return -ENXIO;
	}

	spin_lock_init(&d->spinlock);

	ret = devm_request_irq(
		dev, d->irq, i2c_isr, IRQF_TRIGGER_NONE, pdev->name, d);
	if (ret) {
		dev_err(dev, "failure requesting irq %d\n", d->irq);
		return -EINVAL;
	}

	return 0;
}

static int hs_i2c_init_adapter(struct i2c_adapter *adap,
	struct platform_device *pdev, struct hs_i2c_dev *d)
{
	int ret;
	struct device *dev = &pdev->dev;
	int retries = 0;
	int timeout = 0;

	ret = of_property_read_s32(dev->of_node, "retries", &retries);
	if (ret) {
		dev_info(dev, "no retries ,use default!\n");
		retries = I2C_PROPER_RETRIES;
	} else {
		dev_info(dev, "retries is: %d!\n", retries);
	}

	ret = of_property_read_s32(dev->of_node, "timeout", &timeout);
	if (ret) {
		dev_info(dev, "no timeout ,use default!\n");
		timeout = I2C_TIMEOUT_DEFAULT_VAL;
	} else {
		dev_info(dev, "timeout is: %d!\n", timeout);
	}

	i2c_set_adapdata(adap, d);
	adap->owner = THIS_MODULE;
	adap->class = I2C_CLASS_HWMON;
	strlcpy(adap->name, "Hisilicon I2C adapter",
		sizeof(adap->name));
	adap->algo = &hs_i2c_algo;
	adap->dev.parent = dev;
	adap->dev.of_node = dev->of_node;
	adap->bus_recovery_info = hs_i2c_new_init_rcov_inf(dev);
	if (!adap->bus_recovery_info) {
		dev_err(dev, "bus_recovery_info is NULL\n");
		return -ENOMEM;
	}

	adap->nr = pdev->id;
	adap->retries = retries;
	adap->timeout = timeout;
	dev_info(dev, "adap->retries = %d adap->timeout = %d\n ", adap->retries,
		adap->timeout);

	ret = i2c_add_numbered_adapter(adap);
	if (ret) {
		dev_err(dev, "failure adding adapter\n");
		return ret;
	}

	return 0;
}

static int hs_i2c_master_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hs_i2c_dev *d = NULL;

	if (hs_i2c_new_dev(&d, pdev) || !d) {
		pr_err("%s: new dev fail\n", __func__);
		return -ENODEV;
	}

	if (hs_i2c_dev_reset_cfg(d, dev)) {
		pr_err("%s: reset cfg fail\n", __func__);
		return -1;
	}

	if (hs_i2c_init_clk_rate(dev, d)) {
		pr_err("%s: init clk fail\n", __func__);
		return -1;
	}

	if (clk_prepare_enable(d->clk)) {
		dev_err(dev, "Unable to enable clock!\n");
		return -1;
	}

	init_completion(&d->cmd_complete);
	init_completion(&d->dma_complete);
	mutex_init(&d->lock);

	if (hs_i2c_fill_i2c_dev(d, dev)) {
		dev_err(dev, "%s: fill i2c dev fail\n", __func__);
		goto clr_err;
	}

	if (hs_i2c_init_master(d)) {
		dev_err(dev, "%s: init master fail\n", __func__);
		goto clr_err;
	}

	hs_i2c_disable_int(d);

	if (hs_i2c_request_irq(d, pdev)) {
		dev_err(dev, "%s: request irq fail\n", __func__);
		goto clr_err;
	}

	if (hs_i2c_init_adapter(&d->adapter, pdev, d)) {
		dev_err(dev, "%s: init adapter fail\n", __func__);
		goto clr_err;
	}

	platform_set_drvdata(pdev, d);

	/* DMA probe */
	hs_i2c_dma_probe(d);

	clk_disable(d->clk);

	dev_info(dev, "i2c probe succeed!\n");
	return 0;

clr_err:
	clk_disable_unprepare(d->clk);
	d->clk = NULL;
	put_device(dev);

	return -1;
}

static int hs_i2c_master_remove(struct platform_device *pdev)
{
	struct hs_i2c_dev *d = platform_get_drvdata(pdev);

	if (!d) {
		pr_err("%s: get drvdata failed\n", __func__);
		return -EINVAL;
	}

	platform_set_drvdata(pdev, NULL);
	i2c_del_adapter(&d->adapter);
	put_device(&pdev->dev);
	clk_disable_unprepare(d->clk);
	devm_clk_put(&pdev->dev, d->clk);
	d->clk = NULL;

	hs_i2c_dma_remove(d);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id hs_i2c_master_of_match[] = {
	{
		.compatible = "hisilicon,hisi-i2c-master",
	},
	{},
};
MODULE_DEVICE_TABLE(of, hs_i2c_master_of_match);
#endif

#ifdef CONFIG_PM
static int hs_i2c_master_suspend(struct device *dev)
{
	unsigned long time, timeout;
	struct platform_device *pdev = to_platform_device(dev);
	struct hs_i2c_dev *i_dev = platform_get_drvdata(pdev);

	if (!i_dev) {
		pr_err("%s: get drvdata failed\n", __func__);
		return -EINVAL;
	}

	dev_info(&pdev->dev, "%s: suspend +\n", __func__);

	timeout = jiffies + msecs_to_jiffies(GET_DEV_LOCK_TIMEOUT);
	while (!mutex_trylock(&i_dev->lock)) {
		time = jiffies;
		if (time_after(time, timeout)) {
			dev_info(&pdev->dev,
				"%s: mutex_trylock timeout fail\n", __func__);
			return -EAGAIN;
		}

		/* sleep between 1000 and 2000 us */
		usleep_range(1000, 2000);
	}

	dev_info(&pdev->dev, "%s: suspend -\n", __func__);
	return 0;
}

static int hs_i2c_master_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct hs_i2c_dev *i_dev = platform_get_drvdata(pdev);
	int ret;

	if (!i_dev) {
		pr_err("%s: get drvdata failed\n", __func__);
		return -EINVAL;
	}

	dev_info(&pdev->dev, "%s: resume +\n", __func__);

	ret = clk_enable(i_dev->clk);
	if (ret) {
		dev_err(&pdev->dev, "clk_prepare_enable failed!\n");
		return -EAGAIN;
	}

	/* reset controller failure does not affect the resume */
	ret = hs_i2c_reset_controller(i_dev);
	if (ret)
		dev_err(&pdev->dev, "reset controller fail!\n");

	ret = hs_i2c_init_master(i_dev);
	if (ret)
		dev_err(&pdev->dev, "i2c master init fail!\n");

	hs_i2c_disable_int(i_dev);
	clk_disable(i_dev->clk);

	mutex_unlock(&i_dev->lock);

	dev_info(&pdev->dev, "%s: resume -\n", __func__);
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(
	hs_i2c_master_dev_pm_ops, hs_i2c_master_suspend, hs_i2c_master_resume);

static struct platform_driver hs_i2c_master_driver = {
	.probe = hs_i2c_master_probe,
	.remove = hs_i2c_master_remove,
	.driver = {
		.name = "hs_i2c_master",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hs_i2c_master_of_match),
		.pm = &hs_i2c_master_dev_pm_ops,
	},
};

static int __init i2c_init(void)
{
	return platform_driver_register(&hs_i2c_master_driver);
}

static void __exit i2c_exit(void)
{
	platform_driver_unregister(&hs_i2c_master_driver);
}

subsys_initcall(i2c_init);
module_exit(i2c_exit);

MODULE_DESCRIPTION("I2C_V400 Master bus adapter");
MODULE_ALIAS("platform:i2c_master-hs");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
