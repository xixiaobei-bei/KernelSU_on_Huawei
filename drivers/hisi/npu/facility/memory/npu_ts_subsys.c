/*
 * npu_ts_subsys.c
 *
 * about npu ts subsys
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include "npu_ts_subsys.h"

#include <linux/atomic.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/iommu.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_iommu.h>
#include <linux/platform_device.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/syscalls.h>
#include <linux/hisi-iommu.h>
#include <linux/vmalloc.h>
#include <linux/scatterlist.h>

#include "npu_platform.h"
#include "npu_log.h"

#define DEVDRV_MAX_SG_TABLE_NUM  20
struct t_sg_table_va_map {
	u8 is_used;
	struct sg_table *table;
	int nents;
	unsigned long long virt_addr;
};

struct t_sg_table_va_map_manager {
	spinlock_t spinlock;
	struct t_sg_table_va_map sg_table_va_map[DEVDRV_MAX_SG_TABLE_NUM];
	u8 used_num;
};

static struct t_sg_table_va_map_manager g_sg_table_va_map_manager = {
	.used_num = 0,
};

static int sg_table_va_map_init()
{
	// initial sa_table and g_sg_table_va_map_manager
	spin_lock_init(&g_sg_table_va_map_manager.spinlock);
	return 0;
}
static int sg_table_va_map_insert(struct sg_table *table, int nents, unsigned long long va)
{
	spin_lock(&g_sg_table_va_map_manager.spinlock);
	if (g_sg_table_va_map_manager.used_num >= DEVDRV_MAX_SG_TABLE_NUM) {
		spin_unlock(&g_sg_table_va_map_manager.spinlock);
		return -1;
	}
	g_sg_table_va_map_manager.sg_table_va_map[g_sg_table_va_map_manager.used_num].table = table;
	g_sg_table_va_map_manager.sg_table_va_map[g_sg_table_va_map_manager.used_num].nents = nents;
	g_sg_table_va_map_manager.sg_table_va_map[g_sg_table_va_map_manager.used_num].virt_addr = va;
	g_sg_table_va_map_manager.sg_table_va_map[g_sg_table_va_map_manager.used_num].is_used = 1;
	g_sg_table_va_map_manager.used_num++;
	spin_unlock(&g_sg_table_va_map_manager.spinlock);
	return 0;
}

static int sg_table_va_map_remove(unsigned long long va)
{
	int i;
	spin_lock(&g_sg_table_va_map_manager.spinlock);
	for (i = 0; i < g_sg_table_va_map_manager.used_num; i++) {
		if (g_sg_table_va_map_manager.sg_table_va_map[i].virt_addr == va)
			break;
	}
	if (i == g_sg_table_va_map_manager.used_num) {
		spin_unlock(&g_sg_table_va_map_manager.spinlock);
		return -1;
	}

	g_sg_table_va_map_manager.used_num--;
	if (g_sg_table_va_map_manager.used_num > i)
		g_sg_table_va_map_manager.sg_table_va_map[i] =
			g_sg_table_va_map_manager.sg_table_va_map[g_sg_table_va_map_manager.used_num];

	g_sg_table_va_map_manager.sg_table_va_map[g_sg_table_va_map_manager.used_num].table = NULL;
	g_sg_table_va_map_manager.sg_table_va_map[g_sg_table_va_map_manager.used_num].virt_addr = 0;
	g_sg_table_va_map_manager.sg_table_va_map[g_sg_table_va_map_manager.used_num].is_used = 0;
	spin_unlock(&g_sg_table_va_map_manager.spinlock);
	return 0;
}

static int sg_table_va_map_find(struct sg_table **table, int *nents, unsigned long long va)
{
	int i;
	if (table == NULL)
		return -1;

	spin_lock(&g_sg_table_va_map_manager.spinlock);
	for (i = 0; i < g_sg_table_va_map_manager.used_num; i++) {
		if (g_sg_table_va_map_manager.sg_table_va_map[i].virt_addr == va)
			break;
	}
	if (i == g_sg_table_va_map_manager.used_num) {
		spin_unlock(&g_sg_table_va_map_manager.spinlock);
		return -1;
	}
	*table = g_sg_table_va_map_manager.sg_table_va_map[i].table;
	*nents = g_sg_table_va_map_manager.sg_table_va_map[i].nents;
	spin_unlock(&g_sg_table_va_map_manager.spinlock);
	return 0;
}

int devdrv_npu_ts_subsys_alloc(unsigned long long *virt_addr, unsigned long *iova, unsigned long size)
{
	int i;
	int nents;
	int ret;
	int num;
	struct sg_table *table = NULL;
	struct page *page = NULL;
	struct page **pages = NULL;
	struct devdrv_platform_info *platform_info = NULL;
	struct scatterlist *sg = NULL;

	NPU_DRV_DEBUG("devdrv_npu_ts_subsys_alloc start \n");

	platform_info = devdrv_plat_get_info();
	if (platform_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info failed\n");
		return -ENODEV;
	}

	if (virt_addr == NULL || iova == NULL || size == 0) {
		NPU_DRV_ERR("%s [%d] invalid params!\n", __func__, __LINE__);
		return -1;
	}

	NPU_DRV_DEBUG("start to alloc pages\n");
	num = PAGE_ALIGN(size) / PAGE_SIZE;
	pages = vmalloc(sizeof(struct page *) * num);
	if (!pages)
		return -1;

	NPU_DRV_DEBUG("start to alloc sg_table.\n");
	table = kmalloc(sizeof(*table), GFP_KERNEL);
	if (!table) {
		ret = -1;
		goto free_pages;
	}

	if (sg_alloc_table(table, num, GFP_KERNEL)) {
		ret = -1;
		goto free_table;
	}

	NPU_DRV_DEBUG("start to alloc page, insert them into the sg_table, and set them in pages.\n");
	sg = table->sgl;
	for (i = 0; i < num; i++) {
		page = alloc_page(GFP_KERNEL);
		if (!page) {
			ret = -1;
			goto free_page;
		}
		sg_set_page(sg, page, PAGE_SIZE, 0);
		sg = sg_next(sg);

		pages[i] = page;
	}

	sg = table->sgl;

	*virt_addr = (unsigned long long)(uintptr_t)vmap(pages, num, VM_MAP, (pgprot_t)pgprot_writecombine(PAGE_KERNEL));
	*iova = hisi_iommu_map_sg(DEVDRV_PLAT_GET_PSMMUDEV(platform_info), sg, 0, &size);

	NPU_DRV_DEBUG("%s: iova 0x%lx, va 0x%x\n", __func__, *iova, *virt_addr);
	if (*virt_addr && *iova) {
	// Save the mapping table of sg_table and va, used to release memory
		ret = sg_table_va_map_insert(table, num, *virt_addr);
	} else {
		ret = -1;
	}
	if (ret == 0)
		goto free_pages;

free_page:
	nents = i;
	for_each_sg(table->sgl, sg, nents, i) {
		page = sg_page(sg);
		__free_page(page);
	}
	sg_free_table(table);
free_table:
	kfree(table);
	table = NULL;
free_pages:
	vfree(pages);
	return ret;
}

void devdrv_npu_ts_subsys_free(unsigned long long virt_addr, unsigned long iova)
{
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	int num = 6;
	int nents;
	int ret;
	struct devdrv_platform_info *platform_info = NULL;

	NPU_DRV_DEBUG("smmu_free start \n");

	platform_info = devdrv_plat_get_info();
	if (platform_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info failed\n");
		return;
	}

	NPU_DRV_DEBUG("%s %d\n", __func__, __LINE__);
	(void)sg_table_va_map_find(&table, &num, virt_addr);
	if (!table)
		return;

	sg = table->sgl;
	// 1.free iova
	ret = hisi_iommu_unmap_sg(DEVDRV_PLAT_GET_PSMMUDEV(platform_info), sg, iova);
	NPU_DRV_DEBUG("return %d after unmap sg\n", ret);
	if (ret)
		NPU_DRV_ERR("%s: iova 0x%lx unmap fail\n", __func__, iova);
	// 2. unmap va
	vunmap((const void *)(uintptr_t)virt_addr);

	// 3. free sg_table
	nents = num;
	for_each_sg(table->sgl, sg, nents, num) {
		page = sg_page(sg);
		__free_page(page);
	}
	sg_free_table(table);
	kfree(table);
	NPU_DRV_DEBUG("%s %d\n", __func__, __LINE__);
	// 4. delete map node
	sg_table_va_map_remove(virt_addr);
	return;
}


int devdrv_npu_ts_subsys_probe(struct platform_device *pdev)
{
	struct devdrv_platform_info *platform_info = NULL;

	NPU_DRV_DEBUG("devdrv_smmu_probe start \n");

	platform_info = devdrv_plat_get_info();
	if (platform_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info failed\n");
		return -ENODEV;
	}

	DEVDRV_PLAT_GET_PSMMUDEV(platform_info) = &pdev->dev;
	return 0;
}

int devdrv_npu_ts_subsys_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id npu_ts_subsys_of_match[] = {
	{ .compatible = "hisi,npu_ts_subsys", },
	{},
};
MODULE_DEVICE_TABLE(of, npu_ts_subsys_of_match);

static struct platform_driver devdrv_npu_ts_subsys_driver = {
	.probe = devdrv_npu_ts_subsys_probe,
	.remove = devdrv_npu_ts_subsys_remove,
	.driver = {
		.name = "npu_ts_subsys",
		.of_match_table = npu_ts_subsys_of_match,
	},
};

static int __init devdrv_npu_ts_subsys_init(void)
{
	int ret;

	NPU_DRV_DEBUG("devdrv_npu_ts_subsys_init started\n");

	sg_table_va_map_init();

	ret = platform_driver_register(&devdrv_npu_ts_subsys_driver);
	if (ret) {
		NPU_DRV_ERR("insmod devdrv npu_ts_subsys driver fail\n");
		return ret;
	}
	NPU_DRV_DEBUG("devdrv_npu_ts_subsys_init succeed\n");

	return ret;
}
module_init(devdrv_npu_ts_subsys_init);

static void __exit devdrv_npu_ts_subsys_exit(void)
{
	platform_driver_unregister(&devdrv_npu_ts_subsys_driver);
}
module_exit(devdrv_npu_ts_subsys_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd");
MODULE_DESCRIPTION("DAVINCI driver");
MODULE_VERSION("V1.0");

