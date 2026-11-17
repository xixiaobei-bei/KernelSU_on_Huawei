/*
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#include "hw-peri-dvs.h"
#ifdef CONFIG_HISI_CLK_DEBUG
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#endif
#include "../clk-smc.h"

#define MAX_PERIDVS_PROT_STRLEN   64
#define HIMASH_OFFSET             16
#define VOTE_WIDTH                (g_dvs_cfg->vote_width)
#define VOTE_MASK                 (BIT(VOTE_WIDTH) - 1)
#define BUCK13_VOTE_BIT           2
#define SC_TCP_VOTE_REQ_0         0x8D0
#define SC_TCP_VOTE_STAT          0x8D4
#define CUR_PATH_STAT_0           0xE0
#define DELAY_10_US               10
#define TCP_SET_TIMEOUT           100

/* hisi hw peridvs useful macro funcs */
#define himask_enable(shift)      (0x10001 << (shift))
#define himask_disable(shift)     (0x10000 << (shift))
#define dvs_vote(volt)            ((volt) & VOTE_MASK)
#define himask_volt(volt, shift)  \
	(((VOTE_MASK << HIMASH_OFFSET) | dvs_vote(volt)) << (shift))
#define get_vote(reg, idx)        (((reg) >> ((idx) * VOTE_WIDTH)) & VOTE_MASK)
#define max_vote(v1, v2)          ((v1) > (v2) ? (v1) : (v2))
#define error_code(err_code)      ((unsigned int)(err_code))
#define pmc_writel(val, addr)     \
	writel((val), (g_dvs_cfg->pmc_reg_base + (addr)))
#define pmc_readl(addr)           \
	((unsigned int)readl(g_dvs_cfg->pmc_reg_base + (addr)))
#define sc_writel(val, addr)     \
	writel((val), (g_dvs_cfg->sctrl_reg_base + (addr)))
#define sc_readl(addr)           \
	((unsigned int)readl(g_dvs_cfg->sctrl_reg_base + (addr)))

/* hisi hw peri dvs log macro funcs */
#define dvs_err(fmt, args...)     \
	pr_err("[Hw_PeriDVS] [%s] [ERROR] " fmt, __func__, ## args)
#define dvs_info(fmt, args...)    \
	pr_info("[Hw_PeriDVS] [%s] [INFO] " fmt, __func__, ## args)
#define dvs_dbg(fmt, args...)     \
	pr_debug("[Hw_PeriDVS] [%s] [DEBUG] " fmt, __func__, ## args)

static struct hwspinlock        *g_peri_dvs_hwlock;
static struct peri_dvs_config   *g_dvs_cfg;
static struct peri_dvs_mediabus *g_mediabus_cfg[MEDIABUS_MAX];
static struct peri_dvs_channel **g_dvs_channels;

#ifdef CONFIG_HISI_CLK_BUCK13_DVS
static unsigned int g_ap_buck13_vote;
static DEFINE_SPINLOCK(g_ap_buck13_lock);
#endif

#ifdef CONFIG_HISI_CLK_DEBUG
void peri_dvs_debug_init(void);
#endif

#ifdef CONFIG_HISI_CLK_BUCK13_DVS
static void peri_buck13_tcp_set(unsigned int vote_mng0)
{
	unsigned int val;
	unsigned int timeout = TCP_SET_TIMEOUT; /* max wait for 1ms */

	if (vote_mng0) {
		sc_writel(himask_enable(BUCK13_VOTE_BIT), SC_TCP_VOTE_REQ_0);

		val = sc_readl(SC_TCP_VOTE_STAT);
		val = val & CUR_PATH_STAT_0;
		if (!val)
			return;

		do {
			udelay(DELAY_10_US);
			timeout--;
			val = sc_readl(SC_TCP_VOTE_STAT);
			val = val & CUR_PATH_STAT_0;
		} while (val && timeout);

		if (val)
			dvs_err("buck13 tcp set fail, val 0x%x timeout %u, g_ap_vote 0x%x!\n",
				     val, timeout, g_ap_buck13_vote);
	} else {
		sc_writel(himask_disable(BUCK13_VOTE_BIT), SC_TCP_VOTE_REQ_0);
	}
}

void peri_buck13_mem_volt_set(unsigned int channel, unsigned int buck13_volt)
{
	unsigned long flags;
	unsigned int vote_mng0;

	if (channel >= DVFS_CHANEL_MAX || buck13_volt >= DVFS_BUCK13_MAX) {
		dvs_err("input illegal, channel %u buck13_volt %u!\n",
			channel, buck13_volt);
		return;
	}

	spin_lock_irqsave(&g_ap_buck13_lock, flags);

	if (buck13_volt)
		g_ap_buck13_vote = g_ap_buck13_vote | BIT(channel);
	else
		g_ap_buck13_vote = g_ap_buck13_vote & (~BIT(channel));

	if (g_ap_buck13_vote)
		vote_mng0 = 1;
	else
		vote_mng0 = 0;

	peri_buck13_tcp_set(vote_mng0);

	spin_unlock_irqrestore(&g_ap_buck13_lock, flags);
}

static void peri_dvs_set_buck13_vote(const struct peri_dvs_channel *chan,
				   unsigned int volt)
{
	unsigned int buck13_volt = DVFS_BUCK13_VOLT1;

	/*
	 * The ISP does not go through this process.
	 * The ISP directly invokes peri_buck13_mem_volt_set.
	 */
	if (chan->channel == DVFS_ISP_CHANEL)
		return;

	if (volt < PERI_VOLT_3)
		buck13_volt = DVFS_BUCK13_VOLT0;

	peri_buck13_mem_volt_set(chan->channel, buck13_volt);
}
#endif

static unsigned int peri_dvs_get_chan_vote(const struct peri_dvs_channel *chan)
{
	unsigned int rank_idx = chan->offset[DVS_VOTE_REG];
	unsigned int reg_value;

	reg_value = pmc_readl(chan->reg[DVS_VOTE_REG]);
	return get_vote(reg_value, rank_idx);
}

static void peri_dvs_set_chan_vote(const struct peri_dvs_channel *chan,
				   unsigned int volt)
{
	unsigned int rank_idx = chan->offset[DVS_VOTE_REG];
	unsigned int reg_value;

	/* channel volt vote set */
	reg_value = himask_volt(volt, rank_idx * VOTE_WIDTH);
	pmc_writel(reg_value, chan->reg[DVS_VOTE_REG]);
}

static struct peri_dvs_vote *peri_dvs_get_pdv(const struct peri_volt_poll *pvp)
{
	if (!pvp)
		return NULL;
	return pvp->priv_date;
}

static int peri_dvs_get_temprature(struct peri_volt_poll *pvp)
{
	unsigned int temprature;
	int ret = 0;

	if (unlikely(!pvp || !g_dvs_cfg || !g_dvs_cfg->peri_dvs_enabled))
		return 0;

	temprature = pmc_readl(g_dvs_cfg->lpmcu_offset);
	temprature &= g_dvs_cfg->lowtemp_mask;
	temprature >>= g_dvs_cfg->lowtemp_shift;
	if (temprature != NORMAL_TEMPRATURE)
		ret = -1;

	return ret;
}

static void peri_dvs_chan_vote_flow(const struct peri_dvs_channel *chan,
				    unsigned int volt)
{
	unsigned int reg_value;
	unsigned int timeout = 0;

	/* if peri volt locked, then volt flow will skip here */
	if (g_dvs_cfg->volt_level_fixed)
		return;

	/*
	 * peri volt set to 0.8V, need vote buck13 to 0.8v at first.
	 * otherwise , need vote buck13 to 0.8v at last.
	 */
#ifdef CONFIG_HISI_CLK_BUCK13_DVS
	if (volt == PERI_VOLT_3)
		peri_dvs_set_buck13_vote(chan, volt);
#endif

	/* channel volt vote set */
	peri_dvs_set_chan_vote(chan, volt);

	/* channel valid set */
	if (chan->chan_valid)
		pmc_writel(himask_enable(chan->offset[DVS_VLD_REG]),
			   chan->reg[DVS_VLD_REG]);
	else
		pmc_writel(himask_disable(chan->offset[DVS_VLD_REG]),
			   chan->reg[DVS_VLD_REG]);

	/* bypass complete irq to ACPU/LPMCU */
	pmc_writel(himask_enable(chan->offset[DVS_IRQ_CPT_BP_REG]),
		   chan->reg[DVS_IRQ_CPT_BP_REG]);

	/* channel volt vote request */
	pmc_writel(BIT(chan->offset[DVS_REQ_REG]), chan->reg[DVS_REQ_REG]);
	for (;;) {
		/* wait channel irq complete */
		reg_value = pmc_readl(chan->reg[DVS_IRQ_CPT_REG]);
		if (reg_value & BIT(chan->offset[DVS_IRQ_CPT_REG]))
			break;

		udelay(1);
		timeout++;
		if (timeout > g_dvs_cfg->max_irq_usec) {
			dvs_err("ch %u wait irq state %u timeout!\n",
				     chan->channel, reg_value);
			break;
		}
	}

	/* clear complete irq */
	pmc_writel(BIT(chan->offset[DVS_IRQ_CLR_REG]),
		   chan->reg[DVS_IRQ_CLR_REG]);

#ifdef CONFIG_HISI_CLK_BUCK13_DVS
	if (volt != PERI_VOLT_3)
		peri_dvs_set_buck13_vote(chan, volt);
#endif
}

static unsigned int peri_dvs_get_fixed_volt(void)
{
	unsigned int fix_volt = PERI_FIXED_VOLT_NONE;

#ifdef CONFIG_HISI_DEBUG_FS
	fix_volt = pmc_readl(g_dvs_cfg->lpmcu_offset);
	fix_volt &= g_dvs_cfg->fixed_volt_mask;
	fix_volt >>= g_dvs_cfg->fixed_volt_shift;
#endif
	return fix_volt;
}

#define BUS_DEFAULT_PROFILE         PERI_VOLT_MAX
static int mediabus_clk_freq_set(const struct peri_dvs_mediabus *mediabus,
				 unsigned int target_volt)
{
	unsigned int volt = mediabus->volt_level;
	struct clk *cur_clk = NULL;
	struct clk *tar_clk = NULL;
	int ret;

	if (volt == target_volt)
		return 0;

	if (volt >= PERI_VOLT_MAX)
		return -EINVAL;
	cur_clk = mediabus->pll_clks[volt];

	if (target_volt >= PERI_VOLT_MAX)
		return -EINVAL;
	tar_clk = mediabus->pll_clks[target_volt];

	ret = clk_prepare_enable(tar_clk);
	if (ret) {
		dvs_err("bus %u volt %u: open tar_clk failed, ret = %d, name %s, pre_cnt = %u, en_cnt = %u\n",
			mediabus->bus_type, target_volt, ret,
			tar_clk->core->name, tar_clk->core->prepare_count,
			tar_clk->core->enable_count);
		return ret;
	}

	/* SMC into ATF to switch mediabus freq from volt to target_volt */
	ret = atfd_service_clk_smc(CKM_BUS_FREQ_SET, mediabus->bus_type,
					target_volt, 0);
	if (ret) {
		dvs_err("bus %u volt %u: atf freq set failed,ret=%d\n",
			     mediabus->bus_type, target_volt, ret);
		clk_disable_unprepare(tar_clk);
		return ret;
	}

	clk_disable_unprepare(cur_clk);

	return 0;
}

static void mediabus_clk_enable(const struct peri_dvs_mediabus *mediabus)
{
	unsigned int volt = mediabus->volt_level;
	struct clk *tar_clk = NULL;
	int ret;

	if (volt >= PERI_VOLT_MAX)
		return;
	tar_clk = mediabus->pll_clks[volt];

	ret = clk_prepare_enable(tar_clk);
	if (ret) {
		dvs_err("bus %u volt %u: enable cur_clk failed,ret=%d\n",
			     mediabus->bus_type, volt, ret);
		return;
	}

	/* SMC into ATF to switch mediabus freq from default to volt */
	ret = atfd_service_clk_smc(CKM_BUS_FREQ_SET, mediabus->bus_type,
					volt, 0);
	if (ret)
		dvs_err("bus %u volt %u: atf freq set failed,ret=%d\n",
			     mediabus->bus_type, volt, ret);
}

static void mediabus_clk_disable(const struct peri_dvs_mediabus *mediabus)
{
	unsigned int volt = mediabus->volt_level;
	struct clk *cur_clk = NULL;
	int ret;

	if (volt >= PERI_VOLT_MAX)
		return;
	cur_clk = mediabus->pll_clks[volt];

	/* SMC into ATF to set mediabus freq to default */
	ret = atfd_service_clk_smc(CKM_BUS_FREQ_SET, mediabus->bus_type,
					BUS_DEFAULT_PROFILE, 0);
	if (ret)
		dvs_err("bus %u volt %u: atf freq set failed,ret=%d\n",
			     mediabus->bus_type, volt, ret);

	clk_disable_unprepare(cur_clk);
}

static void mediabus_clk_update(unsigned int type, int low_temp)
{
	struct peri_dvs_vote *pdv = NULL;
	struct peri_dvs_mediabus *mediabus = NULL;
	unsigned int volt = 0;
	unsigned int vol = PERI_VOLT_MAX;
	int ret;

	if (type >= MEDIABUS_MAX || !g_mediabus_cfg[type]) {
		/* ignore invalid mediabus type */
		return;
	}
	mediabus = g_mediabus_cfg[type];

	list_for_each_entry(pdv, &mediabus->mediabus_list, mediabus_node) {
		if (!pdv->vote_valid)
			continue;
		vol = pdv->volt_level;
		volt = max_vote(volt, vol);
	}

	if (volt >= PERI_VOLT_MAX) {
		dvs_err("invalid peri volt level %u\n", volt);
		return;
	}

	/* low temp case */
	if (low_temp && volt > mediabus->low_temp_profile)
		volt = mediabus->low_temp_profile;

	if (vol < PERI_VOLT_MAX) {
		if (!mediabus->valid) {
			mediabus->volt_level = volt;
			mediabus_clk_enable(mediabus);
			mediabus->valid = 1;
			return;
		}
	} else {
		/* No pdvs on mediabus list are vote valid */
		if (mediabus->valid) {
			mediabus_clk_disable(mediabus);
			mediabus->volt_level = volt;
			mediabus->valid = 0;
			return;
		}
	}

	ret = mediabus_clk_freq_set(mediabus, volt);
	if (ret) {
		dvs_err("set mediabus[%u] level %u failed, ret=%d\n",
			     mediabus->bus_type, volt, ret);
		return;
	}

	if (volt < PERI_VOLT_MAX)
		mediabus->volt_level = volt;
}

static void peri_dvs_per_vote_record(const struct peri_dvs_vote *pdv)
{
	unsigned int reg_value;
	void __iomem *reg_base = NULL;
	unsigned int record_mask = pdv->record_mask;
	unsigned int record_shift = pdv->record_shift;

	if (unlikely(!record_mask))
		return;

	if (pdv->record_reg_type == VOTE_RECORD_PMC_REG_BASE) {
		reg_base = g_dvs_cfg->pmc_reg_base;
	} else if (pdv->record_reg_type == VOTE_RECORD_SCTRL_REG_BASE) {
		reg_base = g_dvs_cfg->sctrl_reg_base;
		/* sctrl record reg share map with other ip */
		if (hwspin_lock_timeout(g_peri_dvs_hwlock, HWLOCK_TIMEOUT)) {
			dvs_err("get record hwspinlock timout!\n");
			return;
		}
	} else {
		return;
	}

	/* Only kernel access PMC_PERI_CTRL5 and SCTRL_BAKDATA16 reg */
	reg_value = readl(reg_base + pdv->record_reg_offset);
	reg_value &= (~record_mask);
	if (pdv->vote_valid)
		reg_value |= ((pdv->volt_level << record_shift) & record_mask);
	writel(reg_value, reg_base + pdv->record_reg_offset);

	if (pdv->record_reg_type == VOTE_RECORD_SCTRL_REG_BASE)
		hwspin_unlock(g_peri_dvs_hwlock);
}

static int channel_volt_update(const struct peri_dvs_vote *pdv, int low_temp)
{
	struct peri_dvs_vote *t_pdv = NULL;
	struct peri_dvs_channel *dvs_channel = NULL;
	unsigned int target_volt = 0;
	unsigned int volt;

	if ((pdv->channel >= g_dvs_cfg->max_vote_channel) ||
	    (!g_dvs_channels[pdv->channel])) {
		dvs_err("invalid channel %u\n", pdv->channel);
		return -EINVAL;
	}
	dvs_channel = g_dvs_channels[pdv->channel];
	volt = dvs_channel->volt_level;

	/* gather all volt vote on the same channel */
	list_for_each_entry(t_pdv, &dvs_channel->chan_list, chan_node)
		if (t_pdv->vote_valid && target_volt < t_pdv->volt_level)
			target_volt = t_pdv->volt_level;

	/* if channel volt level not updated, skip volt vote flow */
	if (volt == target_volt && volt != PERI_VOLT_0)
		return 0;

	if (pdv->sysbus_sync_flag) {
		/*
		 * set sysbus sync flag (PERI_CTRL5 bit[31:30]):
		 *     lpm3 (5ms) check this flag to trigger volt request
		 *
		 * update channel volt level
		 */
		dvs_channel->volt_level = target_volt;
		return 0;
	}

	/* mediabus freq down before volt down */
	if (volt >= target_volt)
		mediabus_clk_update(pdv->mediabus_id, low_temp);

	/* peri volt vote flow */
	peri_dvs_chan_vote_flow(dvs_channel, target_volt);

	/* mediabus freq up after volt up */
	if (volt < target_volt)
		mediabus_clk_update(pdv->mediabus_id, low_temp);

	/* update channel volt level */
	dvs_channel->volt_level = target_volt;

	return 0;
}

static int peri_dvs_set_volt_normal(struct peri_volt_poll *pvp, unsigned int volt)
{
	unsigned int old_volt;
	int ret, low_temp;
	struct peri_dvs_vote *pdv = NULL;

	if (volt >= PERI_VOLT_MAX) {
		dvs_err("invalid peri volt");
		return -EINVAL;
	}

	pdv = peri_dvs_get_pdv(pvp);
	if (!pdv) {
		dvs_err("get pdv failed\n");
		return -ENODEV;
	}

	old_volt = pdv->volt_level;
	pdv->volt_level = volt;
	low_temp = peri_dvs_get_temprature(pvp);

	ret = channel_volt_update(pdv, low_temp);
	if (ret < 0) {
		pdv->volt_level = old_volt;
		return ret;
	}

	peri_dvs_per_vote_record(pdv);

	return 0;
}

static unsigned int peri_dvs_get_volt(struct peri_volt_poll *pvp)
{
	struct peri_dvs_vote *pdv = NULL;
	struct peri_dvs_channel *dvs_chan = NULL;

	if (unlikely(!pvp || !g_dvs_cfg))
		return error_code(-EINVAL);

	/* peridvs disabled, then return max level vote */
	if (unlikely(!g_dvs_cfg->peri_dvs_enabled))
		return PERI_VOLT_3;

	if (unlikely(g_dvs_cfg->volt_level_fixed != PERI_FIXED_VOLT_NONE))
		return g_dvs_cfg->volt_level_fixed - 1;

	pdv = peri_dvs_get_pdv(pvp);
	if (!pdv) {
		dvs_err("get pdv failed\n");
		return error_code(-ENODEV);
	}

	/* pdv sysbus sync flag set, then return max level vote */
	if (pdv->sysbus_sync_flag)
		return PERI_VOLT_3;

	if (pdv->channel < g_dvs_cfg->max_vote_channel)
		dvs_chan = g_dvs_channels[pdv->channel];
	if (!dvs_chan) {
		dvs_err("dev_id %u get channel failed\n", pvp->dev_id);
		return error_code(-EINVAL);
	}

	return peri_dvs_get_chan_vote(dvs_chan);
}

static int peri_dvs_set_volt(struct peri_volt_poll *pvp, unsigned int volt)
{
	int ret;

	if (unlikely(!pvp || !g_dvs_cfg))
		return -EINVAL;

	if (unlikely(!g_dvs_cfg->peri_dvs_enabled))
		return 0;

	if ((g_dvs_cfg->volt_level_fixed == PERI_FIXED_VOLT_NONE) ||
	    (volt + 1 <= g_dvs_cfg->volt_level_fixed)) {
		ret = peri_dvs_set_volt_normal(pvp, volt);
		if (ret < 0)
			return ret;
	} else { /* volt + 1 > g_dvs_cfg->volt_level_fixed */
		dvs_err("Failed set volt %u bigger than fixed_volt %u\n",
			volt, g_dvs_cfg->volt_level_fixed - 1);
		return -EINVAL;
	}

	return 0;
}

static int peri_dvs_set_vote_valid(struct peri_volt_poll *pvp, unsigned int endis)
{
	struct peri_dvs_vote *pdv = NULL;
	struct peri_dvs_vote *t_pdv = NULL;
	struct peri_dvs_channel *dvs_channel = NULL;
	unsigned int channel, old_chan_valid, new_chan_valid;

	if (unlikely(!pvp || !g_dvs_cfg))
		return -EINVAL;

	if (unlikely(!g_dvs_cfg->peri_dvs_enabled))
		return 0;

	pdv = peri_dvs_get_pdv(pvp);
	if (!pdv) {
		dvs_err("get pdv failed\n");
		return -ENODEV;
	}

	channel = pdv->channel;
	if (channel >= g_dvs_cfg->max_vote_channel || !g_dvs_channels[channel]) {
		dvs_err("invalid channel %u!\n", channel);
		return -EINVAL;
	}
	dvs_channel = g_dvs_channels[channel];

	old_chan_valid = (!!dvs_channel->chan_valid);
	new_chan_valid = 0;
	pdv->vote_valid = (!!endis);
	list_for_each_entry(t_pdv, &dvs_channel->chan_list, chan_node)
		if (t_pdv->vote_valid)
			new_chan_valid = 1;

	if (old_chan_valid != new_chan_valid) {
		dvs_channel->chan_valid = new_chan_valid;
		/* pdv valid vote flag set */
		if (pdv->valid_vote_sync)
			return peri_dvs_set_volt(pvp, pdv->volt_level);
	}

	return 0;
}

static void peri_dvs_init(struct peri_volt_poll *pvp, unsigned int endis)
{
	/* nothing to do */
}

static struct peri_volt_ops dvs_ops = {
	.get_volt        = peri_dvs_get_volt,
	.recalc_volt     = peri_dvs_get_volt,
	.set_avs         = peri_dvs_set_vote_valid,
	.set_volt        = peri_dvs_set_volt,
	.get_temperature = peri_dvs_get_temprature,
	.init            = peri_dvs_init,
};

static void peri_dvs_vote_deinit(void)
{
	unsigned int i;
	struct peri_dvs_vote *pdv = NULL;
	struct peri_dvs_vote *pdv_next = NULL;
	struct peri_volt_poll *pvp = NULL;

	if (!g_dvs_cfg || !g_dvs_channels)
		return;

	for (i = 0; i < g_dvs_cfg->max_vote_channel; i++) {
		if (!g_dvs_channels[i])
			continue;
		list_for_each_entry_safe(pdv, pdv_next,
					 &g_dvs_channels[i]->chan_list,
					 chan_node) {
			pvp = peri_volt_poll_get(pdv->dev_id, NULL);
			list_del(&pdv->chan_node);
			if (pvp) {
				list_del(&pvp->node);
				kfree(pvp->name);
			}
			kfree(pvp);
			kfree(pdv);
		}
	}
}

static int peri_dvs_vote_bconfig_dt_parse(const struct device_node *np,
					  struct peri_dvs_vote *pdv)
{
	/* record reg type, offset and mask */
	unsigned int record_data[3] = {0};

	if (of_property_read_string(np, "perivolt-output-names", &pdv->name)) {
		dvs_err("%s node doesn't have perivolt-output-names\n",
			np->name);
		return -EINVAL;
	}

	if (of_property_read_u32(np, "perivolt-poll-id", &pdv->dev_id)) {
		dvs_err("%s node doesn't have %s\n",
			np->name, "perivolt-poll-id");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "perivolt-poll-channel", &pdv->channel)) {
		dvs_err("%s node doesn't have perivolt-poll-channel\n",
			np->name);
		return -EINVAL;
	}

	/* record reg type, offset and mask */
	if (of_property_read_u32_array(np, "perivolt-poll-record",
				       &record_data[0], 3)) {
		pdv->record_reg_type = VOTE_RECORD_MAX_REG_BASE;
		pdv->record_reg_offset = 0;
		pdv->record_mask = 0;
		pdv->record_shift = 0;
	} else {
		pdv->record_reg_type = record_data[0];
		pdv->record_reg_offset = record_data[1];
		pdv->record_mask = record_data[2];
		pdv->record_shift = ffs(pdv->record_mask) - 1;
	}

	if (of_property_read_u32(np, "perivolt-poll-mediabus",
			&pdv->mediabus_id))
		pdv->mediabus_id = MEDIABUS_MAX;

	if (of_property_read_u32(np, "perivolt-poll-valid-vote-sync",
				 &pdv->valid_vote_sync))
		pdv->valid_vote_sync = 0;

	if (of_property_read_u32(np, "perivolt-poll-sysbus-sync",
				&pdv->sysbus_sync_flag))
		pdv->sysbus_sync_flag = 0;

	return 0;
}

static int peri_dvs_vote_init(const struct device_node *np,
			      struct peri_dvs_vote **ppdv)
{
	struct peri_dvs_vote *pdv = NULL;
	struct peri_dvs_channel *dvs_channel = NULL;
	unsigned int chan;
	int ret;

	pdv = kzalloc(sizeof(*pdv), GFP_KERNEL);
	if (!pdv)
		return -ENOMEM;

	ret = peri_dvs_vote_bconfig_dt_parse(np, pdv);
	if (ret < 0)
		goto fail;

	chan = pdv->channel;
	if (chan >= g_dvs_cfg->max_vote_channel || !g_dvs_channels[chan]) {
		dvs_err("invalid channel %u\n", chan);
		ret = -EINVAL;
		goto fail;
	}
	dvs_channel = g_dvs_channels[chan];

	/* insert pdv into Channel list according to channel prot */
	list_add(&pdv->chan_node, &dvs_channel->chan_list);

	/* insert pdv into Mediabus list if mediabus_id is valid */
	if (pdv->mediabus_id < MEDIABUS_MAX)
		list_add(&pdv->mediabus_node,
			 &g_mediabus_cfg[pdv->mediabus_id]->mediabus_list);

	pdv->vote_valid = 0;
	pdv->volt_level = PERI_VOLT_0;
	*ppdv = pdv;
	return 0;
fail:
	kfree(pdv);
	return ret;
}

static int pvp_node_init(const struct device_node *np)
{
	struct peri_volt_poll *pvp = NULL;
	struct peri_dvs_vote *pdv = NULL;
	int ret;

	ret = peri_dvs_vote_init(np, &pdv);
	if (ret < 0)
		return ret;

	pvp = kzalloc(sizeof(*pvp), GFP_KERNEL);
	if (!pvp)
		return -ENOMEM;

	pvp->name = kstrdup(pdv->name, GFP_KERNEL);
	if (!pvp->name) {
		ret = -ENOMEM;
		goto err_pvp;
	}

	pvp->dev_id = pdv->dev_id;
	pvp->perivolt_avs_ip = 0; /* Ignore AVS Cases */
	pvp->addr = g_dvs_cfg->pmc_reg_base + g_dvs_cfg->acpu_offset;
	pvp->addr_0 = g_dvs_cfg->pmc_reg_base + g_dvs_cfg->lpmcu_offset;
	pvp->sysreg_base = NULL;
	pvp->bitsshift = pdv->record_shift;
	pvp->bitsmask = pdv->record_mask;
	pvp->ops = &dvs_ops;
	pvp->volt = pdv->volt_level;
	pvp->flags = PERI_GET_VOLT_NOCACHE;
	pvp->priv = g_peri_dvs_hwlock;
	pvp->stat = 0;
	pvp->priv_date = pdv;

	ret = perivolt_register(NULL, pvp);
	if (ret) {
		dvs_err("fail to reigister pvp %s\n", pvp->name);
		goto err_pvp_name;
	}

	dvs_info("peri dvfs node:%s\n", pvp->name);

err_pvp_name:
	kfree(pvp->name);
err_pvp:
	kfree(pvp);
	return ret;
}

PERIVOLT_OF_DECLARE(peridvs, "hisilicon,soc-peri-volt", pvp_node_init);

static void peri_dvs_reg_base_deinit(void)
{
	if (g_dvs_cfg->pmc_reg_base) {
		iounmap(g_dvs_cfg->pmc_reg_base);
		g_dvs_cfg->pmc_reg_base = NULL;
	}

	if (g_dvs_cfg->sctrl_reg_base) {
		iounmap(g_dvs_cfg->sctrl_reg_base);
		g_dvs_cfg->sctrl_reg_base = NULL;
	}
}

static int peri_dvs_reg_base_dt_parse(struct device_node *np)
{
	struct device_node *sctrl_np = NULL;

	g_dvs_cfg->pmc_reg_base = of_iomap(np, 0);
	if (!g_dvs_cfg->pmc_reg_base) {
		dvs_err("get pmc reg base failed\n");
		return -ENODEV;
	}

	sctrl_np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (!sctrl_np) {
		pr_err("[%s] fail to find sctrl node\n", __func__);
		return -ENODEV;
	}
	g_dvs_cfg->sctrl_reg_base = of_iomap(sctrl_np, 0);
	if (!g_dvs_cfg->sctrl_reg_base) {
		dvs_err("get sctrl reg base failed\n");
		return -ENODEV;
	}

	if (of_property_read_u32(np, "perivolt-poll-acpu",
			&g_dvs_cfg->acpu_offset)) {
		dvs_err("%s node doesn't have %s\n",
			np->name, "perivolt-poll-acpu");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "perivolt-poll-lpmcu",
			&g_dvs_cfg->lpmcu_offset)) {
		dvs_err("%s node doesn't have %s\n",
			np->name, "perivolt-poll-lpmcu");
		return -EINVAL;
	}

	return 0;
}

#ifdef CONFIG_HISI_CLK_DEBUG
static void peri_dvs_channel_dump_reg_dt_parse(const struct device_node *np)
{
	if (of_property_read_u32(np, "peridvs-chan-reg-dump-size",
			&g_dvs_cfg->dump_reg_num)) {
		dvs_err("%s node doesn't have %s\n",
			np->name, "peridvs-chan-reg-dump-size");
		g_dvs_cfg->dump_reg_num = MAX_PERI_DVS_DUMP_REG_NUM;
	}
	if (g_dvs_cfg->dump_reg_num > MAX_PERI_DVS_DUMP_REG_NUM)
		g_dvs_cfg->dump_reg_num = MAX_PERI_DVS_DUMP_REG_NUM;

	if (of_property_read_u32_array(np, "peridvs-chan-reg-dump",
		(u32 *)g_dvs_cfg->dump_reg, g_dvs_cfg->dump_reg_num)) {
		dvs_err("%s node doesn't have %s\n",
			np->name, "peridvs-chan-reg-dump");
		g_dvs_cfg->dump_reg_num = 0;
		return;
	}
}
#endif

static int peri_dvs_channel_global_cfg_dt_parse(const struct device_node *np)
{
	if (of_property_read_u32(np, "perivolt-poll-max-voltlevel",
			&g_dvs_cfg->max_volt_level)) {
		dvs_err("%s node doesn't have %s\n",
			np->name, "perivolt-poll-max-voltlevel");
		g_dvs_cfg->max_volt_level = PERI_VOLT_MAX - 1;
	}
	if (g_dvs_cfg->max_volt_level >= PERI_VOLT_MAX)
		g_dvs_cfg->max_volt_level = PERI_VOLT_MAX - 1;

	if (of_property_read_u32(np, "perivolt-poll-vote-width",
			&g_dvs_cfg->vote_width)) {
		dvs_err("%s node doesn't have %s\n",
			np->name, "perivolt-poll-vote-width");
		g_dvs_cfg->vote_width = MAX_PERI_DVS_VOTE_WIDTH;
	}
	if (g_dvs_cfg->vote_width > MAX_PERI_DVS_VOTE_WIDTH)
		g_dvs_cfg->vote_width = MAX_PERI_DVS_VOTE_WIDTH;

	if (of_property_read_u32(np, "perivolt-poll-max-channel-size",
			&g_dvs_cfg->max_vote_channel)) {
		dvs_err("%s node doesn't have %s\n",
			np->name, "perivolt-poll-max-channel-size");
		g_dvs_cfg->max_vote_channel = MAX_PERI_DVS_SFT_CHANNEL;
	}
	if (g_dvs_cfg->max_vote_channel > MAX_PERI_DVS_SFT_CHANNEL)
		g_dvs_cfg->max_vote_channel = MAX_PERI_DVS_SFT_CHANNEL;

#ifdef CONFIG_HISI_CLK_DEBUG
	peri_dvs_channel_dump_reg_dt_parse(np);
#endif
	return 0;
}

static int peri_dvs_global_cfg_dt_parse(const struct device_node *np)
{
	if (of_property_read_u32(np, "perivolt-poll-lpmcu-lowtemp",
			&g_dvs_cfg->lowtemp_mask)) {
		dvs_err("%s node doesn't have %s\n",
			np->name, "perivolt-poll-lpmcu-lowtemp");
		return -EINVAL;
	}
	g_dvs_cfg->lowtemp_shift = ffs(g_dvs_cfg->lowtemp_mask) - 1;

	if (of_property_read_u32(np, "perivolt-poll-lpmcu-fixedvolt",
			&g_dvs_cfg->fixed_volt_mask)) {
		dvs_err("%s node doesn't have %s\n",
			np->name, "perivolt-poll-lpmcu-fixedvolt");
		return -EINVAL;
	}
	g_dvs_cfg->fixed_volt_shift = ffs(g_dvs_cfg->fixed_volt_mask) - 1;

	if (of_property_read_u32(np, "perivolt-poll-max-irq-complete-usec",
		&g_dvs_cfg->max_irq_usec) ||
			g_dvs_cfg->max_irq_usec > MAX_PERI_DVS_IRQ_USEC)
		g_dvs_cfg->max_irq_usec = MAX_PERI_DVS_IRQ_USEC;
	if (g_dvs_cfg->max_irq_usec == 0)
		g_dvs_cfg->max_irq_usec = DEFAULT_PERI_DVS_IRQ_USEC;

	/* "peri-dvs-enabled" not exist, then set peri_dvs_enable to 1 */
	if (of_property_read_u32(np, "peri-dvs-enabled",
				 &g_dvs_cfg->peri_dvs_enabled))
		g_dvs_cfg->peri_dvs_enabled = 1;

	/* update fixed volt level only once */
	g_dvs_cfg->volt_level_fixed = peri_dvs_get_fixed_volt();
	return peri_dvs_channel_global_cfg_dt_parse(np);
}

static void peri_dvs_config_deinit(void)
{
	if (g_dvs_cfg) {
		peri_dvs_reg_base_deinit();
		kfree(g_dvs_cfg);
		g_dvs_cfg = NULL;
	}
}

static int peri_dvs_config_init(struct device *dev)
{
	struct device_node *np = dev->of_node;
	int ret;

	g_dvs_cfg = kzalloc(sizeof(*g_dvs_cfg), GFP_KERNEL);
	if (!g_dvs_cfg)
		return -ENOMEM;

	ret = peri_dvs_reg_base_dt_parse(np);
	if (ret < 0)
		return ret;

	ret = peri_dvs_global_cfg_dt_parse(np);
	if (ret < 0)
		return ret;

	return 0;
}

static int mediabus_pll_clk_init(struct device *dev, struct peri_dvs_mediabus *mediabus)
{
	const struct device_node *np = dev->of_node;
	const char *clock_name = NULL;
	struct clk *clock = NULL;
	char prot[MAX_PERIDVS_PROT_STRLEN];
	int ret, index, pll_clk_num;

	memset(prot, 0, sizeof(prot));
	ret = snprintf(prot, sizeof(prot), "mediabus%u-clock-names",
		mediabus->bus_type);
	if (ret <= 0) {
		dvs_err("snprintf failed! ret=%d\n", ret);
		return -EINVAL;
	}

	pll_clk_num = of_property_count_strings(np, (const char *)&prot[0]);
	if (pll_clk_num <= 0) {
		dvs_err("clock names is null\n");
		return -EINVAL;
	}

	if (pll_clk_num != PERI_VOLT_MAX) {
		dvs_err("clock num %d error\n", pll_clk_num);
		return -EINVAL;
	}

	for (index = 0; index < pll_clk_num; index++) {
		if (of_property_read_string_index(np, prot, index,
						  &clock_name)) {
			dvs_err("failed to find clock! index=%d\n", index);
			return -EINVAL;
		}
		clock = devm_clk_get(dev, clock_name);
		if (IS_ERR_OR_NULL(clock)) {
			dvs_err("failed to get clock %s index=%d\n",
				     clock_name, index);
			return PTR_ERR(clock);
		}
		mediabus->pll_clks[index] = clock;
	}

	return 0;
}

static int peri_dvs_per_mediabus_init(struct device *dev, unsigned int type)
{
	const struct device_node *np = dev->of_node;
	struct peri_dvs_mediabus *mediabus = NULL;
	char prot[MAX_PERIDVS_PROT_STRLEN];
	int ret;

	if (type >= MEDIABUS_MAX)
		return -EINVAL;

	mediabus = kzalloc(sizeof(*mediabus), GFP_KERNEL);
	if (!mediabus)
		return -ENOMEM;

	memset(prot, 0, sizeof(prot));
	ret = snprintf(prot, sizeof(prot), "mediabus%u-lowtemp-profile", type);
	if (ret <= 0) {
		dvs_err("snprintf failed! ret=%d\n", ret);
		ret = -EINVAL;
		goto err_init;
	}

	if (of_property_read_u32(np, prot, &mediabus->low_temp_profile)) {
		dvs_info("%s node doesn't have %s\n", np->name, prot);
		mediabus->low_temp_profile = PERI_VOLT_MAX;
	}

	mediabus->bus_type = type;
	ret = mediabus_pll_clk_init(dev, mediabus);
	if (ret < 0)
		goto err_init;

	mediabus->valid = 0;
	mediabus->volt_level = PERI_VOLT_0;
	INIT_LIST_HEAD(&mediabus->mediabus_list);
	g_mediabus_cfg[type] = mediabus;
	return 0;

err_init:
	kfree(mediabus);
	return ret;
}

static void peri_dvs_mediabus_deinit(void)
{
	enum mediabus_type type;

	for (type = MEDIABUS_VIVOBUS; type < MEDIABUS_MAX; type++) {
		kfree(g_mediabus_cfg[type]);
		g_mediabus_cfg[type] = NULL;
	}
}

static int peri_dvs_mediabus_init(struct device *dev)
{
	int ret;

	ret = peri_dvs_per_mediabus_init(dev, MEDIABUS_VIVOBUS);
	if (ret < 0)
		goto fail;

	ret = peri_dvs_per_mediabus_init(dev, MEDIABUS_VCODECBUS);
	if (ret < 0)
		goto fail;

	return 0;

fail:
	peri_dvs_mediabus_deinit();
	return ret;
}

static int peri_dvs_channel_reg_dt_parse(const struct device_node *np,
					 struct peri_dvs_channel *dvs_channel)
{
	unsigned int reg_num = 0;
	char prot[MAX_PERIDVS_PROT_STRLEN];
	int ret;

	if (of_property_read_u32(np, "peridvs-chan-regnum", &reg_num) ||
	    reg_num != DVS_MAX_REG) {
		dvs_err("%s node doesn't have %s\n",
			     np->name, "peridvs-chan-regnum");
		return -EINVAL;
	}

	memset(prot, 0, sizeof(prot));
	ret = snprintf(prot, sizeof(prot), "peridvs-chan%u-reg", dvs_channel->channel);
	if (ret < 0) {
		dvs_err("dvs reg: snprintf failed! ret=%d\n", ret);
		return -EINVAL;
	}

	if (of_property_read_u32_array(np, prot, (u32 *)dvs_channel->reg,
				       reg_num)) {
		dvs_info("%s node doesn't have %s\n", np->name, prot);
		return -ENODEV;
	}

	memset(prot, 0, sizeof(prot));
	ret = snprintf(prot, sizeof(prot), "peridvs-chan%u-offset",
		dvs_channel->channel);
	if (ret < 0) {
		dvs_err("dvs offset: snprintf failed! ret=%d\n", ret);
		return -EINVAL;
	}

	if (of_property_read_u32_array(np, prot, (u32 *)dvs_channel->offset,
				       reg_num)) {
		dvs_err("%s node doesn't have %s\n", np->name, prot);
		return -EINVAL;
	}
	return 0;
}

static int peri_dvs_per_channel_init(const struct device_node *np,
				     unsigned int chan)
{
	struct peri_dvs_channel *dvs_channel = NULL;
	int ret;

	dvs_channel = kzalloc(sizeof(*dvs_channel), GFP_KERNEL);
	if (!dvs_channel)
		return -ENOMEM;

	dvs_channel->channel = chan;
	ret = peri_dvs_channel_reg_dt_parse(np, dvs_channel);
	if (ret == -ENODEV) {
		kfree(dvs_channel);
		return 0;
	} else if (ret < 0) {
		goto fail;
	}

	if (chan >= g_dvs_cfg->max_vote_channel) {
		dvs_err("invalid channel %u\n", chan);
		ret = -EINVAL;
		goto fail;
	}
	g_dvs_channels[chan] = dvs_channel;
	INIT_LIST_HEAD(&dvs_channel->chan_list);

	dvs_channel->chan_valid = 0;
	dvs_channel->volt_level = PERI_VOLT_0;
	/* bypass complete irq to ACPU/LPMCU */
	pmc_writel(himask_enable(dvs_channel->offset[DVS_IRQ_CPT_BP_REG]),
		   dvs_channel->reg[DVS_IRQ_CPT_BP_REG]);
	return 0;
fail:
	kfree(dvs_channel);
	return ret;
}

static void peri_dvs_channel_deinit(void)
{
	unsigned int i;

	if (!g_dvs_cfg || !g_dvs_channels)
		return;

	for (i = 0; i < g_dvs_cfg->max_vote_channel; i++) {
		kfree(g_dvs_channels[i]);
		g_dvs_channels[i] = NULL;
	}
	kfree(g_dvs_channels);
	g_dvs_channels = NULL;
}

static int peri_dvs_channel_init(struct device *dev)
{
	const struct device_node *np = dev->of_node;
	int ssize = sizeof(struct peri_dvs_channel *);
	int max_chan_num = g_dvs_cfg->max_vote_channel;
	unsigned int i;
	int ret = 0;

	g_dvs_channels = kzalloc(ssize * max_chan_num, GFP_KERNEL);
	if (!g_dvs_channels)
		return -ENOMEM;

	for (i = 0; i < g_dvs_cfg->max_vote_channel; i++) {
		ret = peri_dvs_per_channel_init(np, i);
		if (ret < 0)
			goto fail;
	}
	return 0;

fail:
	peri_dvs_channel_deinit();
	return ret;
}

static void peri_dvs_deinit(void)
{
	if (g_peri_dvs_hwlock && hwspin_lock_free(g_peri_dvs_hwlock) < 0)
		dvs_err("free peri dvs hwlock failed\n");
	g_peri_dvs_hwlock = NULL;

	peri_dvs_vote_deinit();
	peri_dvs_channel_deinit();
	peri_dvs_mediabus_deinit();
	peri_dvs_config_deinit();
}

static int plat_peri_dvs_init(struct device *dev)
{
	struct device_node *parent = dev->of_node;
	struct device_node *np = NULL;
	int ret;

	g_peri_dvs_hwlock = hwspin_lock_request_specific(PERI_DVS_HWLOCK);
	if (!g_peri_dvs_hwlock) {
		dvs_err("peri dvs request hwspin lock failed\n");
		return -ENODEV;
	}

	ret = peri_dvs_config_init(dev);
	if (ret < 0) {
		dvs_err("peri dvs configure init failed! ret=%d\n", ret);
		goto fail;
	}

	ret = peri_dvs_mediabus_init(dev);
	if (ret < 0) {
		dvs_err("peri dvs mediabus init failed! ret=%d\n", ret);
		goto fail;
	}

	ret = peri_dvs_channel_init(dev);
	if (ret < 0) {
		dvs_err("peri dvs channels init failed! ret=%d\n", ret);
		goto fail;
	}

	for_each_child_of_node(parent, np) {
		ret = pvp_node_init(np);
		if (ret < 0)
			goto fail;
	}
	dvs_dbg("[%s] success\n", __func__);
#ifdef CONFIG_HISI_CLK_DEBUG
	peri_dvs_debug_init();
#endif
	return 0;

fail:
	peri_dvs_deinit();
	return ret;
}

static int peri_dvs_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;

	ret = plat_peri_dvs_init(dev);
	if (ret < 0)
		return ret;

	platform_set_drvdata(pdev, NULL);
	return 0;
}

static int peri_dvs_remove(struct platform_device *pdev)
{
	peri_dvs_deinit();
	return 0;
}

#ifdef CONFIG_HISI_CLK_DEBUG
static void peri_dvs_global_config_show(struct seq_file *s)
{
	int i;

	seq_puts(s, "\n<---- Show Global PeriDVS Configure START --->\n");
	seq_printf(s, "  Peri DVS ON/OFF     : %u\n",
		   g_dvs_cfg->peri_dvs_enabled);
	seq_printf(s, "  Peri CTRL5[0x%03x]   : 0x%08x\n",
		   g_dvs_cfg->acpu_offset, pmc_readl(g_dvs_cfg->acpu_offset));
	seq_printf(s, "  Peri CTRL4[0x%03x]   : 0x%08x\n",
		   g_dvs_cfg->lpmcu_offset, pmc_readl(g_dvs_cfg->lpmcu_offset));
	seq_printf(s, "  Low Temp Mask       : 0x%08x shift: %u\n",
		   g_dvs_cfg->lowtemp_mask, g_dvs_cfg->lowtemp_shift);
	seq_printf(s, "  Lock Volt Mask      : 0x%08x shift: %u\n",
		   g_dvs_cfg->fixed_volt_mask, g_dvs_cfg->fixed_volt_shift);
	seq_printf(s, "  Peri Max Volt Level : %u\n",
		   g_dvs_cfg->max_volt_level);
	seq_printf(s, "  Peri Max IRQ Wait   : %u us\n",
		   g_dvs_cfg->max_irq_usec);
	seq_printf(s, "  Fixed Volt Flag     : %u\n",
		   g_dvs_cfg->volt_level_fixed);
	for (i = 0; i < g_dvs_cfg->dump_reg_num; i++)
		seq_printf(s, "  Peri DVS Reg[0x%03x] : 0x%08x\n",
			   g_dvs_cfg->dump_reg[i],
			   pmc_readl(g_dvs_cfg->dump_reg[i]));
	seq_puts(s, "<---- Show Global PeriDVS Configure END --->\n");
}

static void peri_dvs_pdv_show(struct seq_file *s, struct peri_dvs_vote *pdv)
{
	if (!pdv)
		return;

	seq_puts(s, "    ------- PDV --------\n");
	seq_printf(s, "    Name         : %s\n", pdv->name);
	seq_printf(s, "    dev_id       : %u\n", pdv->dev_id);
	seq_printf(s, "    channel      : %u\n", pdv->channel);
	seq_printf(s, "    Valid        : %u\n", pdv->vote_valid);
	seq_printf(s, "    Volt         : %u\n", pdv->volt_level);
	seq_printf(s, "    Bus id       : %u\n", pdv->mediabus_id);
	seq_printf(s, "    valid sync   : %u\n", pdv->valid_vote_sync);
	seq_printf(s, "    sysbus sync  : %u\n", pdv->sysbus_sync_flag);
	seq_printf(s, "    record       : <%u 0x%03x 0x%08x>\n",
		pdv->record_reg_type, pdv->record_reg_offset, pdv->record_mask);
}

static void peri_dvs_mediabus_show(struct seq_file *s, unsigned int bus_id)
{
	struct peri_dvs_vote *pdv = NULL;
	struct peri_dvs_mediabus *bus = NULL;
	struct clk *clock = NULL;
	int i;

	if (bus_id >= MEDIABUS_MAX)
		return;
	bus = g_mediabus_cfg[bus_id];
	if (!bus) {
		seq_printf(s, "ERROR: PeriDVS Mediabus[%u] is NONE!\n", bus_id);
		return;
	}

	seq_printf(s, "\n<---- Show PeriDVS Mediabus[%u] State START --->\n",
		   bus->bus_type);
	seq_printf(s, "  Mediabus Vote Level : %u\n", bus->volt_level);
	seq_printf(s, "  Mediabus Vote Valid : %u\n", bus->valid);
	seq_printf(s, "  Mediabus LowTemp    : %u\n", bus->low_temp_profile);
	seq_puts(s, "  Mediabus Source PLL :\n");
	for (i = 0; i < PERI_VOLT_MAX; i++) {
		clock = bus->pll_clks[i];
		if (clock)
			seq_printf(s, "    [Level_%d] %12s (en_cnt:%u): %lu\n",
				   i, clock->core->name,
				   clock->core->enable_count,
				   clk_get_rate(clock));
	}
	seq_puts(s, "  Mediabus PDV lists  :\n");
	list_for_each_entry(pdv, &bus->mediabus_list, mediabus_node)
		peri_dvs_pdv_show(s, pdv);
	seq_printf(s, "<---- Show PeriDVS Mediabus[%u] State END --->\n",
		bus->bus_type);
}

static void peri_dvs_channel_state_show(struct seq_file *s, unsigned int chan)
{
	struct peri_dvs_channel *dvs_chan = NULL;
	struct peri_dvs_vote *pdv = NULL;
	int i;

	if (chan >= g_dvs_cfg->max_vote_channel)
		return;
	dvs_chan = g_dvs_channels[chan];
	if (!dvs_chan) {
		seq_printf(s, "ERROR: PeriDVS Channel[%u] is NONE!\n", chan);
		return;
	}

	seq_printf(s, "\n<---- Show PeriDVS Channel[%u] State START --->\n",
		dvs_chan->channel);
	seq_printf(s, "  volt Lvl          : %u\n", dvs_chan->volt_level);
	seq_printf(s, "  valid             : %u\n", dvs_chan->chan_valid);
	seq_puts(s, "  Channel Ctrl Regs :\n");
	for (i = 0; i < DVS_MAX_REG; i++)
		seq_printf(s, "    Reg[%d]: 0x%02x  Offset: %u\n",
			i, dvs_chan->reg[i], dvs_chan->offset[i]);
	seq_puts(s, "  Channel PDV lists :\n");
	list_for_each_entry(pdv, &dvs_chan->chan_list, chan_node)
		peri_dvs_pdv_show(s, pdv);
	seq_printf(s, "<---- Show PeriDVS Channel[%u] State END --->\n",
		dvs_chan->channel);
}

static void show_peri_dvs_info(struct seq_file *s)
{
	unsigned int bus, chan;

	if (!s || !g_dvs_cfg)
		return;

	peri_dvs_global_config_show(s);
	for (bus = MEDIABUS_VIVOBUS; bus < MEDIABUS_MAX; bus++)
		peri_dvs_mediabus_show(s, bus);
	for (chan = 0; chan < g_dvs_cfg->max_vote_channel; chan++)
		peri_dvs_channel_state_show(s, chan);
}

static int peri_dvs_summary_show(struct seq_file *s, void *data)
{
	show_peri_dvs_info(s);
	return 0;
}

static int peri_dvs_summary_open(struct inode *inode, struct file *file)
{
	return single_open(file, peri_dvs_summary_show, inode->i_private);
}

#ifdef CONFIG_HISI_DEBUG_FS
static const struct file_operations peri_dvs_summary_fops = {
	.open    = peri_dvs_summary_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
};
#endif /* CONFIG_HISI_DEBUG_FS */

void peri_dvs_debug_init(void)
{
#ifdef CONFIG_HISI_DEBUG_FS
	struct dentry *dvs_parent = NULL;
	struct dentry *dvs_info = NULL;

	dvs_parent = debugfs_create_dir("peri_dvs", NULL);
	if (!dvs_parent) {
		dvs_err("create peri_dvs debugfs dir failed\n");
		return;
	}

	dvs_info = debugfs_create_file("dvs_info", S_IRUGO, dvs_parent,
			NULL, &peri_dvs_summary_fops);
	if (!dvs_info) {
		dvs_err("create dvs_info debugfs file failed\n");
		return;
	}
#endif /* CONFIG_DEBUG_FS */
}
#endif /* CONFIG_CLK_DEBUG */

static const struct of_device_id peri_dvs_of_match[] = {
	{ .compatible = "hisilicon,soc-peri-dvfs" },
	{ /* end */ },
};
MODULE_DEVICE_TABLE(of, peri_dvs_of_match);

static struct platform_driver peri_dvs_driver = {
	.probe          = peri_dvs_probe,
	.remove         = peri_dvs_remove,
	.driver         = {
		.name   = "peridvs",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(peri_dvs_of_match),
	},
};

static int __init peri_volt_poll_init(void)
{
	return platform_driver_register(&peri_dvs_driver);
}
fs_initcall(peri_volt_poll_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HW Peri DVS functions");
