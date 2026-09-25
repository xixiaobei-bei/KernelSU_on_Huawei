/*
 * hw_pogopin.c
 *
 * pogopin driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/power/power_dts.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_pogopin.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/power/vbus_channel/vbus_channel.h>
#include <huawei_platform/power/huawei_charger.h>
#include <linux/mfd/hisi_pmic.h>
#include <huawei_platform/audio/ana_hs_common.h>
#include <huawei_platform/audio/usb_analog_hs_interface.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG hw_pogopin
HWLOG_REGIST();

struct pogopin_sysfs_info {
	struct device_attribute attr;
	u8 name;
};

static struct device *g_pogopin_dev;
static struct class *g_pogopin_class;
static struct pogopin_info *g_pogopin_di;
static struct pogopin_cc_ops *g_pogopin_cc_ops;
struct blocking_notifier_head g_pogopin_vbus_evt_nh;
BLOCKING_NOTIFIER_HEAD(g_pogopin_vbus_evt_nh);
struct blocking_notifier_head g_pogopin_evt_nb;
BLOCKING_NOTIFIER_HEAD(g_pogopin_evt_nb);

static struct pogopin_info *pogopin_get_dev_info(void)
{
	if (!g_pogopin_di) {
		hwlog_err("g_pogopin_di is null\n");
		return NULL;
	}

	return g_pogopin_di;
}

static struct pogopin_cc_ops *pogopin_get_cc_ops(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return NULL;

	if (!di->ops) {
		hwlog_err("g_pogopin cc ops is null\n");
		return NULL;
	}

	return di->ops;
}

void pogopin_event_notify(enum pogopin_event event)
{
	blocking_notifier_call_chain(&g_pogopin_evt_nb, event, NULL);
}

int pogopin_event_notifier_register(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&g_pogopin_evt_nb, nb);
}

int pogopin_event_notifier_unregister(struct notifier_block *nb)
{
	if (!nb)
		return -EINVAL;

	return blocking_notifier_chain_unregister(&g_pogopin_evt_nb, nb);
}

bool pogopin_is_otg_mode(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di || !di->typec_chg_ana_audio_suport)
		return FALSE;

	return di->is_otg_mode;
}

void pogopin_set_otg_mode(bool status)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di || !di->typec_chg_ana_audio_suport)
		return;

	di->is_otg_mode = status;
}

bool pogopin_is_charging(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return false;

	return ((di->current_int_status == POGOPIN_INTERFACE) ||
		(di->current_int_status == POGOPIN_AND_TYPEC)) ? true : false;
}

static inline void pogopin_otg_buckboost_ctrl(int gpio_num, int value)
{
	gpio_set_value(gpio_num, value);
}

static inline void pogopin_vbus_channel_ctrl(int gpio_num, int value)
{
	gpio_set_value(gpio_num, value);
}

static inline void pogopin_dpdm_channel_ctrl(int gpio_num, int value)
{
	gpio_set_value(gpio_num, value);
}

static void pogopin_5pin_vbus_in_switch_from_typec(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return;

	hwlog_info("pogopin vbus in,power and data switch to pogopin\n");

	/* low turn on mos */
	pogopin_vbus_channel_ctrl(di->power_switch_gpio, LOW);
	if (!di->typec_vbus_unctrl)
		gpio_set_value(di->switch_power_gpio, LOW);
	/* low buckboost cutoff */
	pogopin_otg_buckboost_ctrl(di->buck_boost_gpio, LOW);
	/* high dpdm to pogopin */
	pogopin_dpdm_channel_ctrl(di->usb_switch_gpio, HIGH);
	if (di->audio_switch_control)
		usb_analog_hs_plug_in_out_handle(POGOPIN_PLUG_IN);
}

void pogopin_5pin_otg_in_switch_from_typec(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return;

	hwlog_info("pogopin otg in, power and data switch to pogopin\n");

	pogopin_vbus_channel_ctrl(di->power_switch_gpio, HIGH);
	if (!di->typec_vbus_unctrl)
		gpio_set_value(di->switch_power_gpio, HIGH);
	pogopin_otg_buckboost_ctrl(di->buck_boost_gpio, HIGH);
	pogopin_dpdm_channel_ctrl(di->usb_switch_gpio, HIGH);
	if (di->audio_switch_control)
		usb_analog_hs_plug_in_out_handle(POGOPIN_PLUG_IN);
}

void pogopin_5pin_remove_switch_to_typec(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return;

	hwlog_info("pogopin revome, power and data switch to typec\n");

	pogopin_vbus_channel_ctrl(di->power_switch_gpio, HIGH);
	if (!di->typec_vbus_unctrl)
		gpio_set_value(di->switch_power_gpio, HIGH);
	pogopin_otg_buckboost_ctrl(di->buck_boost_gpio, LOW);
	pogopin_dpdm_channel_ctrl(di->usb_switch_gpio, LOW);
	if (di->audio_switch_control)
		usb_analog_hs_plug_in_out_handle(POGOPIN_PLUG_OUT);
}

static int pogopin_5pin_is_usbswitch_at_typec(void)
{
	int usb_switch_value;
	int buck_boost_value;
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return FALSE;

	usb_switch_value = gpio_get_value(di->usb_switch_gpio);
	buck_boost_value = gpio_get_value(di->buck_boost_gpio);
	if ((usb_switch_value == LOW) && (buck_boost_value == LOW))
		return TRUE;

	return FALSE;
}

void pogopin_5pin_typec_detect_disable(bool en)
{
	struct pogopin_cc_ops *cc_ops = NULL;

	cc_ops = pogopin_get_cc_ops();
	if (!cc_ops)
		return;

	if (!cc_ops->typec_detect_disable) {
		hwlog_err("typec_detect_disable is null\n");
		return;
	}

	cc_ops->typec_detect_disable(en);
}

static void pogopin_5pin_set_fcp_support(bool value)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return;

	di->fcp_support = value;
}

void pogopin_5pin_typec_set_status(uint8_t status)
{
	struct pogopin_cc_ops *cc_ops = pogopin_get_cc_ops();
	static uint8_t old_status = POGO_NONE;

	if (!cc_ops) {
		hwlog_err("cc_ops is null\n");
		return;
	}

	if (old_status != status)
		cc_ops->typec_set_status(status);
	old_status = status;
}

int pogopin_typec_chg_ana_audio_suport(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return 0;

	return di->typec_chg_ana_audio_suport;
}

bool pogopin_5pin_get_fcp_support(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return TRUE;

	return di->fcp_support;
}

void pogopin_5pin_completion_devoff(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return;

	complete(&di->dev_off_completion);
}

void pogopin_5pin_reinit_completion_devoff(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return;

	reinit_completion(&di->dev_off_completion);
}

unsigned long pogopin_5pin_wait_for_completion(unsigned long timeout)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return 0;

	return wait_for_completion_timeout(&di->dev_off_completion,
		msecs_to_jiffies(timeout));
}

void pogopin_5pin_int_irq_disable(bool en)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return;

	if (di->pogopin_int_irq <= 0) {
		hwlog_err("pogopin_int_irq is null\n");
		return;
	}

	if (en)
		disable_irq_nosync(di->pogopin_int_irq);
	else
		enable_irq(di->pogopin_int_irq);
}

void pogopin_set_typec_state(int typec_state)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return;

	if ((typec_state == PD_DPM_USB_TYPEC_NONE) ||
		(typec_state == PD_DPM_USB_TYPEC_DETACHED) ||
		(typec_state == PD_DPM_USB_TYPEC_AUDIO_DETACHED))
		di->typec_state = STATE_OFF;
	else
		di->typec_state = STATE_ON;
}

int pogopin_get_interface_status(void)
{
	int pogopin_int_value;
	int typec_int_value;
	struct pogopin_cc_ops *cc_ops = NULL;
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return NO_INTERFACE;

	cc_ops = pogopin_get_cc_ops();
	if (!cc_ops && !di->is_hw_pd)
		return NO_INTERFACE;

	if ((di->typecvbus_from_pd == TRUE) && !di->is_hw_pd) {
		if (!cc_ops->typec_detect_vbus) {
			hwlog_err("typec_detect_vbus is null\n");
			return NO_INTERFACE;
		}
	}

	pogopin_int_value = gpio_get_value(di->pogopin_int_gpio);
	if (di->typecvbus_from_pd == FALSE) {
		typec_int_value = gpio_get_value(di->typec_int_gpio);
	} else {
		if (di->is_hw_pd && (di->pin_num == POGOPIN_3PIN))
			typec_int_value = !di->typec_state;
		else
			typec_int_value = !cc_ops->typec_detect_vbus();
	}

	hwlog_info("pogopin_int=%d, typec_int=%d\n",
		pogopin_int_value, typec_int_value);

	if ((pogopin_int_value == HIGH) && (typec_int_value == HIGH))
		return NO_INTERFACE;
	else if ((pogopin_int_value == HIGH) && (typec_int_value == LOW))
		return TYPEC_INTERFACE;
	else if ((pogopin_int_value == LOW) && (typec_int_value == HIGH))
		return POGOPIN_INTERFACE;
	else if ((pogopin_int_value == LOW) && (typec_int_value == LOW))
		return POGOPIN_AND_TYPEC;

	return NO_INTERFACE;
}
EXPORT_SYMBOL_GPL(pogopin_get_interface_status);

void pogopin_cc_register_ops(struct pogopin_cc_ops *ops)
{
	if (ops) {
		hwlog_info("pogopin cc ops register ok\n");
		g_pogopin_cc_ops = ops;
	} else {
		hwlog_err("pogopin cc ops register fail\n");
	}
}

static bool pogopin_get_pin_num(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return POGOPIN_3PIN;

	return di->pin_num;
}

bool pogopin_otg_from_buckboost(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return FALSE;

	return di->typec_otg_use_buckboost;
}

bool pogopin_is_support(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return FALSE;

	return di->pogopin_support;
}

int pogopin_3pin_get_input_current(void)
{
	return POGOPIN_IIN_CURRENT;
}

int pogopin_3pin_get_charger_current(void)
{
	return POGOPIN_ICHG_CURRENT;
}

int pogopin_3pin_register_pogo_vbus_notifier(struct notifier_block *nb)
{
	if (!nb)
		return -EINVAL;

	return blocking_notifier_chain_register(&g_pogopin_vbus_evt_nh, nb);
}

#ifdef CONFIG_SYSFS
#define POGOPIN_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, pogopin_sysfs_show, store), \
	.name = POGOPIN_SYSFS_##n, \
}

#define POGOPIN_SYSFS_FIELD_RO(_name, n) \
	POGOPIN_SYSFS_FIELD(_name, n, 0444, NULL)

static ssize_t pogopin_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct pogopin_sysfs_info pogopin_sysfs_tbl[] = {
	POGOPIN_SYSFS_FIELD_RO(interface_type, INTERFACE_TYPE),
};

static struct attribute *pogopin_sysfs_attrs[ARRAY_SIZE(pogopin_sysfs_tbl) + 1];

static const struct attribute_group pogopin_sysfs_attr_group = {
	.attrs = pogopin_sysfs_attrs,
};

static void pogopin_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(pogopin_sysfs_tbl);

	for (i = 0; i < limit; i++)
		pogopin_sysfs_attrs[i] = &pogopin_sysfs_tbl[i].attr.attr;

	pogopin_sysfs_attrs[limit] = NULL;
}

static struct pogopin_sysfs_info *pogopin_sysfs_field_lookup(const char *name)
{
	int i;
	int limit = ARRAY_SIZE(pogopin_sysfs_tbl);
	int len;

	if (!name) {
		hwlog_err("name is null\n");
		return NULL;
	}

	len = strlen(name);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, pogopin_sysfs_tbl[i].attr.attr.name, len))
			break;
	}

	if (i >= limit)
		return NULL;

	return &pogopin_sysfs_tbl[i];
}

static ssize_t pogopin_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct pogopin_sysfs_info *info = NULL;
	int len = 0;
	int status = pogopin_get_interface_status();

	info = pogopin_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("info is null\n");
		return -EINVAL;
	}

	switch (info->name) {
	case POGOPIN_SYSFS_INTERFACE_TYPE:
		len = snprintf(buf, PAGE_SIZE, "%u\n", status);
		break;
	default:
		break;
	}

	return len;
}

static int pogopin_sysfs_create_group(struct pogopin_info *di)
{
	if (!di || !di->pdev) {
		hwlog_err("di or pdev is null\n");
		return -EINVAL;
	}

	pogopin_sysfs_init_attrs();
	return sysfs_create_group(&di->pdev->dev.kobj,
		&pogopin_sysfs_attr_group);
}

static inline void pogopin_sysfs_remove_group(struct pogopin_info *di)
{
	sysfs_remove_group(&di->pdev->dev.kobj, &pogopin_sysfs_attr_group);
}
#else
static int pogopin_sysfs_create_group(struct charge_device_info *di)
{
	return 0;
}

static inline void pogopin_sysfs_remove_group(struct charge_device_info *di)
{
}
#endif /* CONFIG_SYSFS */

/* simulated vbus detect process to solve vbus unchange when switch ovp */
void pogopin_vbus_detect(bool status)
{
	int typec_int_value_old;
	int typec_int_value_new;
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return;

	msleep(400); /* wait 400ms for vbus change completed */
	typec_int_value_old = gpio_get_value(di->typec_int_gpio);
	hwlog_info("typec_int_value:%d,status:%d\n", typec_int_value_old, status);

	if ((status == TRUE) && (typec_int_value_old == HIGH))
		return;

	pogopin_event_notify(POGOPIN_VBUS_DISCONNECT);
	msleep(POGOPIN_MOS_DELAYED); /* wait for disconnect completed */
	typec_int_value_new = gpio_get_value(di->typec_int_gpio);
	if (((status == TRUE) && (typec_int_value_old == LOW)) ||
		((status == FALSE) && (typec_int_value_new == LOW)))
		pogopin_event_notify(POGOPIN_VBUS_CONNECT);
}

static void pogopin_5pin_pogo_vbus_in(void)
{
	unsigned long timeout;
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return;

	hwlog_info("pogopin in handle\n");

	pogopin_5pin_set_fcp_support(FALSE);

	hwlog_info("typec detect disable,wait disable completed\n");
	if (di->typec_chg_ana_audio_suport)
		pogopin_5pin_typec_set_status(POGO_CHARGER);
	else
		pogopin_5pin_typec_detect_disable(TRUE);
	if (di->current_int_status == POGOPIN_AND_TYPEC) {
		msleep(POGOPIN_TIMEOUT_500MS);
	} else {
		timeout = wait_for_completion_timeout(&di->dev_off_completion,
			msecs_to_jiffies(POGOPIN_TIMEOUT_50MS));
		hwlog_info("wait typec disable timeout=%ld\n", timeout);
	}

	pogopin_5pin_vbus_in_switch_from_typec();
	if (di->vbus_detect)
		pogopin_vbus_detect(TRUE);
	hisi_usb_otg_bc_again();
	reinit_completion(&di->dev_off_completion);
}

static void pogopin_5pin_pogo_vbus_out(void)
{
	unsigned long timeout;
	struct pogopin_info *di = pogopin_get_dev_info();

	if (!di)
		return;

	hwlog_info("pogopin out handle\n");

	pogopin_5pin_set_fcp_support(TRUE);

	pogopin_5pin_remove_switch_to_typec();
	timeout = wait_for_completion_timeout(&di->dev_off_completion,
		msecs_to_jiffies(POGOPIN_TIMEOUT_500MS));

	hwlog_info("typec detect enable, timeout=%ld\n", timeout);
	if (di->vbus_detect)
		pogopin_vbus_detect(FALSE);
	if (di->typec_chg_ana_audio_suport)
		pogopin_5pin_typec_set_status(POGO_NONE);
	else
		pogopin_5pin_typec_detect_disable(FALSE);
	reinit_completion(&di->dev_off_completion);
}

static void pogopin_5pin_pogo_vbus_irq_handle(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();
	int pogopin_gpio_value;
	int buck_boost_gpio_value;
	int need_do_pogopin_switch = TRUE;
	int now_is_typec;
	int pogopin_int_statue;

	if (!di)
		return;

	now_is_typec = pogopin_5pin_is_usbswitch_at_typec();

	/* only when buck boost leads pogpopin vbus int not need switch */
	buck_boost_gpio_value = gpio_get_value(di->buck_boost_gpio);
	if (buck_boost_gpio_value)
		need_do_pogopin_switch = FALSE;

	hwlog_info("buck_boost_gpio=%d\n", buck_boost_gpio_value);

	pogopin_gpio_value = gpio_get_value(di->pogopin_int_gpio);
	if (di->pogopin_int_debounce) {
		msleep(50); /* add sorftware debounce 50ms of pogopin insert handle */
		pogopin_int_statue = gpio_get_value(di->pogopin_int_gpio);
		hwlog_info("pogopin_gpio_value=%d,typec=%d,pogopin_int_statue:%d\n",
			pogopin_gpio_value, now_is_typec, pogopin_int_statue);
		if (pogopin_int_statue != pogopin_gpio_value)
			return;
	}

	if (need_do_pogopin_switch == FALSE)
		return;

	if (pogopin_gpio_value == LOW) {
		di->current_int_status = pogopin_get_interface_status();
		pogopin_5pin_pogo_vbus_in();
	} else if ((pogopin_gpio_value == HIGH) && !now_is_typec) {
		pogopin_5pin_pogo_vbus_out();
		di->current_int_status = pogopin_get_interface_status();
		if (di->typec_chg_ana_audio_suport)
			pogopin_event_notify(POGOPIN_CHARGER_OUT_COMPLETE);
	} else if (di->typec_chg_ana_audio_suport) {
		di->current_int_status = pogopin_get_interface_status();
		if (di->audio_switch_control && (pogopin_gpio_value == HIGH))
			usb_analog_hs_plug_in_out_handle(POGOPIN_PLUG_OUT);
		pogopin_5pin_set_fcp_support(TRUE);
		pogopin_5pin_typec_set_status(POGO_NONE);
	}
}

static void pogopin_3pin_dpm_notify_work(struct work_struct *work)
{
	hwlog_info("pogopin dpm notify work\n");
	blocking_notifier_call_chain(&g_pogopin_vbus_evt_nh,
		POGOPIN_CHARGER_INSERT, NULL);
}

static int pogopin_3pin_pd_dpm_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct pogopin_info *di = pogopin_get_dev_info();
	int pogo_int_gpio;
	int kb_ldo_gpio;

	if (!di)
		return NOTIFY_OK;

	/* typec report device remove,check pogo status again */
	if (event == CHARGER_TYPE_NONE) {
		kb_ldo_gpio = gpio_get_value(di->keyboard_ldo_gpio);
		pogo_int_gpio = gpio_get_value(di->pogopin_int_gpio);

		if ((pogo_int_gpio == LOW) && (kb_ldo_gpio == LOW)) {
			hwlog_info("typec remove,report pogopin charge\n");
			schedule_delayed_work(&di->dpm_notify_work,
				msecs_to_jiffies(POGOPIN_MOS_DELAYED));
		}
	}

	return NOTIFY_OK;
}

bool pogopin_3pin_ignore_pogo_vbus_in_event(void)
{
	int vbus_ch_state = VBUS_CH_STATE_CLOSE;
	int ret;
	struct pogopin_info *di = pogopin_get_dev_info();
	int pogopin_now_status;

	if (!di)
		return FALSE;

	if (di->pin_num != POGOPIN_3PIN) {
		hwlog_info("only pogopin 3pin do ignore\n");
		return FALSE;
	}

	if (!gpio_is_valid(di->pogopin_int_gpio)) {
		hwlog_err("gpio is not valid\n");
		return FALSE;
	}

	ret = vbus_ch_get_state(VBUS_CH_USER_POGOPIN,
		VBUS_CH_TYPE_POGOPIN_BOOST, &vbus_ch_state);
	if (ret) {
		hwlog_err("get typec buckboost status fail\n");
		return FALSE;
	}

	pogopin_now_status = pogopin_get_interface_status();

	/* typec otg in, ignore */
	if ((pogopin_now_status == POGOPIN_AND_TYPEC) &&
		(vbus_ch_state == VBUS_CH_STATE_OPEN)) {
		hwlog_info("now is otg connect,ignore vbus event\n");
		return TRUE;
	} else if (pogopin_now_status == POGOPIN_INTERFACE) {
		hwlog_info("only pogopin insert,ignore vbus event\n");
		return TRUE;
	}

	return FALSE;
}

static void pogopin_3pin_pogo_vbus_in(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();
	int keyboard_ldo_gpio;
	int typec_status;

	if (!di)
		return;

	keyboard_ldo_gpio = gpio_get_value(di->keyboard_ldo_gpio);
	hwlog_info("keyboard_ldo_gpio=%d\n", keyboard_ldo_gpio);

	if (keyboard_ldo_gpio == HIGH) {
		hwlog_info("now is keyboard ldo output\n");
		di->keyboard_connect = TRUE;
		return;
	}

	pd_dpm_get_typec_state(&typec_status);

	if (typec_status == PD_DPM_USB_TYPEC_DEVICE_ATTACHED) {
		hwlog_info("now is typec charger, ignore pogo in\n");
		return;
	}

	if (di->charge_otg_ctl) {
		hwlog_info("fix hw issue: close charger otg\n");
		charge_otg_mode_enable(OTG_DISABLE, VBUS_CH_USER_WIRED_OTG);
	}

	/* mos open is slow 100ms than gpio int */
	msleep(100);

	blocking_notifier_call_chain(&g_pogopin_vbus_evt_nh,
		POGOPIN_CHARGER_INSERT, NULL);
}

static void pogopin_3pin_pogo_vbus_out(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();
	int keyboard_ldo_gpio;
	int typec_status;
	int vbus_ch_state = VBUS_CH_STATE_CLOSE;

	if (!di)
		return;

	keyboard_ldo_gpio = gpio_get_value(di->keyboard_ldo_gpio);
	hwlog_info("keyboard_ldo_gpio=%d\n", keyboard_ldo_gpio);

	if ((di->keyboard_connect == true) && (keyboard_ldo_gpio == LOW)) {
		di->keyboard_connect = FALSE;
		hwlog_info("keyboard remove leads pogo vbus high,ignore\n");
		return;
	}

	pd_dpm_get_typec_state(&typec_status);

	if (typec_status == PD_DPM_USB_TYPEC_DEVICE_ATTACHED) {
		hwlog_info("now is typec charger, ignore pogo remove\n");
		return;
	}

	cancel_delayed_work(&di->dpm_notify_work);
	blocking_notifier_call_chain(&g_pogopin_vbus_evt_nh,
		POGOPIN_CHARGER_REMOVE, NULL);

	if (di->charge_otg_ctl) {
		vbus_ch_get_state(VBUS_CH_USER_POGOPIN,
			VBUS_CH_TYPE_POGOPIN_BOOST, &vbus_ch_state);
		if ((charge_get_charger_type() == CHARGER_REMOVED) &&
			(vbus_ch_state == VBUS_CH_STATE_OPEN)) {
			/* wait 100ms for charger remove done */
			msleep(100);
			hwlog_info("fix hw issue: open charger otg\n");
			charge_otg_mode_enable(OTG_ENABLE, VBUS_CH_USER_WIRED_OTG);
		}
	}
}

static void pogopin_3pin_pogo_vbus_irq_handle(void)
{
	struct pogopin_info *di = pogopin_get_dev_info();
	int pogopin_gpio_value;

	if (!di)
		return;

	pogopin_gpio_value = gpio_get_value(di->pogopin_int_gpio);
	hwlog_info("pogo vbus changed, int_gpio=%d\n", pogopin_gpio_value);

	if (pogopin_gpio_value == HIGH)
		pogopin_3pin_pogo_vbus_out();
	else
		pogopin_3pin_pogo_vbus_in();
}

static void pogopin_vbus_irq_work(struct work_struct *work)
{
	bool pin_num = pogopin_get_pin_num();

	if (pin_num == POGOPIN_3PIN)
		pogopin_3pin_pogo_vbus_irq_handle();
	else if (pin_num == POGOPIN_5PIN)
		pogopin_5pin_pogo_vbus_irq_handle();
}

static void typec_switch_work(struct work_struct *work)
{
	struct pogopin_info *di = pogopin_get_dev_info();
	int otg_power_supply_val;
	int typec_value;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	disable_irq_nosync(di->typec_int_irq);
	otg_power_supply_val = gpio_get_value(di->buck_boost_gpio);
	typec_value = gpio_get_value(di->typec_int_gpio);
	enable_irq(di->typec_int_irq);

	hwlog_info("%s typec_value:%d otg_power_supply_val:%d\n",
		__func__, typec_value, otg_power_supply_val);

	if (otg_power_supply_val == LOW) {
		if (typec_value == LOW)
			pogopin_event_notify(POGOPIN_SAVE_CHARGER_TYPE);
		hwlog_info("pogopin otg absent\n");
		return;
	}

	if (typec_value == LOW) {
		pogopin_event_notify(POGOPIN_SAVE_CHARGER_TYPE);
		/* wait 200ms for typec vbus stabilize */
		msleep(POGOPIN_TIME_DELAYE_200MS);
		if (gpio_get_value(di->typec_int_gpio) == HIGH)
			return;
		pogopin_event_notify(POGOPIN_OTG_AND_CHG_START);
	} else {
		pogopin_event_notify(POGOPIN_OTG_AND_CHG_STOP);
	}
}

static irqreturn_t pogopin_int_handler(int irq, void *_di)
{
	struct pogopin_info *di = _di;
	int gpio_value;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_HANDLED;
	}

	disable_irq_nosync(di->pogopin_int_irq);
	gpio_value = gpio_get_value(di->pogopin_int_gpio);

	/* pogopin insert */
	if (gpio_value == LOW)
		irq_set_irq_type(di->pogopin_int_irq,
			IRQF_TRIGGER_HIGH | IRQF_NO_SUSPEND);
	else
		irq_set_irq_type(di->pogopin_int_irq,
			IRQF_TRIGGER_LOW | IRQF_NO_SUSPEND);

	enable_irq(di->pogopin_int_irq);
	schedule_work(&di->work);

	return IRQ_HANDLED;
}

void pogopin_otg_status_change_process(uint8_t value)
{
	struct pogopin_info *di = pogopin_get_dev_info();
	int times;

	hwlog_info("%s:%d\n", __func__, value);
	if (!di || !di->typec_chg_ana_audio_suport)
		return;

	switch (value) {
	case POGO_OTG:
		if (gpio_get_value(di->typec_int_gpio) == LOW) {
			/* wait for typec vbus value stabilize */
			msleep(POGOPIN_TIME_DELAYE_200MS);
			if (gpio_get_value(di->typec_int_gpio) == LOW)
				pogopin_event_notify(POGOPIN_OTG_AND_CHG_START);
		}
		break;
	case POGO_NONE:
		/*
		 * typec show charging status when pogo otg and typec charger plut out sometimes.
		 * set discharging when pogo plug out otg and typec plug out charger.
		 */
		for (times = 0; times < CHECK_TIMES; times++) {
			/* wait 200ms for pogo otg status stabilize */
			msleep(POGOPIN_TIME_DELAYE_200MS);
			if (gpio_get_value(di->typec_int_gpio) == HIGH) {
				pogopin_event_notify(POGOPIN_OTG_AND_CHG_STOP);
				break;
			}
		}
		break;
	default:
		hwlog_err("value is invalid\n");
		break;
	}
}

static irqreturn_t typec_int_handler(int irq, void *_di)
{
	struct pogopin_info *di = _di;
	int typec_value;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_HANDLED;
	}
	disable_irq_nosync(di->typec_int_irq);
	typec_value = gpio_get_value(di->typec_int_gpio);

	if (typec_value == LOW) /* typec insert */
		irq_set_irq_type(di->typec_int_irq,
			IRQF_TRIGGER_HIGH | IRQF_NO_SUSPEND);
	else /* typec remove */
		irq_set_irq_type(di->typec_int_irq,
			IRQF_TRIGGER_LOW | IRQF_NO_SUSPEND);

	enable_irq(di->typec_int_irq);
	schedule_work(&di->typec_int_work);
	return IRQ_HANDLED;
}

static int pogopin_request_irq(struct pogopin_info *di)
{
	int ret;
	int gpio_value;
	unsigned long irq_flag;

	di->pogopin_int_irq = gpio_to_irq(di->pogopin_int_gpio);
	if (di->pogopin_int_irq < 0) {
		hwlog_err("gpio map to irq fail\n");
		return -EINVAL;
	}
	hwlog_info("pogopin_int_irq=%d\n", di->pogopin_int_irq);

	gpio_value = gpio_get_value(di->pogopin_int_gpio);

	/* pogopin insert */
	if (gpio_value == LOW) {
		ret = request_irq(di->pogopin_int_irq, pogopin_int_handler,
			IRQF_TRIGGER_HIGH | IRQF_NO_SUSPEND,
			"pogopin_int_irq", di);
	} else {
		ret = request_irq(di->pogopin_int_irq, pogopin_int_handler,
			IRQF_TRIGGER_LOW | IRQF_NO_SUSPEND,
			"pogopin_int_irq", di);
	}

	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->pogopin_int_irq = -1;
	}

	if (di->typec_chg_ana_audio_suport) {
		/* typec irq register */
		di->typec_int_irq = gpio_to_irq(di->typec_int_gpio);
		if (di->typec_int_irq < 0) {
			hwlog_err("could not map pogopin_int_gpio to irq\n");
			return -EINVAL;
		}
		hwlog_info("di->typec_int_irq = %d\n", di->typec_int_irq);

		/* get the value of typec irq */
		gpio_value = gpio_get_value(di->typec_int_gpio);
		if (gpio_value == LOW) /* typec insert */
			irq_flag = IRQF_TRIGGER_HIGH;
		else /* typec remove */
			irq_flag = IRQF_TRIGGER_LOW;
		ret = request_irq(di->typec_int_irq, typec_int_handler,
			irq_flag | IRQF_NO_SUSPEND,
			"typec_int_irq", di);
		if (ret) {
			hwlog_err("could not request typec_int_irq\n");
			di->typec_int_irq = -EINVAL;
		}
	}

	return ret;
}

static int popogpin_set_gpio_direction_irq(struct pogopin_info *di)
{
	int ret;

	ret = gpio_direction_output(di->power_switch_gpio, 0);
	if (ret < 0) {
		hwlog_err("gpio set output fail\n");
		return ret;
	}

	ret = gpio_direction_input(di->pogopin_int_gpio);
	if (ret < 0) {
		hwlog_err("gpio set input fail\n");
		return ret;
	}

	if (di->pin_num == POGOPIN_5PIN) {
		ret = gpio_direction_output(di->usb_switch_gpio, 0);
		if (ret < 0) {
			hwlog_err("gpio set output fail\n");
			return ret;
		}

		if (!di->typec_vbus_unctrl) {
			ret = gpio_direction_output(di->switch_power_gpio, 0);
			if (ret < 0) {
				hwlog_err("gpio set output fail\n");
				return ret;
			}
		}
		ret = gpio_direction_output(di->buck_boost_gpio, 0);
		if (ret < 0) {
			hwlog_err("gpio set output fail\n");
			return ret;
		}
	}

	if (di->typecvbus_from_pd == FALSE) {
		ret = gpio_direction_input(di->typec_int_gpio);
		if (ret < 0) {
			hwlog_err("gpio set input fail\n");
			return ret;
		}
	}

	return pogopin_request_irq(di);
}

static void pogopin_free_irqs(struct pogopin_info *di)
{
	free_irq(di->pogopin_int_irq, di);
	if (di->typec_chg_ana_audio_suport)
		free_irq(di->typec_int_irq, di);
}

static int pogopin_parse_config_dts(struct pogopin_info *di,
	struct device_node *np)
{
	di->typec_vbus_unctrl =
		of_property_read_bool(np, "typec_vbus_unctrl");
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"audio_switch_control", &di->audio_switch_control, 0);
	di->pogopin_int_debounce =
		of_property_read_bool(np, "pogopin_int_debounce");
	di->vbus_detect =
		of_property_read_bool(np, "vbus_detect");
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"typec_chg_ana_audio_suport", &di->typec_chg_ana_audio_suport, 0);
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pin_num", &di->pin_num, 0))
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pogopin_typecvbus_from_pd", &di->typecvbus_from_pd, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"is_hw_pd", &di->is_hw_pd, 0);

	di->typec_otg_use_buckboost = of_property_read_bool(np,
		"typec_otg_use_buckboost");
	hwlog_info("otg_use_buckboost=%d\n", di->typec_otg_use_buckboost);

	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,vbus_channel_pogopin_boost",
		"pogopin_charge_otg_ctl", &di->charge_otg_ctl, 0);

	return 0;
}

static int pogopin_parse_gpio_dts(struct pogopin_info *di,
	struct device_node *np)
{
	int ret = -EINVAL;

	di->power_switch_gpio = of_get_named_gpio(np, "power_switch_gpio", 0);
	hwlog_info("power_switch_gpio=%d\n", di->power_switch_gpio);

	if (!gpio_is_valid(di->power_switch_gpio)) {
		hwlog_err("gpio is not valid\n");
		return ret;
	}

	/* pogopin vbus int gpio */
	di->pogopin_int_gpio = of_get_named_gpio(np, "pogopin_int_gpio", 0);
	hwlog_info("pogopin_int_gpio=%d\n", di->pogopin_int_gpio);

	if (!gpio_is_valid(di->pogopin_int_gpio)) {
		hwlog_err("gpio is not valid\n");
		return ret;
	}

	if (di->typec_otg_use_buckboost) {
		di->keyboard_ldo_gpio = of_get_named_gpio(np,
			"keyboard_ldo_gpio", 0);
		hwlog_info("keyboard_ldo_gpio=%d\n", di->keyboard_ldo_gpio);

		if (!gpio_is_valid(di->keyboard_ldo_gpio)) {
			hwlog_err("gpio is not valid\n");
			return ret;
		}
	}

	if (di->pin_num == POGOPIN_5PIN) {
		if (!di->typec_vbus_unctrl) {
			di->switch_power_gpio = of_get_named_gpio(np,
				"switch_power_gpio", 0);
			hwlog_info("switch_power_gpio=%d\n",
				di->switch_power_gpio);

			if (!gpio_is_valid(di->switch_power_gpio)) {
				hwlog_err("gpio is not valid\n");
				return ret;
			}
		}

		/* usb D+,D- switch GPIO */
		di->usb_switch_gpio = of_get_named_gpio(np,
			"usb_switch_gpio", 0);
		hwlog_info("usb_switch_gpio=%d\n", di->usb_switch_gpio);

		if (!gpio_is_valid(di->usb_switch_gpio)) {
			hwlog_err("gpio is not valid\n");
			return ret;
		}

		/* buck boost gpio for pogo otg */
		di->buck_boost_gpio = of_get_named_gpio(np,
			"buck_boost_gpio", 0);
		hwlog_info("buck_boost_gpio=%d\n", di->buck_boost_gpio);

		if (!gpio_is_valid(di->buck_boost_gpio)) {
			hwlog_err("gpio is not valid\n");
			return ret;
		}
	}

	if (di->typecvbus_from_pd == FALSE) {
		/* typec vbus int gpio */
		di->typec_int_gpio = of_get_named_gpio(np, "typec_int_gpio", 0);
		hwlog_info("typec_int_gpio=%d\n", di->typec_int_gpio);

		if (!gpio_is_valid(di->typec_int_gpio)) {
			hwlog_err("gpio is not valid\n");
			return ret;
		}
	}

	return 0;
}

static int pogopin_request_common_gpio(struct pogopin_info *di)
{
	int ret;

	ret = gpio_request(di->power_switch_gpio, "power_switch");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto fail_power_switch_gpio;
	}

	ret = gpio_request(di->pogopin_int_gpio, "pogopin_int");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto fail_pogopin_int_gpio;
	}

	return 0;

fail_pogopin_int_gpio:
	gpio_free(di->power_switch_gpio);
fail_power_switch_gpio:
	return ret;
}

static void pogopin_free_common_gpio(struct pogopin_info *di)
{
	gpio_free(di->pogopin_int_gpio);
	gpio_free(di->power_switch_gpio);
}

static int pogopin_request_cust_gpio(struct pogopin_info *di)
{
	int ret;

	if (di->typecvbus_from_pd == FALSE) {
		ret = gpio_request(di->typec_int_gpio, "typec_int");
		if (ret) {
			hwlog_err("gpio request fail\n");
			return ret;
		}
	}

	if (di->pin_num == POGOPIN_5PIN) {
		if (!di->typec_vbus_unctrl) {
			ret = gpio_request(di->switch_power_gpio,
				"switch_power");
			if (ret) {
				hwlog_err("gpio request fail\n");
				goto fail_switch_power_gpio;
			}
		}

		ret = gpio_request(di->usb_switch_gpio, "usb_switch");
		if (ret) {
			hwlog_err("gpio request fail\n");
			goto fail_usb_switch_gpio;
		}

		ret = gpio_request(di->buck_boost_gpio, "buck_boost");
		if (ret) {
			hwlog_err("gpio request fail\n");
			goto fail_buck_boost_gpio;
		}
	}

	return 0;

fail_buck_boost_gpio:
	gpio_free(di->usb_switch_gpio);
fail_usb_switch_gpio:
	if (!di->typec_vbus_unctrl)
		gpio_free(di->switch_power_gpio);
fail_switch_power_gpio:
	if (di->typecvbus_from_pd == FALSE)
		gpio_free(di->typec_int_gpio);
	return ret;
}

static int pogopin_parse_and_request_gpios(struct pogopin_info *di,
	struct device_node *np)
{
	int ret;

	ret = pogopin_parse_gpio_dts(di, np);
	if (ret != 0)
		return ret;

	ret = pogopin_request_common_gpio(di);
	if (ret != 0)
		return ret;

	ret = pogopin_request_cust_gpio(di);
	if (ret != 0)
		goto fail_request_gpio;

	return 0;

fail_request_gpio:
	pogopin_free_common_gpio(di);
	return ret;
}

static void pogopin_free_gpios(struct pogopin_info *di)
{
	pogopin_free_common_gpio(di);

	if (di->pin_num == POGOPIN_5PIN) {
		gpio_free(di->buck_boost_gpio);
		if (!di->typec_vbus_unctrl)
			gpio_free(di->switch_power_gpio);
		gpio_free(di->usb_switch_gpio);
	}
	if (di->typecvbus_from_pd == 0)
		gpio_free(di->typec_int_gpio);
}

static int pogopin_init_gpios(struct pogopin_info *di, struct device_node *np)
{
	int ret;

	ret = pogopin_parse_config_dts(di, np);
	if (ret)
		return ret;

	ret = pogopin_parse_and_request_gpios(di, np);
	if (ret)
		return ret;

	ret = popogpin_set_gpio_direction_irq(di);
	if (ret)
		goto fail_request_gpio;

	return 0;

fail_request_gpio:
	pogopin_free_gpios(di);
	return ret;
}

static int pogopin_init_sysfs_and_class(struct pogopin_info *di)
{
	int ret;

	ret = pogopin_sysfs_create_group(di);
	if (ret) {
		hwlog_err("cannot create group\n");
		return ret;
	}

	g_pogopin_class = class_create(THIS_MODULE, "hw_pogopin");
	if (IS_ERR(g_pogopin_class)) {
		hwlog_err("cannot create class\n");
		return -EINVAL;
	}

	if (!g_pogopin_dev) {
		g_pogopin_dev = device_create(g_pogopin_class, NULL, 0,
			NULL, "pogopin");
		if (!g_pogopin_dev) {
			hwlog_err("sysfs device create failed\n");
			ret = -EINVAL;
			goto fail_create_sysfs;
		}

		ret = sysfs_create_link(&g_pogopin_dev->kobj,
				&di->pdev->dev.kobj, "pogopin_data");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			goto fail_create_sysfs;
		}
	}

	return 0;

fail_create_sysfs:
	pogopin_sysfs_remove_group(di);
	return ret;
}

static int pogopin_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = NULL;
	struct device *dev = NULL;
	struct pogopin_info *di = NULL;

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	platform_set_drvdata(pdev, di);
	di->typec_state = STATE_OFF;
	g_pogopin_di = di;
	di->pdev = pdev;
	dev = &pdev->dev;
	np = dev->of_node;

	ret = pogopin_init_gpios(di, np);
	if (ret)
		goto fail_init_gpio;

	if ((di->pin_num != POGOPIN_3PIN) || !di->is_hw_pd) {
		if (g_pogopin_cc_ops) {
			di->ops = g_pogopin_cc_ops;
		} else {
			hwlog_err("cc ops is null\n");
			goto fail_cc_ops_register;
		}
	}

	if (di->pin_num == POGOPIN_3PIN) {
		di->tcpc_nb.notifier_call = pogopin_3pin_pd_dpm_notifier_call;
		ret = register_pd_dpm_notifier(&di->tcpc_nb);
		if (ret < 0) {
			hwlog_err("register_pd_dpm_notifier failed\n");
			goto fail_pd_dpm_notifier;
		}
		INIT_DELAYED_WORK(&di->dpm_notify_work,
			pogopin_3pin_dpm_notify_work);
	}

	INIT_WORK(&di->work, pogopin_vbus_irq_work);
	init_completion(&di->dev_off_completion);
	if (di->typec_chg_ana_audio_suport) {
		INIT_WORK(&di->typec_int_work, typec_switch_work);
		typec_int_handler(di->typec_int_irq, di);
	}
	di->current_int_status = pogopin_get_interface_status();
	hwlog_info("current_int_status=%d\n", di->current_int_status);

	di->fcp_support = true;
	di->pogopin_support = true;

	pogopin_int_handler(di->pogopin_int_irq, di);

	ret = pogopin_init_sysfs_and_class(di);
	if (ret)
		goto fail_create_sysfs;

	hwlog_info("probe end\n");
	return 0;

fail_create_sysfs:
fail_pd_dpm_notifier:
fail_cc_ops_register:
	pogopin_free_irqs(di);
	pogopin_free_gpios(di);
fail_init_gpio:
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);

	return ret;
}

static int pogopin_remove(struct platform_device *pdev)
{
	struct pogopin_info *di = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	pogopin_sysfs_remove_group(di);
	pogopin_free_irqs(di);
	pogopin_free_gpios(di);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_pogopin_di = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id pogopin_match_table[] = {
	{
		.compatible = "huawei,pogopin",
		.data = NULL,
	},
	{},
};

static struct platform_driver pogopin_driver = {
	.probe = pogopin_probe,
	.remove = pogopin_remove,
	.driver = {
		.name = "huawei,pogopin",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(pogopin_match_table),
	},
};

static int __init pogopin_init(void)
{
	return platform_driver_register(&pogopin_driver);
}

static void __exit pogopin_exit(void)
{
	platform_driver_unregister(&pogopin_driver);
}

late_initcall(pogopin_init);
module_exit(pogopin_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei pogopin module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
