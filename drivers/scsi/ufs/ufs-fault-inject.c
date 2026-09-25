/* 
 * Copyright (c) 2013-2019, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * UFS fault inject - add fault inject interface to the ufshcd.
 * This infrastructure can be used for debugging the driver from userspace.
 *
 */

#define pr_fmt(fmt) "ufshcd :" fmt

#include "ufs-fault-inject.h"
#include "ufs-kirin.h"
#include "ufshcd.h"
#include "ufshci.h"
#include "unipro.h"
#include <linux/debugfs.h>
#include <linux/fault-inject.h>
#include <linux/random.h>

#ifdef CONFIG_SCSI_UFS_FAULT_INJECT
#define DEFAULT_ERR_INDEX 0xFFFFFFFF

struct fault_inject_files {
	struct dentry *debugfs_root;
	struct dentry *err_inj_scenario;
	struct dentry *err_inj_err_index;
	struct dentry *err_inj_stats;
	u32 err_inj_scenario_mask;
	struct fault_attr fail_attr;
};

/* UFSHCD UIC layer error flags */
enum {
	UFSHCD_UIC_NON_FATAL_ERROR = 0x0,
	UFSHCD_UIC_DL_PA_INIT_ERROR = (1 << 0), /* Data link layer error */
	UFSHCD_UIC_NL_ERROR = (1 << 1), /* Network layer error */
	UFSHCD_UIC_TL_ERROR = (1 << 2), /* shift 2 for Transport Layer error */
	UFSHCD_UIC_DME_ERROR = (1 << 3), /* shift 3 for DME error */
};

static DECLARE_FAULT_ATTR(fail_default_attr);
static char *fail_request;
static struct fault_inject_files fault_inject_files;
module_param(fail_request, charp, 0444);

/*
 * struct ufsdbg_err_scenario - error scenario use case
 * @name: the name of the error scenario
 * @err_code_arr: error codes array for this error scenario
 * @num_err_codes: number of error codes in err_code_arr
 */
struct ufsdbg_err_scenario {
	const char *name;
	const int *err_code_arr;
	u32 num_err_codes;
	u32 err_code_index;
	u32 num_err_injected;
};

/*
 * the following static arrays are aggregation of possible errors
 * that might occur during the relevant error scenario
 */
static const int err_inject_intr_err_codes[] = {
	CONTROLLER_FATAL_ERROR,
	SYSTEM_BUS_FATAL_ERROR,

	DEVICE_FATAL_ERROR,
	UIC_ERROR,
	UIC_HIBERNATE_EXIT,
	UIC_HIBERNATE_ENTER,
	CRYPTO_ENGINE_FATAL_ERROR
};

static const int err_inject_uic_intr_cause_err_codes[] = {
	UFSHCD_UIC_NON_FATAL_ERROR,
	UFSHCD_UIC_DL_PA_INIT_ERROR,

	UFSHCD_UIC_NL_ERROR,
	UFSHCD_UIC_TL_ERROR,
	UFSHCD_UIC_DME_ERROR
};

static const int err_inject_uic_cmd_err_codes[] = {
	UIC_CMD_RESULT_INVALID_ATTR,
	UIC_CMD_RESULT_INVALID_ATTR_VALUE,
	UIC_CMD_RESULT_READ_ONLY_ATTR,
	UIC_CMD_RESULT_WRITE_ONLY_ATTR,
	UIC_CMD_RESULT_BAD_INDEX,
	UIC_CMD_RESULT_LOCKED_ATTR,
	UIC_CMD_RESULT_PEER_COMM_FAILURE,
	UIC_CMD_RESULT_BUSY,
	UIC_CMD_RESULT_DME_FAILURE
};

static const int err_inject_pwr_mode_change_err_codes[] = {
	PWR_OK,
	PWR_REMOTE,
	PWR_BUSY,
	PWR_ERROR_CAP,
	PWR_FATAL_ERROR
};

static const int err_inject_tr_ocs_err_codes[] = {
	OCS_INVALID_CMD_TABLE_ATTR,
	OCS_INVALID_PRDT_ATTR,
	OCS_MISMATCH_DATA_BUF_SIZE,
	OCS_MISMATCH_RESP_UPIU_SIZE,
	OCS_PEER_COMM_FAILURE,
	OCS_ABORTED,
	OCS_FATAL_ERROR,
	OCS_INVALID_COMMAND_STATUS,
};

static const int err_inject_tr_rsp_err_codes[] = {
	TARGET_FAIL,
};

static const int err_inject_scsi_status_err_codes[] = {
	SAM_STAT_CHECK_CONDITION,
	SAM_STAT_CONDITION_MET,
	SAM_STAT_BUSY,
	SAM_STAT_INTERMEDIATE,
	SAM_STAT_TASK_SET_FULL,
	SAM_STAT_ACA_ACTIVE,
	SAM_STAT_TASK_ABORTED,
};

static const int err_inject_tm_ocs_err_codes[] = {
	TM_OCS_INVALID_TM_FUNCTION_ATTR,
	TM_OCS_MISMATCH_TM_REQUEST_SIZE,
	TM_OCS_MISMATCH_TM_RESPONSE_SIZE,
	TM_OCS_PEER_COMM_FAILURE,
	TM_OCS_ABORTED,
	TM_OCS_FATAL_ERROR,
	TM_OCS_INVALID_OCS_VALUE,
};

static const int err_inject_tm_rsp_err_codes[] = {
	UPIU_TASK_MANAGEMENT_FUNC_COMPL,
	UPIU_TASK_MANAGEMENT_FUNC_NOT_SUPPORTED,
	UPIU_TASK_MANAGEMENT_FUNC_FAILED,
	UPIU_INCORRECT_LOGICAL_UNIT_NO,
};

static const int err_inject_debug_ctrl_intr[] = {
	BIT_UFS_DEV_TMT_INTR,
};

static struct ufsdbg_err_scenario err_scen_arr[] = {
	{
		"ERR_INJECT_INTR",
		err_inject_intr_err_codes,
		ARRAY_SIZE(err_inject_intr_err_codes),
		DEFAULT_ERR_INDEX,
	},
	{
		"ERR_INJECT_UIC_INTR_CAUSE",
		err_inject_uic_intr_cause_err_codes,
		ARRAY_SIZE(err_inject_uic_intr_cause_err_codes),
		DEFAULT_ERR_INDEX,
	},
	{
		"ERR_INJECT_UIC_CMD_ERR",
		err_inject_uic_cmd_err_codes,
		ARRAY_SIZE(err_inject_uic_cmd_err_codes),
		DEFAULT_ERR_INDEX,
	},
	{
		"ERR_INJECT_PWR_MODE_CHANGE_ERR",
		err_inject_pwr_mode_change_err_codes,
		ARRAY_SIZE(err_inject_pwr_mode_change_err_codes),
		DEFAULT_ERR_INDEX,
	},
	{
		"ERR_INJECT_TRANSFER_OCS",
		err_inject_tr_ocs_err_codes,
		ARRAY_SIZE(err_inject_tr_ocs_err_codes),
		DEFAULT_ERR_INDEX,
	},
	{
		"ERR_INJECT_TRANSFER_RSP",
		err_inject_tr_rsp_err_codes,
		ARRAY_SIZE(err_inject_tr_rsp_err_codes),
		DEFAULT_ERR_INDEX,
	},
	{
		"ERR_INJECT_SCSI_STATUS",
		err_inject_scsi_status_err_codes,
		ARRAY_SIZE(err_inject_scsi_status_err_codes),
		DEFAULT_ERR_INDEX,
	},
	{
		"ERR_INJECT_TM_OCS",
		err_inject_tm_ocs_err_codes,
		ARRAY_SIZE(err_inject_tm_ocs_err_codes),
		DEFAULT_ERR_INDEX,
	},
	{
		"ERR_INJECT_TM_RSP",
		err_inject_tm_rsp_err_codes,
		ARRAY_SIZE(err_inject_tm_rsp_err_codes),
		DEFAULT_ERR_INDEX,
	},
	{
		"ERR_INJECT_DEDBUG_CTRL_INTR",
		err_inject_debug_ctrl_intr,
		ARRAY_SIZE(err_inject_debug_ctrl_intr),
		DEFAULT_ERR_INDEX,
	},

};

static bool inject_fatal_err_tr(struct ufs_hba *hba, u8 ocs_err)
{
	int tag;

	/* clear all outstanding requests */
	for_each_set_bit(tag, &hba->outstanding_reqs, hba->nutrs) {
		ufshcd_writel(
			hba, ~(1UL << tag), REG_UTP_TRANSFER_REQ_LIST_CLEAR);
		(&hba->lrb[tag])->utr_descriptor_ptr->header.dword_2 =
			cpu_to_be32(ocs_err);
	}
	/* fatal error injected */
	return true;
}

static bool inject_fatal_err_tm(struct ufs_hba *hba, u8 ocs_err)
{
	int tag;

	/* clear all outstanding tasks */
	for_each_set_bit(tag, &hba->outstanding_tasks, hba->nutrs) {
		ufshcd_writel(hba, ~(1U << tag), REG_UTP_TASK_REQ_LIST_CLEAR);
		(&hba->utmrdl_base_addr[tag])->header.dword_2 =
			cpu_to_be32(ocs_err);
	}
	/* fatal error injected */
	return 1;
}

static bool inject_cmd_hang_tr(struct ufs_hba *hba)
{
	int tag;

	tag = find_first_bit(&hba->outstanding_reqs, hba->nutrs);
	if (tag == hba->nutrs)
		return 0;

	__clear_bit(tag, &hba->outstanding_reqs);
	hba->lrb[tag].cmd = NULL;
	__clear_bit(tag, &hba->lrb_in_use);

	/* command hang injected */
	return 1;
}

static int inject_cmd_hang_tm(struct ufs_hba *hba)
{
	int tag;

	tag = find_first_bit(&hba->outstanding_tasks, hba->nutmrs);
	if (tag == hba->nutmrs)
		return 0;

	__clear_bit(tag, &hba->outstanding_tasks);
	__clear_bit(tag, &hba->tm_slots_in_use);

	/* command hang injected */
	return 1;
}

static void ufsdbg_intr_fail_request(struct ufs_hba *hba, u32 *intr_status)
{
	u8 ocs_err;

	dev_err(hba->dev, "%s: fault-inject error: 0x%x\n", __func__,
		*intr_status);

	switch (*intr_status) {
	case DEVICE_FATAL_ERROR: /* fall through */
		ocs_err = OCS_FATAL_ERROR;
		goto handle_fatal_err;
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
	case CRYPTO_ENGINE_FATAL_ERROR:
		ocs_err = OCS_GENERAL_CRYPTO_ERROR;
		goto handle_fatal_err;
#endif
	case CONTROLLER_FATAL_ERROR: /* fall through */
		ocs_err = OCS_FATAL_ERROR;
		goto handle_fatal_err;
	case SYSTEM_BUS_FATAL_ERROR:
		ocs_err = OCS_INVALID_CMD_TABLE_ATTR;
		goto handle_fatal_err;
	case UIC_HIBERNATE_ENTER:
	case UIC_HIBERNATE_EXIT:
		ocs_err = OCS_FATAL_ERROR;
handle_fatal_err:
		/* clear UTRLRSR & UTMRLRS */
		ufshcd_writel(hba, 0, REG_UTP_TRANSFER_REQ_LIST_RUN_STOP);
		ufshcd_writel(hba, 0, REG_UTP_TASK_REQ_LIST_RUN_STOP);
		if (!inject_fatal_err_tr(hba, ocs_err))
			if (!inject_fatal_err_tm(hba, ocs_err))
				goto out;
		break;
	case CMD_HANG_ERROR:
		if (!inject_cmd_hang_tr(hba))
			inject_cmd_hang_tm(hba);
		break;
	case UIC_ERROR:
		/* do nothing */
		break;
	default:
		dev_err(hba->dev, "invalid fault interrupt");
		/* some configurations ignore panics caused by BUG() */
		break;
	}
out:
	return;
}

static bool ufsdbg_find_err_code(
	enum ufsdbg_err_inject_scenario usecase, int *ret, u32 *index)
{
	struct ufsdbg_err_scenario *err_scen = &err_scen_arr[usecase];
	u32 err_code_index;

	if (!err_scen->num_err_codes)
		return false;

	if (err_scen->err_code_index < err_scen->num_err_codes) {
		err_code_index = err_scen->err_code_index;
	} else if (err_scen->err_code_index == DEFAULT_ERR_INDEX) {
		err_code_index = prandom_u32() % err_scen->num_err_codes;
	} else {
		pr_err("ufs fault inject invalid err code\n");
		return false;
	}
	*index = err_code_index;
	*ret = err_scen->err_code_arr[err_code_index];
	return true;
}

static bool ufsdbg_should_fail(struct ufs_hba *hba,
	enum ufsdbg_err_inject_scenario usecase, int success_value,
	const int *ret_value, unsigned int *opt_ret, u32 *err_code_index)
{
	/* sanity check and verify error scenario bit */
	if ((unlikely(!hba || !ret_value)) ||
		(likely(!(fault_inject_files.err_inj_scenario_mask &
			  BIT(usecase)))))
		return false;

	if (hba->ufshcd_state != UFSHCD_STATE_OPERATIONAL)
		return false;

	if (usecase >= ERR_INJECT_MAX_ERR_SCENARIOS) {
		dev_err(hba->dev, "%s: invalid usecase value (%d)\n", __func__,
			usecase);
		return false;
	}

	if (!ufsdbg_find_err_code(usecase, opt_ret, err_code_index))
		return false;

	if (usecase == ERR_INJECT_DEDBUG_CTRL_INTR &&
		*err_code_index == BIT_UFS_DEV_TMT_INTR &&
		(!(hba->caps & UFSHCD_CAP_DEV_TMT_INTR)))
		return false;

	if (usecase == ERR_INJECT_INTR &&
		((*err_code_index == UIC_HIBERNATE_EXIT) ||
			(*err_code_index == UIC_HIBERNATE_ENTER)) &&
		(!ufshcd_is_auto_hibern8_allowed(hba)))
		return false;

	if (!should_fail(&fault_inject_files.fail_attr, 1))
		return false;

	/* if an error already occurred/injected */
	if (*ret_value != success_value)
		return false;

	return true;
}

void ufsdbg_error_inject_dispatcher(struct ufs_hba *hba,
	enum ufsdbg_err_inject_scenario usecase, int success_value,
	unsigned int *ret_value)
{
	unsigned int opt_ret;
	u32 err_code_index;

	if (!ufsdbg_should_fail(hba, usecase, success_value, ret_value,
		    &opt_ret, &err_code_index))
		goto out;

	switch (usecase) {
	case ERR_INJECT_INTR:
		/* an error already occurred */
		if (*ret_value & UFSHCD_ERROR_MASK)
			goto out;

		ufsdbg_intr_fail_request(hba, (u32 *)&opt_ret);
		opt_ret |= *ret_value;
		break;
	case ERR_INJECT_UIC_INTR_CAUSE:
		opt_ret |= *ret_value;
		break;
	case ERR_INJECT_UIC_CMD_ERR:
		opt_ret |= *ret_value;
		break;
	case ERR_INJECT_PWR_MODE_CHANGE_ERR:
		/* delay 200 ms for pwr_mode_change completed */
		mdelay(200);
		break;
	case ERR_INJECT_TRANSFER_OCS:
		break;
	case ERR_INJECT_TRANSFER_RSP:
		/* shift 8 for ret_value field */
		opt_ret = *ret_value | (opt_ret << 8);
		break;
	case ERR_INJECT_SCSI_STATUS:
		opt_ret |= *ret_value;
		break;
	case ERR_INJECT_TM_OCS:
	case ERR_INJECT_TM_RSP:
	case ERR_INJECT_DEDBUG_CTRL_INTR:
		if (opt_ret == BIT_UFS_DEV_TMT_INTR)
			ufshcd_disable_run_stop_reg(hba);
		break;
	case ERR_INJECT_MAX_ERR_SCENARIOS:
		break;
	default:
		dev_err(hba->dev, "%s: unsupported error scenario\n", __func__);
		goto out;
	}

	dev_err(hba->dev, "%s: Index [%u], Scenario: \"%s\"\n", __func__,
		err_code_index, err_scen_arr[usecase].name);
	dev_err(hba->dev,
		"Original Error Code: 0x%x, Injected Error Code: 0x%x\n",
		*ret_value, opt_ret);
	*ret_value = opt_ret;
	err_scen_arr[usecase].num_err_injected++;
out:
	/*
	 * here it's guaranteed that ret_value has the correct value,
	 * whether it was assigned with a new value, or kept its own
	 * original incoming value
	 */
	return;
}

static int ufsdbg_err_inj_scenario_read(struct seq_file *file, void *data)
{
	enum ufsdbg_err_inject_scenario err_case;

	seq_printf(file, "%-40s %-17s %-15s\n", "Error Scenario:", "Bit[#]",
		"STATUS");

	for (err_case = ERR_INJECT_INTR;
	     err_case < ERR_INJECT_MAX_ERR_SCENARIOS; err_case++)
		seq_printf(file, "%-40s 0x%-15lx %-15s\n",
			   err_scen_arr[err_case].name, UFS_BIT(err_case),
			   fault_inject_files.err_inj_scenario_mask &
					   UFS_BIT(err_case) ?
				   "ENABLE" :
				   "DISABLE");

	seq_printf(file, "bitwise of error scenario is 0x%x\n\n",
		fault_inject_files.err_inj_scenario_mask);

	seq_puts(file, "usage example:\n");
	seq_puts(file, "echo 0x1 > /sys/kernel/debug/ufs_fault_inject/err_inj_scenario\n");
	seq_puts(file, "in order to enable ERR_INJECT_INTR\n");

	return 0;
}

static int ufsdbg_err_inj_scenario_open(struct inode *inode, struct file *file)
{
	return single_open(
		file, ufsdbg_err_inj_scenario_read, inode->i_private);
}

static ssize_t ufsdbg_err_inj_scenario_write(struct file *file,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	int ret;
	u32 err_scen;

	ret = kstrtoint_from_user(ubuf, cnt, 0, &err_scen);
	if (ret) {
		pr_err("%s: Invalid argument\n", __func__);
		return ret;
	}

	fault_inject_files.err_inj_scenario_mask = err_scen;

	return cnt;
}

static const struct file_operations ufsdbg_err_inj_scenario_ops = {
	.open = ufsdbg_err_inj_scenario_open,
	.read = seq_read,
	.write = ufsdbg_err_inj_scenario_write,
	.release = single_release,
};

static int ufsdbg_err_inj_stats_read(struct seq_file *file, void *data)
{
	int err;

	seq_printf(file, "%-40s %-20s\n", "Error Scenario:",
		"Num of Errors Injected");

	for (err = 0; err < ERR_INJECT_MAX_ERR_SCENARIOS; err++)
		seq_printf(file, "%-40s %u\n", err_scen_arr[err].name,
			err_scen_arr[err].num_err_injected);

	return 0;
}

static int ufsdbg_err_inj_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufsdbg_err_inj_stats_read, inode->i_private);
}

static ssize_t ufsdbg_err_inj_stats_write(struct file *file,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	int err;

	for (err = 0; err < ERR_INJECT_MAX_ERR_SCENARIOS; err++)
		err_scen_arr[err].num_err_injected = 0;

	return cnt;
}

static const struct file_operations ufsdbg_err_inj_stats_ops = {
	.open = ufsdbg_err_inj_stats_open,
	.read = seq_read,
	.write = ufsdbg_err_inj_stats_write,
	.release = single_release,
};

static int ufsdbg_err_inj_err_index_read(struct seq_file *file, void *data)
{
	int scenario;
	int ec_index;

	seq_puts(file,
		"Selected Scenarios and Error Codes are as follows:\n\n");
	for (scenario = 0; scenario < ERR_INJECT_MAX_ERR_SCENARIOS;
		scenario++) {
		ec_index = err_scen_arr[scenario].err_code_index;
		if ((ec_index >= 0) &&
			(ec_index < err_scen_arr[scenario].num_err_codes))
			seq_printf(file,
				"Scenario: %-35s(Index: %d), Error Code: %d(Index: %d)\n",
				err_scen_arr[scenario].name, scenario,
				err_scen_arr[scenario].err_code_arr[ec_index],
				ec_index);
	}
	seq_puts(file, "\n\nUsage:\n"
		       "To select a specific fault, write 'S E' to err_inj_index, where:\n"
		       " S - scenario index\n"
		       " E - error code index of the scenario\n"
		       "     All available error code index:\n");
	for (scenario = 0; scenario < ERR_INJECT_MAX_ERR_SCENARIOS; scenario++)
		seq_printf(file,
			"     Scenario: %-35s(Index: %d), Error Code Index Range: [0, %u]\n",
			err_scen_arr[scenario].name, scenario,
			err_scen_arr[scenario].num_err_codes - 1);
	seq_puts(file, "\nTo select a random fault, write 'S 0xFFFFFFFF' to err_inj_index:\n\n");

	seq_puts(file, "usage example:\n");
	seq_puts(file, "echo 0x1 > /sys/kernel/debug/ufs_fault_inject/err_inj_scenario\n");
	seq_puts(file, "echo 0 1 > /sys/kernel/debug/ufs_fault_inject/err_inj_index\n");
	seq_puts(file, "to enable SYSTEM_BUS_FATAL_ERROR in scenario ERR_INJECT_INTR\n");
	return 0;
}

static int ufsdbg_err_inj_err_index_open(struct inode *inode, struct file *file)
{
	return single_open(
		file, ufsdbg_err_inj_err_index_read, inode->i_private);
}

static ssize_t ufsdbg_err_inj_err_index_write(struct file *file,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	int ret;
	int scenario;
	int err_code_index;
	char err_index_str[BUFFER_SIZE] = {0};
	loff_t buff_pos = 0;

	ret = simple_write_to_buffer(
		err_index_str, BUFFER_SIZE, &buff_pos, ubuf, cnt);
	if (ret <= 0)
		return cnt;

	ret = sscanf(err_index_str, "%d %d", &scenario, &err_code_index);
	if (ret == -1) {
		pr_err("%s: Error parsing scenario and err_code_index\n",
			__func__);
		return cnt;
	}

	if ((scenario < 0) || (scenario >= ERR_INJECT_MAX_ERR_SCENARIOS)) {
		pr_err("%s: Invalid scenario: scenario=%d, valid range: [0,%d)\n",
			__func__, scenario, ERR_INJECT_MAX_ERR_SCENARIOS);
		return cnt;
	}
	if ((err_code_index < 0) ||
		(err_code_index >= err_scen_arr[scenario].num_err_codes)) {
		pr_info("%s: Invalid err_code_index: err_code_index=%d, valid range: [0,%u)\n",
			__func__, err_code_index,
			err_scen_arr[scenario].num_err_codes);
		pr_info("%s: Err_code_index if secenario %d is set to 0x%x\n",
			__func__, scenario, DEFAULT_ERR_INDEX);
		err_scen_arr[scenario].err_code_index = DEFAULT_ERR_INDEX;
	} else {
		err_scen_arr[scenario].err_code_index = err_code_index;
	}
	return cnt;
}

static const struct file_operations ufsdbg_err_inj_err_index_ops = {
	.open = ufsdbg_err_inj_err_index_open,
	.read = seq_read,
	.write = ufsdbg_err_inj_err_index_write,
	.release = single_release,
};

static void ufsdbg_setup_fault_injection(void)
{
	struct dentry *fault_dir = NULL;

	fault_inject_files.fail_attr = fail_default_attr;

	if (fail_request)
		setup_fault_attr(&fault_inject_files.fail_attr, fail_request);

	/* suppress dump stack every time failure is injected */
	fault_inject_files.fail_attr.verbose = 0;

	fault_dir = fault_create_debugfs_attr("inject_fault",
		fault_inject_files.debugfs_root, &fault_inject_files.fail_attr);
	if (IS_ERR(fault_dir)) {
		pr_err("%s: failed to create debugfs entry for faultinjection\n",
			__func__);
		return;
	}

	fault_inject_files.err_inj_scenario =
		debugfs_create_file("err_inj_scenario", S_IRUGO | S_IWUGO,
			fault_inject_files.debugfs_root, NULL,
			&ufsdbg_err_inj_scenario_ops);

	if (!fault_inject_files.err_inj_scenario) {
		pr_err("%s: Could not create debugfs entry for err_scenario",
			__func__);
		goto fail_err_inj_scenario;
	}

	fault_inject_files.err_inj_stats = debugfs_create_file("err_inj_stats",
		S_IRUSR | S_IWUSR, fault_inject_files.debugfs_root, NULL,
		&ufsdbg_err_inj_stats_ops);
	if (!fault_inject_files.err_inj_stats) {
		pr_err("%s:  failed create err_inj_stats debugfs entry\n",
			__func__);
		goto fail_err_inj_stats;
	}

	fault_inject_files.err_inj_err_index =
		debugfs_create_file("err_inj_index", S_IRUSR | S_IWUSR,
			fault_inject_files.debugfs_root, NULL,
			&ufsdbg_err_inj_err_index_ops);
	if (!fault_inject_files.err_inj_err_index) {
		pr_err("%s:  failed create err_inj_index debugfs entry\n",
			__func__);
		goto fail_err_inj_err_index;
	}
	return;

fail_err_inj_err_index:
	debugfs_remove(fault_inject_files.err_inj_stats);
fail_err_inj_stats:
	debugfs_remove(fault_inject_files.err_inj_scenario);
fail_err_inj_scenario:
	debugfs_remove_recursive(fault_dir);
}

void ufs_fault_inject_fs_setup(void)
{
	fault_inject_files.debugfs_root =
		debugfs_create_dir("ufs_fault_inject", NULL);
	if (IS_ERR(fault_inject_files.debugfs_root))
		/* Don't complain -- debugfs just isn't enabled */
		goto err_no_root;
	if (!fault_inject_files.debugfs_root) {
		/*
		 * Complain -- debugfs is enabled, but it failed to
		 * create the directory
		 */
		pr_err("%s: NULL debugfs root directory, exiting", __func__);
		goto err_no_root;
	}

	ufsdbg_setup_fault_injection();

	return;

err_no_root:
	pr_err("%s: failed to initialize debugfs\n", __func__);
}

void ufs_fault_inject_fs_remove(void)
{
	debugfs_remove_recursive(fault_inject_files.debugfs_root);
}

#endif /* End of CONFIG_SCSI_UFS_FAULT_INJECT */
