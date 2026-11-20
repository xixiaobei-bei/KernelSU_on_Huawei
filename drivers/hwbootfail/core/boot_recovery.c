/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: implement the global interface for Boot Recovery
 * Author: qidechun
 * Create: 2019-03-05
 */

/* ----includes ---- */
#include <hwbootfail/core/boot_recovery.h>

/* ----local macroes ---- */
#define RECORD_HDR_SIZE (sizeof(struct rrecord_header))
#define RECORD1_OFFSET 0
#define RECORD_SIZE (RECORD_HDR_SIZE + \
	RECORD_COUNT * sizeof(struct rrecord))
#define RECORD2_OFFSET (RECORD1_OFFSET + RECORD_SIZE)
#define RECORD_SIZE_TOTAL (sizeof(struct recovery_info))
#define FAIL_COUNT_1 1
#define FAIL_COUNT_2 2
#define FAIL_COUNT_3 3
#define FAIL_COUNT_4 4
#define BOPD_BEFORE_DOWNLOAD 0x2 /* BOPD: Bootup on Pseudo Data */
#define BOPD_RDONLY_BEFORE_DOWNLOAD 0x3
#define BOPD_DEGRADE_BEFORE_DOWNLOAD 0x6
#define BOPD_RDONLY_DEGRADE_BEFORE_DOWNLOAD 0x7
#define BOPD_AFTER_DOWNLOAD 0xa
#define BOPD_DEGRADE_AFTER_DOWNLOAD 0xe
#define MAX_FAIL_COUNT 0X7FFFFFFF

/* ----local macro functions---- */
#define is_download(meth) (((meth) == BL1_DOWNLOAD) || \
	((meth) == BL2_DOWNLOAD) || ((meth) == KERNEL_DOWNLOAD) || \
	((meth) == NATIVE_DOWNLOAD) || \
	((meth) == FRAMEWORK_DOWNLOAD))
#define is_install_succ(status) ((status) == INSTALL_PACKAGE_SUCC)
#define is_download_succ(method, status) (is_download(method) && \
	is_install_succ(status))
#define is_fault_before_boot_succ(stage, bootfail_errno) \
	(((bootfail_errno) >= HARDWARE_ERRNO_START && \
	(bootfail_errno) <= HARDWARE_ERRNO_END) && \
	((stage) < BOOT_SUCC_STAGE))

/* ----local prototypes---- */
struct fixed_method_map {
	enum bootfail_errno bootfail_errno[3]; /* max count of stage */
	struct recovery_region {
		int start;
		int end;
		enum recovery_method method;
	} region[3]; /* max count of region */
};

struct errno_to_bi {
	struct {
		int start;
		int end;
	} range;

	/* this will be used by erecovery to communicate with BI server */
	unsigned int bi_code;
};

struct select_param {
	int count_before_succ;
	int count_after_succ;
	int count_total;
	int count_bottom;
	int count_upper;
	int count_after_download;
	int is_download_succ;
	int method;
	struct {
		char *pstart;
		int count;
	} record_info;
	struct recovery_run_param *rparam;
};

/* ----local static variables---- */
static struct recovery_info *prrecord;
static struct fixed_method_map fixed_method[] = {
	{
		{
			BL1_CORE_DEV_FAULT,
			BL1_NON_CORE_DEV_FAULT,
			BL1_IMG_FAULT
		},
		{
			/* 1 <= bootfail times < 2 */
			{ 1, 2, BL1_REBOOT },
			/* 2 <= bootfail times < 3 */
			{ 2, 3, BL1_PREVENTION },
			/* 3 <= bootfail times */
			{ 3, MAX_FAIL_COUNT, BL1_DOWNLOAD }
		},
	},
	{
		{
			BL2_CORE_DEV_FAULT,
			BL2_NON_CORE_DEV_FAULT,
			BL2_IMG_FAULT
		},
		{
			/* 1 <= bootfail times < 2 */
			{ 1, 2, BL2_REBOOT },
			/* 2 <= bootfail times < 3 */
			{ 2, 3, BL2_PREVENTION },
			/* 3 <= bootfail times */
			{ 3, MAX_FAIL_COUNT, BL2_DOWNLOAD }
		},
	},
	{
		{
			KERNEL_CORE_DEV_FAULT,
			KERNEL_NON_CORE_DEV_FAULT,
			KERNEL_IMG_FAULT
		},
		{
			/* 1 <= bootfail times < 2 */
			{ 1, 2, KERNEL_REBOOT },
			/* 2 <= bootfail times < 3 */
			{ 2, 3, KERNEL_PREVENTION },
			/* 3 <= bootfail times */
			{ 3, MAX_FAIL_COUNT, KERNEL_DOWNLOAD }
		},
	},
	{
		{
			NATIVE_CORE_DEV_FAULT,
			NATIVE_NON_CORE_DEV_FAULT,
			NATIVE_NON_CORE_DEV_FAULT
		},
		{
			/* 1 <= bootfail times < 2 */
			{ 1, 2, NATIVE_REBOOT },
			/* 2 <= bootfail times < 3 */
			{ 2, 3, NATIVE_PREVENTION },
			/* 3 <= bootfail times */
			{ 3, MAX_FAIL_COUNT, NATIVE_DOWNLOAD }
		},
	},
	{
		{
			NATIVE_PART_MOUNT_FAIL,
			NATIVE_PART_MOUNT_FAIL,
			NATIVE_PART_MOUNT_FAIL
		},
		{
			/* 1 <= bootfail times < 2 */
			{ 1, 2, NATIVE_REBOOT },
			/* 2 <= bootfail times < 3 */
			{ 2, 3, NATIVE_DOWNLOAD },
			/* 3 <= bootfail times */
			{ 3, MAX_FAIL_COUNT, NATIVE_DOWNLOAD }
		},
	},
	{
		{
			NATIVE_DATA_PART_MOUNT_FAIL,
			NATIVE_DATA_PART_MOUNT_FAIL,
			NATIVE_DATA_PART_MOUNT_FAIL
		},
		{
			/* 1 <= bootfail times < 2 */
			{ 1, 2, NATIVE_REBOOT },
			/* 2 <= bootfail times < 3 */
			{ 2, 3, NATIVE_FACTORY_REST },
			/* 3 <= bootfail times */
			{ 3, MAX_FAIL_COUNT, NATIVE_FACTORY_REST }
		},
	},
	{
		{
			NATIVE_DATA_DAMAGED,
			NATIVE_DATA_DAMAGED,
			NATIVE_DATA_DAMAGED
		},
		{
			/* 1 <= bootfail times < 2 */
			{ 1, 2, NATIVE_REBOOT },
			/* 2 <= bootfail times < 3 */
			{ 2, 3, NATIVE_DOWNLOAD },
			/* 3 <= bootfail times */
			{ 3, MAX_FAIL_COUNT, NATIVE_DOWNLOAD }
		},
	},
	{
		{
			FRAMEWORK_CORE_DEV_FAULT,
			FRAMEWORK_NON_CORE_DEV_FAULT,
			FRAMEWORK_NON_CORE_DEV_FAULT
		},
		{
			/* 1 <= bootfail times < 2 */
			{ 1, 2, FRAMEWORK_REBOOT },
			/* 2 <= bootfail times < 3 */
			{ 2, 3, FRAMEWORK_PREVENTION },
			/* 3 <= bootfail times */
			{ 3, MAX_FAIL_COUNT, FRAMEWORK_DOWNLOAD }
		},
	},
	{
		{
			HARDWARE_STORAGE_FAULT,
			HARDWARE_STORAGE_FAULT,
			HARDWARE_STORAGE_FAULT
		},
		{
			/* 1 <= bootfail times < 3 */
			{ 1, 3, KERNEL_REBOOT },
			/* 3 <= bootfail times < 4 */
			{ 3, 4, KERNEL_UFS_RDONLY_BOPD },
			/* 4 <= bootfail times */
			{ 4, MAX_FAIL_COUNT, BL2_UFS_RDONLY_DEGRADE_BOPD }
		},
	},
};

static struct errno_to_bi errno_to_bi_tbl[] = {
	{{ BL1_CORE_DEV_FAULT, BL1_CORE_DEV_FAULT }, BI_DEG_BOOT_FAIL },
	{{ BL1_IMG_FAULT, BL1_IMG_FAULT }, BI_DEG_BOOT_FAIL },
	{{ BL1_ERRNO_START, BL1_ERRNO_END }, BI_BL_BOOT_FAIL },
	{{ BL2_CORE_DEV_FAULT, BL2_CORE_DEV_FAULT }, BI_DEG_BOOT_FAIL },
	{{ BL2_IMG_FAULT, BL2_IMG_FAULT }, BI_DEG_BOOT_FAIL },
	{{ BL2_ERRNO_START, BL2_ERRNO_END }, BI_BL_BOOT_FAIL },
	{{ KERNEL_CORE_DEV_FAULT, KERNEL_CORE_DEV_FAULT }, BI_DEG_BOOT_FAIL },
	{{ KERNEL_IMG_FAULT, KERNEL_IMG_FAULT }, BI_DEG_BOOT_FAIL },
	{{ KERNEL_ERRNO_START, KERNEL_ERRNO_END }, BI_KERNEL_BOOT_FAIL },
	{{ NATIVE_CORE_DEV_FAULT, NATIVE_CORE_DEV_FAULT }, BI_DEG_BOOT_FAIL },
	{{ NATIVE_DATA_DAMAGED, NATIVE_DATA_DAMAGED }, BI_DEG_BOOT_FAIL },
	{{ NATIVE_PART_MOUNT_FAIL, NATIVE_PART_MOUNT_FAIL },
		BI_DEG_BOOT_FAIL },
	{{ NATIVE_SERVICE_CRASH, NATIVE_SERVICE_CRASH },
		BI_KEY_SVC_START_FAIL },
	{{ NATIVE_DATA_PART_MOUNT_FAIL, NATIVE_DATA_PART_MOUNT_FAIL },
		BI_DATA_MNT_FAIL },
	{{ NATIVE_DATA_PART_MOUNT_RO, NATIVE_DATA_PART_MOUNT_RO },
		BI_DATA_MNT_RO },
	{{ NATIVE_ERRNO_START, NATIVE_ERRNO_END }, BI_NATIVE_BOOT_FAIL },
	{{ FRAMEWORK_CORE_DEV_FAULT, FRAMEWORK_CORE_DEV_FAULT },
		BI_NATIVE_BOOT_FAIL },
	{{ AFTER_BOOT_SUCC_ERRNO_START, ERRNO_MAX_COUNT }, BI_UNKNOWN },
};

/* ---- function definitions ---- */
static enum recovery_method get_do_nothing_code(int stage)
{
	if (is_bl1_stage(stage))
		return BL1_DO_NOTHING;
	else if (is_bl2_stage(stage))
		return BL2_DO_NOTHING;
	else if (is_kernel_stage(stage))
		return KERNEL_DO_NOTHING;
	else if (is_native_stage(stage))
		return NATIVE_DO_NOTHING;

	return FRAMEWORK_DO_NOTHING;
}

static void validate_rrecord(struct recovery_run_param *pparam)
{
	struct rrecord_header *phdr = (struct rrecord_header *)&prrecord->hdr;
	struct rrecord_header *pbackup_hdr = (struct rrecord_header *)
		&prrecord->backup_hdr;

	pparam->padp->sec_funcs.memset_s(prrecord,
		(unsigned long)RECORD_SIZE_TOTAL, 0,
		(unsigned long)RECORD_SIZE_TOTAL);
	phdr->record_count = RECORD_COUNT;
	pparam->padp->sha256(phdr->sha256, sizeof(phdr->sha256),
		(unsigned char *)phdr + sizeof(unsigned int),
		RECORD_SIZE - sizeof(unsigned int));
	pparam->padp->sec_funcs.memcpy_s(pbackup_hdr,
		(unsigned long)RECORD_SIZE, phdr,
		(unsigned long)RECORD_SIZE);
}

static enum bootfail_errorcode verify_rrecord(struct recovery_run_param *pparam)
{
	unsigned char sha256[BF_SIZE_32];
	unsigned char sha256_back[BF_SIZE_32];
	struct rrecord_header *phdr = (struct rrecord_header *)&prrecord->hdr;
	struct rrecord_header *pbackup_hdr = (struct rrecord_header *)
		&prrecord->backup_hdr;

	pparam->padp->sec_funcs.memset_s(sha256,
		(unsigned long)sizeof(sha256), 0,
		(unsigned long)sizeof(sha256));
	pparam->padp->sec_funcs.memset_s(sha256_back,
		(unsigned long)sizeof(sha256_back), 0,
		(unsigned long)sizeof(sha256_back));
	pparam->padp->sha256(sha256, sizeof(sha256), (unsigned char *)phdr +
		sizeof(phdr->sha256), RECORD_SIZE - sizeof(phdr->sha256));
	pparam->padp->sha256(sha256_back, sizeof(sha256_back),
		(unsigned char *)pbackup_hdr + sizeof(pbackup_hdr->sha256),
		RECORD_SIZE - sizeof(pbackup_hdr->sha256));
	if (pparam->padp->sec_funcs.memcmp(sha256, phdr->sha256,
		sizeof(phdr->sha256)) != 0 &&
		pparam->padp->sec_funcs.memcmp(sha256_back,
		pbackup_hdr->sha256, sizeof(phdr->sha256)) != 0)
		return BF_RRECORD_DAMAGED;

	/* correct the wrong record according to the right one */
	if (pparam->padp->sec_funcs.memcmp(sha256, phdr->sha256,
		sizeof(phdr->sha256)) == 0)
		pparam->padp->sec_funcs.memcpy_s(pbackup_hdr,
			(unsigned long)RECORD_SIZE, phdr,
			(unsigned long)RECORD_SIZE);
	else
		pparam->padp->sec_funcs.memcpy_s(phdr,
			(unsigned long)RECORD_SIZE, pbackup_hdr,
			(unsigned long)RECORD_SIZE);

	return BF_OK;
}

static int get_bi_code(const struct recovery_run_param *pparam)
{
	int i;
	int bootfail_errno = pparam->pparam->binfo.bootfail_errno;
	int bi_code = BI_UNKNOWN;

	for (i = 0; i < (int)array_size(errno_to_bi_tbl); i++) {
		if (bootfail_errno >= errno_to_bi_tbl[i].range.start &&
			bootfail_errno <= errno_to_bi_tbl[i].range.end) {
			bi_code = errno_to_bi_tbl[i].bi_code;
			break;
		}
	}

	if (pparam->deg_fail != 0)
		bi_code = BI_DEG_BOOT_FAIL;

	return bi_code;
}

static int is_bopd(enum recovery_method method)
{
	return (method == KERNEL_BOPD ||
		method == NATIVE_BOPD ||
		method == FRAMEWORK_BOPD);
}

static int is_rdonly_bopd(enum recovery_method method)
{
	return (method == KERNEL_UFS_RDONLY_BOPD ||
		method == NATIVE_UFS_RDONLY_BOPD ||
		method == FRAMEWORK_UFS_RDONLY_BOPD);
}

static int is_degrade_bopd(enum recovery_method method)
{
	return (method == KERNEL_DEGRADE_BOPD ||
		method == NATIVE_DEGRADE_BOPD ||
		method == FRAMEWORK_DEGRADE_BOPD);
}

static int is_rdonly_with_degrade_bopd(enum recovery_method method)
{
	return (method == KERNEL_UFS_RDONLY_DEGRADE_BOPD ||
		method == NATIVE_UFS_RDONLY_DEGRADE_BOPD ||
		method == FRAMEWORK_UFS_RDONLY_DEGRADE_BOPD);
}

static int is_bopd_after_download(enum recovery_method method)
{
	return (method == KERNEL_BOPD_AFTER_DOWNLOAD ||
		method == NATIVE_BOPD_AFTER_DOWNLOAD ||
		method == FRAMEWORK_BOPD_AFTER_DOWNLOAD);
}

static int is_degrade_bopd_after_download(enum recovery_method method)
{
	return (method == KERNEL_DEGRADE_BOPD_AFTER_DOWNLOAD ||
		method == NATIVE_DEGRADE_BOPD_AFTER_DOWNLOAD ||
		method == FRAMEWORK_DEGRADE_BOPD_AFTER_DOWNLOAD);
}

static unsigned long long get_bopd_mode(int method)
{
	unsigned long long bopd_mode = 0ULL;

	if (is_bopd(method) != 0)
		bopd_mode = BOPD_BEFORE_DOWNLOAD;
	else if (is_rdonly_bopd(method) != 0)
		bopd_mode = BOPD_RDONLY_BEFORE_DOWNLOAD;
	else if (is_degrade_bopd(method) != 0)
		bopd_mode = BOPD_DEGRADE_BEFORE_DOWNLOAD;
	else if (is_rdonly_with_degrade_bopd(method) != 0)
		bopd_mode = BOPD_RDONLY_DEGRADE_BEFORE_DOWNLOAD;
	else if (is_bopd_after_download(method) != 0)
		bopd_mode = BOPD_AFTER_DOWNLOAD;
	else if (is_degrade_bopd_after_download(method) != 0)
		bopd_mode = BOPD_DEGRADE_AFTER_DOWNLOAD;

	return bopd_mode;
}

static enum bootfail_errorcode run_erecovery(struct recovery_run_param *pparam)
{
	union bfi_part_header hdr;

	pparam->padp->sec_funcs.memset_s(&hdr,
		(unsigned long)sizeof(hdr), 0,
		(unsigned long)sizeof(hdr));
	/*lint -e666*/
	pparam->padp->sec_funcs.strncpy_s(hdr.misc_msg.command,
		(unsigned long)sizeof(hdr.misc_msg.command), BOOT_ERECOVERY,
		(unsigned long)min(sizeof(hdr.misc_msg.command) - 1,
		pparam->padp->sec_funcs.strlen(BOOT_ERECOVERY)));
	/*lint +e666*/
	hdr.misc_msg.bi_code = get_bi_code(pparam);
	hdr.misc_msg.bootfail_errno = pparam->pparam->binfo.bootfail_errno;
	hdr.misc_msg.stage = pparam->pparam->binfo.stage;
	hdr.misc_msg.orig_method = pparam->method;
	pparam->padp->save_misc_msg(BOOT_ERECOVERY);
	return pparam->padp->bfi_part.ops.write(
		pparam->padp->bfi_part.dev_path, 0,
		(char *)&hdr, sizeof(hdr));
}

static enum bootfail_errorcode run_bopd(struct recovery_run_param *pparam)
{
	union bfi_part_header hdr;

	pparam->padp->sec_funcs.memset_s(&hdr,
		(unsigned long)sizeof(hdr), 0,
		(unsigned long)sizeof(hdr));
	/*lint -e666*/
	pparam->padp->sec_funcs.strncpy_s(hdr.misc_msg.command,
		(unsigned long)sizeof(hdr.misc_msg.command), BOOT_BOPD,
		(unsigned long)min(sizeof(hdr.misc_msg.command) - 1,
		pparam->padp->sec_funcs.strlen(BOOT_BOPD)));
	/*lint +e666*/
	hdr.misc_msg.bootfail_errno = pparam->pparam->binfo.bootfail_errno;
	hdr.misc_msg.bopd_run_mode = get_bopd_mode(pparam->method);
	return pparam->padp->bfi_part.ops.write(
		pparam->padp->bfi_part.dev_path, 0,
		(char *)&hdr, sizeof(hdr));
}

static enum bootfail_errorcode run_safe_mode(struct recovery_run_param *pparam)
{
	union bfi_part_header hdr;

	pparam->padp->sec_funcs.memset_s(&hdr,
		(unsigned long)sizeof(hdr), 0,
		(unsigned long)sizeof(hdr));
	/*lint -e666*/
	pparam->padp->sec_funcs.strncpy_s(hdr.misc_msg.command,
		(unsigned long)sizeof(hdr.misc_msg.command), BOOT_SAFEMODE,
		(unsigned long)min(sizeof(hdr.misc_msg.command) - 1,
		pparam->padp->sec_funcs.strlen(BOOT_SAFEMODE)));
	/*lint +e666*/
	hdr.misc_msg.bootfail_errno = pparam->pparam->binfo.bootfail_errno;
	return pparam->padp->bfi_part.ops.write(
		pparam->padp->bfi_part.dev_path, 0, (char *)&hdr, sizeof(hdr));
}

static int need_degrade(enum bootfail_errno bootfail_errno)
{
	return (bootfail_errno == BL1_CORE_DEV_FAULT ||
		bootfail_errno == BL2_CORE_DEV_FAULT ||
		bootfail_errno == KERNEL_CORE_DEV_FAULT ||
		bootfail_errno == NATIVE_CORE_DEV_FAULT ||
		bootfail_errno == FRAMEWORK_CORE_DEV_FAULT);
}

static int need_bypass(enum bootfail_errno bootfail_errno)
{
	return (bootfail_errno == BL1_NON_CORE_DEV_FAULT ||
		bootfail_errno == BL2_NON_CORE_DEV_FAULT ||
		bootfail_errno == KERNEL_NON_CORE_DEV_FAULT ||
		bootfail_errno == NATIVE_NON_CORE_DEV_FAULT ||
		bootfail_errno == FRAMEWORK_NON_CORE_DEV_FAULT ||
		bootfail_errno == HARDWARE_STORAGE_FAULT);
}

static int need_load_backup_image(enum bootfail_errno bootfail_errno)
{
	return (bootfail_errno == BL1_IMG_FAULT ||
		bootfail_errno == BL2_IMG_FAULT ||
		bootfail_errno == KERNEL_IMG_FAULT);
}

static void run_prevention(struct recovery_run_param *pparam)
{
	if (need_degrade(pparam->pparam->binfo.bootfail_errno)) {
		if (pparam->padp->prevent.degrade != NULL)
			pparam->padp->prevent.degrade(
				pparam->pparam->binfo.exception_type);
	} else if (need_bypass(pparam->pparam->binfo.bootfail_errno)) {
		if (pparam->padp->prevent.bypass != NULL)
			pparam->padp->prevent.bypass(
				pparam->pparam->binfo.exception_type);
	} else if (need_load_backup_image(
		pparam->pparam->binfo.bootfail_errno)) {
		if (pparam->padp->prevent.load_backup != NULL)
			pparam->padp->prevent.load_backup(
				pparam->pparam->part_name);
	}
}

static int is_prevention(enum recovery_method method)
{
	return (method == BL1_PREVENTION ||
		method == BL2_PREVENTION ||
		method == KERNEL_PREVENTION ||
		method == NATIVE_PREVENTION ||
		method == FRAMEWORK_PREVENTION);
}

static int is_erecovery(enum recovery_method method)
{
	return (method == BL1_DOWNLOAD ||
		method == BL2_DOWNLOAD ||
		method == KERNEL_FACTORY_REST ||
		method == KERNEL_DOWNLOAD ||
		method == NATIVE_FACTORY_REST ||
		method == NATIVE_DOWNLOAD ||
		method == FRAMEWORK_FACTORY_REST ||
		method == FRAMEWORK_DOWNLOAD);
}

static int is_kernel_bopd(enum recovery_method method)
{
	return (method == KERNEL_BOPD ||
		method == KERNEL_UFS_RDONLY_BOPD ||
		method == KERNEL_DEGRADE_BOPD ||
		method == KERNEL_BOPD_AFTER_DOWNLOAD ||
		method == KERNEL_DEGRADE_BOPD_AFTER_DOWNLOAD ||
		method == KERNEL_UFS_RDONLY_DEGRADE_BOPD);
}

static int is_native_bopd(enum recovery_method method)
{
	return (method == NATIVE_BOPD ||
		method == NATIVE_UFS_RDONLY_BOPD ||
		method == NATIVE_DEGRADE_BOPD ||
		method == NATIVE_BOPD_AFTER_DOWNLOAD ||
		method == NATIVE_DEGRADE_BOPD_AFTER_DOWNLOAD ||
		method == NATIVE_UFS_RDONLY_DEGRADE_BOPD);
}

static int is_framework_bopd(enum recovery_method method)
{
	return (method == FRAMEWORK_BOPD ||
		method == FRAMEWORK_UFS_RDONLY_BOPD ||
		method == FRAMEWORK_DEGRADE_BOPD ||
		method == FRAMEWORK_BOPD_AFTER_DOWNLOAD ||
		method == FRAMEWORK_DEGRADE_BOPD_AFTER_DOWNLOAD ||
		method == FRAMEWORK_UFS_RDONLY_DEGRADE_BOPD);
}

static int is_safe_mode(enum recovery_method method)
{
	return (method == FRAMEWORK_SAFE_MODE ||
		method == NATIVE_SAFE_MODE ||
		method == KERNEL_SAFE_MODE);
}

static enum bootfail_errorcode run_method(struct recovery_run_param *pparam)
{
	enum bootfail_errorcode errorcode = BF_OK;

	if (is_prevention(pparam->method) != 0)
		pparam->run_prevention = run_prevention;
	else if (is_erecovery(pparam->method) != 0)
		errorcode = run_erecovery(pparam);
	else if (is_kernel_bopd(pparam->method) != 0 ||
		is_native_bopd(pparam->method) != 0 ||
		is_framework_bopd(pparam->method) != 0)
		errorcode = run_bopd(pparam);
	else if (is_safe_mode(pparam->method))
		errorcode = run_safe_mode(pparam);

	return errorcode;
}

static enum bootfail_errorcode save_rrecord(struct recovery_run_param *pparam)
{
	struct rrecord_header *phdr = (struct rrecord_header *)prrecord;
	struct rrecord_header *pbackup_hdr = (struct rrecord_header *)(
		(char *)prrecord + RECORD2_OFFSET);
	struct rrecord *precord = (struct rrecord *)((char *)phdr +
		RECORD_HDR_SIZE + phdr->write_idx * sizeof(*precord));

	phdr->bootfail_count++;
	phdr->write_idx++;
	phdr->count_before_succ++;
	if (phdr->count_before_succ > RECORD_COUNT)
		phdr->count_before_succ = RECORD_COUNT;
	phdr->write_idx = phdr->write_idx % phdr->record_count;
	precord->rtc_time = pparam->pparam->binfo.rtc_time;
	precord->bootfail_errno = pparam->pparam->binfo.bootfail_errno;
	precord->stage = pparam->pparam->binfo.stage;
	precord->orig_method = pparam->method;
	precord->real_method = precord->orig_method;
	precord->result = RECOVERY_FAIL;
	precord->status = RECOVERY_START;
	pparam->padp->sha256(phdr->sha256, sizeof(phdr->sha256),
		(unsigned char *)phdr + sizeof(phdr->sha256),
		RECORD_SIZE - sizeof(phdr->sha256));
	pparam->padp->sec_funcs.memcpy_s(pbackup_hdr,
		(unsigned long)RECORD_SIZE, phdr,
		(unsigned long)RECORD_SIZE);

	return (pparam->padp->bfi_part.ops.write(
		pparam->padp->bfi_part.dev_path, pparam->rrecord_offset,
		(char *)prrecord, RECORD_SIZE_TOTAL) == 0) ?
		(BF_OK) : (BF_WRITE_PART_FAIL);
}

static int select_fixed_method(struct select_param *pparam)
{
	int i;
	int j;

	for (i = 0; i < (int)array_size(fixed_method); i++) {
		int is_errno_matched = 0;
		for (j = 0; j < (int)array_size(fixed_method[i].bootfail_errno);
			j++) {
			if (pparam->rparam->pparam->binfo.bootfail_errno ==
				(int)fixed_method[i].bootfail_errno[j]) {
				is_errno_matched = 1;
				break;
			}
		}

		if (is_errno_matched == 0)
			continue;

		for (j = 0; j < (int)array_size(fixed_method[i].region); j++) {
			if (pparam->count_total >=
				fixed_method[i].region[j].start &&
				pparam->count_total <
				fixed_method[i].region[j].end) {
				pparam->rparam->method =
					fixed_method[i].region[j].method;
				return 1;
			}
		}
	}

	return 0;
}

static void get_reboot_code(struct select_param *pparam)
{
	if (is_bl1_stage(pparam->rparam->pparam->binfo.stage))
		pparam->rparam->method = BL1_REBOOT;
	else if (is_bl2_stage(pparam->rparam->pparam->binfo.stage))
		pparam->rparam->method = BL2_REBOOT;
	else if (is_kernel_stage(pparam->rparam->pparam->binfo.stage))
		pparam->rparam->method = KERNEL_REBOOT;
	else if (is_native_stage(pparam->rparam->pparam->binfo.stage))
		pparam->rparam->method = NATIVE_REBOOT;
	else
		pparam->rparam->method = FRAMEWORK_REBOOT;
}

static void get_bopd_code(struct select_param *pparam)
{
	if (is_kernel_stage(pparam->rparam->pparam->binfo.stage)) {
		pparam->rparam->method = KERNEL_BOPD;
		return;
	} else if (is_native_stage(pparam->rparam->pparam->binfo.stage)) {
		pparam->rparam->method = NATIVE_BOPD;
		return;
	} else if (is_framework_stage(pparam->rparam->pparam->binfo.stage)) {
		pparam->rparam->method = FRAMEWORK_BOPD;
		return;
	}

	pparam->rparam->method = RECOVERY_METHOD_UNKNOWN;
}

static void get_degrade_bopd_code(struct select_param *pparam)
{
	if (is_kernel_stage(pparam->rparam->pparam->binfo.stage)) {
		pparam->rparam->method = KERNEL_DEGRADE_BOPD;
		return;
	} else if (is_native_stage(pparam->rparam->pparam->binfo.stage)) {
		pparam->rparam->method = NATIVE_DEGRADE_BOPD;
		return;
	} else if (is_framework_stage(pparam->rparam->pparam->binfo.stage)) {
		pparam->rparam->method = FRAMEWORK_DEGRADE_BOPD;
		return;
	}

	pparam->rparam->method = RECOVERY_METHOD_UNKNOWN;
}

static void get_dowload_code(struct select_param *pparam)
{
	if (is_bl1_stage(pparam->rparam->pparam->binfo.stage))
		pparam->rparam->method = BL1_DOWNLOAD;
	else if (is_bl2_stage(pparam->rparam->pparam->binfo.stage))
		pparam->rparam->method = BL2_DOWNLOAD;
	else if (is_kernel_stage(pparam->rparam->pparam->binfo.stage))
		pparam->rparam->method = KERNEL_DOWNLOAD;
	else if (is_native_stage(pparam->rparam->pparam->binfo.stage))
		pparam->rparam->method = NATIVE_DOWNLOAD;
	else
		pparam->rparam->method = FRAMEWORK_DOWNLOAD;
}

static void get_bopd_code_after_download(struct select_param *pparam)
{
	if (pparam->count_after_download == FAIL_COUNT_1) {
		if (is_native_stage(pparam->rparam->pparam->binfo.stage))
			pparam->rparam->method = NATIVE_BOPD_AFTER_DOWNLOAD;
		else if (is_framework_stage(
			pparam->rparam->pparam->binfo.stage))
			pparam->rparam->method = FRAMEWORK_BOPD_AFTER_DOWNLOAD;
		else
			pparam->rparam->method = KERNEL_BOPD_AFTER_DOWNLOAD;
		return;
	}

	if (is_native_stage(pparam->rparam->pparam->binfo.stage))
		pparam->rparam->method = NATIVE_DEGRADE_BOPD_AFTER_DOWNLOAD;
	else if (is_framework_stage(pparam->rparam->pparam->binfo.stage))
		pparam->rparam->method = FRAMEWORK_DEGRADE_BOPD_AFTER_DOWNLOAD;
	else
		pparam->rparam->method = KERNEL_DEGRADE_BOPD_AFTER_DOWNLOAD;
}

static void get_safe_mode_code(struct select_param *pparam)
{
	if (is_native_stage(pparam->rparam->pparam->binfo.stage)) {
		pparam->rparam->method = NATIVE_SAFE_MODE;
		return;
	} else if (is_framework_stage(pparam->rparam->pparam->binfo.stage)) {
		pparam->rparam->method = FRAMEWORK_SAFE_MODE;
		return;
	}

	pparam->rparam->method = RECOVERY_METHOD_UNKNOWN;
}

static void get_factory_reset_code(struct select_param *pparam)
{
	if (is_native_stage(pparam->rparam->pparam->binfo.stage)) {
		pparam->rparam->method = NATIVE_FACTORY_REST;
		return;
	} else if (is_framework_stage(pparam->rparam->pparam->binfo.stage)) {
		pparam->rparam->method = FRAMEWORK_FACTORY_REST;
		return;
	}

	pparam->rparam->method = RECOVERY_METHOD_UNKNOWN;
}

static void parse_kernel_bootfail_further(struct select_param *pparam)
{
	int i;

	for (i = 0; i < (int)array_size(prrecord->rrecord); i++) {
		struct rrecord *precord = (struct rrecord *)(
			&prrecord->rrecord[i]);

		if (precord->bootfail_errno !=
			pparam->rparam->pparam->binfo.bootfail_errno)
			continue;
		if (is_fault_before_boot_succ(precord->stage,
			precord->bootfail_errno)) {
			pparam->count_before_succ++;
			continue;
		}
		pparam->count_after_succ++;
	}
}

static void parse_bootloader_bootfail_further(struct select_param *pparam)
{
	int i;

	for (i = 0; i < pparam->record_info.count; i++) {
		struct rrecord *precord = (struct rrecord *)(
			pparam->record_info.pstart + i *
			sizeof(struct rrecord));

		if (pparam->is_download_succ != 0)
			pparam->count_after_download++;
		if (is_download_succ(precord->real_method, precord->status))
			pparam->is_download_succ = 1;

		if (precord->stage < KERNEL_STAGE_START)
			pparam->count_bottom++;
		else
			pparam->count_upper++;
		pparam->count_total++;
	}
}

static void parse_kernel_bootfail(struct select_param *pparam)
{
	parse_kernel_bootfail_further(pparam);
	if (is_fault_before_boot_succ(pparam->rparam->pparam->binfo.stage,
		pparam->rparam->pparam->binfo.bootfail_errno)) {
		if (pparam->count_before_succ == 0)
			get_reboot_code(pparam);
	}
}

static void parse_bootloader_bootfail(struct select_param *pparam)
{
	int bootfail_count;
	struct rrecord_header *phdr = (struct rrecord_header *)&prrecord->hdr;

	/* parse the existed boot fail info */
	pparam->count_bottom = 0;
	bootfail_count = min(phdr->count_before_succ, phdr->record_count);
	if (phdr->write_idx >= bootfail_count) {
		pparam->record_info.pstart = (char *)prrecord +
			RECORD_HDR_SIZE + (phdr->write_idx - bootfail_count) *
		sizeof(struct rrecord);
		pparam->record_info.count = bootfail_count;
		parse_bootloader_bootfail_further(pparam);
	} else {
		int count_of_end = bootfail_count - phdr->write_idx;
		int count_of_begin = phdr->write_idx;

		pparam->record_info.pstart = (char *)prrecord +
			RECORD_SIZE - count_of_end * sizeof(struct rrecord);
		pparam->record_info.count = count_of_end;
		parse_bootloader_bootfail_further(pparam);
		pparam->record_info.pstart = (char *)prrecord + RECORD_HDR_SIZE;
		pparam->record_info.count = count_of_begin;
		parse_bootloader_bootfail_further(pparam);
	}

	/* parse the current boot fail info */
	if (pparam->is_download_succ != 0)
		pparam->count_after_download++;
	pparam->count_total++;
	if (is_upper_stage(pparam->rparam->pparam->binfo.stage))
		pparam->count_upper++;
	else
		pparam->count_bottom++;
}

static void parse_bootfail(struct select_param *pparam)
{
	if (pparam->rparam->padp->is_bootloader() != 0)
		parse_bootloader_bootfail(pparam);
	else
		parse_kernel_bootfail(pparam);
}

static void select_upper_method_with_bopd(
	struct select_param *pparam)
{
	switch (pparam->count_upper) {
	case FAIL_COUNT_1:
	case FAIL_COUNT_2:
		get_reboot_code(pparam);
		break;
	case FAIL_COUNT_3:
		get_bopd_code(pparam);
		break;
	case FAIL_COUNT_4:
		get_degrade_bopd_code(pparam);
		break;
	default:
		pparam->rparam->deg_fail = 1;
		get_dowload_code(pparam);
		break;
	}
}

static void select_bottom_method_with_bopd(
	struct select_param *pparam)
{
	switch (pparam->count_bottom) {
	case FAIL_COUNT_1:
	case FAIL_COUNT_2:
		get_reboot_code(pparam);
		break;
	default:
		get_dowload_code(pparam);
		break;
	}
}

static enum bootfail_errorcode select_method_with_bopd(
	struct select_param *pparam)
{
	if (pparam->is_download_succ != 0) {
		pparam->rparam->padp->print("func: select_method_with_bopd, "
			"download succ, so do BoPD\n");
		get_bopd_code_after_download(pparam);
		return BF_OK;
	}

	/* process interleave bootfail */
	pparam->rparam->padp->print("func: select_method_with_bopd, "
		"bootfail in bottom: %d, bootfail in upper: %d\n",
		pparam->count_bottom, pparam->count_upper);
	if ((pparam->count_bottom != 0) && (pparam->count_upper != 0)) {
		get_dowload_code(pparam);
		return BF_OK;
	}

	if (pparam->count_upper > 0)
		select_upper_method_with_bopd(pparam);
	else
		select_bottom_method_with_bopd(pparam);

	return BF_OK;
}

static int need_safe_mode(const struct bootfail_proc_param *pparam)
{
	return (pparam->binfo.bootfail_errno == NATIVE_SERVICE_CRASH ||
		pparam->binfo.bootfail_errno == NATIVE_SERVICE_FREEZE ||
		pparam->binfo.bootfail_errno == NATIVE_SUBSYS_FAULT ||
		pparam->binfo.bootfail_errno == NATIVE_SYSTEM_CRASH ||
		pparam->binfo.bootfail_errno == NATIVE_SYSTEM_FREEZE ||
		pparam->binfo.bootfail_errno == FRAMEWORK_SERVICE_CRASH ||
		pparam->binfo.bootfail_errno == FRAMEWORK_SERVICE_FREEZE ||
		pparam->binfo.bootfail_errno == FRAMEWORK_SUBSYS_FAULT ||
		pparam->binfo.bootfail_errno == FRAMEWORK_SYSTEM_CRASH ||
		pparam->binfo.bootfail_errno == FRAMEWORK_SYSTEM_FREEZE);
}

static enum bootfail_errorcode select_method_without_bopd(
	struct select_param *pparam)
{
	if (pparam->is_download_succ != 0) {
		pparam->rparam->padp->print("func: "
			"select_method_without_bopd, "
			"download succ, so do factory reset\n");
		get_factory_reset_code(pparam);
		return BF_OK;
	}

	if ((pparam->count_bottom != 0) && (pparam->count_upper != 0)) {
		pparam->rparam->padp->print("func: "
			"select_method_without_bopd, "
			"bootfail in bottom: %d, bootfail in upper: %d, "
			"so do download\n", pparam->count_bottom,
			pparam->count_upper);
		get_dowload_code(pparam);
		return BF_OK;
	}

	switch (pparam->count_upper) {
	case FAIL_COUNT_1:
		get_reboot_code(pparam);
		break;
	case FAIL_COUNT_2:
		if (pparam->rparam->padp->is_safe_mode_supported() != 0 &&
			need_safe_mode(pparam->rparam->pparam) != 0)
			get_safe_mode_code(pparam);
		else
			get_reboot_code(pparam);
		break;
	default:
		get_dowload_code(pparam);
		break;
	}

	return BF_OK;
}

static enum bootfail_errorcode select_method(struct recovery_run_param *pparam)
{
	struct select_param sparam;
	struct rrecord_header *phdr = (struct rrecord_header *)prrecord;

	/* gets fixed recovery method firstly */
	pparam->padp->sec_funcs.memset_s(&sparam,
		(unsigned long)sizeof(sparam), 0,
		(unsigned long)sizeof(sparam));
	sparam.rparam = pparam;
	parse_bootfail(&sparam);
	if (sparam.is_download_succ != 0)
		phdr->is_download_succ = sparam.is_download_succ;
	if (select_fixed_method(&sparam) != 0) {
		pparam->padp->print("func: select_method, "
			"select_fixed_method: %d\n", sparam.rparam->method);
		return BF_OK;
	}

	return (pparam->padp->is_bopd_supported() != 0) ?
		select_method_with_bopd(&sparam) :
		(select_method_without_bopd(&sparam));
}

static enum bootfail_errorcode read_rrecord(struct recovery_run_param *pparam)
{
	prrecord = pparam->padp->mem_ops.malloc(RECORD_SIZE_TOTAL);
	if (prrecord == NULL) {
		pparam->padp->print("func: read_rrecord, "
			"error: %d(BF_ALLOC_MEM_FAIL), malloc size: %lx\n",
			BF_ALLOC_MEM_FAIL, (unsigned long)RECORD_SIZE_TOTAL);
		return BF_ALLOC_MEM_FAIL;
	}
	pparam->padp->sec_funcs.memset_s((char *)prrecord,
		(unsigned long)RECORD_SIZE_TOTAL, 0,
		(unsigned long)RECORD_SIZE_TOTAL);

	/* read recovery record */
	if (pparam->padp->bfi_part.ops.read(pparam->padp->bfi_part.dev_path,
		pparam->rrecord_offset, (char *)prrecord,
		RECORD_SIZE_TOTAL) != 0) {
		pparam->padp->print("func: read_rrecord, "
			"error: %d(BF_READ_PART_FAIL)\n", BF_READ_PART_FAIL);
		return BF_READ_PART_FAIL;
	}

	/* verify recovery record */
	if (verify_rrecord(pparam) != BF_OK) {
		pparam->padp->print("func: read_rrecord, recovery record "
			"has been damaged, validate it here\n");
		validate_rrecord(pparam);
	}

	return BF_OK;
}

static enum bootfail_errorcode select_method_for_bootloader(
	struct recovery_run_param *pparam)
{
	enum bootfail_errorcode errorcode;

	errorcode = read_rrecord(pparam);
	if (errorcode != BF_OK) {
		pparam->padp->print("func: select_method_for_bootloader, "
			"read_rrecord failed, error: %d\n", errorcode);
		return errorcode;
	}

	errorcode = select_method(pparam);
	if (errorcode != BF_OK) {
		pparam->padp->print("func: select_method_for_bootloader, "
			"select_method failed, error: %d\n", errorcode);
		return errorcode;
	}

	errorcode = save_rrecord(pparam);
	if (errorcode != BF_OK) {
		pparam->padp->print("func: select_method_for_bootloader, "
			"save_rrecord failed, error: %d\n", errorcode);
		return errorcode;
	}

	errorcode = run_method(pparam);
	if (errorcode != BF_OK) {
		pparam->padp->print("func: select_method_for_bootloader, "
			"run_method failed, error: %d\n", errorcode);
		return errorcode;
	}

	return errorcode;
}

static enum bootfail_errorcode select_method_for_kernel(
	struct recovery_run_param *pparam)
{
	struct select_param param;
	enum bootfail_errorcode errorcode;

	errorcode = read_rrecord(pparam);
	if (errorcode != BF_OK) {
		pparam->method = RECOVERY_METHOD_UNKNOWN;
		pparam->padp->print("func: select_method_for_kernel, "
			"read_rrecord failed, error: %d\n", errorcode);
		return errorcode;
	}

	if (pparam->pparam->binfo.bootfail_errno >= HARDWARE_ERRNO_START &&
		pparam->pparam->binfo.bootfail_errno <= HARDWARE_ERRNO_END) {
		pparam->padp->sec_funcs.memset_s((char *)&param,
			(unsigned long)sizeof(param), 0,
			(unsigned long)sizeof(param));
		param.rparam = pparam;
		parse_bootfail(&param);
	} else {
		pparam->method = RECOVERY_METHOD_UNKNOWN;
	}

	return BF_OK;
}

static enum bootfail_errorcode select_and_run_recovery(
	struct recovery_run_param *pparam)
{
	return (pparam->padp->is_bootloader() != 0) ?
		select_method_for_bootloader(pparam) :
		select_method_for_kernel(pparam);
}

/**
 * do_recovery - recovery the system
 * @pparam: recovery_run_param
 * return 0 if success, -others when fail
 * this function only used by boot detector
 */
enum bootfail_errorcode do_recovery(struct recovery_run_param *pparam)
{
	enum bootfail_errorcode errorcode = BF_OK;
	struct select_param sparam;

	if (pparam == NULL)
		return BF_INVALID_PARM;

	if (pparam->rrecord_offset == 0ULL || pparam->pparam == NULL ||
		pparam->padp == NULL)
		return BF_INVALID_PARM;

	pparam->method = get_do_nothing_code(pparam->pparam->binfo.stage);

	if (pparam->padp->is_boot_recovery_enabled() == 0) {
		pparam->padp->print("func: try_to_recovery, "
			"error: %d(BF_BOOT_RECOVERY_DISABLE)\n",
			BF_BOOT_RECOVERY_DISABLED);
		return BF_BOOT_RECOVERY_DISABLED;
	}

	if (pparam->pparam->binfo.suggest_recovery_method ==
		METHOD_DO_NOTHING) {
		pparam->padp->print("func: try_to_recovery, "
			"suggest_recovery_method: METHOD_DO_NOTHING\n");
		return errorcode;
	}

	if (pparam->pparam->binfo.bootfail_errno == HARDWARE_STORAGE_FAULT) {
		pparam->padp->print("func: try_to_recovery, "
			"bootfail_errno: HARDWARE_STORAGE_FAULT\n");
		pparam->padp->sec_funcs.memset_s(&sparam,
			(unsigned long)sizeof(sparam), 0,
			(unsigned long)sizeof(sparam));
		sparam.count_total = pparam->pparam->bootfail_count;
		sparam.rparam = pparam;
		(void)select_fixed_method(&sparam);
		pparam->bopd_run_mode = get_bopd_mode(pparam->method);
		return errorcode;
	}

	errorcode = select_and_run_recovery(pparam);
	if (prrecord != NULL) {
		pparam->padp->mem_ops.free(prrecord);
		prrecord = NULL;
	}

	pparam->padp->print("func: try_to_recovery, "
		"errorcode: %d, selected method: %d\n",
		errorcode, pparam->method);

	return errorcode;
}
