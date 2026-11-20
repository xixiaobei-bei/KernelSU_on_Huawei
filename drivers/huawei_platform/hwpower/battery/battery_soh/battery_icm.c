/*
 * battery_icm.c
 *
 * driver adapter for icm.
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

#include "battery_icm.h"
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/power/hisi/hisi_eis.h>
#include <linux/power/hisi/soh/hisi_soh_interface.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG battery_icm
HWLOG_REGIST();

#define RSV_END_SPACE 2

static struct icm_device_info *g_icm_di;
static struct kobject *g_sysfs_bsoh;

static int dump_acr_to_buf(struct acr_info *acr, char *buf, int buf_size)
{
	int ret;

	if (!acr || !buf) {
		hwlog_err("invalid acr or buf\n");
		return -EINVAL;
	}

	ret = snprintf(buf, buf_size,
		"cycle: %d,acr: %d,chip_temp0: %d,chip_temp1: %d,vol: %d,bat_temp: %d\n",
		acr->batt_cycle, acr->batt_acr, acr->chip_temp[0],
		acr->chip_temp[1], acr->batt_vol, acr->batt_temp);
	if (ret <= 0)
		hwlog_err("dump acr info failed\n");

	hwlog_info("acr_info: %s\n", buf);
	return ret;
}

static void icm_soh_uevent_work(struct work_struct *work)
{
	int ret;
	char *event_buf = NULL;
	struct icm_device_info *di = container_of(work,
		struct icm_device_info, soh_uevent_work.work);

	event_buf = kzalloc(BSOH_EVENT_NOTIFY_SIZE, GFP_KERNEL);
	if (!event_buf)
		return;

	hwlog_info("soh_evt_to_send :%d\n", di->soh_evt_to_send);
	if (di->soh_evt_to_send & BIT(EVT_SOH_ACR_UPDATE)) {
		ret = dump_acr_to_buf(&di->acr_info, event_buf, BSOH_EVENT_NOTIFY_SIZE);
		if (ret > 0)
			bsoh_uevent_rcv(BSOH_EVT_SOH_ACR_UPDATE, event_buf);

		di->soh_evt_to_send &= ~BIT(EVT_SOH_ACR_UPDATE);
	}

	kfree(event_buf);
}

static int icm_soh_notifier_call(struct notifier_block *soh_nb,
	unsigned long event, void *data)
{
	struct icm_device_info *di = container_of(soh_nb,
		struct icm_device_info, soh_nb);

	hwlog_info("event: %ul\n", event);
	if (event >= EVT_SOH_END)
		return NOTIFY_OK;

	switch (event) {
	case EVT_SOH_ACR_UPDATE:
		if (!data)
			return NOTIFY_OK;
		di->acr_info = *((struct acr_info *)data);
		di->soh_evt_to_send |= BIT(EVT_SOH_ACR_UPDATE);
		power_event_notify_sysfs(g_sysfs_bsoh, "icm", "soh_acr");
		break;
	default:
		break;
	}

	schedule_delayed_work(&di->soh_uevent_work, 0);
	return NOTIFY_OK;
}

static int str_int_array(const char *name, const int *array, int nums, char *out, int out_size)
{
	int i;
	int ret;
	int len;

	if (!name || !array || !out)
		return -EINVAL;

	ret = snprintf(out, out_size - 1, "%s: [", name);
	if (ret <= 0)
		return -1;

	len = ret;
	for (i = 0; i < nums && len < (out_size - 1); i++) {
		ret = snprintf(out + len, out_size - len - 1, "%d ", array[i]);
		if (ret <= 0)
			return -1;
		len += ret;
	}
	out[len - 1] = ']';
	out[len++] = '\0';
	hwlog_info("out: %s\n", out);
	return len;
}

static int dump_eis_freq_to_buf(struct eis_freq_array *eis_freq_a, char *buf, int buf_size)
{
	int len = 0;

	if (!eis_freq_a || !buf) {
		hwlog_err("eis_freq or buf is null\n");
		return -EINVAL;
	}

	len = str_int_array("freq", eis_freq_a->freq, LEN_T_FREQ,
		buf, BSOH_EVENT_NOTIFY_SIZE - len - RSV_END_SPACE);
	if ((len <= 0) || ((BSOH_EVENT_NOTIFY_SIZE - len - RSV_END_SPACE) <= 0))
		goto dump_failed;
	buf[len - 1] = ',';

	len += str_int_array("temp", eis_freq_a->temp, LEN_T_FREQ,
		buf + len, BSOH_EVENT_NOTIFY_SIZE - len - RSV_END_SPACE);
	if ((len <= 0) || ((BSOH_EVENT_NOTIFY_SIZE - len - RSV_END_SPACE) <= 0))
		goto dump_failed;
	buf[len - 1] = ',';

	len += str_int_array("cyc", eis_freq_a->cycle, LEN_T_FREQ,
		buf + len, BSOH_EVENT_NOTIFY_SIZE - len - RSV_END_SPACE);
	if ((len <= 0) || ((BSOH_EVENT_NOTIFY_SIZE - len - RSV_END_SPACE) <= 0))
		goto dump_failed;
	buf[len - 1] = ',';

	len += str_int_array("real", eis_freq_a->real, LEN_T_FREQ,
		buf + len, BSOH_EVENT_NOTIFY_SIZE - len - RSV_END_SPACE);
	if ((len <= 0) || ((BSOH_EVENT_NOTIFY_SIZE - len - RSV_END_SPACE) <= 0))
		goto dump_failed;
	buf[len - 1] = ',';

	len += str_int_array("img", eis_freq_a->imag, LEN_T_FREQ,
		buf + len, BSOH_EVENT_NOTIFY_SIZE - len - RSV_END_SPACE);
	if ((len <= 0) || ((BSOH_EVENT_NOTIFY_SIZE - len - RSV_END_SPACE) <= 0))
		goto dump_failed;
	buf[len - 1] = '\n';
	buf[len++] = '\0';

	hwlog_info("eis_freq_str %s", buf);
	return len;

dump_failed:
	hwlog_info("dump_failed\n");
	return -1;
}

static void icm_eis_uevent_work(struct work_struct *work)
{
	int ret;
	char *event_buf = NULL;
	struct icm_device_info *di = container_of(work,
		struct icm_device_info, eis_uevent_work.work);

	event_buf = kzalloc(BSOH_EVENT_NOTIFY_SIZE, GFP_KERNEL);
	if (!event_buf)
		return;

	hwlog_info("eis_evt_to_send: %d\n", di->eis_evt_to_send);
	if (di->eis_evt_to_send & BIT(EVT_EIS_FREQ_UPDATE)) {
		ret = dump_eis_freq_to_buf(&di->eis_freq_a, event_buf, BSOH_EVENT_NOTIFY_SIZE);
		if (ret > 0)
			bsoh_uevent_rcv(BSOH_EVT_EIS_FREQ_UPDATE, event_buf);

		di->soh_evt_to_send &= ~BIT(EVT_EIS_FREQ_UPDATE);
	}

	kfree(event_buf);
}

static void trans_array_eis_freq(struct icm_device_info *di)
{
	int i;
	int nums = di->eis_freq_infos.num_of_info;

	for (i = 0; i < nums; i++) {
		di->eis_freq_a.freq[i] = di->eis_freq_infos.freq_infos[i].freq;
		di->eis_freq_a.temp[i] = di->eis_freq_infos.freq_infos[i].temp;
		di->eis_freq_a.cycle[i] = di->eis_freq_infos.freq_infos[i].cycle;
		di->eis_freq_a.real[i] = (int)di->eis_freq_infos.freq_infos[i].real;
		di->eis_freq_a.imag[i] = (int)di->eis_freq_infos.freq_infos[i].imag;
	}
}

static int icm_eis_notifier_call(struct notifier_block *eis_nb,
	unsigned long event, void *data)
{
	struct icm_device_info *di = container_of(eis_nb,
		struct icm_device_info, eis_nb);

	hwlog_info("event: %ul\n", event);
	if (event >= EVT_EIS_END)
		return NOTIFY_OK;

	switch (event) {
	case EVT_EIS_FREQ_UPDATE:
		if (!data)
			return NOTIFY_OK;
		di->eis_freq_infos = *(struct eis_freq_infos *)data;
		trans_array_eis_freq(di);
		di->eis_evt_to_send |= BIT(EVT_EIS_FREQ_UPDATE);
		power_event_notify_sysfs(g_sysfs_bsoh, "icm", "eis_freq");
		break;
	default:
		break;
	}

	schedule_delayed_work(&di->eis_uevent_work, 0);
	return NOTIFY_OK;
}

#ifdef CONFIG_SYSFS
static ssize_t soh_acr_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	if (!g_icm_di) {
		hwlog_err("g_icm_di is null\n");
		return -ENODEV;
	}

	return dump_acr_to_buf(&g_icm_di->acr_info, buff, PAGE_SIZE);
}

static ssize_t soh_acr_store(struct device *dev,
	struct device_attribute *attr, const char *buff, size_t count)
{
	int ret;

	if (!g_icm_di) {
		hwlog_err("g_icm_di is null\n");
		return -ENODEV;
	}

	ret = sscanf(buff, "cycle: %d,acr: %d,chip_temp0: %d,"
		"chip_temp1: %d,vol: %d,bat_temp: %d",
		&g_icm_di->acr_info.batt_cycle, &g_icm_di->acr_info.batt_acr,
		&(g_icm_di->acr_info.chip_temp[0]), &(g_icm_di->acr_info.chip_temp[1]),
		&g_icm_di->acr_info.batt_vol, &g_icm_di->acr_info.batt_temp);
	if (ret <= 0) {
		hwlog_err("failed to sscanf acr info\n");
		return -1;
	}

	g_icm_di->soh_evt_to_send |= BIT(EVT_SOH_ACR_UPDATE);
	power_event_notify_sysfs(g_sysfs_bsoh, "icm", "soh_acr");
	schedule_delayed_work(&g_icm_di->soh_uevent_work, 0);

	return ret;
}

static ssize_t eis_freq_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	if (!g_icm_di) {
		hwlog_err("g_icm_di is null\n");
		return -ENODEV;
	}

	return dump_eis_freq_to_buf(&g_icm_di->eis_freq_a, buff, PAGE_SIZE);
}

static ssize_t eis_freq_store(struct device *dev,
	struct device_attribute *attr, const char *buff, size_t count)
{
	int ret;

	if (!g_icm_di) {
		hwlog_err("g_icm_di is null\n");
		return -ENODEV;
	}

	ret = sscanf(buff, "freq: %d,temp: %d,cyc: %d,real: %d,img: %d",
		&(g_icm_di->eis_freq_a.freq[0]), &(g_icm_di->eis_freq_a.temp[0]),
		&(g_icm_di->eis_freq_a.cycle[0]), &(g_icm_di->eis_freq_a.real[0]),
		&(g_icm_di->eis_freq_a.imag[0]));
	if (ret <= 0) {
		hwlog_err("failed to sscanf eis_freq[0]\n");
		return -1;
	}

	g_icm_di->soh_evt_to_send |= BIT(EVT_EIS_FREQ_UPDATE);
	power_event_notify_sysfs(g_sysfs_bsoh, "icm", "eis_freq");
	schedule_delayed_work(&g_icm_di->eis_uevent_work, 0);

	return ret;
}

static DEVICE_ATTR(soh_acr, 0640, soh_acr_show, soh_acr_store);
static DEVICE_ATTR(eis_freq, 0640, eis_freq_show, eis_freq_store);

static struct attribute *g_icm_attrs[] = {
	&dev_attr_soh_acr.attr,
	&dev_attr_eis_freq.attr,
	NULL,
};

static struct attribute_group g_icm_group = {
	.name = "icm",
	.attrs = g_icm_attrs,
};

static int icm_sysfs_create_group(struct bsoh_device *di)
{
	g_sysfs_bsoh = &di->dev->kobj;
	return sysfs_create_group(g_sysfs_bsoh, &g_icm_group);
}

static void icm_sysfs_remove_group(struct bsoh_device *di)
{
	sysfs_remove_group(&di->dev->kobj, &g_icm_group);
}
#else
static inline int icm_sysfs_create_group(struct bsoh_device *di)
{
	return 0;
}

static inline void icm_sysfs_remove_group(struct bsoh_device *di)
{
}
#endif /* CONFIG_SYSFS */

static int icm_sys_init(struct bsoh_device *b_di)
{
	int ret;
	struct icm_device_info *info = NULL;

	hwlog_info("icm init begin\n");

	if (!b_di || !b_di->dev)
		return -EINVAL;

	info = kzalloc(sizeof(struct icm_device_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->b_di = b_di;
	info->soh_nb.notifier_call = icm_soh_notifier_call;
	hisi_soh_register_blocking_notifier(&info->soh_nb);
	info->eis_nb.notifier_call = icm_eis_notifier_call;
	hisi_eis_register_blocking_notifier(&info->eis_nb);

	INIT_DELAYED_WORK(&info->soh_uevent_work, icm_soh_uevent_work);
	INIT_DELAYED_WORK(&info->eis_uevent_work, icm_eis_uevent_work);

	ret = icm_sysfs_create_group(b_di);
	if (ret < 0)
		goto sysfs_create_failed;

	g_icm_di = info;
	hwlog_info("icm init ok\n");
	return 0;

sysfs_create_failed:
	hisi_soh_unregister_blocking_notifier(&info->soh_nb);
	hisi_soh_unregister_blocking_notifier(&info->eis_nb);
	kfree(info);
	return -1;
}

static void icm_sys_exit(struct bsoh_device *di)
{
	if (!di || !g_icm_di)
		return;

	hisi_soh_unregister_blocking_notifier(&g_icm_di->soh_nb);
	hisi_soh_unregister_blocking_notifier(&g_icm_di->eis_nb);
	icm_sysfs_remove_group(di);
	g_sysfs_bsoh = NULL;
	kfree(g_icm_di);
	g_icm_di = NULL;
}

static const struct bsoh_sub_sys g_icm_sys = {
	.sys_init = icm_sys_init,
	.sys_exit = icm_sys_exit,
	.event_notify = NULL,
	.dmd_prepare = NULL,
	.type_name = "icm",
	.notify_node = NULL,
};

static int __init icm_init(void)
{
	bsoh_register_sub_sys(BSOH_SUB_SYS_ICM, &g_icm_sys);
	return 0;
}

static void __exit icm_exit(void)
{
}

subsys_initcall(icm_init);
module_exit(icm_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery icm driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
