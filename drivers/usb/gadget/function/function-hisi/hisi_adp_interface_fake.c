/*
 * hisi_adp_interface_fake.c
 *
 * Bsp ACM function
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */


#include <linux/module.h>


typedef void (*USB_ENABLE_CB_T)(void);
typedef void (*USB_DISABLE_CB_T)(void);


__attribute__((weak)) void *bsp_acm_open(unsigned int dev_id)
{
	return NULL;
}

__attribute__((weak)) int bsp_acm_close(void *handle)
{
	return -1;
}

__attribute__((weak)) int bsp_acm_write(void *handle, void *buf, unsigned int size)
{
	return -1;
}

__attribute__((weak)) int bsp_acm_read(void *handle, void *buf, unsigned int size)
{
	return -1;
}

__attribute__((weak)) int bsp_acm_ioctl(void *handle, unsigned int cmd, void *para)
{
	return -1;
}

__attribute__((weak)) unsigned int BSP_USB_RegUdiEnableCB(USB_ENABLE_CB_T pFunc)
{
	return (unsigned int)-1;
}

__attribute__((weak)) unsigned int BSP_USB_RegUdiDisableCB(USB_DISABLE_CB_T pFunc)
{
	return (unsigned int)-1;
}

