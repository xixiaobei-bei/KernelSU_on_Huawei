/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: implement hisee mntn function
 * Create: 2020-02-17
 */
#ifndef __HISEE_MNTN_H__
#define __HISEE_MNTN_H__
#include <linux/hisi/ipc_msg.h>
#include <linux/hisi/rdr_pub.h>

/* used for translation from original value to exception module id */
struct hisee_exc_trans_s {
	u32 int_id_value;
	u32 module_value;
};

struct hlog_header {
	u64 addr;
	u64 max_size;
	u64 real_size;
};

enum hisee_msg_type {
	HISEE_MSG_START,
	/* lpm3 -> kernel hisee mntn driver */
	HISEE_MSG_EXCEPTION,
	HISEE_MSG_IRQ,
	HISEE_MSG_SYNC_TIME,
	HISEE_MSG_LOG_OUT,
	HISEE_MSG_VOTE_RES,
	/* kernel hisee mntn driver -> lpm3 */
	HISEE_MNTN_TYPE,
	MSPC_MSG_RECORD,  /* lpm3 -> kernel */
	MSPC_MSG_SECFLASH_ERR,
	MSPC_MSG_SECFLASH_WARN,
	HISEE_TYPE_END
};

/*
 * msg definition Kernel->lpm3->hisee, so keep the same as defined
 * in lpm3 and hisee, starts
 */
enum hisee_msg_group {
	HISEE_GROUP_START,
	HISEE_MNTN_CTRL,

	HISEE_SYNC_TIME,
	HISEE_RESET,
	HISEE_TEST,
	HISEE_GET_VOTE,
	HISEE_GROUP_END
};

enum hisee_msg_data {
	HISEE_STATUS_START,
	HISEE_TEST_IRQ,
	HISEE_TEST_EXCEPTION,
	HISEE_TEST_RECORD,
	HISEE_TEST_RUN_COS,
	HISEE_TEST_PRINT_ERR,
	HISEE_TEST_PRINT_PLAIN,
	HISEE_TEST_PRINT_CRYPTO,
	HISEE_STATUS_END
};

/* used to define control level, keep the same as define in hisee */
enum hisee_mntn_ctrl {
	HISEE_MNTN_DISABLE = 0x3C5AA5C3,
	HISEE_MNTN_USR = 0xA53CC3A5,
	HISEE_MNTN_BETA = 0xA5C33C5A,
	HISEE_MNTN_ENG = 0xC3A55A3C,
};

enum hisee_mntn_smc_cmd {
	HISEE_SMC_INIT = 1,
	HISEE_SMC_GET_LOG, /* save all log data when hisee reset */
	HISEE_SMC_LOG_OUT, /* get log print of hisee when it is running */
	HISEE_SMC_GET_ATF_VOTE, /* get atf vote value of hisee pwr state */
	HISEE_SMC_GET_TEE_VOTE, /* get tee vote value of hisee pwr state */
};

enum hisee_mntn_state {
	HISEE_STATE_INVALID,
	HISEE_STATE_READY,
	HISEE_STATE_LOG_OUT,
	HISEE_STATE_HISEE_EXC,
};

/* keep the same as define in kernel */
#define HISEE_MNTN_LOG_CTRL_OUT_2_KERNEL    0x5A5A0000
#define HISEE_MNTN_LOG_CTRL_NOT_2_KERNEL    0xA5A50000
#define HISEE_DMD_START  DSM_HISEE_BASE_ERROR_NO
#define HISEE_DMD_END    ((DSM_HISEE_COS_IMAGE_UPGRADE_ERROR_NO) + 1000)

#define HISEE_MNTN_ID            0xC500CC00UL

#define HISEE_MNTN_PATH_MAXLEN   128
#define HISEE_LOG_FLIENAME       "hisee_log"
#define HISEE_PRINTLOG_FLIENAME  "hisee_printlog"
#define HISEE_PRINTLOG_FLIENAME_BAK  "hisee_printlog.1"
#define HISEELOG_LIMIT_SIZE      (512 * 1024)
#define HISEE_FILE_PERMISSION    0660
#define WAIT_LPM3_REPLY_TIMEOUT         1000

#define ROOT_UID            0
#define SYSTEM_GID          1000

#define HISEE_MNTN_PRINT_COS_VER_MS     3000
#define HISEE_MNTN_PRINT_COS_VER_MAXTRY 10

#define HISEE_MNTN_IPC_MSG_LEN   2
#define HISEE_MNTN_MSG_LEN       4
#define HISEE_MNTN_IPC_MSG_0     0
#define HISEE_MNTN_IPC_MSG_1     1
#define HISEE_MNTN_IPC_MSG_2     2
#define HISEE_MNTN_IPC_MSG_3     3

#define LPM3_HISEE_MNTN    IPC_CMD(OBJ_AP, OBJ_INSE, CMD_INQUIRY, HISEE_MNTN_TYPE)
#define HISEE_EXCEPTION    IPC_CMD(OBJ_LPM3, OBJ_INSE, CMD_NOTIFY, HISEE_MSG_EXCEPTION)
#define HISEE_IRQ          IPC_CMD(OBJ_LPM3, OBJ_INSE, CMD_NOTIFY, HISEE_MSG_IRQ)
#define HISEE_TIME         IPC_CMD(OBJ_LPM3, OBJ_INSE, CMD_NOTIFY, HISEE_MSG_SYNC_TIME)
#define HISEE_LOG_OUT      IPC_CMD(OBJ_LPM3, OBJ_INSE, CMD_NOTIFY, HISEE_MSG_LOG_OUT)
#define HISEE_VOTE_RES     IPC_CMD(OBJ_LPM3, OBJ_INSE, CMD_NOTIFY, HISEE_MSG_VOTE_RES)
#define MSPC_RECORD        IPC_CMD(OBJ_LPM3, OBJ_INSE, CMD_NOTIFY, MSPC_MSG_RECORD)
#define MSPC_SECFLASH_ERR  IPC_CMD(OBJ_LPM3, OBJ_INSE, CMD_NOTIFY, MSPC_MSG_SECFLASH_ERR)
#define MSPC_SECFLASH_WARN IPC_CMD(OBJ_LPM3, OBJ_INSE, CMD_NOTIFY, MSPC_MSG_SECFLASH_WARN)

/* Must keep the same as the one define in atf!!! */
#define HISEE_MNTN_LOG_OUT_MAGIC  0x5a7e5a7e
#define HISEE_DMD_BUFF_SIZE       1024

#define DATA_FULL_BUF_LEN         0x400000

char *rdr_get_timestamp(void);
void rdr_count_size(void);
u64 rdr_get_logsize(void);
int rdr_dir_size(const char *path, u32 path_len, bool recursion);

void hisee_mntn_print_cos_info(void);
void hisee_mntn_update_local_ver_info(void);
u32 hisee_mntn_get_vote_val_lpm3(void);
u32 hisee_mntn_get_vote_val_atf(void);
int hisee_mntn_can_power_up_hisee(void);
void hisee_mntn_debug_dump(void);

#ifdef CONFIG_MSPC
u64 mspc_mntn_get_vote_val_tee(void);
#endif

int hisee_mntn_collect_vote_value_cmd(void);
int hisee_mntn_record_dmd_info(long dmd_no, const char *dmd_info);
void hisee_mntn_update_local_lcsmode_val(void);
void rdr_hisee_call_to_record_exc(int data);
#endif
