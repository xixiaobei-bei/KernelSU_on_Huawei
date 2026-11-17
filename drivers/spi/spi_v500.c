/*
 * A driver for the SPI bus master.
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/amba/bus.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/pm_runtime.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <securec.h>

#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/hwspinlock.h>
#include "../hwspinlock/hwspinlock_internal.h"
#include "spi_v500.h"

extern void show_dma64_reg(struct dma_chan *chan);

/**
 * null_cs_control - Dummy chip select function
 * @command: select/delect the chip
 *
 * If no chip select function is provided by client this is used as dummy
 * chip select
 */
static void null_cs_control(u32 command)
{
	pr_debug("hs_spi: dummy chip select control, CS=0x%x\n", command);
}

static void hs_spi_cs_control(struct hs_spi_controller
			*spi_control, u32 command)
{
	if (gpio_is_valid(spi_control->cur_cs))
		gpio_set_value(spi_control->cur_cs, command);
	else
		spi_control->cur_chip->cs_control(command);
	if (command == SSP_CHIP_SELECT)
		spi_control->transfer_state = STATE_CHIP_SELECT;
	else
		spi_control->transfer_state = STATE_CHIP_DESELECT;
}

/**
 * giveback - current spi_message is over, schedule next message and call
 * callback of this message. Assumes that caller already
 * set message->status; dma and pio irqs are blocked
 * @hs_spi: SSP driver private data structure
 */
static void giveback(struct hs_spi_controller *spi_control)
{
	struct spi_transfer *last_transfer;

	spi_control->next_msg_cs_active = false;
	last_transfer = list_last_entry(&spi_control->cur_msg->transfers,
					struct spi_transfer, transfer_list);

	/* Delay if requested before any change in chip select */
	if (last_transfer->delay_usecs)
		/*
		 * FIXME: This runs in interrupt context.
		 * Is this really smart?
		 */
		udelay(last_transfer->delay_usecs);

	if (!last_transfer->cs_change) {
		struct spi_message *next_msg;

		/*
		 * cs_change was not set. We can keep the chip select
		 * enabled if there is message in the queue and it is
		 * for the same spi device.
		 *
		 * We cannot postpone this until pump_messages, because
		 * after calling msg->complete (below) the driver that
		 * sent the current message could be unloaded, which
		 * could invalidate the cs_control() callback...
		 */
		/* get a pointer to the next message, if any */
		next_msg = spi_get_next_queued_message(spi_control->master);
		/*
		 * see if the next and current messages point
		 * to the same spi device.
		 */
		if (next_msg && next_msg->spi != spi_control->cur_msg->spi)
			next_msg = NULL;
		if (!next_msg || spi_control->cur_msg->state == STATE_ERROR)
			hs_spi_cs_control(spi_control, SSP_CHIP_DESELECT);
		else
			spi_control->next_msg_cs_active = true;
	}

	spi_control->cur_msg = NULL;
	spi_control->cur_transfer = NULL;
	spi_control->cur_chip = NULL;

	/* disable the SPI/SSP operation */
	writel((readl(SSP_ENR(spi_control->virtbase)) &
		(~SSP_ENR_MASK_EN)), SSP_ENR(spi_control->virtbase));

	spi_finalize_current_message(spi_control->master);
}

/**
 * flush - flush the FIFO to reach a clean state
 * @hs_spi: SSP driver private data structure
 */
static int flush(struct hs_spi_controller *spi_control)
{
	unsigned long limit = loops_per_jiffy << 1;

	dev_dbg(&spi_control->pdev->dev, "%s\n", __func__);
	do {
		while (readl(SSP_SR(spi_control->virtbase)) & SSP_SR_MASK_RNE)
			readl(SSP_DOUT(spi_control->virtbase));
	} while ((readl(SSP_SR(spi_control->virtbase)) & SSP_SR_MASK_BSY)
			&& limit--);

	spi_control->exp_fifo_level = 0;

	return limit;
}

/**
 * restore_state - Load configuration of current chip
 * @hs_spi: SSP driver private data structure
 */
static void restore_state(struct hs_spi_controller *spi_control)
{
	struct chip_data *chip = spi_control->cur_chip;

	writel(chip->csr, SSP_CSCR(spi_control->virtbase));
	writel(chip->cr, SSP_COMMONCR(spi_control->virtbase));
	writel(chip->enr, SSP_ENR(spi_control->virtbase));
	writel(DEFAULT_SSP_REG_FIFOCR, SSP_FIFOCR(spi_control->virtbase));
	writel(DISABLE_ALL_INTERRUPTS, SSP_IMC(spi_control->virtbase));
	writel(CLEAR_ALL_INTERRUPTS, SSP_ICR(spi_control->virtbase));
}

/*
 * load_ssp_default_config - Load default configuration for SSP
 * @spi_control: SSP driver private data structure
 */
static void load_ssp_default_config(struct hs_spi_controller *spi_control)
{
	writel(DEFAULT_SSP_REG_CSCR, SSP_CSCR(spi_control->virtbase));
	writel(DEFAULT_SSP_REG_COMMONCR, SSP_COMMONCR(spi_control->virtbase));
	writel(DEFAULT_SSP_REG_ENR, SSP_ENR(spi_control->virtbase));
	writel(DISABLE_ALL_INTERRUPTS, SSP_IMC(spi_control->virtbase));
	writel(CLEAR_ALL_INTERRUPTS, SSP_ICR(spi_control->virtbase));
}

static void hs_spi_read_rx(struct hs_spi_controller *spi_control)
{
	while ((readl(SSP_SR(spi_control->virtbase)) & SSP_SR_MASK_RNE)
	       && (spi_control->rx < spi_control->rx_end)) {
		switch (spi_control->read) {
		case READING_NULL:
			readl(SSP_DOUT(spi_control->virtbase));
			break;
		case READING_U8:
			*(u8 *) (spi_control->rx) =
				readl(SSP_DOUT(spi_control->virtbase)) & 0xFFU;
			break;
		case READING_U16:
			*(u16 *) (spi_control->rx) =
				(u16) readl(SSP_DOUT(spi_control->virtbase));
			break;
		case READING_U32:
			*(u32 *) (spi_control->rx) =
				readl(SSP_DOUT(spi_control->virtbase));
			break;
		}
		spi_control->rx += (spi_control->cur_chip->n_bytes);
		spi_control->exp_fifo_level--;
	}
}

static void hs_spi_write_tx(struct hs_spi_controller *spi_control)
{
	/*
	 * Write as much as possible up to the RX FIFO size
	 */
	while ((spi_control->exp_fifo_level < FIFO_DEPTH)
	       && (spi_control->tx < spi_control->tx_end)) {
		switch (spi_control->write) {
		case WRITING_NULL:
			writel(0x0, SSP_DIN(spi_control->virtbase));
			break;
		case WRITING_U8:
			writel(*(u8 *) (spi_control->tx),
				SSP_DIN(spi_control->virtbase));
			break;
		case WRITING_U16:
			writel((*(u16 *) (spi_control->tx)),
				SSP_DIN(spi_control->virtbase));
			break;
		case WRITING_U32:
			writel(*(u32 *) (spi_control->tx),
				SSP_DIN(spi_control->virtbase));
			break;
		}
		spi_control->tx += (spi_control->cur_chip->n_bytes);
		spi_control->exp_fifo_level++;
		/*
		 * This inner reader takes care of things appearing in the RX
		 * FIFO as we're transmitting. This will happen a lot since the
		 * clock starts running when you put things into the TX FIFO,
		 * and then things are continuously clocked into the RX FIFO.
		 */
		 hs_spi_read_rx(spi_control);
			/*
			 * When we exit here the TX FIFO should be full and the
			 * RX FIFO should be empty
			 */
	}
}
/*
 * This will write to TX and read from RX according to the parameters
 * set in spi_control.
 */
static void readwriter(struct hs_spi_controller *spi_control)
{
	dev_dbg(&spi_control->pdev->dev,
		"%s, rx: %pK, rxend: %pK, tx: %pK, txend: %pK\n",
		__func__, spi_control->rx, spi_control->rx_end,
		spi_control->tx, spi_control->tx_end);

	/* Read as much as you can */
	hs_spi_read_rx(spi_control);
	/* Write as much as possible up to the RX FIFO size */
	hs_spi_write_tx(spi_control);
}

/*
 * next_transfer - Move to the Next transfer in the current spi message
 * @spi_control: SSP driver private data structure
 *
 * This function moves though the linked list of spi transfers in the
 * current spi message and returns with the state of current spi
 * message i.e whether its last transfer is done(STATE_DONE) or
 * Next transfer is ready(STATE_RUNNING)
 */
static void *next_transfer(struct hs_spi_controller *spi_control)
{
	struct spi_message *msg = spi_control->cur_msg;
	struct spi_transfer *trans = spi_control->cur_transfer;

	/* Move to next transfer */
	if (trans->transfer_list.next != &msg->transfers) {
		spi_control->cur_transfer =
		    list_entry(trans->transfer_list.next,
			       struct spi_transfer, transfer_list);
		return STATE_RUNNING;
	}
	return STATE_DONE;
}

static void spi_v500_dma_buffer_free(struct hs_spi_controller *spi_control)
{
	errno_t ret;

	if (spi_control->rx_buffer) {
		ret = memcpy_s(spi_control->cur_transfer->rx_buf,
			spi_control->cur_transfer->len,
			spi_control->rx,
			spi_control->cur_transfer->len);
		if (ret != EOK) {
			dev_err(&spi_control->pdev->dev, "memcpy_s failed\n");
			return;
		}

		kfree(spi_control->rx_buffer);
		spi_control->rx_buffer = NULL;
	}

	kfree(spi_control->tx_buffer);
	spi_control->tx_buffer = NULL;
}

static void spi_v500_txrx_buffer_check(
	struct hs_spi_controller *spi_control,
	struct spi_transfer *transfer)
{
	errno_t ret;

	if (virt_to_phys(spi_control->cur_transfer->tx_buf) >
		SPI_4G_PHYS_ADDR) {
		/*
		 * wrining! must be use dma buffer, and need the flag "GFP_DMA"
		 * when alloc memery
		 */
		WARN_ON(1);
		spi_control->tx_buffer =
			kzalloc(spi_control->cur_transfer->len,
				GFP_KERNEL | GFP_DMA);
		if (spi_control->tx_buffer) {
			ret = memcpy_s(spi_control->tx_buffer,
				spi_control->cur_transfer->len,
				transfer->tx_buf,
				spi_control->cur_transfer->len);
			if (ret != EOK) {
				dev_err(&spi_control->pdev->dev,
					"%s:memcpy_s failed\n", __func__);
				return;
			}

			spi_control->tx = (void *)spi_control->tx_buffer;
			dev_err(&spi_control->pdev->dev, "tx is not dma-buffer\n");
		} else {
			spi_control->tx = (void *)transfer->tx_buf;
			dev_err(&spi_control->pdev->dev,
				"can not alloc dma-buffer for tx\n");
		}
	} else {
		spi_control->tx = (void *)transfer->tx_buf;
	}
	spi_control->tx_end = spi_control->tx + spi_control->cur_transfer->len;

	if (virt_to_phys(spi_control->cur_transfer->rx_buf) >
		SPI_4G_PHYS_ADDR) {
		/*
		 * wrining! must be use dma buffer, and need the flag "GFP_DMA"
		 * when alloc memery
		 */
		WARN_ON(1);
		spi_control->rx_buffer =
			kzalloc(spi_control->cur_transfer->len,
				GFP_KERNEL | GFP_DMA);
		if (spi_control->rx_buffer) {
			spi_control->rx = (void *)spi_control->rx_buffer;
			dev_err(&spi_control->pdev->dev, "rx is not dma-buffer\n");
		} else {
			spi_control->rx = (void *)transfer->rx_buf;
			dev_err(&spi_control->pdev->dev,
				"can not alloc dma-buffer for rx\n");
		}
	} else {
		spi_control->rx = (void *)transfer->rx_buf;
	}
	spi_control->rx_end = spi_control->rx + spi_control->cur_transfer->len;
}

/*
 * This DMA functionality is only compiled in if we have
 * access to the generic DMA devices/DMA engine.
 */
#ifdef CONFIG_DMA_ENGINE
static void unmap_free_dma_scatter(struct hs_spi_controller *spi_control)
{
	/* Unmap and free the SG tables */
	dma_unmap_sg(spi_control->dma_tx_channel->device->dev,
		spi_control->sgt_tx.sgl,
		spi_control->sgt_tx.nents, DMA_TO_DEVICE);
	dma_unmap_sg(spi_control->dma_rx_channel->device->dev,
		spi_control->sgt_rx.sgl,
		spi_control->sgt_rx.nents, DMA_FROM_DEVICE);
	sg_free_table(&spi_control->sgt_rx);
	sg_free_table(&spi_control->sgt_tx);
}

static void dma_callback(void *data)
{
	struct hs_spi_controller *spi_control = data;
	struct spi_message *msg = spi_control->cur_msg;

	spin_lock(&spi_control->lock);
	if (!msg || !msg->context) {
		dev_err(&spi_control->pdev->dev, "msg is invalid\n");
		spin_unlock(&spi_control->lock);
		return;
	}
	if (!spi_control->sgt_rx.sgl) {
		dev_err(&spi_control->pdev->dev, "sgt_rx.sgl invalid\n");
		spin_unlock(&spi_control->lock);
		return;
	}
	if (spi_control->transfer_state != STATE_CFG_DMA_DONE) {
		dev_err(&spi_control->pdev->dev, "msg state err\n");
		spin_unlock(&spi_control->lock);
		return;
	}

	spi_v500_dma_buffer_free(spi_control);
	spi_control->transfer_state = STATE_DMA_CALLBACK;
	unmap_free_dma_scatter(spi_control);

	/* Update total bytes transferred */
	msg->actual_length += spi_control->cur_transfer->len;
	if (spi_control->cur_transfer->cs_change)
		hs_spi_cs_control(spi_control, SSP_CHIP_DESELECT);
	/* Move to next transfer */
	msg->state = next_transfer(spi_control);
	if (msg->state != STATE_DONE && spi_control->cur_transfer->cs_change)
		hs_spi_cs_control(spi_control, SSP_CHIP_DESELECT);

	tasklet_schedule(&spi_control->pump_transfers);
	spin_unlock(&spi_control->lock);
}

static void setup_dma_scatter(struct hs_spi_controller *spi_control,
	void *buffer, unsigned int length, struct sg_table *sgtab)
{
	struct scatterlist *sg = NULL;
	unsigned int bytesleft = length;
	void *bufp = buffer;
	int mapbytes;
	u32 i = 0;

	if (buffer) {
		for_each_sg(sgtab->sgl, sg, sgtab->nents, i) {
			/*
			 * If there are less bytes left than what fits
			 * in the current page (plus page alignment offset)
			 * we just feed in this, else we stuff in as much
			 * as we can.
			 */
			if (bytesleft < (PAGE_SIZE - offset_in_page(bufp)))
				mapbytes = bytesleft;
			else
				mapbytes = PAGE_SIZE - offset_in_page(bufp);
			sg_set_page(sg, virt_to_page(bufp),
				    mapbytes, offset_in_page(bufp));
			bufp += mapbytes;
			bytesleft -= mapbytes;
			dev_err(&spi_control->pdev->dev,
				"set RX/TX target page @ %pK, %d bytes, %d left\n",
				bufp, mapbytes, bytesleft);
		}
	} else {
		/* Map the dummy buffer on every page */
		for_each_sg(sgtab->sgl, sg, sgtab->nents, i) {
			if (bytesleft < PAGE_SIZE)
				mapbytes = bytesleft;
			else
				mapbytes = PAGE_SIZE;
			sg_set_page(sg, virt_to_page(spi_control->dummypage),
				    mapbytes, 0);
			bytesleft -= mapbytes;
			dev_err(&spi_control->pdev->dev,
				"set RX/TX to dummy page %d bytes, %d left\n",
				mapbytes, bytesleft);
		}
	}
	BUG_ON(bytesleft);
}

static void config_dma_fifo_level(struct hs_spi_controller *spi_control,
	struct dma_slave_config *rx_conf, struct dma_slave_config *tx_conf)
{
	switch (spi_control->rx_lev_trig) {
	case SSP_RX_1_OR_MORE_ELEM:
		rx_conf->src_maxburst = MAX_BURST_1;
		break;
	case SSP_RX_4_OR_MORE_ELEM:
		rx_conf->src_maxburst = MAX_BURST_4;
		break;
	case SSP_RX_8_OR_MORE_ELEM:
		rx_conf->src_maxburst = MAX_BURST_8;
		break;
	case SSP_RX_16_OR_MORE_ELEM:
		rx_conf->src_maxburst = MAX_BURST_16;
		break;
	case SSP_RX_32_OR_MORE_ELEM:
		rx_conf->src_maxburst = MAX_BURST_32;
		break;
	default:
		rx_conf->src_maxburst = FIFO_DEPTH >> 1;
		break;
	}

	switch (spi_control->tx_lev_trig) {
	case SSP_TX_1_OR_MORE_EMPTY_LOC:
		tx_conf->dst_maxburst = MAX_BURST_1;
		break;
	case SSP_TX_4_OR_MORE_EMPTY_LOC:
		tx_conf->dst_maxburst = MAX_BURST_4;
		break;
	case SSP_TX_8_OR_MORE_EMPTY_LOC:
		tx_conf->dst_maxburst = MAX_BURST_8;
		break;
	case SSP_TX_16_OR_MORE_EMPTY_LOC:
		tx_conf->dst_maxburst = MAX_BURST_16;
		break;
	case SSP_TX_32_OR_MORE_EMPTY_LOC:
		tx_conf->dst_maxburst = MAX_BURST_32;
		break;
	default:
		tx_conf->dst_maxburst = FIFO_DEPTH >> 1;
		break;
	}
}

static void config_dma_bus_width(struct hs_spi_controller *spi_control,
	struct dma_slave_config *rx_conf, struct dma_slave_config *tx_conf)
{
	switch (spi_control->read) {
	case READING_NULL:
		/* Use the same as for writing */
		rx_conf->src_addr_width = DMA_SLAVE_BUSWIDTH_UNDEFINED;
		break;
	case READING_U8:
		rx_conf->src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		break;
	case READING_U16:
		rx_conf->src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
		break;
	case READING_U32:
		rx_conf->src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		break;
	}

	switch (spi_control->write) {
	case WRITING_NULL:
		/* Use the same as for reading */
		tx_conf->dst_addr_width = DMA_SLAVE_BUSWIDTH_UNDEFINED;
		break;
	case WRITING_U8:
		tx_conf->dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		break;
	case WRITING_U16:
		tx_conf->dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
		break;
	case WRITING_U32:
		tx_conf->dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		break;
	}

	/* SPI pecularity: we need to read and write the same width */
	if (rx_conf->src_addr_width == DMA_SLAVE_BUSWIDTH_UNDEFINED)
		rx_conf->src_addr_width = tx_conf->dst_addr_width;
	if (tx_conf->dst_addr_width == DMA_SLAVE_BUSWIDTH_UNDEFINED)
		tx_conf->dst_addr_width = rx_conf->src_addr_width;
	BUG_ON(rx_conf->src_addr_width != tx_conf->dst_addr_width);
}

static int spi_dma_sgl_config(struct hs_spi_controller *spi_control,
	int *rx_sglen, int *tx_sglen)
{
	int ret;
	unsigned int pages, txpages, rxpages;
	struct dma_chan *rxchan = spi_control->dma_rx_channel;
	struct dma_chan *txchan = spi_control->dma_tx_channel;

	/* Create sglists for the transfers */
	pages = DIV_ROUND_UP(spi_control->cur_transfer->len, PAGE_SIZE);
	dev_dbg(&spi_control->pdev->dev,
		"using %d pages for transfer\n", pages);

	rxpages = pages;
	txpages = pages;
	spi_control->transfer_len = spi_control->cur_transfer->len;
	if (spi_control->cur_transfer->len >
		(pages * PAGE_SIZE -
			offset_in_page(
			(unsigned long)(uintptr_t)spi_control->rx)))
		rxpages++;

	if (spi_control->cur_transfer->len >
		(pages * PAGE_SIZE -
			offset_in_page(
			(unsigned long)(uintptr_t)spi_control->tx)))
		txpages++;

	ret = sg_alloc_table(&spi_control->sgt_rx, rxpages, GFP_ATOMIC);
	if (ret)
		goto err_alloc_rx_sg;

	ret = sg_alloc_table(&spi_control->sgt_tx, txpages, GFP_ATOMIC);
	if (ret)
		goto err_alloc_tx_sg;

	/* Fill in the scatterlists for the RX+TX buffers */
	setup_dma_scatter(spi_control, spi_control->rx,
		spi_control->cur_transfer->len, &spi_control->sgt_rx);
	setup_dma_scatter(spi_control, spi_control->tx,
		spi_control->cur_transfer->len, &spi_control->sgt_tx);

	/* Map DMA buffers */
	*rx_sglen = dma_map_sg(rxchan->device->dev, spi_control->sgt_rx.sgl,
		spi_control->sgt_rx.nents, DMA_FROM_DEVICE);
	if (!(*rx_sglen))
		goto err_rx_sgmap;

	*tx_sglen = dma_map_sg(txchan->device->dev, spi_control->sgt_tx.sgl,
		spi_control->sgt_tx.nents, DMA_TO_DEVICE);
	if (!(*tx_sglen))
		goto err_tx_sgmap;

	return ret;

err_tx_sgmap:
	dma_unmap_sg(rxchan->device->dev, spi_control->sgt_rx.sgl,
		spi_control->sgt_rx.nents, DMA_FROM_DEVICE);
err_rx_sgmap:
	sg_free_table(&spi_control->sgt_tx);
err_alloc_tx_sg:
	sg_free_table(&spi_control->sgt_rx);
err_alloc_rx_sg:
	return -ENOMEM;
}

/*
 * configure_dma - configures the channels for the next transfer
 * @hs_spi: SSP driver's private data structure
 */
static int configure_dma(struct hs_spi_controller *spi_control)
{
	struct dma_slave_config rx_conf = {
		.src_addr = SSP_DOUT(spi_control->phybase),
		.direction = DMA_DEV_TO_MEM,
		.device_fc = false,
	};
	struct dma_slave_config tx_conf = {
		.dst_addr = SSP_DIN(spi_control->phybase),
		.direction = DMA_MEM_TO_DEV,
		.device_fc = false,
	};

	int ret;
	int rx_sglen, tx_sglen;
	struct dma_chan *rxchan = spi_control->dma_rx_channel;
	struct dma_chan *txchan = spi_control->dma_tx_channel;
	struct dma_async_tx_descriptor *rxdesc = NULL;
	struct dma_async_tx_descriptor *txdesc = NULL;

	/* Check that the channels are available */
	if (!rxchan || !txchan)
		return -ENODEV;

	config_dma_fifo_level(spi_control, &rx_conf, &tx_conf);
	config_dma_bus_width(spi_control, &rx_conf, &tx_conf);

	dmaengine_slave_config(rxchan, &rx_conf);
	dmaengine_slave_config(txchan, &tx_conf);

	ret = spi_dma_sgl_config(spi_control, &rx_sglen, &tx_sglen);
	if (ret)
		return -ENOMEM;

	/* Send both scatterlists */
	rxdesc = dmaengine_prep_slave_sg(rxchan,
		spi_control->sgt_rx.sgl,
		rx_sglen,
		DMA_DEV_TO_MEM,
		DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!rxdesc)
		goto err_rxdesc;

	txdesc = dmaengine_prep_slave_sg(txchan,
		spi_control->sgt_tx.sgl,
		tx_sglen,
		DMA_MEM_TO_DEV,
		DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!txdesc)
		goto err_txdesc;

	/* Put the callback on the RX transfer only, that should finish last */
	rxdesc->callback = dma_callback;
	rxdesc->callback_param = spi_control;
	/* Submit and fire RX and TX with TX last so we're ready to read! */
	dmaengine_submit(rxdesc);
	dmaengine_submit(txdesc);
	dma_async_issue_pending(rxchan);
	dma_async_issue_pending(txchan);
	spi_control->dma_running = true;
	spi_control->transfer_state = STATE_CFG_DMA_DONE;

	return 0;

err_txdesc:
	dmaengine_terminate_all(txchan);
err_rxdesc:
	dmaengine_terminate_all(rxchan);
	dma_unmap_sg(txchan->device->dev, spi_control->sgt_tx.sgl,
			spi_control->sgt_tx.nents, DMA_TO_DEVICE);
	return -ENOMEM;
}

static int hs_spi_dma_probe(struct hs_spi_controller *spi_control)
{
	dma_cap_mask_t mask;

	/* Try to acquire a generic DMA engine slave channel */
	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);
	/*
	 * We need both RX and TX channels to do DMA, else do none
	 * of them.
	 */
	spi_control->dma_rx_channel = dma_request_channel(mask,
		spi_control->pdata->dma_filter,
		spi_control->pdata->dma_rx_param);
	if (!spi_control->dma_rx_channel) {
		dev_dbg(&spi_control->pdev->dev, "no RX DMA channel!\n");
		goto err_no_rxchan;
	}

	spi_control->dma_tx_channel = dma_request_channel(mask,
						spi_control->pdata->dma_filter,
						spi_control->pdata->dma_tx_param);
	if (!spi_control->dma_tx_channel) {
		dev_dbg(&spi_control->pdev->dev, "no TX DMA channel!\n");
		goto err_no_txchan;
	}

	spi_control->dummypage = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!spi_control->dummypage)
		goto err_no_dummypage;

	dev_info(&spi_control->pdev->dev, "setup for DMA on RX %s, TX %s\n",
		 dma_chan_name(spi_control->dma_rx_channel),
		 dma_chan_name(spi_control->dma_tx_channel));

	return 0;

err_no_dummypage:
	dma_release_channel(spi_control->dma_tx_channel);
err_no_txchan:
	dma_release_channel(spi_control->dma_rx_channel);
	spi_control->dma_rx_channel = NULL;
err_no_rxchan:
	dev_err(&spi_control->pdev->dev,
		"Failed to work in dma mode, work without dma!\n");
	return -ENODEV;
}

static int hs_spi_dma_autoprobe(struct hs_spi_controller *spi_control)
{
	struct device *dev = &spi_control->pdev->dev;
	struct dma_chan *chan;
	int err;

	/* automatically configure DMA channels from platform,
	 * normally using DT
	 */
	chan = dma_request_slave_channel_reason(dev, "rx");
	if (IS_ERR(chan)) {
		err = PTR_ERR(chan);
		goto err_no_rxchan;
	}

	dev_err(dev, "spi DMA channel RX %s\n",
		dma_chan_name(chan));

	spi_control->dma_rx_channel = chan;

	chan = dma_request_slave_channel_reason(dev, "tx");
	if (IS_ERR(chan)) {
		err = PTR_ERR(chan);
		goto err_no_txchan;
	}
	dev_err(dev, "spi DMA channel TX %s\n",
		dma_chan_name(chan));

	spi_control->dma_tx_channel = chan;

	spi_control->dummypage = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!spi_control->dummypage) {
		err = -ENOMEM;
		goto err_no_dummypage;
	}

	return 0;

err_no_dummypage:
	dma_release_channel(spi_control->dma_tx_channel);
	spi_control->dma_tx_channel = NULL;
err_no_txchan:
	dma_release_channel(spi_control->dma_rx_channel);
	spi_control->dma_rx_channel = NULL;
err_no_rxchan:
	return err;
}

static void terminate_dma(struct hs_spi_controller *spi_control)
{
	struct dma_chan *rxchan = spi_control->dma_rx_channel;
	struct dma_chan *txchan = spi_control->dma_tx_channel;

	dmaengine_terminate_all(rxchan);
	dmaengine_terminate_all(txchan);
	unmap_free_dma_scatter(spi_control);
	spi_control->dma_running = false;
}

static void hs_spi_dma_remove(struct hs_spi_controller *spi_control)
{
	if (spi_control->dma_running)
		terminate_dma(spi_control);
	if (spi_control->dma_tx_channel)
		dma_release_channel(spi_control->dma_tx_channel);
	if (spi_control->dma_rx_channel)
		dma_release_channel(spi_control->dma_rx_channel);
	kfree(spi_control->dummypage);
}

#else
static inline int configure_dma(struct hs_spi_controller *spi_control)
{
	return -ENODEV;
}

static inline int hs_spi_dma_autoprobe(struct hs_spi_controller *spi_control)
{
	return 0;
}

static inline int hs_spi_dma_probe(struct hs_spi_controller *spi_control)
{
	return 0;
}

static inline void hs_spi_dma_remove(struct hs_spi_controller *spi_control)
{
}
#endif

static void hs_spi_fifo_overrun(struct hs_spi_controller *spi_control)
{
	struct spi_message *msg = spi_control->cur_msg;

	if (readl(SSP_SR(spi_control->virtbase)) & SSP_SR_MASK_RFF)
		dev_err(&spi_control->pdev->dev,
			"RXFIFO is full\n");
	if (spi_control->cur_chip->enable_dma) {
		dmaengine_terminate_all(spi_control->dma_rx_channel);
		dmaengine_terminate_all(spi_control->dma_tx_channel);
	}
	spi_control->transfer_state = STATE_FIFO_OVERRUN;

	/*
	 * Disable and clear interrupts, disable SSP,
	 * mark message with bad status so it can be
	 * retried.
	 */
	writel(DISABLE_ALL_INTERRUPTS,
		SSP_IMC(spi_control->virtbase));
	writel(CLEAR_ALL_INTERRUPTS, SSP_ICR(spi_control->virtbase));
	writel((readl(SSP_ENR(spi_control->virtbase)) &
		(~SSP_ENR_MASK_EN)), SSP_ENR(spi_control->virtbase));
	msg->state = STATE_ERROR;

	/* Schedule message queue handler */
	tasklet_schedule(&spi_control->pump_transfers);
}

/*
 * hs_spi_interrupt_handler - Interrupt handler for SPI controller
 *
 * This function handles interrupts generated for an interrupt based transfer.
 * If a receive overrun (ROR) interrupt is there then we disable SSP, flag the
 * current message's state as STATE_ERROR and schedule the tasklet
 * pump_transfers which will do the postprocessing of the current message by
 * calling giveback(). Otherwise it reads data from RX FIFO till there is no
 * more data, and writes data in TX FIFO till it is not full. If we complete
 * the transfer we move to the next transfer and schedule the tasklet.
 */
static irqreturn_t hs_spi_interrupt_handler(int irq, void *dev_id)
{
	struct hs_spi_controller *spi_control = dev_id;
	struct spi_message *msg = spi_control->cur_msg;
	u16 irq_status = 0;

	if (unlikely(!msg)) {
		dev_err(&spi_control->pdev->dev,
			"bad message state in interrupt handler");
		/* Never fail */
		return IRQ_HANDLED;
	}

	/* Read the Interrupt Status Register */
	irq_status = readl(SSP_MIS(spi_control->virtbase));
	if (unlikely(!irq_status))
		return IRQ_NONE;

	/*
	 * This handles the FIFO interrupts, the timeout
	 * interrupts are flatly ignored, they cannot be
	 * trusted.
	 */
	if (unlikely(irq_status & SSP_MIS_MASK_RORMIS)) {
		/*
		 * Overrun interrupt - bail out since our Data has been
		 * corrupted
		 */
		dev_err(&spi_control->pdev->dev, "FIFO overrun\n");
		hs_spi_fifo_overrun(spi_control);
		return IRQ_HANDLED;
	}

	readwriter(spi_control);

	if (spi_control->tx == spi_control->tx_end)
		/* Disable Transmit interrupt, enable receive interrupt */
		writel((readl(SSP_IMC(spi_control->virtbase)) &
		       ~SSP_IMC_MASK_TXIM) | SSP_IMC_MASK_RXIM,
		       SSP_IMC(spi_control->virtbase));

	/*
	 * Since all transactions must write as much as shall be read,
	 * we can conclude the entire transaction once RX is complete.
	 * At this point, all TX will always be finished.
	 */
	if (spi_control->rx >= spi_control->rx_end) {
		writel(DISABLE_ALL_INTERRUPTS,
		       SSP_IMC(spi_control->virtbase));
		writel(CLEAR_ALL_INTERRUPTS, SSP_ICR(spi_control->virtbase));
		if (unlikely(spi_control->rx > spi_control->rx_end))
			dev_warn(&spi_control->pdev->dev, "read %u surplus "
				 "bytes (did you request an odd "
				 "number of bytes on a 16bit bus?)\n",
				 (u32) (spi_control->rx - spi_control->rx_end));

		/* Update total bytes transferred */
		msg->actual_length += spi_control->cur_transfer->len;
		if (spi_control->cur_transfer->cs_change)
			hs_spi_cs_control(spi_control, SSP_CHIP_DESELECT);
		/* Move to next transfer */
		msg->state = next_transfer(spi_control);
		tasklet_schedule(&spi_control->pump_transfers);
		return IRQ_HANDLED;
	}

	return IRQ_HANDLED;
}

/**
 * This sets up the pointers to memory for the next message to
 * send out on the SPI bus.
 */
static int set_up_next_transfer(struct hs_spi_controller *spi_control,
				struct spi_transfer *transfer)
{
	int residue;

	/* Sanity check the message for this bus width */
	residue = spi_control->cur_transfer->len %
		spi_control->cur_chip->n_bytes;
	if (unlikely(residue != 0)) {
		dev_err(&spi_control->pdev->dev,
			"message of %u bytes to transmit but the current "
			"chip bus has a data width of %u bytes!\n",
			spi_control->cur_transfer->len,
			spi_control->cur_chip->n_bytes);
		dev_err(&spi_control->pdev->dev, "skipping this message\n");
		return -EIO;
	}

	if (spi_control->cur_chip->enable_dma && (spi_control->dmacheck_addr)) {
		spi_v500_txrx_buffer_check(spi_control, transfer);
	} else {
		spi_control->tx = (void *)transfer->tx_buf;
		spi_control->tx_end = spi_control->tx +
			spi_control->cur_transfer->len;
		spi_control->rx = (void *)transfer->rx_buf;
		spi_control->rx_end = spi_control->rx +
			spi_control->cur_transfer->len;
	}

	spi_control->write =
		spi_control->tx ? spi_control->cur_chip->write : WRITING_NULL;
	spi_control->read = spi_control->rx ?
		spi_control->cur_chip->read : READING_NULL;
	return 0;
}

/**
 * pump_transfers - Tasklet function which schedules next transfer
 * when running in interrupt or DMA transfer mode.
 * @data: SPI driver private data structure
 *
 */
static void pump_transfers(unsigned long data)
{
	struct hs_spi_controller *spi_control =
		(struct hs_spi_controller *) (uintptr_t)data;
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;
	struct spi_transfer *previous = NULL;

	/* Get current state information */
	message = spi_control->cur_msg;
	transfer = spi_control->cur_transfer;

	if (!message || !transfer) {
		dev_err(&spi_control->pdev->dev, "msg or transfer invalid\n");
		return;
	}

	/* Handle for abort */
	if (message->state == STATE_ERROR) {
		message->status = -EIO;
		giveback(spi_control);
		return;
	}

	/* Handle end of message */
	if (message->state == STATE_DONE) {
		message->status = 0;
		flush(spi_control);
		giveback(spi_control);
		return;
	}

	/* Delay if requested at end of transfer before CS change */
	if (message->state == STATE_RUNNING) {
		previous = list_entry(transfer->transfer_list.prev,
					struct spi_transfer,
					transfer_list);
		if (previous->delay_usecs)
			/*
			 * FIXME: This runs in interrupt context.
			 * Is this really smart?
			 */
			udelay(previous->delay_usecs);

		/* Reselect chip select only if cs_change was requested */
		if (previous->cs_change)
			hs_spi_cs_control(spi_control, SSP_CHIP_SELECT);
	} else {
		/* STATE_START */
		message->state = STATE_RUNNING;
	}

	if (set_up_next_transfer(spi_control, transfer)) {
		message->state = STATE_ERROR;
		message->status = -EIO;
		giveback(spi_control);
		return;
	}
	/* Flush the FIFOs and let's go! */
	flush(spi_control);

	if (spi_control->cur_chip->enable_dma) {
		if (configure_dma(spi_control)) {
			dev_dbg(&spi_control->pdev->dev,
				"configuration of DMA failed, fall back to interrupt mode\n");
			goto err_config_dma;
		}
		return;
	}

err_config_dma:
	/* enable all interrupts except RX */
	writel(ENABLE_ALL_INTERRUPTS & SSP_IMC_MASK_RXIM,
		SSP_IMC(spi_control->virtbase));
}

static void do_interrupt_dma_transfer(struct hs_spi_controller *spi_control)
{
	/*
	 * Default is to enable all interrupts except RX -
	 * this will be enabled once TX is complete
	 */
	u32 irqflags = (u32)(ENABLE_ALL_INTERRUPTS & SSP_IMC_MASK_RTIM);
	/* Enable target chip, if not already active */
	if (!spi_control->next_msg_cs_active)
		hs_spi_cs_control(spi_control, SSP_CHIP_SELECT);

	if (set_up_next_transfer(spi_control, spi_control->cur_transfer)) {
		/* Error path */
		spi_control->cur_msg->state = STATE_ERROR;
		spi_control->cur_msg->status = -EIO;
		giveback(spi_control);
		return;
	}
	mutex_lock(&spi_control->master->msg_mutex);
	/* If we're using DMA, set up DMA here */
	if (spi_control->cur_chip->enable_dma) {
		/* Configure DMA transfer */
		if (configure_dma(spi_control)) {
			dev_dbg(&spi_control->pdev->dev,
				"configuration of DMA failed, fall back to interrupt mode\n");
			goto err_config_dma;
		}
		/* Disable interrupts in DMA mode, IRQ from DMA controller */
		irqflags = DISABLE_ALL_INTERRUPTS & ~SSP_IMC_MASK_RORIM;
	}
err_config_dma:
	/* Enable SSP, turn on interrupts */
	writel(irqflags, SSP_IMC(spi_control->virtbase));
	writel((readl(SSP_ENR(spi_control->virtbase)) | SSP_ENR_MASK_EN),
			SSP_ENR(spi_control->virtbase));

	mutex_unlock(&spi_control->master->msg_mutex);
}

static void do_polling_transfer(struct hs_spi_controller *spi_control)
{
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;
	struct spi_transfer *previous = NULL;
	struct chip_data *chip = NULL;
	unsigned long time, timeout;
	unsigned long flags = 0;

	if (spi_control->sync_locked)
		spin_lock_irqsave(&spi_control->sync_spinlock, flags);

	chip = spi_control->cur_chip;
	message = spi_control->cur_msg;

	while (message->state != STATE_DONE) {
		/* Handle for abort */
		if (message->state == STATE_ERROR)
			break;
		transfer = spi_control->cur_transfer;

		/* Delay if requested at end of transfer */
		if (message->state == STATE_RUNNING) {
			previous =
				list_entry(transfer->transfer_list.prev,
					struct spi_transfer, transfer_list);
			if (previous->delay_usecs)
				udelay(previous->delay_usecs);
			if (previous->cs_change)
				hs_spi_cs_control(spi_control, SSP_CHIP_SELECT);
		} else {
			/* STATE_START */
			message->state = STATE_RUNNING;
			if (!spi_control->next_msg_cs_active)
				hs_spi_cs_control(spi_control, SSP_CHIP_SELECT);
		}

		/* Configuration Changing Per Transfer */
		if (set_up_next_transfer(spi_control, transfer)) {
			/* Error path */
			message->state = STATE_ERROR;
			break;
		}
		/* Flush FIFOs and enable SSP */

		flush(spi_control);
		writel((readl(SSP_ENR(spi_control->virtbase)) |
				SSP_ENR_MASK_EN),
				SSP_ENR(spi_control->virtbase));

		dev_dbg(&spi_control->pdev->dev, "polling transfer ongoing ...\n");

		timeout = jiffies + msecs_to_jiffies(SPI_POLLING_TIMEOUT);
		while (spi_control->tx < spi_control->tx_end ||
			spi_control->rx < spi_control->rx_end) {
			time = jiffies;
			readwriter(spi_control);
			if (time_after(time, timeout)) {
				dev_err(&spi_control->pdev->dev,
				"%s: timeout!\n", __func__);
				message->state = STATE_ERROR;
				goto out;
			}
			if (!spi_control->sync_locked)
				cpu_relax();
		}

		/* Update total byte transferred */
		message->actual_length += spi_control->cur_transfer->len;
		if (spi_control->cur_transfer->cs_change)
			hs_spi_cs_control(spi_control, SSP_CHIP_DESELECT);
		/* Move to next transfer */
		message->state = next_transfer(spi_control);
	}
out:
	/* Handle end of message */
	if (message->state == STATE_DONE) {
		message->status = 0;
		flush(spi_control);
	} else {
		message->status = -EIO;
	}

	giveback(spi_control);

	if (spi_control->sync_locked)
		spin_unlock_irqrestore(&spi_control->sync_spinlock, flags);
}

static int hs_spi_transfer_one_message(struct spi_master *master,
					struct spi_message *msg)
{
	struct hs_spi_controller *spi_control = spi_master_get_devdata(master);

	/* Initial message state */
	spi_control->cur_msg = msg;
	msg->state = STATE_START;

	spi_control->cur_transfer = list_entry(msg->transfers.next,
					 struct spi_transfer, transfer_list);

	/* Setup the SPI using the per chip configuration */
	spi_control->cur_chip = spi_get_ctldata(msg->spi);

	if (msg->spi->chip_select <
		spi_control->pdata->num_chipselect)
		spi_control->cur_cs =
			spi_control->chipselects[msg->spi->chip_select];
	else
		dev_err(&spi_control->pdev->dev, "%s: Bad chip_select:%d\n",
			__func__, msg->spi->chip_select);

	restore_state(spi_control);
	flush(spi_control);

	if (spi_control->cur_chip->xfer_type == POLLING_TRANSFER)
		do_polling_transfer(spi_control);
	else
		do_interrupt_dma_transfer(spi_control);

	return 0;
}

void disable_spi(struct spi_master *ctlr)
{
	struct hs_spi_controller *spi_control = spi_master_get_devdata(ctlr);

	/* nothing more to do - disable spi/ssp and power off */
	writel((readl(SSP_ENR(spi_control->virtbase)) &
		(~SSP_ENR_MASK_EN)), SSP_ENR(spi_control->virtbase));
}

static int hs_spi_prepare_transfer_hardware(struct spi_master *master)
{
	struct hs_spi_controller *spi_control = spi_master_get_devdata(master);
	struct hwspinlock *hwlock = spi_control->spi_hwspin_lock;
	unsigned long time, timeout;

	int ret;

	/*
	 * Just make sure we have all we need to run the transfer by syncing
	 * with the runtime PM framework.
	 */
	timeout = jiffies + msecs_to_jiffies(GET_HARDWARE_TIMEOUT);
	if (!spi_control->hardware_mutex) {
		dev_dbg(&spi_control->pdev->dev,
			"%s hardware_mutex is null\n", __func__);
		return 0;
	}

	do {
		ret = hwlock->bank->ops->trylock(hwlock);
		if (ret)
			break;

		time = jiffies;
		if (time_after(time, timeout)) {
			dev_err(&spi_control->pdev->dev,
				"Get hardware_mutex complete timeout\n");
			return -ETIME;
		}
		/* sleep 1000~2000 us */
		usleep_range(1000, 2000);
	} while (!ret);

	return 0;
}

static int hs_spi_unprepare_transfer_hardware(struct spi_master *master)
{
	struct hs_spi_controller *spi_control = spi_master_get_devdata(master);

	struct hwspinlock *hwlock = spi_control->spi_hwspin_lock;

	if (spi_control->hardware_mutex)
		hwlock->bank->ops->unlock(hwlock);

	return 0;
}
static int spi_v500_get_pins_data(struct hs_spi_controller *spi_control,
			struct device *dev)
{
	int status;

	spi_control->pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR(spi_control->pinctrl)) {
		dev_err(dev, "%s pinctrl is error\n", __func__);
		return -EFAULT;
	}

	spi_control->pins_default =
		pinctrl_lookup_state(spi_control->pinctrl,
			PINCTRL_STATE_DEFAULT);
	/* enable pins to be muxed in and configured */
	if (!IS_ERR(spi_control->pins_default)) {
		status = pinctrl_select_state(
			spi_control->pinctrl, spi_control->pins_default);
		if (status)
			dev_err(dev, "could not set default pins\n");
	} else {
		dev_err(dev, "could not get default pinstate\n");
	}

	spi_control->pins_idle =
		pinctrl_lookup_state(spi_control->pinctrl, PINCTRL_STATE_IDLE);
	if (IS_ERR(spi_control->pins_idle))
		dev_dbg(dev, "could not get idle pinstate\n");

	spi_control->pins_sleep =
		pinctrl_lookup_state(spi_control->pinctrl, PINCTRL_STATE_SLEEP);
	if (IS_ERR(spi_control->pins_sleep))
		dev_dbg(dev, "could not get sleep pinstate\n");

	spi_control->pins.p = spi_control->pinctrl;
	spi_control->pins.default_state = spi_control->pins_default;
#ifdef CONFIG_PM
	spi_control->pins.idle_state = spi_control->pins_idle;
	spi_control->pins.sleep_state = spi_control->pins_sleep;
#endif
	dev->pins = &spi_control->pins;

	return 0;
}

static int verify_controller_parameters(struct hs_spi_controller *spi_control,
	struct hs_spi_config_chip const *chip_info)
{
	if ((chip_info->com_mode != INTERRUPT_TRANSFER)
		&& (chip_info->com_mode != DMA_TRANSFER)
		&& (chip_info->com_mode != POLLING_TRANSFER)) {
		dev_err(&spi_control->pdev->dev,
			"Communication mode is configured incorrectly\n");
		return -EINVAL;
	}

	return 0;
}

static inline u32 spi_rate(u32 rate, u16 cpsdvsr, u16 scr)
{
	return rate / (cpsdvsr * (1 + scr));
}

static int spi_freq_check(struct hs_spi_controller *spi_control,
	u32 freq, u32 rate)
{
	int ret = 0;
	u32 max_tclk, min_tclk;

	/* cpsdvscr = 2 & scr 0 */
	max_tclk = spi_rate(rate, CPSDVR_MIN, SCR_MIN);
	/* cpsdvsr = 254 & scr = 255 */
	min_tclk = spi_rate(rate, CPSDVR_MAX, SCR_MAX);

	if (freq > max_tclk) {
		dev_warn(&spi_control->pdev->dev,
			"Max speed that can be programmed is %d Hz, you requested %d\n",
			max_tclk, freq);
		freq = max_tclk;
	}
	if (freq < min_tclk) {
		dev_err(&spi_control->pdev->dev,
			"Requested frequency: %d Hz is less than minimum possible %d Hz\n",
			freq, min_tclk);
		ret = -EINVAL;
	}

	return ret;
}

static int calculate_effective_freq(struct hs_spi_controller *spi_control,
	u32 freq, struct ssp_clock_params *clk_freq)
{
	/* Lets calculate the frequency parameters */
	u16 cpsdvsr = CPSDVR_MIN;
	u16 scr = SCR_MIN;
	u32 rate;
	u32 tmp = 0;
	u32 best_freq = 0;
	u32 best_cpsdvsr = 0;
	u32 best_scr = 0;

	rate = clk_get_rate(spi_control->clk);
	if (spi_freq_check(spi_control, freq, rate))
		return -EINVAL;

	/*
	 * best_freq will give closest possible available rate (<= requested
	 * freq) for all values of scr & cpsdvsr.
	 */
	while ((cpsdvsr <= CPSDVR_MAX) && (tmp != freq)) {
		while (scr <= SCR_MAX) {
			tmp = spi_rate(rate, cpsdvsr, scr);
			if (tmp > (u32)freq) {
				/* we need lower freq */
				scr++;
				continue;
			}

			/*
			 * If found exact value, mark found and break.
			 * If found more closer value, update and break.
			 */
			if (tmp > best_freq) {
				best_freq = tmp;
				best_cpsdvsr = cpsdvsr;
				best_scr = scr;
			}
			/*
			 * increased scr will give lower rates, which are not
			 * required
			 */
			break;
		}
		cpsdvsr += 2;
		scr = SCR_MIN;
	}

	WARN(!best_freq,
		"hs_spi: Matching cpsdvsr and scr not found for %d Hz rate\n",
		freq);

	clk_freq->cpsdvsr = (u8) (best_cpsdvsr & 0xFF);
	clk_freq->scr = (u8) (best_scr & 0xFF);
	dev_dbg(&spi_control->pdev->dev,
		"SSP Target Frequency is: %u, Effective Frequency is %u\n",
		freq, best_freq);
	dev_dbg(&spi_control->pdev->dev, "SSP cpsdvsr = %d, scr = %d\n",
		clk_freq->cpsdvsr, clk_freq->scr);

	return 0;
}

/*
 * A piece of default chip info unless the platform
 * supplies it.
 */
static struct hs_spi_config_chip hs_spi_default_chip_info = {
	.com_mode = POLLING_TRANSFER,
	.speed_mode = SSP_NOMAL,
	.cs_ctrl_en = SSP_CS_CTRL_DISENABLE,
	.rx_lev_trig = SSP_RX_1_OR_MORE_ELEM,
	.tx_lev_trig = SSP_TX_1_OR_MORE_EMPTY_LOC,
	.cs_control = null_cs_control,
};

static int hs_spi_bits_word_check(struct spi_device *spi,
			struct chip_data *chip)
{
	int status = 0;
	unsigned int bits = spi->bits_per_word;

	/* Check bits per word with vendor specific range */
	if (bits <= 3) {
		status = E_SPI_PARAMS;
		dev_err(&spi->dev, "illegal data size for this controller!\n");
		return status;
	} else if (bits <= 8) {
		dev_dbg(&spi->dev, "4 <= n <=8 bits per word\n");
		chip->n_bytes = 1;
		chip->read = READING_U8;
		chip->write = WRITING_U8;
	} else if (bits <= 16) {
		dev_dbg(&spi->dev, "9 <= n <= 16 bits per word\n");
		chip->n_bytes = 2;
		chip->read = READING_U16;
		chip->write = WRITING_U16;
	} else {
		dev_dbg(&spi->dev, "17 <= n <= 32 bits per word\n");
		chip->n_bytes = 4;
		chip->read = READING_U32;
		chip->write = WRITING_U32;
	}
	return status;
}

static struct hs_spi_config_chip *hs_spi_control_data_get_by_dt(
		struct hs_spi_config_chip *chip_info,
		struct spi_device *spi)
{
	struct device_node *np = spi->dev.of_node;
	struct hs_spi_config_chip chip_info_dt;

	if (chip_info == NULL) {
		if (np) {
			chip_info_dt = hs_spi_default_chip_info;
			of_property_read_u32(np, "hs_spi,com-mode",
				&chip_info_dt.com_mode);
			of_property_read_u32(np, "hs_spi,rx-level-trig",
				&chip_info_dt.rx_lev_trig);
			of_property_read_u32(np, "hs_spi,tx-level-trig",
				&chip_info_dt.tx_lev_trig);
			chip_info = &chip_info_dt;
		} else {
			chip_info = &hs_spi_default_chip_info;
			/* spi_board_info.controller_data not is supplied */
			dev_dbg(&spi->dev,
				"using default controller_data settings\n");
		}
	} else {
		dev_dbg(&spi->dev,
			"using user supplied controller_data settings\n");
	}
	return chip_info;
}

static void hs_spi_chipdata_init(struct hs_spi_config_chip *chip_info,
	struct hs_spi_controller *spi_control, struct spi_device *spi,
	struct chip_data *chip, struct ssp_clock_params clk_freq)
{
	u32 tmp;

	/* Now Initialize all register settings required for this chip */
	chip->csr = 0;
	chip->cr = 0;
	chip->enr = 0;

	if ((chip_info->com_mode == DMA_TRANSFER)
		&& ((spi_control->pdata)->enable_dma)) {
		chip->enable_dma = true;
		dev_dbg(&spi->dev, "DMA mode set in controller state\n");
		SSP_WRITE_BITS(chip->cr, SSP_DMA_ENABLED,
			SSP_COMMONCR_MASK_TXDMAE, BIT_TXDMAE);
		SSP_WRITE_BITS(chip->cr, SSP_DMA_ENABLED,
			SSP_COMMONCR_MASK_RXDMAE, BIT_RXDMAE);
	} else {
		chip->enable_dma = false;
		dev_dbg(&spi->dev, "DMA mode NOT set in controller state\n");
		SSP_WRITE_BITS(chip->cr, SSP_DMA_DISABLED,
			SSP_COMMONCR_MASK_TXDMAE, BIT_TXDMAE);
		SSP_WRITE_BITS(chip->cr, SSP_DMA_DISABLED,
			SSP_COMMONCR_MASK_RXDMAE, BIT_RXDMAE);
	}
	SSP_WRITE_BITS(chip->csr, (u32)chip_info->cs_ctrl_en,
		SSP_CSCR_MASK_CSEN, BIT_CSEN);

	chip->csr = clk_freq.cpsdvsr;
	/* Effective data bit width */
	SSP_WRITE_BITS(chip->cr, (u32)spi->bits_per_word - 1,
		SSP_COMMONCR_MASK_FRAMESIZE, BIT_FRAME);

	/* spi speed mode */
	SSP_WRITE_BITS(chip->cr, (u32)chip_info->speed_mode,
		SSP_COMMONCR_MASK_HSPD, BIT_HSPD);

	/* Stuff that is common for all versions */
	if (spi->mode & SPI_CPOL)
		tmp = SSP_CLK_POL_IDLE_HIGH;
	else
		tmp = SSP_CLK_POL_IDLE_LOW;
	SSP_WRITE_BITS(chip->cr, (u32)tmp,
		SSP_COMMONCR_MASK_CPOL, BIT_CPOL);

	if (spi->mode & SPI_CPHA)
		tmp = SSP_CLK_SECOND_EDGE;
	else
		tmp = SSP_CLK_FIRST_EDGE;
	SSP_WRITE_BITS(chip->cr, (u32)tmp, SSP_COMMONCR_MASK_CPHA, BIT_CPHA);

	SSP_WRITE_BITS(chip->cr, (u32)clk_freq.scr, SSP_COMMONCR_MASK_DRPO,
		BIT_DRPO);

	/* Loopback config */
	if (spi->mode & SPI_LOOP)
		tmp = LOOPBACK_ENABLED;
	else
		tmp = LOOPBACK_DISABLED;
	SSP_WRITE_BITS(chip->cr, (u32)tmp, SSP_COMMONCR_LOOPBACK, 1);
	SSP_WRITE_BITS(chip->enr, SSP_DISABLED, SSP_ENR_MASK_EN, 0);
}
/*
 * hs_spi_controller_setup - setup function registered to SPI master framework
 * @spi: spi device which is requesting setup
 */
static int hs_spi_controller_setup(struct spi_device *spi)
{
	struct hs_spi_config_chip *chip_info = NULL;
	struct chip_data *chip = NULL;
	struct ssp_clock_params clk_freq = {.cpsdvsr = 0, .scr = 0};
	int status = 0;
	struct hs_spi_controller *spi_control =
		spi_master_get_devdata(spi->master);

	if (!spi->max_speed_hz)
		return -EINVAL;

	/* Get controller_state if one is supplied */
	chip = spi_get_ctldata(spi);
	if (chip == NULL) {
		chip = kzalloc(sizeof(struct chip_data), GFP_KERNEL);
		if (!chip)
			return -ENOMEM;
		dev_dbg(&spi->dev,
			"allocated memory for controller's runtime state\n");
	}

	/* Get controller data if one is supplied */
	chip_info = spi->controller_data;
	chip_info = hs_spi_control_data_get_by_dt(chip_info, spi);
	/*
	 * We can override with custom divisors, else we use the board
	 * frequency setting
	 */
	if ((chip_info->clk_freq.cpsdvsr == 0)
		&& (chip_info->clk_freq.scr == 0)) {
		status = calculate_effective_freq(spi_control,
						spi->max_speed_hz,
						&clk_freq);
		if (status < 0)
			goto err_config_params;
	} else {
		memcpy_s(&clk_freq, sizeof(clk_freq),
			&chip_info->clk_freq, sizeof(clk_freq));
		if ((clk_freq.cpsdvsr % 2) != 0)
			clk_freq.cpsdvsr =
				clk_freq.cpsdvsr - 1;
	}
	if ((clk_freq.cpsdvsr < CPSDVR_MIN)
		|| (clk_freq.cpsdvsr > CPSDVR_MAX)) {
		status = -EINVAL;
		dev_err(&spi->dev,
			"cpsdvsr is configured incorrectly\n");
		goto err_config_params;
	}

	status = verify_controller_parameters(spi_control, chip_info);
	if (status) {
		dev_err(&spi->dev, "controller data is incorrect");
		goto err_config_params;
	}

	spi_control->rx_lev_trig = chip_info->rx_lev_trig;
	spi_control->tx_lev_trig = chip_info->tx_lev_trig;

	/* Now set controller state based on controller data */
	chip->xfer_type = chip_info->com_mode;
	if (!chip_info->cs_control) {
		chip->cs_control = null_cs_control;
		if (!gpio_is_valid(spi_control->chipselects[spi->chip_select]))
			dev_warn(&spi->dev,
				 "invalid chip select\n");
	} else {
		chip->cs_control = chip_info->cs_control;
	}
	/* Check bits per word with vendor specific range */
	if (hs_spi_bits_word_check(spi, chip))
		goto err_config_params;
	/* Now Initialize all register settings required for this chip */
	hs_spi_chipdata_init(chip_info, spi_control, spi, chip, clk_freq);
	/* Save controller_state */
	spi_set_ctldata(spi, chip);
	return status;
err_config_params:
	spi_set_ctldata(spi, NULL);
	kfree(chip);
	return status;
}

/**
 * hs_spi_cleanup - cleanup function registered to SPI master framework
 * @spi: spi device which is requesting cleanup
 *
 * This function is registered to the SPI framework for this SPI master
 * controller. It will free the runtime state of chip.
 */
static void hs_spi_cleanup(struct spi_device *spi)
{
	struct chip_data *chip = spi_get_ctldata(spi);

	spi_set_ctldata(spi, NULL);
	kfree(chip);
}

static struct hs_spi_platform_data *
hs_spi_platform_data_dt_get(struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct hs_spi_platform_data *pd = NULL;
	u32 tmp = 0xFFFFFFFF;

	if (!np) {
		dev_err(dev, "no dt node defined\n");
		return NULL;
	}

	pd = devm_kzalloc(dev, sizeof(struct hs_spi_platform_data), GFP_KERNEL);
	if (!pd)
		return NULL;

	if (of_property_read_u32(np, "bus-id", &tmp))
		dev_err(dev, "no bus-id defined\n");
	pd->bus_id = tmp;
	dev_info(dev,
		 "spi driver, bus id: 0x%08x\n", pd->bus_id);
	pd->enable_dma = 1;
	of_property_read_u32(np, "num-cs", &tmp);
	pd->num_chipselect = tmp;

	return pd;
}

static void hs_spi_show_err_info(struct spi_master *master)
{
	struct hs_spi_controller *spi_control = spi_master_get_devdata(master);

	dev_err(&spi_control->pdev->dev, "%s state is %d\n", __func__,
			spi_control->transfer_state);
	dev_err(&spi_control->pdev->dev, "%s len is %u\n", __func__,
			spi_control->transfer_len);
	if (spi_control->transfer_state == STATE_CFG_DMA_DONE) {
		show_dma64_reg(spi_control->dma_rx_channel);
		show_dma64_reg(spi_control->dma_tx_channel);
	}
}

bool spi_use_dma_transmode(struct spi_message *msg)
{
	struct chip_data *cur_chip = spi_get_ctldata(msg->spi);

	if (!cur_chip)
		return false;

	if ((cur_chip->xfer_type == DMA_TRANSFER) &&
		(cur_chip->enable_dma == true))
		return true;

	return false;
}

/**
 *	platform_request_regions - release mem regions associated with device
 *	@dev: platform_device structure for device
 *
 *	Release regions claimed by a successful call to amba_request_regions.
 */
static void platform_release_regions(struct platform_device *dev)
{
	u32 size;

	size = resource_size(dev->resource);
	release_mem_region(dev->resource->start, size);
}

static int hs_spi_cfg_from_dt(struct platform_device *pdev,
			struct hs_spi_controller *spi_control,
			struct hs_spi_platform_data *platform_data,
			u64 *clk_rate)
{
	int status = 0;
	struct device_node *np = pdev->dev.of_node;

	status = of_property_read_u64(np, "clock-rate", clk_rate);
	if (status)
		dev_info(&pdev->dev, "[%s] node %s doesn't have clock-rate property!\n",
			__func__, np->name);

	if (of_property_read_u32(np, "hardware-mutex",
		&spi_control->hardware_mutex))
		spi_control->hardware_mutex = 0;
	if (spi_control->hardware_mutex) {
		if (spi_control->hardware_mutex == HARDWARE_LOCK_DEFAULT) {
			/* if hardware-mutex config to 1 in dts,
			 * then hardware lock is default value 27
			 */
			spi_control->spi_hwspin_lock =
				hwspin_lock_request_specific(
					ENUM_SPI_HWSPIN_LOCK);
		} else {
			/* if hardware-mutex is not 0 or 1 int dts,
			 * then hardware lock is config value in dts
			 */
			spi_control->spi_hwspin_lock =
				hwspin_lock_request_specific(
					spi_control->hardware_mutex);
		}

		if (!spi_control->spi_hwspin_lock) {
			dev_err(&pdev->dev, "spi_hwspin_lock request error\n");
			return E_HWSPI_LOCK;
		}
	}

	if (of_find_property(np, "dma-buffer-check", NULL)) {
		spi_control->dmacheck_addr = 1;
		spi_control->tx_buffer = NULL;
		spi_control->rx_buffer = NULL;
	} else {
		spi_control->dmacheck_addr = 0;
	}

	if (of_property_read_u32(np, "sync-spinlock",
		&spi_control->sync_locked))
		spi_control->sync_locked = 0;
	if (spi_control->sync_locked)
		spin_lock_init(&spi_control->sync_spinlock);
	spin_lock_init(&spi_control->lock);

	return 0;
}

static void hs_spi_master_cfg(struct hs_spi_platform_data *platform_data,
	struct spi_master *master, struct device *dev)
{
	/*
	 * Bus Number Which has been Assigned to this SSP controller
	 * on this board
	 */
	master->bus_num = platform_data->bus_id;
	master->num_chipselect = platform_data->num_chipselect;
	master->cleanup = hs_spi_cleanup;
	master->setup = hs_spi_controller_setup;
	master->prepare_transfer_hardware =
		hs_spi_prepare_transfer_hardware;
	master->show_err_info = hs_spi_show_err_info;
	master->transfer_one_message =
		hs_spi_transfer_one_message;
	master->unprepare_transfer_hardware =
		hs_spi_unprepare_transfer_hardware;
	master->dev.of_node = dev->of_node;
	/*
	 * Supports mode 0-3, loopback, and active low CS.
	 */
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH | SPI_LOOP;
	dev_dbg(dev, "BUSNO: %d\n", master->bus_num);
}

static int hs_spi_clk_cfg(struct platform_device *pdev,
	struct hs_spi_controller *spi_control, u64 clk_rate)
{
	int ret = 0;
	int status = 0;

	spi_control->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(spi_control->clk)) {
		status = PTR_ERR(spi_control->clk);
		dev_err(&pdev->dev, "could not retrieve SSP/SPI bus clock\n");
		ret = E_SPI_NO_CLK;
		return ret;
	}
	status = clk_prepare_enable(spi_control->clk);
	if (status) {
		dev_err(&pdev->dev, "could not enable SSP/SPI bus clock\n");
		ret = E_SPI_NO_CLK_EN;
		return ret;
	}

	if (clk_rate > 0) {
		status = clk_set_rate(spi_control->clk, clk_rate);
		if (status) {
			dev_err(&pdev->dev,
				"could not set SSP/SPI rate status[0x%x]\n",
				status);
			ret = E_SPI_CLK_SET_RATE;
			return ret;
		}
		clk_rate =  clk_get_rate(spi_control->clk);
		dev_info(&pdev->dev, "clk_rate:%lld\n", clk_rate);
	}

	return ret;
}

static int hs_spi_probe_paramter_check(struct platform_device *pdev,
	struct hs_spi_platform_data **platform_data)
{
	*platform_data = hs_spi_platform_data_dt_get(&pdev->dev);

	if (!*platform_data) {
		dev_err(&pdev->dev, "probe: no platform data defined\n");
		return -ENODEV;
	}

	if (!(*platform_data)->num_chipselect) {
			dev_err(&pdev->dev, "probe: no chip select defined\n");
			return -ENODEV;
	}

	return 0;
}

static int hs_spi_probe_trans_cfg(struct platform_device *pdev,
			struct hs_spi_platform_data *platform_data,
			struct hs_spi_controller *spi_control)
{
	int status = 0;

	/* Initialize transfer pump */
	tasklet_init(&spi_control->pump_transfers, pump_transfers,
		(uintptr_t)spi_control);

	/* Disable SSP */
	writel((readl(SSP_ENR(spi_control->virtbase)) & (~SSP_ENR_MASK_EN)),
			SSP_ENR(spi_control->virtbase));
	load_ssp_default_config(spi_control);

	status = devm_request_irq(&pdev->dev, spi_control->irq,
			hs_spi_interrupt_handler,
			0, pdev->name, spi_control);
	if (status < 0) {
		dev_err(&pdev->dev, "probe - cannot get IRQ (%d)\n", status);
		return status;
	}

	/* Get DMA channels, try autoconfiguration first */
	status = hs_spi_dma_autoprobe(spi_control);
	if (status == -EPROBE_DEFER) {
		dev_dbg(&pdev->dev, "deferring probe to get DMA channel\n");
		return status;
	}

	/* If that failed, use channels from platform_data */
	if (status == 0) {
		platform_data->enable_dma = 1;
	} else if (platform_data->enable_dma) {
		status = hs_spi_dma_probe(spi_control);
		if (status != 0)
			platform_data->enable_dma = 0;
	}
	return status;
}
static int hs_spi_master_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hs_spi_platform_data *platform_data =
			dev_get_platdata(&pdev->dev);
	struct spi_master *master = NULL;
	struct resource *iores = NULL;
	struct hs_spi_controller *spi_control = NULL;
	int status = 0;
	u64 clk_rate = 0;

	status = hs_spi_probe_paramter_check(pdev, &platform_data);
	if (status)
		return -ENODEV;

	/* Allocate master with space for data */
	master = spi_alloc_master(dev, sizeof(struct hs_spi_controller));
	if (master == NULL) {
		dev_err(&pdev->dev, "probe - cannot alloc SPI master\n");
		return -ENOMEM;
	}

	spi_control = spi_master_get_devdata(master);
	spi_control->master = master;
	spi_control->pdata = platform_data;
	spi_control->pdev = pdev;
	spi_control->irq = platform_get_irq(pdev, 0);
	spi_control->chipselects = devm_kzalloc(dev,
					platform_data->num_chipselect * sizeof(int),
					GFP_KERNEL);
	if (!spi_control->chipselects) {
		status = -ENOMEM;
		goto err_no_mem;
	}

	status = spi_v500_get_pins_data(spi_control, dev);
	if (status)
		goto err_no_pinctrl;

	status = hs_spi_cfg_from_dt(pdev, spi_control, platform_data,
		&clk_rate);
	if (status)
		goto hwspin_lock_err0;

	hs_spi_master_cfg(platform_data, master, dev);

	iores = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	spi_control->phybase = iores->start;
	spi_control->virtbase = devm_ioremap_resource(dev, iores);

	if (spi_control->virtbase == NULL) {
		status = -ENOMEM;
		goto err_no_ioremap;
	}
	dev_info(&pdev->dev, "mapped registers from %pa to %pK\n",
		&pdev->resource->start, spi_control->virtbase);

	status = hs_spi_clk_cfg(pdev, spi_control, clk_rate);
	if (status) {
		switch (status) {
		case E_SPI_NO_CLK:
			goto err_no_clk;
		case E_SPI_NO_CLK_EN:
			goto err_no_clk_en;
		case E_SPI_CLK_SET_RATE:
			goto err_clk_set_rate;
		default:
			break;
		}
	}
	status = hs_spi_probe_trans_cfg(pdev, platform_data, spi_control);
	if (status)
		goto err_no_irq;

	/* Register with the SPI framework */
	platform_set_drvdata(pdev, spi_control);
	status = devm_spi_register_master(&pdev->dev, master);
	if (status != 0) {
		dev_err(&pdev->dev,
			"probe - problem registering spi master\n");
		goto err_spi_register;
	}
	dev_err(dev, "probe succeeded\n");

	clk_disable_unprepare(spi_control->clk);

	return 0;

err_spi_register:
	if (platform_data->enable_dma)
		hs_spi_dma_remove(spi_control);
err_clk_set_rate:
err_no_irq:
	clk_disable_unprepare(spi_control->clk);
err_no_clk_en:
err_no_clk:
err_no_ioremap:
	platform_release_regions(pdev);
hwspin_lock_err0:
err_no_pinctrl:
err_no_mem:
	spi_master_put(master);
	return status;
}

static int
hs_spi_master_remove(struct platform_device *pdev)
{
	struct hs_spi_controller *spi_control = platform_get_drvdata(pdev);

	if (!spi_control)
		return 0;

	/*
	 * undo pm_runtime_put() in probe.  I assume that we're not
	 * accessing the primecell here.
	 */
	pm_runtime_get_noresume(&pdev->dev);

	load_ssp_default_config(spi_control);
	if (spi_control->pdata->enable_dma)
		hs_spi_dma_remove(spi_control);

	clk_disable_unprepare(spi_control->clk);
	platform_release_regions(pdev);
	tasklet_disable(&spi_control->pump_transfers);
	spi_unregister_master(spi_control->master);
	platform_set_drvdata(pdev, NULL);
	return 0;
}
#ifdef CONFIG_OF
static const struct of_device_id hs_spi_master_of_match[] = {
	{
		.compatible = "hisilicon,hisi-spi-master",
	},
	{},
};
MODULE_DEVICE_TABLE(of, hs_spi_master_of_match);
#endif

#ifdef CONFIG_PM_SLEEP
static int hs_spi_suspend(struct device *dev)
{
	struct hs_spi_controller *spi_control = dev_get_drvdata(dev);
	int ret = 0;

	dev_err(dev, "%s: +\n", __func__);
	ret = spi_master_suspend(spi_control->master);
	if (ret) {
		dev_err(dev, "cannot suspend master\n");
		return ret;
	}

	dev_err(dev, "%s: -\n", __func__);
	return 0;
}

static int hs_spi_resume(struct device *dev)
{
	struct hs_spi_controller *spi_control = dev_get_drvdata(dev);
	int ret;

	dev_err(dev, "%s: +\n", __func__);
	/* Start the queue running */
	ret = spi_master_resume(spi_control->master);
	if (ret)
		dev_err(dev, "problem starting queue (%d)\n", ret);

	dev_err(dev, "%s: -\n", __func__);

	return ret;
}
#endif

#ifdef CONFIG_PM
int pl022_runtime_suspend(struct device *dev)
{
	struct hs_spi_controller *spi_control = dev_get_drvdata(dev);

	clk_disable_unprepare(spi_control->clk);
	pinctrl_pm_select_idle_state(dev);

	return 0;
}

int pl022_runtime_resume(struct device *dev)
{
	struct hs_spi_controller *spi_control = dev_get_drvdata(dev);

	pinctrl_pm_select_default_state(dev);
	clk_prepare_enable(spi_control->clk);

	return 0;
}
#endif

static const struct dev_pm_ops hs_spi_master_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(hs_spi_suspend, hs_spi_resume)
};

static struct platform_driver hs_spi_master_driver = {
	.driver = {
		.name = "hs_spi_master",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hs_spi_master_of_match),
		.pm	= &hs_spi_master_dev_pm_ops,
	},
	.probe		= hs_spi_master_probe,
	.remove		= hs_spi_master_remove,
};

static int __init spi_v500_init(void)
{
	return platform_driver_register(&hs_spi_master_driver);
}

static void __exit spi_v500_exit(void)
{
	platform_driver_unregister(&hs_spi_master_driver);
}

subsys_initcall(spi_v500_init);
module_exit(spi_v500_exit);

MODULE_DESCRIPTION("spi controller driver");
MODULE_ALIAS("platform:spi_master_hs");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

