/*
 * hwevext_i2c_ops.c
 *
 * hwevext i2c ops driver
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
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
#include <linux/regmap.h>
#include <huawei_platform/log/hw_log.h>

#include <dsm/dsm_pub.h>
#ifdef CONFIG_HUAWEI_DSM_AUDIO
#include <dsm_audio/dsm_audio.h>
#endif

#include "hwevext_i2c_ops.h"

#define HWLOG_TAG hwevext_i2c_ops
HWLOG_REGIST();

#define REG_PRINT_NUM  8
#define RETRY_TIMES    3
#define HWEVEXT_SLEEP_US_DEAT 500
#define I2C_STATUS_B64 64

#ifndef unused
#define unused(x) ((void)(x))
#endif

#ifndef hwevext_kfree_ops
#define hwevext_kfree_ops(p) \
do {\
	if (p != NULL) { \
		kfree(p); \
		p = NULL; \
	} \
} while (0)
#endif


/*
 * 0 read reg node:   r-reg-addr | mask | value     | delay | 0
 * 1 write reg node:  w-reg-addr | mask | value     | delay | 1
 * 2 delay node:      0          | 0    | 0         | delay | 2
 * 3 skip node:       0          | 0    | 0         | 0     | 3
 * 4 rxorw node:      w-reg-addr | mask | r-reg-addr| delay | 4
 *   this mask is xor mask
 */
enum hwevext_reg_ctl_type {
	HWEVEXT_REG_CTL_TYPE_R = 0,  /* read reg        */
	HWEVEXT_REG_CTL_TYPE_W,      /* write reg       */
	HWEVEXT_REG_CTL_TYPE_DELAY,  /* only time delay */
	HWEVEXT_PARAM_NODE_TYPE_SKIP,
	/* read, xor, write */
	HWEVEXT_PARAM_NODE_TYPE_REG_RXORW,
	EXTERN_DAC_REG_CTL_TYPE_MAX,
};


/* reg val_bits */
#define HWEVEXT_REG_VALUE_B8     8  /* val_bits == 8  */
#define HWEVEXT_REG_VALUE_B16    16 /* val_bits == 16 */
#define HWEVEXT_REG_VALUE_B24    24 /* val_bits == 24 */
#define HWEVEXT_REG_VALUE_B32    32 /* val_bits == 32 */

/* reg value mask by reg val_bits */
#define HWEVEXT_ADC_REG_VALUE_M8     0xFF
#define HWEVEXT_ADC_REG_VALUE_M16    0xFFFF
#define HWEVEXT_ADC_REG_VALUE_M24    0xFFFFFF
#define HWEVEXT_ADC_REG_VALUE_M32    0xFFFFFFFF

struct i2c_err_info {
	unsigned int regs_num;
	unsigned int err_count;
	unsigned long int err_details;
};

static unsigned int g_regmap_value_mask = HWEVEXT_ADC_REG_VALUE_M8;
static struct regmap *g_hwevext_regmap;

void hwevext_i2c_set_reg_value_mask(int val_bits)
{
	switch (val_bits) {
	case HWEVEXT_REG_VALUE_B8:
		g_regmap_value_mask = HWEVEXT_ADC_REG_VALUE_M8;
		break;
	case HWEVEXT_REG_VALUE_B16:
		g_regmap_value_mask = HWEVEXT_ADC_REG_VALUE_M16;
		break;
	case HWEVEXT_REG_VALUE_B24:
		g_regmap_value_mask = HWEVEXT_ADC_REG_VALUE_M24;
		break;
	case HWEVEXT_REG_VALUE_B32:
		g_regmap_value_mask = HWEVEXT_ADC_REG_VALUE_M32;
		break;
	default:
		hwlog_err("%s: val_bits:%d is invalid\n", __func__, val_bits);
		break;
	}
}

void hwevext_i2c_ops_store_regmap(struct regmap *regmap)
{
	if (regmap == NULL)
		return;

	g_hwevext_regmap = regmap;
}

static inline bool hwevext_check_i2c_regmap_valid(void)
{
	if (g_hwevext_regmap)
		return true;

	return false;
}

void hwevext_i2c_free_reg_ctl(struct hwevext_reg_ctl_sequence *reg_ctl)
{
	if (reg_ctl == NULL)
		return;

	hwevext_kfree_ops(reg_ctl->regs);
	kfree(reg_ctl);
	reg_ctl = NULL;
}

static int hwevext_get_prop_of_u32_array(struct device_node *node,
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
		goto hwevext_get_prop_err_out;
	}

	*value = array;
	*num = count;
	return 0;

hwevext_get_prop_err_out:
	hwevext_kfree_ops(array);
	return ret;
}

static int hwevext_i2c_parse_reg_ctl(
	struct hwevext_reg_ctl_sequence **reg_ctl, struct device_node *node,
	const char *ctl_str)
{
	struct hwevext_reg_ctl_sequence *ctl = NULL;
	int count = 0;
	int val_num;
	int ret;

	if ((node == NULL) || (ctl_str == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	ctl = kzalloc(sizeof(struct hwevext_reg_ctl_sequence), GFP_KERNEL);
	if (ctl == NULL)
		return -ENOMEM;

	ret = hwevext_get_prop_of_u32_array(node, ctl_str,
		(u32 **)&ctl->regs, &count);
	if ((count <= 0) || (ret < 0)) {
		hwlog_err("%s: get %s failed or count is 0\n",
			__func__, ctl_str);
		ret = -EFAULT;
		goto parse_reg_ctl_err_out;
	}

	val_num = sizeof(struct hwevext_reg_ctl) / sizeof(unsigned int);
	if ((count % val_num) != 0) {
		hwlog_err("%s: count %d %% val_num %d != 0\n",
			__func__, count, val_num);
		ret = -EFAULT;
		goto parse_reg_ctl_err_out;
	}

	ctl->num = (unsigned int)(count / val_num);
	*reg_ctl = ctl;
	return 0;

parse_reg_ctl_err_out:
	hwevext_i2c_free_reg_ctl(ctl);
	return ret;
}

static void hwevext_print_regs_info(const char *seq_str,
	struct hwevext_reg_ctl_sequence *reg_ctl)
{
	unsigned int print_node_num;
	unsigned int i;
	struct hwevext_reg_ctl *reg = NULL;

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

int hwevext_i2c_parse_dt_reg_ctl(struct i2c_client *i2c,
	struct hwevext_reg_ctl_sequence **reg_ctl, const char *seq_str)
{
	int ret;

	if (!of_property_read_bool(i2c->dev.of_node, seq_str)) {
		hwlog_debug("%s: %s not existed, skip\n", seq_str, __func__);
		return 0;
	}
	ret = hwevext_i2c_parse_reg_ctl(reg_ctl,
		i2c->dev.of_node, seq_str);
	if (ret < 0) {
		hwlog_err("%s: parse %s failed\n", __func__, seq_str);
		goto hwevext_parse_dt_reg_ctl_err_out;
	}

	hwevext_print_regs_info(seq_str, *reg_ctl);

hwevext_parse_dt_reg_ctl_err_out:
	return ret;
}

static int hwevext_regmap_read(unsigned int reg_addr,
	unsigned int *value)
{
	int ret = 0;
	int i;

	if (!hwevext_check_i2c_regmap_valid()) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_read(g_hwevext_regmap, reg_addr, value);
		if (ret == 0)
			break;

		mdelay(1);
	}
	return ret;
}

static  int hwevext_regmap_write(unsigned int reg_addr, unsigned int value)
{
	int ret;
	int i;

	if (!hwevext_check_i2c_regmap_valid()) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_write(g_hwevext_regmap, reg_addr, value);
		if (ret == 0)
			break;

		mdelay(1);
	}
	return ret;
}

static int hwevext_regmap_update_bits(unsigned int reg_addr,
	unsigned int mask, unsigned int value)
{
	int ret;
	int i;

	if (!hwevext_check_i2c_regmap_valid()) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_update_bits(g_hwevext_regmap, reg_addr, mask, value);
		if (ret == 0)
			break;

		mdelay(1);
	}
	return ret;
}

static int hwevext_regmap_xorw(unsigned int reg_addr,
	unsigned int mask, unsigned int read_addr)
{
	int ret;
	unsigned int value = 0;

	if (!hwevext_check_i2c_regmap_valid()) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	ret = hwevext_regmap_read(read_addr, &value);
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

	ret += hwevext_regmap_write(reg_addr, value);
	return ret;
}

static int hwevext_regmap_complex_write(unsigned int reg_addr,
	unsigned int mask, unsigned int value)
{
	int ret;

	if (!hwevext_check_i2c_regmap_valid()) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	if ((mask ^ g_regmap_value_mask) == 0)
		ret = hwevext_regmap_write(reg_addr, value);
	else
		ret = hwevext_regmap_update_bits(reg_addr, mask, value);

	return ret;
}

static void hwevext_delay(unsigned int delay)
{
	if (delay == 0)
		return;

	if (delay < 20000)
		usleep_range(delay, delay + HWEVEXT_SLEEP_US_DEAT);
	else
		msleep(delay / 1000);

}

#ifdef CONFIG_HUAWEI_DSM_AUDIO
static void hwevext_append_dsm_report(char *dst, char *fmt, ...)
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
		strncat(dst, tmp_str, buf_len);
}
#endif

static void hwevext_dsm_report_by_i2c_error(int flag, int errno,
	struct i2c_err_info *info)
{
	char *report = NULL;

	if (errno == 0)
		return;

#ifdef CONFIG_HUAWEI_DSM_AUDIO
	report = kzalloc(sizeof(char) * DSM_BUF_SIZE, GFP_KERNEL);
	if (!report)
		return;

	if (flag == HWEVEXT_I2C_READ) /* read i2c error */
		hwevext_append_dsm_report(report, "read ");
	else /* flag write i2c error == 1 */
		hwevext_append_dsm_report(report, "write ");

	hwevext_append_dsm_report(report, "errno %d", errno);

	if (info != NULL)
		hwevext_append_dsm_report(report,
			" %u fail times of %u all times, err_details is 0x%lx",
			info->err_count, info->regs_num, info->err_details);
	audio_dsm_report_info(AUDIO_CODEC, DSM_HIFI_AK4376_I2C_ERR, "%s", report);
	hwlog_info("%s: dsm report, %s\n", __func__, report);
	kfree(report);
#endif
}

static void hwevext_i2c_dsm_report_reg_nodes(int flag,
	int errno, struct i2c_err_info *info)
{

	hwevext_dsm_report_by_i2c_error(flag, errno, info);
}

void hwevext_i2c_dsm_report(int flag, int errno)
{
	hwevext_dsm_report_by_i2c_error(flag, errno, NULL);
}

static int hwevext_i2c_reg_node_ops(struct hwevext_reg_ctl *reg,
	int index, unsigned int reg_num)
{
	int ret = 0;
	int value;

	switch (reg->ctl_type) {
	case HWEVEXT_REG_CTL_TYPE_DELAY:
		hwevext_delay(reg->delay);
		break;
	case HWEVEXT_PARAM_NODE_TYPE_SKIP:
		break;
	case HWEVEXT_PARAM_NODE_TYPE_REG_RXORW:
		hwlog_info("%s: rworw node %d/%u\n", __func__, index, reg_num);
		ret = hwevext_regmap_xorw(reg->addr, reg->mask, reg->value);
		break;
	case HWEVEXT_REG_CTL_TYPE_R:
		ret = hwevext_regmap_read(reg->addr, &value);
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: read node %d/%u, reg[0x%x]:0x%x, ret:%d\n",
			__func__, index, reg_num, reg->addr, value, ret);
#endif
		break;
	case HWEVEXT_REG_CTL_TYPE_W:
		hwevext_delay(reg->delay);
		ret = hwevext_regmap_complex_write(reg->addr,
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

static void hwevext_i2c_get_i2c_err_info(struct i2c_err_info *info,
	unsigned int index)
{
	if (index < I2C_STATUS_B64)
		info->err_details |= ((unsigned long int)1 << index);

	info->err_count++;
}

static int hwevext_i2c_ctrl_write_regs(struct hwevext_reg_ctl_sequence *seq)
{
	int ret = 0;
	int i;
	int errno = 0;

	struct i2c_err_info info = {
		.regs_num    = 0,
		.err_count   = 0,
		.err_details = 0,
	};

	if (!hwevext_check_i2c_regmap_valid()) {
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
		ret = hwevext_i2c_reg_node_ops(&(seq->regs[i]), i, seq->num);
		if (ret < 0) {
			hwlog_err("%s: ctl %d, reg 0x%x w/r 0x%x err, ret %d\n",
				__func__, i, seq->regs[i].addr,
				seq->regs[i].value, ret);
			hwevext_i2c_get_i2c_err_info(&info, (unsigned int)i);
			errno = ret;
		}
	}
	info.regs_num = seq->num;
	hwevext_i2c_dsm_report_reg_nodes(HWEVEXT_I2C_WRITE, errno, &info);
	return ret;
}


int hwevext_i2c_ops_regs_seq(struct hwevext_reg_ctl_sequence *regs_seq)
{

	if (!hwevext_check_i2c_regmap_valid()) {
		hwlog_err("%s: regmap is invaild\n", __func__);
		return -EINVAL;
	}

	if (regs_seq != NULL)
		return hwevext_i2c_ctrl_write_regs(regs_seq);

	return 0;
}

