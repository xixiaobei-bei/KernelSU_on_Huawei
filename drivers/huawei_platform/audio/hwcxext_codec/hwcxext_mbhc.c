/*
 * hwcxext_mbhc.c
 *
 * hwcxext mbhc driver
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/pm_wakeirq.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/input/matrix_keypad.h>
#include <sound/jack.h>

#include <huawei_platform/log/hw_log.h>

#include "ana_hs_kit/ana_hs.h"

#include "hwcxext_codec_info.h"
#include "hwcxext_mbhc.h"

#define HWLOG_TAG hwcxext_mbhc
HWLOG_REGIST();

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

#define IN_FUNCTION   hwlog_info("%s function comein\n", __func__)
#define OUT_FUNCTION  hwlog_info("%s function comeout\n", __func__)

#define HS_LONG_BTN_PRESS_LIMIT_TIME 6000
#define HS_REPEAT_DETECT_PLUG_TIME_MS 3000 // poll 3s
#define HS_BTN_IRQ_UNHANDLE_TIME_MS 100
#define HS_REPEAT_WAIT_CNT 1
#define HS_REPEAT_DELAY_TIME 15

#define HWCXEXT_MBHC_JACK_BUTTON_MASK (SND_JACK_BTN_0 | SND_JACK_BTN_1 | \
	SND_JACK_BTN_2)

struct jack_key_to_type {
	enum snd_jack_types type;
	int key;
};

enum btn_aready_press_status {
	BTN_AREADY_UP = 0,
	BTN_AREADY_PRESS,
};

enum {
	MICB_DISABLE,
	MICB_ENABLE,
};

enum {
	BTN_IRQ_DISABLE,
	BTN_IRQ_ENABLE,
};

enum {
	JSENSE_PLUGIN_HANDLE,
	JSENSE_PLUGOUT_HANDLE,
};

static void hwcxext_mbhc_plug_in_detect(
	struct hwcxext_mbhc_priv *mbhc_data);
static bool hwcxext_mbhc_check_headset_in(void *priv);

static int hwcxext_mbhc_cancel_long_btn_work(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	int ret;

	/* it can not use cancel_delayed_work_sync, otherwise lead deadlock */
	ret = cancel_delayed_work(&mbhc_data->long_press_btn_work);
	if (ret)
		hwlog_info("%s: cancel long btn work\n", __func__);

	return ret;
}

static bool hwcxext_mbhc_check_headset_in(void *priv)
{
	struct hwcxext_mbhc_priv *mbhc_data = priv;

	if (priv == NULL) {
		hwlog_err("%s: priv is null", __func__);
		return false;
	}

	if (ana_hs_support_usb_sw()) {
		if (ana_hs_pluged_state() == ANA_HS_PLUG_IN) {
			hwlog_info("%s: ananlog hs is plug in", __func__);
			return true;
		}

		hwlog_info("%s: ananlog hs is plug out", __func__);
		return false;
	}

	if (mbhc_data->mbhc_cb->mbhc_check_headset_in)
		return mbhc_data->mbhc_cb->mbhc_check_headset_in(mbhc_data);

	return false;
}

static void hwcxext_mbhc_btn_irq_control(struct hwcxext_mbhc_priv *mbhc_data,
	int req)
{
	int btn_irq_vote_cnt;

	if (!ana_hs_support_usb_sw())
		return;

	mutex_lock(&mbhc_data->btn_irq_vote_mutex);
	btn_irq_vote_cnt = atomic_read(&mbhc_data->btn_irq_vote_cnt);
	hwlog_info("%s: req:%s, btn_irq_vote_cnt:%d\n",
		__func__, req == BTN_IRQ_ENABLE ? "btn_irq enable" :
		"btn_irq disable", btn_irq_vote_cnt);

	if (req == BTN_IRQ_ENABLE) {
		if (btn_irq_vote_cnt)
			goto btn_irq_control_exit;

		atomic_inc(&mbhc_data->btn_irq_vote_cnt);
		hwlog_info("%s: enable btn irq", __func__);
		enable_irq(mbhc_data->plug_btn_irq);
	} else {
		atomic_dec(&mbhc_data->btn_irq_vote_cnt);
		btn_irq_vote_cnt = atomic_read(&mbhc_data->btn_irq_vote_cnt);
		hwlog_info("%s: disable, btn_irq_vote_cnt:%d\n", __func__,
			btn_irq_vote_cnt);
		if (btn_irq_vote_cnt < 0) {
			atomic_set(&mbhc_data->btn_irq_vote_cnt, 0);
		} else if (btn_irq_vote_cnt == 0) {
			hwlog_info("%s: disable btn irq", __func__);
			disable_irq(mbhc_data->plug_btn_irq);
		}
	}
btn_irq_control_exit:
	hwlog_info("%s: out, btn_irq_vote_cnt:%d\n", __func__,
		atomic_read(&mbhc_data->btn_irq_vote_cnt));
	mutex_unlock(&mbhc_data->btn_irq_vote_mutex);
}

static void hwcxext_mbhc_micbias_control(struct hwcxext_mbhc_priv *mbhc_data,
	int req)
{
	int micbias_vote_cnt;

	if (mbhc_data->mbhc_cb->enable_micbias == NULL)
		return;

	mutex_lock(&mbhc_data->micbias_vote_mutex);
	micbias_vote_cnt = atomic_read(&mbhc_data->micbias_vote_cnt);
	hwlog_info("%s: req:%s, micbias_vote_cnt:%d\n",
		__func__, req == MICB_ENABLE ? "micbias enable" :
		"micbias disable", micbias_vote_cnt);
	if (req == MICB_ENABLE) {
		if (micbias_vote_cnt)
			goto mbhc_micbias_control_exit;

		hwlog_info("%s: enable micbias", __func__);
		atomic_inc(&mbhc_data->micbias_vote_cnt);
		mbhc_data->mbhc_cb->enable_micbias(mbhc_data);
		mdelay(10);
	} else {
		atomic_dec(&mbhc_data->micbias_vote_cnt);
		micbias_vote_cnt = atomic_read(&mbhc_data->micbias_vote_cnt);
		hwlog_info("%s: disable micbias micbias_vote_cnt",
			__func__, micbias_vote_cnt);
		if (micbias_vote_cnt < 0) {
			atomic_set(&mbhc_data->micbias_vote_cnt, 0);
		} else if (micbias_vote_cnt == 0) {
			mbhc_data->mbhc_cb->disable_micbias(mbhc_data);
			mdelay(10);
		}
	}

mbhc_micbias_control_exit:
	hwlog_info("%s: out, micbias_vote_cnt:%d\n", __func__,
		atomic_read(&mbhc_data->micbias_vote_cnt));
	mutex_unlock(&mbhc_data->micbias_vote_mutex);
}

static void hwcxext_mbhc_jsense_control(
	struct hwcxext_mbhc_priv *mbhc_data,
	int req)
{
	int jsense_vote_cnt;

	if (mbhc_data->jsense_gpio < 0 ||
		(!gpio_is_valid(mbhc_data->jsense_gpio)))
		return;

	mutex_lock(&mbhc_data->jsense_vote_mutex);
	jsense_vote_cnt = atomic_read(&mbhc_data->jsense_vote_cnt);
	hwlog_info("%s: req:%s, jsense_vote_cnt:%d\n",
		__func__, req == JSENSE_PLUGIN_HANDLE ?
		"jsense_plugin enable" : "jsense_plugout handle",
		jsense_vote_cnt);

	if (req == JSENSE_PLUGIN_HANDLE) {
		if (jsense_vote_cnt)
			goto jsense_control_exit;

		atomic_inc(&mbhc_data->jsense_vote_cnt);
		hwlog_info("%s:set handle plugin", __func__);
		gpio_set_value(mbhc_data->jsense_gpio, 0);
	} else {
		atomic_dec(&mbhc_data->jsense_vote_cnt);
		jsense_vote_cnt = atomic_read(&mbhc_data->jsense_vote_cnt);
		hwlog_info("%s: disable, jsense_vote_cnt:%d\n", __func__,
			jsense_vote_cnt);
		if (jsense_vote_cnt < 0) {
			atomic_set(&mbhc_data->jsense_vote_cnt, 0);
		} else if (jsense_vote_cnt == 0) {
			hwlog_info("%s:set handle plugout", __func__);
			gpio_set_value(mbhc_data->jsense_gpio, 1);
		}
	}
jsense_control_exit:
	hwlog_info("%s: out, jsense_vote_cnt:%d\n", __func__,
		atomic_read(&mbhc_data->jsense_vote_cnt));
	mutex_unlock(&mbhc_data->jsense_vote_mutex);
}

struct btn_type_report {
	const char *info;
	unsigned int btn_type;
	unsigned int report_btn_type;
};

static const struct btn_type_report g_hwcx_btn_map[] = {
	{ "btn hook", BTN_HOOK, SND_JACK_BTN_0 },
	{ "volume up", BTN_UP, SND_JACK_BTN_1 },
	{ "volume down", BTN_DOWN, SND_JACK_BTN_2 },
};

struct btn_envent_report {
	const char *info;
	unsigned int btn_envent_report;
};

static const struct btn_envent_report g_btn_envent_info[] = {
	{ "invaild btn event", BTN_ENENT_INVAILD },
	{ "btn press", BTN_PRESS },
	{ "btn released", BTN_RELEASED },
};

static int hwcxext_get_btn_report(struct hwcxext_mbhc_priv *mbhc_data)
{
	unsigned int i;
	unsigned int size = ARRAY_SIZE(g_hwcx_btn_map);

	for (i = 0; i < size; i++) {
		if (g_hwcx_btn_map[i].btn_type == mbhc_data->get_btn_info.btn_type) {
			hwlog_info("%s: process as %s",
				__func__, g_hwcx_btn_map[i].info);
			mutex_lock(&mbhc_data->status_mutex);
			mbhc_data->btn_report = g_hwcx_btn_map[i].report_btn_type;
			mutex_unlock(&mbhc_data->status_mutex);
			return 0;
		}
	}

	return -EINVAL;
}

static void hwcxext_mbhc_further_detect_plug_type(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	IN_FUNCTION;
	/* further detect */
	msleep(20);
	hwcxext_mbhc_plug_in_detect(mbhc_data);
}

static void hwcxext_mbhc_repeat_detect_plug_work(struct work_struct *work)
{
	unsigned long timeout;
	bool pulg_in = false;
	unsigned int count = 0;
	struct hwcxext_mbhc_priv *mbhc_data = container_of(work,
		struct hwcxext_mbhc_priv, repeat_detect_plug_work.work);

	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is error\n", __func__);
		return;
	}

	if (mbhc_data->mbhc_cb->set_hs_detec_restart == NULL) {
		hwlog_err("%s: set_hs_detec_restart is NULL\n", __func__);
		return;
	}

	__pm_stay_awake(&mbhc_data->plug_repeat_detect_lock);
	IN_FUNCTION;
	timeout = jiffies + msecs_to_jiffies(HS_REPEAT_DETECT_PLUG_TIME_MS);
	while (!time_after(jiffies, timeout)) {
		hwlog_info("%s: prepeat detect time:%d\n", __func__, count);
		mbhc_data->mbhc_cb->set_hs_detec_restart(mbhc_data,
			HS_DETECT_RESTART_BEGIN);
		msleep(HS_REPEAT_DELAY_TIME);
		pulg_in = hwcxext_mbhc_check_headset_in(mbhc_data);
		if (!pulg_in) {
			hwlog_info("%s: plug out happens\n", __func__);
			goto repeat_detect_plug_exit;
		}
		hwcxext_mbhc_plug_in_detect(mbhc_data);
		mbhc_data->mbhc_cb->set_hs_detec_restart(mbhc_data,
			HS_DETECT_RESTART_END);
		count++;
		if (count >= HS_REPEAT_WAIT_CNT)
			break;

		msleep(HS_REPEAT_DELAY_TIME);
	}

repeat_detect_plug_exit:
	__pm_relax(&mbhc_data->plug_repeat_detect_lock);
	OUT_FUNCTION;
}

static void hwevext_mbhc_start_repeat_detect(struct hwcxext_mbhc_priv *mbhc_data)
{
	bool pulg_in = false;

	IN_FUNCTION;
	pulg_in = hwcxext_mbhc_check_headset_in(mbhc_data);
	if (!pulg_in) {
		hwlog_info("%s: plug out happens\n", __func__);
		return;
	}

	queue_delayed_work(mbhc_data->repeat_detect_plug_wq,
		&mbhc_data->repeat_detect_plug_work,
		0);
	OUT_FUNCTION;
}

static void hwcxext_mbhc_long_press_btn_work(struct work_struct *work)
{
	struct hwcxext_mbhc_priv *mbhc_data = container_of(work,
		struct hwcxext_mbhc_priv, long_press_btn_work.work);

	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is null\n", __func__);
		return;
	}

	__pm_stay_awake(&mbhc_data->long_btn_wake_lock);
	IN_FUNCTION;
	if (mbhc_data->hs_status != AUDIO_JACK_HEADSET) {
		hwlog_info("%s: it not headset, ignore\n", __func__);
		goto long_press_btn_exit;
	}

	if (mbhc_data->btn_pressed != BTN_AREADY_PRESS) {
		hwlog_info("%s: btn not press, ignore\n", __func__);
		goto long_press_btn_exit;
	}

	if (!mbhc_data->btn_report) {
		hwlog_warn("%s: btn_report:%d is not right\n", __func__,
			mbhc_data->btn_report);
		goto long_press_btn_exit;
	}

	mutex_lock(&mbhc_data->btn_mutex);
	mbhc_data->btn_report = 0;
	snd_soc_jack_report(&mbhc_data->button_jack,
		mbhc_data->btn_report, HWCXEXT_MBHC_JACK_BUTTON_MASK);
	mbhc_data->btn_pressed = BTN_AREADY_UP;
	mutex_unlock(&mbhc_data->btn_mutex);

long_press_btn_exit:
	__pm_relax(&mbhc_data->long_btn_wake_lock);
	OUT_FUNCTION;
}

static void hwcxext_mbhc_long_press_btn_trigger(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	queue_delayed_work(mbhc_data->long_press_btn_wq,
		&mbhc_data->long_press_btn_work,
		msecs_to_jiffies(HS_LONG_BTN_PRESS_LIMIT_TIME));
}

static void hwcxext_mbhc_btn_release_handle(struct hwcxext_mbhc_priv *mbhc_data)
{
	if (!mbhc_data->btn_report) {
		hwlog_warn("%s: btn_report:%d is not right\n", __func__,
			mbhc_data->btn_report);
		return;
	}

	if (mbhc_data->hs_status != AUDIO_JACK_HEADSET) {
		hwlog_info("%s: it not headset, ignore\n", __func__);
		return;
	}

	hwcxext_mbhc_cancel_long_btn_work(mbhc_data);
	mutex_lock(&mbhc_data->status_mutex);
	mbhc_data->btn_report = 0;
	snd_soc_jack_report(&mbhc_data->button_jack,
		mbhc_data->btn_report, HWCXEXT_MBHC_JACK_BUTTON_MASK);
	mbhc_data->btn_pressed = BTN_AREADY_UP;
	mutex_unlock(&mbhc_data->status_mutex);
}

static void hwcxext_mbhc_btn_press_handle(struct hwcxext_mbhc_priv *mbhc_data)
{
	bool pulg_in = false;
	int ret;

	pulg_in = hwcxext_mbhc_check_headset_in(mbhc_data);
	if (!pulg_in) {
		hwlog_info("%s: plug out happens\n", __func__);
		return;
	}

	ret = hwcxext_get_btn_report(mbhc_data);
	if (!ret) {
		mutex_lock(&mbhc_data->status_mutex);
		mbhc_data->btn_pressed = BTN_AREADY_PRESS;
		mutex_unlock(&mbhc_data->status_mutex);
		/* btn report key event */
		hwlog_info("%s: btn press report type: 0x%x, status: %d",
			__func__, mbhc_data->btn_report,
			mbhc_data->hs_status);
		snd_soc_jack_report(&mbhc_data->button_jack,
			mbhc_data->btn_report,
			HWCXEXT_MBHC_JACK_BUTTON_MASK);

		hwcxext_mbhc_long_press_btn_trigger(mbhc_data);
	} else {
		hwlog_warn("%s:it not a button press, further detect\n",
			__func__);
		hwcxext_mbhc_further_detect_plug_type(mbhc_data);
	}
}

static void hwcxext_mbhc_btn_handle(struct hwcxext_mbhc_priv *mbhc_data)
{
	/* btn down */
	if (mbhc_data->get_btn_info.btn_event == BTN_PRESS &&
		(mbhc_data->btn_pressed == BTN_AREADY_UP)) {
		/* button down event */
		hwlog_info("%s:button down event\n", __func__);
		mdelay(20);
		hwcxext_mbhc_btn_press_handle(mbhc_data);
	} else if (mbhc_data->get_btn_info.btn_event == BTN_RELEASED &&
		mbhc_data->btn_pressed == BTN_AREADY_PRESS) {
		/* button up event */
		hwlog_info("%s : btn up report type: 0x%x release\n", __func__,
			mbhc_data->btn_report);
		hwcxext_mbhc_btn_release_handle(mbhc_data);
	} else {
		if (ana_hs_support_usb_sw()) {
			hwlog_info("%s:btn_pressed status not right, exit\n",
				__func__);
		} else {
			hwlog_info("%s:btn_pressed status not right,"
				"need further detect\n", __func__);
			/* further detect */
			hwcxext_mbhc_further_detect_plug_type(mbhc_data);
		}
	}
}

static void hwcxext_mbhc_btn_detect(struct hwcxext_mbhc_priv *mbhc_data)
{
	int ret;
	bool pulg_in = false;

	IN_FUNCTION;
	mutex_lock(&mbhc_data->btn_mutex);

	pulg_in = hwcxext_mbhc_check_headset_in(mbhc_data);
	if (!pulg_in) {
		hwlog_info("%s: plug out happens\n", __func__);
		goto btn_detect_exit;
	}

	if (mbhc_data->hs_status != AUDIO_JACK_HEADSET) {
		hwlog_info("%s: it need further detect\n", __func__);
		hwcxext_mbhc_further_detect_plug_type(mbhc_data);
		goto btn_detect_exit;
	}

	if (mbhc_data->mbhc_cb->get_btn_type_recognize) {
		ret = mbhc_data->mbhc_cb->get_btn_type_recognize(mbhc_data);
		if (ret) {
			hwlog_err("%s:get btn type error\n", __func__);
			goto btn_detect_exit;
		}
	}

	hwcxext_mbhc_btn_handle(mbhc_data);
btn_detect_exit:
	mutex_unlock(&mbhc_data->btn_mutex);
	OUT_FUNCTION;
}

static void hwcxext_mbhc_btn_work(struct work_struct *work)
{
	struct hwcxext_mbhc_priv *mbhc_data = container_of(work,
		struct hwcxext_mbhc_priv, btn_delay_work.work);

	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is null\n", __func__);
		return;
	}

	if (mbhc_data->get_btn_info.btn_event > BTN_RELEASED) {
		hwlog_err("%s: btn_event:%d is invalid\n",
			__func__, mbhc_data->get_btn_info.btn_event);
		return;
	}

	__pm_stay_awake(&mbhc_data->btn_wake_lock);
	hwlog_info("%s:in, current status:%s, last status:%s, hs_status:%d\n",
		__func__,
		g_btn_envent_info[mbhc_data->get_btn_info.btn_event].info,
		(mbhc_data->btn_pressed == BTN_AREADY_PRESS) ?
		"btn already press" : "btn already up",
		mbhc_data->hs_status);

	hwcxext_mbhc_btn_detect(mbhc_data);
	__pm_relax(&mbhc_data->btn_wake_lock);
	OUT_FUNCTION;
}

static void hwcxext_mbhc_jack_report(struct hwcxext_mbhc_priv *mbhc_data,
	enum audio_jack_states plug_type)
{
#ifdef CONFIG_SWITCH
	enum audio_jack_states jack_status = plug_type;
#endif
	int jack_report = 0;

	hwlog_info("%s: enter current_plug:%d new_plug:%d\n",
		__func__, mbhc_data->hs_status, plug_type);
	if (mbhc_data->hs_status == plug_type) {
		hwlog_info("%s: plug_type already reported, exit\n", __func__);
		return;
	}

	mbhc_data->hs_status = plug_type;
	switch (mbhc_data->hs_status) {
	case AUDIO_JACK_NONE:
		jack_report = 0;
		hwlog_info("%s : plug out\n", __func__);
		break;
	case AUDIO_JACK_HEADSET:
		jack_report = SND_JACK_HEADSET;
		hwlog_info("%s : 4-pole headset plug in\n", __func__);
		break;
	case AUDIO_JACK_HEADPHONE:
		jack_report = SND_JACK_HEADPHONE;
		hwlog_info("%s : 3-pole headphone plug in\n", __func__);
		break;
	case AUDIO_JACK_INVERT:
		jack_report = SND_JACK_HEADPHONE;
		hwlog_info("%s : invert headset plug in\n", __func__);
		break;
	case AUDIO_JACK_EXTERN_CABLE:
		jack_report = 0;
		hwlog_info("%s : extern cable plug in\n", __func__);
		break;
	default:
		hwlog_err("%s : error hs_status:%d\n", __func__,
			mbhc_data->hs_status);
		break;
	}

	/* report jack status */
	snd_soc_jack_report(&mbhc_data->headset_jack, jack_report,
		SND_JACK_HEADSET);

#ifdef CONFIG_SWITCH
	switch_set_state(&mbhc_data->sdev, jack_status);
#endif
}

static void hwcxext_report_hs_status(struct hwcxext_mbhc_priv *mbhc_data,
	enum audio_jack_states plug_type)
{
	hwcxext_mbhc_jack_report(mbhc_data, plug_type);
}

static void hwcxext_mbhc_plug_in_detect(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	bool pulg_in = false;
	enum audio_jack_states plug_type = AUDIO_JACK_NONE;

	IN_FUNCTION;
	hwcxext_mbhc_cancel_long_btn_work(mbhc_data);
	msleep(20);

	pulg_in = hwcxext_mbhc_check_headset_in(mbhc_data);
	if (!pulg_in)
		goto in_detect_for_hs_plguout_exit;

	mutex_lock(&mbhc_data->plug_mutex);
	/* get voltage by read sar in mbhc */
	if (mbhc_data->mbhc_cb->get_hs_type_recognize == NULL) {
		hwlog_err("%s: get_hs_type_recognize is NULL\n", __func__);
		goto in_detect_for_hs_plguout_exit;
	}
	mutex_lock(&mbhc_data->status_mutex);
	plug_type = mbhc_data->mbhc_cb->get_hs_type_recognize(mbhc_data);
	mutex_unlock(&mbhc_data->status_mutex);

	pulg_in = hwcxext_mbhc_check_headset_in(mbhc_data);
	if (!pulg_in) {
		hwlog_info("%s: plug out happens\n", __func__);
		mutex_unlock(&mbhc_data->plug_mutex);
		goto in_detect_for_hs_plguout_exit;
	}

	hwcxext_report_hs_status(mbhc_data, plug_type);
	if (ana_hs_support_usb_sw() && (plug_type == AUDIO_JACK_HEADSET)) {
		hwlog_info("%s: set hs type report jiffies\n", __func__);
		hwcxext_mbhc_btn_irq_control(mbhc_data, BTN_IRQ_ENABLE);
		atomic_set(&mbhc_data->btn_not_need_hanlde, 0);
		mbhc_data->hs_type_report_jiffies = jiffies;
	}

	mutex_unlock(&mbhc_data->plug_mutex);
	OUT_FUNCTION;
	return;
in_detect_for_hs_plguout_exit:
	hwcxext_mbhc_micbias_control(mbhc_data, MICB_DISABLE);
	hwcxext_mbhc_jsense_control(mbhc_data, JSENSE_PLUGOUT_HANDLE);
	OUT_FUNCTION;
}

void hwcxext_mbhc_plug_out_detect(struct hwcxext_mbhc_priv *mbhc_data)
{
	IN_FUNCTION;
	hwcxext_mbhc_cancel_long_btn_work(mbhc_data);
	mutex_lock(&mbhc_data->plug_mutex);
	mutex_lock(&mbhc_data->status_mutex);
	if (mbhc_data->btn_report) {
		hwlog_info("%s: release of button press\n", __func__);
		snd_soc_jack_report(&mbhc_data->button_jack,
			0, HWCXEXT_MBHC_JACK_BUTTON_MASK);
		mbhc_data->btn_report = 0;
		mbhc_data->btn_pressed = BTN_AREADY_UP;
	}
	mutex_unlock(&mbhc_data->status_mutex);
	hwcxext_report_hs_status(mbhc_data, AUDIO_JACK_NONE);
	mutex_unlock(&mbhc_data->plug_mutex);
	if (ana_hs_support_usb_sw())
		hwcxext_mbhc_btn_irq_control(mbhc_data, BTN_IRQ_DISABLE);

	hwcxext_mbhc_micbias_control(mbhc_data, MICB_DISABLE);
	hwcxext_mbhc_jsense_control(mbhc_data, JSENSE_PLUGOUT_HANDLE);
	OUT_FUNCTION;
}

/* work of headset insertion and removal recognize */
static void hwcxext_mbhc_plug_work(struct work_struct *work)
{
	bool pulg_in = false;
	struct hwcxext_mbhc_priv *mbhc_data =
		container_of(work, struct hwcxext_mbhc_priv,
		hs_plug_work.work);

	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is error\n", __func__);
		return;
	}

	__pm_stay_awake(&mbhc_data->plug_wake_lock);
	IN_FUNCTION;
	pulg_in = hwcxext_mbhc_check_headset_in(mbhc_data);
	hwlog_info("%s: the headset is :%s\n", __func__,
		pulg_in ?  "plugin" : "plugout");
	if (pulg_in) {
		hwcxext_mbhc_micbias_control(mbhc_data, MICB_ENABLE);
		hwcxext_mbhc_jsense_control(mbhc_data, JSENSE_PLUGIN_HANDLE);
		/* Jack inserted, determine type */
		hwcxext_mbhc_plug_in_detect(mbhc_data);
	} else {
		/* Jack removed, or spurious IRQ */
		hwcxext_mbhc_plug_out_detect(mbhc_data);
	}

	__pm_relax(&mbhc_data->plug_wake_lock);
	OUT_FUNCTION;
}

/* work of the irq share with hs plugin or plugout and btn press & release */
static void hwcxext_mbhc_irq_plug_btn_work(struct work_struct *work)
{
	struct hwcxext_mbhc_priv *mbhc_data =
		container_of(work, struct hwcxext_mbhc_priv,
		irq_plug_btn_work.work);

	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is error\n", __func__);
		return;
	}

	__pm_wakeup_event(&mbhc_data->plug_btn_wake_lock, 1000);
	IN_FUNCTION;
	if (ana_hs_support_usb_sw()) {
		/* use fas4480, hs plugin or plugout detect by usb */
		/* this is only for btn press & release handle */
		queue_delayed_work(mbhc_data->irq_btn_handle_wq,
			&mbhc_data->btn_delay_work,
			msecs_to_jiffies(20));
	} else { /* recognize irq types, it is plug irq or btn irq */
		if (mbhc_data->hs_status == AUDIO_JACK_NONE ||
			(mbhc_data->hs_status != AUDIO_JACK_NONE &&
			hwcxext_mbhc_check_headset_in(mbhc_data))) {
			/* it no audio jack, it is plug in irq */
			/* it has audio jack, and hs is plugin */
			/* it is plug out irq */
			queue_delayed_work(mbhc_data->irq_plug_handle_wq,
				&mbhc_data->hs_plug_work,
				msecs_to_jiffies(40));
		} else { /* btn irq */
			queue_delayed_work(mbhc_data->irq_btn_handle_wq,
				&mbhc_data->btn_delay_work,
				msecs_to_jiffies(20));
		}
	}
	OUT_FUNCTION;
}

static irqreturn_t hwcxext_mbhc_plug_btn_irq_thread(int irq, void *data)
{
	struct hwcxext_mbhc_priv *mbhc_data = data;

	unsigned long msec_val;
	int btn_not_need_hanlde;

	__pm_wakeup_event(&mbhc_data->plug_btn_irq_wake_lock, 1000);
	hwlog_info("%s: enter, irq = %d\n", __func__, irq);

	btn_not_need_hanlde = atomic_read(&mbhc_data->btn_not_need_hanlde);
	msec_val = jiffies_to_msecs(jiffies - mbhc_data->hs_type_report_jiffies);
	if (ana_hs_support_usb_sw() &&
		!btn_not_need_hanlde &&
		(msec_val < HS_BTN_IRQ_UNHANDLE_TIME_MS)) {
		hwlog_info("%s: btn irq too short, ignore it\n", __func__);
		atomic_inc(&mbhc_data->btn_not_need_hanlde);
		goto plug_btn_irq_thread_exit;
	}
	queue_delayed_work(mbhc_data->irq_plug_btn_wq,
		&mbhc_data->irq_plug_btn_work,
		msecs_to_jiffies(40));
plug_btn_irq_thread_exit:
	OUT_FUNCTION;
	return IRQ_HANDLED;
}

static int hwcxext_mbhc_get_plug_btn_irq_gpio(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	int ret;
	const char *gpio_str = "plug_btn_irq_gpio";
	struct device *dev = mbhc_data->dev;

	mbhc_data->plug_btn_irq_gpio =
		of_get_named_gpio(dev->of_node, gpio_str, 0);
	if (mbhc_data->plug_btn_irq_gpio < 0) {
		hwlog_debug("%s: get_named_gpio:%s failed, %d\n", __func__,
			gpio_str, mbhc_data->plug_btn_irq_gpio);
		ret = of_property_read_u32(dev->of_node, gpio_str,
			(u32 *)&(mbhc_data->plug_btn_irq_gpio));
		if (ret < 0) {
			hwlog_err("%s: of_property_read_u32 gpio failed, %d\n",
				__func__, ret);
			return -EFAULT;
		}
	}

	if (!gpio_is_valid(mbhc_data->plug_btn_irq_gpio)) {
		hwlog_err("%s: irq_handler gpio %d invalid\n", __func__,
			mbhc_data->plug_btn_irq_gpio);
		return -EFAULT;
	}

	ret = gpio_request(mbhc_data->plug_btn_irq_gpio, "hwcxext_plug_irq");
	if (ret < 0) {
		hwlog_err("%s: gpio_request ret %d invalid\n", __func__, ret);
		return -EFAULT;
	}

	ret = gpio_direction_input(mbhc_data->plug_btn_irq_gpio);
	if (ret < 0) {
		hwlog_err("%s set gpio input mode error:%d\n",
			__func__, ret);
		goto get_plug_btn_irq_gpio_err;
	}

	return 0;
get_plug_btn_irq_gpio_err:
	gpio_free(mbhc_data->plug_btn_irq_gpio);
	mbhc_data->plug_btn_irq_gpio = -EINVAL;
	return ret;
}

static int hwcxext_mbhc_request_plug_btn_detect_irq(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	int ret;
	const char *irq_flags_str = "irq_flags";
	struct device *dev = mbhc_data->dev;
	unsigned int irqflags;

	ret = hwcxext_mbhc_get_plug_btn_irq_gpio(mbhc_data);
	if (ret < 0)
		return ret;

	mbhc_data->plug_btn_irq =
		gpio_to_irq((unsigned int)(mbhc_data->plug_btn_irq_gpio));
	hwlog_info("%s detect_irq_gpio: %d, irq: %d\n", __func__,
		mbhc_data->plug_btn_irq_gpio, mbhc_data->plug_btn_irq);

	ret = of_property_read_u32(dev->of_node, irq_flags_str, &irqflags);
	if (ret < 0) {
		hwlog_err("%s: irq_handler get irq_flags failed\n", __func__);
		goto request_plug_irq_err;
	}
	hwlog_info("%s irqflags: %d\n", __func__, irqflags);

	ret = request_threaded_irq(mbhc_data->plug_btn_irq, NULL,
		hwcxext_mbhc_plug_btn_irq_thread,
		irqflags | IRQF_ONESHOT | IRQF_NO_SUSPEND,
		"hwcxext_mbhc_plug_detect", mbhc_data);
	if (ret) {
		hwlog_err("%s: Failed to request IRQ: %d\n", __func__, ret);
		goto request_plug_irq_err;
	}

	atomic_set(&mbhc_data->btn_irq_vote_cnt, 0);
	if (ana_hs_support_usb_sw())
		disable_irq(mbhc_data->plug_btn_irq);

	return 0;
request_plug_irq_err:
	gpio_free(mbhc_data->plug_btn_irq_gpio);
	mbhc_data->plug_btn_irq_gpio = -EINVAL;
	return -EFAULT;
}

static void hwcxext_mbhc_free_plug_irq(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	if (!gpio_is_valid(mbhc_data->plug_btn_irq_gpio))
		return;

	gpio_free(mbhc_data->plug_btn_irq_gpio);
	mbhc_data->plug_btn_irq_gpio = -EINVAL;
	free_irq(mbhc_data->plug_btn_irq, mbhc_data);
}

static void hwcxext_mbhc_detect_init_state(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	bool pulg_in = hwcxext_mbhc_check_headset_in(mbhc_data);

	hwlog_info("%s: the headset is :%s\n", __func__,
		pulg_in ?  "plugin" : "plugout");
	if (!pulg_in)
		return;

	hwcxext_mbhc_micbias_control(mbhc_data, MICB_ENABLE);
	hwcxext_mbhc_jsense_control(mbhc_data, JSENSE_PLUGIN_HANDLE);
	if (ana_hs_support_usb_sw())
		ana_hs_plug_handle(ANA_HS_PLUG_IN);
	else
		/* Jack inserted, determine type */
		hwcxext_mbhc_plug_in_detect(mbhc_data);
}

static int hwcxext_mbhc_register_hs_jack_btn(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	int ret;
	unsigned int i;
	struct jack_key_to_type key_type[] = {
		{ SND_JACK_BTN_0, KEY_MEDIA },
		{ SND_JACK_BTN_1, KEY_VOLUMEUP },
		{ SND_JACK_BTN_2, KEY_VOLUMEDOWN },
	};

	ret = snd_soc_card_jack_new(mbhc_data->component->card,
		"Headset Jack", SND_JACK_HEADSET,
		&mbhc_data->headset_jack, NULL, 0);
	if (ret) {
		hwlog_err("%s: Failed to create new headset jack\n", __func__);
		return -ENOMEM;
	}

	ret = snd_soc_card_jack_new(mbhc_data->component->card,
		"Button Jack",
		HWCXEXT_MBHC_JACK_BUTTON_MASK,
		&mbhc_data->button_jack, NULL, 0);
	if (ret) {
		hwlog_err("%s: Failed to create new button jack\n", __func__);
		return -ENOMEM;
	}

	for (i = 0; i < ARRAY_SIZE(key_type); i++) {
		ret = snd_jack_set_key(mbhc_data->button_jack.jack,
			key_type[i].type, key_type[i].key);
		if (ret) {
			hwlog_err("%s:set code for btn%d, error num: %d",
				i, ret);
			return -ENOMEM;
		}
	}

	return 0;
}

static void hwcxext_mbhc_variables_init(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	mbhc_data->hs_status = AUDIO_JACK_NONE;
	atomic_set(&mbhc_data->micbias_vote_cnt, 0);
	atomic_set(&mbhc_data->btn_not_need_hanlde, 0);
}

static void hwcxext_mbhc_mutex_init(struct hwcxext_mbhc_priv *mbhc_data)
{
	wakeup_source_init(&mbhc_data->plug_btn_wake_lock,
		"hwcxext-mbhc-plug-btn");
	wakeup_source_init(&mbhc_data->plug_btn_irq_wake_lock,
		"hwcxext-mbhc-plug-btn_irq");
	wakeup_source_init(&mbhc_data->plug_wake_lock, "hwcxext-mbhc-plug");
	wakeup_source_init(&mbhc_data->btn_wake_lock, "hwcxext-mbhc-btn");
	wakeup_source_init(&mbhc_data->long_btn_wake_lock,
		"hwcxext-mbhc-long-btn");
	wakeup_source_init(&mbhc_data->plug_repeat_detect_lock,
		"hwcxext-mbhc-repeat-detect");

	mutex_init(&mbhc_data->plug_mutex);
	mutex_init(&mbhc_data->status_mutex);

	mutex_init(&mbhc_data->btn_mutex);
	mutex_init(&mbhc_data->micbias_vote_mutex);
	mutex_init(&mbhc_data->btn_irq_vote_mutex);
	mutex_init(&mbhc_data->jsense_vote_mutex);
}

static int hwcxext_mbhc_variable_check(struct device *dev,
	struct snd_soc_component *component,
	struct hwcxext_mbhc_priv **mbhc_data,
	struct hwcxext_mbhc_cb *mbhc_cb)
{
	if (dev == NULL || mbhc_cb == NULL ||
		component == NULL || mbhc_data == NULL) {
		hwlog_err("%s: params is invaild\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static void hwcxext_mbhc_set_priv_data(struct device *dev,
	struct hwcxext_mbhc_priv *mbhc_pri, struct snd_soc_component *component,
	struct hwcxext_mbhc_priv **mbhc_data, struct hwcxext_mbhc_cb *mbhc_cb)
{
	*mbhc_data = mbhc_pri;
	mbhc_pri->mbhc_cb = mbhc_cb;
	mbhc_pri->component = component;
	mbhc_pri->dev = dev;
}

static int hwcxext_mbhc_register_switch_dev(
	struct hwcxext_mbhc_priv *mbhc_pri)
{
	int ret;

#ifdef CONFIG_SWITCH
	mbhc_pri->sdev.name = "h2w";
	ret = switch_dev_register(&(mbhc_pri->sdev));
	if (ret) {
		hwlog_err("%s: switch_dev_register failed: %d\n",
			__func__, ret);
		return -ENOMEM;
	}
	hwlog_info("%s: switch_dev_register succ\n", __func__);
#endif
	return 0;
}

static void hwcxext_mbhc_remove_plug_btn_workqueue(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->irq_plug_btn_wq) {
		cancel_delayed_work(&mbhc_data->irq_plug_btn_work);
		flush_workqueue(mbhc_data->irq_plug_btn_wq);
		destroy_workqueue(mbhc_data->irq_plug_btn_wq);
		mbhc_data->irq_plug_btn_wq = NULL;
	}
}

static void hwcxext_mbhc_remove_plug_workqueue(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->irq_plug_handle_wq) {
		cancel_delayed_work(&mbhc_data->hs_plug_work);
		flush_workqueue(mbhc_data->irq_plug_handle_wq);
		destroy_workqueue(mbhc_data->irq_plug_handle_wq);
		mbhc_data->irq_plug_handle_wq = NULL;
	}
}

static void hwcxext_mbhc_remove_btn_workqueue(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->irq_btn_handle_wq) {
		cancel_delayed_work(&mbhc_data->btn_delay_work);
		flush_workqueue(mbhc_data->irq_btn_handle_wq);
		destroy_workqueue(mbhc_data->irq_btn_handle_wq);
		mbhc_data->irq_btn_handle_wq = NULL;
	}
}

static void hwcxext_mbhc_remove_long_press_btn_workqueue(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->long_press_btn_wq) {
		cancel_delayed_work(&mbhc_data->long_press_btn_work);
		flush_workqueue(mbhc_data->long_press_btn_wq);
		destroy_workqueue(mbhc_data->long_press_btn_wq);
		mbhc_data->long_press_btn_wq = NULL;
	}
}

static void hwevext_mbhc_remove_repeat_detect_plug_workqueue(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->repeat_detect_plug_wq) {
		cancel_delayed_work(&mbhc_data->repeat_detect_plug_work);
		flush_workqueue(mbhc_data->repeat_detect_plug_wq);
		destroy_workqueue(mbhc_data->repeat_detect_plug_wq);
		mbhc_data->repeat_detect_plug_wq = NULL;
	}
}

static int hwcxext_mbhc_create_singlethread_workqueue(
	struct workqueue_struct **wq, const char *name)
{
	struct workqueue_struct *req_wq = create_singlethread_workqueue(name);

	if (!req_wq) {
		hwlog_err("%s : create %s wq failed", __func__, name);
		return -ENOMEM;
	}

	*wq = req_wq;
	return 0;
}

static int hwcxext_mbhc_create_delay_work(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	int ret;

	ret = hwcxext_mbhc_create_singlethread_workqueue(
		&(mbhc_data->irq_plug_btn_wq), "hwcxext_mbhc_plug_btn_wq");
	if (ret < 0)
		goto create_delay_work_err1;
	INIT_DELAYED_WORK(&mbhc_data->irq_plug_btn_work,
		hwcxext_mbhc_irq_plug_btn_work);

	ret = hwcxext_mbhc_create_singlethread_workqueue(
		&(mbhc_data->irq_plug_handle_wq), "hwcxext_mbhc_plug_wq");
	if (ret < 0)
		goto create_delay_work_err2;
	INIT_DELAYED_WORK(&mbhc_data->hs_plug_work, hwcxext_mbhc_plug_work);

	ret = hwcxext_mbhc_create_singlethread_workqueue(
		&(mbhc_data->irq_btn_handle_wq), "hwcxext_mbhc_btn_delay_wq");
	if (ret < 0)
		goto create_delay_work_err3;
	INIT_DELAYED_WORK(&mbhc_data->btn_delay_work, hwcxext_mbhc_btn_work);

	ret = hwcxext_mbhc_create_singlethread_workqueue(
		&(mbhc_data->long_press_btn_wq),
		"hwcxext_mbhc_long_press_btn_wq");
	if (ret < 0)
		goto create_delay_work_err4;
	INIT_DELAYED_WORK(&mbhc_data->long_press_btn_work,
		hwcxext_mbhc_long_press_btn_work);

	ret = hwcxext_mbhc_create_singlethread_workqueue(
		&(mbhc_data->repeat_detect_plug_wq),
		"hwcxext_mbhc_repeat_detect_plug_wq");
	if (ret < 0)
		goto create_delay_work_err5;
	INIT_DELAYED_WORK(&mbhc_data->repeat_detect_plug_work,
		hwcxext_mbhc_repeat_detect_plug_work);
	return 0;

create_delay_work_err5:
	hwcxext_mbhc_remove_long_press_btn_workqueue(mbhc_data);
create_delay_work_err4:
	hwcxext_mbhc_remove_btn_workqueue(mbhc_data);
create_delay_work_err3:
	hwcxext_mbhc_remove_plug_workqueue(mbhc_data);
create_delay_work_err2:
	hwcxext_mbhc_remove_plug_btn_workqueue(mbhc_data);
create_delay_work_err1:
	return -ENOMEM;
}

static void hwcxext_mbhc_destroy_delay_work(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	hwcxext_mbhc_remove_plug_btn_workqueue(mbhc_data);
	hwcxext_mbhc_remove_btn_workqueue(mbhc_data);
	hwcxext_mbhc_remove_plug_workqueue(mbhc_data);
	hwcxext_mbhc_remove_long_press_btn_workqueue(mbhc_data);
	hwevext_mbhc_remove_repeat_detect_plug_workqueue(mbhc_data);
}

static void hwcxext_mbhc_high_resistence_enable(void *priv, bool enable)
{
	UNUSED_PARAMETER(priv);
	UNUSED_PARAMETER(enable);
}

static void hwcxext_mbhc_for_ext_plug_in_detect(
	void *para)
{
	struct hwcxext_mbhc_priv *mbhc_data = (struct hwcxext_mbhc_priv *)para;

	if (IS_ERR_OR_NULL(para)) {
		hwlog_err("%s: para is error\n", __func__);
		return;
	}

	IN_FUNCTION;
	hwcxext_mbhc_micbias_control(mbhc_data, MICB_ENABLE);
	hwcxext_mbhc_jsense_control(mbhc_data, JSENSE_PLUGIN_HANDLE);
	hwcxext_mbhc_plug_in_detect(mbhc_data);
	hwevext_mbhc_start_repeat_detect(mbhc_data);
	OUT_FUNCTION;
}

static void hwcxext_mbhc_for_ext_plug_out_detect(void *para)
{
	struct hwcxext_mbhc_priv *mbhc_data = (struct hwcxext_mbhc_priv *)para;

	if (IS_ERR_OR_NULL(para)) {
		hwlog_err("%s: para is error\n", __func__);
		return;
	}

	IN_FUNCTION;
	hwcxext_mbhc_plug_out_detect(mbhc_data);
	OUT_FUNCTION;
}

static bool hwcxext_mbhc_ext_check_status_headset_in(void *para)
{
	struct hwcxext_mbhc_priv *mbhc_data = (struct hwcxext_mbhc_priv *)para;

	if (IS_ERR_OR_NULL(para)) {
		hwlog_err("%s: para is error\n", __func__);
		return false;
	}

	return hwcxext_mbhc_check_headset_in(mbhc_data);
}

static int hwcxext_get_mbhc_headset_type(void *para)
{
	struct hwcxext_mbhc_priv *mbhc_data = (struct hwcxext_mbhc_priv *)para;

	if (IS_ERR_OR_NULL(para)) {
		hwlog_err("%s: para is error\n", __func__);
		return AUDIO_JACK_NONE;
	}

	return (int)(mbhc_data->hs_status);
}

static struct ana_hs_codec_dev g_ana_hs_dev = {
	.name = "ana_hs",
	.ops = {
		.check_headset_in = hwcxext_mbhc_ext_check_status_headset_in,
		.plug_in_detect = hwcxext_mbhc_for_ext_plug_in_detect,
		.plug_out_detect = hwcxext_mbhc_for_ext_plug_out_detect,
		.get_headset_type = hwcxext_get_mbhc_headset_type,
		.hs_high_resistence_enable = hwcxext_mbhc_high_resistence_enable,
	},
};

static void hwcxext_mbhc_parse_jsense_gpio(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	int ret;
	const char *jsense_gpio_str = "jsense_gpio";
	struct device *dev = mbhc_data->dev;

	mbhc_data->jsense_gpio = of_get_named_gpio(dev->of_node,
		jsense_gpio_str, 0);
	if (mbhc_data->jsense_gpio < 0) {
		hwlog_debug("%s: get_named_gpio failed, %d\n", __func__,
			mbhc_data->jsense_gpio);
		ret = of_property_read_u32(dev->of_node, jsense_gpio_str,
			(u32 *)&mbhc_data->jsense_gpio);
		if (ret < 0) {
			hwlog_err("%s: of_property_read_u32 gpio failed, %d\n",
				__func__, ret);
			goto parse_jsense_gpio_err;
		}
	}

	if (gpio_request((unsigned int)mbhc_data->jsense_gpio,
		"hwcxext_jsense") < 0) {
		hwlog_err("%s: gpio%d request failed\n", __func__,
			mbhc_data->jsense_gpio);
		goto parse_jsense_gpio_err;
	}
	gpio_direction_output(mbhc_data->jsense_gpio, 1);
	atomic_set(&mbhc_data->jsense_vote_cnt, 0);
	return;

parse_jsense_gpio_err:
	mbhc_data->jsense_gpio = -EINVAL;
}

static int hwcxext_mbhc_resource_init(
	struct hwcxext_mbhc_priv *mbhc_pri)
{
	int ret;

	ret = hwcxext_mbhc_create_delay_work(mbhc_pri);
	if (ret < 0)
		return ret;

	ret = hwcxext_mbhc_request_plug_btn_detect_irq(mbhc_pri);
	if (ret < 0) {
		hwlog_err("%s request plug detect irq failed\n", __func__);
		ret = -EINVAL;
		goto hwcxext_mbhc_resource_init_err1;
	}

	ret = hwcxext_mbhc_register_hs_jack_btn(mbhc_pri);
	if (ret < 0) {
		hwlog_err("%s register hs jack failed\n", __func__);
		goto hwcxext_mbhc_resource_init_err2;
	}

	ret = hwcxext_mbhc_register_switch_dev(mbhc_pri);
	if (ret < 0)
		goto hwcxext_mbhc_resource_init_err2;

	return 0;
hwcxext_mbhc_resource_init_err2:
	hwcxext_mbhc_free_plug_irq(mbhc_pri);
hwcxext_mbhc_resource_init_err1:
	hwcxext_mbhc_destroy_delay_work(mbhc_pri);
	return ret;
}

int hwcxext_mbhc_init(struct device *dev,
	struct snd_soc_component *component,
	struct hwcxext_mbhc_priv **mbhc_data,
	struct hwcxext_mbhc_cb *mbhc_cb)
{
	int ret;
	struct hwcxext_mbhc_priv *mbhc_pri = NULL;

	IN_FUNCTION;
	if (hwcxext_mbhc_variable_check(dev, component, mbhc_data, mbhc_cb) < 0)
		return -EINVAL;

	mbhc_pri = devm_kzalloc(dev,
			sizeof(struct hwcxext_mbhc_priv), GFP_KERNEL);
	if (mbhc_pri == NULL) {
		hwlog_err("%s: kzalloc fail\n", __func__);
		return -ENOMEM;
	}

	hwcxext_mbhc_set_priv_data(dev, mbhc_pri,
		component, mbhc_data, mbhc_cb);
	hwcxext_mbhc_variables_init(mbhc_pri);
	hwcxext_mbhc_mutex_init(mbhc_pri);
	hwcxext_mbhc_parse_jsense_gpio(mbhc_pri);
	ana_hs_codec_dev_register(&g_ana_hs_dev, mbhc_pri);
	ret = hwcxext_mbhc_resource_init(mbhc_pri);
	if (ret < 0)
		goto hwcxext_mbhc_init_err1;

	if (mbhc_pri->mbhc_cb->enable_jack_detect)
		mbhc_pri->mbhc_cb->enable_jack_detect(mbhc_pri,
			ana_hs_support_usb_sw());

	hwcxext_mbhc_detect_init_state(mbhc_pri);
	return 0;
hwcxext_mbhc_init_err1:
	devm_kfree(dev, mbhc_pri);
	return ret;
}
EXPORT_SYMBOL_GPL(hwcxext_mbhc_init);

static void hwcxext_mbhc_mutex_deinit(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	wakeup_source_trash(&mbhc_data->plug_btn_wake_lock);
	wakeup_source_trash(&mbhc_data->plug_btn_irq_wake_lock);
	wakeup_source_trash(&mbhc_data->plug_wake_lock);
	wakeup_source_trash(&mbhc_data->btn_wake_lock);
	wakeup_source_trash(&mbhc_data->long_btn_wake_lock);
	wakeup_source_trash(&mbhc_data->plug_repeat_detect_lock);

	mutex_destroy(&mbhc_data->plug_mutex);
	mutex_destroy(&mbhc_data->status_mutex);
	mutex_destroy(&mbhc_data->btn_mutex);
	mutex_destroy(&mbhc_data->micbias_vote_mutex);
	mutex_destroy(&mbhc_data->btn_irq_vote_mutex);
	mutex_destroy(&mbhc_data->jsense_vote_mutex);
}

void hwcxext_mbhc_exit(struct device *dev,
	struct hwcxext_mbhc_priv *mbhc_data)
{
	IN_FUNCTION;

	if (dev == NULL || mbhc_data == NULL) {
		hwlog_err("%s: params is invaild\n", __func__);
		return;
	}

	hwcxext_mbhc_mutex_deinit(mbhc_data);
	hwcxext_mbhc_free_plug_irq(mbhc_data);
	hwcxext_mbhc_destroy_delay_work(mbhc_data);

#ifdef CONFIG_SWITCH
	switch_dev_unregister(&(mbhc_data->sdev));
#endif
	devm_kfree(dev, mbhc_data);
}
EXPORT_SYMBOL_GPL(hwcxext_mbhc_exit);

MODULE_DESCRIPTION("hwcxext mbhc");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
