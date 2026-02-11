/* page_claiming.h
 *
 * function definations required for Implements the Request-Pages command
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
 */

#ifndef PAGE_CLAIMING_H
#define PAGE_CLAIMING_H

#include "phys_mem_int.h"

/* Implements the Request-Pages command */
int handle_request_pages(struct phys_mem_session *session,
	struct phys_mem_request *request);

int handle_mark_page_poison(struct phys_mem_session *session,
	const struct mark_page_poison *request);

/* The page claiming resultes.*/
#define CLAIMED_SUCCESSFULLY			1
#define CLAIMED_TRY_NEXT			2
#define CLAIMED_ABORT				3

#endif /* PAGE_CLAIMING_H_ */
