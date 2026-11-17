/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: define the common interface for Boot Recovery
 * Author: qidechun
 * Create: 2019-03-05
 */

#ifndef BOOT_RECOVERY_H
#define BOOT_RECOVERY_H

/* ----includes ---- */
#include "adapter.h"
#include "boot_interface.h"

/* ----c++ support ---- */
#ifdef __cplusplus
extern "C" {
#endif

/* ---- export macroes ---- */
#define BOOT_ERECOVERY "boot-erecovery"
#define BOOT_SAFEMODE "boot-safemode"
#define BOOT_BOPD "boot-bopd"
#define NORMAL_LONG_PRESS_TAG "normal_valid_longpress"
#define ERECOVERY_LONG_PRESS_TAG "erecovery_valid_longpress"
#define START_MOUNT_DATA (0x73746d64) /* stmd */
#define FSCK_BEFORE_FORMAT_DATA (0x6673666d) /* fsfm */
#define FORMAT_DATA_WITHOUT_FSCK (0x666d6474) /* fmdt */
#define PANIC_IN_ERECOVERY 0x45524550 /* EREP */
#define PANIC_IN_RECOVERY 0x52454350 /* RECP */

/* BI code for erecovery download */
#define BI_PRESS_KEY (2001)
#define BI_ADB_CMD (2002)
#define BI_UNLOCK (2004)
#define BI_SYSTEM_DAMAGED (2005)
#define BI_VERIFY_FAILED (2006)
#define BI_BL_BOOT_FAIL (2008)
#define BI_KERNEL_BOOT_FAIL (2009)
#define BI_SYSTEM_MNT_FAIL (2010)
#define BI_DATA_MNT_FAIL (2011)
#define BI_DATA_MNT_RO (2012)
#define BI_KEY_SVC_START_FAIL (2013)
#define BI_RECOVERY_PROC_CRASH (2014)
#define BI_AP_CRASH (2015)
#define BI_NON_AP_CRASH (2016)
#define BI_NATIVE_BOOT_FAIL (2017)
#define BI_SECURITY_FAIL (2018)
#define BI_VENDOR_MNT_FAIL (2019)
#define BI_CUST_MNT_FAIL (2020)
#define BI_PRODUCT_MNT_FAIL (2021)
#define BI_VERSION_MNT_FAIL (2022)
#define BI_DEG_BOOT_FAIL (2040)
#define BI_UNKNOWN (2099)
#define RECORD_COUNT (40)

/* ---- export prototypes ---- */
enum suggest_recovery_method {
	/*
	 * if you can not determine the recovery method for a boot fail,
	 * please transfer NO_SUGG to the BFR.
	 */
	METHOD_NO_SUGGESTION = 0x0,
	METHOD_DO_NOTHING
};

enum update_state {
	UPDATE_NON = 0,
	UPDATE_FAIL,
	UPDATE_SUCC_0,
	UPDATE_SUCC_1,
	UPDATE_SUCC_2,
};

/*
 * format:
 * -------------
 * |4|xx|yyy|zzz|
 * -------------
 * 4 - reserved keyword.
 * xx - two decimal digits presenting the type of service.
 * yyy - three decimal digits presenting the type of fault.
 * zzz - three decimal digits presenting the recovey method.
 */
enum recovery_method {
	BL1_DO_NOTHING = 400005000,
	BL1_REBOOT = 400005001,
	BL1_FACTORY_REST = 400005002,
	BL1_SAFE_MODE = 400005003,
	BL1_BOPD = 400005004,
	BL1_DEGRADE_BOPD = 400005005,
	BL1_UFS_RDONLY_BOPD = 400005006,
	BL1_UFS_RDONLY_HW_DEG_BOPD = 400005007,
	BL1_DOWNLOAD = 400005008,
	BL1_PREVENTION = 400005009,
	BL2_DO_NOTHING = 400006000,
	BL2_REBOOT = 400006001,
	BL2_FACTORY_REST = 400006002,
	BL2_SAFE_MODE = 400006003,
	BL2_BOPD = 400006004,
	BL2_DEGRADE_BOPD = 400006005,
	BL2_UFS_RDONLY_BOPD = 400006006,
	BL2_UFS_RDONLY_DEGRADE_BOPD = 400006007,
	BL2_DOWNLOAD = 400006008,
	BL2_PREVENTION = 400006009,
	KERNEL_DO_NOTHING = 400007000,
	KERNEL_REBOOT = 400007001,
	KERNEL_FACTORY_REST = 400007002,
	KERNEL_SAFE_MODE = 400007003,
	KERNEL_BOPD = 400007004,
	KERNEL_DEGRADE_BOPD = 400007005,
	KERNEL_UFS_RDONLY_BOPD = 400007006,
	KERNEL_UFS_RDONLY_DEGRADE_BOPD = 400007007,
	KERNEL_BOPD_AFTER_DOWNLOAD = 400007008,
	KERNEL_DEGRADE_BOPD_AFTER_DOWNLOAD = 400007009,
	KERNEL_DOWNLOAD = 400007010,
	KERNEL_PREVENTION = 400007011,
	NATIVE_DO_NOTHING = 400008000,
	NATIVE_REBOOT = 400008001,
	NATIVE_FACTORY_REST = 400008002,
	NATIVE_SAFE_MODE = 400008003,
	NATIVE_BOPD = 400008004,
	NATIVE_DEGRADE_BOPD = 400008005,
	NATIVE_UFS_RDONLY_BOPD = 400008006,
	NATIVE_UFS_RDONLY_DEGRADE_BOPD = 400008007,
	NATIVE_BOPD_AFTER_DOWNLOAD = 400008008,
	NATIVE_DEGRADE_BOPD_AFTER_DOWNLOAD = 400008009,
	NATIVE_DOWNLOAD = 400008010,
	NATIVE_PREVENTION = 400008011,
	FRAMEWORK_DO_NOTHING = 400009000,
	FRAMEWORK_REBOOT = 400009001,
	FRAMEWORK_FACTORY_REST = 400009002,
	FRAMEWORK_SAFE_MODE = 400009003,
	FRAMEWORK_BOPD = 400009004,
	FRAMEWORK_DEGRADE_BOPD = 400009005,
	FRAMEWORK_UFS_RDONLY_BOPD = 400009006,
	FRAMEWORK_UFS_RDONLY_DEGRADE_BOPD = 400009007,
	FRAMEWORK_BOPD_AFTER_DOWNLOAD = 400009008,
	FRAMEWORK_DEGRADE_BOPD_AFTER_DOWNLOAD = 400009009,
	FRAMEWORK_DOWNLOAD = 400009010,
	FRAMEWORK_PREVENTION = 400009011,
	RECOVERY_METHOD_UNKNOWN = 400999999,
};

enum recovery_status {
	/* run status code for boot prevention */
	PREVENTION_START = 0,
	DEGRADE_BEGIN,
	DEGRADE_SUCC,
	DEGRADE_FAIL,
	BYPASS_BEGIN,
	BYPASS_SUCC,
	BYPASS_FAIL,
	LOAD_BACKUP_IMAGE_BEGIN,
	LOAD_BACKUP_IMAGE_SUCC,
	LOAD_BACKUP_IMAGE_FAIL,
	PREVENTION_END = 1024,

	/* run status code for boot recovery */
	RECOVERY_START = 1025,
	BOOT_ERECOVERY_SUCC,
	BOOT_ERECOVERY_FAIL,
	MOUNT_DATA_BEGIN,
	MOUNT_DATA_SUCC,
	MOUNT_DATA_FAIL,
	FACTORY_REST_BEGIN,
	FACTORY_REST_SUCC,
	FACTORY_REST_FAIL,
	START_WIFI_BEGIN,
	START_WIFI_SUCC,
	START_WIFI_FAIL,
	QUERY_PACKAGE_BEGIN,
	QUERY_PACKAGE_SUCC,
	QUERY_PACKAGE_FAIL,
	DOWNLOAD_PACKAGE_BEGIN,
	DOWNLOAD_PACKAGE_SUCC,
	DOWNLOAD_PACKAGE_FAIL,
	DOWNLOAD_AUTH_FILE_BEGIN,
	DOWNLOAD_AUTH_FILE_SUCC,
	DOWNLOAD_AUTH_FILE_FAIL,
	VERIFY_PACKAGE_BEGIN,
	VERIFY_PACKAGE_SUCC,
	VERIFY_PACKAGE_FAIL,
	INSTALL_PACKAGE_BEGIN,
	INSTALL_PACKAGE_SUCC,
	INSTALL_PACKAGE_FAIL,
	RECOVERY_END = 2048,

	/* others */
};

struct recovery_run_param {
	unsigned long long rrecord_offset;
	struct bootfail_proc_param *pparam;
	struct adapter *padp;
	int method;
	int deg_fail;
	unsigned long long bopd_run_mode;
	void (*run_prevention)(struct recovery_run_param *pparam);
};

enum recovery_result {
	RECOVERY_SUCCESS = (0xA5A5A5A5),
	RECOVERY_FAIL = (0x5A5A5A5A),
};

/* Total size of this struct: 128B. */
struct rrecord_header {
	/* sha256 for data following this field */
	unsigned char sha256[BF_SIZE_32];

	/* boot fail total count for this device */
	int bootfail_count;
	int write_idx;

	/* max size of the record count */
	int record_count;

	/* this field shouid be set as 0 when boot success */
	int count_before_succ;

	/* if download successfully */
	int is_download_succ;

	char reserved[76]; /* reserved count */
} __packed;

/* Total size of this struct: 64B. */
struct rrecord {
	unsigned long long rtc_time;
	int bootfail_errno;
	int stage;
	int orig_method;
	int real_method;
	int result;
	int status;
	char hardware_detail[16]; /* max detail count */
	char reserved[16]; /* reserved count */
} __packed;

/*
 * Total size of this struct: 5376B.
 * You're strongly recommended to dynamically allocate memory for the struct.
 */
struct recovery_info {
	struct rrecord_header hdr;
	struct rrecord rrecord[RECORD_COUNT]; /* 2560B */
	struct rrecord_header backup_hdr;
	struct rrecord backup_rrecord[RECORD_COUNT]; /* 2560B */
} __packed;

/* ---- export macro functions ---- */
#define is_do_nothing_fault(method) \
	(((method) == FRAMEWORK_DO_NOTHING) || \
	((method) == NATIVE_DO_NOTHING) || \
	((method) == KERNEL_DO_NOTHING) || \
	((method) == BL2_DO_NOTHING) || \
	((method) == BL1_DO_NOTHING))

/*
 * DL - DownLoad
 * HW - HardWare
 * RO - ReadOnly
 * DEG - Degrade
 * STG - Storage
 */
#define BOPD_BEFORE_DL 0x2
#define BOPD_STG_RO_BEFORE_DL 0x3
#define BOPD_HW_DEG_BEFORE_DL 0x6
#define BOPD_STG_RO_HW_DEG_BEFORE_DL 0x7
#define BOPD_AFTER_DL 0xa
#define BOPD_HW_DEG_AFTER_DL 0xe
#define BOPD_HW_DEG_VIA_ERECOVERY 0xf
#define is_bopd_run_mode(mode) \
	(((mode) == BOPD_BEFORE_DL) || \
	((mode) == BOPD_STG_RO_BEFORE_DL) || \
	((mode) == BOPD_HW_DEG_BEFORE_DL) || \
	((mode) == BOPD_STG_RO_HW_DEG_BEFORE_DL) || \
	((mode) == BOPD_AFTER_DL) || \
	((mode) == BOPD_HW_DEG_AFTER_DL) || \
	((mode) == BOPD_HW_DEG_VIA_ERECOVERY))

/* ---- export function prototypes ---- */
enum bootfail_errorcode do_recovery(struct recovery_run_param *pparam);

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
