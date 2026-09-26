#define LOG_TAG "Plat"

#include "cts_config.h"
#include "cts_platform.h"
#include "cts_core.h"
#include "cts_firmware.h"

bool cts_show_debug_log = false;
module_param_named(debug_log, cts_show_debug_log, bool, 0660);
MODULE_PARM_DESC(debug_log, "Show debug log control");

extern struct chipone_ts_data *cts_data;

size_t cts_plat_get_max_spi_xfer_size(struct cts_platform_data *pdata)
{
	return CFG_CTS_MAX_SPI_XFER_SIZE;
}

u8 *cts_plat_get_spi_xfer_buf(struct cts_platform_data *pdata, size_t xfer_size)
{
	return cts_data->spi_cache_buf;
}

static int cts_spi_send_recv(struct cts_platform_data *pdata, size_t len , u8 *tx_buffer, u8 *rx_buffer)
{
	struct spi_message msg;
	struct spi_transfer cmd = {
		.cs_change = 0,
		.delay_usecs = 0,
		.speed_hz = cts_data->spi_max_speed,
		.tx_buf = tx_buffer,
		.rx_buf = rx_buffer,
		.len	= len,
		.bits_per_word = 8,
	};
	int ret = 0;
	spi_message_init(&msg);
	spi_message_add_tail(&cmd,  &msg);
	ret = spi_sync(cts_data->tskit_data->ts_platform_data->spi, &msg);
	if (ret) {
		TS_LOG_ERR("spi sync failed %d", ret);
	}
	return ret;
}

int cts_plat_spi_write(struct cts_platform_data *pdata, u8 dev_addr,
	const void *src, size_t len, int retry, int delay)
{
	int ret = 0, retries = 0;
	u16 crc;
	size_t data_len;

	if (len > CFG_CTS_MAX_SPI_XFER_SIZE) {
		TS_LOG_ERR("write too much data:wlen=%zu\n", len);
		return -EIO;
	}

	if (cts_data->cts_dev.rtdata.program_mode) {
		cts_data->spi_tx_buf[0] = dev_addr;
		memcpy(&cts_data->spi_tx_buf[1], src, len);

		do {
			ret = cts_spi_send_recv(pdata, len + 1, cts_data->spi_tx_buf, cts_data->spi_rx_buf);
			if (ret) {
				TS_LOG_ERR("SPI write failed %d", ret);
				if (delay) {
					mdelay(delay);
				}
			} else {
				return 0;
			}
		} while (++retries < retry);
	}
	else {
		data_len = len - 2;
		cts_data->spi_tx_buf[0] = dev_addr;
		cts_data->spi_tx_buf[1] = *((u8 *)src + 1);
		cts_data->spi_tx_buf[2] = *((u8 *)src);
		put_unaligned_le16(data_len, &cts_data->spi_tx_buf[3]);
		crc = (u16)cts_data_check(cts_data->spi_tx_buf, 5);
		put_unaligned_le16(crc, &cts_data->spi_tx_buf[5]);
		memcpy(&cts_data->spi_tx_buf[7], (char *)src + 2, data_len);
		crc = (u16)cts_data_check((char *)src + 2, data_len);
		put_unaligned_le16(crc, &cts_data->spi_tx_buf[7+data_len]);
		do {
			ret = cts_spi_send_recv(pdata, len + 7, cts_data->spi_tx_buf, cts_data->spi_rx_buf);
			udelay(10 * data_len);
			if (ret) {
				TS_LOG_ERR("SPI write failed %d", ret);
				if (delay) {
					mdelay(delay);
				}
			} else {
				return 0;
			}
		} while (++retries < retry);
	}
	return ret;
}

int cts_plat_spi_read(struct cts_platform_data *pdata, u8 dev_addr,
		const u8 *wbuf, size_t wlen, void *rbuf, size_t rlen,
		int retry, int delay)
{
	int ret = 0, retries = 0;
	u16 crc;

	if (wlen > CFG_CTS_MAX_SPI_XFER_SIZE || rlen > CFG_CTS_MAX_SPI_XFER_SIZE) {
		TS_LOG_ERR("write/read too much data:wlen=%zd, rlen=%zd", wlen, rlen);
		return -EIO;
	}

	if (cts_data->cts_dev.rtdata.program_mode)
	{
		cts_data->spi_tx_buf[0] = dev_addr | 0x01;
		memcpy(&cts_data->spi_tx_buf[1], wbuf, wlen);
		do {
			ret = cts_spi_send_recv(pdata, rlen + 5, cts_data->spi_tx_buf, cts_data->spi_rx_buf);
			if (ret) {
				TS_LOG_ERR("SPI read failed %d", ret);
				if (delay) {
					mdelay(delay);
				}
				continue;
			}
			memcpy(rbuf, cts_data->spi_rx_buf+5, rlen);
			return 0;
		} while(++retries < retry);
	} else {
		do {
			if (wlen != 0) {
				cts_data->spi_tx_buf[0] = dev_addr | 0x01;
				cts_data->spi_tx_buf[1] = wbuf[1];
				cts_data->spi_tx_buf[2] = wbuf[0];
				put_unaligned_le16(rlen, &cts_data->spi_tx_buf[3]);
				crc = (u16)cts_data_check(cts_data->spi_tx_buf, 5);
				put_unaligned_le16(crc, &cts_data->spi_tx_buf[5]);
				ret = cts_spi_send_recv(pdata, 7, cts_data->spi_tx_buf, cts_data->spi_rx_buf);
				if (ret) {
					TS_LOG_ERR("SPI read failed %d", ret);
					if (delay) {
						mdelay(delay);
					}
					continue;
				}
			}
			memset(cts_data->spi_tx_buf, 0, 7);
			cts_data->spi_tx_buf[0] = dev_addr | 0x01;
			udelay(100);
			ret =
				cts_spi_send_recv(pdata, rlen + 2, cts_data->spi_tx_buf, cts_data->spi_rx_buf);
			if (ret) {
				TS_LOG_ERR("SPI read failed %d", ret);
				if (delay) {
					mdelay(delay);
				}
				continue;
			}
			memcpy(rbuf, cts_data->spi_rx_buf, rlen);
			crc = (u16) cts_data_check(cts_data->spi_rx_buf, rlen);
			if (get_unaligned_le16(&cts_data->spi_rx_buf[rlen]) != crc) {
				TS_LOG_ERR("SPI RX CRC error: rx_crc %04x != %04x",
					get_unaligned_le16(&cts_data->spi_rx_buf[rlen]), crc);
				continue;
			}
			return 0;
		} while (++retries < retry);
	}
	if (retries >= retry) {
		TS_LOG_ERR("SPI read too much retry");
	}

	return -EIO;
}

int cts_plat_spi_read_delay_idle(struct cts_platform_data *pdata, u8 dev_addr,
		const u8 *wbuf, size_t wlen, void *rbuf,
		size_t rlen, int retry, int delay, int idle)
{
	int ret = 0, retries = 0;
	u16 crc;

	if (wlen > CFG_CTS_MAX_SPI_XFER_SIZE ||
		rlen > CFG_CTS_MAX_SPI_XFER_SIZE) {
		TS_LOG_ERR("write/read too much data:wlen=%zu, rlen=%zu", wlen,
			rlen);
		return -E2BIG;
	}

	if (cts_data->cts_dev.rtdata.program_mode) {
		cts_data->spi_tx_buf[0] = dev_addr | 0x01;
		memcpy(&cts_data->spi_tx_buf[1], wbuf, wlen);
		do {
			ret =
				cts_spi_send_recv(pdata, rlen + 5,
					cts_data->spi_tx_buf,
					cts_data->spi_rx_buf);
			if (ret) {
				TS_LOG_ERR("SPI read failed %d", ret);
				if (delay) {
					mdelay(delay);
				}
				continue;
			}
			memcpy(rbuf, cts_data->spi_rx_buf + 5, rlen);
			return 0;
		} while (++retries < retry);
	} else {
		do {
			if (wlen != 0) {
				cts_data->spi_tx_buf[0] = dev_addr | 0x01;
				cts_data->spi_tx_buf[1] = wbuf[1];
				cts_data->spi_tx_buf[2] = wbuf[0];
				put_unaligned_le16(rlen, &cts_data->spi_tx_buf[3]);
				crc = (u16) cts_data_check(cts_data->spi_tx_buf, 5);
				put_unaligned_le16(crc, &cts_data->spi_tx_buf[5]);
				ret =
					cts_spi_send_recv(pdata, 7, cts_data->spi_tx_buf, cts_data->spi_rx_buf);
				if (ret) {
					TS_LOG_ERR("SPI read failed %d", ret);
					if (delay) {
						mdelay(delay);
					}
					continue;
				}
			}
			memset(cts_data->spi_tx_buf, 0, 7);
			cts_data->spi_tx_buf[0] = dev_addr | 0x01;
			udelay(idle);
			ret = cts_spi_send_recv(pdata, rlen + 2, cts_data->spi_tx_buf, cts_data->spi_rx_buf);
			if (ret) {
				if (delay) {
					mdelay(delay);
				}
				continue;
			}
			memcpy(rbuf, cts_data->spi_rx_buf, rlen);
			crc = (u16)cts_data_check(cts_data->spi_rx_buf, rlen);
			if (get_unaligned_le16(&cts_data->spi_rx_buf[rlen]) != crc) {
				continue;
			}
			return 0;
		} while (++retries < retry);
	}
	if (retries >= retry) {
		TS_LOG_ERR("plat_spi_read error");
	}

	return -EIO;
}

int cts_plat_is_normal_mode(struct cts_platform_data *pdata)
{
	u8 tx_buf[4] = {0};
	u16 fwid;
	u32 addr;
	int ret;

	cts_set_normal_addr(&cts_data->cts_dev);
	addr = CTS_DEVICE_FW_REG_CHIP_TYPE;
	put_unaligned_be16(addr, tx_buf);
	ret = cts_plat_spi_read(pdata, CTS_DEV_NORMAL_MODE_SPIADDR, tx_buf, 2, &fwid, 2, 3, 10);
	fwid = be16_to_cpu(fwid);
	if (ret || !cts_is_fwid_valid(fwid)) {
		return false;
	}

	return true;
}
