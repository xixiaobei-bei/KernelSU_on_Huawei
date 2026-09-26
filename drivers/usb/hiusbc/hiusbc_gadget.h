/*
 * hiusbc_gadget.h -- Device Mode Header File for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __HIUSBC_GADGET_H
#define __HIUSBC_GADGET_H

#include "hiusbc_core.h"

static inline struct hiusbc_ep *to_hiusbc_ep(struct usb_ep *ep)
{
	return container_of(ep, struct hiusbc_ep, ep);
}

static inline struct hiusbc *to_hiusbc(struct usb_gadget *gadget)
{
	return container_of(gadget, struct hiusbc, gadget);
}

static inline struct hiusbc_req *to_hiusbc_req(struct usb_request *r)
{
	return container_of(r, struct hiusbc_req, req);
}

int hiusbc_get_frame_id(const struct hiusbc *hiusbc);
void hiusbc_set_ux_exit(struct hiusbc *hiusbc);
enum usb_device_speed hiusbc_get_link_speed(
	const struct hiusbc *hiusbc);
enum hiusbc_link_state hiusbc_get_link_state(
	const struct hiusbc *hiusbc, enum usb_device_speed speed);
void hiusbc_set_link_state(const struct hiusbc *hiusbc,
	enum usb_device_speed speed, enum hiusbc_link_state state);
int hiusbc_send_cmd_set_addr(struct hiusbc *hiusbc, u8 addr);
int hiusbc_send_cmd_clear_halt(struct hiusbc_ep *hep);
int hiusbc_send_cmd_start_transfer(struct hiusbc_ep *hep,
	u32 start_mf, dma_addr_t buffer_ptr, u8 dcs, u8 init, u8 ctrl_num);
int hiusbc_send_cmd_end_transfer(
	struct hiusbc_ep *hep, bool wait_event);
struct usb_request *hiusbc_gadget_ep_alloc_request(
	struct usb_ep *ep, gfp_t gfp_flags);
void hiusbc_gadget_ep_free_request(
	struct usb_ep *ep, struct usb_request *req);
int hiusbc_set_ep_stall(struct hiusbc_ep *hep, bool set);
int hiusbc_wakeup(const struct hiusbc *hiusbc);
int hiusbc_reinit_interrupt(struct hiusbc *hiusbc);
int hiusbc_enable_ctrl_eps(struct hiusbc *hiusbc);
int hiusbc_disable_ctrl_eps(struct hiusbc *hiusbc);
int hiusbc_run_stop(struct hiusbc *hiusbc, int is_on);
int hiusbc_restart(struct hiusbc *hiusbc);
void hiusbc_stop(struct hiusbc *hiusbc);
int hiusbc_gadget_delegate_req(struct hiusbc *hiusbc,
			const struct usb_ctrlrequest *setup);
void hiusbc_gadget_disconnect(struct hiusbc *hiusbc);
void hiusbc_gadget_suspend(struct hiusbc *hiusbc);
void hiusbc_gadget_resume(struct hiusbc *hiusbc);
void hiusbc_gadget_reset(struct hiusbc *hiusbc);
void hiusbc_gadget_exit_interrupt(struct hiusbc *hiusbc);
int hiusbc_gadget_init(struct hiusbc *hiusbc);
void hiusbc_gadget_exit(struct hiusbc *hiusbc);

#endif /* __HIUSBC_GADGET_H */
