/*
 * charge_pump.c
 *
 * charge pump driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/power/charge_pump.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG charge_pump
HWLOG_REGIST();

static struct charge_pump_dev *g_charge_pump_dev;
static const struct charge_pump_device_data g_charge_pump_device_data[] = {
	{ CP_DEVICE_ID_HL1506, "hl1506" },
	{ CP_DEVICE_ID_SY6510, "sy6510" },
	{ CP_DEVICE_ID_PCA9488, "pca9488" },
	{ CP_DEVICE_ID_HL1512, "hl1512" },
};

static int charge_pump_get_device_id(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_charge_pump_device_data); i++) {
		if (!strncmp(str, g_charge_pump_device_data[i].name,
			strlen(str)))
			return g_charge_pump_device_data[i].id;
	}

	return -EINVAL;
}

static int charge_pump_get_ops_id(void)
{
	int i;
	struct charge_pump_ops *ops = NULL;

	for (i = 0; i < CP_DEVICE_ID_END; i++) {
		ops = g_charge_pump_dev->t_ops[i];
		if (!ops || !ops->dev_check || !ops->post_probe)
			continue;
		if (ops->dev_check() || ops->post_probe())
			continue;
		break;
	}
	if (i >= CP_DEVICE_ID_END)
		return -1;

	return i;
}

static struct charge_pump_ops *charge_pump_get_ops(void)
{
	int id;
	static int retry_cnt;

	if (!g_charge_pump_dev) {
		hwlog_err("get_ops: g_charge_pump_dev null\n");
		return NULL;
	}
	if (g_charge_pump_dev->p_ops)
		return g_charge_pump_dev->p_ops;

	if (retry_cnt >= CP_GET_OPS_RETRY_CNT) {
		hwlog_err("get_ops: retry too many times\n");
		return NULL;
	}

	id = charge_pump_get_ops_id();
	if (id < 0) {
		retry_cnt++;
		return NULL;
	}
	g_charge_pump_dev->p_ops = g_charge_pump_dev->t_ops[id];
	hwlog_info("[get_ops] %s succ\n", g_charge_pump_dev->p_ops->chip_name);

	return g_charge_pump_dev->p_ops;
}

int charge_pump_ops_register(struct charge_pump_ops *ops)
{
	int dev_id;

	if (!g_charge_pump_dev || !ops || !ops->chip_name) {
		hwlog_err("ops_register: ops or chip_name null\n");
		return -EINVAL;
	}

	dev_id = charge_pump_get_device_id(ops->chip_name);
	if (dev_id < 0) {
		hwlog_err("ops_register: chip_name = %s ops register fail\n",
			ops->chip_name);
		return -EINVAL;
	}

	g_charge_pump_dev->t_ops[dev_id] = ops;
	if (dev_id == CP_DEVICE_ID_HL1506)
		g_charge_pump_dev->p_ops = ops;

	hwlog_info("[ops_register] %d:%s ops register ok\n",
		dev_id, ops->chip_name);
	return 0;
}

int charge_pump_chip_init(void)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops();

	if (!l_ops || !l_ops->chip_init)
		return -ENOTSUPP;

	return l_ops->chip_init();
}

int charge_pump_reverse_chip_init(void)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops();

	if (!l_ops || !l_ops->reverse_chip_init)
		return -ENOTSUPP;

	return l_ops->reverse_chip_init();
}

int charge_pump_set_bp_mode(void)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops();

	if (!l_ops || !l_ops->set_bp_mode)
		return -ENOTSUPP;

	return l_ops->set_bp_mode();
}

int charge_pump_set_cp_mode(void)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops();

	if (!l_ops || !l_ops->set_cp_mode)
		return -ENOTSUPP;

	return l_ops->set_cp_mode();
}

bool charge_pump_is_bp_open(void)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops();

	if (!l_ops || !l_ops->is_bp_open)
		return true;

	return l_ops->is_bp_open();
}

bool charge_pump_is_cp_open(void)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops();

	if (!l_ops || !l_ops->is_cp_open)
		return false;

	return l_ops->is_cp_open();
}

int charge_pump_get_cp_ratio(void)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops();

	if (!charge_pump_is_cp_open())
		return CP_BP_RATIO;

	if (!l_ops || !l_ops->get_cp_ratio)
		return CP_CP_RATIO;

	return l_ops->get_cp_ratio();
}

int charge_pump_get_cp_vout(void)
{
	struct charge_pump_ops *l_ops = charge_pump_get_ops();

	if (!l_ops || !l_ops->get_cp_vout)
		return -ENOTSUPP;

	return l_ops->get_cp_vout();
}

static int __init charge_pump_init(void)
{
	struct charge_pump_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_charge_pump_dev = l_dev;

	return 0;
}

static void __exit charge_pump_exit(void)
{
	kfree(g_charge_pump_dev);
	g_charge_pump_dev = NULL;
}

subsys_initcall_sync(charge_pump_init);
module_exit(charge_pump_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("charge pump driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
