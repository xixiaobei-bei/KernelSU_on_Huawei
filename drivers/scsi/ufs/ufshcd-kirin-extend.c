/*
 * ufshcd-kirin-extend.c
 *
 * The hufs extend implements for ufshcd.c
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
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

#include "ufshcd-kirin-extend.h"

#include <linux/of.h>
#include <scsi/ufs/ioctl.h>
#include <linux/bootdevice.h>
#ifdef CONFIG_HUAWEI_STORAGE_ROFA
#include <chipset_common/storage_rofa/storage_rofa.h>
#endif

#include "ufshcd.h"
#include "ufs-quirks.h"
#include "ufs-vendor-cmd.h"
#include "ufs-vendor-mode.h"
#ifdef CONFIG_SCSI_UFS_UNISTORE
#include "ufs_unistore.h"
#endif

#define SERIAL_NUM_SIZE 12
#define UFS_CID_SIZE 4
#define BOARDID_SIZE 4
#define MAX_OLD_PRDCT_NAME_NR 13

static const char old_product_name[MAX_OLD_PRDCT_NAME_NR][MAX_MODEL_LEN] = {
	"KLUBG4G1CE-B0B1", /* Samsung  UFS2.0 32GB */
	"KLUCG4J1CB-B0B1", /* Samsung  UFS2.0 64GB */
	"KLUDG8J1CB-B0B1", /* Samsung  UFS2.0 128GB */
	"KLUCG4J1EB-B0B1", /* Samsung  UFS2.0 64GB */
	"KLUDG8J1EB-B0B1", /* Samsung  UFS2.0 128GB */
	"KLUEG8U1EM-B0B1", /* Samsung  UFS2.0 256GB */
	"KLUEG8U1EM-B0B1", /* Samsung  UFS2.1 256GB */
	"KLUCG4J1ED-B0C1", /* Samsung  UFS2.1 64GB */
	"KLUDG8V1EE-B0C1", /* Samsung  UFS2.1 128GB */
	"KLUEG8U1EM-B0C1", /* Samsung  UFS2.1 256GB */

	"KLUCG2K1EA-B0C1", /* Samsung  UFS2.1 64GB  V4 */
	"KLUDG4U1EA-B0C1", /* Samsung  UFS2.1 128GB V4 */
	"KLUEG8U1EA-B0C1"  /* Samsung  UFS2.1 256GB V4 */
};

static void get_boardid(unsigned int *boardid, int boardid_size)
{
	int ret;
	struct device_node *root = of_find_node_by_path("/");

	if (!root) {
		pr_err("Failed : of_find_node_by_path\n");
		goto err_get_bid;
	}

	ret = of_property_read_u32_array(root, "hisi,boardid", boardid,
					 boardid_size);
	if (ret < 0) {
		pr_err("Failed : of_property_read_u32 %d\n", ret);
		goto err_get_bid;
	}
	pr_info("Board ID.(%x %x %x %x)\n", boardid[0], boardid[1], boardid[2],
		boardid[3]);
	return;

err_get_bid:
	boardid[0] = 0;
	boardid[1] = 0;
	boardid[2] = 0;
	boardid[3] = 0;
	return;
}

/*
 * Distinguish between samsung old and new product.
 * Return 0 if the product is new, 1 if the product is old.
 * For a old product, samsung has 12 byte long of serial number ready to use;
 * for a new product, samsung has 24 byte long of serial number in unicode,
 * which should be transfered to 12 byte long in ascii.
 */
static int is_old_product_name(const char *product_name)
{
	int i;
	unsigned int boardid[BOARDID_SIZE] = { 0 };

	for (i = 0; i < MAX_OLD_PRDCT_NAME_NR; i++) {
		if (strncmp(product_name, old_product_name[i],
			     strlen(old_product_name[i])))
			continue;
		/*
		 * Samsung v4 that has some specific board id should be regard
		 * as old product
		 */
		if (product_name[9] != 'A')
			return 1;
		get_boardid(boardid, BOARDID_SIZE);
		/* for board_id == 6453, keep old uid for workaround */
		if ((boardid[0] == 6 && boardid[1] == 4 && boardid[2] == 5 &&
		     boardid[3] == 3) ||
		    /* for board_id == 6455, keep old uid for workaround */
		    (boardid[0] == 6 && boardid[1] == 4 && boardid[2] == 5 &&
		     boardid[3] == 5) ||
		    /* for board_id == 6908, keep old uid for workaround */
		    (boardid[0] == 6 && boardid[1] == 9 && boardid[2] == 0 &&
		     boardid[3] == 8) ||
		    /* for board_id == 6913, keep old uid for workaround */
		    (boardid[0] == 6 && boardid[1] == 9 && boardid[2] == 1 &&
		     boardid[3] == 3))
			return 1;
		else
			return 0;
	}
	return 0;
}

static void ufs_set_sec_unique_number(struct ufs_hba *hba,
				      const uint8_t *desc_buf,
				      const char *product_name)
{
	int i, idx;
	uint8_t snum_buf[SERIAL_NUM_SIZE + 1];

	memset(&hba->unique_number, 0, sizeof(hba->unique_number));
	memset(snum_buf, 0, sizeof(snum_buf));

	switch (hba->manufacturer_id) {
	case UFS_VENDOR_SAMSUNG:
		if (is_old_product_name(product_name)) {
			/* Samsung has 12Byte long of serail number, just copy it */
			memcpy(snum_buf, desc_buf + QUERY_DESC_HDR_SIZE,
			       SERIAL_NUM_SIZE);
		} else {
			/*
			 * Samsung V4 UFS need 24 Bytes for serial number,
			 * transfer unicode to 12 bytes the magic number 12
			 * here was following original below HYNIX/TOSHIBA
			 * decoding method.
			 */
			for (i = 0; i < SERIAL_NUM_SIZE; i++) {
				idx = QUERY_DESC_HDR_SIZE + i * 2 + 1;
				snum_buf[i] = desc_buf[idx];
			}
		}
		break;
	case UFS_VENDOR_SKHYNIX:
		/* hynix only have 6Byte, add a 0x00 before every byte */
		for (i = 0; i < 6; i++) {
			snum_buf[i * 2] = 0x0;
			snum_buf[i * 2 + 1] = desc_buf[QUERY_DESC_HDR_SIZE + i];
		}
		break;
	case UFS_VENDOR_TOSHIBA:
		/*
		 * toshiba: 20Byte, every two byte has a prefix of 0x00, skip
		 * and add two 0x00 to the end
		 */
		for (i = 0; i < 10; i++) {
			idx = QUERY_DESC_HDR_SIZE + i * 2 + 1;
			snum_buf[i] = desc_buf[idx];
		}
		break;
	case UFS_VENDOR_HI1861:
		memcpy(snum_buf, desc_buf + QUERY_DESC_HDR_SIZE, SERIAL_NUM_SIZE);
		break;
	case UFS_VENDOR_MICRON:
		/* Micron need 4 bytes */
		memcpy(snum_buf, desc_buf + QUERY_DESC_HDR_SIZE, 4);
		break;
	case UFS_VENDOR_SANDISK:
		for (i = 0; i < SERIAL_NUM_SIZE; i++) {
			idx = QUERY_DESC_HDR_SIZE + i * 2 + 1;
			snum_buf[i] = desc_buf[idx];
		}
		break;
	default:
		dev_err(hba->dev, "unknown ufs manufacturer id\n");
		break;
	}

	hba->unique_number.manufacturer_id = hba->manufacturer_id;
	hba->unique_number.manufacturer_date = hba->manufacturer_date;
	memcpy(hba->unique_number.serial_number, snum_buf, SERIAL_NUM_SIZE);
}

static void ufs_set_bootdevcie_info(struct ufs_hba *hba)
{
#ifdef CONFIG_HISI_BOOTDEVICE
	u32 cid[UFS_CID_SIZE];
	int i;

	if (get_bootdevice_type() == BOOT_DEVICE_UFS) {
		memcpy(cid, (u32 *)&hba->unique_number, sizeof(cid));
		for (i = 0; i < UFS_CID_SIZE - 1; i++)
			cid[i] = be32_to_cpu(cid[i]);

		cid[UFS_CID_SIZE - 1] =
			((cid[UFS_CID_SIZE - 1] & 0xffff) << 16) |
			((cid[UFS_CID_SIZE - 1] >> 16) & 0xffff);
		set_bootdevice_cid((u32 *)cid);
		set_bootdevice_product_name(hba->model);
		set_bootdevice_manfid(hba->manufacturer_id);
	}

#ifdef CONFIG_HUAWEI_STORAGE_ROFA
	if (get_bootdevice_type() == BOOT_DEVICE_UFS) {
		storage_rochk_record_bootdevice_type(1); /* set ufs type */
		storage_rochk_record_bootdevice_manfid(hba->manufacturer_id);
		storage_rochk_record_bootdevice_model(hba->model);
	}
#endif /* CONFIG_HUAWEI_STORAGE_ROFA */
#endif /* CONFIG_HISI_BOOTDEVICE */
}

int hufs_get_device_info(struct ufs_hba *hba,
			     struct ufs_dev_desc *dev_desc, u8 *desc_buf)
{
	int err;

	strlcpy(hba->model, dev_desc->model, sizeof(dev_desc->model));
	memset(desc_buf, 0, QUERY_DESC_MAX_SIZE);

	/* spec is unicode but sec use hex data */
	err = ufshcd_read_string_desc(hba, dev_desc->serial_num_index, desc_buf,
				      QUERY_DESC_STRING_MAX_SIZE, UTF16_STD);
	if (err) {
		pr_err("%s err %d, serial_num_index %d\n", __func__, err,
		       dev_desc->serial_num_index);
		return err;
	}

	desc_buf[QUERY_DESC_MAX_SIZE] = '\0';
	ufs_set_sec_unique_number(hba, desc_buf, dev_desc->model);

	ufshcd_get_vendor_info(hba, dev_desc);
	ufs_set_bootdevcie_info(hba);

	return 0;
}

u64 read_utr_doorbell(struct ufs_hba *hba)
{
	u64 tr_doobell;
#ifdef CONFIG_HISI_UFS_HC_CORE_UTR
	tr_doobell = read_core_utr_doorbells(hba);
#else
	tr_doobell = ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_DOOR_BELL);
#endif
	return tr_doobell;
}

static int ufshcd_ioctl_query_read_desc(struct ufs_hba *hba,
					struct ufs_ioctl_query_data *ioctl_data,
					void __user *buffer)
{
	int err;
	u8 index = 0;
	u8 *desc_buf = NULL;
	u8 idn = ioctl_data->idn;
	int desc_len = UFS_DESC_SIZE_MAX;

	desc_buf = kzalloc(desc_len, GFP_KERNEL);
	if (!desc_buf) {
		dev_err(hba->dev, "%s: alloc fail\n", __func__);
		return -ENOMEM;
	}

	switch (idn) {
	case QUERY_DESC_IDN_DEVICE:
	case QUERY_DESC_IDN_HEALTH:
		break;
	default:
		err = -EINVAL;
		goto out;
	}

	err = ufshcd_query_descriptor_retry(hba, UPIU_QUERY_OPCODE_READ_DESC,
					    idn, index, 0, desc_buf, &desc_len);
	if (err) {
		dev_err(hba->dev, "%s: read desc err %d\n", __func__, err);
		goto out;
	}

	ioctl_data->buf_size =(u16)(desc_len < (int)UFS_DESC_SIZE_MAX ?
				    desc_len : UFS_DESC_SIZE_MAX);
	err = copy_to_user(buffer + UFS_IOCTL_QUERY_DATA_SIZE, desc_buf,
			   ioctl_data->buf_size);
	if (err)
		dev_err(hba->dev, "%s: err %d copying back to user.\n",
			__func__, err);
out:
	kfree(desc_buf);
	return err;
}

static int ufshcd_ioctl_query_read_attr(struct ufs_hba *hba,
					struct ufs_ioctl_query_data *ioctl_data,
					void __user *buffer)
{
	int err;
	u8 index = 0;
	u32 att = 0;
	u8 idn = ioctl_data->idn;
	u32 opcode = ioctl_data->opcode;

	switch (idn) {
	case QUERY_ATTR_IDN_PURGE_STATUS:
		break;
	default:
		return -EINVAL;
	}

	err = ufshcd_query_attr(hba, (enum query_opcode)opcode,
				(enum attr_idn)idn, index, 0, &att);
	if (err) {
		dev_err(hba->dev, "%s: read desc err %d\n", __func__, err);
		return err;
	}

	ioctl_data->buf_size = sizeof(u32);
	err = copy_to_user(buffer + UFS_IOCTL_QUERY_DATA_SIZE, &att,
			   ioctl_data->buf_size);
	if (err)
		dev_err(hba->dev, "%s: copy back to user err %d\n", __func__,
			err);
	return err;
}

static int ufshcd_ioctl_query_flag(struct ufs_hba *hba,
				   struct ufs_ioctl_query_data *ioctl_data,
				   const void __user *buffer)
{
	int err;
	bool flag = 0;
	u8 idn = ioctl_data->idn;
	u32 opcode = ioctl_data->opcode;

	err = copy_from_user(&flag, buffer + UFS_IOCTL_QUERY_DATA_SIZE,
			     sizeof(bool));
	if (err) {
		dev_err(hba->dev,
			"%s: Failed copying buffer from user, err %d\n",
			__func__, err);
		return err;
	}

	switch (idn) {
	case QUERY_FLAG_IDN_PURGE_ENABLE:
		break;
	default:
		return -EINVAL;
	}

	err = ufshcd_query_flag_retry(hba, (enum query_opcode)opcode,
				      (enum flag_idn)idn, &flag);
	if (err)
		dev_err(hba->dev, "%s: query flag err %d\n", __func__, err);

	return err;
}

/*
 * ufshcd_query_ioctl - perform user read queries
 * @hba: per-adapter instance
 * @lun: used for lun specific queries
 * @buffer: user space buffer for reading and submitting query data and params
 * @return: 0 for success negative error code otherwise
 *
 * Expected/Submitted buffer structure is struct ufs_ioctl_query_data.
 * It will read the opcode, idn and buf_length parameters, and, put the
 * response in the buffer field while updating the used size in buf_length.
 */
static int ufshcd_ioctl_query(struct ufs_hba *hba, u8 lun, void __user *buffer)
{
	int err;
	struct ufs_ioctl_query_data *ioctl_data = NULL;
	bool copy_back = false;

	ioctl_data = kzalloc(UFS_IOCTL_QUERY_DATA_SIZE, GFP_KERNEL);
	if (!ioctl_data) {
		dev_err(hba->dev, "%s: alloc fail\n", __func__);
		return -ENOMEM;
	}

	/* extract params from user buffer */
	err = copy_from_user(ioctl_data, buffer, UFS_IOCTL_QUERY_DATA_SIZE);
	if (err) {
		dev_err(hba->dev, "%s: copy from user err %d\n", __func__, err);
		goto out;
	}

	switch (ioctl_data->opcode) {
	case UPIU_QUERY_OPCODE_READ_DESC:
		copy_back = true;
		err = ufshcd_ioctl_query_read_desc(hba, ioctl_data, buffer);
		break;
	case UPIU_QUERY_OPCODE_READ_ATTR:
		copy_back = true;
		err = ufshcd_ioctl_query_read_attr(hba, ioctl_data, buffer);
		break;
	case UPIU_QUERY_OPCODE_CLEAR_FLAG:
	case UPIU_QUERY_OPCODE_SET_FLAG:
		err = ufshcd_ioctl_query_flag(hba, ioctl_data, buffer);
		break;
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
	case UPIU_QUERY_OPCODE_TZ_CTRL:
	case UPIU_QUERY_OPCODE_VENDOR_WRITE:
		err = ufshcd_ioctl_query_vcmd(hba, ioctl_data, buffer);
		break;
	case UPIU_QUERY_OPCODE_READ_TZ_DESC:
	case UPIU_QUERY_OPCODE_READ_HI1861_FSR:
		copy_back = true;
		err = ufshcd_ioctl_query_vcmd(hba, ioctl_data, buffer);
		break;
#endif
	default:
		err = -EINVAL;
		break;
	}

	if (err) {
		dev_err(hba->dev, "%s: Query opcode 0x%x idn %d, err %d\n",
			__func__, ioctl_data->opcode, ioctl_data->idn, err);
		goto out;
	}

	/* copy to user */
	if (copy_back == false)
		goto out;
	err = copy_to_user(buffer, ioctl_data, UFS_IOCTL_QUERY_DATA_SIZE);
	if (err)
		dev_err(hba->dev, "%s: copy to user err %d\n", __func__, err);
out:
	kfree(ioctl_data);
	return err;
}

/*
 * ufshcd_ioctl - ufs ioctl callback registered in scsi_host
 * @dev: scsi device required for per LUN queries
 * @cmd: command opcode
 * @buffer: user space buffer for transferring data
 *
 * Supported commands:
 * UFS_IOCTL_QUERY
 */
#ifdef CONFIG_HUAWEI_UFS_VENDOR_MODE
static struct ufs_ioctl_vendor_state_t ufs_ioctl_vendor_state;
#endif
static int ufshcd_ioctl(struct scsi_device *dev, int cmd, void __user *buffer)
{
	int err;
	struct ufs_hba *hba = NULL;

	if (!dev || !buffer) {
		pr_err("%s: sdev or buffer is NULL!\n", __func__);
		return -EINVAL;
	}

	hba = shost_priv(dev->host);
	if (!hba) {
		pr_err("%s: hba is NULL!\n", __func__);
		return -EINVAL;
	}

	pm_runtime_get_sync(hba->dev);

	switch (cmd) {
	case UFS_IOCTL_QUERY:
		err = ufshcd_ioctl_query(hba, ufshcd_scsi_to_upiu_lun(dev->lun),
					 buffer);
		break;
	case UFS_IOCTL_BLKROSET:
		err = -ENOIOCTLCMD;
		break;
#ifdef CONFIG_HUAWEI_UFS_VENDOR_MODE
	case UFS_IOCTL_VENDOR_PACKAGE:
		err = ufs_ioctl_vendor_package(hba, &ufs_ioctl_vendor_state,
					       buffer);
		break;
	case UFS_IOCTL_VENDOR_PACKAGE_TICK:
		err = ufs_ioctl_vendor_package_tick(
			hba, dev, &ufs_ioctl_vendor_state, buffer);
		break;
#endif
	default:
		err = -ENOIOCTLCMD;
		break;
	}

	pm_runtime_put_sync(hba->dev);
	return err;
}


void ufshcd_host_ops_register(struct ufs_hba *hba)
{
	struct scsi_host_template *sht = hba->host->hostt;

	if (!sht)
		return;

	sht->ioctl = ufshcd_ioctl,
#ifdef CONFIG_COMPAT
	sht->compat_ioctl = ufshcd_ioctl,
#endif
	ufshcd_tz_op_register(hba);
#ifdef CONFIG_SCSI_UFS_UNISTORE
	ufshcd_unistore_op_register(hba);
#endif
}
