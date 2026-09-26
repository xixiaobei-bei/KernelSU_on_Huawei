/*
 * ILITEK Touch IC driver
 *
 * Copyright (C) 2011 ILI Technology Corporation.
 *
 * Author: Dicky Chiang <dicky_chiang@ilitek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ilitek.h"


#if SPI_DMA_TRANSFER_SPLIT
#define DMA_TRANSFER_MAX_CHUNK 64 // number of chunks to be transferred.
#define DMA_TRANSFER_MAX_LEN 1024 // length of a chunk.
#define WDATA_LENGTH 34
#define WDATA_HEAD_LENGTH 4

int ili_spi_write_then_read_split(struct spi_device *spi,
		const void *txbuf, unsigned n_tx,
		void *rxbuf, unsigned n_rx)
{
	int status = -1, duplex_len = 0;
	int xfercnt = 0, xferlen = 0, xferloop = 0;
	int offset = 0;
	u8 cmd = 0;
	struct spi_message message;
	struct spi_transfer *xfer;

	xfer = kzalloc(DMA_TRANSFER_MAX_CHUNK * sizeof(struct spi_transfer), GFP_KERNEL);
	if (err_alloc_mem(xfer)) {
		TS_LOG_ERR("xfer kzalloc error\n");
		return -ENOMEM;
	}

	if (n_rx > SPI_RX_BUF_SIZE) {
		TS_LOG_ERR("Rx length is greater than spi local buf, abort\n");
		status = -ENOMEM;
		goto out;
	}

	spi_message_init(&message);
	memset(ilits->spi_tx, 0x0, SPI_TX_BUF_SIZE);
	memset(ilits->spi_rx, 0x0, SPI_RX_BUF_SIZE);

	if ((n_tx > 0) && (n_rx > 0))
		cmd = SPI_READ;
	else
		cmd = SPI_WRITE;

	switch (cmd) {
	case SPI_WRITE:
		if (n_tx % DMA_TRANSFER_MAX_LEN)
			xferloop = (n_tx / DMA_TRANSFER_MAX_LEN) + 1;
		else
			xferloop = n_tx / DMA_TRANSFER_MAX_LEN;

		xferlen = n_tx;
		memcpy(ilits->spi_tx, (u8 *)txbuf, xferlen);

		for (xfercnt = 0; xfercnt < xferloop; xfercnt++) {
			if (xferlen > DMA_TRANSFER_MAX_LEN)
				xferlen = DMA_TRANSFER_MAX_LEN;

			xfer[xfercnt].len = xferlen;
			xfer[xfercnt].tx_buf = ilits->spi_tx + xfercnt * DMA_TRANSFER_MAX_LEN;
			spi_message_add_tail(&xfer[xfercnt], &message);
			xferlen = n_tx - (xfercnt+1) * DMA_TRANSFER_MAX_LEN;
		}
		status = spi_sync(spi, &message);
		break;
	case SPI_READ:
		if (n_tx > DMA_TRANSFER_MAX_LEN) {
			TS_LOG_ERR("Tx length must be lower than dma length %d.\n", DMA_TRANSFER_MAX_LEN);
			status = -EINVAL;
			break;
		}

		if (!atomic_read(&ilits->ice_stat))
			offset = 2;

		memcpy(ilits->spi_tx, txbuf, n_tx);
		duplex_len = n_tx + n_rx + offset;

		if (duplex_len % DMA_TRANSFER_MAX_LEN)
			xferloop = (duplex_len / DMA_TRANSFER_MAX_LEN) + 1;
		else
			xferloop = duplex_len / DMA_TRANSFER_MAX_LEN;

		xferlen = duplex_len;
		for (xfercnt = 0; xfercnt < xferloop; xfercnt++) {
			if (xferlen > DMA_TRANSFER_MAX_LEN)
				xferlen = DMA_TRANSFER_MAX_LEN;

			xfer[xfercnt].len = xferlen;
			xfer[xfercnt].tx_buf = ilits->spi_tx;
			xfer[xfercnt].rx_buf = ilits->spi_rx + xfercnt * DMA_TRANSFER_MAX_LEN;
			spi_message_add_tail(&xfer[xfercnt], &message);
			xferlen = duplex_len - (xfercnt + 1) * DMA_TRANSFER_MAX_LEN;
		}

		status = spi_sync(spi, &message);
		if (status == 0) {
			if (ilits->spi_rx[1] != SPI_ACK && !atomic_read(&ilits->ice_stat)) {
				status = DO_SPI_RECOVER;
				TS_LOG_ERR("Do spi recovery: rxbuf[1] = 0x%x, ice = %d\n", ilits->spi_rx[1], atomic_read(&ilits->ice_stat));
				break;
			}

			memcpy((u8 *)rxbuf, ilits->spi_rx + offset + 1, n_rx);
		} else {
			TS_LOG_ERR("spi read fail, status = %d\n", status);
		}
		break;
	default:
		TS_LOG_INFO("Unknown command 0x%x\n", cmd);
		break;
	}

out:
	ipio_kfree((void **)&xfer);
	return status;
}
#else
int ili_spi_write_then_read_direct(struct spi_device *spi,
		const void *txbuf, unsigned n_tx,
		void *rxbuf, unsigned n_rx)
{
	int status = -1, duplex_len = 0;
	int offset = 0;
	u8 cmd;
	struct spi_message message;
	struct spi_transfer xfer;

	if (n_rx > SPI_RX_BUF_SIZE) {
		TS_LOG_ERR("Rx length is greater than spi local buf, abort\n");
		status = -ENOMEM;
		goto out;
	}

	spi_message_init(&message);
	memset(&xfer, 0, sizeof(xfer));

	if ((n_tx > 0) && (n_rx > 0))
		cmd = SPI_READ;
	else
		cmd = SPI_WRITE;

	switch (cmd) {
	case SPI_WRITE:
		xfer.len = n_tx;
		xfer.tx_buf = txbuf;
		spi_message_add_tail(&xfer, &message);
		status = spi_sync(spi, &message);
		break;
	case SPI_READ:
		if (!atomic_read(&ilits->ice_stat))
			offset = 2;

		duplex_len = n_tx + n_rx + offset;
		if ((duplex_len > SPI_TX_BUF_SIZE) ||
			(duplex_len > SPI_RX_BUF_SIZE)) {
			TS_LOG_ERR("duplex_len is over than dma buf, abort\n");
			status = -ENOMEM;
			break;
		}

		memset(ilits->spi_tx, 0x0, SPI_TX_BUF_SIZE);
		memset(ilits->spi_rx, 0x0, SPI_RX_BUF_SIZE);

		xfer.len = duplex_len;
		memcpy(ilits->spi_tx, txbuf, n_tx);
		xfer.tx_buf = ilits->spi_tx;
		xfer.rx_buf = ilits->spi_rx;

		spi_message_add_tail(&xfer, &message);
		status = spi_sync(spi, &message);
		if (status == 0) {
			if (ilits->spi_rx[1] != SPI_ACK && !atomic_read(&ilits->ice_stat)) {
				status = DO_SPI_RECOVER;
				TS_LOG_ERR("Do spi recovery: rxbuf[1] = 0x%x, ice = %d\n", ilits->spi_rx[1], atomic_read(&ilits->ice_stat));
				break;
			}

			memcpy((u8 *)rxbuf, ilits->spi_rx + offset + 1, n_rx);
		} else {
			TS_LOG_ERR("spi read fail, status = %d\n", status);
		}
		break;
	default:
		TS_LOG_INFO("Unknown command 0x%x\n", cmd);
		break;
	}

out:
	return status;
}
#endif

static int ili_spi_mp_pre_cmd(u8 cdc)
{
	u8 pre[5] = {0};

	if (!atomic_read(&ilits->mp_stat) || cdc != P5_X_SET_CDC_INIT ||
		ilits->chip->core_ver >= CORE_VER_1430)
		return 0;

	TS_LOG_DEBUG("mp test with pre commands\n");

	pre[0] = SPI_WRITE;
	pre[1] = 0x0;// dummy byte
	pre[2] = 0x2;// Write len byte
	pre[3] = P5_X_READ_DATA_CTRL;
	pre[4] = P5_X_GET_CDC_DATA;
	if (ilits->spi_write_then_read(ilits->spi, pre, 5, NULL, 0) < 0) {
		TS_LOG_ERR("Failed to write pre commands\n");
		return -1;
	}

	pre[0] = SPI_WRITE;
	pre[1] = 0x0;// dummy byte
	pre[2] = 0x1;// Write len byte
	pre[3] = P5_X_GET_CDC_DATA;
	if (ilits->spi_write_then_read(ilits->spi, pre, 4, NULL, 0) < 0) {
		TS_LOG_ERR("Failed to write pre commands\n");
		return -1;
	}
	return 0;
}

static int ili_spi_pll_clk_wakeup(void)
{
	int index = 0;
	u8 wdata[32] = {0};
	u8 wakeup[9] = {0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3};
	u32 wlen = sizeof(wakeup);

	wdata[0] = SPI_WRITE;
	wdata[1] = wlen >> 8;
	wdata[2] = wlen & 0xff;
	index = 3;
	wlen += index;

	memcpy(&wdata[index], wakeup, wlen);

	TS_LOG_INFO("Write dummy to wake up spi pll clk\n");
	if (ilits->spi_write_then_read(ilits->spi, wdata, wlen, NULL, 0) < 0) {
		TS_LOG_INFO("spi slave write error\n");
		return -1;
	}

	return 0;
}

static int ili_spi_wrapper(u8 *txbuf, u32 wlen, u8 *rxbuf, u32 rlen, bool spi_irq, bool i2c_irq)
{
	int ret = 0;
	int mode = 0, index = 0;
	u8 wdata[WDATA_LENGTH] = {0};
	u8 checksum = 0;
	bool ice = atomic_read(&ilits->ice_stat);

	if (wlen > 0) {
		if (!txbuf) {
			TS_LOG_ERR("txbuf is null\n");
			return -ENOMEM;
		}

		/* 3 bytes data consist of length and header */
		if (wlen >= sizeof(wdata) - WDATA_HEAD_LENGTH) {
			TS_LOG_ERR("WARNING! wlen%d > wdata%d, using wdata length to transfer\n", wlen, (int)sizeof(wdata));
			wlen = sizeof(wdata) - WDATA_HEAD_LENGTH;
		}
	}

	if (rlen > 0) {
		if (!rxbuf) {
			TS_LOG_ERR("rxbuf is null\n");
			return -ENOMEM;
		}
	}

	if (rlen > 0 && !wlen)
		mode = SPI_READ;
	else
		mode = SPI_WRITE;

		ilits->detect_int_stat = ili_ic_check_int_level;

	if (spi_irq)
		atomic_set(&ilits->cmd_int_check, ENABLE);

	switch (mode) {
	case SPI_WRITE:
		if (ice) {
			wdata[0] = SPI_WRITE;
			index = 1;
		} else {
			wdata[0] = SPI_WRITE;
			wdata[1] = wlen >> 8;
			wdata[2] = wlen & 0xff;
			index = 3;

			if (ilits->tp_suspend && (ilits->actual_tp_mode == P5_X_FW_GESTURE_MODE) &&
				(atomic_read(&ilits->esd_stat) == END) &&
				(atomic_read(&ilits->fw_stat) == END)) {
				TS_LOG_INFO("write dummy to wake up spi pll clk\n");
				ret = ili_spi_pll_clk_wakeup();
				if (ret < 0) {
					TS_LOG_ERR("Wakeup pll clk error\n");
					break;
				}
			}
		}


		memcpy(&wdata[index], txbuf, wlen);
		wlen += index;

		/*
		* NOTE: If TP driver is doing MP test and commanding 0xF1 to FW, we add a checksum
		* to the last index and plus 1 with size.
		*/
		if (atomic_read(&ilits->mp_stat) && wdata[index] == P5_X_SET_CDC_INIT) {
			checksum = ili_calc_packet_checksum(&wdata[index], wlen - index);
			wdata[wlen] = checksum;
			wlen++;
			wdata[1] = (wlen - index) >> 8;
			wdata[2] = (wlen - index) & 0xff;
			ili_dump_data(wdata, 8, wlen, 0, "mp cdc cmd with checksum");
		}

		ret = ilits->spi_write_then_read(ilits->spi, wdata, wlen, txbuf, 0);
		if (ret < 0) {
			TS_LOG_INFO("spi-wrapper write error\n");
			break;
		}

		/* Won't break if it needs to read data following with writing. */
		if (!rlen)
			break;
	case SPI_READ:
		if (!ice && spi_irq) {
			/* Check INT triggered by FW when sending cmds. */
			if (ilits->detect_int_stat(false) < 0) {
				TS_LOG_ERR("ERROR! Check INT timeout\n");
				ret = -ETIME;
				break;
			}
		}

		ret = ili_spi_mp_pre_cmd(wdata[3]);
		if (ret < 0)
			TS_LOG_ERR("spi-wrapper mp pre cmd error\n");

		wdata[0] = SPI_READ;

		ret = ilits->spi_write_then_read(ilits->spi, wdata, 1, rxbuf, rlen);
		if (ret < 0)
			TS_LOG_ERR("spi-wrapper read error\n");

		break;
	default:
		TS_LOG_ERR("Unknown spi mode %d\n", mode);
		ret = -EINVAL;
		break;
	}

	if (spi_irq)
		atomic_set(&ilits->cmd_int_check, DISABLE);

	return ret;
}

int ili_core_spi_setup(int num)
{
	return 0;
}

void ilitek_bus_release(void)
{
	ipio_kfree((void **)&ilits->update_buf);
	ipio_kfree((void **)&ilits->tr_buf);
	ipio_kfree((void **)&ilits->spi_tx);
	ipio_kfree((void **)&ilits->spi_rx);
	ipio_kfree((void **)&ilits->gcoord);
	return;
}

int ilitek_bus_init(void)
{
	struct spi_device *spi = NULL;
	TS_LOG_INFO("spi probe\n");

	ilits->update_buf = kzalloc(MAX_HEX_FILE_SIZE, GFP_KERNEL | GFP_DMA);
	if (err_alloc_mem(ilits->update_buf)) {
		TS_LOG_ERR("fw kzalloc error\n");
		return -ENOMEM;
	}

	/* Used for receiving touch data only, do not mix up with others. */
	ilits->tr_buf = kzalloc(TR_BUF_SIZE, GFP_ATOMIC);
	if (err_alloc_mem(ilits->tr_buf)) {
		TS_LOG_ERR("failed to allocate touch report buffer\n");
		return -ENOMEM;
	}

	ilits->spi_tx = kzalloc(SPI_TX_BUF_SIZE, GFP_KERNEL | GFP_DMA);
	if (err_alloc_mem(ilits->spi_tx)) {
		TS_LOG_ERR("Failed to allocate spi tx buffer\n");
		return -ENOMEM;
	}

	ilits->spi_rx = kzalloc(SPI_RX_BUF_SIZE, GFP_KERNEL | GFP_DMA);
	if (err_alloc_mem(ilits->spi_rx)) {
		TS_LOG_ERR("Failed to allocate spi rx buffer\n");
		return -ENOMEM;
	}

	ilits->gcoord = kzalloc(sizeof(struct gesture_coordinate), GFP_KERNEL);
	if (err_alloc_mem(ilits->gcoord)) {
		TS_LOG_ERR("Failed to allocate gresture coordinate buffer\n");
		return -ENOMEM;
	}

	ilits->i2c = NULL;
	spi = ilits->ts_dev_data->ts_platform_data->spi;
	ilits->spi = spi;
	ilits->dev = &spi->dev;
	ilits->phys = "SPI";
	ilits->wrapper = ili_spi_wrapper;
	ilits->detect_int_stat = ili_ic_check_int_level;

	ilits->int_pulse = false;

	ilits->mp_retry = false;

#if SPI_DMA_TRANSFER_SPLIT
	ilits->spi_write_then_read = ili_spi_write_then_read_split;
#else
	ilits->spi_write_then_read = ili_spi_write_then_read_direct;
#endif

	ilits->actual_tp_mode = P5_X_FW_AP_MODE;
	ilits->tp_data_format = DATA_FORMAT_DEMO;
	ilits->tp_data_len = P5_X_DEMO_MODE_PACKET_LEN;

	if (TDDI_RST_BIND)
		ilits->reset = TP_IC_WHOLE_RST;
	else
		ilits->reset = TP_HW_RST_ONLY;

	ilits->rst_edge_delay = 10;
	ilits->fw_open = REQUEST_FIRMWARE;
	ilits->fw_upgrade_mode = UPGRADE_IRAM;
	ilits->mp_move_code = ili_move_mp_code_iram;
	ilits->gesture_move_code = ili_move_gesture_code_iram;
	ilits->esd_recover = ili_wq_esd_spi_check;
	ilits->ges_recover = ili_touch_esd_gesture_iram;
	ilits->gesture_mode = DATA_FORMAT_GESTURE_INFO;
	ilits->gesture_demo_ctrl = DISABLE;
	ilits->wtd_ctrl = OFF;
	ilits->report = ENABLE;
	ilits->netlink = DISABLE;
	ilits->dnp = DISABLE;
	ilits->irq_tirgger_type = IRQF_TRIGGER_FALLING;
	ilits->info_from_hex = ENABLE;
	ilits->wait_int_timeout = AP_INT_TIMEOUT;


	ilits->gesture = DISABLE;
	ilits->ges_sym.double_tap = DOUBLE_TAP;


	if (ili_core_spi_setup(SPI_CLK) < 0)
		return -EINVAL;

	return 0;
}

