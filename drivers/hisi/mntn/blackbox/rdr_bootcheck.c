/*
 * rdr_bootcheck.c
 *
 * rdr startup abnormal monitoring
 *
 * Copyright (c) 2001-2019 Huawei Technologies Co., Ltd.
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
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/hardirq.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/util.h>
#include <linux/mmc/core.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/syscalls.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <securec.h>
#include <linux/hisi/hisi_pstore.h>
#include <pr_log.h>
#include "platform_ap/rdr_hisi_ap_adapter.h"
#include "rdr_inner.h"
#include "rdr_field.h"
#include "rdr_print.h"
#include "rdr_debug.h"
#include "rdr_recovery_exception.h"

#define PR_LOG_TAG BLACKBOX_TAG

struct bootcheck {
	u64 mask;
	u64 core;
	u32 modid;
	u32 type;
	u32 subtype;
};

#define RDR_NEED_SAVE_MEM 1
#define RDR_DONTNEED_SAVE_MEM 0

static void rdr_check_log_save(u32 boot_type)
{
	if (need_save_mntndump_log(boot_type)) {
		if (save_mntndump_log("need save done"))
			BB_PRINT_ERR("save_mntndump_log fail\n");
	}
}

/*
 * check status of last reboot.
 * return
 * 0 dont need save.
 * 1 need save log.
 */
static int rdr_check_exceptionboot(struct bootcheck *info)
{
	u32 temp_reboot_type;
	struct rdr_base_info_s *base = NULL;
	struct rdr_struct_s *tmpbb = NULL;

	if (!info) {
		BB_PRINT_PN();
		return RDR_DONTNEED_SAVE_MEM;
	}

	temp_reboot_type = rdr_get_reboot_type();
	/* If the exception type is normal, do not need to save log */
	if (temp_reboot_type < REBOOT_REASON_LABEL1 || (temp_reboot_type >= REBOOT_REASON_LABEL4 &&
		   temp_reboot_type < REBOOT_REASON_LABEL5)) {
		BB_PRINT_PN();
		return RDR_DONTNEED_SAVE_MEM;
	}

	/* Save the default value of the log after reset */
	info->modid = RDR_MODID_AP_ABNORMAL_REBOOT;
	info->mask = RDR_AP;
	info->core = RDR_AP;
	info->type = temp_reboot_type;
	info->subtype = rdr_get_exec_subtype_value();

	/* If the exception type is simple process, you need to save the log after restarting */
	if (temp_reboot_type >= REBOOT_REASON_LABEL1 && temp_reboot_type < REBOOT_REASON_LABEL3)
		return RDR_NEED_SAVE_MEM;

	/* Get the memory address of bbox */
	tmpbb = rdr_get_tmppbb();
	if (!tmpbb) {
		BB_PRINT_PN();
		return RDR_DONTNEED_SAVE_MEM;
	}

	/* Get the bbox header struct */
	base = &(tmpbb->base_info);

	/* If the log is not saved before resetting, you need to save it again after the reset is started */
	if (base->start_flag != RDR_PROC_EXEC_DONE || base->savefile_flag != RDR_DUMP_LOG_DONE) {
		BB_PRINT_ERR("ap error:start[%x],save done[%x]\n",
			     base->start_flag, base->savefile_flag);
		info->modid = BBOX_MODID_LAST_SAVE_NOT_DONE;
		return RDR_NEED_SAVE_MEM;
	}
	rdr_check_log_save(temp_reboot_type);

	/* dump log is over but clear text is not, so need to save the clear text */
	if (tmpbb->cleartext_info.savefile_flag != RDR_CLEARTEXT_LOG_DONE)
		rdr_exceptionboot_save_cleartext();

	return RDR_DONTNEED_SAVE_MEM;
}

static void rdr_bootcheck_notify_dump(char *path, u32 path_len, struct bootcheck *info)
{
	u64 result;

	if (!path || !info) {
		BB_PRINT_ERR("[%s], paramtar is NULL\n", __func__);
		return;
	}
	if (path_len < PATH_MAXLEN)
		BB_PRINT_ERR("[%s:%d]: path_len is invalid\n]", __func__, __LINE__);

	BB_PRINT_PN("create dump file path:[%s]\n", path);
	while (info->mask) {
		if ((rdr_get_cur_regcore() & info->mask) == 0) {
			BB_PRINT_PN
			    ("wait module register. cur:[0x%llx],need[0x%llx]\n",
			     rdr_get_cur_regcore(), info->mask);
			msleep(1000);
			continue;
		}
		result = rdr_notify_onemodule_dump(info->modid, info->mask,
						info->type, info->core, path);
		BB_PRINT_PN("info.mask is [%llx], result = [0x%llx]\n", info->mask,
			     result);
		if (result)
			info->mask &= (~result);
		else
			break;
		BB_PRINT_PN("rdr: notify [%s] core dump data done\n",
			     rdr_get_exception_core(result));
	}
}

static int rdr_save_history_log_back(void)
{
	struct rdr_exception_info_s temp;
	char date[DATATIME_MAXLEN];
	int ret;

	ret = snprintf_s(date, DATATIME_MAXLEN, DATATIME_MAXLEN - 1, "%s-%08lld",
		  rdr_get_timestamp(), rdr_get_tick());
	if (unlikely(ret < 0)) {
		BB_PRINT_ERR("[%s], snprintf_s date ret %d!\n", __func__, ret);
		return -1;
	}

	temp.e_from_core = RDR_AP;
	temp.e_reset_core_mask = RDR_AP;
	temp.e_exce_type = rdr_get_reboot_type();
	temp.e_exce_subtype = rdr_get_exec_subtype_value();

	/* reboot reason ap wdt which is error reported, means other failure result into ap wdt */
	if (unlikely(ap_awdt_analysis(&temp) == 0))
		BB_PRINT_ERR("[%s], ap_awdt_analysis correct reboot reason [%s]!\n",
			__func__, rdr_get_exception_type(temp.e_exce_type));

	if (temp.e_exce_type == LPM3_S_EXCEPTION)
		temp.e_from_core = RDR_LPM3;

	if (temp.e_exce_type == MMC_S_EXCEPTION)
		temp.e_from_core = RDR_EMMC;

	if ((temp.e_exce_type >= CP_S_EXCEPTION_START) && (temp.e_exce_type <= CP_S_EXCEPTION_END))
		temp.e_from_core = RDR_CP;

	rdr_save_history_log(&temp, &date[0], DATATIME_MAXLEN, false, get_last_boot_keypoint());

	return 0;
}

int rdr_bootcheck_thread_body(void *arg)
{
	int cur_reboot_times;
	int ret;
	char path[PATH_MAXLEN];
	struct kstat mem_stat;
	struct bootcheck info;
	struct rdr_syserr_param_s p;
	struct rdr_struct_s *temp_pbb = NULL;
	unsigned int enter_recovery_flag;
	int max_reboot_times = rdr_get_reboot_times();

	BB_PRINT_START();
	enter_recovery_flag = get_boot_into_recovery_flag();
	if (enter_recovery_flag) {
		BB_PRINT_PN("enter save recovery log!\n");
		rdr_recovery_exce();
	}


	(void)rdr_dump_init();
	(void)memset_s(path, PATH_MAXLEN, 0, PATH_MAXLEN);

	BB_PRINT_PN("============wait for fs ready start =============\n");
	while (rdr_wait_partition(PATH_MNTN_PARTITION, RDR_WAIT_PARTITION_TIME) != 0);
	BB_PRINT_PN("============wait for fs ready e n d =============\n");

	bbox_diaginfo_exception_save2fs();
	if (is_need_save_dfx2file())
		save_dfxpartition_to_file();

	if (rdr_check_exceptionboot(&info) != RDR_NEED_SAVE_MEM) {
		BB_PRINT_PN("need not save dump file when boot\n");
		goto end;
	}

	temp_pbb = rdr_get_tmppbb();
	if (temp_pbb->top_head.reserve == RDR_UNEXPECTED_REBOOT_MARK_ADDR) {
		cur_reboot_times = rdr_record_reboot_times2file();
		BB_PRINT_PN("ap has reboot %d times\n", cur_reboot_times);
		if (max_reboot_times < cur_reboot_times)
			rdr_reset_reboot_times(); /* reset the file of reboot_times */
	} else {
		rdr_reset_reboot_times();
	}
	p.modid = info.modid;
	p.arg1 = info.core;
	p.arg2 = info.type;

	if (!rdr_check_log_rights()) {
		ret = rdr_save_history_log_back();
		if (ret < 0)
			goto end;
		goto check_log;
	}

	ret = rdr_create_epath_bc(path, PATH_MAXLEN);
	if (ret == -1) {
		BB_PRINT_ERR("failed to create epath!\n");
		goto end;
	}
	rdr_set_saving_state(1);

	rdr_bootcheck_notify_dump(path, PATH_MAXLEN, &info);

	if (check_himntn(HIMNTN_GOBAL_RESETLOG))
		rdr_save_last_baseinfo(path);

	/* Create a new DONE file under the exception directory, indicating that the exception log is saved */
	bbox_save_done(path, BBOX_SAVE_STEP_DONE);

	/* File system sync to ensure read and write tasks are completed */
	if (!in_atomic() && !irqs_disabled() && !in_irq())
		SYS_SYNC();

	rdr_set_saving_state(0);

	BB_PRINT_PN("saving data done\n");
check_log:
	rdr_count_size();
	BB_PRINT_PN("rdr_count_size: done\n");
end:
	/*
	 * Avoid reserved memory leak. There are two place call read_dump_end.
	 * We must sure the context which call read_dump_end be in the same thread, if not
	 * it's dangerous.
	 */
	if (vfs_stat(SRC_DUMPEND, &mem_stat) == 0)
		read_dump_end();

	hisi_free_persist_store();
	rdr_clear_tmppbb();

	BB_PRINT_END();
	return 0;
}
