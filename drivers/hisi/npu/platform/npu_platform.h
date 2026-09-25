/*
 * npu_platform.h
 *
 * about npu platform
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
#ifndef __NPU_PLATFORM_H
#define __NPU_PLATFORM_H
#include "npu_common.h"
#include "npu_platform_resource.h"

#define DEVDRV_SC_TESTREG_INIT          0
#define DEVDRV_SC_TESTREG_TS_READY      0x06
#define DEVDRV_SC_TESTREG_AI_CPU_READY  0x5A
#define DEVDRV_SC_TESTREG_AI_CPU_BUSY   0x6B
#define DEVDRV_SC_TESTREG_AI_CPU_ERROR  0xFF

#define DEVDRV_PLAT_TYPE_FPGA   0x0
#define DEVDRV_PLAT_TYPE_EMU    0x1
#define DEVDRV_PLAT_TYPE_ESL    0x2
#define DEVDRV_PLAT_TYPE_ASIC   0x3
#define DEVDRV_PLAT_VERSION_MASK       0x00000FFF
#define DEVDRV_PLAT_AI_CORE_NUM_MASK   0x0000F000
#define DEVDRV_PLAT_MASK               0x000F0000
#define DEVDRV_PLAT_AI_CORE_NUM_2      0x00002000
#define DEVDRV_PLAT_AI_CORE_NUM_1      0x00000000

#define DEVDRV_PLAT_DEVICE      0
#define DEVDRV_PLAT_HOST        1

#define DEVDRV_RESMEM_MAX_RESOURCE 9
#define DEVDRV_DFX_CHANNEL_MAX_RESOURCE 4

enum devdrv_register_index {
	DEVDRV_REG_GIC_BASE,
	DEVDRV_REG_TS_SUBSYSCTL,
	DEVDRV_REG_TS_DOORBELL,
	DEVDRV_REG_TS_SRAM,
	DEVDRV_REG_SYSCTL,
	DEVDRV_REG_L2BUF_BASE,
	DEVDRV_REG_PERICRG_BASE,
	DEVDRV_REG_HWTS_BASE,
	DEVDRV_REG_AIC0_BASE,
	DEVDRV_REG_AIC1_BASE,
	DEVDRV_REG_MAX_RESOURCE
};

enum devdrv_feature_index {
	DEVDRV_FEATURE_AUTO_POWER_DOWN,
	DEVDRV_FEATURE_NPU_PM_INTF,
	DEVDRV_FEATURE_SVM_MEM,
	DEVDRV_FEATURE_GIC_MULTICHIP,
	DEVDRV_FEATURE_KMALLOC,
	DEVDRV_FEATURE_TASK_DOWN,
	DEVDRV_FEATURE_HWTS,
	DEVDRV_FEATURE_SEC_PLAT,
	DEVDRV_FEATURE_KERNEL_LOAD_IMG,
	DEVDRV_FEATURE_GIC_SUPPORT,
	DEVDRV_FEATURE_MAX_RESOURCE
};

enum devdrv_feature_switch {
	DEVDRV_FEATURE_OFF,
	DEVDRV_FEATURE_ON
};

enum devdrv_irq_index {
	DEVDRV_IRQ_CALC_CQ_UPDATE0,
	DEVDRV_IRQ_DFX_CQ_UPDATE,
	DEVDRV_IRQ_MAILBOX_ACK
};

enum devdrv_dfx_dev_index {
	DEVDRV_DFX_DEV_LOG,
	DEVDRV_DFX_DEV_PROFILE,
	DEVDRV_DFX_DEV_BLACKBOX,
	DEVDRV_DFX_DEV_MAX_RESOURCE
};

enum npu_binary_idx_s {
	AICPU_EL3_S = 0,
	AICPU_EL1_S,
	AICPU_RESV_S,
	TSCPU_EL3_S,
	TSCPU_EL1_S,
	TSCPU_RESV_S,
	BINARY_IDX_S_CNT
};

struct devdrv_pm_adapter {
	int (*npu_power_up)(struct devdrv_dev_ctx *dev_ctx, u32 is_secure);
	int (*npu_open)(void);
	int (*npu_release)(void);
	int (*npu_power_down)(void *svm_dev, u32 is_secure, u32 *stage);
};

struct devdrv_resource_adapter {
	int (*npu_fw_proc)(const struct file *fp, loff_t fsize, loff_t pos, u64 fw_load_addr);
	int (*npu_irq_change_route)(u32 irq_num, u32 cpuid);
	int (*npu_cqsq_share_alloc)(u32 stream_id, void *alloc_func, u32 dev_id);
	int (*npu_trigger_irq)(u32 irq_num);
	int (*npu_mailbox_send)(void *mailbox, int mailbox_len, const void *message, int message_len);
	int (*npu_ctrl_core)(struct devdrv_dev_ctx *dev_ctx, u32 core_num);
};

struct devdrv_platform_specification {
	u32 stream_max;
	u32 event_max;
	u32 notify_max;
	u32 model_max;
	u32 aicore_max;
	u32 aicpu_max;
	u32 calc_sq_max;
	u32 calc_sq_depth;
	u32 calc_cq_max;
	u32 calc_cq_depth;
	u32 dfx_sq_max;
	u32 dfx_cq_max;
	u32 dfx_sqcq_depth;
	u32 doorbell_stride;
};

struct devdrv_mem_desc {
	u32 base;
	u32 len;
};

struct devdrv_platform_adapter {
	struct devdrv_pm_adapter pm_ops;
	struct devdrv_resource_adapter res_ops;
};

struct devdrv_dfx_desc {
	u32 channels[DEVDRV_DFX_CHANNEL_MAX_RESOURCE];
	struct devdrv_mem_desc *bufs;
	u8 channel_num;
};

struct devdrv_dts_info {
	void __iomem *reg_vaddr[DEVDRV_REG_MAX_RESOURCE];
	struct devdrv_mem_desc reg_desc[DEVDRV_REG_MAX_RESOURCE];
	u32 feature_switch[DEVDRV_FEATURE_MAX_RESOURCE];
	struct devdrv_dfx_desc dfx_desc[DEVDRV_DFX_DEV_MAX_RESOURCE];
	struct devdrv_mem_desc resmem_desc[DEVDRV_RESMEM_MAX_RESOURCE];
	struct devdrv_mem_desc dump_region_desc[DEVDRV_DUMP_REGION_MAX];
	int irq_cq_update;

	int irq_mailbox_ack;
	int irq_dfx_cq;
	u32 aicpu_cluster;
	u32 aicpu_core;
	u32 tscpu_cluster;
	u32 tscpu_core;
	u32 gic0_spi_blk_num;
	struct devdrv_mem_desc *tsfw_buf;
	struct devdrv_mem_desc *aifw_buf;
	struct devdrv_mem_desc *sqcq_buf;
	struct devdrv_mem_desc *hwts_sq_buf;
	struct devdrv_mem_desc *hwts_cq_buf;
	struct devdrv_mem_desc *persistent_task_buf;
	struct devdrv_mem_desc *chip_cfg_buf;
	struct devdrv_mem_desc *info_buf;
};

struct devdrv_platform_info {
	struct devdrv_platform_adapter adapter;
	struct devdrv_platform_specification spec;
	struct devdrv_dts_info dts_info;
	struct device *pdev;
	struct device *p_ts_subsys_dev;
	const char* chiptype;
	u32 hardware_version;
	u8 env_type;
	u8 plat_type;
	u8 reserved[2];
};

#define DEVDRV_PLAT_GET_CHIPTYPE(info) \
	((info)->chiptype)
#define DEVDRV_PLAT_GET_HARDWARE(info) \
	((info)->hardware_version)
#define DEVDRV_PLAT_GET_ENV(info) \
	((info)->env_type)
#define DEVDRV_PLAT_GET_TYPE(info) \
	((info)->plat_type)
#define DEVDRV_PLAT_GET_PDEV(info) \
	((info)->pdev)
#define DEVDRV_PLAT_GET_PSMMUDEV(info) \
	((info)->p_ts_subsys_dev)

#define DEVDRV_PLAT_GET_ADAPTER(info) \
	((info)->adapter)
#define DEVDRV_PLAT_GET_PM_OPS(info) \
	(DEVDRV_PLAT_GET_ADAPTER(info).pm_ops)
#define DEVDRV_PLAT_GET_PM_OPEN(info) \
	(DEVDRV_PLAT_GET_PM_OPS(info).npu_open)
#define DEVDRV_PLAT_GET_PM_POWER_UP(info) \
	(DEVDRV_PLAT_GET_PM_OPS(info).npu_power_up)
#define DEVDRV_PLAT_GET_PM_RELEASE(info) \
	(DEVDRV_PLAT_GET_PM_OPS(info).npu_release)
#define DEVDRV_PLAT_GET_PM_POWER_DOWN(info) \
(DEVDRV_PLAT_GET_PM_OPS(info).npu_power_down)

#define DEVDRV_PLAT_GET_RES_OPS(info) \
	(DEVDRV_PLAT_GET_ADAPTER(info).res_ops)
#define DEVDRV_PLAT_GET_RES_FW_PROC(info) \
	(DEVDRV_PLAT_GET_RES_OPS(info).npu_fw_proc)
#define DEVDRV_PLAT_GET_RES_CHG_ROUTE(info) \
	(DEVDRV_PLAT_GET_RES_OPS(info).npu_irq_change_route)
#define DEVDRV_PLAT_GET_RES_SQCQ_ALLOC(info) \
	(DEVDRV_PLAT_GET_RES_OPS(info).npu_cqsq_share_alloc)
#define DEVDRV_PLAT_GET_RES_TIRG_IRQ(info) \
	(DEVDRV_PLAT_GET_RES_OPS(info).npu_trigger_irq)
#define DEVDRV_PLAT_GET_RES_MAILBOX_SEND(info) \
	(DEVDRV_PLAT_GET_RES_OPS(info).npu_mailbox_send)
#define DEVDRV_PLAT_GET_RES_CTRL_CORE(info) \
		(DEVDRV_PLAT_GET_RES_OPS(info).npu_ctrl_core)

#define DEVDRV_PLAT_GET_SPEC(info) \
	((info)->spec)
#define DEVDRV_PLAT_GET_STREAM_MAX(info) \
	(DEVDRV_PLAT_GET_SPEC(info).stream_max)
#define DEVDRV_PLAT_GET_EVENT_MAX(info) \
	(DEVDRV_PLAT_GET_SPEC(info).event_max)
#define DEVDRV_PLAT_GET_NOTIFY_MAX(info) \
	(DEVDRV_PLAT_GET_SPEC(info).notify_max)
#define DEVDRV_PLAT_GET_MODEL_MAX(info) \
	(DEVDRV_PLAT_GET_SPEC(info).model_max)
#define DEVDRV_PLAT_GET_AICORE_MAX(info) \
	(DEVDRV_PLAT_GET_SPEC(info).aicore_max)
#define DEVDRV_PLAT_GET_AICPU_MAX(info) \
	(DEVDRV_PLAT_GET_SPEC(info).aicpu_max)
#define DEVDRV_PLAT_GET_CALC_SQ_MAX(info) \
	(DEVDRV_PLAT_GET_SPEC(info).calc_sq_max)
#define DEVDRV_PLAT_GET_CALC_SQ_DEPTH(info) \
	(DEVDRV_PLAT_GET_SPEC(info).calc_sq_depth)
#define DEVDRV_PLAT_GET_CALC_CQ_MAX(info) \
	(DEVDRV_PLAT_GET_SPEC(info).calc_cq_max)
#define DEVDRV_PLAT_GET_CALC_CQ_DEPTH(info) \
	(DEVDRV_PLAT_GET_SPEC(info).calc_cq_depth)
#define DEVDRV_PLAT_GET_DFX_SQ_MAX(info) \
	(DEVDRV_PLAT_GET_SPEC(info).dfx_sq_max)
#define DEVDRV_PLAT_GET_DFX_CQ_MAX(info) \
	(DEVDRV_PLAT_GET_SPEC(info).dfx_cq_max)
#define DEVDRV_PLAT_GET_DFX_SQCQ_DEPTH(info) \
	(DEVDRV_PLAT_GET_SPEC(info).dfx_sqcq_depth)
#define DEVDRV_PLAT_GET_DOORBELL_STRIDE(info) \
	(DEVDRV_PLAT_GET_SPEC(info).doorbell_stride)

#define DEVDRV_PLAT_GET_DTS(info) \
	((info)->dts_info)
#define DEVDRV_PLAT_GET_FEAUTRE_SWITCH(info, feature_idx) \
	(DEVDRV_PLAT_GET_DTS(info).feature_switch[feature_idx])
#define DEVDRV_PLAT_GET_RESMEM_DESC(info, resmem_idx) \
	(DEVDRV_PLAT_GET_DTS(info).resmem_desc[resmem_idx])
#define DEVDRV_PLAT_GET_RESMEM_S_DESC(info, resmem_s_idx)  (g_resmem_s_desc[resmem_s_idx])

#define DEVDRV_PLAT_GET_REG_DESC(info, reg_idx) \
	(DEVDRV_PLAT_GET_DTS(info).reg_desc[reg_idx])
#define DEVDRV_PLAT_GET_REG_VADDR(info, reg_idx) \
	(DEVDRV_PLAT_GET_DTS(info).reg_vaddr[reg_idx])
#define DEVDRV_PLAT_GET_DUMP_REGION_DESC(info, reg_idx) \
	(DEVDRV_PLAT_GET_DTS(info).dump_region_desc[reg_idx])

#define DEVDRV_PLAT_GET_TSFW_BUF(info) \
	(DEVDRV_PLAT_GET_DTS(info).tsfw_buf)
#define DEVDRV_PLAT_GET_AIFW_BUF(info) \
	(DEVDRV_PLAT_GET_DTS(info).aifw_buf)
#define DEVDRV_PLAT_GET_SQCQ_BUF(info) \
	(DEVDRV_PLAT_GET_DTS(info).sqcq_buf)
#define DEVDRV_PLAT_GET_HWTS_SQ_BUF(info) \
	(DEVDRV_PLAT_GET_DTS(info).hwts_sq_buf)
#define DEVDRV_PLAT_GET_HWTS_CQ_BUF(info) \
	(DEVDRV_PLAT_GET_DTS(info).hwts_cq_buf)
#define DEVDRV_PLAT_GET_PERSISTENT_TASK_BUF(info) \
	(DEVDRV_PLAT_GET_DTS(info).persistent_task_buf)
#define DEVDRV_PLAT_GET_CHIP_CFG_BUF(info) \
	(DEVDRV_PLAT_GET_DTS(info).chip_cfg_buf)
#define DEVDRV_PLAT_GET_INFO_BUF(info) \
	(DEVDRV_PLAT_GET_DTS(info).info_buf)
#define DEVDRV_PLAT_GET_DFX_DESC(info, dfx_idx) \
	(DEVDRV_PLAT_GET_DTS(info).dfx_desc[dfx_idx])
#define DEVDRV_PLAT_GET_AICPU_CLUSTER(info) \
	(DEVDRV_PLAT_GET_DTS(info).aicpu_cluster)
#define DEVDRV_PLAT_GET_AICPU_CORE(info) \
	(DEVDRV_PLAT_GET_DTS(info).aicpu_core)
#define DEVDRV_PLAT_GET_TSCPU_CLUSTER(info) \
	(DEVDRV_PLAT_GET_DTS(info).tscpu_cluster)
#define DEVDRV_PLAT_GET_TSCPU_CORE(info) \
	(DEVDRV_PLAT_GET_DTS(info).tscpu_core)
#define DEVDRV_PLAT_GET_GIC0_SPI_BLK(info) \
	(DEVDRV_PLAT_GET_DTS(info).gic0_spi_blk_num)
#define DEVDRV_PLAT_GET_CQ_UPDATE_IRQ(info) \
	(DEVDRV_PLAT_GET_DTS(info).irq_cq_update)
#define DEVDRV_PLAT_GET_DFX_CQ_IRQ(info) \
	(DEVDRV_PLAT_GET_DTS(info).irq_dfx_cq)
#define DEVDRV_PLAT_GET_MAILBOX_ACK_IRQ(info) \
	(DEVDRV_PLAT_GET_DTS(info).irq_mailbox_ack)

extern struct devdrv_mem_desc g_resmem_s_desc[BINARY_IDX_S_CNT];
struct devdrv_platform_info* devdrv_plat_get_info(void);
u32* devdrv_plat_get_reg_vaddr(u32 reg_idx, u32 offset);
struct devdrv_mem_desc* devdrv_plat_get_reg_desc(u32 reg_idx);
int devdrv_npu_debug_init(void);

#endif
