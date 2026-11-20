/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: Sensor Hub config
 */

#include <linux/err.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/module.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/power/hisi/coul/coul_drv.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#ifdef CONFIG_HW_TOUCH_KEY
#include <huawei_platform/sensor/huawei_key.h>
#endif
#include <securec.h>

#include "als_detect.h"
#include "als_para_table_ams.h"
#include "als_para_table_ams_tmd3702.h"
#include "als_para_table_ams_tmd3725.h"
#include "als_para_table_ams_tsl2591.h"
#include "als_para_table_avago.h"
#include "als_para_table_liteon.h"
#include "als_para_table_rohm.h"
#include "als_para_table_sensortek.h"
#include "als_para_table_silergy.h"
#include "als_para_table_vishay.h"
#include "contexthub_boot.h"
#include "contexthub_pm.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"
#include "protocol.h"
#include "sensor_channel.h"
#include "sensor_config.h"
#include "sensor_sysfs.h"
#include "tp_color.h"
#include "ps_para_table_ams_tmd2755.h"
#include "als_para_table_ams_tmd2755.h"
#include "als_para_table_emin_mn78911.h"
#include "ps_para_table_emin_mn78911.h"
#include "als_para_table_stk_stk33562.h"
#include "ps_para_table_stk_stk33562.h"
#include "mag_channel.h"

#define BH1745_MAX_THRESHOLD_NUM    23
#define BH1745_MIN_THRESHOLD_NUM    24

#define BU27006_MAX_THRESHOLD_NUM    25
#define BU27006_MIN_THRESHOLD_NUM    26
#define TCS3707_MAX_THRESHOLD_NUM    27
#define TCS3707_MIN_THRESHOLD_NUM    28

#define APDS9251_MAX_THRESHOLD_NUM    17
#define APDS9251_MIN_THRESHOLD_NUM    18

#define TMD3725_MAX_THRESHOLD_NUM    27
#define TMD3725_MIN_THRESHOLD_NUM    28

#define LTR582_MAX_THRESHOLD_NUM    24
#define LTR582_MIN_THRESHOLD_NUM    25

#define LTR578_APDS9922_MAX_THRESHOLD_NUM    8
#define LTR578_APDS9922_MIN_THRESHOLD_NUM    9

#define LTR578_MAX_THD_NUM     7
#define LTR578_MIN_THD_NUM     8

#define STK3321_MAX_THD_NUM    7
#define STK3321_MIN_THD_NUM    8

#define STK3235_MAX_THD_NUM    6
#define STK3235_MIN_THD_NUM    7

#define SY3079_MAX_THD_NUM     8
#define SY3079_MIN_THD_NUM     9

#define SY3133_MAX_THD_NUM     8
#define SY3133_MIN_THD_NUM     9

#define RPR531_MAX_THRESHOLD_NUM    14
#define RPR531_MIN_THRESHOLD_NUM    15

#define TMD2745_MAX_THRESHOLD_NUM    8
#define TMD2745_MIN_THRESHOLD_NUM    9

#define APDS9999_MAX_THRESHOLD_NUM    22
#define APDS9999_MIN_THRESHOLD_NUM    23

#define TMD3702_MAX_THRESHOLD_NUM    27
#define TMD3702_MIN_THRESHOLD_NUM    28

#define VCNL36658_MAX_THRESHOLD_NUM    29
#define VCNL36658_MIN_THRESHOLD_NUM    30

#define TSL2591_MAX_THRESHOLD_NUM    13
#define TSL2591_MIN_THRESHOLD_NUM    14

#define BH1726_MAX_THRESHOLD_NUM    14
#define BH1726_MIN_THRESHOLD_NUM    15

#define APDS9308_MAX_THD_NUM    12
#define APDS9308_MIN_THD_NUM    13

#define TP_COLOR_NV_NUM  16
#define TP_COLOR_NV_SIZE 15

int hisi_nve_direct_access(struct hisi_nve_info_user *user_info);

char sensor_chip_info[SENSOR_MAX][MAX_CHIP_INFO_LEN];
static u8 phone_color;
u8 tp_manufacture = TS_PANEL_UNKNOWN;
uint8_t ps_sensor_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
uint8_t tof_sensor_calibrate_data[TOF_CALIDATA_NV_SIZE];
struct hisi_nve_info_user user_info;
union sar_calibrate_data sar_calibrate_datas;
int gsensor_offset[ACC_CALIBRATE_DATA_LENGTH]; /* g-sensor calibrate data */
int gyro_sensor_offset[GYRO_CALIBRATE_DATA_LENGTH];
static char gyro_temperature_offset[GYRO_TEMP_CALI_NV_SIZE];
int ps_sensor_offset[PS_CALIBRATE_DATA_LENGTH];
static uint8_t tof_sensor_offset[TOF_CALIDATA_NV_SIZE];
uint16_t als_dark_noise_offset;
static char str_charger[] = "charger_plug_in_out";
static uint8_t gsensor_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
static uint8_t gyro_sensor_calibrate_data[GYRO_CALIDATA_NV_SIZE];
static uint8_t gyro_temperature_calibrate_data[GYRO_TEMP_CALI_NV_SIZE];
static uint8_t handpress_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
static char vib_calib[VIB_CALIDATA_NV_SIZE] = { 0 };

extern int first_start_flag;
extern int gyro_first_start_flag;
extern int handpress_first_start_flag;
extern int akm_cal_algo;
extern struct airpress_platform_data airpress_data;
extern struct sar_platform_data sar_pdata;
extern struct als_platform_data als_data;
extern struct compass_platform_data mag_data;

static atomic_t enabled = ATOMIC_INIT(0);
int (*send_func)(int) = NULL;

u8 get_phone_color(void)
{
	return phone_color;
}

char *get_sensor_chip_info_address(enum sensor_detect_list index)
{
	return sensor_chip_info[index];
}

uint8_t *get_ps_sensor_calibrate_data(void)
{
	return ps_sensor_calibrate_data;
}

uint8_t *get_tof_sensor_calibrate_data(void)
{
	return tof_sensor_calibrate_data;
}

void __dmd_log_report(int dmd_mark, const char *err_func, const char *err_msg)
{
#ifdef CONFIG_HUAWEI_DSM
	struct dsm_client *client = inputhub_get_shb_dclient();

	if (!dsm_client_ocuppy(client)) {
		dsm_client_record(client, "[%s]%s", err_func, err_msg);
		dsm_client_notify(client, dmd_mark);
	}
#endif
}

int read_calibrate_data_from_nv(int nv_number, int nv_size, char *nv_name)
{
	int ret;

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = nv_number;
	user_info.valid_size = nv_size;
	strncpy(user_info.nv_name, nv_name, sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("hisi_nve_direct_access read nv %d error %d\n",
			nv_number, ret);
		return -1;
	}
	return 0;
}

int write_calibrate_data_to_nv(int nv_number, int nv_size, const char *nv_name,
	const char *temp)
{
	int ret;
	struct hisi_nve_info_user local_user_info;

	memset(&local_user_info, 0, sizeof(local_user_info));
	local_user_info.nv_operation = NV_WRITE_TAG;
	local_user_info.nv_number = nv_number;
	local_user_info.valid_size = nv_size;
	strncpy(local_user_info.nv_name, nv_name,
		sizeof(local_user_info.nv_name));
	local_user_info.nv_name[sizeof(local_user_info.nv_name) - 1] = '\0';
	/* copy to nv by pass */
	memcpy(local_user_info.nv_data, temp,
		(sizeof(local_user_info.nv_data) < local_user_info.valid_size) ?
		sizeof(local_user_info.nv_data) : local_user_info.valid_size);
	ret = hisi_nve_direct_access(&local_user_info);
	if (ret != 0) {
		hwlog_err("hisi_nve_direct_access read nv %d error %d\n",
			nv_number, ret);
		return -1;
	}
	return 0;
}

int send_calibrate_data_to_mcu(int tag, uint32_t subcmd, const void *data,
	int length, bool is_recovery)
{
	int ret;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	pkt_parameter_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	pkg_ap.tag = tag;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	cpkt.subcmd = subcmd;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = length + SUBCMD_LEN;
	memcpy(cpkt.para, data, length);

	if (is_recovery)
		return write_customize_cmd(&pkg_ap, NULL, false);

	ret = write_customize_cmd(&pkg_ap,  &pkg_mcu, true);
	if (ret) {
		hwlog_err("send tag %d cali data err,ret=%d\n", tag, ret);
		return -1;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("send tag %d cali data fail,err=%d\n", tag,
			pkg_mcu.errno);
		return -1;
	}
	hwlog_info("send tag %d calibrate data to mcu success\n", tag);
	return 0;
}

int send_subcmd_data_to_mcu(int32_t tag, uint32_t subcmd, const void *data,
	uint32_t length, int32_t *err_no)
{
	struct write_info pkg_ap = { 0 };
	struct read_info pkg_mcu = { 0 };
	pkt_parameter_req_t spkt = { { 0 }, 0 };
	struct pkt_header *shd = (struct pkt_header *)&spkt;
	int ret;

	if (length > sizeof(spkt.para)) {
		hwlog_info("%s error tag=%d subcmd=%u length=%u\n", __func__,
			tag, subcmd, length);
		return -1;
	}

	spkt.subcmd = subcmd;
	pkg_ap.tag = tag;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = length + SUBCMD_LEN;
	if (data != NULL) {
		if (memcpy_s(spkt.para, sizeof(spkt.para),
			data, length) != EOK) {
			hwlog_info("%s memcpy_s data error tag=%d subcmd=%u\n",
				__func__, tag, subcmd);
			return -1;
		}
	}

	hwlog_info("%s g_iom3_state=%d,tag=%d,subcmd=%u\n",
		__func__, get_iom3_state(), pkg_ap.tag, subcmd);

	if ((get_iom3_state() == IOM3_ST_RECOVERY) ||
		(get_iomcu_power_state() == ST_SLEEP))
		ret = write_customize_cmd(&pkg_ap, NULL, false);
	else
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);

	if (ret)
		hwlog_info("%s error, tag=%d subcmd=%u ret=%d\n", __func__,
			tag, subcmd, ret);
	else if (pkg_mcu.errno != 0)
		hwlog_info("%s fail, tag=%d subcmd=%u errno=%d\n", __func__,
			tag, subcmd, pkg_mcu.errno);
	else
		hwlog_info("%s success, tag=%d subcmd=%u\n", __func__, tag,
			subcmd);

	if (err_no != NULL)
		*err_no = pkg_mcu.errno;

	return ret;
}

int write_gsensor_offset_to_nv(char *temp, int length)
{
	int ret = 0;

	if (!temp) {
		hwlog_err("%s fail, invalid para\n", __func__);
		return -1;
	}

	if (write_calibrate_data_to_nv(ACC_OFFSET_NV_NUM, length, "gsensor", temp))
		return -1;
	memcpy((void *)gsensor_calibrate_data, (void *)temp,
		MAX_SENSOR_CALIBRATE_DATA_LENGTH);
	memcpy(gsensor_offset, gsensor_calibrate_data,
		(sizeof(gsensor_offset) < ACC_OFFSET_NV_SIZE) ?
		sizeof(gsensor_offset) : ACC_OFFSET_NV_SIZE);
	hwlog_info("nve_direct_access write temp %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		gsensor_offset[0], gsensor_offset[1], gsensor_offset[2],
		gsensor_offset[3], gsensor_offset[4], gsensor_offset[5],
		gsensor_offset[6], gsensor_offset[7], gsensor_offset[8],
		gsensor_offset[9], gsensor_offset[10], gsensor_offset[11],
		gsensor_offset[12], gsensor_offset[13], gsensor_offset[14]);

	return ret;
}

int send_gsensor_calibrate_data_to_mcu(void)
{
	if (read_calibrate_data_from_nv(ACC_OFFSET_NV_NUM, ACC_OFFSET_NV_SIZE,
		"gsensor"))
		return -1;

	first_start_flag = 1;
	/* copy to gsensor_offset by pass */
	memcpy(gsensor_offset, user_info.nv_data, sizeof(gsensor_offset));
	hwlog_info("nve_direct_access read gsensor_offset_sen %d %d %d %d %d %d\n",
		gsensor_offset[0], gsensor_offset[1], gsensor_offset[2],
		gsensor_offset[3], gsensor_offset[4], gsensor_offset[5]);
	hwlog_info("nve_direct_access read gsensor_xis_angle %d %d %d %d %d %d %d %d %d\n",
		gsensor_offset[6], gsensor_offset[7], gsensor_offset[8],
		gsensor_offset[9], gsensor_offset[10], gsensor_offset[11],
		gsensor_offset[12], gsensor_offset[13], gsensor_offset[14]);

	memcpy(gsensor_calibrate_data, gsensor_offset,
		(sizeof(gsensor_calibrate_data) < ACC_OFFSET_NV_SIZE) ?
		sizeof(gsensor_calibrate_data) : ACC_OFFSET_NV_SIZE);
	if (send_calibrate_data_to_mcu(TAG_ACCEL, SUB_CMD_SET_OFFSET_REQ,
		gsensor_offset, ACC_OFFSET_NV_SIZE, false))
		return -1;

	return 0;
}

int send_gyro_calibrate_data_to_mcu(void)
{
	if (read_calibrate_data_from_nv(GYRO_CALIDATA_NV_NUM,
		GYRO_CALIDATA_NV_SIZE, "GYRO"))
		return -1;

	gyro_first_start_flag = 1;
	/* copy to gsensor_offset by pass */
	memcpy(gyro_sensor_offset, user_info.nv_data,
		sizeof(gyro_sensor_offset));
	hwlog_info("nve_direct_access read gyro_sensor offset: %d %d %d  sensitity:%d %d %d\n",
		gyro_sensor_offset[0], gyro_sensor_offset[1],
		gyro_sensor_offset[2], gyro_sensor_offset[3],
		gyro_sensor_offset[4], gyro_sensor_offset[5]);
	hwlog_info("nve_direct_access read gyro_sensor xis_angle: %d %d %d  %d %d %d %d %d %d\n",
		gyro_sensor_offset[6], gyro_sensor_offset[7],
		gyro_sensor_offset[8], gyro_sensor_offset[9],
		gyro_sensor_offset[10], gyro_sensor_offset[11],
		gyro_sensor_offset[12], gyro_sensor_offset[13],
		gyro_sensor_offset[14]);
	hwlog_info("nve_direct_access read gyro_sensor online offset: %d %d %d\n",
		gyro_sensor_offset[15], gyro_sensor_offset[16],
		gyro_sensor_offset[17]);

	memcpy(&gyro_sensor_calibrate_data, gyro_sensor_offset,
		(sizeof(gyro_sensor_calibrate_data) < GYRO_CALIDATA_NV_SIZE) ?
		sizeof(gyro_sensor_calibrate_data) : GYRO_CALIDATA_NV_SIZE);
	if (send_calibrate_data_to_mcu(TAG_GYRO, SUB_CMD_SET_OFFSET_REQ,
		gyro_sensor_offset, GYRO_CALIDATA_NV_SIZE, false))
		return -1;
	return 0;
}

int send_gyro_temperature_offset_to_mcu(void)
{
	if (read_calibrate_data_from_nv(GYRO_TEMP_CALI_NV_NUM,
		GYRO_TEMP_CALI_NV_SIZE, "GYTMP"))
		return -1;

	/* copy to gsensor_offset by pass */
	memcpy(gyro_temperature_offset, user_info.nv_data,
		sizeof(gyro_temperature_offset));

	if (send_calibrate_data_to_mcu(TAG_GYRO, SUB_CMD_GYRO_TMP_OFFSET_REQ,
		gyro_temperature_offset, sizeof(gyro_temperature_offset),
		false))
		return -1;

	memcpy(&gyro_temperature_calibrate_data, gyro_temperature_offset,
		(sizeof(gyro_temperature_calibrate_data) < sizeof(gyro_temperature_offset)) ?
		sizeof(gyro_temperature_calibrate_data) :
		sizeof(gyro_temperature_offset));
	return 0;
}

int write_gyro_sensor_offset_to_nv(char *temp, int length)
{
	if (!temp) {
		hwlog_err("%s err,invalid para\n", __func__);
		return -1;
	}

	if (write_calibrate_data_to_nv(GYRO_CALIDATA_NV_NUM, length, "GYRO",
		temp))
		return -1;

	memcpy(gyro_sensor_calibrate_data, temp,
		(sizeof(gyro_sensor_calibrate_data) < length) ?
		sizeof(gyro_sensor_calibrate_data) : length);
	memcpy(gyro_sensor_offset, gyro_sensor_calibrate_data,
		(sizeof(gyro_sensor_offset) < GYRO_CALIDATA_NV_SIZE) ?
		sizeof(gyro_sensor_offset) : GYRO_CALIDATA_NV_SIZE);
	hwlog_info("gyro calibrate data %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d,online offset %d %d %d lens=%d\n",
		gyro_sensor_offset[0], gyro_sensor_offset[1],
		gyro_sensor_offset[2], gyro_sensor_offset[3],
		gyro_sensor_offset[4], gyro_sensor_offset[5],
		gyro_sensor_offset[6], gyro_sensor_offset[7],
		gyro_sensor_offset[8], gyro_sensor_offset[9],
		gyro_sensor_offset[10], gyro_sensor_offset[11],
		gyro_sensor_offset[12], gyro_sensor_offset[13],
		gyro_sensor_offset[14], gyro_sensor_offset[15],
		gyro_sensor_offset[16], gyro_sensor_offset[17], length);
	return 0;
}

int write_ps_sensor_offset_to_nv(char *temp, int length)
{
	if (!temp) {
		hwlog_err("%s fail, invalid para\n", __func__);
		return -1;
	}

	if (write_calibrate_data_to_nv(PS_CALIDATA_NV_NUM, length, "PSENSOR",
		(char *)temp))
		return -1;

	hwlog_info("%s suc len=%d\n", __func__, length);

	return 0;
}

int write_gyro_temperature_offset_to_nv(char *temp, int length)
{
	if (!temp) {
		hwlog_err("write_gyro_temp_offset_to_nv fail, invalid para\n");
		return -1;
	}
	if (write_calibrate_data_to_nv(GYRO_TEMP_CALI_NV_NUM, length, "GYTMP",
		temp))
		return -1;

	memcpy(gyro_temperature_calibrate_data, temp,
		(sizeof(gyro_temperature_calibrate_data) < length) ?
		sizeof(gyro_temperature_calibrate_data) : length);
	hwlog_info("write_gyro_temp_offset_to_nv suc len=%d\n", length);
	return 0;
}

int send_vibrator_calibrate_data_to_mcu(void)
{
	if (read_calibrate_data_from_nv(VIB_CALIDATA_NV_NUM,
		VIB_CALIDATA_NV_SIZE, VIB_CALIDATA_NV_NAME))
		return -1;

	vib_calib[0] = (int8_t)user_info.nv_data[0];
	vib_calib[1] = (int8_t)user_info.nv_data[1];
	vib_calib[2] = (int8_t)user_info.nv_data[2];
	hwlog_err("%s read vib_calib 0x%x  0x%x  0x%x\n", __func__,
		vib_calib[0], vib_calib[1], vib_calib[2]);
	if (send_calibrate_data_to_mcu(TAG_VIBRATOR, SUB_CMD_SET_OFFSET_REQ,
		vib_calib, 3, false)) { /* 3 bytes */
		hwlog_err("send para fail\n");
		return -1;
	}
	return 0;
}


int send_ps_calibrate_data_to_mcu(void)
{
	struct ps_device_info *dev_info = NULL;

	dev_info = ps_get_device_info(TAG_PS);
	if (!dev_info)
		return -1;
	if (read_calibrate_data_from_nv(PS_CALIDATA_NV_NUM,
		PS_CALIDATA_NV_SIZE, "PSENSOR"))
		return -1;

	dev_info->ps_first_start_flag = 1;

	memcpy(ps_sensor_offset, user_info.nv_data, sizeof(ps_sensor_offset));
	hwlog_info("nve_direct_access read ps_offset %d,%d,%d\n",
		ps_sensor_offset[0], ps_sensor_offset[1], ps_sensor_offset[2]);
	memcpy(&ps_sensor_calibrate_data, ps_sensor_offset,
		(sizeof(ps_sensor_calibrate_data) < PS_CALIDATA_NV_SIZE) ?
		sizeof(ps_sensor_calibrate_data) : PS_CALIDATA_NV_SIZE);

	if (ps_external_ir_calibrate_flag == 1) {
		if (ps_sensor_offset[0] != 0 && ps_sensor_offset[1] != 0 &&
			ps_sensor_offset[2] != 0) {
			ps_external_ir_param.external_ir_pwindows_value =
				ps_sensor_offset[2] - ps_sensor_offset[1];
			ps_external_ir_param.external_ir_pwave_value =
				ps_sensor_offset[1] - ps_sensor_offset[0];
			ps_external_ir_param.external_ir_calibrate_noise =
				ps_sensor_offset[0];
			hwlog_info("%s:set ltr578 offset ps_data[0]:%d,ps_data[1]:%d,ps_data[2]:%d,pwindows:%d,pwave:%d\n",
				__func__, ps_sensor_offset[0], ps_sensor_offset[1], ps_sensor_offset[2],
				ps_external_ir_param.external_ir_pwindows_value, ps_external_ir_param.external_ir_pwave_value);
		}
	}

	if (send_calibrate_data_to_mcu(TAG_PS, SUB_CMD_SET_OFFSET_REQ,
		ps_sensor_offset, PS_CALIDATA_NV_SIZE, false))
		return -1;

	return 0;
}

int send_tof_calibrate_data_to_mcu(void)
{
	struct ps_device_info *dev_info = NULL;

	dev_info = ps_get_device_info(TAG_PS);
	if (!dev_info)
		return -1;
	if (read_calibrate_data_from_nv(PS_CALIDATA_NV_NUM,
		TOF_CALIDATA_NV_SIZE, "PSENSOR"))
		return -1;

	dev_info->ps_first_start_flag = 1;

	memcpy(tof_sensor_offset, user_info.nv_data, sizeof(tof_sensor_offset));
	hwlog_info("nve_direct_access read tof sensor offset offset[0]=%d offset[9]=%d offset[19]=%d offset[27]=%d\n",
		tof_sensor_offset[0], tof_sensor_offset[9], tof_sensor_offset[19], tof_sensor_offset[TOF_CALIDATA_NV_SIZE - 1]);

	memcpy(&tof_sensor_calibrate_data, tof_sensor_offset,
		(sizeof(tof_sensor_calibrate_data) < TOF_CALIDATA_NV_SIZE) ?
		sizeof(tof_sensor_calibrate_data) : TOF_CALIDATA_NV_SIZE);

	if (send_calibrate_data_to_mcu(TAG_TOF, SUB_CMD_SET_OFFSET_REQ,
		tof_sensor_offset, TOF_CALIDATA_NV_SIZE, false))
		return -1;

	return 0;
}

int send_als_calibrate_data_to_mcu(void)
{
	struct als_device_info *dev_info = NULL;

	dev_info = als_get_device_info(TAG_ALS);
	if (!dev_info)
		return -1;

	if (read_calibrate_data_from_nv(ALS_CALIDATA_NV_NUM,
		ALS_CALIDATA_NV_SIZE, "LSENSOR"))
		return -1;

	dev_info->als_first_start_flag = 1;
	if (memcpy_s(dev_info->als_offset, sizeof(dev_info->als_offset),
		user_info.nv_data, sizeof(dev_info->als_offset)) != EOK)
		return -1;
	if (memcpy_s(dev_info->als_sensor_calibrate_data,
		sizeof(dev_info->als_sensor_calibrate_data),
		dev_info->als_offset, ALS_CALIDATA_NV_SIZE) != EOK)
		return -1;
	hwlog_info("nve_direct_access read lsensor_offset %d %d %d %d %d %d\n",
		dev_info->als_offset[0], dev_info->als_offset[1],
		dev_info->als_offset[2], dev_info->als_offset[3],
		dev_info->als_offset[4], dev_info->als_offset[5]);

	if (send_calibrate_data_to_mcu(TAG_ALS, SUB_CMD_SET_OFFSET_REQ,
		dev_info->als_sensor_calibrate_data, ALS_CALIDATA_NV_SIZE, false))
		return -1;

	return 0;
}

int send_handpress_calibrate_data_to_mcu(void)
{
	uint8_t *offset = get_hp_offset();
	uint32_t len = MAX_HP_OFFSET_DATA_LENGTH;

	if (read_calibrate_data_from_nv(HANDPRESS_CALIDATA_NV_NUM,
		HANDPRESS_CALIDATA_NV_SIZE, "HPDATA"))
		return -1;

	handpress_first_start_flag = 1;
	if (memcpy_s(offset, len, user_info.nv_data,
		HANDPRESS_CALIDATA_NV_SIZE) != EOK)
		return -1;
	if (memcpy_s(&handpress_calibrate_data,
		sizeof(handpress_calibrate_data), offset,
		(sizeof(handpress_calibrate_data) < len) ?
		sizeof(handpress_calibrate_data) : len) != EOK)
		return -1;
	if (send_calibrate_data_to_mcu(TAG_HANDPRESS, SUB_CMD_SET_OFFSET_REQ,
		offset, HANDPRESS_CALIDATA_NV_SIZE, false))
		return -1;

	return 0;
}

int send_airpress_calibrate_data_to_mcu(void)
{
	if (read_calibrate_data_from_nv(AIRPRESS_CALIDATA_NV_NUM,
		AIRPRESS_CALIDATA_NV_SIZE, "AIRDATA"))
		return -1;

	/* send to mcu */
	memcpy(&airpress_data.offset, user_info.nv_data,
		AIRPRESS_CALIDATA_NV_SIZE);
	hwlog_info("airpress offset data=%d\n", airpress_data.offset);

	if (send_calibrate_data_to_mcu(TAG_PRESSURE, SUB_CMD_SET_OFFSET_REQ,
		&airpress_data.offset, AIRPRESS_CALIDATA_NV_SIZE, false))
		return -1;

	return 0;
}

int send_cap_prox_calibrate_data_to_mcu(void)
{
	if (strlen(sensor_chip_info[CAP_PROX]) == 0) {
		hwlog_info("isn't oversea phone,cap_prox isn't in board\n");
		return 0;
	}

	if (read_calibrate_data_from_nv(CAP_PROX_CALIDATA_NV_NUM,
		CAP_PROX_CALIDATA_NV_SIZE, "Csensor"))
		return -1;

	/* send to mcu */
	memcpy(&sar_calibrate_datas, user_info.nv_data,
		sizeof(sar_calibrate_datas));
	if (!strncmp(sensor_chip_info[CAP_PROX], "huawei,cypress_sar_psoc4000",
		strlen("huawei,cypress_sar_psoc4000"))) {
		hwlog_info("idac:%d, rawdata:%d length:%ld %ld\n",
			sar_calibrate_datas.cypres_cali_data.sar_idac,
			sar_calibrate_datas.cypres_cali_data.raw_data,
			sizeof(sar_calibrate_datas),
			sizeof(sar_calibrate_datas));
	} else if (!strncmp(sensor_chip_info[CAP_PROX], "huawei,semtech-sx9323",
		strlen("huawei,semtech-sx9323"))) {
		hwlog_info("sx9323:offset=%d, diff=%d length:%ld %ld\n",
			sar_calibrate_datas.semtech_cali_data.offset,
			sar_calibrate_datas.semtech_cali_data.diff,
			sizeof(sar_calibrate_datas),
			sizeof(sar_calibrate_datas));
	} else if (!strncmp(sensor_chip_info[CAP_PROX], "huawei,abov-a96t3x6",
		strlen("huawei,abov-a96t3x6"))) {
		hwlog_info("a96t3x6:offset[0]=%d, offset[1]=%d, diff[0]=%d, diff[1]=%d, length:%ld %ld\n",
			sar_calibrate_datas.abov_cali_data.offset[SAR_ABOV_CH_0],
			sar_calibrate_datas.abov_cali_data.offset[SAR_ABOV_CH_1],
			sar_calibrate_datas.abov_cali_data.diff[SAR_ABOV_CH_0],
			sar_calibrate_datas.abov_cali_data.diff[SAR_ABOV_CH_1],
			sizeof(sar_calibrate_datas),
			sizeof(sar_calibrate_datas));
	} else if (!strncmp(sensor_chip_info[CAP_PROX], "huawei,awi-aw9610x",
		strlen("huawei,awi-aw9610x"))) {
		hwlog_info("aw9610:offset=%d, diff=%d length:%ld %ld\n",
			sar_calibrate_datas.aw9610_cali_data.offset,
			sar_calibrate_datas.aw9610_cali_data.diff,
			sizeof(sar_calibrate_datas),
			sizeof(sar_calibrate_datas));
	} else {
		hwlog_info("CAP_PROX cal_offset[0],digi_offset[0]:%x,%x\n",
			sar_calibrate_datas.cap_cali_data.cal_offset[0],
			sar_calibrate_datas.cap_cali_data.digi_offset[0]);
		hwlog_info("CAP_PROX cal_fact_base[0],swap_flag[0]:%x,%x\n",
			sar_calibrate_datas.cap_cali_data.cal_fact_base[0],
			sar_calibrate_datas.cap_cali_data.swap_flag[0]);
	}
	if (send_calibrate_data_to_mcu(TAG_CAP_PROX, SUB_CMD_SET_OFFSET_REQ,
		(const void *)&sar_calibrate_datas,
		sizeof(sar_calibrate_datas), false))
		return -1;

	return 0;
}

static int send_add_data_to_mcu(int tag, uint32_t subcmd, const void *data,
	int length, bool is_recovery)
{
	int ret;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	pkt_parameter_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	memset(&cpkt, 0, sizeof(cpkt));

	pkg_ap.tag = tag;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	cpkt.subcmd = subcmd;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = length + SUBCMD_LEN;
	memcpy(cpkt.para, data, length);

	if (is_recovery)
		return write_customize_cmd(&pkg_ap, NULL, false);

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, false);
	if (ret) {
		hwlog_err("tag %d %s fail,ret=%d\n", tag, __func__, ret);
		return -1;
	}

	return 0;
}

extern struct adux_sar_add_data_t adux_sar_add_data;

int send_sar_add_data_to_mcu(void)
{
	if (strlen(sensor_chip_info[CAP_PROX]) == 0)
		return -1;

	/* send to mcu */
	if (!strncmp(sensor_chip_info[CAP_PROX], "huawei,cypress_sar_psoc4000",
		strlen("huawei,cypress_sar_psoc4000"))) {
	} else if (!strncmp(sensor_chip_info[CAP_PROX], "huawei,semtech-sx9323",
		strlen("huawei,semtech-sx9323"))) {
	} else if (!strncmp(sensor_chip_info[CAP_PROX], "huawei,adi-adux1050",
		strlen("huawei,adi-adux1050"))) {
		hwlog_info("CAP_PROX adi1050 %s\n", __func__);
		if (send_add_data_to_mcu(TAG_CAP_PROX,
			SUB_CMD_SET_ADD_DATA_REQ,
			(const void *)&adux_sar_add_data,
			sizeof(adux_sar_add_data), false))
			return -1;
	}

	return 0;
}

void reset_add_data(void)
{
	int ret;

	ret = send_sar_add_data_to_mcu();
	if (ret)
		hwlog_info("%s failed\n", __func__);
	else
		hwlog_info("%s success\n", __func__);
}

void reset_ps_calibrate_data(void)
{
	struct ps_device_info *dev_info = NULL;

	dev_info = ps_get_device_info(TAG_PS);
	if (!dev_info)
		return;

	if (dev_info->chip_type != PS_CHIP_NONE)
		send_calibrate_data_to_mcu(TAG_PS, SUB_CMD_SET_OFFSET_REQ,
			ps_sensor_calibrate_data, PS_CALIDATA_NV_SIZE, true);
}

static void reset_als_calibrate_data(void)
{
	uint32_t t;
	struct als_device_info *dev_info = NULL;

	dev_info = als_get_device_info(TAG_ALS);
	if (!dev_info) {
		hwlog_err("%s: dev_info is NULL\n", __func__);
		return;
	}

	t = dev_info->chip_type;
	if (t == ALS_CHIP_ROHM_RGB || t == ALS_CHIP_AVAGO_RGB ||
		t == ALS_CHIP_AMS_TMD3725_RGB || t == ALS_CHIP_LITEON_LTR582 ||
		dev_info->is_cali_supported == 1 || t == ALS_CHIP_APDS9999_RGB ||
		t == ALS_CHIP_AMS_TMD3702_RGB || t == ALS_CHIP_APDS9253_RGB||
		t == ALS_CHIP_VISHAY_VCNL36658 || t == ALS_CHIP_TSL2591 ||
		t == ALS_CHIP_BH1726 || t == ALS_CHIP_BU27006 || t == ALS_CHIP_TCS3707 ||
		t == ALS_CHIP_SY3133 || t == ALS_CHIP_TMD2755 || t == ALS_CHIP_STK33562 || t == ALS_CHIP_MN78911)
		send_calibrate_data_to_mcu(TAG_ALS, SUB_CMD_SET_OFFSET_REQ,
			dev_info->als_sensor_calibrate_data, ALS_CALIDATA_NV_SIZE, true);
}

void reset_calibrate_data(void)
{
	send_calibrate_data_to_mcu(TAG_ACCEL, SUB_CMD_SET_OFFSET_REQ,
		gsensor_calibrate_data, ACC_OFFSET_NV_SIZE, true);

	reset_mag_calibrate_data();
	reset_ps_calibrate_data();

	if (get_sensor_tof_flag() == 1)
		send_calibrate_data_to_mcu(TAG_TOF, SUB_CMD_SET_OFFSET_REQ,
			tof_sensor_calibrate_data, TOF_CALIDATA_NV_SIZE, true);

	reset_als_calibrate_data();

	if (strlen(sensor_chip_info[GYRO])) {
		send_calibrate_data_to_mcu(TAG_GYRO, SUB_CMD_SET_OFFSET_REQ,
			gyro_sensor_calibrate_data, GYRO_CALIDATA_NV_SIZE,
			true);
		send_calibrate_data_to_mcu(TAG_GYRO,
			SUB_CMD_GYRO_TMP_OFFSET_REQ,
			gyro_temperature_calibrate_data,
			GYRO_TEMP_CALI_NV_SIZE, true);
	}
	if (strlen(sensor_chip_info[AIRPRESS])) {
		hwlog_info("airpress offset data=%d\n", airpress_data.offset);
		send_calibrate_data_to_mcu(TAG_PRESSURE, SUB_CMD_SET_OFFSET_REQ, &airpress_data.offset, AIRPRESS_CALIDATA_NV_SIZE, true);
	}
	if (strlen(sensor_chip_info[HANDPRESS]))
		send_calibrate_data_to_mcu(TAG_HANDPRESS, SUB_CMD_SET_OFFSET_REQ, handpress_calibrate_data, AIRPRESS_CALIDATA_NV_SIZE, true);
	if (strlen(sensor_chip_info[CAP_PROX]))
		send_calibrate_data_to_mcu(TAG_CAP_PROX, SUB_CMD_SET_OFFSET_REQ, &sar_calibrate_datas, sizeof(sar_calibrate_datas), true);
}

int mcu_i2c_rw(uint8_t bus_num, uint8_t i2c_add, uint8_t *tx, uint32_t tx_len,
	uint8_t *rx_out, uint32_t rx_len)
{
	int ret;
	struct sensor_combo_cfg cfg;

	cfg.bus_type = TAG_I2C;
	cfg.bus_num = bus_num;
	cfg.i2c_address = i2c_add;

	ret = combo_bus_trans(&cfg, tx, tx_len, rx_out, rx_len);
	return (ret < 0) ? -1 : 0;
}

int mcu_spi_rw(uint8_t bus_num, union spi_ctrl ctrl, uint8_t *tx,
	uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len)
{
	int ret;
	struct sensor_combo_cfg cfg;

	cfg.bus_type = TAG_SPI;
	cfg.bus_num = bus_num;
	cfg.ctrl = ctrl;

	ret = combo_bus_trans(&cfg, tx, tx_len, rx_out, rx_len);
	return (ret < 0) ? -1 : 0;
}

int combo_bus_trans(struct sensor_combo_cfg *p_cfg, uint8_t *tx,
	uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len)
{
	int ret;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	pkt_combo_bus_trans_req_t *pkt_combo_trans = NULL;
	uint32_t cmd_wd_len;

	if (!p_cfg) {
		hwlog_err("%s: p_cfg null\n", __func__);
		return -1;
	}

	memset((void *)&pkg_ap, 0, sizeof(pkg_ap));
	memset((void *)&pkg_mcu, 0, sizeof(pkg_mcu));

	pkg_ap.tag = p_cfg->bus_type;

	/* check and get bus type */
	if (pkg_ap.tag == TAG_I2C) {
		pkg_ap.cmd = CMD_I2C_TRANS_REQ;
	} else if (pkg_ap.tag == TAG_SPI) {
		pkg_ap.cmd = CMD_SPI_TRANS_REQ;
	} else {
		hwlog_err("%s:bus_type %d err\n", __func__, p_cfg->bus_type);
		return -1;
	}

	if (tx_len >= (uint32_t)0xFFFF - sizeof(*pkt_combo_trans)) {
		hwlog_err("%s: tx_len %x too big\n", __func__, tx_len);
		return -1;
	}
	cmd_wd_len = tx_len + sizeof(*pkt_combo_trans);
	pkt_combo_trans = kzalloc((size_t)cmd_wd_len, GFP_KERNEL);
	if (!pkt_combo_trans) {
		hwlog_err("alloc failed in %s\n", __func__);
		return -1;
	}

	pkt_combo_trans->busid   = p_cfg->bus_num;
	pkt_combo_trans->ctrl    = p_cfg->ctrl;
	pkt_combo_trans->rx_len  = (uint16_t)rx_len;
	pkt_combo_trans->tx_len  = (uint16_t)tx_len;
	if (tx_len && tx)
		memcpy((void *)pkt_combo_trans->tx, (void *)tx,
			(unsigned long)tx_len);
	pkg_ap.wr_buf = ((struct pkt_header *)pkt_combo_trans + 1);
	pkg_ap.wr_len = (int)(cmd_wd_len - sizeof(pkt_combo_trans->hd));

	hwlog_info("%s: tag %d cmd %d data=%d, tx_len=%d,rx_len=%d\n",
		__func__, pkg_ap.tag, pkg_ap.cmd, p_cfg->data, tx_len, rx_len);

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("send cmd to mcu fail,data=%d,tx_len=%d,rx_len=%d\n",
			p_cfg->data, tx_len, rx_len);
		ret = -1;
	} else {
		if (pkg_mcu.errno != 0) {
			hwlog_err("mcu_rw fail, data=%d,tx_len=%d,rx_len=%d\n",
				p_cfg->data, tx_len, rx_len);
			ret = -1;
		} else {
			if (rx_out && rx_len)
				memcpy((void *)rx_out, (void *)pkg_mcu.data, (unsigned long)rx_len);
			ret = pkg_mcu.data_length;
		}
	}
	kfree(pkt_combo_trans);
	return ret;
}

int fill_extend_data_in_dts(struct device_node *dn, const char *name,
	unsigned char *dest, size_t max_size, int flag)
{
	int ret;
	int buf[SENSOR_PLATFORM_EXTEND_DATA_SIZE] = { 0 };
	struct property *prop = NULL;
	unsigned int len;
	unsigned int i;
	int *pbuf = buf;
	unsigned char *pdest = dest;

	if (!dn || !name || !dest)
		return -1;

	if (max_size == 0)
		return -1;

	prop = of_find_property(dn, name, NULL);
	if (!prop)
		return -EINVAL;

	if (!prop->value)
		return -ENODATA;

	len = prop->length / 4; /* 4: to word */
	if (len == 0 || len > max_size) {
		hwlog_err("In %s: len err! len = %d\n", __func__, len);
		return -1;
	}
	ret = of_property_read_u32_array(dn, name, buf, len);
	if (ret) {
		hwlog_err("In %s: read %s failed\n", __func__, name);
		return -1;
	}
	if (flag == EXTEND_DATA_TYPE_IN_DTS_BYTE) {
		for (i = 0; i < len; i++)
			*(dest + i) = (uint8_t)buf[i];
	}
	if (flag == EXTEND_DATA_TYPE_IN_DTS_HALF_WORD) {
		for (i = 0; i < len; i++) {
			/* copy 2 bytes */
			memcpy(pdest, pbuf, 2);
			pdest += 2; /* 2 bytes */
			pbuf++;
		}
	}
	if (flag == EXTEND_DATA_TYPE_IN_DTS_WORD)
		memcpy(dest, buf, len * sizeof(int));

	return 0;
}

int set_pinhole_para_flag;
int set_rpr531_para_flag;
int set_tmd2745_para_flag;
static DEFINE_MUTEX(mutex_set_para);

static BLOCKING_NOTIFIER_HEAD(tp_notifier_list);
int tpmodule_register_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&tp_notifier_list, nb);
}
EXPORT_SYMBOL(tpmodule_register_client);

int tpmodule_unregister_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&tp_notifier_list, nb);
}
EXPORT_SYMBOL(tpmodule_unregister_client);

int tpmodule_notifier_call_chain(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(&tp_notifier_list, val, v);
}
EXPORT_SYMBOL(tpmodule_notifier_call_chain);

void set_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	als_para_normal_table *als_para_diff_tp_color_table,
	int arraysize)
{
	u8 tplcd_manufacture = get_tplcd_manufacture();
	unsigned int i;
	int min_threshold_num;
	int max_threshold_num;

	for (i = 0; i < arraysize; i++) {
		if (((als_para_diff_tp_color_table + i)->phone_type == als_data.als_phone_type) &&
				((als_para_diff_tp_color_table + i)->phone_version == als_data.als_phone_version) &&
				(((als_para_diff_tp_color_table + i)->tp_manufacture == tplcd_manufacture) ||
				((als_para_diff_tp_color_table + i)->tp_manufacture == TS_PANEL_UNKNOWN)))
			break;
	}

	if (i >= arraysize)
		i = 0;
	dev_info->table_id = i;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		(als_para_diff_tp_color_table + i)->als_para,
		((sizeof(s16) * (als_para_diff_tp_color_table + i)->len) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		(sizeof(s16) * (als_para_diff_tp_color_table + i)->len)) != EOK)
		return;

	min_threshold_num = (als_para_diff_tp_color_table + i)->len - 1;
	/* 2: second to last */
	max_threshold_num = (als_para_diff_tp_color_table + i)->len - 2;
	dev_info->min_thres =
		(als_para_diff_tp_color_table + i)->als_para[min_threshold_num];
	dev_info->max_thres =
		(als_para_diff_tp_color_table + i)->als_para[max_threshold_num];
}

void set_tsl2591_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture();
	int para_table = 0;
	unsigned int i;
	tsl2591_als_para_table *tsl2591_tab = als_get_tsl2591_first_table();
	uint32_t table_size = als_get_tsl2591_table_count();

	for (i = 0; i < table_size; i++) {
		if ((tsl2591_tab[i].phone_type == als_data.als_phone_type) &&
			(tsl2591_tab[i].phone_version == als_data.als_phone_version) &&
			((tsl2591_tab[i].tp_manufacture == tplcd_manufacture) ||
			(tsl2591_tab[i].tp_manufacture == TS_PANEL_UNKNOWN))) {
			para_table = i;
			break;
		}
	}

	dev_info->table_id = para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		tsl2591_tab[para_table].tsl2591_para,
		(sizeof(tsl2591_tab[para_table].tsl2591_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(tsl2591_tab[para_table].tsl2591_para)) != EOK)
		return;

	dev_info->min_thres = tsl2591_tab[para_table].tsl2591_para[TSL2591_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tsl2591_tab[para_table].tsl2591_para[TSL2591_MAX_THRESHOLD_NUM];
}

void set_bh1726_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture();
	int para_table = 0;
	unsigned int i;
	bh1726_als_para_table *bh1726_tab = als_get_bh1726_first_table();
	uint32_t table_size = als_get_bh1726_table_count();

	for (i = 0; i < table_size; i++) {
		if ((bh1726_tab[i].phone_type == als_data.als_phone_type) &&
			(bh1726_tab[i].phone_version == als_data.als_phone_version) &&
			((bh1726_tab[i].tp_manufacture == tplcd_manufacture) ||
			(bh1726_tab[i].tp_manufacture == TS_PANEL_UNKNOWN))) {
			para_table = i;
			break;
		}
	}

	dev_info->table_id = para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		bh1726_tab[para_table].bh1726_para,
		(sizeof(bh1726_tab[para_table].bh1726_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(bh1726_tab[para_table].bh1726_para)) != EOK)
		return;

	dev_info->min_thres = bh1726_tab[para_table].bh1726_para[BH1726_MIN_THRESHOLD_NUM];
	dev_info->max_thres = bh1726_tab[para_table].bh1726_para[BH1726_MAX_THRESHOLD_NUM];
}

/* set als parameters */
void set_apds9308_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture();
	int para_table = 0;
	unsigned int i;
	unsigned int len;
	apds9308_als_para_table *apds9308_tab = als_get_apds9308_first_table();
	uint32_t table_size = als_get_apds9308_table_count();

	for (i = 0; i < table_size; i++) {
		if ((apds9308_tab[i].phone_type == als_data.als_phone_type) &&
			(apds9308_tab[i].phone_version ==
			als_data.als_phone_version) &&
			((apds9308_tab[i].tp_manufacture ==
			tplcd_manufacture) ||
			(apds9308_tab[i].tp_manufacture ==
			TS_PANEL_UNKNOWN))) {
			para_table = i;
			break;
		}
	}
	dev_info->table_id = para_table;
	if (sizeof(apds9308_tab[para_table].apds9308_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE)
		len = SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE;
	else
		len = sizeof(apds9308_tab[para_table].apds9308_para);
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		apds9308_tab[para_table].apds9308_para, len) != EOK)
		return;

	dev_info->min_thres = apds9308_tab[para_table].apds9308_para[APDS9308_MIN_THD_NUM];
	dev_info->max_thres = apds9308_tab[para_table].apds9308_para[APDS9308_MAX_THD_NUM];
}

void set_rpr531_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	int para_table = 0;
	unsigned int i;
	rpr531_als_para_table *rpr531_tab = als_get_rpr531_first_table();
	uint32_t table_size = als_get_rpr531_table_count();

	for (i = 0; i < table_size; i++) {
		if ((rpr531_tab[i].phone_type == als_data.als_phone_type) &&
			(rpr531_tab[i].phone_version == als_data.als_phone_version) &&
			((rpr531_tab[i].tp_manufacture == tp_manufacture) ||
			(rpr531_tab[i].tp_manufacture == TS_PANEL_UNKNOWN))) {
			para_table = i;
			break;
		}
	}
	dev_info->table_id = para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		rpr531_tab[para_table].rpr531_para,
		(sizeof(rpr531_tab[para_table].rpr531_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(rpr531_tab[para_table].rpr531_para)) != EOK)
		return;

	dev_info->min_thres = rpr531_tab[para_table].rpr531_para[RPR531_MIN_THRESHOLD_NUM];
	dev_info->max_thres = rpr531_tab[para_table].rpr531_para[RPR531_MAX_THRESHOLD_NUM];

	hwlog_info("rpr531_als_para_tabel= %d\n", para_table);
}

static void set_pinhole_als_extend_parameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	int als_para_table = 0;
	int i;
	pinhole_als_para_table *pinhole_tab = als_get_pinhole_first_table();
	uint32_t table_size = als_get_pinhole_table_count();

	for (i = 0; i < table_size; i++) {
		if ((pinhole_tab[i].phone_type == als_data.als_phone_type) &&
			(pinhole_tab[i].phone_version == als_data.als_phone_version) &&
			((dev_info->chip_type == ALS_CHIP_APDS9922 &&
				pinhole_tab[i].sens_name == APDS9922) ||
			(dev_info->chip_type == ALS_CHIP_LTR578 &&
				pinhole_tab[i].sens_name == LTR578)) &&
			((pinhole_tab[i].tp_manufacture == tp_manufacture) ||
			(pinhole_tab[i].tp_manufacture == TS_PANEL_UNKNOWN))) {
			als_para_table = i;
			break;
		}
	}
	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		pinhole_tab[als_para_table].pinhole_para,
		(sizeof(pinhole_tab[als_para_table].pinhole_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(pinhole_tab[als_para_table].pinhole_para)) != EOK)
		return;

	dev_info->min_thres = pinhole_tab[als_para_table].pinhole_para[LTR578_APDS9922_MIN_THRESHOLD_NUM];
	dev_info->max_thres = pinhole_tab[als_para_table].pinhole_para[LTR578_APDS9922_MAX_THRESHOLD_NUM];
	hwlog_info("als_para_tab=%d phone_type=%d,phone_ver=%d\n",
		als_para_table, als_data.als_phone_type,
		als_data.als_phone_version);
}

static void set_tmd2745_als_extend_parameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	int als_para_table = 0;
	unsigned int i;
	tmd2745_als_para_table *tmd2745_tab = als_get_tmd2745_first_table();
	uint32_t table_size = als_get_tmd2745_table_count();

	for (i = 0; i < table_size; i++) {
		if ((tmd2745_tab[i].phone_type == als_data.als_phone_type) &&
			(tmd2745_tab[i].phone_version == als_data.als_phone_version) &&
			(tmd2745_tab[i].tp_manufacture == tp_manufacture)) {
			als_para_table = i;
			break;
		}
	}

	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		tmd2745_tab[als_para_table].als_para,
		(sizeof(tmd2745_tab[als_para_table].als_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(tmd2745_tab[als_para_table].als_para)) != EOK)
		return;

	dev_info->min_thres = tmd2745_tab[als_para_table].als_para[TMD2745_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tmd2745_tab[als_para_table].als_para[TMD2745_MAX_THRESHOLD_NUM];

	hwlog_err("als_para_table=%d, phone_type=%d, phone_version=%d\n",
		als_para_table, als_data.als_phone_type, als_data.als_phone_version);
}

static void set_stk33562_als_extend_parameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	int als_para_table = 0;
	unsigned int i;
	stk33562_als_para_table *stk33562_tab = als_get_stk33562_first_table();
	uint32_t table_size = als_get_stk33562_table_count();

	for (i = 0; i < table_size; i++) {
		if ((stk33562_tab[i].phone_type == als_data.als_phone_type) &&
			(stk33562_tab[i].phone_version == als_data.als_phone_version) &&
			(stk33562_tab[i].tp_manufacture == get_tplcd_manufacture_curr())) {
			als_para_table = i;
			break;
		}
	}

	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		stk33562_tab[als_para_table].als_para,
		(sizeof(stk33562_tab[als_para_table].als_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(stk33562_tab[als_para_table].als_para)) != EOK)
		return;

	// dev_info->min_thres = stk33562_tab[als_para_table].als_para[stk33562_MIN_THRESHOLD_IDX];
	// dev_info->max_thres = stk33562_tab[als_para_table].als_para[stk33562_MAX_THRESHOLD_IDX];

	hwlog_err("stk33562:als_para_table=%d, phone_type=%d, phone_version=%d\n",
		als_para_table, als_data.als_phone_type, als_data.als_phone_version);
}

static void set_mn78911_als_extend_parameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	int als_para_table = 0;
	unsigned int i;
	mn78911_als_para_table *mn78911_tab = als_get_mn78911_first_table();
	uint32_t table_size = als_get_mn78911_table_count();

	for (i = 0; i < table_size; i++) {
		if ((mn78911_tab[i].phone_type == als_data.als_phone_type) &&
			(mn78911_tab[i].phone_version == als_data.als_phone_version) &&
			(mn78911_tab[i].tp_manufacture == get_tplcd_manufacture_curr())) {
			als_para_table = i;
			break;
		}
	}

	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		mn78911_tab[als_para_table].als_para,
		(sizeof(mn78911_tab[als_para_table].als_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(mn78911_tab[als_para_table].als_para)) != EOK)
		return;

	// dev_info->min_thres = mn78911_tab[als_para_table].als_para[mn78911_MIN_THRESHOLD_IDX];
	// dev_info->max_thres = mn78911_tab[als_para_table].als_para[mn78911_MAX_THRESHOLD_IDX];

	hwlog_err("mn78911:als_para_table=%d, phone_type=%d, phone_version=%d\n",
		als_para_table, als_data.als_phone_type, als_data.als_phone_version);
}

static void set_mn78911_ps_parameters(struct ps_platform_data *ps_data,
	struct ps_device_info *dev_info)
{
	int ps_para_table = 0;
	unsigned int i;
	mn78911_ps_para_table *mn78911_tab = ps_get_mn78911_first_table();
	uint32_t table_size = ps_get_mn78911_table_count();
	
	if (ps_data == NULL) {
	    return;
	}
	
	for (i = 0; i < table_size; i++) {
		if ((mn78911_tab[i].phone_type == ps_data->ps_phone_type) &&
			(mn78911_tab[i].phone_version == ps_data->ps_phone_version) &&
			(mn78911_tab[i].tp_manufacture == get_tplcd_manufacture_curr())) {
			ps_para_table = i;
			break;
		}
	}

	if (memcpy_s(ps_data->ps_extend_data, sizeof(ps_data->ps_extend_data),
		mn78911_tab[ps_para_table].ps_para,
		(sizeof(mn78911_tab[ps_para_table].ps_para) >
		SENSOR_PLATFORM_EXTEND_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_DATA_SIZE :
		sizeof(mn78911_tab[ps_para_table].ps_para)) != EOK)
		return;


	hwlog_err("mn78911:ps_para_table=%d, phone_type=%d, phone_version=%d\n",
		ps_para_table, ps_data->ps_phone_type, ps_data->ps_phone_version);
}

static void set_tmd2755_als_extend_parameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	int als_para_table = 0;
	unsigned int i;
	tmd2755_als_para_table *tmd2755_tab = als_get_tmd2755_first_table();
	uint32_t table_size = als_get_tmd2755_table_count();

	for (i = 0; i < table_size; i++) {
		if ((tmd2755_tab[i].phone_type == als_data.als_phone_type) &&
			(tmd2755_tab[i].phone_version == als_data.als_phone_version) &&
			(tmd2755_tab[i].tp_manufacture == get_tplcd_manufacture_curr())) {
			als_para_table = i;
			break;
		}
	}

	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		tmd2755_tab[als_para_table].als_para,
		(sizeof(tmd2755_tab[als_para_table].als_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(tmd2755_tab[als_para_table].als_para)) != EOK)
		return;

	hwlog_err("tmd2755:als_para_table=%d, phone_type=%d, phone_version=%d\n",
		als_para_table, als_data.als_phone_type, als_data.als_phone_version);
}

static void set_tmd2755_ps_parameters(struct ps_platform_data *ps_data,
	struct ps_device_info *dev_info)
{
	int ps_para_table = 0;
	unsigned int i;
	tmd2755_ps_para_table *tmd2755_tab = ps_get_tmd2755_first_table();
	uint32_t table_size = ps_get_tmd2755_table_count();
	
	if (ps_data == NULL) {
		return;
	}
	
	for (i = 0; i < table_size; i++) {
		if ((tmd2755_tab[i].phone_type == ps_data->ps_phone_type) &&
			(tmd2755_tab[i].phone_version == ps_data->ps_phone_version) &&
			(tmd2755_tab[i].tp_manufacture == get_tplcd_manufacture_curr())) {
			ps_para_table = i;
			break;
		}
	}

	if (memcpy_s(ps_data->ps_extend_data, sizeof(ps_data->ps_extend_data),
		tmd2755_tab[ps_para_table].ps_para,
		(sizeof(tmd2755_tab[ps_para_table].ps_para) >
		SENSOR_PLATFORM_EXTEND_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_DATA_SIZE :
		sizeof(tmd2755_tab[ps_para_table].ps_para)) != EOK)
		return;

	hwlog_err("tmd2755:ps_para_table=%d, phone_type=%d, phone_version=%d\n",
		ps_para_table, ps_data->ps_phone_type, ps_data->ps_phone_version);
}

static void set_stk33562_ps_parameters(struct ps_platform_data *ps_data,
	struct ps_device_info *dev_info)
{
	int ps_para_table = 0;
	unsigned int i;
	stk33562_ps_para_table *stk33562_tab = ps_get_stk33562_first_table();
	uint32_t table_size = ps_get_stk33562_table_count();

	if (ps_data == NULL) {
	    return;
	}

	for (i = 0; i < table_size; i++) {
		if ((stk33562_tab[i].phone_type == ps_data->ps_phone_type) &&
			(stk33562_tab[i].phone_version == ps_data->ps_phone_version) &&
			(stk33562_tab[i].tp_manufacture == get_tplcd_manufacture_curr())) {
			ps_para_table = i;
			break;
		}
	}

	if (memcpy_s(ps_data->ps_extend_data, sizeof(ps_data->ps_extend_data),
		stk33562_tab[ps_para_table].ps_para,
		(sizeof(stk33562_tab[ps_para_table].ps_para) >
		SENSOR_PLATFORM_EXTEND_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_DATA_SIZE :
		sizeof(stk33562_tab[ps_para_table].ps_para)) != EOK)
		return;

	hwlog_err("stk33562:ps_para_table=%d, phone_type=%d, phone_version=%d\n",
		ps_para_table, ps_data->ps_phone_type, ps_data->ps_phone_version);
}


static void set_ltr578_als_extend_parameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	int als_para_table = 0;
	int i;
	ltr578_als_para_table *ltr578_tab = als_get_ltr578_first_table();
	uint32_t table_size = als_get_ltr578_table_count();

	for (i = 0; i < table_size; i++) {
		if ((ltr578_tab[i].phone_type == als_data.als_phone_type) &&
			(ltr578_tab[i].phone_version ==
			als_data.als_phone_version) &&
			(ltr578_tab[i].sens_name == LTR578) &&
			((ltr578_tab[i].tp_manufacture == tp_manufacture) ||
			(ltr578_tab[i].tp_manufacture == TS_PANEL_UNKNOWN)) &&
			(ltr578_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}

	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		ltr578_tab[als_para_table].para,
		(sizeof(ltr578_tab[als_para_table].para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(ltr578_tab[als_para_table].para)) != EOK) {
		hwlog_err("ltr578 memcpy als_extend_data error\n");
		return;
	}

	dev_info->min_thres =
		ltr578_tab[als_para_table].para[LTR578_MIN_THD_NUM];
	dev_info->max_thres =
		ltr578_tab[als_para_table].para[LTR578_MAX_THD_NUM];
	hwlog_info("phone_type=%d, version=%d\n",
		als_data.als_phone_type, als_data.als_phone_version);
	hwlog_info("table_index=%d, tp_color=%d, tp_manufacture=%d\n",
		als_para_table, phone_color, tp_manufacture);
}

static void set_stk3321_als_extend_parameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	int als_para_table = 0;
	int i;
	stk3321_als_para_table *stk3321_tab = als_get_stk3321_first_table();
	uint32_t table_size = als_get_stk3321_table_count();

	for (i = 0; i < table_size; i++) {
		if ((stk3321_tab[i].phone_type == als_data.als_phone_type) &&
			(stk3321_tab[i].phone_version ==
			als_data.als_phone_version) &&
			(stk3321_tab[i].sens_name == STK3321) &&
			((stk3321_tab[i].tp_manufacture == tp_manufacture) ||
			(stk3321_tab[i].tp_manufacture == TS_PANEL_UNKNOWN)) &&
			(stk3321_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}

	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		stk3321_tab[als_para_table].para,
		(sizeof(stk3321_tab[als_para_table].para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(stk3321_tab[als_para_table].para)) != EOK) {
		hwlog_err("stk3321 memcpy als_extend_data error\n");
		return;
	}

	dev_info->min_thres =
		stk3321_tab[als_para_table].para[STK3321_MIN_THD_NUM];
	dev_info->max_thres =
		stk3321_tab[als_para_table].para[STK3321_MAX_THD_NUM];
	hwlog_info("phone_type=%d, version=%d\n",
		als_data.als_phone_type, als_data.als_phone_version);
	hwlog_info("table_index=%d, tp_color=%d, tp_manufacture=%d\n",
		als_para_table, phone_color, tp_manufacture);
}

static void set_stk3235_als_extend_parameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	int als_para_table = 0;
	int i;
	stk3235_als_para_table *stk3235_tab = als_get_stk3235_first_table();
	uint32_t table_size = als_get_stk3235_table_count();

	for (i = 0; i < table_size; i++) {
		if ((stk3235_tab[i].phone_type ==
			als_data.als_phone_type) &&
			(stk3235_tab[i].phone_version ==
			als_data.als_phone_version) &&
			(stk3235_tab[i].sens_name == STK3235) &&
			((stk3235_tab[i].tp_manufacture == tp_manufacture) ||
			(stk3235_tab[i].tp_manufacture == TS_PANEL_UNKNOWN)) &&
			(stk3235_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}

	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		stk3235_tab[als_para_table].para,
		(sizeof(stk3235_tab[als_para_table].para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(stk3235_tab[als_para_table].para)) != EOK) {
		hwlog_err("stk3235 memcpy als_extend_data error\n");
		return;
	}

	dev_info->min_thres =
		stk3235_tab[als_para_table].para[STK3235_MIN_THD_NUM];
	dev_info->max_thres =
		stk3235_tab[als_para_table].para[STK3235_MAX_THD_NUM];
	hwlog_info("phone_type=%d, version=%d\n",
		als_data.als_phone_type, als_data.als_phone_version);
	hwlog_info("table_index=%d, tp_color=%d, tp_manufacture=%d\n",
		als_para_table, phone_color, tp_manufacture);
}

static void set_sy3079_als_extend_parameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	int als_para_table = 0;
	int i;
	sy3079_als_para_table *sy3079_tab = als_get_sy3079_first_table();
	uint32_t table_size = als_get_sy3079_table_count();

	for (i = 0; i < table_size; i++) {
		if ((sy3079_tab[i].phone_type == als_data.als_phone_type) &&
			(sy3079_tab[i].phone_version ==
			als_data.als_phone_version) &&
			(sy3079_tab[i].sens_name == SY3079) &&
			((sy3079_tab[i].tp_manufacture == tp_manufacture) ||
			(sy3079_tab[i].tp_manufacture == TS_PANEL_UNKNOWN)) &&
			(sy3079_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}

	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		sy3079_tab[als_para_table].para,
		(sizeof(sy3079_tab[als_para_table].para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(sy3079_tab[als_para_table].para)) != EOK) {
		hwlog_err("sy3079 memcpy als_extend_data error\n");
		return;
	}

	dev_info->min_thres =
		sy3079_tab[als_para_table].para[SY3079_MIN_THD_NUM];
	dev_info->max_thres =
		sy3079_tab[als_para_table].para[SY3079_MAX_THD_NUM];
	hwlog_info("phone_type=%d, version=%d\n",
		als_data.als_phone_type, als_data.als_phone_version);
	hwlog_info("table_index=%d, tp_color=%d, tp_manufacture=%d\n",
		als_para_table, phone_color, tp_manufacture);
}

static int read_tp_module_notify(struct notifier_block *nb,
	unsigned long action, void *data)
{
	struct als_platform_data *pf_data = NULL;
	struct als_device_info *dev_info = NULL;

	pf_data = als_get_platform_data(TAG_ALS);
	dev_info = als_get_device_info(TAG_ALS);
	if (!pf_data || !dev_info)
		return NOTIFY_OK;

	hwlog_info("%s, start\n", __func__);
	if (action == TS_PANEL_UNKNOWN)
		return NOTIFY_OK;
	mutex_lock(&mutex_set_para);
	tp_manufacture = action;
	if (set_pinhole_para_flag == 1) {
		set_pinhole_als_extend_parameters(pf_data, dev_info);
	} else if (set_rpr531_para_flag == true) {
		set_rpr531_als_extend_prameters(pf_data, dev_info);
	} else if (set_tmd2745_para_flag == true) {
		set_tmd2745_als_extend_parameters(pf_data, dev_info);
	} else {
		hwlog_info("%s, not get sensor yet\n", __func__);
		mutex_unlock(&mutex_set_para);
		return NOTIFY_OK;
	}
	mutex_unlock(&mutex_set_para);
	hwlog_info("%s, get tp module type = %d\n", __func__, tp_manufacture);
	if (dev_info->send_para_flag == 1)
		resend_als_parameters_to_mcu();
	return NOTIFY_OK;
}

static void select_rohm_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture();
	int als_para_table = 0;
	int i;
	bh1745_als_para_table *bh1745_tab = als_get_bh1745_first_table();
	uint32_t table_size = als_get_bh1745_table_count();

	for (i = 0; i < table_size; i++) {
		if ((bh1745_tab[i].phone_type == als_data.als_phone_type) &&
			(bh1745_tab[i].phone_version ==
			als_data.als_phone_version) &&
			(bh1745_tab[i].tp_lcd_manufacture ==
			tplcd_manufacture ||
			bh1745_tab[i].tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(bh1745_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}
	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		bh1745_tab[als_para_table].bh745_para,
		(sizeof(bh1745_tab[als_para_table].bh745_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(bh1745_tab[als_para_table].bh745_para)) != EOK)
		return;

	dev_info->min_thres = bh1745_tab[als_para_table].bh745_para[BH1745_MIN_THRESHOLD_NUM];
	dev_info->max_thres = bh1745_tab[als_para_table].bh745_para[BH1745_MAX_THRESHOLD_NUM];

	hwlog_info("als_para_tabel=%d,bh1745 phone_color=0x%x,tplcd_manufacture=%d,phone_type=%d,phone_ver=%d\n",
		als_para_table, phone_color, tplcd_manufacture,
		als_data.als_phone_type, als_data.als_phone_version);
}

static void select_sy3133_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture_curr();
	int als_para_table = 0;
	int i;
	sy3133_als_para_table *sy3133_tab = als_get_sy3133_first_table();
	uint32_t table_size = als_get_sy3133_table_count();

	for (i = 0; i < table_size; i++) {
		if ((sy3133_tab[i].phone_type == als_data.als_phone_type) &&
			(sy3133_tab[i].phone_version == als_data.als_phone_version) &&
			(sy3133_tab[i].tp_lcd_manufacture == tplcd_manufacture) &&
			(sy3133_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}
	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		sy3133_tab[als_para_table].sy3133_para,
		(sizeof(sy3133_tab[als_para_table].sy3133_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(sy3133_tab[als_para_table].sy3133_para)) != EOK)
		return;

	dev_info->min_thres = sy3133_tab[als_para_table].sy3133_para[SY3133_MIN_THD_NUM];
	dev_info->max_thres = sy3133_tab[als_para_table].sy3133_para[SY3133_MAX_THD_NUM];

	hwlog_info("als_para_tabel=%d,tcs3707 phone_color=0x%x,tplcd_manufacture=%d,phone_type=%d,phone_ver=%d\n",
		als_para_table, phone_color, tplcd_manufacture,
		als_data.als_phone_type, als_data.als_phone_version);
}

static void select_bu27006_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture();
	int als_para_table = 0;
	int i;
	bu27006_als_para_table *bu27006_tab = als_get_bu27006_first_table();
	uint32_t table_size = als_get_bu27006_table_count();

	for (i = 0; i < table_size; i++) {
		if ((bu27006_tab[i].phone_type == als_data.als_phone_type) &&
			(bu27006_tab[i].phone_version == als_data.als_phone_version) &&
			(bu27006_tab[i].tp_lcd_manufacture == tplcd_manufacture ||
			bu27006_tab[i].tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(bu27006_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}
	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		bu27006_tab[als_para_table].bu27006_para,
		(sizeof(bu27006_tab[als_para_table].bu27006_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(bu27006_tab[als_para_table].bu27006_para)) != EOK)
		return;

	dev_info->min_thres = bu27006_tab[als_para_table].bu27006_para[BU27006_MIN_THRESHOLD_NUM];
	dev_info->max_thres = bu27006_tab[als_para_table].bu27006_para[BU27006_MAX_THRESHOLD_NUM];

	hwlog_info("als_para_tabel=%d,bu27006 phone_color=0x%x,tplcd_manufacture=%d,phone_type=%d,phone_ver=%d\n",
		als_para_table, phone_color, tplcd_manufacture,
		als_data.als_phone_type, als_data.als_phone_version);
}

static void select_tcs3707_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture_curr();
	int als_para_table = 0;
	int i;
	tcs3707_als_para_table *tcs3707_tab = als_get_tcs3707_first_table();
	uint32_t table_size = als_get_tcs3707_table_count();

	for (i = 0; i < table_size; i++) {
		if ((tcs3707_tab[i].phone_type == als_data.als_phone_type) &&
			(tcs3707_tab[i].phone_version == als_data.als_phone_version) &&
			(tcs3707_tab[i].tp_lcd_manufacture == tplcd_manufacture) &&
			(tcs3707_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}
	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		tcs3707_tab[als_para_table].tcs3707_para,
		(sizeof(tcs3707_tab[als_para_table].tcs3707_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(tcs3707_tab[als_para_table].tcs3707_para)) != EOK)
		return;

	dev_info->min_thres = tcs3707_tab[als_para_table].tcs3707_para[TCS3707_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tcs3707_tab[als_para_table].tcs3707_para[TCS3707_MAX_THRESHOLD_NUM];

	hwlog_info("als_para_tabel=%d,tcs3707 phone_color=0x%x,tplcd_manufacture=%d,phone_type=%d,phone_ver=%d\n",
		als_para_table, phone_color, tplcd_manufacture,
		als_data.als_phone_type, als_data.als_phone_version);
}

static void select_avago_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture();
	int als_para_table = 0;
	int i;
	apds9251_als_para_table *apds9251_tab = als_get_apds9251_first_table();
	uint32_t table_size = als_get_apds9251_table_count();

	for (i = 0; i < table_size; i++) {
		if ((apds9251_tab[i].phone_type == als_data.als_phone_type) &&
			(apds9251_tab[i].phone_version ==
			als_data.als_phone_version) &&
			(apds9251_tab[i].tp_lcd_manufacture ==
			tplcd_manufacture ||
			apds9251_tab[i].tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(apds9251_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}
	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		apds9251_tab[als_para_table].apds251_para,
		(sizeof(apds9251_tab[als_para_table].apds251_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(apds9251_tab[als_para_table].apds251_para)) != EOK)
		return;

	dev_info->min_thres = apds9251_tab[als_para_table].apds251_para[APDS9251_MIN_THRESHOLD_NUM];
	dev_info->max_thres = apds9251_tab[als_para_table].apds251_para[APDS9251_MAX_THRESHOLD_NUM];
	hwlog_info("als_para_tabel=%d apds9251 phone_color=0x%x phone_type=%d,phone_version=%d\n",
		als_para_table, phone_color, als_data.als_phone_type,
		als_data.als_phone_version);
}

static void select_apds9999_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture();
	int als_para_table = 0;
	int i;
	apds9999_als_para_table *apds9999_tab =
		als_get_apds9999_first_table();
	uint32_t table_size = als_get_apds9999_table_count();

	for (i = 0; i < table_size; i++) {
		if ((apds9999_tab[i].phone_type == als_data.als_phone_type) &&
			(apds9999_tab[i].phone_version == als_data.als_phone_version) &&
			(apds9999_tab[i].tp_lcd_manufacture == tplcd_manufacture ||
			apds9999_tab[i].tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(apds9999_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}
	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		apds9999_tab[als_para_table].apds9999_para,
		(sizeof(apds9999_tab[als_para_table].apds9999_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(apds9999_tab[als_para_table].apds9999_para)) != EOK)
		return;

	dev_info->min_thres = apds9999_tab[als_para_table].apds9999_para[APDS9251_MIN_THRESHOLD_NUM];
	dev_info->max_thres = apds9999_tab[als_para_table].apds9999_para[APDS9251_MAX_THRESHOLD_NUM];
	hwlog_info("als_para_tabel=%d apds9251 phone_color=0x%x phone_type=%d,phone_version=%d\n",
		als_para_table, phone_color, als_data.als_phone_type, als_data.als_phone_version);
}


static void select_ams_tmd3725_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture();
	int als_para_table = 0;
	int i;
	tmd3725_als_para_table *tmd3725_tab = als_get_tmd3725_first_table();
	uint32_t table_size = als_get_tmd3725_table_count();

	for (i = 0; i < table_size; i++) {
		if ((tmd3725_tab[i].phone_type == als_data.als_phone_type) &&
			(tmd3725_tab[i].phone_version == als_data.als_phone_version) &&
			(tmd3725_tab[i].tp_lcd_manufacture == tplcd_manufacture ||
			tmd3725_tab[i].tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(tmd3725_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}
	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		tmd3725_tab[als_para_table].tmd3725_para,
		(sizeof(tmd3725_tab[als_para_table].tmd3725_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(tmd3725_tab[als_para_table].tmd3725_para)) != EOK)
		return;

	dev_info->min_thres = tmd3725_tab[als_para_table].tmd3725_para[TMD3725_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tmd3725_tab[als_para_table].tmd3725_para[TMD3725_MAX_THRESHOLD_NUM];
	hwlog_info("als_para_tabel=%d tmd3725 phone_color=0x%x phone_type=%d,phone_version=%d\n",
		als_para_table, phone_color, als_data.als_phone_type, als_data.als_phone_version);
}

static void select_ams_tmd3702_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture();
	int als_para_table = 0;
	int i;
	tmd3702_als_para_table *tmd3702_tab = als_get_tmd3702_first_table();
	uint32_t table_size = als_get_tmd3702_table_count();

	for (i = 0; i < table_size; i++) {
		if ((tmd3702_tab[i].phone_type == als_data.als_phone_type) &&
			(tmd3702_tab[i].phone_version == als_data.als_phone_version) &&
			(tmd3702_tab[i].tp_lcd_manufacture == tplcd_manufacture ||
			tmd3702_tab[i].tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(tmd3702_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}
	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		tmd3702_tab[als_para_table].tmd3702_para,
		(sizeof(tmd3702_tab[als_para_table].tmd3702_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(tmd3702_tab[als_para_table].tmd3702_para)) != EOK)
		return;

	dev_info->min_thres = tmd3702_tab[als_para_table].tmd3702_para[TMD3702_MIN_THRESHOLD_NUM];
	dev_info->max_thres = tmd3702_tab[als_para_table].tmd3702_para[TMD3702_MAX_THRESHOLD_NUM];
	hwlog_info("als_para_tabel=%d tmd3702 phone_color=0x%x phone_type=%d,phone_version=%d\n",
		als_para_table, phone_color, als_data.als_phone_type, als_data.als_phone_version);
}

static void select_vishay_vcnl36658_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture();
	int als_para_table = 0;
	int i;
	vcnl36658_als_para_table *vcnl36658_tab =
		als_get_vcnl36658_first_table();
	uint32_t table_size = als_get_vcnl36658_table_count();

	for (i = 0; i < table_size; i++) {
		if ((vcnl36658_tab[i].phone_type == als_data.als_phone_type) &&
			(vcnl36658_tab[i].phone_version == als_data.als_phone_version) &&
			(vcnl36658_tab[i].tp_lcd_manufacture == tplcd_manufacture ||
			vcnl36658_tab[i].tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(vcnl36658_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}
	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		vcnl36658_tab[als_para_table].vcnl36658_para,
		(sizeof(vcnl36658_tab[als_para_table].vcnl36658_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(vcnl36658_tab[als_para_table].vcnl36658_para)) != EOK)
		return;

	dev_info->min_thres = vcnl36658_tab[als_para_table].vcnl36658_para[VCNL36658_MIN_THRESHOLD_NUM];
	dev_info->max_thres = vcnl36658_tab[als_para_table].vcnl36658_para[VCNL36658_MAX_THRESHOLD_NUM];
	hwlog_info("als_para_tabel=%d vcnl36658 phone_color=0x%x phone_type=%d,phone_version=%d\n",
		als_para_table, phone_color, als_data.als_phone_type, als_data.als_phone_version);
}

static void select_liteon_ltr582_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	u8 tplcd_manufacture = get_tplcd_manufacture();
	int als_para_table = 0;
	int i;
	ltr582_als_para_table *ltr582_tab = als_get_ltr582_first_table();
	uint32_t table_size = als_get_ltr582_table_count();

	for (i = 0; i < table_size; i++) {
		if ((ltr582_tab[i].phone_type == als_data.als_phone_type) &&
			(ltr582_tab[i].phone_version == als_data.als_phone_version) &&
			(ltr582_tab[i].tp_lcd_manufacture == tplcd_manufacture ||
			ltr582_tab[i].tp_lcd_manufacture == DEFAULT_TPLCD) &&
			(ltr582_tab[i].tp_color == phone_color)) {
			als_para_table = i;
			break;
		}
	}
	dev_info->table_id = als_para_table;
	if (memcpy_s(als_data.als_extend_data, sizeof(als_data.als_extend_data),
		ltr582_tab[als_para_table].ltr582_para,
		(sizeof(ltr582_tab[als_para_table].ltr582_para) >
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE) ?
		SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE :
		sizeof(ltr582_tab[als_para_table].ltr582_para)) != EOK)
		return;

	dev_info->min_thres = ltr582_tab[als_para_table].ltr582_para[LTR582_MIN_THRESHOLD_NUM];
	dev_info->max_thres = ltr582_tab[als_para_table].ltr582_para[LTR582_MAX_THRESHOLD_NUM];

	hwlog_info("als_para_tabel=%d ltr582 phone_color=0x%x phone_type=%d,phone_version=%d\n",
		als_para_table, phone_color, als_data.als_phone_type, als_data.als_phone_version);
}

static int read_tpcolor_from_nv(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	int ret;
	char nv_tp_color[TP_COLOR_NV_SIZE + 1] = "";
	struct hisi_nve_info_user user_info = { 0 };

	if (!pf_data || !dev_info)
		return -1;

	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = TP_COLOR_NV_NUM;
	user_info.valid_size = TP_COLOR_NV_SIZE;
	strncpy(user_info.nv_name, "TPCOLOR", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access read error %d\n", ret);
		return -EINVAL;
	}
	memcpy(nv_tp_color, user_info.nv_data, sizeof(nv_tp_color) - 1);

	if (strstr(nv_tp_color, "white"))
		phone_color = WHITE;
	else if (strstr(nv_tp_color, "black"))
		phone_color = BLACK;
	else
		hwlog_info("other colors\n");

	hwlog_info("phone_color = 0x%x, nv_tp_color = %s,als_phone_type=%d\n",
		phone_color, nv_tp_color, als_data.als_phone_type);
	als_data.als_phone_tp_colour = phone_color;

	/* sensor flag: 1 sensor detect, 0 sensor not detect */
	if (dev_info->chip_type == ALS_CHIP_SY3079)
		set_sy3079_als_extend_parameters(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_STK3235)
		set_stk3235_als_extend_parameters(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_STK3321)
		set_stk3321_als_extend_parameters(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_LTR578)
		set_ltr578_als_extend_parameters(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_TCS3707)
		select_tcs3707_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_SY3133)
		select_sy3133_als_data(pf_data, dev_info);
	return 0;
}

int get_tpcolor_from_nv(void)
{
	int ret;
	struct als_platform_data *pf_data = NULL;
	struct als_device_info *dev_info = NULL;

	pf_data = als_get_platform_data(TAG_ALS);
	dev_info = als_get_device_info(TAG_ALS);
	if (!pf_data || !dev_info)
		return -1;

	if (dev_info->tp_color_from_nv_flag) {
		hwlog_info("%s: tp_color_from_nv_flag = %d\n",
			__func__, dev_info->tp_color_from_nv_flag);
		ret = read_tpcolor_from_nv(pf_data, dev_info);
		if (ret != 0) {
			hwlog_err("%s: read tp_color from NV fail\n",
				__func__);
			return -EINVAL;
		}
		resend_als_parameters_to_mcu();
	}

	return 0;
}

static int select_als_type1_para(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	struct device_node *dn)
{
	if (dev_info->chip_type == ALS_CHIP_ROHM_RGB) {
		select_rohm_als_data(pf_data, dev_info);
	} else if (dev_info->chip_type == ALS_CHIP_AVAGO_RGB ||
		dev_info->chip_type == ALS_CHIP_APDS9253_RGB) {
		select_avago_als_data(pf_data, dev_info);
	} else if (dev_info->chip_type == ALS_CHIP_APDS9999_RGB) {
		select_apds9999_als_data(pf_data, dev_info);
	} else if (dev_info->chip_type == ALS_CHIP_AMS_TMD3725_RGB) {
		select_ams_tmd3725_als_data(pf_data, dev_info);
	} else if (dev_info->chip_type == ALS_CHIP_AMS_TMD3702_RGB) {
		select_ams_tmd3702_als_data(pf_data, dev_info);
	} else if (dev_info->chip_type == ALS_CHIP_LITEON_LTR582) {
		select_liteon_ltr582_als_data(pf_data, dev_info);
	} else if (dev_info->chip_type == ALS_CHIP_VISHAY_VCNL36658) {
		select_vishay_vcnl36658_als_data(pf_data, dev_info);
	} else if ((dev_info->chip_type == ALS_CHIP_APDS9922) ||
		(dev_info->chip_type == ALS_CHIP_LTR578)) {
		mutex_lock(&mutex_set_para);
		if (als_data.als_phone_type == WAS)
			tp_manufacture = get_tplcd_manufacture();
		set_pinhole_als_extend_parameters(pf_data, dev_info);
		set_pinhole_para_flag = 1;
		mutex_unlock(&mutex_set_para);
	} else if (dev_info->chip_type == ALS_CHIP_RPR531) {
		mutex_lock(&mutex_set_para);
		set_rpr531_als_extend_prameters(pf_data, dev_info);
		set_rpr531_para_flag = true;
		mutex_unlock(&mutex_set_para);
	} else if (dev_info->chip_type == ALS_CHIP_TMD2745) {
		mutex_lock(&mutex_set_para);
		set_tmd2745_als_extend_parameters(pf_data, dev_info);
		set_tmd2745_para_flag = true;
		mutex_unlock(&mutex_set_para);
	}  else if (dev_info->chip_type == ALS_CHIP_TMD2755) {
		mutex_lock(&mutex_set_para);
		set_tmd2755_als_extend_parameters(pf_data, dev_info);
		mutex_unlock(&mutex_set_para);
	} else if (dev_info->chip_type == ALS_CHIP_MN78911) {
		mutex_lock(&mutex_set_para);
		set_mn78911_als_extend_parameters(pf_data, dev_info);
		mutex_unlock(&mutex_set_para);
	} else if (dev_info->chip_type == ALS_CHIP_STK33562) {
		mutex_lock(&mutex_set_para);
		set_stk33562_als_extend_parameters(pf_data, dev_info);
		mutex_unlock(&mutex_set_para);
	} else if (dev_info->chip_type == ALS_CHIP_BU27006) {
		select_bu27006_als_data(pf_data, dev_info);
	} else if (dev_info->chip_type == ALS_CHIP_TCS3707) {
		select_tcs3707_als_data(pf_data, dev_info);
	} else if (dev_info->chip_type == ALS_CHIP_SY3133) {
		select_sy3133_als_data(pf_data, dev_info);
	} else {
		return -1;
	}

	return 0;
}

static int select_als_type2_para(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	struct device_node *dn)
{
	if (dev_info->chip_type == ALS_CHIP_TSL2591) {
		set_tsl2591_als_extend_prameters(pf_data, dev_info);
	} else if (dev_info->chip_type == ALS_CHIP_BH1726) {
		set_bh1726_als_extend_prameters(pf_data, dev_info);
	} else if (dev_info->chip_type == ALS_CHIP_APDS9308) {
		set_apds9308_als_extend_prameters(pf_data, dev_info);
	} else if (dev_info->chip_type == ALS_CHIP_VISHAY_VCNL36832) {
		set_als_extend_prameters(pf_data, dev_info,
			als_get_vcnl36832_first_table(),
			als_get_vcnl36832_table_count());
	} else if (dev_info->chip_type == ALS_CHIP_STK3338) {
		set_als_extend_prameters(pf_data, dev_info,
			als_get_stk3338_first_table(),
			als_get_stk3338_table_count());
	} else if (dev_info->chip_type == ALS_CHIP_LTR2568) {
		set_als_extend_prameters(pf_data, dev_info,
			als_get_ltr2568_first_table(),
			als_get_ltr2568_table_count());
	} else if (dev_info->chip_type == ALS_CHIP_SYH399) {
		set_als_extend_prameters(pf_data, dev_info,
			als_get_syh399_first_table(),
			als_get_syh399_table_count());
	} else if (dev_info->chip_type == ALS_CHIP_LTR311) {
		set_als_extend_prameters(pf_data, dev_info,
			als_get_ltr311_first_table(),
			als_get_ltr311_table_count());
	} else {
		return -1;
	}

	return 0;
}

void select_als_para(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	struct device_node *dn)
{
	int ret;

	if ((!pf_data) || (!dev_info) || (!dn))
		return;

	if (!select_als_type1_para(pf_data, dev_info, dn)) {
	} else if (!select_als_type2_para(pf_data, dev_info, dn)) {
	} else {
		ret = fill_extend_data_in_dts(dn, "als_extend_data",
			als_data.als_extend_data, 12, /* max 12 half word */
			EXTEND_DATA_TYPE_IN_DTS_HALF_WORD);
		if (ret)
			hwlog_err("als_ext_data:fill_ext_data_in_dts err\n");
	}
}

void select_ps_para(struct sensor_detect_manager *sm)
{
	int ret;
	struct ps_device_info *dev_info = ps_get_device_info(TAG_PS);
	struct ps_platform_data *ps_data = NULL;
	if (!dev_info || !sm)
		return;

	ps_data = sm->spara;
	if (!ps_data)
		return;

	if (dev_info->chip_type == PS_CHIP_TMD2755) {
		set_tmd2755_ps_parameters(ps_data, dev_info);
	}
	if (dev_info->chip_type == PS_CHIP_MN78911) {
		set_mn78911_ps_parameters(ps_data, dev_info);
	}
	if (dev_info->chip_type == PS_CHIP_STK33562) {
		set_stk33562_ps_parameters(ps_data, dev_info);
	}
	hwlog_err("select_ps_para: Finished!!!\n");
}

#ifdef CONFIG_HW_TOUCH_KEY
int huawei_set_key_backlight(void *param_t)
{
	int ret;
	int key_brightness;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	pkt_parameter_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;
	struct key_param_t *param = (struct key_param_t *)param_t;

	if (!param || is_sensorhub_disabled()) {
		hwlog_err("param null or sensorhub is disabled\n");
		return 0;
	}

	if (strlen(sensor_chip_info[KEY]) == 0) {
		hwlog_err("no key\n");
		return 0;
	}
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	key_brightness = ((param->test_mode << 16) | param->brightness1 |
		(param->brightness2 << 8));

	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.tag = TAG_KEY;
	cpkt.subcmd = SUB_CMD_BACKLIGHT_REQ;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(key_brightness) + SUBCMD_LEN;
	memcpy(cpkt.para, &key_brightness, sizeof(key_brightness));
	if ((get_iom3_state() == IOM3_ST_RECOVERY) ||
		(get_iomcu_power_state() == ST_SLEEP))
		ret = write_customize_cmd(&pkg_ap, NULL, false);
	else
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);

	if (ret < 0) {
		hwlog_err("err. write cmd\n");
		return -1;
	}

	if (pkg_mcu.errno != 0) {
		hwlog_info("mcu err\n");
		return -1;
	}

	return 0;
}
#else
int huawei_set_key_backlight(void *param_t)
{
	return 0;
}
#endif

static int light_sensor_update_fastboot_info(void)
{
	char *pstr = NULL;
	char *dstr = NULL;
	char tmp[20];
	long tmp_code;
	int err;

	pstr = strstr(saved_command_line, "TP_COLOR=");
	if (!pstr) {
		pr_err("No fastboot TP_COLOR info\n");
		return -EINVAL;
	}
	pstr += strlen("TP_COLOR=");
	dstr = strstr(pstr, " ");
	if (!dstr) {
		pr_err("No find the TP_COLOR end\n");
		return -EINVAL;
	}
	memcpy(tmp, pstr, (unsigned long)(dstr - pstr));
	tmp[dstr - pstr] = '\0';
	err = kstrtol(tmp, TO_DECIMALISM, &tmp_code);
	if (err) {
		hwlog_info("%s: kstrtol failed, err: %d\n", __func__, err);
		return err;
	}
	return (int)tmp_code;
}

static int judge_tp_color_correct(u8 color)
{
	/*
	 * if the tp color is correct,
	 * after the 4~7 bit inversion shoud be same with 0~3 bit;
	 */
	return ((color & 0x0f) == ((~(color >> 4)) & 0x0f));
}

static struct notifier_block readtp_notify = {
	.notifier_call = read_tp_module_notify,
};

void read_tp_color_cmdline(void)
{
	int tp_color;

	tpmodule_register_client(&readtp_notify);
	tp_color = light_sensor_update_fastboot_info();
	if (judge_tp_color_correct(tp_color)) {
		phone_color = tp_color;
	} else {
		hwlog_err("light sensor LCD/TP ID error\n");
		phone_color = WHITE; /* WHITE is the default tp color */
	}
	hwlog_info("light sensor read tp color is %d, 0x%x\n", tp_color,
		phone_color);
}

static void get_current_work_func(struct work_struct *work);
DECLARE_DELAYED_WORK(read_current_work, get_current_work_func);

static void get_current_work_func(struct work_struct *work)
{
	int value;

	value = coul_drv_battery_current();

	/* send current to iom3 */
	if (send_func)
		(*send_func)(value);

	if (atomic_read(&enabled))
		queue_delayed_work(system_power_efficient_wq, &read_current_work,
			msecs_to_jiffies(READ_CURRENT_INTERVAL));
}

int open_send_current(int (*send)(int))
{
	if (!atomic_cmpxchg(&enabled, 0, 1)) {
		queue_delayed_work(system_power_efficient_wq, &read_current_work,
			msecs_to_jiffies(READ_CURRENT_INTERVAL));
		send_func = send;
	} else {
		hwlog_info("%s allready opend\n", __func__);
	}
	return 0;
}

int close_send_current(void)
{
	if (atomic_cmpxchg(&enabled, 1, 0)) {
		send_func = NULL;
		cancel_delayed_work_sync(&read_current_work);
	}
	return 0;
}
