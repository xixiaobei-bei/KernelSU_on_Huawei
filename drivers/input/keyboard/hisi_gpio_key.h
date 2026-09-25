/*
 * volum key driver
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef HISI_GPIO_KEY_H_
#define HISI_GPIO_KEY_H_

typedef int (*init_process_func)(void *init_ctx);
typedef int (*init_rollback_func)(void *init_ctx, int cur_idx, int fail_idx);

struct hisi_init_processor {
	init_process_func init;
	init_rollback_func rollback;
};

#define MAX_INIT_FLOW_CNT 512
int hisi_exec_init_flow(const struct hisi_init_processor *init_flow,
	int flow_size, void *init_ctx);

#endif
