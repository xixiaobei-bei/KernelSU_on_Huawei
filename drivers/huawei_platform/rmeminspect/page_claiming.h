/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: ddr inspect reqeust page head file
 * Author: zhouyubin
 * Create: 2019-05-30
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef PAGE_CLAIMING_H
#define PAGE_CLAIMING_H

#include "phys_mem_int.h"

/* Implements the Request-Pages command */
int handle_request_pages(struct phys_mem_session *session,
				struct phys_mem_request *request);

int handle_mark_page_poison(struct phys_mem_session *session,
				const struct mark_page_poison *request);

/* The page claiming resultes. */
#define CLAIMED_SUCCESSFULLY			0
#define CLAIMED_TRY_NEXT			2
#define CLAIMED_ABORT				3

#define SIZE_IN_DDRINFO				8
#define PAGE_NUM_ONE_MB				256
/* The maxium MB of requesting section */
#define MAX_NUM_REQUEST				128

#endif /* PAGE_CLAIMING_H_ */
