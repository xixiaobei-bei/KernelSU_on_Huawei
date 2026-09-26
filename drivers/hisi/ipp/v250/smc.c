/*
 * Hisilicon IPP Bl31 Driver
 *
 * Copyright (c) 2017 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <asm/uaccess.h>
#include <asm/compiler.h>
#include <securec.h>
#include <bl31_smc.h>

noinline int atfd_hipp_smc(u64 _funcid, u64 _arg0, u64 _arg1, u64 _arg2)
{
	register u64 funcid asm("x0") = _funcid;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;
	asm volatile(
		__asmeq("%0", "x0")
		__asmeq("%1", "x1")
		__asmeq("%2", "x2")
		__asmeq("%3", "x3")
		"smc    #0\n"
		: "+r"(funcid)
		: "r"(arg0), "r"(arg1), "r"(arg2));
	return (int)funcid;
}

int atfhipp_smmu_enable(unsigned int mode)
{
	return atfd_hipp_smc(IPP_FID_SMMUENABLE, mode, 0, 0); /*lint !e570 */
}

int atfhipp_smmu_disable(void)
{
	return atfd_hipp_smc(IPP_FID_SMMUDISABLE, 0, 0, 0); /*lint !e570 */
}

int atfhipp_smmu_smrx(unsigned int sid, unsigned int mode)
{
	return atfd_hipp_smc(IPP_FID_SMMUSMRX, sid, mode, 0); /*lint !e570 */
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hisilicon IPP Bl31 Driver");
MODULE_AUTHOR("isp");
