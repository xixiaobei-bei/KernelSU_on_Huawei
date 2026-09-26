/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Normaltive defination for usb acm, used by modem.
 * Create: 2019-06-23
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
#ifndef _HW_USB_PNP21_H_
#define _HW_USB_PNP21_H_

/* vendor defined notification type, used for cdc flow control */
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

#endif /* _HW_USB_PNP21_H_ */
