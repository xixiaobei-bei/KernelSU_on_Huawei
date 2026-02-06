/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: implement the platform interface for boot fail
 * Author: wangsiwen
 * Create: 2020-09-20
 */

#include "adapter_mtk.h"

#include <linux/fs.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/thread_info.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#include <securec.h>
#include <soc/mediatek/log_store_kernel.h>

#include <hwbootfail/chipsets/common/adapter_common.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/core/boot_interface.h>

#ifdef CONFIG_BOOT_DETECTOR_MTK_4G
#include <hwbootfail/chipsets/bootfail_mtk_4g.h>
#endif

#define BL_LOG_NAME "fastboot_log"
#define KERNEL_LOG_NAME "last_kmsg"
#define BFI_PART_SIZE 0xBE0000 /* 12MB -128 KB */

#define APP_LOG_PATH            "/data/log/hilogs/hiapplogcat-log"
#define APP_LOG1_PATH           APP_LOG_PATH".1"
#define APP_LOG_PATH_MAX_LENGTH 128
#define APP_LOG_NUM 2

#define FASTBOOTLOG_TYPE 0
#define KMSGLOG_TYPE     1
#define APPLOG_TYPE      2

struct every_number_info {
	u64 rtc_time;
	u64 boot_time;
	u64 bootup_keypoint;
	u64 reboot_type;
	u64 exce_subtype;
	u64 fastbootlog_start_addr;
	u64 fastbootlog_size;
	u64 last_kmsg_start_addr;
	u64 last_kmsg_size;
	u64 last_applog_start_addr[APP_LOG_NUM];
	u64 last_applog_size;
};

size_t get_bfi_part_size(void)
{
	return BFI_PART_SIZE;
}

static void get_raw_part_info(struct adapter *padp)
{
	padp->bfi_part.dev_path = BFI_DEV_PATH;
	padp->bfi_part.part_size = (unsigned int)get_bfi_part_size();
}

static enum boot_stage phase_to_stage(u32 phase)
{
	switch (phase) {
	case BOOT_PHASE_PL:
		return BL1_STAGE;
	case BOOT_PHASE_LK:
		return BL2_STAGE;
	case BOOT_PHASE_KERNEL:
		return KERNEL_STAGE;
	case BOOT_PHASE_NATIVE:
		return NATIVE_STAGE;
	case BOOT_PHASE_FRAMEWORK:
		return FRAMEWORK_STAGE;
	case BOOT_PHASE_ANDROID:
	case BOOT_PHASE_SUSPEND:
	case BOOT_PHASE_RESUME:
		return BOOT_SUCC_STAGE;
	default:
		return FRAMEWORK_STAGE;
	}
}

int is_boot_success(void)
{
	if (is_after_suss_stage(phase_to_stage(get_boot_phase())))
		return 1;
	else
		return 0;
}

u32 stage_to_phase(enum boot_stage stage)
{
	switch (stage) {
	case BL1_STAGE:
		return BOOT_PHASE_PL;
	case BL2_STAGE:
		return BOOT_PHASE_LK;
	case KERNEL_STAGE:
		return BOOT_PHASE_KERNEL;
	case NATIVE_STAGE:
		return BOOT_PHASE_NATIVE;
	case FRAMEWORK_STAGE:
		return BOOT_PHASE_FRAMEWORK;
	case BOOT_SUCC_STAGE:
		return BOOT_PHASE_ANDROID;
	default:
		return BOOT_PHASE_LK;
	}
}

static void clear_rofa_flag(void)
{
	char *p_rofa = NULL;

	p_rofa = get_rofa_mem_base();
	if (p_rofa == NULL) {
		print_err("p_rofa is NULL\n");
		return;
	}
	(void)memset_s(p_rofa, ROFA_MEM_SIZE, 0, ROFA_MEM_SIZE);
}

static int mtk_set_boot_stage(int stage)
{
	set_boot_phase(stage_to_phase(stage));

	if (stage == BOOT_SUCC_STAGE) {
		clear_rofa_flag();
		print_err("boot success\n");
	}
	return 0;
}

static int mtk_get_boot_stage(int *pstage)
{
	u32 phase;

	if (pstage == NULL) {
		print_invalid_params("pstage is NULL\n");
		return -1;
	}

	phase = get_boot_phase();
	*pstage = phase_to_stage(phase);
	return 0;
}

static void get_boot_stage_ops(struct adapter *padp)
{
	padp->stage_ops.set_stage = mtk_set_boot_stage;
	padp->stage_ops.get_stage = mtk_get_boot_stage;
}

static void mtk_shutdown(void)
{
	print_err("unsupported\n");
}

static void get_sysctl_ops(struct adapter *padp)
{
	padp->sys_ctl.reboot = emergency_restart;
	padp->sys_ctl.shutdown = mtk_shutdown;
}

static int mtk_read_from_phys_mem(unsigned long dst,
	unsigned long dst_max,
	void *phys_mem_addr,
	unsigned long data_len)
{
	unsigned long i;
	unsigned long bytes_to_read;
	char *pdst = NULL;

	if (phys_mem_addr == NULL || dst == 0 ||
		dst_max == 0 || data_len == 0) {
		print_invalid_params("bootfail: dst: %u, dst_max: %u, data_len: %u\n",
			dst, dst_max, data_len);
		return -1;
	}

	bytes_to_read = min(dst_max, data_len);
	pdst = (char *)(uintptr_t)dst;
	for (i = 0; i < bytes_to_read; i++) {
		*pdst = readb(phys_mem_addr);
		pdst++;
		phys_mem_addr++;
	}

	return 0;
}

static int mtk_write_to_phys_mem(unsigned long dst,
	unsigned long dst_max,
	void *src,
	unsigned long src_len)
{
	unsigned long i;
	unsigned long bytes_to_write;
	char *psrc = NULL;
	char *pdst = NULL;

	if (src == NULL || dst == 0 || dst_max == 0 || src_len == 0) {
		print_invalid_params("bootfail: dst: %u, dst_max: %u, src_len: %u\n",
			dst, dst_max, src_len);
		return -1;
	}

	bytes_to_write = min(dst_max, src_len);
	pdst = (char *)(uintptr_t)dst;
	psrc = (char *)src;
	for (i = 0; i < bytes_to_write; i++) {
		writeb(*psrc, pdst);
		psrc++;
		pdst++;
	}

	return 0;
}

static void get_phys_mem_info(struct adapter *padp)
{
	/* 1024 is corresponding to BF_SIZE_1K */
	padp->pyhs_mem_info.base = (uintptr_t)get_phys_mem_base();
	padp->pyhs_mem_info.size = BF_SIZE_1K - ROFA_MEM_SIZE;
	padp->pyhs_mem_info.ops.read = mtk_read_from_phys_mem;
	padp->pyhs_mem_info.ops.write = mtk_write_to_phys_mem;
}

static void capture_kernel_log(char *pbuf, unsigned int buf_size)
{
	char *klog_addr = NULL;
	unsigned int klog_size;
	errno_t ret;

	if (pbuf == NULL || buf_size == 0) {
		print_invalid_params("buf_size is %u\n", buf_size);
		return;
	}

	klog_addr = log_buf_addr_get();
	klog_size = get_klog_size();

	ret = memcpy_s(pbuf, buf_size,
		klog_addr, min(klog_size, buf_size));

	if (ret != EOK)
		print_err("memcpy_s failed, ret: %d\n", ret);
}

static void capture_pl_lk_log(char *pbuf, unsigned int buf_size)
{
	char *pl_lk_addr = NULL;
	unsigned int pl_lk_log_size;
	errno_t ret;

	if (pbuf == NULL || buf_size == 0) {
		print_invalid_params("buf_size is %u\n", buf_size);
		return;
	}

	pl_lk_addr = get_pl_lk_log_addr();
	pl_lk_log_size = get_pl_lk_log_size();

	ret = memcpy_s(pbuf, buf_size,
		pl_lk_addr, min(pl_lk_log_size, buf_size));

	if (ret != EOK)
		print_err("memcpy_s failed, ret: %d\n", ret);
}

static void capture_app_log(char *pbuf, unsigned int buf_size, const char *path)
{
	int ret;
	mm_segment_t fs;

	if (path == NULL) {
		print_err("%s : invalid path\n", __func__);
		return;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	ret = sys_readlink(path, pbuf, buf_size);
	if (ret < 0)
		print_err("read %s failed ,ret = %d\n", path, ret);

	set_fs(fs);
}

static void get_log_ops_info(struct adapter *padp)
{
	errno_t ret;

	ret = strncpy_s(padp->bl_log_ops.log_name,
		sizeof(padp->bl_log_ops.log_name), BL_LOG_NAME,
		min(strlen(BL_LOG_NAME),
		sizeof(padp->bl_log_ops.log_name) - 1));
	if (ret != EOK)
		print_err("BL_LOG_NAME strncpy_s failed\n");
	padp->bl_log_ops.log_size = get_total_pl_lk_log_size();
	padp->bl_log_ops.capture_bl_log = capture_pl_lk_log;

	ret = strncpy_s(padp->kernel_log_ops.log_name,
		sizeof(padp->kernel_log_ops.log_name), KERNEL_LOG_NAME,
		min(strlen(KERNEL_LOG_NAME),
			sizeof(padp->kernel_log_ops.log_name) - 1));
	if (ret != EOK)
		print_err("KERNEL_LOG_NAME strncpy_s failed\n");
	padp->kernel_log_ops.log_size = get_klog_size();
	padp->kernel_log_ops.capture_kernel_log = capture_kernel_log;
}

static void degrade(int excp_type)
{
	print_err("unsupported\n");
}

static void bypass(int excp_type)
{
	print_err("unsupported\n");
}

static void load_backup(const char *part_name)
{
	print_err("unsupported\n");
}

static void notify_storage_fault(unsigned long long bopd_mode)
{
	print_err("unsupported\n");
}

static void platform_adapter_init(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp is NULL\n");
		return;
	}

	get_raw_part_info(padp);
	get_phys_mem_info(padp);
	get_log_ops_info(padp);
	get_sysctl_ops(padp);
	get_boot_stage_ops(padp);
	padp->prevent.degrade = degrade;
	padp->prevent.bypass = bypass;
	padp->prevent.load_backup = load_backup;
	padp->notify_storage_fault = notify_storage_fault;
}

static void save_long_press_meta_log(struct every_number_info *pinfo)
{
	pinfo->rtc_time = get_sys_rtc_time();
	pinfo->boot_time = get_bootup_time();
	pinfo->bootup_keypoint = phase_to_stage(get_boot_phase());
	pinfo->reboot_type = KERNEL_SYSTEM_FREEZE;
	pinfo->exce_subtype = KERNEL_SYSTEM_FREEZE;

	pinfo->fastbootlog_start_addr = sizeof(struct every_number_info);
	pinfo->fastbootlog_size = get_total_pl_lk_log_size();
	pinfo->last_kmsg_start_addr = pinfo->fastbootlog_start_addr +
		pinfo->fastbootlog_size;
	pinfo->last_kmsg_size = get_klog_size();
	pinfo->last_applog_start_addr[0] = pinfo->last_kmsg_start_addr +
		pinfo->last_kmsg_size;
	pinfo->last_applog_size = APP_LOG_PATH_MAX_LENGTH;
	pinfo->last_applog_start_addr[1] = pinfo->last_applog_start_addr[0] +
		pinfo->last_applog_size;
	write_part(LONG_PRESS_DEV_PATH, 0, (const char *)pinfo,
		sizeof(struct every_number_info));
}

static void save_long_press_log(u32 type, u64 offset,
	u64 size, const char *path)
{
	char *buf = NULL;

	if (size == 0) {
		print_err("log type %d, invalid size\n", type);
		return;
	}

	buf = vzalloc(size);
	if (buf == NULL)
		return;

	switch (type) {
	case FASTBOOTLOG_TYPE:
		capture_pl_lk_log(buf, size);
		break;
	case KMSGLOG_TYPE:
		capture_kernel_log(buf, size);
		break;
	case APPLOG_TYPE:
		capture_app_log(buf, size, path);
		break;
	default:
		print_err("log type %d invliad type\n", type);
		goto out;
	}

	write_part(LONG_PRESS_DEV_PATH, offset, buf, size);

out:
	vfree(buf);
}

void mtk_save_long_press_logs(void)
{
	int i;
	struct every_number_info info;
	const char *app_log_path[APP_LOG_NUM] = {
		APP_LOG1_PATH,
		APP_LOG_PATH,
	};

	save_long_press_meta_log(&info);

	save_long_press_log(FASTBOOTLOG_TYPE, info.fastbootlog_start_addr,
		info.fastbootlog_size, NULL);
	save_long_press_log(KMSGLOG_TYPE, info.last_kmsg_start_addr,
		info.last_kmsg_size, NULL);

	for (i = 0; i < APP_LOG_NUM; i++)
		save_long_press_log(APPLOG_TYPE, info.last_applog_start_addr[i],
			info.last_applog_size, app_log_path[i]);
}

int mtk_adapter_init(struct adapter *padp)
{
	if (common_adapter_init(padp) != 0) {
		print_err("init adapter common failed\n");
		return -1;
	}
	platform_adapter_init(padp);

	return 0;
}
