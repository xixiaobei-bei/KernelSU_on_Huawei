#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/string.h>
#include <linux/spi/spi.h>
#include "huawei_thp.h"

#include <linux/time.h>
#include <linux/syscalls.h>

#define SSL_IC_NAME "ssl_thp"
#define THP_SSL_DEV_NODE_NAME "ssl_thp"

#define MXT680U2_FAMILY_ID  166
#define MXT680U2_VARIANT_ID  22

/******ternence 06 21***/
#define MXT_WAKEUP_TIME 10

// opcodes
#define SPI_WRITE_REQ    0x01
#define SPI_WRITE_OK     0x81
#define SPI_WRITE_FAIL   0x41
#define SPI_READ_REQ     0x02
#define SPI_READ_OK      0x82
#define SPI_READ_FAIL    0x42
#define SPI_INVALID_REQ  0x04
#define SPI_INVALID_CRC  0x08

#define SPI_APP_DATA_MAX_LEN  64
#define SPI_APP_HEADER_LEN     6

#define SPI_BOOTL_HEADER_LEN   2

#define T117_BYTES_READ_LIMIT    1505	// 7 x 215 (T117 size)
#define SPI_APP_BUF_SIZE_WRITE  (SPI_APP_HEADER_LEN+SPI_APP_DATA_MAX_LEN)
#define SPI_APP_BUF_SIZE_READ   (SPI_APP_HEADER_LEN+T117_BYTES_READ_LIMIT)

struct mxt_info {
	u8 family_id;
	u8 variant_id;
	u8 version;
	u8 build;
	u8 matrix_xsize;
	u8 matrix_ysize;
	u8 object_num;
} __packed;;

struct mxt_object {
	u8 type;
	u16 start_address;
	u8 size_minus_one;
	u8 instances_minus_one;
	u8 num_report_ids;
} __packed;

#define MXT_OBJECT_START     0x07	// after struct mxt_info
#define MXT_INFO_CHECKSUM_SIZE  3	// after list of struct mxt_object

static u8 *spi_tx_buf = NULL;
static u8 *spi_rx_buf = NULL;
static u8 *spi_tx_dummy_buf = NULL;
static void *raw_info_block = NULL;
static uint16_t T117_address = 0;

static u8 get_crc8_iter(u8 crc, u8 data)
{
	static const u8 crcpoly = 0x8c;
	u8 index = 8;
	u8 fb;
	do {
		fb = (crc ^ data) & 0x01;
		data >>= 1;
		crc >>= 1;
		if (fb) {
			crc ^= crcpoly;
		}
	} while (--index);
	return crc;
}

static u8 get_header_crc(u8 * p_msg)
{
	u8 calc_crc = 0;
	int i = 0;
	for (; i < SPI_APP_HEADER_LEN - 1; i++) {
		calc_crc = get_crc8_iter(calc_crc, p_msg[i]);
	}
	return calc_crc;
}

static void spi_prepare_header(u8 * header,
			       u8 opcode, u16 start_register, u16 count)
{
	header[0] = opcode;
	header[1] = start_register & 0xff;
	header[2] = start_register >> 8;
	header[3] = count & 0xff;
	header[4] = count >> 8;
	header[5] = get_header_crc(header);
}

static int __mxt_read_reg(struct spi_device *client, u16 start_register,
			  u16 len, u8 * val)
{
	u8 attempt = 0;
	int ret_val;
	int i=0;

	struct thp_core_data *cd = spi_get_drvdata(client);
	struct thp_device *tdev = cd->thp_dev;
	struct spi_message spimsg[2];
	struct spi_transfer spitr;
	do {
		attempt++;
		if (attempt > 1) {
			if (attempt > 3) {
				THP_LOG_ERR("%s: Too many Retries\n", __func__);
				return -EIO;
			}
			THP_LOG_INFO("%s: Retry %d\n", __func__, attempt - 1);
			msleep(MXT_WAKEUP_TIME);
		}
		/* WRITE SPI_READ_REQ */
		spi_prepare_header(spi_tx_buf, SPI_READ_REQ, start_register,
				   len);
		spi_message_init(&spimsg[0]);
		memset(&spitr, 0, sizeof(struct spi_transfer));
		spitr.tx_buf = spi_tx_buf;
		spitr.rx_buf = spi_rx_buf;
		spitr.len = SPI_APP_HEADER_LEN;
		spi_message_add_tail(&spitr, &spimsg[0]);

		thp_bus_lock();
		ret_val = thp_spi_sync(client, &spimsg[0]);
		thp_bus_unlock();

		if (ret_val < 0) {
			THP_LOG_ERR("%s: Error writing to spi\n", __func__);
			return ret_val;
		}
		mdelay(1);
		if (0 != gpio_get_value(cd->gpios.irq_gpio))	// check CHG line low
		{
			THP_LOG_ERR("Timeout on CHG");
		}

		/* READ SPI_READ_OK */
		spi_message_init(&spimsg[1]);
		memset(&spitr, 0, sizeof(struct spi_transfer));
		spitr.tx_buf = spi_tx_dummy_buf;
		spitr.rx_buf = spi_rx_buf;
		spitr.len = SPI_APP_HEADER_LEN + len;
		spi_message_add_tail(&spitr, &spimsg[1]);

		thp_bus_lock();
		ret_val = thp_spi_sync(client, &spimsg[1]);
		thp_bus_unlock();

		if (ret_val < 0) {
			THP_LOG_ERR("%s: Error reading from spi (%d)\n",
				    __func__, ret_val);
			return ret_val;
		}
		if (SPI_READ_OK != spi_rx_buf[0]) {
			THP_LOG_ERR
			    ("%s: SPI_READ_OK != 0x%02X reading from spi\n",
			     __func__, spi_rx_buf[0]);
			return -1;
		}
		if (spi_tx_buf[1] != spi_rx_buf[1]
		    || spi_tx_buf[2] != spi_rx_buf[2]) {
			THP_LOG_ERR ("%s: Unexpected address %d != %d reading from spi\n",
			     __func__, spi_rx_buf[1] | (spi_rx_buf[2] << 8), start_register);
			return -1;
		}
		if (spi_tx_buf[3] != spi_rx_buf[3]
		    || spi_tx_buf[4] != spi_rx_buf[4]) {
			THP_LOG_ERR ("%s: Unexpected count %d != %d reading from spi\n",
			     __func__, spi_rx_buf[3] | (spi_rx_buf[4] << 8), len);
			return -1;
		}
	}
	while (get_header_crc(spi_rx_buf) !=
	       spi_rx_buf[SPI_APP_HEADER_LEN - 1]);

	memcpy(val, spi_rx_buf + SPI_APP_HEADER_LEN, len);
	return 0;
}

static int mxt_read_blks(struct spi_device *client, u16 start, u16 count,
			 u8 * buf, u16 override_limit)
{
	u16 offset = 0;
	int ret_val = 0;
	u16 size;

	while (offset < count) {
		if (0 == override_limit) {
			size = min(SPI_APP_DATA_MAX_LEN, count - offset);
		} else {
			size = min(override_limit, count - offset);
		}

		ret_val = __mxt_read_reg(client,
					 start + offset, size, buf + offset);
		if (ret_val) {
			break;
		}

		offset += size;
	}

	return ret_val;
}

static int thp_ssl_init(struct thp_device *tdev)
{
	int rc;
	struct thp_core_data *cd = tdev->thp_core;
	struct device_node *ssl_node = of_get_child_by_name(cd->thp_node,
							    THP_SSL_DEV_NODE_NAME);

	THP_LOG_INFO("%s: called\n", __func__);

	if (!ssl_node) {
		THP_LOG_INFO("%s: syna dev not config in dts\n", __func__);
		return -ENODEV;
	}

	if (NULL == spi_tx_buf) {
		spi_tx_buf = kzalloc(SPI_APP_BUF_SIZE_WRITE, GFP_KERNEL);
		if (!spi_tx_buf){
			THP_LOG_ERR("%s: out of memory\n", __func__);
			return -ENOMEM;
		}
	}
	if (NULL == spi_rx_buf) {
		spi_rx_buf = kzalloc(SPI_APP_BUF_SIZE_READ, GFP_KERNEL);
		if (!spi_rx_buf){
			THP_LOG_ERR("%s: out of memory\n", __func__);
			return -ENOMEM;
		}
	}
	if (NULL == spi_tx_dummy_buf) {
		spi_tx_dummy_buf = kzalloc(SPI_APP_BUF_SIZE_READ, GFP_KERNEL);
		if (!spi_tx_dummy_buf){
			THP_LOG_ERR("%s: out of memory\n", __func__);
			return -ENOMEM;
		}
		memset(spi_tx_dummy_buf, 0xff, SPI_APP_BUF_SIZE_READ);
	}

	rc = thp_parse_spi_config(ssl_node, cd);
	if (rc)
		THP_LOG_ERR("%s: spi config parse fail\n", __func__);

	rc = thp_parse_timing_config(ssl_node, &tdev->timing_config);
	if (rc)
		THP_LOG_ERR("%s: timing config parse fail\n", __func__);

	rc = thp_parse_feature_config(ssl_node, cd);
	if (rc)
		THP_LOG_ERR("%s: feature_config fail\n", __func__);

	return 0;
}

static void thp_ssl_reset(struct thp_device *tdev)
{
	thp_do_time_delay(20);
	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);

	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
	thp_do_time_delay(tdev->timing_config.boot_reset_low_delay_ms);

	gpio_direction_output(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
}
static int ssl_power_init(struct thp_device *tdev)
{
	int ret_val;

	ret_val = thp_power_supply_get(THP_VCC);
	ret_val |= thp_power_supply_get(THP_IOVDD);
	if (ret_val) {
		THP_LOG_ERR("%s: fail to get power\n", __func__);
	}

	/* workaround for V4 branch to disable ldo29 first */
	thp_power_supply_ctrl(THP_IOVDD, THP_POWER_ON, 0);
	thp_power_supply_ctrl(THP_IOVDD, THP_POWER_OFF, 1);
	return 0;
}
static void ssl_power_release(struct thp_device *tdev)
{
	thp_power_supply_put(THP_VCC);
	thp_power_supply_put(THP_IOVDD);
}
static int ssl_power_on(struct thp_device *tdev)
{
	int ret_val;

	ret_val = thp_power_supply_ctrl(THP_VCC, THP_POWER_ON, 0);
	ret_val |= thp_power_supply_ctrl(THP_IOVDD, THP_POWER_ON, 0);
	if (ret_val) {
		THP_LOG_ERR("%s:power ctrl fail\n", __func__);
		return ret_val;
	}

	thp_ssl_reset(tdev);

	return 0;
}
static int ssl_power_off(struct thp_device *tdev)
{
	int ret_val;

	ret_val = thp_power_supply_ctrl(THP_VCC, THP_POWER_OFF, 0);
	ret_val |= thp_power_supply_ctrl(THP_IOVDD, THP_POWER_ON, 0);
	if (ret_val) {
		THP_LOG_ERR("%s:power ctrl fail\n", __func__);
		return ret_val;
	}

	return 0;
}

static int ssl_chip_info_check(struct thp_device *tdev)
{
	int ret_val, i;
	struct mxt_info mxtinfo;
	u8 *buff = 0;
	size_t curr_size;
	struct mxt_object *object_table;

	/* check ic family_id and variant_id*/
	curr_size = MXT_OBJECT_START;
	ret_val = mxt_read_blks(tdev->thp_core->sdev, 0, curr_size, &mxtinfo, 0);
	if (ret_val) {
		THP_LOG_ERR("%s:read chip info fail\n", __func__);
		return -ENODEV;
	}

	if (mxtinfo.family_id != MXT680U2_FAMILY_ID
	    || mxtinfo.variant_id != MXT680U2_VARIANT_ID) {
		THP_LOG_ERR("%s: chip is not identified (%d, %d)\n", __func__,
			    mxtinfo.family_id, mxtinfo.variant_id);
		return -ENODEV;
	}
	THP_LOG_INFO("%s:chip info check succ\n", __func__);

	/* read T117 address for get frame */
	curr_size +=  mxtinfo.object_num * sizeof(struct mxt_object) +
			    MXT_INFO_CHECKSUM_SIZE;
	buff = (u8 *) kzalloc(curr_size, GFP_KERNEL);
	if (!buff) {
		THP_LOG_ERR("%s: out of memory\n", __func__);
		return -ENOMEM;
	}
	ret_val = mxt_read_blks(tdev->thp_core->sdev,
				MXT_OBJECT_START,
				curr_size - MXT_OBJECT_START,
				buff + MXT_OBJECT_START,
				0 /*override_limit */ );
	if (ret_val) {
		THP_LOG_ERR("%s: mxt_read_blks objs read fail\n", __func__);
		kfree(buff);
		return -EIO;
	}

	object_table = (struct mxt_object *)(buff + MXT_OBJECT_START);
	for (i = 0; i < mxtinfo.object_num; i++) {
		struct mxt_object *object = object_table + i;
		// object start_address from little endian to local CPU endianness **IN PLACE**
		// IMPORTANT: this is only for the first loop through the object table
		le16_to_cpus(&object->start_address);
		if (117 == object->type) {
			T117_address = object->start_address;
			THP_LOG_INFO("%s:T117_address = 0x%x\n", __func__, T117_address);
			break;
		}
	}

	if (0 == T117_address) {
		THP_LOG_ERR("%s:T117_address not found\n", __func__);
		ret_val = -ENODEV;
	}

	kfree(buff);
	return ret_val;
}

static int thp_ssl_chip_detect(struct thp_device *tdev)
{
	int ret_val = 0;

	THP_LOG_INFO("%s: called\n", __func__);
	ssl_power_init(tdev);
	if (ssl_power_on(tdev)) {
		THP_LOG_ERR("%s: power on failed\n", __func__);
	}

	ret_val = ssl_chip_info_check(tdev);
	if (ret_val) {
		THP_LOG_ERR("%s: ic info check failed\n", __func__);
		ssl_power_off(tdev);
		ssl_power_release(tdev);
	}

	return ret_val;
}

static int thp_ssl_get_frame(struct thp_device *tdev, char *buf,
			     unsigned int len)
{
	uint8_t *w_buf;
	int ret_val;

	if (!tdev) {
		THP_LOG_INFO("%s: input dev null\n", __func__);
		return -ENOMEM;
	}

	if (!len) {
		THP_LOG_INFO("%s: read len illegal\n", __func__);
		return -ENOMEM;
	}

	w_buf = tdev->tx_buff;

	memset(tdev->tx_buff, 0, THP_MAX_FRAME_SIZE);

	ret_val =
	    mxt_read_blks(tdev->thp_core->sdev, T117_address, len, buf,
			  T117_BYTES_READ_LIMIT);
	return ret_val;
}

static int thp_ssl_resume(struct thp_device *tdev)
{
	THP_LOG_INFO("%s: called\n", __func__);
	//TODO:complete resume function
	//gpio_set_value(tdev->gpios->cs_gpio, 1);   //check??
	//gpio_set_value(tdev->gpios->rst_gpio, 1);//keep TP rst  high before LCD  reset hign

	return 0;
}

static int thp_ssl_suspend(struct thp_device *tdev)
{
	THP_LOG_INFO("%s: called\n", __func__);
	//TODO:complete suspend function
	//gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
	//gpio_set_value(tdev->gpios->cs_gpio, 0);  //check?? active high?

	return 0;
}

static void thp_ssl_exit(struct thp_device *tdev)
{
	THP_LOG_INFO("%s: called\n", __func__);
	if (tdev) {
		if (tdev->tx_buff) {
			kfree(tdev->tx_buff);
			tdev->tx_buff = NULL;
		}
		if (tdev->rx_buff) {
			kfree(tdev->rx_buff);
			tdev->rx_buff = NULL;
		}
		kfree(tdev);
		tdev = NULL;
	}
	if (NULL != spi_tx_buf) {
		kfree(spi_tx_buf);
	}
	if (NULL != spi_rx_buf) {
		kfree(spi_rx_buf);
	}
	if (NULL != spi_tx_dummy_buf) {
		kfree(spi_tx_dummy_buf);
	}
	if (NULL != raw_info_block) {
		kfree(raw_info_block);
	}
}

struct thp_device_ops ssl_dev_ops = {
	.init = thp_ssl_init,
	.detect = thp_ssl_chip_detect,
	.get_frame = thp_ssl_get_frame,
	.resume = thp_ssl_resume,
	.suspend = thp_ssl_suspend,
	.exit = thp_ssl_exit,
};

static int __init thp_ssl_module_init(void)
{
	int rc;
	struct thp_device *dev;

	THP_LOG_INFO("%s: called \n", __func__);
	dev = kzalloc(sizeof(struct thp_device), GFP_KERNEL);
	if (!dev) {
		THP_LOG_ERR("%s: thp device malloc fail\n", __func__);
		return -ENOMEM;
	}

	dev->tx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	dev->rx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if (!dev->tx_buff || !dev->rx_buff) {
		THP_LOG_ERR("%s: out of memory\n", __func__);
		rc = -ENOMEM;
		goto err;
	}

	dev->ic_name = SSL_IC_NAME;
	dev->ops = &ssl_dev_ops;

	rc = thp_register_dev(dev);
	if (rc) {
		THP_LOG_ERR("%s: register fail\n", __func__);
		goto err;
	} else
		THP_LOG_INFO("%s: register success\n", __func__);

	return rc;
 err:
	thp_ssl_exit(dev);
	return rc;
}

static void __exit thp_ssl_module_exit(void)
{
	THP_LOG_ERR("%s: called \n", __func__);
};

module_init(thp_ssl_module_init);
module_exit(thp_ssl_module_exit);
