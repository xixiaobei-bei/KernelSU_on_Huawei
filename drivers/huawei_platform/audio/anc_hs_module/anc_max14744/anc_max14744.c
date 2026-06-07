/*
 * anc_max14744.c
 *
 * analog headset max14744 driver
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

#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/string.h>
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
#include "huawei_platform/audio/anc_max14744.h"

#define UNUSED(x) ((void)x)
#define HWLOG_TAG anc_max14744
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
	ANC_HS_CHARGE_OFF          = 0,
	ANC_HS_CHARGE_ON           = 1,
};

enum anc_max14744_irq_type {
	ANC_MAX14744_JACK_PLUG_IN  = 0,
	ANC_MAX14744_JACK_PLUG_OUT,
	ANC_MAX14744_BUTTON_PRESS,
	ANC_MAX14744_IQR_MAX,
};

#define  ANC_LIMIT_MIN                     20
#define  ANC_LIMIT_MAX                     200
#define  ANC_CHIP_STARTUP_TIME             30
#define  ADC_CALIBRATE_TIMES               10
#define  ADC_READ_COUNT                    3
#define  ADC_NORMAL_LIMIT_MIN              (-500)
#define  ADC_NORMAL_LIMIT_MAX              500
#define  ADC_OUT_OF_RANGE                  2499
#define  ANC_HOOK_MIN                      160
#define  ANC_HOOK_MAX                      185
#define  ANC_VOLUME_UP_MIN                 205
#define  ANC_VOLUME_UP_MAX                 230
#define  ANC_VOLUME_DOWN_MIN               240
#define  ANC_VOLUME_DOWN_MAX               265
#define  ADC_STEP                          476
#define  ADC_OFFSET                        170
#define  IDET_LVL1                         110
#define  IDET_LVL2                         220
#define  IDET_LVL3                         550
#define  LDO1_SHUTDOWN_MODE                0x00
#define  LDO1_CALL_MODE                    0x03
#define  NO_BUTTON_PRESS                   (-1)
#define  CODEC_GPIO_BASE                   224
#define  I2C_REG_FAIL_MAX_TIMES            10
#define  I2C_FAIL_REPORT_MAX_TIMES         20
#define  DELAY_TIME                        50
#define  UDEFINE_TIME                      500
#define  ADC_VAL_MIN                       (-100)
#define  ADC_VAL_MAX                       100
#define  ADC_CALIBRAT_MIN                  (-50)
#define  ADC_CALIBRAT_MAX                  50
#define  ANC_ADC_CHANNEL_H_DEFAULT         15
#define  ANC_ADC_CHANNEL_L_DEFAULT         14
#define  ANC_RESISTANCE_ADC_DEFAULT_VALUE  255
#define  VAL_STR_SIZE                      20
#define  ANC_HS_VDD                        3300000
#define  ANC_INTERRUPT_REGISTER            0x06
#define  ANC_REGISTER_MIN_ADDRESS          0x00
#define  ANC_REGISTER_MAX_ADDRESS          0x0c
#define  MAX_REGISTER_ADDRESS              0xff
#define  ANC_BTN_MASK (SND_JACK_BTN_0 | SND_JACK_BTN_1 | SND_JACK_BTN_2)

struct anc_max14744_priv {
	struct i2c_client *client;
	struct device *dev;
	void *control_data;
	struct regmap *regmap_l;
	struct regmap *regmap_r;
	int anc_hs_mode;                /* charge status */
	int anc_pwr_en_gpio;            /* VBST_5V EN */
	int channel_pwl_h;              /* adc channel for high voltage */
	int channel_pwl_l;              /* adc channel for low voltage */
	int anc_hs_limit_min;
	int anc_hs_limit_max;
	int anc_hs_btn_hook_min_volt;
	int anc_hs_btn_hook_max_volt;
	int anc_hs_btn_volume_up_min_volt;
	int anc_hs_btn_volume_up_max_volt;
	int anc_hs_btn_volume_down_min_volt;
	int anc_hs_btn_volume_down_max_volt;
	bool irq_flag;
	bool boost_flag;
	int sleep_time;                 /* charge chip pre-charge time */
	bool mic_used;                  /* flag to show mic status */
	bool detect_again;
	int force_charge_ctl;           /* force charge control for userspace */
	int hs_micbias_ctl;             /* hs micbias control */
	int adc_calibration_base;       /* calibration value */
	int button_pressed;
	int headset_type;
	struct mutex btn_mutex;
	struct mutex charge_lock;       /* charge status protect lock */
	struct mutex invert_hs_lock;
	struct wakeup_source wake_lock;
	spinlock_t irq_lock;
	int registered;                 /* anc hs regester flag */
	struct anc_hs_dev *anc_dev;     /* anc hs dev */
	void *private_data;             /* store codec description data */
	int gpio_int;
	int anc_max14744_irq;
	bool ldo_supply_used;
	bool cam_ldo_used;
	int cam_ldo_num;
	int cam_ldo_value;
	struct regulator *anc_hs_vdd;
	/* chip irq workqueue */
	struct workqueue_struct *anc_hs_plugin_delay_wq;
	struct delayed_work anc_hs_plugin_delay_work;
	struct workqueue_struct *anc_hs_plugout_delay_wq;
	struct delayed_work anc_hs_plugout_delay_work;
	struct workqueue_struct *anc_hs_btn_delay_wq;
	struct delayed_work anc_hs_btn_delay_work;
	struct workqueue_struct *anc_hs_invert_ctl_delay_wq;
	struct delayed_work anc_hs_invert_ctl_delay_work;
};

static struct anc_max14744_priv *anc_priv;
static unsigned int i2c_reg_fail_times;
static unsigned int i2c_fail_report_times = I2C_FAIL_REPORT_MAX_TIMES;

static struct reg_default anc_max14744_reg[] = {
	{ 0x00, 0x32 }, /* device id */
	{ 0x01, 0x00 }, /* adc conversion */
	{ 0x02, 0x00 }, /* adc status */
	{ 0x03, 0x00 }, /* status */
	{ 0x04, 0x00 }, /* Msic1 */
	{ 0x05, 0x00 }, /* Msic2 */
	{ 0x06, 0x00 }, /* interrupt */
	{ 0x07, 0x00 }, /* Mask */
	{ 0x08, 0x80 }, /* pins control1 */
	{ 0x09, 0x00 }, /* pins control2 */
	{ 0x0A, 0x00 }, /* adc control */
	{ 0x0B, 0x10 }, /* acc control1 */
	{ 0x0C, 0x00 }, /* acc control2 */
};

static bool anc_max14744_volatile_register(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case ANC_MAX14744_R000_DEVICE_ID:
	case ANC_MAX14744_R001_ADC_CONVERSION:
	case ANC_MAX14744_R002_ADC_STATUS:
	case ANC_MAX14744_R003_STATUS:
	case ANC_MAX14744_R004_MSIC1:
	case ANC_MAX14744_R005_MSIC2:
	case ANC_MAX14744_R006_INTERRUPT:
	case ANC_MAX14744_R00A_ADC_CONTROL:
		return true;
	default:
		return false;
	}
}

static bool anc_max14744_readable_register(struct device *dev, unsigned int reg)
{
	return true;
}

static inline int anc_hs_max14744_get_value(int gpio)
{
	if (gpio >= CODEC_GPIO_BASE)
		return gpio_get_value_cansleep(gpio);

	return gpio_get_value(gpio);
}

static inline void anc_max14744_gpio_set_value(int gpio, int value)
{
	if (gpio >= CODEC_GPIO_BASE)
		gpio_set_value_cansleep(gpio, value);
	else
		gpio_set_value(gpio, value);
}

static inline void anc_hs_enable_irq(int irq)
{
	if (!anc_priv->irq_flag) {
		enable_irq(irq);
		anc_priv->irq_flag = true;
	}
}

static inline void anc_hs_disable_irq(int irq)
{
	if (anc_priv->irq_flag) {
		disable_irq_nosync(irq);
		anc_priv->irq_flag = false;
	}
}

static void anc_dsm_i2c_reg_fail_report(void)
{
	i2c_reg_fail_times++;
	if ((i2c_reg_fail_times > I2C_REG_FAIL_MAX_TIMES) &&
		(i2c_fail_report_times > 0)) {
		i2c_reg_fail_times = 0;
		i2c_fail_report_times--;
#ifdef CONFIG_HUAWEI_DSM
		audio_dsm_report_info(AUDIO_ANC_HS, ANC_HS_I2C_ERR,
			"anc regmap i2c error\n");
#endif
	}
}

static int anc_max14744_regmap_read(int reg, int *value)
{
	int ret;

	ret = regmap_read(anc_priv->regmap_l, reg, value);

	if (ret < 0) {
		anc_dsm_i2c_reg_fail_report();
		hwlog_err("%s error,ret = %d, i2c_reg_fail_times = %d\n",
			__func__, ret, i2c_reg_fail_times);
	}

	return ret;
}

static int anc_max14744_regmap_write(int reg, int value)
{
	int ret;

	ret = regmap_write(anc_priv->regmap_l, reg, value);
	if (ret < 0) {
		anc_dsm_i2c_reg_fail_report();
		hwlog_err("%s error,ret = %d, i2c_reg_fail_times = %d\n",
			__func__, ret, i2c_reg_fail_times);
	}

	return ret;
}

static int anc_max14744_regmap_update_bits(int reg, int mask, int value)
{
	int ret;

	ret = regmap_update_bits(anc_priv->regmap_l, reg, mask, value);

	if (ret < 0) {
		anc_dsm_i2c_reg_fail_report();
		hwlog_err("%s error,ret = %d, i2c_reg_fail_times = %d\n",
			__func__, ret, i2c_reg_fail_times);
	}

	return ret;
}

bool anc_max14744_check_headset_pluged_in(void)
{
	int value = 0;

	if (!anc_priv)
		return false;

	anc_max14744_regmap_read(ANC_MAX14744_R003_STATUS, &value);

	if (!(value & ANC_MAX14744_JACK_DETECT_BIT))
		return true;

	return false;
}

static int force_clear_irq(void)
{
	int value = 0;
	int ret;

	ret = anc_max14744_regmap_read(ANC_MAX14744_R006_INTERRUPT, &value);
	if (ret < 0) {
		hwlog_err("anc_max14744 %s, read irq reg fail\n", __func__);
		return ret;
	}
	hwlog_info("anc_max14744 %s, irq reg : 0x%x\n", __func__, value);

	if ((value & ANC_MAX14744_PLUG_IRQ_BIT) &&
		!anc_max14744_check_headset_pluged_in()) {
		queue_delayed_work(anc_priv->anc_hs_plugout_delay_wq,
			&anc_priv->anc_hs_plugout_delay_work, 0);
		hwlog_info("anc_max14744 %s, plugout event\n", __func__);
	}

	return ret;
}

static void max14744_unmask_btn_irq(void)
{
	hwlog_info("anc_max14744 unmask btn irq\n");

	force_clear_irq();

	anc_max14744_regmap_update_bits(ANC_MAX14744_R007_MASK,
		ANC_MAX14744_BUTTON_PRESS_IRQ_BIT,
		ANC_MAX14744_BUTTON_PRESS_IRQ_BIT);
}

static void max14744_mask_btn_irq(void)
{
	hwlog_info("anc_max14744 mask btn irq\n");

	anc_max14744_regmap_update_bits(ANC_MAX14744_R007_MASK,
		ANC_MAX14744_BUTTON_PRESS_IRQ_BIT, 0x00);
}

static void max14744_unmask_plug_irq(void)
{
	hwlog_info("anc_max14744 unmask plug irq\n");

	anc_max14744_regmap_update_bits(ANC_MAX14744_R007_MASK,
		ANC_MAX14744_PLUG_IRQ_BIT, ANC_MAX14744_PLUG_IRQ_BIT);
}

static void max14744_mask_plug_irq(void)
{
	hwlog_info("anc_max14744 mask plug irq\n");

	anc_max14744_regmap_update_bits(ANC_MAX14744_R007_MASK,
		ANC_MAX14744_PLUG_IRQ_BIT, 0x00);
}

static void max14744_unmask_eoc_irq(void)
{
	hwlog_info("anc_max14744 unmask eoc irq\n");

	anc_max14744_regmap_update_bits(ANC_MAX14744_R007_MASK,
		ANC_MAX14744_EOC_IRQ_BIT, ANC_MAX14744_EOC_IRQ_BIT);
}

static void max14744_mask_eoc_irq(void)
{
	hwlog_info("anc_max14744 mask eoc irq\n");

	anc_max14744_regmap_update_bits(ANC_MAX14744_R007_MASK,
		ANC_MAX14744_EOC_IRQ_BIT, 0x00);
}

static void mic_bias_mode(int mode0)
{
	hwlog_info("max14744 go into %s", __func__);
	if (anc_priv) {
		anc_max14744_regmap_update_bits(ANC_MAX14744_R00B_ACC_CONTROL1,
			ANC_MAX14744_BYPASS_MASK, 0x00);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MODE1_MASK, 0x00);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MODE0_MASK, 0x00);

		mdelay(30);

		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MODE0_MASK, mode0);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MANUL_MIC_SW_MASK,
			ANC_MAX14744_MANUL_MIC_SW_MASK);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_FORCE_MIC_SW_MASK, 0x00);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MANUL_CNTL_MASK,
			ANC_MAX14744_MANUL_CNTL_MASK);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_FORCE_CNTL_MASK,
			ANC_MAX14744_FORCE_CNTL_MASK);
	}
}

static void power_mode(void)
{
	hwlog_info("max14744 go into %s", __func__);
	if (anc_priv) {
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MANUL_MIC_SW_MASK,
			ANC_MAX14744_MANUL_MIC_SW_MASK);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_FORCE_MIC_SW_MASK,
			ANC_MAX14744_FORCE_MIC_SW_MASK);

		mdelay(1);

		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MODE1_MASK,
			ANC_MAX14744_MODE1_MASK);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R00B_ACC_CONTROL1,
			ANC_MAX14744_BYPASS_MASK,
			ANC_MAX14744_BYPASS_MASK);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MANUL_CNTL_MASK,
			ANC_MAX14744_MANUL_CNTL_MASK);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_FORCE_CNTL_MASK,
			ANC_MAX14744_FORCE_CNTL_MASK);
	}
}

static void normal_mode(void)
{
	hwlog_info("max14744 go into %s", __func__);
	if (anc_priv) {
		anc_max14744_regmap_update_bits(ANC_MAX14744_R00B_ACC_CONTROL1,
			ANC_MAX14744_BYPASS_MASK, 0x00);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MODE1_MASK, 0x00);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MODE0_MASK, 0x00);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MANUL_MIC_SW_MASK, 0x00);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_FORCE_MIC_SW_MASK, 0x00);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MANUL_CNTL_MASK,
			ANC_MAX14744_MANUL_CNTL_MASK);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_FORCE_CNTL_MASK, 0x00);
	}
}

void anc_max14744_refresh_headset_type(int headset_type)
{
	if (anc_priv) {
		anc_priv->headset_type = headset_type;
		hwlog_info("update max14744hs_type %d", anc_priv->headset_type);
	}
}

static void anc_hs_invert_ctl_work(struct work_struct *work)
{
	__pm_stay_awake(&anc_priv->wake_lock);
	mutex_lock(&anc_priv->invert_hs_lock);

	if (anc_priv->headset_type == ANC_HS_REVERT_4POLE) {
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MANUL_CNTL_MASK,
			ANC_MAX14744_MANUL_CNTL_MASK);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_FORCE_CNTL_MASK, 0x00);
		hwlog_info("max14744:invert headset plugin,connect MIC GND");
	}

	mutex_unlock(&anc_priv->invert_hs_lock);
	__pm_relax(&anc_priv->wake_lock);
}

void anc_max14744_invert_headset_control(int connect)
{
	switch (connect) {
	case ANC_HS_MIC_GND_DISCONNECT:
		cancel_delayed_work(&anc_priv->anc_hs_invert_ctl_delay_work);
		flush_workqueue(anc_priv->anc_hs_invert_ctl_delay_wq);

		mutex_lock(&anc_priv->invert_hs_lock);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_MANUL_CNTL_MASK, 0x00);
		anc_max14744_regmap_update_bits(ANC_MAX14744_R008_PINS_CONTROL1,
			ANC_MAX14744_FORCE_CNTL_MASK, 0x00);
		mutex_unlock(&anc_priv->invert_hs_lock);

		hwlog_info("max14744: disconnect MIC and GND");
		break;
	case ANC_HS_MIC_GND_CONNECT:
		queue_delayed_work(anc_priv->anc_hs_invert_ctl_delay_wq,
			&anc_priv->anc_hs_invert_ctl_delay_work,
			msecs_to_jiffies(3000));
		hwlog_info("%s: queue delay work", __func__);
		break;
	default:
		hwlog_info("%s: unknown connect type", __func__);
		break;
	}
}

static int anc_max14744_get_adc_delta(void)
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
		ear_pwr_h = hisi_adc_get_value(anc_priv->channel_pwl_h);
		if (ear_pwr_h < 0) {
			hwlog_err("%s:get hkadc(h) fail, err:%d\n", __func__,
				ear_pwr_h);
			fail_count++;
			continue;
		}
		ear_pwr_l = hisi_adc_get_value(anc_priv->channel_pwl_l);
		if (ear_pwr_l < 0) {
			hwlog_err("%s:get hkadc(l) fail, err:%d\n", __func__,
				ear_pwr_l);
			fail_count++;
			continue;
		}
		hwlog_info("%s:adc_h:%d,adc_l:%d\n", __func__, ear_pwr_h,
			ear_pwr_l);

		temp = ear_pwr_h - ear_pwr_l - anc_priv->adc_calibration_base;

		/* if adc value far away from normal value,just abandon it */
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
		hwlog_err("%s:get anc_hs hkadc failed\n", __func__);
		return false;
	}
	/* compute an average value */
	delta /= count;
	hwlog_info("%s:final adc value= %d count=%d\n", __func__, delta, count);
	return delta;
}

static int anc_max14744_get_btn_value(void)
{
	int delta;

	delta = anc_max14744_get_adc_delta();
	if ((delta >= anc_priv->anc_hs_btn_hook_min_volt) &&
		(delta <= anc_priv->anc_hs_btn_hook_max_volt))
		return SND_JACK_BTN_0;

	if ((delta >= anc_priv->anc_hs_btn_volume_up_min_volt) &&
		(delta <= anc_priv->anc_hs_btn_volume_up_max_volt))
		return SND_JACK_BTN_1;

	if ((delta >= anc_priv->anc_hs_btn_volume_down_min_volt) &&
		(delta <= anc_priv->anc_hs_btn_volume_down_max_volt))
		return SND_JACK_BTN_2;

	hwlog_err("[anc_max14744]btn delta not in range delta:%d\n", delta);
	return NO_BUTTON_PRESS;
}

static bool anc_max14744_btn_press(void)
{
	int value = 0;

	if (!anc_priv)
		return false;

	anc_max14744_regmap_read(ANC_MAX14744_R004_MSIC1, &value);

	if (value & ANC_MAX14744_BUTTON_PRESS_BIT)
		return false;

	return true;
}

static void anc_max14744_btn_judge(void)
{
	struct anc_hs_dev *pdev = anc_priv->anc_dev;
	struct anc_hs_codec_ops *fops = &pdev->ops;
	int btn_report;

	if (!anc_priv->registered)
		return;

	hwlog_info("%s%u:deal with button irq event\n", __func__, __LINE__);

	/* should get wake lock before codec power lock which may be blocked */
	mutex_lock(&anc_priv->btn_mutex);

	if (anc_max14744_btn_press() && (anc_priv->button_pressed == 0)) {
		/* button down event */
		hwlog_info("%s%u : button down event\n", __func__, __LINE__);

		mdelay(DELAY_TIME);

		btn_report = anc_max14744_get_btn_value();
		if (btn_report != NO_BUTTON_PRESS) {
			anc_priv->button_pressed = 1;
			fops->btn_report(btn_report, ANC_BTN_MASK);
		} else {
			hwlog_warn("%s: it is not a button press", __func__);
		}
	} else if (anc_priv->button_pressed == 1) {
		/* button up event */
		hwlog_info("%s%u : button up event\n", __func__, __LINE__);

		btn_report = 0;
		anc_priv->button_pressed = 0;

		/*
		 * we permit button up event report to userspace,
		 * make sure down and up in pair
		 */
		fops->btn_report(btn_report, ANC_BTN_MASK);
	}

	mutex_unlock(&anc_priv->btn_mutex);
}

static int anc_hs_send_hifi_msg(int anc_status)
{
	struct mlib_para_meter_voice *anc_hs_para = NULL;
	struct mlib_set_para_info *mlib_set_para = NULL;
	int ret = 0;

	mlib_set_para = kzalloc(sizeof(*mlib_set_para) +
		MLIB_PARA_LENGTH_MAX, GFP_KERNEL);
	if (!mlib_set_para)
		return ERROR_RET;

	mlib_set_para->msg_id = ID_AP_AUDIO_MLIB_SET_PARA_IND;
	mlib_set_para->path_id = MLIB_PATH_CS_VOICE_CALL_MICIN;
	mlib_set_para->size = MLIB_PARA_LENGTH_MAX;
	mlib_set_para->reserve = 0;
	mlib_set_para->module_id = MLIB_MODULE_3A_VOICE;

	anc_hs_para = (struct mlib_para_meter_voice *)mlib_set_para->auc_data;
	anc_hs_para->key = MLIB_ANC_HS_PARA_ENABLE;
	anc_hs_para->value = anc_status;
#ifdef CONFIG_HIFI_DSP_ONE_TRACK
	ret = hifi_send_msg(MAILBOX_MAILCODE_ACPU_TO_HIFI_MISC, mlib_set_para,
		sizeof(*mlib_set_para) + MLIB_PARA_LENGTH_MAX);
#endif
	kfree(mlib_set_para);

	return ret;
}

static bool anc_max14744_need_charge(void)
{
	int delta;

	mdelay(30);
	delta = anc_max14744_get_adc_delta();
	if ((delta >= anc_priv->anc_hs_limit_min) &&
		(delta <= anc_priv->anc_hs_limit_max)) {
		hwlog_info("[%s][%d] anc headset = true\n", __func__, __LINE__);
		return true;
	}

	hwlog_info("[%s][%d] anc headset = false\n", __func__, __LINE__);
	return false;
}

static void anc_max14744_adc_calibration(void)
{
	int loop = ADC_CALIBRATE_TIMES;
	int count;
	int fail_count = 0;
	int adc_h;
	int adc_l;

	anc_priv->adc_calibration_base = 0;

	while (loop) {
		loop--;
		usleep_range(1000, 1100);
		adc_h = hisi_adc_get_value(anc_priv->channel_pwl_h);
		if (adc_h < 0) {
			hwlog_err("[anc_hs]get adc fail,adc_h:%d\n", adc_h);
			fail_count++;
			continue;
		}
		adc_l = hisi_adc_get_value(anc_priv->channel_pwl_l);
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
		anc_priv->adc_calibration_base += (adc_h - adc_l);
	}

	count = ADC_CALIBRATE_TIMES - loop - fail_count;
	if (count == 0) {
		/* if all adc read fail, set 0 to it as default */
		anc_priv->adc_calibration_base = 0;
		hwlog_err("[anc_hs] calibration whole failed\n");
	} else {
		anc_priv->adc_calibration_base /= count;
		hwlog_info("anc_hs:calibration_base = %d with %d times\n",
			anc_priv->adc_calibration_base, count);

		if (anc_priv->adc_calibration_base > ADC_CALIBRAT_MAX ||
			anc_priv->adc_calibration_base < ADC_CALIBRAT_MIN) {
			anc_priv->adc_calibration_base = 0;
			hwlog_err("[anc_hs]calibration value is not illegal\n");
		}
	}
}

static bool anc_max14744_charge_judge(void)
{
	struct anc_hs_dev *pdev = anc_priv->anc_dev;
	struct anc_hs_codec_ops *fops = &pdev->ops;

	/* userspace prohibit charging with using hs-mic pin */
	if (anc_priv->force_charge_ctl == ANC_HS_DISABLE_CHARGE) {
		hwlog_info("%s : charge is occupied by app level\n", __func__);
		/* need second detect for charge */
		anc_priv->detect_again = true;
		return false;
	}

	/* hs mic is using record, not take it */
	if (anc_priv->hs_micbias_ctl == ANC_HS_DISABLE_CHARGE) {
		/* need second detect for charge */
		hwlog_info("%s%u :hs mic is in using\n", __func__, __LINE__);
		anc_priv->detect_again = true;
		return false;
	}

	anc_priv->detect_again = false;
	hwlog_debug("%s%u : anc hs charge\n", __func__, __LINE__);

	/* headset may have pluged out, just return */
	if (!fops->check_headset_in(anc_priv->private_data)) {
		hwlog_info("%s%u : headset has plug out\n", __func__, __LINE__);
		return false;
	}

	mutex_lock(&anc_priv->charge_lock);
	/* connect 5vboost with hs_mic pin */
	power_mode();
	mutex_unlock(&anc_priv->charge_lock);

	/* waiting for anc chip start up */
	hwlog_info("%s: delay %d ms to wait anc chip up\n", __func__,
		anc_priv->sleep_time);

	mdelay(anc_priv->sleep_time);

	mutex_lock(&anc_priv->charge_lock);

	if ((anc_priv->hs_micbias_ctl == ANC_HS_ENABLE_CHARGE) &&
		anc_max14744_need_charge()) {
		/* start to charge for anc hs and respond charging btn event */
		if (anc_priv->anc_hs_mode == ANC_HS_CHARGE_OFF) {
			hwlog_info("%s : anc_hs enable irq\n", __func__);
			anc_priv->anc_hs_mode = ANC_HS_CHARGE_ON;
			max14744_unmask_btn_irq();
		}
		if (anc_hs_send_hifi_msg(ANC_HS_CHARGE_ON) == ERROR_RET)
			hwlog_err("%s:anc_hs_send_hifi_msg TURN ON ANC fail\n",
				__func__);
	} else {
		if (anc_priv->anc_hs_mode == ANC_HS_CHARGE_ON)
			hwlog_info("%s : anc_hs disable irq\n", __func__);

		/* stop charge and change status to CHARGE_OFF */
		max14744_mask_btn_irq();
		mic_bias_mode(LDO1_CALL_MODE);
		udelay(UDEFINE_TIME);
		anc_priv->anc_hs_mode = ANC_HS_CHARGE_OFF;
		if (anc_hs_send_hifi_msg(ANC_HS_CHARGE_OFF) == ERROR_RET)
			hwlog_err("%s:anc_hs_send_hifi_msg TURN OFF ANC fail\n",
				__func__);
	}

	mutex_unlock(&anc_priv->charge_lock);

	if (anc_priv->anc_hs_mode == ANC_HS_CHARGE_ON)
		return true;

	return false;
}

static void update_charge_status(void)
{
	struct anc_hs_dev *pdev = anc_priv->anc_dev;
	struct anc_hs_codec_ops *fops = &pdev->ops;

	if (anc_priv->hs_micbias_ctl == ANC_HS_DISABLE_CHARGE ||
		anc_priv->force_charge_ctl == ANC_HS_DISABLE_CHARGE) {
		/* force stop charge function */
		mutex_lock(&anc_priv->charge_lock);

		if (anc_priv->anc_hs_mode == ANC_HS_CHARGE_ON) {
			max14744_mask_btn_irq();
			mic_bias_mode(LDO1_CALL_MODE);
			udelay(UDEFINE_TIME);

			hwlog_info("%s : stop charging for anc hs\n", __func__);
			anc_priv->anc_hs_mode = ANC_HS_CHARGE_OFF;
			anc_priv->mic_used = true;
		}

		mutex_unlock(&anc_priv->charge_lock);
	} else if (anc_priv->hs_micbias_ctl == ANC_HS_ENABLE_CHARGE &&
		anc_priv->force_charge_ctl == ANC_HS_ENABLE_CHARGE) {
		if (anc_priv->mic_used) {
			anc_priv->mic_used = false;
			/* headset maybe have plug out here */
			if (!fops->check_headset_in(anc_priv->private_data)) {
				hwlog_info("%s : headset plug out\n", __func__);
			} else {
				/* force resume charge for anc headset */
				mutex_lock(&anc_priv->charge_lock);
				if (anc_priv->anc_hs_mode == ANC_HS_CHARGE_OFF) {
					anc_priv->anc_hs_mode =
						ANC_HS_CHARGE_ON;
					power_mode();
					udelay(UDEFINE_TIME);
					max14744_unmask_btn_irq();
					hwlog_info("%s:resume charge anc hs\n",
						__func__);
				}
				mutex_unlock(&anc_priv->charge_lock);
			}
		} else if (anc_priv->detect_again) {
			/* need detect charge again due to interrupted before */
			anc_max14744_charge_judge();
		}
	}
}

void anc_max14744_start_charge(void)
{
	if (!anc_priv || !anc_priv->registered)
		return;

	/* enable 5vboost first, this need time to be stable */
	hwlog_info("%s%u:enable 5vboost\n", __func__, __LINE__);

	/* default let hsbias connect to hs-mic pin */
	mic_bias_mode(LDO1_CALL_MODE);
	anc_max14744_gpio_set_value(anc_priv->anc_pwr_en_gpio, 1);
	anc_priv->anc_hs_mode = ANC_HS_CHARGE_OFF;
}

void anc_max14744_force_charge(int disable)
{
	if (!anc_priv || !anc_priv->registered)
		return;
	/* don't make repeated switch with charge status */
	if (disable == anc_priv->hs_micbias_ctl)
		return;
	anc_priv->hs_micbias_ctl = disable;

	/* update charge status here */
	update_charge_status();
}

bool anc_max14744_charge_detect(int saradc_value, int headset_type)
{
	if (!anc_priv || !anc_priv->registered)
		return false;

	/* calibration adc resistance which can make charge detect accuracy */
	anc_max14744_adc_calibration();

	anc_priv->headset_type = headset_type;

	/* revert 4-pole headset need 5v on to support second recognition */
	if (headset_type == ANC_HS_NORMAL_4POLE) {
		/* 4-pole headset maybe an anc headset */
		hwlog_debug("%s : start anc hs charge judge\n", __func__);
		return anc_max14744_charge_judge();
	}

	if (headset_type == ANC_HS_NORMAL_3POLE) {
		hwlog_info("%s:no disable 5vboost for 3-pole hs\n", __func__);
		/* 3-pole also support second-detect */
		return false;
	}

	return false;
}

void anc_max14744_stop_charge(void)
{
	if (!anc_priv || !anc_priv->registered)
		return;

	hwlog_info("%s : stop anc hs charge\n", __func__);

	max14744_mask_btn_irq();
	normal_mode();
	anc_max14744_gpio_set_value(anc_priv->anc_pwr_en_gpio, 0);
	anc_priv->anc_hs_mode = ANC_HS_CHARGE_OFF;
	anc_priv->headset_type = ANC_HS_NONE;
	anc_priv->button_pressed = 0;

	if (anc_hs_send_hifi_msg(ANC_HS_CHARGE_OFF) == ERROR_RET)
		hwlog_err("%s:anc_send_hifi_msg TURN OFF ANC fail\n", __func__);
}

int anc_max14744_dev_register(struct anc_hs_dev *dev, void *codec_data)
{
	int value = 0;
	int ret;

	/* anc_hs driver not be probed, just return */
	if (!anc_priv)
		return -ENODEV;

	/* only support one codec to be registered */
	if (anc_priv->registered) {
		hwlog_err("one codec has registered, no more permit\n");
		return -EEXIST;
	}
	if (!dev->ops.check_headset_in || !dev->ops.btn_report ||
		!dev->ops.codec_resume_lock || !dev->ops.plug_in_detect ||
		!dev->ops.plug_out_detect) {
		hwlog_err("codec ops function must be all registed\n");
		return -EINVAL;
	}

	anc_priv->anc_dev = dev;
	anc_priv->private_data = codec_data;
	anc_priv->registered = true;

	ret = anc_max14744_regmap_read(ANC_MAX14744_R006_INTERRUPT, &value);
	if (ret < 0)
		hwlog_err("%s:anc_max14744 force_clear_irq,read irq reg fail\n",
			__func__);

	max14744_unmask_plug_irq();
	hwlog_info("%s : anc hs has been register sucessful\n", __func__);

	return 0;
}

bool check_anc_max14744_support(void)
{
	if (!anc_priv || !anc_priv->registered)
		return false;

	return true;
}

bool anc_max14744_plug_enable(void)
{
	if (!anc_priv || !anc_priv->registered)
		return false;

	return true;
}

static int get_irq_type(void)
{
	int irq_type = ANC_MAX14744_IQR_MAX;
	int value = 0;
	int mask = 0;

	anc_max14744_regmap_read(ANC_MAX14744_R006_INTERRUPT, &value);
	anc_max14744_regmap_read(ANC_MAX14744_R007_MASK, &mask);

	if ((value & ANC_MAX14744_PLUG_IRQ_BIT) &&
		anc_max14744_check_headset_pluged_in())
		irq_type = ANC_MAX14744_JACK_PLUG_IN;
	else if ((value & ANC_MAX14744_PLUG_IRQ_BIT) &&
		!anc_max14744_check_headset_pluged_in())
		irq_type = ANC_MAX14744_JACK_PLUG_OUT;
	else if ((value & ANC_MAX14744_BUTTON_PRESS_IRQ_BIT) &&
		(mask & ANC_MAX14744_BUTTON_PRESS_IRQ_MASK_BIT))
		irq_type = ANC_MAX14744_BUTTON_PRESS;

	hwlog_info("max14744 irq value is 0x%x, mask is 0x%x, type is %d",
		value, mask, irq_type);

	return irq_type;
}

static void anc_hs_plugin_work(struct work_struct *work)
{
	struct anc_hs_codec_ops *fops = &anc_priv->anc_dev->ops;

	__pm_stay_awake(&anc_priv->wake_lock);

	if (anc_priv->private_data)
		fops->plug_in_detect(anc_priv->private_data);

	__pm_relax(&anc_priv->wake_lock);
}

static void anc_hs_plugout_work(struct work_struct *work)
{
	struct anc_hs_codec_ops *fops = &anc_priv->anc_dev->ops;

	__pm_stay_awake(&anc_priv->wake_lock);

	if (anc_priv->private_data)
		fops->plug_out_detect(anc_priv->private_data);

	__pm_relax(&anc_priv->wake_lock);
}

static void anc_hs_btn_work(struct work_struct *work)
{
	__pm_stay_awake(&anc_priv->wake_lock);
	anc_max14744_btn_judge();
	__pm_relax(&anc_priv->wake_lock);
}

static irqreturn_t anc_max14744_irq_handler(int irq, void *data)
{
	int irq_type;
	int value = 0;
	int loop = 5;

	if (!anc_priv || !anc_priv->registered) {
		pr_info("anc_max14744 dev has not been regestered\n");
		anc_max14744_regmap_read(ANC_MAX14744_R006_INTERRUPT, &value);
		return IRQ_HANDLED;
	}

	__pm_wakeup_event(&anc_priv->wake_lock, 1000);
	irq_type = get_irq_type();

	switch (irq_type) {
	case ANC_MAX14744_JACK_PLUG_IN:
		queue_delayed_work(anc_priv->anc_hs_plugin_delay_wq,
			&anc_priv->anc_hs_plugin_delay_work,
			msecs_to_jiffies(800));
		break;
	case ANC_MAX14744_JACK_PLUG_OUT:
		queue_delayed_work(anc_priv->anc_hs_plugout_delay_wq,
			&anc_priv->anc_hs_plugout_delay_work, 0);
		break;
	case ANC_MAX14744_BUTTON_PRESS:
		queue_delayed_work(anc_priv->anc_hs_btn_delay_wq,
			&anc_priv->anc_hs_btn_delay_work, msecs_to_jiffies(20));
		break;
	default:
		break;
	}

	while (loop > 0) {
		irq_type = get_irq_type();
		if (irq_type == ANC_MAX14744_IQR_MAX)
			break;

		if (irq_type == ANC_MAX14744_JACK_PLUG_OUT)
			queue_delayed_work(anc_priv->anc_hs_plugout_delay_wq,
				&anc_priv->anc_hs_plugout_delay_work, 0);

		loop = loop - 1;
	}

#ifdef CONFIG_HUAWEI_DSM
	if (loop <= 0)
		audio_dsm_report_info(AUDIO_ANC_HS, ANC_HS_UNHANDLED_IRQ,
			"there is irq unhandled in %s\n", __func__);
#endif

	return IRQ_HANDLED;
}

static int judge_headset_type_further(void)
{
	int value = 0;
	int idet_lvl = 0;
	int retry = 3;
	int idet;

	while (retry--) {
		anc_max14744_regmap_read(ANC_MAX14744_R002_ADC_STATUS, &value);
		if (value & ANC_MAX14744_EOC_BIT)
			break;

		msleep(20);
	}
	if (retry < 0) {
		anc_max14744_regmap_read(ANC_MAX14744_R001_ADC_CONVERSION,
			&value);
		hwlog_err("hs resist:adc not prepare first time %d\n", value);
		return ANC_RESISTANCE_ADC_DEFAULT_VALUE;
	}

	anc_max14744_regmap_read(ANC_MAX14744_R001_ADC_CONVERSION, &value);
	hwlog_info("headset resistance first time: %d\n", value);

	/* compute adc final value */
	anc_max14744_regmap_read(ANC_MAX14744_R00A_ADC_CONTROL, &idet_lvl);
	if (idet_lvl & ANC_MAX14744_IDETLVL_BIT)
		idet =  IDET_LVL3;
	else
		idet =  IDET_LVL1;

	value = ((value * ADC_STEP + ADC_OFFSET) / idet);

	hwlog_info("resistance value: %d with idet:%d\n", value, idet);

	return value;
}

static int compute_final_voltage(void)
{
	int r1 = 51; /* low impedance value */
	int r2 = 100; /* high impedance value */
	int adc_volt_low;
	int adc_volt_high;

	msleep(70);
	adc_volt_low = hisi_adc_get_value(anc_priv->channel_pwl_l);
	adc_volt_high = hisi_adc_get_value(anc_priv->channel_pwl_h);

	hwlog_info("adc_l value: %d adc_h value: %d\n", adc_volt_low, adc_volt_high);

	return (adc_volt_low * (r1 + r2)) / r1;
}

static long anc_max14744_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	int ret;
	int charge_mode;
	int voltage;
	int adc_value;
	unsigned int __user *p_user = (unsigned int __user *)arg;

	if (!p_user)
		return -EBUSY;

	if (!anc_priv->registered)
		return -EBUSY;

	switch (cmd) {
	case IOCTL_ANC_HS_CHARGE_ENABLE_CMD:
		if (anc_priv->force_charge_ctl == ANC_HS_ENABLE_CHARGE)
			break;
		/* resume anc headset charge */
		anc_priv->force_charge_ctl = ANC_HS_ENABLE_CHARGE;
		hwlog_info("app level contrl set charge status with %d\n",
			anc_priv->force_charge_ctl);
		update_charge_status();
		break;
	case IOCTL_ANC_HS_CHARGE_DISABLE_CMD:
		if (anc_priv->force_charge_ctl == ANC_HS_DISABLE_CHARGE)
			break;
		/* force stop anc headset charge */
		anc_priv->force_charge_ctl = ANC_HS_DISABLE_CHARGE;
		hwlog_info("app level contrl set charge status with %d\n",
			anc_priv->force_charge_ctl);
		update_charge_status();
		break;
	case IOCTL_ANC_HS_GET_HEADSET_CMD:
		charge_mode = anc_priv->anc_hs_mode;
		if (charge_mode == ANC_HS_CHARGE_ON) {
			if (!anc_max14744_need_charge()) {
				charge_mode = ANC_HS_CHARGE_OFF;
			} else {
				usleep_range(10000, 10100);
				if (!anc_max14744_need_charge())
					charge_mode = ANC_HS_CHARGE_OFF;
			}
		}
		if (charge_mode == ANC_HS_CHARGE_ON)
			anc_priv->headset_type = ANC_HS_HEADSET;

		ret = put_user((__u32)(anc_priv->headset_type), p_user);
		break;
	case IOCTL_ANC_HS_GET_CHARGE_STATUS_CMD:
		ret = put_user((__u32)(anc_priv->anc_hs_mode), p_user);
		break;
	case IOCTL_ANC_HS_GET_VBST_5VOLTAGE_CMD:
		mic_bias_mode(LDO1_CALL_MODE);
		anc_max14744_gpio_set_value(anc_priv->anc_pwr_en_gpio, 1);
		voltage = compute_final_voltage();
		anc_max14744_gpio_set_value(anc_priv->anc_pwr_en_gpio, 0);
		ret = put_user((__u32)voltage, p_user);
		break;
	case IOCTL_ANC_HS_GET_VDD_BUCK_VOLTAGE_CMD:
		mic_bias_mode(LDO1_CALL_MODE);
		anc_max14744_gpio_set_value(anc_priv->anc_pwr_en_gpio, 0);
		voltage = compute_final_voltage();
		ret = put_user((__u32)voltage, p_user);
		break;
	case IOCTL_ANC_HS_GET_HEADSET_RESISTANCE_CMD:
		adc_value = judge_headset_type_further();
		ret = put_user((__u32)(adc_value), p_user);
		break;
	default:
		hwlog_err("unsupport cmd\n");
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

#ifdef ANC_MAX14744_DEBUG
static ssize_t anc_max14744_reg_list_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int value = 0;
	int reg;
	char val_str[VAL_STR_SIZE] = {0};

	if (!buf) {
		hwlog_info("%s : buf is null\n", __func__);
		return -EINVAL;
	}
	buf[0] = '\0';
	for (reg = 0; reg <= ANC_REGISTER_MAX_ADDRESS; reg++) {
		anc_max14744_regmap_read(reg, &value);
		sprintf(val_str, "0x%02x = 0x%02x\n", reg, value);
		strcat(buf, val_str);
	}
	return strlen(buf);
}

static ssize_t anc_max14744_adc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char val_str[VAL_STR_SIZE] = {0};
	int ear_pwr_h;
	int ear_pwr_l;

	if (!buf) {
		hwlog_info("%s : buf is null\n", __func__);
		return -EINVAL;
	}
	buf[0] = '\0';

	ear_pwr_h = hisi_adc_get_value(anc_priv->channel_pwl_h);
	mdelay(DELAY_TIME);
	ear_pwr_l = hisi_adc_get_value(anc_priv->channel_pwl_l);
	mdelay(DELAY_TIME);
	sprintf(val_str, "h = %d, l = %d\n", ear_pwr_h, ear_pwr_l);
	strcat(buf, val_str);

	return strlen(buf);
}
static ssize_t anc_max14744_reg_write_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	long reg = 0;
	long val = 0;
	const char *p_val = buf;
	char *p_reg = NULL;

	if ((!buf) || (!strstr(buf, " "))) {
		hwlog_err("%s: input parameters error\n", __func__);
		return -EINVAL;
	}

	p_reg = strsep(&p_val, " ");

	if ((!p_reg) || (!p_val) || (*p_reg == '\0') || ('\0' == *p_val)) {
		hwlog_err("%s: input register address or value is \\0\n",
			__func__);
		return -EINVAL;
	}

	ret = kstrtol(p_reg, 16, &reg);
	if (ret < 0) {
		hwlog_err("%s: input register address error:%s\n", __func__,
			p_reg);
		return -EINVAL;
	}

	ret = kstrtol(p_val, 16, &val);
	if (ret < 0) {
		hwlog_err("%s: input register value error:%s\n", __func__,
			p_val);
		return -EINVAL;
	}

	if (reg <= ANC_INTERRUPT_REGISTER || reg > ANC_REGISTER_MAX_ADDRESS) {
		hwlog_err("invalid register address: 0x%x\n", reg);
		return -ENXIO;
	}

	if (val < ANC_REGISTER_MIN_ADDRESS || val > MAX_REGISTER_ADDRESS) {
		hwlog_err("invalid register value: 0x%x\n", val);
		return -ENXIO;
	}

	anc_max14744_regmap_write((int)reg, (int)val);
	return count;
}

static DEVICE_ATTR(reg_list, 0664, anc_max14744_reg_list_show, NULL);
static DEVICE_ATTR(reg_write, 0660, NULL, anc_max14744_reg_write_store);
static DEVICE_ATTR(adc, 0664, anc_max14744_adc_show, NULL);

static struct attribute *anc_max14744_attributes[] = {
	&dev_attr_reg_list.attr,
	&dev_attr_reg_write.attr,
	&dev_attr_adc.attr,
	NULL
};

static const struct attribute_group anc_max14744_attr_group = {
	.attrs = anc_max14744_attributes,
};
#endif

static const struct regmap_config anc_max14744_regmap = {
	.reg_bits	      = 8,
	.val_bits	      = 8,
	.max_register	  = ANC_MAX14744_R014_CONTROL,
	.reg_defaults	  = anc_max14744_reg,
	.num_reg_defaults = ARRAY_SIZE(anc_max14744_reg),
	.volatile_reg	  = anc_max14744_volatile_register,
	.readable_reg	  = anc_max14744_readable_register,
	.cache_type	      = REGCACHE_RBTREE,
};

static const struct file_operations anc_max14744_fops = {
	.owner	          = THIS_MODULE,
	.open		      = simple_open,
	.unlocked_ioctl	  = anc_max14744_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	  = anc_max14744_ioctl,
#endif
};

static struct miscdevice anc_max14744_device = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = "anc_hs",
	.fops   = &anc_max14744_fops,
};

static void read_adc_channel_number(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "adc_channel_h", &temp))
		anc_priv->channel_pwl_h = temp;
	else
		anc_priv->channel_pwl_h = ANC_ADC_CHANNEL_H_DEFAULT;

	if (!of_property_read_u32(node, "adc_channel_l", &temp))
		anc_priv->channel_pwl_l = temp;
	else
		anc_priv->channel_pwl_l = ANC_ADC_CHANNEL_L_DEFAULT;
}

static void read_charge_limit(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "anc_hs_limit_min", &temp))
		anc_priv->anc_hs_limit_min = temp;
	else
		anc_priv->anc_hs_limit_min = ANC_LIMIT_MIN;

	if (!of_property_read_u32(node, "anc_hs_limit_max", &temp))
		anc_priv->anc_hs_limit_max = temp;
	else
		anc_priv->anc_hs_limit_max = ANC_LIMIT_MAX;
}

static void read_hook_limit(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "anc_hs_btn_hook_min_voltage", &temp))
		anc_priv->anc_hs_btn_hook_min_volt = temp;
	else
		anc_priv->anc_hs_btn_hook_min_volt = ANC_HOOK_MIN;

	if (!of_property_read_u32(node, "anc_hs_btn_hook_max_voltage", &temp))
		anc_priv->anc_hs_btn_hook_max_volt = temp;
	else
		anc_priv->anc_hs_btn_hook_max_volt = ANC_HOOK_MAX;
}

static void read_volume_up_limit(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "anc_hs_btn_volume_up_min_voltage",
		&temp))
		anc_priv->anc_hs_btn_volume_up_min_volt = temp;
	else
		anc_priv->anc_hs_btn_volume_up_min_volt =
			ANC_VOLUME_UP_MIN;
	if (!of_property_read_u32(node, "anc_hs_btn_volume_up_max_voltage",
		&temp))
		anc_priv->anc_hs_btn_volume_up_max_volt = temp;
	else
		anc_priv->anc_hs_btn_volume_up_max_volt =
			ANC_VOLUME_UP_MAX;
}

static void read_volume_down_limit(struct device_node *node)
{
	int temp = 0;

	if (!of_property_read_u32(node, "anc_hs_btn_volume_down_min_voltage",
		&temp))
		anc_priv->anc_hs_btn_volume_down_min_volt = temp;
	else
		anc_priv->anc_hs_btn_volume_down_min_volt =
			ANC_VOLUME_DOWN_MIN;
	if (!of_property_read_u32(node, "anc_hs_btn_volume_down_max_voltage",
		&temp))
		anc_priv->anc_hs_btn_volume_down_max_volt = temp;
	else
		anc_priv->anc_hs_btn_volume_down_max_volt =
			ANC_VOLUME_DOWN_MAX;
}

static void load_anc_hs_config(struct device_node *node)
{
	read_adc_channel_number(node);

	read_charge_limit(node);

	read_hook_limit(node);

	read_volume_up_limit(node);

	read_volume_down_limit(node);
}

static void chip_init(void)
{
	int value = 0;

	/* 0x00: mask reset value */
	anc_max14744_regmap_write(ANC_MAX14744_R007_MASK, 0x00);
	/* 0x80: pin_control1 force manual cntl setting */
	anc_max14744_regmap_write(ANC_MAX14744_R008_PINS_CONTROL1, 0x80);
	/* 0x00: pin_control2 reset value */
	anc_max14744_regmap_write(ANC_MAX14744_R009_PINS_CONTROL2, 0x00);
	/* 0x10: adc_control1 reset value */
	anc_max14744_regmap_write(ANC_MAX14744_R00B_ACC_CONTROL1, 0x10);
	/* 0x00: adc_control2 reset value */
	anc_max14744_regmap_write(ANC_MAX14744_R00C_ACC_CONTROL2, 0x00);
	anc_max14744_regmap_read(ANC_MAX14744_R006_INTERRUPT, &value);
}

static struct anc_hs_ops anc_max14744_ops = {
	.anc_hs_dev_register = anc_max14744_dev_register,
	.anc_hs_check_headset_pluged_in = anc_max14744_check_headset_pluged_in,
	.anc_hs_start_charge = anc_max14744_start_charge,
	.anc_hs_charge_detect = anc_max14744_charge_detect,
	.anc_hs_stop_charge = anc_max14744_stop_charge,
	.anc_hs_force_charge = anc_max14744_force_charge,
	.check_anc_hs_support = check_anc_max14744_support,
	.anc_hs_plug_enable = anc_max14744_plug_enable,
	.anc_hs_5v_control = NULL,
	.anc_hs_invert_hs_control = anc_max14744_invert_headset_control,
	.anc_hs_refresh_headset_type = anc_max14744_refresh_headset_type,
};

static void value_initial(void)
{
	/* init all values */
	anc_priv->anc_hs_mode = ANC_HS_CHARGE_OFF;
	anc_priv->irq_flag = true;
	anc_priv->mic_used = false;
	anc_priv->sleep_time = ANC_CHIP_STARTUP_TIME;
	anc_priv->adc_calibration_base = 0;
	anc_priv->hs_micbias_ctl = ANC_HS_ENABLE_CHARGE;
	anc_priv->force_charge_ctl = ANC_HS_ENABLE_CHARGE;
	anc_priv->boost_flag = false;
	anc_priv->registered = false;
	anc_priv->headset_type = ANC_HS_NONE;
}

static int create_irq_workqueue(void)
{
	int ret;
	/* create irq workqueue */
	anc_priv->anc_hs_plugin_delay_wq =
		create_singlethread_workqueue("anc_hs_plugin_delay_wq");
	if (!(anc_priv->anc_hs_plugin_delay_wq)) {
		hwlog_err("%s : plugin wq create failed\n", __func__);
		return -ENOMEM;
	}
	INIT_DELAYED_WORK(&anc_priv->anc_hs_plugin_delay_work,
		anc_hs_plugin_work);

	anc_priv->anc_hs_plugout_delay_wq =
		create_singlethread_workqueue("anc_hs_plugout_delay_wq");
	if (!(anc_priv->anc_hs_plugout_delay_wq)) {
		hwlog_err("%s : plugout wq create failed\n", __func__);
		ret = -ENOMEM;
		goto err_plugin_delay_wq;
	}
	INIT_DELAYED_WORK(&anc_priv->anc_hs_plugout_delay_work,
		anc_hs_plugout_work);

	anc_priv->anc_hs_btn_delay_wq =
		create_singlethread_workqueue("anc_hs_btn_delay_wq");
	if (!(anc_priv->anc_hs_btn_delay_wq)) {
		hwlog_err("%s : btn wq create failed\n", __func__);
		ret = -ENOMEM;
		goto err_plugout_delay_wq;
	}
	INIT_DELAYED_WORK(&anc_priv->anc_hs_btn_delay_work, anc_hs_btn_work);

	anc_priv->anc_hs_invert_ctl_delay_wq =
		create_singlethread_workqueue("anc_hs_invert_ctl_delay_wq");
	if (!(anc_priv->anc_hs_invert_ctl_delay_wq)) {
		hwlog_err("%s : invert_ctl wq create failed\n", __func__);
		ret = -ENOMEM;
		goto err_invert_ctl_delay_wq;
	}
	INIT_DELAYED_WORK(&anc_priv->anc_hs_invert_ctl_delay_work,
		anc_hs_invert_ctl_work);

	return ret;

err_invert_ctl_delay_wq:
	if (anc_priv->anc_hs_btn_delay_wq) {
		cancel_delayed_work(&anc_priv->anc_hs_btn_delay_work);
		flush_workqueue(anc_priv->anc_hs_btn_delay_wq);
		destroy_workqueue(anc_priv->anc_hs_btn_delay_wq);
	}
err_plugout_delay_wq:
	if (anc_priv->anc_hs_plugout_delay_wq) {
		cancel_delayed_work(&anc_priv->anc_hs_plugout_delay_work);
		flush_workqueue(anc_priv->anc_hs_plugout_delay_wq);
		destroy_workqueue(anc_priv->anc_hs_plugout_delay_wq);
	}
err_plugin_delay_wq:
	if (anc_priv->anc_hs_plugin_delay_wq) {
		cancel_delayed_work(&anc_priv->anc_hs_plugin_delay_work);
		flush_workqueue(anc_priv->anc_hs_plugin_delay_wq);
		destroy_workqueue(anc_priv->anc_hs_plugin_delay_wq);
	}

	return ret;
}

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
	if (hs_load_gpio(dev, &(anc_priv->anc_pwr_en_gpio), 0,
		"gpio_pwr_en", 0, "anc_max14744_pwr_en", true) < 0)
		goto anc_get_gpio_pwr_en;

	if (hs_load_gpio(dev, &(anc_priv->gpio_int), 0,
		"gpio_int", 0, "anc_max14744_int", false) < 0)
		goto anc_get_gpio_int_err;

	return 0;

anc_get_gpio_int_err:
	if (anc_priv->anc_pwr_en_gpio > 0)
		gpio_free(anc_priv->anc_pwr_en_gpio);
anc_get_gpio_pwr_en:
	return -ENOENT;
}

static int get_ldo(struct device_node *np, const char *name)
{
	int ret;

	if (anc_priv->ldo_supply_used) {
		anc_priv->anc_hs_vdd = devm_regulator_get(anc_priv->dev, name);
		if (IS_ERR(anc_priv->anc_hs_vdd)) {
			hwlog_err("%s: can't get anc_hs_vdd regulator ip %pK\n",
				__func__, anc_priv->anc_hs_vdd);
			anc_priv->anc_hs_vdd = NULL;
			return -ENOENT;
		}

		ret = regulator_set_voltage(anc_priv->anc_hs_vdd, ANC_HS_VDD,
			ANC_HS_VDD);
		if (ret) {
			hwlog_err("%s: Couldn't set anc_hs_vdd 3.3 voltage\n",
				__func__);
			return -ENOENT;
		}

		ret = regulator_enable(anc_priv->anc_hs_vdd);
		if (ret) {
			hwlog_err("%s:Can't enable anc_hs_vdd ldo\n", __func__);
			return -ENOENT;
		}
	} else if (anc_priv->cam_ldo_used) {
		ret = of_property_read_u32(np, "cam_ldo_num",
			&anc_priv->cam_ldo_num);
		if (ret) {
			anc_priv->cam_ldo_num = -EINVAL;
			hwlog_info("%s fail get cam_ldo_num from device tree\n",
				__func__);
		}

		ret = of_property_read_u32(np, "cam_ldo_value",
			&anc_priv->cam_ldo_value);
		if (ret) {
			anc_priv->cam_ldo_value = -EINVAL;
			hwlog_info("%s fail get cam_ldo_value from dev tree\n",
				__func__);
		}

		if ((anc_priv->cam_ldo_num != -EINVAL) &&
			(anc_priv->cam_ldo_value != -EINVAL))
			hw_extern_pmic_config(anc_priv->cam_ldo_num,
				anc_priv->cam_ldo_value, 1);
	}

	return ret;
}

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

static int set_pinctrl(void)
{
	int ret;
	struct pinctrl *p = NULL;
	struct pinctrl_state *pinctrl_def = NULL;

	p = devm_pinctrl_get(anc_priv->dev);
	hwlog_info("max14744:node name is %s\n",
		anc_priv->dev->of_node->name);
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

static int anc_max14744_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	int val = 0;
	const char *chip_powered_on_time = "chip_powered_on_time";
	unsigned long flag = IRQF_ONESHOT | IRQF_NO_SUSPEND;
	struct anc_max14744_priv *di = NULL;
	struct device_node *np = NULL;

	hwlog_info("%s++\n", __func__);
	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	anc_priv = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;

	di->ldo_supply_used = read_property_value(di->dev, "ldo_supply_used");
	di->cam_ldo_used = read_property_value(di->dev, "cam_ldo_used");

	ret = get_ldo(np, "anc_hs_vdd");
	if (ret && di->ldo_supply_used)
		goto err_out;

	ret = of_property_read_u32(np, chip_powered_on_time, &val);
	if (ret) {
		hwlog_info("%s: fail to get chip_powered_on_time\n", __func__);
		val = CHIP_DEFAULT_POWERED_TIME;
	}
	mdelay(val);

	i2c_set_clientdata(client, di);
	di->regmap_l = regmap_init_i2c(client, &anc_max14744_regmap);
	if (IS_ERR(di->regmap_l)) {
		ret = PTR_ERR(di->regmap_l);
		hwlog_err("Failed to allocate regmap_l: %d\n", ret);
		goto err_out;
	}

	ret = anc_max14744_regmap_read(ANC_MAX14744_R000_DEVICE_ID, &val);
	if (ret < 0) {
		hwlog_err("max14744 chip is not exist, stop the chip init\n");
		ret = -ENXIO;
		goto err_out;
	}

	mutex_init(&di->charge_lock);
	mutex_init(&di->btn_mutex);
	mutex_init(&di->invert_hs_lock);
	spin_lock_init(&di->irq_lock);
	wakeup_source_init(&di->wake_lock, "anc_max14744");

	value_initial();

	load_anc_hs_config(np);

	ret = set_pinctrl();
	if (ret)
		goto err_out_init;
#ifdef ANC_MAX14744_DEBUG
	ret = sysfs_create_group(&client->dev.kobj, &anc_max14744_attr_group);
	if (ret < 0)
		hwlog_err("failed to register sysfs\n");
#endif
	ret = create_irq_workqueue();
	if (ret < 0)
		goto err_out_sysfs;

	ret = anc_hs_load_gpio_pdata(di->dev);
	if (ret < 0) {
		hwlog_err("%s get gpios failed, ret =%d\n", __func__, ret);
		goto err_btn_delay_wq;
	}

	di->anc_max14744_irq = gpio_to_irq(di->gpio_int);

	/* anc max14744 irq request */
	flag |= IRQF_TRIGGER_FALLING;
	ret = request_threaded_irq(di->anc_max14744_irq, NULL,
		anc_max14744_irq_handler, flag, "anc_max14744_irq", NULL);

	if (ret < 0) {
		hwlog_err("anc_max14744_irq request fail: ret = %d\n", ret);
		goto err_out_gpio;
	}

	pr_info("anc_max14744 gpio:%d,%d\n", di->gpio_int, di->anc_pwr_en_gpio);

	ret = anc_hs_ops_register(&anc_max14744_ops);
	if (ret) {
		hwlog_err("register anc_hs_interface ops failed\n");
		goto err_out_irq;
	}

	chip_init();

	/* register misc device for userspace */
	ret = misc_register(&anc_max14744_device);
	if (ret) {
		hwlog_err("%s:anc_max14744 misc dev register failed", __func__);
		goto err_out_irq;
	}


#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_ANC_MAX14744);
#endif

	hwlog_info("%s--\n", __func__);
	return 0;

err_out_irq:
	free_irq(di->anc_max14744_irq, NULL);
err_out_gpio:
	if (anc_priv->gpio_int > 0)
		gpio_free(di->gpio_int);
	if (anc_priv->anc_pwr_en_gpio > 0)
		gpio_free(anc_priv->anc_pwr_en_gpio);
err_btn_delay_wq:
	if (di->anc_hs_invert_ctl_delay_wq) {
		cancel_delayed_work(&di->anc_hs_invert_ctl_delay_work);
		flush_workqueue(di->anc_hs_invert_ctl_delay_wq);
		destroy_workqueue(di->anc_hs_invert_ctl_delay_wq);
	}
err_out_sysfs:
#ifdef ANC_MAX14744_DEBUG
	sysfs_remove_group(&client->dev.kobj, &anc_max14744_attr_group);
#endif
err_out_init:
	wakeup_source_trash(&di->wake_lock);
err_out:
	if (ret < 0) {
		if (di->regmap_l)
			regmap_exit(di->regmap_l);
	}
	devm_kfree(&client->dev, anc_priv);
	anc_priv = NULL;

	return ret;
}

static int anc_max14744_remove(struct i2c_client *client)
{
	struct anc_max14744_priv *di = i2c_get_clientdata(client);
	int ret;

	if (!di)
		return 0;

	if (di && di->regmap_l)
		regmap_exit(di->regmap_l);

	if (di->anc_hs_btn_delay_wq) {
		cancel_delayed_work(&di->anc_hs_btn_delay_work);
		flush_workqueue(di->anc_hs_btn_delay_wq);
		destroy_workqueue(di->anc_hs_btn_delay_wq);
	}

	if (di->anc_hs_plugout_delay_wq) {
		cancel_delayed_work(&di->anc_hs_plugout_delay_work);
		flush_workqueue(di->anc_hs_plugout_delay_wq);
		destroy_workqueue(di->anc_hs_plugout_delay_wq);
	}

	if (di->anc_hs_plugin_delay_wq) {
		cancel_delayed_work(&di->anc_hs_plugin_delay_work);
		flush_workqueue(di->anc_hs_plugin_delay_wq);
		destroy_workqueue(di->anc_hs_plugin_delay_wq);
	}

	if (di->anc_hs_invert_ctl_delay_wq) {
		cancel_delayed_work(&di->anc_hs_invert_ctl_delay_work);
		flush_workqueue(di->anc_hs_invert_ctl_delay_wq);
		destroy_workqueue(di->anc_hs_invert_ctl_delay_wq);
	}

	if (di->ldo_supply_used) {
		ret = regulator_disable(di->anc_hs_vdd);
		if (ret)
			hwlog_err("%s: disable anc hs ldo failed\n", __func__);
	}

	misc_deregister(&anc_max14744_device);
	kfree(anc_priv);
	anc_priv = NULL;
	hwlog_info("%s: exit\n", __func__);

	return 0;
}

static const struct of_device_id anc_max14744_of_match[] = {
	{
		.compatible = "huawei,anc_max14744",
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, anc_max14744_of_match);

static const struct i2c_device_id anc_max14744_i2c_id[] = {
	{ "anc_max14744", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, anc_max14744_i2c_id);

static struct i2c_driver anc_max14744_driver = {
	.probe                    = anc_max14744_probe,
	.remove                   = anc_max14744_remove,
	.id_table                 = anc_max14744_i2c_id,
	.driver = {
		.owner            = THIS_MODULE,
		.name             = "anc_max14744",
		.of_match_table   = of_match_ptr(anc_max14744_of_match),
	},
};

static int __init anc_max14744_init(void)
{
	int ret;

	ret = i2c_add_driver(&anc_max14744_driver);
	if (ret)
		hwlog_err("%s: i2c_add_driver error\n", __func__);

	return ret;
}

static void __exit anc_max14744_exit(void)
{
	i2c_del_driver(&anc_max14744_driver);
}

module_init(anc_max14744_init);
module_exit(anc_max14744_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("anc max14744 headset driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
