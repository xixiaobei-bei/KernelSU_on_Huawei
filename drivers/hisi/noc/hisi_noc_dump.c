/*
 * NoC. (NoC Mntn Module.)
 *
 * Copyright (c) 2016 Huawei Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

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
#include <linux/hisi/reset.h>
#include <linux/clk.h>
#include <linux/hisi/hisi_log.h>
#define HISI_LOG_TAG HISI_NOC_TAG

#include <securec.h>
#include "hisi_noc.h"
#include "hisi_noc_err_probe.h"
#include "hisi_noc_packet.h"
#include "hisi_noc_info.h"
#include "hisi_noc_transcation.h"
#include "hisi_noc_dump.h"
#ifdef CONFIG_HISI_BB
#include <linux/hisi/rdr_hisi_platform.h>
#endif

/* Gloable Stucture For Recording Hisi NoC Dump Info.*/
static struct hisi_noc_dump_info g_noc_dump_info;

static int noc_dump_err_log_parse(struct err_probe_msg *pt_err_msg)
{
	NOC_L_D_PRT("[err_code=%d] %s\n", pt_err_msg->err_code.val, pt_err_msg->err_code.pt_str);
	NOC_L_D_PRT("[opc=%d] %s\n", pt_err_msg->opc.val, pt_err_msg->opc.pt_str);
	/* ERR LOG 1 */
	NOC_L_D_PRT("[init_flow=%d]: %s\n", pt_err_msg->init_flow.val, pt_err_msg->init_flow.pt_str);
	NOC_L_D_PRT("[target_flow=%d]: %s\n", pt_err_msg->target_flow.val, pt_err_msg->target_flow.pt_str);
	NOC_L_D_PRT("[target_subrange]: %d\n", pt_err_msg->targetsubrange);
	/* ERR LOG 3,4 */
	NOC_L_D_PRT("ADDRESS_LOW = 0x%x\n", pt_err_msg->addr_low);
	NOC_L_D_PRT("ADDRESS_HIGH = 0x%x\n", pt_err_msg->addr_high);
	NOC_L_D_PRT("adjusted ADDRESS = 0x%llx\n",
		    (u64) (pt_err_msg->addr_low + pt_err_msg->base_addr) +
		    ((u64) pt_err_msg->addr_high << 32) + ((u64) (pt_err_msg->user_signal & 0x7c0) << 34));
	/* ERR LOG 5 */
	NOC_L_D_PRT("[USER_SIGNAL = 0x%x MID: %s\n", pt_err_msg->user_signal, pt_err_msg->mid_name);
	/* ERR LOG 7 */
	NOC_L_D_PRT("SECURITY = %d\n", pt_err_msg->SECURITY);
	return 0;
}

static int noc_dump_err_probe(void)
{
	struct noc_node *node = NULL;
	unsigned int j, k;
	u8 __iomem *iobase = NULL;
	struct err_probe_msg err_msg;
	struct err_probe_msg *pt_err_msg = &err_msg;
	uint reg_val_buf[10];
	unsigned long long pending;
	uint faulten, errvld;

	NOC_L_D_PRT("***ErrorProbe status***\n");

	pending = noc_get_irq_status(g_noc_dump_info.noc_device_ptr->pctrl_base);

	for (j = 0; j < g_noc_dump_info.noc_bus_node_info.noc_node_idx; j++) {
		node = g_noc_dump_info.noc_bus_node_info.noc_node_pptr[j];

		if (node == NULL)
			break;

		/* Only Dump Error Probe Type NoC Bus Node. */
		if (node->hwirq_type != NOC_ERR_PROBE_IRQ)
			continue;

		if (!is_noc_node_available(node)) {
			pr_err("[%s]: this node is power down, cannot access.\n", node->name);
			continue;
		}
		iobase = node->base + node->eprobe_offset;
		faulten = readl_relaxed(iobase +
					g_noc_dump_info.noc_device_ptr->perr_probe_reg->err_probe_faulten_offset);
		errvld = readl_relaxed(iobase +
				       g_noc_dump_info.noc_device_ptr->perr_probe_reg->err_probe_errvld_offset);
		if (!(pending & BIT((unsigned int)node->eprobe_hwirq))
		    && !(errvld & ERR_PORBE_ERRVLD_BIT)) {
			pr_err("[%s]:no error probe happens on this node!\n", node->name);
			continue;
		}
		NOC_L_D_PRT("ERR_PORBE_BUS is [%s]\n", g_noc_dump_info.noc_bus_node_info.noc_node_pptr[j]->name);
		NOC_L_D_PRT("FAULTEN_OFFSET = 0x%x\n", faulten);
		NOC_L_D_PRT("ERR_VLD = 0x%x\n", errvld);

		for (k = 0; k < 8; k++) {
			reg_val_buf[k] = readl_relaxed(iobase +
			g_noc_dump_info.noc_device_ptr->perr_probe_reg->err_probe_errlog0_offset + 4 * k);
			NOC_L_D_PRT("ERRLOG %d = 0x%08x\n", k, reg_val_buf[k]);
		}
		/* Parse error log and dump */
		noc_err_get_msg(reg_val_buf, node->bus_id, pt_err_msg);
		noc_dump_err_log_parse(pt_err_msg);
	}
	return 0;
}

static int noc_dump_prt(void)
{
	uint reg_value, reg_val1;
	/* NoPendingTrans status */
	NOC_L_D_PRT("***NoPendingTrans status***\n");
	reg_value = readl_relaxed((u8 __iomem *) g_noc_dump_info.noc_device_ptr->pctrl_base
				  + g_noc_dump_info.noc_device_ptr->preg_list->pctrl_stat2_offset);
	NOC_L_D_PRT("PCTRL_PERI_STAT2 = 0x%x\n", reg_value);
	reg_value = readl_relaxed((u8 __iomem *) g_noc_dump_info.noc_device_ptr->sctrl_base
				  + g_noc_dump_info.noc_device_ptr->preg_list->sctrl_scperstatus6_offset);
	NOC_L_D_PRT("SCTRL_SCPERSTATUS6 = 0x%x\n", reg_value);
	/* LowPower HandInHand */
	NOC_L_D_PRT("***LowPower HandInHand***\n");
	reg_value =
	    readl_relaxed((u8 __iomem *) g_noc_dump_info.noc_device_ptr->pmctrl_base
			  + g_noc_dump_info.noc_device_ptr->preg_list->pmctrl_power_idlereq_offset);
	NOC_L_D_PRT("PMCTRL_NOC_POWER_IDLEREQ = 0x%x\n", reg_value);
	reg_value =
	    readl_relaxed((u8 __iomem *) g_noc_dump_info.noc_device_ptr->pmctrl_base
			  + g_noc_dump_info.noc_device_ptr->preg_list->pmctrl_power_idleack_offset);
	NOC_L_D_PRT("PMCTRL_NOC_POWER_IDLEACK = 0x%x\n", reg_value);
	reg_value = readl_relaxed((u8 __iomem *) g_noc_dump_info.noc_device_ptr->pmctrl_base
				  + g_noc_dump_info.noc_device_ptr->preg_list->pmctrl_power_idle_offset);
	NOC_L_D_PRT("PMCTRL_NOC_POWER_IDLE = 0x%x\n", reg_value);
	/* Timeout status */
	NOC_L_D_PRT("***Timeout status***\n");
	reg_value =
	    readl_relaxed((u8 __iomem *) g_noc_dump_info.noc_device_ptr->pmctrl_base
			  + g_noc_dump_info.noc_device_ptr->preg_list->pmctrl_int0_stat_offset);
	NOC_L_D_PRT("PMCTRL_PERI_INT0_STAT = 0x%x\n", reg_value);
	reg_val1 = readl_relaxed((u8 __iomem *) g_noc_dump_info.noc_device_ptr->sctrl_base
				 + g_noc_dump_info.noc_device_ptr->preg_list->sctrl_scperstatus6_offset);
	NOC_L_D_PRT("SCTRL_SCPERSTATUS6 = 0x%x\n", reg_val1);
	/* ErrorProbe */
	if (!IS_NOC_TMOUT_HAPPEN(reg_value, reg_val1)) {
		/* Timeout happened , do not dump error probe register */
		noc_dump_err_probe();
	}

	/* Address interleaving status */
	NOC_L_D_PRT("***Address interleaving status***\n");
	reg_value = readl_relaxed((u8 __iomem *) g_noc_dump_info.noc_device_ptr->pctrl_base
				  + g_noc_dump_info.noc_device_ptr->preg_list->pctrl_ctrl19_offset);
	NOC_L_D_PRT("PCTRL_PERI_CTRL19 = 0x%x\n", reg_value);
	return 0;
}

static int noc_dump_err_type(struct noc_node *node)
{
	if (node->hwirq_type != NOC_ERR_PROBE_IRQ) {
		pr_err("[%s]:Not NoC Error Probe Bus Node.\n", node->name);
		return FAIL;
	}

	if (!is_noc_node_available(node)) {
		pr_err("[%s]:NoC Bus Node Not Availble.\n", node->name);
		return FAIL;
	}

	return 0;
}

static int noc_dump_reg_init(struct noc_dump_data *pt_dump)
{
	uint *ptr = NULL;
	unsigned int reg_num = 0;
	unsigned int i;

	if (!pt_dump) {
		pr_err("[%s:%d]pt_dump is null\n", __func__, __LINE__);
		return -1;
	}

	/* Dump NoC Related Register into bbox.bin. */
	ptr = (u32 *) &pt_dump->noc_reg_array;
	reg_num = hisi_noc_get_dump_reg_list_num();
	if (reg_num > NOC_DUMP_MAX_REG_NUM) {
		pr_err("NoC Dump Register Number Overflow.\n");
		reg_num = NOC_DUMP_MAX_REG_NUM;
	}

	for (i = 0; i < reg_num; i++) {
		if (noc_dump_reg_list[i].addr == NULL)
			return -1;
		*ptr = readl_relaxed((u8 __iomem *) noc_dump_reg_list[i].addr + noc_dump_reg_list[i].offset);
		ptr++;
	}

	return 0;
}

static int noc_dump_reg_data(struct noc_dump_data *pt_dump)
{
	struct noc_node *node = NULL;
	u8 __iomem *iobase = NULL;
	uint *ptr = NULL;
	unsigned int node_idx = 0;
	unsigned int i;
	int ret;

	ret = noc_dump_reg_init(pt_dump);
	if (ret < 0) {
		pr_err("[%s:%d]pt_dump is null\n", __func__, __LINE__);
		return -1;
	}

	/* ERR PROBE Register dumping start */
	for (i = 0; i < g_noc_dump_info.noc_bus_node_info.noc_node_idx; i++) {
		node = g_noc_dump_info.noc_bus_node_info.noc_node_pptr[i];
		if (node == NULL) {
			pr_err("NoC Bus Node Pointer = NULL .\n");
			break;
		}

		/* Only Dump Error Probe Type NoC Bus Node. */
		if (noc_dump_err_type(node) != 0)
			continue;

		ptr = (u32 *) &pt_dump->ErrorProbe[node_idx];

		/* Dump Node Name into bbox.bin. */
		if (strlen(node->name) >= NOC_DUMP_NOC_NODE_NAME_LEN) {
			pr_err("Error: NoC Bus Node Name Too Long : [%d].\n", (unsigned int)strlen(node->name));
			ret = strncpy_s((char *)ptr, NOC_DUMP_NOC_NODE_NAME_LEN,
				(char *)node->name, (NOC_DUMP_NOC_NODE_NAME_LEN - 1));
			if (ret != EOK) {
				pr_err("[%s:%d]strncpy_s ret : %d\n", __func__, __LINE__, ret);
				return -1;
			}
		} else {
			ret = strncpy_s((char *)ptr, NOC_DUMP_NOC_NODE_NAME_LEN,
				(char *)node->name, strlen(node->name));
			if (ret != EOK) {
				pr_err("[%s:%d]strncpy_s ret : %d\n", __func__, __LINE__, ret);
				return -1;
			}
		}

		ptr += NOC_DUMP_NOC_NODE_NAME_LEN / sizeof(uint);

		/* Enable Flag. */
		*ptr++ = NOC_DUMP_NODE_EN_F;
		iobase = (u8 __iomem *) node->base + node->eprobe_offset;

		*ptr++ = readl_relaxed(iobase +
				       g_noc_dump_info.noc_device_ptr->perr_probe_reg->err_probe_faulten_offset);
		*ptr++ = readl_relaxed(iobase +
				       g_noc_dump_info.noc_device_ptr->perr_probe_reg->err_probe_errvld_offset);
		*ptr++ = readl_relaxed(iobase +
				       g_noc_dump_info.noc_device_ptr->perr_probe_reg->err_probe_errlog0_offset);
		*ptr++ = readl_relaxed(iobase +
				       g_noc_dump_info.noc_device_ptr->perr_probe_reg->err_probe_errlog1_offset);
		*ptr++ = readl_relaxed(iobase +
				       g_noc_dump_info.noc_device_ptr->perr_probe_reg->err_probe_errlog3_offset);
		*ptr++ = readl_relaxed(iobase +
				       g_noc_dump_info.noc_device_ptr->perr_probe_reg->err_probe_errlog5_offset);
		*ptr++ = readl_relaxed(iobase +
				       g_noc_dump_info.noc_device_ptr->perr_probe_reg->err_probe_errlog7_offset);
		node_idx++;
	}
	/* Dump Bus Node Number Index. */
	pt_dump->noc_bus_node_idx = node_idx;

	return 0;

}

static int noc_dump(void *dump_addr, unsigned int size)
{
	unsigned int i;
	struct noc_dump_data *pt_dump = kzalloc(sizeof(*pt_dump), GFP_KERNEL);
	uint ret_size = 0;
	unsigned int uPERI_INT0_STAT, uSCPERSTATUS6;
	int ret;

	if (!pt_dump) {
		pr_err("[%s]: pt_dump malloc error\n", __func__);
		return 0;
	}

	if (!dump_addr) {
		pr_err("[%s]: dump_addr error\n", __func__);
		kfree(pt_dump);
		return 0;
	}

	pr_info("[%s]:addr=0x%pK,size=[0x%x/0x%x]", __func__,
		dump_addr, (unsigned int)sizeof(*pt_dump), size);

	for (i = 0; i < (NOC_DUMP_SYNC_LEN - 2); i++) {
		/* sync head 1 */
		pt_dump->sync[i] = NOC_DUMP_SYNC_HEAD1;
	}

	/* save max number of register that noc dump in ap */
	pt_dump->sync[i++] = NOC_DUMP_MAX_REG_NUM;

	/* sync head 2 */
	pt_dump->sync[i] = ((NOC_DUMP_SYNC_HEAD2 & 0xFFFFFF00) |
			    (g_noc_dump_info.noc_device_ptr->noc_property->platform_id & 0xFF));


	uPERI_INT0_STAT = readl_relaxed((u8 __iomem *) g_noc_dump_info.noc_device_ptr->pmctrl_base +
					g_noc_dump_info.noc_device_ptr->preg_list->pmctrl_int0_stat_offset);
	uSCPERSTATUS6 = readl_relaxed((u8 __iomem *) g_noc_dump_info.noc_device_ptr->sctrl_base +
				      g_noc_dump_info.noc_device_ptr->preg_list->sctrl_scperstatus6_offset);

	if (IS_NOC_TMOUT_HAPPEN(uPERI_INT0_STAT, uSCPERSTATUS6))
		/* Timeout happened , do not dump error probe register */
		goto ERR_PROBE_DUMP_END;

	ret = noc_dump_reg_data(pt_dump);
	if (ret == -1) {
		pr_err("[%s:%d]noc_dump_reg fail\n", __func__, __LINE__);
		goto malloc_free;
	}

ERR_PROBE_DUMP_END:
	for (i = 0; i < NOC_DUMP_NOC_LEN; i++)
		pt_dump->tail[i] = NOC_DUMP_TAIL_NR;	/* tail */

	if (sizeof(*pt_dump) >= size) {
		pr_err("NoC Dump Overflow. Dump Size[%lx], Alloc Size[%d].\n", sizeof(*pt_dump), size);
		ret_size = size;
	} else {
		ret_size = sizeof(*pt_dump);
	}

	/* COPY data to RDR module */
	ret = memcpy_s(dump_addr, size, pt_dump, (unsigned long)ret_size);
	if (ret != EOK)
		pr_err("[%s]: copy to dump_addr failed\n", __func__);

malloc_free:
	/* kmalloc free */
	kfree(pt_dump);
	noc_dump_prt();
	return (int)ret_size;
}

/*
 * Function: noc_dump_init
 * Description: noc dump function registers to black-box module, so noc dump can be processed before reset
 * input: none
 * output: none
 * return: 0->success
 */
int noc_dump_init(void)
{
	int ret = -1;
	unsigned int i;

	/* Get Hisi NoC Device Stucture Pointer. */
	g_noc_dump_info.noc_device_ptr = platform_get_drvdata(noc_get_pt_pdev());

	/* Get Hisi NoC Bus Node Stucture Pointer and Number. */
	noc_get_bus_nod_info((void **)&g_noc_dump_info.noc_bus_node_info.noc_node_pptr,
			     &g_noc_dump_info.noc_bus_node_info.noc_node_idx);

	if ((g_noc_dump_info.noc_bus_node_info.noc_node_pptr == NULL) ||
	    (g_noc_dump_info.noc_bus_node_info.noc_node_idx == 0) || (g_noc_dump_info.noc_device_ptr == NULL)) {
		pr_err("[%s] No Bus Node Registered. ", __func__);

		return ret;
	}

	/* Init noc_dump_reg_list Array. */
	for (i = 0; i < hisi_noc_get_dump_reg_list_num(); i++) {
		if (noc_dump_reg_list[i].name == NULL)
			break;
		if (!strncmp(noc_dump_reg_list[i].name, "PCTRL", sizeof("PCTRL")))
			noc_dump_reg_list[i].addr = g_noc_dump_info.noc_device_ptr->pctrl_base;
		else if (!strncmp(noc_dump_reg_list[i].name, "SCTRL", sizeof("SCTRL")))
			noc_dump_reg_list[i].addr = g_noc_dump_info.noc_device_ptr->sctrl_base;
		else if (!strncmp(noc_dump_reg_list[i].name, "PMCTRL", sizeof("PMCTRL")))
			noc_dump_reg_list[i].addr = g_noc_dump_info.noc_device_ptr->pmctrl_base;
		else if (!strncmp(noc_dump_reg_list[i].name, "CRGPERIPH", sizeof("CRGPERIPH")))
			noc_dump_reg_list[i].addr = g_noc_dump_info.noc_device_ptr->pcrgctrl_base;
		else
			pr_err("[%s] noc_dump_reg_list Info Error!\n", __func__);
	}

	/* Debug Print. */
	if (g_noc_dump_info.noc_device_ptr->noc_property->noc_debug) {
		pr_err("[%s] NoC Bus Dump Info:\n", __func__);
		pr_err("noc_device_ptr=%lx\n", (uintptr_t) g_noc_dump_info.noc_device_ptr);
		pr_err("noc_node_ptr=%lx\n", (uintptr_t) g_noc_dump_info.noc_bus_node_info.noc_node_pptr);
		pr_err("noc_node_idx=%d\n", g_noc_dump_info.noc_bus_node_info.noc_node_idx);
	}
#ifdef CONFIG_HISI_BB
	/* function "register_module_dump_mem_func" can only be used when CONFIG_HISI_BB */
	ret = register_module_dump_mem_func(noc_dump, "NOC", MODU_NOC);
#endif

	return ret;
}

/*END OF FILE*/
