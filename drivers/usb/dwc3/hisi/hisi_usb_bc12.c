/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: chip_usb_bc12.c for charger type check
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

#include "hisi_usb_bc12.h"
#include <linux/extcon.h>
#include <linux/version.h>
#if (KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE)
#include <linux/extcon-provider.h>
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/workqueue.h>

#include "hisi_usb_hw.h"

extern struct chip_dwc3_device *chip_dwc3_dev;
static BLOCKING_NOTIFIER_HEAD(charger_type_notifier);

int chip_charger_type_notifier_register(struct notifier_block *nb)
{
	if (!nb)
		return -EINVAL;
	return blocking_notifier_chain_register(&charger_type_notifier, nb);
}

int chip_charger_type_notifier_unregister(struct notifier_block *nb)
{
	if (!nb)
		return -EINVAL;
	return blocking_notifier_chain_unregister(&charger_type_notifier, nb);
}

void chip_charger_type_notify(enum hisi_charger_type charger_type, void *data)
{
	int ret;

	ret = blocking_notifier_call_chain(&charger_type_notifier, charger_type,
			data);
	if (ret)
		usb_err("cb return %d\n", ret);
}

void notify_charger_type(struct chip_dwc3_device *chip_dwc3)
{
	usb_dbg("+\n");
	chip_charger_type_notify(chip_dwc3->charger_type, chip_dwc3);

	if (chip_dwc3->charger_type == CHARGER_TYPE_DCP)
		if (chip_dwc3->set_hi_impedance && chip_dwc3->usb_phy->set_hi_impedance)
			chip_dwc3->usb_phy->set_hi_impedance();
	usb_dbg("-\n");
}

bool enumerate_allowed(const struct chip_dwc3_device *chip_dwc)
{
	if (chip_dwc->bc_again_delay_time == BC_AGAIN_DELAY_TIME_1)
		return false;

	/* do not start peripheral if real charger connected */
	return ((chip_dwc->charger_type == CHARGER_TYPE_SDP) ||
		(chip_dwc->charger_type == CHARGER_TYPE_CDP) ||
		(chip_dwc->charger_type == CHARGER_TYPE_UNKNOWN));
}

bool sleep_allowed(const struct chip_dwc3_device *chip_dwc)
{
	return ((chip_dwc->charger_type == CHARGER_TYPE_DCP) ||
		(chip_dwc->charger_type == CHARGER_TYPE_UNKNOWN));
}

bool bc_again_allowed(const struct chip_dwc3_device *chip_dwc)
{
	if (chip_dwc->bc_unknown_again_flag)
		return ((chip_dwc->charger_type == CHARGER_TYPE_SDP) ||
			(chip_dwc->charger_type == CHARGER_TYPE_UNKNOWN) ||
			(chip_dwc->charger_type == CHARGER_TYPE_CDP));
	else
		return ((chip_dwc->charger_type == CHARGER_TYPE_SDP) ||
			(chip_dwc->charger_type == CHARGER_TYPE_CDP));
}

static void bc_again(struct chip_dwc3_device *chip_dwc)
{
	int ret;
	bool isschedule = false;
	unsigned int bc_again_delay_time = 0;

	usb_dbg("+\n");

	/*
	 * Check usb controller status.
	 */
	if (chip_dwc3_is_powerdown()) {
		usb_err("usb controller is reset, just return\n");
		return;
	}

	/*
	 * STEP 1
	 * stop peripheral which is started when detected as SDP before
	 */
	if (enumerate_allowed(chip_dwc)) {
		ret = extcon_set_state_sync(chip_dwc->edev,
				EXTCON_USB, false);
		if (ret) {
			usb_err("stop peripheral error\n");
			return;
		}
	}

	/*
	 * STEP 2
	 * if it is CHARGER_TYPE_UNKNOWN, we should pull down d+&d- for 20ms
	 */
	if (chip_dwc->charger_type == CHARGER_TYPE_UNKNOWN) {
		chip_usb_dpdm_pulldown(chip_dwc);
		msleep(20); /* sleep 20ms */
		chip_usb_dpdm_pullup(chip_dwc);
	}

	chip_dwc->charger_type = chip_usb_detect_charger_type(chip_dwc);
	notify_charger_type(chip_dwc);

	if (chip_dwc->charger_type == CHARGER_TYPE_UNKNOWN) {
		unsigned long flags;

		spin_lock_irqsave(&chip_dwc->bc_again_lock, flags);
		if (chip_dwc->bc_again_delay_time == BC_AGAIN_DELAY_TIME_1) {
			chip_dwc->bc_again_delay_time = BC_AGAIN_DELAY_TIME_2;
			isschedule = true;
		}

		bc_again_delay_time = chip_dwc->bc_again_delay_time;
		spin_unlock_irqrestore(&chip_dwc->bc_again_lock, flags);
	} else {
		chip_dwc->bc_again_delay_time = 0;
	}

	/*
	 * STEP 3
	 * must recheck enumerate_allowed, because charger_type maybe changed,
	 * and enumerate_allowed according to charger_type
	 */
	if (enumerate_allowed(chip_dwc)) {
		/* start peripheral */
		ret = extcon_set_state_sync(chip_dwc->edev,
				EXTCON_USB, true);
		if (ret) {
			usb_err("start peripheral error\n");
			return;
		}
	} else {
		usb_dbg("it need notify USB_CONNECT_DCP while a real charger connected\n");
		chip_dwc->speed = USB_CONNECT_DCP;
		if (!queue_work(system_power_efficient_wq,
						&chip_dwc->speed_change_work))
			usb_err("schedule speed_change_work wait:%d\n", chip_dwc->speed);
	}

	/* recheck sleep_allowed for charger_type maybe changed */
	if (sleep_allowed(chip_dwc))
		chip_dwc3_wake_unlock(chip_dwc);
	else
		chip_dwc3_wake_lock(chip_dwc);

	if (isschedule) {
		ret = queue_delayed_work(system_power_efficient_wq,
				&chip_dwc->bc_again_work,
				msecs_to_jiffies(bc_again_delay_time));
		usb_dbg("schedule ret:%d, run bc_again_work %dms later\n",
			ret, bc_again_delay_time);
	}

	usb_dbg("-\n");
}

void chip_usb_otg_bc_again(void)
{
	struct chip_dwc3_device *chip_dwc = chip_dwc3_dev;

	usb_dbg("+\n");

	if (!chip_dwc) {
		usb_err("No usb module, can't call bc again api\n");
		return;
	}

	if ((chip_dwc->bc_again_flag == 1) && (chip_dwc->bc_unknown_again_flag == BC_AGAIN_ONCE)) {
		mutex_lock(&chip_dwc->lock);

		/* we are here because it's detected as SDP before */
		if (chip_dwc->charger_type == CHARGER_TYPE_UNKNOWN) {
			usb_dbg("charger_type is UNKNOWN, start bc_again_work\n");
			bc_again(chip_dwc);
		}

		mutex_unlock(&chip_dwc->lock);
	} else {
		usb_dbg("do nothing!\n");
	}
	usb_dbg("-\n");
}
EXPORT_SYMBOL_GPL(chip_usb_otg_bc_again);

static void bc_again_work(struct work_struct *work)
{
	struct chip_dwc3_device *chip_dwc = container_of(work,
			struct chip_dwc3_device, bc_again_work.work);

	usb_dbg("+\n");
	mutex_lock(&chip_dwc->lock);

	/* we are here because it's detected as SDP before */
	if (bc_again_allowed(chip_dwc)) {
		usb_dbg("charger_type is not DCP, start %s\n", __func__);
		bc_again(chip_dwc);
	}

	mutex_unlock(&chip_dwc->lock);
	usb_dbg("-\n");
}

/*
 * In some cases, DCP is detected as SDP wrongly. To avoid this,
 * start bc_again delay work to detect charger type once more.
 * If later the enum process is executed, then it's a real SDP, so
 * the work will be canceled.
 */
void schedule_bc_again(struct chip_dwc3_device *chip_dwc)
{
	int ret;
	unsigned long flags;
	unsigned int bc_again_delay_time;

	usb_dbg("+\n");

	if (!chip_dwc->bc_again_flag)
		return;

	spin_lock_irqsave(&chip_dwc->bc_again_lock, flags);
	if ((chip_dwc->charger_type == CHARGER_TYPE_UNKNOWN)
		&& (chip_dwc->bc_unknown_again_flag == BC_AGAIN_TWICE))
		chip_dwc->bc_again_delay_time = BC_AGAIN_DELAY_TIME_1;
	else
		chip_dwc->bc_again_delay_time = BC_AGAIN_DELAY_TIME_2;

	bc_again_delay_time = chip_dwc->bc_again_delay_time;
	spin_unlock_irqrestore(&chip_dwc->bc_again_lock, flags);

	ret = queue_delayed_work(system_power_efficient_wq,
			&chip_dwc->bc_again_work,
			msecs_to_jiffies(bc_again_delay_time));
	usb_dbg("schedule ret:%d, run bc_again_work %ums later\n",
		ret, bc_again_delay_time);

	usb_dbg("-\n");
}

void chip_usb_cancel_bc_again(int sync)
{
	if (!chip_dwc3_dev) {
		usb_err("chip_dwc3_dev is null\n");
		return;
	}

	cancel_bc_again(chip_dwc3_dev, sync);
}

void cancel_bc_again(struct chip_dwc3_device *chip_dwc, int sync)
{
	usb_dbg("+\n");
	if (chip_dwc->bc_again_flag) {
		int ret;

		if (sync)
			ret = cancel_delayed_work_sync(&chip_dwc->bc_again_work);
		else
			ret = cancel_delayed_work(&chip_dwc->bc_again_work);
		usb_dbg("cancel_delayed_work(result:%d)\n", ret);
		chip_dwc->bc_again_delay_time = 0;
	}
	usb_dbg("-\n");
}

int chip_usb_bc_init(struct chip_dwc3_device *chip_dwc)
{
	struct device *dev = &chip_dwc->pdev->dev;

	usb_dbg("+\n");

	spin_lock_init(&chip_dwc->bc_again_lock);

	if (of_property_read_u32(dev->of_node, "bc_again_flag",
			    &(chip_dwc->bc_again_flag)))
		chip_dwc->bc_again_flag = 0;

	if (chip_dwc->bc_again_flag) {
		INIT_DELAYED_WORK(&chip_dwc->bc_again_work, bc_again_work);
		if (of_property_read_u32(dev->of_node, "bc_unknown_again_flag",
			    &(chip_dwc->bc_unknown_again_flag)))
			chip_dwc->bc_unknown_again_flag = 0;
	} else {
		chip_dwc->bc_unknown_again_flag = 0;
	}
	usb_dbg("-\n");
	return 0;
}

void chip_usb_bc_exit(struct chip_dwc3_device *chip_dwc)
{
	usb_dbg("+\n");

	usb_dbg("-\n");
}
