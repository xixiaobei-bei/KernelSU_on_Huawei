/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2010-2020. All rights reserved.
 * Description: partition table
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "partition: " fmt

#include <linux/bootdevice.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/hisi/partition_macro.h>
#include <linux/hisi/partition_ap_kernel.h>
#include <linux/hisi/partition_def.h>
#include <linux/hisi/partition.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#define A_POSTFIX "_a"
#define B_POSTFIX "_b"

#ifdef CONFIG_PARTITION_PATH_BY_LABEL
#define BLKDEV_PATH_PREFIX "/dev/disk/by-partlabel/"
#else
#define BLKDEV_PATH_PREFIX "/dev/block/by-name/"
#endif

enum AB_PARTITION_TYPE ufs_boot_partition_type = XLOADER_A;

static const struct partition *get_ptable_src(void)
{
	enum bootdevice_type dev_type;

	dev_type = get_bootdevice_type();
	if (dev_type == BOOT_DEVICE_EMMC)
		return partition_table_emmc;
	else if (dev_type == BOOT_DEVICE_UFS)
		return partition_table_ufs;

	return NULL;
}

static u32 get_ptable_size(void)
{
	enum bootdevice_type dev_type;

	dev_type = get_bootdevice_type();
	if (dev_type == BOOT_DEVICE_EMMC)
		return ARRAY_SIZE(partition_table_emmc);
	else if (dev_type == BOOT_DEVICE_UFS)
		return ARRAY_SIZE(partition_table_ufs);

	return 0;
}

static int transform_ptn_name(const char *part_nm, char *part_nm_tmp,
			      unsigned int nm_tmp_len)
{
	enum bootdevice_type dev_type;
	unsigned int nm_len;

	nm_len = strlen(part_nm);
	if (nm_len + sizeof(A_POSTFIX) > nm_tmp_len) {
		pr_err("%s:%d Invalid input part_nm\n", __func__, __LINE__);
		return PTN_ERROR;
	}

	strncpy(part_nm_tmp, part_nm, nm_len);
	dev_type = get_bootdevice_type();
	if (dev_type == BOOT_DEVICE_EMMC) {
		strncpy(part_nm_tmp + nm_len, A_POSTFIX, sizeof(A_POSTFIX));
	} else if (dev_type == BOOT_DEVICE_UFS) {
		if (ufs_boot_partition_type == XLOADER_B)
			strncpy(part_nm_tmp + nm_len, B_POSTFIX,
				sizeof(B_POSTFIX));
		else
			strncpy(part_nm_tmp + nm_len, A_POSTFIX,
				sizeof(A_POSTFIX));
	} else {
		part_nm_tmp[nm_len] = '\0';
	}

	return 0;
}

/*
 * Description: Get block device path by partition name.
 * Input: part_nm, bdev_path buffer length
 * Output: bdev_path, block device path, such as:/dev/block/by-name/xxx
 */
int flash_find_ptn_s(const char *part_nm, char *bdev_path, unsigned int len)
{
	u32 n;
	const struct partition *ptable_src = NULL;
	char part_nm_tmp[MAX_PARTITION_NAME_LENGTH] = { 0 };
	u32 ptn_num;

	if (!bdev_path || !part_nm || !strlen(part_nm)) {
		pr_err("%s:%d Invalid input\n", __func__, __LINE__);
		return PTN_ERROR;
	}

	if (len < (strlen(part_nm) + sizeof(BLKDEV_PATH_PREFIX))) {
		pr_err("%s:%d len too short\n", __func__, __LINE__);
		return PTN_ERROR;
	}

	ptable_src = get_ptable_src();
	if (!ptable_src) {
		pr_err("%s: Failed to get ptable src!\n", __func__);
		return PTN_ERROR;
	}

	ptn_num = get_ptable_size();
	for (n = 0; n < ptn_num; n++) {
		if (!strcmp((ptable_src + n)->name, part_nm)) {
			strncpy(bdev_path, BLKDEV_PATH_PREFIX,
				strlen(BLKDEV_PATH_PREFIX));
			strncpy(bdev_path + strlen(BLKDEV_PATH_PREFIX), part_nm,
				strlen(part_nm) + 1);
			return 0;
		}
	}

	if (transform_ptn_name(part_nm, part_nm_tmp, sizeof(part_nm_tmp))) {
		pr_err("%s:%d transform ptn name fail!\n", __func__, __LINE__);
		return PTN_ERROR;
	}

	for (n = 0; n < ptn_num; n++) {
		if (!strcmp((ptable_src + n)->name, part_nm_tmp)) {
			strncpy(bdev_path, BLKDEV_PATH_PREFIX,
				strlen(BLKDEV_PATH_PREFIX));
			strncpy(bdev_path + strlen(BLKDEV_PATH_PREFIX),
				part_nm_tmp, strlen(part_nm_tmp) + 1);
			return 0;
		}
	}

	pr_err("%s: partition not found, part_nm = %s\n", __func__, part_nm);
	return PTN_ERROR;
}
EXPORT_SYMBOL(flash_find_ptn_s);

/*
 * Function: deprecated interface, get device path by partition name
 * Description: It is up to each module to decide whether to use the new secure
 *              interface "flash_find_ptn_s" or not.
 *              The calling module is responsible for using this deprecated
 *              interface.
 * Input : ptn_name, partition name
 * Output: bdev_path, boot device path, such as:/dev/block/by-name/xxx
 */
int flash_find_ptn(const char *ptn_name, char *bdev_path)
{
	int ret;
	unsigned int len;

	if (!ptn_name || !bdev_path) {
		pr_err("%s:%d input buffer is NULL!\n", __func__, __LINE__);
		return PTN_ERROR;
	}

	len = strlen(ptn_name) + sizeof(BLKDEV_PATH_PREFIX);
	ret = flash_find_ptn_s(ptn_name, bdev_path, len);
	if (ret) {
		pr_err("%s:%d ptn_name find error!\n", __func__, __LINE__);
		return PTN_ERROR;
	}

	return 0;
}
EXPORT_SYMBOL(flash_find_ptn);

/*
 * Get partition offset in total partitions(all lu), not the current LU
 */
int flash_get_ptn_index(const char *bdev_path)
{
	int n;
	const struct partition *ptable_src = NULL;
	char part_nm_tmp[MAX_PARTITION_NAME_LENGTH] = {0};
	u32 ptn_num;

	if (!bdev_path) {
		pr_err("%s: Input partition name is NULL!\n", __func__);
		return PTN_ERROR;
	}

	ptable_src = get_ptable_src();
	if (!ptable_src) {
		pr_err("%s: Failed to get ptable src!\n", __func__);
		return PTN_ERROR;
	}

	ptn_num = get_ptable_size();

	if (!strcmp(PART_XLOADER, bdev_path)) {
		pr_err("%s: This is boot partition\n", __func__);
		return PTN_ERROR;
	}

	/* normal partition */
	for (n = 0; n < ptn_num; n++)
		if (!strcmp((ptable_src + n)->name, bdev_path))
			return n;

	if (transform_ptn_name(bdev_path, part_nm_tmp, sizeof(part_nm_tmp))) {
		pr_err("%s:%d transform ptn name fail!\n", __func__, __LINE__);
		return PTN_ERROR;
	}

	for (n = 0; n < ptn_num; n++)
		if (!strcmp((ptable_src + n)->name, part_nm_tmp))
			return n;

	pr_err("%s: Input partition %s is not found\n", __func__, bdev_path);
	return PTN_ERROR;
}

#ifdef CONFIG_HISI_DEBUG_FS
#define MAX_BDEV_PATH_LEN 128
int flash_find_ptn_s_ut_test(void)
{
	const char ptn_name_noab[] = "isp_firmware";
	char bdev_path[MAX_BDEV_PATH_LEN] = {0};
	const char device_path_noab[] = "/dev/block/by-name/isp_firmware";
	int res;

	/* Scene 1:noab */
	/* Scene 2:current partition name have _a,the macro is close */
	res = flash_find_ptn_s(ptn_name_noab, bdev_path, sizeof(bdev_path));
	if (res) {
		pr_err("%s: test failed\n", __func__);
		return PTN_ERROR;
	}

	if (!strcmp(device_path_noab, bdev_path)) {
		pr_err("%s: test success\n", __func__);
		return 0;
	}

	pr_err("%s: test failed, expect %s but got %s\n",
		__func__, device_path_noab, bdev_path);

	return PTN_ERROR;
}

static DEFINE_SPINLOCK(ptable_print_protect_lock);
int print_partition_info(void)
{
	int part_idx;
	u32 ptn_num;
	unsigned long flags;
	u32 i;
	const struct partition *ptable_src = NULL;

	ptable_src = get_ptable_src();
	ptn_num = get_ptable_size();
	spin_lock_irqsave(&ptable_print_protect_lock, flags);
	pr_err("ptn_num: %u\n", ptn_num);
	pr_err("index\tname\n");

	for (i = 0; i < ptn_num - 1; i++) {
		part_idx = flash_get_ptn_index((ptable_src + i)->name);
		pr_err("%d\t%s\n", part_idx, (ptable_src + i)->name);
	}
	spin_unlock_irqrestore(&ptable_print_protect_lock, flags);
	return 0;
}
#endif /* #CONFIG_MAS_DEBUG_FS */
