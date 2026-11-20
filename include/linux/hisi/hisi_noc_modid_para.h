/*
 *
 * noc modid register.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __DFX_NOC_MODID_PARA_H__
#define __DFX_NOC_MODID_PARA_H__

#include <noc_modid_para.h>

struct noc_err_para_s {
	u32 masterid;
	u32 targetflow;
	enum noc_error_bus bus;
};

struct noc_mid_modid_trans_s {
	struct list_head s_list;
	struct noc_err_para_s err_info_para;
	u32 modid;
	void *reserve_p;
};
void noc_modid_register(struct noc_err_para_s noc_err_info, u32 modid);

#endif
