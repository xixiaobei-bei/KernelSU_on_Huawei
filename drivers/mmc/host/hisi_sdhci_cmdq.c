
#include <linux/delay.h>

#include <linux/mmc/mmc.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include "sdhci.h"
#include <linux/mmc/cmdq_hci.h>
#include <linux/hisi/util.h>
#include <linux/pm_runtime.h>


extern void sdhci_dumpregs(struct sdhci_host *host);

#ifdef CONFIG_MMC_CQ_HCI
void sdhci_cmdq_reset(struct mmc_host *mmc, u8 mask)
{
	struct sdhci_host *host = mmc_priv(mmc);
	sdhci_reset(host, mask);
}

static void sdhci_cmdq_clean_irqs(struct mmc_host *mmc, u32 clean)
{
	struct sdhci_host *host = mmc_priv(mmc);
	sdhci_writel(host, clean, SDHCI_INT_STATUS);
}

static u32 sdhci_cmdq_clear_set_irqs(struct mmc_host *mmc, u32 clear, u32 set)
{
	struct sdhci_host *host = mmc_priv(mmc);
	u32 val, ret;
#if 0
	host->ier = SDHCI_INT_CMDQ_EN | SDHCI_INT_ERROR_MASK;
	sdhci_writel(host, host->ier, SDHCI_INT_ENABLE);
	sdhci_writel(host, host->ier, SDHCI_SIGNAL_ENABLE);
#else
	ret = sdhci_readl(host, SDHCI_INT_ENABLE);
	val = ret & ~clear;
	val |= set;
	sdhci_writel(host, val,
			SDHCI_INT_ENABLE);
	sdhci_writel(host, val,
			SDHCI_SIGNAL_ENABLE);
#endif
	return ret;
}

static int sdhci_cmdq_discard_task(struct mmc_host *mmc, u32 tag, bool entire)
{
	struct sdhci_host *host = mmc_priv(mmc);
	u32 mask, arg, opcode, val, val1, val2, mode;
	int flags;
	unsigned long timeout;

	sdhci_reset(host, SDHCI_RESET_CMD | SDHCI_RESET_DATA);

	timeout = 10;
	mask = SDHCI_CMD_INHIBIT;

	while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask) {
		if (timeout == 0) {
			pr_err("%s: sdhci never released "
				"inhibit bit(s).\n", __func__);
			sdhci_dumpregs(host);
			return -EIO;
		}
		timeout--;
		mdelay(1);
	}

	/* set trans mode reg */
	mode = sdhci_readw(host, SDHCI_TRANSFER_MODE);
	if (host->quirks2 & SDHCI_QUIRK2_CLEAR_TRANSFERMODE_REG_BEFORE_CMD) {
		sdhci_writew(host, 0x0, SDHCI_TRANSFER_MODE);
	} else {
		/* clear Auto CMD settings for no data CMDs */
		val = mode & ~(SDHCI_TRNS_AUTO_CMD12 | SDHCI_TRNS_AUTO_CMD23);
		sdhci_writew(host, val, SDHCI_TRANSFER_MODE);
	}

	/* enable interupt status, */
	/* but not let the interupt be indicated to system */
	val1 = sdhci_readl(host, SDHCI_INT_ENABLE);
	val = val1 | SDHCI_INT_RESPONSE | SDHCI_INT_ERROR_MASK;
	sdhci_writel(host, val, SDHCI_INT_ENABLE);
	val2 = sdhci_readl(host, SDHCI_SIGNAL_ENABLE);
	val = val2 & ~(SDHCI_INT_RESPONSE | SDHCI_INT_ERROR_MASK);
	sdhci_writel(host, val, SDHCI_SIGNAL_ENABLE);

	/* send cmd12 */
	arg = 0;
	sdhci_writel(host, arg, SDHCI_ARGUMENT);

	opcode = MMC_STOP_TRANSMISSION;
	flags = SDHCI_CMD_RESP_SHORT | SDHCI_CMD_CRC |
			SDHCI_CMD_INDEX | SDHCI_CMD_ABORTCMD;
	sdhci_writew(host, SDHCI_MAKE_CMD(opcode, flags), SDHCI_COMMAND);

	timeout = 10;
	mask = SDHCI_INT_RESPONSE | SDHCI_INT_ERROR_MASK;

	while (0 == (mask & (val = sdhci_readl(host, SDHCI_INT_STATUS)))) {
		if (timeout == 0) {
			pr_err("%s: send cmd%d timeout\n", __func__, opcode);
			sdhci_dumpregs(host);
			break;
		}
		timeout--;
		mdelay(1);
	}

	if (val & SDHCI_INT_ERROR_MASK) {
		pr_err("%s: send cmd%d err val = 0x%x\n", __func__,opcode,  val);
		sdhci_dumpregs(host);
	}
	/* clean interupt*/
	sdhci_writel(host, val, SDHCI_INT_STATUS);

	/* wait busy */
	timeout = 1000;
	while (host->cq_host->ops->card_busy(mmc)) {
		if (timeout == 0) {
			pr_err("%s: CMD12 wait busy timeout after stop\n", __func__);
			sdhci_dumpregs(host);
			break;
		}
		timeout--;
		mdelay(1);
	}

	sdhci_reset(host, SDHCI_RESET_CMD | SDHCI_RESET_DATA);
	/* send cmd48 */
	/* CMD48 arg:
	[31:21] reserved
	[20:16]: TaskID
	[15:4]: reserved
	[3:0] TM op-code
	*/
	if (true == entire)
		arg = 1;
	else
		arg = 2 | tag << 16;
	sdhci_writel(host, arg, SDHCI_ARGUMENT);

	opcode = MMC_CMDQ_TASK_MGMT;
	flags = SDHCI_CMD_RESP_SHORT | SDHCI_CMD_CRC | SDHCI_CMD_INDEX;
	sdhci_writew(host, SDHCI_MAKE_CMD(opcode, flags), SDHCI_COMMAND);

	timeout = 10;
	mask = SDHCI_INT_RESPONSE | SDHCI_INT_ERROR_MASK;

	while (0 == (mask & (val = sdhci_readl(host, SDHCI_INT_STATUS)))) {
		if (timeout == 0) {
			pr_err("%s: send cmd%d timeout\n", __func__, opcode);
			sdhci_dumpregs(host);
			break;
		}
		timeout--;
		mdelay(1);
	}

	if (val & SDHCI_INT_ERROR_MASK) {
		pr_err("%s: send cmd%d err val = 0x%x\n", __func__, opcode, val);
		sdhci_dumpregs(host);
	}
	/* clean interupt*/
	sdhci_writel(host, val, SDHCI_INT_STATUS);

	/* recovery interupt enable & mask */
	sdhci_writel(host, val1, SDHCI_INT_ENABLE);
	sdhci_writel(host, val2, SDHCI_SIGNAL_ENABLE);
	/* recovery trans mode */
	sdhci_writew(host, mode, SDHCI_TRANSFER_MODE);

	if (val & SDHCI_INT_RESPONSE) {
		return 0;
	} else {
		pr_err("%s: discard cmdq fail\n", __func__);
		return -EIO;
	}
}

static int sdhci_cmdq_tuning_move(struct mmc_host *mmc, int is_move_strobe, int flag)
{
	struct sdhci_host *host = mmc_priv(mmc);

	return host->ops->tuning_move(host, is_move_strobe, flag);
}

static void sdhci_cmdq_set_data_timeout(struct mmc_host *mmc, u32 val)
{
	struct sdhci_host *host = mmc_priv(mmc);

	sdhci_writeb(host, val, SDHCI_TIMEOUT_CONTROL);
}

static void sdhci_cmdq_dump_vendor_regs(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);

	sdhci_dumpregs(host);
}

#else
void sdhci_cmdq_reset(struct mmc_host *mmc, u8 mask)
{

}

static void sdhci_cmdq_clean_irqs(struct mmc_host *mmc, u32 clean)
{

}

static u32 sdhci_cmdq_clear_set_irqs(struct mmc_host *mmc, u32 clear, u32 set)
{
	return 0;
}

static int sdhci_cmdq_discard_task(struct mmc_host *mmc, u32 tag, bool entire)
{
	return 0;
}

static int sdhci_cmdq_tuning_move(struct mmc_host *mmc, int is_move_strobe, int flag)
{
	return 0;
}

static void sdhci_cmdq_set_data_timeout(struct mmc_host *mmc, u32 val)
{

}

static void sdhci_cmdq_dump_vendor_regs(struct mmc_host *mmc)
{

}

#endif

extern int sdhci_card_busy_data0(struct mmc_host *mmc);


static const struct cmdq_host_ops sdhci_cmdq_ops = {
	.reset = sdhci_cmdq_reset,
	.clean_irqs = sdhci_cmdq_clean_irqs,
	.clear_set_irqs = sdhci_cmdq_clear_set_irqs,
	.set_data_timeout = sdhci_cmdq_set_data_timeout,
	.dump_vendor_regs = sdhci_cmdq_dump_vendor_regs,
	.card_busy = sdhci_card_busy_data0,
	.discard_task = sdhci_cmdq_discard_task,
	.tuning_move = sdhci_cmdq_tuning_move,
};


#ifdef CONFIG_MMC_CQ_HCI
irqreturn_t sdhci_cmdq_irq(struct mmc_host *mmc, u32 intmask)
{
	return cmdq_irq(mmc, intmask);
}

#else
static irqreturn_t sdhci_cmdq_irq(struct mmc_host *mmc, u32 intmask)
{
	pr_err("%s: rxd cmdq-irq when disabled !!!!\n", mmc_hostname(mmc));
	return IRQ_NONE;
}
#endif

void sdhci_cmdq_init(struct sdhci_host *host, struct mmc_host *mmc)
{
		bool dma64;
		int ret;

		dma64 = (host->flags & SDHCI_USE_64_BIT_DMA) ?
			true : false;
		ret = cmdq_init(host->cq_host, mmc, dma64);
		if (ret)
			pr_err("%s: CMDQ init: failed (%d)\n",
			       mmc_hostname(host->mmc), ret);
		else
			host->cq_host->ops = &sdhci_cmdq_ops;
}