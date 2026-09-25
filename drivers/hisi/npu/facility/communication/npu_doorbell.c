/*
 * npu_doorbell.c
 *
 * about npu doorbell
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
#include "npu_doorbell.h"

#include "npu_common.h"
#include "npu_log.h"
#include "npu_pm.h"
#include "npu_platform.h"

static u32 devdrv_dev_doorbell_tbl[DOORBELL_RES_RESERVED] = { 0 };
static u64 doorbell_base = 0;
static u32 doorbell_stride = 0;

int devdrv_dev_doorbell_init(void)
{
	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_irq failed\n");
		return -1;
	}
	doorbell_base = (u64) (uintptr_t) DEVDRV_PLAT_GET_REG_VADDR(plat_info, DEVDRV_REG_TS_DOORBELL);
	if (doorbell_base == 0)
		return -EINVAL;

	NPU_DRV_DEBUG("doorbell base %llx\n", doorbell_base);
	devdrv_dev_doorbell_tbl[DOORBELL_RES_CAL_SQ] = DEVDRV_PLAT_GET_CALC_SQ_MAX(plat_info);
	devdrv_dev_doorbell_tbl[DOORBELL_RES_CAL_CQ] = DEVDRV_PLAT_GET_CALC_CQ_MAX(plat_info);
	devdrv_dev_doorbell_tbl[DOORBELL_RES_DFX_SQ] = DEVDRV_PLAT_GET_DFX_SQ_MAX(plat_info);
	devdrv_dev_doorbell_tbl[DOORBELL_RES_DFX_CQ] = DEVDRV_PLAT_GET_DFX_CQ_MAX(plat_info);
	devdrv_dev_doorbell_tbl[DOORBELL_RES_MAILBOX] = DOORBELL_MAILBOX_MAX_SIZE;
	doorbell_stride = DEVDRV_PLAT_GET_DOORBELL_STRIDE(plat_info);
	return 0;
}
EXPORT_SYMBOL(devdrv_dev_doorbell_init);

int devdrv_dev_doorbell_register(u8 dev_id)
{
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		NPU_DRV_ERR("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}
	return 0;
}

int devdrv_get_doorbell_vaddr(u32 type, u32 index, u32 **doorbell_vaddr)
{
	u8 loop_res_type;
	u32 *addr = NULL;
	u8 acc_index;

	if ((type >= DOORBELL_RES_RESERVED) || (index >= devdrv_dev_doorbell_tbl[type])) {
		NPU_DRV_ERR("input para is invalid\n");
		return -EINVAL;
	}

	acc_index = 0;
	for (loop_res_type = 0; loop_res_type < type; loop_res_type++) {
		acc_index += devdrv_dev_doorbell_tbl[loop_res_type];
	}

	if (acc_index >= DOORBELL_MAX_SIZE) {
		NPU_DRV_ERR("acc_index %d is invalid\n", acc_index);
		return -EINVAL;
	}

	addr = (u32 *) (uintptr_t) (doorbell_base + doorbell_stride * (acc_index + index));
	NPU_DRV_DEBUG("devdrv acc_index %d index %d addr %pK, base %pK \n",
		acc_index, index, (unsigned long long *)addr, (void *)(uintptr_t)doorbell_base);

	*doorbell_vaddr = addr;

	isb();

	return 0;
}
EXPORT_SYMBOL(devdrv_get_doorbell_vaddr);

int devdrv_write_doorbell_val(u32 type, u32 index, u32 val)
{
	u8 loop_res_type;
	u8 acc_index;
	u32 *addr = NULL;

	if ((type >= DOORBELL_RES_RESERVED) || (index >= devdrv_dev_doorbell_tbl[type])) {
		NPU_DRV_ERR("input para is invalid\n");
		return -EINVAL;
	}

	acc_index = 0;
	for (loop_res_type = 0; loop_res_type < type; loop_res_type++) {
		acc_index += devdrv_dev_doorbell_tbl[loop_res_type];
	}

	if (acc_index >= DOORBELL_MAX_SIZE) {
		NPU_DRV_ERR("acc_index %u is invalid\n", acc_index);
		return -EINVAL;
	}
	addr = (u32 *) (uintptr_t) (doorbell_base + doorbell_stride * (acc_index + index));
	NPU_DRV_DEBUG("devdrv acc_index:%u, index:%u, addr:%pK, base:%pK\n",
		acc_index, index, (unsigned long long *)addr, (void *)(uintptr_t)doorbell_base);

	*addr = (type == DOORBELL_RES_MAILBOX) ? DOORBELL_MAILBOX_VALUE : val;

	isb();

	return 0;
}
EXPORT_SYMBOL(devdrv_write_doorbell_val);
