/* Module internals
 *
 * Copyright (C) 2016 Hisilicon, Inc. All Rights Reserved.
 *
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of
 * Hisilicon Inc. and its licensors, and are licensed to the recipient
 * under the terms of a separate license agreement.  They may be
 * adapted and modified by bona fide purchasers under the terms of the
 * separate license agreement for internal use, but no adapted or
 * modified version may be disclosed or distributed to third parties
 * in any manner, medium, or form, in whole or in part, without the
 * prior written consent of Hisilicon Inc.
 */

#ifndef _IPU_SMMU_DRV_H
#define _IPU_SMMU_DRV_H

#include <linux/genalloc.h>
#include <linux/iommu.h>
#include <linux/mutex.h>
#include <linux/scatterlist.h>
#include <linux/hisi/hisi_ion.h>

#define IPU_SMMU_ENABLE
#define CAMBRICON_IPU_IRQ

#define IPU_BASE_ADDRESS 0xff400000
#define IPU_SMMU_MSTR_BASE (IPU_BASE_ADDRESS + 0xA0000)
#define IPU_SMMU_COMM_BASE (IPU_BASE_ADDRESS + 0x80000)
#define IPU_SMMU_MSTR_BASE_ES (IPU_BASE_ADDRESS + 0x84000)
#define IPU_SMMU_COMM_BASE_ES (IPU_BASE_ADDRESS + 0x80000)
#define IPU_SMMU_READ_STREAM_NUMBER (3)
#define IPU_SMMU_TAG_COMPARE_CASE_NUMBER (6)
#define IPU_SMMU_TOTAL_STREAM_ID_NUMBER (4)

#ifdef CONFIG_HUAWEI_DSM
#define REGISTER_INFO_MAX_LEN		(1024)
extern char register_info[REGISTER_INFO_MAX_LEN];
#endif

struct map_format {
	unsigned long iova_start;
	unsigned long iova_size;
	int prot;
};

struct map_data {
	int share_fd;
	struct map_format format;
};

struct memory_manage_node {
	struct list_head head;
	struct map_data map;
};

struct smmu_irq_count {
	unsigned int mstr_wdata_burst;
	unsigned int mstr_wr_va_out_of_128byte;
	unsigned int mstr_wr_va_out_of_boundary;
	unsigned int mstr_rd_va_out_of_128byte;
	unsigned int mstr_rd_va_out_of_boundary;
	unsigned int comm_ptw_ns_stat;
	unsigned int comm_ptw_invalid_stat;
	unsigned int comm_ptw_trans_stat;
	unsigned int comm_tlbmiss_stat;
	unsigned int comm_ext_stat;
	unsigned int comm_permis_stat;
};

struct smmu_statistic {
	unsigned int read_stream_cmd_total[IPU_SMMU_READ_STREAM_NUMBER];
	unsigned int read_stream_cmd_miss[IPU_SMMU_READ_STREAM_NUMBER];
	unsigned int read_stream_data_total[IPU_SMMU_READ_STREAM_NUMBER];
	unsigned int read_stream_cmd_miss_valid;
	unsigned int read_stream_cmd_miss_pending;
	unsigned int read_stream_cmd_hit_valid_not_slide_window;
	unsigned int read_stream_cmd_hit_valid_slide_window;
	unsigned int read_stream_cmd_hit_pending_not_slide_window;
	unsigned int read_stream_cmd_hit_pending_slide_window;
	unsigned int read_stream_cmd_latency;
	unsigned int write_stream_cmd_total;
	unsigned int write_stream_cmd_miss;
	unsigned int write_stream_data_total;
	unsigned int write_stream_cmd_miss_valid;
	unsigned int write_stream_cmd_miss_pending;
	unsigned int write_stream_cmd_hit_valid_not_slide_window;
	unsigned int write_stream_cmd_hit_valid_slide_window;
	unsigned int write_stream_cmd_hit_pending_not_slide_window;
	unsigned int write_stream_cmd_hit_pending_slide_window;
	unsigned int write_stream_cmd_latency;
	struct smmu_irq_count smmu_irq_count;
};

extern struct ion_client* ipu_ion_client;

extern void ipu_smmu_init(unsigned long ttbr0, unsigned long smmu_rw_err_phy_addr, bool port_sel, bool hardware_start);
extern void ipu_smmu_deinit(void);
extern unsigned long ipu_get_smmu_base_phy(struct device *dev);
extern long ipu_smmu_map(struct map_data *map);
extern long ipu_smmu_unmap(struct map_data *map);
extern bool ipu_smmu_interrupt_handler(struct smmu_irq_count *irq_count);
extern void ipu_smmu_reset_statistic(void);
extern void ipu_smmu_record_statistic(struct smmu_statistic *statistic);
extern void ipu_smmu_pte_update(void);
extern bool ipu_smmu_master_get_offset(struct device *dev);
extern bool ipu_smmu_common_get_offset(struct device *dev);
extern void ipu_smmu_override_prefetch_addr(unsigned long reset_va);
extern int ipu_smmu_mngr_init(void);
extern void ipu_smmu_mngr_deinit(void);
extern void ipu_mem_mngr_init(void);
extern void ipu_mem_mngr_deinit(unsigned long *reset_va);
extern void * ipu_mem_mngr_add(struct map_data *map);
extern int ipu_mem_mngr_del(struct map_data *map);
extern int ipu_mem_mngr_check_valid(unsigned int inst_addr);
extern void ipu_mem_mngr_dump(void);
extern void ipu_smmu_dump_strm(void);

#endif
