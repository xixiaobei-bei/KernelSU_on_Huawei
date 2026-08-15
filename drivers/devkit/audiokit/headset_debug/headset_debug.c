/*
 * headset_debug.c
 *
 * headset debug driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include <linux/module.h>
#ifdef CONFIG_HUAWEI_HEADSET_DEBUG_SWITCH
#include <linux/switch.h>
#endif
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <sound/soc.h>
#include <sound/jack.h>
#include <huawei_platform/log/hw_log.h>
#include "headset_debug.h"

#define HWLOG_TAG headset_debug
HWLOG_REGIST();

#define INT_HEX_STR_SIZE 5

#define HEADSET_DEBUG_ALL_KEY_TYPE \
	(SND_JACK_BTN_0 | SND_JACK_BTN_1 | SND_JACK_BTN_2 | \
	SND_JACK_BTN_3 | SND_JACK_BTN_4 | SND_JACK_BTN_5)
#define HEADSET_DEBUG_KEY_TIME_INTERVAL 30          // ms

struct headset_debug {
	struct snd_soc_jack *jack;
#ifdef CONFIG_HUAWEI_HEADSET_DEBUG_SWITCH
	struct switch_dev *sdev;
#endif
	struct input_dev *input_dev;
	struct dentry *df_dir;
	atomic_t state;
	bool use_snd_jack_report;
};

static struct headset_debug g_headset_debug;

void headset_debug_set_state(int state, bool use_snd_jack_input_dev)
{
	// hisi: use snd_soc_jack_report(),
	//       but not defined CONFIG_SND_JACK_INPUT_DEV
	// mtk: not use snd_soc_jack_report()
	if (!use_snd_jack_input_dev || !g_headset_debug.use_snd_jack_report) {
		atomic_set(&g_headset_debug.state, state);
		return;
	}

	// qcom: use snd_soc_jack_report(),
	//       and defined CONFIG_SND_JACK_INPUT_DEV
	switch (state) {
	case 0:
		atomic_set(&g_headset_debug.state, HEADSET_DEBUG_JACK_BIT_NONE);
		break;
	case SND_JACK_HEADPHONE:
		atomic_set(&g_headset_debug.state,
				HEADSET_DEBUG_JACK_BIT_HEADSET_NO_MIC);
		break;
	case SND_JACK_HEADSET:
		atomic_set(&g_headset_debug.state,
				HEADSET_DEBUG_JACK_BIT_HEADSET);
		break;
	case SND_JACK_LINEOUT:
		atomic_set(&g_headset_debug.state,
				HEADSET_DEBUG_JACK_BIT_LINEOUT);
		break;
	default:
		hwlog_err("state error %d\n", state);
		break;
	}
}
EXPORT_SYMBOL_GPL(headset_debug_set_state);

void headset_debug_input_set_state(int state, bool use_snd_jack_input_dev)
{
	headset_debug_set_state(state, use_snd_jack_input_dev);
}
EXPORT_SYMBOL_GPL(headset_debug_input_set_state);

static ssize_t headset_debug_state_read(struct file *file,
		char __user *user_buf, size_t count,
		loff_t *ppos)
{
	char kn_buf[INT_HEX_STR_SIZE] = { 0 };
	ssize_t byte_read;

	snprintf(kn_buf, INT_HEX_STR_SIZE, "%d",
			atomic_read(&g_headset_debug.state));
	byte_read = simple_read_from_buffer(user_buf, count, ppos,
			kn_buf, strlen(kn_buf));

	return byte_read;
}

static int headset_debug_convert_jack_type(int state)
{
	int type = 0;

	switch (state) {
	case HEADSET_DEBUG_JACK_BIT_NONE:
		type = 0;
		break;
	case HEADSET_DEBUG_JACK_BIT_HEADSET:
		type = SND_JACK_HEADSET;
		break;
	case HEADSET_DEBUG_JACK_BIT_HEADSET_NO_MIC:
		type = SND_JACK_HEADPHONE;
		break;
	case HEADSET_DEBUG_JACK_BIT_HEADPHONE:
		type = SND_JACK_HEADPHONE;
		break;
	case HEADSET_DEBUG_JACK_BIT_LINEOUT:
		type = SND_JACK_LINEOUT;
		break;
	default:
		hwlog_err("error hs_status %d\n", state);
	}
	return type;
}

static int headset_debug_convert_key_value(int status)
{
	int value = 0;

	if (g_headset_debug.use_snd_jack_report) { // hisi or qcom
		switch (status) {
		case HEADSET_DEBUG_KEY_0:
			value = SND_JACK_BTN_0;
			break;
		case HEADSET_DEBUG_KEY_1:
			value = SND_JACK_BTN_1;
			break;
		case HEADSET_DEBUG_KEY_2:
			value = SND_JACK_BTN_2;
			break;
		case HEADSET_DEBUG_KEY_3:
			value = SND_JACK_BTN_3;
			break;
		case HEADSET_DEBUG_KEY_4:
			value = SND_JACK_BTN_4;
			break;
		case HEADSET_DEBUG_KEY_5:
			value = SND_JACK_BTN_5;
			break;
		default:
			hwlog_err("error code %d\n", status);
			break;
		}
	} else { // mtk
		switch (status) {
		case HEADSET_DEBUG_KEY_0:
			value = KEY_MEDIA;
			break;
		case HEADSET_DEBUG_KEY_1:
			value = KEY_VOICECOMMAND;
			break;
		case HEADSET_DEBUG_KEY_2:
			value = KEY_VOLUMEUP;
			break;
		case HEADSET_DEBUG_KEY_3:
			value = KEY_VOLUMEDOWN;
			break;
		default:
			hwlog_err("error code %d\n", status);
			break;
		}
	}

	return value;
}

static void headset_debug_jack_report(int status)
{
	int jack_report = headset_debug_convert_jack_type(status);

	atomic_set(&g_headset_debug.state, status);
	if (g_headset_debug.use_snd_jack_report) { // hisi or qcom
#ifdef CONFIG_HUAWEI_HEADSET_DEBUG_SWITCH
		switch_set_state(g_headset_debug.sdev, status);
#endif
		snd_soc_jack_report(g_headset_debug.jack, jack_report,
				SND_JACK_HEADSET);
	} else { // mtk
		input_report_switch(g_headset_debug.input_dev,
				SW_MICROPHONE_INSERT, 0);
		input_report_switch(g_headset_debug.input_dev,
				SW_HEADPHONE_INSERT, 0);
		input_report_switch(g_headset_debug.input_dev,
				SW_LINEOUT_INSERT, 0);
		input_sync(g_headset_debug.input_dev);

		if (jack_report == 0)
			return;

		if (jack_report & SND_JACK_MICROPHONE) {
			input_report_switch(g_headset_debug.input_dev,
					SW_MICROPHONE_INSERT, 1);
		}
		if (jack_report & SND_JACK_HEADPHONE) {
			input_report_switch(g_headset_debug.input_dev,
					SW_HEADPHONE_INSERT, 1);
		}
		if (jack_report & SND_JACK_LINEOUT) {
			input_report_switch(g_headset_debug.input_dev,
					SW_LINEOUT_INSERT, 1);
		}
		input_sync(g_headset_debug.input_dev);
	}
}

static void headset_debug_key_report(int status)
{
	int value = headset_debug_convert_key_value(status);

	if (value == 0)
		return;

	if (g_headset_debug.use_snd_jack_report) {
		snd_soc_jack_report(g_headset_debug.jack, value,
				HEADSET_DEBUG_ALL_KEY_TYPE);
		msleep(HEADSET_DEBUG_KEY_TIME_INTERVAL);
		snd_soc_jack_report(g_headset_debug.jack, 0,
				HEADSET_DEBUG_ALL_KEY_TYPE);
	} else {
		// value: 1 key press, 0 key release
		input_report_key(g_headset_debug.input_dev, value, 1);
		input_sync(g_headset_debug.input_dev);
		msleep(HEADSET_DEBUG_KEY_TIME_INTERVAL);
		input_report_key(g_headset_debug.input_dev, value, 0);
		input_sync(g_headset_debug.input_dev);
	}
}

static ssize_t headset_debug_state_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	char kn_buf[INT_HEX_STR_SIZE] = { 0 };
	ssize_t byte_writen;
	int status;
	int ret;

	byte_writen = simple_write_to_buffer(kn_buf,
			INT_HEX_STR_SIZE - 1, ppos,
			user_buf, count);
	if (byte_writen != count) {
		hwlog_err("simple_write_to_buffer err:%zd\n", byte_writen);
		return -ENOMEM;
	}

	ret = kstrtouint(kn_buf, 0, &status);
	if (ret) {
		hwlog_err("kstrtouint error %d\n", ret);
		return -EINVAL;
	}
	switch (status) {
	case HEADSET_DEBUG_JACK_BIT_PLUGING:
	case HEADSET_DEBUG_JACK_BIT_INVALID:
		if (!g_headset_debug.use_snd_jack_report)
			break;
		// fall through
	case HEADSET_DEBUG_JACK_BIT_NONE:
	case HEADSET_DEBUG_JACK_BIT_HEADSET:
	case HEADSET_DEBUG_JACK_BIT_HEADPHONE:
	case HEADSET_DEBUG_JACK_BIT_HEADSET_NO_MIC:
	case HEADSET_DEBUG_JACK_BIT_LINEOUT:
		headset_debug_jack_report(status);
		break;
	case HEADSET_DEBUG_KEY_0:
	case HEADSET_DEBUG_KEY_1:
	case HEADSET_DEBUG_KEY_2:
	case HEADSET_DEBUG_KEY_3:
	case HEADSET_DEBUG_KEY_4:
	case HEADSET_DEBUG_KEY_5:
		headset_debug_key_report(status);
		break;
	default:
		hwlog_err("error code %d\n", status);
		break;
	}

	return byte_writen;
}

/*lint -e785 */
static const struct file_operations headset_debug_state_fops = {
	.read = headset_debug_state_read,
	.write = headset_debug_state_write,
};

static void headset_debug_init_fs(void)
{
	g_headset_debug.df_dir = debugfs_create_dir("headset", NULL);
	if (!g_headset_debug.df_dir) {
		hwlog_err("create headset debugfs dir\n");
		return;
	}

	if (!debugfs_create_file("state", 0640, g_headset_debug.df_dir,
				NULL, &headset_debug_state_fops)) {
		hwlog_err("create headset debugfs file\n");
		debugfs_remove_recursive(g_headset_debug.df_dir);
		return;
	}
	atomic_set(&g_headset_debug.state, 0);
}

#ifdef CONFIG_HUAWEI_HEADSET_DEBUG_SWITCH
void headset_debug_init(struct snd_soc_jack *jack, struct switch_dev *sdev)
{
	if (jack == NULL || sdev == NULL) {
		hwlog_err("jack or sdev is NULL\n");
		return;
	}
	g_headset_debug.use_snd_jack_report = true;
	g_headset_debug.jack = jack;
	g_headset_debug.sdev = sdev;

	headset_debug_init_fs();
}
#else
void headset_debug_init(struct snd_soc_jack *jack)
{
	if (jack == NULL) {
		hwlog_err("jack is NULL\n");
		return;
	}
	g_headset_debug.use_snd_jack_report = true;
	g_headset_debug.jack = jack;

	headset_debug_init_fs();
}
EXPORT_SYMBOL_GPL(headset_debug_init);
#endif

void headset_debug_uninit(void)
{
	// debugfs_remove_recursive(NULL) is safe, check not required
	debugfs_remove_recursive(g_headset_debug.df_dir);
	g_headset_debug.df_dir = NULL;
	g_headset_debug.jack = NULL;
#ifdef CONFIG_HUAWEI_HEADSET_DEBUG_SWITCH
	g_headset_debug.sdev = NULL;
#endif
}
EXPORT_SYMBOL_GPL(headset_debug_uninit);

void headset_debug_input_init(struct input_dev *accdet_input_dev)
{
	if (accdet_input_dev == NULL) {
		hwlog_err("accdet_input_dev is NULL\n");
		return;
	}
	g_headset_debug.use_snd_jack_report = false;
	g_headset_debug.input_dev = accdet_input_dev;

	headset_debug_init_fs();
}
EXPORT_SYMBOL_GPL(headset_debug_input_init);

void headset_debug_input_uninit(void)
{
	headset_debug_uninit();
}
EXPORT_SYMBOL_GPL(headset_debug_input_uninit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("headset debug driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
