/*
 * NoC. (NoC Mntn Module.)
 *
 * Copyright (c) 2016 Huawei Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/io.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/hisi/hisi_log.h>
#define HISI_LOG_TAG HISI_NOC_TAG
#include "hisi_noc_transcation.h"
#include "hisi_noc_packet.h"
#include "securec.h"

void init_transcation_info(struct noc_node *node)
{
	if (!node) {
		pr_err("%s node is null!\n", __func__);
		return;
	}

	node->tran_cfg.statalarmmax = TRANS_CFG_STATAALARMMAX;
	node->tran_cfg.statperiod = TRANS_CFG_STATPERIOD;
	node->tran_cfg.trans_f_mode = TRANS_CFG_TRANS_F_MODE;
	node->tran_cfg.trans_f_addrbase_low = TRANS_CFG_TRANS_F_ADDRBASE_LOW;
	node->tran_cfg.trans_f_addrwindowsize = TRANS_CFG_TRANS_F_ADDRWINSIZE;
	node->tran_cfg.trans_f_opcode = TRANS_CFG_TRANS_F_OPCODE;
	node->tran_cfg.trans_f_usermask = TRANS_CFG_TRANS_F_USERMASK;
	node->tran_cfg.trans_f_securitymask = TRANS_CFG_TRANS_F_SECMASK;
	node->tran_cfg.trans_p_mode = TRANS_CFG_TRANS_P_MODE;
	node->tran_cfg.trans_p_thresholds_0_0 = TRANS_CFG_TRANS_P_THRESHOLD_0;
	node->tran_cfg.trans_p_thresholds_0_1 = TRANS_CFG_TRANS_P_THRESHOLD_1;
	node->tran_cfg.trans_p_thresholds_0_2 = TRANS_CFG_TRANS_P_THRESHOLD_2;
	node->tran_cfg.trans_p_thresholds_0_3 = TRANS_CFG_TRANS_P_THRESHOLD_3;
	node->tran_cfg.trans_m_counters_0_src = TRANS_CFG_TRANS_M_CNT_0_SRC;
	node->tran_cfg.trans_m_counters_1_src = TRANS_CFG_TRANS_M_CNT_1_SRC;
	node->tran_cfg.trans_m_counters_2_src = TRANS_CFG_TRANS_M_CNT_2_SRC;
	node->tran_cfg.trans_m_counters_3_src = TRANS_CFG_TRANS_M_CNT_3_SRC;
	node->tran_cfg.trans_m_counters_0_alarmmode = TRANS_CFG_TRANS_M_CNT_0_ALARMM;
	node->tran_cfg.trans_m_counters_1_alarmmode = TRANS_CFG_TRANS_M_CNT_1_ALARMM;
	node->tran_cfg.trans_m_counters_2_alarmmode = TRANS_CFG_TRANS_M_CNT_2_ALARMM;
	node->tran_cfg.trans_m_counters_3_alarmmode = TRANS_CFG_TRANS_M_CNT_3_ALARMM;

}

void enable_transcation_probe(const struct noc_node *node, void __iomem *base)
{
	unsigned int temp;

	if (!node) {
		pr_err("%s node is null!\n", __func__);
		return;
	}

	if (!base) {
		pr_err("%s base is null!\n", __func__);
		return;
	}

	/* Disable Transaction Probe */
	noc_clear_bit((char *)base, TRANS_M_CFGCTL, TRANS_M_CFGCTL_INIT_VALUE);
	noc_clear_bit((char *)base, TRANS_M_MAINCTL, TRANS_M_MAINCTL_INIT_VALUE);
	noc_set_bit((char *)base, TRANS_M_MAINCTL, TRANS_M_MAINCTL_SET_EN_VALUE);

	/* config Filter mode */
	writel_relaxed(node->tran_cfg.trans_f_mode, (char *)base + TRANS_F_MODE);

	/* config Filter */
	writel_relaxed(node->tran_cfg.trans_f_addrbase_low, (char *)base + TRANS_F_ADDRBASE_LOW);
	writel_relaxed(node->tran_cfg.trans_f_addrwindowsize, (char *)base + TRANS_F_ADDRWINDOWSIZE);
	writel_relaxed(node->tran_cfg.trans_f_opcode, (char *)base + TRANS_F_OPCODE);
	writel_relaxed(node->tran_cfg.trans_f_usermask, (char *)base + TRANS_F_USERMASK);
	writel_relaxed(node->tran_cfg.trans_f_securitymask, (char *)base + TRANS_F_SECURITYMASK);

	/* enable Transaction Probe */
	writel_relaxed(TRANS_ENABLE, (char *)base + TRANS_P_EN);

	/* config Transaction Probe Mode */
	writel_relaxed(node->tran_cfg.trans_p_mode, (char *)base + TRANS_P_MODE);

	/* config Transaction Probe Bins */
	writel_relaxed(node->tran_cfg.trans_p_thresholds_0_0, (char *)base + TRANS_P_THRESHOLDS_0_0);
	writel_relaxed(node->tran_cfg.trans_p_thresholds_0_1, (char *)base + TRANS_P_THRESHOLDS_0_1);
	writel_relaxed(node->tran_cfg.trans_p_thresholds_0_2, (char *)base + TRANS_P_THRESHOLDS_0_2);
	writel_relaxed(node->tran_cfg.trans_p_thresholds_0_3, (char *)base + TRANS_P_THRESHOLDS_0_3);

	/* config Transaction Probe Bins Source */
	writel_relaxed(node->tran_cfg.trans_m_counters_0_src, (char *)base + TRANS_M_COUNTERS_0_SRC);
	writel_relaxed(node->tran_cfg.trans_m_counters_1_src, (char *)base + TRANS_M_COUNTERS_1_SRC);
	writel_relaxed(node->tran_cfg.trans_m_counters_2_src, (char *)base + TRANS_M_COUNTERS_2_SRC);
	writel_relaxed(node->tran_cfg.trans_m_counters_3_src, (char *)base + TRANS_M_COUNTERS_3_SRC);

	/* config  Statisticscycle period */
	writel_relaxed(node->tran_cfg.statperiod, (char *)base + TRANS_M_STATPERIOD);

	/* config alarm mode */
	writel_relaxed(node->tran_cfg.trans_m_counters_0_alarmmode, (char *)base + TRANS_M_COUNTERS_0_ALARMMODE);
	writel_relaxed(node->tran_cfg.trans_m_counters_1_alarmmode, (char *)base + TRANS_M_COUNTERS_1_ALARMMODE);
	writel_relaxed(node->tran_cfg.trans_m_counters_2_alarmmode, (char *)base + TRANS_M_COUNTERS_2_ALARMMODE);
	writel_relaxed(node->tran_cfg.trans_m_counters_3_alarmmode, (char *)base + TRANS_M_COUNTERS_3_ALARMMODE);

	/* config Statisticscycle alarm MAX */
	writel_relaxed(node->tran_cfg.statalarmmax, (char *)base + TRANS_M_STATALARMMAX);

	/* enable alarm interrupt */
	noc_set_bit((char *)base, TRANS_M_MAINCTL, TRANS_ENABLE_ALARM_INT);

	temp = (u32) readl_relaxed((char *)base + TRANS_M_MAINCTL);
	pr_info("base + TRANS_M_MAINCTL is %d\n", temp);

	/* enable Transaction Probe */
	noc_set_bit((char *)base, TRANS_M_CFGCTL, TRANS_ENABLE_PROBE);

	temp = (u32) readl_relaxed((char *)base + TRANS_M_CFGCTL);
	pr_info("base + TRANS_M_CFGCTL is %d\n", temp);

	wmb();/* drain writebuffer */
}

void disable_transcation_probe(void __iomem *base)
{
	if (!base) {
		pr_err("%s base is null!\n", __func__);
		return;
	}

	noc_clear_bit((char *)base, TRANS_M_CFGCTL, TRANS_M_CFGCTL_INIT_VALUE);
	noc_clear_bit((char *)base, TRANS_M_MAINCTL, TRANS_M_MAINCTL_INIT_VALUE);
	noc_set_bit((char *)base, TRANS_M_MAINCTL, TRANS_M_MAINCTL_SET_VALUE);

	wmb();/* drain writebuffer */
}

void noc_transcation_probe_hanlder(const struct noc_node *node, void __iomem *base, unsigned int idx)
{
	unsigned int val;

	pr_err("noc_transaction_probe_hanlder +++++++++++++++++++++%d\n", idx);
	val = (u32) readl_relaxed((char *)base + TRANS_M_COUNTERS_0_VAL);
	pr_err("the TRANS_M_COUNTERS_0_VAL is 0x%x\n", val);

	val = (u32) readl_relaxed((char *)base + TRANS_M_COUNTERS_1_VAL);
	pr_err("the TRANS_M_COUNTERS_1_VAL is 0x%x\n", val);

	val = (u32) readl_relaxed((char *)base + TRANS_M_COUNTERS_2_VAL);
	pr_err("the TRANS_M_COUNTERS_2_VAL is 0x%x\n", val);

	val = (u32) readl_relaxed((char *)base + TRANS_M_COUNTERS_3_VAL);
	pr_err("the TRANS_M_COUNTERS_3_VAL is 0x%x\n", val);

	val = (u32) readl_relaxed((char *)base + TRANS_P_OVERFLOWSTATUS);

	if (val == TRANS_P_OVERFLOWSTATUS_VALUE)
		writel_relaxed(TRANS_P_OVERFLOWSTATUS_VALUE, (char *)base + TRANS_P_OVERFLOWRESET);

	writel_relaxed(TRANS_M_STATALARMCLR_VALUE, (char *)base + TRANS_M_STATALARMCLR);

	wmb();/* drain writebuffer */

	disable_transcation_probe(base);

	enable_transcation_probe(node, base);
}

void enable_transcation_probe_by_name(const char *name)
{
	struct noc_node *node = NULL;
	void __iomem *base = NULL;

	if (!name) {
		pr_err("%s name is null!\n", __func__);
		return;
	}

	base = get_errprobe_base(name);
	if (base == NULL) {
		pr_err("%s cannot get the node!\n", __func__);
		return;
	}

	node = get_probe_node(name);
	if (node == NULL) {
		pr_err("%s cannot get the node!\n", __func__);
		return;
	}

	enable_transcation_probe(node, base);
}
EXPORT_SYMBOL(enable_transcation_probe_by_name);

void disable_transcation_probe_by_name(const char *name)
{
	void __iomem *base = NULL;

	if (!name) {
		pr_err("%s name is null!\n", __func__);
		return;
	}

	base = get_errprobe_base(name);
	if (base == NULL) {
		pr_err("%s cannot get the node!\n", __func__);
		return;
	}

	disable_transcation_probe(base);
}
EXPORT_SYMBOL(disable_transcation_probe_by_name);

void config_transcation_probe(const char *name, const struct transcation_configration *tran_cfg)
{
	struct noc_node *node = NULL;

	if (!name) {
		pr_err("%s name is null!\n", __func__);
		return;
	}

	if (!tran_cfg) {
		pr_err("%s name is null!\n", __func__);
		return;
	}

	node = get_probe_node(name);
	if (node == NULL) {
		pr_err("%s cannot get the node!\n", __func__);
		return;
	}

	if (memcpy_s(&(node->tran_cfg), sizeof(node->tran_cfg), tran_cfg, sizeof(node->tran_cfg)) != EOK)
		pr_err("[%s:%d]: memcpy_s err\n]", __func__, __LINE__);

}
EXPORT_SYMBOL(config_transcation_probe);
