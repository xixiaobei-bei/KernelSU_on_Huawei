/*
 * Hisilicon BL31 exception driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2019. All rights reserved.
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
#include "hisi_bl31_exception.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <global_ddr_map.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <soc_sctrl_interface.h>
#include <soc_acpu_baseaddr_interface.h>
#include <bl31_platform_memory_def.h>
#include <linux/compiler.h>
#include <asm/compiler.h>
#include <linux/debugfs.h>
#include "../blackbox/rdr_print.h"

static void *sctrl_base = NULL;

u32 get_bl31_exception_flag(void)
{
	u32 val = 0;

	if (sctrl_base)
		val = readl(SOC_SCTRL_SCBAKDATA5_ADDR(sctrl_base));

	return val;
}

void bl31_panic_ipi_handle(void)
{
	pr_info("bl31 panic handler in kernel\n");
	rdr_syserr_process_for_ap((u32)MODID_AP_S_BL31_PANIC, 0ULL, 0ULL);
}

noinline u64 atfd_hisi_service_bl31_dbg_smc(u64 _function_id, u64 _arg0, u64 _arg1, u64 _arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;
	asm volatile (
		      __asmeq("%0", "x0")
		      __asmeq("%1", "x1")
		      __asmeq("%2", "x2")
		      __asmeq("%3", "x3")
		      "smc	#0\n"
		      : "+r"(function_id)
		      : "r"(arg0), "r"(arg1), "r"(arg2));

	return function_id;
}


static int __init hisi_bl31_panic_init(void)
{
	struct rdr_exception_info_s einfo;
	struct device_node *np = NULL;
	uint32_t data[2] = { 0 };
	phys_addr_t bl31_smem_base;
	void *bl31_ctrl_addr = NULL;
	void *bl31_ctrl_addr_phys = NULL;
	s32 ret;

	sctrl_base = (void *)ioremap((phys_addr_t) SOC_ACPU_SCTRL_BASE_ADDR, SCTRL_REG_SIZE);
	if (!sctrl_base) {
		BB_PRINT_ERR("sctrl ioremap faild\n");
		return -EPERM;
	}

	bl31_smem_base = HISI_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_BASE;
	np = of_find_compatible_node(NULL, NULL, "hisilicon, bl31_mntn");
	if (!np) {
		BB_PRINT_ERR("%s: no compatible node found\n", __func__);
		goto iounmap_sctrl;
	}
	/* read bl31 crtl addr ande size from dts , 2 is addr and size count */
	ret = of_property_read_u32_array(np, "hisi,bl31-share-mem", &data[0], 2UL);
	if (ret) {
		BB_PRINT_ERR("%s , get val mem compatible node err\n", __func__);
		goto iounmap_sctrl;
	}
	/* data[0] is bl31 ctrl address , data[1] is reg size */
	bl31_ctrl_addr_phys = (void *)(uintptr_t)(bl31_smem_base + data[0]);
	bl31_ctrl_addr = (void *)ioremap(bl31_smem_base + data[0], (u64)data[1]);
	if (!bl31_ctrl_addr) {
		BB_PRINT_ERR
		    ("%s: %d: allocate memory for bl31_ctrl_addr failed\n",
		     __func__, __LINE__);
		goto iounmap_sctrl;
	}

	pr_info("bl31_ctrl_addr_phys:%pK,bl31_ctrl_addr:%pK\n", bl31_ctrl_addr_phys, bl31_ctrl_addr);

	/* register rdr exception type */
	memset((void *)&einfo, 0, sizeof(einfo));
	einfo.e_modid = MODID_AP_S_BL31_PANIC;
	einfo.e_modid_end = MODID_AP_S_BL31_PANIC;
	einfo.e_process_priority = RDR_ERR;
	einfo.e_reboot_priority = RDR_REBOOT_NOW;
	einfo.e_notify_core_mask = RDR_AP;
	einfo.e_reset_core_mask = RDR_AP;
	einfo.e_reentrant = (u32) RDR_REENTRANT_DISALLOW;
	einfo.e_exce_type = AP_S_BL31_PANIC;
	einfo.e_from_core = RDR_AP;
	memcpy((void *)einfo.e_from_module, (const void *)"RDR BL31 PANIC", sizeof("RDR BL31 PANIC"));
	memcpy((void *)einfo.e_desc, (const void *)"RDR BL31 PANIC", sizeof("RDR BL31 PANIC"));
	ret = (s32)rdr_register_exception(&einfo);
	if (!ret) {
		BB_PRINT_ERR("register bl31 exception fail\n");
		iounmap(bl31_ctrl_addr);
		goto iounmap_sctrl;
	}


	/* enable bl31 switch:route to kernel */
	writel((u32)0x1, bl31_ctrl_addr);
	iounmap(bl31_ctrl_addr);
	goto succ;

iounmap_sctrl:
	iounmap(sctrl_base);
	return -EPERM;
succ:
	return 0;
}

module_init(hisi_bl31_panic_init);
MODULE_DESCRIPTION("hisi bl31 exception driver");
MODULE_ALIAS("hisi bl31 exception module");
MODULE_LICENSE("GPL");
