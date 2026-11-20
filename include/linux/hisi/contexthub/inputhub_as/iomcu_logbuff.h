/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019. All rights reserved.
 * Description: iomcu_logbuff.h.
 * Create: 2019/11/05
 */

#ifndef __IOMCU_LOGBUFF_H__
#define __IOMCU_LOGBUFF_H__

#include <linux/hisi/contexthub/protocol_as.h>

#define EMG_LEVEL   0
#define ERR_LEVEL   1
#define WARN_LEVEL  2
#define INFO_LEVEL  3
#define DEBUG_LEVEL 4

typedef struct {
	struct pkt_header hd;
	uint32_t index;
} log_buff_req_t;

extern int set_log_level(int tag, int argv[], int argc);

#endif
