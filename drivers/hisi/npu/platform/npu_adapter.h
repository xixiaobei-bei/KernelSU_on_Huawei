/*
 * npu_adapter.h
 *
 * about npu adapter
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_ADAPTER_INIT_H__
#define __NPU_ADAPTER_INIT_H__

#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/hisi/hisi_svm.h>

#include "npu_log.h"
#include "npu_common.h"

enum {
	NPU_INIT_BYPASS = 0,
	NPU_NON_BYPASS,
	NPU_BYPASS,
};

int npu_plat_pm_powerup(struct npu_dev_ctx *dev_ctx, u32 work_mode);

int npu_plat_svm_bind(struct npu_dev_ctx *dev_ctx,
	struct task_struct *task, void **svm_dev);

int npu_powerup_aicore(u64 is_secure, u32 aic_flag);

int npu_plat_init_sdma(u64 is_secure);

int npu_powerdown_aicore(u64 is_secure, u32 aic_flag);

int npu_plat_pm_open(uint32_t devid);

int npu_plat_pm_release(uint32_t devid);

int npu_plat_pm_powerdown(uint32_t devid, u32 is_secure, u32 *stage);

int npu_plat_res_mailbox_send(void *mailbox, int mailbox_len,
	const void *message, int message_len);

int npu_plat_send_ts_ctrl_core(uint32_t core_num);

int npu_plat_res_ctrl_core(struct npu_dev_ctx *dev_ctx, u32 core_num);

void __iomem *npu_plat_sram_remap(struct platform_device *pdev,
	resource_size_t sram_addr, resource_size_t sram_size);

void npu_plat_sram_unmap(struct platform_device *pdev, void *sram_addr);

int npu_plat_attach_sc(int fd, u64 offset, u64 size);

int npu_plat_set_sc_prio(u32 prio);

int npu_plat_switch_sc(u32 switch_sc);

int npu_log_init(void);

int npu_log_stop(void);

// common pm func
int npu_plat_powerup_till_npucpu(u64 is_secure);

int npu_plat_powerup_till_ts(u32 is_secure, u32 offset);

int npu_plat_powerup_smmu(struct device *dev);

int npu_plat_poweroff_smmu(uint32_t devid);

int npu_plat_powerdown_ts(u32 offset, u32 is_secure);

int npu_plat_powerdown_npucpu(u32 expect_val, u32 mode);

int npu_plat_powerup_tbu(void);

int npu_plat_powerdown_tbu(u32 aic_flag);

int npu_plat_powerdown_nputop(void);

int npu_plat_aicore_get_disable_status(u32 core_id);

int npu_plat_switch_hwts_aicore_pool(struct npu_dev_ctx *dev_ctx,
	npu_work_mode_info_t *work_mode_info, uint32_t power_status);

bool npu_plat_is_support_ispnn(void);

bool npu_plat_is_support_sc(void);

void npu_plat_mntn_reset(void);

/* define */
typedef struct tmp_log_buf_header {
	volatile unsigned int buf_read;
	/* malloc buffer length, head structure not included */
	volatile unsigned int buf_len;
	volatile unsigned int rev[14];
	volatile unsigned int buf_write;
	volatile unsigned int rev2[15];
} tmp_log_buf_header_t;
#define TMP_CACHE_LINE_LEN  64
#define TMP_LOG_DIR_LEN  128
#define TMP_LOG_BUF_HEAD sizeof(tmp_log_buf_header_t)

int npu_plat_handle_irq_tophalf(u32 irq_index);

int npu_smmu_evt_register_notify(struct notifier_block *n);
int npu_smmu_evt_unregister_notify(struct notifier_block *n);
void npu_plat_aicore_pmu_enable(uint32_t subip_set);
int npu_plat_dev_pm_suspend(void);
void npu_exception_timeout_record(struct npu_dev_ctx *dev_ctx);
int npu_plat_bypass_status(void);
#endif
