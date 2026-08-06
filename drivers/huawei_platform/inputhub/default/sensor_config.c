/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: sensor config source file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#include "sensor_config.h"

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

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#ifdef CONFIG_HW_TOUCH_KEY
#include <huawei_platform/sensor/huawei_key.h>
#endif
#include <securec.h>

#include "acc_sysfs.h"
#include "airpress_sysfs.h"
#include "als_sysfs.h"
#include "als_tp_color.h"
#include "cap_prox_sysfs.h"
#include "contexthub_boot.h"
#include "contexthub_pm.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"
#include "gyro_sysfs.h"
#include "mag_sysfs.h"
#include "protocol.h"
#include "ps_sysfs.h"
#include "sensor_detect.h"
#include "sensor_sysfs.h"
#include "therm_sysfs.h"

static atomic_t enabled = ATOMIC_INIT(0);
int (*send_func)(int) = NULL;

__attribute__((weak)) int hisi_nve_direct_access(
	struct hisi_nve_info_user *user_info)
{
	if (!user_info)
		hwlog_info("%s: user_info is null\n", __func__);
	hwlog_info("%s: use weak\n", __func__);
	return 0;
}

static uint8_t hp_offset[MAX_HP_OFFSET_DATA_LENGTH];
static char sensor_chip_info[SENSOR_MAX][MAX_CHIP_INFO_LEN];
static u8 phone_color;
u8 tp_manufacture = TS_PANEL_UNKNOWN;
uint8_t ps_sensor_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
static uint8_t tof_sensor_calibrate_data[TOF_CALIDATA_NV_SIZE];
struct hisi_nve_info_user user_info;

int gsensor_offset[ACC_CALIBRATE_DATA_LENGTH]; /* g-sensor calibrate data */
int gyro_sensor_offset[GYRO_CALIBRATE_DATA_LENGTH];
char gyro_temperature_offset[GYRO_TEMP_CALI_NV_SIZE];
int32_t ps_sensor_offset[PS_CALIBRATE_DATA_LENGTH] = { 0 };
static uint8_t tof_sensor_offset[TOF_CALIDATA_NV_SIZE];
uint8_t gsensor_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
uint8_t msensor_calibrate_data[MAX_MAG_CALIBRATE_DATA_LENGTH];
uint8_t msensor_folder_calibrate_data[MAX_MAG_FOLDER_CALIBRATE_DATA_LENGTH];
uint8_t msensor1_calibrate_data[MAX_MAG_CALIBRATE_DATA_LENGTH];
uint8_t msensor_akm_calibrate_data[MAX_MAG_AKM_CALIBRATE_DATA_LENGTH];
uint8_t gyro_sensor_calibrate_data[GYRO_CALIDATA_NV_SIZE];
uint8_t gyro_temperature_calibrate_data[GYRO_TEMP_CALI_NV_SIZE];
static uint8_t handpress_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
static char vib_calib[VIB_CALIDATA_NV_SIZE] = { 0 };

uint8_t gsensor1_calibrate_data[ACC1_CALIBRATE_DATA_LENGTH];
int gsensor1_offset[ACC1_OFFSET_DATA_LENGTH]; /* gsensor1 calibrate data */
uint8_t gyro1_sensor_calibrate_data[GYRO1_OFFSET_NV_SIZE];
int gyro1_sensor_offset[GYRO1_CALIBRATE_DATA_LENGTH];

static int handpress_first_start_flag;

uint8_t *get_hp_offset(void)
{
	return hp_offset;
}

u8 get_phone_color(void)
{
	return phone_color;
}

void set_phone_color(u8 color)
{
	phone_color = color;
}

char *get_sensor_chip_info_address(enum sensor_detect_list index)
{
	return sensor_chip_info[index];
}

uint8_t *get_tof_sensor_calibrate_data(void)
{
	return tof_sensor_calibrate_data;
}

int get_handpress_first_start_flag(void)
{
	return handpress_first_start_flag;
}

__attribute__((weak)) int coul_drv_battery_current(void)
{
	hwlog_info("%s: use weak\n", __func__);
	return 0;
}
static void get_current_work_func(struct work_struct *work);
DECLARE_DELAYED_WORK(read_current_work, get_current_work_func);

void __dmd_log_report(int dmd_mark, const char *err_func, const char *err_msg)
{
#ifdef CONFIG_HUAWEI_DSM
	struct dsm_client *client = inputhub_get_shb_dclient();

	if (!dsm_client_ocuppy(client)) {
		dsm_client_record(client, "[%s]%s", err_func, err_msg);
		dsm_client_notify(client, dmd_mark);
	} else {
		hwlog_info("%s:dsm_client_ocuppy fail\n", __func__);
		dsm_client_unocuppy(client);
		if (!dsm_client_ocuppy(client)) {
			dsm_client_record(client, "[%s]%s", err_func, err_msg);
			dsm_client_notify(client, dmd_mark);
		}
	}
#endif
}

int read_calibrate_data_from_nv(int nv_number, int nv_size,
	const char *nv_name)
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
		hwlog_err("hisi_nve_direct_access read %d error %d\n", nv_number, ret);
		return -1;
	}
	return 0;
}

int write_calibrate_data_to_nv(int nv_number, int nv_size,
	const char *nv_name, const char *temp)
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
		hwlog_err("hisi_nve_direct_access write nv %d error %d\n",
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

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("send tag %d calibrate data to mcu fail %d\n", tag, ret);
		return -1;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("send tag %d calibrate data fail %d\n", tag, pkg_mcu.errno);
		return -1;
	} else {
		hwlog_info("send tag %d calibrate data to mcu success\n", tag);
	}
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

int send_subcmd_data_to_mcu_lock(int32_t tag, uint32_t subcmd,
	const void *data, uint32_t length, int32_t *err_no)
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

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret)
		hwlog_info("%s error, tag=%d subcmd=%u ret=%d\n", __func__,
			tag, subcmd, ret);
	else if (pkg_mcu.errno != 0)
		hwlog_info("%s fail, tag=%d subcmd=%u errno=%d\n", __func__,
			tag, subcmd, pkg_mcu.errno);

	if (err_no != NULL)
		*err_no = pkg_mcu.errno;

	return ret;
}

int send_vibrator_calibrate_data_to_mcu(void)
{
	int i;
	if (read_calibrate_data_from_nv(VIB_CALIDATA_NV_NUM,
		VIB_CALIDATA_NV_SIZE, VIB_CALIDATA_NV_NAME))
		return -1;

	/* f0 re change the nv position to ensure save the f0 for the old version */
	/* swap 0 1 2 and 3 4 5 */
	for (i = 0; i < 3; i++) {
		/* vib_calib 0 1 2 correspond to nv_data 3 4 5 */
		vib_calib[i] = (int8_t) user_info.nv_data[3 + i];
		/* vib_calib 3 4 5 correspond to nv_data 0 1 2 */
		vib_calib[3 + i] = (int8_t) user_info.nv_data[i];
	}
	/* from 6 one-to-one correspondence */
	for (i = 6; i < VIB_CALIDATA_NV_SIZE; i++)
		vib_calib[i] = (int8_t) user_info.nv_data[i];

	hwlog_err("%s read vib_calib 0x%x  0x%x  0x%x 0x%x  0x%x  0x%x 0x%x  0x%x  0x%x 0x%x  0x%x  0x%x\n", __func__,
		vib_calib[0], /* index 0 */
		vib_calib[1], /* index 1 */
		vib_calib[2], /* index 2 */
		vib_calib[3], /* index 3 */
		vib_calib[4], /* index 4 */
		vib_calib[5], /* index 5 */
		vib_calib[6], /* index 6 */
		vib_calib[7], /* index 7 */
		vib_calib[8], /* index 8 */
		vib_calib[9], /* index 9 */
		vib_calib[10], /* index 10 */
		vib_calib[11]); /* index 11 */
	if (send_calibrate_data_to_mcu(TAG_VIBRATOR, SUB_CMD_SET_OFFSET_REQ,
		vib_calib, VIB_CALIDATA_NV_SIZE, false)) {
		hwlog_err("send para fail\n");
		return -1;
	}
	return 0;
}

/* mcu send calidata to nv */
int mcu_save_calidata_to_nv(int tag, const int *para)
{
	unsigned long val = 1;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	pkt_parameter_req_t spkt;
	struct pkt_header *shd = (struct pkt_header *)&spkt;
	int ret;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	memset(&spkt, 0, sizeof(spkt));
	spkt.subcmd = SUB_CMD_SET_WRITE_NV_ATTER_SALE;
	pkg_ap.tag = tag;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = MAX_PS_CALIBRATE_DATA_LENGTH + SUBCMD_LEN;
	if (tag == TAG_ALS || tag == TAG_ALS1 || tag == TAG_ALS2) {
		pkg_ap.wr_len = sizeof(val) + SUBCMD_LEN;
		memcpy(spkt.para, &val, sizeof(val));
	} else {
		memcpy(spkt.para, para, MAX_PS_CALIBRATE_DATA_LENGTH);
	}
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("%s: send cmd to mcu fail,ret=%d\n", __func__, ret);
		ret = -1;
	} else {
		if (pkg_mcu.errno != 0) {
			hwlog_err("%s: mcu return fail\n", __func__);
			ret = -1;
		} else {
			hwlog_info("%s: send cmd to mcu succes\n", __func__);
			ret = 0;
		}
	}
	return ret;
}

int send_tof_calibrate_data_to_mcu(void)
{
	struct ps_device_info *dev_info = NULL;

	dev_info = ps_get_device_info(TAG_PS);
	if (dev_info == NULL)
		return -1;
	if (read_calibrate_data_from_nv(PS_CALIDATA_NV_NUM,
		TOF_CALIDATA_NV_SIZE, "PSENSOR"))
		return -1;

	dev_info->ps_first_start_flag = 1;

	memcpy(tof_sensor_offset, user_info.nv_data, sizeof(tof_sensor_offset));
	hwlog_info("nve_direct_access read tof sensor offset offset[0]=%d offset[9]=%d offset[19]=%d offset[27]=%d)\n",
		tof_sensor_offset[0], tof_sensor_offset[9], tof_sensor_offset[19],
		tof_sensor_offset[TOF_CALIDATA_NV_SIZE - 1]);

	memcpy(&tof_sensor_calibrate_data, tof_sensor_offset,
		(sizeof(tof_sensor_calibrate_data) < TOF_CALIDATA_NV_SIZE) ?
		sizeof(tof_sensor_calibrate_data) : TOF_CALIDATA_NV_SIZE);

	if (send_calibrate_data_to_mcu(TAG_TOF, SUB_CMD_SET_OFFSET_REQ,
		tof_sensor_offset, TOF_CALIDATA_NV_SIZE, false))
		return -1;

	return 0;
}

/* handpress nv read write */
int send_handpress_calibrate_data_to_mcu(void)
{
	if (read_calibrate_data_from_nv(HANDPRESS_CALIDATA_NV_NUM,
		HANDPRESS_CALIDATA_NV_SIZE, "HPDATA"))
		return -1;

	handpress_first_start_flag = 1;
	memcpy(hp_offset, user_info.nv_data, HANDPRESS_CALIDATA_NV_SIZE);
	memcpy(&handpress_calibrate_data, hp_offset,
		(sizeof(handpress_calibrate_data) < sizeof(hp_offset)) ?
		sizeof(handpress_calibrate_data) : sizeof(hp_offset));

	if (send_calibrate_data_to_mcu(TAG_HANDPRESS, SUB_CMD_SET_OFFSET_REQ,
		hp_offset, HANDPRESS_CALIDATA_NV_SIZE, false))
		return -1;

	return 0;
}

void reset_calibrate_data(void)
{
	reset_acc_calibrate_data();
	reset_acc1_calibrate_data();
	reset_mag_calibrate_data();
	reset_ps_calibrate_data();

	if (get_sensor_tof_flag() == 1)
		send_calibrate_data_to_mcu(TAG_TOF, SUB_CMD_SET_OFFSET_REQ,
			tof_sensor_calibrate_data, TOF_CALIDATA_NV_SIZE, true);

	reset_als_calibrate_data();
	reset_als1_calibrate_data();
	reset_als2_calibrate_data();
	reset_gyro_calibrate_data();
	reset_gyro1_calibrate_data();
	reset_airpress_calibrate_data();

	if (strlen(sensor_chip_info[HANDPRESS]))
		send_calibrate_data_to_mcu(TAG_HANDPRESS,
			SUB_CMD_SET_OFFSET_REQ, handpress_calibrate_data,
			AIRPRESS_CALIDATA_NV_SIZE, true);

	reset_cap_prox_calibrate_data();
	reset_cap_prox1_calibrate_data();
	reset_therm_calibrate_data();
}

int mcu_i3c_rw(uint8_t bus_num, uint8_t i2c_add, uint8_t *tx, uint32_t tx_len,
	uint8_t *rx_out, uint32_t rx_len)
{
	int ret;
	struct sensor_combo_cfg cfg;

	cfg.bus_type = TAG_I3C;
	cfg.bus_num = bus_num;
	cfg.i2c_address = i2c_add;

	ret = combo_bus_trans(&cfg, tx, tx_len, rx_out, rx_len);
	return (ret < 0) ? -1 : 0;
}

int mcu_i2c_rw(uint8_t i2c_i3c, uint8_t bus_num, uint8_t i2c_add, uint8_t *tx, uint32_t tx_len,
	uint8_t *rx_out, uint32_t rx_len)
{
	int ret;
	struct sensor_combo_cfg cfg;

	cfg.bus_type = i2c_i3c;
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

int combo_bus_trans(const struct sensor_combo_cfg *p_cfg, const uint8_t *tx,
	uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len)
{
	int ret;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	pkt_combo_bus_trans_req_t *pkt_combo_trans = NULL;
	uint32_t cmd_wd_len;
	uint32_t real_tx_len;

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
	} else if (pkg_ap.tag == TAG_I3C) {
		pkg_ap.cmd = CMD_I3C_TRANS_REQ;
	} else {
		hwlog_err("%s: bus_type %d unknown\n", __func__, p_cfg->bus_type);
		return -1;
	}
	/* get the real tx_len */
	if (tx_len & 0x80)
		real_tx_len = tx_len & 0x7F;
	else
		real_tx_len = tx_len;

	if (real_tx_len >= (uint32_t)0xFFFF - sizeof(*pkt_combo_trans)) {
		hwlog_err("%s: tx_len %x too big\n", __func__, real_tx_len);
		return -1;
	}
	cmd_wd_len = real_tx_len + sizeof(*pkt_combo_trans);
	pkt_combo_trans = kzalloc((size_t)cmd_wd_len, GFP_KERNEL);
	if (!pkt_combo_trans) {
		hwlog_err("alloc failed in %s\n", __func__);
		return -1;
	}

	pkt_combo_trans->busid   = p_cfg->bus_num;
	pkt_combo_trans->ctrl    = p_cfg->ctrl;
	pkt_combo_trans->rx_len  = (uint16_t)rx_len;
	pkt_combo_trans->tx_len  = (uint16_t)real_tx_len;
	if (real_tx_len && tx)
		memcpy((void *)pkt_combo_trans->tx, (void *)tx,
			(unsigned long)real_tx_len);

	pkg_ap.wr_buf = ((struct pkt_header *)pkt_combo_trans + 1);
	pkg_ap.wr_len = (int)(cmd_wd_len - sizeof(pkt_combo_trans->hd));

	hwlog_info("%s: tag %d cmd %d data=%d, tx_len=%d,rx_len=%d\n", __func__,
		pkg_ap.tag, pkg_ap.cmd, p_cfg->data, real_tx_len, rx_len);

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("send cmd to mcu fail, data=%d, tx_len=%d,rx_len=%d\n",
			p_cfg->data, real_tx_len, rx_len);
		ret = -1;
	} else {
		if (pkg_mcu.errno != 0) {
			hwlog_err("mcu_rw fail, data=%d, real_tx_len=%d,rx_len=%d\n",
				p_cfg->data, real_tx_len, rx_len);
			ret = -1;
		} else {
			if (rx_out && rx_len)
				memcpy((void *)rx_out, (void *)pkg_mcu.data,
					(unsigned long)rx_len);
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
		hwlog_err("In %s: read %s failed!\n", __func__, name);
		return -1;
	}
	if (flag == EXTEND_DATA_TYPE_IN_DTS_BYTE) {
		for (i = 0; i < len; i++)
			*(dest + i) = (uint8_t)buf[i];
	} else if (flag == EXTEND_DATA_TYPE_IN_DTS_HALF_WORD) {
		for (i = 0; i < len; i++) {
			memcpy(pdest, pbuf, 2); /* copy length is 2 */
			pdest += 2;
			pbuf++;
		}
	} else if (flag == EXTEND_DATA_TYPE_IN_DTS_WORD) {
		memcpy(dest, buf, len * sizeof(int));
	}

	return 0;
}

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

static int read_tp_module_notify(struct notifier_block *nb, unsigned long action, void *data)
{
	als_on_read_tp_module(action, data);
	return NOTIFY_OK;
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
	pkg_ap.wr_len = sizeof(key_brightness)+SUBCMD_LEN;
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
	long tmp_code = 0;
	int err;
	unsigned long tmp_len;

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
	tmp_len = (unsigned long)(dstr - pstr);
	if (tmp_len > sizeof(tmp)) {
		pr_err("command_line format error\n");
		return -EINVAL;
	}
	memcpy(tmp, pstr, tmp_len);
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
		hwlog_err("light sensor LCD/TP ID error!\n");
		phone_color = WHITE; /* WHITE is the default tp color */
	}
	hwlog_err("light sensor read tp color is %d, 0x%x\n",
		tp_color, phone_color);
}

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
	if (atomic_cmpxchg(&enabled, 1, 0))
		cancel_delayed_work_sync(&read_current_work);
	return 0;
}

