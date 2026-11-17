/*
 * storage_rochk.c
 *
 * emmc/ufs Storage Read Only Check
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/reboot.h>
#include <linux/version.h>

#include <scsi/scsi.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>

#include <linux/mmc/mmc.h>
#include <linux/mmc/card.h>
#include <linux/mmc/core.h>
#include <linux/mmc/host.h>

#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <chipset_common/storage_rofa/storage_rofa.h>
#include "storage_rochk.h"
#include "crbroi.h"

#define STORAGE_ROCHK_NAME "storage_rochk"

#define PRINT_ERR(f, arg...) \
	pr_err(STORAGE_ROCHK_NAME ": " f, ## arg)

#define PRINT_INFO(f, arg...) \
	pr_info(STORAGE_ROCHK_NAME ": " f, ## arg)

#define PRINT_DEBUG(f, arg...) \
	pr_devel(STORAGE_ROCHK_NAME ": " f, ## arg)

#define PRINT_INFO_ONCE(format...)	({	\
	static bool __warned;			\
						\
	if (!__warned) {			\
		PRINT_INFO(format);		\
		__warned = true;		\
	}					\
})

#ifndef UFSHCD
#define UFSHCD "ufshcd"
#endif

#define STR_MANUFACTURER_TOSHIBA     "tx"
#define STR_MANUFACTURER_HYNIX       "hy"
#define STR_MANUFACTURER_SAMSUNG     "ss"
#define STR_MANUFACTURER_MICRON      "mt"
#define STR_MANUFACTURER_SANDISK     "sd"
#define STR_MANUFACTURER_HISI        "hi"
#define STR_MANUFACTURER_UNKNOWN     "xx"

#define EMMC_CID_MANFID_SANDISK      0x02
#define EMMC_CID_MANFID_SANDISK_V2   0x45
#define EMMC_CID_MANFID_TOSHIBA      0x11
#define EMMC_CID_MANFID_MICRON       0x13
#define EMMC_CID_MANFID_SAMSUNG      0x15
#define EMMC_CID_MANFID_KINGSTON     0x70
#define EMMC_CID_MANFID_HYNIX        0x90

#define EMMC_CSD_WP_BYTE         14
#define EMMC_CSD_PWRON_WP_MASK   0x10
#define EMMC_CSD_CRC_BYTE        15

#define MMC_RSP_R1_RO_ERROR_MASK (R1_WP_VIOLATION | R1_ILLEGAL_COMMAND)

#define BOOTDEVICE_DISK_NAME DISK_NAME_MAX_SIZE

#define BOOTDEVICE_MODEL_SIZE   32
#define BOOTDEVICE_FWREV_SIZE   32

#define PARTITION_WRTRY_PATH    "/dev/block/by-name/rrecord"
#define PARTITION_WRTRY_LEN     512

#define WRITE_TRY_ERRCODE_OK        0
#define WRITE_TRY_ERRCODE_BLOCK     1
#define WRITE_TRY_ERRCODE_STUCK     2
#define WRITE_TRY_ERRCODE_OTHERS    3

#define PRE_EOL_NORMAL    1
#define PRE_EOL_SERIOUS   2
#define PRE_EOL_CRITICAL  3

#define MONITOR_WRITE_REQ_TIMES     50

struct bootdevice_info {
	int type;   /* 0 for emmc and 1 for ufs */
	unsigned int manfid;
	char model[BOOTDEVICE_MODEL_SIZE];
	char fwrev[BOOTDEVICE_FWREV_SIZE];
	unsigned int pre_eol_info;
};

struct bootdevice_disk_info {
	struct list_head disk_list;
	struct disk_info disk_info;
};

struct storage_rochk_completion {
	struct completion event;
	unsigned int result;
};

static LIST_HEAD(g_bootdev_disks);
static DEFINE_MUTEX(g_bootdev_disks_mutex);
static struct bootdevice_info g_bootdev_info = {
	.type = -1,
	.model = {0},
	.fwrev = {0},
};

static atomic_t g_monitor_enabled = ATOMIC_INIT(0);
static atomic_t g_monitor_times = ATOMIC_INIT(MONITOR_WRITE_REQ_TIMES);
static atomic_t g_action_once = ATOMIC_INIT(0); /* check sense once */

static DEFINE_SPINLOCK(g_crbroi_lock);

static inline const char *str_match_result(int result)
{
	return result ? "[matched]" : "[unmatched]";
}

/*
 * routines to match and statistic storage read only write-fail with
 * method that provided by chip manufacturer
 */
static inline int match_ufs_rofault_samsung(const char *model,
	unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	(void)model;

	return (eol >= PRE_EOL_CRITICAL &&
		sense_key == NO_SENSE && asc == 0x0 && ascq == 0x0);
}

static inline int match_ufs_rofault_hynix(const char *model,
	unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	(void)model;

	return (eol >= PRE_EOL_CRITICAL &&
		sense_key == NO_SENSE && asc == 0x0 && ascq == 0x0);
}

static inline int match_ufs_rofault_sandisk(const char *model,
	unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	(void)model;

	/* for Eagle 8521 */
	return (eol >= PRE_EOL_CRITICAL &&
		sense_key == MEDIUM_ERROR && asc == 0x3 && ascq == 0x2);
}

static inline int match_ufs_rofault_toshiba(const char *model,
	unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	(void)model;
	(void)eol;
	(void)sense_key;
	(void)asc;
	(void)ascq;

	return 0;
}

static inline int match_ufs_rofault_micron(const char *model,
	unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	(void)model;

	return (eol >= PRE_EOL_CRITICAL && sense_key == ILLEGAL_REQUEST);
}

static inline int match_ufs_rofault_hisi(const char *model,
	unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	(void)model;

	return (eol >= PRE_EOL_NORMAL &&
		sense_key == NO_SENSE && asc == 0x0 && ascq == 0x0);
}

static const char *get_ufs_manf_str(unsigned int manfid)
{
	if (manfid == UFS_MANUFACTURER_ID_TOSHIBA)
		return STR_MANUFACTURER_TOSHIBA;
	else if (manfid == UFS_MANUFACTURER_ID_HYNIX)
		return STR_MANUFACTURER_HYNIX;
	else if (manfid == UFS_MANUFACTURER_ID_SAMSUNG)
		return STR_MANUFACTURER_SAMSUNG;
	else if (manfid == UFS_MANUFACTURER_ID_MICRON)
		return STR_MANUFACTURER_MICRON;
	else if (manfid == UFS_MANUFACTURER_ID_SANDISK)
		return STR_MANUFACTURER_SANDISK;
	else if (manfid == UFS_MANUFACTURER_ID_HI1861)
		return STR_MANUFACTURER_HISI;
	else
		return STR_MANUFACTURER_UNKNOWN;
}

static int match_ufs_rofault(unsigned int eol,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	int res = 0;

	if (g_bootdev_info.manfid == UFS_MANUFACTURER_ID_TOSHIBA)
		res = match_ufs_rofault_toshiba(g_bootdev_info.model, eol,
			sense_key, asc, ascq);
	else if (g_bootdev_info.manfid == UFS_MANUFACTURER_ID_HYNIX)
		res = match_ufs_rofault_hynix(g_bootdev_info.model, eol,
			sense_key, asc, ascq);
	else if (g_bootdev_info.manfid == UFS_MANUFACTURER_ID_SAMSUNG)
		res = match_ufs_rofault_samsung(g_bootdev_info.model, eol,
			sense_key, asc, ascq);
	else if (g_bootdev_info.manfid == UFS_MANUFACTURER_ID_MICRON)
		res = match_ufs_rofault_micron(g_bootdev_info.model, eol,
			sense_key, asc, ascq);
	else if (g_bootdev_info.manfid == UFS_MANUFACTURER_ID_SANDISK)
		res = match_ufs_rofault_sandisk(g_bootdev_info.model, eol,
			sense_key, asc, ascq);
	else if (g_bootdev_info.manfid == UFS_MANUFACTURER_ID_HI1861)
		res = match_ufs_rofault_hisi(g_bootdev_info.model, eol,
			sense_key, asc, ascq);

	PRINT_INFO("%s ufs rofault <%d, %d, %d> %s\n",
		get_ufs_manf_str(g_bootdev_info.manfid),
		sense_key, asc, ascq, str_match_result(res));

	return res;
}

static const char *get_emmc_manf_str(unsigned int manfid)
{
	if (manfid == EMMC_CID_MANFID_TOSHIBA)
		return STR_MANUFACTURER_TOSHIBA;
	else if (manfid == EMMC_CID_MANFID_HYNIX)
		return STR_MANUFACTURER_HYNIX;
	else if (manfid == EMMC_CID_MANFID_SAMSUNG)
		return STR_MANUFACTURER_SAMSUNG;
	else if (manfid == EMMC_CID_MANFID_MICRON)
		return STR_MANUFACTURER_MICRON;
	else if (manfid == EMMC_CID_MANFID_SANDISK ||
			manfid == EMMC_CID_MANFID_SANDISK_V2)
		return STR_MANUFACTURER_SANDISK;
	else
		return STR_MANUFACTURER_UNKNOWN;
}

static bool match_mmc_rofault(unsigned int eol, unsigned int resp)
{
	bool res = false;

	if (g_bootdev_info.manfid == EMMC_CID_MANFID_TOSHIBA)
		res = ((resp & (R1_WP_VIOLATION | R1_ILLEGAL_COMMAND)) != 0);
	else if (g_bootdev_info.manfid == EMMC_CID_MANFID_SAMSUNG)
		res = ((resp & R1_WP_VIOLATION) != 0);
	else if (g_bootdev_info.manfid == EMMC_CID_MANFID_MICRON)
		res = ((resp & R1_WP_VIOLATION) != 0);
	else if (g_bootdev_info.manfid == EMMC_CID_MANFID_SANDISK ||
			g_bootdev_info.manfid == EMMC_CID_MANFID_SANDISK_V2)
		res = ((resp & R1_WP_VIOLATION) != 0);

	PRINT_INFO("%s emmc rofault response - %u %s\n",
		get_emmc_manf_str(g_bootdev_info.manfid), resp,
		str_match_result(res));

	return res;
}

/*
 * External interfaces to kernel
 */
void storage_rochk_record_bootdevice_type(int type)
{
	g_bootdev_info.type = type;
}

void storage_rochk_record_bootdevice_manfid(unsigned int manfid)
{
	g_bootdev_info.manfid = manfid;
}

void storage_rochk_record_bootdevice_model(const char *model)
{
	if (model != NULL && strlen(model) < BOOTDEVICE_MODEL_SIZE) {
		strncpy(g_bootdev_info.model, model, BOOTDEVICE_MODEL_SIZE - 1);
	}
}

void storage_rochk_record_bootdevice_fwrev(const char *rev)
{
	if (rev != NULL && strlen(rev) < BOOTDEVICE_FWREV_SIZE) {
		PRINT_INFO("%s: record the device rev %c***\n",
			__func__, rev[0]);
		strncpy(g_bootdev_info.fwrev, rev, BOOTDEVICE_FWREV_SIZE - 1);
	}
}

void storage_rochk_record_bootdevice_pre_eol_info(int eol)
{
	g_bootdev_info.pre_eol_info = eol;
}

bool storage_rochk_filter_sd(const struct scsi_device *sdp)
{
	struct Scsi_Host *shost = sdp->host;

	if (shost != NULL && shost->hostt != NULL &&
	    shost->hostt->name != NULL &&
	    strncmp(shost->hostt->name, UFSHCD, strlen(UFSHCD) + 1) == 0)
		return true;
	else
		return false;
}

bool storage_rochk_is_mmc_card(const struct mmc_card *card)
{
	if (card == NULL)
		return false;

	return mmc_card_mmc(card);
}

/*
 * MUST called with filtered scsi_disk or mmc_card.
 */
int storage_rochk_record_disk(const char *name, int major, int minor)
{
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;
	int res = 0;

	PRINT_INFO("%s: add disk %s\n", __func__, name);

	mutex_lock(&g_bootdev_disks_mutex);

	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		if (strncmp(dinfo->disk_info.name, name,
		    BOOTDEVICE_DISK_NAME) == 0)
			goto out;
	}

	/* not existing, allocate it */
	dinfo = kzalloc(sizeof(*dinfo), GFP_KERNEL);
	if (dinfo == NULL) {
		PRINT_ERR("%s: allocate memory failed\n", __func__);
		res = -ENOMEM;
		goto out;
	}

	strncpy(dinfo->disk_info.name, name, BOOTDEVICE_DISK_NAME - 1);
	dinfo->disk_info.major = major;
	dinfo->disk_info.minor = minor;
	list_add_tail(&dinfo->disk_list, &g_bootdev_disks);

out:
	mutex_unlock(&g_bootdev_disks_mutex);
	return res;
}

/*
 * called in module_exit routine.
 */
static void storage_rochk_release_disk_info_list(void)
{
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;

	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		list_del(&dinfo->disk_list);
		kfree(dinfo);
	}
}

static atomic_t g_sd_rev_got = ATOMIC_INIT(0);

void storage_rochk_record_sd_rev_once(const struct scsi_device *sdev)
{
	char rev[8] = {0};

	if (sdev && sdev->inquiry_len >= 36 && sdev->inquiry != NULL) {
		if (atomic_cmpxchg(&g_sd_rev_got, 0, 1) == 0) {
			snprintf(rev, 5, "%.4s", sdev->inquiry + 32);
			storage_rochk_record_bootdevice_fwrev(rev);
		}
	}
}

/*
 * record storage write-protect status when device enumerated.
 * for ufs, record the status of device write-protect field from mode sense.
 * for emmc, record status read from csd[12|13].
 */
void storage_rochk_record_disk_wp_status(const char *name, unsigned char wp)
{
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;
	unsigned int found = 0;

	mutex_lock(&g_bootdev_disks_mutex);

	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		if (strncmp(dinfo->disk_info.name, name,
		    BOOTDEVICE_DISK_NAME) == 0) {
			PRINT_INFO("%s: record disk %s wp status to %d\n",
				__func__, name, wp);
			dinfo->disk_info.write_prot = wp;
			found = 1;
		}
	}

	mutex_unlock(&g_bootdev_disks_mutex);

	if (found == 0)
		PRINT_ERR("%s: disk %s not found\n", __func__, name);
}

void storage_rochk_record_disk_capacity(const char *name,
	unsigned long long capacity)
{
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;
	unsigned int found = 0;

	mutex_lock(&g_bootdev_disks_mutex);

	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		if (strncmp(dinfo->disk_info.name, name,
		    BOOTDEVICE_DISK_NAME) == 0) {
			PRINT_INFO("%s: record disk %s capacity to %llu\n",
				__func__, name, capacity);
			dinfo->disk_info.capacity = capacity;
			found = 1;
		}
	}

	mutex_unlock(&g_bootdev_disks_mutex);

	if (found == 0)
		PRINT_ERR("%s: disk %s not found\n", __func__, name);
}

/*
 * return -1 for non disk found, 0 for rw, 1 for wp.
 */
static int bootdevice_get_disk_wp_status(const char *name)
{
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;
	struct bootdevice_disk_info *dinfo_target = NULL;
	unsigned long long max_cap = 0;
	int res = -1;

	mutex_lock(&g_bootdev_disks_mutex);

	if (name != NULL) {
		list_for_each_entry_safe(dinfo, next, &g_bootdev_disks,
				disk_list) {
			if (strncmp(dinfo->disk_info.name, name,
			    BOOTDEVICE_DISK_NAME) == 0)
				dinfo_target = dinfo;
		}
	} else {
		list_for_each_entry_safe(dinfo, next, &g_bootdev_disks,
				disk_list) {
			if (dinfo->disk_info.capacity > max_cap) {
				max_cap = dinfo->disk_info.capacity;
				dinfo_target = dinfo;
			}
		}
	}

	if (dinfo_target != NULL) {
		res = (int)dinfo_target->disk_info.write_prot;
		PRINT_INFO("%s: got disk %s wp status %d\n",
			__func__, dinfo_target->disk_info.name, res);
	} else {
		PRINT_ERR("%s: wp target disk not found\n", __func__);
	}

	mutex_unlock(&g_bootdev_disks_mutex);

	return res;
}

static int storage_rochk_action_rofault(unsigned int method,
	unsigned int action)
{
	unsigned long flags;

	PRINT_INFO("%s: set crbroi content <%x, %x>\n",
		__func__, method, action);

	if (method != STORAGE_ROINFO_UNKNOWN_OR_UNCARE) {
		spin_lock_irqsave(&g_crbroi_lock, flags);
		get_storage_crbroi_func()->transfer_crbroi(method);
		spin_unlock_irqrestore(&g_crbroi_lock, flags);
	}

	if (action == STORAGE_ROCHK_ACTION_REBOOT) {
		PRINT_INFO("%s: machine restart...\n", __func__);
		machine_restart(NULL);
	}

	return 0;
}

/*
 * The following three routines are provided to eMMC/UFS driver
 * to judge early if the ro monitor is enabled and report chip write-fail
 * when request is done in ISR context.
 */
int storage_rochk_is_monitor_enabled(void)
{
	return atomic_read(&g_monitor_enabled);
}

/*
 * externally ensure sense_valid && !sense_deferred.
 * MUST called with filtered scsi_disk.
 * So, secondly guard it internaly with storage_rochk_filter_sd.
 */
void storage_rochk_monitor_sd_readonly(const struct scsi_device *sdp,
	const struct request *req, int result,
	unsigned int sense_key, unsigned int asc, unsigned int ascq)
{
	unsigned int eol = g_bootdev_info.pre_eol_info;
	int matched;
	int driver_res = driver_byte(result);
	int status_res = status_byte(result);
	unsigned int method;
	unsigned int action;

	if (atomic_read(&g_monitor_enabled) == 0)
		return;

	if (storage_rochk_filter_sd(sdp) == false)
		return;
	if (rq_data_dir(req) != WRITE)
		return;

	if (atomic_add_unless(&g_monitor_times, -1, 0) == 0) {
		atomic_set(&g_monitor_enabled, 0);
		return;
	}

	if (driver_res != DRIVER_SENSE || status_res != CHECK_CONDITION)
		return;

	matched = match_ufs_rofault(eol, sense_key, asc, ascq);
	if (matched) {
		action = STORAGE_ROCHK_ACTION_REBOOT;
		method = STORAGE_ROINFO_KNL_WRMON_DRIVER;
	} else {
		action = STORAGE_ROCHK_ACTION_NONE;
		method = STORAGE_ROINFO_KNL_WRMON_SUSPICIOUS;
	}

	/* action as early as possible for LLD rofault */
	if (atomic_cmpxchg(&g_action_once, 0, 1) == 0)
		storage_rochk_action_rofault(method, action);
}

static bool mrq_is_read_opcode(const struct mmc_request *mrq)
{
	return mrq->cmd->opcode == MMC_SEND_OP_COND ||
		mrq->cmd->opcode == MMC_ALL_SEND_CID ||
		mrq->cmd->opcode == MMC_SEND_EXT_CSD ||
		mrq->cmd->opcode == MMC_SEND_CSD ||
		mrq->cmd->opcode == MMC_SEND_CID ||
		mrq->cmd->opcode == MMC_SEND_STATUS ||
		mrq->cmd->opcode == MMC_READ_SINGLE_BLOCK ||
		mrq->cmd->opcode == MMC_READ_MULTIPLE_BLOCK ||
		mrq->cmd->opcode == MMC_READ_DAT_UNTIL_STOP;
}

static inline bool mrq_has_ro_type_resp(const struct mmc_request *mrq)
{
	return (mmc_resp_type(mrq->cmd) == MMC_RSP_R1 ||
		mmc_resp_type(mrq->cmd) == MMC_RSP_R1B) &&
		((mrq->cmd->resp[0] & MMC_RSP_R1_RO_ERROR_MASK) != 0);
}

static bool mrq_is_out_dir(const struct mmc_request *mrq)
{
	if (mrq->cmd != NULL && mrq_is_read_opcode(mrq) == false)
		return true;
	if (mrq->data != NULL && (mrq->data->flags & MMC_DATA_WRITE) != 0)
		return true;

	return false;
}

void storage_rochk_monitor_mmc_readonly(const struct mmc_card *card,
	const struct mmc_request *mrq, unsigned int status, unsigned int resp)
{
	unsigned int eol = g_bootdev_info.pre_eol_info;
	bool matched = false;
	unsigned int method;
	unsigned int action;
	bool do_action = false;

	if (atomic_read(&g_monitor_enabled) == 0)
		return;

	if (card == NULL || storage_rochk_is_mmc_card(card) == false)
		return;

	/*
	 * some controller may report TCC interrupt with null mrq
	 * after a busy request is done
	 */
	if (mrq == NULL)
		return;

	/* pass the write cmd and data request */
	if (mrq_is_out_dir(mrq) == false)
		return;

	if (atomic_add_unless(&g_monitor_times, -1, 0) == 0) {
		atomic_set(&g_monitor_enabled, 0);
		return;
	}

	if (status == MMC_STATUS_OK)
		return;

	if (status == MMC_STATUS_CQIS_RED) {
		matched = match_mmc_rofault(eol, resp);
		do_action = true;
	} else if (mrq->data != NULL && mrq->data->error != 0) {
		do_action = true;
	} else if (mrq->cmd != NULL && mrq_is_read_opcode(mrq) == false &&
		   mrq_has_ro_type_resp(mrq)) {
		matched = match_mmc_rofault(eol, mrq->cmd->resp[0]);
		do_action = true;
	}

	if (do_action == false)
		return;

	if (matched) {
		method = STORAGE_ROINFO_KNL_WRMON_DRIVER;
		action = STORAGE_ROCHK_ACTION_REBOOT;
	} else {
		method = STORAGE_ROINFO_KNL_WRMON_SUSPICIOUS;
		action = STORAGE_ROCHK_ACTION_NONE;
	}

	/* action as early as possible for LLD rofault */
	if (atomic_cmpxchg(&g_action_once, 0, 1) == 0)
		storage_rochk_action_rofault(method, action);
}

#ifdef CONFIG_HUAWEI_STORAGE_ROFA_FOR_MTK
static bool cmd_is_read_opcode(const struct mmc_command *cmd)
{
	return cmd->opcode == MMC_SEND_OP_COND ||
		cmd->opcode == MMC_ALL_SEND_CID ||
		cmd->opcode == MMC_SEND_EXT_CSD ||
		cmd->opcode == MMC_SEND_CSD ||
		cmd->opcode == MMC_SEND_CID ||
		((cmd->opcode == MMC_SEND_STATUS) &&
		(cmd->arg & (1 << 15))) ||
		cmd->opcode == MMC_EXECUTE_READ_TASK ||
		cmd->opcode == MMC_READ_SINGLE_BLOCK ||
		cmd->opcode == MMC_READ_MULTIPLE_BLOCK ||
		cmd->opcode == MMC_READ_DAT_UNTIL_STOP;
}

static inline bool cmd_has_ro_type_resp(const struct mmc_command *cmd)
{
	return (mmc_resp_type(cmd) == MMC_RSP_R1 ||
		mmc_resp_type(cmd) == MMC_RSP_R1B) &&
		((cmd->resp[0] & MMC_RSP_R1_RO_ERROR_MASK) != 0);
}

static bool cmd_is_out_dir(const struct mmc_command *cmd)
{
	if (cmd != NULL && cmd_is_read_opcode(cmd) == false)
		return true;
	return false;
}

void storage_rochk_monitor_mmc_cmd_readonly(
	const struct mmc_card *card,
	const struct mmc_command *cmd)
{
	unsigned int eol = g_bootdev_info.pre_eol_info;
	bool matched = false;
	unsigned int method;
	unsigned int action;
	bool do_action = false;
	bool decrease_monitor_times;

	if (atomic_read(&g_monitor_enabled) == 0)
		return;

	if (card == NULL || storage_rochk_is_mmc_card(card) == false)
		return;

	if (cmd == NULL)
		return;

	/* pass the write cmd and data request */
	if (cmd_is_out_dir(cmd) == false)
		return;

	decrease_monitor_times =
		(cmd->opcode == MMC_EXECUTE_WRITE_TASK) ||
		(cmd->opcode == MMC_WRITE_BLOCK) ||
		(cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK);
	if (decrease_monitor_times &&
	    (atomic_add_unless(&g_monitor_times, -1, 0) == 0)) {
		atomic_set(&g_monitor_enabled, 0);
		return;
	}

	if (cmd_has_ro_type_resp(cmd)) {
		matched = match_mmc_rofault(eol, cmd->resp[0]);
		do_action = true;
	}

	if (do_action == false)
		return;

	if (matched) {
		method = STORAGE_ROINFO_KNL_WRMON_DRIVER;
		action = STORAGE_ROCHK_ACTION_REBOOT;
	} else {
		method = STORAGE_ROINFO_KNL_WRMON_SUSPICIOUS;
		action = STORAGE_ROCHK_ACTION_NONE;
	}

	if (atomic_cmpxchg(&g_action_once, 0, 1) == 0)
		storage_rochk_action_rofault(method, action);
}
#endif

/*
 * Internal routines to implement storage readonly detect by writing test
 */
static int write_buffer(int fd, const char *buffer, int size)
{
	int bytes = -EINTR;

	while (bytes != 0) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
		bytes = ksys_write(fd, buffer, size);
#else
		bytes = sys_write(fd, buffer, size);
#endif
		if ((bytes < 0) && (bytes != -EINTR)) {
			return bytes;
		} else if (bytes == size) {
			break;
		} else if (bytes > 0) {
			buffer += bytes;
			size -= bytes;
		}
	}

	return 0;
}

/*
 * return value:
 * 0 for ok
 * WRITE_TRY_ERRCODE_OTHERS for non-storage error
 * WRITE_TRY_ERRCODE_BLOCK for storage error
 */
static unsigned int storage_ro_write_try(void)
{
	mm_segment_t old_fs;
	int fd_wr = -1;
	char *buffer = NULL;
	int res = -1;
	unsigned int ret = 0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	fd_wr = ksys_open(PARTITION_WRTRY_PATH, O_RDWR, 0);
#else
	fd_wr = sys_open(PARTITION_WRTRY_PATH, O_RDWR, 0);
#endif
	if (fd_wr < 0) {
		PRINT_ERR("%s: open partition failed\n", __func__);
		goto out;
	}

	buffer = kmalloc(PARTITION_WRTRY_LEN, GFP_KERNEL);
	if (buffer == NULL) {
		PRINT_ERR("%s: alloc buffer failed\n", __func__);
		goto out;
	}

	/* write try */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	res = ksys_lseek(fd_wr, -1024, SEEK_END);
#else
	res = sys_lseek(fd_wr, -1024, SEEK_END);
#endif
	if (res < 0) {
		PRINT_ERR("%s: seek file failed : %d\n", __func__, res);
		goto out;
	}

	res = write_buffer(fd_wr, buffer, PARTITION_WRTRY_LEN);
	if (res < 0) {
		PRINT_ERR("%s: write returns error : %d\n", __func__, res);
		goto err_out;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	res = ksys_fdatasync(fd_wr);
#else
	res = sys_fdatasync(fd_wr);
#endif
	if (res < 0) {
		PRINT_ERR("%s: fdatasync file failed : %d\n", __func__, res);
		goto err_out;
	}

err_out:
	if (res == -ENOLINK || res == -EREMOTEIO || res == -ETIMEDOUT ||
	    res == -EBADE || res == -ENOSPC || res == -ENODATA || res == -EIO)
		ret = WRITE_TRY_ERRCODE_BLOCK;
	else if (res < 0)
		ret = WRITE_TRY_ERRCODE_OTHERS;

out:
	if (fd_wr >= 0)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
		ksys_close(fd_wr);
#else
		sys_close(fd_wr);
#endif
	kfree(buffer);

	set_fs(old_fs);

	return ret;
}

static int storage_ro_write_try_thread(void *param)
{
	struct storage_rochk_completion *wrtry_comp = param;

	PRINT_INFO("%s: storage write-try thread start\n", __func__);

	wrtry_comp->result = storage_ro_write_try();
	complete(&wrtry_comp->event);

	return 0;
}

/*
 * ioctl implementation routines.
 */
int storage_rochk_ioctl_check_bootdisk_wp(
	struct bootdisk_wp_status_iocb *arg_u)
{
	struct bootdisk_wp_status_iocb ioc;
	char *disk_name = NULL;
	int wp;
	unsigned int method;
	unsigned int action;

	if (copy_from_user(&ioc, arg_u, sizeof(ioc)))
		return -EFAULT;

	if (ioc.use_name)
		disk_name = ioc.disk_name;
	else
		disk_name = NULL;

	wp = bootdevice_get_disk_wp_status(disk_name);
	if (wp == 1)
		ioc.write_prot = 1;
	else
		ioc.write_prot = 0;     /* set to 0 if not get */

	if (copy_to_user(arg_u, &ioc, sizeof(ioc)))
		return -EFAULT;

	if (wp == 1) {
		method = STORAGE_ROINFO_KNL_WRITE_PROT;
		action = STORAGE_ROCHK_ACTION_REBOOT;
		storage_rochk_action_rofault(method, action);
	}

	return 0;
}

int storage_rochk_ioctl_enable_monitor(struct storage_rochk_iocb *arg_u)
{
	struct storage_rochk_iocb ioc;
	int value;

	if (copy_from_user(&ioc, arg_u, sizeof(ioc)))
		return -EFAULT;

	value = ioc.data ? 1 : 0;
	atomic_set(&g_monitor_enabled, value);

	return 0;
}

int storage_rochk_ioctl_run_storage_wrtry_sync(
	struct storage_rochk_iocb *arg_u)
{
	struct storage_rochk_iocb ioc;
	struct storage_rochk_completion wrtry_comp;
	unsigned long complete_intime;
	unsigned int wr_res;
	unsigned int method;
	unsigned int action;

	init_completion(&wrtry_comp.event);
	wrtry_comp.result = 0;

	kthread_run(storage_ro_write_try_thread, (void *)&wrtry_comp,
		"storage_wrtry");

	complete_intime = wait_for_completion_timeout(&wrtry_comp.event,
		msecs_to_jiffies(500));
	if (complete_intime == 0) {
		wr_res = WRITE_TRY_ERRCODE_STUCK;
		PRINT_INFO("%s: wait for result timeout\n", __func__);
	} else {
		wr_res = wrtry_comp.result;
		PRINT_INFO("%s: get write-try result %u\n",
			__func__, wrtry_comp.result);
	}

	ioc.data = wr_res;
	if (copy_to_user(arg_u, &ioc, sizeof(ioc)))
		return -EFAULT;

	if (wr_res == WRITE_TRY_ERRCODE_OK) {
		PRINT_INFO("%s: storage write-try returns ok : rw\n",
			__func__);
		return 0;
	} else if (wr_res == WRITE_TRY_ERRCODE_BLOCK) {
		PRINT_INFO("%s: storage write-try returns ok : blk error\n",
			__func__);
		method = STORAGE_ROINFO_KNL_WRTRY_BLOCK;
		action = STORAGE_ROCHK_ACTION_REBOOT;
	} else if (wr_res == WRITE_TRY_ERRCODE_STUCK) {
		PRINT_INFO("%s: storage write-try returns ok : stuck\n",
			__func__);
		method = STORAGE_ROINFO_KNL_WRTRY_STUCK;
		action = STORAGE_ROCHK_ACTION_REBOOT;
	} else if (wr_res == WRITE_TRY_ERRCODE_OTHERS) {
		PRINT_INFO("%s: storage write-try returns ok : upper error\n",
			__func__);
		method = STORAGE_ROINFO_KNL_WRTRY_MISC;
		action = STORAGE_ROCHK_ACTION_NONE;
	} else {
		PRINT_ERR("%s: storage write-try returns unknown errcode %u\n",
			__func__, wr_res);
		return 0;
	}

	return storage_rochk_action_rofault(method, action);
}

int storage_rofa_ioctl_get_rofa_info(struct storage_rofa_info_iocb *arg_u)
{
	struct storage_rofa_info_iocb ioc;
	int res;

	ioc.mode = get_storage_rofa_bootopt();
	ioc.method = STORAGE_ROINFO_UNKNOWN_OR_UNCARE;
	ioc.round = 0;

	if (ioc.mode != STORAGE_ROFA_BOOTOPT_NOP) {
		res = get_storage_crbroi_func()->get_crbroi_method(
			&ioc.method);
		if (res == -1)
			ioc.method = STORAGE_ROINFO_UNKNOWN_OR_UNCARE;

		res = get_storage_crbroi_func()->get_crbroi_round(
			&ioc.round);
		if (res == -1)
			ioc.round = 0;
	}

	if (copy_to_user(arg_u, &ioc, sizeof(ioc)))
		return -EFAULT;

	return 0;
}

int storage_rochk_ioctl_get_bootdevice_disk_count(
	struct storage_rochk_iocb *arg_u)
{
	struct storage_rochk_iocb ioc;
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;

	mutex_lock(&g_bootdev_disks_mutex);

	ioc.data = 0;
	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		++ioc.data;
	}

	mutex_unlock(&g_bootdev_disks_mutex);

	if (copy_to_user(arg_u, &ioc, sizeof(ioc)))
		return -EFAULT;

	return 0;
}

int storage_rochk_ioctl_get_bootdevice_disk_info(
	struct bootdevice_disk_info_iocb *arg_u)
{
	struct bootdevice_disk_info_iocb iocb;
	struct bootdevice_disk_info *dinfo = NULL;
	struct bootdevice_disk_info *next = NULL;
	unsigned int disk_count;

	if (copy_from_user(&iocb, arg_u, sizeof(iocb)))
		return -EFAULT;

	mutex_lock(&g_bootdev_disks_mutex);

	disk_count = 0;
	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		++disk_count;
	}

	if (iocb.in_cnt < disk_count) {
		PRINT_ERR("%s: output buffer size insufficient\n", __func__);
		mutex_unlock(&g_bootdev_disks_mutex);
		return -EINVAL;
	}

	iocb.out_cnt = 0;
	list_for_each_entry_safe(dinfo, next, &g_bootdev_disks, disk_list) {
		if (copy_to_user(&arg_u->info_arr[iocb.out_cnt++],
		    &dinfo->disk_info, sizeof(dinfo->disk_info))) {
			mutex_unlock(&g_bootdev_disks_mutex);
			return -EFAULT;
		}
	}

	mutex_unlock(&g_bootdev_disks_mutex);

	if (copy_to_user(arg_u, &iocb, sizeof(iocb)))
		return -EFAULT;

	return 0;
}

int storage_rochk_ioctl_get_bootdevice_prod_info(
	struct bootdevice_prod_info_iocb *arg_u)
{
	struct bootdevice_prod_info_iocb info;

	memset(&info, 0, sizeof(info));
	snprintf(info.prod_info, sizeof(info.prod_info), "%s,%s",
		g_bootdev_info.model, g_bootdev_info.fwrev);

	if (copy_to_user(arg_u, &info, sizeof(info)))
		return -EFAULT;

	return 0;
}

static int __init storage_rochk_misc_init(void)
{
	int r;

	r = get_storage_crbroi_func()->crbroi_func_init();
	if (r) {
		PRINT_ERR("%s: init crbroi_func failed\n", __func__);
		return r;
	}

	return r;
}

static void __exit storage_rochk_misc_exit(void)
{
	int r;

	storage_rochk_release_disk_info_list();

	r = get_storage_crbroi_func()->crbroi_func_exit();
	if (r)
		PRINT_ERR("%s: exit crbroi_func failed\n", __func__);

}

module_init(storage_rochk_misc_init);
module_exit(storage_rochk_misc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("storage readonly check driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

#ifdef CONFIG_HUAWEI_STORAGE_ROFA_FAULT_INJECT

/*
 * Storage Read Only Fault Injection module
 */
#ifdef CONFIG_HUAWEI_STORAGE_ROFA_FOR_MTK
#include <soc/mediatek/log_store_kernel.h>
#else
#include <linux/hisi/rdr_hisi_platform.h>
#endif
#include <linux/scatterlist.h>
#include <linux/sched.h>
#include <linux/crc7.h>
#ifdef CONFIG_HUAWEI_STORAGE_ROFA_FOR_MTK
#ifdef CONFIG_BOOT_DETECTOR
#define STORAGE_ROCHK_RO_INJECT_OFFSET         16
#define STORAGE_ROCHK_FAULT_INJECT_PHYMEM_ADDR \
	(get_rofa_mem_base() + STORAGE_ROCHK_RO_INJECT_OFFSET)
#else
#define STORAGE_ROCHK_FAULT_INJECT_PHYMEM_ADDR  \
	(&(((struct sram_log_header *)CONFIG_MTK_DRAM_LOG_STORE_ADDR)->ro_inject))
#endif
#else
#define STORAGE_ROCHK_FAULT_INJECT_PHYMEM_ADDR  \
	(HISI_SUB_RESERVED_UNUSED_PHYMEM_BASE + 16)
#endif
#define EMMC_CSD_CMD_BLKSZ_LEN        16

#define FASTBOOT_STORAGE_ROCHK_ROFAULT_SAMSUNG_UFS      0x46490001
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_HYNIX_UFS        0x46490002
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_MICRON_UFS       0x46490003
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_SANDISK_UFS      0x46490004
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_TOSHIBA_UFS      0x46490005
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_HI1861_UFS       0x46490006
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_FORCE_ROCHK      0x46490007
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_DATA_PROTECT     0x46490008
#define FASTBOOT_STORAGE_ROCHK_ROFAULT_DOORBELL_TIMEOUT 0x46490010

#define KERNEL_STORAGE_ROCHK_ROFAULT_SAMSUNG_UFS        0x46490020
#define KERNEL_STORAGE_ROCHK_ROFAULT_HYNIX_UFS          0x46490021
#define KERNEL_STORAGE_ROCHK_ROFAULT_MICRON_UFS         0x46490022
#define KERNEL_STORAGE_ROCHK_ROFAULT_SANDISK_UFS        0x46490023
#define KERNEL_STORAGE_ROCHK_ROFAULT_TOSHIBA_UFS        0x46490024
#define KERNEL_STORAGE_ROCHK_ROFAULT_HI1861_UFS         0x46490025
#define KERNEL_STORAGE_ROCHK_ROFAULT_DATA_PROTECT       0x46490026
#define KERNEL_STORAGE_ROCHK_ROFAULT_MEDIUM_ERROR       0x46490027
#define KERNEL_STORAGE_ROCHK_ROFAULT_RECOVER_ERROR      0x46490028
#define KERNEL_STORAGE_ROCHK_ROFAULT_WRITE_PROT         0x4649002A

#define STORAGE_ROFI_FASTBOOT_EMMC_WP_VIOLATION         0x46490031
#define STORAGE_ROFI_KERNEL_EMMC_PWRON_WP_SET           0x46490038
#define STORAGE_ROFI_KERNEL_EMMC_PWRON_WP_CLR           0x46490039

static unsigned int g_rofi_value;

unsigned int storage_rofi_should_inject_check_condition_sense(void)
{
	if (g_rofi_value >= KERNEL_STORAGE_ROCHK_ROFAULT_SAMSUNG_UFS &&
	    g_rofi_value <= KERNEL_STORAGE_ROCHK_ROFAULT_RECOVER_ERROR)
		return 1;
	else
		return 0;
}

static void storage_rofi_inject_fault_ccs_samsung(struct scsi_sense_hdr *sshdr)
{
	storage_rochk_record_bootdevice_manfid(UFS_MANUFACTURER_ID_SAMSUNG);
	storage_rochk_record_bootdevice_pre_eol_info(3);
	sshdr->sense_key = NO_SENSE;
	sshdr->asc = 0x0;
	sshdr->ascq = 0x0;
}

static void storage_rofi_inject_fault_ccs_hynix(struct scsi_sense_hdr *sshdr)
{
	storage_rochk_record_bootdevice_manfid(UFS_MANUFACTURER_ID_HYNIX);
	storage_rochk_record_bootdevice_pre_eol_info(3);
	sshdr->sense_key = NO_SENSE;
	sshdr->asc = 0x0;
	sshdr->ascq = 0x0;
}

static void storage_rofi_inject_fault_ccs_micron(struct scsi_sense_hdr *sshdr)
{
	storage_rochk_record_bootdevice_manfid(UFS_MANUFACTURER_ID_MICRON);
	storage_rochk_record_bootdevice_pre_eol_info(3);
	sshdr->sense_key = ILLEGAL_REQUEST;
	sshdr->asc = 0x0;
	sshdr->ascq = 0x0;
}

static void storage_rofi_inject_fault_ccs_sandisk(struct scsi_sense_hdr *sshdr)
{
	storage_rochk_record_bootdevice_manfid(UFS_MANUFACTURER_ID_SANDISK);
	storage_rochk_record_bootdevice_pre_eol_info(3);
	sshdr->sense_key = MEDIUM_ERROR;
	sshdr->asc = 0x3;
	sshdr->ascq = 0x2;
}

static void storage_rofi_inject_fault_ccs_toshiba(struct scsi_sense_hdr *sshdr)
{
	(void)sshdr;
	storage_rochk_record_bootdevice_manfid(UFS_MANUFACTURER_ID_TOSHIBA);
	storage_rochk_record_bootdevice_pre_eol_info(3);
	/* chip unsupport now */
}

static void storage_rofi_inject_fault_ccs_hisi(struct scsi_sense_hdr *sshdr)
{
	(void)sshdr;
	storage_rochk_record_bootdevice_manfid(UFS_MANUFACTURER_ID_HI1861);
	storage_rochk_record_bootdevice_pre_eol_info(3);
	/* chip unsupport now */
}

static void storage_rofi_inject_fault_ccs_data_prot(
	struct scsi_sense_hdr *sshdr)
{
	sshdr->sense_key = DATA_PROTECT;
	sshdr->asc = 0x0;
	sshdr->ascq = 0x0;
}

static void storage_rofi_inject_fault_ccs_medium_error(
	struct scsi_sense_hdr *sshdr)
{
	sshdr->sense_key = MEDIUM_ERROR;
	sshdr->asc = 0x0;
	sshdr->ascq = 0x0;
}

static void storage_rofi_inject_fault_ccs_recovered_error(
	struct scsi_sense_hdr *sshdr)
{
	sshdr->sense_key = RECOVERED_ERROR;
	sshdr->asc = 0x0;
	sshdr->ascq = 0x0;
}

void storage_rofi_inject_fault_check_condition_sense(int *result,
	int *sense_valid, int *sense_deferred, unsigned int *good_bytes,
	struct request *req, struct scsi_device *sdp,
	struct scsi_sense_hdr *sshdr)
{
	unsigned int sense_data_injected = 0;
	unsigned int rofi = g_rofi_value;

	if (rq_data_dir(req) == WRITE) {
		if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_SAMSUNG_UFS) {
			storage_rofi_inject_fault_ccs_samsung(sshdr);
			sense_data_injected = 1;
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_HYNIX_UFS) {
			storage_rofi_inject_fault_ccs_hynix(sshdr);
			sense_data_injected = 1;
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_MICRON_UFS) {
			storage_rofi_inject_fault_ccs_micron(sshdr);
			sense_data_injected = 1;
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_SANDISK_UFS) {
			storage_rofi_inject_fault_ccs_sandisk(sshdr);
			sense_data_injected = 1;
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_TOSHIBA_UFS) {
			storage_rofi_inject_fault_ccs_toshiba(sshdr);
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_HI1861_UFS) {
			storage_rofi_inject_fault_ccs_hisi(sshdr);
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_DATA_PROTECT) {
			storage_rofi_inject_fault_ccs_data_prot(sshdr);
			sense_data_injected = 1;
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_MEDIUM_ERROR) {
			/* good_bytes will to sd_completed_bytes */
			storage_rofi_inject_fault_ccs_medium_error(sshdr);
			sense_data_injected = 1;
		} else if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_RECOVER_ERROR) {
			/* good bytes will to scsi_bufflen */
			storage_rofi_inject_fault_ccs_recovered_error(sshdr);
			sense_data_injected = 1;
		}

		if (sense_data_injected) {
			/* driver_byte | status_byte */
			*result = ((DRIVER_SENSE) << 24) |
				((CHECK_CONDITION) << 1);
			*sense_valid = 1;
			*sense_deferred = 0;
			*good_bytes = 0;

			PRINT_INFO_ONCE("rofi inject fault ccs <%d, %d, %d>\n",
				sshdr->sense_key, sshdr->asc, sshdr->ascq);
		}
	}
}

/*
 * write_prot is read in scsi_disk module init early than this module.
 * read rofi from the phys_mem directly.
 */
unsigned int storage_rofi_should_inject_write_prot_status(void)
{
	unsigned char *la = NULL;
	unsigned int rofi = 0;

#if defined(CONFIG_HUAWEI_STORAGE_ROFA_FOR_MTK) && defined(CONFIG_BOOT_DETECTOR)
	la = (unsigned char *)(STORAGE_ROCHK_FAULT_INJECT_PHYMEM_ADDR);
#else
	la = ioremap_nocache(STORAGE_ROCHK_FAULT_INJECT_PHYMEM_ADDR,
		sizeof(unsigned int));
#endif
	if (la != NULL) {
		rofi = readl(la);
		iounmap(la);
	}

	if (rofi == KERNEL_STORAGE_ROCHK_ROFAULT_WRITE_PROT)
		return 1;
	else
		return 0;
}

static int mmc_program_csd(const struct mmc_card *card,
	const unsigned char *csd_128be, unsigned int len)
{
	struct mmc_request mrq = { NULL };
	struct mmc_command cmd = {0};
	struct mmc_data data = {0};
	struct scatterlist sg;

	if (len != EMMC_CSD_CMD_BLKSZ_LEN)
		return -EINVAL;

	mrq.cmd = &cmd;
	mrq.data = &data;

	cmd.opcode = MMC_PROGRAM_CSD;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_R1 | MMC_CMD_ADTC;

	data.blksz = len;
	data.blocks = 1;	/* only one block */
	data.flags = MMC_DATA_WRITE;
	data.sg = &sg;
	data.sg_len = 1;	/* only one sg */
	mmc_set_data_timeout(&data, card);

	sg_init_one(&sg, csd_128be, len);

	mmc_wait_for_req(card->host, &mrq);

	if (cmd.error) {
		PRINT_ERR("%s: program csd cmd error %d\n",
			__func__, cmd.error);
		return cmd.error;
	}
	if (data.error) {
		PRINT_ERR("%s: program csd data error %d\n",
			__func__, data.error);
		return data.error;
	}

	return 0;
}

int storage_rofi_switch_mmc_card_pwronwp(const struct mmc_card *card)
{
	unsigned char *la = NULL;
	unsigned int wp;
	unsigned char *csd_be128 = NULL;
	unsigned int i;
	unsigned char crc7_val;
	int ret;

	if (card == NULL)
		return -EINVAL;

#if defined(CONFIG_HUAWEI_STORAGE_ROFA_FOR_MTK) && defined(CONFIG_BOOT_DETECTOR)
	la = (unsigned char *)(STORAGE_ROCHK_FAULT_INJECT_PHYMEM_ADDR);
#else
	la = ioremap_nocache(STORAGE_ROCHK_FAULT_INJECT_PHYMEM_ADDR,
		sizeof(unsigned int));
#endif
	if (la == NULL) {
		PRINT_ERR("%s: ioremap failed\n", __func__);
		return -EINVAL;
	}

	wp = readl(la);
	if (wp != STORAGE_ROFI_KERNEL_EMMC_PWRON_WP_SET &&
	    wp != STORAGE_ROFI_KERNEL_EMMC_PWRON_WP_CLR) {
		iounmap(la);
		return 0;
	}
	PRINT_ERR("%s: zero the emmc pwron-wp rofi\n", __func__);
	writel(0, la);
	iounmap(la);
	la = NULL;

	csd_be128 = kmalloc(EMMC_CSD_CMD_BLKSZ_LEN, GFP_KERNEL);
	if (csd_be128 == NULL) {
		PRINT_ERR("%s: alloc buffer failed\n", __func__);
		return -ENOMEM;
	}

	for (i = 0; i < (EMMC_CSD_CMD_BLKSZ_LEN / sizeof(unsigned int)); i++)
		((unsigned int *)csd_be128)[i] = cpu_to_be32(card->raw_csd[i]);

	/* update bit 12 with new temp_wp value */
	if (wp == STORAGE_ROFI_KERNEL_EMMC_PWRON_WP_SET)
		csd_be128[EMMC_CSD_WP_BYTE] |= EMMC_CSD_PWRON_WP_MASK;
	else if (wp == STORAGE_ROFI_KERNEL_EMMC_PWRON_WP_CLR)
		csd_be128[EMMC_CSD_WP_BYTE] &= ~EMMC_CSD_PWRON_WP_MASK;

	/* cal CRC7 of 15 bytes, set CRC7 into bits 7:1, and 1 into bit 0 */
	crc7_val = crc7_be(0, csd_be128, 15);
	csd_be128[EMMC_CSD_CRC_BYTE] = (crc7_val << 1) | 0x1;

	ret = mmc_program_csd(card, csd_be128, EMMC_CSD_CMD_BLKSZ_LEN);
	kfree(csd_be128);
	csd_be128 = NULL;
	if (ret == 0) {
		PRINT_INFO("%s: restart for emmc pwron-wp change\n", __func__);
		machine_restart(NULL);
	} else {
		PRINT_ERR("%s: program csd failed\n", __func__);
	}

	return ret;
}

static int __init storage_rofi_init(void)
{
	unsigned char *la = NULL;

#if defined(CONFIG_HUAWEI_STORAGE_ROFA_FOR_MTK) && defined(CONFIG_BOOT_DETECTOR)
	la = (unsigned char *)(STORAGE_ROCHK_FAULT_INJECT_PHYMEM_ADDR);
#else
	la = ioremap_nocache(STORAGE_ROCHK_FAULT_INJECT_PHYMEM_ADDR,
		sizeof(unsigned int));
#endif
	if (la != NULL) {
		g_rofi_value = readl(la);
		iounmap(la);
		return 0;
	}

	return -1;
}

static void __exit storage_rofi_exit(void)
{
}

module_init(storage_rofi_init);
module_exit(storage_rofi_exit);

#endif
