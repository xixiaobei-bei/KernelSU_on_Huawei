/*
 * da_combine_mbhc.c
 *
 * da_combine mbhc driver
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
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

#include <linux/hisi/da_combine/da_combine_mbhc.h>

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/of_device.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#ifdef CONFIG_SWITCH
#include <linux/switch.h>
#endif
#include <linux/version.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <sound/jack.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif
#include <linux/hisi/audio_log.h>
#include <linux/hisi/da_combine/da_combine_regs.h>
#include <linux/hisi/da_combine/da_combine_mbhc_rear_jack.h>
#include "huawei_platform/audio/anc_hs_interface.h"
#include "huawei_platform/audio/invert_hs.h"
#include "huawei_platform/audio/usb_analog_hs_interface.h"
#include "huawei_platform/audio/ana_hs_common.h"
#include "ana_hs_kit/ana_hs.h"
#include "asoc_adapter.h"
#include "hs_auto_calib.h"

#define LOG_TAG "DA_combine_mbhc"

#define JACK_BTN_MASK (SND_JACK_BTN_0 | SND_JACK_BTN_1 | SND_JACK_BTN_2 | SND_JACK_BTN_3)
#define CLR_IRQ_COMHL_ECO_STATUS        0x3F
#define PLUGIN_WAKE_DELAY_TIME_MS       15
#define MICBIAS_WAKEUP_TIME_MS          3500
#define MICBIAS_DELAYED_WORK_TIME_MS    3000
#define INTERVAL_THRESHOLDS_MS          1500
#define NS2MS                           1000000
#define GPIO_STATUS_JACK_HEADSET        0
#define GPIO_STATUS_JACK_INVERT         1
#define HS_CTRL_GPIO_MAX_NUM            2
#define BTN_UP_ECO_WAKE_LOCK_HZ         100
#define IRQ_NAME_LEN                    15
#define HOOK_KEY_MAP_ENABLE             1
#define HOOK_KEY_MAP_TYPE               0

#define HS_3_POLE_MIN_VOLTAGE           0
#define HS_3_POLE_MAX_VOLTAGE           8
#define HS_4_POLE_MIN_VOLTAGE           1150
#define HS_4_POLE_MAX_VOLTAGE           2600
#define BTN_PLAY_MIN_VOLTAGE            0
#define BTN_PLAY_MAX_VOLTAGE            100
#define BTN_VOLUME_UP_MIN_VOLTAGE       130
#define BTN_VOLUME_UP_MAX_VOLTAGE       320
#define BTN_VOLUME_DOWN_MIN_VOLTAGE     350
#define BTN_VOLUME_DOWN_MAX_VOLTAGE     700
#define HS_EXTERN_CABLE_MIN_VOLTAGE     2651
#define HS_EXTERN_CABLE_MAX_VOLTAGE     2700
#define INVALID_VOLTAGE                 0xFFFFFFFF

#define VOICE_ASSISTANT_VALUE           1
#define HS_CTRL_DAFULT_VALUEL           0x19
#define COEFFICIENT_DAFULT_VALUE        2800 /* saradc range 0 ~ 2800mV */
#define EXTERN_CABLE_VREF_DAFULT_VALUE  0x9E

struct hp_mic_switch {
	unsigned long cur_irq_ts;
	unsigned long pre_irq_ts;
};

struct hp_extern {
	struct hp_mic_switch ph_switch;
};

struct mbhc_hs_ctrl_gpio_cfg {
	unsigned int gpio;
	int jack_none_value;
	int jack_headset_value;
	int jack_invert_value;
};

struct mbhc_hs_type_ctrl {
	int gpio_num;
	struct gpio *gpio_array;
	struct mbhc_hs_ctrl_gpio_cfg gpio_cfg_array[HS_CTRL_GPIO_MAX_NUM];
};

struct request_irq {
	int irq_id;
	irq_handler_t handler;
	const char name[IRQ_NAME_LEN];
};

struct voltage_node_info {
	const char *min_node;
	const char *max_node;
	unsigned int min_def;
	unsigned int max_def;
};

struct cfg_node_info {
	const char *node;
	unsigned int def_val;
};

struct voltage_btn_report {
	const char *info;
	enum hs_voltage_type voltage_type;
	int btn_type;
};

struct jack_key_to_type {
	enum snd_jack_types type;
	int key;
};

/* defination of private data */
struct da_combine_mbhc_priv {
	struct da_combine_mbhc mbhc_pub;
	struct snd_soc_component *codec;
	struct da_combine_resmgr *resmgr;
	struct da_combine_irq *irqmgr;
	struct miscdevice miscdev;
	struct wakeup_source wake_lock;
	struct wakeup_source micbias_wake_lock;
	struct wakeup_source timeout_wake_lock;
	struct mutex plug_mutex;
	struct mutex status_mutex;
	struct mutex saradc_mutex;
	struct workqueue_struct *micbias_delay_wq;
	struct delayed_work micbias_delay_work;
	bool mbhc_micbias_work;
	bool hs_plug_status;
	bool anc_hs_plug_status;
	/* headset status */
	enum audio_jack_states hs_status;
	int btn_report;
	bool need_match_micbias;

#ifdef CONFIG_SWITCH
	struct switch_dev sdev;
#endif
	/* board defination */
	struct da_combine_mbhc_config mbhc_config;
	struct da_combine_hs_cfg hs_cfg;
	struct hp_extern hp_cfg;
	struct mbhc_hs_type_ctrl hs_type_ctrl;
};

#ifdef CONFIG_HUAWEI_DSM
#define MBHC_TYPE_FAIL_MAX_TIMES 5
#define MBHC_TYPE_REPORT_MAX_TIMES 20
static unsigned int g_mbhc_type_fail_times;
static unsigned int g_mbhc_type_report_times = MBHC_TYPE_REPORT_MAX_TIMES;
#endif


static struct da_combine_mbhc_priv *g_mbhc_priv;
static struct snd_soc_jack g_hs_jack;

static const int g_hs_irq_type_arr[] = {
	IRQ_BTNUP_COMP1,
	IRQ_BTNDOWN_COMP1,
	IRQ_BTNDOWN_COMP2,
	IRQ_BTNUP_COMP2,
	IRQ_BTNUP_ECO,
	IRQ_BTNDOWN_ECO,
	IRQ_PLUGIN,
	IRQ_PLUGOUT
};

static const int g_mbhc_btn_bits[] = {
	IRQ_BTNUP_COMP1,
	IRQ_BTNDOWN_COMP1,
	IRQ_BTNUP_COMP2,
	IRQ_BTNDOWN_COMP2,
	IRQ_BTNUP_ECO,
	IRQ_BTNDOWN_ECO
};

#ifdef CONFIG_HUAWEI_DSM
static void mbhc_dmd_fail_report(int adc)
{
	g_mbhc_type_fail_times++;
	if ((g_mbhc_type_fail_times >= MBHC_TYPE_FAIL_MAX_TIMES) &&
		(g_mbhc_type_report_times > 0)) {
		g_mbhc_type_fail_times = 0;
		g_mbhc_type_report_times--;

		audio_dsm_report_info(AUDIO_CODEC, DSM_HI6402_MBHC_HS_ERR_TYPE,
			"abnormal headset type! adc = [%d], total times = [%u]\n",
			adc,
			(MBHC_TYPE_REPORT_MAX_TIMES - g_mbhc_type_report_times) *
			MBHC_TYPE_FAIL_MAX_TIMES);
	}
}
#endif

static void report_soc_jack(int report, int mask)
{
	snd_soc_jack_report(&g_hs_jack, report, mask);
}

static void micbias_work_func(struct work_struct *work)
{
	struct da_combine_mbhc_priv *priv = container_of(work, struct da_combine_mbhc_priv,
		micbias_delay_work.work);

	da_combine_resmgr_release_micbias(priv->resmgr);
}

static void hs_micbias_enable(struct da_combine_mbhc_priv *priv, bool enable)
{
	int ret;

	if (enable) {
		da_combine_resmgr_request_micbias(priv->resmgr);
		return;
	}

	__pm_wakeup_event(&priv->micbias_wake_lock, MICBIAS_WAKEUP_TIME_MS);
	ret = mod_delayed_work(priv->micbias_delay_wq,
		&priv->micbias_delay_work,
		msecs_to_jiffies(MICBIAS_DELAYED_WORK_TIME_MS));
	if (ret != 0) {
		AUDIO_LOGE("mod delayed work error");
		da_combine_resmgr_release_micbias(priv->resmgr);
	}
}

static void usb_ana_hs_micbias_enable(struct da_combine_mbhc_priv *priv,
	bool enable)
{
	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	if (enable)
		da_combine_resmgr_request_micbias(priv->resmgr);
	else
		da_combine_resmgr_release_micbias(priv->resmgr);
}

static int get_hs_ctrl_gpio_cfg(struct da_combine_mbhc_priv *priv,
	const struct device_node *node)
{
	struct mbhc_hs_type_ctrl *hs_ctrl = &priv->hs_type_ctrl;
	struct mbhc_hs_ctrl_gpio_cfg gpio_cfg_array[HS_CTRL_GPIO_MAX_NUM] = {{0}};
	int gpio_cfg_elems_num = sizeof(gpio_cfg_array[0]) / sizeof(int);
	int of_elems_num = of_property_count_u32_elems(node, "hisilicon,hs_ctrl_gpio_cfg");
	int gpio_num = of_elems_num / gpio_cfg_elems_num;
	int status;
	int i;

	if ((of_elems_num % gpio_cfg_elems_num) != 0) {
		AUDIO_LOGE("read of elems num invalid with value %d", of_elems_num);
		return -EINVAL;
	}

	if ((gpio_num <= 0) || (gpio_num > HS_CTRL_GPIO_MAX_NUM)) {
		AUDIO_LOGE("gpio num invalid with value %d", gpio_num);
		return -EINVAL;
	}

	status = of_property_read_u32_array(node, "hisilicon,hs_ctrl_gpio_cfg",
		(unsigned int *)gpio_cfg_array, of_elems_num);
	if (status != 0) {
		AUDIO_LOGE("read gpio config failed");
		return -EINVAL;
	}

	for (i = 0; i < gpio_num; i++) {
		if (gpio_is_valid(gpio_cfg_array[i].gpio) == 0) {
			AUDIO_LOGE("gpio pin: %u is invalid",
				gpio_cfg_array[i].gpio);
			return -EIO;
		}

		AUDIO_LOGI("read gpio: %u, jack none value is: %d",
			gpio_cfg_array[i].gpio,
			gpio_cfg_array[i].jack_none_value);

		AUDIO_LOGI("jack headset value is: %d, jack invert value is: %d",
			gpio_cfg_array[i].jack_headset_value,
			gpio_cfg_array[i].jack_invert_value);
		/*
		 * normalize below value to 0 and 1
		 * the gpio set function only cares if the value is 0 or 1
		 */
		gpio_cfg_array[i].jack_none_value =
			!!gpio_cfg_array[i].jack_none_value;
		gpio_cfg_array[i].jack_headset_value =
			!!gpio_cfg_array[i].jack_headset_value;
		gpio_cfg_array[i].jack_invert_value =
			!!gpio_cfg_array[i].jack_invert_value;
	}

	memcpy(hs_ctrl->gpio_cfg_array, gpio_cfg_array, sizeof(hs_ctrl->gpio_cfg_array));
	hs_ctrl->gpio_num = gpio_num;

	return 0;
}

static void headset_type_ctrl_gpio_init(struct da_combine_mbhc_priv *priv,
	const struct device_node *node)
{
	struct mbhc_hs_type_ctrl *hs_ctrl = &priv->hs_type_ctrl;
	struct gpio *gpio_array = NULL;
	int ret;
	int i;

	ret = get_hs_ctrl_gpio_cfg(priv, node);
	if (ret != 0) {
		AUDIO_LOGW("get hs ctrl gpio cfg failed with value %d", ret);
		return;
	}

	gpio_array = kzalloc(hs_ctrl->gpio_num * sizeof(*gpio_array), GFP_KERNEL);
	if (gpio_array == NULL) {
		AUDIO_LOGE("kzalloc failed");
		return;
	}

	for (i = 0; i < hs_ctrl->gpio_num; ++i) {
		gpio_array[i].gpio = hs_ctrl->gpio_cfg_array[i].gpio;
		gpio_array[i].label = "da_combine headset type ctrl";

		if (hs_ctrl->gpio_cfg_array[i].jack_none_value)
			gpio_array[i].flags = GPIOF_OUT_INIT_HIGH;
		else
			gpio_array[i].flags = GPIOF_OUT_INIT_LOW;
	}

	ret = gpio_request_array(gpio_array, hs_ctrl->gpio_num);
	if (ret != 0) {
		AUDIO_LOGE("gpio request array failed with error %d", ret);
		kfree(gpio_array);
		return;
	}

	hs_ctrl->gpio_array = gpio_array;
	AUDIO_LOGI("ok");
}

static void headset_type_ctrl_gpio_deinit(struct da_combine_mbhc_priv *priv)
{
	struct mbhc_hs_type_ctrl *hs_ctrl = &priv->hs_type_ctrl;

	if (hs_ctrl->gpio_array == NULL) {
		AUDIO_LOGW("headset type control is disable");
		return;
	}

	gpio_free_array(hs_ctrl->gpio_array, hs_ctrl->gpio_num);
	kfree(hs_ctrl->gpio_array);
	hs_ctrl->gpio_array = NULL;
}

static int set_gpio_value(int status, const struct da_combine_mbhc_priv *priv)
{
	const struct mbhc_hs_type_ctrl *hs_ctrl = &priv->hs_type_ctrl;
	const struct mbhc_hs_ctrl_gpio_cfg *gpio_cfg_array = NULL;
	int gpio_value;
	int i;

	if (hs_ctrl->gpio_array == NULL) {
		AUDIO_LOGW("headset type control is disable");
		return 0;
	}

	gpio_cfg_array = hs_ctrl->gpio_cfg_array;

	for (i = 0; i < hs_ctrl->gpio_num; ++i) {
		if (status == AUDIO_JACK_NONE) {
			gpio_value = gpio_cfg_array[i].jack_none_value;
		} else if (status == AUDIO_JACK_HEADSET) {
			gpio_value = gpio_cfg_array[i].jack_headset_value;
		} else if (status == AUDIO_JACK_INVERT) {
			gpio_value = gpio_cfg_array[i].jack_invert_value;
		} else {
			AUDIO_LOGE("headset jack status is invalid with value %d", status);
			return -EINVAL;
		}

		gpio_set_value_cansleep(gpio_cfg_array[i].gpio, gpio_value);
		AUDIO_LOGI("set gpio %u to %d", gpio_cfg_array[i].gpio, gpio_value);
	}

	return 0;
}

static void set_audio_jack_none(const struct da_combine_mbhc *mbhc)
{
	const struct da_combine_mbhc_priv *priv = (const struct da_combine_mbhc_priv *)mbhc;
	int ret;

	ret = set_gpio_value(AUDIO_JACK_NONE, priv);
	if (ret != 0)
		AUDIO_LOGE("set gpio error, ret: %d", ret);
}

static void set_4_pole_headset_type(struct da_combine_mbhc_priv *priv)
{
	struct mbhc_hs_type_ctrl *hs_ctrl = &priv->hs_type_ctrl;
	struct hp_mic_switch *hp_cfg = &priv->hp_cfg.ph_switch;
	struct da_combine_mbhc *mbhc = (struct da_combine_mbhc *)priv;
	unsigned int interval_ms = 0;

	if (hs_ctrl->gpio_array == NULL) {
		AUDIO_LOGW("headset type control is disable");
		return;
	}

	if (hp_cfg->pre_irq_ts == 0) {
		hp_cfg->pre_irq_ts = hisi_getcurtime();
	} else {
		hp_cfg->cur_irq_ts = hisi_getcurtime();
		interval_ms = (hp_cfg->cur_irq_ts - hp_cfg->pre_irq_ts) / NS2MS;
		hp_cfg->pre_irq_ts = hp_cfg->cur_irq_ts;
	}

	AUDIO_LOGI("headset jack status: %d, interval ms: %u",
		priv->hs_status, interval_ms);

	switch (priv->hs_status) {
	case AUDIO_JACK_HEADSET:
		if ((interval_ms > INTERVAL_THRESHOLDS_MS) ||
			(interval_ms == 0))
			AUDIO_LOGI("current plug in type is headset");
		break;
	case AUDIO_JACK_INVERT:
		da_combine_set_audio_jack_invert(mbhc);
		break;
	case AUDIO_JACK_NONE:
		set_audio_jack_none(mbhc);
		break;
	default:
		AUDIO_LOGE("unsupported headset status, status is: %d",
			priv->hs_status);
		break;
	}
}

static void mbhc_jack_report(struct da_combine_mbhc_priv *priv)
{
#ifdef CONFIG_SWITCH
	enum audio_jack_states jack_status = priv->hs_status;
#endif
	int jack_report = 0;

	switch (priv->hs_status) {
	case AUDIO_JACK_NONE:
		jack_report = 0;
		AUDIO_LOGI("plug out");
		set_4_pole_headset_type(priv);
		break;
	case AUDIO_JACK_HEADSET:
		jack_report = SND_JACK_HEADSET;
		AUDIO_LOGI("4-pole headset plug in");
		set_4_pole_headset_type(priv);
		break;
	case AUDIO_JACK_INVERT:
		jack_report = SND_JACK_HEADPHONE;
		AUDIO_LOGI("invert headset plug in");
		set_4_pole_headset_type(priv);
		break;
	case AUDIO_JACK_HEADPHONE:
		jack_report = SND_JACK_HEADPHONE;
		AUDIO_LOGI("3-pole headphone plug in");
		break;
	case AUDIO_JACK_EXTERN_CABLE:
		jack_report = 0;
		AUDIO_LOGI("extern cable plug in, jack report[%d]", jack_report);
		break;
	default:
		AUDIO_LOGE("error hs status: %d", priv->hs_status);
		break;
	}

	/* clear btn event */
	report_soc_jack(0, JACK_BTN_MASK);

	/* btn report jack status */
	if (priv->mbhc_config.hs_report_line_in_out) {
		jack_report |= (SND_JACK_LINEIN | SND_JACK_LINEOUT);
		report_soc_jack(jack_report,
			(SND_JACK_HEADSET | SND_JACK_LINEIN | SND_JACK_LINEOUT));
	} else {
		report_soc_jack(jack_report, SND_JACK_HEADSET);
	}


#ifdef CONFIG_SWITCH
	switch_set_state(&priv->sdev, jack_status);
#endif
}

static inline bool is_headset_pluged_in(struct da_combine_mbhc_priv *priv)
{
	unsigned int irq_source_reg = priv->hs_cfg.mbhc_reg->irq_source_reg;
#ifdef CONFIG_ANC_HS_INTERFACE
	int ret;
#endif
	if (priv->mbhc_config.analog_hs_unsupport)
		return false;

	if (check_usb_analog_hs_support() || ana_hs_support_usb_sw()) {
		if ((usb_analog_hs_check_headset_pluged_in() == ANA_HS_PLUG_IN) ||
			(ana_hs_pluged_state() == ANA_HS_PLUG_IN)) {
			AUDIO_LOGI("ananlog hs is plug in");
			return true;
		}

		AUDIO_LOGI("ananlog hs is plug out");
		return false;
	}
#ifdef CONFIG_ANC_HS_INTERFACE
	ret = anc_hs_interface_check_headset_pluged_in();
	if (ret == NO_MAX14744) {
		/*
		 * 0 : means headset is pluged out
		 * 1 : means headset is pluged in
		 */
		AUDIO_LOGI("headset plug in ret: %d", ret);
		return ((snd_soc_component_read32(priv->codec, irq_source_reg) &
			BIT(DA_COMBINE_PLUGIN_IRQ_BIT)) != 0);
	}

	if (ret == HANDSET_PLUG_IN) {
		AUDIO_LOGI("headset plug in");
		return true;
	}

	AUDIO_LOGI("headset plug out");
	return false;
#else
	return ((snd_soc_component_read32(priv->codec, irq_source_reg) &
		BIT(DA_COMBINE_PLUGIN_IRQ_BIT)) != 0);
#endif
}

static bool is_voltage_in_range(const struct da_combine_mbhc_priv *priv,
	unsigned int voltage, enum hs_voltage_type type)
{
	if ((priv->mbhc_config.voltage[type].min <= voltage) &&
		(priv->mbhc_config.voltage[type].max >= voltage))
		return true;

	return false;
}

static unsigned int get_voltage(struct da_combine_mbhc_priv *priv)
{
	struct hs_mbhc_func *mbhc_func = priv->hs_cfg.mbhc_func;
	unsigned int val;

	if (mbhc_func->hs_get_voltage == NULL) {
		AUDIO_LOGE("cannot get voltage value");
		return 0;
	}

	mutex_lock(&priv->saradc_mutex);
	val = mbhc_func->hs_get_voltage(priv->codec,
		priv->mbhc_config.hs_cfg[HS_COEFFICIENT]);
	mutex_unlock(&priv->saradc_mutex);

	return val;
}

static void get_hstype_by_voltage(struct da_combine_mbhc_priv *priv,
	int *headset_type, unsigned int voltage_value)
{
	if (priv->mbhc_config.voltage[VOLTAGE_POLE3].max >= voltage_value) {
		/* 3-pole headphone */
		AUDIO_LOGI("3 pole is pluged in");
		priv->hs_status = AUDIO_JACK_HEADPHONE;
		*headset_type = ANA_HS_NORMAL_3POLE;
	} else if (is_voltage_in_range(priv, voltage_value, VOLTAGE_POLE4)) {
		/* 4-pole headset */
		AUDIO_LOGI("4 pole is pluged in");
		priv->hs_status = AUDIO_JACK_HEADSET;
		*headset_type = ANA_HS_NORMAL_4POLE;
	} else if (priv->mbhc_config.hs_detect_extern_cable &&
		is_voltage_in_range(priv, voltage_value, VOLTAGE_EXTERN_CABLE)) {
		AUDIO_LOGI("set as extern cable");
		priv->hs_status = AUDIO_JACK_EXTERN_CABLE;
		*headset_type = ANA_HS_REVERT_4POLE;
	} else {
		/* invert 4-pole headset */
		AUDIO_LOGI("need further detect, report as 3-pole, voltage: %u",
			voltage_value);
		priv->hs_status = AUDIO_JACK_INVERT;
		*headset_type = ANA_HS_REVERT_4POLE;

#ifdef CONFIG_INVERT_HS
		/* real invert headset */
		if (priv->mbhc_config.voltage[VOLTAGE_POLE4].min > voltage_value) {
			invert_hs_control(INVERT_HS_MIC_GND_CONNECT);
			ana_hs_invert_hs_control(INVERT_HS_MIC_GND_CONNECT);
		}
#endif
	}
}

static int check_plug_in_detect_para(const struct da_combine_mbhc_priv *priv)
{
	if (priv == NULL) {
		AUDIO_LOGE("priv is not exit");
		return -EINVAL;
	}

	if (priv->hs_cfg.mbhc_func == NULL) {
		AUDIO_LOGE("mbhc func is not exit");
		return -EINVAL;
	}

	if (priv->hs_cfg.mbhc_func->hs_mbhc_on == NULL) {
		AUDIO_LOGE("mbhc on func is not exit");
		return -EINVAL;
	}

	if (priv->hs_cfg.res_detect_func == NULL) {
		AUDIO_LOGE("res detect on func is not exit");
		return -EINVAL;
	}

	return 0;
}

static void set_default_jack(const struct da_combine_mbhc_priv *priv)
{
	if (priv->mbhc_config.hs_support_positive_invert_switch) {
		if (priv->hs_status == AUDIO_JACK_NONE) {
			da_combine_set_audio_jack_headset((const struct da_combine_mbhc *)priv);
			AUDIO_LOGI("set as 4-pole positive headset");
		}
	}
}

static void detect_hs_type(struct da_combine_mbhc_priv *priv, int *headset_type,
	unsigned int *voltage, const struct hs_mbhc_func *mbhc_func)
{
	unsigned int val;

	hs_micbias_enable(priv, true);

	mbhc_func->hs_mbhc_on(priv->codec);

	/* get voltage by read sar in mbhc */
	val = get_voltage(priv);

	mutex_lock(&priv->status_mutex);

	get_hstype_by_voltage(priv, headset_type, val);

	mutex_unlock(&priv->status_mutex);

	*voltage = val;
}

static void handle_anc_hs(struct da_combine_mbhc_priv *priv,
	unsigned int voltage, int type)
{
#ifdef CONFIG_ANC_HS_INTERFACE
	if (check_anc_hs_interface_support() &&
		(priv->hs_status == AUDIO_JACK_HEADSET)) {
		/* mask btn irqs while control boost */
		da_combine_irq_mask_btn_irqs(&priv->mbhc_pub);

		priv->anc_hs_plug_status =
			anc_hs_interface_charge_detect(voltage, type);

		da_combine_irq_unmask_btn_irqs(&priv->mbhc_pub);
	}

	anc_hs_interface_refresh_headset_type(type);
	/* real invert headset */
	if ((priv->mbhc_config.voltage[VOLTAGE_POLE4].min > voltage) &&
		(priv->hs_status == AUDIO_JACK_INVERT))
		anc_hs_interface_invert_hs_control(ANC_HS_MIC_GND_CONNECT);
#endif
	ana_hs_refresh_headset_type(type);
}

static void handle_usb_analog_hs(struct da_combine_mbhc_priv *priv,
	int *headset_type, unsigned int *voltage)
{
	unsigned int val;

	/* old version, should be deleted when not used */
	if ((check_usb_analog_hs_support() || ana_hs_support_usb_sw()) &&
		(priv->hs_status == AUDIO_JACK_INVERT ||
		priv->hs_status == AUDIO_JACK_HEADPHONE)) {
		/*
		 * before change the mic/gnd, power down the MICBIAS
		 * to avoid pop sound in hs
		 */
		usb_ana_hs_micbias_enable(priv, false);
		da_combine_irq_mask_btn_irqs(&priv->mbhc_pub);
		usb_ana_hs_mic_switch_change_state();
		ana_hs_mic_gnd_swap();
		da_combine_irq_unmask_btn_irqs(&priv->mbhc_pub);

		/*
		 * after change the mic/gnd, power on the MICBIAS
		 * to identify the hs type
		 */
		usb_ana_hs_micbias_enable(priv, true);
		msleep(30);

		val = get_voltage(priv);
		mutex_lock(&priv->status_mutex);
		get_hstype_by_voltage(priv, headset_type, val);
		mutex_unlock(&priv->status_mutex);

		*voltage = val;
	}
}

static void hs_res_detect(struct da_combine_mbhc_priv *priv)
{
	struct hs_res_detect_func *res_detect_func =
		priv->hs_cfg.res_detect_func;

	if ((res_detect_func->hs_res_detect != NULL) &&
		(res_detect_func->hs_path_enable != NULL) &&
		(res_detect_func->hs_path_disable != NULL) &&
		(!priv->hs_plug_status) &&
		(!priv->anc_hs_plug_status) &&
		(priv->hs_status != AUDIO_JACK_INVERT)) {
		da_combine_irq_mask_btn_irqs(&priv->mbhc_pub);
		res_detect_func->hs_path_enable(priv->codec);
		res_detect_func->hs_res_detect(priv->codec);
		res_detect_func->hs_path_disable(priv->codec);
		da_combine_irq_unmask_btn_irqs(&priv->mbhc_pub);
		AUDIO_LOGI("hs res detect");
	} else {
		AUDIO_LOGI("no need enable res detect, hs plug: %d, anc hs plug: %d",
			priv->hs_plug_status, priv->anc_hs_plug_status);
	}
}

static void report_detection_result(struct da_combine_mbhc_priv *priv,
	unsigned int voltage)
{
	if (priv->mbhc_config.voltage[VOLTAGE_POLE4].max > voltage) {
		mbhc_jack_report(priv);
		priv->hs_plug_status = true;

		if (priv->mbhc_config.hs_detect_extern_cable)
			AUDIO_LOGI("not turn off hs micbias for extern cable");
		else
			hs_micbias_enable(priv, false);
	} else {
		if (priv->mbhc_config.hs_detect_extern_cable) {
			priv->hs_plug_status = false;
			priv->need_match_micbias = false;
			mbhc_jack_report(priv);
		} else {
			priv->need_match_micbias = true;
#ifdef CONFIG_HUAWEI_DSM
			mbhc_dmd_fail_report(voltage);
#endif
		}
	}
}

static void plug_in_detect(struct da_combine_mbhc_priv *priv)
{
	unsigned int voltage = 0;
	int headset_type = ANA_HS_REVERT_4POLE;
	struct hs_mbhc_func *mbhc_func = NULL;

	if (check_plug_in_detect_para(priv) != 0)
		return;

	mbhc_func = priv->hs_cfg.mbhc_func;

	if (!is_headset_pluged_in(priv))
		return;

	set_default_jack(priv);

	__pm_stay_awake(&priv->wake_lock);
	mutex_lock(&priv->plug_mutex);

	AUDIO_LOGD("in");

#ifdef CONFIG_INVERT_HS
	invert_hs_control(INVERT_HS_MIC_GND_DISCONNECT);
	ana_hs_invert_hs_control(INVERT_HS_MIC_GND_DISCONNECT);
#endif

#ifdef CONFIG_ANC_HS_INTERFACE
	if (check_anc_hs_interface_support()) {
		/* mask btn irqs while control boost */
		da_combine_irq_mask_btn_irqs(&priv->mbhc_pub);
		anc_hs_interface_start_charge();
	}
#endif

	detect_hs_type(priv, &headset_type, &voltage, mbhc_func);

	handle_anc_hs(priv, voltage, headset_type);

	handle_usb_analog_hs(priv, &headset_type, &voltage);

	/*
	 * da_combine_v3 & first plugin detect & not invert headphone & not anc headphone
	 * then headphone res will be detected
	 */
	hs_res_detect(priv);

	if (priv->need_match_micbias == true) {
		hs_micbias_enable(priv, false);
		priv->need_match_micbias = false;
	}

	if (!is_headset_pluged_in(priv)) {
		AUDIO_LOGI("headset has been pluged out");
		goto exit;
	}

	report_detection_result(priv, voltage);

exit:
	mutex_unlock(&priv->plug_mutex);
	__pm_wakeup_event(&priv->timeout_wake_lock, PLUGIN_WAKE_DELAY_TIME_MS);
	__pm_relax(&priv->wake_lock);
}

static const struct voltage_btn_report g_btn_report_map[] = {
	{ "btn up", VOLTAGE_POLE4, 0 },
	{ "btn play", VOLTAGE_PLAY, SND_JACK_BTN_0 },
	{ "volume up", VOLTAGE_VOL_UP, SND_JACK_BTN_1 },
	{ "volume down", VOLTAGE_VOL_DOWN, SND_JACK_BTN_2 },
	{ "voice assistant", VOLTAGE_VOICE_ASSIST, SND_JACK_BTN_3 },
};

static int get_btn_report(struct da_combine_mbhc_priv *priv, unsigned int voltage)
{
	unsigned int i;
	unsigned int size = ARRAY_SIZE(g_btn_report_map);

	for (i = 0; i < size; i++) {
		if (is_voltage_in_range(priv, voltage, g_btn_report_map[i].voltage_type)) {
			AUDIO_LOGI("process as %s", g_btn_report_map[i].info);
			mutex_lock(&priv->status_mutex);
			priv->btn_report = g_btn_report_map[i].btn_type;
			mutex_unlock(&priv->status_mutex);

			if (g_btn_report_map[i].voltage_type == VOLTAGE_VOICE_ASSIST)
				return VOICE_ASSISTANT_VALUE;

			return 0;
		}
	}

	msleep(600);
	plug_in_detect(priv);

	return -EINVAL;
}

static void btn_down(struct da_combine_mbhc_priv *priv)
{
	unsigned int voltage;
	int ret;

	if (!is_headset_pluged_in(priv)) {
		AUDIO_LOGI("hs pluged out");
		return;
	}

	__pm_stay_awake(&priv->wake_lock);

	if (priv->hs_status == AUDIO_JACK_HEADSET) {
		hs_micbias_enable(priv, true);

		voltage = get_voltage(priv);

		if (priv->mbhc_config.hs_detect_extern_cable)
			AUDIO_LOGI("not turn off mbhc micbias for extern cable");
		else
			hs_micbias_enable(priv, false);

		msleep(30);

		if (!is_headset_pluged_in(priv)) {
			AUDIO_LOGI("hs pluged out");
			goto end;
		}

		ret = get_btn_report(priv, voltage);
		if (ret != 0) {
			if (ret == VOICE_ASSISTANT_VALUE)
				goto voice_assistant_key;

			goto end;
		}

		if (!is_headset_pluged_in(priv)) {
			AUDIO_LOGI("hs pluged out");
			goto end;
		}

		startup_fsm(REC_JUDGE, voltage, &(priv->btn_report));

voice_assistant_key:
		/* btn report key event */
		AUDIO_LOGI("btn report type: %d, status: %d",
			priv->btn_report, priv->hs_status);
		report_soc_jack(priv->btn_report, JACK_BTN_MASK);
	}

end:
	__pm_relax(&priv->wake_lock);
}

static irqreturn_t plug_in_handler(int irq, void *data)
{
	struct da_combine_mbhc_priv *priv = data;

	WARN_ON(priv == NULL);

	msleep(300);

	plug_in_detect(priv);

	return IRQ_HANDLED;
}

static void plug_out(struct da_combine_mbhc_priv *priv)
{
	if (priv->hs_cfg.mbhc_func->hs_mbhc_off == NULL) {
		AUDIO_LOGE("mbhc off func is not exit");
		return;
	}

	AUDIO_LOGD("in");

	mutex_lock(&priv->plug_mutex);

	cancel_delayed_work(&priv->micbias_delay_work);
	flush_workqueue(priv->micbias_delay_wq);

	da_combine_irq_mask_btn_irqs(&priv->mbhc_pub);

#ifdef CONFIG_INVERT_HS
	invert_hs_control(INVERT_HS_MIC_GND_CONNECT);
	ana_hs_invert_hs_control(INVERT_HS_MIC_GND_CONNECT);
#endif

	/* stop charge first */
#ifdef CONFIG_ANC_HS_INTERFACE
	anc_hs_interface_stop_charge();
#endif

	da_combine_resmgr_force_release_micbias(priv->resmgr);
	priv->need_match_micbias = false;

	da_combine_irq_mask_btn_irqs(&priv->mbhc_pub);
	priv->hs_cfg.mbhc_func->hs_mbhc_off(priv->codec);

	mutex_lock(&priv->status_mutex);
	priv->hs_status = AUDIO_JACK_NONE;
	priv->btn_report = 0;
	mutex_unlock(&priv->status_mutex);

	headset_auto_calib_reset_interzone();

	mbhc_jack_report(priv);
	priv->hs_plug_status = false;

	mutex_unlock(&priv->plug_mutex);
}

static void plug_out_detect(struct da_combine_mbhc_priv *priv)
{
	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}
	if (is_headset_pluged_in(priv)) {
		AUDIO_LOGI("headset still plugin");
		return;
	}

	plug_out(priv);
}

static irqreturn_t plug_out_handler(int irq, void *data)
{
	struct da_combine_mbhc_priv *priv = data;

	WARN_ON(priv == NULL);

	plug_out_detect(priv);

	return IRQ_HANDLED;
}

static irqreturn_t btn_up_handler(int irq, void *data)
{
	struct da_combine_mbhc_priv *priv = data;

	AUDIO_LOGI("enter");

	if (!is_headset_pluged_in(priv))
		return IRQ_HANDLED;

	if (priv->mbhc_config.hs_detect_extern_cable &&
		priv->hs_status == AUDIO_JACK_EXTERN_CABLE) {
		AUDIO_LOGI("for extern cable return");
		return IRQ_HANDLED;
	}

	if (priv->hs_status == AUDIO_JACK_INVERT) {
		AUDIO_LOGI("further detect");
		msleep(600);
		plug_in_detect(priv);
	} else if (priv->btn_report == 0) {
		if (priv->hs_status != AUDIO_JACK_HEADSET) {
			AUDIO_LOGI("further detect");
			msleep(600);
			plug_in_detect(priv);
		}
	} else {
		mutex_lock(&priv->status_mutex);
		priv->btn_report = 0;
		report_soc_jack(priv->btn_report, JACK_BTN_MASK);
		mutex_unlock(&priv->status_mutex);
		AUDIO_LOGI("btn up");
	}

	return IRQ_HANDLED;
}

static irqreturn_t btn_down_handler(int irq, void *data)
{
	struct da_combine_mbhc_priv *priv = data;

	AUDIO_LOGI("btn down");

	btn_down(priv);

	return IRQ_HANDLED;
}

static irqreturn_t btn_comp2_handler(int irq, void *data,
	enum audio_jack_states status)
{
	struct da_combine_mbhc_priv *priv = data;
	struct snd_soc_component *codec = priv->codec;

	if (priv->hs_status == status) {
		plug_in_detect(priv);
	} else if (priv->mbhc_config.hs_detect_extern_cable) {
		da_combine_irq_mask_btn_irqs(&priv->mbhc_pub);
		plug_in_detect(priv);
		da_combine_irq_unmask_btn_irqs(&priv->mbhc_pub);
		if (priv->mbhc_config.hs_cfg[HS_IRQ_REG0]) {
			msleep(30);
			snd_soc_component_write(codec, priv->mbhc_config.hs_cfg[HS_IRQ_REG0],
				CLR_IRQ_COMHL_ECO_STATUS);
		}
	} else {
		AUDIO_LOGI("need do nothing");
	}

	return IRQ_HANDLED;
}

static irqreturn_t btn_up_comp2_handler(int irq, void *data)
{
	AUDIO_LOGI("btn up comp2");

	return btn_comp2_handler(irq, data, AUDIO_JACK_HEADSET);
}

static irqreturn_t btn_down_comp2_handler(int irq, void *data)
{
	AUDIO_LOGI("btn down comp2");

	return btn_comp2_handler(irq, data, AUDIO_JACK_INVERT);
}

static irqreturn_t btn_up_eco_handler(int irq, void *data)
{
	struct da_combine_mbhc_priv *priv = data;

	if (!is_headset_pluged_in(priv))
		return IRQ_HANDLED;

	if (priv->mbhc_config.hs_detect_extern_cable &&
		priv->hs_status == AUDIO_JACK_EXTERN_CABLE) {
		AUDIO_LOGI("for extern cable return");
		return IRQ_HANDLED;
	}

	__pm_wakeup_event(&priv->wake_lock,
		jiffies_to_msecs(BTN_UP_ECO_WAKE_LOCK_HZ));

	if (priv->hs_status == AUDIO_JACK_INVERT) {
		AUDIO_LOGE("further detect");
		msleep(600);
		plug_in_detect(priv);
	} else if (priv->btn_report == 0) {
		if (priv->hs_status != AUDIO_JACK_HEADSET) {
			AUDIO_LOGE("further detect");
			msleep(600);
			plug_in_detect(priv);
		}
	} else {
		mutex_lock(&priv->status_mutex);
		priv->btn_report = 0;
		report_soc_jack(priv->btn_report, JACK_BTN_MASK);
		mutex_unlock(&priv->status_mutex);
		AUDIO_LOGI("btn up");
	}

	return IRQ_HANDLED;
}

static irqreturn_t btn_down_eco_handler(int irq, void *data)
{
	struct da_combine_mbhc_priv *priv = data;

	AUDIO_LOGI("btn down");

	btn_down(priv);

	return IRQ_HANDLED;
}

#ifdef CONFIG_ANC_HS_INTERFACE
static void check_axi_bus_reg_value(struct snd_soc_component *codec,
	unsigned int reg, unsigned int mask, unsigned int except_val)
{
	unsigned int val;

	val = snd_soc_component_read32(codec, reg);
	if ((val & mask) != except_val)
		AUDIO_LOGE("AXI bus error, reg [%pK] value: %u", (void *)(uintptr_t)reg, val);
}

static void mbhc_check_bus_status(void *priv)
{
	struct da_combine_mbhc_priv *data = priv;

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	/* check the write register's status */
	check_axi_bus_reg_value(data->codec, DA_COMBINE_REG_WRITE_DSP_STATUS,
		DA_COMBINE_WRITE_DSP_STATUS_BIT_MUSK, 0);
	/* check the read register's status */
	check_axi_bus_reg_value(data->codec, DA_COMBINE_REG_READ_DSP_STATUS,
		DA_COMBINE_READ_DSP_STATUS_BIT_MUSK, 0);
	/* check AXI dlock irq status */
	check_axi_bus_reg_value(data->codec, DA_COMBINE_REG_AXI_DLOCK_IRQ_1,
		DA_COMBINE_AXI_DLOCK_IRQ_BIT_MUSK_1, 0);
	check_axi_bus_reg_value(data->codec, DA_COMBINE_REG_AXI_DLOCK_IRQ_2,
		DA_COMBINE_AXI_DLOCK_IRQ_BIT_MUSK_2, 0);
}
#endif

static bool mbhc_check_headset_in(void *priv)
{
	struct da_combine_mbhc_priv *data = priv;

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return false;
	}

	return is_headset_pluged_in(data);
}

static void mbhc_plug_in_detect(void *priv)
{
	struct da_combine_mbhc_priv *data = priv;

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	da_combine_irq_resume_wait(data->irqmgr);

	plug_in_detect(data);
}

static void mbhc_plug_out_detect(void *priv)
{
	struct da_combine_mbhc_priv *data = priv;

	if (priv == NULL) {
		AUDIO_LOGE("null pointer");
		return;
	}

	da_combine_irq_resume_wait(data->irqmgr);

	plug_out_detect(data);
}

static int get_mbhc_headset_type(void *priv)
{
	struct da_combine_mbhc_priv *data = priv;

	if (priv == NULL) {
		AUDIO_LOGE("null pointer");
		return -1;
	}

	return (int)(data->hs_status);
}

static void enable_high_resistence(void *priv, bool enable)
{
	struct da_combine_mbhc_priv *data = priv;

	if (priv == NULL) {
		AUDIO_LOGE("null pointer");
		return;
	}

	da_combine_resmgr_hs_high_resistence_enable(data->resmgr, enable);
}

#ifdef CONFIG_ANC_HS_INTERFACE
static struct anc_hs_dev g_anc_dev = {
	.name = "anc_hs",
	.ops = {
		.check_headset_in = mbhc_check_headset_in,
		.btn_report = report_soc_jack,
		.codec_resume_lock = NULL,
		.plug_in_detect = mbhc_plug_in_detect,
		.plug_out_detect = mbhc_plug_out_detect,
		.check_bus_status = mbhc_check_bus_status,
	},
};
#endif

static struct ana_hs_codec_dev g_ana_hs_dev = {
	.name = "ana_hs",
	.ops = {
		.check_headset_in = mbhc_check_headset_in,
		.plug_in_detect = mbhc_plug_in_detect,
		.plug_out_detect = mbhc_plug_out_detect,
		.get_headset_type = get_mbhc_headset_type,
		.hs_high_resistence_enable = enable_high_resistence,
	},
};


static const struct cfg_node_info g_hs_cfg[] = {
	{ "hisilicon,hs_det", 0 },
	{ "hisilicon,hs_ctrl", HS_CTRL_DAFULT_VALUEL },
	{ "hisilicon,coefficient", COEFFICIENT_DAFULT_VALUE },
	{ "hisilicon,hs_hook_key_map", 0 },
	{ "hisilicon,hs_irq_pm", 0 },
	{ "hisilicon,hs_mbhc_vref_reg_value", EXTERN_CABLE_VREF_DAFULT_VALUE },
	{ "hisilicon,da_combine_irq_reg0", 0 },
};

static void get_hs_config(struct device_node *node,
	struct da_combine_mbhc_config *mbhc_config)
{
	unsigned int i;
	unsigned int val = 0;
	unsigned int size = ARRAY_SIZE(g_hs_cfg);

	for (i = 0; i < size; i++) {
		mbhc_config->hs_cfg[i] = g_hs_cfg[i].def_val;

		if (!of_property_read_u32(node, g_hs_cfg[i].node, &val))
			mbhc_config->hs_cfg[i] = val;
	}
}

static const struct voltage_node_info g_voltage_tbl[] = {
	{ "hisilicon,hs_3_pole_min_voltage", "hisilicon,hs_3_pole_max_voltage",
		HS_3_POLE_MIN_VOLTAGE, HS_3_POLE_MAX_VOLTAGE },
	{ "hisilicon,hs_4_pole_min_voltage", "hisilicon,hs_4_pole_max_voltage",
		HS_4_POLE_MIN_VOLTAGE, HS_4_POLE_MAX_VOLTAGE },
	{ "hisilicon,btn_play_min_voltage", "hisilicon,btn_play_max_voltage",
		BTN_PLAY_MIN_VOLTAGE, BTN_PLAY_MAX_VOLTAGE },
	{ "hisilicon,btn_volume_up_min_voltage", "hisilicon,btn_volume_up_max_voltage",
		BTN_VOLUME_UP_MIN_VOLTAGE, BTN_VOLUME_UP_MAX_VOLTAGE },
	{ "hisilicon,btn_volume_down_min_voltage", "hisilicon,btn_volume_down_max_voltage",
		BTN_VOLUME_DOWN_MIN_VOLTAGE, BTN_VOLUME_DOWN_MAX_VOLTAGE },
	{ "hisilicon,btn_voice_assistant_min_voltage", "hisilicon,btn_voice_assistant_max_voltage",
		INVALID_VOLTAGE, INVALID_VOLTAGE },
	{ "hisilicon,hs_extern_cable_min_voltage", "hisilicon,hs_extern_cable_max_voltage",
		HS_EXTERN_CABLE_MIN_VOLTAGE, HS_EXTERN_CABLE_MAX_VOLTAGE },
};

static void get_voltage_config(const struct device_node *node,
	struct da_combine_mbhc_config *mbhc_config)
{
	unsigned int i;
	unsigned int val = 0;
	unsigned int size = ARRAY_SIZE(g_voltage_tbl);

	for (i = 0; i < size; i++) {
		mbhc_config->voltage[i].min = g_voltage_tbl[i].min_def;
		mbhc_config->voltage[i].max = g_voltage_tbl[i].max_def;

		if (!of_property_read_u32(node, g_voltage_tbl[i].min_node, &val))
			mbhc_config->voltage[i].min = val;
		if (!of_property_read_u32(node, g_voltage_tbl[i].max_node, &val))
			mbhc_config->voltage[i].max = val;
	}
}

static void get_hs_support(const struct device_node *node,
	struct da_combine_mbhc_config *mbhc_config)
{
	unsigned int temp = 0;

	if (of_property_read_bool(node, "hisilicon,hs_report_line_in_out"))
		mbhc_config->hs_report_line_in_out = true;
	else
		mbhc_config->hs_report_line_in_out = false;

	if (of_property_read_bool(node, "hisilicon,hs_support_positive_invert_switch"))
		mbhc_config->hs_support_positive_invert_switch = true;
	else
		mbhc_config->hs_support_positive_invert_switch = false;

	mbhc_config->hs_detect_extern_cable = false;
	if (!of_property_read_u32(node, "hs_detect_extern_cable", &temp)) {
		if (temp != 0)
			mbhc_config->hs_detect_extern_cable = true;
	}

	mbhc_config->analog_hs_unsupport = false;
	if (!of_property_read_u32(node, "analog_hs_unsupport", &temp)) {
		if (temp != 0)
			mbhc_config->analog_hs_unsupport = true;
	}
}

static void set_mbhc_config(struct device_node *node,
	struct da_combine_mbhc_config *mbhc_config)
{
	get_hs_config(node, mbhc_config);

	get_voltage_config(node, mbhc_config);

	get_hs_support(node, mbhc_config);

	headset_auto_calib_init(node);
}

static struct snd_soc_jack_pin g_headset_jack_pins[] = {
	{
		.pin = "Headset Mic Jack",
		.mask = SND_JACK_MICROPHONE,
	},
	{
		.pin = "Headphone Jack",
		.mask = SND_JACK_HEADPHONE,
	},
	{
		.pin = "Speaker Jack",
		.mask = SND_JACK_LINEOUT,
	},
	{
		.pin = "Mic Jack",
		.mask = SND_JACK_LINEIN,
	},
};

static int register_hs_jack_btn(struct snd_soc_component *codec,
	const struct da_combine_mbhc_config *mbhc_config)
{
	int ret;
	unsigned int i;
	struct jack_key_to_type key_type[] = {
		{ SND_JACK_BTN_0, KEY_MEDIA },
		{ SND_JACK_BTN_1, KEY_VOLUMEUP },
		{ SND_JACK_BTN_2, KEY_VOLUMEDOWN },
		{ SND_JACK_BTN_3, KEY_VOICECOMMAND },
		{ SND_JACK_BTN_5, KEY_F14 },
	};

	/* register headset jack */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)
	if (mbhc_config->hs_report_line_in_out) {
		int status = SND_JACK_LINEIN | SND_JACK_LINEOUT;

		ret = snd_soc_card_jack_new(codec->card, "Headset Jack",
			SND_JACK_HEADSET | SND_JACK_LINEIN | SND_JACK_LINEOUT,
			&g_hs_jack, g_headset_jack_pins, ARRAY_SIZE(g_headset_jack_pins));
		report_soc_jack(status, SND_JACK_HEADSET |
			SND_JACK_LINEIN | SND_JACK_LINEOUT);
	} else {
		ret = snd_soc_card_jack_new(codec->card,
			"Headset Jack", SND_JACK_HEADSET, &g_hs_jack, NULL, 0);
	}
#else
	ret = snd_soc_jack_new(codec, "Headset Jack", SND_JACK_HEADSET, &g_hs_jack);
#endif
	if (ret != 0) {
		AUDIO_LOGE("jack error, error num: %d", ret);
		return ret;
	}

	if (mbhc_config->hs_cfg[HS_HOOK_KEY_MAP] == HOOK_KEY_MAP_ENABLE)
		key_type[HOOK_KEY_MAP_TYPE].key = KEY_PLAYPAUSE;

	for (i = 0; i < ARRAY_SIZE(key_type); i++) {
		ret = snd_jack_set_key(g_hs_jack.jack, key_type[i].type,
			key_type[i].key);
		if (ret != 0) {
			AUDIO_LOGE("jack error, error num: %d", ret);
			return ret;
		}
	}

	return 0;
}

static void mbhc_first_detect(struct da_combine_mbhc_priv *priv)
{
	if (is_headset_pluged_in(priv)) {
		if (check_usb_analog_hs_support())
			usb_analog_hs_plug_in_out_handle(ANA_HS_PLUG_IN);
		else if (ana_hs_support_usb_sw())
			ana_hs_plug_handle(ANA_HS_PLUG_IN);
		else
			plug_in_detect(priv);
	}
}

static int check_hs_cfg(const struct da_combine_hs_cfg *hs_cfg)
{
	if (hs_cfg == NULL) {
		AUDIO_LOGE("headset cfg is not exit");
		return -EINVAL;
	}

	if (hs_cfg->mbhc_func == NULL) {
		AUDIO_LOGE("mbhc func is not exit");
		return -EINVAL;
	}

	if (hs_cfg->mbhc_func->hs_enable_hsdet == NULL) {
		AUDIO_LOGE("headset func is not exit");
		return -EINVAL;
	}

	return 0;
}

static const struct request_irq g_irq_tbl[] = {
	{ IRQ_PLUGIN, plug_in_handler, "plugin" },
	{ IRQ_PLUGOUT, plug_out_handler, "plugout" },
	{ IRQ_BTNUP_COMP1, btn_up_handler, "btnup_comp1" },
	{ IRQ_BTNDOWN_COMP1, btn_down_handler, "btndown_comp1" },
	{ IRQ_BTNUP_COMP2, btn_up_comp2_handler, "btnup_comp2" },
	{ IRQ_BTNDOWN_COMP2, btn_down_comp2_handler, "btndown_comp2" },
	{ IRQ_BTNUP_ECO, btn_up_eco_handler, "btnup_eco" },
	{ IRQ_BTNDOWN_ECO, btn_down_eco_handler, "btndown_eco" },
};

static int request_irq_all(struct da_combine_irq *irqmgr,
	struct da_combine_mbhc_priv *priv)
{
	unsigned int i;
	unsigned int j;
	unsigned int size;
	int ret;

	size = ARRAY_SIZE(g_irq_tbl);
	for (i = 0; i < size; i++) {
		ret = da_combine_irq_request_irq(irqmgr, g_irq_tbl[i].irq_id,
			g_irq_tbl[i].handler, g_irq_tbl[i].name, priv);
		if (ret != 0) {
			AUDIO_LOGE("request irq %s fail, ret: %d",
				g_irq_tbl[i].name, ret);
			for (j = i; j > 0; j--)
				da_combine_irq_free_irq(priv->irqmgr,
					g_irq_tbl[j - 1].irq_id, priv);
			break;
		}
	}

	return ret;
}

static void release_irq_all(struct da_combine_mbhc_priv *priv)
{
	if (priv->irqmgr) {
		da_combine_irq_free_irq(priv->irqmgr, IRQ_PLUGOUT, priv);
		da_combine_irq_free_irq(priv->irqmgr, IRQ_PLUGIN, priv);
		da_combine_irq_free_irq(priv->irqmgr, IRQ_BTNUP_ECO, priv);
		da_combine_irq_free_irq(priv->irqmgr, IRQ_BTNDOWN_ECO, priv);
		da_combine_irq_free_irq(priv->irqmgr, IRQ_BTNDOWN_COMP1, priv);
		da_combine_irq_free_irq(priv->irqmgr, IRQ_BTNUP_COMP1, priv);
	}
}

static void mbhc_mutex_init(struct da_combine_mbhc_priv *priv)
{
	wakeup_source_init(&priv->wake_lock, "da_combine-mbhc");
	wakeup_source_init(&priv->micbias_wake_lock, "da_combine-mbhc-micbias");
	wakeup_source_init(&priv->timeout_wake_lock, "da_combine-mbhc-timeout");
	mutex_init(&priv->plug_mutex);
	mutex_init(&priv->status_mutex);
	mutex_init(&priv->saradc_mutex);
}

static void mbhc_mutex_deinit(struct da_combine_mbhc_priv *priv)
{
	wakeup_source_trash(&priv->wake_lock);
	wakeup_source_trash(&priv->micbias_wake_lock);
	wakeup_source_trash(&priv->timeout_wake_lock);
	mutex_destroy(&priv->plug_mutex);
	mutex_destroy(&priv->status_mutex);
	mutex_destroy(&priv->saradc_mutex);
}

static int mbhc_workqueue_init(struct da_combine_mbhc_priv *priv)
{
	priv->micbias_delay_wq =
		create_singlethread_workqueue("da_combine_micbias_delay_wq");
	if (priv->micbias_delay_wq == NULL) {
		AUDIO_LOGE("workqueue create failed");
		return -EINVAL;
	}

	INIT_DELAYED_WORK(&priv->micbias_delay_work, micbias_work_func);

	priv->mbhc_micbias_work = false;

	return 0;
}

static void mbhc_workqueue_deinit(struct da_combine_mbhc_priv *priv)
{
	if (priv->micbias_delay_wq != NULL) {
		cancel_delayed_work(&priv->micbias_delay_work);
		flush_workqueue(priv->micbias_delay_wq);
		destroy_workqueue(priv->micbias_delay_wq);
		priv->micbias_delay_wq = NULL;
	}
}

static int hs_dev_register(struct da_combine_mbhc_priv *priv,
	struct snd_soc_component *codec)
{
	/* register anc hs first */
#ifdef CONFIG_ANC_HS_INTERFACE
	anc_hs_interface_dev_register(&g_anc_dev, priv);
#endif
	usb_analog_hs_dev_register(&g_ana_hs_dev, priv);
	ana_hs_codec_dev_register(&g_ana_hs_dev, priv);

	if (register_hs_jack_btn(codec, &priv->mbhc_config) != 0)
		return -EINVAL;

	return 0;
}

static int mbhc_source_init(struct da_combine_mbhc_priv *priv,
	struct da_combine_mbhc_cfg *mbhc_cfg)
{
	int ret;

#ifdef CONFIG_SWITCH
	priv->sdev.name = "h2w";
	ret = switch_dev_register(&priv->sdev);
	if (ret != 0) {
		AUDIO_LOGE("error registering switch device %d", ret);
		goto switch_err;
	}
#endif
	mbhc_mutex_init(priv);

	ret = mbhc_workqueue_init(priv);
	if (ret != 0)
		goto wq_init_err;

	headset_type_ctrl_gpio_init(priv, mbhc_cfg->node);

	ret = hs_dev_register(priv, mbhc_cfg->codec);
	if (ret != 0)
		goto register_err;


	ret = request_irq_all(mbhc_cfg->irqmgr, priv);
	if (ret != 0)
		goto irq_init_err;

	/* mask btn irqs */
	da_combine_irq_mask_btn_irqs(&priv->mbhc_pub);

	return ret;

irq_init_err:
register_err:
	headset_type_ctrl_gpio_deinit(priv);
	mbhc_workqueue_deinit(priv);
wq_init_err:
	mbhc_mutex_deinit(priv);
#ifdef CONFIG_SWITCH
	switch_dev_unregister(&priv->sdev);
switch_err:
#endif
	return ret;
}

static void mbhc_source_deinit(struct da_combine_mbhc_priv *priv)
{
	release_irq_all(priv);

	headset_type_ctrl_gpio_deinit(priv);
	mbhc_workqueue_deinit(priv);
	mbhc_mutex_deinit(priv);

#ifdef CONFIG_SWITCH
	switch_dev_unregister(&priv->sdev);
#endif
}


void da_combine_disable_hs_irq(struct da_combine_irq *irqmgr)
{
	if (irqmgr == NULL) {
		AUDIO_LOGE("mbhc irq mgr is null");
		return;
	}

	da_combine_irq_disable_irqs(irqmgr, ARRAY_SIZE(g_hs_irq_type_arr),
		g_hs_irq_type_arr);
	AUDIO_LOGI("headset irq disable");
}

void da_combine_enable_hs_irq(struct da_combine_irq *irqmgr)
{
	if (irqmgr == NULL) {
		AUDIO_LOGE("mbhc irq mgr is null");
		return;
	}

	da_combine_irq_enable_irqs(irqmgr, ARRAY_SIZE(g_hs_irq_type_arr),
		g_hs_irq_type_arr);
	AUDIO_LOGI("headset irq enable");
}

void da_combine_irq_mask_btn_irqs(struct da_combine_mbhc *mbhc)
{
	const int *phy_irqs = g_mbhc_btn_bits;
	int irq_num = ARRAY_SIZE(g_mbhc_btn_bits);

	struct da_combine_mbhc_priv *priv = (struct da_combine_mbhc_priv *)mbhc;

	if (priv == NULL) {
		AUDIO_LOGE("null pointer");
		return;
	}

	da_combine_irq_disable_irqs(priv->irqmgr, irq_num, phy_irqs);
}

void da_combine_irq_unmask_btn_irqs(struct da_combine_mbhc *mbhc)
{
	const int *phy_irqs = g_mbhc_btn_bits;
	int irq_num = ARRAY_SIZE(g_mbhc_btn_bits);

	struct da_combine_mbhc_priv *priv = (struct da_combine_mbhc_priv *)mbhc;

	if (priv == NULL) {
		AUDIO_LOGE("null pointer");
		return;
	}

	da_combine_irq_enable_irqs(priv->irqmgr, irq_num, phy_irqs);
}

void da_combine_set_audio_jack_headset(const struct da_combine_mbhc *mbhc)
{
	const struct da_combine_mbhc_priv *priv = NULL;
	int ret;

	if (mbhc == NULL) {
		AUDIO_LOGE("parameter mbhc is invalid");
		return;
	}

	priv = (const struct da_combine_mbhc_priv *)mbhc;

	ret = set_gpio_value(AUDIO_JACK_HEADSET, priv);
	if (ret != 0)
		AUDIO_LOGE("set gpio error, ret: %d", ret);
}

void da_combine_set_audio_jack_invert(const struct da_combine_mbhc *mbhc)
{
	const struct da_combine_mbhc_priv *priv = (const struct da_combine_mbhc_priv *)mbhc;
	int ret;

	ret = set_gpio_value(AUDIO_JACK_INVERT, priv);
	if (ret != 0)
		AUDIO_LOGE("set gpio error, ret: %d", ret);
}

int da_combine_get_4_pole_headset_type(const struct da_combine_mbhc *mbhc)
{
	const struct mbhc_hs_type_ctrl *hs_ctrl = NULL;
	const struct mbhc_hs_ctrl_gpio_cfg *gpio_cfg_array = NULL;
	const struct da_combine_mbhc_priv *priv = NULL;
	bool jack_headset_status = false;
	bool jack_invert_status = false;
	int gpio_cur_value;
	int i;

	if (mbhc == NULL) {
		AUDIO_LOGE("parameter mbhc is invalid");
		return -EINVAL;
	}

	priv = (const struct da_combine_mbhc_priv *)mbhc;
	hs_ctrl = &priv->hs_type_ctrl;
	gpio_cfg_array = hs_ctrl->gpio_cfg_array;

	if (hs_ctrl->gpio_array == NULL) {
		AUDIO_LOGW("headset type control is disable");
		return 0;
	}

	for (i = 0; i < hs_ctrl->gpio_num; ++i) {
		gpio_cur_value = gpio_get_value(gpio_cfg_array[i].gpio);
		AUDIO_LOGI("get gpio %u value is %d",
			hs_ctrl->gpio_cfg_array[i].gpio, gpio_cur_value);

		if (gpio_cur_value == gpio_cfg_array[i].jack_headset_value)
			jack_headset_status = true;
		if (gpio_cur_value == gpio_cfg_array[i].jack_invert_value)
			jack_invert_status = true;
	}

	if (jack_headset_status && !jack_invert_status)
		return GPIO_STATUS_JACK_HEADSET;
	else if (!jack_headset_status && jack_invert_status)
		return GPIO_STATUS_JACK_INVERT;

	AUDIO_LOGW("jack status isn't 4-pole headset");

	return 0;
}

bool da_combine_check_headset_pluged_in(void)
{
	if (g_mbhc_priv == NULL) {
		AUDIO_LOGE("mbhc priv is null");
		return false;
	}

	return is_headset_pluged_in(g_mbhc_priv);
}

bool da_combine_check_saradc_ready_detection(struct snd_soc_component *codec)
{
	unsigned int val;

	if (codec == NULL)
		return false;

	/* read codec status */
	val = snd_soc_component_read32(codec, DA_COMBINE_REG_IRQ_2 - CODEC_BASE_ADDR) &
		BIT(DA_COMBINE_SARADC_RD_BIT);

	/* clr irq */
	da_combine_update_bits(codec, DA_COMBINE_REG_IRQ_2 - CODEC_BASE_ADDR,
		BIT(DA_COMBINE_SARADC_RD_BIT), BIT(DA_COMBINE_SARADC_RD_BIT));

	if (val == 0)
		return false;

	return true;
}

void da_combine_plug_in_detect_wrapper(struct da_combine_mbhc *mbhc)
{
	struct da_combine_mbhc_priv *priv = (struct da_combine_mbhc_priv *)mbhc;

	if (priv == NULL) {
		AUDIO_LOGE("mbhc priv is null");
		return;
	}

	mbhc_plug_in_detect(priv);
}

bool da_combine_support_hs_irq_pm(void)
{
	if (g_mbhc_priv == NULL) {
		AUDIO_LOGE("mbhc priv is null");
		return false;
	}

	AUDIO_LOGI("support headset irq pm: %u",
		g_mbhc_priv->mbhc_config.hs_cfg[HS_IRQ_PM]);

	return g_mbhc_priv->mbhc_config.hs_cfg[HS_IRQ_PM];
}

void da_combine_plug_out_wrapper(struct da_combine_mbhc *mbhc)
{
	struct da_combine_mbhc_priv *priv = (struct da_combine_mbhc_priv *)mbhc;

	if (priv == NULL) {
		AUDIO_LOGE("mbhc priv is null");
		return;
	}

	plug_out(priv);
}

int da_combine_mbhc_init(struct da_combine_mbhc_cfg *mbhc_cfg,
	const struct da_combine_hs_cfg *hs_cfg)
{
	struct da_combine_mbhc_priv *priv = NULL;
	int ret;

	if (check_hs_cfg(hs_cfg) != 0)
		return -EINVAL;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (priv == NULL) {
		AUDIO_LOGE("kzalloc failed");
		return -ENOMEM;
	}

	/* unsafe function ignore: memcpy */
	memcpy(&priv->hs_cfg, hs_cfg, sizeof(*hs_cfg));

	set_mbhc_config(mbhc_cfg->node, &priv->mbhc_config);

	priv->codec = mbhc_cfg->codec;
	priv->resmgr = mbhc_cfg->resmgr;
	priv->irqmgr = mbhc_cfg->irqmgr;

	priv->need_match_micbias = false;
	priv->hp_cfg.ph_switch.cur_irq_ts = 0;
	priv->hp_cfg.ph_switch.pre_irq_ts = 0;

	ret = mbhc_source_init(priv, mbhc_cfg);
	if (ret != 0)
		goto source_err;

	priv->hs_cfg.mbhc_func->hs_enable_hsdet(mbhc_cfg->codec, priv->mbhc_config);

	/* check jack at first time */
	mbhc_first_detect(priv);

	g_mbhc_priv = priv;
	*(mbhc_cfg->mbhc) = &priv->mbhc_pub;

	rear_jack_init(priv->codec);

	return ret;

source_err:
	kfree(priv);
	priv = NULL;

	return ret;
}

void da_combine_mbhc_deinit(struct da_combine_mbhc *mbhc)
{
	struct da_combine_mbhc_priv *priv = (struct da_combine_mbhc_priv *)mbhc;

	if (priv == NULL)
		return;

	mbhc_source_deinit(priv);

	kfree(priv);
}

