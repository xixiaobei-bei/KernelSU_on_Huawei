/*
 * storage_rofa.h
 *
 * storage Read Only First Aid interface
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

#ifndef STORAGE_ROFA_PUBLIC_H
#define STORAGE_ROFA_PUBLIC_H

#define MMC_STATUS_OK              0
#define MMC_STATUS_CQIS_RED        1

#define MMC_RESP_UNCARE            0

struct scsi_device;
struct request;
struct scsi_sense_hdr;
struct mmc_card;
struct mmc_request;
struct mmc_command;

/*
 * routines used to record storage information in ufs/emmc driver
 */
void storage_rochk_record_bootdevice_type(int type);
void storage_rochk_record_bootdevice_manfid(unsigned int manfid);
void storage_rochk_record_bootdevice_model(const char *model);
void storage_rochk_record_bootdevice_fwrev(const char *rev);
void storage_rochk_record_bootdevice_pre_eol_info(int eol);

bool storage_rochk_filter_sd(const struct scsi_device *sdp);
bool storage_rochk_is_mmc_card(const struct mmc_card *card);
int storage_rochk_record_disk(const char *name, int major, int minor);
void storage_rochk_record_sd_rev_once(const struct scsi_device *sdp);
void storage_rochk_record_disk_wp_status(const char *name,
	unsigned char wp);
void storage_rochk_record_disk_capacity(const char *name,
	unsigned long long capacity);

int storage_rochk_is_monitor_enabled(void);
void storage_rochk_monitor_sd_readonly(const struct scsi_device *sdp,
	const struct request *req, int result,
	unsigned int sense_key, unsigned int asc, unsigned int ascq);
void storage_rochk_monitor_mmc_readonly(const struct mmc_card *card,
	const struct mmc_request *mrq, unsigned int status,
	unsigned int resp);
void storage_rofa_info_clear(void);
#ifdef CONFIG_HUAWEI_STORAGE_ROFA_FOR_MTK
void storage_rochk_monitor_mmc_cmd_readonly(
	const struct mmc_card *card,
	const struct mmc_command *cmd);
#endif

#ifdef CONFIG_HUAWEI_STORAGE_ROFA_FAULT_INJECT
unsigned int storage_rofi_should_inject_check_condition_sense(void);
void storage_rofi_inject_fault_check_condition_sense(int *result,
	int *sense_valid, int *sense_deferred, unsigned int *good_bytes,
	struct request *req, struct scsi_device *sdp,
	struct scsi_sense_hdr *sshdr);
unsigned int storage_rofi_should_inject_write_prot_status(void);
int storage_rofi_switch_mmc_card_pwronwp(const struct mmc_card *card);
#endif

/*
 * routines used to implement ioctl interface
 */
struct storage_rochk_iocb;
struct bootdisk_wp_status_iocb;
struct storage_rofa_info_iocb;
struct bootdevice_disk_info_iocb;
struct bootdevice_prod_info_iocb;

int storage_rochk_ioctl_check_bootdisk_wp(
	struct bootdisk_wp_status_iocb *arg);
int storage_rochk_ioctl_enable_monitor(
	struct storage_rochk_iocb *arg);
int storage_rochk_ioctl_run_storage_wrtry_sync(
	struct storage_rochk_iocb *arg);
int storage_rofa_ioctl_get_rofa_info(
	struct storage_rofa_info_iocb *arg);
int storage_rochk_ioctl_get_bootdevice_disk_count(
	struct storage_rochk_iocb *arg);
int storage_rochk_ioctl_get_bootdevice_disk_info(
	struct bootdevice_disk_info_iocb *arg);
int storage_rochk_ioctl_get_bootdevice_prod_info(
	struct bootdevice_prod_info_iocb *arg);

/*
 * "storage_rofa" boot cmdline option
 */
#define STORAGE_ROFA_BOOTOPT_NOP    0
#define STORAGE_ROFA_BOOTOPT_CHECK  1
#define STORAGE_ROFA_BOOTOPT_BYPASS 2

unsigned int get_storage_rofa_bootopt(void);

#endif // STORAGE_ROFA_PUBLIC_H
