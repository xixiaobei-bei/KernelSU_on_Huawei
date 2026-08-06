#include <linux/extcon.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include "dwc3-otg.h"
#include "hisi_usb_bc12.h"

extern struct hisi_dwc3_device *hisi_dwc3_dev;

void notify_charger_type(struct hisi_dwc3_device *hisi_dwc3)
{
	usb_dbg("+\n");
	blocking_notifier_call_chain(&hisi_dwc3->charger_type_notifier,
				hisi_dwc3->charger_type, hisi_dwc3);

	if (hisi_dwc3->charger_type == CHARGER_TYPE_DCP)
		if (hisi_dwc3->phy_ops->set_hi_impedance)
			hisi_dwc3->phy_ops->set_hi_impedance(hisi_dwc3);
	usb_dbg("-\n");
}

bool enumerate_allowed(struct hisi_dwc3_device *hisi_dwc)
{
	if (hisi_dwc->bc_again_delay_time == BC_AGAIN_DELAY_TIME_1)
		return false;

	/* do not start peripheral if real charger connected */
	return ((hisi_dwc->charger_type == CHARGER_TYPE_SDP)
			|| (hisi_dwc->charger_type == CHARGER_TYPE_CDP)
			|| (hisi_dwc->charger_type == CHARGER_TYPE_UNKNOWN));
}

bool sleep_allowed(struct hisi_dwc3_device *hisi_dwc)
{
	return ((hisi_dwc->charger_type == CHARGER_TYPE_DCP)
			|| (hisi_dwc->charger_type == CHARGER_TYPE_UNKNOWN));
}

bool bc_again_allowed(struct hisi_dwc3_device *hisi_dwc)
{
	if (hisi_dwc->bc_unknown_again_flag)
		return ((hisi_dwc->charger_type == CHARGER_TYPE_SDP)
				|| (hisi_dwc->charger_type == CHARGER_TYPE_UNKNOWN)
				|| (hisi_dwc->charger_type == CHARGER_TYPE_CDP));
	else
		return ((hisi_dwc->charger_type == CHARGER_TYPE_SDP)
				|| (hisi_dwc->charger_type == CHARGER_TYPE_CDP));
}

static void bc_again(struct hisi_dwc3_device *hisi_dwc)
{
	int ret;
	bool isschedule = false;
	unsigned int bc_again_delay_time = 0;

	usb_dbg("+\n");

	/*
	 * Check usb controller status.
	 */
	if (hisi_dwc3_is_powerdown()) {
		usb_err("usb controller is reset, just return\n");
		return ;
	}

	/*
	 * STEP 1
	 */
	/* stop peripheral which is started when detected as SDP before */
	if (enumerate_allowed(hisi_dwc)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		ret = extcon_set_state_sync(hisi_dwc->edev,
				EXTCON_USB, false);
#else
		ret = dwc3_otg_work(DWC3_OTG_EVT_VBUS_CLEAR);
#endif
		if (ret) {
			usb_err("stop peripheral error\n");
			return;
		}
	}

	/*
	 * STEP 2
	 */
	/* if it is CHARGER_TYPE_UNKNOWN, we should pull down d+&d- for 20ms*/
	if (hisi_dwc->charger_type == CHARGER_TYPE_UNKNOWN) {
		hisi_bc_dplus_pulldown(hisi_dwc);
		msleep(20);
		hisi_bc_dplus_pullup(hisi_dwc);
	}

	hisi_dwc->charger_type = detect_charger_type(hisi_dwc);
	notify_charger_type(hisi_dwc);

	if (hisi_dwc->charger_type == CHARGER_TYPE_UNKNOWN) {
		unsigned long flags;

		spin_lock_irqsave(&hisi_dwc->bc_again_lock, flags);/*lint !e550*/
		if (hisi_dwc->bc_again_delay_time == BC_AGAIN_DELAY_TIME_1) {
			hisi_dwc->bc_again_delay_time = BC_AGAIN_DELAY_TIME_2;
			isschedule = true;
		}

		bc_again_delay_time = hisi_dwc->bc_again_delay_time;
		spin_unlock_irqrestore(&hisi_dwc->bc_again_lock, flags);/*lint !e550*/
	} else {
		hisi_dwc->bc_again_delay_time = 0;
	}

	/*
	 * STEP 3
	 */
	/* must recheck enumerate_allowed, because charger_type maybe changed,
	 * and enumerate_allowed according to charger_type */
	if (enumerate_allowed(hisi_dwc)) {
		/* start peripheral */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		ret = extcon_set_state_sync(hisi_dwc->edev,
				EXTCON_USB, true);
#else
		ret = dwc3_otg_work(DWC3_OTG_EVT_VBUS_SET);
#endif
		if (ret) {
			usb_err("start peripheral error\n");
			return;
		}
	} else {
		usb_dbg("it need notify USB_CONNECT_DCP while a real charger connected\n");
		hisi_dwc->speed = USB_CONNECT_DCP;
		if (!queue_work(system_power_efficient_wq,
						&hisi_dwc->speed_change_work)) {
			usb_err("schedule speed_change_work wait:%d\n", hisi_dwc->speed);
		}
	}

	/* recheck sleep_allowed for charger_type maybe changed */
	if (sleep_allowed(hisi_dwc))
		hisi_dwc3_wake_unlock(hisi_dwc);
	else
		hisi_dwc3_wake_lock(hisi_dwc);

	if (isschedule) {
		ret = queue_delayed_work(system_power_efficient_wq,
				&hisi_dwc->bc_again_work,
				msecs_to_jiffies(bc_again_delay_time));
		usb_dbg("schedule ret:%d, run bc_again_work %dms later\n",
			ret, bc_again_delay_time);
	}

	usb_dbg("-\n");
}
/*lint -restore */

void hisi_usb_otg_bc_again(void)
{
	struct hisi_dwc3_device *hisi_dwc = hisi_dwc3_dev;

	usb_dbg("+\n");

	if (!hisi_dwc) {
		usb_err("No usb module, can't call bc again api\n");
		return;
	}

	if ((1 == hisi_dwc->bc_again_flag) && (BC_AGAIN_ONCE == hisi_dwc->bc_unknown_again_flag)) {
		mutex_lock(&hisi_dwc->lock);

		/* we are here because it's detected as SDP before */
		if (hisi_dwc->charger_type == CHARGER_TYPE_UNKNOWN) {
			usb_dbg("charger_type is UNKNOWN, start bc_again_work\n");
			bc_again(hisi_dwc);
		}

		mutex_unlock(&hisi_dwc->lock);
	} else
		usb_dbg("hisi_usb_otg_bc_again do nothing!\n");

	usb_dbg("-\n");
}
EXPORT_SYMBOL_GPL(hisi_usb_otg_bc_again);

static void bc_again_work(struct work_struct *work)
{
	struct hisi_dwc3_device *hisi_dwc = container_of(work,
			struct hisi_dwc3_device, bc_again_work.work);

	usb_dbg("+\n");
	mutex_lock(&hisi_dwc->lock);

	/* we are here because it's detected as SDP before */
	if (bc_again_allowed(hisi_dwc)) {
		usb_dbg("charger_type is not DCP, start bc_again_work\n");
		bc_again(hisi_dwc);
	}

	mutex_unlock(&hisi_dwc->lock);
	usb_dbg("-\n");
}

/**
 * In some cases, DCP is detected as SDP wrongly. To avoid this,
 * start bc_again delay work to detect charger type once more.
 * If later the enum process is executed, then it's a real SDP, so
 * the work will be canceled.
 */
void schedule_bc_again(struct hisi_dwc3_device *hisi_dwc)
{
	int ret;
	unsigned long flags;
	unsigned int bc_again_delay_time;

	usb_dbg("+\n");

	if (!hisi_dwc->bc_again_flag)
		return;

	spin_lock_irqsave(&hisi_dwc->bc_again_lock, flags);/*lint !e550*/
	if ((hisi_dwc->charger_type == CHARGER_TYPE_UNKNOWN)
		&& (BC_AGAIN_TWICE == hisi_dwc->bc_unknown_again_flag))
		hisi_dwc->bc_again_delay_time = BC_AGAIN_DELAY_TIME_1;
	else
		hisi_dwc->bc_again_delay_time = BC_AGAIN_DELAY_TIME_2;

	bc_again_delay_time = hisi_dwc->bc_again_delay_time;
	spin_unlock_irqrestore(&hisi_dwc->bc_again_lock, flags);/*lint !e550*/

	ret = queue_delayed_work(system_power_efficient_wq,
			&hisi_dwc->bc_again_work,
			msecs_to_jiffies(bc_again_delay_time));
	usb_dbg("schedule ret:%d, run bc_again_work %dms later\n",
		ret, bc_again_delay_time);

	usb_dbg("-\n");
}

void hisi_usb_cancel_bc_again(int sync)
{
	if (!hisi_dwc3_dev) {
		usb_err("hisi_dwc3_dev is null\n");
		return;
	}

	cancel_bc_again(hisi_dwc3_dev, sync);
}

void cancel_bc_again(struct hisi_dwc3_device *hisi_dwc, int sync)
{
	usb_dbg("+\n");
	if (hisi_dwc->bc_again_flag) {
		int ret;
		if (sync)
			ret = cancel_delayed_work_sync(&hisi_dwc->bc_again_work);
		else
			ret = cancel_delayed_work(&hisi_dwc->bc_again_work);
		usb_dbg("cancel_delayed_work(result:%d)\n", ret);
		hisi_dwc->bc_again_delay_time = 0;
	}
	usb_dbg("-\n");
}
int hisi_usb_bc_init(struct hisi_dwc3_device *hisi_dwc)
{
	struct device *dev = &hisi_dwc->pdev->dev;

	usb_dbg("+\n");

	spin_lock_init(&hisi_dwc->bc_again_lock);/*lint !e550*/

	if (of_property_read_u32(dev->of_node, "bc_again_flag",
			    &(hisi_dwc->bc_again_flag))) {
		hisi_dwc->bc_again_flag = 0;
	}

	if (hisi_dwc->bc_again_flag) {
		INIT_DELAYED_WORK(&hisi_dwc->bc_again_work, bc_again_work);
		if (of_property_read_u32(dev->of_node, "bc_unknown_again_flag",
			    &(hisi_dwc->bc_unknown_again_flag))) {
			hisi_dwc->bc_unknown_again_flag = 0;
		}
	} else
		hisi_dwc->bc_unknown_again_flag = 0;

	usb_dbg("-\n");
	return 0;
}

void hisi_usb_bc_exit(struct hisi_dwc3_device *hisi_dwc)
{
	usb_dbg("+\n");

	usb_dbg("-\n");
}
