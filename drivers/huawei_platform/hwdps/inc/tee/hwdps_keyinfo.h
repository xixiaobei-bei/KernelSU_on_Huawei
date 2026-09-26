/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for generate key and
 *              encrypt key management.
 * Create: 2020-06-16
 */

#ifndef _INC_TEE_KEYINFO_H
#define _INC_TEE_KEYINFO_H
#include <linux/fs.h>
#include <linux/types.h>
#include "inc/base/hwdps_defines.h"

s32 hwdps_get_key(const u8 *descriptor,
	buffer_t *aes_key, uid_t uid);

s32 kernel_new_fek(const u8 *desc, uid_t uid,
	secondary_buffer_t *encoded_buf, buffer_t *fek);

s32 kernel_get_fek_v2(const u8 *desc, uid_t uid,
	const buffer_t *encoded_buf, secondary_buffer_t *fek);

s32 kernel_update_fek(const u8 *desc, const buffer_t *encoded_buf,
	secondary_buffer_t *fek, uid_t new_uid, uid_t old_uid);

s32 kernel_get_fek_v1(u64 profile_id, const buffer_t *encoded_wfek,
	secondary_buffer_t *fek);

s32 kernel_encrypt_key(const u8 *desc, uid_t uid,
	secondary_buffer_t *encoded_buf, buffer_t *fek);

#endif
