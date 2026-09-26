/*
 * huawei_watch_charger.c
 *
 * huawei watch charger for power module
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
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/reboot.h>
#include <linux/power_supply.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/sched.h>
#include <linux/of.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/log/log_jank.h>
#include <huawei_platform/power/huawei_watch_charger.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_ui_ne.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include "ext_sensorhub_api.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG watch_charger
HWLOG_REGIST();
#define SEND_CMD_INTERVAL_MS 600

struct huawei_charge_info {
	struct device *dev;
	struct mcu_chg_info charge_info;
	struct watch_charge_sysfs_data sysfs_data;
	struct work_struct monitor_mcu_charge_work;
	struct work_struct wait_mcu_response_work;
	struct delayed_work poweroff_work;
	bool mcu_report_flag;
	bool low_power_off_flag;
};

struct mcu_report_work_struct {
	struct work_struct report_work;
	unsigned char mcu_report_info[MCU_REPORT_BUFFER_LEN];
};
struct wait_mcu_response_work_struct {
	struct work_struct response_work;
	unsigned char write_mcu_response_info[MCU_RESPONSE_BUFFER_LEN];
};
/*
 * timer to actively query mcu charge info every five second untill get
 * mcu charge info
 */
static struct timer_list g_get_charge_info_timer;
static struct huawei_charge_info *g_charge_info;
static struct sid_cid g_chg_read_mcu_sid_cid[CHG_SID_CID_COUNT] = {
	{ CHG_SERVICE_ID, CHG_READ_COMMAND_ID },
	{ CHG_SERVICE_ID, CHG_WRITE_COMMAND_ID },
};
static struct subscribe_cmds g_read_mcu_cmds = {
	g_chg_read_mcu_sid_cid, CHG_SID_CID_COUNT
};
/* record last jiffies get data */
static unsigned long g_last_jiffies_current;
static unsigned long g_last_jiffies_voltage;
static unsigned long g_last_jiffies_charge_status;

static int watch_send_cmd_to_mcu_charge(
	enum watch_charge_cmd_to_mcu main_cmd, unsigned int sub_cmd_data)
{
	struct send_cmd_st send_cmd = {
		.cmd = main_cmd,
		.sub_cmd = sub_cmd_data,
	};
	struct command cmd = {
		.service_id = CHG_SERVICE_ID,
		.command_id = CHG_WRITE_COMMAND_ID,
		.send_buffer = (unsigned char *)&send_cmd,
		.send_buffer_len = sizeof(send_cmd),
	};

#ifdef CONFIG_HUAWEI_POWER_DEBUG
	hwlog_debug("%s main_cmd:%d, sub_cmd_data:%d\n", __func__,
		main_cmd, sub_cmd_data);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

	return send_command(CHARGE_CHANNEL, &cmd, STATUS_FALSE, NULL);
}

/*
 * Function:watch_set_charge_status
 * Description:set charging status form mcu
 * Parameters:status:charging or discharging
 * return:NA
 */
static void watch_set_charge_status(int status)
{
	g_charge_info->charge_info.charge_st = status;
}

/*
 * Function:watch_get_charge_status
 * Description:get charging status form mcu
 * Parameters:NA
 * return:
 *        POWER_SUPPLY_STATUS_CHARGING :charging status
 *        POWER_SUPPLY_STATUS_DISCHARGING discharging status
 */
static int watch_get_charge_status(void)
{
	int result;
	int ret;

	/*
	 * get mcu charging status through double machine communication
	 * if get fail use the mcu previous report value
	 */
	if (jiffies_to_msecs(jiffies - g_last_jiffies_charge_status) >
		SEND_CMD_INTERVAL_MS) {
		ret = watch_send_cmd_to_mcu_charge(GET_CHG_ST, DEFAULT_CMD);
		if (ret < 0)
			hwlog_err("%s query charging status fail use last value ret:%d\n",
				__func__, ret);
		else
			g_last_jiffies_charge_status = jiffies;
	} else {
		hwlog_info("%s last interval less than %dms\n", __func__,
			SEND_CMD_INTERVAL_MS);
	}
	result = g_charge_info->charge_info.charge_st;
	hwlog_info("%s query charging status is %d\n", __func__, result);
	if (result == CHARGEING_ST)
		return POWER_SUPPLY_STATUS_CHARGING;

	return POWER_SUPPLY_STATUS_DISCHARGING;
}

/*
 * Function:watch_set_battery_voltage
 * Description:set battery voltage form mcu
 * Parameters:voltage:battery voltage
 * return:NA
 */
static void watch_set_battery_voltage(int voltage)
{
	g_charge_info->charge_info.bat_vol = voltage;
}

/*
 * Function:watch_get_battery_voltage
 * Description:get battery voltage form mcu
 * Parameters:NA
 * return:battery voltage
 */
static int watch_get_battery_voltage(void)
{
	int ret;

	/*
	 * get mcu battery voltage through double machine communication
	 * if get fail use the mcu previous report value
	 */
	if (jiffies_to_msecs(jiffies - g_last_jiffies_voltage) >
		SEND_CMD_INTERVAL_MS) {
		ret = watch_send_cmd_to_mcu_charge(GET_BAT_VOLT, DEFAULT_CMD);
		if (ret < 0)
			hwlog_err("%s query voltage fail use last value ret:%d\n",
				__func__, ret);
		else
			g_last_jiffies_voltage = jiffies;
	} else {
		hwlog_info("%s last interval less than %dms\n", __func__,
			SEND_CMD_INTERVAL_MS);
	}
	hwlog_info("%s query battery voltage is %d uV\n", __func__,
		g_charge_info->charge_info.bat_vol * THOUSAND_UNIT);
	return g_charge_info->charge_info.bat_vol * THOUSAND_UNIT;
}

/*
 * Function:watch_set_charge_current
 * Description:set battery charge current form mcu
 * Parameters:charge_current:charge current
 * return:NA
 */
static void watch_set_charge_current(int charge_current)
{
	g_charge_info->charge_info.bat_cur = charge_current;
}

/*
 * Function:watch_get_charge_current
 * Description:get battery current form mcu
 * Parameters:NA
 * return:battery current
 */
static int watch_get_charge_current(void)
{
	int ret;

	/*
	 * get mcu charge current through double machine communication
	 * if get fail use the mcu previous report value
	 */
	if (jiffies_to_msecs(jiffies - g_last_jiffies_current) >
		SEND_CMD_INTERVAL_MS) {
		ret = watch_send_cmd_to_mcu_charge(GET_CHG_CUR, DEFAULT_CMD);
		if (ret < 0)
			hwlog_err("%s query current fail use last value ret:%d\n",
				__func__, ret);
		else
			g_last_jiffies_current = jiffies;
	} else {
		hwlog_info("%s last interval less than %dms\n",
			__func__, SEND_CMD_INTERVAL_MS);
	}
	hwlog_info("%s query charge current is %d mA\n", __func__,
		g_charge_info->charge_info.bat_cur);
	return g_charge_info->charge_info.bat_cur;
}

/*
 * Function:watch_set_battery_capacity
 * Description:set battery capacity form mcu
 * Parameters:capacity:battery capacity
 * return:NA
 */
static void watch_set_battery_capacity(int capacity)
{
	g_charge_info->charge_info.bat_cap = capacity;
}

/*
 * Function:watch_get_battery_capacity
 * Description:get battery capacity form mcu,as capacity is reported by mcu
 * after 1% change,so no need query capacity through double machine communication
 * Parameters:NA
 * return:battery capacity
 */
static int watch_get_battery_capacity(void)
{
	hwlog_info("%s query battery capacity is %d\n", __func__,
		g_charge_info->charge_info.bat_cap);
	/*
	 * poweroff system through kernel if app can not poweroff
	 * when capacity less than or equal to POWEROFF_CAPACTY after 90s
	 */
	if (g_charge_info->charge_info.bat_cap <= POWEROFF_CAPACTY) {
		if (g_charge_info->low_power_off_flag == STATUS_FALSE) {
			schedule_delayed_work(&g_charge_info->poweroff_work,
				round_jiffies_relative(msecs_to_jiffies(POWEROFF_DELAY_MS)));
			g_charge_info->low_power_off_flag = STATUS_TRUE;
		}
	} else {
		if (g_charge_info->low_power_off_flag == STATUS_TRUE) {
			cancel_delayed_work_sync(&g_charge_info->poweroff_work);
			g_charge_info->low_power_off_flag == STATUS_FALSE;
		}
	}
	return g_charge_info->charge_info.bat_cap;
}

/*
 * Function:watch_set_battery_temperature
 * Description:set battery temperature form mcu
 * Parameters:temperature:battery temperature
 * return:NA
 */
static void watch_set_battery_temperature(int temperature)
{
	g_charge_info->charge_info.bat_temp = temperature;
}

/*
 * Function:watch_get_battery_temperature
 * Description:get battery temperature form mcu,as temperature is reported
 * by mcu after 1 degree change,so no need query temperature through double
 * machine communication
 * Parameters:NA
 * return:battery temperature
 */
static int watch_get_battery_temperature(void)
{
	hwlog_info("%s query battery temperature is %d\n", __func__,
		g_charge_info->charge_info.bat_temp);
	return g_charge_info->charge_info.bat_temp * POWER_BASE_DEC;
}

/*
 * Function:watch_get_batid_voltage
 * Description:get battery ID voltage form mcu
 * Parameters:NA
 * return:battery ID voltage
 */
static int watch_get_batid_voltage(void)
{
	hwlog_info("%s query battery id voltage is %d mV\n",
		__func__, g_charge_info->charge_info.bat_id);
	return g_charge_info->charge_info.bat_id;
}

/*
 * Function:watch_get_battery_capacity_rm
 * Description:get battery capacity rm form mcu
 * Parameters:NA
 * return:battery remaining capacity unit mAh
 */
static int watch_get_battery_capacity_rm(void)
{
	hwlog_info("%s query bat_cap_rm is %d mAh\n",
		__func__, g_charge_info->charge_info.bat_cap_rm);
	return g_charge_info->charge_info.bat_cap_rm;
}

/*
 * Function:watch_get_battery_capacity_fcc
 * Description:get battery capacity fcc form mcu
 * Parameters:NA
 * return:battery capacity fcc unit mAh
 */
static int watch_get_battery_capacity_fcc(void)
{
	hwlog_info("%s query battery_capacity_fcc is %d mAh\n",
		__func__, g_charge_info->charge_info.bat_cap_fcc);
	return g_charge_info->charge_info.bat_cap_fcc;
}

/*
 * Function:watch_get_charge_cycle_count
 * Description:get charge cycle count form mcu
 * Parameters:NA
 * return:charge cycle count
 */
static int watch_get_charge_cycle_count(void)
{
	hwlog_info("%s query charge_cycle_cnt is %d\n",
		__func__, g_charge_info->charge_info.charge_cycle_cnt);
	return g_charge_info->charge_info.charge_cycle_cnt;
}

/*
 * Function:watch_get_battery_charge_full
 * Description:get battery charge full capacity from mcu
 * Parameters:NA
 * return:battery charge full capacity unit mAh
 */
static int watch_get_battery_charge_full(void)
{
	hwlog_info("%s query bat_charge_full is %d mAh\n",
		__func__, g_charge_info->charge_info.bat_charge_full);
	return g_charge_info->charge_info.bat_charge_full;
}

/*
 * Function:watch_get_battery_charge_full_design
 * Description:get battery charge full designed capacity from mcu
 * Parameters:NA
 * return:battery charge full designed capacity unit mAh
 */
static int watch_get_battery_charge_full_design(void)
{
	hwlog_info("%s query bat_charge_full_design is %d mAh\n",
		__func__, g_charge_info->charge_info.bat_charge_full_design);
	return g_charge_info->charge_info.bat_charge_full_design;
}

/*
 * Function:watch_get_battery_voltage_max
 * Description:get battery stand cut-off voltage form mcu
 * Parameters:NA
 * return:battery stand cut-off voltage
 */
static int watch_get_battery_voltage_max(void)
{
	hwlog_info("%s query battery stand cut-off voltage is %d mV\n",
		__func__, g_charge_info->charge_info.bat_voltage_max);
	return g_charge_info->charge_info.bat_voltage_max;
}

/*
 * Function:watch_get_battery_capacity_level
 * Description:get battery capacity fcc form mcu
 * Parameters:NA
 * return:battery capacity level
 */
static int watch_get_battery_capacity_level(void)
{
	hwlog_info("%s query bat_capacity_level is %d\n",
		__func__, g_charge_info->charge_info.bat_capacity_level);
	return g_charge_info->charge_info.bat_capacity_level;
}

/*
 * Function:watch_get_battery_charge_now_capacity
 * Description:get battery current capacity
 * Parameters:NA
 * return:battery current capacity
 */
static int watch_get_battery_charge_now_capacity(void)
{
	hwlog_info("%s query bat_charge_now is %d mAh\n",
		__func__, g_charge_info->charge_info.bat_charge_now);
	return g_charge_info->charge_info.bat_charge_now;
}

/*
 * Function:watch_get_charger_online
 * Description:get charger online status form mcu
 * Parameters:NA
 * return:charger online status
 */
static int watch_get_charger_online(void)
{
	hwlog_info("%s query charger online status is %d\n",
		__func__, g_charge_info->charge_info.online);
	return g_charge_info->charge_info.online;
}

/*
 * Function:watch_set_enable_charge
 * Description:set charge enable status to mcu
 * Parameters:value 0:enable 1:disable
 * return:NA
 */
static int watch_set_enable_charge(unsigned int value)
{
	int ret;

	hwlog_info("%s user set enable status:%d\n", __func__, value);
	g_charge_info->sysfs_data.charge_enable = value;
	ret = watch_send_cmd_to_mcu_charge(SET_CHG_ST, value);
	if (ret < 0)
		hwlog_err("%s set mcu charge enable fail ret:%d\n",
			__func__, ret);
	return 0;
}

/*
 * Function:watch_get_mcu_charge_enable_result
 * Description:get mcu real charge enable status
 * Parameters:enable 0:disable 1:enable
 * return:NA
 */
static void watch_get_mcu_charge_enable_result(int enable)
{
	hwlog_info("%s mcu real enable charge result:%d\n", __func__, enable);

	if (enable == MCU_SET_SUCCESSFUL)
		return;

	if (g_charge_info->sysfs_data.charge_enable == STATUS_TRUE)
		g_charge_info->sysfs_data.charge_enable = STATUS_FALSE;
	else
		g_charge_info->sysfs_data.charge_enable = STATUS_TRUE;
}

/*
 * Function:watch_get_enable_charge
 * Description:mcu charge enable status to sysfs node show
 * Parameters:NA
 * return:0
 */
static int watch_get_enable_charge(unsigned int *value)
{
	if (!value)
		return -ENODEV;

	*value = g_charge_info->sysfs_data.charge_enable;
	return 0;
}

/*
 * Function:watch_set_iin_limit
 * Description:set input current to mcu
 * Parameters:current from userspace
 * return:NA
 */
static int watch_set_iin_limit(unsigned int value)
{
	int ret;

	hwlog_info("%s user set iuput current:%d mA\n", __func__, value);
	g_charge_info->sysfs_data.iin_limit = value;
	ret = watch_send_cmd_to_mcu_charge(SET_IIN_LIMIT, value);
	if (ret < 0)
		hwlog_err("%s set mcu input current fail ret:%d\n",
			__func__, ret);
	return 0;
}

/*
 * Function:watch_get_mcu_iin_limit_result
 * Description:get mcu real input current  limit result
 * Parameters:value:mcu real input current
 * return:NA
 */
static void watch_get_mcu_iin_limit_result(int value)
{
	g_charge_info->sysfs_data.iin_limit = value;
	hwlog_info("%s mcu real input current:%d mA\n", __func__, value);
}

/*
 * Function:watch_get_iin_limit
 * Description:mcu real input current to sysfs node show
 * Parameters:value:mcu real input current
 * return:NA
 */
static int watch_get_iin_limit(unsigned int *value)
{
	if (!value)
		return -ENODEV;

	*value = g_charge_info->sysfs_data.iin_limit;
	return 0;
}

/*
 * Function:watch_set_ichg_limit
 * Description:set charge current to mcu
 * Parameters:value:charge current from userspace
 * return:NA
 */
static int watch_set_ichg_limit(unsigned int value)
{
	int ret;

	hwlog_info("%s user set charge current:%d mA\n", __func__, value);
	g_charge_info->sysfs_data.ichg_limit = value;
	ret = watch_send_cmd_to_mcu_charge(SET_ICHG_LIMIT, value);
	if (ret < 0)
		hwlog_err("%s set mcu charge current fail ret:%d\n",
			__func__, ret);
	return 0;
}

/*
 * Function:watch_get_mcu_ichg_limit_result
 * Description:get mcu real charge current limit result
 * Parameters:value:mcu real charge current
 * return:NA
 */
static void watch_get_mcu_ichg_limit_result(int value)
{
	g_charge_info->sysfs_data.ichg_limit = value;
	hwlog_info("%s mcu real charge current:%d mA\n", __func__, value);
}

/*
 * Function:watch_get_ichg_limit
 * Description:mcu real charge current to sysfs node show
 * Parameters:value:mcu real charge current
 * return:NA
 */
static int watch_get_ichg_limit(unsigned int *value)
{
	if (!value)
		return -ENODEV;

	*value = g_charge_info->sysfs_data.ichg_limit;
	return 0;
}

/*
 * Function:watch_set_vterm_limit
 * Description:set charge cut-off voltage to mcu
 * Parameters:value:charge cut-off voltage
 * return:NA
 */
static int watch_set_vterm_limit(unsigned int value)
{
	int ret;

	hwlog_info("%s user set cut-off voltage:%d mV\n", __func__, value);
	g_charge_info->sysfs_data.vterm_limit = value;
	ret = watch_send_cmd_to_mcu_charge(SET_TERM_VOLT, value);
	if (ret < 0)
		hwlog_err("%s set charge cut-off voltage fail ret:%d\n",
			__func__, ret);
	return 0;
}

/*
 * Function:watch_get_mcu_vterm_limit_result
 * Description:get mcu real cut-off voltage limit result
 * Parameters:value:mcu real cut-off voltage
 * return:NA
 */
static void watch_get_mcu_vterm_limit_result(int value)
{
	g_charge_info->sysfs_data.vterm_limit = value;
	hwlog_info("%s mcu set real cut-off voltage:%d mV\n", __func__, value);
}

/*
 * Function:watch_get_vterm_limit
 * Description:mcu real cut-off voltage to sysfs node show
 * Parameters:value:mcu real cut-off voltage
 * return:0
 */
static int watch_get_vterm_limit(unsigned int *value)
{
	if (!value)
		return -ENODEV;

	*value = g_charge_info->sysfs_data.vterm_limit;
	return 0;
}

/*
 * Function:watch_set_hiz_enable
 * Description:set charge enable status to mcu
 * Parameters:value 0:disable 1:enable
 * return:NA
 */
static void watch_set_hiz_enable(unsigned int value)
{
	int ret;

	hwlog_info("%s user set hiz status:%d\n", __func__, value);
	g_charge_info->sysfs_data.hiz_status = value;
	ret = watch_send_cmd_to_mcu_charge(SET_ENABLE_HIZ, value);
	if (ret < 0)
		hwlog_err("%s set mcu hiz fail ret:%d\n",
			__func__, ret);
}

/*
 * Function:watch_get_mcu_hiz_enable_result
 * Description:get mcu real hiz enable status
 * Parameters:value 0:disable 1:enable
 * return:NA
 */
static void watch_get_mcu_hiz_enable_result(int value)
{
	hwlog_info("%s mcu real hiz enable result:%d\n", __func__, value);
	if (value == MCU_SET_SUCCESSFUL)
		return;

	if (g_charge_info->sysfs_data.hiz_status == STATUS_TRUE)
		g_charge_info->sysfs_data.hiz_status = STATUS_FALSE;
	else
		g_charge_info->sysfs_data.hiz_status = STATUS_TRUE;
}

/*
 * Function:watch_get_vterm_limit
 * Description:get charge done status form mcu
 * Parameters:NA
 * return:charge done status
 */
static unsigned int watch_get_charge_done_status(void)
{
	if (g_charge_info->charge_info.bat_cap < CHARGE_DONE_CAPACITY)
		g_charge_info->sysfs_data.charge_done_status = MCU_CHARGE_NOT_DONE;
	else
		g_charge_info->sysfs_data.charge_done_status = MCU_CHARGE_DONE;

	hwlog_info("%s mcu report capacity:%d, charge_done_status:%d\n",
		__func__, g_charge_info->charge_info.bat_cap,
		g_charge_info->sysfs_data.charge_done_status);

	return g_charge_info->sysfs_data.charge_done_status;
}

static void watch_set_wireles_aligned_status(int status)
{
	hwlog_info("%s wireless last aligned status:%d,mcu report status:%d\n",
		__func__, g_charge_info->charge_info.wireless_aligned_status, status);

	if (g_charge_info->charge_info.wireless_aligned_status == status)
		return;

	g_charge_info->charge_info.wireless_aligned_status = status;
	power_ui_event_notify(POWER_UI_NE_WL_OFF_POS, &status);
}

struct power_supply_ops watch_battery_power_supply_ops = {
	.type_name = "bat",
	.get_status_prop = watch_get_charge_status,
	.get_voltage_now_prop = watch_get_battery_voltage,
	.get_current_now_prop = watch_get_charge_current,
	.get_capacity_prop = watch_get_battery_capacity,
	.get_temp_prop = watch_get_battery_temperature,
	.get_batid_prop = watch_get_batid_voltage,
	.get_capacity_rm_prop = watch_get_battery_capacity_rm,
	.get_capacity_fcc_prop = watch_get_battery_capacity_fcc,
	.get_cycle_count_prop = watch_get_charge_cycle_count,
	.get_charge_full_prop = watch_get_battery_charge_full,
	.get_charge_full_design_prop = watch_get_battery_charge_full_design,
	.get_voltage_max_prop = watch_get_battery_voltage_max,
	.get_capacity_level_prop = watch_get_battery_capacity_level,
	.get_charge_now_prop = watch_get_battery_charge_now_capacity,
};

struct power_supply_ops watch_wireless_power_supply_ops = {
	.type_name = "wireless",
	.get_online_prop = watch_get_charger_online,
};

/*
 * Function:watch_get_charge_info
 * Description:init global charge info
 * Parameters:Nuawei_charge_info *info
 * return:NA
 */
static void watch_get_charge_info(struct huawei_charge_info *info)
{
	if (info->mcu_report_flag == STATUS_FALSE) {
		info->charge_info.charge_st = POWER_SUPPLY_STATUS_DISCHARGING;
		info->charge_info.bat_vol = DEFAULT_BAT_VOLTAGE;
		info->charge_info.bat_cur = DEFAULT_CHARGE_CURRENT;
		info->charge_info.bat_cap = DEFAULT_BAT_CAPACITY;
		info->charge_info.bat_temp = DEFAULT_BAT_TEMPERATURE;
		info->charge_info.online = CHARGER_OFFLINE;
		info->charge_info.bat_id = 0;
		info->charge_info.bat_cap_rm = DEFAULT_BAT_CAPACITY_RM;
		info->charge_info.bat_cap_fcc = DEFAULT_BAT_CAPACITY_FCC;
		info->charge_info.charge_cycle_cnt = 0;
		info->charge_info.bat_charge_full = DEFAULT_BAT_CAPACITY_FCC;
		info->charge_info.bat_charge_full_design =
			DEFAULT_BAT_CAPACITY_FCC;
		info->charge_info.bat_voltage_max = DEFAULT_BAT_VOLTAGE_MAX;
		info->charge_info.bat_capacity_level = CAPACITY_LVL_UNKNOWN;
		info->charge_info.bat_charge_now = DEFAULT_BAT_CAPACITY_RM;
		info->charge_info.wireless_aligned_status =
			WIRELESS_CHARGE_ALIGNED;
		info->low_power_off_flag = STATUS_FALSE;
		return;
	}
	info->charge_info.charge_st = g_charge_info->charge_info.charge_st;
	info->charge_info.bat_vol = g_charge_info->charge_info.bat_vol;
	info->charge_info.bat_cur = g_charge_info->charge_info.bat_cur;
	info->charge_info.bat_cap = g_charge_info->charge_info.bat_cap;
	info->charge_info.bat_temp = g_charge_info->charge_info.bat_temp;
	info->charge_info.online = g_charge_info->charge_info.online;
	info->charge_info.bat_id = g_charge_info->charge_info.bat_id;
	info->charge_info.bat_cap_rm = g_charge_info->charge_info.bat_cap_rm;
	info->charge_info.bat_cap_fcc = g_charge_info->charge_info.bat_cap_fcc;
	info->charge_info.charge_cycle_cnt =
		g_charge_info->charge_info.charge_cycle_cnt;
	info->charge_info.bat_charge_full =
		g_charge_info->charge_info.bat_charge_full;
	info->charge_info.bat_charge_full_design =
		g_charge_info->charge_info.bat_charge_full_design;
	info->charge_info.bat_voltage_max =
		g_charge_info->charge_info.bat_voltage_max;
	info->charge_info.bat_capacity_level =
		g_charge_info->charge_info.bat_capacity_level;
	info->charge_info.bat_charge_now =
		g_charge_info->charge_info.bat_charge_now;
	info->charge_info.wireless_aligned_status =
		g_charge_info->charge_info.wireless_aligned_status;
	info->low_power_off_flag = STATUS_FALSE;
}

/*
 * Function:watch_print_mcu_report_info
 * Description:print mcu initiative report charge info
 * Parameters:NA
 * return:NA
 */
static void watch_print_mcu_report_info(void)
{
	hwlog_info("%s mcu report charge status:%d\n", __func__,
		g_charge_info->charge_info.charge_st);
	hwlog_info("%s mcu report battery_voltage:%d mV\n", __func__,
		g_charge_info->charge_info.bat_vol);
	hwlog_info("%s mcu report battery_current:%d mA\n", __func__,
		g_charge_info->charge_info.bat_cur);
	hwlog_info("%s mcu report battery_capacity:%d%%\n", __func__,
		g_charge_info->charge_info.bat_cap);
	hwlog_info("%s mcu report battery_temperature:%d\n", __func__,
		g_charge_info->charge_info.bat_temp);
	hwlog_info("%s mcu report battery_id_voltage:%d mV\n", __func__,
		g_charge_info->charge_info.bat_id);
	hwlog_info("%s mcu report charger_online_status:%d\n", __func__,
		g_charge_info->charge_info.online);
	hwlog_info("%s mcu report bat_cap_rm:%d mAh\n", __func__,
		g_charge_info->charge_info.bat_cap_rm);
	hwlog_info("%s mcu report bat_cap_fcc:%d mAh\n", __func__,
		g_charge_info->charge_info.bat_cap_fcc);
	hwlog_info("%s mcu report charge_cycle_cnt:%d\n", __func__,
		g_charge_info->charge_info.charge_cycle_cnt);
	hwlog_info("%s mcu report bat_charge_full:%d mAh\n", __func__,
		g_charge_info->charge_info.bat_charge_full);
	hwlog_info("%s mcu report bat_charge_full_design:%d mAh\n", __func__,
		g_charge_info->charge_info.bat_charge_full_design);
	hwlog_info("%s mcu report bat_voltage_max:%d mV\n", __func__,
		g_charge_info->charge_info.bat_voltage_max);
	hwlog_info("%s mcu report bat_capacity_level:%d\n", __func__,
		g_charge_info->charge_info.bat_capacity_level);
	hwlog_info("%s mcu report wireless_aligned_status:%d\n", __func__,
		g_charge_info->charge_info.wireless_aligned_status);
	hwlog_info("%s mcu report bat_charge_now:%d mAh\n", __func__,
		g_charge_info->charge_info.bat_charge_now);
}

/*
 * Function:watch_resolve_mcu_report_info
 * Description:resolve mcu initiative report charge info
 * Parameters:NA
 * return:NA
 */
static void watch_resolve_mcu_report_info(
	struct mcu_report_work_struct *report_work)
{
	int i;
	int count[INFO_MAX] = {0};
	int last_online_status;

	g_charge_info->mcu_report_flag = STATUS_TRUE;

#ifdef CONFIG_HUAWEI_POWER_DEBUG
	for (i = 0; i < MCU_REPORT_BUFFER_LEN; i++)
		hwlog_debug("report_work->mcu_report_info:%02x\n",
			    report_work->mcu_report_info[i]);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
	for (i = 0; i < INFO_MAX; i++) {
		if (memcpy_s((char *)&count[i], RESOLVE_STEP_LEN,
			report_work->mcu_report_info + i * RESOLVE_STEP_LEN,
			RESOLVE_STEP_LEN) != EOK) {
			hwlog_err("%s memcpy_s report_info fail\n", __func__);
			return;
		}
#ifdef CONFIG_HUAWEI_POWER_DEBUG
		hwlog_debug("%s count[%d]=%d\n", __func__, i, count[i]);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
	}
	g_charge_info->charge_info.charge_st = count[CHARGE_ST];
	g_charge_info->charge_info.bat_vol = count[BAT_VOLTAGE];
	g_charge_info->charge_info.bat_cur = count[BAT_CURRENT];
	g_charge_info->charge_info.bat_cap = count[BAT_CAPACITY];
	g_charge_info->charge_info.bat_temp = count[BAT_TEMPERATURE];
	g_charge_info->charge_info.bat_id = count[BAT_ID];
	last_online_status = g_charge_info->charge_info.online;
	g_charge_info->charge_info.online = count[ONLINE_ST];
	g_charge_info->charge_info.bat_cap_rm = count[BAT_CAPACITY_RM];
	g_charge_info->charge_info.bat_cap_fcc = count[BAT_CAPACITY_FCC];
	g_charge_info->charge_info.charge_cycle_cnt = count[CHARGE_CYCLE_CNT];
	g_charge_info->charge_info.bat_charge_full = count[BAT_CHARGE_FULL];
	g_charge_info->charge_info.bat_charge_full_design =
		count[BAT_CHARGE_FULL_DESIGN];
	g_charge_info->charge_info.bat_voltage_max = count[BAT_VOLTAGE_MAX];
	g_charge_info->charge_info.bat_capacity_level = count[BAT_CAPACITY_LVL];
	watch_set_wireles_aligned_status(count[WIRELESS_ALIGN]);
	g_charge_info->charge_info.bat_charge_now = count[BAT_CAPACITY_RM];

	if (g_charge_info->charge_info.online != last_online_status) {
		if (g_charge_info->charge_info.online == 0) {
			hwlog_info("%s POWER_NE_USB_DISCONNECT\n", __func__);
			power_event_bnc_notify(POWER_BNT_CONNECT,
				POWER_NE_USB_DISCONNECT, NULL);
		} else {
			hwlog_info("%s POWER_NE_USB_CONNECT\n", __func__);
			power_event_bnc_notify(POWER_BNT_CONNECT,
				POWER_NE_USB_CONNECT, NULL);
		}
	}
	watch_print_mcu_report_info();
}

/*
 * Function:watch_low_power_poweroff_work
 * Description:If the low capacity had been reported for 90s��but app can not
 *             poweroff forcely poweroff to avoid battery overdraw
 * Parameters:struct work_struct *work
 * return:NA
 */
static void watch_low_power_poweroff_work(struct work_struct *work)
{
	hwlog_err("%s low power poweroff as ap can not poweroff\n", __func__);
	kernel_power_off();
}

/*
 * Function:watch_recv_mcu_data_check
 * Description:check the data validity according to command_id
 * Parameters:command_id:command id from double machine communication moudle
 *            data:mcu report data
 *            count:mcu report data length
 * return:NA
 */
static int watch_recv_mcu_data_check(unsigned char command_id,
	unsigned char *data, int count)
{
	switch (command_id) {
	case CHG_READ_COMMAND_ID:
		if (!data || count != MCU_REPORT_BUFFER_LEN) {
			hwlog_err("%s recv mcu report data error datalen:%d\n",
				__func__, count);
			return -EINVAL;
		}
		break;
	case CHG_WRITE_COMMAND_ID:
		if (!data || count != MCU_RESPONSE_BUFFER_LEN) {
			hwlog_err("%s recv mcu response data error datalen:%d\n",
				__func__, count);
			return -EINVAL;
		}
		break;
	default:
		hwlog_err("%s recv command_id invalid\n", __func__);
		return -EINVAL;
	}
	return 0;
}

/*
 * Function:watch_mcu_charge_monitor_work
 * Description:resolve mcu initiative report charge info and send
 *             power supply changed event to supply core and APP;
 * Parameters:struct work_struct *work
 * return:NA
 */
static void watch_mcu_charge_monitor_work(struct work_struct *work)
{
	static int pre_charge_online_st = DEFAULT_CHARGER_ONLINE_ST;

	struct mcu_report_work_struct *report_work =
	container_of(work, struct mcu_report_work_struct, report_work);

	if (!report_work)
		return;

	watch_resolve_mcu_report_info(report_work);
	if ((g_charge_info->charge_info.online != DEFAULT_CHARGER_ONLINE_ST) ||
		(g_charge_info->charge_info.online != pre_charge_online_st)) {
		pre_charge_online_st = g_charge_info->charge_info.online;
		power_supply_sync_changed("Wireless");
		LOG_JANK_D(JLID_USBCHARGING_START, "JL_USBCHARGING_START");
	}
	power_supply_sync_changed("Battery");
	kfree(report_work);
}

/*
 * Function:watch_wait_mcu_response_work
 * Description:resolve mcu response charge info and update corresponding
 *             power supply node;
 * Parameters:struct work_struct *work
 * return:NA
 */
static void watch_wait_mcu_response_work(struct work_struct *work)
{
	int i;
	int count[MCU_RESPONSE_MAX] = {0};
	struct wait_mcu_response_work_struct *response_work =
	container_of(work, struct wait_mcu_response_work_struct, response_work);

	if (!response_work)
		return;
#ifdef CONFIG_HUAWEI_POWER_DEBUG
	for (i = 0; i < MCU_RESPONSE_BUFFER_LEN; i++)
		hwlog_debug("mcu_response_info:%02x\n",
			    response_work->write_mcu_response_info[i]);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
	for (i = 0; i < MCU_RESPONSE_MAX; i++) {
		if (memcpy_s((char *)&count[i], RESOLVE_STEP_LEN,
			response_work->write_mcu_response_info + i * RESOLVE_STEP_LEN,
			RESOLVE_STEP_LEN) != EOK) {
			hwlog_err("%s memcpy_s response info fail\n", __func__);
			kfree(response_work);
			return;
		}
#ifdef CONFIG_HUAWEI_POWER_DEBUG
		hwlog_debug("%s count[%d]=%d\n", __func__, i, count[i]);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
	}
	kfree(response_work);
#ifdef CONFIG_HUAWEI_POWER_DEBUG
	hwlog_info("%s mcu response cmd:%d data:%d\n", __func__,
		   count[MCU_RESPONSE_CMD], count[MCU_RESPONSE_DATA]);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
	switch (count[MCU_RESPONSE_CMD]) {
	case GET_CHG_ST:
		return watch_set_charge_status(count[MCU_RESPONSE_DATA]);
	case GET_BAT_VOLT:
		return watch_set_battery_voltage(count[MCU_RESPONSE_DATA]);
	case GET_BAT_CAP:
		return watch_set_battery_capacity(count[MCU_RESPONSE_DATA]);
	case GET_BAT_TEMP:
		return watch_set_battery_temperature(count[MCU_RESPONSE_DATA]);
	case GET_CHG_CUR:
		return watch_set_charge_current(count[MCU_RESPONSE_DATA]);
	case SET_IIN_LIMIT:
		return watch_get_mcu_iin_limit_result(count[MCU_RESPONSE_DATA]);
	case SET_ICHG_LIMIT:
		return watch_get_mcu_ichg_limit_result(count[MCU_RESPONSE_DATA]);
	case SET_TERM_VOLT:
		return watch_get_mcu_vterm_limit_result(count[MCU_RESPONSE_DATA]);
	case SET_ENABLE_HIZ:
		return watch_get_mcu_hiz_enable_result(count[MCU_RESPONSE_DATA]);
	case SET_CHG_ST:
		return watch_get_mcu_charge_enable_result(count[MCU_RESPONSE_DATA]);
	default:
		return;
	}
}

/*
 * Function:watch_charge_recv_mcu_cb
 * Description:callback of double machine communication to get
 *             info mcu report charge
 * Parameters:NA
 * return:NA
 */
static int watch_charge_recv_mcu_cb(unsigned char service_id,
	unsigned char command_id, unsigned char *data, int data_len)
{
	int ret;
	struct wait_mcu_response_work_struct *resp_ws = NULL;
	struct mcu_report_work_struct *report_ws = NULL;

	if (service_id != CHG_SERVICE_ID) {
		hwlog_err("%s service_id:%d\n", __func__, service_id);
		return -EINVAL;
	}

	ret = watch_recv_mcu_data_check(command_id, data, data_len);
	if (ret != 0)
		return ret;

	switch (command_id) {
	case CHG_READ_COMMAND_ID:
		/* free after callback work has done */
		report_ws = kmalloc(sizeof(*report_ws), GFP_KERNEL);
		if (!report_ws)
			return -ENOMEM;
		INIT_WORK(&report_ws->report_work, watch_mcu_charge_monitor_work);
		if (memcpy_s(report_ws->mcu_report_info, MCU_REPORT_BUFFER_LEN,
			data, data_len) != EOK) {
			hwlog_err("%s memcpy mcu report data fail\n", __func__);
			return -ENOMEM;
		}
		if (!schedule_work(&report_ws->report_work)) {
			hwlog_err("%s schedule report work error\n", __func__);
			kfree(report_ws);
		}
		break;
	case CHG_WRITE_COMMAND_ID:
		/* free after callback work has done */
		resp_ws = kmalloc(sizeof(*resp_ws), GFP_KERNEL);
		if (!resp_ws)
			return -ENOMEM;
		INIT_WORK(&resp_ws->response_work, watch_wait_mcu_response_work);
		if (memcpy_s(resp_ws->write_mcu_response_info, MCU_RESPONSE_BUFFER_LEN,
			data, data_len) != EOK) {
			hwlog_err("%s memcpy response data fail\n", __func__);
			return -ENOMEM;
		}
		if (!schedule_work(&resp_ws->response_work)) {
			hwlog_err("%s schedule response work error\n", __func__);
			kfree(resp_ws);
		}
		break;
	default:
		break;
	}
	return 0;
}

/*
 * Function:watch_register_mcu_report_callback
 * Description:regtister call back to double machine communication
 * Parameters:NA
 * return:return valuse >=0 register successful else register fail for other
 *         reason
 */
static int watch_register_mcu_report_callback(void)
{
	return register_data_callback(CHARGE_CHANNEL, &g_read_mcu_cmds,
		watch_charge_recv_mcu_cb);
}

/*
 * Function:watch_get_charge_info_work
 * Description:mcu can not report charge info in time when system power on so
 * actively query mcu charge info every five second untill get mcu charge info
 * Parameters:struct work_struct *work
 * return:NA
 */
static void watch_get_charge_info_work(struct work_struct *work)
{
	int get_capacity = watch_send_cmd_to_mcu_charge(GET_BAT_CAP, DEFAULT_CMD);
	int get_temp = watch_send_cmd_to_mcu_charge(GET_BAT_TEMP, DEFAULT_CMD);

	hwlog_info("%s send command to get mcu charge info\n", __func__);
	if ((get_capacity < 0) || (get_temp < 0)) {
		g_get_charge_info_timer.expires = jiffies +
			GET_CHARGE_INFO_PERIOD * HZ;
		add_timer(&g_get_charge_info_timer);
		return;
	}
	hwlog_info("%s del get mcu charge info timer\n", __func__);
	del_timer(&g_get_charge_info_timer);
}

/*
 * Function:watch_get_charge_info_timer_callback
 * Description:schedule a work to get mcu charge info
 * Parameters:unsigned long arg
 * return:NA
 */
static DECLARE_WORK(get_charge_info_work, watch_get_charge_info_work);
static void watch_get_charge_info_timer_callback(unsigned long arg)
{
	schedule_work(&get_charge_info_work);
}

/*
 * Function:watch_get_mcu_charge_info_timer_init
 * Description: init get mcu charge info timer to actively query
 * mcu charge info every five second untill get mcu charge info
 * Parameters:NA
 * return:NA
 */
static void watch_get_mcu_charge_info_timer_init(void)
{
	hwlog_info("%s init get mcu charge info timer\n", __func__);
	init_timer(&g_get_charge_info_timer);
	g_get_charge_info_timer.function = watch_get_charge_info_timer_callback;
	g_get_charge_info_timer.expires = jiffies + GET_CHARGE_INFO_PERIOD * HZ;
	add_timer(&g_get_charge_info_timer);
}

static struct power_if_ops watch_charge_if_ops = {
	.set_enable_charger = watch_set_enable_charge,
	.get_enable_charger = watch_get_enable_charge,
	.set_iin_limit = watch_set_iin_limit,
	.get_iin_limit = watch_get_iin_limit,
	.set_ichg_limit = watch_set_ichg_limit,
	.get_ichg_limit = watch_get_ichg_limit,
	.set_vterm_dec = watch_set_vterm_limit,
	.get_vterm_dec = watch_get_vterm_limit,
	.type_name = "wl",
};

static void watch_init_sysfs_data(struct huawei_charge_info *info)
{
	info->sysfs_data.charge_done_status = MCU_CHARGE_NOT_DONE;
	info->sysfs_data.charge_enable = STATUS_FALSE;
	info->sysfs_data.hiz_status = DISABLE_HIZ_MODE;
	info->sysfs_data.ichg_limit = DEFAULT_CHARGE_CURRENT;
	info->sysfs_data.iin_limit = DEFAULT_CHARGE_CURRENT;
	info->sysfs_data.vterm_limit = DEFAULT_BAT_VOLTAGE_MAX;
}

#ifdef CONFIG_SYSFS
static ssize_t watch_charge_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t watch_charge_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info watch_charge_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(watch_charge, 0644,
		WATCH_CHARGE_SYSFS_IIN_RUNNINGTEST, iin_runningtest),
	power_sysfs_attr_rw(watch_charge, 0644,
		WATCH_CHARGE_SYSFS_ICHG_RUNNINGTEST, ichg_runningtest),
	power_sysfs_attr_rw(watch_charge, 0644,
		WATCH_CHARGE_SYSFS_REGULATION_VOLTAGE, regulation_voltage),
	power_sysfs_attr_rw(watch_charge, 0644,
		WATCH_CHARGE_SYSFS_HIZ, enable_hiz),
	power_sysfs_attr_ro(watch_charge, 0444,
		WATCH_CHARGE_SYSFS_CHARGE_TYPE, chargerType),
	power_sysfs_attr_ro(watch_charge, 0444,
		WATCH_CHARGE_SYSFS_CHARGE_DONE_STATUS, charge_done_status),
};

#define SYSFS_ATTRS_SIZE ARRAY_SIZE(watch_charge_sysfs_field_tbl)
static struct attribute *watch_charge_sysfs_attrs[SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group watch_charge_sysfs_attr_group = {
	.attrs = watch_charge_sysfs_attrs,
};

static enum watch_charger_type watch_get_charger_type(void)
{
	return CHARGER_TYPE_WIRELESS;
}

/*
 * Function:watch_charge_sysfs_show
 * Description:show the value for all charge device's node
 * Parameters:dev:device
 *            attr:device_attribute
 *            buf:string of node value
 *  return value:0:success others:fail
 */
static ssize_t watch_charge_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		watch_charge_sysfs_field_tbl,
		ARRAY_SIZE(watch_charge_sysfs_field_tbl));
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case WATCH_CHARGE_SYSFS_IIN_RUNNINGTEST:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u\n",
			g_charge_info->sysfs_data.iin_limit);
	case WATCH_CHARGE_SYSFS_ICHG_RUNNINGTEST:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u\n",
			g_charge_info->sysfs_data.ichg_limit);
	case WATCH_CHARGE_SYSFS_REGULATION_VOLTAGE:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u\n",
			g_charge_info->sysfs_data.vterm_limit);
	case WATCH_CHARGE_SYSFS_HIZ:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u\n",
			g_charge_info->sysfs_data.hiz_status);
	case WATCH_CHARGE_SYSFS_CHARGE_TYPE:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
			watch_get_charger_type());
	case WATCH_CHARGE_SYSFS_CHARGE_DONE_STATUS:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
			watch_get_charge_done_status());
	default:
		return 0;
	}
}

/*
 *  Function:watch_charge_sysfs_store
 *  Description:set the value for charge_data's node which is can be written
 *  Parameters:dev:device
 *             attr:device_attribute
 *             buf:string of node value
 *             count:unused
 *  return value:0:sucess others:fail
 */
static ssize_t watch_charge_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	long val = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		watch_charge_sysfs_field_tbl,
		ARRAY_SIZE(watch_charge_sysfs_field_tbl));
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case WATCH_CHARGE_SYSFS_IIN_RUNNINGTEST:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;

		watch_set_iin_limit(val);
		hwlog_info("%s user set input current:%d\n", __func__, val);
		break;
	case WATCH_CHARGE_SYSFS_ICHG_RUNNINGTEST:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;

		watch_set_ichg_limit(val);
		hwlog_info("%s user set charge current:%d\n", __func__, val);
		break;
	case WATCH_CHARGE_SYSFS_REGULATION_VOLTAGE:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;

		watch_set_vterm_limit(val);
		hwlog_info("%s user set terminal voltage:%d\n", __func__, val);
		break;
	case WATCH_CHARGE_SYSFS_HIZ:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;

		watch_set_hiz_enable(val);
		hwlog_info("%s user set hiz status:%d\n", __func__, val);
		break;
	default:
		break;
	}
	return count;
}

static void watch_charge_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(watch_charge_sysfs_attrs,
		watch_charge_sysfs_field_tbl, ARRAY_SIZE(watch_charge_sysfs_field_tbl));
	power_sysfs_create_link_group("hw_power", "charger", "charge_data",
		dev, &watch_charge_sysfs_attr_group);
}

static void watch_charge_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "charge_data",
		dev, &watch_charge_sysfs_attr_group);
}
#else
static inline void watch_charge_sysfs_create_group(struct device *dev)
{
}

static inline void watch_charge_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void watch_reset_process(void)
{
	if (g_charge_info->charge_info.bat_cap < RESET_POWEROFF_CAPACITY) {
		hwlog_info("%s low poweroff as cap below %d\n", __func__, RESET_POWEROFF_CAPACITY);
		set_reboot_reason(AP_S_COLDBOOT);
		machine_power_off();
	}
}

static int watch_reboot_handler(struct notifier_block *this, unsigned long mode, void *cmd)
{
	if (mode == SYS_RESTART)
		watch_reset_process();

	return 0;
}

static struct notifier_block watch_reboot_nb = {
	.notifier_call = watch_reboot_handler,
	.priority = INT_MAX,
};

static int watch_charge_probe(struct platform_device *pdev)
{
	int ret;
	struct huawei_charge_info *info = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->dev = &pdev->dev;
	np = info->dev->of_node;
	ret = watch_register_mcu_report_callback();
	if (ret < 0)
		hwlog_err("%s callback register fail ret=%d\n", __func__, ret);

	INIT_DELAYED_WORK(&info->poweroff_work, watch_low_power_poweroff_work);
	ret = power_supply_ops_register(&watch_battery_power_supply_ops);
	if (ret != 0)
		hwlog_err("%s bat_psy_ops register fail\n", __func__);

	ret = power_supply_ops_register(&watch_wireless_power_supply_ops);
	if (ret != 0)
		hwlog_err("%s wireless_psy_ops register fail\n", __func__);

	watch_get_charge_info(info);
	watch_init_sysfs_data(info);
	platform_set_drvdata(pdev, info);
	g_charge_info = info;

	watch_charge_sysfs_create_group(info->dev);
	power_if_ops_register(&watch_charge_if_ops);
	watch_get_mcu_charge_info_timer_init();

	if (!power_cmdline_is_factory_mode())
		power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_USB_DISCONNECT, NULL);

	ret = register_reboot_notifier(&watch_reboot_nb);
	if (ret)
		hwlog_err("%s register_watch_reboot_handler fail\n", __func__);

	return 0;
}

static int watch_charge_remove(struct platform_device *pdev)
{
	int ret;

	if (!g_charge_info)
		return -ENODEV;

	ret = unregister_data_callback(CHARGE_CHANNEL);
	if (ret < 0)
		hwlog_err("%s callback unreg fail ret=%d\n", __func__, ret);

	cancel_work(&g_charge_info->monitor_mcu_charge_work);
	cancel_delayed_work(&g_charge_info->poweroff_work);
	watch_charge_sysfs_remove_group(g_charge_info->dev);
	platform_set_drvdata(pdev, NULL);
	kfree(g_charge_info);
	g_charge_info = NULL;

	return 0;
}

static const struct of_device_id watch_charge_match_table[] = {
	{
		.compatible = "huawei,watch_charger",
		.data = NULL,
	},
	{},
};

static struct platform_driver watch_charge_driver = {
	.probe = watch_charge_probe,
	.remove = watch_charge_remove,
	.driver = {
		.name = "huawei,watch_charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(watch_charge_match_table),
	},
};

static int __init watch_charge_init(void)
{
	return platform_driver_register(&watch_charge_driver);
}

static void __exit watch_charge_exit(void)
{
	platform_driver_unregister(&watch_charge_driver);
}

late_initcall(watch_charge_init);
module_exit(watch_charge_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei watch_charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
