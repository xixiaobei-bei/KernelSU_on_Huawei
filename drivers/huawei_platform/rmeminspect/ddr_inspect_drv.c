/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: ddr inspect register chdev
 * Author: zhouyubin
 * Create: 2019-05-30
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "ddr_inspect_drv.h"

#include <global_ddr_map.h>
#include <securec.h>

#define DEV_NAME			"ddr_sanecheck"
#define DEVICE_CLASS_NAME	"ddr_sanecheck"
#define DEVICE_FILE_NAME	"ddr_sanecheck"

#ifdef HISI_SUB_RESERVED_DDR_REPAIR_PHYMEM_BASE
#define DDR_RESERVED		HISI_SUB_RESERVED_DDR_REPAIR_PHYMEM_BASE
#else
#define DDR_RESERVED		0
#endif

#define MAP_SIZE			32
#define RSV_SIZE			0x4000000
#define ER_MAX_RSV			5 /* for data recovery in erecovery */

dev_t ddr_insp_dev_no;
struct ddr_inspect_dev ddr_insp_cdev;
static struct class *dev_class;
static struct ddr_inspect_region region;

static void sane_reserve(void)
{
	uint64_t base;
	uint16_t i;
	int ret;
	char *virt_addr = NULL;
	uint16_t flag = 0;
	struct ddr_rsv_data rsv_data;

	if (!DDR_RESERVED)
		return;

	virt_addr = (char *)early_ioremap(DDR_RESERVED, MAP_SIZE);
	if (!virt_addr) {
		pr_err("%s, early_ioremap fail \n", __func__);
		return;
	}

	ret = memcpy_s(&rsv_data, sizeof(rsv_data), (struct ddr_rsv_data *)virt_addr,
			sizeof(rsv_data));
	if (ret != EOK || rsv_data.cnt > MAX_RSV_NUM) {
		pr_err("%s memcpy fail\n", __func__);
		early_iounmap(virt_addr, MAP_SIZE);
		return;
	}

	for (i = 0; i < rsv_data.cnt; i++)
		flag += rsv_data.data[i];
	if (flag != rsv_data.flag) {
		early_iounmap(virt_addr, MAP_SIZE);
		pr_err("flag had changed\n");
		return;
	}

	for (i = 0; i < rsv_data.cnt; i++)
		base = (uint64_t)(rsv_data.data[i]) * RSV_SIZE;

	pr_err(" %s: %x, cnt %hu\n", __func__, DDR_RESERVED, rsv_data.cnt);

	early_iounmap(virt_addr, MAP_SIZE);
}

static void add_dynamic_area(phys_addr_t base, unsigned long len,
	char *name)
{
	struct ddr_inspect_region *reg = &region;

	if (name == NULL)
		return;

	reg->base = base;
	reg->size = len;
	reg->name = name;
}

static int ddr_inspect_reserve_area(struct reserved_mem *rmem)
{
	char* heapname = NULL;
	char *status = NULL;
	int namesize = 0;

	if (rmem == NULL)
		return 0;

	status = (char *)of_get_flat_dt_prop(rmem->fdt_node, "status", NULL);
	if (status && (strncmp(status, "ok", strlen("ok")) != 0))
		return 0;

	heapname = (char *)of_get_flat_dt_prop(rmem->fdt_node, "region-name",
		&namesize);
	if (!heapname || (namesize <= 0)) {
		pr_err("%s, no 'region-name' property namesize=%d\n",
				__func__, namesize);
		return -EFAULT;
	}

	pr_err("%s %llx %llx\n", __func__, rmem->base, rmem->size);
	add_dynamic_area(rmem->base, rmem->size, heapname);
	sane_reserve();

	return 0;
}
RESERVEDMEM_OF_DECLARE(ddr_inspect_region, "ddr_inspect", ddr_inspect_reserve_area);

static void __exit ddr_inspect_exit(void)
{
	if (!IS_ERR(dev_class)) {
		device_destroy(dev_class, ddr_insp_dev_no);
		class_destroy(dev_class);
	}

	cdev_del(&ddr_insp_cdev.cdev);
	unregister_chrdev_region(ddr_insp_dev_no, DRV_NR);
}

static int __init ddr_inspect_init(void)
{
	int ret;
	struct device *dev_ret = NULL;

	ret = memset_s(&ddr_insp_cdev, sizeof((ddr_insp_cdev)), 0,
		sizeof(ddr_insp_cdev));
	if (ret != EOK)
		return ret;

	ret = alloc_chrdev_region(&ddr_insp_dev_no, 0, DRV_NR, DEV_NAME);
	if (ret < 0)
		return ret;

	cdev_init(&ddr_insp_cdev.cdev, &g_ddr_insp_fops);
	ddr_insp_cdev.cdev.owner = THIS_MODULE;
	ddr_insp_cdev.cdev.ops = &g_ddr_insp_fops;
	sema_init(&ddr_insp_cdev.sem, 1);
	ret = cdev_add(&ddr_insp_cdev.cdev, ddr_insp_dev_no, DRV_NR);
	if (ret < 0) {
		pr_err("%s cdev_add fail\n", __func__);
		goto unreg_dev;
	}

	dev_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME);
	if (IS_ERR(dev_class)) {
		pr_err("%s class_create fail\n", __func__);
		goto del_dev;
	}

	dev_ret = device_create(dev_class, NULL, ddr_insp_dev_no,
				NULL, DEV_NAME);
	if (IS_ERR(dev_ret)) {
		pr_err("%s device_create fail\n", __func__);
		goto del_class;
	}

	return 0;

del_class:
	class_destroy(dev_class);
del_dev:
	cdev_del(&ddr_insp_cdev.cdev);
unreg_dev:
	unregister_chrdev_region(ddr_insp_dev_no, DRV_NR);

	return ret;
}

module_init(ddr_inspect_init);
module_exit(ddr_inspect_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("DDR INSPECT DRIVER");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
