/*
 *
 * Copyright (c) 2013-2015, Linux Foundation. All rights reserved.
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
#define pr_fmt(fmt) "[partition]" fmt

#include <linux/delay.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/bootdevice.h>
#include <asm/uaccess.h>
#include <partition.h>
#include <linux/hisi/kirin_partition.h>

int get_cunrrent_total_ptn_num(void)
{
	int current_ptn_num = 0;
	enum bootdevice_type boot_device_type = BOOT_DEVICE_EMMC;

	boot_device_type = get_bootdevice_type();

	if (boot_device_type == BOOT_DEVICE_EMMC)
		current_ptn_num = sizeof(partition_table_emmc) / sizeof(struct partition);
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
	else if (boot_device_type == BOOT_DEVICE_UFS)
		current_ptn_num = sizeof(partition_table_ufs) / sizeof(struct partition);
#endif
	return current_ptn_num;
}
EXPORT_SYMBOL(get_cunrrent_total_ptn_num);

void flash_find_hisee_ptn(const char *str, char *pblkname)
{
	char device_block_path[] = "/dev/block/";
	char device_path_emmc[] = "mmcblk0p28";
	char device_path_ufs[] = "sdd24";
	enum bootdevice_type boot_device_type;

	if ((!pblkname) || (!str)) {
		pr_err("Input partition name or device path buffer is NULL\n");
		return;
	}

	boot_device_type = get_bootdevice_type();
	if (boot_device_type == BOOT_DEVICE_EMMC) {
		strncpy(pblkname, device_block_path, strlen(device_block_path));/* unsafe_function_ignore: strncpy */
		strncpy(pblkname + strlen(device_block_path), device_path_emmc, strlen(device_path_emmc) + 1);/* unsafe_function_ignore: strncpy */
	} else {
		strncpy(pblkname, device_block_path, strlen(device_block_path));/* unsafe_function_ignore: strncpy */
		strncpy(pblkname + strlen(device_block_path), device_path_ufs, strlen(device_path_ufs) + 1);/* unsafe_function_ignore: strncpy */
	}

}
EXPORT_SYMBOL(flash_find_hisee_ptn);

/*
 *Function: New interface, Get the device path by partition name
 *Description:In the other versions of the P version, it is up to each module
 *to decide whether to use the new security interface.
 *If the calling module decides to use the old interface,
 *the security risk will be invoked by the module itself.
 *Input : ptn_name, partition name
 *Output: pblkname, boot device path, such as:/dev/block/by-name/xxx
 */
int flash_find_ptn_s(const char *ptn_name, char *pblkname,
						unsigned int pblkname_length)
{
	int n;
	char device_path[] = "/dev/block/by-name/";
	int current_ptn_num = 0;
	struct partition *current_partition_table = NULL;
	enum bootdevice_type boot_device_type = BOOT_DEVICE_EMMC;
	char partition_name_tmp[MAX_PARTITION_NAME_LENGTH] = {0};
#ifdef CONFIG_HISI_AB_PARTITION
	enum AB_PARTITION_TYPE storage_boot_partition_type = {0};
#endif

	if ((!pblkname) || (!ptn_name) || (!strlen(ptn_name))) {
		pr_err("[%s]:%d ptn_name or pblkname error\n"
				, __func__, __LINE__);
		return PTN_ERROR;
	}

#ifdef CONFIG_HISI_AB_PARTITION
	if (pblkname_length < (strlen(ptn_name) + strlen('_a')
						+ sizeof(device_path))) {
		pr_err("[%s]:%d pblkname_length is small, pblkname space not enough\n"
				, __func__, __LINE__);
		return PTN_ERROR;
	}
#else
	if (pblkname_length < (strlen(ptn_name) + sizeof(device_path))) {
		pr_err("[%s]:%d pblkname_length is small, pblkname space not enough\n"
				, __func__, __LINE__);
		return PTN_ERROR;
	}
#endif

	boot_device_type = get_bootdevice_type();
	if (boot_device_type == BOOT_DEVICE_EMMC) {
		current_partition_table  = (struct partition *)partition_table_emmc;
	}
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
	else if (boot_device_type == BOOT_DEVICE_UFS) {
		current_partition_table  = (struct partition *)partition_table_ufs;
	}
#endif
	else {
		pr_err("[%s]:%d Invalid boot device type\n"
						, __func__, __LINE__);
		return PTN_ERROR;
	}

	current_ptn_num = get_cunrrent_total_ptn_num();
	for (n = 0; n < current_ptn_num; n++) {
		if (!strcmp((current_partition_table + n)->name, ptn_name)) {/*[false alarm]:current_partition_table!=NULL*/
			strncpy(pblkname, device_path, strlen(device_path));/* unsafe_function_ignore: strncpy */
			strncpy(pblkname + strlen(device_path), ptn_name, strlen(ptn_name)+1);/* unsafe_function_ignore: strncpy */
			return 0;
		}
	}

	if (strlen(ptn_name) > (sizeof(partition_name_tmp) - 3)) {
		pr_err("[%s]:%d Invalid input ptn_name\n", __func__, __LINE__);
		return PTN_ERROR;
	}

	memset(partition_name_tmp, 0, sizeof(partition_name_tmp));/* unsafe_function_ignore: memset */
	strncpy(partition_name_tmp, ptn_name, sizeof(partition_name_tmp) - 3);/* unsafe_function_ignore: strncpy */

#ifdef CONFIG_HISI_AB_PARTITION
	storage_boot_partition_type = get_device_boot_partition_type();
	if (storage_boot_partition_type == BOOT_XLOADER_A) {
		strncpy(partition_name_tmp + strlen(partition_name_tmp), "_a", 3);/* unsafe_function_ignore: strncpy */
	} else if (storage_boot_partition_type == BOOT_XLOADER_B) {
		strncpy(partition_name_tmp + strlen(partition_name_tmp), "_b", 3);/* unsafe_function_ignore: strncpy */
	} else {
		pr_err("[%s]:%d partition is not found, ptn_name = %s, pblkname = %s\n"
				, __func__, __LINE__, ptn_name, pblkname);
		return PTN_ERROR;
	}

	current_ptn_num = get_cunrrent_total_ptn_num();
	for (n = 0; n < current_ptn_num; n++) {
		if (!strcmp((current_partition_table + n)->name, partition_name_tmp)) {/*[false alarm]:current_partition_table!=NULL*/
			strncpy(pblkname, device_path, strlen(device_path));/* unsafe_function_ignore: strncpy */
			strncpy(pblkname + strlen(device_path), partition_name_tmp, strlen(partition_name_tmp)+1);/* unsafe_function_ignore: strncpy */
			return 0;
		}
	}
#else
	strncpy(partition_name_tmp + strlen(partition_name_tmp), "_a", 3);/* unsafe_function_ignore: strncpy */
	current_ptn_num = get_cunrrent_total_ptn_num();

	for (n = 0; n < current_ptn_num; n++) {
		if (!strcmp((current_partition_table + n)->name, partition_name_tmp)) {/*[false alarm]:current_partition_table!=NULL*/
			strncpy(pblkname, device_path, strlen(device_path));/* unsafe_function_ignore: strncpy */
			strncpy(pblkname + strlen(device_path), ptn_name, strlen(ptn_name)+1);/* unsafe_function_ignore: strncpy */
			return 0;
		}
	}
#endif
	pr_err("[%s]:%d partition is not found, ptn_name = %s, pblkname = %s\n"
			, __func__, __LINE__, ptn_name, pblkname);
	return PTN_ERROR;
}
EXPORT_SYMBOL(flash_find_ptn_s);

/*
 *Function: Old interface, Get the device path by partition name
 *Description:In the other versions of the P version, it is up to each module
 *to decide whether to use the new security interface.
 *If the calling module decides to use the old interface,
 *the security risk will be invoked by the module itself.
 *Input : ptn_name, partition name
 *Output: pblkname, boot device path, such as:/dev/block/by-name/xxx
 */
int flash_find_ptn(const char *ptn_name, char *pblkname)
{
	int ret = -1;
	unsigned int pblkname_length = 0;

#ifdef CONFIG_HISI_AB_PARTITION
	pblkname_length = strlen(ptn_name) + strlen('_a')
					+ sizeof("/dev/block/by-name/");
#else
	pblkname_length = strlen(ptn_name) + sizeof("/dev/block/by-name/");
#endif

	ret = flash_find_ptn_s(ptn_name, pblkname, pblkname_length);
	if (ret) {
		pr_err("[%s]:%d pblkname find error\n", __func__, __LINE__);
		return PTN_ERROR;
	}

	return 0;
}
EXPORT_SYMBOL(flash_find_ptn);

/*
 *Get partition offset in total partitions(all lu), not the current LU
 */
int flash_get_ptn_index(const char *pblkname)
{
	int n;
	int current_ptn_num;
	struct partition *current_partition_table = NULL;
	enum bootdevice_type boot_device_type;
	char partition_name_tmp[MAX_PARTITION_NAME_LENGTH];
#ifdef CONFIG_HISI_AB_PARTITION
	enum AB_PARTITION_TYPE storage_boot_partition_type;
#endif
	if (!pblkname) {
		pr_err("[%s]Input partition name is NULL\n", __func__);
		return PTN_ERROR;
	}

	boot_device_type = get_bootdevice_type();
	if (boot_device_type == BOOT_DEVICE_EMMC) {
		current_partition_table  = (struct partition *)partition_table_emmc;
	}
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
	else if (boot_device_type == BOOT_DEVICE_UFS) {
		current_partition_table  = (struct partition *)partition_table_ufs;
	}
#endif
	else {
		pr_err("[%s]Invalid boot device type\n", __func__);
		return PTN_ERROR;
	}

	current_ptn_num = get_cunrrent_total_ptn_num();

	if (!strcmp(PART_XLOADER, pblkname)) {
		pr_err("[%s]This is boot partition\n", __func__);
		return PTN_ERROR;
	}

	/*normal partition*/
	for (n = 0; n < current_ptn_num; n++) {
		if (!strcmp((current_partition_table + n)->name, pblkname))
			return n;
	}

	if (strlen(pblkname) > (sizeof(partition_name_tmp) - 3)) {
		pr_err("[%s]Invalid input pblkname\n", __func__);
		return PTN_ERROR;
	}

	memset(partition_name_tmp, 0, sizeof(partition_name_tmp));/* unsafe_function_ignore: memset */
	strncpy(partition_name_tmp, pblkname, sizeof(partition_name_tmp) - 3);/* unsafe_function_ignore: strncpy */

#ifdef CONFIG_HISI_AB_PARTITION
	/*A/B partition*/
	storage_boot_partition_type = get_device_boot_partition_type();
	if (storage_boot_partition_type == BOOT_XLOADER_A) {
		strncpy(partition_name_tmp + strlen(partition_name_tmp), "_a", 3);/* unsafe_function_ignore: strncpy */
	} else if (storage_boot_partition_type == BOOT_XLOADER_B) {
		strncpy(partition_name_tmp + strlen(partition_name_tmp), "_b", 3);/* unsafe_function_ignore: strncpy */
	} else {
		pr_err("[%s]%d:Input partition(%s) is not found\n",
					__func__, __LINE__, pblkname);
		return PTN_ERROR;
	}
#else
	strncpy(partition_name_tmp + strlen(partition_name_tmp), "_a", 3);/* unsafe_function_ignore: strncpy */
#endif

	for (n = 0; n < current_ptn_num; n++) {
		if (!strcmp((current_partition_table + n)->name, partition_name_tmp))
			return n;
	}

	pr_err("[%s]%d:Input partition(%s) is not found\n",
					__func__, __LINE__, pblkname);
	return PTN_ERROR;
}
EXPORT_SYMBOL(flash_get_ptn_index);

enum AB_PARTITION_TYPE emmc_boot_partition_type = XLOADER_A;
enum AB_PARTITION_TYPE ufs_boot_partition_type = XLOADER_A;

/*
 *Get storage boot partition type:XLOADER_A or XLOADER_B
 */
enum AB_PARTITION_TYPE get_device_boot_partition_type(void)
{
#ifdef CONFIG_HISI_AB_PARTITION
	enum bootdevice_type boot_device_type;

	boot_device_type = get_bootdevice_type();

	if (boot_device_type == BOOT_DEVICE_EMMC) {
		return emmc_boot_partition_type;
	}
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
	else if (boot_device_type == BOOT_DEVICE_UFS) {
		return ufs_boot_partition_type;
	}
#endif
	else {
		pr_err("invalid boot device type\n");
		return ERROR_VALUE;
	}
#else
	pr_info("Not support AB partition\n");
	return NO_SUPPORT_AB;
#endif
}

#ifdef CONFIG_HISI_AB_PARTITION
/*
 *set storage boot partition type
 */
int set_device_boot_partition_type(char boot_partition_type)
{
	int ret;
	enum bootdevice_type boot_device_type;

	boot_device_type = get_bootdevice_type();

	if (boot_device_type == BOOT_DEVICE_EMMC) {
		ret = mmc_set_boot_partition_type(boot_partition_type);
		if (ret) {
			pr_err("set boot device type failed\n");
			return PTN_ERROR;
		}
		emmc_boot_partition_type = boot_partition_type;
	}
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
	else if (boot_device_type == BOOT_DEVICE_UFS) {
		ret = ufs_set_boot_partition_type(boot_partition_type);
		if (ret) {
			pr_err("set boot device type failed\n");
			return PTN_ERROR;
		}
		ufs_boot_partition_type = boot_partition_type;
	}
#endif
	else {
		pr_err("invalid boot device type\n");
		return PTN_ERROR;
	}

	return 0;
}
#endif /* CONFIG_HISI_AB_PARTITION */

#ifdef CONFIG_HISI_DEBUG_FS
int flash_find_ptn_s_ut_test(void)
{
	char ptn_name_noab[] = "isp_firmware";
	char pblkname[100] = {'\0'};
	char device_path_noab[] = "/dev/block/by-name/isp_firmware";
#ifdef CONFIG_HISI_AB_PARTITION
	char ptn_name_ab[] = "isp_firmware_a";
	char device_path_ab[] = "/dev/block/by-name/isp_firmware_a";
#endif
	int res = 0;

	/*Scene 1:noab*/
	/*Scene 2:current partition name have _a,the macro is close*/
	res = flash_find_ptn_s(ptn_name_noab, pblkname, sizeof(pblkname));
	if (res == 0) {
		if (strcmp(device_path_noab, pblkname) == 0) {
			pr_err("[%s]noab, test success\n", __func__);
			return 0;
		}

		pr_err("[%s]noab, test failed, not equal, Expectation is %s, Actual is %s\n"
				, __func__, device_path_noab, pblkname);
	} else {
		pr_err("[%s]noab, test failed,not find\n", __func__);
	}

#ifdef CONFIG_HISI_AB_PARTITION
	/*Scene 3:ab*/
	res = flash_find_ptn_s(ptn_name_ab, pblkname, sizeof(pblkname));
	if (res == 0) {
		if (strcmp(device_path_ab, pblkname) == 0) {
			pr_err("[%s] ab, test success\n", __func__);
			return 0;
		}

		pr_err("[%s]:%d ab,test failed, not equal, Expectation is %s, Actual is %s\n"
			, __func__, __LINE__, device_path_ab, pblkname);
	} else {
		pr_err("[%s]:%d ab,test failed,%s not find\n"
					, __func__, __LINE__, ptn_name_ab);
	}
#endif
	return PTN_ERROR;

}

int print_partition_info(void)
{
	int res = 0;
	int n;
	int current_ptn_num;
	struct partition *current_partition_table = NULL;
	enum bootdevice_type boot_device_type;

	boot_device_type = get_bootdevice_type();
	if (boot_device_type == BOOT_DEVICE_EMMC) {
		current_partition_table =
			(struct partition *)partition_table_emmc;
	}
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
	else if (boot_device_type == BOOT_DEVICE_UFS) {
		current_partition_table =
			(struct partition *)partition_table_ufs;
	}
#endif
	else {
		pr_err("[%s]Invalid boot device type.\n", __func__);
		return PTN_ERROR;
	}

	current_ptn_num = get_cunrrent_total_ptn_num();
	pr_err("[%s]boot_device_type = %d, current_ptn_num = %d .\n",
				__func__, boot_device_type, current_ptn_num);
	pr_err("[%s]index			name\n", __func__);

	/* xloader not in kernel, print separately*/
	if (!strcmp(PART_XLOADER, (current_partition_table + 0)->name)) {
		pr_err("[%s]%d			%s\n",
			__func__, 0, (current_partition_table + 0)->name);
	}

	for (n = 1; n < current_ptn_num - 1; n++) {
		res = flash_get_ptn_index((current_partition_table + n)->name);
		pr_err("[%s]%d			%s\n",
			__func__, res, (current_partition_table + n)->name);
	}

#ifdef CONFIG_HISI_AB_PARTITION
	pr_err("CONFIG_HISI_AB_PARTITION is open.\n");
#else
	pr_err("CONFIG_HISI_AB_PARTITION is close.\n");
#endif

	return 0;
}
#endif /*#CONFIG_HISI_DEBUG_FS*/
