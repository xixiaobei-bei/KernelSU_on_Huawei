/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: bfm_core.h
 *
 * define the core's external public enum/macros/interface for BFM
 * (Boot Fail Monitor)
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

#ifndef BFM_CORE_H
#define BFM_CORE_H

#include <linux/ioctl.h>
#include <linux/types.h>

#include "chipset_common/bfmr/public/bfmr_public.h"
#include "chipset_common/bfmr/common/bfmr_common.h"
#include "chipset_common/bfmr/bfr/core/bfr_core.h"
#include "chipset_common/bfmr/bfm/core/bfm_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct bfmr_dev_path {
	char dev_name[BFMR_SIZE_64];
	char dev_path[BFMR_SIZE_1K];
};

struct bfmr_bf_info {
	int bf_no;
	int suggested_rmethod;
	struct bfmr_bf_addl_info addl_info;
};

struct bfmr_dbrd_ioctl_block {
	unsigned int kbytes;
	int number; /* compatible with brd's brd_number */
};

#define BFMR_SIZE_DISK_NAME 32
#define BFMR_SIZE_BOOTDEVICE_PROD_INFO 64
#define DISK_NAME_MAX_SIZE 32
#define BOOTDEVICE_PROD_INFO_MAX_SIZE 64

/* ioctl data structure */
struct bfmr_storage_rochk_iocb {
	unsigned int data;
};

struct bfmr_bootdisk_wp_status_iocb {
	char disk_name[BFMR_SIZE_DISK_NAME];
	unsigned int use_name;
	unsigned int write_prot;
};

struct bfmr_storage_rofa_info_iocb {
	unsigned int mode;
	unsigned int round;
	unsigned int method;
};

struct bfmr_disk_info {
	char name[BFMR_SIZE_DISK_NAME];
	int major;
	int minor;
	unsigned long long capacity;
	unsigned int write_prot;
};

struct bfmr_bootdevice_disk_info_iocb {
	unsigned int in_cnt;
	unsigned int out_cnt;
	struct bfmr_disk_info info_arr[0];
} __packed;

struct bfmr_bootdevice_prod_info_iocb {
	char prod_info[BFMR_SIZE_BOOTDEVICE_PROD_INFO];
};

struct dbrd_ioctl_block {
        unsigned int kbytes;
        int number; /* compatible with brd's brd_number */
};

struct storage_rochk_iocb {
	unsigned int data;
};

struct bootdisk_wp_status_iocb {
	char disk_name[DISK_NAME_MAX_SIZE];
	unsigned int use_name;
	unsigned int write_prot;
};

struct storage_rofa_info_iocb {
	unsigned int mode;
	unsigned int round;
	unsigned int method;
};

struct disk_info {
	char name[DISK_NAME_MAX_SIZE];
	int major;
	int minor;
	unsigned long long capacity;
	unsigned int write_prot;
};

struct bootdevice_disk_info_iocb {
	unsigned int in_cnt;
	unsigned int out_cnt;
	struct disk_info info_arr[0];
} __packed;

struct bootdevice_prod_info_iocb {
	char prod_info[BOOTDEVICE_PROD_INFO_MAX_SIZE];
};

struct bfmr_rmethod_maptable {
	enum bfr_method rmethod;
	enum bfmr_detail_stage boot_stage;
	enum bfmr_selfheal_code selfheal_code;
};

#define BFMR_IOCTL_BASE 'B'
#define BFMR_GET_TIMER_STATUS _IOR(BFMR_IOCTL_BASE, 1, int)
#define BFMR_ENABLE_TIMER _IOW(BFMR_IOCTL_BASE, 2, int)
#define BFMR_DISABLE_TIMER _IOW(BFMR_IOCTL_BASE, 3, int)
#define BFMR_GET_TIMER_TIMEOUT_VALUE _IOR(BFMR_IOCTL_BASE, 4, int)
#define BFMR_SET_TIMER_TIMEOUT_VALUE _IOW(BFMR_IOCTL_BASE, 5, int)
#define BFMR_GET_BOOT_STAGE  _IOR(BFMR_IOCTL_BASE, 6, int)
#define BFMR_SET_BOOT_STAGE  _IOW(BFMR_IOCTL_BASE, 7, int)
#define BFMR_PROCESS_BOOT_FAIL _IOW(BFMR_IOCTL_BASE, 8, struct bfmr_bf_info)
#define BFMR_GET_DEV_PATH _IOR(BFMR_IOCTL_BASE, 9, struct bfmr_dev_path)
#define BFMR_ENABLE_CTRL _IOW(BFMR_IOCTL_BASE, 10, int)
#define BFMR_ACTION_TIMER_CTL _IOW(BFMR_IOCTL_BASE, 11,\
	struct action_ioctl_data)
#define BFMR_IOC_CREATE_DYNAMIC_RAMDISK _IOWR(BFMR_IOCTL_BASE, 12,\
	struct bfmr_dbrd_ioctl_block)
#define BFMR_IOC_DELETE_DYNAMIC_RAMDISK _IOW(BFMR_IOCTL_BASE, 13,\
	struct bfmr_dbrd_ioctl_block)
#define BFMR_IOC_CHECK_BOOTDISK_WP _IOWR(BFMR_IOCTL_BASE, 14,\
	struct bfmr_bootdisk_wp_status_iocb)
#define BFMR_IOC_ENABLE_MONITOR   _IOW(BFMR_IOCTL_BASE, 15,\
	struct bfmr_storage_rochk_iocb)
#define BFMR_IOC_DO_STORAGE_WRTRY _IOWR(BFMR_IOCTL_BASE, 16,\
	struct bfmr_storage_rochk_iocb)
#define BFMR_IOC_GET_STORAGE_ROFA_INFO _IOWR(BFMR_IOCTL_BASE, 17,\
	struct bfmr_storage_rofa_info_iocb)
#define BFMR_IOC_GET_BOOTDEVICE_DISK_COUNT _IOWR(BFMR_IOCTL_BASE, 18,\
	struct bfmr_storage_rochk_iocb)
#define BFMR_IOC_GET_BOOTDEVICE_DISK_INFO _IOWR(BFMR_IOCTL_BASE, 19,\
	struct bfmr_bootdevice_disk_info_iocb)
#define BFMR_IOC_GET_BOOTDEVICE_PROD_INFO _IOWR(BFMR_IOCTL_BASE, 20,\
	struct bfmr_bootdevice_prod_info_iocb)

/*
 * @brief: save the log and do proper recovery actions when meet with error
 * during system booting process.
 *
 * @param: bootfail_errno [in], boot fail error no,
 *         suggested_recovery_method [in], suggested recovery method.
 * if you don't know, please transfer NO_SUGGESTION for it
 *
 * @param: paddl_info [in], saving additional info such as log path and so on.
 *
 * @return: 0 - succeeded; -1 - failed.
 */
int boot_fail_err(enum bfm_errno_code bootfail_errno,
	enum bfr_suggested_method suggested_recovery_method,
	struct bfmr_bf_addl_info *paddl_info);

/*
 * @brief: get current boot stage during boot process.
 *
 * @param: pboot_stage [out], buffer storing the boot stage.
 *
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_get_boot_stage(enum bfmr_detail_stage *pboot_stage);

/*
 * @brief: get current boot stage during boot process.
 *
 * @param: boot_stage [in], boot stage to be set.
 *
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_set_boot_stage(enum bfmr_detail_stage boot_stage);

/*
 * @brief: get state of the timer.
 *
 * @param: state [out], the state of the boot timer.
 *
 * @return: 0 - success, -1 - failed.
 *
 * @note:
 *	1. this fuction only need be initialized in kernel.
 *	2. if *state == 0, the boot timer is disabled, if *state == 1,
 *	the boot timer is enbaled.
 */
int bfmr_get_timer_state(int *state);

/*
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_enable_timer(void);

/*
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_disable_timer(void);

/*
 * @brief: set timeout value of the kernel timer. Note: the timer which
 * control the boot procedure is in the kernel.
 *
 * @param: timeout [in] timeout value (unit: msec).
 *
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_set_timer_timeout_value(unsigned int timeout);

/*
 * @brief: get timeout value of the kernel timer. Note: the timer which
 * control the boot procedure is in the kernel.
 *
 * @param: timeout [in] buffer will store the timeout value (unit: msec).
 *
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_get_timer_timeout_value(unsigned int *timeout);

/*
 * @brief: init bfm module in kernel.
 *
 * @return: 0 - succeeded; -1 - failed.
 *
 * @note: it need be initialized in bootloader and kernel.
 */
int bfm_init(void);
int bfm_get_log_count(char *bfmr_log_root_path);
void bfm_delete_dir(char *log_path);

/* send sgnal to init, it will show init task trace. */
void bfm_send_signal_to_init(void);

#ifdef __cplusplus
}
#endif

#endif

