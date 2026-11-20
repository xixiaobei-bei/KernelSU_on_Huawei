/*
 * power_dsm.c
 *
 * dsm for power module
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_dsm
HWLOG_REGIST();

static struct device *g_power_dsm_dev;

static struct dsm_dev g_power_dsm_dev_cpu_buck = {
	.name = "dsm_cpu_buck",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev g_power_dsm_dev_usb = {
	.name = "dsm_usb",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_0256,
};

static struct dsm_dev g_power_dsm_dev_battery_detect = {
	.name = "dsm_battery_detect",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev g_power_dsm_dev_battery = {
	.name = "dsm_battery",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_2048,
};

static struct dsm_dev g_power_dsm_dev_charge_monitor = {
	.name = "dsm_charge_monitor",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev g_power_dsm_dev_superswitch = {
	.name = "superswitch",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev g_power_dsm_dev_smpl = {
	.name = "dsm_smpl",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev g_power_dsm_dev_pd_richtek = {
	.name = "dsm_pd_richtek",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev g_power_dsm_dev_pd = {
	.name = "dsm_pd",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_0256,
};

static struct dsm_dev g_power_dsm_dev_uscp = {
	.name = "dsm_usb_short_circuit_protect",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev g_power_dsm_dev_pmu_ocp = {
	.name = "dsm_pmu_ocp",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev g_power_dsm_dev_pmic_irq = {
	.name = "dsm_pmu_irq",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev g_power_dsm_dev_sc = {
	.name = "dsm_direct_charge_sc",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev g_power_dsm_dev_vibrator = {
	.name = "dsm_vibrator",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_1024,
};

static struct dsm_dev g_power_dsm_dev_lightstrap = {
	.name = "dsm_lightstrap",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = POWER_DSM_BUF_SIZE_0128,
};

/* define power dsm struct data */
static struct power_dsm_client g_power_dsm_client_data[] = {
	{ POWER_DSM_CPU_BUCK, "dsm_cpu_buck", NULL,
		&g_power_dsm_dev_cpu_buck },
	{ POWER_DSM_USB, "dsm_usb", NULL,
		&g_power_dsm_dev_usb },
	{ POWER_DSM_BATTERY_DETECT, "dsm_battery_detect", NULL,
		&g_power_dsm_dev_battery_detect },
	{ POWER_DSM_BATTERY, "dsm_battery", NULL,
		&g_power_dsm_dev_battery },
	{ POWER_DSM_CHARGE_MONITOR, "dsm_charge_monitor", NULL,
		&g_power_dsm_dev_charge_monitor },
	{ POWER_DSM_SUPERSWITCH, "superswitch", NULL,
		&g_power_dsm_dev_superswitch },
	{ POWER_DSM_SMPL, "dsm_smpl", NULL,
		&g_power_dsm_dev_smpl },
	{ POWER_DSM_PD_RICHTEK, "dsm_pd_richtek", NULL,
		&g_power_dsm_dev_pd_richtek },
	{ POWER_DSM_PD, "dsm_pd", NULL,
		&g_power_dsm_dev_pd },
	{ POWER_DSM_USCP, "dsm_usb_short_circuit_protect", NULL,
		&g_power_dsm_dev_uscp },
	{ POWER_DSM_PMU_OCP, "dsm_pmu_ocp", NULL,
		&g_power_dsm_dev_pmu_ocp },
	{ POWER_DSM_PMU_IRQ, "dsm_pmu_irq", NULL,
		&g_power_dsm_dev_pmic_irq },
	{ POWER_DSM_DIRECT_CHARGE_SC, "dsm_direct_charge_sc", NULL,
		&g_power_dsm_dev_sc },
	{ POWER_DSM_VIBRATOR_IRQ, "dsm_vibrator", NULL,
		&g_power_dsm_dev_vibrator },
	{ POWER_DSM_LIGHTSTRAP, "dsm_lightstrap", NULL,
		&g_power_dsm_dev_lightstrap },
};

static int power_dsm_check_type(unsigned int type)
{
	if ((type >= POWER_DSM_TYPE_BEGIN) && (type < POWER_DSM_TYPE_END))
		return 0;

	hwlog_err("invalid type=%d\n", type);
	return -1;
}

struct dsm_client *power_dsm_get_dclient(unsigned int type)
{
	int i;
	struct power_dsm_client *p_data = g_power_dsm_client_data;
	int size = ARRAY_SIZE(g_power_dsm_client_data);
	struct dsm_client *client = NULL;

	if (power_dsm_check_type(type))
		return NULL;

	hwlog_info("type=%d\n", type);

	for (i = 0; i < size; i++) {
		if (type == p_data[i].type) {
			client = p_data[i].client;
			break;
		}
	}

	if (!client) {
		hwlog_err("client is null\n");
		return NULL;
	}

	return client;
}

int power_dsm_report_dmd(unsigned int type, int err_no, const char *buf)
{
	struct dsm_client *client = power_dsm_get_dclient(type);

	if (!client || !buf) {
		hwlog_err("client or buf is null\n");
		return -1;
	}

	if (!dsm_client_ocuppy(client)) {
		dsm_client_record(client, "%s", buf);
		dsm_client_notify(client, err_no);
		hwlog_info("report type:%d, err_no:%d\n", type, err_no);
		return 0;
	}

	hwlog_err("power dsm client is busy\n");
	return -1;
}

#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
int power_dsm_report_fmd(unsigned int type, int err_no, const void *msg)
{
	int ret;
	struct dsm_client *client = power_dsm_get_dclient(type);

	if (!client || !msg) {
		hwlog_err("client or msg is null\n");
		return -1;
	}

	if (!dsm_client_ocuppy(client)) {
		ret = dsm_client_copy_ext(client, (struct message *)msg,
			sizeof(struct message));
		dsm_client_notify(client, err_no);
		hwlog_info("report type:%d, err_no:%d\n", type, err_no);
		return ret;
	}

	hwlog_err("power dsm client is busy\n");
	return -1;
}
#endif /* CONFIG_HUAWEI_DATA_ACQUISITION */

static void power_dsm_dump_specified_data(struct power_dsm_dump *data)
{
	int ret;
	char *buf = NULL;
	unsigned int max_size = POWER_DSM_BUF_SIZE_2048;
	unsigned int used;
	unsigned int unused;

	/* skip when dump disabled */
	if (!data->dump_enable)
		return;

	/* skip when platform not support */
	if (data->support && !data->support())
		return;

	buf = (char *)kzalloc(max_size + POWER_DSM_RESERVED_SIZE, GFP_KERNEL);
	if (!buf)
		return;

	if (data->pre_buf) {
		used = strlen(buf);
		unused = max_size - used;
		snprintf(buf, unused, "%s: ", data->pre_buf);
	}

	if (data->check_error) {
		used = strlen(buf);
		unused = max_size - used;
		if (!data->check_error(buf + used, unused))
			goto end;
	}

	if (data->dump) {
		used = strlen(buf);
		unused = max_size - used;
		data->dump(buf + used, unused);
	}

	ret = power_dsm_report_dmd(data->type, data->error_no, buf);
	if (!ret) {
		/* dump will disabeld when no need always report */
		if (!data->dump_always)
			data->dump_enable = false;
	}

end:
	kfree(buf);
}

void power_dsm_reset_dump_enable(struct power_dsm_dump *data, unsigned int size)
{
	unsigned int i;

	if (!data)
		return;

	for (i = 0; i < size; i++) {
		if (data[i].dump_always)
			data[i].dump_enable = true;
	}
}

void power_dsm_dump_data(struct power_dsm_dump *data, unsigned int size)
{
	unsigned int i;

	if (!data)
		return;

	for (i = 0; i < size; i++)
		power_dsm_dump_specified_data(&data[i]);
}

void power_dsm_dump_data_with_error_no(struct power_dsm_dump *data,
	unsigned int size, int err_no)
{
	unsigned int i;

	if (!data)
		return;

	for (i = 0; i < size; i++) {
		if (err_no != data[i].error_no)
			continue;

		power_dsm_dump_specified_data(&data[i]);
		break;
	}
}

#if (defined(CONFIG_HUAWEI_POWER_DEBUG) && defined(CONFIG_SYSFS))
static unsigned int g_power_dsm_type = POWER_DSM_TYPE_END;
static int g_power_dsm_errno;

static ssize_t power_dsm_set_info(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int dsm_type;
	unsigned int dsm_errno;

	if (count >= POWER_DSM_BUF_SIZE_0016) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	/* 2: numbers for dsm_type and dsm_errno */
	if (sscanf(buf, "%d %d", &dsm_type, &dsm_errno) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (power_dsm_check_type(dsm_type))
		return -EINVAL;

	g_power_dsm_type = dsm_type;
	g_power_dsm_errno = dsm_errno;
	power_dsm_report_dmd(dsm_type, dsm_errno, buf);

	return count;
}

static ssize_t power_dsm_show_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "type:%d, err_no:%d",
		g_power_dsm_type, g_power_dsm_errno);
}

static DEVICE_ATTR(info, 0640, power_dsm_show_info, power_dsm_set_info);

static struct attribute *power_dsm_attributes[] = {
	&dev_attr_info.attr,
	NULL,
};

static const struct attribute_group power_dsm_attr_group = {
	.attrs = power_dsm_attributes,
};

static struct device *power_dsm_create_group(void)
{
	return power_sysfs_create_group("hw_power", "power_dsm",
		&power_dsm_attr_group);
}

static void power_dsm_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &power_dsm_attr_group);
}
#else
static inline struct device *power_dsm_create_group(void)
{
	return NULL;
}

static inline void power_dsm_remove_group(struct device *dev)
{
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG && CONFIG_SYSFS */

static int __init power_dsm_init(void)
{
	int i;
	struct power_dsm_client *p_data = g_power_dsm_client_data;
	int size = ARRAY_SIZE(g_power_dsm_client_data);

	for (i = 0; i < size; i++) {
		if (!p_data[i].client)
			p_data[i].client = dsm_register_client(p_data[i].dev);

		if (!p_data[i].client)
			hwlog_err("%s dsm register fail\n", p_data[i].name);
		else
			hwlog_info("%s dsm register ok\n", p_data[i].name);
	}

	g_power_dsm_dev = power_dsm_create_group();

	return 0;
}

static void __exit power_dsm_exit(void)
{
	int i;
	struct power_dsm_client *p_data = g_power_dsm_client_data;
	int size = ARRAY_SIZE(g_power_dsm_client_data);

	for (i = 0; i < size; i++) {
		p_data[i].client = NULL;
		hwlog_info("dsm %s un-register ok\n", p_data[i].name);
	}

	power_dsm_remove_group(g_power_dsm_dev);
	g_power_dsm_dev = NULL;
}

subsys_initcall_sync(power_dsm_init);
module_exit(power_dsm_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("dsm for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
