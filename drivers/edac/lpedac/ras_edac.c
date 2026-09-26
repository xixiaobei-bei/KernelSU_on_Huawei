/*
 * ras_edac.c
 *
 * HISI RAS EDAC driver
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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
#include "ras_edac.h"
#include <asm/cputype.h>
#include <linux/kernel.h>
#include <linux/edac.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/smp.h>
#include <linux/cpu.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include "edac_mc.h"
#include "edac_device.h"

#define EDAC_CPU        "ras_edac"

static DEFINE_SPINLOCK(g_handler_lock);

LINX_CORE_RAS_NODE_LIST(DEFINE_ONE_RAS_NODE)
DEMIOS_CORE_RAS_NODE_LIST(DEFINE_ONE_RAS_NODE)
KLEIN_CORE_RAS_NODE_LIST(DEFINE_ONE_RAS_NODE)
L2_CLUSTER_RAS_NODE_LIST(DEFINE_ONE_RAS_NODE)
L3_MEM_RAS_NODE_LIST(DEFINE_ONE_RAS_NODE)

static struct ras_aux_data linx_core_ras_group[] = {
	LINX_CORE_RAS_GROUP_NODES
};

static struct ras_aux_data demios_core_ras_group[] = {
	DEMIOS_CORE_RAS_GROUP_NODES
};

static struct ras_aux_data klein_core_ras_group[] = {
	KLEIN_CORE_RAS_GROUP_NODES
};

static struct ras_aux_data l2_ras_group[] = {
	L2_CLUSTER_RAS_GROUP_NODES
};

static struct ras_aux_data l3_mem_ras_group[] = {
	L3_MEM_RAS_GROUP_NODES
};

static struct err_record_info ras_node_info[] = {
	ADD_ONE_ERR_GROUP(CPU_PARTNUM_LINX, 0, linx_core_ras_group),
	ADD_ONE_ERR_GROUP(CPU_PARTNUM_DEIMOS, 0, demios_core_ras_group),
	ADD_ONE_ERR_GROUP(CPU_PARTNUM_KLEIN, 1, klein_core_ras_group),
	ADD_ONE_ERR_GROUP(CPU_L2_CLUSTER, 2, l2_ras_group),
	ADD_ONE_ERR_GROUP(CPU_L3_MEM, 0, l3_mem_ras_group)
};

REGISTER_ERR_RECORD_INFO(ras_node_info);

static int edac_ecc_starting_cpu(unsigned int cpu, struct hlist_node *node)
{
	struct erp_drvdata *drv = hlist_entry_safe(node, struct erp_drvdata, node);
	cpumask_t mask;
	int *irq = NULL;
	int irq_num;
	int ret;
	struct erp_l2_drvdata *erp_l2_drv = NULL;
	int i;

	if (cpumask_test_cpu(cpu, &drv->supported_cpus) == 0) {
		pr_err("EDAC: %s failed. drv is %s, cpu = %u\n",
		       __func__, (drv == NULL) ? "NULL" : "NOT NULL", cpu);
		return 0;
	}

	irq = per_cpu(drv->cpu_drv->irq, cpu);
	irq_num = per_cpu(drv->cpu_drv->irq_num, cpu);
	for (i = 0; i < irq_num; i++) {
		if (irq[i] != 0) {
			pr_err("EADC:cpu = %u, irq = %d", cpu, irq[i]);
			ret = irq_force_affinity(irq[i], cpumask_of(cpu));
			if (ret != 0 && num_possible_cpus() > 1)
				pr_err("%s: Unable to set irq affinity irq=%d, cpu=%u\n", __func__, irq[i], cpu);
		}
	}

	erp_l2_drv = drv->l2_drv;

	while (erp_l2_drv != NULL) {
		if (cpumask_test_cpu(cpu, &erp_l2_drv->l2_affinity_cpu) != 0)
			break;
		erp_l2_drv = erp_l2_drv->l2_drv_next;
	}

	if (erp_l2_drv == NULL)
		return 0;

	irq = erp_l2_drv->irq;
	ret = cpumask_and(&mask, &erp_l2_drv->l2_affinity_cpu, cpu_online_mask);
	if (ret == 0) {
		pr_err("Mask is empty");
		return 0;
	}

	for (i = 0; i < erp_l2_drv->irq_num; i++) {
		ret = irq_force_affinity(irq[i], &mask);
		pr_err("EADC:cpu = %u, irq = %d", cpu, irq[i]);
		if (ret != 0)
			pr_err("%s: Unable to set irq affinity irq=%d, cpu=%u\n", __func__, irq[i], cpu);
	}
	return 0;
}

static int edac_ecc_stopping_cpu(unsigned int cpu, struct hlist_node *node)
{
	struct erp_drvdata *drv = hlist_entry_safe(node, struct erp_drvdata, node);
	int *irq = NULL;
	int irq_num;
	int i;
	int ret;

	if (cpumask_test_cpu(cpu, &drv->supported_cpus) == 0) {
		return 0;
	}

	irq = per_cpu(drv->cpu_drv->irq, cpu);
	irq_num = per_cpu(drv->cpu_drv->irq_num, cpu);
	pr_err("EDAC:cpu = %u, irq_num = %d, irq = %p\n", cpu, irq_num, irq);
	for (i = 0; i < irq_num; i++) {
		if (irq[i] != 0) {
			pr_err("EADC:cpu = %u, irq = %d", cpu, irq[i]);
			ret = irq_force_affinity(irq[i], 0);
			if (ret != 0)
				pr_err("%s: Unable to set irq affinity irq=%d, cpu=%u\n", __func__, irq[i], cpu);
		}
	}

	return 0;
}

/* find node err */
static int find_node_err(u32 part_num, struct err_record_info **info)
{
	unsigned int i;

	for_each_err_record_info(i, *info) {
		if ((*info)->node_number == part_num) {
			pr_err("%s part num is %x\n", __func__, part_num);
			return 0;
		}
	}

	return -ENODEV;
}

static bool parse_serr_err(u64 status,const struct ras_error *errors)
{
	u32 serr;
	bool found = false;
	int i;

	serr = (u32)ERR_STATUS_GET_FIELD(status, SERR);
	for (i = 0; errors[i].error_msg != NULL; i++) {
		if (serr == errors[i].serr_idx) {
			pr_err("SERR = %s: 0x%x\n", errors[i].error_msg, serr);
			found = true;
			break;
		}
	}

	return found;
}

/* parse err node */
static void parse_node_err(struct err_record_info *info)
{
	u32 num_idx = info->sysreg.num_idx;
	u32 start_idx = info->sysreg.start_idx;
	u64 status, erxmisc0;
	struct ras_aux_data *aux_data = info->aux_data;
	unsigned int i;
	bool found = false;

	for (i = 0; i < num_idx; i++) {
		write_errselr_el1(start_idx + i);
		isb();
		status = read_erxstatus_el1();

		if (ERR_STATUS_GET_FIELD(status, V) != 0U) {
			erxmisc0 = read_erxmisc0_el1();
			edac_printk(KERN_CRIT, EDAC_CPU,
				    "CPU%d detected a cache error, status = %llx, errxmisc0 = %llx\n",
				    smp_processor_id(), status, erxmisc0);
			found = parse_serr_err(status, aux_data[i].error_records);
			if (!found)
				pr_err("Unknown SERR\n");
			write_erxstatus_el1(status);
			write_erxmisc0_el1(aux_data[i].err_misc);
		}
	}
}

/* parse l3 err node */
static int parse_l3_node_err(struct err_record_info *info)
{
	u32 num_idx = info->sysreg.num_idx;
	u64 status, erxmisc0;
	struct ras_aux_data *aux_data = info->aux_data;
	unsigned int i;
	bool found = false;

	for (i = 0; i < num_idx; i++) {
		status = read_mem_data(L3_ERRSTATUS);

		if (ERR_STATUS_GET_FIELD(status, V) != 0U) {
			erxmisc0 = read_mem_data(L3_ERRMISC0);
			edac_printk(KERN_CRIT, EDAC_CPU,
				    "L3 detected a cache error, errxstatus = %llx, errxmisc = %llx\n",
				    status, erxmisc0);
			found = parse_serr_err(status, aux_data[i].error_records);
			if (!found)
				pr_err("Unknown SERR\n");
			write_mem_data(status, L3_ERRSTATUS);
			write_mem_data(aux_data[i].err_misc, L3_ERRMISC0);
		}
	}

	return 0;
}

static int parse_per_cpu_irq(struct device_node *child, struct platform_device *pdev, int cpu)
{
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	struct erp_cpu_drvdata *erp_cpu_drv = NULL;
	int irq_cnts;
	int i;

	irq_cnts = of_irq_count(child);
	if (irq_cnts <= 0)
		return -ENODEV;

	erp_cpu_drv = per_cpu_ptr(drv->cpu_drv, cpu);
	erp_cpu_drv->irq_num = irq_cnts;
	erp_cpu_drv->irq = devm_kcalloc(&pdev->dev, irq_cnts,
					sizeof(int), GFP_KERNEL);

	if (erp_cpu_drv->irq == NULL)
		return -ENOMEM;

	for (i = 0; i < irq_cnts; i++)
		erp_cpu_drv->irq[i] = of_irq_get(child, i);

	return 0;
}

static int parse_cpu_irqs(struct platform_device *pdev)
{
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	struct device_node *cpu_node = NULL;
	struct device_node *child = NULL;
	struct device_node *dn = NULL;
	int i;
	int ret;

	drv->cpu_drv = alloc_percpu(struct erp_cpu_drvdata);
	if (drv->cpu_drv == NULL) {
		dev_err(&pdev->dev, "failed to alloc per-cpu drvdata\n");
		ret = -ENOMEM;
		goto err_out;
	}

	cpu_node = of_get_child_by_name(pdev->dev.of_node, "cpu-ecc");

	if (cpu_node == NULL) {
		dev_err(&pdev->dev, "cpu-ecc node not found\n");
		ret = -ENODEV;
		goto err_out;
	}

	i = 0;
	for_each_child_of_node(cpu_node, child) {
		dn = of_parse_phandle(child, "interrupt-affinity", 0);
		if (dn == NULL) {
			pr_debug("failed to parse interrupt-affinity for %s\n",
				 child->name);
			continue;
		}
		dev_err(&pdev->dev, "child number is %d\n", i);
		ret = parse_per_cpu_irq(child, pdev, i);
		if (ret != 0)
			goto err_out;
		cpumask_set_cpu(i, &drv->supported_cpus);
		i++;
	}

	of_node_put(pdev->dev.of_node);

err_out:
	of_node_put(pdev->dev.of_node);
	free_percpu(drv->cpu_drv);
	return ret;
}

static irqreturn_t cpu_edac_handler(int irq, void *drvdata)
{
	struct erp_drvdata *drv = NULL;
	unsigned long flags;
	struct err_record_info *info = NULL;
	u32 part_num;
	int ret;

	drv = *(void **)drvdata;
	if (IS_ERR_OR_NULL(drv))
		return IRQ_NONE;
	spin_lock_irqsave(&g_handler_lock, flags);
	part_num = read_cpuid_part_number();
	pr_err("%s part_num is %x\n", __func__, part_num);
	ret = find_node_err(part_num, &info);

	if (ret != 0) {
		pr_err("%s No node have ecc error\n", __func__);
		spin_unlock_irqrestore(&g_handler_lock, flags);
		return IRQ_HANDLED;
	}
	parse_node_err(info);
	spin_unlock_irqrestore(&g_handler_lock, flags);

	return IRQ_HANDLED;
}

static int request_cpu_irq(struct platform_device *pdev, int cpu)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	const irq_handler_t handler = cpu_edac_handler;
	struct erp_cpu_drvdata *erp_cpu_drv = NULL;
	int *irq;
	int ret;
	int i;

	erp_cpu_drv = per_cpu_ptr(drv->cpu_drv, cpu);

	irq = erp_cpu_drv->irq;

	if (irq == NULL) {
		dev_err(dev, "invalid irq is NULL\n");
		return 0;
	}

	for (i = 0; i < erp_cpu_drv->irq_num; i++) {
		dev_err(dev, "irq num is %d of cpu is %d, irq_num is %d\n",
			irq[i], cpu, erp_cpu_drv->irq_num);
		ret = irq_force_affinity(irq[i], cpumask_of(cpu));
		if (ret != 0 && num_possible_cpus() > 1) {
			dev_err(dev, "unable to set irq affinity irq=%d, cpu=%d\n",
				irq[i], cpu);
			goto out;
		}

		ret = request_irq(irq[i], handler,
				  IRQF_NOBALANCING | IRQF_NO_THREAD | IRQF_NO_SUSPEND,
				  "l1-l2-faultirq-ecc",
				  drv);

		if (ret != 0) {
			dev_err(dev, "request l1_l2_irq failed irq=%d, cpu=%d\n",
				irq[i], cpu);
			goto out;
		}
	}
	cpumask_set_cpu(cpu, &drv->active_irqs);
	return 0;

out:
	return ret;
}

static void free_cpu_irqs(struct erp_drvdata *drv)
{
	int cpu;
	struct erp_cpu_drvdata *erp_cpu_drv = NULL;
	int *irq = per_cpu(drv->cpu_drv->irq, cpu);
	int j;

	for_each_cpu(cpu, &drv->supported_cpus) {
		erp_cpu_drv = per_cpu_ptr(drv->cpu_drv, cpu);
		if (cpumask_test_and_clear_cpu(cpu, &drv->active_irqs) == 0)
			continue;
		for (j = 0; j < drv->cpu_drv->irq_num; j++) {
			if (irq[j] == 0)
				continue;
			free_irq(irq[j], drv);
		}
	}
}

static int request_cpu_irqs(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	int cpu;
	int ret;

	ret = parse_cpu_irqs(pdev);
	if (ret != 0) {
		dev_err(dev, "parse cpu irq failed");
		return ret;
	}

	for_each_cpu(cpu, &drv->supported_cpus) {
		ret = request_cpu_irq(pdev, cpu);
		if (ret != 0) {
			dev_err(dev,
				"request l1_l2 irq failed, cpu = %d\n", cpu);
			goto out_cpu_irqs;
		}
	}
	return 0;

out_cpu_irqs:
	free_cpu_irqs(drv);
	return ret;
}

static int parse_per_l2(struct device_node *child,
			struct platform_device *pdev,
			struct erp_l2_drvdata *erp_l2_drv)
{
	struct property *prop = NULL;
	struct device_node *dn = NULL;
	int irq_cnts;
	int i, len;
	int cpu;

	irq_cnts = of_irq_count(child);
	if (irq_cnts <= 0) {
		dev_err(&pdev->dev, "l2_cluster not found irq\n");
		return -ENODEV;
	}

	erp_l2_drv->irq_num = irq_cnts;
	erp_l2_drv->irq = devm_kcalloc(&pdev->dev, irq_cnts,
				       sizeof(int), GFP_KERNEL);

	if (erp_l2_drv->irq == NULL)
		return -ENOMEM;

	for (i = 0; i < irq_cnts; i++)
		erp_l2_drv->irq[i] = of_irq_get(child, i);

	prop = of_find_property(child, "interrupt-affinity", &len);
	len = ((unsigned int)prop->length) / sizeof(u32);
	if (len <= 0)
		return -ENODEV;

	for (i = 0; i < len; i++) {
		dn = of_parse_phandle(child, "interrupt-affinity", i);
		if (dn == NULL)
			continue;

		cpu = of_cpu_node_to_id(dn);
		cpumask_set_cpu(cpu, &erp_l2_drv->l2_affinity_cpu);
	}

	return 0;
}

static int parse_l2_irqs(struct platform_device *pdev)
{
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	struct device_node *l2_node = NULL;
	struct device_node *child = NULL;
	struct erp_l2_drvdata *erp_l2_drv = NULL;
	int i, ret;
	int node_number;

	dev_err(&pdev->dev, "find l2 node\n");
	of_node_get(pdev->dev.of_node);

	l2_node = of_get_child_by_name(pdev->dev.of_node, "l2-ecc");

	if (l2_node == NULL) {
		dev_err(&pdev->dev, "failed to find l2_node\n");
		return -ENODEV;
	}

	drv->l2_drv = devm_kzalloc(&pdev->dev, sizeof(*erp_l2_drv), GFP_KERNEL);

	if (drv->l2_drv == NULL)
		return -ENOMEM;

	drv->l2_drv->l2_drv_next = NULL;

	/* get child node nubmer */
	node_number = of_get_child_count(l2_node);

	for (i = 0; i < node_number; i++) {
		erp_l2_drv = devm_kzalloc(&pdev->dev, sizeof(*erp_l2_drv), GFP_KERNEL);
		if (erp_l2_drv == NULL)
			return -ENOMEM;

		erp_l2_drv->l2_drv_next = drv->l2_drv->l2_drv_next;
		drv->l2_drv->l2_drv_next = erp_l2_drv;
	}

	erp_l2_drv = drv->l2_drv->l2_drv_next;

	/* get number data */
	for_each_child_of_node(l2_node, child) {
		ret = parse_per_l2(child, pdev, erp_l2_drv);
		if (ret != 0)
			return -ENODEV;
		erp_l2_drv = erp_l2_drv->l2_drv_next;
	}

	of_node_put(pdev->dev.of_node);

	return 0;
}

static void free_l2_irqs(struct erp_drvdata *drv)
{
	struct erp_l2_drvdata *erp_l2_drv = drv->l2_drv->l2_drv_next;
	int i;

	while (erp_l2_drv != NULL) {
		for (i = 0; i < erp_l2_drv->irq_num; i++)
			free_irq(erp_l2_drv->irq[i], erp_l2_drv);
		erp_l2_drv = erp_l2_drv->l2_drv_next;
	}
}

static irqreturn_t l2_handler(int irq, void *drvdata)
{
	struct erp_drvdata *drv = NULL;
	unsigned long flags;
	struct err_record_info *info = NULL;
	u32 part_num;
	int ret;

	drv = *(void **)drvdata;
	if (IS_ERR_OR_NULL(drv))
		return IRQ_NONE;

	spin_lock_irqsave(&g_handler_lock, flags);
	part_num = CPU_L2_CLUSTER;

	pr_err("%s part_num is %x\n", __func__, part_num);
	ret = find_node_err(part_num, &info);

	if (ret != 0) {
		pr_err("%s No node have ecc error\n", __func__);
		spin_unlock_irqrestore(&g_handler_lock, flags);
		return IRQ_HANDLED;
	}
	parse_node_err(info);
	spin_unlock_irqrestore(&g_handler_lock, flags);

	return IRQ_HANDLED;
}

static int request_l2_irqs(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	const irq_handler_t handler = l2_handler;
	struct erp_l2_drvdata *erp_l2_drv = NULL;
	int *irq = NULL;
	int ret;
	int i;

	ret = parse_l2_irqs(pdev);
	erp_l2_drv = drv->l2_drv;

	while (erp_l2_drv != NULL) {
		irq = erp_l2_drv->irq;
		for (i = 0; i < erp_l2_drv->irq_num; i++) {
			dev_err(dev, "irq num is %d, irq_num is %d\n",
				irq[i], erp_l2_drv->irq_num);
			ret = irq_force_affinity(irq[i], &erp_l2_drv->l2_affinity_cpu);

			ret = request_irq(irq[i], handler,
					  IRQF_NOBALANCING | IRQF_NO_THREAD | IRQF_NO_SUSPEND,
					  "l2-ecc", drv);
			if (ret != 0) {
				dev_err(dev, "request l2 irq failed\n");
				ret = -ENODEV;
				goto l2_free_irqs;
			}
		}
		erp_l2_drv = erp_l2_drv->l2_drv_next;
	}

	return 0;

l2_free_irqs:
	free_l2_irqs(drv);
	return ret;
}

static int parse_l3_irq(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	struct device_node *node = pdev->dev.of_node;
	struct device_node *l3_node = NULL;
	int irq;
	int ret;

	of_node_get(node);

	l3_node = of_get_child_by_name(node, "l3-ecc");
	if (l3_node == NULL) {
		dev_err(dev, "failed to find l3_node\n");
		ret = -EINVAL;
		goto err_out;
	}

	irq = of_irq_get(l3_node, 0);
	dev_info(dev, "l3 irq = %d\n", irq);

	if (irq <= 0) {
		dev_err(dev, "get invalid l3 irq = %d\n", irq);
		WARN_ON(1);
		ret = -EINVAL;
		goto err_out;
	}
	drv->l3_irq = irq;

	of_node_put(node);
	return 0;
err_out:
	of_node_put(node);
	return ret;
}

static irqreturn_t l3_edac_handler(int irq, void *drvdata)
{
	struct erp_drvdata *drv = NULL;
	unsigned long flags;
	struct err_record_info *info = NULL;
	u32 part_num;
	int ret;

	drv = *(void **)drvdata;
	if (IS_ERR_OR_NULL(drv))
		return IRQ_NONE;

	spin_lock_irqsave(&g_handler_lock, flags);
	part_num = CPU_L3_MEM;

	pr_err("%s part_num is %x\n", __func__, part_num);
	ret = find_node_err(part_num, &info);
	if (ret < 0) {
		pr_err("%s No node have ecc error\n", __func__);
		return IRQ_NONE;
	}

	parse_l3_node_err(info);
	spin_unlock_irqrestore(&g_handler_lock, flags);

	return IRQ_HANDLED;
}

static int request_l3_irq(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	int ret;

	ret = parse_l3_irq(pdev);
	if (ret != 0) {
		dev_err(dev, "parse l3 faultirq failed\n");
		return ret;
	}

	ret = devm_request_threaded_irq(dev, drv->l3_irq, NULL,
					l3_edac_handler,
					IRQF_TRIGGER_NONE | IRQF_ONESHOT,
					"l3-faultirq-ecc", drv);
	if (ret != 0)
		dev_err(dev, "request l3 faultirq failed\n");

	return ret;
}

static int hisi_cpu_erp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv = NULL;
	int rc;

	dev_err(dev, "entry\n");

	drv = devm_kzalloc(dev, sizeof(*drv), GFP_KERNEL);

	if (drv == NULL) {
		dev_err(dev, "fail to alloc mem for drvdata\n");
		return -ENOMEM;
	}

	drv->edev_ctl = edac_device_alloc_ctl_info(0, "cpu",
						   num_possible_cpus(),
						   "L", 3, 1, NULL, 0,
						   edac_device_alloc_index());

	if (drv->edev_ctl == NULL) {
		dev_err(dev, "fail to alloc ctl info\n");
		return -ENOMEM;
	}

	drv->edev_ctl->dev = dev;
	drv->edev_ctl->mod_name = dev_name(dev);
	drv->edev_ctl->dev_name = dev_name(dev);
	drv->edev_ctl->ctl_name = "cache";
	drv->edev_ctl->panic_on_ue = 0;
	platform_set_drvdata(pdev, drv);

	rc = edac_device_add_device(drv->edev_ctl);
	if (rc != 0) {
		dev_err(dev, "add edac device failed\n");
		goto out_mem;
	}

	rc  = cpuhp_setup_state_multi(CPUHP_AP_EDAC_ECC_STARTING,
				      "edac/ecc:starting",
				      edac_ecc_starting_cpu,
				      edac_ecc_stopping_cpu);
	if (rc != 0) {
		dev_err(dev, "CPU hotplug notifier failed: %d\n", rc);
		goto out_dev;
	}

	rc = request_cpu_irqs(pdev);
	if (rc != 0) {
		dev_err(dev, "request cpu irq failed\n");
		goto out_cpuhp;
	}
	rc = request_l2_irqs(pdev);
	if (rc != 0) {
		dev_err(dev, "request l2 cluster irq failed\n");
		goto out_cpu;
	}

	rc = request_l3_irq(pdev);
	if (rc != 0) {
		dev_err(dev, "request l3 irq failed\n");
		goto out_l2_irqs;
	}

	rc = cpuhp_state_add_instance(CPUHP_AP_EDAC_ECC_STARTING, &drv->node);

	if (rc != 0) {
		dev_err(dev, "cuphp add instance failed\n");
		goto out_l2_irqs;
	}

	dev_err(dev, "success\n");
	return 0;

out_l2_irqs:
	free_l2_irqs(drv);

out_cpu:
	free_cpu_irqs(drv);
	free_percpu(drv->cpu_drv);
out_cpuhp:
	cpuhp_remove_multi_state(CPUHP_AP_EDAC_ECC_STARTING);
out_dev:
	edac_device_del_device(dev);
out_mem:
	edac_device_free_ctl_info(drv->edev_ctl);
	return rc;
}

static int hisi_cpu_erp_remove(struct platform_device *pdev)
{
	struct erp_drvdata *drv = dev_get_drvdata(&pdev->dev);
	struct edac_device_ctl_info *edac_ctl = drv->edev_ctl;

	cpuhp_state_remove_instance(CPUHP_AP_EDAC_ECC_STARTING,
				    &drv->node);
	free_l2_irqs(drv);
	free_cpu_irqs(drv);
	free_percpu(drv->cpu_drv);
	cpuhp_remove_multi_state(CPUHP_AP_EDAC_ECC_STARTING);

	edac_device_del_device(edac_ctl->dev);
	edac_device_free_ctl_info(edac_ctl);

	return 0;
}

static const struct of_device_id hisi_cpu_erp_match_table[] = {
	{ .compatible = "hisilicon,cpu-edac-ecc" },
	{ }
};

static struct platform_driver hisi_cpu_erp_driver = {
	.probe = hisi_cpu_erp_probe,
	.remove = hisi_cpu_erp_remove,
	.driver = {
		.name = "hisi_cpu_cache_erp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_cpu_erp_match_table),
	},
};

static int __init hisi_cpu_erp_init(void)
{
	return platform_driver_register(&hisi_cpu_erp_driver);
}
late_initcall(hisi_cpu_erp_init);

static void __exit hisi_cpu_erp_exit(void)
{
	platform_driver_unregister(&hisi_cpu_erp_driver);
}
module_exit(hisi_cpu_erp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("RAS	EDAC driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
