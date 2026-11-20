/*
 * fpf2283_aux.c
 *
 * fpf2283_aux driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/raid/pq.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>
#include "../dual_loadswitch.h"
#include "fpf2283.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG fpf2283_aux
HWLOG_REGIST();

static struct fpf2283_device_info *g_fpf2283_dev;

static int fpf2283_write_byte(u8 reg, u8 value)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

static int fpf2283_read_byte(u8 reg, u8 *value)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

static int fpf2283_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = fpf2283_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return fpf2283_write_byte(reg, val);
}

static void fpf2283_dump_register(void)
{
	int ret;
	u8 i;
	u8 val = 0;

	for (i = 0; i < FPF2283_MAX_REGS; ++i) {
		ret = fpf2283_read_byte(i, &val);
		if (ret)
			hwlog_err("dump_register read fail\n");

		hwlog_info("reg [%x]=0x%x\n", i, val);
	}
}

static int fpf2283_check_i2c(struct fpf2283_device_info *di)
{
	u8 val = 0;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_read_byte(di->client, FPF2283_ID_REG, &val);
}

static int fpf2283_reg_init(void)
{
	int ret;
	int value = 0;
	char *nxp_name = DEV_NAME_NXP6093;
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = fpf2283_write_byte(FPF2283_INTERRUPT_MASK_REG,
		FPF2283_INTERRUPT_MASK_INIT);

	if (!strncmp(di->dev_name, nxp_name, strlen(nxp_name)))
		ret |= fpf2283_write_byte(FPF2283_SRW_REG,
			FPF2283_SRW_REG_INIT);

	/* ovp level: offset 0mv, center value 11.5v */
	value |= ((FPF2283_OVP_OFFSET_0MV <<
		FPF2283_OVP_OFFSET_SHIFT) & FPF2283_OVP_OFFSET_MASK);
	value |= ((FPF2283_OVP_CENTER_VALUE_11500MV <<
		FPF2283_OVP_CENTER_VALUE_SHIFT) &
		FPF2283_OVP_CENTER_VALUE_MASK);
	ret |= fpf2283_write_byte(FPF2283_OVP_REG, value);
	if (ret)
		return -1;

	fpf2283_dump_register();
	return 0;
}

static int fpf2283_charge_enable(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? 0x1 : 0x0;
	char *nxp_name = DEV_NAME_NXP6093;
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = fpf2283_write_mask(FPF2283_ENABLE_REG,
		FPF2283_ENABLE_SW_ENB_MASK, FPF2283_ENABLE_SW_ENB_SHIFT,
		!value ? 1 : 0);
	if (ret)
		return -1;

	if (!value && !strncmp(di->dev_name, nxp_name, strlen(nxp_name)))
		usleep_range(20000, 21000); /* delay 20ms */

	ret = fpf2283_write_mask(FPF2283_OVP_REG,
		FPF2283_OVP_OV_MODE_MASK, FPF2283_OVP_OV_MODE_SHIFT,
		value);
	if (ret)
		return -1;

	ret = fpf2283_read_byte(FPF2283_OVP_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("ovp_reg [%x]=0x%x\n", FPF2283_OVP_REG, reg);
	return 0;
}

static int fpf2283_charge_init(void *dev_data)
{
	int ret;
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->chip_already_init = 1;

	ret = fpf2283_reg_init();
	if (ret)
		return -1;

	di->init_finish_flag = FPF2283_INIT_FINISH;
	return 0;
}

static int fpf2283_charge_exit(void *dev_data)
{
	int ret;
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = fpf2283_charge_enable(FPF2283_OVP_OV_MODE_DISABLE, dev_data);

	di->chip_already_init = 0;
	di->init_finish_flag = FPF2283_NOT_INIT;
	di->int_notify_enable_flag = FPF2283_DISABLE_INT_NOTIFY;

	usleep_range(10000, 11000); /* sleep 10ms */

	return ret;
}

static int fpf2283_discharge(int enable, void *dev_data)
{
	return 0;
}

static int fpf2283_config_watchdog_ms(int time, void *dev_data)
{
	return 0;
}

static int fpf2283_is_ls_close(void *dev_data)
{
	u8 reg = 0;
	int ret;

	ret = fpf2283_read_byte(FPF2283_OVP_REG, &reg);
	if (ret)
		return 1;

	if (!(reg & FPF2283_OVP_OV_MODE_MASK))
		return 1;

	return 0;
}

static int fpf2283_ls_status(void *dev_data)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("ls_status=%d\n", di->chip_already_init);

	if (di->chip_already_init == 1)
		return 0;

	return -1;
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
static void fpf2283_irq_work(struct work_struct *work)
{
	struct fpf2283_device_info *di = NULL;
	struct nty_data *data = NULL;
	u8 detection_status = 0;
	u8 power_switch_flag = 0;
	u8 interrupt_mask = 0;
	int ret;

	if (!work)
		return;

	di = container_of(work, struct fpf2283_device_info, irq_work);
	if (!di || !di->client)
		return;

	data = &(di->nty_data);
	ret = fpf2283_read_byte(FPF2283_DETECTION_STATUS_REG,
		&detection_status);
	ret |= fpf2283_read_byte(FPF2283_POWER_SWITCH_FLAG_REG,
		&power_switch_flag);
	ret |= fpf2283_read_byte(FPF2283_INTERRUPT_MASK_REG,
		&interrupt_mask);
	if (ret)
		hwlog_err("irq_work read fail\n");

	data->event1 = detection_status;
	data->event2 = power_switch_flag;
	data->addr = di->client->addr;

	if (di->int_notify_enable_flag == FPF2283_ENABLE_INT_NOTIFY) {
		if (power_switch_flag & FPF2283_POWER_SWITCH_FLAG_OV_FLG_MASK) {
			hwlog_info("ovp happened\n");
			power_event_anc_notify(POWER_ANT_LVC_FAULT,
				POWER_NE_DC_FAULT_VBUS_OVP, data);
		} else if (power_switch_flag & FPF2283_POWER_SWITCH_FLAG_OC_FLG_MASK) {
			hwlog_info("ocp happened\n");
			power_event_anc_notify(POWER_ANT_LVC_FAULT,
				POWER_NE_DC_FAULT_INPUT_OCP, data);
		} else if (power_switch_flag & FPF2283_POWER_SWITCH_FLAG_OT_FLG_MASK) {
			hwlog_info("otp happened\n");
			power_event_anc_notify(POWER_ANT_LVC_FAULT,
				POWER_NE_DC_FAULT_OTP, data);
		}

		fpf2283_dump_register();
	}

	hwlog_info("detection_status_reg [%x]=0x%x\n",
		FPF2283_DETECTION_STATUS_REG, detection_status);
	hwlog_info("power_switch_flag_reg [%x]=0x%x\n",
		FPF2283_POWER_SWITCH_FLAG_REG, power_switch_flag);
	hwlog_info("interrupt_mask_reg [%x]=0x%x\n",
		FPF2283_INTERRUPT_MASK_REG, interrupt_mask);

	/* clear irq */
	enable_irq(di->irq_int);
}

static irqreturn_t fpf2283_interrupt(int irq, void *_di)
{
	struct fpf2283_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (di->chip_already_init == 0)
		hwlog_err("chip not init\n");

	if (di->init_finish_flag == FPF2283_INIT_FINISH)
		di->int_notify_enable_flag = FPF2283_ENABLE_INT_NOTIFY;

	hwlog_info("fpf2283 int happened\n");

	disable_irq_nosync(di->irq_int);
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static struct dc_ic_ops fpf2283_sysinfo_ops = {
	.dev_name = "fpf2283_aux",
	.ic_init = fpf2283_charge_init,
	.ic_exit = fpf2283_charge_exit,
	.ic_enable = fpf2283_charge_enable,
	.ic_discharge = fpf2283_discharge,
	.is_ic_close = fpf2283_is_ls_close,
	.config_ic_watchdog = fpf2283_config_watchdog_ms,
	.get_ic_status = fpf2283_ls_status,
};

#ifdef CONFIG_SYSFS
static ssize_t fpf2283_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t fpf2283_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info fpf2283_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(fpf2283, 0644, FPF2283_SYSFS_OVP_ENABLE, ovp_enable),
};

#define FPF2283_SYSFS_ATTRS_SIZE  ARRAY_SIZE(fpf2283_sysfs_field_tbl)

static struct attribute *fpf2283_sysfs_attrs[FPF2283_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group fpf2283_sysfs_attr_group = {
	.attrs = fpf2283_sysfs_attrs,
};

static void fpf2283_sysfs_init_attrs(void)
{
	int i;

	for (i = 0; i < FPF2283_SYSFS_ATTRS_SIZE; i++)
		fpf2283_sysfs_attrs[i] = &fpf2283_sysfs_field_tbl[i].attr.attr;

	fpf2283_sysfs_attrs[i] = NULL;
}

static struct power_sysfs_attr_info *fpf2283_sysfs_field_lookup(
	const char *name)
{
	int i;

	for (i = 0; i < FPF2283_SYSFS_ATTRS_SIZE; i++) {
		if (!strncmp(name,
			fpf2283_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= FPF2283_SYSFS_ATTRS_SIZE)
		return NULL;

	return &fpf2283_sysfs_field_tbl[i];
}

static ssize_t fpf2283_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	int len = 0;
	u8 value = 0;

	info = fpf2283_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case FPF2283_SYSFS_OVP_ENABLE:
		fpf2283_read_byte(FPF2283_ENABLE_REG, &value);
		len = snprintf(buf, PAGE_SIZE, "%x\n", value);
		hwlog_info("show ovp_enable=0x%x\n", value);
		break;
	default:
		break;
	}

	return len;
}

static ssize_t fpf2283_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	struct power_sysfs_attr_info *info = NULL;

	info = fpf2283_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case FPF2283_SYSFS_OVP_ENABLE:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0)) {
			hwlog_err("store ovp_enable=%ld\n", val);
			return -EINVAL;
		}
		hwlog_info("store ovp_enable=%ld\n", val);
		fpf2283_write_mask(FPF2283_ENABLE_REG,
			FPF2283_ENABLE_SW_ENB_MASK, FPF2283_ENABLE_SW_ENB_SHIFT,
			!val ? 1 : 0);
		break;
	default:
		break;
	}

	return count;
}

static void fpf2283_sysfs_create_group(struct device *dev)
{
	fpf2283_sysfs_init_attrs();
	power_sysfs_create_link_group("hw_power", "charger", "fpf2283_aux",
		dev, &fpf2283_sysfs_attr_group);
}

static void fpf2283_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "fpf2283_aux",
		dev, &fpf2283_sysfs_attr_group);
}
#else
static inline void fpf2283_sysfs_create_group(struct device *dev)
{
	return 0;
}

static inline void fpf2283_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int fpf2283_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct fpf2283_device_info *di = NULL;
	struct device_node *np = NULL;

	if (g_fpf2283_dev) {
		hwlog_err("aux driver already matched\n");
		return -ENODEV;
	}

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	di->dev_name = (char *)of_device_get_match_data(di->dev);
	hwlog_info("device is %s\n", di->dev_name);

	di->chip_already_init = 1;

	if (fpf2283_check_i2c(di)) {
		hwlog_info("i2c error, ignore this probe\n");
		devm_kfree(&client->dev, di);
		return -ENODEV;
	}
	g_fpf2283_dev = di;

	ret = fpf2283_write_mask(FPF2283_ENABLE_REG,
		FPF2283_ENABLE_SW_ENB_MASK, FPF2283_ENABLE_SW_ENB_SHIFT, 1);
	if (ret)
		hwlog_err("close sw_enb failed\n");

#ifdef POWER_MODULE_DEBUG_FUNCTION
	INIT_WORK(&di->irq_work, fpf2283_irq_work);

	ret = power_gpio_config_interrupt(np,
		"gpio_int", "fpf2283_gpio_int", &di->gpio_int, &di->irq_int);
	if (ret)
		goto fpf2283_fail_1;

	ret = request_irq(di->irq_int, fpf2283_interrupt, IRQF_TRIGGER_FALLING,
		"fpf2283_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		goto fpf2283_fail_2;
	}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

	ret = loadswitch_aux_ops_register(&fpf2283_sysinfo_ops);
	if (ret) {
		hwlog_err("register loadswitch ops failed\n");
		goto fpf2283_fail_3;
	}

	fpf2283_sysfs_create_group(di->dev);
	return 0;

fpf2283_fail_3:
#ifdef POWER_MODULE_DEBUG_FUNCTION
	free_irq(di->irq_int, di);
fpf2283_fail_2:
	gpio_free(di->gpio_int);
fpf2283_fail_1:
#endif /* POWER_MODULE_DEBUG_FUNCTION */
	devm_kfree(&client->dev, di);
	g_fpf2283_dev = NULL;

	return ret;
}

static int fpf2283_remove(struct i2c_client *client)
{
	struct fpf2283_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	fpf2283_sysfs_remove_group(di->dev);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	return 0;
}

MODULE_DEVICE_TABLE(i2c, fpf2283);
static const struct of_device_id fpf2283_of_match[] = {
	{
		.compatible = "huawei,fpf2283_aux",
		.data = DEV_NAME_FPF2283,
	},
	{
		.compatible = "huawei,nxp6093a_aux",
		.data = DEV_NAME_NXP6093,
	},
	{},
};

static const struct i2c_device_id fpf2283_i2c_id[] = {
	{ "fpf2283_aux", 0 },
	{ "nxp6093a_aux", 1 },
};

static struct i2c_driver fpf2283_driver = {
	.probe = fpf2283_probe,
	.remove = fpf2283_remove,
	.id_table = fpf2283_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "huawei,fpf2283_aux",
		.of_match_table = of_match_ptr(fpf2283_of_match),
	},
};

static int __init fpf2283_init(void)
{
	return i2c_add_driver(&fpf2283_driver);
}

static void __exit fpf2283_exit(void)
{
	i2c_del_driver(&fpf2283_driver);
}

module_init(fpf2283_init);
module_exit(fpf2283_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("fpf2283 aux module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
