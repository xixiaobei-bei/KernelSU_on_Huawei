/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: ddr inspect register chdev head file
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

#ifndef DDR_INSPECT_DRV_H
#define DDR_INSPECT_DRV_H

#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/memblock.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_reserved_mem.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#define DRV_NR				1
#define PRE_IOSLATE_NUM		1

#define FIRST_INDEX			1
#define SECOND_INDEX		2
#define THIRD_INDEX			3
#define FORTH_INDEX			4

#define MEM_REQ_ORDER		8

#define MAX_REQ_NR			64
#define PAGE_NUM_ONE_MB		256

#define STR_DDR_INFO_LEN	64
#define MAX_RSV_NUM			3
struct ddr_inspect_vma {
	unsigned long req_pfn; /* start pfn */
	unsigned long long vm_offset;
};

struct ddr_inspect_request {
	unsigned long conti_pfn_nr;
	unsigned long req_nr;
	unsigned long req_pfn[MAX_REQ_NR];
};

struct ddr_info {
	char ddr_info_str[STR_DDR_INFO_LEN];
};

#define NVE_DATA_BYTE_SIZE	64
struct user_nve_info {
	int nv_number;
	int opcode;
	unsigned char nve_data[NVE_DATA_BYTE_SIZE];
	int err_flag;
};

struct reboot_reason {
	unsigned long wp_reboot_reason;
};

#define REBOOT_REASON_SIZE	64
struct reboot_reason_tbl {
	unsigned long reboot_reason_num;
	unsigned char reboot_reason_cmdline[REBOOT_REASON_SIZE];
};

#define MAX_NUM_MEM_SEG 128
struct memory_info {
	unsigned long base;
	unsigned long end;
};

struct ddr_memory {
	unsigned int count;
	struct memory_info memory_info[MAX_NUM_MEM_SEG];
};

struct ddr_memory_type {
	struct ddr_memory memory;
	struct ddr_memory reserved;
};

/* Use 'K' as magic number */
#define IOC_DDR_INSPECT_MAGIC	'K'

/* The 'Configure' IOCTL described above */
#define IOC_DDR_INSPECT_REQ_PAGES \
		_IOW(IOC_DDR_INSPECT_MAGIC, 0, struct ddr_inspect_request)

#define IOC_DDR_INSPECT_MARK_PAGE_BAD \
		_IOW(IOC_DDR_INSPECT_MAGIC, 1, unsigned long)

#define IOC_DDR_INSPECT_GET_DDRINFO \
		_IOW(IOC_DDR_INSPECT_MAGIC, 2, struct ddr_info)

#define IOC_DDR_INSPECT_ACCESS_NVE \
		_IOW(IOC_DDR_INSPECT_MAGIC, 3, struct user_nve_info)

#define IOC_DDR_INSPECT_REBOOT_REASON \
		_IOW(IOC_DDR_INSPECT_MAGIC, 4, struct reboot_reason)

#define IOC_DDR_INSPECT_BBOX_RECORD \
		_IOW(IOC_DDR_INSPECT_MAGIC, 5, struct user_nve_info)

#define IOC_DDR_INSPECT_MEMORY \
		_IOW(IOC_DDR_INSPECT_MAGIC, 6, struct ddr_memory_type)

#define IOC_MAXNR 6

struct ddr_inspect_region {
	char *name;
	phys_addr_t base;
	phys_addr_t size;
};

struct ddr_rsv_data {
	u16 cnt;
	u16 flag;
	u16 data[MAX_RSV_NUM];
};

struct ddr_inspect_dev {
	unsigned long mem_req_num;
	struct cdev cdev;
	struct semaphore sem;
	struct ddr_inspect_vma *req_vma;
};

extern const struct file_operations g_ddr_insp_fops;

struct ddr_insp_ioc {
	unsigned short ioc_no;
	long (*ioc_func)(struct ddr_inspect_dev *, unsigned long);
};

enum ioc_no_total {
	IOC_REQ = 0,
	IOC_MARK_BAD,
	IOC_DDRINFO,
	IOC_NVE,
	IOC_REASON,
	IOC_BBOX,
	IOC_MEMORY
};

enum fault_type {
	NORMAL_REBOOT = 0,
	AP_S_PANIC,
	AP_S_AWDT,
	AP_S_DDRC_SEC,
	AP_S_DDR_UCE_WD,
	AP_S_DDR_FATAL_INTER,
	AP_S_PRESS6S,
	AP_S_COMBINATIONKEY,
	AP_S_F2FS,
	AP_S_BL31_PANIC,
	BR_PRESS_10S,
	BFM_S_NATIVE_BOOT_FAIL,
	BFM_S_BOOT_TIMEOUT,
	BFM_S_FW_BOOT_FAIL,
	BFM_S_NATIVE_DATA_FAIL,
	LPM3_S_LPMCURST,
	LPM3_S_EXCEPTION,
	NR_ERR_TYPES
};

#endif
