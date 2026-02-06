/*
 * ras_err_inj.c
 *
 * HISI RAS ERR INJECT driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include "ras_edac.h"
#include <bl31_smc.h>
#include <linux/arm-smccc.h>
#include <linux/io.h>

static u64 write_error_inject(u64 part_num, u64 level, u64 err_type)
{
	struct arm_smccc_res res;

	arm_smccc_smc(RAS_TEST_FID_VALUE, (u64)part_num, level, err_type,
		      0, 0, 0, 0, &res);
	return res.a0;
}

int test_ecc_error_input(u64 level, u64 err_type)
{
	u64 part_num;
	u64 ret;

	part_num = read_cpuid_part_number();
	pr_err("part_num = 0x%lx\n", part_num);
	ret = write_error_inject(part_num, level, err_type);
	if (ret != 0)
		pr_err("ERROR INJECT FAIL\n");
	return ret;
}

int test_bus_error(u64 reg_address, u64 v)
{
	u64 ret;
	void __iomem *reg_data = NULL;

	ret = write_error_inject(L3_BUS_ERROR, 0, 0);
	if (ret != 0)
		pr_err("ERROR INJECT FAIL\n");
	reg_data = ioremap_cache(reg_address, 0x8);
	writeq(v, reg_data);
	iounmap(reg_data);

	return ret;
}
