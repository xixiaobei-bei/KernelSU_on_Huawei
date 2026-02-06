/*
 *                  Copyright 2010 - 2017, Huawei Tech. Co., Ltd.
 *                            ALL RIGHTS RESERVED
 *
 * Filename      : focaltech_spi.c
 * Author        : lihai
 * Creation time : 2017/12/2
 * Description   :
 *
 * Version       : 1.0
 */

/*****************************************************************************
* Included header files
*****************************************************************************/
#include "focaltech_core.h"

/*****************************************************************************
* Private constant and macro definitions using #define
*****************************************************************************/
#define SPI_BUF_LENGTH                      256
#define SPI_HEADER_LENGTH                   3
#define STATUS_PACKAGE				0x05
#define COMMAND_PACKAGE				0xC0
#define DATA_PACKAGE				0x3F
#define BUSY_QUERY_TIMEOUT				100
#define BUSY_QUERY_DELAY					150 /* unit: us */
#define CS_HIGH_DELAY				30 /* unit: us */
#define DELAY_AFTER_FIRST_BYTE		20
#define POLYNOMIAL_PARAMETER		0x8408

#define CTRL_READ_WRITE_FLAG				7
#define CTRL_CMD_CRC_BIT					6
#define CTRL_DATA_CRC_BIT				5

#define STATUS_BUSY_BIT					7

#define WRITE_CMD					0x00//0x60
#define READ_CMD					0x80//0xE0
#define READ_CMD_8756 (0x80 | DATA_CHECK_EN)
#define SPI_RETRY_NUMBER 3
#define CS_HIGH_DELAY_8756 150
#define DATA_CHECK_EN 0x20
#define SPI_DUMMY_BYTE 3
#define SPI_HEADER_LENGTH_8756 6
#define SPI_TRANSFER_MAX_LEN 1024
#define CHECK_BUF_MIN_SIZE 2
#define CHECK_BIT_SIZE 8
#define LOW_BIT_CHECK 0x01
#define PER_BIT_CHECK 0x01
#define RDATA_CHECK_CODE 0x8408
#define SPI_BUF_LENGTH_8756 2048
#define SPI_STATUS_MASK				0x81

/*****************************************************************************
* Private enumerations, structures and unions using typedef
*****************************************************************************/
struct special_cmd {
	u8 cmd;
	u16 cmd_len;
};

/*****************************************************************************
* Global variable or extern global variabls/functions
*****************************************************************************/
extern  struct ts_kit_device_data *g_focal_dev_data;

/*****************************************************************************
* Static function prototypes
*****************************************************************************/
static u8 spibuf[SPI_BUF_LENGTH] = {0};
static u8 *bus_tx_buf = NULL;
static u8 *bus_rx_buf = NULL;
static struct special_cmd special_cmd_list[] = {
	{0x55, 1},
	{0x90, 1},
	{0xAE, 6},
	{0x85, 6},
	{0xCC, 7},
	{0xCE, 1},
	{0xCD, 1},
	{0x07, 1},
	{0x08, 1},
	{0xA8, 1},
	{0xD0, 1},
	{0xDB, 1},
	{0xF2, 6},
};

/*****************************************************************************
* functions body
*****************************************************************************/
void crckermit(u8 *data, u16 len, u16 *crc_out)
{
	u16 i = 0;
	u16 j = 0;
	u16 crc = 0xFFFF;

	if ((NULL == data) || (NULL == crc_out)) {
		TS_LOG_ERR("%s: data/crc_out is NULL\n", __func__);
		return;
	}

	for( i = 0; i < len; i++)
	{
		crc ^= data[i];
		for(j = 0; j < 8; j++)
		{
			if(crc & 0x01)
			crc = (crc >> 1) ^ POLYNOMIAL_PARAMETER;
		else
			crc=(crc >> 1);
		}
	}

	*crc_out = crc;
}

/*
 * param - cmd : cmd need check
 * cmdlen - specail command length returned
 *
 * return : if is special cmd, return 1, otherwize return 0
 */
static bool fts_check_specail_cmd(u8 cmd, u32 *cmdlen)
{
	int i = 0;
	int list_len = sizeof(special_cmd_list)/sizeof(special_cmd_list[0]);

	if (NULL == cmdlen) {
		TS_LOG_ERR("%s:cmdlen is NULL\n", FTS_TAG);
		return false;
	}

	if (true == g_focal_pdata->fw_is_running) {
		TS_LOG_DEBUG("%s:fw is not running\n", FTS_TAG);
		return false;
	}

	for (i = 0; i < list_len; i++) {
		if (cmd == special_cmd_list[i].cmd) {
			*cmdlen = special_cmd_list[i].cmd_len;
			return true;
		}
	}

	return false;
}

int fts_spi_write(u8 *buf, u32 len)
{
	int ret = 0;
	struct spi_device *spi = g_focal_dev_data->ts_platform_data->spi;
	struct spi_transfer xfer[] = {
		{
			.tx_buf = &buf[0],
			.len = 1,
			.delay_usecs = DELAY_AFTER_FIRST_BYTE,
			.cs_change = 0,
			.bits_per_word = 8,
		},
		{
			.tx_buf = &buf[1],
			.len = len - 1,
		},
	};

	ret = spi_sync_transfer(spi, xfer, ARRAY_SIZE(xfer));
	if (ret) {
		TS_LOG_ERR("%s: spi_transfer(write) error, ret=%d\n",
			FTS_TAG, ret);
		return ret;
	}

	return 0;
}

int fts_spi_read(u8 *buf, u32 len)
{
	int ret = 0;
	struct spi_device *spi = g_focal_dev_data->ts_platform_data->spi;
	struct spi_transfer xfer[] = {
		{
			.tx_buf = &buf[0],
			.rx_buf = &buf[0],
	    		.len = 1,
	  		.delay_usecs = DELAY_AFTER_FIRST_BYTE,
	        	.cs_change = 0,
	    		.bits_per_word = 8,
		},
		{
			.tx_buf = &buf[1],
			.rx_buf = &buf[1],
			.len = len - 1,
		},
	};

	ret = spi_sync_transfer(spi, xfer, ARRAY_SIZE(xfer));
	if (ret) {
		TS_LOG_ERR("%s: spi_transfer(read) error, ret=%d\n",
			FTS_TAG, ret);
		return ret;
	}

	return 0;
}

static int fts_read_status(u8 *status)
{
	int ret = 0;
	u8 status_cmd[2] = {STATUS_PACKAGE, 0xFF};
	struct spi_device *spi = g_focal_dev_data->ts_platform_data->spi;
	struct spi_message msg;
	struct spi_transfer xfer = {
	.tx_buf = status_cmd,
		.rx_buf = status_cmd,
		.len = 2,
	};

	if (NULL == status) {
		TS_LOG_ERR("%s:status is NULL\n", FTS_TAG);
		return -EINVAL;
	}

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	ret = spi_sync(spi, &msg);
	if (ret) {
		TS_LOG_ERR("%s: spi_transfer(read) error, ret=%d\n",
			FTS_TAG, ret);
		return ret;
	}

	*status = status_cmd[1];
	return ret;
}

static int fts_wait_idle(void)
{
	int ret = 0;
	int i = 0;
	u8 status = 0xFF;

	for (i = 0; i < BUSY_QUERY_TIMEOUT; i++) {
		udelay(BUSY_QUERY_DELAY);

		ret = fts_read_status(&status);
		if (ret >= 0) {
			status &= SPI_STATUS_MASK;
			if ((g_focal_pdata->fw_is_running && (0x01 == status))
					|| (!g_focal_pdata->fw_is_running && (0x00 == status))) {
				break;
			} else {
				TS_LOG_DEBUG("fw:%d,spi_st:%x", g_focal_pdata->fw_is_running, (int)status);
			}
		}
	}

	if (i >= BUSY_QUERY_TIMEOUT) {
		TS_LOG_ERR("%s:spi is busy\n", FTS_TAG);
		return -EIO;
	}

	udelay(CS_HIGH_DELAY);
	return (int)status;
}

static int fts_cmd_wirte(u8 ctrl, u8 *cmd, u8 len)
{
	int ret = 0;
	int i = 0;
	int pos = 0;
	u8 buf[MAX_COMMAND_LENGTH] = { 0 };

	if ((len <= 0) || (len >= MAX_COMMAND_LENGTH - 4)) {
		TS_LOG_ERR("%s:command length:%u fail\n", FTS_TAG, len);
		return -EINVAL;
	}

	if (NULL == cmd) {
		TS_LOG_ERR("%s:command is NULL\n", FTS_TAG);
		return -EINVAL;
	}

	buf[pos++] = COMMAND_PACKAGE;
	buf[pos++] = ctrl | (len & 0x0F);
	for (i = 0; i < len; i++) {
		buf[pos++] = cmd[i];
	}

	ret = fts_spi_write(buf, pos);

	return ret;
}

static int fts_boot_write(u8 *cmd, u8 cmdlen, u8 *data, u32 datalen)
{
	int ret = 0;
	u16 crc = 0;
	u8 *txbuf = NULL;
	u32 txlen = 0;
	u8 ctrl = WRITE_CMD;

	if ((!cmd) || (0 == cmdlen) || (datalen > FTS_PACKAGE_SIZE_SPI)) {
		TS_LOG_ERR("%s parameter is invalid\n", FTS_TAG);
		return -EINVAL;
	}

	mutex_lock(&g_focal_pdata->spilock);

	/* wait spi idle */
	ret = fts_wait_idle();
	if (ret < 0) {
		TS_LOG_ERR("%s:wait spi idle fail\n", FTS_TAG);
		goto err_boot_write;
	}

	/* write cmd */
	if ((NULL == data) || (0 == datalen)) {
		ctrl &= ~BIT(CTRL_DATA_CRC_BIT);
	}

	ret = fts_cmd_wirte(ctrl, cmd, cmdlen);
	if (ret < 0) {
		TS_LOG_ERR("%s:command package write fail\n", FTS_TAG);
		goto err_boot_write;
	}

	if (data && datalen) {
		/* wait spi idle */
		ret = fts_wait_idle();
		if (ret < 0) {
			TS_LOG_ERR("%s:wait spi idle from cmd packet fail\n",
				FTS_TAG);
			goto err_boot_write;
		}

		/* write data */
		if (datalen > SPI_BUF_LENGTH - SPI_HEADER_LENGTH) {
			txbuf = kzalloc(datalen + SPI_HEADER_LENGTH, GFP_KERNEL);
			if (NULL == txbuf) {
				TS_LOG_ERR("%s:txbuf kzalloc fail\n", FTS_TAG);
				ret = -ENOMEM;
				goto err_boot_write;
			}
		} else {
			txbuf = spibuf;
		}
		memset(txbuf, 0xFF, datalen + SPI_HEADER_LENGTH);
		txbuf[0] = DATA_PACKAGE;
		memcpy(txbuf + 1, data, datalen);
		txlen = datalen + 1;
		if (ctrl & BIT(CTRL_DATA_CRC_BIT)) {
			crckermit(txbuf, txlen, &crc);
			txbuf[txlen++] = crc & 0xFF;
			txbuf[txlen++] = (crc >> 8) & 0xFF;
		}
		ret = fts_spi_write(txbuf, txlen);
		if (ret < 0) {
			TS_LOG_ERR("%s:data write fail\n", FTS_TAG);
		}

		if ((txbuf) && (datalen > SPI_BUF_LENGTH - SPI_HEADER_LENGTH)) {
			kfree(txbuf);
			txbuf = NULL;
		}
	}
err_boot_write:
	mutex_unlock(&g_focal_pdata->spilock);
	return ret;
}

static int fts_fw_write(u8 addr, u8 *data, u32 datalen)
{
	u8 cmd[3] = { 0 };

	cmd[0] = addr;
	cmd[1] = (datalen >> 8) & 0xFF;
	cmd[2] = datalen & 0xFF;

	return fts_boot_write(cmd, 3, data, datalen);
}

int fts_write(u8 *writebuf, u32 writelen)
{
	int ret = 0;
	u32 cmdlen = 0;

	if ((NULL == writebuf) || (0 == writelen)) {
		TS_LOG_ERR("%s writebuf is null/writelen is 0\n", FTS_TAG);
		return -EINVAL;
	}

	ret = fts_check_specail_cmd(writebuf[0], &cmdlen);
	if (0 == ret) {
		TS_LOG_DEBUG("fw cmd");
		ret = fts_fw_write(writebuf[0], writebuf + 1, writelen - 1);
	} else {
		TS_LOG_DEBUG("boot cmd");
		if (cmdlen == writelen)
			ret = fts_boot_write(writebuf, cmdlen, NULL, 0);
		else
			ret = fts_boot_write(writebuf, cmdlen, writebuf + cmdlen, writelen - cmdlen);
	}

	return ret;
}

static int fts_boot_read(u8 *cmd, u8 cmdlen, u8 *data, u32 datalen)
{
	int ret = 0;
	u8 ctrl = READ_CMD;
	u8 *txbuf = NULL;
	u32 txlen = 0;

	if ((!data) || (0 == datalen) || (datalen > FTS_PACKAGE_SIZE_SPI)) {
		TS_LOG_ERR("%s parameter is invalid\n", FTS_TAG);
		return -EINVAL;
	}

	mutex_lock(&g_focal_pdata->spilock);

	if (cmd && cmdlen) {
		/* wait spi idle */
		ret = fts_wait_idle();
		if (ret < 0) {
			TS_LOG_ERR("%s:wait spi idle fail\n", FTS_TAG);
			goto boot_read_err;
		}

		/* write cmd */
		ret = fts_cmd_wirte(ctrl, cmd, cmdlen);
		if (ret < 0) {
			TS_LOG_ERR("%s:command package write fail\n", FTS_TAG);
			goto boot_read_err;
		}

		/* wait spi idle */
		ret = fts_wait_idle();
		if (ret < 0) {
			TS_LOG_ERR("%s:wait spi idle from cmd packet fail\n",
				FTS_TAG);
			goto boot_read_err;
		}
	}
	/* write data */
	if (datalen > SPI_BUF_LENGTH - SPI_HEADER_LENGTH) {
		txbuf = kzalloc(datalen + SPI_HEADER_LENGTH, GFP_KERNEL);
		if (NULL == txbuf) {
			TS_LOG_ERR("%s:txbuf kzalloc fail\n", FTS_TAG);
			ret =  -ENOMEM;
			goto boot_read_err;
		}
	} else {
		txbuf = spibuf;
	}
	memset(txbuf, 0xFF, datalen + SPI_HEADER_LENGTH);
	txbuf[0] = DATA_PACKAGE;
	txlen = datalen + 1;
	ret = fts_spi_read(txbuf, txlen);
	if (ret < 0) {
		TS_LOG_ERR("%s:data write fail\n", FTS_TAG);
		goto boot_read_err;
	}

	memcpy(data, txbuf + 1, datalen);
boot_read_err:
	if ((txbuf) && (datalen > SPI_BUF_LENGTH - SPI_HEADER_LENGTH)) {
		kfree(txbuf);
		txbuf = NULL;
	}
	mutex_unlock(&g_focal_pdata->spilock);
	return ret;
}

static int fts_fw_read(u8 addr, u8 *data, u32 datalen)
{
	u8 cmd[3] = { 0 };

	cmd[0] = addr;
	cmd[1] = (datalen >> 8) & 0xFF;
	cmd[2] = datalen & 0xFF;

	return fts_boot_read(cmd, 3, data, datalen);
}

int fts_read(u8 *writebuf, u32 writelen, u8 *readbuf, u32 readlen)
{
	int ret = 0;
	u32 cmdlen = 0;

	if ((NULL == readbuf) || (0 == readlen)) {
		TS_LOG_ERR("readbuf/readlen is invalid in fts_read\n");
		return -EINVAL;
	}

	if ((NULL == writebuf) || (0 == writelen)) {
		ret = fts_boot_read(NULL, 0, readbuf, readlen);
	} else {
		ret = fts_check_specail_cmd(writebuf[0], &cmdlen);
		if (0 == ret) {
			TS_LOG_DEBUG("fw cmd");
			ret = fts_fw_read(writebuf[0], readbuf, readlen);
		} else {
			TS_LOG_DEBUG("boot cmd");
			ret = fts_boot_read(writebuf, cmdlen, readbuf, readlen);
		}
	}

	return ret;
}

static int fts_spi_transfer(u8 *tx_buf, u8 *rx_buf, u32 len)
{
	int ret;
	struct spi_device *spi = g_focal_dev_data->ts_platform_data->spi;
	struct spi_message msg;
	struct spi_transfer xfer[2];
	u32 packet_length;
	u32 packet_remainder = 0;

	if (!spi || !tx_buf || !rx_buf || !len) {
		TS_LOG_ERR("spi or tx_buf-rx_buf-len is invalid\n");
		return -EINVAL;
	}

	memset(&xfer[0], 0, sizeof(struct spi_transfer));
	memset(&xfer[1], 0, sizeof(struct spi_transfer));

	spi_message_init(&msg);
	if (len > SPI_TRANSFER_MAX_LEN)
		packet_remainder = len % SPI_TRANSFER_MAX_LEN;
	packet_length = len - packet_remainder;
	xfer[0].tx_buf = &tx_buf[0];
	xfer[0].rx_buf = &rx_buf[0];
	xfer[0].len = packet_length;
	spi_message_add_tail(&xfer[0], &msg);

	if (packet_remainder) {
		xfer[1].tx_buf = &tx_buf[packet_length];
		xfer[1].rx_buf = &rx_buf[packet_length];
		xfer[1].len = packet_remainder;
		spi_message_add_tail(&xfer[1], &msg);
	}
	mutex_lock(&g_focal_pdata->spilock);
	ret = spi_sync(spi, &msg);
	mutex_unlock(&g_focal_pdata->spilock);
	if (ret) {
		TS_LOG_ERR("spi_sync fail,ret:%d", ret);
		return ret;
	}

	return ret;
}

static int rdata_check_8756(u8 *rdata, u32 rlen)
{
	u16 i;
	u16 j;
	u16 value_read;
	u16 value_calc = 0xFFFF;

	if (rdata == NULL) {
		TS_LOG_ERR("rdata_check: rdata is NULL\n");
		return -EIO;
	}
	if (rlen < CHECK_BUF_MIN_SIZE) {
		TS_LOG_ERR("rdata_check: rdata_check buf less than 2: %d\n",
			rlen);
		return -EIO;
	}
	for (i = 0; i < (rlen - CHECK_BUF_MIN_SIZE); i++) {
		value_calc ^= rdata[i];
		for (j = 0; j < CHECK_BIT_SIZE; j++) {
			if (value_calc & LOW_BIT_CHECK)
				value_calc = (value_calc >> PER_BIT_CHECK) ^
					RDATA_CHECK_CODE;
			else
				value_calc = (value_calc >> PER_BIT_CHECK);
		}
	}
	/* rlen - 1 is to get the last element of rdata array */
	value_read = (u16)(rdata[rlen - 1] << CHECK_BIT_SIZE) +
		rdata[rlen - CHECK_BUF_MIN_SIZE];
	if (value_calc != value_read) {
		TS_LOG_ERR("rdata_check: ecc fail:cal %x != buf %x\n",
			value_calc, value_read);
		return -EIO;
	}
	return 0;
}

int fts_write_8756(u8 *writebuf, u32 writelen)
{
	int ret;
	int i;
	u8 *txbuf = NULL;
	u8 *rxbuf = NULL;
	u32 txlen = 0;
	u32 txlen_need = writelen + SPI_HEADER_LENGTH_8756 + SPI_DUMMY_BYTE;
	u32 datalen = writelen - 1;

	if (!writebuf || !writelen) {
		TS_LOG_ERR("writebuf or len is invalid\n");
		return -EINVAL;
	}
	if (txlen_need > SPI_BUF_LENGTH_8756) {
		txbuf = kzalloc(txlen_need, GFP_KERNEL);
		rxbuf = kzalloc(txlen_need, GFP_KERNEL);
		if (!txbuf || !rxbuf) {
			TS_LOG_ERR("txbuf or rxbuf malloc fail\n");
			ret = -ENOMEM;
			goto err_write;
		}
	} else {
		txbuf = bus_tx_buf;
		rxbuf = bus_rx_buf;
		memset(txbuf, 0x0, SPI_BUF_LENGTH_8756);
		memset(rxbuf, 0x0, SPI_BUF_LENGTH_8756);
	}

	txbuf[txlen++] = writebuf[0];
	txbuf[txlen++] = WRITE_CMD;
	txbuf[txlen++] = (datalen >> 8) & 0xFF;
	txbuf[txlen++] = datalen & 0xFF;
	if (datalen > 0) {
		txlen = txlen + SPI_DUMMY_BYTE;
		memcpy(&txbuf[txlen], &writebuf[1], datalen);
		txlen = txlen + datalen;
	}

	for (i = 0; i < SPI_RETRY_NUMBER; i++) {
		ret = fts_spi_transfer(txbuf, rxbuf, txlen);
		if ((ret == 0) && ((rxbuf[3] & 0xA0) == 0)) {
			break;
		} else {
			TS_LOG_ERR("data write status=%x,retry=%d,ret=%d\n",
				rxbuf[3], i, ret);
			ret = -EIO;
			udelay(CS_HIGH_DELAY_8756);
		}
	}
err_write:
	if (txlen_need > SPI_BUF_LENGTH_8756) {
		kfree(txbuf);
		txbuf = NULL;
		kfree(rxbuf);
		rxbuf = NULL;
	}
	udelay(CS_HIGH_DELAY);
	return ret;
}

int fts_read_8756(u8 *cmd, u32 cmdlen, u8 *data, u32 datalen)
{
	int ret = 0;
	int i;
	u8 *txbuf = NULL;
	u8 *rxbuf = NULL;
	u32 txlen = 0;
	u32 txlen_need = datalen + SPI_HEADER_LENGTH_8756 + SPI_DUMMY_BYTE;
	u32 dp = 0;

	if (!cmd || !cmdlen || !data || !datalen) {
		TS_LOG_ERR("cmd/cmdlen/data/datalen is invalid\n");
		return -EINVAL;
	}
	if (txlen_need > SPI_BUF_LENGTH_8756) {
		txbuf = kzalloc(txlen_need, GFP_KERNEL);
		rxbuf = kzalloc(txlen_need, GFP_KERNEL);
		if (!txbuf || !rxbuf) {
			TS_LOG_ERR("txbuf or rxbuf malloc fail\n");
			ret = -ENOMEM;
			goto err_read;
		}
	} else {
		txbuf = bus_tx_buf;
		rxbuf = bus_rx_buf;
		memset(txbuf, 0x0, SPI_BUF_LENGTH_8756);
		memset(rxbuf, 0x0, SPI_BUF_LENGTH_8756);
	}

	txbuf[txlen++] = cmd[0];
	txbuf[txlen++] = READ_CMD_8756;
	txbuf[txlen++] = (datalen >> 8) & 0xFF;
	txbuf[txlen++] = datalen & 0xFF;
	dp = txlen + SPI_DUMMY_BYTE;
	txlen = dp + datalen;
	txlen = txlen + 2;

	for (i = 0; i < SPI_RETRY_NUMBER; i++) {
		ret = fts_spi_transfer(txbuf, rxbuf, txlen);
		if ((ret == 0) && ((rxbuf[3] & 0xA0) == 0)) {
			memcpy(data, &rxbuf[dp], datalen);
			ret = rdata_check_8756(&rxbuf[dp], txlen - dp);
			if (ret < 0) {
				TS_LOG_ERR("data read addr:%x crc abnormal,retry:%d\n",
					cmd[0], i);
				udelay(CS_HIGH_DELAY_8756);
				continue;
			}
			break;
		} else {
			TS_LOG_ERR("data read addr:%x status:%x,retry:%d,ret:%d\n",
					cmd[0], rxbuf[3], i, ret);
			ret = -EIO;
			udelay(CS_HIGH_DELAY);
		}
	}
	if (ret < 0)
		TS_LOG_ERR("data read addr:%x status:%s,retry:%d,ret:%d\n",
			cmd[0], i >= SPI_RETRY_NUMBER? "err":"fail", i, ret);
err_read:
	if (txlen_need > SPI_BUF_LENGTH_8756) {
		kfree(txbuf);
		txbuf = NULL;
		kfree(rxbuf);
		rxbuf = NULL;
	}
	udelay(CS_HIGH_DELAY);
	return ret;
}

int fts_bus_init(void)
{
	bus_tx_buf = kzalloc(SPI_BUF_LENGTH_8756, GFP_KERNEL);
	if (NULL == bus_tx_buf) {
		TS_LOG_ERR("failed to allocate memory for bus_tx_buf");
		return -ENOMEM;
	}

	bus_rx_buf = kzalloc(SPI_BUF_LENGTH_8756, GFP_KERNEL);
	if (NULL == bus_rx_buf) {
		TS_LOG_ERR("failed to allocate memory for bus_rx_buf");
		return -ENOMEM;
	}
	return 0;
}

void fts_bus_exit(void)
{
	if (bus_tx_buf) {
		kfree(bus_tx_buf);
		bus_tx_buf = NULL;
	}

	if (bus_rx_buf) {
		kfree(bus_rx_buf);
		bus_rx_buf = NULL;
	}
}
