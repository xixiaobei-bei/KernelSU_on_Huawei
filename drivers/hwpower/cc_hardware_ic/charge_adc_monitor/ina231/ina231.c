/*
 * ina231.c
 *
 * ina231 driver
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

#include "ina231.h"
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic.h>

#define HWLOG_TAG ina231_for_charge
HWLOG_REGIST();

static struct ina231_device_info *g_ina231_dev;

/*
 * current_lsb = (maximum expected current) / 2^15
 * callibration = 0.00512 / (current_lsb * Rshunt)
 * current = (shunt_voltage * calibration) / 2048
 * power = (current * bus_voltage) / 20000
 */
static struct ina231_config_data ina231_config = {
	/*
	 * INA231_CONFIG_REG [0x4124]:
	 * number of averages:1, Vbus ct:1.1ms,
	 * Vsh ct:1.1ms, powerdown mode
	 */
	.config_sleep_in =
		((INA231_CONFIG_MODE_PD1 << INA231_CONFIG_MODE_SHIFT) |
		(INA231_CONFIG_CT_1100US << INA231_CONFIG_VSHUNTCT_SHIFT) |
		(INA231_CONFIG_CT_1100US << INA231_CONFIG_VBUSCT_SHIFT) |
		(INA231_CONFIG_AVG_NUM_1 << INA231_CONFIG_AVG_SHIFT)),

	/* INA231_CONFIG_REG [0x8000]: reset the whole chip */
	.config_reset = (INA231_CONFIG_RST_ENABLE << INA231_CONFIG_RST_SHIFT),

	/*
	 * INA231_CONFIG_REG [0x4377]:
	 * number of averages:4, Vbus ct:2.116ms,
	 * Vsh ct:4.156ms, shunt and bus continous mode
	 */
	.config_work =
		 ((INA231_CONFIG_MODE_SB_CONS << INA231_CONFIG_MODE_SHIFT) |
		 (INA231_CONFIG_CT_4156US << INA231_CONFIG_VSHUNTCT_SHIFT) |
		 (INA231_CONFIG_CT_2116US << INA231_CONFIG_VBUSCT_SHIFT) |
		 (INA231_CONFIG_AVG_NUM_4 << INA231_CONFIG_AVG_SHIFT)),

	/* INA231_CALIBRATION_REG [0x1000] */
	.calibrate_content = 0x400,

	/* INA231_MASK_ENABLE_REG [0x4001] */
	.mask_enable_content =
		((1 << INA231_MASK_ENABLE_LEN_SHIFT) |
		(1 << INA231_MASK_ENABLE_SUL_SHIFT)),

	/* ALERT_LIMIT [0x320] */
	.alert_limit_content = 0x320, /* shunt 2mV(200mA) under-voltage */

	.shunt_voltage_lsb = 2500, /* 2500 nV/bit */
	.bus_voltage_lsb = 1250, /* 1250 uV/bit */
	.current_lsb = 500, /* 500 uA/bit */
};

static ssize_t ina231_cali_dbg_show(void *dev_data,
	char *buf, size_t size)
{
	struct ina231_device_info *dev_p = dev_data;

	if (!buf || !dev_p || !dev_p->config) {
		hwlog_err("buf or dev_p is null\n");
		return scnprintf(buf, size, "buf or dev_p is null\n");
	}

	return scnprintf(buf, size,
		"ina231_calibrate_content=0x%x\n",
		dev_p->config->calibrate_content);
}

static ssize_t ina231_cali_dbg_store(void *dev_data,
	const char *buf, size_t size)
{
	struct ina231_device_info *dev_p = dev_data;
	u16 calibrate_content = 0;

	if (!buf || !dev_p || !dev_p->config) {
		hwlog_err("buf or dev_p is null\n");
		return -EINVAL;
	}

	if (kstrtou16(buf, 0, &calibrate_content) < 0) {
		hwlog_err("get kstrtou16 failed:%s\n", buf);
		return -EINVAL;
	}

	dev_p->config->calibrate_content = calibrate_content;

	hwlog_info("calibrate_content=0x%x\n",
		dev_p->config->calibrate_content);

	return size;
}

static ssize_t ina231_mask_dbg_show(void *dev_data,
	char *buf, size_t size)
{
	struct ina231_device_info *dev_p = dev_data;

	if (!buf || !dev_p || !dev_p->config) {
		hwlog_err("buf or dev_p is null\n");
		return scnprintf(buf, size, "buf or dev_p is null\n");
	}

	return scnprintf(buf, size,
		"ina231_mask_content=0x%x\n",
		dev_p->config->mask_enable_content);
}

static ssize_t ina231_mask_dbg_store(void *dev_data,
	const char *buf, size_t size)
{
	struct ina231_device_info *dev_p = dev_data;
	u16 mask_content = 0;

	if (!buf || !dev_p || !dev_p->config) {
		hwlog_err("buf or dev_p is null\n");
		return -EINVAL;
	}

	if (kstrtou16(buf, 0, &mask_content) < 0) {
		hwlog_err("get kstrtou16 failed:%s\n", buf);
		return -EINVAL;
	}

	dev_p->config->mask_enable_content = mask_content;

	hwlog_info("mask_enable_content=0x%x\n",
		dev_p->config->mask_enable_content);

	return size;
}

static ssize_t ina231_alert_dbg_show(void *dev_data,
	char *buf, size_t size)
{
	struct ina231_device_info *dev_p = dev_data;

	if (!buf || !dev_p || !dev_p->config) {
		hwlog_err("buf or dev_p is null\n");
		return scnprintf(buf, size, "buf or dev_p is null\n");
	}

	return scnprintf(buf, size,
		"ina231_alert_content=0x%x\n",
		dev_p->config->alert_limit_content);
}

static ssize_t ina231_alert_dbg_store(void *dev_data,
	const char *buf, size_t size)
{
	struct ina231_device_info *dev_p = dev_data;
	u16 alert_content = 0;

	if (!buf || !dev_p || !dev_p->config) {
		hwlog_err("buf or dev_p is null\n");
		return -EINVAL;
	}

	if (kstrtou16(buf, 0, &alert_content) < 0) {
		hwlog_err("get kstrtou16 failed:%s\n", buf);
		return -EINVAL;
	}

	dev_p->config->alert_limit_content = alert_content;

	hwlog_info("alert_content=0x%x\n",
		dev_p->config->alert_limit_content);

	return size;
}

static void ina231_dbg_register(struct i2c_client *client)
{
	power_dbg_ops_register("ina231_cali", i2c_get_clientdata(client),
		(power_dbg_show)ina231_cali_dbg_show,
		(power_dbg_store)ina231_cali_dbg_store);
	power_dbg_ops_register("ina231_mask", i2c_get_clientdata(client),
		(power_dbg_show)ina231_mask_dbg_show,
		(power_dbg_store)ina231_mask_dbg_store);
	power_dbg_ops_register("ina231_alert", i2c_get_clientdata(client),
		(power_dbg_show)ina231_alert_dbg_show,
		(power_dbg_store)ina231_alert_dbg_store);
}

static int ina231_get_shunt_voltage_mv(int *val)
{
	struct ina231_device_info *di = g_ina231_dev;
	s16 reg_value;
	int lsb_value;
	int ret;

	if (!di || !di->client || !di->config || !val) {
		hwlog_err("di or client is null\n");
		return -1;
	}

	lsb_value = di->config->shunt_voltage_lsb;

	ret = i2c_smbus_read_word_swapped(di->client, INA231_SHUNT_VOLTAGE_REG);
	if (ret < 0) {
		hwlog_err("shunt_voltage_reg read fail\n");
		return -1;
	}

	reg_value = (s16)ret;
	*val = (int)(reg_value * lsb_value) / INA231_NV_TO_MV;

	hwlog_info("shunt_voltage=%d, VOLTAGE_REG=0x%x\n", *val, reg_value);
	return 0;
}

static int ina231_get_bus_voltage_mv(int *val, void *dev_data)
{
	struct ina231_device_info *di = g_ina231_dev;
	s16 reg_value;
	int lsb_value;
	int ret;

	if (!di || !di->client || !di->config || !val) {
		hwlog_err("di or client is null\n");
		return -1;
	}

	lsb_value = di->config->bus_voltage_lsb;

	ret = i2c_smbus_read_word_swapped(di->client, INA231_BUS_VOLTAGE_REG);
	if (ret < 0) {
		hwlog_err("bus_voltage_reg read fail\n");
		return -1;
	}

	reg_value = (s16)ret;
	*val = (int)(reg_value * lsb_value) / INA231_UV_TO_MV;

	hwlog_info("voltage=%d, VOLTAGE_REG=0x%x\n", *val, reg_value);
	return 0;
}

static int ina231_get_current_ma(int *val, void *dev_data)
{
	struct ina231_device_info *di = g_ina231_dev;
	s16 reg_value;
	int lsb_value;
	int ret;

	if (!di || !di->client || !di->config || !val) {
		hwlog_err("di or client is null\n");
		return -1;
	}

	lsb_value = di->config->current_lsb;

	ret = i2c_smbus_read_word_swapped(di->client, INA231_CURRENT_REG);
	if (ret < 0) {
		hwlog_err("current_reg read fail\n");
		return -1;
	}

	reg_value = (s16)ret;
	*val = (int)(reg_value * lsb_value) / INA231_UA_TO_MA;

	hwlog_info("current=%d, CURRENT_REG=0x%x\n", *val, reg_value);
	return 0;
}

static int ina231_get_vbat_mv(void *dev_data)
{
	return hw_battery_voltage(BAT_ID_MAX);
}

static int ina231_get_ibat_ma(int *val, void *dev_data)
{
	if (!val)
		return -1;

	*val = -power_platform_get_battery_current();

	return 0;
}

static int ina231_get_device_temp(int *temp, void *dev_data)
{
	if (!temp)
		return -1;

	*temp = INA231_DEVICE_DEFAULT_TEMP;

	return 0;
}

static int ina231_dump_register(void)
{
	struct ina231_device_info *di = g_ina231_dev;
	int i;
	int shunt_volt = 0;
	int bus_volt = 0;
	int cur = 0;

	if (!di || !di->client) {
		hwlog_err("di or client is null\n");
		return -1;
	}

	for (i = 0; i < INA231_MAX_REGS; ++i)
		hwlog_info("reg [%d]=0x%x\n",
			i, i2c_smbus_read_word_swapped(di->client, i));

	ina231_get_shunt_voltage_mv(&shunt_volt);
	ina231_get_bus_voltage_mv(&bus_volt, di);
	ina231_get_current_ma(&cur, di);

	hwlog_info("shunt_voltage=%d, bus_voltage=%d, current=%d\n",
		shunt_volt, bus_volt, cur);

	return 0;
}

static int ina231_device_reset(void)
{
	struct ina231_device_info *di = g_ina231_dev;
	int ret;

	if (!di || !di->client || !di->config) {
		hwlog_err("di or client is null\n");
		return -1;
	}

	/* communication check and reset device */
	ret = i2c_smbus_write_word_swapped(di->client,
		INA231_CONFIG_REG, di->config->config_reset);
	if (ret < 0) {
		hwlog_err("device_reset fail\n");
		return -1;
	}

	(void)power_msleep(DT_MSLEEP_20MS, 0, NULL);

	/* device goto sleep */
	i2c_smbus_write_word_swapped(di->client,
		INA231_CONFIG_REG, di->config->config_sleep_in);
	if (ret < 0) {
		hwlog_err("device_reset fail\n");
		return -1;
	}

	return 0;
}

static int ina231_reg_init(void *dev_data)
{
	struct ina231_device_info *di = g_ina231_dev;
	int ret;

	if (!di || !di->client || !di->config) {
		hwlog_err("di or client is null\n");
		return -1;
	}

	ret = i2c_smbus_write_word_swapped(di->client,
		INA231_CONFIG_REG, di->config->config_work);
	ret += i2c_smbus_write_word_swapped(di->client,
		INA231_CALIBRATION_REG, di->config->calibrate_content);
	ret += i2c_smbus_write_word_swapped(di->client,
		INA231_ALERT_LIMIT_REG, di->config->alert_limit_content);
	if (ret) {
		hwlog_err("reg_init fail\n");
		return -1;
	}

	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);

	di->init_finish_flag = INA231_INIT_FINISH;

	return 0;
}

static int ina231_batinfo_exit(void *dev_data)
{
	return 0;
}

static struct dc_batinfo_ops ina231_batinfo_ops = {
	.init = ina231_reg_init,
	.exit = ina231_batinfo_exit,
	.get_bat_btb_voltage = ina231_get_vbat_mv,
	.get_bat_package_voltage = ina231_get_vbat_mv,
	.get_vbus_voltage = ina231_get_bus_voltage_mv,
	.get_bat_current = ina231_get_ibat_ma,
	.get_ic_ibus = ina231_get_current_ma,
	.get_ic_temp = ina231_get_device_temp,
};

static void ina231_int_en(bool en)
{
	struct ina231_device_info *di = g_ina231_dev;
	unsigned long flags;

	if (!di || !di->client) {
		hwlog_err("di or client is null\n");
		return;
	}

	spin_lock_irqsave(&di->int_lock, flags);
	if (en != di->is_int_en) {
		di->is_int_en = en;
		if (en)
			enable_irq(di->irq_int);
		else
			disable_irq_nosync(di->irq_int);
	}
	spin_unlock_irqrestore(&di->int_lock, flags);
}

static void ina231_interrupt_work(struct work_struct *work)
{
	struct ina231_device_info *di = NULL;
	struct nty_data *data = NULL;
	int ret;
	int cur = 0;
	u16 mask_enable;
	u16 alert_limit;

	if (!work)
		return;

	di = container_of(work, struct ina231_device_info, irq_work);
	if (!di || !di->client)
		return;

	data = &(di->nty_data);

	mask_enable = i2c_smbus_read_word_swapped(di->client,
		INA231_MASK_ENABLE_REG);
	alert_limit = i2c_smbus_read_word_swapped(di->client,
		INA231_ALERT_LIMIT_REG);

	ina231_get_current_ma(&cur, di);

	data->event1 = mask_enable;
	data->event2 = alert_limit;
	data->addr = di->client->addr;

	if ((di->int_notify_enable_flag == INA231_ENABLE_INT_NOTIFY) &&
		((di->notifier_type == POWER_ANT_LVC_FAULT) ||
		(di->notifier_type == POWER_ANT_SC_FAULT))) {
		if (mask_enable & INA231_MASK_ENABLE_SOL_MASK) {
			hwlog_info("shunt over-volt\n");
			power_event_anc_notify(di->notifier_type,
				POWER_NE_DC_FAULT_INA231, data);
		} else if (mask_enable & INA231_MASK_ENABLE_SUL_MASK) {
			if (cur < INA231_INT_UNDER_CURRENT_MA) {
				hwlog_info("shunt under-volt,cur=%d,cnt=%d\n",
					cur, di->count);
				ret = i2c_smbus_write_word_swapped(di->client,
					INA231_MASK_ENABLE_REG, 0);
				if (ret)
					hwlog_err("clear irq failed\n");

				power_event_anc_notify(di->notifier_type,
					POWER_NE_DC_FAULT_INA231, data);
			} else {
				hwlog_info("ignore this int,cur=%d,cnt=%d\n",
					cur, di->count);
				ina231_int_en(true);
			}
			di->count++;
		} else if (mask_enable & INA231_MASK_ENABLE_BOL_MASK) {
			hwlog_info("bus over-volt\n");
			power_event_anc_notify(di->notifier_type,
				POWER_NE_DC_FAULT_INA231, data);
		} else if (mask_enable & INA231_MASK_ENABLE_BUL_MASK) {
			hwlog_info("bus under-volt\n");
			power_event_anc_notify(di->notifier_type,
				POWER_NE_DC_FAULT_INA231, data);
		} else if (mask_enable & INA231_MASK_ENABLE_POL_MASK) {
			hwlog_info("power over-limit\n");
			power_event_anc_notify(di->notifier_type,
				POWER_NE_DC_FAULT_INA231, data);
		}
	}

	ina231_dump_register();

	hwlog_info("mask_enable_reg [%x]=0x%x\n",
		INA231_MASK_ENABLE_REG, mask_enable);
	hwlog_info("alert_limit_reg [%x]=0x%x\n",
		INA231_ALERT_LIMIT_REG, alert_limit);
}

static irqreturn_t ina231_interrupt(int irq, void *_di)
{
	struct ina231_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_HANDLED;
	}

	if (di->init_finish_flag == INA231_INIT_FINISH)
		di->int_notify_enable_flag = INA231_ENABLE_INT_NOTIFY;

	hwlog_info("int happened\n");

	ina231_int_en(false);
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static int ina231_irq_init(struct ina231_device_info *di,
	struct device_node *np)
{
	int ret;

	ret = power_gpio_config_interrupt(np,
		"gpio_int", "ina231_gpio_int", &di->gpio_int, &di->irq_int);
	if (ret)
		return ret;

	ret = request_irq(di->irq_int, ina231_interrupt, IRQF_TRIGGER_FALLING,
		"ina231_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		gpio_free(di->gpio_int);
		return ret;
	}

	return 0;
}

static int ina231_recv_dc_status_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct ina231_device_info *di = NULL;
	u16 mask_enable_content;

	if (!nb)
		return NOTIFY_OK;

	di = container_of(nb, struct ina231_device_info, nb);
	if (!di || !di->client || !di->config)
		return NOTIFY_OK;

	mask_enable_content = di->config->mask_enable_content;

	switch (event) {
	case POWER_NE_DC_LVC_CHARGING:
		hwlog_info("lvc start charging,mask_value=0x%x\n",
			mask_enable_content);
		di->count = 0;
		di->notifier_type = POWER_ANT_LVC_FAULT;
		i2c_smbus_write_word_swapped(di->client,
			INA231_MASK_ENABLE_REG, mask_enable_content);
		ina231_int_en(true);
		break;
	case POWER_NE_DC_SC_CHARGING:
		hwlog_info("sc start charging,mask_value=0x%x\n",
			mask_enable_content);
		di->count = 0;
		di->notifier_type = POWER_ANT_SC_FAULT;
		i2c_smbus_write_word_swapped(di->client,
			INA231_MASK_ENABLE_REG, mask_enable_content);
		ina231_int_en(true);
		break;
	case POWER_NE_DC_STOP_CHARGE:
		hwlog_info("direct_charge stop charge\n");
		ina231_int_en(false);
		di->notifier_type = POWER_ANT_END;
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static void ina231_parse_dts(struct ina231_device_info *di,
	struct device_node *np)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rshunt", &di->rshunt, INA231_DEFAULT_RSHUNT_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ic_role", &di->ic_role, CHARGE_IC_TYPE_MAIN);

	/* callibration = 0.00512 / (current_lsb * Rshunt) */
	ina231_config.calibrate_content = INA231_CALL_DIVISOR /
		(ina231_config.current_lsb * di->rshunt);
	hwlog_info("calibrate_content=0x%x\n",
		ina231_config.calibrate_content);

	ina231_config.alert_limit_content = di->rshunt *
		INA231_INT_UNDER_CURRENT_MA * INA231_VOL_UV_TO_NV /
		ina231_config.shunt_voltage_lsb;
	hwlog_info("alert_limit_content=0x%x\n",
		ina231_config.alert_limit_content);
}

static int ina231_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct ina231_device_info *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_ina231_dev = di;
	di->config = &ina231_config;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);
	INIT_WORK(&di->irq_work, ina231_interrupt_work);

	ina231_parse_dts(di, np);

	ret = ina231_irq_init(di, np);
	if (ret)
		goto ina231_fail_0;

	ret = dc_batinfo_ops_register(LVC_MODE, di->ic_role, &ina231_batinfo_ops);
	if (ret)
		goto ina231_fail_1;

	ret = dc_batinfo_ops_register(SC_MODE, di->ic_role, &ina231_batinfo_ops);
	if (ret)
		goto ina231_fail_1;

	di->chip_already_init = 1;

	ret = ina231_device_reset();
	if (ret)
		goto ina231_fail_1;

	ret = ina231_reg_init(di);
	if (ret)
		goto ina231_fail_1;

	spin_lock_init(&di->int_lock);
	di->is_int_en = true;

	ina231_int_en(false);
	di->notifier_type = POWER_ANT_END;
	di->nb.notifier_call = ina231_recv_dc_status_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_DC, &di->nb);
	if (ret)
		goto ina231_fail_1;

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	ina231_dbg_register(client);
	ina231_dump_register();
	return 0;

ina231_fail_1:
	di->chip_already_init = 0;
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
ina231_fail_0:
	devm_kfree(&client->dev, di);
	g_ina231_dev = NULL;

	return ret;
}

static int ina231_remove(struct i2c_client *client)
{
	struct ina231_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	power_event_bnc_unregister(POWER_BNT_DC, &di->nb);
	devm_kfree(&client->dev, di);
	g_ina231_dev = NULL;

	return 0;
}

static void ina231_shutdown(struct i2c_client *client)
{
	ina231_device_reset();
}

MODULE_DEVICE_TABLE(i2c, ina231);
static const struct of_device_id ina231_of_match[] = {
	{
		.compatible = "huawei,ina231_for_charge",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id ina231_i2c_id[] = {
	{ "ina231_for_charge", 0 },
	{}
};

static struct i2c_driver ina231_driver = {
	.probe = ina231_probe,
	.remove = ina231_remove,
	.shutdown = ina231_shutdown,
	.id_table = ina231_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "huawei_ina231_for_charge",
		.of_match_table = of_match_ptr(ina231_of_match),
	},
};

static int __init ina231_init(void)
{
	return i2c_add_driver(&ina231_driver);
}

static void __exit ina231_exit(void)
{
	i2c_del_driver(&ina231_driver);
}

module_init(ina231_init);
module_exit(ina231_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ina231 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
