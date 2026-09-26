/*
 * hardware spin lock module
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/hwspinlock.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <pr_log.h>
#include "hwspinlock_internal.h"

#define MODULE_NAME "hwspinlock"
#define PR_LOG_TAG HWSPINLOCK_TAG

/*
 * CPU ID for master running our kernel.
 * hwspinlocks should only be used to synchonise operations
 * between the Cortex A_x core and the other CPUs.  Hence
 * forcing the masterID to a preset value
 */
#define MASTER_ID 0x01
#define LOCK_CMD ((MASTER_ID << 1) | 0x01)
#define ID_MASK 0x0f
#define UNLOCK_OFFSET 0x04
#define LOCK_ST_OFFSET 0x08
#define LOCK_ADDRESS_INT 0x400
#define SPINLOCK_RELAX_LEN 50
#define SPIN_LIST_INIT 0
#define HWSPINLOCK_GPIO_DEBUG 0x1
#define HWSPINLOCK_GPIO_MASK_ADDR 0x400
static unsigned int g_peri_base;
unsigned int g_gpio_hwspin_status;

struct native_hwspinlock {
	int id_in_group;
	void __iomem *address;
	int sec_flag;
};

static int lock_bits_g;

unsigned int get_gpio_hwspinlock_status(void)
{
	return g_gpio_hwspin_status;
}

static int hwspinlock_trylock(struct hwspinlock *lock)
{
	struct native_hwspinlock *lock_temp =
		(struct native_hwspinlock *)(lock->priv);
	void __iomem *lock_addr = lock_temp->address;
	int num_in_group = lock_temp->id_in_group;
	unsigned int status;
	unsigned int mask_addr;

	if (lock_temp->sec_flag)
		return 0;

	writel(LOCK_CMD << (unsigned int)(num_in_group * lock_bits_g),
		lock_addr);
	status = (unsigned int)readl(lock_addr + LOCK_ST_OFFSET) >>
		(unsigned int)(num_in_group * lock_bits_g);
	/*
	 * get only fourth (lock_bits_g) bits and compare to masterID
	 * if equal, we have the lock, otherwise
	 * someone else has it
	 */
	if (!((ID_MASK & status) == LOCK_CMD)) {
		mask_addr = (unsigned int)((uintptr_t)lock_addr) - g_peri_base;
		if ((num_in_group == HWSPINLOCK_GPIO_DEBUG) &&
			(mask_addr == HWSPINLOCK_GPIO_MASK_ADDR))
			g_gpio_hwspin_status = (ID_MASK & status);
	}
	return ((ID_MASK & status) == LOCK_CMD);
}

static void hwspinlock_unlock(struct hwspinlock *lock)
{
	struct native_hwspinlock *lock_temp =
		(struct native_hwspinlock *)(lock->priv);
	void __iomem *lock_addr = lock_temp->address;
	int num_in_group = lock_temp->id_in_group;

	/* release the lock by writing LOCK_CMD to it */
	if (lock_temp->sec_flag)
		return;
	writel(LOCK_CMD << (unsigned int)(num_in_group * lock_bits_g),
		lock_addr + UNLOCK_OFFSET);
}

/* what value is recommended here */
static void hwspinlock_relax(struct hwspinlock *lock)
{
	ndelay(SPINLOCK_RELAX_LEN);
}

static const struct hwspinlock_ops spinlock_ops = {
	.trylock = hwspinlock_trylock,
	.unlock = hwspinlock_unlock,
	.relax = hwspinlock_relax,
};

static const struct of_device_id hwlock_of_match[] = {
	{ .compatible = "hisilicon,hwspinlock" },
	{ },
};

static int hwspinlock_ao_base_init(struct device *dev,
	void __iomem **lock_ao_base)
{
	int ret = 0;
	struct device_node *np_sysctrl = NULL;

	np_sysctrl = of_find_compatible_node(NULL, NULL,
		"hisilicon,sysctrl");
	if (!np_sysctrl) {
		dev_err(dev, "no find sysctrl node!\n");
		return -EINVAL;
	}

	*lock_ao_base = of_iomap(np_sysctrl, 0);
	if (!(*lock_ao_base)) {
		dev_err(dev, "failed to iomap sysctrl!\n");
		ret = -ENOMEM;
	}

	of_node_put(np_sysctrl);
	return ret;
}

static int get_ao_lock_prop(struct device_node *np,
	unsigned int *addr, int group_num, struct device *dev,
	unsigned int *sec_flag)
{
	int ret;

	ret = of_property_read_u32_array(np, "hwlock,ao-offset",
		addr, group_num);
	if (ret) {
		dev_err(dev, "no find 'hwlock,ao-offset' property!\n");
		return ret;
	}

	ret = of_property_read_u32_array(np, "secure,ao-flag",
		sec_flag, group_num);
	if (ret) {
		dev_err(dev, "no find 'secure,ao-flag' property!\n");
		return ret;
	}
	return ret;
}

static int ao_hwspinlock_init(struct device *dev,
	struct hwspinlock *hwlock, int group_num, int locks_per_reg,
	struct device_node *np)
{
	struct native_hwspinlock *lock_info = NULL;
	unsigned int *addr = NULL;
	unsigned int *sec_flag = NULL;
	void __iomem *lock_ao_base = NULL;
	int locks = locks_per_reg * group_num;
	int groups = group_num * sizeof(unsigned int);
	int ret, i;

	ret = hwspinlock_ao_base_init(dev, &lock_ao_base);
	if (ret)
		return ret;

	lock_info = devm_kzalloc(dev, locks * sizeof(*lock_info), GFP_KERNEL);
	if (!lock_info) {
		ret = -ENOMEM;
		goto iounmap_base;
	}

	addr = devm_kzalloc(dev, groups, GFP_KERNEL);
	if (!addr) {
		ret = -ENOMEM;
		goto iounmap_base;
	}

	sec_flag = devm_kzalloc(dev, groups, GFP_KERNEL);
	if (!sec_flag) {
		ret = -ENOMEM;
		goto iounmap_base;
	}

	if (get_ao_lock_prop(np, addr, group_num, dev, sec_flag)) {
		ret = -ENOENT;
		goto iounmap_base;
	}

	if (!locks_per_reg) {
		dev_err(dev, "parameter locks_per_reg error!\n");
		goto iounmap_base;
	}

	for (i = 0; i < locks; i++, hwlock++, lock_info++) {
		lock_info->id_in_group = i % locks_per_reg;
		lock_info->address = lock_ao_base + addr[i / locks_per_reg];
		lock_info->sec_flag = (sec_flag[i / locks_per_reg] != 0);
		hwlock->priv = lock_info;
	}

	/* Addr and sec flag is no used */
	devm_kfree(dev, sec_flag);
	devm_kfree(dev, addr);

	return 0;

iounmap_base:
	iounmap(lock_ao_base);
	return ret;
}

static int get_hwspinlock_base_addr(struct device_node **np_lock)
{
	*np_lock = of_find_compatible_node(NULL, NULL,
		"hisilicon,peri_resource_lock");
	if (*np_lock)
		return 0;

	*np_lock = of_find_compatible_node(NULL, NULL,
		"hisilicon,pctrl");
	if (!*np_lock)
		return -EINVAL;
	return 0;
}
static int get_hwspinlock_base(struct device *dev,
	void __iomem **hwspinlock_base)
{
	int ret = 0;
	struct device_node *np_pctrl = NULL;

	ret = get_hwspinlock_base_addr(&np_pctrl);
	if (ret) {
		dev_err(dev, "no find pctrl node!\n");
		return -EINVAL;
	}

	*hwspinlock_base = of_iomap(np_pctrl, 0);
	if (!(*hwspinlock_base)) {
		dev_err(dev, "failed to iomap!\n");
		ret = -ENOMEM;
	}

	of_node_put(np_pctrl);
	return ret;
}

static int get_info_from_fdt(struct device_node *np, struct device *dev,
	int *register_width, int *lock_groups)
{
	int ret;

	ret = of_property_read_u32(np, "hwlock,register-width",
		(u32 *)register_width);
	if (ret) {
		dev_err(dev, "no find 'hwlock,register-width' property!\n");
		return ret;
	}

	ret = of_property_read_u32(np, "hwlock,bits-per-single",
		(u32 *)&lock_bits_g);
	if (ret) {
		dev_err(dev, "no find 'hwlock,bits-per-single' property!\n");
		return ret;
	}

	ret = of_property_read_u32(np, "hwlock,groups", (u32 *)lock_groups);
	if (ret) {
		dev_err(dev, "no find 'hwlock,groups' property!\n");
		return ret;
	}

	return ret;
}

static int hwspin_peri_init(struct platform_device *pdev,
	struct hwspinlock_device *bank, int lock_groups,
	int locks_per_register, void __iomem **hwspinlock_base)
{
	int i, ret;
	struct hwspinlock *hwlock = NULL;
	struct native_hwspinlock *hwspinlock_info = NULL;
	unsigned int *addr = NULL;
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	int pctrl_num_lock;

	pctrl_num_lock = locks_per_register * lock_groups;
	ret = get_hwspinlock_base(dev, hwspinlock_base);
	if (ret)
		return ret;

	addr = devm_kzalloc(dev, lock_groups * sizeof(unsigned int),
		GFP_KERNEL);
	if (!addr) {
		ret = -ENOMEM;
		goto err;
	}

	hwspinlock_info = devm_kzalloc(dev, pctrl_num_lock *
		sizeof(*hwspinlock_info), GFP_KERNEL);
	if (!hwspinlock_info) {
		ret = -ENOMEM;
		goto err;
	}

	ret = of_property_read_u32_array(
		np, "hwlock,offset", addr, lock_groups);
	if (ret) {
		dev_err(dev, "no find 'hwlock,offset' property!\n");
		goto err;
	}

	if (!locks_per_register)
		goto err;

	for (i = 0, hwlock = &bank->lock[SPIN_LIST_INIT]; i < pctrl_num_lock;
		i++, hwlock++, hwspinlock_info++) {
		hwspinlock_info->id_in_group = i % locks_per_register;
		hwspinlock_info->address = (*hwspinlock_base) +
			addr[i / locks_per_register];
		hwspinlock_info->sec_flag = 0;
		hwlock->priv = hwspinlock_info;
	}

	/* Addr is no used */
	devm_kfree(dev, addr);
	return ret;

err:
	iounmap(*hwspinlock_base);
	return ret;
}

static void hwspinlock_is_ao_support(struct device_node *np,
	struct device *dev, int *p_ao_lock_groups, bool *p_lock_ao_support)
{
	int ret;

	ret = of_property_read_u32(np, "hwlock,ao-groups",
		(u32 *)p_ao_lock_groups);
	if (ret) {
		dev_err(dev, "no find 'hwlock,ao-groups' property!\n");
		*p_ao_lock_groups = 0;
		*p_lock_ao_support = false;
	} else {
		dev_err(dev, "ao hwspinlock is supported!\n");
		*p_lock_ao_support = true;
	}
}

static int vendor_hwspinlock_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct hwspinlock_device *bank = NULL;
	struct hwspinlock *hwlock = NULL;
	int ret, num_lock, register_width, lock_groups, locks_per_register;
	int ao_lock_groups;
	void __iomem *hwspinlock_base = NULL;
	bool lock_ao_support = false;

	ret = get_info_from_fdt(np, dev, &register_width, &lock_groups);
	if (ret)
		return ret;

	hwspinlock_is_ao_support(np, dev, &ao_lock_groups, &lock_ao_support);

	if (!lock_bits_g)
		return -1;
	locks_per_register = register_width / lock_bits_g;
	num_lock = locks_per_register * (ao_lock_groups + lock_groups);

	bank = devm_kzalloc(
		dev, sizeof(*bank) + num_lock * sizeof(*hwlock), GFP_KERNEL);
	if (!bank)
		return -ENOMEM;

	ret = hwspin_peri_init(pdev, bank, lock_groups,
		locks_per_register, &hwspinlock_base);
	if (ret)
		return ret;

	g_peri_base = (unsigned int)(uintptr_t)hwspinlock_base;

	if (lock_ao_support) {
		hwlock = &bank->lock[locks_per_register * lock_groups];
		ret = ao_hwspinlock_init(dev, hwlock, ao_lock_groups,
			locks_per_register, np);
		if (ret) {
			dev_err(dev, "Hwspinlock in ao init fail!\n");
			goto iounmap_base_ao;
		}
	}

	platform_set_drvdata(pdev, bank);
	/*
	 * runtime PM will make sure the clock of this module is
	 * enabled iff at least one lock is requested
	 */
	pm_runtime_enable(dev);

	ret = hwspin_lock_register(
		bank, dev, &spinlock_ops, 0, num_lock);
	if (ret) {
		pm_runtime_disable(dev);
		goto iounmap_base_ao;
	}

	dev_info(dev, "init ok\n");
	return 0;

iounmap_base_ao:
	iounmap(hwspinlock_base);
	return ret;
}

static int vendor_hwspinlock_remove(struct platform_device *pdev)
{
	struct hwspinlock_device *bank = NULL;
	struct native_hwspinlock *lock_temp = NULL;
	void __iomem *hwspinlock_base = NULL;
	int ret;

	if (!pdev)
		return -EFAULT;

	bank = platform_get_drvdata(pdev);
	if (!bank)
		return -EFAULT;

	lock_temp =
		(struct native_hwspinlock *)(bank->lock[SPIN_LIST_INIT].priv);
	hwspinlock_base = lock_temp->address - LOCK_ADDRESS_INT;

	ret = hwspin_lock_unregister(bank);
	if (ret) {
		dev_err(&pdev->dev, "hwspinlcoks failed to unregister!\n");
		return ret;
	}

	pm_runtime_disable(&pdev->dev);
	iounmap(hwspinlock_base);

	return 0;
}

MODULE_DEVICE_TABLE(of, hwlock_of_match);

static struct platform_driver vendor_hwspinlock_driver = {
	.probe = vendor_hwspinlock_probe,
	.remove = vendor_hwspinlock_remove,
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hwlock_of_match),
	},
};

static int __init vendor_hwspinlock_init(void)
{
	return platform_driver_register(&vendor_hwspinlock_driver);
}
/* board init code might need to reserve hwspinlocks for predefined purposes */
arch_initcall(vendor_hwspinlock_init);

static void __exit vendor_hwspinlock_exit(void)
{
	platform_driver_unregister(&vendor_hwspinlock_driver);
}
module_exit(vendor_hwspinlock_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hardware Spinlock driver");
