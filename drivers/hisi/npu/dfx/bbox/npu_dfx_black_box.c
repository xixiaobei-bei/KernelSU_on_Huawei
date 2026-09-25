/*
 * npu_dfx_black_box.c
 *
 * about npu dfx black box
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
#include "npu_dfx_black_box.h"

#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/idr.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/notifier.h>
#include <linux/hisi/hisi_svm.h>
#include <linux/hisi/hisi_noc_modid_para.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/rdr_hisi_platform.h>

#include "npu_shm.h"
#include "npu_manager_common.h"
#include "npu_platform.h"
#include "mntn_public_interface.h"
#include "mntn_subtype_exception.h"
#include "soc_mid.h"
#include "npu_platform_register.h"
#include "npu_pm.h"

static vir_addr_t g_bbox_addr_vir = 0;
static vir_addr_t g_bbox_tslog_addr_vir = 0;
static vir_addr_t g_bbox_aicpulog_addr_vir = 0;

struct devdrv_npu_mntn_private_s *g_devdrv_npu_mntn_private = NULL;

struct rdr_exception_info_s devdrv_npu_rdr_excetption_info[] = {
	{
		.e_modid = (u32)EXC_TYPE_TS_AICORE_EXCEPTION,
		.e_modid_end = (u32)EXC_TYPE_TS_AICORE_EXCEPTION,
	.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = AICORE_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "AICORE_EXCP",
	}, {
		.e_modid = (u32)EXC_TYPE_TS_AICORE_TIMEOUT,
		.e_modid_end = (u32)EXC_TYPE_TS_AICORE_TIMEOUT,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype  = AICORE_TIMEOUT,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "AICORE_TIMEOUT",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_TS_RUNNING_EXCEPTION,
		.e_modid_end = (u32)RDR_EXC_TYPE_TS_RUNNING_EXCEPTION,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = TS_RUNNING_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "TS_RUNNING_EXCP",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_TS_RUNNING_TIMEOUT,
		.e_modid_end = (u32)RDR_EXC_TYPE_TS_RUNNING_TIMEOUT,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = TS_RUNNING_TIMEOUT,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "TS_RUNNING_TIMEOUT",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_TS_INIT_EXCEPTION,
		.e_modid_end = (u32)RDR_EXC_TYPE_TS_INIT_EXCEPTION,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = TS_INIT_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "TS_INIT_EXCP",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_AICPU_INIT_EXCEPTION,
		.e_modid_end = (u32)RDR_EXC_TYPE_AICPU_INIT_EXCEPTION,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = AICPU_INIT_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "AICPU_INIT_EXCP",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_AICPU_HEART_BEAT_EXCEPTION,
		.e_modid_end = (u32)RDR_EXC_TYPE_AICPU_HEART_BEAT_EXCEPTION,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = AICPU_HEARTBEAT_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "AICPU_HEARTBEAT_EXCP",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_NPU_POWERUP_FAIL,
		.e_modid_end = (u32)RDR_EXC_TYPE_NPU_POWERUP_FAIL,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = POWERUP_FAIL,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "POWERUP_FAIL",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_NPU_POWERDOWN_FAIL,
		.e_modid_end = (u32)RDR_EXC_TYPE_NPU_POWERDOWN_FAIL,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = POWERDOWN_FAIL,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "POWERDOWN_FAIL",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_NOC_NPU0,
		.e_modid_end = (u32)RDR_EXC_TYPE_NOC_NPU1,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = NPU_NOC_ERR,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "NPU_NOC_ERR",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_NPU_SMMU_EXCEPTION,
		.e_modid_end = (u32)RDR_EXC_TYPE_NPU_SMMU_EXCEPTION,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = SMMU_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "SMMU_EXCP",
	},
};

/* noc target flow */
#define TARGET_FLOW_DEFAULT  0xff

static struct noc_err_para_s npu_noc_para[] = {
	{
		.masterid = (u32)SOC_NPU0_MID,
		.targetflow = TARGET_FLOW_DEFAULT,
		.bus = NOC_ERRBUS_NPU,
	}, {
		.masterid = (u32)SOC_NPU1_MID,
		.targetflow = TARGET_FLOW_DEFAULT,
		.bus = NOC_ERRBUS_NPU,
	},
};

/* *******************************************************************
Description: devdrv_npu_mntn_copy_aicpulog_to_bbox
input: const char *src_addr, unsigned int* offset, unsigned int len
output: NA
return: void
******************************************************************* */
static int devdrv_npu_mntn_copy_aicpulog_to_bbox(const char *src_addr, unsigned int len)
{
	unsigned int temp_offset;
	if ((src_addr == NULL) || (len == 0) || (g_devdrv_npu_mntn_private->mntn_info.rdr_addr == NULL)) {
		NPU_DRV_ERR("input parameter is error!\n");
		return -EINVAL;
	}

	temp_offset = g_devdrv_npu_mntn_private->mntn_info.aicpu_add_offset + len;

	/* aicpu alloc size 64K */
	if (temp_offset >= DEVDRV_AICPU_BBOX_MEM_MAX || temp_offset <= 0) {
		NPU_DRV_WARN("bbox aicpu buf is full ,copy log to bbox is error! temp_offset=0x%x\n", temp_offset);
		g_devdrv_npu_mntn_private->mntn_info.aicpu_add_offset = 0;
		return -ENOMEM ;
	}

	memcpy(((char*)g_devdrv_npu_mntn_private->mntn_info.rdr_addr + DEVDRV_NPU_BBOX_MEM_MAX +
		g_devdrv_npu_mntn_private->mntn_info.aicpu_add_offset), src_addr, len);
	g_devdrv_npu_mntn_private->mntn_info.aicpu_add_offset = temp_offset;

	return 0;
}


/* *******************************************************************
Description: devdrv_npu_mntn_copy_reg_to_bbox
input: const char *src_addr, unsigned int* offset, unsigned int len
output: NA
return: void
******************************************************************* */
static int devdrv_npu_mntn_copy_reg_to_bbox(const char *src_addr, unsigned int len)
{
	unsigned int temp_offset;

	if ((src_addr == NULL) || (len == 0) || (g_devdrv_npu_mntn_private->mntn_info.rdr_addr == NULL)) {
		NPU_DRV_ERR("input parameter is error!\n");
		return -EINVAL;
	}

	temp_offset = g_devdrv_npu_mntn_private->mntn_info.bbox_addr_offset + len;

	/* npu_bbox alloc size 256K */
	if (temp_offset >= DEVDRV_NPU_BBOX_MEM_MAX  - sizeof(exce_module_info_t) || temp_offset <= 0) {
		NPU_DRV_WARN("bbox buf is full ,copy log to bbox is error! temp_offset=0x%x\n", temp_offset);
		g_devdrv_npu_mntn_private->mntn_info.bbox_addr_offset = 0;
		return -ENOMEM;
	}

	memcpy(((char*)g_devdrv_npu_mntn_private->mntn_info.rdr_addr +
		g_devdrv_npu_mntn_private->mntn_info.bbox_addr_offset), src_addr, len);
	g_devdrv_npu_mntn_private->mntn_info.bbox_addr_offset = temp_offset;

	return 0;
}


/* *******************************************************************
Description: devdrv_npu_mntn_write_peri_reg_info
input:	char *file_path
output:	NA
return:	void
******************************************************************* */
static void devdrv_npu_mntn_write_peri_reg_info(unsigned int core_id)
{
	char log_buf[DEVDRV_NPU_BUF_LEN_MAX + 1] = {0};
	struct devdrv_npu_peri_reg_s *peri_reg = &g_devdrv_npu_mntn_private->reg_info[core_id].peri_reg;

	snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX,
		"NPU exception info : peri_stat=%x, ppll_select=%x, power_stat=%x,"
		"power_ack=%x, reset_stat=%x, perclken=%x, perstat=%x\r\n",
		peri_reg->peri_stat, peri_reg->ppll_select, peri_reg->power_stat,
		peri_reg->power_ack, peri_reg->reset_stat,
		peri_reg->perclken0, peri_reg->perstat0);

	devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
	return;
}


/* *******************************************************************
Description: devdrv_npu_mntn_write_mstr_reg_info
input:	char *file_path
output:	NA
return:	void
******************************************************************* */
static void devdrv_npu_mntn_write_mstr_reg_info(unsigned int core_id)
{
	char log_buf[DEVDRV_NPU_BUF_LEN_MAX + 1] = {0};
	struct devdrv_npu_mstr_reg_s *mstr_reg = &g_devdrv_npu_mntn_private->reg_info[core_id].mstr_reg;

	snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX,
		"NPU exception info : RD_BITMAP=%x, WR_BITMAP=%x, rd_cmd_total_cnt[0-3]={%x, %x, %x},"
		" wr_cmd_total_cnt=%x\n", mstr_reg->rd_bitmap, mstr_reg->wr_bitmap,
		mstr_reg->rd_cmd_total_cnt0, mstr_reg->rd_cmd_total_cnt0,
		mstr_reg->rd_cmd_total_cnt2, mstr_reg->wr_cmd_total_cnt);

	devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

	return;
}


/* *******************************************************************
Description: devdrv_npu_mntn_write_reg_log
input:	void
output:	NA
return:	void
******************************************************************* */
static void devdrv_npu_mntn_write_reg_log(unsigned int core_id)
{
	char log_buf[DEVDRV_NPU_BUF_LEN_MAX + 1] = {0};
	unsigned int modid;

	memset((char*)g_devdrv_npu_mntn_private->mntn_info.rdr_addr, 0,
		g_devdrv_npu_mntn_private->mntn_info.npu_ret_info.log_len);

	snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "\r\nNPU exception info : core_id = 0x%x\r\n", core_id);
	devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

	snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : exception_code =0x%x\r\n",
		g_devdrv_npu_mntn_private->mntn_info.dump_info.modid);
	devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
	snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : interrupt_status=0x%x\r\n",
		g_devdrv_npu_mntn_private->exc_info[core_id].interrupt_status);
	devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
	snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : ip=0x%x, ret=0x%x\r\n",
		g_devdrv_npu_mntn_private->exc_info[core_id].target_ip,
		g_devdrv_npu_mntn_private->exc_info[core_id].result);
	devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

	modid = g_devdrv_npu_mntn_private->mntn_info.dump_info.modid;
	switch (modid) {
	case (unsigned int)EXC_TYPE_TS_AICORE_EXCEPTION:
		snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : aicore exception\r\n");
		break;
	case (unsigned int)EXC_TYPE_TS_AICORE_TIMEOUT:
		snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : aicore timeout\r\n");
		break;
	case (unsigned int)EXC_TYPE_TS_SDMA_EXCEPTION:
		snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : sdma exception\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_TS_RUNNING_EXCEPTION:
		snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : ts running exception\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_TS_RUNNING_TIMEOUT:
		snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : ts running timeout\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_OS_EXCEPTION:
		snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : OS running exception\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_TS_INIT_EXCEPTION:
		snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : ts init exception\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_AICPU_INIT_EXCEPTION:
		snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : aicpu init exception\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_AICPU_HEART_BEAT_EXCEPTION:
		snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : aicpu heart beat exception\r\n");
		break;
	default:
		snprintf(log_buf, DEVDRV_NPU_BUF_LEN_MAX, "NPU exception info : no exception\r\n");
		break;
	}
	devdrv_npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
	devdrv_npu_mntn_write_peri_reg_info(core_id);
	devdrv_npu_mntn_write_mstr_reg_info(core_id);
	return;
}

static void devdrv_npu_mntn_dump_aicpulog_work(void)
{
	uint32_t log_len;
	struct exc_module_info_s *exception_info = NULL;
	char log_buf[DEVDRV_NPU_BUF_LEN_MAX + 1] = {0};
	vir_addr_t temp_addr_vir = g_bbox_aicpulog_addr_vir;

	exception_info = (struct exc_module_info_s *)(uintptr_t)(temp_addr_vir);
	log_len = exception_info->e_info_offset + exception_info->e_info_len;

	if (log_len > (uint32_t)AICPU_BUFF_LEN || log_len <= 0) {
		NPU_DRV_ERR("log_len is more than the max len, log_len = %x\n", log_len);
		return ;
	}

	if (log_len > (uint32_t)DEVDRV_NPU_BUF_LEN_MAX) {
		do {
			memcpy((void *)log_buf, (void *)((uintptr_t)temp_addr_vir),
				DEVDRV_NPU_BUF_LEN_MAX);
			devdrv_npu_mntn_copy_aicpulog_to_bbox(log_buf, (unsigned int)DEVDRV_NPU_BUF_LEN_MAX);
			log_len -= DEVDRV_NPU_BUF_LEN_MAX;
			temp_addr_vir += DEVDRV_NPU_BUF_LEN_MAX;
		} while (log_len > DEVDRV_NPU_BUF_LEN_MAX);
	}
	memcpy((void *)log_buf, (void *)((uintptr_t)temp_addr_vir), log_len);

	devdrv_npu_mntn_copy_aicpulog_to_bbox(log_buf, log_len);
}

static void devdrv_npu_rdr_regs_dump(void *dest, const void *src, size_t len)
{
	size_t remain;
	size_t mult;
	size_t i;
	u64 *u64_dst = NULL;
	u64 *u64_src = NULL;

	remain = len % sizeof(u64);
	mult = len / sizeof(u64);
	u64_dst = (u64 *)dest;
	u64_src = (u64 *)src;

	for (i = 0; i < mult; i++) {
		*u64_dst = *u64_src;
		NPU_DRV_DEBUG("index:%u u64_dst:0x%016lx= 0x%016lx  u64_src:0x%016lx = 0x%016lx",
			i, (u64)u64_dst, *u64_dst, (u64)(u64_src), *u64_src);
		u64_dst++;
		u64_src++;
	}

	for (i = 0; i < remain; i++)
		*((u8 *)u64_dst + i) = *((u8 *)u64_src + i);
}

static void devdrv_npu_mntn_dump_npucrg(const struct devdrv_mem_desc *dump_region, vir_addr_t dest_addr)
{
	u8 *source_addr = NULL;
	uint32_t pericrg_stat;
	u64 pericrg_base;
	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_irq failed\n");
		return;
	}

	pericrg_base = (u64) (uintptr_t) DEVDRV_PLAT_GET_REG_VADDR(plat_info,
		DEVDRV_REG_PERICRG_BASE);
	if (pericrg_base == 0) {
		NPU_DRV_ERR("pericrg_base is 0\n");
		return;
	}
	NPU_DRV_WARN("dump region: base=0x%x len=0x%x", dump_region->base, dump_region->len);

	pericrg_stat = readl_relaxed((const volatile void *)(uintptr_t)SOC_CRGPERIPH_NPUCRG_STAT_ADDR(pericrg_base));
	if (0x1 == BITMAP_GET (pericrg_stat, SOC_CRGPERIPH_NPUCRG_IP_RST_BIT_OFFSET_START)) {
		NPU_DRV_ERR("NPU CRG has not been unreset!\n");
		return ;
	}

	source_addr = ioremap_nocache(dump_region->base, dump_region->len);
	if (source_addr == NULL) {
		NPU_DRV_ERR("ioremap_nocache failed.base=0x%x len=0x%x\n",
			dump_region->base, dump_region->len);
		return ;
	}
	devdrv_npu_rdr_regs_dump((void *)(uintptr_t)dest_addr, source_addr, dump_region->len);
	iounmap(source_addr);
}

static void devdrv_npu_mntn_dump_region(void)
{
	u32 i;
	vir_addr_t target_addr = g_bbox_addr_vir + BBOX_ADDR_LEN;
	struct devdrv_mem_desc *dump_region = NULL;
	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();
	struct devdrv_dev_ctx *dev_ctx = NULL;

	for (i = 0; i < DEVDRV_DUMP_REGION_MAX; i++) {
		dump_region = &DEVDRV_PLAT_GET_DUMP_REGION_DESC(plat_info, i);
		if (dump_region == NULL) {
			NPU_DRV_ERR ("dump_region %d is NULL", i);
			break;
		}
		if (dump_region->base == 0 || dump_region->len == 0) {
			NPU_DRV_ERR ("dump_region %d ' base=0x%x len=0x%x",
				i, dump_region->base, dump_region->len);
			break;
		}
		/* from the last area of npu_bbox */
		target_addr = target_addr - dump_region->len;

		NPU_DRV_WARN("dump region %d: base=0x%x len=0x%x, bbox offset=0x%lx",
			i, dump_region->base, dump_region->len, target_addr - g_bbox_addr_vir);

		if ((target_addr < g_bbox_addr_vir) || (target_addr > (g_bbox_addr_vir + BBOX_ADDR_LEN))) {
			NPU_DRV_ERR ("dump_region %d ' out of range; len:0x%x is error", i, dump_region->len);
			return;
		}

		if (i == DEVDRV_DUMP_REGION_NPU_CRG) {
			dev_ctx = get_dev_ctx_by_id(0);
			if (dev_ctx == NULL) {
				NPU_DRV_ERR("dev_ctx 0 is null\n");
				return;
			}

			mutex_lock(&dev_ctx->npu_power_up_off_mutex);

			if (dev_ctx->power_stage != DEVDRV_PM_UP) {
				NPU_DRV_WARN("can not dump npu_crg because npu is powered off\n");
			} else if (dev_ctx->secure_state == NPU_SEC) {
				NPU_DRV_WARN("can not dump npu_crg because npu is in secure mode\n");
			} else {
				devdrv_npu_mntn_dump_npucrg(dump_region, target_addr);
			}

			mutex_unlock(&dev_ctx->npu_power_up_off_mutex);
		}
	}
}


/* *******************************************************************
Description: devdrv_npu_mntn_dump_work
input: struct work_struct *work
output: NA
return: NA
******************************************************************* */
static void devdrv_npu_mntn_dump_work(struct work_struct *work __attribute__((__unused__)))
{
	exce_module_info_t *module_info = NULL;
	char log_buf[DEVDRV_NPU_BUF_LEN_MAX + 1] = {0};
	uint32_t log_len;
	vir_addr_t temp_bbox_tslog_addr_vir = g_bbox_tslog_addr_vir;

	devdrv_npu_mntn_write_reg_log(g_devdrv_npu_mntn_private->core_id);

	/* copy aicpu log to rdr address */
	devdrv_npu_mntn_dump_aicpulog_work();

	module_info = (exce_module_info_t *)(uintptr_t)(g_bbox_addr_vir);
	/* log_len is maxvalue */
	log_len = (module_info->totalset_len > module_info->miniset_len) ?
		module_info->totalset_len : module_info->miniset_len;

	if (log_len > (uint32_t)(DEVDRV_NPU_BBOX_MEM_MAX - sizeof(exce_module_info_t)) || log_len <= 0) {
		NPU_DRV_ERR("log_len is more than the max len or log_len is 0, log_len = 0x%x !\n", log_len);
		return ;
	}

	if (log_len > (uint32_t)DEVDRV_NPU_BUF_LEN_MAX) {
		do {
			memcpy((void *)log_buf, (void *)((uintptr_t)temp_bbox_tslog_addr_vir), DEVDRV_NPU_BUF_LEN_MAX);
			devdrv_npu_mntn_copy_reg_to_bbox(log_buf, (unsigned int)DEVDRV_NPU_BUF_LEN_MAX);
			log_len -= DEVDRV_NPU_BUF_LEN_MAX;
			temp_bbox_tslog_addr_vir += DEVDRV_NPU_BUF_LEN_MAX;
		} while (log_len > DEVDRV_NPU_BUF_LEN_MAX);
	}

	memcpy((void *)log_buf, (void *)((uintptr_t)temp_bbox_tslog_addr_vir), log_len);
	devdrv_npu_mntn_copy_reg_to_bbox(log_buf, log_len);

	if (g_devdrv_npu_mntn_private->mntn_info.dump_info.cb != NULL)
		g_devdrv_npu_mntn_private->mntn_info.dump_info.cb(
			g_devdrv_npu_mntn_private->mntn_info.dump_info.modid,
			g_devdrv_npu_mntn_private->mntn_info.dump_info.coreid);

	devdrv_npu_mntn_dump_region();

	return;
}


/* *******************************************************************
Description : npu_rdr_resource_init
input : void
output : NA
return : int:  0 is OK. other value is Error No.
******************************************************************* */
int npu_rdr_resource_init(void)
{
	g_devdrv_npu_mntn_private = kzalloc(sizeof(struct devdrv_npu_mntn_private_s), GFP_KERNEL);
	if (g_devdrv_npu_mntn_private == NULL) {
		NPU_DRV_ERR("failed to allocate memory!");
		return -ENOMEM;
	}

	g_devdrv_npu_mntn_private->rdr_wq = create_singlethread_workqueue("npu_mntn_rdr_wq");
	if (g_devdrv_npu_mntn_private->rdr_wq == NULL) {
		NPU_DRV_ERR("create_singlethread_workqueue is failed!");
		kfree(g_devdrv_npu_mntn_private);
		g_devdrv_npu_mntn_private = NULL;
		return -EINTR;
	}

	INIT_WORK(&g_devdrv_npu_mntn_private->dump_work, devdrv_npu_mntn_dump_work);

	return 0;
}

/* when smmu is exception, platform  callback this func report */
static int npu_smmu_excp_callback(struct notifier_block *nb, unsigned long value, void *data)
{
	u8 smmu_event_id;
	(void)nb;
	(void)value;

	if (data == NULL)
		return -ENOMEM;

	// data pointer to u64 evt[EVTQ_ENT_DWORDS]
	smmu_event_id = ((u64 *)data)[0] & 0xFF;
	NPU_DRV_WARN("evt_info[0] : 0x%x !", smmu_event_id);
	// fiter arm_smmu_ai event 0x10
	if (smmu_event_id != SMMU_ADDR_TRANSLATION_FAULT && smmu_event_id != SMMU_BAD_CD_FAULT)
		rdr_system_error((u32)RDR_EXC_TYPE_NPU_SMMU_EXCEPTION, 0, 0);
	return 0;
}

static struct notifier_block smmu_excp_notifier = {
	.notifier_call = npu_smmu_excp_callback,
};

/* *******************************************************************
Description : npu_rdr_register_exception
input : void
output : NA
rdr_register_exception : ret == 0 is fail. ret > 0 is success.
******************************************************************* */
int npu_rdr_register_exception(void)
{
	int ret;
	unsigned int  size;
	unsigned long index;

	size = sizeof(devdrv_npu_rdr_excetption_info) / sizeof(devdrv_npu_rdr_excetption_info[0]);
	for (index = 0; index < size; index++) {
		/* error return 0, ok return modid */
		ret = rdr_register_exception(&devdrv_npu_rdr_excetption_info[index]);
		if (ret == 0) {
			NPU_DRV_ERR("rdr_register_exception is failed! index = %ld, ret = %d", index, ret);
			return -EINTR;
		}
	}
	return 0;
}

/* *******************************************************************
Description : npu_rdr_unregister_exception
input : void
output : NA
rdr_unregister_exception : ret < 0 is fail. ret >= 0 success.
******************************************************************* */
int npu_rdr_unregister_exception(void)
{
	int ret;
	unsigned int  size;
	unsigned long index;

	size = sizeof(devdrv_npu_rdr_excetption_info) / sizeof(devdrv_npu_rdr_excetption_info[0]);
	for (index = 0; index < size; index++) {
		/* ret < 0 is fail, ret >= 0 is success */
		ret = rdr_unregister_exception(devdrv_npu_rdr_excetption_info[index].e_modid);
		if (ret < 0) {
			NPU_DRV_ERR("rdr_unregister_exception is failed! index = %ld, ret = %d", index, ret);
			return -EINTR;
		}
	}
	return 0;
}

/* *******************************************************************
Description: devdrv_npu_mntn_rdr_dump
input:modid: module id
		etype:exception type
		coreid: core id
		pathname: log path
		pfn_cb: callback function
output: NA
return: NA
******************************************************************* */
static void devdrv_npu_mntn_rdr_dump(u32 modid, u32 etype, u64 coreid, char *pathname, pfn_cb_dump_done pfn_cb)
{
	NPU_DRV_WARN("modid=0x%x, etype=0x%x, coreid=0x%llx", modid, etype, coreid);
	if (pathname == NULL) {
		NPU_DRV_ERR("pathname is empty!");
		return;
	}
	g_devdrv_npu_mntn_private->mntn_info.dump_info.modid = modid;
	g_devdrv_npu_mntn_private->mntn_info.dump_info.coreid = coreid;
	g_devdrv_npu_mntn_private->mntn_info.dump_info.pathname = pathname;
	g_devdrv_npu_mntn_private->mntn_info.dump_info.cb = pfn_cb;
	g_devdrv_npu_mntn_private->mntn_info.bbox_addr_offset = 0;
	queue_work(g_devdrv_npu_mntn_private->rdr_wq, &g_devdrv_npu_mntn_private->dump_work);

	if (pfn_cb)
		pfn_cb(modid, coreid);

	return;
}


/* *******************************************************************
Description: devdrv_npu_mntn_rdr_reset
input: modid:module id
		etype:exception type
		coreid:core id
output: NA
return: NA
******************************************************************* */
static void devdrv_npu_mntn_rdr_reset(u32 modid, u32 etype, u64 coreid)
{
	NPU_DRV_WARN("modid=0x%x, etype=0x%x, coreid=0x%llx", modid, etype, coreid);
	return;
}


/* *******************************************************************
Description : npu_rdr_register_core
input : void
output : NA
rdr_register_module_ops : ret <0 is fail. ret >= 0 is success.
******************************************************************* */
int npu_rdr_register_core(void)
{
	int ret;
	struct rdr_module_ops_pub s_soc_ops;

	s_soc_ops.ops_dump = devdrv_npu_mntn_rdr_dump;
	s_soc_ops.ops_reset = devdrv_npu_mntn_rdr_reset;
	/* register npu core dump and reset function */
	ret = rdr_register_module_ops((u64)RDR_NPU, &s_soc_ops, &g_devdrv_npu_mntn_private->mntn_info.npu_ret_info);
	if (ret < 0)
		NPU_DRV_ERR("rdr_register_module_ops is failed! ret = 0x%08x", ret);

	return ret;
}

/* *******************************************************************
Description : npu_rdr_unregister_core
input : void
output : NA
rdr_unregister_module_ops : ret < 0 is fail. ret >= 0 is success.
******************************************************************* */
int npu_rdr_unregister_core(void)
{
	int ret;

	/* unregister npu core dump and reset function */
	ret = rdr_unregister_module_ops((u64)RDR_NPU);
	if (ret < 0)
		NPU_DRV_ERR("rdr_unregister_module_ops is failed! ret = 0x%08x", ret);

	return ret;
}

/* *******************************************************************
Description : npu_rdr_addr_map
input : void
output : NA
return : int:  0 is OK. other value is Error No.
******************************************************************* */
int npu_rdr_addr_map(void)
{
	if (g_devdrv_npu_mntn_private == NULL) {
		NPU_DRV_ERR("invalid null pointer");
		return -EINVAL;
	}
#if (!(defined CONFIG_ORLANDO_SUPPORT))
	if (g_devdrv_npu_mntn_private->mntn_info.npu_ret_info.log_len <
		(DEVDRV_NPU_BBOX_MEM_MAX + DEVDRV_AICPU_BBOX_MEM_MAX)) {
		NPU_DRV_ERR("invalid log len 0x%lx", g_devdrv_npu_mntn_private->mntn_info.npu_ret_info.log_len);
		return -EINVAL;
	}
#endif
	g_devdrv_npu_mntn_private->mntn_info.rdr_addr =
		hisi_bbox_map((phys_addr_t)g_devdrv_npu_mntn_private->mntn_info.npu_ret_info.log_addr,
		g_devdrv_npu_mntn_private->mntn_info.npu_ret_info.log_len);
	if (g_devdrv_npu_mntn_private->mntn_info.rdr_addr == NULL) {
		NPU_DRV_ERR("hisi_bbox_map is failed!");
		return -EFAULT;
	}

	return 0;
}

/* *******************************************************************
Description : npu_rdr_addr_unmap
input : void
output : NA
return : int:  0 is OK. other value is Error No.
******************************************************************* */
int npu_rdr_addr_unmap(void)
{
	hisi_bbox_unmap(g_devdrv_npu_mntn_private->mntn_info.rdr_addr);
	g_devdrv_npu_mntn_private->mntn_info.rdr_addr = NULL;
	return 0;
}

/* *******************************************************************
Description : npu_blackbox_addr_init
input : void
output : NA
return : int:  0 is OK. other value is Error No.
******************************************************************* */
int npu_blackbox_addr_init(void)
{
	struct devdrv_platform_info *plat_info = NULL;
	struct devdrv_dfx_desc *dfx_desc = NULL;
	struct devdrv_mem_desc *aicpu_men_desc = NULL;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info error\n");
		return -1;
	}

	dfx_desc = &DEVDRV_PLAT_GET_DFX_DESC(plat_info, DEVDRV_DFX_DEV_BLACKBOX);
	if (dfx_desc == NULL) {
		NPU_DRV_ERR("get dtsi failed\n");
		return -1;
	}
	aicpu_men_desc = &DEVDRV_PLAT_GET_RESMEM_DESC(plat_info, AICPU_EL1);
	if (aicpu_men_desc == NULL) {
		NPU_DRV_ERR("get dtsi failed\n");
		return -1;
	}

	g_bbox_addr_vir = (unsigned long long)(uintptr_t)ioremap_wc(dfx_desc->bufs->base, BBOX_ADDR_LEN);
	if (g_bbox_addr_vir == 0) {
		NPU_DRV_ERR("ioremap_wc failed\n");
		return -1;
	}

	g_bbox_aicpulog_addr_vir = (unsigned long long)(uintptr_t)ioremap_wc(aicpu_men_desc->base +
		AICPU_BUFF_OFFSET, AICPU_BUFF_LEN);
	if (g_bbox_aicpulog_addr_vir == 0) {
		NPU_DRV_ERR("ioremap_wc failed\n");
		return -1;
	}

	g_bbox_tslog_addr_vir = g_bbox_addr_vir + sizeof(exce_module_info_t);
	memset((void *)(uintptr_t)g_bbox_addr_vir, 0, BBOX_ADDR_LEN);
	memset((void *)(uintptr_t)g_bbox_aicpulog_addr_vir, 0, AICPU_BUFF_LEN);

	return 0;
}

int npu_blackbox_addr_release(void)
{
	if (g_bbox_addr_vir != 0) {
		iounmap((void *)((uintptr_t)g_bbox_addr_vir));
		g_bbox_addr_vir = 0;
	}

	if (g_bbox_aicpulog_addr_vir != 0) {
		iounmap((void *)((uintptr_t)g_bbox_aicpulog_addr_vir));
		g_bbox_aicpulog_addr_vir = 0;
	}

	g_bbox_tslog_addr_vir = 0;

	return 0;
}

/* *******************************************************************
Description : npu_black_box_init
input : void
output : NA
return : int:  0 is OK. other value is Error No.
******************************************************************* */
int npu_black_box_init(void)
{
	int ret;
	NPU_DRV_INFO("npu_black_box_init Enter\n");

	ret = npu_rdr_resource_init();
	COND_RETURN_ERROR(ret < 0, ret, "npu_rdr_resource_init is faild ! ret = %d", ret);

	/* register NPU exception */
	ret = npu_rdr_register_exception();
	COND_GOTO_ERROR(ret < 0, error, ret, ret, "npu_rdr_register_exception is faild! ret = %d", ret);

	/* register NPU dump and reset function */
	ret = npu_rdr_register_core();
	if (ret < 0) {
		NPU_DRV_ERR("npu_rdr_register_core is failed! ret = %d", ret);
		npu_rdr_unregister_exception();
		goto error;
	}

	ret = npu_rdr_addr_map();
	if (ret < 0) {
		NPU_DRV_ERR("npu_rdr_addr_map is failed! ret = %d", ret);
		npu_rdr_unregister_core();
		npu_rdr_unregister_exception();
		goto error;
	}

	ret = npu_blackbox_addr_init();
	if (ret < 0) {
		npu_rdr_addr_unmap();
		npu_rdr_unregister_core();
		npu_rdr_unregister_exception();
		NPU_DRV_ERR("npu_blackbox_addr_init is failed ! ret = %d", ret);
		goto error;
	}

	ret = hisi_smmu_evt_register_notify(&smmu_excp_notifier);
	if (ret < 0) {
		npu_blackbox_addr_release();
		npu_rdr_addr_unmap();
		npu_rdr_unregister_core();
		npu_rdr_unregister_exception();
		NPU_DRV_ERR("hisi_smmu_evt_register_notify is failed ! ret = %d", ret);
		goto error;
	}

	return 0;

error:
	if (g_devdrv_npu_mntn_private != NULL) {
		destroy_workqueue(g_devdrv_npu_mntn_private->rdr_wq);
		kfree(g_devdrv_npu_mntn_private);
		g_devdrv_npu_mntn_private = NULL;
	}
	return ret;
}

/* *******************************************************************
Description : npu_black_box_exit
input : void
output : NA
return : int:  0 is OK. other value is Error No.
******************************************************************* */
int npu_black_box_exit(void)
{
	int ret;

	(void)hisi_smmu_evt_unregister_notify(&smmu_excp_notifier);
	npu_blackbox_addr_release();
	(void)npu_rdr_addr_unmap();

	ret = npu_rdr_unregister_core();
	if (ret < 0)
		NPU_DRV_ERR("npu_rdr_unregister_core is faild! ret = %d", ret);

	ret = npu_rdr_unregister_exception();
	if (ret < 0)
		NPU_DRV_ERR("npu_rdr_unregister_exception is faild! ret = %d", ret);

	if (g_devdrv_npu_mntn_private != NULL) {
		destroy_workqueue(g_devdrv_npu_mntn_private->rdr_wq);
		kfree(g_devdrv_npu_mntn_private);
		g_devdrv_npu_mntn_private = NULL;
	}

	return 0;
}

int npu_noc_register(void)
{
#ifdef CONFIG_NPU_NOC
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(npu_noc_para); i++) {
		NPU_DRV_INFO("register noc err, type:%x", ((u32)RDR_EXC_TYPE_NOC_NPU0 + i));
		noc_modid_register(npu_noc_para[i], ((u32)RDR_EXC_TYPE_NOC_NPU0 + i));
	}
#endif
	return 0;
}

