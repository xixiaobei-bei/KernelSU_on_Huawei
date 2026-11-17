/*
 * ufs_test.h
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

#ifndef _UFS_TEST_H_
#define _UFS_TEST_H_

#include "ufshci.h"
#include "ufshcd.h"
#include "ufs.h"

extern struct ufs_hba *hba_addr;
extern int ufshcd_map_sg(struct ufshcd_lrb *lrbp);
extern int ufshcd_send_command(struct ufs_hba *hba, unsigned int task_tag);
extern int ufshcd_compose_upiu(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
extern bool ufshcd_get_dev_cmd_tag(struct ufs_hba *hba, int *tag_out);
#endif /* _UNIPRO_H_ */
