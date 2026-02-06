/**
 * @file cmdlist_drv.c
 * @brief Cmdlist device driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "cmdlist_drv.h"

#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/genalloc.h>

#include  "cmdlist_dev.h"

bool cmdlist_enable = true;
MODULE_PARM_DESC(cmdlist_enable, "cmdlist module config enable");
module_param(cmdlist_enable, bool, 0600);

struct cmdlist_private g_cmdlist_priv;

static int cmdlist_of_device_setup(struct platform_device *pdev)
{
	int ret = 0;
	dma_addr_t dma_handle;
	struct cmdlist_private *priv = &g_cmdlist_priv;

	priv->of_dev = &pdev->dev;

	of_property_read_u32(priv->of_dev->of_node, "scene_num", &priv->scene_num);
	pr_info("get scene_num: %d\n", priv->scene_num);

	of_property_read_u32(priv->of_dev->of_node, "cmdlist_max_size", (uint32_t *)&priv->sum_pool_size);
	pr_info("get cmdlist_max_size: %d\n", priv->sum_pool_size);

	ret = dma_set_mask_and_coherent(priv->of_dev, DMA_BIT_MASK(64));
	if (ret != 0) {
		pr_err("dma set mask and coherent failed %d!\n",  ret);
		return -1;
	}

	/* One cmdlist node is inlcude cmd_header(size: 16Byte) and cmd_item[0..N](size: 16Byte)
	 * N: tatal_items[13:0] = 0x3FFF = 16383
	 */
	priv->sum_pool_size = max(roundup(ITEMS_MAX_NUM * ONE_ITEM_SIZE, PAGE_SIZE) * priv->scene_num, priv->sum_pool_size);
	priv->pool_vir_addr = dma_alloc_coherent(priv->of_dev, priv->sum_pool_size, &dma_handle, GFP_KERNEL);
	if (!priv->pool_vir_addr) {
		pr_err("dma alloc 0x%zx Byte coherent failed %d!\n", priv->sum_pool_size, PTR_ERR(priv->pool_vir_addr));
		return -1;
	}
	priv->pool_phy_addr = dma_handle;
	memset(priv->pool_vir_addr, 0, priv->sum_pool_size);

	/**
	 * devm_gen_pool_create: The pool will be automatically destroyed by the device management code
	 */
	priv->memory_pool = devm_gen_pool_create(priv->of_dev, 4/* order */, -1, DEV_NAME_CMDLIST);
	if (IS_ERR_OR_NULL(priv->memory_pool)) {
		pr_err("create memory pool failed %d!\n", PTR_ERR(priv->memory_pool));
		goto err_pool_create;
	}

	ret = gen_pool_add_virt(priv->memory_pool, (unsigned long)(uintptr_t)priv->pool_vir_addr, priv->pool_phy_addr, priv->sum_pool_size, -1);
	if (ret != 0) {
		pr_err("memory pool add failed %d!\n",  ret);
		goto err_pool_add;
	}
	pr_info("alloc pool[%p].size[0x%x] vir_addr=0x%x, phy_addr=0x%x\n",
		priv->memory_pool, priv->sum_pool_size, priv->pool_vir_addr, priv->pool_phy_addr);

	sema_init(&priv->sem, 1);

	return 0;

err_pool_add:
	gen_pool_destroy(priv->memory_pool);
	priv->memory_pool = NULL;

err_pool_create:
	dma_free_coherent(priv->of_dev, priv->sum_pool_size, priv->pool_vir_addr, priv->pool_phy_addr);
	priv->pool_vir_addr = 0;
	priv->pool_phy_addr = 0;

	return -1;
}

static void cmdlist_of_device_release(struct platform_device *pdev)
{
	struct cmdlist_private *priv = (struct cmdlist_private *)platform_get_drvdata(pdev);

	if (!priv)
		return;

	if (priv->pool_vir_addr == 0)
		return;

	pr_info("free dmabuf vir_addr=0x%x, phy_addr=0x%x\n", priv->pool_vir_addr, priv->pool_phy_addr);

	dma_free_coherent(priv->of_dev, priv->sum_pool_size, priv->pool_vir_addr, priv->pool_phy_addr);

	priv->pool_vir_addr = 0;
	priv->pool_phy_addr = 0;
	priv->memory_pool = NULL;
}

#define CMDLIST_DEVICE_MATCH_DATA(name, ver, setup, release) \
	static struct cmdlist_match_data name = { .version = ver, .of_device_setup = setup, .of_device_release = release }

CMDLIST_DEVICE_MATCH_DATA(cmdlist_basic, GENERIC_DEVICETREE_CMDLIST, cmdlist_of_device_setup, cmdlist_of_device_release);

static const struct of_device_id device_match_table[] = {
	{
		.compatible = DTS_COMP_CMDLIST,
		.data = &cmdlist_basic,
	},
	{},
};
MODULE_DEVICE_TABLE(of, device_match_table);

static int cmdlist_probe(struct platform_device *pdev)
{
	const struct cmdlist_match_data *data = NULL;
	struct cmdlist_private *priv = &g_cmdlist_priv;

	if (!pdev)
		return -EINVAL;

	data = of_device_get_match_data(&pdev->dev);
	if (data == NULL) {
		pr_err("get device data failed!\n");
		return -EINVAL;
	}

	if (priv->device_initialized != 0) {
		pr_err("already initailed!\n");
		return 0;
	}
	memset(priv, 0, sizeof(*priv));
	priv->pdev = pdev;

	if (data->of_device_setup(pdev)) {
		pr_err("Device initialization is failed!\n");
		return -EINVAL;
	}

	if (cmdlist_device_setup(priv) != 0) {
		data->of_device_release(pdev);
		pr_err("Device setup failed!\n");
		return -EINVAL;
	}
	platform_set_drvdata(pdev, priv);

	priv->device_initialized = 1;

	pr_info("Device initialization is complete!\n");

	return 0;
}

/**
 * Clear resource when device removed but not for devicetree device
 */
static int cmdlist_remove(struct platform_device *pdev)
{
	const struct cmdlist_match_data *data = NULL;
	struct cmdlist_private *priv = (struct cmdlist_private *)platform_get_drvdata(pdev);

	if (!priv || (priv->device_initialized == 0)) {
		pr_err("has no initialization!\n");
		return -EINVAL;
	}

	if (priv->chr_major > 0) {
		device_destroy(priv->chr_class, MKDEV(priv->chr_major, 0));
		class_destroy(priv->chr_class);
		unregister_chrdev(priv->chr_major, DEV_NAME_CMDLIST);
		priv->chr_major = 0;
		pr_debug("unregister cmdlist chrdev device succ!\n");
	}

	data = of_device_get_match_data(&pdev->dev);
	if (data != NULL)
		data->of_device_release(pdev);

	pr_info("cmdlist remove complete!\n");

	priv->device_initialized = 0;

	return 0;
}

static struct platform_driver cmdlist_platform_driver = {
	.probe  = cmdlist_probe,
	.remove = cmdlist_remove,
	.driver = {
		.name  = DEV_NAME_CMDLIST,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(device_match_table),
	}
};

static int __init cmdlist_register(void)
{
	return platform_driver_register(&cmdlist_platform_driver);
}

static void __exit cmdlist_unregister(void)
{
	platform_driver_unregister(&cmdlist_platform_driver);
}

module_init(cmdlist_register);
module_exit(cmdlist_unregister);

MODULE_AUTHOR("Graphics Display");
MODULE_DESCRIPTION("Cmdlist Module Driver");
MODULE_LICENSE("GPL");
