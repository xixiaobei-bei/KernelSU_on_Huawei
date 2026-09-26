/*
 * sc8502.c
 *
 * charge-pump sc8502 driver
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

#include "sc8502.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG cp_sc8502
HWLOG_REGIST();

static struct sc8502_dev_info *g_sc8502_di;

static int sc8502_read_byte(u8 reg, u8 *data)
{
	struct sc8502_dev_info *di = g_sc8502_di;
	int i;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	for (i = 0; i < CP_I2C_RETRY_CNT; i++) {
		if (!power_i2c_u8_read_byte(di->client, reg, data))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -EIO;
}

static int sc8502_write_byte(u8 reg, u8 data)
{
	struct sc8502_dev_info *di = g_sc8502_di;
	int i;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	for (i = 0; i < CP_I2C_RETRY_CNT; i++) {
		if (!power_i2c_u8_write_byte(di->client, reg, data))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -EIO;
}

static int sc8502_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = sc8502_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return sc8502_write_byte(reg, val);
}

static int sc8502_chip_init_ex(int type)
{
	int ret;
	u8 ctrl1_val = SC8502_CTRL1_REG_REV_INIT;
	u8 ctrl2_val = SC8502_CTRL2_REG_FWD_BP_INIT;
	u8 pmid_vout_ov_uv_val = SC8502_PMID_VOUT_REV_VAL;
	u8 vout_ov_val = SC8502_VOUT_OV_REG_REV_VAL;
	u8 pmid_ov_val = SC8502_PMID_OV_REG_REV_VAL;
	u8 ibus_ov = SC8502_IBUS_OCP_REV_VAL;

	if (type == SC8502_CHARGING_FWD) {
		ctrl1_val = SC8502_CTRL1_REG_FWD_INIT;
		pmid_vout_ov_uv_val = SC8502_PMID_VOUT_FWD_VAL;
		vout_ov_val = SC8502_VOUT_OV_REG_FWD_VAL;
		pmid_ov_val = SC8502_PMID_OV_REG_FWD_VAL;
		ibus_ov = SC8502_IBUS_OCP_FWD_VAL;
	} else if (type == SC8502_CHARGING_REV) {
		ctrl2_val = SC8502_CTRL2_REG_REV_BP_INIT;
	} else if (type == SC8502_CHARGING_REV_CP) {
		ctrl2_val = SC8502_CTRL2_REG_REV_CP_INIT;
		pmid_vout_ov_uv_val = SC8502_PMID_VOUT_FWD_VAL;
	}

	ret = sc8502_write_mask(SC8502_CTRL1_REG, SC8502_CHARGE_EN_MASK,
		SC8502_CHARGE_EN_SHIFT, SC8502_CP_DIS);
	/* mode config reg */
	ret += sc8502_write_byte(SC8502_CTRL2_REG, ctrl2_val);
	/* pmid vout ov uv */
	ret += sc8502_write_byte(SC8502_PMID2OUT_OVP_UVP_REG,
		pmid_vout_ov_uv_val);
	/* pmid vout oc rc */
	ret += sc8502_write_byte(SC8502_PMID2OUT_OCP_RCP_REG,
		SC8502_PMID2OUT_OCP_RCP_INIT_VAL);
	/* vout ov */
	ret += sc8502_write_byte(SC8502_VOUT_OVP_REG, vout_ov_val);
	/* wd */
	ret += sc8502_write_byte(SC8502_CTRL3_REG, SC8502_CTRL3_REG_INIT);
	/* ibus_ov and tdie alarm */
	ret += sc8502_write_byte(SC8502_IBUS_OCP_REG, ibus_ov);
	ret += sc8502_write_byte(SC8502_CTRL1_REG, ctrl1_val);

	return ret;
}

static int sc8502_chip_init(void *dev_data)
{
	return sc8502_chip_init_ex(SC8502_CHARGING_FWD);
}

static int sc8502_reverse_chip_init(void *dev_data)
{
	return sc8502_chip_init_ex(SC8502_CHARGING_REV);
}

static bool sc8502_is_cp_open(void *dev_data)
{
	int ret;
	u8 cp = 0;
	u8 mode = 0;

	ret = sc8502_read_byte(SC8502_CTRL1_REG, &cp);
	ret += sc8502_read_byte(SC8502_CTRL2_REG, &mode);
	if (!ret && (cp & SC8502_CHARGE_EN_MASK) &&
		((mode & SC8502_CHANGE_MODE_MASK) == SC8502_FWD_CP_MODE_VAL)) {
		hwlog_info("is cp mode\n");
		return true;
	}

	hwlog_err("is not cp mode\n");
	return false;
}

static bool sc8502_is_bp_open(void *dev_data)
{
	int ret;
	u8 cp = 0;
	u8 mode = 0;

	ret = sc8502_read_byte(SC8502_CTRL1_REG, &cp);
	ret += sc8502_read_byte(SC8502_CTRL2_REG, &mode);
	if (!ret && (cp & SC8502_CHARGE_EN_MASK) &&
		!(mode & SC8502_CHANGE_MODE_MASK)) {
		hwlog_info("is bp mode\n");
		return true;
	}

	hwlog_err("is not bp mode\n");
	return false;
}

static int sc8502_set_bp_mode(void *dev_data)
{
	int ret;

	if (sc8502_is_bp_open(dev_data))
		return 0;

	ret = sc8502_write_mask(SC8502_CTRL2_REG,
		SC8502_CHANGE_MODE_MASK, SC8502_CHANGE_MODE_SHIFT,
		SC8502_FWD_BP_MODE_VAL);
	ret += sc8502_write_byte(SC8502_PMID2OUT_OVP_UVP_REG,
		SC8502_PMID_VOUT_FWD_VAL);
	ret += sc8502_write_mask(SC8502_CTRL1_REG,
		SC8502_CHARGE_EN_MASK, SC8502_CHARGE_EN_SHIFT, SC8502_CP_EN);
	if (ret) {
		hwlog_err("set to bp failed\n");
		return ret;
	}

	hwlog_info("set bp mode succ\n");
	return 0;
}

static int sc8502_set_cp_mode(void *dev_data)
{
	int ret;

	if (sc8502_is_cp_open(dev_data))
		return 0;

	ret = sc8502_write_mask(SC8502_CTRL2_REG,
		SC8502_CHANGE_MODE_MASK, SC8502_CHANGE_MODE_SHIFT,
		SC8502_FWD_CP_MODE_VAL);
	ret += sc8502_write_byte(SC8502_PMID2OUT_OVP_UVP_REG,
		SC8502_PMID_VOUT_FWD_CP_VAL);
	ret += sc8502_write_mask(SC8502_CTRL1_REG,
		SC8502_CHARGE_EN_MASK, SC8502_CHARGE_EN_SHIFT, SC8502_CP_EN);
	if (ret) {
		hwlog_err("set to cp failed\n");
		return ret;
	}

	hwlog_info("set cp mode succ\n");
	return 0;
}

static int sc8502_set_reverse_bp_mode(void *dev_data)
{
	int ret;

	ret = sc8502_write_mask(SC8502_CTRL1_REG,
		SC8502_CHARGE_EN_MASK, SC8502_CHARGE_EN_SHIFT, SC8502_CP_DIS);
	ret += sc8502_write_mask(SC8502_CTRL2_REG,
		SC8502_CHANGE_MODE_MASK, SC8502_CHANGE_MODE_SHIFT,
		SC8502_REV_BP_MODE_VAL);
	ret += sc8502_write_byte(SC8502_PMID2OUT_OVP_UVP_REG,
		SC8502_PMID_VOUT_REV_VAL);
	ret += sc8502_write_byte(SC8502_CTRL1_REG, SC8502_CTRL1_REG_REV_INIT);
	if (ret) {
		hwlog_err("set to reverse bp failed\n");
		return ret;
	}

	hwlog_info("set reverse bp mode succ\n");
	return 0;
}

static int sc8502_set_reverse_cp_mode(void *dev_data)
{
	int ret;

	ret = sc8502_write_mask(SC8502_CTRL1_REG,
		SC8502_CHARGE_EN_MASK, SC8502_CHARGE_EN_SHIFT, SC8502_CP_DIS);
	ret += sc8502_write_mask(SC8502_CTRL2_REG,
		SC8502_CHANGE_MODE_MASK, SC8502_CHANGE_MODE_SHIFT,
		SC8502_REV_CP_MODE_VAL);
	ret += sc8502_write_byte(SC8502_PMID2OUT_OVP_UVP_REG,
		SC8502_PMID_VOUT_FWD_VAL);
	ret += sc8502_write_byte(SC8502_CTRL1_REG, SC8502_CTRL1_REG_REV_INIT);
	if (ret) {
		hwlog_err("set to reverse cp failed\n");
		return ret;
	}

	hwlog_info("set reverse cp mode succ\n");
	return 0;
}

static int sc8502_reverse_cp_check(void)
{
	int i;
	int ret;
	u8 cp = 0;
	u8 mode = 0;
	int cnt = SC8502_REV_CP_RETRY_TIME / SC8502_REV_CP_SLEEP_TIME;

	for (i = 0; i < cnt; i++) {
		ret = sc8502_read_byte(SC8502_CTRL1_REG, &cp);
		ret += sc8502_read_byte(SC8502_CTRL2_REG, &mode);
		if (ret || !(cp & SC8502_CHARGE_EN_MASK) ||
		!((mode & SC8502_CHANGE_MODE_MASK) == SC8502_REV_CP_MODE_VAL)) {
			hwlog_err("reverse_cp_check: mode=0x%x err\n", mode);
			msleep(SC8502_REV_CP_SLEEP_TIME);
			continue;
		}

		hwlog_info("reverse cp check succ, cnt=%d\n", i);
		return 0;
	}

	hwlog_err("reverse cp check failed, cnt=%d\n", i);
	return -1;
}

static int sc8502_reverse_cp_chip_init(void *dev_data)
{
	int ret;

	ret = sc8502_chip_init_ex(SC8502_CHARGING_REV_CP);
	ret += sc8502_reverse_cp_check();
	if (ret) {
		hwlog_err("init reverse cp chip failed\n");
		return ret;
	}

	hwlog_info("reversc cp mode init succ\n");
	power_msleep(DT_MSLEEP_50MS, 0, NULL);
	ret = sc8502_write_mask(SC8502_CTRL1_REG,
		SC8502_QB_EN_MARK, SC8502_QB_EN_SHITF,
		SC8502_QB_EN);
	if (ret) {
		hwlog_err("open qb failed\n");
		return ret;
	}

	hwlog_info("open qb succ\n");
	return 0;
}

static int sc8502_set_reverse_bp2cp_mode(void *dev_data)
{
	int ret;

	ret = sc8502_write_mask(SC8502_CTRL1_REG,
		SC8502_CHARGE_EN_MASK, SC8502_CHARGE_EN_SHIFT, SC8502_CP_DIS);
	ret += sc8502_write_byte(SC8502_IBUS_OCP_REG,
		SC8502_IBUS_OCP_FWD_VAL);
	ret += sc8502_write_mask(SC8502_CTRL2_REG,
		SC8502_CHANGE_MODE_MASK, SC8502_CHANGE_MODE_SHIFT,
		SC8502_REV_CP_MODE_VAL);
	if (ret) {
		hwlog_err("set rev cp mode failed\n");
		return ret;
	}

	ret = sc8502_write_byte(SC8502_PMID2OUT_OVP_UVP_REG,
		SC8502_PMID_VOUT_FWD_VAL);
	ret += sc8502_write_byte(SC8502_IBUS_OCP_REG,
		SC8502_IBUS_OCP_REV_VAL);
	ret += sc8502_write_byte(SC8502_CTRL1_REG, SC8502_CTRL1_REG_REV_INIT);
	if (ret) {
		hwlog_err("set rev cp failed\n");
		return ret;
	}

	hwlog_info("set rev 1:2 cp mode succ\n");
	return 0;
}

static void sc8502_irq_work(struct work_struct *work)
{
	hwlog_info("[irq_work] ++\n");
}

static irqreturn_t sc8502_irq_handler(int irq, void *p)
{
	struct sc8502_dev_info *di = g_sc8502_di;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_NONE;
	}

	schedule_work(&di->irq_work);
	return IRQ_HANDLED;
}

static int sc8502_irq_init(struct sc8502_dev_info *di, struct device_node *np)
{
	int ret;

	if (power_gpio_config_interrupt(np,
		"gpio_int", "sc8502_int", &di->gpio_int, &di->irq_int))
		return 0;

	ret = request_irq(di->irq_int, sc8502_irq_handler,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "sc8502_irq", di);
	if (ret) {
		hwlog_err("could not request sc8502_irq\n");
		di->irq_int = -EINVAL;
		gpio_free(di->gpio_int);
		return ret;
	}

	enable_irq_wake(di->irq_int);
	INIT_WORK(&di->irq_work, sc8502_irq_work);
	return 0;
}

static int sc8502_device_check(void *dev_data)
{
	int ret;
	struct sc8502_dev_info *di = g_sc8502_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = sc8502_read_byte(SC8502_DEVICE_ID_REG, &di->chip_id);
	if (ret)
		di->chip_id = CP_DEVICE_ID_SC8502;

	hwlog_info("chip_id=0x%x\n", di->chip_id);
	return 0;
}

static int sc8502_post_probe(void *dev_data)
{
	struct sc8502_dev_info *di = g_sc8502_di;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (sc8502_irq_init(di, di->client->dev.of_node)) {
		hwlog_err("irq init failed\n");
		return -1;
	}

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = di->chip_id;
		power_dev_info->ver_id = 0;
	}

	return 0;
}

static struct charge_pump_ops sc8502_ops = {
	.cp_type = CP_TYPE_MAIN,
	.chip_name = "sc8502",
	.dev_check = sc8502_device_check,
	.post_probe = sc8502_post_probe,
	.chip_init = sc8502_chip_init,
	.rvs_chip_init = sc8502_reverse_chip_init,
	.rvs_cp_chip_init = sc8502_reverse_cp_chip_init,
	.set_bp_mode = sc8502_set_bp_mode,
	.set_cp_mode = sc8502_set_cp_mode,
	.set_rvs_bp_mode = sc8502_set_reverse_bp_mode,
	.set_rvs_cp_mode = sc8502_set_reverse_cp_mode,
	.is_cp_open = sc8502_is_cp_open,
	.is_bp_open = sc8502_is_bp_open,
	.set_rvs_bp2cp_mode = sc8502_set_reverse_bp2cp_mode,
};

static int sc8502_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct sc8502_dev_info *di = NULL;

	if (!client || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_sc8502_di = di;
	di->dev = &client->dev;
	di->client = client;
	i2c_set_clientdata(client, di);

	sc8502_ops.dev_data = (void *)di;
	ret = charge_pump_ops_register(&sc8502_ops);
	if (ret)
		goto ops_register_fail;

	return 0;

ops_register_fail:
	i2c_set_clientdata(client, NULL);
	devm_kfree(&client->dev, di);
	g_sc8502_di = NULL;
	return ret;
}

static int sc8502_remove(struct i2c_client *client)
{
	struct sc8502_dev_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	i2c_set_clientdata(client, NULL);
	g_sc8502_di = NULL;
	return 0;
}

MODULE_DEVICE_TABLE(i2c, charge_pump_sc8502);
static const struct of_device_id sc8502_of_match[] = {
	{
		.compatible = "charge_pump_sc8502",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id sc8502_i2c_id[] = {
	{ "charge_pump_sc8502", 0 }, {}
};

static struct i2c_driver sc8502_driver = {
	.probe = sc8502_probe,
	.remove = sc8502_remove,
	.id_table = sc8502_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "charge_pump_sc8502",
		.of_match_table = of_match_ptr(sc8502_of_match),
	},
};

static int __init sc8502_init(void)
{
	return i2c_add_driver(&sc8502_driver);
}

static void __exit sc8502_exit(void)
{
	i2c_del_driver(&sc8502_driver);
}

rootfs_initcall(sc8502_init);
module_exit(sc8502_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("sc8502 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
