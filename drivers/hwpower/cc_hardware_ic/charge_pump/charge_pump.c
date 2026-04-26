/*
 * charge_pump.c
 *
 * charge pump driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG charge_pump
HWLOG_REGIST();

static struct charge_pump_dev *g_charge_pump_dev;
static const struct charge_pump_device_data g_charge_pump_device_data[] = {
	{ CP_DEVICE_ID_HL1506_MAIN, "hl1506_main" },
	{ CP_DEVICE_ID_HL1506_AUX, "hl1506_aux" },
	{ CP_DEVICE_ID_SY6510, "sy6510" },
	{ CP_DEVICE_ID_PCA9488, "pca9488" },
	{ CP_DEVICE_ID_HL1512, "hl1512" },
	{ CP_DEVICE_ID_SC8502, "sc8502" },
};

static int charge_pump_get_device_id(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_charge_pump_device_data); i++) {
		if (!strncmp(str, g_charge_pump_device_data[i].name,
			strlen(str)) && !g_charge_pump_dev->t_ops[i])
			return g_charge_pump_device_data[i].id;
	}

	return -EINVAL;
}

static int charge_pump_get_ops_id(unsigned int type)
{
	int i;
	struct charge_pump_ops *ops = NULL;

	for (i = 0; i < CP_DEVICE_ID_END; i++) {
		ops = g_charge_pump_dev->t_ops[i];
		if (!ops || !(ops->cp_type & type))
			continue;
		if (ops->cp_type & CP_OPS_BYPASS)
			break;
		if (!ops->dev_check || ops->dev_check(ops->dev_data))
			continue;
		if (!ops->post_probe || ops->post_probe(ops->dev_data))
			continue;
		break;
	}
	if (i >= CP_DEVICE_ID_END)
		return -1;

	return i;
}

static struct charge_pump_ops *charge_pump_get_ops(unsigned int type)
{
	int id;
	static int retry_cnt;

	if (!g_charge_pump_dev) {
		hwlog_err("g_charge_pump_dev is null\n");
		return NULL;
	}
	if (g_charge_pump_dev->p_ops && (g_charge_pump_dev->p_ops->cp_type & type))
		return g_charge_pump_dev->p_ops;

	if (retry_cnt >= CP_GET_OPS_RETRY_CNT) {
		hwlog_err("retry too many times\n");
		return NULL;
	}

	id = charge_pump_get_ops_id(type);
	if (id < 0) {
		retry_cnt++;
		return NULL;
	}

	retry_cnt = 0;
	g_charge_pump_dev->p_ops = g_charge_pump_dev->t_ops[id];
	return g_charge_pump_dev->p_ops;
}

int charge_pump_ops_register(struct charge_pump_ops *ops)
{
	int dev_id;

	if (!g_charge_pump_dev || !ops || !ops->chip_name) {
		hwlog_err("g_charge_pump_dev or ops or chip_name is null\n");
		return -EINVAL;
	}

	dev_id = charge_pump_get_device_id(ops->chip_name);
	if (dev_id < 0) {
		hwlog_err("%s ops register fail\n", ops->chip_name);
		return -EINVAL;
	}

	g_charge_pump_dev->t_ops[dev_id] = ops;

	hwlog_info("%d:%s ops register ok\n", dev_id, ops->chip_name);
	return 0;
}

void charge_pump_chip_enable(unsigned int type, bool enable)
{
	struct charge_pump_dev *dev = g_charge_pump_dev;

	if (!dev)
		return;

	if ((type & CP_TYPE_MAIN) && (dev->gpio_main_en > 0)) {
		gpio_set_value(dev->gpio_main_en, enable ?
			dev->gpio_main_cp_en_val : !dev->gpio_main_cp_en_val);
		hwlog_info("[cp_en_main] gpio %s now\n",
			gpio_get_value(dev->gpio_main_en) ? "high" : "low");
	} else if ((type & CP_TYPE_AUX) && (dev->gpio_aux_en > 0)) {
		gpio_set_value(dev->gpio_aux_en, enable ?
			dev->gpio_aux_cp_en_val : !dev->gpio_aux_cp_en_val);
		hwlog_info("[cp_en_aux] gpio %s now\n",
			gpio_get_value(dev->gpio_aux_en) ? "high" : "low");
	}
}

int charge_pump_chip_init(unsigned int type)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops(type);

	if (!l_ops || !l_ops->chip_init)
		return -ENOTSUPP;

	return l_ops->chip_init(l_ops->dev_data);
}

int charge_pump_reverse_chip_init(unsigned int type)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops(type);

	if (!l_ops || !l_ops->rvs_chip_init)
		return -ENOTSUPP;

	return l_ops->rvs_chip_init(l_ops->dev_data);
}

int charge_pump_set_bp_mode(unsigned int type)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops(type);

	if (!l_ops || !l_ops->set_bp_mode)
		return -ENOTSUPP;

	return l_ops->set_bp_mode(l_ops->dev_data);
}

int charge_pump_set_cp_mode(unsigned int type)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops(type);

	if (!l_ops || !l_ops->set_cp_mode)
		return -ENOTSUPP;

	return l_ops->set_cp_mode(l_ops->dev_data);
}

int charge_pump_set_reverse_bp_mode(unsigned int type)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops(type);

	if (!l_ops || !l_ops->set_rvs_bp_mode)
		return -ENOTSUPP;

	return l_ops->set_rvs_bp_mode(l_ops->dev_data);
}

int charge_pump_set_reverse_cp_mode(unsigned int type)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops(type);

	if (!l_ops || !l_ops->set_rvs_cp_mode)
		return -ENOTSUPP;

	return l_ops->set_rvs_cp_mode(l_ops->dev_data);
}

int charge_pump_set_reverse_bp2cp_mode(unsigned int type)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops(type);

	if (!l_ops || !l_ops->set_rvs_bp2cp_mode)
		return -ENOTSUPP;

	return l_ops->set_rvs_bp2cp_mode(l_ops->dev_data);
}

int charge_pump_reverse_cp_chip_init(unsigned int type)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops(type);

	if (!l_ops || !l_ops->rvs_cp_chip_init)
		return -ENOTSUPP;

	return l_ops->rvs_cp_chip_init(l_ops->dev_data);
}

bool charge_pump_is_bp_open(unsigned int type)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops(type);

	if (!l_ops || !l_ops->is_bp_open)
		return true;

	return l_ops->is_bp_open(l_ops->dev_data);
}

bool charge_pump_is_cp_open(unsigned int type)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops(type);

	if (!l_ops || !l_ops->is_cp_open)
		return false;

	return l_ops->is_cp_open(l_ops->dev_data);
}

int charge_pump_get_cp_ratio(unsigned int type)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops(type);

	if (!charge_pump_is_cp_open(type))
		return CP_BP_RATIO;

	if (!l_ops || !l_ops->get_cp_ratio)
		return CP_CP_RATIO;

	return l_ops->get_cp_ratio(l_ops->dev_data);
}

int charge_pump_get_cp_vout(unsigned int type)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops(type);

	if (!l_ops || !l_ops->get_cp_vout)
		return -ENOTSUPP;

	return l_ops->get_cp_vout(l_ops->dev_data);
}

static void charge_pump_gpio_init(struct charge_pump_dev *di)
{
	const char *status = NULL;

	if (power_dts_read_string_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,charge_pump", "status", &status))
		return;

	if (!strcmp(status, "disabled"))
		return;

	if (power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,charge_pump", "gpio_main_cp_en_val",
		(u32 *)&di->gpio_main_cp_en_val, 0))
		return;

	if (power_gpio_config_output_compatible("huawei,charge_pump",
		"gpio_main_en", "main_cp_chip_en",
		&di->gpio_main_en, di->gpio_main_cp_en_val)) {
		di->gpio_main_en = 0;
		return;
	}

	if (power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,charge_pump", "gpio_aux_cp_en_val",
		(u32 *)&di->gpio_aux_cp_en_val, 0)) {
		gpio_free(di->gpio_main_en);
		di->gpio_main_en = 0;
		return;
	}

	if (power_gpio_config_output_compatible("huawei,charge_pump",
		"gpio_aux_en", "aux_cp_chip_en",
		&di->gpio_aux_en, !di->gpio_aux_cp_en_val)) {
		gpio_free(di->gpio_main_en);
		di->gpio_main_en = 0;
		di->gpio_aux_en = 0;
		return;
	}
}

static int __init charge_pump_init(void)
{
	struct charge_pump_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	charge_pump_gpio_init(l_dev);
	g_charge_pump_dev = l_dev;
	return 0;
}

static void __exit charge_pump_exit(void)
{
	kfree(g_charge_pump_dev);
	g_charge_pump_dev = NULL;
}

fs_initcall(charge_pump_init);
module_exit(charge_pump_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("charge pump driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
