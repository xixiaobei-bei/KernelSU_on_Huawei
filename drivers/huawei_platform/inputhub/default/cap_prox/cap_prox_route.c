/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: cap prox route source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "cap_prox_route.h"

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <asm/io.h>
#include <asm/memory.h>
#include <securec.h>

#include "contexthub_route.h"

#define MIN_CAP_PROX_MODE             0
#define MAZ_CAP_PROX_MODE             2
#define ABOV_WRITE_DATA_LENGTH 36

ssize_t attr_cap_prox_data_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret;
	int32_t err_no = 0;

	if (!buf) {
		hwlog_err("attr_cap_prox_data_mode_store: buf is NULL\n");
		return -EINVAL;
	}
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;
	if (val < MIN_CAP_PROX_MODE || val > MAZ_CAP_PROX_MODE) {
		hwlog_err("cap_prox_data_mode error, val=%lu\n", val);
		return -1;
	}

	ret = send_subcmd_data_to_mcu_lock(TAG_CAP_PROX, SUB_CMD_SET_DATA_MODE,
		(const void *)&val, sizeof(val), &err_no);
	if (ret) {
		hwlog_err("send tag %d sar mode to mcu fail,ret=%d\n",
			(int32_t)TAG_CAP_PROX, ret);
		ret = -1;
	} else {
		if (err_no != 0) {
			hwlog_err("send sar mode to mcu fail\n");
			ret = -1;
		} else {
			hwlog_info("send sar mode to mcu succes\n");
			ret = count;
		}
	}
	return ret;
}

static int abov_str_to_hex(const char *string, uint8_t *buf, int len)
{
	uint8_t high, low;
	int index;
	int i = 0;

	for (index = 0; index < len; index += 2) { /* 2 letters one byte */
		high = string[index];
		low = string[index+1];

		if (high >= '0' && high <= '9') {
			high = high - '0';
		} else if (high >= 'A' && high <= 'F') {
			high = high - 'A' + TO_DECIMALISM;
		} else if (high >= 'a' && high <= 'f') {
			high = high - 'a' + TO_DECIMALISM;
		} else {
			hwlog_info("high = %x\n", high);
			return -1;
		}
		if (low >= '0' && low <= '9') {
			low = low - '0';
		} else if (low >= 'A' && low <= 'F') {
			low = low - 'A' + TO_DECIMALISM;
		} else if (low >= 'a' && low <= 'f') {
			low = low - 'a' + TO_DECIMALISM;
		} else {
			hwlog_info("low = %x\n", low);
			return -1;
		}
		buf[i++] = (high << 4) | low; /* 4 bits */
	}
	return 0;
}

ssize_t attr_cap_prox_abov_data_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint8_t data[ABOV_WRITE_DATA_LENGTH] = { 0 };
	int ret;
	int32_t err_no = 0;

	if (buf == NULL) {
		hwlog_err("%s: buf is NULL\n", __func__);
		return -EINVAL;
	}
	/* 2 letters one byte, include '\0' */
	if (count != (ABOV_WRITE_DATA_LENGTH * 2 + 1)) {
		hwlog_err("%s:wrong input,count is %d\n", __func__, (int)count);
		return -EINVAL;
	}
	/* 2 letters one byte, data length is 72 */
	ret = abov_str_to_hex(buf, data, ABOV_WRITE_DATA_LENGTH * 2);
	if (ret) {
		hwlog_err("%s: str2hex failed\n", __func__);
		return count;
	}

	ret = send_subcmd_data_to_mcu_lock(TAG_CAP_PROX,
		SUB_CMD_SET_ADD_DATA_REQ, (const void *)data, sizeof(data),
		&err_no);
	if (ret) {
		hwlog_err("send tag %d sar mode to mcu fail,ret=%d\n",
			(int32_t)TAG_CAP_PROX, ret);
		ret = -1;
	} else {
		if (err_no != 0) {
			hwlog_err("%s send abov data to mcu fail\n", __func__);
			ret = -1;
		} else {
			hwlog_info("%s send abov data to mcu succes\n", __func__);
			ret = count;
		}
	}
	return ret;
}

#define  ABOV_CHECKSUM_DATA_LENGTH 2
static int32_t abov_bootloader_verify_result;
ssize_t attr_abov_bootloader_verify(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct write_info pkg_ap = { 0 };
	struct read_info pkg_mcu = { 0 };
	pkt_parameter_req_t spkt;
	struct pkt_header *shd = (struct pkt_header *)&spkt;
	uint8_t data[ABOV_CHECKSUM_DATA_LENGTH] = {0};
	int ret;

	if (memset_s(&spkt, sizeof(spkt), 0, sizeof(spkt)) != EOK)
		return -1;

	if (!buf) {
		hwlog_err("%s: buf is NULL\n", __func__);
		return -EINVAL;
	}
	/* 2 letters one byte, include '\0' */
	if (count != (ABOV_CHECKSUM_DATA_LENGTH * 2 + 1)) {
		hwlog_err("%s wrong input, count is %d\n", __func__, (int)count);
		return -EINVAL;
	}
	/* 2 letters one byte */
	ret = abov_str_to_hex(buf, data, ABOV_CHECKSUM_DATA_LENGTH * 2);
	if (ret) {
		hwlog_err("%s: str2hex failed\n", __func__);
		return count;
	}

	spkt.subcmd = SUB_CMD_ADDITIONAL_INFO;
	pkg_ap.tag = TAG_CAP_PROX;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = sizeof(data) + SUBCMD_LEN;
	if (memcpy_s(spkt.para, sizeof(spkt.para), data, sizeof(data)) != EOK)
		return -1;

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret != 0) {
		hwlog_err("%s send tag %d sar mode to mcu fail,ret=%d\n",
			__func__, pkg_ap.tag, ret);
		ret = -1;
	} else {
		if (pkg_mcu.errno != 0) {
			hwlog_err("%s send abov data to mcu return fail\n",
				__func__);
			abov_bootloader_verify_result = -1;
			ret = -1;
		} else {
			abov_bootloader_verify_result = 0;
			hwlog_info("%s mcu return succes,result is %d\n",
				__func__, abov_bootloader_verify_result);
			ret = count;
		}
	}
	return ret;
}

ssize_t attr_abov_bootloader_verify_result_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!dev || !attr || !buf)
		return -1;
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d",
		abov_bootloader_verify_result);
}

#define ABOV_ENTER_BOOTLOADER_CMD 5
static uint8_t  abov_device_id;
ssize_t attr_abov_bootloader_enter(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct write_info pkg_ap = { 0 };
	struct read_info pkg_mcu = { 0 };
	pkt_parameter_req_t spkt;
	struct pkt_header *shd = (struct pkt_header *)&spkt;
	unsigned long val = 0;
	int ret;

	if (memset_s(&spkt, sizeof(spkt), 0, sizeof(spkt)) != EOK)
		return -1;

	if (!buf) {
		hwlog_err("%s: buf is NULL\n", __func__);
		return -EINVAL;
	}
	if (strict_strtoul(buf, TO_DECIMALISM, &val)) {
		hwlog_err("%s: strtoul fail\n", __func__);
		return -EINVAL;
	}
	if (val != ABOV_ENTER_BOOTLOADER_CMD) {
		hwlog_err("%s: wrong value input\n", __func__);
		return -EINVAL;
	}

	spkt.subcmd = SUB_CMD_SELFCALI_REQ;
	pkg_ap.tag = TAG_CAP_PROX;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = sizeof(val) + SUBCMD_LEN;
	if (memcpy_s(spkt.para, sizeof(spkt.para), &val, sizeof(val)) != EOK)
		return -1;

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("%s send tag %d sar mode to mcu fail,ret=%d\n",
			__func__, pkg_ap.tag, ret);
		ret = -1;
	} else {
		if (pkg_mcu.errno != 0) {
			hwlog_err("%s send abov data to mcu return fail\n",
				__func__);
			ret = -1;
		} else {
			if (memcpy_s(&abov_device_id, sizeof(abov_device_id),
				pkg_mcu.data, sizeof(abov_device_id)) != EOK)
				return -1;
			hwlog_info("%s mcu return succes,device is %d\n",
				__func__, abov_device_id);
			ret = count;
		}
	}
	return ret;
}

ssize_t attr_abov_bootloader_enter_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!dev || !attr || !buf)
		return -1;
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%x",
		abov_device_id);
}

ssize_t abov_reg_dump_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct sar_platform_data *pf_data = NULL;
	uint32_t reg = 0;
	uint32_t val = 0;
	uint8_t buf_temp[DEBUG_DATA_LENGTH] = { 0 };
	int res;

	pf_data = cap_prox_get_platform_data(TAG_CAP_PROX);
	if (pf_data == NULL)
		return -1;
	if (count < 5) { /* input length must > 5 */
		hwlog_err("%s, invalid input", __func__);
		return -EINVAL;
	}
	hwlog_info("%s,buf 0x%x,0x%x,0x%x\n", __func__, buf[0], buf[1], buf[2]);

	if (sscanf_s(buf, "%x,%x", &reg, &val) == 2) { /* 2 parameters */
		hwlog_info("%s,reg = 0x%02x, val = 0x%02x\n", __func__,
			*(uint8_t *)&reg, *(uint8_t *)&val);
		buf_temp[0] = reg;
		buf_temp[1] = val;
		res = mcu_i2c_rw(TAG_I2C, pf_data->cfg.bus_num,
			(uint8_t)(pf_data->cfg.i2c_address),
			buf_temp, 2, NULL, 0); /* 2 bytes */
		if (res < 0)
			hwlog_err("%s: i2c write fail\n", __func__);
	}
	return count;
}

ssize_t abov_reg_dump_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sar_platform_data *pf_data = NULL;
	uint8_t reg_value = 0;
	uint8_t i;
	int ret = 0;
	int res;

	pf_data = cap_prox_get_platform_data(TAG_CAP_PROX);
	if (pf_data == NULL)
		return -1;
	if (!dev || !attr || !buf)
		return -1;
	for (i = 0; i < 0x26; i++) { /* reg value */
		res = mcu_i2c_rw(TAG_I2C, pf_data->cfg.bus_num,
			(uint8_t)(pf_data->cfg.i2c_address),
			&i, 1, &reg_value, 1);
		if (res < 0)
			hwlog_err("%s: i2c read fail, i=%x\n", __func__, i);
		ret += snprintf_s(buf+ret, PAGE_SIZE, PAGE_SIZE - 1,
			"(0x%02x)=0x%02x\n", i, reg_value);
	}
	for (i = 0x46; i < 0x4c; i++) { /* reg value */
		res = mcu_i2c_rw(TAG_I2C, pf_data->cfg.bus_num,
			(uint8_t)(pf_data->cfg.i2c_address),
			&i, 1, &reg_value, 1);
		if (res < 0)
			hwlog_err("%s: i2c read fail, i=%x\n", __func__, i);
		ret += snprintf_s(buf+ret, PAGE_SIZE, PAGE_SIZE - 1,
			"(0x%02x)=0x%02x\n", i, reg_value);
	}

	for (i = 0x80; i < 0x8C; i++) { /* reg value */
		res = mcu_i2c_rw(TAG_I2C, pf_data->cfg.bus_num,
			(uint8_t)(pf_data->cfg.i2c_address),
			&i, 1, &reg_value, 1);
		if (res < 0)
			hwlog_err("%s: i2c read fail, i=%x\n", __func__, i);
		ret += snprintf_s(buf + ret, PAGE_SIZE, PAGE_SIZE - 1,
			"(0x%02x)=0x%02x\n", i, reg_value);
	}
	return ret;
}
