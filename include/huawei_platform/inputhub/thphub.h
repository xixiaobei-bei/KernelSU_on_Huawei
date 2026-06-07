/*
 * thphub.h
 *
 * thphub channel driver
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

#ifndef __LINUX_THPHUB_H__
#define __LINUX_THPHUB_H__
#include <linux/ioctl.h>

/* ioctl cmd define */
#define THPIO					0xB1

#define THP_IOCTL_THP_DEBUG_CMD			_IOWR(THPIO, 0x10, short)
#define THP_IOCTL_THP_CONFIG_CMD		_IOWR(THPIO, 0x11, short)
#define THP_IOCTL_THP_OPEN_CMD			_IOWR(THPIO, 0x12, short)
#define THP_IOCTL_THP_CLOSE_CMD			_IOWR(THPIO, 0x13, short)
#define THP_IOCTL_THP_STATUS_CMD		_IOWR(THPIO, 0x14, short)

#define THP_DEBUG_CMD_PARA_LEN 16
#define THP_STATUS_CMD_PARA_LEN 116
#define THP_ALGO_SYNC_BUFF_LEN 128
#define THP_CMD_TYPE_LEN 4

#define THP_IS_INVALID_BUFF(para, len, maxlen) \
	((len < 0) || ((len > 0) && (para == NULL)) || ((len > maxlen)))

/* tpd debug cmd */
enum {
	CMD_TYPE_SET_HOVER_UP,
	CMD_TYPE_SET_HOVER_DOWN,
	CMD_TYPE_SET_FINGER_UP,
	CMD_TYPE_SET_FINGER_DOWN,
	CMD_TYPE_SET_PRESSURE_LV1,
	CMD_TYPE_SET_PRESSURE_LV2,
	CMD_TYPE_SET_PRESSURE_LV3,
	CMD_TYPE_SET_FINGER_RELEASE,
	CMD_TYPE_SET_ENTER_IDLE,
	CMD_TYPE_SET_ENTER_TUI,
	CMD_TYPE_SET_EXIT_TUI,
	CMD_TYPE_SET_LOG_ENABLE,
	CMD_TYPE_SET_LOG_DISABLE,
	CMD_TYPE_MAX
};

enum thp_st_cmd {
	ST_CMD_TYPE_SET_TUI_STATUS,
	ST_CMD_TYPE_SET_CHARGING_STATUS,
	ST_CMD_TYPE_SET_SCREEN_STATUS,
	ST_CMD_TYPE_SET_TPIC_IRQ,
	ST_CMD_TYPE_FINGERPRINT_EVENT,
	ST_CMD_TYPE_RING_EVENT,
	ST_CMD_TYPE_ALGO_SYNC_EVENT,
	ST_CMD_TYPE_ALGO_SYNC_EVENT_SHMEM,
	ST_CMD_TYPE_SET_RECOVER_STATUS,
	ST_CMD_TYPE_SET_AUDIO_STATUS,
	ST_CMD_TYPE_MAX
};

struct thp_ioctl_para {
	/* first uint8_t is detail cmd */
	uint8_t cmd_para[THP_DEBUG_CMD_PARA_LEN];
	int outbuf_len;
	char *outbuf;
};

enum thp_poweroff_status_enum {
	/* bit0:double click on:1,double click off:0 */
	THP_DOUBLE_CLICK_ON = 0,
	/* bit1:tp ud on:1,tp ud off:0 */
	THP_TPUD_ON = 1,
	/* bit2:pen mode on :1,pen mode off :0 */
	THP_PEN_MODE_ON = 2,
	/* bit3:ring support:1 */
	THP_RING_SUPPORT = 3,
	/* bit4:ring switch:1 */
	THP_RING_ON = 4,
	/* bit5:on the phone:1 */
	THP_PHONE_STATUS = 5,
	/* bit6:single click on:1,single click off:0 */
	THP_SINGLE_CLICK_ON = 6,
	/* bit7:volume bar on the left side flag:1 */
	THP_VOLUME_SIDE_STATUS_LEFT = 7,
	/* bit8:volume bar on the right side flag:1 */
	THP_VOLUME_SIDE_STATUS_RIGHT = 8,
};

struct format_data_head_s {
	int16_t err_no;
	uint16_t col;
	uint16_t row;
};

typedef struct thp_status_manage {
	uint8_t tui_mode;
	uint8_t screen_state;
	unsigned int support_mode;
} thp_status_data;

enum thp_state {
	TP_SWITCH_OFF = 0,
	TP_SWITCH_ON = 1,
	TP_ON_THE_PHONE = 2,
	TP_SWITCH_BOTTOM,
};

int send_thp_open_cmd(void);
int send_thp_ap_event(int inpara_len, const char *inpara, uint8_t cmd);
/* thp sync status info for tp driver */
int send_thp_driver_status_cmd(unsigned char power_on,
	unsigned int para, unsigned char cmd);
int send_thp_algo_sync_event(int inpara_len, const char *inpara);
int send_thp_auxiliary_data(unsigned int inpara_len, const char *inpara);

#endif /* __LINUX_THPHUB_H__ */

