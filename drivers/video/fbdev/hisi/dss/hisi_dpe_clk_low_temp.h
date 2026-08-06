/* Copyright (c) 2021-2021, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef HISI_DPE_CLK_LOW_TEMP_H
#define HISI_DPE_CLK_LOW_TEMP_H

bool check_low_temperature(struct dpu_fb_data_type *dpufd);
void dpufb_low_temperature_clk_protect(struct dpu_fb_data_type *dpufd);
void dpufb_low_temperature_clk_restore(struct dpu_fb_data_type *dpufd);
bool is_clk_match_low_temp_cstr(struct dpu_fb_data_type *dpufd, uint64_t pri_clk_rate);

#endif /* HISI_DPE_CLK_LOW_TEMP_H */
