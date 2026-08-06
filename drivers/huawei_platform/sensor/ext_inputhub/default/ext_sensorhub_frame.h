/*
 * ext_sensorhub_frame.h
 *
 * head file for make external sensorhub frame
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

#ifndef EXT_SENSORHUB_FRAME_H
#define EXT_SENSORHUB_FRAME_H

#include <linux/types.h>

#include "ext_sensorhub_commu.h"

int max_send_data_len(void);

#define COMMU_READ_WAKELOCK_TIMEOUT_MS 800
#define MAX_FRAME_BUF_LEN (4 * 1024) /* for buffer len, lager than data len */
#define MAX_FRAME_COUNT 13
#define FRAME_BUF_LEN (MAX_SEND_BUFFER_LEN + 9) /* with header */
#define MAX_SEND_BUFFER_LEN (max_send_data_len())
#define SDIO_MAX_SEND_BUFFER_LEN 3063
/* spi */
#define SPI_SEND_LEN 1024
#define DEFAULT_SPI_READ_LEN 1024
#define SPI_READ_LEN 240

#define SPI_MAX_SEND_BUFFER_LEN (SPI_FRAME_BUF_LEN - 9)
/* spi each send begins with 0x80, frame buffer length will count down */
#define SPI_SEND_DATA_LEN 239
#define SPI_FRAME_BUF_LEN (SPI_SEND_LEN - (SPI_SEND_LEN / SPI_SEND_DATA_LEN + \
(SPI_SEND_LEN % SPI_SEND_DATA_LEN == 0 ? 0 : 1)))
#define SPI_MAX_SEND_BUFFER_LEN   (SPI_FRAME_BUF_LEN - 9)

enum {
	/* hand shake service id */
	SERVICE_ID_COMMU = 0x90,
};

enum {
	/* hand shake req */
	SERVICE_MCU_HANDSHAKE_REQ = 0x06,
	/* upgrade command */
	SERVICE_UPGRADE_1132 = 0x0A,

	SERVICE_FORCE_UPGRADE_MCU = 0x0B,

	SERVICE_RELAX_WAKE_LOCK = 0x0C,

	/* commu common command */
	SERVICE_COMMU_COMMON = 0x09,
};

/* tran_id */
enum {
	TRANSACTION_ID_AP_TO_MCU = 0x21, /* dst is MCU, src is AP */
	TRANSACTION_ID_AP_TO_BT = 0x31, /* dst is BT, src is AP */
	TRANSACTION_ID_MCU_TO_AP = 0x12, /* dst is AP, src is MCU */
	TRANSACTION_ID_AP_IS_DST = 0x10, /* dst is AP */
};

#define FRAME_TRAN_ID_LEN  1
/* sof */
#define START_OF_FRAME 0x5A
/* frame header struct begin */
struct frame_tran_header {
	unsigned char tran_id;
	unsigned char sof;
	unsigned char frame_length_msb;
	unsigned char frame_length_lsb;
};

struct frame_ctrl_header {
	unsigned char control;
};

struct frame_fsn_header {
	unsigned char fsn;
};

#define FRAME_FSN_CTL 0x03
/* control bit0 - bit1 four kinds */
enum {
	FRAME_CTL_NONE   = 0x00,
	FRAME_CTL_FIRST  = 0x01,
	FRAME_CTL_DOING = 0x02,
	FRAME_CTL_END	= 0x03,
};

struct frame_opcode_header {
	unsigned char service_id;
	unsigned char command_id;
};

struct frame_crc16_header {
	unsigned char crc16_msb;
	unsigned char crc16_lsb;
};

#define FRAME_TRAN_HEADER_LEN	  sizeof(struct frame_tran_header)
#define FRAME_CTRL_HEADER_LEN	  sizeof(struct frame_ctrl_header)
#define FRAME_FSN_HEADER_LEN	   sizeof(struct frame_fsn_header)
#define FRAME_OPCODE_HEADER_LEN	sizeof(struct frame_opcode_header)
#define FRAME_CRC16_HEADER_LEN	 sizeof(struct frame_crc16_header)

#define FRAME_HEADER_LEN \
	(FRAME_TRAN_HEADER_LEN + FRAME_CTRL_HEADER_LEN + \
		FRAME_FSN_HEADER_LEN + FRAME_OPCODE_HEADER_LEN + \
		FRAME_CRC16_HEADER_LEN)
#define FRAME_HEADER_WITHOUT_FSN_LEN \
	(FRAME_TRAN_HEADER_LEN + FRAME_CTRL_HEADER_LEN + \
		FRAME_OPCODE_HEADER_LEN + FRAME_CRC16_HEADER_LEN)

/* pyload max length of split frame, with fsn.
 * multi frames except first frame,can send more FRAME_OPCODE_HEADER_LEN data
 */
#define SPLIT_FRAME_MAX_PAYLOAD_LEN \
	(FRAME_BUF_LEN - FRAME_HEADER_LEN + FRAME_OPCODE_HEADER_LEN)

/* frame length calculate body max length; from control to payload */
#define FRAME_LEN_BODY_MAX_LEN	\
	(FRAME_BUF_LEN - FRAME_TRAN_HEADER_LEN - FRAME_CRC16_HEADER_LEN)
/* frame header struct end */

struct frame_content {
	unsigned char frame_body[MAX_FRAME_BUF_LEN];
	/* frame length field */
	unsigned short frame_length;
	unsigned short frame_total_length;
};

struct frame_request {
	struct frame_content *frames;
	unsigned char frame_count;
	unsigned char service_id;
	unsigned char command_id;
};

struct frame_decode_data {
	unsigned char control;
	unsigned char service_id;
	unsigned char command_id;
	unsigned short frame_length;
	unsigned int payload_length;
	unsigned char payload[MAX_FRAME_BUF_LEN];
};

struct resp_buf_list {
	struct list_head list;
	unsigned char	service_id;
	unsigned char	command_id;
	unsigned int	 read_cnt;
	unsigned char	*buffer;
};

#define ext_sensorhub_get_min(x, y) ((x) < (y) ? (x) : (y))

int decode_frame_resp(unsigned char *read_buf,
		      unsigned short read_bufsz, enum commu_driver driver);

int send_frame_data(unsigned char tid, unsigned char service_id,
		    unsigned char command_id, unsigned char *payload_buf,
		    int payload_bufsz);

int ext_sensorhub_frame_init(void);

void ext_sensorhub_frame_exit(void);

/* for ext sensorhub debug node */
int send_debug_frame(unsigned char *payload_buf, int payload_bufsz);
/* for ext sensorhub debug node */
int mock_debug_frame(unsigned char *payload_buf, int payload_bufsz);

void notify_mcu_reboot(enum commu_driver driver);

void notify_commu_wakeup(bool is_wakeup, enum commu_driver driver);

int get_commu_mode(void);

typedef int (*debug_frame_handler)(unsigned char *buf, unsigned short bufsz,
				   enum commu_driver driver);

bool check_commu_handshake(unsigned char tid);


int ext_sensorhub_frame_close_uart(void);

void relax_commu_wake_lock(void);

void hold_commu_wake_lock(unsigned int msec);

void notify_mcu_force_upgrade_irq(void);

void notify_mcu_spi_irq(unsigned char *payload_buf, int payload_bufsz);

#endif /* EXT_SENSORHUB_FRAME_H */
