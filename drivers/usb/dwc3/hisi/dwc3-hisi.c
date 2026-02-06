/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: dwc3-usb.c for DesignWare USB3 DRD Controller
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/pm_runtime.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/io.h>
#include <linux/of_gpio.h>
#include <linux/usb/ch9.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/usb/audio.h>
#include <linux/version.h>
#include <linux/extcon.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
#include <linux/extcon-provider.h>
#endif
#include <huawei_platform/usb/hw_pd_dev.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/usb/hisi_usb_helper.h>
#include <linux/hisi/usb/hisi_usb_reg_cfg.h>
#include <linux/hisi/usb/dwc3_usb_interface.h>
#include <linux/hisi/usb/chip_usb_log.h>
#include <linux/hisi/usb/chip_usb_debug_framework.h>
#include "dwc3-hisi.h"
#include "dwc3-hifi-usb.h"
#include "hisi_usb_bc12.h"
#include "hisi_usb_debug.h"
#include "hisi_usb_hw.h"
#include "combophy.h"
#ifdef CONFIG_HUAWEI_CHARGER_AP
#include <huawei_platform/power/huawei_charger.h>
#endif
#include <chipset_common/hwusb/hw_usb.h>

struct chip_dwc3_device *chip_dwc3_dev;
const struct chip_usb_phy *g_chip_usb_phy;

static const unsigned int usb_extcon_cable[] = {
	EXTCON_USB, EXTCON_USB_HOST, EXTCON_NONE
};

static int dwc3_phy_init(struct chip_dwc3_device *chip_dwc, bool host_mode);
static int dwc3_phy_shutdown(struct chip_dwc3_device *chip_dwc);
static int dwc3_usb20_phy_init(struct chip_dwc3_device *chip_dwc3, unsigned int combophy_flag);
static int dwc3_usb20_phy_shutdown(struct chip_dwc3_device *chip_dwc,
		unsigned int combophy_flag, unsigned int keep_power);

bool chip_usb_state_is_host(void)
{
	struct chip_dwc3_device *chip_dwc3 = chip_dwc3_dev;

	if (chip_dwc3)
		return (chip_dwc3->state == USB_STATE_HOST);
	else
		return false;
}

int chip_dwc3_is_fpga(void)
{
	if (!chip_dwc3_dev) {
		usb_err("usb driver not probed!\n");
		return 0;
	}

	return !!chip_dwc3_dev->fpga_flag;
}

void chip_usb_unreset_phy_if_fpga(void)
{
	unsigned int gpio;

	if (!chip_dwc3_dev || chip_dwc3_dev->fpga_phy_reset_gpio < 0)
		return;

	gpio = (unsigned int)chip_dwc3_dev->fpga_phy_reset_gpio;

	gpio_direction_output(gpio, 1);
	mdelay(10); /* mdelay time */

	gpio_direction_output(gpio, 0);
	mdelay(10); /* mdelay time */
}

void chip_usb_switch_sharedphy_if_fpga(int to_hifi)
{
	if (!chip_dwc3_dev || chip_dwc3_dev->fpga_phy_switch_gpio < 0)
		return;

	gpio_direction_output((unsigned int)chip_dwc3_dev->fpga_phy_switch_gpio,
			!!to_hifi);
	udelay(100); /* udelay time */
}

void dwc3_core_disable_pipe_clock(void)
{
	if (!chip_dwc3_dev || !chip_dwc3_dev->core_ops) {
		usb_err("[USB.CP0] usb driver not setup!\n");
		return;
	}
	usb_info("+\n");
	/* disable pipe clock use for DP4 and must use usb2 */
	chip_dwc3_dev->core_ops->disable_pipe_clock();
	usb_info("-\n");
}

int dwc3_core_enable_u3(void)
{
	if (!chip_dwc3_dev || !chip_dwc3_dev->core_ops) {
		usb_err("[USB.CP0] usb driver not setup!\n");
		return -ENODEV;
	}
	return chip_dwc3_dev->core_ops->enable_u3();
}

enum usb_device_speed chip_dwc3_get_dt_host_maxspeed(void)
{
	enum usb_device_speed speed = USB_SPEED_SUPER;
	struct device *dev = NULL;
	const char *maximum_speed = NULL;
	int err;

	if (!chip_dwc3_dev) {
		usb_err("dwc3_dev is null\n");
		return speed;
	}

	dev = &chip_dwc3_dev->pdev->dev;

	err = device_property_read_string(dev, "host-maximum-speed", &maximum_speed);
	if (err < 0)
		return speed;

	return usb_speed_to_string(maximum_speed, strlen(maximum_speed));
}

static const char *event_type_string(enum otg_dev_event_type event)
{
	static const char * const usb_event_strings[] = {
		[CHARGER_CONNECT_EVENT]		= "CHARGER_CONNECT",
		[CHARGER_DISCONNECT_EVENT]	= "CHARGER_DISCONNECT",
		[ID_FALL_EVENT]			= "OTG_CONNECT",
		[ID_RISE_EVENT]			= "OTG_DISCONNECT",
		[DP_IN]				= "DP_IN",
		[DP_OUT]			= "DP_OUT",
		[START_HIFI_USB]		= "START_HIFI_USB",
		[START_HIFI_USB_RESET_VBUS]	= "START_HIFI_USB_RESET_VBUS",
		[STOP_HIFI_USB]			= "STOP_HIFI_USB",
		[STOP_HIFI_USB_RESET_VBUS]	= "STOP_HIFI_USB_RESET_VBUS",
#ifdef CONFIG_USB_DWC3_NYET_ABNORMAL
		[START_AP_USE_HIFIUSB]		= "START_AP_USE_HIFIUSB",
		[STOP_AP_USE_HIFIUSB]		= "STOP_AP_USE_HIFIUSB",
#endif
		[HIFI_USB_HIBERNATE]		= "HIFI_USB_HIBERNATE",
		[HIFI_USB_WAKEUP]		= "HIFI_USB_WAKEUP",
		[DISABLE_USB3_PORT]		= "DISABLE_USB3_PORT",
		[SWITCH_CONTROLLER]		= "SWITCH_CONTROLLER",
		[NONE_EVENT]			= "NONE",
	};

	if (event > NONE_EVENT)
		return "illegal event";

	return usb_event_strings[event];
}

enum hisi_charger_type chip_get_charger_type(void)
{
	if (!chip_dwc3_dev) {
		usb_err("dwc3 not yet probed!\n");
		return CHARGER_TYPE_NONE;
	}

	usb_info("type: %s\n", charger_type_string(chip_dwc3_dev->charger_type));
	return chip_dwc3_dev->charger_type;
}
EXPORT_SYMBOL_GPL(chip_get_charger_type);

static void set_vbus_power(struct chip_dwc3_device *chip_dwc3, unsigned int is_on)
{
	enum hisi_charger_type new;

	if (is_on == 0)
		new = CHARGER_TYPE_NONE;
	else
		new = PLEASE_PROVIDE_POWER;

	if (chip_dwc3->charger_type != new) {
		usb_dbg("set port power %d\n", is_on);
		chip_dwc3->charger_type = new;
		notify_charger_type(chip_dwc3);
	}

	if (chip_dwc3->fpga_otg_drv_vbus_gpio > 0) {
		gpio_direction_output(chip_dwc3->fpga_otg_drv_vbus_gpio, !!is_on);
		usb_dbg("turn %s drvvbus for fpga\n", is_on ? "on" : "off");
	}
}

void chip_dwc3_wake_lock(struct chip_dwc3_device *chip_dwc3)
{
	if (!chip_dwc3->wake_lock.active) {
		usb_dbg("usb otg wake lock\n");
		__pm_stay_awake(&chip_dwc3->wake_lock);
	}
}

void chip_dwc3_wake_unlock(struct chip_dwc3_device *chip_dwc3)
{
	if (chip_dwc3->wake_lock.active) {
		usb_dbg("usb otg wake unlock\n");
		__pm_relax(&chip_dwc3->wake_lock);
	}
}

/*
 * put the new event en queue
 * if the event_queue is full, return -ENOSPC
 */
static int event_enqueue(struct chip_dwc3_device *chip_dwc3,
		  const struct chip_usb_event *event)
{
	/*
	 * check if there is noly 1 empty space to save event
	 * drop event if it is not OTG_DISCONNECT or CHARGER_DISCONNECT
	 */
	if (kfifo_avail(&chip_dwc3->event_fifo) == 1 &&
	    event->type != ID_RISE_EVENT &&
	    event->type != CHARGER_DISCONNECT_EVENT) {
		usb_err("drop event %s except disconnect evnet\n",
			event_type_string(event->type));
		return -ENOSPC;
	}

	if (kfifo_in(&chip_dwc3->event_fifo, event, 1) == 0) {
		usb_err("drop event %s\n", event_type_string(event->type));
		return -ENOSPC;
	}

	return 0;
}

/*
 * get event frome event_queue
 * return the numbers of event dequeued, currently it is 1
 */
int event_dequeue(struct chip_dwc3_device *chip_dwc3,
		  struct chip_usb_event *event)
{
	return kfifo_out_spinlocked(&chip_dwc3->event_fifo, event,
				    1, &chip_dwc3->event_lock);
}

static int dwc3_remove_child(struct device *dev, void *data)
{
	int ret;

	(void)(data);
	ret = of_platform_device_destroy(dev, NULL);
	if (ret)
		usb_err("device destroy error (ret %d)\n", ret);

	return 0;
}

static void dwc3_check_voltage(struct chip_dwc3_device *chip_dwc)
{
	usb_dbg("+\n");

	if (chip_dwc->check_voltage) {
		/* first dplus pulldown 15k */
		chip_usb_dpdm_pulldown(chip_dwc);
#ifdef CONFIG_HUAWEI_CHARGER_AP
		/* second call charger's API to check voltage */
		power_event_bnc_notify(POWER_BNT_WD, POWER_NE_WD_DETECT_BY_USB_DP_DN, NULL);
#endif
		/* third dplus pullup */
		chip_usb_dpdm_pullup(chip_dwc);
	}
	usb_dbg("-\n");
}

static int start_device(struct chip_dwc3_device *chip_dwc)
{
	int ret;

	/* due to detect charger type, must resume chip_dwc */
	ret = dwc3_phy_init(chip_dwc, false);
	if (ret) {
		usb_err("dwc3_phy_init failed (ret %d)\n", ret);
		return ret;
	}

	/* if the platform support,it need check voltage */
	dwc3_check_voltage(chip_dwc);

	/* detect charger type */
	chip_dwc->charger_type = chip_usb_detect_charger_type(chip_dwc);
	notify_charger_type(chip_dwc);

	/* In some cases, DCP is detected as SDP wrongly. To avoid this,
	 * start bc_again delay work to detect charger type once more.
	 * If later the enum process is executed, then it's a real SDP, so
	 * the work will be canceled.
	 */
	if (bc_again_allowed(chip_dwc))
		schedule_bc_again(chip_dwc);

	/* do not start peripheral if real charger connected */
	if (enumerate_allowed(chip_dwc)) {
		if (chip_dwc->fpga_usb_mode_gpio > 0) {
			gpio_direction_output((unsigned int)chip_dwc->fpga_usb_mode_gpio, 0);
			usb_dbg("switch to device mode\n");
		}

		/* start peripheral */
		ret = extcon_set_state_sync(chip_dwc->edev,
				EXTCON_USB, true);
		if (ret) {
			usb_err("start peripheral error\n");
			return ret;
		}
	} else {
		usb_dbg("it need notify USB_CONNECT_DCP while a real charger connected\n");
		chip_dwc->speed = USB_CONNECT_DCP;
		if (!queue_work(system_power_efficient_wq,
						&chip_dwc->speed_change_work))
			usb_err("schedule speed_change_work wait:%d\n", chip_dwc->speed);
	}

	chip_dwc->state = USB_STATE_DEVICE;

	if (sleep_allowed(chip_dwc))
		chip_dwc3_wake_unlock(chip_dwc);
	else
		chip_dwc3_wake_lock(chip_dwc);

	usb_dbg("usb status: OFF -> DEVICE\n");

	return 0;
}

static void stop_device(struct chip_dwc3_device *chip_dwc)
{
	int ret;

	chip_usb_disable_vdp_src(chip_dwc);

	/* peripheral not started, if real charger connected */
	if (enumerate_allowed(chip_dwc)) {
		/* stop peripheral */
		if (extcon_set_state_sync(chip_dwc->edev, EXTCON_USB, false)) {
			usb_err("stop peripheral error\n");
			return;
		}
	}

	cancel_bc_again(chip_dwc, 0);

	/* usb cable disconnect, notify no charger */
	chip_dwc->charger_type = CHARGER_TYPE_NONE;
	notify_charger_type(chip_dwc);

	chip_dwc->state = USB_STATE_OFF;
	chip_dwc3_wake_unlock(chip_dwc);

	ret = dwc3_phy_shutdown(chip_dwc);
	if (ret)
		usb_err("dwc3_phy_shutdown failed (ret %d)\n", ret);

	usb_dbg("usb status: DEVICE -> OFF\n");
}

static int start_host(struct chip_dwc3_device *chip_dwc)
{
	int ret;

	ret = dwc3_phy_init(chip_dwc, true);
	if (ret) {
		usb_err("dwc3_phy_init failed (ret %d)\n", ret);
		return ret;
	}

	if (chip_dwc->fpga_usb_mode_gpio > 0) {
		gpio_direction_output((unsigned int)chip_dwc->fpga_usb_mode_gpio, 1);
		usb_dbg("switch to host mode\n");
	}

	/* start host */
	ret = extcon_set_state_sync(chip_dwc->edev,
			EXTCON_USB_HOST, true);
	if (ret) {
		usb_err("start host error\n");
		return ret;
	}

	return 0;
}

static int stop_host(struct chip_dwc3_device *chip_dwc)
{
	int ret;

	/* stop host */
	ret = extcon_set_state_sync(chip_dwc->edev,
			EXTCON_USB_HOST, false);
	if (ret) {
		usb_err("stop host error\n");
		return ret;
	}

	ret = dwc3_phy_shutdown(chip_dwc);
	if (ret)
		usb_err("dwc3_phy_shutdown failed (ret %d)\n", ret);

	return ret;
}

static int start_audio_usb(struct chip_dwc3_device *chip_dwc, unsigned int combophy_flag)
{
	if (dwc3_usb20_phy_init(chip_dwc, combophy_flag)) {
		usb_err("audio usb phy init failed\n");
		return -EBUSY;
	}

	if (start_hifi_usb()) {
		if (dwc3_usb20_phy_shutdown(chip_dwc, combophy_flag, 0))
			WARN_ON(1);
		return -EBUSY;
	}

	return 0;
}

static void stop_audio_usb(struct chip_dwc3_device *chip_dwc, unsigned int combophy_flag)
{
	stop_hifi_usb();
	if (dwc3_usb20_phy_shutdown(chip_dwc, combophy_flag, 0))
		WARN_ON(1);
}

static void handle_start_hifi_usb_event(struct chip_dwc3_device *chip_dwc,
					int reset_vbus)
{
	switch (chip_dwc->state) {
	case USB_STATE_OFF:
		set_vbus_power(chip_dwc, 1);
		if (start_audio_usb(chip_dwc, 1)) {
			usb_err("start_audio_usb failed\n");
			if (start_host(chip_dwc)) {
				usb_err("start_host failed\n");
			} else {
				chip_dwc->state = USB_STATE_HOST;
				chip_dwc3_wake_lock(chip_dwc);
				usb_dbg("usb_status: OFF -> HOST\n");
			}
			return;
		}

		chip_dwc->state = USB_STATE_HIFI_USB;
		chip_dwc3_wake_unlock(chip_dwc);
		pd_dpm_wakelock_ctrl(PD_WAKE_UNLOCK);
		usb_dbg("usb state: OFF -> HIFI_USB\n");
		usb_dbg("usb start hifi usb time: %d ms\n",
			jiffies_to_msecs(jiffies - chip_dwc->start_host_time_stamp));
		break;

	case USB_STATE_HOST:
		if (reset_vbus) {
			set_vbus_power(chip_dwc, 0);
			pd_dpm_vbus_ctrl(CHARGER_TYPE_NONE);
		}

		if (stop_host(chip_dwc)) {
			usb_err("stop_host failed\n");
			return;
		}

		if (reset_vbus) {
			set_vbus_power(chip_dwc, 1);
			pd_dpm_vbus_ctrl(PLEASE_PROVIDE_POWER);
		}

		if (start_audio_usb(chip_dwc, 1)) {
			usb_err("start_audio_usb failed\n");
			if (start_host(chip_dwc))
				usb_err("start_host failed\n");
			return;
		}

		chip_dwc->state = USB_STATE_HIFI_USB;
		chip_dwc3_wake_unlock(chip_dwc);
		pd_dpm_wakelock_ctrl(PD_WAKE_UNLOCK);
		usb_dbg("usb state: HOST -> HIFI_USB\n");

		usb_dbg("usb start hifi usb time: %d ms\n",
			jiffies_to_msecs(jiffies - chip_dwc->start_hifiusb_time_stamp));
		break;
	default:
		usb_dbg("event %d in state %d\n", START_HIFI_USB, chip_dwc->state);
		break;
	}
	chip_dwc->start_hifiusb_complete_time_stamp = jiffies;
}

static void handle_stop_hifi_usb_event(struct chip_dwc3_device *chip_dwc,
					int reset_vbus)
{
	int ret;

	switch (chip_dwc->state) {
	case USB_STATE_HIFI_USB:
		chip_dwc3_wake_lock(chip_dwc);
		pd_dpm_wakelock_ctrl(PD_WAKE_LOCK);

		stop_audio_usb(chip_dwc, 1);

		if (reset_vbus) {
			set_vbus_power(chip_dwc, 0);
			pd_dpm_vbus_ctrl(CHARGER_TYPE_NONE);
		}

		if (!chip_dwc->hifi_ip_first)
			msleep(1500);

		if (start_host(chip_dwc)) {
			usb_err("start_host failed\n");
			return;
		}

		if (reset_vbus) {
			set_vbus_power(chip_dwc, 1);
			pd_dpm_vbus_ctrl(PLEASE_PROVIDE_POWER);
		}

		chip_dwc->state = USB_STATE_HOST;
		usb_dbg("usb state: HIFI_USB -> HOST\n");

		break;
	case USB_STATE_HIFI_USB_HIBERNATE:
		chip_dwc3_wake_lock(chip_dwc);
		pd_dpm_wakelock_ctrl(PD_WAKE_LOCK);

		/* phy was closed in this state */
		stop_hifi_usb();

		msleep(1500);

		ret = dwc3_usb20_phy_init(chip_dwc, 0);
		if (ret)
			usb_err("shared_phy_init error ret %d\n", ret);
		ret = dwc3_usb20_phy_shutdown(chip_dwc, 1, 0);
		if (ret)
			usb_err("shared_phy_shutdown error ret %d\n", ret);

		if (start_host(chip_dwc)) {
			usb_err("start_host failed\n");
			return;
		}

		chip_dwc->state = USB_STATE_HOST;
		usb_dbg("usb state: HIFI_USB_HIBERNATE -> HOST\n");

		break;
	default:
		usb_dbg("event %d in state %d\n", STOP_HIFI_USB, chip_dwc->state);
		break;
	}
}

static void handle_start_hifi_usb_hibernate(struct chip_dwc3_device *chip_dwc)
{
	switch (chip_dwc->state) {
	case USB_STATE_HIFI_USB:
		if (hifi_usb_hibernate()) {
			WARN_ON(1);
			return;
		}

		if (dwc3_usb20_phy_shutdown(chip_dwc, 0, 1)) {
			WARN_ON(1);
			return;
		}

		chip_dwc->state = USB_STATE_HIFI_USB_HIBERNATE;
		usb_dbg("usb state: HIFI_USB -> HIFI_USB_HIBERNATE\n");
		msleep(50); /* debounce of suspend state */
		break;
	default:
		usb_dbg("event %d in state %d\n", HIFI_USB_HIBERNATE, chip_dwc->state);
		break;
	}
}

static void handle_start_hifi_usb_wakeup(struct chip_dwc3_device *chip_dwc)
{
	switch (chip_dwc->state) {
	case USB_STATE_HIFI_USB_HIBERNATE:
		if (dwc3_usb20_phy_init(chip_dwc, 0)) {
			WARN_ON(1);
			return;
		}

		if (hifi_usb_revive()) {
			WARN_ON(1);
			return;
		}

		chip_dwc->state = USB_STATE_HIFI_USB;
		usb_dbg("usb state: HIFI_USB_HIBERNATE -> HIFI_USB\n");
		break;
	default:
		usb_dbg("event %d in state %d\n", HIFI_USB_WAKEUP, chip_dwc->state);
		break;
	}
}

/*
 * Caution: this function must be called in "dwc3->lock"
 * Currently, this function called only by usb_resume
 */
int chip_usb_wakeup_hifi_usb(void)
{
	struct chip_dwc3_device *dev = chip_dwc3_dev;

	if (!dev)
		return -ENOENT;

	handle_start_hifi_usb_wakeup(dev);
	return 0;
}

static void handle_charger_connect_event(struct chip_dwc3_device *chip_dwc)
{
	if (chip_dwc->state == USB_STATE_DEVICE) {
		usb_dbg("Already in device mode, do nothing\n");
	} else if (chip_dwc->state == USB_STATE_OFF) {
		if (start_device(chip_dwc))
			usb_err("start_device error\n");
	} else if (chip_dwc->state == USB_STATE_HOST) {
		usb_dbg("Charger connect intrrupt in HOST mode\n");
	} else if (chip_dwc->state == USB_STATE_HIFI_USB) {
		usb_dbg("vbus power in hifi usb state\n");
	} else {
		usb_dbg("can not handle charger connect event in mode %s\n",
				chip_usb_state_string(chip_dwc->state));
	}
}

static void handle_charger_disconnect_event(struct chip_dwc3_device *chip_dwc)
{
	if (chip_dwc->state == USB_STATE_OFF) {
		usb_dbg("Already in off mode, do nothing\n");
	} else if (chip_dwc->state == USB_STATE_DEVICE) {
		stop_device(chip_dwc);
	} else if (chip_dwc->state == USB_STATE_HOST) {
		usb_dbg("Charger disconnect intrrupt in HOST mode\n");
	} else if (chip_dwc->state == USB_STATE_HIFI_USB) {
		usb_dbg("vbus disconnect event in hifi usb state\n");
	} else {
		usb_dbg("can not handle charger disconnect event in mode %s\n",
				chip_usb_state_string(chip_dwc->state));
	}
}

static void handle_start_arm_usb_event(struct chip_dwc3_device *chip_dwc)
{
	switch (chip_dwc->state) {
	case USB_STATE_OFF:
		set_vbus_power(chip_dwc, 1);
		if (start_host(chip_dwc))
			set_vbus_power(chip_dwc, 0);

		chip_dwc->state = USB_STATE_HOST;
		chip_dwc3_wake_lock(chip_dwc);
		usb_dbg("usb_status: OFF -> HOST\n");
		break;
	default:
		usb_dbg("event %d in state %d\n", ID_FALL_EVENT, chip_dwc->state);
		break;
	}
}

static void handle_id_fall_event(struct chip_dwc3_device *chip_dwc)
{
	/*
	 * 1. hifi ip first feature controlled by device tree.
	 * 2. hifi usb need to wait for usbauddio nv.
	 */
	if (chip_usb_otg_use_hifi_ip_first()
		&& (chip_dwc->mode_type == TCPC_USB31_CONNECTED)) {
		usb_dbg("use hifi ip first\n");
		handle_start_hifi_usb_event(chip_dwc, 0);
	} else {
		usb_dbg("use arm ip first\n");
		handle_start_arm_usb_event(chip_dwc);
	}
}

static void handle_id_rise_event(struct chip_dwc3_device *chip_dwc)
{
	switch (chip_dwc->state) {
	case USB_STATE_HOST:
		set_vbus_power(chip_dwc, 0);
		if (stop_host(chip_dwc))
			usb_err("stop_host failed\n");

		chip_dwc->state = USB_STATE_OFF;
		chip_dwc3_wake_unlock(chip_dwc);
		usb_dbg("hiusb_status: HOST -> OFF\n");

		reset_hifi_usb();
		break;
	case USB_STATE_HIFI_USB:
		set_vbus_power(chip_dwc, 0);
		stop_audio_usb(chip_dwc, 0);

		chip_dwc->state = USB_STATE_OFF;
		chip_dwc3_wake_unlock(chip_dwc);
		usb_dbg("usb state: HIFI_USB -> OFF\n");

		reset_hifi_usb();

		usb_dbg("usb stop hifi usb time: %d ms\n",
			jiffies_to_msecs(jiffies - chip_dwc->stop_host_time_stamp));
		break;

	case USB_STATE_HIFI_USB_HIBERNATE:
		set_vbus_power(chip_dwc, 0);
		/* phy was closed in this state */
		stop_hifi_usb();

		chip_dwc->state = USB_STATE_OFF;
		chip_dwc3_wake_unlock(chip_dwc);
		usb_dbg("usb state: HIFI_USB_HIBERNATE -> OFF\n");

		reset_hifi_usb();

		usb_dbg("usb stop hifi usb time: %d ms\n",
			jiffies_to_msecs(jiffies - chip_dwc->stop_host_time_stamp));
		break;

#ifdef CONFIG_USB_DWC3_NYET_ABNORMAL
	case USB_STATE_AP_USE_HIFIUSB:
		set_vbus_power(chip_dwc, 0);
		ap_stop_use_hifiusb();

		if (dwc3_usb20_phy_shutdown(chip_dwc, 0, 0))
			WARN_ON(1);

		chip_dwc->state = USB_STATE_OFF;
		chip_dwc3_wake_unlock(chip_dwc);
		usb_dbg("usb state: AP_USE_HIFI_USB -> OFF\n");

		break;
#endif
	default:
		usb_dbg("event %d in state %d\n", ID_RISE_EVENT, chip_dwc->state);
		break;
	}
}

#ifdef CONFIG_USB_DWC3_NYET_ABNORMAL
static int dwc3_tcpc_is_usb_only(struct chip_dwc3_device *chip_dwc)
{
	if (chip_dwc->usb_phy && chip_dwc->usb_phy->tcpc_is_usb_only)
		return chip_dwc->usb_phy->tcpc_is_usb_only();

	return 0;
}

static void handle_start_ap_use_hifiusb(struct chip_dwc3_device *chip_dwc)
{
	int is_usb_only;

	switch (chip_dwc->state) {
	case USB_STATE_HOST:
		if (stop_host(chip_dwc)) {
			usb_err("stop_host failed\n");
			return;
		}

		is_usb_only = dwc3_tcpc_is_usb_only(chip_dwc);
		if (is_usb_only) {
			usb_dbg("combophy is in usb only mode, do vbus reset\n");
			set_vbus_power(chip_dwc, 0);
			pd_dpm_vbus_ctrl(CHARGER_TYPE_NONE);
			msleep(30);
		}

		if (dwc3_usb20_phy_init(chip_dwc, 0)) {
			usb_err("audio usb phy init failed\n");
		} else if (ap_start_use_hifiusb()) {
			usb_err("start ap use hifiusb failed");
			if (!dwc3_usb20_phy_shutdown(chip_dwc, 0, 0)) {
				if (start_host(chip_dwc))
					usb_err("start_host failed\n");
			} else {
				usb_err("shard phy shutdown failed\n");
			}
		} else {
			chip_dwc->state = USB_STATE_AP_USE_HIFIUSB;
			usb_dbg("usb state: HOST -> AP_USE_HIFI_USB\n");
		}

		if (is_usb_only) {
			msleep(30);
			set_vbus_power(chip_dwc, 1);
			pd_dpm_vbus_ctrl(PLEASE_PROVIDE_POWER);
		}

		break;
	default:
		usb_dbg("event %d in state %d\n", START_AP_USE_HIFIUSB,
				chip_dwc->state);
		break;
	}
}

static void handle_stop_ap_use_hifiusb(struct chip_dwc3_device *chip_dwc)
{
	switch (chip_dwc->state) {
	case USB_STATE_AP_USE_HIFIUSB:
		ap_stop_use_hifiusb();

		if (dwc3_usb20_phy_shutdown(chip_dwc, 0, 0))
			WARN_ON(1);

		if (start_host(chip_dwc)) {
			usb_err("start_host failed\n");
			return;
		}

		chip_dwc->state = USB_STATE_HOST;
		usb_dbg("usb state: AP_USE_HIFI_USB -> HOST\n");

		break;
	default:
		usb_dbg("event %d in state %d\n", STOP_AP_USE_HIFIUSB,
				chip_dwc->state);
		break;
	}
}
#endif

static void usb_disable_usb3(struct chip_dwc3_device *chip_dwc)
{
	if (chip_dwc->usb_phy && chip_dwc->usb_phy->disable_usb3)
		chip_dwc->usb_phy->disable_usb3();
}

static void handle_disable_usb3(struct chip_dwc3_device *chip_dwc)
{
	if (chip_dwc->state != USB_STATE_HOST) {
		usb_dbg("event %d in state %d\n", DISABLE_USB3_PORT,
				chip_dwc->state);
		return;
	}

	if (!chip_dwc->usb_phy || !chip_dwc->usb_phy->disable_usb3)
		return;

	/* stop host */
	if (extcon_set_state_sync(chip_dwc->edev, EXTCON_USB_HOST, false)) {
		usb_err("stop host error\n");
		return;
	}

	usb_disable_usb3(chip_dwc);

	/* start host */
	if (extcon_set_state_sync(chip_dwc->edev, EXTCON_USB_HOST, true)) {
		usb_err("start host error\n");
		return;
	}
}

static void handle_switch_controller(struct chip_dwc3_device *chip_dwc)
{
	if (chip_dwc->state != USB_STATE_DEVICE &&
		chip_dwc->state != USB_STATE_HOST) {
		usb_err("event %d in state %d\n", SWITCH_CONTROLLER,
				chip_dwc->state);
		return;
	}

	if (chip_dwc->state == USB_STATE_HOST) {
		if (extcon_set_state_sync(chip_dwc->edev, EXTCON_USB_HOST, false)) {
			usb_err("stop host error\n");
			return;
		}
	}

	if (chip_dwc->state == USB_STATE_DEVICE) {
		if (extcon_set_state_sync(chip_dwc->edev, EXTCON_USB, false)) {
			usb_err("stop device error\n");
			return;
		}
	}

	if (chip_usb_controller_destroy(chip_dwc)) {
		usb_err("controller destroy error\n");
		return;
	}

	if (chip_usb_controller_exit(chip_dwc)) {
		usb_err("controller exit error\n");
		return;
	}

	if (chip_usb_controller_init(chip_dwc)) {
		usb_err("controller exit error\n");
		return;
	}
	if (chip_usb_controller_probe(chip_dwc)) {
		usb_err("controller probe error\n");
		return;
	}
}

static void handle_event(struct chip_dwc3_device *chip_dwc,
			 enum otg_dev_event_type event_type)
{
	int reset_vbus = 0;

	usb_err("type: %s\n", event_type_string(event_type));

	if (event_type == START_HIFI_USB_RESET_VBUS) {
		event_type = START_HIFI_USB;
		reset_vbus = 1;
	}

	if (event_type == STOP_HIFI_USB_RESET_VBUS) {
		event_type = STOP_HIFI_USB;
		reset_vbus = 1;
	}

	switch (event_type) {
	case CHARGER_CONNECT_EVENT:
		handle_charger_connect_event(chip_dwc);
		chip_dwc->start_device_complete_time_stamp = jiffies;
		break;

	case CHARGER_DISCONNECT_EVENT:
		handle_charger_disconnect_event(chip_dwc);
		break;

	case ID_FALL_EVENT:
		handle_id_fall_event(chip_dwc);
		chip_dwc->start_host_complete_time_stamp = jiffies;
		break;

	case ID_RISE_EVENT:
		handle_id_rise_event(chip_dwc);
		chip_dwc->stop_host_complete_time_stamp = jiffies;
		break;

	case START_HIFI_USB:
		handle_start_hifi_usb_event(chip_dwc, reset_vbus);
		break;

	case STOP_HIFI_USB:
		handle_stop_hifi_usb_event(chip_dwc, reset_vbus);
		chip_dwc->stop_hifiusb_complete_time_stamp = jiffies;
		break;

#ifdef CONFIG_USB_DWC3_NYET_ABNORMAL
	case START_AP_USE_HIFIUSB:
		handle_start_ap_use_hifiusb(chip_dwc);
		break;

	case STOP_AP_USE_HIFIUSB:
		handle_stop_ap_use_hifiusb(chip_dwc);
		break;
#endif

	case HIFI_USB_HIBERNATE:
		handle_start_hifi_usb_hibernate(chip_dwc);
		break;

	case HIFI_USB_WAKEUP:
		chip_dwc3_wake_lock(chip_dwc);
		handle_start_hifi_usb_wakeup(chip_dwc);
		chip_dwc3_wake_unlock(chip_dwc);
		break;

	case DISABLE_USB3_PORT:
		handle_disable_usb3(chip_dwc);
		break;

	case SWITCH_CONTROLLER:
		handle_switch_controller(chip_dwc);
		break;
	default:
		usb_dbg("illegal event type!\n");
		break;
	}
}

static void event_work(struct work_struct *work)
{
	struct chip_usb_event event = {0};

	struct chip_dwc3_device *chip_dwc = container_of(work,
				    struct chip_dwc3_device, event_work);

	usb_err("+\n");
	mutex_lock(&chip_dwc->lock);

	while (event_dequeue(chip_dwc, &event)) {
		if (event.flags & PD_EVENT) {
			chip_dwc->mode_type = (enum tcpc_mux_ctrl_type)event.param1;
			chip_dwc->plug_orien = (enum typec_plug_orien_e)event.param2;
		}
		if (event.callback) {
			if (event.flags & EVENT_CB_AT_PREPARE)
				event.callback(&event);

			if (event.flags & EVENT_CB_AT_HANDLE)
				event.callback(&event);
			else
				handle_event(chip_dwc, event.type);

			if (event.flags & EVENT_CB_AT_COMPLETE)
				event.callback(&event);
		} else {
			handle_event(chip_dwc, event.type);
		}
	}

	mutex_unlock(&chip_dwc->lock);

	usb_err("-\n");
}

static void dwc3_speed_change_work(struct work_struct *work)
{
	struct chip_dwc3_device *chip_dwc = container_of(work,
				    struct chip_dwc3_device, speed_change_work);
	usb_dbg("+\n");
	if (chip_dwc->fpga_flag) {
		usb_dbg("- fpga platform, don't notify speed\n");
		return;
	}

	if (chip_dwc->use_new_frame && chip_dwc->notify_speed) {
		usb_dbg("+device speed is %d\n", chip_dwc->speed);

#ifdef CONFIG_TCPC_CLASS
		if ((chip_dwc->speed != USB_CONNECT_HOST) &&
				(chip_dwc->speed != USB_CONNECT_DCP))
			hw_usb_set_usb_speed(chip_dwc->speed);
#endif

		usb_dbg("-\n");
	} else if (chip_dwc->usb_phy && chip_dwc->usb_phy->notify_speed) {
		chip_dwc->usb_phy->notify_speed(chip_dwc->speed);
	}
	usb_dbg("-\n");
}

static enum otg_dev_event_type hifi_usb_event_filter(
					enum otg_dev_event_type event)
{
	if (event == START_HIFI_USB_RESET_VBUS)
		return START_HIFI_USB;
	else if (event == STOP_HIFI_USB_RESET_VBUS)
		return STOP_HIFI_USB;
	else
		return event;
}

static int id_rise_event_check(enum otg_dev_event_type last_event)
{
	if ((last_event == ID_FALL_EVENT) ||
	    (last_event == START_HIFI_USB) ||
	    (last_event == STOP_HIFI_USB) ||
#ifdef CONFIG_USB_DWC3_NYET_ABNORMAL
	    (last_event == START_AP_USE_HIFIUSB) ||
	    (last_event == STOP_AP_USE_HIFIUSB) ||
#endif
	    (last_event == HIFI_USB_HIBERNATE) ||
	    (last_event == HIFI_USB_WAKEUP) ||
	    (last_event == DISABLE_USB3_PORT) ||
	    (last_event == DP_OUT) ||
	    (last_event == SWITCH_CONTROLLER))
		return 1;
	else
		return 0;
}

static int id_fall_event_check(enum otg_dev_event_type last_event)
{
	if ((last_event == CHARGER_DISCONNECT_EVENT) || (last_event == ID_RISE_EVENT))
		return 1;
	else
		return 0;
}

static int start_hifi_usb_event_check(enum otg_dev_event_type last_event)
{
	if ((last_event == ID_FALL_EVENT) || (last_event == STOP_HIFI_USB) ||
			(last_event == START_HIFI_USB)) /* start hifiusb maybe failed, allow retry */
		return 1;
	else
		return 0;
}

static int stop_hifi_usb_event_check(enum otg_dev_event_type last_event)
{
	if ((last_event == START_HIFI_USB) || (last_event == HIFI_USB_WAKEUP) ||
			(last_event == HIFI_USB_HIBERNATE) || (last_event == ID_FALL_EVENT))
		return 1;
	else
		return 0;
}

static int hifi_usb_hibernate_event_check(enum otg_dev_event_type last_event)
{
	if ((last_event == START_HIFI_USB) || (last_event == HIFI_USB_WAKEUP) ||
			(last_event == ID_FALL_EVENT))
		return 1;
	return 0;
}

static int hifi_usb_wakeup_event_check(enum otg_dev_event_type last_event)
{
	if ((last_event == HIFI_USB_HIBERNATE) || (last_event == HIFI_USB_WAKEUP))
		return 1;
	return 0;
}

static int charger_connect_event_check(enum otg_dev_event_type last_event)
{
	if ((last_event == CHARGER_DISCONNECT_EVENT) || (last_event == ID_RISE_EVENT))
		return 1;
	return 0;
}

static int charger_disconnect_event_check(enum otg_dev_event_type last_event)
{
	if (last_event == CHARGER_CONNECT_EVENT || last_event == SWITCH_CONTROLLER)
		return 1;
	return 0;
}

static int disable_usb3_event_check(enum otg_dev_event_type last_event)
{
	if (last_event == ID_FALL_EVENT || last_event == STOP_HIFI_USB)
		return 1;
	return 0;
}

static int dp_event_check(enum otg_dev_event_type last_event)
{
	if (last_event == ID_FALL_EVENT || last_event == STOP_HIFI_USB ||
			last_event == DP_OUT || last_event == DP_IN)
		return 1;

	return 0;
}

static int start_ap_usb_hifiusb_event_check(enum otg_dev_event_type last_event)
{
	if ((last_event == ID_FALL_EVENT)
				|| (last_event == STOP_HIFI_USB)
				|| (last_event == STOP_AP_USE_HIFIUSB))
			return 1;

	return 0;
}

static int stop_ap_usb_hifiusb_event_check(enum otg_dev_event_type last_event)
{
	if (last_event == ID_FALL_EVENT ||
				last_event == STOP_HIFI_USB)
		return 1;

	return 0;
}

static int event_check(enum otg_dev_event_type last_event,
		       enum otg_dev_event_type new_event)
{
	static int (* const event_check_handle[])(enum otg_dev_event_type) = {
		[CHARGER_CONNECT_EVENT] = charger_connect_event_check,
		[CHARGER_DISCONNECT_EVENT] = charger_disconnect_event_check,
		[ID_FALL_EVENT] = id_fall_event_check,
		[ID_RISE_EVENT] = id_rise_event_check,
		[DP_OUT] = dp_event_check,
		[DP_IN] = dp_event_check,
		[START_HIFI_USB] = start_hifi_usb_event_check,
		[START_HIFI_USB_RESET_VBUS] = NULL,
		[STOP_HIFI_USB] = stop_hifi_usb_event_check,
		[STOP_HIFI_USB_RESET_VBUS] = NULL,
		[START_AP_USE_HIFIUSB] = start_ap_usb_hifiusb_event_check,
		[STOP_AP_USE_HIFIUSB] = stop_ap_usb_hifiusb_event_check,
		[HIFI_USB_HIBERNATE] = hifi_usb_hibernate_event_check,
		[HIFI_USB_WAKEUP] = hifi_usb_wakeup_event_check,
		[DISABLE_USB3_PORT] = disable_usb3_event_check,
	};
	int (*event_check)(enum otg_dev_event_type) = NULL;

	if (last_event == NONE_EVENT)
		return 1;

	last_event = hifi_usb_event_filter(last_event);
	new_event = hifi_usb_event_filter(new_event);
	/*
	 * Check "new_event" must Greater than or equal to
	 * event_check_handle size. Because enum is as unsigned, and
	 * CHARGER_CONNECT_EVENT=0, so verify the end is enough.
	 */
	if (new_event >= ARRAY_SIZE(event_check_handle))
		return 1;

	event_check = event_check_handle[new_event];
	if (event_check)
		return event_check(last_event);

	return 1;
}

static void save_event_time_stamp(struct chip_dwc3_device *chip_dwc3,
			enum otg_dev_event_type event)
{
	if (event == CHARGER_CONNECT_EVENT)
		chip_dwc3->start_device_time_stamp = jiffies;
	else if (event == ID_FALL_EVENT)
		chip_dwc3->start_host_time_stamp = jiffies;
	else if (event == ID_RISE_EVENT)
		chip_dwc3->stop_host_time_stamp = jiffies;
	else if ((event == START_HIFI_USB)
			|| (event == START_HIFI_USB_RESET_VBUS))
		chip_dwc3->start_hifiusb_time_stamp = jiffies;
	else if ((event == STOP_HIFI_USB)
			|| (event == STOP_HIFI_USB_RESET_VBUS))
		chip_dwc3->stop_hifiusb_time_stamp = jiffies;
}

static int usb_vbus_value(void)
{
	return pmic_get_vbus_status();
}

static void notify_speed_change_if_off(struct chip_dwc3_device *chip_dwc3,
				       enum otg_dev_event_type event)
{
	if ((event == ID_RISE_EVENT) ||
	    (event == CHARGER_DISCONNECT_EVENT)) {
		usb_dbg("it need notify USB_SPEED_UNKNOWN to app while usb plugout\n");
		chip_dwc3->speed = USB_SPEED_UNKNOWN;
		if (!queue_work(system_power_efficient_wq,
				&chip_dwc3->speed_change_work))
			usb_err("schedule speed_change_work wait:%d\n",
				chip_dwc3->speed);
	}
}

/*
 * return 0 means event was accepted, others means event was rejected.
 */
int chip_usb_queue_event(struct chip_usb_event *usb_event)
{
	int ret = 0;
#ifdef CONFIG_USB_DWC3_DUAL_ROLE
	unsigned long flags;
	struct chip_dwc3_device *chip_dwc3 = chip_dwc3_dev;
	enum otg_dev_event_type event;

	if (!usb_event)
		return -EINVAL;

	if (!chip_dwc3)
		return -ENODEV;

	if (chip_dwc3->eventmask) {
		usb_dbg("eventmask enabled, mask all events.\n");
		return -EPERM;
	}

	spin_lock_irqsave(&(chip_dwc3->event_lock), flags);

	event = usb_event->type;
	if (event_check(chip_dwc3->last_event, event)) {
		usb_dbg("event: %s\n", event_type_string(event));
		chip_dwc3->last_event = event;

		save_event_time_stamp(chip_dwc3, event);

		if ((event == CHARGER_CONNECT_EVENT)
				|| (event == CHARGER_DISCONNECT_EVENT))
			chip_dwc3_wake_lock(chip_dwc3);

		if (!event_enqueue(chip_dwc3, usb_event)) {
			if (!queue_work(system_power_efficient_wq,
					&chip_dwc3->event_work))
				usb_err("schedule event_work wait:%d\n", event);
		} else {
			usb_err("usb_otg_event can't enqueue event:%d\n", event);
			ret = -EBUSY;
		}

		notify_speed_change_if_off(chip_dwc3, event);
	} else {
		usb_err("last event: [%s], event [%s] was rejected.\n",
			event_type_string(chip_dwc3->last_event),
			event_type_string(event));
		ret = -EINVAL;
	}

	spin_unlock_irqrestore(&(chip_dwc3->event_lock), flags);
#endif
	return ret;
}
EXPORT_SYMBOL_GPL(chip_usb_queue_event);

int chip_usb_otg_event(enum otg_dev_event_type type)
{
	struct chip_usb_event event = {0};

	event.type = type;
	return chip_usb_queue_event(&event);
}
EXPORT_SYMBOL_GPL(chip_usb_otg_event);

int chip_usb_otg_use_hifi_ip_first(void)
{
	struct chip_dwc3_device *chip_dwc3 = chip_dwc3_dev;

	/*
	 * just check if usb module probe.
	 */
	if (!chip_dwc3) {
		usb_err("usb module not probe\n");
		return 0;
	}

	return ((chip_dwc3->hifi_ip_first)
				&& (get_hifi_usb_retry_count() == 0)
				&& (get_never_hifi_usb_value() == 0)
				&& (get_usbaudio_nv_is_ready() == 0));
}
EXPORT_SYMBOL_GPL(chip_usb_otg_use_hifi_ip_first);

int chip_usb_otg_get_typec_orien(void)
{
	struct chip_dwc3_device *chip_dwc3 = chip_dwc3_dev;

	/*
	 * just check if usb module probe.
	 */
	if (!chip_dwc3) {
		usb_err("usb module not probe\n");
		return 0;
	}

	return chip_dwc3->plug_orien;
}
EXPORT_SYMBOL_GPL(chip_usb_otg_get_typec_orien);

int chip_dwc3_is_powerdown(void)
{
	int power_flag = get_chip_dwc3_power_flag();

	return ((power_flag == USB_POWER_OFF) || (power_flag == USB_POWER_HOLD));
}

static int device_event_notifier_fn(struct notifier_block *nb,
			unsigned long event, void *para)
{
	struct chip_dwc3_device *chip_dwc = container_of(nb,
			struct chip_dwc3_device, event_nb);
	enum usb_device_speed  speed;
	unsigned long flags;

	usb_dbg("+\n");

	switch (event) {
	case DEVICE_EVENT_CONNECT_DONE:
		speed = *(enum usb_device_speed  *)para;

		/*
		 * Keep VDP_SRC if speed is USB_SPEED_SUPER
		 * and charger_type is CHARGER_TYPE_CDP.
		 */
		if (chip_dwc->charger_type == CHARGER_TYPE_CDP &&
				speed == USB_SPEED_SUPER)
			chip_usb_enable_vdp_src(chip_dwc);
		break;

	case DEVICE_EVENT_PULLUP:
		/* Disable VDP_SRC for communicaton on D+ */
		chip_usb_disable_vdp_src(chip_dwc);
		break;

	case DEVICE_EVENT_CMD_TMO:
		break;

	case DEVICE_EVENT_SETCONFIG:
		speed = *(enum usb_device_speed  *)para;
		spin_lock_irqsave(&chip_dwc->bc_again_lock, flags);
		if (chip_dwc->charger_type == CHARGER_TYPE_UNKNOWN)
			chip_dwc->charger_type = CHARGER_TYPE_SDP;

		chip_dwc->speed = speed;
		if (!queue_work(system_power_efficient_wq,
						&chip_dwc->speed_change_work))
			usb_err("schedule speed_change_work wait:%d\n", chip_dwc->speed);

		spin_unlock_irqrestore(&chip_dwc->bc_again_lock, flags);
		break;

	default:
		break;
	}

	usb_dbg("-\n");
	return 0;
}

void chip_usb_switch_controller(struct chip_dwc3_device *chip_dwc)
{
	(void)(chip_dwc);
	chip_usb_otg_event(SWITCH_CONTROLLER);
}

/*
 * The st310 is a super-speed mass storage device. A call may cause
 * disconnection. Once it disconnected, force USB as high-speed.
 */
static void st310_quirk(const struct usb_device *udev)
{
	int typec_state = PD_DPM_USB_TYPEC_DETACHED;

#define VID_HUAWEI_ST310 0x12D1
#define PID_HUAWEI_ST310 0x3B40

	if (udev->descriptor.idVendor == VID_HUAWEI_ST310 &&
			udev->descriptor.idProduct == PID_HUAWEI_ST310 &&
			udev->speed == USB_SPEED_SUPER) {
#ifdef CONFIG_TCPC_CLASS
		pd_dpm_get_typec_state(&typec_state);
#endif
		if (typec_state != PD_DPM_USB_TYPEC_DETACHED)
			chip_usb_otg_event(DISABLE_USB3_PORT);
	}
}

static int xhci_notifier_fn(struct notifier_block *nb,
			unsigned long action, void *data)
{
	struct usb_device *udev = (struct usb_device *)data;

	usb_dbg("+\n");

	if (!udev) {
		usb_dbg("udev is null,just return\n");
		return 0;
	}

	if ((action == USB_DEVICE_ADD) && (udev->descriptor.bDeviceClass == USB_CLASS_HUB)) {
		usb_dbg("usb hub don't notify\n");
		return 0;
	}

	if (((action == USB_DEVICE_ADD) || (action == USB_DEVICE_REMOVE))
		&& ((udev->parent != NULL) && (udev->parent->parent == NULL))) {
		usb_dbg("xhci device speed is %d action %s\n", udev->speed,
			(action == USB_DEVICE_ADD) ? "USB_DEVICE_ADD" : "USB_DEVICE_REMOVE");

		/* only device plug out while phone is host mode,not the usb cable */
		if (action == USB_DEVICE_REMOVE)
			chip_dwc3_dev->speed = USB_CONNECT_HOST;
		else
			chip_dwc3_dev->speed = udev->speed;

		if (action == USB_DEVICE_ADD)
			chip_dwc3_dev->device_add_time_stamp = jiffies;

		if (!queue_work(system_power_efficient_wq,
						&chip_dwc3_dev->speed_change_work))
			usb_err("schedule speed_change_work wait:%d\n", chip_dwc3_dev->speed);

		/* disable usb3.0 quirk for ST310-S1 */
		if (action == USB_DEVICE_REMOVE)
			st310_quirk(udev);
	}

	usb_dbg("-\n");
	return 0;
}

/**
 * get_usb_state() - get current USB cable state.
 * @chip_dwc: the instance pointer of struct chip_dwc3_device
 *
 * return current USB cable state according to VBUS status and ID status.
 */
static enum usb_state get_usb_state(const struct chip_dwc3_device *chip_dwc)
{
	if (chip_dwc->fpga_flag) {
		usb_dbg("this is fpga platform, usb is device mode\n");
		return USB_STATE_DEVICE;
	}

	if (usb_vbus_value() == 0)
		return USB_STATE_OFF;
	else
		return USB_STATE_DEVICE;
}

static void get_phy_param(struct chip_dwc3_device *chip_dwc3)
{
	struct device *dev = &chip_dwc3->pdev->dev;

	/* hs phy param for device mode */
	if (of_property_read_u32(dev->of_node, "eye_diagram_param",
			&(chip_dwc3->eye_diagram_param))) {
		usb_dbg("get eye diagram param form dt failed, use default value\n");
		chip_dwc3->eye_diagram_param = 0x1c466e3;
	}
	usb_dbg("eye diagram param: 0x%x\n", chip_dwc3->eye_diagram_param);

	/* hs phy param for host mode */
	if (of_property_read_u32(dev->of_node, "eye_diagram_host_param",
			&(chip_dwc3->eye_diagram_host_param))) {
		usb_dbg("get eye diagram host param form dt failed, use default value\n");
		chip_dwc3->eye_diagram_host_param = 0x1c466e3;
	}
	usb_dbg("eye diagram host param: 0x%x\n", chip_dwc3->eye_diagram_host_param);

	/* tx_vboost_lvl */
	if (of_property_read_u32(dev->of_node, "usb3_phy_tx_vboost_lvl",
			&(chip_dwc3->usb3_phy_tx_vboost_lvl))) {
		usb_dbg("get usb3_phy_tx_vboost_lvl form dt failed, use default value\n");
		chip_dwc3->usb3_phy_tx_vboost_lvl = VBOOST_LVL_DEFAULT_PARAM;
	}
	usb_dbg("usb3_phy_tx_vboost_lvl: %d\n", chip_dwc3->usb3_phy_tx_vboost_lvl);

	if (of_property_read_u32(dev->of_node, "vdp_src_disable",
		&(chip_dwc3->vdp_src_disable))) {
		usb_dbg("get vdp_src_disable form dt failed, use default value\n");
		chip_dwc3->vdp_src_disable = 0;
	}
	usb_dbg("vdp_src_disable: %d\n", chip_dwc3->vdp_src_disable);
}

static int dwc3_phy_init(struct chip_dwc3_device *chip_dwc,
		bool host_mode)
{
	unsigned int eye_diagram_param;
	int ret = -ENODEV;

	if (chip_dwc->use_new_frame) {
		chip_usb_unreset_phy_if_fpga();

		ret = chip_usb2_phy_init(chip_dwc->usb2_phy, host_mode);
		if (ret) {
			usb_err("usb2_phy_init failed\n");
			return ret;
		}

		ret = chip_usb3_phy_init(chip_dwc->usb3_phy,
					 chip_dwc->mode_type,
					 chip_dwc->plug_orien);
		if (ret) {
			usb_err("usb3_phy_init failed\n");
			if (chip_usb2_phy_exit(chip_dwc->usb2_phy))
				usb_err("usb2_phy_exit failed\n");
			return ret;
		}

		ret = chip_usb_controller_init(chip_dwc);
		if (ret) {
			usb_err("usb_controller_init failed\n");
			if (chip_usb2_phy_exit(chip_dwc->usb2_phy))
				usb_err("usb2_phy_exit failed\n");
			if (chip_usb3_phy_exit(chip_dwc->usb3_phy))
				usb_err("usb3_phy_exit failed\n");
			return ret;
		}

		set_chip_dwc3_power_flag(USB_POWER_ON);

		/* dwc3 core_ops should be called after power flag set */
		if (chip_dwc->usb3_phy && chip_dwc->mode_type == TCPC_DP)
			dwc3_core_disable_pipe_clock();
	} else if (chip_dwc->usb_phy->init) {
		if (host_mode)
			eye_diagram_param = chip_dwc->eye_diagram_host_param;
		else
			eye_diagram_param = chip_dwc->eye_diagram_param;
		return chip_dwc->usb_phy->init(chip_dwc->support_dp,
				eye_diagram_param,
				chip_dwc->usb3_phy_tx_vboost_lvl);
	}

	return ret;
}

static int dwc3_phy_shutdown(struct chip_dwc3_device *chip_dwc)
{
	int ret = -ENODEV;

	if (chip_dwc->use_new_frame) {
		set_chip_dwc3_power_flag(USB_POWER_HOLD);
		ret = chip_usb_controller_exit(chip_dwc);
		if (ret)
			usb_err("usb_controller_exit failed\n");

		ret = chip_usb2_phy_exit(chip_dwc->usb2_phy);
		if (ret)
			usb_err("usb2_phy_exit failed\n");

		ret = chip_usb3_phy_exit(chip_dwc->usb3_phy);
		if (ret)
			usb_err("usb3_phy_exit failed\n");

		set_chip_dwc3_power_flag(USB_POWER_OFF);
	} else if (chip_dwc->usb_phy->shutdown) {
		return chip_dwc->usb_phy->shutdown(chip_dwc->support_dp);
	}
	return ret;
}

static int dwc3_suspend_process(struct chip_dwc3_device *chip_dwc3,
				     bool host_mode)
{
	int ret;
	bool keep_power = false;

	usb_dbg("dwc3 in state %s\n",
		chip_usb_state_string(chip_dwc3->state));
	if (!host_mode && chip_dwc3->quirk_keep_u2_power_suspend) {
		ret = phy_power_on(chip_dwc3->usb2_phy);
		if (ret) {
			usb_err("usb2 phy poweron failed\n");
			return ret;
		}
		keep_power = true;
	}
	ret = dwc3_phy_shutdown(chip_dwc3);
	if (ret) {
		usb_err("dwc3_phy_shutdown failed in host state\n");
		goto err_u2_power_off;
	}

	return 0;

err_u2_power_off:
	if (keep_power) {
		if (phy_power_off(chip_dwc3->usb2_phy))
			usb_err("usb2 phy poweron failed\n");
	}

	return ret;
}

static int dwc3_resume_process(struct device *dev,
					struct chip_dwc3_device *chip_dwc3,
					bool host_mode)
{
	int ret;

	usb_dbg("dwc3 in state %s\n",
		chip_usb_state_string(chip_dwc3->state));
	ret = dwc3_phy_init(chip_dwc3, host_mode);
	if (ret)
		usb_err("dwc3_phy_init failed\n");
	pm_runtime_disable(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	return ret;
}

/* Currently this function only called in hifi usb mode */
static int dwc3_usb20_phy_init(struct chip_dwc3_device *chip_dwc,
		unsigned int combophy_flag)
{
	int ret = 0;

	usb_dbg("+\n");
	if (chip_dwc->use_new_frame) {
		chip_usb_unreset_phy_if_fpga();

		ret = chip_usb2_phy_init(chip_dwc->usb2_phy, true);
		if (ret) {
			usb_err("usb2_phy_init failed\n");
			return ret;
		}

		ret = chip_usb_controller_init(chip_dwc);
		if (ret) {
			usb_err("usb_controller_init failed\n");
			if (chip_usb2_phy_exit(chip_dwc->usb2_phy))
				usb_err("usb2_phy_exit failed\n");
		}
	} else if (chip_dwc->usb_phy->shared_phy_init) {
		return chip_dwc->usb_phy->shared_phy_init(chip_dwc->support_dp,
				chip_dwc->eye_diagram_host_param,
				combophy_flag);
	} else {
		WARN_ON(1);
	}
	usb_dbg("-\n");
	return ret;
}

static int dwc3_usb20_phy_shutdown(struct chip_dwc3_device *chip_dwc,
		unsigned int combophy_flag, unsigned int keep_power)
{
	int ret = 0;

	usb_dbg("+\n");
	if (chip_dwc->use_new_frame) {
		ret = chip_usb_controller_exit(chip_dwc);
		if (ret) {
			usb_err("usb_controller_exit failed\n");
			return ret;
		}
		if (keep_power) {
			ret = phy_power_on(chip_dwc->usb2_phy);
			if (ret)
				usb_err("usb2 phy poweron failed\n");
		}
		ret = chip_usb2_phy_exit(chip_dwc->usb2_phy);
		if (ret) {
			usb_err("usb2_phy_exit failed\n");
			if (chip_usb_controller_init(chip_dwc))
				usb_err("usb_controller_init failed\n");
		}
	} else if (chip_dwc->usb_phy->shared_phy_shutdown) {
		return chip_dwc->usb_phy->shared_phy_shutdown(chip_dwc->support_dp,
				combophy_flag, keep_power);
	} else {
		WARN_ON(1);
	}
	usb_dbg("-\n");
	return ret;
}

static void get_resource_for_fpga(struct chip_dwc3_device *chip_dwc3)
{
	struct device *dev = &chip_dwc3->pdev->dev;

	chip_dwc3->fpga_usb_mode_gpio = -1;
	chip_dwc3->fpga_otg_drv_vbus_gpio = -1;
	chip_dwc3->fpga_phy_reset_gpio = -1;
	chip_dwc3->fpga_phy_switch_gpio = -1;

	if (of_property_read_u32(dev->of_node, "fpga_flag",
			    &(chip_dwc3->fpga_flag)))
		chip_dwc3->fpga_flag = 0;

	if (chip_dwc3->fpga_flag == 0)
		return;

	usb_dbg("this is fpga platform\n");

	chip_dwc3->fpga_usb_mode_gpio = of_get_named_gpio(dev->of_node,
			"fpga_usb_mode_gpio", 0);
	chip_dwc3->fpga_otg_drv_vbus_gpio = of_get_named_gpio(dev->of_node,
			"fpga_otg_drv_vbus_gpio", 0);
	chip_dwc3->fpga_phy_reset_gpio = of_get_named_gpio(dev->of_node,
			"fpga_phy_reset_gpio", 0);
	chip_dwc3->fpga_phy_switch_gpio = of_get_named_gpio(dev->of_node,
			"fpga_phy_switch_gpio", 0);

	usb_dbg("fpga usb gpio info:usb_mode=%d, dr_vbus=%d, phy_reset=%d, phy_switch=%d\n",
			chip_dwc3->fpga_usb_mode_gpio, chip_dwc3->fpga_otg_drv_vbus_gpio,
			chip_dwc3->fpga_phy_reset_gpio, chip_dwc3->fpga_phy_switch_gpio);
}

static void get_quirks_dts(struct chip_dwc3_device *chip_dwc3)
{
	struct device *dev = &chip_dwc3->pdev->dev;

	chip_dwc3->quirk_keep_u2_power_suspend = device_property_read_bool(dev,
				"quirk-keep-u2-power-suspend");
}

/**
 * get_resource() - prepare resources
 * @chip_dwc3: the instance pointer of struct chip_dwc3_device
 *
 * 1. get registers base address and map registers region.
 * 2. get regulator handler.
 */
static int get_resource(struct chip_dwc3_device *chip_dwc3)
{
	struct device *dev = &chip_dwc3->pdev->dev;

	get_phy_param(chip_dwc3);

	get_resource_for_fpga(chip_dwc3);

	get_quirks_dts(chip_dwc3);

	if (of_property_read_u32(dev->of_node, "dma_mask_bit",
				&(chip_dwc3->dma_mask_bit)))
		chip_dwc3->dma_mask_bit = 32; /* DMA MASK */

	if (of_property_read_u32(dev->of_node, "hifi_ip_first",
				&(chip_dwc3->hifi_ip_first)))
		chip_dwc3->hifi_ip_first = 0;

#ifdef CONFIG_CONTEXTHUB_PD
	if (of_property_read_u32(dev->of_node, "usb_support_dp",
				&(chip_dwc3->support_dp))) {
		usb_err("usb driver not support dp\n");
		chip_dwc3->support_dp = 0;
	}
#else
	chip_dwc3->support_dp = 0;
#endif

	chip_dwc3->usb_support_s3_wakeup = device_property_read_bool(dev,
				"usb_support_s3_wakeup");
	if (of_property_read_u32(dev->of_node, "usb_support_check_voltage",
				&(chip_dwc3->check_voltage))) {
		usb_err("usb driver not support check voltage\n");
		chip_dwc3->check_voltage = 0;
	}

	if (of_property_read_u32(dev->of_node, "set_hi_impedance",
				&(chip_dwc3->set_hi_impedance))) {
		usb_err("usb driver not support set_hi_impedance\n");
		chip_dwc3->set_hi_impedance = 0;
	}

	return 0;
}

static void request_gpio(int *gpio)
{
	if (*gpio < 0)
		return;

	if (gpio_request((unsigned int)(*gpio), NULL)) {
		usb_err("request gpio %d failed\n", *gpio);
		*gpio = -1;
	}
}

static void request_gpios_for_fpga(struct chip_dwc3_device *chip_dwc)
{
	request_gpio(&chip_dwc->fpga_usb_mode_gpio);
	request_gpio(&chip_dwc->fpga_otg_drv_vbus_gpio);
	request_gpio(&chip_dwc->fpga_phy_reset_gpio);
	request_gpio(&chip_dwc->fpga_phy_switch_gpio);
}

int chip_usb_dwc3_register_phy(const struct chip_usb_phy *phy)
{
	if (g_chip_usb_phy)
		return -EBUSY;

	if (!phy)
		return -EINVAL;

	g_chip_usb_phy = phy;

	return 0;
}

int chip_usb_dwc3_unregister_phy(const struct chip_usb_phy *phy)
{
	if (g_chip_usb_phy != phy)
		return -EINVAL;

	g_chip_usb_phy = NULL;

	return 0;
}

static int dwc3_init_state(struct chip_dwc3_device *chip_dwc)
{
	int ret = 0;

	/* default device state  */
	chip_dwc->state = USB_STATE_DEVICE;

#ifdef CONFIG_USB_DWC3_DUAL_ROLE
	if (chip_dwc->fpga_flag != 0) {
		/* if vbus is on, detect charger type */
		if (usb_vbus_value()) {
			chip_dwc->charger_type =
				chip_usb_detect_charger_type(chip_dwc);
			notify_charger_type(chip_dwc);
		}

		if (sleep_allowed(chip_dwc))
			chip_dwc3_wake_unlock(chip_dwc);
		else
			chip_dwc3_wake_lock(chip_dwc);

		if (enumerate_allowed(chip_dwc)) {
			/* start peripheral */
			ret = extcon_set_state_sync(chip_dwc->edev,
					EXTCON_USB, true);
			if (ret) {
				chip_dwc3_wake_unlock(chip_dwc);
				usb_err("start peripheral error\n");
				return ret;
			}
		}

		chip_dwc->last_event = CHARGER_CONNECT_EVENT;

		if (get_usb_state(chip_dwc) == USB_STATE_OFF) {
			usb_dbg("init state: OFF\n");
			chip_usb_otg_event(CHARGER_DISCONNECT_EVENT);
		}
	} else {
		if (!usb_vbus_value()) {
			chip_dwc->charger_type = CHARGER_TYPE_NONE;
			/* vdp src may enable in fastboot, force disable */
			chip_dwc->vdp_src_enable = 1;
			chip_usb_disable_vdp_src(chip_dwc);
		}
		chip_dwc->state = USB_STATE_OFF;
		chip_dwc->last_event = CHARGER_DISCONNECT_EVENT;

		ret = dwc3_phy_shutdown(chip_dwc);
		if (ret)
			usb_err("dwc3_phy_shutdown failed (ret %d)\n", ret);

		chip_dwc3_wake_unlock(chip_dwc);
	}
#endif
	return ret;
}

static int usb_get_hardware(struct chip_dwc3_device *chip_usb)
{
	struct platform_device *pdev = chip_usb->pdev;
	struct device *dev = &pdev->dev;
	struct device_node *node = pdev->dev.of_node;

	chip_usb->notify_speed = of_property_read_bool(node,
			"notify_speed");

	chip_usb->use_new_frame = of_property_read_bool(node,
			"use_new_frame");

	if (chip_usb->use_new_frame) {
		return chip_usb_get_hw_res(chip_usb, dev);
	} else {
		if (!g_chip_usb_phy || !g_chip_usb_phy->otg_bc_reg_base) {
			usb_err("phy is NULL\n");
			return -EPROBE_DEFER;
		}

		chip_usb->usb_phy = g_chip_usb_phy;
	}

	return 0;
}

static void dwc3_initialize(struct chip_dwc3_device *chip_dwc)
{
	chip_dwc->charger_type = CHARGER_TYPE_SDP;
	chip_dwc->fake_charger_type = CHARGER_TYPE_NONE;
	INIT_KFIFO(chip_dwc->event_fifo);
	chip_dwc->last_event = NONE_EVENT;
	chip_dwc->eventmask = 0;
	chip_dwc->mode_type = TCPC_USB31_CONNECTED;
	chip_dwc->plug_orien = TYPEC_ORIEN_POSITIVE;
	spin_lock_init(&chip_dwc->event_lock);
	INIT_WORK(&chip_dwc->event_work, event_work);
	INIT_WORK(&chip_dwc->speed_change_work, dwc3_speed_change_work);
	mutex_init(&chip_dwc->lock);
	wakeup_source_init(&chip_dwc->wake_lock, "usb_wake_lock");
}

static const struct of_device_id combophy_func_of_match[] = {
	{ .compatible = "hisilicon,combophy-func", },
	{ }
};

static int controller_probe(struct chip_dwc3_device *chip_dwc)
{
	struct platform_device *pdev = chip_dwc->pdev;
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;

	if (chip_dwc->use_new_frame) {
		if (of_platform_bus_probe(node, combophy_func_of_match, dev))
			usb_err("register combophy failed or not support!\n");

		return chip_usb_controller_probe(chip_dwc);
	}

	return of_platform_populate(node, NULL, NULL, dev);
}

static void dwc3_dma_mask_config(struct chip_dwc3_device *chip_dwc)
{
	struct platform_device *pdev = chip_dwc->pdev;
	struct device *dev = &pdev->dev;

	chip_dwc->dma_mask_bit = chip_dwc->dma_mask_bit > 64 ?
			64 : chip_dwc->dma_mask_bit; /* 64-bit machine */
	dev->coherent_dma_mask = DMA_BIT_MASK(chip_dwc->dma_mask_bit);
	dev->dma_mask = &dev->coherent_dma_mask;
}

static int dwc3_runtime_enable(const struct chip_dwc3_device *chip_dwc)
{
	struct platform_device *pdev = chip_dwc->pdev;
	struct device *dev = &pdev->dev;
	int ret;

	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0) {
		usb_err("dwc3 pm_runtime_get_sync failed %d\n", ret);
		return ret;
	}

	pm_runtime_forbid(dev);
	return ret;
}

static int dwc3_device_event_nb(struct notifier_block *nb,
			unsigned long action, void *data)
{
	hiusb_pr_err("+\n");

	if (action == USB_GADGET_DEVICE_RESET) {
		hiusb_pr_err("usb device reset\n");
		chip_usb3_phy_dump_info(chip_dwc3_dev);
	}

	hiusb_pr_err("-\n");
	return NOTIFY_OK;
}

static int dwc3_device_register_notifier(struct chip_dwc3_device *chip_dwc)
{
	int ret;

	chip_dwc->xhci_nb.notifier_call = xhci_notifier_fn;
	usb_register_notify(&chip_dwc->xhci_nb);

	chip_dwc->event_nb.notifier_call = device_event_notifier_fn;
	ret = dwc3_device_event_notifier_register(&chip_dwc->event_nb);
	if (ret) {
		usb_unregister_notify(&chip_dwc->xhci_nb);
		chip_dwc->xhci_nb.notifier_call = NULL;
		chip_dwc->event_nb.notifier_call = NULL;
		hiusb_pr_err("dwc3 device event notifier register failed\n");
		return ret;
	}

	chip_dwc->device_event_nb.notifier_call = dwc3_device_event_nb;
	if (usb_blockerr_register_notify(&chip_dwc->device_event_nb)) {
		chip_dwc->device_event_nb.notifier_call = NULL;
		hiusb_pr_err("usb debug event register failed\n");
	}

	return 0;
}

static void dwc3_device_unregister_notifier(struct chip_dwc3_device *chip_dwc)
{
	if (chip_dwc->device_event_nb.notifier_call) {
		usb_blockerr_unregister_notify(&chip_dwc->device_event_nb);
		chip_dwc->device_event_nb.notifier_call = NULL;
	}

	if (chip_dwc->event_nb.notifier_call) {
		dwc3_device_event_notifier_unregister(&chip_dwc->event_nb);
		chip_dwc->event_nb.notifier_call = NULL;
	}

	if (chip_dwc->xhci_nb.notifier_call) {
		usb_unregister_notify(&chip_dwc->xhci_nb);
		chip_dwc->xhci_nb.notifier_call = NULL;
	}
}

static int dwc3_probe(struct platform_device *pdev)
{
	int ret;
	struct chip_dwc3_device *chip_dwc = NULL;
	struct device *dev = &pdev->dev;

	BUILD_BUG_ON(sizeof(struct chip_usb_event) != SIZE_CHIP_USB_EVENT);

	usb_dbg("+\n");

	/* [first] check arg & create dwc control struct */
	chip_dwc = devm_kzalloc(dev, sizeof(*chip_dwc), GFP_KERNEL);
	if (!chip_dwc)
		return -ENOMEM;

	platform_set_drvdata(pdev, chip_dwc);
	chip_dwc->pdev = pdev;

	ret = usb_get_hardware(chip_dwc);
	if (ret) {
		usb_err("get hardware failed ret %d\n", ret);
		return ret;
	}

	chip_dwc->edev = devm_extcon_dev_allocate(dev, usb_extcon_cable);
	if (IS_ERR(chip_dwc->edev)) {
		dev_err(dev, "failed to allocate extcon device\n");
		ret = PTR_ERR(chip_dwc->edev);
		goto put_hw_res;
	}

	ret = devm_extcon_dev_register(dev, chip_dwc->edev);
	if (ret < 0) {
		dev_err(dev, "failed to register extcon device\n");
		goto put_hw_res;
	}

	chip_dwc3_dev = chip_dwc;

	ret = get_resource(chip_dwc);
	if (ret) {
		dev_err(&pdev->dev, "get resource failed!\n");
		goto err_set_dwc3_null;
	}
	chip_dwc->core_ops = get_usb3_core_ops();
	request_gpios_for_fpga(chip_dwc);

	dwc3_dma_mask_config(chip_dwc);
	/* create sysfs&debugfs files. */
	ret = create_attr_file(chip_dwc);
	if (ret) {
		dev_err(&pdev->dev, "create_attr_file failed!\n");
		goto err_set_dwc3_null;
	}

	ret = chip_usb_register_hw_debugfs(chip_dwc);
	if (ret) {
		dev_err(&pdev->dev, "register hw debugfs failed!\n");
		goto err_set_dwc3_null;
	}

	/* initialize */
	dwc3_initialize(chip_dwc);

	ret = dwc3_device_register_notifier(chip_dwc);
	if (ret)
		goto err_remove_attr;

	/*
	 * enable runtime pm.
	 */
	ret = dwc3_runtime_enable(chip_dwc);
	if (ret < 0)
		goto err_notifier_unregister;

	ret = chip_usb_bc_init(chip_dwc);
	if (ret) {
		usb_err("usb_bc_init failed\n");
		goto err_pm_put;
	}

	/* power on */
	ret = dwc3_phy_init(chip_dwc, false);
	if (ret) {
		usb_err("dwc3_phy_init failed!\n");
		goto err_bc_exit;
	}

	ret = controller_probe(chip_dwc);
	if (ret) {
		usb_err("register controller failed %d!\n", ret);
		goto err_phy_exit;
	}

	ret = dwc3_init_state(chip_dwc);
	if (ret) {
		usb_err("dwc3_init_state failed!\n");
		goto err_remove_child;
	}

	pm_runtime_allow(dev);
	usb_dbg("-\n");

	return 0;

err_remove_child:
	device_for_each_child(dev, NULL, dwc3_remove_child);

err_phy_exit:
	if (dwc3_phy_shutdown(chip_dwc))
		usb_err("dwc3_phy_shutdown failed\n");

err_bc_exit:
	chip_usb_bc_exit(chip_dwc);

err_pm_put:
	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);

err_notifier_unregister:
	dwc3_device_unregister_notifier(chip_dwc);

err_remove_attr:
	remove_attr_file(chip_dwc);
	wakeup_source_trash(&chip_dwc->wake_lock);

err_set_dwc3_null:
	chip_dwc3_dev = NULL;

put_hw_res:
	chip_usb_put_hw_res(chip_dwc);

	return ret;
}

static int dwc3_remove(struct platform_device *pdev)
{
	struct chip_dwc3_device *chip_dwc3 = platform_get_drvdata(pdev);

	if (!chip_dwc3) {
		usb_err("dwc3 NULL\n");
		return -ENODEV;
	}

	device_for_each_child(&pdev->dev, NULL, dwc3_remove_child);

	chip_usb_bc_exit(chip_dwc3);

	dwc3_device_unregister_notifier(chip_dwc3);

	if (dwc3_phy_shutdown(chip_dwc3))
		usb_err("dwc3_phy_shutdown error\n");
	chip_dwc3->usb_phy = NULL;

	chip_usb_put_hw_res(chip_dwc3);

	remove_attr_file(chip_dwc3);

	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	wakeup_source_trash(&chip_dwc3->wake_lock);
	chip_dwc3_dev = NULL;

	return 0;
}

#ifdef CONFIG_PM
#ifdef CONFIG_PM_SLEEP
static int dwc3_prepare(struct device *dev)
{
	struct chip_dwc3_device *chip_dwc = platform_get_drvdata(
				to_platform_device(dev));
	int ret = 0;

	usb_dbg("+\n");

	if (!chip_dwc)
		return -ENODEV;

	mutex_lock(&chip_dwc->lock);

	switch (chip_dwc->state) {
	case USB_STATE_OFF:
		usb_info("off state.\n");
		break;
	case USB_STATE_DEVICE:
		usb_info("device state.\n");

		if (!enumerate_allowed(chip_dwc)) {
			usb_dbg("connected is a real charger\n");
			chip_usb_disable_vdp_src(chip_dwc);
		}

		break;
	case USB_STATE_HOST:
		usb_dbg("host mode, should not go to sleep!\n");
		ret = 0;
		break;
	case USB_STATE_HIFI_USB:
	case USB_STATE_HIFI_USB_HIBERNATE:
		break;
	default:
		usb_err("illegal state!\n");
		ret = -EFAULT;
		goto error;
	}

	usb_dbg("-\n");
	return ret;
error:
	mutex_unlock(&chip_dwc->lock);
	return ret;
}

static void dwc3_complete(struct device *dev)
{
	struct chip_dwc3_device *chip_dwc = platform_get_drvdata(
				to_platform_device(dev));
	usb_dbg("+\n");

	if (!chip_dwc) {
		usb_err("dwc NULL !\n");
		return;
	}

	switch (chip_dwc->state) {
	case USB_STATE_OFF:
		usb_dbg("off state.\n");
		break;
	case USB_STATE_DEVICE:
		usb_dbg("device state.charger_type[%d]\n", chip_dwc->charger_type);

		if (sleep_allowed(chip_dwc))
			chip_dwc3_wake_unlock(chip_dwc);
		else
			chip_dwc3_wake_lock(chip_dwc);

		/* do not start peripheral if real charger connected */
		if (!enumerate_allowed(chip_dwc))
			usb_dbg("a real charger connected\n");

		break;
	case USB_STATE_HOST:
		usb_err("host mode, should not go to sleep!\n");
		break;
	case USB_STATE_HIFI_USB:
	case USB_STATE_HIFI_USB_HIBERNATE:
		/* keep audio usb power on */
		break;
	default:
		usb_err("illegal state!\n");
		break;
	}

	mutex_unlock(&chip_dwc->lock);
	usb_dbg("-\n");
}

static int dwc3_common_suspend(struct device *dev)
{
	struct chip_dwc3_device *chip_dwc3 = platform_get_drvdata(to_platform_device(dev));
	int ret = 0;

	usb_dbg("+\n");

	if (!chip_dwc3) {
		usb_err("dwc3 NULL\n");
		return -EBUSY;
	}

	if (chip_dwc3->state == USB_STATE_DEVICE) {
		if (!sleep_allowed(chip_dwc3)) {
			usb_err("not sleep allowed\n");
			return -EBUSY;
		}

		ret = dwc3_suspend_process(chip_dwc3, false);
		if (ret)
			usb_err("dwc3_suspend_process failed in device state\n");
	} else if (chip_dwc3->state == USB_STATE_HOST) {
		ret = dwc3_suspend_process(chip_dwc3, true);
		if (ret)
			usb_err("dwc3_suspend_process failed in host state\n");
	} else {
		usb_dbg("dwc3 in state %s\n",
			chip_usb_state_string(chip_dwc3->state));
	}

	combophy_suspend_process();

	usb_dbg("-\n");

	return ret;
}

static int dwc3_common_resume(struct device *dev)
{
	struct chip_dwc3_device *chip_dwc3 = platform_get_drvdata(to_platform_device(dev));
	int ret = 0;

	usb_dbg("+\n");

	if (!chip_dwc3) {
		usb_err("dwc3 NULL\n");
		return -EBUSY;
	}

	combophy_resume_process();

	if (chip_dwc3->state == USB_STATE_DEVICE) {
		ret = dwc3_resume_process(dev, chip_dwc3, false);
		if (ret)
			usb_err("dwc3_resume_process failed in device state\n");
	} else if (chip_dwc3->state == USB_STATE_HOST) {
		ret = dwc3_resume_process(dev, chip_dwc3, true);
		if (ret)
			usb_err("dwc3_resume_process failed in host state\n");
	} else {
		usb_dbg("dwc3 in state %s\n",
			chip_usb_state_string(chip_dwc3->state));
	}

	usb_dbg("-\n");

	return ret;
}

static int dwc3_suspend(struct device *dev)
{
	struct chip_dwc3_device *chip_dwc3 = platform_get_drvdata(to_platform_device(dev));

	usb_dbg("+\n");

	if (!chip_dwc3) {
		usb_err("dwc3 NULL\n");
		return -EBUSY;
	}

	if (chip_dwc3->usb_support_s3_wakeup) {
		usb_dbg("for support s3 wakeup, don't showdown phy.\n");
		return 0;
	}

	return dwc3_common_suspend(dev);
}

static int dwc3_resume(struct device *dev)
{
	struct chip_dwc3_device *chip_dwc3 = platform_get_drvdata(to_platform_device(dev));

	usb_dbg("+\n");

	if (!chip_dwc3) {
		usb_err("dwc3 NULL\n");
		return -EBUSY;
	}

	if (chip_dwc3->usb_support_s3_wakeup) {
		usb_dbg("for support s3 wakeup, don't showdown phy.\n");
		return 0;
	}

	return dwc3_common_resume(dev);
}
#endif

static int dwc3_runtime_suspend(struct device *dev)
{
	usb_dbg("+\n");

	return 0;
}

static int dwc3_runtime_resume(struct device *dev)
{
	usb_dbg("+\n");

	return 0;
}

static int dwc3_runtime_idle(struct device *dev)
{
	usb_dbg("+\n");

	return 0;
}
#endif

const struct dev_pm_ops chip_dwc3_dev_pm_ops = {
#ifdef CONFIG_PM_SLEEP
	.prepare = dwc3_prepare,
	.complete = dwc3_complete,
	.suspend = dwc3_suspend,
	.resume = dwc3_resume,
	.freeze = dwc3_common_suspend,
	.thaw = dwc3_common_resume,
	.poweroff = dwc3_common_suspend,
	.restore = dwc3_common_resume,
#endif

	SET_RUNTIME_PM_OPS(dwc3_runtime_suspend, dwc3_runtime_resume,
			dwc3_runtime_idle)
};

static const struct of_device_id dwc3_match[] = {
	{ .compatible = "hisilicon,dwc3-usb" },
	{},
};
MODULE_DEVICE_TABLE(of, dwc3_match);

static struct platform_driver chip_dwc3_driver = {
	.probe		= dwc3_probe,
	.remove		= dwc3_remove,
	.driver		= {
		.name	= "chip-dwc3-usb",
		.of_match_table = of_match_ptr(dwc3_match),
		.pm	= &chip_dwc3_dev_pm_ops,
	},
};
module_platform_driver(chip_dwc3_driver);
