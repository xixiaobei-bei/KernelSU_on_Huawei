/*
 * Hisilicon bsp acm interface declaration.
 *
 * Copyright (C) 2014 Hisilicon
 * Author: Hisilicon
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __BSP_ACM_H__
#define __BSP_ACM_H__

#include <linux/types.h>

typedef void (*USB_UDI_ENABLE_CB_T)(void);
typedef void (*USB_UDI_DISABLE_CB_T)(void);
unsigned int BSP_USB_RegUdiEnableCB(USB_UDI_ENABLE_CB_T pFunc);
unsigned int BSP_USB_RegUdiDisableCB(USB_UDI_DISABLE_CB_T pFunc);

/*
 * Modem API, DO NOT modify these functions!
 */
extern void *bsp_acm_open(u32 dev_id);
extern int bsp_acm_close(void *handle);
extern int bsp_acm_write(void *handle, void *buf, unsigned int size);
extern int bsp_acm_read(void *handle, void *buf, unsigned int size);
extern int bsp_acm_ioctl(void *handle, unsigned int cmd, void *para);

#endif /* __BSP_ACM_H__ */
