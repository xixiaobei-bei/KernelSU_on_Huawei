/*
 * thermal_perf_ctrl.c
 *
 * perf ctrl features of thermal module
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
 */

#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/thermal.h>
#include <asm/io.h>
#include <securec.h>
#include <linux/thermal_perf_ctrl.h>
#include <linux/lpm_thermal.h>

const char thermal_cdev_type_name[THERMAL_CDEV_MAX][THERMAL_NAME_LENGTH] = {
	CDEV_GPU_NAME,
	CDEV_CPU_CLUSTER0_NAME,
	CDEV_CPU_CLUSTER1_NAME,
	CDEV_CPU_CLUSTER2_NAME,
};

const char thermal_zone_name[THERMAL_ZONE_MAX][THERMAL_NAME_LENGTH] = {
	SOC_THERMAL_NAME,
	BOARD_THERMAL_NAME,
};

#define IPA_SENSOR_SYSTEM_H "system_h"
#define IPA_SENSOR_NUM 3

static int get_max_soc_temp(int *temp)
{
	unsigned int id;
	int ret = 0;
	int val = 0;
	int val_max = INT_MIN;
	int sensor_val[IPA_SENSOR_NUM] = {0};

	/* read all sensor: c0, c1, c2, gpu */
	for (id = 0; id < IPA_SENSOR_NUM; id++) {
		ret = ipa_get_sensor_value(id, &val);
		if (ret != 0) {
			pr_err("%s: ipa_get_sensor_value fail, ret=%d\n",
			       __func__, ret);
			return ret;
		}
		sensor_val[id] = val;
		if (val > val_max)
			val_max = val;
	}

	*temp = val_max;

	return ret;
}

static int get_system_temp(struct ipa_stat *status)
{
	int sensor_id, ret;
	int temp = 0;

	/* soc temp */
	ret = get_max_soc_temp(&temp);
	if (ret != 0) {
		pr_err("%s: get_max_soc_temp fail, ret=%d\n", __func__, ret);
		return -EFAULT;
	}
	status->soc_temp = temp;

	/* system_h temp */
	sensor_id = ipa_get_periph_id(IPA_SENSOR_SYSTEM_H);
	if (sensor_id < 0) {
		pr_err("%s: ipa_get_periph_id fail, ret=%d\n",
		       __func__, sensor_id);
		return -EFAULT;
	}

	ret = ipa_get_periph_value(sensor_id, &temp);
	if (ret != 0) {
		pr_err("%s: get system_h temp fail, ret=%d\n", __func__, ret);
		return -EFAULT;
	}
	status->board_temp = temp;

	return 0;
}

static int get_thermal_cdev_power(struct thermal_cdev_power *tcp)
{
	int i, ret, zone_type;
	unsigned int power = 0;

	zone_type = tcp->thermal_zone_type;
	if (zone_type >= THERMAL_ZONE_MAX || zone_type < 0) {
		pr_err("%s: thermal_zone_type %d not support\n",
		       __func__, zone_type);
		return -EINVAL;
	}

	ret = memset_s(tcp->cdev_power, sizeof(tcp->cdev_power),
		       0, sizeof(tcp->cdev_power));
	if (ret != EOK) {
		pr_err("%s: memset_s error ret=%d\n", __func__, ret);
		return -EINVAL;
	}

	for (i = 0; i < THERMAL_CDEV_MAX; i++) {
		ret = thermal_zone_cdev_get_power(thermal_zone_name[zone_type],
						  thermal_cdev_type_name[i], &power);
		if (ret == 0)
			tcp->cdev_power[i] = power;
	}

	return 0;
}

int perf_ctrl_get_ipa_stat(void __user *uarg)
{
	struct ipa_stat ipa_stat_val;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	/* get ipa status */
	if (copy_from_user(&ipa_stat_val, uarg, sizeof(struct ipa_stat))) {
		pr_err("%s: copy_from_user fail\n", __func__);
		return -EFAULT;
	}

	ret = get_ipa_status(&ipa_stat_val);
	ret += get_system_temp(&ipa_stat_val);
	if (ret != 0) {
		pr_err("%s: fail, ret=%d\n", __func__, ret);
		return ret;
	}

	if (copy_to_user(uarg, &ipa_stat_val, sizeof(struct ipa_stat))) {
		pr_err("%s: copy_to_user fail\n", __func__);
		ret = -EFAULT;
	}

	return ret;
}

int perf_ctrl_get_thermal_cdev_power(void __user *uarg)
{
	int ret;
	struct thermal_cdev_power tcp;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&tcp, uarg, sizeof(struct thermal_cdev_power))) {
		pr_err("%s: copy_from_user fail\n", __func__);
		return -EFAULT;
	}

	ret = get_thermal_cdev_power(&tcp);
	if (ret < 0) {
		pr_err("%s: fail, ret=%d\n", __func__, ret);
		return -EFAULT;
	}

	if (copy_to_user(uarg, &tcp, sizeof(struct thermal_cdev_power))) {
		pr_err("%s: copy_to_user fail\n", __func__);
		return -EFAULT;
	}

	return 0;
}
