/*
 *
 * NoC. (NoC Mntn Module.)
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "dfx_noc.h"
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/syscore_ops.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/clk.h>
#include <linux/hisi/util.h>
#include <pr_log.h>
#include <linux/hisi/bl31_smc.h>
#include <linux/hisi/hisi_noc_modid_para.h>
#include "securec.h"
#define PR_LOG_TAG NOC_TAG

#ifdef CONFIG_HISI_BB
#include <linux/hisi/rdr_hisi_platform.h>
#endif
#include "dfx_noc_err_probe.h"
#include "dfx_noc_packet.h"
#include "dfx_noc_info.h"
#include "dfx_noc_transcation.h"
#include "dfx_noc_dump.h"
#include "dfx_qic.h"

#define CLK_REG_SIZE_DTS 2
#define NOC_AOBUS_TIMEOUT_IRQ 2
#define NOC_TARGET_NAME_BASE_SHIFT 32

static struct noc_node *noc_nodes_array[MAX_NOC_NODES_NR] = { NULL };

static unsigned int nodes_array_idx;

static struct dfx_noc_irqd noc_irqdata[NOC_MAX_IRQ_NR];
static struct platform_device *g_this_pdev;

static unsigned int g_noc_init;
static unsigned int g_giveup_idle_retry;

/* NoC Property From DTS. */
struct dfx_noc_property noc_property_dt = {
	.platform_id = 0x0,
	.pctrl_irq_mask = 0x0,
	.stop_cpu_bus_node_name = NULL,
	.smp_stop_cpu_bit_mask = 0x0,
	.apcu_init_flow_array = {0x0, 0x0},
	.faulten_default_enable = false,
	.packet_enable = false,
	.transcation_enable = false,
	.error_enable = false,
	.noc_timeout_enable = false,
	.noc_aobus_int_enable = false,
	.noc_fama_enable = false,
	.noc_fama_mask = 0x0,
	.bus_node_base_errlog1 = NULL,
	.noc_debug = false,
	.noc_list_target_nums = 0,
	.peri_int0_mask = PERI_CRG_TIMEOUT_MASK,
	.peri_int1_mask = PERI_CRG_TIMEOUT_MASK,
	.masterid_mask = MASTER_ID_MASK, /* bit0~bit5 */
	.noc_npu_easc3_flag = false,
};

static struct of_device_id const dfx_noc_match[] = {
	{
		.compatible = "hisilicon,noc", 
		.data = (void *)NULL
	},
	{}			 /* end */
};

static int g_coreid_fix_flag;
static bool g_sctrl_scbakdata27_flag = false;
static unsigned int g_sctrl_scbakdata27_offset;

int get_coreid_fix_flag(void)
{
	return g_coreid_fix_flag;
}

bool get_sctrl_scbakdata27_flag(void)
{
	return g_sctrl_scbakdata27_flag;
}

unsigned int get_sctrl_scbakdata27_offset(void)
{
	return g_sctrl_scbakdata27_offset;
}

/*
 * When ptype = 1, output the register information of noc with printk,
 * When ptype = 0, write it into the memory of pstore
 */

void noc_record_log_pstorememory(const void __iomem *base, int ptype)
{
	unsigned int offset;
	unsigned int *reg = NULL;
	unsigned int size = SZ_128;

	if (!base)
		return;

	pr_err("[base is 0x%pK]\n ", base);

	if (ptype == NOC_PTYPE_PSTORE) {
		mntn_print_to_ramconsole("Error logger dump:\n");
		for (offset = 0; offset < size; offset += 4 * sizeof(unsigned int)) {
			reg = (unsigned int *)((char *)base + offset);
			mntn_print_to_ramconsole
			    ("[%08x] : %08x %08x %08x %08x\n", offset, *reg, *(reg + 1), *(reg + 2), *(reg + 3));
		}
	} else if (ptype == NOC_PTYPE_UART) {
		pr_err("Error logger dump:\n");
		for (offset = 0; offset < size; offset += 4 * sizeof(unsigned int)) {
			reg = (unsigned int *)((char *)base + offset);
			pr_err("[%08x] : %08x %08x %08x %08x\n", offset, *reg, *(reg + 1), *(reg + 2), *(reg + 3));
		}
	} else {
		pr_err("ptype is error ,[%d]\n", ptype);
	}
}

/* Check if the noc clock is enabled.
 * To be backward-compatible,
 * it returns 1 by defualt, if there is no clock defined
 */
static unsigned int noc_clock_enable(struct dfx_noc_device *noc_dev, struct noc_node *node)
{
	unsigned int flag;

	if (!noc_dev->pcrgctrl_base) {
		if (noc_property_dt.noc_debug == 1)
			pr_err("[%s]: pcrgctrl_base is null.\n", __func__);
		return 1; /*  CRG is not defined */
	}

	/* Clock judgement is platform-dependence. */
	flag = dfx_noc_clock_enable(noc_dev, node);

	if (noc_property_dt.noc_debug == 1)
		pr_err("[%s] node(%s) clock enable=%d\n", __func__, node->name, flag);

	return flag;
}

static int position_of_pmctrl_power_idle_reg(unsigned int pwrack_bit)
{
	unsigned long long pwr_bit = 1;

	pwr_bit = pwr_bit << pwrack_bit;

	if ((pwr_bit & MASK_PMCTRL_POWER_IDLE) != 0)
		return PMCTRL_POWER_IDLE_POISION;

	if ((pwr_bit & MASK_PMCTRL_POWER_IDLE1) != 0)
		return PMCTRL_POWER_IDLE_POISION1;

	if ((pwr_bit & MASK_PMCTRL_POWER_IDLE2) != 0)
		return PMCTRL_POWER_IDLE_POISION2;

	return -1;
}
/* set npu bus unsec */
static noinline u64 dfx_noc_set_npu_nosec(void)
{
	register u64 x0 asm("x0") = (u64)MNTN_NOC_SET_NPU_NOSEC;
	pr_err("%s +\n", __func__);
	asm volatile (
		      __asmeq("%0", "x0")
		      "smc    #0\n"
		      : "+r"(x0));

	pr_err("%s -\n", __func__);
	return x0;
}

/*
 * is_noc_node_available - check power & clock for this node state, on/off
 * @node: poiter to the noc node;
 *
 * If both are available, return 1; otherwise, return 0;
 */
int is_noc_node_available(struct noc_node *node)
{
	int ret;
	struct dfx_noc_device *noc_dev = NULL;
	unsigned int request = 0;
	unsigned int ack = 0;
	unsigned int status = 0;
	unsigned int position = 0;
	unsigned int pwrack_bit = 0;
	unsigned long long pwr_bit = 1;

	if (g_this_pdev == NULL || node == NULL) {
		pr_err("[%s] g_this_pdev or node is NULL!!!\n", __func__);
		return 0;
	}

	noc_dev = platform_get_drvdata(g_this_pdev);
	if (noc_dev == NULL) {
		pr_err("[%s] Can not get device node pointer!!\n", __func__);
		return 0;
	}

	if (noc_dev->preg_list == NULL) {
		pr_err("[%s] noc_dev->preg_list is null!!\n", __func__);
		return 0;
	}

	position = position_of_pmctrl_power_idle_reg(node->pwrack_bit);

	pwr_bit = pwr_bit << node->pwrack_bit;

	switch (position) {
	case PMCTRL_POWER_IDLE_POISION:
		request = readl_relaxed(noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idlereq_offset);
		ack = readl_relaxed(noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idleack_offset);
		status = readl_relaxed(noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idle_offset);
		pwrack_bit = (unsigned int)(MASK_PMCTRL_POWER_IDLE & pwr_bit);

		break;

	case PMCTRL_POWER_IDLE_POISION1:
		request = readl_relaxed(noc_dev->pmctrl_base +
					noc_dev->preg_list->pmctrl_power_idlereq1_offset);
		ack = readl_relaxed(noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idleack1_offset);
		status = readl_relaxed(noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idle1_offset);
		pwrack_bit = (unsigned int)((MASK_PMCTRL_POWER_IDLE1 & pwr_bit) >> GET_PMCTRL_POWER_IDLE1);

		break;

	case PMCTRL_POWER_IDLE_POISION2:
		request = readl_relaxed(noc_dev->pmctrl_base +
					noc_dev->preg_list->pmctrl_power_idlereq2_offset);
		ack = readl_relaxed(noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idleack2_offset);
		status = readl_relaxed(noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idle2_offset);
		pwrack_bit = (unsigned int)((MASK_PMCTRL_POWER_IDLE2 & pwr_bit) >> GET_PMCTRL_POWER_IDLE2);

		break;

	default:
		pr_err("[%s] value of node->pwrack_bit is error !!\n", __func__);
		return 0;
	}

	if (!((request | ack | status) & pwrack_bit) && noc_clock_enable(noc_dev, node)) {
		if (noc_property_dt.noc_npu_easc3_flag && !strncmp(node->name, "npu_bus", strlen("npu_bus"))) {
			ret = dfx_noc_set_npu_nosec();
			if (ret) {
				pr_err("easc3 sec set fail \n");
				return 0;
			}
		}
		return 1;
	}

	return 0;
}

static int noc_node_try_to_giveup_idle(struct noc_node *node)
{
	struct dfx_noc_device *noc_dev = NULL;
	unsigned int status, position;
	unsigned int pwrack_bit = 0;
	unsigned long long pwr_bit = 1;
	void __iomem *pm_idlereq_reg = NULL;
	void __iomem *pm_idle_reg = NULL;

	if (g_this_pdev == NULL || node == NULL) {
		pr_err("[%s] g_this_pdev or node is NULL!!!\n", __func__);
		return -1;
	}

	noc_dev = platform_get_drvdata(g_this_pdev);
	if (noc_dev == NULL) {
		pr_err("[%s] Can not get device node pointer!!\n", __func__);
		return -1;
	}

	position = position_of_pmctrl_power_idle_reg(node->pwrack_bit);

	pwr_bit = pwr_bit << node->pwrack_bit;

	/* try to exit idle state */
	switch (position) {
	case PMCTRL_POWER_IDLE_POISION:{
			pm_idlereq_reg = noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idlereq_offset;
			pm_idle_reg = noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idle_offset;
			pwrack_bit = (unsigned int)(MASK_PMCTRL_POWER_IDLE & pwr_bit);
			break;
		}

	case PMCTRL_POWER_IDLE_POISION1:{
			pm_idlereq_reg = noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idlereq1_offset;
			pm_idle_reg = noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idle1_offset;
			pwrack_bit = (unsigned int)((MASK_PMCTRL_POWER_IDLE1 & pwr_bit) >> GET_PMCTRL_POWER_IDLE1);
			break;
		}

	case PMCTRL_POWER_IDLE_POISION2:{
			pm_idlereq_reg = noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idlereq2_offset;
			pm_idle_reg = noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idle2_offset;
			pwrack_bit = (unsigned int)((MASK_PMCTRL_POWER_IDLE2 & pwr_bit) >> GET_PMCTRL_POWER_IDLE2);
			break;
		}

	default:
		pr_err("[%s] value of node->pwrack_bit is error !!\n", __func__);
		return -1;
	}

	status = readl_relaxed(pm_idle_reg);
	pr_err("[%s] before +, NOC_POWER_IDLE = 0x%x;\n", __func__, status);

	status = readl_relaxed(pm_idlereq_reg);
	status &= ~pwrack_bit;
	status |= (pwrack_bit << HIGH_ENABLE_MASK);
	writel_relaxed(status, pm_idlereq_reg);

	status = readl_relaxed(pm_idle_reg);
	pr_err("[%s] after -, NOC_POWER_IDLE = 0x%x;\n", __func__, status);
	return 0;
}

static int is_noc_err_node_available(struct noc_node *node)
{
	if (node == NULL)
		return 0;

	if ((node->hwirq_type == NOC_ERR_PROBE_IRQ) && (node->eprobe_autoenable)
	    && is_noc_node_available(node))
		return 1;

	return 0;
}

static int is_noc_transcation_node_available(struct noc_node *node)
{
	if (node == NULL)
		return 0;

	if ((node->hwirq_type == NOC_TRANS_PROBE_IRQ) &&
		(node->eprobe_autoenable) &&
		is_noc_node_available(node))
		return 1;

	return 0;
}

static int is_noc_packet_node_available(struct noc_node *node)
{
	if (node == NULL)
		return 0;

	if ((node->hwirq_type == NOC_PACKET_PROBE_IRQ)
	    && (node->eprobe_autoenable)
	    && is_noc_node_available(node))
		return 1;

	return 0;
}

struct platform_device *noc_get_pt_pdev(void)
{
	return g_this_pdev;
}

unsigned int is_noc_init(void)
{
	return g_noc_init;
}

static bool noc_err_smp_stop_acpu(unsigned long long irq_status)
{
	unsigned int errlog_1;

	/* NoC Error Happened On Sys Bus Node. */
	if (irq_status & noc_property_dt.smp_stop_cpu_bit_mask) {
		errlog_1 = readl_relaxed(noc_property_dt.bus_node_base_errlog1);
		/* sys_bus, init flow = acpu */
		if ((errlog_1 & noc_property_dt.apcu_init_flow_array[0]) == noc_property_dt.apcu_init_flow_array[1])
			return true;
	}

	return false;
}

/*
 * Function: noc_get_irq_status
 * Description: noc part, get irq status
 * input: void __iomem *pctrl_base: pctrl virtual base address
 * output: none
 * return: irq status
 */
unsigned long long noc_get_irq_status(const void __iomem *pctrl_base)
{
	unsigned long long ret = 0;

	if (!pctrl_base)
		return 0;

	ret = readl_relaxed(pctrl_base + noc_property_dt.pctrl_peri_stat3_off);
	ret = (ret << 32) | (unsigned int)readl_relaxed(pctrl_base + noc_property_dt.pctrl_peri_stat0_off);
	ret = ret & noc_property_dt.pctrl_irq_mask;

	return ret;
}

static irqreturn_t dfx_noc_timeout_irq_handler(int irq, void *data)
{
	void __iomem *pmctrl_base = NULL;
	struct dfx_noc_device *noc_dev = (struct dfx_noc_device *)data;
	unsigned int pending, scperstatus6;
	unsigned int pending1 = 0;
	unsigned int pending2 = 0;
	unsigned long reset_flag;

	pmctrl_base = noc_dev->pmctrl_base;
	pending = readl_relaxed(pmctrl_base + noc_dev->preg_list->pmctrl_int0_stat_offset);

	if (noc_dev->preg_list->pmctrl_int1_stat_offset)
		pending1 = readl_relaxed(pmctrl_base + noc_dev->preg_list->pmctrl_int1_stat_offset);
	else
		pr_err("pmctrl_int1_stat_offset no need set\n");

	if (noc_dev->preg_list->pmctrl_int2_stat_offset)
		pending2 = readl_relaxed(pmctrl_base + noc_dev->preg_list->pmctrl_int2_stat_offset);
	else
		pr_err("pmctrl_int2_stat_offset no need set\n");

	if (pending)
		pr_err("NoC timeout IRQ occured, PERI_INT_STAT0 = 0x%x\n", pending);

	if (pending1)
		pr_err("NoC timeout IRQ occured, PERI_INT_STAT1 = 0x%x\n", pending1);

	if (pending2)
		pr_err("NoC timeout IRQ occured, PERI_INT_STAT2 = 0x%x\n", pending2);

	scperstatus6 = readl_relaxed(noc_dev->sctrl_base + noc_dev->preg_list->sctrl_scperstatus6_offset);
	pr_err("SCTRL_SCPERSTATUS6 = 0x%x\n", scperstatus6);

	if (pending)
		writel_relaxed(pending, pmctrl_base + noc_dev->preg_list->pmctrl_int0_mask_offset);

	if (pending1)
		writel_relaxed(pending1, pmctrl_base + noc_dev->preg_list->pmctrl_int1_mask_offset);

	if (pending2)
		writel_relaxed(pending2, pmctrl_base + noc_dev->preg_list->pmctrl_int2_mask_offset);

	reset_flag = pending & noc_property_dt.peri_int0_mask;
	reset_flag = (reset_flag << MAX_NOC_REG_BITS) | (pending1 & noc_property_dt.peri_int1_mask);

	if (!reset_flag)
		return IRQ_HANDLED;

#ifdef CONFIG_HISI_BB
	if (check_himntn(HIMNTN_NOC_ERROR_REBOOT))
		/* queue timeout work into noc error work queue. */
		noc_timeout_handler_wq();
#endif
	return IRQ_HANDLED;
}

static irqreturn_t dfx_noc_aobus_timeout_irq_handler(int irq, void *data)
{
	unsigned int pending, int_mask;
	struct dfx_noc_device *noc_dev = (struct dfx_noc_device *)data;

	pending = readl_relaxed(noc_dev->sctrl_base + noc_property_dt.sctrl_second_int_org_offset);

	pr_err("SCTRL_SC_SECOND_INT_ORG = 0x%x\n", pending);

	/* Mask corresponding timeout int. */
	int_mask = readl_relaxed(noc_dev->sctrl_base + noc_property_dt.sctrl_second_int_mask_offset);
	int_mask = (int_mask & (~pending)) & noc_property_dt.noc_aobus_second_int_mask;
	writel_relaxed(int_mask, noc_dev->sctrl_base + noc_property_dt.sctrl_second_int_mask_offset);

#ifdef CONFIG_HISI_BB
	if (check_himntn(HIMNTN_NOC_ERROR_REBOOT))
		rdr_syserr_process_for_ap(MODID_AP_S_NOC, 0, 1);
#endif
	return IRQ_HANDLED;
}

static void dfx_noc_err_probe_irq(void __iomem *porbe_base, struct noc_node *node,
	const struct noc_bus_info *noc_bus)
{
	g_giveup_idle_retry = 0;

	if (is_noc_node_available(node))
		noc_record_log_pstorememory(porbe_base, NOC_PTYPE_PSTORE);

	pr_err("NoC Error Probe:\n");
	pr_err("noc_bus: %s\n", noc_bus->name);
	pr_err("noc_node: %s\n", node->name);

NOC_ERROR_CHECK:
	if (is_noc_node_available(node)) {
		noc_err_probe_hanlder(porbe_base, node);
	} else {
		pr_err("noc_node: %s, module powerdown\n", node->name);
		if (g_giveup_idle_retry++ < MAX_GIVEUP_IDLE_NUM) {
			noc_node_try_to_giveup_idle(node);
			pr_err("noc_node_try_to_giveup_idle , %d time\n",
			       g_giveup_idle_retry);
			goto NOC_ERROR_CHECK;
		}
		pr_err("Finally, noc_node: %s powerdown state cann't be changed!\n",
		       node->name);
		/* while(1);
		 * dead loop for manually check out what happened
		 * on the noc bus
		 */
		WARN_ON(1);
		}
}

static void dfx_noc_get_irq_type(const struct noc_bus_info *noc_bus, int offset,
	struct noc_node *node)
{
	void __iomem *porbe_base = NULL;

	porbe_base = node->eprobe_offset + node->base;
	switch (noc_irqdata[offset].type) {
	case NOC_ERR_PROBE_IRQ:
		if (noc_property_dt.error_enable)
			dfx_noc_err_probe_irq(porbe_base, node, noc_bus);
		return;
	case NOC_PACKET_PROBE_IRQ:
		if (noc_property_dt.packet_enable) {
		/* FixMe: packet probe irq handler */
			pr_err("NoC PACKET Probe:\n");
			pr_err("noc_bus: %s\n", noc_bus->name);
			pr_err("noc_node: %s\n", node->name);
			if (is_noc_node_available(node))
				noc_packet_probe_hanlder(node, porbe_base);
			else
				pr_err("noc_node: %s, module powerdown\n", node->name);
		}
		return;
	case NOC_TRANS_PROBE_IRQ:
		if (noc_property_dt.transcation_enable) {
			/* FixMe: trans probe irq handler */
			pr_err("NoC TRANSCATION Probe:\n");
			pr_err("noc_bus: %s\n", noc_bus->name);
			pr_err("noc_node: %s\n", node->name);

			if (is_noc_node_available(node))
				noc_transcation_probe_hanlder(node, porbe_base, node->bus_id);
			else
				pr_err("noc_node: %s, module powerdown\n", node->name);
		}
		return;
	default:
		pr_err("NoC IRQ type is wrong!\n");
	}
}

static void dfx_noc_reset_check(u32 modid_num, u32 *temp_modid)
{
#ifdef CONFIG_HISI_NOC_MODID_REGISTER
	unsigned int j = 0;
	u32 flag = 0;

	/* modid_num > 0 means irq_type == error probe */
	if ((modid_num > 0) && (g_noc_err_coreid == RDR_AP)) {
		for (; j < modid_num; j++) {
			if (*temp_modid != MODID_NEGATIVE) {
				flag = NOC_MODID_MATCH;
				pr_err("[%s] noc modid is matched, modid = %x!\n", __func__, *temp_modid);
				break;
			}
			pr_err("[%s] cannot match modid = %x, j = %u!\n", __func__, *temp_modid, j);
			temp_modid++;
		}
		if (flag == NOC_MODID_MATCH)
			rdr_system_error(*temp_modid, 0, 0);
		else
			rdr_syserr_process_for_ap(MODID_AP_S_NOC, 0, 0);
	} else {
		if (((modid_num > 0) && (g_giveup_idle_retry > MAX_GIVEUP_IDLE_NUM)))
			rdr_syserr_process_for_ap(MODID_AP_S_NOC, 0, 0);
	}
#else
	if ((modid_num > 0) && (g_giveup_idle_retry > MAX_GIVEUP_IDLE_NUM))
		rdr_syserr_process_for_ap(MODID_AP_S_NOC, 0, 0);
#endif
}

static irqreturn_t dfx_noc_irq_handler(int irq, void *data)
{
	unsigned long long pending;
	int offset;
	u32 modid_num = 0;
	struct noc_node *node = NULL;
	struct dfx_noc_device *noc_dev = (struct dfx_noc_device *)data;
	const struct noc_bus_info *noc_bus = NULL;
	u32 temp_modid[MAX_MODID_NUM] = { MODID_NEGATIVE };

	/* Get NoC Bus Irq Status From PCTRL. */
	pending = noc_get_irq_status(noc_dev->pctrl_base);
#ifdef CONFIG_HUAWEI_PRINTK_CTRL
	printk_level_setup(LOGLEVEL_DEBUG);
#endif
	/* Stop Other Cpus Immediately to Dump Call Stack. */
	if (!noc_property_dt.packet_enable || !noc_property_dt.transcation_enable) {
#ifdef CONFIG_HISI_BB
		if (check_himntn(HIMNTN_NOC_ERROR_REBOOT) && noc_err_smp_stop_acpu(pending)) {
			/* Change Consloe Log Level. */
			console_verbose();
			/* Stop Other CPUs. */
			smp_send_stop();
			/* Debug Print. */
			if (noc_property_dt.noc_debug == 1)
				pr_err("%s -- Stop CPUs First.\n", __func__);
		}
#endif
	}

	pr_err("noc irq status = 0x%llx\n", pending);

	if (noc_property_dt.noc_debug == 1)
		pr_err("Get into %s.\n", __func__);

	if (pending) {
		for_each_set_bit(offset, (unsigned long *)&pending, BITS_PER_LONG) {
			node = noc_irqdata[offset].node;
			if (node == NULL) {
				pr_err("node is null pointer !\n");
				goto noc_ret;
			}
			noc_bus = noc_get_bus_info(node->bus_id);
			if (noc_bus == NULL) {
				pr_err("[%s] noc_bus get error!\n", __func__);
				goto noc_ret;
			}
			dfx_noc_get_irq_type(noc_bus, offset, node);
			if ((modid_num < MAX_MODID_NUM) && (noc_irqdata[offset].type == NOC_ERR_PROBE_IRQ)) {
				pr_err("[%s] err_probe node->modid = %x!\n", __func__, node->noc_modid);
				temp_modid[modid_num++] = node->noc_modid;
			}
		}
	}


#ifdef CONFIG_HISI_BB
	if (check_himntn(HIMNTN_NOC_ERROR_REBOOT))
		dfx_noc_reset_check(modid_num, temp_modid);
#endif

noc_ret:
#ifdef CONFIG_HUAWEI_PRINTK_CTRL
	printk_level_setup(sysctl_printk_level);
#endif
	return IRQ_HANDLED;
}

static int find_bus_id_by_name(const char *bus_name)
{
	int i;
	struct noc_arr_info *pt_noc_arr = NULL;
	const struct noc_bus_info *pt_noc_bus_info = NULL;

	pt_noc_arr = noc_get_buses_info();
	pt_noc_bus_info = pt_noc_arr->ptr;

	for (i = 0; (unsigned int)i < pt_noc_arr->len; i++) {
		if (strncmp(bus_name, pt_noc_bus_info[i].name, MAX_NODE_NAME) == 0)
			return i;
	}
	return -ENODEV;
}

/****************************************************
 *  FUNC: get_noc_node_clock()

 *  Description : Read clock information from NOC DTSI.
 *  In DTSI, the clock defination should be like below:
 *  crg_clk0=<0x0c,22>;
 *  crg_clk1=<0x1c,22>;
 *  If it's not defined in DTSI, fill the struct data by 0xFFFFFFFF
 */
static void get_noc_node_clock(struct device_node *np, struct noc_node *node)
{
	unsigned int clk_reg_u32[2] = { 0 };
	char clock_name[16] = { };
	int i;
	int ret;

	if (!np) {
		if (noc_property_dt.noc_debug)
			pr_err("[%s]:np is NULL\n", __func__);
		return;
	}

	if (!node) {
		if (noc_property_dt.noc_debug)
			pr_err("[%s]:node is NULL\n", __func__);
		return;
	}
	/* Init struct data as 0xFFFFFFFF */
	ret = memset_s(&node->crg_clk[0], sizeof(struct noc_clk) * DFX_NOC_CLOCK_MAX,
			0xff, sizeof(struct noc_clk) * DFX_NOC_CLOCK_MAX);
	if (ret != EOK) {
		pr_err("%s():%d:memset_s fail!\n", __func__, __LINE__);
		return;
	}

	for (i = 0; i < DFX_NOC_CLOCK_MAX; i++) {
		ret = snprintf_s(clock_name, sizeof(clock_name), sizeof(clock_name)-1, "crg_clk%d", i);
		if (ret < 0) {
			pr_err("%s():%d:snprintf_s fail!\n", __func__, __LINE__);
			return;
		}

		ret = of_property_read_u32_array(np, clock_name, clk_reg_u32, CLK_REG_SIZE_DTS);
		if (ret) {
			continue;
		} else {
			if (clk_reg_u32[1] > 32) {	 /* 32 bit register */
				pr_err("[%s]:NOC node[%s] %s  invalid mask_bit=%d\n",
				       __func__, node->name, clock_name, node->crg_clk[i].mask_bit);
				continue;
			}
			node->crg_clk[i].offset = clk_reg_u32[0];
			node->crg_clk[i].mask_bit = clk_reg_u32[1];
			if (noc_property_dt.noc_debug)
				pr_info("[%s]:NOC node[%s] %s  offset=0x%x, mask_bit=%d\n",
					__func__, node->name, clock_name,
					node->crg_clk[i].offset, node->crg_clk[i].mask_bit);
		}
	}
}


static void dfx_noc_err_property(struct device_node *np, struct noc_node *node)
{
	int ret;

	/* err probe property */
	if (noc_property_dt.error_enable) {
		if (of_property_read_bool(np, "eprobe-autoenable"))
			node->eprobe_autoenable = true;

		ret = of_property_read_u32(np, "eprobe-hwirq", (u32 *) &node->eprobe_hwirq);
		if (ret < 0) {
			node->eprobe_hwirq = -1;
			pr_debug("the node doesnot have err probe!\n");
		}

		if (node->eprobe_hwirq >= 0) {
			ret = of_property_read_u32(np, "eprobe-offset", &node->eprobe_offset);
			if (ret < 0) {
				node->eprobe_hwirq = -1;
				pr_debug("the node doesnot have err probe!\n");
			}
		}
	}
}


static int noc_node_info(struct device_node *np, struct noc_node *node)
{
	int ret;
	const char *bus_name = NULL;

	if (!np || !node)
		return -1;

	if (nodes_array_idx >= MAX_NOC_NODES_NR)
		return -1;

	ret = of_property_read_string(np, "bus-name", &bus_name);
	if (ret < 0) {
		pr_err("[%s]: cannot find bus-name\n", __func__);
		return -1;
	}

	ret = find_bus_id_by_name(bus_name);
	if (ret < 0)
		return -1;

	node->bus_id = ret;

	/* Get clock information */
	get_noc_node_clock(np, node);

	/* FIXME: handle the transprobe & packet probe property */
	/* Debug info */
	if (noc_property_dt.noc_debug) {
		pr_err("[%s]: nodes_array_idx = %d\n", __func__, nodes_array_idx);
		pr_err("np->name = %s\n", np->name);
		pr_err("node->bus_id = %d\n", node->bus_id);
		pr_err("node->base = 0x%pK\n", node->base);
		pr_err("node->eprobe_hwirq = %d\n", node->eprobe_hwirq);
		pr_err("node->eprobe_offset = 0x%x\n", node->eprobe_offset);
		pr_err("bus_node_base_errlog1 address = 0x%pK\n", noc_property_dt.bus_node_base_errlog1);
	}

	/* put the node into nodes_arry */
	noc_nodes_array[nodes_array_idx] = node;

	nodes_array_idx++;

	return 0;
}

static int register_noc_node(struct device_node *np)
{
	struct noc_node *node = NULL;
	int ret = 0;

	node = kzalloc(sizeof(struct noc_node), GFP_KERNEL);
	if (!node) {
		ret = -ENOMEM;
		goto err;
	}

	node->base = of_iomap(np, 0);
	if (!node->base) {
		WARN_ON(1);
		goto err_iomap;
	}

	/* err probe property */
	dfx_noc_err_property(np, node);

	node->noc_modid = MODID_NEGATIVE;

	if (node->eprobe_hwirq >= 0) {
		ret = of_property_read_u32(np, "hwirq-type", (u32 *) &node->hwirq_type);
		if (ret < 0) {
			node->hwirq_type = -1;
			pr_err("the node doesnot have hwirq type!\n");
		}
	}
	ret = of_property_read_u32(np, "pwrack-bit", &node->pwrack_bit);
	if (ret < 0) {
		pr_err("the node doesnot have pwrack_bit property!\n");
		ret = -ENODEV;
		goto err_pwbit;
	}

	node->name = kstrdup(np->name, GFP_KERNEL);
	if (!node->name) {
		ret = -ENOMEM;
		goto err_pwbit;
	}

	if (!strncmp(node->name, noc_property_dt.stop_cpu_bus_node_name,
		strlen(noc_property_dt.stop_cpu_bus_node_name))) {
		struct dfx_noc_device *noc_dev = platform_get_drvdata(g_this_pdev);

		if ((noc_dev == NULL) || (noc_dev->perr_probe_reg == NULL)) {
			pr_err("get noc_dev error!\n");
			ret = -ENODEV;
			goto err_property;
		}
		noc_property_dt.bus_node_base_errlog1 = (u8 __iomem *) node->base +
		    node->eprobe_offset + noc_dev->perr_probe_reg->err_probe_errlog1_offset;
	}

	ret = noc_node_info(np, node);
	if (ret < 0) {
		WARN_ON(1);
		goto err_property;
	}
	/* FIXME: handle the other irq */

	return 0;

err_property:
	kfree(node->name);
	node->name = NULL;
err_pwbit:
	iounmap(node->base);
	node->base = NULL;
err_iomap:
	kfree(node);
	node = NULL;
err:
	return ret;
}

static void register_irqdata(void)
{
	struct noc_node *node = NULL;
	unsigned int i;

	for (i = 0; i < nodes_array_idx; i++) {
		GET_NODE_FROM_ARRAY(node, i);
		if (!node) {
			pr_err("[%s]: nodes_array index %d not found.\n", __func__, i);
			continue;
		}

		if (node->eprobe_hwirq >= NOC_MAX_IRQ_NR) {
			pr_err("[%s]: node->eprobe_hwirq %d is out of range %d!\n",
			       __func__, node->eprobe_hwirq, NOC_MAX_IRQ_NR);
			continue;
		}
		if ((node->hwirq_type >= NOC_ERR_PROBE_IRQ)
		    && (node->hwirq_type <= NOC_TRANS_PROBE_IRQ)) {
			noc_irqdata[node->eprobe_hwirq].type = (enum NOC_IRQ_TPYE)node->hwirq_type;
			noc_irqdata[node->eprobe_hwirq].node = node;

			if (node->hwirq_type == NOC_TRANS_PROBE_IRQ)
				init_transcation_info(node);
			else if (node->hwirq_type == NOC_PACKET_PROBE_IRQ)
				init_packet_info(node);
		} else {
			pr_err("[%s]: the node type error!!!\n", __func__);
		}
	}
}

static void register_noc_nodes(void)
{
	struct device_node *np = NULL;

	for_each_compatible_node(np, NULL, "hisilicon,noc-node") {
		register_noc_node(np);
	}

	/* register_irqdata */
	register_irqdata();
}

static void unregister_noc_nodes(void)
{
	struct noc_node *node = NULL;
	unsigned int i;

	for (i = 0; i < nodes_array_idx; i++) {
		GET_NODE_FROM_ARRAY(node, i);
		if (!node) {
			pr_err("[%s]: nodes_array index %u not found.\n", __func__, i);
			continue;
		}

		if (noc_property_dt.error_enable) {
			if (node->hwirq_type == NOC_ERR_PROBE_IRQ)
				disable_err_probe(node->base + node->eprobe_offset);
			else if (node->hwirq_type == NOC_TRANS_PROBE_IRQ)
				disable_transcation_probe(node->base + node->eprobe_offset);
			else if (node->hwirq_type == NOC_PACKET_PROBE_IRQ)
				disable_packet_probe(node->base + node->eprobe_offset);
			else
				pr_err("[%s]: the node type error!!!\n", __func__);
		}

		iounmap(node->base);
		node->base = NULL;
		kfree(node->name);
		node->name = NULL;
		kfree(node);
		node = NULL;
	}

	nodes_array_idx = 0;
}

void __iomem *get_errprobe_base(const char *name)
{
	struct noc_node *node = NULL;
	unsigned int i;

	if (!name)
		return NULL;
	for (i = 0; i < nodes_array_idx; i++) {
		GET_NODE_FROM_ARRAY(node, i);
		if (!node) {
			pr_err("[%s]: nodes_array index %u not found.\n", __func__, i);
			continue;
		}

		if (!strncmp(name, node->name, MAX_NODE_NAME))
			return node->base + node->eprobe_offset;
	}

	pr_warn("[%s]  cannot get node base name = %s\n", __func__, name);
	return NULL;
}
EXPORT_SYMBOL(get_errprobe_base);

/* return node index,
 * error: return -1.
 */
int get_bus_id_by_base(const void __iomem *base)
{
	struct noc_node *node = NULL;
	unsigned int i;

	for (i = 0; i < nodes_array_idx; i++) {
		GET_NODE_FROM_ARRAY(node, i);
		if (!node) {
			pr_err("[%s]: nodes_array index %u not found.\n", __func__, i);
			continue;
		}
		if ((node->base + node->eprobe_offset) == base)
			return node->bus_id;
	}
	pr_warn("[%s]  cannot get bus ID\n", __func__);
	return -1;
}

struct noc_node *get_probe_node(const char *name)
{
	struct noc_node *node = NULL;
	unsigned int i;

	if (!name)
		return NULL;

	for (i = 0; i < nodes_array_idx; i++) {
		GET_NODE_FROM_ARRAY(node, i);
		if (!node) {
			pr_err("[%s]: nodes_array index %u not found.\n", __func__, i);
			continue;
		}

		if (!strncmp(name, node->name, MAX_NODE_NAME))
			return node;
	}

	pr_warn("[%s] cannot get node base name = %s\n", __func__, name);
	return NULL;
}
EXPORT_SYMBOL(get_probe_node);

void noc_get_bus_nod_info(void **node_array_pptr, unsigned int *idx_ptr)
{
	if (!node_array_pptr || !idx_ptr)
		return;

	*node_array_pptr = noc_nodes_array;
	*idx_ptr = nodes_array_idx;

	pr_err("[%s]: nodes_array addr: %pK.\n", __func__, noc_nodes_array);
}

static void enable_errprobe(struct device *dev)
{
	struct noc_node *node = NULL;
	unsigned int i;

	for (i = 0; i < nodes_array_idx; i++) {
		GET_NODE_FROM_ARRAY(node, i);
		if (!node) {
			pr_err("[%s]: nodes_array index %d not found.\n", __func__, i);
			continue;
		}
		if (is_noc_err_node_available(node)) {
			if (noc_property_dt.noc_debug == 1)
				pr_err("Start to enable error probe for bus %s.\n", node->name);

			enable_err_probe(node->base + node->eprobe_offset);
		} else {
			if (noc_property_dt.noc_debug == 1)
				pr_err("Noc %s is powerdown, cannot enable\n", node->name);
		}
	}
}

void disable_errprobe(struct device *dev)
{
	struct noc_node *node = NULL;
	unsigned int i;

	for (i = 0; i < nodes_array_idx; i++) {
		GET_NODE_FROM_ARRAY(node, i);
		if (!node) {
			pr_err("[%s]: nodes_array index %d not found.\n", __func__, i);
			continue;
		}

		if (is_noc_err_node_available(node))
			disable_err_probe(node->base + node->eprobe_offset);
	}
}

void enable_noc_transcation_probe(struct device *dev)
{
	struct noc_node *node = NULL;
	unsigned int i;

	for (i = 0; i < nodes_array_idx; i++) {
		GET_NODE_FROM_ARRAY(node, i);
		if (!node) {
			pr_err("[%s]: nodes_array index %d not found.\n", __func__, i);
			continue;
		}
		if (is_noc_transcation_node_available(node)) {
			if (noc_property_dt.noc_debug == 1)
				pr_err("Start to enable transaction probe for bus %s.\n", node->name);

			enable_transcation_probe(node, node->base + node->eprobe_offset);
		}
	}
}

void disable_noc_transcation_probe(struct device *dev)
{
	struct noc_node *node = NULL;
	unsigned int i;

	for (i = 0; i < nodes_array_idx; i++) {
		GET_NODE_FROM_ARRAY(node, i);
		if (!node) {
			pr_err("[%s]: nodes_array index %d not found.\n", __func__, i);
			continue;
		}
		if (is_noc_transcation_node_available(node))
			disable_transcation_probe(node->base + node->eprobe_offset);
	}
}

void enable_noc_packet_probe(struct device *dev)
{
	struct noc_node *node = NULL;
	unsigned int i;

	for (i = 0; i < nodes_array_idx; i++) {
		GET_NODE_FROM_ARRAY(node, i);
		if (!node) {
			pr_err("[%s]: nodes_array index %d not found.\n", __func__, i);
			continue;
		}

		if (is_noc_packet_node_available(node)) {
			if (noc_property_dt.noc_debug == 1)
				pr_err("Start to enable packet probe for bus %s.\n", node->name);

			enable_packet_probe(node, node->base + node->eprobe_offset);
		}
	}
}

void disable_noc_packet_probe(struct device *dev)
{
	struct noc_node *node = NULL;
	unsigned int i;

	for (i = 0; i < nodes_array_idx; i++) {
		GET_NODE_FROM_ARRAY(node, i);
		if (!node) {
			pr_err("[%s]: nodes_array index %d not found.\n", __func__, i);
			continue;
		}
		if (is_noc_packet_node_available(node))
			disable_packet_probe(node->base + node->eprobe_offset);
	}
}

#ifdef CONFIG_PM_SLEEP
static int dfx_noc_suspend(struct device *dev)
{
	pr_err("%s+\n", __func__);
	if (noc_property_dt.error_enable) {
		if (noc_property_dt.transcation_enable)
			disable_noc_transcation_probe(dev);
		if (noc_property_dt.packet_enable)
			disable_noc_packet_probe(dev);
	}

	pr_err("%s-\n", __func__);
	return 0;
}

static int dfx_noc_resume(struct device *dev)
{
	pr_err("%s+\n", __func__);
	if (noc_property_dt.error_enable) {
		if (!noc_property_dt.faulten_default_enable)
			enable_errprobe(dev);

		if (noc_property_dt.transcation_enable)
			enable_noc_transcation_probe(dev);
		if (noc_property_dt.packet_enable)
			enable_noc_packet_probe(dev);
	}
	pr_err("%s-\n", __func__);
	return 0;
}

static SIMPLE_DEV_PM_OPS(noc_pm_ops, dfx_noc_suspend, dfx_noc_resume);
#endif


static void __dfx_noc_property(struct device_node *np)
{
	const char *dts_str = NULL;
	unsigned int dts_u32_value = 0;
	unsigned long long dts_u64_value = 0;

	/* Read PCTRL IRQ mask Form DTS. */
	if (of_property_read_u64(np, "pctrl-irq-mask", &dts_u64_value) == 0)
		noc_property_dt.pctrl_irq_mask = dts_u64_value;

	/* Read PCTRL PERI STAT0 Offset Form DTS. */
	if (of_property_read_u32(np, "pctrl-peri-stat0-offset", &dts_u32_value) == 0)
		noc_property_dt.pctrl_peri_stat0_off = dts_u32_value;

	/* Read PCTRL PERI STAT3 Offset Form DTS. */
	if (of_property_read_u32(np, "pctrl-peri-stat3-offset", &dts_u32_value) == 0)
		noc_property_dt.pctrl_peri_stat3_off = dts_u32_value;

		/* Read PCTRL PERI STAT3 Offset Form DTS. */
	if (of_property_read_u32(np, "sctrl_scbakdata27_offset", &dts_u32_value) == 0)
		g_sctrl_scbakdata27_offset = dts_u32_value;

	/* Get fama_mask Form DTS. */
	if (of_property_read_u32(np, "fama-mask", &dts_u32_value) == 0)
		noc_property_dt.noc_fama_mask = dts_u32_value;

	/* Get smp_stop_cpu_bit_mask Form DTS. */
	if (of_property_read_u64(np, "smp_stop_cpu_bit_mask", &dts_u64_value) == 0)
		noc_property_dt.smp_stop_cpu_bit_mask = dts_u64_value;

	/* Get stop_cpu_bus_node_name Form DTS. */
	if (of_property_read_string(np, "stop_cpu_bus_node_name", &dts_str) == 0)
		noc_property_dt.stop_cpu_bus_node_name = dts_str;

	/* Get noc timeout pmctrl int0 mask Form DTS. */
	if (of_property_read_u32(np, "pmctrl-int0-bit-mask", &dts_u32_value) == 0)
		noc_property_dt.peri_int0_mask = dts_u32_value;

	/* Get noc timeout pmctrl int1 mask Form DTS. */
	if (of_property_read_u32(np, "pmctrl-int1-bit-mask", &dts_u32_value) == 0)
		noc_property_dt.peri_int1_mask = dts_u32_value;
}
static int dfx_noc_get_property(struct device_node *np)
{
	int ret;
	unsigned int dts_u32_value = 0;
	unsigned int dts_u32_array[NOC_ACPU_INIT_FLOW_ARRY_SIZE] = { 0, 0 };

	/* Get platform_id Form DTS. */
	if (of_property_read_u32(np, "platform_id", &dts_u32_value) == 0)
		noc_property_dt.platform_id = dts_u32_value;
	else
		return -1;

	pr_crit("[%s]: Platform id is [%d], from DTS.\n", __func__, noc_property_dt.platform_id);

	__dfx_noc_property(np);
	/* Get noc_acpu_init_flow_array Form DTS. */
	if (of_property_read_u32_array(np,
					"noc_acpu_init_flow_array",
					dts_u32_array,
					NOC_ACPU_INIT_FLOW_ARRY_SIZE) == 0) {
		ret = memcpy_s(noc_property_dt.apcu_init_flow_array, NOC_ACPU_INIT_FLOW_ARRY_SIZE * sizeof(unsigned int),
						dts_u32_array, sizeof(dts_u32_array));
		if (ret != EOK) {
			pr_err("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
			return -1;
		}
	}

	if (of_property_read_u32(np, "aobus_second_int_mask", &dts_u32_value) == 0)
		noc_property_dt.noc_aobus_second_int_mask = dts_u32_value;

	if (of_property_read_u32(np, "sctrl-second-int-org-offset", &dts_u32_value) == 0)
		noc_property_dt.sctrl_second_int_org_offset = dts_u32_value;

	if (of_property_read_u32(np, "sctrl-second-int-mask-offset", &dts_u32_value) == 0)
		noc_property_dt.sctrl_second_int_mask_offset = dts_u32_value;

	/* Get Fuction Enable Flag Form DTS. */
	noc_property_dt.faulten_default_enable = of_property_read_bool(np, "faulten_default_enable");
	noc_property_dt.packet_enable = of_property_read_bool(np, "packet_enable");
	noc_property_dt.transcation_enable = of_property_read_bool(np, "transcation_enable");
	noc_property_dt.error_enable = of_property_read_bool(np, "error_enable");
	noc_property_dt.noc_debug = of_property_read_bool(np, "noc_debug");
	noc_property_dt.noc_timeout_enable = of_property_read_bool(np, "noc_timeout_enable");
	noc_property_dt.noc_fama_enable = of_property_read_bool(np, "fama_enable");
	noc_property_dt.noc_aobus_int_enable = of_property_read_bool(np, "aobus_int_enable");
	/* Debug Print. */
	if (noc_property_dt.noc_debug) {
		pr_err("NoC Property:\n");
		pr_err("pctrl_irq_mask = 0x%lx\n", (unsigned long)noc_property_dt.pctrl_irq_mask);
		pr_err("pctrl_peri_stat0_off = 0x%x\n", noc_property_dt.pctrl_peri_stat0_off);
		pr_err("pctrl_peri_stat3_off = 0x%x\n", noc_property_dt.pctrl_peri_stat3_off);
		pr_err("smp_stop_cpu_bit_mask = 0x%lx\n", (unsigned long)noc_property_dt.smp_stop_cpu_bit_mask);
		pr_err("noc_fama_mask  = 0x%x\n", noc_property_dt.noc_fama_mask);
		pr_err("faulten_default_enable = %d\n", noc_property_dt.faulten_default_enable);
		pr_err("packet_enable = %d\n", noc_property_dt.packet_enable);
		pr_err("error_enable = %d\n", noc_property_dt.error_enable);
		pr_err("noc_debug = %d\n", noc_property_dt.noc_debug);
		pr_err("noc_timeout_enable = %d\n", noc_property_dt.noc_timeout_enable);
		pr_err("noc_fama_enable = %d\n", noc_property_dt.noc_fama_enable);
		pr_err("stop_cpu_bus_node_name = %s\n", noc_property_dt.stop_cpu_bus_node_name);
		pr_err("apcu_init_flow_array[0,1] = 0x%x,0x%x\n",
		       noc_property_dt.apcu_init_flow_array[0], noc_property_dt.apcu_init_flow_array[1]);
	}

	return 0;
}

static int noc_set_platform_info(unsigned int platform_id)
{
	/* buses information select among platforms */
	return noc_set_buses_info(platform_id);
}

static int dfx_noc_part_probe(struct platform_device *pdev, struct device_node *np)
{
	int nums = 0;
	unsigned int ret, i;
	void __iomem *reg_base[NOC_MAX_BASE] = {NULL};
	struct dfx_noc_device *noc_dev = NULL;

	noc_dev = platform_get_drvdata(pdev);
	if (noc_dev == NULL) {
		pr_err("[%s]: noc_dev  is NULL!!!\n", __func__);
		return -1;
	}

	np = of_find_compatible_node(np, NULL, "hisilicon,noc-dump-reg");
	if (!np) {
		pr_err("[%s], cannot find noc-dump-reg node in dts!\n", __func__);
		return -1;
	}

	ret = of_property_read_u32(np, "reg-dump-nums", (u32 *) &nums);
	if (ret) {
		pr_err("[%s], cannot find reg-dump-nums in dts!\n", __func__);
		return -1;
	}

	if (nums > NOC_MAX_BASE) {
		pr_err("[%s], Reg Dump Nums Overflow in dts!\n", __func__);
		nums = NOC_MAX_BASE;
	}

	for (i = 0; i < (unsigned int)nums; i++) {
		reg_base[i] = of_iomap(np, i);
		WARN_ON(!reg_base[i]);
		if (!reg_base[i])
			goto err_unmap;
	}

	noc_dev->sctrl_base = reg_base[NOC_SCTRL_BASE];

	noc_dev->pctrl_base = reg_base[NOC_PCTRL_BASE];

	noc_dev->pcrgctrl_base = reg_base[NOC_PCRGCTRL_BASE];

	noc_dev->pmctrl_base = reg_base[NOC_PMCTRL_BASE];

	/* Record Media1-CRG base address. */
	noc_dev->media1_crg_base = reg_base[NOC_MEDIA1_CRG_BASE];

	/* Record Media2-CRG base address. */
	noc_dev->media2_crg_base = reg_base[NOC_MEDIA2_CRG_BASE];

	noc_dev->pwrctrl_reg = (u8 __iomem *) noc_dev->pmctrl_base + noc_dev->preg_list->pmctrl_power_idleack_offset;

	if (noc_property_dt.noc_debug == 1) {
		pr_err("[%s]: noc dump map addr:\n", __func__);
		pr_err("      sctrl_base = 0x%pK,\n", noc_dev->sctrl_base);
		pr_err("      pctrl_base = 0x%pK,\n", noc_dev->pctrl_base);
		pr_err("      pmctrl_base = 0x%pK,\n", noc_dev->pmctrl_base);
		pr_err("      pcrgctrl_base = 0x%pK,\n", noc_dev->pcrgctrl_base);
		pr_err("      media1_crg_base = 0x%pK,\n", noc_dev->media1_crg_base);
		pr_err("      media2_crg_base = 0x%pK,\n", noc_dev->media2_crg_base);
	}

	/* Record NoC Prorerty Pointer. */
	noc_dev->noc_property = &noc_property_dt;

	return 0;

err_unmap:
	for (i = 0; i < NOC_MAX_BASE; i++) {
		if (reg_base[i]) {
			iounmap(reg_base[i]);
			reg_base[i] = NULL;
		}
	}

	return -ENODEV;
}

static void dfx_noc_get_reg_list_debug(unsigned int flag_debug, unsigned int nums, struct dfx_noc_device *noc_dev)
{
	if (noc_property_dt.noc_debug) {
		if (flag_debug == 0) {
			pr_err("NoC error-probe reg list:\n");
			pr_err("nums = [%d]\n", nums);
			pr_err("err_probe_coreid_offset = 0x%x\n", noc_dev->perr_probe_reg->err_probe_coreid_offset);
			pr_err("err_probe_revisionid_offset = 0x%x\n",
			       noc_dev->perr_probe_reg->err_probe_revisionid_offset);
			pr_err("err_probe_faulten_offset = 0x%x\n", noc_dev->perr_probe_reg->err_probe_faulten_offset);
			pr_err("err_probe_errvld_offset = 0x%x\n", noc_dev->perr_probe_reg->err_probe_errvld_offset);
			pr_err("err_probe_errclr_offset = 0x%x\n", noc_dev->perr_probe_reg->err_probe_errclr_offset);
			pr_err("err_probe_errlog0_offset = 0x%x\n", noc_dev->perr_probe_reg->err_probe_errlog0_offset);
			pr_err("err_probe_errlog1_offset = 0x%x\n", noc_dev->perr_probe_reg->err_probe_errlog1_offset);
			pr_err("err_probe_errlog3_offset = 0x%x\n", noc_dev->perr_probe_reg->err_probe_errlog3_offset);
			pr_err("err_probe_errlog4_offset = 0x%x\n", noc_dev->perr_probe_reg->err_probe_errlog4_offset);
			pr_err("err_probe_errlog5_offset = 0x%x\n", noc_dev->perr_probe_reg->err_probe_errlog5_offset);
			pr_err("err_probe_errlog7_offset = 0x%x\n", noc_dev->perr_probe_reg->err_probe_errlog7_offset);
		} else {
			pr_err("NoC reg-list:\n");
			pr_err("nums = [%d]\n", nums);
			pr_err("pctrl_stat0_offset = 0x%x\n", noc_dev->preg_list->pctrl_stat0_offset);
			pr_err("pctrl_stat2_offset = 0x%x\n", noc_dev->preg_list->pctrl_stat2_offset);
			pr_err("pctrl_stat3_offset = 0x%x\n", noc_dev->preg_list->pctrl_stat3_offset);
			pr_err("pctrl_ctrl19_offset = 0x%x\n", noc_dev->preg_list->pctrl_ctrl19_offset);
			pr_err("sctrl_scperstatus6_offset = 0x%x\n", noc_dev->preg_list->sctrl_scperstatus6_offset);
			pr_err("pmctrl_int0_stat_offset = 0x%x\n", noc_dev->preg_list->pmctrl_int0_stat_offset);
			pr_err("pmctrl_int0_mask_offset = 0x%x\n", noc_dev->preg_list->pmctrl_int0_mask_offset);
			pr_err("pmctrl_int1_stat_offset = 0x%x\n", noc_dev->preg_list->pmctrl_int1_stat_offset);
			pr_err("pmctrl_int1_mask_offset = 0x%x\n", noc_dev->preg_list->pmctrl_int1_mask_offset);
			pr_err("pmctrl_power_idlereq_offset = 0x%x\n", noc_dev->preg_list->pmctrl_power_idlereq_offset);
			pr_err("pmctrl_power_idleack_offset = 0x%x\n", noc_dev->preg_list->pmctrl_power_idleack_offset);
			pr_err("pmctrl_power_idle_offset = 0x%x\n", noc_dev->preg_list->pmctrl_power_idle_offset);
		}
	}
}

static int dfx_noc_get_reg_list(struct platform_device *pdev)
{
	unsigned int reg_list_nums;
	struct device_node *np = NULL;
	struct dfx_noc_device *noc_dev = NULL;

	if (!pdev) {
		pr_err("[%s]: pdev is NULL!!!\n", __func__);
		return -1;
	}

	noc_dev = platform_get_drvdata(pdev);
	if (!noc_dev) {
		pr_err("[%s]: noc_dev is NULL!!!\n", __func__);
		return -1;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,noc-err-probe-reg");
	if (!np) {
		pr_err("[%s], cannot find noc-dump-list node in dts!\n", __func__);
		return -1;
	}

	/* Get reg offset from DTS. */
	if (of_property_read_u32(np, "reg-list-nums", &reg_list_nums) != 0) {
		pr_err("[%s] Get reg_list_nums from DTS error.\n", __func__);
		return -1;
	}

	noc_dev->perr_probe_reg =
	    (struct dfx_noc_err_probe_reg *)devm_kzalloc(&pdev->dev,
							  sizeof(struct dfx_noc_err_probe_reg), GFP_KERNEL);
	if (!noc_dev->perr_probe_reg) {
		dev_err(&pdev->dev, "get reg-list memory failed.\n");
		return -1;
	}

	/* Get reg-dump-list Form DTS. */
	if (of_property_read_u32_array(np,
				       "reg-err-probe-list",
				       (u32 *) noc_dev->perr_probe_reg, (unsigned long)reg_list_nums) != 0) {
		pr_err("[%s] Get noc-err-probe-reg from DTS error.\n", __func__);
		devm_kfree(&pdev->dev, noc_dev->perr_probe_reg);
		return -1;
	}

	dfx_noc_get_reg_list_debug(0, reg_list_nums, noc_dev);

	np = of_find_compatible_node(NULL, NULL, "hisilicon,noc-reg-state-list");
	if (!np) {
		pr_err("[%s], cannot find noc-dump-list node in dts!\n", __func__);
		devm_kfree(&pdev->dev, noc_dev->perr_probe_reg);
		return -1;
	}

	/* Get reg offset from DTS. */
	if (of_property_read_u32(np, "reg-list-nums", &reg_list_nums) != 0) {
		pr_err("[%s] Get reg_list_nums from DTS error.\n", __func__);
		devm_kfree(&pdev->dev, noc_dev->perr_probe_reg);
		return -1;
	}

	noc_dev->preg_list = (struct dfx_noc_reg_list *)devm_kzalloc(&pdev->dev,
								      sizeof(struct dfx_noc_reg_list), GFP_KERNEL);
	if (!noc_dev->preg_list) {
		devm_kfree(&pdev->dev, noc_dev->perr_probe_reg);
		return -1;
	}

	/* Get reg-dump-list Form DTS. */
	if (of_property_read_u32_array(np,
				       "reg-state-list",
				       (u32 *) noc_dev->preg_list, (unsigned long)reg_list_nums) != 0) {
		pr_err("[%s] Get reg-dump-list from DTS error.\n", __func__);
		devm_kfree(&pdev->dev, noc_dev->perr_probe_reg);
		devm_kfree(&pdev->dev, noc_dev->preg_list);
		return -1;
	}

	dfx_noc_get_reg_list_debug(1, reg_list_nums, noc_dev);

	return 0;
}

static int dfx_noc_get_target_list(struct platform_device *pdev)
{
	unsigned int i;
	unsigned int nums = 0;
	int ret;
	struct device_node *np = NULL;
	struct dfx_noc_device *noc_dev = NULL;
	u64 data[MAX_NOC_LIST_TARGETS];
	char *name[MAX_NOC_LIST_TARGETS] = {NULL};

	if (!pdev) {
		pr_err("[%s]: pdev is NULL!!!\n", __func__);
		return -1;
	}

	noc_dev = platform_get_drvdata(pdev);
	if (!noc_dev) {
		pr_err("[%s]: noc_dev is NULL!!!\n", __func__);
		return -1;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,noc-targets-sub-list");
	if (!np) {
		pr_err("[%s], cannot find noc-targets-sub-list node in dts!\n", __func__);
		return -1;
	}

	if (of_property_read_u32(np, "target_nums", &nums) != 0) {
		pr_err("[%s] Get target_name_list_nums from DTS error.\n", __func__);
		return -1;
	}
	if (nums > MAX_NOC_LIST_TARGETS) {
		pr_err("[%s], Reg Dump Nums Overflow in dts!\n", __func__);
		nums = MAX_NOC_LIST_TARGETS;
	}

	noc_dev->ptarget_list = (struct dfx_noc_target_name_list *)devm_kzalloc(&pdev->dev,
										 sizeof(struct
											dfx_noc_target_name_list) *
										 nums, GFP_KERNEL);
	if (!noc_dev->ptarget_list) {
		dev_err(&pdev->dev, "get target-list memory failed.\n");
		return -1;
	}

	ret = of_property_read_u64_array(np, "target_regs", data, nums);
	if (ret) {
		pr_err("[%s], get target regs fail in dts!\n", __func__);
		devm_kfree(&pdev->dev, noc_dev->ptarget_list);
		return -1;
	}
	ret = of_property_read_string_array(np, "target_names", (const char **)&name[0], nums);
	if (ret < 0) {
		pr_err("[%s], get target names  fail in dts,0x%x!\n", __func__, ret);
		devm_kfree(&pdev->dev, noc_dev->ptarget_list);
		return -1;
	}
	for (i = 0; i < nums; i++) {
		noc_dev->ptarget_list[i].base = (u32) (data[i] >> NOC_TARGET_NAME_BASE_SHIFT);
		noc_dev->ptarget_list[i].end = (u32) (data[i]);
		ret = strncpy_s((void *)noc_dev->ptarget_list[i].name,
			MAX_NOC_TARGET_NAME_LEN, (void *)name[i], strlen(name[i]));
		if (ret != EOK) {
			pr_err("[%s:%d]strncpy_s ret : %d\n", __func__, __LINE__, ret);
			devm_kfree(&pdev->dev, noc_dev->ptarget_list);
			return -1;
		}
	}

	noc_dev->noc_property->noc_list_target_nums = nums;

	return 0;
}

static int noc_irq_register(struct dfx_noc_device *noc_dev, struct platform_device *pdev)
{
	int ret;

	if (noc_property_dt.noc_timeout_enable) {
		ret = platform_get_irq(pdev, NOC_TIMEOUT_IRQ_NUM);
		if (ret < 0) {
			dev_err(&pdev->dev, "cannot find noc timeout IRQ\n");
			goto err;
		}

		noc_dev->timeout_irq = ret;
		pr_debug("[%s] timeout_irq = %d\n", __func__, noc_dev->timeout_irq);

		ret = devm_request_irq(&pdev->dev, noc_dev->timeout_irq,
				       dfx_noc_timeout_irq_handler,
				       IRQF_TRIGGER_HIGH, "dfx_noc_timeout_irq", noc_dev);
		if (ret < 0) {
			dev_err(&pdev->dev, "request timeout irq fail!\n");
			goto err;
		}
	}

	if (noc_property_dt.noc_aobus_int_enable) {
		ret = platform_get_irq(pdev, NOC_AOBUS_TIMEOUT_IRQ);
		if (ret < 0) {
			dev_err(&pdev->dev, "cannot find noc aobus timeout IRQ\n");
			goto err;
		}

		noc_property_dt.noc_aobus_timeout_irq = ret;
		ret = devm_request_irq(&pdev->dev, noc_property_dt.noc_aobus_timeout_irq,
				       dfx_noc_aobus_timeout_irq_handler,
				       IRQF_TRIGGER_HIGH, "dfx_noc_aobus_timeout_irq", noc_dev);
		if (ret < 0) {
			dev_err(&pdev->dev, "request aobus timeout irq fail!\n");
			goto err;
		}
		/* Enable ao bus NoC error/timeout Int. */
		writel_relaxed(noc_property_dt.noc_aobus_second_int_mask,
			       noc_dev->sctrl_base + noc_property_dt.sctrl_second_int_mask_offset);
	}
	return 0;
err:
	return ret;
}

static void enable_noc_func(struct device *dev)
{
	if (dev == NULL)
		return;

	/* enable err probe if Soc don't support faultem auto enable. */
	if (noc_property_dt.error_enable && !noc_property_dt.faulten_default_enable)
		enable_errprobe(dev);

	/* enable noc transcation probe */
	if (noc_property_dt.transcation_enable)
		enable_noc_transcation_probe(dev);

	/* enable noc packet probe */
	if (noc_property_dt.packet_enable)
		enable_noc_packet_probe(dev);
}

static int dfx_noc_control_modem_bandwidth(struct dfx_noc_device *noc_dev, struct device_node *np,
	const char *reg_name)
{
	unsigned int dts_u32_array[PERI_CRG_NUM] = { 0, 0 };
	struct pctrl_bandwith temp;
	struct pctrl_bandwith *noc_control_bandwidth = &temp;
	void __iomem *pctrl_reg_base = NULL;
	int ret;

	/* Get noc_acpu_init_flow_array Form DTS. */
	if (of_property_read_u32_array(np, reg_name, dts_u32_array, PERI_CRG_NUM) == 0) {
		ret = memcpy_s(noc_control_bandwidth, sizeof(*noc_control_bandwidth),
						dts_u32_array, sizeof(dts_u32_array));
		if (ret != EOK) {
			pr_err("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
			return -1;
		}
	} else {
		pr_err("dfx noc no need to set ptrl modem bandwith\n");
		return 0;
	}

	pctrl_reg_base = noc_dev->pctrl_base + noc_control_bandwidth->offset;
	writel_relaxed(noc_control_bandwidth->val, pctrl_reg_base);

	return 0;
}
static void set_modem_bandwidth(struct dfx_noc_device *noc_dev, struct device_node *np)
{
	char *reg_name = NULL;
	int ret;

	reg_name = "peri_ctrl_119";
	ret = dfx_noc_control_modem_bandwidth(noc_dev, np, reg_name);
	if (ret < 0)
		pr_err("dfx noc set peri_ctrl_119 fail\n");
	reg_name = "peri_ctrl_120";
	ret = dfx_noc_control_modem_bandwidth(noc_dev, np, reg_name);
	if (ret < 0)
		pr_err("dfx noc set peri_ctrl_120 fail\n");
}
static int dfx_noc_read_dts(struct platform_device *pdev, struct device_node *np,
	struct dfx_noc_device *noc_dev)
{
	int ret = 0;
	struct device *dev = NULL;

	if (NULL == pdev || NULL == np) {
		pr_err("[%s]: pdev or np is NULL!!!\n", __func__);
		goto err;
	}

	dev = &pdev->dev;
	/* Get noc soc register offset from DTS. */
	ret = dfx_noc_get_reg_list(pdev);
	if (ret != 0) {
		pr_err("dfx_noc_get_reg_list err\n");
		goto err;
	}

	/* Transfer noc device pointer to error probe module. */
	ret = noc_err_save_noc_dev(noc_dev);
	if (ret != 0) {
		pr_err("noc_err_save_noc_dev err\n");
		goto err;
	}

	ret = dfx_noc_part_probe(pdev, np);
	if (ret != 0) {
		pr_err("dfx_noc_part_probe err\n");
		goto err;
	}
	set_modem_bandwidth(noc_dev, np);
	/* Get noc soc get target name list from DTS. */
	dfx_noc_get_target_list(pdev);
	/* process each noc node */
	register_noc_nodes();

	enable_noc_func(dev);

err:
	return ret;
}

static int dfx_noc_irq_init(struct platform_device *pdev, struct dfx_noc_device *noc_dev)
{
	int ret;

	/* noc err worker register */
	noc_err_probe_init();

	ret = platform_get_irq(pdev, NOC_ERR_PROBE_IRQ_NUM);
	if (ret < 0) {
		dev_err(&pdev->dev, "cannot find IRQ\n");
		return -1;
	}

	noc_dev->irq = ret;
	if (noc_property_dt.noc_debug == 1)
		pr_err("[%s] noc_irq = %d\n", __func__, noc_dev->irq);

	ret = devm_request_irq(&pdev->dev, noc_dev->irq, dfx_noc_irq_handler,
		IRQF_TRIGGER_HIGH, "dfx_noc", noc_dev);
	if (ret < 0) {
		dev_err(&pdev->dev, "request_irq fail!\n");
		return -1;
	}

	ret = noc_irq_register(noc_dev, pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "noc_irq_register fail\n");
		return -1;
	}

	return 0;
}


static int dfx_noc_dts_pre(struct platform_device *pdev, struct device_node *np)
{
	int ret;
	bool flag = 0;

	/* Get noc property from DTS. */
	ret = dfx_noc_get_property(np);
	if (ret != 0) {
		pr_err("dfx_noc_get_property err\n");
		return -1;
	}

	/* Get noc data(initflow/targetflow/master id etc) from DTS. */
	flag = of_property_read_bool(np, "noc_flag");
	if (flag) {
		ret = dfx_noc_get_data_from_dts(pdev);
		if (ret != 0) {
			pr_err("dfx_noc_get_data err\n");
			return -1;
		}
	}

	flag = of_property_read_bool(np, "coreid_fix_flag");
	if (flag)
		g_coreid_fix_flag = 1;

	flag = of_property_read_bool(np, "sctrl_scbakdata27_flag");
	if (flag)
		g_sctrl_scbakdata27_flag = 1;

	flag = of_property_read_bool(np, "npu_easc3_flag");
	if (flag)
		noc_property_dt.noc_npu_easc3_flag = 1;

	/* Platform information should be executed as early as
	 * the moment getting g_config_dfx_noc_data
	 */
	ret = noc_set_platform_info(noc_property_dt.platform_id);
	if (ret != 0) {
		pr_err("noc_set_platform_info err\n");
		return -1;
	}

	return 0;
}

static int dfx_noc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dfx_noc_device *noc_dev = NULL;
	struct device_node *np = NULL;
	const struct of_device_id *match = NULL;
	int ret = 0;

	pr_info("dfx NOC PROBE START!!!\n");

	g_this_pdev = pdev;	 /* Get platform device pointer */

	/* to check which type of regulator this is */
	match = of_match_device(dfx_noc_match, dev);
	if (match == NULL) {
		pr_err("[%s]: mismatch of dfx noc driver\n\r", __func__);
		WARN_ON(1);
		goto err;
	}

	noc_dev = devm_kzalloc(&pdev->dev, sizeof(struct dfx_noc_device), GFP_KERNEL);
	if (!noc_dev) {
		ret = -ENOMEM;
		goto err;
	}

	platform_set_drvdata(pdev, noc_dev);

	/* get the pwrctrl_reg offset  */
	np = dev->of_node;

	ret = dfx_noc_dts_pre(pdev, np);
	if (ret < 0) {
		dev_err(dev, "cannot get dts\n");
		devm_kfree(&pdev->dev, noc_dev);
		ret = -1;
		goto err;
	}

	ret = dfx_noc_read_dts(pdev, np, noc_dev);
	if (ret < 0) {
		devm_kfree(&pdev->dev, noc_dev);
		goto err;
	}

	ret = dfx_noc_irq_init(pdev, noc_dev);
	if (ret == -1) {
		dev_err(&pdev->dev, "dfx_noc_irq_init fail\n");
		devm_kfree(&pdev->dev, noc_dev);
		goto err;
	}

	if (noc_dump_init() == -1) {
		pr_err("[%s] noc_dump_init failed!\n", __func__);
		devm_kfree(&pdev->dev, noc_dev);
		ret = -1;
		goto err;
	}

	g_noc_init = 1;
	return 0;

err:
	return ret;
}

static int dfx_noc_remove(struct platform_device *pdev)
{
	struct dfx_noc_device *noc_dev = platform_get_drvdata(pdev);

	unregister_noc_nodes();
	free_noc_bus_source();

	if (noc_dev != NULL) {
		free_irq(noc_dev->irq, noc_dev);

		iounmap(noc_dev->pctrl_base);
		noc_dev->pctrl_base = NULL;

		iounmap(noc_dev->sctrl_base);
		noc_dev->sctrl_base = NULL;

		iounmap(noc_dev->pcrgctrl_base);
		noc_dev->pcrgctrl_base = NULL;

		iounmap(noc_dev->pmctrl_base);
		noc_dev->pmctrl_base = NULL;

		iounmap(noc_dev->media1_crg_base);
		noc_dev->media1_crg_base = NULL;

		iounmap(noc_dev->media2_crg_base);
		noc_dev->media2_crg_base = NULL;
	}

	noc_err_probe_exit();

	return 0;
}

MODULE_DEVICE_TABLE(of, dfx_noc_match);

static struct platform_driver dfx_noc_driver = {
	.probe = dfx_noc_probe,
	.remove = dfx_noc_remove,
	.driver = {
		   .name = MODULE_NAME,
		   .owner = THIS_MODULE,
#ifdef CONFIG_PM_SLEEP
		   .pm = &noc_pm_ops,
#endif
		   .of_match_table = of_match_ptr(dfx_noc_match),
		   },
};

static int __init dfx_noc_init(void)
{
	return platform_driver_register(&dfx_noc_driver);
}

static void __exit dfx_noc_exit(void)
{
	platform_driver_unregister(&dfx_noc_driver);
}

late_initcall(dfx_noc_init);
module_exit(dfx_noc_exit);
