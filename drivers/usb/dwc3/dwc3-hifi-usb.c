#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/h2x_interface.h>
#include <linux/usb/hifi-usb-mailbox.h>
#include <linux/of_device.h>

#include "core.h"

#define DBG_HIFI_USB(fmt, arg...) \
	pr_info("[HIFI_USB][DBG]%s:%d "fmt, __func__, __LINE__, ##arg)
#define INFO_HIFI_USB(fmt, arg...) \
	pr_info("[HIFI_USB][INFO]%s:%d "fmt, __func__, __LINE__, ##arg)
#define ERR_HIFI_USB(fmt, arg...) \
	pr_err("[HIFI_USB][ERR]%s:%d "fmt, __func__, __LINE__, ##arg)

#define AP_USE_HIFIUSB_ENUM_DEVICE_TIMEOUT (5 * HZ)

#define NUMBER_PROPS 4

struct hifi_usb {
	struct device		*dev;
	struct platform_device	*xhci;
	struct resource		xhci_resources[DWC3_XHCI_RESOURCES_NUM];
	struct property_entry	props[NUMBER_PROPS];
	int			num_props;

	void __iomem		*regs;
	size_t			regs_size;

	u32			revision;

	struct mutex		lock;

	struct notifier_block	usb_nb;
	u32			device_connect;

	struct completion 	msg_completion;
	struct hifi_usb_runstop_msg runstop_msg;

	struct timer_list	enum_device_timer;

	struct work_struct	handle_hifi_reset_wk;
	u32			ap_use_hifiusb:1;
};

struct hifi_usb *g_hifi_usb;

static int ap_use_hifi_usb_runstop_and_wait(struct hifi_usb *hifi_usb, bool run)
{
	int retval;
	__s32 result;

	DBG_HIFI_USB("+\n");
	DBG_HIFI_USB("run %d\n", run);
	init_completion(&hifi_usb->msg_completion);

	hifi_usb->runstop_msg.mesg_id = ID_AP_USE_HIFI_USB; /*lint !e63 */
	hifi_usb->runstop_msg.reserved = 0; /*lint !e63 */
	hifi_usb->runstop_msg.runstop = run; /*lint !e63 */
	hifi_usb->runstop_msg.result = 0; /*lint !e63 */

	result = hifi_usb_send_mailbox(&hifi_usb->runstop_msg,
			sizeof(hifi_usb->runstop_msg));
	if (result) {
		ERR_HIFI_USB("send mailbox to hifi failed\n");
		return result;
	}

	retval = wait_for_completion_timeout(&hifi_usb->msg_completion,
			HIFI_USB_MSG_TIMEOUT);
	if (retval == 0) {
		ERR_HIFI_USB("timeout!\n");
		WARN_ON(1);
		return -ETIMEDOUT;
	}

	result = hifi_usb->runstop_msg.result;
	DBG_HIFI_USB("result %d\n", result);
	DBG_HIFI_USB("-\n");
	return result;
}

static void handle_hifi_reset(struct work_struct *work)
{
	int ret;

	if (!g_hifi_usb) {
		ERR_HIFI_USB("g_hifi_usb is NULL\n");
		return;
	}

	ERR_HIFI_USB("hifi just restart\n");

	mutex_lock(&g_hifi_usb->lock);
	if (g_hifi_usb->ap_use_hifiusb) {
		/* resend runstop message */
		ret = ap_use_hifi_usb_runstop_and_wait(g_hifi_usb, 1);
		if (ret)
			ERR_HIFI_USB("ap_use_hifi_usb_runstop_and_wait failed err %d\n",
					ret);
	}
	mutex_unlock(&g_hifi_usb->lock);
}

void ap_use_hifi_usb_msg_receiver(void *msg_buf)
{
	struct hifi_usb_mesg_header *msg_header;
	struct hifi_usb_runstop_msg *runstop_msg;

	if (!g_hifi_usb) {
		ERR_HIFI_USB("receive mesg while g_hifi_usb is NULL\n");
		return;
	}

	msg_header = (struct hifi_usb_mesg_header *)msg_buf;
	if (msg_header->msg_id == ID_AP_USE_HIFI_USB) {
		runstop_msg = (struct hifi_usb_runstop_msg *)msg_buf;
		g_hifi_usb->runstop_msg.result = runstop_msg->result;
		complete(&g_hifi_usb->msg_completion);
	} else if (msg_header->msg_id == ID_HIFI_AP_USB_INIT) {
		if (g_hifi_usb->ap_use_hifiusb) {
			if (!queue_work(system_power_efficient_wq,
					&g_hifi_usb->handle_hifi_reset_wk))
				INFO_HIFI_USB("work already on the queue\n");
		}
	}
}

static inline u32 hifi_usb_readl(void __iomem *base, u32 offset)
{
	u32 offs = offset - DWC3_GLOBALS_REGS_START;

	return readl(base + offs);
}

static inline void hifi_usb_writel(void __iomem *base, u32 offset, u32 value)
{
	u32 offs = offset - DWC3_GLOBALS_REGS_START;

	writel(value, base + offs);
}

static int hifi_usb_core_init(struct hifi_usb *hifi_usb)
{
	unsigned long timeout;
	u32		reg;

	reg = hifi_usb_readl(hifi_usb->regs, DWC3_GSNPSID);
	/* This should read as U3 followed by revision number */
	if ((reg & DWC3_GSNPSID_MASK) == 0x55330000) {
		/* Detected DWC_usb3 IP */
		hifi_usb->revision = reg;
	} else if ((reg & DWC3_GSNPSID_MASK) == 0x33310000) {
		/* Detected DWC_usb31 IP */
		hifi_usb->revision = hifi_usb_readl(hifi_usb->regs, DWC3_VER_NUMBER);
		hifi_usb->revision |= DWC3_REVISION_IS_DWC31;
	} else {
		ERR_HIFI_USB("this is not a DesignWare USB3 DRD Core\n");
		return -ENODEV;
	}

	/* set before PHY Interface SoftReset */
	reg = hifi_usb_readl(hifi_usb->regs, DWC3_GUSB2PHYCFG(0));
	reg |= DWC3_GUSB2PHYCFG_PHYIF(UTMI_PHYIF_16_BIT);
	hifi_usb_writel(hifi_usb->regs, DWC3_GUSB2PHYCFG(0), reg);

	/* do SoftReset */
	timeout = jiffies + msecs_to_jiffies(500);

	reg = hifi_usb_readl(hifi_usb->regs, DWC3_DCTL);
	reg &= ~DWC3_DCTL_RUN_STOP;	/*lint !e648 */
	reg |= DWC3_DCTL_CSFTRST;
	hifi_usb_writel(hifi_usb->regs, DWC3_DCTL, reg);
	do {
		reg = hifi_usb_readl(hifi_usb->regs, DWC3_DCTL);
		if (!(reg & DWC3_DCTL_CSFTRST))
			break;

		if (time_after(jiffies, timeout)) {
			ERR_HIFI_USB("Reset Timed Out\n");
			return -ETIMEDOUT;
		}

		cpu_relax();
	} while (true);

	reg = hifi_usb_readl(hifi_usb->regs, DWC3_GCTL);
	reg &= ~DWC3_GCTL_SCALEDOWN_MASK;

	reg &= ~(DWC3_GCTL_PRTCAPDIR(DWC3_GCTL_PRTCAP_OTG));
	reg |= DWC3_GCTL_PRTCAPDIR(DWC3_GCTL_PRTCAP_HOST);

	hifi_usb_writel(hifi_usb->regs, DWC3_GCTL, reg);

	/* Set Turnaround Time = 9 (8-bit UTMI+ / ULPI) */
	reg = hifi_usb_readl(hifi_usb->regs, DWC3_GUSB2PHYCFG(0));
	reg &= ~DWC3_GUSB2PHYCFG_USBTRDTIM_MASK;
	reg |= DWC3_GUSB2PHYCFG_USBTRDTIM(9);
	hifi_usb_writel(hifi_usb->regs, DWC3_GUSB2PHYCFG(0), reg);

	/*
	 * WARNING:ASP USB Controller may access DDR failed by setting
	 * GSBUSCFG0 a higher value than 0x6 or INCRBRSTENA(0x1) been set.
	 */
	hifi_usb_writel(hifi_usb->regs, DWC3_GSBUSCFG0, 0x6);

	return 0;
}

int ap_start_use_hifiusb(void)
{
	int ret;

	DBG_HIFI_USB("+\n");
	if (!g_hifi_usb) {
		ERR_HIFI_USB("no device\n");
		return -ENODEV;
	}

	mutex_lock(&g_hifi_usb->lock);
	ret = usb_h2x_on();
	if (ret) {
		ERR_HIFI_USB("usb_h2x_on failed\n");
		mutex_unlock(&g_hifi_usb->lock);
		return ret;
	}

	ret = ap_use_hifi_usb_runstop_and_wait(g_hifi_usb, 1);
	if (ret) {
		ERR_HIFI_USB("ap_use_hifi_usb_runstop_and_wait failed err %d\n",
				ret);
		goto err_send_msg;
	}

	ret = hifi_usb_core_init(g_hifi_usb);
	if (ret) {
		ERR_HIFI_USB("hifi_usb_core_init failed err %d\n", ret);
		goto err_core_init;
	}

	g_hifi_usb->device_connect = 0;

	usb_register_notify(&g_hifi_usb->usb_nb);

	if (g_hifi_usb->num_props) {
		ret = platform_device_add_properties(g_hifi_usb->xhci,
				g_hifi_usb->props);
		if (ret) {
			ERR_HIFI_USB("couldn't add properties to xHCI device\n");
			goto err_core_init;
		}
	}

	ret = platform_device_add(g_hifi_usb->xhci);
	if (ret) {
		ERR_HIFI_USB("failed to register xHCI device\n");
		goto err_prop_add;
	}

	mod_timer(&g_hifi_usb->enum_device_timer,
				jiffies + AP_USE_HIFIUSB_ENUM_DEVICE_TIMEOUT);

	g_hifi_usb->ap_use_hifiusb = 1;
	mutex_unlock(&g_hifi_usb->lock);

	DBG_HIFI_USB("-\n");
	return 0;

err_prop_add:
	if (g_hifi_usb->num_props)
		device_remove_properties(&g_hifi_usb->xhci->dev);
err_core_init:
	if (ap_use_hifi_usb_runstop_and_wait(g_hifi_usb, 0))
		ERR_HIFI_USB("ap_use_hifi_usb_runstop_and_wait failed err %d\n",
				ret);
err_send_msg:
	if (usb_h2x_off())
		ERR_HIFI_USB("usb_h2x_off failed\n");
	mutex_unlock(&g_hifi_usb->lock);

	return ret;
}

void ap_stop_use_hifiusb(void)
{
	int ret;

	DBG_HIFI_USB("+\n");

	if (!g_hifi_usb) {
		ERR_HIFI_USB("no device\n");
		return;
	}

	del_timer_sync(&g_hifi_usb->enum_device_timer);

	mutex_lock(&g_hifi_usb->lock);

	platform_device_del(g_hifi_usb->xhci);

	usb_unregister_notify(&g_hifi_usb->usb_nb);

	ret = ap_use_hifi_usb_runstop_and_wait(g_hifi_usb, 0);
	if (ret) {
		ERR_HIFI_USB("ap_use_hifi_usb_runstop_and_wait failed err %d\n",
				ret);
	}

	ret = usb_h2x_off();
	if (ret)
		ERR_HIFI_USB("usb_h2x_off failed\n");

	g_hifi_usb->ap_use_hifiusb = 0;
	mutex_unlock(&g_hifi_usb->lock);

	DBG_HIFI_USB("-\n");
}

#define DSM_USB_DEVICE_INFO_LEN_MAX (50)
static int usb_notifier_fn(struct notifier_block *nb,
			unsigned long action, void *data)
{
	struct usb_device *udev = (struct usb_device *)data;

	DBG_HIFI_USB("+\n");
	if (!udev || !udev->parent)
		return 0;

	if (action == USB_DEVICE_ADD) {
		INFO_HIFI_USB("udev USB_DEVICE_ADD\n");
		if (USB_CLASS_HUB != udev->descriptor.bDeviceClass) {
			g_hifi_usb->device_connect++;

			del_timer(&g_hifi_usb->enum_device_timer);
#ifdef CONFIG_HUAWEI_DSM
			if (udev->manufacturer && udev->product) {
				char manufacturer[DSM_USB_DEVICE_INFO_LEN_MAX + 1];
				char product[DSM_USB_DEVICE_INFO_LEN_MAX + 1];
				int len;

				len = strnlen(udev->manufacturer, DSM_USB_DEVICE_INFO_LEN_MAX);
				strncpy(manufacturer, udev->manufacturer, len);
				manufacturer[len] = '\0';

				len = strnlen(udev->product, DSM_USB_DEVICE_INFO_LEN_MAX);
				strncpy(product, udev->product, len);
				product[len] = '\0';
				power_dsm_dmd_report_format(POWER_DSM_USB, DSM_USB_RECORD_ABNORMAL_DEVICE,
						"manufacturer %s, product %s\n",
						manufacturer, product);
			}
#endif
		}
	} else if (action == USB_DEVICE_REMOVE) {
		INFO_HIFI_USB("udev USB_DEVICE_REMOVE\n");
		if (USB_CLASS_HUB != udev->descriptor.bDeviceClass) {
			if (g_hifi_usb->device_connect)
				g_hifi_usb->device_connect--;
			else
				ERR_HIFI_USB("Removing device that has not been added!\n");
		}

		if (g_hifi_usb->device_connect == 0) {
			INFO_HIFI_USB("Thers's no device connected, switch to HOST\n");
			hisi_usb_otg_event(STOP_AP_USE_HIFIUSB);
		}
	}
	DBG_HIFI_USB("-\n");

	return 0;
}

static void ap_use_hifiusb_enum_timeout(unsigned long data)
{
	ERR_HIFI_USB("enum device timeout!\n");
#ifdef CONFIG_HUAWEI_DSM
	power_dsm_dmd_report_format(POWER_DSM_USB, DSM_USB_AP_USE_HIFIUSB_ENUM_TIMEOUT,
			"enum device failed or mayby device has been pulled out\n");
#endif
}

static int hifi_usb_plat_probe(struct platform_device *pdev)
{
	struct property_entry	props[4];
	int			prop_idx = 0;
	struct device		*dev = &pdev->dev;
	struct resource		*res;
	struct hifi_usb		*hifi_usb;
	struct platform_device	*xhci;
	void __iomem		*regs;
	int			ret;
	int			i;

	DBG_HIFI_USB("+\n");

	hifi_usb = devm_kzalloc(dev, sizeof(*hifi_usb), GFP_KERNEL);
	if (!hifi_usb)
		return -ENOMEM;

	hifi_usb->dev = dev;
	mutex_init(&hifi_usb->lock);

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		ERR_HIFI_USB("missing IRQ\n");
		return -ENODEV; /*lint !e429 */
	}
	hifi_usb->xhci_resources[1].start = res->start;
	hifi_usb->xhci_resources[1].end = res->end;
	hifi_usb->xhci_resources[1].flags = res->flags;
	hifi_usb->xhci_resources[1].name = res->name;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		ERR_HIFI_USB("missing memory resource\n");
		return -ENODEV; /*lint !e429 */
	}

	hifi_usb->xhci_resources[0].start = res->start;
	hifi_usb->xhci_resources[0].end = hifi_usb->xhci_resources[0].start +
					DWC3_XHCI_REGS_END;
	hifi_usb->xhci_resources[0].flags = res->flags;
	hifi_usb->xhci_resources[0].name = res->name;

	res->start += DWC3_GLOBALS_REGS_START;

	/*
	 * Request memory region but exclude xHCI regs,
	 * since it will be requested by the xhci-plat driver.
	 */
	regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(regs)) {
		ret = PTR_ERR(regs);
		goto err0;
	}

	hifi_usb->regs = regs;
	hifi_usb->regs_size = resource_size(res);

	xhci = platform_device_alloc("xhci-hcd", PLATFORM_DEVID_AUTO);
	if (!xhci) {
		ERR_HIFI_USB("couldn't allocate xHCI device\n");
		ret = -ENOMEM;
		goto err0;
	}

	/* config dma of xhci device */
	of_dma_configure(&xhci->dev, dev->of_node);

	xhci->dev.parent = hifi_usb->dev;
	dev_set_name(&xhci->dev, "hifi-usb-xhci");

	ret = platform_device_add_resources(xhci, hifi_usb->xhci_resources,
						DWC3_XHCI_RESOURCES_NUM);
	if (ret) {
		ERR_HIFI_USB("couldn't add resources to xHCI device\n");
		goto err1;
	}

	memset(props, 0, sizeof(struct property_entry) * ARRAY_SIZE(props));

	if (device_property_read_bool(dev, "hcd_local_mem"))
		props[prop_idx++].name = "hcd-local-mem";
	if (device_property_read_bool(dev, "disable_lpm"))
		props[prop_idx++].name = "disable-lpm";
	if (device_property_read_bool(dev, "not_support_sg"))
		props[prop_idx++].name = "not-support-sg";

	if (prop_idx >= NUMBER_PROPS) {
		ret = -EINVAL;
		goto err1;
	}

	if (prop_idx) {
		for (i = 0; i < prop_idx; i++)
			hifi_usb->props[i].name = props[i].name;
		hifi_usb->num_props = prop_idx;
	}

	hifi_usb->xhci = xhci;
	platform_set_drvdata(pdev, hifi_usb);

	hifi_usb->usb_nb.notifier_call = usb_notifier_fn;

	INIT_WORK(&hifi_usb->handle_hifi_reset_wk,
			handle_hifi_reset);

	setup_timer(&hifi_usb->enum_device_timer, ap_use_hifiusb_enum_timeout,
		    (unsigned long)hifi_usb);

	g_hifi_usb = hifi_usb;

	DBG_HIFI_USB("-\n");

	return 0;

err1:
	platform_device_put(xhci);
err0:
	/*
	 * restore res->start back to its original value so that, in case the
	 * probe is deferred, we don't end up getting error in request the
	 * memory region the next time probe is called.
	 */
	res->start -= DWC3_GLOBALS_REGS_START;

	return ret; /*lint !e429 */
}

static int hifi_usb_plat_remove(struct platform_device *pdev)
{
	struct hifi_usb *hifi_usb = platform_get_drvdata(pdev);
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	del_timer_sync(&hifi_usb->enum_device_timer);

	mutex_lock(&hifi_usb->lock);
	if (hifi_usb->ap_use_hifiusb) {
		mutex_unlock(&hifi_usb->lock);
		return -EBUSY;
	}

	mutex_unlock(&hifi_usb->lock);

	flush_work(&hifi_usb->handle_hifi_reset_wk);

	g_hifi_usb = NULL;

	/*
	 * restore res->start back to its original value so that, in case the
	 * probe is deferred, we don't end up getting error in request the
	 * memory region the next time probe is called.
	 */
	if (res)
		res->start -= DWC3_GLOBALS_REGS_START;

	platform_device_unregister(hifi_usb->xhci);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id hifi_usb_of_match[] = {
	{
		.compatible = "hisilicon,feb-hifi-usb",
	},
	{ },
};
MODULE_DEVICE_TABLE(of, phcd_of_match);
#endif

#define DEV_PM_OPS	NULL

static struct platform_driver hifi_usb_plat_driver = {
	.probe	= hifi_usb_plat_probe,
	.remove	= hifi_usb_plat_remove,
	.driver	= {
		.name = "hifi-usb",
		.pm = DEV_PM_OPS,
		.of_match_table = of_match_ptr(hifi_usb_of_match),
	},
};

static int __init hifi_usb_plat_init(void)
{
	return platform_driver_register(&hifi_usb_plat_driver);
}

static void __exit hifi_usb_plat_exit(void)
{
	platform_driver_unregister(&hifi_usb_plat_driver);
}

module_init(hifi_usb_plat_init);
module_exit(hifi_usb_plat_exit);

MODULE_DESCRIPTION("dwc3 hifi usb");
MODULE_LICENSE("GPL");
