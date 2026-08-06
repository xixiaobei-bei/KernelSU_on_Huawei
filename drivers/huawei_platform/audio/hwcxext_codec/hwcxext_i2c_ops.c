/*
 * hwcxext_i2c_ops.c
 *
 * hwcxext i2c ops driver
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
#include <linux/slab.h>
#include <linux/of.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>

#include <dsm/dsm_pub.h>
#ifdef CONFIG_HUAWEI_DSM_AUDIO
#include <dsm_audio/dsm_audio.h>
#endif

#include "hwcxext_i2c_ops.h"
#include "hwcxext_codec_def.h"

#define HWLOG_TAG hwcxext_i2c_ops
HWLOG_REGIST();

#define REG_PRINT_NUM  8
#define RETRY_TIMES    3
#define HWCXEXT_SLEEP_US_DEAT 500
#define I2C_STATUS_B64 64

/*
 * 0 read reg node:   r-reg-addr | mask | value     | delay | 0
 * 1 write reg node:  w-reg-addr | mask | value     | delay | 1
 * 2 delay node:      0          | 0    | 0         | delay | 2
 * 3 skip node:       0          | 0    | 0         | 0     | 3
 * 4 rxorw node:      w-reg-addr | mask | r-reg-addr| delay | 4
 *   this mask is xor mask
 */
enum hwcxext_reg_ctl_type {
	HWCXEXT_REG_CTL_TYPE_R = 0,  /* read reg        */
	HWCXEXT_REG_CTL_TYPE_W,      /* write reg       */
	HWCXEXT_REG_CTL_TYPE_DELAY,  /* only time delay */
	HWCXEXT_PARAM_NODE_TYPE_SKIP,
	/* read, xor, write */
	HWCXEXT_PARAM_NODE_TYPE_REG_RXORW,
	HWCXEXT_REG_CTL_TYPE_MAX,
};


/* reg val_bits */
#define HWCXEXT_REG_VALUE_B8     8  /* val_bits == 8  */
#define HWCXEXT_REG_VALUE_B16    16 /* val_bits == 16 */
#define HWCXEXT_REG_VALUE_B24    24 /* val_bits == 24 */
#define HWCXEXT_REG_VALUE_B32    32 /* val_bits == 32 */

/* reg value mask by reg val_bits */
#define HWCXEXT_REG_VALUE_M8     0xFF
#define HWCXEXT_REG_VALUE_M16    0xFFFF
#define HWCXEXT_REG_VALUE_M24    0xFFFFFF
#define HWCXEXT_REG_VALUE_M32    0xFFFFFFFF

#define HWCXEXT_V1_MAX_EQ_COEFF 11
#define HWCXEXT_V1_I2C_LEN 2

struct i2c_err_info {
	unsigned int regs_num;
	unsigned int err_count;
	unsigned long int err_details;
};

static void hwcxext_kfree_ops(void *p)
{
	if (IS_ERR_OR_NULL(p))
		return;

	kfree(p);
}

static bool hwcxext_check_i2c_regmap_valid(
	struct hwcxext_codec_regmap_cfg *cfg)
{
	if (IS_ERR_OR_NULL(cfg))
		return false;

	if (IS_ERR_OR_NULL(cfg->regmap))
		return false;

	return true;
}

void hwcxext_i2c_free_reg_ctl(struct hwcxext_reg_ctl_sequence **reg_ctl)
{
	if (reg_ctl == NULL || *reg_ctl == NULL)
		return;

	hwcxext_kfree_ops((*reg_ctl)->regs);
	kfree(*reg_ctl);
	*reg_ctl = NULL;
}

void hwcxext_i2c_free_regs_size_seq(
	struct hwcxext_regs_size_sequence **seq)
{
	if (seq == NULL || *seq == NULL)
		return;

	hwcxext_kfree_ops((*seq)->regs);
	kfree(*seq);
	*seq = NULL;
}

static int hwcxext_get_prop_of_u32_array(struct device_node *node,
	const char *propname, u32 **value, int *num)
{
	u32 *array = NULL;
	int ret;
	int count = 0;

	if ((node == NULL) || (propname == NULL) || (value == NULL) ||
		(num == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (of_property_read_bool(node, propname))
		count = of_property_count_elems_of_size(node,
			propname, (int)sizeof(u32));

	if (count == 0) {
		hwlog_debug("%s: %s not existed, skip\n", __func__, propname);
		return 0;
	}

	array = kzalloc(sizeof(u32) * count, GFP_KERNEL);
	if (array == NULL)
		return -ENOMEM;

	ret = of_property_read_u32_array(node, propname, array,
		(size_t)(long)count);
	if (ret < 0) {
		hwlog_err("%s: get %s array failed\n", __func__, propname);
		ret = -EFAULT;
		goto hwcxext_get_prop_err_out;
	}

	*value = array;
	*num = count;
	return 0;

hwcxext_get_prop_err_out:
	hwcxext_kfree_ops(array);
	return ret;
}

static int hwcxext_i2c_parse_reg_ctl(
	struct hwcxext_reg_ctl_sequence **reg_ctl, struct device_node *node,
	const char *ctl_str)
{
	struct hwcxext_reg_ctl_sequence *ctl = NULL;
	int count = 0;
	int val_num;
	int ret;

	if ((node == NULL) || (ctl_str == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	ctl = kzalloc(sizeof(struct hwcxext_reg_ctl_sequence), GFP_KERNEL);
	if (ctl == NULL)
		return -ENOMEM;

	ret = hwcxext_get_prop_of_u32_array(node, ctl_str,
		(u32 **)&ctl->regs, &count);
	if ((count <= 0) || (ret < 0)) {
		hwlog_err("%s: get %s failed or count is 0\n",
			__func__, ctl_str);
		ret = -EFAULT;
		goto hwcxext_parse_reg_ctl_err_out;
	}

	val_num = sizeof(struct hwcxext_reg_ctl) / sizeof(unsigned int);
	if ((count % val_num) != 0) {
		hwlog_err("%s: count %d %% val_num %d != 0\n",
			__func__, count, val_num);
		ret = -EFAULT;
		goto hwcxext_parse_reg_ctl_err_out;
	}

	ctl->num = (unsigned int)(count / val_num);
	*reg_ctl = ctl;
	return 0;

hwcxext_parse_reg_ctl_err_out:
	hwcxext_i2c_free_reg_ctl(&ctl);
	return ret;
}

static void hwcxext_print_regs_info(const char *seq_str,
	struct hwcxext_reg_ctl_sequence *reg_ctl)
{
	unsigned int print_node_num;
	unsigned int i;
	struct hwcxext_reg_ctl *reg = NULL;

	if (reg_ctl == NULL)
		return;

	print_node_num =
		(reg_ctl->num < REG_PRINT_NUM) ? reg_ctl->num : REG_PRINT_NUM;

	/* only print two registers */
	for (i = 0; i < print_node_num; i++) {
		reg = &(reg_ctl->regs[i]);
		hwlog_info("%s: %s reg_%d=0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
			__func__, seq_str, i, reg->addr, reg->mask, reg->value,
			reg->delay, reg->ctl_type);
	}
}

int hwcxext_i2c_parse_dt_reg_ctl(struct i2c_client *i2c,
	struct hwcxext_reg_ctl_sequence **reg_ctl, const char *seq_str)
{
	int ret;

	if (!of_property_read_bool(i2c->dev.of_node, seq_str)) {
		hwlog_debug("%s: %s not existed, skip\n", seq_str, __func__);
		return 0;
	}
	ret = hwcxext_i2c_parse_reg_ctl(reg_ctl,
		i2c->dev.of_node, seq_str);
	if (ret < 0) {
		hwlog_err("%s: parse %s failed\n", __func__, seq_str);
		goto hwcxext_parse_dt_reg_ctl_err_out;
	}

	hwcxext_print_regs_info(seq_str, *reg_ctl);

hwcxext_parse_dt_reg_ctl_err_out:
	return ret;
}

static int hwcxext_regmap_read(struct hwcxext_codec_regmap_cfg *cfg,
	unsigned int reg_addr,
	unsigned int *value)
{
	int ret = 0;
	int i;

	if (!hwcxext_check_i2c_regmap_valid(cfg)) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_read(cfg->regmap, reg_addr, value);
		if (ret == 0)
			break;

		mdelay(1);
	}
	return ret;
}

static int hwcxext_regmap_write(
	struct hwcxext_codec_regmap_cfg *cfg,
	unsigned int reg_addr, unsigned int value)
{
	int ret;
	int i;

	if (!hwcxext_check_i2c_regmap_valid(cfg)) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_write(cfg->regmap, reg_addr, value);
		if (ret == 0)
			break;

		mdelay(1);
	}
	return ret;
}

static int hwcxext_regmap_update_bits(
	struct hwcxext_codec_regmap_cfg *cfg,
	unsigned int reg_addr,
	unsigned int mask, unsigned int value)
{
	int ret;
	int i;

	if (!hwcxext_check_i2c_regmap_valid(cfg)) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_update_bits(cfg->regmap, reg_addr, mask, value);
		if (ret == 0)
			break;

		mdelay(1);
	}
	return ret;
}

static int hwcxext_regmap_xorw(struct hwcxext_codec_regmap_cfg *cfg,
	unsigned int reg_addr,
	unsigned int mask, unsigned int read_addr)
{
	int ret;
	unsigned int value = 0;

	if (!hwcxext_check_i2c_regmap_valid(cfg)) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	ret = hwcxext_regmap_read(cfg, read_addr, &value);
	if (ret < 0) {
		hwlog_info("%s: read reg 0x%x failed, ret = %d\n",
			__func__, read_addr, ret);
		return ret;
	}

#ifndef CONFIG_FINAL_RELEASE
	hwlog_debug("%s: read reg 0x%x = 0x%x\n", __func__, read_addr, value);
#endif

	value ^= mask;
#ifndef CONFIG_FINAL_RELEASE
	hwlog_debug("%s: after xor 0x%x, write reg 0x%x = 0x%x\n", __func__,
		mask, reg_addr, value);
#endif

	ret += hwcxext_regmap_write(cfg, reg_addr, value);
	return ret;
}

static int hwcxext_regmap_complex_write(
	struct hwcxext_codec_regmap_cfg *cfg,
	unsigned int reg_addr,
	unsigned int mask, unsigned int value)
{
	int ret;

	if (!hwcxext_check_i2c_regmap_valid(cfg)) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	if ((mask ^ cfg->value_mask) == 0)
		ret = hwcxext_regmap_write(cfg, reg_addr, value);
	else
		ret = hwcxext_regmap_update_bits(cfg, reg_addr, mask, value);

	return ret;
}

static void hwcxext_delay(unsigned int delay)
{
	if (delay == 0)
		return;

	if (delay < 20000) // less than 20 ms
		usleep_range(delay, delay + HWCXEXT_SLEEP_US_DEAT);
	else
		msleep(delay / 1000); // change to ms
}

#ifdef CONFIG_HUAWEI_DSM_AUDIO
static void hwcxext_append_dsm_report(char *dst, char *fmt, ...)
{
	va_list args;
	unsigned int buf_len;
	char tmp_str[DSM_BUF_SIZE] = {0};

	if ((dst == NULL) || (fmt == NULL)) {
		hwlog_err("%s, dst or src is NULL\n", __func__);
		return;
	}

	va_start(args, fmt);
	vscnprintf(tmp_str, (unsigned long)DSM_BUF_SIZE, (const char *)fmt,
		args);
	va_end(args);

	buf_len = DSM_BUF_SIZE - strlen(dst) - 1;
	if (strlen(dst) < DSM_BUF_SIZE - 1)
		if (strncat_s(dst, buf_len, tmp_str, strlen(tmp_str)) != EOK)
			hwlog_err("%s: strncat_s is failed\n", __func__);
}
#endif

static void hwcxext_dsm_report_by_i2c_error(int flag, int errno,
	struct i2c_err_info *info)
{
	char *report = NULL;

	if (errno == 0)
		return;

#ifdef CONFIG_HUAWEI_DSM_AUDIO
	report = kzalloc(sizeof(char) * DSM_BUF_SIZE, GFP_KERNEL);
	if (!report)
		return;

	if (flag == HWCXEXT_I2C_READ) /* read i2c error */
		hwcxext_append_dsm_report(report, "read ");
	else /* flag write i2c error == 1 */
		hwcxext_append_dsm_report(report, "write ");

	hwcxext_append_dsm_report(report, "errno %d", errno);

	if (info != NULL)
		hwcxext_append_dsm_report(report,
			" %u fail times of %u all times, err_details is 0x%lx",
			info->err_count, info->regs_num, info->err_details);
	audio_dsm_report_info(AUDIO_CODEC,
		DSM_HIFI_AK4376_I2C_ERR, "%s", report);
	hwlog_info("%s: dsm report, %s\n", __func__, report);
	kfree(report);
#endif
}

static void hwcxext_i2c_dsm_report_reg_nodes(int flag,
	int errno, struct i2c_err_info *info)
{
	hwcxext_dsm_report_by_i2c_error(flag, errno, info);
}

void hwcxext_i2c_dsm_report(int flag, int errno)
{
	hwcxext_dsm_report_by_i2c_error(flag, errno, NULL);
}

static int hwcxext_i2c_reg_node_ops(struct hwcxext_codec_regmap_cfg *cfg,
	struct hwcxext_reg_ctl *reg,
	int index, unsigned int reg_num)
{
	int ret = 0;
	int value;

	switch (reg->ctl_type) {
	case HWCXEXT_REG_CTL_TYPE_DELAY:
		hwcxext_delay(reg->delay);
		break;
	case HWCXEXT_PARAM_NODE_TYPE_SKIP:
		break;
	case HWCXEXT_PARAM_NODE_TYPE_REG_RXORW:
		hwlog_info("%s: rworw node %d/%u\n", __func__, index, reg_num);
		ret = hwcxext_regmap_xorw(cfg, reg->addr, reg->mask, reg->value);
		break;
	case HWCXEXT_REG_CTL_TYPE_R:
		ret = hwcxext_regmap_read(cfg, reg->addr, &value);
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: read node %d/%u, reg[0x%x]:0x%x, ret:%d\n",
			__func__, index, reg_num, reg->addr, value, ret);
#endif
		break;
	case HWCXEXT_REG_CTL_TYPE_W:
		hwcxext_delay(reg->delay);
		ret = hwcxext_regmap_complex_write(cfg, reg->addr,
			reg->mask, reg->value);
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: w node %d/%u,reg[0x%x]:0x%x,ret:%d delay:%d\n",
			__func__, index, reg_num,
			reg->addr, reg->value, ret, reg->delay);
#endif
		break;
	default:
		hwlog_err("%s: invalid argument\n", __func__);
		break;
	}
	return ret;
}

static void hwcxext_i2c_get_i2c_err_info(struct i2c_err_info *info,
	unsigned int index)
{
	if (index < I2C_STATUS_B64)
		info->err_details |= ((unsigned long int)1 << index);

	info->err_count++;
}

static int hwcxext_i2c_ctrl_write_regs(
	struct hwcxext_codec_regmap_cfg *cfg,
	struct hwcxext_reg_ctl_sequence *seq)
{
	int ret = 0;
	int i;
	int errno = 0;

	struct i2c_err_info info = {
		.regs_num    = 0,
		.err_count   = 0,
		.err_details = 0,
	};

	if (!hwcxext_check_i2c_regmap_valid(cfg)) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	if ((seq == NULL) || (seq->num == 0)) {
		hwlog_err("%s: reg node is invalid\n", __func__);
		return -EINVAL;
	}

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: node num %u\n",	__func__, seq->num);
#endif

	for (i = 0; i < (int)(seq->num); i++) {
		/* regmap node */
		ret = hwcxext_i2c_reg_node_ops(cfg,
			&(seq->regs[i]), i, seq->num);
		if (ret < 0) {
			hwlog_err("%s: ctl %d, reg 0x%x w/r 0x%x err, ret %d\n",
				__func__, i, seq->regs[i].addr,
				seq->regs[i].value, ret);
			hwcxext_i2c_get_i2c_err_info(&info, (unsigned int)i);
			errno = ret;
		}
	}
	info.regs_num = seq->num;
	hwcxext_i2c_dsm_report_reg_nodes(HWCXEXT_I2C_WRITE, errno, &info);
	return ret;
}

int hwcxext_i2c_ops_regs_seq(struct hwcxext_codec_regmap_cfg *cfg,
	struct hwcxext_reg_ctl_sequence *regs_seq)
{
	if (!hwcxext_check_i2c_regmap_valid(cfg)) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	if (regs_seq != NULL)
		return hwcxext_i2c_ctrl_write_regs(cfg, regs_seq);

	return 0;
}

static void hwcxext_i2c_free_regmap_cfg(
	struct hwcxext_codec_regmap_cfg **cfg)
{
	if (cfg == NULL || *cfg == NULL)
		return;

	hwcxext_kfree_ops((*cfg)->reg_writeable);
	hwcxext_kfree_ops((*cfg)->reg_unwriteable);
	hwcxext_kfree_ops((*cfg)->reg_readable);
	hwcxext_kfree_ops((*cfg)->reg_unreadable);
	hwcxext_kfree_ops((*cfg)->reg_volatile);
	hwcxext_kfree_ops((*cfg)->reg_unvolatile);
	hwcxext_kfree_ops((*cfg)->reg_defaults);
	hwcxext_i2c_free_regs_size_seq(&((*cfg)->regs_size_seq));

	kfree(*cfg);
	*cfg = NULL;
}

static unsigned int hwcxext_i2c_get_reg_value_mask(int val_bits)
{
	unsigned int mask;

	if (val_bits == HWCXEXT_REG_VALUE_B16)
		mask = HWCXEXT_REG_VALUE_M16;
	else if (val_bits == HWCXEXT_REG_VALUE_B24)
		mask = HWCXEXT_REG_VALUE_M24;
	else if (val_bits == HWCXEXT_REG_VALUE_B32)
		mask = HWCXEXT_REG_VALUE_M32;
	else
		mask = HWCXEXT_REG_VALUE_M8;

	return mask;
}

static bool hwcxext_i2c_is_reg_in_special_range(unsigned int reg_addr,
	int num, unsigned int *reg)
{
	int i;

	if ((num <= 0) || (reg == NULL)) {
		hwlog_err("%s: invalid arg\n", __func__);
		return false;
	}

	for (i = 0; i < num; i++) {
		if (reg[i] == reg_addr)
			return true;
	}
	return false;
}

static struct hwcxext_codec_regmap_cfg *hwcxext_i2c_get_regmap_cfg(
	struct device *dev)
{
	struct hwcxext_codec_v1_priv *hwcxext_codec = NULL;

	if (dev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return NULL;
	}

	hwcxext_codec = dev_get_drvdata(dev);
	if ((hwcxext_codec == NULL) || (hwcxext_codec->regmap_cfg == NULL)) {
		hwlog_err("%s: regmap_cfg invalid argument\n", __func__);
		return NULL;
	}
	return hwcxext_codec->regmap_cfg;
}

static bool hwcxext_i2c_writeable_reg(struct device *dev,
	unsigned int reg)
{
	struct hwcxext_codec_regmap_cfg *cfg = NULL;

	cfg = hwcxext_i2c_get_regmap_cfg(dev);
	if (cfg == NULL)
		return false;
	/* config writable or unwritable, can not config in dts at same time */
	if (cfg->num_writeable > 0)
		return hwcxext_i2c_is_reg_in_special_range(reg,
			cfg->num_writeable, cfg->reg_writeable);
	if (cfg->num_unwriteable > 0)
		return !hwcxext_i2c_is_reg_in_special_range(reg,
			cfg->num_unwriteable, cfg->reg_unwriteable);

	return true;
}

static bool hwcxext_i2c_readable_reg(struct device *dev,
	unsigned int reg)
{
	struct hwcxext_codec_regmap_cfg *cfg = NULL;

	cfg = hwcxext_i2c_get_regmap_cfg(dev);
	if (cfg == NULL)
		return false;
	/* config readable or unreadable, can not config in dts at same time */
	if (cfg->num_readable > 0)
		return hwcxext_i2c_is_reg_in_special_range(reg,
			cfg->num_readable, cfg->reg_readable);
	if (cfg->num_unreadable > 0)
		return !hwcxext_i2c_is_reg_in_special_range(reg,
			cfg->num_unreadable, cfg->reg_unreadable);

	return true;
}

static bool hwcxext_i2c_volatile_reg(struct device *dev,
	unsigned int reg)
{
	struct hwcxext_codec_regmap_cfg *cfg = NULL;

	cfg = hwcxext_i2c_get_regmap_cfg(dev);
	if (cfg == NULL)
		return false;
	/* config volatile or unvolatile, can not config in dts at same time */
	if (cfg->num_volatile > 0)
		return hwcxext_i2c_is_reg_in_special_range(reg,
			cfg->num_volatile, cfg->reg_volatile);
	if (cfg->num_unvolatile > 0)
		return !hwcxext_i2c_is_reg_in_special_range(reg,
			cfg->num_unvolatile, cfg->reg_unvolatile);

	return true;
}

static int hwcxext_i2c_parse_reg_defaults(struct device_node *node,
	struct hwcxext_codec_regmap_cfg *cfg_info)
{
	const char *reg_defaults_str = "reg_defaults";

	return hwcxext_get_prop_of_u32_array(node, reg_defaults_str,
		(u32 **)&cfg_info->reg_defaults, &cfg_info->num_defaults);
}

static int hwcxext_i2c_parse_regs_size(struct device_node *node,
	struct hwcxext_codec_regmap_cfg *cfg_info)
{
	const char *reg_defaults_str = "regs_size";
	struct hwcxext_regs_size_sequence *seq = NULL;
	int count = 0;
	int val_num;
	int ret;

	if (node == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (!of_property_read_bool(node, reg_defaults_str)) {
		hwlog_debug("%s: %s not existed, skip\n",
			reg_defaults_str, __func__);
		return 0;
	}

	seq = kzalloc(sizeof(struct hwcxext_regs_size_sequence), GFP_KERNEL);
	if (seq == NULL)
		return -ENOMEM;

	ret = hwcxext_get_prop_of_u32_array(node, reg_defaults_str,
			(u32 **)&seq->regs, &count);
	if ((count <= 0) || (ret < 0)) {
		hwlog_err("%s: get %s failed or count is 0\n",
			__func__, reg_defaults_str);
			ret = -EFAULT;
			goto hwcxext_i2c_parse_regs_size_err;
	}

	val_num = sizeof(struct hwcxext_regs_size_ctl) / sizeof(unsigned int);
	if ((count % val_num) != 0) {
		hwlog_err("%s: count %d %% val_num %d != 0\n",
			__func__, count, val_num);
		ret = -EFAULT;
		goto hwcxext_i2c_parse_regs_size_err;
	}

	seq->num = (unsigned int)(count / val_num);
	cfg_info->regs_size_seq = seq;
	return 0;

hwcxext_i2c_parse_regs_size_err:
	hwcxext_i2c_free_regs_size_seq(&seq);
	return ret;
}

static int hwcxext_i2c_parse_special_regs_range(
	struct device_node *node,
	struct hwcxext_codec_regmap_cfg *cfg_info)
{
	const char *reg_writeable_str   = "reg_writeable";
	const char *reg_unwriteable_str = "reg_unwriteable";
	const char *reg_readable_str    = "reg_readable";
	const char *reg_unreadable_str  = "reg_unreadable";
	const char *reg_volatile_str    = "reg_volatile";
	const char *reg_unvolatile_str  = "reg_unvolatile";
	int ret;

	cfg_info->num_writeable   = 0;
	cfg_info->num_unwriteable = 0;
	cfg_info->num_readable    = 0;
	cfg_info->num_unreadable  = 0;
	cfg_info->num_volatile    = 0;
	cfg_info->num_unvolatile  = 0;
	cfg_info->num_defaults    = 0;

	ret = hwcxext_get_prop_of_u32_array(node, reg_writeable_str,
		&cfg_info->reg_writeable, &cfg_info->num_writeable);
	ret += hwcxext_get_prop_of_u32_array(node, reg_unwriteable_str,
		&cfg_info->reg_unwriteable, &cfg_info->num_unwriteable);
	ret += hwcxext_get_prop_of_u32_array(node, reg_readable_str,
		&cfg_info->reg_readable, &cfg_info->num_readable);
	ret += hwcxext_get_prop_of_u32_array(node, reg_unreadable_str,
		&cfg_info->reg_unreadable, &cfg_info->num_unreadable);
	ret += hwcxext_get_prop_of_u32_array(node, reg_volatile_str,
		&cfg_info->reg_volatile, &cfg_info->num_volatile);
	ret += hwcxext_get_prop_of_u32_array(node, reg_unvolatile_str,
		&cfg_info->reg_unvolatile, &cfg_info->num_unvolatile);
	ret += hwcxext_i2c_parse_reg_defaults(node, cfg_info);
	ret += hwcxext_i2c_parse_regs_size(node, cfg_info);
	return ret;
}

static int hwcxext_get_prop_of_u32_value(struct device_node *node,
	const char *propname, u32 *value)
{
	int ret;
	u32 get_value;

	ret = of_property_read_u32(node, propname, &get_value);
	if (ret) {
		hwlog_err("%s: get %s failed\n", __func__, propname);
		return -EFAULT;
	}

	*value = get_value;
	return 0;
}

static int hwcxext_i2c_parse_remap_cfg(struct device_node *node,
	struct hwcxext_codec_regmap_cfg **cfg)
{
	struct hwcxext_codec_regmap_cfg *cfg_info = NULL;
	const char *reg_bits_str     = "reg_bits";
	const char *val_bits_str     = "val_bits";
	const char *cache_type_str   = "cache_type";
	const char *max_register_str = "max_register";
	int ret;

	cfg_info = kzalloc(sizeof(struct hwcxext_codec_regmap_cfg), GFP_KERNEL);
	if (cfg_info == NULL)
		return -ENOMEM;

	ret = hwcxext_get_prop_of_u32_value(node, reg_bits_str,
		(u32 *)&cfg_info->cfg.reg_bits);
	if (ret < 0)
		goto hwcxext_i2c_parse_remap_cfg_err;

	ret = hwcxext_get_prop_of_u32_value(node, val_bits_str,
		(u32 *)&cfg_info->cfg.val_bits);
	if (ret < 0)
		goto hwcxext_i2c_parse_remap_cfg_err;

	cfg_info->value_mask = hwcxext_i2c_get_reg_value_mask(
		cfg_info->cfg.val_bits);

	ret = hwcxext_get_prop_of_u32_value(node, cache_type_str,
		(u32 *)&cfg_info->cfg.cache_type);
	if ((ret < 0) || (cfg_info->cfg.cache_type > REGCACHE_FLAT)) {
		hwlog_err("%s: get cache_type failed\n", __func__);
		ret = -EFAULT;
		goto hwcxext_i2c_parse_remap_cfg_err;
	}

	ret = hwcxext_get_prop_of_u32_value(node, max_register_str,
		&cfg_info->cfg.max_register);
	if (ret < 0)
		goto hwcxext_i2c_parse_remap_cfg_err;

	ret = hwcxext_i2c_parse_special_regs_range(node, cfg_info);
	if (ret < 0)
		goto hwcxext_i2c_parse_remap_cfg_err;

	*cfg = cfg_info;
	return 0;

hwcxext_i2c_parse_remap_cfg_err:
	hwcxext_i2c_free_regmap_cfg(&cfg_info);
	return ret;
}

static unsigned int hwcxext_v1_get_register_size(
	struct hwcxext_codec_regmap_cfg *cfg,
	unsigned int reg)
{
	unsigned int i;
	unsigned int default_size = 1;
	struct hwcxext_regs_size_sequence *seq = cfg->regs_size_seq;

	for (i = 0; i < seq->num; i++) {
		if (seq->regs[i].addr == reg)
			return seq->regs[i].size;
	}

	return default_size;
}

static int hwcxext_v1_reg_raw_write(struct i2c_client *client,
	unsigned int reg, const void *val, size_t val_count)
{
	u8 buf[HWCXEXT_V1_I2C_LEN + HWCXEXT_V1_MAX_EQ_COEFF];
	int ret;

	if (val_count + HWCXEXT_V1_I2C_LEN > sizeof(buf)) {
		hwlog_err("%s: val_count %d is invalid\n", __func__, val_count);
		return -EINVAL;
	}

	buf[0] = reg >> HWCXEXT_REG_VALUE_B8;
	buf[1] = reg & HWCXEXT_REG_VALUE_M8;

	ret = memcpy_s(buf + HWCXEXT_V1_I2C_LEN, HWCXEXT_V1_MAX_EQ_COEFF,
		val, val_count);
	if (ret != EOK) {
		hwlog_err("%s: memcpy_s fail, err:%d\n", __func__, ret);
		return -EIO;
	}

	ret = i2c_master_send(client, buf, val_count + HWCXEXT_V1_I2C_LEN);
	if (ret != val_count + HWCXEXT_V1_I2C_LEN) {
		hwlog_err("%s: I2C write failed, ret: %d\n", __func__, ret);
		return -EIO;
	}

	return 0;
}

/* reg write */
static int hwcxext_v1_reg_write(void *context, unsigned int reg,
	unsigned int value)
{
	__le32 raw_value;
	unsigned int size;
	struct i2c_client *client = context;
	struct hwcxext_codec_v1_priv *hwcxext_codec = i2c_get_clientdata(client);

	size = hwcxext_v1_get_register_size(hwcxext_codec->regmap_cfg, reg);
	raw_value = cpu_to_le32(value);
	return hwcxext_v1_reg_raw_write(client, reg, &raw_value, size);
}

/* reg read */
static int hwcxext_v1_reg_read(void *context, unsigned int reg,
	unsigned int *value)
{
	struct i2c_client *client = context;
	struct hwcxext_codec_v1_priv *hwcxext_codec = i2c_get_clientdata(client);
	__le32 recv_buf = 0;
	struct i2c_msg msgs[HWCXEXT_V1_I2C_LEN];
	unsigned int size;
	u8 send_buf[HWCXEXT_V1_I2C_LEN];
	int ret;

	size = hwcxext_v1_get_register_size(hwcxext_codec->regmap_cfg, reg);
	send_buf[0] = reg >> HWCXEXT_REG_VALUE_B8;
	send_buf[1] = reg & HWCXEXT_REG_VALUE_M8;

	msgs[0].addr = client->addr;
	msgs[0].len = sizeof(send_buf);
	msgs[0].buf = send_buf;
	msgs[0].flags = 0;

	msgs[1].addr = client->addr;
	msgs[1].len = size;
	msgs[1].buf = (u8 *)&recv_buf;
	msgs[1].flags = I2C_M_RD;

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs)) {
		hwlog_err("%s: failed to read reg, ret:%d\n", __func__, ret);
		return -EIO;
	}

	*value = le32_to_cpu(recv_buf);
	return 0;
}

static int hwcxext_i2c_regmap_cfg_data_init(struct device_node *node,
	struct hwcxext_codec_regmap_cfg *cfg)
{
	int val_num;
	const char *custom_r_w_str = "need_custom_read_write_func";

	val_num = sizeof(struct reg_default) / sizeof(unsigned int);
	if (cfg->num_defaults > 0) {
		if ((cfg->num_defaults % val_num) != 0) {
			hwlog_err("%s: reg_defaults %d%%%d != 0\n",
				__func__, cfg->num_defaults, val_num);
			return -EFAULT;
		}
	}

	hwlog_info("%s: regmap_cfg get w%d,%d,r%d,%d,v%d,%d,default%d\n",
		__func__, cfg->num_writeable, cfg->num_unwriteable,
		cfg->num_readable, cfg->num_unreadable, cfg->num_volatile,
		cfg->num_unvolatile, cfg->num_defaults / val_num);

	if (cfg->num_defaults > 0) {
		cfg->num_defaults /= val_num;
		cfg->cfg.reg_defaults = cfg->reg_defaults;
		cfg->cfg.num_reg_defaults = (unsigned int)cfg->num_defaults;
	}

	cfg->cfg.writeable_reg = hwcxext_i2c_writeable_reg;
	cfg->cfg.readable_reg  = hwcxext_i2c_readable_reg;
	cfg->cfg.volatile_reg  = hwcxext_i2c_volatile_reg;

	/* needs custom read/write functions for various register lengths */
	if (of_property_read_bool(node, custom_r_w_str)) {
		hwlog_info("%s: need custom read write func\n", __func__);
		cfg->cfg.reg_read = hwcxext_v1_reg_read;
		cfg->cfg.reg_write = hwcxext_v1_reg_write;
	}
	return 0;
}

int hwcxext_i2c_regmap_init(struct i2c_client *i2c,
	struct hwcxext_codec_v1_priv *hwcxext_codec)
{
	const char *regmap_cfg_str = "regmap_cfg";
	struct hwcxext_codec_regmap_cfg *cfg = NULL;
	struct device_node *node = NULL;
	int ret;

	if (i2c == NULL || hwcxext_codec == NULL) {
		hwlog_err("%s: i2c or hwcxext_codec is NULL\n", __func__);
		return -EINVAL;
	}

	node = of_get_child_by_name(i2c->dev.of_node, regmap_cfg_str);
	if (node == NULL) {
		hwlog_debug("%s: regmap_cfg not existed, skip\n", __func__);
		return 0;
	}

	ret = hwcxext_i2c_parse_remap_cfg(node, &hwcxext_codec->regmap_cfg);
	if (ret < 0)
		return ret;

	ret = hwcxext_i2c_regmap_cfg_data_init(node, hwcxext_codec->regmap_cfg);
	if (ret < 0)
		goto hwcxext_regmap_init_err_out;

	cfg = hwcxext_codec->regmap_cfg;
	cfg->regmap = devm_regmap_init(&i2c->dev, NULL, i2c, &cfg->cfg);
	if (IS_ERR_OR_NULL(cfg->regmap)) {
		hwlog_err("%s: regmap_init_i2c regmap failed\n", __func__);
		ret = -EFAULT;
		goto hwcxext_regmap_init_err_out;
	}
	return 0;

hwcxext_regmap_init_err_out:
	hwcxext_i2c_free_regmap_cfg(&hwcxext_codec->regmap_cfg);
	return ret;
}


void hwcxext_i2c_regmap_deinit(struct hwcxext_codec_regmap_cfg *cfg)
{
	if (cfg == NULL) {
		hwlog_err("%s: cfg is NULL\n", __func__);
		return;
	}

	if (cfg->regmap) {
		regmap_exit(cfg->regmap);
		cfg->regmap = NULL;
	}

	hwcxext_i2c_free_regmap_cfg(&cfg);
}

