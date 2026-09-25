/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: the hw_kernel_stp_proc.c for proc file create and destroy
 * Author: sunhongqing <sunhongqing@huawei.com>
 * Create: 2018-3-31
 */

#ifndef _HW_KERNEL_STP_INTERFACE_H_
#define _HW_KERNEL_STP_INTERFACE_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>

#define STP_ITEM_NAME_LEN 16

#define STP_NAME_KCODE         "kcode"
#define STP_NAME_KCODE_SYSCALL "kcode-syscall"
#define STP_NAME_SE_ENFROCING  "se-enforcing"
#define STP_NAME_SE_HOOK       "se-hook"
#define STP_NAME_ROOT_PROCS    "root-procs"
#define STP_NAME_SETIDS        "setids"
#define STP_NAME_KEY_FILES     "key-files"
#define STP_NAME_EIMA          "eima"
#define STP_NAME_RODATA        "rodata"
#define STP_NAME_USERCOPY      "usercopy"
#define STP_NAME_CFI           "cfi"
#define STP_NAME_MOD_SIGN      "mod-sign"
#define STP_NAME_PTRACE        "ptrace"
#define STP_NAME_HKIP          "hkip"
#define STP_NAME_ITRUSTEE      "itrustee"
#define STP_NAME_DOUBLE_FREE   "double-free"
#define STP_NAME_KSHIELD       "kshield"

#define STP_ID_KCODE           0x00000282
#define STP_ID_KCODE_SYSCALL   0x00000283
#define STP_ID_SE_ENFROCING    0x00000383
#define STP_ID_SE_HOOK         0x00000284
#define STP_ID_ROOT_PROCS      0x00000385
#define STP_ID_SETIDS          0x00000380
#define STP_ID_KEY_FILES       0x00000202
#define STP_ID_EIMA            0x00000280
#define STP_ID_RODATA          0x00000281
#define STP_ID_USERCOPY        0x00000180
#define STP_ID_CFI             0x00000181
#define STP_ID_MOD_SIGN        0x00000182
#define STP_ID_PTRACE          0x00000183
#define STP_ID_HKIP            0x00000184
#define STP_ID_ITRUSTEE        0x00000185
#define STP_ID_DOUBLE_FREE     0x00000186
#define STP_ID_KSHIELD         0x00000188

typedef int (*stp_cb)(void);

typedef union {
	u64 val;
	struct {
		u32 para;
		u32 feat;
	} s;
} stp_proc_type;

enum stp_proc_feature {
	KERNEL_STP_SCAN = 0,
	HARDEN_DBLFREE_CHECK = 1,
	KERNEL_STP_KSHIELD = 2,
	HHEE_SWITCH = 3,
};

enum stp_proc_hhee_bit {
	ENABLE_HHEE_TVM = (1 << 0),
};

enum stp_status {
	STP_SAFE = 0, /* safe */
	STP_RISK = 1, /* risk */
};

enum stp_reliability {
	STP_REFERENCE = 0, /* status not trustable */
	STP_CREDIBLE  = 1, /* status trustable */
};

enum stp_item_category {
	KCODE = 0,
	SYSCALL,
	SE_ENFROCING,
	SE_HOOK,
	ROOT_PROCS,
	SETIDS,
	KEY_FILES,
	EIMA,
	USERCOPY,
	CFI,
	MOD_SIGN,
	PTRACE,
	HKIP,
	ITRUSTEE,
	DOUBLE_FREE,
	KSHIELD,
	STP_ITEM_MAX,
};

struct stp_item_info {
	unsigned int    id;
	char            name[STP_ITEM_NAME_LEN];
};

struct stp_item {
	/* item id formated as 0x0000ccnn:
	 * cc means item category, defined by STP_CATEGORY
	 * ii means item number, which is uniqe
	 * item id used can be seen at:
	 * (wiki address to be updated :)
	 */
	unsigned int    id;
	/* item status defined by STP_STATUS */
	unsigned char   status;
	/* whether status is trusable, defined by STP_RELIABILITY */
	unsigned char   credible;
	/* item version number */
	unsigned char   version;
	/* item name or description string */
	char            name[STP_ITEM_NAME_LEN];
};

/*
 * kernel_stp_scanner_register - each scanner module can regist to the kernel
 * stp by this func
 * Description: accept each scanner module regist to the kernel stp
 * @callbackfunc,the scanner trigger func of each module
 * @return: Result of loading.
 *     0, regist correctly.
 *     -1, regist failed.
 */
#ifdef CONFIG_HW_KERNEL_STP
int kernel_stp_scanner_register(stp_cb callbackfunc);
/* the caller needed to check the return value */
extern const struct stp_item_info *get_item_info_by_idx(int idx);

extern const struct stp_item_info item_info[];
#else
static inline int kernel_stp_scanner_register(stp_cb callbackfunc)
{
	return 0;
}
/* the caller needed to check the return value */
static inline const struct stp_item_info *get_item_info_by_idx(int idx)
{
	return NULL;
}

static const struct stp_item_info item_info[] = {
	[KCODE]        = { STP_ID_KCODE, STP_NAME_KCODE },
	[SYSCALL]      = { STP_ID_KCODE_SYSCALL, STP_NAME_KCODE_SYSCALL },
	[SE_ENFROCING] = { STP_ID_SE_ENFROCING, STP_NAME_SE_ENFROCING },
	[SE_HOOK]      = { STP_ID_SE_HOOK, STP_NAME_SE_HOOK },
	[ROOT_PROCS]   = { STP_ID_ROOT_PROCS, STP_NAME_ROOT_PROCS },
	[SETIDS]       = { STP_ID_SETIDS, STP_NAME_SETIDS },
	[KEY_FILES]    = { STP_ID_KEY_FILES, STP_NAME_KEY_FILES },
	[EIMA]         = { STP_ID_EIMA, STP_NAME_EIMA },
	[USERCOPY]     = { STP_ID_USERCOPY, STP_NAME_USERCOPY },
	[CFI]          = { STP_ID_CFI, STP_NAME_CFI },
	[MOD_SIGN]     = { STP_ID_MOD_SIGN, STP_NAME_MOD_SIGN },
	[PTRACE]       = { STP_ID_PTRACE, STP_NAME_PTRACE },
	[HKIP]         = { STP_ID_HKIP, STP_NAME_HKIP },
	[ITRUSTEE]     = { STP_ID_ITRUSTEE, STP_NAME_ITRUSTEE },
	[DOUBLE_FREE]  = { STP_ID_DOUBLE_FREE, STP_NAME_DOUBLE_FREE },
	[KSHIELD]      = { STP_ID_KSHIELD, STP_NAME_KSHIELD },
};
#endif

/*
 * kernel_stp_upload - for each scanner module to  upload result to stp
 * Description: upload the scanner result to stp
 * @result, scanner result as stp_item
 * @addition_info,the addition info wanted to upload
 * @return: Result of loading.
 *     0, upload correctly.
 *     -1, upload failed.
 */
#ifdef CONFIG_HW_KERNEL_STP
int kernel_stp_upload(struct stp_item result, const char *addition_info);

#else
static inline int kernel_stp_upload(struct stp_item result, const char *addition_info)
{
	return 0;
}
#endif

/*
 * kernel_stp_kshield_upload - kshield info upload result to stp
 * Description: upload kshield info to stp
 * @result, kshield of stp_item
 * @addition_info,the addition info wanted to upload
 * @return: Result of loading.
 *     0, upload correctly.
 *     -1, upload failed.
 */

#ifdef CONFIG_HW_KERNEL_STP
int kernel_stp_kshield_upload(struct stp_item result, const char *addition_info);

#else
static inline int kernel_stp_kshield_upload(struct stp_item result, const char *addition_info)
{
	return 0;
}
#endif

#endif /* _HW_KERNEL_STP_INTERFACE_H_*/
