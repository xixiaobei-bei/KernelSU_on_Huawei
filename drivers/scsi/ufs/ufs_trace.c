/*
 * ufs_trace.c
 *
 * ufs device error debug
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "ufs_trace.h"
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <mntn_subtype_exception.h>

void ufs_rdr_hardware_err(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	if (strstr(saved_command_line, "androidboot.swtype=factory")) {
		dev_err(hba->dev, "%s ufs device hardware error sense key:%d occurs\n",
			__func__, lrbp->ucd_rsp_ptr->sr.sense_data[2] & 0xf);
		rdr_syserr_process_for_ap((u32)RDR_MODID_UFSDEV_PANIC, 0ull, 0ull);
	}
	return;
}

static struct rdr_exception_info_s ufs_einfo[] = {
	{{0, 0}, RDR_MODID_UFSDEV_PANIC, RDR_MODID_UFSDEV_PANIC, RDR_ERR,
	 RDR_REBOOT_NOW, RDR_AP, RDR_AP, RDR_AP,
	 (u32)RDR_REENTRANT_DISALLOW, (u32)MMC_S_EXCEPTION, MMC_UFSDEV_HARDERR, (u32)RDR_UPLOAD_YES,
	 "RDR_UFS", "RDR_UFS", 0, 0, 0},
};

static int ufs_trace_rdr_register_exception(void)
{
	u32 ret;

	ret = rdr_register_exception(ufs_einfo);
	if (ret != RDR_MODID_UFSDEV_PANIC) {
		pr_err("register exception faild [0x%x]\n", ufs_einfo->e_modid);
		return -1;
	}

	return 0;
}

void ufs_trace_fd_init(void)
{
	if (ufs_trace_rdr_register_exception() != 0) {
		pr_err("%s:ufs trace rdr register exception fail!\n", __func__);
		return;
	}
}

