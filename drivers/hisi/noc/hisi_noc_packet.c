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
#include "hisi_noc_packet.h"
#include "securec.h"

void noc_set_bit(void __iomem *base, unsigned int offset, unsigned int bit)
{
	unsigned int temp;

	if (!base)
		return;

	temp = (u32)readl_relaxed((char *)base + offset);

	temp = temp | (0x1 << bit);

	writel_relaxed(temp, (char *)base + offset);
}

void noc_clear_bit(void __iomem *base, unsigned int offset, unsigned int bit)
{
	unsigned int temp;

	if (!base)
		return;

	temp = (u32)readl_relaxed((char *)base + offset);

	temp = temp & (~(0x1 << bit));

	writel_relaxed(temp, (char *)base + offset);
}

void init_packet_info(struct noc_node *node)
{
	node->packet_cfg.statalarmmax                = PACKET_CFG_STATAALARMMAX;
	node->packet_cfg.packet_filterlut            = PACKET_CFG_PACKET_FILTERLUT;
	node->packet_cfg.packet_f0_addrbase          = PACKET_CFG_PACKET_F0_ADDRBASE;
	node->packet_cfg.packet_f0_windowsize        = PACKET_CFG_PACKET_F0_WINSIZE;
	node->packet_cfg.packet_f0_securitymask      = PACKET_CFG_PACKET_F0_SECMASK;
	node->packet_cfg.packet_f0_opcode            = PACKET_CFG_PACKET_F0_OPCODE;
	node->packet_cfg.packet_f0_status            = PACKET_CFG_PACKET_F0_STATUS;
	node->packet_cfg.packet_f0_length            = PACKET_CFG_PACKET_F0_LENGTH;
	node->packet_cfg.packet_f0_urgency           = PACKET_CFG_PACKET_F0_URGENCY;
	node->packet_cfg.packet_f0_usermask          = PACKET_CFG_PACKET_F0_USERMASK;
	node->packet_cfg.statperiod                  = PACKET_CFG_STATPERIOD;
	node->packet_cfg.packet_f0_routeidbase       = PACKET_CFG_PACKET_F0_ROUTEIDBASE;
	node->packet_cfg.packet_f0_routeidmask       = PACKET_CFG_PACKET_F0_ROUTEIDMASK;
	node->packet_cfg.packet_f1_routeidbase       = PACKET_CFG_PACKET_F1_ROUTEIDBASE;
	node->packet_cfg.packet_f1_routeidmask       = PACKET_CFG_PACKET_F1_ROUTEIDMASK;
	node->packet_cfg.packet_counters_0_src       = PACKET_CFG_PACKET_COUTNERS_0_SRC;
	node->packet_cfg.packet_f1_addrbase          = PACKET_CFG_PACKET_F1_ADDRBASE;
	node->packet_cfg.packet_f1_windowsize        = PACKET_CFG_PACKET_F1_WINSIZE;
	node->packet_cfg.packet_f1_securitymask      = PACKET_CFG_PACKET_F1_SECMASK;
	node->packet_cfg.packet_f1_opcode            = PACKET_CFG_PACKET_F1_OPCODE;
	node->packet_cfg.packet_f1_status            = PACKET_CFG_PACKET_F1_STATUS;
	node->packet_cfg.packet_f1_length            = PACKET_CFG_PACKET_F1_LENGTH;
	node->packet_cfg.packet_f1_urgency           = PACKET_CFG_PACKET_F1_URGENCY;
	node->packet_cfg.packet_f1_usermask          = PACKET_CFG_PACKET_F1_USERMASK;
	node->packet_cfg.packet_counters_1_src       = PACKET_CFG_PACKET_COUNTERS_1_SRC;
	node->packet_cfg.packet_counters_0_alarmmode = PACKET_CFG_PACKET_COUNTERS_0_ALARMMODE;
	node->packet_cfg.packet_counters_1_alarmmode = PACKET_CFG_PACKET_COUNTERS_1_ALARMMODE;
}

void enable_packet_probe(const struct noc_node *node, void __iomem *base)
{
	unsigned int temp;

	if ((!node) || (!base))
		return;

	/* Disable Packet Probe */
	noc_clear_bit(base, PACKET_CFGCTL, PACKET_CFGCTL_INIT_VALUE);
	noc_clear_bit(base, PACKET_MAINCTL, PACKET_MAINCTL_INIT_VALUE);
	noc_set_bit(base, PACKET_MAINCTL, PACKET_MAINCTL_SET_EN_VALUE);

	/* config packet source */
	writel_relaxed(node->packet_cfg.packet_counters_0_src, (char *)base + PACKET_COUNTERS_0_SRC);
	writel_relaxed(node->packet_cfg.packet_counters_1_src, (char *)base + PACKET_COUNTERS_1_SRC);

	/* config Statisticscycle*/
	writel_relaxed(node->packet_cfg.statperiod, (char *)base + PACKET_STATPERIOD);

	/* config counters alarmmode */
	writel_relaxed(node->packet_cfg.packet_counters_0_alarmmode, (char *)base + PACKET_COUNTERS_0_ALARMMODE);
	writel_relaxed(node->packet_cfg.packet_counters_1_alarmmode, (char *)base + PACKET_COUNTERS_1_ALARMMODE);

	writel_relaxed(node->packet_cfg.statalarmmax, (char *)base + PACKET_STATALARMMAX);

	/* config Filter */
	writel_relaxed(node->packet_cfg.packet_filterlut, (char *)base + PACKET_FILTERLUT);
	writel_relaxed(node->packet_cfg.packet_f0_routeidbase, (char *)base + PACKET_F0_ROUTEIDBASE);
	writel_relaxed(node->packet_cfg.packet_f0_routeidmask, (char *)base + PACKET_F0_ROUTEIDMASK);
	writel_relaxed(node->packet_cfg.packet_f0_addrbase, (char *)base + PACKET_F0_ADDRBASE);
	writel_relaxed(node->packet_cfg.packet_f0_windowsize, (char *)base + PACKET_F0_WINDOWSIZE);
	writel_relaxed(node->packet_cfg.packet_f0_securitymask, (char *)base + PACKET_F0_SECURITYMASK);
	writel_relaxed(node->packet_cfg.packet_f0_opcode, (char *)base + PACKET_F0_OPCODE);
	writel_relaxed(node->packet_cfg.packet_f0_status, (char *)base + PACKET_F0_STATUS);
	writel_relaxed(node->packet_cfg.packet_f0_length, (char *)base + PACKET_F0_LENGTH);
	writel_relaxed(node->packet_cfg.packet_f0_urgency, (char *)base + PACKET_F0_URGENCY);
	writel_relaxed(node->packet_cfg.packet_f0_usermask, (char *)base + PACKET_F0_USERMASK);

	writel_relaxed(node->packet_cfg.packet_f1_routeidbase, (char *)base + PACKET_F1_ROUTEIDBASE);
	writel_relaxed(node->packet_cfg.packet_f1_routeidmask, (char *)base + PACKET_F1_ROUTEIDMASK);
	writel_relaxed(node->packet_cfg.packet_f1_addrbase, (char *)base + PACKET_F1_ADDRBASE);
	writel_relaxed(node->packet_cfg.packet_f1_windowsize, (char *)base + PACKET_F1_WINDOWSIZE);
	writel_relaxed(node->packet_cfg.packet_f1_securitymask, (char *)base + PACKET_F1_SECURITYMASK);
	writel_relaxed(node->packet_cfg.packet_f1_opcode, (char *)base + PACKET_F1_OPCODE);
	writel_relaxed(node->packet_cfg.packet_f1_status, (char *)base + PACKET_F1_STATUS);
	writel_relaxed(node->packet_cfg.packet_f1_length, (char *)base + PACKET_F1_LENGTH);
	writel_relaxed(node->packet_cfg.packet_f1_urgency, (char *)base + PACKET_F1_URGENCY);
	writel_relaxed(node->packet_cfg.packet_f1_usermask, (char *)base + PACKET_F1_USERMASK);

	/* enable alarm interrupt */
	noc_set_bit(base, PACKET_MAINCTL, PACKET_ENABLE_ALARM_INT);

	temp = (u32)readl_relaxed((char *)base + PACKET_MAINCTL);
	pr_info("the PACKET_MAINCTL is 0x%x\n", temp);

	/* enable Packet Probe */
	noc_set_bit(base, PACKET_CFGCTL, PACKET_ENABLE);

	temp = (u32)readl_relaxed((char *)base + PACKET_CFGCTL);
	pr_info("the PACKET_CFGCTL is 0x%x\n", temp);

	wmb();/* drain writebuffer */
}

void disable_packet_probe(void __iomem *base)
{

	if (!base)
		return;

	noc_clear_bit(base, PACKET_CFGCTL, PACKET_CFGCTL_INIT_VALUE);
	noc_clear_bit(base, PACKET_MAINCTL, PACKET_MAINCTL_INIT_VALUE);
	noc_set_bit(base, PACKET_MAINCTL, PACKET_MAINCTL_SET_VALUE);

	wmb();/* drain writebuffer */
}

void noc_packet_probe_hanlder(const struct noc_node *node, void __iomem *base)
{
	unsigned int val;

	/* read packet counters value register */
	val = (unsigned int)readl_relaxed((char *)base + PACKET_COUNTERS_0_VAL);
	pr_err("%s +++++++++++++++++++++\n", __func__);
	pr_err("the PACKET_COUNTERS_0_VAL is 0x%x\n", val);

	val = (unsigned int)readl_relaxed((char *)base + PACKET_COUNTERS_1_VAL);
	pr_err("the PACKET_COUNTERS_1_VAL is 0x%x\n", val);
	pr_err("%s ---------------------\n", __func__);

	/* clear interrupt */
	writel_relaxed(PACKET_STATALARMCLR_VALUE, (char *)base + PACKET_STATALARMCLR);

	wmb();/* drain writebuffer */

	disable_packet_probe(base);

	enable_packet_probe(node, base);
}

void enable_packet_probe_by_name(const char *name)
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

	enable_packet_probe(node, base);
}
EXPORT_SYMBOL(enable_packet_probe_by_name);


void disable_packet_probe_by_name(const char *name)
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

	disable_packet_probe(base);
}
EXPORT_SYMBOL(disable_packet_probe_by_name);

void config_packet_probe(const char *name, const struct packet_configration *packet_cfg)
{
	struct noc_node *node = NULL;

	if (!name) {
		pr_err("%s name is null!\n", __func__);
		return;
	}

	if (!packet_cfg) {
		pr_err("%s packet_cfg is null!\n", __func__);
		return;
	}

	node = get_probe_node(name);
	if (node == NULL) {
		pr_err("%s cannot get the node!\n", __func__);
		return;
	}

	if (memcpy_s(&(node->packet_cfg), sizeof(node->packet_cfg), packet_cfg, sizeof(node->packet_cfg)) != EOK)
		pr_err("[%s:%d]: memcpy_s err\n]", __func__, __LINE__);

}
EXPORT_SYMBOL(config_packet_probe);
