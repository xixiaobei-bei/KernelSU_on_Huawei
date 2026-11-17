/*
 * Synopsys DesignWare I2C adapter driver (master only).
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
#include "i2c-designware-core.h"

#define HISI_DW_IC_CON 0x0
#define HISI_DW_IC_TAR 0x4
#define HISI_DW_IC_ENABLE 0x6C
#define HISI_DW_IC_STATUS 0x70
#define HISI_DW_IC_TXFLR 0x74
#define HISI_DW_IC_RXFLR 0x78
#define DW_IC_DMA_CR_MASK 0x3

#define DW_IC_DMA_CR 0x88
#define DW_IC_DMA_TDLR 0x8C
#define DW_IC_DMA_RDLR 0x90

#define HISI_DW_IC_TX_ABRT_SOURCE 0x80
#define HISI_DW_IC_DATA_CMD 0x10
#define HISI_DW_IC_ERR_TX_ABRT 0x1

#define DW_IC_TXDMAE (1 << 1) /* enable transmit dma */
#define DW_IC_RXDMAE (1 << 0) /* enable receive dma */

#define HISI_DW_IC_RD_CFG (1 << 8)
#define HISI_DW_IC_CON_RECOVERY_CFG 0x43
#define HISI_DW_IC_TAR_RECOVERY_CFG 0x7F

#define HISI_STATUS_WRITE_IN_PROGRESS 0x1

#define I2C_DW_MAX_DMA_BUF_LEN (60 * 1024)

#define I2C_DELAY_OFF_NS 70
#define I2C_SCL_HIGHT_NS 100
#define I2C_SCL_LOW_NS 200
#define I2C_SDA_HOLD_NS 300
#define I2C_SDA_FALLING_NS 300
#define I2C_SCL_FALLING_NS 300

#define I2C_TIMEOUT_DEFAULT_VAL 100
#define I2C_RECOVER_SLEEP_MS 100

#define I2C_KHZ 1000
#define I2C_WAIT_STATUS_TIMES 10

#define GET_DEV_LOCK_TIMEOUT 500
#define SPEED_INTERVAL 1000000
#define I2C_DMA_MAX_BURST 16
#define I2C_DMA_RDLR_BURST 15

#define I2C_FIFO_TX_DEPTH_SHIFT 16
#define I2C_FIFO_RX_DEPTH_SHIFT 8

#define I2C_PROPER_RETRIES 3
#define I2C_RESET_REG_CNT 4

#define I2C_IRQ_STOP BIT(9)
#define I2C_READ_CMD 0x100

#define HISI_SEC_SLAVE_MAX_NUM 2
#define HISI_DEFAULT_SEC_SLAVE 0x28
#define HISI_DEFAULT_SEC_SLAVE_NUM 1
#define HISI_SLAVE_WRITE_REG            0xC5010002

#ifdef CONFIG_HUAWEI_DSM
static struct dsm_dev dsm_i2c = {
	.name = "dsm_i2c",
	.device_name = "i2c_v300",
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};
#endif
struct i2c_adapter *device_adap_addr;

static u32 hs_i2c_dw_get_clk_rate_khz(struct dw_i2c_dev *dev)
{
	u32 rate;

	rate = clk_get_rate(dev->clk) / I2C_KHZ;
	dev_dbg(dev->dev, "input_clock_khz value is %u\n", rate);

	return rate;
}

int dw_i2c_pins_ctrl(struct dw_i2c_dev *dev, const char *name)
{
	struct dw_i2c_controller *controller = dev->priv_data;
	struct pinctrl_state *s = NULL;
	int ret;

	if (!controller) {
		dev_err(dev->dev, "%s:i2c ctrler doesn't be init\n", __func__);
		return -ENODEV;
	}

	if (!controller->pinctrl_flag) {
		controller->pinctrl = devm_pinctrl_get(dev->dev);
		if (IS_ERR(controller->pinctrl)) {
			dev_err(dev->dev, "%s: get pinctrl fail\n", __func__);
			return -1;
		}
		controller->pinctrl_flag = 1;
	}

	s = pinctrl_lookup_state(controller->pinctrl, name);
	if (IS_ERR(s)) {
		devm_pinctrl_put(controller->pinctrl);
		controller->pinctrl_flag = 0;
		dev_err(dev->dev, "%s: lookup state fail\n", __func__);
		return -1;
	}

	ret = pinctrl_select_state(controller->pinctrl, s);
	if (ret < 0) {
		devm_pinctrl_put(controller->pinctrl);
		controller->pinctrl_flag = 0;
		dev_err(dev->dev, "%s: select state fail\n", __func__);
		return -1;
	}

	return 0;
}

static int hs_i2c_dw_reset_controller(struct dw_i2c_dev *dev)
{
	struct dw_i2c_controller *ctrl = dev->priv_data;
	struct hs_i2c_priv_data *priv = NULL;
	u32 val = 0;
	int timeout = I2C_WAIT_STATUS_TIMES;

	if (!ctrl) {
		dev_err(dev->dev, "%s:i2c ctrler doesn't be init\n", __func__);
		return -ENODEV;
	}

	priv = &ctrl->priv;

	writel(BIT(priv->reset_bit),
		ctrl->reset_reg_base + priv->reset_enable_off);
	do {
		val = readl(ctrl->reset_reg_base + priv->reset_status_off);
		val &= BIT(priv->reset_bit);
		udelay(1);
	} while (!val && timeout--);

	if (timeout <= 0) {
		dev_err(dev->dev, "%s:i2c reset enable timeout\n", __func__);
		return -ENODEV;
	}

	timeout = I2C_WAIT_STATUS_TIMES;

	writel(BIT(priv->reset_bit),
		ctrl->reset_reg_base + priv->reset_disable_off);
	do {
		val = readl(ctrl->reset_reg_base + priv->reset_status_off);
		val &= BIT(priv->reset_bit);
		udelay(1);
	} while (val && timeout--);

	if (timeout <= 0) {
		dev_err(dev->dev, "%s:i2c reset disable timeout\n", __func__);
		return -ENODEV;
	}

	return 0;
}

void reset_i2c_controller(struct dw_i2c_dev *dev)
{
	struct dw_i2c_controller *controller = dev->priv_data;
	int r;

	if (!controller) {
		dev_err(dev->dev, "%s:i2c ctrler doesn't be init\n", __func__);
		return;
	}

	disable_irq(dev->irq);

	r = dw_i2c_pins_ctrl(dev, PINCTRL_STATE_IDLE);
	if (r < 0)
		dev_warn(dev->dev, "pins are not configured from the driver\n");

	r = hs_i2c_dw_reset_controller(dev);
	if (r)
		dev_err(dev->dev, "%s:i2c resest controller fail\n", __func__);

	i2c_dw_init_master(dev);
	i2c_dw_disable_int(dev);

	r = dw_i2c_pins_ctrl(dev, PINCTRL_STATE_DEFAULT);
	if (r < 0)
		dev_warn(dev->dev, "pins are not configured from the driver\n");

	enable_irq(dev->irq);
}

#ifdef CONFIG_DMA_ENGINE
static void i2c_dw_dma_probe_initcall(struct dw_i2c_dev *dev)
{
	struct dma_chan *chan = NULL;
	dma_cap_mask_t mask;
	struct dma_slave_config tx_conf = {};
	struct dma_slave_config rx_conf = {};
	struct dw_i2c_controller *controller = dev->priv_data;

	if (!controller) {
		dev_err(dev->dev, "%s:i2c ctrler doesn't be init\n", __func__);
		return;
	}

	/* DMA is the sole user of the platform data right now */
	tx_conf.dst_addr = controller->mapbase + HISI_DW_IC_DATA_CMD;
	tx_conf.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	tx_conf.direction = DMA_MEM_TO_DEV;
	tx_conf.dst_maxburst = I2C_DMA_MAX_BURST;

	rx_conf.src_addr = controller->mapbase + HISI_DW_IC_DATA_CMD;
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
	controller->dmatx.chan = chan;

	dev_info(dev->dev, "DMA channel TX %s-%d\n",
		dma_chan_name(controller->dmatx.chan),
		controller->dmatx.chan->chan_id);

	chan = dma_request_slave_channel(dev->dev, "rx");
	if (!chan) {
		dev_err(dev->dev, "no RX DMA channel!\n");
		return;
	}

	dmaengine_slave_config(chan, &rx_conf);
	controller->dmarx.chan = chan;

	dev_info(dev->dev, "DMA channel RX %s-%d\n",
		dma_chan_name(controller->dmarx.chan),
		controller->dmarx.chan->chan_id);
}

static void i2c_dw_dma_probe(struct dw_i2c_dev *dev)
{
	i2c_dw_dma_probe_initcall(dev);
}

static void i2c_dw_dma_remove(struct dw_i2c_dev *dev)
{
	struct dw_i2c_controller *controller = dev->priv_data;

	if (!controller) {
		dev_err(dev->dev, "%s:i2c ctrler doesn't be init\n", __func__);
		return;
	}

	/* remove the initcall if it has not yet executed */
	if (controller->dmatx.chan)
		dma_release_channel(controller->dmatx.chan);
	if (controller->dmarx.chan)
		dma_release_channel(controller->dmarx.chan);
}

/*
 * The current DMA TX buffer has been sent.
 * Try to queue up another DMA buffer.
 */
static void i2c_dw_dma_tx_callback(void *data)
{
	struct dw_i2c_dev *dev = data;
	struct dw_i2c_controller *controller = dev->priv_data;

	if (!controller) {
		dev_err(dev->dev, "%s:i2c ctrler doesn't be init\n", __func__);
		return;
	}

	dev_dbg(dev->dev, "%s: entry\n", __func__);

	controller->dmacr &= ~DW_IC_TXDMAE;
	dw_writel(dev, controller->dmacr & DW_IC_DMA_CR_MASK, DW_IC_DMA_CR);
	controller->using_tx_dma = false;

	if (!(controller->using_tx_dma) && !(controller->using_rx_dma))
		complete(&controller->dma_complete);
}

static int i2c_dw_dma_tx_refill(struct dw_i2c_dev *dev)
{
	struct dw_i2c_dma_data *dmatx = NULL;
	struct dma_chan *chan = NULL;
	struct dma_device *dma_dev = NULL;
	struct dma_async_tx_descriptor *desc = NULL;
	struct dw_i2c_controller *controller = dev->priv_data;

	if (!controller) {
		dev_err(dev->dev, "%s:i2c ctrler doesn't be init\n", __func__);
		return -ENODEV;
	}

	dmatx = &controller->dmatx;
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
	desc->callback = i2c_dw_dma_tx_callback;
	desc->callback_param = dev;

	desc->tx_submit(desc);

	/* Fire the DMA transaction */
	dma_dev->device_issue_pending(chan);

	return 0;
}

static void i2c_dw_dma_rx_callback(void *data)
{
	struct dw_i2c_dev *dev = data;
	struct dw_i2c_controller *controller = dev->priv_data;
	struct i2c_msg *msgs = NULL;
	struct dw_i2c_dma_data *dmarx = NULL;
	int rx_valid;
	int rd_idx = 0;
	u32 len;
	u8 *buf = NULL;

	dev_dbg(dev->dev, "%s: entry\n", __func__);

	if (!controller) {
		dev_err(dev->dev, "%s:i2c ctrler doesn't be init\n", __func__);
		return;
	}

	msgs = dev->msgs;
	dmarx = &controller->dmarx;
	rx_valid = dmarx->sg.length;

	/* Sync in buffer , only 1 mapped entry */
	dma_sync_sg_for_cpu(dev->dev, &dmarx->sg, 1, DMA_FROM_DEVICE);

	controller->dmacr &= ~DW_IC_RXDMAE;
	dw_writel(dev, controller->dmacr & DW_IC_DMA_CR_MASK, DW_IC_DMA_CR);

	for (; dev->msg_read_idx < dev->msgs_num; dev->msg_read_idx++) {
		if (!(msgs[dev->msg_read_idx].flags & I2C_M_RD))
			continue;

		len = msgs[dev->msg_read_idx].len;
		buf = msgs[dev->msg_read_idx].buf;

		for (; len > 0 && rx_valid > 0; len--, rx_valid--)
			*buf++ = dmarx->buf[rd_idx++];
	}

	controller->using_rx_dma = false;

	if (!controller->using_tx_dma && !controller->using_rx_dma)
		complete(&controller->dma_complete);
}

static int i2c_dw_dma_rx_trigger_dma(struct dw_i2c_dev *dev)
{
	struct dw_i2c_dma_data *dmarx = NULL;
	struct dma_chan *rxchan = NULL;
	struct dma_device *dma_dev = NULL;
	struct dma_async_tx_descriptor *desc = NULL;
	struct dw_i2c_controller *controller = dev->priv_data;

	if (!controller) {
		dev_err(dev->dev, "%s:i2c ctrler doesn't be init\n", __func__);
		return -ENODEV;
	}

	dmarx = &controller->dmarx;
	rxchan = dmarx->chan;
	if (!rxchan) {
		dev_err(dev->dev, "rxchan is NULL!\n");
		return -EIO;
	}

	dma_dev = rxchan->device;

	dev_dbg(dev->dev, "%s, %u bytes to read\n",
		__func__, controller->dmarx.sg.length);

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
	desc->callback = i2c_dw_dma_rx_callback;
	desc->callback_param = dev;

	/* All errors should happen at prepare time */
	dmaengine_submit(desc);

	/* Fire the DMA transaction */
	dma_async_issue_pending(rxchan);

	return 0;
}

static int i2c_dw_dma_sg_init(struct dw_i2c_dev *dev,
	struct dw_i2c_dma_data *dma_data, unsigned long length)
{
	dma_data->buf = devm_kzalloc(dev->dev, length, GFP_KERNEL);
	if (!dma_data->buf)
		return -ENOMEM;

	sg_init_one(&dma_data->sg, dma_data->buf, length);

	return 0;
}

void i2c_dw_dma_clear(struct dw_i2c_dev *dev)
{
	struct dw_i2c_controller *controller = dev->priv_data;

	if (!controller) {
		dev_err(dev->dev, "%s:i2c ctrler doesn't be init\n", __func__);
		return;
	}

	if (controller->dmatx.buf) {
		dmaengine_terminate_all(controller->dmatx.chan);
		dma_unmap_sg(controller->dmatx.chan->device->dev,
			&controller->dmatx.sg, 1, DMA_TO_DEVICE);
		devm_kfree(dev->dev, controller->dmatx.buf);
		controller->dmatx.buf = NULL;
		controller->using_tx_dma = false;
	}

	if (controller->dmarx.buf) {
		dmaengine_terminate_all(controller->dmarx.chan);
		dma_unmap_sg(controller->dmarx.chan->device->dev,
			&controller->dmarx.sg, 1, DMA_FROM_DEVICE);
		devm_kfree(dev->dev, controller->dmarx.buf);
		controller->dmarx.buf = NULL;
		controller->using_rx_dma = false;
	}

	controller->using_dma = false;
	controller->dmacr = 0;
	dw_writel(dev, controller->dmacr & DW_IC_DMA_CR_MASK, DW_IC_DMA_CR);
}

void i2c_dw_dma_fifo_cfg(struct dw_i2c_dev *dev)
{
	dw_writel(dev, dev->tx_fifo_depth - I2C_DMA_MAX_BURST, DW_IC_DMA_TDLR);
	dw_writel(dev, I2C_DMA_RDLR_BURST, DW_IC_DMA_RDLR);
}

#else
/* Blank functions if the DMA engine is not available */
void i2c_dw_dma_probe(struct dw_i2c_dev *dev)
{
}

void i2c_dw_dma_remove(struct dw_i2c_dev *dev)
{
}

static int i2c_dw_dma_tx_refill(struct dw_i2c_dev *dev)
{
	return -EIO;
}

static int i2c_dw_dma_rx_trigger_dma(struct dw_i2c_dev *dev)
{
	return -EIO;
}

static int i2c_dw_dma_sg_init(struct dw_i2c_dev *dev,
	struct dw_i2c_dma_data *dma_data, unsigned long length)
{
	return -EIO;
}

void i2c_dw_dma_clear(struct dw_i2c_dev *dev)
{
}

void i2c_dw_dma_fifo_cfg(struct dw_i2c_dev *dev)
{
}

#endif

static void i2c_dw_gen_dmatx_buf(u16 *dma_txbuf, int *o_rx_len,
	struct dw_i2c_dev *dev)
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

static int i2c_dw_xfer_msg_dma_check_ctrl(
	struct dw_i2c_dev *dev, struct dw_i2c_controller *controller)
{
	if (!controller) {
		dev_err(dev->dev, "%s:i2c ctrler doesn't be init\n", __func__);
		return -ENODEV;
	}

	if (!controller->dmatx.chan || !controller->dmarx.chan)
		return -EPERM;

	return 0;
}

static int i2c_dw_get_msg_total_len(struct dw_i2c_dev *dev)
{
	int i;
	int total_len = 0;

	for (i = dev->msg_write_idx; i < dev->msgs_num; i++)
		total_len += dev->msgs[i].len;

	dev_dbg(dev->dev, "%s: msg num: %d, total length: %d\n", __func__,
		dev->msgs_num, total_len);

	return total_len;
}

static int i2c_dw_get_tx_len(int *alllen, int *o_tx_len, struct dw_i2c_dev *dev)
{
	int total_len;
	int tx_len;

	/* If total date length less than a fifodepth, not use DMA */
	total_len = i2c_dw_get_msg_total_len(dev);
	if ((u32)total_len < dev->tx_fifo_depth)
		return -EPERM;

	tx_len = total_len * sizeof(unsigned short);
	if (tx_len > I2C_DW_MAX_DMA_BUF_LEN) {
		dev->msg_err = -EINVAL;
		dev_err(dev->dev, "Too long to send with DMA: %d\n", tx_len);
		return -EPERM;
	}

	*o_tx_len = tx_len;

	if (alllen)
		*alllen = total_len;

	return 0;
}

static int i2c_dw_prepare_xfer(struct dw_i2c_dev *dev,
	struct dw_i2c_controller *controller, int tx_len)
{
	int ret;
	int rx_len = 0;

	reinit_completion(&controller->dma_complete);
	controller->using_dma = true;

	ret = i2c_dw_dma_sg_init(dev, &controller->dmatx, tx_len);
	if (ret < 0) {
		dev_err(dev->dev, "Dma sg init failed. ret=%d\n", ret);
		return ret;
	}

	controller->using_tx_dma = true;
	i2c_dw_gen_dmatx_buf((u16 *)controller->dmatx.buf, &rx_len, dev);
	dev_dbg(dev->dev, "%s: dev->dmatx.sg.length: %u, tx_len: %d\n",
		__func__, controller->dmatx.sg.length, tx_len);

	if (rx_len > 0) {
		ret = i2c_dw_dma_sg_init(dev, &controller->dmarx, rx_len);
		if (ret < 0) {
			dev_err(dev->dev, "Dma sg init failed. ret=%d\n", ret);
			return ret;
		}

		controller->using_rx_dma = true;

		ret = i2c_dw_dma_rx_trigger_dma(dev);
		if (ret < 0) {
			dev_warn(dev->dev, "Dma rx failed\n");
			return ret;
		}

		controller->dmacr |= DW_IC_RXDMAE;
	}

	ret = i2c_dw_dma_tx_refill(dev);
	if (ret < 0) {
		dev_warn(dev->dev, "Dma tx failed\n");
		return ret;
	}

	controller->dmacr |= DW_IC_TXDMAE;

	return 0;
}

int i2c_dw_xfer_msg_dma(struct dw_i2c_dev *dev, int *alllen)
{
	struct dw_i2c_controller *controller = dev->priv_data;
	int tx_len = 0;
	int ret;

	ret = i2c_dw_xfer_msg_dma_check_ctrl(dev, controller);
	if (ret) {
		dev_dbg(dev->dev, "%s check controller fail\n", __func__);
		return ret;
	}

	/* If total date length less than a fifodepth, not use DMA */
	ret = i2c_dw_get_tx_len(alllen, &tx_len, dev);
	if (ret) {
		dev_dbg(dev->dev, "%s get tx len fail\n", __func__);
		return ret;
	}

	ret = i2c_dw_prepare_xfer(dev, controller, tx_len);
	if (ret < 0) {
		dev_dbg(dev->dev, "%s prepare xfer fail\n", __func__);
		goto clr_err;
	}

	dw_writel(dev, controller->dmacr & DW_IC_DMA_CR_MASK, DW_IC_DMA_CR);

	ret = 0;
clr_err:
	if (ret < 0) {
		controller->using_dma = false;
		/* Restore for CPU transfer */
		dev->msg_write_idx = 0;
		dev->msg_read_idx = 0;
		dev->status &= ~HISI_STATUS_WRITE_IN_PROGRESS;
		dev_err(dev->dev, "%s return error %d\n", __func__, ret);
	}

	return ret;
}

/*
 * This routine does i2c bus recovery by using i2c_generic_gpio_recovery
 * which is provided by I2C Bus recovery infrastructure.
 */
static void dw_i2c_prepare_recovery(struct i2c_adapter *adap)
{
	struct dw_i2c_dev *dev = i2c_get_adapdata(adap);
	int ret;

	ret = dw_i2c_pins_ctrl(dev, PINCTRL_STATE_IDLE);
	if (ret < 0)
		dev_warn(dev->dev, "pins are not configured to idle\n");
}

static void dw_i2c_unprepare_recovery(struct i2c_adapter *adap)
{
	struct dw_i2c_dev *dev = i2c_get_adapdata(adap);
	int ret;

	ret = dw_i2c_pins_ctrl(dev, PINCTRL_STATE_DEFAULT);
	if (ret < 0)
		dev_warn(dev->dev, "pins are not configured to default\n");
}

void dw_i2c_scl_recover_bus(struct i2c_adapter *adap)
{
	struct dw_i2c_dev *dev = i2c_get_adapdata(adap);
	struct i2c_bus_recovery_info *bri = adap->bus_recovery_info;
	int ret;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	int sda_gpio;
#endif

	dev_info(dev->dev, "bus prepares recovery ...\n");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	sda_gpio = desc_to_gpio(bri->sda_gpiod);
	ret = gpio_request_one(sda_gpio, GPIOF_IN, "i2c-sda");
	if (ret)
		dev_warn(dev->dev,
			"Can't get SDA gpio: %d. Not using SDA polling\n",
			sda_gpio);
#else
	ret = gpio_request_one(bri->sda_gpio, GPIOF_IN, "i2c-sda");
	if (ret)
		dev_warn(dev->dev,
			"Can't get SDA gpio: %d. Not using SDA polling\n",
			bri->sda_gpio);
#endif
	/* disable IC, 0:Disable */
	dw_writel(dev, 0, HISI_DW_IC_ENABLE);

	/* speed is 100KHz */
	dw_writel(dev, HISI_DW_IC_CON_RECOVERY_CFG, HISI_DW_IC_CON);

	/* config slave address to 0x7F */
	dw_writel(dev, HISI_DW_IC_TAR_RECOVERY_CFG, HISI_DW_IC_TAR);
	/* enable IC, 1:Enable */
	dw_writel(dev, 1, HISI_DW_IC_ENABLE);
	/* recived data from bus */
	dw_writel(dev, HISI_DW_IC_RD_CFG, HISI_DW_IC_DATA_CMD);

	msleep(I2C_RECOVER_SLEEP_MS);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	gpio_free(sda_gpio);
#else
	gpio_free(bri->sda_gpio);
#endif

	ret = dw_i2c_pins_ctrl(dev, PINCTRL_STATE_DEFAULT);
	if (ret < 0)
		dev_warn(dev->dev, "pins are not configured to default\n");

	i2c_dw_init_master(dev);

	dev_info(dev->dev, "bus recovered completely!\n");
}

int i2c_init_secos(struct i2c_adapter *adap)
{
	int ret;
	struct dw_i2c_dev *dev = NULL;

	if (!adap) {
		pr_err("%s: i2c adapter is NULL!\n", __func__);
		return -ENODEV;
	}

	dev = i2c_get_adapdata(adap);
	if (!dev) {
		pr_err("%s: can not get i2c dev!\n", __func__);
		return -ENODEV;
	}

	mutex_lock(&dev->lock);

	ret = dw_i2c_pins_ctrl(dev, PINCTRL_STATE_DEFAULT);
	if (ret < 0) {
		dev_err(dev->dev, "%s: pins are not configured to DEFAULT!\n",
			__func__);
		goto clr_err;
	}

	return 0;

clr_err:
	mutex_unlock(&dev->lock);
	return -EINVAL;
}
EXPORT_SYMBOL_GPL(i2c_init_secos);

int i2c_exit_secos(struct i2c_adapter *adap)
{
	struct dw_i2c_dev *dev = NULL;
	int ret;

	if (!adap) {
		pr_err("%s: i2c adapter is NULL!\n", __func__);
		return -ENODEV;
	}

	dev = i2c_get_adapdata(adap);
	if (!dev) {
		pr_err("%s: can not get i2c dev!\n", __func__);
		return -ENODEV;
	}

	ret = dw_i2c_pins_ctrl(dev, PINCTRL_STATE_IDLE);
	if (ret < 0)
		dev_err(dev->dev, "%s:pins are not cfged to IDLE!\n", __func__);

	mutex_unlock(&dev->lock);

	return 0;
}
EXPORT_SYMBOL_GPL(i2c_exit_secos);

static struct i2c_algorithm hs_i2c_dw_algo = {
	.master_xfer = i2c_dw_xfer,
	.functionality = i2c_dw_func,
};

static void i2c_frequency_division(struct dw_i2c_dev *dev, u32 clk)
{
	u32 sda_falling_time, scl_falling_time;

	sda_falling_time = dev->sda_falling_time ? : I2C_SDA_FALLING_NS; /* ns */
	scl_falling_time = dev->scl_falling_time ? : I2C_SCL_FALLING_NS; /* ns */

	dev->hs_hcnt = i2c_dw_scl_hcnt(
		clk, I2C_SCL_HIGHT_NS,   /* tHD;STA = tHIGH = 0.1 us */
		sda_falling_time,        /* tf = 0.3 us */
		0,                       /* 0: DW default, 1: Ideal */
		0);                      /* No offset */
	dev->hs_lcnt = i2c_dw_scl_lcnt(clk, I2C_SCL_LOW_NS, /* tLOW = 0.2 us */
		scl_falling_time,        /* tf = 0.3 us */
		0);                      /* No offset */
}

static void hs_dw_i2c_get_secure_slave(struct platform_device *pdev,
	struct dw_i2c_dev *d)
{
	struct device *dev = &pdev->dev;
	u32 secure_slave_num = 0;
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
		&secure_slave_reg[0], secure_slave_num);
	if (ret) {
		secure_slave_reg[0] = HISI_DEFAULT_SEC_SLAVE;
		dev_info(dev, "doesn't have slave-reg property, use default\n");
	}

	for (i = 0; i < secure_slave_num; i++)
		arm_smccc_1_1_smc(HISI_SLAVE_WRITE_REG, d->reg_base,
			secure_slave_reg[i], &res);
}

static int hs_dw_i2c_new_dev(struct dw_i2c_dev **i2c_dev,
	struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dw_i2c_dev *d = NULL;
	struct dw_i2c_controller *ctrl = NULL;
	u32 secure_mode = 0;
	struct resource *iores = NULL;
	int ret;

	d = devm_kzalloc(dev, sizeof(struct dw_i2c_dev), GFP_KERNEL);
	if (!d)
		return -ENOMEM;

	ctrl = devm_kzalloc(dev, sizeof(struct dw_i2c_controller), GFP_KERNEL);
	if (!ctrl)
		return -ENOMEM;

	d->dev = get_device(dev);
	d->priv_data = ctrl;

	/* NOTE:driver uses the static register mapping, resource index is 0 */
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

#ifdef CONFIG_HUAWEI_DSM
	/* DMD register */
	if (of_property_read_u32(dev->of_node,
		"dmd_support",
		&(d->dmd_support))) {
			dev_info(dev, "doesn't have dmd_support property!\n");
			d->dmd_support = 0;
		}

	if ((d->dmd_support) && (!d->i2c_dclient)) {
		d->dsm_count = 0;
		d->i2c_dclient = dsm_register_client(&dsm_i2c);
	}
#endif

	ctrl->mapbase = iores->start;
	if (secure_mode) {
		d->reg_base = iores->start;
		hs_dw_i2c_get_secure_slave(pdev, d);
	} else {
		d->base = ioremap(iores->start, resource_size(iores));
		if (!d->base) {
			pr_err("%s ioremap fail\n", __func__);
			return -EADDRNOTAVAIL;
		}
	}

	ctrl->pinctrl_flag = 0;
	ctrl->pinctrl = NULL;

	*i2c_dev = d;

	return 0;
}

static struct i2c_bus_recovery_info *hs_dw_i2c_new_init_rcov_inf(
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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	gpio_recovery_info->recover_bus = i2c_generic_scl_recovery;
#else
	gpio_recovery_info->recover_bus = i2c_generic_gpio_recovery;
#endif

	gpio_recovery_info->prepare_recovery = dw_i2c_prepare_recovery;
	gpio_recovery_info->unprepare_recovery = dw_i2c_unprepare_recovery;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	gpio_recovery_info->scl_gpiod = gpio_to_desc(gpio_scl);
	gpio_recovery_info->sda_gpiod = gpio_to_desc(gpio_sda);
#else
	gpio_recovery_info->scl_gpio = gpio_scl;
	gpio_recovery_info->sda_gpio = gpio_sda;
#endif

	return gpio_recovery_info;
}

static void hs_dw_i2c_read_ctrl_cfg(struct dw_i2c_controller *ctrl,
	struct device *dev)
{
	int ret;
	u32 data[I2C_RESET_REG_CNT] = {0};

	ret = of_property_read_u32_array(dev->of_node, "reset-reg-base",
		&data[0], ARRAY_SIZE(data));
	if (ret) {
		dev_err(dev, "doesn't have reset-reg-base property!\n");
	} else {
		ctrl->reset_reg_base = devm_ioremap(dev, data[1], data[3]);
		dev_info(dev, "i2c reset register phy_addr is: %x\n", data[1]);
	}

	ret = of_property_read_u32(dev->of_node, "delay-off", &ctrl->delay_off);
	if (ret)
		dev_err(dev, "doesn't have delay-off property!\n");

	ret = of_property_read_u32_array(dev->of_node, "reset-controller-reg",
		&data[0], ARRAY_SIZE(data));
	if (ret)
		dev_err(dev, "doesn't have reset-controller-reg property!\n");

	/* data[0~2] is reset register address, data[3] is reset bit for i2c */
	ctrl->priv.reset_enable_off = data[0];
	ctrl->priv.reset_disable_off = data[1];
	ctrl->priv.reset_status_off = data[2];
	ctrl->priv.reset_bit = data[3];
	ctrl->reset_controller = reset_i2c_controller;
	ctrl->recover_bus = dw_i2c_scl_recover_bus;
}

static int hs_dw_i2c_init_clk_rate(struct device *dev, struct dw_i2c_dev *d)
{
	u64 clk_rate = 0;
	int ret;
	struct clk *i2c_clk = NULL;

	d->get_clk_rate_khz = hs_i2c_dw_get_clk_rate_khz;
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

static hs_dw_i2c_init_adapter(struct i2c_adapter *adap,
	struct platform_device *pdev, struct dw_i2c_dev *d)
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
	strlcpy(adap->name, "Synopsys DesignWare I2C adapter",
		sizeof(adap->name));
	adap->algo = &hs_i2c_dw_algo;
	adap->dev.parent = dev;
	adap->dev.of_node = dev->of_node;
	adap->bus_recovery_info = hs_dw_i2c_new_init_rcov_inf(dev);
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

static void hs_dw_i2c_read_speed_mode(u32 *sp_mode, struct device *dev)
{
	int ret;
	u32 speed_mode = 0;

	ret = of_property_read_u32(dev->of_node, "speed-mode", &speed_mode);
	if (ret) {
		dev_info(dev, "no speed-mode property, use default!\n");
		speed_mode = DW_IC_CON_SPEED_FAST;
	} else {
		if ((speed_mode != DW_IC_CON_SPEED_STD) &&
			(speed_mode != DW_IC_CON_SPEED_FAST) &&
			(speed_mode != DW_IC_CON_SPEED_HIGH))
			speed_mode = DW_IC_CON_SPEED_FAST;

		dev_info(dev, "speed mode is %d!\n", speed_mode);
	}

	*sp_mode = speed_mode;
}

static int hs_dw_i2c_request_irq(
	struct dw_i2c_dev *d, struct platform_device *pdev)
{
	int ret;
	struct dw_i2c_controller *ctrl = NULL;
	struct device *dev = &pdev->dev;

	d->irq = platform_get_irq(pdev, 0);
	if (d->irq < 0) {
		dev_err(dev, "no irq resource\n");
		return -ENXIO;
	}

	ctrl = (struct dw_i2c_controller *)d->priv_data;
	ctrl->irq_is_run = 0;

	ret = devm_request_irq(
		dev, d->irq, i2c_dw_isr, IRQF_TRIGGER_NONE, pdev->name, d);
	if (ret) {
		dev_err(dev, "failure requesting irq %d\n", d->irq);
		return -EINVAL;
	}

	return 0;
}

static int hs_dw_i2c_fill_i2c_dev(struct dw_i2c_dev *d, struct device *dev)
{
	struct dw_i2c_controller *ctrl = NULL;
	u32 speed_mode = 0;
	u32 input_clock_khz, param1;
	int ret;

	input_clock_khz = hs_i2c_dw_get_clk_rate_khz(d);
	i2c_frequency_division(d, input_clock_khz);
	hs_dw_i2c_read_speed_mode(&speed_mode, dev);

	ctrl = (struct dw_i2c_controller *)d->priv_data;
	d->functionality = I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR |
			   I2C_FUNC_SMBUS_BYTE | I2C_FUNC_SMBUS_BYTE_DATA |
			   I2C_FUNC_SMBUS_WORD_DATA | I2C_FUNC_SMBUS_I2C_BLOCK;

	d->master_cfg = DW_IC_CON_MASTER | DW_IC_CON_SLAVE_DISABLE |
			DW_IC_CON_RESTART_EN | speed_mode;

	d->flags = ACCESS_32BIT;
	ret = hs_i2c_dw_reset_controller(d);
	if (ret) {
		dev_err(dev, "i2c:%s reset contoller fail", __func__);
		return ret;
	}

	param1 = dw_readl(d, DW_IC_COMP_PARAM_1);

	d->tx_fifo_depth = ((param1 >> I2C_FIFO_TX_DEPTH_SHIFT) & 0xFF) + 1;
	d->rx_fifo_depth = ((param1 >> I2C_FIFO_RX_DEPTH_SHIFT) & 0xFF) + 1;
	dev_info(dev, "tx_fifo_depth: %u, rx_fifo_depth: %u\n",
		d->tx_fifo_depth, d->rx_fifo_depth);

	if (ctrl->delay_off == I2C_DELAY_OFF_NS)
		d->sda_hold_time =
			(input_clock_khz * I2C_DELAY_OFF_NS) / SPEED_INTERVAL;
	else
		d->sda_hold_time =
			(input_clock_khz * I2C_SDA_HOLD_NS) / SPEED_INTERVAL;

	return 0;
}

static int hs_dw_i2c_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dw_i2c_dev *d = NULL;
	struct dw_i2c_controller *ctrl = NULL;

	if (hs_dw_i2c_new_dev(&d, pdev) || !d) {
		pr_err("%s: new dev fail\n", __func__);
		return -ENODEV;
	}

	ctrl = (struct dw_i2c_controller *)d->priv_data;

	hs_dw_i2c_read_ctrl_cfg(ctrl, dev);

	if (hs_dw_i2c_init_clk_rate(dev, d)) {
		pr_err("%s: init clk fail\n", __func__);
		return -1;
	}

	if (clk_prepare_enable(d->clk)) {
		dev_err(dev, "Unable to enable clock!\n");
		return -1;
	}

	init_completion(&d->cmd_complete);
	init_completion(&ctrl->dma_complete);
	mutex_init(&d->lock);

	if (hs_dw_i2c_fill_i2c_dev(d, dev)) {
		dev_err(dev, "%s: fill i2c dev fail\n", __func__);
		goto clr_err;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	if (i2c_dw_set_timings_master(d)) {
		pr_err("%s: i2c_dw_set_timings_master fail\n", __func__);
		return -1;
	}
#endif

	if (i2c_dw_init_master(d)) {
		dev_err(dev, "%s: init master fail\n", __func__);
		goto clr_err;
	}

	i2c_dw_disable_int(d);

	if (hs_dw_i2c_request_irq(d, pdev)) {
		dev_err(dev, "%s: request irq fail\n", __func__);
		goto clr_err;
	}

	if (hs_dw_i2c_init_adapter(&d->adapter, pdev, d)) {
		dev_err(dev, "%s: init adapter fail\n", __func__);
		goto clr_err;
	}

	platform_set_drvdata(pdev, d);

	/* DMA probe */
	i2c_dw_dma_probe(d);

	dev_info(dev, "i2c probe succeed!\n");
	return 0;

clr_err:
	clk_disable_unprepare(d->clk);
	d->clk = NULL;
	put_device(dev);

	return -1;
}

static int hs_dw_i2c_remove(struct platform_device *pdev)
{
	struct dw_i2c_dev *d = platform_get_drvdata(pdev);

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

	i2c_dw_dma_remove(d);

	i2c_dw_disable(d);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id hs_dw_i2c_of_match[] = {
	{
		.compatible = "hisilicon,designware-i2c",
	},
	{},
};
MODULE_DEVICE_TABLE(of, hs_dw_i2c_of_match);
#endif

#ifdef CONFIG_PM
static int hs_dw_i2c_suspend(struct device *dev)
{
	unsigned long time, timeout;
	struct platform_device *pdev = to_platform_device(dev);
	struct dw_i2c_dev *i_dev = platform_get_drvdata(pdev);

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

	clk_disable(i_dev->clk);
	dev_info(&pdev->dev, "%s: suspend -\n", __func__);
	return 0;
}

static int hs_dw_i2c_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct dw_i2c_dev *i_dev = platform_get_drvdata(pdev);
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
	ret = hs_i2c_dw_reset_controller(i_dev);
	if (ret)
		dev_err(&pdev->dev, "reset controller fail!\n");

	i2c_dw_init_master(i_dev);
	i2c_dw_disable_int(i_dev);

	mutex_unlock(&i_dev->lock);

	dev_info(&pdev->dev, "%s: resume -\n", __func__);
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(
	hs_dw_i2c_dev_pm_ops, hs_dw_i2c_suspend, hs_dw_i2c_resume);

static struct platform_driver hs_dw_i2c_driver = {
	.probe = hs_dw_i2c_probe,
	.remove = hs_dw_i2c_remove,
	.driver = {
		.name = "i2c_designware-hs",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hs_dw_i2c_of_match),
		.pm = &hs_dw_i2c_dev_pm_ops,
	},
};

static int __init i2c_init(void)
{
	return platform_driver_register(&hs_dw_i2c_driver);
}

static void __exit i2c_exit(void)
{
	platform_driver_unregister(&hs_dw_i2c_driver);
}

subsys_initcall(i2c_init);
module_exit(i2c_exit);

MODULE_DESCRIPTION("HS Synopsys DesignWare I2C bus adapter");
MODULE_ALIAS("platform:i2c_designware-hs");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
