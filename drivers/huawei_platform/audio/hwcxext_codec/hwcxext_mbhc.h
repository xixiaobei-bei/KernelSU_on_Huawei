/*
 * HWCXEXT_MBHC.h
 *
 * HWCXEXT_MBHC header file
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
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

#ifndef __HWCXEXT_MBHC__
#define __HWCXEXT_MBHC__

#include <linux/pm_wakeup.h>
#include <linux/switch.h>
#include <linux/spinlock.h>

enum audio_jack_states {
	AUDIO_JACK_NONE = 0,     /* unpluged */
	AUDIO_JACK_HEADSET,      /* pluged 4-pole headset */
	AUDIO_JACK_HEADPHONE,    /* pluged 3-pole headphone */
	AUDIO_JACK_INVERT,       /* pluged invert 4-pole headset */
	AUDIO_JACK_EXTERN_CABLE, /* pluged extern cable,such as antenna cable */
	AUDIO_JACK_INVAILD,
};

enum btn_type {
	BTN_TYPE_INVAILD = 0,
	BTN_HOOK,
	BTN_UP,
	BTN_DOWN,
};

enum btn_event {
	BTN_ENENT_INVAILD = 0,
	BTN_PRESS,
	BTN_RELEASED,
};

enum hs_restart_status {
	HS_DETECT_RESTART_BEGIN = 1,
	HS_DETECT_RESTART_END
};

struct hwcxext_mbhc_priv;

struct hwcxext_mbhc_cb {
	bool (*mbhc_check_headset_in)(struct hwcxext_mbhc_priv *mbhc);
	void (*enable_micbias)(struct hwcxext_mbhc_priv *mbhc);
	void (*disable_micbias)(struct hwcxext_mbhc_priv *mbhc);
	void (*enable_jack_detect)(struct hwcxext_mbhc_priv *mbhc,
		bool support_usb_switch);
	int (*get_hs_type_recognize)(struct hwcxext_mbhc_priv *mbhc);
	int (*get_btn_type_recognize)(struct hwcxext_mbhc_priv *mbhc);
	void (*dump_regs)(struct hwcxext_mbhc_priv *mbhc);
	void (*set_hs_detec_restart)(struct hwcxext_mbhc_priv *mbhc, int status);
};

struct btn_type_info {
	unsigned int btn_type;
	unsigned int btn_press_times;
	unsigned int btn_event;
};

/* defination of private data */
struct hwcxext_mbhc_priv {
	struct device *dev;
	struct snd_soc_component *component;
	struct wakeup_source plug_btn_wake_lock;
	struct wakeup_source plug_btn_irq_wake_lock;
	struct wakeup_source plug_wake_lock;
	struct wakeup_source btn_wake_lock;
	struct wakeup_source long_btn_wake_lock;
	struct wakeup_source plug_repeat_detect_lock;

	struct mutex plug_mutex;
	struct mutex status_mutex;
	struct mutex btn_mutex;
	int plug_btn_irq_gpio;
	int plug_btn_irq;
	struct snd_soc_jack headset_jack;
	struct snd_soc_jack button_jack;
	/* headset status */
	enum audio_jack_states hs_status;
	int btn_report;
	int btn_pressed;
	struct btn_type_info get_btn_info;
	unsigned long hs_type_report_jiffies;
	atomic_t btn_not_need_hanlde;

#ifdef CONFIG_SWITCH
	struct switch_dev sdev;
#endif
	struct hwcxext_mbhc_cb *mbhc_cb;

	struct workqueue_struct *irq_plug_btn_wq;
	struct delayed_work irq_plug_btn_work;

	struct workqueue_struct *irq_plug_handle_wq;
	struct delayed_work hs_plug_work;
	struct workqueue_struct *irq_btn_handle_wq;
	struct delayed_work btn_delay_work;

	struct workqueue_struct *long_press_btn_wq;
	struct delayed_work long_press_btn_work;
	struct workqueue_struct *repeat_detect_plug_wq;
	struct delayed_work repeat_detect_plug_work;

	struct mutex micbias_vote_mutex;
	atomic_t micbias_vote_cnt;
	struct mutex btn_irq_vote_mutex;
	atomic_t btn_irq_vote_cnt;
	int jsense_gpio;
	atomic_t jsense_vote_cnt;
	struct mutex jsense_vote_mutex;
};

#ifdef CONFIG_HWCXEXT_MBHC
int hwcxext_mbhc_init(struct device *dev,
	struct snd_soc_component *component,
	struct hwcxext_mbhc_priv **mbhc_data,
	struct hwcxext_mbhc_cb *mbhc_cb);
void hwcxext_mbhc_exit(struct device *dev,
	struct hwcxext_mbhc_priv *mbhc_data);
#else
static inline int hwcxext_mbhc_init(struct device *dev,
	struct snd_soc_component *component,
	struct hwcxext_mbhc_priv **mbhc_data,
	struct hwcxext_mbhc_cb *mbhc_cb)
{
	return 0;
}

static inline void hwcxext_mbhc_exit(struct device *dev,
	struct hwcxext_mbhc_priv *mbhc_data);
{
}
#endif
#endif // HWCXEXT_MBHC
