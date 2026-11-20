#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/h2x_interface.h>
#include <linux/usb/hifi-usb-mailbox.h>
#include <linux/of_device.h>


#define dbg(fmt, arg...) \
	pr_info("[HIFI_USB][DBG]%s:%d "fmt, __func__, __LINE__, ##arg)
#define info(fmt, arg...) \
	pr_info("[HIFI_USB][INFO]%s:%d "fmt, __func__, __LINE__, ##arg)
#define err(fmt, arg...) \
	pr_err("[HIFI_USB][ERR]%s:%d "fmt, __func__, __LINE__, ##arg)

/* DWC3 registers memory space boundries */
#define DWC3_XHCI_REGS_START		0x0
#define DWC3_XHCI_REGS_END		0x7fff
#define DWC3_GLOBALS_REGS_START		0xc100


/* Global Registers */
#define DWC3_GSBUSCFG0		0xc100
#define DWC3_GCTL		0xc110
#define DWC3_GSNPSID		0xc120
#define DWC3_VER_NUMBER		0xc1a0
#define dwc3_gusb2phycfg(n)	(0xc200 + ((n) * 0x04))

/* Device Registers */
#define DWC3_DCTL		0xc704

/* Global Configuration Register */
#define dwc3_gctl_scaledown(n)		((n) << 4)
#define DWC3_GCTL_SCALEDOWN_MASK	dwc3_gctl_scaledown(3)
#define DWC3_GCTL_PRTCAP_HOST	1
#define DWC3_GCTL_PRTCAP_OTG	3
#define dwc3_gctl_prtcapdir(n)	((n) << 12)


/* Global Constants */
#define DWC3_GSNPSID_MASK	0xffff0000
#define DWC3_XHCI_RESOURCES_NUM	2


/* Global USB2 PHY Configuration Register */
#define dwc3_gusb2phycfg_phyif(n)	((n) << 3)
#define USBTRDTIM_UTMI_16_BIT		5
#define dwc3_gusb2phycfg_usbtrdtim(n)	((n) << 10)
#define DWC3_GUSB2PHYCFG_USBTRDTIM_MASK	dwc3_gusb2phycfg_usbtrdtim(0xf)
#define UTMI_PHYIF_16_BIT		1


/* Device Control Register */
#define DWC3_DCTL_RUN_STOP	(1 << 31)
#define DWC3_DCTL_CSFTRST	(1 << 30)
#define DWC3_REVISION_IS_DWC31		0x80000000

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
	u32			ap_use_hifiusb : 1;
};

struct hifi_usb *g_hifi_usb;

static int ap_use_hifi_usb_runstop_and_wait(struct hifi_usb *hifi_usb, bool run)
{
	int retval;
	__s32 result;

	dbg("+\n");
	dbg("run %d\n", run);
	init_completion(&hifi_usb->msg_completion);

	hifi_usb->runstop_msg.mesg_id = ID_AP_USE_HIFI_USB;
	hifi_usb->runstop_msg.reserved = 0;
	hifi_usb->runstop_msg.runstop = run;
	hifi_usb->runstop_msg.result = 0;

	result = hifi_usb_send_mailbox((struct hifi_usb_op_msg *)(&hifi_usb->runstop_msg),
			sizeof(hifi_usb->runstop_msg));
	if (result) {
		err("send mailbox to hifi failed\n");
		return result;
	}

	retval = wait_for_completion_timeout(&hifi_usb->msg_completion,
			HIFI_USB_MSG_TIMEOUT);
	if (retval == 0) {
		err("timeout!\n");
		WARN_ON(1);
		return -ETIMEDOUT;
	}

	result = hifi_usb->runstop_msg.result;
	dbg("result %d\n", result);
	dbg("-\n");
	return result;
}

static void handle_hifi_reset(struct work_struct *work)
{
	int ret;

	if (!g_hifi_usb) {
		err("g_hifi_usb is NULL\n");
		return;
	}

	err("hifi just restart\n");

	mutex_lock(&g_hifi_usb->lock);
	if (g_hifi_usb->ap_use_hifiusb) {
		/* resend runstop message */
		ret = ap_use_hifi_usb_runstop_and_wait(g_hifi_usb, 1);
		if (ret)
			err("ap_use_hifi_usb_runstop_and_wait failed err %d\n",
					ret);
	}
	mutex_unlock(&g_hifi_usb->lock);
}

void ap_use_hifi_usb_msg_receiver(void *msg_buf)
{
	struct hifi_usb_mesg_header *msg_header;
	struct hifi_usb_runstop_msg *runstop_msg;

	if (!g_hifi_usb) {
		err("receive mesg while g_hifi_usb is NULL\n");
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
				info("work already on the queue\n");
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
		err("this is not a DesignWare USB3 DRD Core\n");
		return -ENODEV;
	}

	/* set before PHY Interface SoftReset */
	reg = hifi_usb_readl(hifi_usb->regs, dwc3_gusb2phycfg(0));
	reg |= dwc3_gusb2phycfg_phyif(UTMI_PHYIF_16_BIT);
	hifi_usb_writel(hifi_usb->regs, dwc3_gusb2phycfg(0), reg);

	/* do SoftReset */
	timeout = jiffies + msecs_to_jiffies(500);

	reg = hifi_usb_readl(hifi_usb->regs, DWC3_DCTL);
	reg &= ~DWC3_DCTL_RUN_STOP;
	reg |= DWC3_DCTL_CSFTRST;
	hifi_usb_writel(hifi_usb->regs, DWC3_DCTL, reg);
	do {
		reg = hifi_usb_readl(hifi_usb->regs, DWC3_DCTL);
		if (!(reg & DWC3_DCTL_CSFTRST))
			break;

		if (time_after(jiffies, timeout)) {
			err("Reset Timed Out\n");
			return -ETIMEDOUT;
		}

		cpu_relax();
	} while (true);

	reg = hifi_usb_readl(hifi_usb->regs, DWC3_GCTL);
	reg &= ~DWC3_GCTL_SCALEDOWN_MASK;

	reg &= ~(dwc3_gctl_prtcapdir(DWC3_GCTL_PRTCAP_OTG));
	reg |= dwc3_gctl_prtcapdir(DWC3_GCTL_PRTCAP_HOST);

	hifi_usb_writel(hifi_usb->regs, DWC3_GCTL, reg);

	/* Set Turnaround Time = 9 (8-bit UTMI+ / ULPI) */
	reg = hifi_usb_readl(hifi_usb->regs, dwc3_gusb2phycfg(0));
	reg &= ~DWC3_GUSB2PHYCFG_USBTRDTIM_MASK;
	reg |= dwc3_gusb2phycfg_usbtrdtim(9);
	hifi_usb_writel(hifi_usb->regs, dwc3_gusb2phycfg(0), reg);

	/*
	 * WARNING:ASP USB Controller may access DDR failed by setting
	 * GSBUSCFG0 a higher value than 0x6 or INCRBRSTENA(0x1) been set.
	 */
	hifi_usb_writel(hifi_usb->regs, DWC3_GSBUSCFG0, 0x6);

	return 0;
}

static int hifi_usb_add_xhci_device(struct hifi_usb *hifi_usb)
{
	struct property_entry	props[4];
	int			prop_idx = 0;
	struct platform_device *xhci = NULL;
	int			ret;
	int			i;

	xhci = platform_device_alloc("xhci-hcd", PLATFORM_DEVID_AUTO);
	if (!xhci) {
		err("couldn't allocate xHCI device\n");
		return -ENOMEM;
	}

	xhci->dev.parent = hifi_usb->dev;
	dev_set_name(&xhci->dev, "hifi-usb-xhci");

	ret = platform_device_add_resources(xhci, hifi_usb->xhci_resources,
						DWC3_XHCI_RESOURCES_NUM);
	if (ret) {
		err("couldn't add resources to xHCI device\n");
		goto put_xhci;
	}

	memset(props, 0, sizeof(struct property_entry) * ARRAY_SIZE(props));

	if (device_property_read_bool(hifi_usb->dev, "hcd_local_mem"))
		props[prop_idx++].name = "hcd-local-mem";
	if (device_property_read_bool(hifi_usb->dev, "disable_lpm"))
		props[prop_idx++].name = "disable-lpm";
	if (device_property_read_bool(hifi_usb->dev, "not_support_sg"))
		props[prop_idx++].name = "not-support-sg";

	if (prop_idx >= NUMBER_PROPS) {
		ret = -EINVAL;
		goto put_xhci;
	}

	if (prop_idx) {
		for (i = 0; i < prop_idx; i++)
			hifi_usb->props[i].name = props[i].name;
		hifi_usb->num_props = prop_idx;
	}

	if (hifi_usb->num_props) {
		ret = platform_device_add_properties(xhci, hifi_usb->props);
		if (ret) {
			err("couldn't add properties to xHCI device\n");
			goto put_xhci;
		}
	}

	ret = platform_device_add(xhci);
	if (ret) {
		err("failed to register xHCI device\n");
		goto remove_prop;
	}

	hifi_usb->xhci = xhci;

	return 0;

remove_prop:
	if (hifi_usb->num_props)
		device_remove_properties(&xhci->dev);
put_xhci:
	platform_device_put(xhci);

	return ret;
}

int ap_start_use_hifiusb(void)
{
	int ret;

	dbg("+\n");
	if (!g_hifi_usb) {
		err("no device\n");
		return -ENODEV;
	}

	mutex_lock(&g_hifi_usb->lock);
	ret = usb_h2x_on();
	if (ret) {
		err("usb_h2x_on failed\n");
		mutex_unlock(&g_hifi_usb->lock);
		return ret;
	}

	ret = ap_use_hifi_usb_runstop_and_wait(g_hifi_usb, 1);
	if (ret) {
		err("ap_use_hifi_usb_runstop_and_wait failed err %d\n",
				ret);
		goto err_send_msg;
	}

	ret = hifi_usb_core_init(g_hifi_usb);
	if (ret) {
		err("hifi_usb_core_init failed err %d\n", ret);
		goto err_core_init;
	}

	g_hifi_usb->device_connect = 0;

	usb_register_notify(&g_hifi_usb->usb_nb);

	ret = hifi_usb_add_xhci_device(g_hifi_usb);
	if (ret)
		goto err_core_init;

	mod_timer(&g_hifi_usb->enum_device_timer,
				jiffies + AP_USE_HIFIUSB_ENUM_DEVICE_TIMEOUT);

	g_hifi_usb->ap_use_hifiusb = 1;
	mutex_unlock(&g_hifi_usb->lock);

	dbg("-\n");
	return 0;

err_core_init:
	if (ap_use_hifi_usb_runstop_and_wait(g_hifi_usb, 0))
		err("ap_use_hifi_usb_runstop_and_wait failed err %d\n",
				ret);
err_send_msg:
	if (usb_h2x_off())
		err("usb_h2x_off failed\n");
	mutex_unlock(&g_hifi_usb->lock);

	return ret;
}

void ap_stop_use_hifiusb(void)
{
	int ret;

	dbg("+\n");

	if (!g_hifi_usb) {
		err("no device\n");
		return;
	}

	del_timer_sync(&g_hifi_usb->enum_device_timer);

	mutex_lock(&g_hifi_usb->lock);

	platform_device_unregister(g_hifi_usb->xhci);

	usb_unregister_notify(&g_hifi_usb->usb_nb);

	ret = ap_use_hifi_usb_runstop_and_wait(g_hifi_usb, 0);
	if (ret)
		err("ap_use_hifi_usb_runstop_and_wait failed err %d\n",
				ret);

	ret = usb_h2x_off();
	if (ret)
		err("usb_h2x_off failed\n");

	g_hifi_usb->ap_use_hifiusb = 0;
	mutex_unlock(&g_hifi_usb->lock);

	dbg("-\n");
}

#define DSM_USB_DEVICE_INFO_LEN_MAX 50
static int usb_notifier_fn(struct notifier_block *nb,
			unsigned long action, void *data)
{
	struct usb_device *udev = (struct usb_device *)data;

	dbg("+\n");
	if (!udev || !udev->parent)
		return 0;

	if (action == USB_DEVICE_ADD) {
		info("udev USB_DEVICE_ADD\n");
		if (USB_CLASS_HUB != udev->descriptor.bDeviceClass) {
			g_hifi_usb->device_connect++;

			del_timer(&g_hifi_usb->enum_device_timer);
		}
	} else if (action == USB_DEVICE_REMOVE) {
		info("udev USB_DEVICE_REMOVE\n");
		if (USB_CLASS_HUB != udev->descriptor.bDeviceClass) {
			if (g_hifi_usb->device_connect)
				g_hifi_usb->device_connect--;
			else
				err("Removing device that has not been added!\n");
		}

		if (g_hifi_usb->device_connect == 0) {
			info("Thers's no device connected, switch to HOST\n");
			chip_usb_otg_event(STOP_AP_USE_HIFIUSB);
		}
	}
	dbg("-\n");

	return 0;
}

static void ap_use_hifiusb_enum_timeout(unsigned long data)
{
	err("enum device timeout!\n");
}

static int hifi_usb_plat_probe(struct platform_device *pdev)
{
	struct device		*dev = &pdev->dev;
	struct resource		*res;
	struct hifi_usb		*hifi_usb;
	void __iomem		*regs;
	int			ret;

	dbg("+\n");

	hifi_usb = devm_kzalloc(dev, sizeof(*hifi_usb), GFP_KERNEL);
	if (!hifi_usb)
		return -ENOMEM;

	hifi_usb->dev = dev;
	mutex_init(&hifi_usb->lock);

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		err("missing IRQ\n");
		return -ENODEV;
	}
	hifi_usb->xhci_resources[1].start = res->start;
	hifi_usb->xhci_resources[1].end = res->end;
	hifi_usb->xhci_resources[1].flags = res->flags;
	hifi_usb->xhci_resources[1].name = res->name;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		err("missing memory resource\n");
		return -ENODEV;
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
		goto probe_err;
	}

	hifi_usb->regs = regs;
	hifi_usb->regs_size = resource_size(res);

	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0) {
		err("pm_runtime_get_sync failed %d\n", ret);
		goto probe_err;
	}

	pm_runtime_forbid(dev);

	platform_set_drvdata(pdev, hifi_usb);

	hifi_usb->usb_nb.notifier_call = usb_notifier_fn;

	INIT_WORK(&hifi_usb->handle_hifi_reset_wk,
			handle_hifi_reset);

	setup_timer(&hifi_usb->enum_device_timer, ap_use_hifiusb_enum_timeout,
		    (unsigned long)hifi_usb);

	g_hifi_usb = hifi_usb;

	dbg("-\n");

	return 0;

probe_err:
	/*
	 * restore res->start back to its original value so that, in case the
	 * probe is deferred, we don't end up getting error in request the
	 * memory region the next time probe is called.
	 */
	res->start -= DWC3_GLOBALS_REGS_START;

	return ret;
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

	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

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

#ifdef CONFIG_PM
static int hifi_usb_runtime_suspend(struct device *dev)
{
	return 0;
}

static int hifi_usb_runtime_resume(struct device *dev)
{
	return 0;
}

static int hifi_usb_runtime_idle(struct device *dev)
{
	return -EBUSY;
}
#endif /* CONFIG_PM */

static const struct dev_pm_ops hifi_usb_dev_pm_ops = {
	SET_RUNTIME_PM_OPS(hifi_usb_runtime_suspend, hifi_usb_runtime_resume,
			hifi_usb_runtime_idle)
};

static struct platform_driver hifi_usb_plat_driver = {
	.probe	= hifi_usb_plat_probe,
	.remove	= hifi_usb_plat_remove,
	.driver	= {
		.name = "hifi-usb",
		.pm = &hifi_usb_dev_pm_ops,
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
