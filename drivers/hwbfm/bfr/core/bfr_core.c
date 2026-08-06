/*
 * bfr_core.c
 *
 * define the core's external public enum/macros/interface for
 * BFR(Boot Fail Recovery)
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <chipset_common/bfmr/bfr/chipsets/bfr_chipsets.h>
#include <chipset_common/bfmr/bfr/core/bfr_core.h>
#include <chipset_common/bfmr/common/bfmr_common.h>
#include <linux/init.h>
#include <linux/kernel.h>

#define BFR_RECORD_COPIES_MAX_COUNT 2
#define BFR_RRECORD_READ_MAX_COUNT 10
#define BF_0_TIME 0
#define BF_1_TIME 1
#define BF_2_TIME 2
#define BF_3_TIME 3
#define BF_4_TIME 4
#define MISC_CMD_SIZE 32
#define MISC_STATUS_SIZE 32
#define MISC_REC_SIZE 1024
#define is_package_install_succ(running_status_code)               \
	((running_status_code) == (RMRSC_ERECOVERY_INSTALL_PACKAGES_SUCCESS))
#define is_factory_reset(factory_reset_flag)                          \
	((factory_reset_flag) == (BFR_FACTORY_RESET_DONE))
#define is_recovery_succ(result)                         \
	((result) == (BF_RECOVERY_SUCCESS))
#define is_download_succ(recovery_method, running_status_code)     \
	(bfr_is_download_recovery_method(recovery_method) &&       \
	is_package_install_succ(running_status_code))

struct bfr_misc_msg {
	char command[MISC_CMD_SIZE];
	char status[MISC_STATUS_SIZE];
	char recovery[MISC_REC_SIZE];
};

struct bfr_method_param {
	unsigned int boot_fail_stage;
	unsigned int boot_fail_no;
	bfr_suggested_recovery_method_e method;
	int latest_record_count;
	int failed_times;
	int times_bottom;
	int times_app;
	int times_after_download;
	int safe_mode_times;
	bool not_parse_further;
	bool has_fixed_method;
	bool is_download_succ;
	bool is_hardware_degrade;
	bfr_recovery_method_e fixed_method;
	bfr_recovery_method_e recovery_method;
	bfr_recovery_record_t latest_rrecord[BFR_RRECORD_READ_MAX_COUNT];
	char *bootfail_detail_info;
};

struct bfr_method_desc {
	bfr_recovery_method_e method;
	char *desc;
};

struct bfr_safemode_recovery_param {
	unsigned int bootfail_errno;
	bool has_safemode_recovery_method;
};

struct bfr_rrecord_param {
	int start_idx;
	int count;
};

static bfr_enter_erecovery_reason_map_t erecovery_map[] = {
	{
		{ BL1_ERRNO_START, BL1_PL1_START - 1 },
		ENTER_ERECOVERY_REASON_BECAUSE_BOOTLOADER_BOOT_FAIL
	}, {
		{ BL1_PL1_START, BL1_PL2_START - 1 },
		ENTER_ERECOVERY_REASON_BECAUSE_BOOTLOADER_BOOT_FAIL
	}, {
		{ BL1_PL2_START, BL2_ERRNO_START - 1},
		ENTER_ERECOVERY_REASON_BECAUSE_BOOTLOADER_BOOT_FAIL
	}, {
		{ BL2_ERRNO_START, BL2_PL1_START - 1 },
		ENTER_ERECOVERY_REASON_BECAUSE_BOOTLOADER_BOOT_FAIL
	}, {
		{ BL2_PL1_START, BL2_PL2_START - 1 },
		ENTER_ERECOVERY_REASON_BECAUSE_BOOTLOADER_BOOT_FAIL
	}, {
		{ BL2_PL2_START, KERNEL_ERRNO_START - 1 },
		ENTER_ERECOVERY_REASON_BECAUSE_BOOTLOADER_BOOT_FAIL
	}, {
		{ KERNEL_ERRNO_START, KERNEL_PL1_START - 1 },
		ENTER_ERECOVERY_REASON_BECAUSE_KERNEL_BOOT_FAIL
	}, {
		{ KERNEL_PL1_START, KERNEL_PL2_START - 1 },
		ENTER_ERECOVERY_REASON_BECAUSE_KERNEL_BOOT_FAIL
	}, {
		{ KERNEL_PL2_START, NATIVE_ERRNO_START - 1 },
		ENTER_ERECOVERY_REASON_BECAUSE_KERNEL_BOOT_FAIL
	}, {
		{ SYSTEM_MOUNT_FAIL, SYSTEM_MOUNT_FAIL},
		ENTER_ERECOVERY_BECAUSE_SYSTEM_MOUNT_FAILED
	}, {
		{ SECURITY_FAIL, SECURITY_FAIL},
		ENTER_ERECOVERY_BECAUSE_SECURITY_FAIL
	}, {
		{ CRITICAL_SERVICE_FAIL_TO_START,
		  CRITICAL_SERVICE_FAIL_TO_START },
		ENTER_ERECOVERY_BECAUSE_KEY_PROCESS_START_FAILED
	}, {
		{ DATA_MOUNT_FAILED_AND_ERASED, DATA_MOUNT_FAILED_AND_ERASED },
		ENTER_ERECOVERY_BECAUSE_DATA_MOUNT_FAILED
	}, {
		{ DATA_MOUNT_RO, DATA_MOUNT_RO },
		ENTER_ERECOVERY_BECAUSE_DATA_MOUNT_RO
	}, {
		{ VENDOR_MOUNT_FAIL, VENDOR_MOUNT_FAIL },
		ENTER_ERECOVERY_BECAUSE_VENDOR_MOUNT_FAILED
	}, {
		{ NATIVE_ERRNO_START, PACKAGE_MANAGER_SETTING_FILE_DAMAGED },
		ENTER_ERECOVERY_BECAUSE_APP_BOOT_FAIL
	},
};

static bfr_recovery_policy_e fixed_policy[] = {
	{
		SYSTEM_MOUNT_FAIL,
		1,
		{
			{ 1, FRM_DOWNLOAD },
			{ 1, FRM_DOWNLOAD },
			{ 1, FRM_DOWNLOAD },
			{ 1, FRM_DOWNLOAD }
		},
	}, {
		VENDOR_MOUNT_FAIL,
		1,
		{
			{ 1, FRM_DOWNLOAD },
			{ 1, FRM_DOWNLOAD },
			{ 1, FRM_DOWNLOAD },
			{ 1, FRM_DOWNLOAD }
		},
	}, {
		CRITICAL_SERVICE_FAIL_TO_START,
		1,
		{
			{ 1, FRM_REBOOT },
			{ 1, FRM_FACTORY_RST },
			{ 1, FRM_DOWNLOAD },
			{ 1, FRM_DOWNLOAD }
		},
	}, {
		DATA_MOUNT_FAILED_AND_ERASED,
		1,
		{
			{ 1, FRM_REBOOT },
			{ 1, FRM_LOWLEVEL_FORMAT_DATA },
			{ 1, FRM_DOWNLOAD },
			{ 1, FRM_DOWNLOAD }
		},
	}, {
		DATA_MOUNT_RO,
		1,
		{
			{ 1, FRM_REBOOT },
			{ 1, FRM_LOWLEVEL_FORMAT_DATA },
			{ 1, FRM_DOWNLOAD },
			{ 1, FRM_DOWNLOAD }
		},
	}, {
		DATA_NOSPC,
		1,
		{
			{ 1, FRM_DEL_FILES_FOR_NOSPC },
			{ 1, FRM_FACTORY_RST },
			{ 1, FRM_DOWNLOAD },
			{ 1, FRM_DOWNLOAD }
		},
	}, {
		FRK_USER_DATA_DAMAGED,
		1,
		{
			{ 1, FRM_REBOOT },
			{ 1, FRM_LOWLEVEL_FORMAT_DATA },
			{ 1, FRM_LOWLEVEL_FORMAT_DATA },
			{ 1, FRM_LOWLEVEL_FORMAT_DATA }
		},
	}, {
		KERNEL_BOOT_TIMEOUT,
		1,
		{
			{ 1, FRM_REBOOT },
			{ 1, FRM_FACTORY_RST },
			{ 1, FRM_DOWNLOAD },
			{ 1, FRM_DOWNLOAD }
		},
	}, {
		FRK_USER_DATA_DAMAGED,
		1,
		{
			{ 1, FRM_REBOOT },
			{ 1, FRM_LOWLEVEL_FORMAT_DATA },
			{ 1, FRM_LOWLEVEL_FORMAT_DATA },
			{ 1, FRM_LOWLEVEL_FORMAT_DATA }
		},
	}, {
		BFM_HARDWARE_FAULT,
		1,
		{
			{ 1, FRM_REBOOT },
			{ 1, FRM_DO_NOTHING },
			{ 1, FRM_DO_NOTHING },
			{ 1, FRM_DO_NOTHING }
		},
	}
};

static struct bfr_record_param rrecord_param[BFR_RECORD_COPIES_MAX_COUNT];
static struct bfr_method_desc method_desc[] = {
	{
		FRM_DO_NOTHING,
		"do nothing"
	}, {
		FRM_REBOOT,
		"reboot"
	}, {
		FRM_GOTO_B_SYSTEM,
		"goto B system"
	}, {
		FRM_DEL_FILES_FOR_BF,
		"del files in /data part"
	}, {
		FRM_DEL_FILES_FOR_NOSPC,
		"del files in /data part because of no space"
	}, {
		FRM_FACTORY_RST,
		"recommend user to do factory reset"
	}, {
		FRM_FORMAT_DATA_PART,
		"recommend user to format /data part"
	}, {
		FRM_DOWNLOAD,
		"download and recovery"
	}, {
		FRM_DOWNLOAD_AND_DEL_FILES,
		"recommend to del files in /data after download and recovery"
	}, {
		FRM_DOWNLOAD_AND_FACTORY_RST,
		"recommend to do factory reset after download and recovery"
	}, {
		FRM_NOTIFY_USER_FAIL,
		"recovery failed, the boot fault can't be recoveried by BFRM"
	}, {
		FRM_LOWLEVEL_FORMAT_DATA,
		"recommend user to do low-level formatting data"
	}, {
		FRM_ENTER_SAFE_MODE,
		"enter android safe mode to uninstall the third party APK"
	}, {
		FRM_FACTORY_RST_AFTER_DOWNLOAD,
		"factory reset after download and recovery"
	}, {
		FRM_BOPD,
		"BoPD"
	}, {
		FRM_STORAGE_RDONLY_BOPD,
		"storage read only and BoPD"
	}, {
		FRM_HW_DEGRADE_BOPD,
		"hardware degrade BoPD"
	}, {
		FRM_STORAGE_RDONLY_HW_DEGRADE_BOPD,
		"storage read only and hardware degrade BoPD"
	}, {
		FRM_BOPD_AFTER_DOWNLOAD,
		"BoPD after download and recovery"
	}, {
		FRM_HW_DEGRADE_BOPD_AFTER_DOWNLOAD,
		"Hardware degrade BoPD after download and recovery"
	},
};

static struct bfr_safemode_recovery_param safemode_param[] = {
	{ KERNEL_AP_PANIC, true },
	{ KERNEL_PRESS10S, true },
	{ KERNEL_AP_WDT, true },
	{ KERNEL_AP_S_ABNORMAL, true },
	{ KERNEL_BOOT_TIMEOUT, true },
	{ CRITICAL_SERVICE_FAIL_TO_START, true },
};
static bool recovery_record_has_been_read_and_verified;

static bool bfr_has_fixed_method(struct bfr_method_param *pselect_param);
static bfr_recovery_method_running_status_e bfr_method_init(
		bfr_recovery_method_e recovery_method);
static bfr_recovery_method_run_result_e bfr_method_result(
		bfr_recovery_method_e recovery_method);
static int select_with_safemode(struct bfr_method_param *pselect_param);
static int select_without_safemode(struct bfr_method_param *pselect_param);
static bfr_boot_fail_stage_e get_bootfail_stage(unsigned int boot_fail_stage);
static int run_method(struct bfr_method_param *pselect_param);
static int set_misc_msg(void);
static int init_rrecord_param(void);
static int read_rrecord_to_buf(void);
static int verify_rrecord(void);
static int read_and_verify_rrecord(void);
static int init_rrecord_header(void);
static int read_rrecord(bfr_recovery_record_t *precord,
		int record_count, int *actually_read);
static int create_rrecord(bfr_recovery_record_t *precord);
static int renew_rrecord(bfr_recovery_record_t *precord);
static bool has_safemode_method(unsigned int bootfail_errno);
static bool need_factory_reset(struct bfr_method_param *pselect_param);
static unsigned int get_bootfail_uptime(void);

static bool has_safemode_method(unsigned int bootfail_errno)
{
	unsigned int i;
	unsigned int count = ARRAY_SIZE(safemode_param);

	if (bfr_bopd_has_been_enabled()) {
		BFMR_PRINT_ERR("BoPD is enabled! safe mode can't be used!\n");
		return false;
	}

	for (i = 0; i < count; i++) {
		if (safemode_param[i].bootfail_errno == bootfail_errno)
			return safemode_param[i].has_safemode_recovery_method;
	}

	return false;
}

static bool bfr_is_hardware_fault(unsigned int bootfail_no)
{
	return ((bfmr_bootfail_errno_e)bootfail_no == BFM_HARDWARE_FAULT);
}

static void update_rmethod_for_hw_fault(struct bfr_method_param *select_param,
		bfmr_get_hw_fault_info_param_t *fault_info_param)
{
	switch (bfr_get_hardware_fault_times(fault_info_param)) {
	case 0:
		if (bfmr_is_boot_success(select_param->boot_fail_stage))
			select_param->recovery_method = FRM_DO_NOTHING;
		else
			select_param->recovery_method = FRM_REBOOT;
		break;
	case 1:
		select_param->recovery_method = FRM_DO_NOTHING;
		break;
	default:
		select_param->recovery_method = FRM_DO_NOTHING;
		break;
	}
}

static bool has_fixed_method_for_hw_fault(struct bfr_method_param *select_param)
{
	bfmr_get_hw_fault_info_param_t *fault_info_param = NULL;
	struct bfr_method_param *param = select_param;
	unsigned int tmp_num;

	param->not_parse_further = true;
	param->fixed_method = FRM_DO_NOTHING;
	if (param->bootfail_detail_info == NULL)
		return true;

	fault_info_param = bfmr_malloc(sizeof(*fault_info_param));
	if (fault_info_param == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		return true;
	}
	memset(fault_info_param, 0, sizeof(*fault_info_param));

	fault_info_param->fault_stage =
		bfmr_is_boot_success(param->boot_fail_stage) ?
				    HW_FAULT_STAGE_AFTER_BOOT_SUCCESS :
				    HW_FAULT_STAGE_DURING_BOOTUP;
	tmp_num = sizeof(fault_info_param->hw_excp_info.ocp_excp_info.ldo_num);
	memcpy(fault_info_param->hw_excp_info.ocp_excp_info.ldo_num,
		param->bootfail_detail_info,
		BFMR_MIN(tmp_num - 1, strlen(param->bootfail_detail_info)));
	update_rmethod_for_hw_fault(param, fault_info_param);

	/* release memory */
	if (fault_info_param != NULL) {
		bfmr_free(fault_info_param);
		fault_info_param = NULL;
	}

	param->fixed_method = param->recovery_method;
	return true;
}

static void update_fixed_method(struct bfr_method_param *select_param,
		int index)
{
	struct bfr_method_param *param = select_param;
	int tmp_times = param->times_app;
	int method_count = ARRAY_SIZE(fixed_policy[index].param);

	if ((tmp_times <= 0) || (tmp_times > method_count)) {
		param->fixed_method = FRM_DOWNLOAD;
		BFMR_PRINT_KEY_INFO(
		    "ErrNo: %x fail_times_in_app: %d recovery_method: %d\n",
		    (unsigned int)param->boot_fail_no,
		    tmp_times, param->fixed_method);
		return;
	}

	if (fixed_policy[index].param[tmp_times - 1].enable_this_method != 0)
		param->fixed_method =
		    fixed_policy[index].param[tmp_times - 1].recovery_method;
	else
		param->fixed_method = FRM_DO_NOTHING;

	BFMR_PRINT_KEY_INFO("ErrNo: %x recovery_method: %x\n",
			    param->boot_fail_no,
			    param->fixed_method);
}

static bool bfr_has_fixed_method(struct bfr_method_param *select_param)
{
	int count = ARRAY_SIZE(fixed_policy);
	int i;

	if (bfr_is_hardware_fault(select_param->boot_fail_no))
		return has_fixed_method_for_hw_fault(select_param);

	for (i = 0; i < count; i++) {
		if (fixed_policy[i].boot_fail_no != select_param->boot_fail_no)
			continue;

		if (fixed_policy[i].has_fixed_policy == 0)
			break;
		update_fixed_method(select_param, i);
		return true;
	}

	return false;
}

static unsigned int get_erecovery_reason(unsigned int boot_fail_no)
{
	int i;
	int count = ARRAY_SIZE(erecovery_map);

	for (i = 0; i < count; i++) {
		if ((boot_fail_no >= erecovery_map[i].range.start) &&
		    boot_fail_no <= erecovery_map[i].range.end)
			return erecovery_map[i].enter_erecovery_reason;
	}

	return ENTER_ERECOVERY_UNKNOWN;
}

static int write_misc_msg(bfr_misc_cmd_e cmd_type, struct bfr_misc_msg *pmsg,
		const char *dev_path)
{
	int ret = -1;
	const char *cmd_str = NULL;

	switch (cmd_type) {
	case BFR_MISC_CMD_ERECOVERY:
		cmd_str = BFR_ENTER_ERECOVERY_CMD;
		break;
	case BFR_MISC_CMD_RECOVERY:
		cmd_str = BFR_ENTER_RECOVERY_CMD;
		break;
	default:
		return ret;
	}

	memcpy(pmsg->command, (void *)cmd_str,
		BFMR_MIN(strlen(cmd_str), sizeof(pmsg->command) - 1));
	ret = bfmr_write_emmc_raw_part(dev_path, 0x0, (char *)pmsg,
					sizeof(*pmsg));

	return ret;
}

static int bfr_set_misc_msg(bfr_misc_cmd_e cmd_type)
{
	int ret = -1;
	struct bfr_misc_msg *pmsg = NULL;
	char *dev_path = NULL;

	dev_path = bfmr_malloc(BFMR_DEV_FULL_PATH_MAX_LEN + 1);
	if (dev_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}
	memset((void *)dev_path, 0, BFMR_DEV_FULL_PATH_MAX_LEN + 1);

	pmsg = bfmr_malloc(sizeof(*pmsg));
	if (pmsg == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}
	memset((void *)pmsg, 0, sizeof(*pmsg));

	ret = bfmr_get_device_full_path(BFR_MISC_PART_NAME, dev_path,
					BFMR_DEV_FULL_PATH_MAX_LEN);
	if (ret != 0) {
		BFMR_PRINT_ERR("get path: %s failed!\n", BFR_MISC_PART_NAME);
		goto __out;
	}

	ret = write_misc_msg(cmd_type, pmsg, dev_path);
	if (ret != 0)
		BFMR_PRINT_ERR("write misc cmd failed!\n");

__out:
	if (pmsg != NULL) {
		bfmr_free(pmsg);
		pmsg = NULL;
	}

	if (dev_path != NULL) {
		bfmr_free(dev_path);
		dev_path = NULL;
	}

	return ret;
}

static int set_misc_msg(void)
{
	return bfr_set_misc_msg(BFR_MISC_CMD_ERECOVERY);
}

static bool need_factory_reset(struct bfr_method_param *param)
{
	bfr_recovery_record_t *tmp_rec = NULL;

	if (param->latest_record_count <= 0) {
		BFMR_PRINT_ERR("recovery record count: %d\n",
			param->latest_record_count);
		return false;
	}

	if (bfr_bopd_has_been_enabled()) {
		BFMR_PRINT_ERR("BoPD enabled! factory reset can't be used!\n");
		return false;
	}

	tmp_rec = &(param->latest_rrecord[param->latest_record_count - 1]);
	BFMR_PRINT_KEY_INFO(
	    "recovery_method: %d, running_status_code: %d, recovery_result: %x, factory_reset_flag:%x\n",
	    tmp_rec->recovery_method, tmp_rec->running_status_code,
	    tmp_rec->recovery_result, tmp_rec->factory_reset_flag);

	if (bfr_is_download_recovery_method(tmp_rec->recovery_method) &&
	    is_package_install_succ(tmp_rec->running_status_code) &&
	    !is_recovery_succ(tmp_rec->recovery_result) &&
	    !is_factory_reset(tmp_rec->factory_reset_flag))
		return true;

	return false;
}

static void update_failed_times(struct bfr_method_param *pselect_param)
{
	int i;

	/* record count is >0 */
	for (i = 0; i < pselect_param->latest_record_count; i++) {
		if (pselect_param->latest_rrecord[i].recovery_result ==
			BF_RECOVERY_SUCCESS) {
			pselect_param->failed_times = 0;
			pselect_param->times_bottom = 0;
			pselect_param->safe_mode_times = 0;
			continue;
		}
		pselect_param->failed_times++;
		if (pselect_param->latest_rrecord[i].boot_fail_stage <
			NATIVE_STAGE_START)
			pselect_param->times_bottom++;

		/* check the count of boot fail has safe mode or not */
		if (has_safemode_method(
		pselect_param->latest_rrecord[i].boot_fail_no))
			pselect_param->safe_mode_times++;
	}
}

static int bfr_parse_boot_fail_info(struct bfr_method_param *pselect_param)
{
	if (need_factory_reset(pselect_param)) {
		pselect_param->recovery_method = FRM_FACTORY_RST_AFTER_DOWNLOAD;
		pselect_param->not_parse_further = true;
		return 0;
	}

	update_failed_times(pselect_param);

	pselect_param->failed_times++;
	if (pselect_param->boot_fail_stage < NATIVE_STAGE_START)
		pselect_param->times_bottom++;

	pselect_param->times_app = pselect_param->failed_times -
					pselect_param->times_bottom;
	BFMR_PRINT_KEY_INFO(
	    "bf_total_times:%d bf_bottom_layer_times: %d bf_app_times: %d\n",
	    pselect_param->failed_times,
	    pselect_param->times_bottom,
	    pselect_param->times_app);

	/* It is the first failure */
	pselect_param->has_fixed_method = bfr_has_fixed_method(pselect_param);
	if (pselect_param->latest_record_count == 0) {
		BFMR_PRINT_KEY_INFO(
		     "System has no valid rrecord, boot fail occurs in %s\n",
		    (pselect_param->boot_fail_stage < NATIVE_STAGE_START) ?
		    ("Bottom layer") : ("APP"));
		pselect_param->recovery_method =
		    (pselect_param->has_fixed_method) ?
		    (pselect_param->fixed_method) : (FRM_REBOOT);
		pselect_param->not_parse_further = true;
		BFMR_PRINT_KEY_INFO("recovery_method: %d\n",
				    pselect_param->recovery_method);
	}

	return 0;
}

static bool bfr_bootfail_need_factory_reset(unsigned int bootfail_no)
{
	return ((bootfail_no == DATA_MOUNT_FAILED_AND_ERASED) ||
		(bootfail_no == FRK_USER_DATA_DAMAGED));
}

static bool bfr_bootfail_need_download(unsigned int bootfail_no)
{
	return ((bootfail_no == SYSTEM_MOUNT_FAIL) ||
		(bootfail_no == VENDOR_MOUNT_FAIL));
}

static bool is_bopd_download_succ(bfr_recovery_record_t *precord,
		struct bfr_method_param *pselect_param)
{
	if ((is_download_succ(precord->recovery_method,
				precord->running_status_code))) {
		pselect_param->is_download_succ = true;

		/* include the bootfail this time */
		pselect_param->times_after_download++;
		return pselect_param->is_download_succ;
	}

	if (precord->boot_fail_stage < KERNEL_STAGE_START)
		pselect_param->times_bottom++;

	pselect_param->times_after_download++;

	return pselect_param->is_download_succ;
}

static int bfr_parse_bootfail_for_bopd(struct bfr_method_param *pselect_param)
{
	int i;
	int j;
	int record_cnt;
	int count_in_ringbuffer_begin = 0;
	int count_in_ringbuffer_end = 0;
	unsigned int header_size = sizeof(bfr_recovery_record_header_t);
	unsigned int record_size = sizeof(bfr_recovery_record_t);
	bfr_recovery_record_header_t *precord_header = NULL;
	bfr_recovery_record_t *precord = NULL;
	struct bfr_rrecord_param parse_param[2];

	pselect_param->times_bottom = 0;
	precord_header = (bfr_recovery_record_header_t *)rrecord_param[0].buf;
	record_cnt = BFMR_MIN(precord_header->rec_cnt_before_boot_succ,
			      precord_header->record_count);
	count_in_ringbuffer_begin =
		(precord_header->next_record_idx >= record_cnt) ?
		(record_cnt) : (precord_header->next_record_idx);
	count_in_ringbuffer_end =
		(precord_header->next_record_idx >= record_cnt) ?
		(0) : (record_cnt - precord_header->next_record_idx);
	parse_param[0].count = count_in_ringbuffer_begin;
	parse_param[0].start_idx = precord_header->next_record_idx;
	parse_param[1].count = count_in_ringbuffer_end;
	parse_param[1].start_idx = precord_header->record_count;
	for (i = 0; i < ARRAY_SIZE(parse_param); i++) {
		bool recovery_successfully = false;
		bool download_successfully = false;

		for (j = 0; j < parse_param[i].count; j++) {
			precord = (bfr_recovery_record_t *)
				  (rrecord_param[0].buf + header_size +
				  (parse_param[i].start_idx - j - 1) *
				  record_size);
			if (precord->recovery_result == BF_RECOVERY_SUCCESS) {
				recovery_successfully = true;
				break;
			}
			if (is_bopd_download_succ(precord, pselect_param)) {
				download_successfully = true;
				break;
			}
		}

		if (recovery_successfully || download_successfully)
			break;
	}

	return 0;
}

static void select_rmethod_times_app_bopd(
		struct bfr_method_param *pselect_param)
{
	switch (pselect_param->times_app) {
	case BF_1_TIME:
	case BF_2_TIME:
		pselect_param->recovery_method = FRM_REBOOT;
		break;
	case BF_3_TIME:
		pselect_param->recovery_method = FRM_BOPD;
		break;
	case BF_4_TIME:
		pselect_param->recovery_method = FRM_HW_DEGRADE_BOPD;
		break;
	default:
		pselect_param->is_hardware_degrade = true;
		pselect_param->recovery_method = FRM_DOWNLOAD;
		break;
	}
}

static int bfr_select_recovery_method_with_bopd(
		struct bfr_method_param *pselect_param)
{
	/* process the special boot fail that need low-level format */
	if (bfr_bootfail_need_factory_reset(pselect_param->boot_fail_no)) {
		pselect_param->recovery_method = FRM_LOWLEVEL_FORMAT_DATA;
		return 0;
	}

	/* process the special boot fail that need download */
	if (bfr_bootfail_need_download(pselect_param->boot_fail_no)) {
		pselect_param->recovery_method = FRM_DOWNLOAD;
		return 0;
	}

	/* 1. parse bootfail for bopd firstly */
	if (bfr_parse_bootfail_for_bopd(pselect_param) != 0) {
		BFMR_PRINT_ERR("bfr_parse_bootfail_for_bopd failed!\n");
		return 0;
	}

	/* 2. use the result processed by bfr_parse_bootfail_for_bopd */
	if (pselect_param->is_download_succ) {
		pselect_param->recovery_method =
			(pselect_param->times_after_download == BF_1_TIME) ?
			(FRM_BOPD_AFTER_DOWNLOAD) :
			(FRM_HW_DEGRADE_BOPD_AFTER_DOWNLOAD);
		return 0;
	}

	/*
	 * If there're interleave fault between bootloader stage
	 * and non-bootloader stage, download firstly!
	 */
	if (pselect_param->times_bottom != 0) {
		pselect_param->is_hardware_degrade = true;
		pselect_param->recovery_method = FRM_DOWNLOAD;
		return 0;
	}

	select_rmethod_times_app_bopd(pselect_param);

	return 0;
}

static void get_safemode_rmethod(struct bfr_method_param *pselect_param)
{
	switch (pselect_param->failed_times) {
	case BF_1_TIME:
		pselect_param->recovery_method = FRM_REBOOT;
		break;
	case BF_2_TIME:
		switch (pselect_param->safe_mode_times) {
		case BF_1_TIME:
			pselect_param->recovery_method = FRM_ENTER_SAFE_MODE;
			break;
		default:
			pselect_param->recovery_method = FRM_REBOOT;
			break;
		}
		break;
	case BF_3_TIME:
		switch (pselect_param->safe_mode_times) {
		case BF_0_TIME:
			pselect_param->recovery_method = FRM_REBOOT;
			break;
		case BF_1_TIME:
			pselect_param->recovery_method = FRM_ENTER_SAFE_MODE;
			break;
		case BF_2_TIME:
		default:
			pselect_param->recovery_method = FRM_FACTORY_RST;
			break;
		}
		break;
	case BF_4_TIME:
	default:
		pselect_param->recovery_method = FRM_DOWNLOAD;
		break;
	}
}

static void get_non_safemode_rmethod(struct bfr_method_param *pselect_param)
{
	switch (pselect_param->failed_times) {
	case BF_1_TIME:
	case BF_2_TIME:
		pselect_param->recovery_method =
			(pselect_param->has_fixed_method) ?
			(pselect_param->fixed_method) :
			(FRM_REBOOT);
		break;
	case BF_3_TIME:
		pselect_param->recovery_method =
			(pselect_param->has_fixed_method) ?
			(pselect_param->fixed_method) :
			(FRM_FACTORY_RST);
		break;
	case BF_4_TIME:
	default:
		pselect_param->recovery_method =
			(pselect_param->has_fixed_method) ?
			(pselect_param->fixed_method) :
			FRM_DOWNLOAD;
		break;
	}
}

static int select_with_safemode(struct bfr_method_param *pselect_param)
{
	if (bfr_parse_boot_fail_info(pselect_param) != 0) {
		BFMR_PRINT_ERR("Failed to parse boot fail info!\n");
		return -1;
	}
	if (pselect_param->not_parse_further)
		return 0;

	if (has_safemode_method(pselect_param->boot_fail_no))
		get_safemode_rmethod(pselect_param);
	else
		get_non_safemode_rmethod(pselect_param);

	return 0;
}

static void get_rmethod_zero_time(struct bfr_method_param *pselect_param)
{
	if (pselect_param->has_fixed_method) {
		BFMR_PRINT_KEY_INFO(
		    "[Bottom layer no boot fail] APP has fixed rmethod: %d\n",
		    pselect_param->fixed_method);
		pselect_param->recovery_method = pselect_param->fixed_method;
		return;
	}
	switch (pselect_param->times_app) {
	case BF_1_TIME:
		BFMR_PRINT_KEY_INFO("[Bottom layer no boot fail] FRM_REBOOT\n");
		pselect_param->recovery_method = FRM_REBOOT;
		break;
	case BF_2_TIME:
		BFMR_PRINT_KEY_INFO(
		    "[Bottom layer has no boot fail] FRM_DEL_FILES_FOR_BF\n");
		pselect_param->recovery_method = FRM_DEL_FILES_FOR_BF;
		break;
	case BF_3_TIME:
		BFMR_PRINT_KEY_INFO(
		    "[Bottom layer has no boot fail] FRM_FACTORY_RST\n");
		pselect_param->recovery_method = FRM_FACTORY_RST;
		break;
	case BF_4_TIME:
	default:
		BFMR_PRINT_KEY_INFO(
		    "[Bottom layer has no boot fail] FRM_DOWNLOAD\n");
		pselect_param->recovery_method = FRM_DOWNLOAD;
		break;
	}
}

static void get_rmethod_one_or_two_times(struct bfr_method_param *pselect_param)
{
	if (pselect_param->has_fixed_method) {
		BFMR_PRINT_KEY_INFO(
		    "[Bottom layer has 1 or 2 boot fail] APP has fixed recovery method\n");
		pselect_param->recovery_method = pselect_param->fixed_method;
		return;
	}
	switch (pselect_param->times_app) {
	case BF_0_TIME:
		BFMR_PRINT_KEY_INFO("[APP has no Bootfail] FRM_REBOOT\n");
		pselect_param->recovery_method = FRM_REBOOT;
		break;
	case BF_1_TIME:
		BFMR_PRINT_KEY_INFO("[Bottom layer has Bootfail] FRM_REBOOT\n");
		pselect_param->recovery_method = FRM_REBOOT;
		break;
	case BF_2_TIME:
		BFMR_PRINT_KEY_INFO(
		    "[Bottom layer has boot fail] FRM_DEL_FILES_FOR_BF\n");
		pselect_param->recovery_method = FRM_DEL_FILES_FOR_BF;
		break;
	case BF_3_TIME:
	default:
		BFMR_PRINT_KEY_INFO(
		   "[Bottom layer has Bootfail FRM_DOWNLOAD_AND_FACTORY_RST\n");
		pselect_param->recovery_method = FRM_DOWNLOAD_AND_FACTORY_RST;
		break;
	}
}

static int select_without_safemode(struct bfr_method_param *pselect_param)
{
	if (bfr_parse_boot_fail_info(pselect_param) != 0) {
		BFMR_PRINT_ERR("Failed to parse boot fail info!\n");
		return -1;
	}
	if (pselect_param->not_parse_further)
		return 0;
	if (bfr_bopd_has_been_enabled())
		return bfr_select_recovery_method_with_bopd(pselect_param);

	switch (pselect_param->times_bottom) {
	case BF_0_TIME:
		get_rmethod_zero_time(pselect_param);
		break;
	case BF_1_TIME:
	case BF_2_TIME:
		get_rmethod_one_or_two_times(pselect_param);
		break;
	case BF_3_TIME:
	default:
		BFMR_PRINT_KEY_INFO(
		    "[APP has no boot fail] FRM_DOWNLOAD\n");
		pselect_param->recovery_method = FRM_DOWNLOAD;
		break;
	}
	return 0;
}

static bfr_boot_fail_stage_e get_bootfail_stage(unsigned int boot_fail_stage)
{
	if (bfmr_is_bl1_stage(boot_fail_stage) ||
	    bfmr_is_bl2_stage(boot_fail_stage))
		return BFS_BOOTLOADER;

	if (bfmr_is_kernel_stage(boot_fail_stage))
		return BFS_KERNEL;

	return BFS_APP;
}

static int run_method(struct bfr_method_param *pselect_param)
{
	bfmr_rrecord_misc_msg_param_t misc_msg;

	memset((void *)&misc_msg, 0, sizeof(misc_msg));
	misc_msg.boot_fail_no = (unsigned int)pselect_param->boot_fail_no;
	memcpy((void *)misc_msg.command, (void *)BFR_ENTER_BOPD_MODE_CMD,
		strlen(BFR_ENTER_BOPD_MODE_CMD));
	switch (pselect_param->recovery_method) {
	case FRM_DEL_FILES_FOR_BF:
	case FRM_DEL_FILES_FOR_NOSPC:
	case FRM_FACTORY_RST:
	case FRM_FORMAT_DATA_PART:
	case FRM_DOWNLOAD_AND_DEL_FILES:
	case FRM_DOWNLOAD_AND_FACTORY_RST:
	case FRM_DOWNLOAD:
	case FRM_LOWLEVEL_FORMAT_DATA:
	case FRM_FACTORY_RST_AFTER_DOWNLOAD:
		memset(misc_msg.command, 0, sizeof(misc_msg.command));
		memcpy((void *)misc_msg.command,
			(void *)BFR_ENTER_ERECOVERY_CMD,
			strlen(BFR_ENTER_ERECOVERY_CMD) + 1);
		misc_msg.enter_erecovery_reason = EER_BOOT_FAIL_SOLUTION;
		misc_msg.enter_erecovery_reason_number =
		    (pselect_param->is_hardware_degrade) ?
		    (ENTER_ERECOVERY_BECAUSE_HW_DEGRADE_BOOT_FAIL) :
		    (get_erecovery_reason(pselect_param->boot_fail_no));
		misc_msg.boot_fail_stage_for_erecovery =
		    get_bootfail_stage(pselect_param->boot_fail_stage);
		misc_msg.recovery_method =
		    (unsigned int)pselect_param->recovery_method;
		misc_msg.boot_fail_no = pselect_param->boot_fail_no;
		(void)set_misc_msg();
		break;
	case FRM_NOTIFY_USER_FAIL:
		break;
	case FRM_ENTER_SAFE_MODE:
		BFMR_PRINT_KEY_INFO("FRM_ENTER_SAFE_MODE!\n");
		memset(misc_msg.command, 0, sizeof(misc_msg.command));
		memcpy((void *)misc_msg.command,
		    (void *)BFR_ENTER_SAFE_MODE_CMD,
		    strlen(BFR_ENTER_SAFE_MODE_CMD) + 1);
		break;
	case FRM_BOPD:
		BFMR_PRINT_KEY_INFO("FRM_BOPD!\n");
		misc_msg.bopd_mode_value = BOPD_BEFORE_DOWNLOAD;
		break;
	case FRM_STORAGE_RDONLY_BOPD:
		BFMR_PRINT_KEY_INFO("FRM_STORAGE_RDONLY_BOPD!\n");
		misc_msg.bopd_mode_value = BOPD_STORAGE_RDONLY_BEFORE_DOWNLOAD;
		break;
	case FRM_HW_DEGRADE_BOPD:
		BFMR_PRINT_KEY_INFO("FRM_HW_DEGRADE_BOPD!\n");
		misc_msg.bopd_mode_value = BOPD_HW_DEGRADE_BEFORE_DOWNLOAD;
		break;
	case FRM_STORAGE_RDONLY_HW_DEGRADE_BOPD:
		BFMR_PRINT_KEY_INFO("FRM_STORAGE_RDONLY_HW_DEGRADE_BOPD!\n");
		misc_msg.bopd_mode_value =
		    BOPD_STORAGE_RDONLY_HW_DEGRADE_BEFORE_DOWNLOAD;
		break;
	case FRM_BOPD_AFTER_DOWNLOAD:
		BFMR_PRINT_KEY_INFO("FRM_BOPD_AFTER_DOWNLOAD!\n");
		misc_msg.bopd_mode_value = BOPD_AFTER_DOWNLOAD;
		break;
	case FRM_HW_DEGRADE_BOPD_AFTER_DOWNLOAD:
		BFMR_PRINT_KEY_INFO("FRM_HW_DEGRADE_BOPD_AFTER_DOWNLOAD!\n");
		misc_msg.bopd_mode_value = BOPD_HW_DEGRADE_AFTER_DOWNLOAD;
		break;
	default:
		return 0;
	}
	(void)bfmr_write_rrecord_misc_msg(&misc_msg);
	return 0;
}

static bfr_recovery_method_running_status_e bfr_method_init(
		bfr_recovery_method_e recovery_method)
{
	switch (recovery_method) {
	case FRM_DOWNLOAD:
	case FRM_DOWNLOAD_AND_DEL_FILES:
	case FRM_DOWNLOAD_AND_FACTORY_RST:
	case FRM_LOWLEVEL_FORMAT_DATA:
	case FRM_FACTORY_RST_AFTER_DOWNLOAD:
	case FRM_FACTORY_RST:
	case FRM_FORMAT_DATA_PART:
	case FRM_DEL_FILES_FOR_BF:
	case FRM_DEL_FILES_FOR_NOSPC:
		return RMRSC_ERECOVERY_BOOT_FAILED;
	default:
		break;
	}
	return RMRSC_EXEC_COMPLETED;
}

static bfr_recovery_method_run_result_e bfr_method_result(
		bfr_recovery_method_e recovery_method)
{
	switch (recovery_method) {
	case FRM_DOWNLOAD:
	case FRM_DOWNLOAD_AND_DEL_FILES:
	case FRM_DOWNLOAD_AND_FACTORY_RST:
		return RMRR_FAILED;
	default:
		break;
	}
	return RMRR_SUCCESS;
}

static int init_one_rrecord_param(int index)
{
	rrecord_param[index].buf = bfmr_malloc(BFR_EACH_RECORD_PART_SIZE);
	if (rrecord_param[index].buf == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		release_rrecord_param();
		return -1;
	}
	memset((void *)rrecord_param[index].buf, 0, BFR_EACH_RECORD_PART_SIZE);
	rrecord_param[index].buf_size = BFR_EACH_RECORD_PART_SIZE;
	if (index == 0)
		rrecord_param[index].part_offset = BFR_RRECORD_1ST_PART_OFS;
	else if (index == 1)
		rrecord_param[index].part_offset = BFR_RRECORD_2ND_PART_OFS;
	else
		rrecord_param[index].part_offset = BFR_RRECORD_3RD_PART_OFS;

	return 0;
}

static int init_rrecord_param(void)
{
	int i = 0;
	int count = ARRAY_SIZE(rrecord_param);

	/* return right now if the memory of the local param has been inited */
	if (rrecord_param[0].buf != NULL)
		return 0;

	memset((void *)&rrecord_param, 0, sizeof(rrecord_param));
	for (i = 0; i < count; i++) {
		if (init_one_rrecord_param(i) != 0)
			return -1;
	}

	return 0;
}

void release_rrecord_param(void)
{
	int i;
	int count = ARRAY_SIZE(rrecord_param);

	for (i = 0; i < count; i++) {
		if(rrecord_param[i].buf == NULL)
			continue;
		bfmr_free(rrecord_param[i].buf);
		rrecord_param[i].buf = NULL;
	}
	recovery_record_has_been_read_and_verified = false;
	return;
}

static int read_rrecord_to_buf(void)
{
	int i = 0;
	int ret = -1;
	int count = ARRAY_SIZE(rrecord_param);
	char *dev_path = NULL;

	/* 1. read recovery record here firstly */
	if (init_rrecord_param() != 0) {
		BFMR_PRINT_ERR("Failed to init local rrecord read param!\n");
		goto __out;
	}

	/* 2. get path of the rrecord path */
	ret = bfr_get_full_path_of_rrecord_part(&dev_path);
	if (ret != 0) {
		BFMR_PRINT_ERR("find the full path of rrecord part failed!\n");
		goto __out;
	}

	/* 3. read recovery record to local */
	for (i = 0; i < count; i++) {
		ret = bfmr_read_emmc_raw_part(dev_path,
					      rrecord_param[i].part_offset,
					      rrecord_param[i].buf,
					      rrecord_param[i].buf_size);
		if (ret != 0) {
			BFMR_PRINT_ERR("read %s failed!\n", dev_path);
			break;
		}
	}

__out:
	if (dev_path != NULL) {
		bfmr_free(dev_path);
		dev_path = NULL;
	}

	return ret;
}

static void check_crc32_rrecord(int *record_damaged, int *valid_record_idx)
{
	int i;
	int count = ARRAY_SIZE(rrecord_param);

	for (i = 0; i < count; i++) {
		unsigned int local_crc32;
		unsigned int crc_size;
		bfr_recovery_record_header_t *pheader =
		(bfr_recovery_record_header_t *)rrecord_param[i].buf;
		crc_size = (unsigned int)sizeof(pheader->crc32);

		local_crc32 = bfmr_get_crc32(
				rrecord_param[i].buf + crc_size,
				rrecord_param[i].buf_size - crc_size);
		if (pheader->crc32 != local_crc32) {
			BFMR_PRINT_ERR(
			"CRC check failed! orig_crc32:0x%08x local_crc32: 0x%08x\n",
			pheader->crc32, local_crc32);
			rrecord_param[i].record_damaged = 1;
			(*record_damaged)++;
		} else {
			rrecord_param[i].record_damaged = 0;
			(*valid_record_idx) = i;
		}
	}
}

static int correct_rrecord(char *dev_path, int valid_record_idx)
{
	int i;
	int ret = 0;
	int count = ARRAY_SIZE(rrecord_param);

	for (i = 0; i < count; i++) {
		if (rrecord_param[i].record_damaged == 0)
			continue;
		memcpy((void *)rrecord_param[i].buf,
			(void *)rrecord_param[valid_record_idx].buf,
			rrecord_param[valid_record_idx].buf_size);
		ret = bfmr_write_emmc_raw_part(dev_path,
			rrecord_param[i].part_offset,
			rrecord_param[i].buf,
			rrecord_param[i].buf_size);
		if (ret != 0) {
			BFMR_PRINT_ERR("write data to %s failed!\n",
					dev_path);
			break;
		}
		rrecord_param[i].record_damaged = 0;
	}
	return ret;
}

static int verify_rrecord(void)
{
	bfr_recovery_record_header_t *pfirst_header = NULL;
	bfr_recovery_record_header_t *psecond_header = NULL;
	int ret = -1;
	int valid_record_idx = -1;
	int record_damaged = 0;
	char *dev_path = NULL;

	/* 1. check the magic number */
	pfirst_header = (bfr_recovery_record_header_t *)rrecord_param[0].buf;
	psecond_header = (bfr_recovery_record_header_t *)rrecord_param[1].buf;
	if ((pfirst_header->magic_number != BFR_RRECORD_MAGIC_NUMBER) &&
	    (psecond_header->magic_number != BFR_RRECORD_MAGIC_NUMBER)) {
		/* maybe it is the first time teh rrecord part hae been used */
		return init_rrecord_header();
	}

	/* 2. check the crc32 value */
	check_crc32_rrecord(&record_damaged, &valid_record_idx);

	/* 3. no valid record has been found, init the record header */
	if (valid_record_idx < 0) {
		BFMR_PRINT_ERR("There is no valid recovery record!\n");
		return init_rrecord_header();
	}

	/* 4. malloc memory for the full path buffer of rrecord */
	if (record_damaged != 0) {
		ret = bfr_get_full_path_of_rrecord_part(&dev_path);
		if (ret != 0) {
			BFMR_PRINT_ERR("get path of rrecord part failed!\n");
			goto __out;
		}
	}

	/* 5. correct the recovery record
	 *    set the value of ret as 0 here
	 *    because maybe all the parts are valid
	 */
	ret = correct_rrecord(dev_path, valid_record_idx);

__out:
	if (dev_path != NULL) {
		bfmr_free(dev_path);
		dev_path = NULL;
	}

	return ret;
}

static int init_rrecord_header(void)
{
	bfr_recovery_record_header_t *pheader = NULL;
	unsigned int header_size = sizeof(bfr_recovery_record_header_t);
	int i = 0;
	int ret;
	int count = ARRAY_SIZE(rrecord_param);
	char *dev_path = NULL;

	ret = bfr_get_full_path_of_rrecord_part(&dev_path);
	if (ret != 0) {
		BFMR_PRINT_ERR("find the full path of rrecord part failed!\n");
		goto __out;
	}

	for (i = 0; i < count; i++) {
		unsigned int crc_size;

		memset((void *)rrecord_param[i].buf, 0,
			rrecord_param[i].buf_size);
		rrecord_param[i].record_damaged = 0;
		pheader = (bfr_recovery_record_header_t *)rrecord_param[i].buf;
		pheader->magic_number = BFR_RRECORD_MAGIC_NUMBER;
		pheader->safe_mode_enable_flag =
					(bfr_safe_mode_has_been_enabled()) ?
					(BFR_SAFE_MODE_ENABLE_FLAG) : (0x0);
		pheader->record_count =
				(rrecord_param[i].buf_size - header_size) /
				 sizeof(bfr_recovery_record_t);
		crc_size = (unsigned int)sizeof(pheader->crc32);
		pheader->crc32 = bfmr_get_crc32(
			rrecord_param[i].buf + crc_size,
			rrecord_param[i].buf_size - crc_size);
		ret = bfmr_write_emmc_raw_part(dev_path,
						rrecord_param[i].part_offset,
						(char *)rrecord_param[i].buf,
						rrecord_param[i].buf_size);
		if (ret != 0) {
			BFMR_PRINT_ERR("Write part %s failed [ret = %d]!\n",
					dev_path, ret);
			break;
		}
	}

__out:
	if (dev_path != NULL) {
		bfmr_free(dev_path);
		dev_path = NULL;
	}

	return ret;
}

static int read_and_verify_rrecord(void)
{
	int ret;

	if (recovery_record_has_been_read_and_verified)
		return 0;

	/* 1. read recovery record to local buffer */
	ret = read_rrecord_to_buf();
	if (ret != 0) {
		BFMR_PRINT_ERR("read_rrecord_to_buf failed!\n");
		return -1;
	}

	/* 2. verify the recovery record */
	ret = verify_rrecord();
	if (ret != 0) {
		BFMR_PRINT_ERR("verify_rrecord failed!\n");
		return -1;
	}

	/* 3. update the local flag */
	recovery_record_has_been_read_and_verified = true;
	return ret;
}

static int read_rrecord_from_buf(bfr_recovery_record_t *precord,
		bfr_recovery_record_header_t *precord_header,
		const int *record_count_actually_read)
{
	unsigned int buf_size;
	unsigned int header_size = sizeof(bfr_recovery_record_header_t);
	unsigned int record_size = sizeof(bfr_recovery_record_t);
	unsigned int end_cnt;
	unsigned int begin_cnt;

	buf_size = (*record_count_actually_read) *
		    sizeof(bfr_recovery_record_t);
	if (precord_header->next_record_idx >= *record_count_actually_read) {
		memcpy((void *)precord,
			(void *)(rrecord_param[0].buf + header_size +
				(precord_header->next_record_idx -
				*record_count_actually_read) *
				record_size),
			buf_size); /* lint !e679 */
	} else {
		end_cnt = *record_count_actually_read -
			   precord_header->next_record_idx;
		begin_cnt = precord_header->next_record_idx;

		memcpy((void *)precord,
			(void *)(rrecord_param[0].buf + header_size +
				(precord_header->record_count - end_cnt) *
				record_size),
			end_cnt * record_size); /* lint !e647 !e679 */
		memcpy((void *)((char *)precord + end_cnt * record_size),
			(void *)(rrecord_param[0].buf + header_size),
			begin_cnt * record_size);
		}
	return 0;
}

static int read_rrecord(bfr_recovery_record_t *precord,
				    int record_count_to_read,
				    int *record_count_actually_read)
{
	bfr_recovery_record_header_t *precord_header = NULL;

	if (unlikely((record_count_actually_read == NULL))) {
		BFMR_PRINT_INVALID_PARAMS("record_count_actually_read\n");
		return -1;
	}

	/* 1. init the record_count_actually_read as 0 */
	*record_count_actually_read = 0;

	/* 2. read and verify the recovery record */
	if (read_and_verify_rrecord() != 0) {
		BFMR_PRINT_ERR("Failed to read and verify the rrecord!\n");
		return -1;
	}

	/* 3. read recovery record */
	precord_header = (bfr_recovery_record_header_t *)rrecord_param[0].buf;
	*record_count_actually_read = BFMR_MIN(precord_header->boot_fail_count,
					BFMR_MIN(precord_header->record_count,
						record_count_to_read));

	return read_rrecord_from_buf(precord, precord_header,
					record_count_actually_read);
}

static int write_one_rrecord_header(bfr_recovery_record_t *precord,
				    const char *dev_path,
				    int index)
{
	int ret;
	unsigned int crc_size;
	unsigned int header_size = sizeof(bfr_recovery_record_header_t);
	unsigned int record_size = sizeof(bfr_recovery_record_t);
	bfr_recovery_record_header_t *pheader =
		(bfr_recovery_record_header_t *)rrecord_param[index].buf;

	memcpy((void *)(rrecord_param[index].buf + header_size +
			pheader->next_record_idx * record_size),
		(void *)precord, record_size);
	pheader->boot_fail_count++;
	pheader->next_record_idx++;
	pheader->rec_cnt_before_boot_succ++;
	if (pheader->next_record_idx >= pheader->record_count) {
		pheader->next_record_idx = 0;
		pheader->last_record_idx = pheader->record_count - 1;
	} else {
		pheader->last_record_idx = pheader->next_record_idx - 1;
	}
	pheader->safe_mode_enable_flag =
	    (bfr_safe_mode_has_been_enabled()) ?
	    (BFR_SAFE_MODE_ENABLE_FLAG) : (0x0);
	crc_size = (unsigned int)sizeof(pheader->crc32);
	pheader->crc32 = bfmr_get_crc32(
	    rrecord_param[index].buf + crc_size,
	    rrecord_param[index].buf_size - crc_size);

	/* 1. write header */
	ret = bfmr_write_emmc_raw_part(dev_path,
		(unsigned long long)rrecord_param[index].part_offset,
		(char *)pheader, (unsigned long long)header_size);

	return ret;

}

static int write_one_rrecord(bfr_recovery_record_t *precord, char *dev_path,
		int index)
{
	int ret;
	unsigned int header_size = sizeof(bfr_recovery_record_header_t);
	unsigned int record_size = sizeof(bfr_recovery_record_t);
	bfr_recovery_record_header_t *pheader = NULL;

	ret = write_one_rrecord_header(precord, dev_path, index);
	if (ret != 0) {
		BFMR_PRINT_ERR("Write record header to %s failed!\n",
				dev_path);
		return ret;
	}

	pheader = (bfr_recovery_record_header_t *)rrecord_param[index].buf;
	/* 2. write record */
	ret = bfmr_write_emmc_raw_part(dev_path,
			(unsigned long long)(rrecord_param[index].part_offset +
				(unsigned long long)header_size +
				pheader->last_record_idx *
				(unsigned long long)record_size),
			(char *)precord,
			(unsigned long long)record_size);
	if (ret != 0) {
		BFMR_PRINT_ERR("Write recovery record to %s failed\n",
				dev_path);
		return ret;
	}
	return ret;
}

static int create_rrecord(bfr_recovery_record_t *precord)
{
	int i;
	int ret = -1;
	int count = ARRAY_SIZE(rrecord_param);
	char *dev_path = NULL;

	if (bfr_get_full_path_of_rrecord_part(&dev_path) != 0) {
		BFMR_PRINT_ERR("find the full path of rrecord part failed!\n");
		goto __out;
	}

	for (i = 0; i < count; i++) {
		ret = write_one_rrecord(precord, dev_path, i);
		if (ret != 0)
			goto __out;
	}

__out:
	if (dev_path != NULL) {
		bfmr_free(dev_path);
		dev_path = NULL;
	}

	return ret;
}

static bool is_hardware_fault(bfr_recovery_record_t *precord,
		bfmr_get_hw_fault_info_param_t *pfault_info_param)
{
	char *bootfail_detail = NULL;

	bootfail_detail = precord->bootfail_detail;
	bootfail_detail[sizeof(precord->bootfail_detail) - 1] = '\0';
	switch (pfault_info_param->fault_stage) {
	case HW_FAULT_STAGE_DURING_BOOTUP:
		if (precord->boot_fail_stage == STAGE_BOOT_SUCCESS)
			break;
		if (strcmp(bootfail_detail,
		    pfault_info_param->hw_excp_info.ocp_excp_info.ldo_num) == 0)
			return true;
		break;
	case HW_FAULT_STAGE_AFTER_BOOT_SUCCESS:
		if (precord->boot_fail_stage != STAGE_BOOT_SUCCESS)
			break;

		if (strcmp(bootfail_detail,
		    pfault_info_param->hw_excp_info.ocp_excp_info.ldo_num) == 0)
			return true;
		break;
	default:
		break;
	}

	return false;
}

static int get_hw_fault_times(int record_count,
		bfmr_get_hw_fault_info_param_t *pfault_info_param)
{
	int i;
	int fault_times = 0;
	bfr_recovery_record_t *precord = NULL;

	for (i = 0; i < record_count; i++) {
		precord = (bfr_recovery_record_t *)(rrecord_param[0].buf +
				sizeof(bfr_recovery_record_header_t) +
				i * sizeof(bfr_recovery_record_t));
		if (precord->boot_fail_no != BFM_HARDWARE_FAULT)
			continue;

		if (is_hardware_fault(precord, pfault_info_param))
			fault_times++;
	}

	return fault_times;
}

int bfr_get_hardware_fault_times(
		bfmr_get_hw_fault_info_param_t *pfault_info_param)
{
	bfr_recovery_record_header_t *pheader = NULL;
	int record_count;

	if (unlikely((pfault_info_param == NULL))) {
		BFMR_PRINT_INVALID_PARAMS("pfault_info_param\n");
		return -1;
	}

	/* 1. read and verify the recovery record */
	if (read_and_verify_rrecord() != 0) {
		BFMR_PRINT_ERR("Failed to read and verify the rrecord!\n");
		return -1;
	}

	pheader = (bfr_recovery_record_header_t *)(rrecord_param[0].buf);
	if (pheader == NULL) {
		BFMR_PRINT_ERR("the local rrecord not been read success!\n");
		return -1;
	}

	record_count = BFMR_MIN(pheader->boot_fail_count,
				pheader->record_count);

	return get_hw_fault_times(record_count, pfault_info_param);
}

int bfr_get_real_recovery_info(bfr_real_recovery_info_t *preal_recovery_info)
{
	int i = 0;
	int ret = -1;
	int offset;
	int actual_read = 0;
	size_t count;
	bfr_recovery_record_t *rrecord = NULL;

	if (preal_recovery_info == NULL) {
		BFMR_PRINT_INVALID_PARAMS("preal_recovery_info\n");
		return -1;
	}

	/* 1. read recovery record to local */
	count = ARRAY_SIZE(preal_recovery_info->recovery_method);
	rrecord = bfmr_malloc(sizeof(*rrecord) * count);
	if (rrecord == NULL) {
		BFMR_PRINT_KEY_INFO("bfmr_malloc failed!\n");
		goto __out;
	}
	memset(rrecord, 0, sizeof(*rrecord) * count);

	ret = read_rrecord(rrecord, count, &actual_read);
	if (ret != 0) {
		BFMR_PRINT_ERR("Failed to read the recovery record!\n");
		goto __out;
	}

	memset(preal_recovery_info, 0, sizeof(*preal_recovery_info));
	/* actual_read <= count of preal_recovery_info->recovery_method */
	preal_recovery_info->record_count = BFMR_MIN(
		rrecord_param[0].rec_cnt_before_boot_succ, actual_read);
	if (preal_recovery_info->record_count <= 0) {
		BFMR_PRINT_ERR(
		    "There're no valid recovery record, rec_cnt_before_boot_succ:%d, record_cnt_actual_read:%d\n",
		    rrecord_param[0].rec_cnt_before_boot_succ,
		    actual_read);
		goto __out;
	}

	offset = (rrecord_param[0].rec_cnt_before_boot_succ < actual_read) ?
		(actual_read - rrecord_param[0].rec_cnt_before_boot_succ) : (0);
	for (i = 0; i < preal_recovery_info->record_count; i++) {
		int tmp = i + offset;

		preal_recovery_info->recovery_method[i] =
			rrecord[tmp].recovery_method;
		preal_recovery_info->recovery_method_original[i] =
			rrecord[tmp].recovery_method_original; /* lint !e679 */
		preal_recovery_info->boot_fail_no[i] =
			rrecord[tmp].boot_fail_no; /* lint !e679 */
		preal_recovery_info->boot_fail_stage[i] =
			rrecord[tmp].boot_fail_stage; /* lint !e679 */
		preal_recovery_info->boot_fail_time[i] =
			rrecord[tmp].boot_fail_time; /* lint !e679 */
		preal_recovery_info->boot_fail_rtc_time[i] =
			rrecord[tmp].boot_fail_detected_time;
	}
	BFMR_PRINT_KEY_INFO("There're %d recovery record\n",
				preal_recovery_info->record_count);

__out:
	if (rrecord != NULL) {
		bfmr_free(rrecord);
		rrecord = NULL;
	}

	return ret;
}

static bool update_rrecord(bfr_recovery_record_t *precord)
{
	int i;
	int count = ARRAY_SIZE(rrecord_param);
	bfr_recovery_record_t recovery_record;
	unsigned int header_size = sizeof(bfr_recovery_record_header_t);
	unsigned int record_size = sizeof(bfr_recovery_record_t);
	bool system_boot_fail_last_time = false;

	/* update recovery record */
	for (i = 0; i < count; i++) {
		unsigned int crc_size;
		int rec_cnt_before_boot_succ = 0;
		bfr_recovery_record_header_t *pheader =
			(bfr_recovery_record_header_t *)rrecord_param[i].buf;

		memset((void *)&recovery_record, 0, record_size);
		memcpy((void *)&recovery_record,
			(void *)(rrecord_param[i].buf + header_size +
				 pheader->last_record_idx * record_size),
			record_size); /* lint !e679 */
		if (recovery_record.recovery_result != BF_RECOVERY_SUCCESS)
			system_boot_fail_last_time = true;

		recovery_record.recovery_result = precord->recovery_result;
		memcpy((void *)(rrecord_param[i].buf + header_size +
				pheader->last_record_idx * record_size),
			(void *)&recovery_record, record_size);
		rec_cnt_before_boot_succ = pheader->rec_cnt_before_boot_succ;
		pheader->rec_cnt_before_boot_succ =
			(precord->recovery_result == BF_RECOVERY_SUCCESS) ?
			(0) : (pheader->rec_cnt_before_boot_succ);
		crc_size = (unsigned int)sizeof(pheader->crc32);
		pheader->crc32 = bfmr_get_crc32(
			rrecord_param[i].buf + crc_size,
			rrecord_param[i].buf_size - crc_size);
		rrecord_param[i].rec_cnt_before_boot_succ =
			rec_cnt_before_boot_succ;
	}

	return system_boot_fail_last_time;
}

static int update_and_save_rrecord(char *dev_path,
		bfr_recovery_record_t *precord)
{
	int i;
	int ret = 0;
	int count = ARRAY_SIZE(rrecord_param);
	bool system_boot_fail_last_time = false;

	system_boot_fail_last_time = update_rrecord(precord);

	/* save recovery record */
	for (i = 0; i < count; i++) {
		if (system_boot_fail_last_time ||
		    (rrecord_param[i].record_damaged != 0)) {
			ret = bfmr_write_emmc_raw_part(dev_path,
						rrecord_param[i].part_offset,
						(char *)rrecord_param[i].buf,
						rrecord_param[i].buf_size);
			if (ret != 0) {
				BFMR_PRINT_ERR("Write %s failed errno: %d\n",
						dev_path, ret);
				return ret;
			}
		}
	}

	return ret;
}

/*
 * this function need not be realized in bootloader,
 * it should be realized in kernel and erecovery
 */
static int renew_rrecord(bfr_recovery_record_t *precord)
{
	int ret = -1;
	char *dev_path = NULL;
	bfr_recovery_record_header_t *precord_header = NULL;

	/* 1. read recovery record to local */
	if (read_and_verify_rrecord() != 0) {
		BFMR_PRINT_ERR("Failed to read and verify the rrecord!\n");
		goto __out;
	}

	/* 2. get the full path of the valid rrecord part */
	if (bfr_get_full_path_of_rrecord_part(&dev_path) != 0) {
		BFMR_PRINT_ERR("find the full path of rrecord part failed!\n");
		goto __out;
	}

	/* 3. chech if there're valid recovery record or not */
	precord_header = (bfr_recovery_record_header_t *)rrecord_param[0].buf;
	if (precord_header->boot_fail_count == 0) {
		BFMR_PRINT_KEY_INFO("There is no valid recovery record!\n");
		ret = 0;
		goto __out;
	}

	ret = update_and_save_rrecord(dev_path, precord);

__out:
	if (dev_path != NULL) {
		bfmr_free(dev_path);
		dev_path = NULL;
	}

	return ret;
}

char *bfr_get_recovery_method_desc(int rmethod)
{
	int i = 0;
	int count = ARRAY_SIZE(method_desc);

	for (i = 0; i < count; i++) {
		if ((bfr_recovery_method_e)rmethod == method_desc[i].method)
			return method_desc[i].desc;
	}
	return "unknown";
}

/*
 * @function: void boot_status_notify(int boot_success)
 * @brief: when the system bootup successfully, the BFM must call this
 *	function to notify the BFR, and the BFM was notified by the BFD.
 * @note: this fuction only need be initialized in kernel.
 */
void boot_status_notify(int boot_success)
{
	bfr_recovery_record_t recovery_record;

	BFMR_PRINT_KEY_INFO("Recovery boot fail Successfully!\n");
	memset((void *)&recovery_record, 0, sizeof(recovery_record));
	recovery_record.recovery_result = (boot_success == 0) ?
					  (BF_RECOVERY_FAILURE) :
					  (BF_RECOVERY_SUCCESS);
	(void)renew_rrecord(&recovery_record);
}

static unsigned int get_bootfail_uptime(void)
{
	long boottime = bfmr_get_bootup_time();

	return (boottime == 0L) ? ((unsigned int)(-1)) : (unsigned int)boottime;
}

/*
 * @function: bfr_recovery_method_e try_to_recovery(
 *    unsigned long long boot_fail_detected_time,
 *    bfmr_bootfail_errno_e boot_fail_no,
 *    bfmr_detail_boot_stage_e boot_fail_stage,
 *    bfr_suggested_recovery_method_e suggested_recovery_method,
 *    char *args)
 * @brief: do recovery for the boot fail.
 * @param: boot_fail_detected_time [in], rtc time when boot fail was detected.
 * @param: boot_fail_no [in], boot fail errno.
 * @param: boot_fail_stage [in], the stage when boot fail happened.
 * @param: suggested_recovery_method [in], suggested recovery method transfered
 *    by the BFD(Boot Fail Detection).
 * @param: args [in], extra parametrs for recovery.
 * @return: the recovery method selected by the BFR.
 */
bfr_recovery_method_e try_to_recovery(
		unsigned long long boot_fail_detected_time,
		bfmr_bootfail_errno_e boot_fail_no,
		bfmr_detail_boot_stage_e boot_fail_stage,
		bfr_suggested_recovery_method_e suggested_recovery_method,
		char *args)
{
	int ret;
	bfr_recovery_record_t cur_rrecord;
	bfr_recovery_method_e recovery_method = FRM_DO_NOTHING;
	struct bfr_method_param *pselect_param = NULL;

	if (!bfr_has_been_enabled()) {
		BFMR_PRINT_ERR("BFR disabled, not entery recovery method\n");
		return recovery_method;
	}

	if (!((boot_fail_no >= HARDWARE_ERRNO_START) &&
	      (boot_fail_no <= HARDWARE_ERRNO_END)) &&
	    (suggested_recovery_method == DO_NOTHING))
		return FRM_DO_NOTHING;

	BFMR_PRINT_KEY_INFO(
		"boot_fail_stage:%x, boot_fail_no: %x, suggested_recovery_method: %d!\n",
		boot_fail_stage, boot_fail_no, (int)suggested_recovery_method);

	/* 1. malloc memory here instead in case of stack overflow */
	pselect_param = bfmr_malloc(sizeof(*pselect_param));
	if (pselect_param == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed for boot fail [0x%x]!\n",
				(unsigned int)boot_fail_no);
		return FRM_DO_NOTHING;
	}
	memset((void *)pselect_param, 0, sizeof(*pselect_param));

	/* 2. read recovery record */
	ret = read_rrecord(pselect_param->latest_rrecord,
			   ARRAY_SIZE(pselect_param->latest_rrecord),
			   &(pselect_param->latest_record_count));
	if (ret != 0) {
		BFMR_PRINT_ERR("Failed to read recovery record [0x%x]!\n",
				(unsigned int)boot_fail_no);
		goto __out;
	}

	/* 3. select recovery method */
	pselect_param->boot_fail_stage = boot_fail_stage;
	pselect_param->boot_fail_no = boot_fail_no;
	pselect_param->method = suggested_recovery_method;
	pselect_param->bootfail_detail_info = args;
	ret = bfr_safe_mode_has_been_enabled() ?
		select_with_safemode(pselect_param) :
		select_without_safemode(pselect_param);
	if (ret != 0) {
		BFMR_PRINT_ERR("Failed to select recovery method [0x%x]!\n",
				(unsigned int)boot_fail_no);
		goto __out;
	}

	/* 4. save recovery record */
	memset((void *)&cur_rrecord, 0, sizeof(cur_rrecord));
	cur_rrecord.boot_fail_detected_time = boot_fail_detected_time;
	cur_rrecord.boot_fail_stage = boot_fail_stage;
	cur_rrecord.boot_fail_no = boot_fail_no;
	cur_rrecord.recovery_method =
		(suggested_recovery_method == DO_NOTHING) ?
		FRM_DO_NOTHING : pselect_param->recovery_method;
	cur_rrecord.running_status_code =
		bfr_method_init(pselect_param->recovery_method);
	cur_rrecord.method_run_result =
		bfr_method_result(pselect_param->recovery_method);
	cur_rrecord.recovery_result = BF_RECOVERY_FAILURE;
	cur_rrecord.recovery_method_original = pselect_param->recovery_method;
	cur_rrecord.boot_fail_time = get_bootfail_uptime();
	if (args != NULL)
		memcpy(cur_rrecord.bootfail_detail, args,
			BFMR_MIN(sizeof(cur_rrecord.bootfail_detail) - 1,
			strlen(args)));
	ret = create_rrecord(&cur_rrecord);
	if (ret != 0) {
		BFMR_PRINT_ERR("Failed to create recovery record [0x%x]!\n",
				(unsigned int)boot_fail_no);
		goto __out;
	}

	/* 5. run recovery method. Note: reboot is executed by the caller now */
	ret = run_method(pselect_param);
	if (ret != 0) {
		BFMR_PRINT_ERR("Failed to run recovery method [0x%x]!\n",
				(unsigned int)boot_fail_no);
		goto __out;
	}

	/* 6. store the real recovery method selected in local variable */
	recovery_method = pselect_param->recovery_method;

__out:
	if (pselect_param != NULL) {
		bfmr_free(pselect_param);
		pselect_param = NULL;
	}

	/* 7. return recovery method to the caller */
	BFMR_PRINT_KEY_INFO("recovery_method: %d for boot fail 0x%x!\n",
			recovery_method, (unsigned int)boot_fail_no);
	return recovery_method;
}

/*
 * @function: int bfr_init(void)
 * @brief: init BFR.
 */
int bfr_init(void)
{
	memset((void *)&rrecord_param, 0, sizeof(rrecord_param));
	return 0;
}
