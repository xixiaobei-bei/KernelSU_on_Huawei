/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: bootfail_common.h
 *
 * define bootfail ioctl command
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

#ifndef BOOTFAIL_COMMON_H
#define BOOTFAIL_COMMON_H

/* ---- includes ---- */
#include <linux/types.h>
#include <hwbootfail/core/adapter.h>
#include <hwbootfail/core/boot_interface.h>

/* ---- c++ support ---- */
#ifdef __cplusplus
extern "C" {
#endif

/* ---- export macroes ---- */
struct dbrd_ioctl_block;
struct storage_rochk_iocb;
struct bootdisk_wp_status_iocb;
struct storage_rofa_info_iocb;
struct disk_info;
struct bootdevice_disk_info_iocb;
struct bootdevice_prod_info_iocb;
struct bootfail_info;
struct dev_path_info;
#define BOOT_DETECTOR_IOCTL_BASE 'B'
#define GET_TIMER_STATUS _IOR(BOOT_DETECTOR_IOCTL_BASE, 1, int)
#define RESUME_TIMER _IOW(BOOT_DETECTOR_IOCTL_BASE, 2, int)
#define SUSPEND_TIMER _IOW(BOOT_DETECTOR_IOCTL_BASE, 3, int)
#define GET_TIMER_TIMEOUT_VALUE _IOR(BOOT_DETECTOR_IOCTL_BASE, 4, int)
#define SET_TIMER_TIMEOUT_VALUE _IOW(BOOT_DETECTOR_IOCTL_BASE, 5, int)
#define GET_BFI_DEV_PATH _IOR(BOOT_DETECTOR_IOCTL_BASE,\
	9, struct bfi_dev_path)
#define LEGACY_ENABLE_CTRL _IOW(BOOT_DETECTOR_IOCTL_BASE, 10, int)
#define ACTION_TIMER_CTL _IOW(BOOT_DETECTOR_IOCTL_BASE, 11,\
	struct action_ioctl_data)
#define IOC_CREATE_DYNAMIC_RAMDISK _IOWR(BOOT_DETECTOR_IOCTL_BASE, 12,\
	struct dbrd_ioctl_block)
#define IOC_DELETE_DYNAMIC_RAMDISK _IOW(BOOT_DETECTOR_IOCTL_BASE, 13,\
	struct dbrd_ioctl_block)
#define IOC_CHECK_BOOTDISK_WP _IOWR(BOOT_DETECTOR_IOCTL_BASE, 14,\
	struct bootdisk_wp_status_iocb)
#define IOC_ENABLE_MONITOR   _IOW(BOOT_DETECTOR_IOCTL_BASE, 15,\
	struct storage_rochk_iocb)
#define IOC_DO_STORAGE_WRTRY _IOWR(BOOT_DETECTOR_IOCTL_BASE, 16,\
	struct storage_rochk_iocb)
#define IOC_GET_STORAGE_ROFA_INFO _IOWR(BOOT_DETECTOR_IOCTL_BASE, 17,\
	struct storage_rofa_info_iocb)
#define IOC_GET_BOOTDEVICE_DISK_COUNT _IOWR(BOOT_DETECTOR_IOCTL_BASE, 18,\
	struct storage_rochk_iocb)
#define IOC_GET_BOOTDEVICE_DISK_INFO _IOWR(BOOT_DETECTOR_IOCTL_BASE, 19,\
	struct bootdevice_disk_info_iocb)
#define IOC_GET_BOOTDEVICE_PROD_INFO _IOWR(BOOT_DETECTOR_IOCTL_BASE, 20,\
	struct bootdevice_prod_info_iocb)
#define GET_BOOT_STAGE _IOR(BOOT_DETECTOR_IOCTL_BASE, 100, int)
#define SET_BOOT_STAGE _IOW(BOOT_DETECTOR_IOCTL_BASE, 101, int)
#define PROCESS_BOOTFAIL \
	_IOW(BOOT_DETECTOR_IOCTL_BASE, 102, struct bootfail_proc_param)
#define BOOT_DETECTOR_ENABLE_CTRL _IOW(BOOT_DETECTOR_IOCTL_BASE, 103, int)
#define BOOT_DETECTOR_DBG_LEVEL_CTRL \
	_IOW(BOOT_DETECTOR_IOCTL_BASE, 104, unsigned int)
#define GET_KEY_STRUCT_SIZE \
	_IOR(BOOT_DETECTOR_IOCTL_BASE, 1000, struct key_struct_size)
#define SIMULATE_STORAGE_RDONLY \
	_IOW(BOOT_DETECTOR_IOCTL_BASE, 1001, int)

#define DISK_NAME_MAX_SIZE 32
#define BOOTDEVICE_PROD_INFO_MAX_SIZE 64

#define print_invalid_params(format, ...) pr_err(\
	"bootfail: func: %s invalid params: " format, __func__, ##__VA_ARGS__)
#define print_err(format, ...) pr_err("bootfail: func: %s line: %d, " \
	format, __func__, __LINE__, ##__VA_ARGS__)
#define print_info(format, ...) pr_err("bootfail: " format, ##__VA_ARGS__)
#define print_enter(format, ...) pr_err("bootfail: enter [%s], line: %d.\n" \
	format, __func__, __LINE__, ##__VA_ARGS__)
#define print_exit(format, ...) pr_err("bootfail: exit [%s], line: %d.\n" \
	format, __func__, __LINE__, ##__VA_ARGS__)

#define BOOT_DETECTOR_UT (0X55545554) /* UT - Unit Test*/
#define BOOT_DETECTOR_MST (0X4D844D84) /* MST - MST Test*/
#define BOOT_DETECTOR_ST (0X53545354) /* ST - System Test*/
#define BOOT_DETECTOR_AT (0X41544154) /* AT - Acceptance Test*/

#if BITS_PER_LONG == 32
#define bootfail_lstat sys_lstat64
#else
#define bootfail_lstat sys_newlstat
#endif

/* ---- export prototypes ---- */
#if BITS_PER_LONG == 32
typedef struct stat64 bootfail_stat_t;
#else
typedef struct stat bootfail_stat_t;
#endif

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

struct common_init_params {
	void *ctx;
};

struct bfi_dev_path {
	char path[128];
};

struct key_struct_size {
	size_t binfo_size;
	size_t proc_param_size;
	size_t bfi_part_hdr_size;
	size_t meta_size;
	size_t log_hdr_size;
	size_t rrecord_hdr_size;
	size_t rrecord_size;
	size_t recovery_size;
	size_t bfi_part_size;
};

/*---- export function prototypes ----*/
int is_boot_detector_enabled(void);
int is_boot_recovery_enabled(void);
int is_bopd_supported(void);
bool check_recovery_mode(void);
bool is_erecovery_mode(void);
bool data_can_be_mounted(void);
void dump_init_log(void);
unsigned long long get_sys_rtc_time(void);
bool is_dir_existed(const char *pdir_path);
bool wait_file_node_ready(const char *file_node, int timeouts);
int read_from_phys_mem(unsigned long dst,
	unsigned long dst_max,
	void *phys_mem_addr,
	unsigned long data_len);
int write_to_phys_mem(unsigned long dst,
	unsigned long dst_max,
	void *src,
	unsigned long src_len);
void get_data_part_info(struct bootfail_proc_param *pparam);
int bootfail_common_init(struct common_init_params *pparam);
int read_part(const char *dev_path,
	unsigned long long offset,
	char *buf,
	unsigned long long buf_size);
int write_part(const char *dev_path,
	unsigned long long offset,
	const char *buf,
	unsigned long long buf_size);
int process_data_mount_in_erecovery(const char __user *dir_name);

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
