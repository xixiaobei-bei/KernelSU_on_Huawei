/*
 * ddr_info.c
 *
 * for ddr operation
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
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

#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/seq_file.h>
#include <linux/printk.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/of.h>
#include "soc_sctrl_interface.h"
#include "soc_acpu_baseaddr_interface.h"

#define DDR_INFO_ADDR (SOC_SCTRL_SCBAKDATA7_ADDR(SOC_ACPU_SCTRL_BASE_ADDR))

/*
 * DDR_INFO_ADDR is the physical address of the register for storing
 * DDR manufacturer id and memory size.the register vlaue is conveyed
 * from xloader to Linux kernel,defined in "xloader/ddr/ddr_asic.h",
 * and named  SYSCTRL_DDRINFO.
 */

/*
 * getting the DDR manufacturer id and memory size from xloader.
 * RETURN VALUE:
 *   0 -- error.
 *   nonzero value -- the register value,DDR manufacturer id and memory size.
 */
static unsigned int get_ddr_info(void)
{
	unsigned int tmp_reg_value;

	unsigned long *virtual_addr = (unsigned long *)ioremap_nocache
		(DDR_INFO_ADDR & 0xFFFFF000, 0x800);

	if (virtual_addr == NULL) {
		pr_err("%s  ioremap ERROR !!\n", __func__);
		return 0;
	}
	pr_info("ddr_info va = 0x%lx\n", (uintptr_t)virtual_addr);

	tmp_reg_value = *(unsigned long *)((uintptr_t)virtual_addr +
		(DDR_INFO_ADDR & 0x00000FFF));
	iounmap(virtual_addr);
	return tmp_reg_value;
}

static int ddr_info_show(struct seq_file *m, void *v)
{
	unsigned int tmp_reg_value;

	tmp_reg_value = get_ddr_info();

	tmp_reg_value = tmp_reg_value & 0xFFF;
	seq_printf(m, "ddr_info:\n0x%x\n", tmp_reg_value);
	return 0;
}

static int ddrinfo_open(struct inode *inode, struct file *file)
{
	return single_open(file, ddr_info_show, NULL);
}

static const struct file_operations proc_ddrinfo_operations = {
	.open		= ddrinfo_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_ddr_info_init(void)
{
	void *ret = NULL;

	ret = proc_create("ddr_info", 0644, NULL, &proc_ddrinfo_operations);
	if (ret == NULL)
		pr_err("%s /proc/ddr_info init ERROR !!\n", __func__);

	return 0;
}

module_init(proc_ddr_info_init);
