/*
 * hi6xxx_mbhc.c -- hi6xxx mbhc driver
 *
 * Copyright (c) 2017 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "hi6xxx_mbhc.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/hisi/hisi_adc.h>
#include <sound/jack.h>
#include <linux/input/matrix_keypad.h>
#include <linux/interrupt.h>

#include "hs_auto_calib.h"
#include "hi6555_reg.h"
#ifdef CONFIG_SND_SOC_CODEC_HI6555V3
#include "hi6555_utils.h"
#include "hi6555v3.h"
#endif


#include "huawei_platform/audio/usb_analog_hs_interface.h"
#include "huawei_platform/audio/ana_hs_common.h"
#include "ana_hs_kit/ana_hs.h"


#include "audio_log.h"

#define LOG_TAG "hi6xxx_mbhc"
#define UNUSED_PARAMETER(x)        (void)(x)

#define MICBIAS_PD_WAKE_LOCK_MS        3500
#define MICBIAS_PD_DELAY_MS            3000
#define IRQ_HANDLE_WAKE_LOCK_MS        2000
#define LINEOUT_PO_RECHK_WAKE_LOCK_MS  3500
#define LINEOUT_PO_RECHK_DELAY_MS      800
#define HI6XXX_HKADC_CHN               14
#define HI6XXX_INVALID_IRQ             (-1)

/* 0-earphone not pluged, 1-earphone pluged */
#define IRQ_STAT_PLUG_IN (0x1 <<  HS_DET_IRQ_OFFSET)
/* 0-normal btn event    1- no normal btn event */
#define IRQ_STAT_KEY_EVENT (0x1 << HS_MIC_NOR2_IRQ_OFFSET)
/* 0-eco btn event     1- no eco btn event */
#define IRQ_STAT_ECO_KEY_EVENT (0x1 << HS_MIC_ECO_IRQ_OFFSET)
#define JACK_RPT_MSK_BTN (SND_JACK_BTN_0 | SND_JACK_BTN_1 | SND_JACK_BTN_2 | SND_JACK_BTN_3)

enum jack_states {
	PLUG_NONE           = 0,
	PLUG_HEADSET        = 1,
	PLUG_HEADPHONE      = 2,
	PLUG_INVERT         = 3,
	PLUGING             = 4,
	PLUG_INVALID        = 5,
	PLUG_LINEOUT        = 6,
};

enum irq_action_type {
	IRQ_PLUG_OUT = 0,
	IRQ_PLUG_IN,
	IRQ_ECO_BTN_DOWN,
	IRQ_ECO_BTN_UP,
	IRQ_COMP_L_BTN_DOWN,
	IRQ_COMP_L_BTN_UP,
	IRQ_COMP_H_BTN_DOWN,
	IRQ_COMP_H_BTN_UP,
	IRQ_ACTION_NUM,
};

enum delay_action_type {
	DELAY_MICBIAS_ENABLE,
	DELAY_LINEOUT_RECHECK,
	DELAY_ACTION_NUM,
};

enum hs_event_voltage_type {
	VOLTAGE_POLE3 = 0,
	VOLTAGE_POLE4,
	VOLTAGE_PLAY,
	VOLTAGE_VOL_UP,
	VOLTAGE_VOL_DOWN,
	VOLTAGE_VOICE_ASSIST,
	VOLTAGE_NUM,
};

enum delay_time {
	/* ms */
	HS_TIME_PI              = 0,
	HS_TIME_PI_DETECT       = 800,
	HS_TIME_COMP_IRQ_TRIG   = 30,
	HS_TIME_COMP_IRQ_TRIG_2 = 50,
};

struct mbhc_wq {
	struct workqueue_struct *dwq;
	struct delayed_work dw;
};

struct jack_data {
	struct snd_soc_jack jack;
	int report;
	struct switch_dev sdev;
	bool is_dev_registered;
};

struct hs_handler_config {
	void (*handler)(struct work_struct *work);
	const char *name;
};

struct voltage_interval {
	int min;
	int max;
};

struct mbhc_data {
	struct hi6xxx_mbhc mbhc_pub;
	struct snd_soc_codec *codec;
	struct jack_data hs_jack;
	enum jack_states hs_status;
	enum jack_states old_hs_status;
	int pressed_btn_type;
	int adc_voltage;

	struct mbhc_wq irq_wqs[IRQ_ACTION_NUM];
	struct mbhc_wq delay_wqs[DELAY_ACTION_NUM];

	struct mutex io_mutex;
	struct mutex hkadc_mutex;
	struct mutex plug_mutex;
	struct mutex hs_micbias_mutex;
	struct wakeup_source wake_lock;

	unsigned int hs_micbias_dapm;
	bool hs_micbias_mbhc;
	bool pre_status_is_lineout;
	bool usb_ana_need_recheck;
	int gpio_intr_pin;
	int gpio_irq;

	struct voltage_interval intervals[VOLTAGE_NUM];
	struct hi6xxx_mbhc_ops mbhc_ops;
};

static const char *const mbhc_print_str[] = {
	"pole3", "pole4", "play", "vol_up", "vol_down", "voice_assist",
};

struct voltage_dts_node {
	const char *min;
	const char *max;
};

struct voltage_dts_node voltage_config[VOLTAGE_NUM] = {
	{ "hisilicon,hs_3_pole_min_voltage", "hisilicon,hs_3_pole_max_voltage" },
	{ "hisilicon,hs_4_pole_min_voltage", "hisilicon,hs_4_pole_max_voltage" },
	{ "hisilicon,btn_play_min_voltage", "hisilicon,btn_play_max_voltage" },
	{ "hisilicon,btn_volume_up_min_voltage", "hisilicon,btn_volume_up_max_voltage" },
	{ "hisilicon,btn_volume_down_min_voltage", "hisilicon,btn_volume_down_max_voltage" },
	{ "hisilicon,btn_voice_assistant_min_voltage", "hisilicon,btn_voice_assistant_max_voltage" }
};

static inline unsigned int irq_status_check(const struct mbhc_data *priv,
	unsigned int irq_stat_bit)
{
	unsigned int irq_state;
	unsigned int ret;

	irq_state = snd_soc_read(priv->codec, ANA_IRQ_SIG_STAT_REG);
	irq_state &= irq_stat_bit;

	switch (irq_stat_bit) {
	case IRQ_STAT_KEY_EVENT:
	case IRQ_STAT_ECO_KEY_EVENT:
		/* convert */
		ret = !irq_state;
		break;
	case IRQ_STAT_PLUG_IN:
		if (check_usb_analog_hs_support() || ana_hs_support_usb_sw()) {
			if(usb_analog_hs_check_headset_pluged_in() == ANA_HS_PLUG_IN ||
				ana_hs_pluged_state() == ANA_HS_PLUG_IN) {
				ret = ANA_HS_PLUG_IN;
				logi("usb ananlog hs is IRQ_PLUG_IN\n");
			} else {
				ret = ANA_HS_PLUG_OUT;
				logi("usb ananlog hs is IRQ_PLUG_OUT\n");
			}
		} else {
			ret = irq_state;
		}
		break;
	default:
		/* no need to convert */
		ret = irq_state;
		break;
	}
	logd("bit=0x%x, ret=%d\n", irq_stat_bit, ret);

	return ret;
}

static inline void irqs_clr(struct mbhc_data *priv,
	unsigned int irqs)
{
	logd("Before irqs clr,IRQ_REG0=0x%x, clr=0x%x\n",
		snd_soc_read(priv->codec, ANA_IRQ_REG0_REG), irqs);
	snd_soc_write(priv->codec, ANA_IRQ_REG0_REG, irqs);
	logd("After irqs clr,IRQ_REG0=0x%x\n",
		snd_soc_read(priv->codec, ANA_IRQ_REG0_REG));
}

static inline void irqs_mask_set(struct mbhc_data *priv,
	unsigned int irqs)
{
	logd("Before mask set,IRQM_REG0=0x%x, mskset=0x%x\n",
		snd_soc_read(priv->codec, ANA_IRQM_REG0_REG), irqs);
	snd_soc_update_bits(priv->codec, ANA_IRQM_REG0_REG, irqs, 0xff);
	logd("After mask set,IRQM_REG0=0x%x\n",
		snd_soc_read(priv->codec, ANA_IRQM_REG0_REG));
}

static inline void irqs_mask_clr(struct mbhc_data *priv,
	unsigned int irqs)
{
	logd("Before mask clr,IRQM_REG0=0x%x, mskclr=0x%x\n",
		snd_soc_read(priv->codec, ANA_IRQM_REG0_REG), irqs);
	snd_soc_update_bits(priv->codec, ANA_IRQM_REG0_REG, irqs, 0);
	logd("After mask clr,IRQM_REG0=0x%x\n",
		snd_soc_read(priv->codec, ANA_IRQM_REG0_REG));
}

static void hs_micbias_power(struct mbhc_data *priv, bool enable)
{
	unsigned int irq_mask;

	/* to avoid irq while MBHD_COMP power up, mask all COMP irq,when pwr up finished clean it and cancel mask */
	irq_mask = snd_soc_read(priv->codec, ANA_IRQM_REG0_REG);
	irqs_mask_set(priv, irq_mask | IRQ_MSK_COMP);

	if (enable) {
		/* open ibias */
		priv->mbhc_ops.set_ibias_hsmicbias(priv->codec, true);

		/* disable ECO--key detect mode switch to NORMAL mode */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG,
			BIT(MBHD_ECO_EN_OFFSET), 0);

		/* micbias discharge off */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW7_REG,
			BIT(HSMICB_DSCHG_OFFSET), 0);

		/* hs micbias pu */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW7_REG,
			BIT(HSMICB_PD_OFFSET), 0);
		msleep(10);
		/* enable NORMAL key detect and identify:1.open normal compare 2.key identify adc voltg buffer on */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG,
			BIT(MBHD_COMP_PD_OFFSET), 0);
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG,
			BIT(MBHD_BUFF_PD_OFFSET), 0);
		usleep_range(100, 150);
	} else {
		/* disable NORMAL key detect and identify */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG,
			BIT(MBHD_BUFF_PD_OFFSET), 0xff);
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG,
			BIT(MBHD_COMP_PD_OFFSET), 0xff);

		/* hs micbias pd */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW7_REG,
			BIT(HSMICB_PD_OFFSET), 0xff);
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW7_REG,
			BIT(HSMICB_DSCHG_OFFSET), 0xff);
		msleep(5);
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW7_REG,
			BIT(HSMICB_DSCHG_OFFSET), 0);

		/* key detect mode switch to ECO mode */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG,
			BIT(MBHD_ECO_EN_OFFSET), 0xff);
		msleep(20);

		/* close ibias */
		priv->mbhc_ops.set_ibias_hsmicbias(priv->codec, false);
		irqs_clr(priv, IRQ_MSK_COMP);
		irqs_mask_clr(priv, IRQ_MSK_BTN_ECO);
	}
}

static void set_hs_micbias(struct mbhc_data *priv, bool enable)
{
	logi("begin,en = %d\n", enable);
	WARN_ON(!priv);

	/* hs_micbias power up,then power down 3 seconds later */
	cancel_delayed_work(&priv->delay_wqs[DELAY_MICBIAS_ENABLE].dw);
	flush_workqueue(priv->delay_wqs[DELAY_MICBIAS_ENABLE].dwq);

	if (enable) {
		/* read hs_micbias pd status,1:pd */
		if (((snd_soc_read(priv->codec, CODEC_ANA_RW7_REG))&(BIT(HSMICB_PD_OFFSET))))
			hs_micbias_power(priv, true);
	} else {
		if ((priv->hs_micbias_dapm == 0) && !priv->hs_micbias_mbhc) {
			__pm_wakeup_event(&priv->wake_lock, MICBIAS_PD_WAKE_LOCK_MS);
			mod_delayed_work(priv->delay_wqs[DELAY_MICBIAS_ENABLE].dwq,
					&priv->delay_wqs[DELAY_MICBIAS_ENABLE].dw,
					msecs_to_jiffies(MICBIAS_PD_DELAY_MS));
		}
	}
}

void hi6xxx_mbhc_set_micbias(struct hi6xxx_mbhc *mbhc, bool enable)
{
	struct mbhc_data *priv = (struct mbhc_data *)mbhc;

	IN_FUNCTION;

	if (priv == NULL) {
		loge("priv is null\n");
		return;
	}

	mutex_lock(&priv->hs_micbias_mutex);
	if (enable) {
		if (priv->hs_micbias_dapm == 0)
			set_hs_micbias(priv, true);

		if (priv->hs_micbias_dapm == MAX_UINT32) {
			loge("hs_micbias_dapm will overflow\n");
			mutex_unlock(&priv->hs_micbias_mutex);
			return;
		}
		++priv->hs_micbias_dapm;
	} else {
		if (priv->hs_micbias_dapm == 0) {
			loge("hs_micbias_dapm is 0, fail to disable micbias\n");
			mutex_unlock(&priv->hs_micbias_mutex);
			return;
		}

		--priv->hs_micbias_dapm;
		if (priv->hs_micbias_dapm == 0)
			set_hs_micbias(priv, false);
	}
	mutex_unlock(&priv->hs_micbias_mutex);

	OUT_FUNCTION;
}

static void hs_micbias_mbhc_enable(struct mbhc_data *priv, bool enable)
{
	IN_FUNCTION;

	WARN_ON(!priv);

	mutex_lock(&priv->hs_micbias_mutex);
	if (enable) {
		if (!priv->hs_micbias_mbhc) {
			set_hs_micbias(priv, true);
			priv->hs_micbias_mbhc = true;
		}
	} else {
		if (priv->hs_micbias_mbhc) {
			priv->hs_micbias_mbhc = false;
			set_hs_micbias(priv, false);
		}
	}
	mutex_unlock(&priv->hs_micbias_mutex);

	OUT_FUNCTION;
}

static void hs_micbias_delay_pd_work(struct work_struct *work)
{
	struct mbhc_data *priv = container_of(work, struct mbhc_data, delay_wqs[DELAY_MICBIAS_ENABLE].dw.work);

	IN_FUNCTION;

	WARN_ON(!priv);

	hs_micbias_power(priv, false);

	OUT_FUNCTION;
}

static inline int read_hkadc_value(struct mbhc_data *priv)
{
	int hkadc_value;

	WARN_ON(!priv);

	priv->adc_voltage = hisi_adc_get_value(HI6XXX_HKADC_CHN);
	if (priv->adc_voltage < 0)
		return -EFAULT;

	/* HKADC voltage, real value should devided 0.6 */
	hkadc_value = ((priv->adc_voltage)*(10))/(6);
	logi("adc_voltage = %d\n", priv->adc_voltage);

	return hkadc_value;
}

static irqreturn_t irq_handler(int irq, void *data)
{
	struct mbhc_data *priv = NULL;
	unsigned int i;
	unsigned int irqs;
	unsigned int irq_mask;
	unsigned int irq_masked;

	unsigned int irq_time[IRQ_ACTION_NUM] = {
		HS_TIME_PI, HS_TIME_PI_DETECT,
		HS_TIME_COMP_IRQ_TRIG, HS_TIME_COMP_IRQ_TRIG_2,
		HS_TIME_COMP_IRQ_TRIG, HS_TIME_COMP_IRQ_TRIG_2,
		HS_TIME_COMP_IRQ_TRIG, HS_TIME_COMP_IRQ_TRIG_2,
	};

	logd(">>>>>Begin\n");
	if (data == NULL) {
		loge("data is null\n");
		return IRQ_NONE;
	}

	priv = (struct mbhc_data *)data;

	irqs = snd_soc_read(priv->codec, ANA_IRQ_REG0_REG);
	if (irqs == 0)
		return IRQ_NONE;

	irq_mask = snd_soc_read(priv->codec, ANA_IRQM_REG0_REG);
	irq_mask &= (~IRQ_MSK_PLUG_IN);
	irq_masked = irqs & (~irq_mask);

	if (irq_masked == 0) {
		irqs_clr(priv, irqs);
		return IRQ_HANDLED;
	}

	__pm_wakeup_event(&priv->wake_lock, IRQ_HANDLE_WAKE_LOCK_MS);

	for (i = IRQ_PLUG_OUT; i < IRQ_ACTION_NUM; i++) {
		if (irq_masked & BIT(IRQ_ACTION_NUM - i - 1)) {
			queue_delayed_work(priv->irq_wqs[i].dwq,
					&priv->irq_wqs[i].dw,
					msecs_to_jiffies(irq_time[i]));
		}
	}

	/* clear all read irq bits */
	irqs_clr(priv, irqs);
	logd("<<<End, irq_masked=0x%x,irq_read=0x%x, irq_aftclr=0x%x, IRQM=0x%x, IRQ_RAW=0x%x\n",
			irq_masked, irqs,
			snd_soc_read(priv->codec, ANA_IRQ_REG0_REG),
			snd_soc_read(priv->codec, ANA_IRQM_REG0_REG),
			snd_soc_read(priv->codec, ANA_IRQ_SIG_STAT_REG));

	return IRQ_HANDLED;
}

static_t void hs_jack_report(struct mbhc_data *priv)
{
	int jack_report = 0;

	switch (priv->hs_status) {
	case PLUG_NONE:
		jack_report = 0;
		logi("plug out\n");
		break;
	case PLUG_HEADSET:
		jack_report = SND_JACK_HEADSET;
		logi("4-pole headset plug in\n");
		break;
	case PLUG_INVERT:
		jack_report = SND_JACK_HEADPHONE;
		logi("invert headset plug in\n");
		break;
	case PLUG_HEADPHONE:
		jack_report = SND_JACK_HEADPHONE;
		logi("3-pole headphone plug in\n");
		break;
	default:
		loge("error hs_status(%d)\n", priv->hs_status);
		return;
	}

	/* report jack status */
	snd_soc_jack_report(&priv->hs_jack.jack, jack_report, SND_JACK_HEADSET);
#ifdef CONFIG_SWITCH
	switch_set_state(&priv->hs_jack.sdev, priv->hs_status);
#endif
}

static void hs_type_recognize(struct mbhc_data *priv, int hkadc_value)
{
	if (hkadc_value <= priv->intervals[VOLTAGE_POLE3].max) {
		priv->hs_status = PLUG_HEADPHONE;
		logi("headphone is 3 pole, saradc=%d\n", hkadc_value);
	} else if (hkadc_value >= priv->intervals[VOLTAGE_POLE4].min &&
			hkadc_value <= priv->intervals[VOLTAGE_POLE4].max) {
		priv->hs_status = PLUG_HEADSET;
		logi("headphone is 4 pole, saradc=%d\n", hkadc_value);
	} else if (hkadc_value >= priv->intervals[VOLTAGE_POLE4].max) {
		priv->hs_status = PLUG_LINEOUT;
		logi("headphone is lineout, saradc=%d\n", hkadc_value);
	} else {
		priv->hs_status = PLUG_INVERT;
		logi("headphone is invert 4 pole, saradc=%d\n", hkadc_value);
	}
}

static void hs_plug_out_detect(struct mbhc_data *priv)
{
	IN_FUNCTION;
	WARN_ON(!priv);

	mutex_lock(&priv->plug_mutex);
	/*
	 * Avoid hs_micbias_delay_pd_dw waiting for entering eco,
	 * so cancel the delay work then power off hs_micbias.
	 */
	cancel_delayed_work(&priv->delay_wqs[DELAY_MICBIAS_ENABLE].dw);
	flush_workqueue(priv->delay_wqs[DELAY_MICBIAS_ENABLE].dwq);
	priv->hs_micbias_mbhc = false;
	hs_micbias_power(priv, false);

	/* mbhc vref pd */
	snd_soc_update_bits(priv->codec, HI6XXX_MBHD_VREF_CTRL,
		BIT(MBHD_VREF_PD_OFFSET), 0xff);

	/* disable ECO */
	snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG,
		BIT(MBHD_ECO_EN_OFFSET), 0);

	/* mask all btn irq */
	irqs_mask_set(priv, IRQ_MSK_COMP);
	mutex_lock(&priv->io_mutex);
	priv->hs_jack.report = 0;

	if (priv->pressed_btn_type != 0) {
		priv->hs_jack.jack.jack->type = priv->pressed_btn_type;
		/* report key event */
		logi("report type=0x%x, status=0x%x\n",
			priv->hs_jack.report,
			priv->hs_status);
		snd_soc_jack_report(&priv->hs_jack.jack,
			priv->hs_jack.report, JACK_RPT_MSK_BTN);
	}

	priv->pressed_btn_type = 0;
	priv->hs_status = PLUG_NONE;
	priv->old_hs_status = PLUG_INVALID;
	mutex_unlock(&priv->io_mutex);

	priv->pre_status_is_lineout = false;

	/* report headset info */
	hs_jack_report(priv);
	headset_auto_calib_reset_interzone();
	irqs_clr(priv, IRQ_MSK_COMP);
	irqs_mask_clr(priv, IRQ_MSK_PLUG_IN);
	mutex_unlock(&priv->plug_mutex);

	OUT_FUNCTION;
}

static void usb_ana_hs_recheck(struct mbhc_data *priv, int hkadc_value)
{
	irqs_mask_set(priv, IRQ_MSK_COMP);
	hs_micbias_power(priv, false);
	usb_ana_hs_mic_switch_change_state();
	ana_hs_mic_gnd_swap();
	msleep(10);
	hs_micbias_power(priv, true);
	mutex_lock(&priv->hkadc_mutex);
	hkadc_value = read_hkadc_value(priv);
	if (hkadc_value < 0) {
		loge("invalid hkadc: %d\n", hkadc_value);
		mutex_unlock(&priv->hkadc_mutex);
		return;
	}
	hs_type_recognize(priv, hkadc_value);
	mutex_unlock(&priv->hkadc_mutex);
}

static void hs_status_process(struct mbhc_data *priv)
{
	int hkadc_value = 0;
	bool usb_ana_hs_support =
		check_usb_analog_hs_support() || ana_hs_support_usb_sw();

	if ((priv->hs_status != PLUG_LINEOUT) &&
		(priv->hs_status != PLUG_NONE) &&
		(priv->hs_status != priv->old_hs_status)) {
		if (usb_ana_hs_support &&
			(priv->hs_status == PLUG_INVERT ||
			priv->hs_status == PLUG_HEADPHONE))
			usb_ana_hs_recheck(priv, hkadc_value);

		priv->old_hs_status = priv->hs_status;
		logi("hs status=%d, pre_status_is_lineout:%d\n",
			priv->hs_status, priv->pre_status_is_lineout);
		/* report headset info */
		hs_jack_report(priv);
	} else if (priv->hs_status == PLUG_LINEOUT) {
		priv->pre_status_is_lineout = true;
		logi("hs status=%d, old_hs_status=%d, lineout is plugin\n",
			priv->hs_status, priv->old_hs_status);
		/* not the first time recognize as lineout, headphone/set plug out from lineout */
		if (priv->old_hs_status != PLUG_INVALID) {
			priv->hs_status = PLUG_NONE;
			priv->old_hs_status = PLUG_INVALID;
			/* headphone/set plug out from lineout,need report plugout event */
			hs_jack_report(priv);
			priv->hs_status = PLUG_LINEOUT;
		}
	} else {
		logi("hs status=%d(old=%d) not changed\n", priv->hs_status, priv->old_hs_status);
		/* recheck usb headset when invert or hp && same as old */
		if (priv->usb_ana_need_recheck && usb_ana_hs_support &&
			(priv->hs_status == PLUG_INVERT ||
			priv->hs_status == PLUG_HEADPHONE)) {
			usb_ana_hs_recheck(priv, hkadc_value);
			if (priv->hs_status != priv->old_hs_status) {
				hs_jack_report(priv);
				priv->old_hs_status = priv->hs_status;
			}
		}
	}
}

static void hs_plug_in_detect(struct mbhc_data *priv)
{
	int hkadc_value;

	IN_FUNCTION;

	WARN_ON(!priv);

	/* check state - plugin */
	if (!irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		logi("plug_in SIG STAT: not plug in, irq_state=0x%x, IRQM=0x%x, RAW_irq =0x%x\n",
				snd_soc_read(priv->codec, ANA_IRQ_REG0_REG),
				snd_soc_read(priv->codec, ANA_IRQM_REG0_REG),
				snd_soc_read(priv->codec, ANA_IRQ_SIG_STAT_REG));

		hs_plug_out_detect(priv);
		return;
	}

	mutex_lock(&priv->plug_mutex);
	/* mask plug out */
	irqs_mask_set(priv, IRQ_MSK_PLUG_OUT | IRQ_MSK_COMP);
	mutex_lock(&priv->hkadc_mutex);
	snd_soc_update_bits(priv->codec, HI6XXX_MBHD_VREF_CTRL,
		BIT(MBHD_VREF_PD_OFFSET), 0);
	hs_micbias_mbhc_enable(priv, true);
	msleep(150);
	hkadc_value = read_hkadc_value(priv);

	if (hkadc_value < 0) {
		loge("get adc fail,can't read adc value\n");
		mutex_unlock(&priv->hkadc_mutex);
		mutex_unlock(&priv->plug_mutex);
		return;
	}

	/* value greater than 2565 can not trigger eco btn,
	 * so,the hs_micbias can't be closed until second detect finish.
	 */
	if ((hkadc_value <= priv->intervals[VOLTAGE_POLE4].max) &&
		(priv->pre_status_is_lineout == false))
		hs_micbias_mbhc_enable(priv, false);

	mutex_unlock(&priv->hkadc_mutex);

	mutex_lock(&priv->io_mutex);
	hs_type_recognize(priv, hkadc_value);

	irqs_clr(priv, IRQ_MSK_PLUG_OUT);
	irqs_mask_clr(priv, IRQ_MSK_PLUG_OUT);
	mutex_unlock(&priv->io_mutex);

	if (!irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		logi("plug out happens\n");
		mutex_unlock(&priv->plug_mutex);
		hs_plug_out_detect(priv);
		return;
	}

	hs_status_process(priv);

	/* to avoid irq while MBHD_COMP power up, mask the irq then clean it */
	irqs_clr(priv, IRQ_MSK_COMP);
	irqs_mask_clr(priv, IRQ_MSK_BTN_NOR);
	mutex_unlock(&priv->plug_mutex);

	OUT_FUNCTION;
}

static int hs_btn_type_recognize(const struct voltage_interval *voltages, int hkadc_value)
{
	int btn_type = 0;
	unsigned int i;
	int jacks[] = { SND_JACK_BTN_0,
		SND_JACK_BTN_1, SND_JACK_BTN_2, SND_JACK_BTN_3 };

	for (i = VOLTAGE_PLAY; i < VOLTAGE_NUM; i++) {
		if (hkadc_value >= voltages[i].min && hkadc_value <= voltages[i].max) {
			btn_type = jacks[i - VOLTAGE_PLAY];
			logi("key %s is pressed down, saradc value: %d\n",
				mbhc_print_str[i], hkadc_value);
			break;
		}
	}

	if (i == VOLTAGE_NUM)
		loge("hkadc value: %d is not in range\n", hkadc_value);

	return btn_type;
}

static void hs_btn_report(struct mbhc_data *priv, int btn_type)
{
	mutex_lock(&priv->io_mutex);
	priv->pressed_btn_type = btn_type;
	priv->hs_jack.report = btn_type;
	priv->hs_jack.jack.jack->type = btn_type;
	mutex_unlock(&priv->io_mutex);

	if (!irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		logi("plug out happened\n");
	} else {
		/* report key event */
		logi("report type=0x%x, status=0x%x\n",
			priv->hs_jack.report, priv->hs_status);
		snd_soc_jack_report(&priv->hs_jack.jack,
			priv->hs_jack.report, JACK_RPT_MSK_BTN);
	}

}

static void hs_btn_down_detect(struct mbhc_data *priv)
{
	int pr_btn_type;
	int hkadc_value;

	IN_FUNCTION;

	WARN_ON(!priv);

	if (!irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		logi("plug out happened\n");
		return;
	}

	if (priv->hs_status != PLUG_HEADSET) {
		/* enter the second detect,it's triggered by btn irq  */
		logi("enter btn_down 2nd time hp type recognize, btn_type=0x%x\n",
			priv->pressed_btn_type);
		hs_plug_in_detect(priv);
		return;
	}

	if (priv->pressed_btn_type != PLUG_NONE) {
		loge("btn_type:0x%x has been pressed\n", priv->pressed_btn_type);
		return;
	}

	/* hs_micbias power up,then power down 3 seconds later */
	cancel_delayed_work(&priv->delay_wqs[DELAY_MICBIAS_ENABLE].dw);
	flush_workqueue(priv->delay_wqs[DELAY_MICBIAS_ENABLE].dwq);

	mutex_lock(&priv->hkadc_mutex);
	hs_micbias_mbhc_enable(priv, true);
	hkadc_value = read_hkadc_value(priv);
	if (hkadc_value < 0) {
		loge("get adc fail,can't read adc value, %d\n", hkadc_value);
		mutex_unlock(&priv->hkadc_mutex);
		return;
	}

	if (!priv->pre_status_is_lineout)
		hs_micbias_mbhc_enable(priv, false);

	mutex_unlock(&priv->hkadc_mutex);
	msleep(30);
	/* micbias power up have done, now is in normal mode, clean all COMP IRQ and cancel NOR int mask */
	irqs_clr(priv, IRQ_MSK_COMP);
	irqs_mask_clr(priv, IRQ_MSK_BTN_NOR);
	logi("mask clean\n");

	pr_btn_type = hs_btn_type_recognize(priv->intervals, hkadc_value);

	if (pr_btn_type == SND_JACK_BTN_3)
		goto VOCIE_ASSISTANT_KEY;

	startup_FSM(REC_JUDGE, hkadc_value, &pr_btn_type);

VOCIE_ASSISTANT_KEY:
	hs_btn_report(priv, pr_btn_type);

	OUT_FUNCTION;
}

static void hs_btn_up_detect(struct mbhc_data *priv)
{
	IN_FUNCTION;

	WARN_ON(!priv);

	if (!irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		logi("plug out happened\n");
		return;
	}

	mutex_lock(&priv->io_mutex);
	if (priv->pressed_btn_type == 0) {
		mutex_unlock(&priv->io_mutex);

		/* second detect */
		if ((priv->hs_status == PLUG_INVERT) ||
		(priv->hs_status == PLUG_HEADPHONE)
		|| (priv->hs_status == PLUG_LINEOUT)) {
			logi("enter btn_up 2nd time hp type recognize\n");
				hs_plug_in_detect(priv);
		} else {
			logi("ignore the key up irq\n");
		}

		return;
	}

	priv->hs_jack.jack.jack->type = priv->pressed_btn_type;
	priv->hs_jack.report = 0;

	/* report key event */
	logi("report type=0x%x, status=0x%x\n",
		priv->hs_jack.report, priv->hs_status);
	snd_soc_jack_report(&priv->hs_jack.jack,
		priv->hs_jack.report, JACK_RPT_MSK_BTN);
	priv->pressed_btn_type = 0;
	mutex_unlock(&priv->io_mutex);

	OUT_FUNCTION;
}

static void lineout_rm_recheck_work(struct work_struct *work)
{
	struct mbhc_data *priv = container_of(work, struct mbhc_data, delay_wqs[DELAY_LINEOUT_RECHECK].dw.work);
	int hkadc_value;

	WARN_ON(!priv);

	IN_FUNCTION;

	mutex_lock(&priv->plug_mutex);
	if (!irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		irqs_clr(priv, IRQ_MSK_COMP);
		irqs_mask_clr(priv, IRQ_MSK_PLUG_IN);
		mutex_unlock(&priv->plug_mutex);
		logi("plugout has happened,ignore this irq\n");
		return;
	}

	mutex_lock(&priv->hkadc_mutex);
	hkadc_value = read_hkadc_value(priv);
	mutex_unlock(&priv->hkadc_mutex);
	if (hkadc_value < 0) {
		loge("get adc fail,can't read adc value\n");
		mutex_unlock(&priv->plug_mutex);
		return;
	}

	if (hkadc_value <= priv->intervals[VOLTAGE_POLE4].max) {
		/* btn_up event */
		mutex_unlock(&priv->plug_mutex);
		return;
	}

	/* report plugout event,and set hs_status to lineout mode */
	if (priv->hs_status == PLUG_LINEOUT) {
		mutex_unlock(&priv->plug_mutex);
		logi("lineout recheck, hs_status is lineout, just return\n");
		return;
	}

	priv->hs_status = PLUG_NONE;
	priv->old_hs_status = PLUG_INVALID;
	/* report plugout event */
	hs_jack_report(priv);
	priv->hs_status = PLUG_LINEOUT;
	priv->pre_status_is_lineout = true;

	irqs_clr(priv, IRQ_MSK_COMP);
	irqs_mask_clr(priv, IRQ_MSK_BTN_NOR);
	logi("lineout recheck,and report remove\n");

	mutex_unlock(&priv->plug_mutex);
}

static void hs_lineout_rm_recheck(struct mbhc_data *priv)
{
	WARN_ON(!priv);

	cancel_delayed_work(&priv->delay_wqs[DELAY_LINEOUT_RECHECK].dw);
	flush_workqueue(priv->delay_wqs[DELAY_LINEOUT_RECHECK].dwq);

	__pm_wakeup_event(&priv->wake_lock, LINEOUT_PO_RECHK_WAKE_LOCK_MS);
	mod_delayed_work(priv->delay_wqs[DELAY_LINEOUT_RECHECK].dwq,
			&priv->delay_wqs[DELAY_LINEOUT_RECHECK].dw,
			msecs_to_jiffies(LINEOUT_PO_RECHK_DELAY_MS));
}

static bool hs_lineout_plug_out(struct mbhc_data *priv)
{
	int hkadc_value;

	mutex_lock(&priv->plug_mutex);
	if (!irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		irqs_clr(priv, IRQ_MSK_COMP);
		irqs_mask_clr(priv, IRQ_MSK_PLUG_IN);
		mutex_unlock(&priv->plug_mutex);
		logi("plugout has happened,ignore this irq\n");
		return true;
	}

	mutex_lock(&priv->hkadc_mutex);
	hkadc_value = read_hkadc_value(priv);
	mutex_unlock(&priv->hkadc_mutex);
	if (hkadc_value < 0) {
		loge("get adc fail,can't read adc value, %d\n", hkadc_value);
		mutex_unlock(&priv->plug_mutex);
		return false;
	}

	if (hkadc_value <= priv->intervals[VOLTAGE_POLE4].max) {
		/* btn_up event */
		mutex_unlock(&priv->plug_mutex);
		hs_lineout_rm_recheck(priv);
		return false;
	}

	/* report plugout event,and set hs_status to lineout mode */
	mutex_lock(&priv->io_mutex);
	priv->hs_jack.report = 0;
	if (priv->pressed_btn_type != 0) {
		priv->hs_jack.jack.jack->type = priv->pressed_btn_type;
		/* report key event */
		logi("report type=0x%x, status=0x%x\n",
			priv->hs_jack.report, priv->hs_status);
		snd_soc_jack_report(&priv->hs_jack.jack,
			priv->hs_jack.report, JACK_RPT_MSK_BTN);
	}
	priv->pressed_btn_type = 0;
	mutex_unlock(&priv->io_mutex);

	priv->hs_status = PLUG_NONE;
	priv->old_hs_status = PLUG_INVALID;
	/* report plugout event */
	hs_jack_report(priv);
	priv->hs_status = PLUG_LINEOUT;
	priv->pre_status_is_lineout = true;

	irqs_clr(priv, IRQ_MSK_COMP);
	irqs_mask_clr(priv, IRQ_MSK_BTN_NOR);
	mutex_unlock(&priv->plug_mutex);
	hs_lineout_rm_recheck(priv);

	return true;
}

static void hs_pi_work(struct work_struct *work)
{
	struct mbhc_data *priv = container_of(work, struct mbhc_data, irq_wqs[IRQ_PLUG_IN].dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	hs_plug_in_detect(priv);
}

static void hs_po_work(struct work_struct *work)
{
	struct mbhc_data *priv = container_of(work, struct mbhc_data, irq_wqs[IRQ_PLUG_OUT].dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	hs_plug_out_detect(priv);
}

static void hs_comp_l_btn_down_work(struct work_struct *work)
{
	struct mbhc_data *priv = container_of(work, struct mbhc_data, irq_wqs[IRQ_COMP_L_BTN_DOWN].dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	hs_btn_down_detect(priv);
}

static void hs_comp_l_btn_up_work(struct work_struct *work)
{
	struct mbhc_data *priv = container_of(work, struct mbhc_data, irq_wqs[IRQ_COMP_L_BTN_UP].dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	hs_btn_up_detect(priv);
}

static void hs_comp_h_btn_down_work(struct work_struct *work)
{
	struct mbhc_data *priv = container_of(work, struct mbhc_data, irq_wqs[IRQ_COMP_H_BTN_DOWN].dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	if (hs_lineout_plug_out(priv)) {
		logi("hs plugout from lineout, return\n");
		return;
	}

	hs_btn_down_detect(priv);
}

static void hs_comp_h_btn_up_work(struct work_struct *work)
{
	struct mbhc_data *priv = container_of(work, struct mbhc_data, irq_wqs[IRQ_COMP_H_BTN_UP].dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	hs_btn_up_detect(priv);
}

static void hs_eco_btn_down_work(struct work_struct *work)
{
	struct mbhc_data *priv = container_of(work, struct mbhc_data, irq_wqs[IRQ_ECO_BTN_DOWN].dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	hs_btn_down_detect(priv);
}

static void hs_eco_btn_up_work(struct work_struct *work)
{
	struct mbhc_data *priv = container_of(work, struct mbhc_data, irq_wqs[IRQ_ECO_BTN_UP].dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	hs_btn_up_detect(priv);
}

static void mbhc_reg_init(struct snd_soc_codec *codec, struct mbhc_data *priv)
{
	if (priv->mbhc_ops.enable_hsd)
		priv->mbhc_ops.enable_hsd(codec);

	/* eliminate btn dithering */
	snd_soc_write(codec, DEB_CNT_HS_MIC_CFG_REG, 0x14);

	/* MBHC compare config 125mV 800mV 95% */
	snd_soc_write(codec, HI6XXX_MBHD_VREF_CTRL, 0x9E);

	/* HSMICBIAS config voltage 2.7V */
	snd_soc_write(codec, HI6XXX_HSMICB_CFG, 0x0B);

	/* clear HP MIXER channel select */
	snd_soc_write(codec, CODEC_ANA_RW20_REG, 0x0);

	/* config HP PGA gain to -20dB */
	snd_soc_write(codec, CODEC_ANA_RW21_REG, 0x0);

	/* Charge Pump clk pd, freq 768kHz */
	snd_soc_write(codec, HI6XXX_CHARGE_PUMP_CLK_PD, 0x1A);

	/* disable ECO */
	snd_soc_update_bits(codec, CODEC_ANA_RW8_REG, BIT(MBHD_ECO_EN_OFFSET), 0);

	/* HSMICBIAS PD */
	snd_soc_update_bits(codec, CODEC_ANA_RW7_REG,
		BIT(HSMICB_PD_OFFSET), 0xff);

	/* avoid irq triggered while codec power up */
	snd_soc_update_bits(codec, ANA_IRQM_REG0_REG, IRQ_MSK_HS_ALL, 0xff);
	snd_soc_write(codec, ANA_IRQ_REG0_REG, IRQ_MSK_HS_ALL);
}

static void mbhc_config_print(const struct voltage_interval *intervals)
{
	unsigned int i;

	for (i = VOLTAGE_POLE3; i < VOLTAGE_NUM; i++)
		logi("headset voltage %s min: %d, max: %d\n", mbhc_print_str[i],
			intervals[i].min, intervals[i].max);
}

static void mbhc_config_init(const struct device_node *np,
	struct voltage_interval *intervals)
{
	unsigned int i;
	unsigned int val = 0;

	intervals[VOLTAGE_VOICE_ASSIST].min = -1;
	intervals[VOLTAGE_VOICE_ASSIST].max = -1;
	for (i = VOLTAGE_POLE3; i < VOLTAGE_NUM; i++) {
		if (!of_property_read_u32(np, voltage_config[i].min, &val))
			intervals[i].min = val;
		if (!of_property_read_u32(np, voltage_config[i].max, &val))
			intervals[i].max = val;
	}

	mbhc_config_print(intervals);
}

static const struct hs_handler_config irq_wq_cfgs[IRQ_ACTION_NUM] = {
	{ hs_po_work, "hs_po_dwq" },
	{ hs_pi_work, "hs_pi_dwq" },
	{ hs_eco_btn_down_work, "hs_eco_btn_down_dwq" },
	{ hs_eco_btn_up_work, "hs_eco_btn_up_dwq" },
	{ hs_comp_l_btn_down_work, "hs_comp_l_btn_down_dwq" },
	{ hs_comp_l_btn_up_work, "hs_comp_l_btn_up_dwq" },
	{ hs_comp_h_btn_down_work, "hs_comp_h_btn_down_dwq" },
	{ hs_comp_h_btn_up_work, "hs_comp_h_btn_up_dwq" },
};

static const struct hs_handler_config delay_wq_cfgs[DELAY_ACTION_NUM] = {
	{ hs_micbias_delay_pd_work, "hs_micbias_delay_pd_dwq" },
	{ lineout_rm_recheck_work, "lineout_po_recheck_dwq" },
};

static int mbhc_wq_init(struct mbhc_data *priv)
{
	unsigned int i;

	for (i = IRQ_PLUG_OUT; i < IRQ_ACTION_NUM; i++) {
		priv->irq_wqs[i].dwq = create_singlethread_workqueue(irq_wq_cfgs[i].name);
		if (priv->irq_wqs[i].dwq == NULL) {
			loge("%s workqueue create failed\n", irq_wq_cfgs[i].name);
			return -EFAULT;
		}
		INIT_DELAYED_WORK(&priv->irq_wqs[i].dw, irq_wq_cfgs[i].handler);
	}

	for (i = DELAY_MICBIAS_ENABLE; i < DELAY_ACTION_NUM; i++) {
		priv->delay_wqs[i].dwq = create_singlethread_workqueue(delay_wq_cfgs[i].name);
		if (priv->delay_wqs[i].dwq == NULL) {
			loge("%s workqueue create failed\n", delay_wq_cfgs[i].name);
			return -EFAULT;
		}
		INIT_DELAYED_WORK(&priv->delay_wqs[i].dw, delay_wq_cfgs[i].handler);
	}

	return 0;
}

static void mbhc_wq_deinit(struct mbhc_data *priv)
{
	unsigned int i;

	for (i = IRQ_PLUG_OUT; i < IRQ_ACTION_NUM; i++) {
		if (priv->irq_wqs[i].dwq != NULL) {
			cancel_delayed_work(&priv->irq_wqs[i].dw);
			flush_workqueue(priv->irq_wqs[i].dwq);
			destroy_workqueue(priv->irq_wqs[i].dwq);
			priv->irq_wqs[i].dwq = NULL;
		}
	}

	for (i = DELAY_MICBIAS_ENABLE; i < DELAY_ACTION_NUM; i++) {
		if (priv->delay_wqs[i].dwq != NULL) {
			cancel_delayed_work(&priv->delay_wqs[i].dw);
			flush_workqueue(priv->delay_wqs[i].dwq);
			destroy_workqueue(priv->delay_wqs[i].dwq);
			priv->delay_wqs[i].dwq = NULL;
		}
	}
}

static int register_hs_jack_btn(struct mbhc_data *priv)
{
	/* Headset jack */
	int ret = snd_soc_card_jack_new(priv->codec->component.card, "Headset Jack",
		SND_JACK_HEADSET, (&priv->hs_jack.jack), NULL, 0);
	if (ret) {
		loge("jack new error, ret = %d\n", ret);
		return ret;
	}

	/* set a key mapping on a jack */
	ret = snd_jack_set_key(priv->hs_jack.jack.jack, SND_JACK_BTN_0, KEY_MEDIA);
	if (ret) {
		loge("jack set key(0x%x) error, ret = %d\n", SND_JACK_BTN_0, ret);
		return ret;
	}

	ret = snd_jack_set_key(priv->hs_jack.jack.jack, SND_JACK_BTN_1, KEY_VOLUMEUP);
	if (ret) {
		loge("jack set key(0x%x) error, ret = %d\n", SND_JACK_BTN_1, ret);
		return ret;
	}

	ret = snd_jack_set_key(priv->hs_jack.jack.jack, SND_JACK_BTN_2, KEY_VOLUMEDOWN);
	if (ret) {
		loge("jack set key(0x%x) error, ret = %d\n", SND_JACK_BTN_2, ret);
		return ret;
	}

	ret = snd_jack_set_key(priv->hs_jack.jack.jack, SND_JACK_BTN_3, KEY_VOICECOMMAND);
	if (ret) {
		loge("jack set key(0x%x) error, ret = %d\n", SND_JACK_BTN_3, ret);
		return ret;
	}

	return 0;
}

static void plug_in_detect(void *priv)
{
	struct mbhc_data *private_data = (struct mbhc_data *)priv;

	hs_plug_in_detect(private_data);
}

static void plug_out_detect(void *priv)
{
	struct mbhc_data *private_data = (struct mbhc_data *)priv;

	hs_plug_out_detect(private_data);
}

static void hs_high_resistence_enable(void *priv, bool enable)
{
	UNUSED_PARAMETER(priv);
	if (enable)
		logi("change codec hs resistance high to reduce pop noise\n");
	else
		logi("reset codec hs resistance to default\n");
}

static struct ana_hs_codec_dev usb_analog_dev = {
	.name = "usb_analog_hs",
	.ops = {
		.check_headset_in = NULL,
		.plug_in_detect = plug_in_detect,
		.plug_out_detect = plug_out_detect,
		.get_headset_type = NULL,
		.hs_high_resistence_enable = hs_high_resistence_enable,
	},
};

static void mbhc_lock_init(struct mbhc_data *priv)
{
	mutex_init(&priv->io_mutex);
	mutex_init(&priv->hs_micbias_mutex);
	mutex_init(&priv->hkadc_mutex);
	mutex_init(&priv->plug_mutex);
	wakeup_source_init(&priv->wake_lock, "hi6xxx_mbhc");
}

static void mbhc_lock_deinit(struct mbhc_data *priv)
{
	mutex_destroy(&priv->io_mutex);
	mutex_destroy(&priv->hs_micbias_mutex);
	mutex_destroy(&priv->hkadc_mutex);
	mutex_destroy(&priv->plug_mutex);
	wakeup_source_trash(&priv->wake_lock);
}

static void mbhc_priv_init(struct snd_soc_codec *codec, struct mbhc_data *priv, const struct hi6xxx_mbhc_ops *ops)
{
	priv->codec = codec;
	mbhc_lock_init(priv);
	mbhc_config_init(codec->dev->of_node, priv->intervals);

	priv->hs_status = PLUG_NONE;
	priv->old_hs_status = PLUG_INVALID;
	priv->hs_jack.report = 0;
	priv->pressed_btn_type = 0;
	priv->pre_status_is_lineout = false;
	priv->hs_micbias_mbhc = false;
	priv->hs_micbias_dapm = 0;

	memcpy(&priv->mbhc_ops, ops, sizeof(priv->mbhc_ops)); /* unsafe_function_ignore: memset memcpy */
}

static void mbhc_ana_usb_cfg_get(struct mbhc_data *priv, struct device_node *np)
{
	unsigned int val = 0;

	priv->usb_ana_need_recheck = false;
	if (of_property_read_u32(np, "usb_ana_need_recheck", &val))
		return;
	if (val != 0)
		priv->usb_ana_need_recheck = true;
}

static int mbhc_gpio_irq_init(struct mbhc_data *priv, struct device_node *np)
{
	int ret;

	priv->gpio_intr_pin = of_get_named_gpio(np, "gpios", 0);
	if (!gpio_is_valid(priv->gpio_intr_pin)) {
		loge("gpio_intr_pin gpio:%d is invalied\n", priv->gpio_intr_pin);
		ret = -EINVAL;
		goto gpio_pin_err;
	}

	/* this gpio is shared by pmu and acore, and by quested in pmu, so need not requet here */
	priv->gpio_irq = gpio_to_irq(priv->gpio_intr_pin);
	if (priv->gpio_irq < 0) {
		loge("gpio_to_irq err, gpio_irq=%d, gpio_intr_pin=%d\n",
			priv->gpio_irq, priv->gpio_intr_pin);
		ret = -EINVAL;
		goto gpio_to_irq_err;
	}
	logi("gpio_to_irq succ, gpio_irq=%d, gpio_intr_pin=%d\n",
		priv->gpio_irq, priv->gpio_intr_pin);

	/* irq shared with pmu */
	ret = request_irq(priv->gpio_irq, irq_handler,
		IRQF_TRIGGER_LOW | IRQF_SHARED | IRQF_NO_SUSPEND, "codec_irq", priv);
	if (ret) {
		loge("request_irq failed, ret = %d\n", ret);
		goto gpio_to_irq_err;
	}

	return ret;

gpio_to_irq_err:
	priv->gpio_irq = HI6XXX_INVALID_IRQ;
gpio_pin_err:
	priv->gpio_intr_pin = ARCH_NR_GPIOS;

	return ret;
}

static int mbhc_switch_init(struct mbhc_data *priv)
{
#ifdef CONFIG_SWITCH
	int ret;

	priv->hs_jack.sdev.name = "h2w";
	ret = switch_dev_register(&(priv->hs_jack.sdev));
	if (ret) {
		loge("Registering switch device error, ret=%d\n", ret);
		return ret;
	}
	priv->hs_jack.is_dev_registered = true;
#endif

	return 0;
}

static void mbhc_switch_deinit(struct mbhc_data *priv)
{
#ifdef CONFIG_SWITCH
	if (priv->hs_jack.is_dev_registered) {
		switch_dev_unregister(&(priv->hs_jack.sdev));
		priv->hs_jack.is_dev_registered = false;
	}
#endif
}

static void mbhc_detect_initial_state(struct mbhc_data *priv)
{
	/* detect headset present or not */
	logi("irq soure stat %#04x",
		snd_soc_read(priv->codec, ANA_IRQ_SIG_STAT_REG));

	if (irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		if (check_usb_analog_hs_support())
			usb_analog_hs_plug_in_out_handle(ANA_HS_PLUG_IN);
		else if (ana_hs_support_usb_sw())
			ana_hs_plug_handle(ANA_HS_PLUG_IN);
		else
			hs_plug_in_detect(priv);
	} else {
		irqs_mask_clr(priv, IRQ_MSK_PLUG_IN);
	}
}

int hi6xxx_mbhc_init(struct snd_soc_codec *codec, struct hi6xxx_mbhc **mbhc, const struct hi6xxx_mbhc_ops *ops)
{
	int ret;
	struct mbhc_data *priv = NULL;

	logi("Begin\n");

	if (codec == NULL || mbhc == NULL || ops == NULL) {
		loge("invalid input parameters\n");
		return -EINVAL;
	}

	priv = devm_kzalloc(codec->dev, sizeof(*priv), GFP_KERNEL);
	if (priv == NULL) {
		loge("priv devm_kzalloc failed\n");
		return -ENOMEM;
	}

	priv->codec = codec;
	mbhc_priv_init(codec, priv, ops);
	mbhc_reg_init(codec, priv);

	ret = usb_analog_hs_dev_register(&usb_analog_dev, priv);
	if (ret)
		loge("Not support usb analog headset\n");

	ret = ana_hs_codec_dev_register(&usb_analog_dev, priv);
	if (ret)
		loge("ana_hs_codec_dev_register fail, ignore\n");

	ret = register_hs_jack_btn(priv);
	if (ret)
		goto jack_err;

	ret = mbhc_switch_init(priv);
	if (ret)
		goto jack_err;


	ret = mbhc_wq_init(priv);
	if (ret)
		goto wq_init_err;

	ret = mbhc_gpio_irq_init(priv, codec->dev->of_node);
	if (ret)
		goto wq_init_err;

	mbhc_ana_usb_cfg_get(priv, codec->dev->of_node);
	logi("priv->usb_ana_need_recheck %d\n", priv->usb_ana_need_recheck);

	mbhc_detect_initial_state(priv);
	headset_auto_calib_init(codec->dev->of_node);

	*mbhc = &priv->mbhc_pub;

	goto end;

wq_init_err:
	mbhc_wq_deinit(priv);
	mbhc_switch_deinit(priv);

jack_err:
	mbhc_lock_deinit(priv);
end:
	logi("End\n");
	return ret;
}

void hi6xxx_mbhc_deinit(struct hi6xxx_mbhc *mbhc)
{
	struct mbhc_data *priv = (struct mbhc_data *)mbhc;

	IN_FUNCTION;

	if (priv == NULL) {
		loge("priv is NULL\n");
		return;
	}

	mbhc_lock_deinit(priv);


	mbhc_switch_deinit(priv);
	mbhc_wq_deinit(priv);

	if (priv->gpio_irq >= 0) {
		free_irq(priv->gpio_irq, priv);
		priv->gpio_irq = HI6XXX_INVALID_IRQ;
	}

	OUT_FUNCTION;
}

MODULE_DESCRIPTION("hi6xxx_mbhc");
MODULE_AUTHOR("wangqi <wangqi55@hisilicon.com>");
MODULE_LICENSE("GPL");

