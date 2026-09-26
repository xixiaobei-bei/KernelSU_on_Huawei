/*
 * hwcxext_codec_info.c
 *
 * hwcxext codec info driver
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
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
#include <linux/module.h>
#include <linux/slab.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>
#include "hwcxext_codec_info.h"

#define HWLOG_TAG hwcxext_codec_info
HWLOG_REGIST();

#define HWCXEXT_INFO_BUF_MAX           512
#define HWCXEXT_REG_CTL_COUNT          32
#define HWCXEXT_CMD_PARAM_OFFSET       2
#define HWCXEXT_REG_R_PARAM_NUM_MIN    1
#define HWCXEXT_REG_W_PARAM_NUM_MIN    2
#define HWCXEXT_REG_OPS_BULK_PARAM     1
#define HWCXEXT_STR_TO_INT_BASE        16

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) ((void)(x))
#endif

#define HWCXEXT_INFO_HELP \
	"Usage:\n" \
	"dump_regs: echo d > hwcxext_codec_reg_ctl\n" \
	"read_regs:\n" \
	"echo \"r,reg_addr,[bulk_count_once]\" > hwcxext_codec_reg_ctl\n" \
	"write_regs:\n" \
	"echo \"w,reg_addr,reg_value,[reg_value2...]\" > hwcxext_codec_reg_ctl\n"

struct hwcxext_codec_reg_ctl_params {
	char cmd;
	int params_num;
	union {
		int params[HWCXEXT_REG_CTL_COUNT];
		struct {
			int addr_r;
			int bulk_r;
		};
		struct {
			int addr_w;
			int value[0];
		};
	};
};

static struct hwcxext_codec_info_ctl_ops *g_info_ops;
static struct hwcxext_codec_regmap_cfg *g_cx_regmap_cfg;
static char hwcxext_info_buffer[HWCXEXT_INFO_BUF_MAX] = {0};
static bool g_cx_info_ctl_support = false;
/* codec info function is bypass, if want to use, pls set it to false */
static bool g_cx_info_set_bypass = true;

void hwcxext_codec_register_info_ctl_ops(
	struct hwcxext_codec_info_ctl_ops *ops)
{
	if (ops == NULL)
		return;

	g_info_ops = ops;
}

void hwcxext_codec_info_store_regmap(
	struct hwcxext_codec_regmap_cfg *cfg)
{
	if (cfg == NULL)
		return;

	g_cx_regmap_cfg = cfg;
}

void hwcxext_codec_info_set_ctl_support(bool status)
{
	g_cx_info_ctl_support = status;
}

static struct hwcxext_codec_reg_ctl_params g_hwcxext_ctl_params;
static int hwcxext_reg_ctl_flag;

static int hwcxext_codec_check_init_status(char *buffer)
{
	int ret;

	if (!g_cx_info_ctl_support) {
		ret = snprintf_s(buffer, (unsigned long)HWCXEXT_INFO_BUF_MAX,
			(unsigned long)HWCXEXT_INFO_BUF_MAX - 1,
			"not support hwcxext adc info ctl\n");
		if (ret < 0)
			hwlog_err("%s: snprintf_s is failed\n", __func__);

		return ret;
	}

	if (hwcxext_reg_ctl_flag == 0) {
		ret = snprintf_s(buffer, (unsigned long)HWCXEXT_INFO_BUF_MAX,
			(unsigned long)HWCXEXT_INFO_BUF_MAX - 1,
			HWCXEXT_INFO_HELP);
		if (ret < 0)
			hwlog_err("%s: snprintf_s is failed\n", __func__);

		return ret;
	}

	return 0;
}

static int hwcxext_codec_get_reg_ctl(char *buffer,
	const struct kernel_param *kp)
{
	int ret;

	UNUSED_PARAMETER(kp);
	if (buffer == NULL) {
		hwlog_err("%s: buffer is NULL\n", __func__);
		return -EINVAL;
	}

	if (hwcxext_codec_check_init_status(buffer) < 0)
		return -EINVAL;

	if (strlen(hwcxext_info_buffer) > 0) {
		ret = snprintf_s(buffer, (unsigned long)HWCXEXT_INFO_BUF_MAX,
			(unsigned long)HWCXEXT_INFO_BUF_MAX - 1,
			hwcxext_info_buffer);
		if (ret < 0)
			hwlog_err("%s: snprintf_s is failed\n", __func__);

		if (memset_s(hwcxext_info_buffer, HWCXEXT_INFO_BUF_MAX,
			0, HWCXEXT_INFO_BUF_MAX) != EOK)
			hwlog_err("%s: memset_s is failed\n", __func__);
	} else {
		ret = snprintf_s(buffer, (unsigned long)HWCXEXT_INFO_BUF_MAX,
			(unsigned long)HWCXEXT_INFO_BUF_MAX - 1,
			"hwcxext reg_ctl success\n"
			"(dmesg -c | grep hwcxext)");
		if (ret < 0)
			hwlog_err("%s: snprintf_s is failed\n", __func__);
	}

	return ret;
}

static int hwcxext_codec_info_parse_reg_ctl(const char *val)
{
	char buf[HWCXEXT_INFO_BUF_MAX] = {0};
	char *tokens = NULL;
	char *pbuf = NULL;
	int index = 0;
	int ret;

	if (val == NULL) {
		hwlog_err("%s: val is NULL\n", __func__);
		return -EINVAL;
	}

	if (memset_s(&g_hwcxext_ctl_params, sizeof(g_hwcxext_ctl_params),
		0, sizeof(g_hwcxext_ctl_params)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);

	/* ops cmd */
	hwlog_info("%s: val = %s\n", __func__, val);
	if (strncpy_s(buf, (unsigned long)HWCXEXT_INFO_BUF_MAX,
		val, (unsigned long)(HWCXEXT_INFO_BUF_MAX - 1)) != EOK)
		hwlog_err("%s: strncpy_s is failed\n", __func__);

	g_hwcxext_ctl_params.cmd = buf[0];
	pbuf = &buf[HWCXEXT_CMD_PARAM_OFFSET];

	/* parse dump/read/write ops params */
	do {
		tokens = strsep(&pbuf, ",");
		if (tokens == NULL)
			break;

		ret = kstrtoint(tokens, HWCXEXT_STR_TO_INT_BASE,
			&g_hwcxext_ctl_params.params[index]);
		if (ret < 0)
			continue;

		hwlog_info("%s: tokens %d=%s, 0x%x\n", __func__, index,
			tokens, g_hwcxext_ctl_params.params[index]);

		index++;
		if (index == HWCXEXT_REG_CTL_COUNT) {
			hwlog_info("%s: params count max is %u\n", __func__,
				HWCXEXT_REG_CTL_COUNT);
			break;
		}
	} while (true);

	g_hwcxext_ctl_params.params_num = index;
	return 0;
}

static int hwcxext_bulk_read_regs(void)
{
	int bulk_count_once = g_hwcxext_ctl_params.bulk_r;
	int addr = g_hwcxext_ctl_params.addr_r;
	unsigned char *value = NULL;
	int i;

	value = kzalloc(bulk_count_once, GFP_KERNEL);
	if (value == NULL)
		return -ENOMEM;

	regmap_bulk_read(g_cx_regmap_cfg->regmap, addr, value, bulk_count_once);
	for (i = 0; i < bulk_count_once; i++)
		hwlog_info("%s: bulk read reg 0x%x=0x%x\n", __func__,
			addr + i, value[i]);

	kfree(value);
	return 0;
}

static int hwcxext_codec_info_reg_read(void)
{
	unsigned int value = 0;
	int ret;

	if (g_hwcxext_ctl_params.params_num < HWCXEXT_REG_R_PARAM_NUM_MIN) {
		hwlog_info("%s: params_num %d < %d\n",
			__func__, g_hwcxext_ctl_params.params_num,
			HWCXEXT_REG_R_PARAM_NUM_MIN);
		return -EINVAL;
	}

	if (g_hwcxext_ctl_params.bulk_r > 0)
		return hwcxext_bulk_read_regs();

	regmap_read(g_cx_regmap_cfg->regmap,
		g_hwcxext_ctl_params.addr_r, &value);

	hwlog_info("%s:read reg 0x%x=0x%x\n", __func__,
		g_hwcxext_ctl_params.addr_r, value);

	if (memset_s(hwcxext_info_buffer, HWCXEXT_INFO_BUF_MAX,
		0, HWCXEXT_INFO_BUF_MAX) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);

	ret = snprintf_s(hwcxext_info_buffer,
		(unsigned long)HWCXEXT_INFO_BUF_MAX,
		(unsigned long)(HWCXEXT_INFO_BUF_MAX - 1), "reg 0x%08x=0x%08x\n",
		g_hwcxext_ctl_params.addr_r, value);
	if (ret < 0)
		hwlog_err("%s: snprintf_s is failed\n", __func__);

	return 0;
}

static int hwcxext_bulk_write_regs(void)
{
	unsigned char *bulk_value = NULL;
	int addr = g_hwcxext_ctl_params.addr_w;
	int bulk_count_once = g_hwcxext_ctl_params.params_num -
		HWCXEXT_REG_OPS_BULK_PARAM;
	int i;

	bulk_value = kzalloc(bulk_count_once, GFP_KERNEL);
	if (bulk_value == NULL)
		return -ENOMEM;

	/* get regs value for bulk write */
	for (i = 0; i < bulk_count_once; i++) {
		bulk_value[i] = (unsigned char)g_hwcxext_ctl_params.value[i];
		hwlog_info("%s: bulk write reg 0x%x=0x%x\n", __func__,
			addr + i, bulk_value[i]);
	}

	regmap_bulk_write(g_cx_regmap_cfg->regmap, addr,
		bulk_value, bulk_count_once);
	kfree(bulk_value);
	return 0;
}

static int hwcxext_codec_info_reg_write(void)
{
	if (g_hwcxext_ctl_params.params_num < HWCXEXT_REG_W_PARAM_NUM_MIN) {
		hwlog_info("%s: params_num %d < %d\n",
			__func__, g_hwcxext_ctl_params.params_num,
			HWCXEXT_REG_W_PARAM_NUM_MIN);
		return -EINVAL;
	}

	if (g_hwcxext_ctl_params.params_num > HWCXEXT_REG_W_PARAM_NUM_MIN)
		return hwcxext_bulk_write_regs();

	hwlog_info("%s:write reg 0x%x=0x%x\n", __func__,
		g_hwcxext_ctl_params.addr_w, g_hwcxext_ctl_params.value[0]);
	regmap_write(g_cx_regmap_cfg->regmap,
		g_hwcxext_ctl_params.addr_w, g_hwcxext_ctl_params.value[0]);
	return 0;
}

static int hwcxext_codec_info_do_reg_ctl(void)
{
	int ret;

	if (g_info_ops == NULL || g_cx_regmap_cfg == NULL ||
		g_cx_regmap_cfg->regmap == NULL) {
		hwlog_err("%s: param is NULL\n", __func__);
		return -EINVAL;
	}

	/* dump/read/write ops */
	switch (g_hwcxext_ctl_params.cmd) {
	case 'd': /* dump regs */
		ret = g_info_ops->dump_regs(g_cx_regmap_cfg);
		break;
	case 'r':
		ret = hwcxext_codec_info_reg_read();
		break;
	case 'w':
		ret = hwcxext_codec_info_reg_write();
		break;
	default:
		hwlog_info("%s: not support cmd %c/0x%x\n", __func__,
			g_hwcxext_ctl_params.cmd, g_hwcxext_ctl_params.cmd);
		ret = -EFAULT;
		break;
	}
	return ret;
}

static int hwcxext_codec_set_reg_ctl(const char *val,
	const struct kernel_param *kp)
{
	int ret;

	UNUSED_PARAMETER(kp);
	if (!g_cx_info_ctl_support) {
		hwlog_info("%s: not support hwcxext info ctl\n", __func__);
		return 0;
	}

	if (g_cx_info_set_bypass) {
		hwlog_info("%s: hwcxext info set ctl is bypass\n", __func__);
		return 0;
	}

	if (g_info_ops == NULL || g_cx_regmap_cfg == NULL ||
		g_cx_regmap_cfg->regmap == NULL) {
		hwlog_err("%s: param is NULL\n", __func__);
		return -EINVAL;
	}
	hwcxext_reg_ctl_flag = 0;

	ret = hwcxext_codec_info_parse_reg_ctl(val);
	if (ret < 0)
		return ret;

	ret = hwcxext_codec_info_do_reg_ctl();
	if (ret < 0)
		return ret;

	/* reg_ctl success */
	hwcxext_reg_ctl_flag = 1;
	return 0;
}

static struct kernel_param_ops hwcxext_codec_ops_reg_ctl = {
	.get = hwcxext_codec_get_reg_ctl,
	.set = hwcxext_codec_set_reg_ctl,
};

module_param_cb(hwcxext_codec_reg_ctl, &hwcxext_codec_ops_reg_ctl, NULL, 0644);

MODULE_DESCRIPTION("hwcxext info driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");

