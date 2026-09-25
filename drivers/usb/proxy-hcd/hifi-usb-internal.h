/*
 * hisi-usb-internal.h
 *
 * utilityies for hifi-usb
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _HIFI_USB_INTERNAL_H_
#define _HIFI_USB_INTERNAL_H_

#include "hifi-usb.h"

void hifi_usb_wait_for_free_dev(struct hifi_usb_proxy *proxy);
void hifi_usb_port_disconnect(struct hifi_usb_proxy *proxy);

#endif /* _HIFI_USB_INTERNAL_H_ */
