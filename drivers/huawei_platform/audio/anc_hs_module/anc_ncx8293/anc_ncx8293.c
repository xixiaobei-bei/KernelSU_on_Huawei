/*
 * anc_ncx8293.c
 *
 * anc ncx8293 headset driver
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
 */

#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/irq.h>
#include <linux/ioctl.h>
#include <linux/io.h>
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
#include <linux/regmap.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/hisi/hisi_adc.h>
#include <media/huawei/hw_extern_pmic.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif
#include "huawei_platform/audio/anc_ncx8293.h"

#define UNUSED(x) ((void)x)
#define HWLOG_TAG anc_ncx8293
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
	ANC_HS_CHARGE_OFF        = 0,
	ANC_HS_CHARGE_ON         = 1,
};

enum anc_ncx8293_irq_type {
	ANC_NCX8293_JACK_PLUG_IN  = 0,
	ANC_NCX8293_JACK_PLUG_OUT,
	ANC_NCX8293_BUTTON_PRESS,
	ANC_NCX8293_IQR_MAX,
};

#define  ANC_HS_LIMIT_MIN                  20
#define  ANC_HS_LIMIT_MAX                  200
#define  ANC_CHIP_STARTUP_TIME             30
#define  ADC_CALIBRATION_TIMES             10
#define  ADC_READ_COUNT                    3
#define  ADC_NORMAL_LIMIT_MIN              (-500)
#define  ADC_NORMAL_LIMIT_MAX              500
#define  ANC_HS_HOOK_MIN                   160
#define  ANC_HS_HOOK_MAX                   185
#define  ANC_HS_VOLUME_UP_MIN              205
#define  ANC_HS_VOLUME_UP_MAX              230
#define  ANC_HS_VOLUME_DOWN_MIN            240
#define  ANC_HS_VOLUME_DOWN_MAX            265
#define  NO_BUTTON_PRESS                   (-1)
#define  CODEC_GPIO_BASE                   224
#define  I2C_REG_FAIL_MAX_TIME             10
#define  I2C_FAIL_REPORT_MAX_TIMES         20
#define  MDELAY_TIME                       50
#define  UDELAY_TIME                       500
#define  ADC_VAL_MIN                       (-100)
#define  ADC_VAL_MAX                       100
#define  ADC_CALIBRAT_MIN                  (-50)
#define  ADC_CALIBRAT_MAX                  50
#define  ANC_ADC_CHANNEL_H_DEFAULT         15
#define  ANC_ADC_CHANNEL_L_DEFAULT         14
#define  VAL_STR_SIZE                      20
#define  ANC_HS_VDD                        3300000
#define  ANC_REGISTER_MAX_ADDRESS          0x0f
#define  ANC_BTN_MASK (SND_JACK_BTN_0 | SND_JACK_BTN_1 | SND_JACK_BTN_2)

struct anc_ncx8293_priv {
	struct i2c_client *client;
	struct device *dev;
	struct regmap *l_regmap;
	int anc_charge;                     /* charge status */
	int gpio_pwr_en;                    /* VBST_5V EN */
	int h_channel_pwl;                  /* adc channel for high voltage */
	int l_channel_pwl;                  /* adc channel for low voltage */
	int anc_hs_min_limit;
	int anc_hs_max_limit;
	int anc_btn_hook_min_volt;
	int anc_btn_hook_max_volt;
	int anc_btn_volume_up_min_volt;
	int anc_btn_volume_up_max_volt;
	int anc_btn_volume_down_min_volt;
	int anc_btn_volume_down_max_volt;
	bool irq_status;
	bool boost_status;
	int sleep_time;                     /* charge chip pre-charge time */
	bool mic_status;                    /* flag to show mic status */
	bool detect_again;
	int force_charge;                   /* force charge control for user */
	int hs_micbias_ctl;                 /* hs micbias control */
	int adc_calibrate_base;             /* calibration value */
	int button_pressed;
	int hs_type;
	struct mutex btn_mutex;
	struct mutex charge_lock;           /* charge status protect lock */
	struct mutex invert_hs_lock;
	struct wakeup_source wake_lock;
	spinlock_t irq_lock;
	int registered;                     /* anc hs regester flag */
	struct anc_hs_dev *anc_dev;         /* anc hs dev */
	void *priv_data;                    /* store codec description data */
	int gpio_int;
	int anc_ncx8293_irq;
	bool ldo_supply_used;
	bool cam_ldo_used;
	int cam_ldo_num;
	int cam_ldo_val;
	struct regulator *anc_hs_vdd;
	/* chip irq workqueue */
	struct workqueue_struct *anc_plugin_delay_wq;
	struct delayed_work anc_plugin_delay_work;
	struct workqueue_struct *anc_plugout_delay_wq;
	struct delayed_work anc_plugout_delay_work;
	struct workqueue_struct *anc_btn_delay_wq;
	struct delayed_work anc_btn_delay_work;
	struct workqueue_struct *anc_invert_ctl_delay_wq;
	struct delayed_work anc_invert_ctl_delay_work;
};

static struct anc_ncx8293_priv *anc_priv;
static unsigned int i2c_reg_fail_time;
static unsigned int i2c_fail_report_times = I2C_FAIL_REPORT_MAX_TIMES;

static struct reg_default anc_ncx8293_reg[] = {
	{ 0x00, 0x17 }, /* Device ID */
	{ 0x01, 0x41 }, /* Device Setup */
	{ 0x02, 0x00 }, /* Interrupt */
	{ 0x03, 0x00 }, /* MIC Path Select */
	{ 0x04, 0x00 }, /* DET Trigger */
	{ 0x05, 0x00 }, /* Accessory Status */
	{ 0x06, 0x03 }, /* Switch Status 1 */
	{ 0x07, 0x00 }, /* Switch Status 2 */
	{ 0x08, 0x03 }, /* Manual Switch Control 1 */
	{ 0x09, 0x00 }, /* Manual Switch Control 2 */
	{ 0x0A, 0x00 }, /* Key Press */
	{ 0x0B, 0x03 }, /* Double Click Time */
	{ 0x0C, 0x2e }, /* Threshold 1 */
	{ 0x0D, 0x55 }, /* Threshold 2 */
	{ 0x0E, 0x89 }, /* Threshold 3 */
	{ 0x0F, 0x57 }, /* Threshold 4 */
	{ 0x10, 0x7c }, /* ANC Upper Threshold */
	{ 0x32, 0x11 }, /* ANC Key Debounce Threshold */
};

static bool anc_ncx8293_volatile_register(struct device *dev, unsigned int reg)
{
	return true;
}

static bool anc_ncx8293_readable_register(struct device *dev, unsigned int reg)
{
	return true;
}

static inline int anc_ncx8293_get_value(int gpio)
{
	if (gpio >= CODEC_GPIO_BASE)
		return gpio_get_value_cansleep(gpio);

	return gpio_get_value(gpio);
}

static inline void anc_ncx8293_gpio_set_value(int gpio, int value)
{
	if (gpio >= CODEC_GPIO_BASE)
		gpio_set_value_cansleep(gpio, value);
	else
		gpio_set_value(gpio, value);
}

static inline void anc_hs_enable_irq(int irq)
{
	if (!anc_priv->irq_status) {
		enable_irq(irq);
		anc_priv->irq_status = true;
	}
}

static inline void anc_hs_disable_irq(int irq)
{
	if (anc_priv->irq_status) {
		disable_irq_nosync(irq);
		anc_priv->irq_status = false;
	}
}

static void anc_dsm_i2c_reg_fail_report(void)
{
	i2c_reg_fail_time++;
	if ((i2c_reg_fail_time > I2C_REG_FAIL_MAX_TIME) &&
		(i2c_fail_report_times > 0)) {
		i2c_reg_fail_time = 0;
		i2c_fail_report_times--;
#ifdef CONFIG_HUAWEI_DSM
		audio_dsm_report_info(AUDIO_ANC_HS, ANC_HS_I2C_ERR,
			"anc regmap i2c error\n");
#endif
	}
}

static int anc_ncx8293_regmap_read(int reg, int *value)
{
	int ret;

	ret = regmap_read(anc_priv->l_regmap, reg, value);

	if (ret < 0) {
		anc_dsm_i2c_reg_fail_report();
		hwlog_err("anc_ncx8293 regmap read error,%d\n", ret);
	}

	return ret;
}

static int anc_ncx8293_regmap_write(int reg, int value)
{
	int ret;

	ret = regmap_write(anc_priv->l_regmap, reg, value);
	if (ret < 0) {
		anc_dsm_i2c_reg_fail_report();
		hwlog_err("anc_ncx8293 regmap write error,%d\n", ret);
	}

	return ret;
}

static int anc_ncx8293_regmap_update_bits(int reg, int mask, int value)
{
	int ret;

	ret = regmap_update_bits(anc_priv->l_regmap, reg, mask, value);
	if (ret < 0) {
		anc_dsm_i2c_reg_fail_report();
		hwlog_err("anc_ncx8293 regmap update bits error,%d\n", ret);
	}

	return ret;
}

bool anc_ncx8293_check_headset_pluged_in(void)
{
	int value = 0;

	if (!anc_priv)
		return false;

	anc_ncx8293_regmap_read(ANC_NCX8293_R005_ACCESSORY_STATUS, &value);

	if (value & ANC_NCX8293_ATTACH_BIT)
		return true;

	return false;
}

static int force_clear_irq(void)
{
	int value = 0;
	int ret;

	ret = anc_ncx8293_regmap_read(ANC_NCX8293_R002_INTERRUPT, &value);
	if (ret < 0) {
		hwlog_err("anc_ncx8293 %s, read irq reg fail\n", __func__);
		return ret;
	}
	hwlog_info("anc_ncx8293 %s, irq reg : 0x%x\n", __func__, value);

	if ((value & ANC_NCX8293_REMOVAL_IRQ_BIT) &&
		!anc_ncx8293_check_headset_pluged_in()) {
		queue_delayed_work(anc_priv->anc_plugout_delay_wq,
			&anc_priv->anc_plugout_delay_work, 0);
		hwlog_info("anc_ncx8293 %s, plugout event\n", __func__);
	}

	return ret;
}

static void anc_ncx8293_unmask_btn_irq(void)
{
	hwlog_info("anc_ncx8293 unmask btn irq\n");

	force_clear_irq();

	anc_ncx8293_regmap_update_bits(ANC_NCX8293_R001_DEVICE_SETUP,
		ANC_NCX8293_KEY_PRESS_DET_DISABLE_BIT, 0x00);
}

static void anc_ncx8293_mask_btn_irq(void)
{
	hwlog_info("anc_ncx8293 mask btn irq\n");

	anc_ncx8293_regmap_update_bits(ANC_NCX8293_R001_DEVICE_SETUP,
		ANC_NCX8293_KEY_PRESS_DET_DISABLE_BIT,
		ANC_NCX8293_KEY_PRESS_DET_DISABLE_BIT);
}

static void mic_bias_mode(void)
{
	hwlog_info("anc_ncx8293 go into %s\n", __func__);
	if (anc_priv) {
		anc_ncx8293_regmap_write(ANC_NCX8293_R008_MANUAL_SWITCH_CONTROL1,
			ANC_NCX8293_MIC_BIAS_MODE_BIT);
		/* disconnect the hs mic & gnd */
		anc_ncx8293_regmap_write(ANC_NCX8293_R001_DEVICE_SETUP,
			ANC_NCX8293_CTRL_OUTPUT_LOW_BIT);
	}
}

static void power_mode(void)
{
	hwlog_info("anc_ncx8293 go into %s\n", __func__);
	if (anc_priv) {
		/* close mic/pwr to mic_out ,prevent high pulse on mbhc pin */
		anc_ncx8293_regmap_update_bits(
			ANC_NCX8293_R008_MANUAL_SWITCH_CONTROL1,
			ANC_NCX8293_MIC_BIAS_MODE_BIT, 0x00);
		mdelay(CLOSE_MIC_OUT_PATH_TIME);

		anc_ncx8293_regmap_write(ANC_NCX8293_R008_MANUAL_SWITCH_CONTROL1,
			ANC_NCX8293_POWER_MODE_BIT);
		/* disconnect the hs mic & gnd */
		anc_ncx8293_regmap_write(ANC_NCX8293_R001_DEVICE_SETUP,
			ANC_NCX8293_CTRL_OUTPUT_LOW_BIT);
	}
}

static void idle_mode(void)
{
	hwlog_info("anc_ncx8293 go into %s\n", __func__);
	if (anc_priv) {
		/* close mic/pwr & mic_out path,close mic/pwr & 5VCharge path */
		anc_ncx8293_regmap_update_bits(
			ANC_NCX8293_R008_MANUAL_SWITCH_CONTROL1,
			ANC_NCX8293_MIC_BIAS_MODE_BIT, 0x00);
		anc_ncx8293_regmap_update_bits(
			ANC_NCX8293_R008_MANUAL_SWITCH_CONTROL1,
			ANC_NCX8293_POWER_MODE_BIT, 0x00);
		/* connect the hs mic & gnd, to prevent pop when plug in */
		anc_ncx8293_regmap_write(ANC_NCX8293_R001_DEVICE_SETUP,
			ANC_NCX8293_CTRL_OUTPUT_HIGH_BIT);
	}
}

static void anc_ncx8293_discharge(void)
{
	hwlog_info("ncx8293 chip begin discharge\n");
	/*
	 * when change power_mode to mic_bias_mode,
	 * mic/pwr pin switch to GND, to release the residual charge,
	 * to prevent damage the Codec chip
	 */
	anc_ncx8293_regmap_write(ANC_NCX8293_R008_MANUAL_SWITCH_CONTROL1,
		ANC_NCX8293_GND_SL_SWITCH_BIT);
	/*
	 * when change power_mode to mic_bias_mode,
	 * ANC hs has 30ms to supply power outside.
	 * In this period of time, the hs mic is not work.
	 */
	mdelay(ANC_HS_DISCHARGE_TIME);
}

void anc_ncx8293_refresh_headset_type(int headset_type)
{
	if (anc_priv) {
		anc_priv->hs_type = headset_type;
		hwlog_info("ncx8293: refresh hs_type %d",
			anc_priv->hs_type);
	}
}

static void anc_hs_invert_ctl_work(struct work_struct *work)
{
	__pm_stay_awake(&anc_priv->wake_lock);
	mutex_lock(&anc_priv->invert_hs_lock);

	if (anc_priv->hs_type == ANC_HS_REVERT_4POLE) {
		anc_ncx8293_regmap_write(ANC_NCX8293_R001_DEVICE_SETUP,
			ANC_NCX8293_CTRL_OUTPUT_HIGH_BIT);
		hwlog_info("ncx8293: invert headset plugin, connect MIC & GND");
	}

	mutex_unlock(&anc_priv->invert_hs_lock);
	__pm_relax(&anc_priv->wake_lock);
}

void anc_ncx8293_invert_headset_control(int connect)
{
	switch (connect) {
	case ANC_HS_MIC_GND_DISCONNECT:
		cancel_delayed_work(&anc_priv->anc_invert_ctl_delay_work);
		flush_workqueue(anc_priv->anc_invert_ctl_delay_wq);

		mutex_lock(&anc_priv->invert_hs_lock);
		anc_ncx8293_regmap_write(ANC_NCX8293_R001_DEVICE_SETUP,
			ANC_NCX8293_CTRL_OUTPUT_LOW_BIT);
		mutex_unlock(&anc_priv->invert_hs_lock);

		hwlog_info("anc_ncx8293: disconnect MIC and GND\n");
		break;

	case ANC_HS_MIC_GND_CONNECT:
		queue_delayed_work(anc_priv->anc_invert_ctl_delay_wq,
			&anc_priv->anc_invert_ctl_delay_work,
			msecs_to_jiffies(3000));
		hwlog_info("%s:queue delay work", __func__);
		break;

	default:
		hwlog_info("%s:unknown connect type", __func__);
		break;
	}
}

static int anc_ncx8293_get_adc_delta(void)
{
	int ear_pwr_h;
	int ear_pwr_l;
	int delta = 0;
	int count;
	int fail_count = 0;
	int loop = ADC_READ_COUNT;
	int temp;
	bool need_report = true;

	while (loop) {
		loop--;
		mdelay(1);
		ear_pwr_h = hisi_adc_get_value(anc_priv->h_channel_pwl);
		if (ear_pwr_h < 0) {
			hwlog_err("%s:get hkadc(h) fail, err:%d\n", __func__,
				ear_pwr_h);
			fail_count++;
			continue;
		}

		ear_pwr_l = hisi_adc_get_value(anc_priv->l_channel_pwl);
		if (ear_pwr_l < 0) {
			hwlog_err("%s:get hkadc(l) fail, err:%d\n", __func__,
				ear_pwr_l);
			fail_count++;
			continue;
		}
		hwlog_info("%s:adc_h:%d,adc_l:%d\n", __func__, ear_pwr_h,
			ear_pwr_l);

		temp = ear_pwr_h - ear_pwr_l - anc_priv->adc_calibrate_base;

		/* if adc value far away from normal value, just abandon it */
		if ((temp > ADC_NORMAL_LIMIT_MAX) ||
			(temp < ADC_NORMAL_LIMIT_MIN)) {
			fail_count++;
			need_report = false;
			continue;
		}

		delta += temp;
	}

	/* if adc value is out of rage, we make a dmd report */
	count = ADC_READ_COUNT - loop - fail_count;
	if (count == 0) {
		hwlog_err("%s:get anc_hs hkadc fail\n", __func__);
		return false;
	}
	/* compute an average value */
	delta /= count;
	hwlog_info("%s:final adc value= %d count=%d\n", __func__, delta, count);

	return delta;
}

static int anc_ncx8293_get_btn_value(void)
{
	int delta;

	delta = anc_ncx8293_get_adc_delta();
	if ((delta >= anc_priv->anc_btn_hook_min_volt) &&
		(delta <= anc_priv->anc_btn_hook_max_volt))
		return SND_JACK_BTN_0;
	else if ((delta >= anc_priv->anc_btn_volume_up_min_volt) &&
		(delta <= anc_priv->anc_btn_volume_up_max_volt))
		return SND_JACK_BTN_1;
	else if ((delta >= anc_priv->anc_btn_volume_down_min_volt) &&
		(delta <= anc_priv->anc_btn_volume_down_max_volt))
		return SND_JACK_BTN_2;

	hwlog_err("ncx8293 btn delta not in range delta : %d\n", delta);
	return NO_BUTTON_PRESS;

}

static bool anc_ncx8293_btn_press(void)
{
	int value = 0;

	if (!anc_priv)
		return false;

	anc_ncx8293_regmap_read(ANC_NCX8293_R00A_KEY_PRESS, &value);
	if ((value & ANC_NCX8293_KEY1_PRESS) ||
		(value & ANC_NCX8293_KEY2_PRESS) ||
		(value & ANC_NCX8293_KEY3_PRESS))
		return true;
	else
		return false;
}

static void anc_ncx8293_btn_judge(void)
{
	struct anc_hs_dev *pdev = anc_priv->anc_dev;
	struct anc_hs_codec_ops *fops = &pdev->ops;
	int btn_report;

	if (!anc_priv->registered)
		return;

	hwlog_info("%s%u:deal with button irq event\n", __func__, __LINE__);

	/* should get wake lock before codec power lock which may be blocked */
	mutex_lock(&anc_priv->btn_mutex);

	if (anc_ncx8293_btn_press() && (anc_priv->button_pressed == 0)) {
		/* button down event */
		hwlog_info("%s%u:button down event\n", __func__, __LINE__);
		mdelay(MDELAY_TIME);
		btn_report = anc_ncx8293_get_btn_value();
		if (btn_report != NO_BUTTON_PRESS) {
			anc_priv->button_pressed = 1;
			fops->btn_report(btn_report, ANC_BTN_MASK);
		} else {
			hwlog_warn("%s: it is not a button press\n", __func__);
		}
	} else if (anc_priv->button_pressed == 1) {
		/* button up event */
		hwlog_info("%s%u : button up\n", __func__, __LINE__);

		btn_report = 0;
		anc_priv->button_pressed = 0;

		/* we permit button up event report to userspace,
		 * make sure down and up in pair
		 */
		fops->btn_report(btn_report, ANC_BTN_MASK);
	}

	mutex_unlock(&anc_priv->btn_mutex);
}

static int anc_hs_send_hifi_msg(int anc_status)
{
	struct mlib_parameter_voice  *para_anc_hs = NULL;
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

	para_anc_hs = (struct mlib_parameter_voice *)mlib_set_para->auc_data;
	para_anc_hs->key = MLIB_ANC_HS_PARA_ENABLE;
	para_anc_hs->value = anc_status;
#ifdef CONFIG_HIFI_DSP_ONE_TRACK
	ret = hifi_send_msg(MAILBOX_MAILCODE_ACPU_TO_HIFI_MISC, mlib_set_para,
		sizeof(*mlib_set_para) + MLIB_PARA_LENGTH_MAX);
#endif
	kfree(mlib_set_para);

	return ret;
}

static bool anc_ncx8293_need_charge(void)
{
	int delta;

	mdelay(30);
	delta = anc_ncx8293_get_adc_delta();
	if ((delta >= anc_priv->anc_hs_min_limit) &&
		(delta <= anc_priv->anc_hs_max_limit)) {
		hwlog_info("%s %d anc headset = true\n", __func__, __LINE__);
		return true;
	}

	hwlog_info("%s %d anc headset = false\n", __func__, __LINE__);
	return false;
}

static void anc_ncx8293_adc_calibration(void)
{
	int loop = ADC_CALIBRATION_TIMES;
	int count;
	int fail_count = 0;
	int adc_h;
	int adc_l;

	anc_priv->adc_calibrate_base = 0;

	while (loop) {
		loop--;
		usleep_range(1000, 1100);
		adc_h = hisi_adc_get_value(anc_priv->h_channel_pwl);
		if (adc_h < 0) {
			hwlog_err("[anc_hs]get adc fail,adc_h:%d\n", adc_h);
			fail_count++;
			continue;
		}
		adc_l = hisi_adc_get_value(anc_priv->l_channel_pwl);
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
		anc_priv->adc_calibrate_base += (adc_h - adc_l);
	}

	count = ADC_CALIBRATION_TIMES - loop - fail_count;
	if (count == 0) {
		/* if all adc read fail, set 0 to it as default */
		anc_priv->adc_calibrate_base = 0;
		hwlog_err("[anc_hs] calibration whole failed\n");
	} else {
		anc_priv->adc_calibrate_base /= count;
		hwlog_info("anc_hs:calibration_base = %d with %d times\n",
			anc_priv->adc_calibrate_base, count);

		if (anc_priv->adc_calibrate_base > ADC_CALIBRAT_MAX ||
			anc_priv->adc_calibrate_base < ADC_CALIBRAT_MIN) {
			anc_priv->adc_calibrate_base = 0;
			hwlog_err("[anc_hs]calibration value is not illegal\n");
		}
	}
}

static bool anc_ncx8293_charge_judge(void)
{
	struct anc_hs_dev *pdev = anc_priv->anc_dev;
	struct anc_hs_codec_ops *fops = &pdev->ops;

	/* userspace prohibit charging with using hs-mic pin */
	if (anc_priv->force_charge == ANC_HS_DISABLE_CHARGE) {
		hwlog_info("%s%u : charge is occupied by app level\n",
			__func__, __LINE__);
		/* need second detect for charge */
		anc_priv->detect_again = true;
		return false;
	}

	/* hs mic is using record, not take it */
	if (anc_priv->hs_micbias_ctl == ANC_HS_DISABLE_CHARGE) {
		/* need second detect for charge */
		hwlog_info("%s%u :hs mic is using\n", __func__, __LINE__);
		anc_priv->detect_again = true;
		return false;
	}

	anc_priv->detect_again = false;
	hwlog_debug("%s%u : anc hs charge\n", __func__, __LINE__);

	/* headset may have pluged out, just return */
	if (!fops->check_headset_in(anc_priv->priv_data)) {
		hwlog_info("%s%u:headset has plug out\n", __func__, __LINE__);
		return false;
	}

	mutex_lock(&anc_priv->charge_lock);
	/* connect 5vboost with hs_mic pin */
	power_mode();
	mutex_unlock(&anc_priv->charge_lock);

	/* waiting for anc chip start up */
	hwlog_info("%s: delay %d ms wait anc chip up\n", __func__,
		anc_priv->sleep_time);
	mdelay(anc_priv->sleep_time);

	mutex_lock(&anc_priv->charge_lock);

	if ((anc_priv->hs_micbias_ctl == ANC_HS_ENABLE_CHARGE) &&
		anc_ncx8293_need_charge()) {
		/* start charge for anc hs and respond charging btn event */
		if (anc_priv->anc_charge == ANC_HS_CHARGE_OFF) {
			hwlog_info("%s%u : anc_hs enable irq\n", __func__,
				__LINE__);
			anc_priv->anc_charge = ANC_HS_CHARGE_ON;
			anc_ncx8293_unmask_btn_irq();
		}

		if (anc_hs_send_hifi_msg(ANC_HS_CHARGE_ON) == ERROR_RET)
			hwlog_err("%s%u:anc_send_hifi_msg TURN ON ANC fail\n",
				__func__, __LINE__);
	} else {
		if (anc_priv->anc_charge == ANC_HS_CHARGE_ON)
			hwlog_info("%s : anc_hs disable irq\n", __func__);
		/* stop charge and change status to CHARGE_OFF */
		anc_ncx8293_mask_btn_irq();
		anc_ncx8293_discharge();
		mic_bias_mode();
		udelay(UDELAY_TIME);
		anc_priv->anc_charge = ANC_HS_CHARGE_OFF;
		if (anc_hs_send_hifi_msg(ANC_HS_CHARGE_OFF) == ERROR_RET)
			hwlog_err("%s%u:anc_send_hifi_msg TURN OFF ANC fail\n",
				__func__, __LINE__);
	}

	mutex_unlock(&anc_priv->charge_lock);

	if (anc_priv->anc_charge == ANC_HS_CHARGE_ON)
		return true;

	return false;
}

static void update_charge_status(void)
{
	struct anc_hs_dev *pdev = anc_priv->anc_dev;
	struct anc_hs_codec_ops *fops = &pdev->ops;

	if (anc_priv->hs_micbias_ctl == ANC_HS_DISABLE_CHARGE ||
		anc_priv->force_charge == ANC_HS_DISABLE_CHARGE) {
		/* force stop charge function */
		mutex_lock(&anc_priv->charge_lock);

		if (anc_priv->anc_charge == ANC_HS_CHARGE_ON) {
			anc_ncx8293_mask_btn_irq();
			anc_ncx8293_discharge();
			mic_bias_mode();
			udelay(UDELAY_TIME);

			hwlog_info("%s%u : stop charging for anc hs\n",
				__func__, __LINE__);
			anc_priv->anc_charge = ANC_HS_CHARGE_OFF;
			anc_priv->mic_status = true;
		}

		mutex_unlock(&anc_priv->charge_lock);
	} else if (anc_priv->hs_micbias_ctl == ANC_HS_ENABLE_CHARGE &&
		anc_priv->force_charge == ANC_HS_ENABLE_CHARGE) {
		if (anc_priv->mic_status) {
			anc_priv->mic_status = false;
			/* headset maybe have plug out here */
			if (!fops->check_headset_in(anc_priv->priv_data)) {
				hwlog_info("%s%u :headset has plug out\n",
					__func__, __LINE__);
			} else {
				/* force resume charge for anc headset */
				mutex_lock(&anc_priv->charge_lock);

				if (anc_priv->anc_charge ==
					ANC_HS_CHARGE_OFF) {
					anc_priv->anc_charge =
						ANC_HS_CHARGE_ON;
					power_mode();
					udelay(UDELAY_TIME);
					anc_ncx8293_unmask_btn_irq();
					hwlog_info("%s%u:resume charge Anc\n",
						__func__, __LINE__);
				}

				mutex_unlock(&anc_priv->charge_lock);
			}
		} else if (anc_priv->detect_again) {
			/* need detect charge again due to interrupted before */
			anc_ncx8293_charge_judge();
		}
	}
}

void anc_ncx8293_start_charge(void)
{
	if (!anc_priv || !anc_priv->registered)
		return;

	hwlog_info("%s%u : enable 5v boost\n", __func__, __LINE__);

	/* default let hsbias connect to hs-mic pin */
	mic_bias_mode();
	/* enable 5vboost first, this need time to be stable */
	anc_ncx8293_gpio_set_value(anc_priv->gpio_pwr_en, 1);
	anc_priv->anc_charge = ANC_HS_CHARGE_OFF;
}

void anc_ncx8293_force_charge(int disable)
{
	if (!anc_priv || !anc_priv->registered)
		return;

	/* don't make repeated switch with charge status */
	if (anc_priv->hs_micbias_ctl == disable)
		return;

	anc_priv->hs_micbias_ctl = disable;

	/* update charge status here */
	update_charge_status();
}

bool anc_ncx8293_charge_detect(int saradc_value, int headset_type)
{
	if (!anc_priv || !anc_priv->registered)
		return false;

	/* calibration adc resistance which can make charge detect accuracy */
	anc_ncx8293_adc_calibration();

	anc_priv->hs_type = headset_type;

	/* revert 4-pole headset need 5vboost on for second recognition */
	if (headset_type == ANC_HS_NORMAL_4POLE) {
		/* 4-pole headset maybe an anc headset */
		hwlog_debug("%s : start anc hs charge judge\n", __func__);
		return anc_ncx8293_charge_judge();
	}

	if (headset_type == ANC_HS_NORMAL_3POLE) {
		hwlog_info("%s : no disable 5vboost for 3-pole headset\n",
			__func__);
		/* 3-pole also support second-detect */
		return false;
	}

	return false;
}

void anc_ncx8293_stop_charge(void)
{
	if (!anc_priv || !anc_priv->registered)
		return;

	hwlog_info("%s : stop anc hs charge\n", __func__);

	anc_ncx8293_mask_btn_irq();
	idle_mode();
	anc_ncx8293_gpio_set_value(anc_priv->gpio_pwr_en, 0);
	anc_priv->anc_charge = ANC_HS_CHARGE_OFF;
	anc_priv->hs_type = ANC_HS_NONE;
	anc_priv->button_pressed = 0;

	if (anc_hs_send_hifi_msg(ANC_HS_CHARGE_OFF) == ERROR_RET)
		hwlog_err("%s%u:anc_hs_send_hifi_msg TURN OFF ANC_HS fail\n",
			__func__, __LINE__);
}

int anc_ncx8293_dev_register(struct anc_hs_dev *dev, void *codec_data)
{
	/* anc_hs driver not be probed, just return */
	if (!anc_priv)
		return -ENODEV;

	/* only support one codec to be registered */
	if (anc_priv->registered) {
		hwlog_err("one codec registered, no more permit\n");
		return -EEXIST;
	}

	if (!dev->ops.check_headset_in || !dev->ops.btn_report ||
		!dev->ops.codec_resume_lock || !dev->ops.plug_in_detect ||
		!dev->ops.plug_out_detect) {
		hwlog_err("codec ops function must be all registed\n");
		return -EINVAL;
	}

	anc_priv->anc_dev = dev;
	anc_priv->priv_data = codec_data;
	anc_priv->registered = true;

	force_clear_irq();
	hwlog_info("%s%u : anc hs has been register sucessful\n",
		__func__, __LINE__);

	return 0;
}

bool check_anc_ncx8293_support(void)
{
	if (!anc_priv || !anc_priv->registered)
		return false;

	return true;
}

bool anc_ncx8293_plug_enable(void)
{
	if (!anc_priv || !anc_priv->registered)
		return false;

	return true;
}

static int get_irq_type(void)
{
	int irq_type = ANC_NCX8293_IQR_MAX;
	int value = 0;

	anc_ncx8293_regmap_read(ANC_NCX8293_R002_INTERRUPT, &value);

	if ((value & ANC_NCX8293_INSERTION_IRQ_BIT) &&
		anc_ncx8293_check_headset_pluged_in())
		irq_type = ANC_NCX8293_JACK_PLUG_IN;
	else if ((value & ANC_NCX8293_REMOVAL_IRQ_BIT) &&
		!anc_ncx8293_check_headset_pluged_in())
		irq_type = ANC_NCX8293_JACK_PLUG_OUT;
	else if (value & ANC_NCX8293_KEY_PRESS_IRQ_BIT)
		irq_type = ANC_NCX8293_BUTTON_PRESS;

	hwlog_info("anc_ncx8293 irq value is 0x%x, type is %d\n",
		value, irq_type);

	return irq_type;
}

static void anc_hs_plugin_work(struct work_struct *work)
{
	struct anc_hs_codec_ops *fops = &anc_priv->anc_dev->ops;

	__pm_stay_awake(&anc_priv->wake_lock);

	if (anc_priv->priv_data)
		fops->plug_in_detect(anc_priv->priv_data);

	__pm_relax(&anc_priv->wake_lock);
}

static void anc_hs_plugout_work(struct work_struct *work)
{
	struct anc_hs_codec_ops *fops = &anc_priv->anc_dev->ops;

	__pm_stay_awake(&anc_priv->wake_lock);

	if (anc_priv->priv_data)
		fops->plug_out_detect(anc_priv->priv_data);

	__pm_relax(&anc_priv->wake_lock);
}

static void anc_hs_btn_work(struct work_struct *work)
{
	__pm_stay_awake(&anc_priv->wake_lock);
	anc_ncx8293_btn_judge();
	__pm_relax(&anc_priv->wake_lock);
}

static irqreturn_t anc_ncx8293_irq_handler(int irq, void *data)
{
	int irq_type;
	int value = 0;
	int loop = 5; /* repeat 5 times */

	if (!anc_priv || !anc_priv->registered) {
		pr_info("anc_ncx8293 dev has not been regestered\n");
		anc_ncx8293_regmap_read(ANC_NCX8293_R002_INTERRUPT, &value);
		return IRQ_HANDLED;
	}

	__pm_wakeup_event(&anc_priv->wake_lock, 1000);
	irq_type = get_irq_type();

	switch (irq_type) {
	case ANC_NCX8293_JACK_PLUG_IN:
		queue_delayed_work(anc_priv->anc_plugin_delay_wq,
			&anc_priv->anc_plugin_delay_work,
			msecs_to_jiffies(800));
		break;
	case ANC_NCX8293_JACK_PLUG_OUT:
		queue_delayed_work(anc_priv->anc_plugout_delay_wq,
			&anc_priv->anc_plugout_delay_work, 0);
		break;
	case ANC_NCX8293_BUTTON_PRESS:
		queue_delayed_work(anc_priv->anc_btn_delay_wq,
			&anc_priv->anc_btn_delay_work, msecs_to_jiffies(20));
		break;
	default:
		break;
	}

	while (loop > 0) {
		irq_type = get_irq_type();
		if (irq_type == ANC_NCX8293_IQR_MAX)
			break;

		if (irq_type == ANC_NCX8293_JACK_PLUG_OUT)
			queue_delayed_work(anc_priv->anc_plugout_delay_wq,
				&anc_priv->anc_plugout_delay_work, 0);

		loop = loop - 1;
	}

#ifdef CONFIG_HUAWEI_DSM
	if (loop <= 0)
		audio_dsm_report_info(AUDIO_ANC_HS, ANC_HS_UNHANDLED_IRQ,
			"there is irq unhandled in anc_max14744_irq_handler\n");
#endif

	return IRQ_HANDLED;
}

static int compute_final_voltage(void)
{
	int r1 = 51; /* low impedance value */
	int r2 = 100; /* high impedance value */
	int adc_volt_low;
	int adc_volt_high;

	msleep(70);
	adc_volt_low = hisi_adc_get_value(anc_priv->l_channel_pwl);
	adc_volt_high = hisi_adc_get_value(anc_priv->h_channel_pwl);

	hwlog_info("adc_l value: %d adc_h value: %d\n", adc_volt_low, adc_volt_high);

	return (adc_volt_low * (r1 + r2)) / r1;
}

static long anc_ncx8293_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	int ret = 0;
	int charge_mode = ANC_HS_CHARGE_OFF;
	int voltage;
	unsigned int __user *p_user = (unsigned int __user *)arg;
	int resistance_type = 0;

	if (!p_user)
		return -EBUSY;

	if (!anc_priv->registered)
		return -EBUSY;

	switch (cmd) {
	case IOCTL_ANC_HS_CHARGE_ENABLE_CMD:
		if (anc_priv->force_charge == ANC_HS_ENABLE_CHARGE)
			break;

		/* resume anc headset charge */
		anc_priv->force_charge = ANC_HS_ENABLE_CHARGE;
		hwlog_info("app level contrl set charge status %d\n",
			anc_priv->force_charge);
		update_charge_status();
		break;
	case IOCTL_ANC_HS_CHARGE_DISABLE_CMD:
		if (anc_priv->force_charge == ANC_HS_DISABLE_CHARGE)
			break;

		/* force stop anc headset charge */
		anc_priv->force_charge = ANC_HS_DISABLE_CHARGE;
		hwlog_info("app level contrl set charge status %d\n",
			anc_priv->force_charge);
		update_charge_status();
		break;
	case IOCTL_ANC_HS_GET_HEADSET_CMD:
		charge_mode = anc_priv->anc_charge;
		if (charge_mode == ANC_HS_CHARGE_ON) {
			if (!anc_ncx8293_need_charge()) {
				charge_mode = ANC_HS_CHARGE_OFF;
			} else {
				usleep_range(10000, 10100);
				if (!anc_ncx8293_need_charge())
					charge_mode = ANC_HS_CHARGE_OFF;
			}
		}
		if (charge_mode == ANC_HS_CHARGE_ON)
			anc_priv->hs_type = ANC_HS_HEADSET;

		ret = put_user((__u32)(anc_priv->hs_type),
			p_user);
		break;
	case IOCTL_ANC_HS_GET_CHARGE_STATUS_CMD:
		ret = put_user((__u32)(anc_priv->anc_charge),
			p_user);
		break;
	case IOCTL_ANC_HS_GET_VBST_5VOLTAGE_CMD:
		mic_bias_mode();
		anc_ncx8293_gpio_set_value(anc_priv->gpio_pwr_en, 1);
		voltage = compute_final_voltage();
		anc_ncx8293_gpio_set_value(anc_priv->gpio_pwr_en, 0);
		ret = put_user((__u32)voltage, p_user);
		break;
	case IOCTL_ANC_HS_GET_VDD_BUCK_VOLTAGE_CMD:
		mic_bias_mode();
		anc_ncx8293_gpio_set_value(anc_priv->gpio_pwr_en, 0);
		voltage = compute_final_voltage();
		ret = put_user((__u32)voltage, p_user);
		break;
	case IOCTL_ANC_HS_GET_HEADSET_RESISTANCE_CMD:
		ret = put_user((__u32)resistance_type, p_user);
		break;
	default:
		hwlog_err("unsupport cmd\n");
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}
#ifdef ANC_NCX8293_DEBUG

static ssize_t anc_ncx8293_reg_list_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int value = 0;
	int reg;
	char val_str[VAL_STR_SIZE];

	if (!buf) {
		hwlog_info("%s : buf is null\n", __func__);
		return -EINVAL;
	}
	buf[0] = '\0';
	for (reg = 0; reg <= ANC_REGISTER_MAX_ADDRESS; reg++) {
		anc_ncx8293_regmap_read(reg, &value);
		snprintf(val_str, sizeof(val_str), "0x%02x = 0x%02x\n", reg,
			value);

		strcat(buf, val_str);
	}
	return strlen(buf);
}

static ssize_t anc_ncx8293_adc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char val_str[VAL_STR_SIZE];
	int ear_pwr_h;
	int ear_pwr_l;

	if (!buf) {
		hwlog_err("buf is null\n");
		return -EINVAL;
	}
	buf[0] = '\0';

	ear_pwr_h = hisi_adc_get_value(anc_priv->h_channel_pwl);
	mdelay(MDELAY_TIME);
	ear_pwr_l = hisi_adc_get_value(anc_priv->l_channel_pwl);
	mdelay(MDELAY_TIME);
	snprintf(val_str, sizeof(val_str), "h = %d, l = %d\n", ear_pwr_h,
		ear_pwr_l);
	strcat(buf, val_str);

	return strlen(buf);
}

static DEVICE_ATTR(reg_list, 0664, anc_ncx8293_reg_list_show, NULL);
static DEVICE_ATTR(adc, 0664, anc_ncx8293_adc_show, NULL);

static struct attribute *anc_ncx8293_attributes[] = {
	&dev_attr_reg_list.attr,
	&dev_attr_adc.attr,
	NULL
};

static const struct attribute_group anc_ncx8293_attr_group = {
	.attrs = anc_ncx8293_attributes,
};
#endif

static const struct regmap_config anc_ncx8293_regmap = {
	.reg_bits            = 8,
	.val_bits            = 8,
	.max_register        = ANC_NCX8293_R032_ANC_KEY_DEBOUNCE_THRESHOLD,
	.reg_defaults        = anc_ncx8293_reg,
	.num_reg_defaults    = ARRAY_SIZE(anc_ncx8293_reg),
	.volatile_reg        = anc_ncx8293_volatile_register,
	.readable_reg        = anc_ncx8293_readable_register,
	.cache_type          = REGCACHE_RBTREE,
};

static const struct file_operations anc_ncx8293_fops = {
	.owner            = THIS_MODULE,
	.open             = simple_open,
	.unlocked_ioctl   = anc_ncx8293_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl     = anc_ncx8293_ioctl,
#endif
};

static struct miscdevice anc_ncx8293_device = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = "anc_hs",
	.fops   = &anc_ncx8293_fops,
};

static void read_adc_channel_number(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "adc_channel_h", &temp))
		anc_priv->h_channel_pwl = temp;
	else
		anc_priv->h_channel_pwl = ANC_ADC_CHANNEL_H_DEFAULT;
	if (!of_property_read_u32(node, "adc_channel_l", &temp))
		anc_priv->l_channel_pwl = temp;
	else
		anc_priv->l_channel_pwl = ANC_ADC_CHANNEL_L_DEFAULT;
}

static void read_charge_limit(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "anc_hs_limit_min", &temp))
		anc_priv->anc_hs_min_limit = temp;
	else
		anc_priv->anc_hs_min_limit = ANC_HS_LIMIT_MIN;
	if (!of_property_read_u32(node, "anc_hs_limit_max", &temp))
		anc_priv->anc_hs_max_limit = temp;
	else
		anc_priv->anc_hs_max_limit = ANC_HS_LIMIT_MAX;
}

static void read_hook_limit(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "anc_hs_btn_hook_min_voltage", &temp))
		anc_priv->anc_btn_hook_min_volt = temp;
	else
		anc_priv->anc_btn_hook_min_volt = ANC_HS_HOOK_MIN;
	if (!of_property_read_u32(node, "anc_hs_btn_hook_max_voltage", &temp))
		anc_priv->anc_btn_hook_max_volt = temp;
	else
		anc_priv->anc_btn_hook_max_volt = ANC_HS_HOOK_MAX;
}

static void read_volume_up_limit(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "anc_hs_btn_volume_up_min_voltage",
		&temp))
		anc_priv->anc_btn_volume_up_min_volt = temp;
	else
		anc_priv->anc_btn_volume_up_min_volt =
			ANC_HS_VOLUME_UP_MIN;
	if (!of_property_read_u32(node, "anc_hs_btn_volume_up_max_voltage",
		&temp))
		anc_priv->anc_btn_volume_up_max_volt = temp;
	else
		anc_priv->anc_btn_volume_up_max_volt =
			ANC_HS_VOLUME_UP_MAX;
}

static void read_volume_down_limit(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "anc_hs_btn_volume_down_min_voltage",
		&temp))
		anc_priv->anc_btn_volume_down_min_volt = temp;
	else
		anc_priv->anc_btn_volume_down_min_volt =
			ANC_HS_VOLUME_DOWN_MIN;
	if (!of_property_read_u32(node, "anc_hs_btn_volume_down_max_voltage",
		&temp))
		anc_priv->anc_btn_volume_down_max_volt = temp;
	else
		anc_priv->anc_btn_volume_down_max_volt =
			ANC_HS_VOLUME_DOWN_MAX;
}

/* load dts config for board difference */
static void load_anc_hs_config(struct device_node *node)
{
	read_adc_channel_number(node);

	read_charge_limit(node);

	read_hook_limit(node);

	read_volume_up_limit(node);

	read_volume_down_limit(node);
}

static struct anc_hs_ops anc_ncx8293_ops = {
	.anc_hs_dev_register = anc_ncx8293_dev_register,
	.anc_hs_check_headset_pluged_in = anc_ncx8293_check_headset_pluged_in,
	.anc_hs_start_charge = anc_ncx8293_start_charge,
	.anc_hs_charge_detect = anc_ncx8293_charge_detect,
	.anc_hs_stop_charge = anc_ncx8293_stop_charge,
	.anc_hs_force_charge = anc_ncx8293_force_charge,
	.check_anc_hs_support = check_anc_ncx8293_support,
	.anc_hs_plug_enable = anc_ncx8293_plug_enable,
	.anc_hs_5v_control = NULL,
	.anc_hs_invert_hs_control = anc_ncx8293_invert_headset_control,
	.anc_hs_refresh_headset_type = anc_ncx8293_refresh_headset_type,
};

static bool read_property_value(struct device *dev,
	const char *property_name)
{
	int ret;
	int value = 0;
	bool pro_val = true;

	ret = of_property_read_u32(dev->of_node, property_name, &value);
	if (ret) {
		hwlog_info("%s:can't get %s, set defaut value\n",
			__func__, property_name);
		pro_val = false;
	} else {
		if (value) {
			pro_val = true;
			hwlog_info("%s:%s is true\n", __func__, property_name);
		} else {
			pro_val = false;
			hwlog_info("%s:%s is false\n", __func__, property_name);
		}
	}

	return pro_val;
}

static int work_voltage_supply(struct anc_ncx8293_priv *di,
	struct device_node *np)
{
	int ret;
	const char *ldo_supply_used_name = "ldo_supply_used";
	const char *anc_hs_vdd_name = "anc_hs_vdd";
	const char *cam_ldo_used_name = "cam_ldo_used";

	di->ldo_supply_used = read_property_value(di->dev, ldo_supply_used_name);
	di->cam_ldo_used = read_property_value(di->dev, cam_ldo_used_name);

	if (di->ldo_supply_used) {
		di->anc_hs_vdd = devm_regulator_get(di->dev, anc_hs_vdd_name);
		if (IS_ERR(di->anc_hs_vdd)) {
			hwlog_err("%s: can't get anc_hs_vdd regulator %pK\n",
				__func__, di->anc_hs_vdd);
			di->anc_hs_vdd = NULL;
			goto err_out;
		}

		ret = regulator_set_voltage(di->anc_hs_vdd, ANC_HS_VDD, ANC_HS_VDD);
		if (ret) {
			hwlog_err("%s: can't set anc_hs_vdd 3.3 voltage\n",
				__func__);
			goto err_out;
		}

		ret = regulator_enable(di->anc_hs_vdd);
		if (ret) {
			hwlog_err("%s:can't enable anc_hs_vdd ldo\n", __func__);
			goto err_out;
		}
	} else if (di->cam_ldo_used) {
		ret = of_property_read_u32(np, "cam_ldo_num", &di->cam_ldo_num);
		if (ret) {
			di->cam_ldo_num = -EINVAL;
			hwlog_err("%s fail to get cam_ldo_num from dev tree\n",
				__func__);
			goto err_out;
		}

		ret = of_property_read_u32(np, "cam_ldo_value",
			&di->cam_ldo_val);
		if (ret) {
			di->cam_ldo_val = -EINVAL;

			hwlog_err("%s fail get cam_ldo_value from dev tree\n",
				__func__);
			goto err_out;
		}

		ret = hw_extern_pmic_config(di->cam_ldo_num, di->cam_ldo_val,
			1);
		if (ret) {
			hwlog_err("%s camera pmic LDO_%d power on fail\n",
				__func__, di->cam_ldo_num + 1);
			goto err_out;
		}
	} else {
		hwlog_info("%s: ldo_supply_used and cam_ldo_used both fail\n",
			__func__);
	}

	return 0;

err_out:
	return -1;
}

static int create_irq_workqueue(struct i2c_client *client,
	struct anc_ncx8293_priv *di, struct device_node *np)
{
	int ret;
#ifdef ANC_NCX8293_DEBUG
	ret = sysfs_create_group(&client->dev.kobj, &anc_ncx8293_attr_group);
	if (ret < 0)
		hwlog_err("failed to register sysfs\n");
#endif

	di->anc_plugin_delay_wq =
		create_singlethread_workqueue("anc_plugin_delay_wq");
	if (!(di->anc_plugin_delay_wq)) {
		hwlog_err("%s : plugin wq create failed\n", __func__);
		ret = -ENOMEM;
		goto err_out_sysfs;
	}
	INIT_DELAYED_WORK(&di->anc_plugin_delay_work, anc_hs_plugin_work);

	di->anc_plugout_delay_wq =
		create_singlethread_workqueue("anc_plugout_delay_wq");
	if (!(di->anc_plugout_delay_wq)) {
		hwlog_err("%s : plugout wq create failed\n", __func__);
		ret = -ENOMEM;
		goto err_plugin_delay_wq;
	}
	INIT_DELAYED_WORK(&di->anc_plugout_delay_work, anc_hs_plugout_work);

	di->anc_btn_delay_wq =
		create_singlethread_workqueue("anc_btn_delay_wq");
	if (!(di->anc_btn_delay_wq)) {
		hwlog_err("%s : btn wq create failed\n", __func__);
		ret = -ENOMEM;
		goto err_plugout_delay_wq;
	}
	INIT_DELAYED_WORK(&di->anc_btn_delay_work, anc_hs_btn_work);

	di->anc_invert_ctl_delay_wq =
		create_singlethread_workqueue("anc_invert_ctl_delay_wq");
	if (!(di->anc_invert_ctl_delay_wq)) {
		hwlog_err("%s : invert_ctl wq create failed\n", __func__);
		ret = -ENOMEM;
		goto err_btn_delay_wq;
	}
	INIT_DELAYED_WORK(&di->anc_invert_ctl_delay_work,
		anc_hs_invert_ctl_work);

	return ret;

err_btn_delay_wq:
	if (di->anc_btn_delay_wq) {
		cancel_delayed_work(&di->anc_btn_delay_work);
		flush_workqueue(di->anc_btn_delay_wq);
		destroy_workqueue(di->anc_btn_delay_wq);
	}

err_plugout_delay_wq:
	if (di->anc_plugout_delay_wq) {
		cancel_delayed_work(&di->anc_plugout_delay_work);
		flush_workqueue(di->anc_plugout_delay_wq);
		destroy_workqueue(di->anc_plugout_delay_wq);
	}

err_plugin_delay_wq:
	if (di->anc_plugin_delay_wq) {
		cancel_delayed_work(&di->anc_plugin_delay_work);
		flush_workqueue(di->anc_plugin_delay_wq);
		destroy_workqueue(di->anc_plugin_delay_wq);
	}

err_out_sysfs:
#ifdef ANC_NCX8293_DEBUG
	sysfs_remove_group(&client->dev.kobj, &anc_ncx8293_attr_group);
#endif
	return ret;
}

static void anc_hs_remove_workqueue(struct anc_ncx8293_priv *di)
{
	if (di->anc_invert_ctl_delay_wq) {
		cancel_delayed_work(&di->anc_invert_ctl_delay_work);
		flush_workqueue(di->anc_invert_ctl_delay_wq);
		destroy_workqueue(di->anc_invert_ctl_delay_wq);
	}

	if (di->anc_btn_delay_wq) {
		cancel_delayed_work(&di->anc_btn_delay_work);
		flush_workqueue(di->anc_btn_delay_wq);
		destroy_workqueue(di->anc_btn_delay_wq);
	}

	if (di->anc_plugout_delay_wq) {
		cancel_delayed_work(&di->anc_plugout_delay_work);
		flush_workqueue(di->anc_plugout_delay_wq);
		destroy_workqueue(di->anc_plugout_delay_wq);
	}

	if (di->anc_plugin_delay_wq) {
		cancel_delayed_work(&di->anc_plugin_delay_work);
		flush_workqueue(di->anc_plugin_delay_wq);
		destroy_workqueue(di->anc_plugin_delay_wq);
	}
}

static int chip_init(struct i2c_client *client, struct anc_ncx8293_priv *di,
	struct device_node *np)
{
	int ret;
	unsigned long flag = IRQF_ONESHOT | IRQF_NO_SUSPEND;

	di->gpio_pwr_en = of_get_named_gpio(np, "gpio_pwr_en", 0);
	if (di->gpio_pwr_en < 0) {
		hwlog_err("gpio_pwr_en is invalid\n");
		goto err_invert_ctl_delay_wq;
	}

	ret = gpio_request(di->gpio_pwr_en, "anc_ncx8293_pwr_en");
	if (ret) {
		hwlog_err("error request GPIO for gpio_pwr_en fail %d\n",
			ret);
		goto err_invert_ctl_delay_wq;
	}

	gpio_direction_output(di->gpio_pwr_en, 0);

	di->gpio_int = of_get_named_gpio(np, "gpio_int", 0);
	if (di->gpio_int < 0) {
		hwlog_err("gpio_int is invalid\n");
		goto err_gpio_pwr_en;
	}

	ret = gpio_request(di->gpio_int, "anc_ncx8293_int");
	if (ret) {
		hwlog_err("error request GPIO for gpio_int fail %d\n", ret);
		goto err_gpio_pwr_en;
	}

	/* set irq gpio to input status */
	gpio_direction_input(di->gpio_int);
	di->anc_ncx8293_irq = gpio_to_irq(di->gpio_int);

	/* anc ncx8293 irq request */
	flag |= IRQF_TRIGGER_FALLING;
	ret = request_threaded_irq(di->anc_ncx8293_irq, NULL,
		anc_ncx8293_irq_handler, flag, "anc_ncx8293_irq", NULL);

	if (ret < 0) {
		hwlog_err("anc_ncx8293_irq request fail: ret = %d\n", ret);
		goto err_out_gpio;
	}

	pr_info("anc_ncx8293 gpio_int:%d, gpio_pwr_en:%d\n", di->gpio_int,
		di->gpio_pwr_en);

	ret = anc_hs_ops_register(&anc_ncx8293_ops);
	if (ret) {
		hwlog_err("register anc_hs_interface ops failed\n");
		goto err_out_irq;
	}

	/*
	 * Change the key debounce time from 60ms(default) to 30ms,
	 * to ensure the MMT-equipment test pass.
	 * The MMT-equipment test ANC key, from key release to next key press,
	 * has 55ms latency time.
	 */
	ret = anc_ncx8293_regmap_write(
		ANC_NCX8293_R032_ANC_KEY_DEBOUNCE_THRESHOLD,
		ANC_NCX8293_30MS_KEY_DEBOUNCE_TIME);
	if (ret < 0)
		hwlog_err("%s:modify key debounce time fail, MMT-equip fail\n",
			__func__);

	idle_mode();

	/* register misc device for userspace */
	ret = misc_register(&anc_ncx8293_device);
	if (ret) {
		hwlog_err("%s: anc_ncx8293 misc device register failed",
			__func__);
		goto err_out_irq;
	}
	return ret;

err_out_irq:
	free_irq(di->anc_ncx8293_irq, NULL);

err_out_gpio:
	gpio_free(di->gpio_int);

err_gpio_pwr_en:
	gpio_free(di->gpio_pwr_en);

err_invert_ctl_delay_wq:
	anc_hs_remove_workqueue(di);

#ifdef ANC_NCX8293_DEBUG
	sysfs_remove_group(&client->dev.kobj, &anc_ncx8293_attr_group);
#endif

	return ret;
}

static int anc_hs_regmap_init(struct i2c_client *client)
{
	int ret;
	int val = 0;

	anc_priv->l_regmap = regmap_init_i2c(client, &anc_ncx8293_regmap);
	if (IS_ERR(anc_priv->l_regmap)) {
		ret = PTR_ERR(anc_priv->l_regmap);
		hwlog_err("Failed to allocate l_regmap: %d\n", ret);
		return ret;
	}

	ret = anc_ncx8293_regmap_read(ANC_NCX8293_R000_DEVICE_ID, &val);
	if (ret < 0) {
		hwlog_err("ncx8293 chip is not exist, stop the chip init\n");
		ret = -ENXIO;
	}

	return ret;
}

static void para_init(void)
{
	anc_priv->anc_charge = ANC_HS_CHARGE_OFF;
	anc_priv->irq_status = true;
	anc_priv->mic_status = false;
	anc_priv->sleep_time = ANC_CHIP_STARTUP_TIME;
	anc_priv->adc_calibrate_base = 0;
	anc_priv->hs_micbias_ctl = ANC_HS_ENABLE_CHARGE;
	anc_priv->force_charge = ANC_HS_ENABLE_CHARGE;
	anc_priv->boost_status = false;
	anc_priv->registered = false;
	anc_priv->hs_type = ANC_HS_NONE;
}

static int dev_pinctrl(struct device_node *np)
{
	int ret;
	struct pinctrl *p = NULL;
	struct pinctrl_state *pinctrl_def = NULL;

	p = devm_pinctrl_get(anc_priv->dev);
	hwlog_info("ncx8293:node name is %s\n", np->name);
	if (IS_ERR(p)) {
		hwlog_err("could not get pinctrl dev\n");
		return -ENOMEM;
	}
	pinctrl_def = pinctrl_lookup_state(p, "default");
	if (IS_ERR(pinctrl_def))
		hwlog_err("could not get defstate\n");

	ret = pinctrl_select_state(p, pinctrl_def);
	if (ret)
		hwlog_err("could not set pins to default state\n");

	return 0;
}

static void lock_init(void)
{
	mutex_init(&anc_priv->charge_lock);
	mutex_init(&anc_priv->btn_mutex);
	mutex_init(&anc_priv->invert_hs_lock);
	spin_lock_init(&anc_priv->irq_lock);
	wakeup_source_init(&anc_priv->wake_lock, "anc_ncx8293");
}

static int anc_ncx8293_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	int val = 0;
	const char *chip_powered_on_time = "chip_powered_on_time";
	struct anc_ncx8293_priv *di = NULL;
	struct device_node *np = NULL;

	hwlog_info("%s++\n", __func__);
	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	anc_priv = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;

	ret = work_voltage_supply(di, np);
	if (ret < 0)
		goto err_out;

	ret = of_property_read_u32(np, chip_powered_on_time, &val);
	if (ret) {
		hwlog_err("%s: fail to get chip_powered_on_time\n", __func__);
		val = CHIP_DEFAULT_POWERED_TIME;
	}
	mdelay(val);

	i2c_set_clientdata(client, di);

	ret = anc_hs_regmap_init(client);
	if (ret)
		goto err_out;

	lock_init();
	para_init();
	load_anc_hs_config(np);

	ret = dev_pinctrl(np);
	if (ret)
		goto err_out_init;

	ret = create_irq_workqueue(client, di, np);
	if (ret < 0)
		goto err_out_init;

	ret = chip_init(client, di, np);
	if (ret < 0)
		goto err_out_init;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_ANC_MAX14744);
#endif

	hwlog_info("%s--\n", __func__);
	return 0;

err_out_init:
	wakeup_source_trash(&di->wake_lock);
err_out:
	if (ret < 0) {
		if (di->l_regmap)
			regmap_exit(di->l_regmap);
	}
	devm_kfree(&client->dev, anc_priv);
	anc_priv = NULL;

	return ret;
}

static int anc_ncx8293_remove(struct i2c_client *client)
{
	struct anc_ncx8293_priv *di = i2c_get_clientdata(client);
	int ret = -1;

	if (!di)
		return 0;

	if (di && di->l_regmap)
		regmap_exit(di->l_regmap);

	if (di->anc_plugin_delay_wq) {
		cancel_delayed_work(&di->anc_plugin_delay_work);
		flush_workqueue(di->anc_plugin_delay_wq);
		destroy_workqueue(di->anc_plugin_delay_wq);
	}

	if (di->anc_plugout_delay_wq) {
		cancel_delayed_work(&di->anc_plugout_delay_work);
		flush_workqueue(di->anc_plugout_delay_wq);
		destroy_workqueue(di->anc_plugout_delay_wq);
	}

	if (di->anc_btn_delay_wq) {
		cancel_delayed_work(&di->anc_btn_delay_work);
		flush_workqueue(di->anc_btn_delay_wq);
		destroy_workqueue(di->anc_btn_delay_wq);
	}

	if (di->anc_invert_ctl_delay_wq) {
		cancel_delayed_work(&di->anc_invert_ctl_delay_work);
		flush_workqueue(di->anc_invert_ctl_delay_wq);
		destroy_workqueue(di->anc_invert_ctl_delay_wq);
	}

	if (di->ldo_supply_used) {
		ret = regulator_disable(di->anc_hs_vdd);
		if (ret)
			hwlog_err("%s: disable anc hs ldo failed\n", __func__);
	}

	misc_deregister(&anc_ncx8293_device);
	kfree(anc_priv);
	anc_priv = NULL;
	hwlog_info("%s: exit\n", __func__);

	return 0;
}

static const struct of_device_id anc_ncx8293_of_match[] = {
	{
		.compatible = "huawei,anc_ncx8293",
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, anc_ncx8293_of_match);

static const struct i2c_device_id anc_ncx8293_i2c_id[] = {
	{ "anc_ncx8293", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, anc_ncx8293_i2c_id);

static struct i2c_driver anc_ncx8293_driver = {
	.probe                = anc_ncx8293_probe,
	.remove               = anc_ncx8293_remove,
	.id_table             = anc_ncx8293_i2c_id,
	.driver = {
		.owner            = THIS_MODULE,
		.name             = "anc_ncx8293",
		.of_match_table   = of_match_ptr(anc_ncx8293_of_match),
	},
};

static int __init anc_ncx8293_init(void)
{
	int ret;

	ret = i2c_add_driver(&anc_ncx8293_driver);
	if (ret)
		hwlog_err("%s: i2c_add_driver error\n", __func__);

	return ret;
}

static void __exit anc_ncx8293_exit(void)
{
	i2c_del_driver(&anc_ncx8293_driver);
}

module_init(anc_ncx8293_init);
module_exit(anc_ncx8293_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("anc ncx8293 headset driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
