/*
 * hisi_usb_phy_chip_efuse.h
 *
 * Hisilicon Hi6502 efuse interface defination
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __HISI_USB_PHY_EFUSE_H__
#define __HISI_USB_PHY_EFUSE_H__

#if defined(CONFIG_HISI_USB_PHY_EFUSE)
int hisi_usb_phy_chip_get_dieid(char *buf, unsigned int buf_len);
s32 get_upc_efuse_value(u32 start_bit, u32 *buf, u32 bit_cnt);
s32 set_upc_efuse_value(u32 start_bit, u32 *buf, u32 bit_cnt);
#else
static inline int hisi_usb_phy_chip_get_dieid(char *buf, unsigned int buf_len)
{
	return -1;
}
static inline s32 get_upc_efuse_value(u32 start_bit, u32 *buf, u32 bit_cnt)
{
	return -1;
}
static inline s32 set_upc_efuse_value(u32 start_bit, u32 *buf, u32 bit_cnt)
{
	return -1;
}
#endif /* CONFIG_HISI_USB_PHY_EFUSE */

#endif /* __HISI_USB_PHY_EFUSE_H__ */
