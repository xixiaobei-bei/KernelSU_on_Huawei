/*
 * npu_irq.c
 *
 * about npu irq
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
#include "npu_irq.h"
#include "npu_log.h"

int npu_plat_parse_irq(struct platform_device *pdev,
	struct npu_platform_info *plat_info)
{
	plat_info->dts_info.irq_cq_update = platform_get_irq(pdev,
		NPU_IRQ_CALC_CQ_UPDATE0);
	plat_info->dts_info.irq_dfx_cq = platform_get_irq(pdev,
		NPU_IRQ_DFX_CQ_UPDATE);
	plat_info->dts_info.irq_mailbox_ack = platform_get_irq(pdev,
		NPU_IRQ_MAILBOX_ACK);

	npu_drv_debug("calc_cq_update0=%d, dfx_cq_update=%d, mailbox_ack=%d\n",
		plat_info->dts_info.irq_cq_update,
		plat_info->dts_info.irq_dfx_cq,
		plat_info->dts_info.irq_mailbox_ack);

	return 0;
}
