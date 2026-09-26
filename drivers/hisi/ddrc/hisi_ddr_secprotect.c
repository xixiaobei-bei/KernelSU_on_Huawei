/*
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
#include "hisi_ddr_secprotect.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include "securec.h"
#include <global_ddr_map.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/util.h>
#include <linux/hisi/hisi_ddr.h>
#include <linux/hisi/rdr_hisi_platform.h>
#ifdef CONFIG_HISI_DDR_SUBREASON
#include "hisi_ddr_subreason/hisi_ddr_subreason.h"
#endif
#ifdef CONFIG_DRMDRIVER
#include <linux/hisi/hisi_drmdriver.h>
#endif

#define DMR_SHARE_MEM_PHY_BASE (HISI_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_BASE + DRM_SHARE_MEM_OFFSET)
#define DMSS_OPTI_SHARE_MEM_SIZE 0x8
#define INTRSRC_MASK 0xFFFF

u64 *g_dmss_intr_fiq = NULL;
u64 *g_dmss_intr = NULL;
u64 *g_dmss_subreason = NULL;

struct dmss_subreason_mem {
	unsigned master_reason:8;
	unsigned sub_reason:8;
	unsigned init_flag:4;
};

#define DMSS_SUBREASON_OFFSET 28
#define DMSS_SUBREASON_ADDR (HISI_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_BASE + DMSS_SUBREASON_OFFSET)
#define DMSS_SUBREASON_SIZE 0x4
#define DMSS_SUBREASON_FAIL (-1)
#define DMSS_SUBREASON_SUCCESS 0
#define DMSS_MODID_FAIL 1
#define SUBREASON_MODID_FIND_FAIL 0

#ifdef CONFIG_HISI_DDR_SUBREASON
static void dmss_register_exception(void)
{
	u32 ret, i;
	u32 max_einfo;
	max_einfo = sizeof(g_ddr_einfo) / sizeof(struct rdr_exception_info_s);
	for (i = 0; i < max_einfo; i++) {
		ret = rdr_register_exception(&g_ddr_einfo[i]);
		if (ret == 0)
			pr_err("register dmss rxception fail ddr_einfo[%d]!\n", i);
	}
}

static u32 get_sub_reason_modid(u32 master_reason, u32 sub_reason)
{
	u32 i;
	u32 max_einfo;
	max_einfo = sizeof(g_ddr_einfo) / sizeof(struct rdr_exception_info_s);
	for (i = 0; i < max_einfo; i++)
		if (master_reason == g_ddr_einfo[i].e_exce_type &&
		    sub_reason == g_ddr_einfo[i].e_exce_subtype) {
			pr_info("[%s], modid is [0x%x]\n", __func__, g_ddr_einfo[i].e_modid);
			return g_ddr_einfo[i].e_modid;
		}

	pr_err("cannot find modid!\n");
	return DMSS_MODID_FAIL;
}

static s32 dmss_subreason_get(struct dmss_subreason_mem *para_mem)
{
	struct dmss_subreason_mem *subreason_info = NULL;

	if (g_dmss_subreason == NULL) {
		g_dmss_subreason = ioremap_nocache(DMSS_SUBREASON_ADDR, DMSS_SUBREASON_SIZE);
		if (g_dmss_subreason == NULL) {
			pr_err("%s fiq_handler ioremap_nocache fail\n", __func__);
			return DMSS_SUBREASON_FAIL;
		}
	}

	pr_err("%s share mem data is %pK\n", __func__, g_dmss_subreason);

	subreason_info = (struct dmss_subreason_mem *)g_dmss_subreason;

	para_mem->master_reason = subreason_info->master_reason;
	para_mem->sub_reason = subreason_info->sub_reason;

	pr_err("%s exe info master_reason[%u],sub_reason[%u]\n", __func__,
		para_mem->master_reason, para_mem->sub_reason);

	return DMSS_SUBREASON_SUCCESS;
}

static u32 dmss_subreason_modid_find(void)
{
	struct dmss_subreason_mem subreason_mem = { 0 };
	u32 ret;
	s32 eret;

	eret = dmss_subreason_get(&subreason_mem);
	if (eret == DMSS_SUBREASON_FAIL) {
		pr_err("%s subreason get fail\n", __func__);
		return 0;
	}

	ret = get_sub_reason_modid(subreason_mem.master_reason, subreason_mem.sub_reason);
	if (ret == DMSS_MODID_FAIL) {
		pr_err("%s subreason get modi fail\n", __func__);
		return 0;
	}

	return ret;
}
#else
static u32 dmss_subreason_modid_find(void)
{
	return SUBREASON_MODID_FIND_FAIL;
}
#endif


struct semaphore modemddrc_happen_sem;

void dmss_ipi_handler(void)
{
	pr_crit("dmss_ipi_inform start\n");
	up(&modemddrc_happen_sem);
}

static int modemddrc_happen(void *arg)
{
	pr_err("modemddrc happen start\n");
	down(&modemddrc_happen_sem);
	pr_err("modemddrc happen goto rdr_system_error\n");
	rdr_system_error(RDR_MODEM_DMSS_MOD_ID, 0, 0);
	return 0;
}

static void fiq_print_src(u64 intrsrc)
{
	unsigned int intr = intrsrc & INTRSRC_MASK;

	/* check with FIQ number */
	if (intr == IRQ_WDT_INTR_FIQ) {
		pr_err("fiq triggered by: Watchdog\n");
	} else if (intr == IRQ_DMSS_INTR_FIQ) {
		smp_send_stop();
		pr_err("fiq triggered by: DMSS\n");
	} else {
		pr_err("fiq triggered by: Unknown, intr=0x%x\n", (unsigned int)intrsrc);
	}
}

void dmss_fiq_handler(void)
{
	u32 ret;

	if (g_dmss_intr_fiq == NULL) {
		pr_err("fiq_handler intr ptr is null\n");
		g_dmss_intr_fiq = ioremap_nocache(HISI_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_BASE,
			DMSS_OPTI_SHARE_MEM_SIZE);
		if (g_dmss_intr_fiq == NULL) {
			pr_err("fiq_handler ioremap_nocache fail\n");
			return;
		}
	}
	fiq_print_src(*g_dmss_intr_fiq);
	if ((g_dmss_intr_fiq != NULL) && ((u64)(DMSS_INTR_FIQ_FLAG | IRQ_DMSS_INTR_FIQ) == (*g_dmss_intr_fiq))) {
		pr_err("dmss fiq handler\n");
		pr_err("dmss intr happened. please see bl31 log\n");

		ret = dmss_subreason_modid_find();
		if (ret == 0) {
			pr_err("%s get suberason modid fail\n", __func__);
			rdr_syserr_process_for_ap(MODID_AP_S_DDRC_SEC, 0ULL, 0ULL);
		} else {
			rdr_syserr_process_for_ap(ret, 0ULL, 0ULL);
		}
	}
}


#ifdef CONFIG_DRMDRIVER
int hisi_sec_ddr_set(drm_sec_cfg *sec_cfg, dynamic_ddr_sec_type type)
{
	int ret;
	drm_sec_cfg *p_sec_cfg = NULL;

	p_sec_cfg = (drm_sec_cfg *)ioremap_nocache(DMR_SHARE_MEM_PHY_BASE, sizeof(*sec_cfg));
	if (p_sec_cfg == NULL || sec_cfg == NULL)
		return -1;
	p_sec_cfg->start_addr = sec_cfg->start_addr;
	p_sec_cfg->sub_rgn_size = sec_cfg->sub_rgn_size;
	p_sec_cfg->bit_map = sec_cfg->bit_map;
	p_sec_cfg->sec_port = sec_cfg->sec_port;

	ret = atfd_hisi_service_access_register_smc(ACCESS_REGISTER_FN_MAIN_ID,
		(u64)DMR_SHARE_MEM_PHY_BASE, (u64)type, ACCESS_REGISTER_FN_SUB_ID_DDR_DRM_SET);
	iounmap(p_sec_cfg);
	return ret;
}

int hisi_sec_ddr_clr(dynamic_ddr_sec_type type)
{
	return atfd_hisi_service_access_register_smc(ACCESS_REGISTER_FN_MAIN_ID,
		(u64)DMR_SHARE_MEM_PHY_BASE, (u64)type, ACCESS_REGISTER_FN_SUB_ID_DDR_DRM_CLR);
}
#endif

static int hisi_ddr_secprotect_probe(struct platform_device *pdev)
{

#ifdef CONFIG_HISI_DDR_SUBREASON
	dmss_register_exception();
#endif

	g_dmss_intr_fiq = ioremap_nocache(HISI_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_BASE, DMSS_OPTI_SHARE_MEM_SIZE);
	if (g_dmss_intr_fiq == NULL) {
		pr_err(" ddr ioremap_nocache fail\n");
		return -ENOMEM;
	}

	g_dmss_subreason = ioremap_nocache(DMSS_SUBREASON_ADDR, DMSS_SUBREASON_SIZE);
	if (g_dmss_subreason == NULL) {
		pr_err(" ddr subreason ioremap_nocache fail\n");
		return -ENOMEM;
	}


	sema_init(&modemddrc_happen_sem, 0);


	if (!kthread_run(modemddrc_happen, NULL, "modemddrc_emit"))
		pr_err("create thread modemddrc_happen faild\n");
	return 0;
}

static int hisi_ddr_secprotect_remove(struct platform_device *pdev)
{
	if (g_dmss_intr_fiq != NULL)
		iounmap(g_dmss_intr_fiq);

	g_dmss_intr_fiq = NULL;

	if (g_dmss_intr != NULL)
		iounmap(g_dmss_intr);

	g_dmss_intr = NULL;

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id hs_ddr_of_match[] = {
	{ .compatible = "hisilicon,ddr_secprotect", },
	{},
};
MODULE_DEVICE_TABLE(of, hs_ddr_of_match);
#endif

static struct platform_driver hisi_ddr_secprotect_driver = {
	.probe        = hisi_ddr_secprotect_probe,
	.remove       = hisi_ddr_secprotect_remove,
	.driver       = {
		.name           = "ddr_secprotect",
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(hs_ddr_of_match),
	},
};
module_platform_driver(hisi_ddr_secprotect_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hisi ddr secprotect driver");
MODULE_ALIAS("hisi ddr_secprotect module");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
