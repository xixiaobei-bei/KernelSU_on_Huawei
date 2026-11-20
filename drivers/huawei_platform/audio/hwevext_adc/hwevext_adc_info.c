/*
 * hwevext_adc_info.c
 *
 * extern_adc info driver
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
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/audio/hwevext_adc.h>
#include <securec.h>

#define HWLOG_TAG hwevext_adc
HWLOG_REGIST();

#define MAX_BUF_LEN                    512
#define HWEVEXT_ADC_REG_CTL_COUNT       32
#define HWEVEXT_ADC_CMD_PARAM_OFFSET    2
#define HWEVEXT_ADC_R_PARAM_NUM_MIN     2
#define HWEVEXT_ADC_W_PARAM_NUM_MIN     3
#define HWEVEXT_ADC_OPS_BULK_PARAM      2
#define ADC_BYTES_OF_8_BITS            1
#define ADC_BYTES_OF_16_BITS           2
#define HWEVEXT_ADC_STR_TO_INT_BASE     16

static char ext_adc_info_buf[MAX_BUF_LEN] = {0};

struct hwevext_adc_reg_ctl_params {
	char cmd;
	int params_num;
	union {
		int params[HWEVEXT_ADC_REG_CTL_COUNT];
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

static struct hwevext_adc_reg_ctl_params ext_adc_reg_ctl_params;
static int reg_ctl_flag;

#define HWEVEXT_ADC_INFO_HELP \
	"Usage:\n" \
	"dump_regs: echo d > ext_adc_reg_ctl\n" \
	"read_regs:\n" \
	"  echo \"r,adc_index,reg_addr,[bulk_count_once]\" > reg_ctl\n" \
	"write_regs:\n" \
	"  echo \"w,adc_index,reg_addr,reg_value,[reg_value2...]\" > reg_ctl\n"

static int hwevext_adc_get_reg_ctl(char *buffer, const struct kernel_param *kp)
{
	int ret;
	struct hwevext_adc_priv *priv = NULL;

	unused(kp);
	if (buffer == NULL) {
		hwlog_err("%s: buffer is NULL\n", __func__);
		return -EINVAL;
	}

	priv = hwevext_adc_get_misc_priv();
	if (IS_ERR_OR_NULL(priv)) {
		ret = snprintf_s(buffer, (unsigned long)MAX_BUF_LEN,
			(unsigned long)MAX_BUF_LEN - 1,
			"not support hwevext adc info ctl\n");
		if (ret < 0)
			hwlog_err("%s: snprintf_s is failed\n", __func__);

		return ret;
	}

	if (reg_ctl_flag == 0) {
		ret = snprintf_s(buffer, (unsigned long)MAX_BUF_LEN,
			(unsigned long)MAX_BUF_LEN - 1, HWEVEXT_ADC_INFO_HELP);
		if (ret < 0)
			hwlog_err("%s: snprintf_s is failed\n", __func__);

		return ret;
	}

	if (strlen(ext_adc_info_buf) > 0) {
		ret = snprintf_s(buffer, (unsigned long)MAX_BUF_LEN,
			(unsigned long)MAX_BUF_LEN - 1, ext_adc_info_buf);
		if (ret < 0)
			hwlog_err("%s: snprintf_s is failed\n", __func__);

		if (memset_s(ext_adc_info_buf, sizeof(ext_adc_info_buf),
			0, sizeof(ext_adc_info_buf)) != EOK)
			hwlog_err("%s: memset_s is failed\n", __func__);
	} else {
		ret = snprintf_s(buffer, (unsigned long)MAX_BUF_LEN,
			(unsigned long)MAX_BUF_LEN - 1,
			"hwevext_adc reg_ctl success\n"
			"(dmesg -c | grep hwevext_adc)");
		if (ret < 0)
			hwlog_err("%s: snprintf_s is failed\n", __func__);
	}

	return ret;
}

static int hwevext_adc_info_parse_reg_ctl(const char *val)
{
	char buf[MAX_BUF_LEN] = {0};
	char *tokens = NULL;
	char *pbuf = NULL;
	int index = 0;
	int ret;

	if (val == NULL) {
		hwlog_err("%s: val is NULL\n", __func__);
		return -EINVAL;
	}
	if (memset_s(&ext_adc_reg_ctl_params,
		sizeof(ext_adc_reg_ctl_params),
		0, sizeof(ext_adc_reg_ctl_params)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);

	/* ops cmd */
	hwlog_info("%s: val = %s\n", __func__, val);
	if (strncpy_s(buf, (unsigned long)MAX_BUF_LEN,
		val, (unsigned long)(MAX_BUF_LEN - 1)) != EOK)
		hwlog_err("%s: strncpy_s is failed\n", __func__);

	ext_adc_reg_ctl_params.cmd = buf[0];
	pbuf = &buf[HWEVEXT_ADC_CMD_PARAM_OFFSET];

	/* parse read/write ops params */
	do {
		tokens = strsep(&pbuf, ",");
		if (tokens == NULL)
			break;

		ret = kstrtoint(tokens, HWEVEXT_ADC_STR_TO_INT_BASE,
			&ext_adc_reg_ctl_params.params[index]);
		if (ret < 0)
			continue;

		hwlog_info("%s: tokens %d=%s, %d\n", __func__, index,
			tokens, ext_adc_reg_ctl_params.params[index]);

		index++;
		if (index == HWEVEXT_ADC_REG_CTL_COUNT) {
			hwlog_info("%s: params count max is %u\n", __func__,
				HWEVEXT_ADC_REG_CTL_COUNT);
			break;
		}
	} while (true);

	ext_adc_reg_ctl_params.params_num = index;
	return 0;
}

static int hwevext_adc_info_check_i2c_priv(int index)
{
	struct hwevext_adc_priv *priv = NULL;

	priv = hwevext_adc_get_misc_priv();
	if ((priv == NULL) ||
		(priv->i2c_priv[index] == NULL) ||
		(priv->i2c_priv[index]->regmap_cfg == NULL) ||
		(priv->i2c_priv[index]->regmap_cfg->regmap == NULL)) {
		hwlog_err("%s: priv or i2c_priv is NULL\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static int ext_adc_info_bulk_read_8_bit(struct hwevext_adc_regmap_cfg *cfg)
{
	int bulk_count_once = ext_adc_reg_ctl_params.bulk_r;
	int addr = ext_adc_reg_ctl_params.addr_r;
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

static int ext_adc_info_bulk_read_16_bit(struct hwevext_adc_regmap_cfg *cfg)
{
	int bulk_count_once = ext_adc_reg_ctl_params.bulk_r;
	int addr = ext_adc_reg_ctl_params.addr_r;
	unsigned short *value = NULL;
	int bulk_bytes = ADC_BYTES_OF_16_BITS;
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

static int hwevext_adc_bulk_read_regs(struct hwevext_adc_priv *priv)
{
	struct hwevext_adc_regmap_cfg *regmap_cfg = NULL;
	int bulk_value_bytes;
	int index = ext_adc_reg_ctl_params.index_r;

	if (hwevext_adc_info_check_i2c_priv(index) < 0)
		return -EINVAL;

	regmap_cfg = priv->i2c_priv[index]->regmap_cfg;

	bulk_value_bytes = regmap_cfg->cfg.val_bits / HWEVEXT_ADC_REG_VALUE_B8;

	if (bulk_value_bytes == ADC_BYTES_OF_8_BITS)
		return ext_adc_info_bulk_read_8_bit(regmap_cfg);
	if (bulk_value_bytes == ADC_BYTES_OF_16_BITS)
		return ext_adc_info_bulk_read_16_bit(regmap_cfg);

	hwlog_err("%s: bulk read reg, val_bits %d not supported\n",
		__func__, regmap_cfg->cfg.val_bits);

	return -EINVAL;
}

static int hwevext_adc_info_bulk_write_8_bit(struct hwevext_adc_regmap_cfg *cfg,
	int index)
{
	unsigned char *bulk_value = NULL;
	int addr = ext_adc_reg_ctl_params.addr_w;
	int bulk_count_once = ext_adc_reg_ctl_params.params_num -
		HWEVEXT_ADC_OPS_BULK_PARAM;
	int i;

	bulk_value = kzalloc(bulk_count_once, GFP_KERNEL);
	if (bulk_value == NULL)
		return -ENOMEM;

	/* get regs value for bulk write */
	for (i = 0; i < bulk_count_once; i++) {
		bulk_value[i] = (unsigned char)ext_adc_reg_ctl_params.value[i];
		hwlog_info("%s: pa%d bulk write reg 0x%x=0x%x\n", __func__,
			index, addr + i, bulk_value[i]);
	}

	regmap_bulk_write(cfg->regmap, addr, bulk_value, bulk_count_once);
	kfree(bulk_value);
	return 0;
}

static int hwevext_adc_info_bulk_write_16_bit(struct hwevext_adc_regmap_cfg *cfg,
	int index)
{
	unsigned short *bulk_value = NULL;
	int bulk_size;
	int addr = ext_adc_reg_ctl_params.addr_w;
	int bulk_count_once = ext_adc_reg_ctl_params.params_num -
		HWEVEXT_ADC_OPS_BULK_PARAM;
	int i;

	bulk_size = bulk_count_once * ADC_BYTES_OF_16_BITS;
	bulk_value = kzalloc(bulk_size, GFP_KERNEL);
	if (bulk_value == NULL)
		return -ENOMEM;

	/* get regs value for bulk write */
	for (i = 0; i < bulk_count_once; i++) {
		bulk_value[i] = (unsigned short)ext_adc_reg_ctl_params.value[i];
		hwlog_info("%s: pa%d bulk write reg 0x%x=0x%x\n", __func__,
			index, addr + i, bulk_value[i]);
	}

	regmap_bulk_write(cfg->regmap, addr, bulk_value, bulk_size);
	kfree(bulk_value);
	return 0;
}

static int hwevext_adc_bulk_write_regs(struct hwevext_adc_priv *priv)
{
	struct hwevext_adc_regmap_cfg *regmap_cfg = NULL;
	int bulk_value_bytes;
	int index = ext_adc_reg_ctl_params.index_w;

	if (hwevext_adc_info_check_i2c_priv(index) < 0)
		return -EINVAL;

	regmap_cfg = priv->i2c_priv[index]->regmap_cfg;
	/* bulk write regs: index, addr, value1, value2... */
	bulk_value_bytes = regmap_cfg->cfg.val_bits / HWEVEXT_ADC_REG_VALUE_B8;
	if (bulk_value_bytes == ADC_BYTES_OF_8_BITS)
		return hwevext_adc_info_bulk_write_8_bit(regmap_cfg, index);
	if (bulk_value_bytes == ADC_BYTES_OF_16_BITS)
		return hwevext_adc_info_bulk_write_16_bit(regmap_cfg, index);

	hwlog_err("%s: bulk write reg, val_bits %d not supported\n",
		__func__, regmap_cfg->cfg.val_bits);

	return -EINVAL;
}

static int hwevext_adc_info_reg_read(struct hwevext_adc_priv *priv)
{
	int index;
	int ret;
	unsigned int value = 0;

	index = ext_adc_reg_ctl_params.index_r;
	if ((ext_adc_reg_ctl_params.params_num < HWEVEXT_ADC_R_PARAM_NUM_MIN) ||
		(index >= priv->adc_num)) {
		hwlog_info("%s: params_num %d < 2, or index %d >= pa_num\n",
			__func__, ext_adc_reg_ctl_params.params_num, index);
		return -EINVAL;
	}

	if (ext_adc_reg_ctl_params.bulk_r > 0)
		return hwevext_adc_bulk_read_regs(priv);

	if (hwevext_adc_info_check_i2c_priv(index) < 0)
		return -EINVAL;

	regmap_read(priv->i2c_priv[index]->regmap_cfg->regmap,
		ext_adc_reg_ctl_params.addr_r, &value);

	hwlog_info("%s: adc%d read reg 0x%x=0x%x\n", __func__, index,
		ext_adc_reg_ctl_params.addr_r, value);

	if (memset_s(ext_adc_info_buf, MAX_BUF_LEN, 0, MAX_BUF_LEN) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);

	ret = snprintf_s(ext_adc_info_buf, (unsigned long)MAX_BUF_LEN,
		(unsigned long)(MAX_BUF_LEN - 1), "reg 0x%x=0x%04x\n", 
		ext_adc_reg_ctl_params.addr_r, value);
	if (ret < 0)
		hwlog_err("%s: snprintf_s is failed\n", __func__);

	return 0;
}

static int hwevext_adc_info_reg_write(struct hwevext_adc_priv *priv)
{
	int index;

	index = ext_adc_reg_ctl_params.index_w;
	if ((ext_adc_reg_ctl_params.params_num < HWEVEXT_ADC_W_PARAM_NUM_MIN) ||
		(index >= priv->adc_num)) {
		hwlog_info("%s: params_num %d < 3, index %d >= pa_num\n",
			__func__, ext_adc_reg_ctl_params.params_num, index);
		return -EINVAL;
	}

	if (ext_adc_reg_ctl_params.params_num > HWEVEXT_ADC_W_PARAM_NUM_MIN)
		return hwevext_adc_bulk_write_regs(priv);

	if (hwevext_adc_info_check_i2c_priv(index) < 0)
		return -EINVAL;

	hwlog_info("%s: adc%d write reg 0x%x=0x%x\n", __func__, index,
		ext_adc_reg_ctl_params.addr_w, ext_adc_reg_ctl_params.value[0]);
	regmap_write(priv->i2c_priv[index]->regmap_cfg->regmap,
		ext_adc_reg_ctl_params.addr_w, ext_adc_reg_ctl_params.value[0]);
	return 0;
}

static int hwevext_adc_info_do_reg_ctl(struct hwevext_adc_priv *priv)
{
	int ret = 0;

	/* dump/read/write ops */
	switch (ext_adc_reg_ctl_params.cmd) {
	case 'r':
		ret = hwevext_adc_info_reg_read(priv);
		break;
	case 'w':
		ret = hwevext_adc_info_reg_write(priv);
		break;
	case 'd':
		hwevext_adc_dump_chips(priv);
		break;
	default:
		hwlog_info("%s: not support cmd %c/0x%x\n", __func__,
			ext_adc_reg_ctl_params.cmd, ext_adc_reg_ctl_params.cmd);
		ret = -EFAULT;
		break;
	}
	return ret;
}

static int hwevext_adc_set_reg_ctl(const char *val,
	const struct kernel_param *kp)
{
	static struct hwevext_adc_priv *priv = NULL;
	int ret;

	unused(kp);
	priv = hwevext_adc_get_misc_priv();
	if (IS_ERR_OR_NULL(priv)) {
		hwlog_err("%s: priv is invaild\n", __func__);
		return -EINVAL;
	}
	reg_ctl_flag = 0;

	ret = hwevext_adc_info_parse_reg_ctl(val);
	if (ret < 0)
		return ret;

	ret = hwevext_adc_info_do_reg_ctl(priv);
	if (ret < 0)
		return ret;

	/* reg_ctl success */
	reg_ctl_flag = 1;
	return 0;
}

static struct kernel_param_ops param_ops_hwevext_adc_reg_ctl = {
	.get = hwevext_adc_get_reg_ctl,
	.set = hwevext_adc_set_reg_ctl,
};

module_param_cb(ext_adc_reg_ctl, &param_ops_hwevext_adc_reg_ctl, NULL, 0644);

MODULE_DESCRIPTION("extern adc info driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");

