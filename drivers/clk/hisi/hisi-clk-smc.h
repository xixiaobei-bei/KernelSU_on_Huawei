/*
 * hisi-clk-smc.h
 *
 * hisi sec clk smc cmd support
 *
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
#ifndef __HISI_CLK_SMC_H_
#define __HISI_CLK_SMC_H_

#include <linux/types.h>

/*
 * hisi clk smc handler x0 value, every sec clk cmd needs unique
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
	CKM_CMD_MUX = 0x20
};

#define HISI_CLK_REGISTER_FN_ID		0xc500f0f0

/*
 * this func can help SEC CLK CMD send to BL3
 * BL3 achieve data verification, arg0 is module FN_ID
 */
noinline int atfd_hisi_service_clk_smc(u64 _arg1,
	u64 _arg2, u64 _arg3, u64 _arg4);


#endif /* __HISI_CLK_SMC_H_ */
