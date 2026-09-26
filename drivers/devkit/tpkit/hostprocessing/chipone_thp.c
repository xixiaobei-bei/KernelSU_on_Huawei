/*
 * Thp driver code for chipone
 *
 * Copyright (c) 2012-2022 Huawei Technologies Co., Ltd.
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
#include <linux/delay.h>
#include <linux/gpio.h>
#include "huawei_thp.h"

#define CTS_DEV_NAME "chipone"
#define CTS_IC_NAME "chipone"
#define CTS_IC_HWID_MASK 0x00FFFFF0
#define CTS_IC_HWID 0x00990160

#define PROG_RD 0x61
#define PROG_RD_HEAD_SIZ 5
#define NORM_RD 0xF1

#define RESET_LOW_DELAY_MS 1
#define RESET_HIGH_DELAY_MS 6

#define ENTER_PROG_RETRIES 3
#define ENTER_PROG_DELAY_MS 5

#define REG_HW_ID 0x30000

#define FRAME_SIZE 1400

#define DETECT_RETRY_TIME 3
#define DETECT_RETRY_DELAY_MS 6

struct check_result {
	int retcode;
	uint16_t curr_size;
	uint16_t next_size;
	uint8_t tcs_errcode;
	uint16_t tcs_cmdid;
};

/* Enter program mode command */
static uint8_t enter_prog_cmd[] = {
	0xCC, 0x33, 0x55, 0x5A
};

/* Get frame data command */
static uint8_t get_frame_cmd[] = {
	NORM_RD, 0x22, 0x41, 0xD4, 0x04, 0xF4, 0x7F
};

static inline uint32_t get_unaligned_le24(const void *p)
{
	const uint8_t *puc = (const uint8_t *)p;

	/* Concatenate bytes based on bits. */
	return (puc[0] | (puc[1] << 8) | (puc[2] << 16));
}

static inline void put_unaligned_be24(uint32_t v, void *p)
{
	uint8_t *puc = (uint8_t *)p;

	/* Split bytes by bits */
	puc[0] = (v >> 16) & 0xFF;
	puc[1] = (v >> 8) & 0xFF;
	puc[2] = (v >> 0) & 0xFF;
}

static int touch_driver_spi_transfer(struct thp_device *tdev,
	void *tx_buf, void *rx_buf, size_t len)
{
	struct spi_transfer xfer = {
		.tx_buf = tx_buf,
		.rx_buf = rx_buf,
		.len = len,
	};
	struct spi_message msg;
	int rc;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);
	rc = thp_spi_sync(tdev->sdev, &msg);
	if (rc)
		thp_log_err("%s: spi_sync failed: rc=%d\n", __func__, rc);
	return rc;
}

static int touch_driver_prog_read_reg(struct thp_device *tdev,
	uint32_t reg_addr, size_t rlen)
{
	uint16_t total_len = PROG_RD_HEAD_SIZ + rlen;
	int rc;
	bool error_flag = (!rlen) || (total_len > THP_MAX_FRAME_SIZE);

	thp_log_info("Enter %s\n", __func__);
	if (error_flag) {
		thp_log_err("%s: Invalid read size %ld\n", __func__, rlen);
		return -EINVAL;
	}

	thp_bus_lock();
	memset(tdev->tx_buff, 0, PROG_RD_HEAD_SIZ);
	tdev->tx_buff[0] = PROG_RD;
	put_unaligned_be24(reg_addr, &tdev->tx_buff[1]);
	rc = touch_driver_spi_transfer(tdev, tdev->tx_buff, tdev->rx_buff,
		total_len);
	thp_bus_unlock();
	if (rc)
		thp_log_err("%s: Prog read reg %#07x failed: rc=%d\n", __func__,
			reg_addr, rc);
	thp_log_info("Exit %s\n", __func__);
	return rc;
}

static int touch_driver_enter_prog_mode(struct thp_device *tdev)
{
	int retries = ENTER_PROG_RETRIES;
	int cmd_size = sizeof(enter_prog_cmd);
	int rc;

	thp_log_info("Enter %s\n", __func__);
	thp_bus_lock();
	memcpy(tdev->tx_buff, enter_prog_cmd, cmd_size);
	while (retries--) {
		rc = touch_driver_spi_transfer(tdev, tdev->tx_buff,
			tdev->rx_buff, cmd_size);
		if (!rc)
			break;
		thp_time_delay(ENTER_PROG_DELAY_MS);
	}
	thp_bus_unlock();
	if (rc)
		thp_log_err("%s: Enter prog mode failed: rc=%d\n", __func__, rc);
	thp_log_info("Exit %s: rc=%d\n", __func__, rc);
	return rc;
}

static void touch_driver_hw_reset(struct thp_device *tdev)
{
	thp_log_info("Enter %s\n", __func__);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	thp_time_delay(tdev->timing_config.boot_reset_low_delay_ms);
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_HIGH);
	thp_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
	thp_log_info("Exit %s\n", __func__);
}

static int touch_driver_prog_get_hwid(struct thp_device *tdev, uint32_t *hwid)
{
	int rc;

	thp_log_info("Enter %s\n", __func__);
	rc = touch_driver_enter_prog_mode(tdev);
	if (rc) {
		thp_log_err("%s: Enter program mode failed: rc=%d\n", __func__, rc);
		return rc;
	}
	rc = touch_driver_prog_read_reg(tdev, REG_HW_ID, sizeof(uint32_t));
	if (rc) {
		thp_log_err("%s: Prog read hwid failed: rc=%d\n", __func__, rc);
		return rc;
	}
	*hwid = get_unaligned_le24(tdev->rx_buff + PROG_RD_HEAD_SIZ);
	thp_log_info("%s: hwid = %#06x\n", __func__, *hwid);
	return 0;
}

static int thp_dev_free(struct thp_device *tdev)
{
	thp_log_info("Enter %s\n", __func__);
	if (tdev) {
		kfree(tdev->rx_buff);
		tdev->rx_buff = NULL;
		kfree(tdev->tx_buff);
		tdev->tx_buff = NULL;
		kfree(tdev);
		tdev = NULL;
	}
	thp_log_info("Exit %s\n", __func__);
	return 0;
}

static struct thp_device *thp_dev_malloc(void)
{
	struct thp_device *tdev = NULL;
	bool error_flag;

	thp_log_info("Enter %s\n", __func__);
	tdev = kzalloc(sizeof(struct thp_device), GFP_KERNEL);
	if (!tdev)
		return NULL;
	tdev->tx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	tdev->rx_buff = kzalloc(THP_MAX_FRAME_SIZE, GFP_KERNEL);
	error_flag = (!tdev->tx_buff) || (!tdev->rx_buff);
	if (error_flag)
		goto err_thp_dev_free;
	thp_log_info("Exit %s\n", __func__);
	return tdev;

err_thp_dev_free:
	thp_dev_free(tdev);
	return NULL;
}

static int touch_driver_init(struct thp_device *tdev)
{
	struct thp_core_data *cd = tdev->thp_core;
	struct device_node *cts_node;
	int rc;

	thp_log_info("Enter %s\n", __func__);
	cts_node = of_get_child_by_name(cd->thp_node, CTS_DEV_NAME);
	if (!cts_node) {
		thp_log_err("%s node NOT found in dts\n", CTS_DEV_NAME);
		return -ENODEV;
	}
	if (tdev->sdev->master->flags & SPI_MASTER_HALF_DUPLEX) {
		thp_log_err("Full duplex not supported by master\n");
		return -EIO;
	}
	rc = thp_parse_spi_config(cts_node, cd);
	if (rc) {
		thp_log_err("%s: Spi config parse failed: rc=%d\n", __func__, rc);
		return -EINVAL;
	}
	rc = thp_parse_timing_config(cts_node, &tdev->timing_config);
	if (rc) {
		thp_log_err("%s: Timing config parse failed: rc=%d\n", __func__, rc);
		return -EINVAL;
	}
	thp_log_info("Exit %s, dts parsed ok\n", __func__);
	return 0;
}

static int touch_driver_detect(struct thp_device *tdev)
{
	uint32_t hwid;
	int rc;
	int retry;

	thp_log_info("Enter %s\n", __func__);
	for (retry = 0; retry < DETECT_RETRY_TIME; retry++) {
		touch_driver_hw_reset(tdev);
		rc = touch_driver_prog_get_hwid(tdev, &hwid);
		if (rc) {
			thp_log_err("%s: Get hwid failed: rc=%d\n", __func__, rc);
			thp_time_delay(DETECT_RETRY_DELAY_MS);
			continue;
		}
		if ((hwid & CTS_IC_HWID_MASK) != CTS_IC_HWID) {
			thp_log_err("%s: Mismatch hwid count:%d, got %#06x while %#06x expected\n",
				__func__, retry, hwid, CTS_IC_HWID);
		} else {
			thp_log_info("Exit %s, hwid=%06x\n", __func__, hwid);
			return 0;
		}
		thp_time_delay(DETECT_RETRY_DELAY_MS);
	}
	if (tdev->thp_core->fast_booting_solution) {
		kfree(tdev->rx_buff);
		tdev->rx_buff = NULL;
		kfree(tdev->tx_buff);
		tdev->tx_buff = NULL;
		kfree(tdev);
		tdev = NULL;
	}
	return -EINVAL;
}

static int touch_driver_get_frame(struct thp_device *tdev, char *buf,
	unsigned int len)
{
	int rc;

	thp_bus_lock();
	memcpy(tdev->tx_buff, get_frame_cmd, sizeof(get_frame_cmd));
	rc = touch_driver_spi_transfer(tdev, tdev->tx_buff, buf,
		FRAME_SIZE);
	if (rc) {
		thp_log_err("%s: Get frame data failed: rc=%d\n", __func__, rc);
		thp_bus_unlock();
		return -EIO;
	}
	thp_bus_unlock();
	return rc;
}

static int touch_driver_resume(struct thp_device *tdev)
{
	thp_log_info("Enter %s\n", __func__);
	touch_driver_hw_reset(tdev);
	thp_log_info("Exit %s\n", __func__);
	return 0;
}

static int thp_driver_suspend(struct thp_device *tdev)
{
	thp_log_info("Enter %s\n", __func__);
	bool error_flag = (!tdev) || (!tdev->thp_core) || (!tdev->thp_core->sdev);

	if (error_flag) {
		thp_log_err("Invalid parameters\n");
		return -EINVAL;
	}
	thp_log_info("Power off mode");
	gpio_set_value(tdev->gpios->rst_gpio, GPIO_LOW);
	thp_log_info("Exit %s\n", __func__);
	return 0;
}

static void touch_driver_exit(struct thp_device *tdev)
{
	thp_log_info("Enter %s\n", __func__);
	thp_dev_free(tdev);
	thp_log_info("Exit %s\n", __func__);
}

static struct thp_device_ops cts_dev_ops = {
	.init = touch_driver_init,
	.detect = touch_driver_detect,
	.get_frame = touch_driver_get_frame,
	.resume = touch_driver_resume,
	.suspend = thp_driver_suspend,
	.exit = touch_driver_exit,
};

static int __init touch_driver_module_init(void)
{
	int rc;
	struct thp_device *tdev;
	struct thp_core_data *cd = thp_get_core_data();

	thp_log_info("THP dirver register start\n");
	tdev = thp_dev_malloc();
	if (!tdev) {
		thp_log_err("%s: Malloc for thp device failed\n", __func__);
		return -EINVAL;
	}
	tdev->ic_name = CTS_IC_NAME;
	tdev->dev_node_name = CTS_DEV_NAME;
	tdev->ops = &cts_dev_ops;
	if (cd && cd->fast_booting_solution) {
		thp_send_detect_cmd(tdev, NO_SYNC_TIMEOUT);
		/*
		 * The thp_register_dev will be called later to complete
		 * the real detect action.If it fails, the detect function will
		 * release the resources requested here.
		 */
		return 0;
	}
	rc = thp_register_dev(tdev);
	if (rc) {
		thp_log_err("%s: Register thp device failed: rc=%d\n", __func__, rc);
		thp_dev_free(tdev);
		return rc;
	}
	thp_log_info("THP dirver registered\n");
	return 0;
}

static void __exit touch_driver_module_exit(void)
{
	thp_log_info("%s called, do nothing\n", __func__);
}

module_init(touch_driver_module_init);
module_exit(touch_driver_module_exit);
MODULE_LICENSE("GPL");
