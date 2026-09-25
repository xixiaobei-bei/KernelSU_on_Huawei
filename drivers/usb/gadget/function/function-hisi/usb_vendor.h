/*
 * usb_vendor.c
 *
 * Hisilicon usb notifier
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
#ifndef __USB_VENDOR_H__
#define __USB_VENDOR_H__

#include <linux/notifier.h>

/* Normaltive defination for usb acm, used by both balong_acm and hw_acm. */

/* used for cdc flow control */
#define USB_CDC_VENDOR_NTF_FLOW_CONTROL     0x01

/* interface descriptor for pnp2.1 */
#define HW_PNP21_CLASS       0xff
#define HW_PNP21_SUBCLASS    0x13

enum USB_PID_UNIFY_IF_PROT_T {
	USB_IF_PROTOCOL_VOID		= 0x00,
	USB_IF_PROTOCOL_3G_DIAG		= 0x03,
	USB_IF_PROTOCOL_3G_GPS		= 0x05,
	USB_IF_PROTOCOL_CTRL		= 0x06,
	USB_IF_PROTOCOL_BLUETOOTH	= 0x0A,
	USB_IF_PROTOCOL_MODEM		= 0x10,
	USB_IF_PROTOCOL_PCUI		= 0x12,
	USB_IF_PROTOCOL_DIAG		= 0x13,
	USB_IF_PROTOCOL_GPS		= 0x14,
	USB_IF_PROTOCOL_CDMA_LOG	= 0x17,
	USB_IF_PROTOCOL_SKYTONE		= 0x1e,
	USB_IF_PROTOCOL_HW_MODEM	= 0x21,
	USB_IF_PROTOCOL_HW_PCUI		= 0x22,
	USB_IF_PROTOCOL_HW_DIAG		= 0x23,
	USB_IF_PROTOCOL_CDROM		= 0xA1,
	USB_IF_PROTOCOL_SDRAM		= 0xA2,
	USB_IF_PROTOCOL_RNDIS		= 0xA3,

	USB_IF_PROTOCOL_NOPNP		= 0xFF
};

/* The usb_vendor declarations */

#define USB_NOTIF_PRIO_ADP		0       /* adp has lowest priority */
#define USB_NOTIF_PRIO_FD		100     /* function drvier */
#define USB_NOTIF_PRIO_CORE		200     /* usb core */
#define USB_NOTIF_PRIO_HAL		300     /* hardware has highest priority */

#define USB_BALONG_DEVICE_INSERT	1
#define USB_BALONG_CHARGER_IDEN		2
#define USB_BALONG_ENUM_DONE		3
#define USB_BALONG_PERIP_INSERT		4
#define USB_BALONG_PERIP_REMOVE		5
#define USB_BALONG_DEVICE_REMOVE	0
#define USB_BALONG_DEVICE_DISABLE	0xF1

/* notify interface */
void bsp_usb_register_notify(struct notifier_block *nb);
void bsp_usb_unregister_notify(struct notifier_block *nb);

/* usb status change interface */
void bsp_usb_status_change(int status);

/* usb enum done interface */
void bsp_usb_add_setup_dev_fdname(unsigned int intf_id, const char *fd_name);
void bsp_usb_remove_setup_dev_fdname(void);
void bsp_usb_set_enum_stat(unsigned int intf_id, int enum_stat);

#endif /* __USB_VENDOR_H__ */
