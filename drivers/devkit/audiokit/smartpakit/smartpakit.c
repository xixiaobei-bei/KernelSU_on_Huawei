/*
 * smartpakit.c
 *
 * smartpakit driver
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
#include "smartpakit.h"

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of_device.h>
#include <linux/i2c-dev.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/workqueue.h>
#include <huawei_platform/log/hw_log.h>
#include <dsm/dsm_pub.h>
#include "securec.h"

#ifdef CONFIG_HUAWEI_DSM_AUDIO_MODULE
#define CONFIG_HUAWEI_DSM_AUDIO
#endif
#ifdef CONFIG_HUAWEI_DSM_AUDIO
#include <dsm_audio/dsm_audio.h>
#endif

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#if (defined(CONFIG_MTK_PLATFORM) || defined(CONFIG_ARCH_QCOM))
#include <hwmanufac/dev_detect/dev_detect.h>
#else
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#endif

#define HWLOG_TAG smartpakit
HWLOG_REGIST();
#define REG_PRINT_NUM 2
#define EREMOTEIO    121

#ifdef CONFIG_SND_SOC_AU_PA
extern struct smartpa_regulator_reg_ops hisi_reg_ops;

int __attribute__((weak)) smartpa_regulator_reg_ops_register(
	struct smartpa_regulator_reg_ops *reg_ops)
{
	return 0;
}
#endif

static struct smartpakit_priv *smartpakit_priv_data;

/* misc device 0 not init completed, 1 init completed */
static int smartpakit_init_flag;

static int (*smartpakit_ctrl_init_chips)(struct smartpakit_priv *pakit_priv,
	const void __user *arg, int compat_mode, unsigned int id);
static int (*smartpakit_ctrl_write_chips)(struct smartpakit_priv *pakit_priv,
	const void __user *arg, int compat_mode, unsigned int id);

struct smartpakit_priv *smartpakit_get_misc_priv(void)
{
	return smartpakit_priv_data;
}

int smartpakit_get_misc_init_flag(void)
{
	return smartpakit_init_flag;
}

void smartpakit_reg_info_add_list(struct smartpakit_reg_info *reg_info,
	struct list_head *pos)
{
	struct smartpakit_reg_info *list_node_info = NULL;

	if ((reg_info == NULL) || (pos == NULL))
		return;

	list_node_info = kzalloc(sizeof(*list_node_info), GFP_KERNEL);
	if (list_node_info == NULL)
		return;

	list_node_info->chip_id  = reg_info->chip_id;
	list_node_info->info     = reg_info->info;
	list_node_info->mask     = reg_info->mask;
	list_node_info->reg_addr = reg_info->reg_addr;
	list_add_tail(&list_node_info->list, pos);
}

void smartpakit_reg_info_del_list_all(struct list_head *head)
{
	struct smartpakit_reg_info *pos = NULL;
	struct smartpakit_reg_info *n = NULL;

	if (head == NULL)
		return;

	list_for_each_entry_safe(pos, n, head, list) {
		list_del(&pos->list);
		kfree(pos);
	}
}

static void smartpakit_map_i2c_addr_to_chip_id(
	struct smartpakit_i2c_priv *i2c_priv, unsigned int id)
{
	unsigned int addr;
	unsigned char chip_id;

	if ((i2c_priv->i2c == NULL) ||
		(!smartpakit_priv_data->misc_rw_permission_enable))
		return;

	/*
	 * use pseudo address to rw i2c
	 * for example:
	 * pa index:    0    1    2    3
	 * i2c addr:    0x35 0x36 0x35 0x36
	 * pseudo addr: 0xAA 0xBB 0xCC 0xDD
	 */
	chip_id = (unsigned char)id;
	addr = smartpakit_priv_data->misc_i2c_use_pseudo_addr ?
		i2c_priv->i2c_pseudo_addr : i2c_priv->i2c->addr;

	if (addr < SMARTPAKIT_I2C_ADDR_ARRAY_MAX)
		smartpakit_priv_data->i2c_addr_to_pa_index[addr] = chip_id;
}

static bool smartpakit_is_chip_model_different(void)
{
	int i;

	if (smartpakit_priv_data->pa_num <= 1)
		return false;

	for (i = 1; i < smartpakit_priv_data->pa_num; i++) {
		if (strcmp(smartpakit_priv_data->chip_model_list[0],
			smartpakit_priv_data->chip_model_list[i]) != 0)
			return true;
	}

	return false;
}

static void smartpakit_set_hw_dev_flag(void)
{
	if (smartpakit_priv_data->i2c_num < smartpakit_priv_data->pa_num) {
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: i2c_num < pa_num\n", __func__);
#endif
		return;
	}

	if (smartpakit_is_chip_model_different()) {
		hwlog_info("%s: NOTICE: CHIP_MODEL NOT SAME\n", __func__);
	} else {
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: check chip_model success\n", __func__);
#endif

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
		set_hw_dev_flag(DEV_I2C_SPEAKER);
#endif
	}
}

static void smartpakit_match_gpio_by_chip_id(
	struct smartpakit_i2c_priv *i2c_priv)
{
	unsigned int chip_id;
	int gpio;

	chip_id = i2c_priv->chip_id;

	if (i2c_priv->hw_reset != NULL) {
		gpio = i2c_priv->hw_reset->gpio;
		smartpakit_priv_data->gpio_of_hw_reset[chip_id] = gpio;
	}

	if (i2c_priv->irq_handler != NULL) {
		gpio = i2c_priv->irq_handler->gpio;
		smartpakit_priv_data->gpio_of_irq[chip_id] = gpio;
	}
}

int smartpakit_register_i2c_device(struct smartpakit_i2c_priv *i2c_priv)
{
	unsigned int str_length;

	if ((smartpakit_priv_data == NULL) || (i2c_priv == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (i2c_priv->chip_id >= smartpakit_priv_data->pa_num) {
		hwlog_err("%s: error, chip_id %u >= pa_num %u\n", __func__,
			i2c_priv->chip_id, smartpakit_priv_data->pa_num);
		smartpakit_priv_data->chip_register_failed = true;
		return -EINVAL;
	}

	if (smartpakit_priv_data->i2c_priv[i2c_priv->chip_id] != NULL) {
		hwlog_err("%s: chip_id reduplicated error\n", __func__);
		smartpakit_priv_data->chip_register_failed = true;
		return -EINVAL;
	}

	i2c_priv->priv_data = (void *)smartpakit_priv_data;
	smartpakit_priv_data->i2c_num++;
	smartpakit_priv_data->i2c_priv[i2c_priv->chip_id] = i2c_priv;

	str_length = (strlen(i2c_priv->chip_model) < SMARTPAKIT_NAME_MAX) ?
		strlen(i2c_priv->chip_model) : (SMARTPAKIT_NAME_MAX - 1);
	if (strncpy_s(smartpakit_priv_data->chip_model_list[i2c_priv->chip_id],
		sizeof(smartpakit_priv_data->chip_model_list[i2c_priv->chip_id]),
		i2c_priv->chip_model, str_length) != EOK)
		hwlog_err("%s: strncpy_s is failed\n", __func__);

	smartpakit_map_i2c_addr_to_chip_id(i2c_priv, i2c_priv->chip_id);
	smartpakit_set_hw_dev_flag();
	smartpakit_match_gpio_by_chip_id(i2c_priv);

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: i2c_priv registered, success\n", __func__);
#endif

	return 0;
}
EXPORT_SYMBOL_GPL(smartpakit_register_i2c_device);

int smartpakit_deregister_i2c_device(struct smartpakit_i2c_priv *i2c_priv)
{
	int i;
	unsigned int chip_id;

	if ((smartpakit_priv_data == NULL) || (i2c_priv == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < (int)smartpakit_priv_data->pa_num; i++) {
		if (smartpakit_priv_data->i2c_priv[i] == NULL)
			continue;
		chip_id = smartpakit_priv_data->i2c_priv[i]->chip_id;
		if (i2c_priv->chip_id != chip_id)
			continue;

		smartpakit_map_i2c_addr_to_chip_id(i2c_priv,
			SMARTPAKIT_INVALID_PA_INDEX);
		i2c_priv->priv_data = NULL;

		smartpakit_priv_data->i2c_num--;
		smartpakit_priv_data->i2c_priv[i] = NULL;
		hwlog_info("%s: i2c_priv deregistered, success\n", __func__);
		break;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(smartpakit_deregister_i2c_device);

void smartpakit_register_i2c_ctl_ops(struct smartpakit_i2c_ctl_ops *ops)
{
	if ((smartpakit_priv_data == NULL) || (ops == NULL)) {
		hwlog_err("%s: ioctl_ops register failed\n", __func__);
		return;
	}

	if (smartpakit_priv_data->ioctl_ops != NULL) {
#ifndef CONFIG_FINAL_RELEASE
		hwlog_debug("%s: ioctl_ops had registered, skip\n", __func__);
#endif
		return;
	}

	smartpakit_priv_data->ioctl_ops = ops;
#ifndef CONFIG_FINAL_RELEASE
	hwlog_debug("%s: ioctl_ops registered, success\n", __func__);
#endif
}
EXPORT_SYMBOL_GPL(smartpakit_register_i2c_ctl_ops);

void smartpakit_deregister_i2c_ctl_ops(void)
{
	if (smartpakit_priv_data == NULL) {
		hwlog_err("%s: ioctl_ops deregister failed\n", __func__);
		return;
	}

	if (smartpakit_priv_data->i2c_num != 0) {
		hwlog_debug("%s: skip deregister ioctl_ops\n", __func__);
		return;
	}

	smartpakit_priv_data->ioctl_ops = NULL;
	hwlog_info("%s: ioctl_ops deregistered, success\n", __func__);
}
EXPORT_SYMBOL_GPL(smartpakit_deregister_i2c_ctl_ops);

static void smartpakit_reset_pa_ctl_sequence(
	struct smartpakit_pa_ctl_sequence *seq)
{
	if (seq == NULL)
		return;

	seq->pa_ctl_mask      = 0;
	seq->pa_poweron_flag  = SMARTPA_REGS_FLAG_POWER_OFF;
	seq->pa_ctl_num       = 0;
	seq->pa_ctl_index_max = 0;
	seq->pa_ctl_index[0]  = 0;
	seq->param_num        = 0;
	smartpakit_kfree_ops(seq->node);
}

static int smartpakit_write_regs_to_multi_chips(
	struct smartpakit_priv *pakit_priv,
	struct smartpakit_pa_ctl_sequence *sequence)
{
	struct smartpakit_param_node *param_node = NULL;
	struct smartpakit_i2c_priv *i2c_priv_p = NULL;
	int pa_num_need_ops;
	unsigned int reg_num_need_ops;
	int index;
	int ret = 0;
	int i;

	if ((pakit_priv->ioctl_ops == NULL) ||
		(pakit_priv->ioctl_ops->write_regs == NULL)) {
		hwlog_err("%s: write_regs is invalid\n", __func__);
		return -ECHILD;
	}
	pa_num_need_ops = (int)sequence->pa_ctl_num;
	reg_num_need_ops = sequence->param_num_of_one_pa;
	for (i = 0; i < pa_num_need_ops; i++) {
		index = (int)sequence->pa_ctl_index[i];
		param_node = sequence->node + (i * (int)reg_num_need_ops);
		i2c_priv_p = pakit_priv->i2c_priv[index];
		ret += pakit_priv->ioctl_ops->write_regs(i2c_priv_p,
			reg_num_need_ops, param_node);
	}
	return ret;
}

int smartpakit_hw_reset_chips(struct smartpakit_priv *pakit_priv)
{
	struct smartpakit_i2c_priv *i2c_priv_p = NULL;
	int ret = 0;
	int i;

	if (pakit_priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if ((pakit_priv->ioctl_ops == NULL) ||
		(pakit_priv->ioctl_ops->hw_reset == NULL)) {
		hwlog_err("%s: i2c_hw_reset ops is NULL\n", __func__);
		return -ECHILD;
	}

	/* If one gpio control multi-pa, some hw_reset of pa maybe NULL */
	for (i = 0; i < (int)pakit_priv->pa_num; i++) {
		i2c_priv_p = pakit_priv->i2c_priv[i];
		ret = pakit_priv->ioctl_ops->hw_reset(i2c_priv_p);
		if (ret < 0) {
			hwlog_err("%s: chip_id %d reset error\n", __func__, i);
			break;
		}
	}

	return ret;
}
EXPORT_SYMBOL_GPL(smartpakit_hw_reset_chips);

int smartpakit_resume_chips(struct smartpakit_priv *pakit_priv)
{
	struct smartpakit_pa_ctl_sequence *sequence = NULL;
	struct smartpakit_i2c_priv *i2c_priv_p = NULL;
	int ret;
	int i;

	if (pakit_priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if ((pakit_priv->ioctl_ops == NULL) ||
		(pakit_priv->ioctl_ops->write_regs == NULL)) {
		hwlog_err("%s: i2c do_write_regs ops is NULL\n", __func__);
		return -ECHILD;
	}

	pakit_priv->force_refresh_chip = true;

	mutex_lock(&pakit_priv->resume_sequence_lock);

	/* init chips */
	hwlog_info("%s: init chips\n", __func__);
	sequence = &pakit_priv->resume_sequence;

	ret = smartpakit_write_regs_to_multi_chips(pakit_priv, sequence);
	if (ret < 0) {
		hwlog_err("%s: write init regs failed\n", __func__);
		goto err_out;
	}

	/* poweron chips */
	for (i = 0; i < SMARTPAKIT_PA_ID_MAX; i++) {
		sequence = &pakit_priv->poweron_seq[i];
		if (sequence->pa_poweron_flag != SMARTPA_REGS_FLAG_NEED_RESUME)
			continue;
		i2c_priv_p = pakit_priv->i2c_priv[i];
		hwlog_info("%s: poweron pa %u\n", __func__,
			sequence->pa_ctl_index[0]);
		ret = pakit_priv->ioctl_ops->write_regs(i2c_priv_p,
			sequence->param_num, sequence->node);
		if (ret < 0) {
			hwlog_err("%s: pa %d failed\n", __func__, i);
			goto err_out;
		}
	}

err_out:
	pakit_priv->force_refresh_chip = false;
	mutex_unlock(&pakit_priv->resume_sequence_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(smartpakit_resume_chips);

int smartpakit_dump_chips(struct smartpakit_priv *pakit_priv)
{
	struct smartpakit_i2c_priv *i2c_priv_p = NULL;
	int ret = 0;
	int i;

	if (pakit_priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if ((pakit_priv->ioctl_ops == NULL) ||
		(pakit_priv->ioctl_ops->dump_regs == NULL)) {
		hwlog_err("%s: i2c dump_regs ops is NULL\n", __func__);
		return -ECHILD;
	}

	for (i = 0; i < (int)pakit_priv->pa_num; i++) {
		i2c_priv_p = pakit_priv->i2c_priv[i];
		ret = pakit_priv->ioctl_ops->dump_regs(i2c_priv_p);
		if (ret < 0)
			break;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(smartpakit_dump_chips);

static bool smartpakit_is_param_valid(struct smartpakit_set_param *param)
{
	unsigned int val_num;

	val_num = sizeof(struct smartpakit_param_node) / sizeof(unsigned int);
	if (val_num == 0)
		return false;

	if ((param->param_num == 0) ||
		(param->param_num > SMARTPAKIT_IO_PARAMS_NUM_MAX) ||
		(param->params == NULL)) {
		hwlog_err("%s: argument invalid, param num %u\n", __func__,
			param->param_num);
		return false;
	}

	if ((param->param_num % val_num) != 0) {
		hwlog_err("%s: regs_num %u %% %u != 0, please check XML\n",
			__func__, param->param_num, val_num);
		return false;
	}

	return true;
}

static int smartpakit_get_param_header_from_user(const void __user *arg,
	int compat_mode, struct smartpakit_set_param *param)
{
	unsigned int param_size;

#ifdef CONFIG_COMPAT
	if (compat_mode == 0) {
#endif /* CONFIG_COMPAT */
		/* for system/lib64/x.so(64 bits) */
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: copy_from_user b64\n", __func__);
#endif
		if (copy_from_user(param, arg, sizeof(*param))) {
			hwlog_err("%s: get param failed\n", __func__);
			goto err_out;
		}

#ifdef CONFIG_COMPAT
	} else {
		struct smartpakit_set_param_compat s_compat;

		param_size = sizeof(s_compat);
		/* for system/lib/x.so(32 bits) */
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: copy_from_user32\n", __func__);
#endif
		if (copy_from_user(&s_compat, arg, param_size)) {
			hwlog_err("%s: get param failed\n", __func__);
			goto err_out;
		}

		param->pa_ctl_mask = s_compat.pa_ctl_mask;
		param->param_num = s_compat.param_num;
		param->params = compat_ptr(s_compat.params_ptr);
	}
#endif /* CONFIG_COMPAT */

	if (smartpakit_is_param_valid(param))
		return 0;
err_out:
	return -EFAULT;
}

static int smartpakit_get_param_from_user(const void __user *arg,
	int compat_mode,
	struct smartpakit_set_param *param, struct smartpakit_param_node **node)
{
	struct smartpakit_param_node *par_node = NULL;
	int ret;
	unsigned int par_size;

	ret = smartpakit_get_param_header_from_user(arg, compat_mode, param);
	if (ret < 0) {
		hwlog_err("%s: get param head failed\n", __func__);
		return ret;
	}

	par_size = sizeof(int) * param->param_num;
	par_node = kzalloc(par_size, GFP_KERNEL);
	if (par_node == NULL)
		return -ENOMEM;

	if (copy_from_user(par_node, (void __user *)param->params, par_size)) {
		hwlog_err("%s: get param data failed\n", __func__);
		kfree(par_node);
		return -EIO;
	}
	*node = par_node;
	return 0;
}

static int smartpakit_fill_pa_ctl_items(
	struct smartpakit_pa_ctl_sequence *sequence,
	unsigned int ctl_mask)
{
	unsigned int pa_ctl_write;
	unsigned int i;

	if (smartpakit_priv_data == NULL)
		return -EINVAL;

	sequence->pa_ctl_num = 0;
	sequence->pa_ctl_index_max = 0;
	for (i = 0; i < SMARTPAKIT_PA_ID_MAX; i++) {
		pa_ctl_write = ctl_mask >> (i * SMARTPAKIT_PA_CTL_OFFSET);
		pa_ctl_write &= SMARTPAKIT_PA_CTL_MASK;

		if (pa_ctl_write == 0)
			continue;

		if (i >= smartpakit_priv_data->i2c_num) {
			hwlog_err("%s: ctl_mask invalid\n", __func__);
			return -EINVAL;
		}

		sequence->pa_ctl_index_max = i;
		sequence->pa_ctl_index[sequence->pa_ctl_num] = i;
		sequence->pa_ctl_num++;
	}
	return 0;
}

static int smartpakit_fill_param_sequence(
	struct smartpakit_pa_ctl_sequence *sequence,
	struct smartpakit_set_param *param,
	struct smartpakit_param_node *node)
{
	unsigned int val_num;
#ifndef CONFIG_FINAL_RELEASE
	int i;
#endif

	sequence->pa_poweron_flag = SMARTPA_REGS_FLAG_POWER_OFF;
	sequence->pa_ctl_mask = param->pa_ctl_mask;
	/* check whether power on or not */
	if ((param->pa_ctl_mask & SMARTPAKIT_NEED_RESUME_MASK) != 0) {
		/* clear need resume flag */
		param->pa_ctl_mask &= (~SMARTPAKIT_NEED_RESUME_MASK);
		sequence->pa_poweron_flag = SMARTPA_REGS_FLAG_NEED_RESUME;
	} else if ((param->pa_ctl_mask & SMARTPAKIT_PA_MUTE_MASK) != 0) {
		/* clear pa mute flag */
		param->pa_ctl_mask &= (~SMARTPAKIT_PA_MUTE_MASK);
		sequence->pa_poweron_flag = SMARTPA_REGS_FLAG_PA_MUTE;
	}
	if (smartpakit_fill_pa_ctl_items(sequence, param->pa_ctl_mask) < 0)
		return -EINVAL;

	val_num = sizeof(*node) / sizeof(unsigned int);
	sequence->param_num = param->param_num / val_num;
	sequence->param_num_of_one_pa = sequence->param_num;
	sequence->node = node;

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: ctl_mask=0x%08x, ctl_num=%d\n",
		__func__, param->pa_ctl_mask, sequence->pa_ctl_num);
	for (i = 0; i < SMARTPAKIT_PA_ID_MAX; i++)
		hwlog_info("%s: pa_ctl_index:%d %u\n",
			__func__, i, sequence->pa_ctl_index[i]);
#endif
	return 0;
}

static void smartpakit_print_node_info(struct smartpakit_param_node *node,
	unsigned int node_num)
{
	unsigned int print_node_num;
	unsigned int i;

	print_node_num = (node_num < REG_PRINT_NUM) ? node_num : REG_PRINT_NUM;

	/* only print two registers */
	for (i = 0; i < print_node_num; i++)
		hwlog_info("%s: reg_%d=0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
			__func__, i, node[i].index, node[i].mask, node[i].value,
			node[i].delay, node[i].reserved);
}

static int smartpakit_parse_params(struct smartpakit_pa_ctl_sequence *sequence,
	const void __user *arg, int compat_mode)
{
	struct smartpakit_param_node *node = NULL;
	struct smartpakit_set_param param;
	int ret;

	hwlog_debug("%s: enter\n", __func__);
	if ((sequence == NULL) || (arg == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (memset_s(&param, sizeof(param), 0, sizeof(param)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);
	ret = smartpakit_get_param_from_user(arg, compat_mode, &param, &node);
	if (ret < 0) {
		hwlog_err("%s: get param from user failed\n", __func__);
		return ret;
	}
	ret = smartpakit_fill_param_sequence(sequence, &param, node);
	if (ret < 0) {
		hwlog_err("%s: fill sequence failed\n", __func__);
		goto err_out;
	}
	smartpakit_print_node_info(sequence->node, sequence->param_num);
	hwlog_debug("%s: enter end, success\n", __func__);
	return 0;

err_out:
	smartpakit_kfree_ops(node);
	hwlog_debug("%s: error end, ret=%d\n", __func__, ret);
	return ret;
}

static int smartpakit_check_sequence_pa_ctl_property(
	struct smartpakit_pa_ctl_sequence *seq)
{
	if ((smartpakit_priv_data == NULL) || (seq == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	/* check pa_ctl_num */
	if ((seq->pa_ctl_num > smartpakit_priv_data->pa_num) ||
		(seq->pa_ctl_num == 0)) {
		hwlog_err("%s: pa_ctl_num %u invalid\n", __func__,
			seq->pa_ctl_num);
		return -EINVAL;
	}

	if (seq->pa_ctl_index_max >= smartpakit_priv_data->pa_num) {
		hwlog_err("%s: pa_ctl_index_max %u> pa num %u\n", __func__,
			seq->pa_ctl_index_max, smartpakit_priv_data->pa_num);
		return -EINVAL;
	}

	if (seq->param_num % seq->pa_ctl_num != 0) {
		hwlog_err("%s: param_num can not split correct\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int smartpakit_split_sequence_for_multi_chips_ctl(
	struct smartpakit_pa_ctl_sequence *sequence)
{
	int i;

	if ((sequence->param_num == 0) || (sequence->node == NULL)) {
		hwlog_err("%s: invalid input sequence\n", __func__);
		return -EINVAL;
	}
	/* if ctl_mask not config, all 1 as default */
	if (sequence->pa_ctl_mask == 0) {
		sequence->pa_ctl_num = smartpakit_priv_data->pa_num;
		for (i = 0; i < (int)sequence->pa_ctl_num; i++)
			sequence->pa_ctl_index[i] = (unsigned int)i;
		/* 1: one pa */
		if (smartpakit_priv_data->pa_num >= 1)
			sequence->pa_ctl_index_max = smartpakit_priv_data->pa_num - 1;
		else
			sequence->pa_ctl_index_max = 0;
	}

	if (smartpakit_check_sequence_pa_ctl_property(sequence) < 0)
		return -EINVAL;

	sequence->param_num_of_one_pa = sequence->param_num /
		sequence->pa_ctl_num;
	return 0;
}

static int smartpakit_set_poweron_regs(struct smartpakit_priv *pakit_priv,
	struct smartpakit_pa_ctl_sequence *sequence)
{
	struct smartpakit_pa_ctl_sequence *poweron = NULL;
	struct smartpakit_param_node *node = NULL;
	int i;
	unsigned int offset;
	unsigned int param_size;
	unsigned int index;

	if ((pakit_priv == NULL) || (sequence == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s:mask=0x%x,poweron_flag=%u,ctl_num=%u,%u node per pa\n",
		__func__, sequence->pa_ctl_mask, sequence->pa_poweron_flag,
		sequence->pa_ctl_num, sequence->param_num_of_one_pa);

	for (i = 0; i < (int)sequence->pa_ctl_num; i++) {
		if (sequence->pa_poweron_flag == SMARTPA_REGS_FLAG_PA_MUTE)
			continue;

		index = sequence->pa_ctl_index[i];
		poweron = &pakit_priv->poweron_seq[index];
		smartpakit_reset_pa_ctl_sequence(poweron);
		if (sequence->pa_poweron_flag == SMARTPA_REGS_FLAG_POWER_OFF)
			continue;

		node = sequence->node + (i * sequence->param_num_of_one_pa);
		offset = (index * SMARTPAKIT_PA_CTL_OFFSET);

		poweron->pa_ctl_mask = (1 << offset);
		poweron->pa_poweron_flag = SMARTPA_REGS_FLAG_NEED_RESUME;
		poweron->pa_ctl_num = 1;
		poweron->pa_ctl_index_max = sequence->pa_ctl_index[i];
		poweron->pa_ctl_index[0] = sequence->pa_ctl_index[i];
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: set mask 0x%x, index %u\n", __func__,
			poweron->pa_ctl_mask, poweron->pa_ctl_index_max);
#endif
		poweron->param_num = sequence->param_num_of_one_pa;
		param_size = sizeof(*node) * poweron->param_num;
		poweron->node = kzalloc(param_size, GFP_KERNEL);
		if (poweron->node == NULL)
			return -ENOMEM;

		if (memcpy_s(poweron->node, param_size,
			node, param_size) != EOK)
			hwlog_err("%s: memcpy_s is failed\n", __func__);
	}

	return 0;
}

static int smartpakit_ctrl_open(struct inode *inode, struct file *filp)
{
	int ret;

	if ((inode == NULL) || (filp == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	ret = nonseekable_open(inode, filp);
	if (ret < 0) {
		hwlog_err("%s: nonseekable_open failed\n", __func__);
		return ret;
	}

	filp->private_data = (void *)smartpakit_priv_data;
	return ret;
}

static int smartpakit_ctrl_release(struct inode *inode, struct file *filp)
{
	if ((inode == NULL) || (filp == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	filp->private_data = NULL;
	return 0;
}

static int smartpakit_get_current_i2c_client(
	struct smartpakit_priv *pakit_priv, unsigned short addr)
{
	unsigned char pa_index;

	if (pakit_priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&pakit_priv->i2c_ops_lock);

	pakit_priv->current_i2c_client = NULL;
	if (addr >= SMARTPAKIT_I2C_ADDR_ARRAY_MAX) {
		hwlog_err("%s: invalid i2c addr 0x%x\n", __func__, addr);
		goto err_out;
	}
	pa_index = pakit_priv->i2c_addr_to_pa_index[addr];
	if (pa_index == SMARTPAKIT_INVALID_PA_INDEX) {
		hwlog_err("%s: i2c 0x%x not register\n", __func__, addr);
		goto err_out;
	}

	pakit_priv->current_i2c_client = pakit_priv->i2c_priv[pa_index]->i2c;
	mutex_unlock(&pakit_priv->i2c_ops_lock);
	return 0;
err_out:
	mutex_unlock(&pakit_priv->i2c_ops_lock);
	return -EINVAL;
}

static ssize_t smartpakit_ctrl_read(struct file *file, char __user *buf,
	size_t nbytes, loff_t *pos)
{
	struct smartpakit_priv *pakit_priv = NULL;
	void *kern_buf = NULL;
	int ret;

	unused(pos);
	if ((file == NULL) || (buf == NULL)) {
		hwlog_err("%s: invalid arg, file or buf is NULL\n", __func__);
		return -EINVAL;
	}
	pakit_priv = file->private_data;

	if ((pakit_priv == NULL) || (pakit_priv->current_i2c_client == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EFAULT;
	}

	mutex_lock(&pakit_priv->i2c_ops_lock);

	if ((nbytes == 0) || (nbytes > SMARTPAKIT_RW_PARAMS_NUM_MAX)) {
		hwlog_err("%s: nbytes %d is 0 or >%u bytes, error\n", __func__,
			(int)nbytes, SMARTPAKIT_RW_PARAMS_NUM_MAX);
		ret = -EINVAL;
		goto err_out;
	}

	kern_buf = kmalloc(nbytes, GFP_KERNEL);
	if (kern_buf == NULL) {
		ret = -ENOMEM;
		goto err_out;
	}

	if ((pakit_priv->ioctl_ops == NULL) ||
		(pakit_priv->ioctl_ops->i2c_read == NULL)) {
		ret = -ECHILD;
		goto err_out;
	}

	ret = pakit_priv->ioctl_ops->i2c_read(pakit_priv->current_i2c_client,
		(char *)kern_buf, (int)nbytes);
	if (ret < 0) {
		hwlog_err("%s: i2c read error %d\n", __func__, ret);
		goto err_out;
	}

	if (copy_to_user((void __user *)buf, kern_buf, nbytes)) {
		ret = -EFAULT;
		goto err_out;
	}

	mutex_unlock(&pakit_priv->i2c_ops_lock);
	kfree(kern_buf);
	return (ssize_t)nbytes;
err_out:
	mutex_unlock(&pakit_priv->i2c_ops_lock);
	if (kern_buf != NULL)
		kfree(kern_buf);
	return ret;
}

static ssize_t smartpakit_ctrl_write(struct file *file,
	const char __user *buf, size_t nbytes, loff_t *ppos)
{
	struct smartpakit_priv *pakit_priv = NULL;
	void *kern_buf = NULL;
	int ret;

	unused(ppos);
	if ((file == NULL) || (buf == NULL)) {
		hwlog_err("%s: invalid arg, file or buf is NULL\n", __func__);
		return -EINVAL;
	}

	pakit_priv = file->private_data;
	if ((pakit_priv == NULL) || (pakit_priv->current_i2c_client == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EFAULT;
	}

	mutex_lock(&pakit_priv->i2c_ops_lock);

	if ((nbytes == 0) || (nbytes > SMARTPAKIT_RW_PARAMS_NUM_MAX)) {
		hwlog_err("%s: nbytes %d is 0 or >%u bytes, error\n", __func__,
			(int)nbytes, SMARTPAKIT_RW_PARAMS_NUM_MAX);
		ret = -EINVAL;
		goto err_out;
	}

	kern_buf = kmalloc(nbytes, GFP_KERNEL);
	if (kern_buf == NULL) {
		ret = -ENOMEM;
		goto err_out;
	}

	if (copy_from_user(kern_buf, (void __user *)buf, nbytes)) {
		ret = -EFAULT;
		goto err_out;
	}

	if ((pakit_priv->ioctl_ops == NULL) ||
		(pakit_priv->ioctl_ops->i2c_write == NULL)) {
		ret = -ECHILD;
		goto err_out;
	}

	ret = pakit_priv->ioctl_ops->i2c_write(pakit_priv->current_i2c_client,
		(char *)kern_buf, (int)nbytes);
	if (ret < 0) {
		hwlog_err("%s: i2c write error %d\n", __func__, ret);
		goto err_out;
	}
	mutex_unlock(&pakit_priv->i2c_ops_lock);

	kfree(kern_buf);
	return (ssize_t)nbytes;
err_out:
	mutex_unlock(&pakit_priv->i2c_ops_lock);
	if (kern_buf != NULL)
		kfree(kern_buf);

	return ret;
}

static void smartpakit_get_model_from_i2c_cfg(char *dst,
	struct smartpakit_priv *pakit_priv, unsigned int dst_len)
{
	char report_tmp[SMARTPAKIT_NAME_MAX] = {0};
	unsigned int model_len;
	unsigned int tmp_len;
	int i;

	if ((dst == NULL) || (pakit_priv == NULL) || (pakit_priv->pa_num == 0))
		return;

	for (i = 0; i < pakit_priv->pa_num; i++) {
		if (i < (pakit_priv->pa_num - 1)) {
			if (snprintf_s(report_tmp,
				(unsigned long)SMARTPAKIT_NAME_MAX,
				(unsigned long)SMARTPAKIT_NAME_MAX - 1,
				"%s_", pakit_priv->chip_model_list[i]) < 0)
				hwlog_err("%s: snprintf_s is failed\n", __func__);
		} else {
			if (snprintf_s(report_tmp,
				(unsigned long)SMARTPAKIT_NAME_MAX,
				(unsigned long)SMARTPAKIT_NAME_MAX - 1,
				"%s", pakit_priv->chip_model_list[i]) < 0)
				hwlog_err("%s: snprintf_s is failed\n", __func__);
		}

		model_len = dst_len - (strlen(dst) + 1);
		tmp_len = strlen(report_tmp);
		model_len = (tmp_len < model_len) ? tmp_len : model_len;
		strncat(dst, report_tmp, model_len);
	}
}

static void smartpakit_get_model_from_config_str(char *dst, const char *src,
	unsigned int dst_len)
{
	unsigned int model_len;

	if ((dst == NULL) || (src == NULL))
		return;

	model_len = strlen(src);
	model_len = (model_len < (dst_len - 1)) ? model_len : (dst_len - 1);
	if (strncpy_s(dst, dst_len, src, model_len) != EOK)
		hwlog_err("%s: strncpy_s is failed\n", __func__);
}

static int smartpakit_get_particular_pa_info(struct smartpakit_info *info,
	struct smartpakit_priv *pakit_priv)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;
	int ret = -EINVAL;
	unsigned int i;

	for (i = 0; i < pakit_priv->pa_num; i++) {
		if (pakit_priv->i2c_priv[i] != NULL) {
			i2c_priv = pakit_priv->i2c_priv[i];
			break;
		}
	}
	info->chip_vendor = SMARTPAKIT_VENDR_INVALID;

	switch (pakit_priv->algo_in) {
	case SMARTPAKIT_ALGO_IN_CODEC_DSP:
	case SMARTPAKIT_ALGO_IN_SOC_DSP:
	case SMARTPAKIT_ALGO_IN_SIMPLE_WITH_I2C:
		info->pa_num = pakit_priv->i2c_num;
		if (i2c_priv != NULL) {
			info->chip_vendor = i2c_priv->chip_vendor;
			smartpakit_get_model_from_i2c_cfg(info->chip_model,
				pakit_priv, SMARTPAKIT_NAME_MAX);
			ret = 0;
		}
		break;
	case SMARTPAKIT_ALGO_IN_WITH_DSP:
		info->pa_num = pakit_priv->i2c_num;
		if (i2c_priv != NULL) {
			info->chip_vendor = i2c_priv->chip_vendor;
			smartpakit_get_model_from_config_str(info->chip_model,
				i2c_priv->chip_model, SMARTPAKIT_NAME_MAX);
			ret = 0;
		}
		break;
	case SMARTPAKIT_ALGO_IN_SIMPLE:
		info->pa_num = pakit_priv->pa_num;
		info->chip_vendor = SMARTPAKIT_CHIP_VENDOR_OTHER;
		smartpakit_get_model_from_config_str(info->chip_model,
			pakit_priv->chip_model, SMARTPAKIT_NAME_MAX);
		ret = 0;
		break;
	case SMARTPAKIT_ALGO_IN_WITH_DSP_PLUGIN:
		info->pa_num = pakit_priv->pa_num;
		info->chip_vendor = pakit_priv->chip_vendor;
		smartpakit_get_model_from_config_str(info->chip_model,
			pakit_priv->chip_model, SMARTPAKIT_NAME_MAX);
		ret = 0;
		break;
	default:
		break;
	}

	if (ret < 0)
		hwlog_err("%s: end failed\n", __func__);
	return ret;
}

/* only for with dsp and only one vendor smartpa scene to set pa vendor info */
int smartpakit_set_info(struct smartpa_vendor_info *vendor_info)
{
	if (smartpakit_priv_data == NULL) {
		hwlog_err("%s: smartpakit_priv is NULL\n", __func__);
		return -EFAULT;
	}

	if ((vendor_info == NULL) ||
		(vendor_info->vendor >= SMARTPAKIT_CHIP_VENDOR_MAX) ||
		(vendor_info->chip_model == NULL)) {
		hwlog_err("%s: vendor_info error\n", __func__);
		return -EINVAL;
	}
#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s:vendor %u,chip_model %s\n", __func__,
		vendor_info->vendor, vendor_info->chip_model);
#endif
	smartpakit_priv_data->chip_vendor = vendor_info->vendor;
	smartpakit_priv_data->chip_model = vendor_info->chip_model;

	return 0;
}
EXPORT_SYMBOL_GPL(smartpakit_set_info);

int smartpakit_get_pa_info_from_dts_config(struct smartpakit_priv *pakit_priv,
	struct smartpakit_info *info)
{
	size_t cust_len;
	size_t special_name_len;
	size_t product_name_len;
	size_t chip_model_len;
	unsigned int i;

	if ((pakit_priv == NULL) || (info == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	/* common info */
	info->soc_platform    = pakit_priv->soc_platform;
	info->algo_in         = pakit_priv->algo_in;
	info->out_device      = pakit_priv->out_device;
	info->algo_delay_time = pakit_priv->algo_delay_time;
	info->param_version    = pakit_priv->param_version;
	info->cali_data_update_mode = pakit_priv->cali_data_update_mode;

	// smartpa cust info
	if (pakit_priv->cust != NULL) {
		cust_len = strlen(pakit_priv->cust);
		cust_len = (cust_len < SMARTPAKIT_NAME_MAX) ? cust_len : (SMARTPAKIT_NAME_MAX - 1);
		if (strncpy_s(info->cust, sizeof(info->cust), pakit_priv->cust, cust_len) != EOK)
			hwlog_err("%s: strncpy_s is failed\n", __func__);
	}

	if (pakit_priv->special_name_config != NULL) {
		special_name_len = strlen(pakit_priv->special_name_config);
		special_name_len = (special_name_len < SMARTPAKIT_NAME_MAX) ?
			special_name_len : (SMARTPAKIT_NAME_MAX - 1);
		if (strncpy_s(info->special_name_config, sizeof(info->special_name_config),
			pakit_priv->special_name_config, special_name_len) != EOK)
			hwlog_err("%s: strncpy_s is failed\n", __func__);
	}

	if (pakit_priv->product_name != NULL) {
		product_name_len = strlen(pakit_priv->product_name);
		product_name_len = (product_name_len < SMARTPAKIT_NAME_MAX) ?
			product_name_len : (SMARTPAKIT_NAME_MAX - 1);
		if (strncpy_s(info->product_name, sizeof(info->product_name),
			pakit_priv->product_name, product_name_len) != EOK)
			hwlog_err("%s: strncpy_s is failed\n", __func__);
	}

	for (i = 0; i < SMARTPAKIT_PA_ID_MAX; i++) {
		chip_model_len = strlen(pakit_priv->chip_model_list[i]);
		chip_model_len = (chip_model_len < SMARTPAKIT_NAME_MAX) ?
			chip_model_len : (SMARTPAKIT_NAME_MAX - 1);
		if (strncpy_s(info->chip_model_list[i], sizeof(info->chip_model_list[i]),
			pakit_priv->chip_model_list[i], chip_model_len) != EOK)
			hwlog_err("%s: strncpy_s is failed\n", __func__);
	}

	return smartpakit_get_particular_pa_info(info, pakit_priv);
}
EXPORT_SYMBOL_GPL(smartpakit_get_pa_info_from_dts_config);

static int smartpakit_get_hismartpa_cfg(struct smartpakit_priv *pakit_priv,
	struct smartpakit_info *info)
{
	struct smartpakit_i2c_priv *i2c_priv_p = NULL;
	unsigned int i;
	int ret = 0;

	for (i = 0; i < pakit_priv->pa_num; i++) {
		i2c_priv_p = pakit_priv->i2c_priv[i];
		if (i2c_priv_p == NULL)
			continue;
		ret += smartpakit_i2c_get_hismartpa_info(i2c_priv_p, &(info->otp[i]));
	}
	return ret;
}

static int smartpakit_get_info(struct smartpakit_priv *pakit_priv,
	void __user *arg)
{
	struct smartpakit_info info;

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: enter\n", __func__);
#endif
	if ((pakit_priv == NULL) || (arg == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}
#ifdef CONFIG_HUAWEI_DSM_AUDIO
	smartpakit_handle_i2c_probe_dsm_report(pakit_priv);
#endif
	if (memset_s(&info, sizeof(info), 0, sizeof(info)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);
	if (smartpakit_get_pa_info_from_dts_config(pakit_priv, &info) < 0)
		return -EINVAL;

	hwlog_info("%s: soc_platform=%u, algo_in=%u, out_device=0x%08x\n",
		__func__, info.soc_platform, info.algo_in, info.out_device);
	hwlog_info("%s: pa_num=%u, algo_delay_time=%u, chip_vendor=%u\n",
		__func__, info.pa_num, info.algo_delay_time, info.chip_vendor);
#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: chip_model = %s, special_name_config = %s\n",
		__func__, info.chip_model, info.special_name_config);
#endif

	if (smartpakit_get_hismartpa_cfg(pakit_priv, &info) < 0)
		return -EINVAL;

	if (info.pa_num != pakit_priv->pa_num)
		hwlog_info("%s: NOTICE I2C_NUM %u != PA_NUM %u\n", __func__,
			info.pa_num, pakit_priv->pa_num);

	if (copy_to_user((void *)arg, &info, sizeof(info))) {
		hwlog_err("%s: copy kit info to user failed\n", __func__);
		return -EIO;
	}

	pakit_priv->resume_sequence_permission_enable = true;
#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: enter end\n", __func__);
#endif
	return 0;
}

static int smartpakit_ctrl_prepare(struct smartpakit_priv *pakit_priv)
{
	unused(pakit_priv);
	return 0;
}

static int smartpakit_ctrl_unprepare(struct smartpakit_priv *pakit_priv)
{
	unused(pakit_priv);
	return 0;
}

static int smartpakit_get_single_irq_filter_cfgs(
	struct smartpakit_param_node *node, unsigned int node_num,
	int chip_id, struct list_head *cfg_list)
{
	int i;
	unsigned int node_type;
	struct smartpakit_reg_info reg_info;

	if ((node == NULL) || (node_num <= 0) || (cfg_list == NULL)) {
		hwlog_err("%s: invalid input argument\n", __func__);
		return -EINVAL;
	}

	if (memset_s(&reg_info, sizeof(reg_info), 0, sizeof(reg_info)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);
	for (i = 0; i < node_num; i++) {
		node_type = node[i].node_type;
		if (node_type == SMARTPAKIT_PARAM_NODE_TYPE_IRQ_FILTER) {
			reg_info.chip_id  = chip_id;
			reg_info.reg_addr = node[i].index;
			reg_info.mask     = node[i].mask;
			reg_info.info     = node[i].value;
			smartpakit_reg_info_add_list(&reg_info, cfg_list);
			hwlog_info("IRQ_FILTER,pa%d,reg 0x%x,m 0x%x, v 0x%x\n",
				chip_id, reg_info.reg_addr, reg_info.mask,
				reg_info.info);
		}
	}

	return 0;
}

static void smartpakit_get_irq_filter_cfgs(struct smartpakit_priv *pakit_priv)
{
	struct smartpakit_param_node *node = NULL;
	struct smartpakit_pa_ctl_sequence *sequence = NULL;
	int param_num_of_one_pa;
	int chip_id;
	int j;
	int ret;

	if ((pakit_priv == NULL) ||
		(pakit_priv->resume_sequence.node == NULL)) {
		hwlog_err("%s: irq configs invalid\n", __func__);
		return;
	}

	smartpakit_reg_info_del_list_all(&pakit_priv->irq_filter_list);

	sequence = &pakit_priv->resume_sequence;
	param_num_of_one_pa = sequence->param_num_of_one_pa;

	for (j = 0; j < sequence->pa_ctl_num; j++) {
		chip_id = sequence->pa_ctl_index[j];
		node = sequence->node + j * param_num_of_one_pa;
		ret = smartpakit_get_single_irq_filter_cfgs(node,
			param_num_of_one_pa, chip_id,
			&pakit_priv->irq_filter_list);
		if (ret < 0)
			hwlog_err("%s: get pa%d irq filter err\n", __func__,
				chip_id);
	}
}

static int smartpakit_get_reg_node_seq_for_multi_chips(
	struct smartpakit_pa_ctl_sequence *sequence,
	const void __user *arg, int compat_mode)
{
	int ret;

	ret = smartpakit_parse_params(sequence, arg, compat_mode);
	if (ret < 0)
		return ret;

	ret = smartpakit_split_sequence_for_multi_chips_ctl(sequence);
	if (ret < 0)
		smartpakit_reset_pa_ctl_sequence(sequence);

	return ret;
}

static int smartpakit_init_chips_with_i2c(struct smartpakit_priv *pakit_priv,
	const void __user *arg, int compat_mode, unsigned int id)
{
	struct smartpakit_pa_ctl_sequence *sequence = NULL;
	int ret;
	int i;

	unused(id);
#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: enter\n", __func__);
#endif
	if ((pakit_priv == NULL) || (arg == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}
	/* avoid reinit repeatedly */
	if (!pakit_priv->resume_sequence_permission_enable) {
		hwlog_err("%s: resume permission disable, skip\n", __func__);
		return -EINVAL;
	}

	pakit_priv->resume_sequence_permission_enable = false;

	/* clear power on sequence to avoid audio server died */
	for (i = 0; i < SMARTPAKIT_PA_ID_MAX; i++) {
		sequence = &pakit_priv->poweron_seq[i];
		smartpakit_reset_pa_ctl_sequence(sequence);
	}
	mutex_lock(&pakit_priv->i2c_ops_lock);
	mutex_lock(&pakit_priv->resume_sequence_lock);

	sequence = &pakit_priv->resume_sequence;
	smartpakit_reset_pa_ctl_sequence(sequence);
	ret = smartpakit_get_reg_node_seq_for_multi_chips(sequence, arg,
		compat_mode);
	if (ret < 0) {
		mutex_unlock(&pakit_priv->resume_sequence_lock);
		mutex_unlock(&pakit_priv->i2c_ops_lock);
		hwlog_err("%s: get legal init_regs failed\n", __func__);
		return -EINVAL;
	}

	smartpakit_get_irq_filter_cfgs(pakit_priv);

	/* init chip regs */
	ret = smartpakit_write_regs_to_multi_chips(pakit_priv, sequence);
	mutex_unlock(&pakit_priv->resume_sequence_lock);
	mutex_unlock(&pakit_priv->i2c_ops_lock);

	hwlog_info("%s: enter end, ret=%d\n", __func__, ret);
	return ret;
}

/* call function in i2c file and that function copy form user */
static int smartpakit_ctrl_read_regs(struct smartpakit_priv *pakit_priv,
	void __user *arg, unsigned int id)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;
	int ret;
	unsigned int chip_id = 0;
	struct smartpakit_get_param reg;

	if ((pakit_priv == NULL) || (arg == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&pakit_priv->i2c_ops_lock);
	if (pakit_priv->pa_num >= (id + 1))
		chip_id = id;
	i2c_priv = pakit_priv->i2c_priv[chip_id];

	if (memset_s(&reg, sizeof(reg), 0, sizeof(reg)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);
	if (copy_from_user(&reg, (void *)arg, sizeof(reg))) {
		hwlog_err("%s: read reg copy_from_user failed\n", __func__);
		ret = EFAULT;
		goto err_out;
	}

	if ((pakit_priv->ioctl_ops == NULL) ||
		(pakit_priv->ioctl_ops->read_regs == NULL)) {
		hwlog_err("%s: ioctl_ops or read_regs is NULL\n", __func__);
		ret = -ECHILD;
		goto err_out;
	}

	ret = pakit_priv->ioctl_ops->read_regs(i2c_priv, &reg);
	if (ret < 0)
		goto err_out;

	if (copy_to_user((void *)arg, &reg, sizeof(reg))) {
		hwlog_err("%s: send reg value to user failed\n", __func__);
		ret = -EFAULT;
	}
err_out:
	mutex_unlock(&pakit_priv->i2c_ops_lock);
	return ret;
}

static int pakit_write_regs_to_single_pa(struct smartpakit_priv *pakit_priv,
	const void __user *arg, int compat_mode, unsigned int id)
{
	struct smartpakit_i2c_priv *i2c_priv = NULL;
	unsigned int chip_id = 0;
	int ret;
	struct smartpakit_pa_ctl_sequence sequence;

	if (pakit_priv->pa_num >= (id + 1))
		chip_id = id;

	mutex_lock(&pakit_priv->i2c_ops_lock);
	if (memset_s(&sequence, sizeof(sequence), 0, sizeof(sequence)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);
	ret = smartpakit_parse_params(&sequence, arg, compat_mode);
	if (ret < 0)
		goto err_out;

	if ((pakit_priv->ioctl_ops == NULL) ||
		(pakit_priv->ioctl_ops->write_regs == NULL)) {
		hwlog_err("%s: write_regs is invalid\n", __func__);
		ret = -ECHILD;
		goto err_out;
	}

	i2c_priv = pakit_priv->i2c_priv[chip_id];
	ret = pakit_priv->ioctl_ops->write_regs(i2c_priv, sequence.param_num,
		sequence.node);
	if (ret < 0)
		goto err_out;
	sequence.pa_ctl_index[0] = chip_id;
	sequence.pa_ctl_num = 1;
	ret = smartpakit_set_poweron_regs(pakit_priv, &sequence);
err_out:
	smartpakit_kfree_ops(sequence.node);
	mutex_unlock(&pakit_priv->i2c_ops_lock);
	hwlog_debug("%s: end, ret=%d\n", __func__, ret);
	return ret;
}

static void smartpakit_i2c_error_reset_and_resume_chip(struct smartpakit_priv *pakit_priv)
{
	if (pakit_priv->need_reset_resume_chip) {
		pakit_priv->need_reset_resume_chip = false;
		pakit_priv->i2c_errno = 0;
		hwlog_info("pa i2c error need reset chip\n");
		smartpakit_hw_reset_chips(pakit_priv);

		/* init and resume power */
		smartpakit_resume_chips(pakit_priv);
		if (pakit_priv->i2c_errno == 0) {
			pakit_priv->need_reset_resume_chip = true;
			hwlog_info("pa i2c resume success\n");
		}
	}
}

static int smartpakit_ctrl_write_regs(struct smartpakit_priv *pakit_priv,
	const void __user *arg, int compat_mode, unsigned int id)
{
	int ret;
	struct smartpakit_pa_ctl_sequence sequence;

	hwlog_debug("%s: enter id=0x%x\n", __func__, id);
	if ((pakit_priv == NULL) || (arg == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (id != SMARTPAKIT_PA_ID_ALL)
		return pakit_write_regs_to_single_pa(pakit_priv, arg,
			compat_mode, id);

	mutex_lock(&pakit_priv->i2c_ops_lock);
	if (memset_s(&sequence, sizeof(sequence), 0, sizeof(sequence)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);
	ret = smartpakit_get_reg_node_seq_for_multi_chips(&sequence,
		arg, compat_mode);
	if (ret < 0)
		goto err_out;

	ret = smartpakit_write_regs_to_multi_chips(pakit_priv, &sequence);
	if (ret < 0)
		goto err_out;
	if (pakit_priv->i2c_errno == -EREMOTEIO)
		smartpakit_i2c_error_reset_and_resume_chip(pakit_priv);

	ret = smartpakit_set_poweron_regs(pakit_priv, &sequence);
err_out:
	smartpakit_kfree_ops(sequence.node);
	mutex_unlock(&pakit_priv->i2c_ops_lock);
	hwlog_debug("%s: end, ret=%d\n", __func__, ret);
	return ret;
}

int smartpakit_gpio_node_ctl(void *priv, struct smartpakit_param_node *reg)
{
	struct smartpakit_priv *pakit_priv = NULL;
	int gpio;

	pakit_priv = priv;
	if ((pakit_priv == NULL) || (pakit_priv->switch_ctl == NULL)) {
		hwlog_info("%s: switch_ctl is invalid\n", __func__);
		return -EINVAL;
	}
	if (reg == NULL) {
		hwlog_err("%s: param node reg is invalid\n", __func__);
		return -EINVAL;
	}

	if (reg->index >= pakit_priv->switch_num) {
		hwlog_err("%s: Invalid arg, gpio index %u>= switch num %u\n",
			__func__, reg->index, pakit_priv->switch_num);
		return -EINVAL;
	}

	gpio = pakit_priv->switch_ctl[reg->index].gpio;
	gpio_direction_output((unsigned int)gpio, (int)reg->value);
	if (reg->delay > SMARTPAKIT_DELAY_US_TO_MS)
		msleep(reg->delay / SMARTPAKIT_DELAY_US_TO_MS);
	else
		udelay(reg->delay);

	hwlog_info("%s: gpio %d set %u, delay %u\n", __func__, gpio,
		reg->value, reg->delay);

	return 0;
}
EXPORT_SYMBOL_GPL(smartpakit_gpio_node_ctl);

static int smartpakit_simple_pa_gpio_node_ctl(
	struct smartpakit_priv *pakit_priv, struct smartpakit_param_node *node)
{
	if (node == NULL) {
		hwlog_err("%s: node is NULL\n", __func__);
		return -EINVAL;
	}

	/*
	 * for simple pa, node type is:
	 * 1. gpio node
	 * 2. delay time node
	 */
	if (node->node_type == SMARTPAKIT_PARAM_NODE_TYPE_DELAY) {
		if (node->delay > SMARTPAKIT_DELAY_US_TO_MS)
			msleep(node->delay / SMARTPAKIT_DELAY_US_TO_MS);
		else
			udelay(node->delay);
		return 0;
	}

	/* gpio node type */
	return smartpakit_gpio_node_ctl((void *)pakit_priv, node);
}

static int smartpakit_ctrl_simple_pa(struct smartpakit_priv *pakit_priv,
	const void __user *arg, int compat_mode, unsigned int id)
{
	struct smartpakit_pa_ctl_sequence sequence;
	struct smartpakit_param_node *node = NULL;
	int ret;
	int i;

	unused(id);
	hwlog_info("%s: enter\n", __func__);
	if ((pakit_priv == NULL) || (pakit_priv->switch_ctl == NULL) ||
		(arg == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (memset_s(&sequence, sizeof(sequence), 0, sizeof(sequence)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);
	ret = smartpakit_parse_params(&sequence, arg, compat_mode);
	if (ret < 0) {
		hwlog_err("%s: parse gpio_state failed\n", __func__);
		goto err_out;
	}

	for (i = 0; i < (int)sequence.param_num; i++) {
		node = &sequence.node[i];
		ret = smartpakit_simple_pa_gpio_node_ctl(pakit_priv, node);
		if (ret < 0) {
			hwlog_err("%s: node %d ops err\n", __func__, i);
			goto err_out;
		}
	}

err_out:
	smartpakit_kfree_ops(sequence.node);
	return ret;
}

static unsigned int smartpakit_get_ctl_pa_id_by(unsigned int cmd)
{
	unsigned int id = SMARTPAKIT_PA_ID_MAX;

	switch (cmd) {
	case SMARTPAKIT_R_PRIL:
	case SMARTPAKIT_W_PRIL:
		id = SMARTPAKIT_PA_ID_PRIL;
		break;
	case SMARTPAKIT_R_PRIR:
	case SMARTPAKIT_W_PRIR:
		id = SMARTPAKIT_PA_ID_PRIR;
		break;
	case SMARTPAKIT_R_SECL:
	case SMARTPAKIT_W_SECL:
		id = SMARTPAKIT_PA_ID_SECL;
		break;
	case SMARTPAKIT_R_SECR:
	case SMARTPAKIT_W_SECR:
		id = SMARTPAKIT_PA_ID_SECR;
		break;
	case SMARTPAKIT_W_ALL:
		id = SMARTPAKIT_PA_ID_ALL;
		break;
	default:
		hwlog_err("%s: get pa id err\n", __func__);
		break;
	}
	return id;
}

static int set_pa_check_loop(int state)
{
	hwlog_info("%s: enter\n", __func__);
	if (!smartpakit_priv_data->timer) {
		hwlog_err("%s: timer is null\n", __func__);
		smartpakit_priv_data->loop_state = false;
		return -1;
	}

	if (state == 1) {
		smartpakit_priv_data->loop_state = true;
		if (!smartpakit_priv_data->timer_state) {
			smartpakit_priv_data->timer_state = true;
			smartpakit_priv_data->timer->expires =
				jiffies + jiffies_to_msecs(2 * HZ); // 8S loop
			add_timer(smartpakit_priv_data->timer);
		}
	} else {
		smartpakit_priv_data->loop_state = false;
	}

	return 0;
}

static int smartpakit_check(struct smartpakit_priv *pakit_priv,
	void __user *arg)
{
	int ret;
	int pa_check;

	if (pakit_priv->need_pa_check == 0) {
		hwlog_info("%s: do not need pa check\n", __func__);
		return -1;
	}

	ret = copy_from_user(&pa_check, arg, sizeof(int));
	if (ret != 0) {
		hwlog_err("%s: get pa_check state fail\n", __func__);
		return ret;
	}

	hwlog_info("%s: pa_check = %d\n", __func__, pa_check);

	ret = set_pa_check_loop(pa_check);
	if (ret != 0) {
		hwlog_err("%s: set pa check loop fail\n", __func__);
		return ret;
	}

	return 0;
}

static int smartpakit_ctrl_do_ioctl(struct file *file, unsigned int cmd,
	void __user *arg, int compat_mode)
{
	struct smartpakit_priv *pakit_priv = NULL;
	int ret;
	unsigned int id;

	if ((cmd != I2C_SLAVE) && (cmd != I2C_SLAVE_FORCE))
		hwlog_debug("%s: enter, cmd 0x%x, compat_mode=%d\n", __func__,
			cmd, compat_mode);

	if ((file == NULL) || (file->private_data == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}
	pakit_priv = file->private_data;
	mutex_lock(&pakit_priv->do_ioctl_lock);

	switch (cmd) {
	/*
	 * NOTE:
	 * 1. smartpa with dsp, ex. tfa9895
	 * 2. climax tool for NXP
	 */
	case I2C_SLAVE:
	case I2C_SLAVE_FORCE:
		ret = smartpakit_get_current_i2c_client(pakit_priv,
			(unsigned short)(uintptr_t)arg);
		break;
	case SMARTPAKIT_R_PRIL:
	case SMARTPAKIT_R_PRIR:
	case SMARTPAKIT_R_SECL:
	case SMARTPAKIT_R_SECR:
		id = smartpakit_get_ctl_pa_id_by(cmd);
		ret = smartpakit_ctrl_read_regs(pakit_priv, arg, id);
		break;
	case SMARTPAKIT_HW_RESET:
		mutex_lock(&pakit_priv->hw_reset_lock);
		ret = smartpakit_hw_reset_chips(pakit_priv);
		mutex_unlock(&pakit_priv->hw_reset_lock);
		break;
	case SMARTPAKIT_REGS_DUMP:
		mutex_lock(&pakit_priv->dump_regs_lock);
		ret = smartpakit_dump_chips(pakit_priv);
		mutex_unlock(&pakit_priv->dump_regs_lock);
		break;
	/* write cmd */
	case SMARTPAKIT_INIT:
		if (smartpakit_ctrl_init_chips == NULL) {
			hwlog_err("%s: init func invalid\n", __func__);
			ret = -EINVAL;
			break;
		}
		ret = smartpakit_ctrl_init_chips(pakit_priv, arg,
			compat_mode, SMARTPAKIT_PA_ID_ALL);
		break;
	case SMARTPAKIT_W_ALL:
	case SMARTPAKIT_W_PRIL:
	case SMARTPAKIT_W_PRIR:
	case SMARTPAKIT_W_SECL:
	case SMARTPAKIT_W_SECR:
		id = smartpakit_get_ctl_pa_id_by(cmd);
		if (smartpakit_ctrl_write_chips == NULL) {
			hwlog_err("%s: write func invalid\n", __func__);
			ret = -EINVAL;
			break;
		}
		ret = smartpakit_ctrl_write_chips(pakit_priv, arg,
			compat_mode, id);
		break;
	case SMARTPAKIT_GET_INFO:
		ret = smartpakit_get_info(pakit_priv, arg);
		break;
	case SMARTPAKIT_HW_PREPARE:
		ret = smartpakit_ctrl_prepare(pakit_priv);
		break;
	case SMARTPAKIT_HW_UNPREPARE:
		ret = smartpakit_ctrl_unprepare(pakit_priv);
		break;
	case SMARTPAKIT_CHECK:
		ret = smartpakit_check(pakit_priv, arg);
		break;
	default:
		hwlog_err("%s: not support cmd = 0x%x\n", __func__, cmd);
		ret = -EIO;
		break;
	}

	mutex_unlock(&pakit_priv->do_ioctl_lock);
	if ((cmd != I2C_SLAVE) && (cmd != I2C_SLAVE_FORCE))
		hwlog_info("%s: end, cmd:0x%x ret=%d\n", __func__, cmd, ret);

	return ret;
}

static long smartpakit_ctrl_ioctl(struct file *file, unsigned int command,
	unsigned long arg)
{
	return smartpakit_ctrl_do_ioctl(file, command, (void __user *)(uintptr_t)arg, 0);
}

#ifdef CONFIG_COMPAT
static long smartpakit_ctrl_ioctl_compat(struct file *file,
	unsigned int command, unsigned long arg)
{
	switch (command) {
	case SMARTPAKIT_INIT_COMPAT:
		command = SMARTPAKIT_INIT;
		break;
	case SMARTPAKIT_W_ALL_COMPAT:
		command = SMARTPAKIT_W_ALL;
		break;
	case SMARTPAKIT_W_PRIL_COMPAT:
		command = SMARTPAKIT_W_PRIL;
		break;
	case SMARTPAKIT_W_PRIR_COMPAT:
		command = SMARTPAKIT_W_PRIR;
		break;
	case SMARTPAKIT_W_SECL_COMPAT:
		command = SMARTPAKIT_W_SECL;
		break;
	case SMARTPAKIT_W_SECR_COMPAT:
		command = SMARTPAKIT_W_SECR;
		break;
	default:
		break;
	}
	return smartpakit_ctrl_do_ioctl(file, command,
		compat_ptr((unsigned int)arg), 1);
}
#else
#define smartpakit_ctrl_ioctl_compat NULL /* function pointer */
#endif

static const struct file_operations smartpakit_ctrl_fops = {
	.owner          = THIS_MODULE,
	.open           = smartpakit_ctrl_open,
	.release        = smartpakit_ctrl_release,
	.read           = smartpakit_ctrl_read,
	.write          = smartpakit_ctrl_write,
	.unlocked_ioctl = smartpakit_ctrl_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = smartpakit_ctrl_ioctl_compat,
#endif
};

static struct miscdevice smartpakit_ctrl_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "smartpakit",
	.fops  = &smartpakit_ctrl_fops,
};

int smartpakit_get_prop_of_u32_array(struct device_node *node,
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
		goto err_out;
	}

	*value = array;
	*num = count;
	return 0;

err_out:
	smartpakit_kfree_ops(array);
	return ret;
}

int smartpakit_get_prop_of_u32_type(struct device_node *node,
	const char *key_str, unsigned int *type, bool is_requisite)
{
	int ret = 0;

	if ((node == NULL) || (key_str == NULL) || (type == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (!of_property_read_bool(node, key_str)) {
		if (is_requisite)
			ret = -EINVAL;
		hwlog_debug("%s: %s is not config in dts\n", __func__, key_str);
		return ret;
	}

	ret = of_property_read_u32(node, key_str, type);
	if (ret < 0)
		hwlog_err("%s: get %s error\n", __func__, key_str);

	return ret;
}

int smartpakit_get_prop_of_str_type(struct device_node *node,
	const char *key_str, const char **dst)
{
	int ret = 0;

	if ((node == NULL) || (key_str == NULL) || (dst == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}
	if (of_property_read_bool(node, key_str)) {
		ret = of_property_read_string(node, key_str, dst);
		if (ret < 0) {
			hwlog_err("%s: get %s failed\n", __func__, key_str);
			return ret;
		}
#ifndef CONFIG_FINAL_RELEASE
		hwlog_debug("%s: %s=%s\n", __func__, key_str, *dst);
#endif
	} else {
		hwlog_debug("%s: %s not existed, skip\n", __func__, key_str);
	}

	return ret;
}

static void smartpakit_simple_pa_power_off(struct platform_device *pdev)
{
	struct smartpakit_priv *pakit_priv = NULL;
	struct smartpakit_switch_node *ctl = NULL;
	int i;

	if (pdev == NULL) {
		hwlog_err("%s: invalid arg\n", __func__);
		return;
	}

	pakit_priv = platform_get_drvdata(pdev);
	if ((pakit_priv == NULL) || (pakit_priv->switch_ctl == NULL) ||
		(pakit_priv->algo_in != SMARTPAKIT_ALGO_IN_SIMPLE)) {
		hwlog_err("%s: pakit_priv invalid arg\n", __func__);
		return;
	}
	ctl = pakit_priv->switch_ctl;

	for (i = 0; i < (int)pakit_priv->switch_num; i++) {
		/* set gpio default state, for AW8737 shutdown sequence */
		gpio_direction_output((unsigned int)ctl[i].gpio, ctl[i].state);
		mdelay(1);
	}
}

static int smartpakit_get_prop_num(struct device_node *node, const char *str)
{
	int i = 0;
	struct property *pp = NULL;

	for_each_property_of_node(node, pp) {
		if (strncmp(pp->name, str, strlen(str)) != 0)
			continue;

		i++;
	}
	return i;
}

static int smartpakit_save_switch_ctl_delay_value(struct device_node *node,
	struct smartpakit_switch_node *ctl, unsigned int num)
{
	const char *gpio_delay_default_str = "gpio_delay";
	int *gpio_delay = NULL;
	int count = 0;
	int ret;
	int i;

	if (!of_property_read_bool(node, gpio_delay_default_str)) {
		hwlog_info("%s: default prop not existed, skip\n", __func__);
		return 0;
	}

	ret = smartpakit_get_prop_of_u32_array(node, gpio_delay_default_str,
		(u32 **)&gpio_delay, &count);
	if ((ret < 0) || (count <= 0))
		return -EINVAL;

	if (count != (int)num) {
		hwlog_err("%s: delay count %d != %u\n", __func__, count, num);
		smartpakit_kfree_ops(gpio_delay);
		return -EFAULT;
	}

	for (i = 0; i < (int)num; i++)
		ctl[i].delay = gpio_delay[i];

	return 0;
}

static int smartpakit_save_switch_ctl_default_value(struct device_node *node,
	struct smartpakit_switch_node *ctl, unsigned int num)
{
	const char *gpio_state_default_str = "gpio_state_default";
	int *gpio_state = NULL;
	int count = 0;
	int ret;
	int i;

	if (!of_property_read_bool(node, gpio_state_default_str)) {
		hwlog_info("%s: default prop not existed, skip\n", __func__);
		return 0;
	}

	ret = smartpakit_get_prop_of_u32_array(node, gpio_state_default_str,
		(u32 **)&gpio_state, &count);
	if ((ret < 0) || (count <= 0))
		return -EINVAL;

	if (count != (int)num) {
		hwlog_err("%s: state count %d != %u\n", __func__, count, num);
		smartpakit_kfree_ops(gpio_state);
		return -EFAULT;
	}

	for (i = 0; i < (int)num; i++)
		ctl[i].state = gpio_state[i];

	return 0;
}

static int smartpakit_save_switch_ctl_node(struct device_node *node,
	struct smartpakit_switch_node **switch_ctl, unsigned int num)
{
	struct smartpakit_switch_node *ctl = NULL;
	struct property *pp = NULL;
	const char *gpio_rst_str = "gpio_reset";
	int i = 0;
	int ret;
	unsigned int switch_node_size;

	switch_node_size = sizeof(*ctl) * num;
	ctl = kzalloc(switch_node_size, GFP_KERNEL);
	if (ctl == NULL)
		return -ENOMEM;

	for_each_property_of_node(node, pp) {
		if (strncmp(pp->name, gpio_rst_str, strlen(gpio_rst_str)) != 0)
			continue;

		ret = snprintf_s(ctl[i].name, (unsigned long)SMARTPAKIT_NAME_MAX,
			(unsigned long)SMARTPAKIT_NAME_MAX - 1, "smartpakit_gpio_reset_%d", i);
		if (ret < 0) {
			hwlog_err("%s: set gpio name failed\n", __func__);
			ret = -EFAULT;
			goto err_out;
		}

		ctl[i].gpio = of_get_named_gpio(node, pp->name, 0);
		if (ctl[i].gpio < 0) {
			hwlog_info("%s: %s of_get_named_gpio failed, %d\n",
				__func__, pp->name, ctl[i].gpio);
			ret = of_property_read_u32(node, pp->name,
				(u32 *)&ctl[i].gpio);
			if (ret < 0) {
				hwlog_err("%s: %s read gpio prop failed, %d\n",
					__func__, pp->name, ret);
				ret = -EFAULT;
				goto err_out;
			}
		}

		ctl[i].state = -1;
		i++;
	}
	*switch_ctl = ctl;
	return 0;
err_out:
	kfree(ctl);
	return ret;
}

static void smartpakit_free_switch_ctl_gpio(unsigned int ctl_num,
	struct smartpakit_switch_node *ctl)
{
	int i;

	if ((ctl_num == 0) || (ctl == NULL))
		return;

	for (i = 0; i < (int)ctl_num; i++) {
		if (ctl[i].gpio > 0) {
			gpio_free((unsigned int)ctl[i].gpio);
			ctl[i].gpio = 0;
		}
	}

	kfree(ctl);
	ctl = NULL;
}

static int smartpakit_init_switch_ctl_gpio(struct smartpakit_priv *pakit_priv)
{
	struct smartpakit_switch_node *ctl = NULL;
	int i;
	int ret;

	ctl = pakit_priv->switch_ctl;
	if ((pakit_priv->switch_num == 0) || (ctl == NULL)) {
		if (pakit_priv->algo_in == SMARTPAKIT_ALGO_IN_SIMPLE)
			return -EINVAL;

		return 0;
	}

	for (i = 0; i < (int)pakit_priv->switch_num; i++) {
		hwlog_info("%s: %d node gpio %d, state %d\n", __func__, i,
			ctl[i].gpio, ctl[i].state);
		if (ctl[i].gpio == 0) {
			hwlog_info("%s: gpio is 0, return\n", __func__);
			ret = 0;
			goto err_out;
		}
		if (gpio_request((unsigned int)ctl[i].gpio, ctl[i].name) < 0) {
			hwlog_err("%s: request gpio %d failed\n",
				__func__, ctl[i].gpio);
			ctl[i].gpio = 0;
			ret = -EFAULT;
			goto err_out;
		}

		if (ctl[i].state < 0)
			continue;
		gpio_direction_output((unsigned int)ctl[i].gpio, ctl[i].state);

		if (ctl[i].delay > 0)
			mdelay(ctl[i].delay);
	}
	return 0;
err_out:
	pakit_priv->switch_num = 0;
	smartpakit_kfree_ops(pakit_priv->switch_ctl);
	return ret;
}

static int smartpakit_parse_switch_ctl_gpio(struct device_node *node,
	struct smartpakit_priv *pakit_priv)
{
	const char *gpio_reset_str = "gpio_reset";
	int ret;
	unsigned int gpio_num;

	gpio_num = (unsigned int)smartpakit_get_prop_num(node, gpio_reset_str);
	if (gpio_num == 0) {
		hwlog_err("%s: switch_ctl gpio num is 0\n", __func__);
		return -EINVAL;
	}
	ret = smartpakit_save_switch_ctl_node(node, &pakit_priv->switch_ctl,
		gpio_num);
	if (ret < 0) {
		hwlog_err("%s: save switch ctl node err\n", __func__);
		goto err_out;
	}

	ret = smartpakit_save_switch_ctl_default_value(node,
		pakit_priv->switch_ctl, gpio_num);
	if (ret < 0) {
		hwlog_err("%s: save default value err\n", __func__);
		goto err_out;
	}
	ret = smartpakit_save_switch_ctl_delay_value(node,
		pakit_priv->switch_ctl, gpio_num);
	if (ret < 0) {
		hwlog_err("%s: save delay value err\n", __func__);
		goto err_out;
	}
	pakit_priv->switch_num = gpio_num;
	return 0;
err_out:
	smartpakit_kfree_ops(pakit_priv->switch_ctl);
	pakit_priv->switch_num = 0;
	return ret;
}

static int smartpakit_parse_dt_switch_ctl_cfg(struct device *dev,
	struct smartpakit_priv *pakit_priv)
{
	const char *switch_ctl_str = "switch_ctl";
	struct device_node *node = NULL;
	int ret;

	node = of_get_child_by_name(dev->of_node, switch_ctl_str);
	if (node == NULL) {
		hwlog_debug("%s: node not existed, skip\n", __func__);
		return 0;
	}
	ret = smartpakit_parse_switch_ctl_gpio(node, pakit_priv);
	if (ret < 0)
		hwlog_err("%s: get switch ctl config err\n", __func__);

	return ret;
}

static int smartpakit_parse_dt_irq_pinctrl(struct device *dev,
	struct smartpakit_priv *pakit_priv)
{
	const char *smartpakit_irq_pinctrl = "smartpakit_irq_pinctrl";
	struct pinctrl *pinctrl = NULL;
	struct pinctrl_state *pin_default_state = NULL;
	const char *pin_ctrl_state_name = "default";
	bool support_irq_pinctrl = false;
	int ret = 0;

	support_irq_pinctrl = of_property_read_bool(dev->of_node,
					smartpakit_irq_pinctrl);
	if (support_irq_pinctrl == false) {
		hwlog_debug("%s: node:%s not existed, skip\n",
			__func__, smartpakit_irq_pinctrl);
		goto exit_parse_dt_irq_pinctrl;
	}

	pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR_OR_NULL(pinctrl)) {
		hwlog_err("%s: get pinctrl fail %d\n",
			__func__, IS_ERR(pinctrl));
		ret = -EINVAL;
		goto exit_parse_dt_irq_pinctrl;
	}

	pin_default_state = pinctrl_lookup_state(pinctrl, pin_ctrl_state_name);
	if (IS_ERR_OR_NULL(pin_default_state)) {
		ret = -EINVAL;
		hwlog_err("%s: pinctrl_lookup_state fail %d\n",
			__func__, IS_ERR(pin_default_state));
		goto exit_parse_dt_irq_pinctrl;
	}

	if (pinctrl_select_state(pinctrl, pin_default_state) != 0) {
		ret = -EINVAL;
		hwlog_err("%s: pinctrl_select_state fail\n", __func__);
		goto exit_parse_dt_irq_pinctrl;
	}

exit_parse_dt_irq_pinctrl:
	return ret;
}

static int smartpakit_parse_dt_misc_rw_permission(struct device *dev,
	struct smartpakit_priv *pakit_priv)
{
	const char *misc_rw_permission_str = "misc_rw_permission_enable";

	if (of_property_read_bool(dev->of_node, misc_rw_permission_str)) {
		pakit_priv->misc_rw_permission_enable = true;
		hwlog_info("%s: prop existed\n", __func__);
	} else {
		hwlog_debug("%s: prop not existed\n", __func__);
	}

#ifdef SMARTPAKIT_MISC_RW_PERMISSION_ENABLE
	pakit_priv->misc_rw_permission_enable = true;
	hwlog_info("%s: config MISC_RW_PERMISSION_ENABLE in eng build\n",
		__func__);
#endif
	return 0;
}

static int smartpakit_parse_dt_i2c_addr_type(struct device *dev,
	struct smartpakit_priv *pakit_priv)
{
	const char *type_str = "misc_i2c_use_pseudo_addr";

	if (of_property_read_bool(dev->of_node, type_str)) {
		pakit_priv->misc_i2c_use_pseudo_addr = true;
		hwlog_info("%s: %s prop existed\n", __func__, type_str);
	} else {
		hwlog_debug("%s: %s not existed\n", __func__, type_str);
	}
	return 0;
}

static int smartpakit_check_dt_info_valid(struct smartpakit_priv *pakit_priv)
{
	int ret = 0;

	if (pakit_priv->soc_platform >= SMARTPAKIT_SOC_PLATFORM_MAX) {
		hwlog_err("%s: soc_platform %u invalid\n", __func__,
			pakit_priv->soc_platform);
		ret = -EINVAL;
	}

	if (pakit_priv->algo_in >= SMARTPAKIT_ALGO_IN_MAX) {
		hwlog_err("%s: algo_in %u invalid\n", __func__,
			pakit_priv->algo_in);
		ret = -EINVAL;
	}

	if (pakit_priv->chip_vendor >= SMARTPAKIT_CHIP_VENDOR_MAX) {
		hwlog_err("%s: chip_vendor %u invalid\n", __func__,
			pakit_priv->chip_vendor);
		ret = -EINVAL;
	}
	return ret;
}

static int smartpakit_parse_out_device_info(struct device *dev,
	struct smartpakit_priv *pakit_priv)
{
	const char *out_device_str = "out_device";
	u32 *out_device = NULL;
	int count = 0;
	int i;
	int ret;
	unsigned int offset;

	ret = smartpakit_get_prop_of_u32_array(dev->of_node, out_device_str,
		&out_device, &count);
	if (ret < 0) {
		hwlog_err("%s: get out device prop failed\n", __func__);
		return -EFAULT;
	}

	if ((count <= 0) || (count > SMARTPAKIT_PA_ID_MAX)) {
		hwlog_err("%s: get pa_num failed %d, or pa_num %d>%d\n",
			__func__, count, count, SMARTPAKIT_PA_ID_MAX);
		ret = -EFAULT;
		goto err_out;
	}

	pakit_priv->out_device = 0;
	for (i = 0; i < count; i++) {
		if (out_device[i] >= SMARTPAKIT_OUT_DEVICE_MAX) {
			hwlog_err("%s: out_device error %u>%d\n", __func__,
				out_device[i], SMARTPAKIT_OUT_DEVICE_MAX);
			ret = -EFAULT;
			goto err_out;
		}
		offset = i * SMARTPAKIT_PA_OUT_DEVICE_SHIFT;
		pakit_priv->out_device |= (out_device[i] << offset);
	}
	pakit_priv->pa_num = count;
#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: pa_num %u, out_device 0x%08x\n", __func__,
		pakit_priv->pa_num, pakit_priv->out_device);
#endif
err_out:
	smartpakit_kfree_ops(out_device);
	return ret;
}

static int smartpakit_parse_dt_info(struct platform_device *pdev,
	struct smartpakit_priv *pakit_priv)
{
	const char *soc_platform_str = "soc_platform";
	const char *algo_in_str      = "algo_in";
	const char *algo_delay_str   = "algo_delay";
	const char *chip_vendor_str  = "chip_vendor";
	const char *param_version_str = "param_version";
	const char *chip_model_str   = "chip_model";
	const char *cust_str         = "cust";
	const char *special_cfg_str  = "special_name_config";
	const char *product_name_str  = "product_name";
	const char *cali_data_update_mode_str = "cali_data_update_mode";
	const char *need_pa_check_str = "need_pa_check";
	int ret;

	if ((pdev == NULL) || (pakit_priv == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	ret = smartpakit_get_prop_of_u32_type(pdev->dev.of_node,
		soc_platform_str, &pakit_priv->soc_platform, true);
	ret += smartpakit_get_prop_of_u32_type(pdev->dev.of_node, algo_in_str,
		&pakit_priv->algo_in, true);
	ret += smartpakit_get_prop_of_u32_type(pdev->dev.of_node,
		algo_delay_str, &pakit_priv->algo_delay_time, false);
	ret += smartpakit_get_prop_of_u32_type(pdev->dev.of_node,
		chip_vendor_str, &pakit_priv->chip_vendor, false);
	ret += smartpakit_get_prop_of_u32_type(pdev->dev.of_node,
		param_version_str, &pakit_priv->param_version, false);
	ret += smartpakit_get_prop_of_u32_type(pdev->dev.of_node,
		cali_data_update_mode_str,
		&pakit_priv->cali_data_update_mode, false);

	ret += smartpakit_get_prop_of_str_type(pdev->dev.of_node,
		chip_model_str, &pakit_priv->chip_model);
	ret += smartpakit_get_prop_of_str_type(pdev->dev.of_node,
		cust_str, &pakit_priv->cust);
	ret += smartpakit_get_prop_of_str_type(pdev->dev.of_node,
		special_cfg_str, &pakit_priv->special_name_config);
	ret += smartpakit_get_prop_of_str_type(pdev->dev.of_node,
		product_name_str, &pakit_priv->product_name);
	ret += smartpakit_parse_out_device_info(&pdev->dev, pakit_priv);
	ret += smartpakit_parse_dt_irq_pinctrl(&pdev->dev, pakit_priv);

	ret += smartpakit_parse_dt_misc_rw_permission(&pdev->dev, pakit_priv);
	ret += smartpakit_parse_dt_i2c_addr_type(&pdev->dev, pakit_priv);
	if (ret < 0)
		return ret;

	ret = smartpakit_check_dt_info_valid(pakit_priv);
	if (ret < 0)
		return ret;

	ret = smartpakit_get_prop_of_u32_type(pdev->dev.of_node,
		need_pa_check_str, &pakit_priv->need_pa_check, true);
	if (ret < 0) {
		hwlog_info("%s: get pa check flag fail, do not need pa check", __func__);
		pakit_priv->need_pa_check = 0;
	}

	return smartpakit_parse_dt_switch_ctl_cfg(&pdev->dev, pakit_priv);
}

static void smartpakit_free(struct smartpakit_priv *pakit_priv)
{
	if (pakit_priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return;
	}
	smartpakit_free_switch_ctl_gpio(pakit_priv->switch_num,
		pakit_priv->switch_ctl);

	kfree(pakit_priv);
}

static void smartpakit_reset_pakit_priv_data(struct smartpakit_priv *pakit_priv)
{
	int i;
	int len;

	if (pakit_priv == NULL)
		return;

	pakit_priv->chip_vendor = 0;
	pakit_priv->resume_sequence_permission_enable = false;
	pakit_priv->misc_rw_permission_enable = false;
	pakit_priv->misc_i2c_use_pseudo_addr = false;
	pakit_priv->current_i2c_client = NULL;
	pakit_priv->force_refresh_chip = false;
	pakit_priv->chip_register_failed = false;
	pakit_priv->need_reset_resume_chip = true;

	len = (strlen(SMARTPAKIT_NAME_INVALID) < SMARTPAKIT_NAME_MAX) ?
		strlen(SMARTPAKIT_NAME_INVALID) : (SMARTPAKIT_NAME_MAX - 1);

	if (memset_s(pakit_priv->i2c_addr_to_pa_index,
		sizeof(pakit_priv->i2c_addr_to_pa_index),
		SMARTPAKIT_INVALID_PA_INDEX,
		sizeof(pakit_priv->i2c_addr_to_pa_index)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);
	for (i = 0; i < SMARTPAKIT_PA_ID_MAX; i++) {
		if (strncpy_s(pakit_priv->chip_model_list[i],
			sizeof(pakit_priv->chip_model_list[i]),
			SMARTPAKIT_NAME_INVALID, len) != EOK)
			hwlog_err("%s: strncpy_s is failed\n", __func__);
		pakit_priv->chip_model_list[i][len] = '\0';
	}
}

static void smartpakit_init_chip_ctl_ops(struct smartpakit_priv *pakit_priv)
{
	if (pakit_priv == NULL)
		return;

	if ((pakit_priv->algo_in == SMARTPAKIT_ALGO_IN_SIMPLE) ||
		(pakit_priv->algo_in == SMARTPAKIT_ALGO_IN_WITH_DSP_PLUGIN)) {
		smartpakit_ctrl_init_chips = smartpakit_ctrl_simple_pa;
		smartpakit_ctrl_write_chips = smartpakit_ctrl_simple_pa;
	} else {
		smartpakit_ctrl_init_chips = smartpakit_init_chips_with_i2c;
		smartpakit_ctrl_write_chips = smartpakit_ctrl_write_regs;
	}
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
static void pa_check_handle(struct timer_list *timer)
{
}
#else
static void pa_check_handle(unsigned long data)
{
	hwlog_info("%s: enter\n", __func__);
	queue_delayed_work(smartpakit_priv_data->pa_check_delay_wq,
		&smartpakit_priv_data->pa_check_delay_work,
		msecs_to_jiffies(0));

	if (smartpakit_priv_data->loop_state) {
		smartpakit_priv_data->timer->expires =
			jiffies + jiffies_to_msecs(2 * HZ); // 8S loop
		add_timer(smartpakit_priv_data->timer);
	} else {
		hwlog_info("%s: pa check loop exit\n", __func__);
		smartpakit_priv_data->timer_state = false;
	}
}
#endif

static void pa_check_work(struct work_struct *work)
{
	int i;
	int ret = 0;
	struct smartpakit_get_param reg;
	struct smartpakit_i2c_priv *i2c_priv = NULL;

	hwlog_info("%s: enter\n", __func__);

	if (!smartpakit_priv_data->ioctl_ops ||
		!smartpakit_priv_data->ioctl_ops->read_regs) {
		hwlog_err("%s: read ops null\n", __func__);
		return;
	}

	for (i = 0; i < smartpakit_priv_data->pa_num; i++) {
		i2c_priv = smartpakit_priv_data->i2c_priv[i];
		reg.index = i2c_priv->version_regs_seq->regs[0].addr;
		reg.value = 0;

		mutex_lock(&smartpakit_priv_data->i2c_ops_lock);
		ret = smartpakit_priv_data->ioctl_ops->read_regs(i2c_priv, &reg);
		mutex_unlock(&smartpakit_priv_data->i2c_ops_lock);
		if ((ret < 0) || (reg.value == 0)) {
			hwlog_err("%s: pa check fail, reset pa\n", __func__);
#ifdef CONFIG_HUAWEI_DSM_AUDIO
			smartpakit_handle_i2c_probe_dsm_report(smartpakit_priv_data);
#endif
			smartpakit_hw_reset_chips(smartpakit_priv_data);
			smartpakit_resume_chips(smartpakit_priv_data);
			break;
		}
	}
}

static int pa_check_init(struct smartpakit_priv *pakit_priv)
{
	if (pakit_priv->need_pa_check == 0)
		return 0;

	hwlog_info("%s: need pa check, init\n", __func__);

	pakit_priv->timer_state = false;
	pakit_priv->loop_state = false;

	if (pakit_priv->timer) {
		hwlog_info("%s: timer has been init\n", __func__);
		return 0;
	}

	pakit_priv->timer = kzalloc(sizeof(struct timer_list), GFP_KERNEL);
	if (!pakit_priv->timer) {
		hwlog_err("%s: timer alloc fail\n", __func__);
		return -1;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	timer_setup(pakit_priv->timer, pa_check_handle, 0);
#else
	init_timer(pakit_priv->timer);
	pakit_priv->timer->function = pa_check_handle;
#endif

	pakit_priv->pa_check_delay_wq = create_singlethread_workqueue("pa_check_delay_wq");
	if (!pakit_priv->pa_check_delay_wq) {
		hwlog_err("%s: workqueue create fail\n", __func__);
		del_timer(pakit_priv->timer);
		kfree(pakit_priv->timer);
		pakit_priv->timer = NULL;
		return -1;
	}
	INIT_DELAYED_WORK(&pakit_priv->pa_check_delay_work, pa_check_work);

	return 0;
}

static int smartpakit_probe(struct platform_device *pdev)
{
	struct smartpakit_priv *pakit_priv = NULL;
	int ret;

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: enter\n", __func__);
#endif

	if (pdev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	pakit_priv = kzalloc(sizeof(*pakit_priv), GFP_KERNEL);
	if (pakit_priv == NULL)
		return -ENOMEM;

	platform_set_drvdata(pdev, pakit_priv);
	smartpakit_reset_pakit_priv_data(pakit_priv);

	ret = smartpakit_parse_dt_info(pdev, pakit_priv);
	if (ret < 0) {
		hwlog_err("%s: parse dt_info failed, %d\n", __func__, ret);
		goto err_out;
	}

	ret = smartpakit_init_switch_ctl_gpio(pakit_priv);
	if (ret < 0) {
		hwlog_err("%s: init switch ctl err, %d\n", __func__, ret);
		goto err_out;
	}

	/* init ops lock */
	mutex_init(&pakit_priv->irq_handler_lock);
	mutex_init(&pakit_priv->hw_reset_lock);
	mutex_init(&pakit_priv->dump_regs_lock);
	mutex_init(&pakit_priv->i2c_ops_lock);
	mutex_init(&pakit_priv->do_ioctl_lock);
	mutex_init(&pakit_priv->resume_sequence_lock);

	ret = misc_register(&smartpakit_ctrl_miscdev);
	if (ret != 0) {
		hwlog_err("%s: register miscdev failed, %d\n", __func__, ret);
		goto err_out;
	}

	smartpakit_priv_data = pakit_priv;
	smartpakit_init_flag = 1; /* 1: init success */
	smartpakit_init_chip_ctl_ops(pakit_priv);

	INIT_LIST_HEAD(&pakit_priv->irq_filter_list);
	INIT_LIST_HEAD(&pakit_priv->i2c_dev_list);

#ifdef CONFIG_SND_SOC_AU_PA
	hwlog_info("%s: register_hisi reg ops\n", __func__);
	smartpa_regulator_reg_ops_register(&hisi_reg_ops);
#endif

	ret = pa_check_init(pakit_priv);
	if (ret != 0) {
		hwlog_err("%s: pa check init fail\n", __func__);
		goto err_misc;
	}

	hwlog_info("%s: end success\n", __func__);
	return 0;

err_misc:
	misc_deregister(&smartpakit_ctrl_miscdev);

err_out:
	smartpakit_free(pakit_priv);
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static void delete_i2c_dev_info_list_all(struct list_head *head)
{
	struct i2c_dev_info *pos = NULL;
	struct i2c_dev_info *n = NULL;

	list_for_each_entry_safe(pos, n, head, list) {
		list_del(&pos->list);
		kfree(pos);
	}
}

static int smartpakit_remove(struct platform_device *pdev)
{
	struct smartpakit_priv *pakit_priv = NULL;
	struct smartpakit_pa_ctl_sequence *sequence = NULL;
	int i;

	if (pdev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	pakit_priv = platform_get_drvdata(pdev);
	if (pakit_priv == NULL) {
		hwlog_err("%s: pakit_priv invalid argument\n", __func__);
		return -EINVAL;
	}
	platform_set_drvdata(pdev, NULL);

	smartpakit_reg_info_del_list_all(&pakit_priv->irq_filter_list);
	smartpakit_reset_pa_ctl_sequence(&pakit_priv->resume_sequence);
	delete_i2c_dev_info_list_all(&pakit_priv->i2c_dev_list);
	for (i = 0; i < SMARTPAKIT_PA_ID_MAX; i++) {
		sequence = &pakit_priv->poweron_seq[i];
		smartpakit_reset_pa_ctl_sequence(sequence);
	}

	if (pakit_priv->pa_check_delay_wq) {
		cancel_delayed_work(&pakit_priv->pa_check_delay_work);
		flush_workqueue(pakit_priv->pa_check_delay_wq);
		destroy_workqueue(pakit_priv->pa_check_delay_wq);
	}

	if (pakit_priv->timer) {
		del_timer(pakit_priv->timer);
		kfree(pakit_priv->timer);
		pakit_priv->timer = NULL;
	}

	smartpakit_free(pakit_priv);
	smartpakit_priv_data = NULL;

	misc_deregister(&smartpakit_ctrl_miscdev);
	return 0;
}

static void smartpakit_shutdown(struct platform_device *pdev)
{
	smartpakit_simple_pa_power_off(pdev);
}

static const struct of_device_id smartpakit_match[] = {
	{ .compatible = "huawei,smartpakit", },
	{},
};
MODULE_DEVICE_TABLE(of, smartpakit_match);

static struct platform_driver smartpakit_driver = {
	.driver = {
		.name           = "smartpakit",
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(smartpakit_match),
	},
	.probe    = smartpakit_probe,
	.remove   = smartpakit_remove,
	.shutdown = smartpakit_shutdown,
};

static int __init smartpakit_init(void)
{
	int ret;

#ifdef CONFIG_HUAWEI_SMARTPAKIT_AUDIO_MODULE
	hwlog_info("%s enter from modprobe\n", __func__);
#else
	hwlog_info("%s: platform_driver_register\n", __func__);
#endif
	ret = platform_driver_register(&smartpakit_driver);
	if (ret != 0)
		hwlog_err("%s: platform_driver_register failed, %d\n",
			__func__, ret);

	return ret;
}

static void __exit smartpakit_exit(void)
{
	platform_driver_unregister(&smartpakit_driver);
}

late_initcall(smartpakit_init);
module_exit(smartpakit_exit);

MODULE_DESCRIPTION("smartpakit driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");

