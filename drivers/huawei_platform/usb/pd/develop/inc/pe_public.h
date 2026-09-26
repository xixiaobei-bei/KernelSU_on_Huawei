/*
 * pe_state.h
 *
 * policy engine public interface
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
 *
 */

#ifndef _PE_PUBLIC_H_
#define _PE_PUBLIC_H_

enum pd_pe_state_machine {
	PE_STATE_MACHINE_IDLE = 0,
	PE_STATE_MACHINE_NORMAL,
	PE_STATE_MACHINE_DR_SWAP,
	PE_STATE_MACHINE_PR_SWAP,
	PE_STATE_MACHINE_VCONN_SWAP,
};

#define pe_state_discard_and_unexpected(pd_port) { \
	pd_port->pe_data.pe_state_flags = \
		PE_STATE_FLAG_HRESET_IF_SR_TIMEOUT | \
		PE_STATE_FLAG_IGNORE_UNKNOWN_EVENT; \
}

/* policy engine runtime flags */
#define PE_STATE_FLAG_BACK_READY_IF_RECV_WAIT      (1 << 0)
#define PE_STATE_FLAG_BACK_READY_IF_RECV_REJECT    (1 << 1)
#define PE_STATE_FLAG_BACK_READY_IF_SR_TIMER_TOUT  (1 << 2)
#define PE_STATE_FLAG_BACK_READY_IF_TX_FAILED      (1 << 3)
#define PE_STATE_FLAG_HRESET_IF_SR_TIMEOUT         (1 << 4)
#define PE_STATE_FLAG_HRESET_IF_TX_FAILED          (1 << 5)
#define PE_STATE_FLAG_IGNORE_UNKNOWN_EVENT         (1 << 6)
#define PE_STATE_FLAG_ENABLE_SENDER_RESPONSE_TIMER (1 << 7)

#define pe_state_wait_response(pd_port) { \
	pd_port->pe_data.pe_state_flags = \
		PE_STATE_FLAG_ENABLE_SENDER_RESPONSE_TIMER; \
}

#define pe_state_wait_msg(pd_port) { \
	pd_port->pe_data.pe_state_flags = \
		PE_STATE_FLAG_BACK_READY_IF_SR_TIMER_TOUT | \
		PE_STATE_FLAG_ENABLE_SENDER_RESPONSE_TIMER; \
}

#define pe_state_wait_msg_hreset_if_tout(pd_port) { \
	pd_port->pe_data.pe_state_flags = \
		PE_STATE_FLAG_HRESET_IF_SR_TIMEOUT | \
		PE_STATE_FLAG_ENABLE_SENDER_RESPONSE_TIMER; \
}

#define pe_state_wait_msg_or_tx_failed(pd_port) { \
	pd_port->pe_data.pe_state_flags = \
		PE_STATE_FLAG_BACK_READY_IF_TX_FAILED | \
		PE_STATE_FLAG_BACK_READY_IF_SR_TIMER_TOUT | \
		PE_STATE_FLAG_ENABLE_SENDER_RESPONSE_TIMER; \
}

#define pe_state_wait_msg_or_rj(pd_port) { \
	pd_port->pe_data.pe_state_flags = \
		PE_STATE_FLAG_BACK_READY_IF_RECV_REJECT | \
		PE_STATE_FLAG_BACK_READY_IF_SR_TIMER_TOUT | \
		PE_STATE_FLAG_ENABLE_SENDER_RESPONSE_TIMER; \
}

#define pe_state_wait_answer_msg(pd_port) { \
	pd_port->pe_data.pe_state_flags = \
		PE_STATE_FLAG_BACK_READY_IF_RECV_WAIT | \
		PE_STATE_FLAG_BACK_READY_IF_RECV_REJECT | \
		PE_STATE_FLAG_BACK_READY_IF_SR_TIMER_TOUT | \
		PE_STATE_FLAG_ENABLE_SENDER_RESPONSE_TIMER; \
}

#define pe_state_hreset_if_tx_failed(pd_port) { \
	pd_port->pe_data.pe_state_flags = \
		PE_STATE_FLAG_HRESET_IF_TX_FAILED; \
}

#define pe_state_ignore_unknown_event(pd_port) { \
	pd_port->pe_data.pe_state_flags = \
		PE_STATE_FLAG_IGNORE_UNKNOWN_EVENT; \
}

#define pe_state_recv_soft_reset(pd_port) { \
	pd_port->pe_data.pe_state_flags = \
		PE_STATE_FLAG_HRESET_IF_TX_FAILED | \
		PE_STATE_FLAG_IGNORE_UNKNOWN_EVENT; \
}

#define pe_state_send_soft_reset(pd_port) { \
	pd_port->pe_data.pe_state_flags = \
		PE_STATE_FLAG_HRESET_IF_TX_FAILED | \
		PE_STATE_FLAG_HRESET_IF_SR_TIMEOUT | \
		PE_STATE_FLAG_IGNORE_UNKNOWN_EVENT | \
		PE_STATE_FLAG_ENABLE_SENDER_RESPONSE_TIMER; \
}

#define PE_STATE_FLAG_BACK_READY_IF_RECV_GOOD_CRC  (1 << 0)
#define PE_STATE_FLAG_BACK_READY_IF_DPM_ACK        (1 << 1)
#define PE_STATE_FLAG_DPM_ACK_IMMEDIATELY          (1 << 7)

#define pe_state_wait_tx_success(pd_port) { \
	pd_port->pe_data.pe_state_flags2 = \
		PE_STATE_FLAG_BACK_READY_IF_RECV_GOOD_CRC; \
}

#define pe_state_dpm_informed(pd_port) { \
	pd_port->pe_data.pe_state_flags2 = \
		PE_STATE_FLAG_BACK_READY_IF_DPM_ACK | \
		PE_STATE_FLAG_DPM_ACK_IMMEDIATELY; \
}

#define pe_state_wait_dpm_ack(pd_port) { \
	pd_port->pe_data.pe_state_flags2 = \
		PE_STATE_FLAG_BACK_READY_IF_DPM_ACK; \
}

#define pe_state_dpm_ack_immediately(pd_port) { \
	pd_port->pe_data.pe_state_flags2 |= \
		PE_STATE_FLAG_DPM_ACK_IMMEDIATELY; \
}

#define VDM_STATE_FLAG_DPM_ACK_IMMEDIATELY         (1 << 4)
#define VDM_STATE_FLAG_BACK_READY_IF_DPM_ACK       (1 << 6)
#define VDM_STATE_FLAG_BACK_READY_IF_RECV_GOOD_CRC (1 << 7)

#define pe_vdm_state_dpm_informed(pd_port) { \
	pd_port->pe_data.vdm_state_flags = \
		VDM_STATE_FLAG_BACK_READY_IF_DPM_ACK | \
		VDM_STATE_FLAG_DPM_ACK_IMMEDIATELY; \
}

#define pe_vdm_state_reply_svdm_request(pd_port) { \
	pd_port->pe_data.vdm_state_flags = \
		VDM_STATE_FLAG_BACK_READY_IF_RECV_GOOD_CRC; \
}

#define pe_vdm_state_noresp_cmd(pd_port) { \
	pd_port->pe_data.vdm_state_flags = \
		VDM_STATE_FLAG_BACK_READY_IF_RECV_GOOD_CRC; \
}

struct pd_port;
struct tcpc_device;

int pe_run(struct tcpc_device *tcpc_dev);
void pe_state_init(struct pd_port *port);
bool pe_state_ready(struct pd_port *port);
bool pe_vdm_state_ready(struct pd_port *port);

#endif /* _PE_PUBLIC_H_ */
