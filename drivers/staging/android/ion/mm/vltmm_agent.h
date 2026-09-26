/* Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * FileName: vltmm_agent.h
 * Description: This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 * either version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _VLTMM_AGENT_H
#define _VLTMM_AGENT_H

enum {
	VLTMM_CMD_ALLOC,
	VLTMM_CMD_FREE
};

struct vltmm_msg_t {
	uint32_t  magic;
	uint32_t  cmd;
	int32_t   ret;
	uint32_t   cid;
	uint32_t   num;
	uint32_t   pad;
	uint64_t  data[0];
};

#define VLTMM_BUFF_LEN PAGE_SIZE

int vltmm_agent_register(void);

#endif
