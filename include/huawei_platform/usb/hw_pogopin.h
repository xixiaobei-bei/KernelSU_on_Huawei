/*
 * hw_pogopin.h
 *
 * huawei pogopin driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef _HW_POGOPIN_H_
#define _HW_POGOPIN_H_

#ifndef HIGH
#define HIGH                       1
#endif
#ifndef LOW
#define LOW                        0
#endif

#ifndef TRUE
#define TRUE                       1
#endif
#ifndef FALSE
#define FALSE                      0
#endif

#define POGOPIN_3PIN               0
#define POGOPIN_5PIN               1

#define POGOPIN_TIMEOUT_50MS       50
#define POGOPIN_MOS_DELAYED        120 /* mos open has 120 delay */
#define POGOPIN_TIME_DELAYE_200MS  200
#define POGOPIN_TIME_DELAYE_300MS  300
#define POGOPIN_TIMEOUT_500MS      500
#define CHECK_TIMES                5

#define POGOPIN_HISHOW_ADC_MIN     270
#define POGOPIN_HISHOW_ADC_MAX     450

#define POGOPIN_CHARGER_INSERT     0
#define POGOPIN_CHARGER_REMOVE     1

#define POGOPIN_IIN_CURRENT        2000
#define POGOPIN_ICHG_CURRENT       2100

#define POGOPIN_DETECT_GPIO_DELAY  50

enum pogo_status {
	POGO_NONE = 0,
	POGO_CHARGER,
	POGO_OTG,
	POGO_STATUS_END,
};

#define STATE_ON                   1
#define STATE_OFF                  0

#define ATTACH_RESTORE             0
#define ATTACH_ENABLE              1

struct pogopin_info {
	struct platform_device *pdev;
	struct pogopin_cc_ops *ops;
	int usb_switch_gpio;
	int power_switch_gpio;
	int switch_power_gpio;
	int buck_boost_gpio;
	int pogopin_int_gpio;
	int pogopin_int_irq;
	int typec_int_gpio;
	struct work_struct work;
	int current_int_status;
	int pogopin_support;
	struct completion dev_off_completion;
	u32 pin_num; /* some not contians D+,D-line */
	u32 is_hw_pd;
	int typec_state;
	u32 typecvbus_from_pd; /* some not have typec vbus int gpio */
	bool fcp_support;
	bool hishow_support;
	int hishow_id_adc;
	int hishow_adc_value;
	bool typec_otg_use_buckboost;
	int pogo_otg_buckboost_gpio;
	int pogo_vbus_mos_ctrl_gpio;
	int keyboard_ldo_gpio;
	bool keyboard_connect;
	bool typec_vbus_unctrl;
	bool pogopin_int_debounce;
	bool vbus_detect;
	struct notifier_block tcpc_nb;
	struct delayed_work dpm_notify_work;
	u32 charge_otg_ctl;
	int typec_int_irq;
	struct work_struct typec_int_work;
	int typec_chg_ana_audio_suport;
	int audio_switch_control;
	enum pogo_status pogo_insert_status;
	bool ana_audio_status;
	int pogopin_pmic_vbus_attach_enable;
};

struct pogopin_cc_ops {
	int (*typec_detect_disable)(bool);
	int (*typec_detect_vbus)(void);
};

enum pogopin_sysfs_type {
	POGOPIN_SYSFS_INTERFACE_TYPE = 0,
};

enum current_working_interface {
	POGOPIN_INTERFACE_BEGIN = 0,
	NO_INTERFACE = POGOPIN_INTERFACE_BEGIN,
	TYPEC_INTERFACE,
	POGOPIN_INTERFACE,
	POGOPIN_AND_TYPEC,
	POGOPIN_INTERFACE_END,
};

enum pogopin_event {
	POGOPIN_EVENT_START = 0,
	POGOPIN_OTG_AND_CHG_START, /* start typec charging when pogopin otg */
	POGOPIN_OTG_AND_CHG_STOP, /* stop typec charging when pogopin otg */
	POGOPIN_SAVE_CHARGER_TYPE, /* save charger type when typec gpio low */
	POGOPIN_CHARGER_OUT_COMPLETE, /* notify pogopin charger out */
	POGOPIN_EVENT_END,
};

#ifdef CONFIG_POGO_PIN
extern void pogopin_cc_register_ops(struct pogopin_cc_ops *ops);
extern bool pogopin_is_support(void);
extern int pogopin_get_interface_status(void);
extern bool pogopin_otg_from_buckboost(void);
extern void pogopin_5pin_completion_devoff(void);
extern unsigned long pogopin_5pin_wait_for_completion(unsigned long timeout);
extern void pogopin_5pin_reinit_completion_devoff(void);
extern void pogopin_5pin_typec_detect_disable(bool en);
extern void pogopin_5pin_otg_in_switch_from_typec(void);
extern void pogopin_5pin_remove_switch_to_typec(void);
extern void pogopin_5pin_int_irq_disable(bool en);
extern bool pogopin_5pin_get_fcp_support(void);
extern void pogopin_3pin_typec_otg_buckboost_ctrl(bool enable);
extern int pogopin_3pin_register_pogo_vbus_notifier(struct notifier_block *nb);
extern bool pogopin_3pin_ignore_pogo_vbus_in_event(void);
extern int pogopin_3pin_get_input_current(void);
extern int pogopin_3pin_get_charger_current(void);
extern int pogopin_event_notifier_register(struct notifier_block *nb);
extern int pogopin_event_notifier_unregister(struct notifier_block *nb);
extern void pogopin_event_notify(enum pogopin_event event);
extern bool pogopin_is_charging(void);
extern void pogopin_otg_status_change_process(uint8_t value);
extern bool pogopin_typec_chg_ana_audio_support(void);
extern void pogopin_set_typec_state(int typec_state);
extern void pogopin_5pin_set_pogo_status(enum pogo_status status);
extern enum pogo_status pogopin_5pin_get_pogo_status(void);
extern void pogopin_5pin_set_ana_audio_status(bool status);
extern bool pogopin_5pin_get_ana_audio_status(void);
extern int pogopin_get_pmic_vbus_irq_enable(void);
extern void pogopin_set_buck_boost_gpio(int value);
#else
static inline void pogopin_cc_register_ops(struct pogopin_cc_ops *ops)
{
}

static inline bool pogopin_is_support(void)
{
	return 0;
}

static inline void pogopin_5pin_completion_devoff(void)
{
}

static inline unsigned long pogopin_5pin_wait_for_completion(
	unsigned long timeout)
{
	return 0;
}

static inline void pogopin_5pin_reinit_completion_devoff(void)
{
}

static inline int pogopin_get_interface_status(void)
{
	return 0;
}

static inline void pogopin_5pin_typec_detect_disable(bool en)
{
}

static inline void pogopin_5pin_otg_in_switch_from_typec(void)
{
}

static inline void pogopin_5pin_remove_switch_to_typec(void)
{
}

static inline void pogopin_5pin_int_irq_disable(bool en)
{
}

static inline bool pogopin_5pin_get_fcp_support(void)
{
	return true;
}

static inline void pogopin_3pin_typec_otg_buckboost_ctrl(bool enable)
{
}

static inline int pogopin_3pin_register_pogo_vbus_notifier(
	struct notifier_block *nb)
{
	return 0;
}

static inline bool pogopin_3pin_ignore_pogo_vbus_in_event(void)
{
	return false;
}

static inline bool pogopin_otg_from_buckboost(void)
{
	return false;
}

static inline int pogopin_3pin_get_input_current(void)
{
	return 0;
}

static inline int pogopin_3pin_get_charger_current(void)
{
	return 0;
}

static inline int pogopin_event_notifier_register(struct notifier_block *nb)
{
	return 0;
}

static inline int pogopin_event_notifier_unregister(struct notifier_block *nb)
{
	return 0;
}

static inline void pogopin_event_notify(enum pogopin_event event)
{
}

static inline bool pogopin_is_charging(void)
{
	return false;
}

static inline void pogopin_otg_status_change_process(uint8_t value)
{
}

static inline bool pogopin_typec_chg_ana_audio_support(void)
{
	return false;
}

static inline void pogopin_set_typec_state(int typec_state)
{
}

static inline void pogopin_5pin_set_pogo_status(enum pogo_status status)
{
}

static inline enum pogo_status pogopin_5pin_get_pogo_status(void)
{
	return POGO_NONE;
}

static inline void pogopin_5pin_set_ana_audio_status(bool status)
{
}

static inline bool pogopin_5pin_get_ana_audio_status(void)
{
	return false;
}

static inline int pogopin_get_pmic_vbus_irq_enable(void)
{
	return ATTACH_RESTORE;
}

static inline void pogopin_set_buck_boost_gpio(int value)
{
}
#endif /* CONFIG_POGO_PIN */

#endif /* _HW_POGOPIN_H_ */
