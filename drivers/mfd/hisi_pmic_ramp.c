/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_ramp.c
 *
 * Device driver for PMU RAMP interrupts
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "hisi_pmic_ramp.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdesc.h>
#include <linux/irqnr.h>
#include <linux/of.h>
#include <linux/err.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <linux/mfd/hisi_pmic_mntn.h>
#include <linux/mfd/hisi_pmic.h>

static unsigned int pmic_ramp_reg_read(
	 struct pmic_ramp *pmic_ramp, int addr)
{
	struct device_node *np = NULL;

	np = pmic_ramp->dev->of_node;

	if (of_device_is_compatible(np, PMU1_RAMP_COMP_STR))
		return hisi_sub_pmic_reg_read(addr);
	else
		return pmic_read_reg(addr);
}

static void pmic_ramp_reg_write(
	 struct pmic_ramp *pmic_ramp, int addr, int val)
{
	struct device_node *np = NULL;

	np = pmic_ramp->dev->of_node;

	if (of_device_is_compatible(np, PMU1_RAMP_COMP_STR))
		hisi_sub_pmic_reg_write(addr, val);
	else
		pmic_write_reg(addr, val);
}

static void ramp_event_regs_show(struct pmic_ramp *pmic_ramp)
{
	int i;
	u32 reg;
	u32 val;
	struct device *dev = pmic_ramp->dev;

	for (i = 0; i < pmic_ramp->event_reg_num; i++) {
		reg = pmic_ramp->event_regs[i];
		val = pmic_ramp_reg_read(pmic_ramp, reg);
		if (val)
			dev_err(dev, "%s ramp event reg: 0x%x = 0x%x\n",
				__func__, reg, val);
	}
}

#ifdef CONFIG_HISI_DEBUG_FS
void pmic_ramp_test(int pmu, u32 addr, u8 v1, u8 v2)
{
	pr_err("%s, pmu %d addr 0x%x, v1 0x%x, v2 0x%x\n", __func__,
		pmu, addr, v1, v2);
	if (pmu == 1) {
		hisi_sub_pmic_reg_write(addr, v1);
		hisi_sub_pmic_reg_write(addr, v2);
		hisi_sub_pmic_reg_write(addr, v1);
	} else {
		pmic_write_reg(addr, v1);
		pmic_write_reg(addr, v2);
		pmic_write_reg(addr, v1);
	}
}
#endif

static void ramp_event_regs_clr(struct pmic_ramp *pmic_ramp)
{
	int i;
	u32 reg;
	u32 val;

	for (i = 0; i < pmic_ramp->event_reg_num; i++) {
		reg = pmic_ramp->event_regs[i];
		val = pmic_ramp_reg_read(pmic_ramp, reg);
		if (val)
			pmic_ramp_reg_write(pmic_ramp, reg, val);
	}
}

static irqreturn_t pmic_ramp_irq_handle(int irq, void *data)
{
	int i;
	struct pmic_ramp *pmic_ramp = (struct pmic_ramp *)data;
	struct device *dev = pmic_ramp->dev;
	struct ramp_irq_info *ramp_irq = NULL;

	ramp_event_regs_show(pmic_ramp);

	for (i = 0; i < pmic_ramp->irq_num; i++) {
		ramp_irq = &pmic_ramp->ramp_irqs[i];
		if (ramp_irq->irq != irq)
			continue;
		dev_err(dev, "%s %s happen!\n", __func__, ramp_irq->irq_name);
		/* system panic, fastboot read event regs */
		if (ramp_irq->reset)
			pmic_mntn_panic_handler();

		break;
	}

	ramp_event_regs_clr(pmic_ramp);
	return IRQ_HANDLED;
}

static int ramp_irq_init(struct pmic_ramp *pmic_ramp)
{
	int i;
	int ret;
	struct device *dev = pmic_ramp->dev;
	struct device_node *np = dev->of_node;
	struct spmi_device *pdev = to_spmi_device(dev);
	struct ramp_irq_info *ramp_irq = NULL;

	for (i = 0; i < pmic_ramp->irq_num; i++) {
		ramp_irq = &pmic_ramp->ramp_irqs[i];

		ret = of_property_read_string_index(np,
			     "interrupt-names", i,
			     (const char **)&(ramp_irq->irq_name));
		if (ret) {
			dev_err(dev, "interrupt-names[%d] read fail\n", i);
			return ret;
		}

		ramp_irq->irq = spmi_get_irq_byname(pdev, NULL,
			ramp_irq->irq_name);
		if (ramp_irq->irq < 0) {
			dev_err(dev, "get %s irq fail\n", ramp_irq->irq_name);
			return -ENOENT;
		}

		ret = devm_request_irq(dev, ramp_irq->irq,
			pmic_ramp_irq_handle, IRQF_NO_SUSPEND,
			ramp_irq->irq_name, pmic_ramp);
		if (ret < 0) {
			dev_err(dev, "failed to request %s irq\n",
				ramp_irq->irq_name);
			return -ENOENT;
		}

		dev_info(dev, "interrupt-names[%d] %s, irq %d, reset %d\n", i,
			ramp_irq->irq_name, ramp_irq->irq, ramp_irq->reset);
	}

	return 0;
}
static int ramp_dts_para_init(
		struct pmic_ramp *pmic_ramp, struct device_node *np)
{
	int i;
	int ret;
	struct device *dev = pmic_ramp->dev;
	struct ramp_irq_info *ramp_irq = NULL;

	pmic_ramp->irq_num = of_property_count_strings(np, "interrupt-names");
	if (pmic_ramp->irq_num <= 0) {
		dev_err(dev, "interrupt-names is null\n");
		return -EINVAL;
	}

	pmic_ramp->ramp_irqs = (struct ramp_irq_info *)devm_kzalloc(dev,
		pmic_ramp->irq_num * sizeof(*ramp_irq), GFP_KERNEL);
	if (pmic_ramp->ramp_irqs == NULL)
		return -ENOMEM;

	for (i = 0; i < pmic_ramp->irq_num; i++) {
		ramp_irq = &pmic_ramp->ramp_irqs[i];
		ret = of_property_read_u32_index(np, "hisilicon,ramp-reset", i,
			&(ramp_irq->reset));
		if (ret) {
			dev_err(dev, "ramp-reset[%d] read fail\n", i);
			return ret;
		}
	}

	ret = of_property_read_u32(np, "hisilicon,ramp-event-reg-num",
						&pmic_ramp->event_reg_num);
	if (ret) {
		dev_err(dev, "no ramp-event-reg-num property set\n");
		return ret;
	}

	pmic_ramp->event_regs = (u32 *)devm_kzalloc(dev,
		pmic_ramp->event_reg_num * sizeof(u32),
		GFP_KERNEL);

	if (pmic_ramp->event_regs == NULL)
		return -ENOMEM;

	ret = of_property_read_u32_array(np,
		"hisilicon,ramp-event-regs", pmic_ramp->event_regs,
		pmic_ramp->event_reg_num);
	if (ret) {
		dev_err(dev, "[%s]get ramp-event-regs failed\n", __func__);
		return -ENODEV;
	}

	return ret;
}
static int pmic_ramp_probe(struct spmi_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct pmic_ramp *pmic_ramp = NULL;
	int ret;

	pmic_ramp = devm_kzalloc(dev, sizeof(*pmic_ramp), GFP_KERNEL);
	if (pmic_ramp == NULL)
		return -ENOMEM;

	pmic_ramp->dev = dev;

	ret = ramp_dts_para_init(pmic_ramp, np);
	if (ret) {
		dev_err(dev, "[%s]ramp_dts_para_init failed\n", __func__);
		return -ENODEV;
	}

	ret = ramp_irq_init(pmic_ramp);
	if (ret) {
		dev_err(dev, "[%s]ramp_irq_init failed\n", __func__);
		return -ENODEV;
	}

	dev_info(dev, "[%s] succ\n", __func__);
	return 0;
}

static int pmic_ramp_remove(struct spmi_device *pdev)
{
	return 0;
}

#ifdef CONFIG_PM
static int pmic_ramp_suspend(struct spmi_device *pdev, pm_message_t pm)
{
	return 0;
}

static int pmic_ramp_resume(struct spmi_device *pdev)
{
	return 0;
}
#endif

const static struct of_device_id pmic_ramp_match_tbl[] = {
	{
		.compatible = PMU0_RAMP_COMP_STR,
	},
	{
		.compatible = PMU1_RAMP_COMP_STR,
	},
	{ }    /* end */
};

static struct spmi_driver pmic_ramp_driver = {
	.driver = {
			.name = "hisilicon-hisi-spmi-pmic-ramp",
			.owner = THIS_MODULE,
			.of_match_table = pmic_ramp_match_tbl,
		},
	.probe = pmic_ramp_probe,
	.remove = pmic_ramp_remove,
#ifdef CONFIG_PM
	.suspend = pmic_ramp_suspend,
	.resume = pmic_ramp_resume,
#endif
};

static int __init pmic_ramp_init(void)
{
	int ret;

	ret = spmi_driver_register(&pmic_ramp_driver);
	if (ret)
		pr_err("[%s]spmi_driver_register failed\n", __func__);

	return ret;
}

static void __exit pmic_ramp_exit(void)
{
	spmi_driver_unregister(&pmic_ramp_driver);
}

module_init(pmic_ramp_init);
module_exit(pmic_ramp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("SPMI PMU RAMP Driver");
