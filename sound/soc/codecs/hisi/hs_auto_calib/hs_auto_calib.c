/*
 * hs_auto_calib.c
 *
 * hs_auto_calib driver
 *
 * Copyright (c) 2016-2020 Huawei Technologies Co., Ltd.
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

#include "hs_auto_calib.h"

#include <linux/hisi/audio_log.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <sound/jack.h>

#define LOG_TAG "hs_auto_calib"

#define of_key_value_get(np, name, value, def_val) \
	do { \
		if (of_property_read_u32(np, name, value) != 0) { \
			*value = def_val; \
			AUDIO_LOGI("param_name is %d", *value); \
		} \
	} while (0)

#define DEFAULT_ERROR 35

struct calib_btn_voltage {
	unsigned short key_up_min_value;
	unsigned short key_up_max_value;
	unsigned int key_play_min_value;
	unsigned int key_play_max_value;
	unsigned int key_forward_min_value;
	unsigned int key_forward_max_value;
	unsigned int key_back_min_value;
	unsigned int key_back_max_value;
	unsigned short key_3_pole_min_voltage;
	unsigned short key_3_pole_max_voltage;
	unsigned short key_4_pole_min_voltage;
	unsigned short key_4_pole_max_voltage;
};

enum vol_interzone_type {
	HOOK = 0,
	FORWARD,
	BACK,
	DEFAULT_TYPE,
	KEY_TYPE_MAX = DEFAULT_TYPE
};

enum auto_btn_voltage {
	KEY_HOOK_MIN_VALUE = 0, /* mv */
	KEY_HOOK_MAX_VALUE = 135,
	KEY_FORWARD_MIN_VALUE = 136,
	KEY_FORWARD_MAX_VALUE = 350,
	KEY_BACK_MIN_VALUE = 351,
	KEY_BACK_MAX_VALUE = 700
};

enum {
	KEY_BACK_RANGE = 1,
	KEY_FORWARD_RANGE = 2
};

static bool g_hs_auto_calib_enable;
static int g_vol_record[KEY_TYPE_MAX] = {-1};
static struct calib_btn_voltage g_btn_voltage = {0};
static int g_vol_error[KEY_TYPE_MAX] = {DEFAULT_ERROR,
	DEFAULT_ERROR, DEFAULT_ERROR};
static int g_vol_distance = DEFAULT_ERROR;
static int g_vol_btn = FORWARD;

static bool is_vol_record_empty(void)
{
	int i;

	for (i = FORWARD; i < g_vol_btn; i++) {
		if (g_vol_record[i] != -1)
			return false;
	}
	return true;
}

void headset_auto_calib_init(struct device_node *np)
{
	struct device_node *ncp = NULL;
	const char *status = NULL;

	AUDIO_LOGI("begin");
	ncp = of_get_child_by_name(np, "hs_auto_calib");
	if (ncp == NULL)
		goto err;

	if (of_property_read_string(ncp, "status", &status))
		goto err;

	if (!strncmp(status, "ok", strlen("ok")))
		g_hs_auto_calib_enable = true;
	else
		goto err;

	of_key_value_get(ncp, "key_play_min_value",
		&g_btn_voltage.key_play_min_value, KEY_HOOK_MIN_VALUE);
	of_key_value_get(ncp, "key_play_max_value",
		&g_btn_voltage.key_play_max_value, KEY_HOOK_MAX_VALUE);
	of_key_value_get(ncp, "key_forward_min_value",
		&g_btn_voltage.key_forward_min_value, KEY_FORWARD_MIN_VALUE);
	of_key_value_get(ncp, "key_forward_max_value",
		&g_btn_voltage.key_forward_max_value, KEY_FORWARD_MAX_VALUE);
	of_key_value_get(ncp, "key_back_min_value",
		&g_btn_voltage.key_back_min_value, KEY_BACK_MIN_VALUE);
	of_key_value_get(ncp, "key_back_max_value",
		&g_btn_voltage.key_back_max_value, KEY_BACK_MAX_VALUE);
	AUDIO_LOGI("end");
	return;

err:
	g_hs_auto_calib_enable = false;
}

void headset_auto_calib_reset_interzone(void)
{
	int i;

	AUDIO_LOGI("begin");
	for (i = FORWARD; i < KEY_TYPE_MAX; i++) {
		g_vol_record[i] = -1;
		g_vol_error[i] = DEFAULT_ERROR;
	}

	g_vol_btn = FORWARD;
	AUDIO_LOGI("end");
}

static enum vol_interzone_type check_hs_type(unsigned int hkadc_value)
{
	enum vol_interzone_type press_key = DEFAULT_TYPE;

	AUDIO_LOGI("begin, hkadc_value %u", hkadc_value);
	if ((hkadc_value >= g_btn_voltage.key_play_min_value) &&
		(hkadc_value <= g_btn_voltage.key_play_max_value))
		press_key = HOOK;
	else if ((hkadc_value >= g_btn_voltage.key_forward_min_value) &&
		(hkadc_value <= g_btn_voltage.key_forward_max_value))
		press_key = FORWARD;
	else if ((hkadc_value >= g_btn_voltage.key_back_min_value) &&
		(hkadc_value <= g_btn_voltage.key_back_max_value))
		press_key = BACK;
	else
		AUDIO_LOGI("hkadc value is not in range, hkadc value is %u",
			hkadc_value);

	AUDIO_LOGI("end press_key %d", press_key);
	return press_key;
}

static void set_hs_type(int *btn_type, int press_key)
{
	AUDIO_LOGI("begin, btn_type %d", *btn_type);
	if (press_key != DEFAULT_TYPE)
		*btn_type = (int)(SND_JACK_BTN_0 >> press_key);
	else
		*btn_type = 0;
	AUDIO_LOGI("end");
}

static void store_error(void)
{
	int temp;
	int vol_distance;

	AUDIO_LOGI("begin g_vol_error[FORWARD] %d g_vol_error[BACK] %d",
		g_vol_error[FORWARD], g_vol_error[BACK]);
	vol_distance = g_vol_record[BACK] - g_vol_record[FORWARD];
	g_vol_distance = (g_vol_distance > vol_distance) ? g_vol_distance :
		vol_distance;
	g_vol_error[BACK] = g_vol_distance >> 1;
	temp = (g_vol_record[FORWARD] - g_btn_voltage.key_play_max_value) >> 1;
	g_vol_error[FORWARD] =
		(g_vol_error[BACK] > temp) ? temp : g_vol_error[BACK];
	g_vol_error[BACK] = g_vol_distance - g_vol_error[FORWARD];
	AUDIO_LOGI("end g_vol_error[FORWARD] %d g_vol_error[BACK] %d",
		g_vol_error[FORWARD], g_vol_error[BACK]);
}

static void record_vol(unsigned int hkadc_value)
{
	int temp;

	AUDIO_LOGI("hkadc_value %u", hkadc_value);
	if (hkadc_value < g_btn_voltage.key_play_max_value) {
		g_vol_record[HOOK] = hkadc_value;
		return;
	}

	switch (g_vol_btn) {
	case FORWARD:
		g_vol_record[FORWARD] = hkadc_value;
		g_vol_btn = BACK;
		break;

	case BACK:
		if ((int)hkadc_value > g_vol_record[FORWARD]) {
			g_vol_record[BACK] = hkadc_value;
		} else {
			temp = g_vol_record[FORWARD];
			g_vol_record[FORWARD] = hkadc_value;
			g_vol_record[BACK] = temp;
		}
		g_vol_btn = KEY_TYPE_MAX;
		store_error();
		break;

	case KEY_TYPE_MAX:
		/* discard error */
		if ((int)hkadc_value < g_vol_record[FORWARD])
			g_vol_record[FORWARD] = hkadc_value;
		else if ((int)hkadc_value > g_vol_record[BACK])
			g_vol_record[BACK] = hkadc_value;

		store_error();
		break;

	default:
		break;
	}
	AUDIO_LOGI("end");
}

/*
 * normally, this function will be called when unpressed key has been pressed.
 * Exceptionally, this function will be called
 * when a button error is out of range.
 */
static void readjust_interzone(void)
{
	AUDIO_LOGI("begin");
	g_btn_voltage.key_forward_max_value =
		(g_vol_record[FORWARD] + g_vol_record[BACK]) /
			KEY_FORWARD_RANGE;
	g_btn_voltage.key_back_min_value =
		g_btn_voltage.key_forward_max_value + KEY_BACK_RANGE;
	AUDIO_LOGI("interzone reset to key_forward_max = %d , key_back_min = %d",
		g_btn_voltage.key_forward_max_value,
		g_btn_voltage.key_back_min_value);
	AUDIO_LOGI("end");
}

/*
 * g_vol_record compare with it's g_vol_error.
 * return false means the voltage not belonging to error range,
 * return true means the voltage is error.
 */
static bool is_voltage_error(unsigned int hkadc_value)
{
	int i;

	AUDIO_LOGI("begin, hkadc_value %u", hkadc_value);
	/* has this button being pressed */
	for (i = (int)FORWARD; i < g_vol_btn; i++) {
		if (abs((int)hkadc_value - g_vol_record[i]) < g_vol_error[i])
			return true;
	}

	AUDIO_LOGI("end");
	return false;
}

static void get_adjust_state(enum adjust_state state,
	unsigned int hkadc_value, int *pr_btn_type)
{
	/* the process of acquiring adjust_state until the end of BTN_REPORT */
	while (true) {
		int headset_type = 0;

		switch (state) {
		case REC_JUDGE:
			/*
			 * if all record is empty goto report,
			 * else ERROR_JUDGE
			 */
			if (is_vol_record_empty())
				state = BTN_RECORD;
			else
				state = ERROR_JUDGE;
			break;

		case ERROR_JUDGE:
			/* avoid the hardware feedback error */
			if (is_voltage_error(hkadc_value))
				state = BTN_REPORT;
			else
				state = BTN_RECORD;
			break;

		case BTN_RECORD:
			/* record vol and modify g_vol_error g_vol_record */
			record_vol(hkadc_value);
			if (g_vol_btn == KEY_TYPE_MAX)
				state = VOL_INTERZONE_READJUST;
			else
				state = BTN_REPORT;
			break;

		case VOL_INTERZONE_READJUST:
			/* interzone judge and re-adjust the threshold */
			readjust_interzone();
			state = BTN_REPORT;
			break;

		case BTN_REPORT:
			headset_type = (int)check_hs_type(hkadc_value);
			set_hs_type((int *)pr_btn_type, headset_type);
			AUDIO_LOGI("hkadc_value %u BTN_REPORT %d",
				hkadc_value, headset_type);
			return;

		default:
			break;
		}
	}
}

void startup_fsm(enum adjust_state state, unsigned int hkadc_value, int *pr_btn_type)
{
	if (!g_hs_auto_calib_enable)
		return;

	if (hkadc_value > g_btn_voltage.key_back_max_value) {
		AUDIO_LOGE("error:hkadc_value %u is bigger than max value",
			hkadc_value);
		return;
	}

	AUDIO_LOGI("begin, adjust_state %d hkadc_value %u", state, hkadc_value);
	get_adjust_state(state, hkadc_value, pr_btn_type);
}

