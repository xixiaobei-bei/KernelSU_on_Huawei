/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: bfm_chipsets.h
 *
 * define the chipsets' external public enum/macros/interface for BFM
 * (Boot Fail Monitor)
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

#ifndef BFM_CHIPSETS_H
#define BFM_CHIPSETS_H
#define OCP_EXCP_INFO_COUNT 10

#include "chipset_common/bfmr/common/bfmr_common.h"
#include "chipset_common/bfmr/bfr/core/bfr_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* this struct is defined for HISI currently */
struct bfi_head_info {
	unsigned int magic;
	unsigned int total_number;
	unsigned int cur_number;
	unsigned int every_number_size;
};

/* this struct is defined for HISI currently */
struct bfi_member_Info {
	unsigned long long rtc_value;
	unsigned int errno;
	unsigned int stage_code;
	unsigned int is_rooted;
	unsigned int is_user_perceptiable;
	unsigned int rcv_method;
	unsigned int rcv_result;
	unsigned int dmd_errno;
	char excep_info[BFMR_SIZE_128];
	unsigned int bootup_time;
	unsigned int is_log_saved;
	unsigned int sugst_rcv_method;
	unsigned int is_boot_success;
	unsigned int reboot_type;
};

/* this struct is defined for qcom currently */
struct bfm_record_Info {
	unsigned int magic;
	unsigned int errno;
	unsigned long long bf_time;
	unsigned int boot_stage;
	unsigned int sugst_rcv_method;
	unsigned int rcv_method;
	unsigned int total_log_lenth;
	unsigned int log_lenth[LOG_TYPE_MAX_COUNT];
	char log_dir[BFMR_SIZE_128];
	char log_name[LOG_TYPE_MAX_COUNT][BFMR_SIZE_128];
};

struct bfm_per_bf_log_info {
	char log_dir[BFMR_MAX_PATH];
};

struct bfm_bf_log_info {
	struct bfm_per_bf_log_info bootfail_logs[BFM_LOG_MAX_COUNT];
	struct bfr_real_rcv_info real_rcv_info;
	int log_dir_count;
};

struct bfm_param {
	enum bfm_errno_code bootfail_errno;
	enum bfmr_detail_stage boot_stage;
	unsigned long long bootfail_time;
	enum bfr_suggested_method suggested_recovery_method;
	enum bfr_method recovery_method;

	/* for hisi platform */
	unsigned int reboot_type;

	/*
	 * 1 - the user can sense the presence of the boot fail, 0 - can not
	 * sense the fault
	 */
	int is_user_sensible;

	/* 1 - the system has been rooted, 0 - the system has not been rooted */
	int is_system_rooted;

	/*
	 * 1 - save bottom layer bootfail log, just for HISI now, 0 - save upper
	 * layer bootfail log
	 */
	int save_bottom_layer_bootfail_log;

	/*
	 * 1 - process boot fail in chipsets, just for HISI currently, 0 - for
	 * non-HISI platform, this field is 0
	 */
	int bootfail_can_only_be_processed_in_platform;
	int dmd_num;

	/* save log to: log part, raw patition, memory etc. */
	int dst_type;

	/* log dir such as: /log/boot_fail/bootfail_20161010101010_0x03000005 */
	char bootfail_log_dir[BFMR_MAX_PATH];

	/* additional info such as: /cache/critical_process_crash.txt */
	struct bfmr_bf_addl_info addl_info;

	/*
	 * varified log path such as:
	 * /log/boot_fail/bootfail_20161010101010_0x03000005/xloader_log
	 */
	char bootfail_log_path[BFMR_MAX_PATH];

	/* for HISI currently */
	void *log_save_context;

	/* for bfm_chipsets call bfm_core's function to save log */
	void *catch_save_bf_log;
	char critical_process_name[BFMR_SIZE_128];
	bool save_log_after_reboot;
	uid_t user_space_log_uid;
	gid_t user_space_log_gid;
	char *user_space_log_buf;
	long user_space_log_len;
	long user_space_log_read_len;
	char excep_info[BFMR_SIZE_128];
	struct bfm_bf_log_info bootfail_log_info;
	unsigned int bootup_time;
	bool is_bootup_successfully;
};

typedef int (*bfmr_catch_save_bf_log)(struct bfm_param *param);

typedef int (*bfmr_process_ocp_excp)(enum bfmr_hw_fault_type fault_type,
	struct ocp_excp_info *pexcp_info);

struct bfm_log_save_param {
	bfmr_catch_save_bf_log catch_save_bf_log;
};

struct bfm_ocp_excp_param {
	bfmr_process_ocp_excp process_ocp_excp;
	struct bfm_ocp_excp_info ocp_excp_info[OCP_EXCP_INFO_COUNT];
};

struct bfm_chipsets_init_param {
	struct bfm_log_save_param log_saving_param;
	bfmr_process_ocp_excp process_ocp_excp;
};

struct bfm_ocp_test_param {
	int sleep_time; /* ms */
	int ldo_count; /* the total count of ldos to be test, such as 3 */
	int ldo_initial_idx; /* the beginning idx of the ldo, such as 21 */
};

#define BFM_CRITICAL_PROCESS_CRASH_LOG_NAME "critical_process_crash.txt"
#define BFM_TOMBSTONE_LOG_NAME "tombstone_00"
#define BFM_SYSTEM_SERVER_CRASH_LOG_NAME "system_server_crash@0.txt"
#define BFM_SYSTEM_SERVER_WATCHDOG_LOG_NAME "system_server_watchdog@0.txt.gz"
#define BFM_LOGCAT_FILE_PATH "/data/log/android_logs/applogcat-log"
#define BFM_OLD_LOGCAT_FILE_PATH "/data/log/android_logs/applogcat-log.1"
#define BFM_LOGCAT_FILE_NAME "applogcat-log"
#define BFM_LOGCAT_FILE_NAME_KEYWORD "applogcat-log"
#define BFM_FRAMEWORK_BOOTFAIL_LOG_PATH "/data/anr/framework_boot_fail.log"
#define BFM_FRAMEWORK_BOOTFAIL_LOG_FILE_NAME "framework_boot_fail.log"
#define BFM_BETA_KMSG_LOG_PATH "/data/log/android_logs/kmsgcat-log"
#define BFM_BETA_OLD_KMSG_LOG_PATH "/data/log/android_logs/kmsgcat-log.1"
#define BFM_BL1_LOG_FILENAME "xloader_log"
#define BFM_BL2_LOG_FILENAME "fastboot_log"
#define BFM_KERNEL_LOG_FILENAME "last_kmsg"
#define BFM_ALT_BL1_LOG_FILENAME "sbl1.log"
#define BFM_ALT_BL2_LOG_FILENAME "lk.log"
#define BFM_ALT_KERNEL_LOG_FILENAME "kmsg.log"
#define BFM_PMSG_LOG_FILENAME "pmsg-ramoops-0"
#define BFM_KERNEL_LOG_GZ_FILENAME_KEYWORD "last_kmsg.android"
#define BFM_BOOT_FAIL_LOGCAT_FILE_MAX_SIZE (512 * 1024)
#define BFM_UPLOADING_DIR_NAME "uploading"
#define BFM_BOOTFAIL_LOG_DIR_NAME_FORMAT "bootfail_%s_0x%08x"
#define BFM_BLOCK_CALLING_PROCESS_INTERVAL (3600 * 1000)

/*
 * @brief: capture log from system.
 *
 * @param: buf [out], buffer to save log.
 * @param: buf_len [in], length of buffer.
 * @param: src [in], src log info.
 * @param: timeout [in], timeout value of capturing log.
 *
 * @return: the length of the log has been captured.
 */
unsigned int bfmr_capture_log_from_system(char *buf, unsigned int buf_len,
	struct bfmr_log_src *src, int timeout);

/*
 * @brief: parse and save bottom layer bootfail log.
 *
 * @param: process_bootfail_param[in], bootfail process params.
 * @param: buf [in], buffer to save log.
 * @param: buf_len [in], length of buffer.
 *
 * @return: 0 - success, -1 - failed.
 *
 * @note: HISI must realize this function in detail, the other platform can
 * return 0 when enter this function
 */
int bfm_parse_and_save_bottom_layer_bootfail_log(
	struct bfm_param *process_bootfail_param,
	char *buf,
	unsigned int buf_len);
/*
 * @brief: save log to file system.
 *
 * @param: dst_file_path [in], full path of the dst log file.
 * @param: buf [in], buffer saving the boto fail log.
 * @param: log_len [in], length of the log.
 * @param: append [in], 0 - not append, 1 - append.
 *
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_save_log_to_fs(char *dst_file_path, char *buf, unsigned int log_len,
	int append);

/*
 * @brief: save log to raw partition.
 *
 * @param: raw_part_name [in], such as: rrecord/recovery/boot,
 *         not the full path of the device.
 * @param: offset [in], offset from the beginning of the "raw_part_name".
 * @param: buf [in], buffer saving log.
 * @param: log_len [in], length of the log which is <= the length of buf.
 *
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_save_log_to_raw_part(char *raw_part_name, unsigned long long offset,
	char *buf, unsigned int log_len);

/*
 * @brief: save log to memory buffer.
 *
 * @param: dst_buf [in] dst buffer.
 * @param: dst_buf_len [in], length of the dst buffer.
 * @param: src_buf [in] ,source buffer saving log.
 * @param: log_len [in], length of the buffer.
 *
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_save_log_to_mem_buffer(char *dst_buf, unsigned int dst_buf_len,
	const char *src_buf, unsigned int log_len);

/*
 * @brief: get log root path
 *
 * @return: the path of bfmr log's root path.
 */
char *bfm_get_bfmr_log_root_path(void);
char *bfm_get_bfmr_log_uploading_path(void);
char *bfm_get_bfmr_log_part_mount_point(void);
int bfm_get_boot_stage(enum bfmr_detail_stage *pbfmr_bootstage);
int bfm_set_boot_stage(enum bfmr_detail_stage bfmr_bootstage);
int bfm_capture_and_save_do_nothing_bootfail_log(struct bfm_param *param);

/*
 * @brief: process boot fail in chipsets module
 *
 * @param: param [int] params for further process boot fail in chipsets
 *
 * @return: 0 - succeeded; -1 - failed.
 *
 * @note: realize this function according to diffirent platforms
 */
int bfm_platform_process_boot_fail(struct bfm_param *param);

/*
 * @brief: process boot success in chipsets module
 *
 * @return: 0 - succeeded; -1 - failed.
 *
 * @note: HISI must realize this function in detail, the other platform can
 * return 0 when enter this function
 */
int bfm_platform_process_boot_success(void);
int bfm_is_system_rooted(void);
int bfm_is_user_sensible_bootfail(enum bfm_errno_code bootfail_errno,
	enum bfr_suggested_method suggested_recovery_method);
char *bfm_get_bl1_bootfail_log_name(void);
char *bfm_get_bl2_bootfail_log_name(void);
char *bfm_get_kernel_bootfail_log_name(void);
char *bfm_get_ramoops_bootfail_log_name(void);
char *bfm_get_platform_name(void);
unsigned int bfm_get_dfx_log_length(void);
int bfm_update_platform_logs(struct bfm_bf_log_info *pbootfail_log_info);

/*
 * @brief: copy dfx data to local buffer
 *
 * @return: 0 - succeeded; -1 - failed.
 *
 * @note: HISI must realize this function in detail, the other platform can
 * return when enter this function
 */
void bfmr_copy_data_from_dfx_to_bfmr_tmp_buffer(void);
char *bfm_get_raw_part_name(void);
int bfm_get_raw_part_offset(void);
void bfmr_alloc_and_init_raw_log_info(struct bfm_param *pparam,
	struct bfmr_log_dst *pdst);
void bfmr_save_and_free_raw_log_info(struct bfm_param *pparam);
void bfmr_update_raw_log_info(struct bfmr_log_src *psrc, struct bfmr_log_dst *pdst,
	unsigned int bytes_read);
unsigned int bfmr_capture_log_from_src_file(char *buf, unsigned int buf_len,
	char *src_log_path);
int bfm_process_ocp_boot_fail_test(void *param);
void bfm_set_valid_long_press_flag(void);
int bfm_chipsets_init(struct bfm_chipsets_init_param *param);

#ifdef __cplusplus
}
#endif

#endif

