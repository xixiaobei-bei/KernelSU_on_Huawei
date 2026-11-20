/*
 * dsm_ufs.c
 *
 * deal with dsm_ufs
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "dsm_ufs.h"
#include <dsm/dsm_pub.h>
#include <linux/async.h>
#include <linux/of_gpio.h>
#include "ufs.h"
#include "ufshcd.h"
#include "unipro.h"
#include "mphy-unipro-vs.h"
#include "ufshcd-kirin-interface.h"

/* lint -e747 -e713 -e715 -e732 -e835 */
struct ufs_dsm_log  g_ufs_dsm_log;
static struct ufs_dsm_adaptor g_ufs_dsm_adaptor;
struct dsm_client *ufs_dclient;
unsigned int ufs_dsm_real_upload_size;
static int dsm_ufs_enable;
struct ufs_uic_err_history uic_err_history;
struct dsm_dev dsm_ufs = {
	.name = "dsm_ufs",
	.buff_size = UFS_DSM_BUFFER_SIZE,
};

struct dsm_err_type_category err_type_category[] = {
	{UFS_FASTBOOT_PWMODE_ERR, DSM_UFS_FASTBOOT_PWMODE_ERR, "Fastboot PwrMode Error"},
	{UFS_FASTBOOT_RW_ERR, DSM_UFS_FASTBOOT_RW_ERR, "Fastboot RW Error"},
	{UFS_UIC_TRANS_ERR, DSM_UFS_UIC_TRANS_ERR, "UIC No Fatal Error"},
	{UFS_UIC_CMD_ERR, DSM_UFS_UIC_CMD_ERR, "UIC Cmd Error"},
	{UFS_CONTROLLER_ERR, DSM_UFS_CONTROLLER_ERR, "Ufs Controller Error"},
	{UFS_DEV_ERR, DSM_UFS_DEV_ERR, "UFS Device Error"},
	{UFS_TIMEOUT_ERR, DSM_UFS_TIMEOUT_ERR, "UFS TimeOut Error"},
	{UFS_UTP_TRANS_ERR, DSM_UFS_UTP_ERR, "UTP Transfer Error"},
	{UFS_SCSI_CMD_ERR, DSM_UFS_SCSI_CMD_ERR, "UFS SCSI CMD Error"},
	{UFS_VOLT_GPIO_ERR, DSM_UFS_VOLT_GPIO_ERR, "UFS Volt Fall Error"},
	{UFS_LINKUP_ERR, DSM_UFS_LINKUP_ERR, "UFS Linkup Error"},
	{UFS_ENTER_OR_EXIT_H8_ERR, DSM_UFS_ENTER_OR_EXIT_H8_ERR, "UFS Enter/Exit H8 Fail"},
	{UFS_LIFETIME_EXCCED_ERR, DSM_UFS_LIFETIME_EXCCED_ERR, "UFS Life time exceed"},
	{UFS_TIMEOUT_SERIOUS, DSM_UFS_TIMEOUT_SERIOUS, "UFS_TIMEOUT_SERIOUS"},
	{UFS_DEV_INTERNEL_ERR, DSM_UFS_DEV_INTERNEL_ERR, "UFS_DEV_INTERNEL_ERR"},
	{UFS_INLINE_CRYPTO_ERR, DSM_UFS_INLINE_CRYPTO_ERR, "UFS inline crypto error"},
	{UFS_HARDWARE_ERR, DSM_UFS_HARDWARE_ERR, "UFS hardware error:Sense_key = 4"},
	{UFS_LINE_RESET_ERR, DSM_UFS_LINE_RESET_ERR, "UFS occur Line reset"},
	{UFS_TEMP_LOW_ERR, DSM_UFS_TEMP_LOW_ERR, "UFS temperature too low"},
	{UFS_HI1861_INTERNEL_ERR, DSM_UFS_HI1861_INTERNEL_ERR, "DSM_UFS_HI1861_INTERNEL_ERR"},
	{UFS_POWER_DOWN_FLUSH_ERR, DSM_UFS_POWER_DOWN_FLUSH_ERR, "UFS_POWER_DOWN_FLUSH_ERR"},
};

EXPORT_SYMBOL(ufs_dclient);

unsigned long dsm_ufs_if_err(void)
{
	return g_ufs_dsm_adaptor.err_type;
}

int dsm_ufs_update_upiu_info(struct ufs_hba *hba, int tag, int err_code)
{
	struct scsi_cmnd *scmd = NULL;
	int i;
	if (!dsm_ufs_enable)
		return -1;
	if (err_code == DSM_UFS_TIMEOUT_ERR) {
		if (test_and_set_bit_lock(UFS_TIMEOUT_ERR,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
	} else if (err_code == DSM_UFS_TIMEOUT_SERIOUS) {
		if (test_and_set_bit_lock(UFS_TIMEOUT_SERIOUS,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
	} else if (err_code == DSM_UFS_POWER_DOWN_FLUSH_ERR) {
		if (test_and_set_bit_lock(UFS_POWER_DOWN_FLUSH_ERR,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
	} else {
		return -1;
	}
	if ((tag != -1) && (hba->lrb[tag].cmd)) {
		scmd = hba->lrb[tag].cmd;
		for (i = 0; i < COMMAND_SIZE(scmd->cmnd[0]); i++)
		g_ufs_dsm_adaptor.timeout_scmd[i] = scmd->cmnd[i];
	} else {
		for (i = 0; i < MAX_CDB_SIZE; i++)
		g_ufs_dsm_adaptor.timeout_scmd[i] = 0;
	}
	g_ufs_dsm_adaptor.timeout_slot_id = tag;
	g_ufs_dsm_adaptor.tr_doorbell = read_utr_doorbell(hba);
	g_ufs_dsm_adaptor.outstanding_reqs = hba->outstanding_reqs;

	return 0;

}
static u32 crypto_conf_index[CRYPTO_CONFIG_LEN];
int dsm_ufs_updata_ice_info(struct ufs_hba *hba)
{
	unsigned int i;
	if (!dsm_ufs_enable)
		return -1;
	if (test_and_set_bit_lock(UFS_INLINE_CRYPTO_ERR,
	&g_ufs_dsm_adaptor.err_type))
		return -1;
	g_ufs_dsm_adaptor.ice_outstanding = hba->outstanding_reqs;
	g_ufs_dsm_adaptor.ice_doorbell = read_utr_doorbell(hba);
	for (i = 0; i < (unsigned int)hba->nutrs; i++) {
		if (!((g_ufs_dsm_adaptor.ice_doorbell >> i) & 0x1))
			continue;
		if (ufshcd_is_hufs_hc(hba))
			crypto_conf_index[i] =
				(hba->lrb[i].hufs_utr_descriptor_ptr)
					->header.dword_0;
		else
			crypto_conf_index[i] = (hba->lrb[i].utr_descriptor_ptr)
						       ->header.dword_0;
	}
	return 0;
}
int dsm_ufs_update_scsi_info(struct ufshcd_lrb *lrbp,
	int scsi_status, int err_code)
{
	struct scsi_cmnd *scmd = NULL;
	int i;
	if (!dsm_ufs_enable)
		return -1;
	switch (err_code) {
	case DSM_UFS_DEV_INTERNEL_ERR:
		if (test_and_set_bit_lock(UFS_DEV_INTERNEL_ERR,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
		break;
	case DSM_UFS_SCSI_CMD_ERR:
		if (test_and_set_bit_lock(UFS_SCSI_CMD_ERR,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
		break;
	case DSM_UFS_HARDWARE_ERR:
		if (test_and_set_bit_lock(UFS_HARDWARE_ERR,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
		break;
	default:
		return -1;
	}
	if (lrbp->cmd)
		scmd = lrbp->cmd;
	else
		return -1;
	g_ufs_dsm_adaptor.scsi_status = scsi_status;
	for (i = 0; i < COMMAND_SIZE(scmd->cmnd[0]); i++)
		g_ufs_dsm_adaptor.timeout_scmd[i] = scmd->cmnd[i];
	for (i = 0; i < SCSI_SENSE_BUFFERSIZE; i++)
		g_ufs_dsm_adaptor.sense_buffer[i] = lrbp->sense_buffer[i];
	return 0;
}

int dsm_ufs_update_ocs_info(struct ufs_hba *hba, int err_code, int ocs)
{
	if (!dsm_ufs_enable)
		return -1;
	if (test_and_set_bit_lock(UFS_UTP_TRANS_ERR, &g_ufs_dsm_adaptor.err_type))
		return -1;
	g_ufs_dsm_adaptor.ocs = ocs;
	return 0;
}

int dsm_ufs_update_fastboot_info(struct ufs_hba *hba)
{
	char *pstr = NULL;
	unsigned int err_code;
	if (!dsm_ufs_enable)
		return -1;
	pstr = strstr(saved_command_line, "fastbootdmd=");
	if (!pstr) {
		pr_err("No fastboot dmd info\n");
		return -EINVAL;
	}
	if (1 != sscanf(pstr, "fastbootdmd=%d", &err_code)) {
		pr_err("Failed to get err_code\n");
		return -EINVAL;
	}
	if (err_code) {
		if (err_code & FASTBOOTDMD_PWR_ERR)
			if (test_and_set_bit_lock(UFS_FASTBOOT_PWMODE_ERR,
				&g_ufs_dsm_adaptor.err_type))
				return -1;
		if (err_code & FASTBOOTDMD_RW_ERR)
			if (test_and_set_bit_lock(UFS_FASTBOOT_RW_ERR,
				&g_ufs_dsm_adaptor.err_type))
				return -1;
	} else {
		return -EINVAL;
	}
	return 0;
}

static void dsm_ufs_fastboot_async(void *data, async_cookie_t cookie)
{
	int ret;
	struct ufs_hba *hba = (struct ufs_hba *)data;
	ret = dsm_ufs_update_fastboot_info(hba);
	if (!ret) {
		pr_info("Ufs get fastboot dmd err info\n");
		if (dsm_ufs_enabled())
			queue_work(system_freezable_wq, &hba->dsm_work);
	}
}

void dsm_ufs_enable_uic_err(struct ufs_hba *hba)
{
	unsigned long flags;
	clear_bit_unlock(0x0, &(g_ufs_dsm_adaptor.uic_disable));
	spin_lock_irqsave(hba->host->host_lock, flags); /* lint !e550 */
	ufshcd_enable_intr(hba, UIC_ERROR);
	spin_unlock_irqrestore(hba->host->host_lock, flags); /* lint !e550 */
	return;
}

int dsm_ufs_disable_uic_err(void)
{
	return (test_and_set_bit_lock(0x0, &(g_ufs_dsm_adaptor.uic_disable)));
}

int dsm_ufs_if_uic_err_disable(void)
{
	return (test_bit(0x0, &(g_ufs_dsm_adaptor.uic_disable)));
}

int dsm_ufs_update_uic_info(struct ufs_hba *hba, int err_code)
{
	int pos;
	if (!dsm_ufs_enable)
		return -1;
	if (dsm_ufs_if_uic_err_disable())
		return 0;
	if (test_and_set_bit_lock(UFS_UIC_TRANS_ERR, &g_ufs_dsm_adaptor.err_type))
		return -1;
	pos = (hba->ufs_stats.pa_err.pos - 1) % UIC_ERR_REG_HIST_LENGTH;
	g_ufs_dsm_adaptor.uic_uecpa = hba->ufs_stats.pa_err.reg[pos];
	pos = (hba->ufs_stats.dl_err.pos - 1) % UIC_ERR_REG_HIST_LENGTH;
	g_ufs_dsm_adaptor.uic_uecdl = hba->ufs_stats.dl_err.reg[pos];
	pos = (hba->ufs_stats.nl_err.pos - 1) % UIC_ERR_REG_HIST_LENGTH;
	g_ufs_dsm_adaptor.uic_uecn = hba->ufs_stats.nl_err.reg[pos];
	pos = (hba->ufs_stats.tl_err.pos - 1) % UIC_ERR_REG_HIST_LENGTH;
	g_ufs_dsm_adaptor.uic_uect = hba->ufs_stats.tl_err.reg[pos];
	pos = (hba->ufs_stats.dme_err.pos - 1) % UIC_ERR_REG_HIST_LENGTH;
	g_ufs_dsm_adaptor.uic_uedme = hba->ufs_stats.dme_err.reg[pos];
	if (g_ufs_dsm_adaptor.uic_uecpa & UIC_PHY_ADAPTER_LAYER_ERROR_LINE_RESET) {
		if (test_and_set_bit_lock(UFS_LINE_RESET_ERR,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
	}
	return 0;
}

static void update_uic_info(struct ufs_hba *hba)
{
	g_ufs_dsm_adaptor.uic_info[0] = ufshcd_readl(hba, REG_UIC_COMMAND);
	g_ufs_dsm_adaptor.uic_info[UIC_INFO_CMD_INDEX_1] = ufshcd_readl(hba,
		REG_UIC_COMMAND_ARG_1);
	g_ufs_dsm_adaptor.uic_info[UIC_INFO_CMD_INDEX_2] = ufshcd_readl(hba,
		REG_UIC_COMMAND_ARG_2);
	g_ufs_dsm_adaptor.uic_info[UIC_INFO_CMD_INDEX_3] = ufshcd_readl(hba,
		REG_UIC_COMMAND_ARG_3);
}

int dsm_ufs_update_error_info(struct ufs_hba *hba, int code)
{
	if (!dsm_ufs_enable)
		return -1;
	switch (code) {
	case DSM_UFS_VOLT_GPIO_ERR:
		(void)test_and_set_bit_lock(UFS_VOLT_GPIO_ERR,
			&g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_LINKUP_ERR:
		(void)test_and_set_bit_lock(UFS_LINKUP_ERR,
			&g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_UIC_CMD_ERR:
		if (test_and_set_bit_lock(UFS_UIC_CMD_ERR,
			&g_ufs_dsm_adaptor.err_type))
			break;
		update_uic_info(hba);
		break;
	case DSM_UFS_CONTROLLER_ERR:
		(void)test_and_set_bit_lock(UFS_CONTROLLER_ERR,
				&g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_DEV_ERR:
		(void)test_and_set_bit_lock(UFS_DEV_ERR,
				&g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_ENTER_OR_EXIT_H8_ERR:
		(void)test_and_set_bit_lock(UFS_ENTER_OR_EXIT_H8_ERR,
				&g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_LIFETIME_EXCCED_ERR:
		(void)test_and_set_bit_lock(UFS_LIFETIME_EXCCED_ERR,
			&g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_TEMP_LOW_ERR:
		(void)test_and_set_bit_lock(UFS_TEMP_LOW_ERR,
			&g_ufs_dsm_adaptor.err_type);
		g_ufs_dsm_adaptor.temperature = hba->ufs_temp.temp;
		break;
	default:
		return 0;
	}
	return 0;
}

void dsm_ufs_update_lifetime_info(u8 lifetime_a, u8 lifetime_b)
{
	g_ufs_dsm_adaptor.lifetime_a = lifetime_a;
	g_ufs_dsm_adaptor.lifetime_b = lifetime_b;
}

/* put error message into buffer */
#ifdef CONFIG_HISI_BOOTDEVICE
void get_bootdevice_product_name(char *product_name, u32 len);
#endif

static void set_dsm_dev_ic_module_name(const char *product_name, const char *rev)
{
	int ret;
	dsm_ufs.ic_name = product_name;
	dsm_ufs.module_name = rev;
	ret = dsm_update_client_vendor_info(&dsm_ufs);
	if (ret)
		pr_err("update dsm ufs ic_name and module_name failed!\n");
	dsm_ufs.ic_name = NULL;
	dsm_ufs.module_name = NULL;
}

static void update_dsm_log_module_name(struct ufs_hba *hba, char **log_buff,
	int *size)
{
	int ret;
	char product_name[PRODUCT_NAME_LEN] = {0};
	char rev[REV_LEN] = {0};

#ifdef CONFIG_HISI_BOOTDEVICE
	/* depend on hufs bootdevice */
	get_bootdevice_product_name(product_name, PRODUCT_NAME_LEN);
	ret = snprintf(*log_buff, *size, "%s,",
		product_name);
	*log_buff += ret;
	*size -= ret;
#endif
	if ((hba->sdev_ufs_device != NULL)
		&& (hba->sdev_ufs_device->rev != NULL))
		snprintf(rev, REV_LEN, "%.4s,", hba->sdev_ufs_device->rev);
	ret = snprintf(*log_buff, *size, "%s,", rev);
	*log_buff += ret;
	*size -= ret;
	set_dsm_dev_ic_module_name(product_name, rev);
}

static void update_fastboot_pwmode_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_FASTBOOT_PWMODE_ERR))) {
		pr_err("UFS DSM Error! Err Num:%lu err_type:%lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
			"Fastboot Power Mode Change Err\n");
	log_buff += ret;
	*size -= ret;
}

static void update_fastboot_rw_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_FASTBOOT_RW_ERR))) {
		pr_err("UFS DSM Error! Err Num:%lu err_type:%lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size, "Fastboot Read Write Err\n");
	log_buff += ret;
	*size -= ret;
}

static void update_uic_trans_line_reset_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & ((1<<UFS_UIC_TRANS_ERR) |
		(1<<UFS_LINE_RESET_ERR)))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
			"UECPA:%08x,UECDL:%08x,UECN:%08x,UECT:%08x,UEDME:%08x\n",
			g_ufs_dsm_adaptor.uic_uecpa, g_ufs_dsm_adaptor.uic_uecdl,
			g_ufs_dsm_adaptor.uic_uecn,
			g_ufs_dsm_adaptor.uic_uect, g_ufs_dsm_adaptor.uic_uedme);
	log_buff += ret;
	*size -= ret;
}

static void update_uic_cmd_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1<<UFS_UIC_CMD_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %ld\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
			"UIC_CMD:%08x,ARG1:%08x,ARG2:%08x,ARG3:%08x\n",
			g_ufs_dsm_adaptor.uic_info[0],
			g_ufs_dsm_adaptor.uic_info[UIC_INFO_CMD_INDEX_1],
			g_ufs_dsm_adaptor.uic_info[UIC_INFO_CMD_INDEX_2],
			g_ufs_dsm_adaptor.uic_info[UIC_INFO_CMD_INDEX_3]);
	log_buff += ret;
	*size -= ret;
}

static void update_common_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
#ifndef CONFIG_HISI_UFS_HC_CORE_UTR
	int i;
#endif
	if (!(g_ufs_dsm_adaptor.err_type & ((1 << UFS_CONTROLLER_ERR) |
		(1 << UFS_INLINE_CRYPTO_ERR) | (1 << UFS_DEV_ERR) |
		(1 << UFS_ENTER_OR_EXIT_H8_ERR)))) {
		pr_err("UFS DSM Error! Err Num: %lu, %ld\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
#ifndef CONFIG_HISI_UFS_HC_CORE_UTR
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_INLINE_CRYPTO_ERR)) {
		ret = snprintf(log_buff, *size, "outstanding:0x%lx, doorbell: 0x%lx\n",
			g_ufs_dsm_adaptor.ice_outstanding, g_ufs_dsm_adaptor.ice_doorbell);
		log_buff += ret;
		*size -= ret;
		for (i = 0; i < CRYPTO_CONFIG_LEN; i++) {
			if (crypto_conf_index[i]) {
				ret = snprintf(log_buff, *size, "UTP_DESC[%d]: 0x%x\n", i,
					crypto_conf_index[i]);
				log_buff += ret;
				*size -= ret;
				crypto_conf_index[i] = 0;
			}
		}
	}
#endif
	if ((g_ufs_dsm_adaptor.err_type & (1 << UFS_CONTROLLER_ERR))) {
		ret = snprintf(log_buff, *size, "controller error\n");
		log_buff += ret;
		*size -= ret;
	}
	ret = snprintf(log_buff, *size, "UECDL:%08x,UECN:%08x\n",
		g_ufs_dsm_adaptor.uic_uecdl, g_ufs_dsm_adaptor.uic_uecn);
	log_buff += ret;
	*size -= ret;
	ret = snprintf(log_buff, *size, "UECT:%08x,UEDME:%08x\n",
			g_ufs_dsm_adaptor.uic_uect, g_ufs_dsm_adaptor.uic_uedme);
	log_buff += ret;
	*size -= ret;
}

static void update_utp_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_UTP_TRANS_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size, "UTP_OCS_ERR:%d\n", g_ufs_dsm_adaptor.ocs);
	log_buff += ret;
	*size -= ret;
}

static void update_timeout_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret, i;
	if (!(g_ufs_dsm_adaptor.err_type & ((1 << UFS_TIMEOUT_ERR) |
		(1 << UFS_TIMEOUT_SERIOUS)))) {
		pr_err("UFS DSM Error! Err Num: %lu, %ld\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
			"UTP_tag:%02x,doorbell:%lx,outstandings:%lu, "
			"pwrmode:0x%x, tx_gear:%d, rx_gear:%d, scsi_cdb:\n",
			g_ufs_dsm_adaptor.timeout_slot_id,
			g_ufs_dsm_adaptor.tr_doorbell,
			g_ufs_dsm_adaptor.outstanding_reqs,
			g_ufs_dsm_adaptor.pwrmode,
			g_ufs_dsm_adaptor.tx_gear,
			g_ufs_dsm_adaptor.rx_gear);
	log_buff += ret;
	*size -= ret;
	for (i = 0; i < COMMAND_SIZE(g_ufs_dsm_adaptor.timeout_scmd[0]); i++) {
		ret = snprintf(log_buff, *size, "%02x",
			g_ufs_dsm_adaptor.timeout_scmd[i]);
		log_buff += ret;
		*size -= ret;
	}
}

static void update_hardware_common_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret, i;
	if (!(g_ufs_dsm_adaptor.err_type & ((1 << UFS_SCSI_CMD_ERR) |
		(1 << UFS_DEV_INTERNEL_ERR) | (1 << UFS_HARDWARE_ERR)))) {
		pr_err("UFS DSM Error! Err Num: %lu, %ld\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size, "scsi_status:%x scsi_cdb:\n",
		g_ufs_dsm_adaptor.scsi_status);
	log_buff += ret;
	*size -= ret;
	for (i = 0; i < COMMAND_SIZE(g_ufs_dsm_adaptor.timeout_scmd[0]); i++) {
		ret = snprintf(log_buff, *size, "%02x",
			g_ufs_dsm_adaptor.timeout_scmd[i]);
		log_buff += ret;
		*size -= ret;
	}
	ret = snprintf(log_buff, *size,
		"\t\tResponse_code:%02x, Sense_key:%02x, ASC:%02x, ASCQ:%x\n",
		g_ufs_dsm_adaptor.sense_buffer[0],
		g_ufs_dsm_adaptor.sense_buffer[SENSE_KEY_OFFSET],
		g_ufs_dsm_adaptor.sense_buffer[ASC_OFFSET],
		g_ufs_dsm_adaptor.sense_buffer[ASCQ_OFFSET]);
	log_buff += ret;
	*size -= ret;
}

static void update_vplt_gpio_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_VOLT_GPIO_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
			"UFS BUCK has got a falling down Volt\n");
	log_buff += ret;
	*size -= ret;
}

static void update_linkup_err_log(struct ufs_hba *hba, char *log_buff,
	int *size, unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_LINKUP_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
			"UFS Linkup Line not correct, lane_rx:%d, lane_tx:%d\n",
			hba->max_pwr_info.info.lane_rx,
			hba->max_pwr_info.info.lane_tx);
	log_buff += ret;
	*size -= ret;
}

static void update_lifetime_exceed_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_LIFETIME_EXCCED_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size, "life_time_a:%u, life_time_b:%u\n",
			g_ufs_dsm_adaptor.lifetime_a, g_ufs_dsm_adaptor.lifetime_b);
	log_buff += ret;
	*size -= ret;
}

static void update_temp_low_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_TEMP_LOW_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
		"UFS temp too low: %d\n", g_ufs_dsm_adaptor.temperature);
	log_buff += ret;
	*size -= ret;
}

static void update_pwrdown_flush_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_POWER_DOWN_FLUSH_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
		"UFS power down: No flush sent!\n");
	log_buff += ret;
	*size -= ret;
}

static int update_log_buff(struct ufs_hba *hba, char *log_buff,
	int *size, unsigned long code)
{
	switch (code) {
	case DSM_UFS_FASTBOOT_PWMODE_ERR:
		update_fastboot_pwmode_err_log(log_buff, size, code);
		break;
	case DSM_UFS_FASTBOOT_RW_ERR:
		update_fastboot_rw_err_log(log_buff, size, code);
		break;
	case DSM_UFS_UIC_TRANS_ERR:
		/* fall through */
	case DSM_UFS_LINE_RESET_ERR:
		update_uic_trans_line_reset_err_log(log_buff, size, code);
		break;
	case DSM_UFS_UIC_CMD_ERR:
		update_uic_cmd_err_log(log_buff, size, code);
		break;
	case DSM_UFS_CONTROLLER_ERR:
		/* fall through */
	case DSM_UFS_DEV_ERR:
		/* fall through */
	case DSM_UFS_ENTER_OR_EXIT_H8_ERR:
		/* fall through */
	case DSM_UFS_INLINE_CRYPTO_ERR:
		update_common_err_log(log_buff, size, code);
		break;
	case DSM_UFS_UTP_ERR:
		update_utp_err_log(log_buff, size, code);
		break;
	case DSM_UFS_TIMEOUT_ERR:
		/* fall through */
	case DSM_UFS_TIMEOUT_SERIOUS:
		update_timeout_err_log(log_buff, size, code);
		break;
	case DSM_UFS_SCSI_CMD_ERR:
		/* fall through */
	case DSM_UFS_DEV_INTERNEL_ERR:
		/* fall through */
	case DSM_UFS_HARDWARE_ERR:
		update_hardware_common_err_log(log_buff, size, code);
		break;
	case DSM_UFS_VOLT_GPIO_ERR:
		update_vplt_gpio_err_log(log_buff, size, code);
		dev_err(hba->dev, "%s: UFS BUCK got a falling Volt \n", __func__);
		break;
	case DSM_UFS_LINKUP_ERR:
		update_linkup_err_log(hba, log_buff, size, code);
		break;
	case DSM_UFS_LIFETIME_EXCCED_ERR:
		update_lifetime_exceed_err_log(log_buff, size, code);
		break;
	case DSM_UFS_TEMP_LOW_ERR:
		update_temp_low_err_log(log_buff, size, code);
		break;
	case DSM_UFS_POWER_DOWN_FLUSH_ERR:
		update_pwrdown_flush_err_log(log_buff, size, code);
		break;
	default:
		pr_err("unknown error code: %lu\n", code);
		return -1;
	}
	return 0;
}

int dsm_ufs_get_log(struct ufs_hba *hba, unsigned long code, char *err_msg)
{
	int ret, buff_size;
	char *dsm_log_buff = NULL;
	unsigned int j;
	buff_size = sizeof(g_ufs_dsm_log.dsm_log);
	dsm_log_buff = g_ufs_dsm_log.dsm_log;
	if (!dsm_ufs_enable)
		return 0;
	g_ufs_dsm_adaptor.manufacturer_id = hba->manufacturer_id;
	memset(g_ufs_dsm_log.dsm_log, 0, buff_size);

	ret = snprintf(dsm_log_buff, buff_size,
		"0x%04x,",
		g_ufs_dsm_adaptor.manufacturer_id);
	dsm_log_buff += ret;
	buff_size -= ret;
	update_dsm_log_module_name(hba, &dsm_log_buff, &buff_size);

	ret = snprintf(dsm_log_buff, buff_size,
		"%04x,",
		hba->manufacturer_date);
	dsm_log_buff += ret;
	buff_size -= ret;

	for (j = 0; j < strlen(g_ufs_dsm_log.dsm_log); j++)
		g_ufs_dsm_log.dsm_log[j] -= 1;

	ret = snprintf(dsm_log_buff, buff_size,
		"\nError Num:%lu, %s\n",
		code, err_msg);
	dsm_log_buff += ret;
	buff_size -= ret;

	/* print vendor info */
	ret = update_log_buff(hba, dsm_log_buff, &buff_size, code);
	if (ret) {
		pr_err("unknown error code: %lu\n", code);
		return 0;
	}
	ufs_dsm_real_upload_size = sizeof(g_ufs_dsm_log.dsm_log)-buff_size;
	if (buff_size <= 0) {
		ufs_dsm_real_upload_size = sizeof(g_ufs_dsm_log.dsm_log) - 1;
		pr_err("dsm log buff overflow!\n");
	}
	pr_err("dsm_log:\n %s\n", g_ufs_dsm_log.dsm_log);
	pr_err("buff_size = %d\n", buff_size);
	return 1;
}

int dsm_ufs_enabled(void)
{
	return dsm_ufs_enable;
}

/* lint -e648 -e845 */
#if defined(CONFIG_RPMB_UFS)
extern int get_rpmb_key_status(void);
#endif
int dsm_ufs_utp_err_need_report(struct ufs_hba *hba)
{
	int key_status = 0;
	if (unlikely(hba->host->is_emulator))
		return 0;
#if defined(CONFIG_RPMB_UFS)
	key_status = get_rpmb_key_status();
#endif
	if ((key_status == 0) && (g_ufs_dsm_adaptor.ocs == OCS_FATAL_ERROR))
		return 0;
	return 1;
}

void dsm_ufs_enable_volt_irq(struct ufs_hba *hba)
{
	if (hba->volt_irq < 0)
		return;
	else
		enable_irq(hba->volt_irq);
}

void dsm_ufs_disable_volt_irq(struct ufs_hba *hba)
{
	if (hba->volt_irq < 0)
		return;
	else
		disable_irq(hba->volt_irq);
}

int dsm_ufs_uic_cmd_err_need_report(void)
{
	u32 uic_cmd = g_ufs_dsm_adaptor.uic_info[0];
	if ((uic_cmd == UIC_CMD_DME_HIBER_ENTER) ||
		(uic_cmd == UIC_CMD_DME_HIBER_EXIT))
		return 0;
	return 1;
}

void print_uic_err_history(void)
{
	int i;
	for (i = 0; i < UIC_ERR_HISTORY_LEN; i++)
		pr_info("transfered_bits[%d] = %lu (10^10)\n", i,
		uic_err_history.transfered_bits[i]);
	pr_info("Pos: %u, delta_err_cnt: %lu, delta_bit_cnt: %lu\n",
		uic_err_history.pos,
		uic_err_history.delta_err_cnt,
		uic_err_history.delta_bit_cnt);
}

int dsm_ufs_uic_err_need_report(struct ufs_hba *hba)
{
	u32 ov_flag = 0;
	u32 counter = 0;
	static unsigned long uic_err_cnt = 0;
	static unsigned long tmp_bits = 0;
	static unsigned long ten_billon_bits_cnt = 0;

	uic_err_cnt++;
	ufshcd_dme_get(hba, UIC_ARG_MIB((u32)VS_DEBUG_COUNTER_OVERFLOW), &ov_flag); /* get ov flag */
	/* get synbol cnt /1024 */
	ufshcd_dme_get(hba, UIC_ARG_MIB((u32)VS_DEBUG_COUNTER0), &counter);
	/* reset cnter0 and enable it */
#ifdef CONFIG_HISI_UFS_HC
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)DEBUGCOUNTER_CLR), BIT_DBG_CNT0_CLR);
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)DEBUGCOUNTER_EN), BIT_DBG_CNT0_EN);
#else
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)VS_DEBUG_COUNTER_CONTROL), 0x5);
#endif
	if (ov_flag & 0x1)
		tmp_bits += (1UL << SHIFT_32) * BITS_PER_CNT;
	tmp_bits += (unsigned long)counter * BITS_PER_CNT;
	ten_billon_bits_cnt +=  tmp_bits / TEN_BILLION_BITS;
	tmp_bits = tmp_bits % TEN_BILLION_BITS;
	uic_err_history.transfered_bits[uic_err_history.pos] = ten_billon_bits_cnt;
	uic_err_history.pos = (uic_err_history.pos + 1) % UIC_ERR_HISTORY_LEN;
	uic_err_history.delta_err_cnt = min(uic_err_cnt,
		(unsigned long)UIC_ERR_HISTORY_LEN);
	uic_err_history.delta_bit_cnt = ten_billon_bits_cnt -
		uic_err_history.transfered_bits[uic_err_history.pos];
	print_uic_err_history();
	pr_info("%s: errcnt = %lu, 10^10_cnt = %lu, tmpbits = %lu, "
		"ovflag = %u, counter = %u\n", __func__, uic_err_cnt,
		ten_billon_bits_cnt, tmp_bits, ov_flag & 0x1, counter);
	if ((ten_billon_bits_cnt >= MIN_BER_SAMPLE_BITS) &&
		(uic_err_history.delta_err_cnt > uic_err_history.delta_bit_cnt))
		return 1;
	pr_info("%s: UECPA:%08x,UECDL:%08x,UECN:%08x,UECT:%08x,UEDME:%08x\n",
		__func__, g_ufs_dsm_adaptor.uic_uecpa,
		g_ufs_dsm_adaptor.uic_uecdl, g_ufs_dsm_adaptor.uic_uecn,
		g_ufs_dsm_adaptor.uic_uect, g_ufs_dsm_adaptor.uic_uedme);
	pr_info("%s: UIC error cnt below threshold, do not report!\n", __func__);
	return 0;
}

void report_controller_fatal_error(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.uic_uecdl & UIC_DATA_LINK_LAYER_ERROR_PA_INIT)
		dsm_ufs_log(hba, DSM_UFS_CONTROLLER_ERR, "UFS PA INIT ERROR");
	else
		dsm_ufs_log(hba, DSM_UFS_CONTROLLER_ERR, "UFS Controller Error");
}

static void dsm_ufs_err_handle(struct dsm_err_type_category ufs_err,
	struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (unsigned long)(unsigned)(1 << ufs_err.err_bit)) {
		dsm_ufs_log(hba, ufs_err.err_no, ufs_err.err_desc); /*lint !e592*/
		clear_bit_unlock(ufs_err.err_bit, &g_ufs_dsm_adaptor.err_type);
	}
}

static void dsm_uic_trans_err_handle(struct ufs_hba *hba)
{
	unsigned long flags;
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_UIC_TRANS_ERR)) {
		if (dsm_ufs_uic_err_need_report(hba))
			dsm_ufs_log(hba, DSM_UFS_UIC_TRANS_ERR,
				"UIC No Fatal Error");
		clear_bit_unlock(UFS_UIC_TRANS_ERR, &g_ufs_dsm_adaptor.err_type);
		msleep(DELAY_MS_100);
		spin_lock_irqsave(hba->host->host_lock, flags);
		ufshcd_enable_intr(hba, UIC_ERROR);
		spin_unlock_irqrestore(hba->host->host_lock, flags);
	}
}

static void dsm_timeout_err_handle(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_TIMEOUT_ERR)) {
		ufshcd_dme_get(hba, UIC_ARG_MIB(PA_PWRMODE),
			&g_ufs_dsm_adaptor.pwrmode);
		ufshcd_dme_get(hba, UIC_ARG_MIB(PA_TXGEAR),
			&g_ufs_dsm_adaptor.tx_gear);
		ufshcd_dme_get(hba, UIC_ARG_MIB(PA_RXGEAR),
			&g_ufs_dsm_adaptor.rx_gear);
		dsm_ufs_log(hba, DSM_UFS_TIMEOUT_ERR, "UFS TimeOut Error");
		clear_bit_unlock(UFS_TIMEOUT_ERR, &g_ufs_dsm_adaptor.err_type);
	}
}

static void dsm_uic_cmd_err_handle(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_UIC_CMD_ERR)) {
		if (dsm_ufs_uic_cmd_err_need_report())
			dsm_ufs_log(hba, DSM_UFS_UIC_CMD_ERR, "UIC Cmd Error");
		clear_bit_unlock(UFS_UIC_CMD_ERR, &g_ufs_dsm_adaptor.err_type);
	}
}

static void dsm_utp_trans_err_handle(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_UTP_TRANS_ERR)) {
		if (dsm_ufs_utp_err_need_report(hba))
			dsm_ufs_log(hba, DSM_UFS_UTP_ERR, "UTP Transfer Error");
		clear_bit_unlock(UFS_UTP_TRANS_ERR, &g_ufs_dsm_adaptor.err_type);
	}
}

static void dsm_controller_err_handle(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_CONTROLLER_ERR)) {
		report_controller_fatal_error(hba);
		clear_bit_unlock(UFS_CONTROLLER_ERR, &g_ufs_dsm_adaptor.err_type);
	}
}

static void dsm_pwrdownflush_err_handle(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_POWER_DOWN_FLUSH_ERR)) {
		dsm_ufs_log(hba, DSM_UFS_POWER_DOWN_FLUSH_ERR, "Powerdown Flush Error");
		clear_bit_unlock(UFS_POWER_DOWN_FLUSH_ERR, &g_ufs_dsm_adaptor.err_type);
	}
}

void dsm_ufs_handle_work(struct work_struct *work)
{
	struct ufs_hba *hba;
	int len, i;
	hba = container_of(work, struct ufs_hba, dsm_work);
	mutex_lock(&hba->eh_mutex);
	len = sizeof(err_type_category) / sizeof(err_type_category[0]);
	for (i = 0; i < len; i++) {
		switch (err_type_category[i].err_bit) {
		case UFS_UIC_TRANS_ERR:
			dsm_uic_trans_err_handle(hba);
			break;
		case UFS_UIC_CMD_ERR:
			dsm_uic_cmd_err_handle(hba);
			break;
		case UFS_CONTROLLER_ERR:
			dsm_controller_err_handle(hba);
			break;
		case UFS_UTP_TRANS_ERR:
			dsm_utp_trans_err_handle(hba);
			break;
		case UFS_TIMEOUT_ERR:
			dsm_timeout_err_handle(hba);
			break;
		case UFS_POWER_DOWN_FLUSH_ERR:
			dsm_pwrdownflush_err_handle(hba);
			break;
		default:
			dsm_ufs_err_handle(err_type_category[i], hba);
			break;
		}
	}
	mutex_unlock(&hba->eh_mutex);
}

void schedule_ufs_dsm_work(struct ufs_hba *hba)
{
	if ((dsm_ufs_enabled()) && (dsm_ufs_if_err()))
		queue_work(system_freezable_wq, &hba->dsm_work);
}

static irqreturn_t ufs_gpio_irq_handle(int irq, void *__hba)
{
	struct ufs_hba *hba = __hba;
	dsm_ufs_update_error_info(hba, DSM_UFS_VOLT_GPIO_ERR);
	return IRQ_HANDLED;
}

static int ufs_volt_gpio_init(struct device *dev)
{
	int err;
	int irq;
	struct device_node *node =  dev->of_node;
	unsigned int gpio;
	gpio = of_get_named_gpio(node, "volt-irq,ufs-gpios", 0);
	if (!gpio_is_valid(gpio)) {
		pr_err("gpio is unvalid!\n");
		return -ENOENT;
	}
	err = gpio_request(gpio, "ufs_volt");
	if (err < 0) {
		pr_info("Can`t request UFS_VOLT gpio %d\n", gpio);
		return -EIO;
	}
	err = gpio_direction_input(gpio);
	if (err < 0)
		goto out_free;
	irq = gpio_to_irq(gpio);
	if (irq < 0) {
		pr_info("Could not set UFS_VOLT_GPIO irq = %d!, err = %d\n",
				gpio, irq);
		goto out_free;
	}
	err = request_irq((unsigned int)irq,
		ufs_gpio_irq_handle, (unsigned long)IRQF_TRIGGER_FALLING,
		"ufs_volt", NULL);
	if (err) {
		free_irq(gpio, NULL);
		pr_info("Request ufs volt gpio irq failed.\n");
		goto out_free;
	}
	return irq;
out_free:
	gpio_free(gpio);
	return -EIO;
}

void dsm_ufs_init(struct ufs_hba *hba)
{
	int ufs_volt_irq;
	struct device *dev = hba->dev;
	memset((void*)&uic_err_history, 0, sizeof(uic_err_history));
	ufs_volt_irq = ufs_volt_gpio_init(dev);
	if (ufs_volt_irq < 0) {
		hba->volt_irq = -1;
		pr_err("ufs volt gpio irq failed.\n");
	} else {
		hba->volt_irq = ufs_volt_irq;
	}
	ufs_dclient = dsm_register_client(&dsm_ufs);
	if (!ufs_dclient) {
		pr_err("ufs dclient init error");
		return;
	}
	INIT_WORK(&hba->dsm_work, dsm_ufs_handle_work);
	mutex_init(&g_ufs_dsm_log.lock);
	dsm_ufs_enable = 1;
	async_schedule(dsm_ufs_fastboot_async, hba);
}
/* lint +e747 +e713 +e715 +e732 +e835 */
