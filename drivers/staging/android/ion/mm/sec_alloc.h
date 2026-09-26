/* Copyright (c) Hisilicon Technologies Co., Ltd. 2001-2019. All rights reserved.
 * FileName: ion_sec_sg.h
 * Description: This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 * either version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _ION_SEC_ALLOC_H
#define _ION_SEC_ALLOC_H

extern void sec_alloc_init(struct platform_device *pdev);
extern struct cma *get_svc_cma(int id);
extern void set_svc_cma(u64 svc_id, struct cma *cma);
#endif
