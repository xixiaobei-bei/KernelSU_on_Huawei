/*
 * HWEVEXT_mbhc.h
 *
 * HWEVEXT_mbhc header file
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
 *
 */

#ifndef __HWEVEXT_MBHC__
#define __HWEVEXT_MBHC__

#include <linux/pm_wakeup.h>
#include <sound/soc.h>
#include <linux/switch.h>
#include <linux/spinlock.h>

enum audio_jack_states {
	AUDIO_JACK_NONE = 0,     /* unpluged */
	AUDIO_JACK_HEADSET,      /* pluged 4-pole headset */
	AUDIO_JACK_HEADPHONE,    /* pluged 3-pole headphone */
	AUDIO_JACK_INVERT,       /* pluged invert 4-pole headset */
	AUDIO_JACK_EXTERN_CABLE, /* pluged extern cable,such as antenna cable */
};

enum hs_voltage_type {
	VOLTAGE_POLE3 = 0,
	VOLTAGE_POLE4,
	VOLTAGE_EXTERN_CABLE,
	VOLTAGE_HOOK,
	VOLTAGE_VOL_UP,
	VOLTAGE_VOL_DOWN,
	VOLTAGE_TYPE_BUTT
};

struct voltage_type_node {
	unsigned int min;
	unsigned int max;
};

struct hwevext_mbhc_priv;

struct hwevext_mbhc_config {
	/* board defination */
	struct voltage_type_node voltage[VOLTAGE_TYPE_BUTT];
	bool analog_hs_unsupport;
};

struct hwevext_mbhc_cb {
	bool (*mbhc_check_headset_in)(struct hwevext_mbhc_priv *mbhc);
	void (*enable_micbias)(struct hwevext_mbhc_priv *mbhc);
	void (*disable_micbias)(struct hwevext_mbhc_priv *mbhc);
	unsigned int(*get_hs_status_reg_value)(struct hwevext_mbhc_priv *mbhc);
};

/* defination of private data */
struct hwevext_mbhc_priv {
	struct device *dev;
	struct snd_soc_component *component;
	struct wakeup_source plug_wake_lock;
	struct wakeup_source btn_wake_lock;
	struct wakeup_source plug_repeat_detect_lock;
	struct wakeup_source plug_irq_wake_lock;
	struct wakeup_source btn_irq_wake_lock;
	struct wakeup_source long_btn_wake_lock;

	struct mutex plug_mutex;
	struct mutex status_mutex;
	struct mutex adc_mutex;
	struct mutex btn_mutex;
	int plug_irq_gpio;
	int plug_irq;
	int btn_irq_gpio;
	int btn_irq;
	struct snd_soc_jack headset_jack;
	struct snd_soc_jack button_jack;
	unsigned int hkadc_channel;
	/* headset status */
	enum audio_jack_states hs_status;
	int btn_report;
	int btn_pressed;

#ifdef CONFIG_SWITCH
	struct switch_dev sdev;
#endif
	/* board defination */
	struct hwevext_mbhc_config mbhc_config;
	struct hwevext_mbhc_cb *mbhc_cb;

	struct mutex read_hs_status_reg_lock;
	int hs_status_reg_status0;
	int hs_status_reg_status1;
	int hs_status_reg_read_retry;
	bool repeat_detect_work_stop;

	struct workqueue_struct *irq_plug_wq;
	struct delayed_work plug_work;
	struct workqueue_struct *irq_btn_handle_wq;
	struct delayed_work btn_delay_work;
	struct workqueue_struct *read_hs_status_wq;
	struct delayed_work read_hs_status_work;
	struct workqueue_struct *repeat_detect_plug_wq;
	struct delayed_work repeat_detect_plug_work;
	struct workqueue_struct *long_press_btn_wq;
	struct delayed_work long_press_btn_work;
	struct mutex micbias_vote_mutex;
	atomic_t micbias_vote_cnt;
	struct mutex btn_irq_vote_mutex;
	int btn_irq_vote_cnt;

	spinlock_t btn_irq_lock;
	unsigned long btn_irq_jiffies;
	atomic_t need_further_detect_for_btn_irq;
	int adc_power_supply_en_gpio;
	struct mutex adc_supply_en_vote_mutex;
	atomic_t adc_supply_en_vote_cnt;
};

struct voltage_btn_report {
	const char *info;
	enum hs_voltage_type voltage_type;
	int btn_type;
};

#ifdef CONFIG_HWEVEXT_MBHC
int hwevext_mbhc_init(struct device *dev,
	struct snd_soc_component *component,
	struct hwevext_mbhc_priv **mbhc_data,
	struct hwevext_mbhc_cb *mbhc_cb);
void hwevext_mbhc_exit(struct device *dev,
	struct hwevext_mbhc_priv *mbhc_data);
#else
static inline int hwevext_mbhc_init(struct device *dev,
	struct snd_soc_component *component,
	struct hwevext_mbhc_priv **mbhc_data,
	struct hwevext_mbhc_cb *mbhc_cb)
{
	return 0;
}

static inline void hwevext_mbhc_exit(struct device *dev,
	struct hwevext_mbhc_priv *mbhc_data);
{
}
#endif
#endif // HWEVEXT_MBHC