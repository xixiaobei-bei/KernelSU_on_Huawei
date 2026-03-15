/*
 * hi6xxx_mbhc.c -- hi6xxx mbhc driver
 *
 * Copyright (c) 2017 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
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
#include <linux/hisi/hi6xxx/hi6xxx_mbhc.h>
#include "hisi/hs_auto_calib/hs_auto_calib.h"

#include "hisi/hi6555v2_utility.h"
#include "hisi/hi6555v2_pmu_reg_def.h"
#include "hisi/hi6555v2.h"




#define LOG_TAG "hi6xxx_mbhc"

static inline unsigned int hi6xxx_irq_status_check(struct hi6xxx_mbhc_priv *priv, unsigned int irq_stat_bit)
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
	default:
		/* no need to convert */
		ret = irq_state;
		break;
	}
	logd("bit=0x%x, ret=%d\n", irq_stat_bit, ret);

	return ret;
}

static inline void hi6xxx_irqs_clr(struct hi6xxx_mbhc_priv *priv, unsigned int irqs)
{
	logd("Before irqs clr,IRQ_REG0=0x%x, clr=0x%x\n", snd_soc_read(priv->codec, ANA_IRQ_REG0_REG), irqs);
	snd_soc_write(priv->codec, ANA_IRQ_REG0_REG, irqs);
	logd("After irqs clr,IRQ_REG0=0x%x\n", snd_soc_read(priv->codec, ANA_IRQ_REG0_REG));
}

static inline void hi6xxx_irqs_mask_set(struct hi6xxx_mbhc_priv *priv, unsigned int irqs)
{
	logd("Before mask set,IRQM_REG0=0x%x, mskset=0x%x\n", snd_soc_read(priv->codec, ANA_IRQM_REG0_REG), irqs);
	snd_soc_update_bits(priv->codec, ANA_IRQM_REG0_REG, irqs, 0xff);
	logd("After mask set,IRQM_REG0=0x%x\n", snd_soc_read(priv->codec, ANA_IRQM_REG0_REG));
}

static inline void hi6xxx_irqs_mask_clr(struct hi6xxx_mbhc_priv *priv, unsigned int irqs)
{
	logd("Before mask clr,IRQM_REG0=0x%x, mskclr=0x%x\n", snd_soc_read(priv->codec, ANA_IRQM_REG0_REG), irqs);
	snd_soc_update_bits(priv->codec, ANA_IRQM_REG0_REG, irqs, 0);
	logd("After mask clr,IRQM_REG0=0x%x\n", snd_soc_read(priv->codec, ANA_IRQM_REG0_REG));
}

static void hi6xxx_hs_micbias_power(struct hi6xxx_mbhc_priv *priv, bool enable)
{
	unsigned int irq_mask;

	/* to avoid irq while MBHD_COMP power up, mask all COMP irq,when pwr up finished clean it and cancel mask */
	irq_mask = snd_soc_read(priv->codec, ANA_IRQM_REG0_REG);
	hi6xxx_irqs_mask_set(priv, irq_mask | IRQ_MSK_COMP);

	if (enable) {
		/*open ibias*/
		ibias_hsmicbias_enable(priv->codec, true);

		/* disable ECO--key detect mode switch to NORMAL mode */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG, BIT(MBHD_ECO_EN_OFFSET), 0);

		/* micbias discharge off */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW7_REG, BIT(HSMICB_DSCHG_OFFSET), 0);

		/* hs micbias pu */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW7_REG, BIT(HSMICB_PD_OFFSET), 0);
		msleep(10);

		/* enable NORMAL key detect and identify:1.open normal compare 2.key identify adc voltg buffer on */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG, BIT(MBHD_COMP_PD_OFFSET), 0);
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG, BIT(MBHD_BUFF_PD_OFFSET), 0);
		usleep_range(100, 150);
	} else {
		/* disable NORMAL key detect and identify */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG, BIT(MBHD_BUFF_PD_OFFSET), 0xff);
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG, BIT(MBHD_COMP_PD_OFFSET), 0xff);

		/* hs micbias pd */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW7_REG, BIT(HSMICB_PD_OFFSET), 0xff);
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW7_REG, BIT(HSMICB_DSCHG_OFFSET), 0xff);
		msleep(5);
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW7_REG, BIT(HSMICB_DSCHG_OFFSET), 0);

		/* key detect mode switch to ECO mode */
		snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG, BIT(MBHD_ECO_EN_OFFSET), 0xff);
		msleep(20);

		/*close ibias*/
		ibias_hsmicbias_enable(priv->codec, false);

		hi6xxx_irqs_clr(priv, IRQ_MSK_COMP);
		hi6xxx_irqs_mask_clr(priv, IRQ_MSK_BTN_ECO);
	}
}

static void hi6xxx_hs_micbias_enable(struct hi6xxx_mbhc_priv *priv, bool enable)
{
	logi("begin,en=%d\n",enable);
	WARN_ON(!priv);

	/* hs_micbias power up,then power down 3 seconds later */
	cancel_delayed_work(&priv->hs_micbias_delay_pd_dw);
	flush_workqueue(priv->hs_micbias_delay_pd_dwq);

	if (enable) {
		/* read hs_micbias pd status,1:pd */
		if (((snd_soc_read(priv->codec, CODEC_ANA_RW7_REG))&(BIT(HSMICB_PD_OFFSET))))
			hi6xxx_hs_micbias_power(priv, true);
	} else {
		if ((0 == priv->hs_micbias_dapm) && !priv->hs_micbias_mbhc) {
			wake_lock_timeout(&priv->wake_lock, msecs_to_jiffies(MICBIAS_PD_WAKE_LOCK_MS));
			mod_delayed_work(priv->hs_micbias_delay_pd_dwq,
					&priv->hs_micbias_delay_pd_dw,
					msecs_to_jiffies(MICBIAS_PD_DELAY_MS));
		}
	}
}

void hi6xxx_hs_micbias_dapm_enable(struct hi6xxx_mbhc *mbhc, bool enable)
{
	struct hi6xxx_mbhc_priv *priv = (struct hi6xxx_mbhc_priv *)mbhc;

	IN_FUNCTION;

	if (!priv) {
		loge("priv is null\n");
		return;
	}

	mutex_lock(&priv->hs_micbias_mutex);
	if (enable) {
		if (0 == priv->hs_micbias_dapm)
			hi6xxx_hs_micbias_enable(priv, true);

		if (MAX_UINT32 == priv->hs_micbias_dapm) {
			loge("hs_micbias_dapm will overflow\n");
			mutex_unlock(&priv->hs_micbias_mutex);
			return;
		}
		++priv->hs_micbias_dapm;
	} else {
		if (0 == priv->hs_micbias_dapm) {
			loge("hs_micbias_dapm is 0, fail to disable micbias\n");
			mutex_unlock(&priv->hs_micbias_mutex);
			return;
		}

		--priv->hs_micbias_dapm;
		if (0 == priv->hs_micbias_dapm)
			hi6xxx_hs_micbias_enable(priv, false);
	}
	mutex_unlock(&priv->hs_micbias_mutex);

	OUT_FUNCTION;
}

static void hi6xxx_hs_micbias_mbhc_enable(struct hi6xxx_mbhc_priv *priv, bool enable)
{
	IN_FUNCTION;

	WARN_ON(!priv);

	mutex_lock(&priv->hs_micbias_mutex);
	if (enable) {
		if (!priv->hs_micbias_mbhc) {
			hi6xxx_hs_micbias_enable(priv, true);
			priv->hs_micbias_mbhc = true;
		}
	} else {
		if (priv->hs_micbias_mbhc) {
			priv->hs_micbias_mbhc = false;
			hi6xxx_hs_micbias_enable(priv, false);
		}
	}
	mutex_unlock(&priv->hs_micbias_mutex);

	OUT_FUNCTION;
}

static void hi6xxx_hs_micbias_delay_pd_work(struct work_struct *work)
{
	struct hi6xxx_mbhc_priv *priv = container_of(work, struct hi6xxx_mbhc_priv, hs_micbias_delay_pd_dw.work);

	IN_FUNCTION;

	WARN_ON(!priv);

	hi6xxx_hs_micbias_power(priv, false);

	OUT_FUNCTION;
}

static inline int _read_hkadc_value(struct hi6xxx_mbhc_priv *priv)
{
	int hkadc_value;

	WARN_ON(!priv);

	priv->adc_voltage = hisi_adc_get_value(HI6XXX_HKADC_CHN);
	if (priv->adc_voltage < 0) {
		return -EFAULT;
	}

	/* HKADC voltage, real value should devided 0.6 */
	hkadc_value = ((priv->adc_voltage)*(10))/(6);
	logi("adc_voltage = %d\n", priv->adc_voltage);

	return hkadc_value;
}

static irqreturn_t hi6xxx_irq_handler(int irq, void *data)
{
	struct hi6xxx_mbhc_priv *priv;
	unsigned int irqs = 0;
	unsigned int irq_mask = 0, irq_masked = 0;

	logd(">>>>>Begin\n");
	if (!data) {
		loge("data is null\n");
		return IRQ_NONE;
	}

	priv = (struct hi6xxx_mbhc_priv *)data;

	irqs = snd_soc_read(priv->codec, ANA_IRQ_REG0_REG);
	if (0 == irqs) {
		return IRQ_NONE;
	}

	irq_mask = snd_soc_read(priv->codec, ANA_IRQM_REG0_REG);
	irq_mask &= (~IRQ_PLUG_IN);
	irq_masked = irqs & (~irq_mask);

	if (0 == irq_masked) {
		hi6xxx_irqs_clr(priv, irqs);
		return IRQ_HANDLED;
	}

	wake_lock_timeout(&priv->wake_lock, msecs_to_jiffies(IRQ_HANDLE_WAKE_LOCK_MS));

	if (irq_masked & IRQ_PLUG_OUT) {
		queue_delayed_work(priv->hs_po_dwq,
				&priv->hs_po_dw,
				msecs_to_jiffies(0));
	}

	if (irq_masked & IRQ_PLUG_IN) {
		queue_delayed_work(priv->hs_pi_dwq,
				&priv->hs_pi_dw,
				msecs_to_jiffies(HS_TIME_PI_DETECT));
	}

	if (irq_masked & IRQ_COMP_L_BTN_DOWN) {
		queue_delayed_work(priv->hs_comp_l_btn_down_dwq,
				&priv->hs_comp_l_btn_down_dw,
				msecs_to_jiffies(HS_TIME_COMP_IRQ_TRIG));
	}

	if (irq_masked & IRQ_COMP_L_BTN_UP) {
		queue_delayed_work(priv->hs_comp_l_btn_up_dwq,
				&priv->hs_comp_l_btn_up_dw,
				msecs_to_jiffies(HS_TIME_COMP_IRQ_TRIG_2));
	}

	if (irq_masked & IRQ_ECO_BTN_DOWN) {
		queue_delayed_work(priv->hs_eco_btn_down_dwq,
				&priv->hs_eco_btn_down_dw,
				msecs_to_jiffies(HS_TIME_COMP_IRQ_TRIG));
	}

	if (irq_masked & IRQ_ECO_BTN_UP) {
		queue_delayed_work(priv->hs_eco_btn_up_dwq,
				&priv->hs_eco_btn_up_dw,
				msecs_to_jiffies(HS_TIME_COMP_IRQ_TRIG_2));
	}

	if (irq_masked & IRQ_COMP_H_BTN_DOWN) {
		queue_delayed_work(priv->hs_comp_h_btn_down_dwq,
				&priv->hs_comp_h_btn_down_dw,
				msecs_to_jiffies(HS_TIME_COMP_IRQ_TRIG));
	}

	if (irq_masked & IRQ_COMP_H_BTN_UP) {
		queue_delayed_work(priv->hs_comp_h_btn_up_dwq,
				&priv->hs_comp_h_btn_up_dw,
				msecs_to_jiffies(HS_TIME_COMP_IRQ_TRIG_2));
	}

	/* clear all read irq bits */
	hi6xxx_irqs_clr(priv, irqs);
	logd("<<<End, irq_masked=0x%x,irq_read=0x%x, irq_aftclr=0x%x, IRQM=0x%x, IRQ_RAW=0x%x\n",
			irq_masked, irqs,
			snd_soc_read(priv->codec, ANA_IRQ_REG0_REG),
			snd_soc_read(priv->codec, ANA_IRQM_REG0_REG),
			snd_soc_read(priv->codec, ANA_IRQ_SIG_STAT_REG));

	return IRQ_HANDLED;
}

static_t void hs_jack_report(struct hi6xxx_mbhc_priv *priv)
{
	int jack_report = 0;

	switch (priv->hs_status) {
	case HI6XXX_JACK_BIT_NONE:
		jack_report = 0;
		logi("plug out\n");
		break;
	case HI6XXX_JACK_BIT_HEADSET:
		jack_report = SND_JACK_HEADSET;
		logi("4-pole headset plug in\n");
		break;
	case HI6XXX_JACK_BIT_INVERT:
		jack_report = SND_JACK_HEADPHONE;
		logi("invert headset plug in\n");
		break;
	case HI6XXX_JACK_BIT_HEADPHONE:
		jack_report = SND_JACK_HEADPHONE;
		logi("3-pole headphone plug in\n");
		break;
	default:
		loge("error hs_status(%d)\n", priv->hs_status);
		return;
	}

	/* report jack status */
	snd_soc_jack_report(&priv->hs_jack.jack, jack_report, SND_JACK_HEADSET);
	switch_set_state(&priv->hs_jack.sdev, priv->hs_status);
}

static void _hs_type_recognize(struct hi6xxx_mbhc_priv *priv, int hkadc_value)
{
	/* 3-pole headphone */
	if (hkadc_value <= priv->mbhc_config.hs_3_pole_max_voltage) {
		priv->hs_status = HI6XXX_JACK_BIT_HEADPHONE;
		logi("headphone is 3 pole, saradc=%d\n", hkadc_value);
	} else if ((hkadc_value >= priv->mbhc_config.hs_4_pole_min_voltage)
			&& (hkadc_value <= priv->mbhc_config.hs_4_pole_max_voltage)) {
		/* 4-pole headset */
		priv->hs_status = HI6XXX_JACK_BIT_HEADSET;
		logi("headphone is 4 pole, saradc=%d\n", hkadc_value);
	} else if (hkadc_value > priv->mbhc_config.hs_4_pole_max_voltage) {
		/* lineout */
		priv->hs_status = HI6XXX_JACK_BIT_LINEOUT;
		logi("headphone is lineout, saradc=%d\n", hkadc_value);
	} else {
		/* invert 4-pole headset */
		priv->hs_status = HI6XXX_JACK_BIT_INVERT;
		logi("headphone is invert 4 pole, saradc=%d\n", hkadc_value);
	}
}

static void _hs_plug_out_detect(struct hi6xxx_mbhc_priv *priv)
{
	IN_FUNCTION;
	WARN_ON(!priv);

	mutex_lock(&priv->plug_mutex);
	/*
	 * Avoid hs_micbias_delay_pd_dw waiting for entering eco,
	 * so cancel the delay work then power off hs_micbias.
	 */
	cancel_delayed_work(&priv->hs_micbias_delay_pd_dw);
	flush_workqueue(priv->hs_micbias_delay_pd_dwq);
	priv->hs_micbias_mbhc = false;
	hi6xxx_hs_micbias_power(priv, false);

	/* mbhc vref pd */
	snd_soc_update_bits(priv->codec, HI6XXX_MBHD_VREF_CTRL, BIT(MBHD_VREF_PD_OFFSET), 0xff);

	/* disable ECO */
	snd_soc_update_bits(priv->codec, CODEC_ANA_RW8_REG, BIT(MBHD_ECO_EN_OFFSET), 0);

	/* mask all btn irq */
	hi6xxx_irqs_mask_set(priv, IRQ_MSK_COMP);
	mutex_lock(&priv->io_mutex);
	priv->hs_jack.report = 0;

	if (0 != priv->pressed_btn_type) {
		priv->hs_jack.jack.jack->type = priv->pressed_btn_type;
		/*report key event*/
		logi("report type=0x%x, status=0x%x\n", priv->hs_jack.report, priv->hs_status);
		snd_soc_jack_report(&priv->hs_jack.jack, priv->hs_jack.report, JACK_RPT_MSK_BTN);
	}

	priv->pressed_btn_type = 0;
	priv->hs_status = HI6XXX_JACK_BIT_NONE;
	priv->old_hs_status = HI6XXX_JACK_BIT_INVALID;
	mutex_unlock(&priv->io_mutex);

	priv->pre_status_is_lineout = false;

	/*report headset info */
	hs_jack_report(priv);
	headset_auto_calib_reset_interzone();
	hi6xxx_irqs_clr(priv, IRQ_MSK_COMP);
	hi6xxx_irqs_mask_clr(priv, IRQ_PLUG_IN);
	mutex_unlock(&priv->plug_mutex);

	OUT_FUNCTION;

	return;
}

static void _hs_plug_in_detect(struct hi6xxx_mbhc_priv *priv)
{
	int hkadc_value = 0;

	IN_FUNCTION;

	WARN_ON(!priv);

	/* check state - plugin */
	if (!hi6xxx_irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		logi("plug_in SIG STAT: not plug in, irq_state=0x%x, IRQM=0x%x, RAW_irq =0x%x\n",
				snd_soc_read(priv->codec, ANA_IRQ_REG0_REG),
				snd_soc_read(priv->codec, ANA_IRQM_REG0_REG),
				snd_soc_read(priv->codec, ANA_IRQ_SIG_STAT_REG));

		_hs_plug_out_detect(priv);
		return;
	}

	mutex_lock(&priv->plug_mutex);
	/* mask plug out */
	hi6xxx_irqs_mask_set(priv, IRQ_PLUG_OUT | IRQ_MSK_COMP);
	mutex_lock(&priv->hkadc_mutex);
	snd_soc_update_bits(priv->codec, HI6XXX_MBHD_VREF_CTRL, BIT(MBHD_VREF_PD_OFFSET), 0);
	hi6xxx_hs_micbias_mbhc_enable(priv, true);
	msleep(150);
	hkadc_value = _read_hkadc_value(priv);

	if (hkadc_value < 0) {
		loge("get adc fail,can't read adc value\n");
		mutex_unlock(&priv->hkadc_mutex);
		mutex_unlock(&priv->plug_mutex);
		return;
	}

	/* value greater than 2565 can not trigger eco btn,
	 * so,the hs_micbias can't be closed until second detect finish.
	 */
	if ((hkadc_value <= priv->mbhc_config.hs_4_pole_max_voltage) && (priv->pre_status_is_lineout == false)) {
		hi6xxx_hs_micbias_mbhc_enable(priv, false);
	}
	mutex_unlock(&priv->hkadc_mutex);

	mutex_lock(&priv->io_mutex);
	_hs_type_recognize(priv, hkadc_value);

	hi6xxx_irqs_clr(priv, IRQ_PLUG_OUT);
	hi6xxx_irqs_mask_clr(priv, IRQ_PLUG_OUT);
	mutex_unlock(&priv->io_mutex);

	if (!hi6xxx_irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		logi("plug out happens\n");
		mutex_unlock(&priv->plug_mutex);
		_hs_plug_out_detect(priv);
		return;
	}

	if ((HI6XXX_JACK_BIT_LINEOUT != priv->hs_status) && (HI6XXX_JACK_BIT_NONE != priv->hs_status) && (priv->hs_status != priv->old_hs_status)) {
		priv->old_hs_status = priv->hs_status;
		logi("hs status=%d, pre_status_is_lineout:%d\n", priv->hs_status, priv->pre_status_is_lineout);
		/*report headset info */
		hs_jack_report(priv);
	} else if (HI6XXX_JACK_BIT_LINEOUT == priv->hs_status) {
		priv->pre_status_is_lineout = true;
		logi("hs status=%d, old_hs_status=%d, lineout is plugin\n", priv->hs_status, priv->old_hs_status);
		/*not the first time recognize as lineout, headphone/set plug out from lineout */
		if(HI6XXX_JACK_BIT_INVALID != priv->old_hs_status) {
			priv->hs_status = HI6XXX_JACK_BIT_NONE;
			priv->old_hs_status = HI6XXX_JACK_BIT_INVALID;
			/*headphone/set plug out from lineout,need report plugout event*/
			hs_jack_report(priv);
			priv->hs_status = HI6XXX_JACK_BIT_LINEOUT;
		}
	} else {
		logi("hs status=%d(old=%d) not changed\n", priv->hs_status, priv->old_hs_status);
	}

	/* to avoid irq while MBHD_COMP power up, mask the irq then clean it */
	hi6xxx_irqs_clr(priv, IRQ_MSK_COMP);
	hi6xxx_irqs_mask_clr(priv, IRQ_MSK_BTN_NOR);
	mutex_unlock(&priv->plug_mutex);

	OUT_FUNCTION;

	return;
}

static void _hs_btn_down_detect(struct hi6xxx_mbhc_priv *priv)
{
	int pr_btn_type = 0;
	int hkadc_value = 0;

	IN_FUNCTION;

	WARN_ON(!priv);

	if (!hi6xxx_irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		logi("plug out happened\n");
		return;
	}

	if (HI6XXX_JACK_BIT_HEADSET != priv->hs_status) {
		/* enter the second detect,it's triggered by btn irq  */
		logi("enter btn_down 2nd time hp type recognize, saradc_value=%d, btn_type=0x%x\n", hkadc_value, priv->pressed_btn_type);
		_hs_plug_in_detect(priv);
		return;
	}

	if (HI6XXX_JACK_BIT_NONE != priv->pressed_btn_type) {
		loge("btn_type:0x%x has been pressed\n",priv->pressed_btn_type);
		return;
	}

	/* hs_micbias power up,then power down 3 seconds later */
	cancel_delayed_work(&priv->hs_micbias_delay_pd_dw);
	flush_workqueue(priv->hs_micbias_delay_pd_dwq);

	mutex_lock(&priv->hkadc_mutex);
	hi6xxx_hs_micbias_mbhc_enable(priv, true);
	hkadc_value = _read_hkadc_value(priv);
	if (hkadc_value < 0) {
		loge("get adc fail,can't read adc value, %d\n", hkadc_value);
		mutex_unlock(&priv->hkadc_mutex);
		return;
	}

	if( !priv->pre_status_is_lineout ) {
		hi6xxx_hs_micbias_mbhc_enable(priv, false);
	}
	mutex_unlock(&priv->hkadc_mutex);
	msleep(30);
	/* micbias power up have done, now is in normal mode, clean all COMP IRQ and cancel NOR int mask */
	hi6xxx_irqs_clr(priv, IRQ_MSK_COMP);
	hi6xxx_irqs_mask_clr(priv, IRQ_MSK_BTN_NOR);
	logi("mask clean\n");

	/*judge key type*/
	if ((hkadc_value >= priv->mbhc_config.btn_play_min_voltage) && (hkadc_value <= priv->mbhc_config.btn_play_max_voltage)) {
		pr_btn_type = SND_JACK_BTN_0;
		logi("key1 play down, saradc value is %d\n", hkadc_value);
	} else if ((hkadc_value >= priv->mbhc_config.btn_volume_up_min_voltage) && (hkadc_value <= priv->mbhc_config.btn_volume_up_max_voltage)) {
		pr_btn_type = SND_JACK_BTN_1;
		logi("key2 vol+ down, saradc value is %d\n", hkadc_value);
	} else if ((hkadc_value >= priv->mbhc_config.btn_volume_down_min_voltage) && (hkadc_value <= priv->mbhc_config.btn_volume_down_max_voltage)) {
		pr_btn_type = SND_JACK_BTN_2;
		logi("key3 vol- down , saradc value is %d\n", hkadc_value);
	} else if ((hkadc_value > priv->mbhc_config.btn_voice_assistant_min_voltage) && (hkadc_value < priv->mbhc_config.btn_voice_assistant_max_voltage)) {
		pr_btn_type = SND_JACK_BTN_3;
		logi("key voice_assistant , saradc value is %d\n", hkadc_value);
		goto VOCIE_ASSISTANT_KEY;
	} else {
		loge("hkadc value is not in range , hkadc value is %d\n", hkadc_value);
		//return;
	}
	startup_FSM(REC_JUDGE, hkadc_value, &pr_btn_type);
VOCIE_ASSISTANT_KEY:

	mutex_lock(&priv->io_mutex);
	priv->pressed_btn_type = pr_btn_type;
	priv->hs_jack.report = pr_btn_type;
	priv->hs_jack.jack.jack->type = pr_btn_type;
	mutex_unlock(&priv->io_mutex);

	if (!hi6xxx_irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		logi("plug out happened\n");
	} else {
		/*report key event*/
		logi("report type=0x%x, status=0x%x\n", priv->hs_jack.report, priv->hs_status);
		snd_soc_jack_report(&priv->hs_jack.jack, priv->hs_jack.report, JACK_RPT_MSK_BTN);
	}

	OUT_FUNCTION;

	return;
}

static void _hs_btn_up_detect(struct hi6xxx_mbhc_priv *priv)
{
	IN_FUNCTION;

	WARN_ON(!priv);

	if (!hi6xxx_irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		logi("plug out happened\n");
		return;
	}

	mutex_lock(&priv->io_mutex);
	if (0 == priv->pressed_btn_type) {
		mutex_unlock(&priv->io_mutex);

		/* second detect */
		if ((HI6XXX_JACK_BIT_INVERT == priv->hs_status) || (HI6XXX_JACK_BIT_HEADPHONE == priv->hs_status)
		|| (HI6XXX_JACK_BIT_LINEOUT == priv->hs_status)) {
			logi("enter btn_up 2nd time hp type recognize\n");
			_hs_plug_in_detect(priv);
		} else {
			logi("ignore the key up irq\n");
		}

		return;
	}

	priv->hs_jack.jack.jack->type = priv->pressed_btn_type;
	priv->hs_jack.report = 0;

	/*report key event*/
	logi("report type=0x%x, status=0x%x\n", priv->hs_jack.report, priv->hs_status);
	snd_soc_jack_report(&priv->hs_jack.jack, priv->hs_jack.report, JACK_RPT_MSK_BTN);
	priv->pressed_btn_type = 0;
	mutex_unlock(&priv->io_mutex);

	OUT_FUNCTION;

	return;
}

static void hi6xxx_lineout_rm_recheck_work(struct work_struct *work)
{
	struct hi6xxx_mbhc_priv *priv = container_of(work, struct hi6xxx_mbhc_priv, lineout_po_recheck_dw.work);
	int hkadc_value;

	WARN_ON(!priv);

	IN_FUNCTION;

	mutex_lock(&priv->plug_mutex);
	if(!hi6xxx_irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		hi6xxx_irqs_clr(priv, IRQ_MSK_COMP);
		hi6xxx_irqs_mask_clr(priv, IRQ_PLUG_IN);
		mutex_unlock(&priv->plug_mutex);
		logi("plugout has happened,ignore this irq\n");
		return;
	}

	mutex_lock(&priv->hkadc_mutex);
	hkadc_value = _read_hkadc_value(priv);
	mutex_unlock(&priv->hkadc_mutex);
	if (hkadc_value < 0) {
		loge("get adc fail,can't read adc value\n");
		mutex_unlock(&priv->plug_mutex);
		return;
	}

	if (hkadc_value <= priv->mbhc_config.hs_4_pole_max_voltage) {
		/*btn_up event*/
		mutex_unlock(&priv->plug_mutex);
		return;
	}

	/*report plugout event,and set hs_status to lineout mode*/
	if (priv->hs_status == HI6XXX_JACK_BIT_LINEOUT) {
		mutex_unlock(&priv->plug_mutex);
		logi("lineout recheck, hs_status is lineout, just return\n");
		return;
	}

	priv->hs_status = HI6XXX_JACK_BIT_NONE;
	priv->old_hs_status = HI6XXX_JACK_BIT_INVALID;
	/*report plugout event*/
	hs_jack_report(priv);
	priv->hs_status = HI6XXX_JACK_BIT_LINEOUT;
	priv->pre_status_is_lineout = true;

	hi6xxx_irqs_clr(priv, IRQ_MSK_COMP);
	hi6xxx_irqs_mask_clr(priv, IRQ_MSK_BTN_NOR);
	logi("lineout recheck,and report remove\n");

	mutex_unlock(&priv->plug_mutex);
	return;
}

static void hs_lineout_rm_recheck(struct hi6xxx_mbhc_priv *priv)
{
	WARN_ON(!priv);

	cancel_delayed_work(&priv->lineout_po_recheck_dw);
	flush_workqueue(priv->lineout_po_recheck_dwq);

	wake_lock_timeout(&priv->wake_lock, msecs_to_jiffies(LINEOUT_PO_RECHK_WAKE_LOCK_MS));
	mod_delayed_work(priv->lineout_po_recheck_dwq,
			&priv->lineout_po_recheck_dw,
			msecs_to_jiffies(LINEOUT_PO_RECHK_DELAY_MS));
}

static bool hs_lineout_plug_out(struct hi6xxx_mbhc_priv *priv)
{
	int hkadc_value;

	mutex_lock(&priv->plug_mutex);
	if(!hi6xxx_irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		hi6xxx_irqs_clr(priv, IRQ_MSK_COMP);
		hi6xxx_irqs_mask_clr(priv, IRQ_PLUG_IN);
		mutex_unlock(&priv->plug_mutex);
		logi("plugout has happened,ignore this irq\n");
		return true;
	}

	mutex_lock(&priv->hkadc_mutex);
	hkadc_value = _read_hkadc_value(priv);
	mutex_unlock(&priv->hkadc_mutex);
	if (hkadc_value < 0) {
		loge("get adc fail,can't read adc value, %d\n", hkadc_value);
		mutex_unlock(&priv->plug_mutex);
		return false;
	}

	if (hkadc_value <= priv->mbhc_config.hs_4_pole_max_voltage) {
		/*btn_up event*/
		mutex_unlock(&priv->plug_mutex);
		hs_lineout_rm_recheck(priv);
		return false;
	} else {
		/*report plugout event,and set hs_status to lineout mode*/
		mutex_lock(&priv->io_mutex);
		priv->hs_jack.report = 0;
		if (0 != priv->pressed_btn_type) {
			priv->hs_jack.jack.jack->type = priv->pressed_btn_type;
			/*report key event*/
			logi("report type=0x%x, status=0x%x\n", priv->hs_jack.report, priv->hs_status);
			snd_soc_jack_report(&priv->hs_jack.jack, priv->hs_jack.report, JACK_RPT_MSK_BTN);
		}
		priv->pressed_btn_type = 0;
		mutex_unlock(&priv->io_mutex);

		priv->hs_status = HI6XXX_JACK_BIT_NONE;
		priv->old_hs_status = HI6XXX_JACK_BIT_INVALID;
		/*report plugout event*/
		hs_jack_report(priv);
		priv->hs_status = HI6XXX_JACK_BIT_LINEOUT;
		priv->pre_status_is_lineout = true;

		hi6xxx_irqs_clr(priv, IRQ_MSK_COMP);
		hi6xxx_irqs_mask_clr(priv, IRQ_MSK_BTN_NOR);
		mutex_unlock(&priv->plug_mutex);
		hs_lineout_rm_recheck(priv);
	}

	return true;
}

static void hi6xxx_hs_pi_work(struct work_struct *work)
{
	struct hi6xxx_mbhc_priv *priv = container_of(work, struct hi6xxx_mbhc_priv, hs_pi_dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	_hs_plug_in_detect(priv);
}

static void hi6xxx_hs_po_work(struct work_struct *work)
{
	struct hi6xxx_mbhc_priv *priv = container_of(work, struct hi6xxx_mbhc_priv, hs_po_dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	_hs_plug_out_detect(priv);
}

static void hi6xxx_hs_comp_l_btn_down_work(struct work_struct *work)
{
	struct hi6xxx_mbhc_priv *priv = container_of(work, struct hi6xxx_mbhc_priv, hs_comp_l_btn_down_dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	_hs_btn_down_detect(priv);
}

static void hi6xxx_hs_comp_l_btn_up_work(struct work_struct *work)
{
	struct hi6xxx_mbhc_priv *priv = container_of(work, struct hi6xxx_mbhc_priv, hs_comp_l_btn_up_dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	_hs_btn_up_detect(priv);
}

static void hi6xxx_hs_comp_h_btn_down_work(struct work_struct *work)
{
	struct hi6xxx_mbhc_priv *priv = container_of(work, struct hi6xxx_mbhc_priv, hs_comp_h_btn_down_dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	if(hs_lineout_plug_out(priv)) {
		logi("hs plugout from lineout, return\n");
		return;
	}

	_hs_btn_down_detect(priv);
}

static void hi6xxx_hs_comp_h_btn_up_work(struct work_struct *work)
{
	struct hi6xxx_mbhc_priv *priv = container_of(work, struct hi6xxx_mbhc_priv, hs_comp_h_btn_up_dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	_hs_btn_up_detect(priv);
}

static void hi6xxx_hs_eco_btn_down_work(struct work_struct *work)
{
	struct hi6xxx_mbhc_priv *priv = container_of(work, struct hi6xxx_mbhc_priv, hs_eco_btn_down_dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	_hs_btn_down_detect(priv);
}

static void hi6xxx_hs_eco_btn_up_work(struct work_struct *work)
{
	struct hi6xxx_mbhc_priv *priv = container_of(work, struct hi6xxx_mbhc_priv, hs_eco_btn_up_dw.work);

	logi("enter\n");
	WARN_ON(!priv);

	_hs_btn_up_detect(priv);
}

static void hi6xxx_mbhc_reg_init(struct snd_soc_codec *codec)
{
	/* HSD mode sel to schmitt and enable */
	snd_soc_write(codec, HI6XXX_HS_PI_DETECT, 0x1);

	/* eliminate btn dithering */
	snd_soc_write(codec, DEB_CNT_HS_MIC_CFG_REG, 0x14);

	/* MBHC compare config 125mV 800mV 95% */
	snd_soc_write(codec, HI6XXX_MBHD_VREF_CTRL, 0x9E);

	/*HSMICBIAS config voltage 2.7V */
	snd_soc_write(codec, HI6XXX_HSMICB_CFG, 0x0B);

	/* clear HP MIXER channel select */
	snd_soc_write(codec, CODEC_ANA_RW20_REG, 0x0);

	/* config HP PGA gain to -20dB */
	snd_soc_write(codec, CODEC_ANA_RW21_REG, 0x0);

	/* Charge Pump clk pd, freq 768kHz */
	snd_soc_write(codec, HI6XXX_CHARGE_PUMP_CLK_PD, 0x1A);

	/* disable ECO*/
	snd_soc_update_bits(codec, CODEC_ANA_RW8_REG, BIT(MBHD_ECO_EN_OFFSET), 0);
}

static void hi6xxx_mbhc_3_pole_voltage_config(struct device_node *np,
										struct hi6xxx_mbhc_config *mbhc_config)
{
	unsigned int temp = 0;

	if (!of_property_read_u32(np, "hisilicon,hs_3_pole_max_voltage", &temp))
		mbhc_config->hs_3_pole_max_voltage = temp;
	else
		mbhc_config->hs_3_pole_max_voltage = 0;
}

static void hi6xxx_mbhc_4_pole_voltage_config(struct device_node *np,
										struct hi6xxx_mbhc_config *mbhc_config)
{
	unsigned int temp = 0;

	if (!of_property_read_u32(np, "hisilicon,hs_4_pole_min_voltage", &temp))
		mbhc_config->hs_4_pole_min_voltage = temp;
	else
		mbhc_config->hs_4_pole_min_voltage = 0;

	if (!of_property_read_u32(np, "hisilicon,hs_4_pole_max_voltage", &temp))
		mbhc_config->hs_4_pole_max_voltage = temp;
	else
		mbhc_config->hs_4_pole_max_voltage = 0;
}

static void hi6xxx_mbhc_btn_voltage_config(struct device_node *np,
										struct hi6xxx_mbhc_config *mbhc_config)
{
	unsigned int temp = 0;

	mbhc_config->btn_play_min_voltage = 0;
	mbhc_config->btn_play_max_voltage = 0;
	mbhc_config->btn_volume_up_min_voltage = 0;
	mbhc_config->btn_volume_up_max_voltage = 0;
	mbhc_config->btn_volume_down_min_voltage = 0;
	mbhc_config->btn_volume_down_max_voltage = 0;
	mbhc_config->btn_voice_assistant_min_voltage = -1;
	mbhc_config->btn_voice_assistant_max_voltage = -1;

	if (!of_property_read_u32(np, "hisilicon,btn_play_min_voltage", &temp))
		mbhc_config->btn_play_min_voltage = temp;

	if (!of_property_read_u32(np, "hisilicon,btn_play_max_voltage", &temp))
		mbhc_config->btn_play_max_voltage = temp;

	if (!of_property_read_u32(np, "hisilicon,btn_volume_up_min_voltage", &temp))
		mbhc_config->btn_volume_up_min_voltage = temp;

	if (!of_property_read_u32(np, "hisilicon,btn_volume_up_max_voltage", &temp))
		mbhc_config->btn_volume_up_max_voltage = temp;

	if (!of_property_read_u32(np, "hisilicon,btn_volume_down_min_voltage", &temp))
		mbhc_config->btn_volume_down_min_voltage = temp;

	if (!of_property_read_u32(np, "hisilicon,btn_volume_down_max_voltage", &temp))
		mbhc_config->btn_volume_down_max_voltage = temp;

	if (!of_property_read_u32(np, "hisilicon,btn_voice_assistant_min_voltage", &temp))
		mbhc_config->btn_voice_assistant_min_voltage = temp;

	if (!of_property_read_u32(np, "hisilicon,btn_voice_assistant_max_voltage", &temp))
		mbhc_config->btn_voice_assistant_max_voltage = temp;
}

static void hi6xxx_mbhc_config_set(struct device_node *np, struct hi6xxx_mbhc_config *mbhc_config)
{
	hi6xxx_mbhc_3_pole_voltage_config(np, mbhc_config);
	hi6xxx_mbhc_4_pole_voltage_config(np, mbhc_config);
	logi("headset_voltage threshold: 3pole=%d, 4pole=%d-%d\n",
			mbhc_config->hs_3_pole_max_voltage,
			mbhc_config->hs_4_pole_min_voltage,
			mbhc_config->hs_4_pole_max_voltage);

	hi6xxx_mbhc_btn_voltage_config(np, mbhc_config);
	logi("btn_voltage threshold: btn_play=%d-%d, btn_volume_down=%d-%d, btn_volume_up=%d-%d, btn_voice_assistant=%d-%d\n",
			mbhc_config->btn_play_min_voltage,
			mbhc_config->btn_play_max_voltage,
			mbhc_config->btn_volume_down_min_voltage,
			mbhc_config->btn_volume_down_max_voltage,
			mbhc_config->btn_volume_up_min_voltage,
			mbhc_config->btn_volume_up_max_voltage,
			mbhc_config->btn_voice_assistant_min_voltage,
			mbhc_config->btn_voice_assistant_max_voltage);
}

static int hi6xxx_irq_init(struct hi6xxx_mbhc_priv *priv)
{
	priv->hs_pi_dwq = create_singlethread_workqueue("hs_pi_dwq");
	if (NULL == priv->hs_pi_dwq) {
		loge("hs_pi_dwq workqueue create failed\n");
		return -EFAULT;
	}
	INIT_DELAYED_WORK(&priv->hs_pi_dw, hi6xxx_hs_pi_work);

	priv->hs_po_dwq = create_singlethread_workqueue("hs_po_dwq");
	if (NULL == priv->hs_po_dwq) {
		loge("hs_po_dwq workqueue create failed\n");
		return -EFAULT;
	}
	INIT_DELAYED_WORK(&priv->hs_po_dw, hi6xxx_hs_po_work);

	priv->hs_comp_l_btn_down_dwq = create_singlethread_workqueue("hs_comp_l_btn_down_dwq");
	if (NULL == priv->hs_comp_l_btn_down_dwq) {
		loge("hs_btn_dwq workqueue create failed\n");
		return -EFAULT;
	}
	INIT_DELAYED_WORK(&priv->hs_comp_l_btn_down_dw, hi6xxx_hs_comp_l_btn_down_work);

	priv->hs_comp_l_btn_up_dwq = create_singlethread_workqueue("hs_comp_l_btn_up_dwq");
	if (NULL == priv->hs_comp_l_btn_up_dwq) {
		loge("hs_btn_dwq workqueue create failed\n");
		return -EFAULT;
	}
	INIT_DELAYED_WORK(&priv->hs_comp_l_btn_up_dw, hi6xxx_hs_comp_l_btn_up_work);

	priv->hs_comp_h_btn_down_dwq = create_singlethread_workqueue("hs_comp_h_btn_down_dwq");
	if (NULL == priv->hs_comp_h_btn_down_dwq) {
		loge("hs_btn_dwq workqueue create failed\n");
		return -EFAULT;
	}
	INIT_DELAYED_WORK(&priv->hs_comp_h_btn_down_dw, hi6xxx_hs_comp_h_btn_down_work);

	priv->hs_comp_h_btn_up_dwq = create_singlethread_workqueue("hs_comp_h_btn_up_dwq");
	if (NULL == priv->hs_comp_h_btn_up_dwq) {
		loge("hs_btn_dwq workqueue create failed\n");
		return -EFAULT;
	}
	INIT_DELAYED_WORK(&priv->hs_comp_h_btn_up_dw, hi6xxx_hs_comp_h_btn_up_work);

	priv->hs_eco_btn_down_dwq = create_singlethread_workqueue("hs_eco_btn_down_dwq");
	if (NULL == priv->hs_eco_btn_down_dwq) {
		loge("hs_btn_dwq workqueue create failed\n");
		return -EFAULT;
	}
	INIT_DELAYED_WORK(&priv->hs_eco_btn_down_dw, hi6xxx_hs_eco_btn_down_work);

	priv->hs_eco_btn_up_dwq = create_singlethread_workqueue("hs_eco_btn_up_dwq");
	if (NULL == priv->hs_eco_btn_up_dwq) {
		loge("hs_btn_dwq workqueue create failed\n");
		return -EFAULT;
	}
	INIT_DELAYED_WORK(&priv->hs_eco_btn_up_dw, hi6xxx_hs_eco_btn_up_work);

	return 0;
}

static void hi6xxx_irq_deinit(struct hi6xxx_mbhc_priv *priv)
{
	if (NULL != priv->hs_pi_dwq) {
		cancel_delayed_work(&priv->hs_pi_dw);
		flush_workqueue(priv->hs_pi_dwq);
		destroy_workqueue(priv->hs_pi_dwq);
		priv->hs_pi_dwq = NULL;
	}

	if (NULL != priv->hs_po_dwq) {
		cancel_delayed_work(&priv->hs_po_dw);
		flush_workqueue(priv->hs_po_dwq);
		destroy_workqueue(priv->hs_po_dwq);
		priv->hs_po_dwq = NULL;
	}

	if (NULL != priv->hs_comp_l_btn_down_dwq) {
		cancel_delayed_work(&priv->hs_comp_l_btn_down_dw);
		flush_workqueue(priv->hs_comp_l_btn_down_dwq);
		destroy_workqueue(priv->hs_comp_l_btn_down_dwq);
		priv->hs_comp_l_btn_down_dwq = NULL;
	}

	if (NULL != priv->hs_comp_l_btn_up_dwq) {
		cancel_delayed_work(&priv->hs_comp_l_btn_up_dw);
		flush_workqueue(priv->hs_comp_l_btn_up_dwq);
		destroy_workqueue(priv->hs_comp_l_btn_up_dwq);
		priv->hs_comp_l_btn_up_dwq = NULL;
	}

	if (NULL != priv->hs_comp_h_btn_down_dwq) {
		cancel_delayed_work(&priv->hs_comp_h_btn_down_dw);
		flush_workqueue(priv->hs_comp_h_btn_down_dwq);
		destroy_workqueue(priv->hs_comp_h_btn_down_dwq);
		priv->hs_comp_h_btn_down_dwq = NULL;
	}

	if (NULL != priv->hs_comp_h_btn_up_dwq) {
		cancel_delayed_work(&priv->hs_comp_h_btn_up_dw);
		flush_workqueue(priv->hs_comp_h_btn_up_dwq);
		destroy_workqueue(priv->hs_comp_h_btn_up_dwq);
		priv->hs_comp_h_btn_up_dwq = NULL;
	}

	if (NULL != priv->hs_eco_btn_down_dwq) {
		cancel_delayed_work(&priv->hs_eco_btn_down_dw);
		flush_workqueue(priv->hs_eco_btn_down_dwq);
		destroy_workqueue(priv->hs_eco_btn_down_dwq);
		priv->hs_eco_btn_down_dwq = NULL;
	}

	if (NULL != priv->hs_eco_btn_up_dwq) {
		cancel_delayed_work(&priv->hs_eco_btn_up_dw);
		flush_workqueue(priv->hs_eco_btn_up_dwq);
		destroy_workqueue(priv->hs_eco_btn_up_dwq);
		priv->hs_eco_btn_up_dwq = NULL;
	}

	return;
}

static int hi6xxx_hs_delay_workqueue_init(struct hi6xxx_mbhc_priv *priv)
{
	priv->hs_micbias_delay_pd_dwq = create_singlethread_workqueue("hs_micbias_delay_pd_dwq");
	if (NULL == priv->hs_micbias_delay_pd_dwq) {
		loge("hs_micbias_delay_pd_dwq workqueue create failed\n");
		return -EFAULT;
	}
	INIT_DELAYED_WORK(&priv->hs_micbias_delay_pd_dw, hi6xxx_hs_micbias_delay_pd_work);

	/*lineout remove out, recheck later*/
	priv->lineout_po_recheck_dwq = create_singlethread_workqueue("lineout_po_recheck_dwq");
	if (NULL == priv->lineout_po_recheck_dwq) {
		loge("lineout_po_recheck_dwq workqueue create failed\n");
		return -EFAULT;
	}
	INIT_DELAYED_WORK(&priv->lineout_po_recheck_dw, hi6xxx_lineout_rm_recheck_work);

	return 0;
}

static void hi6xxx_hs_delay_workqueue_deinit(struct hi6xxx_mbhc_priv *priv)
{
	if (NULL != priv->hs_micbias_delay_pd_dwq) {
		cancel_delayed_work(&priv->hs_micbias_delay_pd_dw);
		flush_workqueue(priv->hs_micbias_delay_pd_dwq);
		destroy_workqueue(priv->hs_micbias_delay_pd_dwq);
		priv->hs_micbias_delay_pd_dwq = NULL;
	}

	if (NULL != priv->lineout_po_recheck_dwq) {
		cancel_delayed_work(&priv->lineout_po_recheck_dw);
		flush_workqueue(priv->lineout_po_recheck_dwq);
		destroy_workqueue(priv->lineout_po_recheck_dwq);
		priv->lineout_po_recheck_dwq = NULL;
	}

	return;
}

static int hi6xxx_register_hs_jack_btn(struct hi6xxx_mbhc_priv *priv)
{
	int ret;

	/* Headset jack */
	ret = snd_soc_card_jack_new(priv->codec->component.card, "Headset Jack", SND_JACK_HEADSET, (&priv->hs_jack.jack), NULL, 0);
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

int hi6xxx_mbhc_init(struct snd_soc_codec *codec, struct hi6xxx_mbhc **mbhc)
{
	int ret;
	struct device *dev;
	struct device_node *np;
	struct hi6xxx_mbhc_priv *priv;

	logi("Begin\n");

	if (!codec) {
		loge("codec is null\n");
		return -EINVAL;
	}

	if (!mbhc) {
		loge("mbhc is invalid\n");
		return -EINVAL;
	}

	dev = codec->dev;
	np = dev->of_node;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		loge("priv devm_kzalloc failed\n");
		return -ENOMEM;
	}

	priv->codec = codec;

	mutex_init(&priv->io_mutex);
	mutex_init(&priv->hs_micbias_mutex);
	mutex_init(&priv->hkadc_mutex);
	mutex_init(&priv->plug_mutex);
	wake_lock_init(&priv->wake_lock, WAKE_LOCK_SUSPEND, "hi6xxx_mbhc");

	hi6xxx_mbhc_reg_init(codec);

	/* init hp_type&hp_key voltage value*/
	hi6xxx_mbhc_config_set(np, &priv->mbhc_config);

	priv->hs_status = HI6XXX_JACK_BIT_NONE;
	priv->old_hs_status = HI6XXX_JACK_BIT_INVALID;
	priv->hs_jack.report = 0;
	priv->pressed_btn_type = 0;
	priv->pre_status_is_lineout = false;
	priv->hs_micbias_mbhc = false;
	priv->hs_micbias_dapm = 0;

	/*HSMICBIAS PD  */
	snd_soc_update_bits(priv->codec, CODEC_ANA_RW7_REG, BIT(HSMICB_PD_OFFSET), 0xff);

	/* avoid irq triggered while codec power up */
	hi6xxx_irqs_mask_set(priv, IRQ_MSK_HS_ALL);
	hi6xxx_irqs_clr(priv, IRQ_MSK_HS_ALL);

	ret = hi6xxx_register_hs_jack_btn(priv);
	if (ret) {
		goto jack_err;
	}

	priv->hs_jack.sdev.name = "h2w";
	ret = switch_dev_register(&(priv->hs_jack.sdev));
	if (ret) {
		loge("Registering switch device error, ret=%d\n", ret);
		goto jack_err;
	}
	priv->hs_jack.is_dev_registered = true;

	ret = hi6xxx_irq_init(priv);
	if (ret) {
		loge("hi6xxx irq init error\n");
		goto hi6xxx_irq_init_err;
	}

	ret = hi6xxx_hs_delay_workqueue_init(priv);
	if (ret) {
		loge("hi6xxx delay workqueue config error\n");
		goto delay_wq_init_err;
	}

	/* config intr for codec */
	priv->gpio_intr_pin = of_get_named_gpio(np, "gpios", 0);
	if (!gpio_is_valid(priv->gpio_intr_pin)) {
		loge("gpio_intr_pin gpio:%d is invalied\n", priv->gpio_intr_pin);
		goto gpio_pin_err;
	}

	/* this gpio is shared by pmu and acore, and by quested in pmu, so need not requet here */
	priv->gpio_irq = gpio_to_irq(priv->gpio_intr_pin);
	if (priv->gpio_irq < 0) {
		loge("gpio_to_irq err, gpio_irq=%d, gpio_intr_pin=%d\n", priv->gpio_irq, priv->gpio_intr_pin);
		goto gpio_to_irq_err;
	}
	logi("gpio_to_irq succ, gpio_irq=%d, gpio_intr_pin=%d\n", priv->gpio_irq, priv->gpio_intr_pin);

	/* irq shared with pmu */
	ret = request_irq(priv->gpio_irq, hi6xxx_irq_handler, IRQF_TRIGGER_LOW | IRQF_SHARED | IRQF_NO_SUSPEND, "codec_irq", priv);
	if (ret) {
		loge("request_irq failed, ret = %d\n", ret);
		goto request_irq_err;
	}

	/* judge headset present or not */
	logi("irq soure stat %#04x", snd_soc_read(codec, ANA_IRQ_SIG_STAT_REG));
	if (hi6xxx_irq_status_check(priv, IRQ_STAT_PLUG_IN)) {
		_hs_plug_in_detect(priv);
	} else {
		hi6xxx_irqs_mask_clr(priv, IRQ_PLUG_IN);
	}
	headset_auto_calib_init(np);


	*mbhc = &priv->mbhc_pub;

	goto end;

request_irq_err:
gpio_to_irq_err:
	priv->gpio_irq = HI6XXX_INVALID_IRQ;
gpio_pin_err:
	priv->gpio_intr_pin = ARCH_NR_GPIOS;
delay_wq_init_err:
	hi6xxx_hs_delay_workqueue_deinit(priv);

hi6xxx_irq_init_err :
	hi6xxx_irq_deinit(priv);

	switch_dev_unregister(&(priv->hs_jack.sdev));
	priv->hs_jack.is_dev_registered = false;

jack_err:
	mutex_destroy(&priv->io_mutex);
	mutex_destroy(&priv->hs_micbias_mutex);
	mutex_destroy(&priv->hkadc_mutex);
	mutex_destroy(&priv->plug_mutex);
	wake_lock_destroy(&priv->wake_lock);
end:
	logi("End\n");
	return ret;
}

void hi6xxx_mbhc_deinit(struct hi6xxx_mbhc *mbhc)
{
	struct hi6xxx_mbhc_priv* priv = (struct hi6xxx_mbhc_priv*)mbhc;

	IN_FUNCTION;

	if (!priv) {
		loge("priv is NULL\n");
		return;
	}

	mutex_destroy(&priv->io_mutex);
	mutex_destroy(&priv->hs_micbias_mutex);
	mutex_destroy(&priv->hkadc_mutex);
	mutex_destroy(&priv->plug_mutex);
	wake_lock_destroy(&priv->wake_lock);


	if (priv->hs_jack.is_dev_registered) {
		switch_dev_unregister(&(priv->hs_jack.sdev));
		priv->hs_jack.is_dev_registered = false;
	}

	hi6xxx_irq_deinit(priv);
	hi6xxx_hs_delay_workqueue_deinit(priv);

	if (priv->gpio_irq >= 0) {
		free_irq(priv->gpio_irq, priv);
		priv->gpio_irq = HI6XXX_INVALID_IRQ;
	}


	OUT_FUNCTION;
	return;
}

MODULE_DESCRIPTION("hi6xxx_mbhc");
MODULE_AUTHOR("wangqi <wangqi55@hisilicon.com>");
MODULE_LICENSE("GPL");

