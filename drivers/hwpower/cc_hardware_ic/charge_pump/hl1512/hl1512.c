/*
 * hl1512.c
 *
 * charge-pump hl1512 driver
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

#include "hl1512.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG cp_hl1512
HWLOG_REGIST();

static struct hl1512_dev_info *g_hl1512_di;

static int hl1512_read_byte(u8 reg, u8 *data)
{
	struct hl1512_dev_info *di = g_hl1512_di;
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

static int hl1512_write_byte(u8 reg, u8 data)
{
	struct hl1512_dev_info *di = g_hl1512_di;
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

static int hl1512_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = hl1512_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return hl1512_write_byte(reg, val);
}

static void hl1512_set_init_data(struct hl1512_dev_info *di,
	struct hl1512_init_data *data, int type)
{
	if (type == HL1512_CHARGING_FWD) {
		if (di->chip_id == HL1512Y_DEVICE_ID) {
			data->mode_val = HL1512_FWD_MOD_INIT_VAL;
			data->control_val = HL1512Y_CONTROL_REG_VAL;
			data->pmid_vout_ov_uv_val = HL1512_PMID_VOUT_FWD_VAL;
			data->vout_ov_val = HL1512Y_VOUT_OV_REG_FWD_VAL;
			data->current_limit_val = HL1512Y_CUR_LIMIT_FWD_VAL;
		} else {
			data->mode_val = HL1512_FWD_MOD_INIT_VAL;
			data->control_val = HL1512_CONTROL_REG_VAL;
			data->pmid_vout_ov_uv_val = HL1512_PMID_VOUT_FWD_VAL;
			data->vout_ov_val = HL1512_VOUT_OV_REG_FWD_VAL;
			data->current_limit_val = HL1512_CUR_LIMIT_FWD_VAL;
		}
	} else if (type == HL1512_CHARGING_REV) {
		if (di->chip_id == HL1512Y_DEVICE_ID) {
			data->mode_val = HL1512_REV_MOD_INIT_VAL;
			data->control_val = HL1512Y_CONTROL_REG_VAL;
			data->pmid_vout_ov_uv_val = HL1512_PMID_VOUT_FWD_VAL;
			data->vout_ov_val = HL1512Y_VOUT_OV_REG_FWD_VAL;
			data->current_limit_val = HL1512Y_CUR_LIMIT_FWD_VAL;
		} else {
			data->mode_val = HL1512_REV_MOD_INIT_VAL;
			data->control_val = HL1512_CONTROL_REG_VAL;
			data->pmid_vout_ov_uv_val = HL1512_PMID_VOUT_REV_VAL;
			data->vout_ov_val = HL1512_VOUT_OV_REG_REV_VAL;
			data->current_limit_val = HL1512_CUR_LIMIT_REV_VAL;
		}
		if (di->current_limit_val == HL1512_CUR_LIMIT_DISABLE_VAL) {
			data->current_limit_val &= ~HL1512_CUR_LIMIT_MASK;
			data->current_limit_val |= HL1512_CUR_LIMIT_QRB_DIS_VAL;
		}
	} else if (type == HL1512_CHARGING_REV_CP) {
		data->mode_val = HL1512_REV_PMID_INIT_VAL;
		data->control_val = HL1512_CONTROL_REV_CP_VAL;
		data->pmid_vout_ov_uv_val = HL1512_PMID_VOUT_REV_VAL;
		data->vout_ov_val = HL1512_VOUT_OV_REG_REV_VAL;
		data->current_limit_val = HL1512_CUR_LIMIT_REV_CP_VAL;
		if (di->current_limit_val == HL1512_CUR_LIMIT_DISABLE_VAL) {
			data->current_limit_val &= ~HL1512_CUR_LIMIT_MASK;
			data->current_limit_val |= HL1512_CUR_LIMIT_QRB_DIS_VAL;
		}
	}
}

static int hl1512_chip_init_ex(int type, void *dev_data)
{
	int ret;
	struct hl1512_init_data data = { 0 };
	struct hl1512_dev_info *di = dev_data;

	if (!di)
		return -EINVAL;

	hl1512_set_init_data(di, &data, type);

	/* ovp pre-protection */
	ret = hl1512_write_byte(HL1512_CLK_SYNC_REG, HL1512_CLK_SYNC_OVP_VAL);
	/* init a7 reg */
	ret += hl1512_write_byte(HL1512_INIT_REGA7, HL1512_INIT_REGA7_VAL);
	/* current ilimit */
	ret += hl1512_write_byte(HL1512_CUR_LIMIT_REG, data.current_limit_val);
	/* control reg */
	ret += hl1512_write_byte(HL1512_CONTROL_REG, data.control_val);
	/* pmid vout ov uv */
	ret += hl1512_write_byte(HL1512_PMID_VOUT_OV_UV_REG, data.pmid_vout_ov_uv_val);
	/* vout ov */
	ret += hl1512_write_byte(HL1512_VOUT_OV_REG, data.vout_ov_val);
	/* wd and clock */
	ret += hl1512_write_byte(HL1512_WD_CLOCK_REG, HL1512_WD_CLOCK_VAL);
	/* init 58 reg */
	ret += hl1512_write_byte(HL1512_INIT_REG58, HL1512_INIT_REG58_VAL);

	if (type == HL1512_CHARGING_FWD) /* need init 53 reg */
		ret += hl1512_write_byte(HL1512_INIT_REG53, HL1512_INIT_REG53_VAL);

	/* temp alarm th */
	ret += hl1512_write_byte(HL1512_TEMP_ALARM_TH_REG,
		HL1512_TEMP_ALARM_TH_REG_VAL);
	power_usleep(DT_USLEEP_10MS);
	if (type == HL1512_CHARGING_REV_CP) {
		power_usleep(DT_USLEEP_5MS); /* for stable */
		ret += hl1512_write_byte(HL1512_WD_CLOCK_REG, HL1512_WD_CLOCK_FREQ_VAL);
		ret += hl1512_write_byte(HL1512_MOD_REG, HL1512_REV_QRB_DIS_INIT_VAL);
		return ret;
	}
	/* mode config reg */
	ret += hl1512_write_byte(HL1512_MOD_REG, data.mode_val);

	return ret;
}

static int hl1512_chip_init(void *dev_data)
{
	return hl1512_chip_init_ex(HL1512_CHARGING_FWD, dev_data);
}

static int hl1512_reverse_chip_init(void *dev_data)
{
	int ret;

	power_usleep(DT_USLEEP_10MS);
	ret = hl1512_chip_init_ex(HL1512_CHARGING_REV, dev_data);
	if (ret) {
		hwlog_err("init reverse chip failed\n");
		return ret;
	}

	return 0;
}

static bool hl1512_is_cp_open(void *dev_data)
{
	int i, ret;
	u8 val = 0;

	/* check 20ms*3=60ms at most to ensure cp open */
	for (i = 0; i < 3; i++) {
		ret = hl1512_read_byte(HL1512_STATUS_REG, &val);
		if (!ret && ((val & HL1512_STATUS_REG_CP_VAL) == HL1512_STATUS_REG_CP_VAL))
			return true;
		(void)power_msleep(DT_MSLEEP_20MS, 0, NULL);
	}

	return false;
}

static bool hl1512_is_bp_open(void *dev_data)
{
	int ret;
	u8 val = 0;

	ret = hl1512_read_byte(HL1512_STATUS_REG, &val);
	if (!ret && ((val & HL1512_STATUS_REG_BP_VAL) == HL1512_STATUS_REG_BP_VAL))
		return true;

	return false;
}

static int hl1512_set_bp_mode(void *dev_data)
{
	int ret;
	struct hl1512_dev_info *di = dev_data;

	if (!di)
		return -EINVAL;

	ret = hl1512_write_mask(HL1512_MOD_REG,
		HL1512_FORCE_BPCP_MASK, HL1512_FORCE_BPCP_SHIFT,
		HL1512_FORCE_BP_EN);
	power_usleep(DT_USLEEP_10MS);
	if (di->chip_id != HL1512Y_DEVICE_ID)
		ret += hl1512_write_byte(HL1512_PMID_VOUT_OV_UV_REG,
			HL1512_PMID_VOUT_FWD_VAL);
	if (ret) {
		hwlog_err("set op to bp failed\n");
		return ret;
	}

	return 0;
}

static int hl1512_set_cp_mode(void *dev_data)
{
	int ret = 0;
	struct hl1512_dev_info *di = dev_data;

	if (!di)
		return -EINVAL;

	if (di->chip_id != HL1512Y_DEVICE_ID)
		ret = hl1512_write_byte(HL1512_PMID_VOUT_OV_UV_REG,
			HL1512_PMID_VOUT_CP_VAL);
	ret += hl1512_write_mask(HL1512_MOD_REG,
		HL1512_FORCE_BPCP_MASK, HL1512_FORCE_BPCP_SHIFT,
		HL1512_FORCE_CP_EN);
	if (ret) {
		hwlog_err("set op to cp failed\n");
		return ret;
	}

	return 0;
}

static int hl1512_reverse_cp_check(void *dev_data)
{
	int i;
	int ret;
	u8 mode = 0;
	int cnt = HL1512_REV_CP_RETRY_TIME / HL1512_REV_CP_SLEEP_TIME;
	struct hl1512_dev_info *di = dev_data;

	if (!di)
		return -EINVAL;

	(void)power_msleep(DT_MSLEEP_50MS, 0, NULL); /* reduce the charging current */

	ret = hl1512_write_byte(HL1512_MOD_REG, HL1512_REV_QRB_EN_INIT_VAL);
	if (ret)
		return ret;

	if (di->chip_id == HL1512Y_DEVICE_ID)
		return 0;

	for (i = 0; i < cnt; i++) {
		ret = hl1512_read_byte(HL1512_STATUS_REG, &mode);
		if (ret || (mode != HL1512_STATUS_REG_REV_CP_VAL)) {
			hwlog_err("reverse_cp_check: mode=0x%x err\n", mode);
			msleep(HL1512_REV_CP_SLEEP_TIME);
			continue;
		}
		ret += hl1512_write_byte(HL1512_CONTROL_REG,
			HL1512_CONTROL_REG_VAL);
		if (ret) {
			hwlog_err("reverse_cp_check: write failed\n");
			continue;
		}
		ret += hl1512_write_byte(HL1512_WD_CLOCK_REG, HL1512_WD_CLOCK_VAL);
		ret += hl1512_write_byte(HL1512_MOD_REG, HL1512_REV_WPC_INIT_VAL);
		if (ret) {
			hwlog_err("reverse_cp_check: set freq failed\n");
			continue;
		}
		hwlog_info("reverse cp check succ, cnt=%d\n", i);
		return 0;
	}

	hwlog_err("reverse cp check failed, cnt=%d\n", i);
	return -1;
}

static int hl1512_reverse_cp_chip_init(void *dev_data)
{
	int ret;

	ret = hl1512_chip_init_ex(HL1512_CHARGING_REV_CP, dev_data);
	ret += hl1512_reverse_cp_check(dev_data);
	if (ret) {
		hwlog_err("init reverse chip failed\n");
		return ret;
	}

	return 0;
}

static int hl1512_set_reverse_bp2cp_mode(void *dev_data)
{
	int ret;
	u8 ilim_val = HL1512_CUR_LIMIT_REV_CP_VAL;
	struct hl1512_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	ret = hl1512_write_byte(HL1512_CONTROL_REG, HL1512_CONTROL_REV_CP_VAL);
	ret += hl1512_write_byte(HL1512_CUR_LIMIT_REG,
		HL1512_CUR_LIMIT_FWD_VAL);
	ret += hl1512_write_byte(HL1512_MOD_REG, HL1512_REV_PMID_INIT_VAL);
	if (ret) {
		hwlog_err("set rev cp pmid failed\n");
		return ret;
	}

	power_usleep(DT_USLEEP_10MS);
	ret = hl1512_write_byte(HL1512_MOD_REG, HL1512_REV_WPC_INIT_VAL);
	if (ret) {
		hwlog_err("set rev cp wpc failed\n");
		return ret;
	}

	power_usleep(DT_USLEEP_10MS);
	ret = hl1512_write_byte(HL1512_PMID_VOUT_OV_UV_REG,
		HL1512_PMID_VOUT_FWD_VAL);
	ret += hl1512_write_byte(HL1512_CONTROL_REG, HL1512_CONTROL_REG_VAL);
	if (di->current_limit_val == HL1512_CUR_LIMIT_DISABLE_VAL) {
		ilim_val &= ~HL1512_CUR_LIMIT_MASK;
		ilim_val |= HL1512_CUR_LIMIT_QRB_DIS_VAL;
	}
	ret += hl1512_write_byte(HL1512_CUR_LIMIT_REG, ilim_val);
	if (ret) {
		hwlog_err("set rev cp failed\n");
		return ret;
	}

	hwlog_info("set rev 1:2 cp mode succ\n");
	return 0;
}

static void hl1512_irq_work(struct work_struct *work)
{
	hwlog_info("[irq_work] ++\n");
}

static irqreturn_t hl1512_irq_handler(int irq, void *p)
{
	struct hl1512_dev_info *di = g_hl1512_di;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_NONE;
	}

	schedule_work(&di->irq_work);
	return IRQ_HANDLED;
}

static int hl1512_irq_init(struct hl1512_dev_info *di, struct device_node *np)
{
	int ret;

	if (power_gpio_config_interrupt(np,
		"gpio_int", "hl1512_int", &di->gpio_int, &di->irq_int))
		return 0;

	ret = request_irq(di->irq_int, hl1512_irq_handler,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "hl1512_irq", di);
	if (ret) {
		hwlog_err("could not request hl1512_irq\n");
		di->irq_int = -EINVAL;
		gpio_free(di->gpio_int);
		return ret;
	}

	enable_irq_wake(di->irq_int);
	INIT_WORK(&di->irq_work, hl1512_irq_work);
	return 0;
}

static int hl1512_device_check(void *dev_data)
{
	int ret;
	struct hl1512_dev_info *di = g_hl1512_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = hl1512_read_byte(HL1512_DEVICE_ID_REG, &di->chip_id);
	if (ret) {
		hwlog_err("get chip_id failed\n");
		return ret;
	}

	hwlog_info("chip_id=0x%x\n", di->chip_id);
	return 0;
}

static int hl1512_post_probe(void *dev_data)
{
	struct hl1512_dev_info *di = g_hl1512_di;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (hl1512_irq_init(di, di->client->dev.of_node)) {
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

static struct charge_pump_ops hl1512_ops = {
	.cp_type = CP_TYPE_MAIN,
	.chip_name = "hl1512",
	.dev_check = hl1512_device_check,
	.post_probe = hl1512_post_probe,
	.chip_init = hl1512_chip_init,
	.rvs_chip_init = hl1512_reverse_chip_init,
	.rvs_cp_chip_init = hl1512_reverse_cp_chip_init,
	.set_bp_mode = hl1512_set_bp_mode,
	.set_cp_mode = hl1512_set_cp_mode,
	.set_rvs_bp_mode = hl1512_set_bp_mode,
	.set_rvs_cp_mode = hl1512_set_cp_mode,
	.is_cp_open = hl1512_is_cp_open,
	.is_bp_open = hl1512_is_bp_open,
	.set_rvs_bp2cp_mode = hl1512_set_reverse_bp2cp_mode,
};

static void hl1512_parse_dts(struct device_node *np, struct hl1512_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"current_limit_val", (u32 *)&di->current_limit_val,
		HL1512_CUR_LIMIT_DEFAULT);
}

static int hl1512_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct hl1512_dev_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_hl1512_di = di;
	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	hl1512_parse_dts(np, di);
	hl1512_ops.dev_data = (void *)di;
	ret = charge_pump_ops_register(&hl1512_ops);
	if (ret)
		goto ops_register_fail;

	return 0;

ops_register_fail:
	i2c_set_clientdata(client, NULL);
	devm_kfree(&client->dev, di);
	g_hl1512_di = NULL;
	return ret;
}

static int hl1512_remove(struct i2c_client *client)
{
	struct hl1512_dev_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	i2c_set_clientdata(client, NULL);
	g_hl1512_di = NULL;
	return 0;
}

MODULE_DEVICE_TABLE(i2c, charge_pump_hl1512);
static const struct of_device_id hl1512_of_match[] = {
	{
		.compatible = "charge_pump_hl1512",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id hl1512_i2c_id[] = {
	{ "charge_pump_hl1512", 0 }, {}
};

static struct i2c_driver hl1512_driver = {
	.probe = hl1512_probe,
	.remove = hl1512_remove,
	.id_table = hl1512_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "charge_pump_hl1512",
		.of_match_table = of_match_ptr(hl1512_of_match),
	},
};

static int __init hl1512_init(void)
{
	return i2c_add_driver(&hl1512_driver);
}

static void __exit hl1512_exit(void)
{
	i2c_del_driver(&hl1512_driver);
}

rootfs_initcall(hl1512_init);
module_exit(hl1512_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hl1512 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
