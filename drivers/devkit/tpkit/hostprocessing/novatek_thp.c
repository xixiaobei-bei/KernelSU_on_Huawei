/*
 * Thp driver code for novatek
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

#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif
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
#include "huawei_thp.h"


#define NOVATECH_IC_NAME "novatech"
#define THP_NOVA_DEV_NODE_NAME "novatech"

#define spi_write_mask(a) ((a) | (0x80))
#define spi_read_mask(a) ((a) & (0x7F))

#define nvt_low_word_bit_mask(x) (uint8_t)(((x) >> (7)) & (0x000000FF))
#define nvt_hi_word_bit_mask(x) (uint8_t)(((x) >> (15)) & (0x000000FF))
#define NVT_ID_BYTE_MAX 6
#define NVT_SW_RESET_RW_LEN 4
#define NVT_SW_RESET_REG_ADDR 0x3F0FE
#define NVT_SW_RESET_DELAY_MS 10
#define NVT_SW_RETRY_TIME_DELAY_MS 10
#define NVT_CHIP_VER_TRIM_RW_LEN 16
#define NVT_CHIP_VER_TRIM_REG_ADDR 0x1F64E
#define NVT_CHIP_VER_TRIM_REG_ADDR_9Z 0x3F004
#define NVT_CHIP_VER_TRIM_RETRY_TIME 5
#define WAIT_FOR_SPI_BUS_READ_DELAY 5
#define NVT_SUSPEND_AFTER_DELAY_MS 30

#define NVT_CMD_ADDR 0x21C50
#define NVT_CMD_STAMP_ADDR 0x21C77
#define NVT36672C_CMD_ADDR 0x22D50
#define NVT36672C_CMD_STAMP_ADDR 0x22D77
#define N9Z_CMD_ADDR 0x2FE50
#define N9Z_CMD_STAMP_ADDR 0x2FE77
#define NVT_GET_GESTURE_ADDR 0x218CC
#define NVT_CMD_ENTER_CUSTOMIZED_DP_DSTB 0x7D
#define NVT_CMD_ENTER_GESTURE_MODE 0x13
#define FORMAL_REPORT 1
#define RETRY_NUM 20
#define SEND_CMD_LEN 3
#define GESTURE_DATA_LEN 9
#define DEBUG_GESTURE_DATA_LEN 12
#define NOVA_DEBUG 1
#define D_SEND_CMD_RETEY 5
#define D_SEND_CMD_LEN 7
#define NVT_SPI_RW_BUF_LEN 10
#define NVT_GET_FRAME_CMD 0x20
#define STYLUS_EVENT 1
#define STYLUS_GESTURE_ID 0x6F
#define DOUBLE_CLICLK_GESTURE_ID 0x7F
#define BT_CONNECT_CMD_RETRY 3
#define STATUS_CMD 0xC2
#define STYLUS_CONNECT 2
#define STYLUS_DISCONNECT 3
#define SEND_BT_CONNECT_STATUS_AFTER_RESET_DELAY 200
#define WAIT_FOR_SPI_BUS_RESUMED_DELAY_QCOM 100
#define STYLUS3_CONNECT 2

static int send_status_flag;
/*
 * 0:default status
 * 2:stylus connect status
 * 3:stylus disconnect status
 */

enum nvt_ic_type {
	NVT36672C_SERIES = 1,
	N9Z_SERIES = 5,
};

struct nvt_ts_trim_id_table_entry {
	uint8_t id[NVT_ID_BYTE_MAX];
	uint8_t mask[NVT_ID_BYTE_MAX];
};

static const struct nvt_ts_trim_id_table_entry trim_id_table[] = {
	{
		.id = { 0x0C, 0xFF, 0xFF, 0x72, 0x66, 0x03 },
		.mask = { 1, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0x0C, 0xFF, 0xFF, 0x82, 0x66, 0x03 },
		.mask = { 1, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0x0A, 0xFF, 0xFF, 0x72, 0x65, 0x03 },
		.mask = { 1, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0x0B, 0xFF, 0xFF, 0x25, 0x65, 0x03 },
		.mask = { 1, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0x0A, 0xFF, 0xFF, 0x72, 0x67, 0x03 },
		.mask = { 1, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0x0A, 0xFF, 0xFF, 0x82, 0x66, 0x03 },
		.mask = { 1, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0x0B, 0xFF, 0xFF, 0x82, 0x66, 0x03 },
		.mask = { 1, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0x0A, 0xFF, 0xFF, 0x72, 0x66, 0x03 },
		.mask = { 1, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0x55, 0x00, 0xFF, 0x00, 0x00, 0x00 },
		.mask = { 1, 1, 0, 1, 1, 1 }
	}, {
		.id = { 0x55, 0x72, 0xFF, 0x00, 0x00, 0x00 },
		.mask = { 1, 1, 0, 1, 1, 1 }
	}, {
		.id = { 0xAA, 0x00, 0xFF, 0x00, 0x00, 0x00 },
		.mask = { 1, 1, 0, 1, 1, 1 }
	}, {
		.id = { 0xAA, 0x72, 0xFF, 0x00, 0x00, 0x00 },
		.mask = { 1, 1, 0, 1, 1, 1 }
	}, {
		.id = { 0xFF, 0xFF, 0xFF, 0x72, 0x67, 0x03 },
		.mask = { 0, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0xFF, 0xFF, 0xFF, 0x70, 0x66, 0x03 },
		.mask = { 0, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0xFF, 0xFF, 0xFF, 0x70, 0x67, 0x03 },
		.mask = { 0, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0xFF, 0xFF, 0xFF, 0x72, 0x66, 0x03 },
		.mask = { 0, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0xFF, 0xFF, 0xFF, 0x25, 0x65, 0x03 },
		.mask = { 0, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0xFF, 0xFF, 0xFF, 0x70, 0x68, 0x03 },
		.mask = { 0, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0xFF, 0xFF, 0xFF, 0x26, 0x65, 0x03 },
		.mask = { 0, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0x0B, 0xFF, 0xFF, 0x23, 0x65, 0x03 },
		.mask = { 1, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0x0A, 0xFF, 0xFF, 0x23, 0x65, 0x03 },
		.mask = { 1, 0, 0, 1, 1, 1 }
	}, {
		.id = { 0x20, 0xFF, 0xFF, 0x23, 0x65, 0x03 },
		.mask = { 1, 0, 0, 1, 1, 1 }
	}
};

struct nvt_ts_spi_buf {
	uint8_t *w_buf;
	uint8_t *r_buf;
} g_nvt_ts_spi_buf;

static int touch_driver_nt_bt_handler(struct thp_device *tdev,
	bool delay_enable);

static int touch_driver_spi_read_write(struct spi_device *client,
	void *tx_buf, void *rx_buf, size_t len)
{
	struct spi_transfer t = {
		.tx_buf = tx_buf,
		.rx_buf = rx_buf,
		.len    = len,
	};
	struct spi_message m;
	int rc;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);

	rc = thp_bus_lock();
	if (rc < 0) {
		thp_log_err("%s:get lock failed\n", __func__);
		return -EINVAL;
	}
#ifndef CONFIG_HUAWEI_THP_MTK
	thp_spi_cs_set(GPIO_HIGH);
#endif
	rc = thp_spi_sync(client, &m);
	thp_bus_unlock();

	return rc;
}

static int touch_driver_check_spi_master_capacity(struct spi_device *sdev)
{
	if (sdev->master->flags & SPI_MASTER_HALF_DUPLEX) {
		thp_log_err("%s: Full duplex not supported by master\n",
			__func__);
		return -EIO;
	} else {
		return 0;
	}
}

void touch_driver_sw_reset_idle(struct spi_device *sdev)
{
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	/* This register addr is for NT36870 */
	uint32_t addr = NVT_SW_RESET_REG_ADDR;
	unsigned int rw_len;
	int index = 0;
	int ret;

	if (!w_buf) {
		thp_log_err("%s: w_buf is null\n", __func__);
		return;
	}
	/* write 0xAA @0x3F0FE */
	w_buf[index++] = spi_write_mask(0x7F);
	w_buf[index++] = nvt_hi_word_bit_mask(addr);
	w_buf[index++] = nvt_low_word_bit_mask(addr);
	rw_len = 3; /* 3 is read or write data len */

	ret = touch_driver_spi_read_write(sdev, w_buf, NULL, rw_len);
	if (ret)
		thp_log_err("%s, spi write failed\n", __func__);

	w_buf[0] = (uint8_t)(spi_write_mask((addr & 0x0000007F)));
	w_buf[1] = 0xAA;
	rw_len = 2; /* 2 is read or write data len */
	ret = touch_driver_spi_read_write(sdev, w_buf, NULL, rw_len);
	if (ret)
		thp_log_err("%s, spi write failed\n", __func__);

	thp_time_delay(NVT_SW_RESET_DELAY_MS);
}

void touch_driver_bootloader_reset(struct spi_device *sdev)
{
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	/* This register addr is for NT36870 */
	uint32_t addr = NVT_SW_RESET_REG_ADDR;
	unsigned int rw_len;
	int index = 0;
	int ret;

	thp_log_info("%s: called\n", __func__);
	if (!w_buf) {
		thp_log_err("%s: w_buf is null\n", __func__);
		return;
	}
	/* write 0xAA @0x3F0FE */
	w_buf[index++] = spi_write_mask(0x7F);
	w_buf[index++] = nvt_hi_word_bit_mask(addr);
	w_buf[index++] = nvt_low_word_bit_mask(addr);
	rw_len = 3; /* 3 is read or write data len */

	ret = touch_driver_spi_read_write(sdev, w_buf, NULL, rw_len);
	if (ret)
		thp_log_err("%s, spi write failed\n", __func__);

	w_buf[0] = (uint8_t)(spi_write_mask((addr & 0x0000007F)));
	w_buf[1] = 0x69; /* bootloader reset cmd */
	rw_len = 2; /* 2 is read or write data len */
	ret = touch_driver_spi_read_write(sdev, w_buf, NULL, rw_len);
	if (ret)
		thp_log_err("%s, spi write failed\n", __func__);

	thp_time_delay(NVT_SW_RESET_DELAY_MS);
}

static int touch_driver_check_ver_trim_table(uint8_t *r_buf)
{
	int i;
	int list;
	const int buf_len = 2;

	thp_log_info("%s:begin\n", __func__);

	for (list = 0; list < ARRAY_SIZE(trim_id_table); list++) {
		for (i = 0; i < NVT_ID_BYTE_MAX; i++) {
			if (trim_id_table[list].mask[i] &&
				r_buf[i + buf_len] != trim_id_table[list].id[i])
				break;
		}

		if (i == NVT_ID_BYTE_MAX) {
			thp_log_info("%s:done, list=%d\n", __func__, list);
			return 0;
		}
	}

	return -ENODEV;
}

static void touch_driver_timing_work(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;

	thp_log_err("%s:called,do sw reset idle in hard reset\n", __func__);
	cd = tdev->thp_core;
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_direction_output(tdev->gpios->rst_gpio, GPIO_LOW);
	thp_time_delay(tdev->timing_config.boot_reset_low_delay_ms);
	touch_driver_sw_reset_idle(tdev->sdev);
	gpio_set_value(tdev->gpios->rst_gpio, THP_RESET_HIGH);
	thp_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
#else
	if (tdev->thp_core->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return;
	}
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_low);
	thp_time_delay(tdev->timing_config.boot_reset_low_delay_ms);
	touch_driver_sw_reset_idle(tdev->sdev);
	pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.reset_high);
	thp_time_delay(tdev->timing_config.boot_reset_hi_delay_ms);
#endif
}

static int8_t touch_driver_check_chip_ver_trim(struct spi_device *sdev)
{
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	uint8_t *r_buf = g_nvt_ts_spi_buf.r_buf;
	uint32_t addr = NVT_CHIP_VER_TRIM_REG_ADDR;
	int rw_len;
	int32_t retry;
	int ret;
	struct thp_core_data *cd = thp_get_core_data();

	if (!w_buf) {
		thp_log_err("%s: w_buf is null\n", __func__);
		return -EINVAL;
	}
	if (!r_buf) {
		thp_log_err("%s: r_buf is null\n", __func__);
		return -EINVAL;
	}

	if (cd->support_vendor_ic_type == N9Z_SERIES)
		addr = NVT_CHIP_VER_TRIM_REG_ADDR_9Z;

	/* Check for 5 times */
	for (retry = NVT_CHIP_VER_TRIM_RETRY_TIME; retry > 0; retry--) {
		int index = 0;

		touch_driver_bootloader_reset(sdev);
		touch_driver_sw_reset_idle(sdev);
		/* read chip id @0x1F64E */
		w_buf[index++] = spi_write_mask(0x7F);
		w_buf[index++] = nvt_hi_word_bit_mask(addr);
		w_buf[index++] = nvt_low_word_bit_mask(addr);
		rw_len = 3; /* 3 is read or write data len */
		ret = touch_driver_spi_read_write(sdev, w_buf, NULL, rw_len);
		if (ret)
			thp_log_err("%s, spi write failed\n", __func__);

		memset(w_buf, 0, NVT_CHIP_VER_TRIM_RW_LEN);
		w_buf[0] = (uint8_t)(spi_read_mask(addr));
		rw_len = 8; /* 8 is read or write data len */
		ret = touch_driver_spi_read_write(sdev, w_buf, r_buf, rw_len);
		if (ret)
			thp_log_err("%s, spi transfer failed\n", __func__);

		if (!touch_driver_check_ver_trim_table(r_buf))
			return 0;

		thp_time_delay(NVT_SW_RETRY_TIME_DELAY_MS);
	}

	return -ENODEV;
}

static int touch_driver_init(struct thp_device *tdev)
{
	int rc;
	struct thp_core_data *cd = tdev->thp_core;
	struct device_node *nova_node = of_get_child_by_name(cd->thp_node,
						THP_NOVA_DEV_NODE_NAME);

	thp_log_info("%s: called\n", __func__);

	if (!nova_node) {
		thp_log_info("%s: dev not config in dts\n", __func__);
		return -ENODEV;
	}

	rc = touch_driver_check_spi_master_capacity(tdev->sdev);
	if (rc) {
		thp_log_err("%s: spi capacity check fail\n", __func__);
		return rc;
	}

	rc = thp_parse_spi_config(nova_node, cd);
	if (rc)
		thp_log_err("%s: spi config parse fail\n", __func__);

	rc = thp_parse_timing_config(nova_node, &tdev->timing_config);
	if (rc)
		thp_log_err("%s: timing config parse fail\n", __func__);

	rc = of_property_read_u32(nova_node, "send_cmd_retry_delay_ms",
		&cd->send_cmd_retry_delay_ms);
	if (rc)
		cd->send_cmd_retry_delay_ms = 0;
	thp_log_info("%s: send_cmd_retry_delay_ms %u\n", __func__,
		cd->send_cmd_retry_delay_ms);

	rc = of_property_read_u32(nova_node, "support_spi_resume_delay",
		&cd->support_spi_resume_delay);
	if (rc)
		cd->support_spi_resume_delay = 0;
	thp_log_info("%s: support_spi_resume_delay %u\n", __func__,
		cd->support_spi_resume_delay);

	return 0;
}

static int touch_driver_chip_detect(struct thp_device *tdev)
{
	int rc;
	struct thp_core_data *cd = thp_get_core_data();
	struct device_node *nova_node = of_get_child_by_name(cd->thp_node,
		THP_NOVA_DEV_NODE_NAME);

	g_nvt_ts_spi_buf.w_buf = kzalloc(NVT_CHIP_VER_TRIM_RW_LEN, GFP_KERNEL);
	if (!g_nvt_ts_spi_buf.w_buf) {
		thp_log_err("%s:w_buf request memory fail\n", __func__);
		rc = -ENOMEM;
		goto exit;
	}

	g_nvt_ts_spi_buf.r_buf = kzalloc(NVT_CHIP_VER_TRIM_RW_LEN, GFP_KERNEL);
	if (!g_nvt_ts_spi_buf.r_buf) {
		thp_log_err("%s:r_buf request memory fail\n", __func__);
		rc = -ENOMEM;
		goto exit;
	}

	touch_driver_timing_work(tdev);

	rc =  touch_driver_check_chip_ver_trim(tdev->thp_core->sdev);
	if (rc) {
		thp_log_info("%s:chip is not identified\n", __func__);
		rc = -ENODEV;
		goto exit;
	}

	if (cd->support_vendor_ic_type == N9Z_SERIES) {
		rc = thp_parse_trigger_config(nova_node, cd);
		if (rc)
			thp_log_err("%s: trigger_config fail\n", __func__);

		rc = thp_parse_feature_config(nova_node, cd);
		if (rc)
			thp_log_err("%s: feature_config fail\n", __func__);
	}

	if ((cd->send_bt_status_to_fw) && (cd->support_vendor_ic_type == N9Z_SERIES)) {
		if (touch_driver_nt_bt_handler(tdev, false))
			thp_log_err("power on send stylus3 connect status fail\n");
	}

	return 0;

exit:
	kfree(g_nvt_ts_spi_buf.w_buf);
	g_nvt_ts_spi_buf.w_buf = NULL;
	kfree(g_nvt_ts_spi_buf.r_buf);
	g_nvt_ts_spi_buf.r_buf = NULL;
	if (tdev->thp_core->fast_booting_solution) {
		kfree(tdev->tx_buff);
		tdev->tx_buff = NULL;
		kfree(tdev->rx_buff);
		tdev->rx_buff = NULL;
		kfree(tdev);
		tdev = NULL;
	}
	return rc;
}

static int touch_driver_get_frame(struct thp_device *tdev,
	char *buf, unsigned int len)
{
	unsigned char get_frame_cmd = 0x20; /* read frame data command */
	uint8_t *w_buf = NULL;
	int ret;

	if (!tdev) {
		thp_log_info("%s: input dev null\n", __func__);
		return -ENOMEM;
	}

	if (!len) {
		thp_log_info("%s: read len illegal\n", __func__);
		return -ENOMEM;
	}

	w_buf = tdev->tx_buff;

	memset(tdev->tx_buff, 0, THP_MAX_FRAME_SIZE);
	/* write cmd 0x20 and get frame */
	w_buf[0] = (uint8_t)(spi_read_mask(get_frame_cmd));
	ret = touch_driver_spi_read_write(tdev->thp_core->sdev,
		w_buf, buf, len);
	if (ret)
		thp_log_err("%s, spi transfer failed\n", __func__);

	return 0;
}

static int touch_driver_resume(struct thp_device *tdev)
{
	struct thp_core_data *cd = NULL;

	thp_log_info("%s: called\n", __func__);
	cd = tdev->thp_core;
#ifndef CONFIG_HUAWEI_THP_MTK
	gpio_set_value(tdev->gpios->cs_gpio, 1);
	touch_driver_sw_reset_idle(tdev->sdev);
	gpio_set_value(tdev->gpios->rst_gpio, 1);
	thp_time_delay(tdev->timing_config.resume_reset_after_delay_ms);
#else
	if (tdev->thp_core->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return 0;
	}
	pinctrl_select_state(tdev->thp_core->pctrl, cd->mtk_pinctrl.cs_high);
	touch_driver_sw_reset_idle(tdev->sdev);
	pinctrl_select_state(tdev->thp_core->pctrl, cd->mtk_pinctrl.reset_high);
	thp_time_delay(tdev->timing_config.resume_reset_after_delay_ms);
#endif
	return 0;
}

static int touch_driver_wrong_touch(struct thp_device *tdev)
{
	if ((!tdev) || (!tdev->thp_core)) {
		thp_log_err("%s: input dev is null\n", __func__);
		return -EINVAL;
	}

	if (tdev->thp_core->support_gesture_mode) {
		mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
		tdev->thp_core->easy_wakeup_info.off_motion_on = true;
		mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
		thp_log_info("%s, done\n", __func__);
	}
	return 0;
}

static int gesture_data_read_write_by_frame(struct thp_device *tdev,
	uint8_t *r_buf, uint8_t *w_buf)
{
	int retval;
	u32 i;

	w_buf[0] = (uint8_t)(spi_read_mask(NVT_GET_FRAME_CMD));
	for (i = 0; i < tdev->thp_core->gesture_retry_times; i++) {
		retval = touch_driver_spi_read_write(tdev->thp_core->sdev,
			w_buf, r_buf, DEBUG_GESTURE_DATA_LEN);
		if (retval == 0)
			break;
		thp_log_info("%s: spi write abnormal retval %d retry\n",
			__func__, retval);
		msleep(WAIT_FOR_SPI_BUS_READ_DELAY); /* retry time delay */
	}
	/* r_buf bit 6 and 7 are used to check gesture id */
	if ((tdev->thp_core->support_vendor_ic_type == N9Z_SERIES) && (tdev->thp_core->pen_supported) &&
		((r_buf[6] == STYLUS_GESTURE_ID) && (((~r_buf[6] + 1) & 0xFF) == r_buf[7]))) {
		return STYLUS_EVENT;
	} else if ((r_buf[6] == DOUBLE_CLICLK_GESTURE_ID) && (((~r_buf[6] + 1) & 0xFF) == r_buf[7])) {
		thp_log_info("found valid gesture id\n");
		return NO_ERR;
	}
	/* r_buf bit 6 to 11 are used to debug error gesture id info */
	thp_log_err("found invalid gesture id: %02X-%02X-%02X-%02X-%02X-%02X\n",
		r_buf[6], r_buf[7], r_buf[8], r_buf[9], r_buf[10], r_buf[11]);
	return -EINVAL;
}

static int gesture_data_read_write(struct thp_device *tdev)
{
	int retval;
	u32 i;
	uint8_t *r_buf = g_nvt_ts_spi_buf.r_buf;
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;

	if ((r_buf == NULL) || (w_buf == NULL)) {
		thp_log_info("%s: r_buf or w_buf is null\n", __func__);
		return -ENOMEM;
	}
	memset(r_buf, 0, NVT_SPI_RW_BUF_LEN);
	memset(w_buf, 0, NVT_SPI_RW_BUF_LEN);
	if ((tdev->thp_core->support_vendor_ic_type == NVT36672C_SERIES) ||
		(tdev->thp_core->support_vendor_ic_type == N9Z_SERIES))
		return gesture_data_read_write_by_frame(tdev, r_buf, w_buf);
	w_buf[0] = spi_write_mask(0x7F);
	w_buf[1] = nvt_hi_word_bit_mask(NVT_GET_GESTURE_ADDR);
	/* w_buf[2] low word bit mask */
	w_buf[2] = nvt_low_word_bit_mask(NVT_GET_GESTURE_ADDR);
	for (i = 0; i < tdev->thp_core->gesture_retry_times; i++) {
		retval = touch_driver_spi_read_write(tdev->thp_core->sdev,
			w_buf, NULL, SEND_CMD_LEN);
		if (retval == 0)
			break;
		thp_log_info("%s: spi write abnormal retval %d retry\n",
			__func__, retval);
		msleep(WAIT_FOR_SPI_BUS_READ_DELAY); /* retry time delay */
	}

	memset(r_buf, 0, NVT_SPI_RW_BUF_LEN);
	memset(w_buf, 0, NVT_SPI_RW_BUF_LEN);
	w_buf[0] = (uint8_t)(spi_read_mask(NVT_GET_GESTURE_ADDR));
	retval = touch_driver_spi_read_write(tdev->thp_core->sdev,
		w_buf, r_buf, GESTURE_DATA_LEN);
	if (retval < 0) {
		thp_log_err("%s: spi read abnormal retval %d\n",
			__func__, retval);
		return -EINVAL;
	}
	/* calc r_buf[2] and r_buf[3] to check data is valid */
	if (((~r_buf[2] + 1) & 0xFF) != r_buf[3])
		return -EINVAL;
	/* r_buf[2] is gesture ID, r_buf[3] is CheckSum data */
	if ((r_buf[2] != 0x7F) && (r_buf[3] != 0x81)) {
		thp_log_err("%s: invalid gesture ID!\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s, gesture trigger\n", __func__);
	return NO_ERR;
}

static int touch_driver_gesture_report(struct thp_device *tdev,
	unsigned int *gesture_wakeup_value)
{
	int ret;

	if ((!tdev) || (!tdev->thp_core) || (!tdev->thp_core->sdev)) {
		thp_log_info("%s: input dev null\n", __func__);
		return -ENOMEM;
	}
	if ((!gesture_wakeup_value) ||
		(!tdev->thp_core->support_gesture_mode)) {
		thp_log_err("%s, gesture not support\n", __func__);
		return -EINVAL;
	}

	if ((tdev->thp_core->support_vendor_ic_type == N9Z_SERIES) &&
		(tdev->thp_core->support_spi_resume_delay))
		msleep(WAIT_FOR_SPI_BUS_RESUMED_DELAY_QCOM);

	ret = gesture_data_read_write(tdev);
	if (ret < 0) {
		thp_log_err("%s: gesture data read or write fail!\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
	if (tdev->thp_core->easy_wakeup_info.off_motion_on == true) {
		tdev->thp_core->easy_wakeup_info.off_motion_on = false;
		*gesture_wakeup_value = TS_DOUBLE_CLICK;
		if ((tdev->thp_core->support_vendor_ic_type == N9Z_SERIES) && (ret == STYLUS_EVENT))
			*gesture_wakeup_value = TS_STYLUS_WAKEUP_TO_MEMO;
	}
	mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);

	return 0;
}

static int touch_driver_read_fw_stamp(struct thp_device *tdev,
	uint8_t *stamp_val, unsigned int stamp_addr)
{
	uint8_t *r_buf = g_nvt_ts_spi_buf.r_buf;
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	int rw_len;
	int ret;

	if ((r_buf == NULL) || (w_buf == NULL)) {
		thp_log_info("%s: r_buf or w_buf is null\n", __func__);
		return -ENOMEM;
	}
	memset(r_buf, 0, NVT_SPI_RW_BUF_LEN);
	memset(w_buf, 0, NVT_SPI_RW_BUF_LEN);
	/* Read FW cmd stamp */
	w_buf[0] = spi_write_mask(0x7F);
	w_buf[1] = nvt_hi_word_bit_mask(stamp_addr);
	/* w_buf[2] low word bit mask */
	w_buf[2] = nvt_low_word_bit_mask(stamp_addr);
	rw_len = SEND_CMD_LEN;
	ret = touch_driver_spi_read_write(tdev->thp_core->sdev,
		w_buf, NULL, rw_len);
	if (ret) {
		thp_log_err("%s, send write stamp cmd failed\n", __func__);
		return ret;
	}

	memset(w_buf, 0, SEND_CMD_LEN);
	memset(r_buf, 0, SEND_CMD_LEN);
	w_buf[0] = (uint8_t)(spi_read_mask((stamp_addr & 0x0000007F)));
	rw_len = SEND_CMD_LEN;
	ret = touch_driver_spi_read_write(tdev->thp_core->sdev,
		w_buf, r_buf, rw_len);
	if (ret) {
		thp_log_err("%s, send read stamp cmd failed\n", __func__);
		return ret;
	}

	thp_log_info("%s current stamp is %x %x [%x]\n", __func__,
		r_buf[0], r_buf[1], r_buf[2]); /* byte2 is valid value */
	*stamp_val = r_buf[2]; /* byte2 is valid value */

	return NO_ERR;
}

static int touch_driver_write_cmd_addr(struct thp_device *tdev,
	unsigned int cmd_addr)
{
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	int rw_len;
	int ret;

	if (w_buf == NULL) {
		thp_log_info("%s: w_buf is null\n", __func__);
		return -ENOMEM;
	}
	/* write cmd @0x21C50 */
	memset(w_buf, 0, NVT_SPI_RW_BUF_LEN);
	w_buf[0] = spi_write_mask(0x7F);
	w_buf[1] = nvt_hi_word_bit_mask(cmd_addr);
	/* w_buf[2] low word bit mask */
	w_buf[2] = nvt_low_word_bit_mask(cmd_addr);
	rw_len = SEND_CMD_LEN;

	ret = touch_driver_spi_read_write(tdev->thp_core->sdev,
		w_buf, NULL, rw_len);
	if (ret) {
		thp_log_err("%s, send write 0x21C50 failed\n", __func__);
		return ret;
	}
	return NO_ERR;
}

static int touch_driver_set_cmd_args(struct thp_device *tdev, uint8_t *w_buf)
{
	if ((!w_buf) || (!tdev)) {
		thp_log_err("%s, wbuf or tdev is null\n", __func__);
		return -EINVAL;
	}

	/*
	 * 3:need to send stylus status
	 * 4:stylus connect or disconnect
	 */
	if ((send_status_flag == STYLUS_CONNECT) &&
		(tdev->thp_core->support_vendor_ic_type == N9Z_SERIES) &&
		(tdev->thp_core->send_bt_status_to_fw)) {
		w_buf[3] = 0x02;
		w_buf[4] = 0x01;
	} else if ((send_status_flag == STYLUS_DISCONNECT) &&
		(tdev->thp_core->support_vendor_ic_type == N9Z_SERIES) &&
		(tdev->thp_core->send_bt_status_to_fw)) {
		w_buf[3] = 0x02;
		w_buf[4] = 0x00;
	} else {
		w_buf[3] = 0x00;
		w_buf[4] = 0x00;
	}

	return 0;
}

static int touch_driver_send_fw_cmd(struct thp_device *tdev, uint8_t cmd)
{
	uint8_t *w_buf = g_nvt_ts_spi_buf.w_buf;
	uint8_t cur_idx;
	uint8_t loop;
	uint8_t stamp_val;
	int ret;
	unsigned int cmd_addr = NVT_CMD_ADDR;
	unsigned int stamp_addr = NVT_CMD_STAMP_ADDR;

	if (tdev->thp_core->support_vendor_ic_type == NVT36672C_SERIES) {
		cmd_addr = NVT36672C_CMD_ADDR;
		stamp_addr = NVT36672C_CMD_STAMP_ADDR;
	}

	if (tdev->thp_core->support_vendor_ic_type == N9Z_SERIES) {
		cmd_addr = N9Z_CMD_ADDR;
		stamp_addr = N9Z_CMD_STAMP_ADDR;
	}

	if (w_buf == NULL) {
		thp_log_info("%s: w_buf is null\n", __func__);
		return -ENOMEM;
	}
	ret = touch_driver_read_fw_stamp(tdev, &stamp_val, stamp_addr);
	if (ret) {
		thp_log_err("%s, fail read fw stamp\n", __func__);
		return ret;
	}
	thp_log_info("read stamp value is %x, cmd is %x\n", stamp_val, cmd);
	ret = touch_driver_write_cmd_addr(tdev, cmd_addr);
	if (ret)
		return ret;
	/*
	 * 0:r/w
	 * 1:cmd
	 * 2:handshake
	 * 3/4:arg
	 * 5:stamp, use previous number in case collision with HAL
	 * 6:checksum
	 */
	memset(w_buf, 0, NVT_SPI_RW_BUF_LEN);
	w_buf[0] = (uint8_t)(spi_write_mask((cmd_addr & 0x0000007F)));
	w_buf[1] = cmd;
	w_buf[2] = 0xBB;

	ret = touch_driver_set_cmd_args(tdev, w_buf);
	if (ret)
		return ret;

	w_buf[5] = stamp_val - 1;
	w_buf[6] = (uint8_t)(~(w_buf[1] + w_buf[2] + w_buf[3] +
		w_buf[4] + w_buf[5]) + 1);
	ret = touch_driver_spi_read_write(tdev->thp_core->sdev,
		w_buf, NULL, D_SEND_CMD_LEN);
	if (ret < 0) {
		thp_log_err("%s, send write %u failed\n", __func__, cmd);
		return ret;
	}

	/* Check if cmd was received and wait handle 50ms */
	cur_idx = w_buf[5];
	stamp_val = 0;
	for (loop = 0; loop < D_SEND_CMD_RETEY; loop++) {
		if (tdev->thp_core->send_cmd_retry_delay_ms)
			mdelay(tdev->thp_core->send_cmd_retry_delay_ms);
		else
			mdelay(1); /* wait fw respone, retry delay time */
		ret = touch_driver_read_fw_stamp(tdev, &stamp_val, stamp_addr);
		if (ret) {
			thp_log_err("%s, fail read fw stamp\n", __func__);
			return ret;
		}
		if (stamp_val == cur_idx) {
			thp_log_info("%s cmd successfully received\n",
				__func__);
			/* 50ms spec, make sure gesture enter success */
			msleep(50);
			return NO_ERR;
		}
	}
#if defined(CONFIG_HUAWEI_DSM)
	thp_dmd_report(DSM_TPHOSTPROCESSING_DEV_GESTURE_EXP1,
		"%s, stamp_val is 0x%x, cur_idx is 0x%x\n", __func__,
		stamp_val, cur_idx);
#endif
	thp_log_info("%s cmd failed!\n", __func__);
	return -EINVAL;
}

static int touch_driver_second_poweroff(void)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_device *tdev = cd->thp_dev;

	return touch_driver_send_fw_cmd(tdev, NVT_CMD_ENTER_CUSTOMIZED_DP_DSTB);
}

static int touch_driver_nt_bt_handler(struct thp_device *tdev,
	bool delay_enable)
{
	unsigned int stylus3_status;
	int i;
	int ret;

	stylus3_status = atomic_read(&tdev->thp_core->last_stylus3_status);

	if (delay_enable == true)
		msleep(SEND_BT_CONNECT_STATUS_AFTER_RESET_DELAY);

	for (i = 0; i < BT_CONNECT_CMD_RETRY; i++) {
		if ((stylus3_status & STYLUS3_CONNECT) == STYLUS3_CONNECT) {
			send_status_flag = STYLUS_CONNECT;
			ret = touch_driver_send_fw_cmd(tdev, STATUS_CMD);
		} else if ((stylus3_status & STYLUS3_CONNECT) != STYLUS3_CONNECT) {
			send_status_flag = STYLUS_DISCONNECT;
			ret = touch_driver_send_fw_cmd(tdev, STATUS_CMD);
		}
		if (!ret)
			break;
	}
	if (i == BT_CONNECT_CMD_RETRY) {
		thp_log_err("%s, send stylus status fail, retry = %d\n", __func__, i);
		ret = -EINVAL;
	}

	send_status_flag = 0;

	return ret;
}

static void touch_driver_enter_gesture_mode(
	struct thp_device *tdev)
{
	int retval;

	retval = touch_driver_send_fw_cmd(tdev,
		NVT_CMD_ENTER_GESTURE_MODE);
	if (retval < 0)
		thp_log_err("%s, enter gesture mode failed\n", __func__);
	mutex_lock(&tdev->thp_core->thp_wrong_touch_lock);
	tdev->thp_core->easy_wakeup_info.off_motion_on = true;
	mutex_unlock(&tdev->thp_core->thp_wrong_touch_lock);
}

static int touch_driver_suspend(struct thp_device *tdev)
{
	int pt_test_mode;
	struct thp_core_data *cd = NULL;
	enum ts_sleep_mode gesture_status;

	thp_log_info("%s: called\n", __func__);
	if ((!tdev) || (!tdev->thp_core) || (!tdev->thp_core->sdev)) {
		thp_log_err("%s: tdev null\n", __func__);
		return -EINVAL;
	}
	cd = tdev->thp_core;
	pt_test_mode = is_pt_test_mode(tdev);
	gesture_status = tdev->thp_core->easy_wakeup_info.sleep_mode;

	if (tdev->thp_core->support_gesture_mode &&
		tdev->thp_core->lcd_gesture_mode_support) {
		if (pt_test_mode) { /* PT test spec */
			thp_log_info("%s: PT sleep mode\n", __func__);
			/* gesture mode don't need */
			(void)touch_driver_send_fw_cmd(tdev,
				NVT_CMD_ENTER_CUSTOMIZED_DP_DSTB);
#ifndef CONFIG_HUAWEI_THP_MTK
			gpio_set_value(tdev->gpios->cs_gpio, 0);
#endif
		} else if (gesture_status == TS_GESTURE_MODE &&
			tdev->thp_core->lcd_gesture_mode_support) {
			thp_log_info("%s: TS_GESTURE_MODE\n", __func__);
			touch_driver_enter_gesture_mode(tdev);
#ifdef CONFIG_LCDKIT_DRIVER
			ts_kit_gesture_func = TS_GESTURE_MODE;
#endif
		} else { /* power off spec */
			thp_log_info("%s: power off mode\n", __func__);
			/* gesture mode don't need */
			(void)touch_driver_send_fw_cmd(tdev,
				NVT_CMD_ENTER_CUSTOMIZED_DP_DSTB);
#ifndef CONFIG_HUAWEI_THP_MTK
			if (!cd->suspend_no_reset)
				gpio_set_value(tdev->gpios->rst_gpio, 0);
			gpio_set_value(tdev->gpios->cs_gpio, 0);
#ifdef CONFIG_LCDKIT_DRIVER
			ts_kit_gesture_func = TS_POWER_OFF_MODE;
#endif
#else
			if (cd->support_pinctrl == 0) {
				thp_log_info("%s: not support pinctrl\n",
					__func__);
				return 0;
			}

			pinctrl_select_state(cd->pctrl,
				cd->mtk_pinctrl.reset_low);
#endif
		}
		if (cd->suspend_no_reset)
			thp_time_delay(NVT_SUSPEND_AFTER_DELAY_MS);
		else
			thp_time_delay(
				tdev->timing_config.suspend_reset_after_delay_ms);
		return 0;
	}

#ifndef CONFIG_HUAWEI_THP_MTK
	if (pt_test_mode) {
		thp_log_info("%s: sleep mode\n", __func__);
		gpio_set_value(tdev->gpios->cs_gpio, 0);
	} else {
		thp_log_info("%s: power off mode\n", __func__);
		gpio_set_value(tdev->gpios->rst_gpio, 0);
		gpio_set_value(tdev->gpios->cs_gpio, 0);
	}
#else
	if (cd->support_pinctrl == 0) {
		thp_log_info("%s: not support pinctrl\n", __func__);
		return 0;
	}

	if (pt_test_mode) {
		thp_log_info("%s: sleep mode\n", __func__);
		pinctrl_select_state(cd->pctrl, cd->mtk_pinctrl.cs_low);
	} else {
		thp_log_info("%s: power off mode\n", __func__);
		pinctrl_select_state(cd->pctrl, cd->pins_idle);
	}
#endif

	thp_time_delay(tdev->timing_config.suspend_reset_after_delay_ms);

	return 0;
}

static void touch_driver_exit(struct thp_device *tdev)
{
	thp_log_info("%s: called\n", __func__);
	kfree(g_nvt_ts_spi_buf.w_buf);
	g_nvt_ts_spi_buf.w_buf = NULL;
	kfree(g_nvt_ts_spi_buf.r_buf);
	g_nvt_ts_spi_buf.r_buf = NULL;
	kfree(tdev->tx_buff);
	tdev->tx_buff = NULL;
	kfree(tdev->rx_buff);
	tdev->rx_buff = NULL;
	kfree(tdev);
	tdev = NULL;
}

struct thp_device_ops nova_dev_ops = {
	.init = touch_driver_init,
	.detect = touch_driver_chip_detect,
	.get_frame = touch_driver_get_frame,
	.resume = touch_driver_resume,
	.suspend = touch_driver_suspend,
	.exit = touch_driver_exit,
	.chip_wrong_touch = touch_driver_wrong_touch,
	.chip_gesture_report = touch_driver_gesture_report,
	.second_poweroff = touch_driver_second_poweroff,
	.bt_handler = touch_driver_nt_bt_handler,
};

static int __init touch_driver_module_init(void)
{
	int rc;
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

	dev->ic_name = NOVATECH_IC_NAME;
	dev->ops = &nova_dev_ops;
	if (cd && cd->fast_booting_solution) {
		thp_send_detect_cmd(dev, NO_SYNC_TIMEOUT);
		/*
		 * The thp_register_dev will be called later to complete
		 * the real detect action.If it fails, the detect function will
		 * release the resources requested here.
		 */
		return 0;
	}
	rc = thp_register_dev(dev);
	if (rc) {
		thp_log_err("%s: register fail\n", __func__);
		goto err;
	}

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

#ifdef CONFIG_HUAWEI_THP_QCOM
late_initcall(touch_driver_module_init);
#else
module_init(touch_driver_module_init);
#endif
module_exit(touch_driver_module_exit);
MODULE_AUTHOR("novatech");
MODULE_DESCRIPTION("novatech driver");
MODULE_LICENSE("GPL");
