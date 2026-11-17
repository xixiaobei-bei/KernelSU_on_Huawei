/*
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __CLK_SMC_H_
#define __CLK_SMC_H_

#include <linux/types.h>

/*
 * clk smc handler x0 value, every sec clk cmd needs unique
 * to prevent impact on other commands
 */
enum {
	CKM_ENABLE = 0x0,
	CKM_DISABLE,
	CKM_SENSOR_CFG,
	CKM_SENSOR_ENABLE,
	CKM_SENSOR_DISABLE,
	CKM_QUERY_SENSOR,
	CKM_QUERY_ALARM,
	CKM_CLEAR_IRQ,
	CKM_BUS_FREQ_SET,
	CLK_PLL_ENABLE,
	CLK_PLL_DISABLE,
	CKM_CMD_MUX = 0x20
};

#define CLK_REGISTER_FN_ID		0xc500f0f0

struct smc_ppll {
	uint64_t smc_fid;
	uint64_t arg1;
	uint64_t arg2;
	uint64_t arg3;
	uint64_t arg4;
};

/*
 * this func can help SEC CLK CMD send to BL3
 * BL3 achieve data verification, arg0 is module FN_ID
 */
noinline int atfd_service_clk_smc(u64 _arg1,
	u64 _arg2, u64 _arg3, u64 _arg4);

noinline struct smc_ppll atfd_ppll_clk_smc(struct smc_ppll args);

#endif /* __CLK_SMC_H_ */
