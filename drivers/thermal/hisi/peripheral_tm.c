/*
 * peripheral_tm.c
 *
 * peripheral thermal module
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#define pr_fmt(fmt)	"%s: " fmt, __func__

#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/completion.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/ipc_msg.h>
#include <linux/hisi/hisi_adc.h>
#include <securec.h>
#include "peripheral_tm.h"

#define PM_TM_DEV_NAME	"hisi_peripheral_tm"
#define PERIP_TO_LPM3_MSG_NUM	2
#define PERIP_RED_BACK_MSG_NUM	2
#define TSENSOR_USED_NUM		5
#define BOARD_BUFFER_LENGTH	40

enum tm_trip_type {
	TM_TRIP_HOT = 0,
	TM_TRIP_COOL,
	TM_TRIP_ORIGMAX,
#ifdef CONFIG_THERMAL_TRIP
	TSENS_TRIP_THROTTLING = TM_TRIP_ORIGMAX,
	TSENS_TRIP_SHUTDOWN,
	TSENS_TRIP_BELOW_VR_MIN,
	TSENS_TRIP_OVER_SKIN,
	TM_TRIP_MAX,
#endif
};

struct config {
	u8 channel;
	u8 ret;
	u16 value; /* volt for HKADC ,ADC for Tsensor */
};

struct tm_switch {
	u8 channel; /* tid */
	u8 which_thershold; /* level */
	u8 ret;
	u8 res;
};

struct tm_event {
	u8 channel; /* tid */
	u8 which_thershold; /* level */
	u16 value;
};

typedef union {
	unsigned char ipc_data[4];
	struct config setting;
	struct config getting;
	struct tm_switch onoff;
	struct tm_event  notify;
} u_thermal_ipc_para; /* IPC Data Register1 */

enum peripheral_temp_chanel {
	DETECT_SYSTEM_H_CHANEL = 0,
	DETECT_FLASH_LED_CHANEL,
	DETECT_CHARGER_CHANEL,
	DETECT_PA_0_CHANEL,
	DETECT_PA_1_CHANEL,
	DETECT_DCXO0_CHANEL,
	DETECT_SHELL_CHANEL,
	DETECT_CHARGER1_CHANEL,
	DETECT_RFBOARD_CHANEL,
	DETECT_USB_CHANEL,
	DETECT_WIRELESS_CHANEL,
	DETECT_IR_CHANEL,
	DETECT_DOT_CHANEL
};

char *peripheral_chanel[] = {
	[DETECT_SYSTEM_H_CHANEL] = "system_h",
	[DETECT_FLASH_LED_CHANEL] = "flash_led",
	[DETECT_CHARGER_CHANEL] = "charger",
	[DETECT_PA_0_CHANEL] = "pa_0",
	[DETECT_PA_1_CHANEL] = "pa_1",
	[DETECT_DCXO0_CHANEL] = "dcxo0",
	[DETECT_SHELL_CHANEL] = "shell",
	[DETECT_CHARGER1_CHANEL] = "charger1",
	[DETECT_RFBOARD_CHANEL] = "rfboard",
	[DETECT_USB_CHANEL] = "usb",
	[DETECT_WIRELESS_CHANEL] = "wireless",
	[DETECT_IR_CHANEL] = "ir",
	[DETECT_DOT_CHANEL] = "dot",
};

static struct peripheral_tm_chip *g_tm_dev;
int g_tm_debug = 0;

void tm_debug(int onoff)
{
	g_tm_debug = onoff;
}

static int peripheral_get_highres_temp(struct thermal_zone_device *thermal,
					    int *temp)
{
	struct periph_tsens_tm_device_sensor *chip = thermal->devdata;
	int ret, volt;

	if (chip == NULL)
		return -EINVAL;

	volt = hisi_adc_get_value(chip->chanel);
	if (volt <= 0) {
		pr_err("AP get volt value is fail,chan %d volt %d!\n",
		       chip->chanel, volt);
		volt = ADC_RTMP_DEFAULT_VALUE;
	}

	ret = peripheral_ntc_2_temp(chip, temp, volt);
	if (ret < 0) {
		pr_err("%s,get temp failed\n\r", __func__);
		return ret;
	}

	return 0;
}

#ifndef CONFIG_THERMAL_PERIPHERAL_HIGHRESTEMP
static int peripheral_tm_get_temp(struct thermal_zone_device *thermal,
				       int *temp)
{
	int ret;

	if (thermal == NULL || temp == NULL)
		return -EINVAL;

	ret = peripheral_get_highres_temp(thermal, temp);
	if (ret < 0) {
		pr_err("%s,get temp failed\n\r", __func__);
		return ret;
	}

	*temp = (*temp) / 1000;

	return 0;
}
#endif

static int peripheral_tm_get_mode(struct thermal_zone_device *thermal,
				       enum thermal_device_mode *mode)
{
	struct periph_tsens_tm_device_sensor *chip = thermal->devdata;

	if (chip == NULL || mode == NULL)
		return -EINVAL;

	*mode = chip->mode;

	return 0;
}

int peripheral_tm_target(union ipc_data *tm_ipc, int *value)
{
	int ret;
	u32 ack_buffer[2] = {0};
	union ipc_data *ipc_phr = (union ipc_data *)&ack_buffer[0];
	u_thermal_ipc_para *tm_ipc_para = (u_thermal_ipc_para *)&ack_buffer[1];

	ret = RPROC_SYNC_SEND(HISI_ACPU_LPM3_MBX_7, (rproc_msg_t *)tm_ipc,
			      (rproc_msg_len_t)PERIP_TO_LPM3_MSG_NUM,
			      (rproc_msg_t *)ack_buffer,
			      (rproc_msg_len_t)PERIP_RED_BACK_MSG_NUM);
	if (ret != 0) {
		pr_err("%s send data:%d %d\n", __func__, tm_ipc->data[0],
		       tm_ipc->data[1]);
		return -EINVAL;
	}

	if (ack_buffer[0] != tm_ipc->data[0]) {
		pr_err("%s recv data0 should same to send data0, %d != %d\n",
		       __func__, ack_buffer[0], tm_ipc->data[0]);
		return -EINVAL;
	}

	if (g_tm_debug != 0)
		pr_err("%s SEN 0x%0x 0x%0x ACK 0x%0x 0x%0x\n",
		       __func__, tm_ipc->data[0], tm_ipc->data[1],
		       ack_buffer[0], ack_buffer[1]); /*lint !e567*/

	switch (ipc_phr->cmd_mix.cmd) {
	case CMD_NOTIFY:
		if (value != NULL)
			*value = tm_ipc_para->getting.value;
	/* dont process cmd setting on off */
	case CMD_SETTING: /*lint !e616*/
	case CMD_ON:
	case CMD_OFF:
		break;
	default:
		pr_err("%s no this cmd ,ack_buffer reg0 %d reg1 %d\n",
		       __func__, ack_buffer[0], ack_buffer[1]);
		return -EINVAL;
	}

	return 0;
}

static int peripheral_tm_get_trip_temp(struct thermal_zone_device *thermal,
					    int trip, int *temp)
{
	struct periph_tsens_tm_device_sensor *tm_sensor = thermal->devdata;
	union ipc_data tm_ipc = {{0}};
	int volt = 0;
	int ret;
	u_thermal_ipc_para *tm_ipc_para =
		(u_thermal_ipc_para *)tm_ipc.cmd_mix.cmd_para;

	if (tm_sensor == NULL || trip < 0 || temp == NULL)
		return -EINVAL;

	(void)memset_s((void *)&tm_ipc, sizeof(tm_ipc), 0, sizeof(tm_ipc));

	switch (trip) {
	case TM_TRIP_HOT:
		tm_ipc.cmd_mix.cmd_type = TYPE_UPLIMIT;
		break;
	case TM_TRIP_COOL:
		tm_ipc.cmd_mix.cmd_type = TYPE_DNLIMIT;
		break;
#ifdef CONFIG_THERMAL_TRIP
	case TSENS_TRIP_THROTTLING:
		*temp = tm_sensor->temp_throttling;
		return 0;
	case TSENS_TRIP_SHUTDOWN:
		*temp = tm_sensor->temp_shutdown;
		return 0;
	case TSENS_TRIP_BELOW_VR_MIN:
		*temp = tm_sensor->temp_below_vr_min;
		return 0;
	case TSENS_TRIP_OVER_SKIN:
		*temp = tm_sensor->temp_over_skin;
		return 0;
#endif
	default:
		return -EINVAL;
	}

	tm_ipc.cmd_mix.cmd = CMD_NOTIFY;
	tm_ipc.cmd_mix.cmd_obj = OBJ_HKADC;
	tm_ipc.cmd_mix.cmd_src = OBJ_AP;
	tm_ipc_para->getting.channel = (u8)tm_sensor->chanel;

	ret = peripheral_tm_target(&tm_ipc, &volt);
	if (ret < 0) {
		pr_err("%s peripheral_tm_target ret %d\n", __func__, ret);
		return -EINVAL;
	}

	ret = peripheral_ntc_2_temp(tm_sensor, temp, volt);
	if (ret < 0) {
		pr_err("%s peripheral_get_temp ret %d volt %d\n",
		       __func__, ret, volt);
		return -EINVAL;
	}
	*temp = (*temp) / 1000;

	return 0;
}

static int peripheral_tm_set_trip_temp(struct thermal_zone_device *thermal,
					    int trip, int temp)
{
	struct periph_tsens_tm_device_sensor *tm_sensor = thermal->devdata;
	u16 volt = 0;
	int ret;
	union ipc_data tm_ipc = {{0}};
	u_thermal_ipc_para *tm_ipc_para = (u_thermal_ipc_para *)tm_ipc.cmd_mix.cmd_para;

	if (tm_sensor == NULL || trip < 0 || temp < NTC_TEMP_MIN_VALUE ||
	    temp > NTC_TEMP_MAX_VALUE) {
		pr_err("%s parm err temp %d\n", __func__, temp);
		return -EINVAL;
	}

	(void)memset_s((void *)&tm_ipc, sizeof(tm_ipc), 0, sizeof(tm_ipc));

	ret = peripheral_temp_2_ntc(tm_sensor, temp, &volt);
	if (ret < 0) {
		pr_err("%s: peripheral_temp_2_ntc ret %d\n", __func__, ret);
		return -EINVAL;
	}

	switch (trip) {
	case TM_TRIP_HOT:
		tm_ipc.cmd_mix.cmd_type = TYPE_UPLIMIT;
		break;
	case TM_TRIP_COOL:
		tm_ipc.cmd_mix.cmd_type = TYPE_DNLIMIT;
		break;
#ifdef CONFIG_THERMAL_TRIP
	case TSENS_TRIP_OVER_SKIN:
		tm_sensor->temp_over_skin = temp;
		return 0;
#endif
	default:
		return -EINVAL;
	}

	tm_ipc.cmd_mix.cmd = CMD_SETTING;
	tm_ipc.cmd_mix.cmd_obj = OBJ_HKADC;
	tm_ipc.cmd_mix.cmd_src = OBJ_AP;
	tm_ipc_para->setting.channel = (u8)tm_sensor->chanel;
	tm_ipc_para->setting.value = volt;

	ret = peripheral_tm_target(&tm_ipc, NULL);
	if (ret < 0) {
		pr_err("%s peripheral_tm_target ret %d\n", __func__, ret);
		return -EINVAL;
	}

	return 0;
}

static int peripheral_tm_trip_type(struct thermal_zone_device *thermal,
					int trip, enum thermal_trip_type *type)
{
	struct periph_tsens_tm_device_sensor *tm_sensor = thermal->devdata;

	if (tm_sensor == NULL || trip < 0 || type == NULL)
		return -EINVAL;
	switch (trip) {
	case TM_TRIP_HOT:
		*type = THERMAL_TRIP_CONFIGURABLE_HI;
		break;
	case TM_TRIP_COOL:
		*type = THERMAL_TRIP_CONFIGURABLE_LOW;
		break;
#ifdef CONFIG_THERMAL_TRIP
	case TSENS_TRIP_THROTTLING:
		*type = THERMAL_TRIP_THROTTLING;
		break;
	case TSENS_TRIP_SHUTDOWN:
		*type = THERMAL_TRIP_SHUTDOWN;
		break;
	case TSENS_TRIP_BELOW_VR_MIN:
		*type = THERMAL_TRIP_BELOW_VR_MIN;
		break;
	case TSENS_TRIP_OVER_SKIN:
		*type = THERMAL_TRIP_OVER_SKIN;
		break;
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

static int
peripheral_tm_activate_trip_type(struct thermal_zone_device *thermal,
				      int trip,
				      enum thermal_trip_activation_mode mode)
{
	struct periph_tsens_tm_device_sensor *tm_sensor = thermal->devdata;
	int ret;
	union ipc_data tm_ipc = {{0}};
	u_thermal_ipc_para *tm_ipc_para = (u_thermal_ipc_para *)tm_ipc.cmd_mix.cmd_para;

	if (tm_sensor == NULL || trip < 0)
		return -EINVAL;

	(void)memset_s((void *)&tm_ipc, sizeof(tm_ipc), 0, sizeof(tm_ipc));

	if (trip == TM_TRIP_HOT || trip == TM_TRIP_COOL) {
		tm_ipc_para->onoff.which_thershold = !trip;
	} else {
		pr_err("%s trip %d\n", __func__, trip);
		return -EINVAL;
	}

	if (mode == THERMAL_TRIP_ACTIVATION_ENABLED)
		tm_ipc.cmd_mix.cmd = CMD_ON;
	else
		tm_ipc.cmd_mix.cmd = CMD_OFF;

	tm_ipc.cmd_mix.cmd_type = TYPE_T;
	tm_ipc.cmd_mix.cmd_obj = OBJ_HKADC;
	tm_ipc.cmd_mix.cmd_src = OBJ_AP;
	tm_ipc_para->onoff.channel = (u8)tm_sensor->chanel;

	ret = peripheral_tm_target(&tm_ipc, NULL);
	if (ret != 0) {
		pr_err("%s peripheral_tm_target ret %d\n", __func__, ret);
		return -EINVAL;
	}

	return 0;
}

static struct thermal_zone_device_ops peripheral_thermal_zone_ops = {
#ifdef CONFIG_THERMAL_PERIPHERAL_HIGHRESTEMP
	.get_temp = peripheral_get_highres_temp,
#else
	.get_temp = peripheral_tm_get_temp,
#endif
	.get_mode = peripheral_tm_get_mode,
	.get_trip_type = peripheral_tm_trip_type,
	.activate_trip_type = peripheral_tm_activate_trip_type,
	.get_trip_temp = peripheral_tm_get_trip_temp,
	.set_trip_temp = peripheral_tm_set_trip_temp,
};

static int get_ntc_chanel(struct platform_device *pdev, int i)
{
	struct device_node *of_node = pdev->dev.of_node; /*lint !e578*/
	int equipment_chanel_value = 0;
	char temp_buffer[BOARD_BUFFER_LENGTH] = {0};
	int rc;

	rc = snprintf_s(temp_buffer, sizeof(temp_buffer),
			sizeof(temp_buffer) - 1, "hisi,detect_%s_tm_chanel",
			peripheral_chanel[i]);
	if (rc < 0) {
		dev_err(&pdev->dev, "snprintf_s error in %s :%d line\n",
			__func__, __LINE__);
		return -EINVAL;
	}

	rc = of_property_read_s32(of_node, temp_buffer,
				  &equipment_chanel_value);
	if (rc != 0) {
		dev_err(&pdev->dev, "canot get %d tm chanel\n", i);
		return -ENODEV;
	}

	g_tm_dev->sensor[i].chanel = equipment_chanel_value;

	return 0;
}

static int get_ntc_state(struct platform_device *pdev, int i)
{
	struct device_node *of_node = pdev->dev.of_node; /*lint !e578*/
	const char *ntc_state = NULL;
	char temp_buffer[BOARD_BUFFER_LENGTH] = {0};
	int rc;

	rc = snprintf_s(temp_buffer, sizeof(temp_buffer),
			sizeof(temp_buffer) - 1,
			"hisi,detect_%s_tm_state",
			peripheral_chanel[i]);
	if (rc < 0) {
		dev_err(&pdev->dev, "snprintf_s error in %s :%d line\n",
			__func__, __LINE__);
		return -EINVAL;
	}
	rc = of_property_read_string(of_node, temp_buffer,
				     &ntc_state);
	if (rc != 0) {
		dev_err(&pdev->dev, "detect %d tm ntc state failed\n", i);
		return -EINVAL;
	}
	if (strncmp(ntc_state, "enable",
		    strlen(ntc_state)) == 0) {
		g_tm_dev->sensor[i].state = 1;
	} else if (strncmp(ntc_state, "disable",
		   strlen(ntc_state)) == 0) {
		g_tm_dev->sensor[i].state = 0;
	} else {
		dev_err(&pdev->dev, "input ntc %d state is error\n", i);
		return -EINVAL;
	}
	return 0;
}

static int get_ntc_name(struct platform_device *pdev, int i)
{
	struct device_node *of_node = pdev->dev.of_node; /*lint !e578*/
	const char *equipment_chanel_ntc_name = NULL;
	char temp_buffer[BOARD_BUFFER_LENGTH] = {0};
	int rc;

	rc = snprintf_s(temp_buffer, sizeof(temp_buffer),
			sizeof(temp_buffer) - 1,
			"hisi,detect_%s_tm_ntc",
			peripheral_chanel[i]);
	if (rc < 0)  {
		dev_err(&pdev->dev, "snprintf_s error in %s :%d line\n",
			__func__, __LINE__);
		return -EINVAL;
	}
	rc = of_property_read_string(of_node, temp_buffer,
				     &equipment_chanel_ntc_name);
	if (rc != 0) {
		dev_err(&pdev->dev, "detect %d tm ntc name failed\n", i);
		return -EINVAL;
	}

	g_tm_dev->sensor[i].ntc_name =
		(const char *)kmemdup((const void *)equipment_chanel_ntc_name,
				      strlen(equipment_chanel_ntc_name) + 1,
				      GFP_KERNEL);
	if (g_tm_dev->sensor[i].ntc_name == NULL) {
		dev_err(&pdev->dev, "application %d chanel ntc "
			"name room failing\n", i);
		return -EINVAL;
	}
	return 0;
}

static int get_equipment_tree_data(struct platform_device *pdev, int sensor_num)
{
	int i, rc, j;

	/* get detect equipment thermal HKADC chanel, name and state */
	for (i = DETECT_SYSTEM_H_CHANEL, j = DETECT_SYSTEM_H_CHANEL;
	     i < sensor_num; i++, j++) {
		rc = get_ntc_chanel(pdev, i);
		if (rc < 0)
			goto read_chanel_fail;

		rc = get_ntc_state(pdev, i);
		if (rc < 0) {
			goto read_state_fail;
		}

		rc = get_ntc_name(pdev, i);
		if (rc < 0) {
			goto read_name_fail;
		}

		g_tm_dev->sensor[i].sensor_num =
			(unsigned int)(i + TSENSOR_USED_NUM);
	}

	return 0;

read_name_fail:
read_state_fail:
read_chanel_fail:
	for (i = DETECT_SYSTEM_H_CHANEL; i < j; i++) {
		kfree(g_tm_dev->sensor[i].ntc_name);
		g_tm_dev->sensor[i].ntc_name = NULL;
	}

	return rc;
}

static int get_periph_tm_device_tree_data(struct platform_device *pdev)
{
	const struct device_node *of_node = pdev->dev.of_node; /*lint !e578*/
	u32 rc;
	int periph_tsens_num_sensors = 0;

	rc = of_property_read_s32(of_node, "hisi,peripheral_sensors",
				  &periph_tsens_num_sensors);
	if (rc != 0) {
		dev_err(&pdev->dev, "missing sensor number\n");
		return -ENODEV;
	}

	g_tm_dev = devm_kzalloc(&pdev->dev, sizeof(struct peripheral_tm_chip) +
				periph_tsens_num_sensors *
				sizeof(struct periph_tsens_tm_device_sensor),
				GFP_KERNEL);
	if (g_tm_dev == NULL) {
		pr_err("%s: kzalloc() failed.\n", __func__);
		return -ENOMEM;
	}

	g_tm_dev->tsens_num_sensor = periph_tsens_num_sensors;
	rc = get_equipment_tree_data(pdev, periph_tsens_num_sensors);
	if (rc != 0) {
		dev_err(&pdev->dev, "get periph equipment dts data error\n");
		return -ENODEV;
	}

	return rc;
}

static int tm_m3_notifier(struct notifier_block *nb, unsigned long len,
			       void *msg)
{
	int i, hw_channel;
	union ipc_data *ptm_ipc = (union ipc_data *)msg;
	u_thermal_ipc_para *tm_ipc_para =
		(u_thermal_ipc_para *)ptm_ipc->cmd_mix.cmd_para;
	int m3_notfier_flag = IPC_CMD(OBJ_LPM3, OBJ_HKADC, CMD_NOTIFY, TYPE_T);
	if (m3_notfier_flag != ptm_ipc->data[0])
		return 0;

	if (g_tm_dev == NULL)
		return 0;

	hw_channel = (tm_ipc_para->notify).channel;

	for (i = 0; i < g_tm_dev->tsens_num_sensor; i++) {
		if (g_tm_dev->sensor[i].chanel == hw_channel) {
			schedule_work(&g_tm_dev->sensor[i].work);
			return 0;
		}
	}

	return 0;
}

static void notify_thermal_app(struct work_struct *work)
{
	struct periph_tsens_tm_device_sensor *tm =
		container_of(work, struct periph_tsens_tm_device_sensor, work);

	sysfs_notify(&tm->tz_dev->device.kobj, NULL, "type");
	if (g_tm_debug == 1)
		pr_err("%s TZ %s hw channel %d\n", __func__,
		       tm->tz_dev->type, tm->chanel);
}

#ifdef CONFIG_THERMAL_CONTEXTHUB
static int parse_adc_table(void)
{
	int i, ret, s_size, chub_hdr_len, chub_len;
	char *p_chub_ddr = NULL;
	unsigned long src;
	enum hkadc_table_id t_id = HKADC_MAX_TABLEID;
	int used_tbl_idx = 0;

	p_chub_ddr = g_tm_dev->chub->share_addr +
		     CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE;
	chub_hdr_len = CONTEXTHUB_THERMAL_DDR_TOTAL_SIZE -
		CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE;

	for (i = 0; i < g_tm_dev->tsens_num_sensor; i++) {
		if (g_tm_dev->sensor[i].state == 0)
			continue;

		g_tm_dev->chub->adc_table[used_tbl_idx].usr_id =
			(unsigned short int)g_tm_dev->sensor[i].sensor_num;
		g_tm_dev->chub->adc_table[used_tbl_idx].hw_channel =
			(unsigned short int)g_tm_dev->sensor[i].chanel;
		(void)peripheral_get_table_info(g_tm_dev->sensor[i].ntc_name,
						     NULL, &t_id);
		g_tm_dev->chub->adc_table[used_tbl_idx].table_id = t_id;
		s_size = peripheral_get_table_info(NULL, &src, &t_id);
		if (s_size > CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE ||
		    s_size <= 0) {
			pr_err("%s tableID %d SIZE %d MAX %d\n", __func__,
			       t_id, s_size, CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE);
			return -EINVAL;
		}
		g_tm_dev->chub->adc_table[used_tbl_idx].table_size = s_size;
		chub_len = chub_hdr_len -
			   CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE * t_id;
		ret = memcpy_s((void *)(p_chub_ddr +
			       CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE * t_id),
			       chub_len,
			       (void *)(uintptr_t)src, s_size);
		if (ret != EOK) {
			pr_err("%s:%d %d :memcpy_s copy failed.\n",
			       __func__, __LINE__, ret);
			return -EINVAL;
		}
		used_tbl_idx = used_tbl_idx + 1;
	}
	return 0;
}
#endif

void thermal_contexthub_init(void)
{
#ifdef CONFIG_THERMAL_CONTEXTHUB
	int chan_table_size, ret;
	struct hw_chan_table *p_ddr_header = NULL;

	if (g_tm_dev == NULL) {
		pr_err("%s g_tm_dev is NULL\n", __func__);
		return;
	}

	chan_table_size = g_tm_dev->tsens_num_sensor  *
			  sizeof(struct hw_chan_table);
	g_tm_dev->chub = devm_kzalloc(&g_tm_dev->pdev->dev,
				      sizeof(struct contexthub_chip) +
				      chan_table_size, GFP_KERNEL);
	if (g_tm_dev->chub == NULL) {
		pr_err("%s kzalloc() failed.\n", __func__);
		return;
	}

	(void)memset_s((void *)g_tm_dev->chub->adc_table, chan_table_size, 0xFF,
		       chan_table_size);

	g_tm_dev->chub->share_addr =
		ioremap_wc(CONTEXTHUB_THERMAL_DDR_HEADER_ADDR,
			   CONTEXTHUB_THERMAL_DDR_TOTAL_SIZE);
	if (g_tm_dev->chub->share_addr == NULL) {
		pr_err("%s share_addr ioremap_wc failed.\n", __func__);
		return;
	}

	p_ddr_header = g_tm_dev->chub->share_addr;
	ret = parse_adc_table();
	if (ret != 0)
		return;

	if (CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE < chan_table_size +
	    sizeof(struct hw_chan_table)) {
		pr_err("%s chan_table_size>max %d\n", __func__,
		       CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE);
		return;
	}

	p_ddr_header++;
	while (p_ddr_header->hw_channel != 0xFFFF)
		p_ddr_header++;
	ret = memcpy_s((void *)p_ddr_header, chan_table_size,
		       g_tm_dev->chub->adc_table, chan_table_size);
	if (ret != EOK) {
		pr_err("%s:%d:memcpy_s copy failed.\n", __func__, __LINE__);
		return;
	}
	p_ddr_header = g_tm_dev->chub->share_addr;
	p_ddr_header->hw_channel = 0xFFFE;
#endif
}

#ifdef CONFIG_THERMAL_TRIP
static void get_system_h_dts_info(struct platform_device *pdev, int i)
{
	char node_name[50] = {0};
	s32 temp_throttling = 0;
	s32 temp_shutdown = 0;
	s32 temp_below_vr_min = 0;
	s32 temp_over_skin = 0;
	const struct device_node *node = pdev->dev.of_node;
	int rc;

	rc = snprintf_s(node_name, sizeof(node_name),
			sizeof(node_name) - 1,
			"hisi,detect_%s_throttling",
			peripheral_chanel[i]);
	if (rc < 0) {
		dev_err(&pdev->dev, "snprintf_s error in %s :%d line\n",
			__func__, __LINE__);
		temp_throttling = 0;
	}
	rc = of_property_read_s32(node, node_name,
				  &temp_throttling);
	if (rc != 0) {
		dev_err(&pdev->dev, "temp_throttling node not found!\n");
		temp_throttling = 0;
	}
	g_tm_dev->sensor[i].temp_throttling = temp_throttling;
	(void)memset_s((void *)node_name, sizeof(node_name), 0,
		       sizeof(node_name));
	rc = snprintf_s(node_name, sizeof(node_name),
			sizeof(node_name) - 1,
			"hisi,detect_%s_shutdown",
			peripheral_chanel[i]);
	if (rc < 0) {
		dev_err(&pdev->dev, "snprintf_s error in %s :%d line\n",
			__func__, __LINE__);
		temp_shutdown = 0;
	}
	rc = of_property_read_s32(node, node_name, &temp_shutdown);
	if (rc != 0) {
		dev_err(&pdev->dev, "temp_shutdown node not found!\n");
		temp_shutdown = 0;
	}
	g_tm_dev->sensor[i].temp_shutdown = temp_shutdown;

	(void)memset_s((void *)node_name, sizeof(node_name), 0,
		       sizeof(node_name));
	rc = snprintf_s(node_name, sizeof(node_name),
			sizeof(node_name) - 1,
			"hisi,detect_%s_below_vr_min",
			peripheral_chanel[i]);
	if (rc < 0) {
		dev_err(&pdev->dev, "snprintf_s error "
			"in %s :%d line\n", __func__, __LINE__);
		temp_below_vr_min = 0;
	}
	rc = of_property_read_s32(node, node_name,
				  &temp_below_vr_min);
	if (rc != 0) {
		dev_err(&pdev->dev, "temp_below_vr_min "
			"node not found!\n");
		temp_below_vr_min = 0;
	}
	g_tm_dev->sensor[i].temp_below_vr_min = temp_below_vr_min;

	(void)memset_s((void *)node_name, sizeof(node_name), 0,
		       sizeof(node_name));
	rc = snprintf_s(node_name, sizeof(node_name),
			sizeof(node_name) - 1,
			"hisi,detect_%s_over_skin",
			peripheral_chanel[i]);
	if (rc < 0) {
		dev_err(&pdev->dev, "snprintf_s error "
			"in %s :%d line\n", __func__, __LINE__);
		temp_over_skin = 0;
	}
	rc = of_property_read_s32(node, node_name, &temp_over_skin);
	if (rc != 0) {
		dev_err(&pdev->dev, "temp_over_skin node not found!\n");
		temp_over_skin = 0;
	}
	g_tm_dev->sensor[i].temp_over_skin = temp_over_skin;
}
#endif

static int peripheral_tm_probe(struct platform_device *pdev)
{
	int rc, i, flag;
	char name[BOARD_BUFFER_LENGTH] = {0};
	unsigned int trips_num, k;

	if (!of_device_is_available(pdev->dev.of_node)) {
		dev_err(&pdev->dev, "peripheral dev not found\n");
		return -ENODEV;
	}

	rc = get_periph_tm_device_tree_data(pdev);
	if (rc != 0) {
		dev_err(&pdev->dev, "Error reading peripheral tm\n");
		return rc;
	}

	g_tm_dev->pdev = pdev;

	thermal_contexthub_init();

	for (i = DETECT_SYSTEM_H_CHANEL, flag = DETECT_SYSTEM_H_CHANEL;
	     i < (g_tm_dev->tsens_num_sensor + DETECT_SYSTEM_H_CHANEL); i++) {
		unsigned int mask = 0;

		if (g_tm_dev->sensor[i].state == 0)
			continue;

		(void)memset_s((void *)name, sizeof(name), 0, sizeof(name));
		rc = snprintf_s(name, sizeof(name), sizeof(name) - 1, "%s",
				peripheral_chanel[i]);
		if (rc < 0) {
			dev_err(&pdev->dev, "snprintf_s error in %s :%d line\n",
				__func__, __LINE__);
			goto fail;
		}

		g_tm_dev->sensor[i].mode = THERMAL_DEVICE_ENABLED;

#ifdef CONFIG_THERMAL_TRIP
		if ((enum peripheral_temp_chanel)i == DETECT_SYSTEM_H_CHANEL) {
			get_system_h_dts_info(pdev, i);
			trips_num = TM_TRIP_MAX;
		} else {
			trips_num = TM_TRIP_ORIGMAX;
		}
#else
		trips_num = TM_TRIP_ORIGMAX;
#endif

		for (k = 0; k < trips_num; k++)
			mask |= (unsigned int)(1U << k);

		g_tm_dev->sensor[i].tz_dev =
			thermal_zone_device_register(name, trips_num, (int)mask,
						     &g_tm_dev->sensor[i],
						     &peripheral_thermal_zone_ops,
						     NULL, 0, 0);
		flag++;
		if (IS_ERR(g_tm_dev->sensor[i].tz_dev)) {
			dev_err(&pdev->dev, "thermal_zone_device_register() failed.\n");
			rc = -ENODEV;
			goto fail;
		}
	}

	platform_set_drvdata(pdev, g_tm_dev);

	for (k = 0; k < (unsigned int)g_tm_dev->tsens_num_sensor; k++)
		INIT_WORK(&g_tm_dev->sensor[k].work, notify_thermal_app);

	g_tm_dev->nb.next = NULL;
	g_tm_dev->nb.notifier_call = tm_m3_notifier;
	rc = RPROC_MONITOR_REGISTER(HISI_LPM3_ACPU_MBX_1, &g_tm_dev->nb);
	if (rc != 0) {
		dev_err(&pdev->dev, "%s rproc monitor register Failed!\n", __func__);
		return -ENODEV;
	}

	return 0;
fail:
	for (i = DETECT_SYSTEM_H_CHANEL; i < flag; i++)
		thermal_zone_device_unregister(g_tm_dev->sensor[i].tz_dev);

	return rc;
}

static int peripheral_tm_remove(struct platform_device *pdev)
{
	struct peripheral_tm_chip *chip = platform_get_drvdata(pdev);
	int i;

	if (chip != NULL) {
		platform_set_drvdata(pdev, NULL);
		for (i = DETECT_SYSTEM_H_CHANEL;
		     i < (g_tm_dev->tsens_num_sensor + DETECT_SYSTEM_H_CHANEL); i++) {
			kfree(g_tm_dev->sensor[i].ntc_name);
			g_tm_dev->sensor[i].ntc_name = NULL;
			thermal_zone_device_unregister(g_tm_dev->sensor[i].tz_dev);
		}
		kfree(chip);
	}
	return 0;
}

static struct of_device_id peripheral_tm_match[] = {
	{ .compatible = "hisi,hisi-peripheral-tm", },
	{}
};

static struct platform_driver peripheral_tm_driver = {
	.probe = peripheral_tm_probe,
	.remove = peripheral_tm_remove,
	.driver = {
		.name = PM_TM_DEV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = peripheral_tm_match,
	},
};

static int __init peripheral_tm_init(void)
{
	return platform_driver_register(&peripheral_tm_driver);
}

static void __exit peripheral_tm_exit(void)
{
	platform_driver_unregister(&peripheral_tm_driver);
}

int periph_get_temp(u32 sensor, int *temp)
{
	int i;
	struct thermal_zone_device *thermal = NULL;
	int ret = -EINVAL;
	int tmp = 0;

	if (g_tm_dev == NULL) {
		pr_err("%s g_tm_dev is NULL, maybe periph probe is failed.\n",
		       __func__);
		return -EINVAL;
	}

	for (i = 0; i < g_tm_dev->tsens_num_sensor; i++) {
		if ((sensor + TSENSOR_USED_NUM) == g_tm_dev->sensor[i].sensor_num) {
			thermal = g_tm_dev->sensor[i].tz_dev;
			if (thermal != NULL)
				ret = peripheral_get_highres_temp(thermal, &tmp);
			*temp = tmp;
		}
	}

	return ret;
}

int ipa_get_periph_id(const char *name)
{
	int ret = -ENODEV;
	u32 id;
	u32 sensor_num = sizeof(peripheral_chanel) / sizeof(char *);

	if (name == NULL)
		return ret;

	pr_info("IPA periph sensor_num = %d\n", sensor_num);
	for (id = 0; id < sensor_num; id++) {
		pr_info("IPA: sensor_name = %s, tsensor_name %d = %s\n",
			name, id, peripheral_chanel[id]);

		if (strlen(name) == strlen(peripheral_chanel[id]) &&
		    strncmp(name, peripheral_chanel[id], strlen(name)) == 0) {
			ret = (int)id;
			pr_info("sensor_id = %d\n", ret);
			return ret;
		}
	}

	return ret;
}
EXPORT_SYMBOL_GPL(ipa_get_periph_id);

int ipa_get_periph_value(u32 sensor, int *val)
{
	int ret = -EINVAL;
	u32 periph_num = sizeof(peripheral_chanel) / sizeof(char *);

	if (val == NULL) {
		pr_err("%s parm null\n", __func__);
		return ret;
	}

	if (sensor < periph_num)
		ret = periph_get_temp(sensor, val);
	else
		pr_err("peripheral sensor %d is not supported!\n", sensor);

	return ret;
}
EXPORT_SYMBOL_GPL(ipa_get_periph_value);

module_init(peripheral_tm_init);
module_exit(peripheral_tm_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Peripheral Thermal Manager driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:" PM_TM_DEV_NAME);
