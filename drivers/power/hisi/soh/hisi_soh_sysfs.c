/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: hisi_soh_sysfs.c
 *
 * soh sysfs driver
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
#include <linux/export.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <securec.h>
#include <linux/power/hisi/bci_battery.h>
#include <linux/power/hisi/soh/hisi_soh_interface.h>
#include <hisi_soh_core.h>

static BLOCKING_NOTIFIER_HEAD(hisi_soh_blocking_notifier_list);

int hisi_soh_register_blocking_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(
		&hisi_soh_blocking_notifier_list, nb);
}

int hisi_soh_unregister_blocking_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(
		&hisi_soh_blocking_notifier_list, nb);
}

int hisi_call_soh_blocking_notifiers(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(&hisi_soh_blocking_notifier_list,
		val, v);
}

#ifdef CONFIG_SYSFS
/*
 * sysfs node for set soh ctrl nv
 * close soh function by nv ctrl set
 * valid in ENG Version
 */
static struct device *soh_dev;

struct device *get_soh_sys_dev(void)
{
	return soh_dev;
}

ssize_t nv_ctrl(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int status = count;
	long val = 0;
	struct hisi_soh_device *di = get_soh_core_device();

	if (!buf) {
		hisi_soh_err("Input param buf is invalid\n");
		return -EINVAL;
	}

	if (kstrtol(buf, 0, &val) < 0)
		return -EINVAL;

	if (!di)
		return status;

#ifdef CONFIG_HISI_DEBUG_FS
	switch (val) {
	case SOH_ACR:
		di->soh_acr_dev.acr_nv.acr_control = 1;
		soh_rw_nv_info(di, SOH_ACR, NV_WRITE);
		break;
	case SOH_DCR:
		di->soh_dcr_dev.dcr_nv.dcr_control = 1;
		soh_rw_nv_info(di, SOH_DCR, NV_WRITE);
		break;
	case SOH_PD_LEAK:
		di->soh_pd_leak_dev.pd_leak_nv.pd_control = 1;
		soh_rw_nv_info(di, SOH_PD_LEAK, NV_WRITE);
		break;
	default:
		break;
	}
#endif
	return status;
}

#ifdef CONFIG_HISI_DEBUG_FS
static void clear_nvinfo(struct hisi_soh_device *di, const long val)
{
	switch (val) {
	case SOH_ACR:
		(void)memset_s(&di->soh_acr_dev.acr_nv,
			sizeof(di->soh_acr_dev.acr_nv), 0,
			sizeof(di->soh_acr_dev.acr_nv));
		(void)memset_s(&di->soh_acr_dev.soh_acr_info,
			sizeof(di->soh_acr_dev.soh_acr_info), 0,
			sizeof(di->soh_acr_dev.soh_acr_info));

		soh_rw_nv_info(di, SOH_ACR, NV_WRITE);
		break;
	case SOH_DCR:
		(void)memset_s(&di->soh_dcr_dev.dcr_nv,
			sizeof(di->soh_dcr_dev.dcr_nv), 0,
			sizeof(di->soh_dcr_dev.dcr_nv));
		(void)memset_s(&di->soh_dcr_dev.soh_dcr_info,
			sizeof(di->soh_dcr_dev.soh_dcr_info), 0,
			sizeof(di->soh_dcr_dev.soh_dcr_info));

		soh_rw_nv_info(di, SOH_DCR, NV_WRITE);
		break;
	case SOH_PD_LEAK:
		(void)memset_s(&di->soh_pd_leak_dev.pd_leak_nv,
			sizeof(di->soh_pd_leak_dev.pd_leak_nv), 0,
			sizeof(di->soh_pd_leak_dev.pd_leak_nv));
		(void)memset_s(&di->soh_pd_leak_dev.soh_pd_leak_current_info,
			sizeof(di->soh_pd_leak_dev.soh_pd_leak_current_info), 0,
			sizeof(di->soh_pd_leak_dev.soh_pd_leak_current_info));

		soh_rw_nv_info(di, SOH_PD_LEAK, NV_WRITE);
		break;
	default:
		break;
	}
}
#endif
/*
 * sysfs node for clear soh ctrl nv
 * clear soh nv, valid in ENG Version
 */
ssize_t nv_clear(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int status = count;
	long val = 0;
	struct hisi_soh_device *di = get_soh_core_device();

	if (!buf) {
		hisi_soh_err("Input param buf is invalid\n");
		return -EINVAL;
	}

	if (kstrtol(buf, 0, &val) < 0)
		return -EINVAL;

	if (!di)
		return status;

#ifdef CONFIG_HISI_DEBUG_FS
	clear_nvinfo(di, val);
#endif
	return status;
}

static enum soh_type nv_read_sel_flag;
/*
 * sel nv read soh type
 */
ssize_t nv_read_sel(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int status = count;
	long val = 0;
	struct hisi_soh_device *di = get_soh_core_device();

	if (!buf) {
		hisi_soh_err("Input param buf is invalid\n");
		return -EINVAL;
	}

	if (kstrtol(buf, 0, &val) < 0)
		return -EINVAL;

	if (!di)
		return status;

	switch (val) {
	case SOH_ACR:
		nv_read_sel_flag = SOH_ACR;
		break;
	case SOH_DCR:
		nv_read_sel_flag = SOH_DCR;
		break;
	case SOH_PD_LEAK:
		nv_read_sel_flag = SOH_PD_LEAK;
		break;
	default:
		break;
	}
	hisi_soh_info("[%s] sel = %d!\n", __func__, nv_read_sel_flag);

	return status;
}

void soh_nv_printf(struct hisi_soh_device *_di, enum soh_type type)
{
	int i;
	struct hisi_soh_device *di = _di;

	if (!di)
		return;

	switch (type) {
	case SOH_ACR:
		hisi_soh_info("[%s]ACR:order[%d]  ctrl [%d]\n",
			__func__, di->soh_acr_dev.acr_nv.order_num,
			di->soh_acr_dev.acr_nv.acr_control);
		for (i = 0; i < SOH_ACR_NV_DATA_NUM; i++)
			hisi_soh_info("[%s]Arrary[%d]: cycle:%d  acr:%d  chip_temp0:%d  chip_temp1:%d  vol:%d  bat_temp:%d\n",
				__func__, i, di->soh_acr_dev.acr_nv.soh_nv_acr_info[i].batt_cycle,
				di->soh_acr_dev.acr_nv.soh_nv_acr_info[i].batt_acr,
				di->soh_acr_dev.acr_nv.soh_nv_acr_info[i].chip_temp[0],
				di->soh_acr_dev.acr_nv.soh_nv_acr_info[i].chip_temp[1],
				di->soh_acr_dev.acr_nv.soh_nv_acr_info[i].batt_vol,
				di->soh_acr_dev.acr_nv.soh_nv_acr_info[i].batt_temp);
		break;

	case SOH_DCR:
		hisi_soh_info("[%s]DCR:order[%d]  ctrl [%d]\n",
			__func__, di->soh_dcr_dev.dcr_nv.order_num,
			di->soh_dcr_dev.dcr_nv.dcr_control);
			for (i = 0; i < SOH_DCR_NV_DATA_NUM; i++)
				hisi_soh_info("[%s]Array[%d]:cycle[%d]  dcr[%d]  r0[%d]  vol[%d]  bat_temp[%d]\n",
					__func__, i, di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[i].batt_cycle,
					di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[i].batt_dcr,
					di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[i].batt_r0,
					di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[i].batt_vol,
					di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[i].batt_temp);
		break;
	case SOH_PD_LEAK:
		hisi_soh_info("[%s]PD_LEAK:order[%d]  ctrl [%d]\n",
			__func__, di->soh_pd_leak_dev.pd_leak_nv.order_num,
			di->soh_pd_leak_dev.pd_leak_nv.pd_control);
		for (i = 0; i < SOH_PD_NV_DATA_NUM; i++)
			hisi_soh_info("[%s]Array[%d]:cycle[%d]  leak_ma[%d]  chip_t0[%d]  chip_t1[%d]  cur0[%d]  cur1[%d] vol0[%d] vol1[%d] rtc0[%d] rtc1[%d] sys_cc[%d]\n",
			      __func__, i, di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[0].batt_cycle,
				  di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].leak_current_ma,
				  di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].chip_temp[0],
				  di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].chip_temp[1],
				  di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].batt_current[0],
				  di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].batt_current[1],
				  di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].batt_vol[0],
				  di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].batt_vol[1],
				  di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].rtc_time[0],
				  di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].rtc_time[1],
				  di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].sys_pd_leak_cc);
		break;
	default:
		break;
	}
}

static int acr_nv_print (struct hisi_soh_device *di, char *buf, size_t size)
{
	int cnt;
	int ret;
	int i;

	if (size > PAGE_SIZE || size < 1)
		return -1;
	cnt = snprintf_s(buf, size, size - 1, "ACR:order[%d]  ctrl [%d]",
			di->soh_acr_dev.acr_nv.order_num,
			di->soh_acr_dev.acr_nv.acr_control);
	if (cnt < 0) {
		hisi_soh_err("[%s]snprintf_s fail, err=%d\n", __func__, cnt);
		return cnt;
	}
	for (i = 0; i < SOH_ACR_NV_DATA_NUM; i++) {
		if (size < (unsigned int)cnt + 1) {
			hisi_soh_err("[%s]buf out of bound cnt:%d\n", __func__, cnt);
			return cnt;
		}
		ret = snprintf_s(buf + cnt, size - cnt, size - cnt - 1,
			"Arrary[%d]: cycle:%d  acr:%d  chip_temp0:%d  chip_temp1:%d  vol:%d  bat_temp:%d",
			i, di->soh_acr_dev.acr_nv.soh_nv_acr_info[i].batt_cycle,
			di->soh_acr_dev.acr_nv.soh_nv_acr_info[i].batt_acr,
			di->soh_acr_dev.acr_nv.soh_nv_acr_info[i].chip_temp[0],
			di->soh_acr_dev.acr_nv.soh_nv_acr_info[i].chip_temp[1],
			di->soh_acr_dev.acr_nv.soh_nv_acr_info[i].batt_vol,
			di->soh_acr_dev.acr_nv.soh_nv_acr_info[i].batt_temp);
		if (ret < 0) {
			hisi_soh_err("[%s]snprintf_s fail, err=%d\n",
				__func__, ret);
			return ret;
		}
		cnt += ret;
	}
	return cnt;
}

static int dcr_nv_print (struct hisi_soh_device *di, char *buf, size_t size)
{
	int cnt;
	int ret;
	int i;

	if (size > PAGE_SIZE || size < 1)
		return -1;
	cnt = snprintf_s(buf, size, size - 1, "DCR:order[%d]  ctrl [%d]",
			di->soh_dcr_dev.dcr_nv.order_num,
			di->soh_dcr_dev.dcr_nv.dcr_control);
	if (cnt < 0) {
		hisi_soh_err("[%s]snprintf_s fail, err=%d\n",
			__func__, cnt);
		return cnt;
	}
	for (i = 0; i < SOH_DCR_NV_DATA_NUM; i++) {
		if (size < (unsigned int)cnt + 1) {
			hisi_soh_err("[%s]buf out of bound cnt:%d\n",
				__func__, cnt);
			return cnt;
		}
		ret = snprintf_s(buf + cnt, size - cnt, size - cnt - 1,
			"array[%d]: cycle[%d]  dcr[%d]  r0[%d]  vol[%d]  bat_temp[%d]",
			i, di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[i].batt_cycle,
			di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[i].batt_dcr,
			di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[i].batt_r0,
			di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[i].batt_vol,
			di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[i].batt_temp);
		if (ret < 0) {
			hisi_soh_err("[%s]snprintf_s fail, err=%d\n", __func__, ret);
			return ret;
		}
		cnt += ret;
	}
	return cnt;
}

static int pdleak_nv_print (struct hisi_soh_device *di, char *buf, size_t size)
{
	int cnt;
	int ret;
	int i;

	if (size > PAGE_SIZE || size < 1)
		return -1;
	cnt = snprintf_s(buf, size, size - 1, "PD_LEAK:order[%d]  ctrl [%d]",
			di->soh_pd_leak_dev.pd_leak_nv.order_num,
			di->soh_pd_leak_dev.pd_leak_nv.pd_control);
	if (cnt < 0) {
		hisi_soh_err("[%s]snprintf_s fail, err=%d\n", __func__, cnt);
		return cnt;
	}
	for (i = 0; i < SOH_PD_NV_DATA_NUM; i++) {
		if (size < (unsigned int)cnt + 1) {
			hisi_soh_err("[%s]buf out of bound cnt:%d\n", __func__, cnt);
			return cnt;
		}
		ret = snprintf_s(buf + cnt, size - cnt, size - cnt - 1,
			"Array[%d]:cycle[%d]  leak_ma[%d]  chip_t0[%d]  chip_t1[%d]  cur0[%d]  cur1[%d] vol0[%d] vol1[%d] rtc0[%d] rtc1[%d] sys_cc[%d]\n",
			i, di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[0].batt_cycle,
			di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].leak_current_ma,
			di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].chip_temp[0],
			di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].chip_temp[1],
			di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].batt_current[0],
			di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].batt_current[1],
			di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].batt_vol[0],
			di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].batt_vol[1],
			di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].rtc_time[0],
			di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].rtc_time[1],
			di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[i].sys_pd_leak_cc);
		if (ret < 0) {
			hisi_soh_err("[%s]snprintf_s fail, err=%d\n",
				__func__, ret);
			return ret;
		}
		cnt += ret;
	}
	return cnt;
}
/*
 * read nv Depend on the soh type by nv_read_sel
 */
ssize_t nv_read(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret;
	struct hisi_soh_device *di = get_soh_core_device();

	if (!di)
		return -1;

	if (!buf) {
		hisi_soh_err("Input param buf is invalid\n");
		return -EINVAL;
	}

	switch (nv_read_sel_flag) {
	case SOH_ACR:
		soh_rw_nv_info(di, SOH_ACR, NV_READ);
		ret = acr_nv_print(di, buf, PAGE_SIZE);
		if (ret < 0)
			hisi_soh_err("[%s]acr nv print fail, err=%d\n",
				__func__, ret);
		break;
	case SOH_DCR:
		soh_rw_nv_info(di, SOH_DCR, NV_READ);
		ret = dcr_nv_print(di, buf, PAGE_SIZE);
		if (ret < 0)
			hisi_soh_err("[%s]dcr nv print fail, err=%d\n",
				__func__, ret);
		break;
	case SOH_PD_LEAK:
		soh_rw_nv_info(di, SOH_PD_LEAK, NV_READ);
		ret = pdleak_nv_print(di, buf, PAGE_SIZE);
		if (ret < 0)
			hisi_soh_err("[%s]pdleak nv print fail, err=%d\n",
				__func__, ret);
		break;
	default:
		break;
	}
	return strlen(buf);
}

ssize_t acr_raw_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hisi_soh_device *di = NULL;
	struct soh_acr_device *acr_dev = NULL;
	errno_t err;

	if (!buf) {
		hisi_soh_err("Input param buf is invalid\n");
		return -EINVAL;
	}
	di = get_soh_core_device();
	if (!di)
		return -1;
	acr_dev = &di->soh_acr_dev;
	err = memcpy_s(buf, PAGE_SIZE,
		&acr_dev->soh_acr_info, sizeof(acr_dev->soh_acr_info));
	if (err != EOK) {
		hisi_soh_err("[%s]memcpy_s fail, err=%d\n", __func__, err);
		return -1;
	}
	return sizeof(acr_dev->soh_acr_info);
}

ssize_t dcr_raw_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hisi_soh_device *di = get_soh_core_device();
	struct soh_dcr_device *dcr_dev = NULL;
	errno_t err;

	if (!buf) {
		hisi_soh_err("Input param buf is invalid\n");
		return -EINVAL;
	}
	if (!di)
		return -1;
	dcr_dev = &di->soh_dcr_dev;
	err = memcpy_s(buf, PAGE_SIZE,
		&dcr_dev->soh_dcr_info, sizeof(dcr_dev->soh_dcr_info));
	if (err != EOK) {
		hisi_soh_err("[%s]memcpy_s fail, err=%d\n", __func__, err);
		return -1;
	}
	return sizeof(dcr_dev->soh_dcr_info);
}

static DEVICE_ATTR(nv_ctrl, 0640, NULL, nv_ctrl);
static DEVICE_ATTR(nv_clear, 0640, NULL, nv_clear);
static DEVICE_ATTR(nv_read, 0640, nv_read, nv_read_sel);
static DEVICE_ATTR_RO(acr_raw);
static DEVICE_ATTR_RO(dcr_raw);

static struct attribute *soh_attributes[] = {
	&dev_attr_nv_ctrl.attr,
	&dev_attr_nv_clear.attr,
	&dev_attr_nv_read.attr,
	&dev_attr_acr_raw.attr,
	&dev_attr_dcr_raw.attr,
	NULL,
};

static const struct attribute_group soh_attr_group = {
	.attrs = soh_attributes,
};

int soh_create_sysfs_file(struct hisi_soh_device *di)
{
	int retval;
	struct class *power_class = NULL;

	if (!di) {
		hisi_soh_err("%s input di is null", __func__);
		return -1;
	}

	retval = sysfs_create_group(&di->dev->kobj, &soh_attr_group);
	if (retval) {
		hisi_soh_err("%s failed to create sysfs group!!!\n", __func__);
		return -1;
	}
	power_class = power_sysfs_get_class("hw_power");
	if (power_class) {
		if (soh_dev == NULL) {
			soh_dev = device_create(power_class,
				NULL, 0, NULL, "soh");
			if (IS_ERR(soh_dev))
				soh_dev = NULL;
		}
		if (soh_dev != NULL) {
			retval = sysfs_create_link(&soh_dev->kobj, &di->dev->kobj, "soh_data");
			if (retval != 0)
				hisi_soh_err("%s failed to create sysfs link!!!\n", __func__);
		} else {
			hisi_soh_err("%s failed to create new_dev!!!\n", __func__);
		}
	}

	if (retval)
		sysfs_remove_group(&di->dev->kobj, &soh_attr_group);
	return retval;
}

void soh_sysfs_remove_file(struct hisi_soh_device *di)
{
	if (!di) {
		hisi_soh_err("%s input di is null", __func__);
		return;
	}
	if (!soh_dev) {
		hisi_soh_err("%s soh_dev is null", __func__);
		return;
	}
	sysfs_remove_link(&soh_dev->kobj, "soh_data");

	sysfs_remove_group(&di->dev->kobj, &soh_attr_group);
	put_device(soh_dev);
	device_unregister(soh_dev);
}
#else
int soh_create_sysfs_file(struct hisi_soh_device *di)
{
	return 0;
}

void soh_sysfs_remove_file(struct hisi_soh_device *di)
{
}
#endif

#ifdef CONFIG_HISI_DEBUG_FS
void test_acr_task_schedule(void)
{
	struct hisi_soh_device *di = get_soh_core_device();

	if (!di)
		return;
	queue_delayed_work(system_power_efficient_wq,
		&di->soh_acr_dev.acr_work, msecs_to_jiffies(0));
}

void test_dcr_task_schedule(void)
{
	struct hisi_soh_device *di = get_soh_core_device();

	if (!di)
		return;
	queue_delayed_work(system_power_efficient_wq,
		&di->soh_dcr_dev.dcr_work, msecs_to_jiffies(0));
}

void test_ovp_task_schedule(void)
{
	struct hisi_soh_device *di = get_soh_core_device();

	if (!di)
		return;
	queue_delayed_work(system_power_efficient_wq,
		&di->soh_ovp_dev.soh_ovp_work, msecs_to_jiffies(0));
}

int test_get_acr(enum acr_type type)
{
	struct acr_info r;

	return soh_get_acr_resistance(&r, type);
}

int test_get_dcr(void)
{
	struct dcr_info r;

	return soh_get_dcr_resistance(&r);
}

int test_get_pd_leak(void)
{
	struct pd_leak_current_info i;

	return soh_get_poweroff_leakage(&i);
}


void test_set_ovp_thd_value(enum soh_ovp_type type,
	int order, int temp, int vol_mv)
{
	struct hisi_soh_device *di = get_soh_core_device();

	if (!di)
		return;

	if (type == SOH_OVH) {
		if (order >= SOH_OVH_THRED_NUM || order < 0)
			return;
		di->soh_ovp_dev.soh_ovh_thres[order].bat_vol_mv = vol_mv;
		di->soh_ovp_dev.soh_ovh_thres[order].temp       = temp;
	} else if (type == SOH_OVL) {
		if (order >= SOH_OVH_THRED_NUM || order < 0)
			return;
		di->soh_ovp_dev.soh_ovl_thres[order].bat_vol_mv = vol_mv;
		di->soh_ovp_dev.soh_ovl_thres[order].temp       = temp;
	} else {
		di->soh_ovp_dev.soh_ovl_safe_thres.bat_vol_mv   = vol_mv;
		di->soh_ovp_dev.soh_ovl_safe_thres.temp         = temp;
	}
}

void test_set_ovp_start_time(int time)
{
	struct hisi_soh_device *di = get_soh_core_device();

	if (!di)
		return;
	di->soh_ovp_dev.soh_ovp_start_time = time;
}

void test_soh_nv_printf(enum soh_type type)
{
	struct hisi_soh_device *di = get_soh_core_device();

	if (!di)
		return;
	soh_nv_printf(di, type);
}

void test_soh_report(int type)
{
	struct hisi_soh_device *di = get_soh_core_device();

	if (!di)
		return;

	switch (type) {
	case 0: /* acr */
		sysfs_notify(&di->dev->kobj, NULL, "acr_raw");
		break;
	case 1: /* dcr */
		sysfs_notify(&di->dev->kobj, NULL, "dcr_raw");
		break;
	default:
		break;
	}
}

void test_set_ovp_protect(void)
{
	struct hisi_soh_device *di = get_soh_core_device();

	if (!di)
		return;
	soh_ovp_set_protect_threshold(di);
}

void test_set_bat_cyc_inc(enum soh_type type, int cycle)
{
	struct hisi_soh_device *di = get_soh_core_device();

	if (!di)
		return;
	if (type == SOH_ACR)
		di->soh_acr_dev.acr_cycle_inc = cycle;
	if (type == SOH_DCR)
		di->soh_dcr_dev.dcr_cycle_inc = cycle;
}
#endif
