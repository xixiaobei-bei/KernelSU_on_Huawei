/*
 * hwevext_adc.c
 *
 * extern adc driver
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
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/i2c-dev.h>
#include <linux/regmap.h>
#include <huawei_platform/log/hw_log.h>
#include <dsm/dsm_pub.h>
#include <huawei_platform/audio/hwevext_adc.h>
#ifdef CONFIG_HUAWEI_DSM_AUDIO
#include <dsm_audio/dsm_audio.h>
#endif
#include <securec.h>

#define HWLOG_TAG hwevext_adc
HWLOG_REGIST();

#define ADC_POWERON     1
#define ADC_POWEROFF    0

#define BYTE_LEN    8

typedef int (*hwevext_adc_ops_fun)(struct hwevext_adc_i2c_priv *priv);

static struct hwevext_adc_priv *hwevext_adc_priv_data = NULL;

/* misc device 0 not init completed, 1 init completed */
static int hwevext_adc_init_flag = 0;

#define HWEVEXT_ADC_PGA_VOLUME_SINGLE(xname, chip_id, pga_volum_index) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
  .info = snd_hwevext_adc_volume_info, \
  .get = snd_hwevext_adc_get_volume, .put = snd_hwevext_adc_put_volume, \
  .private_value = chip_id | (pga_volum_index << 16)}


#define HWEVEXT_ADC_KCTRL_SINGLE(xname, chip_id, kctl_index) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
  .info = snd_hwevext_adc_kctl_info, \
  .get = snd_hwevext_adc_get_kctl, .put = snd_hwevext_adc_put_kctl, \
  .private_value = chip_id | (kctl_index << 16)}

struct hwevext_adc_priv *hwevext_adc_get_misc_priv(void)
{
	return hwevext_adc_priv_data;
}

int hwevext_adc_get_misc_init_flag(void)
{
	return hwevext_adc_init_flag;
}

static void hwevext_adc_map_i2c_addr_to_chip_id(
	struct hwevext_adc_i2c_priv *i2c_priv, unsigned int id)
{
	unsigned int addr;
	unsigned char chip_id;

	if (i2c_priv->i2c == NULL)
		return;

	chip_id = (unsigned char)id;
	addr = i2c_priv->i2c->addr;

	if (addr < HWEVEXT_ADC_I2C_ADDR_ARRAY_MAX)
		hwevext_adc_priv_data->i2c_addr_to_pa_index[addr] = chip_id;
}

int hwevext_adc_register_i2c_device(struct hwevext_adc_i2c_priv *i2c_priv)
{
	unsigned int str_length;

	if ((hwevext_adc_priv_data == NULL) || (i2c_priv == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (i2c_priv->chip_id >= hwevext_adc_priv_data->adc_num) {
		hwlog_err("%s: error, chip_id %u >= adc_num %u\n", __func__,
			i2c_priv->chip_id, hwevext_adc_priv_data->adc_num);
		hwevext_adc_priv_data->chip_register_failed = true;
		return -EINVAL;
	}

	if (hwevext_adc_priv_data->i2c_priv[i2c_priv->chip_id] != NULL) {
		hwlog_err("%s: chip_id reduplicated error\n", __func__);
		hwevext_adc_priv_data->chip_register_failed = true;
		return -EINVAL;
	}

	i2c_priv->priv_data = (void *)hwevext_adc_priv_data;
	hwevext_adc_priv_data->i2c_num++;
	hwevext_adc_priv_data->i2c_priv[i2c_priv->chip_id] = i2c_priv;

	str_length = (strlen(i2c_priv->chip_model) < HWEVEXT_ADC_ID_MAX) ?
		strlen(i2c_priv->chip_model) : (HWEVEXT_ADC_NAME_MAX - 1);
	if (strncpy_s(hwevext_adc_priv_data->chip_model_list[i2c_priv->chip_id],
		sizeof(hwevext_adc_priv_data->chip_model_list[i2c_priv->chip_id]),
		i2c_priv->chip_model,
		str_length) != EOK)
		hwlog_err("%s: strncpy_s is failed\n", __func__);

	hwevext_adc_map_i2c_addr_to_chip_id(i2c_priv, i2c_priv->chip_id);

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: i2c_priv registered, success\n", __func__);
#endif

	return 0;
}

int hwevext_adc_deregister_i2c_device(struct hwevext_adc_i2c_priv *i2c_priv)
{
	int i;
	unsigned int chip_id;

	if ((hwevext_adc_priv_data == NULL) || (i2c_priv == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < (int)hwevext_adc_priv_data->adc_num; i++) {
		if (hwevext_adc_priv_data->i2c_priv[i] == NULL)
			continue;
		chip_id = hwevext_adc_priv_data->i2c_priv[i]->chip_id;
		if (i2c_priv->chip_id != chip_id)
			continue;

		hwevext_adc_map_i2c_addr_to_chip_id(i2c_priv,
			HWEVEXT_ADC_INVALID_PA_INDEX);
		i2c_priv->priv_data = NULL;

		hwevext_adc_priv_data->i2c_num--;
		hwevext_adc_priv_data->i2c_priv[i] = NULL;
		hwlog_info("%s: i2c_priv deregistered, success\n", __func__);
		break;
	}

	return 0;
}

void hwevext_adc_register_i2c_ctl_ops(struct hwevext_adc_i2c_ctl_ops *ops)
{
	if ((hwevext_adc_priv_data == NULL) || (ops == NULL)) {
		hwlog_err("%s: ioctl_ops register failed\n", __func__);
		return;
	}

	if (hwevext_adc_priv_data->ioctl_ops != NULL) {
#ifndef CONFIG_FINAL_RELEASE
		hwlog_debug("%s: ioctl_ops had registered, skip\n", __func__);
#endif
		return;
	}

	hwevext_adc_priv_data->ioctl_ops = ops;
#ifndef CONFIG_FINAL_RELEASE
	hwlog_debug("%s: ioctl_ops registered, success\n", __func__);
#endif
}

void hwevext_adc_deregister_i2c_ctl_ops(void)
{
	if (hwevext_adc_priv_data == NULL) {
		hwlog_err("%s: ioctl_ops deregister failed\n", __func__);
		return;
	}

	if (hwevext_adc_priv_data->i2c_num != 0) {
		hwlog_debug("%s: skip deregister ioctl_ops\n", __func__);
		return;
	}

	hwevext_adc_priv_data->ioctl_ops = NULL;
	hwlog_info("%s: ioctl_ops deregistered, success\n", __func__);
}

void hwevext_adc_dump_chips(struct hwevext_adc_priv *adc_priv)
{
	struct hwevext_adc_i2c_priv *i2c_priv_p = NULL;
	int ret = 0;
	int i;

	if (adc_priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return;
	}

	if ((adc_priv->ioctl_ops == NULL) ||
		(adc_priv->ioctl_ops->dump_regs == NULL)) {
		hwlog_err("%s: i2c dump_regs ops is NULL\n", __func__);
		return;
	}

	mutex_lock(&adc_priv->dump_regs_lock);
	for (i = 0; i < (int)adc_priv->adc_num; i++) {
		i2c_priv_p = adc_priv->i2c_priv[i];
		if (i2c_priv_p == NULL) {
			hwlog_err("%s: i2c_priv_p is NULL\n", __func__);
			goto hwevext_adc_dump_chips_end;
		}
		ret = adc_priv->ioctl_ops->dump_regs(i2c_priv_p);
		if (ret < 0)
			break;
	}

 hwevext_adc_dump_chips_end:
	mutex_unlock(&adc_priv->dump_regs_lock);
}
EXPORT_SYMBOL_GPL(hwevext_adc_dump_chips);

static void hwevext_adc_get_model_from_i2c_cfg(char *dst,
	struct hwevext_adc_priv *adc_priv, unsigned int dst_len)
{
	char report_tmp[HWEVEXT_ADC_NAME_MAX] = {0};
	unsigned int model_len;
	unsigned int tmp_len;
	int i;

	if ((dst == NULL) || (adc_priv == NULL) || (adc_priv->adc_num == 0))
		return;

	for (i = 0; i < adc_priv->adc_num; i++) {
		if (i < (adc_priv->adc_num - 1)) {
			if (snprintf_s(report_tmp,
				(unsigned long)HWEVEXT_ADC_NAME_MAX,
				(unsigned long)HWEVEXT_ADC_NAME_MAX - 1,
				"%s_", adc_priv->chip_model_list[i]) < 0)
				hwlog_err("%s: snprintf_s is failed\n",
					__func__);
		} else {
			if (snprintf_s(report_tmp,
				(unsigned long)HWEVEXT_ADC_NAME_MAX,
				(unsigned long)HWEVEXT_ADC_NAME_MAX - 1,
				"%s", adc_priv->chip_model_list[i]) < 0)
				hwlog_err("%s: snprintf_s is failed\n",
					__func__);
		}

		model_len = dst_len - (strlen(dst) + 1);
		tmp_len = strlen(report_tmp);
		model_len = (tmp_len < model_len) ? tmp_len : model_len;
		if (strncat_s(dst, model_len, report_tmp, tmp_len) != EOK)
			hwlog_err("%s: strncat_s is failed\n", __func__);
	}
}

static int get_hwevext_adc_info_from_dts_config(struct hwevext_adc_priv *adc_priv,
	struct hwevext_adc_info *info)
{
	size_t chip_model_len;
	unsigned int i;

	if ((adc_priv == NULL) || (info == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	info->adc_num = adc_priv->i2c_num;
	for (i = 0; i < HWEVEXT_ADC_ID_MAX; i++) {
		chip_model_len = strlen(adc_priv->chip_model_list[i]);
		chip_model_len = (chip_model_len < HWEVEXT_ADC_NAME_MAX) ?
			chip_model_len : (HWEVEXT_ADC_NAME_MAX - 1);
		if (strncpy_s(info->chip_model_list[i],
			sizeof(info->chip_model_list[i]),
			adc_priv->chip_model_list[i],
			chip_model_len) != EOK)
			hwlog_err("%s: strncpy_s is failed\n", __func__);
	}

	hwevext_adc_get_model_from_i2c_cfg(info->chip_model,
		adc_priv, HWEVEXT_ADC_NAME_MAX);
	hwlog_info("%s:adc chip_model:%s \n", info->chip_model, __func__);

	return 0;
}

static int hwevext_adc_get_info(struct hwevext_adc_priv *adc_priv)
{
	struct hwevext_adc_info info;

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: enter\n", __func__);
#endif
	if (adc_priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (memset_s(&info, sizeof(info), 0, sizeof(info)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);

	if (get_hwevext_adc_info_from_dts_config(adc_priv, &info) < 0)
		return -EINVAL;

	hwlog_info("%s: adc_num=%u, chip_model=%s\n",
		__func__, info.adc_num, info.chip_model);

	if (info.adc_num != adc_priv->adc_num)
		hwlog_info("%s: NOTICE I2C_NUM %u != ADC_NUM %u\n", __func__,
			info.adc_num, adc_priv->adc_num);

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: enter end\n", __func__);
#endif
	return 0;
}

static int hwevext_adc_write_regs_to_multi_chips(
	struct hwevext_adc_priv *adc_priv, hwevext_adc_ops_fun ops_fun,
	unsigned int status)
{
	struct hwevext_adc_i2c_priv *i2c_priv_p = NULL;
	int ret = 0;
	int i;

	if (adc_priv == NULL || ops_fun == NULL) {
		hwlog_err("%s: write_regs is invalid\n", __func__);
		return -ECHILD;
	}

	mutex_lock(&adc_priv->i2c_ops_lock);
	for (i = 0; i < (int)adc_priv->adc_num; i++) {
		i2c_priv_p = adc_priv->i2c_priv[i];
		if (i2c_priv_p == NULL) {
			hwlog_err("%s: i2c_priv is NULL\n", __func__);
			continue;
		}

		ret = ops_fun(i2c_priv_p);
		if (ret < 0)
			break;

		adc_priv->power_status[i] = status;
	}
	mutex_unlock(&adc_priv->i2c_ops_lock);

	return ret;
}

#ifdef CONFIG_NEED_WRITE_SINGLE_ADC
static int hwevext_write_regs_to_single_adc(
	struct hwevext_adc_priv *adc_priv, hwevext_adc_ops_fun ops_fun,
	unsigned int id, unsigned int status)
{
	struct hwevext_adc_i2c_priv *i2c_priv = NULL;
	unsigned int chip_id = 0;
	int ret = 0;

	if (adc_priv == NULL || ops_fun == NULL) {
		hwlog_err("%s: write_regs is invalid\n", __func__);
		return -ECHILD;
	}

	if (adc_priv->adc_num >= (id + 1))
		chip_id = id;

	mutex_lock(&adc_priv->i2c_ops_lock);
	i2c_priv = adc_priv->i2c_priv[chip_id];
	if (i2c_priv == NULL) {
		hwlog_err("%s: i2c_priv is NULL\n", __func__);
		goto single_adc_exit;
	}

	ret = ops_fun(i2c_priv);
	if (ret < 0)
		goto single_adc_exit;

	adc_priv->power_status[i] = status;
single_adc_exit:
	mutex_unlock(&adc_priv->i2c_ops_lock);
	return ret;
}
#endif

int hwevext_adc_get_prop_of_u32_array(struct device_node *node,
	const char *propname, u32 **value, int *num)
{
	u32 *array = NULL;
	int ret;
	int count = 0;

	if ((node == NULL) || (propname == NULL) || (value == NULL) ||
		(num == NULL)) {
		pr_err("%s: invalid argument\n", __func__);
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
		goto hwevext_adc_get_prop_err_out;
	}

	*value = array;
	*num = count;
	return 0;

hwevext_adc_get_prop_err_out:
	hwevext_adc_kfree_ops(array);
	return ret;
}

int hwevext_adc_get_prop_of_u32_type(struct device_node *node,
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

int hwevext_adc_get_prop_of_str_type(struct device_node *node,
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

static unsigned int hwevext_adc_get_fist_bit_from_mask(unsigned int mask)
{
	unsigned int size = sizeof(mask) * BYTE_LEN;
	unsigned int i;

	for (i = 0; i < size; i++) {
		if ((mask >> i) & 0x1)
			break;
	}

	return i;
}

static int hwevext_adc_check_adc_volume_info_valid(unsigned long private_value)
{
	int index = private_value & 0xff;
	int pga_reg_index = (private_value >> 16) & 0xff;

	struct hwevext_adc_i2c_priv *i2c_priv = NULL;
	struct hwevext_adc_reg_ctl_sequence *seq = NULL;

	if (hwevext_adc_priv_data == NULL) {
		hwlog_info("%s: hwevext_adc_priv_data is NULL\n", __func__);
		return -EINVAL;
	}

	if (index >= HWEVEXT_ADC_ID_MAX) {
		hwlog_info("%s: index %d is invaild\n", __func__, index);
		return -EINVAL;
	}

	i2c_priv = hwevext_adc_priv_data->i2c_priv[index];
	if (i2c_priv == NULL) {
		hwlog_info("%s: i2c_priv[%d] is NULL\n", __func__, index);
		return -EINVAL;
	}

	seq = i2c_priv->pga_volume_seq;
	if (seq == NULL) {
		hwlog_err("%s: seq is NULL\n", __func__);
		return -EINVAL;
	}

	if (pga_reg_index >= seq->num ||
		pga_reg_index >= HWEVEXT_DAC_PGA_VOLUME_MAX) {
		hwlog_err("%s: index %d is invaild\n", __func__, pga_reg_index);
		return -EINVAL;
	}

	return 0;
}

static int snd_hwevext_adc_volume_info(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_info *uinfo)
{
	int chip_id;
	int pga_reg_index;
	int ret;

	struct hwevext_adc_i2c_priv *i2c_priv = NULL;
	struct hwevext_adc_reg_ctl_sequence *seq = NULL;
	struct hwevext_adc_reg_ctl *regs = NULL;

	if (kcontrol == NULL || uinfo == NULL) {
		hwlog_err("%s: input pointer is null", __func__);
		return -1;
	}

	ret = hwevext_adc_check_adc_volume_info_valid(kcontrol->private_value);
	if (ret < 0) {
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 0;
		return 0;
	}

	chip_id = kcontrol->private_value & 0xff;
	pga_reg_index = (kcontrol->private_value >> 16) & 0xff;
	i2c_priv = hwevext_adc_priv_data->i2c_priv[chip_id];
	seq = i2c_priv->pga_volume_seq;
	regs = &(seq->regs[pga_reg_index]);

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = regs->min_value;
	uinfo->value.integer.max = regs->max_value;
	return 0;
}

static int snd_hwevext_adc_get_volume(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	unsigned int chip_id;
	unsigned int pga_reg_index;
	unsigned int shif_bit;
	int ret;
	struct hwevext_adc_reg_ctl pga_regs;
	struct hwevext_adc_i2c_priv *i2c_priv = NULL;
	struct hwevext_adc_reg_ctl_sequence *seq = NULL;
	struct hwevext_adc_reg_ctl *regs = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		hwlog_err("%s: input pointer is null", __func__);
		return -1;
	}

	ret = hwevext_adc_check_adc_volume_info_valid(kcontrol->private_value);
	if (ret < 0) {
		ucontrol->value.integer.value[0] = 0;
		return 0;
	}

	chip_id = kcontrol->private_value & 0xff;
	pga_reg_index = (kcontrol->private_value >> 16) & 0xff;
	i2c_priv = hwevext_adc_priv_data->i2c_priv[chip_id];
	seq = i2c_priv->pga_volume_seq;
	regs = &(seq->regs[pga_reg_index]);
	mutex_lock(&hwevext_adc_priv_data->i2c_ops_lock);
	pga_regs.addr = regs->addr;
	pga_regs.mask = regs->mask;
	pga_regs.delay = regs->delay;
	pga_regs.ctl_type = HWEVEXT_DAC_REG_CTL_TYPE_R;
	shif_bit = hwevext_adc_get_fist_bit_from_mask(pga_regs.mask);

	if (hwevext_adc_priv_data->ioctl_ops == NULL ||
		hwevext_adc_priv_data->ioctl_ops->read_reg == NULL) {
		hwlog_err("%s: ioctl_ops is NULL\n", __func__);
		ucontrol->value.integer.value[0] =
			hwevext_adc_priv_data->pag_volume[chip_id][pga_reg_index];
		goto exit_get_volume;
	}

	ret = hwevext_adc_priv_data->ioctl_ops->read_reg(i2c_priv, &pga_regs);
	if (ret < 0) {
		hwlog_err("%s: read adc:%d, %d, reg failed",
			__func__, chip_id, pga_reg_index);
		ucontrol->value.integer.value[0] =
			hwevext_adc_priv_data->pag_volume[chip_id][pga_reg_index];
	} else {
		ucontrol->value.integer.value[0] =
			(pga_regs.value & pga_regs.mask) >> shif_bit;
	}
exit_get_volume:
	mutex_unlock(&hwevext_adc_priv_data->i2c_ops_lock);
	return 0;
}

static int snd_hwevext_adc_put_volume(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	unsigned int chip_id;
	unsigned int pga_reg_index;
	unsigned int shif_bit;
	int ret;
	struct hwevext_adc_reg_ctl_sequence *seq = NULL;
	struct hwevext_adc_reg_ctl *regs = NULL;

	struct hwevext_adc_reg_ctl_sequence pga_seq;
	struct hwevext_adc_reg_ctl pga_regs;
	struct hwevext_adc_i2c_priv *i2c_priv = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		hwlog_err("%s: input pointer is null", __func__);
		return 0;
	}
#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: chip_id 0x%x, pga_reg_index 0x%x\n", __func__,
		chip_id, pga_reg_index);
#endif
	ret = hwevext_adc_check_adc_volume_info_valid(kcontrol->private_value);
	if (ret < 0)
		return 0;

	chip_id = kcontrol->private_value & 0xff;
	pga_reg_index = (kcontrol->private_value >> 16) & 0xff;
	i2c_priv = hwevext_adc_priv_data->i2c_priv[chip_id];
	seq = i2c_priv->pga_volume_seq;
	regs = &(seq->regs[pga_reg_index]);

	mutex_lock(&hwevext_adc_priv_data->i2c_ops_lock);
	pga_regs.addr = regs->addr;
	pga_regs.mask = regs->mask;
	pga_regs.delay = regs->delay;

	shif_bit = hwevext_adc_get_fist_bit_from_mask(pga_regs.mask);

	pga_regs.value = ucontrol->value.integer.value[0] << shif_bit;
	pga_regs.ctl_type = HWEVEXT_DAC_REG_CTL_TYPE_W;

	pga_seq.num = 1;
	pga_seq.regs = &pga_regs;
	if (hwevext_adc_priv_data->ioctl_ops == NULL ||
		hwevext_adc_priv_data->ioctl_ops->write_regs == NULL) {
		hwlog_err("%s: ioctl_ops is NULL\n", __func__);
		goto hwevext_adc_exit_put_volume;
	}
	hwevext_adc_priv_data->ioctl_ops->write_regs(i2c_priv, &pga_seq);

hwevext_adc_exit_put_volume:
	hwevext_adc_priv_data->pag_volume[chip_id][pga_reg_index] =
		ucontrol->value.integer.value[0];
	mutex_unlock(&hwevext_adc_priv_data->i2c_ops_lock);
	return 0;
}

#define HWEVEXT_ADC_PGA_VOLUME_KCONTROLS \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC0_LEFT_PGA_VOLUME", 0, 0), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC0_RIGHT_PGA_VOLUME", 0, 1), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC1_LEFT_PGA_VOLUME", 1, 0), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC1_RIGHT_PGA_VOLUME", 1, 1), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC2_LEFT_PGA_VOLUME", 2, 0), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC2_RIGHT_PGA_VOLUME", 2, 1), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC3_LEFT_PGA_VOLUME", 3, 0), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC3_RIGHT_PGA_VOLUME", 3, 1), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC4_LEFT_PGA_VOLUME", 4, 0), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC4_RIGHT_PGA_VOLUME", 4, 1), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC5_LEFT_PGA_VOLUME", 5, 0), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC5_RIGHT_PGA_VOLUME", 5, 1), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC6_LEFT_PGA_VOLUME", 6, 0), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC6_RIGHT_PGA_VOLUME", 6, 1), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC7_LEFT_PGA_VOLUME", 7, 0), \
	HWEVEXT_ADC_PGA_VOLUME_SINGLE("EXT_ADC7_RIGHT_PGA_VOLUME", 7, 1), \

static int hwevext_adc_check_adc_kctl_info_valid(unsigned long private_value)
{
	int chip_id = private_value & 0xff;
	int kctl_index = (private_value >> 16) & 0xff;

	struct hwevext_adc_i2c_priv *i2c_priv = NULL;
	struct hwevext_adc_reg_ctl_sequence *seq = NULL;

	if (hwevext_adc_priv_data == NULL) {
		hwlog_info("%s: priv_data is NULL, index:%d, kctl_index:%d\n",
			__func__, chip_id, kctl_index);
		return -EINVAL;
	}

	if (chip_id >= HWEVEXT_ADC_ID_MAX) {
		hwlog_info("%s: index %d is invaild\n", __func__, chip_id);
		return -EINVAL;
	}

	if (kctl_index >= HWEVEXT_DAC_KCTL_MAX) {
		hwlog_err("%s: kctl_index %d is invaild\n", __func__, kctl_index);
		return -EINVAL;
	}

	i2c_priv = hwevext_adc_priv_data->i2c_priv[chip_id];
	if (i2c_priv == NULL) {
		hwlog_info("%s: i2c_priv[%d] is NULL\n", __func__, chip_id);
		return -EINVAL;
	}

	seq = i2c_priv->kctl_seqs[kctl_index];
	if (seq == NULL) {
		hwlog_err("%s: seq is NULL\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int snd_hwevext_adc_kctl_info(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_info *uinfo)
{
	int chip_id;
	int kctl_index;
	int ret;

	struct hwevext_adc_i2c_priv *i2c_priv = NULL;
	struct hwevext_adc_reg_ctl_sequence *seq = NULL;
	struct hwevext_adc_reg_ctl *regs = NULL;

	if (kcontrol == NULL || uinfo == NULL) {
		hwlog_err("%s: input pointer is null", __func__);
		return -1;
	}

	ret = hwevext_adc_check_adc_kctl_info_valid(kcontrol->private_value);
	if (ret < 0) {
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 0;
		return 0;
	}

	chip_id = kcontrol->private_value & 0xff;
	kctl_index = (kcontrol->private_value >> 16) & 0xff;
	i2c_priv = hwevext_adc_priv_data->i2c_priv[chip_id];
	seq = i2c_priv->kctl_seqs[kctl_index];
	/* regs num is alway is 1 */
	regs = seq->regs;

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = regs->min_value;
	uinfo->value.integer.max = regs->max_value;
	return 0;
}

static int snd_hwevext_adc_get_kctl(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	unsigned int chip_id;
	unsigned int kctl_index;
	unsigned int shif_bit;
	int ret;
	struct hwevext_adc_reg_ctl pga_regs;
	struct hwevext_adc_i2c_priv *i2c_priv = NULL;
	struct hwevext_adc_reg_ctl_sequence *seq = NULL;
	struct hwevext_adc_reg_ctl *regs = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		hwlog_err("%s: input pointer is null", __func__);
		return -1;
	}

	ret = hwevext_adc_check_adc_kctl_info_valid(kcontrol->private_value);
	if (ret < 0) {
		ucontrol->value.integer.value[0] = 0;
		return 0;
	}

	chip_id = kcontrol->private_value & 0xff;
	kctl_index = (kcontrol->private_value >> 16) & 0xff;
	i2c_priv = hwevext_adc_priv_data->i2c_priv[chip_id];
	seq = i2c_priv->kctl_seqs[kctl_index];
	/* regs num is alway is 1 */
	regs = seq->regs;

	mutex_lock(&hwevext_adc_priv_data->i2c_ops_lock);
	pga_regs.addr = regs->addr;
	pga_regs.mask = regs->mask;
	pga_regs.delay = regs->delay;
	pga_regs.ctl_type = HWEVEXT_DAC_REG_CTL_TYPE_R;
	shif_bit = hwevext_adc_get_fist_bit_from_mask(pga_regs.mask);

	if (hwevext_adc_priv_data->ioctl_ops == NULL ||
		hwevext_adc_priv_data->ioctl_ops->read_reg == NULL) {
		hwlog_err("%s: ioctl_ops is NULL\n", __func__);
		ucontrol->value.integer.value[0] =
			hwevext_adc_priv_data->kctl_status[chip_id][kctl_index];
		goto hwevext_adc_exit_get_kctl;
	}

	ret = hwevext_adc_priv_data->ioctl_ops->read_reg(i2c_priv, &pga_regs);
	if (ret < 0) {
		hwlog_err("%s: read kctl:%d reg failed", __func__, kctl_index);
		ucontrol->value.integer.value[0] =
			hwevext_adc_priv_data->kctl_status[chip_id][kctl_index];
	} else {
	        hwlog_info("%s: read kctl:%d reg:0x%x value:0x%x mask:0x%x",
                        __func__, kctl_index, pga_regs.addr,
                        pga_regs.value, pga_regs.mask);
		ucontrol->value.integer.value[0] =
                        (pga_regs.value & pga_regs.mask) >> shif_bit;
	}
hwevext_adc_exit_get_kctl:
	mutex_unlock(&hwevext_adc_priv_data->i2c_ops_lock);
	return 0;
}

static int snd_hwevext_adc_put_kctl(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	unsigned int chip_id;
	unsigned int kctl_index;
	unsigned int shif_bit;
	int ret;
	struct hwevext_adc_reg_ctl_sequence *seq = NULL;
	struct hwevext_adc_reg_ctl *regs = NULL;

	struct hwevext_adc_reg_ctl_sequence pga_seq;
	struct hwevext_adc_reg_ctl pga_regs;
	struct hwevext_adc_i2c_priv *i2c_priv = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		hwlog_err("%s: input pointer is null", __func__);
		return 0;
	}

	ret = hwevext_adc_check_adc_kctl_info_valid(kcontrol->private_value);
	if (ret < 0)
		return 0;

	chip_id = kcontrol->private_value & 0xff;
	kctl_index = (kcontrol->private_value >> 16) & 0xff;

	i2c_priv = hwevext_adc_priv_data->i2c_priv[chip_id];
	seq = i2c_priv->kctl_seqs[kctl_index];
	/* regs num is alway is 1 */
	regs = seq->regs;

	mutex_lock(&hwevext_adc_priv_data->i2c_ops_lock);
	pga_regs.addr = regs->addr;
	pga_regs.mask = regs->mask;
	pga_regs.delay = regs->delay;
	shif_bit = hwevext_adc_get_fist_bit_from_mask(pga_regs.mask);

	pga_regs.value = ucontrol->value.integer.value[0] << shif_bit;
	pga_regs.ctl_type = HWEVEXT_DAC_REG_CTL_TYPE_W;
	pga_seq.num = 1;
	pga_seq.regs = &pga_regs;

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: chip_id %u,index %u,reg:0x%x,value:0x%x,mask:0x%x\n",
		__func__, chip_id, kctl_index, pga_regs.addr,
		pga_regs.value, pga_regs.mask);
#endif

	if (hwevext_adc_priv_data->ioctl_ops == NULL ||
		hwevext_adc_priv_data->ioctl_ops->write_regs == NULL) {
		hwlog_err("%s: ioctl_ops is NULL\n", __func__);
		goto exit_put_kctl;
	}
	hwevext_adc_priv_data->ioctl_ops->write_regs(i2c_priv, &pga_seq);

exit_put_kctl:
	hwevext_adc_priv_data->kctl_status[chip_id][kctl_index] =
		ucontrol->value.integer.value[0];
	mutex_unlock(&hwevext_adc_priv_data->i2c_ops_lock);
	return 0;
}

#define HWEVEXT_ADC_PGA_KCTL_KCONTROLS \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC0_TDM_MODE", 0, HWEVEXT_DAC_KCTL_TDM_MODE), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC1_TDM_MODE", 1, HWEVEXT_DAC_KCTL_TDM_MODE), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC2_TDM_MODE", 2, HWEVEXT_DAC_KCTL_TDM_MODE), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC3_TDM_MODE", 3, HWEVEXT_DAC_KCTL_TDM_MODE), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC4_TDM_MODE", 4, HWEVEXT_DAC_KCTL_TDM_MODE), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC5_TDM_MODE", 5, HWEVEXT_DAC_KCTL_TDM_MODE), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC6_TDM_MODE", 6, HWEVEXT_DAC_KCTL_TDM_MODE), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC7_TDM_MODE", 7, HWEVEXT_DAC_KCTL_TDM_MODE), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC0_DATA_SEL", 0, HWEVEXT_DAC_KCTL_DATA_SEL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC1_DATA_SEL", 1, HWEVEXT_DAC_KCTL_DATA_SEL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC2_DATA_SEL", 2, HWEVEXT_DAC_KCTL_DATA_SEL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC3_DATA_SEL", 3, HWEVEXT_DAC_KCTL_DATA_SEL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC4_DATA_SEL", 4, HWEVEXT_DAC_KCTL_DATA_SEL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC5_DATA_SEL", 5, HWEVEXT_DAC_KCTL_DATA_SEL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC6_DATA_SEL", 6, HWEVEXT_DAC_KCTL_DATA_SEL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC7_DATA_SEL", 7, HWEVEXT_DAC_KCTL_DATA_SEL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC0_MUTE_CTL", 0, HWEVEXT_DAC_KCTL_MUTE_CTL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC1_MUTE_CTL", 1, HWEVEXT_DAC_KCTL_MUTE_CTL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC2_MUTE_CTL", 2, HWEVEXT_DAC_KCTL_MUTE_CTL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC3_MUTE_CTL", 3, HWEVEXT_DAC_KCTL_MUTE_CTL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC4_MUTE_CTL", 4, HWEVEXT_DAC_KCTL_MUTE_CTL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC5_MUTE_CTL", 5, HWEVEXT_DAC_KCTL_MUTE_CTL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC6_MUTE_CTL", 6, HWEVEXT_DAC_KCTL_MUTE_CTL), \
	HWEVEXT_ADC_KCTRL_SINGLE("EXT_ADC7_MUTE_CTL", 7, HWEVEXT_DAC_KCTL_MUTE_CTL), \


static const char * const adc_control_text[] = { "OFF", "ON" };

static const struct soc_enum adc_control_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(adc_control_text), adc_control_text),
};

static int adc_control_status_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	int ret = ADC_POWERON;
	int i;

	if (kcontrol == NULL || ucontrol == NULL) {
		hwlog_err("%s: input pointer is null", __func__);
		return 0;
	}

	if (hwevext_adc_priv_data == NULL) {
		hwlog_err("%s: hwevext_adc_priv_data is NULL\n", __func__);
		return 0;
	}

	for (i = 0; i < (int)hwevext_adc_priv_data->adc_num; i++) {
		if (hwevext_adc_priv_data->power_status[i] == ADC_POWEROFF) {
			ret = ADC_POWEROFF;
			break;
		}
	}
	ucontrol->value.integer.value[0] = ret;

	return 0;
}

static int adc_control_status_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;

	if (kcontrol == NULL || ucontrol == NULL) {
		hwlog_err("%s: input pointer is null", __func__);
		return 0;
	}

	if (hwevext_adc_priv_data == NULL) {
		hwlog_err("%s: hwevext_adc_priv_data is NULL\n", __func__);
		return 0;
	}

	ret = ucontrol->value.integer.value[0];

	if (hwevext_adc_priv_data->ioctl_ops == NULL ||
		hwevext_adc_priv_data->ioctl_ops->power_on == NULL ||
		hwevext_adc_priv_data->ioctl_ops->power_off == NULL) {
		hwlog_err("%s: ioctl_ops is NULL\n", __func__);
		return 0;
	}

	if (ret == ADC_POWERON) {
		hwlog_info("%s: set adc poweron\n", __func__);
		hwevext_adc_write_regs_to_multi_chips(hwevext_adc_priv_data,
			hwevext_adc_priv_data->ioctl_ops->power_on, ADC_POWERON);
	} else {
		hwlog_info("%s: set adc poweroff\n", __func__);
		hwevext_adc_write_regs_to_multi_chips(hwevext_adc_priv_data,
			hwevext_adc_priv_data->ioctl_ops->power_off, ADC_POWEROFF);
	}

	return ret;
}

#define HWEVEXT_ADC_POWER_KCONTROLS \
	SOC_ENUM_EXT("EXTERN_ADCS_POWER_CONTROL", adc_control_enum[0], \
		adc_control_status_get, adc_control_status_set),\


static const struct snd_kcontrol_new snd_controls[] = {
	HWEVEXT_ADC_PGA_VOLUME_KCONTROLS
	HWEVEXT_ADC_PGA_KCTL_KCONTROLS
	HWEVEXT_ADC_POWER_KCONTROLS
};

int hwevext_adc_add_kcontrol(struct snd_soc_component *codec)
{
	int ret;

	if (codec == NULL) {
		hwlog_err("%s: codec parameter is NULL\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: enter\n", __func__);

	ret = snd_soc_add_component_controls(codec, snd_controls,
		ARRAY_SIZE(snd_controls));
	if (ret < 0) {
		hwlog_err("%s: add kcontrol failed: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(hwevext_adc_add_kcontrol);

static int hwevext_adc_check_dt_info_valid(struct hwevext_adc_priv *adc_priv)
{
	int ret = 0;

	if (adc_priv->adc_num >= HWEVEXT_ADC_ID_MAX) {
		hwlog_err("%s: adc_num %u invalid\n", __func__,
			adc_priv->adc_num);
		ret = -EINVAL;
	}

	return ret;
}

static int hwevext_adc_parse_dt_info(struct platform_device *pdev,
	struct hwevext_adc_priv *adc_priv)
{
	const char *adc_num = "adc_num";
	int ret;

	if ((pdev == NULL) || (adc_priv == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	ret = hwevext_adc_get_prop_of_u32_type(pdev->dev.of_node,
		adc_num, &adc_priv->adc_num, true);
	ret += hwevext_adc_check_dt_info_valid(adc_priv);

	return ret;
}

static void hwevext_adc_free(struct hwevext_adc_priv *adc_priv)
{
	hwevext_adc_kfree_ops(adc_priv);
}

static void hwevext_adc_reset_priv_data(struct hwevext_adc_priv *adc_priv)
{
	int i;
	int len;

	if (adc_priv == NULL)
		return;

	adc_priv->chip_register_failed = false;

	len = (strlen(HWEVEXT_ADC_NAME_INVALID) < HWEVEXT_ADC_NAME_MAX) ?
		strlen(HWEVEXT_ADC_NAME_INVALID) : (HWEVEXT_ADC_NAME_MAX - 1);

	if (memset_s(adc_priv->i2c_addr_to_pa_index,
		sizeof(adc_priv->i2c_addr_to_pa_index),
		HWEVEXT_ADC_INVALID_PA_INDEX,
		sizeof(adc_priv->i2c_addr_to_pa_index)) != EOK)
		hwlog_err("%s: memset_s is failed\n", __func__);

	for (i = 0; i < HWEVEXT_ADC_ID_MAX; i++) {
		if (strncpy_s(adc_priv->chip_model_list[i],
			sizeof(adc_priv->chip_model_list[i]),
			HWEVEXT_ADC_NAME_INVALID,
			len) != EOK)
			hwlog_err("%s: strncpy_s is failed\n", __func__);

		adc_priv->chip_model_list[i][len] = '\0';
	}
}

static ssize_t hwevext_adc_regs_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (buf == NULL)
		return 0;

	if (hwevext_adc_priv_data == NULL || hwevext_adc_init_flag == 0)
		return 0;

	hwevext_adc_get_info(hwevext_adc_priv_data);
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE -1, "%s\n", "adc info");
}

static DEVICE_ATTR(hwevext_adc_regs_info, 0440, hwevext_adc_regs_info_show, NULL);

static struct attribute *hwevext_adc_attributes[] = {
	&dev_attr_hwevext_adc_regs_info.attr,
	NULL
};

static const struct attribute_group hwevext_adc_attr_group = {
	.attrs = hwevext_adc_attributes,
};

static int hwevext_adc_probe(struct platform_device *pdev)
{
	struct hwevext_adc_priv *adc_priv = NULL;
	int ret;

#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: enter\n", __func__);
#endif

	if (pdev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	adc_priv = kzalloc(sizeof(struct hwevext_adc_priv), GFP_KERNEL);
	if (adc_priv == NULL)
		return -ENOMEM;

	platform_set_drvdata(pdev, adc_priv);
	hwevext_adc_reset_priv_data(adc_priv);

	ret = hwevext_adc_parse_dt_info(pdev, adc_priv);
	if (ret < 0) {
		hwlog_err("%s: parse dt_info failed, %d\n", __func__, ret);
		goto probe_err_out;
	}

	/* init ops lock */
	mutex_init(&adc_priv->dump_regs_lock);
	mutex_init(&adc_priv->i2c_ops_lock);

	hwevext_adc_priv_data = adc_priv;
	hwevext_adc_init_flag = 1; /* 1: init success */

	ret = sysfs_create_group(&(pdev->dev.kobj), &hwevext_adc_attr_group);
	if (ret < 0)
		hwlog_err("failed to register sysfs\n");

	hwlog_info("%s: end success\n", __func__);
	return 0;

probe_err_out:
	hwevext_adc_free(adc_priv);
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int hwevext_adc_remove(struct platform_device *pdev)
{
	struct hwevext_adc_priv *adc_priv = NULL;

	if (pdev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	adc_priv = platform_get_drvdata(pdev);
	if (adc_priv == NULL) {
		hwlog_err("%s: adc_priv invalid argument\n", __func__);
		return -EINVAL;
	}
	platform_set_drvdata(pdev, NULL);
	hwevext_adc_free(adc_priv);
	hwevext_adc_priv_data = NULL;

	return 0;
}

static void hwevext_adc_shutdown(struct platform_device *pdev)
{
}

static const struct of_device_id hwevext_adc_match[] = {
	{ .compatible = "huawei,hwevext_adc", },
	{},
};
MODULE_DEVICE_TABLE(of, hwevext_adc_match);

static struct platform_driver hwevext_adc_driver = {
	.driver = {
		.name           = "hwevext_adc",
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(hwevext_adc_match),
	},
	.probe    = hwevext_adc_probe,
	.remove   = hwevext_adc_remove,
	.shutdown = hwevext_adc_shutdown,
};

static int __init hwevext_adc_init(void)
{
	int ret;

	ret = platform_driver_register(&hwevext_adc_driver);
	if (ret != 0)
		hwlog_err("%s: platform_driver_register failed, %d\n",
			__func__, ret);

	return ret;
}

static void __exit hwevext_adc_exit(void)
{
	platform_driver_unregister(&hwevext_adc_driver);
}

fs_initcall(hwevext_adc_init);
module_exit(hwevext_adc_exit);

MODULE_DESCRIPTION("hwevext_adc driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");

