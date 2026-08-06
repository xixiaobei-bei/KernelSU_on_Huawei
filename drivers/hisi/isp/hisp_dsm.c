/*
 * driver, hisp_dsm.c
 *
 * Copyright (c) 2013- ISP Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/rproc_share.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#define A7PC_OFFSET         0xA0
#define PMEVCNTR0_EL0       0x1000
#define PMCCNTR_EL0         0x107C

#define ISPCPU_EXC_TYPE_MAX 6
#define ISPCPU_EXC_INFO_MAX 30
/* CRG Regs Offset */
#define CRG_C84_PERIPHISP_SEC_RSTDIS        (0xC84)
#define CRG_C88_PERIPHISP_SEC_RSTSTAT       (0xC88)
#define IP_RST_ISPA7                        (1 << 4)
#define IP_RST_ISP                          (1 << 0)

#define CRG_C90_PERIPHISP_ISPA7_CTRL_BASE   (0xC90)
#define ISPA7_DBGPWRDUP                     (1 << 2)
#define ISPA7_VINITHI_HIGH                  (1 << 1)
#define ISPA7_REMAP_ENABLE                  (1 << 11)

#define CRGPERIPH_CLKDIV10_ADDR         (0x0D0)
#define CRGPERIPH_CLKDIV18_ADDR         (0x0F0)
#define CRGPERIPH_CLKDIV20_ADDR         (0x0F8)
#define CRGPERIPH_PERCLKEN0_ADDR        (0x008)
#define CRGPERIPH_PERSTAT0_ADDR         (0x00C)
#define CRGPERIPH_PERCLKEN3_ADDR        (0x038)
#define CRGPERIPH_PERSTAT3_ADDR         (0x03C)
#define CRGPERIPH_PERCLKEN5_ADDR        (0x058)
#define CRGPERIPH_PERSTAT5_ADDR         (0x05C)
#define CRGPERIPH_PERRSTSTAT0_ADDR      (0x068)
#define CRGPERIPH_PERRSTSTAT3_ADDR      (0x08C)
#define CRGPERIPH_ISOSTAT_ADDR          (0x14C)
#define CRGPERIPH_PERPWRSTAT_ADDR       (0x158)
#define CRGPERIPH_PERPWRACK_ADDR        (0x15C)

/* PMCtrl Regs Offset */
#define PMCTRL_384_NOC_POWER_IDLEACK        (0x384)
#define PMCTRL_388_NOC_POWER_IDLE           (0x388)

/* ISP Regs Offset */
#define REVISION_ID_OFFSET                  (0x20008)

/* PCtrl Regs Offset */
#define PCTRL_PERI_STAT2_ADDR       (0x09C)
#define PCTRL_PERI_STAT10_ADDR      (0x0BC)

/* MEDIA1 Regs Offset */
#define MEDIA1_PERCLKEN0_ADDR        (0x008)
#define MEDIA1_PERSTAT0_ADDR         (0x00C)
#define MEDIA1_PERCLKEN1_ADDR        (0x018)
#define MEDIA1_PERSTAT1_ADDR         (0x01C)
#define MEDIA1_CLKDIV9_ADDR          (0x084)
#define PMC_NOC_POWER_IDLEREQ_0      (0x380)
#define PMC_NOC_POWER_IDLEACK_0      (0x384)
#define PMC_NOC_POWER_IDLE_0         (0x388)
#define SC_BAKDATA11                 (0x438)


#ifdef CONFIG_HUAWEI_DSM
struct dsm_client *client_isp;
struct dsm_client_ops dsm_isp_ops = {
	.poll_state = NULL,
	.dump_func = NULL,
};

struct dsm_dev dev_isp = {
	.name = "dsm_isp",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = &dsm_isp_ops,
	.buff_size = 256,
};
#endif

int hisp_dsm_register(void)
{
	int ret = 0;
#ifdef CONFIG_HUAWEI_DSM
	if (client_isp != NULL)
		return ret;

	client_isp = dsm_register_client(&dev_isp);
	if (client_isp == NULL) {
		pr_err("Failed : dsm_register_client fail\n");
		ret = -EFAULT;
	}
#endif
	return ret;
}

void hisp_dsm_unregister(void)
{
#ifdef CONFIG_HUAWEI_DSM
	if (client_isp != NULL) {
		dsm_unregister_client(client_isp, &dev_isp);
		client_isp = NULL;
	}
#endif
}

static void dump_bootware(void)
{
	int i = 0;
	u64 remap_addr;
	void __iomem *bw_base = NULL;

	remap_addr = get_a7remap_addr();
	if (remap_addr == 0) {
		pr_err("[%s] failed : remap addr.0 err!\n", __func__);
		return;
	}

	bw_base = (void __iomem *)ioremap(remap_addr, SZ_8K);/*lint !e446*/
	for (i = 0; i < 1024; i += 16)
		pr_alert("0x%08x 0x%08x 0x%08x 0x%08x\n",
				__raw_readl(bw_base + i),
				__raw_readl(bw_base + i + 4),
				__raw_readl(bw_base + i + 8),
				__raw_readl(bw_base + i + 12));
	iounmap(bw_base);
}

static void dump_ispa7_regs(void)
{
#define DUMP_A7PC_TIMES (3)
	void __iomem *crg_base;
	void __iomem *cssys_base;
	void __iomem *isp_base;
	unsigned int sec_rststat;
	int i = 0;

	crg_base = get_regaddr_by_pa(CRGPERI);
	cssys_base = get_regaddr_by_pa(CSSYS);
	isp_base = get_regaddr_by_pa(ISPCORE);

	if (crg_base == NULL || cssys_base == NULL || isp_base == NULL) {
		pr_err("[%s] Failed : ioremap.(crg.%pK, cssys.%pK, isp.%pK)\n",
				__func__, crg_base, cssys_base, isp_base);
		return;
	}

	sec_rststat = __raw_readl(crg_base + CRG_C88_PERIPHISP_SEC_RSTSTAT);
	if (sec_rststat != 0)
		pr_alert("sec_rststat.0x%x = 0x%x\n",
			CRG_C88_PERIPHISP_SEC_RSTSTAT, sec_rststat);

	if ((sec_rststat & IP_RST_ISPA7) == 0) {
		for (i = 0; i < DUMP_A7PC_TIMES; i++)
			pr_alert("OFFSET.0x%x, PMEVCNTR0.0x%x, PMCCNTR.0x%x\n",
					__raw_readl(cssys_base + A7PC_OFFSET),
					__raw_readl(cssys_base + PMEVCNTR0_EL0),
					__raw_readl(cssys_base + PMCCNTR_EL0));
	}

	if ((sec_rststat & IP_RST_ISP) == 0)
		pr_alert("isp.dump.version.0x%x\n",
			__raw_readl(isp_base + REVISION_ID_OFFSET));
}

static void dump_crg_regs(void)
{
	void __iomem *crg_base;

	crg_base = get_regaddr_by_pa(CRGPERI);
	if (crg_base == NULL) {
		pr_err("[%s] Failed : ioremap crg_base\n", __func__);
		return;
	}

	pr_alert("DIV10.0x%x, DIV18.0x%x, DIV20.0x%x\n",
		__raw_readl(crg_base + CRGPERIPH_CLKDIV10_ADDR),
		__raw_readl(crg_base + CRGPERIPH_CLKDIV18_ADDR),
		__raw_readl(crg_base + CRGPERIPH_CLKDIV20_ADDR));
	pr_alert("CLKEN0.0x%x, STAT0.0x%x, CLKEN3.0x%x\n",
		__raw_readl(crg_base + CRGPERIPH_PERCLKEN0_ADDR),
		__raw_readl(crg_base + CRGPERIPH_PERSTAT0_ADDR),
		__raw_readl(crg_base + CRGPERIPH_PERCLKEN3_ADDR));
	pr_alert("STAT3.0x%x, CLKEN5.0x%x, STAT5.0x%x\n",
		__raw_readl(crg_base + CRGPERIPH_PERSTAT3_ADDR),
		__raw_readl(crg_base + CRGPERIPH_PERCLKEN5_ADDR),
		__raw_readl(crg_base + CRGPERIPH_PERSTAT5_ADDR));

	pr_alert("RSTSTAT0.0x%x, RSTSTAT3.0x%x, RSTSTAT.0x%x\n",
		__raw_readl(crg_base + CRGPERIPH_PERRSTSTAT0_ADDR),
		__raw_readl(crg_base + CRGPERIPH_PERRSTSTAT3_ADDR),
		__raw_readl(crg_base + CRG_C88_PERIPHISP_SEC_RSTSTAT));

	pr_alert("ISOSTAT.0x%x, PWRSTAT.0x%x, PWRACK.0x%x\n",
		__raw_readl(crg_base + CRGPERIPH_ISOSTAT_ADDR),
		__raw_readl(crg_base + CRGPERIPH_PERPWRSTAT_ADDR),
		__raw_readl(crg_base + CRGPERIPH_PERPWRACK_ADDR));
}

static void dump_noc_regs(void)
{
	void __iomem *pmc_base;

	pmc_base = get_regaddr_by_pa(PMCTRL);
	if (pmc_base == NULL) {
		pr_err("[%s] Failed : ioremap pmc_base\n", __func__);
		return;
	}

	pr_alert("NOC_POWER: IDLEACK.0x%x, IDLE.0x%x\n",
			__raw_readl(pmc_base + PMCTRL_384_NOC_POWER_IDLEACK),
			__raw_readl(pmc_base + PMCTRL_388_NOC_POWER_IDLE));
}

/*lint -e559 */
static int dump_a7boot_stone(void)
{
	struct rproc_shared_para *param = NULL;
	int entry = 0;

	hisp_lock_sharedbuf();
	param = rproc_get_share_para();
	if (param == NULL) {
		pr_err("[%s] Failed : rproc_get_share_para.%pK\n",
			__func__, param);
		hisp_unlock_sharedbuf();
		return -EINVAL;
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
	pr_err("[%s]: FIRMWARE.0x%x, OS.0x%x\n", __func__,
		param->bw_stat, param->fw_stat);
#pragma GCC diagnostic pop
	entry = param->bw_stat.entry;
	hisp_unlock_sharedbuf();

	return entry;
}
/*lint +e559 */

static void dump_ispcpu_stone(void)
{
	return;
}

void hisp_boot_stat_dump(void)
{
	pr_alert("[%s] +\n", __func__);

	dump_a7boot_stone();
	if (use_sec_isp()) {
		isp_secisp_dump();
		return;
	}

	if (use_nonsec_isp()) {
		dump_ispcpu_stone();
		return;
	}
	dump_ispa7_regs();
	dump_crg_regs();
	dump_noc_regs();
	dump_bootware();

	pr_alert("[%s] -\n", __func__);
}
