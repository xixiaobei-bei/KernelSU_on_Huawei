/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: ddr inspect process ioctl request
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

#include <asm/pgtable.h>
#include <asm/pgtable-types.h>
#include <asm/tlbflush.h>
#include <linux/delay.h>
#include <linux/fcntl.h>
#include <linux/gfp.h>
#if defined(CONFIG_CHIP_HONGHU)
#else
#include <linux/hisi/hisi_bbox_diaginfo.h>
#endif
#include <linux/hugetlb.h>
#include <linux/log2.h>
#include <linux/memory_hotplug.h>
#include <linux/moduleparam.h>
#include <linux/mmzone.h>
#include <linux/mm_types.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/page_ext.h>
#include <linux/page-flags.h>
#include <linux/pageblock-flags.h>
#include <linux/page-isolation.h>
#include <linux/swap.h>
#include <linux/time.h>
#include <linux/version.h>
#include <linux/vmstat.h>

#include <securec.h>

#include "internal.h"
#if defined(CONFIG_CHIP_HONGHU)
#define DDR_INFO_ADDR 0xF8000428
#else
#include "soc_acpu_baseaddr_interface.h"
#include "soc_sctrl_interface.h"

#define DDR_INFO_ADDR (SOC_SCTRL_SCBAKDATA7_ADDR(SOC_ACPU_SCTRL_BASE_ADDR))
#endif

unsigned long g_pre_isolate_ddr;

struct reboot_reason_tbl g_reason_tbl[] = {
	{AP_S_PANIC, "AP_S_PANIC"},
	{AP_S_AWDT, "AP_S_AWDT"},
	{AP_S_F2FS, "AP_S_F2FS"},
	{AP_S_PRESS6S, "AP_S_PRESS6S"},
	{AP_S_COMBINATIONKEY, "AP_S_COMBINATIONKEY"},
	{BFM_S_NATIVE_BOOT_FAIL, "BFM_S_NATIVE_BOOT_FAIL"},
	{BFM_S_BOOT_TIMEOUT, "BFM_S_BOOT_TIMEOUT"},
	{BFM_S_FW_BOOT_FAIL, "BFM_S_FW_BOOT_FAIL"},
	{BFM_S_NATIVE_DATA_FAIL, "BFM_S_NATIVE_DATA_FAIL"},
	{NORMAL_REBOOT, "NULL"}
};

#define round_up_to(x, y)		(((x) + (y) - 1) / (y) * (y))
#define round_up_to_page(x)		round_up_to((x), PAGE_SIZE)

unsigned long g_reboot_flag_ddr;

/* alloc conti pfn like alloc_gigantic_page */
static bool pfn_range_valid(struct zone *z,
				unsigned long start_pfn, unsigned long nr_pages)
{
	unsigned long i;
	unsigned long end_pfn = start_pfn + nr_pages;
	struct page *page = NULL;

	if (z == NULL)
		return -EINVAL;

	for (i = start_pfn; i < end_pfn; i++) {
		if (!pfn_valid(i))
			return false;

		page = pfn_to_page(i);
		if (page_zone(page) != z)
			return false;

		if (PageReserved(page))
			return false;

		if (page_count(page) > 0)
			return false;

		if (PageHuge(page))
			return false;
	}

	return true;
}

int alloc_conti_pages(unsigned long start_pfn, unsigned long nr_pages)
{
	unsigned long ret;
	unsigned long flags;
	struct zone *z = NULL;

	z = page_zone(pfn_to_page(start_pfn));
	spin_lock_irqsave(&z->lock, flags);

	if (pfn_range_valid(z, start_pfn, nr_pages)) {
		spin_unlock_irqrestore(&z->lock, flags);
		ret = alloc_contig_range(start_pfn, start_pfn + nr_pages,
			MIGRATE_MOVABLE, GFP_KERNEL | __GFP_NOWARN);
		if (!ret)
			return 0;
		spin_lock_irqsave(&z->lock, flags);
	}
	spin_unlock_irqrestore(&z->lock, flags);

	return 1;
}

int set_pfn_poisoned(unsigned long pfn)
{
	struct page *p = NULL;
	int res = 0;

	p = pfn_to_page(pfn);
#ifdef CONFIG_MEMORY_FAILURE
	if (TestSetPageHWPoison(p))
		return 0;
	shake_page(p, 0);
	lock_page(p);
	if (!PageHWPoison(p)) {
		pr_err("%s:: %#lx:not poison\n", __func__, pfn);
		unlock_page(p);
		put_hwpoison_page(p);
		return 0;
	}
	wait_on_page_writeback(p);
	unlock_page(p);

	if (!get_hwpoison_page(p)) {
		if (is_free_buddy_page(p))
			return 0;
		else
			return -EBUSY;
	}
#endif

	return res;
}

int write_hisi_nve_ddrfault(struct user_nve_info *request)
{
	struct hisi_nve_info_user nve_info;
	int ret;
	char name[] = "PPR";
	int nv_number;
	int opcode;

	if (request == NULL)
		return -1;

	nv_number = request->nv_number;
	opcode = request->opcode;

	if (memset_s(&nve_info, sizeof(nve_info), 0, sizeof(nve_info)) != EOK)
		return -1;

	if (memcpy_s(nve_info.nv_name, sizeof(nve_info.nv_name), name,
		strlen(name)) != EOK)
		return -1;

	nve_info.nv_number = nv_number;
	nve_info.valid_size = NVE_DATA_BYTE_SIZE;
	nve_info.nv_operation = opcode;
	if (opcode == NV_WRITE) {
		if (memcpy_s(nve_info.nv_data, NVE_DATA_BYTE_SIZE,
			request->nve_data, NVE_DATA_BYTE_SIZE) != EOK)
			return -1;
	}

	ret = hisi_nve_direct_access(&nve_info);
	if (ret)
		return -1;

	if (opcode == NV_READ) {
		if (memcpy_s(request->nve_data, NVE_DATA_BYTE_SIZE,
			nve_info.nv_data, NVE_DATA_BYTE_SIZE) != EOK)
			return -1;
		return (int)(nve_info.nv_data[0]);
	}

	return 0;
}

static int __init wp_reboot_reason_cmdline(char *reboot_reason_cmdline)
{
	int i = 0;

	if (reboot_reason_cmdline == NULL)
		return -1;

	g_reboot_flag_ddr = NORMAL_REBOOT;
	while (strcmp("NULL", g_reason_tbl[i].reboot_reason_cmdline) != 0) {
		if (!strcmp(reboot_reason_cmdline,
			g_reason_tbl[i].reboot_reason_cmdline)) {
			g_reboot_flag_ddr = g_reason_tbl[i].reboot_reason_num;
			return 0;
		}
		i++;
	}

	return 0;
}

int ddr_bbox_diaginfo(const struct user_nve_info *request)
{
	if (request == NULL)
		return -1;

#if defined(CONFIG_CHIP_HONGHU)
	return 1;
#else
	bbox_diaginfo_record(LPM3_DDR_FAIl, NULL,
		"ddr device fail (%u 64MB-section): %u %u %u %u",
		*((uint16_t *)(request->nve_data)),
		*((uint16_t *)(request->nve_data) + FIRST_INDEX),
		*((uint16_t *)(request->nve_data) + SECOND_INDEX),
		*((uint16_t *)(request->nve_data) + THIRD_INDEX),
		*((uint16_t *)(request->nve_data) + FORTH_INDEX));
#endif

	return 1;
}

early_param("reboot_reason", wp_reboot_reason_cmdline);

void free_per_page(unsigned long start_pfn, unsigned int nr_pages)
{
	if (likely(g_pre_isolate_ddr != PRE_IOSLATE_NUM)) {
		for (; nr_pages--; start_pfn++) {
			struct page *page = pfn_to_page(start_pfn);

			if (!pfn_valid(start_pfn))
				continue;
			if (page_count(page))
				__free_page(page);
			else
				pr_err("not free 0x%x.\n", page_to_pfn(page));
		}
	}
}

static int ddr_inspect_open(struct inode *inode, struct file *filp)
{
	int minor;
	struct ddr_inspect_dev *dev = NULL;

	if (!inode || !filp)
		return -EINVAL;

	minor = MINOR(inode->i_rdev);
	if (minor >= DRV_NR)
		return -ENODEV;

	dev = container_of(inode->i_cdev, struct ddr_inspect_dev, cdev);
	if (!dev)
		return -ENODEV;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	dev->mem_req_num = 0;
	dev->req_vma = NULL;

	filp->private_data = dev;

	return 0;
}

static int ddr_inspect_release(struct inode *inode, struct file *filp)
{
	unsigned int i;
	unsigned long start_pfn;
	struct ddr_inspect_dev *dev = NULL;

	if ((inode == NULL) || (filp == NULL))
		return -EINVAL;

	dev = container_of(inode->i_cdev, struct ddr_inspect_dev, cdev);
	if (!dev) {
		up(&dev->sem);
		return -ENODEV;
	}

	if (dev->req_vma) {
		for (i = 0; i < dev->mem_req_num; i++) {
			start_pfn = dev->req_vma[i].req_pfn;
			if (start_pfn != 0)
				free_per_page(start_pfn, 1 << MEM_REQ_ORDER);
		}
		vfree(dev->req_vma);
		dev->req_vma = NULL;
		dev->mem_req_num = 0;
	}

	g_pre_isolate_ddr = 0;

	up(&dev->sem);

	return 0;
}

int verify_req_pfn(unsigned long start_pfn, int conti_nr)
{
	int i;

	for (i = 0; i < conti_nr; i++) {
		if (!pfn_valid(start_pfn + i))
			return -ENOMEM;
	}

	return 0;
}

int handle_request_pages(struct ddr_inspect_dev *ddr_insp_dev,
				const struct ddr_inspect_request *request)
{
	int i;
	unsigned int size;
	unsigned long start_pfn;
	int conti_pfn_nr = PAGE_NUM_ONE_MB;
	unsigned long current_offset_in_vma = 0;

	if (!ddr_insp_dev || !request)
		return -EINVAL;

	size = sizeof(struct ddr_inspect_vma) * request->req_nr;
	ddr_insp_dev->req_vma = vmalloc(size);
	if (ddr_insp_dev->req_vma == NULL)
		return -ENOMEM;

	if (memset_s(ddr_insp_dev->req_vma, size, 0, size) != EOK)
		goto out_to_open;

	ddr_insp_dev->mem_req_num = request->req_nr;
	for (i = 0; i < request->req_nr; i++) {
		start_pfn = request->req_pfn[i];
		if (start_pfn % conti_pfn_nr != 0)
			goto out_to_open;

		if (verify_req_pfn(start_pfn, conti_pfn_nr))
			continue;

		if (!alloc_conti_pages(start_pfn, conti_pfn_nr)) {
			ddr_insp_dev->req_vma[i].req_pfn = start_pfn;
			ddr_insp_dev->req_vma[i].vm_offset =
				current_offset_in_vma;
			current_offset_in_vma += PAGE_SIZE * conti_pfn_nr;
		} else {
			ddr_insp_dev->req_vma[i].req_pfn = 0;
			ddr_insp_dev->req_vma[i].vm_offset = 0;
		}
	}

	return 0;

out_to_open:

	pr_err("%s ioctl fail!\n", __func__);
	vfree(ddr_insp_dev->req_vma);
	ddr_insp_dev->req_vma = NULL;

	return -EINVAL;
}

static long file_ioctl_req_page(struct ddr_inspect_dev *ddr_insp_dev,
				unsigned long arg)
{
	int ret;
	struct ddr_inspect_request request;

	if (!ddr_insp_dev)
		return -EINVAL;

	if (memset_s(&request, sizeof(request), 0, sizeof(request)) != EOK) {
		pr_err("%s memset_s failed\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&request,
		(struct ddr_inspect_request __user *)(uintptr_t)arg,
		sizeof(request))) {
		pr_err("%s copy_from_user fail\n", __func__);
		ret = -EFAULT;
	} else {
		if (request.conti_pfn_nr != PAGE_NUM_ONE_MB ||
				request.req_nr != MAX_REQ_NR)
			ret = -EINVAL;
		else
			ret = handle_request_pages(ddr_insp_dev, &request);
	}

	return ret;
}

static long file_ioctl_mark_page(struct ddr_inspect_dev *ddr_insp_dev,
				unsigned long arg)
{
	int ret = 0;
	unsigned long bad_pfn;
	struct zone *zone = NULL;

	if (!ddr_insp_dev)
		return -EINVAL;

	if (__get_user(bad_pfn, (unsigned long __user *)(uintptr_t)arg)) {
		pr_err("%s failed\n", __func__);
		return -EFAULT;
	}

	if (!pfn_valid(bad_pfn)) {
		ret = -EINVAL;
	} else {
#ifdef CONFIG_MEMORY_FAILURE
		if (!PageHWPoison(pfn_to_page(bad_pfn))) {
			ret = set_pfn_poisoned(bad_pfn);
			if (ret)
				return ret;
		}
#endif
		for_each_populated_zone(zone)
			drain_all_pages(zone);
	}
	return ret;
}

static long file_ioctl_get_ddr_info(struct ddr_inspect_dev *ddr_insp_dev,
				unsigned long arg)
{
	int ret;
	struct ddr_info request;
	unsigned int tmp_reg_value;
	unsigned long *virtual_addr = NULL;

	if (ddr_insp_dev == NULL)
		return -EINVAL;

#if defined(CONFIG_CHIP_HONGHU)
	return 0;
#endif

	if (memset_s((void *)&request, sizeof(request), 0, sizeof(request)) !=
		EOK) {
		pr_err("%s memset_s failed\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&request, (struct ddr_info __user *)(uintptr_t)arg,
				sizeof(request))) {
		pr_err("%s copy_from_user fail\n", __func__);
		ret = -EFAULT;
	} else {
		virtual_addr = (unsigned long *)ioremap_nocache(
			DDR_INFO_ADDR & 0xFFFFF000, 0x800);
		if (virtual_addr == NULL) {
			pr_err("%s	ioremap ERROR !!\n", __func__);
			return -EFAULT;
		}

		tmp_reg_value = *(unsigned long *)((uintptr_t)virtual_addr +
			(DDR_INFO_ADDR & 0x00000FFF));
		iounmap(virtual_addr);

		tmp_reg_value = tmp_reg_value & 0xFFF;
		ret = snprintf_s(request.ddr_info_str, STR_DDR_INFO_LEN,
			STR_DDR_INFO_LEN - 1, "ddr_info:\n0x%x\n",
			tmp_reg_value);
		if (ret < 0) {
			pr_err("%s	snprintf ERROR !!\n", __func__);
			return -EFAULT;
		}
		ret = copy_to_user((struct ddr_info __user *)(uintptr_t)arg,
			&request, sizeof(request));
	}

	return ret;
}

static long file_ioctl_access_nve(struct ddr_inspect_dev *ddr_insp_dev,
				unsigned long arg)
{
	int ret;
	struct user_nve_info request;

	if (!ddr_insp_dev)
		return -EINVAL;

	if (memset_s((void *)&request, sizeof(request), 0, sizeof(request)) !=
		EOK) {
		pr_err("%s memset_s failed\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&request,
		(struct user_nve_info __user *)(uintptr_t)arg,
		sizeof(request))) {
		pr_err("%s copy_from_user fail\n", __func__);
		ret = -EFAULT;
	} else {
		ret = write_hisi_nve_ddrfault(&request);
		if (ret == -1) {
			pr_err("%s access nve fail.\n", __func__);
			request.err_flag = -1;
		}
		ret = copy_to_user((struct user_nve_info __user *)(uintptr_t)arg,
			&request, sizeof(request));
	}

	return ret;
}

static long file_ioctl_reason(struct ddr_inspect_dev *ddr_insp_dev,
				unsigned long arg)
{
	int ret;
	struct reboot_reason request;

	if (!ddr_insp_dev)
		return -EINVAL;

	if (memset_s((void *)&request, sizeof(request), 0, sizeof(request)) !=
		EOK) {
		pr_err("%s memset_s failed\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&request,
		(struct reboot_reason __user *)(uintptr_t)arg,
		sizeof(request))) {
		pr_err("%s copy_from_user fail\n", __func__);
		ret = -EFAULT;
	} else {
		g_pre_isolate_ddr = request.wp_reboot_reason;
		request.wp_reboot_reason = g_reboot_flag_ddr;
		ret = copy_to_user((struct reboot_reason __user *)(uintptr_t)arg,
			&request, sizeof(request));
	}

	return ret;
}

static long file_ioctl_bbox_record(struct ddr_inspect_dev *ddr_insp_dev,
				unsigned long arg)
{
	int ret = 0;
	struct user_nve_info request;

	if (!ddr_insp_dev)
		return -EINVAL;

	if (memset_s((void *)&request, sizeof(request), 0, sizeof(request)) !=
		EOK) {
		pr_err("%s memset_s failed\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&request,
		(struct user_nve_info __user *)(uintptr_t)arg,
		sizeof(request))) {
		pr_err("%s copy_from_user fail\n", __func__);
		ret = -EFAULT;
	} else {
		ddr_bbox_diaginfo(&request);
	}

	return ret;
}

static long file_ioctl_get_memory(struct ddr_inspect_dev *ddr_insp_dev,
				unsigned long arg)
{
	int ret = 0;
	struct ddr_memory_type memory_type;
	struct memblock_region *reg = NULL;
	int memory_count = 0;
	int reserved_count = 0;

	if (!ddr_insp_dev)
		return -EINVAL;

	if (memset_s((void *)&memory_type, sizeof(memory_type), 0,
		sizeof(memory_type)) != EOK) {
		pr_err("%s memset_s failed\n", __func__);
		return -EFAULT;
	}

	for_each_memblock(memory, reg) {
		memory_type.memory.memory_info[memory_count].base = reg->base;
		memory_type.memory.memory_info[memory_count].end =
			reg->base + reg->size - 1;
		memory_count++;
	}
	for_each_memblock(reserved, reg) {
		memory_type.reserved.memory_info[reserved_count].base =
			reg->base;
		memory_type.reserved.memory_info[reserved_count].end =
			reg->base + reg->size - 1;
		reserved_count++;
	}
	memory_type.memory.count = memory_count;
	memory_type.reserved.count = reserved_count;
	ret = copy_to_user((struct ddr_memory_type __user *)(uintptr_t)arg,
			&memory_type, sizeof(memory_type));

	return ret;
}

static ssize_t ddr_inspect_read(struct file *filp, char __user *buf,
	size_t count, loff_t *f_pos)
{
	ssize_t ret = 0;
	ssize_t max_size;
	struct ddr_inspect_dev *ddr_insp_dev = NULL;

	if ((buf == NULL) || (filp == NULL) || (f_pos == NULL))
		return -EINVAL;

	ddr_insp_dev = (struct ddr_inspect_dev *)filp->private_data;
	max_size = sizeof(struct ddr_inspect_vma) * ddr_insp_dev->mem_req_num;

	if (*f_pos > max_size)
		return ret;

	if (*f_pos + count > max_size)
		count = max_size - *f_pos;

	if (copy_to_user(buf, (void *)(uintptr_t)ddr_insp_dev->req_vma + *f_pos,
		count))
		return -EFAULT;

	*f_pos += count;

	return count;
}

const struct ddr_insp_ioc g_ddr_insp_ioc_tbl[IOC_MAXNR + 1] = {
	{IOC_REQ, file_ioctl_req_page},
	{IOC_MARK_BAD, file_ioctl_mark_page},
	{IOC_DDRINFO, file_ioctl_get_ddr_info},
	{IOC_NVE, file_ioctl_access_nve},
	{IOC_REASON, file_ioctl_reason},
	{IOC_BBOX, file_ioctl_bbox_record},
	{IOC_MEMORY, file_ioctl_get_memory}
};

static long ddr_inspect_ioctl(struct file *filp, unsigned int cmd,
				unsigned long arg)
{
	int ioc_no;
	int ret = 0;
	struct ddr_inspect_dev *ddr_insp_dev = NULL;
	long (*ioc_func)(struct ddr_inspect_dev *, unsigned long) = NULL;

	if (filp == NULL)
		return -EINVAL;

	ioc_no = _IOC_NR(cmd);
	if (_IOC_TYPE(cmd) != IOC_DDR_INSPECT_MAGIC || ioc_no > IOC_MAXNR)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void __user *)(uintptr_t)arg,
			_IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret = !access_ok(VERIFY_READ, (void __user *)(uintptr_t)arg,
			_IOC_SIZE(cmd));
	if (ret)
		return -EFAULT;

	ddr_insp_dev = (struct ddr_inspect_dev *)filp->private_data;

	/* now, ioc_no is valid, then we can get the right ioctl. */
	ioc_func = g_ddr_insp_ioc_tbl[ioc_no].ioc_func;
	if (ioc_func) {
		ret = ioc_func(ddr_insp_dev, arg);
	} else {
		pr_err("%s not succ, ioc cmd %d\n", __func__, ioc_no);
		ret = -EIO;
	}

	return ret;
}

static void ddr_inspect_vma_open(struct vm_area_struct *vma)
{
	if (!vma)
		return;
}

static void ddr_inspect_vma_close(struct vm_area_struct *vma)
{
	if (!vma)
		return;
}

const struct vm_operations_struct g_ddr_insp_vm_ops = {
	.open	= ddr_inspect_vma_open,
	.close	= ddr_inspect_vma_close,
};

static int ddr_inspect_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = 0;
	unsigned long size;
	unsigned long max_size;
	unsigned long req_idx;
	struct ddr_inspect_dev *ddr_insp_dev = NULL;

	if (!filp || !vma)
		return -EINVAL;

	ddr_insp_dev = (struct ddr_inspect_dev *)filp->private_data;
	size = sizeof(struct ddr_inspect_vma) * ddr_insp_dev->mem_req_num;
	max_size = round_up_to_page(PAGE_NUM_ONE_MB * size);
	max_size <<= PAGE_SHIFT;

	if (vma->vm_end - vma->vm_start > max_size) {
		pr_err("mmap error:  %lx > %lx", vma->vm_end - vma->vm_start,
			max_size);
		return -EINVAL;
	}

	for (req_idx = 0; req_idx < ddr_insp_dev->mem_req_num; req_idx++) {
		struct ddr_inspect_vma *insp_vma =
			&ddr_insp_dev->req_vma[req_idx];

		if (insp_vma->req_pfn != 0) {
			flush_tlb_all();
			ret = remap_pfn_range(vma,
				vma->vm_start + insp_vma->vm_offset,
				insp_vma->req_pfn, PAGE_SIZE * PAGE_NUM_ONE_MB,
				vma->vm_page_prot);
			flush_tlb_all();

			if (ret) {
				pr_err("%s %u error %d", __func__,
					insp_vma->req_pfn, ret);
				return ret;
			}
		}
	}

	vma->vm_flags |= VM_LOCKED;
	vma->vm_ops = &g_ddr_insp_vm_ops;
	vma->vm_private_data = ddr_insp_dev;

	ddr_inspect_vma_open(vma);

	return ret;
}

const struct file_operations g_ddr_insp_fops = {
	.owner		= THIS_MODULE,
	.open		= ddr_inspect_open,
	.unlocked_ioctl = ddr_inspect_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = ddr_inspect_ioctl,
#endif
	.llseek		= no_llseek,
	.read		= ddr_inspect_read,
	.mmap		= ddr_inspect_mmap,
	.release	= ddr_inspect_release,
};

