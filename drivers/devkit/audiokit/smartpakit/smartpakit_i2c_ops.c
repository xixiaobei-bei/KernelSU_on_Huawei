/*
 * smartpakit_i2c_ops.c
 *
 * smartpakit i2c driver
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
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <huawei_platform/log/hw_log.h>
#include <dsm/dsm_pub.h>
#include "securec.h"

#ifdef CONFIG_SND_SOC_AU_PA
#include <sound/au_pa.h>
#endif

#ifdef CONFIG_HUAWEI_DSM_AUDIO_MODULE
#define CONFIG_HUAWEI_DSM_AUDIO
#endif
#ifdef CONFIG_HUAWEI_DSM_AUDIO
#include <dsm_audio/dsm_audio.h>
#endif

#ifdef CONFIG_HUAWEI_DEVICEBOX_AUDIO_MODULE
#define CONFIG_HUAWEI_DEVICEBOX_AUDIO
#endif

#ifdef CONFIG_HUAWEI_DEVICEBOX_AUDIO
#include <boxid/boxid.h>
#endif

#include "smartpakit.h"

#define HWLOG_TAG smartpakit
HWLOG_REGIST();

#ifdef CONFIG_HUAWEI_DSM_AUDIO
#define DSM_BUF_SIZE DSM_SMARTPA_BUF_SIZE
#endif

#define HISMARTPA_OTP_REG_NUM      8
#define RETRY_TIMES                3
#define KIT_I2C_READ               0
#define KIT_I2C_WRITE              1
#define AW_CORRECT_COEFF          (1 << 13)
#define AW_TRIM_VALUE_MASK         0x03ff
#define AW_TRIM_VALUE_BASE         1000

#define aw_fill_short_type_sign(a) ((((a) & 0x200) == 0) ? (a) : ((a) | 0xfc00))
#define TIME_COUNT_FOR_DMD 5
#define TIME_DIFF 60 // 60s

#ifdef CONFIG_SND_SOC_AU_PA
int __attribute__((weak)) au_pa_cfg_iomux(enum au_pa_mode mode)
{
	return 0;
}
#endif

/* 0 i2c device not init completed, 1 init completed */
static bool smartpakit_i2c_probe_skip[SMARTPAKIT_PA_ID_MAX] = { 0, 0, 0, 0 , 0, 0, 0, 0 };

static void delete_list_info_by_given_chip_id(unsigned int chip_id)
{
	struct i2c_dev_info *pos = NULL;
	struct i2c_dev_info *n = NULL;
	struct smartpakit_priv *pakit_priv = NULL;

	pakit_priv = smartpakit_get_misc_priv();
	if (pakit_priv == NULL) {
		hwlog_err("%s: smartpakit_priv is NULL\n", __func__);
		return;
	}

	list_for_each_entry_safe(pos, n, &pakit_priv->i2c_dev_list, list) {
		if (pos->chip_id != chip_id)
			continue;
		list_del(&pos->list);
		kfree(pos);
	}
}

static void insert_i2c_dev_info_list(struct smartpakit_i2c_priv *i2c_priv,
	unsigned int status)
{
	struct i2c_dev_info *dev_info = NULL;
	struct smartpakit_priv *pakit_priv = NULL;
	unsigned int model_len;

	pakit_priv = smartpakit_get_misc_priv();
	if (pakit_priv == NULL) {
		hwlog_err("%s: smartpakit_priv is NULL\n", __func__);
		return;
	}

	dev_info = kzalloc(sizeof(struct i2c_dev_info), GFP_KERNEL);
	if (dev_info == NULL)
		return;
	dev_info->chip_id = i2c_priv->chip_id;
	dev_info->status = status;
	dev_info->addr = pakit_priv->misc_i2c_use_pseudo_addr ?
		i2c_priv->i2c_pseudo_addr : i2c_priv->i2c->addr;
	model_len = strlen(i2c_priv->chip_model);
	model_len = (model_len < sizeof(dev_info->chip_model)) ?
		model_len : (sizeof(dev_info->chip_model) - 1);
	if (strncpy_s(dev_info->chip_model, sizeof(dev_info->chip_model),
		i2c_priv->chip_model, model_len) != EOK)
		hwlog_err("%s: strncpy_s is failed\n", __func__);

	list_add(&dev_info->list, &pakit_priv->i2c_dev_list);
}

static inline int smartpakit_check_i2c_regmap_cfg(
	struct smartpakit_i2c_priv *i2c_priv)
{
	if ((i2c_priv == NULL) || (i2c_priv->regmap_cfg == NULL) ||
		(i2c_priv->regmap_cfg->regmap == NULL))
		return -EINVAL;
	return 0;
}

static inline int smartpakit_regmap_read(struct regmap *regmap,
	unsigned int reg_addr, unsigned int *value)
{
	int ret = 0;
	int i;

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_read(regmap, reg_addr, value);
		if (ret == 0)
			break;

		mdelay(1);
	}
	return ret;
}

static inline int smartpakit_regmap_write(struct regmap *regmap,
	unsigned int reg_addr, unsigned int value)
{
	int ret = 0;
	int i;

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_write(regmap, reg_addr, value);
		if (ret == 0)
			break;

		mdelay(1);
	}
	return ret;
}

static inline int smartpakit_regmap_update_bits(struct regmap *regmap,
	unsigned int reg_addr, unsigned int mask, unsigned int value)
{
	int ret = 0;
	int i;

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_update_bits(regmap, reg_addr, mask, value);
		if (ret == 0)
			break;

		mdelay(1);
	}
	return ret;
}

static int smartpakit_regmap_xorw(struct regmap *regmap,
	unsigned int reg_addr, unsigned int mask, unsigned int read_addr)
{
	int ret;
	unsigned int value = 0;

	ret = smartpakit_regmap_read(regmap, read_addr, &value);
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

	ret += smartpakit_regmap_write(regmap, reg_addr, value);
	return ret;
}

static int smartpakit_regmap_complex_write(struct smartpakit_regmap_cfg *cfg,
	unsigned int reg_addr, unsigned int mask, unsigned int value)
{
	int ret;

	if (cfg == NULL)
		return -EINVAL;

	if ((mask ^ cfg->value_mask) == 0)
		ret = smartpakit_regmap_write(cfg->regmap, reg_addr, value);
	else
		ret = smartpakit_regmap_update_bits(cfg->regmap, reg_addr,
			mask, value);

	return ret;
}

static void smartpakit_delay(unsigned int delay, int index, int num)
{
	if (delay == 0)
		return;
	if ((index < 0) || (num < 0))
		return;

	if (index == num)
		msleep(delay);
	else
		mdelay(delay);
}

#ifdef CONFIG_HUAWEI_DSM_AUDIO
static void smartpakit_append_dsm_report(char *dst, char *fmt, ...)
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
	if (strlen(dst) < (DSM_BUF_SIZE - 1))
		strncat(dst, tmp_str, buf_len);
}
#endif

static void smartpakit_dsm_report_by_i2c_error(const char *model, int id,
	int flag, int errno, struct i2c_err_info *info)
{
	char *report = NULL;

	unused(report);
	if (errno == 0)
		return;

#ifdef CONFIG_HUAWEI_DSM_AUDIO
	report = kzalloc(sizeof(char) * DSM_BUF_SIZE, GFP_KERNEL);
	if (!report)
		return;

	smartpakit_append_dsm_report(report, "%s_%d i2c ", model, id);

	if (flag == KIT_I2C_READ) /* read i2c error */
		smartpakit_append_dsm_report(report, "read ");
	else /* flag write i2c error == 1 */
		smartpakit_append_dsm_report(report, "write ");

	smartpakit_append_dsm_report(report, "errno %d", errno);

	if (info != NULL)
		smartpakit_append_dsm_report(report,
			" %u fail times of %u all times, err_details is 0x%lx",
			info->err_count, info->regs_num, info->err_details);

	audio_dsm_report_info(AUDIO_SMARTPA, DSM_SMARTPA_I2C_ERR, "%s", report);
	hwlog_info("%s: dsm report, %s\n", __func__, report);
	kfree(report);
#endif
}

static void smartpakit_i2c_dsm_report(struct smartpakit_i2c_priv *i2c_priv,
	int flag, int errno, struct i2c_err_info *info)
{
	if (i2c_priv->probe_completed == 0)
		return;
	smartpakit_dsm_report_by_i2c_error(i2c_priv->chip_model,
		(int)i2c_priv->chip_id, flag, errno, info);
}

#ifdef CONFIG_HUAWEI_DSM_AUDIO
static void add_probed_result_of_given_chip_id(char *report, unsigned int id,
	struct list_head *head)
{
	struct i2c_dev_info *dev_info = NULL;

	list_for_each_entry(dev_info, head, list) {
		if (dev_info->chip_id != id)
			continue;
		smartpakit_append_dsm_report(report, "%u:", id);
		if (dev_info->status == 0)
			smartpakit_append_dsm_report(report, "f,");
		else
			smartpakit_append_dsm_report(report, "s,");
		break;
	}
}

static void add_probed_devs_by_given_chip_id(char *report, unsigned int id,
	struct list_head *head)
{
	struct i2c_dev_info *dev_info = NULL;

	list_for_each_entry(dev_info, head, list) {
		if (dev_info->chip_id != id)
			continue;
		smartpakit_append_dsm_report(report, "%s_%x,",
			dev_info->chip_model, dev_info->addr);
	}
}

static bool is_i2c_probe_dsm_report_required(struct smartpakit_priv *pakit_priv)
{
	if (pakit_priv == NULL)
		return false;

	if (pakit_priv->pa_num == pakit_priv->i2c_num)
		return false;

	/*
	 * if i2c check is success, but register to smartpa platform failed,
	 * need not to upload dsm report
	 */
	if (pakit_priv->chip_register_failed)
		return false;

	if ((pakit_priv->algo_in != SMARTPAKIT_ALGO_IN_CODEC_DSP) &&
		(pakit_priv->algo_in != SMARTPAKIT_ALGO_IN_SOC_DSP) &&
		(pakit_priv->algo_in != SMARTPAKIT_ALGO_IN_SIMPLE_WITH_I2C))
		return false;
	return true;
}

void smartpakit_handle_i2c_probe_dsm_report(struct smartpakit_priv *pakit_priv)
{
	char *report = NULL;
	unsigned int i;

	if (!is_i2c_probe_dsm_report_required(pakit_priv))
		return;

	report = kzalloc(sizeof(char) * DSM_BUF_SIZE, GFP_KERNEL);
	if (report == NULL)
		return;

	smartpakit_append_dsm_report(report, "Probed%u/%u;",
		pakit_priv->i2c_num, pakit_priv->pa_num);
	for (i = 0; i < SMARTPAKIT_PA_ID_MAX; i++) {
		add_probed_result_of_given_chip_id(report, i,
			&pakit_priv->i2c_dev_list);
		add_probed_devs_by_given_chip_id(report, i,
			&pakit_priv->i2c_dev_list);
	}
	hwlog_info("%s: %s", __func__, report);

	audio_dsm_report_info(AUDIO_SMARTPA, DSM_SMARTPA_I2C_ERR, "%s",
		report);
	kfree(report);
}
#endif

static bool smartpakit_i2c_is_request_same_irq_gpio(int gpio)
{
	struct smartpakit_priv *pakit_priv = NULL;
	int i;

	pakit_priv = smartpakit_get_misc_priv();
	if (pakit_priv == NULL) {
		hwlog_err("%s: smartpakit_priv is NULL\n", __func__);
		return false;
	}

	for (i = 0; i < pakit_priv->pa_num; i++) {
		if (pakit_priv->gpio_of_irq[i] == gpio)
			return true;
	}
	return false;
}

static bool smartpakit_i2c_is_request_same_reset_gpio(int gpio)
{
	struct smartpakit_priv *pakit_priv = NULL;
	int i;

	pakit_priv = smartpakit_get_misc_priv();
	if (pakit_priv == NULL) {
		hwlog_err("%s: smartpakit_priv is NULL\n", __func__);
		return false;
	}

	for (i = 0; i < pakit_priv->pa_num; i++) {
		if (pakit_priv->gpio_of_hw_reset[i] == gpio)
			return true;
	}
	return false;
}

#ifdef CONFIG_SND_SOC_AU_PA
static void smartpakit_switch_hismartpa_gpio(const char *model, int state, unsigned int gpio, int i2c_state)
{
	int ret = 0;
	const char *hismartpa_str = "hi6565";
	const char *hisipa_type = "04-01"; // hismartpa chip_model type

	if (strcmp(model, hismartpa_str) != 0 &&
		strcmp(model, hisipa_type) != 0) {
		return;
	}

	if (state == GPIO_LOW) {
		/* config gpio mode */
		ret = au_pa_cfg_iomux(RESET_MODE);
		if (ret < 0)
			hwlog_err("RESET_MODE err, ret is %d\n", ret);
		gpio_direction_output(gpio, i2c_state);
		hwlog_info("au_pa_cfg_iomux RESET_MODE\n");
	} else {
		/* recovery fuction mode */
		mdelay(1);
		ret = au_pa_cfg_iomux(NORMAL_MODE);
		if (ret < 0)
			hwlog_err("NORMAL_MODE err, ret is %d\n", ret);
		gpio_direction_input(gpio);
		hwlog_info("au_pa_cfg_iomux NORMAL_MODE\n");
	}
}
#endif

static void smartpakit_i2c_hw_reset_gpio_ctl(struct smartpakit_i2c_priv *i2c_priv,
	struct smartpakit_gpio_sequence *sequence)
{
	struct smartpakit_gpio_state *node = NULL;
	int j;
	int num;

	if ((sequence->node == NULL) || (sequence->num == 0)) {
		hwlog_info("%s: invalid gpio seq\n", __func__);
		return;
	}

	num = (int)sequence->num;
	node = sequence->node;

	for (j = 0; j < num; j++) {
		hwlog_info("%s: gpio %d set %u, delay %u\n", __func__, i2c_priv->hw_reset->gpio,
			node[j].state, node[j].delay);
		gpio_direction_output((unsigned int)i2c_priv->hw_reset->gpio, (int)node[j].state);
#ifdef CONFIG_SND_SOC_AU_PA
		if (i2c_priv->irq_handler != NULL)
			smartpakit_switch_hismartpa_gpio(i2c_priv->chip_model,
				(int)node[j].state,
				(unsigned int)i2c_priv->irq_handler->gpio,
				(int)i2c_priv->i2c_addr_state);
#endif
		smartpakit_delay(node[j].delay, j, num - 1);
	}
}

static void kit_i2c_reset_irq_debounce_time(
	struct smartpakit_i2c_priv *i2c_priv)
{
	if (i2c_priv->reset_debounce_wait_time == 0)
		return;

	cancel_delayed_work_sync(&i2c_priv->irq_debounce_work);
	hwlog_info("%s: pa%d software reset debounce, trigger delayed_work\n",
		__func__, i2c_priv->chip_id);

	i2c_priv->irq_debounce_jiffies = jiffies +
		msecs_to_jiffies(i2c_priv->reset_debounce_wait_time);
	schedule_delayed_work(&i2c_priv->irq_debounce_work,
		msecs_to_jiffies(i2c_priv->reset_debounce_wait_time));
}

static int smartpakit_i2c_hw_reset(struct smartpakit_i2c_priv *i2c_priv)
{
	struct smartpakit_gpio_sequence *sequence = NULL;

	if (i2c_priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (i2c_priv->hw_reset == NULL)
		return 0;

	/* set hw_reset debounce */
	if (i2c_priv->reset_debounce_wait_time > 0) {
		/*
		 * if chip is not probed completed,
		 * no need cancel_delayed_work_sync
		 */
		if (smartpakit_i2c_probe_skip[i2c_priv->chip_id])
			cancel_delayed_work_sync(&i2c_priv->irq_debounce_work);

		hwlog_info("%s: pa%d reset debounce, trigger delayed_work\n",
			__func__, i2c_priv->chip_id);
		i2c_priv->irq_debounce_jiffies = jiffies +
			msecs_to_jiffies(i2c_priv->reset_debounce_wait_time);
		schedule_delayed_work(&i2c_priv->irq_debounce_work,
			msecs_to_jiffies(i2c_priv->reset_debounce_wait_time));
	}

	sequence = &i2c_priv->hw_reset->sequence;
	smartpakit_i2c_hw_reset_gpio_ctl(i2c_priv, sequence);
	return 0;
}

static int kit_i2c_dts_regs_read(struct smartpakit_i2c_priv *i2c_priv,
	struct smartpakit_reg_ctl *reg, struct list_head *dump_reg_list)
{
	struct smartpakit_reg_info reg_info;
	struct regmap *regmap = NULL;
	unsigned int reg_addr;
	unsigned int value = 0;
	int ret = 0;
	int ret_once;
	int j;

	regmap = i2c_priv->regmap_cfg->regmap;
	reg_addr = reg->addr;
	if (memset_s(&reg_info, sizeof(reg_info), 0, sizeof(reg_info)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);

	for (j = 0; j < (int)reg->value; j++) {
		ret_once = smartpakit_regmap_read(regmap, reg_addr, &value);
		smartpakit_i2c_dsm_report(i2c_priv, KIT_I2C_READ, ret_once,
			NULL);

		if (ret_once < 0) {
			ret += ret_once;
		} else {
			reg_info.chip_id = i2c_priv->chip_id;
			reg_info.reg_addr = reg_addr;
			reg_info.info = value;
			smartpakit_reg_info_add_list(&reg_info, dump_reg_list);
		}
		hwlog_info("%s: pa %u, r reg 0x%x = 0x%x\n", __func__,
			i2c_priv->chip_id, reg_addr, value);

		reg_addr++;
	}
	return ret;
}

static int smartpakit_i2c_dts_reg_ops(struct smartpakit_i2c_priv *i2c_priv,
	struct smartpakit_reg_ctl_sequence *sequence,
	struct list_head *dump_reg_list)
{
	unsigned int reg_addr;
	unsigned int ctl_value;
	unsigned int ctl_type;
	int ret = 0;
	int ret_once;
	int i;
	struct regmap *regmap = NULL;

	if (smartpakit_check_i2c_regmap_cfg(i2c_priv) < 0) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if ((sequence == NULL) || (sequence->num == 0) ||
		(sequence->regs == NULL))
		return 0;

	regmap = i2c_priv->regmap_cfg->regmap;

	for (i = 0; i < (int)sequence->num; i++) {
		reg_addr = sequence->regs[i].addr;
		ctl_value = sequence->regs[i].value;
		ctl_type = sequence->regs[i].ctl_type;

		if (ctl_type == SMARTPAKIT_REG_CTL_TYPE_W) {
			ret_once = smartpakit_regmap_write(regmap, reg_addr,
				ctl_value);
			smartpakit_i2c_dsm_report(i2c_priv, KIT_I2C_WRITE,
				ret_once, NULL);

			ret += ret_once;
			hwlog_info("%s: pa %u, w reg 0x%x 0x%x\n", __func__,
				i2c_priv->chip_id, reg_addr, ctl_value);
		} else if (ctl_type == SMARTPAKIT_REG_CTL_TYPE_DELAY) {
			if (ctl_value > 0) /* delay time units: msecs */
				msleep(ctl_value);
		} else { /* SMARTPAKIT_REG_CTL_TYPE_R */
			ret += kit_i2c_dts_regs_read(i2c_priv,
				&sequence->regs[i], dump_reg_list);
		}
	}

	return ret;
}

static int smartpakit_i2c_dump_regs(struct smartpakit_i2c_priv *i2c_priv)
{
	if (i2c_priv == NULL) {
		hwlog_err("%s: i2c_priv NULL\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: pa %u, dump regs\n", __func__, i2c_priv->chip_id);
	return smartpakit_i2c_dts_reg_ops(i2c_priv,
		i2c_priv->dump_regs_sequence, NULL);
}

int smartpakit_i2c_get_hismartpa_info(struct smartpakit_i2c_priv *i2c_priv,
	struct hismartpa_coeff *cfg)
{
	struct smartpakit_reg_info *pos = NULL;
	unsigned char reg_val[HISMARTPA_OTP_REG_NUM] = {0};
	struct list_head otp_req_list;
	int ret;
	int i = 0;

	if ((i2c_priv == NULL) || (cfg == NULL)) {
		hwlog_err("%s: invalid input argument\n", __func__);
		return -EINVAL;
	}

	if (i2c_priv->hismartpa_otp_sequence == NULL)
		return 0;

	if (sizeof(*cfg) != sizeof(reg_val))
		return -EINVAL;

	INIT_LIST_HEAD(&otp_req_list);

	hwlog_debug("%s: pa %u, operate hismartpa otp regs\n", __func__,
		i2c_priv->chip_id);
	ret = smartpakit_i2c_dts_reg_ops(i2c_priv,
		i2c_priv->hismartpa_otp_sequence, &otp_req_list);
	if (ret < 0) {
		hwlog_err("%s: read hismartpa_otp_regs err\n", __func__);
		return ret;
	}

	list_for_each_entry(pos, &otp_req_list, list) {
		if (i >= HISMARTPA_OTP_REG_NUM) {
			hwlog_err("%s: hismartpa_otp_regs num err\n", __func__);
			break;
		}
		reg_val[i] = (unsigned char)pos->info;
		i++;
	}

	if (memcpy_s(cfg, sizeof(*cfg), reg_val, sizeof(*cfg)) != EOK)
		hwlog_err("%s: memcpy_s is failed\n", __func__);

	hwlog_info("%s: pa%d, 0x%x, 0x%x, 0x%x, 0x%x\n", __func__,
		i2c_priv->chip_id, cfg->kv1, cfg->kv2, cfg->kr1, cfg->kr2);

	smartpakit_reg_info_del_list_all(&otp_req_list);
	return ret;
}
EXPORT_SYMBOL_GPL(smartpakit_i2c_get_hismartpa_info);

static int smartpakit_i2c_ctrl_read_regs(struct smartpakit_i2c_priv *i2c_priv,
	struct smartpakit_get_param *reg)
{
	int ret;

	if (smartpakit_check_i2c_regmap_cfg(i2c_priv) < 0) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	ret = smartpakit_regmap_read(i2c_priv->regmap_cfg->regmap, reg->index,
		&reg->value);
	smartpakit_i2c_dsm_report(i2c_priv, KIT_I2C_READ, ret, NULL);
	if (ret < 0)
		hwlog_debug("%s: regmap_read failed ret = %d\n", __func__, ret);

	return ret;
}

static bool smartpakit_is_need_write_regs(struct smartpakit_i2c_priv *i2c_priv,
	unsigned int num, struct smartpakit_param_node *regs)
{
	struct smartpakit_priv *pakit_priv = NULL;
	struct smartpakit_pa_ctl_sequence *old = NULL;
	unsigned int regs_size;

	pakit_priv = smartpakit_get_misc_priv();
	if (pakit_priv == NULL)
		return false;

	if (pakit_priv->force_refresh_chip)
		return true;

	old = &pakit_priv->poweron_seq[i2c_priv->chip_id];
	regs_size = sizeof(*regs) * num;

	if ((old->node != NULL) && (old->param_num == num) &&
		(memcmp(old->node, regs, regs_size) == 0))
		return false;

	return true;
}

static int aw_smartpa_get_trim_value(struct regmap *regmap,
	unsigned int reg_addr, int *trim)
{
	int ret;
	unsigned int reg_val = 0;
	unsigned short val;

	ret = smartpakit_regmap_read(regmap, reg_addr, &reg_val);
	if (ret < 0) {
		hwlog_err("%s: read reg 0x%x failed\n", __func__, reg_addr);
		return ret;
	}

	hwlog_debug("%s: read reg 0x%x = 0x%x\n", __func__, reg_addr, reg_val);

	/* calk value is ten bits, the highest bit is sign */
	val = (unsigned short)reg_val & AW_TRIM_VALUE_MASK;
	val = aw_fill_short_type_sign(val);

	*trim = AW_TRIM_VALUE_BASE + (short)val;
	return ret;
}

static int aw_smartpa_set_correction(struct regmap *regmap,
	unsigned int reg_addr, unsigned int isn_addr, unsigned int vsn_addr)
{
	int ret;
	int i_trim = 0;
	int v_trim = 0;
	int reg_val;

	ret = aw_smartpa_get_trim_value(regmap, isn_addr, &i_trim);
	ret += aw_smartpa_get_trim_value(regmap, vsn_addr, &v_trim);
	if (ret < 0)
		return ret;

	hwlog_info("%s: i_trim 0x%x, v_trim 0x%x\n", __func__, i_trim, v_trim);

	/* get vcal from i_trim&v_trim */
	reg_val = AW_CORRECT_COEFF * i_trim / v_trim;

	hwlog_info("%s: write reg 0x%x value 0x%x\n", __func__, reg_addr,
		reg_val);

	ret = smartpakit_regmap_write(regmap, reg_addr, (unsigned int)reg_val);
	if (ret < 0)
		hwlog_err("%s: write reg 0x%x failed\n", __func__, reg_addr);

	return ret;
}

static int smartpakit_i2c_reg_node_ops(struct smartpakit_regmap_cfg *cfg,
	struct smartpakit_param_node *reg, int index, unsigned int node_num)
{
	int ret = 0;
	int value = 0;

	/*
	 * reg node type
	 * 1. write reg node
	 * 3. delay time node
	 * 4. skip node
	 * 5. rxorw node
	 * 6. read reg node
	 * 7. irq filter config node
	 * 8. aw smartpa set correction
	 */
	switch (reg->node_type) {
	case SMARTPAKIT_PARAM_NODE_TYPE_DELAY:
		if (reg->delay > 0)
			msleep(reg->delay);
		break;
	case SMARTPAKIT_PARAM_NODE_TYPE_SKIP:
	case SMARTPAKIT_PARAM_NODE_TYPE_IRQ_FILTER:
		break;
	case SMARTPAKIT_PARAM_NODE_TYPE_REG_RXORW:
		hwlog_info("%s: rworw node %d/%u\n", __func__, index, node_num);
		ret = smartpakit_regmap_xorw(cfg->regmap, reg->index,
			reg->mask, reg->value);
		break;
	case SMARTPAKIT_PARAM_NODE_TYPE_REG_READ:
		ret = smartpakit_regmap_read(cfg->regmap, reg->index, &value);
		hwlog_info("%s: read node %d/%u, reg 0x%x = 0x%x, ret = %d\n",
			__func__, index, node_num, reg->index, value, ret);
		break;
	case SMARTPAKIT_PARAM_NODE_TYPE_AW_CORRECT:
		ret = aw_smartpa_set_correction(cfg->regmap, reg->index,
			reg->mask, reg->value);
		break;
	default: /* SMARTPAKIT_PARAM_NODE_TYPE_REG_WRITE 1 */
		ret = smartpakit_regmap_complex_write(cfg, reg->index,
			reg->mask, reg->value);
		smartpakit_delay(reg->delay, index, (int)node_num - 1);
		break;
	}
	return ret;
}

static void smartpakit_i2c_get_i2c_err_info(struct i2c_err_info *info,
	unsigned int index)
{
	if (index < I2C_STATUS_B64)
		info->err_details |= ((unsigned long int)1 << index);

	info->err_count++;
}

static int smartpakit_i2c_ctrl_write_regs(struct smartpakit_i2c_priv *i2c_priv,
	unsigned int num, struct smartpakit_param_node *regs)
{
	struct smartpakit_priv *pakit_priv = NULL;
	struct smartpakit_regmap_cfg *cfg = NULL;
	int ret = 0;
	int i;
	int errno = 0;
	struct i2c_err_info info = {
		.regs_num    = 0,
		.err_count   = 0,
		.err_details = 0,
	};

	if (smartpakit_check_i2c_regmap_cfg(i2c_priv) < 0) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: chip model %s, chip_id %u, node num %u\n", __func__,
		i2c_priv->chip_model, i2c_priv->chip_id, num);
#endif

	if ((num == 0) || (regs == NULL)) {
		hwlog_err("%s: reg node is invalid\n", __func__);
		return -EINVAL;
	}

	if (!smartpakit_is_need_write_regs(i2c_priv, num, regs)) {
		hwlog_info("%s: pa%d not need re-write the same regs\n",
			__func__, i2c_priv->chip_id);
		return 0;
	}
	pakit_priv = smartpakit_get_misc_priv();
	if (pakit_priv == NULL)
		return false;

	cfg = i2c_priv->regmap_cfg;
	/* for smartpa, node type has gpio node and reg node */
	for (i = 0; i < (int)num; i++) {
		/* gpio node */
		if (regs[i].node_type == SMARTPAKIT_PARAM_NODE_TYPE_GPIO) {
			ret = smartpakit_gpio_node_ctl(i2c_priv->priv_data,
				&regs[i]);
			if (ret == 0)
				continue;
			hwlog_err("%s: node %d, set gpio%u %u err\n", __func__,
				i, regs[i].index, regs[i].value);
			return ret;
		} else if (regs[i].node_type ==       /* reset irq debounce */
			SMARTPAKIT_PARAM_NODE_TYPE_IRQ_DEBOUNCE) {
			kit_i2c_reset_irq_debounce_time(i2c_priv);
			continue;
		}
		/* regmap node */
		ret = smartpakit_i2c_reg_node_ops(cfg, &regs[i], i, num);
		if (ret < 0) {
			hwlog_err("%s: node %d, reg 0x%x w/r %x err, ret %d\n",
				__func__, i, regs[i].index, regs[i].value, ret);
			smartpakit_i2c_get_i2c_err_info(&info, (unsigned int)i);
			errno = ret;
			pakit_priv->i2c_errno = errno;
		}
	}
	info.regs_num = num;
	smartpakit_i2c_dsm_report(i2c_priv, KIT_I2C_WRITE, errno, &info);
	return ret;
}

static int smartpakit_i2c_ctrl_read(struct i2c_client *i2c, char *rx_data,
	int length)
{
	int ret;
	struct i2c_msg msg[] = {
		{
			.addr  = i2c->addr,
			.flags = I2C_M_RD,
			.len   = (unsigned short)length,
			.buf   = (unsigned char *)rx_data,
		},
	};

	ret = i2c_transfer(i2c->adapter, msg, 1);
	if (ret < 0) {
		smartpakit_dsm_report_by_i2c_error("none",
			SMARTPAKIT_INVALID_PA_INDEX, KIT_I2C_READ, ret, NULL);
		hwlog_err("%s: transfer error %d\n", __func__, ret);
	}

	return ret;
}

static int smartpakit_i2c_ctrl_write(struct i2c_client *i2c,
	char *rx_data, int length)
{
	int ret;
	struct i2c_msg msg[] = {
		{
			.addr  = i2c->addr,
			.flags = 0,
			.len   = (unsigned short)length,
			.buf   = (unsigned char *)rx_data,
		},
	};

	ret = i2c_transfer(i2c->adapter, msg, 1);
	if (ret < 0) {
		smartpakit_dsm_report_by_i2c_error("none",
			SMARTPAKIT_INVALID_PA_INDEX, KIT_I2C_WRITE, ret, NULL);
		hwlog_err("%s: transfer error %d\n", __func__, ret);
	}

	return ret;
}

/* these interface is applied to misc for pa ctl */
struct smartpakit_i2c_ctl_ops i2c_ctl_ops = {
	.hw_reset   = smartpakit_i2c_hw_reset,
	.dump_regs  = smartpakit_i2c_dump_regs,
	.read_regs  = smartpakit_i2c_ctrl_read_regs,
	.write_regs = smartpakit_i2c_ctrl_write_regs,
	.i2c_read   = smartpakit_i2c_ctrl_read,
	.i2c_write  = smartpakit_i2c_ctrl_write,
};

static void smartpakit_sync_irq_debounce_time(
	struct smartpakit_i2c_priv *i2c_priv)
{
	struct smartpakit_priv *pakit_priv = NULL;
	struct smartpakit_i2c_priv *i2c_priv_p = NULL;
	int i;
	unsigned int wait_time;
	unsigned long wait_jiffies;

	if ((i2c_priv == NULL) || (i2c_priv->probe_completed == 0) ||
		(!i2c_priv->sync_irq_debounce_time))
		return;

	pakit_priv = i2c_priv->priv_data;
	if (pakit_priv == NULL)
		return;

	for (i = 0; i < (int)pakit_priv->pa_num; i++) {
		i2c_priv_p = pakit_priv->i2c_priv[i];
		if (i2c_priv_p == NULL) {
			hwlog_err("%s: i2c_priv %d is NULL\n", __func__, i);
			break;
		}
		wait_time = i2c_priv_p->reset_debounce_wait_time;
		if ((i2c_priv_p == i2c_priv) || (wait_time == 0))
			continue;
		/* reset_debounce_wait_time must > 0 */
		cancel_delayed_work_sync(&i2c_priv_p->irq_debounce_work);

		wait_jiffies = msecs_to_jiffies(wait_time);
		i2c_priv_p->irq_debounce_jiffies = jiffies + wait_jiffies;
		schedule_delayed_work(&i2c_priv_p->irq_debounce_work,
			wait_jiffies);
	}
	hwlog_info("%s: pa%u irq trigger, sync debounce time\n", __func__,
		i2c_priv->chip_id);
}

static bool smartpakit_is_rw_sequence_exited(
	struct smartpakit_i2c_priv *i2c_priv)
{
	if ((i2c_priv == NULL) ||
		(i2c_priv->irq_handler == NULL) ||
		(i2c_priv->irq_handler->rw_sequence == NULL))
		return false;

	return true;
}

static bool is_irq_filter_config_hited(
	struct list_head *dump_reg_list, struct smartpakit_reg_info *cfg_info)
{
	struct smartpakit_reg_info *read_reg_info = NULL;
	unsigned int read_val;

	if ((!dump_reg_list) || (!cfg_info)) {
		hwlog_err("%s: Input argument invalid, not reset\n", __func__);
		return false;
	}

	list_for_each_entry(read_reg_info, dump_reg_list, list) {
		if ((read_reg_info->chip_id == cfg_info->chip_id) &&
			(read_reg_info->reg_addr == cfg_info->reg_addr)) {
			read_val = read_reg_info->info & cfg_info->mask;
			if (read_val) {
				hwlog_info("%s:pa%d,reg:0x%x,info:0x%x,mask:0x%x\n",
					__func__, read_reg_info->chip_id,
					read_reg_info->reg_addr,
					read_reg_info->info, cfg_info->mask);
				hwlog_info("%s:Filter trigger reset\n",
					__func__);
				return true;
			}
		}
	}

	return false;
}

static bool smartpakit_i2c_is_trigger_reset(
	struct smartpakit_i2c_priv *i2c_priv, struct list_head *dump_reg_list)
{
	struct smartpakit_reg_info *reg_info = NULL;
	struct smartpakit_priv *pakit_priv = NULL;

	if (!i2c_priv->irq_handler->need_filter_irq)
		return true;

	pakit_priv = i2c_priv->priv_data;
	if (pakit_priv == NULL)
		return false;

	/* find need reset config by given pa */
	list_for_each_entry(reg_info, &pakit_priv->irq_filter_list, list) {
		if (reg_info->chip_id != i2c_priv->chip_id)
			continue;
		if (is_irq_filter_config_hited(dump_reg_list, reg_info))
			return true;
	}

	return false;
}

static bool smartpakit_is_need_reset(struct smartpakit_i2c_priv *i2c_priv,
	struct list_head *dump_reg_list)
{
	struct smartpakit_priv *pakit_priv = NULL;
	bool check = false;
	int i;

	if (!smartpakit_i2c_is_trigger_reset(i2c_priv, dump_reg_list))
		return false;

	/*
	 * As long as one chip configed need reset in dts,
	 * all chips need reset
	 */
	pakit_priv = i2c_priv->priv_data;
	for (i = 0; i < (int)pakit_priv->pa_num; i++) {
		if ((pakit_priv->i2c_priv[i] == NULL) ||
			(pakit_priv->i2c_priv[i]->irq_handler == NULL)) {
			hwlog_err("%s: chip %d, check failed\n", __func__, i);
			break;
		}

		if (pakit_priv->i2c_priv[i]->irq_handler->need_reset) {
			check = true;
			break;
		}
	}
	return check;
}

static int smartpakit_rw_sequence_dump(struct smartpakit_priv *pakit_priv,
	struct list_head *dump_reg_list)
{
	struct smartpakit_i2c_priv *i2c_priv_p = NULL;
	struct smartpakit_reg_ctl_sequence *rw_sequence = NULL;
	int ret = 0;
	int i;

	for (i = 0; i < (int)pakit_priv->pa_num; i++) {
		i2c_priv_p = pakit_priv->i2c_priv[i];
		if (!smartpakit_is_rw_sequence_exited(i2c_priv_p)) {
			hwlog_info("%s: pa %d no rw_sequence\n", __func__, i);
			continue;
		}

		hwlog_info("%s: pa %u, rw_sequence ctl\n", __func__,
			i2c_priv_p->chip_id);
		rw_sequence = i2c_priv_p->irq_handler->rw_sequence;
		ret = smartpakit_i2c_dts_reg_ops(i2c_priv_p, rw_sequence,
			dump_reg_list);
		if (ret < 0) {
			hwlog_err("%s: pa %u, rw_sequence reg ctl failed\n",
				__func__, i2c_priv_p->chip_id);
			break;
		}
	}
	return ret;
}

#ifdef CONFIG_HUAWEI_DSM_AUDIO
static void smartpakit_i2c_handler_irq_dsm_report(
	struct smartpakit_i2c_priv *i2c_priv, struct list_head *dump_reg_list,
	bool need_reset)
{
	struct smartpakit_priv *pakit_priv = NULL;
	struct smartpakit_reg_info *reg_info = NULL;
	struct smartpakit_i2c_priv *i2c_priv_p = NULL;
	char *report = NULL;
	int i;

	pakit_priv = i2c_priv->priv_data;

	report = kzalloc(sizeof(char) * DSM_BUF_SIZE, GFP_KERNEL);
	if (report == NULL)
		return;
	if (need_reset)
		smartpakit_append_dsm_report(report, "RST,");

	for (i = 0; i < (int)pakit_priv->pa_num; i++) {
		i2c_priv_p = pakit_priv->i2c_priv[i];
		if (i2c_priv_p == NULL)
			continue;

		/* current pa, irq trigger from this pa */
		if (i2c_priv_p->chip_id == i2c_priv->chip_id)
			smartpakit_append_dsm_report(report, "*");

		smartpakit_append_dsm_report(report, "pa %s_%d:",
			i2c_priv_p->chip_model, i2c_priv_p->chip_id);

#ifdef CONFIG_HUAWEI_DEVICEBOX_AUDIO
		/*
		 * int boxid_read(int out_id)
		 * out_id: from 0 ~ 3
		 */
		smartpakit_append_dsm_report(report, "boxid=%d,",
			boxid_read(i2c_priv_p->chip_id));
#endif

		list_for_each_entry(reg_info, dump_reg_list, list) {
			if (i2c_priv_p->chip_id != reg_info->chip_id)
				continue;
			smartpakit_append_dsm_report(report, "reg 0x%x=0x%x,",
				reg_info->reg_addr, reg_info->info);
		}
	}

	/* dsm report */
	hwlog_info("%s: dsm report, %s\n", __func__, report);
	audio_dsm_report_info(AUDIO_SMARTPA, DSM_SMARTPA_INT_ERR,
		"smartpakit,%s", report);

	kfree(report);
}
#endif

#ifdef CONFIG_HUAWEI_DSM_AUDIO
bool record_i2c_irq_time(void)
{
	struct timeval tv;
	static int time_num = 0;
	static long time_buf[TIME_COUNT_FOR_DMD] = {0};
	int i;

	do_gettimeofday(&tv);

	if (time_num >= TIME_COUNT_FOR_DMD)
		time_num = 0;

	hwlog_info("%s tv.tv_sec %ld time_buf %ld time_num %d", __func__, tv.tv_sec,
		time_buf[time_num], time_num);
	if ((time_buf[time_num] != 0) && (tv.tv_sec - time_buf[time_num] < TIME_DIFF)) {
		for (i = 0; i < TIME_COUNT_FOR_DMD; i++) {
			time_buf[i] = 0;
		}
		hwlog_info("%s dmd report %d", __func__, time_num);
		return true;
	}
	time_buf[time_num] = tv.tv_sec;
	time_num++;
	return false;
}
#endif

void smartpakit_i2c_handler_irq(struct work_struct *work)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;
	struct smartpakit_priv *pakit_priv = NULL;
	bool need_reset = false;
	struct list_head dump_regs_list;

	i2c_priv = container_of(work, struct smartpakit_i2c_priv,
		irq_handle_work);

	hwlog_info("%s: enter, chip %u\n", __func__, i2c_priv->chip_id);
	if ((i2c_priv->priv_data == NULL) || (i2c_priv->irq_handler == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return;
	}
	pakit_priv = i2c_priv->priv_data;

	mutex_lock(&pakit_priv->irq_handler_lock);
	mutex_lock(&pakit_priv->hw_reset_lock);
	mutex_lock(&pakit_priv->dump_regs_lock);
	mutex_lock(&pakit_priv->i2c_ops_lock);

	INIT_LIST_HEAD(&dump_regs_list);

	if (smartpakit_rw_sequence_dump(pakit_priv, &dump_regs_list) < 0)
		goto err_out;

	need_reset = smartpakit_is_need_reset(i2c_priv, &dump_regs_list);
#ifdef CONFIG_HUAWEI_DSM_AUDIO
	if (record_i2c_irq_time())
		smartpakit_i2c_handler_irq_dsm_report(i2c_priv, &dump_regs_list,
			need_reset);
#endif
	/* dump regs */
	smartpakit_dump_chips(pakit_priv);

	if (need_reset) {
		/* reset chip by hw_reset */
		smartpakit_hw_reset_chips(pakit_priv);

		/* init and resume power */
		smartpakit_resume_chips(pakit_priv);
	} else {
		hwlog_info("%s: not need reset_chips, skip\n", __func__);
	}

err_out:
	smartpakit_reg_info_del_list_all(&dump_regs_list);
	mutex_unlock(&pakit_priv->i2c_ops_lock);
	mutex_unlock(&pakit_priv->dump_regs_lock);
	mutex_unlock(&pakit_priv->hw_reset_lock);
	mutex_unlock(&pakit_priv->irq_handler_lock);
	hwlog_info("%s: enter end\n", __func__);
}

irqreturn_t smartpakit_i2c_thread_irq(int irq, void *data)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;

	hwlog_info("%s: enter, irq = %d\n", __func__, irq);

	i2c_priv = data;
	smartpakit_sync_irq_debounce_time(i2c_priv);
	if (i2c_priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return IRQ_HANDLED;
	}

	if (i2c_priv->irq_debounce_jiffies > 0) {
		if (time_is_after_jiffies(i2c_priv->irq_debounce_jiffies)) {
			hwlog_info("%s: debounce wait, skip this irq\n",
				__func__);
			return IRQ_HANDLED;
		}
	}

	if (i2c_priv->probe_completed == 0) {
		hwlog_err("%s: probe not completed, skip\n", __func__);
		return IRQ_HANDLED;
	}

	if (!work_busy(&i2c_priv->irq_handle_work)) {
		hwlog_info("%s: schedule_work\n", __func__);
		schedule_work(&i2c_priv->irq_handle_work);
	} else {
		hwlog_info("%s: work busy, skip\n", __func__);
	}

	hwlog_info("%s: enter end\n", __func__);
	return IRQ_HANDLED;
}
EXPORT_SYMBOL_GPL(smartpakit_i2c_thread_irq);

static void smartpakit_i2c_irq_debounce_work(struct work_struct *work)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;

	hwlog_info("%s: reset debounce, delayed_work enter\n", __func__);
	if (work == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return;
	}

	i2c_priv = container_of(work, struct smartpakit_i2c_priv,
		irq_debounce_work.work);
	i2c_priv->irq_debounce_jiffies = 0;
	hwlog_info("%s: pa%d reset debounce, delayed_work stop\n", __func__,
		i2c_priv->chip_id);
}

static bool smartpakit_i2c_is_reg_in_special_range(unsigned int reg_addr,
	int num, unsigned int *reg)
{
	int i;

	if ((num == 0) || (reg == NULL)) {
		hwlog_err("%s: invalid arg\n", __func__);
		return false;
	}

	for (i = 0; i < num; i++) {
		if (reg[i] == reg_addr)
			return true;
	}
	return false;
}

static struct smartpakit_regmap_cfg *smartpakit_i2c_get_regmap_cfg(
	struct device *dev)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;

	if (dev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return NULL;
	}
	i2c_priv = dev_get_drvdata(dev);
	if ((i2c_priv == NULL) || (i2c_priv->regmap_cfg == NULL)) {
		hwlog_err("%s: regmap_cfg invalid argument\n", __func__);
		return NULL;
	}
	return i2c_priv->regmap_cfg;
}

static bool smartpakit_i2c_writeable_reg(struct device *dev, unsigned int reg)
{
	struct smartpakit_regmap_cfg *cfg = NULL;

	cfg = smartpakit_i2c_get_regmap_cfg(dev);
	if (cfg == NULL)
		return false;
	/* config writable or unwritable, can not config in dts at same time */
	if (cfg->num_writeable > 0)
		return smartpakit_i2c_is_reg_in_special_range(reg,
			cfg->num_writeable, cfg->reg_writeable);
	if (cfg->num_unwriteable > 0)
		return !smartpakit_i2c_is_reg_in_special_range(reg,
			cfg->num_unwriteable, cfg->reg_unwriteable);

	return true;
}

static bool smartpakit_i2c_readable_reg(struct device *dev, unsigned int reg)
{
	struct smartpakit_regmap_cfg *cfg = NULL;

	cfg = smartpakit_i2c_get_regmap_cfg(dev);
	if (cfg == NULL)
		return false;
	/* config readable or unreadable, can not config in dts at same time */
	if (cfg->num_readable > 0)
		return smartpakit_i2c_is_reg_in_special_range(reg,
			cfg->num_readable, cfg->reg_readable);
	if (cfg->num_unreadable > 0)
		return !smartpakit_i2c_is_reg_in_special_range(reg,
			cfg->num_unreadable, cfg->reg_unreadable);

	return true;
}

static bool smartpakit_i2c_volatile_reg(struct device *dev, unsigned int reg)
{
	struct smartpakit_regmap_cfg *cfg = NULL;

	cfg = smartpakit_i2c_get_regmap_cfg(dev);
	if (cfg == NULL)
		return false;
	/* config volatile or unvolatile, can not config in dts at same time */
	if (cfg->num_volatile > 0)
		return smartpakit_i2c_is_reg_in_special_range(reg,
			cfg->num_volatile, cfg->reg_volatile);
	if (cfg->num_unvolatile > 0)
		return !smartpakit_i2c_is_reg_in_special_range(reg,
			cfg->num_unvolatile, cfg->reg_unvolatile);

	return true;
}

static void smartpakit_i2c_free_hw_reset(
	struct smartpakit_gpio_reset *hw_reset)
{
	if (hw_reset == NULL)
		return;
	smartpakit_kfree_ops(hw_reset->sequence.node);
	smartpakit_kfree_ops(hw_reset);
}

static int smartpakit_i2c_parse_hw_reset_gpio_ctl_seq(struct device_node *node,
	struct smartpakit_gpio_sequence *sequence)
{
	const char *ctl_sequence_str = "ctl_sequence";
	struct smartpakit_gpio_state *gpio_node = NULL;
	int count = 0;
	int val_num;
	int ret;
	unsigned int node_num;

	ret = smartpakit_get_prop_of_u32_array(node, ctl_sequence_str,
		(u32 **)&gpio_node, &count);
	if ((count <= 0) || (ret < 0)) {
		hwlog_err("%s: get gpio_state sequence err\n", __func__);
		return -EFAULT;
	}

	val_num = sizeof(struct smartpakit_gpio_state) / sizeof(unsigned int);
	if ((count % val_num) != 0) {
		hwlog_err("%s: count %d%%%d != 0\n", __func__, count, val_num);
		goto err_out;
	}

	node_num = (unsigned int)(count / val_num);
	sequence->num = node_num;
	sequence->node = gpio_node;
	return 0;

err_out:
	smartpakit_kfree_ops(gpio_node);
	return -EFAULT;
}

static int smartpakit_i2c_parse_hw_reset_info(
	struct smartpakit_i2c_priv *i2c_priv, struct device_node *node,
	struct smartpakit_gpio_reset **reset)
{
	const char *gpio_reset_str        = "gpio_reset";
	const char *not_need_shutdown_str = "not_need_shutdown";
	struct smartpakit_gpio_reset *reset_info = NULL;
	int ret;

	reset_info = kzalloc(sizeof(*reset_info), GFP_KERNEL);
	if (reset_info == NULL)
		return -ENOMEM;

	reset_info->not_need_shutdown = of_property_read_bool(node,
		not_need_shutdown_str);
	hwlog_debug("%s: not_need_shutdown = %d\n", __func__,
		(int)reset_info->not_need_shutdown);

	reset_info->gpio = of_get_named_gpio(node, gpio_reset_str, 0);
	if (reset_info->gpio < 0) {
		hwlog_debug("%s: get_named_gpio failed, %d\n", __func__,
			reset_info->gpio);
		ret = of_property_read_u32(node, gpio_reset_str,
			(u32 *)&reset_info->gpio);
		if (ret < 0) {
			hwlog_err("%s: of_property_read_u32 gpio failed, %d\n",
				__func__, ret);
			ret = -EFAULT;
			goto err_out;
		}
	}

	ret = snprintf_s(reset_info->gpio_name,
		(unsigned long)SMARTPAKIT_NAME_MAX,
		(unsigned long)SMARTPAKIT_NAME_MAX - 1, "%s_gpio_reset_%d",
		i2c_priv->chip_model, i2c_priv->chip_id);
	if (ret < 0) {
		hwlog_err("%s: set gpio_name failed\n", __func__);
		goto err_out;
	}

	ret = smartpakit_i2c_parse_hw_reset_gpio_ctl_seq(node,
		&reset_info->sequence);
	if (ret < 0)
		goto err_out;

	*reset = reset_info;
	return 0;
err_out:
	smartpakit_i2c_free_hw_reset(reset_info);
	return ret;
}

static int smartpakit_i2c_request_reset_gpio(
	struct smartpakit_i2c_priv *i2c_priv)
{
	struct smartpakit_gpio_reset *reset = NULL;

	reset = i2c_priv->hw_reset;
	if (reset == NULL) {
		hwlog_info("%s: hw_reset not configed\n", __func__);
		return 0;
	}

	if (smartpakit_i2c_is_request_same_reset_gpio(reset->gpio)) {
		hwlog_info("%s: gpio %d had been requested, skip\n", __func__,
			reset->gpio);
		return 0;
	}

	if (gpio_request((unsigned int)reset->gpio, reset->gpio_name) < 0) {
		hwlog_err("%s:gpio%d request failed\n", __func__, reset->gpio);
		reset->gpio = 0;
		return -EFAULT;
	}
	return 0;
}

static int smartpakit_i2c_parse_dt_reset(struct i2c_client *i2c,
	struct smartpakit_i2c_priv *i2c_priv)
{
	const char *hw_reset_str = "hw_reset";
	struct device_node *node = NULL;
	int ret;

	node = of_get_child_by_name(i2c->dev.of_node, hw_reset_str);
	if (node == NULL) {
		hwlog_debug("%s: hw_reset node not existed, skip\n", __func__);
		return 0;
	}
	ret = smartpakit_i2c_parse_hw_reset_info(i2c_priv, node,
		&i2c_priv->hw_reset);
	if (ret < 0)
		hwlog_err("%s: parse reset node err\n", __func__);

	return ret;
}

static void smartpakit_i2c_free_reg_ctl(
	struct smartpakit_reg_ctl_sequence *reg_ctl)
{
	if (reg_ctl == NULL)
		return;

	smartpakit_kfree_ops(reg_ctl->regs);
	kfree(reg_ctl);
	reg_ctl = NULL;
}

static int smartpakit_i2c_parse_reg_ctl(
	struct smartpakit_reg_ctl_sequence **reg_ctl, struct device_node *node,
	const char *ctl_str)
{
	struct smartpakit_reg_ctl_sequence *ctl = NULL;
	int count = 0;
	int val_num;
	int ret;

	if ((node == NULL) || (ctl_str == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	ctl = kzalloc(sizeof(*ctl), GFP_KERNEL);
	if (ctl == NULL)
		return -ENOMEM;

	ret = smartpakit_get_prop_of_u32_array(node, ctl_str,
		(u32 **)&ctl->regs, &count);
	if ((count <= 0) || (ret < 0)) {
		hwlog_err("%s: get %s failed or count is 0\n",
			__func__, ctl_str);
		ret = -EFAULT;
		goto err_out;
	}

	val_num = sizeof(struct smartpakit_reg_ctl) / sizeof(unsigned int);
	if ((count % val_num) != 0) {
		hwlog_err("%s: count %d %% val_num %d != 0\n",
			__func__, count, val_num);
		ret = -EFAULT;
		goto err_out;
	}

	ctl->num = (unsigned int)(count / val_num);
	*reg_ctl = ctl;
	return 0;

err_out:
	smartpakit_i2c_free_reg_ctl(ctl);
	return ret;
}

static void smartpakit_i2c_free_irq_handler(
	struct smartpakit_gpio_irq *irq_handler)
{
	if (irq_handler == NULL)
		return;

	smartpakit_i2c_free_reg_ctl(irq_handler->rw_sequence);

	kfree(irq_handler);
	irq_handler = NULL;
}

static int smartpakit_i2c_parse_irq_handler_info(
	struct smartpakit_i2c_priv *i2c_priv, struct device_node *node,
	struct smartpakit_gpio_irq **irq)
{
	const char *gpio_irq_str        = "gpio_irq";
	const char *irq_flags_str       = "irq_flags";
	const char *need_reset_str      = "need_reset";
	const char *need_filter_irq_str = "need_filter_irq";
	const char *rw_sequence_str     = "rw_sequence";
	struct smartpakit_gpio_irq *irq_info = NULL;
	int ret;

	irq_info = kzalloc(sizeof(*irq_info), GFP_KERNEL);
	if (irq_info == NULL)
		return -ENOMEM;

	irq_info->gpio = of_get_named_gpio(node, gpio_irq_str, 0);
	if (irq_info->gpio < 0) {
		hwlog_debug("%s: of_get_named_gpio failed, %d\n", __func__,
			irq_info->gpio);
		ret = of_property_read_u32(node, gpio_irq_str,
			(u32 *)&irq_info->gpio);
		if (ret < 0) {
			hwlog_err("%s: read gpio_irq failed, %d\n",
				__func__, ret);
			ret = -EFAULT;
			goto err_out;
		}
	}

	ret = of_property_read_u32(node, irq_flags_str, &irq_info->irqflags);
	if (ret < 0) {
		hwlog_err("%s: irq_handler get irq_flags failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}
	irq_info->need_reset = of_property_read_bool(node, need_reset_str);
	irq_info->need_filter_irq = of_property_read_bool(node,
		need_filter_irq_str);
#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: need_reset = %d, need_filter_irq = %d\n", __func__,
		(int)irq_info->need_reset, (int)irq_info->need_filter_irq);
#endif

	ret = snprintf_s(irq_info->gpio_name, (unsigned long)SMARTPAKIT_NAME_MAX,
		(unsigned long)SMARTPAKIT_NAME_MAX - 1, "%s_gpio_irq_%d",
		i2c_priv->chip_model, i2c_priv->chip_id);
	if (ret < 0)
		hwlog_err("%s: set gpio_name failed\n", __func__);
	ret += snprintf_s(irq_info->irq_name, (unsigned long)SMARTPAKIT_NAME_MAX,
		(unsigned long)SMARTPAKIT_NAME_MAX - 1, "%s_irq_%d",
		i2c_priv->chip_model, i2c_priv->chip_id);
	if (ret < 0) {
		hwlog_err("%s: set gpio_name/irq_name failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}

	/* rw_sequence configed in dts */
	if (of_property_read_bool(node, rw_sequence_str)) {
		ret = smartpakit_i2c_parse_reg_ctl(&irq_info->rw_sequence,
			node, rw_sequence_str);
		if (ret < 0) {
			hwlog_err("%s: parse rw_sequence failed\n", __func__);
			goto err_out;
		}
	} else {
		hwlog_debug("%s: rw_sequence not existed, skip\n", __func__);
	}
	*irq = irq_info;
	return 0;
err_out:
	smartpakit_i2c_free_irq_handler(irq_info);
	return ret;
}

static int smartpakit_i2c_request_threaded_irq(struct i2c_client *i2c,
	struct smartpakit_i2c_priv *i2c_priv)
{
	struct smartpakit_gpio_irq *irq_handler = NULL;
	int gpio;
	int ret;

	irq_handler = i2c_priv->irq_handler;
	if (irq_handler == NULL) {
		hwlog_info("%s: irq_handler not configed\n", __func__);
		return 0;
	}

	gpio = irq_handler->gpio;
	if (!gpio_is_valid(gpio)) {
		hwlog_err("%s: irq_handler gpio %d invalid\n", __func__, gpio);
		return -EFAULT;
	}

	if (smartpakit_i2c_is_request_same_irq_gpio(gpio)) {
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: irq gpio %d had been requested, skip\n",
			__func__, gpio);
#endif
		return 0;
	}

	ret = devm_gpio_request_one(&i2c->dev, (unsigned int)gpio,
		(unsigned long)GPIOF_DIR_IN, irq_handler->gpio_name);
	if (ret < 0) {
		hwlog_err("%s: gpio set GPIOF_DIR_IN failed\n", __func__);
		return -EFAULT;
	}

	irq_handler->irq = gpio_to_irq((unsigned int)gpio);
#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: irq_handler get irq %d, irqflags=%u\n", __func__,
		irq_handler->irq, irq_handler->irqflags);
#endif
	ret = devm_request_threaded_irq(&i2c->dev,
		(unsigned int)irq_handler->irq, NULL,
		smartpakit_i2c_thread_irq,
		(unsigned long)(irq_handler->irqflags | IRQF_ONESHOT),
		irq_handler->irq_name, (void *)i2c_priv);
	if (ret < 0) {
		hwlog_err("%s: devm_request_threaded_irq failed\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static int smartpakit_i2c_parse_dt_irq(struct i2c_client *i2c,
	struct smartpakit_i2c_priv *i2c_priv)
{
	const char *irq_handler_str = "irq_handler";
	struct device_node *node = NULL;
	int ret;

	node = of_get_child_by_name(i2c->dev.of_node, irq_handler_str);
	if (node == NULL) {
		hwlog_debug("%s: irq_handler not existed, skip\n", __func__);
		return 0;
	}
	ret = smartpakit_i2c_parse_irq_handler_info(i2c_priv, node,
		&i2c_priv->irq_handler);

	return ret;
}

static int smartpakit_i2c_parse_dt_version_regs(struct i2c_client *i2c,
	struct smartpakit_i2c_priv *i2c_priv)
{
	const char *version_regs_str = "version_regs";
	int ret;

	if (!of_property_read_bool(i2c->dev.of_node, version_regs_str)) {
		hwlog_debug("%s: version_regs not existed, skip\n", __func__);
		return 0;
	}
	ret = smartpakit_i2c_parse_reg_ctl(&i2c_priv->version_regs_seq,
		i2c->dev.of_node, version_regs_str);
	if (ret < 0)
		hwlog_err("%s: parse version_regs failed\n", __func__);

	return ret;
}

static int smartpakit_i2c_parse_dt_dump_regs(struct i2c_client *i2c,
	struct smartpakit_i2c_priv *i2c_priv)
{
	const char *dump_regs_str = "dump_regs";
	int ret;

	if (!of_property_read_bool(i2c->dev.of_node, dump_regs_str)) {
		hwlog_debug("%s: dump_regs not existed, skip\n", __func__);
		return 0;
	}
	ret = smartpakit_i2c_parse_reg_ctl(&i2c_priv->dump_regs_sequence,
		i2c->dev.of_node, dump_regs_str);
	if (ret < 0)
		hwlog_err("%s: parse dump_regs failed\n", __func__);

	return ret;
}

static int smartpakit_i2c_parse_dt_hismartpa(struct i2c_client *i2c,
	struct smartpakit_i2c_priv *i2c_priv)
{
	const char *hismartpa_otp_regs_str = "hismartpa_otp";
	const char *hismartpa_i2c_state_str = "i2c_addr_state";
	int ret;

	if (!of_property_read_bool(i2c->dev.of_node, hismartpa_otp_regs_str)) {
		hwlog_debug("%s: hismartpa_otp not existed, skip\n", __func__);
		return 0;
	}
	ret = smartpakit_i2c_parse_reg_ctl(&i2c_priv->hismartpa_otp_sequence,
		i2c->dev.of_node, hismartpa_otp_regs_str);
	if (ret < 0) {
		hwlog_err("%s: parse hismartpa_otp_regs failed\n", __func__);
		return -EFAULT;
	}

	ret = smartpakit_get_prop_of_u32_type(i2c->dev.of_node, hismartpa_i2c_state_str,
		&i2c_priv->i2c_addr_state, false);
	if (ret < 0) {
		hwlog_err("%s: parse i2c_addr_state failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static unsigned int smartpakit_i2c_get_reg_value_mask(int val_bits)
{
	unsigned int mask;

	if (val_bits == SMARTPAKIT_REG_VALUE_B16)
		mask = SMARTPAKIT_REG_VALUE_M16;
	else if (val_bits == SMARTPAKIT_REG_VALUE_B24)
		mask = SMARTPAKIT_REG_VALUE_M24;
	else if (val_bits == SMARTPAKIT_REG_VALUE_B32)
		mask = SMARTPAKIT_REG_VALUE_M32;
	else /* SMARTPAKIT_REG_VALUE_B8 or other */
		mask = SMARTPAKIT_REG_VALUE_M8;

	return mask;
}

static void smartpakit_i2c_free_regmap_cfg(struct smartpakit_regmap_cfg *cfg)
{
	if (cfg == NULL)
		return;

	smartpakit_kfree_ops(cfg->reg_writeable);
	smartpakit_kfree_ops(cfg->reg_unwriteable);
	smartpakit_kfree_ops(cfg->reg_readable);
	smartpakit_kfree_ops(cfg->reg_unreadable);
	smartpakit_kfree_ops(cfg->reg_volatile);
	smartpakit_kfree_ops(cfg->reg_unvolatile);
	smartpakit_kfree_ops(cfg->reg_defaults);

	kfree(cfg);
	cfg = NULL;
}

static int smartpakit_i2c_parse_reg_defaults(struct device_node *node,
	struct smartpakit_regmap_cfg *cfg_info)
{
	const char *reg_defaults_str = "reg_defaults";

	return smartpakit_get_prop_of_u32_array(node, reg_defaults_str,
		(u32 **)&cfg_info->reg_defaults, &cfg_info->num_defaults);
}

static int smartpakit_i2c_parse_special_regs_range(struct device_node *node,
	struct smartpakit_regmap_cfg *cfg_info)
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

	ret = smartpakit_get_prop_of_u32_array(node, reg_writeable_str,
		&cfg_info->reg_writeable, &cfg_info->num_writeable);
	ret += smartpakit_get_prop_of_u32_array(node, reg_unwriteable_str,
		&cfg_info->reg_unwriteable, &cfg_info->num_unwriteable);
	ret += smartpakit_get_prop_of_u32_array(node, reg_readable_str,
		&cfg_info->reg_readable, &cfg_info->num_readable);
	ret += smartpakit_get_prop_of_u32_array(node, reg_unreadable_str,
		&cfg_info->reg_unreadable, &cfg_info->num_unreadable);
	ret += smartpakit_get_prop_of_u32_array(node, reg_volatile_str,
		&cfg_info->reg_volatile, &cfg_info->num_volatile);
	ret += smartpakit_get_prop_of_u32_array(node, reg_unvolatile_str,
		&cfg_info->reg_unvolatile, &cfg_info->num_unvolatile);
	ret += smartpakit_i2c_parse_reg_defaults(node, cfg_info);
	return ret;
}

static int smartpakit_i2c_parse_remap_cfg(struct device_node *node,
	struct smartpakit_regmap_cfg **cfg)
{
	struct smartpakit_regmap_cfg *cfg_info = NULL;
	const char *reg_bits_str     = "reg_bits";
	const char *val_bits_str     = "val_bits";
	const char *cache_type_str   = "cache_type";
	const char *max_register_str = "max_register";
	int ret;

	cfg_info = kzalloc(sizeof(*cfg_info), GFP_KERNEL);
	if (cfg_info == NULL)
		return -ENOMEM;

	ret = of_property_read_u32(node, reg_bits_str,
		(u32 *)&cfg_info->cfg.reg_bits);
	if (ret < 0) {
		hwlog_err("%s: get reg_bits failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}

	ret = of_property_read_u32(node, val_bits_str,
		(u32 *)&cfg_info->cfg.val_bits);
	if (ret < 0) {
		hwlog_err("%s: get val_bits failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}
	cfg_info->value_mask = smartpakit_i2c_get_reg_value_mask(
		cfg_info->cfg.val_bits);

	ret = of_property_read_u32(node, cache_type_str,
		(u32 *)&cfg_info->cfg.cache_type);
	if ((ret < 0) || (cfg_info->cfg.cache_type > REGCACHE_FLAT)) {
		hwlog_err("%s: get cache_type failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}

	ret = of_property_read_u32(node, max_register_str,
		&cfg_info->cfg.max_register);
	if (ret < 0) {
		hwlog_err("%s: get max_register failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}

	ret = smartpakit_i2c_parse_special_regs_range(node, cfg_info);
	if (ret < 0)
		goto err_out;

	*cfg = cfg_info;
	return 0;

err_out:
	smartpakit_i2c_free_regmap_cfg(cfg_info);
	return ret;
}

static int smartpakit_i2c_regmap_init(struct i2c_client *i2c,
	struct smartpakit_i2c_priv *i2c_priv)
{
	const char *regmap_cfg_str = "regmap_cfg";
	struct smartpakit_regmap_cfg *cfg = NULL;
	struct device_node *node = NULL;
	int ret;
	int val_num;

	node = of_get_child_by_name(i2c->dev.of_node, regmap_cfg_str);
	if (node == NULL) {
		hwlog_debug("%s: regmap_cfg not existed, skip\n", __func__);
		return 0;
	}

	ret = smartpakit_i2c_parse_remap_cfg(node, &i2c_priv->regmap_cfg);
	if (ret < 0)
		return ret;

	cfg = i2c_priv->regmap_cfg;
	val_num = sizeof(struct reg_default) / sizeof(unsigned int);
	if (cfg->num_defaults > 0) {
		if ((cfg->num_defaults % val_num) != 0) {
			hwlog_err("%s: reg_defaults %d%%%d != 0\n",
				__func__, cfg->num_defaults, val_num);
			ret = -EFAULT;
			goto err_out;
		}
	}

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: regmap_cfg get w%d,%d,r%d,%d,v%d,%d,default%d\n",
		__func__, cfg->num_writeable, cfg->num_unwriteable,
		cfg->num_readable, cfg->num_unreadable, cfg->num_volatile,
		cfg->num_unvolatile, cfg->num_defaults / val_num);
#endif

	/* set num_reg_defaults */
	if (cfg->num_defaults > 0) {
		cfg->num_defaults /= val_num;
		cfg->cfg.reg_defaults = cfg->reg_defaults;
		cfg->cfg.num_reg_defaults = (unsigned int)cfg->num_defaults;
	}

	cfg->cfg.writeable_reg = smartpakit_i2c_writeable_reg;
	cfg->cfg.readable_reg  = smartpakit_i2c_readable_reg;
	cfg->cfg.volatile_reg  = smartpakit_i2c_volatile_reg;

	cfg->regmap = regmap_init_i2c(i2c, &cfg->cfg);
	if (IS_ERR(cfg->regmap)) {
		hwlog_err("%s: regmap_init_i2c regmap failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}
	return 0;
err_out:
	smartpakit_i2c_free_regmap_cfg(i2c_priv->regmap_cfg);
	return ret;
}

static int smartpakit_i2c_regmap_deinit(struct smartpakit_i2c_priv *i2c_priv)
{
	struct smartpakit_regmap_cfg *cfg = NULL;

	if ((i2c_priv == NULL) || (i2c_priv->regmap_cfg == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}
	cfg = i2c_priv->regmap_cfg;

	regmap_exit(cfg->regmap);
	cfg->regmap = NULL;
	smartpakit_i2c_free_regmap_cfg(cfg);
	return 0;
}

static int smartpakit_i2c_check_chip_info_valid(
	struct smartpakit_i2c_priv *i2c_priv)
{
	if (i2c_priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (i2c_priv->chip_id >= SMARTPAKIT_PA_ID_MAX) {
		hwlog_err("%s: invalid chip_id %u\n", __func__,
			i2c_priv->chip_id);
		return -EFAULT;
	}

	if (i2c_priv->chip_vendor >= SMARTPAKIT_CHIP_VENDOR_MAX) {
		hwlog_err("%s: invalid chip_vendor %u\n", __func__,
			i2c_priv->chip_vendor);
		return -EFAULT;
	}
	return 0;
}

static int smartpakit_i2c_parse_dt_chip(struct i2c_client *i2c,
	struct smartpakit_i2c_priv *i2c_priv)
{
	const char *chip_vendor_str  = "chip_vendor";
	const char *chip_id_str      = "chip_id";
	const char *chip_model_str   = "chip_model";
	const char *wait_time_str    = "reset_debounce_wait_time";
	const char *i2c_addr_str     = "i2c_pseudo_addr";
	const char *irq_debounce_str = "sync_irq_debounce_time";
	int ret;

	ret = smartpakit_get_prop_of_u32_type(i2c->dev.of_node, chip_vendor_str,
		&i2c_priv->chip_vendor, true);
	ret += smartpakit_get_prop_of_u32_type(i2c->dev.of_node, chip_id_str,
		&i2c_priv->chip_id, true);
	ret += smartpakit_get_prop_of_str_type(i2c->dev.of_node, chip_model_str,
		&i2c_priv->chip_model);

	ret += smartpakit_get_prop_of_u32_type(i2c->dev.of_node, wait_time_str,
		&i2c_priv->reset_debounce_wait_time, false);

	ret += smartpakit_get_prop_of_u32_type(i2c->dev.of_node, i2c_addr_str,
		&i2c_priv->i2c_pseudo_addr, false);
	if (ret < 0)
		goto err_out;
	ret = smartpakit_i2c_check_chip_info_valid(i2c_priv);
	if (ret < 0)
		goto err_out;

	if (of_property_read_bool(i2c->dev.of_node, irq_debounce_str)) {
		i2c_priv->sync_irq_debounce_time = true;
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: sync irq debounce time enabled\n", __func__);
#endif
	}

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: chip_vendor:%u,chip_id:%u,chip_model:%s\n", __func__,
		i2c_priv->chip_vendor, i2c_priv->chip_id,
		i2c_priv->chip_model);
#endif
	return 0;

err_out:
	return ret;
}

static void smartpakit_i2c_free(struct smartpakit_i2c_priv *i2c_priv)
{
	if (i2c_priv == NULL) {
		hwlog_err("%s: i2c_priv invalid argument\n", __func__);
		return;
	}

	if (i2c_priv->hw_reset != NULL) {
		gpio_free((unsigned int)i2c_priv->hw_reset->gpio);
		i2c_priv->hw_reset->gpio = 0;
		/* irq_debounce_work was set in hw_reset flow */
		if (i2c_priv->reset_debounce_wait_time > 0)
			cancel_delayed_work_sync(&i2c_priv->irq_debounce_work);
	}

	smartpakit_i2c_free_hw_reset(i2c_priv->hw_reset);
	smartpakit_i2c_free_irq_handler(i2c_priv->irq_handler);
	smartpakit_i2c_free_reg_ctl(i2c_priv->version_regs_seq);
	smartpakit_i2c_free_reg_ctl(i2c_priv->dump_regs_sequence);
	smartpakit_i2c_free_reg_ctl(i2c_priv->hismartpa_otp_sequence);

	if (i2c_priv->regmap_cfg != NULL)
		smartpakit_i2c_regmap_deinit(i2c_priv);

	kfree(i2c_priv);
}

static void smartpakit_i2c_reset_priv_data(
	struct smartpakit_i2c_priv *i2c_priv)
{
	if (i2c_priv == NULL)
		return;

	i2c_priv->probe_completed          = 0;
	i2c_priv->reset_debounce_wait_time = 0;
	i2c_priv->irq_debounce_jiffies     = 0;
	i2c_priv->reset_debounce_wait_time = 0;
	i2c_priv->i2c_pseudo_addr          = 0;
	i2c_priv->sync_irq_debounce_time   = false;
}

static int smartpakit_i2c_init_chip(struct i2c_client *i2c,
	struct smartpakit_i2c_priv *i2c_priv)
{
	int ret;

	ret = smartpakit_i2c_request_reset_gpio(i2c_priv);
	if (ret < 0) {
		hwlog_err("%s: request reset gpio err\n", __func__);
		goto err_out;
	}

	/* reset chip */
	smartpakit_i2c_hw_reset(i2c_priv);

	return smartpakit_i2c_request_threaded_irq(i2c, i2c_priv);
err_out:
	smartpakit_i2c_free_hw_reset(i2c_priv->hw_reset);
	i2c_priv->hw_reset = NULL;
	return ret;
}

static int smartpakit_i2c_read_chip_version(
	struct smartpakit_i2c_priv *i2c_priv)
{
	int ret;

	if (i2c_priv->version_regs_seq == NULL)
		return 0;

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: pa %u, read version\n", __func__, i2c_priv->chip_id);
#endif
	ret = smartpakit_i2c_dts_reg_ops(i2c_priv, i2c_priv->version_regs_seq,
		NULL);
	if (ret < 0)
		hwlog_err("%s: %s read version regs failed, %d\n", __func__,
			i2c_priv->chip_model, ret);

	return ret;
}

static int smartpakit_i2c_probe(struct i2c_client *i2c,
	const struct i2c_device_id *id)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;
	int ret;

	if (smartpakit_get_misc_init_flag() == 0) {
		hwlog_info("%s: this driver need probe_defer\n", __func__);
		return -EPROBE_DEFER;
	}

	if (i2c == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: device %s, addr=0x%x, flags=0x%x\n", __func__,
		id->name, i2c->addr, i2c->flags);
	if (!i2c_check_functionality(i2c->adapter, I2C_FUNC_I2C)) {
		hwlog_err("%s: i2c check functionality error\n", __func__);
		return -ENODEV;
	}

	i2c_priv = kzalloc(sizeof(*i2c_priv), GFP_KERNEL);
	if (i2c_priv == NULL)
		return -ENOMEM;

	smartpakit_i2c_reset_priv_data(i2c_priv);

	ret = smartpakit_i2c_parse_dt_chip(i2c, i2c_priv);
	if (ret < 0)
		goto err_out;

	if (smartpakit_i2c_probe_skip[i2c_priv->chip_id]) {
		hwlog_info("%s: chip_id = %u has been probed success, skip\n",
			__func__, i2c_priv->chip_id);
		ret = -EINVAL;
		goto skip_probe;
	}

	i2c_priv->dev = &i2c->dev;
	i2c_priv->i2c = i2c;

	i2c_set_clientdata(i2c, i2c_priv);
	dev_set_drvdata(&i2c->dev, i2c_priv);

	INIT_WORK(&i2c_priv->irq_handle_work, smartpakit_i2c_handler_irq);
#ifdef CONFIG_HUAWEI_ARMPC_PLATFORM
	INIT_WORK(&i2c_priv->pm_s4_work, smartpakit_restore_process);
#endif
	INIT_DELAYED_WORK(&i2c_priv->irq_debounce_work,
		smartpakit_i2c_irq_debounce_work);

	ret = smartpakit_i2c_parse_dt_reset(i2c, i2c_priv);
	ret += smartpakit_i2c_parse_dt_irq(i2c, i2c_priv);
	ret += smartpakit_i2c_parse_dt_version_regs(i2c, i2c_priv);
	ret += smartpakit_i2c_parse_dt_dump_regs(i2c, i2c_priv);
	ret += smartpakit_i2c_parse_dt_hismartpa(i2c, i2c_priv);
	if (ret < 0) {
		smartpakit_i2c_free_hw_reset(i2c_priv->hw_reset);
		goto err_out;
	}
	/* init chip */
	ret = smartpakit_i2c_init_chip(i2c, i2c_priv);
	if (ret < 0)
		goto err_out;

	/* int regmap */
	ret = smartpakit_i2c_regmap_init(i2c, i2c_priv);
	if (ret < 0)
		goto err_out;

	ret = smartpakit_i2c_read_chip_version(i2c_priv);
	if (ret < 0) {
		insert_i2c_dev_info_list(i2c_priv, 0); /* i2c check failed */
		goto err_out;
	}

	/* register i2c device to smartpakit device */
	ret = smartpakit_register_i2c_device(i2c_priv);
	if (ret < 0)
		goto err_out;

	smartpakit_register_i2c_ctl_ops(&i2c_ctl_ops);

	delete_list_info_by_given_chip_id(i2c_priv->chip_id);
	insert_i2c_dev_info_list(i2c_priv, 1); /* i2c check success */
	i2c_priv->probe_completed = 1;
	smartpakit_i2c_probe_skip[i2c_priv->chip_id] = true;
	hwlog_info("%s: end success\n", __func__);
	return 0;

err_out:
	hwlog_err("%s: end failed\n", __func__);
skip_probe:
	smartpakit_i2c_free(i2c_priv);
	return ret;
}

static int smartpakit_i2c_remove(struct i2c_client *i2c)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;

	hwlog_info("%s: remove\n", __func__);
	if (i2c == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	i2c_priv = i2c_get_clientdata(i2c);
	if (i2c_priv == NULL) {
		hwlog_err("%s: i2c_priv invalid\n", __func__);
		return -EINVAL;
	}

	i2c_set_clientdata(i2c, NULL);
	dev_set_drvdata(&i2c->dev, NULL);

	/* deregister i2c device */
	smartpakit_deregister_i2c_device(i2c_priv);
	smartpakit_deregister_i2c_ctl_ops();

	smartpakit_i2c_free(i2c_priv);
	return 0;
}

static void smartpakit_i2c_shutdown(struct i2c_client *i2c)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;
	struct smartpakit_gpio_sequence *sequence = NULL;
	unsigned int gpio;
	int i;

	hwlog_info("%s: shutdown for smartpakit_i2c\n", __func__);
	if (i2c == NULL) {
		hwlog_err("%s: invalid input argument\n", __func__);
		return;
	}

	i2c_priv = i2c_get_clientdata(i2c);
	if (i2c_priv == NULL) {
		hwlog_err("%s: i2c_priv invalid\n", __func__);
		return;
	}

	if ((i2c_priv->hw_reset == NULL) ||
		(i2c_priv->hw_reset->sequence.node == NULL) ||
		(i2c_priv->hw_reset->sequence.num == 0)) {
		hwlog_info("%s: hw_reset is invalid\n", __func__);
		return;
	}

	/* for shutdown in music playing */
	if (i2c_priv->hw_reset->not_need_shutdown) {
		hwlog_info("%s: not need shutdown, skip\n", __func__);
		return;
	}

	sequence = &i2c_priv->hw_reset->sequence;
	gpio = (unsigned int)i2c_priv->hw_reset->gpio;
	for (i = (int)(sequence->num - 1); i >= 0; i--) {
		gpio_direction_output(gpio,
			(int)sequence->node[i].state);
		hwlog_info("%s: set gpio %d %u, delay %u\n", __func__,
			i2c_priv->hw_reset->gpio, sequence->node[i].state,
			sequence->node[i].delay);
		smartpakit_delay(sequence->node[i].delay, i, 0);
	}
}

#ifdef CONFIG_PM
static int smartpakit_i2c_suspend(struct device *dev)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;

	if (dev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	i2c_priv = dev_get_drvdata(dev);
	if (i2c_priv == NULL)
		return 0;

#ifdef CONFIG_HUAWEI_ARMPC_PLATFORM
	struct smartpakit_gpio_irq *irq_handler = NULL;

	irq_handler = i2c_priv->irq_handler;
	if (irq_handler != NULL) {
		disable_irq(irq_handler->irq);
		hwlog_info("%s: disable irq:%d\n", __func__, irq_handler->irq);
	}
#endif
	if ((i2c_priv->regmap_cfg != NULL) &&
		(i2c_priv->regmap_cfg->regmap != NULL) &&
		(i2c_priv->regmap_cfg->cfg.cache_type == REGCACHE_RBTREE))
		regcache_cache_only(i2c_priv->regmap_cfg->regmap, (bool)true);

	return 0;
}

static int smartpakit_i2c_resume(struct device *dev)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;

	if (dev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	i2c_priv = dev_get_drvdata(dev);
	if (i2c_priv == NULL)
		return 0;

	if ((i2c_priv->regmap_cfg != NULL) &&
		(i2c_priv->regmap_cfg->regmap != NULL) &&
		(i2c_priv->regmap_cfg->cfg.cache_type == REGCACHE_RBTREE)) {
		regcache_cache_only(i2c_priv->regmap_cfg->regmap, (bool)false);
		regcache_sync(i2c_priv->regmap_cfg->regmap);
	}
#ifdef CONFIG_HUAWEI_ARMPC_PLATFORM
	struct smartpakit_gpio_irq *irq_handler = NULL;

	irq_handler = i2c_priv->irq_handler;
	if (irq_handler != NULL) {
		enable_irq(irq_handler->irq);
		hwlog_info("%s: end and enable irq : %d\n",
					__func__, irq_handler->irq);
	}
#endif
	return 0;
}
#else
#define smartpakit_i2c_suspend NULL /* function pointer */
#define smartpakit_i2c_resume  NULL /* function pointer */
#endif /* CONFIG_PM */

static const struct dev_pm_ops smartpakit_i2c_pm_ops = {
	.suspend = smartpakit_i2c_suspend,
	.resume  = smartpakit_i2c_resume,
#ifdef CONFIG_HUAWEI_ARMPC_PLATFORM
	.freeze = smartpakit_i2c_freeze,
	.restore = smartpakit_i2c_restore,
#endif
};

static const struct i2c_device_id smartpakit_i2c_id[] = {
	{ "smartpakit_i2c", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, smartpakit_i2c_id);

static const struct of_device_id smartpakit_i2c_match[] = {
	{ .compatible = "huawei,smartpakit_i2c", },
	{},
};
MODULE_DEVICE_TABLE(of, smartpakit_i2c_match);

static struct i2c_driver smartpakit_i2c_driver = {
	.driver = {
		.name           = "smartpakit_i2c",
		.owner          = THIS_MODULE,
		.pm             = &smartpakit_i2c_pm_ops,
		.of_match_table = of_match_ptr(smartpakit_i2c_match),
	},
	.probe    = smartpakit_i2c_probe,
	.remove   = smartpakit_i2c_remove,
	.shutdown = smartpakit_i2c_shutdown,
	.id_table = smartpakit_i2c_id,
};

static int __init smartpakit_i2c_init(void)
{
#ifdef CONFIG_HUAWEI_SMARTPAKIT_AUDIO_MODULE
	hwlog_info("%s enter from modprobe\n", __func__);
#endif
	return i2c_add_driver(&smartpakit_i2c_driver);
}

static void __exit smartpakit_i2c_exit(void)
{
	i2c_del_driver(&smartpakit_i2c_driver);
}

late_initcall(smartpakit_i2c_init);
module_exit(smartpakit_i2c_exit);

MODULE_DESCRIPTION("smartpakit i2c driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");

