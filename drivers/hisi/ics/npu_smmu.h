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

#ifndef _NPU_SMMU_H
#define _NPU_SMMU_H

#include <linux/platform_device.h>

#define NPU_SMMU_READ_STREAM_NUMBER (3)
#define NPU_SMMU_TAG_COMPARE_CASE_NUMBER (6)
#define NPU_SMMU_TOTAL_STREAM_ID_NUMBER (4)
#define NPU_RATE_HZ_TO_MHZ (1000000)

/* ioctl interface */
struct map_format {
	unsigned long iova_start;
	unsigned long iova_size;
	int prot;
};

struct map_data {
	int share_fd;
	struct map_format format;
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

typedef struct smmu_statistic {
	unsigned int coreID;
	unsigned int read_stream_cmd_total[NPU_SMMU_READ_STREAM_NUMBER];
	unsigned int read_stream_cmd_miss[NPU_SMMU_READ_STREAM_NUMBER];
	unsigned int read_stream_data_total[NPU_SMMU_READ_STREAM_NUMBER];
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
}SMMU_STAT_S;

struct bandwidth_manager {
	unsigned int bandwidth_lmt_strategy;
	void __iomem *noc_bus_io_addr;
	void __iomem *dmss_flux_io_addr;
};

int npu_smmu_mngr_init(struct device *dev, unsigned int bandwidth_lmt_strategy);
void npu_smmu_mngr_exit(struct device *dev);
void npu_smmu_init(unsigned int coreID);
void npu_smmu_bypass_init(unsigned int coreID);
void npu_smmu_exit(unsigned int coreID);
long npu_smmu_map(struct map_data *map);
long npu_smmu_unmap(struct map_data *map);
bool npu_smmu_interrupt_handler(unsigned int coreID);
void npu_smmu_reset_statistic(unsigned int coreID);
void npu_smmu_record_statistic(unsigned int coreID);
long npu_smmu_report_statistic(SMMU_STAT_S *statistic);
void npu_smmu_pte_update(unsigned int coreID);
void npu_smmu_override_prefetch_addr(unsigned int coreID);

void npu_mem_mngr_init(void);
void npu_mem_mngr_exit(void);
int npu_mem_mngr_check_valid(unsigned int inst_addr, unsigned int inst_size);
void npu_mem_mngr_dump(void);
void npu_smmu_dump_strm(unsigned int coreID);
long npu_smmu_reset_virt(struct map_data *map);
void npu_smmu_set_stat_en(int enable);
int npu_smmu_get_stat_en(void);

void npu_bandwidth_lmt_config(unsigned int coreID, void *npu_irq_io_addr, unsigned int npu_rate, unsigned int vcodecbus_rate);
void npu_bandwidth_lmt_reset(unsigned int coreID);

#endif

