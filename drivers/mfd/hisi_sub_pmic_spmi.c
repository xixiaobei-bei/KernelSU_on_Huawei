/*
 *Copyright (c) 2019 Hisilicon.
 *Description: Device driver for regulators in HISI SUB PMIC IC
 *Author: Hisilicon
 *Create: 2019-05-23
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/hisi-spmi.h>
#include <linux/hisi/hisi_log.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/of_hisi_spmi.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "hisi_pmic_sim_hpd.h"

#ifndef NO_IRQ
#define NO_IRQ       0
#endif

#define HISI_LOG_TAG HISI_SUB_PMIC_TAG
#define PMIC_FPGA_FLAG          1

#define HISI_MASK_STATE			0xff
#define HISI_MASK_FIELD         0xFF
#define HISI_BITS                       8
#define PMIC_FPGA_FLAG          1

static struct hisi_pmic *g_pmic;
static struct hisi_pmic *g_mmw_pmic;

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

static u32 hisi_mmw_pmic_read(struct hisi_pmic *pmic, int reg)
{
	u32 ret;
	u8 read_value = 0;
	struct spmi_device *pdev = NULL;

	if (pmic == NULL) {
		pr_err("g_pmic  is NULL\n");
		return 0;
	}

	pdev = to_spmi_device(pmic->dev);
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
static void hisi_mmw_pmic_write(struct hisi_pmic *pmic, int reg, u32 val)
{
	u32 ret;
	struct spmi_device *pdev = NULL;

	if (pmic == NULL) {
		pr_err(" g_pmic  is NULL\n");
		return;
	}
	pdev = to_spmi_device(pmic->dev);
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

unsigned int hisi_mmw_pmic_reg_read(int addr)
{
	return (unsigned int)hisi_mmw_pmic_read(g_mmw_pmic, addr);
}
EXPORT_SYMBOL(hisi_mmw_pmic_reg_read);

void hisi_mmw_pmic_reg_write(int addr, int val)
{
	hisi_mmw_pmic_write(g_mmw_pmic, addr, val);
}
EXPORT_SYMBOL(hisi_mmw_pmic_reg_write);

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

static int get_sub_pmic_dieid_tree_data(
	struct spmi_device *pdev, struct hisi_pmic *pmic)
{
	int ret;
	struct device_node *root = NULL;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	root = of_find_compatible_node(
		np, NULL, "hisilicon-hisi-sub-pmic-dieid");
	if (!root) {
		pr_err("[%s]no hisilicon-hisi-sub-pmic-dieid root node.\n",
			__func__);
		return -ENODEV;
	}

	pmic->dieid_name = (char *)devm_kzalloc(
		dev, sizeof(*pmic->dieid_name), GFP_KERNEL);
	if (!pmic->dieid_name) {
		pr_err("[%s]kzalloc dieid_name failed.\n", __func__);
		return -ENOMEM;
	}
	ret = of_property_read_string(root,
		"hisilicon,hisi-sub-pmic-dieid-name",
		(const char **)&(pmic->dieid_name));
	if (ret) {
		pr_err("no hisilicon,hisi-sub-pmic-dieid-name property set\n");
		devm_kfree(dev, pmic->dieid_name);
		return -ENODEV;
	}

	ret = of_property_read_u32(root,
		"hisilicon,hisi-sub-pmic-dieid-reg-num",
		(u32 *)&(pmic->dieid_reg_num));
	if (ret) {
		pr_err("no hisilicon,hisi-sub-pmic-dieid-reg-num set\n");
		devm_kfree(dev, pmic->dieid_name);
		return -ENODEV;
	}
	pmic->dieid_regs = (u32 *)devm_kzalloc(
		dev, sizeof(u32) * pmic->dieid_reg_num, GFP_KERNEL);
	if (!pmic->dieid_regs) {
		pr_err("[%s]kzalloc dieid_regs buffer failed.\n", __func__);
		devm_kfree(dev, pmic->dieid_name);
		return -ENOMEM;
	}
	ret = of_property_read_u32_array(root,
		"hisilicon,hisi-sub-pmic-dieid-regs", pmic->dieid_regs,
		pmic->dieid_reg_num);
	if (ret) {
		pr_err("[%s]get hisi-sub-pmic-dieid-regs attribute failed.\n",
			__func__);
		devm_kfree(dev, pmic->dieid_name);
		devm_kfree(dev, pmic->dieid_regs);
		return -ENODEV;
	}
	return ret;
}

int hisi_subpmu_pmic_get_dieid(char *dieid, unsigned int len)
{
	int ret;
	unsigned int i;
	unsigned char value;
	unsigned int length;
	char pmu_buf[HISI_PMIC_DIEID_BUF] = {0};
	char buf[HISI_PMIC_DIEID_TEM_SAVE_BUF] = {0};

	if (dieid == NULL) {
		pr_err("%s  dieid  is NULL\n", __func__);
		return -ENOMEM;
	}

	if (g_pmic == NULL) {
		pr_err("%s g_pmic  is NULL\n", __func__);
		return -ENOMEM;
	}

	ret = snprintf(pmu_buf, sizeof(pmu_buf), "%s%s%s", "\r\n",
		g_pmic->dieid_name, ":0x");
	if (ret < 0) {
		pr_err("%s read main pmu dieid head fail.\n", __func__);
		return ret;
	}

	for (i = 0; i < g_pmic->dieid_reg_num; i++) {
		value = hisi_sub_pmic_reg_read(g_pmic->dieid_regs[i]);
		ret = snprintf(buf, sizeof(buf), "%02x",
			value);
		if (ret < 0) {
			pr_err("%s read main pmu dieid fail.\n", __func__);
			return ret;
		}
		strncat(pmu_buf, buf, strlen(buf));
	}

	strncat(pmu_buf, "\r\n", strlen("\r\n"));

	length = strlen(pmu_buf);
	if (len >= length) {
		strncat(dieid, pmu_buf, length);
	} else {
		pr_err("%s:dieid buf length is not enough!\n", __func__);
		return length;
	}

	return 0;
}

#define HISI_MMW_PMIC_COMP_STR	"hisilicon-hisi-mmw-pmic-spmi"

static  const struct of_device_id of_hisi_pmic_match_tbl[] = {
	{
		.compatible = "hisilicon-hisi-sub-pmic-spmi",
	},
	{
		.compatible = HISI_MMW_PMIC_COMP_STR,
	},
	{} };

static irqreturn_t hisi_irq_handler(int irq, void *data)
{
	struct hisi_pmic *pmic = (struct hisi_pmic *)data;
	unsigned long pending = 0;
	int i, offset;

	hisi_sub_pmic_sim_hpd_proc();

	for (i = 0; i < pmic->irqarray; i++) {
		pending = hisi_sub_pmic_reg_read(pmic->irq_addr_arry[i]);
		pending &= HISI_MASK_FIELD;
		if (pending != 0) {
			pr_info("pending[%d]=0x%lx\n\r", i, pending);
		}

		hisi_sub_pmic_reg_write(pmic->irq_addr_arry[i], pending);

		if (pending) {
			for_each_set_bit(offset, &pending, HISI_BITS)
				generic_handle_irq(pmic->irqs[offset + i * HISI_BITS]);/*lint !e679 */
		}
	}

	return IRQ_HANDLED;
}

static void hisi_irq_mask(struct irq_data *d)
{
	struct hisi_pmic *pmic = irq_data_get_irq_chip_data(d);
	u32 data, offset;
	unsigned long flags;

	if (NULL == g_pmic) {
		pr_err(" g_pmic  is NULL\n");
		return;
	}

	offset = (irqd_to_hwirq(d) >> 3);
	offset = pmic->irq_mask_addr_arry[offset];
	spin_lock_irqsave(&g_pmic->lock, flags);
	data = hisi_sub_pmic_reg_read(offset);
	data |= (1 << (irqd_to_hwirq(d) & 0x07));
	hisi_sub_pmic_reg_write(offset, data);
	spin_unlock_irqrestore(&g_pmic->lock, flags);
}

static void hisi_irq_unmask(struct irq_data *d)
{
	struct hisi_pmic *pmic = irq_data_get_irq_chip_data(d);
	u32 data, offset;
	unsigned long flags;

	if (NULL == g_pmic) {
		pr_err(" g_pmic  is NULL\n");
		return;
	}

	offset = (irqd_to_hwirq(d) >> 3);
	offset = pmic->irq_mask_addr_arry[offset];
	spin_lock_irqsave(&g_pmic->lock, flags);
	data = hisi_sub_pmic_reg_read(offset);
	data &= ~(1 << (irqd_to_hwirq(d) & 0x07)); /*lint !e502 */
	hisi_sub_pmic_reg_write(offset, data);
	spin_unlock_irqrestore(&g_pmic->lock, flags);
}

static struct irq_chip hisi_pmu_irqchip = {
	.name		= "hisi-irq",
	.irq_mask	= hisi_irq_mask,
	.irq_unmask	= hisi_irq_unmask,
	.irq_disable	= hisi_irq_mask,
	.irq_enable	= hisi_irq_unmask,
};

static int hisi_irq_map(struct irq_domain *d, unsigned int virq,
			  irq_hw_number_t hw)
{
	struct hisi_pmic *pmic = d->host_data;
	int ret = 0;

	irq_set_chip_and_handler_name(virq, &hisi_pmu_irqchip,
				      handle_simple_irq, "hisi");
	irq_set_chip_data(virq, pmic);
	ret = irq_set_irq_type(virq, IRQ_TYPE_NONE);
	if (ret < 0) {
		pr_err("irq set type fail\n");
	}
	return 0;
}

static struct irq_domain_ops hisi_domain_ops = {
	.map	= hisi_irq_map,
	.xlate	= irq_domain_xlate_twocell,
};

/*lint -e570 -e64*/
static int get_sub_pmic_device_tree_data(struct spmi_device *pdev, struct hisi_pmic *pmic)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	/*get pmic irq num*/
	ret = of_property_read_u32_array(np, "hisilicon,hisi-pmic-irq-num",
						&(pmic->irqnum), 1);
	if (ret) {
		pr_err("no hisilicon,hisi-pmic-irq-num property set\n");
		ret = -ENODEV;
		return ret;
	}

	/*get pmic irq array number*/
	ret = of_property_read_u32_array(np, "hisilicon,hisi-pmic-irq-array",
						&(pmic->irqarray), 1);
	if (ret) {
		pr_err("no hisilicon,hisi-pmic-irq-array property set\n");
		ret = -ENODEV;
		return ret;
	}

	pmic->irq_mask_addr_arry = (int *)devm_kzalloc(
		dev, sizeof(int) * pmic->irqarray, GFP_KERNEL);
	if (!pmic->irq_mask_addr_arry) {
		dev_err(dev, "[%s]kzalloc irq_mask_addr_arry buffer failed.\n",
			__func__);
		return -ENOMEM;
	}
	ret = of_property_read_u32_array(np,
		"hisilicon,hisi-pmic-irq-mask-addr",
		(int *)pmic->irq_mask_addr_arry, pmic->irqarray);
	if (ret) {
		pr_err("no hisilicon,hisi-pmic-irq-mask-addr property set\n");
		ret = -ENODEV;
		return ret;
	}

	pmic->irq_addr_arry = (int *)devm_kzalloc(
		dev, sizeof(int) * pmic->irqarray, GFP_KERNEL);
	if (!pmic->irq_addr_arry) {
		dev_err(dev, "[%s]kzalloc irq_addr_arry buffer failed.\n",
			__func__);
		return -ENOMEM;
	}

	ret = of_property_read_u32_array(np, "hisilicon,hisi-pmic-irq-addr",
		(int *)pmic->irq_addr_arry, pmic->irqarray);
	if (ret) {
		pr_err("no hisilicon,hisi-pmic-irq-addr property set\n");
		ret = -ENODEV;
		return ret;
	}

	return ret;
}/*lint -restore*/

static void hisi_sub_pmic_irq_prc(struct hisi_pmic *pmic)
{
	int i;
	for (i = 0; i < pmic->irqarray; i++) {
		hisi_sub_pmic_write(
			pmic, pmic->irq_mask_addr_arry[i], HISI_MASK_STATE);
	}

	for (i = 0; i < pmic->irqarray; i++) {
		unsigned int pending =
			hisi_sub_pmic_read(pmic, pmic->irq_addr_arry[i]);
		pr_debug("PMU IRQ address value:irq[0x%x] = 0x%x\n",
			pmic->irq_addr_arry[i], pending);
		hisi_sub_pmic_write(
			pmic, pmic->irq_addr_arry[i], HISI_MASK_STATE);
	}
}

static int hisi_sub_pmic_probe(struct spmi_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct hisi_pmic *pmic = NULL;
	int ret = 0;
	enum of_gpio_flags flags;
	unsigned int fpga_flag = 0;
	unsigned int virq;
	int i;
	bool is_process_irq = false;

	pmic = devm_kzalloc(dev, sizeof(*pmic), GFP_KERNEL);
	if (!pmic) {
		dev_err(dev, "cannot allocate hisi_pmic device info\n");
		return -ENOMEM;
	}

	/* get sub pmic dieid dts info */
	ret = get_sub_pmic_dieid_tree_data(pdev, pmic);
	if (ret) {
		dev_info(&pdev->dev, "Error reading hisi pmic dieid dts \n");
	}

	spin_lock_init(&pmic->lock);
	pmic->dev = dev;

	if (of_device_is_compatible(np, HISI_MMW_PMIC_COMP_STR)) {
		g_mmw_pmic = pmic;
	} else {
		g_pmic = pmic;
	}

	/* get spmi sid */
	ret = of_property_read_u32(np, "slave_id", (u32 *)&(pdev->sid));
	if (ret) {
		pr_err("no slave_id property set, use default value\n");
		return -ENOMEM;
	}

	is_process_irq = of_property_read_bool(np, "handle_irq");
	if (!is_process_irq) {
		pr_err(" sub pmu not need process irq\n");
		return 0;
	}

	ret = of_property_read_u32_array(np, "hisilicon,pmic_fpga_flag", &fpga_flag, 1);
	if (ret) {
		pr_err("no hisilicon,pmic_fpga_flag!\n");
	}
	if (PMIC_FPGA_FLAG == fpga_flag) {
		goto after_irq_register;
	}

	ret = get_sub_pmic_device_tree_data(pdev, pmic);
	if (ret) {
		dev_err(&pdev->dev, "Error reading hisi pmic  all dts \n");
		return ret;
	}

	pmic->gpio = of_get_gpio_flags(np, 0, &flags);
	if (pmic->gpio < 0)
		return pmic->gpio;

	if (!gpio_is_valid(pmic->gpio))
		return -EINVAL;
	ret = gpio_request_one(pmic->gpio, GPIOF_IN, "pmic1");
	if (ret < 0) {
		dev_err(dev, "failed to request gpio%d\n", pmic->gpio);
		return ret;
	}

	pmic->irq = gpio_to_irq(pmic->gpio);

	/* mask && clear IRQ status */
	hisi_sub_pmic_irq_prc(pmic);

	pmic->irqs = (unsigned int *)devm_kmalloc(dev, pmic->irqnum * sizeof(int), GFP_KERNEL);
	if (!pmic->irqs) {
		pr_err("%s:Failed to alloc memory for pmic irq number!\n", __func__);
		goto irq_malloc;
	}

	pmic->domain = irq_domain_add_simple(np, pmic->irqnum, 0,
					     &hisi_domain_ops, pmic);
	if (!pmic->domain) {
		dev_err(dev, "failed irq domain add simple!\n");
		ret = -ENODEV;
		goto irq_domain;
	}

	for (i = 0; i < pmic->irqnum; i++) {
		virq = irq_create_mapping(pmic->domain, i);
		if (virq == NO_IRQ) {
			pr_debug("Failed mapping hwirq\n");
			ret = -ENOSPC;
			goto irq_create_mapping;
		}
		pmic->irqs[i] = virq;
		pr_info("[%s]. pmic->irqs[%d] = %d\n", __func__, i, pmic->irqs[i]);
	}

	ret = request_threaded_irq(pmic->irq, hisi_irq_handler, NULL,
				IRQF_TRIGGER_LOW | IRQF_SHARED | IRQF_NO_SUSPEND,
				   "pmic1", pmic);
	if (ret < 0) {
		dev_err(dev, "could not claim pmic %d\n", ret);
		ret = -ENODEV;
		goto request_theaded_irq;
	}

after_irq_register:
	return 0;


request_theaded_irq:
irq_create_mapping:
irq_domain:
irq_malloc:
	gpio_free(pmic->gpio);
	g_pmic = NULL;
	return ret;
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
