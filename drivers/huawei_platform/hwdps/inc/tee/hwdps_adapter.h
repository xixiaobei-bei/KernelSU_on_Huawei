/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function definations required for
 *             operations about hwdps_adapter
 * Create: 2020-10-10
 */

#ifndef _HWDPS_ADAPTER_H
#define _HWDPS_ADAPTER_H
#include <linux/types.h>
#include "inc/base/hwdps_list.h"

void init_list(void);

void purge_phase1_key(void);

s32 kernel_init_credential(u64 profile_id, const u8 *cred,
	s32 cred_length);

s32 kernel_clear_credential(u64 profile_id);

s32 get_phase1_key_from_list(u64 profile_id, u8 **phase1_key,
	u32 *phase1_key_size);

#endif
