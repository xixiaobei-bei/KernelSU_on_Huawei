/*
 * ufs_hisi_test.c
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#define pr_fmt(fmt) "ufshcd :" fmt

#include <linux/delay.h>
#include <scsi/ufs/ufs.h>

#include "ufshci.h"
#include "ufshcd.h"
#include "ufs.h"
#include "ufs_test.h"

#define TRP_DBR_TIMEOUT 1000 /* uint: 10ms */
#define AUTO_H8_TIMEOUT 1000 /* uint: 1ms */
#define AUTO_H8_EXIT_TIMEOUT 50 /* uint: 1ms */

/* call this function before change ufs voltage for other module */
void ufs_change_vol_pre(void)
{
	uint32_t i;
	int retry;
	struct ufs_hba *hba = hba_addr;

	if (!hba)
		return;

	for (i = 0; i < CORE_NUM; i ++)
		ufshcd_writel(hba, 0, UFS_CORE_UTRLRSR(i));

	retry = TRP_DBR_TIMEOUT;
	while (retry--) {
		if (!ufshcd_readl(hba, TRP_MARB_DBR_NEW_DFX0) &&
		    !ufshcd_readl(hba, TRP_MARB_DBR_NEW_DFX0))
			break;
		msleep(10);
	}
	if (retry < 0)
		dev_err(hba->dev, "wait doorbell clear fail\n");

	retry = AUTO_H8_TIMEOUT;
	while(retry--) {
		if (ufshcd_readl(hba, UFS_AUTO_H8_STATE_OFF) == AH8_H8ING)
			break;
		msleep(1);
	}
	if (retry < 0)
		dev_err(hba->dev, "wait auto h8 enter fail\n");
}

/* call this function after change ufs voltage for other module */
void ufs_change_vol_post(void)
{
	uint32_t i;
	uint32_t val;
	int retry = AUTO_H8_EXIT_TIMEOUT;
	struct ufs_hba *hba = hba_addr;

	if (!hba)
		return;

	for (i = 0; i < CORE_NUM; i ++)
		ufshcd_writel(hba, 1, UFS_CORE_UTRLRSR(i));

	ufshcd_writel(hba, 1, UFS_AHIT_EXIT_REQ);
	while (retry--) {
		val = ufshcd_readl(hba, UFS_AUTO_H8_STATE_OFF) &
		      UFS_HC_AH8_STATE;
		if (val == AH8_XFER || val == AH8_IDLE)
			break;
		msleep(1);
	}
	if (retry < 0)
		dev_err(hba->dev, "wait auto h8 exit fail\n");
	ufshcd_writel(hba, 0, UFS_AHIT_EXIT_REQ);
}
