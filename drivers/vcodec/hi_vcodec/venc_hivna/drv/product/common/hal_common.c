/*
 * hal_common.c
 *
 * common config
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "hal_common.h"
#include <linux/delay.h>
#include "drv_common.h"
#include "hal_venc.h"
#include "smmu.h"

#define REQ_BUS_IDLE_TIME_OUT 10

void vedu_hal_request_bus_idle(uint32_t *reg_base)
{
	int32_t i;
	S_HEVC_AVC_REGS_TYPE *all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;

	all_reg->VEDU_BUS_IDLE_REQ.bits.vedu_bus_idle_req = 1;

	for (i = 0; i < REQ_BUS_IDLE_TIME_OUT; i++) {
		if (all_reg->FUNC_VCPI_BUS_IDLE_FLAG.bits.bus_idle_flag)
			break;
		udelay(1);
	}

	if (i == REQ_BUS_IDLE_TIME_OUT)
		HI_FATAL_VENC("request bus idle timeout");
}

void vedu_hal_cfg_reg_intraset(const struct encode_info *channel_cfg,
	uint32_t *reg_base)
{
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_INTRA_RDO_FACTOR_0.u32),
		channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_0.u32);

	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_INTRA_RDO_FACTOR_1.u32),
		channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.u32);

	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_INTRA_RDO_FACTOR_2.u32),
		channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.u32);

	{
		U_VEDU_PME_NEW_COST D32;

		D32.bits.pme_mvp3median_en =
			channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_mvp3median_en;
		D32.bits.pme_cost_lamda_en =
			channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda_en;
		D32.bits.pme_new_cost_en =
			channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_new_cost_en;
		D32.bits.pme_cost_lamda2 =
			channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda2;
		D32.bits.pme_cost_lamda1 =
			channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda1;
		D32.bits.pme_cost_lamda0 =
			channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_NEW_COST.u32),
			D32.u32);
	}
}

void vedu_hal_cfg_reg_lambda_set(const struct encode_info *channel_cfg,
	uint32_t *reg_base)
{
	/* lambd reg cfg set */
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG00.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG00.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG01.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG01.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG02.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG02.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG03.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG03.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG04.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG04.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG05.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG05.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG06.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG06.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG07.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG07.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG08.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG08.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG09.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG09.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG10.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG10.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG11.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG11.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG12.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG12.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG13.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG13.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG14.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG14.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG15.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG15.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG16.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG16.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG17.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG17.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG18.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG18.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG19.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG19.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG20.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG20.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG21.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG21.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG22.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG22.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG23.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG23.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG24.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG24.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG25.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG25.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG26.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG26.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG27.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG27.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG28.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG28.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG29.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG29.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG30.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG30.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG31.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG31.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG32.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG32.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG33.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG33.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG34.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG34.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG35.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG35.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG36.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG36.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG37.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG37.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG38.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG38.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG39.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG39.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG40.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG40.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG41.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG41.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG42.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG42.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG43.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG43.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG44.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG44.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG45.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG45.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG46.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG46.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG47.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG47.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG48.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG48.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG49.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG49.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG50.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG50.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG51.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG51.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG52.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG52.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG53.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG53.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG54.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG54.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG55.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG55.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG56.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG56.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG57.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG57.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG58.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG58.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG59.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG59.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG60.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG60.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG61.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG61.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG62.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG62.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG63.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG63.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG64.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG64.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG65.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG65.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG66.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG66.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG67.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG67.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG68.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG68.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG69.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG69.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG70.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG70.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG71.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG71.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG72.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG72.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG73.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG73.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG74.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG74.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG75.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG75.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG76.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG76.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG77.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG77.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG78.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG78.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_LAMBDA_CTRL_REG79.u32),
		channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG79.u32);
}

void vedu_hal_cfg_reg_qpg_map_set(const struct encode_info *channel_cfg,
	uint32_t *reg_base)
{
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_CFG0.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_CFG1.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG1.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_CFG2.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG2.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_0.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_0.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_1.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_1.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_2.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_2.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_3.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_3.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_4.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_4.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_5.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_5.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_6.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_6.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_7.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_7.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_0.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_START_0.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_1.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_START_1.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_2.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_START_2.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_3.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_START_3.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_4.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_START_4.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_5.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_START_5.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_6.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_START_6.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_7.u32),
		channel_cfg->all_reg.VEDU_VCTRL_ROI_START_7.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_RC_ENABLE.u32),
		channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_SMART_REG.u32),
		channel_cfg->all_reg.VEDU_QPG_SMART_REG.u32);
}

void vedu_hal_cfg_reg_addr_set(const struct encode_info *channel_cfg,
	uint32_t *reg_base)
{
	HI_DBG_VENC("set Vedu Hal cfg reg addr");
	/* Col_frm_flag PMC_POC And So On; I P B All need to Cfg */
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_STRFMT.u32),
		channel_cfg->all_reg.VEDU_VCPI_STRFMT.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REF_FLAG.u32),
		channel_cfg->all_reg.VEDU_VCPI_REF_FLAG.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PMV_POC_0), channel_cfg->all_reg.VEDU_PMV_POC_0);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PMV_POC_1), channel_cfg->all_reg.VEDU_PMV_POC_1);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PMV_POC_2), channel_cfg->all_reg.VEDU_PMV_POC_2);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PMV_POC_3), channel_cfg->all_reg.VEDU_PMV_POC_3);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PMV_POC_4), channel_cfg->all_reg.VEDU_PMV_POC_4);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PMV_POC_5), channel_cfg->all_reg.VEDU_PMV_POC_5);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_TUNLCELL_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_TUNLCELL_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SRC_YADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_SRC_YADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SRC_CADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_SRC_CADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SRC_VADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_SRC_VADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_TUNLCELL_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_TUNLCELL_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SRC_YADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_SRC_YADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SRC_CADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_SRC_CADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SRC_VADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_SRC_VADDR_H);
	/* hivcodec500 stream */
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_STRMADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_STRMADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_STRMADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_STRMADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SWPTRADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_SWPTRADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SWPTRADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_SWPTRADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SRPTRADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_SRPTRADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SRPTRADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_SRPTRADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_STRIDE.u32),
		channel_cfg->all_reg.VEDU_VCPI_STRIDE.u32);

	if (channel_cfg->all_reg.VEDU_VCPI_STRFMT.bits.vcpi_str_fmt == 10) {
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_YH_ADDR_L),
			channel_cfg->all_reg.VEDU_VCPI_YH_ADDR_L);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_CH_ADDR_L),
			channel_cfg->all_reg.VEDU_VCPI_CH_ADDR_L);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_YH_ADDR_H),
			channel_cfg->all_reg.VEDU_VCPI_YH_ADDR_H);
		vedu_hal_cfg(reg_base,
			 offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_CH_ADDR_H),
			 channel_cfg->all_reg.VEDU_VCPI_CH_ADDR_H);
		vedu_hal_cfg(reg_base, offsetof(S_HEVC_AVC_REGS_TYPE,
			VEDU_VCPI_EXT_STRIDE.u32),
			channel_cfg->all_reg.VEDU_VCPI_EXT_STRIDE.u32);
		vedu_hal_cfg(reg_base, offsetof(S_HEVC_AVC_REGS_TYPE,
			VEDU_CURLD_OSD01_ALPHA.u32),
			channel_cfg->all_reg.VEDU_CURLD_OSD01_ALPHA.u32);
	}

	// hivcodec500 36bit va low-----REC,REF0,REF1,PME0,PME1,NBI Relate ADDR
	// Stide---------REC--------------REC-------------REC------------------
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REC_YADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_REC_YADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REC_CADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_REC_CADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REC_STRIDE.u32),
		channel_cfg->all_reg.VEDU_VCPI_REC_STRIDE.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REC_YH_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_REC_YH_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REC_CH_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_REC_CH_ADDR_L);
	// REF--------------REF----------------REF-------------------REF0
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFY_L0_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_REFY_L0_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFC_L0_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_REFC_L0_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REF_L0_STRIDE.u32),
		channel_cfg->all_reg.VEDU_VCPI_REF_L0_STRIDE.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFYH_L0_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_REFYH_L0_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFCH_L0_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_REFCH_L0_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMELD_L0_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_PMELD_L0_ADDR_L);
	/* REF1 */
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFY_L1_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_REFY_L1_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFC_L1_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_REFC_L1_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REF_L1_STRIDE.u32),
		channel_cfg->all_reg.VEDU_VCPI_REF_L1_STRIDE.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFYH_L1_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_REFYH_L1_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFCH_L1_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_REFCH_L1_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMELD_L1_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_PMELD_L1_ADDR_L);
	/* PME NBI */
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMEST_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_PMEST_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMEST_STRIDE),
		channel_cfg->all_reg.VEDU_VCPI_PMEST_STRIDE);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMELD_STRIDE),
		channel_cfg->all_reg.VEDU_VCPI_PMELD_STRIDE);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_NBI_MVST_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_NBI_MVST_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_NBI_MVLD_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_NBI_MVLD_ADDR_L);
	/* PMEINFO */
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMEINFO_ST_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_PMEINFO_ST_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMEINFO_LD0_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_PMEINFO_LD0_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMEINFO_LD1_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_PMEINFO_LD1_ADDR_L);
	/* QPGLD */
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_QPGLD_INF_ADDR_L),
		channel_cfg->all_reg.VEDU_VCPI_QPGLD_INF_ADDR_L);
	// hivcodec500 36bit va high-----REC,REF0,REF1,PME0,PME1,NBI Relate ADDR
	// Stide----------REC---------------REC-------------REC----------------
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REC_YADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_REC_YADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REC_CADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_REC_CADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REC_YH_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_REC_YH_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REC_CH_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_REC_CH_ADDR_H);
	// REF-------------------REF-----------------REF-------------REF0
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFY_L0_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_REFY_L0_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFC_L0_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_REFC_L0_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFYH_L0_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_REFYH_L0_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFCH_L0_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_REFCH_L0_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMELD_L0_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_PMELD_L0_ADDR_H);
	/* REF1 */
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFY_L1_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_REFY_L1_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFC_L1_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_REFC_L1_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFYH_L1_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_REFYH_L1_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REFCH_L1_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_REFCH_L1_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMELD_L1_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_PMELD_L1_ADDR_H);
	/* PME NBI */
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMEST_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_PMEST_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_NBI_MVST_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_NBI_MVST_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_NBI_MVLD_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_NBI_MVLD_ADDR_H);
	/* PMEINFO */
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMEINFO_ST_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_PMEINFO_ST_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMEINFO_LD0_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_PMEINFO_LD0_ADDR_H);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PMEINFO_LD1_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_PMEINFO_LD1_ADDR_H);
	/* QPGLD */
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_QPGLD_INF_ADDR_H),
		channel_cfg->all_reg.VEDU_VCPI_QPGLD_INF_ADDR_H);
	/* skipweight */
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SKIPWEIGHT_LD_ADDR_L),
		channel_cfg->all_reg.VEDU_SKIPWEIGHT_LD_ADDR_L);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SKIPWEIGHT_LD_ADDR_H),
		channel_cfg->all_reg.VEDU_SKIPWEIGHT_LD_ADDR_H);
}

void vedu_hal_cfg_reg_slc_head_set(const struct encode_info *channel_cfg,
	uint32_t *reg_base)
{
	// ----------------Stream head info----------------------------
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLCST_STRMBUFLEN),
		channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_GLB_CFG.u32),
		channel_cfg->all_reg.VEDU_CABAC_GLB_CFG.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_SLCHDR_SIZE.u32),
		channel_cfg->all_reg.VEDU_CABAC_SLCHDR_SIZE.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_SLCHDR_PART1.u32),
		channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART1.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_VLC_CONFIG.u32),
		channel_cfg->all_reg.VEDU_VCPI_VLC_CONFIG.u32);
	if (channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_protocol == VEDU_H265) {
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_SLCHDR_PART2_SEG1),
			channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG1);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_SLCHDR_PART2_SEG2),
			channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG2);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_SLCHDR_PART2_SEG3),
			channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG3);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_SLCHDR_PART2_SEG4),
			channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG4);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_SLCHDR_PART2_SEG5),
			channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG5);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_SLCHDR_PART2_SEG6),
			channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG6);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_SLCHDR_PART2_SEG7),
			channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG7);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_SLCHDR_PART2_SEG8),
			channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG8);
	} else {
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLC_SLCHDRSTRM0),
			channel_cfg->all_reg.VEDU_VLC_SLCHDRSTRM0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLC_SLCHDRSTRM1),
			channel_cfg->all_reg.VEDU_VLC_SLCHDRSTRM1);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLC_SLCHDRSTRM2),
			channel_cfg->all_reg.VEDU_VLC_SLCHDRSTRM2);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLC_SLCHDRSTRM3),
			channel_cfg->all_reg.VEDU_VLC_SLCHDRSTRM3);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLC_REORDERSTRM0),
			channel_cfg->all_reg.VEDU_VLC_REORDERSTRM0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLC_REORDERSTRM1),
			channel_cfg->all_reg.VEDU_VLC_REORDERSTRM1);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLC_MARKINGSTRM0),
			channel_cfg->all_reg.VEDU_VLC_MARKINGSTRM0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLC_MARKINGSTRM1),
			channel_cfg->all_reg.VEDU_VLC_MARKINGSTRM1);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLC_SLCHDRPARA.u32),
			channel_cfg->all_reg.VEDU_VLC_SLCHDRPARA.u32);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLC_SVC.u32),
			channel_cfg->all_reg.VEDU_VLC_SVC.u32);
	}
}

void vedu_hal_cfgreg_rcset(const struct encode_info *channel_cfg, uint32_t *reg_base)
{
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CU_QP_DELTA_THRESH_REG0.u32),
		channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG0.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CU_QP_DELTA_THRESH_REG1.u32),
		channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG1.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CU_QP_DELTA_THRESH_REG2.u32),
		channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG2.u32);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CU_QP_DELTA_THRESH_REG3.u32),
		channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG3.u32);

	{
		U_VEDU_QPG_MADI_SWITCH_THR D32;

		D32.bits.qpg_qp_madi_switch_thr =
			channel_cfg->all_reg.VEDU_QPG_MADI_SWITCH_THR.bits.qpg_qp_madi_switch_thr;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_MADI_SWITCH_THR.u32),
			D32.u32);
	}
	HI_DBG_VENC("----qpg_qp_madi_switch_thr:%d----",
		channel_cfg->all_reg.VEDU_QPG_MADI_SWITCH_THR.bits.qpg_qp_madi_switch_thr);
	HI_DBG_VENC("veduhal_cfgreg_rcset");
}

void vedu_hal_cfg_reg_simple(struct encode_info *channel_cfg, uint32_t core_id)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	uint32_t *reg_base = venc->ctx[core_id].reg_base;

	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_FRAMENO),
		channel_cfg->all_reg.VEDU_VCPI_FRAMENO);

	{
		U_VEDU_QPG_MAX_MIN_QP D32;

		D32.bits.qpg_min_qp =
			channel_cfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_min_qp;
		D32.bits.qpg_max_qp =
			channel_cfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_max_qp;
		D32.bits.qpg_cu_qp_delta_enable_flag = 1;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_MAX_MIN_QP.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_MODE D32;

		D32.bits.vcpi_refc_nload =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_refc_nload;
		D32.bits.vcpi_ref_cmp_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_ref_cmp_en;
		D32.bits.vcpi_time_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_time_en;
		D32.bits.vcpi_10bit_mode =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_10bit_mode;
		D32.bits.vcpi_tiles_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_tiles_en;
		D32.bits.vcpi_high_speed_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_high_speed_en;
		D32.bits.vcpi_sobel_weight_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sobel_weight_en;
		D32.bits.vcpi_pr_inter_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_pr_inter_en;
		D32.bits.vcpi_blk8_inter =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_blk8_inter;
		D32.bits.vcpi_trans_mode =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_trans_mode;
		D32.bits.vcpi_pskip_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_pskip_en;
		D32.bits.vcpi_ref_num =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_ref_num;
		D32.bits.vcpi_long_term_refpic =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_long_term_refpic;
		D32.bits.vcpi_entropy_mode =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_entropy_mode;
		D32.bits.vcpi_frame_type =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_frame_type;
		D32.bits.vcpi_protocol =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_protocol;

		if (channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_frame_type == 0)
			D32.bits.vcpi_idr_pic = 1;
		else
			D32.bits.vcpi_idr_pic = 0;

		D32.bits.vcpi_img_improve_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_img_improve_en;
		D32.bits.vcpi_rec_cmp_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_rec_cmp_en;
		D32.bits.vcpi_sao_chroma =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sao_chroma;
		D32.bits.vcpi_sao_luma =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sao_luma;
		D32.bits.vcpi_slice_int_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_slice_int_en;
		D32.bits.vcpi_cfg_mode =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_cfg_mode;
		D32.bits.vedu_selfrst_en = 0;
		D32.bits.vcpi_vedsel =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_vedsel;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_MODE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_QPCFG D32;

		D32.bits.vcpi_cb_qp_offset =
			channel_cfg->all_reg.VEDU_VCPI_QPCFG.bits.vcpi_cb_qp_offset;
		D32.bits.vcpi_cr_qp_offset =
			channel_cfg->all_reg.VEDU_VCPI_QPCFG.bits.vcpi_cr_qp_offset;
		D32.bits.vcpi_frm_qp =
			channel_cfg->all_reg.VEDU_VCPI_QPCFG.bits.vcpi_frm_qp;
		D32.bits.vcpi_intra_det_qp_en =
			channel_cfg->all_reg.VEDU_VCPI_QPCFG.bits.vcpi_intra_det_qp_en;
		D32.bits.vcpi_rc_cu_madi_en =
			channel_cfg->all_reg.VEDU_VCPI_QPCFG.bits.vcpi_rc_cu_madi_en;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_QPCFG.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_AVERAGE_LCU_BITS D32;

		D32.bits.qpg_ave_lcu_bits =
			channel_cfg->all_reg.VEDU_QPG_AVERAGE_LCU_BITS.bits.qpg_ave_lcu_bits;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_AVERAGE_LCU_BITS.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_ROW_TARGET_BITS D32;

		D32.bits.qpg_qp_delta =
			channel_cfg->all_reg.VEDU_QPG_ROW_TARGET_BITS.bits.qpg_qp_delta;
		D32.bits.qpg_row_target_bits =
			channel_cfg->all_reg.VEDU_QPG_ROW_TARGET_BITS.bits.qpg_row_target_bits;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_ROW_TARGET_BITS.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_SKIN D32;

		D32.bits.qpg_skin_qp_delta =
			channel_cfg->all_reg.VEDU_QPG_SKIN.bits.qpg_skin_qp_delta;
		D32.bits.qpg_skin_max_qp =
			channel_cfg->all_reg.VEDU_QPG_SKIN.bits.qpg_skin_max_qp;
		D32.bits.qpg_skin_min_qp =
			channel_cfg->all_reg.VEDU_QPG_SKIN.bits.qpg_skin_min_qp;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_SKIN.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_TMV_LOAD D32;

		D32.bits.vcpi_tmv_wr_rd_avail =
			channel_cfg->all_reg.VEDU_VCPI_TMV_LOAD.bits.vcpi_tmv_wr_rd_avail;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_TMV_LOAD.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_MULTISLC D32;

		D32.bits.vcpi_multislc_en =
			channel_cfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_multislc_en;
		D32.bits.vcpi_slcspilt_mod =
			channel_cfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_slcspilt_mod;
		D32.bits.vcpi_slice_size =
			channel_cfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_slice_size;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_MULTISLC.u32),
			D32.u32);
	}

	vedu_hal_cfgreg_rcset(channel_cfg, reg_base);
	vedu_hal_cfg_reg_intraset(channel_cfg, reg_base);
	vedu_hal_cfg_reg_lambda_set(channel_cfg, reg_base);
	vedu_hal_cfg_reg_qpg_map_set(channel_cfg, reg_base);
	vedu_hal_cfg_reg_addr_set(channel_cfg, reg_base);
	vedu_hal_cfg_reg_slc_head_set(channel_cfg, reg_base);
	vedu_hal_cfg_smmu(channel_cfg, core_id);
}

void vedu_hal_cfg_reg(struct encode_info *channel_cfg, uint32_t core_id)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	uint32_t *reg_base = venc->ctx[core_id].reg_base;
	uint32_t vcpi_protocol =
		channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_protocol;

	vedu_hal_set_int(reg_base);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_FRAMENO),
		channel_cfg->all_reg.VEDU_VCPI_FRAMENO);

	{
		U_VEDU_BUS_IDLE_REQ D32;

		D32.u32 = 0;
		D32.bits.vedu_bus_idle_req = 0;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_BUS_IDLE_REQ.u32),
			D32.u32);
	}

	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_TIMEOUT),
		channel_cfg->all_reg.VEDU_VCPI_TIMEOUT);

	{
		U_VEDU_VCPI_MODE D32;

		D32.bits.vcpi_refc_nload =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_refc_nload;
		D32.bits.vcpi_ref_cmp_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_ref_cmp_en;
		D32.bits.vcpi_time_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_time_en;
		D32.bits.vcpi_10bit_mode =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_10bit_mode;
		D32.bits.vcpi_tiles_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_tiles_en;
		D32.bits.vcpi_high_speed_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_high_speed_en;
		D32.bits.vcpi_sobel_weight_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sobel_weight_en;
		D32.bits.vcpi_pr_inter_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_pr_inter_en;
		D32.bits.vcpi_blk8_inter =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_blk8_inter;
		D32.bits.vcpi_trans_mode =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_trans_mode;
		D32.bits.vcpi_pskip_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_pskip_en;
		D32.bits.vcpi_ref_num =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_ref_num;
		D32.bits.vcpi_long_term_refpic =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_long_term_refpic;
		D32.bits.vcpi_entropy_mode =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_entropy_mode;
		D32.bits.vcpi_frame_type =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_frame_type;
		D32.bits.vcpi_protocol =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_protocol;

		if (channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_frame_type == 0)
			D32.bits.vcpi_idr_pic = 1;
		else
			D32.bits.vcpi_idr_pic = 0;

		D32.bits.vcpi_img_improve_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_img_improve_en;
		D32.bits.vcpi_rec_cmp_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_rec_cmp_en;
		D32.bits.vcpi_sao_chroma =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sao_chroma;
		D32.bits.vcpi_sao_luma =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sao_luma;
		D32.bits.vcpi_slice_int_en =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_slice_int_en;
		D32.bits.vcpi_cfg_mode =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_cfg_mode;
		D32.bits.vedu_selfrst_en = 0;
		D32.bits.vcpi_vedsel =
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_vedsel;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_MODE.u32), D32.u32);
	}

	{
		U_VEDU_EMAR_SCRAMBLE_TYPE D32;

		D32.bits.emar_rec_b7_scramble = 0;
		D32.bits.emar_rec_b8_scramble = 0;
		D32.bits.emar_ori_y_b7_scramble = 0;
		D32.bits.emar_ori_y_b8_scramble = 0;
		D32.bits.emar_ori_y_b9_scramble = 0;
		D32.bits.emar_ori_uv_b7_scramble = 0;
		D32.bits.emar_ori_uv_b8_scramble = 0;
		D32.bits.emar_ori_uv_b9_scramble = 0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_EMAR_SCRAMBLE_TYPE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_LOW_POWER D32;

		D32.bits.vcpi_osd_clkgete_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_osd_clkgete_en;
		D32.bits.vcpi_ghdr_clkgate_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_ghdr_clkgate_en;
		D32.bits.vcpi_curld_dcmp_clkgate_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_curld_dcmp_clkgate_en;
		D32.bits.vcpi_refld_dcmp_clkgate_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_refld_dcmp_clkgate_en;
		D32.bits.vcpi_cpi_clkgate_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_cpi_clkgate_en;
		D32.bits.vcpi_ddr_cross_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_ddr_cross_en;
		D32.bits.vcpi_mem_clkgate_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_mem_clkgate_en;
		D32.bits.vcpi_clkgate_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_clkgate_en;
		D32.bits.vcpi_mrg_gtck_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_mrg_gtck_en;
		D32.bits.vcpi_tqitq_gtck_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_tqitq_gtck_en;
		D32.bits.vcpi_ddr_cross_idx =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_ddr_cross_idx;
		D32.bits.vcpi_ime_lowpow_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_ime_lowpow_en;
		D32.bits.vcpi_fme_lowpow_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_fme_lowpow_en;
		D32.bits.powerpro_func_bypass = 0;
		D32.bits.vcpi_intra_lowpow_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_intra_lowpow_en;
		D32.bits.vcpi_vlc_clkgate_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_vlc_clkgate_en;
		D32.bits.vcpi_recst_hfbc_clkgate_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_recst_hfbc_clkgate_en;
#ifndef VENC_DPM_ENABLE
		D32.bits.venc_dpm_apb_clk_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.venc_dpm_apb_clk_en;
#else
		D32.bits.venc_dpm_apb_clk_en = 1;
#endif
		D32.bits.venc_axi_dfx_clk_en =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.venc_axi_dfx_clk_en;
		D32.bits.venc_apb_cfg_force_clk_on =
			channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.venc_apb_cfg_force_clk_on;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_LOW_POWER.u32), D32.u32);
	}

	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_LLILD_ADDR_L), 0x0);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_LLILD_ADDR_H), 0x0);

	{
		U_VEDU_VCPI_RC_ENABLE D32;

		D32.bits.vcpi_rc_cu_qp_en =
			channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.bits.vcpi_rc_cu_qp_en;
		D32.bits.vcpi_rc_row_qp_en =
			channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.bits.vcpi_rc_row_qp_en;
		D32.bits.vcpi_move_scene_en =
			channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.bits.vcpi_move_scene_en;
		D32.bits.vcpi_strong_edge_move_en =
			channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.bits.vcpi_strong_edge_move_en;
		D32.bits.vcpi_rc_low_luma_en =
			channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.bits.vcpi_rc_low_luma_en;
		D32.bits.vcpi_rd_min_sad_flag_en =
			channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.bits.vcpi_prev_min_sad_en;
		D32.bits.vcpi_low_min_sad_en =
			channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.bits.vcpi_low_min_sad_en;
		D32.bits.vcpi_prev_min_sad_en =
			channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.bits.vcpi_prev_min_sad_en;
		D32.bits.vcpi_qpgld_en =
			channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.bits.vcpi_qpgld_en;
		D32.bits.vcpi_map_roikeep_en =
			channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.bits.vcpi_map_roikeep_en;
		D32.bits.vcpi_flat_region_en =
			channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.bits.vcpi_flat_region_en;
		D32.bits.vcpi_qp_restrain_large_sad =
			channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.bits.vcpi_qp_restrain_large_sad;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_RC_ENABLE.u32),
			D32.u32);
	}

	{
		U_VEDU_VLCST_PTBITS_EN D32;

		D32.bits.vlcst_ptbits_en = 0;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLCST_PTBITS_EN.u32),
			D32.u32);
	}

	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLCST_PTBITS), 0);

	{
		U_VEDU_PPFD_ST_CFG D32;

		D32.u32 = 0;
		D32.bits.ppfd_st_bypass_en = 1;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PPFD_ST_CFG.u32),
			D32.u32);
	}

	{
		U_VEDU_IME_INTER_MODE D32;

		D32.bits.tile_boundry_improve_en =
			channel_cfg->all_reg.VEDU_IME_INTER_MODE.bits.tile_boundry_improve_en;
		D32.bits.ime_intra4_lowpow_en =
			channel_cfg->all_reg.VEDU_IME_INTER_MODE.bits.ime_intra4_lowpow_en;
		D32.bits.ime_high3pre_en =
			channel_cfg->all_reg.VEDU_IME_INTER_MODE.bits.ime_high3pre_en;
		D32.bits.ime_flat_region_force_low3layer =
			channel_cfg->all_reg.VEDU_IME_INTER_MODE.bits.ime_flat_region_force_low3layer;
		D32.bits.ime_inter8x8_en =
			channel_cfg->all_reg.VEDU_IME_INTER_MODE.bits.ime_inter8x8_en;
		D32.bits.ime_layer3to2_en =
			channel_cfg->all_reg.VEDU_IME_INTER_MODE.bits.ime_layer3to2_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_IME_INTER_MODE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_QPCFG D32;

		D32.bits.vcpi_cb_qp_offset =
			channel_cfg->all_reg.VEDU_VCPI_QPCFG.bits.vcpi_cb_qp_offset;
		D32.bits.vcpi_cr_qp_offset =
			channel_cfg->all_reg.VEDU_VCPI_QPCFG.bits.vcpi_cr_qp_offset;
		D32.bits.vcpi_frm_qp =
			channel_cfg->all_reg.VEDU_VCPI_QPCFG.bits.vcpi_frm_qp;
		D32.bits.vcpi_intra_det_qp_en =
			channel_cfg->all_reg.VEDU_VCPI_QPCFG.bits.vcpi_intra_det_qp_en;
		D32.bits.vcpi_rc_cu_madi_en =
			channel_cfg->all_reg.VEDU_VCPI_QPCFG.bits.vcpi_rc_cu_madi_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_QPCFG.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_MAX_MIN_QP D32;

		D32.bits.qpg_max_qp =
			channel_cfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_max_qp;
		D32.bits.qpg_cu_qp_delta_enable_flag =
			channel_cfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_cu_qp_delta_enable_flag;
		D32.bits.qpg_min_qp =
			channel_cfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_min_qp;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_MAX_MIN_QP.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_SMART_REG D32;

		D32.bits.qpg_smart_get_cu32_qp_mode =
			channel_cfg->all_reg.VEDU_QPG_SMART_REG.bits.qpg_smart_get_cu32_qp_mode;
		D32.bits.qpg_smart_get_cu64_qp_mode =
			channel_cfg->all_reg.VEDU_QPG_SMART_REG.bits.qpg_smart_get_cu64_qp_mode;
		D32.bits.qpg_qp_detlta_size_cu64 =
			channel_cfg->all_reg.VEDU_QPG_SMART_REG.bits.qpg_qp_detlta_size_cu64;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_SMART_REG.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_ROW_TARGET_BITS D32;
		D32.bits.qpg_qp_delta =
			channel_cfg->all_reg.VEDU_QPG_ROW_TARGET_BITS.bits.qpg_qp_delta;
		D32.bits.qpg_row_target_bits =
			channel_cfg->all_reg.VEDU_QPG_ROW_TARGET_BITS.bits.qpg_row_target_bits;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_ROW_TARGET_BITS.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_AVERAGE_LCU_BITS D32;

		D32.bits.qpg_ave_lcu_bits =
			channel_cfg->all_reg.VEDU_QPG_AVERAGE_LCU_BITS.bits.qpg_ave_lcu_bits;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_AVERAGE_LCU_BITS.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_CU_QP_DELTA_THRESH_REG0 D32;

		D32.bits.qpg_cu_qp_delta_thresh3 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG0.bits.qpg_cu_qp_delta_thresh3;
		D32.bits.qpg_cu_qp_delta_thresh2 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG0.bits.qpg_cu_qp_delta_thresh2;
		D32.bits.qpg_cu_qp_delta_thresh1 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG0.bits.qpg_cu_qp_delta_thresh1;
		D32.bits.qpg_cu_qp_delta_thresh0 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG0.bits.qpg_cu_qp_delta_thresh0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CU_QP_DELTA_THRESH_REG0.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_CU_QP_DELTA_THRESH_REG1 D32;

		D32.bits.qpg_cu_qp_delta_thresh7 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG1.bits.qpg_cu_qp_delta_thresh7;
		D32.bits.qpg_cu_qp_delta_thresh6 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG1.bits.qpg_cu_qp_delta_thresh6;
		D32.bits.qpg_cu_qp_delta_thresh5 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG1.bits.qpg_cu_qp_delta_thresh5;
		D32.bits.qpg_cu_qp_delta_thresh4 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG1.bits.qpg_cu_qp_delta_thresh4;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CU_QP_DELTA_THRESH_REG1.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_CU_QP_DELTA_THRESH_REG2 D32;

		D32.bits.qpg_cu_qp_delta_thresh11 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG2.bits.qpg_cu_qp_delta_thresh11;
		D32.bits.qpg_cu_qp_delta_thresh10 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG2.bits.qpg_cu_qp_delta_thresh10;
		D32.bits.qpg_cu_qp_delta_thresh9 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG2.bits.qpg_cu_qp_delta_thresh9;
		D32.bits.qpg_cu_qp_delta_thresh8 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG2.bits.qpg_cu_qp_delta_thresh8;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CU_QP_DELTA_THRESH_REG2.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_CU_QP_DELTA_THRESH_REG3 D32;

		D32.bits.qpg_cu_qp_delta_thresh15 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG3.bits.qpg_cu_qp_delta_thresh15;
		D32.bits.qpg_cu_qp_delta_thresh14 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG3.bits.qpg_cu_qp_delta_thresh14;
		D32.bits.qpg_cu_qp_delta_thresh13 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG3.bits.qpg_cu_qp_delta_thresh13;
		D32.bits.qpg_cu_qp_delta_thresh12 =
			channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG3.bits.qpg_cu_qp_delta_thresh12;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CU_QP_DELTA_THRESH_REG3.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_DELTA_LEVEL D32;

		D32.bits.qpg_qp_delta_level_0 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_0;
		D32.bits.qpg_qp_delta_level_1 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_1;
		D32.bits.qpg_qp_delta_level_2 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_2;
		D32.bits.qpg_qp_delta_level_3 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_3;
		D32.bits.qpg_qp_delta_level_4 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_4;
		D32.bits.qpg_qp_delta_level_5 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_5;
		D32.bits.qpg_qp_delta_level_6 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_6;
		D32.bits.qpg_qp_delta_level_7 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_7;
		D32.bits.qpg_qp_delta_level_8 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_8;
		D32.bits.qpg_qp_delta_level_9 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_9;
		D32.bits.qpg_qp_delta_level_10 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_10;
		D32.bits.qpg_qp_delta_level_11 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_11;
		D32.bits.qpg_qp_delta_level_12 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_12;
		D32.bits.qpg_qp_delta_level_13 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_13;
		D32.bits.qpg_qp_delta_level_14 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_14;
		D32.bits.qpg_qp_delta_level_15 =
			channel_cfg->all_reg.VEDU_QPG_DELTA_LEVEL.bits.qpg_qp_delta_level_15;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_DELTA_LEVEL.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_MADI_SWITCH_THR D32;

		D32.bits.qpg_qp_madi_switch_thr =
			channel_cfg->all_reg.VEDU_QPG_MADI_SWITCH_THR.bits.qpg_qp_madi_switch_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_MADI_SWITCH_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_LOWLUMA D32;

		D32.bits.qpg_lowluma_min_qp =
			channel_cfg->all_reg.VEDU_QPG_LOWLUMA.bits.qpg_lowluma_min_qp;
		D32.bits.qpg_lowluma_max_qp =
			channel_cfg->all_reg.VEDU_QPG_LOWLUMA.bits.qpg_lowluma_max_qp;
		D32.bits.qpg_lowluma_qp_delta =
			channel_cfg->all_reg.VEDU_QPG_LOWLUMA.bits.qpg_lowluma_qp_delta;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_LOWLUMA.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_HEDGE D32;

		D32.bits.qpg_hedge_mim_qp =
			channel_cfg->all_reg.VEDU_QPG_HEDGE.bits.qpg_hedge_mim_qp;
		D32.bits.qpg_hedge_max_qp =
			channel_cfg->all_reg.VEDU_QPG_HEDGE.bits.qpg_hedge_max_qp;
		D32.bits.qpg_hedge_qp_delta =
			channel_cfg->all_reg.VEDU_QPG_HEDGE.bits.qpg_hedge_qp_delta;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_HEDGE.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_HEDGE_MOVE D32;

		D32.bits.qpg_hedge_move_min_qp =
			channel_cfg->all_reg.VEDU_QPG_HEDGE_MOVE.bits.qpg_hedge_move_min_qp;
		D32.bits.qpg_hedge_move_max_qp =
			channel_cfg->all_reg.VEDU_QPG_HEDGE_MOVE.bits.qpg_hedge_move_max_qp;
		D32.bits.qpg_hedge_move_qp_delta =
			channel_cfg->all_reg.VEDU_QPG_HEDGE_MOVE.bits.qpg_hedge_move_qp_delta;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_HEDGE_MOVE.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_SKIN D32;

		D32.bits.qpg_skin_qp_delta =
			channel_cfg->all_reg.VEDU_QPG_SKIN.bits.qpg_skin_qp_delta;
		D32.bits.qpg_skin_max_qp =
			channel_cfg->all_reg.VEDU_QPG_SKIN.bits.qpg_skin_max_qp;
		D32.bits.qpg_skin_min_qp =
			channel_cfg->all_reg.VEDU_QPG_SKIN.bits.qpg_skin_min_qp;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_SKIN.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_INTRA_DET D32;

		D32.bits.qpg_intra_det_qp_delta =
			channel_cfg->all_reg.VEDU_QPG_INTRA_DET.bits.qpg_intra_det_qp_delta;
		D32.bits.qpg_intra_det_max_qp =
			channel_cfg->all_reg.VEDU_QPG_INTRA_DET.bits.qpg_intra_det_max_qp;
		D32.bits.qpg_intra_det_min_qp =
			channel_cfg->all_reg.VEDU_QPG_INTRA_DET.bits.qpg_intra_det_min_qp;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_INTRA_DET.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_CU32_DELTA D32;

		D32.bits.qpg_cu32_delta_low =
			channel_cfg->all_reg.VEDU_QPG_CU32_DELTA.bits.qpg_cu32_delta_low;
		D32.bits.qpg_cu32_delta_high =
			channel_cfg->all_reg.VEDU_QPG_CU32_DELTA.bits.qpg_cu32_delta_high;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CU32_DELTA.u32),
			D32.u32);
	}

	{
		U_VEDU_TILE_RC D32;

		D32.bits.vcpi_tile_bound_qp_delta =
			channel_cfg->all_reg.VEDU_TILE_RC.bits.vcpi_tile_bound_qp_delta;
		D32.bits.vcpi_tile_line_qp_delta =
			channel_cfg->all_reg.VEDU_TILE_RC.bits.vcpi_tile_line_qp_delta;
		D32.bits.vcpi_tile_level_rc_en =
			channel_cfg->all_reg.VEDU_TILE_RC.bits.vcpi_tile_level_rc_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_TILE_RC.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_LAMBDA_MODE D32;

		D32.bits.qpg_lambda_qp_offset =
			channel_cfg->all_reg.VEDU_QPG_LAMBDA_MODE.bits.qpg_lambda_qp_offset;
		D32.bits.qpg_rdo_lambda_choose_mode =
			channel_cfg->all_reg.VEDU_QPG_LAMBDA_MODE.bits.qpg_rdo_lambda_choose_mode;
		D32.bits.qpg_lambda_inter_stredge_en =
			channel_cfg->all_reg.VEDU_QPG_LAMBDA_MODE.bits.qpg_lambda_inter_stredge_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_LAMBDA_MODE.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_QP_RESTRAIN D32;

		D32.bits.qpg_qp_restrain_madi_thr =
			channel_cfg->all_reg.VEDU_QPG_QP_RESTRAIN.bits.qpg_qp_restrain_madi_thr;
		D32.bits.qpg_qp_restrain_mode =
			channel_cfg->all_reg.VEDU_QPG_QP_RESTRAIN.bits.qpg_qp_restrain_mode;
		D32.bits.qpg_qp_restrain_en =
			channel_cfg->all_reg.VEDU_QPG_QP_RESTRAIN.bits.qpg_qp_restrain_en;
		D32.bits.qpg_qp_restrain_delta_blk32 =
			channel_cfg->all_reg.VEDU_QPG_QP_RESTRAIN.bits.qpg_qp_restrain_delta_blk32;
		D32.bits.qpg_qp_restrain_delta_blk16 =
			channel_cfg->all_reg.VEDU_QPG_QP_RESTRAIN.bits.qpg_qp_restrain_delta_blk16;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_QP_RESTRAIN.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_CU_MIN_SAD_REG D32;

		D32.bits.qpg_min_sad_level =
			channel_cfg->all_reg.VEDU_QPG_CU_MIN_SAD_REG.bits.qpg_min_sad_level;
		D32.bits.qpg_low_min_sad_mode =
			channel_cfg->all_reg.VEDU_QPG_CU_MIN_SAD_REG.bits.qpg_low_min_sad_mode;
		D32.bits.qpg_min_sad_madi_en =
			channel_cfg->all_reg.VEDU_QPG_CU_MIN_SAD_REG.bits.qpg_min_sad_madi_en;
		D32.bits.qpg_min_sad_qp_restrain_en =
			channel_cfg->all_reg.VEDU_QPG_CU_MIN_SAD_REG.bits.qpg_min_sad_qp_restrain_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CU_MIN_SAD_REG.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_FLAT_REGION D32;

		D32.bits.qpg_flat_region_qp_delta =
			channel_cfg->all_reg.VEDU_QPG_FLAT_REGION.bits.qpg_flat_region_qp_delta;
		D32.bits.qpg_flat_region_max_qp =
			channel_cfg->all_reg.VEDU_QPG_FLAT_REGION.bits.qpg_flat_region_max_qp;
		D32.bits.qpg_flat_region_min_qp =
			channel_cfg->all_reg.VEDU_QPG_FLAT_REGION.bits.qpg_flat_region_min_qp;
		D32.bits.vcpi_cu32_use_cu16_mean_en =
			channel_cfg->all_reg.VEDU_QPG_FLAT_REGION.bits.vcpi_cu32_use_cu16_mean_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_FLAT_REGION.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_QPG_RES_COEF D32;

		D32.bits.vcpi_small_res_coef =
			channel_cfg->all_reg.VEDU_QPG_RES_COEF.bits.vcpi_small_res_coef;
		D32.bits.vcpi_large_res_coef =
			channel_cfg->all_reg.VEDU_QPG_RES_COEF.bits.vcpi_large_res_coef;
		D32.bits.vcpi_res_coef_en =
			channel_cfg->all_reg.VEDU_QPG_RES_COEF.bits.vcpi_res_coef_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_RES_COEF.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_QPG_CURR_SAD_EN D32;

		D32.bits.vcpi_rc_cu_sad_en =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_EN.bits.vcpi_rc_cu_sad_en;
		D32.bits.vcpi_sad_switch_thr =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_EN.bits.vcpi_sad_switch_thr;
		D32.bits.vcpi_rc_cu_sad_mod =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_EN.bits.vcpi_rc_cu_sad_mod;
		D32.bits.vcpi_rc_cu_sad_offset =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_EN.bits.vcpi_rc_cu_sad_offset;
		D32.bits.vcpi_rc_cu_sad_gain =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_EN.bits.vcpi_rc_cu_sad_gain;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CURR_SAD_EN.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_QPG_CURR_SAD_LEVEL D32;

		D32.bits.vcpi_curr_sad_level_15 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_15;
		D32.bits.vcpi_curr_sad_level_14 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_14;
		D32.bits.vcpi_curr_sad_level_13 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_13;
		D32.bits.vcpi_curr_sad_level_12 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_12;
		D32.bits.vcpi_curr_sad_level_11 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_11;
		D32.bits.vcpi_curr_sad_level_10 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_10;
		D32.bits.vcpi_curr_sad_level_9 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_9;
		D32.bits.vcpi_curr_sad_level_8 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_8;
		D32.bits.vcpi_curr_sad_level_7 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_7;
		D32.bits.vcpi_curr_sad_level_6 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_6;
		D32.bits.vcpi_curr_sad_level_5 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_5;
		D32.bits.vcpi_curr_sad_level_4 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_4;
		D32.bits.vcpi_curr_sad_level_3 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_3;
		D32.bits.vcpi_curr_sad_level_2 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_2;
		D32.bits.vcpi_curr_sad_level_1 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_1;
		D32.bits.vcpi_curr_sad_level_0 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_LEVEL.bits.vcpi_curr_sad_level_0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CURR_SAD_LEVEL.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_QPG_CURR_SAD_THRESH0 D32;

		D32.bits.vcpi_curr_sad_thresh_0 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH0.bits.vcpi_curr_sad_thresh_0;
		D32.bits.vcpi_curr_sad_thresh_1 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH0.bits.vcpi_curr_sad_thresh_1;
		D32.bits.vcpi_curr_sad_thresh_2 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH0.bits.vcpi_curr_sad_thresh_2;
		D32.bits.vcpi_curr_sad_thresh_3 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH0.bits.vcpi_curr_sad_thresh_3;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CURR_SAD_THRESH0.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_QPG_CURR_SAD_THRESH1 D32;

		D32.bits.vcpi_curr_sad_thresh_4 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH1.bits.vcpi_curr_sad_thresh_4;
		D32.bits.vcpi_curr_sad_thresh_5 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH1.bits.vcpi_curr_sad_thresh_5;
		D32.bits.vcpi_curr_sad_thresh_6 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH1.bits.vcpi_curr_sad_thresh_6;
		D32.bits.vcpi_curr_sad_thresh_7 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH1.bits.vcpi_curr_sad_thresh_7;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CURR_SAD_THRESH1.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_QPG_CURR_SAD_THRESH2 D32;

		D32.bits.vcpi_curr_sad_thresh_8 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH2.bits.vcpi_curr_sad_thresh_8;
		D32.bits.vcpi_curr_sad_thresh_9 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH2.bits.vcpi_curr_sad_thresh_9;
		D32.bits.vcpi_curr_sad_thresh_10 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH2.bits.vcpi_curr_sad_thresh_10;
		D32.bits.vcpi_curr_sad_thresh_11 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH2.bits.vcpi_curr_sad_thresh_11;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CURR_SAD_THRESH2.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_QPG_CURR_SAD_THRESH3 D32;

		D32.bits.vcpi_curr_sad_thresh_12 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH3.bits.vcpi_curr_sad_thresh_12;
		D32.bits.vcpi_curr_sad_thresh_13 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH3.bits.vcpi_curr_sad_thresh_13;
		D32.bits.vcpi_curr_sad_thresh_14 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH3.bits.vcpi_curr_sad_thresh_14;
		D32.bits.vcpi_curr_sad_thresh_15 =
			channel_cfg->all_reg.VEDU_QPG_CURR_SAD_THRESH3.bits.vcpi_curr_sad_thresh_15;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_CURR_SAD_THRESH3.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_LUMA_RC D32;

		D32.bits.vcpi_rc_luma_en =
			channel_cfg->all_reg.VEDU_LUMA_RC.bits.vcpi_rc_luma_en;
		D32.bits.vcpi_rc_luma_mode =
			channel_cfg->all_reg.VEDU_LUMA_RC.bits.vcpi_rc_luma_mode;
		D32.bits.vcpi_rc_luma_switch_thr =
			channel_cfg->all_reg.VEDU_LUMA_RC.bits.vcpi_rc_luma_switch_thr;
		D32.bits.vcpi_rc_luma_low_madi_thr =
			channel_cfg->all_reg.VEDU_LUMA_RC.bits.vcpi_rc_luma_low_madi_thr;
		D32.bits.vcpi_rc_luma_high_madi_thr =
			channel_cfg->all_reg.VEDU_LUMA_RC.bits.vcpi_rc_luma_high_madi_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_LUMA_RC.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_LUMA_LEVEL D32;

		D32.bits.vcpi_rc_luma_level_15 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_15;
		D32.bits.vcpi_rc_luma_level_14 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_14;
		D32.bits.vcpi_rc_luma_level_13 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_13;
		D32.bits.vcpi_rc_luma_level_12 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_12;
		D32.bits.vcpi_rc_luma_level_11 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_11;
		D32.bits.vcpi_rc_luma_level_10 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_10;
		D32.bits.vcpi_rc_luma_level_9 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_9;
		D32.bits.vcpi_rc_luma_level_8 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_8;
		D32.bits.vcpi_rc_luma_level_7 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_7;
		D32.bits.vcpi_rc_luma_level_6 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_6;
		D32.bits.vcpi_rc_luma_level_5 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_5;
		D32.bits.vcpi_rc_luma_level_4 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_4;
		D32.bits.vcpi_rc_luma_level_3 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_3;
		D32.bits.vcpi_rc_luma_level_2 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_2;
		D32.bits.vcpi_rc_luma_level_1 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_1;
		D32.bits.vcpi_rc_luma_level_0 =
			channel_cfg->all_reg.VEDU_LUMA_LEVEL.bits.vcpi_rc_luma_level_0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_LUMA_LEVEL.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_LUMA_THRESH0 D32;

		D32.bits.vcpi_rc_luma_thresh_0 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH0.bits.vcpi_rc_luma_thresh_0;
		D32.bits.vcpi_rc_luma_thresh_1 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH0.bits.vcpi_rc_luma_thresh_1;
		D32.bits.vcpi_rc_luma_thresh_2 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH0.bits.vcpi_rc_luma_thresh_2;
		D32.bits.vcpi_rc_luma_thresh_3 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH0.bits.vcpi_rc_luma_thresh_3;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_LUMA_THRESH0.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_LUMA_THRESH1 D32;

		D32.bits.vcpi_rc_luma_thresh_4 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH1.bits.vcpi_rc_luma_thresh_4;
		D32.bits.vcpi_rc_luma_thresh_5 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH1.bits.vcpi_rc_luma_thresh_5;
		D32.bits.vcpi_rc_luma_thresh_6 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH1.bits.vcpi_rc_luma_thresh_6;
		D32.bits.vcpi_rc_luma_thresh_7 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH1.bits.vcpi_rc_luma_thresh_7;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_LUMA_THRESH1.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_LUMA_THRESH2 D32;

		D32.bits.vcpi_rc_luma_thresh_8 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH2.bits.vcpi_rc_luma_thresh_8;
		D32.bits.vcpi_rc_luma_thresh_9 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH2.bits.vcpi_rc_luma_thresh_9;
		D32.bits.vcpi_rc_luma_thresh_10 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH2.bits.vcpi_rc_luma_thresh_10;
		D32.bits.vcpi_rc_luma_thresh_11 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH2.bits.vcpi_rc_luma_thresh_11;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_LUMA_THRESH2.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_LUMA_THRESH3 D32;

		D32.bits.vcpi_rc_luma_thresh_12 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH3.bits.vcpi_rc_luma_thresh_12;
		D32.bits.vcpi_rc_luma_thresh_13 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH3.bits.vcpi_rc_luma_thresh_13;
		D32.bits.vcpi_rc_luma_thresh_14 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH3.bits.vcpi_rc_luma_thresh_14;
		D32.bits.vcpi_rc_luma_thresh_15 =
			channel_cfg->all_reg.VEDU_LUMA_THRESH3.bits.vcpi_rc_luma_thresh_15;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_LUMA_THRESH3.u32),
			D32.u32);
	}

	{
		/* ADD IN V500 */
		U_VEDU_CHROMA_PROTECT D32;

		D32.bits.vcpi_chroma_qp_delta =
			channel_cfg->all_reg.VEDU_CHROMA_PROTECT.bits.vcpi_chroma_qp_delta;
		D32.bits.vcpi_chroma_in_qp =
			channel_cfg->all_reg.VEDU_CHROMA_PROTECT.bits.vcpi_chroma_in_qp;
		D32.bits.vcpi_chroma_max_qp =
			channel_cfg->all_reg.VEDU_CHROMA_PROTECT.bits.vcpi_chroma_max_qp;
		D32.bits.vcpi_chroma_prot_en =
			channel_cfg->all_reg.VEDU_CHROMA_PROTECT.bits.vcpi_chroma_prot_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CHROMA_PROTECT.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_QPG_RC_THR0 D32;

		D32.bits.pme_madi_dif_thr =
			channel_cfg->all_reg.VEDU_PME_QPG_RC_THR0.bits.pme_madi_dif_thr;
		D32.bits.pme_cur_madi_dif_thr =
			channel_cfg->all_reg.VEDU_PME_QPG_RC_THR0.bits.pme_cur_madi_dif_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_QPG_RC_THR0.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_QPG_RC_THR1 D32;

		D32.bits.pme_min_sad_thr_gain =
			channel_cfg->all_reg.VEDU_PME_QPG_RC_THR1.bits.pme_min_sad_thr_gain;
		D32.bits.pme_min_sad_thr_offset =
			channel_cfg->all_reg.VEDU_PME_QPG_RC_THR1.bits.pme_min_sad_thr_offset;
		D32.bits.pme_min_sad_thr_offset_cur =
			channel_cfg->all_reg.VEDU_PME_QPG_RC_THR1.bits.pme_min_sad_thr_gain_cur;
		D32.bits.pme_min_sad_thr_gain_cur =
			channel_cfg->all_reg.VEDU_PME_QPG_RC_THR1.bits.pme_min_sad_thr_offset_cur;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_QPG_RC_THR1.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_LOW_LUMA_THR D32;

		D32.bits.pme_low_luma_thr =
			channel_cfg->all_reg.VEDU_PME_LOW_LUMA_THR.bits.pme_low_luma_thr;
		D32.bits.pme_low_luma_madi_thr =
			channel_cfg->all_reg.VEDU_PME_LOW_LUMA_THR.bits.pme_low_luma_madi_thr;
		D32.bits.pme_high_luma_thr =
			channel_cfg->all_reg.VEDU_PME_LOW_LUMA_THR.bits.pme_high_luma_thr;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_LOW_LUMA_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_CHROMA_FLAT D32;

		D32.bits.pme_flat_u_thr_low =
			channel_cfg->all_reg.VEDU_PME_CHROMA_FLAT.bits.pme_flat_u_thr_low;
		D32.bits.pme_flat_u_thr_high =
			channel_cfg->all_reg.VEDU_PME_CHROMA_FLAT.bits.pme_flat_u_thr_high;
		D32.bits.pme_flat_v_thr_low =
			channel_cfg->all_reg.VEDU_PME_CHROMA_FLAT.bits.pme_flat_v_thr_low;
		D32.bits.pme_flat_v_thr_high =
			channel_cfg->all_reg.VEDU_PME_CHROMA_FLAT.bits.pme_flat_v_thr_high;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_CHROMA_FLAT.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_LUMA_FLAT D32;

		D32.bits.pme_flat_high_luma_thr =
			channel_cfg->all_reg.VEDU_PME_LUMA_FLAT.bits.pme_flat_high_luma_thr;
		D32.bits.pme_flat_low_luma_thr =
			channel_cfg->all_reg.VEDU_PME_LUMA_FLAT.bits.pme_flat_low_luma_thr;
		D32.bits.pme_flat_luma_madi_thr =
			channel_cfg->all_reg.VEDU_PME_LUMA_FLAT.bits.pme_flat_luma_madi_thr;
		D32.bits.pme_flat_pmemv_thr =
			channel_cfg->all_reg.VEDU_PME_LUMA_FLAT.bits.pme_flat_pmemv_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_LUMA_FLAT.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_MADI_FLAT D32;

		D32.bits.pme_flat_madi_times =
			channel_cfg->all_reg.VEDU_PME_MADI_FLAT.bits.pme_flat_madi_times;
		D32.bits.pme_flat_region_cnt =
			channel_cfg->all_reg.VEDU_PME_MADI_FLAT.bits.pme_flat_region_cnt;
		D32.bits.pme_flat_icount_thr =
			channel_cfg->all_reg.VEDU_PME_MADI_FLAT.bits.pme_flat_icount_thr;
		D32.bits.pme_flat_pmesad_thr =
			channel_cfg->all_reg.VEDU_PME_MADI_FLAT.bits.pme_flat_pmesad_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_MADI_FLAT.u32),
			D32.u32);
	}

	{
		U_VEDU_VLCST_DESCRIPTOR D32;

		D32.bits.vlcst_chnid =
			channel_cfg->all_reg.VEDU_VLCST_DESCRIPTOR.bits.vlcst_chnid;

		if (channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_frame_type == 0)
			D32.bits.vlcst_idrind = 1;
		else
			D32.bits.vlcst_idrind = 0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VLCST_DESCRIPTOR.u32),
			D32.u32);
	}

	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PPFD_ST_LEN0),
		channel_cfg->all_reg.VEDU_PPFD_ST_LEN0);

	{
		U_VEDU_CURLD_CLIP_LUMA D32;

		D32.bits.curld_clip_luma_min =
			channel_cfg->all_reg.VEDU_CURLD_CLIP_LUMA.bits.curld_clip_luma_min;
		D32.bits.curld_clip_luma_max =
			channel_cfg->all_reg.VEDU_CURLD_CLIP_LUMA.bits.curld_clip_luma_max;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_CLIP_LUMA.u32),
			D32.u32);
	}

	{
		U_VEDU_CURLD_CLIP_CHROMA D32;

		D32.bits.curld_clip_chroma_min =
			channel_cfg->all_reg.VEDU_CURLD_CLIP_CHROMA.bits.curld_clip_chroma_min;
		D32.bits.curld_clip_chroma_max =
			channel_cfg->all_reg.VEDU_CURLD_CLIP_CHROMA.bits.curld_clip_chroma_max;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_CLIP_CHROMA.u32),
			D32.u32);
	}

	{
		U_VEDU_TQITQ_DEADZONE D32;

		D32.bits.tqitq_deadzone_intra_slice =
			channel_cfg->all_reg.VEDU_TQITQ_DEADZONE.bits.tqitq_deadzone_intra_slice;
		D32.bits.tqitq_deadzone_inter_slice =
			channel_cfg->all_reg.VEDU_TQITQ_DEADZONE.bits.tqitq_deadzone_inter_slice;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_TQITQ_DEADZONE.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA0_START D32;

		D32.bits.sao_area0_start_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA0_START.bits.sao_area0_start_lcux;
		D32.bits.sao_area0_start_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA0_START.bits.sao_area0_start_lcuy;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA0_START.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA0_END D32;

		D32.bits.sao_area0_end_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA0_END.bits.sao_area0_end_lcux;
		D32.bits.sao_area0_end_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA0_END.bits.sao_area0_end_lcuy;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA0_END.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA1_START D32;

		D32.bits.sao_area1_start_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA1_START.bits.sao_area1_start_lcux;
		D32.bits.sao_area1_start_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA1_START.bits.sao_area1_start_lcuy;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA1_START.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA1_END D32;

		D32.bits.sao_area1_end_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA1_END.bits.sao_area1_end_lcux;
		D32.bits.sao_area1_end_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA1_END.bits.sao_area1_end_lcuy;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA1_END.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA2_START D32;

		D32.bits.sao_area2_start_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA2_START.bits.sao_area2_start_lcux;
		D32.bits.sao_area2_start_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA2_START.bits.sao_area2_start_lcuy;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA2_START.u32),
			D32.u32);
	}
	{
		U_VEDU_SAO_SSD_AREA2_END D32;

		D32.bits.sao_area2_end_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA2_END.bits.sao_area2_end_lcux;
		D32.bits.sao_area2_end_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA2_END.bits.sao_area2_end_lcuy;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA2_END.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA3_START D32;

		D32.bits.sao_area3_start_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA3_START.bits.sao_area3_start_lcux;
		D32.bits.sao_area3_start_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA3_START.bits.sao_area3_start_lcuy;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA3_START.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA3_END D32;

		D32.bits.sao_area3_end_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA3_END.bits.sao_area3_end_lcux;
		D32.bits.sao_area3_end_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA3_END.bits.sao_area3_end_lcuy;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA3_END.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA4_START D32;

		D32.bits.sao_area4_start_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA4_START.bits.sao_area4_start_lcux;
		D32.bits.sao_area4_start_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA4_START.bits.sao_area4_start_lcuy;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA4_START.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA4_END D32;

		D32.bits.sao_area4_end_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA4_END.bits.sao_area4_end_lcux;
		D32.bits.sao_area4_end_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA4_END.bits.sao_area4_end_lcuy;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA4_END.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA5_START D32;

		D32.bits.sao_area5_start_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA5_START.bits.sao_area5_start_lcux;
		D32.bits.sao_area5_start_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA5_START.bits.sao_area5_start_lcuy;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA5_START.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA5_END D32;

		D32.bits.sao_area5_end_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA5_END.bits.sao_area5_end_lcux;
		D32.bits.sao_area5_end_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA5_END.bits.sao_area5_end_lcuy;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA5_END.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA6_START D32;

		D32.bits.sao_area6_start_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA6_START.bits.sao_area6_start_lcux;
		D32.bits.sao_area6_start_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA6_START.bits.sao_area6_start_lcuy;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA6_START.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA6_END D32;

		D32.bits.sao_area6_end_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA6_END.bits.sao_area6_end_lcux;
		D32.bits.sao_area6_end_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA6_END.bits.sao_area6_end_lcuy;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA6_END.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA7_START D32;

		D32.bits.sao_area7_start_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA7_START.bits.sao_area7_start_lcux;
		D32.bits.sao_area7_start_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA7_START.bits.sao_area7_start_lcuy;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA7_START.u32),
			D32.u32);
	}

	{
		U_VEDU_SAO_SSD_AREA7_END D32;

		D32.bits.sao_area7_end_lcux =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA7_END.bits.sao_area7_end_lcux;
		D32.bits.sao_area7_end_lcuy =
			channel_cfg->all_reg.VEDU_SAO_SSD_AREA7_END.bits.sao_area7_end_lcuy;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SAO_SSD_AREA7_END.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_BG_ENABLE D32;

		D32.bits.vcpi_bg_info_st_en = 0;
		D32.bits.vcpi_bg_refresh_st_en = 0;
		D32.bits.vcpi_bg_ld_en =
			channel_cfg->all_reg.VEDU_VCPI_BG_ENABLE.bits.vcpi_bg_ld_en;
		D32.bits.vcpi_bg_en =
			channel_cfg->all_reg.VEDU_VCPI_BG_ENABLE.bits.vcpi_bg_en;
		D32.bits.vcpi_bg_stat_frame =
			channel_cfg->all_reg.VEDU_VCPI_BG_ENABLE.bits.vcpi_bg_stat_frame;
		D32.bits.vcpi_bg_th_frame =
			channel_cfg->all_reg.VEDU_VCPI_BG_ENABLE.bits.vcpi_bg_th_frame;
		D32.bits.vcpi_bg_percentage_en =
			channel_cfg->all_reg.VEDU_VCPI_BG_ENABLE.bits.vcpi_bg_percentage_en;
		D32.bits.vcpi_bg_start_idc =
			channel_cfg->all_reg.VEDU_VCPI_BG_ENABLE.bits.vcpi_bg_start_idc;
		D32.bits.vcpi_bg_start_frame =
			channel_cfg->all_reg.VEDU_VCPI_BG_ENABLE.bits.vcpi_bg_start_frame;
		D32.bits.vcpi_bg_reset_diff_en =
			channel_cfg->all_reg.VEDU_VCPI_BG_ENABLE.bits.vcpi_bg_reset_diff_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BG_ENABLE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_BG_FLT_PARA0 D32;

		D32.bits.vcpi_bg_alpha_fix_0 =
			channel_cfg->all_reg.VEDU_VCPI_BG_FLT_PARA0.bits.vcpi_bg_alpha_fix_0;
		D32.bits.vcpi_bg_alpha_fix_1 =
			channel_cfg->all_reg.VEDU_VCPI_BG_FLT_PARA0.bits.vcpi_bg_alpha_fix_1;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BG_FLT_PARA0.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_BG_FLT_PARA1 D32;

		D32.bits.vcpi_bg_belta_fix_0 =
			channel_cfg->all_reg.VEDU_VCPI_BG_FLT_PARA1.bits.vcpi_bg_belta_fix_0;
		D32.bits.vcpi_bg_belta_fix_1 =
			channel_cfg->all_reg.VEDU_VCPI_BG_FLT_PARA1.bits.vcpi_bg_belta_fix_1;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BG_FLT_PARA1.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_BG_FLT_PARA2 D32;

		D32.bits.vcpi_bg_delta_fix_0 =
			channel_cfg->all_reg.VEDU_VCPI_BG_FLT_PARA2.bits.vcpi_bg_delta_fix_0;
		D32.bits.vcpi_bg_delta_fix_1 =
			channel_cfg->all_reg.VEDU_VCPI_BG_FLT_PARA2.bits.vcpi_bg_delta_fix_1;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BG_FLT_PARA2.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_BG_THR0 D32;

		D32.bits.vcpi_bg_th_ave =
			channel_cfg->all_reg.VEDU_VCPI_BG_THR0.bits.vcpi_bg_th_ave;
		D32.bits.vcpi_bg_stat_th =
			channel_cfg->all_reg.VEDU_VCPI_BG_THR0.bits.vcpi_bg_stat_th;
		D32.bits.vcpi_bg_ave_update_th =
			channel_cfg->all_reg.VEDU_VCPI_BG_THR0.bits.vcpi_bg_ave_update_th;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BG_THR0.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_BG_THR1 D32;

		D32.bits.vcpi_bg_dist_th =
			channel_cfg->all_reg.VEDU_VCPI_BG_THR1.bits.vcpi_bg_dist_th;
		D32.bits.vcpi_bg_frame_num =
			channel_cfg->all_reg.VEDU_VCPI_BG_THR1.bits.vcpi_bg_frame_num;
		D32.bits.vcpi_bg_min_diff =
			channel_cfg->all_reg.VEDU_VCPI_BG_THR1.bits.vcpi_bg_min_diff;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BG_THR1.u32),
			D32.u32);
	}
	/* hivcodec500 36bit */
	{
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BGL_ADDR_L),
			channel_cfg->all_reg.VEDU_VCPI_BGL_ADDR_L);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BGC_ADDR_L),
			channel_cfg->all_reg.VEDU_VCPI_BGC_ADDR_L);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BGINF_ADDR_L),
			channel_cfg->all_reg.VEDU_VCPI_BGINF_ADDR_L);
		vedu_hal_cfg(reg_base, offsetof(S_HEVC_AVC_REGS_TYPE,
			VEDU_VCPI_BGL_EXT_ADDR_L),
			channel_cfg->all_reg.VEDU_VCPI_BGL_EXT_ADDR_L);
		vedu_hal_cfg(reg_base, offsetof(S_HEVC_AVC_REGS_TYPE,
			VEDU_VCPI_BGC_EXT_ADDR_L),
			channel_cfg->all_reg.VEDU_VCPI_BGC_EXT_ADDR_L);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BGL_ADDR_H),
			channel_cfg->all_reg.VEDU_VCPI_BGL_ADDR_H);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BGC_ADDR_H),
			channel_cfg->all_reg.VEDU_VCPI_BGC_ADDR_H);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BGINF_ADDR_H),
			channel_cfg->all_reg.VEDU_VCPI_BGINF_ADDR_H);
		vedu_hal_cfg(reg_base, offsetof(S_HEVC_AVC_REGS_TYPE,
			VEDU_VCPI_BGL_EXT_ADDR_H),
			channel_cfg->all_reg.VEDU_VCPI_BGL_EXT_ADDR_H);
		vedu_hal_cfg(reg_base, offsetof(S_HEVC_AVC_REGS_TYPE,
			VEDU_VCPI_BGC_EXT_ADDR_H),
			channel_cfg->all_reg.VEDU_VCPI_BGC_EXT_ADDR_H);
	}

	{
		U_VEDU_VCPI_BG_STRIDE D32;

		D32.bits.vcpi_bgl_stride =
			channel_cfg->all_reg.VEDU_VCPI_BG_STRIDE.bits.vcpi_bgl_stride;
		D32.bits.vcpi_bgc_stride =
			channel_cfg->all_reg.VEDU_VCPI_BG_STRIDE.bits.vcpi_bgc_stride;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BG_STRIDE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_BG_EXT_STRIDE D32;

		D32.bits.vcpi_bgl_ext_stride =
			channel_cfg->all_reg.VEDU_VCPI_BG_EXT_STRIDE.bits.vcpi_bgl_ext_stride;
		D32.bits.vcpi_bgc_ext_stride =
			channel_cfg->all_reg.VEDU_VCPI_BG_EXT_STRIDE.bits.vcpi_bgc_ext_stride;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_BG_EXT_STRIDE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_OUTSTD D32;

		D32.bits.vcpi_w_outstanding =
			channel_cfg->all_reg.VEDU_VCPI_OUTSTD.bits.vcpi_w_outstanding;
		D32.bits.vcpi_r_outstanding =
			channel_cfg->all_reg.VEDU_VCPI_OUTSTD.bits.vcpi_r_outstanding;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_OUTSTD.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_LCU_BASELINE D32;

		D32.bits.vctrl_lcu_performance_baseline =
			channel_cfg->all_reg.VEDU_VCTRL_LCU_BASELINE.bits.vctrl_lcu_performance_baseline;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_LCU_BASELINE.u32),
			D32.u32);
	}

	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SPMEM_CTRL_0), 0x00015858);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SPMEM_CTRL_1.u32), 0x55554505);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SPMEM_CTRL_2.u32), 0x00052A05);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_TPMEM_CTRL_0), 0x00000850);
	vedu_hal_cfg(reg_base,
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_TPMEM_CTRL_1), 0x00004858);

	{
		U_VEDU_CURLD_GCFG D32;

		D32.bits.wide_ynarrow_en =
			channel_cfg->all_reg.VEDU_CURLD_GCFG.bits.wide_ynarrow_en;
		D32.bits.wide_cnarrow_en =
			channel_cfg->all_reg.VEDU_CURLD_GCFG.bits.wide_cnarrow_en;
		D32.bits.rgb_clip_en =
			channel_cfg->all_reg.VEDU_CURLD_GCFG.bits.rgb_clip_en;
		D32.bits.mlsb_sel =
			channel_cfg->all_reg.VEDU_CURLD_GCFG.bits.mlsb_sel;
		D32.bits.curld_col2gray_en =
			channel_cfg->all_reg.VEDU_CURLD_GCFG.bits.curld_col2gray_en;
		D32.bits.curld_clip_en =
			channel_cfg->all_reg.VEDU_CURLD_GCFG.bits.curld_clip_en;
		D32.bits.curld_read_interval =
			channel_cfg->all_reg.VEDU_CURLD_GCFG.bits.curld_read_interval;
		D32.bits.curld_lowdly_en =
			channel_cfg->all_reg.VEDU_CURLD_GCFG.bits.curld_lowdly_en;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_GCFG.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_OSD_ENABLE D32;

		D32.bits.vcpi_osd_en = 0;
		D32.bits.vcpi_osd7_absqp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd7_absqp;
		D32.bits.vcpi_osd6_absqp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd6_absqp;
		D32.bits.vcpi_osd5_absqp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd5_absqp;
		D32.bits.vcpi_osd4_absqp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd4_absqp;
		D32.bits.vcpi_osd3_absqp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd3_absqp;
		D32.bits.vcpi_osd2_absqp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd2_absqp;
		D32.bits.vcpi_osd1_absqp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd1_absqp;
		D32.bits.vcpi_osd0_absqp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd0_absqp;
		D32.bits.vcpi_osd7_en =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd7_en;
		D32.bits.vcpi_osd6_en =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd6_en;
		D32.bits.vcpi_osd5_en =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd5_en;
		D32.bits.vcpi_osd4_en =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd4_en;
		D32.bits.vcpi_osd3_en =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd3_en;
		D32.bits.vcpi_osd2_en =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd2_en;
		D32.bits.vcpi_osd1_en =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd1_en;
		D32.bits.vcpi_osd0_en =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_osd0_en;
		D32.bits.vcpi_roi_osd_sel_0 =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_roi_osd_sel_0;
		D32.bits.vcpi_roi_osd_sel_1 =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_roi_osd_sel_1;
		D32.bits.vcpi_roi_osd_sel_2 =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_roi_osd_sel_2;
		D32.bits.vcpi_roi_osd_sel_3 =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_roi_osd_sel_3;
		D32.bits.vcpi_roi_osd_sel_4 =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_roi_osd_sel_4;
		D32.bits.vcpi_roi_osd_sel_5 =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_roi_osd_sel_5;
		D32.bits.vcpi_roi_osd_sel_6 =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_roi_osd_sel_6;
		D32.bits.vcpi_roi_osd_sel_7 =
			channel_cfg->all_reg.VEDU_VCPI_OSD_ENABLE.bits.vcpi_roi_osd_sel_7;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_OSD_ENABLE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_STRFMT D32;

		D32.bits.vcpi_crop_en =
			channel_cfg->all_reg.VEDU_VCPI_STRFMT.bits.vcpi_crop_en;
		D32.bits.vcpi_scale_en =
			channel_cfg->all_reg.VEDU_VCPI_STRFMT.bits.vcpi_scale_en;
		D32.bits.vcpi_recst_disable =
			channel_cfg->all_reg.VEDU_VCPI_STRFMT.bits.vcpi_recst_disable;
		D32.bits.vcpi_package_sel =
			channel_cfg->all_reg.VEDU_VCPI_STRFMT.bits.vcpi_package_sel;
		D32.bits.vcpi_str_fmt =
			channel_cfg->all_reg.VEDU_VCPI_STRFMT.bits.vcpi_str_fmt;
		D32.bits.vcpi_blk_type = 0;
		D32.bits.vcpi_store_mode =
			channel_cfg->all_reg.VEDU_VCPI_STRFMT.bits.vcpi_store_mode;
		D32.bits.vcpi_recst_hfbc_raw_en =
			channel_cfg->all_reg.VEDU_VCPI_STRFMT.bits.vcpi_recst_hfbc_raw_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_STRFMT.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_INTRA_INTER_CU_EN D32;

		D32.bits.vcpi_mrg_cu_en =
			channel_cfg->all_reg.VEDU_VCPI_INTRA_INTER_CU_EN.bits.vcpi_mrg_cu_en;
		D32.bits.vcpi_fme_cu_en =
			channel_cfg->all_reg.VEDU_VCPI_INTRA_INTER_CU_EN.bits.vcpi_fme_cu_en;
		D32.bits.vcpi_intra_h264_cutdiag =
			channel_cfg->all_reg.VEDU_VCPI_INTRA_INTER_CU_EN.bits.vcpi_intra_h264_cutdiag;
		D32.bits.vcpi_ipcm_en =
			channel_cfg->all_reg.VEDU_VCPI_INTRA_INTER_CU_EN.bits.vcpi_ipcm_en;
		D32.bits.vcpi_intra_cu_en =
			channel_cfg->all_reg.VEDU_VCPI_INTRA_INTER_CU_EN.bits.vcpi_intra_cu_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_INTRA_INTER_CU_EN.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_PRE_JUDGE_EXT_EN D32;

		D32.bits.vcpi_ext_edge_en =
			channel_cfg->all_reg.VEDU_VCPI_PRE_JUDGE_EXT_EN.bits.vcpi_ext_edge_en;
		D32.bits.vcpi_pintra_inter_flag_disable =
			channel_cfg->all_reg.VEDU_VCPI_PRE_JUDGE_EXT_EN.bits.vcpi_pintra_inter_flag_disable;
		D32.bits.vcpi_force_inter =
			channel_cfg->all_reg.VEDU_VCPI_PRE_JUDGE_EXT_EN.bits.vcpi_force_inter;
		D32.bits.vcpi_pblk_pre_en =
			channel_cfg->all_reg.VEDU_VCPI_PRE_JUDGE_EXT_EN.bits.vcpi_pblk_pre_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PRE_JUDGE_EXT_EN.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_CROSS_TILE_SLC D32;

		D32.bits.vcpi_cross_tile =
			channel_cfg->all_reg.VEDU_VCPI_CROSS_TILE_SLC.bits.vcpi_cross_tile;
		D32.bits.vcpi_cross_slice =
			channel_cfg->all_reg.VEDU_VCPI_CROSS_TILE_SLC.bits.vcpi_cross_slice;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_CROSS_TILE_SLC.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_MULTISLC D32;

		D32.bits.vcpi_multislc_en =
			channel_cfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_multislc_en;
		D32.bits.vcpi_slcspilt_mod =
			channel_cfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_slcspilt_mod;
		D32.bits.vcpi_slice_size =
			channel_cfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_slice_size;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_MULTISLC.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_LCU_TARGET_BIT D32;

		D32.bits.vctrl_lcu_target_bit =
			channel_cfg->all_reg.VEDU_VCTRL_LCU_TARGET_BIT.bits.vctrl_lcu_target_bit;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_LCU_TARGET_BIT.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_DBLKCFG D32;

		D32.bits.vcpi_dblk_filter_flag =
			channel_cfg->all_reg.VEDU_VCPI_DBLKCFG.bits.vcpi_dblk_filter_flag;
		D32.bits.vcpi_dblk_alpha =
			channel_cfg->all_reg.VEDU_VCPI_DBLKCFG.bits.vcpi_dblk_alpha;
		D32.bits.vcpi_dblk_beta =
			channel_cfg->all_reg.VEDU_VCPI_DBLKCFG.bits.vcpi_dblk_beta;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_DBLKCFG.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_SW_L0_SIZE D32;

		D32.bits.vcpi_sw_l0_height =
			channel_cfg->all_reg.VEDU_VCPI_SW_L0_SIZE.bits.vcpi_sw_l0_height;
		D32.bits.vcpi_sw_l0_width =
			channel_cfg->all_reg.VEDU_VCPI_SW_L0_SIZE.bits.vcpi_sw_l0_width;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SW_L0_SIZE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_SW_L1_SIZE D32;

		D32.bits.vcpi_sw_l1_height =
			channel_cfg->all_reg.VEDU_VCPI_SW_L1_SIZE.bits.vcpi_sw_l1_height;
		D32.bits.vcpi_sw_l1_width =
			channel_cfg->all_reg.VEDU_VCPI_SW_L1_SIZE.bits.vcpi_sw_l1_width;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SW_L1_SIZE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_I_SLC_INSERT D32;

		D32.bits.vcpi_insert_i_slc_en =
			channel_cfg->all_reg.VEDU_VCPI_I_SLC_INSERT.bits.vcpi_insert_i_slc_en;
		D32.bits.vcpi_insert_i_slc_idx =
			channel_cfg->all_reg.VEDU_VCPI_I_SLC_INSERT.bits.vcpi_insert_i_slc_idx;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_I_SLC_INSERT.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_SAFE_CFG D32;

		D32.bits.pme_safe_line =
			channel_cfg->all_reg.VEDU_PME_SAFE_CFG.bits.pme_safe_line;
		D32.bits.pme_safe_line_val =
			channel_cfg->all_reg.VEDU_PME_SAFE_CFG.bits.pme_safe_line_val;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_SAFE_CFG.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_IBLK_REFRESH D32;

		D32.bits.pme_iblk_refresh_start_num =
			channel_cfg->all_reg.VEDU_PME_IBLK_REFRESH.bits.pme_iblk_refresh_start_num;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_IBLK_REFRESH.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_IBLK_REFRESH_NUM D32;

		D32.bits.pme_iblk_refresh_num =
			channel_cfg->all_reg.VEDU_PME_IBLK_REFRESH_NUM.bits.pme_iblk_refresh_num;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_IBLK_REFRESH_NUM.u32),
			D32.u32);
	}

	{
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_CHNL4_ANG_0EN.u32),
			(dist_protocol(vcpi_protocol, (0xffffffff | 0x2), (0xffffffff | 0x4))));
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_CHNL4_ANG_1EN.u32), (7 & 0x7));
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_CHNL8_ANG_0EN.u32),
			(dist_protocol(vcpi_protocol, (0xffffffff | 0x2), (0xffffffff | 0x4))));
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_CHNL8_ANG_1EN.u32), (7 & 0x7));
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_CHNL16_ANG_0EN.u32),
			(dist_protocol(vcpi_protocol, (0xffffffff | 0x2), (0xffffffff | 0x4))));
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_CHNL16_ANG_1EN.u32), (7 & 0x7));
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_CHNL32_ANG_0EN.u32), (0xffffffff | 0x2));
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_CHNL32_ANG_1EN.u32), (7 & 0x7));
	}

	{
		U_VEDU_PACK_CU_PARAMETER D32;

		D32.bits.pack_vcpi2cu_qp_min_cu_size =
			channel_cfg->all_reg.VEDU_PACK_CU_PARAMETER.bits.pack_vcpi2cu_qp_min_cu_size;
		D32.bits.pack_vcpi2cu_tq_bypass_enabled_flag =
			channel_cfg->all_reg.VEDU_PACK_CU_PARAMETER.bits.pack_vcpi2cu_tq_bypass_enabled_flag;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PACK_CU_PARAMETER.u32),
			D32.u32);
	}

	{
		U_VEDU_PACK_PCM_PARAMETER D32;

		D32.bits.pack_vcpi2pu_log2_max_ipcm_cbsizey =
			channel_cfg->all_reg.VEDU_PACK_PCM_PARAMETER.bits.pack_vcpi2pu_log2_max_ipcm_cbsizey;
		D32.bits.pack_vcpi2pu_log2_min_ipcm_cbsizey =
			channel_cfg->all_reg.VEDU_PACK_PCM_PARAMETER.bits.pack_vcpi2pu_log2_min_ipcm_cbsizey;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PACK_PCM_PARAMETER.u32),
			D32.u32);
	}

	{
		U_VEDU_QPG_READLINE_INTERVAL D32;

		D32.bits.vcpi_tile_qpg_readline_interval =
			channel_cfg->all_reg.VEDU_QPG_READLINE_INTERVAL.bits.vcpi_tile_qpg_readline_interval;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_QPG_READLINE_INTERVAL.u32),
			D32.u32);
	}

	{
		U_VEDU_PMV_READLINE_INTERVAL D32;

		D32.bits.vcpi_pmv_readline_interval =
			channel_cfg->all_reg.VEDU_PMV_READLINE_INTERVAL.bits.vcpi_pmv_readline_interval;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PMV_READLINE_INTERVAL.u32),
			D32.u32);
	}

	{
		U_VEDU_TBLDST_READLINE_INTERVAL D32;

		D32.bits.vcpi_tbldst_readline_interval =
			channel_cfg->all_reg.VEDU_TBLDST_READLINE_INTERVAL.bits.vcpi_tbldst_readline_interval;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_TBLDST_READLINE_INTERVAL.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_OSD_QP0 D32;

		D32.bits.vcpi_osd3_qp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_QP0.bits.vcpi_osd3_qp;
		D32.bits.vcpi_osd2_qp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_QP0.bits.vcpi_osd2_qp;
		D32.bits.vcpi_osd1_qp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_QP0.bits.vcpi_osd1_qp;
		D32.bits.vcpi_osd0_qp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_QP0.bits.vcpi_osd0_qp;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_OSD_QP0.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_OSD_QP1 D32;

		D32.bits.vcpi_osd7_qp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_QP1.bits.vcpi_osd7_qp;
		D32.bits.vcpi_osd6_qp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_QP1.bits.vcpi_osd6_qp;
		D32.bits.vcpi_osd5_qp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_QP1.bits.vcpi_osd5_qp;
		D32.bits.vcpi_osd4_qp =
			channel_cfg->all_reg.VEDU_VCPI_OSD_QP1.bits.vcpi_osd4_qp;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_OSD_QP1.u32),
			D32.u32);
	}

	{
		venc_hal_cfg_curld_osd01(channel_cfg, reg_base);
	}

	{
		U_VEDU_CURLD_OSD23_ALPHA D32;

		D32.bits.vcpi_filter_hor_0coef =
			channel_cfg->all_reg.VEDU_CURLD_OSD23_ALPHA.bits.vcpi_filter_hor_0coef;
		D32.bits.vcpi_filter_hor_1coef =
			channel_cfg->all_reg.VEDU_CURLD_OSD23_ALPHA.bits.vcpi_filter_hor_1coef;
		D32.bits.vcpi_filter_hor_2coef =
			channel_cfg->all_reg.VEDU_CURLD_OSD23_ALPHA.bits.vcpi_filter_hor_2coef;
		D32.bits.vcpi_filter_hor_3coef =
			channel_cfg->all_reg.VEDU_CURLD_OSD23_ALPHA.bits.vcpi_filter_hor_3coef;
		D32.bits.vcpi_filter_hor_rnd =
			channel_cfg->all_reg.VEDU_CURLD_OSD23_ALPHA.bits.vcpi_filter_hor_rnd;
		D32.bits.vcpi_filter_hor_shift =
			channel_cfg->all_reg.VEDU_CURLD_OSD23_ALPHA.bits.vcpi_filter_hor_shift;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD23_ALPHA.u32),
			D32.u32);
	}

	{
		U_VEDU_CURLD_OSD45_ALPHA D32;

		D32.bits.vcpi_filter_ver_0coef =
			channel_cfg->all_reg.VEDU_CURLD_OSD45_ALPHA.bits.vcpi_filter_ver_0coef;
		D32.bits.vcpi_filter_ver_1coef =
			channel_cfg->all_reg.VEDU_CURLD_OSD45_ALPHA.bits.vcpi_filter_ver_1coef;
		D32.bits.vcpi_filter_ver_2coef =
			channel_cfg->all_reg.VEDU_CURLD_OSD45_ALPHA.bits.vcpi_filter_ver_2coef;
		D32.bits.vcpi_filter_ver_3coef =
			channel_cfg->all_reg.VEDU_CURLD_OSD45_ALPHA.bits.vcpi_filter_ver_3coef;
		D32.bits.vcpi_filter_ver_rnd =
			channel_cfg->all_reg.VEDU_CURLD_OSD45_ALPHA.bits.vcpi_filter_ver_rnd;
		D32.bits.vcpi_filter_ver_shift =
			channel_cfg->all_reg.VEDU_CURLD_OSD45_ALPHA.bits.vcpi_filter_ver_shift;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD45_ALPHA.u32),
			D32.u32);
	}

	{
		U_VEDU_CURLD_OSD67_ALPHA D32;

		D32.bits.srcyh_stride =
			channel_cfg->all_reg.VEDU_CURLD_OSD67_ALPHA.bits.srcyh_stride;
		D32.bits.srcch_stride =
			channel_cfg->all_reg.VEDU_CURLD_OSD67_ALPHA.bits.srcch_stride;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD67_ALPHA.u32),
			D32.u32);
	}

	{
		U_VEDU_RGB2YUV_COEF_P0 D32;

		D32.bits.rgb2yuv_y_0coef =
			channel_cfg->all_reg.VEDU_RGB2YUV_COEF_P0.bits.rgb2yuv_y_0coef;
		D32.bits.rgb2yuv_y_1coef =
			channel_cfg->all_reg.VEDU_RGB2YUV_COEF_P0.bits.rgb2yuv_y_1coef;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_RGB2YUV_COEF_P0.u32),
			D32.u32);
	}

	{
		U_VEDU_RGB2YUV_COEF_P1 D32;

		D32.bits.rgb2yuv_y_2coef =
			channel_cfg->all_reg.VEDU_RGB2YUV_COEF_P1.bits.rgb2yuv_y_2coef;
		D32.bits.rgb2yuv_u_0coef =
			channel_cfg->all_reg.VEDU_RGB2YUV_COEF_P1.bits.rgb2yuv_u_0coef;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_RGB2YUV_COEF_P1.u32),
			D32.u32);
	}

	{
		U_VEDU_RGB2YUV_COEF_P2 D32;

		D32.bits.rgb2yuv_u_1coef =
			channel_cfg->all_reg.VEDU_RGB2YUV_COEF_P2.bits.rgb2yuv_u_1coef;
		D32.bits.rgb2yuv_u_2coef =
			channel_cfg->all_reg.VEDU_RGB2YUV_COEF_P2.bits.rgb2yuv_u_2coef;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_RGB2YUV_COEF_P2.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_OSD_POS_0 D32;

		D32.bits.rgb2yuv_v_0coef =
			channel_cfg->all_reg.VEDU_VCPI_OSD_POS_0.bits.rgb2yuv_v_0coef;
		D32.bits.rgb2yuv_v_1coef =
			channel_cfg->all_reg.VEDU_VCPI_OSD_POS_0.bits.rgb2yuv_v_1coef;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_OSD_POS_0.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_OSD_POS_1 D32;

		D32.bits.rgb2yuv_v_2coef =
			channel_cfg->all_reg.VEDU_VCPI_OSD_POS_1.bits.rgb2yuv_v_2coef;
		D32.bits.rgb_v_rnd =
			channel_cfg->all_reg.VEDU_VCPI_OSD_POS_1.bits.rgb_v_rnd;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_OSD_POS_1.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_OSD_POS_4 D32;

		D32.bits.curld_narrow_chrm_max =
			channel_cfg->all_reg.VEDU_VCPI_OSD_POS_4.bits.curld_narrow_chrm_max;
		D32.bits.curld_narrow_chrm_min =
			channel_cfg->all_reg.VEDU_VCPI_OSD_POS_4.bits.curld_narrow_chrm_min;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_OSD_POS_4.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_OSD_POS_5 D32;

		D32.bits.curld_narrow_luma_max =
			channel_cfg->all_reg.VEDU_VCPI_OSD_POS_5.bits.curld_narrow_luma_max;
		D32.bits.curld_narrow_luma_min =
			channel_cfg->all_reg.VEDU_VCPI_OSD_POS_5.bits.curld_narrow_luma_min;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_OSD_POS_5.u32),
			D32.u32);
	}

	{
		U_VEDU_RGB2YUV_OFFSET D32;

		D32.bits.rgb_y_rnd =
			channel_cfg->all_reg.VEDU_RGB2YUV_OFFSET.bits.rgb_y_rnd;
		D32.bits.rgb_u_rnd =
			channel_cfg->all_reg.VEDU_RGB2YUV_OFFSET.bits.rgb_u_rnd;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_RGB2YUV_OFFSET.u32),
			D32.u32);
	}

	{
		U_VEDU_RGB2YUV_CLIP_THR_Y D32;

		D32.bits.rgb2yuv_clip_min_y =
			channel_cfg->all_reg.VEDU_RGB2YUV_CLIP_THR_Y.bits.rgb2yuv_clip_min_y;
		D32.bits.rgb2yuv_clip_max_y =
			channel_cfg->all_reg.VEDU_RGB2YUV_CLIP_THR_Y.bits.rgb2yuv_clip_max_y;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_RGB2YUV_CLIP_THR_Y.u32),
			D32.u32);
	}

	{
		U_VEDU_RGB2YUV_CLIP_THR_C D32;

		D32.bits.rgb2yuv_clip_min_c =
			channel_cfg->all_reg.VEDU_RGB2YUV_CLIP_THR_C.bits.rgb2yuv_clip_min_c;
		D32.bits.rgb2yuv_clip_max_c =
			channel_cfg->all_reg.VEDU_RGB2YUV_CLIP_THR_C.bits.rgb2yuv_clip_max_c;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_RGB2YUV_CLIP_THR_C.u32),
			D32.u32);
	}

	{
		U_VEDU_RGB2YUV_SHIFT_WIDTH D32;

		D32.bits.rgb2yuv_shift_width =
			channel_cfg->all_reg.VEDU_RGB2YUV_SHIFT_WIDTH.bits.rgb2yuv_shift_width;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_RGB2YUV_SHIFT_WIDTH.u32),
			D32.u32);
	}
	/* hivcodec500 reserved */
	{
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD0_ADDR_L), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD1_ADDR_L), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD2_ADDR_L), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD3_ADDR_L), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD4_ADDR_L), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD5_ADDR_L), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD6_ADDR_L), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD7_ADDR_L), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD0_ADDR_H), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD1_ADDR_H), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD2_ADDR_H), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD3_ADDR_H), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD4_ADDR_H), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD5_ADDR_H), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD6_ADDR_H), 0x0);
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CURLD_OSD7_ADDR_H), 0x0);
	}

	{
		U_VEDU_VCTRL_ROI_CFG0 D32;

		D32.bits.vctrl_roi_en =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region7en |
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region6en |
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region5en |
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region4en |
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region3en |
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region2en |
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region1en |
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region0en;

		D32.bits.vctrl_region7keep =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region7keep;
		D32.bits.vctrl_region6keep =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region6keep;
		D32.bits.vctrl_region5keep =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region5keep;
		D32.bits.vctrl_region4keep =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region4keep;
		D32.bits.vctrl_region3keep =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region3keep;
		D32.bits.vctrl_region2keep =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region2keep;
		D32.bits.vctrl_region1keep =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region1keep;
		D32.bits.vctrl_region0keep =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region0keep;
		D32.bits.vctrl_absqp7 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_absqp7;
		D32.bits.vctrl_absqp6 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_absqp6;
		D32.bits.vctrl_absqp5 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_absqp5;
		D32.bits.vctrl_absqp4 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_absqp4;
		D32.bits.vctrl_absqp3 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_absqp3;
		D32.bits.vctrl_absqp2 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_absqp2;
		D32.bits.vctrl_absqp1 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_absqp1;
		D32.bits.vctrl_absqp0 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_absqp0;
		D32.bits.vctrl_region7en =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region7en;
		D32.bits.vctrl_region6en =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region6en;
		D32.bits.vctrl_region5en =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region5en;
		D32.bits.vctrl_region4en =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region4en;
		D32.bits.vctrl_region3en =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region3en;
		D32.bits.vctrl_region2en =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region2en;
		D32.bits.vctrl_region1en =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region1en;
		D32.bits.vctrl_region0en =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.bits.vctrl_region0en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_CFG0.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_CFG1 D32;

		D32.bits.vctrl_roiqp3 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG1.bits.vctrl_roiqp3;
		D32.bits.vctrl_roiqp2 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG1.bits.vctrl_roiqp2;
		D32.bits.vctrl_roiqp1 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG1.bits.vctrl_roiqp1;
		D32.bits.vctrl_roiqp0 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG1.bits.vctrl_roiqp0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_CFG1.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_CFG2 D32;

		D32.bits.vctrl_roiqp7 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG2.bits.vctrl_roiqp7;
		D32.bits.vctrl_roiqp6 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG2.bits.vctrl_roiqp6;
		D32.bits.vctrl_roiqp5 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG2.bits.vctrl_roiqp5;
		D32.bits.vctrl_roiqp4 =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG2.bits.vctrl_roiqp4;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_CFG2.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_SIZE_0 D32;

		D32.bits.vctrl_size0_roiheight =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_0.bits.vctrl_size0_roiheight;
		D32.bits.vctrl_size0_roiwidth =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_0.bits.vctrl_size0_roiwidth;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_0.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_START_0 D32;

		D32.bits.vctrl_start0_roistarty =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_0.bits.vctrl_start0_roistarty;
		D32.bits.vctrl_start0_roistartx =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_0.bits.vctrl_start0_roistartx;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_0.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_SIZE_1 D32;

		D32.bits.vctrl_size1_roiheight =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_1.bits.vctrl_size1_roiheight;
		D32.bits.vctrl_size1_roiwidth =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_1.bits.vctrl_size1_roiwidth;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_1.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_START_1 D32;

		D32.bits.vctrl_start1_roistarty =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_1.bits.vctrl_start1_roistarty;
		D32.bits.vctrl_start1_roistartx =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_1.bits.vctrl_start1_roistartx;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_1.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_SIZE_2 D32;

		D32.bits.vctrl_size2_roiheight =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_2.bits.vctrl_size2_roiheight;
		D32.bits.vctrl_size2_roiwidth =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_2.bits.vctrl_size2_roiwidth;

		vedu_hal_cfg(reg_base,
			 offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_2.u32),
			 D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_START_2 D32;

		D32.bits.vctrl_start2_roistarty =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_2.bits.vctrl_start2_roistarty;
		D32.bits.vctrl_start2_roistartx =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_2.bits.vctrl_start2_roistartx;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_2.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_SIZE_3 D32;

		D32.bits.vctrl_size3_roiheight =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_3.bits.vctrl_size3_roiheight;
		D32.bits.vctrl_size3_roiwidth =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_3.bits.vctrl_size3_roiwidth;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_3.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_START_3 D32;

		D32.bits.vctrl_start3_roistarty =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_3.bits.vctrl_start3_roistarty;
		D32.bits.vctrl_start3_roistartx =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_3.bits.vctrl_start3_roistartx;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_3.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_SIZE_4 D32;

		D32.bits.vctrl_size4_roiheight =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_4.bits.vctrl_size4_roiheight;
		D32.bits.vctrl_size4_roiwidth =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_4.bits.vctrl_size4_roiwidth;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_4.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_START_4 D32;

		D32.bits.vctrl_start4_roistarty =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_4.bits.vctrl_start4_roistarty;
		D32.bits.vctrl_start4_roistartx =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_4.bits.vctrl_start4_roistartx;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_4.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_SIZE_5 D32;

		D32.bits.vctrl_size5_roiheight =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_5.bits.vctrl_size5_roiheight;
		D32.bits.vctrl_size5_roiwidth =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_5.bits.vctrl_size5_roiwidth;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_5.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_START_5 D32;

		D32.bits.vctrl_start5_roistarty =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_5.bits.vctrl_start5_roistarty;
		D32.bits.vctrl_start5_roistartx =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_5.bits.vctrl_start5_roistartx;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_5.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_SIZE_6 D32;

		D32.bits.vctrl_size6_roiheight =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_6.bits.vctrl_size6_roiheight;
		D32.bits.vctrl_size6_roiwidth =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_6.bits.vctrl_size6_roiwidth;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_6.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_START_6 D32;

		D32.bits.vctrl_start6_roistarty =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_6.bits.vctrl_start6_roistarty;
		D32.bits.vctrl_start6_roistartx =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_6.bits
			.vctrl_start6_roistartx;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_6.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_SIZE_7 D32;

		D32.bits.vctrl_size7_roiheight =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_7.bits.vctrl_size7_roiheight;
		D32.bits.vctrl_size7_roiwidth =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_7.bits.vctrl_size7_roiwidth;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_SIZE_7.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ROI_START_7 D32;

		D32.bits.vctrl_start7_roistarty =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_7.bits.vctrl_start7_roistarty;
		D32.bits.vctrl_start7_roistartx =
			channel_cfg->all_reg.VEDU_VCTRL_ROI_START_7.bits.vctrl_start7_roistartx;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ROI_START_7.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_TILE_SIZE D32;

		D32.bits.vcpi_tile_height =
			channel_cfg->all_reg.VEDU_VCPI_TILE_SIZE.bits.vcpi_tile_height;
		D32.bits.vcpi_tile_width =
			channel_cfg->all_reg.VEDU_VCPI_TILE_SIZE.bits.vcpi_tile_width;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_TILE_SIZE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_PICSIZE_PIX D32;

		D32.bits.vcpi_imgheight_pix =
			channel_cfg->all_reg.VEDU_VCPI_PICSIZE_PIX.bits.vcpi_imgheight_pix;
		D32.bits.vcpi_imgwidth_pix =
			channel_cfg->all_reg.VEDU_VCPI_PICSIZE_PIX.bits.vcpi_imgwidth_pix;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PICSIZE_PIX.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_VLC_CONFIG D32;

		if (channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_protocol == 0 ||
			channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_tiles_en)
				D32.bits.vcpi_byte_stuffing = 0;
		else
				D32.bits.vcpi_byte_stuffing =
					channel_cfg->all_reg.VEDU_VCPI_VLC_CONFIG.bits.vcpi_byte_stuffing;

		D32.bits.vcpi_cabac_init_idc = 0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_VLC_CONFIG.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_REF_FLAG D32;

		D32.bits.vcpi_col_from_l0_flag =
			channel_cfg->all_reg.VEDU_VCPI_REF_FLAG.bits.vcpi_col_from_l0_flag;
		D32.bits.vcpi_curr_ref_long_flag =
			channel_cfg->all_reg.VEDU_VCPI_REF_FLAG.bits.vcpi_curr_ref_long_flag;
		D32.bits.vcpi_col_long_flag =
			channel_cfg->all_reg.VEDU_VCPI_REF_FLAG.bits.vcpi_col_long_flag;
		D32.bits.vcpi_predflag_sel =
			channel_cfg->all_reg.VEDU_VCPI_REF_FLAG.bits.vcpi_predflag_sel;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REF_FLAG.u32), D32.u32);
	}

	{
		U_VEDU_PMV_TMV_EN D32;

		D32.bits.pmv_tmv_en =
			channel_cfg->all_reg.VEDU_PMV_TMV_EN.bits.pmv_tmv_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PMV_TMV_EN.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_TMV_LOAD D32;

		D32.bits.vcpi_tmv_wr_rd_avail =
			channel_cfg->all_reg.VEDU_VCPI_TMV_LOAD.bits.vcpi_tmv_wr_rd_avail;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_TMV_LOAD.u32),
			D32.u32);
	}

	{
		U_VEDU_CABAC_GLB_CFG D32;

		D32.u32 = 0;
		D32.bits.cabac_max_num_mergecand =
			channel_cfg->all_reg.VEDU_CABAC_GLB_CFG.bits.cabac_max_num_mergecand;

		if (channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_frame_type == 0)
			D32.bits.cabac_nal_unit_head = 0x2601;
		else
			D32.bits.cabac_nal_unit_head = 0x0201;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_GLB_CFG.u32),
			D32.u32);
	}

	{
		U_VEDU_CABAC_SLCHDR_SIZE D32;

		D32.u32 = 0;

		D32.bits.cabac_slchdr_size_part1 =
			channel_cfg->all_reg.VEDU_CABAC_SLCHDR_SIZE.bits.cabac_slchdr_size_part1;
		D32.bits.cabac_slchdr_size_part2 =
			channel_cfg->all_reg.VEDU_CABAC_SLCHDR_SIZE.bits.cabac_slchdr_size_part2;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_SLCHDR_SIZE.u32),
			D32.u32);
	}

	{
		U_VEDU_CABAC_SLCHDR_SIZE_I D32;

		D32.u32 = 0;

		D32.bits.cabac_slchdr_size_part1_i =
			channel_cfg->all_reg.VEDU_CABAC_SLCHDR_SIZE_I.bits.cabac_slchdr_size_part1_i;
		D32.bits.cabac_slchdr_size_part2_i =
			channel_cfg->all_reg.VEDU_CABAC_SLCHDR_SIZE_I.bits.cabac_slchdr_size_part2_i;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CABAC_SLCHDR_SIZE_I.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_PME_PARAM D32;

		D32.bits.vcpi_move_sad_en =
			channel_cfg->all_reg.VEDU_VCPI_PME_PARAM.bits.vcpi_move_sad_en;
		D32.bits.vcpi_pblk_pre_mvx_thr =
			channel_cfg->all_reg.VEDU_VCPI_PME_PARAM.bits.vcpi_pblk_pre_mvx_thr;
		D32.bits.vcpi_pblk_pre_mvy_thr =
			channel_cfg->all_reg.VEDU_VCPI_PME_PARAM.bits.vcpi_pblk_pre_mvy_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PME_PARAM.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_PIC_STRONG_EN D32;

		D32.bits.vcpi_skin_en =
			channel_cfg->all_reg.VEDU_VCPI_PIC_STRONG_EN.bits.vcpi_skin_en;
		D32.bits.vcpi_strong_edge_en =
			channel_cfg->all_reg.VEDU_VCPI_PIC_STRONG_EN.bits.vcpi_strong_edge_en;
		D32.bits.vcpi_still_en =
			channel_cfg->all_reg.VEDU_VCPI_PIC_STRONG_EN.bits.vcpi_still_en;
		D32.bits.vcpi_skin_close_angle =
			channel_cfg->all_reg.VEDU_VCPI_PIC_STRONG_EN.bits.vcpi_skin_close_angle;
		D32.bits.vcpi_rounding_sobel_en =
			channel_cfg->all_reg.VEDU_VCPI_PIC_STRONG_EN.bits.vcpi_rounding_sobel_en;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PIC_STRONG_EN.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_PINTRA_THRESH0 D32;

		D32.bits.vcpi_pintra_pu16_amp_th =
			channel_cfg->all_reg.VEDU_VCPI_PINTRA_THRESH0.bits.vcpi_pintra_pu16_amp_th;
		D32.bits.vcpi_pintra_pu32_amp_th =
			channel_cfg->all_reg.VEDU_VCPI_PINTRA_THRESH0.bits.vcpi_pintra_pu32_amp_th;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PINTRA_THRESH0.u32),
			D32.u32);
	}
	{
		U_VEDU_VCPI_PINTRA_THRESH1 D32;

		D32.bits.vcpi_pintra_pu16_std_th =
			channel_cfg->all_reg.VEDU_VCPI_PINTRA_THRESH1.bits.vcpi_pintra_pu16_std_th;
		D32.bits.vcpi_pintra_pu32_std_th =
			channel_cfg->all_reg.VEDU_VCPI_PINTRA_THRESH1.bits.vcpi_pintra_pu32_std_th;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PINTRA_THRESH1.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_PINTRA_THRESH2 D32;

		D32.bits.vcpi_pintra_pu16_angel_cost_th =
			channel_cfg->all_reg.VEDU_VCPI_PINTRA_THRESH2.bits.vcpi_pintra_pu16_angel_cost_th;
		D32.bits.vcpi_pintra_pu32_angel_cost_th =
			channel_cfg->all_reg.VEDU_VCPI_PINTRA_THRESH2.bits.vcpi_pintra_pu32_angel_cost_th;
		D32.bits.vcpi_rpintra_pu4_strong_edge_th =
			channel_cfg->all_reg.VEDU_VCPI_PINTRA_THRESH2.bits.vcpi_rpintra_pu4_strong_edge_th;
		D32.bits.vcpi_rpintra_pu4_mode_distance_th =
			channel_cfg->all_reg.VEDU_VCPI_PINTRA_THRESH2.bits.vcpi_rpintra_pu4_mode_distance_th;
		D32.bits.vcpi_rpintra_bypass =
			channel_cfg->all_reg.VEDU_VCPI_PINTRA_THRESH2.bits.vcpi_rpintra_bypass;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PINTRA_THRESH2.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_INTRA32_LOW_POWER D32;

		D32.bits.vcpi_intra32_low_power_thr =
			channel_cfg->all_reg.VEDU_VCPI_INTRA32_LOW_POWER.bits.vcpi_intra32_low_power_thr;
		D32.bits.vcpi_intra32_low_power_en =
			channel_cfg->all_reg.VEDU_VCPI_INTRA32_LOW_POWER.bits.vcpi_intra32_low_power_en;
		D32.bits.vcpi_intra32_low_power_gain =
			channel_cfg->all_reg.VEDU_VCPI_INTRA32_LOW_POWER.bits.vcpi_intra32_low_power_gain;
		D32.bits.vcpi_intra32_low_power_offset =
			channel_cfg->all_reg.VEDU_VCPI_INTRA32_LOW_POWER.bits.vcpi_intra32_low_power_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_INTRA32_LOW_POWER.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_INTRA16_LOW_POWER D32;

		D32.bits.vcpi_intra16_low_power_thr =
			channel_cfg->all_reg.VEDU_VCPI_INTRA16_LOW_POWER.bits.vcpi_intra16_low_power_thr;
		D32.bits.vcpi_intra16_low_power_en =
			channel_cfg->all_reg.VEDU_VCPI_INTRA16_LOW_POWER.bits.vcpi_intra16_low_power_en;
		D32.bits.vcpi_intra16_low_power_gain =
			channel_cfg->all_reg.VEDU_VCPI_INTRA16_LOW_POWER.bits.vcpi_intra16_low_power_gain;
		D32.bits.vcpi_intra16_low_power_offset =
			channel_cfg->all_reg.VEDU_VCPI_INTRA16_LOW_POWER.bits.vcpi_intra16_low_power_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_INTRA16_LOW_POWER.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_INTRA_REDUCE_RDO_NUM D32;

		D32.bits.vcpi_intra_reduce_rdo_num_thr =
			channel_cfg->all_reg.VEDU_VCPI_INTRA_REDUCE_RDO_NUM.bits.vcpi_intra_reduce_rdo_num_thr;
		D32.bits.vcpi_intra_reduce_rdo_num_en =
			channel_cfg->all_reg.VEDU_VCPI_INTRA_REDUCE_RDO_NUM.bits.vcpi_intra_reduce_rdo_num_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_INTRA_REDUCE_RDO_NUM.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_NOFORCEZERO D32;

		D32.bits.vcpi_bislayer0flag =
			channel_cfg->all_reg.VEDU_VCPI_NOFORCEZERO.bits.vcpi_bislayer0flag;
		D32.bits.vcpi_bnoforcezero_flag =
			channel_cfg->all_reg.VEDU_VCPI_NOFORCEZERO.bits.vcpi_bnoforcezero_flag;
		D32.bits.vcpi_bnoforcezero_posx =
			channel_cfg->all_reg.VEDU_VCPI_NOFORCEZERO.bits.vcpi_bnoforcezero_posx;
		D32.bits.vcpi_bnoforcezero_posy =
			channel_cfg->all_reg.VEDU_VCPI_NOFORCEZERO.bits.vcpi_bnoforcezero_posy;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_NOFORCEZERO.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_SKIP_LARGE_RES D32;

		D32.bits.pme_skip_sad_thr_offset =
			channel_cfg->all_reg.VEDU_PME_SKIP_LARGE_RES.bits.pme_skip_sad_thr_offset;
		D32.bits.pme_skip_sad_thr_gain =
			channel_cfg->all_reg.VEDU_PME_SKIP_LARGE_RES.bits.pme_skip_sad_thr_gain;
		D32.bits.pme_skip_large_res_det =
			channel_cfg->all_reg.VEDU_PME_SKIP_LARGE_RES.bits.pme_skip_large_res_det;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_SKIP_LARGE_RES.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_SKIN_SAD_THR D32;

		D32.bits.vcpi_skin_sad_thr_offset =
			channel_cfg->all_reg.VEDU_PME_SKIN_SAD_THR.bits.vcpi_skin_sad_thr_offset;
		D32.bits.vcpi_skin_sad_thr_gain =
			channel_cfg->all_reg.VEDU_PME_SKIN_SAD_THR.bits.vcpi_skin_sad_thr_gain;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_SKIN_SAD_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_NM_ACOFFSET_DENOISE D32;

		D32.bits.vctrl_ChosOthBlkSpecOffset =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ACOFFSET_DENOISE.bits.vctrl_ChosOthBlkSpecOffset;
		D32.bits.vctrl_ChosOthBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ACOFFSET_DENOISE.bits.vctrl_ChosOthBlkOffset16;
		D32.bits.vctrl_IChosCurBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ACOFFSET_DENOISE.bits.vctrl_IChosCurBlkOffset16;
		D32.bits.vctrl_LowFreqACBlk16 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ACOFFSET_DENOISE.bits.vctrl_LowFreqACBlk16;
		D32.bits.vctrl_ChosOthBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ACOFFSET_DENOISE.bits.vctrl_ChosOthBlkOffset32;
		D32.bits.vctrl_IChosCurBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ACOFFSET_DENOISE.bits.vctrl_IChosCurBlkOffset32;
		D32.bits.vctrl_LowFreqACBlk32 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ACOFFSET_DENOISE.bits.vctrl_LowFreqACBlk32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_NM_ACOFFSET_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_NM_ENGTHR_DENOISE D32;

		D32.bits.vctrl_ChosOthBlkSpecThr =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ENGTHR_DENOISE.bits.vctrl_ChosOthBlkSpecThr;
		D32.bits.vctrl_ChosOthBlkEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ENGTHR_DENOISE.bits.vctrl_ChosOthBlkEngThr;
		D32.bits.vctrl_ChosOthBlkThr3 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ENGTHR_DENOISE.bits.vctrl_ChosOthBlkThr3;
		D32.bits.vctrl_ChosOthBlkThr2 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ENGTHR_DENOISE.bits.vctrl_ChosOthBlkThr2;
		D32.bits.vctrl_ChosOthBlkThr =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ENGTHR_DENOISE.bits.vctrl_ChosOthBlkThr;
		D32.bits.vctrl_RmAllHighACThr =
			channel_cfg->all_reg.VEDU_VCTRL_NM_ENGTHR_DENOISE.bits.vctrl_RmAllHighACThr;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_NM_ENGTHR_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_NM_TU8_DENOISE D32;

		D32.bits.vctrl_RingEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_NM_TU8_DENOISE.bits.vctrl_RingEngThr;
		D32.bits.vctrl_RingACThr =
			channel_cfg->all_reg.VEDU_VCTRL_NM_TU8_DENOISE.bits.vctrl_RingACThr;
		D32.bits.vctrl_PChosOthBlkOffset =
			channel_cfg->all_reg.VEDU_VCTRL_NM_TU8_DENOISE.bits.vctrl_PChosOthBlkOffset;
		D32.bits.vctrl_PChosOthBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_TU8_DENOISE.bits.vctrl_PChosOthBlkOffset8;
		D32.bits.vctrl_IChosCurBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_TU8_DENOISE.bits.vctrl_IChosCurBlkOffset8;
		D32.bits.vctrl_LowFreqACBlk8 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_TU8_DENOISE.bits.vctrl_LowFreqACBlk8;
		D32.bits.vctrl_Blk8EnableFlag =
			channel_cfg->all_reg.VEDU_VCTRL_NM_TU8_DENOISE.bits.vctrl_Blk8EnableFlag;
		D32.bits.vctrl_ChosOthBlkOffsetSec16 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_TU8_DENOISE.bits.vctrl_ChosOthBlkOffsetSec16;
		D32.bits.vctrl_ChosOthBlkOffsetSec32 =
			channel_cfg->all_reg.VEDU_VCTRL_NM_TU8_DENOISE.bits.vctrl_ChosOthBlkOffsetSec32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_NM_TU8_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_SK_ACOFFSET_DENOISE D32;

		D32.bits.vctrl_SkinChosOthBlkSpecOffset =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ACOFFSET_DENOISE.bits.vctrl_SkinChosOthBlkSpecOffset;
		D32.bits.vctrl_SkinChosOthBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ACOFFSET_DENOISE.bits.vctrl_SkinChosOthBlkOffset16;
		D32.bits.vctrl_SkinIChosCurBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ACOFFSET_DENOISE.bits.vctrl_SkinIChosCurBlkOffset16;
		D32.bits.vctrl_SkinLowFreqACBlk16 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ACOFFSET_DENOISE.bits.vctrl_SkinLowFreqACBlk16;
		D32.bits.vctrl_SkinChosOthBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ACOFFSET_DENOISE.bits.vctrl_SkinChosOthBlkOffset32;
		D32.bits.vctrl_SkinIChosCurBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ACOFFSET_DENOISE.bits.vctrl_SkinIChosCurBlkOffset32;
		D32.bits.vctrl_SkinLowFreqACBlk32 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ACOFFSET_DENOISE.bits.vctrl_SkinLowFreqACBlk32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_SK_ACOFFSET_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_SK_ENGTHR_DENOISE D32;

		D32.bits.vctrl_SkinChosOthBlkSpecThr =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ENGTHR_DENOISE.bits.vctrl_SkinChosOthBlkSpecThr;
		D32.bits.vctrl_SkinChosOthBlkEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ENGTHR_DENOISE.bits.vctrl_SkinChosOthBlkEngThr;
		D32.bits.vctrl_SkinChosOthBlkThr3 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ENGTHR_DENOISE.bits.vctrl_SkinChosOthBlkThr3;
		D32.bits.vctrl_SkinChosOthBlkThr2 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ENGTHR_DENOISE.bits.vctrl_SkinChosOthBlkThr2;
		D32.bits.vctrl_SkinChosOthBlkThr =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ENGTHR_DENOISE.bits.vctrl_SkinChosOthBlkThr;
		D32.bits.vctrl_SkinRmAllHighACThr =
			channel_cfg->all_reg.VEDU_VCTRL_SK_ENGTHR_DENOISE.bits.vctrl_SkinRmAllHighACThr;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_SK_ENGTHR_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_SK_TU8_DENOISE D32;

		D32.bits.vctrl_SkinRingEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_SK_TU8_DENOISE.bits.vctrl_SkinRingEngThr;
		D32.bits.vctrl_SkinRingACThr =
			channel_cfg->all_reg.VEDU_VCTRL_SK_TU8_DENOISE.bits.vctrl_SkinRingACThr;
		D32.bits.vctrl_SkinPChosOthBlkOffset =
			channel_cfg->all_reg.VEDU_VCTRL_SK_TU8_DENOISE.bits.vctrl_SkinPChosOthBlkOffset;
		D32.bits.vctrl_SkinPChosOthBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_TU8_DENOISE.bits.vctrl_SkinPChosOthBlkOffset8;
		D32.bits.vctrl_SkinIChosCurBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_TU8_DENOISE.bits.vctrl_SkinIChosCurBlkOffset8;
		D32.bits.vctrl_SkinLowFreqACBlk8 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_TU8_DENOISE.bits.vctrl_SkinLowFreqACBlk8;
		D32.bits.vctrl_SkinBlk8EnableFlag =
			channel_cfg->all_reg.VEDU_VCTRL_SK_TU8_DENOISE.bits.vctrl_SkinBlk8EnableFlag;
		D32.bits.vctrl_SkinChosOthBlkOffsetSec16 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_TU8_DENOISE.bits.vctrl_SkinChosOthBlkOffsetSec16;
		D32.bits.vctrl_SkinChosOthBlkOffsetSec32 =
			channel_cfg->all_reg.VEDU_VCTRL_SK_TU8_DENOISE.bits.vctrl_SkinChosOthBlkOffsetSec32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_SK_TU8_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ST_ACOFFSET_DENOISE D32;

		D32.bits.vctrl_StillChosOthBlkSpecOffset =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ACOFFSET_DENOISE.bits.vctrl_StillChosOthBlkSpecOffset;
		D32.bits.vctrl_StillChosOthBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ACOFFSET_DENOISE.bits.vctrl_StillChosOthBlkOffset16;
		D32.bits.vctrl_StillIChosCurBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ACOFFSET_DENOISE.bits.vctrl_StillIChosCurBlkOffset16;
		D32.bits.vctrl_StillLowFreqACBlk16 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ACOFFSET_DENOISE.bits.vctrl_StillLowFreqACBlk16;
		D32.bits.vctrl_StillChosOthBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ACOFFSET_DENOISE.bits.vctrl_StillChosOthBlkOffset32;
		D32.bits.vctrl_StillIChosCurBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ACOFFSET_DENOISE.bits.vctrl_StillIChosCurBlkOffset32;
		D32.bits.vctrl_StillLowFreqACBlk32 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ACOFFSET_DENOISE.bits.vctrl_StillLowFreqACBlk32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ST_ACOFFSET_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ST_ENGTHR_DENOISE D32;

		D32.bits.vctrl_StillChosOthBlkSpecThr =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ENGTHR_DENOISE.bits.vctrl_StillChosOthBlkSpecThr;
		D32.bits.vctrl_StillChosOthBlkEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ENGTHR_DENOISE.bits.vctrl_StillChosOthBlkEngThr;
		D32.bits.vctrl_StillChosOthBlkThr3 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ENGTHR_DENOISE.bits.vctrl_StillChosOthBlkThr3;
		D32.bits.vctrl_StillChosOthBlkThr2 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ENGTHR_DENOISE.bits.vctrl_StillChosOthBlkThr2;
		D32.bits.vctrl_StillChosOthBlkThr =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ENGTHR_DENOISE.bits.vctrl_StillChosOthBlkThr;
		D32.bits.vctrl_StillRmAllHighACThr =
			channel_cfg->all_reg.VEDU_VCTRL_ST_ENGTHR_DENOISE.bits.vctrl_StillRmAllHighACThr;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ST_ENGTHR_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_ST_TU8_DENOISE D32;

		D32.bits.vctrl_StillRingEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_ST_TU8_DENOISE.bits.vctrl_StillRingEngThr;
		D32.bits.vctrl_StillRingACThr =
			channel_cfg->all_reg.VEDU_VCTRL_ST_TU8_DENOISE.bits.vctrl_StillRingACThr;
		D32.bits.vctrl_StillPChosOthBlkOffset =
			channel_cfg->all_reg.VEDU_VCTRL_ST_TU8_DENOISE.bits.vctrl_StillPChosOthBlkOffset;
		D32.bits.vctrl_StillPChosOthBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_TU8_DENOISE.bits.vctrl_StillPChosOthBlkOffset8;
		D32.bits.vctrl_StillIChosCurBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_TU8_DENOISE.bits.vctrl_StillIChosCurBlkOffset8;
		D32.bits.vctrl_StillLowFreqACBlk8 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_TU8_DENOISE.bits.vctrl_StillLowFreqACBlk8;
		D32.bits.vctrl_StillBlk8EnableFlag =
			channel_cfg->all_reg.VEDU_VCTRL_ST_TU8_DENOISE.bits.vctrl_StillBlk8EnableFlag;
		D32.bits.vctrl_StillChosOthBlkOffsetSec16 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_TU8_DENOISE.bits.vctrl_StillChosOthBlkOffsetSec16;
		D32.bits.vctrl_StillChosOthBlkOffsetSec32 =
			channel_cfg->all_reg.VEDU_VCTRL_ST_TU8_DENOISE.bits.vctrl_StillChosOthBlkOffsetSec32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_ST_TU8_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_SE_ACOFFSET_DENOISE D32;

		D32.bits.vctrl_EdgeChosOthBlkSpecOffset =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ACOFFSET_DENOISE.bits.vctrl_EdgeChosOthBlkSpecOffset;
		D32.bits.vctrl_EdgeChosOthBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ACOFFSET_DENOISE.bits.vctrl_EdgeChosOthBlkOffset16;
		D32.bits.vctrl_EdgeIChosCurBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ACOFFSET_DENOISE.bits.vctrl_EdgeIChosCurBlkOffset16;
		D32.bits.vctrl_EdgeLowFreqACBlk16 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ACOFFSET_DENOISE.bits.vctrl_EdgeLowFreqACBlk16;
		D32.bits.vctrl_EdgeChosOthBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ACOFFSET_DENOISE.bits.vctrl_EdgeChosOthBlkOffset32;
		D32.bits.vctrl_EdgeIChosCurBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ACOFFSET_DENOISE.bits.vctrl_EdgeIChosCurBlkOffset32;
		D32.bits.vctrl_EdgeLowFreqACBlk32 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ACOFFSET_DENOISE.bits.vctrl_EdgeLowFreqACBlk32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_SE_ACOFFSET_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_SE_ENGTHR_DENOISE D32;

		D32.bits.vctrl_EdgeChosOthBlkSpecThr =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ENGTHR_DENOISE.bits.vctrl_EdgeChosOthBlkSpecThr;
		D32.bits.vctrl_EdgeChosOthBlkEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ENGTHR_DENOISE.bits.vctrl_EdgeChosOthBlkEngThr;
		D32.bits.vctrl_EdgeChosOthBlkThr3 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ENGTHR_DENOISE.bits.vctrl_EdgeChosOthBlkThr3;
		D32.bits.vctrl_EdgeChosOthBlkThr2 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ENGTHR_DENOISE.bits.vctrl_EdgeChosOthBlkThr2;
		D32.bits.vctrl_EdgeChosOthBlkThr =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ENGTHR_DENOISE.bits.vctrl_EdgeChosOthBlkThr;
		D32.bits.vctrl_EdgeRmAllHighACThr =
			channel_cfg->all_reg.VEDU_VCTRL_SE_ENGTHR_DENOISE.bits.vctrl_EdgeRmAllHighACThr;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_SE_ENGTHR_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_SE_TU8_DENOISE D32;

		D32.bits.vctrl_EdgeRingEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_SE_TU8_DENOISE.bits.vctrl_EdgeRingEngThr;
		D32.bits.vctrl_EdgeRingACThr =
			channel_cfg->all_reg.VEDU_VCTRL_SE_TU8_DENOISE.bits.vctrl_EdgeRingACThr;
		D32.bits.vctrl_EdgePChosOthBlkOffset =
			channel_cfg->all_reg.VEDU_VCTRL_SE_TU8_DENOISE.bits.vctrl_EdgePChosOthBlkOffset;
		D32.bits.vctrl_EdgePChosOthBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_TU8_DENOISE.bits.vctrl_EdgePChosOthBlkOffset8;
		D32.bits.vctrl_EdgeIChosCurBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_TU8_DENOISE.bits.vctrl_EdgeIChosCurBlkOffset8;
		D32.bits.vctrl_EdgeLowFreqACBlk8 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_TU8_DENOISE.bits.vctrl_EdgeLowFreqACBlk8;
		D32.bits.vctrl_EdgeBlk8EnableFlag =
			channel_cfg->all_reg.VEDU_VCTRL_SE_TU8_DENOISE.bits.vctrl_EdgeBlk8EnableFlag;
		D32.bits.vctrl_EdgeChosOthBlkOffsetSec16 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_TU8_DENOISE.bits.vctrl_EdgeChosOthBlkOffsetSec16;
		D32.bits.vctrl_EdgeChosOthBlkOffsetSec32 =
			channel_cfg->all_reg.VEDU_VCTRL_SE_TU8_DENOISE.bits.vctrl_EdgeChosOthBlkOffsetSec32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_SE_TU8_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_SELM_ACOFFSET_DENOISE D32;

		D32.bits.vctrl_EdgeAndMoveChosOthBlkSpecOffset =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ACOFFSET_DENOISE.bits.vctrl_EdgeAndMoveChosOthBlkSpecOffset;
		D32.bits.vctrl_EdgeAndMoveChosOthBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ACOFFSET_DENOISE.bits.vctrl_EdgeAndMoveChosOthBlkOffset16;
		D32.bits.vctrl_EdgeAndMoveIChosCurBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ACOFFSET_DENOISE.bits.vctrl_EdgeAndMoveIChosCurBlkOffset16;
		D32.bits.vctrl_EdgeAndMoveLowFreqACBlk16 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ACOFFSET_DENOISE.bits.vctrl_EdgeAndMoveLowFreqACBlk16;
		D32.bits.vctrl_EdgeAndMoveChosOthBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ACOFFSET_DENOISE.bits.vctrl_EdgeAndMoveChosOthBlkOffset32;
		D32.bits.vctrl_EdgeAndMoveIChosCurBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ACOFFSET_DENOISE.bits.vctrl_EdgeAndMoveIChosCurBlkOffset32;
		D32.bits.vctrl_EdgeAndMoveLowFreqACBlk32 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ACOFFSET_DENOISE.bits.vctrl_EdgeAndMoveLowFreqACBlk32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_SELM_ACOFFSET_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_SELM_ENGTHR_DENOISE D32;

		D32.bits.vctrl_EdgeAndMoveChosOthBlkSpecThr =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ENGTHR_DENOISE.bits.vctrl_EdgeAndMoveChosOthBlkSpecThr;
		D32.bits.vctrl_EdgeAndMoveChosOthBlkEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ENGTHR_DENOISE.bits.vctrl_EdgeAndMoveChosOthBlkEngThr;
		D32.bits.vctrl_EdgeAndMoveChosOthBlkThr3 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ENGTHR_DENOISE.bits.vctrl_EdgeAndMoveChosOthBlkThr3;
		D32.bits.vctrl_EdgeAndMoveChosOthBlkThr2 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ENGTHR_DENOISE.bits.vctrl_EdgeAndMoveChosOthBlkThr2;
		D32.bits.vctrl_EdgeAndMoveChosOthBlkThr =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ENGTHR_DENOISE.bits.vctrl_EdgeAndMoveChosOthBlkThr;
		D32.bits.vctrl_EdgeAndMoveRmAllHighACThr =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_ENGTHR_DENOISE.bits.vctrl_EdgeAndMoveRmAllHighACThr;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_SELM_ENGTHR_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_SELM_TU8_DENOISE D32;

		D32.bits.vctrl_EdgeAndMoveRingEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_TU8_DENOISE.bits.vctrl_EdgeAndMoveRingEngThr;
		D32.bits.vctrl_EdgeAndMoveRingACThr =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_TU8_DENOISE.bits.vctrl_EdgeAndMoveRingACThr;
		D32.bits.vctrl_EdgeAndMovePChosOthBlkOffset =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_TU8_DENOISE.bits.vctrl_EdgeAndMovePChosOthBlkOffset;
		D32.bits.vctrl_EdgeAndMovePChosOthBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_TU8_DENOISE.bits.vctrl_EdgeAndMovePChosOthBlkOffset8;
		D32.bits.vctrl_EdgeAndMoveIChosCurBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_TU8_DENOISE.bits.vctrl_EdgeAndMoveIChosCurBlkOffset8;
		D32.bits.vctrl_EdgeAndMoveLowFreqACBlk8 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_TU8_DENOISE.bits.vctrl_EdgeAndMoveLowFreqACBlk8;
		D32.bits.vctrl_EdgeAndMoveBlk8EnableFlag =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_TU8_DENOISE.bits.vctrl_EdgeAndMoveBlk8EnableFlag;
		D32.bits.vctrl_EdgeAndMoveChosOthBlkOffsetSec16 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_TU8_DENOISE.bits.vctrl_EdgeAndMoveChosOthBlkOffsetSec16;
		D32.bits.vctrl_EdgeAndMoveChosOthBlkOffsetSec32 =
			channel_cfg->all_reg.VEDU_VCTRL_SELM_TU8_DENOISE.bits.vctrl_EdgeAndMoveChosOthBlkOffsetSec32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_SELM_TU8_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_WS_ACOFFSET_DENOISE D32;

		D32.bits.vctrl_WeakChosOthBlkSpecOffset =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ACOFFSET_DENOISE.bits.vctrl_WeakChosOthBlkSpecOffset;
		D32.bits.vctrl_WeakChosOthBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ACOFFSET_DENOISE.bits.vctrl_WeakChosOthBlkOffset16;
		D32.bits.vctrl_WeakIChosCurBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ACOFFSET_DENOISE.bits.vctrl_WeakIChosCurBlkOffset16;
		D32.bits.vctrl_WeakLowFreqACBlk16 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ACOFFSET_DENOISE.bits.vctrl_WeakLowFreqACBlk16;
		D32.bits.vctrl_WeakChosOthBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ACOFFSET_DENOISE.bits.vctrl_WeakChosOthBlkOffset32;
		D32.bits.vctrl_WeakIChosCurBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ACOFFSET_DENOISE.bits.vctrl_WeakIChosCurBlkOffset32;
		D32.bits.vctrl_WeakLowFreqACBlk32 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ACOFFSET_DENOISE.bits.vctrl_WeakLowFreqACBlk32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_WS_ACOFFSET_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_WS_ENGTHR_DENOISE D32;

		D32.bits.vctrl_WeakChosOthBlkSpecThr =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ENGTHR_DENOISE.bits.vctrl_WeakChosOthBlkSpecThr;
		D32.bits.vctrl_WeakChosOthBlkEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ENGTHR_DENOISE.bits.vctrl_WeakChosOthBlkEngThr;
		D32.bits.vctrl_WeakChosOthBlkThr3 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ENGTHR_DENOISE.bits.vctrl_WeakChosOthBlkThr3;
		D32.bits.vctrl_WeakChosOthBlkThr2 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ENGTHR_DENOISE.bits.vctrl_WeakChosOthBlkThr2;
		D32.bits.vctrl_WeakChosOthBlkThr =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ENGTHR_DENOISE.bits.vctrl_WeakChosOthBlkThr;
		D32.bits.vctrl_WeakRmAllHighACThr =
			channel_cfg->all_reg.VEDU_VCTRL_WS_ENGTHR_DENOISE.bits.vctrl_WeakRmAllHighACThr;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_WS_ENGTHR_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_WS_TU8_DENOISE D32;

		D32.bits.vctrl_WeakRingEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_WS_TU8_DENOISE.bits.vctrl_WeakRingEngThr;
		D32.bits.vctrl_WeakRingACThr =
			channel_cfg->all_reg.VEDU_VCTRL_WS_TU8_DENOISE.bits.vctrl_WeakRingACThr;
		D32.bits.vctrl_WeakPChosOthBlkOffset =
			channel_cfg->all_reg.VEDU_VCTRL_WS_TU8_DENOISE.bits.vctrl_WeakPChosOthBlkOffset;
		D32.bits.vctrl_WeakPChosOthBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_TU8_DENOISE.bits.vctrl_WeakPChosOthBlkOffset8;
		D32.bits.vctrl_WeakIChosCurBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_TU8_DENOISE.bits.vctrl_WeakIChosCurBlkOffset8;
		D32.bits.vctrl_WeakLowFreqACBlk8 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_TU8_DENOISE.bits.vctrl_WeakLowFreqACBlk8;
		D32.bits.vctrl_WeakBlk8EnableFlag =
			channel_cfg->all_reg.VEDU_VCTRL_WS_TU8_DENOISE.bits.vctrl_WeakBlk8EnableFlag;
		D32.bits.vctrl_WeakChosOthBlkOffsetSec16 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_TU8_DENOISE.bits.vctrl_WeakChosOthBlkOffsetSec16;
		D32.bits.vctrl_WeakChosOthBlkOffsetSec32 =
			channel_cfg->all_reg.VEDU_VCTRL_WS_TU8_DENOISE.bits.vctrl_WeakChosOthBlkOffsetSec32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_WS_TU8_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_SSSE_ACOFFSET_DENOISE D32;

		D32.bits.vctrl_StrongChosOthBlkSpecOffset =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ACOFFSET_DENOISE.bits.vctrl_StrongChosOthBlkSpecOffset;
		D32.bits.vctrl_StrongChosOthBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ACOFFSET_DENOISE.bits.vctrl_StrongChosOthBlkOffset16;
		D32.bits.vctrl_StrongIChosCurBlkOffset16 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ACOFFSET_DENOISE.bits.vctrl_StrongIChosCurBlkOffset16;
		D32.bits.vctrl_StrongLowFreqACBlk16 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ACOFFSET_DENOISE.bits.vctrl_StrongLowFreqACBlk16;
		D32.bits.vctrl_StrongChosOthBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ACOFFSET_DENOISE.bits.vctrl_StrongChosOthBlkOffset32;
		D32.bits.vctrl_StrongIChosCurBlkOffset32 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ACOFFSET_DENOISE.bits.vctrl_StrongIChosCurBlkOffset32;
		D32.bits.vctrl_StrongLowFreqACBlk32 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ACOFFSET_DENOISE.bits.vctrl_StrongLowFreqACBlk32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_SSSE_ACOFFSET_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_SSSE_ENGTHR_DENOISE D32;

		D32.bits.vctrl_StrongChosOthBlkSpecThr =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ENGTHR_DENOISE.bits.vctrl_StrongChosOthBlkSpecThr;
		D32.bits.vctrl_StrongChosOthBlkEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ENGTHR_DENOISE.bits.vctrl_StrongChosOthBlkEngThr;
		D32.bits.vctrl_StrongChosOthBlkThr3 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ENGTHR_DENOISE.bits.vctrl_StrongChosOthBlkThr3;
		D32.bits.vctrl_StrongChosOthBlkThr2 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ENGTHR_DENOISE.bits.vctrl_StrongChosOthBlkThr2;
		D32.bits.vctrl_StrongChosOthBlkThr =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ENGTHR_DENOISE.bits.vctrl_StrongChosOthBlkThr;
		D32.bits.vctrl_StrongRmAllHighACThr =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_ENGTHR_DENOISE.bits.vctrl_StrongRmAllHighACThr;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_SSSE_ENGTHR_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_SSSE_TU8_DENOISE D32;

		D32.bits.vctrl_StrongRingEngThr =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_TU8_DENOISE.bits.vctrl_StrongRingEngThr;
		D32.bits.vctrl_StrongRingACThr =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_TU8_DENOISE.bits.vctrl_StrongRingACThr;
		D32.bits.vctrl_StrongPChosOthBlkOffset =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_TU8_DENOISE.bits.vctrl_StrongPChosOthBlkOffset;
		D32.bits.vctrl_StrongPChosOthBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_TU8_DENOISE.bits.vctrl_StrongPChosOthBlkOffset8;
		D32.bits.vctrl_StrongIChosCurBlkOffset8 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_TU8_DENOISE.bits.vctrl_StrongIChosCurBlkOffset8;
		D32.bits.vctrl_StrongLowFreqACBlk8 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_TU8_DENOISE.bits.vctrl_StrongLowFreqACBlk8;
		D32.bits.vctrl_StrongBlk8EnableFlag =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_TU8_DENOISE.bits.vctrl_StrongBlk8EnableFlag;
		D32.bits.vctrl_StrongChosOthBlkOffsetSec16 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_TU8_DENOISE.bits.vctrl_StrongChosOthBlkOffsetSec16;
		D32.bits.vctrl_StrongChosOthBlkOffsetSec32 =
			channel_cfg->all_reg.VEDU_VCTRL_SSSE_TU8_DENOISE.bits.vctrl_StrongChosOthBlkOffsetSec32;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_SSSE_TU8_DENOISE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_INTRA_RDO_FACTOR_0 D32;

		D32.bits.vctrl_norm_intra_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_0.bits.vctrl_norm_intra_cu32_rdcost_offset;
		D32.bits.vctrl_norm_intra_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_0.bits.vctrl_norm_intra_cu16_rdcost_offset;
		D32.bits.vctrl_norm_intra_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_0.bits.vctrl_norm_intra_cu8_rdcost_offset;
		D32.bits.vctrl_norm_intra_cu4_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_0.bits.vctrl_norm_intra_cu4_rdcost_offset;
		D32.bits.vctrl_strmov_intra_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_0.bits.vctrl_strmov_intra_cu32_rdcost_offset;
		D32.bits.vctrl_strmov_intra_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_0.bits.vctrl_strmov_intra_cu16_rdcost_offset;
		D32.bits.vctrl_strmov_intra_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_0.bits.vctrl_strmov_intra_cu8_rdcost_offset;
		D32.bits.vctrl_strmov_intra_cu4_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_0.bits.vctrl_strmov_intra_cu4_rdcost_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_INTRA_RDO_FACTOR_0.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_MRG_RDO_FACTOR_0 D32;

		D32.bits.vctrl_norm_mrg_cu64_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_0.bits.vctrl_norm_mrg_cu64_rdcost_offset;
		D32.bits.vctrl_norm_mrg_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_0.bits.vctrl_norm_mrg_cu32_rdcost_offset;
		D32.bits.vctrl_norm_mrg_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_0.bits.vctrl_norm_mrg_cu16_rdcost_offset;
		D32.bits.vctrl_norm_mrg_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_0.bits.vctrl_norm_mrg_cu8_rdcost_offset;
		D32.bits.vctrl_strmov_mrg_cu64_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_0.bits.vctrl_strmov_mrg_cu64_rdcost_offset;
		D32.bits.vctrl_strmov_mrg_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_0.bits.vctrl_strmov_mrg_cu32_rdcost_offset;
		D32.bits.vctrl_strmov_mrg_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_0.bits.vctrl_strmov_mrg_cu16_rdcost_offset;
		D32.bits.vctrl_strmov_mrg_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_0.bits.vctrl_strmov_mrg_cu8_rdcost_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_MRG_RDO_FACTOR_0.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_FME_RDO_FACTOR_0 D32;

		D32.bits.vctrl_norm_fme_cu64_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_0.bits.vctrl_norm_fme_cu64_rdcost_offset;
		D32.bits.vctrl_norm_fme_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_0.bits.vctrl_norm_fme_cu32_rdcost_offset;
		D32.bits.vctrl_norm_fme_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_0.bits.vctrl_norm_fme_cu16_rdcost_offset;
		D32.bits.vctrl_norm_fme_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_0.bits.vctrl_norm_fme_cu8_rdcost_offset;
		D32.bits.vctrl_strmov_fme_cu64_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_0.bits.vctrl_strmov_fme_cu64_rdcost_offset;
		D32.bits.vctrl_strmov_fme_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_0.bits.vctrl_strmov_fme_cu32_rdcost_offset;
		D32.bits.vctrl_strmov_fme_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_0.bits.vctrl_strmov_fme_cu16_rdcost_offset;
		D32.bits.vctrl_strmov_fme_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_0.bits.vctrl_strmov_fme_cu8_rdcost_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_FME_RDO_FACTOR_0.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_INTRA_RDO_FACTOR_1 D32;

		D32.bits.vctrl_skin_intra_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.bits.vctrl_skin_intra_cu32_rdcost_offset;
		D32.bits.vctrl_skin_intra_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.bits.vctrl_skin_intra_cu16_rdcost_offset;
		D32.bits.vctrl_skin_intra_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.bits.vctrl_skin_intra_cu8_rdcost_offset;
		D32.bits.vctrl_skin_intra_cu4_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.bits.vctrl_skin_intra_cu4_rdcost_offset;
		D32.bits.vctrl_sobel_str_intra_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.bits.vctrl_sobel_str_intra_cu32_rdcost_offset;
		D32.bits.vctrl_sobel_str_intra_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.bits.vctrl_sobel_str_intra_cu16_rdcost_offset;
		D32.bits.vctrl_sobel_str_intra_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.bits.vctrl_sobel_str_intra_cu8_rdcost_offset;
		D32.bits.vctrl_sobel_str_intra_cu4_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.bits.vctrl_sobel_str_intra_cu4_rdcost_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_INTRA_RDO_FACTOR_1.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_MRG_RDO_FACTOR_1 D32;

		D32.bits.vctrl_skin_mrg_cu64_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_1.bits.vctrl_skin_mrg_cu64_rdcost_offset;
		D32.bits.vctrl_skin_mrg_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_1.bits.vctrl_skin_mrg_cu32_rdcost_offset;
		D32.bits.vctrl_skin_mrg_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_1.bits.vctrl_skin_mrg_cu16_rdcost_offset;
		D32.bits.vctrl_skin_mrg_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_1.bits.vctrl_skin_mrg_cu8_rdcost_offset;
		D32.bits.vctrl_sobel_str_mrg_cu64_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_1.bits.vctrl_sobel_str_mrg_cu64_rdcost_offset;
		D32.bits.vctrl_sobel_str_mrg_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_1.bits.vctrl_sobel_str_mrg_cu32_rdcost_offset;
		D32.bits.vctrl_sobel_str_mrg_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_1.bits.vctrl_sobel_str_mrg_cu16_rdcost_offset;
		D32.bits.vctrl_sobel_str_mrg_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_1.bits.vctrl_sobel_str_mrg_cu8_rdcost_offset;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_MRG_RDO_FACTOR_1.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_FME_RDO_FACTOR_1 D32;

		D32.bits.vctrl_skin_fme_cu64_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_1.bits.vctrl_skin_fme_cu64_rdcost_offset;
		D32.bits.vctrl_skin_fme_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_1.bits.vctrl_skin_fme_cu32_rdcost_offset;
		D32.bits.vctrl_skin_fme_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_1.bits.vctrl_skin_fme_cu16_rdcost_offset;
		D32.bits.vctrl_skin_fme_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_1.bits.vctrl_skin_fme_cu8_rdcost_offset;
		D32.bits.vctrl_sobel_str_fme_cu64_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_1.bits.vctrl_sobel_str_fme_cu64_rdcost_offset;
		D32.bits.vctrl_sobel_str_fme_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_1.bits.vctrl_sobel_str_fme_cu32_rdcost_offset;
		D32.bits.vctrl_sobel_str_fme_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_1.bits.vctrl_sobel_str_fme_cu16_rdcost_offset;
		D32.bits.vctrl_sobel_str_fme_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_1.bits.vctrl_sobel_str_fme_cu8_rdcost_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_FME_RDO_FACTOR_1.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_INTRA_RDO_FACTOR_2 D32;

		D32.bits.vctrl_hedge_intra_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.bits.vctrl_hedge_intra_cu32_rdcost_offset;
		D32.bits.vctrl_hedge_intra_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.bits.vctrl_hedge_intra_cu16_rdcost_offset;
		D32.bits.vctrl_hedge_intra_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.bits.vctrl_hedge_intra_cu8_rdcost_offset;
		D32.bits.vctrl_hedge_intra_cu4_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.bits.vctrl_hedge_intra_cu4_rdcost_offset;
		D32.bits.vctrl_sobel_tex_intra_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.bits.vctrl_sobel_tex_intra_cu32_rdcost_offset;
		D32.bits.vctrl_sobel_tex_intra_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.bits.vctrl_sobel_tex_intra_cu16_rdcost_offset;
		D32.bits.vctrl_sobel_tex_intra_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.bits.vctrl_sobel_tex_intra_cu8_rdcost_offset;
		D32.bits.vctrl_sobel_tex_intra_cu4_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.bits.vctrl_sobel_tex_intra_cu4_rdcost_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_INTRA_RDO_FACTOR_2.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_MRG_RDO_FACTOR_2 D32;

		D32.bits.vctrl_hedge_mrg_cu64_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_2.bits.vctrl_hedge_mrg_cu64_rdcost_offset;
		D32.bits.vctrl_hedge_mrg_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_2.bits.vctrl_hedge_mrg_cu32_rdcost_offset;
		D32.bits.vctrl_hedge_mrg_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_2.bits.vctrl_hedge_mrg_cu16_rdcost_offset;
		D32.bits.vctrl_hedge_mrg_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_2.bits.vctrl_hedge_mrg_cu8_rdcost_offset;
		D32.bits.vctrl_sobel_tex_mrg_cu64_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_2.bits.vctrl_sobel_tex_mrg_cu64_rdcost_offset;
		D32.bits.vctrl_sobel_tex_mrg_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_2.bits.vctrl_sobel_tex_mrg_cu32_rdcost_offset;
		D32.bits.vctrl_sobel_tex_mrg_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_2.bits.vctrl_sobel_tex_mrg_cu16_rdcost_offset;
		D32.bits.vctrl_sobel_tex_mrg_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_MRG_RDO_FACTOR_2.bits.vctrl_sobel_tex_mrg_cu8_rdcost_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_MRG_RDO_FACTOR_2.u32),
			D32.u32);
	}

	{
		U_VEDU_VCTRL_FME_RDO_FACTOR_2 D32;

		D32.bits.vctrl_hedge_fme_cu64_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_2.bits.vctrl_hedge_fme_cu64_rdcost_offset;
		D32.bits.vctrl_hedge_fme_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_2.bits.vctrl_hedge_fme_cu32_rdcost_offset;
		D32.bits.vctrl_hedge_fme_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_2.bits.vctrl_hedge_fme_cu16_rdcost_offset;
		D32.bits.vctrl_hedge_fme_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_2.bits.vctrl_hedge_fme_cu8_rdcost_offset;
		D32.bits.vctrl_sobel_tex_fme_cu64_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_2.bits.vctrl_sobel_tex_fme_cu64_rdcost_offset;
		D32.bits.vctrl_sobel_tex_fme_cu32_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_2.bits.vctrl_sobel_tex_fme_cu32_rdcost_offset;
		D32.bits.vctrl_sobel_tex_fme_cu16_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_2.bits.vctrl_sobel_tex_fme_cu16_rdcost_offset;
		D32.bits.vctrl_sobel_tex_fme_cu8_rdcost_offset =
			channel_cfg->all_reg.VEDU_VCTRL_FME_RDO_FACTOR_2.bits.vctrl_sobel_tex_fme_cu8_rdcost_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCTRL_FME_RDO_FACTOR_2.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_NEW_COST D32;

		D32.bits.pme_mvp3median_en =
			channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_mvp3median_en;
		D32.bits.pme_cost_lamda_en =
			channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda_en;
		D32.bits.pme_new_cost_en =
			channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_new_cost_en;
		D32.bits.pme_cost_lamda2 =
			channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda2;
		D32.bits.pme_cost_lamda1 =
			channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda1;
		D32.bits.pme_cost_lamda0 =
			channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_NEW_COST.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_COST_OFFSET D32;

		D32.bits.pme_l0_cost_offset =
			channel_cfg->all_reg.VEDU_PME_COST_OFFSET.bits.pme_l0_cost_offset;
		D32.bits.pme_l1_cost_offset =
			channel_cfg->all_reg.VEDU_PME_COST_OFFSET.bits.pme_l1_cost_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_COST_OFFSET.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_SW_ADAPT_EN D32;

		D32.bits.pme_l0_psw_adapt_en =
			channel_cfg->all_reg.VEDU_PME_SW_ADAPT_EN.bits.pme_l0_psw_adapt_en;
		D32.bits.pme_l1_psw_adapt_en =
			channel_cfg->all_reg.VEDU_PME_SW_ADAPT_EN.bits.pme_l1_psw_adapt_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_SW_ADAPT_EN.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_WINDOW_SIZE0_L0 D32;

		D32.bits.pme_l0_win0_width =
			channel_cfg->all_reg.VEDU_PME_WINDOW_SIZE0_L0.bits.pme_l0_win0_width;
		D32.bits.pme_l0_win0_height =
			channel_cfg->all_reg.VEDU_PME_WINDOW_SIZE0_L0.bits.pme_l0_win0_height;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_WINDOW_SIZE0_L0.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_WINDOW_SIZE0_L1 D32;

		D32.bits.pme_l1_win0_width =
			channel_cfg->all_reg.VEDU_PME_WINDOW_SIZE0_L1.bits.pme_l1_win0_width;
		D32.bits.pme_l1_win0_height =
			channel_cfg->all_reg.VEDU_PME_WINDOW_SIZE0_L1.bits.pme_l1_win0_height;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_WINDOW_SIZE0_L1.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_SKIP_PRE D32;

		D32.bits.pme_skipblk_pre_en =
			channel_cfg->all_reg.VEDU_PME_SKIP_PRE.bits.pme_skipblk_pre_en;
		D32.bits.pme_skipblk_pre_cost_thr =
			channel_cfg->all_reg.VEDU_PME_SKIP_PRE.bits.pme_skipblk_pre_cost_thr;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_SKIP_PRE.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_PBLK_PRE1 D32;

		D32.bits.pme_pblk_pre_mv_dif_cost_thr =
			channel_cfg->all_reg.VEDU_PME_PBLK_PRE1.bits.pme_pblk_pre_mv_dif_cost_thr;
		D32.bits.pme_pblk_pre_mv_dif_thr0 =
			channel_cfg->all_reg.VEDU_PME_PBLK_PRE1.bits.pme_pblk_pre_mv_dif_thr0;
		D32.bits.pme_pblk_pre_mv_dif_thr1 =
			channel_cfg->all_reg.VEDU_PME_PBLK_PRE1.bits.pme_pblk_pre_mv_dif_thr1;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_PBLK_PRE1.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_PRE_JUDGE_COST_THR D32;

		D32.bits.vcpi_pblk_pre_cost_thr =
			channel_cfg->all_reg.VEDU_VCPI_PRE_JUDGE_COST_THR.bits.vcpi_pblk_pre_cost_thr;
		D32.bits.vcpi_iblk_pre_cost_thr =
			channel_cfg->all_reg.VEDU_VCPI_PRE_JUDGE_COST_THR.bits.vcpi_iblk_pre_cost_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PRE_JUDGE_COST_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_IBLK_PRE_MV_THR D32;

		D32.bits.vcpi_iblk_pre_mvy_thr =
			channel_cfg->all_reg.VEDU_VCPI_IBLK_PRE_MV_THR.bits.vcpi_iblk_pre_mvy_thr;
		D32.bits.vcpi_iblk_pre_mvx_thr =
			channel_cfg->all_reg.VEDU_VCPI_IBLK_PRE_MV_THR.bits.vcpi_iblk_pre_mvx_thr;
		D32.bits.vcpi_iblk_pre_mv_dif_thr1 =
			channel_cfg->all_reg.VEDU_VCPI_IBLK_PRE_MV_THR.bits.vcpi_iblk_pre_mv_dif_thr1;
		D32.bits.vcpi_iblk_pre_mv_dif_thr0 =
			channel_cfg->all_reg.VEDU_VCPI_IBLK_PRE_MV_THR.bits.vcpi_iblk_pre_mv_dif_thr0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_IBLK_PRE_MV_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_IBLK_COST_THR D32;

		D32.bits.pme_iblk_pre_cost_thr_h264 =
			channel_cfg->all_reg.VEDU_PME_IBLK_COST_THR.bits.pme_iblk_pre_cost_thr_h264;
		D32.bits.pme_intrablk_det_cost_thr1 =
			channel_cfg->all_reg.VEDU_PME_IBLK_COST_THR.bits.pme_intrablk_det_cost_thr1;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_IBLK_COST_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_TR_WEIGHTX D32;

		D32.bits.pme_tr_weightx_2 =
			channel_cfg->all_reg.VEDU_PME_TR_WEIGHTX.bits.pme_tr_weightx_2;
		D32.bits.pme_tr_weightx_1 =
			channel_cfg->all_reg.VEDU_PME_TR_WEIGHTX.bits.pme_tr_weightx_1;
		D32.bits.pme_tr_weightx_0 =
			channel_cfg->all_reg.VEDU_PME_TR_WEIGHTX.bits.pme_tr_weightx_0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_TR_WEIGHTX.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_TR_WEIGHTY D32;

		D32.bits.pme_tr_weighty_2 =
			channel_cfg->all_reg.VEDU_PME_TR_WEIGHTY.bits.pme_tr_weighty_2;
		D32.bits.pme_tr_weighty_1 =
			channel_cfg->all_reg.VEDU_PME_TR_WEIGHTY.bits.pme_tr_weighty_1;
		D32.bits.pme_tr_weighty_0 =
			channel_cfg->all_reg.VEDU_PME_TR_WEIGHTY.bits.pme_tr_weighty_0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_TR_WEIGHTY.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_SR_WEIGHT D32;

		D32.bits.pme_sr_weight_2 =
			channel_cfg->all_reg.VEDU_PME_SR_WEIGHT.bits.pme_sr_weight_2;
		D32.bits.pme_sr_weight_1 =
			channel_cfg->all_reg.VEDU_PME_SR_WEIGHT.bits.pme_sr_weight_1;
		D32.bits.pme_sr_weight_0 =
			channel_cfg->all_reg.VEDU_PME_SR_WEIGHT.bits.pme_sr_weight_0;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_SR_WEIGHT.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_INTRABLK_DET D32;

		D32.bits.pme_intrablk_det_cost_thr0 =
			channel_cfg->all_reg.VEDU_PME_INTRABLK_DET.bits.pme_intrablk_det_cost_thr0;
		D32.bits.pme_pskip_mvy_consistency_thr =
			channel_cfg->all_reg.VEDU_PME_INTRABLK_DET.bits.pme_pskip_mvy_consistency_thr;
		D32.bits.pme_pskip_mvx_consistency_thr =
			channel_cfg->all_reg.VEDU_PME_INTRABLK_DET.bits.pme_pskip_mvx_consistency_thr;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_INTRABLK_DET.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_INTRABLK_DET_THR D32;

		D32.bits.pme_intrablk_det_mv_dif_thr1 =
			channel_cfg->all_reg.VEDU_PME_INTRABLK_DET_THR.bits.pme_intrablk_det_mv_dif_thr1;
		D32.bits.pme_intrablk_det_mv_dif_thr0 =
			channel_cfg->all_reg.VEDU_PME_INTRABLK_DET_THR.bits.pme_intrablk_det_mv_dif_thr0;
		D32.bits.pme_intrablk_det_mvy_thr =
			channel_cfg->all_reg.VEDU_PME_INTRABLK_DET_THR.bits.pme_intrablk_det_mvy_thr;
		D32.bits.pme_intrablk_det_mvx_thr =
			channel_cfg->all_reg.VEDU_PME_INTRABLK_DET_THR.bits.pme_intrablk_det_mvx_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_INTRABLK_DET_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_INTRA_LOWPOW D32;

		D32.bits.pme_intra16_madi_thr =
			channel_cfg->all_reg.VEDU_PME_INTRA_LOWPOW.bits.pme_intra16_madi_thr;
		D32.bits.pme_intra32_madi_thr =
			channel_cfg->all_reg.VEDU_PME_INTRA_LOWPOW.bits.pme_intra32_madi_thr;
		D32.bits.pme_intra_lowpow_en =
			channel_cfg->all_reg.VEDU_PME_INTRA_LOWPOW.bits.pme_intra_lowpow_en;
		D32.bits.pme_inter_first =
			channel_cfg->all_reg.VEDU_PME_INTRA_LOWPOW.bits.pme_inter_first;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_INTRA_LOWPOW.u32),
			D32.u32);
	}

	{
		U_VEDU_IME_FME_LPOW_THR D32;

		D32.bits.ime_lowpow_fme_thr1 =
			channel_cfg->all_reg.VEDU_IME_FME_LPOW_THR.bits.ime_lowpow_fme_thr1;
		D32.bits.ime_lowpow_fme_thr0 =
			channel_cfg->all_reg.VEDU_IME_FME_LPOW_THR.bits.ime_lowpow_fme_thr0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_IME_FME_LPOW_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_SKIN_THR D32;

		D32.bits.pme_skin_v_min_thr =
			channel_cfg->all_reg.VEDU_PME_SKIN_THR.bits.pme_skin_v_min_thr;
		D32.bits.pme_skin_v_max_thr =
			channel_cfg->all_reg.VEDU_PME_SKIN_THR.bits.pme_skin_v_max_thr;
		D32.bits.pme_skin_u_min_thr =
			channel_cfg->all_reg.VEDU_PME_SKIN_THR.bits.pme_skin_u_min_thr;
		D32.bits.pme_skin_u_max_thr =
			channel_cfg->all_reg.VEDU_PME_SKIN_THR.bits.pme_skin_u_max_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_SKIN_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_STRONG_EDGE D32;

		D32.bits.pme_still_scene_thr =
			channel_cfg->all_reg.VEDU_PME_STRONG_EDGE.bits.pme_still_scene_thr;
		D32.bits.pme_strong_edge_cnt =
			channel_cfg->all_reg.VEDU_PME_STRONG_EDGE.bits.pme_strong_edge_cnt;
		D32.bits.pme_strong_edge_thr =
			channel_cfg->all_reg.VEDU_PME_STRONG_EDGE.bits.pme_strong_edge_thr;
		D32.bits.pme_skin_num =
			channel_cfg->all_reg.VEDU_PME_STRONG_EDGE.bits.pme_skin_num;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_STRONG_EDGE.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_LARGE_MOVE_THR D32;

		D32.bits.pme_move_sad_thr =
			channel_cfg->all_reg.VEDU_PME_LARGE_MOVE_THR.bits.pme_move_sad_thr;
		D32.bits.pme_move_scene_thr =
			channel_cfg->all_reg.VEDU_PME_LARGE_MOVE_THR.bits.pme_move_scene_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_LARGE_MOVE_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_INTER_STRONG_EDGE D32;

		D32.bits.pme_interstrongedge_madi_thr =
			channel_cfg->all_reg.VEDU_PME_INTER_STRONG_EDGE.bits.pme_interstrongedge_madi_thr;
		D32.bits.pme_interdiff_max_min_madi_times =
			channel_cfg->all_reg.VEDU_PME_INTER_STRONG_EDGE.bits.pme_interdiff_max_min_madi_times;
		D32.bits.pme_interdiff_max_min_madi_abs =
			channel_cfg->all_reg.VEDU_PME_INTER_STRONG_EDGE.bits.pme_interdiff_max_min_madi_abs;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_INTER_STRONG_EDGE.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_NEW_MADI_TH D32;

		D32.bits.vcpi_new_madi_th0 =
			channel_cfg->all_reg.VEDU_PME_NEW_MADI_TH.bits.vcpi_new_madi_th0;
		D32.bits.vcpi_new_madi_th1 =
			channel_cfg->all_reg.VEDU_PME_NEW_MADI_TH.bits.vcpi_new_madi_th1;
		D32.bits.vcpi_new_madi_th2 =
			channel_cfg->all_reg.VEDU_PME_NEW_MADI_TH.bits.vcpi_new_madi_th2;
		D32.bits.vcpi_new_madi_th3 =
			channel_cfg->all_reg.VEDU_PME_NEW_MADI_TH.bits.vcpi_new_madi_th3;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_NEW_MADI_TH.u32), D32.u32);
	}

	{
		U_VEDU_PME_NEW_LAMBDA D32;

		D32.bits.vcpi_new_lambda =
			channel_cfg->all_reg.VEDU_PME_NEW_LAMBDA.bits.vcpi_new_lambda;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_NEW_LAMBDA.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_SKIP_FLAG D32;

		D32.bits.vcpi_pskip_strongedge_madi_thr =
			channel_cfg->all_reg.VEDU_PME_SKIP_FLAG.bits.vcpi_pskip_strongedge_madi_thr;
		D32.bits.vcpi_pskip_strongedge_madi_times =
			channel_cfg->all_reg.VEDU_PME_SKIP_FLAG.bits.vcpi_pskip_strongedge_madi_times;
		D32.bits.vcpi_pskip_flatregion_madi_thr =
			channel_cfg->all_reg.VEDU_PME_SKIP_FLAG.bits.vcpi_pskip_flatregion_madi_thr;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_SKIP_FLAG.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_PSW_LPW D32;

		D32.bits.pme_psw_lp_diff_thx =
			channel_cfg->all_reg.VEDU_PME_PSW_LPW.bits.pme_psw_lp_diff_thx;
		D32.bits.pme_psw_lp_diff_thy =
			channel_cfg->all_reg.VEDU_PME_PSW_LPW.bits.pme_psw_lp_diff_thy;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_PSW_LPW.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_PBLK_PRE2 D32;

		D32.bits.pme_pblk_pre_madi_times =
			channel_cfg->all_reg.VEDU_PME_PBLK_PRE2.bits.pme_pblk_pre_madi_times;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_PBLK_PRE2.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_ADJUST_PMEMV_H264 D32;

		D32.bits.pme_adjust_pmemv_dist_times =
			channel_cfg->all_reg.VEDU_PME_ADJUST_PMEMV_H264.bits.pme_adjust_pmemv_dist_times;
		D32.bits.pme_adjust_pmemv_en =
			channel_cfg->all_reg.VEDU_PME_ADJUST_PMEMV_H264.bits.pme_adjust_pmemv_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_ADJUST_PMEMV_H264.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_CHROMA_STRONG_EDGE D32;

		D32.bits.vcpi_strong_edge_thr_u =
			channel_cfg->all_reg.VEDU_PME_CHROMA_STRONG_EDGE.bits.vcpi_strong_edge_thr_u;
		D32.bits.vcpi_strong_edge_cnt_u =
			channel_cfg->all_reg.VEDU_PME_CHROMA_STRONG_EDGE.bits.vcpi_strong_edge_cnt_u;
		D32.bits.vcpi_strong_edge_thr_v =
			channel_cfg->all_reg.VEDU_PME_CHROMA_STRONG_EDGE.bits.vcpi_strong_edge_thr_v;
		D32.bits.vcpi_strong_edge_cnt_v =
			channel_cfg->all_reg.VEDU_PME_CHROMA_STRONG_EDGE.bits.vcpi_strong_edge_cnt_v;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_CHROMA_STRONG_EDGE.u32),
			D32.u32);
	}

	{
		U_VEDU_CHROMA_SAD_THR D32;

		D32.bits.vcpi_chroma_sad_thr_offset =
			channel_cfg->all_reg.VEDU_CHROMA_SAD_THR.bits.vcpi_chroma_sad_thr_offset;
		D32.bits.vcpi_chroma_sad_thr_gain =
			channel_cfg->all_reg.VEDU_CHROMA_SAD_THR.bits.vcpi_chroma_sad_thr_gain;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CHROMA_SAD_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_CHROMA_FG_THR D32;

		D32.bits.vcpi_chroma_u0_thr_min =
			channel_cfg->all_reg.VEDU_CHROMA_FG_THR.bits.vcpi_chroma_u0_thr_min;
		D32.bits.vcpi_chroma_u0_thr_max =
			channel_cfg->all_reg.VEDU_CHROMA_FG_THR.bits.vcpi_chroma_u0_thr_max;
		D32.bits.vcpi_chroma_v0_thr_min =
			channel_cfg->all_reg.VEDU_CHROMA_FG_THR.bits.vcpi_chroma_v0_thr_min;
		D32.bits.vcpi_chroma_v0_thr_max =
			channel_cfg->all_reg.VEDU_CHROMA_FG_THR.bits.vcpi_chroma_v0_thr_max;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CHROMA_FG_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_CHROMA_BG_THR D32;

		D32.bits.vcpi_chroma_u1_thr_min =
			channel_cfg->all_reg.VEDU_CHROMA_BG_THR.bits.vcpi_chroma_u1_thr_min;
		D32.bits.vcpi_chroma_u1_thr_max =
			channel_cfg->all_reg.VEDU_CHROMA_BG_THR.bits.vcpi_chroma_u1_thr_max;
		D32.bits.vcpi_chroma_v1_thr_min =
			channel_cfg->all_reg.VEDU_CHROMA_BG_THR.bits.vcpi_chroma_v1_thr_min;
		D32.bits.vcpi_chroma_v1_thr_max =
			channel_cfg->all_reg.VEDU_CHROMA_BG_THR.bits.vcpi_chroma_v1_thr_max;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CHROMA_BG_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_CHROMA_SUM_FG_THR D32;

		D32.bits.vcpi_chroma_uv0_thr_min =
			channel_cfg->all_reg.VEDU_CHROMA_SUM_FG_THR.bits.vcpi_chroma_uv0_thr_min;
		D32.bits.vcpi_chroma_uv0_thr_max =
			channel_cfg->all_reg.VEDU_CHROMA_SUM_FG_THR.bits.vcpi_chroma_uv0_thr_max;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CHROMA_SUM_FG_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_CHROMA_SUM_BG_THR D32;

		D32.bits.vcpi_chroma_uv1_thr_min =
			channel_cfg->all_reg.VEDU_CHROMA_SUM_BG_THR.bits.vcpi_chroma_uv1_thr_min;
		D32.bits.vcpi_chroma_uv1_thr_max =
			channel_cfg->all_reg.VEDU_CHROMA_SUM_BG_THR.bits.vcpi_chroma_uv1_thr_max;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CHROMA_SUM_BG_THR.u32), D32.u32);
	}

	{
		U_VEDU_CHROMA_FG_COUNT_THR D32;

		D32.bits.vcpi_chroma_count0_thr_min =
			channel_cfg->all_reg.VEDU_CHROMA_FG_COUNT_THR.bits.vcpi_chroma_count0_thr_min;
		D32.bits.vcpi_chroma_count0_thr_max =
			channel_cfg->all_reg.VEDU_CHROMA_FG_COUNT_THR.bits.vcpi_chroma_count0_thr_max;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CHROMA_FG_COUNT_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_CHROMA_BG_COUNT_THR D32;

		D32.bits.vcpi_chroma_count1_thr_min =
			channel_cfg->all_reg.VEDU_CHROMA_BG_COUNT_THR.bits.vcpi_chroma_count1_thr_min;
		D32.bits.vcpi_chroma_count1_thr_max =
			channel_cfg->all_reg.VEDU_CHROMA_BG_COUNT_THR.bits.vcpi_chroma_count1_thr_max;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_CHROMA_BG_COUNT_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_PME_MOVE_SCENE_THR D32;

		D32.bits.vcpi_move_scene_mv_thr =
			channel_cfg->all_reg.VEDU_PME_MOVE_SCENE_THR.bits.vcpi_move_scene_mv_thr;
		D32.bits.vcpi_move_scene_mv_en =
			channel_cfg->all_reg.VEDU_PME_MOVE_SCENE_THR.bits.vcpi_move_scene_mv_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_PME_MOVE_SCENE_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_IME_RDOCFG D32;

		D32.bits.ime_lambdaoff16 =
			channel_cfg->all_reg.VEDU_IME_RDOCFG.bits.ime_lambdaoff16;
		D32.bits.ime_lambdaoff8 =
			channel_cfg->all_reg.VEDU_IME_RDOCFG.bits.ime_lambdaoff8;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_IME_RDOCFG.u32),
			D32.u32);
	}

	{
		U_VEDU_IME_LAYER3TO2_THR D32;

		D32.bits.ime_layer3to2_thr0 =
			channel_cfg->all_reg.VEDU_IME_LAYER3TO2_THR.bits.ime_layer3to2_thr0;
		D32.bits.ime_layer3to2_thr1 =
			channel_cfg->all_reg.VEDU_IME_LAYER3TO2_THR.bits.ime_layer3to2_thr1;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_IME_LAYER3TO2_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_IME_LAYER3TO2_THR1 D32;

		D32.bits.ime_layer3to2_cost_diff_thr =
			channel_cfg->all_reg.VEDU_IME_LAYER3TO2_THR1.bits.ime_layer3to2_cost_diff_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_IME_LAYER3TO2_THR1.u32),
			D32.u32);
	}

	{
		U_VEDU_IME_LAYER3TO1_THR D32;

		D32.bits.ime_layer3to1_en =
			channel_cfg->all_reg.VEDU_IME_LAYER3TO1_THR.bits.ime_layer3to1_en;
		D32.bits.ime_layer3to1_pu64_madi_thr =
			channel_cfg->all_reg.VEDU_IME_LAYER3TO1_THR.bits.ime_layer3to1_pu64_madi_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_IME_LAYER3TO1_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_IME_LAYER3TO1_THR1 D32;

		D32.bits.ime_layer3to1_pu32_cost_thr =
			channel_cfg->all_reg.VEDU_IME_LAYER3TO1_THR1.bits.ime_layer3to1_pu32_cost_thr;
		D32.bits.ime_layer3to1_pu64_cost_thr =
			channel_cfg->all_reg.VEDU_IME_LAYER3TO1_THR1.bits.ime_layer3to1_pu64_cost_thr;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_IME_LAYER3TO1_THR1.u32),
			D32.u32);
	}

	{
		U_VEDU_FME_BIAS_COST0 D32;

		D32.bits.fme_pu8_bias_cost =
			channel_cfg->all_reg.VEDU_FME_BIAS_COST0.bits.fme_pu8_bias_cost;
		D32.bits.fme_pu16_bias_cost =
			channel_cfg->all_reg.VEDU_FME_BIAS_COST0.bits.fme_pu16_bias_cost;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_FME_BIAS_COST0.u32),
			D32.u32);
	}

	{
		U_VEDU_FME_BIAS_COST1 D32;

		D32.bits.fme_pu32_bias_cost =
			channel_cfg->all_reg.VEDU_FME_BIAS_COST1.bits.fme_pu32_bias_cost;
		D32.bits.fme_pu64_bias_cost =
			channel_cfg->all_reg.VEDU_FME_BIAS_COST1.bits.fme_pu64_bias_cost;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_FME_BIAS_COST1.u32),
			D32.u32);
	}

	{
		U_VEDU_FME_PU64_LWP D32;

		D32.bits.fme_pu64_lwp_flag =
			channel_cfg->all_reg.VEDU_FME_PU64_LWP.bits.fme_pu64_lwp_flag;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_FME_PU64_LWP.u32),
			D32.u32);
	}

	{
		U_VEDU_MRG_FORCE_ZERO_EN D32;

		D32.bits.mrg_force_zero_en =
			channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_force_zero_en;
		D32.bits.mrg_force_y_zero_en =
			channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_force_y_zero_en;
		D32.bits.mrg_force_u_zero_en =
			channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_force_u_zero_en;
		D32.bits.mrg_force_v_zero_en =
			channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_force_v_zero_en;
		D32.bits.force_adapt_en =
			channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.force_adapt_en;
		D32.bits.fme_rdo_lpw_en =
			channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.fme_rdo_lpw_en;
		D32.bits.dct4_en =
			channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.dct4_en;
		D32.bits.fme_rdo_lpw_th =
			channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.fme_rdo_lpw_th;
		D32.bits.mrg_skip_weight_en =
			channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_skip_weight_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_MRG_FORCE_ZERO_EN.u32),
			D32.u32);
	}

	{
		U_VEDU_MRG_FORCE_SKIP_EN D32;

		D32.bits.mrg_force_skip_en =
			channel_cfg->all_reg.VEDU_MRG_FORCE_SKIP_EN.bits.mrg_force_skip_en;
		D32.bits.inter32_use_tu16_en =
			channel_cfg->all_reg.VEDU_MRG_FORCE_SKIP_EN.bits.inter32_use_tu16_en;
		D32.bits.mrg_not_use_sad_en =
			channel_cfg->all_reg.VEDU_MRG_FORCE_SKIP_EN.bits.mrg_not_use_sad_en;
		D32.bits.mrg_not_use_sad_th =
			channel_cfg->all_reg.VEDU_MRG_FORCE_SKIP_EN.bits.mrg_not_use_sad_th;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_MRG_FORCE_SKIP_EN.u32),
			D32.u32);
	}

	{
		U_VEDU_MRG_BIAS_COST0 D32;

		D32.bits.mrg_pu8_bias_cost =
			channel_cfg->all_reg.VEDU_MRG_BIAS_COST0.bits.mrg_pu8_bias_cost;
		D32.bits.mrg_pu16_bias_cost =
			channel_cfg->all_reg.VEDU_MRG_BIAS_COST0.bits.mrg_pu16_bias_cost;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_MRG_BIAS_COST0.u32),
			D32.u32);
	}

	{
		U_VEDU_MRG_BIAS_COST1 D32;

		D32.bits.mrg_pu32_bias_cost =
			channel_cfg->all_reg.VEDU_MRG_BIAS_COST1.bits.mrg_pu32_bias_cost;
		D32.bits.mrg_pu64_bias_cost =
			channel_cfg->all_reg.VEDU_MRG_BIAS_COST1.bits.mrg_pu64_bias_cost;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_MRG_BIAS_COST1.u32),
			D32.u32);
	}

	{
		U_VEDU_MRG_ABS_OFFSET0 D32;

		D32.bits.mrg_pu8_abs_offset =
			channel_cfg->all_reg.VEDU_MRG_ABS_OFFSET0.bits.mrg_pu8_abs_offset;
		D32.bits.mrg_pu16_abs_offset =
			channel_cfg->all_reg.VEDU_MRG_ABS_OFFSET0.bits.mrg_pu16_abs_offset;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_MRG_ABS_OFFSET0.u32),
			D32.u32);
	}

	{
		U_VEDU_MRG_ABS_OFFSET1 D32;

		D32.bits.mrg_pu32_abs_offset =
			channel_cfg->all_reg.VEDU_MRG_ABS_OFFSET1.bits.mrg_pu32_abs_offset;
		D32.bits.mrg_pu64_abs_offset =
			channel_cfg->all_reg.VEDU_MRG_ABS_OFFSET1.bits.mrg_pu64_abs_offset;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_MRG_ABS_OFFSET1.u32),
			D32.u32);
	}

	{
		U_VEDU_MRG_ADJ_WEIGHT D32;

		D32.bits.cu8_fz_weight =
			channel_cfg->all_reg.VEDU_MRG_ADJ_WEIGHT.bits.cu8_fz_weight;
		D32.bits.cu16_fz_weight =
			channel_cfg->all_reg.VEDU_MRG_ADJ_WEIGHT.bits.cu16_fz_weight;
		D32.bits.cu32_fz_weight =
			channel_cfg->all_reg.VEDU_MRG_ADJ_WEIGHT.bits.cu32_fz_weight;
		D32.bits.cu64_fz_weight =
			channel_cfg->all_reg.VEDU_MRG_ADJ_WEIGHT.bits.cu64_fz_weight;
		D32.bits.cu8_fz_adapt_weight =
			channel_cfg->all_reg.VEDU_MRG_ADJ_WEIGHT.bits.cu8_fz_adapt_weight;
		D32.bits.cu16_fz_adapt_weight =
			channel_cfg->all_reg.VEDU_MRG_ADJ_WEIGHT.bits.cu16_fz_adapt_weight;
		D32.bits.cu32_fz_adapt_weight =
			channel_cfg->all_reg.VEDU_MRG_ADJ_WEIGHT.bits.cu32_fz_adapt_weight;
		D32.bits.cu64_fz_adapt_weight =
			channel_cfg->all_reg.VEDU_MRG_ADJ_WEIGHT.bits.cu64_fz_adapt_weight;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_MRG_ADJ_WEIGHT.u32),
			D32.u32);
	}

	{
		U_VEDU_INTRA_CFG D32;

		D32.bits.constrained_intra_pred_flag =
			channel_cfg->all_reg.VEDU_INTRA_CFG.bits.constrained_intra_pred_flag;
		D32.bits.intra_smooth =
			channel_cfg->all_reg.VEDU_INTRA_CFG.bits.intra_smooth;
		D32.bits.vcpi_force_cu16_low_pow =
			channel_cfg->all_reg.VEDU_INTRA_CFG.bits.vcpi_force_cu16_low_pow;
		D32.bits.vcpi_force_cu32_low_pow =
			channel_cfg->all_reg.VEDU_INTRA_CFG.bits.vcpi_force_cu32_low_pow;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_CFG.u32),
			D32.u32);
	}
	/* low power cfg */
	{
		U_VEDU_INTRA_LOW_POW D32;

		D32.bits.cu16_weak_ang_thr =
			channel_cfg->all_reg.VEDU_INTRA_LOW_POW.bits.cu16_weak_ang_thr;
		D32.bits.cu16_medium_ang_thr =
			channel_cfg->all_reg.VEDU_INTRA_LOW_POW.bits.cu16_medium_ang_thr;
		D32.bits.cu16_strong_ang_thr =
			channel_cfg->all_reg.VEDU_INTRA_LOW_POW.bits.cu16_strong_ang_thr;
		D32.bits.cu16_rdo_num =
			channel_cfg->all_reg.VEDU_INTRA_LOW_POW.bits.cu16_rdo_num;
		D32.bits.cu16_adaptive_reduce_rdo_en =
			channel_cfg->all_reg.VEDU_INTRA_LOW_POW.bits.cu16_adaptive_reduce_rdo_en;
		D32.bits.cu32_weak_ang_thr =
			channel_cfg->all_reg.VEDU_INTRA_LOW_POW.bits.cu32_weak_ang_thr;
		D32.bits.cu32_medium_ang_thr =
			channel_cfg->all_reg.VEDU_INTRA_LOW_POW.bits.cu32_medium_ang_thr;
		D32.bits.cu32_strong_ang_thr =
			channel_cfg->all_reg.VEDU_INTRA_LOW_POW.bits.cu32_strong_ang_thr;
		D32.bits.cu32_rdo_num =
			channel_cfg->all_reg.VEDU_INTRA_LOW_POW.bits.cu32_rdo_num;
		D32.bits.cu32_adaptive_reduce_rdo_en =
			channel_cfg->all_reg.VEDU_INTRA_LOW_POW.bits.cu32_adaptive_reduce_rdo_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_LOW_POW.u32),
			D32.u32);
	}

	{
		U_VEDU_INTRA_BIT_WEIGHT D32;

		D32.bits.intra_bit_weight =
			channel_cfg->all_reg.VEDU_INTRA_BIT_WEIGHT.bits.intra_bit_weight;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_BIT_WEIGHT.u32),
			D32.u32);
	}

	{
		U_VEDU_INTRA_RDO_COST_OFFSET_0 D32;

		D32.bits.intra_cu16_rdo_cost_offset =
			channel_cfg->all_reg.VEDU_INTRA_RDO_COST_OFFSET_0.bits.intra_cu16_rdo_cost_offset;
		D32.bits.intra_cu32_rdo_cost_offset =
			channel_cfg->all_reg.VEDU_INTRA_RDO_COST_OFFSET_0.bits.intra_cu32_rdo_cost_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_RDO_COST_OFFSET_0.u32),
			D32.u32);
	}

	{
		U_VEDU_INTRA_RDO_COST_OFFSET_1 D32;

		D32.bits.intra_cu4_rdo_cost_offset =
			channel_cfg->all_reg.VEDU_INTRA_RDO_COST_OFFSET_1.bits.intra_cu4_rdo_cost_offset;
		D32.bits.intra_cu8_rdo_cost_offset =
			channel_cfg->all_reg.VEDU_INTRA_RDO_COST_OFFSET_1.bits.intra_cu8_rdo_cost_offset;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_RDO_COST_OFFSET_1.u32),
			D32.u32);
	}

	{
		U_VEDU_INTRA_NO_DC_COST_OFFSET_0 D32;

		D32.bits.intra_cu16_non_dc_mode_offset =
			channel_cfg->all_reg.VEDU_INTRA_NO_DC_COST_OFFSET_0.bits.intra_cu16_non_dc_mode_offset;
		D32.bits.intra_cu32_non_dc_mode_offset =
			channel_cfg->all_reg.VEDU_INTRA_NO_DC_COST_OFFSET_0.bits.intra_cu32_non_dc_mode_offset;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_NO_DC_COST_OFFSET_0.u32),
			D32.u32);
	}

	{
		U_VEDU_INTRA_NO_DC_COST_OFFSET_1 D32;

		D32.bits.intra_cu8_non_dc_mode_offset =
			channel_cfg->all_reg.VEDU_INTRA_NO_DC_COST_OFFSET_1.bits.intra_cu8_non_dc_mode_offset;
		D32.bits.intra_cu4_non_dc_mode_offset =
			channel_cfg->all_reg.VEDU_INTRA_NO_DC_COST_OFFSET_1.bits.intra_cu4_non_dc_mode_offset;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_NO_DC_COST_OFFSET_1.u32),
			D32.u32);
	}

	{
		U_VEDU_INTRA_RDO_COST_OFFSET_3 D32;

		D32.bits.intra_h264_rdo_cost_offset =
			channel_cfg->all_reg.VEDU_INTRA_RDO_COST_OFFSET_3.bits.intra_h264_rdo_cost_offset;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_INTRA_RDO_COST_OFFSET_3.u32),
			D32.u32);
	}

	{
		U_VEDU_SEL_OFFSET_STRENGTH D32;

		D32.bits.sel_offset_strength =
			channel_cfg->all_reg.VEDU_SEL_OFFSET_STRENGTH.bits.sel_offset_strength;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SEL_OFFSET_STRENGTH.u32),
			D32.u32);
	}

	{
		U_VEDU_SEL_CU32_DC_AC_TH_OFFSET D32;

		D32.bits.sel_cu32_dc_ac_th_offset =
			channel_cfg->all_reg.VEDU_SEL_CU32_DC_AC_TH_OFFSET.bits.sel_cu32_dc_ac_th_offset;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SEL_CU32_DC_AC_TH_OFFSET.u32),
			D32.u32);
	}

	{
		U_VEDU_SEL_CU32_QP_TH D32;

		D32.bits.sel_cu32_qp0_th =
			channel_cfg->all_reg.VEDU_SEL_CU32_QP_TH.bits.sel_cu32_qp0_th;
		D32.bits.sel_cu32_qp1_th =
			channel_cfg->all_reg.VEDU_SEL_CU32_QP_TH.bits.sel_cu32_qp1_th;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SEL_CU32_QP_TH.u32),
			D32.u32);
	}

	{
		U_VEDU_SEL_RES_DC_AC_TH D32;

		D32.bits.sel_res16_luma_dc_th =
			channel_cfg->all_reg.VEDU_SEL_RES_DC_AC_TH.bits.sel_res16_luma_dc_th;
		D32.bits.sel_res16_chroma_dc_th =
			channel_cfg->all_reg.VEDU_SEL_RES_DC_AC_TH.bits.sel_res16_chroma_dc_th;
		D32.bits.sel_res16_luma_ac_th =
			channel_cfg->all_reg.VEDU_SEL_RES_DC_AC_TH.bits.sel_res16_luma_ac_th;
		D32.bits.sel_res16_chroma_ac_th =
			channel_cfg->all_reg.VEDU_SEL_RES_DC_AC_TH.bits.sel_res16_chroma_ac_th;
		D32.bits.sel_res32_luma_dc_th =
			channel_cfg->all_reg.VEDU_SEL_RES_DC_AC_TH.bits.sel_res32_luma_dc_th;
		D32.bits.sel_res32_chroma_dc_th =
			channel_cfg->all_reg.VEDU_SEL_RES_DC_AC_TH.bits.sel_res32_chroma_dc_th;
		D32.bits.sel_res32_luma_ac_th =
			channel_cfg->all_reg.VEDU_SEL_RES_DC_AC_TH.bits.sel_res32_luma_ac_th;
		D32.bits.sel_res32_chroma_ac_th =
			channel_cfg->all_reg.VEDU_SEL_RES_DC_AC_TH.bits.sel_res32_chroma_ac_th;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_SEL_RES_DC_AC_TH.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_STRIDE D32;

		D32.bits.vcpi_curld_y_stride =
			channel_cfg->all_reg.VEDU_VCPI_STRIDE.bits.vcpi_curld_y_stride;
		D32.bits.vcpi_curld_c_stride =
			channel_cfg->all_reg.VEDU_VCPI_STRIDE.bits.vcpi_curld_c_stride;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_STRIDE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_EXT_STRIDE D32;

		D32.bits.vcpi_curld_ext_y_stride =
			channel_cfg->all_reg.VEDU_VCPI_EXT_STRIDE.bits.vcpi_curld_ext_y_stride;
		D32.bits.vcpi_curld_ext_c_stride =
			channel_cfg->all_reg.VEDU_VCPI_EXT_STRIDE.bits.vcpi_curld_ext_c_stride;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_EXT_STRIDE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_REF_L0_STRIDE D32;

		D32.u32 = 0;
		D32.bits.vcpi_refc_l0_stride =
			channel_cfg->all_reg.VEDU_VCPI_REF_L0_STRIDE.bits.vcpi_refc_l0_stride;
		D32.bits.vcpi_refy_l0_stride =
			channel_cfg->all_reg.VEDU_VCPI_REF_L0_STRIDE.bits.vcpi_refy_l0_stride;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REF_L0_STRIDE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_REF_L1_STRIDE D32;

		D32.u32 = 0;
		D32.bits.vcpi_refc_l1_stride =
			channel_cfg->all_reg.VEDU_VCPI_REF_L1_STRIDE.bits.vcpi_refc_l1_stride;
		D32.bits.vcpi_refy_l1_stride =
			channel_cfg->all_reg.VEDU_VCPI_REF_L1_STRIDE.bits.vcpi_refy_l1_stride;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REF_L1_STRIDE.u32),
			D32.u32);
	}

	{
		U_VEDU_VCPI_REC_STRIDE D32;

		D32.u32 = 0;
		D32.bits.vcpi_recst_ystride =
			channel_cfg->all_reg.VEDU_VCPI_REC_STRIDE.bits.vcpi_recst_ystride;
		D32.bits.vcpi_recst_cstride =
			channel_cfg->all_reg.VEDU_VCPI_REC_STRIDE.bits.vcpi_recst_cstride;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_REC_STRIDE.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_CTRL D32;

		D32.u32 = 0;
		D32.bits.ghdr_en = 0;
		D32.bits.ghdr_ck_gt_en = 0;
		D32.bits.ghdr_demo_en = 0;
		D32.bits.ghdr_demo_mode =
			channel_cfg->all_reg.VEDU_GHDR_CTRL.bits.ghdr_demo_mode;
		D32.bits.ghdr_demo_pos =
			channel_cfg->all_reg.VEDU_GHDR_CTRL.bits.ghdr_demo_pos;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_CTRL.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DEGAMMA_CTRL D32;

		D32.u32 = 0;
		D32.bits.ghdr_degmm_en = 0;
		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DEGAMMA_CTRL.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DEGAMMA_STEP D32;

		D32.u32 = 0;
		D32.bits.g_degmm_x1_step =
			channel_cfg->all_reg.VEDU_GHDR_DEGAMMA_STEP.bits.g_degmm_x1_step;
		D32.bits.g_degmm_x2_step =
			channel_cfg->all_reg.VEDU_GHDR_DEGAMMA_STEP.bits.g_degmm_x2_step;
		D32.bits.g_degmm_x3_step =
			channel_cfg->all_reg.VEDU_GHDR_DEGAMMA_STEP.bits.g_degmm_x3_step;
		D32.bits.g_degmm_x4_step =
			channel_cfg->all_reg.VEDU_GHDR_DEGAMMA_STEP.bits.g_degmm_x4_step;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DEGAMMA_STEP.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DEGAMMA_POS1 D32;

		D32.u32 = 0;
		D32.bits.g_degmm_x1_pos =
			channel_cfg->all_reg.VEDU_GHDR_DEGAMMA_POS1.bits.g_degmm_x1_pos;
		D32.bits.g_degmm_x2_pos =
			channel_cfg->all_reg.VEDU_GHDR_DEGAMMA_POS1.bits.g_degmm_x2_pos;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DEGAMMA_POS1.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DEGAMMA_POS2 D32;

		D32.u32 = 0;
		D32.bits.g_degmm_x3_pos =
			channel_cfg->all_reg.VEDU_GHDR_DEGAMMA_POS2.bits.g_degmm_x3_pos;
		D32.bits.g_degmm_x4_pos =
			channel_cfg->all_reg.VEDU_GHDR_DEGAMMA_POS2.bits.g_degmm_x4_pos;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DEGAMMA_POS2.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DEGAMMA_NUM D32;

		D32.u32 = 0;
		D32.bits.g_degmm_x1_num =
			channel_cfg->all_reg.VEDU_GHDR_DEGAMMA_NUM.bits.g_degmm_x1_num;
		D32.bits.g_degmm_x2_num =
			channel_cfg->all_reg.VEDU_GHDR_DEGAMMA_NUM.bits.g_degmm_x2_num;
		D32.bits.g_degmm_x3_num =
			channel_cfg->all_reg.VEDU_GHDR_DEGAMMA_NUM.bits.g_degmm_x3_num;
		D32.bits.g_degmm_x4_num =
			channel_cfg->all_reg.VEDU_GHDR_DEGAMMA_NUM.bits.g_degmm_x4_num;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DEGAMMA_NUM.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_CTRL D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_en = 0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_CTRL.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_COEF00 D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_coef00 =
			channel_cfg->all_reg.VEDU_GHDR_GAMUT_COEF00.bits.ghdr_gamut_coef00;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_COEF00.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_COEF01 D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_coef01 =
			channel_cfg->all_reg.VEDU_GHDR_GAMUT_COEF01.bits.ghdr_gamut_coef01;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_COEF01.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_COEF02 D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_coef02 =
			channel_cfg->all_reg.VEDU_GHDR_GAMUT_COEF02.bits.ghdr_gamut_coef02;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_COEF02.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_COEF10 D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_coef10 =
			channel_cfg->all_reg.VEDU_GHDR_GAMUT_COEF10.bits.ghdr_gamut_coef10;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_COEF10.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_COEF11 D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_coef11 =
			channel_cfg->all_reg.VEDU_GHDR_GAMUT_COEF11.bits.ghdr_gamut_coef11;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_COEF11.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_COEF12 D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_coef12 =
			channel_cfg->all_reg.VEDU_GHDR_GAMUT_COEF12.bits.ghdr_gamut_coef12;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_COEF12.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_COEF20 D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_coef20 =
			channel_cfg->all_reg.VEDU_GHDR_GAMUT_COEF20.bits.ghdr_gamut_coef20;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_COEF20.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_COEF21 D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_coef21 =
			channel_cfg->all_reg.VEDU_GHDR_GAMUT_COEF21.bits.ghdr_gamut_coef21;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_COEF21.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_COEF22 D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_coef22 =
			channel_cfg->all_reg.VEDU_GHDR_GAMUT_COEF22.bits.ghdr_gamut_coef22;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_COEF22.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_SCALE D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_scale =
			channel_cfg->all_reg.VEDU_GHDR_GAMUT_SCALE.bits.ghdr_gamut_scale;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_SCALE.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_CLIP_MIN D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_clip_min =
			channel_cfg->all_reg.VEDU_GHDR_GAMUT_CLIP_MIN.bits.ghdr_gamut_clip_min;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_CLIP_MIN.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMUT_CLIP_MAX D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamut_clip_max =
			channel_cfg->all_reg.VEDU_GHDR_GAMUT_CLIP_MAX.bits.ghdr_gamut_clip_max;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMUT_CLIP_MAX.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_CTRL D32;

		D32.u32 = 0;
		D32.bits.ghdr_tmap_en = 0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_CTRL.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_REN D32;

		D32.u32 = 0;
		D32.bits.ghdr_tmap_rd_en = 0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_REN.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_STEP D32;

		D32.u32 = 0;
		D32.bits.g_tmap_x1_step =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_STEP.bits.g_tmap_x1_step;
		D32.bits.g_tmap_x2_step =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_STEP.bits.g_tmap_x2_step;
		D32.bits.g_tmap_x3_step =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_STEP.bits.g_tmap_x3_step;
		D32.bits.g_tmap_x4_step =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_STEP.bits.g_tmap_x4_step;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_STEP.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_POS1 D32;

		D32.u32 = 0;
		D32.bits.g_tmap_x1_pos =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_POS1.bits.g_tmap_x1_pos;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_POS1.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_POS2 D32;

		D32.u32 = 0;
		D32.bits.g_tmap_x2_pos =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_POS2.bits.g_tmap_x2_pos;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_POS2.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_POS3 D32;

		D32.u32 = 0;
		D32.bits.g_tmap_x3_pos =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_POS3.bits.g_tmap_x3_pos;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_POS3.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_POS4 D32;

		D32.u32 = 0;
		D32.bits.g_tmap_x4_pos =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_POS4.bits.g_tmap_x4_pos;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_POS4.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_NUM D32;

		D32.u32 = 0;
		D32.bits.g_tmap_x1_num =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_NUM.bits.g_tmap_x1_num;
		D32.bits.g_tmap_x2_num =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_NUM.bits.g_tmap_x2_num;
		D32.bits.g_tmap_x3_num =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_NUM.bits.g_tmap_x3_num;
		D32.bits.g_tmap_x4_num =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_NUM.bits.g_tmap_x4_num;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_NUM.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_LUMA_COEF0 D32;

		D32.u32 = 0;
		D32.bits.ghdr_tmap_luma_coef0 =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_LUMA_COEF0.bits.ghdr_tmap_luma_coef0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_LUMA_COEF0.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_LUMA_COEF1 D32;

		D32.u32 = 0;
		D32.bits.ghdr_tmap_luma_coef1 =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_LUMA_COEF1.bits.ghdr_tmap_luma_coef1;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_LUMA_COEF1.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_LUMA_COEF2 D32;

		D32.u32 = 0;
		D32.bits.ghdr_tmap_luma_coef2 =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_LUMA_COEF2.bits.ghdr_tmap_luma_coef2;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_LUMA_COEF2.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_LUMA_SCALE D32;

		D32.u32 = 0;
		D32.bits.ghdr_tmap_luma_scale =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_LUMA_SCALE.bits.ghdr_tmap_luma_scale;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_LUMA_SCALE.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_COEF_SCALE D32;

		D32.u32 = 0;
		D32.bits.ghdr_tmap_coef_scale =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_COEF_SCALE.bits.ghdr_tmap_coef_scale;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_COEF_SCALE.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_OUT_CLIP_MIN D32;

		D32.u32 = 0;
		D32.bits.ghdr_tmap_out_clip_min =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_OUT_CLIP_MIN.bits.ghdr_tmap_out_clip_min;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_OUT_CLIP_MIN.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_TONEMAP_OUT_CLIP_MAX D32;

		D32.u32 = 0;
		D32.bits.ghdr_tmap_out_clip_max =
			channel_cfg->all_reg.VEDU_GHDR_TONEMAP_OUT_CLIP_MAX.bits.ghdr_tmap_out_clip_max;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_TONEMAP_OUT_CLIP_MAX.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMMA_CTRL D32;

		D32.u32 = 0;
		D32.bits.ghdr_gmm_en = 0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMMA_CTRL.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMMA_REN D32;

		D32.u32 = 0;
		D32.bits.ghdr_gamma_rd_en = 0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMMA_REN.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMMA_STEP1 D32;

		D32.u32 = 0;
		D32.bits.g_gmm_x1_step =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_STEP1.bits.g_gmm_x1_step;
		D32.bits.g_gmm_x2_step =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_STEP1.bits.g_gmm_x2_step;
		D32.bits.g_gmm_x3_step =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_STEP1.bits.g_gmm_x3_step;
		D32.bits.g_gmm_x4_step =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_STEP1.bits.g_gmm_x4_step;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMMA_STEP1.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMMA_STEP2 D32;

		D32.u32 = 0;
		D32.bits.g_gmm_x5_step =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_STEP2.bits.g_gmm_x5_step;
		D32.bits.g_gmm_x6_step =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_STEP2.bits.g_gmm_x6_step;
		D32.bits.g_gmm_x7_step =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_STEP2.bits.g_gmm_x7_step;
		D32.bits.g_gmm_x8_step =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_STEP2.bits.g_gmm_x8_step;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMMA_STEP2.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMMA_POS1 D32;

		D32.u32 = 0;
		D32.bits.g_gmm_x1_pos =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_POS1.bits.g_gmm_x1_pos;
		D32.bits.g_gmm_x2_pos =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_POS1.bits.g_gmm_x2_pos;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMMA_POS1.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMMA_POS2 D32;

		D32.u32 = 0;
		D32.bits.g_gmm_x3_pos =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_POS2.bits.g_gmm_x3_pos;
		D32.bits.g_gmm_x4_pos =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_POS2.bits.g_gmm_x4_pos;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMMA_POS2.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMMA_POS3 D32;

		D32.u32 = 0;
		D32.bits.g_gmm_x5_pos =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_POS3.bits.g_gmm_x5_pos;
		D32.bits.g_gmm_x6_pos =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_POS3.bits.g_gmm_x6_pos;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMMA_POS3.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMMA_POS4 D32;

		D32.u32 = 0;
		D32.bits.g_gmm_x7_pos =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_POS4.bits.g_gmm_x7_pos;
		D32.bits.g_gmm_x8_pos =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_POS4.bits.g_gmm_x8_pos;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMMA_POS4.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMMA_NUM1 D32;

		D32.u32 = 0;
		D32.bits.g_gmm_x1_num =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_NUM1.bits.g_gmm_x1_num;
		D32.bits.g_gmm_x2_num =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_NUM1.bits.g_gmm_x2_num;
		D32.bits.g_gmm_x3_num =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_NUM1.bits.g_gmm_x3_num;
		D32.bits.g_gmm_x4_num =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_NUM1.bits.g_gmm_x4_num;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMMA_NUM1.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_GAMMA_NUM2 D32;

		D32.u32 = 0;
		D32.bits.g_gmm_x5_num =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_NUM2.bits.g_gmm_x5_num;
		D32.bits.g_gmm_x6_num =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_NUM2.bits.g_gmm_x6_num;
		D32.bits.g_gmm_x7_num =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_NUM2.bits.g_gmm_x7_num;
		D32.bits.g_gmm_x8_num =
			channel_cfg->all_reg.VEDU_GHDR_GAMMA_NUM2.bits.g_gmm_x8_num;

		vedu_hal_cfg(reg_base, offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_GAMMA_NUM2.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DITHER_CTRL D32;

		D32.u32 = 0;
		D32.bits.ghdr_dither_tap_mode =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_CTRL.bits.ghdr_dither_tap_mode;
		D32.bits.ghdr_dither_domain_mode =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_CTRL.bits.ghdr_dither_domain_mode;
		D32.bits.ghdr_dither_round =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_CTRL.bits.ghdr_dither_round;
		D32.bits.ghdr_dither_mode =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_CTRL.bits.ghdr_dither_mode;
		D32.bits.ghdr_dither_en = 0;
		D32.bits.ghdr_dither_round_unlim =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_CTRL.bits.ghdr_dither_round_unlim;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DITHER_CTRL.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DITHER_THR D32;

		D32.u32 = 0;
		D32.bits.ghdr_dither_thr_min =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_THR.bits.ghdr_dither_thr_min;
		D32.bits.ghdr_dither_thr_max =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_THR.bits.ghdr_dither_thr_max;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DITHER_THR.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DITHER_SED_Y0 D32;

		D32.u32 = 0;
		D32.bits.ghdr_dither_sed_y0 =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_SED_Y0.bits.ghdr_dither_sed_y0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DITHER_SED_Y0.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DITHER_SED_U0 D32;

		D32.u32 = 0;
		D32.bits.ghdr_dither_sed_u0 =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_SED_U0.bits.ghdr_dither_sed_u0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DITHER_SED_U0.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DITHER_SED_V0 D32;

		D32.u32 = 0;
		D32.bits.ghdr_dither_sed_v0 =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_SED_V0.bits.ghdr_dither_sed_v0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DITHER_SED_V0.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DITHER_SED_W0 D32;

		D32.u32 = 0;
		D32.bits.ghdr_dither_sed_w0 =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_SED_W0.bits.ghdr_dither_sed_w0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DITHER_SED_W0.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DITHER_SED_Y1 D32;

		D32.u32 = 0;
		D32.bits.ghdr_dither_sed_y1 =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_SED_Y1.bits.ghdr_dither_sed_y1;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DITHER_SED_Y1.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DITHER_SED_U1 D32;

		D32.u32 = 0;
		D32.bits.ghdr_dither_sed_u1 =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_SED_U1.bits.ghdr_dither_sed_u1;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DITHER_SED_U1.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DITHER_SED_V1 D32;

		D32.u32 = 0;
		D32.bits.ghdr_dither_sed_v1 =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_SED_V1.bits.ghdr_dither_sed_v1;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DITHER_SED_V1.u32),
			D32.u32);
	}

	{
		U_VEDU_GHDR_DITHER_SED_W1 D32;

		D32.u32 = 0;
		D32.bits.ghdr_dither_sed_w1 =
			channel_cfg->all_reg.VEDU_GHDR_DITHER_SED_W1.bits.ghdr_dither_sed_w1;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_GHDR_DITHER_SED_W1.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_CTRL D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_en =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_CTRL.bits.hihdr_r2y_en;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_CTRL.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_COEF0 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_coef00 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_COEF0.bits.hihdr_r2y_coef00;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_COEF0.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_COEF1 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_coef01 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_COEF1.bits.hihdr_r2y_coef01;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_COEF1.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_COEF2 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_coef02 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_COEF2.bits.hihdr_r2y_coef02;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_COEF2.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_COEF3 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_coef10 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_COEF3.bits.hihdr_r2y_coef10;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_COEF3.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_COEF4 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_coef11 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_COEF4.bits.hihdr_r2y_coef11;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_COEF4.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_COEF5 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_coef12 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_COEF5.bits.hihdr_r2y_coef12;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_COEF5.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_COEF6 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_coef20 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_COEF6.bits.hihdr_r2y_coef20;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_COEF6.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_COEF7 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_coef21 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_COEF7.bits.hihdr_r2y_coef21;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_COEF7.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_COEF8 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_coef22 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_COEF8.bits.hihdr_r2y_coef22;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_COEF8.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_SCALE2P D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_scale2p =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_SCALE2P.bits.hihdr_r2y_scale2p;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_SCALE2P.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_IDC0 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_idc0 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_IDC0.bits.hihdr_r2y_idc0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_IDC0.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_IDC1 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_idc1 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_IDC1.bits.hihdr_r2y_idc1;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_IDC1.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_IDC2 D32;

		D32.u32 = 0;

		D32.bits.hihdr_r2y_idc2 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_IDC2.bits.hihdr_r2y_idc2;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_IDC2.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_ODC0 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_odc0 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_ODC0.bits.hihdr_r2y_odc0;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_ODC0.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_ODC1 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_odc1 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_ODC1.bits.hihdr_r2y_odc1;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_ODC1.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_ODC2 D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_odc2 =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_ODC2.bits.hihdr_r2y_odc2;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_ODC2.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_MIN D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_clip_min =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_MIN.bits.hihdr_r2y_clip_min;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_MIN.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_RGB2YUV_MAX D32;

		D32.u32 = 0;
		D32.bits.hihdr_r2y_clip_max =
			channel_cfg->all_reg.VEDU_HIHDR_G_RGB2YUV_MAX.bits.hihdr_r2y_clip_max;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_RGB2YUV_MAX.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_00_01 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef00 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_00_01.bits.hihdr_tonemap_coef00;
		D32.bits.hihdr_tonemap_coef01 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_00_01.bits.hihdr_tonemap_coef01;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_00_01.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_02_03 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef02 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_02_03.bits.hihdr_tonemap_coef02;
		D32.bits.hihdr_tonemap_coef03 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_02_03.bits.hihdr_tonemap_coef03;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_02_03.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_04_05 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef04 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_04_05.bits.hihdr_tonemap_coef04;
		D32.bits.hihdr_tonemap_coef05 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_04_05.bits.hihdr_tonemap_coef05;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_04_05.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_06_07 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef06 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_06_07.bits.hihdr_tonemap_coef06;
		D32.bits.hihdr_tonemap_coef07 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_06_07.bits.hihdr_tonemap_coef07;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_06_07.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_08_09 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef08 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_08_09.bits.hihdr_tonemap_coef08;
		D32.bits.hihdr_tonemap_coef09 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_08_09.bits.hihdr_tonemap_coef09;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_08_09.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_10_11 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef10 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_10_11.bits.hihdr_tonemap_coef10;
		D32.bits.hihdr_tonemap_coef11 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_10_11.bits.hihdr_tonemap_coef11;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_10_11.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_12_13 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef12 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_12_13.bits.hihdr_tonemap_coef12;
		D32.bits.hihdr_tonemap_coef13 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_12_13.bits.hihdr_tonemap_coef13;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_12_13.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_14_15 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef14 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_14_15.bits.hihdr_tonemap_coef14;
		D32.bits.hihdr_tonemap_coef15 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_14_15.bits.hihdr_tonemap_coef15;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_14_15.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_16_17 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef16 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_16_17.bits.hihdr_tonemap_coef16;
		D32.bits.hihdr_tonemap_coef17 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_16_17.bits.hihdr_tonemap_coef17;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_16_17.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_18_19 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef18 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_18_19.bits.hihdr_tonemap_coef18;
		D32.bits.hihdr_tonemap_coef19 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_18_19.bits.hihdr_tonemap_coef19;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_18_19.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_20_21 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef20 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_20_21.bits.hihdr_tonemap_coef20;
		D32.bits.hihdr_tonemap_coef21 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_20_21.bits.hihdr_tonemap_coef21;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_20_21.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_22_23 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef22 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_22_23.bits.hihdr_tonemap_coef22;
		D32.bits.hihdr_tonemap_coef23 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_22_23.bits.hihdr_tonemap_coef23;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_22_23.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_24_25 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef24 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_24_25.bits.hihdr_tonemap_coef24;
		D32.bits.hihdr_tonemap_coef25 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_24_25.bits.hihdr_tonemap_coef25;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_24_25.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_26_27 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef26 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_26_27.bits.hihdr_tonemap_coef26;
		D32.bits.hihdr_tonemap_coef27 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_26_27.bits.hihdr_tonemap_coef27;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_26_27.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_28_29 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef28 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_28_29.bits.hihdr_tonemap_coef28;
		D32.bits.hihdr_tonemap_coef29 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_28_29.bits.hihdr_tonemap_coef29;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_28_29.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_TONEMAP_COEF_30_31 D32;

		D32.u32 = 0;
		D32.bits.hihdr_tonemap_coef30 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_30_31.bits.hihdr_tonemap_coef30;
		D32.bits.hihdr_tonemap_coef31 =
			channel_cfg->all_reg.VEDU_HIHDR_G_TONEMAP_COEF_30_31.bits.hihdr_tonemap_coef31;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_TONEMAP_COEF_30_31.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_00_09_P0 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef0 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_00_09_P0.bits.hihdr_gmm_coef0;
		D32.bits.hihdr_gmm_coef1 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_00_09_P0.bits.hihdr_gmm_coef1;
		D32.bits.hihdr_gmm_coef2_l8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_00_09_P0.bits.hihdr_gmm_coef2_l8b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_00_09_P0.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_00_09_P1 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef2_h4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_00_09_P1.bits.hihdr_gmm_coef2_h4b;
		D32.bits.hihdr_gmm_coef3 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_00_09_P1.bits.hihdr_gmm_coef3;
		D32.bits.hihdr_gmm_coef4 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_00_09_P1.bits.hihdr_gmm_coef4;
		D32.bits.hihdr_gmm_coef5_l4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_00_09_P1.bits.hihdr_gmm_coef5_l4b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_00_09_P1.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_00_09_P2 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef5_h8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_00_09_P2.bits.hihdr_gmm_coef5_h8b;
		D32.bits.hihdr_gmm_coef6 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_00_09_P2.bits.hihdr_gmm_coef6;
		D32.bits.hihdr_gmm_coef7 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_00_09_P2.bits.hihdr_gmm_coef7;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_00_09_P2.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_00_09_P3 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef8 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_00_09_P3.bits.hihdr_gmm_coef8;
		D32.bits.hihdr_gmm_coef9 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_00_09_P3.bits.hihdr_gmm_coef9;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_00_09_P3.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_10_19_P0 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef10 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_10_19_P0.bits.hihdr_gmm_coef10;
		D32.bits.hihdr_gmm_coef11 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_10_19_P0.bits.hihdr_gmm_coef11;
		D32.bits.hihdr_gmm_coef12_l8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_10_19_P0.bits.hihdr_gmm_coef12_l8b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_10_19_P0.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_10_19_P1 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef12_h4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_10_19_P1.bits.hihdr_gmm_coef12_h4b;
		D32.bits.hihdr_gmm_coef13 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_10_19_P1.bits.hihdr_gmm_coef13;
		D32.bits.hihdr_gmm_coef14 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_10_19_P1.bits.hihdr_gmm_coef14;
		D32.bits.hihdr_gmm_coef15_l4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_10_19_P1.bits.hihdr_gmm_coef15_l4b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_10_19_P1.u32), D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_10_19_P2 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef15_h8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_10_19_P2.bits.hihdr_gmm_coef15_h8b;
		D32.bits.hihdr_gmm_coef16 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_10_19_P2.bits.hihdr_gmm_coef16;
		D32.bits.hihdr_gmm_coef17 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_10_19_P2.bits.hihdr_gmm_coef17;

		vedu_hal_cfg(reg_base,
			 offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_10_19_P2.u32),
			 D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_10_19_P3 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef18 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_10_19_P3.bits.hihdr_gmm_coef18;
		D32.bits.hihdr_gmm_coef19 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_10_19_P3.bits.hihdr_gmm_coef19;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_10_19_P3.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_20_29_P0 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef20 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_20_29_P0.bits.hihdr_gmm_coef20;
		D32.bits.hihdr_gmm_coef21 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_20_29_P0.bits.hihdr_gmm_coef21;
		D32.bits.hihdr_gmm_coef22_l8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_20_29_P0.bits.hihdr_gmm_coef22_l8b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_20_29_P0.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_20_29_P1 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef22_h4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_20_29_P1.bits.hihdr_gmm_coef22_h4b;
		D32.bits.hihdr_gmm_coef23 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_20_29_P1.bits.hihdr_gmm_coef23;
		D32.bits.hihdr_gmm_coef24 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_20_29_P1.bits.hihdr_gmm_coef24;
		D32.bits.hihdr_gmm_coef25_l4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_20_29_P1.bits.hihdr_gmm_coef25_l4b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_20_29_P1.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_20_29_P2 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef25_h8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_20_29_P2.bits.hihdr_gmm_coef25_h8b;
		D32.bits.hihdr_gmm_coef26 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_20_29_P2.bits.hihdr_gmm_coef26;
		D32.bits.hihdr_gmm_coef27 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_20_29_P2.bits.hihdr_gmm_coef27;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_20_29_P2.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_20_29_P3 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef28 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_20_29_P3.bits.hihdr_gmm_coef28;
		D32.bits.hihdr_gmm_coef29 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_20_29_P3.bits.hihdr_gmm_coef29;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_20_29_P3.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_30_39_P0 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef30 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_30_39_P0.bits.hihdr_gmm_coef30;
		D32.bits.hihdr_gmm_coef31 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_30_39_P0.bits.hihdr_gmm_coef31;
		D32.bits.hihdr_gmm_coef32_l8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_30_39_P0.bits.hihdr_gmm_coef32_l8b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_30_39_P0.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_30_39_P1 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef32_h4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_30_39_P1.bits.hihdr_gmm_coef32_h4b;
		D32.bits.hihdr_gmm_coef33 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_30_39_P1.bits.hihdr_gmm_coef33;
		D32.bits.hihdr_gmm_coef34 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_30_39_P1.bits.hihdr_gmm_coef34;
		D32.bits.hihdr_gmm_coef35_l4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_30_39_P1.bits.hihdr_gmm_coef35_l4b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_30_39_P1.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_30_39_P2 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef35_h8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_30_39_P2.bits.hihdr_gmm_coef35_h8b;
		D32.bits.hihdr_gmm_coef36 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_30_39_P2.bits.hihdr_gmm_coef36;
		D32.bits.hihdr_gmm_coef37 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_30_39_P2.bits.hihdr_gmm_coef37;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_30_39_P2.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_30_39_P3 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef38 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_30_39_P3.bits.hihdr_gmm_coef38;
		D32.bits.hihdr_gmm_coef39 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_30_39_P3.bits.hihdr_gmm_coef39;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_30_39_P3.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_40_49_P0 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef40 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_40_49_P0.bits.hihdr_gmm_coef40;
		D32.bits.hihdr_gmm_coef41 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_40_49_P0.bits.hihdr_gmm_coef41;
		D32.bits.hihdr_gmm_coef42_l8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_40_49_P0.bits.hihdr_gmm_coef42_l8b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_40_49_P0.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_40_49_P1 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef42_h4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_40_49_P1.bits.hihdr_gmm_coef42_h4b;
		D32.bits.hihdr_gmm_coef43 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_40_49_P1.bits.hihdr_gmm_coef43;
		D32.bits.hihdr_gmm_coef44 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_40_49_P1.bits.hihdr_gmm_coef44;
		D32.bits.hihdr_gmm_coef45_l4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_40_49_P1.bits.hihdr_gmm_coef45_l4b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_40_49_P1.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_40_49_P2 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef45_h8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_40_49_P2.bits.hihdr_gmm_coef45_h8b;
		D32.bits.hihdr_gmm_coef46 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_40_49_P2.bits.hihdr_gmm_coef46;
		D32.bits.hihdr_gmm_coef47 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_40_49_P2.bits.hihdr_gmm_coef47;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_40_49_P2.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_40_49_P3 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef48 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_40_49_P3.bits.hihdr_gmm_coef48;
		D32.bits.hihdr_gmm_coef49 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_40_49_P3.bits.hihdr_gmm_coef49;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_40_49_P3.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_50_59_P0 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef50 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_50_59_P0.bits.hihdr_gmm_coef50;
		D32.bits.hihdr_gmm_coef51 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_50_59_P0.bits.hihdr_gmm_coef51;
		D32.bits.hihdr_gmm_coef52_l8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_50_59_P0.bits.hihdr_gmm_coef52_l8b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_50_59_P0.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_50_59_P1 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef52_h4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_50_59_P1.bits.hihdr_gmm_coef52_h4b;
		D32.bits.hihdr_gmm_coef53 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_50_59_P1.bits.hihdr_gmm_coef53;
		D32.bits.hihdr_gmm_coef54 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_50_59_P1.bits.hihdr_gmm_coef54;
		D32.bits.hihdr_gmm_coef55_l4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_50_59_P1.bits.hihdr_gmm_coef55_l4b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_50_59_P1.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_50_59_P2 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef55_h8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_50_59_P2.bits.hihdr_gmm_coef55_h8b;
		D32.bits.hihdr_gmm_coef56 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_50_59_P2.bits.hihdr_gmm_coef56;
		D32.bits.hihdr_gmm_coef57 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_50_59_P2.bits.hihdr_gmm_coef57;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_50_59_P2.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_50_59_P3 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef58 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_50_59_P3.bits.hihdr_gmm_coef58;
		D32.bits.hihdr_gmm_coef59 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_50_59_P3.bits.hihdr_gmm_coef59;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_50_59_P3.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_60_69_P0 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef60 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_60_69_P0.bits.hihdr_gmm_coef60;
		D32.bits.hihdr_gmm_coef61 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_60_69_P0.bits.hihdr_gmm_coef61;
		D32.bits.hihdr_gmm_coef62_l8b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_60_69_P0.bits.hihdr_gmm_coef62_l8b;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_60_69_P0.u32),
			D32.u32);
	}

	{
		U_VEDU_HIHDR_G_GMM_COEF_60_69_P1 D32;

		D32.u32 = 0;
		D32.bits.hihdr_gmm_coef62_h4b =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_60_69_P1.bits.hihdr_gmm_coef62_h4b;
		D32.bits.hihdr_gmm_coef63 =
			channel_cfg->all_reg.VEDU_HIHDR_G_GMM_COEF_60_69_P1.bits.hihdr_gmm_coef63;

		vedu_hal_cfg(reg_base,
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_HIHDR_G_GMM_COEF_60_69_P1.u32),
			D32.u32);
	}

	vedu_hal_cfgreg_rcset(channel_cfg, reg_base);

	vedu_hal_cfg_reg_intraset(channel_cfg, reg_base);

	vedu_hal_cfg_reg_lambda_set(channel_cfg, reg_base);

	vedu_hal_cfg_reg_qpg_map_set(channel_cfg, reg_base);

	vedu_hal_cfg_reg_addr_set(channel_cfg, reg_base);

	vedu_hal_cfg_reg_slc_head_set(channel_cfg, reg_base);

	vedu_hal_cfg_smmu(channel_cfg, core_id);
}
