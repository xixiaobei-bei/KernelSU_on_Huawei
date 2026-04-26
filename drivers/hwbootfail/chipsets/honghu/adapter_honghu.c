/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: implement the platform interface for boot fail
 * Author: qidechun
 * Create: 2019-03-05
 */

/* ----includes ---- */
#include "adapter_honghu.h"
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mntn/mntn_public_interface.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/securec.h>
#include <mntn_public_interface.h>
#include <hwbootfail/core/boot_interface.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/chipsets/common/adapter_common.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>

/* ----local macroes ---- */
#define BFI_PART_SIZE (0x7E0000) /* 8MB -128 KB */
#define BFI_DEV_PATH "/dev/block/by-name/rrecord"
#define BL_LOG_NAME "fastboot_log"
#define KERNEL_LOG_NAME "last_kmsg"

/* ---- local prototypes ---- */
enum boot_detector_modid {
	MODID_BOOT_DETECTOR_START = HISI_BB_MOD_BFM_START,
	MODID_BOOT_FAIL,
	MODID_BOOT_DETECTOR_END = HISI_BB_MOD_BFM_END
};

/* ---- local static variables ---- */
struct rdr_exception_info_s boot_detector_excp_info[] = {
	{
		{0, 0}, (u32)HISI_BB_MOD_BFM_START, (u32)HISI_BB_MOD_BFM_END,
		RDR_ERR, RDR_REBOOT_NOW, RDR_AP | RDR_BFM, RDR_AP, RDR_AP,
		(u32)RDR_REENTRANT_DISALLOW, (u32)BFM_S_NATIVE_DATA_FAIL,
		0, (u32)RDR_UPLOAD_YES, "bfm", "bfm-reboot", 0, 0, 0
	},
};

/* ---- local function definitions --- */
static unsigned int stage_to_kp(enum boot_stage stage)
{
	if (stage >= BL1_STAGE_START && stage <= BL1_STAGE_END)
		return STAGE_XLOADER_START;
	else if (stage >= BL2_STAGE_START && stage <= BL2_STAGE_END)
		return STAGE_FASTBOOT_START;
	else if (stage >= KERNEL_STAGE_START && stage <= KERNEL_STAGE_END)
		return STAGE_KERNEL_BOOTANIM_COMPLETE;
	else if (stage >= NATIVE_STAGE_START && stage <= NATIVE_STAGE_END)
		return STAGE_INIT_INIT_START;
	else if (stage >= FRAMEWORK_STAGE_START && stage < BOOT_SUCC_STAGE)
		return STAGE_ANDROID_ZYGOTE_START;

	return STAGE_BOOTUP_END;
}

static enum boot_stage kp_to_stage(unsigned int kp)
{
	if (kp >= STAGE_XLOADER_START && kp <= STAGE_XLOADER_END)
		return BL1_STAGE;
	else if (kp >= STAGE_FASTBOOT_START && kp <= STAGE_FASTBOOT_END)
		return BL2_STAGE;
	else if (kp >= STAGE_KERNEL_EARLY_INITCALL &&
		kp < STAGE_INIT_INIT_START)
		return KERNEL_STAGE;
	else if (kp >= STAGE_INIT_INIT_START &&
		kp < STAGE_ANDROID_ZYGOTE_START)
		return NATIVE_STAGE;
	else if (kp >= STAGE_ANDROID_ZYGOTE_START &&
		kp < STAGE_BOOTUP_END)
		return FRAMEWORK_STAGE;
	else if (kp >= STAGE_BOOTUP_END)
		return BOOT_SUCC_STAGE;

	return INVALID_STAGE;
}

static int honghu_set_boot_stage(int stage)
{
	unsigned int kp = stage_to_kp((enum boot_stage)stage);

	set_boot_keypoint(kp);

	return 0;
}

static int honghu_get_boot_stage(int *stage)
{
	unsigned int kp;

	if (unlikely(stage == NULL)) {
		print_invalid_params("stage is NULL!\n");
		return -1;
	}

	kp = get_boot_keypoint();
	*stage = kp_to_stage(kp);

	return 0;
}

static void dump_callback(u32 modid,
	u32 etype, u64 coreid,	char *log_path, pfn_cb_dump_done pfn_cb)
{
	if (log_path == NULL) {
		print_err("%s: log_path is null\n", __func__);
		return;
	}

	print_info("bootfail Enter %s, log_path: %s\n", __func__, log_path);
}

static void reset_callback(u32 modid, u32 etype, u64 coreid)
{
	print_info("bootfail Enter %s, modid: %u etype: %u coreid:%llu.\n",
		__func__, modid, etype, coreid);
}

static int register_callbacks_to_rdr(void)
{
	struct rdr_module_ops_pub soc_ops;
	struct rdr_register_module_result retinfo;
	int ret;

	soc_ops.ops_dump = dump_callback;
	soc_ops.ops_reset = reset_callback;
	ret = rdr_register_module_ops(RDR_BFM, &soc_ops, &retinfo);
	if (ret != 0)
		print_err("rdr_register_module_ops failed! ret = [%d]\n", ret);

	return ret;
}

static void register_exceptions_to_rdr(void)
{
	unsigned int i;
	u32 ret;

	for (i = 0; i < array_size(boot_detector_excp_info); i++) {
		ret = rdr_register_exception(&boot_detector_excp_info[i]);
		if (ret == 0)
			print_err("rdr_register_exception failed! ret: %d\n", ret);
	}
}

static void honghu_reboot(void)
{
	(void)register_callbacks_to_rdr();
	register_exceptions_to_rdr();
	rdr_syserr_process_for_ap(MODID_BOOT_FAIL, 0, 0);
}

static void honghu_shutdown(void)
{
	print_info("%s: honghu not support\n", __func__);
}

void *bf_map(phys_addr_t paddr, size_t size)
{
	return hisi_bbox_map(paddr, size);
}

void bf_unmap(const void *vaddr)
{
	hisi_bbox_unmap(vaddr);
	return;
}

static void capture_bl_log(char *pbuf, unsigned int buf_size)
{
	unsigned int len;
	char *fastbootlog_addr = bf_map(HISI_SUB_RESERVED_FASTBOOT_LOG_PYHMEM_BASE,
		HISI_SUB_RESERVED_FASTBOOT_LOG_PYHMEM_SIZE);

	if (fastbootlog_addr == NULL) {
		print_err("bf_map fastboot log addr failed!\n");
		return;
	}

	len = min(buf_size, HISI_SUB_RESERVED_FASTBOOT_LOG_PYHMEM_SIZE);
	if (memcpy_s(pbuf, buf_size, fastbootlog_addr, len) != EOK)
		print_err("capture_bl_log memcpy_s buf failed!\n");

	bf_unmap(fastbootlog_addr);
}

static void capture_kernel_log(char *pbuf, unsigned int buf_size)
{
	void *pstore_addr = NULL;
	void *last_applog_addr = NULL;
	void *last_kmsg_addr = NULL;
	unsigned int len;

	pstore_addr = bf_map(HISI_RESERVED_PSTORE_PHYMEM_BASE,
		HISI_RESERVED_PSTORE_PHYMEM_SIZE);
	if (pstore_addr == NULL) {
		print_err("bf_vmap failed!\n");
		return;
	}
	last_applog_addr = pstore_addr + HISI_RESERVED_PSTORE_PHYMEM_SIZE -
		LAST_APPLOG_SIZE;
	last_kmsg_addr = last_applog_addr - LAST_KMSG_SIZE;
	len = min((unsigned int)LAST_KMSG_SIZE, buf_size);
	if (memcpy_s(pbuf, buf_size, last_kmsg_addr, len) != EOK)
		print_err("capture_kernel_log memcpy_s buf failed!\n");

	bf_unmap(pstore_addr);
}

char *get_bfi_dev_path(void)
{
	return BFI_DEV_PATH;
}

size_t get_bfi_part_size(void)
{
	return (size_t)BFI_PART_SIZE;
}

static void get_raw_part_info(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p is NULL\n", padp);
		return;
	}

	padp->bfi_part.dev_path = BFI_DEV_PATH;
	padp->bfi_part.part_size = (unsigned int)get_bfi_part_size();
}

static void get_boot_stage_ops(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p is NULL\n", padp);
		return;
	}

	padp->stage_ops.set_stage = honghu_set_boot_stage;
	padp->stage_ops.get_stage = honghu_get_boot_stage;
}

static void get_sysctl_ops(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p is NULL\n", padp);
		return;
	}

	padp->sys_ctl.reboot = honghu_reboot;
	padp->sys_ctl.shutdown = honghu_shutdown;
}

static void get_phys_mem_info(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p is NULL\n", padp);
		return;
	}

	padp->pyhs_mem_info.base = HISI_RESERVED_MNTN_DDR_BFMR_BASE;
	padp->pyhs_mem_info.size = BF_SIZE_1K;
}

static void get_log_ops_info(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p is NULL\n", padp);
		return;
	}

	strncpy(padp->bl_log_ops.log_name,
		BL_LOG_NAME, min(strlen(BL_LOG_NAME),
		sizeof(padp->bl_log_ops.log_name) - 1));
	padp->bl_log_ops.log_size = (unsigned int)
		HISI_SUB_RESERVED_FASTBOOT_LOG_PYHMEM_SIZE;
	padp->bl_log_ops.capture_bl_log = capture_bl_log;
	strncpy(padp->kernel_log_ops.log_name,
		KERNEL_LOG_NAME, min(strlen(KERNEL_LOG_NAME),
		sizeof(padp->kernel_log_ops.log_name) - 1));
	padp->kernel_log_ops.log_size = (unsigned int)LAST_KMSG_SIZE;
	padp->kernel_log_ops.capture_kernel_log = capture_kernel_log;
}

static void degrade(int excp_type)
{
	print_info("%s: honghu not support\n", __func__);
}

static void bypass(int excp_type)
{
	print_info("%s: honghu not support\n", __func__);
}

static void load_backup(const char *part_name)
{
	print_info("%s: honghu not support\n", __func__);
}

static void notify_storage_fault(unsigned long long bopd_mode)
{
	print_info("%s: honghu not support\n", __func__);
}

static int honghu_read_from_phys_mem(unsigned long dst,
	unsigned long dst_max, void *phys_mem_addr, unsigned long data_len)
{
	char *pdst = NULL;
	char *paddr = NULL;
	unsigned long i;
	size_t bytes_to_read;

	bytes_to_read = min(dst_max, data_len);
	/*lint -e446*/
	paddr = (char *)bf_map((phys_addr_t)(uintptr_t)phys_mem_addr,
		bytes_to_read);
	/*lint +e446*/
	if (paddr == NULL)
		return -1;

	pdst = (char *)(uintptr_t)dst;
	for (i = 0; i < bytes_to_read; i++) {
		*pdst = readb(paddr);
		pdst++;
		paddr++;
	}
	bf_unmap(paddr);

	return 0;
}

static int honghu_write_to_phys_mem(unsigned long dst,
	unsigned long dst_max, void *src, unsigned long src_len)
{
	char *psrc = NULL;
	char *paddr = NULL;
	unsigned long i;
	size_t bytes_to_write;

	bytes_to_write = min(dst_max, src_len);
	/*lint -e446*/
	paddr = (char *)bf_map((phys_addr_t)dst, bytes_to_write);
	/*lint +e446*/
	if (paddr == NULL)
		return -1;

	psrc = (char *)src;
	for (i = 0; i < bytes_to_write; i++) {
		writeb(*psrc, paddr);
		psrc++;
		paddr++;
	}
	bf_unmap(paddr);

	return 0;
}

static void platform_adapter_init(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p is NULL\n", padp);
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

static void rewrite_adapter_init(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p is NULL\n", padp);
		return;
	}

	/* common interface is not available, need rewrite for honghu */
	padp->pyhs_mem_info.ops.read = honghu_read_from_phys_mem;
	padp->pyhs_mem_info.ops.write = honghu_write_to_phys_mem;
}

int honghu_adapter_init(struct adapter *padp)
{
	if (padp == NULL) {
		print_invalid_params("padp: %p\n", padp);
		return -1;
	}

	if (common_adapter_init(padp) != 0) {
		print_err("init adapter common failed!\n");
		return -1;
	}
	platform_adapter_init(padp);
	rewrite_adapter_init(padp);

	return 0;
}
