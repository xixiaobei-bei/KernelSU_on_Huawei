/**
 * @file cmdlist_drv.h
 * @brief Interface for cmdlist driver function
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
#ifndef __CMDLIST_DRV_H__
#define __CMDLIST_DRV_H__

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/platform_device.h>

#define DEV_NAME_CMDLIST "cmdlist"
#define DTS_COMP_CMDLIST "hisilicon,dpu_cmdlist"

#undef pr_fmt
#define pr_fmt(fmt)  "cmdlist: " fmt

/* 128 bit */
#define ONE_ITEM_SIZE (128 / 8)

/* One cmdlist node is inlcude cmd_header(size: 16Byte) and cmd_item[0..N](size: 16Byte)
 * N: tatal_items[13:0] = 0x3FFF = 16383
 */
#define ITEMS_MAX_NUM (0x3FFF)

extern bool cmdlist_enable;
extern struct cmdlist_private g_cmdlist_priv;

enum cmdlist_arch_version {
	GENERIC_DEVICETREE_CMDLIST,
	CMDLIST_VERSION_MAX,
};

struct cmdlist_match_data {
	enum cmdlist_arch_version version;
	int (*of_device_setup) (struct platform_device *pdev);
	void (*of_device_release) (struct platform_device *pdev);
};

struct cmdlist_private {
	struct platform_device *pdev;
	struct device *of_dev;

	uint32_t chr_major;
	struct class *chr_class;
	struct device *chr_dev;

	uint32_t scene_num;

	size_t sum_pool_size;
	void *pool_vir_addr;
	dma_addr_t pool_phy_addr;
	struct gen_pool *memory_pool;

	int device_initialized;
	struct semaphore sem;
};

static inline struct cmdlist_private *get_cmdlist_private_data(void)
{
	return &g_cmdlist_priv;
}

int cmdlist_device_setup(struct cmdlist_private *priv);

#endif
