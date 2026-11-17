/*
 * Thp driver code for bu21150
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

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
#include <asm/byteorder.h>
#include "huawei_thp.h"
#include <lcdkit_tp.h>

#define THP_BU21150_DEV_NODE_NAME "rohm-bu21150"
#define BU21150_IC_NAME "rohm"

#define SPI_HEADER_SIZE 3
#define SPI_BITS_PER_WORD_READ 8
#define SPI_BITS_PER_WORD_WRITE 8
#define COMM_TEST_REG 0x0010
#define COMM_TEST_RW_LENGTH 8
#define COMM_TEST_STR "HOST_SPI"
#define REG_SIZE 2
#define COMM_TEST_RW_RETRY_TIME 3
#define COMM_TEST_RW_RETRY_DELAY_MS 50
#define FRAME_DATA_REG 0x0400
#define IRQ_SETTINGS_REG 0x007E
#define INT_RUN_ENB_REG 0x00CE
#define SENS_START_REG 0x0086
#define OTP_ADDR_REG 0x0088
#define OTP_ADDR_RECALL_REG 0x0098
#define OTP_SIZE 8
#define MULTI_FACTOR 2
#define COMM_INDEX2 2

static int touch_driver_otp_read(char *otp_data);
__attribute__((weak)) void lcd_huawei_thp_register(
	struct tp_thp_device_ops *tp_thp_device_ops);
struct tp_thp_device_ops ts_thp_ops = {
	.thp_otp_read = touch_driver_otp_read,
};

static void swap_2byte(unsigned char *buf, unsigned int size)
{
	unsigned int i;

	if ((size % REG_SIZE) == 1) {
		thp_log_err("%s:error size is odd.size=[%u]\n", __func__, size);
		return;
	}

	for (i = 0; i < size; i += REG_SIZE)
		be16_to_cpus((__u16 *)(buf + i));
}

static int touch_driver_spi_transfer(struct thp_device *tdev,
	struct spi_transfer *xfer, struct spi_message *msg)
{
	int rc;
	struct spi_device *sdev = tdev->sdev;

	thp_log_debug("%s called\n", __func__);

	spi_message_add_tail(xfer, msg);
	rc = thp_bus_lock();
	if (rc < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return -EINVAL;
	}
	rc = thp_spi_sync(sdev, msg);
	thp_bus_unlock();

	return rc;
}

static int touch_driver_read_register_(struct thp_device *tdev,
	u32 addr, u16 size, u8 *data)
{
	struct spi_transfer t;
	struct spi_message msg;
	int rc;
	char *tx_buff = tdev->tx_buff;
	char *rx_buff = tdev->rx_buff;

	thp_log_debug("%s: called\n", __func__);

#if defined(CONFIG_TEE_TUI)
	if (tui_enable)
		return 0;
#endif

	/* set header */
	tx_buff[0] = 0x03; /* read command */
	tx_buff[1] = (addr & 0xFF00) >> 8; /* address high 8 bit */
	tx_buff[2] = (addr & 0x00FF) >> 0; /* address low 8 bit */

	/* read data */
	spi_message_init(&msg);
	memset(&t, 0, sizeof(struct spi_transfer));
	t.tx_buf = tx_buff;
	t.rx_buf = rx_buff;
	t.len = size + SPI_HEADER_SIZE;
	t.cs_change = 1;
	t.bits_per_word = SPI_BITS_PER_WORD_READ;

	rc = touch_driver_spi_transfer(tdev, &t, &msg);
	if (rc)
		thp_log_err("%s: spi sync err,rc=%d\n", __func__, rc);
	else
		memcpy(data, rx_buff + SPI_HEADER_SIZE, size);

	return rc;
}

static int touch_driver_read_register(struct thp_device *tdev,
	u32 addr, u16 size, u8 *data)
{
	int rc = touch_driver_read_register_(tdev, addr, size, data);
	if (!rc)
		swap_2byte(data, size);

	return rc;
}

static int touch_driver_write_register(struct thp_device *tdev,
	u32 addr, u16 size, u8 *data)
{
	struct spi_transfer t;
	struct spi_message msg;
	int rc;
	char *tx_buff = tdev->tx_buff;
	char *rx_buff = tdev->rx_buff;

	thp_log_debug("%s: called\n", __func__);

#if defined(CONFIG_TEE_TUI)
	if (tui_enable)
		return 0;
#endif

	/* set header */
	tx_buff[0] = 0x02; /* write command */
	tx_buff[1] = (addr & 0xFF00) >> 8; /* address high 8 bit */
	tx_buff[2] = (addr & 0x00FF) >> 0; /* address low 8 bit */

	/* set data */
	memcpy(tx_buff + SPI_HEADER_SIZE, data, size);
	swap_2byte(tx_buff + SPI_HEADER_SIZE, size);

	/* write data */
	memset(&t, 0, sizeof(struct spi_transfer));
	spi_message_init(&msg);
	t.tx_buf = tx_buff;
	t.rx_buf = rx_buff;
	t.len = size + SPI_HEADER_SIZE;
	t.cs_change = 1;
	t.bits_per_word = SPI_BITS_PER_WORD_WRITE;

	rc = touch_driver_spi_transfer(tdev, &t, &msg);
	if (rc)
		thp_log_err("%s:spi sync error:rc=%d\n", __func__, rc);

	return rc;
}

static int touch_driver_otp_read(char *otp_data)
{
	int ret;
	uint16_t i;
	u8 otp_recall[COMM_INDEX2] = {0x01, 0x00};
	u8 otp_read[COMM_INDEX2] = {0x00, 0x00};
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_device *tdev = cd->thp_dev;

	if (!otp_data) {
		thp_log_err("%s:input null\n", __func__);
		return -EINVAL;
	}

	ret = touch_driver_write_register(tdev, OTP_ADDR_RECALL_REG,
			(u16)sizeof(otp_recall), otp_recall);
	if (ret) {
		thp_log_err("bu21150 OTP write failed!");
		return ret;
	}

	msleep(1);

	for (i = 0; i < OTP_SIZE; i++) {
		/* one opt read contains 2 bytes */
		touch_driver_read_register(tdev, (OTP_ADDR_REG +
			i * MULTI_FACTOR), (u16)sizeof(otp_read), otp_read);
		sprintf(&otp_data[i * MULTI_FACTOR], "%c", otp_read[0]);
		sprintf(&otp_data[i * MULTI_FACTOR + 1], "%c", otp_read[1]);
	}

	thp_log_info("%s:bu21150 OTP read data: %s\n", __func__, otp_data);

	return ret;
}

static int touch_driver_init(struct thp_device *tdev)
{
	int rc;
	struct thp_core_data *cd = tdev->thp_core;
	struct device_node *bu21150_node = of_get_child_by_name(cd->thp_node,
						THP_BU21150_DEV_NODE_NAME);

	thp_log_info("%s: called\n", __func__);

	if (!bu21150_node) {
		thp_log_info("%s: bu21150 dev not config in dts\n", __func__);
		return -ENODEV;
	}

	rc = thp_parse_spi_config(bu21150_node, cd);
	if (rc)
		thp_log_err("%s: spi config parse fail\n", __func__);

	rc = thp_parse_timing_config(bu21150_node, &tdev->timing_config);
	if (rc)
		thp_log_err("%s: timing config parse fail\n", __func__);

	return 0;
}

static void touch_driver_timing_work(struct thp_device *tdev)
{
	thp_log_err("%s:called\n", __func__);

	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
	thp_do_time_delay(tdev->timing_config.boot_reset_low_delay_ms);

	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	thp_do_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
}

static int touch_driver_communication_check(struct thp_device *tdev)
{
	int i;
	u8 buf_zero[COMM_TEST_RW_LENGTH] = {0};
	u8 buf_read[COMM_TEST_RW_LENGTH + 1] = {0};

	for (i = 0; i < COMM_TEST_RW_RETRY_TIME; i++) {
		touch_driver_write_register(tdev, COMM_TEST_REG,
			COMM_TEST_RW_LENGTH, (u8 *)COMM_TEST_STR);
		touch_driver_read_register(tdev, COMM_TEST_REG,
			COMM_TEST_RW_LENGTH, buf_read);
		touch_driver_write_register(tdev, COMM_TEST_REG,
			COMM_TEST_RW_LENGTH, buf_zero);

		thp_log_info("%s:get str: %s\n", __func__, buf_read);

		if (!strncmp((char *)buf_read, COMM_TEST_STR,
			(unsigned long)COMM_TEST_RW_LENGTH)) {
			thp_log_info("%s:spi comm check success\n", __func__);
			return 0;
		}
		thp_log_err("%s:spi comm failed %d\n", __func__, i);
		msleep(COMM_TEST_RW_RETRY_DELAY_MS);
	}

	return -ENODEV;
}

static int touch_driver_chip_detect(struct thp_device *tdev)
{
	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}

	touch_driver_timing_work(tdev);

	return touch_driver_communication_check(tdev);
}

static int touch_driver_get_frame(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	if (!tdev) {
		thp_log_info("%s: input dev null\n", __func__);
		return -EINVAL;
	}

	if (!len) {
		thp_log_info("%s: read len illegal\n", __func__);
		return -EINVAL;
	}

	return touch_driver_read_register_(tdev, FRAME_DATA_REG, len, buf);
}

static int touch_driver_resume(struct thp_device *tdev)
{
	thp_log_info("%s: called_\n", __func__);

	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);
	thp_do_time_delay(tdev->timing_config.resume_reset_after_delay_ms);

	return 0;
}

static int touch_driver_suspend(struct thp_device *tdev)
{
	int rc;
	u8 sens_stop_cmd[] = { 0x00, 0x00 }; /* set sens stop command */
	u8 int_stop_cmd[] = { 0x04, 0x00 }; /* set INT stop command */

	thp_log_info("%s: called\n", __func__);

	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}

	rc = touch_driver_write_register(tdev, SENS_START_REG,
			(u16)sizeof(sens_stop_cmd), sens_stop_cmd);
	if (rc)
		thp_log_err("%s:err to write SENS_START_REG\n", __func__);

	rc = touch_driver_write_register(tdev, INT_RUN_ENB_REG,
			(u16)sizeof(int_stop_cmd), int_stop_cmd);
	if (rc)
		thp_log_err("%s:err to write INT_RUN_ENB_REG\n", __func__);

	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_LOW);
	thp_do_time_delay(tdev->timing_config.suspend_reset_after_delay_ms);

	return 0;
}

static void touch_driver_exit(struct thp_device *tdev)
{
	thp_log_info("%s: called\n", __func__);

	if (!tdev) {
		thp_log_err("%s: tdev null\n", __func__);
		return;
	}

	kfree(tdev->tx_buff);
	tdev->tx_buff = NULL;
	kfree(tdev->rx_buff);
	tdev->rx_buff = NULL;
	kfree(tdev);
	tdev = NULL;
}

static struct thp_device_ops bu21150_dev_ops = {
	.init = touch_driver_init,
	.detect = touch_driver_chip_detect,
	.get_frame = touch_driver_get_frame,
	.resume = touch_driver_resume,
	.suspend = touch_driver_suspend,
	.exit = touch_driver_exit,
};

static int __init touch_driver_module_init(void)
{
	int rc = 0;
	struct thp_core_data *cd = thp_get_core_data();

	struct thp_device *dev = kzalloc(sizeof(struct thp_device), GFP_KERNEL);

	if (!dev) {
		thp_log_err("%s: thp device malloc fail\n", __func__);
		return -ENOMEM;
	}

	dev->tx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	dev->rx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if (!dev->tx_buff || !dev->rx_buff) {
		thp_log_err("%s: out of memory\n", __func__);
		rc = -ENOMEM;
		goto err;
	}

	dev->ic_name = BU21150_IC_NAME;
	dev->dev_node_name = THP_BU21150_DEV_NODE_NAME;
	dev->ops = &bu21150_dev_ops;
	if (cd && cd->fast_booting_solution) {
		thp_log_err("%s: don't support this solution\n", __func__);
		goto err;
	}
	rc = thp_register_dev(dev);
	if (rc) {
		thp_log_err("%s: register fail\n", __func__);
		goto err;
	}
#ifndef CONFIG_LCD_KIT_DRIVER
	lcd_huawei_thp_register(&ts_thp_ops);
#endif
	return rc;
err:
	kfree(dev->tx_buff);
	dev->tx_buff = NULL;

	kfree(dev->rx_buff);
	dev->rx_buff = NULL;

	kfree(dev);
	dev = NULL;

	return rc;
}

static void __exit touch_driver_module_exit(void)
{
	thp_log_info("%s: called\n", __func__);
};

module_init(touch_driver_module_init);
module_exit(touch_driver_module_exit);
MODULE_AUTHOR("bu21150");
MODULE_DESCRIPTION("bu21150 driver");
MODULE_LICENSE("GPL");
