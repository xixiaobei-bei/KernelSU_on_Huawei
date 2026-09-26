/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: provide Macro and struct for jpeg.
 * Author: zhuchunyu
 * Create: 2016-04-01
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __HJPEG_INTF_H__
#define __HJPEG_INTF_H__

struct _tag_hjpeg_vtbl;
typedef struct _tag_hjpeg_intf {
	struct _tag_hjpeg_vtbl *vtbl;
} hjpeg_intf_t;

typedef struct _tag_hjpeg_vtbl {
	int (*get_name)(hjpeg_intf_t *i);
	int (*encode_process)(hjpeg_intf_t *i, void *cfg);
	int (*power_on)(hjpeg_intf_t *i);
	int (*power_down)(hjpeg_intf_t *i);
} hjpeg_vtbl_t;

extern int hjpeg_register(struct platform_device *pdev, hjpeg_intf_t *si);

extern void hjpeg_unregister(struct platform_device *pdev);

#endif /* __HJPEG_INTF_H__ */