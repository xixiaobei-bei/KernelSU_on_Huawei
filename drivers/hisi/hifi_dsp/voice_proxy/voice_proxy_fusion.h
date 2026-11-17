/*
 * voice_proxy_fusion.h - HW voice proxy fusion in kernel, it is used for pass
 * through voice data between audio hal and hifi for fusion(wifi or cell).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */
#ifndef __VOICE_PROXY_FUSION_H__
#define __VOICE_PROXY_FUSION_H__

#include <linux/types.h>

#define VPEER_VOICE_RTP_MAX_DATA_LEN 256
#define FUSION_VOICE_MAX_DATA_LEN 252
#define MAX_DUMP_BUFFER_SIZE 500

/* ID_PROXY_VOICE_FUSIONCALL_CTRL_IND */
struct proxy_fusion_voice_ntf {
	uint16_t msg_name; /* voice_proxy_fusioncall_msg_id */
	uint16_t reserve1;
	uint32_t msg_len;
	uint8_t msg[4];
};

/* ID_VOICE_PROXY_VPEER_CTRL_CNF */
struct voice_proxy_fusion_cnf {
	uint16_t msg_name; /* voice_proxy_fusioncall_msg_id */
	uint16_t reserve1;
	uint32_t result;
};

/* ID_FUSIONCALL_VOICE_ECHO_NTF */
struct fusioncall_voice_echo_ntf {
	uint16_t msg_id;
	uint16_t ver; /* start from 1 */
	uint32_t ts;
	uint32_t feature_fusioncall :1;
	uint32_t reserved2 :31;
};

/* ID_VOICE_FUSIONCALL_ECHO_CNF */
struct voice_fusioncall_echo_cnf {
	uint16_t msg_id;
	uint16_t ver;
	uint32_t ts;
	uint8_t reason_code;
	uint8_t revser[3];
	uint32_t feature_fusioncall :1; /* bit for each feature */
	uint32_t reserved2 :31;
};
/*ID_FUSIONCALL_VOICE_RTCP_ENABLE_IND/ID_FUSIONCALL_VOICE_RTCP_DISABLE_IND */
struct fusioncall_voice_rtcp_able_ind {
	uint16_t msg_id;
	uint16_t reserved1;
	uint32_t channel_id;
	uint32_t reserved2;
};

/* ID_FUSIONCALL_VOICE_TX_RTCP_IND */
struct fusioncall_voice_tx_rtcp_ind {
	uint16_t msg_id;
	uint16_t reserved;
	uint32_t channel_id;
	uint32_t data_len; /* 0: hisi use default rtcp */
	uint8_t data[FUSION_VOICE_MAX_DATA_LEN];
};

/* ID_VOICE_FUSIONCALL_RX_RTCP_IND */
struct voice_fusioncall_rx_rtcp_ind {
	uint16_t msg_id;
	uint16_t reserved;
	uint32_t channel_id;
	uint32_t local_ssrc;
	uint32_t remote_ssrc;
	uint32_t data_len;
	uint8_t data[FUSION_VOICE_MAX_DATA_LEN];
};

/* ID_VPEER_VOICE_RX_RTP_IND */
struct vpeer_voice_rx_rtp_ntf {
	uint16_t msg_id;
	uint16_t reserved1;
	uint32_t channel_id;
	uint32_t port_type; /* Port Type: 0 RTP; 1 RTCP */
	uint16_t data_len;
	uint16_t data_seq;
	uint8_t frag_seq;
	uint8_t frag_max;
	uint16_t data_type;
	uint8_t data[VPEER_VOICE_RTP_MAX_DATA_LEN];
	unsigned int reserve[2];
};

/* ID_VOICE_VPEER_TX_RTP_IND */
struct voice_vpeer_tx_rtp_ntf {
	uint16_t msg_id;
	uint16_t reserved1;
	uint32_t channel_id;
	uint32_t port_type; /* Port Type: 0 RTP; 1 RTCP */
	uint16_t data_len;
	uint16_t data_seq;
	uint8_t frag_seq;
	uint8_t frag_max;
	uint16_t data_type;
	uint8_t data[VPEER_VOICE_RTP_MAX_DATA_LEN];
};

/* fusioncall defined here */
enum voice_proxy_fusioncall_msg_id {
	ID_PROXY_VOICE_FUSIONCALL_CTRL_IND = 0xDF4E, /* FusionCall->HIFI */
	ID_VOICE_PROXY_FUSIONCALL_CTRL_IND = 0xDF4F, /* HIFI->FusionCall */
	ID_PROXY_VOICE_VPEER_CTRL_NTF = 0xDF50, /* VPEER->HIFI, rx data */
	ID_VOICE_PROXY_VPEER_CTRL_CNF = 0xDF51, /* HIFI->VoiceProxy rx data cnf */
	ID_VOICE_PROXY_VPEER_CTRL_NTF = 0xDF52, /* HIFI->VPEER tx data */
};

enum voice_proxy_fusioncall_sub_msg_id {
	ID_FUSIONCALL_VOICE_ECHO_NTF = 0xDF46, /* FusionCall->hifi ECHO */
	ID_VOICE_FUSIONCALL_ECHO_CNF = 0xDF47, /* hifi->FusionCall ECHO CNF */
	ID_FUSIONCALL_VOICE_RTCP_ENABLE_IND = 0xDF48, /* FusionCall->HIFI */
	ID_FUSIONCALL_VOICE_RTCP_DISABLE_IND = 0xDF49, /* FusionCall->HIFI */
	ID_VOICE_FUSIONCALL_RX_RTCP_IND = 0xDF4A, /* HIFI->FusionCall */
	ID_FUSIONCALL_VOICE_TX_RTCP_IND = 0xDF4B, /* FusionCall->HIFI */
	ID_VPEER_VOICE_RX_RTP_IND = 0xDF4C, /* VPEER->HIFI RX RTP */
	ID_VOICE_VPEER_TX_RTP_IND = 0xDF4D, /* HIFI-VPEER TX RTP */
};

#endif /* __VOICE_PROXY_FUSION_H__ */
