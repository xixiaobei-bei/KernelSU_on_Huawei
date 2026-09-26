/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2020 Ilitek.
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

#include "huawei_thp.h"
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

#define ILITEK_IC_NAME "ilitek"
#define THP_ILITEK_DEV_NODE_NAME "ilitek"

#define TDDI_PID_ADDR 0x4009C
#define ILI9881_CHIP 0x9881
#define ILI9882_CHIP 0x9882
#define ENABLE 1
#define DISABLE 0

#define SPI_WRITE 0x82
#define SPI_READ 0x83

#define ENABLE_ICE_MODE_RETRY 3
#define DUMMY_LEN 1
#define ICE_MODE_MAX_LEN 16

#define REGISTER_HEAD 0x25
#define REGISTER_LEN 4
#define TXBUF_LEN 4
#define OFFSET_8 8
#define OFFSET_16 16
#define OFFSET_24 24
#define ICE_MODE_CTRL_CMD 0x181062
#define ICE_MODE_ENABLE_CMD_HEADER_MCU_ON 0x1F
#define ICE_MODE_ENABLE_CMD_HEADER_MCU_OFF 0x25
#define ICE_MODE_DISABLE_CMD_HEADER 0x1B

static u32 g_chip_id;

static int touch_driver_spi_read_write(struct spi_device *client,
	void *tx_buf, void *rx_buf, size_t len)
{
	struct spi_transfer xfer = {
		.tx_buf = tx_buf,
		.rx_buf = rx_buf,
		.len = len,
	};
	struct spi_message msg;
	int ret;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	ret = thp_bus_lock();
	if (ret < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return -EINVAL;
	}

	ret = spi_sync(client, &msg);
	if (ret < 0) {
		thp_log_err("%s:spi_sync failed\n", __func__);
		thp_bus_unlock();
		return -EINVAL;
	}
	thp_bus_unlock();

	return ret;
}

static void touch_driver_hw_reset(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;

	thp_log_info("%s: called,do hardware reset\n", __func__);
	cd = tdev->thp_core;
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	mdelay(tdev->timing_config.boot_reset_hi_delay_ms);
	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
	mdelay(tdev->timing_config.boot_reset_low_delay_ms);
	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	mdelay(tdev->timing_config.boot_reset_hi_delay_ms);
#else
	if (cd->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return;
	}
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_high);
	mdelay(tdev->timing_config.boot_reset_hi_delay_ms);
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_low);
	mdelay(tdev->timing_config.boot_reset_low_delay_ms);
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_high);
	mdelay(tdev->timing_config.boot_reset_hi_delay_ms);
#endif
}

static int touch_driver_init(struct thp_device *tdev)
{
	int ret;
	struct thp_core_data *cd = tdev->thp_core;
	struct device_node *ilitek_node = of_get_child_by_name(cd->thp_node,
		THP_ILITEK_DEV_NODE_NAME);

	thp_log_info("%s: called\n", __func__);

	if (!ilitek_node) {
		thp_log_err("%s: dev not config in dts\n", __func__);
		return -ENODEV;
	}

	ret = thp_parse_spi_config(ilitek_node, cd);
	if (ret)
		thp_log_err("%s: spi config parse fail\n", __func__);

	ret = thp_parse_timing_config(ilitek_node, &tdev->timing_config);
	if (ret)
		thp_log_err("%s: timing config parse fail\n", __func__);

	return 0;
}

static int touch_driver_spi_write(struct spi_device *sdev,
	const void *buf, int len)
{
	int ret = 0;
	u8 *txbuf = NULL;
	int safe_size = len + DUMMY_LEN;

	if ((len <= 0) || (len > THP_MAX_FRAME_SIZE)) {
		thp_log_err("%s: spi write len is invaild\n", __func__);
		return -EINVAL;
	}

	txbuf = kzalloc(safe_size, GFP_ATOMIC);
	if (!txbuf) {
		thp_log_err("%s: failed to allocate txbuf\n", __func__);
		ret = -ENOMEM;
		goto out;
	}

	txbuf[0] = SPI_WRITE;
	memcpy(txbuf + DUMMY_LEN, buf, len);

	if (touch_driver_spi_read_write(sdev, txbuf, NULL, safe_size) < 0) {
		thp_log_err("%s: spi write data err in ice mode\n", __func__);
		ret = -EIO;
	}
	kfree(txbuf);
out:
	txbuf = NULL;
	return ret;
}

static int touch_driver_spi_read(struct spi_device *sdev, u8 *buf, int len)
{
	int ret = 0;
	u8 *txbuf = NULL;
	struct thp_core_data *cd = thp_get_core_data();

	if (!buf) {
		thp_log_info("%s: input buf null\n", __func__);
		return -EINVAL;
	}

	txbuf = cd->thp_dev->tx_buff;
	if ((len <= 0) || (len > THP_MAX_FRAME_SIZE)) {
		thp_log_err("%s: spi read len is invaild\n", __func__);
		return -EINVAL;
	}

	txbuf[0] = SPI_READ;
	if (touch_driver_spi_read_write(sdev, txbuf, buf, len) < 0) {
		thp_log_err("%s: spi read data error in ice mode\n", __func__);
		ret = -EIO;
		return ret;
	}

	return ret;
}

static int touch_driver_ice_mode_read(struct spi_device *sdev,
	u32 addr, u32 *data, int len)
{
	int ret;
	int index = 0;
	u8 *rxbuf = NULL;
	u8 txbuf[TXBUF_LEN] = { 0 };

	txbuf[index++] = (u8)REGISTER_HEAD;
	txbuf[index++] = (u8)(addr);
	txbuf[index++] = (u8)(addr >> OFFSET_8);
	txbuf[index++] = (u8)(addr >> OFFSET_16);

	if ((len <= 0) || (len > ICE_MODE_MAX_LEN)) {
		thp_log_err("%s: ice mode read len is invaild\n", __func__);
		return -EINVAL;
	}

	rxbuf = kzalloc(len + DUMMY_LEN, GFP_ATOMIC);
	if (!rxbuf) {
		thp_log_err("%s: Failed to allocate rxbuf\n", __func__);
		return -ENOMEM;
	}

	ret = touch_driver_spi_write(sdev, txbuf, REGISTER_LEN);
	if (ret < 0) {
		thp_log_err("%s: spi write failed\n", __func__);
		goto out;
	}

	ret = touch_driver_spi_read(sdev, rxbuf, len + DUMMY_LEN);
	if (ret < 0) {
		thp_log_err("%s: spi read failed\n", __func__);
		goto out;
	}

	if (len == sizeof(u8))
		*data = rxbuf[1];
	else
		*data = ((rxbuf[1]) | (rxbuf[2] << OFFSET_8) |
			(rxbuf[3] << OFFSET_16) | (rxbuf[4] << OFFSET_24));

out:
	if (ret < 0)
		thp_log_err("%s: failed to read data in ice mode, ret = %d\n",
			__func__, ret);

	kfree(rxbuf);
	rxbuf = NULL;
	return ret;
}

static int touch_driver_ice_mode_ctrl(struct thp_device *tdev,
	bool ice_enable, bool mcu_enable)
{
	int i;
	int ret = 0;
	int index = 0;
	u32 chip_id = 0;
	u8 cmd[REGISTER_LEN] = {0};

	thp_log_info("%s: ice_enable = %d, mcu_enable = %d\n", __func__,
		ice_enable, mcu_enable);

	cmd[index++] = (u8)ICE_MODE_ENABLE_CMD_HEADER_MCU_OFF;
	cmd[index++] = (u8)(ICE_MODE_CTRL_CMD);
	cmd[index++] = (u8)(ICE_MODE_CTRL_CMD >> OFFSET_8);
	cmd[index++] = (u8)(ICE_MODE_CTRL_CMD >> OFFSET_16);

	if (ice_enable) {
		if (mcu_enable)
			cmd[0] = ICE_MODE_ENABLE_CMD_HEADER_MCU_ON;
		for (i = 0; i < ENABLE_ICE_MODE_RETRY; i++) {
			touch_driver_hw_reset(tdev);
			if (touch_driver_spi_write(tdev->thp_core->sdev, cmd,
				sizeof(cmd)) < 0)
				thp_log_err("%s: write ice mode cmd error\n",
					__func__);

			if (touch_driver_ice_mode_read(tdev->thp_core->sdev,
				TDDI_PID_ADDR, &chip_id, sizeof(u32)) < 0)
				thp_log_err("%s: Read chip_pid error\n",
					__func__);

			thp_log_info("%s: chipid 0x%X\n", __func__, chip_id);
			if (((chip_id >> OFFSET_16) == ILI9881_CHIP) ||
				((chip_id >> OFFSET_16) == ILI9882_CHIP))
				break;
		}

		if (i >= ENABLE_ICE_MODE_RETRY) {
			thp_log_err("%s: Enter to ICE Mode failed\n", __func__);
			return -EINVAL;
		}
		g_chip_id = chip_id;
	} else {
		cmd[0] = ICE_MODE_DISABLE_CMD_HEADER;
		ret = touch_driver_spi_write(tdev->thp_core->sdev,
			cmd, sizeof(cmd));
		if (ret < 0)
			thp_log_err("%s: Exit to ICE Mode failed\n", __func__);
	}

	return ret;
}

static int touch_driver_chip_detect(struct thp_device *tdev)
{
	int ret;

	ret = touch_driver_ice_mode_ctrl(tdev, ENABLE, DISABLE);
	/* pull reset to exit ice mode */
	touch_driver_hw_reset(tdev);
	if (ret) {
		thp_log_err("%s: chip is not detected\n", __func__);
		return -ENODEV;
	}

	return 0;
}

static int touch_driver_get_frame(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	int ret;

	if (!tdev) {
		thp_log_err("%s: input dev null\n", __func__);
		return -ENOMEM;
	}

	ret = touch_driver_spi_read(tdev->thp_core->sdev, buf, len);
	if (ret < 0) {
		thp_log_err("%s: Read frame packet failed, ret = %d\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

static int touch_driver_resume(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;

	thp_log_info("%s: called\n", __func__);
	cd = tdev->thp_core;
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_set_value(tdev->gpios->cs_gpio, GPIO_HIGH);
	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
#else
	if (cd->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return 0;
	}
	pinctrl_select_state(cd->pctrl, cd->pins_default);
#endif
	mdelay(tdev->timing_config.resume_reset_after_delay_ms);

	return 0;
}

static int touch_driver_suspend(struct thp_device *tdev)
{
	int pt_test_mode = is_pt_test_mode(tdev);

	thp_log_info("%s: called\n", __func__);
	if (pt_test_mode)
		thp_log_info("%s: This is pt test mode\n", __func__);
	else
		thp_log_info("%s: This is normal sleep-in mode\n", __func__);

	return 0;
}

static void touch_driver_exit(struct thp_device *tdev)
{
	thp_log_info("%s: called\n", __func__);
	kfree(tdev->tx_buff);
	tdev->tx_buff = NULL;
	kfree(tdev);
	tdev = NULL;
}

struct thp_device_ops ilitek_dev_ops = {
	.init = touch_driver_init,
	.detect = touch_driver_chip_detect,
	.get_frame = touch_driver_get_frame,
	.resume = touch_driver_resume,
	.suspend = touch_driver_suspend,
	.exit = touch_driver_exit,
};

static int __init touch_driver_module_init(void)
{
	int ret;
	struct thp_device *dev = NULL;

	thp_log_info("%s: called\n", __func__);
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		thp_log_err("%s: thp device malloc fail\n", __func__);
		return -ENOMEM;
	}

	dev->tx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	if (!dev->tx_buff) {
		thp_log_err("%s: out of memory\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	dev->ic_name = ILITEK_IC_NAME;
	dev->ops = &ilitek_dev_ops;

	ret = thp_register_dev(dev);
	if (ret) {
		thp_log_err("%s: register fail\n", __func__);
		goto err;
	}

	return ret;
err:
	kfree(dev->tx_buff);
	dev->tx_buff = NULL;
	kfree(dev);
	dev = NULL;
	return ret;
}

static void __exit touch_driver_module_exit(void)
{
	thp_log_info("%s: called\n", __func__);
};

module_init(touch_driver_module_init);
module_exit(touch_driver_module_exit);
MODULE_AUTHOR("ilitek");
MODULE_DESCRIPTION("ilitek driver");
MODULE_LICENSE("GPL");
