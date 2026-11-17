/*
 * log_rdr_cfg.c
 *
 * for rdr log cfg api define
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "log_cfg_api.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/hisi/rdr_pub.h>

#include <securec.h>
#include "tc_ns_log.h"
#include "tlogger.h"

#define TEEOS_MODID HISI_BB_MOD_TEE_START
#define TEEOS_MODID_END  HISI_BB_MOD_TEE_END

struct rdr_register_module_result g_mem_info;
static const u64 g_current_core_id = RDR_TEEOS;

static void tee_fn_dump(u32 mod_id, u32 e_type, u64 core_id,
	char *path_name, pfn_cb_dump_done pfn_cb)
{
	(void)e_type;
	(void)core_id;
	(void)path_name;

	pfn_cb(mod_id, g_current_core_id);
}

static int tee_rdr_register_core(void)
{
	struct rdr_module_ops_pub module_ops = {
		.ops_dump = NULL,
		.ops_reset = NULL
	};
	int ret;

	module_ops.ops_dump = tee_fn_dump;
	module_ops.ops_reset = NULL;
	ret = rdr_register_module_ops(g_current_core_id,
		&module_ops, &g_mem_info);
	if (ret)
		tloge("register rdr mem failed\n");

	return ret;
}

void unregister_log_exception(void)
{
	int ret;

	ret = rdr_unregister_exception((u32)TEEOS_MODID);
	if (ret)
		tloge("unregister rdr exception error\n");
}

static int init_rdr_excep_info(struct rdr_exception_info_s *info)
{
	errno_t ret;
	const char tee_module_name[] = "RDR_TEEOS";
	const char tee_module_desc[] = "RDR_TEEOS crash";

	info->e_modid = (u32)TEEOS_MODID;
	info->e_modid_end = (u32)TEEOS_MODID_END;
	info->e_process_priority = RDR_ERR;
	info->e_reboot_priority = RDR_REBOOT_WAIT;
	info->e_notify_core_mask = RDR_TEEOS | RDR_AP;
	info->e_reset_core_mask = RDR_TEEOS | RDR_AP;
	info->e_reentrant = (u32)RDR_REENTRANT_ALLOW;
	info->e_exce_type = TEE_S_EXCEPTION;
	info->e_from_core = RDR_TEEOS;
	info->e_upload_flag = (u32)RDR_UPLOAD_YES;

	ret = memcpy_s(info->e_from_module, sizeof(info->e_from_module),
		tee_module_name, sizeof(tee_module_name));
	if (ret) {
		tloge("memcpy module name failed\n");
		return ret;
	}
	ret = memcpy_s(info->e_desc, sizeof(info->e_desc),
		tee_module_desc, sizeof(tee_module_desc));
	if (ret) {
		tloge("memcpy module desc failed\n");
		return ret;
	}

	return ret;
}

int register_log_exception(void)
{
	struct rdr_exception_info_s info;
	int ret;

	(void)memset_s(&info, sizeof(info), 0, sizeof(info));
	ret = init_rdr_excep_info(&info);
	if (ret)
		return ret;

	ret = rdr_register_exception(&info); /* return value 0 is error */
	if (!ret) {
		tloge("register exception info failed\n");
		return -1;
	}

	return 0;
}

/* Register log memory */
int register_log_mem(u64 *addr, u32 *len)
{
	int ret;

	if (!addr || !len) {
		tloge("check addr or len is failed\n");
		return -1;
	}

	ret = tee_rdr_register_core();
	if (ret)
		return ret;

	ret = register_mem_to_teeos(g_mem_info.log_addr,
		g_mem_info.log_len, false);
	if (ret)
		return ret;

	*addr = g_mem_info.log_addr;
	*len = g_mem_info.log_len;
	return ret;
}

#define RDR_ERROR_ADDR  0x83000001
void report_log_system_error(void)
{
	rdr_system_error(RDR_ERROR_ADDR, 0, 0);
}

void ta_crash_report_log(void)
{
}

int *map_log_mem(u64 mem_addr, u32 mem_len)
{
	return (int *)hisi_bbox_map(mem_addr, mem_len);
}

void unmap_log_mem(int *log_buffer)
{
	hisi_bbox_unmap((void *)log_buffer);
	(void)rdr_unregister_module_ops(g_current_core_id);
}

#define ROOT_UID                0
#define SYSTEM_GID              1000
void get_log_chown(uid_t *user, gid_t *group)
{
	if (!user || !group) {
		tloge("user or group buffer is null\n");
		return;
	}

	*user = ROOT_UID;
	*group = SYSTEM_GID;
}
