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

#include "hisi_drm_drv.h"

int hisi_dss_mmbuf_init(void);
void hisi_dss_mmbuf_deinit(void);
uint32_t hisi_dss_mmbuf_alloc(uint32_t size);
void hisi_dss_mmbuf_free(uint32_t addr);

#endif  /* HISI_MIPI_DSI_H */
