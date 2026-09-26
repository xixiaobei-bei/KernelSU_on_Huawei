/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_i2c.c
 *
 * pmu i2c driver
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, pmicstributed, and mopmicfied under those terms.
 *
 * This program is pmicstributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include<linux/of_gpio.h>
#include <linux/regmap.h>
#include <linux/irq.h>

#define MASK 0
#define UNMASK 1
#define PMIC_BITS 8
#define PMIC_MASK_FIELD 0xff
#define PMIC_MASK_STATE 0xff
#ifndef NO_IRQ
#define NO_IRQ 0
#endif
#define PMIC_IRQ_NAME_SIZE 20
#define PMIC_CHIP_IRQ_NAME_SIZE 20
#define ONE_IRQ_GROUP_NUM 8
#define REGMAP_SIZE 6

struct pmic_i2c_priv {
	struct i2c_client *client;
	struct device *dev;
	struct regmap *regmap;
	struct mutex irq_lock;
	struct irq_domain *domain;
	int gpio;
	int irq;
	int irqnum;
	int irqarray;
	int irq_ops;
	unsigned int *irqs;
	int *irq_mask_addr_arry;
	int *irq_addr_arry;
	char *irq_name;
	char *chip_irq_name;
	struct delayed_work irq_work;
	struct irq_chip irq_chip;
};

static const struct regmap_config pmic_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0xff,
};

static void _pmic_irq_mask(struct irq_data *d, int maskflag)
{
	struct pmic_i2c_priv *pmic = irq_data_get_irq_chip_data(d);
	u32 data = 0;
	u32 offset;
	int ret;

	if (!pmic) {
		pr_err(" irq_mask pmic is NULL\n");
		return;
	}

	/* Convert interrupt  data to interrupt offset */
	offset = (irqd_to_hwirq(d) / ONE_IRQ_GROUP_NUM);
	offset = pmic->irq_mask_addr_arry[offset];
	ret = regmap_read(pmic->regmap, offset, &data);
	if (ret < 0) {
		pr_err("%s regmap read interrupt offset 0x%x failed\n",
			__func__, offset);
		return;
	}
	if (maskflag == MASK)
		data |= (1 << (irqd_to_hwirq(d) & 0x07));
	else
		data &= ~(u32)(1 << (irqd_to_hwirq(d) & 0x07));

	ret = regmap_write(pmic->regmap, offset, data);
	if (ret < 0)
		pr_err("%s regmap write interrupt offset 0x%x failed\n",
			__func__, offset);
}

static void pmic_i2c_irq_bus_unlock(struct irq_data *d)
{
	struct pmic_i2c_priv *pmic = irq_data_get_irq_chip_data(d);

	if (!pmic) {
		pr_err(" %s pmic is NULL\n", __func__);
		return;
	}

	_pmic_irq_mask(d, pmic->irq_ops);

	mutex_unlock(&pmic->irq_lock);
}

static void pmic_i2c_irq_bus_lock(struct irq_data *d)
{
	struct pmic_i2c_priv *pmic = irq_data_get_irq_chip_data(d);

	if (!pmic) {
		pr_err(" %s pmic is NULL\n", __func__);
		return;
	}

	mutex_lock(&pmic->irq_lock);
}

static void pmic_irq_mask(struct irq_data *d)
{
	struct pmic_i2c_priv *pmic = irq_data_get_irq_chip_data(d);

	pmic->irq_ops = MASK;
}

static void pmic_irq_unmask(struct irq_data *d)
{
	struct pmic_i2c_priv *pmic = irq_data_get_irq_chip_data(d);

	pmic->irq_ops = UNMASK;
}

static struct irq_chip pmu_i2c_irqchip = {
	.name = "pmu-i2c-irq",
	.irq_mask = pmic_irq_mask,
	.irq_unmask = pmic_irq_unmask,
	.irq_disable = pmic_irq_mask,
	.irq_enable = pmic_irq_unmask,
	.irq_bus_lock = pmic_i2c_irq_bus_lock,
	.irq_bus_sync_unlock = pmic_i2c_irq_bus_unlock,
};

static int pmic_irq_map(
	struct irq_domain *d, unsigned int virq, irq_hw_number_t hw)
{
	struct pmic_i2c_priv *pmic = d->host_data;
	int ret;

	irq_set_chip_data(virq, pmic);
	irq_set_chip_and_handler_name(
		virq, &pmu_i2c_irqchip, handle_simple_irq, "pmic");
	ret = irq_set_irq_type(virq, IRQ_TYPE_NONE);
	if (ret < 0)
		pr_err("irq set type fail\n");

	return 0;
}

static const struct irq_domain_ops pmic_domain_ops = {
	.map = pmic_irq_map,
	.xlate = irq_domain_xlate_twocell,
};

static irqreturn_t pmic_i2c_irq_handler(int irq, void *data)
{
	struct pmic_i2c_priv *pmic = (struct pmic_i2c_priv *)data;

	disable_irq_nosync(pmic->irq);
	schedule_delayed_work(&pmic->irq_work, 0);

	return IRQ_HANDLED;
}

static void pmic_i2c_interrupt_work(struct work_struct *work)
{
	struct pmic_i2c_priv *pmic = container_of(work,
					   struct pmic_i2c_priv, irq_work.work);
	unsigned int pending = 0;
	unsigned long pending_s;
	int i, offset;
	int ret;

	dev_err(pmic->dev, "%s +\n", __func__);

	for (i = 0; i < pmic->irqarray; i++) {
		ret = regmap_read(pmic->regmap, pmic->irq_addr_arry[i],
			&pending);
		if (ret < 0) {
			pr_err("%s regmap read 0x%x failed\n", __func__,
				pmic->irq_addr_arry[i]);
			return;
		}
		pending &= PMIC_MASK_FIELD;
		if (pending != 0)
			dev_err(pmic->dev, "pending[%d] 0x%x=0x%x\n\r", i,
					pmic->irq_addr_arry[i], pending);

		ret = regmap_write(pmic->regmap, i + pmic->irq_addr_arry[i],
			pending);
		if (ret < 0) {
			pr_err("%s regmap write 0x%x failed\n", __func__,
				i + pmic->irq_addr_arry[i]);
			return;
		}
		pending_s = (unsigned long)pending;
		if (pending_s) {
			for_each_set_bit(offset, &pending_s, PMIC_BITS)
				generic_handle_irq(
					pmic->irqs[offset + i * PMIC_BITS]);
		}
	}

	enable_irq(pmic->irq);

	dev_err(pmic->dev, "%s -\n", __func__);
}

static int pmic_i2c_irq_create_mapping(struct pmic_i2c_priv *pmic)
{
	int i;
	unsigned int virq;

	for (i = 0; i < pmic->irqnum; i++) {
		virq = irq_create_mapping(pmic->domain, i);
		if (virq == NO_IRQ) {
			pr_err("Failed mapping hwirq\n");
			return -ENOSPC;
		}
		pmic->irqs[i] = virq;
		dev_info(pmic->dev, "[%s] pmic->irqs[%d] = %d\n", __func__, i,
			pmic->irqs[i]);
	}
	return 0;
}

static void pmic_i2c_irq_prc(struct pmic_i2c_priv *pmic)
{
	int i;
	unsigned int pending;
	int ret;

	for (i = 0; i < pmic->irqarray; i++) {
		ret = regmap_write(pmic->regmap, pmic->irq_mask_addr_arry[i],
			PMIC_MASK_STATE);
		if (ret < 0) {
			pr_err("%s regmap write 0x%x failed\n", __func__,
				pmic->irq_mask_addr_arry[i]);
			return;
		}
	}

	for (i = 0; i < pmic->irqarray; i++) {
		ret = regmap_read(pmic->regmap, pmic->irq_addr_arry[i], &pending);
		if (ret < 0) {
			pr_err("%s regmap read 0x%x failed\n", __func__,
				pmic->irq_addr_arry[i]);
			return;
		}

		pr_debug("PMU IRQ address value:irq[0x%x] = 0x%x\n",
			pmic->irq_addr_arry[i], pending);
		ret = regmap_write(
			pmic->regmap, pmic->irq_addr_arry[i], PMIC_MASK_STATE);
		if (ret < 0)
			pr_err("%s regmap write 0x%x failed\n", __func__,
				pmic->irq_addr_arry[i]);
	}
}

static int pmic_i2c_irq_register(
	struct i2c_client *client, struct pmic_i2c_priv *pmic, const char *name)
{
	int ret;
	enum of_gpio_flags flags;
	struct device *dev = &client->dev;
	struct device_node *np = dev->of_node;

	if (!client || !pmic || !name)
		return -EINVAL;

	pmic->gpio = of_get_gpio_flags(np, 0, &flags);
	if (pmic->gpio < 0) {
		dev_err(dev, "failed to get_gpio_flags:%d\n", pmic->gpio);
		return pmic->gpio;
	}

	if (!gpio_is_valid(pmic->gpio))
		return -EINVAL;

	ret = gpio_request_one(pmic->gpio, GPIOF_IN, name);
	if (ret < 0)
		dev_err(dev, "failed to request gpio%d, %d, %s\n",
			pmic->gpio, ret, name);

	pmic->irq = gpio_to_irq(pmic->gpio);
	dev_info(dev, "%s gpio %d, irq %d\n", __func__, pmic->gpio, pmic->irq);
	/* mask && clear IRQ status */
	pmic_i2c_irq_prc(pmic);

	pmic->irqs = (unsigned int *)devm_kmalloc(
		dev, pmic->irqnum * sizeof(int), GFP_KERNEL);
	if (!pmic->irqs)
		return -ENODEV;

	/* Dynamic obtain struct irq_chip */
	pmic->irq_chip.name = pmic->chip_irq_name;
	pmic->irq_chip.irq_mask = pmic_irq_mask;
	pmic->irq_chip.irq_unmask = pmic_irq_unmask;
	pmic->irq_chip.irq_disable = pmic_irq_mask;
	pmic->irq_chip.irq_enable = pmic_irq_unmask;
	pmic->irq_chip.irq_bus_lock = pmic_i2c_irq_bus_lock;
	pmic->irq_chip.irq_bus_sync_unlock = pmic_i2c_irq_bus_unlock;

	pmic->domain = irq_domain_add_simple(
		np, pmic->irqnum, 0, &pmic_domain_ops, pmic);
	if (!pmic->domain) {
		dev_err(dev, "failed irq domain add simple!\n");
		return -ENODEV;
	}

	ret = pmic_i2c_irq_create_mapping(pmic);
	if (ret < 0)
		return ret;

	ret = request_threaded_irq(pmic->irq, pmic_i2c_irq_handler, NULL,
		IRQF_TRIGGER_LOW | IRQF_NO_SUSPEND, name, pmic);
	if (ret < 0) {
		dev_err(dev, "request irq fail %d\n", ret);
		return -ENODEV;
	}

	return 0;
}

static int get_pmic_i2c_irq_data(
	struct i2c_client *client, struct pmic_i2c_priv *pmic)
{
	int ret;
	struct device *dev = &client->dev;
	struct device_node *np = dev->of_node;

	if (client == NULL || pmic == NULL)
		return -ENOMEM;

	/* get pmic irq num */
	ret = of_property_read_u32_array(
		np, "hisilicon,pmic-irq-num", &(pmic->irqnum), 1);
	if (ret) {
		pr_err("no hisilicon,pmic-irq-num property set\n");
		return -ENODEV;
	}

	/* get pmic irq array number */
	ret = of_property_read_u32_array(
		np, "hisilicon,pmic-irq-array", &(pmic->irqarray), 1);
	if (ret) {
		pr_err("no hisilicon,pmic-irq-array property set\n");
		return -ENODEV;
	}

	pmic->irq_mask_addr_arry = (int *)devm_kzalloc(dev,
		sizeof(int) * pmic->irqarray, GFP_KERNEL);
	if (!pmic->irq_mask_addr_arry)
		return -ENOMEM;

	ret = of_property_read_u32_array(np,
		"hisilicon,pmic-irq-mask-addr",
		(int *)pmic->irq_mask_addr_arry, pmic->irqarray);
	if (ret) {
		pr_err("no hisilicon,pmic-irq-mask-addr property set\n");
		return -ENODEV;
	}

	pmic->irq_addr_arry = (int *)devm_kzalloc(dev,
		sizeof(int) * pmic->irqarray, GFP_KERNEL);
	if (!pmic->irq_mask_addr_arry)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "hisilicon,pmic-irq-addr",
		(int *)pmic->irq_addr_arry, pmic->irqarray);
	if (ret) {
		pr_err("no hisilicon,pmic-irq-addr property set\n");
		return -ENODEV;
	}
	return 0;
}

static int pmic_i2c_get_chip_and_irq_name(
	struct i2c_client *client, struct pmic_i2c_priv *pmic)
{
	int ret;
	struct device *dev = &client->dev;
	struct device_node *np = dev->of_node;

	if (client == NULL || pmic == NULL)
		return -ENOMEM;

	pmic->irq_name = devm_kzalloc(dev, PMIC_IRQ_NAME_SIZE, GFP_KERNEL);
	if (!pmic->irq_name)
		return -ENOMEM;

	ret = of_property_read_string(np, "hisilicon,hisi-pmic-irq-name",
		(const char **)&pmic->irq_name);
	if (ret < 0) {
		pr_err("no hisilicon,hisi-pmic-irq-name\n");
		return -ENODEV;
	}

	pmic->chip_irq_name = devm_kzalloc(dev, PMIC_CHIP_IRQ_NAME_SIZE,
		GFP_KERNEL);
	if (!pmic->chip_irq_name)
		return -ENOMEM;

	ret = of_property_read_string(np, "hisilicon,hisi-pmic-chip-irq-name",
		(const char **)&pmic->chip_irq_name);
	if (ret < 0) {
		pr_err("no hisilicon,hisi-pmic-chip-irq-name\n");
		return -ENODEV;
	}
	return 0;
}

#ifdef CONFIG_HISI_DEBUG_FS
static struct pmic_i2c_priv *g_pmic;

int hisi_pmic_test_read(unsigned int reg)
{
	struct pmic_i2c_priv *pmic = g_pmic;
	unsigned int val = 0;
	int ret;

	if (!pmic) {
		pr_err("%s pmic is null\n", __func__);
		return -1;
	}

	ret = regmap_read(pmic->regmap, reg, &val);
	if (ret < 0) {
		pr_err("%s reg 0x%x failed\n", __func__, reg);
		return ret;
	}
	pr_err("%s reg 0x%x = 0x%x\n", __func__, reg, val);
	return val;
}

int hisi_pmic_test_write(unsigned int reg, unsigned int val)
{
	struct pmic_i2c_priv *pmic = g_pmic;
	int ret;

	if (!pmic) {
		pr_err("%s pmic is null\n", __func__);
		return -1;
	}

	pr_err("%s reg 0x%x = 0x%x\n", __func__, reg, val);
	ret = regmap_write(pmic->regmap, reg, val);
	if (ret < 0) {
		pr_err("%s reg 0x%x failed\n", __func__, reg);
		return ret;
	}
	return hisi_pmic_test_read(reg);
}
#endif

static int pmic_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct pmic_i2c_priv *pmic = NULL;
	struct device_node *np = NULL;

	pmic = devm_kzalloc(&client->dev, sizeof(*pmic), GFP_KERNEL);
	if (!pmic) {
		dev_err(&client->dev, "[%s]pmic is null\n", __func__);
		return -ENOMEM;
	}
	pmic->dev = &client->dev;

	np = pmic->dev->of_node;
	pmic->client = client;
	pmic->regmap = devm_regmap_init_i2c(client, &pmic_regmap_config);
	if (IS_ERR(pmic->regmap))
		return PTR_ERR(pmic->regmap);

	mutex_init(&pmic->irq_lock);
	INIT_DELAYED_WORK(&pmic->irq_work, pmic_i2c_interrupt_work);

	ret = get_pmic_i2c_irq_data(client, pmic);
	if (ret)
		return ret;

	ret = pmic_i2c_get_chip_and_irq_name(client, pmic);
	if (ret) {
		dev_err(pmic->dev, "[%s] get irq name fail\n", __func__);
		return ret;
	}

	ret = pmic_i2c_irq_register(client, pmic, pmic->irq_name);
	if (ret) {
		dev_err(pmic->dev, "[%s] irq register fail ret %d\n",
			__func__, ret);
		goto fail;
	}

	i2c_set_clientdata(client, pmic);

	ret = of_platform_populate(np, NULL, NULL, pmic->dev);
	if (ret) {
		dev_info(pmic->dev, "%s populate fail %d\n", __func__, ret);
		goto fail;
	} else {
#ifdef CONFIG_HISI_DEBUG_FS
		g_pmic = pmic;
#endif
		dev_info(pmic->dev, "%s success\n", __func__);
	}
	return ret;
fail:
	gpio_free(pmic->gpio);
	mutex_destroy(&pmic->irq_lock);
	return ret;
}

static int pmic_i2c_remove(struct i2c_client *client)
{
	struct pmic_i2c_priv *pmic = i2c_get_clientdata(client);

	if (!pmic)
		return -1;

	free_irq(pmic->irq, pmic);
	gpio_free(pmic->gpio);
	mutex_destroy(&pmic->irq_lock);
	devm_kfree(&client->dev, pmic);
	return 0;
}

MODULE_DEVICE_TABLE(i2c, pmic_i2c_drv);
const static struct of_device_id pmic_i2c_of_match[] = {
	{
		.compatible = "hisilicon,hisi-pmic-i2c",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id pmic_i2c_id[] = {
	{"pmic_i2c_drv", 0},
	{},
};

static struct i2c_driver pmic_i2c_driver = {
	.probe = pmic_i2c_probe,
	.remove = pmic_i2c_remove,
	.id_table = pmic_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "pmic_i2c_drv",
		.of_match_table = of_match_ptr(pmic_i2c_of_match),
	},
};

static int __init pmic_i2c_init(void)
{
	int ret;

	ret = i2c_add_driver(&pmic_i2c_driver);
	if (ret)
		pr_err("%s: i2c_add_driver error!!!\n", __func__);

	return ret;
}

static void __exit pmic_i2c_exit(void)
{
	i2c_del_driver(&pmic_i2c_driver);
}

subsys_initcall_sync(pmic_i2c_init);
module_exit(pmic_i2c_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("PMIC i2c driver");