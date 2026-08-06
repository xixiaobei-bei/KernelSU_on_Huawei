/*
 * hwevext_mbhc.c
 *
 * hwevext mbhc driver
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
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/property.h>
#include <linux/pm_wakeirq.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/input/matrix_keypad.h>
#include <linux/hisi/hisi_adc.h>

#include <sound/soc.h>
#include <sound/jack.h>

#include <huawei_platform/log/hw_log.h>

#include "hwevext_codec_v1.h"
#include "hwevext_codec_info.h"
#include "hwevext_mbhc.h"

#define HWLOG_TAG hwevext_mbhc
HWLOG_REGIST();

#define IN_FUNCTION   hwlog_info("%s function comein\n", __func__)
#define OUT_FUNCTION  hwlog_info("%s function comeout\n", __func__)

#define HS_3_POLE_MIN_VOLTAGE           0
#define HS_3_POLE_MAX_VOLTAGE           8
#define HS_4_POLE_MIN_VOLTAGE           1150
#define HS_4_POLE_MAX_VOLTAGE           2600
#define BTN_HOOK_MIN_VOLTAGE            0
#define BTN_HOOK_MAX_VOLTAGE            100
#define BTN_VOLUME_UP_MIN_VOLTAGE       130
#define BTN_VOLUME_UP_MAX_VOLTAGE       320
#define BTN_VOLUME_DOWN_MIN_VOLTAGE     350
#define BTN_VOLUME_DOWN_MAX_VOLTAGE     700
#define HS_EXTERN_CABLE_MIN_VOLTAGE     2651
#define HS_EXTERN_CABLE_MAX_VOLTAGE     2700
#define INVALID_VOLTAGE                 0xFFFFFFFF

#define HS_STATUS_READ_REG_RETRY_TIME   6
#define HS_REPEAT_DETECT_PLUG_TIME_MS   3000 // poll 3s
#define HS_REPEAT_WAIT_CNT              12
#define HS_LONG_BTN_PRESS_LIMIT_TIME    6000
#define CONTINUE_BTN_IRQ_LIMIT_MS       28

#define HWEVEXT_MBHC_JACK_BUTTON_MASK (SND_JACK_BTN_0 | SND_JACK_BTN_1 | \
	SND_JACK_BTN_2)

struct voltage_node_info {
	const char *min_node;
	const char *max_node;
	unsigned int min_def;
	unsigned int max_def;
};

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
	ADC_POWER_SUPPLY_DISABLE,
	ADC_POWER_SUPPLY_ENABLE,
};

static void hwevext_mbhc_adc_power_supply_control(
	struct hwevext_mbhc_priv *mbhc_data,
	int req)
{
	int adc_supply_en_vote_cnt;

	if (mbhc_data->adc_power_supply_en_gpio < 0 ||
		(!gpio_is_valid(mbhc_data->adc_power_supply_en_gpio)))
		return;

	mutex_lock(&mbhc_data->adc_supply_en_vote_mutex);
	adc_supply_en_vote_cnt = atomic_read(&mbhc_data->adc_supply_en_vote_cnt);
	hwlog_info("%s: req:%s, adc_supply_en_vote_cnt:%d\n",
		__func__, req == ADC_POWER_SUPPLY_ENABLE ?
		"adc_power_supply enable" :"adc_power_supply disable",
		adc_supply_en_vote_cnt);

	if (req == ADC_POWER_SUPPLY_ENABLE) {
		if (adc_supply_en_vote_cnt)
			goto adc_power_supply_control_exit;

		atomic_inc(&mbhc_data->adc_supply_en_vote_cnt);
		hwlog_info("%s: enable adc_power_supply", __func__);
		gpio_set_value(mbhc_data->adc_power_supply_en_gpio, 1);
	} else {
		atomic_dec(&mbhc_data->adc_supply_en_vote_cnt);
		adc_supply_en_vote_cnt =
			atomic_read(&mbhc_data->adc_supply_en_vote_cnt);
		hwlog_info("%s: disable, adc_supply_en_vote_cnt:%d\n", __func__,
			adc_supply_en_vote_cnt);

		if (adc_supply_en_vote_cnt < 0) {
			atomic_set(&mbhc_data->adc_supply_en_vote_cnt, 0);
		} else if (adc_supply_en_vote_cnt == 0) {
			hwlog_info("%s: disable adc_power_supply", __func__);
			gpio_set_value(mbhc_data->adc_power_supply_en_gpio, 0);
		}
	}
adc_power_supply_control_exit:
	hwlog_info("%s: out, adc_supply_en_vote_cnt:%d\n", __func__,
		atomic_read(&mbhc_data->adc_supply_en_vote_cnt));
	mutex_unlock(&mbhc_data->adc_supply_en_vote_mutex);
}

static int hwevext_mbhc_cancel_long_btn_work(struct hwevext_mbhc_priv *mbhc_data)
{
	/* it can not use cancel_delayed_work_sync, otherwise lead deadlock */
	int ret = cancel_delayed_work(&mbhc_data->long_press_btn_work);

	if (ret)
		hwlog_info("%s: cancel long btn work\n", __func__);

	return ret;
}

static void hwevext_mbhc_btn_irq_control(struct hwevext_mbhc_priv *mbhc_data,
	int req)
{
	mutex_lock(&mbhc_data->btn_irq_vote_mutex);
	hwlog_info("%s: req:%s, btn_irq_vote_cnt:%d\n",
		__func__, req == BTN_IRQ_ENABLE ? "btn_irq enable" :
		"btn_irq disable", mbhc_data->btn_irq_vote_cnt);

	if (req == BTN_IRQ_ENABLE) {
		mbhc_data->btn_irq_vote_cnt++;
		if (mbhc_data->btn_irq_vote_cnt == 1) {
			hwlog_info("%s: enable btn irq", __func__);
			enable_irq(mbhc_data->btn_irq);
		}
	} else {
		mbhc_data->btn_irq_vote_cnt--;
		if (mbhc_data->btn_irq_vote_cnt < 0) {
			mbhc_data->btn_irq_vote_cnt = 0;
		} else if (mbhc_data->btn_irq_vote_cnt == 0) {
			hwlog_info("%s: disable btn irq", __func__);
			disable_irq(mbhc_data->btn_irq);
		}
	}
	hwlog_info("%s: out, btn_irq_vote_cnt:%d\n", __func__,
		mbhc_data->btn_irq_vote_cnt);
	mutex_unlock(&mbhc_data->btn_irq_vote_mutex);
}

static void hwevext_mbhc_micbias_control(struct hwevext_mbhc_priv *mbhc_data,
	int req)
{
	int micbias_vote_cnt;

	if (mbhc_data->mbhc_cb->disable_micbias == NULL)
		return;

	mutex_lock(&mbhc_data->micbias_vote_mutex);
	micbias_vote_cnt = atomic_read(&mbhc_data->micbias_vote_cnt);
	hwlog_info("%s: req:%s, micbias_vote_cnt:%d\n",
		__func__, req == MICB_ENABLE ? "micbias enable" :
		"micbias disable", micbias_vote_cnt);
	if (req == MICB_ENABLE) {
		if (micbias_vote_cnt)
				goto mbhc_micbias_control_exit;

		atomic_inc(&mbhc_data->micbias_vote_cnt);
		hwevext_mbhc_btn_irq_control(mbhc_data, BTN_IRQ_DISABLE);
		mbhc_data->mbhc_cb->enable_micbias(mbhc_data);
		mdelay(10);
		hwevext_mbhc_btn_irq_control(mbhc_data, BTN_IRQ_ENABLE);
	} else {
		atomic_dec(&mbhc_data->micbias_vote_cnt);
		micbias_vote_cnt = atomic_read(&mbhc_data->micbias_vote_cnt);
		hwlog_info("%s: disable, micbias_vote_cnt:%d\n", __func__,
			micbias_vote_cnt);
		if (micbias_vote_cnt < 0) {
			atomic_set(&mbhc_data->micbias_vote_cnt, 0);
		} else if (micbias_vote_cnt == 0) {
			hwevext_mbhc_btn_irq_control(mbhc_data, BTN_IRQ_DISABLE);
			mbhc_data->mbhc_cb->disable_micbias(mbhc_data);
			mdelay(10);
			hwevext_mbhc_btn_irq_control(mbhc_data, BTN_IRQ_ENABLE);
		}
	}

mbhc_micbias_control_exit:
	hwlog_info("%s: out, micbias_vote_cnt:%d\n", __func__,
		atomic_read(&mbhc_data->micbias_vote_cnt));
	mutex_unlock(&mbhc_data->micbias_vote_mutex);
}

static void hwevext_mbhc_jack_report(struct hwevext_mbhc_priv *mbhc_data,
	enum audio_jack_states plug_type)
{
#ifdef CONFIG_SWITCH
	enum audio_jack_states jack_status = plug_type;
#endif
	int jack_report = 0;

	hwlog_info("%s: enter current_plug(%d) new_plug(%d)\n",
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

static unsigned int hwevext_get_voltage(struct hwevext_mbhc_priv *mbhc_data)
{
	int val;

	mutex_lock(&mbhc_data->adc_mutex);
	val = hisi_adc_get_value(mbhc_data->hkadc_channel);
	mutex_unlock(&mbhc_data->adc_mutex);
	hwlog_info("%s: hkadc value:%d\n", __func__, val);

	return val;
}

static bool is_voltage_in_range(struct hwevext_mbhc_priv *mbhc_data,
	unsigned int voltage, enum hs_voltage_type type)
{
	if ((mbhc_data->mbhc_config.voltage[type].min <= voltage) &&
		(mbhc_data->mbhc_config.voltage[type].max >= voltage))
		return true;

	return false;
}

static int hwevext_hs_type_recognize(struct hwevext_mbhc_priv *mbhc_data,
	int hkadc_value)
{
	if (is_voltage_in_range(mbhc_data, hkadc_value, VOLTAGE_POLE3)) {
		hwlog_info("%s: headphone is 3 pole, hkadc_value=%d\n",
			__func__,  hkadc_value);
		return AUDIO_JACK_HEADPHONE;
	} else if (is_voltage_in_range(mbhc_data, hkadc_value, VOLTAGE_POLE4)) {
		hwlog_info("%s: headphone is 4 pole, hkadc_value=%d\n",
			__func__, hkadc_value);
		return AUDIO_JACK_HEADSET;
	} else if (is_voltage_in_range(mbhc_data, hkadc_value,
		VOLTAGE_EXTERN_CABLE)) {
		hwlog_info("%s:headphone is lineout, hkadc_value=%d\n",
			__func__, hkadc_value);
		return AUDIO_JACK_EXTERN_CABLE;
	} else {
		hwlog_info("%s:headphone is invert 4 pole, hkadc_value=%d\n",
			__func__, hkadc_value);
		return AUDIO_JACK_INVERT;
	}
}

static void hwevext_report_hs_status(struct hwevext_mbhc_priv *mbhc_data,
	enum audio_jack_states plug_type)
{
	hwevext_mbhc_jack_report(mbhc_data, plug_type);
}

static bool mbhc_check_status_headset_in(struct hwevext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->mbhc_cb->mbhc_check_headset_in)
		return mbhc_data->mbhc_cb->mbhc_check_headset_in(mbhc_data);

	return false;
}

static void hwevext_mbhc_plug_in_detect(struct hwevext_mbhc_priv *mbhc_data)
{
	bool pulg_in = false;
	int val;
	enum audio_jack_states plug_type = AUDIO_JACK_NONE;

	IN_FUNCTION;
	mutex_lock(&mbhc_data->plug_mutex);
	/* first disable btn irq */
	disable_irq(mbhc_data->btn_irq);
	msleep(20);
	pulg_in = mbhc_check_status_headset_in(mbhc_data);
	if (!pulg_in) {
		hwlog_info("%s: plug out happens\n", __func__);
		hwevext_mbhc_micbias_control(mbhc_data, MICB_DISABLE);
		hwevext_mbhc_adc_power_supply_control(mbhc_data,
			ADC_POWER_SUPPLY_DISABLE);
		goto plug_in_detect_exit;
	}

	/* get voltage by read sar in mbhc */
	val = hwevext_get_voltage(mbhc_data);
	mutex_lock(&mbhc_data->status_mutex);
	plug_type = hwevext_hs_type_recognize(mbhc_data, val);
	mutex_unlock(&mbhc_data->status_mutex);

	msleep(100);
	pulg_in = mbhc_check_status_headset_in(mbhc_data);
	if (!pulg_in) {
		hwlog_info("%s: plug out happens\n", __func__);
		hwevext_mbhc_micbias_control(mbhc_data, MICB_DISABLE);
		hwevext_mbhc_adc_power_supply_control(mbhc_data,
			ADC_POWER_SUPPLY_DISABLE);
		goto plug_in_detect_exit;
	}

	hwevext_report_hs_status(mbhc_data, plug_type);
plug_in_detect_exit:
	/* enable btn irq */
	enable_irq(mbhc_data->btn_irq);
	mutex_unlock(&mbhc_data->plug_mutex);
	OUT_FUNCTION;
}

static void hwevext_mbhc_plug_out_detect(struct hwevext_mbhc_priv *mbhc_data)
{
	IN_FUNCTION;
	mutex_lock(&mbhc_data->plug_mutex);
	mutex_lock(&mbhc_data->status_mutex);
	if (mbhc_data->btn_report) {
		hwlog_info("%s: release of button press\n", __func__);
		snd_soc_jack_report(&mbhc_data->button_jack,
			0, HWEVEXT_MBHC_JACK_BUTTON_MASK);
		mbhc_data->btn_report = 0;
		mbhc_data->btn_pressed = 0;
	}

	mutex_unlock(&mbhc_data->status_mutex);
	hwevext_report_hs_status(mbhc_data, AUDIO_JACK_NONE);
	mutex_unlock(&mbhc_data->plug_mutex);
	OUT_FUNCTION;
}

static void hwevext_mbhc_read_hs_status_work(struct work_struct *work)
{
	struct hwevext_mbhc_priv *mbhc_data =
		container_of(work, struct hwevext_mbhc_priv,
		read_hs_status_work.work);

	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is error\n", __func__);
		return;
	}

	if (mbhc_data->mbhc_cb->get_hs_status_reg_value == NULL)
		return;

	mutex_lock(&mbhc_data->read_hs_status_reg_lock);
	mbhc_data->hs_status_reg_status0 =
		mbhc_data->mbhc_cb->get_hs_status_reg_value(mbhc_data);

	if(mbhc_data->hs_status_reg_status0 != mbhc_data->hs_status_reg_status1) {
		mbhc_data->hs_status_reg_status1 = mbhc_data->hs_status_reg_status0;
		mbhc_data->hs_status_reg_read_retry = HS_STATUS_READ_REG_RETRY_TIME;
	} else {
		mbhc_data->hs_status_reg_read_retry--;
	}

	if (mbhc_data->hs_status_reg_read_retry > 0) {
		queue_delayed_work(mbhc_data->read_hs_status_wq,
			&mbhc_data->read_hs_status_work,
			msecs_to_jiffies(50));

		hwlog_info("%s:re-try%d, read hs status reg after 50 msec",
			__func__, mbhc_data->hs_status_reg_read_retry);
	}
	mutex_unlock(&mbhc_data->read_hs_status_reg_lock);
}

static void hwevext_mbhc_repeat_detect_plug_work(struct work_struct *work)
{
	unsigned long timeout;
	bool pulg_in = false;
	unsigned int i;
	struct hwevext_mbhc_priv *mbhc_data = container_of(work,
		struct hwevext_mbhc_priv, repeat_detect_plug_work.work);

	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is error\n", __func__);
		return;
	}

	__pm_stay_awake(&mbhc_data->plug_repeat_detect_lock);
	IN_FUNCTION;
	timeout = jiffies + msecs_to_jiffies(HS_REPEAT_DETECT_PLUG_TIME_MS);
	while (!time_after(jiffies, timeout)) {
		hwlog_info("%s:repeat_detect_work_stop:%d, btn irq detect:%d\n",
			__func__, mbhc_data->repeat_detect_work_stop,
		atomic_read(&mbhc_data->need_further_detect_for_btn_irq));
		for (i = 0; i < HS_REPEAT_WAIT_CNT; i++) {
			if (mbhc_data->repeat_detect_work_stop) {
				hwlog_info("%s: stop requested: %d\n", __func__,
					mbhc_data->repeat_detect_work_stop);
					goto repeat_detect_plug_exit;
			}

			msleep(20);
			pulg_in = mbhc_check_status_headset_in(mbhc_data);
			if (!pulg_in) {
				hwlog_info("%s: plug out happens\n", __func__);
				hwevext_mbhc_micbias_control(mbhc_data,
					MICB_DISABLE);
				hwevext_mbhc_adc_power_supply_control(mbhc_data,
					ADC_POWER_SUPPLY_DISABLE);
				goto repeat_detect_plug_exit;
			}
			msleep(15);
		}
		hwevext_mbhc_plug_in_detect(mbhc_data);
	}

repeat_detect_plug_exit:
	atomic_set(&mbhc_data->need_further_detect_for_btn_irq, 0);
	__pm_relax(&mbhc_data->plug_repeat_detect_lock);
	OUT_FUNCTION;
}

static void hwevext_mbhc_start_repeat_detect(struct hwevext_mbhc_priv *mbhc_data)
{
	bool pulg_in = false;

	pulg_in = mbhc_check_status_headset_in(mbhc_data);
	if (!pulg_in) {
		hwlog_info("%s: plug out happens\n", __func__);
		return;
	}

	if (mbhc_data->hs_status != AUDIO_JACK_HEADSET &&
		mbhc_data->hs_status != AUDIO_JACK_NONE) {
		hwlog_info("%s: schedule repeat recognize work\n", __func__);
		mbhc_data->repeat_detect_work_stop = false;
		queue_delayed_work(mbhc_data->repeat_detect_plug_wq,
			&mbhc_data->repeat_detect_plug_work,
			0);
	}
}

static void hwevext_mbhc_plug_work(struct work_struct *work)
{
	bool pulg_in = false;
	struct hwevext_mbhc_priv *mbhc_data =
		container_of(work, struct hwevext_mbhc_priv,
		plug_work.work);

	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is error\n", __func__);
		return;
	}

	__pm_stay_awake(&mbhc_data->plug_wake_lock);
	IN_FUNCTION;
	mbhc_data->repeat_detect_work_stop = true;
	if (cancel_delayed_work_sync(&mbhc_data->repeat_detect_plug_work))
		hwlog_info("%s: repeat recognize work is canceled\n",
			__func__);

	hwevext_mbhc_cancel_long_btn_work(mbhc_data);
	pulg_in = mbhc_check_status_headset_in(mbhc_data);
	hwlog_info("%s: the headset is :%s\n", __func__,
		pulg_in ?  "plugin" : "plugout");
	if (pulg_in) {
		msleep(600);
		pulg_in = mbhc_check_status_headset_in(mbhc_data);
		hwlog_info("%s: after delay, the headset is :%s\n", __func__,
		pulg_in ?  "plugin" : "plugout");
		if (!pulg_in)
			goto hwevext_mbhc_plug_work_exit;

		hwevext_mbhc_adc_power_supply_control(mbhc_data,
			ADC_POWER_SUPPLY_ENABLE);
		hwevext_mbhc_micbias_control(mbhc_data, MICB_ENABLE);
		/* Jack inserted, determine type */
		hwevext_mbhc_plug_in_detect(mbhc_data);
		hwevext_mbhc_start_repeat_detect(mbhc_data);
	} else {
		/* Jack removed, or spurious IRQ */
		hwevext_mbhc_plug_out_detect(mbhc_data);
		hwevext_mbhc_micbias_control(mbhc_data, MICB_DISABLE);
		hwevext_mbhc_adc_power_supply_control(mbhc_data,
			ADC_POWER_SUPPLY_DISABLE);
	}

hwevext_mbhc_plug_work_exit:
	__pm_relax(&mbhc_data->plug_wake_lock);
	OUT_FUNCTION;
}

static irqreturn_t hwevext_mbhc_plug_irq_thread(int irq, void *data)
{
	struct hwevext_mbhc_priv *mbhc_data = data;

	__pm_wakeup_event(&mbhc_data->plug_irq_wake_lock, 1000);
	hwlog_info("%s: enter, irq = %d\n", __func__, irq);

	queue_delayed_work(mbhc_data->read_hs_status_wq,
		&mbhc_data->read_hs_status_work,
		0);
	queue_delayed_work(mbhc_data->irq_plug_wq,
		&mbhc_data->plug_work,
		msecs_to_jiffies(40));

	OUT_FUNCTION;
	return IRQ_HANDLED;
}

static int hwevext_mbhc_request_plug_detect_irq(
	struct hwevext_mbhc_priv *mbhc_data)
{
	int ret;
	const char *gpio_plug_str = "plug_irq_gpio";
	struct device *dev = mbhc_data->dev;

	mbhc_data->plug_irq_gpio =
		of_get_named_gpio(dev->of_node, gpio_plug_str, 0);
	if (mbhc_data->plug_irq_gpio < 0) {
		hwlog_debug("%s: get_named_gpio:%s failed, %d\n", __func__,
			gpio_plug_str, mbhc_data->plug_irq_gpio);
		ret = of_property_read_u32(dev->of_node, gpio_plug_str,
			(u32 *)&(mbhc_data->plug_irq_gpio));
		if (ret < 0) {
			hwlog_err("%s: of_property_read_u32 gpio failed, %d\n",
				__func__, ret);
			return -EFAULT;
		}
	}

	if (!gpio_is_valid(mbhc_data->plug_irq_gpio)) {
		hwlog_err("%s: irq_handler gpio %d invalid\n", __func__,
			mbhc_data->plug_irq_gpio);
		return -EFAULT;
	}

	ret = gpio_request(mbhc_data->plug_irq_gpio, "hwevext_plug_irq");
	if (ret < 0) {
		hwlog_err("%s: gpio_request ret %d invalid\n", __func__, ret);
		return -EFAULT;
	}

	ret = gpio_direction_input(mbhc_data->plug_irq_gpio);
	if (ret < 0) {
		hwlog_err("%s set gpio input mode error:%d\n",
			__func__, ret);
		 goto request_plug_irq_err;
	}

	mbhc_data->plug_irq =
		gpio_to_irq((unsigned int)(mbhc_data->plug_irq_gpio));
	hwlog_info("%s detect_irq_gpio: %d, irq: %d\n", __func__,
		mbhc_data->plug_irq_gpio, mbhc_data->plug_irq);

	ret = request_threaded_irq(mbhc_data->plug_irq, NULL,
		hwevext_mbhc_plug_irq_thread,
		IRQF_TRIGGER_FALLING | IRQF_ONESHOT | IRQF_NO_SUSPEND,
		"hwevext_mbhc_plug_detect", mbhc_data);
	if (ret) {
		hwlog_err("%s: Failed to request IRQ: %d\n", __func__, ret);
		goto request_plug_irq_err;
	}
	return 0;

request_plug_irq_err:
	gpio_free(mbhc_data->plug_irq_gpio);
	mbhc_data->plug_irq_gpio = -EINVAL;
	return -EFAULT;
}

static void hwevext_mbhc_free_plug_irq(struct hwevext_mbhc_priv *mbhc_data)
{
	if (!gpio_is_valid(mbhc_data->plug_irq_gpio))
		return;

	gpio_free(mbhc_data->plug_irq_gpio);
	mbhc_data->plug_irq_gpio = -EINVAL;
	free_irq(mbhc_data->plug_irq, mbhc_data);
}

static void hwevext_mbhc_detect_init_state(struct hwevext_mbhc_priv *mbhc_data)
{
	bool pulg_in = mbhc_check_status_headset_in(mbhc_data);

	hwlog_info("%s: the headset is :%s\n", __func__,
		pulg_in ?  "plugin" : "plugout");
	if (!pulg_in)
		return;

	hwevext_mbhc_adc_power_supply_control(mbhc_data,
		ADC_POWER_SUPPLY_ENABLE);
	hwevext_mbhc_micbias_control(mbhc_data, MICB_ENABLE);
	/* Jack inserted, determine type */
	hwevext_mbhc_plug_in_detect(mbhc_data);
	hwevext_mbhc_start_repeat_detect(mbhc_data);
}

static const struct voltage_btn_report g_btn_report_map[] = {
	{ "btn hook", VOLTAGE_HOOK, SND_JACK_BTN_0 },
	{ "volume up", VOLTAGE_VOL_UP, SND_JACK_BTN_1 },
	{ "volume down", VOLTAGE_VOL_DOWN, SND_JACK_BTN_2 },
};

static int hwevext_get_btn_report(struct hwevext_mbhc_priv *mbhc_data,
	unsigned int voltage)
{
	unsigned int i;
	unsigned int size = ARRAY_SIZE(g_btn_report_map);

	for (i = 0; i < size; i++) {
		if (is_voltage_in_range(mbhc_data, voltage,
			g_btn_report_map[i].voltage_type)) {
			hwlog_info("%s: process as %s, hkadc:%d", __func__,
				g_btn_report_map[i].info, voltage);
			mutex_lock(&mbhc_data->status_mutex);
			mbhc_data->btn_report = g_btn_report_map[i].btn_type;
			mutex_unlock(&mbhc_data->status_mutex);
			return 0;
		}
	}

	return -EINVAL;
}

static bool hwevext_mbhc_get_btn_press_status(struct hwevext_mbhc_priv *mbhc_data)
{
	int value = gpio_get_value(mbhc_data->btn_irq_gpio);

	hwlog_info("%s: enter, value:%d, %s\n", __func__, value,
		value ? "btn press" : "btn up");
	if (value)
		/* when btn press, the gpio btn_irq is output high */
		return true;

	/* when btn not press or relese, the gpio btn_irq is output low */
	return false;
}

static void hwevext_mbhc_further_detect_plug_type(
	struct hwevext_mbhc_priv *mbhc_data)
{
	IN_FUNCTION;
	/* further detect */
	msleep(20);
	hwevext_mbhc_plug_in_detect(mbhc_data);
	hwevext_mbhc_start_repeat_detect(mbhc_data);
}

static void hwevext_mbhc_continue_btn_irq_detect_plug_type(
	struct hwevext_mbhc_priv *mbhc_data)
{
	if (atomic_read(&mbhc_data->need_further_detect_for_btn_irq) > 0) {
		hwlog_info("%s: it need further detect\n", __func__);
		queue_delayed_work(mbhc_data->repeat_detect_plug_wq,
			&mbhc_data->repeat_detect_plug_work,
			msecs_to_jiffies(20));
	}
}

static void hwevext_mbhc_long_press_btn_trigger(
	struct hwevext_mbhc_priv *mbhc_data)
{
	queue_delayed_work(mbhc_data->long_press_btn_wq,
		&mbhc_data->long_press_btn_work,
		msecs_to_jiffies(HS_LONG_BTN_PRESS_LIMIT_TIME));
}

static void hwevext_mbhc_btn_detect(struct hwevext_mbhc_priv *mbhc_data)
{
	int ret;
	bool pulg_in = false;
	int voltage;

	IN_FUNCTION;
	mutex_lock(&mbhc_data->btn_mutex);

	pulg_in = mbhc_check_status_headset_in(mbhc_data);
	if (!pulg_in) {
		hwlog_info("%s: plug out happens\n", __func__);
		goto btn_detect_exit;
	}

	if (mbhc_data->hs_status != AUDIO_JACK_HEADSET) {
		hwlog_info("%s: it need further detect\n", __func__);
		hwevext_mbhc_further_detect_plug_type(mbhc_data);
		goto btn_detect_exit;
	}

	/* btn down */
	if (hwevext_mbhc_get_btn_press_status(mbhc_data) &&
		(mbhc_data->btn_pressed == 0)) {
		/* button down event */
		hwlog_info("%s:button down event\n", __func__);
		mdelay(20);
		voltage = hwevext_get_voltage(mbhc_data);
		ret = hwevext_get_btn_report(mbhc_data, voltage);
		if (!ret) {
			pulg_in = mbhc_check_status_headset_in(mbhc_data);
			if (!pulg_in) {
				hwlog_info("%s: plug out happens\n", __func__);
				goto btn_detect_exit;
			}

			mutex_lock(&mbhc_data->status_mutex);
			mbhc_data->btn_pressed = BTN_AREADY_PRESS;
			mutex_unlock(&mbhc_data->status_mutex);
			/* btn report key event */
			hwlog_info("%s: btn press report type: 0x%x, status: %d",
				__func__, mbhc_data->btn_report,
				mbhc_data->hs_status);
			snd_soc_jack_report(&mbhc_data->button_jack,
				mbhc_data->btn_report,
				HWEVEXT_MBHC_JACK_BUTTON_MASK);
			hwevext_mbhc_continue_btn_irq_detect_plug_type(mbhc_data);
			hwevext_mbhc_long_press_btn_trigger(mbhc_data);
		} else {
			hwlog_warn("%s:it not a button press, further detect\n",
				__func__);
			hwevext_mbhc_further_detect_plug_type(mbhc_data);
		}
	} else if (mbhc_data->btn_pressed == BTN_AREADY_PRESS) {
		/* button up event */
		hwlog_info("%s : btn up report type: 0x%x release\n", __func__,
			mbhc_data->btn_report);
		hwevext_mbhc_cancel_long_btn_work(mbhc_data);
		mutex_lock(&mbhc_data->status_mutex);
		mbhc_data->btn_report = 0;
		snd_soc_jack_report(&mbhc_data->button_jack,
			mbhc_data->btn_report, HWEVEXT_MBHC_JACK_BUTTON_MASK);
		mbhc_data->btn_pressed = BTN_AREADY_UP;
		mutex_unlock(&mbhc_data->status_mutex);
		hwevext_mbhc_continue_btn_irq_detect_plug_type(mbhc_data);
	} else {
		hwlog_info("%s:btn_pressed status not right, further detect\n",
			__func__);
		/* further detect */
		hwevext_mbhc_further_detect_plug_type(mbhc_data);
	}
btn_detect_exit:
	mutex_unlock(&mbhc_data->btn_mutex);
	OUT_FUNCTION;
}

static void hwevext_mbhc_long_press_btn_work(struct work_struct *work)
{
	struct hwevext_mbhc_priv *mbhc_data = container_of(work,
		struct hwevext_mbhc_priv, long_press_btn_work.work);;

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

	mutex_lock(&mbhc_data->btn_mutex);
	mbhc_data->btn_report = 0;
	snd_soc_jack_report(&mbhc_data->button_jack,
		mbhc_data->btn_report, HWEVEXT_MBHC_JACK_BUTTON_MASK);
	mbhc_data->btn_pressed = BTN_AREADY_UP;
	mutex_unlock(&mbhc_data->btn_mutex);

long_press_btn_exit:
	__pm_relax(&mbhc_data->long_btn_wake_lock);
	OUT_FUNCTION;
}

static void hwevext_mbhc_btn_work(struct work_struct *work)
{
	struct hwevext_mbhc_priv *mbhc_data = container_of(work,
		struct hwevext_mbhc_priv, btn_delay_work.work);;

	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is null\n", __func__);
		return;
	}

	__pm_stay_awake(&mbhc_data->btn_wake_lock);

	hwlog_info("%s:in, current status:%s, last status:%s, hs_status:%d\n",
		__func__, hwevext_mbhc_get_btn_press_status(mbhc_data) ?
		"btn press" : "btn up",
		(mbhc_data->btn_pressed == BTN_AREADY_PRESS) ?
		"btn already press" : "btn already up",
		mbhc_data->hs_status);

	hwevext_mbhc_btn_detect(mbhc_data);
	__pm_relax(&mbhc_data->btn_wake_lock);
	OUT_FUNCTION;
}

static irqreturn_t hwevext_mbhc_btn_irq_thread(int irq, void *data)
{
	unsigned long msec_val;
	struct hwevext_mbhc_priv *mbhc_data = data;
	bool pulg_in = false;

	__pm_wakeup_event(&mbhc_data->btn_irq_wake_lock, 800);
	hwlog_info("%s: enter, irq = %d\n", __func__, irq);
	pulg_in = mbhc_check_status_headset_in(mbhc_data);
	if (!pulg_in) {
		hwlog_info("%s: plug out happens\n", __func__);
		goto btn_irq_thread_exit;
	}

	spin_lock_bh(&mbhc_data->btn_irq_lock);
	if (mbhc_data->btn_irq_jiffies != 0) {
		msec_val = jiffies_to_msecs(jiffies - mbhc_data->btn_irq_jiffies);
		if (msec_val < CONTINUE_BTN_IRQ_LIMIT_MS) {
			hwlog_info("%s: continue irq occured\n", __func__);
			atomic_inc(&mbhc_data->need_further_detect_for_btn_irq);
		}
	}
	mbhc_data->btn_irq_jiffies = jiffies;
	spin_unlock_bh(&mbhc_data->btn_irq_lock);
	queue_delayed_work(mbhc_data->irq_btn_handle_wq,
		&mbhc_data->btn_delay_work,
		msecs_to_jiffies(30));

btn_irq_thread_exit:
	OUT_FUNCTION;
	return IRQ_HANDLED;
}

static int hwevext_mbhc_parse_btn_irq_gpio(struct hwevext_mbhc_priv *mbhc_data)
{
	int ret;

	const char *gpio_btn_str = "btn_irq_gpio";
	struct device *dev = mbhc_data->dev;

	mbhc_data->btn_irq_gpio =
		of_get_named_gpio(dev->of_node, gpio_btn_str, 0);
	if (mbhc_data->btn_irq_gpio < 0) {
		hwlog_debug("%s: get_named_gpio:%s failed, %d\n", __func__,
			gpio_btn_str, mbhc_data->btn_irq_gpio);
		ret = of_property_read_u32(dev->of_node, gpio_btn_str,
			(u32 *)&(mbhc_data->btn_irq_gpio));
		if (ret < 0) {
			hwlog_err("%s: of_property_read_u32 gpio failed, %d\n",
				__func__, ret);
			return -EFAULT;
		}
	}

	if (!gpio_is_valid(mbhc_data->btn_irq_gpio)) {
		hwlog_err("%s: irq_handler gpio %d invalid\n", __func__,
			mbhc_data->btn_irq_gpio);
		return -EFAULT;
	}

	ret = gpio_request(mbhc_data->btn_irq_gpio, "hwevext_btn_irq");
	if (ret < 0) {
		hwlog_err("%s: gpio_request ret %d invalid\n", __func__, ret);
		return -EFAULT;
	}

	ret = gpio_direction_input(mbhc_data->btn_irq_gpio);
	if (ret < 0) {
		hwlog_err("%s set gpio input mode error:%d\n",
			__func__, ret);
		goto parse_btn_gpio_err;
	}

	mbhc_data->btn_irq =
		gpio_to_irq((unsigned int)(mbhc_data->btn_irq_gpio));
	hwlog_info("%s detect_irq_gpio: %d, irq: %d\n", __func__,
		mbhc_data->btn_irq_gpio, mbhc_data->btn_irq);
	return 0;

parse_btn_gpio_err:
	gpio_free(mbhc_data->btn_irq_gpio);
	mbhc_data->btn_irq_gpio = -EINVAL;

	return -EFAULT;
}

static int hwevext_mbhc_request_btn_detect_irq(
	struct hwevext_mbhc_priv *mbhc_data)
{
	int ret;

	ret = hwevext_mbhc_parse_btn_irq_gpio(mbhc_data);
	if (ret < 0)
		return ret;

	ret = request_threaded_irq(mbhc_data->btn_irq, NULL,
		hwevext_mbhc_btn_irq_thread,
		IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_ONESHOT |
		IRQF_NO_SUSPEND,
		"hwevext_mbhc_btn_detect", mbhc_data);
	if (ret) {
		hwlog_err("%s: Failed to request IRQ: %d\n", __func__, ret);
		goto request_btn_irq_err;
	}

	/* first disable btn irq */
	disable_irq(mbhc_data->btn_irq);
	mbhc_data->btn_irq_vote_cnt = 0;
	return 0;

request_btn_irq_err:
	gpio_free(mbhc_data->btn_irq_gpio);
	mbhc_data->btn_irq_gpio = -EINVAL;
	return -EFAULT;
}

static void hwevext_mbhc_free_btn_irq(struct hwevext_mbhc_priv *mbhc_data)
{
	if (!gpio_is_valid(mbhc_data->btn_irq_gpio))
		return;

	gpio_free(mbhc_data->btn_irq_gpio);
	mbhc_data->btn_irq_gpio = -EINVAL;
	free_irq(mbhc_data->btn_irq, mbhc_data);
}

static const struct voltage_node_info g_voltage_tbl[] = {
	{ "hs_3_pole_min_voltage", "hs_3_pole_max_voltage",
		HS_3_POLE_MIN_VOLTAGE, HS_3_POLE_MAX_VOLTAGE },
	{ "hs_4_pole_min_voltage", "hs_4_pole_max_voltage",
		HS_4_POLE_MIN_VOLTAGE, HS_4_POLE_MAX_VOLTAGE },
	{ "hs_extern_cable_min_voltage", "hs_extern_cable_max_voltage",
		HS_EXTERN_CABLE_MIN_VOLTAGE, HS_EXTERN_CABLE_MAX_VOLTAGE },
	{ "btn_hook_min_voltage", "btn_hook_max_voltage",
		BTN_HOOK_MIN_VOLTAGE, BTN_HOOK_MAX_VOLTAGE },
	{ "btn_volume_up_min_voltage", "btn_volume_up_max_voltage",
		BTN_VOLUME_UP_MIN_VOLTAGE, BTN_VOLUME_UP_MAX_VOLTAGE },
	{ "btn_volume_down_min_voltage", "btn_volume_down_max_voltage",
		BTN_VOLUME_DOWN_MIN_VOLTAGE, BTN_VOLUME_DOWN_MAX_VOLTAGE },
};

static void hwevext_mbhc_config_print(struct hwevext_mbhc_config *mbhc_config)
{
	unsigned int i;
	unsigned int size = ARRAY_SIZE(g_voltage_tbl);
	const char *mbhc_print_str[VOLTAGE_TYPE_BUTT] = {
		"pole3", "pole4", "hs_extern_cable",
		"btn_hook", "btn_volume_up", "btn_volume_down",
	};

	if (size != VOLTAGE_TYPE_BUTT)
		return;

	for (i = 0; i < size; i++)
		hwlog_info("%s: headset %s min: %d, max: %d\n", __func__,
			mbhc_print_str[i], mbhc_config->voltage[i].min,
			mbhc_config->voltage[i].max);
}

static int hwevext_get_voltage_config(struct device *dev,
	struct hwevext_mbhc_config *mbhc_config)
{
	unsigned int i;
	unsigned int val = 0;
	unsigned int size = ARRAY_SIZE(g_voltage_tbl);
	int ret;
	struct device_node *np = dev->of_node;

	if (size != VOLTAGE_TYPE_BUTT) {
		hwlog_err("%s: size %u is invaild\n", __func__, size);
		return -EINVAL;;
	}

	for (i = 0; i < size; i++) {
		ret = of_property_read_u32(np, g_voltage_tbl[i].min_node, &val);
		if (ret < 0) {
			hwlog_info("%s: get %s from dtsi failed, use default",
				__func__, g_voltage_tbl[i].min_node);
			mbhc_config->voltage[i].min = g_voltage_tbl[i].min_def;
		} else {
			mbhc_config->voltage[i].min = val;
		}

		ret = of_property_read_u32(np, g_voltage_tbl[i].max_node, &val);
		if (ret < 0) {
			hwlog_info("%s: get %s from dtsi failed, use default",
				__func__, g_voltage_tbl[i].max_node);
			mbhc_config->voltage[i].max = g_voltage_tbl[i].max_def;
		} else {
			mbhc_config->voltage[i].max = val;
		}
	}

	hwevext_mbhc_config_print(mbhc_config);
	return 0;
}

static int hwevext_mbhc_register_hs_jack_btn(struct hwevext_mbhc_priv *mbhc_data)
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
		HWEVEXT_MBHC_JACK_BUTTON_MASK,
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

static int hwevext_get_hkadc_channel(const struct device *dev,
	struct hwevext_mbhc_priv *mbhc_data)
{
	int ret;

	ret = of_property_read_u32(dev->of_node, "hkadc_channel",
		&mbhc_data->hkadc_channel);
	if (ret < 0) {
		hwlog_err("%s: get hkadc channel failed\n", __func__);
		return -EFAULT;
	}

	hwlog_info("%s: get hkadc channel:%d\n", __func__,
		mbhc_data->hkadc_channel);
	hwevext_codec_info_set_adc_channel(mbhc_data->hkadc_channel);

	return 0;
}

static int hwevext_mbhc_parse_dts_pdata(struct device *dev,
	struct hwevext_mbhc_priv *mbhc_data)
{
	int ret;

	IN_FUNCTION;
	hwlog_info("%s: of_node name:%s, full_name:%s\n", __func__,
		dev->of_node->name,
		dev->of_node->full_name);
	ret = hwevext_get_voltage_config(dev, &mbhc_data->mbhc_config);
	if (ret)
		goto parse_dts_pdata_err;

	ret = hwevext_get_hkadc_channel(dev, mbhc_data);
	if (ret)
		goto parse_dts_pdata_err;

	return 0;

parse_dts_pdata_err:
	hwlog_err("%s: parse dts pdata err\n", __func__);
	return -EFAULT;
}

static void hwevext_mbhc_variables_init(struct hwevext_mbhc_priv *mbhc_data)
{
	mbhc_data->hs_status_reg_read_retry = HS_STATUS_READ_REG_RETRY_TIME;
	mbhc_data->hs_status_reg_status0 = 0;
	mbhc_data->hs_status_reg_status1 = 0;
	mbhc_data->repeat_detect_work_stop = true;
	mbhc_data->hs_status = AUDIO_JACK_NONE;
	mbhc_data->btn_irq_jiffies = 0;
	mbhc_data->plug_irq_gpio = -EINVAL;
	mbhc_data->btn_irq_gpio = -EINVAL;
	atomic_set(&mbhc_data->need_further_detect_for_btn_irq, 0);
	atomic_set(&mbhc_data->micbias_vote_cnt, 0);
}

static void hwevext_mbhc_mutex_init(struct hwevext_mbhc_priv *mbhc_data)
{
	wakeup_source_init(&mbhc_data->plug_wake_lock, "hwevext-mbhc-plug");
	wakeup_source_init(&mbhc_data->btn_wake_lock, "hwevext-mbhc-btn");
	wakeup_source_init(&mbhc_data->plug_repeat_detect_lock,
		"hwevext-mbhc-repeat-detect");
	wakeup_source_init(&mbhc_data->plug_irq_wake_lock,
		"hwevext-mbhc-irq-plug");
	wakeup_source_init(&mbhc_data->btn_irq_wake_lock,
		"hwevext-mbhc-irq-btn");
	wakeup_source_init(&mbhc_data->long_btn_wake_lock,
		"hwevext-mbhc-long-btn");

	mutex_init(&mbhc_data->plug_mutex);
	mutex_init(&mbhc_data->status_mutex);
	mutex_init(&mbhc_data->adc_mutex);
	mutex_init(&mbhc_data->btn_mutex);
	mutex_init(&mbhc_data->read_hs_status_reg_lock);
	mutex_init(&mbhc_data->micbias_vote_mutex);
	mutex_init(&mbhc_data->btn_irq_vote_mutex);
	mutex_init(&mbhc_data->adc_supply_en_vote_mutex);
	spin_lock_init(&mbhc_data->btn_irq_lock);
}

static int hwevext_mbhc_variable_check(struct device *dev,
	struct snd_soc_component *component,
	struct hwevext_mbhc_priv **mbhc_data,
	struct hwevext_mbhc_cb *mbhc_cb)
{
	if (dev == NULL || mbhc_cb == NULL ||
		component == NULL || mbhc_data == NULL) {
		hwlog_err("%s: params is invaild\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static void hwevext_mbhc_set_priv_data(struct device *dev,
	struct hwevext_mbhc_priv *mbhc_pri, struct snd_soc_component *component,
	struct hwevext_mbhc_priv **mbhc_data, struct hwevext_mbhc_cb *mbhc_cb)
{
	*mbhc_data = mbhc_pri;
	mbhc_pri->mbhc_cb = mbhc_cb;
	mbhc_pri->component = component;
	mbhc_pri->dev = dev;
}

static int hwevext_mbhc_register_switch_dev(struct hwevext_mbhc_priv *mbhc_pri)
{
	int ret;

#ifdef CONFIG_SWITCH
	mbhc_pri->sdev.name = "h2w";
	ret = switch_dev_register(&(mbhc_pri->sdev));
	if (ret) {
		hwlog_err("%s: switch_dev_register failed: %d\n", __func__, ret);
		return -ENOMEM;
	}
	hwlog_info("%s: switch_dev_register succ\n", __func__);
#endif
	return 0;
}

static void hwevext_mbhc_remove_plug_workqueue(
	struct hwevext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->irq_plug_wq) {
		cancel_delayed_work(&mbhc_data->plug_work);
		flush_workqueue(mbhc_data->irq_plug_wq);
		destroy_workqueue(mbhc_data->irq_plug_wq);
		mbhc_data->irq_plug_wq = NULL;
	}
}

static void hwevext_mbhc_remove_btn_workqueue(
	struct hwevext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->irq_btn_handle_wq) {
		cancel_delayed_work(&mbhc_data->btn_delay_work);
		flush_workqueue(mbhc_data->irq_btn_handle_wq);
		destroy_workqueue(mbhc_data->irq_btn_handle_wq);
		mbhc_data->irq_btn_handle_wq = NULL;
	}
}

static void hwevext_mbhc_remove_read_hs_status_workqueue(
	struct hwevext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->read_hs_status_wq) {
		cancel_delayed_work(&mbhc_data->read_hs_status_work);
		flush_workqueue(mbhc_data->read_hs_status_wq);
		destroy_workqueue(mbhc_data->read_hs_status_wq);
		mbhc_data->read_hs_status_wq = NULL;
	}
}

static void hwevext_mbhc_remove_repeat_detect_plug_workqueue(
	struct hwevext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->repeat_detect_plug_wq) {
		cancel_delayed_work(&mbhc_data->repeat_detect_plug_work);
		flush_workqueue(mbhc_data->repeat_detect_plug_wq);
		destroy_workqueue(mbhc_data->repeat_detect_plug_wq);
		mbhc_data->repeat_detect_plug_wq = NULL;
	}
}

static void hwevext_mbhc_remove_long_press_btn_workqueue(
	struct hwevext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->long_press_btn_wq) {
		cancel_delayed_work(&mbhc_data->long_press_btn_work);
		flush_workqueue(mbhc_data->long_press_btn_wq);
		destroy_workqueue(mbhc_data->long_press_btn_wq);
		mbhc_data->long_press_btn_wq = NULL;
	}
}

static int hwevext_mbhc_create_delay_work(struct hwevext_mbhc_priv *mbhc_data)
{
	IN_FUNCTION;
	mbhc_data->irq_plug_wq = 
		create_singlethread_workqueue("hwevext_mbhc_plug_wq");
	if (!(mbhc_data->irq_plug_wq)) {
		hwlog_err("%s : irq_plug_wq create failed", __func__);
		goto create_delay_work_err1;
	}
	INIT_DELAYED_WORK(&mbhc_data->plug_work, hwevext_mbhc_plug_work);

	mbhc_data->irq_btn_handle_wq =
		create_singlethread_workqueue("hwevext_mbhc_btn_delay_wq");
	if (!(mbhc_data->irq_btn_handle_wq)) {
		hwlog_err("%s : irq_btn_handle_wq create failed", __func__);
		goto create_delay_work_err2;
	}
	INIT_DELAYED_WORK(&mbhc_data->btn_delay_work, hwevext_mbhc_btn_work);

	mbhc_data->read_hs_status_wq =
		create_singlethread_workqueue("hwevext_mbhc_read_hs_status_wq");
	if (!(mbhc_data->read_hs_status_wq)) {
		hwlog_err("%s : read_hs_status_wq create failed", __func__);
		goto create_delay_work_err3;
	}
	INIT_DELAYED_WORK(&mbhc_data->read_hs_status_work,
		hwevext_mbhc_read_hs_status_work);

	mbhc_data->repeat_detect_plug_wq =
		create_singlethread_workqueue("hwevext_mbhc_repeat_detect_plug_wq");
	if (!(mbhc_data->repeat_detect_plug_wq)) {
		hwlog_err("%s : repeat_detect_plug_wq create failed", __func__);
		goto create_delay_work_err4;
	}
	INIT_DELAYED_WORK(&mbhc_data->repeat_detect_plug_work,
		hwevext_mbhc_repeat_detect_plug_work);

	mbhc_data->long_press_btn_wq =
		create_singlethread_workqueue("hwevext_mbhc_long_press_btn_wq");
	if (!(mbhc_data->long_press_btn_wq)) {
		hwlog_err("%s : long_press_btn_wq create failed", __func__);
		goto create_delay_work_err5;
	}
	INIT_DELAYED_WORK(&mbhc_data->long_press_btn_work,
		hwevext_mbhc_long_press_btn_work);

	return 0;
create_delay_work_err5:
	hwevext_mbhc_remove_long_press_btn_workqueue(mbhc_data);
create_delay_work_err4:
	hwevext_mbhc_remove_read_hs_status_workqueue(mbhc_data);
create_delay_work_err3:
	hwevext_mbhc_remove_btn_workqueue(mbhc_data);
create_delay_work_err2:
	hwevext_mbhc_remove_plug_workqueue(mbhc_data);
create_delay_work_err1:
	return -ENOMEM;
}

static void hwevext_mbhc_destroy_delay_work(struct hwevext_mbhc_priv *mbhc_data)
{
	hwevext_mbhc_remove_repeat_detect_plug_workqueue(mbhc_data);
	hwevext_mbhc_remove_read_hs_status_workqueue(mbhc_data);
	hwevext_mbhc_remove_btn_workqueue(mbhc_data);
	hwevext_mbhc_remove_plug_workqueue(mbhc_data);
	hwevext_mbhc_remove_long_press_btn_workqueue(mbhc_data);
}

static void hwevext_mbhc_parse_adc_power_supply(struct device *dev,
	struct hwevext_mbhc_priv *mbhc_data)
{
	int ret;
	const char *en_gpio_str = "adc_power_supply_en_gpio";

	mbhc_data->adc_power_supply_en_gpio = of_get_named_gpio(dev->of_node,
		en_gpio_str, 0);
	if (mbhc_data->adc_power_supply_en_gpio < 0) {
		hwlog_debug("%s: get_named_gpio failed, %d\n", __func__,
			mbhc_data->adc_power_supply_en_gpio);
		ret = of_property_read_u32(dev->of_node, en_gpio_str,
			(u32 *)&mbhc_data->adc_power_supply_en_gpio);
		if (ret < 0) {
			hwlog_err("%s: of_property_read_u32 gpio failed, %d\n",
				__func__, ret);
			goto parse_adc_power_supply_err;
		}
	}

	if (gpio_request((unsigned int)mbhc_data->adc_power_supply_en_gpio,
		"adc_power_supply_en") < 0) {
		hwlog_err("%s: gpio%d request failed\n", __func__,
			mbhc_data->adc_power_supply_en_gpio);
		goto parse_adc_power_supply_err;
	}
	gpio_direction_output(mbhc_data->adc_power_supply_en_gpio, 0);
	atomic_set(&mbhc_data->adc_supply_en_vote_cnt, 0);
	return;

parse_adc_power_supply_err:
	mbhc_data->adc_power_supply_en_gpio = -EINVAL;
}

int hwevext_mbhc_init(struct device *dev,
	struct snd_soc_component *component,
	struct hwevext_mbhc_priv **mbhc_data,
	struct hwevext_mbhc_cb *mbhc_cb)
{
	int ret;
	struct hwevext_mbhc_priv *mbhc_pri = NULL;

	IN_FUNCTION;
	if (hwevext_mbhc_variable_check(dev, component, mbhc_data, mbhc_cb) < 0)
		return -EINVAL;

	mbhc_pri = devm_kzalloc(dev,
			sizeof(struct hwevext_mbhc_priv), GFP_KERNEL);
	if (mbhc_pri == NULL) {
		hwlog_err("%s: kzalloc fail\n", __func__);
		return -ENOMEM;
	}

	hwevext_mbhc_set_priv_data(dev, mbhc_pri, component, mbhc_data, mbhc_cb);
	hwevext_mbhc_variables_init(mbhc_pri);
	hwevext_mbhc_mutex_init(mbhc_pri);
	ret = hwevext_mbhc_parse_dts_pdata(dev, mbhc_pri);
	if (ret < 0) {
		hwlog_err("%s parse dts data failed\n", __func__);
		ret = -EINVAL;
		goto hwevext_mbhc_init_err1;
	}

	ret = hwevext_mbhc_create_delay_work(mbhc_pri);
	if (ret < 0)
		goto hwevext_mbhc_init_err1;

	ret = hwevext_mbhc_request_plug_detect_irq(mbhc_pri);
	if (ret < 0) {
		hwlog_err("%s request plug detect irq failed\n", __func__);
		ret = -EINVAL;
		goto hwevext_mbhc_init_err2;
	}

	ret = hwevext_mbhc_request_btn_detect_irq(mbhc_pri);
	if (ret < 0) {
		hwlog_err("%s request btn detect irq failed\n", __func__);
		ret = -EINVAL;
		goto hwevext_mbhc_init_err3;
	}

	ret = hwevext_mbhc_register_hs_jack_btn(mbhc_pri);
	if (ret < 0) {
		hwlog_err("%s register hs jack failed\n", __func__);
		goto hwevext_mbhc_init_err4;
	}

	ret = hwevext_mbhc_register_switch_dev(mbhc_pri);
	if (ret < 0)
		goto hwevext_mbhc_init_err4;

	hwevext_mbhc_parse_adc_power_supply(dev, mbhc_pri);
	hwevext_mbhc_detect_init_state(mbhc_pri);
	return 0;

hwevext_mbhc_init_err4:
	hwevext_mbhc_free_btn_irq(mbhc_pri);
hwevext_mbhc_init_err3:
	hwevext_mbhc_free_plug_irq(mbhc_pri);
hwevext_mbhc_init_err2:
	hwevext_mbhc_destroy_delay_work(mbhc_pri);
hwevext_mbhc_init_err1:
	devm_kfree(dev, mbhc_pri);
	return ret;
}
EXPORT_SYMBOL_GPL(hwevext_mbhc_init);

static void hwevext_mbhc_mutex_deinit(struct hwevext_mbhc_priv *mbhc_data)
{
	wakeup_source_trash(&mbhc_data->plug_wake_lock);
	wakeup_source_trash(&mbhc_data->btn_wake_lock);
	wakeup_source_trash(&mbhc_data->plug_repeat_detect_lock);
	wakeup_source_trash(&mbhc_data->plug_irq_wake_lock);
	wakeup_source_trash(&mbhc_data->btn_irq_wake_lock);
	wakeup_source_trash(&mbhc_data->long_btn_wake_lock);

	mutex_destroy(&mbhc_data->plug_mutex);
	mutex_destroy(&mbhc_data->status_mutex);
	mutex_destroy(&mbhc_data->adc_mutex);
	mutex_destroy(&mbhc_data->btn_mutex);
	mutex_destroy(&mbhc_data->read_hs_status_reg_lock);
	mutex_destroy(&mbhc_data->micbias_vote_mutex);
	mutex_destroy(&mbhc_data->btn_irq_vote_mutex);
	mutex_destroy(&mbhc_data->adc_supply_en_vote_mutex);
}

void hwevext_mbhc_exit(struct device *dev,
	struct hwevext_mbhc_priv *mbhc_data)
{
	IN_FUNCTION;

	if (dev == NULL || mbhc_data == NULL) {
		hwlog_err("%s: params is invaild\n", __func__);
		return;
	}

	hwevext_mbhc_mutex_deinit(mbhc_data);
	hwevext_mbhc_free_btn_irq(mbhc_data);
	hwevext_mbhc_free_plug_irq(mbhc_data);
	hwevext_mbhc_destroy_delay_work(mbhc_data);

#ifdef CONFIG_SWITCH
	switch_dev_unregister(&(mbhc_data->sdev));
#endif
	devm_kfree(dev, mbhc_data);
}
EXPORT_SYMBOL_GPL(hwevext_mbhc_exit);

MODULE_DESCRIPTION("hwevext mbhc");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
