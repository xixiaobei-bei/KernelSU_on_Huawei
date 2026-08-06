/*
 * hwevext_codec_info.c
 *
 * hwevext codec info driver
 *
 * Copyright (c) 2021~2022 Huawei Technologies Co., Ltd.
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
#include <huawei_platform/log/hw_log.h>
#include <linux/hisi/hisi_adc.h>
#include "hwevext_codec_info.h"

#define HWLOG_TAG hwevext_codec_info
HWLOG_REGIST();

#define HWEVEXT_INFO_BUF_MAX           512
#define HWEVEXT_REG_CTL_COUNT          32
#define HWEVEXT_CMD_PARAM_OFFSET       2
#define HWEVEXT_REG_R_PARAM_NUM_MIN    1
#define HWEVEXT_REG_W_PARAM_NUM_MIN    2
#define HWEVEXT_REG_OPS_BULK_PARAM     1
#define HWEVEXT_STR_TO_INT_BASE        16

#ifndef unused
#define unused(x) ((void)(x))
#endif

#define HWEVEXT_INFO_HELP \
	"Usage:\n" \
	"dump_regs: echo d > hwevext_codec_reg_ctl\n" \
	"read_regs:\n" \
	"echo \"r,reg_addr,[bulk_count_once]\" > hwevext_codec_reg_ctl\n" \
	"write_regs:\n" \
	"echo \"w,reg_addr,reg_value,[reg_value2...]\" > hwevext_codec_reg_ctl\n"

struct hwevext_codec_reg_ctl_params {
	char cmd;
	int params_num;
	union {
		int params[HWEVEXT_REG_CTL_COUNT];
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

static struct hwevext_codec_info_ctl_ops *g_info_ops;
static struct regmap *g_info_regmap;
static char hwevext_info_buffer[HWEVEXT_INFO_BUF_MAX] = {0};
static bool g_info_ctl_support = false;
static int g_adc_channel = -1;

void hwevext_codec_register_info_ctl_ops(struct hwevext_codec_info_ctl_ops *ops)
{
	if (ops == NULL)
		return;

	g_info_ops = ops;
}

void hwevext_codec_info_store_regmap(struct regmap *regmap)
{
	if (regmap == NULL)
		return;

	g_info_regmap = regmap;
}

void hwevext_codec_info_set_ctl_support(bool status)
{
	g_info_ctl_support = status;
}

void hwevext_codec_info_set_adc_channel(unsigned int channel)
{
	g_adc_channel = (int)channel;
}

static struct hwevext_codec_reg_ctl_params g_hwevext_ctl_params;
static int hwevext_reg_ctl_flag;

static int hwevext_codec_get_reg_ctl(char *buffer, const struct kernel_param *kp)
{
	int ret;

	unused(kp);
	if (buffer == NULL) {
		hwlog_err("%s: buffer is NULL\n", __func__);
		return -EINVAL;
	}

	if (!g_info_ctl_support) {
		ret = snprintf(buffer, (unsigned long)HWEVEXT_INFO_BUF_MAX,
			"not support hwevext info ctl\n");
		return ret;
	}

	if (hwevext_reg_ctl_flag == 0) {
		ret = snprintf(buffer, (unsigned long)HWEVEXT_INFO_BUF_MAX,
			HWEVEXT_INFO_HELP);
		return ret;
	}

	if (strlen(hwevext_info_buffer) > 0) {
		ret = snprintf(buffer, (unsigned long)HWEVEXT_INFO_BUF_MAX,
			hwevext_info_buffer);
		memset(hwevext_info_buffer, 0, HWEVEXT_INFO_BUF_MAX);
	} else {
		ret = snprintf(buffer, (unsigned long)HWEVEXT_INFO_BUF_MAX,
			"hwevext reg_ctl success\n"
			"(dmesg -c | grep hwevext)");
	}

	return ret;
}

static int hwevext_codec_info_parse_reg_ctl(const char *val)
{
	char buf[HWEVEXT_INFO_BUF_MAX] = {0};
	char *tokens = NULL;
	char *pbuf = NULL;
	int index = 0;
	int ret;

	if (val == NULL) {
		hwlog_err("%s: val is NULL\n", __func__);
		return -EINVAL;
	}
	memset(&g_hwevext_ctl_params, 0, sizeof(g_hwevext_ctl_params));

	/* ops cmd */
	hwlog_info("%s: val = %s\n", __func__, val);
	strncpy(buf, val, (unsigned long)(HWEVEXT_INFO_BUF_MAX - 1));
	g_hwevext_ctl_params.cmd = buf[0];
	pbuf = &buf[HWEVEXT_CMD_PARAM_OFFSET];

	/* parse dump/read/write ops params */
	do {
		tokens = strsep(&pbuf, ",");
		if (tokens == NULL)
			break;

		ret = kstrtoint(tokens, HWEVEXT_STR_TO_INT_BASE,
			&g_hwevext_ctl_params.params[index]);
		if (ret < 0)
			continue;

		hwlog_info("%s: tokens %d=%s, 0x%x\n", __func__, index,
			tokens, g_hwevext_ctl_params.params[index]);

		index++;
		if (index == HWEVEXT_REG_CTL_COUNT) {
			hwlog_info("%s: params count max is %u\n", __func__,
				HWEVEXT_REG_CTL_COUNT);
			break;
		}
	} while (true);

	g_hwevext_ctl_params.params_num = index;
	return 0;
}

static int hwevext_bulk_read_regs(void)
{
	int bulk_count_once = g_hwevext_ctl_params.bulk_r;
	int addr = g_hwevext_ctl_params.addr_r;
	unsigned char *value = NULL;
	int i;

	value = kzalloc(bulk_count_once, GFP_KERNEL);
	if (value == NULL)
		return -ENOMEM;

	regmap_bulk_read(g_info_regmap, addr, value, bulk_count_once);
	for (i = 0; i < bulk_count_once; i++)
		hwlog_info("%s: bulk read reg 0x%x=0x%x\n", __func__,
			addr + i, value[i]);

	kfree(value);
	return 0;
}

static int hwevext_codec_info_reg_read(void)
{
	unsigned int value = 0;

	if (g_hwevext_ctl_params.params_num < HWEVEXT_REG_R_PARAM_NUM_MIN) {
		hwlog_info("%s: params_num %d < %d\n",
			__func__, g_hwevext_ctl_params.params_num,
			HWEVEXT_REG_R_PARAM_NUM_MIN);
		return -EINVAL;
	}

	if (g_hwevext_ctl_params.bulk_r > 0)
		return hwevext_bulk_read_regs();

	regmap_read(g_info_regmap,
		g_hwevext_ctl_params.addr_r, &value);

	hwlog_info("%s:read reg 0x%x=0x%x\n", __func__,
		g_hwevext_ctl_params.addr_r, value);

	memset(hwevext_info_buffer, 0, HWEVEXT_INFO_BUF_MAX);
	snprintf(hwevext_info_buffer, (unsigned long)HWEVEXT_INFO_BUF_MAX,
		"reg 0x%x=0x%04x\n", g_hwevext_ctl_params.addr_r, value);
	return 0;
}

static int hwevext_bulk_write_regs(void)
{
	unsigned char *bulk_value = NULL;
	int addr = g_hwevext_ctl_params.addr_w;
	int bulk_count_once = g_hwevext_ctl_params.params_num -
		HWEVEXT_REG_OPS_BULK_PARAM;
	int i;

	bulk_value = kzalloc(bulk_count_once, GFP_KERNEL);
	if (bulk_value == NULL)
		return -ENOMEM;

	/* get regs value for bulk write */
	for (i = 0; i < bulk_count_once; i++) {
		bulk_value[i] = (unsigned char)g_hwevext_ctl_params.value[i];
		hwlog_info("%s: bulk write reg 0x%x=0x%x\n", __func__,
			addr + i, bulk_value[i]);
	}

	regmap_bulk_write(g_info_regmap, addr, bulk_value, bulk_count_once);
	kfree(bulk_value);
	return 0;
}

static int hwevext_codec_info_reg_write(void)
{
	if (g_hwevext_ctl_params.params_num < HWEVEXT_REG_W_PARAM_NUM_MIN) {
		hwlog_info("%s: params_num %d < %d\n",
			__func__, g_hwevext_ctl_params.params_num,
			HWEVEXT_REG_W_PARAM_NUM_MIN);
		return -EINVAL;
	}

	if (g_hwevext_ctl_params.params_num > HWEVEXT_REG_W_PARAM_NUM_MIN)
		return hwevext_bulk_write_regs();

	hwlog_info("%s:write reg 0x%x=0x%x\n", __func__,
		g_hwevext_ctl_params.addr_w, g_hwevext_ctl_params.value[0]);
	regmap_write(g_info_regmap,
		g_hwevext_ctl_params.addr_w, g_hwevext_ctl_params.value[0]);
	return 0;
}

static int hwevext_codec_info_do_reg_ctl(void)
{
	int ret;

	if (g_info_ops == NULL || g_info_regmap == NULL) {
		hwlog_err("%s: param is NULL\n", __func__);
		return -EINVAL;
	}

	/* dump/read/write ops */
	switch (g_hwevext_ctl_params.cmd) {
	case 'd': /* dump regs */
		ret = g_info_ops->dump_regs(g_info_regmap);
		break;
	case 'r':
		ret = hwevext_codec_info_reg_read();
		break;
	case 'w':
		ret = hwevext_codec_info_reg_write();
		break;
	default:
		hwlog_info("%s: not support cmd %c/0x%x\n", __func__,
			g_hwevext_ctl_params.cmd, g_hwevext_ctl_params.cmd);
		ret = -EFAULT;
		break;
	}
	return ret;
}

static int hwevext_codec_set_reg_ctl(const char *val,
	const struct kernel_param *kp)
{
	int ret;

	unused(kp);
	if (!g_info_ctl_support) {
		hwlog_info("%s: not support hwevext info ctl\n", __func__);
		return 0;
	}

	if (g_info_ops == NULL || g_info_regmap == NULL) {
		hwlog_err("%s: param is NULL\n", __func__);
		return -EINVAL;
	}
	hwevext_reg_ctl_flag = 0;

	ret = hwevext_codec_info_parse_reg_ctl(val);
	if (ret < 0)
		return ret;

	ret = hwevext_codec_info_do_reg_ctl();
	if (ret < 0)
		return ret;

	/* reg_ctl success */
	hwevext_reg_ctl_flag = 1;
	return 0;
}


static int hwevext_codec_get_adc_value(char *buffer,
	const struct kernel_param *kp)
{
	int val;
	int ret;

	unused(kp);
	if (g_adc_channel < 0) {
		ret = snprintf(buffer, (unsigned long)HWEVEXT_INFO_BUF_MAX,
			"not support get adc value\n");
		return ret;
	}

	val = hisi_adc_get_value(g_adc_channel);
	hwlog_info("%s: hkadc(%d) value:%d\n", __func__, g_adc_channel, val);

	return snprintf(buffer, (unsigned long)HWEVEXT_INFO_BUF_MAX,
			"hkadc(%d) value: %d\n", g_adc_channel, val);
}

static struct kernel_param_ops hwevext_codec_ops_adc_info = {
	.get = hwevext_codec_get_adc_value,
};

static struct kernel_param_ops hwevext_codec_ops_reg_ctl = {
	.get = hwevext_codec_get_reg_ctl,
	.set = hwevext_codec_set_reg_ctl,
};

module_param_cb(hwevext_codec_adc_info, &hwevext_codec_ops_adc_info, NULL, 0444);
module_param_cb(hwevext_codec_reg_ctl, &hwevext_codec_ops_reg_ctl, NULL, 0644);

MODULE_DESCRIPTION("hwevext info driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");

