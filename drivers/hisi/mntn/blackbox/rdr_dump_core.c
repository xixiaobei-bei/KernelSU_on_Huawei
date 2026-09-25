/*
 * rdr_dump_core.c
 *
 * blackbox. (kernel run data recorder.)
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
#include "rdr_inner.h"
#include "rdr_print.h"
#include "rdr_field.h"
#include <hisi_partition.h>
#include <linux/hisi/kirin_partition.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <securec.h>
#include <linux/hisi/hisi_log.h>
#include <linux/hisi/hisi_hw_diag.h>
#include <linux/hisi/bl31_log.h>
#include <bl31_platform_memory_def.h>
#define HISI_LOG_TAG HISI_BLACKBOX_TAG
#define DFX_LOG_SIZE_SHIFT 4

u32 dfx_size_tbl[DFX_MAX_MODULE];
u32 dfx_addr_tbl[DFX_MAX_MODULE];

/*
 * func description:
 *  save exce info to history.log
 * return
 *  !0   fail
 *  == 0 success
 */
int rdr_save_history_log(struct rdr_exception_info_s *p, char *date, u32 datelen,
				bool is_save_done, u32 bootup_keypoint)
{
	int ret = 0;
	char buf[HISTORY_LOG_SIZE];
	struct kstat historylog_stat;
	char local_path[PATH_MAXLEN];
	char *reboot_from_ap = NULL;
	char *subtype_name = NULL;

	if (p == NULL || date == NULL) {
		BB_PRINT_ERR("%s():%d:invalid parameter, p_exce_info or date is null\n",
			__func__, __LINE__);
		return -1;
	}
	if (!check_himntn(HIMNTN_GOBAL_RESETLOG))
		return 0;
	BB_PRINT_START();
	if (datelen < (strlen(date) + 1))
		date[DATATIME_MAXLEN - 1] = '\0';
	(void)memset_s(buf, HISTORY_LOG_SIZE, 0, HISTORY_LOG_SIZE);

	if (p->e_reset_core_mask & RDR_AP)
		reboot_from_ap = "true";
	else
		reboot_from_ap = "false";
	/*
	 * The record is normal if in simple reset process.
	 * Otherwise, the string of last_save_not_done needs to be added.
	 */
	subtype_name = rdr_get_subtype_name(p->e_exce_type, p->e_exce_subtype);

	if (is_save_done) {
		if (subtype_name != NULL)
			(void)snprintf(buf, HISTORY_LOG_SIZE,
				"system exception core [%s], reason [%s:%s], time [%s], sysreboot [%s], bootup_keypoint [%u], category [%s]\n",
				rdr_get_exception_core(p->e_from_core),
				rdr_get_exception_type(p->e_exce_type),
				subtype_name,
				date,
				reboot_from_ap,
				bootup_keypoint,
				rdr_get_category_name(p->e_exce_type, p->e_exce_subtype));
		else
			(void)snprintf(buf, HISTORY_LOG_SIZE,
				"system exception core [%s], reason [%s], time [%s], sysreboot [%s], bootup_keypoint [%u], category [%s]\n",
				rdr_get_exception_core(p->e_from_core),
				rdr_get_exception_type(p->e_exce_type),
				date,
				reboot_from_ap,
				bootup_keypoint,
				rdr_get_category_name(p->e_exce_type, p->e_exce_subtype));
	} else {
		if (subtype_name != NULL)
			(void)snprintf(buf, HISTORY_LOG_SIZE,
				"system exception core [%s], reason [%s:%s], time [%s][last_save_not_done], sysreboot [%s], bootup_keypoint [%u], category [%s]\n",
				rdr_get_exception_core(p->e_from_core),
				rdr_get_exception_type(p->e_exce_type),
				subtype_name,
				date,
				reboot_from_ap,
				bootup_keypoint,
				rdr_get_category_name(p->e_exce_type, p->e_exce_subtype));
		else
			(void)snprintf(buf, HISTORY_LOG_SIZE,
				"system exception core [%s], reason [%s], time [%s][last_save_not_done], sysreboot [%s], bootup_keypoint [%u], category [%s]\n",
				rdr_get_exception_core(p->e_from_core),
				rdr_get_exception_type(p->e_exce_type),
				date,
				reboot_from_ap,
				bootup_keypoint,
				rdr_get_category_name(p->e_exce_type, p->e_exce_subtype));
	}

	memset(local_path, 0, PATH_MAXLEN);
	(void)snprintf(local_path, PATH_MAXLEN, "%s/%s", PATH_ROOT, "history.log");

	if (vfs_stat(local_path, &historylog_stat) == 0 && historylog_stat.size > HISTORY_LOG_MAX)
		sys_unlink(local_path); /* delete history.log */

	if (vfs_stat(PATH_ROOT, &historylog_stat) != 0) {
		ret = rdr_dump_init(NULL);
		if (ret) {
			BB_PRINT_ERR("%s():rdr_create_dir fail\n", __func__);
			return ret;
		}
	}

	(void)rdr_savebuf2fs(PATH_ROOT, "history.log", buf, strlen(buf), 1);
#ifdef CONFIG_HISI_HW_DIAG
	hisi_hw_diaginfo_record(date);
#endif

	BB_PRINT_END();
	return ret;
}

int rdr_save_history_log_for_undef_exception(struct rdr_syserr_param_s *p)
{
	char buf[HISTORY_LOG_SIZE];
	struct kstat historylog_stat;
	char local_path[PATH_MAXLEN];

	if (p == NULL) {
		BB_PRINT_ERR("exception: NULL\n");
		return -1;
	}
	if (!check_himntn(HIMNTN_GOBAL_RESETLOG))
		return 0;
	BB_PRINT_START();
	(void)memset_s(buf, HISTORY_LOG_SIZE, 0, HISTORY_LOG_SIZE);
	(void)snprintf(buf, HISTORY_LOG_SIZE,
		"system exception undef. modid[0x%x], arg [0x%x], arg [0x%x]\n",
		p->modid, p->arg1, p->arg2);

	memset(local_path, 0, PATH_MAXLEN);
	(void)snprintf(local_path, PATH_MAXLEN, "%s/%s", PATH_ROOT, "history.log");

	if (vfs_stat(local_path, &historylog_stat) == 0 &&
	    historylog_stat.blksize > HISTORY_LOG_MAX)
		sys_unlink(local_path); /* delete history.log */

	(void)rdr_savebuf2fs(PATH_ROOT, "history.log", buf, strlen(buf), 1);
	BB_PRINT_END();
	return 0;
}


static void rdr_save_logbuf_log(const char *path)
{
	char *src = NULL;
	char *dst = NULL;
	char *buffer = NULL;
	u32 lenth;
	int ret;

	lenth = log_buf_len_get();
	if (lenth == 0) {
		BB_PRINT_ERR("%s():%d:len is zero\n", __func__, __LINE__);
		return;
	}
	src = log_buf_addr_get();
	if (src == NULL) {
		BB_PRINT_ERR("%s():%d:src is null\n", __func__, __LINE__);
		return;
	}

	/* for logbuf write, save before */
	dst = vmalloc(lenth);
	if (dst == NULL) {
		BB_PRINT_ERR("%s():%d:vmalloc buf fail\n", __func__, __LINE__);
		return;
	}

	if (memcpy_s(dst, lenth, src, lenth - 1) != EOK) {
		BB_PRINT_ERR("%s():%d: memcpy_s fail\n", __func__, __LINE__);
		goto err;
	}

	ret = rdr_savebuf2fs(path, "logbuf.bin", dst, lenth, 0);
	if (ret < 0) {
		BB_PRINT_ERR("%s(): rdr_savelogbuf2fs error = %d\n", __func__, ret);
		goto err;
	}

	buffer = (char *)hisirdr_ex_log_first_idx;

	ret = rdr_savebuf2fs(path, "logbuf.bin", buffer, sizeof(lenth), 1);
	if (ret < 0) {
		BB_PRINT_ERR("%s(): rdr_savefirst_idx2fs error = %d\n", __func__, ret);
		goto err;
	}

	buffer = (char *)hisirdr_ex_log_next_idx;

	ret = rdr_savebuf2fs(path, "logbuf.bin", buffer, sizeof(lenth), 1);
	if (ret < 0) {
		BB_PRINT_ERR("%s(): rdr_savenext_idx2fs error = %d\n", __func__, ret);
		goto err;
	}

err:
	vfree(dst);
}

void rdr_save_pstore_log(const struct rdr_exception_info_s *p_exce_info, const char *path)
{
	u32 save_flags;

	if (p_exce_info == NULL || path == NULL) {
		BB_PRINT_ERR("%s(): invalid parameter. p_exce_info or path is null\n", __func__);
		return;
	}

	/* system(ap) reset, save logs in reboot */
	if (p_exce_info->e_reset_core_mask & RDR_AP) {
		BB_PRINT_PN("%s(): system reset, no need to save\n", __func__);
		return;
	}

	save_flags = p_exce_info->e_save_log_flags;
	if (save_flags & RDR_SAVE_BL31_LOG) {
		BB_PRINT_PN("%s(): bl31_log saving\n", __func__);
	}

	if (save_flags & RDR_SAVE_DMESG)
		BB_PRINT_PN("%s(): dmsg saving\n", __func__);

	if (save_flags & RDR_SAVE_CONSOLE_MSG)
		BB_PRINT_PN("%s(): console msg saving\n", __func__);

	if (save_flags & RDR_SAVE_LOGBUF) {
		BB_PRINT_PN("%s(): %s logbuf msg saving\n", __func__, path);
		rdr_save_logbuf_log(path);
	}
}

/*
 * func descripton:
 *  append(save) data to path
 * func args:
 *  char*  path,        path of save file
 *  char*  name,        name of save file
 *  void*  buf,         save data
 *  u32 len,            data lenght
 *  u32 is_append,      determine whether write with append
 * return
 *  >=len fail
 *  ==len success
 */
int rdr_savebuf2fs(const char *logpath, const char *filename,
				const void *buf, u32 len, u32 is_append)
{
	int ret, flags;
	struct file *fp = NULL;
	char path[PATH_MAXLEN];

	BB_PRINT_START();
	if (logpath == NULL || filename == NULL || buf == NULL || len <= 0) {
		BB_PRINT_ERR("invalid  parameter. path:%pK, name:%pK buf:%pK len:0x%x\n",
		     logpath, filename, buf, len);
		ret = -1;
		goto end;
	}

	(void)snprintf(path, PATH_MAXLEN, "%s/%s", logpath, filename);

	flags = O_CREAT | O_RDWR | (is_append ? O_APPEND : O_TRUNC);
	fp = filp_open(path, flags, FILE_LIMIT);
	if (IS_ERR(fp)) {
		BB_PRINT_ERR("%s():create file %s err. fp=0x%pK\n", __func__, path, fp);
		ret = -1;
		goto end;
	}
	vfs_llseek(fp, 0L, SEEK_END);
	ret = vfs_write(fp, buf, len, &(fp->f_pos));
	if (ret != len) {
		BB_PRINT_ERR("%s():write file %s exception with ret %d\n",
			     __func__, path, ret);
		goto close;
	}

	vfs_fsync(fp, 0);
close:
	filp_close(fp, NULL);

	/*
	 * According to the permission requirements,
	 * the hisi_logs directory and subdirectory group are adjusted to root-system.
	 */
	ret = (int)bbox_chown((const char __user *)path, ROOT_UID,
				SYSTEM_GID, false);
	if (ret)
		BB_PRINT_ERR("[%s], chown %s uid [%d] gid [%d] failed err [%d]\n",
		     __func__, path, ROOT_UID, SYSTEM_GID, ret);
end:
	BB_PRINT_END();
	return ret;
}

/*
 * Description:    split bbox.bin to the directory of bbox_split_bin
 * Input:          logpath;base_addr:base address of bbox
 */
static void bbox_save_every_core_data(const char *logpath, char *base_addr)
{
	int ret;
	u32 value, i;
	u32 data[RDR_CORE_MAX];
	struct device_node *np = NULL;
	struct bbox_area_core_data bbox_area_core_data;
	char tmp_logpath[PATH_MAXLEN] = {0};

	bbox_area_core_data.addr = NULL;
	bbox_area_core_data.bbox_area_names = NULL;

	if (logpath == NULL || base_addr == NULL) {
		BB_PRINT_ERR("%s():%d:invalid parameter, logpath or base_addr is null\n",
			__func__, __LINE__);
		return;
	}

	if (bbox_get_every_core_area_info(&value, data, &np, RDR_CORE_MAX)) {
		BB_PRINT_ERR("[%s], bbox_get_every_core_area_info fail\n",
			     __func__);
		return;
	}

	/* create bbox_split_bin path */
	strncat(tmp_logpath, logpath, (unsigned long)(PATH_MAXLEN - 1));
	strncat(tmp_logpath, BBOX_SPLIT_BIN, (unsigned long)((PATH_MAXLEN - 1) - strlen(logpath)));
	if (rdr_create_dir(tmp_logpath)) {
		BB_PRINT_ERR("[%s][%d], create temp logpath failed\n", __func__, __LINE__);
		return;
	}

	bbox_area_core_data.addr = base_addr + rdr_get_pbb_size();
	bbox_area_core_data.size = 0;

	for (i = value - 1; i > 0; i--) {
		bbox_area_core_data.addr -= data[i];
		bbox_area_core_data.size += data[i];
		ret = of_property_read_string_index(np, "bbox_area_names",
			  (int)i, (const char **)&bbox_area_core_data.bbox_area_names);
		if (ret) {
			BB_PRINT_ERR("[%s][%d], of_property_read_string_index fail\n",
					 __func__, __LINE__);
			return;
		}
		BB_PRINT_PN("[%s], size[0x%x], addr[0x%pK], name[%s]\n", __func__,
		    data[i], bbox_area_core_data.addr, bbox_area_core_data.bbox_area_names);

		if (data[i] > 0)
			(void)rdr_savebuf2fs(tmp_logpath, bbox_area_core_data.bbox_area_names,
				  bbox_area_core_data.addr, data[i], 0);
	}

	bbox_area_core_data.addr = base_addr;

	BB_PRINT_PN("[%s], addr[0x%pK], name[%s]\n", __func__,
	    bbox_area_core_data.addr, bbox_area_core_data.bbox_area_names);
	(void)rdr_savebuf2fs(tmp_logpath, BBOX_HEAD_INFO, bbox_area_core_data.addr, RDR_BASEINFO_SIZE, 0);

	/* save AP data info */
	bbox_area_core_data.addr = base_addr + RDR_BASEINFO_SIZE;
	bbox_area_core_data.size = (u32)rdr_get_pbb_size() - (bbox_area_core_data.size + RDR_BASEINFO_SIZE);
	ret = of_property_read_string_index(np, "bbox_area_names", 0,
	    (const char **)&bbox_area_core_data.bbox_area_names);
	if (ret) {
		BB_PRINT_ERR("[%s][%d], of_property_read_string_index fail\n",
				 __func__, __LINE__);
		return;
	}

	BB_PRINT_PN("[%s], size[0x%x], addr[0x%pK], name[%s]\n", __func__,
	    bbox_area_core_data.size, bbox_area_core_data.addr, bbox_area_core_data.bbox_area_names);
	(void)rdr_savebuf2fs(tmp_logpath, bbox_area_core_data.bbox_area_names,
		  bbox_area_core_data.addr, bbox_area_core_data.size, 0);
}

void rdr_save_cur_baseinfo(const char *logpath)
{
	BB_PRINT_START();
	if (logpath == NULL) {
		BB_PRINT_ERR("logpath is null");
		BB_PRINT_END();
		return;
	}
	/* save pbb to fs */
	(void)rdr_savebuf2fs(logpath, RDR_BIN, rdr_get_pbb(), rdr_get_pbb_size(), 0);
	bbox_save_every_core_data(logpath, (char *)rdr_get_pbb());

	BB_PRINT_END();
}

void rdr_save_last_baseinfo(const char *logpath)
{
	BB_PRINT_START();
	if (logpath == NULL) {
		BB_PRINT_ERR("logpath is null");
		BB_PRINT_END();
		return;
	}
	/* save pbb to fs */
	(void)rdr_savebuf2fs(logpath, RDX_BIN, rdr_get_tmppbb(), rdr_get_pbb_size(), 0);
	bbox_save_every_core_data(logpath, (char *)rdr_get_tmppbb());

	bbox_cleartext_proc(logpath, (char *)rdr_get_tmppbb());
	BB_PRINT_END();
}

/*
 * Description:    get_system_time
 * Return:         system_time
 */
static u64 get_system_time(void)
{
	struct timeval tv = {0};

	do_gettimeofday(&tv);

	return (u64)tv.tv_sec;
}

/*
 * Description:    judge whether need save dfx to file
 * Return:         true:need
 */
bool is_need_save_dfx2file(void)
{
#ifdef CONFIG_HISI_PARTITION
	char *buf = NULL;
	int ret, cnt;
	int fd_dfx = -1;
	char p_name[RDR_PNAME_SIZE];
	bool is_save_dfx2file = false;
	struct dfx_head_info *dfx_head_info = NULL;

	if (!check_himntn(HIMNTN_DFXPARTITION_TO_FILE)) {
		BB_PRINT_PN("%s():%d:switch is close\n", __func__, __LINE__);
		goto out;
	}

	if (rdr_wait_partition("/data/lost+found", RDR_WAIT_PARTITION_TIME) != 0) {
		BB_PRINT_PN("%s():%d:data is not ready\n", __func__, __LINE__);
		goto out;
	}

	buf = kzalloc(SZ_4K, GFP_KERNEL);
	if (buf == NULL) {
		BB_PRINT_ERR("%s():%d:kzalloc buf fail\n", __func__, __LINE__);
		goto out;
	}

	ret = flash_find_ptn_s(PART_DFX, buf, SZ_4K);
	if (ret != 0) {
		BB_PRINT_ERR("%s():%d:flash_find_ptn_s fail[%d]\n", __func__, __LINE__, ret);
		kfree(buf);
		goto out;
	}

	(void)memset_s(p_name, RDR_PNAME_SIZE, 0, RDR_PNAME_SIZE);
	strncpy(p_name, buf, RDR_PNAME_SIZE);
	p_name[RDR_PNAME_SIZE - 1] = '\0';

	fd_dfx = sys_open(p_name, O_RDONLY, FILE_LIMIT);
	if (fd_dfx < 0) {
		BB_PRINT_ERR("%s():%d:open %s fail[%d]\n", __func__, __LINE__, p_name, fd_dfx);
		kfree(buf);
		goto out;
	}

	memset(buf, 0, SZ_4K);
	cnt = sys_read(fd_dfx, buf, SZ_4K);
	if (cnt < 0) {
		BB_PRINT_ERR("%s():%d:read fail[%d]\n", __func__, __LINE__, cnt);
		goto close;
	}

	dfx_head_info = (struct dfx_head_info *)buf;
	if (dfx_head_info->magic == DFX_MAGIC_NUMBER &&
	    dfx_head_info->need_save_number > 0 &&
	    dfx_head_info->need_save_number <= TOTAL_NUMBER)
		is_save_dfx2file = true;

close:
	sys_close(fd_dfx);
	kfree(buf);
out:
	return is_save_dfx2file;
#else
	return false;
#endif
}

/*
 * Description:    judge need_save_dfxbuffer2file
 * Input:          reboot_type:record the type of reboot, bootup_keypoint
 * Return:         true:need save; false:not need save
 */
static bool need_save_dfxbuffer2file(u64 reboot_type, u64 bootup_keypoint)
{
	if (reboot_type == BFM_S_NATIVE_BOOT_FAIL) {
		BB_PRINT_ERR("%s():%d:reboot_type is [0x%llx]\n",
			__func__, __LINE__, reboot_type);
		return true;
	}

	if ((bootup_keypoint < STAGE_XLOADER_START ||
		bootup_keypoint >= STAGE_FASTBOOT_END) &&
		(reboot_type != AP_S_PRESS6S)) {
		BB_PRINT_PN("%s():%d:bootup_keypoint is [0x%llx]\n",
			__func__, __LINE__, bootup_keypoint);
		return false;
	}

	if (reboot_type >= REBOOT_REASON_LABEL1 &&
		reboot_type < REBOOT_REASON_LABEL3) {
		BB_PRINT_PN("%s():%d:reboot_type is [0x%llx]\n",
			__func__, __LINE__, reboot_type);
		return false;
	}

	return true;
}

static int check_dfx_head_valid(struct dfx_head_info *base, const void *cur_address, u32 size)
{
	u32 noreboot_size;

	noreboot_size = dfx_size_tbl[DFX_NOREBOOT];

	if (cur_address >= (void *)base && size <= EVERY_NUMBER_SIZE &&
		cur_address + size <= (void *)base + noreboot_size)
		return 0;

	BB_PRINT_ERR("%s(), base:0x%pK, cur_address:0x%pK, size:0x%x, noreboot_size:0x%x\n",
	     __func__, base, cur_address, size, noreboot_size);

	return -1;
}

/*
 * Description:    save dfx's ddr_buffer to file
 * Return:         0:success
 */
#ifdef CONFIG_HISI_PARTITION
static int save_dfxbuffer_to_file(struct dfx_head_info *dfx_head_info)
{
	char *buf = NULL;
	int last_number, ret;
	int fd = -1;
	struct rdr_exception_info_s temp;
	struct every_number_info *every_number_info = NULL;
	struct dfx_save_file dfx_save_file;

	dfx_save_file.offset = NULL;

	BB_PRINT_START();

	if (dfx_head_info->need_save_number > TOTAL_NUMBER) {
		BB_PRINT_ERR("need_save_number error %d\n", dfx_head_info->need_save_number);
		dfx_head_info->need_save_number = TOTAL_NUMBER;
	}

	(void)memset_s(&temp, sizeof(temp), 0, sizeof(temp));
	last_number = (dfx_head_info->cur_number - dfx_head_info->need_save_number + TOTAL_NUMBER) % TOTAL_NUMBER;

	while (dfx_head_info->need_save_number != 0) {
		buf = (char *)dfx_head_info + dfx_head_info->every_number_addr[last_number];
		every_number_info = (struct every_number_info *)buf;

		if (!need_save_dfxbuffer2file(every_number_info->reboot_type, every_number_info->bootup_keypoint)) {
			dfx_head_info->need_save_number--;
			last_number = (last_number + 1 + TOTAL_NUMBER) % TOTAL_NUMBER;
			BB_PRINT_ERR("%s continue here, dfx_head_info->need_save_number is %d\n",
			    __func__, dfx_head_info->need_save_number);
			continue;
		}

		temp.e_from_core = RDR_AP;
		temp.e_reset_core_mask = RDR_AP;
		temp.e_exce_type = every_number_info->reboot_type;
		temp.e_exce_subtype = every_number_info->exce_subtype;

		memset(dfx_save_file.path, 0, PATH_MAXLEN);
		memset(dfx_save_file.date, 0, DATATIME_MAXLEN);

		if (!rdr_check_log_rights()) {
			ret = snprintf_s(dfx_save_file.date, DATATIME_MAXLEN, DATATIME_MAXLEN - 1, "%s-%08lld",
				  rdr_get_timestamp(), rdr_get_tick());
			if (unlikely(ret < 0)) {
				BB_PRINT_ERR("[%s], snprintf_s date ret %d\n", __func__, ret);
				return -1;
			}

			rdr_save_history_log(&temp, &dfx_save_file.date[0], DATATIME_MAXLEN,
			    false, every_number_info->bootup_keypoint);

			dfx_head_info->need_save_number--;
			last_number = (last_number + 1 + TOTAL_NUMBER) % TOTAL_NUMBER;

			continue;
		}

		if (bbox_create_dfxlog_path(&dfx_save_file.path[0], &dfx_save_file.date[0], DATATIME_MAXLEN) != 0) {
			BB_PRINT_ERR("bbox_create_dfxlog_path fail\n");
			return -1;
		}

		strncat(dfx_save_file.path, "/ap_log/", ((PATH_MAXLEN - 1) - strlen(dfx_save_file.path)));
		fd = sys_mkdir(dfx_save_file.path, DIR_LIMIT);
		if (fd < 0) {
			BB_PRINT_ERR("%s():%d:mkdir %s fail[%d]\n", __func__, __LINE__, dfx_save_file.path, fd);
			return -1;
		}

		if (every_number_info->fastbootlog_size != 0) {
			dfx_save_file.offset = (void *)every_number_info + every_number_info->fastbootlog_start_addr;
			dfx_save_file.size = (u32)every_number_info->fastbootlog_size;
			if (!check_dfx_head_valid(dfx_head_info, dfx_save_file.offset, dfx_save_file.size))
				(void)rdr_savebuf2fs(dfx_save_file.path, "fastboot_log",
				    dfx_save_file.offset, dfx_save_file.size, 0);
		}

		if (every_number_info->last_kmsg_size != 0) {
			dfx_save_file.offset = (void *)every_number_info + every_number_info->last_kmsg_start_addr;
			dfx_save_file.size = (u32)every_number_info->last_kmsg_size;
			if (!check_dfx_head_valid(dfx_head_info, dfx_save_file.offset, dfx_save_file.size))
				(void)rdr_savebuf2fs(dfx_save_file.path, "last_kmsg",
				    dfx_save_file.offset, dfx_save_file.size, 0);
		}

		if (every_number_info->last_applog_size != 0) {
			dfx_save_file.offset = (void *)every_number_info + every_number_info->last_applog_start_addr;
			dfx_save_file.size = (u32)every_number_info->last_applog_size;
			if (!check_dfx_head_valid(dfx_head_info, dfx_save_file.offset, dfx_save_file.size))
				(void)rdr_savebuf2fs(dfx_save_file.path, "pmsg-ramoops-0",
				    dfx_save_file.offset, dfx_save_file.size, 0);
		}

		(void)rdr_save_history_log(&temp, &dfx_save_file.date[0],
			  DATATIME_MAXLEN, true, every_number_info->bootup_keypoint);

		dfx_save_file.path[strlen(dfx_save_file.path) - strlen("/ap_log/")] = '\0';
		bbox_save_done(dfx_save_file.path, BBOX_SAVE_STEP_DONE);
		dfx_head_info->need_save_number--;
		last_number = (last_number + 1 + TOTAL_NUMBER) % TOTAL_NUMBER;

		rdr_count_size();
	}

	return 0;
}
#endif

/*
 * Description:    save dfx partition's log to file
 */
void save_dfxpartition_to_file(void)
{
#ifdef CONFIG_HISI_PARTITION
	char p_name[RDR_PNAME_SIZE];
	void *buf = NULL;
	void *dfx_buf = NULL;
	void *dfx_buf_temp = NULL;
	int ret, need_read_size;
	int fd_dfx = -1;

	BB_PRINT_START();

	if (rdr_wait_partition("/data/lost+found", RDR_WAIT_PARTITION_TIME) != 0) {
		BB_PRINT_PN("%s():%d:data is not ready\n", __func__, __LINE__);
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
	strncpy(p_name, buf, RDR_PNAME_SIZE);
	p_name[RDR_PNAME_SIZE - 1] = '\0';

	fd_dfx = sys_open(p_name, O_RDONLY, FILE_LIMIT);
	if (fd_dfx < 0) {
		BB_PRINT_ERR("%s():%d:open fail[%d]\n", __func__, __LINE__, fd_dfx);
		goto out;
	}

	dfx_buf_temp = dfx_buf;
	need_read_size = DFX_USED_SIZE;
	memset((void *)dfx_buf_temp, 0, DFX_USED_SIZE);

	while (need_read_size > 0) {
		ret = sys_read(fd_dfx, buf, SZ_4K);
		if (ret < 0) {
			BB_PRINT_ERR("%s():%d:read fail[%d]\n", __func__, __LINE__, ret);
			goto close;
		}

		if (ret == 0)
			break;

		memcpy((void *)dfx_buf_temp, (const void *)buf, ret);
		dfx_buf_temp = dfx_buf_temp + ret;
		need_read_size -= ret;
	}

	sys_close(fd_dfx);

	ret = save_dfxbuffer_to_file((struct dfx_head_info *)dfx_buf);
	if (ret) {
		BB_PRINT_ERR("%s():%d:save_dfxbuffer_to_file fail[%d]\n", __func__, __LINE__, ret);
		goto out;
	}

	fd_dfx = sys_open(p_name, O_WRONLY, FILE_LIMIT);
	if (fd_dfx < 0) {
		BB_PRINT_ERR("%s():%d:open fail[%d]\n", __func__, __LINE__, fd_dfx);
		goto out;
	}

	ret = sys_write(fd_dfx, (void *)dfx_buf, SZ_4K);
	if (ret <= 0) {
		BB_PRINT_ERR("%s():%d:write fail[%d]\n", __func__, __LINE__, ret);
		goto close;
	}

close:
	sys_close(fd_dfx);
out:
	vfree(dfx_buf);
	kfree(buf);
#endif
}

#ifdef CONFIG_HISI_PARTITION
/*
 * note: caller initialize tmp_buf to zero
 */
static int get_dfx_ptn_name(char *part_name, unsigned int name_size)
{
	int ret;
	void *buf = NULL;

	buf = kzalloc(SZ_4K, GFP_KERNEL);
	if (buf == NULL) {
		BB_PRINT_ERR("%s():%d:kzalloc buf fail\n", __func__, __LINE__);
		return -1;
	}

	ret = flash_find_ptn_s(PART_DFX, buf, SZ_4K);
	if (ret != 0) {
		BB_PRINT_ERR("%s():%d:flash_find_ptn_s fail\n", __func__, __LINE__);
		kfree(buf);
		return -1;
	}

	/* before use of part_name, initialize part_name to zero */
	(void)memset_s(part_name, name_size, 0, name_size);
	name_size = (name_size < SZ_4K ? name_size : SZ_4K);
	ret = strncpy_s(part_name, name_size, buf, name_size - 1);
	if (ret != EOK) {
		BB_PRINT_ERR("%s():%d:strncpy_s fail\n", __func__, __LINE__);
		kfree(buf);
		return -1;
	}

	kfree(buf);

	return 0;
}

/*
 * note: caller initialize tmp_buf to zero
 */
static int read_part_into_buff(const char *part_name, char *tmp_buf, unsigned int size)
{
	int cnt;
	int fd = -1;

	fd = sys_open(part_name, O_RDONLY, FILE_LIMIT);
	if (fd < 0) {
		BB_PRINT_ERR("%s():%d:open fail[%d]\n", __func__, __LINE__, fd);
		return -1;
	}

	cnt = sys_read(fd, tmp_buf, size);
	if (cnt < 0) {
		BB_PRINT_ERR("%s():%d:read fail[%d]\n", __func__, __LINE__, cnt);
		sys_close(fd);
		return -1;
	}

	sys_close(fd);

	return 0;
}

static int write_info_into_buff(struct every_number_info *every_number_info, const char *part_name, char *tmp_buf)
{
	int ret, need_write_size, cfo, cnt;
	int fd_dfx = -1;
	struct dfx_head_info *dfx_head_info = NULL;
	void *local_buf = NULL;

	dfx_head_info = (struct dfx_head_info *)tmp_buf;

	fd_dfx = sys_open(part_name, O_WRONLY, FILE_LIMIT);
	if (fd_dfx < 0) {
		BB_PRINT_ERR("%s():%d:open fail[%d]\n", __func__, __LINE__, fd_dfx);
		return -1;
	}

	cfo = dfx_head_info->every_number_addr[dfx_head_info->cur_number];
	ret = sys_lseek(fd_dfx, cfo, SEEK_SET);
	if (ret != cfo) {
		BB_PRINT_ERR("%s():%d:lseek fail[%d]\n", __func__, __LINE__, ret);
		goto close;
	}

	local_buf = kzalloc(SZ_4K, GFP_KERNEL);
	if (local_buf == NULL) {
		BB_PRINT_ERR("%s():%d:kzalloc local_buf fail\n", __func__, __LINE__);
		goto close;
	}

	need_write_size = EVERY_NUMBER_SIZE;
	while (need_write_size > 0) {
		if (memcpy_s(local_buf, SZ_4K, every_number_info, SZ_4K) != EOK) {
			BB_PRINT_ERR("%s():%d:memcpy_s fail\n", __func__, __LINE__);
			kfree(local_buf);
			local_buf = NULL;
			goto close;
		}

		cnt = sys_write(fd_dfx, local_buf, SZ_4K);
		if (cnt <= 0) {
			BB_PRINT_ERR("%s():%d:write fail[%d]\n", __func__, __LINE__, cnt);
			kfree(local_buf);
			local_buf = NULL;
			goto close;
		}
		need_write_size -= cnt;
		every_number_info =
		    (struct every_number_info *)((char *)every_number_info + cnt);
	}

	ret = sys_lseek(fd_dfx, 0, SEEK_SET);
	if (ret != 0) {
		BB_PRINT_ERR("%s():%d:lseek fail[%d]\n", __func__, __LINE__, ret);
		kfree(local_buf);
		local_buf = NULL;
		goto close;
	}

	dfx_head_info->need_save_number++;
	if (dfx_head_info->need_save_number > TOTAL_NUMBER)
		dfx_head_info->need_save_number = TOTAL_NUMBER;
	dfx_head_info->cur_number = (dfx_head_info->cur_number + 1) % TOTAL_NUMBER;
	cnt = sys_write(fd_dfx, tmp_buf, DFX_HEAD_SIZE);
	if (cnt <= 0) {
		BB_PRINT_ERR("%s():%d:write fail[%d]\n", __func__, __LINE__, cnt);
		kfree(local_buf);
		local_buf = NULL;
		goto close;
	}
	kfree(local_buf);
	local_buf = NULL;

	ret = (int)sys_fsync(fd_dfx);
	if (ret < 0)
		BB_PRINT_ERR("[%s]sys_fsync failed, ret is %d\n", __func__, ret);
	sys_close(fd_dfx);

	return 0;

close:
	ret = (int)sys_fsync(fd_dfx);
	if (ret < 0)
		BB_PRINT_ERR("[%s]sys_fsync failed, ret is %d\n", __func__, ret);
	sys_close(fd_dfx);

	return -1;
}

static int check_dfx_head_info(const char *tmp_buf)
{
	struct dfx_head_info *dfx_head_info = NULL;

	dfx_head_info = (struct dfx_head_info *)tmp_buf;
	if (dfx_head_info->cur_number < 0 || dfx_head_info->cur_number >= TOTAL_NUMBER) {
		BB_PRINT_ERR("%s():%d:dfx_head_info->cur_number=%d error\n", __func__, __LINE__,
					dfx_head_info->cur_number);
		return -1;
	}

	return 0;
}
#endif
/*
 * Description:    write input arg to dfx_partition's loopbuffer
 * Input:          every_number_info:the data that need to write
 */
static void save_buffer_to_dfx_loopbuffer(struct every_number_info *every_number_info)
{
#ifdef CONFIG_HISI_PARTITION
	char *buf = NULL;
	char p_name[RDR_PNAME_SIZE];
	int ret;

	BB_PRINT_START();

	ret = get_dfx_ptn_name(p_name, RDR_PNAME_SIZE);
	if (ret < 0)
		return;

	buf = kzalloc((unsigned long)DFX_HEAD_SIZE, GFP_KERNEL);
	if (buf == NULL) {
		BB_PRINT_ERR("%s():%d:kzalloc buf fail\n", __func__, __LINE__);
		return;
	}

	ret = read_part_into_buff(p_name, buf, DFX_HEAD_SIZE);
	if (ret < 0)
		goto free_buffer;

	ret = check_dfx_head_info(buf);
	if (ret < 0)
		goto free_buffer;

	ret = write_info_into_buff(every_number_info, p_name, buf);
	if (ret < 0)
		BB_PRINT_ERR("%s():%d:write info fail[%d]\n", __func__, __LINE__, ret);

free_buffer:
		kfree(buf);
#endif
}

/*
 * Description:    save some log to dfx partition when bbox_system_error is called
 */
void systemerror_save_log2dfx(u32 reboot_type)
{
	void *buf = NULL;
	void *fastbootlog_addr = NULL;
	void *pstore_addr = NULL;
	void *last_kmsg_addr = NULL;
	void *last_applog_addr = NULL;
	struct every_number_info *every_number_info = NULL;

	BB_PRINT_START();

	if (in_atomic() || irqs_disabled() || in_irq()) {
		BB_PRINT_PN(
		    "%s():%d:unsupport in atomic or irqs disabled or in irq\n",
		    __func__, __LINE__);
		return;
	}

	buf = vmalloc(EVERY_NUMBER_SIZE);
	if (buf == NULL) {
		BB_PRINT_ERR("%s():%d:vmalloc buf fail\n", __func__, __LINE__);
		return;
	}

	fastbootlog_addr = ioremap_wc(HISI_SUB_RESERVED_FASTBOOT_LOG_PYHMEM_BASE,
		HISI_SUB_RESERVED_FASTBOOT_LOG_PYHMEM_SIZE);
	if (fastbootlog_addr == NULL) {
		BB_PRINT_ERR("%s():%d:ioremap_wc fastbootlog_addr fail\n", __func__, __LINE__);
		vfree(buf);
		return;
	}

	pstore_addr = bbox_vmap(HISI_RESERVED_PSTORE_PHYMEM_BASE,
		HISI_RESERVED_PSTORE_PHYMEM_SIZE);
	if (pstore_addr == NULL) {
		BB_PRINT_ERR("%s():%d:bbox_vmap pstore_addr fail\n", __func__, __LINE__);
		vfree(buf);
		iounmap(fastbootlog_addr);
		return;
	}

	last_applog_addr = pstore_addr +
		HISI_RESERVED_PSTORE_PHYMEM_SIZE -
		LAST_APPLOG_SIZE;
	last_kmsg_addr = last_applog_addr - LAST_KMSG_SIZE;

	(void)memset_s(buf, EVERY_NUMBER_SIZE, 0, EVERY_NUMBER_SIZE);
	every_number_info = buf;
	every_number_info->rtc_time = get_system_time();
	every_number_info->boot_time = hisi_getcurtime();
	every_number_info->bootup_keypoint = get_boot_keypoint();
	every_number_info->reboot_type = reboot_type;
	every_number_info->fastbootlog_start_addr = DFX_HEAD_SIZE;
	every_number_info->fastbootlog_size = FASTBOOTLOG_SIZE;
	every_number_info->last_kmsg_start_addr = every_number_info->fastbootlog_start_addr + FASTBOOTLOG_SIZE;
	every_number_info->last_kmsg_size = 0;
	every_number_info->last_applog_start_addr = every_number_info->last_kmsg_start_addr + LAST_KMSG_SIZE;
	every_number_info->last_applog_size = 0;

	memcpy((char *)every_number_info + every_number_info->fastbootlog_start_addr,
	       fastbootlog_addr,
	       every_number_info->fastbootlog_size);

	if (readl(last_kmsg_addr + DFX_LOG_SIZE_SHIFT) != 0) {
		every_number_info->last_kmsg_size = LAST_KMSG_SIZE;
		memcpy((char *)every_number_info + every_number_info->last_kmsg_start_addr,
		       last_kmsg_addr,
		       every_number_info->last_kmsg_size);
	}

	if (readl(last_applog_addr + DFX_LOG_SIZE_SHIFT) != 0) {
		every_number_info->last_applog_size = LAST_APPLOG_SIZE;
		memcpy((char *)every_number_info + every_number_info->last_applog_start_addr,
		       last_applog_addr,
		       every_number_info->last_applog_size);
	}

	save_buffer_to_dfx_loopbuffer(every_number_info);

	vfree(buf);
	iounmap(fastbootlog_addr);
	vunmap(pstore_addr);
}

/*
 * Description:    write input arg to dfx_partition's tempbuffer
 * Input:          every_number_info:the data that need to write
 */
static void save_buffer_to_dfx_tempbuffer(struct every_number_info *every_number_info)
{
#ifdef CONFIG_HISI_PARTITION
	void *buf = NULL;
	char p_name[RDR_PNAME_SIZE];
	struct dfx_head_info *dfx_head_info = NULL;
	int ret, cnt, need_write_size;
	int fd_dfx = -1;

	buf = kzalloc(SZ_4K, GFP_KERNEL);
	if (buf == NULL) {
		BB_PRINT_ERR("%s():%d:kzalloc buf fail\n", __func__, __LINE__);
		return;
	}

	ret = flash_find_ptn_s(PART_DFX, buf, SZ_4K);
	if (ret != 0) {
		BB_PRINT_ERR("%s():%d:flash_find_ptn_s fail\n", __func__, __LINE__);
		kfree(buf);
		return;
	}

	(void)memset_s(p_name, RDR_PNAME_SIZE, 0, RDR_PNAME_SIZE);
	strncpy(p_name, buf, RDR_PNAME_SIZE);
	p_name[RDR_PNAME_SIZE - 1] = '\0';

	fd_dfx = sys_open(p_name, O_RDONLY, FILE_LIMIT);
	if (fd_dfx < 0) {
		BB_PRINT_ERR("%s():%d:open fail[%d]\n", __func__, __LINE__, fd_dfx);
		goto open_fail;
	}

	memset(buf, 0, SZ_4K);
	cnt = sys_read(fd_dfx, buf, SZ_4K);
	if (cnt < 0) {
		BB_PRINT_ERR("%s():%d:read fail[%d]\n", __func__, __LINE__, cnt);
		goto close;
	}

	sys_close(fd_dfx);

	fd_dfx = sys_open(p_name, O_WRONLY, FILE_LIMIT);
	if (fd_dfx < 0) {
		BB_PRINT_ERR("%s():%d:open fail[%d]\n", __func__, __LINE__, fd_dfx);
		goto open_fail;
	}

	dfx_head_info = buf;

	if (dfx_head_info->magic != DFX_MAGIC_NUMBER) {
		BB_PRINT_ERR("%s():%d:magic error\n", __func__, __LINE__);
		goto close;
	}

	ret = sys_lseek(fd_dfx, dfx_head_info->temp_number_addr, SEEK_SET);
	if (ret != dfx_head_info->temp_number_addr) {
		BB_PRINT_ERR("%s():%d:lseek fail[%d]\n", __func__, __LINE__, ret);
		goto close;
	}

	memset(buf, 0, SZ_4K);
	need_write_size = EVERY_NUMBER_SIZE;
	while (need_write_size > 0) {
		memcpy(buf, every_number_info, SZ_4K);
		cnt = sys_write(fd_dfx, buf, SZ_4K);
		if (cnt <= 0) {
			BB_PRINT_ERR("%s():%d:write fail[%d]\n", __func__, __LINE__, cnt);
			goto close;
		}
		need_write_size -= cnt;
		every_number_info =
		    (struct every_number_info *)((char *)every_number_info + SZ_4K);
	}

close:
	sys_close(fd_dfx);
open_fail:
	kfree(buf);
#endif
}

/*
 * Description:    save log to the temp buffer of dfx partition
 */
void save_log_to_dfx_tempbuffer(u32 reboot_type)
{
	void *buf = NULL;
	void *fastbootlog_addr = NULL;
	void *pstore_addr = NULL;
	void *last_kmsg_addr = NULL;
	void *last_applog_addr = NULL;
	struct every_number_info *every_number_info = NULL;

	buf = vmalloc(EVERY_NUMBER_SIZE);
	if (buf == NULL) {
		BB_PRINT_ERR("%s():%d:vmalloc buf fail\n", __func__, __LINE__);
		return;
	}

	fastbootlog_addr = ioremap_wc(HISI_SUB_RESERVED_FASTBOOT_LOG_PYHMEM_BASE,
		HISI_SUB_RESERVED_FASTBOOT_LOG_PYHMEM_SIZE);
	if (fastbootlog_addr == NULL) {
		BB_PRINT_ERR("%s():%d:ioremap_wc fastbootlog_addr fail\n", __func__, __LINE__);
		vfree(buf);
		return;
	}

	pstore_addr = bbox_vmap(HISI_RESERVED_PSTORE_PHYMEM_BASE,
		HISI_RESERVED_PSTORE_PHYMEM_SIZE);
	if (pstore_addr == NULL) {
		BB_PRINT_ERR("%s():%d:bbox_vmap pstore_addr fail\n", __func__, __LINE__);
		vfree(buf);
		iounmap(fastbootlog_addr);
		return;
	}

	last_applog_addr = pstore_addr +
		HISI_RESERVED_PSTORE_PHYMEM_SIZE -
		LAST_APPLOG_SIZE;
	last_kmsg_addr = last_applog_addr - LAST_KMSG_SIZE;

	(void)memset_s(buf, EVERY_NUMBER_SIZE, 0, EVERY_NUMBER_SIZE);
	every_number_info = buf;
	every_number_info->rtc_time = get_system_time();
	every_number_info->boot_time = hisi_getcurtime();
	every_number_info->bootup_keypoint = get_boot_keypoint();
	every_number_info->reboot_type = reboot_type;
	every_number_info->fastbootlog_start_addr = DFX_HEAD_SIZE;
	every_number_info->fastbootlog_size = FASTBOOTLOG_SIZE;
	every_number_info->last_kmsg_start_addr = every_number_info->fastbootlog_start_addr + FASTBOOTLOG_SIZE;
	every_number_info->last_kmsg_size = 0;
	every_number_info->last_applog_start_addr = every_number_info->last_kmsg_start_addr + LAST_KMSG_SIZE;
	every_number_info->last_applog_size = 0;

	memcpy((char *)every_number_info + every_number_info->fastbootlog_start_addr,
	       fastbootlog_addr,
	       every_number_info->fastbootlog_size);

	if (readl(last_kmsg_addr + DFX_LOG_SIZE_SHIFT) != 0) {
		every_number_info->last_kmsg_size = LAST_KMSG_SIZE;
		memcpy((char *)every_number_info + every_number_info->last_kmsg_start_addr,
		       last_kmsg_addr,
		       every_number_info->last_kmsg_size);
	}

	if (readl(last_applog_addr + DFX_LOG_SIZE_SHIFT) != 0) {
		every_number_info->last_applog_size = LAST_APPLOG_SIZE;
		memcpy((char *)every_number_info + every_number_info->last_applog_start_addr,
		       last_applog_addr,
		       every_number_info->last_applog_size);
	}

	save_buffer_to_dfx_tempbuffer(every_number_info);

	vfree(buf);
	iounmap(fastbootlog_addr);
	vunmap(pstore_addr);
}

/*
 * Description:    clear the tempbuffer of dfx partition
 */
void clear_dfx_tempbuffer(void)
{
	void *buf = NULL;

	buf = vmalloc(EVERY_NUMBER_SIZE);
	if (buf == NULL) {
		BB_PRINT_ERR("%s():%d:vmalloc buf fail\n", __func__, __LINE__);
		return;
	}
	(void)memset_s(buf, EVERY_NUMBER_SIZE, 0, EVERY_NUMBER_SIZE);
	save_buffer_to_dfx_tempbuffer((struct every_number_info *)buf);

	vfree(buf);
}

static int get_dfx_core_size(void)
{
	int ret;
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL,
				     "hisilicon,dfx_partition");
	if (np == NULL) {
		BB_PRINT_ERR("[%s], cannot find rdr_ap_adapter node in dts\n",
		       __func__);
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "dfx_noreboot_size",
				   &dfx_size_tbl[DFX_NOREBOOT]);
	if (ret) {
		BB_PRINT_ERR("[%s], cannot find dfx_noreboot_size in dts\n",
		       __func__);
		return ret;
	}
	dfx_addr_tbl[DFX_ZEROHUNG] += dfx_size_tbl[DFX_NOREBOOT];
	ret = of_property_read_u32(np, "dfx_zerohung_size",
				   &dfx_size_tbl[DFX_ZEROHUNG]);
	if (ret) {
		BB_PRINT_ERR("[%s], cannot find dfx_zerohung_size in dts\n",
		       __func__);
		return ret;
	}

	return 0;
}

u32 get_dfx_size(u32 module)
{
	if (module >= DFX_MAX_MODULE)
		return 0;

	return dfx_size_tbl[module];
}

u32 get_dfx_addr(u32 module)
{
	if (module >= DFX_MAX_MODULE)
		return 0;

	return dfx_addr_tbl[module];
}


int dfx_partition_init(void)
{
	int ret;

	ret = get_dfx_core_size();
	if (ret < 0)
		goto err;

	BB_PRINT_PN("%s success\n", __func__);
	return 0;
err:
	return ret;
}

early_initcall(dfx_partition_init);
