/*
 * drv_mailbox_msg.h
 *
 * this file defines mailbox interfaces
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef __DRV_MAILBOX_MSG_H__
#define __DRV_MAILBOX_MSG_H__

#include "drv_mailbox_cfg.h"

/* Generate remote call initiation function */
#define IFC_GEN_CALL0(id, name) _IFC_GEN_CALLx(0, id, name)
#define IFC_GEN_CALL1(id, name, ...) _IFC_GEN_CALLx(1, id, name, __VA_ARGS__)
#define IFC_GEN_CALL2(id, name, ...) _IFC_GEN_CALLx(2, id, name, __VA_ARGS__)
#define IFC_GEN_CALL3(id, name, ...) _IFC_GEN_CALLx(3, id, name, __VA_ARGS__)
#define IFC_GEN_CALL4(id, name, ...) _IFC_GEN_CALLx(4, id, name, __VA_ARGS__)
#define IFC_GEN_CALL5(id, name, ...) _IFC_GEN_CALLx(5, id, name, __VA_ARGS__)
#define IFC_GEN_CALL6(id, name, ...) _IFC_GEN_CALLx(6, id, name, __VA_ARGS__)

/* Generate remote call execution function with various parameters */
#define IFC_GEN_EXEC0(name) __IFC_GEN_EXECx(0, name)
#define IFC_GEN_EXEC1(name, ...) __IFC_GEN_EXECx(1, name, __VA_ARGS__)
#define IFC_GEN_EXEC2(name, ...) __IFC_GEN_EXECx(2, name, __VA_ARGS__)
#define IFC_GEN_EXEC3(name, ...) __IFC_GEN_EXECx(3, name, __VA_ARGS__)
#define IFC_GEN_EXEC4(name, ...) __IFC_GEN_EXECx(4, name, __VA_ARGS__)
#define IFC_GEN_EXEC5(name, ...) __IFC_GEN_EXECx(5, name, __VA_ARGS__)
#define IFC_GEN_EXEC6(name, ...) __IFC_GEN_EXECx(6, name, __VA_ARGS__)

/* Register remote call execution function */
#define mailbox_ifc_register(id, name) \
	mailbox_ifc_register_exec(id, IFC_GEN_EXEC_NAME(name))

#ifdef CONFIG_HIFI_DSP_ONE_TRACK
bool is_dsp_img_loaded(void);
#endif

/* Initialize IPC mailbox, called by the system during initialization */
int mailbox_init(void);

void mailbox_destory(void);

/* Mail users send mail data */
uint32_t mailbox_send_msg(uint32_t mailcode, const void *data,
	uint32_t length);

/* Register mail data receiving function */
int mailbox_reg_msg_cb(uint32_t mailcode, mb_msg_cb func, void *data);

/*
 * The mailbox user call in the data receiving callback function,
 * to read an email that arrives first from the mailbox
 */
uint32_t mailbox_read_msg_data(struct mb_queue *mail_handle, char *buff,
	uint32_t *size);

#endif /* __DRV_MAILBOX_MSG_H__ */
