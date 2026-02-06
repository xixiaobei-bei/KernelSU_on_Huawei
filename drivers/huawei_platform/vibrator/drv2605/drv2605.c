/*
 * drv2605.c
 *
 * drv2605 driver
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#include "drv2605.h"

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/semaphore.h>
#include <linux/device.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/spinlock_types.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/pm_wakeup.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/jiffies.h>
#include <linux/hisi/hw_cmdline_parse.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

/* enable time */
static unsigned long g_enable_time_jef;
static unsigned long g_vibra_time_jef;
static long g_enable_time;
static long g_vibra_time;

/* calibration */
static char vib_calib[CALI_DATA_SIZE];
static int vib_calib_result;
static bool vib_init_calibdata;
int hisi_nve_direct_access(struct hisi_nve_info_user *user_info);

/* data */
static struct i2c_client *client_temp;
static struct wakeup_source vib_wakelock;
static struct drv2605_pdata *pdata;
static struct drv2605_data *data;

static char g_effect_bank = EFFECT_LIBRARY;
static bool vibrator_is_playing;

/* for finger sensor */
#if defined(CONFIG_HISI_VIBRATOR)
extern volatile int vibrator_shake;
#else
volatile int vibrator_shake;
#endif

static char reg_value;
static char reg_add;
static char rtp_strength = 0x7F; // vibrator strength init value
static ssize_t haptics_write(struct file *filp, const char *buff,
			     size_t len, loff_t *off);

/* reg_map */
static const uint8_t erm_autocal_sequence[] = {
	MODE_REG, AUTO_CALIBRATION,
	REAL_TIME_PLAYBACK_REG, REAL_TIME_PLAYBACK_STRENGTH,
	LIBRARY_SELECTION_REG, EFFECT_LIBRARY,
	WAVEFORM_SEQUENCER_REG, WAVEFORM_SEQUENCER_DEFAULT,
	WAVEFORM_SEQUENCER_REG2, WAVEFORM_SEQUENCER_DEFAULT,
	WAVEFORM_SEQUENCER_REG3, WAVEFORM_SEQUENCER_DEFAULT,
	WAVEFORM_SEQUENCER_REG4, WAVEFORM_SEQUENCER_DEFAULT,
	WAVEFORM_SEQUENCER_REG5, WAVEFORM_SEQUENCER_DEFAULT,
	WAVEFORM_SEQUENCER_REG6, WAVEFORM_SEQUENCER_DEFAULT,
	WAVEFORM_SEQUENCER_REG7, WAVEFORM_SEQUENCER_DEFAULT,
	WAVEFORM_SEQUENCER_REG8, WAVEFORM_SEQUENCER_DEFAULT,
	OVERDRIVE_TIME_OFFSET_REG, 0x00,
	SUSTAIN_TIME_OFFSET_POS_REG, 0x00,
	SUSTAIN_TIME_OFFSET_NEG_REG, 0x00,
	BRAKE_TIME_OFFSET_REG, 0x00,
	AUDIO_HAPTICS_CONTROL_REG,
	AUDIO_HAPTICS_RECT_20MS | AUDIO_HAPTICS_FILTER_125HZ,
	AUDIO_HAPTICS_MIN_INPUT_REG, AUDIO_HAPTICS_MIN_INPUT_VOLTAGE,
	AUDIO_HAPTICS_MAX_INPUT_REG, AUDIO_HAPTICS_MAX_INPUT_VOLTAGE,
	AUDIO_HAPTICS_MIN_OUTPUT_REG, AUDIO_HAPTICS_MIN_OUTPUT_VOLTAGE,
	AUDIO_HAPTICS_MAX_OUTPUT_REG, AUDIO_HAPTICS_MAX_OUTPUT_VOLTAGE,
	RATED_VOLTAGE_REG, ERM_RATED_VOLTAGE,
	OVERDRIVE_CLAMP_VOLTAGE_REG, ERM_OVERDRIVE_CLAMP_VOLTAGE,
	AUTO_CALI_RESULT_REG, DEFAULT_ERM_AUTOCAL_COMPENSATION,
	AUTO_CALI_BACK_EMF_RESULT_REG, DEFAULT_ERM_AUTOCAL_BACKEMF,
	FEEDBACK_CONTROL_REG,
	FB_BRAKE_FACTOR_3X | LOOP_RESPONSE_MEDIUM |
		FEEDBACK_CONTROL_BEMF_ERM_GAIN2,
	CONTROL1_REG, STARTUP_BOOST_ENABLED | DEFAULT_DRIVE_TIME,
	CONTROL2_REG,
	BIDIRECT_INPUT | AUTO_RES_GAIN_MEDIUM | BLANKING_TIME_SHORT |
		IDISS_TIME_SHORT,
	CONTROL3_REG, ERM_OPENLOOP_ENABLED | NG_THRESH_2,
	AUTOCAL_MEM_INTERFACE_REG, AUTOCAL_TIME_500MS,
	GO_REG, GO,
};

static const uint8_t lra_autocal_sequence[] = {
	MODE_REG, AUTO_CALIBRATION,
	FEEDBACK_CONTROL_REG,
	FEEDBACK_CONTROL_MODE_LRA | FB_BRAKE_FACTOR_3X | LOOP_RESPONSE_MEDIUM,
	RATED_VOLTAGE_REG, LRA_RATED_VOLTAGE,
	OVERDRIVE_CLAMP_VOLTAGE_REG, LRA_OVERDRIVE_CLAMP_VOLTAGE,
	CONTROL3_REG, NG_THRESH_1,
	GO_REG, GO,
};

#if SKIP_LRA_AUTOCAL == 1
static const uint8_t lra_init_sequence[] = {
	MODE_REG, MODE_INTERNAL_TRIGGER,
	REAL_TIME_PLAYBACK_REG, REAL_TIME_PLAYBACK_STRENGTH,
	LIBRARY_SELECTION_REG, LIBRARY_F,
	GO_REG, STOP,
	CONTROL1_REG, 0x90,
	CONTROL2_REG, 0xC2,
	CONTROL3_REG, 0xA0,
	AUTOCAL_MEM_INTERFACE_REG, 0x30,
};
#endif

struct {
	int haptics_type;
	char haptics_value[HAPTICS_NUM];
} haptics_table[] = {
	{ 1, { 0x04, 0, 0, 0, 0, 0, 0, 0 } },
	{ 2, { 0x18, 0x06, 0x18, 0x06, 0x18, 0, 0, 0 } },
	{ 3, { 0x01, 0, 0, 0, 0, 0, 0, 0 } },
	{ 4, { 0x02, 0, 0, 0, 0, 0, 0, 0 } },
	{ 5, { 0x07, 0, 0, 0, 0, 0, 0, 0 } },
	{ 6, { 0x0A, 0, 0, 0, 0, 0, 0, 0 } },
	{ 7, { 0x0E, 0x85, 0x0E, 0x85, 0, 0, 0, 0 } },
	{ 8, { 0x10, 0xE4, 0, 0, 0, 0, 0, 0 } },
	{ 9, { 0x67, 0, 0, 0, 0, 0, 0, 0} },
	{ 10, { 0x67, 0x85, 0x67, 0x85, 0, 0, 0, 0 } },
	{ 11, { 0x05, 0, 0, 0, 0, 0, 0, 0 } },
	{ 12, { 0x15, 0, 0, 0, 0, 0, 0, 0 } },
	{ 13, { 0x16, 0, 0, 0, 0, 0, 0, 0 } },
	{ 14, { 0x1B, 0, 0, 0, 0, 0, 0, 0 } },
	{ 15, { 0x1C, 0, 0, 0, 0, 0, 0, 0 } },
	{ 16, { 0x52, 0x15, 0, 0, 0, 0, 0, 0 } },
	{ 17, { 0x53, 0x15, 0, 0, 0, 0, 0, 0 } },
	{ 18, { 0x6A, 0x16, 0, 0, 0, 0, 0, 0 } },
	{ 19, { 0x04, 0, 0, 0, 0, 0, 0, 0 } },
	{ 20, { 0x06, 0, 0, 0, 0, 0, 0, 0 } },
	{ 21, { 0x06, 0, 0, 0, 0, 0, 0, 0 } },
	{ 22, { 0x05, 0, 0, 0, 0, 0, 0, 0 } },
	{ 23, { 0x04, 0, 0, 0, 0, 0, 0, 0 } },
	{ 31, { 0x2E, 0, 0, 0, 0, 0, 0, 0 } },
	{ 32, { 0x2D, 0, 0, 0, 0, 0, 0, 0 } },
	{ 33, { 0x2C, 0, 0, 0, 0, 0, 0, 0 } },
};

#ifdef CONFIG_HUAWEI_DSM
static struct dsm_dev dsm_vibrator = {
	.name = "dsm_vibrator",
	.device_name = "DRV2605",
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024, // vibrator dsm buf size
};
static struct dsm_client *vib_dclient;
#endif

#ifdef CONFIG_HUAWEI_DSM
#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
static char *vibra_cali_name[VIB_CAL_NUM] = {
	"VIBRATOR_CAL_STATUS",
	"VIBRATOR_CAL_COMPENSATE",
	"VIBRATOR_CAL_BACK_EMF",
	"VIBRATOR_CAL_FEEDBACK_CTL"
};
static int cali_result_val[VIB_CAL_NUM];

static void set_event_of_msg(struct message *msg, int index, int result)
{
	const int max_throshold = 0xFF;
	const int min_threshold = 0x0;
	struct event *events = &(msg->events[index]);
	char *test_result = result == 0 ? "fail" : "pass";
	int error_code = result == 0 ? (-1) : (0);

	memcpy(events->station, VIB_STATION, sizeof(VIB_STATION));
	memcpy(events->device_name, VIB_TEST_NAME, sizeof(VIB_TEST_NAME));
	memcpy(events->bsn, NOT_INVOLVE, sizeof(NOT_INVOLVE));
	memcpy(events->firmware, NOT_INVOLVE, sizeof(NOT_INVOLVE));
	memcpy(events->description, NOT_INVOLVE, sizeof(NOT_INVOLVE));
	memcpy(events->result, test_result, (strlen(test_result) + 1));
	memcpy(events->test_name, vibra_cali_name[index],
		(strlen(vibra_cali_name[index]) + 1));

	events->item_id = VIB_CAL_STATUS_MSG + index;
	events->error_code = error_code;

	snprintf(events->max_threshold, MAX_VAL_LEN, "%d", max_throshold);
	snprintf(events->min_threshold, MAX_VAL_LEN, "%d", min_threshold);
	snprintf(events->value, MAX_VAL_LEN, "%d", cali_result_val[index]);
}

static int vibra_cal_enq_notify_work(int result)
{
	int ret;
	struct message *msg = NULL;
	int i;

	msg = kzalloc(sizeof(*msg), GFP_KERNEL);
	if (msg == NULL) {
		dev_err(&client_temp->dev, "%s:NULL point\n", __func__);
		return -1;
	}

	/* init msg */
	msg->data_source = DATA_FROM_KERNEL;
	msg->num_events = 0;
	msg->version = DEFAULT_VERSION;

	for (i = 0; i < MAX_MSG_EVENT_NUM && i < VIB_CAL_NUM; i++)
		set_event_of_msg(msg, i, result);
	msg->num_events = i;
	ret = dsm_client_copy_ext(vib_dclient, msg, sizeof(struct message));
	if (ret <= 0) {
		dev_err(&client_temp->dev, "dsm_client_copy_ext failed\n");
		kfree(msg);
		return -1;
	}

	dev_info(&client_temp->dev, "%s: succ\n", __func__);
	kfree(msg);
	return 0;
}

static void enq_notify_work_vibrator(int result)
{
	int ret;

	if (!dsm_client_ocuppy(vib_dclient)) {
		ret = vibra_cal_enq_notify_work(result);
		if (ret == 0)
			dsm_client_notify(vib_dclient, DA_VIBRATOR_ERROR_NO);
	}
}
#endif
#endif

static void drv2605_i2c_write(struct i2c_client *client,
			      uint8_t reg_address,
			      uint8_t reg_val)
{
	int ret;

	if (client == NULL) {
		dev_err(&client_temp->dev, "%s:NULL point\n", __func__);
		return;
	}

	ret = i2c_smbus_write_byte_data(client, reg_address, reg_val);
	if (ret < 0)
		dev_err(&client_temp->dev,
			"%s:i2c wirte fail, data = %u, %u\n",
			__func__, reg_address, reg_val);
}

static void drv2605_write_reg_val(struct i2c_client *client,
				  const uint8_t *data,
				  uint32_t size)
{
	uint8_t i;
	int ret;

	if (client == NULL || data == NULL) {
		dev_err(&client_temp->dev, "%s:NULL point\n", __func__);
		return;
	}
	if (size % 2 != 0) { // size must be even
		dev_err(&client_temp->dev, "%s:invalid size\n", __func__);
		return;
	}

	for (i = 0; i < size; i = i + 2) {
		ret = i2c_smbus_write_byte_data(client, data[i], data[i + 1]);
		if (ret < 0)
			dev_err(&client_temp->dev,
				"%s:i2c wirte fail, data = %u, %u\n",
				__func__, data[i], data[i + 1]);
	}
}

static void drv2605_set_go_bit(struct i2c_client *client, uint8_t val)
{
	uint8_t go[] = { GO_REG, val };

	if (client == NULL) {
		dev_err(&client_temp->dev, "%s:NULL point\n", __func__);
		return;
	}

	drv2605_write_reg_val(client, go, sizeof(go));
}

static int drv2605_read_reg(struct i2c_client *client, uint8_t reg)
{
	int ret;

	if (client == NULL) {
		dev_err(&client_temp->dev, "%s:NULL point\n", __func__);
		return -1;
	}

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0)
		dev_err(&client_temp->dev,
			"%s:read i2c error reg = %u, ret = %d\n",
			__func__, reg, ret);
	return ret;
}

static void drv2605_poll_go_bit(struct i2c_client *client)
{
	if (client == NULL) {
		dev_err(&client_temp->dev, "%s:NULL point\n", __func__);
		return;
	}

	while (drv2605_read_reg(client, GO_REG) == GO)
		schedule_timeout_interruptible(
			msecs_to_jiffies(GO_BIT_POLL_INTERVAL));
}

static void drv2605_select_library(struct i2c_client *client, uint8_t lib)
{
	char library[] = { LIBRARY_SELECTION_REG, lib };

	if (client == NULL) {
		dev_err(&client_temp->dev, "%s:NULL point\n", __func__);
		return;
	}

	drv2605_write_reg_val(client, library, sizeof(library));
}

static void drv2605_set_rtp_val(struct i2c_client *client, uint8_t value)
{
	unsigned long vibra_jef;
	long time_dif = 0;
	uint8_t rtp_val[] = { REAL_TIME_PLAYBACK_REG, value };

	if (client == NULL) {
		dev_err(&client_temp->dev, "%s:NULL point\n", __func__);
		return;
	}

	if (value == 0) {
		vibra_jef = jiffies - g_vibra_time_jef;
		g_vibra_time = jiffies_to_msecs(vibra_jef);
		time_dif = g_enable_time - g_vibra_time;
		if ((time_dif > 0) && (time_dif < MAX_TIME_DIFF))
			msleep(time_dif);
		dev_info(&client->dev,
			 "%s:g_enable_time:%ld g_vibra_time:%ld\n",
			 __func__, g_enable_time, g_vibra_time);
	} else {
		g_vibra_time_jef = jiffies;
	}
	drv2605_write_reg_val(client, rtp_val, sizeof(rtp_val));
	dev_info(&client->dev, "Strength: %02X value: time_dif:%ld\n",
		 value, time_dif);
}

static void drv2605_set_waveform_sequence(struct i2c_client *client,
					  uint8_t *seq, uint32_t size)
{
	uint8_t data[WAVEFORM_SEQUENCER_MAX + 1] = {0};

	if (client == NULL || seq == NULL) {
		dev_err(&client_temp->dev, "%s:NULL point\n", __func__);
		return;
	}

	if (size > WAVEFORM_SEQUENCER_MAX) {
		dev_err(&client_temp->dev, "%s:invaild size\n", __func__);
		return;
	}
	data[0] = WAVEFORM_SEQUENCER_REG;
	memcpy(&(data[1]), seq, size); // seq copy to data 1~8
	i2c_master_send(client, data, sizeof(data));
}

static void drv2605_change_mode(struct i2c_client *client, uint8_t mode)
{
	unsigned char tmp[] = { MODE_REG, mode };

	if (client == NULL) {
		dev_err(&client_temp->dev, "%s:NULL point\n", __func__);
		return;
	}

	drv2605_write_reg_val(client, tmp, sizeof(tmp));
}

static int read_vibrator_calib_value_from_nv(void)
{
	int ret;
	struct hisi_nve_info_user user_info;

	memset(&user_info, 0, sizeof(user_info));

	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = VIB_CALIDATA_NV_NUM;
	user_info.valid_size = VIB_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "VIBCAL", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		dev_err(&client_temp->dev,
			 "nve_direct_access read error(%d)\n", ret);
		return -1;
	}

	vib_calib[0] = (int8_t)user_info.nv_data[0];
	vib_calib[1] = (int8_t)user_info.nv_data[1];
	vib_calib[2] = (int8_t)user_info.nv_data[2];
	dev_info(&client_temp->dev, "read vib_calib (0x%x  0x%x  0x%x )\n",
		 vib_calib[0], vib_calib[1], vib_calib[2]);
	return 0;
}

static int write_vibrator_calib_value_to_nv(const uint8_t *temp, uint8_t size)
{
	struct hisi_nve_info_user user_info;

	if (temp == NULL || size < CALI_DATA_SIZE) {
		dev_err(&client_temp->dev, "%s: fail, invalid para\n",
			__func__);
		return -1;
	}

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE_TAG;
	user_info.nv_number = VIB_CALIDATA_NV_NUM;
	user_info.valid_size = VIB_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "VIBCAL", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	user_info.nv_data[0] = temp[0];
	user_info.nv_data[1] = temp[1];
	user_info.nv_data[2] = temp[2];
	if (hisi_nve_direct_access(&user_info) != 0) {
		dev_err(&client_temp->dev,
			 "nve_direct_access write error\n");
		return -1;
	}

	dev_info(&client_temp->dev,
		 "nve_direct_access write nv_data (0x%x 0x%x 0x%x )\n",
		 user_info.nv_data[0],
		 user_info.nv_data[1],
		 user_info.nv_data[2]);

	return 0;
}

static int save_vibrator_calib_value_to_reg(void)
{
	int reg_value;
	int ret;
	uint8_t gain_value;

	reg_value = drv2605_read_reg(client_temp, AUTO_CALI_RESULT_REG);
	if (reg_value != AUTO_CALI_RESULT_REG_VALUE) {
		dev_err(&client_temp->dev,
			"%s: AUTO_CALI_RESULT_REG err = %d\n",
			__func__, reg_value);
		return -1;
	}
	reg_value = drv2605_read_reg(client_temp,
				     AUTO_CALI_BACK_EMF_RESULT_REG);
	if (reg_value != EMF_RESULT_REG_VALUE) {
		dev_err(&client_temp->dev,
			"%s: AUTO_CALI_BACK_EMF_RESULT_REG err = %d\n",
			__func__, reg_value);
		return -1;
	}
	reg_value = drv2605_read_reg(client_temp, FEEDBACK_CONTROL_REG);
	if ((reg_value & 0x02) == 0) { // bit 2 is 1
		dev_err(&client_temp->dev,
			"%s: FEEDBACK_CONTROL_REG err = %d\n",
			__func__, reg_value);
		return -1;
	}

	ret = read_vibrator_calib_value_from_nv();
	if (ret != 0) {
		dev_err(&client_temp->dev,
			"%s: read_vibrator_calib_value_from_nv ret = %d\n",
			__func__, ret);
		return -1;
	}

	drv2605_i2c_write(client_temp, AUTO_CALI_RESULT_REG, vib_calib[0]);
	drv2605_i2c_write(client_temp, AUTO_CALI_BACK_EMF_RESULT_REG,
		vib_calib[1]);

	/* reg_value high 6 bit & vib_calib[2] low 2 bit */
	gain_value = ((uint8_t)reg_value & 0xFC) | (vib_calib[2] & 0x3);
	drv2605_i2c_write(client_temp, FEEDBACK_CONTROL_REG, gain_value);

	return 0;
}

#ifdef CONFIG_HUAWEI_DSM
static void check_power_state(void)
{
	int gpio_val1 = -1;
	int gpio_val2 = -1;
	static int fpc_check_time;

	if (pdata->check_power_state1 < 0 && pdata->check_power_state2 < 0)
		return;
	if (pdata->check_power_state1 >= 0)
		gpio_val1 = gpio_get_value(pdata->check_power_state1);
	if (pdata->check_power_state2 >= 0)
		gpio_val2 = gpio_get_value(pdata->check_power_state2);
	if ((gpio_val1 == HIGH_VOL_ERR) || (gpio_val2 == HIGH_VOL_ERR)) {
		dev_err(&(data->client->dev),
			"get check_power_state1 = %d, stat2 = %d\n",
			gpio_val1, gpio_val2);
		if (fpc_check_time > (pdata->check_power_time))
			return;
		if (fpc_check_time == pdata->check_power_time) {
			if (!dsm_client_ocuppy(vib_dclient)) {
				dsm_client_record(vib_dclient,
					"gpio_%d = %d, gpio_%d = %d\n",
					pdata->check_power_state1, gpio_val1,
					pdata->check_power_state2, gpio_val2);
				dsm_client_notify(vib_dclient,
					DSM_VIBRATOR_FPC_CHECK_NO);
			}
		}
		fpc_check_time++;
	}
}
#endif

static void vibrator_off(struct drv2605_data *data)
{
	if (!vibrator_is_playing) {
		dev_err(&(data->client->dev), "drv2605 is not playing\n");
		return;
	}
	drv2605_set_rtp_val(data->client, 0);
	msleep(25); // wait 25ms for vibra stop
	vibrator_is_playing = false;
	drv2605_change_mode(data->client, MODE_STANDBY);
	vibrator_shake = 0;
	#ifdef CONFIG_HUAWEI_DSM
	check_power_state();
	#endif
	dev_info(&(data->client->dev), "drv2605 off!");
}

static void vibrator_on(struct drv2605_data *dara)
{
	int ret;

	drv2605_change_mode(data->client, MODE_DEVICE_READY);
	udelay(1000); // delay 1ms for mode ok
	if (!vib_init_calibdata) {
		ret = save_vibrator_calib_value_to_reg();
		if (ret != 0)
			dev_err(&(data->client->dev),
				"save vibrator calib value fail:%d\n", ret);
		vib_init_calibdata = true;
	}

	ret = drv2605_read_reg(data->client, MODE_REG);
	if ((ret & DRV260X_MODE_MASK) != MODE_REAL_TIME_PLAYBACK) {
		vibrator_shake = 1;
		drv2605_set_rtp_val(data->client, rtp_strength);
		drv2605_change_mode(data->client, MODE_REAL_TIME_PLAYBACK);
		vibrator_is_playing = true;
	}
}

static void vibrator_enable(struct led_classdev *dev, enum led_brightness value)
{
	struct drv2605_data *data = NULL;
	unsigned long enable_time;

	if (dev == NULL || value < 0) {
		pr_err("%s: invalid data", __func__);
		return;
	}

	data = container_of(dev, struct drv2605_data, cclassdev);
	if (data == NULL) {
		pr_err("%s: vibrator data is NULL\n", __func__);
		return;
	}
	data->value = value;
	dev_info(&(data->client->dev), "value: %d\n", data->value);

	if (value > 0) {
		g_enable_time_jef = jiffies; // start vibra time
	} else {
		enable_time = jiffies - g_enable_time_jef;
		g_enable_time = jiffies_to_msecs(enable_time);
	}

	schedule_work(&data->work_enable);
}

static void vibrator_work(struct work_struct *work)
{
	struct drv2605_data *data = NULL;

	data = container_of(work, struct drv2605_data, work);
	if (data == NULL) {
		pr_err("%s: vibrator data is NULL\n", __func__);
		return;
	}

	vibrator_off(data);
}

static void vibrator_work_enable(struct work_struct *work)
{
	struct drv2605_data *data = NULL;
	uint8_t value;

	data = container_of(work, struct drv2605_data, work_enable);
	if (data == NULL) {
		pr_err("%s: vibrator data is NULL\n", __func__);
		return;
	}
	value = (uint8_t)data->value;

	mutex_lock(&data->lock);

	if (value > 0)
		vibrator_on(data);
	else
		vibrator_off(data);

	mutex_unlock(&data->lock);
}

static void play_effect(struct work_struct *work)
{
	struct drv2605_data *data = NULL;

	data = container_of(work, struct drv2605_data, work_play_eff);
	if (data == NULL) {
		pr_err("%s: NULL point\n", __func__);
		return;
	}

	vibrator_shake = 1;
	switch_set_state(&data->sw_dev, SW_STATE_SEQUENCE_PLAYBACK);
	drv2605_change_mode(data->client, MODE_DEVICE_READY);
	udelay(1000); // wait 1ms for change mode succ
	vibrator_is_playing = true;
	drv2605_set_waveform_sequence(data->client, data->sequence,
				      sizeof(data->sequence));
	drv2605_set_go_bit(data->client, GO);

	while ((drv2605_read_reg(data->client, GO_REG) == GO) &&
		(!data->should_stop))
		schedule_timeout_interruptible(
			msecs_to_jiffies(GO_BIT_POLL_INTERVAL));

	if (data->should_stop == YES)
		drv2605_set_go_bit(data->client, STOP);

	drv2605_change_mode(data->client, MODE_STANDBY);
	switch_set_state(&data->sw_dev, SW_STATE_IDLE);
	vibrator_is_playing = false;
	vibrator_shake = 0;
}

static int drv2605_parse_dt(struct device *dev, struct drv2605_pdata *pdata)
{
	unsigned int gpo;
	int temp = 0;

	if (of_property_read_string(dev->of_node, "ti,label",
		&pdata->name) != 0) {
		pdata->name = "vibrator";
		dev_info(dev, "%s: read name fail, use default\n", __func__);
	}

	/* configure minimum idle timeout */
	if (of_property_read_u32(dev->of_node, "ti,max-timeout-ms",
		&temp) != 0)
		pdata->max_timeout_ms = MAX_TIMEOUT;
	else
		pdata->max_timeout_ms = temp;
	/* configure reduce timeout */
	if (of_property_read_u32(dev->of_node, "ti,reduce-timeout-ms",
		&temp) != 0)
		pdata->reduce_timeout_ms = 0;
	else
		pdata->reduce_timeout_ms = temp;
	if (of_property_read_u32(dev->of_node, "lra_rated_voltage",
		&temp) != 0)
		pdata->lra_rated_voltage = LRA_RATED_VOLTAGE;
	else
		pdata->lra_rated_voltage = (char)temp;
	if (of_property_read_u32(dev->of_node, "lra_overdriver_voltage",
		&temp) != 0)
		pdata->lra_overdriver_voltage = LRA_OVERDRIVE_CLAMP_VOLTAGE;
	else
		pdata->lra_overdriver_voltage = (char)temp;
	if (of_property_read_u32(dev->of_node, "lra_rtp_strength",
		&temp) != 0)
		pdata->lra_rtp_strength = REAL_TIME_PLAYBACK_STRENGTH;
	else
		pdata->lra_rtp_strength = (char)temp;
	if (of_property_read_u32(dev->of_node, "support_amplitude_control",
		&temp) != 0)
		pdata->support_amplitude_control = 0;
	else
		pdata->support_amplitude_control = (char)temp;

	pdata->check_power_state1 = of_get_named_gpio(dev->of_node,
						      "check_power_state1", 0);
	pdata->check_power_state2 = of_get_named_gpio(dev->of_node,
						      "check_power_state2", 0);
	dev_info(dev, "read dts check power state1:%d, state2:%d\n",
		pdata->check_power_state1, pdata->check_power_state2);

	if (!gpio_is_valid(pdata->check_power_state1)) {
		dev_info(dev, "check_power_state1 is invalid\n");
		pdata->check_power_state1 = -1;
	} else {
		gpo = (unsigned int)(pdata->check_power_state1);
		if (gpio_request(gpo, "vib_check1") < 0)
			dev_info(dev, "request vib_check1 err\n");
		else
			dev_info(dev, "power_state1 support dmd check\n");
	}
	if (!gpio_is_valid(pdata->check_power_state2)) {
		dev_info(dev, "check_power_state2 is invalid\n");
		pdata->check_power_state2 = -1;
	} else {
		gpo = (unsigned int)(pdata->check_power_state2);
		if (gpio_request(gpo, "vib_check2") < 0)
			dev_info(dev, "request vib_check2 err\n");
		else
			dev_info(dev, "power_state2 support dmd check\n");
	}

	if (of_property_read_u32(dev->of_node, "check_power_time",
		&temp) < 0)
		pdata->check_power_time = -1; // invalid time
	else
		pdata->check_power_time = temp;

	dev_info(dev, "max check_power_time:%d\n", pdata->check_power_time);
	dev_info(dev, "max timedout_ms:%d\n", pdata->max_timeout_ms);

	pdata->gpio_enable = of_get_named_gpio(dev->of_node, "gpio-enable", 0);
	if (!gpio_is_valid(pdata->gpio_enable))
		dev_err(dev, "pdata->gpio_enable is invalid\n");

	return 0;
}
static void set_dbc_test_close(void)
{
	uint8_t mode_close[] = { MODE_REG, 0x0 };

	drv2605_write_reg_val(client_temp, mode_close, sizeof(mode_close));
	drv2605_change_mode(client_temp, MODE_STANDBY); // standby
}

static void set_dbc_test_open(void)
{
	uint8_t mode_open[] = { MODE_REG, 0x5 };

	drv2605_write_reg_val(client_temp, mode_open, sizeof(mode_open));
}

static void set_dbc_out_add(void)
{
	uint8_t rtp_out[] = { REAL_TIME_PLAYBACK_REG, 0x7f };

	drv2605_write_reg_val(client_temp, rtp_out, sizeof(rtp_out));
}

static void set_dbc_out_delete(void)
{
	uint8_t rtp_out[] = { REAL_TIME_PLAYBACK_REG, 0x81 };

	drv2605_write_reg_val(client_temp, rtp_out, sizeof(rtp_out));

}

static void dbc_test_set_erm_mode(void)
{
	uint8_t erm_mode;
	int ret;

	erm_mode = drv2605_read_reg(client_temp, FEEDBACK_CONTROL_REG);
	erm_mode = 0x7f & erm_mode; // set bit 8 = 0
	dev_info(&client_temp->dev, "%s: erm_mode = %u\n", __func__,
		erm_mode);
	ret = i2c_smbus_write_byte_data(client_temp,
		FEEDBACK_CONTROL_REG, erm_mode);
	if (ret < 0)
		dev_err(&client_temp->dev, "wirte mode fail\n");
}

static void dbc_test_set_erm_loop(void)
{
	uint8_t erm_loop;
	int ret;

	erm_loop = drv2605_read_reg(client_temp, CONTROL3_REG);
	erm_loop = 0x20 | erm_loop; // set bit 6 = 1
	dev_info(&client_temp->dev, "%s: erm_loop = %u\n", __func__,
		erm_loop);
	ret = i2c_smbus_write_byte_data(client_temp, CONTROL3_REG, erm_loop);
	if (ret < 0)
		dev_err(&client_temp->dev, "wirte loop fail\n");
}

static void dbc_test_set_overdrvier_val(void)
{
	uint8_t overdriver[] = { OVERDRIVE_CLAMP_VOLTAGE_REG, 0xff };

	drv2605_write_reg_val(client_temp, overdriver, sizeof(overdriver));
}

static ssize_t vibrator_dbc_test_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf,
				       size_t count)
{
	uint64_t value = 0;

	if (buf == NULL) {
		pr_err("%s: NULL point\n", __func__);
		return -EINVAL;
	}

	dev_info(&client_temp->dev, "start vibrator_dbc_test\n");

	drv2605_change_mode(client_temp, MODE_DEVICE_READY);
	udelay(1000); // wait 1ms for change mode succ
	dbc_test_set_erm_mode();
	dbc_test_set_erm_loop();
	dbc_test_set_overdrvier_val();

	if (kstrtoll(buf, 16, &value)) {
		dev_info(&client_temp->dev, "%s: read value error\n",
			__func__);
		return -EINVAL;
	}

	switch (value) {
	case CLOSE_TEST:
		dev_info(&client_temp->dev, "vibrator dbc test close\n");
		set_dbc_test_close();
		break;
	case OUT_ADD_TEST:
		dev_info(&client_temp->dev, "vibrator dbc test out+\n");
		set_dbc_out_add();
		set_dbc_test_open();
		break;
	case OUT_DELETE_TEST:
		dev_info(&client_temp->dev, "vibrator dbc test out-\n");
		set_dbc_out_delete();
		set_dbc_test_open();
		break;
	default:
		dev_info(&client_temp->dev, "%s invalid value\n", __func__);
		break;
	}

	return (ssize_t)count;
}

static ssize_t vibrator_calib_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf,
				    size_t count)
{
	int status;
	uint8_t lra_mode;
	int ret;
	char calib_value[CALI_DATA_SIZE] = {0};

	dev_info(&client_temp->dev, "start vibrator auto calibrate\n");

	drv2605_change_mode(client_temp, MODE_DEVICE_READY);
	udelay(1000); // wait 1ms for change mode succ
	lra_mode = drv2605_read_reg(client_temp, FEEDBACK_CONTROL_REG);
	lra_mode = 0x80 | lra_mode; // just care high bite

	/* wirte reg, start calibration */
	drv2605_i2c_write(client_temp, OVERDRIVE_CLAMP_VOLTAGE_REG,
			  LRA_OVERDRIVE_CLAMP_VOLTAGE);
	drv2605_i2c_write(client_temp, FEEDBACK_CONTROL_REG, lra_mode);
	drv2605_i2c_write(client_temp, MODE_REG, MODE_REG_VALUE);
	drv2605_set_go_bit(client_temp, GO);

	/* Wait until the procedure is done */
	drv2605_poll_go_bit(client_temp);

	/* Read calibration result */
	status = drv2605_read_reg(client_temp, STATUS_REG);
	dev_info(&client_temp->dev, "calibration status =0x%x\n", status);
#ifdef CONFIG_HUAWEI_DSM
#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
	cali_result_val[0] = status;
#endif
#endif

	/* Read calibration value */
	calib_value[0] = drv2605_read_reg(client_temp,
					  AUTO_CALI_RESULT_REG);
	calib_value[1] = drv2605_read_reg(client_temp,
					  AUTO_CALI_BACK_EMF_RESULT_REG);
	calib_value[2] = drv2605_read_reg(client_temp,
					  FEEDBACK_CONTROL_REG);
	calib_value[2] = 0x03 & calib_value[2]; // get bit 0~1
	dev_info(&client_temp->dev, "calibration value =0x%x, 0x%x, 0x%x\n",
		 calib_value[0], calib_value[1], calib_value[2]);

	status = 0x08 & status; // get bit 5
	if (status != 0) {
		vib_calib_result = 0;
		dev_info(&client_temp->dev, "vibrator calibration fail\n");
	} else {
		vib_calib_result = 1;
	}

#ifdef CONFIG_HUAWEI_DSM
#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
	/* copy cal value to big data array */
	cali_result_val[1] = calib_value[0];
	cali_result_val[2] = calib_value[1];
	cali_result_val[3] = calib_value[2];
	enq_notify_work_vibrator(vib_calib_result);
#endif
#endif
	if (vib_calib_result == 0) // 0 calib fail
		return (ssize_t)count;
	/* write calibration value to nv */
	ret = write_vibrator_calib_value_to_nv(calib_value, CALI_DATA_SIZE);
	if (ret != 0) {
		vib_calib_result = 0;
		dev_info(&client_temp->dev,
			 "vibrator calibration result write to nv fail\n");
	}

	drv2605_i2c_write(client_temp, OVERDRIVE_CLAMP_VOLTAGE_REG,
			  pdata->lra_overdriver_voltage);
	drv2605_change_mode(client_temp, MODE_STANDBY);

	return (ssize_t)count;
}

static ssize_t vibrator_calib_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", vib_calib_result);
}

static ssize_t vibrator_get_reg_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf,
				      size_t count)
{
	uint64_t value = 0;
	char reg_address;

	if (buf == NULL) {
		dev_info(&client_temp->dev, "%s: NULL point\n", __func__);
		return (ssize_t)count;
	}

	if (kstrtoull(buf, 16, &value)) {
		dev_info(&client_temp->dev, "%s: read value error\n",
			__func__);
		return (ssize_t)count;
	}
	reg_address = (char)value;
	reg_value = drv2605_read_reg(client_temp, reg_address);
	dev_info(&client_temp->dev, "reg_address is 0x%x, reg_value is 0x%x\n",
		 reg_address, reg_value);
	return (ssize_t)count;
}

static ssize_t vibrator_get_reg_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", (int)reg_value);
}

static ssize_t vibrator_set_reg_address_store(struct device *dev,
					      struct device_attribute *attr,
					      const char *buf,
					      size_t count)
{
	uint64_t value = 0;

	if (buf == NULL) {
		dev_info(&client_temp->dev, "%s: NULL point\n", __func__);
		return (ssize_t)count;
	}

	if (kstrtoull(buf, 16, &value)) {
		dev_info(&client_temp->dev, "%s: read value error\n",
			__func__);
		return (ssize_t)count;
	}
	reg_add = (char)value;
	dev_info(&client_temp->dev, "reg_addr is 0x%x\n", reg_add);
	return (ssize_t)count;
}

static ssize_t vibrator_set_reg_value_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf,
					    size_t count)
{
	uint8_t val;
	uint64_t value = 0;

	if (buf == NULL) {
		dev_info(&client_temp->dev, "%s: NULL point\n", __func__);
		return (ssize_t)count;
	}

	if (kstrtoull(buf, 16, &value)) {
		dev_info(&client_temp->dev, "%s read value error\n",
			__func__);
		return (ssize_t)count;
	}

	drv2605_change_mode(client_temp, MODE_DEVICE_READY);
	udelay(1000); // wait 1ms for change mode succ
	val = (uint8_t)value;
	dev_info(&client_temp->dev, "reg_add is 0x%x, reg_value is 0x%x.\n",
		reg_add, val);
	drv2605_i2c_write(client_temp, reg_add, val);
	drv2605_change_mode(client_temp, MODE_STANDBY);
	return (ssize_t)count;
}

static char str_to_num_u8(char num1, char num2)
{
	char num = 0;

	if (num1 >= '0' && num1 <= '9')
		num = num + (num1 - '0') * 16;
	else if (num1 >= 'a' && num1 <= 'f')
		num = num + (num1 - 'a' + 10) * 16;
	else
		return (-1);

	if (num2 >= '0' && num2 <= '9')
		num = num + (num2 - '0');
	else if (num2 >= 'a' && num2 <= 'f')
		num = num + (num1 - 'a' + 10);
	else
		return (-1);

	return num;
}

static ssize_t haptic_test_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf,
				 size_t count)
{
	char haptic_value[MAX_HAP_BUF_SIZE] = {0};
	char value = 0;
	int i;
	int j;

	if (count < MIN_HAP_BUF_SIZE ||
		count > MAX_HAP_BUF_SIZE ||
		buf == NULL) {
		dev_info(&client_temp->dev, "haptic_test bad value\n");
		return -1;
	}

	for (i = 0, j = 0; i < (count - 1) && j < MAX_HAP_BUF_SIZE; i = i + 2) {
		value = str_to_num_u8(buf[i], buf[i + 1]);
		if (value < 0 || value == END_HAPTIC_TEST_FLAG)
			break;
		haptic_value[j] = value;
		dev_info(&client_temp->dev, "-----> haptic_test2 is 0x%x\n",
			haptic_value[j]);
		j++;
	}

	/* vibrator work */
	cancel_work_sync(&data->work);
	vibrator_off(data);
	memcpy(&data->sequence, &haptic_value, HAPTICS_NUM);
	schedule_work(&data->work_play_eff);
	return (ssize_t)count;
}

static ssize_t vibrator_set_rtp_value_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf,
					    size_t count)
{
	uint64_t value = 0;

	if (buf == NULL) {
		dev_info(&client_temp->dev, "%s: NULL point\n", __func__);
		return (ssize_t)count;
	}

	if (kstrtoull(buf, 16, &value)) {
		dev_info(&client_temp->dev, "%s read value error\n",
			__func__);
		return (ssize_t)count;
	}

	rtp_strength = (char)value;
	dev_info(&client_temp->dev, "rtp_value is 0x%x\n", rtp_strength);
	return (ssize_t)count;
}

static ssize_t vibrator_reg_value_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	uint8_t reg_address;
	uint8_t reg_val;

	if (buf == NULL) {
		dev_info(&client_temp->dev, "%s: NULL point\n", __func__);
		return -EINVAL;
	}

	reg_address = (uint8_t)reg_add;
	reg_val = drv2605_read_reg(client_temp, reg_address);
	dev_info(&client_temp->dev, "reg_address is 0x%x, reg_value is 0x%x\n",
		reg_address, reg_val);

	return snprintf(buf, PAGE_SIZE, "%u\n", (unsigned int)reg_val);
}

static ssize_t vibrator_reg_value_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf,
					size_t count)
{
	uint8_t val;
	uint64_t value = 0;

	if (buf == NULL) {
		dev_info(&client_temp->dev, "%s: NULL point\n", __func__);
		return (ssize_t)count;
	}
	if (kstrtoull(buf, 16, &value)) {
		dev_info(&client_temp->dev, "%s: read value error\n",
			__func__);
		return (ssize_t)count;
	}
	val = (uint8_t)value;
	dev_info(&client_temp->dev, "reg_add is 0x%x, reg_value is 0x%x\n",
		 reg_add, val);

	drv2605_i2c_write(client_temp, reg_add, val);
	return (ssize_t)count;
}

static ssize_t set_amplitude_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf,
				   size_t count)
{
	uint8_t val;
	uint64_t value = 0;

	if (buf == NULL) {
		dev_info(&client_temp->dev, "%s: NULL point\n", __func__);
		return (ssize_t)count;
	}
	if (kstrtoull(buf, 10, &value)) {
		dev_info(&client_temp->dev, "%s read value error\n",
			__func__);
		return (ssize_t)count;
	}
	val = (uint8_t)value;

	drv2605_change_mode(client_temp, MODE_DEVICE_READY);
	udelay(1000); // wait 1ms for change mode succ
	dev_info(&client_temp->dev, "%s:reg_values = 0x%x\n", __func__, val);
	drv2605_i2c_write(client_temp, RATED_VOLTAGE_REG, val);
	drv2605_change_mode(client_temp, MODE_STANDBY);
	return (ssize_t)count;
}

static ssize_t support_amplitude_control_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (buf == NULL) {
		dev_info(&client_temp->dev, "%s: NULL point\n", __func__);
		return -EINVAL;
	}

	return snprintf(buf, PAGE_SIZE, "%d\n",
		pdata->support_amplitude_control);
}

static DEVICE_ATTR(vibrator_dbc_test, 0600, NULL,
		   vibrator_dbc_test_store);
static DEVICE_ATTR(vibrator_calib, 0600, vibrator_calib_show,
		   vibrator_calib_store);
static DEVICE_ATTR(vibrator_get_reg, 0600, vibrator_get_reg_show,
		   vibrator_get_reg_store);
static DEVICE_ATTR(vibrator_set_reg_address, 0600, NULL,
		   vibrator_set_reg_address_store);
static DEVICE_ATTR(vibrator_set_reg_value, 0600, NULL,
		   vibrator_set_reg_value_store);
static DEVICE_ATTR(haptic_test, 0600, NULL, haptic_test_store);
static DEVICE_ATTR(vibrator_set_rtp_value, 0660, NULL,
		   vibrator_set_rtp_value_store);
static DEVICE_ATTR(vibrator_reg_value, 0600, vibrator_reg_value_show,
		   vibrator_reg_value_store);
static DEVICE_ATTR(set_amplitude, 0600, NULL,
		   set_amplitude_store);
static DEVICE_ATTR(support_amplitude_control, 0600,
		   support_amplitude_control_show,
		   NULL);

static struct attribute *vb_attributes[] = {
	&dev_attr_vibrator_dbc_test.attr,
	&dev_attr_vibrator_calib.attr,
	&dev_attr_vibrator_get_reg.attr,
	&dev_attr_vibrator_set_reg_address.attr,
	&dev_attr_vibrator_set_reg_value.attr,
	&dev_attr_haptic_test.attr,
	&dev_attr_vibrator_set_rtp_value.attr,
	&dev_attr_vibrator_reg_value.attr,
	&dev_attr_set_amplitude.attr,
	&dev_attr_support_amplitude_control.attr,
	NULL
};

static const struct attribute_group vb_attr_group = {
	.attrs = vb_attributes,
};

static int haptics_open(struct inode *i_node, struct file *filp)
{
	if (data == NULL || filp == NULL)
		return -ENODEV;
	dev_info(&client_temp->dev, "%s\n", __func__);
	filp->private_data = data;
	return 0;
}

static ssize_t haptics_write(struct file *filp, const char *buff,
			     size_t len, loff_t *off)
{
	struct drv2605_data *data = NULL;
	int i;
	int table_num;
	uint64_t type = 0;
	char write_buf[MAX_BUF_LEGTH] = {0};

	if (len > MAX_BUF_LEGTH || buff == NULL || filp == NULL) {
		dev_info(&client_temp->dev, "%s: bad value\n", __func__);
		return len;
	}

	data = (struct drv2605_data *)filp->private_data;
	if (data == NULL) {
		dev_info(&client_temp->dev, "%s:data error\n", __func__);
		return len;
	}

	mutex_lock(&data->lock);

	if (copy_from_user(write_buf, buff, len)) {
		dev_info(&(data->client->dev), "%s: copy_from_user failed\n",
			__func__);
		goto out;
	}

	if (kstrtoll(write_buf, 10, &type)) {
		dev_err(&(data->client->dev), "%s: read value error\n",
			__func__);
		goto out;
	}

	if (type == HAPTIC_STOP) {
		data->should_stop = YES;
		cancel_work_sync(&data->work);
		vibrator_off(data);
		goto out;
	}

	for (i = 0; i < ARRAY_SIZE(haptics_table); i++) {
		if (type == haptics_table[i].haptics_type) {
			table_num = i;
			dev_info(&(data->client->dev),
				"%s: type:%llu,table_num:%d\n",
				__func__, type, table_num);
			break;
		}
	}
	if (i >= ARRAY_SIZE(haptics_table)) {
		dev_info(&(data->client->dev), "%s: undefined type:%lld\n",
			__func__, type);
		goto out;
	}

	/* start haptic */
	data->should_stop = YES;
	cancel_work_sync(&data->work);
	vibrator_off(data);
	memcpy(&data->sequence, &haptics_table[table_num].haptics_value,
		HAPTICS_NUM);
	data->should_stop = NO;
	schedule_work(&data->work_play_eff);

out:
	mutex_unlock(&data->lock);
	return len;
}

static const struct file_operations fops = {
	.open = haptics_open,
	.write = haptics_write,
};

static int haptics_init(struct drv2605_data *data)
{
	int ret;

	data->version = MKDEV(0, 0);
	ret = alloc_chrdev_region(&data->version, 0, 1, CDEVIE_NAME);
	if (ret < 0) {
		dev_info(&(data->client->dev), "%s: error major number %d\n",
			__func__, ret);
		return ret;
	}

	data->class = class_create(THIS_MODULE, CDEVIE_NAME);
	if (data->class == NULL) {
		dev_info(&(data->client->dev), "%s: error creating class\n",
			__func__);
		goto unregister_cdev_region;
	}

	data->device = device_create(data->class, NULL,
				     data->version, NULL, CDEVIE_NAME);
	if (data->device == NULL) {
		dev_info(&(data->client->dev), "%s: error creating device\n",
			__func__);
		goto destory_class;
	}

	cdev_init(&data->cdev, &fops);
	data->cdev.owner = THIS_MODULE;
	data->cdev.ops = &fops;
	ret = cdev_add(&data->cdev, data->version, 1);
	if (ret != 0) {
		dev_info(&(data->client->dev), "%s: fail to add cdev\n",
			__func__);
		goto destory_device;
	}

	data->sw_dev.name = "haptics";
	ret = switch_dev_register(&data->sw_dev);
	if (ret < 0) {
		dev_info(&(data->client->dev), "%s: fail to register switch\n",
			__func__);
		goto unregister_switch_dev;
	}

	INIT_WORK(&data->work_play_eff, play_effect);

	return 0;

unregister_switch_dev:
	switch_dev_unregister(&data->sw_dev);
destory_device:
	device_destroy(data->class, data->version);
destory_class:
	class_destroy(data->class);
unregister_cdev_region:
	unregister_chrdev_region(data->version, 1);

	return ret;
}

static void drv2605_reg_int(void)
{
	uint8_t lra_mode;

#if SKIP_LRA_AUTOCAL == 1
	if (g_effect_bank != LIBRARY_F)
		drv2605_write_reg_val(data->client, erm_autocal_sequence,
			sizeof(erm_autocal_sequence));
	else
		drv2605_write_reg_val(data->client, lra_init_sequence,
			sizeof(lra_init_sequence));
#else
	if (g_effect_bank == LIBRARY_F)
		drv2605_write_reg_val(data->client, lra_autocal_sequence,
			sizeof(lra_autocal_sequence));
	else
		drv2605_write_reg_val(data->client, erm_autocal_sequence,
			sizeof(erm_autocal_sequence));
#endif

	/* write lra_mode */
	lra_mode = drv2605_read_reg(data->client, FEEDBACK_CONTROL_REG);
	lra_mode = ((0x03 & lra_mode) | 0xFC) & 0xD3;
	dev_info(&client_temp->dev, "lra_mode:0x%x\n", lra_mode);
	drv2605_i2c_write(data->client, FEEDBACK_CONTROL_REG, lra_mode);

	/* set voltage */
	drv2605_i2c_write(data->client, RATED_VOLTAGE_REG,
		pdata->lra_rated_voltage);
	if (runmode_is_factory())
		drv2605_i2c_write(data->client, OVERDRIVE_CLAMP_VOLTAGE_REG,
			FACTORY_VOLTAGE_VALUE);
	else
		drv2605_i2c_write(data->client, OVERDRIVE_CLAMP_VOLTAGE_REG,
			pdata->lra_overdriver_voltage);

	/* Choose default effect library */
	drv2605_select_library(data->client, g_effect_bank);
	drv2605_change_mode(data->client, MODE_STANDBY);
}

static int drv2605_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int ret;
	struct led_classdev *cclassdev = NULL;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c is not supported\n");
		return -EIO;
	}

	client_temp = client;
	if (client->dev.of_node == NULL)
		return -EINVAL;

	pdata = devm_kzalloc(&client->dev, sizeof(*pdata), GFP_KERNEL);
	if (pdata == NULL)
		return -ENOMEM;

	/* parse DT */
	ret = drv2605_parse_dt(&client->dev, pdata);
	if (ret != 0) {
		devm_kfree(&client->dev, pdata);
		dev_err(&client->dev, "DT parsing failed\n");
		return -EIO;
	}

	data = devm_kzalloc(&client->dev, sizeof(*data), GFP_KERNEL);
	if (data == NULL) {
		devm_kfree(&client->dev, pdata);
		return -ENOMEM;
	}

	wakeup_source_init(&vib_wakelock, "vib_drv2605");
	mutex_init(&data->lock);
	mutex_lock(&data->lock);

	data->gpio_enable = pdata->gpio_enable;
	data->max_timeout_ms = pdata->max_timeout_ms;
	data->reduce_timeout_ms = pdata->reduce_timeout_ms;
	rtp_strength = pdata->lra_rtp_strength;
	dev_info(&client->dev, "rtp_strength:0x%x\n", rtp_strength);

	i2c_set_clientdata(client, data);

	data->client = client;
	data->device = &(client->dev);
	if (gpio_request(data->gpio_enable, "vibrator-en") < 0) {
		dev_err(&client->dev,
			"drv2605: error requesting enable gpio\n");
		goto destroy_mutex;
	}

	INIT_WORK(&data->work, vibrator_work);
	INIT_WORK(&data->work_play_eff, play_effect);
	INIT_WORK(&data->work_enable, vibrator_work_enable);

	data->cclassdev.name = pdata->name;
	cclassdev = &(data->cclassdev);
	cclassdev->name = "vibrator";
	cclassdev->flags = LED_CORE_SUSPENDRESUME;
	cclassdev->brightness_set = vibrator_enable;
	cclassdev->default_trigger = "transient";
	mutex_unlock(&data->lock);

	ret = devm_led_classdev_register(data->device, cclassdev);
	if (ret != 0) {
		dev_err(&client->dev, "unable to register with led_classdev\n");
		goto unregister_led_classdev;
	}

	ret = sysfs_create_group(&data->cclassdev.dev->kobj, &vb_attr_group);
	if (ret != 0) {
		dev_err(&client->dev, "unable create vibrator's sysfs\n");
		goto unregister_led_classdev;
	}

	/* enable the drv2605 chip wait 30 us for enbale succ */
	(void)gpio_direction_output(data->gpio_enable, GPIO_LEVEL_HIGH);
	udelay(30); // delay 30 us

	/* init */
	drv2605_reg_int();
	haptics_init(data);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_VIBRATOR_LRA);
#endif

#ifdef CONFIG_HUAWEI_DSM
#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
	if (!vib_dclient) {
		vib_dclient = dsm_register_client(&dsm_vibrator);
		dev_info(&client->dev, "drv2605 dsm register success\n");
	}
#endif
#endif

	dev_info(&client->dev, "drv2605 probe succeeded\n");
	return 0;

unregister_led_classdev:
	gpio_free(data->gpio_enable);
destroy_mutex:
	mutex_destroy(&data->lock);
	wakeup_source_trash(&vib_wakelock);
	devm_kfree(&client->dev, data);
	devm_kfree(&client->dev, pdata);
	return ret;
}

static int drv2605_remove(struct i2c_client *client)
{
	struct drv2605_data *data = i2c_get_clientdata(client);

	wakeup_source_trash(&vib_wakelock);
	mutex_destroy(&data->lock);
	sysfs_remove_group(&data->cclassdev.dev->kobj, &vb_attr_group);
	led_classdev_unregister(&data->cclassdev);
	cancel_work_sync(&data->work);
	cancel_work_sync(&data->work_play_eff);
	cancel_work_sync(&data->work_enable);
	return 0;
}

static const struct i2c_device_id drv2605_id_table[] = {
	{ "drv2605", 0 },
	{},
};

MODULE_DEVICE_TABLE(i2c, drv2605_id_table);

static const struct of_device_id drv2605_of_id_table[] = {
	{ .compatible = "ti,drv2605" },
	{},
};

static struct i2c_driver drv2605_i2c_driver = {
	.driver = {
		.name = "drv2605",
		.owner = THIS_MODULE,
		.of_match_table = drv2605_of_id_table,
	},
	.probe = drv2605_probe,
	.remove = drv2605_remove,
	.id_table = drv2605_id_table,
};

module_i2c_driver(drv2605_i2c_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("drv2605 driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
