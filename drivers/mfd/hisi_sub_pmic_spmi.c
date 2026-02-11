/*
 *Copyright (c) 2019 Hisilicon.
 *Description: Device driver for regulators in HISI SUB PMIC IC
 *Author: Hisilicon
 *Create: 2019-05-23
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/hisi-spmi.h>
#include <linux/hisi/hisi_log.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_hisi_spmi.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#define HISI_LOG_TAG HISI_SUB_PMIC_TAG

static struct hisi_pmic *g_pmic;

u32 hisi_sub_pmic_read(struct hisi_pmic *pmic, int reg)
{
	u32 ret;
	u8 read_value = 0;
	struct spmi_device *pdev = NULL;

	if (g_pmic == NULL) {
		pr_err("g_pmic  is NULL\n");
		return 0;
	}

	pdev = to_spmi_device(g_pmic->dev);
	if (pdev == NULL) {
		pr_err("%s:pdev get failed!\n", __func__);
		return 0;
	}

	ret = spmi_ext_register_readl(
		pdev->ctrl, pdev->sid, reg, (unsigned char *)&read_value, 1);
	if (ret) {
		pr_err("%s:spmi_ext_register_readl failed!\n", __func__);
		return ret;
	}
	return (u32)read_value;
}
EXPORT_SYMBOL(hisi_sub_pmic_read);
void hisi_sub_pmic_write(struct hisi_pmic *pmic, int reg, u32 val)
{
	u32 ret;
	struct spmi_device *pdev = NULL;

	if (g_pmic == NULL) {
		pr_err(" g_pmic  is NULL\n");
		return;
	}
	pdev = to_spmi_device(g_pmic->dev);
	if (pdev == NULL) {
		pr_err("%s:pdev get failed!\n", __func__);
		return;
	}

	ret = spmi_ext_register_writel(pdev->ctrl, pdev->sid, reg,
		(unsigned char *)&val, 1); /*lint !e734 !e732 */
	if (ret) {
		pr_err("%s:spmi_ext_register_writel failed!\n", __func__);
		return;
	}
}
EXPORT_SYMBOL(hisi_sub_pmic_write);

unsigned int hisi_sub_pmic_reg_read(int addr)
{
	return (unsigned int)hisi_sub_pmic_read(g_pmic, addr);
}
EXPORT_SYMBOL(hisi_sub_pmic_reg_read);

void hisi_sub_pmic_reg_write(int addr, int val)
{
	hisi_sub_pmic_write(g_pmic, addr, val);
}
EXPORT_SYMBOL(hisi_sub_pmic_reg_write);

int hisi_sub_pmic_array_read(int addr, char *buff, unsigned int len)
{
	unsigned int i;

	if ((len > 32) || (buff == NULL))
		return -EINVAL;

	/*
	 * Here is a bug in the pmu die.
	 * the coul driver will read 4 bytes,
	 * but the ssi bus only read 1 byte, and the pmu die
	 * will make sampling 1/10669us about vol cur,so the driver
	 * read the data is not the same sampling
	 */
	for (i = 0; i < len; i++)
		*(buff + i) = hisi_sub_pmic_reg_read(addr + i);

	return 0;
}
int hisi_sub_pmic_array_write(int addr, const char *buff, unsigned int len)
{
	unsigned int i;

	if ((len > 32) || (buff == NULL))
		return -EINVAL;

	for (i = 0; i < len; i++)
		hisi_sub_pmic_reg_write(addr + i, *(buff + i));

	return 0;
}

static  const struct of_device_id of_hisi_pmic_match_tbl[] = {
	{
		.compatible = "hisilicon-hisi-sub-pmic-spmi",
	},
	{} };

static int hisi_sub_pmic_probe(struct spmi_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct hisi_pmic *pmic = NULL;
	int ret = 0;

	pmic = devm_kzalloc(dev, sizeof(*pmic), GFP_KERNEL);
	if (!pmic) {
		dev_err(dev, "cannot allocate hisi_pmic device info\n");
		return -ENOMEM;
	}
	pmic->dev = dev;
	g_pmic = pmic;
	/* get spmi sid */
	ret = of_property_read_u32(np, "slave_id", (u32 *)&(pdev->sid));
	if (ret) {
		pr_err("no slave_id property set, use default value\n");
		return -ENOMEM;
	}
	return 0;
}

static int hisi_sub_pmic_remove(struct spmi_device *pdev)
{

	struct hisi_pmic *pmic = dev_get_drvdata(&pdev->dev);

	devm_kfree(&pdev->dev, pmic);

	return 0;
}

static const struct spmi_device_id pmic_spmi_id[] = {
	{"sub", 0},
	{}
};

MODULE_DEVICE_TABLE(spmi, pmic_spmi_id);
static struct spmi_driver hisi_sub_pmic_driver = {
	.driver = {
			.name = "hisi_sub_pmic",
			.owner = THIS_MODULE,
			.of_match_table = of_hisi_pmic_match_tbl,
		},
	.id_table = pmic_spmi_id,
	.probe = hisi_sub_pmic_probe,
	.remove = hisi_sub_pmic_remove,
};

static int __init hisi_sub_pmic_init(void)
{
	return spmi_driver_register(&hisi_sub_pmic_driver);
}

static void __exit hisi_sub_pmic_exit(void)
{
	spmi_driver_unregister(&hisi_sub_pmic_driver);
}

subsys_initcall_sync(hisi_sub_pmic_init);
module_exit(hisi_sub_pmic_exit);

MODULE_DESCRIPTION("SUB PMIC driver");
MODULE_LICENSE("GPL v2");
