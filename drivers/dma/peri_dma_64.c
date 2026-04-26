/*
 * Synopsys DesignWare DMA adapter driver.
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_dma.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <pr_log.h>
#include "virt-dma.h"

#define PR_LOG_TAG DMA64_TAG

#define DRIVER_NAME "peri-dma64"
#define DMA_ALIGN 3
#define DMA_MAX_SIZE 0x1ffc

#define INT_STAT 0x00
#define INT_TC1 0x04
#define INT_ERR1 0x0c
#define INT_ERR2 0x10
#define INT_TC1_MASK 0x18
#define INT_ERR1_MASK 0x20
#define INT_ERR2_MASK 0x24
#define INT_TC1_RAW 0x600
#define INT_TC2_RAW 0x608
#define INT_ERR1_RAW 0x610
#define INT_ERR2_RAW 0x618
#define CH_PRI 0x688
#define CH_STAT 0x690
#define DMA_CTRL 0x698
#define CX_CUR_CNT 0x404
#define CX_LLI_H 0x804
#define CX_LLI_L 0x800
#define CX_CNT 0x81C
#define CX_SRC_L 0x820
#define CX_SRC_H 0x824
#define CX_DST_L 0x828
#define CX_DST_H 0x82C
#define CX_CONFIG 0x830
#define AXI_CONFIG 0x834

#define CX_LLI_CHAIN_EN 0x2
#define CCFG_EN 0x1
#define CCFG_MEM2PER (0x1 << 2)
#define CCFG_PER2MEM (0x2 << 2)
#define CCFG_SRCINCR (0x1UL << 31)
#define CCFG_DSTINCR (0x1UL << 30)
#define DMA_CTRL_PIPE_LINE (0x1 << 8)
#define GET_HIGH_32BIT(a) ((u32)((u64)(a) >> 32))
#define GET_LOW_32BIT(a) ((u32)((u64)(a)&0xffffffff))
#define U32_TO_U64(u32h, u32l) ((u64)(u32l) | ((u64)(u32h) << 32))

#define DMA_CH_CFG_PERI 4
#define DMA_CH_CFG_DW   12
#define DMA_CH_CFG_SW   16
#define DMA_CH_CFG_DL   20
#define DMA_CH_CFG_SL   24
#define DMA_2BITS_MASK  0x3

#define DMA_MAX_BURST 16
#define DMA_WAIT_PHY_STATUS_TIMEOUT 2000

#define DMA_SLAVE_BUS_WIDTH_VAL_1BYTE 0
#define DMA_SLAVE_BUS_WIDTH_VAL_2BYTE 1
#define DMA_SLAVE_BUS_WIDTH_VAL_4BYTE 2
#define DMA_SLAVE_BUS_WIDTH_VAL_8BYTE 8

#define DMA_WAIT_CHANN_TIMEOUT 500
#define DMAC_REGISTER_FN_ID 0xc501de00
#define UNSUPPORT 0xFF
#define CURRENT_COUNT_OFFSET 0x20
#define DMA_CHAN_OFFSET 0x40

struct peri_dma_desc_hw {
	u64 lli;
	u32 reserved1[5];
	u32 count;
	u64 saddr;
	u64 daddr;
	u32 config;
	u32 reserved2[3];
} __aligned(64);

struct peri_dma_desc_sw {
	struct virt_dma_desc vd;
	dma_addr_t desc_hw_lli;
	size_t desc_num;
	size_t size;
	struct peri_dma_desc_hw desc_hw[0];
};

struct peri_dma_phy;

struct peri_dma_chan {
	u32 ccfg;
	struct virt_dma_chan vc;
	struct peri_dma_phy *phy;
	struct list_head node;
	enum dma_transfer_direction dir;
	dma_addr_t dev_addr;
	enum dma_status status;
};

struct peri_dma_phy {
	u32 idx;
	void __iomem *base;
	struct peri_dma_chan *vchan;
	struct peri_dma_desc_sw *ds_run;
	struct peri_dma_desc_sw *ds_done;
};

struct peri_dma_dev {
	struct dma_device slave;
	void __iomem *base;
	struct tasklet_struct task;
	spinlock_t lock;
	struct list_head chan_pending;
	struct peri_dma_phy *phy;
	struct peri_dma_chan *chans;
	struct clk *clk;
	u32 dma_channels;
	u32 dma_requests;
	u32 dma_min_chan;
	u32 dma_used_chans;
	u32 dma_cpuid;
	int dma_irq;
	bool dma_share;
	u32 dma_pipeline_en;
};

#define to_peri_dma(dmadev) container_of(dmadev, struct peri_dma_dev, slave)

#ifdef CONFIG_HISI_DMA_PM_RUNTIME
/* Milliseconds */
#define HISI_DMA_AUTOSUSPEND_DELAY 1000
#endif

static struct peri_dma_chan *to_peri_dma_chan(struct dma_chan *chan)
{
	return container_of(chan, struct peri_dma_chan, vc.chan);
}

static void peri_dma_pause_dma(struct peri_dma_phy *phy,
	struct peri_dma_dev *d, bool on)
{
	u32 val;
	int timeout;

	if (on) {
		val = (u32)readl(phy->base + CX_CONFIG);
		val |= CCFG_EN;
		writel(val, phy->base + CX_CONFIG);
		*(int *)(phy->vchan->vc.chan.private) = 1;
	} else {
		val = (u32)readl(phy->base + CX_CONFIG);
		val &= ~CCFG_EN;
		writel(val, phy->base + CX_CONFIG);
		*(int *)(phy->vchan->vc.chan.private) = 0;
		/* Wait for channel inactive */
		for (timeout = DMA_WAIT_PHY_STATUS_TIMEOUT;
			timeout > 0; timeout--) {
			if (!(BIT(phy->idx) & (u64)readl(d->base + CH_STAT)))
				break;
			writel(val, phy->base + CX_CONFIG);
			/* delay 1us */
			udelay(1);
		}

		if (!timeout)
			pr_err("channel%u timeout wait for pause\n", phy->idx);
	}
}

static void peri_dma_terminate_chan(struct peri_dma_phy *phy,
	struct peri_dma_dev *d)
{
	u32 val;

	peri_dma_pause_dma(phy, d, false);

	/* phy->idx must be 0~15 */
	val = (u32)0x1 << phy->idx;
	writel(val, d->base + INT_TC1_RAW);
	writel(val, d->base + INT_ERR1_RAW);
	writel(val, d->base + INT_ERR2_RAW);
}

static void peri_dma_set_desc(struct peri_dma_phy *phy, struct peri_dma_desc_hw *hw)
{
	writel(GET_HIGH_32BIT(hw->lli), phy->base + CX_LLI_H);
	writel(GET_LOW_32BIT(hw->lli), phy->base + CX_LLI_L);
	writel(hw->count, phy->base + CX_CNT);
	writel(GET_HIGH_32BIT(hw->saddr), phy->base + CX_SRC_H);
	writel(GET_LOW_32BIT(hw->saddr), phy->base + CX_SRC_L);
	writel(GET_HIGH_32BIT(hw->daddr), phy->base + CX_DST_H);
	writel(GET_LOW_32BIT(hw->daddr), phy->base + CX_DST_L);
	writel(hw->config, phy->base + CX_CONFIG);
}

static u32 peri_dma_get_curr_cnt(struct peri_dma_dev *d,
	struct peri_dma_phy *phy)
{
	u32 cnt;

	cnt = (u32)readl(d->base + CX_CUR_CNT + phy->idx *
		CURRENT_COUNT_OFFSET);
	cnt &= 0xffff; /* current count mask bits */

	return cnt;
}

static u64 peri_dma_get_curr_lli(struct peri_dma_phy *phy)
{
	return U32_TO_U64(readl(phy->base + CX_LLI_H),
		readl(phy->base + CX_LLI_L));
}

static u32 peri_dma_get_chan_stat(struct peri_dma_dev *d)
{
	return readl(d->base + CH_STAT);
}

static void peri_dma_enable_dma(struct peri_dma_dev *d, bool on)
{
	if (on) {
		/* set same priority */
		writel(0x0, d->base + CH_PRI);

		/* unmask irq */
		writel(d->dma_used_chans, d->base + INT_TC1_MASK);
		writel(d->dma_used_chans, d->base + INT_ERR1_MASK);
		writel(d->dma_used_chans, d->base + INT_ERR2_MASK);
	} else {
		/* mask irq */
		writel(0x0, d->base + INT_TC1_MASK);
		writel(0x0, d->base + INT_ERR1_MASK);
		writel(0x0, d->base + INT_ERR2_MASK);
	}
}

static void peri_dma_tc1_int_prehdl(struct peri_dma_dev *d,
	u32 *tc1_irq, u32 i, u32 stats)
{
	struct peri_dma_chan *c = NULL;
	struct peri_dma_phy *p = NULL;
	unsigned long flags;

	p = &d->phy[i];
	c = p->vchan;
	if (c) {
		spin_lock_irqsave(&c->vc.lock, flags);
		if (p->ds_run)
			vchan_cookie_complete(&p->ds_run->vd);
		p->ds_done = p->ds_run;
		spin_unlock_irqrestore(&c->vc.lock, flags);
	} else {
		dev_err(d->slave.dev, "%s: phy[%u] stats[0x%x]\n", __func__,
			p->idx, stats);
	}

	*tc1_irq = (*tc1_irq) | (u32)BIT(i);
}

static void peri_dma_err_int_prehdl(struct peri_dma_dev *d, u32 i)
{
	struct peri_dma_chan *c = NULL;
	struct peri_dma_phy *p = NULL;

	p = &d->phy[i];
	c = p->vchan;
	if (c)
		c->status = DMA_ERROR;
}

static irqreturn_t peri_dma_int_handler(int irq, void *dev_id)
{
	struct peri_dma_dev *d = (struct peri_dma_dev *)dev_id;
	u32 stat = (u32)readl(d->base + INT_STAT);
	u32 tc1 = (u32)readl(d->base + INT_TC1);
	u32 err1 = (u32)readl(d->base + INT_ERR1);
	u32 err2 = (u32)readl(d->base + INT_ERR2);
	u32 i;
	u32 tc1_irq = 0;
	u32 err1_irq = 0;
	u32 err2_irq = 0;
	u32 stats = stat;

	while (stat) {
		i = (u32)__ffs((unsigned long)stat);
		stat &= (stat - 1);

		if (i >= d->dma_channels)
			continue;

		if (likely(tc1 & BIT(i)))
			peri_dma_tc1_int_prehdl(d, &tc1_irq, i, stats);

		if (unlikely((err1 & BIT(i)) || (err2 & BIT(i)))) {
			peri_dma_err_int_prehdl(d, i);

			if (err1 & BIT(i))
				err1_irq |= (u32)BIT(i);

			if (err2 & BIT(i))
				err2_irq |= (u32)BIT(i);

			dev_warn(d->slave.dev,
				"DMA ERR phy:%u st:0x%x err1:0x%x err2:0x%x\n",
				d->phy[i].idx, stats, err1, err2);
		}
	}

	writel(tc1_irq, d->base + INT_TC1_RAW);
	writel(err1_irq, d->base + INT_ERR1_RAW);
	writel(err2_irq, d->base + INT_ERR2_RAW);

	if (tc1_irq || err1_irq || err2_irq) {
		tasklet_schedule(&d->task);
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static int peri_dma_start_txd(struct peri_dma_chan *c)
{
	struct peri_dma_dev *d = to_peri_dma(c->vc.chan.device);
	struct virt_dma_desc *vd = vchan_next_desc(&c->vc);

	if (!c->phy) {
		dev_dbg(d->slave.dev, "%s: dma chan phy is NULL\n", __func__);
		return -ENODEV;
	}

	if (BIT(c->phy->idx) & peri_dma_get_chan_stat(d)) {
		dev_err(d->slave.dev, "%s: chan[%d] phy[%u] stat[0x%x]\n",
			__func__, c->vc.chan.chan_id, c->phy->idx,
			peri_dma_get_chan_stat(d));
		return -EBUSY;
	}

	if (vd) {
		struct peri_dma_desc_sw *ds =
			container_of(vd, struct peri_dma_desc_sw, vd);
		/*
		 * fetch and remove request from vc->desc_issued
		 * so vc->desc_issued only contains desc pending
		 */
		list_del(&ds->vd.node);
		c->phy->ds_run = ds;
		c->phy->ds_done = NULL;
		/* start dma */
		peri_dma_set_desc(c->phy, &ds->desc_hw[0]);
		if (c->ccfg & CCFG_EN)
			*(int *)(c->vc.chan.private) = 1;
		else
			*(int *)(c->vc.chan.private) = 0;

		return 0;
	}
	c->phy->ds_done = NULL;
	c->phy->ds_run = NULL;

	return -EAGAIN;
}

static void peri_dma_chk_running_dma(struct peri_dma_chan *c,
	struct peri_dma_dev *d)
{
	struct peri_dma_phy *p = NULL;
	unsigned long flags;
	int ret;

	if (!c) {
		dev_err(d->slave.dev, "%s:c is NULL\n", __func__);
		return;
	}

	spin_lock_irqsave(&c->vc.lock, flags);
	p = c->phy;

	if (p && p->ds_done) {
		ret = peri_dma_start_txd(c);
		if (ret == -EAGAIN) {
#ifdef CONFIG_HISI_DMA_PM_RUNTIME
			pm_runtime_mark_last_busy(d->slave.dev);
			pm_runtime_put_autosuspend(d->slave.dev);
#endif
			/* No current txd associated with this channel */
			dev_dbg(d->slave.dev, "pchan %u: free\n", p->idx);
			/* Mark this channel free */
			c->phy = NULL;
			p->vchan = NULL;
		} else if (ret) {
			spin_unlock_irqrestore(&c->vc.lock, flags);
			return;
		}
	} else if (p && c->status == DMA_ERROR) {
		peri_dma_terminate_chan(p, d);
#ifdef CONFIG_HISI_DMA_PM_RUNTIME
		pm_runtime_mark_last_busy(d->slave.dev);
		pm_runtime_put_autosuspend(d->slave.dev);
#endif
		c->phy = NULL;
		p->vchan = NULL;
		p->ds_run = p->ds_done = NULL;
	}

	spin_unlock_irqrestore(&c->vc.lock, flags);
}

static void peri_dma_tasklet(unsigned long arg)
{
	struct peri_dma_dev *d = (struct peri_dma_dev *)(uintptr_t)arg;
	struct peri_dma_phy *p = NULL;
	struct peri_dma_chan *c = NULL;
	struct peri_dma_chan *cn = NULL;
	unsigned int pch;
	unsigned int pch_alloc = 0;
	unsigned long flags;

	/* check new dma request of running channel in vc->desc_issued */
	list_for_each_entry_safe(c, cn, &d->slave.channels,
		vc.chan.device_node) {
		peri_dma_chk_running_dma(c, d);
	}

	/* check new channel request in d->chan_pending */
	spin_lock_irqsave(&d->lock, flags);

	for (pch = d->dma_min_chan; pch < d->dma_channels; pch++) {
		p = &d->phy[pch];

		if (!p->vchan && !list_empty(&d->chan_pending)) {
			c = list_first_entry(
				&d->chan_pending, struct peri_dma_chan, node);
			/* remove from d->chan_pending */
			list_del_init(&c->node);
			pch_alloc |= (u32)1 << pch;
#ifdef CONFIG_HISI_DMA_PM_RUNTIME
			pm_runtime_get_sync(d->slave.dev);
#endif
			/* Mark this channel allocated */
			p->vchan = c;
			c->phy = p;
			dev_dbg(d->slave.dev, "pchan %u: alloc vchan %pK\n",
				pch, &c->vc);
		}
	}
	spin_unlock_irqrestore(&d->lock, flags);

	/* d->dma_channels not larger than 16 */
	for (pch = d->dma_min_chan; pch < d->dma_channels; pch++) {
		if (pch_alloc & ((u32)1 << pch)) {
			p = &d->phy[pch];
			c = p->vchan;
			if (c) {
				spin_lock_irqsave(&c->vc.lock, flags);
				peri_dma_start_txd(c);
				spin_unlock_irqrestore(&c->vc.lock, flags);
			}
		}
	}
}

static int peri_dma_alloc_chan_resources(struct dma_chan *chan)
{
	return 0;
}

static void peri_dma_free_chan_resources(struct dma_chan *chan)
{
	struct peri_dma_chan *c = to_peri_dma_chan(chan);
	struct peri_dma_dev *d = to_peri_dma(chan->device);
	unsigned long flags;

	spin_lock_irqsave(&d->lock, flags);
	list_del_init(&c->node);
	spin_unlock_irqrestore(&d->lock, flags);

	vchan_free_chan_resources(&c->vc);
	c->ccfg = 0;
}

static enum dma_status peri_dma_tx_status(struct dma_chan *chan,
	dma_cookie_t cookie, struct dma_tx_state *state)
{
	struct peri_dma_chan *c = to_peri_dma_chan(chan);
	struct peri_dma_dev *d = to_peri_dma(chan->device);
	struct peri_dma_phy *p = NULL;
	struct virt_dma_desc *vd = NULL;
	unsigned long flags;
	enum dma_status ret;
	size_t bytes = 0;

	ret = dma_cookie_status(&c->vc.chan, cookie, state);
	if (ret == DMA_COMPLETE)
		return ret;

	spin_lock_irqsave(&c->vc.lock, flags);
	p = c->phy;
	ret = c->status;

	/*
	 * If the cookie is on our issue queue, then the residue is
	 * its total size.
	 */
	vd = vchan_find_desc(&c->vc, cookie);
	if (vd) {
		bytes = container_of(vd, struct peri_dma_desc_sw, vd)->size;
	} else if ((!p) || (!p->ds_run)) {
		bytes = 0;
	} else {
		struct peri_dma_desc_sw *ds = p->ds_run;
		u64 clli, index;

#ifdef CONFIG_HISI_DMA_PM_RUNTIME
		pm_runtime_get_sync(d->slave.dev);
#endif
		bytes = peri_dma_get_curr_cnt(d, p);
		clli = peri_dma_get_curr_lli(p);
		index = (clli - ds->desc_hw_lli) / sizeof(struct peri_dma_desc_hw);
		for (; index < ds->desc_num; index++) {
			bytes += ds->desc_hw[index].count;
			/* end of lli */
			if (!ds->desc_hw[index].lli)
				break;
		}
#ifdef CONFIG_HISI_DMA_PM_RUNTIME
		pm_runtime_mark_last_busy(d->slave.dev);
		pm_runtime_put_autosuspend(d->slave.dev);
#endif
	}
	spin_unlock_irqrestore(&c->vc.lock, flags);
	dma_set_residue(state, (u32)bytes);

	return ret;
}

static void peri_dma_issue_pending(struct dma_chan *chan)
{
	struct peri_dma_chan *c = to_peri_dma_chan(chan);
	struct peri_dma_dev *d = to_peri_dma(chan->device);
	unsigned long flags;

	spin_lock_irqsave(&c->vc.lock, flags);
	/* add request to vc->desc_issued */
	if (vchan_issue_pending(&c->vc)) {
		spin_lock(&d->lock);
		if (!c->phy)
			if (list_empty(&c->node)) {
				/* if new channel, add chan_pending */
				list_add_tail(&c->node, &d->chan_pending);
				/* check in tasklet */
				tasklet_schedule(&d->task);
				dev_dbg(d->slave.dev, "vchan %pK: issued\n",
					&c->vc);
			}
		spin_unlock(&d->lock);
	} else {
		dev_dbg(d->slave.dev, "vchan %pK: nothing to issue\n", &c->vc);
	}

	spin_unlock_irqrestore(&c->vc.lock, flags);
}

static void peri_dma_fill_desc(struct peri_dma_desc_sw *ds, dma_addr_t dst,
	dma_addr_t src, size_t len, u32 num, u32 ccfg)
{
	if ((num + 1) < ds->desc_num)
		ds->desc_hw[num].lli = ds->desc_hw_lli +
			(u64)(num + 1) * sizeof(struct peri_dma_desc_hw);

	ds->desc_hw[num].lli |= CX_LLI_CHAIN_EN;
	ds->desc_hw[num].count = (u32)len;
	ds->desc_hw[num].saddr = src;
	ds->desc_hw[num].daddr = dst;
	ds->desc_hw[num].config = ccfg;
}

static struct dma_async_tx_descriptor *peri_dma_prep_memcpy(
	struct dma_chan *chan, dma_addr_t dst, dma_addr_t src, size_t len,
	unsigned long flags)
{
	struct peri_dma_chan *c = to_peri_dma_chan(chan);
	struct peri_dma_desc_sw *ds = NULL;
	size_t copy = 0;
	u32 num;

	if (!len)
		return NULL;

	num = (u32)DIV_ROUND_UP(len, DMA_MAX_SIZE);
	ds = kzalloc(sizeof(*ds) + num * sizeof(ds->desc_hw[0]),
		GFP_ATOMIC | GFP_DMA);
	if (!ds)
		return NULL;

	ds->desc_hw_lli = __virt_to_phys((uintptr_t)&ds->desc_hw[0]);
	ds->size = len;
	ds->desc_num = (size_t)num;
	num = 0;

	if (!c->ccfg) {
		/* default is memtomem, without calling device_control */
		c->ccfg = CCFG_SRCINCR | CCFG_DSTINCR | CCFG_EN;
		/* burst : 16 */
		c->ccfg |= (0xf << DMA_CH_CFG_DL) | (0xf << DMA_CH_CFG_SL);
		/* width : 64 bit */
		c->ccfg |= (DMA_2BITS_MASK << DMA_CH_CFG_DW) |
			(DMA_2BITS_MASK << DMA_CH_CFG_SW);
	}

	do {
		copy = min_t(size_t, len, DMA_MAX_SIZE);
		peri_dma_fill_desc(ds, dst, src, copy, num++, c->ccfg);

		if (c->dir == DMA_MEM_TO_DEV) {
			src += copy;
		} else if (c->dir == DMA_DEV_TO_MEM) {
			dst += copy;
		} else {
			src += copy;
			dst += copy;
		}
		len -= copy;
	} while (len);

	ds->desc_hw[num - 1].lli = 0; /* end of link */
#if defined(CONFIG_HISI_DMA_NO_CCI)
	dma_sync_single_for_device(chan->device->dev, ds->desc_hw_lli,
		ds->desc_num * sizeof(ds->desc_hw[0]), DMA_TO_DEVICE);
#endif
	return vchan_tx_prep(&c->vc, &ds->vd, flags);
}

static u32 dma_calc_sg_num(struct scatterlist *sgl, unsigned int sglen)
{
	u32 i;
	size_t avail;
	struct scatterlist *sg = NULL;
	u32 num = sglen;

	for_each_sg(sgl, sg, sglen, i) {
		avail = sg_dma_len(sg);
		if (avail > DMA_MAX_SIZE)
			num += (u32)DIV_ROUND_UP(avail, DMA_MAX_SIZE) - 1;
	}

	return num;
}

static void dma_get_dst_and_src(
	const struct peri_dma_chan *c, enum dma_transfer_direction dir,
	dma_addr_t sg_addr,
	dma_addr_t *dst, dma_addr_t *src)
{
	if (dir == DMA_MEM_TO_DEV) {
		*src = sg_addr;
		*dst = c->dev_addr;
	} else if (dir == DMA_DEV_TO_MEM) {
		*src = c->dev_addr;
		*dst = sg_addr;
	}
}

static struct dma_async_tx_descriptor *peri_dma_prep_slave_sg(
	struct dma_chan *chan, struct scatterlist *sgl, unsigned int sglen,
	enum dma_transfer_direction dir, unsigned long flags, void *context)
{
	struct peri_dma_chan *c = to_peri_dma_chan(chan);
	struct peri_dma_desc_sw *ds = NULL;
	size_t len, avail;
	size_t total = 0;
	struct scatterlist *sg = NULL;
	dma_addr_t addr;
	dma_addr_t src = 0;
	dma_addr_t dst = 0;
	u32 num, i;

	(void)context;
	if (!sgl)
		return NULL;

	num = dma_calc_sg_num(sgl, sglen);

	ds = kzalloc(sizeof(*ds) + num * sizeof(ds->desc_hw[0]),
		GFP_ATOMIC | GFP_DMA);
	if (!ds)
		return NULL;

	ds->desc_hw_lli = __virt_to_phys((uintptr_t)&ds->desc_hw[0]);
	ds->desc_num = num;
	num = 0;

	for_each_sg(sgl, sg, sglen, i) {
		addr = sg_dma_address(sg);
		avail = sg_dma_len(sg);
		total += avail;

		do {
			len = min_t(size_t, avail, DMA_MAX_SIZE);

			dma_get_dst_and_src(c, dir, addr, &dst, &src);
			peri_dma_fill_desc(ds, dst, src, len, num++, c->ccfg);

			addr += len;
			avail -= len;
		} while (avail);
	}

	ds->desc_hw[(int)num - 1].lli = 0; /* end of link */
#if defined(CONFIG_HISI_DMA_NO_CCI)
	dma_sync_single_for_device(chan->device->dev, ds->desc_hw_lli,
		ds->desc_num * sizeof(ds->desc_hw[0]), DMA_TO_DEVICE);
#endif
	ds->size = total;
	return vchan_tx_prep(&c->vc, &ds->vd, flags);
}

static u32 peri_dma_convert_width(enum dma_slave_buswidth width)
{
	u32 val = 0;

	switch (width) {
	case DMA_SLAVE_BUSWIDTH_1_BYTE:
		val = DMA_SLAVE_BUS_WIDTH_VAL_1BYTE;
		break;
	case DMA_SLAVE_BUSWIDTH_2_BYTES:
		val = DMA_SLAVE_BUS_WIDTH_VAL_2BYTE;
		break;
	case DMA_SLAVE_BUSWIDTH_4_BYTES:
		val = DMA_SLAVE_BUS_WIDTH_VAL_4BYTE;
		break;
	case DMA_SLAVE_BUSWIDTH_8_BYTES:
		val = DMA_SLAVE_BUS_WIDTH_VAL_8BYTE;
		break;
	default:
		break;
	}

	return val;
}

static int peri_dma_config(struct dma_chan *chan,
	struct dma_slave_config *config)
{
	struct peri_dma_chan *c = to_peri_dma_chan(chan);
	struct peri_dma_dev *d = to_peri_dma(chan->device);
	u32 maxburst = 0;
	u32 val;
	enum dma_slave_buswidth width = DMA_SLAVE_BUSWIDTH_UNDEFINED;

	if (!config) {
		dev_warn(d->slave.dev, "config is NULL\n");
		return -EINVAL;
	}

	c->dir = config->direction;
	if (c->dir == DMA_DEV_TO_MEM) {
		c->ccfg = CCFG_DSTINCR;
		c->dev_addr = config->src_addr;
		maxburst = config->src_maxburst;
		width = config->src_addr_width;
	} else if (c->dir == DMA_MEM_TO_DEV) {
		c->ccfg = CCFG_SRCINCR;
		c->dev_addr = config->dst_addr;
		maxburst = config->dst_maxburst;
		width = config->dst_addr_width;
	} else if (c->dir == DMA_MEM_TO_MEM) {
		c->ccfg = CCFG_SRCINCR | CCFG_DSTINCR;
	}

	val = peri_dma_convert_width(width);
	c->ccfg |= (val << DMA_CH_CFG_DW) | (val << DMA_CH_CFG_SW);

	if (!maxburst || (maxburst > DMA_MAX_BURST))
		val = DMA_MAX_BURST;
	else
		val = maxburst - 1;

	c->ccfg |= (val << DMA_CH_CFG_DL) | (val << DMA_CH_CFG_SL);
	c->ccfg |= CCFG_MEM2PER | CCFG_EN;

	/* specific request line */
	c->ccfg |= (u32)c->vc.chan.chan_id << DMA_CH_CFG_PERI;
	return 0;
}

static int peri_dma_pause(struct dma_chan *chan)
{
	struct peri_dma_chan *c = to_peri_dma_chan(chan);
	struct peri_dma_dev *d = to_peri_dma(chan->device);
	unsigned long flags;
	struct peri_dma_phy *p = NULL;

	dev_dbg(d->slave.dev, "vchan %pK: pause\n", &c->vc);
	if (c->status == DMA_IN_PROGRESS) {
		c->status = DMA_PAUSED;
		spin_lock_irqsave(&d->lock, flags);
		p = c->phy;
		if (p) {
			spin_unlock_irqrestore(&d->lock, flags);
			peri_dma_pause_dma(p, d, false);
		} else {
			list_del_init(&c->node);
			spin_unlock_irqrestore(&d->lock, flags);
		}
	}

	return 0;
}

static int peri_dma_resume(struct dma_chan *chan)
{
	struct peri_dma_chan *c = to_peri_dma_chan(chan);
	struct peri_dma_dev *d = to_peri_dma(chan->device);
	unsigned long flags;
	struct peri_dma_phy *p = NULL;

	dev_dbg(d->slave.dev, "vchan %pK: resume\n", &c->vc);
	spin_lock_irqsave(&c->vc.lock, flags);
	p = c->phy;
	if (c->status == DMA_PAUSED) {
		c->status = DMA_IN_PROGRESS;
		if (p) {
			peri_dma_pause_dma(p, d, true);
		} else if (!list_empty(&c->vc.desc_issued)) {
			spin_lock(&d->lock);
			list_add_tail(&c->node, &d->chan_pending);
			spin_unlock(&d->lock);
		}
	}
	spin_unlock_irqrestore(&c->vc.lock, flags);

	return 0;
}

static int peri_dma_terminate_all(struct dma_chan *chan)
{
	struct peri_dma_chan *c = to_peri_dma_chan(chan);
	struct peri_dma_dev *d = to_peri_dma(chan->device);
	unsigned long flags;
	struct peri_dma_phy *p = NULL;
	LIST_HEAD(head);

	dev_dbg(d->slave.dev, "vchan %pK: terminate all\n", &c->vc);
	/* Prevent this channel being scheduled */
	spin_lock_irqsave(&d->lock, flags);
	list_del_init(&c->node);
	spin_unlock_irqrestore(&d->lock, flags);

	/* Clear the tx descriptor lists */
	spin_lock_irqsave(&c->vc.lock, flags);
	p = c->phy;
	vchan_get_all_descriptors(&c->vc, &head);
	if (p && p->ds_run && !p->ds_done)
		list_add_tail(&p->ds_run->vd.node, &head);

	if (p) {
		/* vchan is assigned to a pchan - stop the channel */
		peri_dma_terminate_chan(p, d);
		c->phy = NULL;
		p->vchan = NULL;
		p->ds_run = p->ds_done = NULL;
#ifdef CONFIG_HISI_DMA_PM_RUNTIME
		pm_runtime_mark_last_busy(d->slave.dev);
		pm_runtime_put_autosuspend(d->slave.dev);
#endif
	}
	spin_unlock_irqrestore(&c->vc.lock, flags);
	vchan_dma_desc_free_list(&c->vc, &head);

	return 0;
}

static int check_dma_chan(struct dma_chan *chan)
{
	if (!chan) {
		pr_err("%s: dma_chan *chan is NULL!\n", __func__);
		return -EINVAL;
	}

	pr_err("%s: chan[0x%pK] id[%d] cookie[%d-%d]!\n", __func__, chan,
		chan->chan_id, chan->cookie, chan->completed_cookie);

	if (!chan->device) {
		pr_err("show_dma_reg: chan->device * is NULL!\n");
		return -EINVAL;
	}

	return 0;
}

static int check_peri_dma_dev(struct peri_dma_dev *d)
{
	if (!d) {
		pr_err("peri_dma_dev *d is NULL!\n");
		return -EINVAL;
	}
	if (!d->base) {
		pr_err("d-base is NULL!\n");
		return -EINVAL;
	}
	if (!d->slave.dev) {
		pr_err("d->slave.dev is NULL!\n");
		return -EINVAL;
	}

	return 0;
}

static int check_peri_dma_chan(struct peri_dma_dev *d, struct peri_dma_chan *c)
{
	if (!(c->phy)) {
		dev_err(d->slave.dev, "d->phy is NULL!\n");
		return -EINVAL;
	}
	if (!(c->phy->base)) {
		dev_err(d->slave.dev, "d->phy->base is NULL!\n");
		return -EINVAL;
	}

	return 0;
}

static void show_peri_dma_dev_base_reg(struct peri_dma_dev *d)
{
	dev_info(d->slave.dev, "INT_STAT = 0x%x\n", readl(d->base + INT_STAT));
	dev_info(d->slave.dev, "INT_TC1  = 0x%x\n", readl(d->base + INT_TC1));
	dev_info(d->slave.dev, "INT_ERR1 = 0x%x\n", readl(d->base + INT_ERR1));
	dev_info(d->slave.dev, "INT_ERR2 = 0x%x\n", readl(d->base + INT_ERR2));

	dev_info(d->slave.dev, "INT_TC1_MASK  = 0x%x\n",
		readl(d->base + INT_TC1_MASK));
	dev_info(d->slave.dev, "INT_ERR1_MASK = 0x%x\n",
		readl(d->base + INT_ERR1_MASK));
	dev_info(d->slave.dev, "INT_ERR2_MASK = 0x%x\n",
		readl(d->base + INT_ERR2_MASK));

	dev_info(d->slave.dev, "INT_TC1_RAW  = 0x%x\n",
		readl(d->base + INT_TC1_RAW));
	dev_info(d->slave.dev, "INT_TC2_RAW  = 0x%x\n",
		readl(d->base + INT_TC2_RAW));
	dev_info(d->slave.dev, "INT_ERR1_RAW = 0x%x\n",
		readl(d->base + INT_ERR1_RAW));
	dev_info(d->slave.dev, "INT_ERR2_RAW = 0x%x\n",
		readl(d->base + INT_ERR2_RAW));

	dev_info(d->slave.dev, "CH_STAT  = 0x%x\n", readl(d->base + CH_STAT));
	dev_info(d->slave.dev, "DMA_CTRL = 0x%x\n", readl(d->base + DMA_CTRL));
}

void show_dma64_reg(struct dma_chan *chan)
{
	struct peri_dma_chan *c = NULL;
	struct peri_dma_dev *d = NULL;
	struct peri_dma_phy *p = NULL;
	u32 i;

	if (check_dma_chan(chan))
		return;

	d = to_peri_dma(chan->device);
	if (check_peri_dma_dev(d))
		return;

	c = to_peri_dma_chan(chan);
	if (check_peri_dma_chan(d, c))
		return;

	p = c->phy;
	dev_info(d->slave.dev,
		"%s:chan[%pK] ccfg[0x%x] dir[%d] dev_addr[0x%llx] status[%d]\n",
		__func__, c, c->ccfg, c->dir, c->dev_addr, c->status);
	dev_info(d->slave.dev,
		"%s: phy idx[0x%x] ds_run[%pK] ds_done[%pK]\n", __func__,
		p->idx, p->ds_run, p->ds_done);

	for (i = d->dma_min_chan; i < d->dma_channels; i++) {
		p = &d->phy[i];
		dev_info(d->slave.dev,
			"idx[%2d]:CX_CONFIG:[0x%32x], CX_AXI_CONF:[0x%32x]",
			i, readl(p->base + CX_CONFIG),
			readl(p->base + AXI_CONFIG));
		dev_info(d->slave.dev,
			"SRC_H:[0x%32x], SRC_L:[0x%32x], DST_H:[0x%32x]",
			readl(p->base + CX_SRC_H), readl(p->base + CX_SRC_L),
			readl(p->base + CX_DST_H));
		dev_info(d->slave.dev,
			"DST_L:[0x%32x], CNT0:[0x%32x], CX_CUR_CNT0:[0x%32x]\n",
			readl(p->base + CX_DST_L), readl(p->base + CX_CNT),
			readl(p->base - i * CURRENT_COUNT_OFFSET + CX_CUR_CNT));
	}

	show_peri_dma_dev_base_reg(d);
}
EXPORT_SYMBOL(show_dma64_reg);

static void peri_dma_free_desc(struct virt_dma_desc *vd)
{
	struct peri_dma_desc_sw *ds =
		container_of(vd, struct peri_dma_desc_sw, vd);
#if defined(CONFIG_HISI_DMA_NO_CCI)
	dma_sync_single_for_cpu(vd->tx.chan->device->dev, ds->desc_hw_lli,
		ds->desc_num * sizeof(ds->desc_hw[0]), DMA_FROM_DEVICE);
#endif
	kfree(ds);
}

static const struct of_device_id peri_pdma64_dt_ids[] = {
	{
		.compatible = "hisilicon,hisi-dma64-1.0",
	},
	{}
};
MODULE_DEVICE_TABLE(of, peri_pdma64_dt_ids);

static struct dma_chan *peri_of_dma_simple_xlate(
	struct of_phandle_args *dma_spec, struct of_dma *ofdma)
{
	struct peri_dma_dev *d = ofdma->of_dma_data;
	unsigned int request = dma_spec->args[0];

	if (request > d->dma_requests)
		return NULL;

	return dma_get_slave_channel(&(d->chans[request].vc.chan));
}

static noinline int atfd_peri_service_dmac_smc(
	u64 _function_id, u64 _arg0, u64 _arg1, u64 _arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;
	asm volatile(__asmeq("%0", "x0")
		     __asmeq("%1", "x1")
		     __asmeq("%2", "x2")
		     __asmeq("%3", "x3") "smc    #0\n"
		     : "+r"(function_id)
		     : "r"(arg0), "r"(arg1), "r"(arg2));

	return (int)function_id;
}

static void peri_dma_set_slave_cfg(struct dma_device *slave,
	struct platform_device *op, struct peri_dma_dev *d)
{
	INIT_LIST_HEAD(&slave->channels);
	dma_cap_set(DMA_SLAVE, slave->cap_mask);
	dma_cap_set(DMA_MEMCPY, slave->cap_mask);
	dma_cap_set(DMA_PRIVATE, slave->cap_mask);
	slave->dev = &op->dev;
	slave->device_alloc_chan_resources = peri_dma_alloc_chan_resources;
	slave->device_free_chan_resources = peri_dma_free_chan_resources;
	slave->device_tx_status = peri_dma_tx_status;
	slave->device_prep_dma_memcpy = peri_dma_prep_memcpy;
	slave->device_prep_slave_sg = peri_dma_prep_slave_sg;
	slave->device_issue_pending = peri_dma_issue_pending;
	slave->device_config = peri_dma_config;
	slave->device_pause = peri_dma_pause;
	slave->device_resume = peri_dma_resume;
	slave->device_terminate_all = peri_dma_terminate_all;
	slave->copy_align = DMA_ALIGN;
	slave->chancnt = d->dma_requests;
}

static int peri_dma_read_dev_dt_cfg(struct platform_device *op,
	struct peri_dma_dev *d)
{
	int ret = 0;
	const struct of_device_id *of_id = NULL;

	of_id = of_match_device(peri_pdma64_dt_ids, &op->dev);
	if (of_id)
		ret = of_property_read_u32(
			(&op->dev)->of_node, "dma-channels", &d->dma_channels);

	if (ret) {
		dev_err(&op->dev, "%s doesn't have dma-channels property!\n",
			__func__);
		return ret;
	}

	ret = of_property_read_u32((&op->dev)->of_node, "dma-requests",
		&d->dma_requests);
	if (ret) {
		dev_err(&op->dev, "%s doesn't have dma-request property!\n",
			__func__);
		return ret;
	}

	ret = of_property_read_u32((&op->dev)->of_node, "dma-min-chan",
		&d->dma_min_chan);
	if (ret) {
		dev_info(&op->dev, "%s doesn't have dma-min-chan property!\n",
			__func__);
		return ret;
	}

	ret = of_property_read_u32((&op->dev)->of_node, "dma-pipeline-en",
		&d->dma_pipeline_en);
	if (ret) {
		d->dma_pipeline_en = UNSUPPORT; /* not support dma_pipeline */
		dev_info(&op->dev, "%s not support dma_pipeline property!\n",
			__func__);
	}

	(void)atfd_peri_service_dmac_smc(DMAC_REGISTER_FN_ID, d->dma_min_chan,
		0, 0);

	ret = of_property_read_u32((&op->dev)->of_node, "dma-used-chans",
		&d->dma_used_chans);
	if (ret) {
		dev_info(&op->dev, "%s doesn't have dma-used-chans property!\n",
			__func__);
		return ret;
	}

	if (of_property_read_u32((&op->dev)->of_node, "dma-cpuid",
		&d->dma_cpuid)) {
		d->dma_cpuid = 0;
		dev_info(&op->dev, "%s doesn't have dma_cpuid property!\n",
			__func__);
	}

	d->dma_share = of_property_read_bool((&op->dev)->of_node, "dma-share");
	if (d->dma_share)
		dev_info(&op->dev, "here dma_share is true\n");

	return ret;
}

static void peri_set_dma_ctrl_pipeline(struct peri_dma_dev *d)
{
	unsigned int dma_ctrl_cfg;

	if (d->dma_pipeline_en == 0) {
		dma_ctrl_cfg = readl(d->base + DMA_CTRL);
		dma_ctrl_cfg &= ~DMA_CTRL_PIPE_LINE;
		writel(dma_ctrl_cfg, d->base + DMA_CTRL);
	}
}
static int peri_dma_new_dev(struct peri_dma_dev **dev,
	struct platform_device *op)
{
	int ret;
	struct peri_dma_dev *d = NULL;
	struct resource *iores = NULL;

	iores = platform_get_resource(op, IORESOURCE_MEM, 0);
	if (!iores)
		return -EINVAL;

	d = devm_kzalloc(&op->dev, sizeof(*d), GFP_KERNEL);
	if (!d)
		return -ENOMEM;

	d->base = devm_ioremap_resource(&op->dev, iores);
	if (!d->base)
		return -EADDRNOTAVAIL;

	ret = peri_dma_read_dev_dt_cfg(op, d);
	if (ret) {
		dev_err(&op->dev, "dma read dev cfg failed. ret=[%d]\n", ret);
		return ret;
	}
	peri_set_dma_ctrl_pipeline(d);

	dev_info(&op->dev, "dma_channels:0x%x dma_requests:0x%x",
		d->dma_channels, d->dma_requests);
	dev_info(&op->dev, "dma_min_chan:0x%x dma_used_chans:0x%x\n",
		d->dma_min_chan, d->dma_used_chans);
	*dev = d;

	return ret;
}

static int peri_dma_init_virtual_channel(
	struct peri_dma_dev *d, struct platform_device *op)
{
	u32 i;

	d->chans = devm_kzalloc(&op->dev,
		d->dma_requests * sizeof(struct peri_dma_chan), GFP_KERNEL);
	if (!d->chans)
		return -ENOMEM;

	for (i = 0; i < d->dma_requests; i++) {
		struct peri_dma_chan *c = &d->chans[i];
		c->status = DMA_IN_PROGRESS;
		INIT_LIST_HEAD(&c->node);
		c->vc.desc_free = peri_dma_free_desc;
		vchan_init(&c->vc, &d->slave);
		c->vc.chan.private = kmalloc(sizeof(int), GFP_KERNEL);
		if (!c->vc.chan.private) {
			dev_err(&op->dev, "chan %d kmalloc failed\n", i);
			return -ENOMEM;
		} else {
			*(int *)(c->vc.chan.private) = 0;
		}
	}

	return 0;
}

static int peri_dma_init_phy(struct platform_device *op, struct peri_dma_dev *d)
{
	u32 i;

	d->phy = devm_kzalloc(&op->dev,
		d->dma_channels * sizeof(struct peri_dma_phy), GFP_KERNEL);
	if (!d->phy)
		return -ENOMEM;

	for (i = d->dma_min_chan; i < d->dma_channels; i++) {
		struct peri_dma_phy *p = &d->phy[i];

		p->idx = i;
		p->base = d->base + i * DMA_CHAN_OFFSET;
	}

	return 0;
}

static int peri_dma_pm_runtime_prepare(struct peri_dma_dev *d,
	struct platform_device *op)
{
	int ret;

#ifdef CONFIG_HISI_DMA_PM_RUNTIME
	pm_runtime_irq_safe(d->slave.dev);
	pm_runtime_set_autosuspend_delay(
		d->slave.dev, HISI_DMA_AUTOSUSPEND_DELAY);
	pm_runtime_use_autosuspend(d->slave.dev);
	pm_runtime_enable(d->slave.dev);
	ret = clk_prepare(d->clk);
	if (ret < 0) {
		dev_err(&op->dev, "clk_prepare failed: %d\n", ret);
		return -EINVAL;
	}
#else
	/* Enable clock before accessing registers */
	ret = clk_prepare_enable(d->clk);
	if (ret < 0) {
		dev_err(&op->dev, "clk_prepare_enable failed: %d\n", ret);
		return -EINVAL;
	}
	peri_dma_enable_dma(d, true);
#endif
	return 0;
}

static int peri_dma_prepare_irq(struct platform_device *op,
	struct peri_dma_dev *d)
{
	int ret;

	d->dma_irq = platform_get_irq(op, 0);
	if (d->dma_irq < 0) {
		dev_err(&op->dev, "platform get irq failed. irq=%d\n",
			d->dma_irq);
		return -ENXIO;
	}

	ret = devm_request_irq(
		&op->dev, (u32)d->dma_irq, peri_dma_int_handler, 0,
			DRIVER_NAME, d);
	if (ret) {
		dev_err(&op->dev, "request irq failed irq=%d\n", d->dma_irq);
		return ret;
	}

	if (cpu_online(d->dma_cpuid) && d->dma_cpuid) {
		ret = irq_set_affinity(d->dma_irq, cpumask_of(d->dma_cpuid));
		if (ret)
			dev_info(&op->dev, "bind dma irq to cpu%u failed\n",
				d->dma_cpuid);
	}

	return 0;
}

static int peri_dma_probe(struct platform_device *op)
{
	struct peri_dma_dev *d = NULL;

	if (peri_dma_new_dev(&d, op) || !d) {
		dev_err(&op->dev, "new dma dev failed\n");
		return -ENOMEM;
	}

	op->dev.dma_mask = &(op->dev.coherent_dma_mask);
	op->dev.coherent_dma_mask = DMA_BIT_MASK(64); /* 64bit */

	d->clk = devm_clk_get(&op->dev, NULL);
	if (IS_ERR(d->clk)) {
		dev_err(&op->dev, "no dma clk\n");
		return (int)PTR_ERR(d->clk);
	}

	if (peri_dma_prepare_irq(op, d)) {
		dev_err(&op->dev, "prepare irq failed\n");
		return -1;
	}

	/* init phy channel */
	if (peri_dma_init_phy(op, d)) {
		dev_err(&op->dev, "init phy channel failed\n");
		return -1;
	}

	op->dev.coherent_dma_mask = DMA_BIT_MASK(64); /* 64bit */
	op->dev.dma_mask = &op->dev.coherent_dma_mask;
	peri_dma_set_slave_cfg(&d->slave, op, d);

	/* init virtual channel */
	if (peri_dma_init_virtual_channel(d, op)) {
		dev_err(&op->dev, "init virtual channel failed\n");
		return -1;
	}

	if (peri_dma_pm_runtime_prepare(d, op)) {
		dev_err(&op->dev, "prepare clk failed\n");
		return -1;
	}

	if (dma_async_device_register(&d->slave)) {
		dev_err(&op->dev, "async device register failed\n");
		return -1;
	}

	if (of_dma_controller_register(
		(&op->dev)->of_node, peri_of_dma_simple_xlate, d)) {
		dev_err(&op->dev, "dma device register failed\n");
		dma_async_device_unregister(&d->slave);
		return -1;
	}

	spin_lock_init(&d->lock);
	INIT_LIST_HEAD(&d->chan_pending);
	tasklet_init(&d->task, peri_dma_tasklet, (uintptr_t)d);
	platform_set_drvdata(op, d);

	dev_info(&op->dev, "initialized. peri_dma64_probe ok!\n");

	return 0;
}

static int peri_dma_remove(struct platform_device *op)
{
	struct peri_dma_chan *c = NULL;
	struct peri_dma_chan *cn = NULL;
	struct peri_dma_dev *d = platform_get_drvdata(op);

	if (!d) {
		pr_err("%s: get drvdata failed\n", __func__);
		return -EINVAL;
	}

	dma_async_device_unregister(&d->slave);
	of_dma_controller_free((&op->dev)->of_node);

	/* Clear virtual channel resource */
	list_for_each_entry_safe(c, cn, &d->slave.channels,
		vc.chan.device_node) {
		if (c->vc.chan.private)
			kfree(c->vc.chan.private);
		list_del(&c->vc.chan.device_node);
		tasklet_kill(&c->vc.task);
	}
	tasklet_kill(&d->task);
#ifdef CONFIG_HISI_DMA_PM_RUNTIME
	clk_unprepare(d->clk);
#else
	clk_disable_unprepare(d->clk);
#endif
	return 0;
}

#if defined(CONFIG_PM_SLEEP)
static int peri_dma_pltfm_suspend(struct device *dev)
{
	struct peri_dma_dev *d = dev_get_drvdata(dev);
	u32 stat;
	u32 limit = DMA_WAIT_CHANN_TIMEOUT;
	u32 cx_cfg = 0;
	u32 axi_cfg = 0;
	u32 i;

	if (!d) {
		pr_err("%s: get drvdata failed\n", __func__);
		return -EINVAL;
	}

	dev_info(d->slave.dev, "%s: suspend +\n", __func__);
	pm_runtime_get_sync(d->slave.dev);

	stat = peri_dma_get_chan_stat(d);
	stat = stat & d->dma_used_chans;

	while (stat && limit--) {
		udelay(1000); /* delay 1000 us */
		stat = peri_dma_get_chan_stat(d);
		stat = stat & d->dma_used_chans;
	}

	if (stat) {
		dev_warn(d->slave.dev, "chan 0x%x is running fail to suspend\n",
			stat);

		for (i = d->dma_min_chan; i < d->dma_channels; i++) {
			dev_warn(d->slave.dev, "channel:%u\n", i);
			cx_cfg = readl(d->base + CX_CONFIG +
				i * DMA_CHAN_OFFSET);
			dev_warn(d->slave.dev, "CX_CONFIG:0x%x\n", cx_cfg);
			axi_cfg = readl(d->base + AXI_CONFIG +
				i * DMA_CHAN_OFFSET);
			dev_warn(d->slave.dev, "AXI_CONFIG:0x%x\n", axi_cfg);
		}

		return -1;
	}

	peri_dma_enable_dma(d, false);
	if (!d->dma_share)
		clk_disable(d->clk);

	pm_runtime_put_sync(d->slave.dev);
	dev_info(d->slave.dev, "%s: suspend -\n", __func__);
	return 0;
}

static int peri_dma_pltfm_resume(struct device *dev)
{
	struct peri_dma_dev *d = dev_get_drvdata(dev);
	int ret = 0;

	if (!d) {
		pr_err("%s: get drvdata failed\n", __func__);
		return -EINVAL;
	}

	dev_info(d->slave.dev, "%s: resume +\n", __func__);
	pm_runtime_get_sync(d->slave.dev);

	peri_set_dma_ctrl_pipeline(d);

	if (!d->dma_share) {
		ret = clk_enable(d->clk);
		if (ret < 0) {
			dev_err(d->slave.dev, "clk_prepare_enable failed: %d\n",
				ret);
			return -EINVAL;
		}
	}

	if (cpu_online(d->dma_cpuid) && d->dma_cpuid) {
		ret = irq_set_affinity(d->dma_irq, cpumask_of(d->dma_cpuid));
		if (ret)
			dev_info(d->slave.dev, "bind dma irq to cpu%u failed\n",
				d->dma_cpuid);
	}
	peri_dma_enable_dma(d, true);

	pm_runtime_put_sync(d->slave.dev);
	dev_info(d->slave.dev, "%s: resume -\n", __func__);
	return 0;
}

#ifdef CONFIG_HISI_DMA_PM_RUNTIME
static int peri_dma_runtime_suspend(struct device *dev)
{
	u32 stat;
	struct peri_dma_dev *d = dev_get_drvdata(dev);
	int ret = 0;
	u32 limit = DMA_WAIT_CHANN_TIMEOUT;
	u32 cx_cfg = 0;
	u32 axi_cfg = 0;
	u32 i;

	if (!d) {
		pr_err("%s: get drvdata failed\n", __func__);
		return -EINVAL;
	}

	dev_info(d->slave.dev, "%s: +,\n", __func__);

	stat = peri_dma_get_chan_stat(d);
	stat = stat & d->dma_used_chans;

	while ((stat) && limit--) {
		udelay(1000); /* delay 1000 us */
		stat = peri_dma_get_chan_stat(d);
		stat = stat & d->dma_used_chans;
	}
	if (stat) {
		dev_warn(d->slave.dev, "chan 0x%x is running fail to suspend\n",
			stat);

		for (i = d->dma_min_chan; i < d->dma_channels; i++) {
			dev_warn(d->slave.dev, "channel:%u\n", i);
			cx_cfg = readl(d->base + CX_CONFIG +
				i * DMA_CHAN_OFFSET);
			dev_warn(d->slave.dev, "CX_CONFIG:0x%x\n", cx_cfg);
			axi_cfg = readl(d->base + AXI_CONFIG +
				i * DMA_CHAN_OFFSET);
			dev_warn(d->slave.dev, "AXI_CONFIG:0x%x\n", axi_cfg);
		}
		return -1;
	}

	peri_dma_enable_dma(d, false);
	if (!d->dma_share)
		clk_disable(d->clk);

	dev_info(d->slave.dev, "%s: -,\n", __func__);
	return ret;
}

static int peri_dma_runtime_resume(struct device *dev)
{
	struct peri_dma_dev *d = dev_get_drvdata(dev);
	int ret = 0;

	if (!d) {
		pr_err("%s: get drvdata failed\n", __func__);
		return -EINVAL;
	}

	dev_info(d->slave.dev, "%s: +,\n", __func__);
	if (!d->dma_share) {
		ret = clk_enable(d->clk);
		if (ret < 0) {
			dev_err(d->slave.dev, "clk_prepare_enable failed:%d\n",
				ret);
			return -EINVAL;
		}
	}
	peri_dma_enable_dma(d, true);
	dev_info(d->slave.dev, "%s: -,\n", __func__);
	return ret;
}
#endif /* CONFIG_HISI_DMA_PM_RUNTIME */
#endif /* CONFIG_PM_SLEEP */

const struct dev_pm_ops peri_dma64_pm_ops = {
#ifdef CONFIG_HISI_DMA_PM_RUNTIME
	 SET_RUNTIME_PM_OPS(peri_dma_runtime_suspend,
		peri_dma_runtime_resume, NULL)
#endif
	SET_LATE_SYSTEM_SLEEP_PM_OPS(peri_dma_pltfm_suspend,
		peri_dma_pltfm_resume)
};

static struct platform_driver peri_pdma64_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.pm = &peri_dma64_pm_ops,
		.of_match_table = peri_pdma64_dt_ids,
	},
	.probe = peri_dma_probe,
	.remove = peri_dma_remove,
};

static int __init dmac_module_init(void)
{
	int retval;

	retval = platform_driver_register(&peri_pdma64_driver);
	if (retval) {
		pr_err("peri dma platform driver register failed\n");
		return retval;
	}

	return retval;
}

static void __exit dmac_module_exit(void)
{
	platform_driver_unregister(&peri_pdma64_driver);
}

arch_initcall(dmac_module_init);
MODULE_DESCRIPTION("peri 64bit DMA Driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
