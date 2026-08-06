/*
 * rdr_recovery_exception.c
 *
 * blackbox save recovery log  moudle
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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
#include "rdr_recovery_exception.h"
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/rtc.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/of.h>
#include <linux/ctype.h>
#include <linux/printk.h>

#include <linux/hisi/hisi_bootup_keypoint.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/util.h>
#include <linux/hisi/partition_ap_kernel.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <pr_log.h>
#include <linux/hisi/hisi_pstore.h>

#include <partition_macro.h>
#include <securec.h>
#include <bl31_platform_memory_def.h>

#include "platform_ap/rdr_hisi_ap_adapter.h"
#include "platform_ap/rdr_hisi_ap_exception_logsave.h"
#include "../mntn_filesys.h"

#include "rdr_inner.h"
#include "rdr_print.h"
#include "rdr_field.h"

#define PR_LOG_TAG BLACKBOX_TAG

static int recovery_create_log_path(char *path, char *date, u32 data_len)
{
	int ret;
	static int number = 1;

	BB_PRINT_START();
	if (!check_himntn(HIMNTN_GOBAL_RESETLOG))
		return -1;

	if (path == NULL || date == NULL) {
		BB_PRINT_ERR("invalid  parameter\n");
		BB_PRINT_END();
		return -1;
	}

	if (data_len < DATATIME_MAXLEN) {
		BB_PRINT_ERR("invalid  len\n");
		return -1;
	}

	(void)memset_s(date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);

	ret = snprintf_s(date, DATATIME_MAXLEN, DATATIME_MAXLEN - 1, "%s-%08lld",
		 rdr_get_timestamp(), rdr_get_tick() + number);
	if (ret < 0) {
		BB_PRINT_ERR("%s():%d:snprintf_s fail!\n", __func__, __LINE__);
		return -1;
	}

	number++;
	(void)memset_s(path, PATH_MAXLEN, 0, PATH_MAXLEN);
	ret = snprintf_s(path, PATH_MAXLEN, PATH_MAXLEN - 1, "%s%s/", RECOVERY_LOG_PATH, date);
	if (ret < 0) {
		BB_PRINT_ERR("%s():%d:snprintf_s fail!\n", __func__, __LINE__);
		return -1;
	}

	ret = rdr_create_dir(path);
	BB_PRINT_END();
	return ret;
}

static void rdr_save_history_log_to_splash2(struct rdr_exception_info_s *p, char *date, u32 datelen,
				u32 bootup_keypoint)
{
	int ret;
	char buf[HISTORY_LOG_SIZE];
	struct kstat historylog_stat;
	char local_path[PATH_MAXLEN];
	char *reboot_from_ap = NULL;
	char *subtype_name = NULL;

	if (p == NULL || date == NULL) {
		BB_PRINT_ERR("%s():%d:invalid parameter, p_exce_info or date is null\n",
			__func__, __LINE__);
		return;
	}

	BB_PRINT_START();
	if (datelen < (strlen(date) + 1))
		date[DATATIME_MAXLEN - 1] = '\0';
	(void)memset_s(buf, HISTORY_LOG_SIZE, 0, HISTORY_LOG_SIZE);

	if (p->e_reset_core_mask & RDR_AP)
		reboot_from_ap = "true";
	else
		reboot_from_ap = "false";

	subtype_name = rdr_get_subtype_name(p->e_exce_type, p->e_exce_subtype);
	if (subtype_name != NULL)
		ret = snprintf_s(buf, HISTORY_LOG_SIZE, HISTORY_LOG_SIZE - 1,
			"system exception core [%s], reason [%s:%s], time [%s], sysreboot [%s], bootup_keypoint [%u], category [%s]\n",
			rdr_get_exception_core(p->e_from_core),
			rdr_get_exception_type(p->e_exce_type),
			subtype_name,
			date,
			reboot_from_ap,
			bootup_keypoint,
			rdr_get_category_name(p->e_exce_type, p->e_exce_subtype));
	else
		ret = snprintf_s(buf, HISTORY_LOG_SIZE, HISTORY_LOG_SIZE - 1,
			"system exception core [%s], reason [%s], time [%s], sysreboot [%s], bootup_keypoint [%u], category [%s]\n",
			rdr_get_exception_core(p->e_from_core),
			rdr_get_exception_type(p->e_exce_type),
			date,
			reboot_from_ap,
			bootup_keypoint,
			rdr_get_category_name(p->e_exce_type, p->e_exce_subtype));
	if (ret < 0)
		return;

	(void)memset_s(local_path, PATH_MAXLEN, 0, PATH_MAXLEN);
	ret = snprintf_s(local_path, PATH_MAXLEN, PATH_MAXLEN - 1, "%s/%s", RECOVERY_LOG_PATH, "history.log");
	if (ret < 0)
		return;

	if (vfs_stat(local_path, &historylog_stat) == 0 && historylog_stat.size > RECOVERY_HISTORY_LOG_MAX)
		SYS_UNLINK(local_path); /* delete history.log */

	(void)rdr_savebuf2fs(RECOVERY_LOG_PATH, "history.log", buf, strlen(buf), 1);

	BB_PRINT_END();
}

static int rdr_save_last_kmsg_and_dmesg(const char *log_path, int tmp_cnt, char *pbuff)
{
	int i, ret;
	char dst_str[NEXT_LOG_PATH_LEN];
	char fullpath_arr[LOG_PATH_LEN];

	for (i = 0; i < tmp_cnt; i++) {
		/* generated absolute paths of source file */
		(void)memset_s((void *)fullpath_arr, sizeof(fullpath_arr), 0, sizeof(fullpath_arr));
		ret = strncat_s(fullpath_arr, LOG_PATH_LEN, PSTORE_PATH, ((LOG_PATH_LEN - 1) - strlen(fullpath_arr)));
		if (ret != EOK) {
			BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
			return -1;
		}
		BB_PRINT_PN("file is [%s]\n", (pbuff + (i * MNTN_FILESYS_PURE_DIR_NAME_LEN)));
		ret = strncat_s(fullpath_arr, LOG_PATH_LEN,
				(const char *)(pbuff + (i * MNTN_FILESYS_PURE_DIR_NAME_LEN)),
				((LOG_PATH_LEN - 1) - strlen(fullpath_arr)));
		if (ret != EOK) {
			BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
			return -1;
		}

		/* If not the console, the destination file is not last_kmsg, which is dmesg-ramoops-x */
		if (strncmp((const char *)(pbuff + (i * MNTN_FILESYS_PURE_DIR_NAME_LEN)),
				"console-ramoops",
				strlen("console-ramoops")) == 0) {
			(void)memset_s((void *)dst_str, NEXT_LOG_PATH_LEN, 0, NEXT_LOG_PATH_LEN);
			ret = strncat_s(dst_str, NEXT_LOG_PATH_LEN, log_path, ((NEXT_LOG_PATH_LEN - 1) - strlen(dst_str)));
			if (ret != EOK) {
				BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
				return -1;
			}

			ret = strncat_s(dst_str, NEXT_LOG_PATH_LEN, "last_kmsg", ((NEXT_LOG_PATH_LEN - 1) - strlen(dst_str)));
			if (ret != EOK) {
				BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
				return -1;
			}
		} else if (strncmp((const char *)(pbuff + (i * MNTN_FILESYS_PURE_DIR_NAME_LEN)),
				"dmesg-ramoops",
				strlen("dmesg-ramoops")) == 0) {
			(void)memset_s((void *)dst_str, NEXT_LOG_PATH_LEN, 0, NEXT_LOG_PATH_LEN);
			ret = strncat_s(dst_str, NEXT_LOG_PATH_LEN, log_path, ((NEXT_LOG_PATH_LEN - 1) - strlen(dst_str)));
			if (ret != EOK) {
				BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
				return -1;
			}

			ret = strncat_s(dst_str, NEXT_LOG_PATH_LEN,
					(const char *)(pbuff + (i * MNTN_FILESYS_PURE_DIR_NAME_LEN)),
					((NEXT_LOG_PATH_LEN - 1) - strlen(dst_str)));
			if (ret != EOK) {
				BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
				return -1;
			}
		} else {
			continue;
		}

		/* Copy the content of the source file to the destination file */
		ret = rdr_copy_file_apend(dst_str, fullpath_arr);
		if (ret) {
			BB_PRINT_ERR("rdr_copy_file_apend [%s] fail, ret = [%d]\n", fullpath_arr, ret);
			return -1;
		}
	}

	return 0;
}

/*
 * Description:    copy file from /proc/balong/pstore/ to dst_dir_str
 */
static void rdr_save_pstore_to_splash2(void)
{
	int ret, tmp_cnt;
	char *pbuff = NULL;
	char log_path[PATH_MAXLEN];
	char date[DATATIME_MAXLEN];
	struct rdr_exception_info_s temp;

	if (rdr_wait_partition("/splash2/recovery", RDR_WAIT_PARTITION_TIME) != 0) {
		BB_PRINT_PN("%s():%d:splash2  is not ready\n", __func__, __LINE__);
		return;
	}

	(void)memset_s(log_path, PATH_MAXLEN, 0, PATH_MAXLEN);
	(void)memset_s(date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);
	(void)memset_s(&temp, sizeof(temp), 0, sizeof(temp));

	hisi_create_pstore_entry();
	if (rdr_wait_partition(PSTORE_PATH, RECOVERY_WAIT_TIME)) {
		BB_PRINT_ERR("pstore is not ready\n");
		return;
	}

	tmp_cnt = MNTN_FILESYS_MAX_CYCLE * MNTN_FILESYS_PURE_DIR_NAME_LEN;
	/* Apply for a memory for storing all files name in the /sys/fs/pstore/ directory */
	pbuff = kzalloc(tmp_cnt, GFP_KERNEL);
	if (!pbuff) {
		BB_PRINT_ERR("kmalloc tmp_cnt fail, tmp_cnt = [%d]\n", tmp_cnt);
		return;
	}

	/*
	 * Invoke the interface to save the names of all files in
	 * the /sys/fs/pstore/ directory to the pbuff
	 */
	tmp_cnt = mntn_filesys_dir_list(PSTORE_PATH, pbuff, tmp_cnt, DT_REG);
	if (tmp_cnt <= 0) {
		kfree(pbuff);
		return;
	}

	temp.e_from_core = RDR_AP;
	temp.e_reset_core_mask = RDR_AP;
	temp.e_exce_type = rdr_get_reboot_type();
	temp.e_exce_subtype = rdr_get_exec_subtype_value();

	if (recovery_create_log_path(&log_path[0], &date[0], DATATIME_MAXLEN) != 0) {
		BB_PRINT_ERR("recovery_create_dfxlog_path fail\n");
		kfree(pbuff);
		return;
	}

	rdr_save_history_log_to_splash2(&temp, &date[0], DATATIME_MAXLEN, get_boot_keypoint());

	ret = rdr_save_last_kmsg_and_dmesg(log_path, tmp_cnt, pbuff);
	if (ret)
		BB_PRINT_ERR("rdr_save_pstore_to_splash2 fail\n");

	hisi_remove_pstore_entry();
	kfree(pbuff);
}

/*
 * Description:    save dfx's ddr_buffer to file
 * Return:         0:success
 */
#ifdef CONFIG_PARTITION_TABLE
static int recovery_save_dfxbuffer_to_file(struct dfx_head_info *dfx_head_info)
{
	char *buf = NULL;
	int last_number;
	struct rdr_exception_info_s temp;
	struct every_number_info *every_number_info = NULL;
	struct dfx_save_file dfx_save_file;

	(void)memset_s(&dfx_save_file, sizeof(struct dfx_save_file), 0, sizeof(struct dfx_save_file));
	(void)memset_s(&temp, sizeof(temp), 0, sizeof(temp));
	BB_PRINT_START();

	if (dfx_head_info->need_save_number > TOTAL_NUMBER) {
		BB_PRINT_ERR("need_save_number error %d\n", dfx_head_info->need_save_number);
		dfx_head_info->need_save_number = TOTAL_NUMBER;
	}
	last_number = (dfx_head_info->cur_number - dfx_head_info->need_save_number + TOTAL_NUMBER) % TOTAL_NUMBER;

	BB_PRINT_ERR("[%s], dfx_head_info->need_save_number = %d\n", __func__, dfx_head_info->need_save_number);
	while (dfx_head_info->need_save_number != 0) {
		buf = (char *)dfx_head_info + dfx_head_info->every_number_addr[last_number];
		every_number_info = (struct every_number_info *)buf;

		temp.e_from_core = RDR_AP;
		temp.e_reset_core_mask = RDR_AP;
		temp.e_exce_type = every_number_info->reboot_type;
		temp.e_exce_subtype = every_number_info->exce_subtype;

		(void)memset_s(dfx_save_file.path, PATH_MAXLEN, 0, PATH_MAXLEN);
		(void)memset_s(dfx_save_file.date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);

		if (every_number_info->last_kmsg_size != 0) {
			if (recovery_create_log_path(&dfx_save_file.path[0], &dfx_save_file.date[0], DATATIME_MAXLEN) != 0) {
				BB_PRINT_ERR("recovery_create_dfxlog_path fail\n");
				return -1;
			}
			dfx_save_file.offset = (void *)every_number_info + every_number_info->last_kmsg_start_addr;
			dfx_save_file.size = (u32)every_number_info->last_kmsg_size;
			if (!check_dfx_head_valid(dfx_head_info, dfx_save_file.offset, dfx_save_file.size))
				(void)rdr_savebuf2fs(dfx_save_file.path, "dfx_last_kmsg",
				    dfx_save_file.offset, dfx_save_file.size, 0);

			rdr_save_history_log_to_splash2(&temp, &dfx_save_file.date[0], DATATIME_MAXLEN,
				every_number_info->bootup_keypoint);
		}

		dfx_head_info->need_save_number--;
		last_number = (last_number + 1 + TOTAL_NUMBER) % TOTAL_NUMBER;
	}

	return 0;
}

/*
 * Description: save dfx partition's log to file
 */
static void rdr_save_dfxlog_to_splash2(void)
{
	char p_name[RDR_PNAME_SIZE];
	void *buf = NULL;
	void *dfx_buf = NULL;
	void *dfx_buf_temp = NULL;
	int ret, need_read_size;
	int fd_dfx;
	int iret;
	int str_len;

	BB_PRINT_START();
	if (rdr_wait_partition("/splash2/recovery", RDR_WAIT_PARTITION_TIME) != 0) {
		BB_PRINT_PN("%s():%d:splash2  is not ready\n", __func__, __LINE__);
		return;
	}

	buf = kzalloc(SZ_4K, GFP_KERNEL);
	if (buf == NULL) {
		BB_PRINT_ERR("%s():%d:kzalloc buf fail\n", __func__, __LINE__);
		return;
	}

	dfx_buf = vmalloc(DFX_USED_SIZE);
	if (dfx_buf == NULL) {
		BB_PRINT_ERR("%s():%d:vmalloc dfx_buf fail\n", __func__, __LINE__);
		kfree(buf);
		return;
	}

	ret = flash_find_ptn_s(PART_DFX, buf, SZ_4K);
	if (ret != 0) {
		BB_PRINT_ERR("%s():%d:flash_find_ptn_s fail[%d]\n", __func__, __LINE__, ret);
		goto out;
	}

	(void)memset_s(p_name, RDR_PNAME_SIZE, 0, RDR_PNAME_SIZE);
	iret = memcpy_s(p_name, RDR_PNAME_SIZE, buf, RDR_PNAME_SIZE - 1);
	if (iret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail[%d]\n", __func__, __LINE__, iret);
		goto out;
	}
	p_name[RDR_PNAME_SIZE - 1] = '\0';

	str_len = strlen(p_name) - strlen(PART_DFX);
	if (str_len < 0 || strncmp(p_name + str_len, PART_DFX, strlen(PART_DFX)) != 0) {
		BB_PRINT_ERR("%s():%d:p_name err[%s]\n", __func__, __LINE__, p_name);
		goto out;
	}

	fd_dfx = SYS_OPEN(p_name, O_RDONLY, FILE_LIMIT);
	if (fd_dfx < 0) {
		BB_PRINT_ERR("%s():%d:open fail[%d]\n", __func__, __LINE__, fd_dfx);
		goto out;
	}

	dfx_buf_temp = dfx_buf;
	need_read_size = DFX_USED_SIZE;
	(void)memset_s((void *)dfx_buf_temp, DFX_USED_SIZE, 0, DFX_USED_SIZE);

	while (need_read_size > 0) {
		ret = SYS_READ(fd_dfx, buf, SZ_4K);
		if (ret < 0) {
			BB_PRINT_ERR("%s():%d:read fail[%d]\n", __func__, __LINE__, ret);
			goto close;
		}

		if (ret == 0)
			break;

		iret = memcpy_s((void *)dfx_buf_temp, need_read_size, (const void *)buf, ret);
		/* In the last loop, need_read_size may be smaller than ret, memcpy_s failure is normal, don't need return */
		if (iret != EOK)
			BB_PRINT_ERR("%s():%d:memcpy_s fail[%d]\n", __func__, __LINE__, iret);

		dfx_buf_temp = dfx_buf_temp + ret;
		need_read_size -= ret;
	}
	SYS_CLOSE(fd_dfx);
	ret = recovery_save_dfxbuffer_to_file((struct dfx_head_info *)dfx_buf);
	if (ret) {
		BB_PRINT_ERR("%s():%d:save_dfxbuffer_to_file fail[%d]\n", __func__, __LINE__, ret);
		goto out;
	}

	fd_dfx = SYS_OPEN(p_name, O_WRONLY, FILE_LIMIT);
	if (fd_dfx < 0) {
		BB_PRINT_ERR("%s():%d:open fail[%d]\n", __func__, __LINE__, fd_dfx);
		goto out;
	}

	ret = SYS_WRITE(fd_dfx, (void *)dfx_buf, SZ_4K);
	if (ret <= 0) {
		BB_PRINT_ERR("%s():%d:write fail[%d]\n", __func__, __LINE__, ret);
		goto close;
	}

close:
	SYS_CLOSE(fd_dfx);
out:
	vfree(dfx_buf);
	kfree(buf);
}
#else
static inline void rdr_save_dfxlog_to_splash2(void)
{
	return;
}
#endif

void rdr_recovery_exce(void)
{
	int ret;
	unsigned int reboot_type;
	char log_path[PATH_MAXLEN];

	(void)memset_s(log_path, PATH_MAXLEN, 0, PATH_MAXLEN);
	ret = memcpy_s(log_path, PATH_MAXLEN, RECOVERY_LOG_PATH, strlen(RECOVERY_LOG_PATH));
	if (ret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail[%d]\n", __func__, __LINE__, ret);
		return;
	}

	reboot_type = rdr_get_reboot_type();
	BB_PRINT_PN("%s reboot_type = [0x%x]\n", __func__, reboot_type);

	rdr_save_dfxlog_to_splash2();
	BB_PRINT_PN("%s rdr_save_dfxlog_to_splash2 end\n", __func__);

	if (reboot_type >= REBOOT_REASON_LABEL1 && reboot_type < REBOOT_REASON_LABEL3) {
		rdr_save_pstore_to_splash2();
		BB_PRINT_PN("%s rdr_save_pstore_to_splash2 end\n", __func__);
	}
	/* at most 5 logs */
	mntn_rm_old_log(log_path, MAX_LOG_NUM);
}

