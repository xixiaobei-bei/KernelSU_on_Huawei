/*
 * pcie-kport-idle.c
 *
 * PCIe Idle Feature
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#include <linux/of_irq.h>
#include <linux/hisi/hisi_pcie_idle_sleep.h>
#include "pcie-kport.h"
#include "pcie-kport-idle.h"

#define  PCIE_SUPPORT_IDLE_FEATURE	"support_32k_idle"
#define  PCIE_IDLE_TIMEOUT_IRQ		"pcie_idle_timeout"
#define  PCIE_IDLE_WAKE_IRQ		"pcie_idle_wake"

#define IRQ_IDLE_NUMS		2
#define IRQ_IDLE_TIMEOUT	0
#define IRQ_IDLE_WAKE		1

static inline void idle_reg_writel(struct pcie_idle_sleep *idle_sleep, u32 val, u32 reg)
{
	writel(val, idle_sleep->idle_reg_base + reg);
}

static inline uint32_t idle_reg_readl(struct pcie_idle_sleep *idle_sleep, u32 reg)
{
	return readl(idle_sleep->idle_reg_base + reg);
}

static struct pcie_idle_sleep *get_idle_sleep_by_host_id(uint32_t host_id)
{
	struct pcie_kport *pcie = get_pcie_by_id(host_id);

	if (pcie)
		return pcie->idle_sleep;

	return NULL;
}

static void pcie_refclk_on(struct pcie_idle_sleep *idle_sleep)
{
	struct pcie_kport *pcie = NULL;

	if (atomic_cmpxchg(&(idle_sleep->ref_clk_onoff), 0, 1))
		return;

	pcie = get_pcie_by_id(idle_sleep->host_id);
	if (!pcie)
		return;

	if (pcie->plat_ops && pcie->plat_ops->ref_clk_on) {
		PCIE_PR_I("Turn on ref_clk");
		pcie->plat_ops->ref_clk_on(pcie);
	}
}

static void pcie_refclk_off(struct pcie_idle_sleep *idle_sleep)
{
	struct pcie_kport *pcie = NULL;

	if (!atomic_cmpxchg(&(idle_sleep->ref_clk_onoff), 1, 0))
		return;

	pcie = get_pcie_by_id(idle_sleep->host_id);
	if (!pcie)
		return;

	if (pcie->plat_ops && pcie->plat_ops->ref_clk_off) {
		PCIE_PR_I("Turn off ref_clk");
		pcie->plat_ops->ref_clk_off(pcie);
	}
}

static void pcie_refclk_vote(struct pcie_idle_sleep *idle_sleep, u32 vote)
{
	unsigned long flags;

	if (unlikely(!idle_sleep->idle_sleep_enable))
		return;

	spin_lock_irqsave(&(idle_sleep->idle_sleep_lock), flags);
	if (vote) {
		if (!atomic_read(&(idle_sleep->ref_clk_vote)))
			pcie_refclk_on(idle_sleep);
		atomic_inc(&(idle_sleep->ref_clk_vote));
	} else {
		if (!atomic_read(&(idle_sleep->ref_clk_vote))) {
			/* should NOT run to this flow */
			PCIE_PR_E("pcie ref clk vote already 0!");
			goto SPIN_UNLOCK;
		}
		atomic_dec(&(idle_sleep->ref_clk_vote));
	}
SPIN_UNLOCK:
	spin_unlock_irqrestore(&(idle_sleep->idle_sleep_lock), flags);
}

void pcie_refclk_host_vote(struct pcie_port *pp, u32 vote)
{
	struct dw_pcie *pci = NULL;
	struct pcie_kport *pcie = NULL;

	if (unlikely(!pp))
		return;

	pci = to_dw_pcie_from_pp(pp);
	pcie = to_pcie_port(pci);

	if (pcie->idle_sleep)
		pcie_refclk_vote(pcie->idle_sleep, vote);
}
EXPORT_SYMBOL_GPL(pcie_refclk_host_vote);

void pcie_kport_refclk_device_vote(u32 ep_type, u32 rc_id, u32 vote)
{
	struct pcie_idle_sleep *idle_sleep = NULL;
	unsigned long flags;

	if (ep_type >= PCIE_DEVICE_MAX) {
		PCIE_PR_E("Invalid ep_type %u", ep_type);
		return;
	}

	idle_sleep = get_idle_sleep_by_host_id(rc_id);
	if (!idle_sleep || unlikely(!idle_sleep->idle_sleep_enable))
		return;

	PCIE_PR_I("%s: %u rc_id: %u ep_type: %u",
			__func__, vote, rc_id, ep_type);

	spin_lock_irqsave(&(idle_sleep->idle_sleep_lock), flags);
	if (vote) {
		atomic_set(&(idle_sleep->ref_clk_device_vote), 1);
		pcie_refclk_on(idle_sleep);
	} else {
		atomic_set(&(idle_sleep->ref_clk_device_vote), 0);
	}
	spin_unlock_irqrestore(&(idle_sleep->idle_sleep_lock), flags);
}
EXPORT_SYMBOL_GPL(pcie_kport_refclk_device_vote);

static irqreturn_t pcie_idle_timeout_irq_handler(int irq, void *arg)
{
	struct pcie_idle_sleep *idle_sleep = (struct pcie_idle_sleep *)arg;
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&(idle_sleep->idle_sleep_lock), flags);

	if (!is_pcie_pwr_on(idle_sleep->host_id))
		goto PWR_OFF;

	if (!atomic_read(&(idle_sleep->ref_clk_vote)) &&
	    !atomic_read(&(idle_sleep->ref_clk_device_vote))) {
		PCIE_PR_I("Allow idle sleep! Ref clk will off");
		pcie_refclk_off(idle_sleep);
	}

	/* clear pcie idle timeout int */
	val = idle_reg_readl(idle_sleep, idle_sleep->idle_timeout_clr_addr);
	val |= BIT(idle_sleep->idle_timeout_clr_bit);
	idle_reg_writel(idle_sleep, val, idle_sleep->idle_timeout_clr_addr);

	/* clear pcie idle wake int */
	val = idle_reg_readl(idle_sleep, idle_sleep->idle_wake_clr_addr);
	val |= BIT(idle_sleep->idle_wake_clr_bit);
	idle_reg_writel(idle_sleep, val, idle_sleep->idle_wake_clr_addr);

	/* clear pcie l1ss wake int mask */
	val = idle_reg_readl(idle_sleep, idle_sleep->idle_wake_int_addr);
	val &= ~(BIT(idle_sleep->idle_wake_mask_bit));
	idle_reg_writel(idle_sleep, val, idle_sleep->idle_wake_int_addr);

PWR_OFF:
	spin_unlock_irqrestore(&(idle_sleep->idle_sleep_lock), flags);

	return IRQ_HANDLED;
}

static irqreturn_t pcie_idle_wake_irq_handler(int irq, void *arg)
{
	struct pcie_idle_sleep *idle_sleep = (struct pcie_idle_sleep *)arg;
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&(idle_sleep->idle_sleep_lock), flags);

	if (!is_pcie_pwr_on(idle_sleep->host_id))
		goto PWR_OFF;

	/* clear pcie idle wake int */
	val = idle_reg_readl(idle_sleep, idle_sleep->idle_wake_clr_addr);
	val |= BIT(idle_sleep->idle_wake_clr_bit);
	idle_reg_writel(idle_sleep, val, idle_sleep->idle_wake_clr_addr);

	/* add pcie l1ss wake int mask */
	val = idle_reg_readl(idle_sleep, idle_sleep->idle_wake_int_addr);
	val |= BIT(idle_sleep->idle_wake_mask_bit);
	idle_reg_writel(idle_sleep, val, idle_sleep->idle_wake_int_addr);
	pcie_refclk_on(idle_sleep);

PWR_OFF:
	spin_unlock_irqrestore(&(idle_sleep->idle_sleep_lock), flags);

	return IRQ_HANDLED;
}

static int pcie_get_idle_irq(struct pcie_idle_sleep *idle_sleep,
			struct device_node *np)
{
	int irq;

	irq = of_irq_get_byname(np, PCIE_IDLE_TIMEOUT_IRQ);
	if (irq < 0) {
		PCIE_PR_E("Failed to get [%s] irq ", PCIE_IDLE_TIMEOUT_IRQ);
		return -ENODEV;
	}
	idle_sleep->irq[IRQ_IDLE_TIMEOUT] = (unsigned int)irq;

	irq = of_irq_get_byname(np, PCIE_IDLE_WAKE_IRQ);
	if (irq < 0) {
		PCIE_PR_E("Failed to get [%s] irq ", PCIE_IDLE_WAKE_IRQ);
		return -ENODEV;
	}
	idle_sleep->irq[IRQ_IDLE_WAKE] = (unsigned int)irq;

	PCIE_PR_I("irq nums, idle_timeout:%u, idle_wake:%u",
		idle_sleep->irq[IRQ_IDLE_TIMEOUT],
		idle_sleep->irq[IRQ_IDLE_WAKE]);

	return 0;
}

static int32_t pcie_register_idle_irq(struct pcie_idle_sleep *idle_sleep,
			struct device *dev)
{
	int32_t ret;

	ret = devm_request_irq(dev,
			idle_sleep->irq[IRQ_IDLE_TIMEOUT],
			pcie_idle_timeout_irq_handler,
			IRQF_TRIGGER_RISING,
			PCIE_IDLE_TIMEOUT_IRQ,
			idle_sleep);
	if (ret) {
		PCIE_PR_E("Register idle timeout irq");
		return ret;
	}

	ret = devm_request_irq(dev,
			idle_sleep->irq[IRQ_IDLE_WAKE],
			pcie_idle_wake_irq_handler,
			IRQF_TRIGGER_RISING,
			PCIE_IDLE_WAKE_IRQ,
			idle_sleep);
	if (ret) {
		PCIE_PR_E("Register idle wake irq");
		return ret;
	}

	return 0;
}

static int32_t get_pcie_idle_sleep_dtsinfo(struct pcie_idle_sleep *idle_sleep,
			struct device_node *np)
{
	u32 val[3] = {0}; /* get three offset from dts info */

	idle_sleep->idle_sleep_enable = 1;

	/* get dts info : enable bit, mask bit, reg_addr */
	if (of_property_read_u32_array(np, "idle_timeout_en", val, 3)) {
		PCIE_PR_E("idle timeout info");
		return -1;
	}
	idle_sleep->idle_timeout_en_bit = val[0];
	idle_sleep->idle_timeout_mask_bit = val[1];
	idle_sleep->idle_timeout_int_addr = val[2];

	/* get dts info : clear  bit, reg_addr */
	if (of_property_read_u32_array(np, "idle_timeout_clr", val, 2)) {
		PCIE_PR_E("idle timeout clear info");
		return -1;
	}
	idle_sleep->idle_timeout_clr_bit = val[0];
	idle_sleep->idle_timeout_clr_addr = val[1];

	/* get dts info : enable bit, mask bit, reg_addr */
	if (of_property_read_u32_array(np, "idle_wake_en", val, 3)) {
		PCIE_PR_E("idle wake info");
		return -1;
	}
	idle_sleep->idle_wake_en_bit = val[0];
	idle_sleep->idle_wake_mask_bit = val[1];
	idle_sleep->idle_wake_int_addr = val[2];

	/* get dts info : clear  bit, reg_addr */
	if (of_property_read_u32_array(np, "idle_wake_clr", val, 2)) {
		PCIE_PR_E("idle wake clear info");
		return -1;
	}
	idle_sleep->idle_wake_clr_bit = val[0];
	idle_sleep->idle_wake_clr_addr = val[1];

	/*
	 * get dts info :
	 *    l1ss cnt set val(if 0, then use default), l1ss cnt reg addr
	 */
	if (of_property_read_u32_array(np, "l1ss_cnt_base", val, 2)) {
		PCIE_PR_E("l1ss cnt info");
		return -1;
	}
	idle_sleep->l1ss_cnt_base = val[0];
	idle_sleep->l1ss_cnt_base_addr = val[1];

	return 0;
}

static void pcie_idle_sleep_init(struct pcie_idle_sleep *idle_sleep)
{
	u32 val;

	PCIE_PR_I("+%s+", __func__);

	if (!idle_sleep->idle_sleep_enable)
		goto SKIP_IDLE_SLEEP_INIT;

	/* enable pcie idle timeout int */
	val = idle_reg_readl(idle_sleep, idle_sleep->idle_timeout_int_addr);
	val &= ~(BIT(idle_sleep->idle_timeout_mask_bit));
	val |= BIT(idle_sleep->idle_timeout_en_bit);
	idle_reg_writel(idle_sleep, val, idle_sleep->idle_timeout_int_addr);

	/* enable pcie idle wake int & mask */
	val = idle_reg_readl(idle_sleep, idle_sleep->idle_wake_int_addr);
	val |= BIT(idle_sleep->idle_wake_en_bit);
	val |= BIT(idle_sleep->idle_wake_mask_bit);
	idle_reg_writel(idle_sleep, val, idle_sleep->idle_wake_int_addr);

	/* init pcie l1ss base cnt */
	if (idle_sleep->l1ss_cnt_base)
		idle_reg_writel(idle_sleep,
				idle_sleep->l1ss_cnt_base,
				idle_sleep->idle_wake_int_addr);
SKIP_IDLE_SLEEP_INIT:
	/* set refclk state to 1 */
	atomic_set(&(idle_sleep->ref_clk_onoff), 1);

	PCIE_PR_I("idle sleep enable state %u", idle_sleep->idle_sleep_enable);
	PCIE_PR_I("idle timeout and wake int state[0x%x]: 0x%x",
		idle_sleep->idle_timeout_int_addr,
		idle_reg_readl(idle_sleep, idle_sleep->idle_timeout_int_addr));
	PCIE_PR_I("L1ss cnt base[0x%x]: 0x%x", idle_sleep->l1ss_cnt_base_addr,
		idle_reg_readl(idle_sleep, idle_sleep->l1ss_cnt_base_addr));

	PCIE_PR_I("-%s-", __func__);
}

static void pcie_idle_sleep_deinit(struct pcie_idle_sleep *idle_sleep)
{
	u32 val;

	PCIE_PR_I("+%s+", __func__);

	if (!idle_sleep->idle_sleep_enable)
		goto SKIP_IDLE_SLEEP_DEINIT;

	/* clear pcie idle timeout int */
	val = idle_reg_readl(idle_sleep, idle_sleep->idle_timeout_clr_addr);
	val |= BIT(idle_sleep->idle_timeout_clr_bit);
	idle_reg_writel(idle_sleep, val, idle_sleep->idle_timeout_clr_addr);

	/* clear pcie idle wake int */
	val = idle_reg_readl(idle_sleep, idle_sleep->idle_wake_clr_addr);
	val |= BIT(idle_sleep->idle_wake_clr_bit);
	idle_reg_writel(idle_sleep, val, idle_sleep->idle_wake_clr_addr);

	/* PCIe L1ss idle wake interrupts mask */
	val = idle_reg_readl(idle_sleep, idle_sleep->idle_wake_int_addr);
	val |= BIT(idle_sleep->idle_wake_mask_bit);
	idle_reg_writel(idle_sleep, val, idle_sleep->idle_wake_int_addr);

	/* PCIe L1ss idle timeout interrupts mask */
	val = idle_reg_readl(idle_sleep, idle_sleep->idle_timeout_int_addr);
	val |= BIT(idle_sleep->idle_timeout_mask_bit);
	idle_reg_writel(idle_sleep, val, idle_sleep->idle_timeout_int_addr);

SKIP_IDLE_SLEEP_DEINIT:
	/* clear refclk vote state */
	atomic_set(&(idle_sleep->ref_clk_onoff), 0);
	atomic_set(&(idle_sleep->ref_clk_vote), 0);

	PCIE_PR_I("idle timeout and wake int state[0x%x]: 0x%x",
		idle_sleep->idle_timeout_int_addr,
		idle_reg_readl(idle_sleep, idle_sleep->idle_timeout_int_addr));

	PCIE_PR_I("-%s-", __func__);
}

static inline void register_init_ops(struct pcie_idle_sleep *idle_sleep)
{
	idle_sleep->init = pcie_idle_sleep_init;
	idle_sleep->deinit = pcie_idle_sleep_deinit;
}

static void pcie_idle_sleep_bind_host(struct pcie_kport *pcie,
			struct pcie_idle_sleep *idle_sleep)
{
	register_init_ops(idle_sleep);

	pcie->idle_sleep = idle_sleep;
	idle_sleep->host_id = pcie->rc_id;
	idle_sleep->idle_reg_base = pcie->apb_base;
}

/*
 * Vote default: ready for 32K ?
 * device: No
 * host: Yes
 */
static inline void pcie_idle_vote_init(struct pcie_idle_sleep *idle_sleep)
{
	if (idle_sleep)
		atomic_set(&(idle_sleep->ref_clk_device_vote), 1);
}

int32_t register_pcie_idle_hook(struct platform_device *pdev)
{
	int32_t ret;
	struct pcie_kport *pcie = platform_get_drvdata(pdev);
	struct device *dev = &(pdev->dev);
	struct device_node *np = NULL;
	struct pcie_idle_sleep *idle_sleep = NULL;

	PCIE_PR_I("+%s+", __func__);

	if (!pcie || !dev) {
		PCIE_PR_I("Null platform");
		return 0;
	}

	np = dev->of_node;
	if (!np) {
		PCIE_PR_I("Null np");
		return 0;
	}

	if (!of_find_property(np, PCIE_SUPPORT_IDLE_FEATURE, NULL)) {
		PCIE_PR_I("Not Support 32K idle");
		return 0;
	}

	idle_sleep = devm_kzalloc(dev, sizeof(struct pcie_idle_sleep), GFP_KERNEL);
	if (!idle_sleep) {
		PCIE_PR_E("Alloc idle info");
		return -ENOMEM;
	}

	ret = get_pcie_idle_sleep_dtsinfo(idle_sleep, np);
	if (ret) {
		devm_kfree(dev, idle_sleep);
		return ret;
	}

	pcie_idle_sleep_bind_host(pcie, idle_sleep);
	spin_lock_init(&(idle_sleep->idle_sleep_lock));
	pcie_idle_vote_init(idle_sleep);

	ret = pcie_get_idle_irq(idle_sleep, np);
	if (ret) {
		devm_kfree(dev, idle_sleep);
		return ret;
	}

	ret = pcie_register_idle_irq(idle_sleep, dev);
	if (ret) {
		devm_kfree(dev, idle_sleep);
		return ret;
	}

	PCIE_PR_I("-%s-", __func__);
	return 0;
}

void pcie_dump_ilde_hw_stat(u32 rc_id)
{
	struct pcie_idle_sleep *idle_sleep = get_idle_sleep_by_host_id(rc_id);
	struct pcie_kport *pcie = get_pcie_by_id(rc_id);

	if (!idle_sleep || !pcie) {
		PCIE_PR_E("Invalid rc_id");
		return;
	}

	if (!is_pcie_pwr_on(rc_id))
		return;

	/* PLL Gate Status Reg Val */
	if (pcie->plat_ops->pll_status)
		pcie->plat_ops->pll_status(pcie);

	/* PCIECTRL state dump */
	pr_info("0x%-8x: %8x %8x %8x %8x\n",
		0x43C, /* pciectrl_stat15 */
		idle_reg_readl(idle_sleep, 0x43C),  /* pciectrl_stat15 */
		idle_reg_readl(idle_sleep, 0x440),  /* pciectrl_stat16 */
		idle_reg_readl(idle_sleep, 0x444),  /* pciectrl_stat17 */
		idle_reg_readl(idle_sleep, 0x448)); /* pciectrl_stat18 */
}

void pcie_dump_ilde_sw_stat(u32 rc_id)
{
	struct pcie_idle_sleep *idle_sleep = get_idle_sleep_by_host_id(rc_id);

	if (!idle_sleep) {
		PCIE_PR_E("Invalid rc_id");
		return;
	}

	/* idle sleep state dump */
	pr_info("[PCIe%u] Refclk onoff state  : %d\n",
		idle_sleep->host_id,
		atomic_read(&(idle_sleep->ref_clk_onoff)));
	pr_info("[PCIe%u] Refclk device vote  : %d\n",
		idle_sleep->host_id,
		atomic_read(&(idle_sleep->ref_clk_device_vote)));
	pr_info("[PCIe%u] Refclk other vote   : %d\n",
		idle_sleep->host_id,
		atomic_read(&(idle_sleep->ref_clk_vote)));
}

#ifdef CONFIG_PCIE_KPORT_TEST
void pcie_refclk_vote_test(u32 rc_id, u32 vote)
{
	struct pcie_idle_sleep *idle_sleep = get_idle_sleep_by_host_id(rc_id);

	if (idle_sleep)
		pcie_refclk_vote(idle_sleep, vote);
}

void pcie_idle_sleep_enable_test(u32 rc_id, u32 endis)
{
	struct pcie_idle_sleep *idle_sleep = get_idle_sleep_by_host_id(rc_id);

	if (idle_sleep)
		idle_sleep->idle_sleep_enable = (!!endis);
}
#endif
