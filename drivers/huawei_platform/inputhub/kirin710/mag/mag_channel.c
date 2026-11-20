/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mag channel source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "mag_channel.h"

#include <linux/err.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_adapter.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/power/huawei_charger.h>
#include <linux/delay.h>

#include <securec.h>

#include "contexthub_route.h"
#include "mag_detect.h"

#define MAX_STR_CHARGE_SIZE 50
#define SEND_ERROR (-1)
#define SEND_SUC 0
#define MAG_CURRENT_FAC_RAIO 10000
#define CURRENT_MAX_VALUE 9000
#define CURRENT_MIN_VALUE 0
#define DELAY_5000MS 5000
#define CHARGER_10V2P25A 9
#define CHARGER_5V4P5A 2
#define UNPLUG_CHARGER 4
#define GET_CHARGER_CNT 3
#define MAX_INDEX 3
struct charge_current_mag_t {
	char str_charge[MAX_STR_CHARGE_SIZE];
	int current_offset_x;
	int current_offset_y;
	int current_offset_z;
	int current_value;
};

static int get_charger_flag = 0;
static unsigned int charge_index = CHARGE_TYPE_UNKNOWN;
static int current_mag_x_pre;
static int current_mag_y_pre;
static int current_mag_z_pre;
static char str_charger[] = "charger_plug_in_out";
static char str_charger_current_in[] = "charger_plug_current_in";
static char str_charger_current_out[] = "charger_plug_current_out";
static struct notifier_block charger_notify = {
	.notifier_call = NULL,
};
extern struct compass_platform_data mag_data;
extern struct hisi_nve_info_user user_info;

struct charge_current_mag_t charge_current_data;

static uint8_t msensor_akm_calibrate_data[MAX_MAG_AKM_CALIBRATE_DATA_LENGTH];
static uint8_t msensor_calibrate_data[MAX_MAG_CALIBRATE_DATA_LENGTH];

void send_mag_charger_to_mcu(void)
{
	if (send_calibrate_data_to_mcu(TAG_MAG, SUB_CMD_SET_OFFSET_REQ,
		&charge_current_data, sizeof(charge_current_data), false))
		hwlog_err("notify mag environment change failed\n");
	else
		hwlog_info("magnetic %s event ! current_offset = %d, %d, %d\n",
			charge_current_data.str_charge,
			charge_current_data.current_offset_x,
			charge_current_data.current_offset_y,
			charge_current_data.current_offset_z);
}

void get_current_charger_type(void)
{
	int current_charger_type = -1;
	int dcp_flag = -1;
	int fcp_flag = -1;
	int ada_type = -1;
	int count = 0;
	msleep(DELAY_5000MS);

	for (count = GET_CHARGER_CNT; count > 0; count--) {
		if (charge_get_charger_online()) {
			current_charger_type = 1;
			ada_type = dc_get_adapter_type();
			hwlog_info("get_current_charger_type current_charger_type = %d, ada_type = %d\n", current_charger_type, ada_type);
			if (ada_type == CHARGER_10V2P25A)
				charge_index =  CHARGE_TYPE_10V_2P25A;
			else if (ada_type == CHARGER_5V4P5A)
				charge_index =  CHARGE_TYPE_5V_4P5A;
		}
		if (ada_type == 0) {// The current charger is a fast charger.
			dcp_flag = charge_get_charger_online();
			if (dcp_flag == 1) {
				hwlog_info("ada_type = %d, dcp_flag = %d, fcp_flag = %d, current_charger_type = %d\n",
					ada_type, dcp_flag, fcp_flag, current_charger_type);
				fcp_flag = get_fcp_charging_flag();
				if (fcp_flag == 1)
					charge_index =  CHARGE_TYPE_9V_2A;
				else if (fcp_flag == 0)
					charge_index =  CHARGE_TYPE_5V_2A;
			}
		}
	}
}

int send_current_to_mcu_mag(int current_value_now)
{
	struct mag_device_info *dev_info = NULL;

	dev_info = mag_get_device_info(TAG_MAG);
	if (dev_info == NULL)
		return SEND_SUC;
	current_value_now = -current_value_now;

	if (get_charger_flag == 0) {
		get_current_charger_type();
		if (charge_index == CHARGE_TYPE_UNKNOWN)
			charge_index = CHARGE_TYPE_5V_2A;
		if (charge_index > MAX_INDEX)
			return SEND_ERROR;
		get_charger_flag = 1;
	}
	hwlog_info("send_current_to_mcu_mag: current_value_now = %d, charge_index =%d\n",
		 current_value_now,  charge_index);
	hwlog_info("send_current_to_mcu_mag: akm_current_x_fac = %x, akm_current_y_fac = %x, akm_current_z_fac = %x \n",
	   dev_info->akm_current_x_fac[charge_index], dev_info->akm_current_y_fac[charge_index], dev_info->akm_current_z_fac[charge_index]);

	if (current_value_now < CURRENT_MIN_VALUE ||
		current_value_now > CURRENT_MAX_VALUE)
		return SEND_ERROR;

	if ((dev_info->akm_current_x_fac[charge_index] == 0xFF) &&
		(dev_info->akm_current_y_fac[charge_index] == 0xFF) &&
		(dev_info->akm_current_z_fac[charge_index] == 0xFF) && get_mag_opened()) {
		hwlog_debug("The data is error\n");
		charge_current_data.current_offset_x = 0;
		charge_current_data.current_offset_y = 0;
		charge_current_data.current_offset_z = 0;
		charge_current_data.current_value = current_value_now;
		send_mag_charger_to_mcu();
		return SEND_SUC;
	}

	charge_current_data.current_offset_x = current_value_now *
		dev_info->akm_current_x_fac[charge_index] / MAG_CURRENT_FAC_RAIO;
	charge_current_data.current_offset_y = current_value_now *
		dev_info->akm_current_y_fac[charge_index] / MAG_CURRENT_FAC_RAIO;
	charge_current_data.current_offset_z = current_value_now *
		dev_info->akm_current_z_fac[charge_index] / MAG_CURRENT_FAC_RAIO;
	charge_current_data.current_value = current_value_now;

	if (((charge_current_data.current_offset_x != current_mag_x_pre) ||
		(charge_current_data.current_offset_y != current_mag_y_pre) ||
		(charge_current_data.current_offset_z != current_mag_z_pre)) &&
		get_mag_opened()) {
		hwlog_debug("get_mag_opened is true\n");
		current_mag_x_pre = charge_current_data.current_offset_x;
		current_mag_y_pre = charge_current_data.current_offset_y;
		current_mag_z_pre = charge_current_data.current_offset_z;
		send_mag_charger_to_mcu();
	}

	hwlog_info("current_offset_x = %d, current_offset_y = %d, current_offset_z = %d\n",
		charge_current_data.current_offset_x, charge_current_data.current_offset_y, charge_current_data.current_offset_z);
	hwlog_debug("current_mag_x_pre = %d, current_mag_y_pre = %d, current_mag_z_pre = %d\n",
	    current_mag_x_pre, current_mag_y_pre, current_mag_z_pre);
	return SEND_SUC;
}

void mag_charge_notify_close(void)
{
	hwlog_info("mag_charge_notify_close enter.\n");
	close_send_current();
	if (memset_s(&charge_current_data, sizeof(charge_current_data),
		0, sizeof(charge_current_data)) != EOK)
		return;
	if (memcpy_s(charge_current_data.str_charge,
		sizeof(charge_current_data.str_charge),
		str_charger_current_out,
		sizeof(str_charger_current_out)) != EOK)
		return;
	send_mag_charger_to_mcu();
}

void mag_charge_notify_open(void)
{
	hwlog_info("mag_charge_notify_open enter.\n");
	if (memset_s(&charge_current_data, sizeof(charge_current_data),
		0, sizeof(charge_current_data)) != EOK)
		return;
	if (memcpy_s(charge_current_data.str_charge,
		sizeof(charge_current_data.str_charge),
		str_charger_current_in,
		sizeof(str_charger_current_in)) != EOK)
		return;
	open_send_current(send_current_to_mcu_mag);
}

int mag_enviroment_change_notify(struct notifier_block *nb,
			unsigned long action, void *data)
{
	struct mag_device_info *dev_info = NULL;

	dev_info = mag_get_device_info(TAG_MAG);
	if (dev_info == NULL)
		return 0;

	hwlog_info("mag_enviroment_change_notify  chargerakm_need:%d action:%d\n", 
		dev_info->akm_need_charger_current, action);

	if (dev_info->akm_need_charger_current) {
		if (action == UNPLUG_CHARGER) {
			get_charger_flag = 0;
			mag_charge_notify_close();
		}
		else if ((action == 2) || (action == 0)) {
			hwlog_info("mag_enviroment_change_notify open action is %d\n",action);
			mag_charge_notify_open();
		}
	} else {
		if (memset_s(&charge_current_data, sizeof(charge_current_data),
			0, sizeof(charge_current_data)) != EOK)
			return 0;
		if (memcpy_s(charge_current_data.str_charge,
			sizeof(charge_current_data.str_charge),
			str_charger, sizeof(str_charger)) != EOK)
			return 0;
		send_mag_charger_to_mcu();
	}
	return 0;
}

int mag_current_notify(void)
{
	int ret = 0;

	if (mag_data.charger_trigger == 1) {
		charger_notify.notifier_call = mag_enviroment_change_notify;
		ret = chip_charger_type_notifier_register(&charger_notify);
		if (ret < 0)
			hwlog_err("mag_charger_type_notifier_register err\n");
	}
	hwlog_info("mag_current_notify  charger_trigger:%d\n", mag_data.charger_trigger);
	return ret;
}

int send_mag_calibrate_data_to_mcu(void)
{
	int mag_size;
	struct mag_device_info *dev_info = mag_get_device_info(TAG_MAG);

	if (dev_info == NULL) {
		hwlog_err("send_mag_calibrate_data_to_mcu get dev_info fail\n");
		return -1;
	}

	if (dev_info->akm_cal_algo == 1)
		mag_size = MAG_AKM_CALIBRATE_DATA_NV_SIZE;
	else
		mag_size = MAG_CALIBRATE_DATA_NV_SIZE;

	if (read_calibrate_data_from_nv(MAG_CALIBRATE_DATA_NV_NUM, mag_size, "msensor"))
		return -1;

	if (dev_info->akm_cal_algo == 1) {
		memcpy(msensor_akm_calibrate_data, user_info.nv_data, MAG_AKM_CALIBRATE_DATA_NV_SIZE);
		hwlog_info("send mag_sensor data %d, %d, %d to mcu success\n", msensor_akm_calibrate_data[0],
			msensor_akm_calibrate_data[1], msensor_akm_calibrate_data[2]);
		if (send_calibrate_data_to_mcu(TAG_MAG, SUB_CMD_SET_OFFSET_REQ, msensor_akm_calibrate_data,
			MAG_AKM_CALIBRATE_DATA_NV_SIZE, false))
			return -1;
	} else {
		memcpy(msensor_calibrate_data, user_info.nv_data, MAG_CALIBRATE_DATA_NV_SIZE);
		hwlog_info("send mag_sensor data %d, %d, %d to mcu success\n", msensor_calibrate_data[0],
			msensor_calibrate_data[1], msensor_calibrate_data[2]);
		if (send_calibrate_data_to_mcu(TAG_MAG, SUB_CMD_SET_OFFSET_REQ, msensor_calibrate_data,
			MAG_CALIBRATE_DATA_NV_SIZE, false))
			return -1;
	}

	return 0;
}

int write_magsensor_calibrate_data_to_nv(const char *src)
{
	int mag_size;
	struct mag_device_info *dev_info = mag_get_device_info(TAG_MAG);

	if (dev_info == NULL) {
		hwlog_err("send_mag_calibrate_data_to_mcu get dev_info fail\n");
		return -1;
	}

	if (!src) {
		hwlog_err("%s fail, invalid para\n", __func__);
		return -1;
	}

	if (dev_info->akm_cal_algo == 1) {
		mag_size = MAG_AKM_CALIBRATE_DATA_NV_SIZE;
		memcpy(&msensor_akm_calibrate_data, src,
			sizeof(msensor_akm_calibrate_data));
		if (write_calibrate_data_to_nv(MAG_CALIBRATE_DATA_NV_NUM,
			mag_size, "msensor", src))
			return -1;
	} else {
		mag_size = MAG_CALIBRATE_DATA_NV_SIZE;
		memcpy(&msensor_calibrate_data, src,
			sizeof(msensor_calibrate_data));
		if (write_calibrate_data_to_nv(MAG_CALIBRATE_DATA_NV_NUM,
			mag_size, "msensor", src))
			return -1;
	}

	return 0;
}

void reset_mag_calibrate_data(void)
{
	struct mag_device_info *dev_info = mag_get_device_info(TAG_MAG);

	if (dev_info == NULL)
		return;

	if (dev_info->akm_cal_algo == 1)
		send_calibrate_data_to_mcu(TAG_MAG, SUB_CMD_SET_OFFSET_REQ,
			msensor_akm_calibrate_data,
			MAG_AKM_CALIBRATE_DATA_NV_SIZE, true);
	else
		send_calibrate_data_to_mcu(TAG_MAG, SUB_CMD_SET_OFFSET_REQ,
			msensor_calibrate_data, MAG_CALIBRATE_DATA_NV_SIZE, true);
}
