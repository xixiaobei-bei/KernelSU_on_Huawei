/*
 * npu_doorbell.c
 *
 * about npu doorbell
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
#include "npu_doorbell.h"

#include "npu_common.h"
#include "npu_log.h"
#include "npu_pm_framework.h"
#include "npu_platform.h"

static u32 npu_dev_doorbell_tbl[DOORBELL_RES_RESERVED] = { 0 };
static u64 doorbell_base;
static u32 doorbell_stride;

int npu_dev_doorbell_init(void)
{
	struct npu_platform_info *plat_info = npu_plat_get_info();

	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_irq failed\n");
		return -1;
	}
	doorbell_base = (u64)(uintptr_t)
		plat_info->dts_info.reg_vaddr[NPU_REG_TS_DOORBELL];
	if (doorbell_base == 0)
		return -EINVAL;

	npu_drv_debug("doorbell base %llx\n", doorbell_base);
	npu_dev_doorbell_tbl[DOORBELL_RES_CAL_SQ] = plat_info->spec.calc_sq_max;
	npu_dev_doorbell_tbl[DOORBELL_RES_CAL_CQ] = plat_info->spec.calc_cq_max;
	npu_dev_doorbell_tbl[DOORBELL_RES_DFX_SQ] = plat_info->spec.dfx_sq_max;
	npu_dev_doorbell_tbl[DOORBELL_RES_DFX_CQ] = plat_info->spec.dfx_cq_max;
	npu_dev_doorbell_tbl[DOORBELL_RES_MAILBOX] = DOORBELL_MAILBOX_MAX_SIZE;
	doorbell_stride = plat_info->spec.doorbell_stride;
	return 0;
}
EXPORT_SYMBOL(npu_dev_doorbell_init);

int npu_dev_doorbell_register(u8 dev_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}
	return 0;
}

int npu_get_doorbell_vaddr(u32 type, u32 index, u32 **doorbell_vaddr)
{
	u8 loop_res_type;
	u32 *addr = NULL;
	u8 acc_index;

	if ((type >= DOORBELL_RES_RESERVED) ||
		(index >= npu_dev_doorbell_tbl[type])) {
		npu_drv_err("input para is invalid\n");
		return -EINVAL;
	}

	acc_index = 0;
	for (loop_res_type = 0; loop_res_type < type; loop_res_type++)
		acc_index += npu_dev_doorbell_tbl[loop_res_type];

	if (acc_index >= DOORBELL_MAX_SIZE) {
		npu_drv_err("acc_index %d is invalid\n", acc_index);
		return -EINVAL;
	}

	addr = (u32 *)(uintptr_t)
		(doorbell_base + doorbell_stride * (acc_index + index));
	npu_drv_debug("devdrv acc_index %d index %d addr %pK, base %pK\n",
		acc_index, index, (unsigned long long *)addr,
		(void *)(uintptr_t)doorbell_base);

	*doorbell_vaddr = addr;

	isb();

	return 0;
}
EXPORT_SYMBOL(npu_get_doorbell_vaddr);

int npu_write_doorbell_val(u32 type, u32 index, u32 val)
{
	u8 loop_res_type;
	u8 acc_index;
	u32 *addr = NULL;

	if ((type >= DOORBELL_RES_RESERVED) ||
		(index >= npu_dev_doorbell_tbl[type])) {
		npu_drv_err("input para is invalid\n");
		return -EINVAL;
	}

	acc_index = 0;
	for (loop_res_type = 0; loop_res_type < type; loop_res_type++)
		acc_index += npu_dev_doorbell_tbl[loop_res_type];

	if (acc_index >= DOORBELL_MAX_SIZE) {
		npu_drv_err("acc_index %u is invalid\n", acc_index);
		return -EINVAL;
	}
	addr = (u32 *)(uintptr_t)
		(doorbell_base + doorbell_stride * (acc_index + index));
	npu_drv_debug("devdrv acc_index:%u, index:%u, addr:%pK, base:%pK\n",
		acc_index, index, (unsigned long long *)addr,
		(void *)(uintptr_t)doorbell_base);

	*addr = (type == DOORBELL_RES_MAILBOX) ? DOORBELL_MAILBOX_VALUE : val;

	isb();

	return 0;
}
EXPORT_SYMBOL(npu_write_doorbell_val);
