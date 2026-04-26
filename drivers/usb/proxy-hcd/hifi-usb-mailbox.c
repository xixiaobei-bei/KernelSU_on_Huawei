/*
 * hifi-usb-mailbox.c
 *
 * utilityies for hifi-usb sending message
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#include "hifi-usb.h"
#include <linux/errno.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/usb/hifi-usb-mailbox.h>
#include "../dwc3/hisi/dwc3-hifi-usb.h"

#define HIFI_USB_MBX     HISI_ACPU_HIFI_MBX_1

static void usb_notify_recv_isr(const void *usr_para,
		struct mb_queue *mail_handle, unsigned int mail_len)
{
	struct hifi_usb_op_msg mail_buf;
	unsigned int ret;
	unsigned int mail_size = mail_len;

	if ((mail_len == 0) || (mail_len > sizeof(struct hifi_usb_op_msg)))
		return;

	memset(&mail_buf, 0, sizeof(mail_buf));

	ret = DRV_MAILBOX_READMAILDATA(mail_handle, (unsigned char *)&mail_buf, &mail_size);
	if ((ret != 0) || (mail_size == 0) ||
		(mail_size > sizeof(struct hifi_usb_op_msg))) {
		pr_err("Empty point or data length error! size: %u ret:%u max size:%lu\n",
			mail_size, ret, sizeof(struct hifi_usb_op_msg));
		return;
	}

#ifdef CONFIG_USB_DWC3_NYET_ABNORMAL
	ap_use_hifi_usb_msg_receiver((void *)&mail_buf);
#endif

	hifi_usb_msg_receiver(&mail_buf);
}

static int usb_notify_isr_register(mb_msg_cb pisr)
{
	int ret = 0;
	unsigned int mailbox_ret;

	pr_info("register isr for usb channel !\n");
	if (pisr == NULL) {
		pr_err("pisr==NULL!\n");
		ret = -EINVAL;
	} else {
		/*
		 * Micro "MAILBOX_MAILCODE_HIFI_TO_ACPU_USB" defined in
		 * drivers/hisi/hifi_mailbox/mailbox/drv_mailbox_cfg.h
		 */
		mailbox_ret = DRV_MAILBOX_REGISTERRECVFUNC(
				MAILBOX_MAILCODE_HIFI_TO_ACPU_USB,
				pisr, NULL);
		if (mailbox_ret != 0) {
			ret = -ENOENT;
			pr_err("register isr for usb channel failed, ret:%d,0x%x\n",
				ret, MAILBOX_MAILCODE_HIFI_TO_ACPU_USB);
		}
	}

	return ret;
}

int hifi_usb_send_mailbox(const struct hifi_usb_op_msg *op_msg, unsigned int len)
{
	int count = 100; /* maximum timeout times */

	while (RPROC_IS_SUSPEND(HIFI_USB_MBX) && count) {
		msleep(10);
		pr_err("[%s] mailbox is suspend,need wait\n", __func__);
		count--;
	}

	if (count == 0) {
		pr_err("[%s] mailbox is always suspend\n", __func__);
		return -ESHUTDOWN;
	}

	/*
	 * Micro "MAILBOX_MAILCODE_ACPU_TO_HIFI_USB_RT" defined in
	 * drivers/hisi/hifi_mailbox/mailbox/drv_mailbox_cfg.h
	 */
	return mailbox_send_msg(MAILBOX_MAILCODE_ACPU_TO_HIFI_USB_RT,
		op_msg, len);
}

int hifi_usb_mailbox_init(void)
{
	int ret;

	pr_info("%s\n", __func__);

	/* register mailbox recv isr */
	ret = usb_notify_isr_register(usb_notify_recv_isr);
	if (ret)
		pr_err("usb_notify_isr_register failed : %d\n", ret);

	return ret;
}

void hifi_usb_mailbox_exit(void)
{
}
