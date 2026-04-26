/*
 * smartpakit_info.c
 *
 * smartpakit info driver
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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
#include "smartpakit.h"
#include <securec.h>

#define HWLOG_TAG smartpakit
HWLOG_REGIST();

#define INFO_BUF_MAX               512
#define SMARTPAKIT_REG_CTL_COUNT   32
#define KIT_CMD_PARAM_OFFSET       2
#define KIT_REG_R_PARAM_NUM_MIN    2
#define KIT_REG_W_PARAM_NUM_MIN    3
#define KIT_REG_OPS_BULK_PARAM     2
#define BYTES_OF_8_BITS            1
#define BYTES_OF_16_BITS           2
#define KIT_STR_TO_INT_BASE        16

struct name_to_index {
	char name[SMARTPAKIT_NAME_MAX];
	unsigned int index;
};

#define to_name_index(X)   {#X, X}

static char info_buffer[INFO_BUF_MAX] = {0};

static struct name_to_index soc_name_index[SMARTPAKIT_SOC_PLATFORM_MAX] = {
	to_name_index(SMARTPAKIT_SOC_PLATFORM_HISI),
	to_name_index(SMARTPAKIT_SOC_PLATFORM_QCOM),
};

#define get_soc_platform(index) \
	(soc_name_index[index].name + strlen("SMARTPAKIT_SOC_PLATFORM_"))

static struct name_to_index algo_in_name_index[SMARTPAKIT_ALGO_IN_MAX] = {
	to_name_index(SMARTPAKIT_ALGO_IN_CODEC_DSP),
	to_name_index(SMARTPAKIT_ALGO_IN_SOC_DSP),
	to_name_index(SMARTPAKIT_ALGO_IN_WITH_DSP),
	to_name_index(SMARTPAKIT_ALGO_IN_SIMPLE),
	to_name_index(SMARTPAKIT_ALGO_IN_SIMPLE_WITH_I2C),
	to_name_index(SMARTPAKIT_ALGO_IN_WITH_DSP_PLUGIN),
};

#define get_algo_in(index) \
	(algo_in_name_index[index].name + strlen("SMARTPAKIT_ALGO_IN_"))

static struct name_to_index out_dev_name_index[SMARTPAKIT_OUT_DEVICE_MAX] = {
	to_name_index(SMARTPAKIT_OUT_DEVICE_SPEAKER),
	to_name_index(SMARTPAKIT_OUT_DEVICE_RECEIVER),
};

#define get_out_device(index) \
	(out_dev_name_index[index].name + strlen("SMARTPAKIT_OUT_DEVICE_"))

static struct name_to_index chip_vendor_index[SMARTPAKIT_CHIP_VENDOR_MAX] = {
	to_name_index(SMARTPAKIT_CHIP_VENDOR_MAXIM),
	to_name_index(SMARTPAKIT_CHIP_VENDOR_NXP),
	to_name_index(SMARTPAKIT_CHIP_VENDOR_TI),
	to_name_index(SMARTPAKIT_CHIP_VENDOR_OTHER),
	to_name_index(SMARTPAKIT_CHIP_VENDOR_CUSTOMIZE),
};

#define get_chip_vendor(index) \
	(chip_vendor_index[index].name + strlen("SMARTPAKIT_CHIP_VENDOR_"))

static void smartpakit_append_info(char *fmt, ...)
{
	va_list args;
	char tmp_str[INFO_BUF_MAX] = {0};
	unsigned int info_len;

	if (fmt == NULL) {
		hwlog_err("%s, dst or src is NULL\n", __func__);
		return;
	}

	va_start(args, fmt);
	vscnprintf(tmp_str, (unsigned long)INFO_BUF_MAX, (const char *)fmt,
		args);
	va_end(args);

	info_len = INFO_BUF_MAX - strlen(info_buffer) - 1;
	strncat(info_buffer, tmp_str, info_len);
}

static void smartpakit_get_out_device_info(struct smartpakit_info *info)
{
	int i;
	unsigned int device;
	unsigned int offset;

	/* out_device */
	hwlog_info("%s: out_device: 0x%08x\n", __func__, info->out_device);
	smartpakit_append_info("out_device: 0x%08x\n", info->out_device);

	/* out_device type: spk or rcv */
	for (i = 0; i < (int)info->pa_num; i++) {
		offset = i * SMARTPAKIT_PA_OUT_DEVICE_SHIFT;
		device = (info->out_device >> offset);
		device &= SMARTPAKIT_PA_OUT_DEVICE_MASK;

		switch (device) {
		case SMARTPAKIT_OUT_DEVICE_SPEAKER:  /* speaker */
		case SMARTPAKIT_OUT_DEVICE_RECEIVER: /* receiver */
			hwlog_info("%s: out_device id %d is %s\n", __func__, i,
				get_out_device(device));
			smartpakit_append_info("out_device id %d is %s\n", i,
				get_out_device(device));
			break;
		default:
			break;
		}
	}
}

static void smartpakit_get_chip_vendor_and_model(struct smartpakit_info *info)
{
	if (info->chip_vendor == SMARTPAKIT_VENDR_INVALID) {
		/* get chip vendor in i2c err */
		hwlog_info("%s: chip_vendor: invalid\n", __func__);
		hwlog_info("%s: chip_model: invalid\n", __func__);
		smartpakit_append_info("chip_vendor: invalid\n"
			"chip_model: invalid\n");
		return;
	}

	hwlog_info("%s: chip_vendor: %u, %s\n", __func__, info->chip_vendor,
		get_chip_vendor(info->chip_vendor));
	hwlog_info("%s: chip_model: %s\n", __func__, info->chip_model);
	smartpakit_append_info("chip_vendor: %u, %s\nchip_model: %s\n",
		info->chip_vendor, get_chip_vendor(info->chip_vendor),
		info->chip_model);
}

/* pa info */
static int smartpakit_get_pa_info(char *buffer, const struct kernel_param *kp)
{
	struct smartpakit_priv *kit = NULL;
	struct smartpakit_info info;
	int len;

	kit = smartpakit_get_misc_priv();
	if (memset_s(info_buffer, sizeof(info_buffer),
		0, sizeof(info_buffer)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);
	if (memset_s(&info, sizeof(info), 0, sizeof(info)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);

	unused(kp);
	if ((buffer == NULL) || (kit == NULL)) {
		hwlog_err("%s: buffer or kit is NULL\n", __func__);
		return -EINVAL;
	}

	smartpakit_get_pa_info_from_dts_config(kit, &info);

	/* simple pa */
	if ((info.algo_in == SMARTPAKIT_ALGO_IN_SIMPLE) ||
		(info.algo_in == SMARTPAKIT_ALGO_IN_SIMPLE_WITH_I2C)) {
		hwlog_info("%s: simple pa info\n", __func__);
		smartpakit_append_info("simple pa info:\n");
	} else {
		/* smart pa */
		hwlog_info("%s: smart pa info\n", __func__);
		smartpakit_append_info("smart pa info:\n");
	}

	/* soc_platform */
	hwlog_info("%s: soc_platform: %u, %s\n", __func__, info.soc_platform,
		get_soc_platform(info.soc_platform));
	smartpakit_append_info("soc_platform: %u, %s\n", info.soc_platform,
		get_soc_platform(info.soc_platform));

	/* algo_in */
	hwlog_info("%s: algo_in: %u, %s\n", __func__, info.algo_in,
		get_algo_in(info.algo_in));
	smartpakit_append_info("algo_in: %u, %s\n", info.algo_in,
		get_algo_in(info.algo_in));

	/* out device */
	smartpakit_get_out_device_info(&info);

	/* pa_num and algo_delay_time */
	hwlog_info("%s: pa_num: %u\n", __func__, info.pa_num);
	hwlog_info("%s: algo_delay_time: %u\n", __func__,
		info.algo_delay_time);
	smartpakit_append_info("pa_num: %u\nalgo_delay_time: %u\n",
		info.pa_num, info.algo_delay_time);

	/* chip vendor and model */
	smartpakit_get_chip_vendor_and_model(&info);
	smartpakit_append_info("special_name: %s\n", info.special_name_config);

	len = snprintf_s(buffer, (unsigned long)INFO_BUF_MAX,
		(unsigned long)INFO_BUF_MAX - 1, info_buffer);
	if (len < 0)
		hwlog_err("%s: snprintf_s is failed\n", __func__);
	if (memset_s(info_buffer, sizeof(info_buffer),
		0, sizeof(info_buffer)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);

	return len;
}

#define SMARTPAKIT_INFO_HELP \
	"Usage:\n" \
	"report remap info: echo m > reg_ctl\n" \
	"hw_reset: echo h > reg_ctl\n" \
	"irq_trigger: echo i,pa_index > reg_ctl\n" \
	"dump_regs: echo d > reg_ctl\n" \
	"read_regs:\n" \
	"  echo \"r,pa_index,reg_addr,[bulk_count_once]\" > reg_ctl\n" \
	"write_regs:\n" \
	"  echo \"w,pa_index,reg_addr,reg_value,[reg_value2...]\" > reg_ctl\n"

struct smartpakit_reg_ctl_params {
	char cmd;
	int params_num;
	union {
		int params[SMARTPAKIT_REG_CTL_COUNT];
		int index_i;
		struct {
			int index_d;
			int bulk_d;
		};
		struct {
			int index_r;
			int addr_r;
			int bulk_r;
		};
		struct {
			int index_w;
			int addr_w;
			int value[0];
		};
	};
};

static struct smartpakit_reg_ctl_params reg_ctl_params;
static int reg_ctl_flag;

static int smartpakit_get_reg_ctl(char *buffer, const struct kernel_param *kp)
{
	int ret;

	unused(kp);
	if (buffer == NULL) {
		hwlog_err("%s: buffer is NULL\n", __func__);
		return -EINVAL;
	}

	if (reg_ctl_flag == 0) {
		ret = snprintf_s(buffer, (unsigned long)INFO_BUF_MAX,
			(unsigned long)INFO_BUF_MAX - 1, SMARTPAKIT_INFO_HELP);
		if (ret < 0)
			hwlog_err("%s: snprintf_s is failed\n", __func__);
		return ret;
	}

	if (strlen(info_buffer) > 0) {
		ret = snprintf_s(buffer, (unsigned long)INFO_BUF_MAX,
			(unsigned long)INFO_BUF_MAX - 1, info_buffer);
		if (ret < 0)
			hwlog_err("%s: snprintf_s is failed\n", __func__);
		if (memset_s(info_buffer, INFO_BUF_MAX, 0, INFO_BUF_MAX) != EOK)
			hwlog_err("%s: memset_s is failed\n", __func__);
	} else {
		ret = snprintf_s(buffer, (unsigned long)INFO_BUF_MAX,
			(unsigned long)INFO_BUF_MAX - 1, "smartpa reg_ctl success\n"
			"(dmesg -c | grep smartpakit)");
		if (ret < 0)
			hwlog_err("%s: snprintf_s is failed\n", __func__);
	}

	return ret;
}

static int smartpakit_info_parse_reg_ctl(const char *val)
{
	char buf[INFO_BUF_MAX] = {0};
	char *tokens = NULL;
	char *pbuf = NULL;
	int index = 0;
	int ret;

	if (val == NULL) {
		hwlog_err("%s: val is NULL\n", __func__);
		return -EINVAL;
	}
	if (memset_s(&reg_ctl_params, sizeof(reg_ctl_params), 0, sizeof(reg_ctl_params)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);

	/* ops cmd */
	hwlog_info("%s: val = %s\n", __func__, val);
	if (strncpy_s(buf, (unsigned long)INFO_BUF_MAX, val,
		(unsigned long)(INFO_BUF_MAX - 1)) != EOK)
		hwlog_err("%s: strncpy_s is failed\n", __func__);
	reg_ctl_params.cmd = buf[0];
	pbuf = &buf[KIT_CMD_PARAM_OFFSET];

	/* parse dump/read/write ops params */
	do {
		tokens = strsep(&pbuf, ",");
		if (tokens == NULL)
			break;

		ret = kstrtoint(tokens, KIT_STR_TO_INT_BASE,
			&reg_ctl_params.params[index]);
		if (ret < 0)
			continue;

		hwlog_info("%s: tokens %d=%s, %d\n", __func__, index,
			tokens, reg_ctl_params.params[index]);

		index++;
		if (index == SMARTPAKIT_REG_CTL_COUNT) {
			hwlog_info("%s: params count max is %u\n", __func__,
				SMARTPAKIT_REG_CTL_COUNT);
			break;
		}
	} while (true);

	reg_ctl_params.params_num = index;
	return 0;
}

static int smartpakit_info_check_i2c_priv(int index)
{
	struct smartpakit_priv *kit = NULL;

	kit = smartpakit_get_misc_priv();
	if ((kit == NULL) ||
		(kit->i2c_priv[index] == NULL) ||
		(kit->i2c_priv[index]->regmap_cfg == NULL) ||
		(kit->i2c_priv[index]->regmap_cfg->regmap == NULL)) {
		hwlog_err("%s: kit or i2c_priv is NULL\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static int kit_info_bulk_read_8_bit(struct smartpakit_regmap_cfg *cfg)
{
	int bulk_count_once = reg_ctl_params.bulk_r;
	int addr = reg_ctl_params.addr_r;
	unsigned char *value = NULL;
	int i;

	value = kzalloc(bulk_count_once, GFP_KERNEL);
	if (value == NULL)
		return -ENOMEM;

	regmap_bulk_read(cfg->regmap, addr, value, bulk_count_once);
	for (i = 0; i < bulk_count_once; i++)
		hwlog_info("%s: bulk read reg 0x%x=0x%x\n", __func__,
			addr + i, value[i]);

	kfree(value);
	return 0;
}

static int kit_info_bulk_read_16_bit(struct smartpakit_regmap_cfg *cfg)
{
	int bulk_count_once = reg_ctl_params.bulk_r;
	int addr = reg_ctl_params.addr_r;
	unsigned short *value = NULL;
	int bulk_bytes = BYTES_OF_16_BITS;
	int i;

	value = kzalloc(bulk_count_once * bulk_bytes, GFP_KERNEL);
	if (value == NULL)
		return -ENOMEM;

	regmap_bulk_read(cfg->regmap, addr, value,
		bulk_count_once * bulk_bytes);
	for (i = 0; i < bulk_count_once; i++)
		hwlog_info("%s: bulk read reg 0x%x=0x%x\n", __func__,
			addr + i, value[i]);

	kfree(value);
	return 0;
}

static int smartpakit_bulk_read_regs(struct smartpakit_priv *kit)
{
	struct smartpakit_regmap_cfg *regmap_cfg = NULL;
	int bulk_value_bytes;
	int index = reg_ctl_params.index_r;

	if (smartpakit_info_check_i2c_priv(index) < 0)
		return -EINVAL;

	regmap_cfg = kit->i2c_priv[index]->regmap_cfg;

	bulk_value_bytes = regmap_cfg->cfg.val_bits / SMARTPAKIT_REG_VALUE_B8;

	if (bulk_value_bytes == BYTES_OF_8_BITS)
		return kit_info_bulk_read_8_bit(regmap_cfg);
	if (bulk_value_bytes == BYTES_OF_16_BITS)
		return kit_info_bulk_read_16_bit(regmap_cfg);

	hwlog_err("%s: bulk read reg, val_bits %d not supported\n",
		__func__, regmap_cfg->cfg.val_bits);

	return -EINVAL;
}

static int kit_info_bulk_write_8_bit(struct smartpakit_regmap_cfg *cfg,
	int index)
{
	unsigned char *bulk_value = NULL;
	int addr = reg_ctl_params.addr_w;
	int bulk_count_once = reg_ctl_params.params_num -
		KIT_REG_OPS_BULK_PARAM;
	int i;

	bulk_value = kzalloc(bulk_count_once, GFP_KERNEL);
	if (bulk_value == NULL)
		return -ENOMEM;

	/* get regs value for bulk write */
	for (i = 0; i < bulk_count_once; i++) {
		bulk_value[i] = (unsigned char)reg_ctl_params.value[i];
		hwlog_info("%s: pa%d bulk write reg 0x%x=0x%x\n", __func__,
			index, addr + i, bulk_value[i]);
	}

	regmap_bulk_write(cfg->regmap, addr, bulk_value, bulk_count_once);
	kfree(bulk_value);
	return 0;
}

static int kit_info_bulk_write_16_bit(struct smartpakit_regmap_cfg *cfg,
	int index)
{
	unsigned short *bulk_value = NULL;
	int bulk_size;
	int addr = reg_ctl_params.addr_w;
	int bulk_count_once = reg_ctl_params.params_num -
		KIT_REG_OPS_BULK_PARAM;
	int i;

	bulk_size = bulk_count_once * BYTES_OF_16_BITS;
	bulk_value = kzalloc(bulk_size, GFP_KERNEL);
	if (bulk_value == NULL)
		return -ENOMEM;

	/* get regs value for bulk write */
	for (i = 0; i < bulk_count_once; i++) {
		bulk_value[i] = (unsigned short)reg_ctl_params.value[i];
		hwlog_info("%s: pa%d bulk write reg 0x%x=0x%x\n", __func__,
			index, addr + i, bulk_value[i]);
	}

	regmap_bulk_write(cfg->regmap, addr, bulk_value, bulk_size);
	kfree(bulk_value);
	return 0;
}

static int smartpakit_bulk_write_regs(struct smartpakit_priv *kit)
{
	struct smartpakit_regmap_cfg *regmap_cfg = NULL;
	int bulk_value_bytes;
	int index = reg_ctl_params.index_w;

	if (smartpakit_info_check_i2c_priv(index) < 0)
		return -EINVAL;

	regmap_cfg = kit->i2c_priv[index]->regmap_cfg;
	/* bulk write regs: index, addr, value1, value2... */
	bulk_value_bytes = regmap_cfg->cfg.val_bits / SMARTPAKIT_REG_VALUE_B8;
	if (bulk_value_bytes == BYTES_OF_8_BITS)
		return kit_info_bulk_write_8_bit(regmap_cfg, index);
	if (bulk_value_bytes == BYTES_OF_16_BITS)
		return kit_info_bulk_write_16_bit(regmap_cfg, index);

	hwlog_err("%s: bulk write reg, val_bits %d not supported\n",
		__func__, regmap_cfg->cfg.val_bits);

	return -EINVAL;
}

static int smartpakit_info_report_regmap_info(struct smartpakit_priv *kit)
{
	struct smartpakit_regmap_cfg *regmap_cfg = NULL;

	if (smartpakit_info_check_i2c_priv(0) < 0)
		return -EINVAL;

	regmap_cfg = kit->i2c_priv[0]->regmap_cfg;
	hwlog_info("%s: reg_bits=%d,val_bits=%d,max_register=%u\n", __func__,
		regmap_cfg->cfg.reg_bits, regmap_cfg->cfg.val_bits,
		regmap_cfg->cfg.max_register);

	return 0;
}

static int smartpakit_info_trigger_irq(struct smartpakit_priv *kit)
{
	int index;
	int irq;

	index = reg_ctl_params.index_i;
	if ((reg_ctl_params.params_num < 1) || (index >= kit->pa_num)) {
		hwlog_info("%s: params_num %d < 1, or index %d >= pa_num\n",
			__func__, reg_ctl_params.params_num, index);
		return -EINVAL;
	}

	if ((kit->i2c_priv[index] != NULL) &&
		(kit->i2c_priv[index]->irq_handler != NULL)) {
		hwlog_info("%s: pa %d trigger irq by test\n", __func__, index);
		irq = kit->i2c_priv[index]->irq_handler->irq;
		smartpakit_i2c_thread_irq(irq, kit->i2c_priv[index]);
	}
	return 0;
}

static int smartpakit_info_reg_read(struct smartpakit_priv *kit)
{
	int index;
	unsigned int value = 0;

	index = reg_ctl_params.index_r;
	if ((reg_ctl_params.params_num < KIT_REG_R_PARAM_NUM_MIN) ||
		(index >= kit->pa_num)) {
		hwlog_info("%s: params_num %d < 2, or index %d >= pa_num\n",
			__func__, reg_ctl_params.params_num, index);
		return -EINVAL;
	}

	if (reg_ctl_params.bulk_r > 0)
		return smartpakit_bulk_read_regs(kit);

	if (smartpakit_info_check_i2c_priv(index) < 0)
		return -EINVAL;

	regmap_read(kit->i2c_priv[index]->regmap_cfg->regmap,
		reg_ctl_params.addr_r, &value);

	hwlog_info("%s: pa%d read reg 0x%x=0x%x\n", __func__, index,
		reg_ctl_params.addr_r, value);

	if (memset_s(info_buffer, INFO_BUF_MAX, 0, INFO_BUF_MAX) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);
	if (snprintf_s(info_buffer, (unsigned long)INFO_BUF_MAX,
		(unsigned long)INFO_BUF_MAX - 1, "reg 0x%x=0x%04x\n",
		reg_ctl_params.addr_r, value) < 0)
		hwlog_err("%s: snprintf_s is failed\n", __func__);
	return 0;
}

static int smartpakit_info_reg_write(struct smartpakit_priv *kit)
{
	int index;

	index = reg_ctl_params.index_w;
	if ((reg_ctl_params.params_num < KIT_REG_W_PARAM_NUM_MIN) ||
		(index >= kit->pa_num)) {
		hwlog_info("%s: params_num %d < 3, index %d >= pa_num\n",
			__func__, reg_ctl_params.params_num, index);
		return -EINVAL;
	}

	if (reg_ctl_params.params_num > KIT_REG_W_PARAM_NUM_MIN)
		return smartpakit_bulk_write_regs(kit);

	if (smartpakit_info_check_i2c_priv(index) < 0)
		return -EINVAL;

	hwlog_info("%s: pa%d write reg 0x%x=0x%x\n", __func__, index,
		reg_ctl_params.addr_w, reg_ctl_params.value[0]);
	regmap_write(kit->i2c_priv[index]->regmap_cfg->regmap,
		reg_ctl_params.addr_w, reg_ctl_params.value[0]);
	return 0;
}

static int smartpakit_info_do_reg_ctl(struct smartpakit_priv *kit)
{
	int ret;
	/* dump/read/write ops */
	switch (reg_ctl_params.cmd) {
	case 'm': /* report remap info */
		ret = smartpakit_info_report_regmap_info(kit);
		break;
	case 'h': /* hw_reset */
		ret = smartpakit_hw_reset_chips(kit);
		break;
	case 'i': /* irq trigger */
		ret = smartpakit_info_trigger_irq(kit);
		break;
	case 'd': /* dump regs */
		ret = smartpakit_dump_chips(kit);
		break;
	case 'r':
		ret = smartpakit_info_reg_read(kit);
		break;
	case 'w':
		ret = smartpakit_info_reg_write(kit);
		break;
	default:
		hwlog_info("%s: not support cmd %c/0x%x\n", __func__,
			reg_ctl_params.cmd, reg_ctl_params.cmd);
		ret = -EFAULT;
		break;
	}
	return ret;
}

static int smartpakit_set_reg_ctl(const char *val,
	const struct kernel_param *kp)
{
	struct smartpakit_priv *kit = NULL;
	int ret;

	unused(kp);
	kit = smartpakit_get_misc_priv();
	if (kit == NULL) {
		hwlog_err("%s: kit is NULL\n", __func__);
		return -EINVAL;
	}
	reg_ctl_flag = 0;

	/* simple pa: not support i2c r/w ops */
	if ((kit->algo_in == SMARTPAKIT_ALGO_IN_SIMPLE) ||
		(kit->algo_in == SMARTPAKIT_ALGO_IN_WITH_DSP_PLUGIN)) {
		hwlog_err("%s: not support i2c ops, algo_in=%u\n", __func__,
			kit->algo_in);
		return -EFAULT;
	}

	ret = smartpakit_info_parse_reg_ctl(val);
	if (ret < 0)
		return ret;

	ret = smartpakit_info_do_reg_ctl(kit);
	if (ret < 0)
		return ret;

	/* reg_ctl success */
	reg_ctl_flag = 1;
	return 0;
}

static struct kernel_param_ops param_ops_pa_info = {
	.get = smartpakit_get_pa_info,
};

static struct kernel_param_ops param_ops_reg_ctl = {
	.get = smartpakit_get_reg_ctl,
	.set = smartpakit_set_reg_ctl,
};

module_param_cb(pa_info, &param_ops_pa_info, NULL, 0444);
module_param_cb(reg_ctl, &param_ops_reg_ctl, NULL, 0644);

MODULE_DESCRIPTION("smartpakit info driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");

