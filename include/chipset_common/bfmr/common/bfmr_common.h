/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: bfmr_common.h
 *
 * define the common external public enum/macros/interface for BFMR
 *   (Boot Fail Monitor and Recovery)
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

#ifndef BFMR_COMMON_H
#define BFMR_COMMON_H

#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_PATH_SIZE 1024
#define DETAIL_INFO_LEN 512
#define RESERVED_INFO_SIZE 2556
#define PARAM_COMMAND_LEN 32
#define PARAM_RESERVED_SIZE 948
#define INFO_RESERVED_SIZE 240
#define LDO_NUM_SIZE 16
#define MOUNT_POINT_SIZE 32

enum bfmr_log_type {
	LOG_TYPE_BOOTLOADER_1 = 0,
	LOG_TYPE_BOOTLOADER_2,
	LOG_TYPE_BFMR_TEMP_BUF,
	LOG_TYPE_KMSG,
	LOG_TYPE_ANDROID_KMSG = LOG_TYPE_KMSG,
	LOG_TYPE_TEXT_KMSG,
	LOG_TYPE_RAMOOPS,
	LOG_TYPE_BETA_APP_LOGCAT,
	LOG_TYPE_CRITICAL_PROCESS_CRASH,
	LOG_TYPE_VM_TOMBSTONES,
	LOG_TYPE_VM_CRASH,
	LOG_TYPE_VM_WATCHDOG,
	LOG_TYPE_NORMAL_FRAMEWORK_BOOTFAIL_LOG,
	LOG_TYPE_FIXED_FRAMEWORK_BOOTFAIL_LOG,
	LOG_TYPE_BFM_BFI_LOG,
	LOG_TYPE_BFM_RECOVERY_LOG,
	LOG_TYPE_MAX_COUNT,
};

enum bfmr_dst_type {
	DST_FILE = 0,
	DST_RAW_PART,
	DST_MEMORY_BUFFER,
};

struct bfmr_log_dst {
	enum bfmr_dst_type type;

	union {
		char *filename;
		struct {
			char *addr;
			int len;
		} buffer;
		struct {
			char *raw_part_name;
			int offset;
		} raw_part;
	} dst_info;
};

struct bfmr_log_src {
	enum bfmr_log_type log_type;
	char *src_log_file_path;
	void *log_save_context;
	void *log_save_additional_context;
	bool save_log_after_reboot;
};

#if BITS_PER_LONG == 32
typedef   struct stat64   bfm_stat_t;
#define   bfm_sys_lstat sys_lstat64
#else
typedef   struct stat bfm_stat_t;
#define   bfm_sys_lstat sys_newlstat
#endif

enum bfmr_hw_fault_type {
	HW_FAULT_OCP = 0,
	HW_FAULT_MAX_COUNT,
};

enum bfmr_hw_fault_stage {
	HW_FAULT_STAGE_DURING_BOOTUP = 0,
	HW_FAULT_STAGE_AFTER_BOOT_SUCCESS,
	HW_FAULT_STAGE_MAX_COUNT,
};

/* bootfail additional info */
struct bfmr_bf_addl_info {
	char log_path[LOG_PATH_SIZE];
	char detail_info[DETAIL_INFO_LEN];
	enum bfmr_hw_fault_type hardware_fault_type;
	char reserved[RESERVED_INFO_SIZE];
};

struct bfmr_rrecord_misc_param {
	/* "boot-erecovery" */
	char command[PARAM_COMMAND_LEN];

	/* main reason */
	int enter_erecovery_reason;

	/* sub reason need for BI */
	int enter_erecovery_reason_number;

	/* boot stage when boot fail occurs */
	int boot_fail_stage_for_erecovery;

	unsigned int boot_fail_no;
	unsigned int recovery_method;

	/* mark if the misc write success,yes:0xAA55AA55 */
	unsigned int sync_misc_flag;

	/* abnormal shutdown flag */
	unsigned int abns_flag;

	unsigned int format_data_flag;
	unsigned int original_recovery_method;
	unsigned long long bopd_mode_value;

	/* reserved for future usage */
	char reserved[PARAM_RESERVED_SIZE];
} __attribute__((__packed__));

struct ocp_excp_info {
	char ldo_num[LDO_NUM_SIZE];
	char reserved[INFO_RESERVED_SIZE];
};

struct bfm_ocp_excp_info {
	struct ocp_excp_info excp_info;
	enum bfmr_hw_fault_type fault_type;
	struct semaphore sem;
};

struct bfmr_hw_fault_info_param {
	enum bfmr_hw_fault_stage fault_stage;
	union {
		struct ocp_excp_info ocp_excp_info;
	} hw_excp_info;
};

struct bfmr_part_mount_result {
	char mount_point[MOUNT_POINT_SIZE];
	bool mount_result;
};

#define BFMR_AID_ROOT 0
#define BFMR_AID_SYSTEM 1000

#define BFMR_DIR_LIMIT 0775
#define BFMR_FILE_LIMIT 0664

#ifdef __KERNEL__
#define bfmr_malloc vmalloc
#define bfmr_free vfree
#else
#define bfmr_malloc malloc
#define bfmr_free free
#endif

#define BFMR_SIZE_32 ((unsigned int)32)
#define BFMR_SIZE_64 ((unsigned int)64)
#define BFMR_SIZE_128 ((unsigned int)128)
#define BFMR_SIZE_256 ((unsigned int)256)
#define BFMR_SIZE_512 ((unsigned int)512)
#define BFMR_SIZE_1K ((unsigned int)1024)
#define BFMR_SIZE_2K ((unsigned int)2048)
#define BFMR_SIZE_4K ((unsigned int)4096)
#define BFMR_SIZE_32K ((unsigned int)32768)

#define BFMR_DEV_FULL_PATH_MAX_LEN BFMR_SIZE_256
#define BFMR_TEMP_BUF_LEN BFMR_SIZE_32K
#define BFMR_MAX_PATH BFMR_SIZE_4K

#define ENTER_ERECOVERY_BY_PRESS_KEY 2001
#define ENTER_ERECOVERY_BY_ADB_CMD 2002
#define ENTER_ERECOVERY_BECAUSE_UNLOCK 2004
#define ENTER_ERECOVERY_BECAUSE_SYSTEM_DAMAGED 2005
#define ENTER_ERECOVERY_BECAUSE_HUAWEI_VERIFY_FAILED 2006
#define ENTER_ERECOVERY_BECAUSE_HUAWEI_GOOGLE_VERIFY_FAILED 2007
#define ENTER_ERECOVERY_REASON_BECAUSE_BOOTLOADER_BOOT_FAIL 2008
#define ENTER_ERECOVERY_REASON_BECAUSE_KERNEL_BOOT_FAIL 2009
#define ENTER_ERECOVERY_BECAUSE_SYSTEM_MOUNT_FAILED 2010
#define ENTER_ERECOVERY_BECAUSE_DATA_MOUNT_FAILED 2011
#define ENTER_ERECOVERY_BECAUSE_DATA_MOUNT_RO 2012
#define ENTER_ERECOVERY_BECAUSE_KEY_PROCESS_START_FAILED 2013
#define ENTER_ERECOVERY_BECAUSE_RECOVERY_PROCESS_CRASH 2014
#define ENTER_ERECOVERY_BECAUSE_AP_CRASH_REPEATEDLY 2015
#define ENTER_ERECOVERY_BECAUSE_NON_AP_CRASH_REPEATEDLY 2016
#define ENTER_ERECOVERY_BECAUSE_APP_BOOT_FAIL 2017
#define ENTER_ERECOVERY_BECAUSE_SECURITY_FAIL 2018
#define ENTER_ERECOVERY_BECAUSE_VENDOR_MOUNT_FAILED 2019
#define ENTER_ERECOVERY_BECAUSE_CUST_MOUNT_FAILED 2020
#define ENTER_ERECOVERY_BECAUSE_PRODUCT_MOUNT_FAILED 2021
#define ENTER_ERECOVERY_BECAUSE_VERSION_MOUNT_FAILED 2022
#define ENTER_ERECOVERY_BECAUSE_HW_DEGRADE_BOOT_FAIL 2040
#define ENTER_ERECOVERY_UNKNOWN 2099

#define BFMR_PRINT_INVALID_PARAMS(format, ...) \
	pr_err("func: %s line: %d invalid parameters: "\
	format, __func__, __LINE__, ##__VA_ARGS__)
#define BFMR_PRINT_ERR(format, ...) \
	pr_err("func: %s line: %d, "\
	format, __func__, __LINE__, ##__VA_ARGS__)
#define BFMR_PRINT_INFO(format, ...) \
	pr_info("func: %s line: %d, "\
	format, __func__, __LINE__, ##__VA_ARGS__)
#define BFMR_PRINT_KEY_INFO(format, ...) \
	pr_err("func: %s line: %d, "\
	format, __func__, __LINE__, ##__VA_ARGS__)
#define BFMR_PRINT_SIMPLE_INFO(args...) pr_err(args)
#define BFMR_PRINT_DBG(format, ...) \
	pr_devel("func: %s line: %d, "\
	format, __func__, __LINE__, ##__VA_ARGS__)
#define BFMR_PRINT_ENTER(format, ...) \
	pr_info(">>>>enter func: %s, line: %d\n"\
	format, __func__, __LINE__, ##__VA_ARGS__)
#define BFMR_PRINT_EXIT(format, ...) \
	pr_info("<<<<exit func: %s, line: %d\n"\
	format, __func__, __LINE__, ##__VA_ARGS__)
#define BFMR_BOOTLOCK_FIELD_NAME "bootlock"
#define BFMR_ENABLE_FIELD_NAME "hw_bfm_enable"
#define BFR_ENABLE_FIELD_NAME "hw_bfr_enable"
#define BFR_RRECORD_PART_NAME "rrecord"
#define BFR_MISC_PART_OFFSET ((unsigned int)0x0)
#define BFR_RRECORD_PART_MAX_COUNT 2
#define BFMR_DEV_NAME "hw_bfm"
#define BFMR_DEV_PATH "/dev/hw_bfm"
#define BFM_LOG_MAX_COUNT 10
#define BFM_LOG_MAX_COUNT_PER_DIR 10
#define BFM_MAX_INT_NUMBER_LEN 21
#define BFMR_BOPD_MODE_FIELD_NAME "bopd.mode"
#define BFMR_MOUNT_NAME_SIZE 32

#define BFMR_SHA256_HASH_LEN 32

int bfmr_sha256(unsigned char *pout, unsigned int out_len,
	const void *pin, unsigned long in_len);

/*
 * @brief: get full path of the "dev_name".
 *
 * @param: dev_name [in] device name such as: boot/recovery/rrecord.
 * @param: path_buf [out] buffer will store the full path of "dev_name".
 * @param: path_buf_len [in] length of the path_buf.
 *
 * @return: 0 - succeeded; -1 - failed.
 *
 */
int bfmr_get_device_full_path(char *dev_name, char *path_buf,
	unsigned int path_buf_len);
int bfmr_read_emmc_raw_part(const char *dev_path,
	unsigned long long offset, char *buf,
	unsigned long long buf_size);
int bfmr_write_emmc_raw_part(const char *dev_path,
	unsigned long long offset, char *buf,
	unsigned long long buf_size);
void bfmr_change_own_mode(char *path, int uid, int gid, int mode);
void bfmr_change_file_ownership(char *path, uid_t uid, gid_t gid);
void bfmr_change_file_mode(char *path, umode_t mode);
int bfmr_get_file_ownership(char *pfile_path, uid_t *puid, gid_t *pgid);
bool bfmr_is_file_existed(const char *pfile_path);
bool bfmr_is_dir_existed(const char *pdir_path);
int bfmr_save_log(const char *logpath, const char *filename, void *buf,
	unsigned int len, unsigned int is_append);
long bfmr_get_proc_file_length(const char *pfile_path);
bool bfmr_is_part_mounted_rw(const char *pmount_point);
long bfmr_get_file_length(const char *pfile_path);
long long bfmr_get_fs_available_space(const char *pmount_point);
bool bfmr_is_part_ready_without_timeout(char *dev_name);
int bfmr_wait_for_part_mount_without_timeout(const char *pmount_point);
int bfmr_wait_for_part_mount_with_timeout(const char *pmount_point,
	int timeouts);
int bfmr_create_log_path(char *path);
char *bfmr_convert_rtc_time_to_asctime(unsigned long long rtc_time);
char *bfm_get_bootlock_value_from_cmdline(void);
bool bfmr_has_been_enabled(void);
bool bfr_has_been_enabled(void);
void bfmr_enable_ctl(int enable_flag);
char *bfmr_reverse_find_string(char *psrc, const char *pstr_to_be_found);
bool bfm_get_symbol_link_path(const char *file_path, char *psrc_path,
	size_t src_path_size);
long bfmr_full_read(int fd, char *buf, size_t buf_size);
long bfmr_full_write(int fd, char *buf, size_t buf_size);
long bfmr_full_read_with_file_path(const char *pfile_path, char *buf,
	size_t buf_size);
long bfmr_full_write_with_file_path(const char *pfile_path, char *buf,
	size_t buf_size);
void bfmr_unlink_file(char *pfile_path);
int bfmr_get_uid_gid(uid_t *puid, gid_t *pgid);
int bfmr_read_rrecord_misc_msg(struct bfmr_rrecord_misc_param *pparam);
int bfmr_write_rrecord_misc_msg(struct bfmr_rrecord_misc_param *pparam);
unsigned int bfmr_get_bootup_time(void);
char *bfm_get_boot_stage_name(unsigned int boot_stage);
bool bfm_is_beta_version(void);
bool bfmr_is_oversea_commercail_version(void);
int bfm_write_sub_bootfail_magic_num(unsigned int magic_num, void *phys_addr);
int bfm_write_sub_bootfail_num(unsigned int bootfail_errno, void *phys_addr);
int bfm_write_sub_bootfail_count(unsigned int bootfail_count, void *phys_addr);
int bfmr_common_init(void);
void bfmr_set_mount_state(const char *bfmr_mount_point, bool mount_result,
	unsigned int size);

#ifdef __cplusplus
}
#endif

#endif

