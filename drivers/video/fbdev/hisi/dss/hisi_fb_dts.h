/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#ifndef HISI_FB_DTS_H
#define HISI_FB_DTS_H


/* dpufb dts */
int hisi_fb_read_property_from_dts(struct device_node *np, struct device *dev);
int hisi_fb_get_irq_no_from_dts(struct device_node *np, struct device *dev);
int hisi_fb_get_baseaddr_from_dts(struct device_node *np, struct device *dev);
int hisi_fb_get_clk_name_from_dts(struct device_node *np, struct device *dev);
void hisi_fb_read_logo_buffer_from_dts(struct device_node *np, struct device *dev);
int dpufb_init_irq(struct dpu_fb_data_type *dpufd, uint32_t type);
void dpufb_init_base_addr(struct dpu_fb_data_type *dpufd);
void dpufb_init_clk_name(struct dpu_fb_data_type *dpufd);

#endif  /* HISI_FB_DTS_H */

