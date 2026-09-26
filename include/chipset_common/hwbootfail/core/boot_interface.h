/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: define the common interface for Boot Detector and Boot Recovery
 * Author: qidechun
 * Create: 2019-03-05
 */

#ifndef BOOT_INTERFACE_H
#define BOOT_INTERFACE_H

/* ----includes ---- */
/* ----c++ support ---- */
#ifdef __cplusplus
extern "C" {
#endif

/* ---- export macroes ---- */
#ifndef BF_SIZE_32
#define BF_SIZE_32 (32)
#endif
#ifndef BF_SIZE_64
#define BF_SIZE_64 (64)
#endif
#ifndef BF_SIZE_128
#define BF_SIZE_128 (128)
#endif
#ifndef BF_SIZE_256
#define BF_SIZE_256 (256)
#endif
#ifndef BF_SIZE_512
#define BF_SIZE_512 (512)
#endif
#ifndef BF_SIZE_1K
#define BF_SIZE_1K (1024)
#endif
#ifndef BF_SIZE_4K
#define BF_SIZE_4K (4096)
#endif
#ifndef BF_SIZE_128K
#define BF_SIZE_128K (BF_SIZE_128 * BF_SIZE_1K)
#endif
#ifndef BF_SIZE_512K
#define BF_SIZE_512K (BF_SIZE_512 * BF_SIZE_1K)
#endif
#ifndef BF_SIZE_1M
#define BF_SIZE_1M (BF_SIZE_1K * BF_SIZE_1K)
#endif

/* boot fail log info */
#ifndef array_size
#define array_size(a) (sizeof(a) / sizeof((a)[0]))
#endif
#define is_bl1_stage(stage) ((stage) == BL1_STAGE)
#define is_bl2_stage(stage) ((stage) == BL2_STAGE)
#define is_kernel_stage(stage) ((stage) == KERNEL_STAGE)
#define is_native_stage(stage) ((stage) == NATIVE_STAGE)
#define is_framework_stage(stage) ((stage) == FRAMEWORK_STAGE)
#define is_after_suss_stage(stage) ((stage) == BOOT_SUCC_STAGE)
#define is_bottom_stage(stage) (is_bl1_stage(stage) || \
	(is_bl2_stage(stage)))
#define is_upper_stage(stage) (is_kernel_stage(stage) || \
	is_native_stage(stage) || \
	is_framework_stage(stage) || \
	is_after_suss_stage(stage))
#define is_valid_stage(stage) (is_bottom_stage(stage) || \
	is_upper_stage(stage))
#define is_vaild_bootfail_errno(bootfail_errno) ( \
	(((bootfail_errno) >= BL1_ERRNO_START) && \
	((bootfail_errno) <= BL1_ERRNO_END)) || \
	(((bootfail_errno) >= BL2_ERRNO_START) && \
	((bootfail_errno) <= BL2_ERRNO_END)) || \
	(((bootfail_errno) >= KERNEL_ERRNO_START) && \
	((bootfail_errno) <= KERNEL_ERRNO_END)) || \
	(((bootfail_errno) >= NATIVE_ERRNO_START) && \
	((bootfail_errno) <= NATIVE_ERRNO_END)) || \
	(((bootfail_errno) >= FRAMEWORK_ERRNO_START) && \
	((bootfail_errno) <= FRAMEWORK_ERRNO_END)) || \
	(((bootfail_errno) >= HARDWARE_ERRNO_START) && \
	((bootfail_errno) <= HARDWARE_ERRNO_END)))

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef __packed
#define __packed __attribute((packed))
#endif

#define BFI_PART_HDR_SIZE (sizeof(union bfi_part_header))
#define BF_INFO_OFFSET (BFI_PART_HDR_SIZE)
#define BF_INFO_SIZE (sizeof(struct bootfail_proc_param))
#define BF_META_LOG_OFFSET (BFI_PART_HDR_SIZE + BF_INFO_SIZE)
#define BF_META_LOG_SIZE (sizeof(struct bootfail_meta_log))
#define BF_LOG_OFFSET (BFI_PART_HDR_SIZE + BF_INFO_SIZE + BF_META_LOG_SIZE)

/* ---- export prototypes ---- */
enum raw_stage {
	STAGE_PBL = 0, /* not used */
	STAGE_BL1, /* BL1- bootloader stage 1, xloader for hisi, sbl1 for qcom */
	STAGE_BL2, /* BL2 - bootloader stage 2, fastboot for hisi, lk for qcom */
	STAGE_KERNEL,
	STAGE_NATIVE,
	STAGE_FRAMEWORK,
	STAGE_AFTER_SUCC,
};

enum bootfail_errno {
	/*
	 * ===========================================
	 * BL1 -- bootloader stage 1, it's xloader for hisi, sbl1 for qualcomm
	 * ===========================================
	 */
	BL1_ERRNO_START = (((unsigned int)STAGE_BL1) << 28),
	BL1_CORE_DEV_FAULT,
	BL1_NON_CORE_DEV_FAULT,
	BL1_IMG_FAULT,
	BL1_SYSTEM_CRASH,
	BL1_SYSTEM_FREEZE,
	BL1_BAD_ENV,
	BL1_ERRNO_END = (((unsigned int)STAGE_BL2) << 28) - 1,

	/*
	 * ===========================================
	 * BL2 -- bootloader stage 2, it's fastboot for hisi, lk for qualcomm
	 * ===========================================
	 */
	BL2_ERRNO_START = (((unsigned int)STAGE_BL2) << 28),
	BL2_CORE_DEV_FAULT,
	BL2_NON_CORE_DEV_FAULT,
	BL2_IMG_FAULT,
	BL2_SYSTEM_CRASH,
	BL2_SYSTEM_FREEZE,
	BL2_BAD_ENV,
	BL2_ERRNO_END = (((unsigned int)STAGE_KERNEL) << 28) - 1,

	/*
	 * ===========================================
	 * kernel stage's boot fail errors
	 * ===========================================
	 */
	KERNEL_ERRNO_START = (((unsigned int)STAGE_KERNEL) << 28),
	KERNEL_CORE_DEV_FAULT,
	KERNEL_NON_CORE_DEV_FAULT,
	KERNEL_IMG_FAULT,
	KERNEL_SUBSYS_FAULT,
	KERNEL_SYSTEM_CRASH,
	KERNEL_SYSTEM_FREEZE,
	KERNEL_BAD_ENV,
	KERNEL_ERRNO_END = (((unsigned int)STAGE_NATIVE) << 28) - 1,

	/*
	 * ===========================================
	 * natvie stage's boot fail errors
	 * ===========================================
	 */
	NATIVE_ERRNO_START = (((unsigned int)STAGE_NATIVE) << 28),
	NATIVE_CORE_DEV_FAULT,
	NATIVE_NON_CORE_DEV_FAULT,
	NATIVE_SERVICE_CRASH,
	NATIVE_SERVICE_FREEZE,
	NATIVE_DATA_PART_MOUNT_FAIL,
	NATIVE_DATA_PART_MOUNT_RO,
	NATIVE_DATA_DAMAGED,
	NATIVE_PART_MOUNT_FAIL,
	NATIVE_SUBSYS_FAULT,
	NATIVE_SYSTEM_CRASH,
	NATIVE_SYSTEM_FREEZE,
	NATIVE_BAD_ENV,
	NATIVE_ERRNO_END = (((unsigned int)STAGE_FRAMEWORK) << 28) - 1,

	/*
	 * ===========================================
	 * framework stage's boot fail errors
	 * ===========================================
	 */
	FRAMEWORK_ERRNO_START = (((unsigned int)STAGE_FRAMEWORK) << 28),
	FRAMEWORK_CORE_DEV_FAULT,
	FRAMEWORK_NON_CORE_DEV_FAULT,
	FRAMEWORK_SERVICE_CRASH,
	FRAMEWORK_SERVICE_FREEZE,
	FRAMEWORK_DATA_DAMAGED,
	FRAMEWORK_SUBSYS_FAULT,
	FRAMEWORK_SYSTEM_CRASH,
	FRAMEWORK_SYSTEM_FREEZE,
	FRAMEWORK_BAD_ENV,
	FRAMEWORK_ERRNO_END = (((unsigned int)STAGE_AFTER_SUCC) << 28) - 1,

	/*
	 * ===========================================
	 * bootfail errno definition after boot success
	 * ===========================================
	 */
	AFTER_BOOT_SUCC_ERRNO_START = (((unsigned int)STAGE_AFTER_SUCC) << 28),
	AFTER_BOOT_SUCC_ERRNO_END = 0x7fff0000 - 1,

	/*
	 * ===========================================
	 * bootfail errno for hardware fault
	 * ===========================================
	 */
	HARDWARE_ERRNO_START = 0x7fff0000,
	HARDWARE_CORE_DEV_FAULT_START,
	HARDWARE_CORE_DEV_FAULT = HARDWARE_CORE_DEV_FAULT_START,
	HARDWARE_STORAGE_FAULT,
	HARDWARE_CORE_DEV_FAULT_END = 0x7fff0fff,
	HARDWARE_NON_CORE_DEV_FAULT_START = 0x7fff1000,
	HARDWARE_NON_CORE_DEV_FAULT = HARDWARE_NON_CORE_DEV_FAULT_START,
	HARDWARE_NON_CORE_DEV_FAULT_END = 0x7ffffffa - 1,
	HARDWARE_ERRNO_END = 0x7ffffffa,

	ERRNO_MAX_COUNT = 0x7fffffff,
};

enum boot_stage {
	INVALID_STAGE = 0,

	/*
	 * ===========================================
	 * BL1 -- bootloader stage 1, it's xloader for hisi, sbl1 for qualcomm
	 * ===========================================
	 */
	BL1_STAGE_START = (((unsigned int)STAGE_BL1) << 28),
	BL1_STAGE = (((unsigned int)STAGE_BL2) << 28) - 2,
	BL1_STAGE_END = (((unsigned int)STAGE_BL2) << 28) - 1,

	/*
	 * ===========================================
	 * BL2 -- bootloader stage 2, it's fastboot for hisi, lk for qualcomm
	 * ===========================================
	 */
	BL2_STAGE_START = (((unsigned int)STAGE_BL2) << 28),
	BL2_STAGE = (((unsigned int)STAGE_KERNEL) << 28) - 2,
	BL2_STAGE_END = (((unsigned int)STAGE_KERNEL) << 28) - 1,

	/*
	 * ===========================================
	 * kernel stage
	 * ===========================================
	 */
	KERNEL_STAGE_START = (((unsigned int)STAGE_KERNEL) << 28),
	KERNEL_STAGE = (((unsigned int)STAGE_NATIVE) << 28) - 2,
	KERNEL_STAGE_END = (((unsigned int)STAGE_NATIVE) << 28) - 1,

	/*
	 * ===========================================
	 * native stage
	 * ===========================================
	 */
	NATIVE_STAGE_START = (((unsigned int)STAGE_NATIVE) << 28),
	NATIVE_STAGE = (((unsigned int)STAGE_FRAMEWORK) << 28) - 2,
	NATIVE_STAGE_END = (((unsigned int)STAGE_FRAMEWORK) << 28) - 1,

	/*
	 * ===========================================
	 * framework stage
	 * ===========================================
	 */
	FRAMEWORK_STAGE_START = (((unsigned int)STAGE_FRAMEWORK) << 28),
	FRAMEWORK_STAGE = (((unsigned int)STAGE_AFTER_SUCC) << 28) - 2,
	FRAMEWORK_STAGE_END = (((unsigned int)STAGE_AFTER_SUCC) << 28) - 1,

	/*
	 * ===========================================
	 * boot success start stage
	 * ===========================================
	 */
	AFTER_SUCC_STAGE_START = (((unsigned int)STAGE_AFTER_SUCC) << 28),
	AFTER_SUCC_STAGE = 0x7FFFFFFF - 2,
	AFTER_SUCC_STAGE_END = 0x7FFFFFFF - 1,

	/*
	 * ===========================================
	 * boot success stage code
	 * ===========================================
	 */
	BOOT_SUCC_STAGE = 0x7FFFFFFF,
};

enum bootfail_errorcode {
	BF_OK = 0,
	BF_HAS_INITED,
	BF_NOT_INIT_SUCC,
	BF_INVALID_PARM,
	BF_INVALID_FUNC_PTR,
	BF_INVALID_DEV_PATH_OR_SIZE,
	BF_INVALID_MEM_ADDR_OR_SIZE,
	BF_INVALID_BOOTFAIL_INFO,
	BF_INVALID_LOG_NAME,
	BF_BOOT_DETECTOR_DISABLED,
	BF_BOOT_RECOVERY_DISABLED,
	BF_ALLOC_MEM_FAIL,
	BF_READ_PART_FAIL,
	BF_WRITE_PART_FAIL,
	BF_RRECORD_DAMAGED,
	BF_INVALID_RECOVERY_METHOD,
	BF_PLATFORM_ERR,
	BF_ERRCODE_MAX_COUNT
};

/*
 * Total size of this struct: 64B.
 * You're strongly recommended to dynamically allocate memory for the struct.
 */
struct bootfail_basic_info {
	unsigned long long rtc_time;
	int bootup_time;
	int bootfail_errno;
	int exception_type;
	int sub_exception_type;
	int stage; /* stage saved by bootdetector */
	int exception_stage; /* stage provided by platform */
	int suggest_recovery_method;
	int recovery_method;
	int recovery_result;
	int dmd_errno;
	int post_action;
	char is_rooted;
	char is_perceptible;
	char is_bootup_succ;
	char is_updated;
	char reserved[8]; /* reserved count */
} __packed;

/*
 * Total size of this struct: 1KB.
 * You're strongly recommended to dynamically allocate memory for the struct.
 */
struct bootfail_proc_param {
	unsigned int magic;
	struct bootfail_basic_info binfo;
	char detail_info[384]; /* max exception info count */
	char log_path[3][128]; /* max count and size of upper layer logs */
	char part_name[32]; /* max part name count */
	char hardware_detail[16]; /* max detail count */
	unsigned int space_left; /* sapce left on data partition, Unit: MB */
	unsigned int inodes_left; /* inodes left on data partition */
	int bootfail_count; /* for bootfail inject and storage fault */
	char reserved[128]; /* reserved count */
} __packed;

/*
 * Total size of this struct: 1KB.
 * You're strongly recommended to dynamically allocate memory for the struct.
 */
struct bfi_misc_msg {
	/* "boot-erecovery" */
	char command[32]; /* max command count */

	/* BI code for eRecovery */
	int bi_code;

	/* boot fail no */
	int bootfail_errno;

	/* boot stage when boot fail occurs */
	int stage;

	/* abnormal shutdown flag */
	int abns_flag;

	/* format data flag */
	unsigned int fmt_data_flag;

	/* original recovery method */
	unsigned int orig_method;

	/* bopd mode from erecovery subsystem */
	unsigned long long bopd_run_mode;

	/* reserved for future */
	char reserved[960]; /* reserved count */
} __packed;

union bfi_part_header {
	struct bfi_misc_msg misc_msg;
} __packed;

struct bootfail_log_info {
	struct {
		char name[16]; /* max file name length */
		unsigned int start;
		unsigned int size;
	} __packed bl_log;
	struct {
		char name[16]; /* max file name length */
		unsigned int start;
		unsigned int size;
	} __packed kernel_log;
} __packed;

/*
 * Total size of this struct: 1KB.
 * You're strongly recommended to dynamically allocate memory for the struct.
 */
struct bootfail_meta_log {
	/* check validity of the data following this field in this struct */
	unsigned char sha256[BF_SIZE_32];
	unsigned int write_idx;
	unsigned int log_count;
	unsigned int log_max_count;
	unsigned int rrecord_offset;
	struct bootfail_log_info logs_info[10]; /* max log count */
	char reserved[496]; /* reserved count */
} __packed;

/*
 * Total size of this struct: 1KB.
 * You're strongly recommended to dynamically allocate memory for the struct.
 */
struct bootfail_log_header {
	struct bootfail_basic_info binfo;
	char detail_info[384]; /* max exception info count */
	char log_path[3][128]; /* max count and size of upper layer logs */
	char part_name[32]; /* max part name count */
	char hardware_detail[16]; /* max detail count */
	unsigned int space_left; /* sapce left on data partition, Unit: MB */
	unsigned int inodes_left; /* inodes left on data partition */
	char reserved[136]; /* reserved count */
} __packed;

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
