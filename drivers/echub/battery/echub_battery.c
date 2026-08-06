/*
 * echub_battery.c
 *
 * battery driver for echub
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/power_supply.h>
#include <linux/pm_wakeup.h>
#include <linux/notifier.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/rtc.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <securec.h>

#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/ioctl.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/interrupt.h>
#include "echub_battery.h"
#include "../echub.h"

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#define WINDOW_LEN                   (10)
#define CAPACITY_DEC_WINDOW_LEN                   (20)
#define ECHUB_I2C_ADAPTER                   (7)
#define ECHUB_AC_GPIO                       (23)
#define WORK_INTERVAL_QUICK                 (1000)

#define BATTERY_CAPACITY_FULL                 (100)
#define BATTERY_CAPACITY_LOW                  (10)
#define BATTERY_CAPACITY_CRITICAL             (3)
#define BATTERY_VOLTAGE_MAX_DESIGN_4S1P       (17600000)
#define BATTERY_VOLTAGE_MAX_DESIGN_2S2P       (8800000)

/* add CRC check define */
#define REG_BATT_LEN                          (0x01)
#define CRC_MASK                              (0x0F)
#define CRC_SHIFT                             (0x04)
#define CRC_LEN                               (0x09)

/* add batt i2c transfer define */
#define BATT_SEND_BYTE_LEN                    (0x04)
#define BATT_SEND_TRANS_LEN                   (0x02)

#ifdef CONFIG_HUAWEI_DSM
#define DSM_BAT_BUF_SIZE (256)
#define DSM_BAT_DETECTION_ERROR_NUM (920001001)
#define DSM_BAT_NOT_EXIST_NUM (920001002)
#define DSM_BAT_CAPACITY_CRITICAL_NUM (920001003)
#define DSM_BAT_CAHRGER_WAKEUP_FAIL_NUM (920001004)
#define DSM_BAT_WAKEUP_FAIL_NUM (920001005)
#define DSM_BAT_PRECHARGE_FAIL_NUM (920001006)
#define DSM_BAT_NORMAL_CHARGE_FAIL_NUM (920001007)
#define DSM_BAT_OVER_VOLTAGE_NUM (920001008)
#define DSM_BAT_OVER_TEMPERATURE_NUM (920001009)
#define DSM_BAT_PF_OR_SAFETY_ERROR_NUM (920001010)
#define DSM_BAT_RSOC_READ_FAIL_NUM (920001011)
#define DSM_BAT_STATIC_INFO_FAIL_NUM (920001012)
#define DSM_BAT_ERROR_NUM (920001013)
#endif


static struct wakeup_source low_power_lock;
static int is_low_power_locked;
static unsigned int capacity_filter[WINDOW_LEN];
static unsigned int capacity_sum;
//static unsigned int capacity_dec_filter[CAPACITY_DEC_WINDOW_LEN];
//static unsigned int capacity_dec_sum;
//static unsigned int capacity_dec_init_value;
//static unsigned int capacity_dec_cnt;
static unsigned int suspend_capacity;

#ifdef CONFIG_HUAWEI_DSM
static struct dsm_client *bat_dsm_client;
struct dsm_client *get_bat_dsm_client(void);

#define bat_dsm_report(err_no, fmt, args...)\
do {\
	if (get_bat_dsm_client()) {\
		if (!dsm_client_ocuppy(get_bat_dsm_client())) {\
			dsm_client_record(get_bat_dsm_client(), fmt, ##args); \
			dsm_client_notify(get_bat_dsm_client(), err_no); \
			printk(KERN_INFO "[BAT DSM]bat dsm report err = %d\n", err_no);\
		} else\
		    printk(KERN_ERR "[BAT DSM]bat dsm is busy err = %d\n", err_no);\
	} else\
		 printk(KERN_ERR "[BAT DSM]bat dsm client is null\n");\
} while (0)

struct dsm_dev dsm_bat = {
	.name = "dsm_bat",
	.device_name = "bat",
	.fops = NULL,
	.buff_size = DSM_BAT_BUF_SIZE,
};

struct dsm_client *get_bat_dsm_client(void)
{
	return bat_dsm_client;
}

#endif

struct echub_batt_device_info {
	int bat_cycle_count;
	int bat_voltage;
	int bat_volt_max_design;
	int bat_volt_avg;
	int bat_temperature;
	int bat_exist;
	int bat_health;
	int bat_capacity;
	int bat_capacity_level;
	int bat_technolog;
	int bat_design_fcc;
	int bat_rm;
	int bat_fcc;
	int bci_soc_at_term;
	int bat_current;
	int gpio_int;
	int irq_int;
	unsigned int bat_err;
	int charge_status;
	int power_supply_status;
	u8 usb_online;
	u8 ac_online;
	u8 chargedone_stat;
	u16 monitoring_interval;
	int watchdog_timer_status;
	unsigned long event;
	unsigned int capacity;
	unsigned int capacity_filter_count;
	unsigned int prev_capacity;
	unsigned int charge_full_count;
	unsigned int wakelock_enabled;
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0))
	struct power_supply bat;
	struct power_supply usb;
	struct power_supply bat_google;
	struct power_supply ac;
	struct power_supply bk_bat;
	#else
	struct power_supply    *bat;
	struct power_supply    *usb;
	struct power_supply    *bat_google;
	struct power_supply    *ac;
	struct power_supply    *wireless;
	struct power_supply    *bk_bat;
	#endif
	struct device *dev;
	struct notifier_block nb;
	struct delayed_work echub_batt_monitor_work;
	struct delayed_work irq_work;
	struct work_interval_para interval_data[WORK_INTERVAL_PARA_LEVEL];
	struct hrtimer capacity_dec_timer;
	int capacity_dec_base_decimal;
	int capacity_dec_sample_nums;
	int capacity_dec_timer_interval;
	int capacity_dec_total_level;
	struct capacity_dec_para cap_dec_para[CAPATICY_DEC_PARA_LEVEL];
	unsigned int capacity_dec;
	struct cap_lock_para vth_correct_data[CAP_LOCK_PARA_LEVEL];
	unsigned int vth_correct_en;
};

struct echub_batt_device_info *g_echub_bat_dev;

static enum power_supply_property echub_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN,
	POWER_SUPPLY_PROP_ENERGY_FULL,
	POWER_SUPPLY_PROP_ENERGY_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
};

static enum power_supply_property echub_ac_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_HEALTH,
};

static unsigned char get_transfer_date(char devaddr, int regoffset, char regbatt, char *value, int index)
{
	unsigned char data;

	switch (index) {
	case 0:
		data = (unsigned char)(devaddr << 1);
		break;
	case 1:
		data = (unsigned char)(regoffset >> 8);
		break;
	case 2:
		data = (unsigned char)(regoffset & 0xFF);
		break;
	case 3:
		data = REG_BATT_LEN;
		break;
	case 4:
		data = regbatt;
		break;
	case 5:
		data = (unsigned char)(devaddr << 1) + 1;
		break;
	case 6:
		data = value[0];
		break;
	case 7:
		data = value[1];
		break;
	case 8:
		data = value[2];
		break;
	default:
		pr_info("echub %s error\n", __func__);
		break;
	}
	return data;
}

static unsigned char calc_bus_PEC(char devaddr, int regoffset, char regbatt, char *value)
{
	static const char PEC_TABLE[] = {
		0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
		0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D
	};
	unsigned char i2cCmdPEC = 0;
	unsigned char sourcebyte = 0;
	int i = 0;

	for (i = 0; i < CRC_LEN; i++) {
		unsigned char data = 0;
		unsigned char crc = 0;

		sourcebyte = get_transfer_date(devaddr, regoffset, regbatt, value, i);
		data = i2cCmdPEC ^ sourcebyte;
		crc = (crc  << CRC_SHIFT) ^ PEC_TABLE[((crc ^ data) >> CRC_SHIFT) & CRC_MASK];
		data <<= CRC_SHIFT;
		i2cCmdPEC = (crc  << CRC_SHIFT) ^ PEC_TABLE[((crc ^ data) >> CRC_SHIFT) & CRC_MASK];
	}
	return i2cCmdPEC;
}

int echub_batt_get_info(char devaddr, int regoffset, char regbatt, char *value, int len)
{
	struct i2c_adapter *adap;
	int ret;
	struct i2c_msg msg[2];
	unsigned char data[4];

	adap = i2c_get_adapter(ECHUB_I2C_ADAPTER);
	if (!adap)
		return -ENODEV;
	msg[0].addr = devaddr;
	msg[0].flags = 0;
	msg[0].len = BATT_SEND_BYTE_LEN;
	msg[0].buf = data;
	data[0] = (u8)(regoffset >> 8);
	data[1] = (u8)(regoffset & 0xFF);
	data[2] = 0x01;
	data[3] = regbatt;

	msg[1].addr = devaddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = value;

	ret = i2c_transfer(adap, msg, BATT_SEND_TRANS_LEN);
	i2c_put_adapter(adap);

	/* check batt transfer result */
	if (ret != BATT_SEND_TRANS_LEN)
		return -ENODEV;
	if (value[0]) {
		echub_err("get battery info failed, err_flag = %d\n", value[0]);
		return -EIO;
	}
	if (regoffset == 0x0280) {
		if (value[1] != 1)
			return -EINVAL;
		if (value[3] != calc_bus_PEC(devaddr, regoffset, regbatt, value))
			return -EBADMSG;
	}
	if (regoffset == 0x0451) {
		if (value[1] != 2)
			return -EINVAL;
	}

	return 0;
}

static int echub_state_i2c_recv(char devaddr, int regoffset, char *value, int len)
{
	struct i2c_adapter *adap;
	int err;
	struct i2c_msg msg[2];
	unsigned char data[4];

	adap = i2c_get_adapter(ECHUB_I2C_ADAPTER);
	if (!adap)
		return -ENODEV;
	msg[0].addr = devaddr;
	msg[0].flags = 0;
	msg[0].len = 4;
	msg[0].buf = data;
	data[0] = 0x02;
	data[1] = 0xB2;
	data[2] = (u8)(regoffset >> 8);
	data[3] = (u8)(regoffset & 0xFF);

	msg[1].addr = devaddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = value;

	err = i2c_transfer(adap, msg, 2);

	i2c_put_adapter(adap);
	if (err > 0)
		return 0;
	return err;
}

static void echub_ac_irq_work(struct work_struct *work)
{
	struct echub_batt_device_info *di =
			container_of(work, struct echub_batt_device_info, irq_work.work);
	int ac_status = 0;

	ac_status = gpio_get_value(ECHUB_AC_GPIO);

	if (ac_status) {
		if (di->ac_online != 1) {
			di->ac_online = 1;
			queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(2000));
			power_supply_changed(di->ac);
			power_supply_changed(di->bat);
		}
		if (di->capacity == BATTERY_CAPACITY_FULL) {
			di->bat_current = 0;
			if (di->charge_status != POWER_SUPPLY_STATUS_FULL) {
				di->charge_status = POWER_SUPPLY_STATUS_FULL;
				queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(2000));
				power_supply_changed(di->ac);
				power_supply_changed(di->bat);
			}
		} else if (di->bat_current == 0) {
			if (di->charge_status != POWER_SUPPLY_STATUS_NOT_CHARGING) {
				di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
				queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(2000));
				power_supply_changed(di->ac);
				power_supply_changed(di->bat);
			}
		} else if (di->charge_status != POWER_SUPPLY_STATUS_CHARGING) {
			di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
			queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(2000));
			power_supply_changed(di->ac);
			power_supply_changed(di->bat);
		}
	} else if ((ac_status <= 0) && (di->charge_status != POWER_SUPPLY_STATUS_DISCHARGING)) {
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
		di->ac_online = 0;
		queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(2000));
		power_supply_changed(di->ac);
		power_supply_changed(di->bat);
	}
	queue_delayed_work(system_power_efficient_wq, &di->irq_work, msecs_to_jiffies(WORK_INTERVAL_QUICK));
}

static void update_charging_status(struct echub_batt_device_info *di, unsigned long event)
{
	if ((di->usb_online || di->ac_online) && di->capacity == CAPACITY_FULL &&
		 di->charge_status != POWER_SUPPLY_STATUS_NOT_CHARGING)
		di->charge_status = POWER_SUPPLY_STATUS_FULL;
	/*
	 * in case charger can not get the report of charger removed, so
	 * update the status of charger.
	 */
	if (event == VCHRG_CHARGE_DONE_EVENT)
		di->chargedone_stat = 1;
	else
		di->chargedone_stat = 0;
}

static void echub_battery_work(struct work_struct *work)
{
	struct echub_batt_device_info *di = container_of(work, struct echub_batt_device_info, echub_batt_monitor_work.work);
	char value[30] = {0};
	int ret = 0;

	ret = echub_batt_get_info(0x38, 0x0280, 0x90, value, 4);
	if (ret)
		goto echub_trans_failed;
	di->capacity = value[2];

	msleep(10);

	//bat_voltage
	ret = echub_batt_get_info(0x38, 0x0451, 0x6, value, 5);
	if (ret) {
		echub_err("battery: get bat_voltage failed: %d\n", ret);
		goto echub_trans_failed;
	}
	di->bat_voltage = (u16)(value[3]<<8) + value[2];

	msleep(10);

	//bat_current
	ret = echub_batt_get_info(0x38, 0x0451, 0x1, value, 5);
	if (ret) {
		echub_err("battery: get bat_current failed: %d\n", ret);
		goto echub_trans_failed;
	}
	di->bat_current = (short)((u16)(value[3]<<8) + (u16)value[2]);
	echub_info("battery: bat_current = %d\n", di->bat_current);

	msleep(10);

	//bat_remaining_capacity
	ret = echub_batt_get_info(0x38, 0x0451, 0x7, value, 5);
	if (ret) {
		echub_err("battery: get bat_rm failed: %d\n", ret);
		goto echub_trans_failed;
	}
	di->bat_rm = (u16)(value[3]<<8) + value[2];

	msleep(10);

	//di->bat_fcc
	ret = echub_batt_get_info(0x38, 0x0451, 0x8, value, 5);
	if (ret) {
		echub_err("battery: get bat_fcc failed: %d\n", ret);
		goto echub_trans_failed;
	}
	di->bat_fcc = (u16)(value[3]<<8) + value[2];

	msleep(10);

	//di->bat_temperature
	ret = echub_batt_get_info(0x38, 0x0451, 0x9, value, 5);
	if (ret) {
		echub_err("battery: get bat_temperature failed: %d\n", ret);
		goto echub_trans_failed;
	}
	di->bat_temperature = ((u16)(value[3]<<8) + value[2] - 2730 + 5) / 10;

	msleep(10);

	//di->bat_health
	ret = echub_batt_get_info(0x38, 0x0280, 0x82, value, 4);
	if (ret)
		goto echub_trans_failed;
	if ((value[2]>>4) % 2 == 0) {
		di->bat_health = POWER_SUPPLY_HEALTH_GOOD;
	} else {
		di->bat_health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
#ifdef CONFIG_HUAWEI_DSM
		bat_dsm_report(DSM_BAT_PF_OR_SAFETY_ERROR_NUM, "Battery PF error or Safety error\n");
#endif
	}
	if ((value[2]>>5) % 2 == 1) {
		di->bat_health = POWER_SUPPLY_HEALTH_OVERHEAT;
#ifdef CONFIG_HUAWEI_DSM
		bat_dsm_report(DSM_BAT_OVER_TEMPERATURE_NUM, "Battery is over temperature\n");
#endif
	}
	if ((value[2]>>6) % 2 == 1) {
		di->bat_health = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
#ifdef CONFIG_HUAWEI_DSM
		bat_dsm_report(DSM_BAT_OVER_VOLTAGE_NUM, "Battery is over voltage\n");
#endif
	}
#ifdef CONFIG_HUAWEI_DSM
	if ((value[2]>>2) % 2 == 1) {
		bat_dsm_report(DSM_BAT_ERROR_NUM, "Battery is error\n");
	}
#endif

	msleep(10);

	//di->bat_exist
	ret = echub_batt_get_info(0x38, 0x0280, 0x81, value, 4);
	if (ret) {
#ifdef CONFIG_HUAWEI_DSM
		bat_dsm_report(DSM_BAT_DETECTION_ERROR_NUM, "Check battery exist status error\n");
#endif
		goto echub_trans_failed;
	}
	if ((value[2]>>1) % 2 == 1) {
		di->bat_exist = 1;
	} else {
		di->bat_exist = 0;
#ifdef CONFIG_HUAWEI_DSM
		bat_dsm_report(DSM_BAT_NOT_EXIST_NUM, "Battery is not exist\n");
#endif
	}

	msleep(10);

	queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(di->monitoring_interval));
	power_supply_changed(di->bat);
	return;

echub_trans_failed:
	queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(di->monitoring_interval));
	return;
}

static int echub_ac_get_property(struct power_supply *psy,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct echub_batt_device_info *di = g_echub_bat_dev;

	if (!di) {
		echub_info("NULL point in [%s]\n", __func__);
		return -EINVAL;
	}

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		if (di->ac_online)
			val->intval = 1;
		else
			val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = di->power_supply_status;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/**********************************************************
* Function:		echub_battery_get_property
* Discription:		OS get battery property
* Parameters:		struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val,
* return value:	0-sucess or others-fail
**********************************************************/
static int echub_battery_get_property(struct power_supply *psy,
					 enum power_supply_property psp,
					 union power_supply_propval *val)
{
	struct echub_batt_device_info *di = g_echub_bat_dev;

	if (di == NULL) {
		echub_info("NULL point in [%s]\n", __func__);
		return -EINVAL;
	}
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if (di->ac_online == 1) {
			if (di->capacity == BATTERY_CAPACITY_FULL) {
				di->bat_current = 0;
				if (di->charge_status != POWER_SUPPLY_STATUS_FULL)
					di->charge_status = POWER_SUPPLY_STATUS_FULL;
			} else if (di->bat_current == 0) {
				if (di->charge_status != POWER_SUPPLY_STATUS_NOT_CHARGING)
					di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
			} else if (di->charge_status != POWER_SUPPLY_STATUS_CHARGING) {
				di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
			}
		}
		val->intval = di->charge_status;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = di->bat_voltage * 1000;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = di->bat_current * 1000;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		if (!di->bat_exist)
			val->intval = 0;
		else
			val->intval = di->bat_temperature * 10;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = di->bat_exist;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = di->ac_online;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		if (!di->bat_exist)
			val->intval = POWER_SUPPLY_HEALTH_UNKNOWN;
		else
			val->intval = di->bat_health;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = di->capacity;
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		if (!di->bat_exist) {
			di->bat_capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
		} else if (di->capacity <= BATTERY_CAPACITY_CRITICAL) {
			di->bat_capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
#ifdef CONFIG_HUAWEI_DSM
			bat_dsm_report(DSM_BAT_CAPACITY_CRITICAL_NUM, "Battery capacity is critical\n");
#endif
		} else if (di->capacity <= BATTERY_CAPACITY_LOW) {
			di->bat_capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_LOW;
		} else if (di->capacity == BATTERY_CAPACITY_FULL) {
			di->bat_capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_FULL;
		} else {
			di->bat_capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
		}
		val->intval = di->bat_capacity_level;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		if (!di->bat_exist)
			di->bat_technolog = POWER_SUPPLY_TECHNOLOGY_UNKNOWN;
		else
			di->bat_technolog = POWER_SUPPLY_TECHNOLOGY_LION;
		val->intval = di->bat_technolog;
		break;
	case POWER_SUPPLY_PROP_CHARGE_NOW:
		val->intval = di->bat_rm * 1000;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		val->intval = di->bat_fcc * 1000;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		if (!di->bat_exist)
			val->intval = 0;
		else
			val->intval = di->bat_design_fcc * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		if (!di->bat_exist)
			val->intval = 0;
		else
			val->intval = di->bat_volt_max_design;
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL:
		val->intval = di->bat_fcc * di->bat_volt_avg;
		break;
	case POWER_SUPPLY_PROP_ENERGY_NOW:
		val->intval = di->bat_rm * di->bat_volt_avg;
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:
		if (!di->bat_exist)
			val->intval = 0;
		else
			val->intval = di->bat_design_fcc * di->bat_volt_avg;
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		val->intval = di->bat_cycle_count;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static char *echub_supplied_to[] = {
	"echub_battery",
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
static const struct power_supply_config echub_battery_cfg = {
	.supplied_to = echub_supplied_to,
	.num_supplicants = ARRAY_SIZE(echub_supplied_to),
};

static const struct power_supply_desc echub_battery_desc = {
	.name			= "Battery",
	.type			= POWER_SUPPLY_TYPE_BATTERY,
	.properties		= echub_battery_props,
	.num_properties		= ARRAY_SIZE(echub_battery_props),
	.get_property		= echub_battery_get_property,
};

static const struct power_supply_desc echub_mains_desc = {
	.name			= "Mains",
	.type			= POWER_SUPPLY_TYPE_MAINS,
	.properties		= echub_ac_props,
	.num_properties		= ARRAY_SIZE(echub_ac_props),
	.get_property		= echub_ac_get_property,
};
#endif
/**********************************************************
*  Function:       parse_cap_dec_dts
*  Discription:    parse the module dts config value of capacity decimal
*  Parameters:   np:device_node
*                      di:echub_batt_device_info
*  return value:  null
**********************************************************/
static void parse_cap_dec_dts(struct device_node *np, struct echub_batt_device_info *di)
{
	int ret = 0;
	int i = 0;
	int array_len = 0;
	u32 cap_dec_tmp[CAP_DEC_PARA_TOTAL*CAPATICY_DEC_PARA_LEVEL] = {0};

	array_len = of_property_count_u32_elems(np, "capacity_dec_para");
	if ((array_len <= 0) || (array_len % CAP_DEC_PARA_TOTAL != 0)) {
		echub_err("capacity_dec_para is invaild, please check capacity_dec_para number!!\n");
		goto FuncEnd;
	} else if (array_len > CAPATICY_DEC_PARA_LEVEL * CAP_DEC_PARA_TOTAL) {
		echub_err("capacity_dec_para is too long, please check capacity_dec_para number!!\n");
		goto FuncEnd;
	} else {
		ret = of_property_read_u32_array(np, "capacity_dec_para", cap_dec_tmp, array_len);
		if (ret) {
			echub_err("dts:get capacity_dec_para fail!\n");
			goto FuncEnd;
		} else {
			di->capacity_dec_total_level = array_len/CAP_DEC_PARA_TOTAL;
			for (i = 0; i < di->capacity_dec_total_level; i++) {
				di->cap_dec_para[i].cap_dec_base_decimal =
					cap_dec_tmp[(int)(CAP_DEC_BASE_DECIMAL+CAP_DEC_PARA_TOTAL*i)];
				di->cap_dec_para[i].cap_dec_sample_nums =
					cap_dec_tmp[(int)(CAP_DEC_SAMPLE_NUMS+CAP_DEC_PARA_TOTAL*i)];
				di->cap_dec_para[i].cap_dec_timer_interval =
					cap_dec_tmp[(int)(CAP_DEC_TIMER_INTERVAL+CAP_DEC_PARA_TOTAL*i)];
				echub_info("cap_dec_para[%d], base_decimal: %-3d sample_nums: %-3d timer_interval: %-3d\n",
						i, di->cap_dec_para[i].cap_dec_base_decimal,
						di->cap_dec_para[i].cap_dec_sample_nums, di->cap_dec_para[i].cap_dec_timer_interval);
			}
			return;
		}
	}

FuncEnd:
	di->capacity_dec_total_level = 1;
	di->cap_dec_para[0].cap_dec_base_decimal = BASE_DECIMAL;
	di->cap_dec_para[0].cap_dec_sample_nums = CAPACITY_DEC_SAMPLE_NUMS;
	di->cap_dec_para[0].cap_dec_timer_interval = CAPATICY_DEC_TIMER_INTERVAL;
	echub_info("cap_dec_para[0], base_decimal: %-3d sample_nums: %-3d timer_interval: %-3d\n",
			di->cap_dec_para[0].cap_dec_base_decimal,
			di->cap_dec_para[0].cap_dec_sample_nums, di->cap_dec_para[0].cap_dec_timer_interval);
}
/**********************************************************
*  Function:       parse_cap_lock_dts
*  Discription:    parse the module dts config value
*  Parameters:   np:device_node
*                      di:echub_batt_device_info
*  return value:  0-sucess or others-fail
**********************************************************/
static int parse_vth_correct_dts(struct device_node *np, struct echub_batt_device_info *di)
{
	int i = 0;

	if (of_property_read_u32(np, "vth_correct_en", (u32 *)&di->vth_correct_en)) {
		di->vth_correct_en = 0;
		echub_err("error:get vth_correct_en value failed, used default value!\n");
	}

	if (of_property_read_u32_array(np, "vth_correct_para",
			(u32 *)&di->vth_correct_data[0],
			sizeof(di->vth_correct_data) / sizeof(int))) {
		di->vth_correct_en = 0;
		echub_err("get vth_correct_para fail!!\n");
		return -EINVAL;
	}

	for (i = 0; i < CAP_LOCK_PARA_LEVEL; i++) {
		echub_info("di->vth_correct_data[%d]:%d,%d\n", i,
			di->vth_correct_data[i].cap, di->vth_correct_data[i].level_vol);
	}
	return 0;

}

/**********************************************************
*  Function:       echub_batt_parse_dts
*  Discription:    parse the module dts config value
*  Parameters:   np:device_node
*                      di:echub_batt_device_info
*  return value:  0-sucess or others-fail
**********************************************************/
static int echub_batt_parse_dts(struct device_node *np, struct echub_batt_device_info *di)
{
	int ret = 0;
	int array_len = 0;
	int i = 0;
	const char *bci_data_string = NULL;
	int idata = 0;

	//if (of_property_read_u32(np, "battery_design_fcc", (u32 *)&di->bat_design_fcc)) {
	//	echub_err("error:get battery_design_fcc value failed, used default value from batt_parm!\n");
	//}

	if (of_property_read_u32(np, "bci_soc_at_term", (u32 *)&di->bci_soc_at_term)) {
		di->bci_soc_at_term = 100;
		echub_err("error:get bci_soc_at_term value failed, no early term in bci !\n");
	}
	echub_info("bci_soc_at_term =%d", di->bci_soc_at_term);

	/*bci_work_interval_para*/
	array_len = of_property_count_strings(np, "bci_work_interval_para");
	if ((array_len <= 0) || (array_len % WORK_INTERVAL_PARA_TOTAL != 0)) {
		echub_err("bci_work_interval_para is invaild,please check iput number!!\n");
		return -EINVAL;
	}

	if (array_len > WORK_INTERVAL_PARA_LEVEL * WORK_INTERVAL_PARA_TOTAL) {
		echub_err("bci_work_interval_para is too long,use only front %d paras!!\n", array_len);
		return -EINVAL;
	}

	ret = memset_s(di->interval_data, sizeof(di->interval_data),
			0, WORK_INTERVAL_PARA_LEVEL * sizeof(struct work_interval_para));
	if (ret) {
		echub_err("memset_s failed\n");
		return -EINVAL;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "bci_work_interval_para", i, &bci_data_string);
		if (ret) {
			echub_err("get bci_work_interval_para failed\n");
			return -EINVAL;
		}

		idata = simple_strtol(bci_data_string, NULL, 10);
		switch (i % WORK_INTERVAL_PARA_TOTAL) {
		case WORK_INTERVAL_CAP_MIN:
			if ((idata < CAPACITY_MIN) || (idata > CAPACITY_MAX)) {
				echub_err("the bci_work_interval_para cap_min is out of range!!\n");
				return -EINVAL;
			}
			di->interval_data[i / (WORK_INTERVAL_PARA_TOTAL)].cap_min = idata;
			break;
		case WORK_INTERVAL_CAP_MAX:
			if ((idata < CAPACITY_MIN) || (idata > CAPACITY_MAX)) {
				echub_err("the bci_work_interval_para cap_max is out of range!!\n");
				return -EINVAL;
			}
			di->interval_data[i / (WORK_INTERVAL_PARA_TOTAL)].cap_max = idata;
			break;
		case WORK_INTERVAL_VALUE:
			if ((idata < WORK_INTERVAL_MIN) || (idata > WORK_INTERVAL_MAX)) {
				echub_err("the bci_work_interval_para work_interval is out of range!!\n");
				return -EINVAL;
			}
			di->interval_data[i / (WORK_INTERVAL_PARA_TOTAL)].work_interval = idata;
			break;
		}
		echub_info("di->interval_data[%d][%d] = %d\n",
					i / (WORK_INTERVAL_PARA_TOTAL), i % (WORK_INTERVAL_PARA_TOTAL), idata);
	}
	parse_cap_dec_dts(np, di);
	ret |= parse_vth_correct_dts(np, di);
	return ret;
}

/**********************************************************
*  Function:       echub_battery_probe
*  Discription:    module probe
*  Parameters:     platform_device, i2c_device_id
*  return value:   0-sucess or others-fail
**********************************************************/
static int echub_battery_probe(struct platform_device *pdev)
{
	struct echub_batt_device_info *di = NULL;
	int low_bat_flag = 0;
	int ret = 0;
	char value[30] = {0};
	char temp = 0;
	struct device_node *np = NULL;

	echub_info("echub_battery Probe start\n");
	np = pdev->dev.of_node;
	if (np == NULL)
		return -1;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di) {
		echub_err("kzalloc err NULL\n");
		return -ENOMEM;
	}

	g_echub_bat_dev = di;
	di->monitoring_interval = WORK_INTERVAL_NOARMAL;
	di->dev = &pdev->dev;
	di->bat_health = POWER_SUPPLY_HEALTH_GOOD;
	di->bat_err = 0;
	di->power_supply_status = POWER_SUPPLY_HEALTH_GOOD;
	di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;

	//bat_design_fcc
	ret = echub_batt_get_info(0x38, 0x0280, 0xa2, value, 4);
	if (ret)
		echub_info("get bat_design_fcc failed %d\n", ret);
	temp = value[2];
	ret = echub_batt_get_info(0x38, 0x0280, 0xa3, value, 4);
	if (ret)
		echub_info("get bat_design_fcc failed %d\n", ret);
	di->bat_design_fcc = (u16)(value[2]<<8) + temp;
	echub_info("di->bat_design_fcc = %d\n", di->bat_design_fcc);

	//get bat_volt_avg
	ret = echub_batt_get_info(0x38, 0x0280, 0xa4, value, 4);
	if (ret)
		echub_info("get bat_volt_avg failed %d\n", ret);
	temp = value[2];
	ret = echub_batt_get_info(0x38, 0x0280, 0xa5, value, 4);
	if (ret)
		echub_info("get bat_volt_avg failed %d\n", ret);
	di->bat_volt_avg = (u16)(value[2]<<8) + temp;
	echub_info("di->bat_volt_avg = %d\n", di->bat_volt_avg);

	//bat_cycle_count
	ret = echub_batt_get_info(0x38, 0x0280, 0xAA, value, 4);
	if (ret)
		echub_info("get bat_cycle_count failed %d\n", ret);
	temp = value[2];
	ret = echub_batt_get_info(0x38, 0x0280, 0xAB, value, 4);
	if (ret)
		echub_info("get bat_cycle_count failed %d\n", ret);
	di->bat_cycle_count = (short)((u16)(value[2]<<8) + (u16)temp);
	echub_info("di->bat_cycle_count = %d\n", di->bat_cycle_count);

	/* Battery is 4S1S or 2S2S */
	if (di->bat_volt_avg < 8000) {
		echub_info("Battery is 2S\n");
		di->bat_volt_max_design = BATTERY_VOLTAGE_MAX_DESIGN_2S2P;
	} else if (di->bat_volt_avg < 12000) {
		echub_info("Battery is 3S\n");
		di->bat_volt_max_design = BATTERY_VOLTAGE_MAX_DESIGN_4S1P;
	} else {
		echub_info("Battery is 4S\n");
		di->bat_volt_max_design = BATTERY_VOLTAGE_MAX_DESIGN_4S1P;
	}

	di->capacity = -1;
	di->capacity_filter_count = 0;
	di->charge_full_count = 0;

	hrtimer_init(&di->capacity_dec_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	platform_set_drvdata(pdev, di);

	wakeup_source_init(&low_power_lock, "low_power_wake_lock");

	if ((low_bat_flag & BQ_FLAG_LOCK) == BQ_FLAG_LOCK) {
		__pm_stay_awake(&low_power_lock);
		is_low_power_locked = 1;
	}

	echub_batt_parse_dts(np, di);

	di->bat = power_supply_register(&pdev->dev, &echub_battery_desc, &echub_battery_cfg);
	if (IS_ERR(di->bat)) {
		echub_info("failed to register main battery\n");
		goto batt_failed;
	}

	di->ac = power_supply_register(&pdev->dev, &echub_mains_desc, NULL);
	if (IS_ERR(di->ac)) {
		echub_info("failed to register ac power supply\n");
		goto ac_failed;
	}

	INIT_DELAYED_WORK(&di->irq_work, echub_ac_irq_work);
	INIT_DELAYED_WORK(&di->echub_batt_monitor_work, echub_battery_work);
	queue_delayed_work(system_power_efficient_wq, &di->irq_work, 0);
	queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, 0);

#ifdef CONFIG_HUAWEI_DSM
	if (!bat_dsm_client)
		bat_dsm_client = dsm_register_client(&dsm_bat);

	if (!bat_dsm_client)
		printk(KERN_ERR "[RTC DSM]Battery dsm register failed\n");

	ret = echub_batt_get_info(0x38, 0x0280, 0xe1, value, 4);
	if (ret)
		echub_info("failed to get info from EC (0xe1)\n");
	if ((value[2]) % 2 == 1)
		bat_dsm_report(DSM_BAT_CAHRGER_WAKEUP_FAIL_NUM, "Wake up charger failed\n");
	if ((value[2]>>1) % 2 == 1)
		bat_dsm_report(DSM_BAT_WAKEUP_FAIL_NUM, "Wake up battery failed\n");
	if ((value[2]>>2) % 2 == 1)
		bat_dsm_report(DSM_BAT_PRECHARGE_FAIL_NUM, "Precharge configuration failed\n");
	if ((value[2]>>4) % 2 == 1)
		bat_dsm_report(DSM_BAT_NORMAL_CHARGE_FAIL_NUM, "Normal charging configuration failed\n");

	ret = echub_batt_get_info(0x38, 0x0280, 0xe2, value, 4);
	if (ret)
		echub_info("failed to get info from EC (0xe2)\n");
	if ((value[2]>>1) % 2 == 1)
		bat_dsm_report(DSM_BAT_RSOC_READ_FAIL_NUM, "Read rsoc failed\n");
	if ((value[2]>>2) % 2 == 1)
		bat_dsm_report(DSM_BAT_STATIC_INFO_FAIL_NUM, "Get battery static information failed\n");
#endif

	echub_info("echub_batt probe ok!\n");

	return 0;

ac_failed:
	power_supply_unregister(di->usb);
batt_failed:
	wakeup_source_trash(&low_power_lock);
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;
	return ret;
}

static int echub_battery_remove(struct platform_device *pdev)
{
	struct echub_batt_device_info *di = platform_get_drvdata(pdev);

	pr_info("remove\n");

	if (di == NULL) {
		echub_err("di is NULL!\n");
		return -ENODEV;
	}

	cancel_delayed_work(&di->echub_batt_monitor_work);
	wakeup_source_trash(&low_power_lock);
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;

	return 0;
}

void echub_send_pmoff_msg(void)
{
	int ret = 0;
	char value[2] = {0};

	ret = echub_state_i2c_recv(0x38, 0x0125, value, 2);
	pr_info("echub_send_pmoff_msg ret = %d value = %*ph\n", ret, 2, value);
	return;
}

void echub_send_pmreboot_msg(void)
{
	int ret = 0;
	char value[2] = {0};

	ret = echub_state_i2c_recv(0x38, 0x0117, value, 2);
	pr_info("echub_send_pmreboot_msg ret = %d value = %*ph\n", ret, 2, value);
	return;
}

static void echub_send_pmon_msg(void)
{
	int ret = 0;
	char value[2] = {0};

	ret = echub_state_i2c_recv(0x38, 0x0116, value, 2);
	pr_info("echub_send_pmon_msg ret = %d value = %*ph\n", ret, 2, value);
	return;
}

void echub_send_pmsuspend_msg(void)
{
	int ret = 0;
	char value[2] = {0};

	ret = echub_state_i2c_recv(0x38, 0x0123, value, 2);
	pr_info("echub_send_pmsuspend_msg ret = %d value = %*ph\n", ret, 2, value);
	return;
}

#ifdef CONFIG_PM
static int echub_battery_suspend(struct platform_device *pdev,
				    pm_message_t state)
{
	struct echub_batt_device_info *di = platform_get_drvdata(pdev);

	if (di == NULL) {
		echub_err("di is NULL!\n");
		return -ENODEV;
	}
	echub_info("%s:+\n", __func__);
	cancel_delayed_work(&di->echub_batt_monitor_work);
	echub_info("%s:-\n", __func__);
	return 0;
}

static int echub_battery_resume(struct platform_device *pdev)
{
	struct echub_batt_device_info *di = platform_get_drvdata(pdev);
	int i = 0, resume_capacity = 0;

	if (di == NULL) {
		echub_err("di is NULL!\n");
		return -ENODEV;
	}

	echub_info("%s:+\n", __func__);

	if (di->charge_status == POWER_SUPPLY_STATUS_DISCHARGING
	    || di->charge_status == POWER_SUPPLY_STATUS_NOT_CHARGING) {
		if ((suspend_capacity - resume_capacity) >= 2) {
			capacity_sum = 0;
			for (i = 0; i < WINDOW_LEN; i++) {
				capacity_filter[i] = resume_capacity;
				capacity_sum += capacity_filter[i];
			}
		}
	}
	queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, 0);
	echub_info("%s:-\n", __func__);
	return 0;
}
#endif /* CONFIG_PM */

static struct of_device_id echub_battery_match_table[] = {
	{
	.compatible = "huawei,echub_battery",
	.data = NULL,
	},
	{
	},
};

static struct platform_driver echub_battery_driver = {
	.probe = echub_battery_probe,
	.remove = echub_battery_remove,
#ifdef CONFIG_PM
	.suspend = echub_battery_suspend,
	.resume = echub_battery_resume,
#endif
	.driver = {
		   .name = "huawei,echub_battery",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(echub_battery_match_table),
		   },
};

static int __init echub_battery_init(void)
{
	echub_send_pmon_msg();
	return platform_driver_register(&echub_battery_driver);
}

module_init(echub_battery_init);

static void __exit echub_battery_exit(void)
{
	platform_driver_unregister(&echub_battery_driver);
}

module_exit(echub_battery_exit);

MODULE_LICENSE("GPL");
MODULE_ALIAS("huawei echub_battery");
