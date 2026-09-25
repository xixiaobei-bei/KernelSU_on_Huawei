/*
 * file: bfm_mtk.c
 *
 * BFM (Boot Fail Monitor) on MTK
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/stat.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kprobes.h>
#include <linux/reboot.h>
#include <linux/io.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <asm/barrier.h>
#include <linux/platform_device.h>
#include <linux/of_fdt.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/of_address.h>
#include <linux/kallsyms.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/syscalls.h>
#include <soc/mediatek/log_store_kernel.h>
#include <chipset_common/bfmr/public/bfmr_public.h>
#include <chipset_common/bfmr/bfm/core/bfm_core.h>
#include <chipset_common/bfmr/bfm/chipsets/mtk/bfm_mtk.h>

struct boot_context *boot_cxt;

#define OFFSET_TAG_SIZE 8
#define OFFSET_TAG "PAD0"

static u32 hwboot_calculate_checksum(unsigned char *addr, u32 len)
{
	int i;
	uint8_t *w = addr;
	u32 sum = 0;

	for (i = 0; i < len; i++)
		sum += *w++;

	return sum;
}

void set_boot_stage(enum bfmr_detail_stage stage)
{
	if (boot_cxt != NULL) {
		boot_cxt->boot_stage = stage;
		pr_info("%s: boot_stage = 0x%08x\n", __func__,
			boot_cxt->boot_stage);
		boot_cxt->hash_code = hwboot_calculate_checksum((u8 *)boot_cxt,
			BOOT_LOG_CHECK_SUM_SIZE);
	}
}
EXPORT_SYMBOL(set_boot_stage);

u32 get_boot_stage(void)
{
	if (boot_cxt != NULL) {
		pr_info(" boot_stage = 0x%08x\n", boot_cxt->boot_stage);
		return boot_cxt->boot_stage;
	}
	pr_err("get boot stage fail\n");
	return 0;
}
EXPORT_SYMBOL(get_boot_stage);

int set_boot_fail_flag(enum bfm_errno_code bootfail_errno)
{
	int ret = -1;

	if (boot_cxt == NULL)
		return ret;
	if (!boot_cxt->boot_error_no) {
		boot_cxt->boot_error_no = bootfail_errno;
		boot_cxt->hash_code = hwboot_calculate_checksum((u8 *)boot_cxt,
			BOOT_LOG_CHECK_SUM_SIZE);
		pr_info(" boot_error_no = 0x%08x\n", boot_cxt->boot_error_no);
		ret = 0;
	}
	return ret;
}
EXPORT_SYMBOL(set_boot_fail_flag);

void  hwboot_fail_init_struct(void)
{
	/* init lk log structure */
	struct sram_log_header *bf_sram_header = NULL;

	bf_sram_header = ioremap_wc(CONFIG_MTK_DRAM_LOG_STORE_ADDR,
		CONFIG_MTK_DRAM_LOG_STORE_SIZE);

	if (bf_sram_header->sig != SRAM_HEADER_SIG) {
		pr_err("bf_sram_header is not match\n");
		return;
	}

	boot_cxt = (struct boot_context *)&bf_sram_header->boot_cxt;

	if (boot_cxt->boot_magic != HWBOOT_MAGIC_NUMBER) {
		pr_err("boot_cxt structure is err\n");
		boot_cxt = NULL;
		return;
	}

	if (boot_cxt != NULL)
		set_boot_stage(KERNEL_STAGE_START);
	else
		pr_notice("hwboot: bootlog is null\n");
}
EXPORT_SYMBOL(hwboot_fail_init_struct);

void hwboot_clear_magic(void)
{
	boot_cxt->boot_magic = 0;
}
EXPORT_SYMBOL(hwboot_clear_magic);
