/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: implement the platform interface for boot fail
 * Author: yuanshuai
 * Create: 2020-10-26
 */

#include "adapter_qcom.h"
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/reboot.h>
#include <securec.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/of_fdt.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/thread_info.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/soc/qcom/smem.h>
#include <linux/syscalls.h>
#include <hwbootfail/core/boot_interface.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/chipsets/common/adapter_common.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>
#include <linux/rainbow_reason.h>

/* bootfail reserve ddr region */
static void *g_bf_rmem_addr;
static unsigned long g_bf_rmem_size;

#define APP_LOG_PATH "/data/log/hilogs/hiapplogcat-log"
#define APP_LOG_PATH_MAX_LENGTH 128

#define BF_BL_LOG_NAME_SIZE 16
#define BF_KERNEL_LOG_NAME_SIZE 16

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
	u64 last_applog_start_addr;
	u64 last_applog_size;
};

void *get_bf_mem_addr(void)
{
	return g_bf_rmem_addr;
}

unsigned long get_bf_mem_size(void)
{
	return g_bf_rmem_size;
}

static int qcom_read_from_phys_mem(unsigned long dst,
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

static int qcom_write_to_phys_mem(unsigned long dst,
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

__weak long ksys_readlink(const char __user *path, char __user *buf, int bufsiz)
{
	print_err("please implement ksys_readlink\n");
	return 0;
}

long bf_readlink(const char __user *path, char __user *buf, int bufsiz)
{
#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	return sys_readlink(path, buf, bufsiz);
#else
	return ksys_readlink(path, buf, bufsiz);
#endif
}
/**
 * @brief Init reserve ddr region, get ddr info from dtsi.
 * @param NONE.
 * @return BF_OK on success.
 * @since 1.0
 * @version 1.0
 */
int bf_rmem_init(void)
{
	struct device_node *bootfail_node = NULL;
	const u32 *bootfail_dts_addr = NULL;
	unsigned long addr= 0;

	bootfail_node = of_find_compatible_node(NULL,
		NULL, "bootfail_mem");
	if (bootfail_node == 0) {
		print_err("bootfail_mem find fail in DTSI\n");
		goto rmem_init_err;
	}
	bootfail_dts_addr = of_get_address(bootfail_node, 0,
		(u64 *)(&g_bf_rmem_size), NULL);
	if (bootfail_dts_addr == 0) {
		print_err("can not find bootfail mem addr\n");
		goto rmem_init_err;
	}
	addr = (unsigned long)of_translate_address(bootfail_node,
		bootfail_dts_addr);
	if (addr == 0 || g_bf_rmem_size != 2 * BF_SIZE_1K) {
		print_err("bootfail mem have wrong address or size");
		goto rmem_init_err;
	}
#ifdef CONFIG_ARM
	g_bf_rmem_addr = (struct rb_header *)ioremap_nocache(addr, g_bf_rmem_size);
#else
	g_bf_rmem_addr = (struct rb_header *)ioremap_wc(addr, g_bf_rmem_size);
#endif
	if (g_bf_rmem_addr == NULL) {
		print_err("bootfail mem ioremap fail\n");
		goto rmem_init_err;
	}
	print_err("g_bf_rmem_addr:%p, size:%x\n", g_bf_rmem_addr, g_bf_rmem_size);
	return BF_OK;
rmem_init_err:
	g_bf_rmem_addr = NULL;
	g_bf_rmem_size = 0;
	return BF_NOT_INIT_SUCC;
}

static inline void bf_qcom_degrade(int excp_type)
{
	print_err("unsupported\n");
}

static inline void bf_qcom_bp(int excp_type)
{
	print_err("unsupported\n");
}

static inline void bf_qcom_load_backup(const char *part_name)
{
	print_err("unsupported\n");
}

static inline void bf_qcom_notify_storage_fault(unsigned long long bopd_mode)
{
	print_err("unsupported\n");
}

/* Set rrecord info to qcom adapter */
static inline void get_rrecord_part_info(struct adapter *padp)
{
	padp->bfi_part.dev_path = BFI_DEV_PATH;
	padp->bfi_part.part_size = BFI_PART_SIZE;
}

/* Set reserve ddr info to qcom adapter */
static void get_phys_mem_info(struct adapter *padp)
{
	padp->pyhs_mem_info.base = (uintptr_t)get_bf_mem_addr();
	if (padp->pyhs_mem_info.base == 0) {
		print_err("rmem addr init fail\n");
		goto rmem_err;
	}
	padp->pyhs_mem_info.size = get_bf_mem_size();
	if (padp->pyhs_mem_info.size >= BF_SIZE_1K) {
		padp->pyhs_mem_info.size = BF_SIZE_1K - BF_BOPD_RESERVE;
	} else {
		print_err("rmem size too short\n");
		goto rmem_err;
	}
	padp->pyhs_mem_info.ops.read = qcom_read_from_phys_mem;
	padp->pyhs_mem_info.ops.write = qcom_write_to_phys_mem;
	print_err("pyhs_mem_info:%x, %x\n", padp->pyhs_mem_info.base,
		padp->pyhs_mem_info.size);
	return;
rmem_err:
	padp->pyhs_mem_info.base = 0;
	padp->pyhs_mem_info.size = 0;
	return;
}

/**
 * @brief Get xbl UEFI and abl log.
 * @param pbuf - desc buffer.
 * @param buf_size - copy size, actually limited by rainbow bl log size.
 * @return NONE.
 * @since 1.0
 * @version 1.0
 */
static void capture_bl_log(char *pbuf, unsigned int buf_size)
{
	unsigned int bl_log_size;
	char *bl_addr = NULL;
	char *dst_buf = NULL;
	size_t bytes_to_read;
	int i;

	if (pbuf == NULL) {
		print_invalid_params("pbuf is null\n");
		return;
	}
	bl_addr = rb_bl_log_get(&bl_log_size);
	if (bl_addr == NULL) {
		print_invalid_params("bl log params get fail\n");
		return;
	}

	print_info("bl_addr:%p,bl_log_size:%u\n", bl_addr, bl_log_size);
	if (bl_addr == NULL ||
		bl_log_size > BL_LOG_MAX_LEN ||
		buf_size != bl_log_size) {
		print_invalid_params("bl log params err\n");
		return;
	}
	dst_buf = pbuf;
	bytes_to_read = min(buf_size, bl_log_size);
	for (i = 0; i < bytes_to_read; i++) {
		*dst_buf = readb(bl_addr);
		dst_buf++;
		bl_addr++;
	}
	return;
}

/**
 * @brief Get kernel log.
 * @param pbuf - desc buffer.
 * @param buf_size - copy size, actually limited by kernel log buff size
 * @return NONE.
 * @since 1.0
 * @version 1.0
 */
static void capture_kernel_log(char *pbuf, unsigned int buf_size)
{
	char *kbuf_addr = NULL;
	unsigned int kbuf_size = log_buf_len_get();
	errno_t ret;

	if (pbuf == NULL) {
		print_invalid_params("pbuf is null\n");
		return;
	}
	kbuf_addr = log_buf_addr_get();
	if (kbuf_addr == NULL || buf_size != log_buf_len_get()) {
		print_invalid_params("kbuf_addr or buf_size err\n");
		return;
	}
	ret = memcpy_s(pbuf, buf_size,
		kbuf_addr, min(kbuf_size, buf_size));
	if (ret != EOK)
		print_err("memcpy_s failed, ret: %d\n", ret);
}

/**
 * @brief Set get kernel log and fastboot log method to adapter.
 * @param padp - bootfail qcom adapter pointer
 * @return NONE.
 * @since 1.0
 * @version 1.0
 */
static void get_log_ops_info(struct adapter *padp)
{
	errno_t ret;
	/* set bl log info */
	ret = strncpy_s(padp->bl_log_ops.log_name,
		BF_BL_LOG_NAME_SIZE,
		BL_LOG_NAME, min(strlen(BL_LOG_NAME),
		sizeof(padp->bl_log_ops.log_name) - 1));
	if (ret != EOK)
		print_err("bl log name strncpy_s failed, ret: %d\n", ret);

	padp->bl_log_ops.log_size = (unsigned int)BL_LOG_MAX_LEN;
	padp->bl_log_ops.capture_bl_log = capture_bl_log;

	/* set kernel log info */
	ret = strncpy_s(padp->kernel_log_ops.log_name,
		BF_KERNEL_LOG_NAME_SIZE,
		KERNEL_LOG_NAME, min(strlen(KERNEL_LOG_NAME),
		sizeof(padp->kernel_log_ops.log_name) - 1));
        if (ret != EOK)
                print_err("kernel log name strncpy_s failed, ret: %d\n", ret);
	padp->kernel_log_ops.log_size = (unsigned int)log_buf_len_get();
	padp->kernel_log_ops.capture_kernel_log = capture_kernel_log;
}

static void qcom_shutdown(void)
{
	print_err("unsupported\n");
}

static void qcom_reboot(void)
{
	kernel_restart("bootfail");
}

/* Set reboot and shutdown method to adapter */
static void get_sysctl_ops(struct adapter *padp)
{
	padp->sys_ctl.reboot = qcom_reboot;
	padp->sys_ctl.shutdown = qcom_shutdown;
}

static int qcom_set_boot_stage(int stage)
{
	unsigned long addr = (unsigned long)get_bf_mem_addr();
	unsigned long max_size = get_bf_mem_size();

	if (max_size < BF_SIZE_1K + sizeof(int)) {
		print_err("fail, reserve ddr region too short, please check\n");
		return BF_PLATFORM_ERR;
	}
	addr += BF_SIZE_1K;
	qcom_write_to_phys_mem(addr, sizeof(int), (char *)&stage, sizeof(int));
	print_err("set boot stage:%x\n", *(int *)addr);
	return BF_OK;
}

static int qcom_get_boot_stage(int *stage)
{
	unsigned long addr = (unsigned long)get_bf_mem_addr();
	unsigned long max_size = get_bf_mem_size();

	if (max_size < BF_SIZE_1K + sizeof(int)) {
		print_err("fail, reserve ddr region too short, please check\n");
		return BF_PLATFORM_ERR;
	}
	addr += BF_SIZE_1K;
	qcom_read_from_phys_mem((unsigned long)(stage), sizeof(int), (void *)addr, sizeof(int));
	print_err("read boot stage:%x\n", *(int *)addr);
	return BF_OK;
}

/**
 * @brief Set bootstage method to adapter.
 *        record bootstage by qcom share memory function
 * @param padp - bootfail qcom adapter pointer
 * @return NONE.
 * @since 1.0
 * @version 1.0
 */
static void get_boot_stage_ops(struct adapter *padp)
{
	if (get_bf_mem_addr() == NULL || get_bf_mem_size() <= BF_SIZE_1K)
		print_err("bootstage function err, please check\n");

	padp->stage_ops.set_stage = qcom_set_boot_stage;
	padp->stage_ops.get_stage = qcom_get_boot_stage;
}

/* Qcom platform adapter init */
static void platform_adapter_init(struct adapter *padp)
{
	if (padp == NULL) {
		print_err("padp is NULL\n");
		return;
	}

	get_rrecord_part_info(padp);
	get_phys_mem_info(padp);
	get_log_ops_info(padp);
	get_sysctl_ops(padp);
	get_boot_stage_ops(padp);
	padp->prevent.degrade = bf_qcom_degrade;
	padp->prevent.bypass = bf_qcom_bp;
	padp->prevent.load_backup = bf_qcom_load_backup;
	padp->notify_storage_fault = bf_qcom_notify_storage_fault;
}

static void capture_app_log(char *pbuf, unsigned int buf_size)
{
	int ret;
	mm_segment_t fs;

	fs = get_fs();
	set_fs(KERNEL_DS);

	ret = bf_readlink(APP_LOG_PATH, pbuf, buf_size);

	if (ret < 0)
		print_err("read %s failed ,ret = %d\n", APP_LOG_PATH, ret);

	set_fs(fs);
}

static void save_long_press_meta_log(struct every_number_info *pinfo)
{
	int bootstage;
	unsigned int bl_log_size;
	char *bl_addr = NULL;
	int ret;

	ret = qcom_get_boot_stage(&bootstage);
	if (ret != BF_OK) {
		print_err("get boot stage fail\n");
		bootstage = INVALID_STAGE;
	}
	pinfo->rtc_time = get_sys_rtc_time();
	pinfo->boot_time = get_bootup_time();
	pinfo->bootup_keypoint = bootstage;
	pinfo->reboot_type = KERNEL_SYSTEM_FREEZE;
	pinfo->exce_subtype = KERNEL_SYSTEM_FREEZE;

	pinfo->fastbootlog_start_addr = sizeof(struct every_number_info);
	bl_addr = rb_bl_log_get(&bl_log_size);
	if (bl_addr == NULL) {
		print_invalid_params("bl log params get fail\n");
		return;
	}
	pinfo->fastbootlog_size = bl_log_size;
	pinfo->last_kmsg_start_addr = pinfo->fastbootlog_start_addr +
		pinfo->fastbootlog_size;
	pinfo->last_kmsg_size = log_buf_len_get();
	pinfo->last_applog_start_addr = pinfo->last_kmsg_start_addr +
		pinfo->last_kmsg_size;
	pinfo->last_applog_size = APP_LOG_PATH_MAX_LENGTH;

	write_part(LONG_PRESS_DEV_PATH, 0, (const char *)pinfo,
		sizeof(struct every_number_info));
}

static void save_long_press_log(u32 type, u64 offset, u64 size)
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
		capture_bl_log(buf, size);
		break;
	case KMSGLOG_TYPE:
		capture_kernel_log(buf, size);
		break;
	case APPLOG_TYPE:
		capture_app_log(buf, size);
		break;
	default:
		print_err("log type %d invliad type\n", type);
		goto out;
	}

	write_part(LONG_PRESS_DEV_PATH, offset, buf, size);

out:
	vfree(buf);
}

void qcom_save_long_press_logs(void)
{
	struct every_number_info info;

	save_long_press_meta_log(&info);

	save_long_press_log(FASTBOOTLOG_TYPE, info.fastbootlog_start_addr,
		info.fastbootlog_size);
	save_long_press_log(KMSGLOG_TYPE, info.last_kmsg_start_addr,
		info.last_kmsg_size);
	save_long_press_log(APPLOG_TYPE, info.last_applog_start_addr,
		info.last_applog_size);
}

/**
 * @brief Init bootfail qcom adapter, include common init and platform init.
 * @param padp - bootfail qcom adapter pointer
 * @return 0 on success.
 * @since 1.0
 * @version 1.0
 */
int qcom_adapter_init(struct adapter *padp)
{
	if (common_adapter_init(padp) != 0) {
		print_err("init adapter common failed\n");
		return -1;
	}
	platform_adapter_init(padp);
	return 0;
}
