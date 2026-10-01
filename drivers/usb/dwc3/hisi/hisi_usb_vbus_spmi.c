#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <linux/hisi/usb/hisi_usb.h>
#include "hisi_usb_vbus.h"

static int hisi_usb_spmi_get_irq_byname(void *device, const char *name)
{
	struct spmi_device *pdev = (struct spmi_device *)device;

	if(NULL == name) {
		return 0;
	}

	return spmi_get_irq_byname(pdev, &pdev->res, name);
}

static const struct usb_vbus_ops spmi_vbus_ops = {
	.get_irq_byname = hisi_usb_spmi_get_irq_byname
};

static int hisi_usb_spmi_vbus_probe(struct spmi_device *pdev)
{
	int ret;
	pr_info("[%s]+\n", __func__);

	ret = hisi_usb_vbus_request_irq(pdev, &spmi_vbus_ops);
	if (ret) {
		pr_err("usb spmi request irq failed: [%d] \n", ret);
	}

	pr_info("[%s]-\n", __func__);

	return ret;
}

static int hisi_usb_spmi_vbus_remove(struct spmi_device *pdev)
{
	hisi_usb_vbus_free_irq((void *)pdev);
	return 0;
}

/*lint -e785*/
static struct of_device_id hisi_usb_vbus_of_match_table[] = {
	{ .compatible = "hisilicon-usbvbus", },
	{ },
};

static const struct spmi_device_id hisi_usb_vbus_of_match[] = {
	{"hisilicon-usbvbus", 0},
	{}
};

static struct spmi_driver hisi_usb_vbus_drv = {
	.probe		= hisi_usb_spmi_vbus_probe,
	.remove		= hisi_usb_spmi_vbus_remove,
	.driver		= {
		.owner		= THIS_MODULE,
		.name		= "hisi_spmi_usb_vbus",
		.of_match_table	= hisi_usb_vbus_of_match_table,
	},
	.id_table = hisi_usb_vbus_of_match,
};
/*lint +e785*/
module_driver(hisi_usb_vbus_drv, spmi_driver_register, spmi_driver_unregister);

MODULE_AUTHOR("hisilicon");
MODULE_DESCRIPTION("This module detect USB VBUS connection/disconnection use spmi interface");
MODULE_LICENSE("GPL v2");
