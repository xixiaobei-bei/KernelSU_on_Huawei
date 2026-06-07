/*
 * anc_hs.c
 *
 * analog headset driver
 *
 * Copyright (c) 2014-2019 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/pm_wakeup.h>
#include <linux/miscdevice.h>
#include <linux/regulator/consumer.h>
#include <linux/workqueue.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/uaccess.h>
#include <sound/jack.h>
#include <linux/fs.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif
#include "huawei_platform/audio/anc_hs.h"

#define UNUSED(x) ((void)x)
#define HWLOG_TAG anc_hs
HWLOG_REGIST();

#ifdef CONFIG_HIFI_DSP_ONE_TRACK
__attribute__((weak)) int hifi_send_msg(unsigned int mailcode, const void *data,
	unsigned int length)
{
	UNUSED(mailcode);
	UNUSED(data);
	UNUSED(length);
	hwlog_debug("%s use weak\n", __func__);
	return 0;
}
#endif

enum anc_hs_mode {
	ANC_HS_CHARGE_OFF         = 0,
	ANC_HS_CHARGE_ON          = 1,
};

/* anc headset charge status */
enum switch_chip_status {
	SWITCH_CHIP_HSBIAS        = 0,  /* not charging */
	SWITCH_CHIP_5VBOOST       = 1,  /* charging */
};

enum anc_hs_gpio_type {
	ANC_HS_GPIO_SOC           = 0,
	ANC_HS_GPIO_CODEC         = 1,
};

enum headset_status {
	ANC_HS_HEADSET_PLUG_OUT   = 0,
	ANC_HS_HEADSET_PLUG_IN    = 1,
};

#define  ANC_HS_LIMIT_MIN                    20
#define  ANC_HS_LIMIT_MAX                    200
#define  ANC_CHIP_STARTUP_TIME               30
#define  ADC_CALIBRATION_TIMES               10
#define  ADC_READ_COUNT                      3
#define  ADC_NORMAL_LIMIT_MIN                (-500)
#define  ADC_NORMAL_LIMIT_MAX                500
#define  ADC_OUT_OF_RANGE                    2499
#define  ANC_HS_HOOK_MIN                     160
#define  ANC_HS_HOOK_MAX                     185
#define  ANC_HS_VOLUME_UP_MIN                205
#define  ANC_HS_VOLUME_UP_MAX                230
#define  ANC_HS_VOLUME_DOWN_MIN              240
#define  ANC_HS_VOLUME_DOWN_MAX              265
#define  UDELAY_TIME                         500
#define  MAILBOX_MAILCODE_ACPU_TO_HIFI_MISC  262148
#define  NO_BUTTON_PRESS                     (-1)
#define  BUFFER_SIZE                         1024
#define  SMALL_BUFFER_SIZE                   10
#define  BIG_BUFFER_SIZE                     32
#define  ADC_VAL_MIN                         (-100)
#define  ADC_VAL_MAX                         100
#define  ADC_CALIBRAT_MIN                    (-50)
#define  ADC_CALIBRAT_MAX                    50
#define  ANC_ADC_CHANNEL_H_DEFAULT           15
#define  ANC_ADC_CHANNEL_L_DEFAULT           14
#define  ANC_BTN_MASK (SND_JACK_BTN_0 | SND_JACK_BTN_1 | SND_JACK_BTN_2)

#ifdef CONFIG_HUAWEI_DSM
/* dmd error report definition */
static struct dsm_dev dsm_anc_hs = {
	.name = "dsm_anc_hs",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = BUFFER_SIZE,
};
static struct dsm_client *anc_hs_dclient;
#endif

struct anc_hs_data {
	int anc_hs_mode;         /* charge status */
	int gpio_mic_sw;         /* switch chip control gpio */
	int mic_irq_gpio;
	int mic_irq;             /* charging btn irq num */
	int anc_pwr_en_gpio;     /* VBST_5V EN */
	int channel_pwl_h;       /* adc channel for high voltage */
	int channel_pwl_l;       /* adc channel for low voltage */
	int anc_hs_limit_min;
	int anc_hs_limit_max;
	int anc_hs_btn_hook_min_voltage;
	int anc_hs_btn_hook_max_voltage;
	int anc_hs_btn_volume_up_min_voltage;
	int anc_hs_btn_volume_up_max_voltage;
	int anc_hs_btn_volume_down_min_voltage;
	int anc_hs_btn_volume_down_max_voltage;
	bool irq_flag;
	bool boost_flag;
	int sleep_time;           /* charge chip pre-charge time */
	bool mic_used;            /* flag to show mic status */
	bool detect_again;
	int force_charge_ctl;     /* force charge control for userspace */
	int hs_micbias_ctl;       /* hs micbias control */
	int adc_calibration_base; /* calibration value */
	int button_pressed;
	int hs_status;
	struct mutex btn_mutex;
	struct mutex charge_lock; /* charge status protect lock */
	struct wakeup_source wake_lock;
	int registered;           /* anc hs regester flag */
	struct anc_hs_dev *dev;   /* anc hs dev */
	void *private_data;       /* store codec description data */
	int check_reg_value;      /* check register value before control gpio */
	int gpio_type;
	/* charging button irq workqueue */
	struct workqueue_struct *anc_hs_btn_delay_wq;
	struct delayed_work anc_hs_btn_delay_work;
};

#ifdef CONFIG_LLT_TEST
struct anc_hs_static_ops {
	int  (*anc_hs_gpio_get_value)(int gpio);
	void (*anc_hs_gpio_set_value)(int gpio, int value);
	void (*anc_hs_dump)(void);
	void (*anc_dsm_report)(int anc_errno, int sys_errno);
	void (*anc_hs_enable_irq)(void);
	void (*anc_hs_disable_irq)(void);
	int  (*enable_boost)(bool enable);
	int  (*anc_hs_get_adc_delta)(void);
	bool (*anc_hs_need_charge)(void);
	int  (*anc_hs_get_btn_value)(void);
	void (*anc_hs_adc_calibration)(void);
	bool (*check_anc_hs_support)(void);
	long (*anc_hs_ioctl)(struct file *file, unsigned int cmd,
		unsigned long arg);
	ssize_t (*anc_detect_limit_store)(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*anc_gpio_sw_store)(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*anc_precharge_time_store)(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
	void (*load_anc_hs_config)(struct device_node *node);
	int  (*anc_hs_probe)(struct platform_device *pdev);
	int  (*anc_hs_remove)(struct platform_device *pdev);
	struct anc_hs_data **pdata;
};
#endif

static struct anc_hs_data *anc_pdata;

static int anc_hs_gpio_get_value(int gpio)
{
	int ret;
	struct anc_hs_dev *pdev = anc_pdata->dev;
	struct anc_hs_codec_ops *fops = &pdev->ops;

	if (anc_pdata->gpio_type == ANC_HS_GPIO_CODEC) {
		if (anc_pdata->check_reg_value)
			fops->check_bus_status(anc_pdata->private_data);

		ret = gpio_get_value_cansleep(gpio);
		if (anc_pdata->check_reg_value)
			fops->check_bus_status(anc_pdata->private_data);

		return ret;
	}

	return gpio_get_value(gpio);
}

static void anc_hs_gpio_set_value(int gpio, int value)
{
	struct anc_hs_dev *pdev = anc_pdata->dev;
	struct anc_hs_codec_ops *fops = &pdev->ops;

	if (anc_pdata->gpio_type == ANC_HS_GPIO_CODEC) {
		if (anc_pdata->check_reg_value)
			fops->check_bus_status(anc_pdata->private_data);

		gpio_set_value_cansleep(gpio, value);
		if (anc_pdata->check_reg_value)
			fops->check_bus_status(anc_pdata->private_data);
	} else {
		gpio_set_value(gpio, value);
	}
}

static void anc_hs_dump(void)
{
	hwlog_info("%s: mode:%d,irq:%d,sleep_time:%d,mic_used:%d,switch:%d\n",
		__func__, anc_pdata->anc_hs_mode, anc_pdata->irq_flag,
		anc_pdata->sleep_time, anc_pdata->mic_used,
		anc_hs_gpio_get_value(anc_pdata->gpio_mic_sw));
}

static void anc_dsm_report(int anc_errno, int sys_errno)
{
#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_ANC_HS, anc_errno,
		"mode:%d,irq:%d,sleep_time:%d,mic_used:%d,switch:%d,errno:%d\n",
		anc_pdata->anc_hs_mode, anc_pdata->irq_flag,
		anc_pdata->sleep_time, anc_pdata->mic_used,
		anc_hs_gpio_get_value(anc_pdata->gpio_mic_sw), sys_errno);
#endif
}

static inline void anc_hs_enable_irq(void)
{
	if (!anc_pdata->irq_flag) {
		enable_irq(anc_pdata->mic_irq);
		anc_pdata->irq_flag = true;
	}
}

static inline void anc_hs_disable_irq(void)
{
	if (anc_pdata->irq_flag) {
		disable_irq_nosync(anc_pdata->mic_irq);
		anc_pdata->irq_flag = false;
	}
}

__attribute__((weak)) void boost5v_denoise_headphone_enable(bool enable)
{
	UNUSED(enable);
	hwlog_debug("%s use weak\n", __func__);
}

static int enable_boost(bool enable)
{
	/* use boost_flag to void un-balance call */
	if (enable) {
		if (!anc_pdata->boost_flag) {
#ifdef CONFIG_HISI_BOOST5V_CONTROL
			boost5v_denoise_headphone_enable(true);
#endif
			/* 5vboost stable time which is dependent on hardware */
			msleep(70);
			anc_pdata->boost_flag = true;
		}
	} else {
		if (anc_pdata->boost_flag) {
#ifdef CONFIG_HISI_BOOST5V_CONTROL
			boost5v_denoise_headphone_enable(false);
#endif
			anc_pdata->boost_flag = false;
		}
	}
	return 0;
}

static int anc_hs_get_adc_delta(void)
{
	int ear_pwr_h = 0;
	int ear_pwr_l = 0;
	int delta = 0;
	int count;
	int fail_count = 0;
	int loop = ADC_READ_COUNT;
	int temp;
	bool need_report = true;

	while (loop) {
		loop--;
		mdelay(1);
		ear_pwr_h = hisi_adc_get_value(anc_pdata->channel_pwl_h);
		if (ear_pwr_h < 0) {
			hwlog_err("%s:get hkadc(h) fail, err:%d\n", __func__,
				ear_pwr_h);
			fail_count++;
			continue;
		}
		ear_pwr_l = hisi_adc_get_value(anc_pdata->channel_pwl_l);
		if (ear_pwr_l < 0) {
			hwlog_err("%s:get hkadc(l) fail, err:%d\n", __func__,
				ear_pwr_l);
			fail_count++;
			continue;
		}
		hwlog_info("%s:adc_h:%d,adc_l:%d\n", __func__,
			ear_pwr_h, ear_pwr_l);

		temp = ear_pwr_h - ear_pwr_l - anc_pdata->adc_calibration_base;

		/* if the adc value far away from normal value, abandon it */
		if ((temp > ADC_NORMAL_LIMIT_MAX) ||
			(temp < ADC_NORMAL_LIMIT_MIN)) {
			fail_count++;
			need_report = false;
			continue;
		}

		delta += temp;
	}
#ifdef CONFIG_HUAWEI_DSM
	/* if adc value is out of rage, we make a dmd report */
	if (ear_pwr_h >= ADC_OUT_OF_RANGE || ear_pwr_l >= ADC_OUT_OF_RANGE)
		anc_dsm_report(ANC_HS_ADC_FULL_ERR, 0);
#endif
	count = ADC_READ_COUNT - loop - fail_count;
	if (count == 0) {
		hwlog_err("%s:get anc_hs hkadc failed\n", __func__);
#ifdef CONFIG_HUAWEI_DSM
		if (need_report)
			anc_dsm_report(ANC_HS_ADCH_READ_ERR, 0);
#endif
		return false;
	}
	/* compute an average value */
	delta /= count;
	hwlog_info("%s:final adc value= %d,count=%d\n", __func__, delta, count);
	return delta;
}

static int anc_hs_send_hifi_msg(int anc_status)
{
	struct mlib_para_meter_voice  *para_anc_hs = NULL;
	struct mlib_set_para_info *mlib_set_para = NULL;
	int ret = OK_RET;

	mlib_set_para = kzalloc(sizeof(*mlib_set_para) +
		MLIB_PARA_LENGTH_MAX, GFP_KERNEL);
	if (!mlib_set_para)
		return ERROR_RET;

	mlib_set_para->msg_id = ID_AP_AUDIO_MLIB_SET_PARA_IND;
	mlib_set_para->path_id = MLIB_PATH_CS_VOICE_CALL_MICIN;
	mlib_set_para->size = MLIB_PARA_LENGTH_MAX;
	mlib_set_para->reserve = 0;
	mlib_set_para->module_id = MLIB_MODULE_3A_VOICE;

	para_anc_hs = (struct mlib_para_meter_voice *)mlib_set_para->auc_data;

	if (!para_anc_hs) {
		kfree(mlib_set_para);
		hwlog_err("%s: kzalloc failed\n", __func__);
		return ERROR_RET;
	}

	para_anc_hs->key = MLIB_ANC_HS_PARA_ENABLE;
	para_anc_hs->value = anc_status;
#ifdef CONFIG_HIFI_DSP_ONE_TRACK
	ret = hifi_send_msg(MAILBOX_MAILCODE_ACPU_TO_HIFI_MISC, mlib_set_para,
		sizeof(*mlib_set_para) + MLIB_PARA_LENGTH_MAX);
#endif
	kfree(mlib_set_para);
	return ret;
}

static bool anc_hs_need_charge(void)
{
	int delta;

	mdelay(30);

	delta = anc_hs_get_adc_delta();
	if ((delta >= anc_pdata->anc_hs_limit_min) &&
		(delta <= anc_pdata->anc_hs_limit_max)) {
		hwlog_info("%s %d anc headset = true\n", __func__, __LINE__);
	} else {
		hwlog_info("%s %d anc headset = false\n", __func__, __LINE__);
		return false;
	}

	return true;
}

static int anc_hs_get_btn_value(void)
{
	int delta;

	delta = anc_hs_get_adc_delta();
	if ((delta >= anc_pdata->anc_hs_btn_hook_min_voltage) &&
		(delta <= anc_pdata->anc_hs_btn_hook_max_voltage))
		return SND_JACK_BTN_0;

	if ((delta >= anc_pdata->anc_hs_btn_volume_up_min_voltage) &&
		(delta <= anc_pdata->anc_hs_btn_volume_up_max_voltage))
		return SND_JACK_BTN_1;

	if ((delta >= anc_pdata->anc_hs_btn_volume_down_min_voltage) &&
		(delta <= anc_pdata->anc_hs_btn_volume_down_max_voltage))
		return SND_JACK_BTN_2;

	hwlog_err("[anc_hs]btn delta not in range delta:%d\n", delta);
#ifdef CONFIG_HUAWEI_DSM
	anc_dsm_report(ANC_HS_BTN_NOT_IN_RANGE, 0);
#endif
	return NO_BUTTON_PRESS;
}

static void anc_hs_adc_calibration(void)
{
	int loop = ADC_CALIBRATION_TIMES;
	int count;
	int fail_count = 0;
	int adc_h;
	int adc_l;

	anc_pdata->adc_calibration_base = 0;

	while (loop) {
		loop--;
		usleep_range(1000, 1100);
		adc_h = hisi_adc_get_value(anc_pdata->channel_pwl_h);
		if (adc_h < 0) {
			hwlog_err("[anc_hs]get adc fail,adc_h:%d\n", adc_h);
			fail_count++;
			continue;
		}
		adc_l = hisi_adc_get_value(anc_pdata->channel_pwl_l);
		if (adc_l < 0) {
			hwlog_err("[anc_hs]get adc fail,adc_l:%d\n", adc_l);
			fail_count++;
			continue;
		}

		/* one calibrate value completely unnormal, abandon it */
		if ((adc_h - adc_l < ADC_VAL_MIN) ||
			(adc_h - adc_l > ADC_VAL_MAX)) {
			hwlog_err("[anc_hs]adc value is not expect, %d\n",
				adc_h - adc_l);
			fail_count++;
			continue;
		}
		anc_pdata->adc_calibration_base += (adc_h - adc_l);
	}

	count = ADC_CALIBRATION_TIMES - loop - fail_count;
	if (count == 0) {
		/* if all adc read fail, set 0 to it as default */
		anc_pdata->adc_calibration_base = 0;
		hwlog_err("[anc_hs] calibration whole failed\n");
	} else {
		anc_pdata->adc_calibration_base /= count;
		hwlog_info("anc_hs:calibration_base = %d with %d times\n",
			anc_pdata->adc_calibration_base, count);

		if (anc_pdata->adc_calibration_base > ADC_CALIBRAT_MAX ||
			anc_pdata->adc_calibration_base < ADC_CALIBRAT_MIN) {
			anc_pdata->adc_calibration_base = 0;
			hwlog_err("[anc_hs] calibration value is not illegal, error occurred\n");
		}
	}
}

static bool anc_hs_charge_judge(void)
{
	struct anc_hs_dev *pdev = anc_pdata->dev;
	struct anc_hs_codec_ops *fops = &pdev->ops;

	/* userspace prohibit charging with using hs-mic pin */
	if (anc_pdata->force_charge_ctl == ANC_HS_DISABLE_CHARGE) {
		hwlog_info("%s %u:charge is occupied by app level\n",
			__func__, __LINE__);
		/* need second detect for charge */
		anc_pdata->detect_again = true;
		return false;
	}

	/* hs mic is using record, not take it */
	if (anc_pdata->hs_micbias_ctl == ANC_HS_DISABLE_CHARGE) {
		/* need second detect for charge */
		hwlog_info("%s %u:hs mic is in using\n", __func__, __LINE__);
		anc_pdata->detect_again = true;
		return false;
	}

	anc_pdata->detect_again = false;
	hwlog_debug("%s %u:anc hs charge\n", __func__, __LINE__);

	/* headset may have pluged out, just return */
	if (!fops->check_headset_in(anc_pdata->private_data)) {
		hwlog_info("%s %u:headset has plug out\n", __func__, __LINE__);
		anc_hs_dump();
		return false;
	}

	mutex_lock(&anc_pdata->charge_lock);
	/* connect 5vboost with hs_mic pin */
	anc_hs_gpio_set_value(anc_pdata->gpio_mic_sw, SWITCH_CHIP_5VBOOST);
	mutex_unlock(&anc_pdata->charge_lock);

	/* waiting for anc chip start up */
	hwlog_info("%s: delay %d ms to wait anc chip up\n", __func__,
		anc_pdata->sleep_time);

	mdelay(anc_pdata->sleep_time);

	mutex_lock(&anc_pdata->charge_lock);

	if ((anc_pdata->hs_micbias_ctl == ANC_HS_ENABLE_CHARGE) &&
		anc_hs_need_charge()) {
		/* start to charge for anc hs and respond charging btn event */
		if (anc_pdata->anc_hs_mode == ANC_HS_CHARGE_OFF) {
			hwlog_info("%s %u:anc_hs enable irq\n",
				__func__, __LINE__);
			anc_hs_enable_irq();
			anc_pdata->anc_hs_mode = ANC_HS_CHARGE_ON;
		}
		if (anc_hs_send_hifi_msg(ANC_HS_CHARGE_ON) == ERROR_RET)
			hwlog_err("%s %u:AncHsSendHifiMsg turn on Anc FAIL\n",
				__func__, __LINE__);
	} else {
		if (anc_pdata->anc_hs_mode == ANC_HS_CHARGE_ON) {
			anc_hs_disable_irq();
			hwlog_info("%s %u:anc_hs disable irq\n",
				__func__, __LINE__);
		}
		/* stop charge and change status to CHARGE_OFF */
		anc_hs_gpio_set_value(anc_pdata->gpio_mic_sw,
			SWITCH_CHIP_HSBIAS);
		udelay(UDELAY_TIME);
		anc_pdata->anc_hs_mode = ANC_HS_CHARGE_OFF;
		if (anc_hs_send_hifi_msg(ANC_HS_CHARGE_OFF) == ERROR_RET)
			hwlog_err("%s %u:AncHsSendHifiMsg turn off Anc FAIL\n",
				__func__, __LINE__);
	}

	anc_hs_dump();

	mutex_unlock(&anc_pdata->charge_lock);
	if (anc_pdata->anc_hs_mode == ANC_HS_CHARGE_ON)
		return true;

	return false;
}

static void update_charge_status(void)
{
	struct anc_hs_dev *pdev = anc_pdata->dev;
	struct anc_hs_codec_ops *fops = &pdev->ops;

	if (anc_pdata->hs_micbias_ctl == ANC_HS_DISABLE_CHARGE ||
		anc_pdata->force_charge_ctl == ANC_HS_DISABLE_CHARGE) {
		/* force stop charge function */
		mutex_lock(&anc_pdata->charge_lock);

		if (anc_pdata->anc_hs_mode == ANC_HS_CHARGE_ON) {
			anc_hs_disable_irq();
			anc_hs_gpio_set_value(anc_pdata->gpio_mic_sw,
				SWITCH_CHIP_HSBIAS);

			udelay(UDELAY_TIME);

			hwlog_info("%s %u:stop charging for anc hs\n",
				__func__, __LINE__);
			anc_pdata->anc_hs_mode = ANC_HS_CHARGE_OFF;
			anc_pdata->mic_used = true;
		} else {
			/* here just make a dmd report */
			if (anc_hs_gpio_get_value(anc_pdata->gpio_mic_sw) ==
				SWITCH_CHIP_5VBOOST) {
				hwlog_err("%s %u:gpio status is not right\n",
					__func__, __LINE__);
#ifdef CONFIG_HUAWEI_DSM
				anc_dsm_report(ANC_HS_MIC_WITH_GPIO_ERR, 0);
#endif
			}
		}

		anc_hs_dump();
		mutex_unlock(&anc_pdata->charge_lock);
	} else if (anc_pdata->hs_micbias_ctl == ANC_HS_ENABLE_CHARGE &&
		anc_pdata->force_charge_ctl == ANC_HS_ENABLE_CHARGE) {
		if (anc_pdata->mic_used) {
			anc_pdata->mic_used = false;
			/* headset maybe have plug out here */
			if (!fops->check_headset_in(anc_pdata->private_data)) {
				hwlog_info("%s %u:headset has plug out\n",
					__func__, __LINE__);
				anc_hs_dump();
			} else {
				/* force resume charge for anc headset */
				mutex_lock(&anc_pdata->charge_lock);
				if (anc_pdata->anc_hs_mode == ANC_HS_CHARGE_OFF) {
					anc_pdata->anc_hs_mode = ANC_HS_CHARGE_ON;
					anc_hs_gpio_set_value(anc_pdata->gpio_mic_sw,
						SWITCH_CHIP_5VBOOST);
					udelay(UDELAY_TIME);
					anc_hs_enable_irq();
					hwlog_info("%s %u:resume charge anc\n",
						__func__, __LINE__);
				}
				mutex_unlock(&anc_pdata->charge_lock);
			}
		} else if (anc_pdata->detect_again) {
			/* need detect charge again due to interrupted before */
			anc_hs_charge_judge();
		}
	}
}

void anc_hs_start_charge(void)
{
	if (!anc_pdata || !anc_pdata->registered)
		return;

	anc_pdata->hs_status = ANC_HS_HEADSET_PLUG_IN;
	anc_hs_disable_irq();

	/* enable 5vboost first, this need time to be stable */
	hwlog_info("%s %u:enable 5vboost\n", __func__, __LINE__);

	/* default let hsbias connect to hs-mic pin */
	anc_hs_gpio_set_value(anc_pdata->gpio_mic_sw, SWITCH_CHIP_HSBIAS);
	anc_hs_gpio_set_value(anc_pdata->anc_pwr_en_gpio, 1);
	enable_boost(true);
	anc_pdata->anc_hs_mode = ANC_HS_CHARGE_OFF;

	anc_hs_dump();
}

void anc_hs_force_charge(int disable)
{
	if (!anc_pdata || !anc_pdata->registered)
		return;

	/* don't make repeated switch with charge status */
	if (disable == anc_pdata->hs_micbias_ctl)
		return;

	anc_pdata->hs_micbias_ctl = disable;

	/* update charge status here */
	update_charge_status();
}

bool anc_hs_charge_detect(int saradc_value, int headset_type)
{
	if (!anc_pdata || !anc_pdata->registered)
		return false;

	/*
	 * calibration adc resistance which can
	 * make charge detect more accuracy
	 */
	anc_hs_adc_calibration();

	/*
	 * revert 4-pole headset still need 5vboost on
	 * to support second recognition
	 */
	if (headset_type == ANC_HS_NORMAL_4POLE) {
		/* 4-pole headset maybe an anc headset */
		hwlog_debug("%s : start anc hs charge judge\n", __func__);
		return anc_hs_charge_judge();
	}

	if (headset_type == ANC_HS_NORMAL_3POLE) {
		hwlog_info("%s : no disable 5vboost for 3-pole headset\n",
			__func__);
		/* 3-pole also support second-detect */
		return false;
	}

	return false;
}

void anc_hs_5v_control(int enable)
{
	if (!anc_pdata || !anc_pdata->registered)
		return;

	if (enable) {
		anc_hs_gpio_set_value(anc_pdata->anc_pwr_en_gpio, 1);
	} else {
		if (anc_pdata->hs_status == ANC_HS_HEADSET_PLUG_OUT)
			anc_hs_gpio_set_value(anc_pdata->anc_pwr_en_gpio, 0);
	}
}

void anc_hs_stop_charge(void)
{
	if (!anc_pdata || !anc_pdata->registered)
		return;

	hwlog_info("%s : stop anc hs charge\n", __func__);

	/* keep irq always in mask status */
	anc_hs_disable_irq();

	/* cancel the extra button delay work when headset plug out */
	cancel_delayed_work(&anc_pdata->anc_hs_btn_delay_work);

	enable_boost(false);
	anc_hs_gpio_set_value(anc_pdata->gpio_mic_sw, SWITCH_CHIP_HSBIAS);
	anc_hs_gpio_set_value(anc_pdata->anc_pwr_en_gpio, 0);
	anc_pdata->hs_status = ANC_HS_HEADSET_PLUG_OUT;
	anc_pdata->anc_hs_mode = ANC_HS_CHARGE_OFF;
	if (anc_hs_send_hifi_msg(ANC_HS_CHARGE_OFF) == ERROR_RET)
		hwlog_err("%s %u:AncHsSendHifiMsg TURN OFF AncHs FAIL\n",
			__func__, __LINE__);
}

int anc_hs_dev_register(struct anc_hs_dev *dev, void *codec_data)
{
	/* anc_hs driver not be probed, just return */
	if (!anc_pdata)
		return -ENODEV;

	/* only support one codec to be registered */
	if (anc_pdata->registered) {
		hwlog_err("one codec has registered, no more permit\n");
		return -EEXIST;
	}
	if (!dev->ops.check_headset_in || !dev->ops.btn_report ||
		!dev->ops.codec_resume_lock) {
		hwlog_err("codec ops function must be all registed\n");
		return -EINVAL;
	}

	anc_pdata->dev = dev;
	anc_pdata->private_data = codec_data;
	anc_pdata->registered = true;

	hwlog_info("%s %u:anc hs has been register sucessful\n",
		__func__, __LINE__);

	return 0;
}

bool check_anc_hs_support(void)
{
	if (!anc_pdata || !anc_pdata->registered)
		return false;

	return true;
}

static void anc_hs_btn_judge(struct work_struct *work)
{
	struct anc_hs_dev *pdev = anc_pdata->dev;
	struct anc_hs_codec_ops *fops = &pdev->ops;
	int btn_report;

	if (!anc_pdata->registered)
		return;

	if (anc_pdata->anc_hs_mode == ANC_HS_CHARGE_OFF) {
		hwlog_info("%s %u:ignore this irq\n", __func__, __LINE__);
		if (anc_pdata->button_pressed == 1) {
			btn_report = 0;
			anc_pdata->button_pressed = 0;
			fops->btn_report(btn_report, ANC_BTN_MASK);
			hwlog_info("%s:irq level:%d", __func__,
				anc_hs_gpio_get_value(anc_pdata->mic_irq_gpio));
		}
		return;
	}

	hwlog_info("%s %u:deal with button irq event\n", __func__, __LINE__);

	/* should get wake lock before codec power lock which may be blocked */
	__pm_stay_awake(&anc_pdata->wake_lock);

	/* enable irq first */
	anc_hs_enable_irq();
	mutex_lock(&anc_pdata->btn_mutex);

	if (!anc_hs_gpio_get_value(anc_pdata->mic_irq_gpio) &&
		(anc_pdata->button_pressed == 0)) {
		/* button down event */
		hwlog_info("%s %u:button down event\n", __func__, __LINE__);
		mdelay(50);
		btn_report = anc_hs_get_btn_value();
		if (btn_report != NO_BUTTON_PRESS) {
			anc_pdata->button_pressed = 1;
			fops->btn_report(btn_report, ANC_BTN_MASK);
		}
	} else if (anc_pdata->button_pressed == 1) {
		/* button up event */
		hwlog_info("%s %u:button up event\n", __func__, __LINE__);

		btn_report = 0;
		anc_pdata->button_pressed = 0;

		/*
		 * we permit button up event report to userspace,
		 * make sure down and up in pair
		 */
		fops->btn_report(btn_report, ANC_BTN_MASK);
	}

	mutex_unlock(&anc_pdata->btn_mutex);
	__pm_relax(&anc_pdata->wake_lock);
}

static irqreturn_t anc_hs_btn_handler(int irq, void *data)
{
	/* make sure delay_work to be scheduled */
	__pm_wakeup_event(&anc_pdata->wake_lock, jiffies_to_msecs(50));

	anc_hs_disable_irq();

	/* deal with button judge at bottom */
	queue_delayed_work(anc_pdata->anc_hs_btn_delay_wq,
		&anc_pdata->anc_hs_btn_delay_work, msecs_to_jiffies(20));

	return IRQ_HANDLED;
}

static long anc_hs_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;
	int charge_mode;
	unsigned int __user *p_user = (unsigned int __user *)arg;

	if (!p_user)
		return -EBUSY;

	if (!anc_pdata->registered)
		return -EBUSY;

	switch (cmd) {
	case IOCTL_ANC_HS_CHARGE_ENABLE_CMD:
		if (anc_pdata->force_charge_ctl == ANC_HS_ENABLE_CHARGE)
			break;
		/* resume anc headset charge */
		anc_pdata->force_charge_ctl = ANC_HS_ENABLE_CHARGE;
		hwlog_info("app level contrl set charge status with %d\n",
			anc_pdata->force_charge_ctl);
		update_charge_status();
		break;
	case IOCTL_ANC_HS_CHARGE_DISABLE_CMD:
		if (anc_pdata->force_charge_ctl == ANC_HS_DISABLE_CHARGE)
			break;
		/* force stop anc headset charge */
		anc_pdata->force_charge_ctl = ANC_HS_DISABLE_CHARGE;
		hwlog_info("app level contrl set charge status with %d\n",
			anc_pdata->force_charge_ctl);
		update_charge_status();
		break;
	case IOCTL_ANC_HS_GET_HEADSET_CMD:
		charge_mode = anc_pdata->anc_hs_mode;
		if (charge_mode == ANC_HS_CHARGE_ON) {
			if (!anc_hs_need_charge()) {
				charge_mode = ANC_HS_CHARGE_OFF;
			} else {
				usleep_range(10000, 10100);
				if (!anc_hs_need_charge())
					charge_mode = ANC_HS_CHARGE_OFF;
			}
		}
		if (charge_mode == ANC_HS_CHARGE_ON)
			ret = put_user((__u32)(ANC_HS_HEADSET), p_user);
		else
			ret = put_user((__u32)(ANC_HS_NORMAL_4POLE), p_user);
		break;
	case IOCTL_ANC_HS_GET_CHARGE_STATUS_CMD:
		ret = put_user((__u32)(anc_pdata->anc_hs_mode), p_user);
		break;
	default:
		hwlog_err("unsupport cmd\n");
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

#ifdef ANC_HS_DEBUG
static ssize_t anc_hs_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
#define HS_INFO_SIZE 512
	if (!buf)
		return 0;

	memset(buf, 0, HS_INFO_SIZE);

	snprintf(buf, HS_INFO_SIZE, "%sanc_hs_limit_min: %d\n", buf,
		anc_pdata->anc_hs_limit_min);
	snprintf(buf, HS_INFO_SIZE, "%sanc_hs_limit_max: %d\n", buf,
		anc_pdata->anc_hs_limit_max);
	snprintf(buf, HS_INFO_SIZE, "%sanc_hs_mode: %d\n", buf,
		anc_pdata->anc_hs_mode);
	snprintf(buf, HS_INFO_SIZE, "%sirq_flag: %d\n", buf,
		anc_pdata->irq_flag);
	snprintf(buf, HS_INFO_SIZE, "%schannel_pwl_h: %d\n", buf,
		anc_pdata->channel_pwl_h);
	snprintf(buf, HS_INFO_SIZE, "%schannel_pwl_l: %d\n", buf,
		anc_pdata->channel_pwl_l);
	snprintf(buf, HS_INFO_SIZE, "%sgpio_mic_sw: %d\n", buf,
		anc_hs_gpio_get_value(anc_pdata->gpio_mic_sw));
	snprintf(buf, HS_INFO_SIZE, "%smic_used: %d\n", buf,
		anc_pdata->mic_used);
	snprintf(buf, HS_INFO_SIZE, "%sadc_calibration_base: %d\n", buf,
		anc_pdata->adc_calibration_base);
	snprintf(buf, HS_INFO_SIZE, "%scalibration_current: %d\n", buf,
		(hisi_adc_get_value(anc_pdata->channel_pwl_h) -
		hisi_adc_get_value(anc_pdata->channel_pwl_l)));

	snprintf(buf, HS_INFO_SIZE, "%sadc_h: %d\n", buf,
		hisi_adc_get_value(anc_pdata->channel_pwl_h));
	snprintf(buf, HS_INFO_SIZE, "%sadc_l: %d\n", buf,
		hisi_adc_get_value(anc_pdata->channel_pwl_l));
	snprintf(buf, HS_INFO_SIZE, "%sanc_hs_btn_hook_min_voltage: %d\n", buf,
		anc_pdata->anc_hs_btn_hook_min_voltage);
	snprintf(buf, HS_INFO_SIZE, "%sanc_hs_btn_hook_max_voltage: %d\n", buf,
		anc_pdata->anc_hs_btn_hook_max_voltage);
	snprintf(buf, HS_INFO_SIZE, "%sanc_hs_btn_volume_up_min_voltage: %d\n",
		buf, anc_pdata->anc_hs_btn_volume_up_min_voltage);
	snprintf(buf, HS_INFO_SIZE, "%sanc_hs_btn_volume_up_max_voltage: %d\n",
		buf, anc_pdata->anc_hs_btn_volume_up_max_voltage);
	snprintf(buf, HS_INFO_SIZE, "%sanc_hs_btn_volume_down_min_voltage:%d\n",
		buf, anc_pdata->anc_hs_btn_volume_down_min_voltage);
	snprintf(buf, HS_INFO_SIZE, "%sanc_hs_btn_volume_down_max_voltage:%d\n",
		buf, anc_pdata->anc_hs_btn_volume_down_max_voltage);

	return HS_INFO_SIZE;
}

static ssize_t anc_detect_limit_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!buf)
		return 0;

	return snprintf(buf, BIG_BUFFER_SIZE, "%d\n",
		anc_pdata->anc_hs_limit_min);
}

static ssize_t anc_detect_limit_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;

	if (!buf)
		return 0;

	ret = kstrtoint(buf, SMALL_BUFFER_SIZE, &(anc_pdata->anc_hs_limit_min));
	if (ret) {
		hwlog_err("%s : convert to int type failed\n", __func__);
		return ret;
	}

	return count;
}

static ssize_t anc_gpio_sw_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!buf)
		return 0;

	return snprintf(buf, BIG_BUFFER_SIZE, "%d\n",
		anc_hs_gpio_get_value(anc_pdata->gpio_mic_sw));
}

static ssize_t anc_gpio_sw_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	int value = 0;

	if (!buf)
		return 0;

	ret = kstrtoint(buf, SMALL_BUFFER_SIZE, &value);
	if (ret) {
		hwlog_err("%s : convert to int type failed\n", __func__);
		return ret;
	}
	if (value)
		anc_hs_gpio_set_value(anc_pdata->gpio_mic_sw, 1);
	else
		anc_hs_gpio_set_value(anc_pdata->gpio_mic_sw, 0);

	return count;
}

static ssize_t anc_precharge_time_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!buf)
		return 0;

	return snprintf(buf, BIG_BUFFER_SIZE, "%d\n", anc_pdata->sleep_time);
}

static ssize_t anc_precharge_time_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	int value = 0;

	if (!buf)
		return 0;

	ret = kstrtoint(buf, SMALL_BUFFER_SIZE, &value);
	if (ret) {
		hwlog_err("%s : convert to int type failed\n", __func__);
		return ret;
	}
	anc_pdata->sleep_time = value;
	return count;
}

static DEVICE_ATTR(hs_info, 0664, anc_hs_info_show, NULL);
static DEVICE_ATTR(detect_limit, 0660, anc_detect_limit_show,
	anc_detect_limit_store);
static DEVICE_ATTR(gpio_sw, 0660, anc_gpio_sw_show, anc_gpio_sw_store);
static DEVICE_ATTR(precharge_time, 0660, anc_precharge_time_show,
	anc_precharge_time_store);

static struct attribute *anc_hs_attributes[] = {
	&dev_attr_hs_info.attr,
	&dev_attr_detect_limit.attr,
	&dev_attr_gpio_sw.attr,
	&dev_attr_precharge_time.attr,
	NULL
};

static const struct attribute_group anc_hs_attr_group = {
	.attrs = anc_hs_attributes,
};
#endif

static const struct file_operations anc_hs_fops = {
	.owner               = THIS_MODULE,
	.open                = simple_open,
	.unlocked_ioctl      = anc_hs_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl        = anc_hs_ioctl,
#endif
};

static struct miscdevice anc_hs_device = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = "anc_hs",
	.fops   = &anc_hs_fops,
};

static const struct of_device_id anc_hs_of_match[] = {
	{
		.compatible = "huawei,anc_hs",
	},
	{},
};
MODULE_DEVICE_TABLE(of, anc_hs_of_match);

static void read_adc_channel_number(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "adc_channel_h", &temp))
		anc_pdata->channel_pwl_h = temp;
	else
		anc_pdata->channel_pwl_h = ANC_ADC_CHANNEL_H_DEFAULT;

	if (!of_property_read_u32(node, "adc_channel_l", &temp))
		anc_pdata->channel_pwl_l = temp;
	else
		anc_pdata->channel_pwl_l = ANC_ADC_CHANNEL_L_DEFAULT;
}

static void read_charge_limit(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "anc_hs_limit_min", &temp))
		anc_pdata->anc_hs_limit_min = temp;
	else
		anc_pdata->anc_hs_limit_min = ANC_HS_LIMIT_MIN;

	if (!of_property_read_u32(node, "anc_hs_limit_max", &temp))
		anc_pdata->anc_hs_limit_max = temp;
	else
		anc_pdata->anc_hs_limit_max = ANC_HS_LIMIT_MAX;

	if (!of_property_read_u32(node, "gpio_type", &temp))
		anc_pdata->gpio_type = temp;
	else
		anc_pdata->gpio_type = ANC_HS_GPIO_SOC;
}

static void read_hook_limit(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "anc_hs_btn_hook_min_voltage", &temp))
		anc_pdata->anc_hs_btn_hook_min_voltage = temp;
	else
		anc_pdata->anc_hs_btn_hook_min_voltage = ANC_HS_HOOK_MIN;

	if (!of_property_read_u32(node, "anc_hs_btn_hook_max_voltage", &temp))
		anc_pdata->anc_hs_btn_hook_max_voltage = temp;
	else
		anc_pdata->anc_hs_btn_hook_max_voltage = ANC_HS_HOOK_MAX;
}

static void read_volume_up_limit(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "anc_hs_btn_volume_up_min_voltage",
		&temp))
		anc_pdata->anc_hs_btn_volume_up_min_voltage = temp;
	else
		anc_pdata->anc_hs_btn_volume_up_min_voltage =
			ANC_HS_VOLUME_UP_MIN;

	if (!of_property_read_u32(node, "anc_hs_btn_volume_up_max_voltage",
		&temp))
		anc_pdata->anc_hs_btn_volume_up_max_voltage = temp;
	else
		anc_pdata->anc_hs_btn_volume_up_max_voltage =
			ANC_HS_VOLUME_UP_MAX;
}

static void read_volume_down_limit(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "anc_hs_btn_volume_down_min_voltage",
		&temp))
		anc_pdata->anc_hs_btn_volume_down_min_voltage = temp;
	else
		anc_pdata->anc_hs_btn_volume_down_min_voltage =
			ANC_HS_VOLUME_DOWN_MIN;

	if (!of_property_read_u32(node, "anc_hs_btn_volume_down_max_voltage",
		&temp))
		anc_pdata->anc_hs_btn_volume_down_max_voltage = temp;
	else
		anc_pdata->anc_hs_btn_volume_down_max_voltage =
			ANC_HS_VOLUME_DOWN_MAX;
}

static void load_anc_hs_config(struct device_node *node)
{
	read_adc_channel_number(node);

	read_charge_limit(node);

	read_hook_limit(node);

	read_volume_up_limit(node);

	read_volume_down_limit(node);
}

static struct anc_hs_ops anc_hs_ops = {
	.anc_hs_dev_register = anc_hs_dev_register,
	.anc_hs_check_headset_pluged_in = NULL,
	.anc_hs_start_charge = anc_hs_start_charge,
	.anc_hs_charge_detect = anc_hs_charge_detect,
	.anc_hs_stop_charge = anc_hs_stop_charge,
	.anc_hs_force_charge = anc_hs_force_charge,
	.check_anc_hs_support = check_anc_hs_support,
	.anc_hs_5v_control = anc_hs_5v_control,
	.anc_hs_invert_hs_control = NULL,
	.anc_hs_refresh_headset_type = NULL,
};

static int hs_load_gpio(struct device *dev, int *gpio_index,
	int out_value, const char *gpio_name, int index,
	const char *request_gpio_name, bool gpio_output)
{
	int gpio;
	int ret;

	gpio = of_get_named_gpio(dev->of_node, gpio_name, index);
	if (gpio < 0) {
		hwlog_info("%s:Looking up %s property in node %s failed %d\n",
			__func__, dev->of_node->full_name, gpio_name, gpio);
		return -ENOENT;
	}
	if (!gpio_is_valid(gpio)) {
		hwlog_err("%s mic_gnd_switch is unvalid\n", __func__);
		return -ENOENT;
	}
	ret = gpio_request(gpio, request_gpio_name);
	if (ret < 0) {
		hwlog_err("%s request GPIO for %s fail %d\n",
			__func__, request_gpio_name, ret);
	} else {
		if (gpio_output)
			gpio_direction_output(gpio, out_value);
		else
			gpio_direction_input(gpio);
	}
	*gpio_index = gpio;
	return ret;
}

static int anc_hs_load_gpio_pdata(struct device *dev)
{
	if (hs_load_gpio(dev, &(anc_pdata->gpio_mic_sw), SWITCH_CHIP_HSBIAS,
		"gpios", 0, "gpio_mic_sw", true) < 0)
		goto anc_get_gpio_mic_sw_err;

	if (hs_load_gpio(dev, &(anc_pdata->mic_irq_gpio), 0,
		"gpios", 1, "gpio_mic_irq", false) < 0)
		goto anc_get_gpio_mic_irq_err;

	if (hs_load_gpio(dev, &(anc_pdata->anc_pwr_en_gpio), SWITCH_CHIP_HSBIAS,
		"gpios", 2, "gpio_anc_pwr_en", true) < 0)
		goto anc_get_gpio_anc_pwr_en_err;

	return 0;

anc_get_gpio_anc_pwr_en_err:
	if (anc_pdata->mic_irq_gpio > 0)
		gpio_free(anc_pdata->mic_irq_gpio);
anc_get_gpio_mic_irq_err:
	if (anc_pdata->gpio_mic_sw > 0)
		gpio_free(anc_pdata->gpio_mic_sw);
anc_get_gpio_mic_sw_err:
	return -ENOENT;
}

static void value_initial(void)
{
	/* init all values */
	anc_pdata->anc_hs_mode = ANC_HS_CHARGE_OFF;
	anc_pdata->irq_flag = true;
	anc_pdata->mic_used = false;
	anc_pdata->sleep_time = ANC_CHIP_STARTUP_TIME;
	anc_pdata->adc_calibration_base = 0;
	anc_pdata->hs_micbias_ctl = ANC_HS_ENABLE_CHARGE;
	anc_pdata->force_charge_ctl = ANC_HS_ENABLE_CHARGE;
	anc_pdata->boost_flag = false;
	anc_pdata->registered = false;
	anc_pdata->hs_status = ANC_HS_HEADSET_PLUG_OUT;
}

static void anc_hs_remove_workqueue(void)
{
	if (anc_pdata->anc_hs_btn_delay_wq) {
		cancel_delayed_work(&anc_pdata->anc_hs_btn_delay_work);
		flush_workqueue(anc_pdata->anc_hs_btn_delay_wq);
		destroy_workqueue(anc_pdata->anc_hs_btn_delay_wq);
	}
}

static int anc_hs_create_workqueue(void)
{
	anc_pdata->anc_hs_btn_delay_wq =
		create_singlethread_workqueue("anc_hs_btn_delay_wq");
	if (!(anc_pdata->anc_hs_btn_delay_wq)) {
		hwlog_err("%s : anc_hs_btn_delay_wq create failed\n", __func__);
		return -ENOMEM;
	}
	INIT_DELAYED_WORK(&anc_pdata->anc_hs_btn_delay_work, anc_hs_btn_judge);

	return 0;
}

static int anc_hs_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	int ret;

	anc_pdata = devm_kzalloc(dev, sizeof(*anc_pdata), GFP_KERNEL);
	if (!anc_pdata)
		return -ENOMEM;

	mutex_init(&anc_pdata->charge_lock);
	mutex_init(&anc_pdata->btn_mutex);
	wakeup_source_init(&anc_pdata->wake_lock, "anc_hs");

	value_initial();

	/* CONFIG_GPIO_HI6402 is temporarily added for chicago 4.1 kernel */
#ifdef CONFIG_GPIO_HI6402
	ret = anc_hs_load_gpio_pdata(dev);
	if (ret < 0) {
		hwlog_err("%s get gpios failed, ret =%d\n", __func__, ret);
		goto gpio_mic_sw_err;
	}

	anc_pdata->mic_irq = gpio_to_irq(anc_pdata->mic_irq_gpio);

	if (of_property_read_u32(node, "check_reg_value",
		&anc_pdata->check_reg_value))
		anc_pdata->check_reg_value = 0;

	/* load dts config for board difference */
	load_anc_hs_config(node);

	/* create workqueue */
	ret = anc_hs_create_workqueue();
	if (ret)
		goto anc_hs_btn_wq_err;

	/* anc hs irq request */
	ret = request_threaded_irq(anc_pdata->mic_irq, NULL, anc_hs_btn_handler,
		IRQF_ONESHOT | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
		IRQF_NO_SUSPEND, "anc_hs_btn", NULL);

	if (ret < 0)
		goto anc_hs_btn_err;
	/* disable btn irq by default */
	anc_hs_disable_irq();

	/* register misc device for userspace */
	ret = misc_register(&anc_hs_device);
	if (ret) {
		hwlog_err("%s: anc hs misc device register failed", __func__);
		goto anc_hs_irq_err;
	}
#ifdef ANC_HS_DEBUG
	/* create sysfs for debug function */
	ret = sysfs_create_group(&dev->kobj, &anc_hs_attr_group);
	if (ret < 0)
		hwlog_err("failed to register sysfs\n");
#endif
	ret = anc_hs_ops_register(&anc_hs_ops);
	if (ret) {
		pr_err("register anc_hs_interface ops failed\n");
		goto anc_hs_irq_err;
	}

	return 0;

anc_hs_irq_err:
	free_irq(anc_pdata->mic_irq, NULL);
anc_hs_btn_err:
	anc_hs_remove_workqueue();
anc_hs_btn_wq_err:
	gpio_free(anc_pdata->gpio_mic_sw);
gpio_mic_sw_err:
	wakeup_source_trash(&anc_pdata->wake_lock);
	kfree(anc_pdata);
	anc_pdata = NULL;
#endif
	return ret;
}

static int anc_hs_remove(struct platform_device *pdev)
{
	if (!anc_pdata)
		return 0;

	free_irq(anc_pdata->mic_irq, NULL);

	if (anc_pdata->anc_hs_btn_delay_wq) {
		cancel_delayed_work(&anc_pdata->anc_hs_btn_delay_work);
		flush_workqueue(anc_pdata->anc_hs_btn_delay_wq);
		destroy_workqueue(anc_pdata->anc_hs_btn_delay_wq);
	}

	if (anc_pdata->anc_pwr_en_gpio >= 0) {
		gpio_direction_output(anc_pdata->anc_pwr_en_gpio, 0);
		gpio_free(anc_pdata->anc_pwr_en_gpio);
	}
	gpio_free(anc_pdata->gpio_mic_sw);
	gpio_free(anc_pdata->mic_irq_gpio);
	kfree(anc_pdata);
	anc_pdata = NULL;

	return 0;
}

static struct platform_driver anc_hs_driver = {
	.driver = {
		.name   = "anc_hs",
		.owner  = THIS_MODULE,
		.of_match_table = anc_hs_of_match,
	},
	.probe  = anc_hs_probe,
	.remove = anc_hs_remove,
};

static int __init anc_hs_init(void)
{
	return platform_driver_register(&anc_hs_driver);
}

static void __exit anc_hs_exit(void)
{
	platform_driver_unregister(&anc_hs_driver);
}

device_initcall(anc_hs_init);
module_exit(anc_hs_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("anc headset driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

#ifdef CONFIG_LLT_TEST
struct anc_hs_static_ops anc_hs_ops_llt = {
	.anc_hs_gpio_get_value = anc_hs_gpio_get_value,
	.anc_hs_gpio_set_value = anc_hs_gpio_set_value,
	.anc_hs_dump = anc_hs_dump,
#ifdef CONFIG_HUAWEI_DSM
	.anc_dsm_report = anc_dsm_report,
#endif
	.anc_hs_enable_irq = anc_hs_enable_irq,
	.anc_hs_disable_irq = anc_hs_disable_irq,
	.enable_boost = enable_boost,
	.anc_hs_get_adc_delta = anc_hs_get_adc_delta,
	.anc_hs_need_charge = anc_hs_need_charge,
	.anc_hs_get_btn_value = anc_hs_get_btn_value,
	.anc_hs_adc_calibration = anc_hs_adc_calibration,
	.check_anc_hs_support = check_anc_hs_support,
	.anc_hs_ioctl = anc_hs_ioctl,
	.anc_detect_limit_store = anc_detect_limit_store,
	.anc_gpio_sw_store = anc_gpio_sw_store,
	.anc_precharge_time_store = anc_precharge_time_store,
	.load_anc_hs_config = load_anc_hs_config,
	.anc_hs_probe = anc_hs_probe,
	.anc_hs_remove = anc_hs_remove,
	.anc_pdata = &anc_pdata,
};
#endif
