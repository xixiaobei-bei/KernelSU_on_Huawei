/*
 * rdr_audio_adapter.c
 *
 * audio rdr adpter
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "rdr_audio_adapter.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/thread_info.h>
#include <linux/notifier.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_ap_hook.h>
#include <linux/hisi/audio_log.h>

#include "rdr_print.h"
#include "rdr_inner.h"
#include "rdr_audio_codec.h"
#include "rdr_audio_soc.h"
#include "rdr_audio_dump_socdsp.h"

#define LOG_TAG "rdr_audio_adapter"

/*lint -e730*/
struct rdr_audio_des_s {
	struct rdr_register_module_result audio_info;
	char soc_pathname[RDR_FNAME_LEN];
	char codec_pathname[RDR_FNAME_LEN];
};
static struct rdr_audio_des_s g_rdr_audio_des;

enum {
	TASK = 0,
	INTERRUPT,
	REGION,
	TARGET_BUTT
};

struct cpuview_task_info {
	unsigned int task_no;
	char task_name[CPUVIEW_TASK_INFO_LEN];
};

static const struct cpuview_task_info g_cpuview_codec_task_info[] = {
	{ 0, "FID_RT" },
	{ 1, "FID_NORMAL" },
	{ 2, "FID_LOW" },
	{ 3, "IDLE" },
	{ 4, "UNDEFINE" },
};

static const struct cpuview_task_info g_cpuview_soc_task_info[] = {
	{ 0, "FID_RT" },
	{ 1, "FID_NORMAL" },
	{ 2, "FID_LOW" },
	{ 3, "UNDEFINE" },
	{ 4, "IDLE" },
};

struct cpuview_interrupt_info {
	unsigned int interrupt_no;
	char interrupt_name[CPUVIEW_INTERRPUT_INFO_LEN];
};

static const struct cpuview_interrupt_info g_cpuview_soc_int_info[] = {
	{ 0, "NMI_0" },
	{ 1, "SoftINT_1" },
	{ 2, "RPROC_2" },
	{ 3, "AP2HIFI_3" },
	{ 4, "INT_4" },
	{ 5, "HIFI_Timer0_5" },
	{ 6, "HIFI_Timer1_6" },
	{ 7, "INT_7" },
	{ 8, "ASP_Timer0_8" },
	{ 9, "ASP_Timer1_9" },
	{ 10, "INT_10" },
	{ 11, "INT_11" },
	{ 12, "INT_12" },
	{ 13, "INT_13" },
	{ 14, "INT_14" },
	{ 15, "OCBC_15" },
	{ 16, "INT_16" },
	{ 17, "DMAC_S_17" },
	{ 18, "DMAC_NS_18" },
	{ 19, "INT_19" },
	{ 20, "INT_20" },
	{ 21, "INT_21" },
	{ 22, "WatchDog_22" },
	{ 23, "LPM3_2_HIFI_23" },
	{ 24, "INT_24" },
	{ 25, "CCORE_2_HIFI_25" },
	{ 26, "BBE16_2_HIFI_26" },
	{ 27, "INT_27" },
	{ 28, "INT_28" },
	{ 29, "WriteErr_29" },
	{ 30, "INT_30" },
	{ 31, "INT_31" },
};

static const struct cpuview_interrupt_info g_cpuview_codec_int_info[] = {
	{ 0, "NMI_0" },
	{ 1, "SoftINT_1" },
	{ 2, "INT_2" },
	{ 3, "INT_3" },
	{ 4, "INT_4" },
	{ 5, "HIFI_Timer0_5" },
	{ 6, "HIFI_Timer1_6" },
	{ 7, "INT_7" },
	{ 8, "INT_8" },
	{ 9, "INT_9" },
	{ 10, "INT_10" },
	{ 11, "INT_11" },
	{ 12, "DMAC_12" },
	{ 13, "MAD_13" },
	{ 14, "CMD_14" },
	{ 15, "CFG_CLK_SW_15" },
	{ 16, "Timer0_0_16" },
	{ 17, "Timer0_1_17" },
	{ 18, "Timer0_1_18" },
	{ 19, "Timer0_1_19" },
	{ 20, "GPIO0_20" },
	{ 21, "GPIO0_21" },
	{ 22, "GPIO0_22" },
	{ 23, "GPIO0_23" },
	{ 24, "Dlock_24" },
	{ 25, "UART_25" },
	{ 26, "CFG_26" },
	{ 27, "INT_27" },
	{ 28, "INT_28" },
	{ 29, "WriteErr_29" },
	{ 30, "INT_30" },
	{ 31, "INT_31" },
};

struct cpuview_region_info {
	unsigned int region_no;
	char region_name[CPUVIEW_REGION_INFO_LEN];
};

static const struct cpuview_region_info g_cpuview_soc_region_info[] = {
	{ 0, "UCOM_WFI" },
	{ 1, "UCOM_DRF" },
	{ 2, "UCOM_POWERDOWN" },
	{ 3, "VOS_INIT" },
	{ 4, "TEXT_CHECK" },
	{ 5, "TASK_PROC" },
	{ 6, "TASK_SWITCH_FLL" },
	{ 7, "UNDEFINE" },
	{ 8, "AUDIO_PCM_UPDATE_BUFF_PLAY" },
	{ 9, "AUDIO_PCM_UPDATE_BUFF_CAPTURE" },
	{ 10, "AUDIO_DSP_IN_EFFECT_PROCESS" },
	{ 11, "AUDIO_DSP_OUT_EFFECT_PROCESS" },
	{ 12, "UNDEFINE" },
	{ 13, "UNDEFINE" },
	{ 14, "AUDIO_PLAYER_DECODE" },
	{ 15, "AUDIO_PLAYER_SRC" },
	{ 16, "AUDIO_PLAYER_DTS" },
	{ 17, "WAKEUP_PCM_UPDATE_BUFF" },
	{ 18, "WAKEUP_DECODE_PROC" },
	{ 19, "WAKEUP_DECODE_MSG" },
	{ 20, "WAKEUP_LP_DECODE_PROC" },
	{ 21, "WAKEUP_LP_DECODE_MSG" },
	{ 22, "UNDEFINE" },
	{ 23, "UNDEFINE" },
	{ 24, "VOICE_PROC_MICIN" },
	{ 25, "VOICE_PROC_SPKOUT" },
	{ 26, "VOICE_ENCODE" },
	{ 27, "VOICE_DECODE" },
	{ 28, "UNDEFINE" },
};

static const struct cpuview_region_info g_cpuview_codec_region_info[] = {
	{ 0, "ANC_MLIB_PROC" },
	{ 1, "PLL_SWITCH_WFI" },
	{ 2, "PA_MLIB_PROC" },
	{ 3, "INIT" },
	{ 4, "FIRST_WFI" },
	{ 5, "IDLE_WFI" },
	{ 6, "WAKEUP_MLIB_PROC" },
	{ 7, "SEND_PLL_SW_CNF" },
	{ 8, "RECEIVE_MSG" },
	{ 9, "SEND_MSG_CNF" },
	{ 10, "SEND_PWRON_CNF" },
	{ 11, "HISI_WAKEUP_DECODE_PROC" },
	{ 12, "HISI_WAKEUP_DECODE_MSG" },
};

struct cpuview_total_info {
	const struct cpuview_task_info *task_info;
	const struct cpuview_interrupt_info *int_info;
	const struct cpuview_region_info *region_info;

	unsigned int task_num;
	unsigned int int_num;
	unsigned int region_num;
};

static void parse_single_cpuview_info(char *info_buf, const unsigned int buf_len,
	const struct cpuview_slice_record *record, struct cpuview_total_info *total_info)
{
	memset(info_buf, 0, buf_len);

	switch (record->target) {
	case TASK:
		if (record->target_id < total_info->task_num)
			snprintf(info_buf, buf_len, "%-15s %-40s", "TASK",
				total_info->task_info[record->target_id].task_name);
		else
			snprintf(info_buf, buf_len, "%-15s %-40s", "TASK", "UNDEFINE");

		break;
	case INTERRUPT:
		if (record->target_id < total_info->int_num)
			snprintf(info_buf, buf_len, "%-15s %-40s", "INT",
				total_info->int_info[record->target_id].interrupt_name);
		else
			snprintf(info_buf, buf_len, "%-15s %-40s", "INT", "UNDEFINE");

		break;
	case REGION:
		if (record->target_id < total_info->region_num)
			snprintf(info_buf, buf_len, "%-15s %-40s", "REGION",
				total_info->region_info[record->target_id].region_name);
		else
			snprintf(info_buf, buf_len, "%-15s %-40s", "REGION", "UNDEFINE");

		break;
	default:
		snprintf(info_buf, buf_len, "%-55s %u", "UNDEFINE TARGET", record->target_id);
		break;
	}

	snprintf(info_buf + strlen(info_buf), buf_len - strlen(info_buf), "%-15s %-11u",
		record->action == 0 ? "ENTER" : "EXIT", record->time_stamp);
}

static int set_total_info(struct cpuview_total_info *total_info, int *cpuview_info_num,
	int *parsed_data_size, unsigned int core_type)
{
	if (core_type == CODECDSP) {
		*cpuview_info_num = CODEC_CPUVIEW_DETAIL_MAX_NUM;
		*parsed_data_size = PARSER_CODEC_CPUVIEW_LOG_SIZE;
		total_info->int_info = g_cpuview_codec_int_info;
		total_info->int_num = ARRAY_SIZE(g_cpuview_codec_int_info);
		total_info->region_info = g_cpuview_codec_region_info;
		total_info->region_num = ARRAY_SIZE(g_cpuview_codec_region_info);
		total_info->task_info = g_cpuview_codec_task_info;
		total_info->task_num = ARRAY_SIZE(g_cpuview_codec_task_info);
	} else if (core_type == SOCDSP) {
		*cpuview_info_num = SOCDSP_CPUVIEW_DETAIL_MAX_NUM;
		*parsed_data_size = PARSER_SOCDSP_CPUVIEW_LOG_SIZE;
		total_info->int_info = g_cpuview_soc_int_info;
		total_info->int_num = ARRAY_SIZE(g_cpuview_soc_int_info);
		total_info->region_info = g_cpuview_soc_region_info;
		total_info->region_num = ARRAY_SIZE(g_cpuview_soc_region_info);
		total_info->task_info = g_cpuview_soc_task_info;
		total_info->task_num = ARRAY_SIZE(g_cpuview_soc_task_info);
	} else {
		AUDIO_LOGE("input core type error, %u", core_type);
		return -EINVAL;
	}

	return 0;
}

int parse_dsp_cpuview(const char *original_buf, unsigned int original_buf_size,
	char *parsed_buf, unsigned int parsed_buf_size, unsigned int core_type)
{
	int ret;
	unsigned short index;
	unsigned int cpuview_info_num;
	unsigned int original_data_size;
	unsigned int parsed_data_size;
	struct cpuview_details *details = NULL;
	struct cpuview_total_info total_info;

	if (!original_buf || !parsed_buf) {
		AUDIO_LOGE("input data buffer is null");
		return -EINVAL;
	}

	ret = set_total_info(&total_info, &cpuview_info_num, &parsed_data_size, core_type);
	if (ret < 0)
		return -EINVAL;

	original_data_size = sizeof(*details) +
		cpuview_info_num * sizeof(struct cpuview_slice_record);
	if (original_buf_size < original_data_size || parsed_buf_size < parsed_data_size) {
		AUDIO_LOGE("input buf size error, original_buf_size: %u, parsed_buf_size: %u",
			original_buf_size, parsed_buf_size);
		return -EINVAL;
	}

	memset(parsed_buf, 0, parsed_buf_size);
	details = (struct cpuview_details *)original_buf;
	index = details->curr_idx;
	snprintf(parsed_buf, parsed_data_size, "\n\n/*********[cpuview info begin]*********/\n\n");

	if (index < cpuview_info_num) {
		unsigned int i;
		char single_info[CPUVIEW_ONE_INFO_LEN];
		struct cpuview_slice_record *slice = NULL;

		snprintf(parsed_buf + strlen(parsed_buf), parsed_data_size - strlen(parsed_buf),
			"Target Target Id ACTION OrigTS\n");

		for (i = 0; i < cpuview_info_num; i++) {
			slice = details->records + index;
			parse_single_cpuview_info(single_info, CPUVIEW_ONE_INFO_LEN, slice, &total_info);

			snprintf(parsed_buf + strlen(parsed_buf), parsed_data_size - strlen(parsed_buf),
				"%s\n", single_info);

			index++;
			if (index == cpuview_info_num)
				index = 0;
		}
	} else {
		AUDIO_LOGE("record index error, %hu", index);
		ret = -EINVAL;
	}

	snprintf(parsed_buf + strlen(parsed_buf), parsed_data_size - strlen(parsed_buf),
		"\n\n/*********[cpuview info end]*********/\n\n");

	return ret;
}

/*lint -e548*/
int parse_dsp_trace(const char *original_data, unsigned int original_data_size,
	char *parsed_data, unsigned int parsed_data_size, unsigned int core_type)
{
	unsigned int i;
	unsigned int stack_depth;
	unsigned int stack_top;
	unsigned int *stack = NULL;

	if (!original_data || !parsed_data) {
		AUDIO_LOGE("input data buffer is null");
		return -EINVAL;
	}

	memset(parsed_data, 0, parsed_data_size);
	snprintf(parsed_data, parsed_data_size, "\n\n/*********[trace info begin]*********/\n\n");
	stack = (unsigned int *)original_data + 8;
	stack_top = *((unsigned int *)original_data + 4);
	stack_depth = (original_data_size - 0x20) / 4;

	snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
		"panic addr: 0x%08x, cur_pc: 0x%08x, pre_pc: 0x%08x, cause: 0x%08x\n",
		*(unsigned int *)original_data, *((unsigned int *)original_data + 1),
		*((unsigned int *)original_data + 2), *((unsigned int *)original_data + 3));
	for (i = 0; i < stack_depth; i += 4)
		snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
			"addr: %08x %08x %08x %08x %08x\n", stack_top + i * 4,
			*(stack + i), *(stack + 1 + i), *(stack + 2 + i), *(stack + 3 + i));

	snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
		"\n\n/*********[trace info end]*********/\n\n");

	return 0;
}

static int create_dir(const char *path)
{
	long fd = -1;

	WARN_ON(path == NULL);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	fd = ksys_access(path, 0);
#else
	fd = sys_access(path, 0);
#endif
	if (fd != 0) {
		AUDIO_LOGI("need create dir %s", path);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
		fd = ksys_mkdir(path, 0770);
#else
		fd = sys_mkdir(path, 0770);
#endif
		if (fd < 0) {
			AUDIO_LOGE("create dir %s failed, fd: %ld", path, fd);
			return -EBADF;
		}
		AUDIO_LOGI("create dir %s successed, fd: %ld", path, fd);
	}

	/* change dir limit root-system */
	if (bbox_chown(path, ROOT_UID, SYSTEM_GID, false))
		AUDIO_LOGE("chown %s dir failed", path);

	return 0;
}

static int rdr_audio_create_dir(const char *path)
{
	char cur_path[RDR_FNAME_LEN];
	int index = 0;

	memset(cur_path, 0, RDR_FNAME_LEN);
	if (*path != '/')
		return -ENOTDIR;

	cur_path[index++] = *path++;

	while (*path != '\0') {
		if (*path == '/') {
			if (create_dir(cur_path))
				return -ENOENT;
		}
		cur_path[index] = *path;
		path++;
		index++;
	}

	return 0;
}

static int rdr_audio_loopwrite_open(const char *name, unsigned int *pfd)
{
	int ret;
	int fd = -1;

	ret = rdr_audio_create_dir(name);
	if (ret != 0) {
		AUDIO_LOGE("create dir fail, name: %s, ret: %d", name, ret);
		return ret;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	fd = (int)ksys_open(name, O_CREAT | O_RDWR | O_APPEND, 0660);
#else
	fd = (int)sys_open(name, O_CREAT | O_RDWR | O_APPEND, 0660);
#endif
	if (fd < 0) {
		AUDIO_LOGE("create and open file fail, name: %s, fd: %d", name, fd);
		return -EBADF;
	}

	*pfd = (unsigned int)fd;

	return ret;
}

static int rdr_audio_loopwrite_append(unsigned int fd, const void *address, unsigned int length)
{
	long bytes;

	WARN_ON(address == NULL);
	WARN_ON(length == 0);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	bytes = ksys_write(fd, address, (unsigned long)length);
#else
	bytes = sys_write(fd, address, (unsigned long)length);
#endif
	if (bytes != length) {
		AUDIO_LOGE("write data failed, bytes: %ld", bytes);
		return -EINVAL;
	}

	return 0;
}

static void rdr_audio_loopwrite_close(unsigned int fd)
{
	long ret;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	ksys_sync();

	ret = ksys_close(fd);
	if (ret)
		AUDIO_LOGE("close file failed, ret: %ld", ret);

#else
	ret = sys_fsync(fd);
	if (ret < 0)
		AUDIO_LOGE("sys_fsync failed, ret is %ld", ret);

	ret = sys_close(fd);
	if (ret != 0)
		AUDIO_LOGE("close file failed, ret: %ld", ret);
#endif
	return;
}

int rdr_audio_write_file(const char *name, const char *data, unsigned int size)
{
	int ret;
	unsigned int fd = 0;
	mm_segment_t oldfs;

	if (!name || !data) {
		AUDIO_LOGE("para is invalid");
		return -EINVAL;
	}

	WARN_ON(size == 0);

	oldfs = get_fs(); /*lint !e501*/
	set_fs(KERNEL_DS); /*lint !e501*/

	ret = rdr_audio_loopwrite_open(name, &fd);
	if (ret != 0) {
		AUDIO_LOGE("open %s fail", name);
		set_fs(oldfs);
		return ret;
	}

	ret = rdr_audio_loopwrite_append(fd, data, size);
	if (ret != 0)
		AUDIO_LOGE("write %s fail", name);

	if (bbox_chown(name, ROOT_UID, SYSTEM_GID, false))
		AUDIO_LOGE("chown %s file failed", name);

	rdr_audio_loopwrite_close(fd);

	set_fs(oldfs);

	return ret;
}
/*lint +e548*/

static void print_log_head(unsigned int modid, unsigned int etype, unsigned long long coreid,
	const char *pathname, pfn_cb_dump_done pfn_cb)
{
	AUDIO_LOGD(" ====================================");
	AUDIO_LOGD(" modid:          [0x%x]", modid);
	AUDIO_LOGD(" coreid:         [0x%llx]", coreid);
	AUDIO_LOGD(" exce tpye:      [0x%x]", etype);
	AUDIO_LOGD(" path name:      [%s]", pathname);
	AUDIO_LOGD(" dump start:     [0x%pK]", (void *)g_rdr_audio_des.audio_info.log_addr);
	AUDIO_LOGD(" dump len:       [%u]", g_rdr_audio_des.audio_info.log_len);
	AUDIO_LOGD(" nve:            [0x%llx]", g_rdr_audio_des.audio_info.nve);
	AUDIO_LOGD(" callback:       [0x%pK]", pfn_cb);
	AUDIO_LOGD(" ====================================");
}

static void rdr_audio_dump_log(unsigned int modid, unsigned int etype,
	unsigned long long coreid, char *pathname, pfn_cb_dump_done pfn_cb)
{
	if (!pathname) {
		AUDIO_LOGE("input path name is null");
		return;
	}

	if (!pfn_cb) {
		AUDIO_LOGE("input dump done cb is null");
		return;
	}

	print_log_head(modid, etype, coreid, pathname, pfn_cb);

	switch (modid) {
	case ((unsigned int)RDR_AUDIO_SOC_WD_TIMEOUT_MODID):
	case ((unsigned int)RDR_AUDIO_NOC_MODID):
		snprintf(g_rdr_audio_des.soc_pathname, RDR_FNAME_LEN, "%s", pathname);
#ifdef RDR_SOCDSP
		rdr_audio_soc_dump(modid, g_rdr_audio_des.soc_pathname, pfn_cb);
#endif
		break;
	case ((unsigned int)RDR_AUDIO_CODEC_WD_TIMEOUT_MODID):
		snprintf(g_rdr_audio_des.codec_pathname, RDR_FNAME_LEN, "%s", pathname);
		rdr_audio_codec_dump(modid, g_rdr_audio_des.codec_pathname, pfn_cb);
		break;
	case ((unsigned int)RDR_AUDIO_CODEC_ERR_MODID):
	case ((unsigned int)RDR_AUDIO_SLIMBUS_LOSTSYNC_MODID):
	case ((unsigned int)RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID):
		/* add dump log process here, if needed */
		break;
	default:
		if (modid >= (unsigned int)HISI_BB_MOD_CP_START && modid <= (unsigned int)HISI_BB_MOD_CP_END) {
#ifdef RDR_SOCDSP
			snprintf(g_rdr_audio_des.soc_pathname, RDR_FNAME_LEN, "%s", pathname);
			AUDIO_LOGI("modem reset soc hifi dump: %s, begin", g_rdr_audio_des.soc_pathname);
			rdr_audio_soc_dump(modid, g_rdr_audio_des.soc_pathname, pfn_cb);
			AUDIO_LOGI("modem reset soc hifi dump: %s, end", g_rdr_audio_des.soc_pathname);
#else
			AUDIO_LOGI("sochifi rdr is close right now, do not dump hifi log");
			pfn_cb(modid, RDR_HIFI);
#endif
		} else {
			AUDIO_LOGE("mod id is invalide: 0x%x[0x%x - 0x%x]", modid,
				(unsigned int)RDR_AUDIO_MODID_START, (unsigned int)RDR_AUDIO_MODID_END);
		}
		break;
	}
}

void rdr_audio_reset(unsigned int modid, unsigned int etype, unsigned long long coreid)
{
	AUDIO_LOGD(" ============================");
	AUDIO_LOGD(" modid:         [0x%x]", modid);
	AUDIO_LOGD(" coreid:        [0x%llx]", coreid);
	AUDIO_LOGD(" exce tpye:     [0x%x]", etype);
	AUDIO_LOGD(" ============================");

	switch (modid) {
	case ((unsigned int)RDR_AUDIO_SOC_WD_TIMEOUT_MODID):
	case ((unsigned int)RDR_AUDIO_NOC_MODID):
#ifdef RDR_SOCDSP
		rdr_audio_soc_reset(modid, etype, coreid);
#endif
		break;
	case ((unsigned int)RDR_AUDIO_CODEC_WD_TIMEOUT_MODID):
		rdr_audio_codec_reset(modid, etype, coreid);
		break;
	case ((unsigned int)RDR_AUDIO_CODEC_ERR_MODID):
		rdr_audio_codec_err_process();
		break;
	case ((unsigned int)RDR_AUDIO_SLIMBUS_LOSTSYNC_MODID):
	case ((unsigned int)RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID):
		/* system will be reboot, do nothing here */
		break;
	default:
		AUDIO_LOGE("mod id is invalide: 0x%x[0x%x - 0x%x]", modid,
			(unsigned int)RDR_AUDIO_MODID_START, (unsigned int)RDR_AUDIO_MODID_END);
		break;
	}
}

static int rdr_audio_register_core(void)
{
	int ret;
	struct rdr_module_ops_pub module_ops;

	AUDIO_LOGI("enter blackbox");

	module_ops.ops_dump = rdr_audio_dump_log;
	module_ops.ops_reset = rdr_audio_reset;

	ret = rdr_register_module_ops((u64)RDR_HIFI, &module_ops, &g_rdr_audio_des.audio_info);
	if (ret < 0) {
		AUDIO_LOGE("rdr register dsp module ops error");
		return -EBUSY;
	}
	AUDIO_LOGI("exit  blackbox");

	return 0;
}

static struct rdr_exception_info_s audio_exception_info[] = {
	{
		.e_modid            = (unsigned int)RDR_AUDIO_SOC_WD_TIMEOUT_MODID,
		.e_modid_end        = (unsigned int)RDR_AUDIO_SOC_WD_TIMEOUT_MODID,
		.e_process_priority = RDR_WARN,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_HIFI,
		.e_reset_core_mask  = RDR_HIFI,
		.e_from_core        = RDR_HIFI,
		.e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
		.e_exce_type        = SOCHIFI_S_EXCEPTION,
		.e_exce_subtype     = 0,
		.e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
		.e_from_module      = "sochifi",
		.e_desc             = "sochifi watchdog timeout",
	},
	{
		.e_modid            = (unsigned int)RDR_AUDIO_CODEC_WD_TIMEOUT_MODID,
		.e_modid_end        = (unsigned int)RDR_AUDIO_CODEC_WD_TIMEOUT_MODID,
		.e_process_priority = RDR_WARN,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_HIFI,
		.e_reset_core_mask  = RDR_HIFI,
		.e_from_core        = RDR_HIFI,
		.e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
		.e_exce_type        = CODECHIFI_S_EXCEPTION,
		.e_exce_subtype     = 0,
		.e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
		.e_from_module      = "codechifi",
		.e_desc             = "codechifi watchdog timeout",
	},
	{
		.e_modid            = (unsigned int)RDR_AUDIO_NOC_MODID,
		.e_modid_end        = (unsigned int)RDR_AUDIO_NOC_MODID,
		.e_process_priority = RDR_WARN,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_HIFI,
		.e_reset_core_mask  = RDR_HIFI,
		.e_from_core        = RDR_HIFI,
		.e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
		.e_exce_type        = SOCHIFI_S_EXCEPTION,
		.e_exce_subtype     = 0,
		.e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
		.e_from_module      = "asp",
		.e_desc             = "audio noc exception",
	},
	{
		.e_modid            = (unsigned int)RDR_AUDIO_CODEC_CRASH_MODID_START,
		.e_modid_end        = (unsigned int)RDR_AUDIO_CODEC_CRASH_MODID_END,
		.e_process_priority = RDR_WARN,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_AP,
		.e_reset_core_mask  = RDR_HIFI,
		.e_from_core        = RDR_HIFI,
		.e_reentrant        = (unsigned int)RDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_CODEC_EXCEPTION,
		.e_exce_subtype     = 0,
		.e_upload_flag      = (unsigned int)RDR_UPLOAD_YES,
		.e_from_module      = "audio codec",
		.e_desc             = "audio codec crash",
	},
};

static int rdr_audio_register_exception(void)
{
	int ret = 0;
	unsigned long i;

	AUDIO_LOGI("enter blackbox");

	for (i = 0; i < ARRAY_SIZE(audio_exception_info); i++) {
		if (!rdr_register_exception(&audio_exception_info[i])) {
			AUDIO_LOGE("regist audio exception fail, index: %lu", i);
			ret = -EBUSY;
		}
	}

	AUDIO_LOGI("exit  blackbox");

	return ret;
}

static int rdr_audio_init_early(void)
{
	int ret;

	ret = rdr_audio_register_exception();
	if (ret != 0) {
		AUDIO_LOGE("rdr dsp register exception fail");
		return ret;
	}

	ret = rdr_audio_register_core();
	if (ret != 0) {
		AUDIO_LOGE("rdr dsp register core fail");
		return ret;
	}

	return ret;
}

static int __init rdr_audio_init(void)
{
	int ret;

	AUDIO_LOGI("enter blackbox");

	ret = rdr_audio_init_early();
	if (ret != 0) {
		AUDIO_LOGE("init early fail");
		AUDIO_LOGI("exit blackbox");
		return ret;
	}

	memset(&g_rdr_audio_des.audio_info, 0, sizeof(struct rdr_register_module_result));
	memset(g_rdr_audio_des.soc_pathname, 0, RDR_FNAME_LEN);
	memset(g_rdr_audio_des.codec_pathname, 0, RDR_FNAME_LEN);

#ifdef RDR_SOCDSP
	ret = rdr_audio_soc_init();
	if (ret != 0)
		AUDIO_LOGE("init rdr socdsp fail");
#endif

#ifdef RDR_CODECDSP
	ret = rdr_audio_codec_init();
	if (ret != 0)
		AUDIO_LOGE("init rdr dsp fail");
#endif

	AUDIO_LOGI("exit  blackbox");

	return ret;
}

static void __exit rdr_audio_exit(void)
{
#ifdef RDR_SOCDSP
	rdr_audio_soc_exit();
#endif
	rdr_audio_codec_exit();
}

module_init(rdr_audio_init);
module_exit(rdr_audio_exit);

