/*
 * driver, hisp_loadbin.c
 *
 * Copyright (c) 2013- ISP Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <isp_ddr_map.h>
#include <linux/syscalls.h>
#include <securec.h>

#define PART_ISPFW_SIZE 0x00E00000
#define ISP_BIN_RSC_OFFSET 0x3000

struct hisp_loadbin_dev {
	int loadbin;
	void *isp_bin_vaddr;
	unsigned int isp_bin_state;
	struct task_struct *loadispbin;
};

static struct hisp_loadbin_dev g_hisp_bin_dev;

int is_use_loadbin(void)
{
	struct hisp_loadbin_dev *dev = &g_hisp_bin_dev;

	return dev->loadbin;
}

static int isp_bin_load(void *data)
{
#define PART_ISPFW_SIZE 0x00E00000
	int ret = -1;
	unsigned int fw_size;
	struct hisp_loadbin_dev *bin_dev = &g_hisp_bin_dev;

	pr_info("[%s] : +\n", __func__);
	fw_size = ISP_FW_SIZE > PART_ISPFW_SIZE ? ISP_FW_SIZE : PART_ISPFW_SIZE;
	bin_dev->isp_bin_vaddr = vmalloc(fw_size);
	if (bin_dev->isp_bin_vaddr == NULL) {
		pr_err("[%s] Failed : vmalloc.%pK\n",
			__func__, bin_dev->isp_bin_vaddr);
		return -ENOMEM;
	}

	ret = hisp_bsp_read_bin("isp_firmware", 0, PART_ISPFW_SIZE,
					bin_dev->isp_bin_vaddr);
	if (ret < 0) {
		vfree(bin_dev->isp_bin_vaddr);
		bin_dev->isp_bin_vaddr = NULL;
		pr_err("[%s] Failed : hisp_bsp_read_bin.%d\n", __func__, ret);
		return ret;
	}
	bin_dev->isp_bin_state = 1;
	pr_info("[%s] : -\n", __func__);

	return 0;
}


void *hisp_loadbin_rsctable_init(void)
{
	struct hisp_loadbin_dev *bin_dev = &g_hisp_bin_dev;

	pr_info("[%s] +\n", __func__);
	if (bin_dev->isp_bin_vaddr == NULL) {
		pr_err("[%s] isp_bin_vaddr.%pK\n", __func__,
				bin_dev->isp_bin_vaddr);
		return NULL;
	}

	return kmemdup(bin_dev->isp_bin_vaddr + ISP_BIN_RSC_OFFSET,
			SZ_4K, GFP_KERNEL);
}

int isp_loadbin_debug_elf(void)
{

	return 0;
}

int hisp_bin_load_segments(struct rproc *rproc)
{
	struct hisp_loadbin_dev *bin_dev = &g_hisp_bin_dev;
	int ret = 0;
	void *ptr = NULL;

	/* go through the available ELF segments */
	u32 da = TEXT_BASE;
	u32 memsz = ISP_TEXT_SIZE;

	pr_info("[%s] : +\n", __func__);
	/* grab the kernel address for this device address */
	ptr = rproc_da_to_va(rproc, da, memsz);
	pr_info("[%s] : text section ptr = %pK\n", __func__, ptr);
	if (ptr == NULL) {
		pr_err("[%s]:bad phdr da 0x%x mem 0x%x\n", __func__, da, memsz);
		ret = -EINVAL;
		return ret;
	}
	/* put the segment where the remote processor expects it */
	ret = memcpy_s(ptr, memsz, bin_dev->isp_bin_vaddr, memsz);
	if (ret != 0) {
		pr_err("%s:memcpy text vaddr to ptr fail.%d\n", __func__, ret);
		return ret;
	}

	/* go through the available ELF segments */
	da = DATA_BASE;
	memsz = ISP_BIN_DATA_SIZE;
	ptr = rproc_da_to_va(rproc, da, memsz);
	pr_info("[%s] : data section ptr = %pK\n", __func__, ptr);
	if (ptr == NULL) {
		pr_err("%s:bad phdr da 0x%x mem 0x%x\n", __func__, da, memsz);
		ret = -EINVAL;
		return ret;
	}
	/* put the segment where the remote processor expects it */
	ret = memcpy_s(ptr, memsz,
			bin_dev->isp_bin_vaddr + ISP_TEXT_SIZE, memsz);
	if (ret != 0) {
		pr_err("%s:memcpy data vaddr to ptr fail.%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

int wakeup_ispbin_kthread(void)
{
	struct hisp_loadbin_dev *bin_dev = &g_hisp_bin_dev;
	int timeout = 1000;

	pr_info("[%s] : +\n", __func__);

	if (is_use_loadbin() == 0)
		return 0;

	if (bin_dev->isp_bin_state == 1)
		return 0;

	if (bin_dev->loadispbin == NULL) {
		pr_err("%s:Failed : loadispbin is NULL\n", __func__);
		return -1;
	}

	wake_up_process(bin_dev->loadispbin);
	do {
		timeout--;
		mdelay(10);
		if (bin_dev->isp_bin_state == 1)
			break;
	} while (timeout > 0);

	pr_info("[%s] : isp_bin_state.%d, timeout.%d\n",
		__func__, bin_dev->isp_bin_state, timeout);

	return timeout > 0 ? 0 : -1;
}

int hisp_loadbin_init(struct platform_device *pdev)
{
	int ret;
	struct device *dev = NULL;
	struct device_node *np = NULL;
	struct hisp_loadbin_dev *bin_dev = &g_hisp_bin_dev;

	bin_dev->isp_bin_state = 0;
	bin_dev->isp_bin_vaddr = NULL;

	if (pdev == NULL) {
		pr_err("[%s] Failed : pdev.%pK\n", __func__, pdev);
		return -ENOMEM;
	}

	dev = &pdev->dev;
	np = dev->of_node;

	ret = of_property_read_u32(np, "useloadbin", &bin_dev->loadbin);
	if (ret < 0) {/*lint !e64 */
		pr_err("[%s] Failed: loadbin.0x%x of_property_read_u32.%d\n",
			__func__, bin_dev->loadbin, ret);
		return ret;
	}

	if (bin_dev->loadbin) {
		bin_dev->loadispbin = kthread_create(isp_bin_load,
			NULL, "loadispbin");
		if (IS_ERR(bin_dev->loadispbin)) {
			ret = -1;
			pr_err("[%s] Failed : kthread_create.%ld\n",
				__func__, PTR_ERR(bin_dev->loadispbin));
			return ret;
		}
	}

	return 0;
}

void hisp_loadbin_deinit(void)
{
	struct hisp_loadbin_dev *bin_dev = &g_hisp_bin_dev;

	if (bin_dev->loadbin) {
		if (bin_dev->loadispbin != NULL) {
			kthread_stop(bin_dev->loadispbin);
			bin_dev->loadispbin = NULL;
		}
	}

	bin_dev->isp_bin_state = 0;
	bin_dev->isp_bin_vaddr = NULL;
}


