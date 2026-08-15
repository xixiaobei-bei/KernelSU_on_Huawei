/*
 * hw_pogopin.h
 *
 * huawei pogopin driver
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
#define POGOPIN_TIMEOUT_500MS      500

#define POGOPIN_HISHOW_ADC_MIN     270
#define POGOPIN_HISHOW_ADC_MAX     450

#define POGOPIN_CHARGER_INSERT     0
#define POGOPIN_CHARGER_REMOVE     1

#define POGOPIN_IIN_CURRENT        2000
#define POGOPIN_ICHG_CURRENT       2100

#define POGOPIN_MOS_DELAYED        120 /* mos open has 120ms delay */

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
	int pin_num; /* some not contians D+,D-line */
	int typecvbus_from_pd; /* some not have typec vbus int gpio */
	bool fcp_support;
	bool hishow_support;
	int hishow_id_adc;
	int hishow_adc_value;
	bool typec_otg_use_buckboost;
	int pogo_otg_buckboost_gpio;
	int pogo_vbus_mos_ctrl_gpio;
	int keyboard_ldo_gpio;
	bool keyboard_connect;
	struct notifier_block tcpc_nb;
	struct delayed_work dpm_notify_work;
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
#endif /* CONFIG_POGO_PIN */

#endif /* _HW_POGOPIN_H_ */
