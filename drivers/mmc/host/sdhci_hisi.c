/*
 * flush reduce for eMMC
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/version.h>

#include "sdhci.h"

/*
 * sdhci_trylock_hostlock - try to claim the sdhci host lock;
 * @host: sdhci host;
 * @flags: store the flags for restore;
 * try to claim the sdhci host lock;
 */
static int sdhci_trylock_hostlock(struct sdhci_host *host, unsigned long *flags)
{
	int locked;
	unsigned int trycount = 100000;/* wait host lock 1s */

	do {
		locked = spin_trylock_irqsave(&host->lock, *flags);
		if (locked)
			break;
		udelay(10);
	} while (--trycount > 0);
	return locked;
}

/*
 * sdhci_send_command_direct - send cmd diretly without irq system;
 * @mmc: mmc host;
 * @mrq: mmc request;
 * sometimes the irq system cannot work.so we send cmd and polling the
 * int status register instead of waiting for the response interrupt;
 */
int sdhci_send_command_direct(struct mmc_host *mmc, struct mmc_request *mrq)
{
	u32 val, val1, val2, timeout, mask, opcode;
	int ret = 0;
	struct sdhci_host *host = NULL;
	unsigned long flags;

	host = mmc_priv(mmc);
	WARN_ON(host->cmd != NULL);
	opcode = mrq->cmd->opcode;

	/* Get hostlock timeout, the abnormal context may have the locker */
	if (!sdhci_trylock_hostlock(host, &flags)) {
		pr_err("%s, can't get the hostlock!\n", __func__);
		ret = -EIO;
		goto out;
	}

	mask = SDHCI_INT_ERROR_MASK;
	val = sdhci_readl(host, SDHCI_INT_STATUS);
	if (mask & val) {
		pr_err("%s host is in err or busy status 0x%x\n", __func__, val);
		ret = -EIO;
		goto unlock;
	}

	/*
	 * enable interupt status,
	 * but not let the interupt be indicated to system
	 */
	val1 = sdhci_readl(host, SDHCI_INT_ENABLE);
	val = val1 | SDHCI_INT_RESPONSE | SDHCI_INT_ERROR_MASK;
	sdhci_writel(host, val, SDHCI_INT_ENABLE);
	val2 = sdhci_readl(host, SDHCI_SIGNAL_ENABLE);
	sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);

	sdhci_send_command(host, mrq->cmd);
	/* wait cmd timeout busy 10ms */
	timeout = 10;
	mask = SDHCI_INT_RESPONSE | SDHCI_INT_ERROR_MASK;
	while (!(mask & sdhci_readl(host, SDHCI_INT_STATUS))) {
		if (!timeout) {
			pr_err("%s: send cmd%u timeout\n", __func__, opcode);
			sdhci_dumpregs(host);
			ret = -ETIMEDOUT;
			goto reg_recovery;
		}
		timeout--;
		mdelay(1);
	}

	val = sdhci_readl(host, SDHCI_INT_STATUS);
	if (val & SDHCI_INT_ERROR_MASK) {
		pr_err("%s: send cmd%u err val = 0x%x\n", __func__, opcode, val);
		sdhci_dumpregs(host);
		ret = -EIO;
		goto reg_recovery;
	}

	/* check data0 busy 1s */
	timeout = 1000;
	while (sdhci_card_busy_data0(mmc)) {
		if (!timeout) {
			pr_err("%s: wait busy timeout after stop\n", __func__);
			sdhci_dumpregs(host);
			ret = -ETIMEDOUT;
			goto reg_recovery;
		}
		timeout--;
		mdelay(1);
	}

	if (!ret)
		host->cmd->resp[0] = sdhci_readl(host, SDHCI_RESPONSE);
reg_recovery:
	/* clean interupt, cmdq int cannot be cleaned */
	val &= (~SDHCI_INT_CMDQ);
	sdhci_writel(host, val, SDHCI_INT_STATUS);
	/* recovery interupt enable & mask */
	sdhci_writel(host, val1, SDHCI_INT_ENABLE);
	sdhci_writel(host, val2, SDHCI_SIGNAL_ENABLE);
unlock:
	spin_unlock_irqrestore(&host->lock, flags);
out:
	host->cmd = NULL;

	return ret;
}

void sdhci_retry_req(struct sdhci_host *host, struct mmc_request *mrq)
{
	int ret;

	if (host->flags & SDHCI_WITHOUT_TUNING_MOVE)
		return;

	if (host->mmc->card && mmc_card_mmc(host->mmc->card) &&
		((mrq->cmd && mrq->cmd->error) ||
		(mrq->sbc && mrq->sbc->error) ||
		(mrq->data && (mrq->data->error ||
		(mrq->data->stop && mrq->data->stop->error)))) &&
		(!((unsigned int)(host->flags) & SDHCI_EXE_SOFT_TUNING)) &&
		(host->mmc->ios.timing >= MMC_TIMING_MMC_HS200) &&
		host->ops->tuning_move) {
		if ((mrq->data && mrq->data->error &&
			    (host->mmc->ios.timing == MMC_TIMING_MMC_HS400)) ||
			host->mmc->card->ext_csd.strobe_enhanced_en)
			ret = host->ops->tuning_move(host, TUNING_STROBE, TUNING_FLAG_NOUSE);
		else
			ret = host->ops->tuning_move(host, TUNING_CLK, TUNING_FLAG_NOUSE);

		/* cmd5 no need to retry */
		if ((!ret) && mrq->cmd && (mrq->cmd->opcode != MMC_SLEEP_AWAKE)) {
			mrq->cmd->retries++;
			if (mrq->cmd->data && mrq->cmd->data->error)
				mrq->cmd->data->error = 0;

			if (mrq->sbc && mrq->sbc->error)
				mrq->sbc->error = 0;

			if (mrq->cmd->data && mrq->cmd->data->stop &&
				mrq->cmd->data->stop->error)
				mrq->cmd->data->stop->error = 0;

			if (!mrq->cmd->error)
				mrq->cmd->error = -EILSEQ;
		}
	}
}

/* In Libra, do not do vcc power up an off operation in first exception */
int need_vcc_off(struct mmc_host *mmc)
{
	int vcc_off = 1;

	if (mmc->is_coldboot_on_reset_fail) {
		if (mmc->reset_num == 1)
			vcc_off = 0;
	}

	return vcc_off;
}

void sdhci_set_vmmc_power(struct sdhci_host *host, unsigned short vdd)
{
	struct mmc_host *mmc = host->mmc;

	if (!need_vcc_off(mmc))
		return;

	if  (!(host->quirks2 & SDHCI_QUIRK2_USE_1_8_V_VMMC)) {
		mmc_regulator_set_ocr(mmc, mmc->supply.vmmc, vdd);
		return;
	}

	if (!vdd) {
		if (mmc->regulator_enabled) {
			if (!regulator_disable(mmc->supply.vmmc))
				mmc->regulator_enabled = false;
			else
				pr_err("%s: regulator vmmc disable failed !\n", __func__);
		}
		return;
	}

	/* supply vmmc 2.95v */
	if (regulator_set_voltage(mmc->supply.vmmc, 2950000, 2950000))
		pr_err("%s: regulator vmmc set_voltage failed !\n", __func__);

	if (!mmc->regulator_enabled) {
		if (!regulator_enable(mmc->supply.vmmc))
			mmc->regulator_enabled = true;
		else
			pr_err("%s: regulator vmmc enable failed !\n", __func__);
	}
}
