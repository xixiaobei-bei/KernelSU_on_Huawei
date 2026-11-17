/*
 * HISI Secure Digital Host Controller Interface.
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>

#include "sdhci-pltfm.h"
#include "sdhci_hisi_mmc.h"

void sdhci_hisi_hardware_reset(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;

	if (!host->hsdt1crg) {
		pr_err("%s: hsdt1 is null, can't reset mmc!\n", __func__);
		return;
	}

	switch (sdhci_hisi->sdhci_host_id) {
	case SDHCI_SD_ID:
		/* sd reset */
		sdhci_hsdt1crg_writel(host, IP_RST_SD | IP_HRST_SD, PERRSTEN0);
		udelay(100);
		break;
	case SDHCI_SDIO_ID:
		/* sdio reset */
		sdhci_hsdt1crg_writel(host, IP_RST_SDIO | IP_HRST_SDIO, PERRSTEN1);
		udelay(100);
	case SDHCI_EMMC_ID:
	default:
		break;
	}
}

void sdhci_hisi_hardware_disreset(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_hisi_data *sdhci_hisi = pltfm_host->priv;

	/* sd dis-reset */
	if (!host->hsdt1crg) {
		pr_err("%s: hsdt1 is null, can't reset mmc!\n", __func__);
		return;
	}

	switch (sdhci_hisi->sdhci_host_id) {
	case SDHCI_SD_ID:
		/* sd disreset */
		sdhci_hsdt1crg_writel(host, IP_RST_SD | IP_HRST_SD, PERRSTDIS0);
		udelay(100);
		break;
	case SDHCI_SDIO_ID:
		/* sdio disreset */
		sdhci_hsdt1crg_writel(host, IP_RST_SDIO | IP_HRST_SDIO, PERRSTDIS1);
		udelay(100);
	case SDHCI_EMMC_ID:
	default:
		break;
	}

}

int sdhci_hisi_get_resource(struct platform_device *pdev, struct sdhci_host *host)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,hsdt1_crg");
	if (!np) {
		pr_err("can't find hsdt1 crg!\n");
		return -1;
	}

	host->hsdt1crg = of_iomap(np, 0);
	if (!host->hsdt1crg) {
		pr_err("hsdt1crg iomap error!\n");
		return -1;
	}

	return 0;
}

