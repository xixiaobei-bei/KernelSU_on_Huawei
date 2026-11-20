/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hisi_eis_core_sysfs.c
 *
 * eis core sysfs interface header
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

#include <linux/device.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <securec.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <linux/power/hisi/bci_battery.h>
#include <linux/power/hisi/hisi_eis.h>
#include "hisi_eis_data_archive.h"
#include "pmic/hisi_pmic_eis.h"
#include "hisi_eis_core_sysfs.h"

static struct device *eis_dev;

static BLOCKING_NOTIFIER_HEAD(hisi_eis_blocking_notifier_list);

int hisi_eis_register_blocking_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(
		&hisi_eis_blocking_notifier_list, nb);
}

int hisi_eis_unregister_blocking_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(
		&hisi_eis_blocking_notifier_list, nb);
}

int hisi_call_eis_blocking_notifiers(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(&hisi_eis_blocking_notifier_list,
		val, v);
}

ssize_t freq_raw_show(
	struct device *dev, struct device_attribute *attr, char *buf)
{
	int i;
	struct hisi_eis_device *di = NULL;
	int t[LEN_T_FREQ], c[LEN_T_FREQ], f[LEN_T_FREQ];
	int re[LEN_T_FREQ], im[LEN_T_FREQ];

	di = get_g_eis_di();
	if (di == NULL)
		return _ERROR;

	for (i = 0; i < LEN_T_FREQ; i++) {
		f[i] = di->eis_freq_dev.freq_info[i].freq;
		t[i] = di->eis_freq_dev.freq_info[i].temp;
		c[i] = di->eis_freq_dev.freq_info[i].cycle;
		re[i] = di->eis_freq_dev.freq_info[i].real;
		im[i] = di->eis_freq_dev.freq_info[i].imag;
	}

	/*
	 * 0-15 correspond to 16 frequencys
	 * f: freq, t: temp, c: chargecycle, re: EIS real, im: EIS image
	 */
	i = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"freq[1] = %d, temp[1] = %d, cyc[1] = %d, real[1] = %d, imag[1] = %d\n"
		"freq[2] = %d, temp[2] = %d, cyc[2] = %d, real[2] = %d, imag[2] = %d\n"
		"freq[3] = %d, temp[3] = %d, cyc[3] = %d, real[3] = %d, imag[3] = %d\n"
		"freq[4] = %d, temp[4] = %d, cyc[4] = %d, real[4] = %d, imag[4] = %d\n"
		"freq[5] = %d, temp[5] = %d, cyc[5] = %d, real[5] = %d, imag[5] = %d\n"
		"freq[6] = %d, temp[6] = %d, cyc[6] = %d, real[6] = %d, imag[6] = %d\n"
		"freq[7] = %d, temp[7] = %d, cyc[7] = %d, real[7] = %d, imag[7] = %d\n"
		"freq[8] = %d, temp[8] = %d, cyc[8] = %d, real[8] = %d, imag[8] = %d\n"
		"freq[9] = %d, temp[9] = %d, cyc[9] = %d, real[9] = %d, imag[9] = %d\n"
		"freq[10] = %d, temp[10] = %d, cyc[10] = %d, real[10] = %d, imag[10] = %d\n"
		"freq[11] = %d, temp[11] = %d, cyc[11] = %d, real[11] = %d, imag[11] = %d\n"
		"freq[12] = %d, temp[12] = %d, cyc[12] = %d, real[12] = %d, imag[12] = %d\n"
		"freq[13] = %d, temp[13] = %d, cyc[13] = %d, real[13] = %d, imag[13] = %d\n"
		"freq[14] = %d, temp[14] = %d, cyc[14] = %d, real[14] = %d, imag[14] = %d\n"
		"freq[15] = %d, temp[15] = %d, cyc[15] = %d, real[15] = %d, imag[15] = %d\n"
		"freq[16] = %d, temp[16] = %d, cyc[16] = %d, real[16] = %d, imag[16] = %d\n",
		f[0], t[0], c[0], re[0], im[0], f[1], t[1], c[1], re[1], im[1],
		f[2], t[2], c[2], re[2], im[2], f[3], t[3], c[3], re[3], im[3],
		f[4], t[4], c[4], re[4], im[4], f[5], t[5], c[5], re[5], im[5],
		f[6], t[6], c[6], re[6], im[6], f[7], t[7], c[7], re[7], im[7],
		f[8], t[8], c[8], re[8], im[8], f[9], t[9], c[9], re[9], im[9],
		f[10], t[10], c[10], re[10], im[10], f[11], t[11], c[11],
		re[11], im[11], f[12], t[12], c[12], re[12], im[12], f[13],
		t[13], c[13], re[13], im[13], f[14], t[14], c[14], re[14],
		im[14], f[15], t[15], c[15], re[15], im[15]);
	if (i < 0) {
		hisi_eis_err("snprintf_s failed\n");
		return -1;
	}
	return i;
}

static DEVICE_ATTR(freq_raw_data, 0444, freq_raw_show, NULL);

static struct attribute *eis_attributes[] = {
	&dev_attr_freq_raw_data.attr,
	NULL,
};

static const struct attribute_group eis_attr_group = {
	.attrs = eis_attributes,
};

int eis_create_sysfs_file(struct hisi_eis_device *di)
{
	int retval;
	struct class *power_class = NULL;

	if (di == NULL) {
		hisi_eis_err("_di is null!\n");
		return _ERROR;
	}
	retval = sysfs_create_group(&di->dev->kobj, &eis_attr_group);
	if (retval != 0) {
		hisi_eis_err("failed to create sysfs group!\n");
		return _ERROR;
	}
	power_class = power_sysfs_get_class("hw_power");
	if (power_class != NULL) {
		if (eis_dev == NULL) {
			eis_dev = device_create(
				power_class, NULL, 0, "%s", "eis");
			if (IS_ERR(eis_dev))
				eis_dev = NULL;
		}
		if (eis_dev != NULL) {
			retval = sysfs_create_link(&eis_dev->kobj,
				&di->dev->kobj, "eis_data");
			if (retval != 0)
				hisi_eis_err("failed to create sysfs link!\n");
		} else {
			hisi_eis_err("failed to create new_dev!\n");
		}
	}
	if (retval != 0)
		sysfs_remove_group(&di->dev->kobj, &eis_attr_group);
	return retval;
}

