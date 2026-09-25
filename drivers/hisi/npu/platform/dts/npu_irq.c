/*
 * npu_irq.c
 *
 * about npu irq
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
#include "npu_irq.h"
#include "npu_log.h"

int devdrv_plat_parse_irq(struct platform_device *pdev, struct devdrv_platform_info *plat_info)
{
	DEVDRV_PLAT_GET_CQ_UPDATE_IRQ(plat_info) = platform_get_irq(pdev, DEVDRV_IRQ_CALC_CQ_UPDATE0);
	DEVDRV_PLAT_GET_DFX_CQ_IRQ(plat_info) = platform_get_irq(pdev, DEVDRV_IRQ_DFX_CQ_UPDATE);
	DEVDRV_PLAT_GET_MAILBOX_ACK_IRQ(plat_info) = platform_get_irq(pdev, DEVDRV_IRQ_MAILBOX_ACK);

	NPU_DRV_DEBUG("calc_cq_update0=%d\n", DEVDRV_PLAT_GET_CQ_UPDATE_IRQ(plat_info));
	NPU_DRV_DEBUG("dfx_cq_update=%d\n", DEVDRV_PLAT_GET_DFX_CQ_IRQ(plat_info));
	NPU_DRV_DEBUG("mailbox_ack=%d\n", DEVDRV_PLAT_GET_MAILBOX_ACK_IRQ(plat_info));

	return 0;
}
