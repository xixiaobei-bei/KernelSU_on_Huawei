/*
 * dump.c
 *
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "dump.h"
#include <linux/mm.h>
#include <linux/sizes.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/stat.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/highmem.h>
#include <linux/uaccess.h>
#include <linux/memblock.h>
#include <linux/hisi/util.h>
#include <linux/hisi/reset.h>
#include <pr_log.h>
#include <securec.h>
#include <global_ddr_map.h>

#include "blackbox/rdr_print.h"
#include "blackbox/rdr_inner.h"

#define PR_LOG_TAG DUMP_TAG
#define TRANSFER_BASE 16
#define STRINGLEN 9
#define MAX_LEN_OF_RSTLOGADDR_STR 30
#define MAX_MEMDUMP_NAME 16
#define MEMDUMP_ADDR_LEN 10
#define MEMDUMP_END_LEN 10
#define MEMDUMP_SIZE_LEN 10
#define MEMDUMP_SKP_ADDR 33
#define MEMDUMP_SKP_LEN  10
#define MEMDUMP_FASTBOOT_FLAG_LEN 6
#define MEMDUMP_RESIZE_FLAG_ADDR 40
#define MEMDUMP_RESIZE_FLAG_LEND 12

#define memdump_remap(phys_addr, size) memdump_remap_type((phys_addr), (size), PAGE_KERNEL)
#define memdump_unmap(vaddr) vunmap((void *)(uintptr_t)(((uintptr_t)(vaddr)) & (~(PAGE_SIZE - 1))))

unsigned int g_dump_flag;
unsigned int g_ddr_size = 0x80000000; /* just for compile */
char g_core_flag[STRINGLEN];
struct proc_dir_entry *g_core_trace = NULL;
struct proc_dir_entry *g_core_flag_file = NULL;
/* dump file information, set to file->private_data */
struct dump_info {
	void *p; /* dump region phy/virtual address */
	loff_t size; /* dump region size */
};
/* Save the address information of the abnormal area transferred by the fastboot */
static phys_addr_t g_memdump_addr = 0;
static phys_addr_t g_memdump_end = 0;
static unsigned int g_memdump_size = 0;
static unsigned int g_skp_flag = 0;
static u64 g_resize_addr = 0;

static DEFINE_MUTEX(g_memdump_mutex);
struct memdump {
	char name[MAX_MEMDUMP_NAME];
	unsigned long base;
	unsigned long size;
};

void create_hisi_kdump_gzip_log_file(void)
{
	struct file *fp = NULL;

	fp = filp_open(KDUMP_DONE_FILE, O_CREAT | O_RDWR | O_APPEND, FILE_LIMIT);
	if (IS_ERR(fp) || (fp == NULL)) {
		BB_PRINT_ERR("%s():%d:open %s fail!\n", __func__, __LINE__, KDUMP_DONE_FILE);
		return;
	}

	filp_close(fp, NULL);
}

static void *memdump_remap_type(uintptr_t phys_addr, size_t size, pgprot_t pgprot)
{
	int i;
	u8 *vaddr = NULL;
	int npages =
		PAGE_ALIGN((phys_addr & (PAGE_SIZE - 1)) + size) >> PAGE_SHIFT;
	uintptr_t offset = phys_addr & (PAGE_SIZE - 1);
	struct page **pages = NULL;

	pages = vmalloc(sizeof(*pages) * npages);
	if (pages == NULL) {
		BB_PRINT_ERR("%s: vmalloc return NULL!\n", __func__);
		return NULL;
	}
	pages[0] = phys_to_page(phys_addr);
	for (i = 0; i < npages - 1; i++)
		pages[i + 1] = pages[i] + (uintptr_t)1;

	vaddr = (u8 *)vmap(pages, npages, VM_MAP, pgprot);
	if (vaddr == NULL)
		BB_PRINT_ERR("%s: vmap return NULL!\n", __func__);
	else
		vaddr += offset;
	vfree(pages);
	return (void *)vaddr;
}

/* read dump file content */
static ssize_t dump_phy_mem_proc_file_read(struct file *file,
				char __user *userbuf, size_t bytes,
				loff_t *off)
{
	struct dump_info *info = NULL;
	void __iomem *p = NULL;
	ssize_t copy;

	if ((file == NULL) || (userbuf == NULL) || (off == NULL))
		return -EFAULT;

	info = (struct dump_info *)file->private_data;

	if (info == NULL) {
		BB_PRINT_ERR("the proc file don't be created in advance\n");
		return 0;
	}

	if ((*off < 0) || (*off > info->size)) {
		BB_PRINT_ERR("read offset error\n");
		return 0;
	}

	if (*off == info->size) {
		/* end of file */
		return 0;
	}

	copy = (ssize_t) min(bytes, (size_t) (info->size - *off));

	p = memdump_remap((phys_addr_t)(uintptr_t)((char *)info->p + *off), copy);
	if (p == NULL) {
		BB_PRINT_ERR("%s ioremap fail\n", __func__);
		return -ENOMEM;
	}
	mutex_lock(&g_memdump_mutex);
	if (copy_to_user(userbuf, p, copy)) {
		BB_PRINT_ERR("%s copy to user error\n", __func__);
		copy = -EFAULT;
		goto copy_err;
	}

	*off += copy;

copy_err:
	memdump_unmap(p);
	mutex_unlock(&g_memdump_mutex);
	return copy;
}

static int dump_phy_mem_proc_file_open(struct inode *inode, struct file *file)
{
	if ((inode == NULL) || (file == NULL))
		return -EFAULT;

	file->private_data = PDE_DATA(inode);

	if (!g_memdump_addr) {
		BB_PRINT_ERR("%s: linux dump is already free\r\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static int dump_phy_mem_proc_file_release(struct inode *inode,
				struct file *file)
{
	if (file == NULL)
		return -EFAULT;

	file->private_data = NULL;

	return 0;
}

static const struct file_operations dump_phy_mem_proc_fops = {
	.open = dump_phy_mem_proc_file_open,
	.read = dump_phy_mem_proc_file_read,
	.release = dump_phy_mem_proc_file_release,
};

/* create memory dump file to dump phy memory */
static void create_dump_phy_mem_proc_file(const char *name, unsigned long phy_addr,
				size_t size)
{
	struct dump_info *info = NULL;

	/* as a public interface, we should check the parameter */
	if ((name == NULL) || (phy_addr == 0) || (size == 0)) {
		BB_PRINT_ERR(
			   "%s %d parameter error : name 0x%pK phy_addr 0x%lx ize %lu\r\n",
			   __func__, __LINE__, name, phy_addr, size);
		return;
	}

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		BB_PRINT_ERR("%s kzalloc fail !\r\n", __func__);
		return;
	}

	info->p = (void *)(uintptr_t)(phy_addr);
	info->size = size;

	if (dfx_create_memory_proc_entry(name, S_IRUSR | S_IRGRP,
					&dump_phy_mem_proc_fops, info) == NULL)
		kfree(info);
}

static ssize_t dump_end_proc_read(struct file *file, char __user *userbuf,
				  size_t bytes, loff_t *off)
{
	phys_addr_t addr;
	struct page *page = NULL;

	mutex_lock(&g_memdump_mutex);
	if (!g_memdump_addr || !g_memdump_size) {
		mutex_unlock(&g_memdump_mutex);
		return -EFAULT;
	}

	for (addr = g_memdump_addr; addr < (g_memdump_addr + g_memdump_size);
	     addr += PAGE_SIZE) {
		page = pfn_to_page(addr >> PAGE_SHIFT);
		if (PageReserved(page))
			free_reserved_page(page);
		else
			pr_err("%s page is not reserved\n", __func__);
	}

	memblock_free(g_memdump_addr, g_memdump_size);

	pr_err
		("%s:g_memdump_addr=0x%x, g_memdump_end=0x%x,g_memdump_size=0x%x\n", __func__,
		 (unsigned int)g_memdump_addr, (unsigned int)g_memdump_end,
		 g_memdump_size);
	pr_info("%s:addr is %lu\n", __func__, (unsigned long)addr);
	g_memdump_addr = 0;
	g_memdump_end = 0;
	g_memdump_size = 0;

	mutex_unlock(&g_memdump_mutex);
	return 0;
}

static const struct file_operations dump_end_proc_fops = {
	.read = dump_end_proc_read,
};


#define MEMDUMPADDR_CMDLINE_PARAMS_NUM 5
static int early_parse_memdumpaddr_cmdline(char *p)
{
	int ret;
	char *ptr = NULL;

	if (p == NULL)
		return -1;

	ptr = p;
	while (*ptr != 0) {
		if (*ptr == '+')
			*ptr = ' ';
		ptr++;
	}

	ret = sscanf_s(p, "%llx %llx %x %x %llx",
		&g_memdump_addr, &g_memdump_end, &g_memdump_size, &g_skp_flag, &g_resize_addr);
	if (ret != MEMDUMPADDR_CMDLINE_PARAMS_NUM) {
		pr_err("[%s] p:%s sscanf parse error!\n", __func__, p);
		return -1;
	}
	pr_err("[%s] p:%s, g_memdump_addr:0x%lx, g_memdump_end:0x%lx, g_memdump_size:0x%x, "
		"g_skp_flag:0x%x, g_resize_addr:0x%llx\n",
		__func__, (const char *)p, (unsigned long)g_memdump_addr,
		(unsigned long)g_memdump_end, g_memdump_size, g_skp_flag, g_resize_addr);

	return 0;
}
early_param("memdump_addr", early_parse_memdumpaddr_cmdline);


u64 skp_skp_resizeaddr(void)
{
	return g_resize_addr;
}

unsigned int skp_skp_flag(void)
{
	return g_skp_flag;
}

static int memdump_dts_verify(void)
{
	struct device_node *np = NULL;
	const char *out = NULL;
	int ret;

	np = of_find_node_by_path("/reserved-memory/kerneldump");
	if (!np) {
		BB_PRINT_ERR("[%s], cannot find kerneldump node in dts!\n", __func__);
		return -ENODEV;
	}

	ret = of_property_read_string(np, "status", &out);
	if (ret) {
		BB_PRINT_ERR("[%s], cannot find status in dts!\n", __func__);
		return ret;
	}

	if (!out) {
		BB_PRINT_ERR("[%s], cannot find status value in dts!\n", __func__);
		return -1;
	}

	if (strncmp("ok", out, sizeof("ok")) != 0) {
		BB_PRINT_ERR("[%s], kerneldump status is:%s\n", __func__, out);
		return -1;
	}

	return 0;
}

static int memdump_mem_verify(void)
{
	if (g_memdump_addr == HISI_RESERVED_KERNEL_DUMP_PROTECT_BASE &&
	    g_memdump_size == HISI_RESERVED_KERNEL_DUMP_PRORECT_SIZE)
		return 0;

	return -1;
}

static int memdump_verify(void)
{
	if (memdump_dts_verify())
		return -1;

	return memdump_mem_verify();
}

static int __init memdump_init(void)
{
	struct memdump *mem_info = NULL;
	void __iomem *memdump_head = NULL;

	if (!check_himntn(HIMNTN_GOBAL_RESETLOG))
		return 0;
	if (g_memdump_addr == 0)
		return 0;

	if (memdump_verify()) {
		BB_PRINT_ERR("memdump check fail\n");
		return 0;
	}

	/* to free the reserve mem of memdump */
	if (dfx_create_memory_proc_entry("dump_end", S_IRUSR | S_IRGRP,
					    &dump_end_proc_fops, NULL) == NULL)
		return 0;

	memdump_head = memdump_remap(g_memdump_addr, PAGE_SIZE);
	if (memdump_head == NULL) {
		BB_PRINT_ERR("memdump_remap fail,g_memdump_addr is 0x%llx",
			     g_memdump_addr);
		return 0;
	}

	mem_info = (struct memdump *)memdump_head;

	while (mem_info->name[0] != 0) {
		pr_err("%s,name:%s\n", __func__, mem_info->name);
		pr_err("%s:base:0x%lx, size:0x%lx\n", __func__,
			   mem_info->base, mem_info->size);
		create_dump_phy_mem_proc_file(mem_info->name, mem_info->base,
					     (size_t) mem_info->size);
		mem_info++;
	}

	memdump_unmap(memdump_head);

	return 0;
}

arch_initcall(memdump_init);
