/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
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

#ifndef PHYS_MEM__H
#define PHYS_MEM__H

#include <linux/ioctl.h>
#include "soc_sctrl_interface.h"
#include "soc_acpu_baseaddr_interface.h"

/*
 * dynamic major by default.
 * 0 == dynamic allocation of the major number
 */
#define PHYS_MEM_MAJOR	0

#define MEMDEV_NR_DEVS	1

// Use free page claimer
#define SOURCE_FREE_PAGE	0x00001
/*
 * Use the 'alloc_conti_pages' Claimer
 */
#define SOURCE_FREE_CONTI_PAGE	0x00002

/* Not a source but the reply for invalid (too large) PFNs */
#define SOURCE_INVALID_PFN	0x80000

/* Failed to insert Page in VMA */
#define SOURCE_ERROR_NOT_MAPPABLE	0x100000

#define SOURCE_MASK			0x000FFFFF
#define SOURCE_ERROR_MASK		0xFFF00000

#define pfn_is_claimed(phys_mem_frame_status) \
	(((phys_mem_frame_status)->actual_source & SOURCE_MASK) != 0)

#define PRE_IOSLATE_NUM	1
/* A single request for a single pfn */
struct phys_mem_frame_request {
	unsigned long requested_pfn;
	unsigned long allowed_sources; /* Bitmask of SOURCE_* */
};

#define SEEKSET	0
#define SEEKCUR	1
#define SEEKEND	2

/* The status of a pfn */
struct phys_mem_frame_status {
	struct phys_mem_frame_request request;
	unsigned long long vma_offset_of_first_byte;
	unsigned long pfn;
	unsigned long actual_source;
	struct page *page;
};

/* The different configurable parameters */
extern int g_phys_mem_devs;
extern int phys_mem_order;
extern int phys_mem_qset;

/*
 * Ioctl definitions, for checking
 * The protocol_version version of this IOCTL call.
 * Must be IOCTL_REQUEST_VERSION
 */

#define IOCTL_REQUEST_VERSION	1

struct phys_mem_request {
	unsigned long protocol_version;
	unsigned long conti_pageframe_cnt;
	unsigned long num_requests;
	struct phys_mem_frame_request *req;
};

struct mark_page_poison {
	unsigned long protocol_version;
	unsigned long bad_pfn;
};

#define DDR_INFO_ADDR (SOC_SCTRL_SCBAKDATA7_ADDR(SOC_ACPU_SCTRL_BASE_ADDR))
#define STR_DDR_INFO_LEN	64
struct ddr_info {
	unsigned long protocol_version;
	char ddr_info_str[STR_DDR_INFO_LEN];
};

#define NVE_DATA_BYTE_SIZE	64
struct user_nve_info {
	unsigned long protocol_version;
	int nv_number;
	int opcode;
	unsigned char nve_data[NVE_DATA_BYTE_SIZE];
	int err_flag;
};

struct reboot_reason {
	unsigned long protocol_version;
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
	unsigned long protocol_version;
	struct ddr_memory memory;
	struct ddr_memory reserved;
};

/* Use 'K' as magic number */
#define PHYS_MEM_IOC_MAGIC	'K'

/* The 'Configure' IOCTL described above */
#define PHYS_MEM_IOC_REQUEST_PAGES \
		_IOW(PHYS_MEM_IOC_MAGIC, 0, struct phys_mem_request)

#define PHYS_MEM_IOC_MARK_FRAME_BAD \
		_IOW(PHYS_MEM_IOC_MAGIC, 1, struct mark_page_poison)

#define PHYS_MEM_IOC_GET_DDRINFO \
		_IOW(PHYS_MEM_IOC_MAGIC, 2, struct ddr_info)

#define PHYS_MEM_IOC_ACCESS_NVE \
		_IOW(PHYS_MEM_IOC_MAGIC, 3, struct user_nve_info)

#define PHYS_MEM_IOC_REBOOT_REASON \
		_IOW(PHYS_MEM_IOC_MAGIC, 4, struct reboot_reason)

#define PHYS_MEM_IOC_BBOX_RECORD \
		_IOW(PHYS_MEM_IOC_MAGIC, 5, struct user_nve_info)

#define PHYS_MEM_IOC_MEMORY \
		_IOW(PHYS_MEM_IOC_MAGIC, 6, struct ddr_memory_type)

#define PHYS_MEM_IOC_MAXNR 6

#endif
