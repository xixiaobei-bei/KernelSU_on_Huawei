/*
 * bq40z50.c
 *
 * bq40z50 driver
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

#include "bq40z50.h"
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_log.h>

#define HWLOG_TAG bq40z50
HWLOG_REGIST();

static struct bq40z50_device_info *g_bq40z50_dev;

static struct bq40z50_device_info *bq40z50_get_di(void)
{
	if (!g_bq40z50_dev) {
		hwlog_err("chip not init\n");
		return NULL;
	}

	return g_bq40z50_dev;
}

static int bq40z50_read_word(u8 reg, u16 *data)
{
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return -ENODEV;

	mutex_lock(&di->rw_lock);
	ret = power_i2c_u8_read_word(di->client, reg, data, false);
	mutex_unlock(&di->rw_lock);
	if (ret < 0)
		hwlog_err("reg=0x%x read fail\n", reg);

	return ret;
}

static void bq40z50_charger_event_process(struct bq40z50_device_info *di,
	unsigned int event)
{
	if (!di)
		return;

	hwlog_info("receive charge event=%u\n", event);
	switch (event) {
	case VCHRG_START_USB_CHARGING_EVENT:
	case VCHRG_START_AC_CHARGING_EVENT:
	case VCHRG_START_CHARGING_EVENT:
		di->charge_status = BQ40Z50_CHARGE_STATE_START_CHARGING;
		break;
	case VCHRG_STOP_CHARGING_EVENT:
		di->charge_status = BQ40Z50_CHARGE_STATE_STOP_CHARGING;
		break;
	case VCHRG_CHARGE_DONE_EVENT:
		di->charge_status = BQ40Z50_CHARGE_STATE_CHRG_DONE;
		break;
	case VCHRG_NOT_CHARGING_EVENT:
		di->charge_status = BQ40Z50_CHARGE_STATE_NOT_CHARGING;
		break;
	case VCHRG_POWER_SUPPLY_OVERVOLTAGE:
		di->charge_status = BQ40Z50_CHARGE_STATE_NOT_CHARGING;
		break;
	case VCHRG_POWER_SUPPLY_WEAKSOURCE:
		di->charge_status = BQ40Z50_CHARGE_STATE_NOT_CHARGING;
		break;
	default:
		di->charge_status = BQ40Z50_CHARGE_STATE_NOT_CHARGING;
		break;
	}
}

static int bq40z50_is_ready(void)
{
	return (g_bq40z50_dev == NULL) ? false : true;
}

static int bq40z50_get_battery_id_vol(void)
{
	return 0;
}

static int bq40z50_get_battery_temp(void)
{
	int ret;
	int temp_c;
	u16 temp_k = 0;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return BQ40Z50_BATT_TEMP_ABNORMAL_LOW - 1;

	ret = bq40z50_read_word(BQ40Z50_REG_TEMP, &temp_k);
	if (ret) {
		temp_c = di->cache.temp;
	} else {
		temp_c = (int)temp_k / POWER_BASE_DEC - BQ40Z50_BATT_TEMP_ZERO;
		di->cache.temp = temp_c;
	}

	hwlog_info("battery_temp=%d\n", temp_c);
	return temp_c;
}

#ifdef CONFIG_HLTHERM_RUNTEST
static int bq40z50_is_battery_exist(void)
{
	return false;
}
#else
static int bq40z50_is_battery_exist(void)
{
	int temp;

	temp = bq40z50_get_battery_temp();
	if ((temp <= BQ40Z50_BATT_TEMP_ABNORMAL_LOW) ||
		(temp >= BQ40Z50_BATT_TEMP_ABNORMAL_HIGH))
		return false;

	return true;
}
#endif /* CONFIG_HLTHERM_RUNTEST */

static int bq40z50_get_battery_soc(void)
{
	int ret;
	u16 soc = 0;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return (int)soc;

	if (di->charge_status == BQ40Z50_CHARGE_STATE_CHRG_DONE) {
		hwlog_info("charge done, force soc to full\n");
		return BQ40Z50_BATT_CAPACITY_FULL;
	}

	ret = bq40z50_read_word(BQ40Z50_REG_SOC, &soc);
	if (ret)
		soc = di->cache.soc;
	else
		di->cache.soc = soc;

	hwlog_info("soc=%u\n", soc);
	return (int)soc;
}

static int bq40z50_is_battery_reach_threshold(void)
{
	int soc;

	if (!bq40z50_is_battery_exist())
		return 0;

	soc = bq40z50_get_battery_soc();
	if (soc > BQ40Z50_BATT_CAPACITY_WARNING_LVL)
		return 0;
	else if (soc > BQ40Z50_BATT_CAPACITY_LOW_LVL)
		return BQ_FLAG_SOC1;

	return BQ_FLAG_LOCK;
}

static char *bq40z50_get_battery_brand(void)
{
	return "bq40z50";
}

static int bq40z50_get_battery_vol(void)
{
	u16 vol = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return (int)vol;

	ret = bq40z50_read_word(BQ40Z50_REG_VOLT, &vol);
	if (ret)
		vol = di->cache.vol;
	else
		di->cache.vol = vol;

	hwlog_info("battery_vol=%u\n", vol);
	return (int)vol;
}

static int bq40z50_get_battery_vol_uv(void)
{
	return THOUSAND_UNIT * bq40z50_get_battery_vol();
}

static int bq40z50_get_battery_curr(void)
{
	short curr = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return (int)curr;

	ret = bq40z50_read_word(BQ40Z50_REG_CURR, &curr);
	if (ret)
		curr = di->cache.curr;
	else
		di->cache.curr = curr;

	hwlog_info("battery_cur=%d\n", curr);
	return -(int)curr;
}

static int bq40z50_get_battery_avgcurr(void)
{
	short avg_curr = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return (int)avg_curr;

	ret = bq40z50_read_word(BQ40Z50_REG_AVRGCURR, &avg_curr);
	if (ret)
		avg_curr = di->cache.avg_curr;
	else
		di->cache.avg_curr = avg_curr;

	hwlog_info("battery_avgcur=%d\n", avg_curr);
	return (int)avg_curr;
}

static int bq40z50_get_battery_rm(void)
{
	u16 rm = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return (int)rm;

	ret = bq40z50_read_word(BQ40Z50_REG_RM, &rm);
	if (ret)
		rm = di->cache.rm;
	else
		di->cache.rm = rm;

	hwlog_info("battery_rm=%u\n", rm);
	return (int)rm;
}

static int bq40z50_get_battery_dc(void)
{
	u16 dc = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return (int)dc;

	ret = bq40z50_read_word(BQ40Z50_REG_DC, &dc);
	if (ret)
		dc = di->cache.dc;
	else
		di->cache.dc = dc;

	hwlog_info("battery_dc=%u\n", dc);
	return (int)dc;
}

static int bq40z50_get_battery_fcc(void)
{
	u16 fcc = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return (int)fcc;

	ret = bq40z50_read_word(BQ40Z50_REG_FCC, &fcc);
	if (ret)
		fcc = di->cache.fcc;
	else
		di->cache.fcc = fcc;

	hwlog_info("battery_fcc=%u\n", fcc);
	return (int)fcc;
}

static int bq40z50_get_battery_soh(void)
{
	u16 soh = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return (int)soh;

	ret = bq40z50_read_word(BQ40Z50_REG_SOH, &soh);
	if (ret)
		soh = di->cache.soh;
	else
		di->cache.soh = soh;

	hwlog_info("battery_soh=%u\n", soh);
	return (int)soh;
}

static int bq40z50_get_battery_tte(void)
{
	u16 tte = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return (int)tte;

	ret = bq40z50_read_word(BQ40Z50_REG_TTE, &tte);
	if (ret)
		tte = di->cache.tte;
	else
		di->cache.tte = tte;

	if (tte == BQ40Z50_REG_MAX_VAL) {
		hwlog_err("the battery is not being discharged\n");
		return -1;
	}

	hwlog_info("battery_tte=%u\n", tte);
	return (int)tte;
}

static int bq40z50_get_battery_ttf(void)
{
	u16 ttf = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return (int)ttf;

	ret = bq40z50_read_word(BQ40Z50_REG_TTF, &ttf);
	if (ret)
		ttf = di->cache.ttf;
	else
		di->cache.ttf = ttf;

	if (ttf == BQ40Z50_REG_MAX_VAL) {
		hwlog_err("the battery is not being charged\n");
		return -1;
	}

	hwlog_info("battery_ttf=%u\n", ttf);
	return (int)ttf;
}

static int bq40z50_get_battery_cycle(void)
{
	u16 cycle = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return (int)cycle;

	ret = bq40z50_read_word(BQ40Z50_REG_CYCLE, &cycle);
	if (ret)
		cycle = di->cache.cycle;
	else
		di->cache.cycle = cycle;

	hwlog_info("battery_cycle=%u\n", cycle);
	return (int)cycle;
}

static int bq40z50_battery_unfiltered_soc(void)
{
	return bq40z50_get_battery_soc();
}

static int bq40z50_get_battery_health(void)
{
	int temp;
	int status = POWER_SUPPLY_HEALTH_GOOD;

	if (!bq40z50_is_battery_exist())
		return 0;

	temp = bq40z50_get_battery_temp();
	if (temp < BQ40Z50_BATT_TEMP_ABNORMAL_LOW)
		status = POWER_SUPPLY_HEALTH_COLD;
	else if (temp > BQ40Z50_BATT_TEMP_ABNORMAL_HIGH)
		status = POWER_SUPPLY_HEALTH_OVERHEAT;

	hwlog_info("battery_health=%d\n", status);
	return status;
}

static int bq40z50_get_battery_capacity_level(void)
{
	int capacity;
	int level;

	if (!bq40z50_is_battery_exist())
		return 0;

	capacity = bq40z50_get_battery_soc();
	if ((capacity < BQ40Z50_BATT_CAPACITY_ZERO) ||
		(capacity > BQ40Z50_BATT_CAPACITY_FULL))
		level = POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
	else if (capacity <= BQ40Z50_BATT_CAPACITY_CRITICAL)
		level = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
	else if (capacity <= BQ40Z50_BATT_CAPACITY_LOW)
		level = POWER_SUPPLY_CAPACITY_LEVEL_LOW;
	else if (capacity < BQ40Z50_BATT_CAPACITY_HIGH)
		level = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
	else if (capacity < BQ40Z50_BATT_CAPACITY_FULL)
		level = POWER_SUPPLY_CAPACITY_LEVEL_HIGH;
	else
		level = POWER_SUPPLY_CAPACITY_LEVEL_FULL;

	hwlog_info("battery_cap_level=%d\n", level);
	return level;
}

static int bq40z50_get_battery_technology(void)
{
	/* default technology is "Li-poly" */
	return POWER_SUPPLY_TECHNOLOGY_LIPO;
}

static int bq40z50_get_battery_vbat_max(void)
{
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return -ENODEV;

	hwlog_info("battery_vbat_max=%d\n", di->vbat_max);
	return di->vbat_max;
}

static int bq40z50_is_fcc_debounce(void)
{
	return 0;
}

static int bq40z50_device_check(void)
{
	return 0;
}

static int bq40z50_battery_charger_event_rcv(unsigned int evt)
{
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di || !bq40z50_is_battery_exist())
		return 0;

	bq40z50_charger_event_process(di, evt);

	return 0;
}

struct chrg_para_lut *bq40z50_get_battery_charge_params(void)
{
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return NULL;

	return di->para_batt_data;
}

static int bq40z50_get_battery_status(void)
{
	u16 status = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return -ENODEV;

	ret = bq40z50_read_word(BQ40Z50_REG_BATTERY_STATUS, &status);
	if (ret)
		status = di->cache.status;
	else
		di->cache.status = status;

	hwlog_info("battery status=%u\n", status);
	return (int)status;
}

static int bq40z50_get_desired_charging_current(void)
{
	u16 curr = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return -ENODEV;

	ret = bq40z50_read_word(BQ40Z50_REG_CHARGING_CURRENT, &curr);
	if (ret)
		curr = di->cache.charge_current;
	else
		di->cache.charge_current = curr;

	hwlog_info("battery charging current=%u\n", curr);
	return (int)curr;
}

static int bq40z50_get_desired_charging_voltage(void)
{
	u16 vol = 0;
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return -ENODEV;

	ret = bq40z50_read_word(BQ40Z50_REG_CHARGING_VOLTAGE, &vol);
	if (ret)
		vol = di->cache.charge_voltage;
	else
		di->cache.charge_voltage = vol;

	hwlog_info("battery charging voltage=%u\n", vol);
	return (int)vol;
}

static int bq40z50_is_smart_battery(void)
{
	struct bq40z50_device_info *di = bq40z50_get_di();

	if (!di)
		return -ENODEV;

	if (di->is_smart_battery)
		return true;

	return false;
}

static int bq40z50_is_battery_full_charged(void)
{
	int status;

	status = bq40z50_get_battery_status();
	if ((status >= 0) && (status & BQ40Z50_BATT_STATUS_FC))
		return true;

	return false;
}

static int bq40z50_dump_log_data(char *buffer, int size, void *dev_data)
{
	struct bq40z50_device_info *di = dev_data;
	struct bq40z50_log_data log_data;

	if (!buffer || !di)
		return -1;

	log_data.vbat = bq40z50_get_battery_vol();
	log_data.ibat = bq40z50_get_battery_curr();
	log_data.avg_ibat = bq40z50_get_battery_avgcurr();
	log_data.rm = bq40z50_get_battery_rm();
	log_data.temp = bq40z50_get_battery_temp();
	log_data.soc = bq40z50_get_battery_soc();
	log_data.fcc = bq40z50_get_battery_fcc();

	snprintf(buffer, size, "%-7d%-7d%-7d%-7d%-7d%-7d%-7d   ",
		log_data.temp, log_data.vbat, log_data.ibat,
		log_data.avg_ibat, log_data.rm, log_data.soc,
		log_data.fcc);

	return 0;
}

static int bq40z50_get_log_head(char *buffer, int size, void *dev_data)
{
	struct bq40z50_device_info *di = dev_data;

	if (!buffer || !di)
		return -1;

	snprintf(buffer, size,
		"    Temp   Vbat   Ibat   AIbat   Rm   Soc   Fcc");

	return 0;
}

static struct power_log_ops bq40z50_log_ops = {
	.dev_name = "bq40z50",
	.dump_log_head = bq40z50_get_log_head,
	.dump_log_content = bq40z50_dump_log_data,
};

static struct coulometer_ops bq40z50_ops = {
	.battery_id_voltage = bq40z50_get_battery_id_vol,
	.is_coul_ready = bq40z50_is_ready,
	.is_battery_exist = bq40z50_is_battery_exist,
	.is_battery_reach_threshold = bq40z50_is_battery_reach_threshold,
	.battery_brand = bq40z50_get_battery_brand,
	.battery_voltage = bq40z50_get_battery_vol,
	.battery_voltage_uv = bq40z50_get_battery_vol_uv,
	.battery_current = bq40z50_get_battery_curr,
	.fifo_avg_current = bq40z50_get_battery_curr,
	.battery_current_avg = bq40z50_get_battery_avgcurr,
	.battery_unfiltered_capacity = bq40z50_battery_unfiltered_soc,
	.battery_capacity = bq40z50_get_battery_soc,
	.battery_temperature = bq40z50_get_battery_temp,
	.battery_rm = bq40z50_get_battery_rm,
	.battery_fcc = bq40z50_get_battery_fcc,
	.battery_tte = bq40z50_get_battery_tte,
	.battery_ttf = bq40z50_get_battery_ttf,
	.battery_health = bq40z50_get_battery_health,
	.battery_capacity_level = bq40z50_get_battery_capacity_level,
	.battery_technology = bq40z50_get_battery_technology,
	.battery_charge_params = bq40z50_get_battery_charge_params,
	.battery_vbat_max = bq40z50_get_battery_vbat_max,
	.charger_event_rcv = bq40z50_battery_charger_event_rcv,
	.coul_is_fcc_debounce = bq40z50_is_fcc_debounce,
	.battery_cycle_count = bq40z50_get_battery_cycle,
	.battery_fcc_design = bq40z50_get_battery_dc,
	.dev_check = bq40z50_device_check,
	.battery_temperature_for_charger = bq40z50_get_battery_temp,
	.get_desired_charging_current = bq40z50_get_desired_charging_current,
	.get_desired_charging_voltage = bq40z50_get_desired_charging_voltage,
	.is_smart_battery = bq40z50_is_smart_battery,
	.is_battery_full_charged = bq40z50_is_battery_full_charged,
};

static int bq40z50_read_dts_string_array(const char *prop,
	long *data, u32 row, u32 col)
{
	int i, len;
	const char *tmp_string = NULL;

	len = power_dts_read_count_strings_compatible(power_dts_tag(HWLOG_TAG),
		"bq40z50_battery", prop, row, col);
	if (len < 0)
		return -EINVAL;

	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index_compatible(power_dts_tag(HWLOG_TAG),
			"bq40z50_battery", prop, i, &tmp_string))
			return -EINVAL;

		if (kstrtol(tmp_string, 0, &data[i]))
			return -EINVAL;
	}

	return len;
}

static int bq40z50_get_temp_para_data(struct chrg_para_lut *pdata)
{
	int len;

	if (!pdata)
		return -EINVAL;

	len = bq40z50_read_dts_string_array("temp_para", &pdata->temp_data,
		TEMP_PARA_LEVEL, TEMP_PARA_TOTAL);
	if (len < 0)
		return -EINVAL;

	pdata->temp_len = len;
	return 0;
}

static int bq40z50_get_vbat_para_data(struct chrg_para_lut *pdata)
{
	int len;

	if (!pdata)
		return -EINVAL;

	len = bq40z50_read_dts_string_array("vbat_para", &pdata->volt_data,
		VOLT_PARA_LEVEL, VOLT_PARA_TOTAL);
	if (len < 0)
		return -EINVAL;

	pdata->volt_len = len;
	return 0;
}

static int bq40z50_get_segment_para_data(struct chrg_para_lut *pdata)
{
	int len;

	if (!pdata)
		return -EINVAL;

	len = bq40z50_read_dts_string_array("segment_para", &pdata->segment_data,
		SEGMENT_PARA_LEVEL, SEGMENT_PARA_TOTAL);
	if (len < 0)
		return -EINVAL;

	pdata->segment_len = len;
	return 0;
}

static int bq40z50_get_batt_para(void)
{
	int ret;
	struct bq40z50_device_info *di = bq40z50_get_di();
	struct chrg_para_lut *para_data = NULL;

	if (!di)
		return -ENODEV;

	para_data = di->para_batt_data;
	/* vbat_max */
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"bq40z50_battery", "vbat_max", &di->vbat_max,
		BQ40Z50_BATT_MAX_VOLTAGE_DEFAULT);

	/* smart charge support */
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"bq40z50_battery", "is_smart_battery", &di->is_smart_battery,
		true);

	ret = bq40z50_get_temp_para_data(para_data);
	if (ret)
		return -EINVAL;
	ret = bq40z50_get_vbat_para_data(para_data);
	if (ret)
		return -EINVAL;
	ret = bq40z50_get_segment_para_data(para_data);
	if (ret)
		return -EINVAL;

	return 0;
}

static int bq40z50_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = -ENOMEM;
	struct bq40z50_device_info *di = NULL;
	struct device_node *np = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return ret;

	di->cache.vol = BQ40Z50_BATT_VOLTAGE_DEFAULT;
	di->cache.temp = BQ40Z50_BATT_TEMP_ABNORMAL_LOW - 1;
	di->cache.soc = BQ40Z50_BATT_CAPACITY_DEFAULT;
	di->charge_status = BQ40Z50_CHARGE_STATE_NOT_CHARGING;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);
	mutex_init(&di->rw_lock);
	g_bq40z50_dev = di;

	di->para_batt_data = kzalloc(sizeof(struct chrg_para_lut), GFP_KERNEL);
	if (!di->para_batt_data)
		goto alloc_mem_fail;

	ret = bq40z50_get_batt_para();
	if (ret)
		goto get_para_fail;

	ret = coul_drv_coul_ops_register(&bq40z50_ops, COUL_BQ40Z50);
	if (ret)
		goto register_coul_ops_fail;

	bq40z50_log_ops.dev_data = (void *)di;
	power_log_ops_register(&bq40z50_log_ops);

	return 0;

register_coul_ops_fail:
get_para_fail:
	kfree(di->para_batt_data);
	di->para_batt_data = NULL;
alloc_mem_fail:
	mutex_destroy(&di->rw_lock);
	kfree(di);
	g_bq40z50_dev = NULL;
	return ret;
}

static int bq40z50_remove(struct i2c_client *client)
{
	struct bq40z50_device_info *di = i2c_get_clientdata(client);

	if (!di || !di->para_batt_data)
		return -ENODEV;

	mutex_destroy(&di->rw_lock);
	kfree(di->para_batt_data);
	di->para_batt_data = NULL;
	kfree(di);
	g_bq40z50_dev = NULL;

	return 0;
}

static int bq40z50_suspend(struct i2c_client *client)
{
	return 0;
}

static int bq40z50_resume(struct i2c_client *client)
{
	return 0;
}

MODULE_DEVICE_TABLE(i2c, bq40z50);
static const struct of_device_id bq40z50_of_match[] = {
	{
		.compatible = "huawei,bq40z50",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bq40z50_i2c_id[] = {
	{ "bq40z50", 0 },
	{},
};

static struct i2c_driver bq40z50_driver = {
	.probe = bq40z50_probe,
	.remove = bq40z50_remove,
	.id_table = bq40z50_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bq40z50_fg",
		.of_match_table = of_match_ptr(bq40z50_of_match),
	},
};

static int __init bq40z50_init(void)
{
	return i2c_add_driver(&bq40z50_driver);
}

static void __exit bq40z50_exit(void)
{
	i2c_del_driver(&bq40z50_driver);
}

rootfs_initcall(bq40z50_init);
module_exit(bq40z50_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bq40z50 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
