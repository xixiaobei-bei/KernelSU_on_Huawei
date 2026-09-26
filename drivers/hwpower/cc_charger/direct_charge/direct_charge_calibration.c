/*
 * direct_charge_calibration.c
 *
 * calibration for direct charge
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

#include <chipset_common/hwpower/common_module/power_calibration.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_nv.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG direct_charge_cali
HWLOG_REGIST();

#define CUR_A_MIN          650000
#define CUR_A_MAX          1300000

enum dc_cali_cur_para_index {
	CUR_A_OFFSET = 0,
	CUR_B_OFFSET,
	AUX_CUR_A_OFFSET,
	AUX_CUR_B_OFFSET,
	CALI_DATA_SIZE,
};

static int g_cali_data[CALI_DATA_SIZE + 1];

static int dc_cali_check_para_validity(int value, int min, int max)
{
	if (value && (value < min || value > max))
		return -1;

	return 0;
}

static int __init dc_cali_parse_cmdline(char *p)
{
	char *token = NULL;
	int ret;
	int i;

	if (!p)
		return -1;

	for (i = 0; i < CALI_DATA_SIZE; i++) {
		token = strsep(&p, ",");
		if (!token)
			continue;
		if (kstrtoint(token, POWER_BASE_DEC, &g_cali_data[i]) != 0)
			return -1;
	}

	ret = dc_cali_check_para_validity(g_cali_data[CUR_A_OFFSET],
		CUR_A_MIN, CUR_A_MAX);
	ret += dc_cali_check_para_validity(g_cali_data[AUX_CUR_A_OFFSET],
		CUR_A_MIN, CUR_A_MAX);
	if (ret) {
		memset(g_cali_data, 0, sizeof(g_cali_data));
		return -1;
	}

	return 0;
}
early_param("direct_charger", dc_cali_parse_cmdline);

static int dc_cali_get_data(unsigned int offset, int *value, void *dev_data)
{
	if (!value)
		return -1;

	if (offset >= CALI_DATA_SIZE) {
		hwlog_err("offset %u,%u invalid\n", offset, CALI_DATA_SIZE);
		return -1;
	}

	*value = g_cali_data[offset];
	return 0;
}

static int dc_cali_set_data(unsigned int offset, int value, void *dev_data)
{
	if (offset >= CALI_DATA_SIZE) {
		hwlog_err("offset %u,%u invalid\n", offset, CALI_DATA_SIZE);
		return -1;
	}

	g_cali_data[offset] = value;
	return 0;
}

static int dc_cali_save_data(void *dev_data)
{
	return power_nv_write(POWER_NV_SCCALCUR, g_cali_data,
		sizeof(int) * CALI_DATA_SIZE);
}

static int dc_cali_show_data(char *buf, unsigned int size, void *dev_data)
{
	int cali_data[CALI_DATA_SIZE] = { 0 };

	if (!buf || (size == 0) || !power_cmdline_is_factory_mode())
		return -1;

	if (power_nv_read(POWER_NV_SCCALCUR, cali_data,
		sizeof(int) * CALI_DATA_SIZE))
		return -1;

	if (snprintf(buf, size, "%d,%d,%d,%d", cali_data[CUR_A_OFFSET],
		cali_data[CUR_B_OFFSET], cali_data[AUX_CUR_A_OFFSET],
		cali_data[AUX_CUR_B_OFFSET]) < 0) {
		hwlog_err("snprintf failed\n");
		return -1;
	}
	return 0;
}

static struct power_cali_ops dc_cali_ops = {
	.name = "dc",
	.get_data = dc_cali_get_data,
	.set_data = dc_cali_set_data,
	.save_data = dc_cali_save_data,
	.show_data = dc_cali_show_data,
};

static int __init dc_cali_init(void)
{
	power_cali_ops_register(&dc_cali_ops);
	return 0;
}

static void __exit dc_cali_exit(void)
{
}

subsys_initcall(dc_cali_init);
module_exit(dc_cali_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("calibration for direct charge module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
