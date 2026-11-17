/*
 * ufs-hisi-hci.h
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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
 * Description: This file is used to put functions, struct, macro from
 * hufs controller, interactive with linux ufs driver.
 *
 */

#ifndef HUFS_HCI_H_
#define HUFS_HCI_H_

#include "ufshcd.h"

/* VS intr bytes */
#define AH8_ENTER_REQ_CNF_FAIL_INTR UFS_BIT(13)
#define AH8_EXIT_REQ_CNF_FAIL_INTR UFS_BIT(14)
#define UFS_RX_CPORT_TIMEOUT_INTR UFS_BIT(15)

/* ufs error hba->errors expand bit */
#define UFS_CORE_IO_TIMEOUT UFS_BIT(31)


/* Unipro intr bytes */
#define HSH8ENT_LR_INTR UFS_BIT(15)
#define LOCAL_ATTR_FAIL_INTR UFS_BIT(22)

#ifdef CONFIG_HISI_UFS_HC
irqreturn_t ufshcd_hufs_vs_intr(struct ufs_hba *hba);
void ufshcd_hufs_is_intr_inject(struct ufs_hba *hba, u32 *intr_status);
#else
static inline irqreturn_t ufshcd_hufs_vs_intr(struct ufs_hba *hba)
{
	return IRQ_NONE;
}

static inline void ufshcd_hufs_is_intr_inject(struct ufs_hba *hba,
					      u32 *intr_status)
{
}
#endif /* End of CONFIG_HISI_UFS_HC */


#endif
