/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: Function definition for token decry, update, verify and so on.
 * Author: qiqingchao  q00XXXXXX
 * Create: 2016-06-21
 */
#include "security_auth_enhance.h"
#include <linux/sched.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <linux/err.h>
#include <securec.h>
#include "teek_client_constants.h"
#include "teek_client_id.h"
#include "tc_ns_client.h"
#include "tc_ns_log.h"
#include "teek_client_type.h"
#include "securectype.h"
#include "gp_ops.h"
#include "tc_client_sub_driver.h"

#if !defined(UINT64_MAX)
	#define UINT64_MAX ((uint64_t)0xFFFFFFFFFFFFFFFFULL)
#endif

#ifdef SECURITY_AUTH_ENHANCE
#define GLOBAL_CMD_ID_SSA               0x2DCB /* SSA cmd_id 11723 */
#define GLOBAL_CMD_ID_MT                0x2DCC /* MT cmd_id 11724 */
#define GLOBAL_CMD_ID_MT_UPDATE         0x2DCD /* MT_IPDATE cmd_id 11725 */
#define TEEC_PENDING2_AGENT             0xFFFF2001

static bool is_token_empty(const uint8_t *token, uint32_t token_len)
{
	uint32_t i;

	if (token == NULL) {
		tloge("bad parameters, token is null\n");
		return true;
	}
	for (i = 0; i < token_len; i++) {
		if (*(token + i))
			return false;
	}
	return true;
}

static TEEC_Result scrambling_timestamp(const void *in, void *out,
	uint32_t data_len, const void *key, uint32_t key_len)
{
	uint32_t i;
	bool check_value = false;

	if (in == NULL || out == NULL || key == NULL) {
		tloge("bad parameters, input_data is null\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}
	check_value = (data_len == 0 || data_len > SECUREC_MEM_MAX_LEN ||
			key_len > SECUREC_MEM_MAX_LEN || key_len == 0);
	if (check_value) {
		tloge("bad parameters, data_len is %u, scrambling_len is %u\n",
		      data_len, key_len);
		return TEEC_ERROR_BAD_PARAMETERS;
	}
	for (i = 0; i < data_len; i++)
		*((uint8_t *)out + i) =
			*((uint8_t *)in + i) ^ *((uint8_t *)key + i % key_len);

	return TEEC_SUCCESS;
}

static int32_t change_time_stamp(uint8_t flag, uint64_t *time_stamp)
{
	if (flag == INC) {
		if(*time_stamp < UINT64_MAX) {
			(*time_stamp)++;
		} else {
			tloge("val overflow\n");
			return -EFAULT;
		}
	} else if (flag == DEC) {
		if(*time_stamp > 0) {
			(*time_stamp)--;
		} else {
			tloge("val down overflow\n");
			return -EFAULT;
		}
	} else {
		tloge("flag error , 0x%x\n", flag);
		return -EFAULT;
	}
	return EOK;
}

static int32_t descrambling_timestamp(uint8_t *in_token_buf,
	const struct session_secure_info *secure_info, uint8_t flag)
{
	uint64_t time_stamp = 0;
	int32_t ret;

	if (in_token_buf == NULL || secure_info == NULL) {
		tloge("invalid params!\n");
		return -EINVAL;
	}
	if (scrambling_timestamp(&in_token_buf[TIMESTAMP_BUFFER_INDEX],
				 &time_stamp, TIMESTAMP_LEN_DEFAULT,
				 secure_info->scrambling, SCRAMBLING_KEY_LEN)) {
		tloge("descrambling_timestamp failed\n");
		return -EFAULT;
	}
	ret = change_time_stamp(flag, &time_stamp);
	if(ret != EOK) {
		return ret;
	}

	tlogd("timestamp is %llu\n", time_stamp);
	if (scrambling_timestamp(&time_stamp,
				 &in_token_buf[TIMESTAMP_BUFFER_INDEX],
				 TIMESTAMP_LEN_DEFAULT,
				 secure_info->scrambling,
				 SCRAMBLING_KEY_LEN)) {
		tloge("descrambling_timestamp failed\n");
		return -EFAULT;
	}
	return EOK;
}

TEEC_Result update_timestamp(const tc_ns_smc_cmd *cmd)
{
	tc_ns_session *session = NULL;
	struct session_secure_info *secure_info = NULL;
	uint8_t *token_buffer = NULL;
	bool filter_flag = false;
	bool need_check_flag = false;

	if (cmd == NULL) {
		tloge("cmd is NULL, error!");
		return TEEC_ERROR_BAD_PARAMETERS;
	}
	/* if cmd is agent, not check uuid. and sometime uuid canot access it */
	filter_flag = (cmd->agent_id != 0) ||
			(cmd->ret_val == TEEC_PENDING2_AGENT);
	if (filter_flag)
		return TEEC_SUCCESS;

	need_check_flag = (cmd->global_cmd == false) && (cmd->agent_id == 0) &&
		(cmd->ret_val != TEEC_PENDING2_AGENT);
	if (need_check_flag) {
		token_buffer = phys_to_virt((phys_addr_t)(cmd->token_h_phys) <<
					    ADDR_TRANS_NUM | (cmd->token_phys));
		if (token_buffer == NULL ||
			is_token_empty(token_buffer, TOKEN_BUFFER_LEN)) {
			tloge("token is NULL or token is empyt, error!\n");
			return TEEC_ERROR_GENERIC;
		}

		session = tc_find_session2(cmd->dev_file_id, cmd);
		if (session == NULL) {
			tlogd("tc_find_session_key find session FAILURE\n");
			return TEEC_ERROR_GENERIC;
		}
		secure_info = &session->secure_info;
		if (descrambling_timestamp(token_buffer,
						secure_info, INC) != EOK) {
			put_session_struct(session);
			tloge("update token_buffer error\n");
			return TEEC_ERROR_GENERIC;
		}
		put_session_struct(session);
		token_buffer[SYNC_INDEX] = UN_SYNCED;
	} else {
		tlogd("global cmd or agent, do not update timestamp\n");
	}
	return TEEC_SUCCESS;
}

TEEC_Result sync_timestamp(const tc_ns_smc_cmd *cmd, uint8_t *token,
	uint32_t token_len, bool global)
{
	tc_ns_session *session = NULL;
	bool check_val = false;

	check_val = (cmd == NULL || token == NULL || token_len <= SYNC_INDEX);
	if (check_val) {
		tloge("parameters is NULL, error!\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}
	if (cmd->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION && global) {
		tlogd("OpenSession would not need sync timestamp\n");
		return TEEC_SUCCESS;
	}
	if (token[SYNC_INDEX] == UN_SYNCED) {
		tlogd("flag is UN_SYNC, to sync timestamp!\n");

		session = tc_find_session2(cmd->dev_file_id, cmd);
		if (session == NULL) {
			tloge("sync_timestamp find session FAILURE\n");
			return TEEC_ERROR_GENERIC;
		}
		if (descrambling_timestamp(token,
						&session->secure_info,
						DEC) != EOK) {
			put_session_struct(session);
			tloge("sync token_buffer error\n");
			return TEEC_ERROR_GENERIC;
		}
		put_session_struct(session);
		return TEEC_SUCCESS;
	} else if (token[SYNC_INDEX] == IS_SYNCED) {
		return TEEC_SUCCESS;
	} else {
		tloge("sync flag error! 0x%x\n", token[SYNC_INDEX]);
	}
	return TEEC_ERROR_GENERIC;
}

/* scrambling operation and pid */
static void scrambling_operation(tc_ns_smc_cmd *cmd, uint32_t scrambler)
{
	if (cmd == NULL)
		return;
	if (cmd->operation_phys != 0 || cmd->operation_h_phys != 0) {
		cmd->operation_phys = cmd->operation_phys ^ scrambler;
		cmd->operation_h_phys = cmd->operation_h_phys ^ scrambler;
	}
	cmd->pid = cmd->pid ^ scrambler;
}

static bool agent_msg(uint32_t cmd_id)
{
	bool agent = cmd_id == GLOBAL_CMD_ID_SSA ||
		cmd_id == GLOBAL_CMD_ID_MT ||
		cmd_id == GLOBAL_CMD_ID_MT_UPDATE;

	return agent;
}

/* calculate cmd checksum and scrambling operation */
TEEC_Result update_chksum(tc_ns_smc_cmd *cmd)
{
	tc_ns_session *session = NULL;
	struct session_secure_info *secure_info = NULL;
	uint32_t scrambler_oper = 0;
	bool check_value = false;

	if (cmd == NULL) {
		tloge("cmd is NULL, error\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}
	/*
	 * if cmd is agent, do not check uuid.
	 * and sometimes uuid cannot access it
	 */
	check_value = (cmd->agent_id != 0 ||
				cmd->ret_val == TEEC_PENDING2_AGENT);
	if (check_value == true)
		return TEEC_SUCCESS;

	if (agent_msg(cmd->cmd_id)) {
		tlogd("SSA cmd, no need to update_chksum\n");
		return TEEC_SUCCESS;
	}
	/* cmd is invoke command */
	check_value = (cmd->global_cmd == false) && (cmd->agent_id == 0) &&
		(cmd->ret_val != TEEC_PENDING2_AGENT);

	if (check_value) {
		session = tc_find_session2(cmd->dev_file_id, cmd);
		if (session != NULL) {
			secure_info = &session->secure_info;
			scrambler_oper =
				secure_info->scrambling[SCRAMBLING_OPERATION];
			scrambling_operation(cmd, scrambler_oper);
			put_session_struct(session);
		}
	}
	return TEEC_SUCCESS;
}

TEEC_Result verify_chksum(const tc_ns_smc_cmd *cmd)
{
	tc_ns_session *session = NULL;
	struct session_secure_info *secure_info = NULL;
	bool check_flag = false;

	if (cmd == NULL) {
		tloge("cmd is NULL, error\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}
	if (agent_msg(cmd->cmd_id)) {
		tlogd("SSA cmd, no need to update_chksum\n");
		return TEEC_SUCCESS;
	}

	/* cmd is invoke command */
	check_flag = cmd->global_cmd == false &&
			cmd->cmd_id != GLOBAL_CMD_ID_CLOSE_SESSION &&
			cmd->cmd_id != GLOBAL_CMD_ID_KILL_TASK &&
			cmd->agent_id == 0;
	if (check_flag) {
		session = tc_find_session2(cmd->dev_file_id, cmd);
		if (session) {
			secure_info = &session->secure_info;
			put_session_struct(session);
		}
	}
	return TEEC_SUCCESS;
}
#endif
