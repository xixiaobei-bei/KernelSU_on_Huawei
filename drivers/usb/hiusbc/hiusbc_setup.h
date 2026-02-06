/*
 * hiusbc_setup.h -- Device Mode Header File for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __HIUSBC_REQ_H
#define __HIUSBC_REQ_H

#include <linux/usb/ch9.h>
#include "hiusbc_core.h"

int hiusbc_handle_std_req(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup);
int hiusbc_req_set_test_mode(struct hiusbc *hiusbc, u8 mode);

#endif /* __HIUSBC_REQ_H */
