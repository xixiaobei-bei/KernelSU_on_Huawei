/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: implement hisee power function
 * Create: 2020-02-17
 */
#include "hisee_power.h"
#include <asm/compiler.h>
#include <linux/clk.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/fcntl.h>
#include <linux/fd.h>
#include <linux/fs.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/ipc_msg.h>
#include <linux/hisi/partition_ap_kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/uaccess.h>
#include <securec.h>
#ifdef CONFIG_HISEE_MNTN
#include "hisee_mntn.h"
#endif
#include "hisee.h"
#include "hisee_chip_test.h"
#include "hisee_fs.h"
#include "soc_acpu_baseaddr_interface.h"
#include "soc_sctrl_interface.h"

#define POWER_VOTE_OFF_STATUS    0x0
#define POWER_VOTE_UNIT_MASK     0xFF
#define MAX_COUNT                0xFF
#define NO_VOTE_MAX_COUNT        0xFFFFFFFF

static union hisee_power_vote_status g_power_vote_status;
static unsigned int g_power_vote_cnt;
static unsigned int g_cos_id;

/* save the current booting cos_id */
static unsigned int g_runtime_cosid = COS_IMG_ID_0;

static enum hisee_power_vote_record g_vote_record_method;
/*
 * whether we need to pre enable clk before powering on hisee:
 * 0->needn't ; 1->need
 */
static bool g_power_pre_enable_clk;

static bool g_daemon_created;
static int g_unhandled_timer_cnt;
static struct semaphore g_hisee_poweroff_sem;
static struct mutex g_poweron_timeout_mutex;
static struct list_head g_unhandled_timer_list;

void set_pre_enable_clk(bool value)
{
	g_power_pre_enable_clk = value;
}

unsigned long get_power_vote_status(void)
{
	return g_power_vote_status.value;
}

unsigned int get_used_cos_id(void)
{
	return g_cos_id;
}

unsigned int get_runtime_cos_id(void)
{
	return g_runtime_cosid;
}

void hisee_power_ctrl_init(void)
{
	g_vote_record_method = HISEE_POWER_VOTE_RECORD_PRO;
	g_power_vote_status.value = POWER_VOTE_OFF_STATUS;
	g_power_vote_cnt = 0;
	mutex_init(&g_poweron_timeout_mutex);
	g_unhandled_timer_cnt = 0;
	INIT_LIST_HEAD(&g_unhandled_timer_list);
}

static int hisee_powerctrl_paras_check(unsigned int vote_process,
				       enum hisee_power_operation op_type,
				       unsigned int op_cosid,
				       int power_cmd)
{
	if (g_vote_record_method == HISEE_POWER_VOTE_RECORD_PRO &&
	    vote_process >= MAX_POWER_PROCESS_ID) {
		pr_err("%s(): vote_procs=%u invalid\n", __func__, vote_process);
		return HISEE_INVALID_PARAMS;
	}

	if (op_type < HISEE_POWER_OFF || op_type >= HISEE_POWER_MAX_OP) {
		pr_err("%s(): op_type=%x invalid\n",  __func__, op_type);
		return HISEE_INVALID_PARAMS;
	}

	if (op_cosid >= MAX_COS_IMG_ID) {
		pr_err("%s(): cosid=%u invalid\n",  __func__, op_cosid);
		return HISEE_INVALID_PARAMS;
	}

	if (power_cmd != HISEE_POWER_CMD_ON &&
	    power_cmd != HISEE_POWER_CMD_OFF) {
		pr_err("%s(): power_cmd=%d invalid\n",  __func__, power_cmd);
		return HISEE_INVALID_PARAMS;
	}

	if ((op_type != HISEE_POWER_OFF && power_cmd == HISEE_POWER_CMD_OFF) ||
	    (op_type == HISEE_POWER_OFF && power_cmd != HISEE_POWER_CMD_OFF)) {
		pr_err("%s():power_cmd and op_type is not match.\n",  __func__);
		return HISEE_INVALID_PARAMS;
	}
	return HISEE_OK;
}

static int hisee_power_ctrl(enum hisee_power_operation op_type,
			    unsigned int op_cosid, int power_cmd)
{
	int ret;
	u64 cmd_to_atf;
	struct hisee_module_data *hisee_data_ptr = NULL;

	/* check para */
	if (power_cmd != HISEE_POWER_CMD_ON &&
	    power_cmd != HISEE_POWER_CMD_OFF) {
		pr_err("%s() para check failed.\n", __func__);
		ret = HISEE_INVALID_PARAMS;
		goto end;
	}

	hisee_data_ptr = get_hisee_data_ptr();
	if (power_cmd == HISEE_POWER_CMD_ON) {
		cmd_to_atf = (u64)CMD_HISEE_POWER_ON;
		if (g_power_pre_enable_clk) {
			ret = clk_prepare_enable(hisee_data_ptr->hisee_clk);
			if (ret < 0) {
				pr_err("%s() clk_prepare_enable failed ret=%d.\n",
				       __func__, ret);
				ret = HISEE_BULK_CLK_ENABLE_ERROR;
				goto end;
			}
		}
	} else {
		cmd_to_atf = (u64)CMD_HISEE_POWER_OFF;
	}

	/* send power command to atf */
	ret = atfd_hisee_smc((u64)HISEE_FN_MAIN_SERVICE_CMD,
			     cmd_to_atf, (u64)op_type, (u64)op_cosid);
	if (ret != HISEE_OK) {
		pr_err("%s(): power_cmd=%d to atf failed, ret=%d\n",
		       __func__, power_cmd, ret);
	} else if (power_cmd == HISEE_POWER_CMD_OFF) {
		if (g_power_pre_enable_clk)
			clk_disable_unprepare(hisee_data_ptr->hisee_clk);
	} else {
		pr_err("%s(): do nothing.\n", __func__);
	}
end:
	return set_errno_then_exit(ret);
}

#ifdef CONFIG_SMX_PROCESS
int smx_process(enum hisee_power_operation op_type,
		unsigned int op_cosid, int power_cmd)
{
	int ret;
	u64 cmd_to_atf;
	struct hisee_module_data *hisee_data_ptr = NULL;

	/* check para */
	if (power_cmd != SMX_PROCESS_STEP1_CMD &&
	    power_cmd != SMX_PROCESS_STEP2_CMD) {
		pr_err("%s() para check failed.\n", __func__);
		return SMX_PROCESS_INVALID_PARAMS;
	}

	hisee_data_ptr = get_hisee_data_ptr();
	if (power_cmd == SMX_PROCESS_STEP1_CMD) {
		cmd_to_atf = (u64)CMD_SMX_PROCESS_STEP1;
		if (g_power_pre_enable_clk) {
			ret = clk_prepare_enable(hisee_data_ptr->hisee_clk);
			if (ret < 0) {
				pr_err("%s() clk_prepare_enable failed ret=%d.\n",
				       __func__, ret);
				return SMX_PROCESS_CLK_ENABLE_ERROR;
			}
		}
	} else {
		cmd_to_atf = (u64)CMD_SMX_PROCESS_STEP2;
	}

	/* send power command to atf */
	ret = atfd_hisee_smc((u64)SMX_PROCESS_FN_MAIN_SERVICE_CMD, cmd_to_atf,
			     (u64)op_type, (u64)op_cosid);
	if (ret == SMX_PROCESS_SUPPORT_BUT_ERROR) {
		pr_err("%s(): power_cmd=%d to atf failed, ret=%d\n",
		       __func__, power_cmd, ret);
	} else if (power_cmd == SMX_PROCESS_STEP2_CMD) {
		if (g_power_pre_enable_clk)
			clk_disable_unprepare(hisee_data_ptr->hisee_clk);
	} else {
		pr_err("%s(): do nothing.\n", __func__);
	}

	return ret;
}
#endif

static int _set_vote_process_method(u32 vote_process, enum hisee_power_cmd power_cmd)
{
	u32 shift;
	u64 current_count;
	int ret = HISEE_OK;

	/* the 3 means every vote status unit is 8 bits */
	shift = (vote_process - COS_PROCESS_WALLET) << 3;
	current_count = (g_power_vote_status.value >> shift) &
			POWER_VOTE_UNIT_MASK;
	g_power_vote_status.value &= ~((u64)POWER_VOTE_UNIT_MASK << shift);

	if (power_cmd == HISEE_POWER_CMD_ON) {
		if (current_count == MAX_COUNT) {
			pr_err("Vote is the maximum number.\n");
			ret = HISEE_ERROR;
		} else {
			current_count++;
		}
	} else {
		if (current_count > 0)
			current_count--;
		else
			pr_err("Vote is zero already.\n");
	}
	g_power_vote_status.value |= current_count << shift;
	pr_err("VoteStatus:%lx.\n",  g_power_vote_status.value);
	return ret;
}

static int _set_vote_cnt_method(enum hisee_power_cmd power_cmd)
{
	int ret = HISEE_OK;

	if (power_cmd == HISEE_POWER_CMD_ON) {
		if (g_power_vote_cnt < NO_VOTE_MAX_COUNT) {
			g_power_vote_cnt++;
		} else {
			pr_err("Vote count is already max.\n");
			ret = HISEE_ERROR;
		}
	} else {
		if (g_power_vote_cnt > 0)
			g_power_vote_cnt--;
		else
			pr_err("Vote is already zero.\n");
	}

	pr_err("VoteCnt:%x.\n",  g_power_vote_cnt);
	return ret;
}

static int hisee_set_power_vote_status(u32 vote_process,
				       enum hisee_power_cmd power_cmd)
{
	int ret;

	if (g_vote_record_method == HISEE_POWER_VOTE_RECORD_PRO)
		ret = _set_vote_process_method(vote_process, power_cmd);
	else
		ret = _set_vote_cnt_method(power_cmd);

	return ret;
}

enum hisee_power_status hisee_get_power_status(void)
{
	if (g_vote_record_method == HISEE_POWER_VOTE_RECORD_PRO) {
		if (g_power_vote_status.value != POWER_VOTE_OFF_STATUS)
			return HISEE_POWER_STATUS_ON;
	} else {
		if (g_power_vote_cnt > 0)
			return HISEE_POWER_STATUS_ON;
	}

	return HISEE_POWER_STATUS_OFF;
}

static int _power_vote_off2on(unsigned int vote_process,
			      enum hisee_power_operation op_type,
			      unsigned int op_cosid,
			      enum hisee_power_cmd power_cmd)
{
	int ret;

	/* from the off status to on status */
	ret = hisee_power_ctrl(op_type, op_cosid, power_cmd);
	if (ret != HISEE_OK) {
		pr_err("%s(): hisee power on failed.\n",  __func__);
		return ret;
	}

	ret = hisee_set_power_vote_status(vote_process, power_cmd);
	if (ret != HISEE_OK)
		pr_err("%s(): status is off, hisee power on vote failed.\n", __func__);

	return ret;
}

static int _power_vote_on2off(unsigned int vote_process,
			      enum hisee_power_operation op_type,
			      unsigned int op_cosid,
			      enum hisee_power_cmd power_cmd)
{
	int ret, ret_tmp;
	enum hisee_power_status power_status;

	/*
	 * vote then check is the vote_status if off,
	 * if the status is off, then power_off the hisee.
	 */
	ret = hisee_set_power_vote_status(vote_process, power_cmd);
	if (ret != HISEE_OK) {
		pr_err("%s(): status is on, hisee power off vote failed.\n",
		       __func__);
		return ret;
	}

	/* get the current power status again */
	power_status = hisee_get_power_status();
	if (power_status == HISEE_POWER_STATUS_OFF) {
		ret = hisee_power_ctrl(op_type, op_cosid, power_cmd);
		if (ret != HISEE_OK) {
			/* recover the vote if power off failed. */
			ret_tmp = hisee_set_power_vote_status(
							vote_process,
							HISEE_POWER_CMD_ON);
			if (ret_tmp != HISEE_OK)
				pr_err("%s(): hisee power off failed, vote the on status failed.\n",
				       __func__);
			pr_err("%s(): hisee power off failed.\n", __func__);
			return ret;
		}
	}

	return HISEE_OK;
}

static int hisee_power_vote(unsigned int vote_process, enum hisee_power_operation op_type,
			    unsigned int op_cosid, enum hisee_power_cmd power_cmd)
{
	int ret;
	enum hisee_power_status power_status;

	pr_err("VoteIn:%x,%x,%x,%x\n",
	       vote_process, op_type, op_cosid, power_cmd);
	ret = hisee_powerctrl_paras_check(vote_process,
					  op_type, op_cosid, power_cmd);
	if (ret != HISEE_OK) {
		pr_err("%s(): para check failed.\n",  __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	power_status = hisee_get_power_status();
	if (power_status == HISEE_POWER_STATUS_OFF &&
	    power_cmd == HISEE_POWER_CMD_ON) {
		ret = _power_vote_off2on(vote_process, op_type,
					 op_cosid, power_cmd);
	} else if (power_status == HISEE_POWER_STATUS_ON &&
		   power_cmd == HISEE_POWER_CMD_ON) {
		/* hisee is on, so just vote */
		ret = hisee_set_power_vote_status(vote_process, power_cmd);
		if (ret != HISEE_OK)
			pr_err("%s(): status is on, hisee power on vote failed.\n",
			       __func__);
	} else if (power_status == HISEE_POWER_STATUS_ON &&
		   power_cmd == HISEE_POWER_CMD_OFF) {
		ret = _power_vote_on2off(vote_process, op_type,
					 op_cosid, power_cmd);
	} else if (power_status == HISEE_POWER_STATUS_OFF &&
		   power_cmd == HISEE_POWER_CMD_OFF) {
		/* hisee is already off */
		pr_err("%s(): hisee is already off.\n",  __func__);
		ret = HISEE_OK;
	} else {
		pr_err("%s(): input power status(0x%x) or cmd(0x%x) error.\n",
		       __func__, power_status, power_cmd);
		ret = HISEE_INVALID_PARAMS;
	}

	return set_errno_then_exit(ret);
}

/*
 * @brief      : hisee_get_cosid_processid
 * @param[in]  : buf, content string buffer
 * @param[out] : cos_id, return the cos id
 * @param[out] : process_id, return the process id, can only be 0-3
 * @return     : ::int, 0 on success, other value on failure
 */
int hisee_get_cosid_processid(const void *buf, unsigned int *cos_id,
			      unsigned int *process_id)
{
	int ret = HISEE_OK;
	unsigned char *arg_vector = (unsigned char *)buf;

	if (!cos_id || !process_id)
		return HISEE_INVALID_PARAMS;

	if (!buf) {
		/* if no args, use default imgid 0. */
		*cos_id = COS_IMG_ID_0;
		*process_id = COS_PROCESS_CHIP_TEST;
	} else if (*(char *)arg_vector == HISEE_CHAR_NEWLINE ||
		   *(char *)arg_vector == '\0') {
		/* if no args, use default imgid 0. */
		*cos_id = COS_IMG_ID_0;
		*process_id = COS_PROCESS_UNKNOWN;
	/*
	 * parameter's length must be more than or equal to 3: one blank,
	 * one cos id and one process id.
	 */
	} else if ((strlen((char *)arg_vector) >= MAX_CMD_BUFF_PARAM_LEN - 1) &&
		   (arg_vector[0] == HISEE_CHAR_SPACE) &&
		   (arg_vector[HISEE_COS_ID_POS] >= '0') &&
		   (arg_vector[HISEE_COS_ID_POS] < '0' + MAX_COS_IMG_ID) &&
		   (arg_vector[HISEE_PROCESS_TYPE_POS] >= '0') &&
		   (arg_vector[HISEE_PROCESS_TYPE_POS] <
		    '0' + MAX_POWER_PROCESS_ID)) {
		*cos_id = arg_vector[HISEE_COS_ID_POS] - '0';
		*process_id = arg_vector[HISEE_PROCESS_TYPE_POS] - '0';
		if (*process_id == COS_PROCESS_UNKNOWN) {
			pr_err("%s(): input process_id is unknown", __func__);
			ret = HISEE_INVALID_PARAMS;
		}
	} else {
		pr_err("%s(): input cos_id process_id error", __func__);
		ret = HISEE_INVALID_PARAMS;
	}

	pr_info("hisee:%s():cos_id is %u\n", __func__, *cos_id);
	return ret;
}


int hisee_poweron_booting_func(const void *buf, int para)
{
	int ret;
	unsigned int cos_id = COS_IMG_ID_0;
	unsigned int process_id = 0;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	mutex_lock(&hisee_data_ptr->hisee_mutex);
	ret = hisee_get_cosid_processid(buf, &cos_id, &process_id);
	if (ret != HISEE_OK) {
		pr_err("%s() hisee_get_cosid failed ret=%d\n", __func__, ret);
		goto end;
	}
#ifdef CONFIG_HISEE_MNTN
	/*
	 * don't power up hisee, if current is dm mode and cos has not been upgraded,
	 * of there will be many hisee exception log reporting to apr.
	 * COS_FLASH is the specific cos_flash image, bypass the judgement.
	 */
	if (hisee_mntn_can_power_up_hisee() == HISEE_ERROR) {
		ret = HISEE_ERROR;
		goto end;
	}
#endif

	if (g_vote_record_method == HISEE_POWER_VOTE_RECORD_PRO &&
	    (process_id >= MAX_POWER_PROCESS_ID ||
	     process_id == COS_PROCESS_TIMEOUT)) {
		ret = HISEE_INVALID_PARAMS;
		pr_err("%s() process_id: %u error ret=%d\n",
		       __func__, process_id, ret);
		goto end;
	}

	if (para != HISEE_POWER_ON_BOOTING_MISC)
		para = HISEE_POWER_ON_BOOTING;

	ret = hisee_power_vote(process_id, (enum hisee_power_operation)para,
			       cos_id, HISEE_POWER_CMD_ON);
	if (ret != HISEE_OK) {
		pr_err("%s() hisee_power_vote failed ret=%d\n", __func__, ret);
		goto end;
	}
	/* record the current cosid in booting phase */
	g_runtime_cosid = cos_id;

end:
	mutex_unlock(&hisee_data_ptr->hisee_mutex);
	check_and_print_result_with_cosid();
	return set_errno_then_exit(ret);
}

int hisee_poweron_upgrade_func(const void *buf, int para)
{
	int ret;
	unsigned int cos_id = COS_IMG_ID_0;
	unsigned int process_id = 0;
	unsigned int hisee_lcs_mode = 0;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	ret = get_hisee_lcs_mode(&hisee_lcs_mode);
	if (ret != HISEE_OK) {
		pr_err("%s() get_hisee_lcs_mode failed,ret=%d\n", __func__, ret);
		return set_errno_then_exit(ret);
	}

	if (hisee_lcs_mode == HISEE_SM_MODE_MAGIC)
		para = HISEE_POWER_ON_UPGRADE_SM;
	else
		para = HISEE_POWER_ON_UPGRADE;

	mutex_lock(&hisee_data_ptr->hisee_mutex);

	ret = hisee_get_cosid_processid(buf, &cos_id, &process_id);
	if (ret != HISEE_OK) {
		pr_err("%s() hisee_get_cosid failed ret=%d\n", __func__, ret);
		goto end;
	}

	if (g_vote_record_method == HISEE_POWER_VOTE_RECORD_PRO &&
	    process_id != COS_PROCESS_UPGRADE &&
	    process_id != COS_PROCESS_UNKNOWN) {
		ret = HISEE_INVALID_PARAMS;
		pr_err("%s() process_id error ret=%d\n", __func__, ret);
		goto end;
	}

	/* To fit chiptest, change unkonw process id to upgrade id. */
	process_id = COS_PROCESS_UPGRADE;

	/* Record the cosid for check usr mismatch operation. */
	g_cos_id = cos_id;

	ret = hisee_power_vote(process_id, (enum hisee_power_operation)para,
			       cos_id, HISEE_POWER_CMD_ON);
	if (ret != HISEE_OK) {
		pr_err("%s() hisee_power_vote failed ret=%d\n", __func__, ret);
		goto end;
	}

end:
	mutex_unlock(&hisee_data_ptr->hisee_mutex);
	check_and_print_result_with_cosid();
	return set_errno_then_exit(ret);
}

int hisee_poweroff_func(const void *buf, int para)
{
	int ret;
	unsigned int cos_id = COS_IMG_ID_0;
	unsigned int process_id = 0;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	mutex_lock(&hisee_data_ptr->hisee_mutex);

	ret = hisee_get_cosid_processid(buf, &cos_id, &process_id);
	if (ret != HISEE_OK) {
		pr_err("%s() hisee_get_cosid failed ret=%d\n", __func__, ret);
		goto end;
	}
	if (g_vote_record_method == HISEE_POWER_VOTE_RECORD_PRO &&
	    (process_id >= MAX_POWER_PROCESS_ID ||
	     process_id == COS_PROCESS_TIMEOUT)) {
		ret = HISEE_INVALID_PARAMS;
		pr_err("%s() process_id error ret=%d\n", __func__, ret);
		goto end;
	}

	ret = hisee_power_vote(process_id, HISEE_POWER_OFF,
			       cos_id, HISEE_POWER_CMD_OFF);
	if (ret != HISEE_OK) {
		pr_err("%s() hisee_power_vote failed ret=%d\n", __func__, ret);
		goto end;
	}

end:
	mutex_unlock(&hisee_data_ptr->hisee_mutex);
	check_and_print_result_with_cosid();
	return set_errno_then_exit(ret);
}

static int handle_hisee_poweroff_timeout(void)
{
	int ret = HISEE_OK;
	unsigned int process_id, cos_id;
	struct timer_entry_list *cursor = NULL;
	struct timer_entry_list *next = NULL;

	/* got the sema */
	mutex_lock(&g_poweron_timeout_mutex);
	if (g_unhandled_timer_cnt > 0) {
		g_unhandled_timer_cnt--;
		if (g_unhandled_timer_cnt == 0) {
			process_id = COS_PROCESS_TIMEOUT;
			cos_id = HISEE_DEFAULT_COSID; /* the default cos */
			ret = hisee_power_vote(process_id, HISEE_POWER_OFF,
					       cos_id, HISEE_POWER_CMD_OFF);
			if (ret != HISEE_OK)
				pr_err("%s  poweroff failed, ret=%d\n",
				       __func__, ret);
			else
				pr_err("%s  poweroff success!\n", __func__);
		}
	}

	list_for_each_entry_safe(cursor, next, &g_unhandled_timer_list, list)
		if (atomic_read(&cursor->handled)) {
			list_del(&cursor->list);
			kfree(cursor);
		}

	mutex_unlock(&g_poweron_timeout_mutex);
	return ret;
}

static int hisee_poweroff_daemon_body(void *arg)
{
	int ret;

	for (; ;) {
		if (down_timeout(&g_hisee_poweroff_sem,
				 (long)HISEE_THREAD_WAIT_TIMEOUT)) {
			mutex_lock(&g_poweron_timeout_mutex);
			if (g_unhandled_timer_cnt == 0) {
				/*
				 * exit this thread if wait sema timeout and
				 * there is no timer to be handled
				 */
				g_daemon_created = false;
				mutex_unlock(&g_poweron_timeout_mutex);
				return 0;
			}
			mutex_unlock(&g_poweron_timeout_mutex);
			continue;
		}

		ret = handle_hisee_poweroff_timeout();
		if (ret != HISEE_OK)
			pr_err("%s: handle_hisee_poweroff_timeout failed, ret %d\n", __func__, ret);
	}
	return 0;
}

static int create_hisee_poweroff_daemon(void)
{
	struct task_struct *hisee_poweroff_daemon = NULL;

	/* create semaphore for daemon to wait poweroff signal */
	sema_init(&g_hisee_poweroff_sem, 0);

	hisee_poweroff_daemon = kthread_run(hisee_poweroff_daemon_body,
					    NULL, "hisee_poweroff_daemon");
	if (IS_ERR(hisee_poweroff_daemon)) {
		pr_err("hisee err create hisee_poweroff_daemon failed\n");
		return HISEE_THREAD_CREATE_ERROR;
	}

	g_daemon_created = true;
	return HISEE_OK;
}

static void poweroff_handle(unsigned long arg)
{
	struct timer_entry_list *p_timer_entry = (struct timer_entry_list *)(uintptr_t)arg;

	atomic_set(&p_timer_entry->handled, HISEE_TRUE);

	up(&g_hisee_poweroff_sem);
}

static int parse_arg_get_id(char *buf, unsigned int *id)
{
	char *p = buf;

	if (!buf || !id)
		return HISEE_INVALID_PARAMS;

	while (*p == ' ')
		p++; /* bypass blank */

	if (*p == '\0' || *p == '\n') {
		*id = NFC_SERVICE;
		return HISEE_OK;
	}
	if ((*p >= '0' + NFC_SERVICE) && (*p < '0' + MAX_TIMEOUT_ID)) {
		*id = *p - '0';
		return HISEE_OK;
	}

	pr_err("%s(): input timeout id : %c error\n", __func__, *p);
	return HISEE_INVALID_PARAMS;
}

static int _parse_arg_get_timeout(char *str, unsigned int *time,
				  unsigned int *id)
{
	/* reserved 1byte for terminated char */
	char timeout[TIMEOUT_MAX_LEN + 1] = {0};
	char *cmd = NULL;
	int i = 0;
	int ret;

	cmd = str;

	/* extract timeout value */
	while (*str != '\n' && *str != ' ' &&
	       *str != '\0' && i <= TIMEOUT_MAX_LEN)
		timeout[i++] = *str++;

	if (i >= TIMEOUT_MAX_LEN) {
		pr_err("Timeout value overflow:%s\n", cmd);
		return HISEE_INVALID_PARAMS;
	}
	/* if there is other para(id), there will be a blank */
	if (*str == ' ') {
		timeout[i] = '\0';
		if (kstrtouint(timeout, 0, time))
			return HISEE_INVALID_PARAMS;
		ret = parse_arg_get_id(str, id);
	} else {
		/* its ok that cmd end with new line */
		if (kstrtouint(cmd, 0, time))
			return HISEE_INVALID_PARAMS;
		*id = NFC_SERVICE;
		ret = HISEE_OK;
	}
	return ret;
}

static int parse_arg_get_timeout(const void *buf, int para,
				 unsigned int *time, unsigned int *id)
{
	char *p = NULL;

	/* interface for direct call */
	if (!buf || !time || !id) {
		if (para <= 0)
			return HISEE_INVALID_PARAMS;
		return para;
	}

	p = (char *)buf;
	while (*p != '\0' && *p != ' ')
		p++; /* bypass cmd name. */

	if (*p == '\0')
		return HISEE_INVALID_PARAMS;

	while (*p == ' ')
		p++; /* bypass blank */

	return _parse_arg_get_timeout(p, time, id);
}

/*
 * @brief     : poweron hisee and add a timer to poweroff hisee _msecs_ ms later
 * @param[in] : buf, content string buffer
 * @param[in] : para, parameters
 * @return     : ::int, 0 on success, other value on failure
 */
int hisee_poweron_timeout_func(const void *buf, int para)
{
	int ret_tmp;
	int ret;
	struct timer_list *p_timer = NULL;
	struct timer_entry_list *p_timer_entry = NULL;
	unsigned int msecs = 0;
	unsigned int power_vote = 0;

	ret = parse_arg_get_timeout(buf, para, &msecs, &power_vote);
	if (ret != HISEE_OK) {
		pr_err("%s() timeout params is invalid.\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}

	mutex_lock(&g_poweron_timeout_mutex);

	if (!g_daemon_created) {
		ret = create_hisee_poweroff_daemon();
		if (ret != HISEE_OK)
			goto end;
	}

	p_timer_entry = kzalloc(sizeof(struct timer_entry_list), GFP_KERNEL);
	if (!p_timer_entry) {
		pr_err("%s()  timer kmalloc failed\n", __func__);
		ret = HISEE_NO_RESOURCES;
		goto end;
	}
	atomic_set(&p_timer_entry->handled, HISEE_FALSE);

	p_timer = &p_timer_entry->timer;
	init_timer(p_timer);
	p_timer->function = poweroff_handle;
	p_timer->data = (unsigned long)(uintptr_t)p_timer_entry;
	/* +1 makes timeout >= msecs */
	p_timer->expires = jiffies + msecs_to_jiffies(msecs) + 1;

	if (g_unhandled_timer_cnt == 0) {
		const unsigned int process_id = COS_PROCESS_TIMEOUT;
		const unsigned int cos_id = HISEE_DEFAULT_COSID;

		ret = hisee_power_vote(process_id, HISEE_POWER_ON_BOOTING,
				       cos_id, HISEE_POWER_CMD_ON);
		if (ret != HISEE_OK) {
			ret_tmp = hisee_power_vote(process_id, HISEE_POWER_OFF,
						   cos_id, HISEE_POWER_CMD_OFF);

			kfree(p_timer_entry);
			pr_err("%s() poweron booting failed %d,abort poweron_timeout\n",
			       __func__, ret);
			if (ret_tmp != HISEE_OK)
				pr_err("%s() also poweroff failed, ret=%d\n",
				       __func__, ret_tmp);
			goto end;
		}
		/* record the current cosid in booting phase */
		g_runtime_cosid = cos_id;
	}

	add_timer(p_timer);
	list_add(&p_timer_entry->list, &g_unhandled_timer_list);
	g_unhandled_timer_cnt++;
end:
	pr_err("%s():power_vote is %u!\n", __func__, power_vote);
	mutex_unlock(&g_poweron_timeout_mutex);
	return set_errno_then_exit(ret);
}

static void hisee_power_off_all(void)
{
	if (g_power_vote_cnt > 0 ||
	    g_power_vote_status.value != POWER_VOTE_OFF_STATUS) {
		pr_err("%s: %lx, vote_cnt = %x\n", __func__,
		       g_power_vote_status.value, g_power_vote_cnt);
		g_power_vote_status.value = POWER_VOTE_OFF_STATUS;
		g_power_vote_cnt = 0;
		if (hisee_power_ctrl(HISEE_POWER_OFF, 0, HISEE_POWER_CMD_OFF) !=
		    HISEE_OK)
			pr_err("%s: power_off failed!\n", __func__);
	}
}

int hisee_force_power_off(void)
{
	struct hisee_module_data *hisee_data_ptr = get_hisee_data_ptr();

	if (!hisee_data_ptr) {
		pr_err("%s():get_hisee_data failed!\n", __func__);
		return HISEE_ERROR;
	}

	mutex_lock(&hisee_data_ptr->hisee_mutex);
	hisee_power_off_all();
	mutex_unlock(&hisee_data_ptr->hisee_mutex);

	return HISEE_OK;
}

int hisee_suspend(struct platform_device *pdev, struct pm_message state)
{
	struct timer_entry_list *cursor = NULL;
	struct timer_entry_list *next = NULL;
	struct hisee_module_data *hisee_data_ptr = NULL;

	hisee_data_ptr = get_hisee_data_ptr();
	if (hisee_api_is_bypassed()) {
		pr_err("hisee suspend is bypassed\n");
		return HISEE_OK;
	}
	pr_err("hisee_suspend: +\n");

	mutex_lock(&g_poweron_timeout_mutex);

	list_for_each_entry_safe(cursor, next, &g_unhandled_timer_list, list) {
		list_del(&cursor->list);
		del_timer_sync(&cursor->timer);
		kfree(cursor);
	}

	sema_init(&g_hisee_poweroff_sem, 0);
	g_unhandled_timer_cnt = 0;

	mutex_unlock(&g_poweron_timeout_mutex);

	mutex_lock(&hisee_data_ptr->hisee_mutex);
	hisee_power_off_all();
	mutex_unlock(&hisee_data_ptr->hisee_mutex);

	pr_err("hisee_suspend: -\n");
	return HISEE_OK;
}

int wait_hisee_ready(enum hisee_state ready_state, unsigned int timeout_ms)
{
	enum hisee_state state;
	const unsigned int unit = 20; /* 20ms */
	unsigned int cnt;

	timeout_ms = (timeout_ms < unit) ? unit : timeout_ms;
	cnt = timeout_ms / unit;
	do {
		state = (enum hisee_state)atfd_hisee_smc((u64)HISEE_FN_MAIN_SERVICE_CMD,
					(u64)CMD_GET_STATE, (u64)0, (u64)0);
		if (state == ready_state) {
			if (ready_state == HISEE_STATE_COS_READY) /* add log for check */
				pr_err("%s cost about %dms\n",
				       __func__, timeout_ms - cnt * unit);
			return HISEE_OK;
		}
		hisee_mdelay(unit);
		cnt--;
	} while (cnt);

	pr_err("%s fail, ready state is %d, timeout is %u ms!\n",
	       __func__, ready_state, timeout_ms);
	return HISEE_WAIT_READY_TIMEOUT;
}

static int _check_power_state_show(char *result_buff, size_t len)
{
	int ret;
	size_t remaining_len;

	/* 3 is the 0/1/2 max string length */
	ret = snprintf_s(result_buff, len, (u64)3,
			 "%d,", HISEE_POWER_STATE_SHOW_VALUE);
	if (ret == HISEE_SECLIB_ERROR) {
		pr_err("%s(): snprintf2 err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}
	remaining_len = strlen("cos unready");
	ret = strncat_s(result_buff, len,
			"cos unready", remaining_len);
	if (ret != EOK) {
		pr_err("%s(): strncat_s err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}

	return HISEE_OK;
}

static int _check_failure_show(char *result_buff, size_t len)
{
	int ret;
	size_t remaining_len;

	/* 4 is the -0/1/2 max string length */
	ret = snprintf_s(result_buff, len, (u64)4,
			 "%d,", HISEE_FAILURE_SHOW_VALUE);
	if (ret == HISEE_SECLIB_ERROR) {
		pr_err("%s(): snprintf3 err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}
	remaining_len = strlen("failed");
	ret = strncat_s(result_buff, len,
			"failed", remaining_len);
	if (ret != EOK) {
		pr_err("%s(): strncat_s err.\n", __func__);
		return HISEE_SECUREC_ERR;
	}

	return HISEE_OK;
}

/*
 * @brief      : check whether the hisee is ready
 * @param[out] : buf, check result string buffer
 * @return     : ::ssize_t, the length of output data
 */
ssize_t hisee_check_ready_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	enum hisee_state state;
	int ret;
#ifdef CONFIG_HISEE_MNTN
	unsigned int vote_lpm3;
	unsigned int vote_atf;
#endif

	if (!buf) {
		pr_err("%s buf parameters is null\n", __func__);
		return set_errno_then_exit(HISEE_INVALID_PARAMS);
	}
	state = (enum hisee_state)atfd_hisee_smc((u64)HISEE_FN_MAIN_SERVICE_CMD,
					(u64)CMD_GET_STATE, (u64)0, (u64)0);
	if (state == HISEE_STATE_COS_READY)
		ret = _check_cos_ready_show(buf, HISEE_BUF_SHOW_LEN);
	else if (state == HISEE_STATE_POWER_DOWN ||
		 state == HISEE_STATE_POWER_UP ||
		 state == HISEE_STATE_MISC_READY ||
		 state == HISEE_STATE_POWER_DOWN_DOING ||
		 state == HISEE_STATE_POWER_UP_DOING)
		ret = _check_power_state_show(buf, HISEE_BUF_SHOW_LEN);
	else
		ret = _check_failure_show(buf, HISEE_BUF_SHOW_LEN);

	if (ret != HISEE_OK) {
		pr_err("%s _check_xxx_show fail!\n", __func__);
		return set_errno_then_exit(ret);
	}
	if (state != HISEE_STATE_COS_READY) {
#ifdef CONFIG_HISEE_MNTN
		hisee_mntn_collect_vote_value_cmd();
		vote_lpm3 = hisee_mntn_get_vote_val_lpm3();
		vote_atf = hisee_mntn_get_vote_val_atf();
		pr_err("%s(): votes:lpm3 0x%08x atf 0x%08x kernel 0x%lx\n",
		       __func__, vote_lpm3, vote_atf, g_power_vote_status.value);
#endif
	}

	pr_err("%s(): state=%d, %s\n", __func__, (int)state, buf);
	return (ssize_t)strlen(buf);
}

