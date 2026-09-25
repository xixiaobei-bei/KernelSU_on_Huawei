/*
 * bfm_chipsets.c
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

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/sched/clock.h>
#include <soc/mediatek/ram_console.h>
#ifdef CONFIG_HUAWEI_STORAGE_ROFA
#include "chipset_common/storage_rofa/storage_rofa.h"
#endif
#include "chipset_common/bfmr/public/bfmr_public.h"
#include "chipset_common/bfmr/common/bfmr_common.h"
#include "chipset_common/bfmr/bfm/chipsets/bfm_chipsets.h"
#include "chipset_common/bfmr/bfm/chipsets/mtk/bfm_mtk.h"

#define BFM_LOG_PART_MOUINT_POINT  "/log"
#define BFM_LOG_ROOT_PATH          "/log/reliability/boot_fail"
#define BFM_DBG_FILE_NAME          "db.fatal.dbg"
#define BFM_LOG_UPLOADING_PATH     BFM_LOG_ROOT_PATH "/" BFM_UPLOADING_DIR_NAME
#define BFM_BL1_BF_LOG_NAME        "sbl1.log"
#define BFM_BL2_BF_LOG_NAME        "lk.log"
#define BFM_KERNEL_BF_LOG_NAME     "last_kmsg"
#define BFM_RAMOOPS_BF_LOG_NAME    "pmsg-ramoops-0"

/* the bfi part has been devided into many pieces whose size are 4096 */
#define BFM_BFI_PART_NAME          "bootfail_info"
#define BFM_BFI_MAGIC_NUMBER       0x42464958
#define BFM_BFI_HEADER_SIZE        BFMR_SIZE_4K
#define BFM_BFI_MEMBER_SIZE        BFMR_SIZE_4K
#define BFM_BFI_UNSAVE             0X55AAAA55
#define BFM_BFI_SAVED              0XAA5555AA
#define BFM_BFI_RECORD_TOTAL_COUNT 20
#define BFM_BFI_PWK_PRESS_OFF      (BFMR_SIZE_4K * 1024)

#define BFM_RAW_PART_NAME          "bootfail_info"
#define BFM_RAW_LOG_MAGIC          0x12345678
#define BFM_RAW_PART_OFFSET        (512 * 1024)
#define BFM_KMSG_TMP_BUF_LEN       (3 * 1024 * 512)
#define BFM_KMSG_TEXT_LOG_LEN      (512 * 1024)
#define BFM_BOOT_UP_60_SECOND      60
#define BFM_TIME_S_TO_NS           (1000 * 1000 * 1000)

#define BFM_WAIT_FOR_LOG_PART_TIMEOUT 40
#define BFM_BFI_PART_MAX_COUNT        2

static struct bfm_log_save_param bf_log_saving_param;

static int bfm_get_bfi_part_full_path(char *path_buf,
	unsigned int path_buf_len);
unsigned int bfmr_capture_log_from_src_file(char *buf,
	unsigned int buf_len, char *src_log_path);
static unsigned int bfmr_capture_tombstone(char *buf,
	unsigned int buf_len, char *src_log_file_path);
static unsigned int bfmr_capture_vm_crash(char *buf,
	unsigned int buf_len, char *src_log_file_path);
static unsigned int bfmr_capture_vm_watchdog(char *buf,
	unsigned int buf_len, char *src_log_file_path);
static unsigned int bfmr_capture_normal_framework_bootfail_log(
	char *buf, unsigned int buf_len, char *src_log_file_path);
static unsigned int capture_logcat_on_beta(
	char *buf, unsigned int buf_len, char *src_log_file_path);
static unsigned int bfmr_capture_critical_process_crash_log(
	char *buf, unsigned int buf_len, char *src_log_file_path);
static unsigned int bfmr_capture_fixed_framework_bootfail_log(
	char *buf, unsigned int buf_len, char *src_log_file_path);
static unsigned int bfm_capture_kmsg_log(char *buf, unsigned int buf_len);

int bfm_get_boot_stage(enum bfmr_detail_stage *bootstage)
{
	if (unlikely(bootstage == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("pbfmr_bootstage: %p\n", bootstage);
		return -1;
	}

	*bootstage = get_boot_stage();
	return 0;
}

int bfm_set_boot_stage(enum bfmr_detail_stage bfmr_bootstage)
{
	set_boot_stage(bfmr_bootstage);
	return 0;
}

unsigned int bfmr_capture_log_from_src_file(char *buf,
	unsigned int buf_len, char *src_log_path)
{
	int fd_src = -1;
	char *ptemp = NULL;
	long src_file_len;
	mm_segment_t old_fs;
	long bytes_to_read;
	long bytes_read;
	long seek_pos;
	unsigned long bytes_read_total = 0;

	if (unlikely((buf == NULL) || (src_log_path == NULL))) {
		BFMR_PRINT_INVALID_PARAMS("buf: %p src_file_path: %p\n",
			buf, src_log_path);
		return 0;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	/* get the length of the source file */
	src_file_len = bfmr_get_file_length(src_log_path);
	if (src_file_len <= 0) {
		BFMR_PRINT_ERR("length of %s is: %ld\n", src_log_path,
			src_file_len);
		goto __out;
	}

	fd_src = sys_open(src_log_path, O_RDONLY, 0);
	if (fd_src < 0) {
		BFMR_PRINT_ERR("sys_open %s failed, ret = %d\n",
			src_log_path, fd_src);
		goto __out;
	}
	BFMR_PRINT_INFO("src_file_len:%lld, source_Log_path:%s, bufflen:%lld\n",
		src_file_len, src_log_path, buf_len);
	/* lseek for reading the latest log */
	seek_pos = (src_file_len <= (long)buf_len) ?
			(0L) : (src_file_len - (long)buf_len);
	if (sys_lseek(fd_src, (unsigned int)seek_pos, SEEK_SET) < 0) {
		BFMR_PRINT_ERR("sys_lseek %s failed\n", src_log_path);
		goto __out;
	}

	/* read data from the user space source file */
	ptemp = buf;
	bytes_to_read = BFMR_MIN(src_file_len, (long)buf_len);
	while (bytes_to_read > 0) {
		bytes_read = bfmr_full_read(fd_src, ptemp, bytes_to_read);
		if (bytes_read < 0) {
			BFMR_PRINT_ERR("bfmr_full_read %s failed\n",
				src_log_path);
			goto __out;
		}
		bytes_to_read -= bytes_read;
		ptemp += bytes_read;
		bytes_read_total += bytes_read;
	}

__out:
	if (fd_src >= 0)
		sys_close(fd_src);

	set_fs(old_fs);

	return bytes_read_total;
}

static unsigned int bfmr_capture_tombstone(char *buf,
	unsigned int buf_len, char *src_log_file_path)
{
	return bfmr_capture_log_from_src_file(buf, buf_len, src_log_file_path);
}

static unsigned int bfmr_capture_vm_crash(char *buf,
	unsigned int buf_len, char *src_log_file_path)
{
	return bfmr_capture_log_from_src_file(buf, buf_len, src_log_file_path);
}

static unsigned int bfmr_capture_vm_watchdog(char *buf,
	unsigned int buf_len, char *src_log_file_path)
{
	return bfmr_capture_log_from_src_file(buf, buf_len, src_log_file_path);
}

static unsigned int bfmr_capture_normal_framework_bootfail_log(
	char *buf, unsigned int buf_len, char *src_log_file_path)
{
	return bfmr_capture_log_from_src_file(buf, buf_len, src_log_file_path);
}

static unsigned int capture_logcat_on_beta(char *buf,
	unsigned int buf_len, char *src_log_file_path)
{
	mm_segment_t old_fs;
	char src_path[BFMR_SIZE_256] = { 0 };

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	if (sys_readlink(src_log_file_path, src_path, sizeof(src_path) - 1) > 0)
		src_log_file_path = src_path;
	set_fs(old_fs);

	return bfmr_capture_log_from_src_file(buf, buf_len, src_log_file_path);
}

static unsigned int bfmr_capture_critical_process_crash_log(
	char *buf, unsigned int buf_len, char *src_log_file_path)
{
	return bfmr_capture_log_from_src_file(buf, buf_len, src_log_file_path);
}

static unsigned int bfmr_capture_fixed_framework_bootfail_log(
	char *buf, unsigned int buf_len, char *src_log_file_path)
{
	return bfmr_capture_log_from_src_file(buf, buf_len, src_log_file_path);
}

static unsigned int bfm_capture_kmsg_log(char *buf, unsigned int buf_len)
{
	int ret;
	char *kmsg_tmp = NULL;

	if (buf == NULL)
		return 0;

	kmsg_tmp = bfmr_malloc(BFM_KMSG_TMP_BUF_LEN);

	if (kmsg_tmp == NULL) {
		ret = kmsg_print_to_ddr(buf, (int)buf_len);
		ret = ((ret > 0) ? ret : 0);
		return (unsigned int)ret;
	}

	ret = kmsg_print_to_ddr(kmsg_tmp, BFM_KMSG_TMP_BUF_LEN);
	ret = ((ret > 0) ? ret : 0);
	if (ret > buf_len) {
		memcpy(buf, kmsg_tmp + ret - buf_len, buf_len);
		ret = buf_len;
	} else {
		memcpy(buf, kmsg_tmp, ret);
	}
	bfmr_free(kmsg_tmp);

	return (unsigned int)ret;
}

unsigned int bfmr_capture_log_from_system(char *buf,
	unsigned int buf_len, struct bfmr_log_src *src, int timeout)
{
	unsigned int bytes_captured = 0U;

	if (unlikely((buf == NULL) || (src == NULL))) {
		BFMR_PRINT_INVALID_PARAMS("buf: %p, src: %p\n", buf, src);
		return 0U;
	}

	switch (src->log_type) {
	case LOG_TYPE_BOOTLOADER_1:
	case LOG_TYPE_BOOTLOADER_2:
		break;
	case LOG_TYPE_BFMR_TEMP_BUF:
		bytes_captured = bfm_get_dfx_log_length();
		break;
	case LOG_TYPE_TEXT_KMSG:
		if (src->save_log_after_reboot)
			bytes_captured = get_last_kmsg_pstore(buf, buf_len);
		else
			bytes_captured = bfm_capture_kmsg_log(buf, buf_len);
		break;
	case LOG_TYPE_RAMOOPS:
		break;
	case LOG_TYPE_BETA_APP_LOGCAT:
		if (!bfm_is_beta_version())
			break;
		if (src->save_log_after_reboot) {
			BFMR_PRINT_KEY_INFO("Save logcat after reboot\n");
			bytes_captured = capture_logcat_on_beta(buf, buf_len,
				BFM_LOGCAT_FILE_PATH);
			BFMR_PRINT_INFO("app1 length is %lld\n", bytes_captured);
			bytes_captured += capture_logcat_on_beta(
				(char *)(buf + bytes_captured),
				buf_len - bytes_captured,
				BFM_OLD_LOGCAT_FILE_PATH);
			BFMR_PRINT_INFO("app2 length is %lld\n", bytes_captured);
		} else {
			BFMR_PRINT_KEY_INFO("Save logcat now from android_logs instead!\n");
			bytes_captured = capture_logcat_on_beta(buf, buf_len,
				BFM_LOGCAT_FILE_PATH);
		}
		break;
	case LOG_TYPE_CRITICAL_PROCESS_CRASH:
		bytes_captured = bfmr_capture_critical_process_crash_log(buf,
				buf_len, src->src_log_file_path);
		break;
	case LOG_TYPE_VM_TOMBSTONES:
		bytes_captured = bfmr_capture_tombstone(buf, buf_len,
				src->src_log_file_path);
		break;
	case LOG_TYPE_VM_CRASH:
		bytes_captured = bfmr_capture_vm_crash(buf, buf_len,
				src->src_log_file_path);
		break;
	case LOG_TYPE_VM_WATCHDOG:
		bytes_captured = bfmr_capture_vm_watchdog(buf, buf_len,
				src->src_log_file_path);
		break;
	case LOG_TYPE_NORMAL_FRAMEWORK_BOOTFAIL_LOG:
		bytes_captured = bfmr_capture_normal_framework_bootfail_log(buf,
				buf_len, src->src_log_file_path);
		break;
	case LOG_TYPE_FIXED_FRAMEWORK_BOOTFAIL_LOG:
		bytes_captured = bfmr_capture_fixed_framework_bootfail_log(buf,
				buf_len, BFM_FRAMEWORK_BOOTFAIL_LOG_PATH);
		break;
	default:
		BFMR_PRINT_ERR("Invalid log type: %d\n",
			(int)(src->log_type));
		break;
	}

	return bytes_captured;
}

void *get_bfi_part_full_path(int length)
{
	char *bfi_part_full_path = NULL;

	bfi_part_full_path = bfmr_malloc(length);
	if (bfi_part_full_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		return NULL;
	}
	memset((void *)bfi_part_full_path, 0, length);
	return bfi_part_full_path;
}

static int bfm_check_validity_bf_log(struct bfi_member_Info *pbfi_info,
	u64 rtc_time, int *log_idx)
{
	char *pdata_buf = NULL;
	char *bfi_path = NULL;
	long bytes_read;
	long seek_result;
	mm_segment_t old_fs;
	int ret = -1;
	int fd = -1;
	int i;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	bfi_path = get_bfi_part_full_path(BFMR_DEV_FULL_PATH_MAX_LEN + 1);
	if (bfi_path == NULL)
		goto __out;

	pdata_buf = bfmr_malloc(BFM_BFI_MEMBER_SIZE);
	if (pdata_buf == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		goto __out;
	}
	memset((void *)pdata_buf, 0, BFM_BFI_MEMBER_SIZE);

	ret = bfm_get_bfi_part_full_path(bfi_path, BFMR_DEV_FULL_PATH_MAX_LEN);
	if (ret != 0) {
		BFMR_PRINT_ERR("failed ot get full path of bfi part\n");
		goto __out;
	}

	ret = -1;
	fd = sys_open(bfi_path, O_RDONLY, BFMR_FILE_LIMIT);
	if (fd < 0) {
		BFMR_PRINT_ERR("sys_open %s failed, ret = %d\n", bfi_path, fd);
		goto __out;
	}

	/* seek the header in the beginning of the BFI part */
	seek_result = sys_lseek(fd, BFM_BFI_HEADER_SIZE +
		BFM_BFI_MEMBER_SIZE * (*log_idx), SEEK_SET);
	if (seek_result !=
		(long)(BFM_BFI_HEADER_SIZE +
			BFM_BFI_MEMBER_SIZE * (*log_idx))) {
		BFMR_PRINT_ERR("sys_lseek %s failed", bfi_path);
		BFMR_PRINT_ERR("seek_results: %ld ,it must be: %ld\n",
			seek_result,
			(long)BFM_BFI_HEADER_SIZE);

		goto __out;
	}

	for (i = (*log_idx); i < BFM_BFI_RECORD_TOTAL_COUNT; i++) {
		memset((void *)pdata_buf, 0, BFM_BFI_MEMBER_SIZE);
		bytes_read = bfmr_full_read(fd, pdata_buf, BFM_BFI_MEMBER_SIZE);
		if (bytes_read < 0) {
			BFMR_PRINT_ERR("bfmr_full_read %s failed\n", bfi_path);
			BFMR_PRINT_ERR("ret = %ld\n", bytes_read);
			goto __out;
		}

		memcpy((void *)pbfi_info, (void *)pdata_buf,
			sizeof(struct bfi_member_Info));
		if ((pbfi_info->rtc_value != 0) &&
			(pbfi_info->is_log_saved == BFM_BFI_UNSAVE)) {
			*log_idx = i + 1;
			BFMR_PRINT_KEY_INFO("RTC time in BFI %d: %llx\n", i,
				pbfi_info->rtc_value);
			BFMR_PRINT_KEY_INFO("is_log_saved: %llx\n",
				pbfi_info->is_log_saved);
			ret = 0;
			break;
		}
		BFMR_PRINT_KEY_INFO("RTC time in BFI %d: %llx\n", i,
			pbfi_info->rtc_value);
		BFMR_PRINT_KEY_INFO("is_log_saved: %llx\n",
			pbfi_info->is_log_saved);
	}

	BFMR_PRINT_KEY_INFO("get bfi info %s\n", (ret == 0) ?
		("successfully") : ("failed"));

__out:
	if (fd >= 0)
		sys_close(fd);

	set_fs(old_fs);

	bfmr_free(bfi_path);
	bfmr_free(pdata_buf);

	return ret;
}

static int bfm_update_save_flag_in_bfi(
	struct bfi_member_Info *pbfi_info)
{
	struct bfi_member_Info *pbfi_info_local = NULL;
	char *bfi_path = NULL;
	long bytes_read;
	long bytes_write;
	long seek_result;
	mm_segment_t old_fs;
	int ret = -1;
	int fd = -1;
	int i;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	bfi_path = get_bfi_part_full_path(BFMR_DEV_FULL_PATH_MAX_LEN + 1);
	if (bfi_path == NULL)
		goto __out;

	pbfi_info_local = bfmr_malloc(BFM_BFI_MEMBER_SIZE);
	if (pbfi_info_local == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		goto __out;
	}
	memset((void *)pbfi_info_local, 0, BFM_BFI_MEMBER_SIZE);

	ret = bfm_get_bfi_part_full_path(bfi_path, BFMR_DEV_FULL_PATH_MAX_LEN);
	if (ret != 0) {
		BFMR_PRINT_ERR("failed ot get full path of bfi part\n");
		goto __out;
	}

	ret = -1;
	fd = sys_open(bfi_path, O_RDWR, BFMR_FILE_LIMIT);
	if (fd < 0) {
		BFMR_PRINT_ERR("sys_open %s failed, ret = %d\n", bfi_path, fd);
		goto __out;
	}

	/* seek the header in the beginning of the BFI part */
	seek_result = sys_lseek(fd, BFM_BFI_HEADER_SIZE, SEEK_SET);
	if (seek_result != (long)BFM_BFI_HEADER_SIZE) {
		BFMR_PRINT_ERR("sys_lseek %s failed\n", bfi_path);
		BFMR_PRINT_ERR("seek_results: %ld ,it must be: %ld\n",
			seek_result,
			(long)BFM_BFI_HEADER_SIZE);
		goto __out;
	}

	for (i = 0; i < BFM_BFI_RECORD_TOTAL_COUNT; i++) {
		memset((void *)pbfi_info_local, 0, BFM_BFI_MEMBER_SIZE);
		bytes_read = bfmr_full_read(fd, (char *)pbfi_info_local,
			BFM_BFI_MEMBER_SIZE);
		if (bytes_read < 0) {
			BFMR_PRINT_ERR("bfmr_full_read %s failed\n", bfi_path);
			BFMR_PRINT_ERR("ret = %ld\n", bytes_read);
			goto __out;
		}

		if ((pbfi_info->rtc_value == pbfi_info_local->rtc_value) &&
			(pbfi_info_local->is_log_saved != BFM_BFI_SAVED)) {
			seek_result = sys_lseek(fd,
				(long)(BFM_BFI_HEADER_SIZE +
				i * BFM_BFI_MEMBER_SIZE),
				SEEK_SET);
			if (seek_result !=
				(long)(BFM_BFI_HEADER_SIZE +
				i * BFM_BFI_MEMBER_SIZE)) {
				BFMR_PRINT_ERR("sys_lseek %s failed\n",
					bfi_path);
				BFMR_PRINT_ERR("seek_result: %ld, it must be: %ld\n",
					seek_result,
					(long)(BFM_BFI_HEADER_SIZE +
					i * BFM_BFI_MEMBER_SIZE));
				goto __out;
			}

			bytes_write = bfmr_full_write(fd, (char *)pbfi_info,
				BFM_BFI_MEMBER_SIZE);
			if (bytes_write <= 0L) {
				BFMR_PRINT_ERR("bfmr_full_write %s failed\n",
					bfi_path);
				BFMR_PRINT_ERR("ret = %ld\n",
					bytes_write);
				goto __out;
			}
			ret = 0;
			break;
		}
	}

__out:
	if (fd >= 0)
		sys_close(fd);
	set_fs(old_fs);
	bfmr_free(bfi_path);
	bfmr_free(pbfi_info_local);
	BFMR_PRINT_KEY_INFO("update log-saving flag in bfi part %s\n",
			(ret == 0) ? ("successfully") : ("failed"));

	return ret;
}

static int bfm_save_bootfail_log_to_fs_immediately(
	struct bfm_param *pfail_param,
	struct bfm_log_save_param *psave_param,
	int save_bottom_layer_bootfail_log)
{
	int ret = -1;
	int log_idx = 0;
	int check_times = 0;
	struct bfi_member_Info *pbfi_info = NULL;

	BFMR_PRINT_ENTER();
	if (unlikely((pfail_param == NULL) || (psave_param == NULL) ||
		((void *)psave_param->catch_save_bf_log == NULL))) {
		BFMR_PRINT_INVALID_PARAMS("parameters is NULL\n");
		return -1;
	}

	pbfi_info = bfmr_malloc(BFM_BFI_MEMBER_SIZE);
	if (pbfi_info == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		goto __out;
	}
	memset((void *)pbfi_info, 0, BFM_BFI_MEMBER_SIZE);

	while ((bfm_check_validity_bf_log(pbfi_info, 0, &log_idx) == 0) &&
		(check_times < BFM_BFI_RECORD_TOTAL_COUNT)) {
		if (pbfi_info->stage_code >= NATIVE_STAGE_START)
			pfail_param->save_bottom_layer_bootfail_log = 0;
		else
			pfail_param->save_bottom_layer_bootfail_log =
				save_bottom_layer_bootfail_log;
		pfail_param->bootfail_errno = pbfi_info->errno;
		pfail_param->bootfail_time = pbfi_info->rtc_value;
		pfail_param->bootup_time = pbfi_info->bootup_time;
		pfail_param->recovery_method =
			(enum bfr_method)pbfi_info->rcv_method;
		pfail_param->boot_stage =
			(enum bfmr_detail_stage)pbfi_info->stage_code;
		pfail_param->is_user_sensible =
			(int)pbfi_info->is_user_perceptiable;
		pfail_param->is_system_rooted = (int)pbfi_info->is_rooted;
		pfail_param->dmd_num = pbfi_info->dmd_errno;
		pfail_param->is_bootup_successfully = pbfi_info->is_boot_success;
		pfail_param->suggested_recovery_method =
			pbfi_info->sugst_rcv_method;
		pfail_param->reboot_type = pbfi_info->reboot_type;
		strncpy(pfail_param->excep_info,
			pbfi_info->excep_info,
			sizeof(pbfi_info->excep_info) - 1);

		ret = psave_param->catch_save_bf_log(pfail_param);
		if (ret != 0) {
			BFMR_PRINT_ERR("Save boot fail log failed\n");
		} else {
			pbfi_info->is_log_saved = BFM_BFI_SAVED;
			(void)bfm_update_save_flag_in_bfi(pbfi_info);
			memset((void *)pbfi_info, 0, BFM_BFI_MEMBER_SIZE);
		}
		check_times++;
	}

__out:
	bfmr_free(pbfi_info);
	BFMR_PRINT_EXIT();

	return ret;
}

int bfm_parse_and_save_bottom_layer_bootfail_log(struct bfm_param *param,
	char *buf,
	unsigned int buf_len)
{
	int ret = -1;
	struct dfx_head_info *pdfx_head_info = (struct dfx_head_info *)buf;

	if (unlikely((param == NULL) || (buf == NULL))) {
		BFMR_PRINT_INVALID_PARAMS("psave_param: %p, buf: %p\n",
			param, buf);
		goto __out;
	}

	/* 1. wait for the log part */
	BFMR_PRINT_SIMPLE_INFO("==== wait for log part start =====\n");
	if (bfmr_wait_for_part_mount_with_timeout(
		bfm_get_bfmr_log_part_mount_point(),
		BFM_WAIT_FOR_LOG_PART_TIMEOUT) != 0) {
		BFMR_PRINT_ERR("%s is not ready, logs can't be saved\n",
			bfm_get_bfmr_log_part_mount_point());
		goto __out;
	}
	BFMR_PRINT_SIMPLE_INFO("==== wait for log part end =====\n");

	param->save_log_after_reboot = true;
	ret = bfm_save_bootfail_log_to_fs_immediately(param,
		&bf_log_saving_param, 1);
	BFMR_PRINT_KEY_INFO("Save boot fail log %s\n",
		(ret == 0) ? ("successfully") : ("failed"));

__out:
	return ret;
}

int bfmr_save_log_to_fs(char *dst_file_path, char *buf,
	unsigned int log_len, int append)
{
	int ret = -1;
	int fd = -1;
	mm_segment_t fs;
	long bytes_written;

	if (unlikely(dst_file_path == NULL || buf == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("dst_file_path: %p, buf: %p\n",
			dst_file_path, buf);
		return -1;
	}

	if (log_len == 0U) {
		BFMR_PRINT_KEY_INFO("The log length is: %u\n", log_len);
		return 0;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	/* 1. open file for writing, please note the parameter-append */
	fd = sys_open(dst_file_path, O_CREAT | O_RDWR | ((append == 0) ?
		(0U) : O_APPEND), BFMR_FILE_LIMIT);
	if (fd < 0) {
		BFMR_PRINT_ERR("sys_open %s failed, fd: %d\n",
			dst_file_path, fd);
		goto __out;
	}

	/* 2. write file */
	bytes_written = bfmr_full_write(fd, buf, log_len);
	if ((long)log_len != bytes_written) {
		BFMR_PRINT_ERR("bfmr_full_write %s failed\n", dst_file_path);
		BFMR_PRINT_ERR("log_len: %ld bytes_written: %ld\n",
			(long)log_len, bytes_written);
		goto __out;
	}

	/* 3. change own and mode for the file */
	bfmr_change_own_mode(dst_file_path, BFMR_AID_ROOT, BFMR_AID_SYSTEM,
		BFMR_FILE_LIMIT);

	/* 4. write successfully, modify the value of ret */
	ret = 0;

__out:
	if (fd >= 0) {
		sys_fsync(fd);
		sys_close(fd);
	}

	set_fs(fs);

	return ret;
}

int bfmr_save_log_to_raw_part(char *raw_part_name,
	unsigned long long offset, char *buf, unsigned int log_len)
{
	int ret = -1;
	char *pdev_full_path = NULL;

	if (!log_len) {
		BFMR_PRINT_INVALID_PARAMS("raw_part_name: %p\n",
			raw_part_name);
		BFMR_PRINT_INVALID_PARAMS("buf: %p, loglen = %d\n",
			buf, log_len);
		return 0;
	}

	if (unlikely(raw_part_name == NULL || buf == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("raw_part_name: %p, buf: %p\n",
			raw_part_name, buf);
		return -1;
	}

	pdev_full_path = bfmr_malloc((BFMR_DEV_FULL_PATH_MAX_LEN + 1) *
		sizeof(char));
	if (pdev_full_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		goto __out;
	}
	memset((void *)pdev_full_path, 0, ((BFMR_DEV_FULL_PATH_MAX_LEN + 1) *
		sizeof(char)));

	ret = bfmr_get_device_full_path(raw_part_name, pdev_full_path,
		BFMR_DEV_FULL_PATH_MAX_LEN);
	if (ret != 0)
		goto __out;

	ret = bfmr_write_emmc_raw_part(pdev_full_path, offset,
		buf, (unsigned long long)log_len);
	if (ret != 0) {
		ret = -1;
		BFMR_PRINT_ERR("write %s failed ret: %d\n",
			pdev_full_path, ret);
		goto __out;
	}

__out:
	bfmr_free(pdev_full_path);

	return ret;
}

int bfmr_read_log_from_raw_part(char *raw_part_name,
	unsigned long long offset, char *buf, unsigned int buf_size)
{
	int ret = -1;
	char *pdev_full_path = NULL;

	if (!buf_size) {
		BFMR_PRINT_INVALID_PARAMS("raw_part_name: %p\n",
			raw_part_name);
		BFMR_PRINT_INVALID_PARAMS("buf: %p,loglen = %d\n",
			buf, buf_size);
		return 0;
	}

	if (unlikely(raw_part_name == NULL || buf == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("raw_part_name: %p, buf: %p\n",
			raw_part_name, buf);
		return -1;
	}

	pdev_full_path = bfmr_malloc((BFMR_DEV_FULL_PATH_MAX_LEN + 1) *
		sizeof(char));
	if (pdev_full_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		goto __out;
	}
	memset((void *)pdev_full_path, 0, ((BFMR_DEV_FULL_PATH_MAX_LEN + 1) *
		sizeof(char)));

	ret = bfmr_get_device_full_path(raw_part_name, pdev_full_path,
		BFMR_DEV_FULL_PATH_MAX_LEN);
	if (ret != 0)
		goto __out;

	ret = bfmr_read_emmc_raw_part(pdev_full_path, offset,
		buf, (unsigned long long)buf_size);
	if (ret != 0) {
		BFMR_PRINT_ERR("Read %s failed ret: %d\n",
			pdev_full_path, ret);
		goto __out;
	}

__out:
	bfmr_free(pdev_full_path);

	return ret;
}

int bfmr_save_log_to_mem_buffer(char *dst_buf,
	unsigned int dst_buf_len, const char *src_buf, unsigned int log_len)
{
	if (unlikely(dst_buf == NULL || src_buf == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("dst_buf: %p, src_buf: %p\n",
			dst_buf, src_buf);
		return -1;
	}

	memcpy(dst_buf, src_buf, BFMR_MIN(dst_buf_len, log_len));

	return 0;
}

char *bfm_get_bfmr_log_root_path(void)
{
	return (char *)BFM_LOG_ROOT_PATH;
}

char *bfm_get_bfmr_log_uploading_path(void)
{
	return (char *)BFM_LOG_UPLOADING_PATH;
}

char *bfm_get_bfmr_log_part_mount_point(void)
{
	return (char *)BFM_LOG_PART_MOUINT_POINT;
}

static int bfm_save_pwrkey_press_flag_in_bfi(struct bfm_param *pparam)
{
	struct bfi_member_Info *pbfi_info_local = NULL;
	char *bfi_part_full_path = NULL;
	int ret = -1;

	if (unlikely(pparam == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("pparam: %p\n", pparam);
		return -1;
	}

	bfi_part_full_path = bfmr_malloc(
		BFMR_DEV_FULL_PATH_MAX_LEN + 1);
	if (bfi_part_full_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		goto __out;
	}
	memset((void *)bfi_part_full_path, 0, (BFMR_DEV_FULL_PATH_MAX_LEN + 1));

	pbfi_info_local = bfmr_malloc(BFM_BFI_MEMBER_SIZE);
	if (pbfi_info_local == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		goto __out;
	}
	memset((void *)pbfi_info_local, 0, BFM_BFI_MEMBER_SIZE);

	ret = bfm_get_bfi_part_full_path(bfi_part_full_path,
		BFMR_DEV_FULL_PATH_MAX_LEN);
	if (ret != 0) {
		BFMR_PRINT_ERR("failed ot get full path of bfi part\n");
		goto __out;
	}
	pbfi_info_local->errno = pparam->bootfail_errno;
	pbfi_info_local->stage_code = pparam->boot_stage;
	pbfi_info_local->is_log_saved = BFM_BFI_UNSAVE;
	ret = bfmr_write_emmc_raw_part(bfi_part_full_path,
		BFM_BFI_PWK_PRESS_OFF,
		(char *)pbfi_info_local,
		BFM_BFI_MEMBER_SIZE);

__out:
	if (bfi_part_full_path != NULL)
		bfmr_free(bfi_part_full_path);

	if (pbfi_info_local != NULL)
		bfmr_free(pbfi_info_local);
	return ret;
}

int bfm_capture_and_save_do_nothing_bootfail_log(struct bfm_param *param)
{
	int ret = 0;
	bfmr_catch_save_bf_log save_log;

	if (unlikely(param == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("param: %p\n", param);
		return -1;
	}

	save_log = (bfmr_catch_save_bf_log)(param->catch_save_bf_log);
	if (!save_log) {
		BFMR_PRINT_INVALID_PARAMS("bootfail_log_tmp : %p\n",
			save_log);
		return -1;
	}

	switch (param->bootfail_errno) {
	case KERNEL_PRESS10S: {
		/*
		 * save_log_to_raw_part
		 * then forward save log to log part in sbl1 and try_to_recovery
		 */
		param->dst_type = DST_RAW_PART;
		param->recovery_method = FRM_REBOOT;
		param->bootup_time = sched_clock() / (BFM_TIME_S_TO_NS);
		if (param->bootup_time < BFM_BOOT_UP_60_SECOND) {
			BFMR_PRINT_ERR("KERNEL_PRESS10S at %d\n",
				param->bootup_time);
			return ret;
		}
		bfm_save_pwrkey_press_flag_in_bfi(param);
		break;
	}
	default:
	{
		if (bfmr_wait_for_part_mount_without_timeout(
			bfm_get_bfmr_log_part_mount_point()) != 0) {
			BFMR_PRINT_ERR("log part %s is not ready\n",
				BFM_LOG_PART_MOUINT_POINT);
			ret = -1;
		} else {
			param->recovery_method = FRM_DO_NOTHING;
			(void)save_log(param);
		}
		break;
	}
	}

	return ret;
}

static int bfm_get_bfi_part_full_path(char *path_buf,
	unsigned int path_buf_len)
{
	int ret;
	int i;
	char *bfi_part_names[BFM_BFI_PART_MAX_COUNT] = { BFM_BFI_PART_NAME,
		BFM_BFI_PART_NAME };
	int count = ARRAY_SIZE(bfi_part_names);

	if (unlikely(path_buf == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("path_buf: %p\n", path_buf);
		return -1;
	}

	for (i = 0; i < count; i++) {
		memset((void *)path_buf, 0, path_buf_len);

		ret = bfmr_get_device_full_path(bfi_part_names[i],
			path_buf, path_buf_len);
		if (!ret) {
			BFMR_PRINT_ERR("get path for %s success\n",
				bfi_part_names[i]);
			return ret;
		}
	}

	return -1;
} /*lint !e429 */

int bfm_platform_process_boot_fail(struct bfm_param *param)
{
	if (unlikely(param == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("param: %p\n", param);
		return -1;
	}

	set_boot_fail_flag(param->bootfail_errno);
	BFMR_PRINT_ERR("bfm:bootfail happened NO=0x%08x!!\n", param->bootfail_errno);
#ifndef CONFIG_FINAL_RELEASE
	sys_sync();
	msleep(2000);
	sys_sync();
#endif
	panic("bfm:bootfail happened NO=0x%08x!!\n", param->bootfail_errno);
	return 0;
}

int bfm_update_platform_logs(struct bfm_bf_log_info *pbootfail_log_info)
{
	char *dst_file_path = NULL;
	char *src_file_path = NULL;
	char *pdata = NULL;
	long src_file_len;
	mm_segment_t oldfs;
	int ret = -1;

	if (unlikely(pbootfail_log_info == NULL)) {
		BFMR_PRINT_INVALID_PARAMS("pbootfail_log_info: %p\n",
			pbootfail_log_info);
		return ret;
	}

	if (pbootfail_log_info->log_dir_count < 1)
		return ret;

	dst_file_path = bfmr_malloc(BFMR_MAX_PATH);
	if (dst_file_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		goto __out;
	}

	src_file_path = bfmr_malloc(BFMR_MAX_PATH);
	if (src_file_path == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		goto __out;
	}

	memset(dst_file_path, 0, BFMR_MAX_PATH);
	memset(src_file_path, 0, BFMR_MAX_PATH);

	snprintf(dst_file_path, BFMR_MAX_PATH - 1, "%s/%s",
		pbootfail_log_info->bootfail_logs[0].log_dir,
		BFM_DBG_FILE_NAME);
	snprintf(src_file_path, BFMR_MAX_PATH - 1, "%s/%s",
		BFM_LOG_ROOT_PATH, BFM_DBG_FILE_NAME);

	if (!bfmr_is_file_existed(src_file_path))
		goto __out;

	src_file_len = bfmr_get_file_length(src_file_path);
	if (src_file_len <= 0L) {
		BFMR_PRINT_ERR("the length of %s is :%ld\n",
			src_file_path, src_file_len);
		goto __out;
	}

	pdata = bfmr_malloc(src_file_len);
	if (pdata == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		goto __out;
	}

	memset(pdata, 0, src_file_len);
	src_file_len = bfmr_full_read_with_file_path(src_file_path,
		pdata, src_file_len);
	if (src_file_len <= 0) {
		BFMR_PRINT_ERR("read %s failed\n", src_file_path);
		bfmr_free(pdata);
		goto __out;
	}
	bfmr_save_log(pbootfail_log_info->bootfail_logs[0].log_dir,
		BFM_DBG_FILE_NAME, pdata, src_file_len, 0);
	bfmr_free(pdata);
	oldfs = get_fs();
	set_fs(KERNEL_DS); /*lint !e501 */
	sys_unlink(src_file_path);
	set_fs(oldfs);
	ret = 0;
__out:
	if (dst_file_path != NULL)
		bfmr_free(dst_file_path);
	if (src_file_path != NULL)
		bfmr_free(src_file_path);
	return ret;
}

int bfm_platform_process_boot_success(void)
{
#ifdef CONFIG_HUAWEI_STORAGE_ROFA
	storage_rofa_info_clear();
#endif
	return 0;
}

int bfm_is_system_rooted(void)
{
	char *bootlock_value = bfm_get_bootlock_value_from_cmdline();

	if (bootlock_value == NULL)
		return 0;

	if (strcmp(bootlock_value, "bootlock=locked") == 0)
		return 0;

	return 1;
}

int bfm_is_user_sensible_bootfail(enum bfm_errno_code bootfail_errno,
	enum bfr_suggested_method suggested_recovery_method)
{
	return (suggested_recovery_method == DO_NOTHING) ? 0 : 1;
}

char *bfm_get_bl1_bootfail_log_name(void)
{
	return BFM_BL1_BF_LOG_NAME;
}

char *bfm_get_bl2_bootfail_log_name(void)
{
	return BFM_BL2_BF_LOG_NAME;
}

char *bfm_get_kernel_bootfail_log_name(void)
{
	return BFM_KERNEL_BF_LOG_NAME;
}

char *bfm_get_ramoops_bootfail_log_name(void)
{
	return BFM_RAMOOPS_BF_LOG_NAME;
}

char *bfm_get_platform_name(void)
{
	return "mtk";
}

unsigned int bfm_get_dfx_log_length(void)
{
	return 0x1000;
}

char *bfm_get_raw_part_name(void)
{
	return BFM_RAW_PART_NAME;
}

int bfm_get_raw_part_offset(void)
{
	return BFM_RAW_PART_OFFSET;
}

static char *get_file_name(char *file_full_path)
{
	char *ptemp = NULL;

	if (unlikely((file_full_path == NULL))) {
		BFMR_PRINT_INVALID_PARAMS("file_full_path: %p\n",
			file_full_path);
		return NULL;
	}

	ptemp = strrchr(file_full_path, '/');
	if (ptemp == NULL)
		return file_full_path;

	return (ptemp + 1);
}

void bfmr_alloc_and_init_raw_log_info(struct bfm_param *pparam,
	struct bfmr_log_dst *pdst)
{
	struct bfm_record_Info *brit = NULL;

	pparam->log_save_context = bfmr_malloc(sizeof(struct bfm_record_Info));
	if (pparam->log_save_context == NULL)
		return;

	memset(pparam->log_save_context, 0, sizeof(struct bfm_record_Info));
	brit = (struct bfm_record_Info *)(pparam->log_save_context);

	brit->magic = BFM_RAW_LOG_MAGIC;
	brit->errno = pparam->bootfail_errno;
	brit->boot_stage = pparam->boot_stage;
	brit->bf_time = pparam->bootfail_time;
	brit->sugst_rcv_method = pparam->suggested_recovery_method;
	brit->rcv_method = pparam->recovery_method;

	strncpy(brit->log_dir, pparam->bootfail_log_dir, BFMR_SIZE_128 - 1);
	brit->total_log_lenth += sizeof(struct bfm_record_Info);
	pdst->dst_info.raw_part.offset += sizeof(struct bfm_record_Info);
	BFMR_PRINT_KEY_INFO("brit->total_log_lenth %d\n",
		brit->total_log_lenth);
}

void bfmr_save_and_free_raw_log_info(struct bfm_param *pparam)
{
	struct bfm_record_Info *brit = NULL;

	if (pparam->log_save_context == NULL)
		return;

	brit = (struct bfm_record_Info *)(pparam->log_save_context);
	bfmr_save_log_to_raw_part(BFM_RAW_PART_NAME,
		BFM_RAW_PART_OFFSET,
		(char *)brit, sizeof(struct bfm_record_Info));

	BFMR_PRINT_KEY_INFO("brit->total_log_lenth %d\n",
		brit->total_log_lenth);
	bfmr_free(pparam->log_save_context);
	pparam->log_save_context = NULL;
}

void bfmr_update_raw_log_info(struct bfmr_log_src *psrc,
	struct bfmr_log_dst *pdst, unsigned int bytes_read)
{
	struct bfm_record_Info *brit = NULL;

	if (!psrc || !psrc->log_save_context || !pdst)
		return;

	BFMR_PRINT_KEY_INFO("start logtype %d\n", psrc->log_type);
	brit = (struct bfm_record_Info *)(psrc->log_save_context);

	/* get file name */
	switch (psrc->log_type) {
	case LOG_TYPE_BOOTLOADER_1:
		strncpy(brit->log_name[psrc->log_type],
			BFM_BL1_BF_LOG_NAME,
			BFMR_SIZE_128 - 1);
		break;
	case LOG_TYPE_BOOTLOADER_2:
		strncpy(brit->log_name[psrc->log_type],
			BFM_BL2_BF_LOG_NAME,
			BFMR_SIZE_128 - 1);
		break;
	case LOG_TYPE_TEXT_KMSG:
		strncpy(brit->log_name[psrc->log_type],
			BFM_KERNEL_BF_LOG_NAME,
			BFMR_SIZE_128 - 1);
		break;
	case LOG_TYPE_RAMOOPS:
		strncpy(brit->log_name[psrc->log_type],
			BFM_RAMOOPS_BF_LOG_NAME,
			BFMR_SIZE_128 - 1);
		break;
	case LOG_TYPE_BETA_APP_LOGCAT:
		strncpy(brit->log_name[psrc->log_type],
			BFM_LOGCAT_FILE_NAME, BFMR_SIZE_128 - 1);
		break;
	case LOG_TYPE_FIXED_FRAMEWORK_BOOTFAIL_LOG:
		strncpy(brit->log_name[psrc->log_type],
			BFM_FRAMEWORK_BOOTFAIL_LOG_FILE_NAME,
			BFMR_SIZE_128 - 1);
		break;
	case LOG_TYPE_CRITICAL_PROCESS_CRASH:
	case LOG_TYPE_VM_TOMBSTONES:
	case LOG_TYPE_VM_CRASH:
	case LOG_TYPE_VM_WATCHDOG:
	case LOG_TYPE_NORMAL_FRAMEWORK_BOOTFAIL_LOG:
	case LOG_TYPE_BFM_BFI_LOG:
	case LOG_TYPE_BFM_RECOVERY_LOG:
		strncpy(brit->log_name[psrc->log_type],
			get_file_name(psrc->src_log_file_path),
			BFMR_SIZE_128 - 1);
		break;
	case LOG_TYPE_BFMR_TEMP_BUF:
	default:
		BFMR_PRINT_ERR("Invalid log type: %d\n",
			(int)(psrc->log_type));
		break;
	}

	/*  update relation struct  */
	pdst->dst_info.raw_part.offset += bytes_read;
	brit->log_lenth[psrc->log_type] = bytes_read;

	brit->total_log_lenth += bytes_read;
	BFMR_PRINT_KEY_INFO("brit->total_log_lenth %d\n",
		brit->total_log_lenth);
}

void bfmr_copy_data_from_dfx_to_bfmr_tmp_buffer(void)
{
	BFMR_PRINT_KEY_INFO("do nothing on mtk platform\n");
}

int bfm_chipsets_init(struct bfm_chipsets_init_param *param)
{
	if (unlikely((param == NULL))) {
		BFMR_PRINT_KEY_INFO("param is NULL\n");
		return -1;
	}

	BFMR_PRINT_ENTER();

	memset((void *)&bf_log_saving_param, 0,
		sizeof(struct bfm_log_save_param));
	memcpy((void *)&bf_log_saving_param,
		(void *)&param->log_saving_param,
		sizeof(struct bfm_log_save_param));
	BFMR_PRINT_EXIT();

	return 0;
}
