/*
 * coul_calibration.c
 *
 * calibration for coul
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

#include <chipset_common/hwpower/coul/coul_calibration.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_calibration.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_nv.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG coul_cali
HWLOG_REGIST();

#define COUL_CALI_DATA_SIZE      11
#define COUL_CALI_CMD_LEN        8

static struct coul_cali_dev *g_coul_cali_dev;
static int g_coul_cali_data[COUL_CALI_MODE_END][COUL_CALI_PARA_CUR];

static const char * const g_coul_cali_device_id_table[] = {
	[COUL_CALI_MODE_MAIN] = "main",
	[COUL_CALI_MODE_AUX] = "aux",
};

static int coul_cali_get_device_id(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_coul_cali_device_id_table); i++) {
		if (!strncmp(str, g_coul_cali_device_id_table[i], strlen(str)))
			return i;
	}

	return -1;
}

int coul_cali_ops_register(struct coul_cali_ops *ops)
{
	struct coul_cali_dev *l_dev = g_coul_cali_dev;
	int dev_id;

	if (!l_dev || !ops || !ops->dev_name) {
		hwlog_err("info or ops is null\n");
		return -EINVAL;
	}

	dev_id = coul_cali_get_device_id(ops->dev_name);
	if (dev_id < 0) {
		hwlog_err("%s ops register fail\n", ops->dev_name);
		return -EINVAL;
	}

	l_dev->ops[dev_id] = ops;
	l_dev->total_ops++;

	hwlog_info("total_ops=%d %d:%s ops register ok\n",
	l_dev->total_ops, dev_id, ops->dev_name);
	return 0;
}

static int __init coul_cali_parse_cmd(char *p)
{
	int ret;

	if (!p)
		return 0;

	ret = sscanf(p, "0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x",
		&g_coul_cali_data[COUL_CALI_MODE_MAIN][COUL_CALI_PARA_CUR_A],
		&g_coul_cali_data[COUL_CALI_MODE_MAIN][COUL_CALI_PARA_CUR_B],
		&g_coul_cali_data[COUL_CALI_MODE_MAIN][COUL_CALI_PARA_VOL_A],
		&g_coul_cali_data[COUL_CALI_MODE_MAIN][COUL_CALI_PARA_VOL_B],
		&g_coul_cali_data[COUL_CALI_MODE_AUX][COUL_CALI_PARA_CUR_A],
		&g_coul_cali_data[COUL_CALI_MODE_AUX][COUL_CALI_PARA_CUR_B],
		&g_coul_cali_data[COUL_CALI_MODE_AUX][COUL_CALI_PARA_VOL_A],
		&g_coul_cali_data[COUL_CALI_MODE_AUX][COUL_CALI_PARA_VOL_B]);
	hwlog_info("parse_cmd ret=%d\n", ret);

	return 0;
}
early_param("fg_cali", coul_cali_parse_cmd);

static int coul_cali_set_data(unsigned int offset, int value, void *dev_data)
{
	int mode;
	struct coul_cali_ops *l_ops = NULL;
	struct coul_cali_dev *l_dev = g_coul_cali_dev;

	if (!l_dev)
		return -1;

	mode = l_dev->mode;
	if ((mode < COUL_CALI_MODE_BEGIN) || (mode >= COUL_CALI_MODE_END)) {
		hwlog_err("invalid coul cali mode %d\n", mode);
		return -1;
	}

	l_ops = l_dev->ops[mode];

	switch (offset) {
	case COUL_CALI_PARA_CUR_A:
		g_coul_cali_data[mode][offset] = value;
		if (!l_ops || !l_ops->set_current_gain)
			return 0;
		return l_ops->set_current_gain(value, dev_data);
	case COUL_CALI_PARA_CUR_B:
		g_coul_cali_data[mode][offset] = value;
		if (!l_ops || !l_ops->set_current_offset)
			return 0;
		return l_ops->set_current_offset(value, dev_data);
	case COUL_CALI_PARA_VOL_A:
		g_coul_cali_data[mode][offset] = value;
		if (!l_ops || !l_ops->set_voltage_gain)
			return 0;
		return l_ops->set_voltage_gain(value, dev_data);
	case COUL_CALI_PARA_VOL_B:
		g_coul_cali_data[mode][offset] = value;
		if (!l_ops || !l_ops->set_voltage_offset)
			return 0;
		return l_ops->set_voltage_offset(value, dev_data);
	default:
		return -1;
	}
}

static int coul_cali_get_data(unsigned int offset, int *value, void *dev_data)
{
	int mode;
	struct coul_cali_ops *l_ops = NULL;
	struct coul_cali_dev *l_dev = g_coul_cali_dev;

	if (!l_dev || !value)
		return -1;

	mode = l_dev->mode;
	if ((mode < COUL_CALI_MODE_BEGIN) || (mode >= COUL_CALI_MODE_END)) {
		hwlog_err("invalid coul cali mode %d\n", mode);
		return -1;
	}

	l_ops = l_dev->ops[mode];

	switch (offset) {
	case COUL_CALI_PARA_CUR_A:
	case COUL_CALI_PARA_CUR_B:
	case COUL_CALI_PARA_VOL_A:
	case COUL_CALI_PARA_VOL_B:
		*value = g_coul_cali_data[mode][offset];
		return 0;
	case COUL_CALI_PARA_CUR:
		if (!l_ops || !l_ops->get_current)
			return 0;
		return l_ops->get_current(value, dev_data);
	case COUL_CALI_PARA_VOL:
		if (!l_ops || !l_ops->get_voltage)
			return 0;
		return l_ops->get_voltage(value, dev_data);
	default:
		return -1;
	}
}

static int coul_cali_set_mode(int value, void *dev_data)
{
	int mode;
	struct coul_cali_ops *l_ops = NULL;
	struct coul_cali_dev *l_dev = g_coul_cali_dev;

	if (!l_dev)
		return -1;

	if ((value >= COUL_CALI_MODE_BEGIN) && (value < COUL_CALI_MODE_END)) {
		l_dev->mode = value;
		return 0;
	}

	mode = l_dev->mode;
	if ((mode < COUL_CALI_MODE_BEGIN) || (mode >= COUL_CALI_MODE_END)) {
		hwlog_err("invalid coul cali mode %d\n", mode);
		return -1;
	}

	l_ops = l_dev->ops[mode];

	switch (value) {
	case COUL_CALI_MODE_CALI_ENTER:
	case COUL_CALI_MODE_CALI_EXIT:
		if (!l_ops || !l_ops->set_cali_mode)
			return 0;
		return l_ops->set_cali_mode(value == COUL_CALI_MODE_CALI_ENTER, dev_data);
	default:
		return -1;
	}
}

static int coul_cali_get_mode(int *value, void *dev_data)
{
	struct coul_cali_dev *l_dev = g_coul_cali_dev;

	if (!l_dev || !value)
		return -1;

	*value = l_dev->mode;
	return 0;
}

static int coul_cali_save_data(void *dev_data)
{
	return power_nv_write(POWER_NV_CUROFFSET, g_coul_cali_data, sizeof(g_coul_cali_data));
}

static struct power_cali_ops coul_cali_ops = {
	.name = "coul",
	.set_data = coul_cali_set_data,
	.get_data = coul_cali_get_data,
	.set_mode = coul_cali_set_mode,
	.get_mode = coul_cali_get_mode,
	.save_data = coul_cali_save_data,
};

int coul_cali_get_para(int mode, int offset, int *value)
{
	struct coul_cali_dev *l_dev = g_coul_cali_dev;

	if (!l_dev || !value)
		return -1;

	if ((mode < COUL_CALI_MODE_BEGIN) || (mode >= COUL_CALI_MODE_END))
		return -1;

	if ((offset < COUL_CALI_PARA_BEGIN) || (offset >= COUL_CALI_PARA_CUR))
		return -1;

	*value = g_coul_cali_data[mode][offset];
	return 0;
}

static int __init coul_cali_init(void)
{
	struct coul_cali_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_coul_cali_dev = l_dev;
	power_cali_ops_register(&coul_cali_ops);
	return 0;
}

static void __exit coul_cali_exit(void)
{
	struct coul_cali_dev *l_dev = g_coul_cali_dev;

	if (!l_dev)
		return;

	kfree(l_dev);
	g_coul_cali_dev = NULL;
}

subsys_initcall_sync(coul_cali_init);
module_exit(coul_cali_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("coul calibration module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
