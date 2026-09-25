/*
 * bfm_core.c
 *
 * define the core's external public enum/macros/interface for
 * BSP(Boot Fail Measure)
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/reboot.h>
#include <linux/delay.h>
#include <linux/dirent.h>
#include <linux/statfs.h>
#include <linux/rtc.h>
#include <linux/cpu.h>

#include "chipset_common/bfmr/bfm/core/bfm_core.h"
#include "chipset_common/bfmr/bfm/chipsets/bfm_chipsets.h"
#include "chipset_common/bfmr/bfm/core/bfm_timer.h"
#include "chipset_common/bfmr/bfm/core/bfm_stage_info.h"
#include "chipset_common/bfmr/bfr/core/bfr_core.h"
#ifdef CONFIG_HUAWEI_DYNAMIC_BRD
#include "chipset_common/storage_rofa/dynbrd_public.h"
#endif
#ifdef CONFIG_HUAWEI_STORAGE_ROFA
#include "chipset_common/storage_rofa/storage_rofa.h"
#endif

#define BFMR_LLSEEK no_llseek
#define BFM_BFI_FILE_NAME "bootFail_info.txt"
#define BFM_RCV_FILE_NAME "recovery_info.txt"
#define BFM_LOG_END_TAG_PER_LINE "\r\n"
#define BFM_DONE_FILE_NAME "DONE"
#define BFM_PERCENT 100
#define BFM_BFI_CONTENT_FORMAT \
	"time%s:%s\r\n" \
	"bootFailErrno:%s\r\n" \
	"boot_stage:%s_%s\r\n" \
	"isSystemRooted:%d\r\n" \
	"isUserPerceptible:%d\r\n" \
	"SpaceLeftOnData:%lldMB [%lld%%]\r\n" \
	"iNodesUsedOnData:%lld [%lld%%]\r\n" \
	"\r\n" \
	"time%s:0x%llx\r\n" \
	"bootFailErrno:0x%x\r\n" \
	"boot_stage:0x%x\r\n" \
	"isSystemRooted:%d\r\n" \
	"isUserPerceptible:%d\r\n"\
	"dmdErrNo:%d\r\n"\
	"bootFailDetail:%s\r\n"\
	"bootup_time:%dS\r\n" \
	"isBootUpSuccessfully:%s\r\n" \
	"RebootType:0x%x\r\n" \
	"\r\n"\
	"the bootlock field in cmdline is: [%s] this time\r\n"

#define BFM_RCV_CONTENT_FORMAT \
	"rcvMethod%s:%s\r\n" \
	"rcvResult%s:%s\r\n" \
	"\r\n" \
	"rcvMethod%s:%d\r\n" \
	"rcvResult%s:%d\r\n" \
	"selfHealChain%s:%s\r\n"

#define BFM_RCV_SUCCESS_STR "success"
#define BFM_RCV_SUCCESS_VALUE 1
#define BFM_RCV_FAIL_STR "fail"
#define BFM_RCV_FAIL_VALUE 0
#define BFM_PBL_LOG_MAX_LEN BFMR_SIZE_1K
#define BFM_BL1_LOG_MAX_LEN BFMR_SIZE_1K
#define BFM_BL2_LOG_MAX_LEN (128 * BFMR_SIZE_1K)
#define BFM_KMSG_LOG_MAX_LEN (BFMR_SIZE_1K * BFMR_SIZE_1K)
#define BFM_KMSG_TEXT_LOG_MAX_LEN (512 * BFMR_SIZE_1K)
#define BFM_LOG_RAMOOPS_LOG_MAX_LEN (128 * BFMR_SIZE_1K)
#define BFM_APP_BETA_LOGCAT_LOG_MAX_LEN (2048 * BFMR_SIZE_1K)
#define BFM_CRITICAL_PROCESS_CRASH_LOG_MAX_LEN (128 * BFMR_SIZE_1K)
#define BFM_VM_TOMBSTONES_LOG_MAX_LEN (256 * BFMR_SIZE_1K)
#define BFM_VM_CRASH_LOG_MAX_LEN (128 * BFMR_SIZE_1K)
#define BFM_VM_WATCHDOG_LOG_MAX_LEN (256 * BFMR_SIZE_1K)
#define BFM_NORMAL_FRAMEWORK_BOOTFAIL_LOG_MAX_LEN (256 * BFMR_SIZE_1K)
#define BFM_FIXED_FRAMEWORK_BOOTFAIL_LOG_MAX_LEN (256 * BFMR_SIZE_1K)
#define BFM_BF_INFO_LOG_MAX BFMR_SIZE_1K
#define BFM_RCV_INFO_LOG_MAX BFMR_SIZE_1K
#define BFM_BASIC_LOG_MAX_LEN (BFM_BF_INFO_LOG_MAX + BFM_RCV_INFO_LOG_MAX)
#define BFM_BFMR_TEMP_BUF_LOG_MAX_LEN \
	(BFM_BL1_LOG_MAX_LEN + BFM_BL2_LOG_MAX_LEN + \
	 BFM_KMSG_LOG_MAX_LEN + BFM_LOG_RAMOOPS_LOG_MAX_LEN)

#define BFM_TOMBSTONE_FILE_NAME_TAG "tombstone"
#define BFM_SYSTEM_SERVER_CRASH_FILE_NAME_TAG "system_server_crash"
#define BFM_SYSTEM_SERVER_WATCHDOG_FILE_NAME_TAG "system_server_watchdog"
#define BFM_SAVE_LOG_INTERVAL 1000
#define BFM_US_PERSECOND 1000000
#define BFM_SAVE_LOG_MAX_TIME 6000

#define BFM_DATA_PART_TIME_OUT 5
#define BFM_LOG_SEPRATOR "================time:%s================\r\n"
#define BFM_LOG_SEPRATOR_NEWLINE "\n" BFM_LOG_SEPRATOR
#define BFM_NOT_SENSIBLE_MAX_CNT 2
#define BFM_USER_MAX_TOLERANT_BOOTTIME 60
#define BFM_BOOT_SLOWLY_THRESHOLD (5 * 60)

#define SIG_TO_INIT 40
#define SIG_INT_VALUE 1234

#define BFM_TEXT_LOG_SEPRATOR_ADD_TIME \
	"starttime:%s\r\n" \
	"endtime:%lld\r\n" \
	"%s"

#define BFM_SELF_HEAL_CHAIN_MAX 256
#define BFM_SELF_HEAL_STAGE_OFF_LEN 24
#define BFM_BOOT_STAGE_SHIFTING 24 /* 3 byte */

struct bfm_log_type_buffer_info {
	enum bfmr_log_type log_type;
	unsigned int buf_len;
	char *buf;
};

struct bfm_boot_fail_no_desc {
	enum bfm_errno_code bootfail_errno;
	char *desc;
};

struct bfm_log_size_param {
	enum bfmr_detail_stage boot_stage;
	long long log_size_allowed;
};

static long bfmr_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int bfmr_open(struct inode *inode, struct file *file);
static ssize_t bfmr_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos);
static ssize_t bfmr_write(struct file *file, const char *data,
		size_t len, loff_t *ppos);
static int bfmr_release(struct inode *inode, struct file *file);
static int bfm_update_info_for_each_log(void);
static int bfm_notify_boot_success(void *param);
static int bfm_lookup_dir_by_create_time(const char *root,
		char *log_path, size_t log_path_len, int find_oldest_log);
static void bfm_wait_for_compeletion_of_processing_boot_fail(void);

static void bfm_delete_oldest_log(long long bytes_need_this_time);
static long long bfm_get_basic_space(enum bfm_errno_code bootfail_errno);
static long long bfm_get_extra_space(struct bfm_param *pparam);
static int bfm_save_extra_bootfail_logs(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam);
static int bfm_capture_and_save_bl1_bootfail_log(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam);
static int bfm_capture_and_save_bl2_bootfail_log(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam);
static int bfm_capture_and_save_kernel_bootfail_log(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam);
static int bfm_capture_and_save_ramoops_bootfail_log(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam);
static int bfm_capture_and_save_native_bootfail_log(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam);
static int bfm_capture_and_save_framework_bootfail_log(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam);
static bool bfm_is_log_existed(unsigned long long rtc_time,
		unsigned int bootfail_errno);
static int bfm_catch_save_bf_log(struct bfm_param *pparam);
static void bfm_process_after_save_bootfail_log(void);
static char *bfm_get_bf_no_desc(enum bfm_errno_code bootfail_errno,
		struct bfm_param *pparam);
static int bfm_save_bf_info_txt(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam);
static void bfm_save_rcv_info_txt(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam);
static char *bfm_get_file_name(const char *file_full_path);
static int bfm_process_upper_layer_boot_fail(void *param);
static int bfm_process_bottom_layer_boot_fail(void *param);
static int bfmr_capture_and_save_bottom_layer_boot_fail_log(void);
/*
 * @function: static int bfmr_capture_and_save_log(enum bfmr_log_type type,
 * struct bfmr_log_dst *dst)
 * @brief: capture and save log, initialised in core.
 * @param: src [in] log src info.
 * @param: dst [in] infomation about the media of saving log.
 * @return: 0 - succeeded; -1 - failed.
 * @note: it need be initialized in bootloader and kernel.
 */
static int bfmr_capture_and_save_log(struct bfmr_log_src *src,
		struct bfmr_log_dst *dst, struct bfm_param *pparam);
static unsigned long long bfm_get_system_time(void);
static int bfm_get_fs_state(const char *pmount_point,
		struct statfs *pstatfsbuf);
static bool bfm_is_user_sensible(struct bfm_bf_log_info *bf_log);
static void bfm_merge_bootfail_logs(struct bfm_bf_log_info *bf_log);
static void bfm_delete_user_unsensible_logs(struct bfm_bf_log_info *bf_log);
static int bfm_traverse_log_root_dir(struct bfm_bf_log_info *bf_log);
static void bfm_user_space_process_read_its_own_file(struct bfm_param *pparam);

static const struct file_operations bfmr_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = bfmr_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = bfmr_ioctl,
#endif
	.open = bfmr_open,
	.read = bfmr_read,
	.write = bfmr_write,
	.release = bfmr_release,
	.llseek = BFMR_LLSEEK,
};

static struct miscdevice bfmr_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = BFMR_DEV_NAME,
	.fops = &bfmr_fops,
};

static DEFINE_MUTEX(process_boot_stage_mutex);
static DEFINE_MUTEX(process_boot_fail_mutex);
static DECLARE_COMPLETION(process_boot_fail_comp);

static struct bfmr_rmethod_maptable rmethod_maptable[] = {
	/* Map Table for BL1 Stage */
	{
		FRM_DO_NOTHING,
		BL1_STAGE_START,
		SH_BL1_SELFHEAL
	}, {
		FRM_REBOOT,
		BL1_STAGE_START,
		SH_BL1_REBOOT
	}, {
		FRM_ENTER_SAFE_MODE,
		BL1_STAGE_START,
		SH_BL1_SAFE_MODE
	}, {
		FRM_FACTORY_RST,
		BL1_STAGE_START,
		SH_BL1_FACTORY_RESET
	}, {
		FRM_FORMAT_DATA_PART,
		BL1_STAGE_START,
		SH_BL1_FACTORY_RESET
	}, {
		FRM_LOWLEVEL_FORMAT_DATA,
		BL1_STAGE_START,
		SH_BL1_FACTORY_RESET
	}, {
		FRM_DOWNLOAD,
		BL1_STAGE_START,
		SH_BL1_DOWNLOAD_VERSION
	}, {
		FRM_DOWNLOAD_AND_DEL_FILES,
		BL1_STAGE_START,
		SH_BL1_DOWNLOAD_VERSION
	}, {
		FRM_DOWNLOAD_AND_FACTORY_RST,
		BL1_STAGE_START,
		SH_BL1_DOWNLOAD_VERSION
	}, {
		FRM_FACTORY_RST_AFTER_DOWNLOAD,
		BL1_STAGE_START,
		SH_BL1_FACTORY_RESET
	}, {
		FRM_DO_NOTHING, /* Map Table for BL2 Stage */
		BL2_STAGE_START,
		SH_BL2_SELFHEAL
	}, {
		FRM_REBOOT,
		BL2_STAGE_START,
		SH_BL2_REBOOT
	}, {
		FRM_ENTER_SAFE_MODE,
		BL2_STAGE_START,
		SH_BL2_SAFE_MODE
	}, {
		FRM_FACTORY_RST,
		BL2_STAGE_START,
		SH_BL2_FACTORY_RESET
	}, {
		FRM_FORMAT_DATA_PART,
		BL2_STAGE_START,
		SH_BL2_FACTORY_RESET
	}, {
		FRM_LOWLEVEL_FORMAT_DATA,
		BL2_STAGE_START,
		SH_BL2_FACTORY_RESET
	}, {
		FRM_DOWNLOAD,
		BL2_STAGE_START,
		SH_BL2_DOWNLOAD_VERSION
	}, {
		FRM_DOWNLOAD_AND_DEL_FILES,
		BL2_STAGE_START,
		SH_BL2_DOWNLOAD_VERSION
	}, {
		FRM_DOWNLOAD_AND_FACTORY_RST,
		BL2_STAGE_START,
		SH_BL2_DOWNLOAD_VERSION
	}, {
		FRM_FACTORY_RST_AFTER_DOWNLOAD,
		BL2_STAGE_START,
		SH_BL2_FACTORY_RESET
	}, {
		FRM_DO_NOTHING, /* Map Table for Kernel Stage */
		KERNEL_STAGE_START,
		SH_KERNEL_SELFHEAL
	}, {
		FRM_REBOOT,
		KERNEL_STAGE_START,
		SH_KERNEL_REBOOT
	}, {
		FRM_ENTER_SAFE_MODE,
		KERNEL_STAGE_START,
		SH_KERNEL_SAFE_MODE
	}, {
		FRM_FACTORY_RST,
		KERNEL_STAGE_START,
		SH_KERNEL_FACTORY_RESET
	}, {
		FRM_FORMAT_DATA_PART,
		KERNEL_STAGE_START,
		SH_KERNEL_FACTORY_RESET
	}, {
		FRM_LOWLEVEL_FORMAT_DATA,
		KERNEL_STAGE_START,
		SH_KERNEL_FACTORY_RESET
	}, {
		FRM_DOWNLOAD,
		KERNEL_STAGE_START,
		SH_KERNEL_DOWNLOAD_VERSION
	}, {
		FRM_DOWNLOAD_AND_DEL_FILES,
		KERNEL_STAGE_START,
		SH_KERNEL_DOWNLOAD_VERSION
	}, {
		FRM_DOWNLOAD_AND_FACTORY_RST,
		KERNEL_STAGE_START,
		SH_KERNEL_DOWNLOAD_VERSION
	}, {
		FRM_FACTORY_RST_AFTER_DOWNLOAD,
		KERNEL_STAGE_START,
		SH_KERNEL_FACTORY_RESET
	}, {
		FRM_BOPD,
		KERNEL_STAGE_START,
		SH_KERNEL_BOPD
	}, {
		FRM_STORAGE_RDONLY_BOPD,
		KERNEL_STAGE_START,
		SH_KERNEL_STORAGE_RDONLY_BOPD
	}, {
		FRM_HW_DEGRADE_BOPD,
		KERNEL_STAGE_START,
		SH_KERNEL_HARDWARE_DEGRADE_BOPD
	}, {
		FRM_BOPD_AFTER_DOWNLOAD,
		KERNEL_STAGE_START,
		SH_KERNEL_BOPD_AFTER_DOWNLOAD
	}, {
		FRM_HW_DEGRADE_BOPD_AFTER_DOWNLOAD,
		KERNEL_STAGE_START,
		SH_KERNEL_HARDWARE_DEGRADE_BOPD_AFTER_DOWNLOAD
	}, {
		FRM_DO_NOTHING, /* Map Table for Native Stage */
		NATIVE_STAGE_START,
		SH_NATIVE_SELFHEAL
	}, {
		FRM_REBOOT,
		NATIVE_STAGE_START,
		SH_NATIVE_REBOOT
	}, {
		FRM_ENTER_SAFE_MODE,
		NATIVE_STAGE_START,
		SH_NATIVE_SAFE_MODE
	}, {
		FRM_FACTORY_RST,
		NATIVE_STAGE_START,
		SH_NATIVE_FACTORY_RESET
	}, {
		FRM_FORMAT_DATA_PART,
		NATIVE_STAGE_START,
		SH_NATIVE_FACTORY_RESET
	}, {
		FRM_LOWLEVEL_FORMAT_DATA,
		NATIVE_STAGE_START,
		SH_NATIVE_FACTORY_RESET
	}, {
		FRM_DOWNLOAD,
		NATIVE_STAGE_START,
		SH_NATIVE_DOWNLOAD_VERSION
	}, {
		FRM_DOWNLOAD_AND_DEL_FILES,
		NATIVE_STAGE_START,
		SH_NATIVE_DOWNLOAD_VERSION
	}, {
		FRM_DOWNLOAD_AND_FACTORY_RST,
		NATIVE_STAGE_START,
		SH_NATIVE_DOWNLOAD_VERSION
	}, {
		FRM_FACTORY_RST_AFTER_DOWNLOAD,
		NATIVE_STAGE_START,
		SH_NATIVE_FACTORY_RESET
	}, {
		FRM_BOPD,
		NATIVE_STAGE_START,
		SH_NATIVE_BOPD
	}, {
		FRM_STORAGE_RDONLY_BOPD,
		NATIVE_STAGE_START,
		SH_NATIVE_STORAGE_RDONLY_BOPD
	}, {
		FRM_HW_DEGRADE_BOPD,
		NATIVE_STAGE_START,
		SH_NATIVE_HARDWARE_DEGRADE_BOPD
	}, {
		FRM_BOPD_AFTER_DOWNLOAD,
		NATIVE_STAGE_START,
		SH_NATIVE_BOPD_AFTER_DOWNLOAD
	}, {
		FRM_HW_DEGRADE_BOPD_AFTER_DOWNLOAD,
		NATIVE_STAGE_START,
		SH_NATIVE_HARDWARE_DEGRADE_BOPD_AFTER_DOWNLOAD
	}, {
		FRM_DO_NOTHING, /* Map Table for Framework Stage */
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_SELFHEAL
	}, {
		FRM_REBOOT,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_REBOOT
	}, {
		FRM_ENTER_SAFE_MODE,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_SAFE_MODE
	}, {
		FRM_FACTORY_RST,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_FACTORY_RESET
	}, {
		FRM_FORMAT_DATA_PART,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_FACTORY_RESET
	}, {
		FRM_LOWLEVEL_FORMAT_DATA,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_FACTORY_RESET
	}, {
		FRM_DOWNLOAD,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_DOWNLOAD_VERSION
	}, {
		FRM_DOWNLOAD_AND_DEL_FILES,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_DOWNLOAD_VERSION
	}, {
		FRM_DOWNLOAD_AND_FACTORY_RST,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_DOWNLOAD_VERSION
	}, {
		FRM_FACTORY_RST_AFTER_DOWNLOAD,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_FACTORY_RESET
	}, {
		FRM_BOPD,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_BOPD
	}, {
		FRM_STORAGE_RDONLY_BOPD,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_STORAGE_RDONLY_BOPD
	}, {
		FRM_HW_DEGRADE_BOPD,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_HARDWARE_DEGRADE_BOPD
	}, {
		FRM_BOPD_AFTER_DOWNLOAD,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_BOPD_AFTER_DOWNLOAD
	}, {
		FRM_HW_DEGRADE_BOPD_AFTER_DOWNLOAD,
		ANDROID_FRAMEWORK_STAGE_START,
		SH_FRAMEWORK_HARDWARE_DEGRADE_BOPD_AFTER_DOWNLOAD
	},
};

/* modifiy this array if you add new log type */
static struct bfm_log_type_buffer_info log_type_buffer_info[LOG_TYPE_MAX_COUNT] = {
	{ LOG_TYPE_BOOTLOADER_1, BFM_BL1_LOG_MAX_LEN, NULL },
	{ LOG_TYPE_BOOTLOADER_2, BFM_BL2_LOG_MAX_LEN, NULL },
	{ LOG_TYPE_BFMR_TEMP_BUF, BFM_BFMR_TEMP_BUF_LOG_MAX_LEN, NULL },
	{ LOG_TYPE_ANDROID_KMSG, BFM_KMSG_LOG_MAX_LEN, NULL },
	{ LOG_TYPE_RAMOOPS, BFM_LOG_RAMOOPS_LOG_MAX_LEN, NULL },
	{ LOG_TYPE_BETA_APP_LOGCAT, BFM_APP_BETA_LOGCAT_LOG_MAX_LEN, NULL },
	{ LOG_TYPE_CRITICAL_PROCESS_CRASH,
	  BFM_CRITICAL_PROCESS_CRASH_LOG_MAX_LEN, NULL },
	{ LOG_TYPE_VM_TOMBSTONES, BFM_VM_TOMBSTONES_LOG_MAX_LEN, NULL },
	{ LOG_TYPE_VM_CRASH, BFM_VM_CRASH_LOG_MAX_LEN, NULL },
	{ LOG_TYPE_VM_WATCHDOG, BFM_VM_WATCHDOG_LOG_MAX_LEN, NULL },
	{ LOG_TYPE_NORMAL_FRAMEWORK_BOOTFAIL_LOG,
	  BFM_NORMAL_FRAMEWORK_BOOTFAIL_LOG_MAX_LEN, NULL },
	{ LOG_TYPE_FIXED_FRAMEWORK_BOOTFAIL_LOG,
	  BFM_FIXED_FRAMEWORK_BOOTFAIL_LOG_MAX_LEN, NULL },
	{ LOG_TYPE_TEXT_KMSG, BFM_KMSG_TEXT_LOG_MAX_LEN, NULL },
};

/* modifiy this array according to the log count captured for each bootfail */
struct bfm_log_size_param log_size_param[] = {
	{
		(PBL_STAGE << BFM_BOOT_STAGE_SHIFTING),
		BFM_PBL_LOG_MAX_LEN + BFM_BASIC_LOG_MAX_LEN
	}, {
		(BL1_STAGE << BFM_BOOT_STAGE_SHIFTING),
		BFM_BL1_LOG_MAX_LEN + BFM_BASIC_LOG_MAX_LEN
	}, {
		(BL2_STAGE << BFM_BOOT_STAGE_SHIFTING),
		BFM_BL1_LOG_MAX_LEN + BFM_BL2_LOG_MAX_LEN +
		BFM_BASIC_LOG_MAX_LEN
	}, {
		(KERNEL_STAGE << BFM_BOOT_STAGE_SHIFTING),
		BFM_BL1_LOG_MAX_LEN + BFM_BL2_LOG_MAX_LEN +
		BFM_KMSG_LOG_MAX_LEN + BFM_LOG_RAMOOPS_LOG_MAX_LEN +
		BFM_BASIC_LOG_MAX_LEN
	}, {
		(NATIVE_STAGE << BFM_BOOT_STAGE_SHIFTING),
		BFM_BL1_LOG_MAX_LEN + BFM_BL2_LOG_MAX_LEN +
		BFM_KMSG_LOG_MAX_LEN + BFM_LOG_RAMOOPS_LOG_MAX_LEN +
		BFM_VM_TOMBSTONES_LOG_MAX_LEN + BFM_BASIC_LOG_MAX_LEN
	}, {
		(ANDROID_FRAMEWORK_STAGE << BFM_BOOT_STAGE_SHIFTING),
		BFM_BL1_LOG_MAX_LEN + BFM_BL2_LOG_MAX_LEN +
		BFM_KMSG_LOG_MAX_LEN + BFM_LOG_RAMOOPS_LOG_MAX_LEN +
		BFM_VM_WATCHDOG_LOG_MAX_LEN + BFM_BASIC_LOG_MAX_LEN
	},
};

/* modifiy this array if you add new boot fail no */
static struct bfm_boot_fail_no_desc bf_errno_desc[] = {
	{ BL1_DDR_INIT_FAIL, "bl1 ddr init failed" },
	{ BL1_EMMC_INIT_FAIL, "bl1 emmc init failed" },
	{ BL1_BL2_LOAD_FAIL, "load image image failed" },
	{ BL1_BL2_VERIFY_FAIL, "verify image failed" },
	{ BL2_EMMC_INIT_FAIL, "bl2 emmc init failed" },
	{ BL1_WDT, "bl1 wdt" },
	{ BL1_VRL_LOAD_FAIL, "bl1 vrl load error" },
	{ BL1_VRL_VERIFY_FAIL, "bl1 vrl verify image error" },
	{ BL1_ERROR_GROUP_BOOT, "bl1 group boot error" },
	{ BL1_ERROR_GROUP_BUSES, "bl1 group buses error" },
	{ BL1_ERROR_GROUP_BAM, "bl1 group bam" },
	{ BL1_ERROR_GROUP_BUSYWAIT, "bl1 group busy wait" },
	{ BL1_ERROR_GROUP_CLK, "bl1 group clock" },
	{ BL1_ERROR_GROUP_CRYPTO, "bl1 group crypto" },
	{ BL1_ERROR_GROUP_DAL, "bl1 group dal" },
	{ BL1_ERROR_GROUP_DEVPROG, "bl1 group devprog" },
	{ BL1_ERROR_GROUP_DEVPROG_DDR, "bl1 group devprog ddr" },
	{ BL1_ERROR_GROUP_EFS, "bl1 group efs" },
	{ BL1_ERROR_GROUP_EFS_LITE, "bl1 group efs lite" },
	{ BL1_ERROR_GROUP_HOTPLUG, "bl1 group hot-plug" },
	{ BL1_ERROR_GROUP_HSUSB, "bl1 group high speed usb" },
	{ BL1_ERROR_GROUP_ICB, "bl1 group icb" },
	{ BL1_ERROR_GROUP_LIMITS, "bl1 group limits" },
	{ BL1_ERROR_GROUP_MHI, "bl1 group mhi" },
	{ BL1_ERROR_GROUP_PCIE, "bl1 group pcie" },
	{ BL1_ERROR_GROUP_PLATFOM, "bl1 group platform" },
	{ BL1_ERROR_GROUP_PMIC, "bl1 group pmic" },
	{ BL1_ERROR_GROUP_POWER, "bl1 group power" },
	{ BL1_ERROR_GROUP_PRNG, "bl1 group prng" },
	{ BL1_ERROR_GROUP_QUSB, "bl1 group qusb" },
	{ BL1_ERROR_GROUP_SECIMG, "bl1 group secimg" },
	{ BL1_ERROR_GROUP_SECBOOT, "bl1 group secboot" },
	{ BL1_ERROR_GROUP_SECCFG, "bl1 group seccfg" },
	{ BL1_ERROR_GROUP_SMEM, "bl1 group smem" },
	{ BL1_ERROR_GROUP_SPMI, "bl1 group spmi" },
	{ BL1_ERROR_GROUP_SUBSYS, "bl1 group subsystem" },
	{ BL1_ERROR_GROUP_TLMM, "bl1 group tlmm" },
	{ BL1_ERROR_GROUP_TSENS, "bl1 group tsensor" },
	{ BL1_ERROR_GROUP_HWENGINES, "bl1 group hwengines" },
	{ BL1_ERROR_GROUP_IMAGE_VERSION, "bl1 group image version" },
	{ BL1_ERROR_GROUP_SECURITY, "bl1 group system security" },
	{ BL1_ERROR_GROUP_STORAGE, "bl1 group storage" },
	{ BL1_ERROR_GROUP_SYSTEMDRIVERS, "bl1 group system drivers" },
	{ BL1_ERROR_GROUP_EXCEPTIONS, "bl1 group exceptions" },
	{ BL1_ERROR_GROUP_MPROC, "bl1 group mppoc" },
	{ BL2_PANIC, "bl2 panic" },
	{ BL2_WDT, "bl2 wdt" },
	{ BL2_PL1_OCV_ERROR, "ocv error" },
	{ BL2_PL1_BAT_TEMP_ERROR, "battery temperature error" },
	{ BL2_PL1_MISC_ERROR, "misc part dmaged" },
	{ BL2_PL1_DTIMG_ERROR, "dt image dmaged" },
	{ BL2_PL1_LOAD_OTHER_IMGS_ERRNO, "image dmaged" },
	{ BL2_PL1_KERNEL_IMG_ERROR, "kernel image verify failed" },
	{ BL2_MMC_INIT_FAILED, "bl2 mmc init error" },
	{ BL2_QSEECOM_START_ERROR, "bl2 qseecom start error" },
	{ BL2_RPMB_INIT_FAILED, "bl2 rpmb init failed" },
	{ BL2_LOAD_SECAPP_FAILED, "bl2 load secapp failed" },
	{ BL2_ABOOT_DLKEY_DETECTED, "bl2 dlkey failed" },
	{ BL2_ABOOT_NORMAL_BOOT_FAIL, "bl2 aboot normal boot failed" },
	{ BL2_BR_POWERON_BY_SMPL, "bl2 poweron by smpl" },
	{ BL2_FASTBOOT_S_LOADLPMCU_FAIL, "bl2 load lpmcu failed" },
	{ BL2_FASTBOOT_S_IMG_VERIFY_FAIL, "bl2 image verify failed" },
	{ BL2_FASTBOOT_S_SOC_TEMP_ERR, "bl2 soc tmp err" },
	{ BL2_PL1_BAT_LOW_POWER, "battery low power" },
	{ KERNEL_AP_PANIC, "kernel ap panic" },
	{ KERNEL_EMMC_INIT_FAIL, "kernel emm init failed" },
	{ KERNEL_AP_WDT, "kernel ap wdt" },
	{ KERNEL_PRESS10S, "kernel press10s" },
	{ KERNEL_BOOT_TIMEOUT, "kernel boot timeout" },
	{ KERNEL_AP_COMBINATIONKEY, "kernel ap combinationkey" },
	{ KERNEL_AP_S_ABNORMAL, "kernel ap abnormal" },
	{ KERNEL_AP_S_TSENSOR0, "kernel ap tsensor0" },
	{ KERNEL_AP_S_TSENSOR1, "kernel ap tsensor1" },
	{ KERNEL_LPM3_S_GLOBALWDT, "kernel lpm3 global wdt" },
	{ KERNEL_G3D_S_G3DTSENSOR, "kernel g3d g3dtsensor" },
	{ KERNEL_LPM3_S_LPMCURST, "kernel lpm3 lp mcu reset" },
	{ KERNEL_CP_S_CPTSENSOR, "kernel cp cpt sensor" },
	{ KERNEL_IOM3_S_IOMCURST, "kernel iom3 io mcu reset" },
	{ KERNEL_ASP_S_ASPWD, "kernel asp as pwd" },
	{ KERNEL_CP_S_CPWD, "kernel cp cp pwd" },
	{ KERNEL_IVP_S_IVPWD, "kernel ivp iv pwd" },
	{ KERNEL_ISP_S_ISPWD, "kernel isp is pwd" },
	{ KERNEL_AP_S_DDR_UCE_WD, "kernel ap ddr uce wd" },
	{ KERNEL_AP_S_DDR_FATAL_INTER, "kernel ap ddr fatal inter" },
	{ KERNEL_AP_S_DDR_SEC, "kernel ap ddr sec" },
	{ KERNEL_AP_S_MAILBOX, "kernel ap mailbox" },
	{ KERNEL_CP_S_MODEMDMSS, "kernel cp modem dmss" },
	{ KERNEL_CP_S_MODEMNOC, "kernel cp modem noc" },
	{ KERNEL_CP_S_MODEMAP, "kernel cp modem ap" },
	{ KERNEL_CP_S_EXCEPTION, "kernel cp exception" },
	{ KERNEL_CP_S_RESETFAIL, "kernel cp reset failed" },
	{ KERNEL_CP_S_NORMALRESET, "kernel cp normal reset" },
	{ KERNEL_CP_S_ABNORMAL, "kernel cp abnormal" },
	{ KERNEL_LPM3_S_EXCEPTION, "kernel lpm3 exception" },
	{ KERNEL_HIFI_S_EXCEPTION, "kernel hisi exception" },
	{ KERNEL_HIFI_S_RESETFAIL, "kernel hisi reset failed" },
	{ KERNEL_ISP_S_EXCEPTION, "kernel isp exception" },
	{ KERNEL_IVP_S_EXCEPTION, "kernel ivp exception" },
	{ KERNEL_IOM3_S_EXCEPTION, "kernel iom3 exception" },
	{ KERNEL_TEE_S_EXCEPTION, "kernel tee exception" },
	{ KERNEL_MMC_S_EXCEPTION, "kernel mmc exception" },
	{ KERNEL_CODECHIFI_S_EXCEPTION, "kernel codec hifi exception" },
	{ KERNEL_CP_S_RILD_EXCEPTION, "kernel cp rild exception" },
	{ KERNEL_CP_S_3RD_EXCEPTION, "kernel cp 3rd exception" },
	{ KERNEL_IOM3_S_USER_EXCEPTION, "kernel iom3 user exception" },
	{ KERNEL_MODEM_PANIC, "kernel modem panic" },
	{ KERNEL_VENUS_PANIC, "kernel venus panic" },
	{ KERNEL_WCNSS_PANIC, "kernel wcnss panic" },
	{ KERNEL_SENSOR_PANIC, "kernel sensor panic" },
	{ KERNEL_OCBC_S_WD, "kernel ocbc wd" },
	{ KERNEL_AP_S_NOC, "kernel noc" },
	{ KERNEL_AP_S_RESUME_SLOWY, "kernel resume slowly" },
	{ KERNEL_AP_S_F2FS, "kernel f2fs abnormal" },
	{ KERNLE_AP_S_BL31_PANIC, "kernel bl31 panic" },
	{ KERNLE_HISEE_S_EXCEPTION, "kernel hisee exception" },
	{ KERNEL_AP_S_PMU, "kernel ap pmu" },
	{ KERNEL_AP_S_SMPL, "kernel ap smpl" },
	{ KERNLE_AP_S_SCHARGER, "kernel charger abnormal" },
	{ SYSTEM_MOUNT_FAIL, "system part mount failed" },
	{ SECURITY_FAIL, "security failed" },
	{ CRITICAL_SERVICE_FAIL_TO_START, "critical service start failed" },
	{ DATA_MOUNT_FAILED_AND_ERASED, "data part mount failed" },
	{ DATA_MOUNT_RO, "data part mounted ro" },
	{ DATA_NOSPC, "no space on data part" },
	{ VENDOR_MOUNT_FAIL, "vendor part mount failed" },
	{ NATIVE_PANIC, "native critical fail" },
	{ MAPLE_ESCAPE_DLOPEN_MRT, "maple escape dlopen mrt fail" },
	{ MAPLE_ESCAPE_CRITICAL_CRASH, "maple escape critical crash fail" },
	{ MAPLE_CRITICAL_CRASH_MYGOTE_DATA,
	  "maple critical crash mygote data fail" },
	{ MAPLE_CRITICAL_CRASH_SYSTEMSERVER_DATA,
	  "maple critical crash systemserver data fail" },
	{ SYSTEM_SERVICE_LOAD_FAIL, "system service load failed" },
	{ PREBOOT_BROADCAST_FAIL, "preboot broadcast failed" },
	{ VM_OAT_FILE_DAMAGED, "ota file damaged" },
	{ PACKAGE_MANAGER_SETTING_FILE_DAMAGED,
	  "package manager setting file damaged" },
	{ BFM_HARDWARE_FAULT, "hardware-fault" },
	{ BOOTUP_SLOWLY, "bootup slowly" },
	{ POWEROFF_ABNORMAL, "power off abnormally" },
};

static enum bfm_errno_code sensible_bf_errno[] = {
	KERNEL_PRESS10S,
	KERNEL_BOOT_TIMEOUT,
	CRITICAL_SERVICE_FAIL_TO_START,
	POWEROFF_ABNORMAL,
	KERNEL_AP_WDT,
	KERNEL_LPM3_S_GLOBALWDT,
	KERNEL_LPM3_S_LPMCURST,
	BL2_WDT,
};

static char *valid_log_name[] = {
	BFM_BFI_FILE_NAME,
	BFM_RCV_FILE_NAME,
	BFM_CRITICAL_PROCESS_CRASH_LOG_NAME,
	BFM_TOMBSTONE_LOG_NAME,
	BFM_SYSTEM_SERVER_CRASH_LOG_NAME,
	BFM_SYSTEM_SERVER_WATCHDOG_LOG_NAME,
	BFM_BL1_LOG_FILENAME,
	BFM_BL2_LOG_FILENAME,
	BFM_KERNEL_LOG_FILENAME,
	BFM_ALT_BL1_LOG_FILENAME,
	BFM_ALT_BL2_LOG_FILENAME,
	BFM_ALT_KERNEL_LOG_FILENAME,
	BFM_FRAMEWORK_BOOTFAIL_LOG_FILE_NAME,
	BFM_PMSG_LOG_FILENAME,
};

static bool is_boot_success;
static bool is_process_boot_success;

static enum bfmr_selfheal_code bfm_get_selfheal_code(enum bfr_method rmethod,
		enum bfmr_detail_stage boot_stage)
{
	int i;
	int count = ARRAY_SIZE(rmethod_maptable);

	if (boot_stage == STAGE_BOOT_SUCCESS)
		boot_stage = ANDROID_FRAMEWORK_STAGE_START;

	for (i = 0; i < count; i++) {
		if (rmethod == rmethod_maptable[i].rmethod &&
		    ((unsigned int)boot_stage >> BFM_SELF_HEAL_STAGE_OFF_LEN) ==
			((unsigned int)rmethod_maptable[i].boot_stage >>
			  BFM_SELF_HEAL_STAGE_OFF_LEN))
			return rmethod_maptable[i].selfheal_code;
	}

	return SH_UNKNOW;
}

static void bfm_update_bf_logs_to_file(struct bfm_bf_log_info *bf_log,
		char *pdata, long pdata_len, long src_file_len)
{
	char *ptemp = NULL;
	unsigned long long end_time;
	char start_time[BFM_MAX_INT_NUMBER_LEN] = {0};
	char *temp_str = NULL;
	long buf_len = pdata_len;
	int str_len;
	int log_dir_cnt;

	log_dir_cnt = bf_log->log_dir_count;
	if (log_dir_cnt == 0) {
		BFMR_PRINT_ERR("log_dir_count=0!\n");
		return;
	}
	temp_str = bfmr_malloc(buf_len);
	if (temp_str == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc temp_str failed!\n");
		return;
	}
	memset(temp_str, 0, buf_len);

	end_time = bfm_get_system_time();
	snprintf(start_time, sizeof(start_time) - 1, "%u",
		 bf_log->real_rcv_info.boot_fail_rtc_time[log_dir_cnt - 1]);

	ptemp = bfmr_reverse_find_string(pdata, "RebootType");
	if (ptemp == NULL) {
		BFMR_PRINT_ERR("Can not find RebootType\n");

		snprintf(temp_str, BFM_SELF_HEAL_CHAIN_MAX - 1,
			 BFM_TEXT_LOG_SEPRATOR_ADD_TIME, start_time, end_time,
			 "");
		buf_len = strlen(temp_str);
		if (buf_len != 0)
			bfmr_save_log(bf_log->bootfail_logs[0].log_dir,
				      BFM_BFI_FILE_NAME, temp_str, buf_len, 1);
	} else {
		strncpy(temp_str, ptemp, buf_len);
		str_len = src_file_len - strlen(temp_str);
		if (str_len > 0)
			snprintf(ptemp, buf_len - str_len,
				 BFM_TEXT_LOG_SEPRATOR_ADD_TIME, start_time,
				 end_time, temp_str);
		bfmr_save_log(bf_log->bootfail_logs[0].log_dir,
			      BFM_BFI_FILE_NAME, pdata, buf_len, 0);
	}
	if (temp_str != NULL)
		bfmr_free(temp_str);
}

static void bfm_update_bootfail_logs(struct bfm_bf_log_info *bf_log)
{
	char *src_file_path = NULL;
	char *pdata = NULL;
	long src_file_len;
	long buf_len;

	if (unlikely(bf_log == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("bf_log\n");
		return;
	}
	if (bf_log->log_dir_count == 0) {
		BFMR_PRINT_ERR("log_dir_count=0!\n");
		return;
	}

	src_file_path = bfmr_malloc(BFMR_MAX_PATH);
	if (src_file_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}

	memset(src_file_path, 0, BFMR_MAX_PATH);
	snprintf(src_file_path, BFMR_MAX_PATH - 1, "%s/%s",
		 bf_log->bootfail_logs[0].log_dir, BFM_BFI_FILE_NAME);

	/* continue if the src file doesn't exist */
	if (!bfmr_is_file_existed(src_file_path))
		goto __out;

	/* get file length of src file */
	src_file_len = bfmr_get_file_length(src_file_path);
	if (src_file_len <= 0L) {
		BFMR_PRINT_ERR("the length of [%s] is :%ld\n", src_file_path,
				src_file_len);
		goto __out;
	}

	/* allocate mem */
	buf_len = src_file_len + BFM_SELF_HEAL_CHAIN_MAX + 1;
	pdata = bfmr_malloc(buf_len);
	if (pdata == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc pdata failed!\n");
		goto __out;
	}
	memset(pdata, 0, buf_len);

	/* read src file */
	src_file_len = bfmr_full_read_with_file_path(src_file_path, pdata,
						     src_file_len);
	if (src_file_len <= 0) {
		BFMR_PRINT_ERR("read [%s] failed!\n", src_file_path);
		goto __out;
	}

	bfm_update_bf_logs_to_file(bf_log, pdata, buf_len, src_file_len);

__out:
	if (pdata != NULL)
		bfmr_free(pdata);
	if (src_file_path != NULL)
		bfmr_free(src_file_path);
}

void bfm_send_signal_to_init(void)
{
	int pid = 1;
	int ret;
	struct siginfo info;
	struct task_struct *t = NULL;

	info.si_signo = SIG_TO_INIT;
	info.si_code = SI_QUEUE;
	info.si_int = SIG_INT_VALUE;

	rcu_read_lock();
	t = find_task_by_vpid(pid);
	if (t == NULL) {
		BFMR_PRINT_ERR("Init dump: no such pid\n");
		rcu_read_unlock();
	} else {
		rcu_read_unlock();
		ret = send_sig_info(SIG_TO_INIT, &info, t);
		if (ret < 0)
			BFMR_PRINT_ERR("Init dump: error sending signal\n");
		else
			BFMR_PRINT_ERR("Init dump: sending signal success\n");
	}
}

static char *bfm_get_bf_no_desc(enum bfm_errno_code bootfail_errno,
		struct bfm_param *pparam)
{
	int i;
	int count = ARRAY_SIZE(bf_errno_desc);

	for (i = 0; i < count; i++) {
		if (bootfail_errno == bf_errno_desc[i].bootfail_errno)
			return bf_errno_desc[i].desc;
	}

	return "unknown";
}

static int bfm_get_fs_state(const char *pmount_point, struct statfs *pstatfsbuf)
{
	mm_segment_t old_fs;
	int ret;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	memset((void *)pstatfsbuf, 0, sizeof(*pstatfsbuf));
	ret = sys_statfs(pmount_point, pstatfsbuf);
	set_fs(old_fs);

	return ret;
}

static int save_bf_info_to_dst(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam,
		char *log, size_t log_len)
{
	int ret;

	switch (pdst->type) {
	case DST_FILE:
		ret = bfmr_save_log(pparam->bootfail_log_dir, BFM_BFI_FILE_NAME,
				    (void *)log, log_len, 0);
		if (ret != 0)
			BFMR_PRINT_ERR("save [%s] failed!\n",
				       BFM_BFI_FILE_NAME);
		break;
	case DST_RAW_PART:
		bfmr_save_log_to_raw_part(pdst->dst_info.raw_part.raw_part_name,
					  pdst->dst_info.raw_part.offset,
					  (void *)log, log_len);
		psrc->log_type = LOG_TYPE_BFM_BFI_LOG;
		strncpy(psrc->src_log_file_path,
			BFM_BFI_FILE_NAME,
			BFMR_SIZE_1K - 1);
		bfmr_update_raw_log_info(psrc, pdst, log_len);
		ret = 0;
		break;
	case DST_MEMORY_BUFFER:
	default:
		bfmr_save_log_to_mem_buffer(pdst->dst_info.buffer.addr,
					    pdst->dst_info.buffer.len,
					    (void *)log, log_len);
		ret = 0;
		break;
	}
	return ret;
}

static size_t format_bf_info(struct bfm_param *pparam,char *out_data,
		int out_len)
{
	size_t bytes_formatted;
	int ret;
	char record_count_str[BFM_MAX_INT_NUMBER_LEN] = {'\0'};
	struct statfs statfsbuf = {0};

	ret = bfmr_wait_for_part_mount_with_timeout("/data",
		BFM_DATA_PART_TIME_OUT);
	if (ret == 0)
		ret = bfm_get_fs_state("/data", &statfsbuf);
	if (pparam->bootfail_log_info.log_dir_count == 0)
		snprintf(record_count_str,
			sizeof(record_count_str) - 1, "%s", "");
	else
		snprintf(record_count_str, sizeof(record_count_str) - 1,
			"_%d", pparam->bootfail_log_info.log_dir_count + 1);

	bytes_formatted = snprintf(out_data, out_len - 1,
		BFM_BFI_CONTENT_FORMAT,
		record_count_str,
		bfmr_convert_rtc_time_to_asctime(pparam->bootfail_time),
		bfm_get_bf_no_desc(pparam->bootfail_errno, pparam),
		bfm_get_platform_name(),
		bfm_get_boot_stage_name((unsigned int)pparam->boot_stage),
		pparam->is_system_rooted,
		pparam->is_user_sensible,
		(ret == 0) ? ((long long)(statfsbuf.f_bavail * statfsbuf.f_bsize) /
			     (long long)(BFMR_SIZE_1K * BFMR_SIZE_1K)) : (0L),
		(ret == 0) ? ((statfsbuf.f_blocks <= 0) ? (0LL) :
			     ((long long)(BFM_PERCENT * statfsbuf.f_bavail) /
			     (long long)statfsbuf.f_blocks)) : (0LL),
		(ret == 0) ? ((long long)(statfsbuf.f_files - statfsbuf.f_ffree)) : (0L),
		(ret == 0) ? ((statfsbuf.f_files <= 0) ?
			     (0LL) : ((long long)(BFM_PERCENT *
			     (statfsbuf.f_files - statfsbuf.f_ffree)) /
			     (long long)statfsbuf.f_files)) : (0LL),
		record_count_str, pparam->bootfail_time,
		(unsigned int)pparam->bootfail_errno,
		(unsigned int)pparam->boot_stage,
		pparam->is_system_rooted,
		pparam->is_user_sensible,
		pparam->dmd_num,
		pparam->excep_info,
		(unsigned int)pparam->bootup_time,
		pparam->is_bootup_successfully ? "yes" : "no",
		pparam->reboot_type,
		bfm_get_bootlock_value_from_cmdline());

	if ((bytes_formatted < (out_len - 1)) &&
	    ((pparam->bootfail_errno == BOOTUP_SLOWLY) ||
	    (pparam->bootfail_errno == KERNEL_BOOT_TIMEOUT))) {
		BFMR_PRINT_KEY_INFO("Record boot stages information\n");
		bytes_formatted += bfm_save_stages_info_txt(
					(char *)(out_data + bytes_formatted),
					(out_len - bytes_formatted));
	}

	return bytes_formatted;
}

static int bfm_save_bf_info_txt(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam)
{
	int ret;
	size_t bytes_formatted;
	char *pdata = NULL;

	pdata = (char *)bfmr_malloc(BFMR_TEMP_BUF_LEN);
	if (pdata == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		return -1;
	}
	memset(pdata, 0, BFMR_TEMP_BUF_LEN);
	bytes_formatted = format_bf_info(pparam, pdata, BFMR_TEMP_BUF_LEN);

	ret = save_bf_info_to_dst(pdst, psrc, pparam, pdata, bytes_formatted);

	bfmr_free(pdata);
	return ret;
}

static int bfm_get_rcv_result(
		enum bfr_suggested_method suggested_recovery_method)
{
	return (suggested_recovery_method == DO_NOTHING) ?
		(int)(BFM_RCV_SUCCESS_VALUE) : (int)(BFM_RCV_FAIL_VALUE);
}

static void save_rcv_info_to_dst(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc,
		struct bfm_param *pparam,
		char *pdata)
{
	int ret;

	switch (pdst->type) {
	case DST_FILE:
		ret = bfmr_save_log(pparam->bootfail_log_dir,
				    BFM_RCV_FILE_NAME, (void *)pdata,
				    strlen(pdata), 0);
		if (ret != 0)
			BFMR_PRINT_ERR("save [%s] fail\n", BFM_RCV_FILE_NAME);
		break;
	case DST_RAW_PART:
		bfmr_save_log_to_raw_part(pdst->dst_info.raw_part.raw_part_name,
					  pdst->dst_info.raw_part.offset,
					  (void *)pdata, strlen(pdata));
		psrc->log_type = LOG_TYPE_BFM_RECOVERY_LOG;
		strncpy(psrc->src_log_file_path, BFM_RCV_FILE_NAME,
			BFMR_SIZE_1K - 1);
		bfmr_update_raw_log_info(psrc, pdst, strlen(pdata));
		break;
	case DST_MEMORY_BUFFER:
	default:
		bfmr_save_log_to_mem_buffer(pdst->dst_info.buffer.addr,
					    pdst->dst_info.buffer.len,
					    (void *)pdata, strlen(pdata));
		break;
	}
}

static void format_rcv_info_for_save(struct bfm_param *pparam, char *pdata)
{
	int rcv_ret;
	enum bfmr_selfheal_code code;
	char record_count_str[BFM_MAX_INT_NUMBER_LEN] = {'\0'};

	if (pparam->suggested_recovery_method == DO_NOTHING)
		snprintf(record_count_str,
			 sizeof(record_count_str) - 1, "%s", "");
	else
		snprintf(record_count_str, sizeof(record_count_str) - 1,
			 "_%d", pparam->bootfail_log_info.log_dir_count + 1);

	code = bfm_get_selfheal_code(pparam->recovery_method,
					pparam->boot_stage);
	rcv_ret = bfm_get_rcv_result(pparam->suggested_recovery_method);
	(void)snprintf(pdata, BFMR_TEMP_BUF_LEN - 1,
			BFM_RCV_CONTENT_FORMAT,
			record_count_str,
			bfr_get_recovery_method_desc(pparam->recovery_method),
			record_count_str,
			(rcv_ret == BFM_RCV_SUCCESS_VALUE) ?
			BFM_RCV_SUCCESS_STR : BFM_RCV_FAIL_STR,
			record_count_str, code,
			record_count_str, rcv_ret,
			record_count_str, "");
}

static void bfm_save_rcv_info_txt(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam)
{
	char *pdata = NULL;

	pdata = (char *)bfmr_malloc(BFMR_TEMP_BUF_LEN);
	if (pdata == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		return;
	}
	memset(pdata, 0, BFMR_TEMP_BUF_LEN);

	format_rcv_info_for_save(pparam, pdata);

	save_rcv_info_to_dst(pdst, psrc, pparam, pdata);

	bfmr_free(pdata);
}

int bfm_get_log_count(char *bfmr_log_root_path)
{
	int num;
	mm_segment_t oldfs;
	int fd = -1;
	int log_count = 0;
	void *buf = NULL;
	char *full_path = NULL;
	struct linux_dirent64 *dirp = NULL;

	if (unlikely((bfmr_log_root_path == NULL))) {
		BFMR_PRINT_INVALID_PARAMS("bfmr_log_root_path\n");
		return 0;
	}

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	fd = sys_open(bfmr_log_root_path, O_RDONLY, 0);
	if (fd < 0) {
		BFMR_PRINT_ERR("open [%s] failed!\n", bfmr_log_root_path);
		goto __out;
	}

	buf = bfmr_malloc(BFMR_MAX_PATH);
	if (buf == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}

	full_path = bfmr_malloc(BFMR_MAX_PATH);
	if (full_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}

	dirp = buf;
	num = sys_getdents64(fd, dirp, BFMR_MAX_PATH);
	while (num > 0) {
		while (num > 0) {
			bfm_stat_t st;
			int ret;

			memset(full_path, 0, BFMR_MAX_PATH);
			snprintf(full_path, BFMR_MAX_PATH - 1, "%s/%s",
				 bfmr_log_root_path, dirp->d_name);
			if ((strcmp(dirp->d_name, ".") == 0) ||
			    (strcmp(dirp->d_name, "..") == 0)) {
				num -= dirp->d_reclen;
				dirp = (void *)dirp + dirp->d_reclen;
				continue;
			}

			memset((void *)&st, 0, sizeof(st));
			ret = bfm_sys_lstat(full_path, &st);
			if ((ret == 0) && (S_ISDIR(st.st_mode)) &&
			    (strcmp(dirp->d_name, BFM_UPLOADING_DIR_NAME) != 0))
				log_count++;

			num -= dirp->d_reclen;
			dirp = (void *)dirp + dirp->d_reclen;
		}

		dirp = buf;
		memset(buf, 0, BFMR_MAX_PATH);
		num = sys_getdents64(fd, dirp, BFMR_MAX_PATH);
	}

__out:
	if (fd >= 0) {
		sys_close(fd);
		fd = -1;
	}
	set_fs(oldfs);
	BFMR_PRINT_DBG("Log count: %d\n", log_count);
	bfmr_free(buf);
	bfmr_free(full_path);

	return log_count;
}

void bfm_delete_dir(char *log_path)
{
	int num;
	mm_segment_t oldfs;
	int fd = -1;
	void *buf = NULL;
	char *full_path = NULL;
	struct linux_dirent64 *dirp = NULL;

	oldfs = get_fs();
	set_fs(KERNEL_DS); /* lint !e501 */
	fd = sys_open(log_path, O_RDONLY, 0);
	if (fd < 0) {
		BFMR_PRINT_ERR("open [%s] failed![ret = %d]\n", log_path, fd);
		goto __out;
	}

	buf = bfmr_malloc(BFMR_MAX_PATH);
	if (buf == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}
	memset(buf, 0, BFMR_MAX_PATH);

	full_path = bfmr_malloc(BFMR_MAX_PATH);
	if (full_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}
	memset(full_path, 0, BFMR_MAX_PATH);

	dirp = buf;
	num = sys_getdents64(fd, dirp, BFMR_MAX_PATH);
	while (num > 0) {
		while (num > 0) {
			bfm_stat_t st;
			int ret;

			memset(full_path, 0, BFMR_MAX_PATH);
			memset((void *)&st, 0, sizeof(st));
			snprintf(full_path, BFMR_MAX_PATH - 1, "%s/%s",
				log_path, dirp->d_name);
			if ((strcmp(dirp->d_name, ".") == 0) ||
			    (strcmp(dirp->d_name, "..") == 0)) {
				num -= dirp->d_reclen;
				dirp = (void *)dirp + dirp->d_reclen;
				continue;
			}

			ret = bfm_sys_lstat(full_path, &st);
			if (ret == 0 && S_ISDIR(st.st_mode))
				sys_rmdir(full_path);
			else if (ret == 0)
				sys_unlink(full_path);
			num -= dirp->d_reclen;
			dirp = (void *)dirp + dirp->d_reclen;
		}
		dirp = buf;
		memset(buf, 0, BFMR_MAX_PATH);
		num = sys_getdents64(fd, dirp, BFMR_MAX_PATH);
	}

__out:
	if (fd >= 0) {
		sys_close(fd);
		fd = -1;
	}

	/* remove the log path */
	sys_rmdir(log_path);
	set_fs(oldfs);

	bfmr_free(buf);
	buf = NULL;
	bfmr_free(full_path);
	full_path = NULL;
}

static void bfm_delete_oldest_log(long long bytes_need_this_time)
{
	int log_count;
	char *log_path = NULL;
	long long available_space;
	bool should_delete_oldest_log = false;

	log_count = bfm_get_log_count(bfm_get_bfmr_log_root_path());
	BFMR_PRINT_KEY_INFO("There are %d logs in %s\n",
			    log_count, bfm_get_bfmr_log_root_path());
	if (log_count < BFM_LOG_MAX_COUNT) {
		available_space = bfmr_get_fs_available_space(
					bfm_get_bfmr_log_part_mount_point());
		if (available_space < bytes_need_this_time)
			should_delete_oldest_log = true;
	} else {
		should_delete_oldest_log = true;
	}

	if (!should_delete_oldest_log)
		goto __out;

	log_path = bfmr_malloc(BFMR_MAX_PATH);
	if (log_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}
	memset(log_path, 0, BFMR_MAX_PATH);

	(void)bfm_lookup_dir_by_create_time(bfm_get_bfmr_log_root_path(),
					    log_path, BFMR_MAX_PATH, 1);
	if (strlen(log_path) != 0)
		bfm_delete_dir(log_path);

__out:
	bfmr_free(log_path);
}

static void bfm_process_after_save_bootfail_log(void)
{
	BFMR_PRINT_KEY_INFO("restart system now!\n");
	kernel_restart(NULL);
}

static long long bfm_get_basic_space(enum bfm_errno_code bootfail_errno)
{
	int i;
	int count = ARRAY_SIZE(log_size_param);

	for (i = 0; i < count; i++) {
		if (bfmr_get_boot_stage_from_bootfail_errno(bootfail_errno) ==
		    log_size_param[i].boot_stage)
			return log_size_param[i].log_size_allowed;
	}

	return (long long)(BFM_BF_INFO_LOG_MAX + BFM_RCV_INFO_LOG_MAX);
}

static long long bfm_get_extra_space(struct bfm_param *pparam)
{
	long long bytes_need;

	if (strcmp(pparam->addl_info.log_path,
		BFM_FRAMEWORK_BOOTFAIL_LOG_PATH) != 0)
		bytes_need =
			(long long)(bfmr_get_file_length(BFM_LOGCAT_FILE_PATH) +
			bfmr_get_file_length(BFM_FRAMEWORK_BOOTFAIL_LOG_PATH) +
			bfmr_get_file_length(pparam->addl_info.log_path));
	else
		bytes_need =
			(long long)(bfmr_get_file_length(BFM_LOGCAT_FILE_PATH) +
			bfmr_get_file_length(pparam->addl_info.log_path));

	return bytes_need;
}

static int bfm_save_extra_bootfail_logs(struct bfmr_log_dst *pdst,
	struct bfmr_log_src *psrc,
	struct bfm_param *pparam)
{
	int ret;
	char src_path[BFMR_SIZE_256] = {0};

	/* 1. save logcat */
	memset((void *)pparam->bootfail_log_path, 0,
		sizeof(pparam->bootfail_log_path));
	(void)snprintf(pparam->bootfail_log_path,
			sizeof(pparam->bootfail_log_path) - 1,
			"%s/%s",
			pparam->bootfail_log_dir, BFM_LOGCAT_FILE_NAME);
	if (bfm_get_symbol_link_path(BFM_LOGCAT_FILE_PATH,
	    src_path, sizeof(src_path)))
		(void)snprintf(pparam->bootfail_log_path,
				sizeof(pparam->bootfail_log_path) - 1,
				"%s/%s.gz", pparam->bootfail_log_dir,
				BFM_LOGCAT_FILE_NAME_KEYWORD);
	psrc->log_type = LOG_TYPE_BETA_APP_LOGCAT;
	ret = bfmr_capture_and_save_log(psrc, pdst, pparam);

	/* 2. save framework bootfail log */
	if ((pparam->boot_stage >= ANDROID_FRAMEWORK_STAGE_START) &&
	    (strcmp(pparam->addl_info.log_path,
		    BFM_FRAMEWORK_BOOTFAIL_LOG_PATH) != 0)) {
		memset((void *)pparam->bootfail_log_path, 0,
			sizeof(pparam->bootfail_log_path));
		(void)snprintf(pparam->bootfail_log_path,
				sizeof(pparam->bootfail_log_path) - 1,
				"%s/%s", pparam->bootfail_log_dir,
				BFM_FRAMEWORK_BOOTFAIL_LOG_FILE_NAME);
		psrc->log_type = LOG_TYPE_FIXED_FRAMEWORK_BOOTFAIL_LOG;
		ret = bfmr_capture_and_save_log(psrc, pdst, pparam);
		bfmr_change_file_ownership(BFM_FRAMEWORK_BOOTFAIL_LOG_PATH,
					   pparam->user_space_log_uid,
					   pparam->user_space_log_gid);
	}

	return ret;
}

static int bfm_capture_and_save_bl1_bootfail_log(struct bfmr_log_dst *pdst,
	struct bfmr_log_src *psrc,
	struct bfm_param *pparam)
{
	memset((void *)pparam->bootfail_log_path, 0,
		sizeof(pparam->bootfail_log_path));
	(void)snprintf(pparam->bootfail_log_path,
		sizeof(pparam->bootfail_log_path) - 1,
		"%s/%s", pparam->bootfail_log_dir,
		bfm_get_bl1_bootfail_log_name());
	psrc->log_type = LOG_TYPE_BOOTLOADER_1;

	return bfmr_capture_and_save_log(psrc, pdst, pparam);
}

static int bfm_capture_and_save_bl2_bootfail_log(struct bfmr_log_dst *pdst,
	struct bfmr_log_src *psrc,
	struct bfm_param *pparam)
{
	memset((void *)pparam->bootfail_log_path, 0,
		sizeof(pparam->bootfail_log_path));
	(void)snprintf(pparam->bootfail_log_path,
		sizeof(pparam->bootfail_log_path) - 1,
		"%s/%s", pparam->bootfail_log_dir,
		bfm_get_bl2_bootfail_log_name());
	psrc->log_type = LOG_TYPE_BOOTLOADER_2;

	return bfmr_capture_and_save_log(psrc, pdst, pparam);
}

static int bfm_capture_and_save_kernel_bootfail_log(struct bfmr_log_dst *pdst,
	struct bfmr_log_src *psrc,
	struct bfm_param *pparam)
{
	bool file_existed = false;

	/* capture last kmsg of zip type on Beta version */
	memset((void *)pparam->bootfail_log_path, 0,
		sizeof(pparam->bootfail_log_path));
	file_existed = bfmr_is_file_existed(BFM_BETA_KMSG_LOG_PATH);
	BFMR_PRINT_KEY_INFO("%s %s!\n", BFM_BETA_KMSG_LOG_PATH,
			    (file_existed) ? ("exists") : ("doesn't exist"));
	if (bfm_is_beta_version() && file_existed) {
		(void)snprintf(pparam->bootfail_log_path,
				sizeof(pparam->bootfail_log_path) - 1,
				"%s/%s.gz", pparam->bootfail_log_dir,
				BFM_KERNEL_LOG_GZ_FILENAME_KEYWORD);
		psrc->log_type = LOG_TYPE_ANDROID_KMSG;
		(void)bfmr_capture_and_save_log(psrc, pdst, pparam);
	}

	/* capture last kmsg of text type */
	memset((void *)pparam->bootfail_log_path, 0,
		sizeof(pparam->bootfail_log_path));
	(void)snprintf(pparam->bootfail_log_path,
		sizeof(pparam->bootfail_log_path) - 1,
		"%s/%s", pparam->bootfail_log_dir,
		bfm_get_kernel_bootfail_log_name());
	psrc->log_type = LOG_TYPE_TEXT_KMSG;

	return bfmr_capture_and_save_log(psrc, pdst, pparam);
}

static int bfm_capture_and_save_ramoops_bootfail_log(struct bfmr_log_dst *pdst,
	struct bfmr_log_src *psrc,
	struct bfm_param *pparam)
{
	memset((void *)pparam->bootfail_log_path, 0,
		sizeof(pparam->bootfail_log_path));
	(void)snprintf(pparam->bootfail_log_path,
		sizeof(pparam->bootfail_log_path) - 1,
		"%s/%s", pparam->bootfail_log_dir,
		bfm_get_ramoops_bootfail_log_name());
	psrc->log_type = LOG_TYPE_RAMOOPS;

	return bfmr_capture_and_save_log(psrc, pdst, pparam);
}

static char *bfm_get_file_name(const char *file_full_path)
{
	char *ptemp = NULL;

	if (unlikely((file_full_path == NULL))) {
		BFMR_PRINT_INVALID_PARAMS("file_full_path\n");
		return NULL;
	}

	ptemp = strrchr(file_full_path, '/');
	if (ptemp == NULL)
		return (char *)file_full_path;

	return (ptemp + 1);
}

static int bfm_capture_and_save_native_bootfail_log(struct bfmr_log_dst *pdst,
	struct bfmr_log_src *psrc,
	struct bfm_param *pparam)
{
	int ret;

	if (strlen(pparam->addl_info.log_path) == 0) {
		BFMR_PRINT_KEY_INFO("user log path hasn't been set!\n");
		return -1;
	}
	memset((void *)pparam->bootfail_log_path, 0,
		sizeof(pparam->bootfail_log_path));
	if (strstr(pparam->addl_info.log_path,
	    BFM_TOMBSTONE_FILE_NAME_TAG) != NULL) {
		(void)snprintf(pparam->bootfail_log_path,
				sizeof(pparam->bootfail_log_path) - 1,
				"%s/%s", pparam->bootfail_log_dir,
				BFM_TOMBSTONE_LOG_NAME);
		psrc->log_type = LOG_TYPE_VM_TOMBSTONES;
	} else if (strstr(pparam->addl_info.log_path,
		   BFM_SYSTEM_SERVER_CRASH_FILE_NAME_TAG) != NULL) {
		(void)snprintf(pparam->bootfail_log_path,
				sizeof(pparam->bootfail_log_path) - 1,
				"%s/%s", pparam->bootfail_log_dir,
				BFM_SYSTEM_SERVER_CRASH_LOG_NAME);
		psrc->log_type = LOG_TYPE_VM_CRASH;
	} else if (strstr(pparam->addl_info.log_path,
			  BFM_SYSTEM_SERVER_WATCHDOG_FILE_NAME_TAG) != NULL) {
		(void)snprintf(pparam->bootfail_log_path,
				sizeof(pparam->bootfail_log_path) - 1,
				"%s/%s", pparam->bootfail_log_dir,
				BFM_SYSTEM_SERVER_WATCHDOG_LOG_NAME);
		psrc->log_type = LOG_TYPE_VM_WATCHDOG;
	} else if (strstr(pparam->addl_info.log_path,
		   BFM_CRITICAL_PROCESS_CRASH_LOG_NAME) != NULL) {
		(void)snprintf(pparam->bootfail_log_path,
				sizeof(pparam->bootfail_log_path) - 1,
				"%s/%s", pparam->bootfail_log_dir,
				BFM_CRITICAL_PROCESS_CRASH_LOG_NAME);
		psrc->log_type = LOG_TYPE_CRITICAL_PROCESS_CRASH;
	} else {
		BFMR_PRINT_KEY_INFO("Invalid user bootfail log!\n");
		return -1;
	}

	ret = bfmr_capture_and_save_log(psrc, pdst, pparam);
	bfmr_change_file_ownership(pparam->addl_info.log_path,
				   pparam->user_space_log_uid,
				   pparam->user_space_log_gid);

	/* remove critical process crash log in /cache */
	if (strstr(pparam->addl_info.log_path,
		   BFM_CRITICAL_PROCESS_CRASH_LOG_NAME) != NULL)
		bfmr_unlink_file(pparam->addl_info.log_path);

	return ret;
}

static int bfm_capture_and_save_framework_bootfail_log(struct bfmr_log_dst *pdst,
		struct bfmr_log_src *psrc, struct bfm_param *pparam)
{
	int ret;

	if (strlen(pparam->addl_info.log_path) == 0) {
		BFMR_PRINT_KEY_INFO("user log path hasn't been set!\n");
		return -1;
	}

	memset((void *)pparam->bootfail_log_path, 0,
		sizeof(pparam->bootfail_log_path));
	(void)snprintf(pparam->bootfail_log_path,
		       sizeof(pparam->bootfail_log_path) - 1,
		       "%s/%s", pparam->bootfail_log_dir,
		       bfm_get_file_name(pparam->addl_info.log_path));
	psrc->log_type = LOG_TYPE_NORMAL_FRAMEWORK_BOOTFAIL_LOG;
	ret = bfmr_capture_and_save_log(psrc, pdst, pparam);
	bfmr_change_file_ownership(pparam->addl_info.log_path,
				   pparam->user_space_log_uid,
				   pparam->user_space_log_gid);

	return ret;
}

static bool bfm_is_log_existed(unsigned long long rtc_time,
	unsigned int bootfail_errno)
{
	char *log_full_path = NULL;
	bool ret = false;

	log_full_path = bfmr_malloc(BFMR_MAX_PATH);
	if (log_full_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}
	memset((void *)log_full_path, 0, BFMR_MAX_PATH);

	snprintf(log_full_path, BFMR_MAX_PATH - 1, "%s/"
		 BFM_BOOTFAIL_LOG_DIR_NAME_FORMAT, bfm_get_bfmr_log_root_path(),
		 bfmr_convert_rtc_time_to_asctime(rtc_time), bootfail_errno);
	ret = bfmr_is_dir_existed(log_full_path);
	BFMR_PRINT_KEY_INFO("%s %s\n", log_full_path, (ret) ? ("exists!") :
			    ("does't exist!"));

__out:
	bfmr_free(log_full_path);
	log_full_path = NULL;
	return ret;
}

static int prepare_for_save_log(struct bfmr_log_dst *dst,
		struct bfmr_log_src *src, struct bfm_param *pparam)
{
	dst->type = pparam->dst_type;
	src->log_save_additional_context = (void *)pparam;
	switch (dst->type) {
	case DST_FILE:
		dst->dst_info.filename = pparam->bootfail_log_path;
		src->src_log_file_path = pparam->addl_info.log_path;
		src->log_save_context = pparam->log_save_context;
		if (!bfmr_is_part_mounted_rw(bfm_get_bfmr_log_part_mount_point())) {
			BFMR_PRINT_ERR("log part hasn't mounted to: %s\n",
					bfm_get_bfmr_log_part_mount_point());
			return -1;
		}

		/* create log root path */
		(void)bfmr_create_log_path(bfm_get_bfmr_log_root_path());

		/* create uploading path */
		(void)bfmr_create_log_path(bfm_get_bfmr_log_uploading_path());

		/* check if the log is existed or not */
		if (bfm_is_log_existed(pparam->bootfail_time,
				       (unsigned int)pparam->bootfail_errno))
			return 1;
		/* delete oldest log */
		if (pparam->save_bottom_layer_bootfail_log == 0)
			bfm_delete_oldest_log(
				bfm_get_basic_space(pparam->bootfail_errno) +
				bfm_get_extra_space(pparam));
		else
			bfm_delete_oldest_log(
				bfm_get_basic_space(pparam->bootfail_errno));

		/* traverse all the bootfail logs */
		bfm_traverse_log_root_dir(&(pparam->bootfail_log_info));

		/* create boot fail log dir */
		(void)snprintf(pparam->bootfail_log_dir,
				sizeof(pparam->bootfail_log_dir) - 1,
				"%s/" BFM_BOOTFAIL_LOG_DIR_NAME_FORMAT,
				bfm_get_bfmr_log_root_path(),
				bfmr_convert_rtc_time_to_asctime(pparam->bootfail_time),
				(unsigned int)pparam->bootfail_errno);
		bfmr_create_log_path(pparam->bootfail_log_dir);

		break;
	case DST_RAW_PART:
		dst->dst_info.raw_part.raw_part_name = bfm_get_raw_part_name();
		dst->dst_info.raw_part.offset += bfm_get_raw_part_offset();
		src->src_log_file_path = pparam->addl_info.log_path;
		(void)snprintf(pparam->bootfail_log_dir,
				sizeof(pparam->bootfail_log_dir) - 1,
				"%s/" BFM_BOOTFAIL_LOG_DIR_NAME_FORMAT,
				bfm_get_bfmr_log_root_path(),
				bfmr_convert_rtc_time_to_asctime(pparam->bootfail_time),
				(unsigned int)pparam->bootfail_errno);
		bfmr_alloc_and_init_raw_log_info(pparam, dst);
		src->log_save_context = pparam->log_save_context;
		break;
	case DST_MEMORY_BUFFER:
		break;
	default:
		BFMR_PRINT_ERR("Invalid dst type: %d\n", dst->type);
		return -1;
	}
	return 0;
}

static void bfm_save_bootfail_runtime_log(struct bfmr_log_dst *dst,
		struct bfmr_log_src *src, struct bfm_param *pparam)
{
	switch (bfmr_get_boot_stage_from_bootfail_errno(pparam->bootfail_errno)) {
	case BL1_STAGE:
		BFMR_PRINT_KEY_INFO("Boot fail @ BL1, bootfail_errno: 0x%x\n",
				    (unsigned int)pparam->bootfail_errno);
		bfm_capture_and_save_bl1_bootfail_log(dst, src, pparam);
		break;
	case BL2_STAGE:
		BFMR_PRINT_KEY_INFO("Boot fail @ BL2, bootfail_errno: 0x%x\n",
				    (unsigned int)pparam->bootfail_errno);
		bfm_capture_and_save_bl1_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_bl2_bootfail_log(dst, src, pparam);
		break;
	case KERNEL_STAGE:
		BFMR_PRINT_KEY_INFO("Boot fail @ Kernel, bootfail_errno: 0x%x\n",
				    (unsigned int)pparam->bootfail_errno);
		bfm_capture_and_save_bl1_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_bl2_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_kernel_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_ramoops_bootfail_log(dst, src, pparam);
		break;
	case NATIVE_STAGE:
		BFMR_PRINT_KEY_INFO("Boot fail @ Native, bootfail_errno: 0x%x\n",
				    (unsigned int)pparam->bootfail_errno);
		bfm_capture_and_save_bl1_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_bl2_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_kernel_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_ramoops_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_native_bootfail_log(dst, src, pparam);
		break;
	case ANDROID_FRAMEWORK_STAGE:
		BFMR_PRINT_KEY_INFO("Boot fail @ Framework, bootfail_errno: 0x%x\n",
				    (unsigned int)pparam->bootfail_errno);
		bfm_capture_and_save_bl1_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_bl2_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_kernel_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_ramoops_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_native_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_framework_bootfail_log(dst, src, pparam);
		break;
	default:
		BFMR_PRINT_KEY_INFO("Boot fail @ Unknown stage, bootfail_errno: 0x%x\n",
				    (unsigned int)pparam->bootfail_errno);
		bfm_capture_and_save_bl1_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_bl2_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_kernel_bootfail_log(dst, src, pparam);
		bfm_capture_and_save_ramoops_bootfail_log(dst, src, pparam);
		break;
	}

	/* save extra logs such as: logcat */
	if (pparam->save_bottom_layer_bootfail_log == 0)
		bfm_save_extra_bootfail_logs(dst, src, pparam);

	if (dst->type == DST_RAW_PART)
		bfmr_save_and_free_raw_log_info(pparam);
}

static int bfm_catch_save_bf_log(struct bfm_param *pparam)
{
	int ret;
	struct bfmr_log_dst dst;
	struct bfmr_log_src src;

	if (unlikely(pparam == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("pparam\n");
		return -1;
	}

	memset((void *)&dst, 0, sizeof(dst));
	memset((void *)&src, 0, sizeof(src));

	ret = prepare_for_save_log(&dst, &src, pparam);
	if (ret == 1) { /* the log is existed */
		ret = 0;
		return ret;
	}
	if (ret != 0)
		return ret;

	/* save bootFail_info.txt */
	bfm_save_bf_info_txt(&dst, &src, pparam);

	/* save recovery_info.txt */
	bfm_save_rcv_info_txt(&dst, &src, pparam);

	bfm_save_bootfail_runtime_log(&dst, &src, pparam);
	return ret;
}

static int bfm_process_upper_layer_boot_fail(void *param)
{
	struct bfm_param *pparam = param;

	if ((strlen(pparam->addl_info.log_path) != 0) &&
	    (pparam->user_space_log_read_len <= 0)) {
		uid_t uid = 0;
		gid_t gid = 0;

		/* if the user space log's original ownership has not been */
		/* gotten correctly, we can not change its onwership here */
		if ((bfmr_get_uid_gid(&uid, &gid) == 0) &&
		    ((int)pparam->user_space_log_uid != -1) &&
		    ((int)pparam->user_space_log_gid != -1))
			bfmr_change_file_ownership(pparam->addl_info.log_path,
						   uid, gid);
	}

	/* 1. this case means the boot fail has been resolved */
	if (pparam->suggested_recovery_method == DO_NOTHING) {
		BFMR_PRINT_KEY_INFO("suggested recovery method is: \"DO_NOTHING\"\n");
		(void)bfm_capture_and_save_do_nothing_bootfail_log(pparam);
		goto __out;
	}

	/* 2. process boot fail furtherly in platform */
	(void)bfm_platform_process_boot_fail(pparam);

	if (pparam->bootfail_can_only_be_processed_in_platform == 0) {
		/* 3. capture and save log for most boot fail errno */
		bfm_catch_save_bf_log(pparam);

		/* 4. post process after save bootfail log */
		bfm_process_after_save_bootfail_log();
	}

__out:

	if (pparam != NULL) {
		if (pparam->user_space_log_buf != NULL)
			bfmr_free(pparam->user_space_log_buf);
		bfmr_free(param);
	}
	msleep(BFM_SAVE_LOG_INTERVAL);
	complete(&process_boot_fail_comp);

	return 0;
}

static void bfm_wait_for_compeletion_of_processing_boot_fail(void)
{
	while (1)
		msleep_interruptible(BFM_BLOCK_CALLING_PROCESS_INTERVAL);
}

static void bfm_user_space_process_read_its_own_file(struct bfm_param *pparam)
{
	pparam->user_space_log_len = bfmr_get_file_length(
					pparam->addl_info.log_path);
	if (pparam->user_space_log_len > 0) {
		pparam->user_space_log_buf = bfmr_malloc(
					pparam->user_space_log_len + 1);
		if (pparam->user_space_log_buf != NULL) {
			memset(pparam->user_space_log_buf, 0,
			       pparam->user_space_log_len + 1);
			pparam->user_space_log_read_len =
				bfmr_full_read_with_file_path(
					pparam->addl_info.log_path,
					pparam->user_space_log_buf,
					pparam->user_space_log_len);
			BFMR_PRINT_KEY_INFO("Read file %s :%ld Bytes successsfully, its length is %ld Bytes!\n",
				pparam->addl_info.log_path,
				pparam->user_space_log_read_len,
				pparam->user_space_log_len);
		}
	}
}

static enum bfmr_detail_stage bfm_correct_boot_stage(
		enum bfmr_detail_stage boot_stage,
		enum bfm_errno_code bootfail_errno)
{
	unsigned int stage1 = bfmr_get_boot_stage_from_detail_stage(boot_stage);
	unsigned int stage2 = bfmr_get_boot_stage_from_bootfail_errno(bootfail_errno);

	if (stage1 < stage2)
		return ((stage2 << BFM_BOOT_STAGE_SHIFTING) + 1);

	return boot_stage;
}

/*
 * @brief: save the log and do proper recovery actions
 *         when meet with error during system booting process.
 * @param: bootfail_errno [in], boot fail error no.
 * @param: suggested_recovery_method [in], suggested recovery method,
 *         if you don't know, please transfer NO_SUGGESTION for it
 * @param: paddl_info [in], saving additional info such as log path and so on.
 * @return: 0 - succeeded; -1 - failed.
 */
int boot_fail_err(enum bfm_errno_code bootfail_errno,
	enum bfr_suggested_method suggested_recovery_method,
	struct bfmr_bf_addl_info *paddl_info)
{
	enum bfmr_detail_stage boot_stage;
	struct bfm_param *pparam = NULL;
	static bool s_is_comp_init;

	BFMR_PRINT_ENTER();
	if (!bfmr_has_been_enabled()) {
		BFMR_PRINT_KEY_INFO("BFMR is disabled!\n");
		return 0;
	}

	mutex_lock(&process_boot_fail_mutex);
	if (!s_is_comp_init) {
		complete(&process_boot_fail_comp);
		s_is_comp_init = true;
	}

	bfmr_get_boot_stage(&boot_stage);
	if (bfmr_is_boot_success(boot_stage)) {
		if ((bootfail_errno != CRITICAL_SERVICE_FAIL_TO_START) &&
		    (bootfail_errno != BOOTUP_SLOWLY) &&
		    (!(bootfail_errno >= MAPLE_ESCAPE_DLOPEN_MRT &&
		     bootfail_errno <= MAPLE_CRITICAL_CRASH_SYSTEMSERVER_DATA)) &&
		    (bootfail_errno != BFM_HARDWARE_FAULT)) {
			BFMR_PRINT_ERR("Error: can't set errno %x after device boot success!\n",
					(unsigned int)bootfail_errno);
			goto __out;
		}

		BFMR_PRINT_KEY_INFO("%s",
			(bootfail_errno == CRITICAL_SERVICE_FAIL_TO_START) ?
			("critical process work abnormally after boot success") :
			("bootup slowly!\n"));
	}

	if (!bfr_has_been_enabled()) {
		BFMR_PRINT_ERR("BFR has been disabled, so set suggested_recovery_method = DO_NOTHING!\n");
		suggested_recovery_method = DO_NOTHING;
	}

	if (suggested_recovery_method == DO_NOTHING) {
		if (wait_for_completion_timeout(&process_boot_fail_comp,
		    msecs_to_jiffies(BFM_SAVE_LOG_MAX_TIME)) == 0) {
			BFMR_PRINT_KEY_INFO("last boot_err is in processing, this error skip for DO_NOTHING!\n");
			goto __out;
		}
	} else {
		wait_for_completion(&process_boot_fail_comp);
	}

	pparam = bfmr_malloc(sizeof(*pparam));
	if (pparam == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}
	memset((void *)pparam, 0, sizeof(*pparam));

	pparam->bootfail_errno = bootfail_errno;
	pparam->boot_stage = bfm_correct_boot_stage(boot_stage, bootfail_errno);
	pparam->is_user_sensible = (suggested_recovery_method != DO_NOTHING) ?
		(1) : (0);
	pparam->bootfail_errno = bootfail_errno;
	pparam->suggested_recovery_method = suggested_recovery_method;
	pparam->user_space_log_uid = (uid_t)-1;
	pparam->user_space_log_gid = (gid_t)-1;
	pparam->is_bootup_successfully = is_boot_success;
	if (paddl_info != NULL) {
		memcpy(&pparam->addl_info, paddl_info,
		sizeof(pparam->addl_info));
		(void)bfmr_get_file_ownership(pparam->addl_info.log_path,
					      &pparam->user_space_log_uid,
					      &pparam->user_space_log_gid);
		bfm_user_space_process_read_its_own_file(pparam);
	}
	pparam->bootfail_time = bfm_get_system_time();
	pparam->bootup_time = bfmr_get_bootup_time();
	pparam->is_user_sensible =
		bfm_is_user_sensible_bootfail(pparam->bootfail_errno,
					pparam->suggested_recovery_method),
	pparam->is_system_rooted = bfm_is_system_rooted();
	pparam->bootfail_can_only_be_processed_in_platform = 0;
	pparam->catch_save_bf_log = bfm_catch_save_bf_log;
	kthread_run(bfm_process_upper_layer_boot_fail, (void *)pparam,
		    "bfm_process_upper_layer_boot_fail");
	if (!bfr_has_been_enabled()) {
		wait_for_completion(&process_boot_fail_comp);
		complete(&process_boot_fail_comp);
	}

	if (suggested_recovery_method != DO_NOTHING)
		bfm_wait_for_compeletion_of_processing_boot_fail();
__out:
	mutex_unlock(&process_boot_fail_mutex);
	BFMR_PRINT_EXIT();

	return 0; /*lint !e429 */
}

/*
 * @function: int bfmr_set_boot_stage(enum bfmr_detail_stage boot_stage)
 * @brief: get current boot stage during boot process.
 * @param: boot_stage [in], boot stage to be set.
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_set_boot_stage(enum bfmr_detail_stage boot_stage)
{
	mutex_lock(&process_boot_stage_mutex);
	(void)bfm_set_boot_stage(boot_stage);

	// record the start time of boot stage, only before bootup successful
	if (is_boot_success == false) {
		struct bfm_stage_info stage_info = {0};

		stage_info.stage = boot_stage;
		stage_info.start_time = bfmr_get_bootup_time();

		bfm_add_stage_info(&stage_info);
	}

	if (bfmr_is_boot_success(boot_stage)) {
		BFMR_PRINT_KEY_INFO("boot success!\n");
		bfm_stop_boot_timer();
		kthread_run(bfm_notify_boot_success, NULL,
			    "bfm_notify_boot_success");
	}
	mutex_unlock(&process_boot_stage_mutex);

	return 0;
}

/*
 * @function: int bfmr_get_boot_stage(enum bfmr_detail_stage *pboot_stage)
 * @brief: get current boot stage during boot process.
 * @param: pboot_stage [out], buffer storing the boot stage.
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_get_boot_stage(enum bfmr_detail_stage *pboot_stage)
{
	int ret;
	enum bfmr_detail_stage boot_stage;

	mutex_lock(&process_boot_stage_mutex);
	ret = bfm_get_boot_stage(&boot_stage);
	*pboot_stage = boot_stage;
	mutex_unlock(&process_boot_stage_mutex);

	return ret;
}

static int bfm_update_bf_info_for_each_log(struct bfm_bf_log_info *bf_Log)
{
	bfm_update_bootfail_logs(bf_Log);
	return 0;
}

static void format_rcv_info_for_update(char *pdata,
		struct bfr_real_rcv_info *rinfo, int record_count)
{
	int i;
	char count_str[BFM_MAX_INT_NUMBER_LEN] = {'\0'};
	char time[BFM_MAX_INT_NUMBER_LEN] = {'\0'};
	int bytes_formatted = 0;
	enum bfmr_selfheal_code code;
	char chain[BFM_SELF_HEAL_CHAIN_MAX] = {'\0'};
	int chain_len = 0;

	for (i = 0; i < record_count; i++) {
		bool success = false;

		code = bfm_get_selfheal_code(rinfo->recovery_method[i],
						rinfo->boot_fail_stage[i]);
		chain_len += snprintf(chain + chain_len,
					BFM_SELF_HEAL_CHAIN_MAX - chain_len - 1,
					"%d, ", code);

		if (record_count == (i + 1) || bfmr_is_boot_success(rinfo->boot_fail_stage[i]))
			success = true;
		if (record_count == (i + 1))
			snprintf(count_str, sizeof(count_str) - 1, "%s", "");
		else
			snprintf(count_str, sizeof(count_str) - 1, "_%d", (i + 1));

		snprintf(time, sizeof(time) - 1, "0x%08x",
			 rinfo->boot_fail_rtc_time[i]);
		bytes_formatted += snprintf(pdata + bytes_formatted,
			BFMR_SIZE_4K - bytes_formatted,
			(record_count == 1) ?
				("%s" BFM_RCV_CONTENT_FORMAT) :
				(BFM_LOG_SEPRATOR BFM_RCV_CONTENT_FORMAT),
			(record_count == 1) ? ("") : (time),
			count_str,
			bfr_get_recovery_method_desc(rinfo->recovery_method[i]),
			count_str,
			(success ? BFM_RCV_SUCCESS_STR : BFM_RCV_FAIL_STR),
			count_str, code,
			count_str,
			(success ? (BFM_RCV_SUCCESS_VALUE) : (BFM_RCV_FAIL_VALUE)),
			count_str,
			(success ? chain : ""));
	}
}

static int bfm_update_rcv_info_for_each_log(struct bfm_bf_log_info *bf_log)
{
	char *pdata = NULL;
	char c = '\0';

	pdata = bfmr_malloc(BFMR_SIZE_4K + sizeof(char));
	if (pdata == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		return -1;
	}
	memset(pdata, 0, BFMR_SIZE_4K + sizeof(char));

	/* format the recovery info */
	format_rcv_info_for_update(pdata, &(bf_log->real_rcv_info),
				   bf_log->real_rcv_info.record_count);

	/* save recovery info */
	if (bfmr_save_log(bf_log->bootfail_logs[0].log_dir, BFM_RCV_FILE_NAME,
			  (void *)pdata, strlen(pdata), 0) != 0)
		BFMR_PRINT_ERR("Failed to update recovery_info.txt!\n");

	/* Create DONE file for each file */
	bfmr_save_log(bf_log->bootfail_logs[0].log_dir, BFM_DONE_FILE_NAME,
		      (void *)&c, sizeof(char), 0);

	bfmr_free(pdata);

	return 0;
}

static bool is_user_sensible_rmethod(struct bfm_bf_log_info *bf_log)
{
	int i;
	struct bfr_real_rcv_info *rinfo = &(bf_log->real_rcv_info);

	for (i = 0; i < rinfo->record_count; i++) {
		if (rinfo->recovery_method_original[i] != FRM_REBOOT) {
			BFMR_PRINT_KEY_INFO("The original recovery method of bootfail [%x] is: %d, not \"FRM_REBOOT\"!, it is user sensible\n",
					    rinfo->boot_fail_no[i],
					    rinfo->recovery_method_original[i]);
			return true;
		}
	}
	return false;
}

static bool is_user_sensible_boot_fail_no(struct bfm_bf_log_info *bf_log)
{
	int i, j;
	int count = ARRAY_SIZE(sensible_bf_errno);
	struct bfr_real_rcv_info *rinfo = &(bf_log->real_rcv_info);

	for (i = 0; i < count; i++) {
		for (j = 0; j < rinfo->record_count; j++) {
			if ((unsigned int)sensible_bf_errno[i] == rinfo->boot_fail_no[j]) {
				BFMR_PRINT_KEY_INFO("The bootfail [%x] is user sensible!\n",
						    rinfo->boot_fail_no[j]);
				return true;
			}
		}
	}
	return false;
}

static bool is_user_sensible_boot_fail_stage(struct bfm_bf_log_info *bf_log)
{
	int i;
	struct bfr_real_rcv_info *rinfo = &(bf_log->real_rcv_info);

	for (i = 0; i < rinfo->record_count; i++) {
		if (rinfo->boot_fail_stage[i] >= NATIVE_STAGE_START) {
			BFMR_PRINT_KEY_INFO("The bootfail [%x] occurs @%x stage, it is user sensible!\n",
					    rinfo->boot_fail_no[i],
					    rinfo->boot_fail_stage[i]);
			return true;
		}
	}
	return false;
}

static bool is_user_sensible_boot_fail_time(struct bfm_bf_log_info *bf_log)
{
	int i;
	struct bfr_real_rcv_info *rinfo = &(bf_log->real_rcv_info);

	for (i = 0; i < rinfo->record_count; i++) {
		if (rinfo->boot_fail_time[i] > BFM_USER_MAX_TOLERANT_BOOTTIME) {
			BFMR_PRINT_KEY_INFO("The bootfail [%x] occurs @%u second, it is user sensible!\n",
					    rinfo->boot_fail_no[i],
					    rinfo->boot_fail_time[i]);
			return true;
		}
	}
	return false;
}

static bool bfm_is_user_sensible(struct bfm_bf_log_info *bf_log)
{
	if ((bf_log->real_rcv_info.record_count == 0) &&
	    (bf_log->log_dir_count > 0)) {
		/* Maybe DO_NOTHING log */
		return true;
	}

	/* 2, if bootfail times > 2, it must be a user sensible bootfail */
	if (bf_log->real_rcv_info.record_count > BFM_NOT_SENSIBLE_MAX_CNT)
		return true;

	/* 3, if the recovery method of bootfail is not REBOOT, it must be user sensible */
	if (is_user_sensible_rmethod(bf_log))
		return true;

	/* 4, some time even if the recovery method is FRM_REBOOT, it is also user sensible such as: PRESS10S */
	if (is_user_sensible_boot_fail_no(bf_log))
		return true;

	/* 5, if the bootfail occurs @ NATIVE/FRAMEWORK stage, it maybe a user sensible bootfail with high probability */
	if (is_user_sensible_boot_fail_stage(bf_log))
		return true;

	/* 6, if the bootfail occurs @ NATIVE/FRAMEWORK stage, it maybe a user sensible bootfail with high probability */
	if (is_user_sensible_boot_fail_time(bf_log))
		return true;

	return false;
}

static void bfm_merge_one_bootfail_logs(struct bfm_bf_log_info *bf_log,
		int log_dir_idx, int log_name_idx)
{
	char *pdata = NULL;
	char *pstart = NULL;
	long offset;
	long src_file_len;
	long buf_len;
	char time[BFM_MAX_INT_NUMBER_LEN] = {0};
	char *src_file_path = NULL;

	src_file_path = bfmr_malloc(BFMR_MAX_PATH);
	if (src_file_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}

	memset(src_file_path, 0, BFMR_MAX_PATH);
	snprintf(src_file_path, BFMR_MAX_PATH - 1, "%s/%s",
		 bf_log->bootfail_logs[log_dir_idx].log_dir,
		 valid_log_name[log_name_idx]);

	/* continue if the src file doesn't exist */
	if (!bfmr_is_file_existed(src_file_path))
		goto __out;

	/* get file length of src file */
	src_file_len = bfmr_get_file_length(src_file_path);
	if (src_file_len <= 0L) {
		BFMR_PRINT_ERR("the length of [%s] is :%ld\n", src_file_path,
				src_file_len);
		goto __out;
	}

	/* allocate mem */
	buf_len = strlen(BFM_LOG_SEPRATOR_NEWLINE) +
			 src_file_len + sizeof(time) + 1;
	pdata = (char *)bfmr_malloc(buf_len);
	if (pdata == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}
	memset(pdata, 0, buf_len);

	/* read src file */
	snprintf(time, sizeof(time) - 1, "0x%08x",
		 bf_log->real_rcv_info.boot_fail_rtc_time[log_dir_idx]);
	offset = snprintf(pdata, buf_len,
			  BFM_LOG_SEPRATOR_NEWLINE, time);
	src_file_len = bfmr_full_read_with_file_path(src_file_path,
						     pdata + offset,
						     src_file_len);
	if (src_file_len <= 0) {
		BFMR_PRINT_ERR("read [%s] failed!\n", src_file_path);
		bfmr_free(pdata);
		goto __out;
	}
	pstart = (((pdata[offset + src_file_len - 1] == '\n') ||
		  (log_dir_idx == 0)) ? (pdata + 1) : (pdata));
	buf_len = (((pdata[offset + src_file_len - 1] == '\n') ||
		   (log_dir_idx == 0)) ?
		   (offset + src_file_len - 1) : (offset + src_file_len));
	bfmr_save_log(bf_log->bootfail_logs[0].log_dir,
		      valid_log_name[log_name_idx], pstart, buf_len,
		      (log_dir_idx == 0) ? (0) : 1);
	bfmr_free(pdata);

__out:
	bfmr_free(src_file_path);
}

static void bfm_merge_bootfail_logs(struct bfm_bf_log_info *bf_log)
{
	int i, j;
	int log_count = ARRAY_SIZE(valid_log_name);

	if (bf_log->log_dir_count <= 1)
		return;

	for (i = 0; i < bf_log->log_dir_count; i++) {
		for (j = 0; j < log_count; j++)
			bfm_merge_one_bootfail_logs(bf_log, i, j);

		if (i > 0)
			bfm_delete_dir(bf_log->bootfail_logs[i].log_dir);
	}
}

static void bfm_delete_user_unsensible_logs(struct bfm_bf_log_info *bf_log)
{
	int i;

	for (i = 0; i < bf_log->log_dir_count; i++)
		bfm_delete_dir(bf_log->bootfail_logs[i].log_dir);
}

static int bfm_traverse_log_root_dir(struct bfm_bf_log_info *bf_log)
{
	int i;
	int fd = -1;
	int num;
	size_t log_idx = 0;
	size_t log_max_count;
	void *buf = NULL;
	char *full_path = NULL;
	struct linux_dirent64 *dirp = NULL;
	mm_segment_t oldfs;

	if (unlikely(bf_log == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("bf_log\n");
		return -1;
	}

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	fd = sys_open(bfm_get_bfmr_log_root_path(), O_RDONLY, 0);
	if (fd < 0) {
		BFMR_PRINT_ERR("open %s failed!\n",
			bfm_get_bfmr_log_root_path());
		goto __out;
	}

	buf = bfmr_malloc(BFMR_MAX_PATH);
	if (buf == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}

	full_path = bfmr_malloc(BFMR_MAX_PATH);
	if (full_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}

	log_max_count = ARRAY_SIZE(bf_log->bootfail_logs);
	dirp = buf;
	num = sys_getdents64(fd, dirp, BFMR_MAX_PATH);
	while (num > 0) {
		while ((num > 0) && (log_idx < log_max_count)) {
			bfm_stat_t st;
			int ret;
			struct bfm_per_bf_log_info *log_info =
				&(bf_log->bootfail_logs[log_idx]);

			if ((strcmp(dirp->d_name, ".") == 0) ||
			    (strcmp(dirp->d_name, "..") == 0) ||
			    (strcmp(dirp->d_name, BFM_UPLOADING_DIR_NAME) == 0))
				goto __continue;

			memset(log_info->log_dir, 0, sizeof(log_info->log_dir));
			snprintf(log_info->log_dir,
				 sizeof(log_info->log_dir) - 1, "%s/%s",
				 bfm_get_bfmr_log_root_path(), dirp->d_name);
			memset((void *)&st, 0, sizeof(st));

			ret = bfm_sys_lstat(log_info->log_dir, &st);
			if ((ret != 0) || (!S_ISDIR(st.st_mode))) {
				BFMR_PRINT_ERR("newlstat %s failed or %s isn't a dir!\n",
						log_info->log_dir,
						log_info->log_dir);
				goto __continue;
			}

			/* check if the log belongs to the last */
			/*	bootfail or not */
			memset(full_path, 0, BFMR_MAX_PATH);
			snprintf(full_path, BFMR_MAX_PATH - 1, "%s/%s",
				 log_info->log_dir, BFM_DONE_FILE_NAME);
			ret = bfm_sys_lstat(full_path, &st);
			log_idx = (ret == 0) ? (log_idx) : (log_idx + 1);

__continue:
			num -= dirp->d_reclen;
			dirp = (void *)dirp + dirp->d_reclen;
		}

		if (log_idx >= log_max_count) {
			BFMR_PRINT_ERR("extent max count: %d!\n",
				(int)log_max_count);
			break;
		}
		dirp = buf;
		memset(buf, 0, BFMR_MAX_PATH);
		num = sys_getdents64(fd, dirp, BFMR_MAX_PATH);
	}

	/* save log count */
	bf_log->log_dir_count = log_idx;

__out:
	if (fd >= 0) {
		sys_close(fd);
		fd = -1;
	}
	set_fs(oldfs);
	bfmr_free(buf);
	buf = NULL;
	bfmr_free(full_path);
	full_path = NULL;
	BFMR_PRINT_ERR("There're %d valid bootfail logs:\n",
		bf_log->log_dir_count);
	for (i = 0; i < bf_log->log_dir_count; i++)
		BFMR_PRINT_SIMPLE_INFO("%s\n", bf_log->bootfail_logs[i].log_dir);

	return 0;
}

static int bfm_update_info_for_each_log(void)
{
	struct bfm_bf_log_info *bf_log = NULL;

	bf_log = bfmr_malloc(sizeof(*bf_log));
	if (bf_log == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		return -1;
	}
	memset(bf_log, 0, sizeof(*bf_log));

	/* traverse the log dir and save the path of valid bootfail log */
	bfm_traverse_log_root_dir(bf_log);

	/* get real recovery info */
	if (bfr_get_real_recovery_info(&(bf_log->real_rcv_info)) != 0) {
		BFMR_PRINT_ERR("get real recovery info failed!\n");
		goto __out;
	}

	/* delete the user unsensible bootfail log */
	if (!bfm_is_user_sensible(bf_log)) {
		BFMR_PRINT_KEY_INFO("%s is a user unsensible bootfail log!\n",
			bf_log->bootfail_logs[0].log_dir);
		(void)bfm_delete_user_unsensible_logs(bf_log);
		goto __out;
	}

	/* update bootfail info for each usefull log */
	bfm_merge_bootfail_logs(bf_log);
	bfm_update_platform_logs(bf_log);
	(void)bfm_update_bf_info_for_each_log(bf_log);
	(void)bfm_update_rcv_info_for_each_log(bf_log);

__out:
	bfmr_free(bf_log);

	return 0;
}

static int bfm_lookup_dir_by_create_time(const char *root,
	char *log_path,
	size_t log_path_len,
	int find_oldest_log)
{
	int fd;
	int num;
	int log_count = 0;
	void *buf = NULL;
	char *full_path = NULL;
	struct linux_dirent64 *dirp = NULL;
	mm_segment_t oldfs;
	unsigned long long special_time = 0;
	unsigned long long cur_time;

	if (unlikely(log_path == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("log_path\n");
		return -1;
	}

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	memset((void *)log_path, 0, log_path_len);
	fd = sys_open(root, O_RDONLY, 0);
	if (fd < 0) {
		BFMR_PRINT_ERR("open [%s] failed!\n", root);
		goto __out;
	}

	buf = bfmr_malloc(BFMR_MAX_PATH);
	if (buf == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}

	full_path = (char *)bfmr_malloc(BFMR_MAX_PATH);
	if (full_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}

	dirp = buf;
	num = sys_getdents64(fd, dirp, BFMR_MAX_PATH);
	while (num > 0) {
		while (num > 0) {
			bfm_stat_t st;
			int ret;

			memset(full_path, 0, BFMR_MAX_PATH);
			snprintf(full_path, BFMR_MAX_PATH - 1, "%s/%s",
				 root, dirp->d_name);
			if ((strcmp(dirp->d_name, ".") == 0) ||
			    (strcmp(dirp->d_name, "..") == 0)) {
				num -= dirp->d_reclen;
				dirp = (void *)dirp + dirp->d_reclen;
				continue;
			}

			memset((void *)&st, 0, sizeof(st));
			ret = bfm_sys_lstat(full_path, &st);
			if (ret != 0) {
				num -= dirp->d_reclen;
				dirp = (void *)dirp + dirp->d_reclen;
				BFMR_PRINT_ERR("newlstat %s failed!\n",
					full_path);
				continue;
			}

			if (!S_ISDIR(st.st_mode)) {
				num -= dirp->d_reclen;
				dirp = (void *)dirp + dirp->d_reclen;
				BFMR_PRINT_ERR("%s is not a dir!\n", full_path);
				continue;
			}

			/* Note: We must exclude the uploaded dir */
			if (strcmp(dirp->d_name, BFM_UPLOADING_DIR_NAME) == 0) {
				num -= dirp->d_reclen;
				dirp = (void *)dirp + dirp->d_reclen;
				BFMR_PRINT_ERR("%s must be excluded!\n",
						full_path);
				continue;
			}

			cur_time = (unsigned long long)st.st_mtime *
				BFM_US_PERSECOND + st.st_mtime_nsec / 1000;
			if (special_time == 0) {
				strncpy(log_path, full_path, log_path_len - 1);
				special_time = cur_time;
			} else if (find_oldest_log != 0 &&
				   special_time > cur_time) {
				strncpy(log_path, full_path, log_path_len - 1);
				special_time = cur_time;
			} else if (find_oldest_log == 0 &&
				   special_time < cur_time) {
				strncpy(log_path, full_path, log_path_len - 1);
				special_time = cur_time;
			}

			log_count++;
			num -= dirp->d_reclen;
			dirp = (void *)dirp + dirp->d_reclen;
		}
		dirp = buf;
		memset(buf, 0, BFMR_MAX_PATH);
		num = sys_getdents64(fd, dirp, BFMR_MAX_PATH);
	}

__out:
	if (fd >= 0) {
		sys_close(fd);
		fd = -1;
	}
	set_fs(oldfs);

	BFMR_PRINT_KEY_INFO("Find %d log in %s, the %s log path is: %s\n",
			    log_count, root,
			    (find_oldest_log == 0) ? ("newest") : ("oldest"),
			    log_path);

	bfmr_free(buf);
	bfmr_free(full_path);
	buf = NULL;
	full_path = NULL;
	return 0;
}

static int bfm_notify_boot_success(void *param)
{
	unsigned int bootup_time;

	mutex_lock(&process_boot_fail_mutex);
	if (is_process_boot_success) {
		mutex_unlock(&process_boot_fail_mutex);
		BFMR_PRINT_ERR("is_process_boot_success be set already!\n");
		return 0;
	}

	/* 1. notify boot success event to the BFR */
	boot_status_notify(1);
	/* 2. let platfrom process the boot success */
	bfm_platform_process_boot_success();

	/* 3. update recovery result in recovery_info.txt */
	bfm_update_info_for_each_log();
	is_process_boot_success = true;
	release_rrecord_param();
	mutex_unlock(&process_boot_fail_mutex);

	/* 4. check bootup slowly event */
	bootup_time = bfmr_get_bootup_time();
	if ((bootup_time >= BFM_BOOT_SLOWLY_THRESHOLD) && (!is_boot_success)) {
		BFMR_PRINT_ERR("bootup time %dS is too long\n", bootup_time);
		boot_fail_err(BOOTUP_SLOWLY, DO_NOTHING, NULL);
	} else {
		// once if boot success, free boot stage info resource
		bfm_deinit_stage_info();
	}

	is_boot_success = true;

	return 0;
}

/*
 * @function: int bfmr_get_timer_state(int *state)
 * @brief: get state of the timer.
 * @param: state [out], the state of the boot timer.
 * @return: 0 - success, -1 - failed.
 * @note:
 *       1. this fuction only need be initialized in kernel.
 *       2. if *state == 0, the boot timer is disabled, if *state == 1,
 *   the boot timer is enbaled.
 */
int bfmr_get_timer_state(int *state)
{
	return bfm_get_boot_timer_state(state);
}

/*
 * @function: int bfm_enable_timer(void)
 * @brief: enbale timer.
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_enable_timer(void)
{
	return bfmr_resume_boot_timer();
}

/*
 * @function: int bfm_disable_timer(void)
 * @brief: disable timer.
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_disable_timer(void)
{
	return bfm_suspend_boot_timer();
}

/*
 * @function: int bfm_set_timer_timeout_value(unsigned int timeout)
 * @brief: set timeout value of the kernel timer.
 * @Note: the timer which control the boot procedure is in the kernel.
 * @param: timeout [in] timeout value (unit: msec).
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_set_timer_timeout_value(unsigned int timeout)
{
	return bfm_set_boot_timer_timeout_value(timeout);
}

/*
 * @function: int bfm_get_timer_timeout_value(unsigned int *timeout)
 * @brief: get timeout value of the kernel timer.
 * @Note: the timer which control the boot procedure is in the kernel.
 * @param: timeout [in] buffer will store the timeout value (unit: msec).
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_get_timer_timeout_value(unsigned int *timeout)
{
	return bfm_get_boot_timer_timeout_value(timeout);
}

/*
 * @function: int bfmr_action_timer_ctl(struct action_ioctl_data *pact_data)
 * @brief: acttion timers opertion
 * @param: pact_data  ationt ctl data
 * @return: 0 - succeeded; -1 - failed.
 */
static int bfmr_action_timer_ctl(struct action_ioctl_data *pact_data)
{
	int ret = -EPERM;

	switch (pact_data->op) {
	case ACT_TIMER_START:
		ret = bfm_action_timer_start(pact_data->action_name,
				pact_data->action_timer_timeout_value);
		break;
	case ACT_TIMER_STOP:
		ret = bfm_action_timer_stop(pact_data->action_name);
		break;
	case ACT_TIMER_PAUSE:
		ret = bfm_action_timer_pause(pact_data->action_name);
		break;
	case ACT_TIMER_RESUME:
		ret = bfm_action_timer_resume(pact_data->action_name);
		break;
	default:
		break;
	}

	return ret;
}

static unsigned long long bfm_get_system_time(void)
{
	struct timeval tv = {0};

	do_gettimeofday(&tv);

	return (unsigned long long)tv.tv_sec;
}

/*
 * @function: int bfmr_capture_and_save_log(enum bfmr_log_type type,
 *    struct bfmr_log_dst *dst)
 * @brief: capture and save log, initialised in core.
 * @param: src [in] log src info.
 * @param: dst [in] infomation about the media of saving log.
 * @return: 0 - succeeded; -1 - failed.
 * @note: it need be initialized in bootloader and kernel.
 */
static int bfmr_capture_and_save_log(struct bfmr_log_src *src,
		struct bfmr_log_dst *dst, struct bfm_param *pparam)
{
	int i;
	bool is_valid_log_type = false;
	int count = ARRAY_SIZE(log_type_buffer_info);
	unsigned int bytes_read;

	if (unlikely((src == NULL) || (dst == NULL) || (pparam == NULL))) {
		BFMR_PRINT_INVALID_PARAMS("src or dst or pparam\n");
		return -1;
	}
	for (i = 0; i < count; i++) {
		if (src->log_type != log_type_buffer_info[i].log_type)
			continue;

		log_type_buffer_info[i].buf =
			bfmr_malloc(log_type_buffer_info[i].buf_len + 1);
		if (log_type_buffer_info[i].buf == NULL) {
			BFMR_PRINT_ERR("bfmr_malloc failed!\n");
			return -1;
		}
		memset(log_type_buffer_info[i].buf, 0,
			log_type_buffer_info[i].buf_len + 1);
		is_valid_log_type = true;
		break;
	}

	if (!is_valid_log_type) {
		BFMR_PRINT_ERR("Invalid log type: %d\n", (int)(src->log_type));
		return -1;
	}

	src->save_log_after_reboot = pparam->save_log_after_reboot;
	bytes_read = bfmr_capture_log_from_system(log_type_buffer_info[i].buf,
			log_type_buffer_info[i].buf_len, src, 0);
	if (bfmr_is_oversea_commercail_version()) {
		BFMR_PRINT_ERR("Note: logs of oversea commercail version can't be saved!\n");
	} else {
		switch (dst->type) {
		case DST_FILE:
			bfmr_save_log_to_fs(dst->dst_info.filename,
					    log_type_buffer_info[i].buf,
					    bytes_read, 0);
			break;
		case DST_RAW_PART:
			if (dst->dst_info.raw_part.offset >= 0) {
				bfmr_save_log_to_raw_part(
					dst->dst_info.raw_part.raw_part_name,
					(unsigned long long)dst->dst_info.raw_part.offset,
					log_type_buffer_info[i].buf,
					bytes_read);
				bfmr_update_raw_log_info(src, dst, bytes_read);
			} else {
				BFMR_PRINT_ERR("dst->dst_info.raw_part.offset is negative %d\n",
						dst->dst_info.raw_part.offset);
			}
			break;
		case DST_MEMORY_BUFFER:
		default:
			bfmr_save_log_to_mem_buffer(dst->dst_info.buffer.addr,
						    dst->dst_info.buffer.len,
						    log_type_buffer_info[i].buf,
						    bytes_read);
			break;
		}
	}

	bfmr_free(log_type_buffer_info[i].buf);
	log_type_buffer_info[i].buf = NULL;

	return 0;
}

static long bfmr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	unsigned int timeout_value = -1;
	enum bfmr_detail_stage boot_stage;
	struct bfmr_bf_info *bf_info = NULL;
	struct action_ioctl_data *pact_data = NULL;
	uintptr_t args = arg;

	if (!bfmr_has_been_enabled()) {
		BFMR_PRINT_KEY_INFO("BFMR is disabled!\n");
		return -EPERM;
	}

	if ((void *)args == NULL) {
		BFMR_PRINT_INVALID_PARAMS("arg\n");
		return -EINVAL;
	}

	switch (cmd) {
	case BFMR_GET_TIMER_STATUS: {
		int state;
		bfmr_get_timer_state(&state);
		if (copy_to_user((int *)args, &state, sizeof(state)) != 0) {
			BFMR_PRINT_ERR("copy_to_user failed!\n");
			ret = -EFAULT;
			break;
		}
		BFMR_PRINT_KEY_INFO("short timer stats is: %d\n", state);
		break;
	}
	case BFMR_ENABLE_TIMER:
		bfmr_enable_timer();
		break;
	case BFMR_DISABLE_TIMER:
		bfmr_disable_timer();
		break;
	case BFMR_GET_TIMER_TIMEOUT_VALUE:
		bfmr_get_timer_timeout_value(&timeout_value);
		if (copy_to_user((int *)args, &timeout_value,
		    sizeof(timeout_value)) != 0) {
			BFMR_PRINT_ERR("copy_to_user failed!\n");
			ret = -EFAULT;
			break;
		}
		BFMR_PRINT_KEY_INFO("short timer timeout value is: %d\n",
				    timeout_value);
		break;
	case BFMR_SET_TIMER_TIMEOUT_VALUE:
		if (copy_from_user(&timeout_value, (int *)args,
		    sizeof(timeout_value)) != 0) {
			BFMR_PRINT_ERR("copy_from_user failed!\n");
			ret = -EFAULT;
			break;
		}
		bfmr_set_timer_timeout_value(timeout_value);
		BFMR_PRINT_KEY_INFO("set short timer timeout value to: %d\n",
				    timeout_value);
		break;
	case BFMR_GET_BOOT_STAGE:
		bfmr_get_boot_stage(&boot_stage);
		if (copy_to_user((enum bfmr_detail_stage *)args,
		    &boot_stage, sizeof(boot_stage)) != 0) {
			BFMR_PRINT_ERR("copy_to_user failed!\n");
			ret = -EFAULT;
			break;
		}
		BFMR_PRINT_KEY_INFO("bfmr_bootstage is: 0x%08x\n",
				    (unsigned int)boot_stage);
		break;
	case BFMR_SET_BOOT_STAGE: {
		enum bfmr_detail_stage old_boot_stage;

		bfmr_get_boot_stage(&old_boot_stage);
		if (copy_from_user(&boot_stage, (int *)args,
		    sizeof(boot_stage)) != 0) {
			BFMR_PRINT_ERR("copy_from_user failed!\n");
			ret = -EFAULT;
			break;
		}
		BFMR_PRINT_KEY_INFO("set bfmr_bootstage to: 0x%08x\n",
				    (unsigned int)boot_stage);
		bfmr_set_boot_stage(boot_stage);

		break;
	}
	case BFMR_PROCESS_BOOT_FAIL:
		bf_info = bfmr_malloc(sizeof(*bf_info));
		if (bf_info == NULL) {
			BFMR_PRINT_ERR("bfmr_malloc failed!\n");
			ret = -ENOMEM;
			break;
		}
		memset((void *)bf_info, 0, sizeof(*bf_info));
		if (copy_from_user(bf_info, ((struct bfmr_bf_info *)args),
		    sizeof(*bf_info)) != 0) {
			BFMR_PRINT_ERR("copy_from_user failed!\n");
			bfmr_free(bf_info);
			bf_info = NULL;
			ret = -EFAULT;
			break;
		}
		bf_info->addl_info.log_path[sizeof(bf_info->addl_info.log_path) - 1] = '\0';
		BFMR_PRINT_KEY_INFO("bootfail_errno: 0x%08x, suggested_recovery_method: %d, log_file [%s]'s lenth:%ld\n",
				    (unsigned int)bf_info->bf_no,
				    (int)bf_info->suggested_rmethod,
				    bf_info->addl_info.log_path,
				    bfmr_get_file_length(bf_info->addl_info.log_path));
		(void)boot_fail_err(bf_info->bf_no, bf_info->suggested_rmethod,
				    &bf_info->addl_info);
		bfmr_free(bf_info);
		break;
	case BFMR_GET_DEV_PATH:
		break;
	case BFMR_ENABLE_CTRL: {
		int enbale;

		if (copy_from_user(&enbale, (int *)args, sizeof(enbale)) != 0) {
			BFMR_PRINT_KEY_INFO("Failed to copy enable flag from user!\n");
			ret = -EFAULT;
			break;
		}
		BFMR_PRINT_KEY_INFO("set bfmr enable flag: 0x%08x\n",
				    (unsigned int)enbale);
		bfmr_enable_ctl(enbale);
		break;
	}
	case BFMR_ACTION_TIMER_CTL:
		pact_data = bfmr_malloc(sizeof(*pact_data));
		if (pact_data == NULL) {
			BFMR_PRINT_ERR("bfmr_malloc failed!\n");
			ret = -ENOMEM;
			break;
		}

		if (copy_from_user(pact_data, (struct action_ioctl_data *)args,
				   sizeof(*pact_data)) != 0) {
			BFMR_PRINT_KEY_INFO("Failed to copy acttion_ioctl data from user buffer!\n");
			bfmr_free(pact_data);
			ret = -EFAULT;
			break;
		}

		pact_data->action_name[BFMR_ACTION_NAME_LEN - 1] = '\0';
		BFMR_PRINT_KEY_INFO("set bfmr action timer: %x, %s, %d\n",
				    pact_data->op, pact_data->action_name,
				    pact_data->action_timer_timeout_value);
		ret = bfmr_action_timer_ctl(pact_data);
		bfmr_free(pact_data);
		break;
#ifdef CONFIG_HUAWEI_DYNAMIC_BRD
	case BFMR_IOC_CREATE_DYNAMIC_RAMDISK:
		ret = create_dynamic_ramdisk(
			(struct bfmr_dbrd_ioctl_block __user *)args);
		break;
	case BFMR_IOC_DELETE_DYNAMIC_RAMDISK:
		ret = delete_dynamic_ramdisk(
			(struct bfmr_dbrd_ioctl_block __user *)args);
		break;
#endif
#ifdef CONFIG_HUAWEI_STORAGE_ROFA
	case BFMR_IOC_CHECK_BOOTDISK_WP:
		ret = storage_rochk_ioctl_check_bootdisk_wp(
			(struct bfmr_bootdisk_wp_status_iocb __user *)args);
		break;
	case BFMR_IOC_ENABLE_MONITOR:
		ret = storage_rochk_ioctl_enable_monitor(
			(struct bfmr_storage_rochk_iocb __user *)args);
		break;
	case BFMR_IOC_DO_STORAGE_WRTRY:
		ret = storage_rochk_ioctl_run_storage_wrtry_sync(
			(struct bfmr_storage_rochk_iocb __user *)args);
		break;
	case BFMR_IOC_GET_STORAGE_ROFA_INFO:
		ret = storage_rofa_ioctl_get_rofa_info(
			(struct bfmr_storage_rofa_info_iocb __user *)args);
		break;
	case BFMR_IOC_GET_BOOTDEVICE_DISK_COUNT:
		ret = storage_rochk_ioctl_get_bootdevice_disk_count(
			(struct bfmr_storage_rochk_iocb __user *)args);
		break;
	case BFMR_IOC_GET_BOOTDEVICE_DISK_INFO:
		ret = storage_rochk_ioctl_get_bootdevice_disk_info(
			(struct bfmr_bootdevice_disk_info_iocb __user *)args);
		break;
	case BFMR_IOC_GET_BOOTDEVICE_PROD_INFO:
		ret = storage_rochk_ioctl_get_bootdevice_prod_info(
			(struct bfmr_bootdevice_prod_info_iocb __user *)args);
		break;

#endif

	default:
		BFMR_PRINT_ERR("Invalid CMD: 0x%x\n", cmd);
		ret = -EFAULT;
		break;
	}

	return ret;
}

static int bfmr_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

static ssize_t bfmr_write(struct file *file,
	const char *data, size_t len, loff_t *ppos)
{
	return len;
}

static ssize_t bfmr_read(struct file *file,
	char __user *buf, size_t count, loff_t *pos)
{
	return count;
}

static int bfmr_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int bfm_process_bottom_layer_boot_fail(void *param)
{
	struct bfmr_log_src src;
	int i;
	int ret = -1;
	int count = ARRAY_SIZE(log_type_buffer_info);
	unsigned int bytes_read;
	bool find_log_type_buffer_info = false;

	mutex_lock(&process_boot_fail_mutex);
	memset((void *)&src, 0, sizeof(src));
	src.log_type = LOG_TYPE_BFMR_TEMP_BUF;
	for (i = 0; i < count; i++) {
		if (log_type_buffer_info[i].log_type != LOG_TYPE_BFMR_TEMP_BUF)
			continue;

		/* update the buf_len firstly */
		log_type_buffer_info[i].buf_len = bfm_get_dfx_log_length();
		log_type_buffer_info[i].buf =
			bfmr_malloc(log_type_buffer_info[i].buf_len + 1);
		if (log_type_buffer_info[i].buf == NULL) {
			BFMR_PRINT_ERR("bfmr_malloc failed!\n");
			goto __out;
		}
		memset(log_type_buffer_info[i].buf, 0,
			log_type_buffer_info[i].buf_len + 1);
		find_log_type_buffer_info = true;
		break;
	}
	if (!find_log_type_buffer_info) {
		BFMR_PRINT_ERR("Can not find log buffer info for \"LOG_TYPE_BFMR_TEMP_BUF\"\n");
		goto __out;
	}

	bytes_read = bfmr_capture_log_from_system(log_type_buffer_info[i].buf,
						  log_type_buffer_info[i].buf_len,
						  &src, 0);
	if (bytes_read == 0U) {
		ret = 0;
		BFMR_PRINT_ERR("There is no bottom layer bootfail log!\n");
		goto __out;
	}

	ret = bfm_parse_and_save_bottom_layer_bootfail_log(
		(struct bfm_param *)param,
		log_type_buffer_info[i].buf,
		log_type_buffer_info[i].buf_len);
	if (ret != 0) {
		BFMR_PRINT_ERR("Failed to save bottom layer bootfail log!\n");
		goto __out;
	}

__out:
	if (i < count) {
		if (log_type_buffer_info[i].buf != NULL) {
			bfmr_free(log_type_buffer_info[i].buf);
			log_type_buffer_info[i].buf = NULL;
		}
	}
	bfmr_free(param);
	mutex_unlock(&process_boot_fail_mutex);

	return ret;
}

static int bfmr_capture_and_save_bottom_layer_boot_fail_log(void)
{
	struct task_struct *tsk = NULL;
	struct bfm_param *pparam = NULL;

	pparam = bfmr_malloc(sizeof(*pparam));
	if (pparam == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		return -1;
	}
	memset((void *)pparam, 0, sizeof(*pparam));

	tsk = kthread_run(bfm_process_bottom_layer_boot_fail, (void *)pparam,
			  "save_bl_bf_log");
	if (IS_ERR(tsk)) {
		BFMR_PRINT_ERR("Failed to create thread to save bottom layer bootfail log!\n");
		return -1;
	}

	return 0;
}

/* this function can not be invoked in interrupt context */
static int bfm_process_ocp_excp(enum bfmr_hw_fault_type fault_type,
		struct ocp_excp_info *pexcp_info)
{
	struct bfmr_bf_addl_info *paddl_info = NULL;
	struct bfmr_hw_fault_info_param *pfault_info_param = NULL;
	int fault_times;

	if (unlikely(pexcp_info == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("pexcp_info\n");
		return -1;
	}

	paddl_info = bfmr_malloc(sizeof(*paddl_info));
	if (paddl_info == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}
	memset(paddl_info, 0, sizeof(*paddl_info));

	pfault_info_param = bfmr_malloc(sizeof(*pfault_info_param));
	if (pfault_info_param == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		goto __out;
	}
	memset((void *)pfault_info_param, 0,
		sizeof(*pfault_info_param));

	pfault_info_param->fault_stage = HW_FAULT_STAGE_AFTER_BOOT_SUCCESS;
	memcpy(&pfault_info_param->hw_excp_info.ocp_excp_info,
		pexcp_info,
		sizeof(pfault_info_param->hw_excp_info.ocp_excp_info));
	memcpy(paddl_info->detail_info, pexcp_info->ldo_num,
		BFMR_MIN(sizeof(paddl_info->detail_info) - 1,
		strlen(pexcp_info->ldo_num)));
	paddl_info->hardware_fault_type = HW_FAULT_OCP;
	if (is_boot_success) {
		mutex_lock(&process_boot_fail_mutex);
		fault_times = bfr_get_hardware_fault_times(pfault_info_param);
		mutex_unlock(&process_boot_fail_mutex);
		if (fault_times == 0)
			boot_fail_err(BFM_HARDWARE_FAULT, DO_NOTHING, paddl_info);
		else
			BFMR_PRINT_KEY_INFO("BFM has processed OCP after boot-success one time!\n");

		goto __out;
	}

	pfault_info_param->fault_stage = HW_FAULT_STAGE_DURING_BOOTUP;
	mutex_lock(&process_boot_fail_mutex);
	fault_times = bfr_get_hardware_fault_times(pfault_info_param);
	mutex_unlock(&process_boot_fail_mutex);
	switch (fault_times) {
	case 0:
		BFMR_PRINT_KEY_INFO("%s has ocp once!\n", pexcp_info->ldo_num);
		boot_fail_err(BFM_HARDWARE_FAULT, NO_SUGGESTION, paddl_info);
		break;
	case 1:
		BFMR_PRINT_KEY_INFO("%s has ocp twice!\n", pexcp_info->ldo_num);
		boot_fail_err(BFM_HARDWARE_FAULT, DO_NOTHING, paddl_info);
		break;
	default:
		BFMR_PRINT_KEY_INFO("BFM has processed the same OCP during bootup!\n");
		break;
	}

__out:
	bfmr_free(paddl_info);
	bfmr_free(pfault_info_param);

	return 0;
}

/*
 * @function: int bfm_init(void)
 * @brief: init bfm module in kernel.
 * @param: none.
 * @return: 0 - succeeded; -1 - failed.
 * @note: it need be initialized in bootloader and kernel.
 */
int bfm_init(void)
{
	int ret;
	struct bfm_chipsets_init_param chipsets_init_param;

	if (!bfmr_has_been_enabled()) {
		BFMR_PRINT_KEY_INFO("BFMR is disabled!\n");
		return 0;
	}

	ret = misc_register(&bfmr_miscdev);
	if (ret != 0) {
		BFMR_PRINT_ERR("misc_register failed, ret: %d\n", ret);
		return ret;
	}

	(void)bfm_init_boot_timer();
	(void)bfm_init_stage_info();
	memset((void *)&chipsets_init_param, 0,
		sizeof(chipsets_init_param));
	chipsets_init_param.log_saving_param.catch_save_bf_log =
		bfm_catch_save_bf_log;
	chipsets_init_param.process_ocp_excp = bfm_process_ocp_excp;
	bfm_chipsets_init(&chipsets_init_param);
	bfmr_capture_and_save_bottom_layer_boot_fail_log();

	return 0;
}
