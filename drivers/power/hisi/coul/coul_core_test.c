/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: test func for coul module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <securec.h>
#include "coul_core.h"
#include "coul_interface.h"

#define MIN_VALUE 0
#define MAX_VALUE 100

static int g_coul_running = 1; /* 1 is running, 0 is suspend */

static int coul_state_ops_set(const char *buffer, const struct kernel_param *kp)
{
	int run;
	pm_message_t pm = {0};
	struct platform_device *pdev = get_coul_pdev();

	if ((buffer == NULL) || (pdev == NULL))
		return -1;

	run = buffer[0] - '0';
	if (run && !g_coul_running) {
		g_coul_running = 1;
		coul_resume(pdev);
	} else if (!run && g_coul_running) {
		g_coul_running = 0;
		coul_suspend(pdev, pm);
	}
	return 0;
}

static int coul_state_ops_get(char *buffer, const struct kernel_param *kp)
{
	int ret = -1;

	if (buffer == NULL)
		return ret;
	ret = snprintf_s(buffer, PAGE_SIZE, PAGE_SIZE - 1, "%d", g_coul_running);

	return ret;
}

static struct kernel_param_ops g_coul_state_ops = {
	.set = coul_state_ops_set,
	.get = coul_state_ops_get,
};

module_param_cb(coul_running, &g_coul_state_ops, &g_coul_running, 0644);

void ss_di_show(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL) {
		coul_core_err("NULL point in %s\n", __func__);
		return;
	}

	coul_core_err("prev_pc_unusable = %d\n", di->prev_pc_unusable);
	coul_core_err("batt_ocv = %d\n", di->batt_ocv);
	coul_core_err("batt_changed_flag = %d\n", di->batt_changed_flag);
	coul_core_err("soc_limit_flag = %d\n", di->soc_limit_flag);
	coul_core_err("batt_temp = %d\n", di->batt_temp);
	coul_core_err("batt_fcc = %d\n", di->batt_fcc);
	coul_core_err("batt_limit_fcc = %d\n", di->batt_limit_fcc);
	coul_core_err("batt_rm = %d\n", di->batt_rm);
	coul_core_err("batt_ruc = %d\n", di->batt_ruc);
	coul_core_err("batt_uuc = %d\n", di->batt_uuc);
	coul_core_err("rbatt = %d\n", di->rbatt);
	coul_core_err("soc_work_interval = %d\n", di->soc_work_interval);
	coul_core_err("charging_begin_soc = %d\n", di->charging_begin_soc);
	coul_core_err("charging_state = %d\n", di->charging_state);
}

#ifdef CONFIG_HISI_DEBUG_FS
int test_cc_discharge_percent(unsigned int percent)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL) {
		coul_core_err("NULL point in %s\n", __func__);
		return -1;
	}

	percent = clamp_val(percent, MIN_VALUE, MAX_VALUE);
	di->dischg_ocv_soc = percent;

	return di->dischg_ocv_soc;
}

int test_polar_supply_table(int neg, int temp, int soc)
{
	if (neg)
		temp = -temp;
	return coul_get_polar_table_val(temp, soc);
}

#endif

