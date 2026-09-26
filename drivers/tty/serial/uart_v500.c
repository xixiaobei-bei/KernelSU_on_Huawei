/*
 * Driver for UART V500 serial ports
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
 *
 * This is a generic driver for UART V500 serial ports.  They
 * have a lot of 16550-like features, but are not register compatible.
 * Note that although they do have CTS, DCD and DSR inputs, they do
 * not have an RI input, nor do they have DTR or RTS outputs. If
 * required, these have to be supplied via some other means (eg, GPIO)
 * and hooked into this driver.
 */
#if defined(CONFIG_SERIAL_UART_V500_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif
#include "uart_v500.h"

static int uart_v500_dma_tx_refill(struct uart_v500_port *uap);
static void uart_v500_start_tx_pio(struct uart_v500_port *uap);
static void uart_v500_dma_rx_callback(void *data);
static bool uart_v500_tx_chars(struct uart_v500_port *uap, bool from_irq);
struct uart_v500_port *uart_ports_ne[MAX_UART_IDX];

struct uart_v500_port *get_uart_ports_uap(int index)
{
	if ((index >= MAX_UART_IDX) || (index < 0))
		return NULL;

	return uart_ports_ne[index];
}

unsigned int uart_v500_reg_to_offset(const struct uart_v500_port *uap,
	unsigned int reg)
{
	return uap->reg_offset[reg];
}

unsigned int uart_v500_read(const struct uart_v500_port *uap, unsigned int reg)
{
	void __iomem *addr = NULL;

	addr = uap->port.membase + uart_v500_reg_to_offset(uap, reg);
	return (uap->port.iotype == UPIO_MEM32) ? readl_relaxed(addr)
		: readw_relaxed(addr);
}

void uart_v500_write(unsigned int val, const struct uart_v500_port *uap,
	unsigned int reg)
{
	void __iomem *addr = NULL;

	addr = uap->port.membase + uart_v500_reg_to_offset(uap, reg);
	if (uap->port.iotype == UPIO_MEM32)
		writel_relaxed(val, addr);
	else
		writew_relaxed(val, addr);
}

static int uart_v500_probe_get_clk_freq(struct platform_device *dev,
	struct uart_v500_port *uap, int num)
{
	int ret;
	u64 clk_rate = 0; /* high speed clk vaule */

	ret = of_property_read_u64(dev->dev.of_node, "clock-rate", &clk_rate);
	if (!ret && (clk_rate > 0)) {
		dev_info(&dev->dev, "%s:reset clock rate[%lld]!\n", __func__,
			clk_rate);
		/* reset the clock rate */
		ret = clk_set_rate(uap->clk, clk_rate);
		if (ret) {
			dev_err(&dev->dev, "%s: clk set:%lld fail! ret:0x%x\n",
				__func__, clk_rate, ret);
			devm_clk_put(&dev->dev, uap->clk);
		}
	} else {
		dev_info(&dev->dev, "UART%d haven't clock-rate value in dts!\n",
			num);
	}

	return ret;
}

static void uart_v500_pm(struct uart_port *port, unsigned int state,
	unsigned int oldstate)
{
	int retval;
	struct uart_v500_port *uap = (struct uart_v500_port *)port;

	(void)oldstate;
	switch (state) {
	case UART_PM_STATE_ON:
		retval = clk_prepare_enable(uap->clk);
		if (retval)
			dev_err(uap->port.dev, "%s: clk prepare failed\n",
				__func__);
		break;
	case UART_PM_STATE_OFF:
		clk_disable_unprepare(uap->clk);
		break;
	default:
		dev_err(uap->port.dev, "%s: pm state fail state=%u\n",
			__func__, state);
		break;
	}
}

#ifdef CONFIG_PM_SLEEP
static int uart_v500_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct uart_v500_port *uap = platform_get_drvdata(pdev);
	int ret;

	if (!uap) {
		pr_err("%s:uap is NULL\n", __func__);
		return -EINVAL;
	}

	dev_info(uap->port.dev, "%s: +\n", __func__);
	ret = uart_suspend_port(&g_uart_v500, &uap->port);
	dev_info(uap->port.dev, "%s: -\n", __func__);

	return ret;
}

static int uart_v500_resume(struct platform_device *pdev)
{
	struct uart_v500_port *uap = platform_get_drvdata(pdev);
	int ret;

	if (!uap) {
		pr_err("%s:uap is NULL\n", __func__);
		return -EINVAL;
	}

	dev_info(uap->port.dev, "%s: +\n", __func__);
	ret = uart_resume_port(&g_uart_v500, &uap->port);
	dev_info(uap->port.dev, "%s: -\n", __func__);

	return ret;
}
#endif

static int uart_v500_pinctrl(struct platform_device *dev,
	struct uart_v500_port *uap)
{
	int ret = 0;

	uap->pinctrl = devm_pinctrl_get(&dev->dev);
	if (IS_ERR(uap->pinctrl)) {
		ret = PTR_ERR(uap->pinctrl);
		return ret;
	}

	uap->pinctrls_default = pinctrl_lookup_state(uap->pinctrl,
		PINCTRL_STATE_DEFAULT);
	if (IS_ERR(uap->pinctrls_default))
		dev_err(&dev->dev, "could not get default pinstate\n");

	uap->pinctrls_idle = pinctrl_lookup_state(uap->pinctrl,
		PINCTRL_STATE_IDLE);
	if (IS_ERR(uap->pinctrls_idle))
		dev_err(&dev->dev, "could not get sleep pinstate\n");

	return ret;
}

static void uart_v500_disable_msk(struct uart_port *port)
{
	struct uart_v500_port *uap =
		container_of(port, struct uart_v500_port, port);

	uap->int_msk |= UART_V500_INT_CTSM;
	uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
}

static void uart_v500_fifo_err_check(struct uart_v500_port *uap,
	unsigned int *ch, unsigned int *flag)
{
	if (*ch & UART_V500_DR_BE) {
		*ch &= ~(UART_V500_DR_FE | UART_V500_DR_PE);
		uap->port.icount.brk++;
		if (uart_handle_break(&uap->port))
			return;
	} else if (*ch & UART_V500_DR_PE) {
		uap->port.icount.parity++;
	} else if (*ch & UART_V500_DR_FE) {
		uap->port.icount.frame++;
	}

	if (*ch & UART_V500_DR_OE)
		uap->port.icount.overrun++;

	*ch &= uap->port.read_status_mask;

	if (*ch & UART_V500_DR_BE)
		*flag = TTY_BREAK;
	else if (*ch & UART_V500_DR_PE)
		*flag = TTY_PARITY;
	else if (*ch & UART_V500_DR_FE)
		*flag = TTY_FRAME;
}
/*
 * Reads up to 256 characters from the FIFO or until it's empty and
 * inserts them into the TTY layer. Returns the number of characters
 * read from the FIFO.
 */
static int uart_v500_fifo_to_tty(struct uart_v500_port *uap)
{
	unsigned int status;
	unsigned int ch, flag;
	unsigned int max_count = FIFO_MAX_COUNT;
	int fifotaken = 0;

	while (max_count--) {
		status = uart_v500_read(uap, REG_UART_STAT);
		if (status & UART_V500_STAT_RXFE)
			break;

		/* Take chars from the FIFO and update status */
		ch = uart_v500_read(uap, REG_UART_RX_FIFO);
		flag = TTY_NORMAL;
		uap->port.icount.rx++;
		fifotaken++;

		if (unlikely(ch & UART_V500_DR_ERROR))
			uart_v500_fifo_err_check(uap, &ch, &flag);

		if (uart_handle_sysrq_char(&uap->port, ch & UART_TX_WORDS))
			continue;

		uart_insert_char(&uap->port, ch, UART_V500_DR_OE, ch, flag);
	}

	return fifotaken;
}

#ifdef CONFIG_DMA_ENGINE
#define UART_V500_DMA_BUFFER_SIZE PAGE_SIZE
static int uart_v500_sgbuf_init(struct dma_chan *chan,
	struct uart_v500_sgbuf *sg)
{
	dma_addr_t dma_addr;

	sg->buf = dma_alloc_coherent(chan->device->dev,
		UART_V500_DMA_BUFFER_SIZE, &dma_addr, GFP_KERNEL);
	if (!sg->buf)
		return -ENOMEM;

	sg_init_table(&sg->sg, 1); /* 1: Number of entries in table */
	sg_set_page(&sg->sg, phys_to_page(dma_addr), UART_V500_DMA_BUFFER_SIZE,
		offset_in_page(dma_addr));
	sg_dma_address(&sg->sg) = dma_addr;
	sg_dma_len(&sg->sg) = UART_V500_DMA_BUFFER_SIZE;

	return 0;
}

static void uart_v500_sgbuf_free(struct dma_chan *chan,
	struct uart_v500_sgbuf *sg)
{
	if (sg->buf)
		dma_free_coherent(chan->device->dev, UART_V500_DMA_BUFFER_SIZE,
			sg->buf, sg_dma_address(&sg->sg));
}

static void uart_v500_dma_tx_init(struct uart_v500_port *uap,
	struct device *dev, struct uart_v500_dma_data *plat)
{
	struct dma_chan *request_chan = NULL;
	dma_cap_mask_t mask;
	struct dma_slave_config tx_conf = {
		.dst_addr = uap->port.mapbase + uart_v500_reg_to_offset(uap,
			REG_UART_TX_FIFO),
		.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE,
		.direction = DMA_MEM_TO_DEV,
		.dst_maxburst = (uap->fifosize >> 2),
		.device_fc = false,
	};

	request_chan = dma_request_slave_channel_reason(dev, "tx");
	if (IS_ERR(request_chan)) {
		if (PTR_ERR(request_chan) == -EPROBE_DEFER) {
			uap->is_dma_probed = false;
			return;
		}
		/* check platform data */
		if (!plat || !plat->dma_filter) {
			dev_info(uap->port.dev, "No DMA platform data\n");
			return;
		}
		/* Try to acquire a generic DMA engine slave TX channel */
		dma_cap_zero(mask);
		dma_cap_set(DMA_SLAVE, mask);

		request_chan = dma_request_channel(mask, plat->dma_filter,
			plat->dma_tx_param);
		if (!request_chan) {
			dev_err(uap->port.dev, "No Tx DMA channel!\n");
			return;
		}
	}

	dmaengine_slave_config(request_chan, &tx_conf);
	uap->dmatx.chan = request_chan;
	dev_info(uap->port.dev, "DMA channel Tx %s\n",
		dma_chan_name(uap->dmatx.chan));
}

static void uart_v500_dma_rx_init(struct uart_v500_port *uap,
	struct device *dev, struct uart_v500_dma_data *plat)
{
	struct dma_chan *request_chan = NULL;
	struct dma_slave_caps caps;
	dma_cap_mask_t mask;
	struct dma_slave_config rx_conf = {
		.src_addr = uap->port.mapbase +
			uart_v500_reg_to_offset(uap, REG_UART_RX_FIFO),
		.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE,
		.direction = DMA_DEV_TO_MEM,
		.src_maxburst = ((uap->fifosize >> 2) - 1),
		.device_fc = false,
	};

	request_chan = dma_request_slave_channel(dev, "rx");
	if (!request_chan && plat && plat->dma_rx_param) {
		request_chan = dma_request_channel(mask, plat->dma_filter,
			plat->dma_rx_param);
		if (!request_chan) {
			dev_err(uap->port.dev, "No Rx DMA channel!\n");
			return;
		}
	}

	if (request_chan) {
		/*
		 * Some DMA controllers provide information on their
		 * capabilities. If the controller does, check for suitable
		 * residue processing otherwise assime all is well.
		 */
		if (dma_get_slave_caps(request_chan, &caps) == 0) {
			if (caps.residue_granularity ==
				DMA_RESIDUE_GRANULARITY_DESCRIPTOR) {
				dma_release_channel(request_chan);
				dev_info(uap->port.dev,
					"no residue process disable rx dma\n");
				return;
			}
		}
		dmaengine_slave_config(request_chan, &rx_conf);
		uap->dmarx.chan = request_chan;

		dev_info(uap->port.dev, "DMA channel Rx %s\n",
			 dma_chan_name(uap->dmarx.chan));
	}
}

static void uart_v500_dma_probe(struct uart_v500_port *uap)
{
	/* DMA is the sole user of the platform data right now */
	struct uart_v500_dma_data *plat = dev_get_platdata(uap->port.dev);
	struct device *dev = uap->port.dev;

	uap->is_dma_probed = true;
	uart_v500_dma_tx_init(uap, dev, plat);
	uart_v500_dma_rx_init(uap, dev, plat);
}

static void uart_v500_dma_remove(struct uart_v500_port *uap)
{
	if (uap->dmatx.chan)
		dma_release_channel(uap->dmatx.chan);
	if (uap->dmarx.chan)
		dma_release_channel(uap->dmarx.chan);
}

/*
 * The current DMA TX buffer has been sent.
 * Try to queue up another DMA buffer.
 */
static void uart_v500_dma_tx_callback(void *data)
{
	struct uart_v500_port *uap = data;
	struct uart_v500_dmatx_data *dmatx = &uap->dmatx;
	unsigned long flags;
	unsigned int dma_conctrl;

	spin_lock_irqsave(&uap->port.lock, flags);
	if (uap->dmatx.queued)
		/* 1: Number of entries in table */
		dma_unmap_sg(dmatx->chan->device->dev, &dmatx->sg, 1,
			DMA_TO_DEVICE);

	dma_conctrl = uap->dma_conctrl;
	uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);
	uart_v500_write(UART_V500_TX_DMA_DIS, uap, REG_UART_CONFIG);
	uap->dma_conctrl = uart_v500_read(uap, REG_UART_CONFIG);
	uap->dma_conctrl |= uap->dma_conctrl << UART_CONFIG_BIT_MASK;

	if (!(dma_conctrl & UART_V500_TX_DMA_EN) ||
		uart_tx_stopped(&uap->port) ||
		uart_circ_empty(&uap->port.state->xmit)) {
		uap->dmatx.queued = false;
		spin_unlock_irqrestore(&uap->port.lock, flags);
		return;
	}

	if (uart_v500_dma_tx_refill(uap) <= 0)
		uart_v500_start_tx_pio(uap);

	spin_unlock_irqrestore(&uap->port.lock, flags);
}

static void uart_v500_dma_tx_xmit_fill(struct uart_v500_port *uap,
	unsigned int *count, struct circ_buf *xmit,
	struct uart_v500_dmatx_data *dma_tx)
{
	size_t first = UART_XMIT_SIZE - xmit->tail;
	size_t second;

	*count = uart_circ_chars_pending(xmit);
	if (*count < (uap->fifosize >> 1)) {
		uap->dmatx.queued = false;
		return;
	}

	/* Else proceed to copy the TX chars to the DMA buffer and fire DMA */
	if (*count > UART_V500_DMA_BUFFER_SIZE)
		*count = UART_V500_DMA_BUFFER_SIZE;

	if (xmit->tail < xmit->head) {
		if (memcpy_s(&dma_tx->buffer[0], UART_V500_DMA_BUFFER_SIZE,
			&xmit->buf[xmit->tail], *count) != EOK)
			dev_err(uap->port.dev, "TX DMA memcpy_s once failed\n");
	} else {
		if (first > *count)
			first = *count;
		second = *count - first;

		if (memcpy_s(&dma_tx->buffer[0], UART_V500_DMA_BUFFER_SIZE,
			&xmit->buf[xmit->tail], first) != EOK)
			dev_err(uap->port.dev, "dma_tx memcpy_s first fail\n");
		if (second) {
			if (memcpy_s(&dma_tx->buffer[first],
				UART_V500_DMA_BUFFER_SIZE - first,
				&xmit->buf[0], second) != EOK)
			dev_err(uap->port.dev, "dma_tx memcpy_s second fail\n");
		}
	}

	dma_tx->sg.length = *count;
}

/*
 * Try to refill the TX DMA buffer.
 * Locking: called with port lock held and IRQs disabled.
 * Returns:
 *   1 if we queued up a TX DMA buffer.
 *   0 if we didn't want to handle this by DMA
 *  <0 on error
 */
static int uart_v500_dma_tx_refill(struct uart_v500_port *uap)
{
	struct uart_v500_dmatx_data *dma_tx = &uap->dmatx;
	struct dma_chan *tx_chan = dma_tx->chan;
	struct dma_device *dma_dev = tx_chan->device;
	struct dma_async_tx_descriptor *dma_desc = NULL;
	struct circ_buf *xmit = &uap->port.state->xmit;
	unsigned int count;

	/*
	 * Try to avoid the overhead involved in using DMA if the transaction
	 * fits in the first half of the FIFO, by using the standard interrupt
	 * handling. This ensures that we issue a uart_write_wakeup() at the
	 * appropriate time.
	 */
	uart_v500_dma_tx_xmit_fill(uap, &count, xmit, dma_tx);
	if (dma_map_sg(dma_dev->dev, &dma_tx->sg, 1, DMA_TO_DEVICE) != 1) {
		uap->dmatx.queued = false;
		dev_dbg(uap->port.dev, "unable to map TX DMA\n");
		return -EBUSY;
	}

	/* 1: Number of entries in table */
	dma_desc = dmaengine_prep_slave_sg(tx_chan, &dma_tx->sg, 1,
		DMA_MEM_TO_DEV, DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!dma_desc) {
		dma_unmap_sg(dma_dev->dev, &dma_tx->sg, 1, DMA_TO_DEVICE);
		uap->dmatx.queued = false;
		/*
		 * If DMA can not be used right now, we complete this
		 * transaction via IRQ and let the TTY layer retry.
		 */
		dev_dbg(uap->port.dev, "TX DMA busy\n");
		return -EBUSY;
	}

	/* Some data to go along to the callback */
	dma_desc->callback = uart_v500_dma_tx_callback;
	dma_desc->callback_param = uap;

	/* All errors should happen at prepare time */
	dmaengine_submit(dma_desc);

	/* Fire the DMA transaction */
	dma_dev->device_issue_pending(tx_chan);

	uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);
	uart_v500_write(UART_V500_TX_DMA_EN, uap, REG_UART_CONFIG);
	uap->dma_conctrl = uart_v500_read(uap, REG_UART_CONFIG);
	uap->dma_conctrl |= uap->dma_conctrl << UART_CONFIG_BIT_MASK;
	uap->dmatx.queued = true;

	/*
	 * Now we know that DMA will fire, so advance the ring buffer
	 * with the stuff we just dispatched.
	 */
	xmit->tail = (xmit->tail + count) & (UART_XMIT_SIZE - 1);
	uap->port.icount.tx += count;

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&uap->port);

	return 1;
}

/*
 * We received a transmit interrupt without a pending X-char but with
 * pending characters.
 * Locking: called with port lock held and IRQs disabled.
 * Returns:
 *   false if we want to use PIO to transmit
 *   true if we queued a DMA buffer
 */
static bool uart_v500_dma_tx_irq(struct uart_v500_port *uap)
{
	if (!uap->using_tx_dma)
		return false;

	/*
	 * If we already have a TX buffer queued, but received a
	 * TX interrupt, it will be because we've just sent an X-char.
	 * Ensure the TX DMA is enabled and the TX IRQ is disabled.
	 */
	if (uap->dmatx.queued) {
		uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);
		uart_v500_write(UART_V500_TX_DMA_EN, uap, REG_UART_CONFIG);
		uap->dma_conctrl = uart_v500_read(uap, REG_UART_CONFIG);
		uap->dma_conctrl |= uap->dma_conctrl << UART_CONFIG_BIT_MASK;
		uap->int_msk |= UART_V500_INT_TXM;
		uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
		return true;
	}

	/*
	 * We don't have a TX buffer queued, so try to queue one.
	 * If we successfully queued a buffer, mask the TX IRQ.
	 */
	if (uart_v500_dma_tx_refill(uap) > 0) {
		uap->int_msk |= UART_V500_INT_TXM;
		uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
		return true;
	}
	return false;
}

/*
 * Stop the DMA transmit (eg, due to received XOFF).
 * Locking: called with port lock held and IRQs disabled.
 */
static void uart_v500_dma_tx_stop(struct uart_v500_port *uap)
{
	if (uap->dmatx.queued) {
		uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);
		uart_v500_write(UART_V500_TX_DMA_DIS, uap, REG_UART_CONFIG);
		uap->dma_conctrl = uart_v500_read(uap, REG_UART_CONFIG);
		uap->dma_conctrl |= uap->dma_conctrl << UART_CONFIG_BIT_MASK;
	}
}

/*
 * Try to start a DMA transmit, or in the case of an XON/OFF
 * character queued for send, try to get that character out ASAP.
 * Locking: called with port lock held and IRQs disabled.
 * Returns:
 *   false if we want the TX IRQ to be enabled
 *   true if we have a buffer queued
 */
static bool uart_v500_dma_tx_start(struct uart_v500_port *uap)
{
	if (!uap->using_tx_dma)
		return false;

	if (!uap->port.x_char) {
		/* no X-char, try to push chars out in DMA mode */
		bool ret = true;

		if (!uap->dmatx.queued) {
			if (uart_v500_dma_tx_refill(uap) > 0) {
				uap->int_msk |= UART_V500_INT_TXM;
				uart_v500_write(uap->int_msk, uap,
					REG_UART_INT_MSK);
			} else {
				ret = false;
			}
		} else if (!(uap->dma_conctrl & UART_V500_TX_DMA_EN)) {
			uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);
			uart_v500_write(UART_V500_TX_DMA_EN, uap,
				REG_UART_CONFIG);
			uap->dma_conctrl = uart_v500_read(uap, REG_UART_CONFIG);
			uap->dma_conctrl |= uap->dma_conctrl <<
				UART_CONFIG_BIT_MASK;
		}
		return ret;
	}

	uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);
	uart_v500_write(UART_V500_TX_DMA_DIS, uap, REG_UART_CONFIG);

	if (uart_v500_read(uap, REG_UART_STAT) & UART_V500_STAT_TXFF)
		return false;

	uart_v500_write(uap->port.x_char, uap, REG_UART_TX_FIFO);
	uap->port.icount.tx++;
	uap->port.x_char = 0;

	/* Success - restore the DMA state */
	uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);

	return true;
}

/*
 * Flush the transmit buffer.
 * Locking: called with port lock held and IRQs disabled.
 */
static void uart_v500_dma_flush_buffer(struct uart_port *port)
__releases(&uap->port.lock)
__acquires(&uap->port.lock)
{
	struct uart_v500_port *uap = container_of(port,
		struct uart_v500_port, port);

	if (!uap->using_tx_dma)
		return;

	dmaengine_terminate_async(uap->dmatx.chan);

	if (uap->dmatx.queued) {
		/* 1: Number of entries in table */
		dma_unmap_sg(uap->dmatx.chan->device->dev, &uap->dmatx.sg, 1,
			DMA_TO_DEVICE);
		uap->dmatx.queued = false;
		uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);
		uart_v500_write(UART_V500_TX_DMA_DIS, uap, REG_UART_CONFIG);
		uap->dma_conctrl = uart_v500_read(uap, REG_UART_CONFIG);
		uap->dma_conctrl |= uap->dma_conctrl << UART_CONFIG_BIT_MASK;
	}
}

static int uart_v500_dma_rx_trigger_dma(struct uart_v500_port *uap)
{
	struct dma_chan *rx_chan = uap->dmarx.chan;
	struct uart_v500_dmarx_data *dma_rx = &uap->dmarx;
	struct dma_async_tx_descriptor *dma_desc = NULL;
	struct uart_v500_sgbuf *sgbuf = NULL;

	if (!rx_chan)
		return -EIO;

	/* Start the Rx DMA work */
	sgbuf = uap->dmarx.use_buf_b ? &uap->dmarx.sgbuf_b :
		&uap->dmarx.sgbuf_a;
	/* 1: Number of entries in table */
	dma_desc = dmaengine_prep_slave_sg(rx_chan, &sgbuf->sg, 1,
		DMA_DEV_TO_MEM, DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	/*
	 * If the DMA engine is busy and can not prepare a
	 * channel, no big deal, the driver will fall back
	 * to interrupt mode as a result of this error code.
	 */
	if (!dma_desc) {
		uap->dmarx.running = false;
		dmaengine_terminate_all(rx_chan);
		return -EBUSY;
	}

	/* Some data to go along to the callback */
	dma_desc->callback = uart_v500_dma_rx_callback;
	dma_desc->callback_param = uap;
	dma_rx->dma_cookie = dmaengine_submit(dma_desc);
	dma_async_issue_pending(rx_chan);

	uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);
	uart_v500_write(UART_V500_RX_DMA_EN, uap, REG_UART_CONFIG);
	uap->dma_conctrl = uart_v500_read(uap, REG_UART_CONFIG);
	uap->dma_conctrl |= uap->dma_conctrl << UART_CONFIG_BIT_MASK;
	uap->dmarx.running = true;

	uap->int_msk |= UART_V500_INT_RXM;
	uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);

	return 0;
}

/*
 * This is called when either the DMA job is complete, or
 * the FIFO timeout interrupt occurred. This must be called
 * with the port spinlock uap->port.lock held.
 */
static void uart_v500_dma_rx_chars(struct uart_v500_port *uap,
	unsigned int pending, bool use_buf_b, bool readfifo)
{
	struct tty_port *port = &uap->port.state->port;
	struct uart_v500_sgbuf *sgbuf = use_buf_b ?
		&uap->dmarx.sgbuf_b : &uap->dmarx.sgbuf_a;
	unsigned int dma_count = 0;
	unsigned int int_clr;
	unsigned int fifotaken = 0;
	unsigned int dmataken = 0;

	/* Pick the remain data from the DMA */
	if (pending) {
		/*
		 * First take all chars in the DMA pipe, then look in the FIFO.
		 * Note that tty_insert_flip_buf() tries to take as many chars
		 * as it can.
		 */
		dma_count = tty_insert_flip_string(port, sgbuf->buf + dmataken,
				pending);

		uap->port.icount.rx += dma_count;
		if (dma_count < pending)
			dev_warn(uap->port.dev,
				 "can't insert all characters (TTY is full)\n");
	}

	/*
	 * Only continue with trying to read the FIFO if all DMA chars have
	 * been taken first.
	 */
	if (dma_count == pending && readfifo) {
		/* Clear any error flags */
		int_clr = (UART_V500_INT_OES | UART_V500_INT_BES |
			UART_V500_INT_PES | UART_V500_INT_FES);
		uart_v500_write(int_clr, uap, REG_UART_INT_CLR);

		/*
		 * If we read all the DMA'd characters, and we had an
		 * incomplete buffer, that could be due to an rx error, or
		 * maybe we just timed out. Read any pending chars and check
		 * the error status.
		 *
		 * Error conditions will only occur in the FIFO, these will
		 * trigger an immediate interrupt and stop the DMA job, so we
		 * will always find the error in the FIFO, never in the DMA
		 * buffer.
		 */
		fifotaken = uart_v500_fifo_to_tty(uap);
	}

	spin_unlock(&uap->port.lock);
	dev_vdbg(uap->port.dev,
		 "Took %u chars from DMA buffer and %d chars from the FIFO\n",
		 dma_count, fifotaken);
	tty_flip_buffer_push(port);
	spin_lock(&uap->port.lock);
}

static void uart_v500_dma_rx_irq(struct uart_v500_port *uap)
{
	struct uart_v500_dmarx_data *dmarx = &uap->dmarx;
	struct dma_chan *rxchan = dmarx->chan;
	struct uart_v500_sgbuf *sgbuf = dmarx->use_buf_b ?
		&dmarx->sgbuf_b : &dmarx->sgbuf_a;
	size_t pending;
	struct dma_tx_state state;
	enum dma_status dmastat;

	/* Disable RX DMA - incoming data will wait in the FIFO */
	uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);
	uart_v500_write(UART_V500_RX_DMA_DIS, uap, REG_UART_CONFIG);
	uap->dma_conctrl = uart_v500_read(uap, REG_UART_CONFIG);
	uap->dma_conctrl |= uap->dma_conctrl << UART_CONFIG_BIT_MASK;

	/*
	 * Pause the transfer so we can trust the current counter,
	 * do this before we pause the uart block, else we may
	 * overflow the FIFO.
	 */
	if (dmaengine_pause(rxchan))
		dev_err(uap->port.dev, "unable to pause DMA transfer\n");
	dmastat = rxchan->device->device_tx_status(rxchan,
		dmarx->dma_cookie, &state);
	if (dmastat != DMA_PAUSED)
		dev_err(uap->port.dev, "not pause DMA transfer dmastat = %d\n",
			dmastat);

	if (dmastat == DMA_COMPLETE || state.residue == 0) {
		uap->port.icount.buf_overrun++;
		uap->int_msk |= UART_V500_INT_RTM;
		uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
		return;
	}

	uap->dmarx.running = false;

	pending = sgbuf->sg.length - state.residue;
	BUG_ON(pending > UART_V500_DMA_BUFFER_SIZE);
	/* Then we terminate the transfer - we now know our residue */
	dmaengine_terminate_all(rxchan);

	/*
	 * This will take the chars we have so far and insert
	 * into the framework.
	 */
	uart_v500_dma_rx_chars(uap, pending, dmarx->use_buf_b, true);

	/* Switch buffer & re-trigger DMA job */
	dmarx->use_buf_b = !dmarx->use_buf_b;
	if (uart_v500_dma_rx_trigger_dma(uap)) {
		dev_dbg(uap->port.dev, "could not retrigger RX DMA job "
			"fall back to interrupt mode\n");
		uap->int_msk &= (~UART_V500_INT_RXM);
		uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
	}
}

static void uart_v500_dma_rx_callback(void *data)
{
	struct uart_v500_port *uap = data;
	struct uart_v500_dmarx_data *dmarx = &uap->dmarx;
	struct dma_chan *rxchan = dmarx->chan;
	bool lastbuf = dmarx->use_buf_b;
	struct uart_v500_sgbuf *sgbuf = dmarx->use_buf_b ?
		&dmarx->sgbuf_b : &dmarx->sgbuf_a;
	size_t pending;
	struct dma_tx_state state;
	int ret;
	/*
	 * This completion interrupt occurs typically when the
	 * RX buffer is totally stuffed but no timeout has yet
	 * occurred. When that happens, we just want the RX
	 * routine to flush out the secondary DMA buffer while
	 * we immediately trigger the next DMA job.
	 */
	spin_lock_irq(&uap->port.lock);
	if (uap->uart_rx_dma_disabled) {
		spin_unlock_irq(&uap->port.lock);
		return;
	}
	uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);
	uart_v500_write(UART_V500_RX_DMA_DIS, uap, REG_UART_CONFIG);
	uap->dma_conctrl = uart_v500_read(uap, REG_UART_CONFIG);
	uap->dma_conctrl |= uap->dma_conctrl << UART_CONFIG_BIT_MASK;

	/*
	 * Rx data can be taken by the UART interrupts during
	 * the DMA irq handler. So we check the residue here.
	 */
	rxchan->device->device_tx_status(rxchan, dmarx->dma_cookie, &state);
	pending = sgbuf->sg.length - state.residue;
	BUG_ON(pending > UART_V500_DMA_BUFFER_SIZE);
	/* Then we terminate the transfer - we now know our residue */
	dmaengine_terminate_all(rxchan);

	uap->dmarx.running = false;
	dmarx->use_buf_b = !lastbuf;
	ret = uart_v500_dma_rx_trigger_dma(uap);

	uart_v500_dma_rx_chars(uap, pending, lastbuf, false);
	spin_unlock_irq(&uap->port.lock);
	/*
	 * Do this check after we picked the DMA chars so we don't
	 * get some IRQ immediately from RX.
	 */
	if (ret) {
		dev_dbg(uap->port.dev, "could not retrigger RX DMA job "
			"fall back to interrupt mode\n");
		uap->int_msk &= ~(UART_V500_INT_RXM | UART_V500_INT_RTM);
		uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
	} else {
		uap->int_msk |= UART_V500_INT_RXM;
		uap->int_msk &= (~UART_V500_INT_RTM);
		uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
	}
}

/*
 * Stop accepting received characters, when we're shutting down or
 * suspending this port.
 * Locking: called with port lock held and IRQs disabled.
 */
static void uart_v500_dma_rx_stop(struct uart_v500_port *uap)
{
	/* Just disable the DMA enable */
	uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);
	uart_v500_write(UART_V500_RX_DMA_DIS, uap, REG_UART_CONFIG);
	uap->dma_conctrl = uart_v500_read(uap, REG_UART_CONFIG);
	uap->dma_conctrl |= uap->dma_conctrl << UART_CONFIG_BIT_MASK;
}

static void uart_v500_dma_startup(struct uart_v500_port *uap)
{
	int val;

	if (!uap->is_dma_probed)
		uart_v500_dma_probe(uap);

	if (!uap->dmatx.chan)
		return;

	uap->dmatx.buffer = kmalloc(UART_V500_DMA_BUFFER_SIZE,
		GFP_KERNEL | __GFP_DMA);
	if (!uap->dmatx.buffer) {
		dev_err(uap->port.dev, "no memory for DMA TX buffer\n");
		uap->port.fifosize = uap->fifosize;
		return;
	}

	sg_init_one(&uap->dmatx.sg, uap->dmatx.buffer,
		UART_V500_DMA_BUFFER_SIZE);

	/* The DMA buffer is now the FIFO the tty subsystem can use */
	uap->port.fifosize = UART_V500_DMA_BUFFER_SIZE;
	uap->using_tx_dma = true;

	if (!uap->dmarx.chan)
		goto skip_rx;

	/* Allocate and map DMA RX buffers */
	if (!uap->rx_global_sg_buf_alloced) {
		val = uart_v500_sgbuf_init(uap->dmarx.chan,
			&uap->dmarx.sgbuf_a);
		if (val) {
			dev_err(uap->port.dev, "DMA rx buffer A init fail %d\n",
				val);
			goto skip_rx;
		}

		val = uart_v500_sgbuf_init(uap->dmarx.chan,
			&uap->dmarx.sgbuf_b);
		if (val) {
			dev_err(uap->port.dev, "DMA rx buffer B init fail %d\n",
				val);
			uart_v500_sgbuf_free(uap->dmarx.chan,
				&uap->dmarx.sgbuf_a);
			goto skip_rx;
		}
		if (uap->rx_use_global_sg_buf)
			uap->rx_global_sg_buf_alloced = true;
	}

	uap->using_rx_dma = true;
	uap->uart_rx_dma_disabled = false;

skip_rx:
	uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);

	if (uap->using_rx_dma) {
		if (uart_v500_dma_rx_trigger_dma(uap))
			dev_dbg(uap->port.dev, "could not trigger initial "
				"RX DMA job, fall back to interrupt mode\n");
	}
}

static void uart_v500_dma_shutdown(struct uart_v500_port *uap)
{
	unsigned int uart_cfg;

	if (!(uap->using_tx_dma || uap->using_rx_dma))
		return;

	uart_v500_write(UART_V500_CFG_CTS_DIS, uap, REG_UART_CONFIG);
	uart_cfg = uart_v500_read(uap, REG_UART_CONFIG);
	uart_cfg |= uart_cfg << UART_CONFIG_BIT_MASK;

	/* Disable RX and TX DMA */
	while (uart_v500_read(uap, REG_UART_STAT) & uap->vendor->stat_busy)
		cpu_relax();

	spin_lock_irq(&uap->port.lock);
	uap->dma_conctrl |= uart_cfg;
	uart_v500_write(uap->dma_conctrl, uap, REG_UART_CONFIG);
	uart_v500_write((UART_V500_TX_DMA_DIS | UART_V500_RX_DMA_DIS), uap,
		REG_UART_CONFIG);
	uap->dma_conctrl = uart_v500_read(uap, REG_UART_CONFIG);
	uap->dma_conctrl |= uap->dma_conctrl << UART_CONFIG_BIT_MASK;
	uap->uart_rx_dma_disabled = true;
	spin_unlock_irq(&uap->port.lock);

	if (uap->using_tx_dma) {
		/* this should already be done by uart_v500_dma_flush_buffer */
		dmaengine_terminate_all(uap->dmatx.chan);
		if (uap->dmatx.queued) {
			dma_unmap_sg(uap->dmatx.chan->device->dev,
				&uap->dmatx.sg, 1, DMA_TO_DEVICE);
			uap->dmatx.queued = false;
		}
		kfree(uap->dmatx.buffer);
		uap->using_tx_dma = false;
	}

	if (uap->using_rx_dma) {
		dmaengine_terminate_all(uap->dmarx.chan);
		/* Clean up the RX DMA */
		if (!uap->rx_global_sg_buf_alloced) {
			uart_v500_sgbuf_free(uap->dmarx.chan,
				&uap->dmarx.sgbuf_a);
			uart_v500_sgbuf_free(uap->dmarx.chan,
				&uap->dmarx.sgbuf_b);
		}
		uap->using_rx_dma = false;
	}
}

static inline bool uart_v500_dma_rx_available(struct uart_v500_port *uap)
{
	return uap->using_rx_dma;
}

static inline bool uart_v500_dma_rx_running(struct uart_v500_port *uap)
{
	return uap->using_rx_dma && uap->dmarx.running;
}

#else
/* Blank functions if the DMA engine is not available */
static inline void uart_v500_dma_probe(struct uart_v500_port *uap)
{
}

static inline void uart_v500_dma_remove(struct uart_v500_port *uap)
{
}

static inline void uart_v500_dma_startup(struct uart_v500_port *uap)
{
}

static inline void uart_v500_dma_shutdown(struct uart_v500_port *uap)
{
}

static inline bool uart_v500_dma_tx_irq(struct uart_v500_port *uap)
{
	return false;
}

static inline void uart_v500_dma_tx_stop(struct uart_v500_port *uap)
{
}

static bool uart_v500_dma_tx_start(struct uart_v500_port *uap)
{
	return false;
}

static inline void uart_v500_dma_rx_irq(struct uart_v500_port *uap)
{
}

static inline void uart_v500_dma_rx_stop(struct uart_v500_port *uap)
{
}

static inline int uart_v500_dma_rx_trigger_dma(struct uart_v500_port *uap)
{
	return -EIO;
}

static inline bool uart_v500_dma_rx_available(struct uart_v500_port *uap)
{
	return false;
}

static inline bool uart_v500_dma_rx_running(struct uart_v500_port *uap)
{
	return false;
}

static inline void uart_v500_dma_flush_buffer(struct uart_v500_port *uap)
{
	return;
}
#endif

static void uart_v500_stop_tx(struct uart_port *port)
{
	struct uart_v500_port *uap =
		container_of(port, struct uart_v500_port, port);

	uap->int_msk |= UART_V500_INT_TXM;
	uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
	uart_v500_dma_tx_stop(uap);
}

static void uart_v500_start_tx_pio(struct uart_v500_port *uap)
{
	if (uart_v500_tx_chars(uap, false)) {
		uap->int_msk &= (~UART_V500_INT_TXM);
		uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
	}
}

static void uart_v500_start_tx(struct uart_port *port)
{
	struct uart_v500_port *uap =
		container_of(port, struct uart_v500_port, port);

	if (!uart_v500_dma_tx_start(uap))
		uart_v500_start_tx_pio(uap);
}

static void uart_v500_stop_rx(struct uart_port *port)
{
	struct uart_v500_port *uap =
		container_of(port, struct uart_v500_port, port);

	uap->int_msk |= UART_V500_INT_MASK;
	uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);

	uart_v500_dma_rx_stop(uap);
}

static void uart_v500_enable_msk(struct uart_port *port)
{
	struct uart_v500_port *uap =
		container_of(port, struct uart_v500_port, port);

	uap->int_msk &= (~UART_V500_INT_CTSM);
	uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
}

static void uart_v500_rx_chars(struct uart_v500_port *uap)
__releases(&uap->port.lock)
__acquires(&uap->port.lock)
{
	uart_v500_fifo_to_tty(uap);

	spin_unlock(&uap->port.lock);
	tty_flip_buffer_push(&uap->port.state->port);
	/*
	 * If we were temporarily out of DMA mode for a while,
	 * attempt to switch back to DMA mode again.
	 */
	if (uart_v500_dma_rx_available(uap)) {
		if (uart_v500_dma_rx_trigger_dma(uap)) {
			dev_dbg(uap->port.dev, "could not trigger RX DMA job "
				"fall back to interrupt mode again\n");
			uap->int_msk &= ~(UART_V500_INT_RXM |
				UART_V500_INT_RTM);
			uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
		} else {
			uap->int_msk |= UART_V500_INT_RXM;
			uap->int_msk &= (~UART_V500_INT_RTM);
			uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
		}
	}
	spin_lock(&uap->port.lock);
}

static void uart_v500_flow_ctrl_status(struct uart_v500_port *uap)
{
	unsigned int status, delta;

	status = (~uart_v500_read(uap, REG_UART_STAT)) & UART_V500_STAT_CTS;

	delta = status ^ uap->old_status;
	uap->old_status = status;

	if (!delta)
		return;

	if (delta & uap->vendor->stat_cts)
		uart_handle_cts_change(&uap->port, status &
			uap->vendor->stat_cts);

	wake_up_interruptible(&uap->port.state->port.delta_msr_wait);
}
static bool uart_v500_tx_char(struct uart_v500_port *uap, unsigned char c,
	bool from_irq)
{
	if (unlikely(!from_irq) && uart_v500_read(uap, REG_UART_STAT) &
		UART_V500_STAT_TXFF)
		return false; /* unable to transmit character */

	uart_v500_write(c, uap, REG_UART_TX_FIFO);
	uap->port.icount.tx++;

	return true;
}

/* Returns true if tx interrupts have to be (kept) enabled */
static bool uart_v500_tx_chars(struct uart_v500_port *uap, bool from_irq)
{
	struct circ_buf *xmit = &uap->port.state->xmit;
	int count = uap->fifosize >> 1;

	if (uap->port.x_char) {
		if (!uart_v500_tx_char(uap, uap->port.x_char, from_irq))
			return true;
		uap->port.x_char = 0;
		--count;
	}
	if (uart_circ_empty(xmit) || uart_tx_stopped(&uap->port)) {
		uart_v500_stop_tx(&uap->port);
		return false;
	}

	/* If we are using DMA mode, try to send some characters */
	if (uart_v500_dma_tx_irq(uap))
		return true;

	do {
		if (likely(from_irq) && count-- == 0)
			break;

		if (!uart_v500_tx_char(uap, xmit->buf[xmit->tail], from_irq))
			break;

		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
	} while (!uart_circ_empty(xmit));

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&uap->port);

	if (uart_circ_empty(xmit)) {
		uart_v500_stop_tx(&uap->port);
		return false;
	}
	return true;
}

static void uart_v500_rx_irq_handler(struct uart_v500_port *uap)
{
	if (uart_v500_dma_rx_running(uap))
		uart_v500_dma_rx_irq(uap);
	else
		uart_v500_rx_chars(uap);
}

static irqreturn_t uart_v500_int_handler(int irq, void *dev_id)
{
	struct uart_v500_port *uap = dev_id;
	unsigned long flags;
	unsigned int status;
	unsigned int pass_counter = UART_ISR_PASS_LIMIT;
	int handled = 0;
	unsigned int int_clr;

	spin_lock_irqsave(&uap->port.lock, flags);
	status = uart_v500_read(uap, REG_UART_INT_STAT);
	int_clr = status;
	if (status) {
		do {
			int_clr &= ~(UART_V500_INT_TXS | UART_V500_INT_RTS |
				UART_V500_INT_RXS);
			uart_v500_write(int_clr, uap, REG_UART_INT_CLR);

			if (status & (UART_V500_INT_RTS | UART_V500_INT_RXS))
				uart_v500_rx_irq_handler(uap);

			if (status & UART_V500_INT_CTSS)
				uart_v500_flow_ctrl_status(uap);

			if (status & UART_V500_INT_TXS)
				uart_v500_tx_chars(uap, true);

			if (pass_counter-- == 0)
				break;

			status = uart_v500_read(uap, REG_UART_INT_STAT);
		} while (status != 0);
		handled = 1;
	}

	spin_unlock_irqrestore(&uap->port.lock, flags);

	return IRQ_RETVAL(handled);
}

static unsigned int uart_v500_tx_empty(struct uart_port *port)
{
	struct uart_v500_port *uap =
		container_of(port, struct uart_v500_port, port);
	unsigned int status = uart_v500_read(uap, REG_UART_STAT);

	return (status & UART_V500_STAT_TXFE) ? 1 : 0;
}

static unsigned int uart_v500_get_mctrl(struct uart_port *port)
{
	struct uart_v500_port *uap =
		container_of(port, struct uart_v500_port, port);
	unsigned int result = 0;
	unsigned int status;

	status = ~uart_v500_read(uap, REG_UART_STAT);
	if (status & uap->vendor->stat_cts)
		result |= TIOCM_CTS;

	return result;
}

static void uart_v500_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct uart_v500_port *uap =
		container_of(port, struct uart_v500_port, port);
	unsigned int uart_cfg;

	if (mctrl & TIOCM_LOOP)
		uart_cfg = UART_V500_CFG_LOOP_EN;
	else
		uart_cfg = UART_V500_CFG_LOOP_DIS;

	uart_v500_write(uart_cfg, uap, REG_UART_CONFIG);

	if (uap->auto_rts) {
		/* We need to disable auto-RTS if we want to turn RTS off */
		if (mctrl & TIOCM_RTS)
			uart_cfg = UART_V500_CFG_RTS_EN;
		else
			uart_cfg = UART_V500_CFG_RTS_DIS;

		uart_v500_write(uart_cfg, uap, REG_UART_CONFIG);
	}
}

static void uart_v500_break_ctl(struct uart_port *port, int break_state)
{
	struct uart_v500_port *uap =
		container_of(port, struct uart_v500_port, port);
	unsigned long flags;
	unsigned int brk_cfg;

	spin_lock_irqsave(&uap->port.lock, flags);
	if (break_state == -1)
		brk_cfg = UART_V500_CFG_BRK_EN;
	else
		brk_cfg = UART_V500_CFG_BRK_DIS;
	uart_v500_write(brk_cfg, uap, REG_UART_CONFIG);
	spin_unlock_irqrestore(&uap->port.lock, flags);
}

static int uart_v500_hwinit(struct uart_port *port)
{
	struct uart_v500_port *uap = container_of(port,
		struct uart_v500_port, port);
	int retval;

	/* Optionaly enable pins to be muxed in and configured */
	pinctrl_pm_select_default_state(port->dev);
	retval = clk_prepare_enable(uap->clk);
	if (retval)
		return retval;

	uap->port.uartclk = clk_get_rate(uap->clk);

	/* Clear pending error and receive interrupts */
	uart_v500_write((~(UART_V500_INT_STAUS) << UART_CONFIG_BIT_MASK) &
		UART_V500_CLR_INT_ALL, uap, REG_UART_INT_MSK);

	/*
	 * Save interrupts enable mask, and enable RX interrupts in case if
	 * the interrupt is used for NMI entry.
	 */
	uap->int_msk = uart_v500_read(uap, REG_UART_INT_MSK);
	uart_v500_write((uap->int_msk | UART_V500_INT_RXM | UART_V500_INT_RTM) &
		UART_V500_MSK_INT_ALL, uap, REG_UART_INT_MSK);

	if (dev_get_platdata(uap->port.dev)) {
		struct uart_v500_dma_data *plat;

		plat = dev_get_platdata(uap->port.dev);
		if (plat->init)
			plat->init();
	}
	/* Optionaly enable pins to be muxed in and configured */
	if (!IS_ERR(uap->pinctrls_default)) {
		retval = pinctrl_select_state(uap->pinctrl,
			uap->pinctrls_default);
		if (retval) {
			dev_err(port->dev, "could not set default pins\n");
			return retval;
		}
	}
	return 0;
}

static int uart_v500_allocate_irq(struct uart_v500_port *uap)
{
	uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);

	return request_irq(uap->port.irq, uart_v500_int_handler, 0,
		"uart_v500", uap);
}

/*
 * Enable interrupts, only timeouts when using DMA
 * if initial RX DMA job failed, start in interrupt mode
 * as well.
 */
static void uart_v500_enable_interrupts(struct uart_v500_port *uap)
{
	unsigned int i;

	spin_lock_irq(&uap->port.lock);
	/* Clear out any spuriously appearing RX interrupts */
	uart_v500_write(UART_V500_INT_RTS | UART_V500_INT_RXS, uap,
		REG_UART_INT_CLR);

	for (i = 0; i < uap->fifosize * 2; ++i) {
		if (uart_v500_read(uap, REG_UART_STAT) & UART_V500_STAT_RXFE)
			break;

		uart_v500_read(uap, REG_UART_RX_FIFO);
	}

	uap->int_msk = (~UART_V500_INT_RTM);
	if (!uart_v500_dma_rx_running(uap))
		uap->int_msk &= (~UART_V500_INT_RXM);
	uart_v500_write(uap->int_msk, uap, REG_UART_INT_MSK);
	spin_unlock_irq(&uap->port.lock);
}

static int uart_v500_startup(struct uart_port *port)
{
	struct uart_v500_port *uap = container_of(port,
		struct uart_v500_port, port);
	unsigned int uart_cfg;
	int retval;

	dev_info(port->dev, "%s: ttyHW%d\n", __func__, port->line);
	uart_chip_reset_endisable(uap, 1);
	uart_chip_reset_endisable(uap, 0);

	retval = uart_v500_hwinit(port);
	if (retval)
		goto clk_dis;
	retval = uart_v500_allocate_irq(uap);
	if (retval)
		goto clk_dis;

	if (cpu_online(A53_CLUSTER0_CPU1) && uap->bind_interrupt_flag) {
		retval = irq_set_affinity(uap->port.irq,
			cpumask_of(A53_CLUSTER0_CPU1));
		if (retval)
			pr_err("bind uart%d interrupt to cpu1 failed\n",
				port->line);
	}

	uart_v500_write(uap->vendor->fifo_cfg, uap, REG_UART_FIFO_CFG);

	spin_lock_irq(&uap->port.lock);

	uart_cfg = uap->old_config;
	uart_cfg |= UART_V500_CFG_UART_EN | UART_V500_CFG_RX_EN |
		UART_V500_CFG_TX_EN;
	uart_v500_write(uart_cfg, uap, REG_UART_CONFIG);

	uart_v500_flow_ctrl_status(uap);

	spin_unlock_irq(&uap->port.lock);
	/* Startup DMA */
	uart_v500_dma_startup(uap);

	uart_v500_enable_interrupts(uap);

	return 0;

clk_dis:
	clk_disable_unprepare(uap->clk);
	return retval;
}

/*
 * disable the port. It should not disable RTS and DTR.
 * Also RTS and DTR state should be preserved to restore
 * it during startup().
 */
static void uart_v500_disable_uart(struct uart_v500_port *uap)
{
	unsigned int uart_cfg;

	uap->auto_rts = false;
	spin_lock_irq(&uap->port.lock);
	uart_cfg = uart_v500_read(uap, REG_UART_CONFIG);
	uart_cfg |= (uart_cfg << UART_CONFIG_BIT_MASK);
	uap->old_config = uart_cfg;
	uart_cfg = (UART_V500_CFG_TX_EN | UART_V500_CFG_UART_EN);
	uart_v500_write(uart_cfg, uap, REG_UART_CONFIG);
	spin_unlock_irq(&uap->port.lock);

	/*
	 * disable break condition
	 */
	uart_v500_write(UART_V500_CFG_BRK_DIS, uap, REG_UART_CONFIG);
}

static void uart_v500_disable_interrupts(struct uart_v500_port *uap)
{
	spin_lock_irq(&uap->port.lock);
	/* mask all interrupts and clear all pending ones */
	uart_v500_write(UART_V500_MSK_INT_ALL, uap, REG_UART_INT_MSK);
	uart_v500_write(UART_V500_CLR_INT_ALL, uap, REG_UART_INT_CLR);
	spin_unlock_irq(&uap->port.lock);
}

static void uart_v500_shutdown(struct uart_port *port)
{
	struct uart_v500_port *uap =
		container_of(port, struct uart_v500_port, port);
	int retval;

	dev_info(port->dev, "%s: ttyHW%d\n", __func__, port->line);
	disable_irq(uap->port.irq);

	uart_v500_disable_interrupts(uap);

	uart_v500_dma_shutdown(uap);

	free_irq(uap->port.irq, uap);
	uart_v500_disable_uart(uap);
	/*
	 * Shut down the clock producer
	 */
	clk_disable_unprepare(uap->clk);
	/* Optionally let pins go into sleep states */
	if (!IS_ERR(uap->pinctrls_idle) && ((console_uart_name_is_ttyhw == 0) ||
		(get_console_index() != (int)uap->port.line))) {
		retval = pinctrl_select_state(uap->pinctrl, uap->pinctrls_idle);
		if (retval)
			dev_err(port->dev, "can't set pins to sleep state\n");
	}

	if (dev_get_platdata(uap->port.dev)) {
		struct uart_v500_dma_data *plat;

		plat = dev_get_platdata(uap->port.dev);
		if (plat->exit)
			plat->exit();
	}

	if (uap->port.ops->flush_buffer)
		uap->port.ops->flush_buffer(port);
}

static void uart_v500_setup_status_masks(struct uart_port *port,
	struct ktermios *termios)
{
	port->read_status_mask = UART_V500_DR_OE | 255;
	if (termios->c_iflag & INPCK)
		port->read_status_mask |= UART_V500_DR_FE | UART_V500_DR_PE;
	if (termios->c_iflag & (IGNBRK | BRKINT | PARMRK))
		port->read_status_mask |= UART_V500_DR_BE;

	/*
	 * Characters to ignore
	 */
	port->ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		port->ignore_status_mask |= UART_V500_DR_FE | UART_V500_DR_PE;
	if (termios->c_iflag & IGNBRK) {
		port->ignore_status_mask |= UART_V500_DR_BE;
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			port->ignore_status_mask |= UART_V500_DR_OE;
	}
}

static void uart_v500_set_frame(struct uart_v500_port *uap,
	struct ktermios *termios)
{
	unsigned int frame_cfg;

	switch (termios->c_cflag & CSIZE) {
	case CS5:
		frame_cfg = UART_V500_FRAME_LEN_5;
		break;
	case CS6:
		frame_cfg = UART_V500_FRAME_LEN_6;
		break;
	case CS7:
		frame_cfg = UART_V500_FRAME_LEN_7;
		break;
	default: /* CS8 */
		frame_cfg = UART_V500_FRAME_LEN_8;
		break;
	}
	if (termios->c_cflag & CSTOPB)
		frame_cfg |= UART_V500_FRAME_STOP;
	if (termios->c_cflag & PARENB) {
		if (!(termios->c_cflag & PARODD))
			frame_cfg |= UART_V500_FRAME_POS;
		if (termios->c_cflag & CMSPAR)
			frame_cfg &= (~UART_V500_FRAME_NPS);
	}
	uart_v500_write(frame_cfg, uap, REG_UART_FRAME);
}

static void uart_v500_set_uart_cfg(struct uart_v500_port *uap,
	struct ktermios *termios)
{
	unsigned int uart_cfg;

	/* store the uart cfg old status */
	uart_cfg = uart_v500_read(uap, REG_UART_CONFIG);
	uart_cfg |= (uart_cfg << UART_CONFIG_BIT_MASK);
	/* first, disable everything */
	uart_v500_write(UART_V500_CFG_DIS_ALL, uap, REG_UART_CONFIG);

	if (uap->fifosize > 1)
		uart_cfg |= (UART_V500_CFG_RX_EN | UART_V500_CFG_TX_EN |
			UART_V500_CFG_UART_EN);

	if (termios->c_cflag & CRTSCTS) {
		uart_cfg |= UART_V500_CFG_RTS_EN | UART_V500_CFG_CTS_EN;
		uart_v500_write(uart_cfg, uap, REG_UART_CONFIG);
		uap->auto_rts = true;
	} else {
		uart_v500_write(uart_cfg, uap, REG_UART_CONFIG);
		uart_cfg = (UART_V500_CFG_CTS_DIS | UART_V500_CFG_RTS_DIS);
		uart_v500_write(uart_cfg, uap, REG_UART_CONFIG);
		uap->auto_rts = false;
	}
}

static void uart_v500_set_termios(struct uart_port *port,
	struct ktermios *termios, struct ktermios *old)
{
	struct uart_v500_port *uap = container_of(port,
		struct uart_v500_port, port);
	unsigned long flags;
	unsigned int baud, quot, quotn;
	unsigned int count = 0;

	baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk / 16);
	if (!baud) {
		dev_err(port->dev, "%s error baud!\n", __func__);
		return;
	}

	if (baud > port->uartclk / 16)
		quot = DIV_ROUND_CLOSEST(port->uartclk * 8, baud);
	else
		quot = DIV_ROUND_CLOSEST(port->uartclk * 4, baud);

	uart_v500_set_frame(uap, termios);

	spin_lock_irqsave(&port->lock, flags);

	uart_update_timeout(port, termios->c_cflag, baud);

	uart_v500_setup_status_masks(port, termios);

	if (UART_ENABLE_MS(port, termios->c_cflag))
		uart_v500_enable_msk(port);
	else
		uart_v500_disable_msk(port);

	uart_v500_set_uart_cfg(uap, termios);

	while ((uart_v500_read(uap, REG_UART_STAT) & uap->vendor->stat_busy) &&
		(count < TXFF_TIMEOUT)) {
		udelay(1);
		count++;
		barrier();
	}

	/* Set baud rate */
	quotn = quot;
	quot = (quot & UART_BAUD_FRAC_MASK) << UART_CONFIG_BIT_MASK;
	quotn = (quotn >> 6) & UART_BAUD_INT_MASK;
	quot |= quotn;
	uart_v500_write(quot, uap, REG_UART_BAUD);

	spin_unlock_irqrestore(&port->lock, flags);
}

static const char *uart_v500_type(struct uart_port *port)
{
	struct uart_v500_port *uap = container_of(port,
		struct uart_v500_port, port);

	return uap->port.type == PORT_UART_V500 ? uap->uart_type : NULL;
}

/*
 * Release the memory region(s) being used by 'port'
 */
static void uart_v500_release_port(struct uart_port *port)
{
	release_mem_region(port->mapbase, SZ_4K);
}

/*
 * Request the memory region(s) being used by 'port'
 */
static int uart_v500_request_port(struct uart_port *port)
{
	return request_mem_region(port->mapbase, SZ_4K, "uart_v500")
		!= NULL ? 0 : -EBUSY;
}

/*
 * Configure auto configure the port.
 */
static void uart_v500_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE) {
		port->type = PORT_UART_V500;
		uart_v500_request_port(port);
	}
}

/*
 * verify the new serial_struct (for TIOCSSERIAL).
 */
static int uart_v500_verify_port(struct uart_port *port,
	struct serial_struct *ser)
{
	int ret = 0;

	if (ser->type != PORT_UNKNOWN && ser->type != PORT_UART_V500)
		ret = -EINVAL;
	if (ser->irq < 0 || ser->irq >= nr_irqs)
		ret = -EINVAL;
	if (ser->baud_base < 9600)
		ret = -EINVAL;

	return ret;
}

static const struct uart_ops uart_v500_pops = {
	.tx_empty	= uart_v500_tx_empty,
	.set_mctrl	= uart_v500_set_mctrl,
	.get_mctrl	= uart_v500_get_mctrl,
	.stop_tx	= uart_v500_stop_tx,
	.start_tx	= uart_v500_start_tx,
	.stop_rx	= uart_v500_stop_rx,
	.enable_ms	= uart_v500_enable_msk,
	.break_ctl	= uart_v500_break_ctl,
	.startup	= uart_v500_startup,
	.shutdown	= uart_v500_shutdown,
	.flush_buffer	= uart_v500_dma_flush_buffer,
	.set_termios	= uart_v500_set_termios,
	.type		= uart_v500_type,
	.release_port	= uart_v500_release_port,
	.request_port	= uart_v500_request_port,
	.config_port	= uart_v500_config_port,
	.verify_port	= uart_v500_verify_port,
	.pm		= uart_v500_pm,
};

static void uart_v500_unregister_port(struct uart_v500_port *uap)
{
	unsigned int i;
	bool busy = false;

	for (i = 0; i < ARRAY_SIZE(uart_ports_ne); i++) {
		if (uart_ports_ne[i] == uap)
			uart_ports_ne[i] = NULL;
		else if (uart_ports_ne[i])
			busy = true;
	}

	uart_v500_dma_remove(uap);
	if (!busy)
		uart_unregister_driver(&g_uart_v500);
}

static int uart_setup_port(struct device *dev, struct uart_v500_port *uap,
	struct resource *mmiobase, int index)
{
	void __iomem *base = NULL;
	int ret;
	struct platform_device *pdev = container_of(dev,
		struct platform_device, dev);

	base = devm_ioremap_resource(dev, mmiobase);
	if (IS_ERR(base))
		return PTR_ERR(base);

	index = uart_probe_dt_alias(index, dev);
	ret = uart_v500_pinctrl(pdev, uap);
	if (ret) {
		dev_err(&pdev->dev, "%s get pinctrl failed!\n", __func__);
		return ret;
	}
	uap->old_config = 0;
	uap->port.dev = dev;
	uap->port.mapbase = mmiobase->start;
	uap->port.membase = base;
	uap->port.fifosize = uap->fifosize;
	uap->port.flags = UPF_BOOT_AUTOCONF;
	uap->port.line = index;
	uart_ports_ne[index] = uap;

	ret = uart_v500_probe_reset_func_enable(pdev, uap);
	if (ret)
		dev_err(&pdev->dev, "%s not enable reset func!\n", __func__);

	ret = clk_prepare_enable(uap->clk);
	if (ret) {
		dev_err(&pdev->dev, "%s clk prepare enable fail!\n", __func__);
		return ret;
	}
	/* Ensure interrupts from this UART are masked and cleared */
	uart_v500_write(UART_V500_MSK_INT_ALL, uap, REG_UART_INT_MSK);
	uart_v500_write(UART_V500_CLR_INT_ALL, uap, REG_UART_INT_CLR);

	clk_disable_unprepare(uap->clk);

	uart_v500_probe_console_enable(pdev, uap, uart_v500_console.name);

	return 0;
}

static int uart_v500_register_port(struct uart_v500_port *uap)
{
	int ret;

	ret = uart_add_one_port(&g_uart_v500, &uap->port);
	if (ret) {
		if (console_uart_name_is_ttyhw && (get_console_index() ==
			(int)uap->port.line))
			uart_v500_async_print_work_uninit(uap);

		uart_v500_unregister_port(uap);
	}

	return ret;
}

static int uart_v500_probe(struct platform_device *pdev)
{
	struct uart_v500_port *uap = NULL;
	struct vendor_data *vendor = &g_vendor_uart_v500;
	int port_num, ret;
	struct resource *resource = NULL;

	port_num = uart_v500_find_free_port();
	if (port_num < 0)
		return port_num;

	uap = devm_kzalloc(&pdev->dev, sizeof(struct uart_v500_port),
		GFP_KERNEL);
	if (!uap)
		return -ENOMEM;

	uap->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(uap->clk))
		return PTR_ERR(uap->clk);

	ret = uart_v500_probe_get_clk_freq(pdev, uap, port_num);
	if (ret)
		dev_err(&pdev->dev, "%s can not get clk freq!\n", __func__);

	if (of_property_read_u32(pdev->dev.of_node, "bind-interrupt-flag",
		&uap->bind_interrupt_flag))
		uap->bind_interrupt_flag = 0;

	uap->rx_global_sg_buf_alloced = false;
	if (of_property_read_u32(pdev->dev.of_node, "rx-use-global-sg-buf",
		&uap->rx_use_global_sg_buf))
		uap->rx_use_global_sg_buf = 0;

	uap->reg_offset = vendor->reg_offset;
	uap->vendor = vendor;
	uap->fifosize = FIFO_SIZE_UART_V500;
	uap->port.iotype = vendor->access_32b ? UPIO_MEM32 : UPIO_MEM;
	uap->port.irq = platform_get_irq(pdev, 0);
	uap->port.ops = &uart_v500_pops;

	ret = snprintf_s(uap->uart_type, sizeof(uap->uart_type),
		sizeof("uart_v500"), "%s", "uart_v500");
	if (ret < 0) {
		pr_err("%s: snprintf_s fail %d\n", __func__, ret);
		return ret;
	}

	resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	ret = uart_setup_port(&pdev->dev, uap, resource, port_num);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, uap);

	return uart_v500_register_port(uap);
}

static int uart_v500_remove(struct platform_device *pdev)
{
	struct uart_v500_port *uap = platform_get_drvdata(pdev);

	devm_pinctrl_put(uap->pinctrl);

	uart_remove_one_port(&g_uart_v500, &uap->port);
	uart_v500_unregister_port(uap);
	return 0;
}

#if defined(CONFIG_OF)
static const struct of_device_id uart_v500_ids[] = {
	{ .compatible = "hisilicon,uart" },
	{ },
};
#endif

static struct platform_driver uart_v500_platform_driver = {
	.probe = uart_v500_probe,
	.remove = uart_v500_remove,
	.suspend = uart_v500_suspend,
	.resume = uart_v500_resume,
	.driver = {
		.name = "uart_v500",
		.of_match_table = of_match_ptr(uart_v500_ids),
	},
};

static int __init uart_v500_init(void)
{
	int ret;

	printk(KERN_ERR "Serial: UART V500 driver\n");
	ret = uart_register_driver(&g_uart_v500);
	if (ret) {
		printk(KERN_ERR "uart_register_driver fail\n");
		return ret;
	}
	ret = platform_driver_register(&uart_v500_platform_driver);
	if (ret) {
		printk(KERN_ERR "platform_driver_register fail\n");
		uart_unregister_driver(&g_uart_v500);
	}

	return ret;
}

static void __exit uart_v500_exit(void)
{
	platform_driver_unregister(&uart_v500_platform_driver);
	uart_unregister_driver(&g_uart_v500);
}
/*
 * While this can be a module, if builtin it's most likely the console
 * So let's leave module_exit but move module_init to an earlier place
 */
arch_initcall(uart_v500_init);
module_exit(uart_v500_exit);

MODULE_DESCRIPTION("UART V500 port driver");
MODULE_LICENSE("GPL");
