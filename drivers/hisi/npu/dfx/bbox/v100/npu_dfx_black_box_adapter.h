/*
 * npu_dfx_black_box_adapter.h
 *
 * about npu black box adapter
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
 * use in hi3690, kirin990_cs2, hi6280, hi6290
 */
#ifndef __NPU_BLACK_BOX_ADAPTER_H
#define __NPU_BLACK_BOX_ADAPTER_H

struct rdr_exception_info_s npu_rdr_excetption_info[] = {
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
	}, {
		.e_modid = (u32)RDR_TYPE_HWTS_BUS_ERROR,
		.e_modid_end = (u32)RDR_TYPE_HWTS_BUS_ERROR,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = HWTS_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "HWTS_EXCP",
	}, {
		.e_modid = (u32)EXC_TYPE_TS_SDMA_EXCEPTION,
		.e_modid_end = (u32)EXC_TYPE_TS_SDMA_TIMEOUT,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = HWTS_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG,
		.e_from_module = "NPU",
		.e_desc = "SDMA_EXCP_OR_TIMEOUT",
	},
};

#ifdef CONFIG_NPU_NOC
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

u32 modid_array[] = {(u32)RDR_EXC_TYPE_NOC_NPU0, (u32)RDR_EXC_TYPE_NOC_NPU1};
#endif

static struct npu_dump_reg peri_regs[0];

#endif /* __NPU_BLACK_BOX_ADAPTER_H */
