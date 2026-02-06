/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: kbhub channel header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef _KBHUB_CHANNEL_H_
#define _KBHUB_CHANNEL_H_

#include <linux/types.h>

#include <huawei_platform/inputhub/kbhub.h>
#include <log/hiview_hievent.h>

#include "protocol.h"

#define KBHB_MCUREADY                      0xFD
#define KBHB_NFCINFO_REQ                   0x0B
#define KBHB_RECV_WRITE_NFC_RESULT         0x0C
#define KBHB_NFC_NEAR_OR_FAR               0x0D

#define KBHUB_REPORT_DATA_SIZE             64

#define KB_CONNECTED                       1
#define KB_NFC_WRITE_SUCCESS               1
#define KB_NFC_WRITE_FAIL                  0

enum kb_type_t {
	KB_TYPE_START = 0x0,
	KB_TYPE_UART_RUN,
	KB_TYPE_UART_STOP,
	KB_TYPE_UART_RESTART,
	KB_TYPE_END,
};

struct pkt_kb_data_req_t {
	struct pkt_header hd;
	enum kb_type_t cmd;
};

struct kb_outreport_t {
	unsigned int sub_cmd;
	int report_len;
	uint8_t report_data[KBHUB_REPORT_DATA_SIZE];
};

struct kb_cmd_map_t {
	int fhb_ioctl_app_cmd;
	int ca_type;
	int tag;
	enum obj_cmd cmd;
	enum obj_sub_cmd subcmd;
};

struct kbdev_proxy {
	struct kb_dev_ops *ops;
	int notify_event;
};

extern void disable_kb_when_sysreboot(void);

#endif /* _KBHUB_CHANNEL_H_ */
