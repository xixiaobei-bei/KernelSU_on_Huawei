/*
 * npu_resmem.h
 *
 * about npu resmem
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
#ifndef __NPU_RESMEM_H
#define __NPU_RESMEM_H

#include <linux/platform_device.h>
#include "npu_platform.h"

#define DEVDRV_RESMEM_LEN_NAME  "reserved_memory_len"
#define DEVDRV_RESMEM_BASE_NAME "reserved_memory_base"
#define DEVDRV_RESMEM_S_LEN_NAME  "reserved_memory_firmware_len_sec"
#define DEVDRV_RESMEM_S_BASE_NAME "reserved_memory_s_base"
#define DEVDRV_RESMEM_TSFW_NAME "ts_fw_buf_idx"
#define DEVDRV_RESMEM_AIFW_NAME "ai_fw_buf_idx"
#define DEVDRV_RESMEM_SQCQ_NAME "sqcq_buf_idx"
#define DEVDRV_RESMEM_PERSISTENT_TAKS_BUF_NAME "persistent_task_buf_idx"
#define DEVDRV_RESMEM_CHIP_CFG_NAME "chip_cfg_buf_idx"
#define DEVDRV_RESMEM_INFO_NAME "info_buf_idx"
#define DEVDRV_RESMEM_HWTS_SQ_NAME "hwts_sq_buf_idx"
#define DEVDRV_RESMEM_HWTS_CQ_NAME "hwts_cq_buf_idx"

int devdrv_plat_parse_resmem_desc(const struct device_node *root, struct devdrv_platform_info *plat_info);

int devdrv_plat_parse_resmem_s_desc(const struct device_node *root, struct devdrv_platform_info *plat_info);

int devdrv_plat_parse_resmem_usage(struct device_node *module_np, struct devdrv_platform_info *plat_info);

int devdrv_plat_find_resmem_idx(const struct device_node *module_np, struct devdrv_platform_info *plat_info,
                                const char* tag, struct devdrv_mem_desc **result);

#endif
