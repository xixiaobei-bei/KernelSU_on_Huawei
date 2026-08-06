/*
 * hw_typec.c
 *
 * typec driver
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
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/wakelock.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/version.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_typec_dev.h>
#include <huawei_platform/usb/hw_typec_platform.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <huawei_platform/usb/switch/switch_ap/switch_usb_class.h>
#ifdef CONFIG_DUAL_ROLE_USB_INTF
#include <linux/usb/class-dual-role.h>
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG hw_typec
HWLOG_REGIST();

struct typec_device_info *g_typec_dev;
struct class *typec_class;
struct device *typec_dev;

void typec_wake_lock(struct typec_device_info *di)
{
	if (!wake_lock_active(&di->wake_lock)) {
		hwlog_err("typec wake lock\n");
		wake_lock(&di->wake_lock);
	}
}

void typec_wake_unlock(struct typec_device_info *di)
{
	if (wake_lock_active(&di->wake_lock)) {
		hwlog_err("typec wake unlock\n");
		wake_unlock(&di->wake_lock);
	}
}

int typec_current_notifier_register(struct notifier_block *nb)
{
	struct typec_device_info *di = g_typec_dev;

	if (!di || !nb) {
		hwlog_err("di or nb is null\n");
		return -EINVAL;
	}

	return blocking_notifier_chain_register(&di->typec_current_nh, nb);
}

static void typec_current_notifier_call(struct typec_device_info *di)
{
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	blocking_notifier_call_chain(&di->typec_current_nh,
		TYPEC_CURRENT_CHANGE, NULL);
}

/*
 * start otg work by calling related modules.
 * usbswitch_common_manual_sw() is to connect USB signal path.
 * hisi_usb_id_change() is to open VBUS to charge slave devices.
 */
static void typec_open_otg(void)
{
	usbswitch_common_dcd_timeout_enable(true);
	usbswitch_common_manual_sw(FSA9685_USB1_ID_TO_IDBYPASS);

	mdelay(10); /* delay 10ms */

	hisi_usb_id_change(ID_FALL_EVENT);
}

/*
 * stop otg work as a pair of typec_open_otg().
 */
static void typec_close_otg(void)
{
	usbswitch_common_dcd_timeout_enable(false);

	hisi_usb_id_change(ID_RISE_EVENT);
}

/*
 * clean interrupt mask and wait for the next interrupt.
 */
static int typec_clean_int_mask(void)
{
	struct typec_device_info *di = g_typec_dev;

	if (!di || !di->ops || !di->ops->clean_int_mask) {
		hwlog_err("di or ops or clean_int_mask is null\n");
		return -ENOMEM;
	}

	return di->ops->clean_int_mask();
}

/*
 * detect the usb state(attach or detach).
 */
static int typec_detect_attachment_status(void)
{
	struct typec_device_info *di = g_typec_dev;

	if (!di || !di->ops || !di->ops->detect_attachment_status) {
		hwlog_err("di or ops or detect_attachment_status is null\n");
		return -ENOMEM;
	}

	return di->ops->detect_attachment_status();
}

/*
 * detect inserted plug orientation of typec devices.
 */
int typec_detect_cc_orientation(void)
{
	struct typec_device_info *di = g_typec_dev;

	if (!di || !di->ops || !di->ops->detect_cc_orientation) {
		hwlog_err("di or ops or detect_cc_orientation is null\n");
		return -ENOMEM;
	}

	return di->ops->detect_cc_orientation();
}
EXPORT_SYMBOL_GPL(typec_detect_cc_orientation);

/*
 * detect typec protocol defined current,
 * which may be called by Charge IC or other related modules.
 */
int typec_detect_current_mode(void)
{
	struct typec_device_info *di = g_typec_dev;

	if (!di || !di->ops || !di->ops->detect_input_current) {
		hwlog_err("di or ops or detect_input_current is null\n");
		return -ENOMEM;
	}

	return di->ops->detect_input_current();
}
EXPORT_SYMBOL_GPL(typec_detect_current_mode);

/*
 * detect port mode as a result of ID detection on cc pins.
 * DFP means host, while UFP means slave.
 * As a distinctive mode of typec protocol, DRP can switch between
 * DFP and UFP alternatively with the management of driver.
 */
int typec_detect_port_mode(void)
{
	struct typec_device_info *di = g_typec_dev;

	if (!di || !di->ops || !di->ops->detect_port_mode) {
		hwlog_err("di or ops or detect_port_mode is null\n");
		return -ENOMEM;
	}

	return di->ops->detect_port_mode();
}
EXPORT_SYMBOL_GPL(typec_detect_port_mode);

/*
 * create a device node to control typec protocol current
 * when as a DFP host, charging the UFP slave one.
 */
static ssize_t typec_output_current_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u8 value;
	struct typec_device_info *di = g_typec_dev;

	if (!di || !di->ops || !di->ops->ctrl_output_current) {
		hwlog_err("di or ops or ctrl_output_current is null\n");
		return -ENOMEM;
	}

	if (kstrtou8(buf, 0, &value) < 0) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	switch (value) {
	case TYPEC_HOST_CURRENT_DEFAULT:
		di->ops->ctrl_output_current(TYPEC_HOST_CURRENT_DEFAULT);
		break;

	case TYPEC_HOST_CURRENT_MID:
		di->ops->ctrl_output_current(TYPEC_HOST_CURRENT_MID);
		break;

	case TYPEC_HOST_CURRENT_HIGH:
		di->ops->ctrl_output_current(TYPEC_HOST_CURRENT_HIGH);
		break;

	default:
		hwlog_err("invalid value(%d)\n", value);
		return -EINVAL;
	}

	return size;
}

/*
 * create a device node to control port mode, which can be
 * used to choose host and slave alternatively.
 */
static ssize_t typec_port_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u8 value;
	struct typec_device_info *di = g_typec_dev;

	if (!di || !di->ops || !di->ops->ctrl_port_mode) {
		hwlog_err("di or ops or ctrl_port_mode is null\n");
		return -ENOMEM;
	}

	if (kstrtou8(buf, 0, &value) < 0) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	switch (value) {
	case TYPEC_HOST_PORT_MODE_DFP:
		di->ops->ctrl_port_mode(TYPEC_HOST_PORT_MODE_DFP);
		break;

	case TYPEC_HOST_PORT_MODE_UFP:
		di->ops->ctrl_port_mode(TYPEC_HOST_PORT_MODE_UFP);
		break;

	case TYPEC_HOST_PORT_MODE_DRP:
		di->ops->ctrl_port_mode(TYPEC_HOST_PORT_MODE_DRP);
		break;

	default:
		hwlog_err("invalid value(%d)\n", value);
		return -EINVAL;
	}

	return size;
}

/*
 * create a device node for userspace to check whether cc1 or cc2 has detected.
 */
static ssize_t typec_cc_orientation_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", typec_detect_cc_orientation());
}

static DEVICE_ATTR(ctrl_output_current, 0200, NULL, typec_output_current_store);
static DEVICE_ATTR(ctrl_port_mode, 0200, NULL, typec_port_mode_store);
static DEVICE_ATTR(cc_orientation, 0444, typec_cc_orientation_show, NULL);

static struct attribute *typec_ctrl_attributes[] = {
	&dev_attr_ctrl_output_current.attr,
	&dev_attr_ctrl_port_mode.attr,
	&dev_attr_cc_orientation.attr,
	NULL,
};

static const struct attribute_group typec_attr_group = {
	.attrs = typec_ctrl_attributes,
};

#ifdef CONFIG_DUAL_ROLE_USB_INTF
static enum dual_role_property fusb_drp_properties[] = {
	DUAL_ROLE_PROP_MODE,
	DUAL_ROLE_PROP_PR,
	DUAL_ROLE_PROP_DR,
};

/*
 * callback for "cat /sys/class/dual_role_usb/otg_default/<property>"
 */
static int dual_role_get_local_prop(struct dual_role_phy_instance *dual_role,
	enum dual_role_property prop, unsigned int *val)
{
	struct typec_device_info *di = dual_role_get_drvdata(dual_role);
	int port_mode;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -EINVAL;
	}

	port_mode = typec_detect_port_mode();

	if (port_mode == TYPEC_DEV_PORT_MODE_DFP) {
		if (prop == DUAL_ROLE_PROP_MODE)
			*val = DUAL_ROLE_PROP_MODE_DFP;
		else if (prop == DUAL_ROLE_PROP_PR)
			*val = DUAL_ROLE_PROP_PR_SRC;
		else if (prop == DUAL_ROLE_PROP_DR)
			*val = DUAL_ROLE_PROP_DR_HOST;
		else
			return -EINVAL;
	} else if (port_mode == TYPEC_DEV_PORT_MODE_UFP) {
		if (prop == DUAL_ROLE_PROP_MODE)
			*val = DUAL_ROLE_PROP_MODE_UFP;
		else if (prop == DUAL_ROLE_PROP_PR)
			*val = DUAL_ROLE_PROP_PR_SNK;
		else if (prop == DUAL_ROLE_PROP_DR)
			*val = DUAL_ROLE_PROP_DR_DEVICE;
		else
			return -EINVAL;
	} else {
		if (prop == DUAL_ROLE_PROP_MODE)
			*val = DUAL_ROLE_PROP_MODE_NONE;
		else if (prop == DUAL_ROLE_PROP_PR)
			*val = DUAL_ROLE_PROP_PR_NONE;
		else if (prop == DUAL_ROLE_PROP_DR)
			*val = DUAL_ROLE_PROP_DR_NONE;
		else
			return -EINVAL;
	}

	return 0;
}

/*
 * decides whether userspace can change a specific property.
 */
static int dual_role_is_writeable(struct dual_role_phy_instance *drp,
	enum dual_role_property prop)
{
	if (prop == DUAL_ROLE_PROP_MODE)
		return 1;
	else
		return 0;
}

/*
 * 1. check to see if current attached_state is same as requested state
 * if yes, then, return.
 * 2. disonect current session
 * 3. set approrpriate mode (dfp or ufp)
 * 4. wait for 1.5 secs to see if we get into the corresponding target state
 * if yes, return
 * 5. if not, fallback to Try.SNK
 * 6. wait for 1.5 secs to see if we get into one of the attached states
 * 7. return -EIO
 * also we have to fallback to Try.SNK state machine on cable disconnect
 */
static int dual_role_set_mode_prop(struct dual_role_phy_instance *dual_role,
	enum dual_role_property prop, const unsigned int *val)
{
	struct typec_device_info *di = dual_role_get_drvdata(dual_role);
	int port_mode;
	int timeout;
	int ret = 0;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -EINVAL;
	}

	if (*val != DUAL_ROLE_PROP_MODE_DFP && *val != DUAL_ROLE_PROP_MODE_UFP)
		return -EINVAL;

	port_mode = typec_detect_port_mode();

	if (port_mode != TYPEC_DEV_PORT_MODE_DFP &&
		port_mode != TYPEC_DEV_PORT_MODE_UFP)
		return 0;

	if (port_mode == TYPEC_DEV_PORT_MODE_DFP &&
		*val == DUAL_ROLE_PROP_MODE_DFP)
		return 0;

	if (port_mode == TYPEC_DEV_PORT_MODE_UFP &&
		*val == DUAL_ROLE_PROP_MODE_UFP)
		return 0;

	hwlog_info("set_mode_prop begin (mode=%d,val=%d)\n", port_mode, *val);

	mutex_lock(&di->typec_lock);

	if (port_mode == TYPEC_DEV_PORT_MODE_DFP) {
		/* as DFP now, try reversing, form source to sink */
		hwlog_info("try reversing, form Source to Sink\n");

		di->reverse_state = REVERSE_ATTEMPT;
		di->sink_attached = 0;

		/* turn off VBUS first */
		typec_close_otg();
		msleep(WAIT_VBUS_OFF_MS);

		di->ops->ctrl_port_mode(TYPEC_HOST_PORT_MODE_UFP);
	} else if (port_mode == TYPEC_DEV_PORT_MODE_UFP) {
		/* as UFP now, try reversing, form Sink to Source */
		hwlog_err("try reversing, form Sink to Source\n");

		/* reverse to DFP, from sink to source  */
		di->reverse_state = REVERSE_ATTEMPT;
		/* set current advertisement to default */
		di->ops->ctrl_output_current(TYPEC_HOST_CURRENT_DEFAULT);

		di->ops->ctrl_port_mode(TYPEC_HOST_PORT_MODE_DFP);
	}

	mutex_unlock(&di->typec_lock);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
	reinit_completion(&di->reverse_completion);
#else
	INIT_COMPLETION(di->reverse_completion);
#endif /* LINUX_VERSION_CODE */

	timeout = wait_for_completion_timeout(&di->reverse_completion,
		msecs_to_jiffies(DUAL_ROLE_SET_MODE_WAIT_MS));
	if (!timeout) {
		hwlog_err("reverse failed, set mode to DRP\n");

		/* set mode to DRP */
		di->ops->ctrl_port_mode(TYPEC_HOST_PORT_MODE_DRP);
		di->reverse_state = 0;

		hwlog_err("wait for the attached state\n");

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
		reinit_completion(&di->reverse_completion);
#else
		INIT_COMPLETION(di->reverse_completion);
#endif /* LINUX_VERSION_CODE */

		wait_for_completion_timeout(&di->reverse_completion,
			msecs_to_jiffies(DUAL_ROLE_SET_MODE_WAIT_MS));

		ret = -EIO;
	}

	hwlog_info("set_mode_prop end (ret=%d)\n", ret);
	return ret;
}

/*
 * callback for "echo <value> > /sys/class/dual_role_usb/<name>/<property>"
 * block until the entire final state is reached.
 * blocking is one of the better ways to signal when the operation is done.
 * This function tries to switch to Attached.SRC or Attached.SNK
 * by forcing the mode into SRC or SNK.
 * on failure, we fall back to Try.SNK state machine.
 */
static int dual_role_set_prop(struct dual_role_phy_instance *dual_role,
	enum dual_role_property prop, const unsigned int *val)
{
	if (prop == DUAL_ROLE_PROP_MODE)
		return dual_role_set_mode_prop(dual_role, prop, val);
	else
		return -EINVAL;
}

static void typec_wdog_work(struct work_struct *work)
{
	struct typec_device_info *di = NULL;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct typec_device_info, g_wdog_work.work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (!di->ops || !di->ops->ctrl_port_mode) {
		hwlog_err("ops or ctrl_port_mode is null\n");
		return;
	}

	di->ops->ctrl_port_mode(TYPEC_HOST_PORT_MODE_DRP);

	/* to notify userspace that the state might have changed */
	if (di->dual_role)
		dual_role_instance_changed(di->dual_role);
}
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

static void typec_attach(struct typec_device_info *di)
{
	int port_mode;
	int input_current;
	int cc_orient;

	port_mode = typec_detect_port_mode();

	/* Type-C attached as DFP */
	if (port_mode == TYPEC_DEV_PORT_MODE_DFP) {
		if (di->typec_trigger_otg) {
#ifdef CONFIG_DUAL_ROLE_USB_INTF
			hwlog_info("attached as DFP/Source\n");
			if (di->reverse_state == REVERSE_ATTEMPT) {
				hwlog_info("reversed success, Sink detected\n");

				di->reverse_state = REVERSE_COMPLETE;
				di->sink_attached = 1;

				/* turn on VBUS */
				typec_open_otg();

				/* notify the attached state */
				complete(&di->reverse_completion);
			} else if (di->trysnk_attempt) {
				hwlog_info("TrySNK fail, Sink detected again\n");

				/* TryNK has been attempted, clear the flag */
				di->trysnk_attempt = 0;
				cancel_delayed_work(&di->g_wdog_work);

				/* turn on VBUS */
				di->sink_attached = 1;
				typec_open_otg();

				/* notify the attached state */
				complete(&di->reverse_completion);
			} else {
				hwlog_info("Sink detected, perform TrySNK\n");
				di->trysnk_attempt = 1;
				/* set current advertisement to default */
				di->ops->ctrl_output_current(
					TYPEC_HOST_CURRENT_DEFAULT);
				di->ops->ctrl_port_mode(
					TYPEC_HOST_PORT_MODE_UFP);
				schedule_delayed_work(&di->g_wdog_work,
					msecs_to_jiffies(TRYSNK_TIMEOUT_MS));
			}
#else
			hwlog_info("UFP OTG detected\n");
			di->sink_attached = true;
			typec_open_otg();
#endif /* CONFIG_DUAL_ROLE_USB_INTF */
		}
	} else if (port_mode == TYPEC_DEV_PORT_MODE_UFP) {
	/* Type-C attached as UFP */
#ifdef CONFIG_DUAL_ROLE_USB_INTF
		hwlog_info("attached as UFP/Sink\n");
		/*
		 * exception handling:
		 * if CC pin is not stable, the state transition may from
		 * AS DFP to AS UFP direct, VBUS should be turned off first
		 */
		if (di->sink_attached) {
			hwlog_info("%s: remove Sink first\n", __func__);
			typec_close_otg();
			di->reverse_state = 0;
			di->trysnk_attempt = 0;
			di->sink_attached = 0;
		}

		if (di->reverse_state == REVERSE_ATTEMPT) {
			hwlog_info("reversed success, Source and VBUS detected\n");
			di->reverse_state = REVERSE_COMPLETE;
		}

		if (di->trysnk_attempt) {
			hwlog_info("TrySNK success, Source and VBUS detected\n");
			di->trysnk_attempt = 0;
			/* cancel watch dog work */
			cancel_delayed_work(&di->g_wdog_work);
		}

		/* notify the attached state */
		complete(&di->reverse_completion);
#else
		hwlog_info("DFP HOST detected\n");
#endif /* CONFIG_DUAL_ROLE_USB_INTF */
		input_current = typec_detect_current_mode();
		if (input_current == TYPEC_DEV_CURRENT_HIGH)
			hwlog_info("detected type c current is 3A\n");
		else if (input_current == TYPEC_DEV_CURRENT_MID)
			hwlog_info("detected type c current is 1.5A\n");
		else if (input_current == TYPEC_DEV_CURRENT_DEFAULT)
			hwlog_info("detected type c current is default\n");
		else
			hwlog_err("cannot detect a correct input current\n");

	} else {
		hwlog_err("cannot detect a correct port mode\n");
	}

	cc_orient = typec_detect_cc_orientation();
	if (cc_orient == TYPEC_ORIENT_CC1)
		hwlog_info("CC1 detected\n");
	else if (cc_orient == TYPEC_ORIENT_CC2)
		hwlog_info("CC2 detected\n");
	else
		hwlog_err("cannot detect a cc orientation\n");
}

static void typec_detach(struct typec_device_info *di)
{
	hwlog_info("typec detach\n");

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	/* set current advertisement to high when detach*/
	di->ops->ctrl_output_current(TYPEC_HOST_CURRENT_HIGH);
	/* turn off VBUS when unattached */
	if (di->sink_attached) {
		hwlog_info("Sink removed\n");
		typec_close_otg();
	}

	/*
	 * clear flags and make sure set the port mode back to DRP
	 * when unattached
	 */
	di->reverse_state = 0;
	di->trysnk_attempt = 0;
	di->sink_attached = 0;
	di->ops->ctrl_port_mode(TYPEC_HOST_PORT_MODE_DRP);
#else
	if (di->sink_attached) {
		hwlog_info("UFP OTG detach\n");
		di->sink_attached = false;
		typec_close_otg();
	}
#endif /* CONFIG_DUAL_ROLE_USB_INTF */
}

/*
 * handle the public interrupt work which is triggered from typec chips.
 */
static void typec_intb_work(struct work_struct *work)
{
	int attach_status;
	struct typec_device_info *di = NULL;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct typec_device_info, g_intb_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	mutex_lock(&di->typec_lock);

	attach_status = typec_detect_attachment_status();

	if (attach_status == TYPEC_ATTACH) {
		hwlog_info("typec attach\n");
		typec_attach(di);
	} else if (attach_status == TYPEC_DETACH) {
		hwlog_info("typec detach\n");
		typec_detach(di);
	} else if (attach_status == TYPEC_CUR_CHANGE_FOR_FSC) {
		hwlog_info("cc current change interrupt\n");
		typec_current_notifier_call(di);
	} else if (attach_status == TYPEC_ACC_MODE_CHANGE) {
		hwlog_info("accessory mode change interrupt\n");
	} else {
		hwlog_err("cannot detect a correct attachment status\n");
	}

	mutex_unlock(&di->typec_lock);

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	/* to notify userspace that the state might have changed */
	if (di->dual_role)
		dual_role_instance_changed(di->dual_role);
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	typec_clean_int_mask();

	typec_wake_unlock(di);
}

/*
 * create a class for both core layer and chip layers.
 */
struct class *hw_typec_get_class(void)
{
	if (!typec_class) {
		typec_class = class_create(THIS_MODULE, "hw_typec");
		if (IS_ERR(typec_class)) {
			hwlog_err("cannot create class\n");
			return NULL;
		}
	}

	return typec_class;
}
EXPORT_SYMBOL_GPL(hw_typec_get_class);

/*
 * register ops pointer for chip layer.
 * so the external modules can operate chips directly from core layer.
 * return value is a struct pointer to deliver interrupt message.
 */
struct typec_device_info *typec_chip_register(
	struct typec_device_info *device_info,
	struct typec_device_ops *ops, struct module *owner)
{
	int ret;
	struct typec_device_info *di = g_typec_dev;
#ifdef CONFIG_DUAL_ROLE_USB_INTF
	struct dual_role_phy_desc *desc = NULL;
	struct dual_role_phy_instance *dual_role = NULL;
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	if (!di ||
		!ops ||
		!ops->clean_int_mask ||
		!ops->detect_attachment_status ||
		!ops->detect_cc_orientation ||
		!ops->detect_input_current ||
		!ops->detect_port_mode ||
		!ops->ctrl_output_current ||
		!ops->ctrl_port_mode) {
		hwlog_err("di or ops is null\n");
		return NULL;
	}

	di->ops = ops;
	di->owner = owner;
	di->typec_trigger_otg = device_info->typec_trigger_otg;
	di->gpio_intb = device_info->gpio_intb;

	mutex_init(&di->typec_lock);
	INIT_WORK(&di->g_intb_work, typec_intb_work);

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	INIT_DELAYED_WORK(&di->g_wdog_work, typec_wdog_work);
	init_completion(&di->reverse_completion);

	desc = devm_kzalloc(device_info->dev, sizeof(*desc), GFP_KERNEL);
	if (!desc)
		return NULL;

	desc->name = "otg_default";
	desc->supported_modes = DUAL_ROLE_SUPPORTED_MODES_DFP_AND_UFP;
	desc->get_property = dual_role_get_local_prop;
	desc->set_property = dual_role_set_prop;
	desc->properties = fusb_drp_properties;
	desc->num_properties = ARRAY_SIZE(fusb_drp_properties);
	desc->property_is_writeable = dual_role_is_writeable;
	dual_role = devm_dual_role_instance_register(device_info->dev, desc);
	dual_role->drv_data = di;
	di->dual_role = dual_role;
	di->desc = desc;
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	typec_class = class_create(THIS_MODULE, "hw_typec");
	if (IS_ERR(typec_class)) {
		hwlog_err("cannot create class\n");
		return NULL;
	}

	if (typec_class) {
		typec_dev = device_create(typec_class, NULL, 0, NULL, "typec");
		if (IS_ERR(typec_dev)) {
			hwlog_err("sysfs device create failed\n");
			return NULL;
		}

		ret = sysfs_create_group(&typec_dev->kobj, &typec_attr_group);
		if (ret) {
			hwlog_err("sysfs group create failed\n");
			goto fail_free_sysfs;
		}
	}

	return di;

fail_free_sysfs:
	sysfs_remove_group(&typec_dev->kobj, &typec_attr_group);

	return NULL;
}
EXPORT_SYMBOL_GPL(typec_chip_register);

static int __init typec_init(void)
{
	struct typec_device_info *di = NULL;

	hwlog_info("probe begin\n");

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_typec_dev = di;

	BLOCKING_INIT_NOTIFIER_HEAD(&di->typec_current_nh);
	wake_lock_init(&di->wake_lock, WAKE_LOCK_SUSPEND, "typec_wake_lock");

	hwlog_info("probe end\n");
	return 0;
}

static void __exit typec_exit(void)
{
	hwlog_info("remove begin\n");

	if (!typec_class || !typec_dev || !g_typec_dev)
		return;

	class_destroy(typec_class);
	sysfs_remove_group(&typec_dev->kobj, &typec_attr_group);
	wake_lock_destroy(&g_typec_dev->wake_lock);

	hwlog_info("remove end\n");
}

subsys_initcall(typec_init);
module_exit(typec_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei typec module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
