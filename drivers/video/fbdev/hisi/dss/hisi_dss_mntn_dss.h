/* Copyright (c) 2019-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef _HISI_DSS_MNTN_DSS_H_
#define _HISI_DSS_MNTN_DSS_H_

#include "hisi_fb.h"
#ifdef CONFIG_HISI_NOC_MODID_REGISTER
#include "hisi_noc_modid_para.h"
#endif

#ifdef CONFIG_HISI_NOC_MODID_REGISTER
struct noc_err_para_s *dss_get_noc_err_para(uint32_t *noc_para_num);
#endif

int dss_check_media_subsys_status(struct dpu_fb_data_type *dpufd);
void dss_power_up(struct dpu_fb_data_type *dpufd);

#endif
