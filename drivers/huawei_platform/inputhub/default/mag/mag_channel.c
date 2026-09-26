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

#include <securec.h>

#include "contexthub_route.h"
#include "mag_sysfs.h"

#define MAX_STR_CHARGE_SIZE 50
#define SEND_ERROR (-1)
#define SEND_SUC 0
#define MAG_CURRENT_FAC_RAIO 10000
#define CURRENT_MAX_VALUE 9000
#define CURRENT_MIN_VALUE 0

struct charge_current_mag_t {
	char str_charge[MAX_STR_CHARGE_SIZE];
	int current_offset_x;
	int current_offset_y;
	int current_offset_z;
	int current_value;
};

static int current_mag_x_pre;
static int current_mag_y_pre;
static int current_mag_z_pre;
static char str_charger[] = "charger_plug_in_out";
static char str_charger_current_in[] = "charger_plug_in";
static char str_charger_current_out[] = "charger_plug_out";
static struct notifier_block charger_notify = {
	.notifier_call = NULL,
};

struct charge_current_mag_t charge_current_data;

int send_mag_calibrate_data_to_mcu(void)
{
	struct mag_device_info *dev_info = NULL;
	int mag_size = MAG_CALIBRATE_DATA_NV_SIZE;

	dev_info = mag_get_device_info(TAG_MAG);
	if (dev_info == NULL)
		return -1;

	if (dev_info->mag_folder_function == 1)
		mag_size = MAG_FOLDER_CALIBRATE_DATA_NV_SIZE;
	else if (dev_info->akm_cal_algo == 1)
		mag_size = MAG_AKM_CALIBRATE_DATA_NV_SIZE;

	if (read_calibrate_data_from_nv(MAG_CALIBRATE_DATA_NV_NUM,
		mag_size, "msensor"))
		return -1;

	if (dev_info->mag_folder_function == 1) {
		if (memcpy_s(msensor_folder_calibrate_data,
			sizeof(msensor_folder_calibrate_data),
			user_info.nv_data, mag_size) != EOK)
			return -1;
		hwlog_info("send mag_sensor data %d, %d, %d to mcu success\n",
			msensor_folder_calibrate_data[0],
			msensor_folder_calibrate_data[1],
			msensor_folder_calibrate_data[2]);
		if (send_calibrate_data_to_mcu(TAG_MAG, SUB_CMD_SET_OFFSET_REQ,
			msensor_folder_calibrate_data, mag_size, false))
			return -1;
	} else if (dev_info->akm_cal_algo == 1) {
		if (memcpy_s(msensor_akm_calibrate_data,
			sizeof(msensor_akm_calibrate_data),
			user_info.nv_data, mag_size) != EOK)
			return -1;
		hwlog_info("send mag_sensor data %d, %d, %d to mcu success\n",
			msensor_akm_calibrate_data[0],
			msensor_akm_calibrate_data[1],
			msensor_akm_calibrate_data[2]);
		if (send_calibrate_data_to_mcu(TAG_MAG, SUB_CMD_SET_OFFSET_REQ,
			msensor_akm_calibrate_data, mag_size, false))
			return -1;
	} else {
		if (memcpy_s(msensor_calibrate_data,
			sizeof(msensor_calibrate_data),
			user_info.nv_data, mag_size) != EOK)
			return -1;
		hwlog_info("send mag_sensor data %d, %d, %d to mcu success\n",
			msensor_calibrate_data[0], msensor_calibrate_data[1],
			msensor_calibrate_data[2]);
		if (send_calibrate_data_to_mcu(TAG_MAG, SUB_CMD_SET_OFFSET_REQ,
			msensor_calibrate_data, mag_size, false))
			return -1;
	}

	return 0;
}

int send_mag1_calibrate_data_to_mcu(void)
{
	int mag_size;

	mag_size = MAG1_OFFSET_NV_SIZE;

	if (read_calibrate_data_from_nv(MAG1_OFFSET_NV_NUM,
		mag_size, MAG1_NV_NAME))
		return -1;

	if (memcpy_s(msensor1_calibrate_data,
		sizeof(msensor1_calibrate_data),
		user_info.nv_data, MAG1_OFFSET_NV_SIZE) != EOK)
		return -1;
	hwlog_info("send mag1_sensor data %d, %d, %d to mcu success\n",
		msensor1_calibrate_data[0], msensor1_calibrate_data[1],
		msensor1_calibrate_data[2]);
	if (send_calibrate_data_to_mcu(TAG_MAG1, SUB_CMD_SET_OFFSET_REQ,
		msensor1_calibrate_data, MAG1_OFFSET_NV_SIZE, false))
		return -1;

	return 0;
}

int write_mag1sensor_calibrate_data_to_nv(const char *src)
{
	int ret = 0;
	int mag_size;

	if (!src) {
		hwlog_err("%s fail, invalid para\n", __func__);
		return -1;
	}

	mag_size = MAG_CALIBRATE_DATA_NV_SIZE;
	if (memcpy_s(&msensor1_calibrate_data,
		sizeof(msensor1_calibrate_data),
		src, sizeof(msensor1_calibrate_data)) != EOK)
		return -1;
	if (write_calibrate_data_to_nv(MAG1_OFFSET_NV_NUM, mag_size,
		MAG1_NV_NAME, src))
		return -1;

	return ret;
}

int write_magsensor_calibrate_data_to_nv(const char *src, int length)
{
	struct mag_device_info *dev_info = NULL;
	int ret = 0;
	int mag_size;

	dev_info = mag_get_device_info(TAG_MAG);
	if (dev_info == NULL)
		return -1;

	if (!src) {
		hwlog_err("%s fail, invalid para\n", __func__);
		return -1;
	}

	/*
	 * for 2012 mag algo, calib data len is 12,
	 * for AKM mag algo, calib data len is 28.
	 * set max_mag_calib_nv_size to 28.
	 */
	if (length > MAG_MAX_CALIB_NV_SIZE)
		return -1;

	if (dev_info->mag_folder_function == 1) {
		mag_size = MAG_FOLDER_CALIBRATE_DATA_NV_SIZE;
		if (memcpy_s(&msensor_folder_calibrate_data,
			sizeof(msensor_folder_calibrate_data),
			src, sizeof(msensor_folder_calibrate_data)) != EOK)
			return -1;
		if (write_calibrate_data_to_nv(MAG_CALIBRATE_DATA_NV_NUM,
			mag_size, "msensor", src))
			return -1;
	} else if (dev_info->akm_cal_algo == 1) {
		mag_size = MAG_AKM_CALIBRATE_DATA_NV_SIZE;
		if (memcpy_s(&msensor_akm_calibrate_data,
			sizeof(msensor_akm_calibrate_data),
			src, sizeof(msensor_akm_calibrate_data)) != EOK)
			return -1;
		if (write_calibrate_data_to_nv(MAG_CALIBRATE_DATA_NV_NUM,
			mag_size, "msensor", src))
			return -1;
	} else {
		mag_size = MAG_CALIBRATE_DATA_NV_SIZE;
		if (memcpy_s(&msensor_calibrate_data,
			sizeof(msensor_calibrate_data),
			src, sizeof(msensor_calibrate_data)) != EOK)
			return -1;
		if (write_calibrate_data_to_nv(MAG_CALIBRATE_DATA_NV_NUM,
			mag_size, "msensor", src))
			return -1;
	}

	return ret;
}

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

int send_current_to_mcu_mag(int current_value_now)
{
	struct mag_device_info *dev_info = NULL;

	dev_info = mag_get_device_info(TAG_MAG);
	if (dev_info == NULL)
		return SEND_SUC;

	current_value_now = -current_value_now;
	if (current_value_now < CURRENT_MIN_VALUE ||
		current_value_now > CURRENT_MAX_VALUE)
		return SEND_ERROR;

	if ((dev_info->akm_current_x_fac == 0xFF) &&
		(dev_info->akm_current_y_fac == 0xFF) &&
		(dev_info->akm_current_z_fac == 0xFF) && get_mag_opened()) {
		charge_current_data.current_offset_x = 0;
		charge_current_data.current_offset_y = 0;
		charge_current_data.current_offset_z = 0;
		charge_current_data.current_value = current_value_now;
		send_mag_charger_to_mcu();
		return SEND_SUC;
	}

	charge_current_data.current_offset_x = current_value_now *
		dev_info->akm_current_x_fac / MAG_CURRENT_FAC_RAIO;
	charge_current_data.current_offset_y = current_value_now *
		dev_info->akm_current_y_fac / MAG_CURRENT_FAC_RAIO;
	charge_current_data.current_offset_z = current_value_now *
		dev_info->akm_current_z_fac / MAG_CURRENT_FAC_RAIO;
	charge_current_data.current_value = current_value_now;

	if (((charge_current_data.current_offset_x != current_mag_x_pre) ||
		(charge_current_data.current_offset_y != current_mag_y_pre) ||
		(charge_current_data.current_offset_z != current_mag_z_pre)) &&
		get_mag_opened()) {
		current_mag_x_pre = charge_current_data.current_offset_x;
		current_mag_y_pre = charge_current_data.current_offset_y;
		current_mag_z_pre = charge_current_data.current_offset_z;
		send_mag_charger_to_mcu();
	}
	return SEND_SUC;
}

void mag_charge_notify_close(void)
{
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

	if (dev_info->akm_need_charger_current) {
		if (dev_info->mag_folder_function == 1) {
			if (action == CHARGER_TYPE_NONE)
				mag_charge_notify_close();
			else
				mag_charge_notify_open();
		} else {
			if (action >= CHARGER_TYPE_NONE)
				mag_charge_notify_close();
			else
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
	struct compass_platform_data *pf_data = NULL;
	int ret = 0;

	pf_data = mag_get_platform_data(TAG_MAG);
	if (pf_data == NULL)
		return 0;
	if (pf_data->charger_trigger == 1) {
		charger_notify.notifier_call = mag_enviroment_change_notify;
		ret = chip_charger_type_notifier_register(&charger_notify);
		if (ret < 0)
			hwlog_err("mag_charger_type_notifier_register failed\n");
	}
	return ret;
}

void reset_mag_calibrate_data(void)
{
	struct mag_device_info *dev_info = NULL;

	dev_info = mag_get_device_info(TAG_MAG);
	if (dev_info == NULL)
		return;

	if (dev_info->mag_folder_function == 1)
		send_calibrate_data_to_mcu(TAG_MAG, SUB_CMD_SET_OFFSET_REQ,
			msensor_folder_calibrate_data,
			MAG_FOLDER_CALIBRATE_DATA_NV_SIZE, true);
	else if (dev_info->akm_cal_algo == 1)
		send_calibrate_data_to_mcu(TAG_MAG, SUB_CMD_SET_OFFSET_REQ,
			msensor_akm_calibrate_data,
			MAG_AKM_CALIBRATE_DATA_NV_SIZE, true);
	else
		send_calibrate_data_to_mcu(TAG_MAG, SUB_CMD_SET_OFFSET_REQ,
			msensor_calibrate_data, MAG_CALIBRATE_DATA_NV_SIZE, true);
}

