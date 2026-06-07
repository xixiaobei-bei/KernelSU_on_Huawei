/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#ifndef HISI_MMBUF_MANAGER_H
#define HISI_MMBUF_MANAGER_H

#include "hisi_fb.h"

/*******************************************************************************
**  mmbuf manager struct config
*/
extern struct gen_pool *g_mmbuf_gen_pool;
extern uint32_t mmbuf_max_size;
extern dss_mmbuf_t dss_mmbuf_reserved_info[RESERVED_SERVICE_MAX];
extern struct dss_comm_mmbuf_info g_primary_online_mmbuf[DSS_CHN_MAX_DEFINE];
extern struct dss_comm_mmbuf_info g_external_online_mmbuf[DSS_CHN_MAX_DEFINE];

void hisi_mmbuf_info_init(void);
int hisi_mmbuf_reserved_size_query(struct fb_info *info, void __user *argp);
int hisi_mmbuf_request(struct fb_info *info, void __user *argp);
int hisi_mmbuf_release(struct fb_info *info, const void __user *argp);
int hisi_mmbuf_free_all(struct fb_info *info, const void __user *argp);
void* hisi_dss_mmbuf_init(struct hisi_fb_data_type *hisifd);
void hisi_dss_mmbuf_deinit(struct hisi_fb_data_type *hisifd);
uint32_t hisi_dss_mmbuf_alloc(void *handle, uint32_t size);
void hisi_dss_mmbuf_free(void *handle, uint32_t addr, uint32_t size);
void hisi_dss_mmbuf_info_clear(struct hisi_fb_data_type *hisifd, int idx);
dss_mmbuf_info_t* hisi_dss_mmbuf_info_get(struct hisi_fb_data_type *hisifd, int idx);
void hisi_mmbuf_info_get_online(struct hisi_fb_data_type *hisifd);
void hisi_dss_mmbuf_free_all(struct hisi_fb_data_type *hisifd);
void hisi_dss_check_use_comm_mmbuf(uint32_t display_id,
int *use_comm_mmbuf, dss_mmbuf_t *offline_mmbuf, bool has_rot);
int hisi_dss_rdma_set_mmbuf_base_and_size(struct hisi_fb_data_type *hisifd,
dss_layer_t *layer,int ovl_idx,dss_rect_ltrb_t *afbc_rect,uint32_t *mm_base_0,uint32_t *mm_base_1);
void hisi_dss_mmbuf_on(struct hisi_fb_data_type *hisifd);

void hisi_mmbuf_sem_pend(void);
void hisi_mmbuf_sem_post(void);

#endif  /* HISI_MIPI_DSI_H */
