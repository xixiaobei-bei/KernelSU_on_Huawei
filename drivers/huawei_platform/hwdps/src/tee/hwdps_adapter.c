/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function definations required for
 *              operations about khandler
 * Create: 2020-10-10
 */

#include "inc/tee/hwdps_adapter.h"
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/random.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <securec.h>
#include "inc/base/hwdps_utils.h"
#include "inc/base/hwdps_list.h"
#include "inc/base/hwdps_defines.h"
#include "inc/tee/base_alg.h"
#include "inc/tee/hwdps_alg.h"
#include "inc/tee/hwdps_tee.h"

static DEFINE_RWLOCK(g_phase1_key_lock);

struct list_head g_head;

void init_list(void)
{
	INIT_LIST_HEAD(&g_head);
}

void purge_phase1_key(void)
{
	purge_list(&g_head);
}

s32 get_phase1_key_from_list(u64 profile_id, u8 **phase1_key,
	u32 *phase1_key_size)
{
	s32 err_code;
	u8 *data = NULL;
	u32 data_size = 0;
	u8 *phase1_key_list = NULL;

	if (!phase1_key || !phase1_key_size)
		return ERR_MSG_BAD_PARAM;
	read_lock(&g_phase1_key_lock);
	err_code = retrieve_from_list(profile_id, &g_head, &data, &data_size);
	if ((err_code == ERR_MSG_SUCCESS) && data) {
		if (data_size != PHASE_1_KEY_LENGTH) {
			read_unlock(&g_phase1_key_lock);
			return ERR_MSG_KERNEL_PHASE1_KEY_NULL;
		}
		phase1_key_list = kzalloc(data_size, GFP_KERNEL);
		if (!phase1_key_list) {
			read_unlock(&g_phase1_key_lock);
			return ERR_MSG_OUT_OF_MEMORY;
		}
		if (memcpy_s(phase1_key_list, data_size, data,
			data_size) != EOK) {
			read_unlock(&g_phase1_key_lock);
			kzfree(phase1_key_list);
			return ERR_MSG_GENERATE_FAIL;
		}
		/* data is not malloc, it just point to list node */
		data = NULL;
	} else {
		read_unlock(&g_phase1_key_lock);
		return ERR_MSG_KERNEL_PHASE1_KEY_NULL;
	}
	*phase1_key = phase1_key_list;
	*phase1_key_size = PHASE_1_KEY_LENGTH;
	read_unlock(&g_phase1_key_lock);
	return err_code;
}

s32 kernel_init_credential(u64 profile_id, const u8 *cred,
	s32 cred_length)
{
	s32 err_code;
	u8 *phase1_key = NULL;
	u32 phase1_key_size = 0;

	if (!cred || (cred_length != PHASE_1_KEY_LENGTH))
		return ERR_MSG_BAD_PARAM;
	err_code = get_phase1_key_from_list(profile_id, &phase1_key,
		&phase1_key_size);
	if (err_code == ERR_MSG_SUCCESS) {
		hwdps_pr_info("phase1_key in list");
		goto cleanup;
	}
	err_code = hwdps_init_tee();
	if (err_code != ERR_MSG_SUCCESS) {
		hwdps_pr_err("hwdps init tee failed");
		goto cleanup;
	}
	err_code = send_credential_request(profile_id, cred,
		cred_length, HWDPS_INIT_USER, &phase1_key, &phase1_key_size);
	if ((err_code != ERR_MSG_SUCCESS) ||
		(phase1_key_size != PHASE_1_KEY_LENGTH)) {
		hwdps_pr_err("hwdps init tee failed");
		err_code = ERR_MSG_GENERATE_FAIL;
		goto cleanup;
	}
	write_lock(&g_phase1_key_lock);
	err_code = insert_into_list(&g_head, phase1_key, phase1_key_size,
		profile_id);
	write_unlock(&g_phase1_key_lock);
	if (err_code == ERR_MSG_SUCCESS) {
		hwdps_pr_info("phase1_key into list success");
		phase1_key = NULL;
	} else {
		hwdps_pr_err("phase1_key into list failed");
		goto cleanup;
	}
	hwdps_pr_info("cred check success");

cleanup:
	kzfree(phase1_key);
	return err_code;
}

s32 kernel_clear_credential(u64 profile_id)
{
	s32 err_code;
	u8 *data = NULL;

	write_lock(&g_phase1_key_lock);
	err_code = delete_from_list(profile_id, &g_head, &data);
	write_unlock(&g_phase1_key_lock);
	if (err_code == ERR_MSG_SUCCESS)
		kzfree(data);
	else if (err_code == ERR_MSG_LIST_NODE_NOT_EXIST)
		err_code = ERR_MSG_SUCCESS;

	return err_code;
}
