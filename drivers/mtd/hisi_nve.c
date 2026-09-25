/* Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2019. All rights reserved.
 * FileName: kernel/drivers/mtd/hisi_nve.c
 * Description: complement NV partition(or called block) read and write
 * in kernel.
 * Author: jinguojun
 * Create: 2012-12-22
 * Revision history:2019-06-20 zhanghugang NVE CSEC
 */
#define pr_fmt(fmt) "[NVE]" fmt

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/random.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/compat.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#include "hisi_nve.h"
#ifdef CONFIG_CRC_SUPPORT
#include "hisi_nve_crc.h"
#endif

#if CONFIG_HISI_NVE_WHITELIST
#include "hisi_nve_whitelist.h"
#endif

static struct semaphore nv_sem;
static struct class *nve_class;
static unsigned int dev_major_num;
static struct nve_ramdisk_struct *nve_data_struct;
static int hisi_nv_setup_ok;
static char *nve_block_device_name;
#if CONFIG_HISI_NVE_WHITELIST
static int nve_whitelist_en = 1;
#endif
/* The number of valid nv items */
static unsigned int valid_nv_item_num;
/*
 * Function name:update_header_valid_nvme_items.
 * Discription:update the actual valid item in ramdisk
 * Parameters:
 * @ nve_ramdisk:the ramdisk stores the total nv partition
 */
static int update_header_valid_nvme_items(
				struct nv_partittion_struct *nve_ramdisk)
{
	unsigned int i;
	struct nv_items_struct nve_item = { 0 };
	unsigned int valid_items;

	if (!nve_ramdisk) {
		pr_err("[%s]line:%d nve_ramdisk is NULL.\n",
			__func__, __LINE__);
		return NVE_ERROR;
	}

	if (nve_ramdisk->header.valid_items > NV_ITEMS_MAX_NUMBER) {
		pr_err("[%s]line:%d valid_items is biger than 1023\n",
			__func__, __LINE__);
		return NVE_ERROR;
	}

	for (i = 0; i < nve_ramdisk->header.valid_items; i++) {
		/* find nve item's nv_number and check */
		nve_item = nve_ramdisk->NV_items[i];
		if (i != nve_item.nv_number)
			break;
	}
	valid_items = i;
	/* update ram valid_items.
	 * when the driver is just started, data is not loaded into memory,
	 * valid_nv_item_num is 0
	 */
	if (valid_nv_item_num == 0) {
		nve_ramdisk->header.valid_items = valid_items;
		return 0;
	}
	/* After the data is loaded, the read and write operations go here.
	 * The following unequalities indicate that the data read from
	 * the device is corrupted.
	 */
	if (valid_nv_item_num != valid_items) {
		pr_err("[%s]error, valid_items(%u) isnot equal to valid_nv_item_num(%u),device read data is broken.\n",
			__func__, valid_items, valid_nv_item_num);
		return NVE_ERROR;
	}

	return 0;
}

#ifdef CONFIG_CRC_SUPPORT
/*
 * Function name:check_crc_for_valid_items.
 * Discription:check the crc for nv items
 * Parameters:
 * @ nv_item_start:the start nv number
 * @ check_items:the number of check items
 * @ nve_ramdisk:the ramdisk stores the total nv partition
 * return value:
 *   0:check success;
 *   others:check failed
 */
static int check_crc_for_valid_items(unsigned int nv_item_start,
				unsigned int check_items,
				struct nv_partittion_struct *nve_ramdisk)
{
	int i = 0;
	uint8_t crc_data[NVE_NV_CRC_HEADER_SIZE + NVE_NV_DATA_SIZE] = { 0 }; /*lint !e302*/
	struct nv_items_struct *nv_item = NULL;
	uint32_t temp_crc = 0;
	int nv_number = 0;

	/* nv_item_start means the start nv number, range is 0-1022 */
	if (nv_item_start >= NV_ITEMS_MAX_NUMBER) {
		pr_err("invalid nv_item_start in check crc fuction, nv_item_start = %u\n",
		       nv_item_start);
		return -EINVAL;
	}

	/* check_items means the number of chek nv items, range is 1-1023 */
	if (check_items > NV_ITEMS_MAX_NUMBER || check_items <= 0) {
		pr_err("invalid check_items in check crc fuction, check_items = %u\n",
		       check_items);
		return -EINVAL;
	}

	if (!nve_ramdisk) {
		pr_err("[%s]line:%d nve_ramdisk is NULL.\n", __func__,
		       __LINE__);
		return -EINVAL;
	}

	for (i = 0; i < (int)check_items; i++) {
		nv_number = nv_item_start + i;
		if (nv_number >= NV_ITEMS_MAX_NUMBER) {
			pr_err("invalid nv number in check crc fuction, nv_item_start = %u, check_items = %u\n",
			       nv_item_start, check_items);
			return -EINVAL;
		}
		nv_item = &nve_ramdisk->NV_items[nv_number];
		memcpy(crc_data, nv_item, NVE_NV_CRC_HEADER_SIZE);
		memcpy(crc_data + NVE_NV_CRC_HEADER_SIZE, nv_item->nv_data,
		       (size_t)NVE_NV_DATA_SIZE);
		temp_crc = ~crc32c_nve(CRC32C_REV_SEED, crc_data,
				       sizeof(crc_data));

		if (nv_item->crc != temp_crc) {
			pr_err("crc check failed, item {%u}, old_crc_value = 0x%x, new_crc_value = 0x%x, crc_data = %s\n",
			       nv_number, nv_item->crc, temp_crc, crc_data);
			return -EINVAL;
		}
	}

	return 0;
}

/*
 * Function name:caculate_crc_for_valid_items.
 * Discription:caculate crc for nv item and update
 * Parameters:
 * @ nv_items:the pointer of nv item
 */
static void caculate_crc_for_valid_items(struct nv_items_struct *nv_items)
{
	uint8_t crc_data[NVE_NV_CRC_HEADER_SIZE + NVE_NV_DATA_SIZE] = { 0 }; /*lint !e302*/

	if (!nv_items) {
		pr_err("[%s]line:%d nv_items is NULL.\n", __func__,
		       __LINE__);
		return;
	}

	memcpy(crc_data, nv_items, NVE_NV_CRC_HEADER_SIZE);
	memcpy(crc_data + NVE_NV_CRC_HEADER_SIZE, nv_items->nv_data,
	       (size_t)NVE_NV_DATA_SIZE);
	nv_items->crc =
		~crc32c_nve(CRC32C_REV_SEED, crc_data, sizeof(crc_data));
}
#endif

/*
 * Function name:nve_increment.
 * Discription:complement NV block' increment automatically, when current block
 * has been writeen, block index will pointer to next block, and if current
 * block
 * is maximum block count ,then block index will be assigned "1", ensuring all
 * of
 * NV block will be used and written circularly.
 */
static void nve_increment(struct nve_ramdisk_struct *nvep)
{
	if (!nvep) {
		pr_err("[%s]line:%d nvep is NULL.\n", __func__, __LINE__);
		return;
	}

	if (nvep->nve_current_id >= nvep->nve_partition_count - 1)
		nvep->nve_current_id = 1;
	else
		nvep->nve_current_id++;
}

/*
 * Function name:nve_decrement.
 * Discription:complement NV block' decrement automatically, when current block
 * has been writeen, block index will pointer to next block, if write failed,
 * we will recover the index
 */
static void nve_decrement(struct nve_ramdisk_struct *nvep)
{
	if (!nvep) {
		pr_err("[%s]line:%d nvep is NULL.\n", __func__, __LINE__);
		return;
	}

	/* we only use 1-7 partition, if id is 1,
	 * next decrement id is 7, after init
	 * nvep->nve_partition_count is 8
	 */
	if (nvep->nve_current_id <= 1)
		nvep->nve_current_id = nvep->nve_partition_count - 1;
	else
		nvep->nve_current_id--;
}

/*
 * Function name:nve_read.
 * Discription:read NV partition to buf.
 * Parameters:
 *          @ from:emmc start block number that will be read.
 *          @ len:total bytes that will be read from emmc.
 *          @ buf:buffer used to store bytes that is read from emmc.
 */
static int nve_read(loff_t from, size_t len, u_char *buf)
{
	int ret;
	int fd = -1;

	/*lint -e501*/
	mm_segment_t oldfs = get_fs();

	set_fs(get_ds());
	/*lint +e501*/
	fd = sys_open(nve_block_device_name, O_RDONLY, 0);
	if (fd < 0) {
		pr_err("[%s]open nv block device failed, and fd = %d!\n",
		       __func__, fd);
		ret = -ENODEV;
		goto err;
	}

	ret = sys_lseek((unsigned int)fd, from, SEEK_SET);
	if (ret < 0) {
		pr_err("[%s] Fatal seek error, read flash from = 0x%llx, len = 0x%zx, ret = d.\n",
		       __func__, from, len, ret);
		ret = -EIO;
		goto out;
	}

	ret = sys_read((unsigned int)fd, (char *)buf, len);
	if (ret < 0) {
		pr_err("[%s] Fatal read error, read flash from = 0x%llx, len = 0x%zx, ret = d.\n",
		       __func__, from, len, ret);
		ret = -EIO;
		goto out;
	} else {
		ret = 0;
	}

out:
	sys_close((unsigned int)fd);
err:
	set_fs(oldfs);
	return ret;
}

/*
 * Function name:nve_write.
 * Discription:write NV partition.
 * Parameters:
 *          @ from:emmc start block number that will be written.
 *          @ len:total bytes that will be written from emmc.
 *          @ buf:given buffer whose bytes will be written to emmc.
 */
static int nve_write(loff_t from, size_t len, u_char *buf)
{
	int ret;
	int fd = -1;

	/*lint -e501*/
	mm_segment_t oldfs = get_fs();

	set_fs(get_ds());
	/*lint +e501*/
	fd = sys_open(nve_block_device_name, O_RDWR, 0);
	if (fd < 0) {
		pr_err("[%s]open nv block device failed, and fd = %x!\n",
		       __func__, fd);
		ret = -ENODEV;
		goto err;
	}

	ret = sys_lseek((unsigned int)fd, from, SEEK_SET);
	if (ret < 0) {
		pr_err("[%s] Fatal seek error, read flash from = 0x%llx, len = 0x%zx, ret = 0x%x.\n",
		       __func__, from, len, ret);
		ret = -EIO;
		goto out;
	}

	ret = sys_write((unsigned int)fd, (char *)buf, len);
	if (ret < 0) {
		pr_err("[%s] Fatal write error, read flash from = 0x%llx, len = 0x%zx, ret = 0x%x.\n",
		       __func__, from, len, ret);
		ret = -EIO;
		goto out;
	}

	ret = sys_fsync((unsigned int)fd);
	if (ret < 0) {
		pr_err("[%s] Fatal sync error, read flash from = 0x%llx, len = 0x%zx, ret = 0x%x.\n",
		       __func__, from, len, ret);
		ret = -EIO;
		goto out;
	} else {
		ret = 0;
	}

out:
	sys_close((unsigned int)fd);
err:
	set_fs(oldfs);
	return ret;
}

/*
 * Function name:nve_check_partition.
 * Discription:check current NV partition is valid partition or not by means of
 * comparing current partition's name to NVE_HEADER_NAME.
 * Parameters:
 *          @ ramdisk:struct nve_ramdisk_struct pointer.
 *          @ index  :indicates current NV partion that will be checked.
 * return value:
 *          @ 0 - current parition is valid.
 *          @ others - current parition is invalid.
 */
static int nve_check_partition(
			struct nv_partittion_struct *ramdisk, uint32_t index)
{
	int ret = 0;
	struct nve_partition_header_struct *nve_partition_header = NULL;

	if ((!ramdisk) || (index >= NVE_PARTITION_COUNT)) {
		pr_err("[%s]Input error in line %d!\n", __func__,
		       __LINE__);
		return -EINVAL;
	}

	nve_partition_header = &ramdisk->header;

	ret = nve_read((loff_t)index * NVE_PARTITION_SIZE,
		       (size_t)NVE_PARTITION_SIZE, (u_char *)ramdisk);
	if (ret) {
		pr_err("[%s]nve_read error in line %d!\n",
			__func__, __LINE__);
		return ret;
	}

	/* update for valid nvme items */
	ret = update_header_valid_nvme_items(ramdisk);
	if (ret) {
		pr_err("[%s]line:%d  valid_nv_items error!\n",
			__func__, __LINE__);
		return ret;
	}

	/* compare partition_name with const name,
	 * if return 0,then current partition is valid
	 */
	ret = strncmp(NVE_HEADER_NAME,
		      nve_partition_header->nve_partition_name,
		      sizeof(NVE_HEADER_NAME));
	if (ret)
		return ret;

#ifdef CONFIG_CRC_SUPPORT
	if (nve_partition_header->nve_crc_support ==
	    NVE_CRC_SUPPORT_VERSION) {
		/* check the crc for valid nvme items */
		ret = check_crc_for_valid_items(
			0, nve_partition_header->valid_items, ramdisk);
		if (ret) {
			pr_err("index = %u, valid_items = %u, version = %u, crc_support = %u\n",
			       index, nve_partition_header->valid_items,
			       nve_partition_header->nve_version,
			       nve_partition_header->nve_crc_support);
			return ret;
		}
	}
#endif

	return 0;
}

/*
 * Function name:nve_find_valid_partition.
 * Discription:find valid NV partition in terms of checking every
 * partition circularly. when two or more NV paritions are both valid,
 * nve_age will be used to indicates which one is really valid, i.e. the
 * partition whose age is the biggest is valid partition.
 */
static void nve_find_valid_partition(struct nve_ramdisk_struct *nvep)
{
	uint32_t i = 0;
	uint32_t age_temp = 0;
	int partition_invalid = 0;
	struct nve_partition_header_struct *nve_partition_header = NULL;

	if (!nvep) {
		pr_err("[%s]line:%d nvep is NULL.\n", __func__, __LINE__);
		return;
	}

	nve_partition_header = &nvep->nve_store_ramdisk->header;
	nvep->nve_current_id = NVE_INVALID_NVM;
	/* Nv partition is 1-7 */
	for (i = 1; i < nvep->nve_partition_count; i++) {
		partition_invalid =
			nve_check_partition(nvep->nve_store_ramdisk, i);

		if (partition_invalid)
			continue;

		if (nve_partition_header->nve_age > age_temp) {
			nvep->nve_current_id = i;
			age_temp = nve_partition_header->nve_age;
		}
	}

	pr_info("[%s]current_id = %u valid_items = %u, version = %u, crc_support = %u\n",
		__func__, nvep->nve_current_id,
		nve_partition_header->valid_items,
		nve_partition_header->nve_version,
		nve_partition_header->nve_crc_support);

}

static int write_ramdisk_to_device(unsigned int id,
				   struct nv_partittion_struct *ramdisk)
{
	struct nve_partition_header_struct *nve_partition_header = NULL;
	int ret;
	loff_t start_addr = 0;
	unsigned int nve_update_age = 0;

	if (id >= NVE_PARTITION_COUNT) {
		pr_err("[%s]invalid id in line %d!\n", __func__, __LINE__);
		return -EINVAL;
	}

	if (!ramdisk) {
		pr_err("[%s]line:%d ramdisk is NULL.\n", __func__,
		       __LINE__);
		return -EINVAL;
	}

	nve_partition_header = &ramdisk->header;
	nve_update_age = nve_partition_header->nve_age + 1;
	/* write to next partition a invalid age */
	nve_partition_header->nve_age = NVE_PARTITION_INVALID_AGE;
	/* write the old partition head */
	start_addr = (((loff_t)id + 1) * NVE_PARTITION_SIZE) - NVE_BLOCK_SIZE;
	ret = nve_write(start_addr, (size_t)NVE_BLOCK_SIZE,
			((unsigned char *)ramdisk + NVE_PARTITION_SIZE - NVE_BLOCK_SIZE));
	if (ret) {
		pr_err("[%s]write old nv partition head failed in line %d!\n",
		       __func__, __LINE__);
		/* recover the age */
		nve_partition_header->nve_age = nve_update_age - 1;
		return ret;
	}

	/* write the partition data */
	start_addr = (loff_t)id * NVE_PARTITION_SIZE;
	ret = nve_write(start_addr, (size_t)NVE_PARTITION_SIZE - NVE_BLOCK_SIZE,
			(unsigned char *)ramdisk);
	if (ret) {
		pr_err("[%s]write nv partition data failed in line %d!\n",
		       __func__, __LINE__);
		/* recover the age */
		nve_partition_header->nve_age = nve_update_age - 1;
		return ret;
	}

	/* after writing partition to device, read the partition again to check,
	 * if check not pass, return error and not update header age
	 */
	ret = nve_check_partition(ramdisk, id);
	if (ret) {
		pr_err("[%s]after writing partition to device, read the partition again to check failed!\n",
		       __func__);
		/* recover the age */
		nve_partition_header->nve_age = nve_update_age - 1;
		return ret;
	}

	/* update the partition head age */
	nve_partition_header->nve_age = nve_update_age;
	/* write the latest partition head */
	start_addr = (((loff_t)id + 1) * NVE_PARTITION_SIZE) - NVE_BLOCK_SIZE;
	ret = nve_write(start_addr, (size_t)NVE_BLOCK_SIZE,
			((unsigned char *)ramdisk + NVE_PARTITION_SIZE - NVE_BLOCK_SIZE));
	if (ret) {
		pr_err("[%s]write nv latest partition head failed in line %d!\n",
		       __func__, __LINE__);
		return ret;
	}

	return ret;
}

static int erase_ramdisk_to_device(unsigned int id,
				   struct nv_partittion_struct *ramdisk)
{
	int ret = 0;
	loff_t start_addr = 0;

	if (id >= NVE_PARTITION_COUNT) {
		pr_err("[%s]invalid id in line %d!\n", __func__, __LINE__);
		return -EINVAL;
	}

	if (!ramdisk) {
		pr_err("[%s]line:%d ramdisk is NULL.\n", __func__,
		       __LINE__);
		return -EINVAL;
	}

	memset((void *)ramdisk, 0, (unsigned long)NVE_PARTITION_SIZE);
	/* erase partition head */
	start_addr = (((loff_t)id + 1) * NVE_PARTITION_SIZE) - NVE_BLOCK_SIZE;
	ret = nve_write(start_addr, (size_t)NVE_BLOCK_SIZE,
			((unsigned char *)ramdisk + NVE_PARTITION_SIZE - NVE_BLOCK_SIZE));
	if (ret) {
		pr_err("[%s]erase partition head failed in line %d!\n",
		       __func__, __LINE__);
		return ret;
	}

	/* erase partition data */
	start_addr = (loff_t)id * NVE_PARTITION_SIZE;
	ret = nve_write(start_addr, (size_t)NVE_PARTITION_SIZE - NVE_BLOCK_SIZE,
			(unsigned char *)ramdisk);
	if (ret) {
		pr_err("[%s]erase partition data failed in line %d!\n",
		       __func__, __LINE__);
		return ret;
	}

	return ret;
}

/*
 * Function name:nve_update_and_check_item
 * Discription:update the nv item
 * Parameters:
 *          @ 0  - success
 *          @ others - failure
 */
static int nve_update_and_check_item(unsigned int update_items,
			      unsigned int valid_check_items,
			      struct nv_partittion_struct *nve_store_ramdisk,
			      struct nv_partittion_struct *nve_update_ramdisk)
{
	unsigned int i = 0;
#ifdef CONFIG_CRC_SUPPORT
	struct nve_partition_header_struct *nve_update_partition_header = NULL;
#endif

	if ((update_items > NV_ITEMS_MAX_NUMBER) ||
	    (valid_check_items > NV_ITEMS_MAX_NUMBER)) {
		pr_err("[%s] update_items or valid_check_items error\n",
		       __func__);
		return -EINVAL;
	}

	if (!nve_store_ramdisk) {
		pr_err("[%s]line:%d nve_store_ramdisk is NULL.\n",
		       __func__, __LINE__);
		return -EINVAL;
	}

	if (!nve_update_ramdisk) {
		pr_err("[%s]line:%d nve_update_ramdisk is NULL.\n",
		       __func__, __LINE__);
		return -EINVAL;
	}

#ifdef CONFIG_CRC_SUPPORT
	nve_update_partition_header = &nve_update_ramdisk->header;
#endif

	/* this place is update the latest nvpartition's nv items */
	for (i = 0; i < update_items; i++) {
		/* min valid items of two partition
		 * should check the name and property
		 */
		if ((i < valid_check_items) &&
			(nve_store_ramdisk->NV_items[i].nv_property)) {
			/* check the name is normal and reserved */
#ifdef CONFIG_CRC_SUPPORT
			/* when version is same, skip non-volatile NV item;
			 * when version is diff,
			 * to do something to non-volatile NV item.
			 * when version is diff, update is support CRC,
			 * only caculate the old item crc and
			 * change, then skip
			 */
			if (nve_update_partition_header->nve_crc_support == NVE_CRC_SUPPORT_VERSION)
				caculate_crc_for_valid_items(&nve_store_ramdisk->NV_items[i]);
			/* when version is diff, update is not
			 * support CRC, clear old item crc, then skip
			 */
			else
				nve_store_ramdisk->NV_items[i].crc = 0;
#endif
			continue;
		}
		/* update current partition ram */
		memcpy((void *)&nve_store_ramdisk->NV_items[i],
		       (void *)&nve_update_ramdisk->NV_items[i],
		       sizeof(struct nv_items_struct));
	}

	return 0;
}

/*
 * Function name:nve_restore.
 * Discription:NV is divided into 8 partitions(partition0 - parititon 7),
 * when we need to add new NV items, we should update partition0 first,
 * and then restore parition0 to current valid partition which shall be
 * one of partition0 - partition7.
 * Parameters:
 *          @ 0  - success
 *          @ others - failure
 */
static int nve_restore(struct nve_ramdisk_struct *nvep)
{
	int ret = 0;
	unsigned int valid_check_items = 0;
	unsigned int update_items = 0;
	unsigned int nve_age_temp = 0;
	struct nve_partition_header_struct *nve_store_partition_header = NULL;
	struct nve_partition_header_struct *nve_update_partition_header = NULL;

	if (!nvep) {
		pr_err("[%s]line:%d nvep is NULL.\n", __func__, __LINE__);
		return -EINVAL;
	}

	if (nvep->nve_current_id == NVE_INVALID_NVM) {
		ret = nve_read((loff_t)0, (size_t)NVE_PARTITION_SIZE,
			       (u_char *)nvep->nve_store_ramdisk);
		if (ret) {
			pr_err("[%s] nve read error %d in line [%d].\n",
			       __func__, ret, __LINE__);
			return -ENODEV;
		}
		/* update nv ram's header valid items */
		ret = update_header_valid_nvme_items(nvep->nve_store_ramdisk);
		if (ret) {
			pr_err("[%s]line:%d  valid_nv_items error!\n",
				__func__, __LINE__);
			return ret;
		}
		nve_store_partition_header = &nvep->nve_store_ramdisk->header;
		nvep->nve_current_id = 0;
		valid_check_items = 0;
		update_items = nve_store_partition_header->valid_items;
	} else {
		if (nve_read((loff_t)nvep->nve_current_id * NVE_PARTITION_SIZE,
			     (size_t)NVE_PARTITION_SIZE,
			     (u_char *)nvep->nve_store_ramdisk)) {
			pr_err("[%s] nve read error in line [%d].\n",
			       __func__, __LINE__);
			return -EFAULT;
		}
		/* update nv ram's header valid items */
		ret = update_header_valid_nvme_items(nvep->nve_store_ramdisk);
		if (ret) {
			pr_err("[%s]line:%d  valid_nv_items error!\n",
				__func__, __LINE__);
			return ret;
		}
		nve_store_partition_header = &nvep->nve_store_ramdisk->header;

		if (nve_read((loff_t)0, (size_t)NVE_PARTITION_SIZE,
			     (u_char *)nvep->nve_update_ramdisk)) {
			pr_err("[%s] nve read error in line [%d].\n",
			       __func__, __LINE__);
			return -EFAULT;
		}
		/* update nv ram's header valid items */
		ret = update_header_valid_nvme_items(nvep->nve_update_ramdisk);
		if (ret) {
			pr_err("[%s]line:%d  valid_nv_items error!\n",
				__func__, __LINE__);
			return ret;
		}
		nve_update_partition_header = &nvep->nve_update_ramdisk->header;

		/* get the min items in partition 0 and current partition.
		 * check the valid head for min items
		 */
		valid_check_items =
			min(nve_store_partition_header->valid_items,
			    nve_update_partition_header->valid_items);
		/* get the max items in partition 0 and current partition.
		 * if 0 partition valid item is less than current partition ,
		 * the delete items should also be updated
		 */
		update_items = max(nve_store_partition_header->valid_items,
				   nve_update_partition_header->valid_items);
		pr_info("valid_items [%u] and update_items [%u], update version = %u, crc_support = %u!\n",
			valid_check_items, update_items,
			nve_update_partition_header->nve_version,
			nve_update_partition_header->nve_crc_support);

		ret = nve_update_and_check_item(update_items, valid_check_items,
						nvep->nve_store_ramdisk,
						nvep->nve_update_ramdisk);
		if (ret) {
			pr_err("[%s]ERROR!!!nve_update_and_check_item failed!\n",
					__func__);
			return ret;
		}

		/* when current partition header is not valid,
		 * we will update the header and set the age
		 */
		if (strncmp(NVE_HEADER_NAME,
			    nve_store_partition_header->nve_partition_name,
			    (unsigned int)sizeof(NVE_HEADER_NAME))) {
			/* nve partition is corrupt,
			 * we need to recover the header too
			 */
			pr_err("[%s]ERROR!!! partition %d is corrupted invalidly,recover the header!\n",
					__func__, nvep->nve_current_id);
			/* store the orignal age */
			nve_age_temp = nve_store_partition_header->nve_age;
			/* update current header */
			memcpy(nve_store_partition_header,
					nve_update_partition_header,
					PARTITION_HEADER_SIZE);
			/* set the orignal age */
			nve_store_partition_header->nve_age = nve_age_temp;
		}

		nve_store_partition_header->valid_items =
			nve_update_partition_header->valid_items;
		nve_store_partition_header->nve_version =
			nve_update_partition_header->nve_version;
		nve_store_partition_header->nve_crc_support =
			nve_update_partition_header->nve_crc_support;
	}

	nve_increment(nvep);
	/* write to next partition */
	ret = write_ramdisk_to_device(nvep->nve_current_id,
				      nvep->nve_store_ramdisk);
	if (ret) {
		pr_err("[%s]write to device failed in line [%d].\n",
		       __func__, __LINE__);
		/* recover the current id */
		nve_decrement(nvep);
		return ret;
	}

	/* if nve item update items is not same we will restore an other one */
	if (valid_check_items != update_items) {
		nve_increment(nvep);
		/* write to next partition */
		ret = write_ramdisk_to_device(nvep->nve_current_id,
					      nvep->nve_store_ramdisk);
		if (ret) {
			pr_err("[%s]write to device failed in line [%d].\n",
			       __func__, __LINE__);
			/* recover the current id */
			nve_decrement(nvep);
			return ret;
		}
	}
	/* OK we will update the current ramdisk */
	memcpy(nvep->nve_current_ramdisk, nvep->nve_store_ramdisk,
	       NVE_PARTITION_SIZE);
	/* clear 0 partition */
	ret = erase_ramdisk_to_device(0, nvep->nve_update_ramdisk);
	if (ret) {
		pr_err("[%s]erase 0 partition failed in line [%d].\n",
		       __func__, __LINE__);
		return ret;
	}

	return ret;
}

/* test NV items in kernel. if you want to use this, please set macro
 * TEST_NV_IN_KERNEL to "1".
 */
#ifdef CONFIG_HISI_DEBUG_FS
#define NVE_TEST_TIMES 20
#define NVE_TEST_STRESS_TIMES 50
#define NVE_TEST_WRITE_VALUE "test_data"
#define NVE_TEST_VALID_SIZE sizeof(NVE_TEST_WRITE_VALUE)
#define NVE_TEST_OK 0
#define NVE_TEST_ERR 1

uint64_t g_nve_write_start_time;
uint64_t g_nve_write_end_time;
uint64_t g_nve_read_start_time;
uint64_t g_nve_read_end_time;
uint64_t g_nve_cost_time;

struct hisi_nve_info_user nv_read_info;
struct hisi_nve_info_user nv_write_info;
struct hisi_nve_info_user nv_init_info;

#if CONFIG_HISI_NVE_WHITELIST
void nve_whitelist_en_set(int en_whitelist)
{
	nve_whitelist_en = en_whitelist;
}

void nve_dump_whitelist(void)
{
	unsigned int i = 0;

	pr_err("nv_num whitelist:\n");
	for (i = 0; i < ARRAY_SIZE(nv_num_whitelist); i++)
		pr_err("%d ", nv_num_whitelist[i]);

	pr_err("\n\n");

	for (i = 0; i < ARRAY_SIZE(nv_process_whitelist); i++)
		pr_err("%s\n", nv_process_whitelist[i]);

	pr_err("\n");
}
#endif /* CONFIG_HISI_NVE_WHITELIST */

/*
 * Discription: Convert numbers to characters,
 *              such as the number 0 to the character '0'
 * Parameters: num：The range of num is 0x0-0xF
 * return：The converted characters
 */
static char num2char(int num)
{
	if ((num >= 0) && (num <= 9))
		return (char)(num + '0');
	else
		return (char)((num - 10) + 'A');
}

/*
 * Function name:nve_out_log.
 * Discription:output log of reading and writing nv.
 * Parameters:
 *          @ struct hisi_nve_info_user *user_info pointer.
 *          @ bool isRead
 * return value:
 *          void
 */
static void nve_out_log(struct hisi_nve_info_user *user_info, int isRead)
{
	unsigned int i;
	unsigned int j;
	char log[NVE_NV_DATA_SIZE + 1] = { 0 };

	if (!user_info) {
		pr_err("[%s]:user_info is null!\n", __func__);
		return;
	}

	pr_err("[%s]:nv number= %u\n", __func__, user_info->nv_number);
	pr_err("[%s]:nv data = 0x\n", __func__);
	for (i = 0, j = 0; i < user_info->valid_size; i++) {
		log[j++] = num2char((user_info->nv_data[i] >> 4) & 0xF);
		log[j++] = num2char(user_info->nv_data[i] & 0xF);
		log[j++] = ',';

		/* Print 16 characters per line */
		if (((i + 1) % 16 == 0) && (i > 0)) {
			pr_err("%s\n", log);
			memset(log, 0, NVE_NV_DATA_SIZE + 1);
			j = 0;
		}
	}
	pr_err("%s\n", log);

	memset(log, 0, NVE_NV_DATA_SIZE + 1);
	memcpy(log, user_info->nv_data, (size_t)user_info->valid_size);
	if (isRead)
		pr_err("[%s]:read data = %s\n", __func__, log);
	else
		pr_err("[%s]:write data = %s\n", __func__, log);
}

static int nve_print_partition_test(struct nv_partittion_struct *nve_partition)
{
	struct nve_partition_header_struct *nve_partiiton_header = NULL;
	uint32_t i = 0;
	char log_nv_data[NVE_NV_DATA_SIZE + 1];

	nve_partiiton_header = &nve_partition->header;

	pr_err("[NVE]partition name  :%s\n",
	       nve_partiiton_header->nve_partition_name);
	pr_err("[NVE]nve version :%u\n", nve_partiiton_header->nve_version);
	pr_err("[NVE]nve age :%u\n", nve_partiiton_header->nve_age);
	pr_err("[NVE]nve blockid     :%u\n",
	       nve_partiiton_header->nve_block_id);
	pr_err("[NVE]nve blockcount  :%u\n",
	       nve_partiiton_header->nve_block_count);
	pr_err("[NVE]valid items:%u\n", nve_partiiton_header->valid_items);
	pr_err("nv checksum     :%u\n", nve_partiiton_header->nv_checksum);
	pr_err("nv crc support     :%u\n",
	       nve_partiiton_header->nve_crc_support);

	if (nve_partiiton_header->valid_items > NV_ITEMS_MAX_NUMBER) {
		pr_err("[%s] valid_items error\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < nve_partiiton_header->valid_items; i++) {
		memset(log_nv_data, 0, NVE_NV_DATA_SIZE + 1);
		memcpy(log_nv_data, nve_partition->NV_items[i].nv_data,
		       nve_partition->NV_items[i].valid_size);
		pr_err("%u %s %u %u 0x%x %s\n",
		       nve_partition->NV_items[i].nv_number,
		       nve_partition->NV_items[i].nv_name,
		       nve_partition->NV_items[i].nv_property,
		       nve_partition->NV_items[i].valid_size,
		       nve_partition->NV_items[i].crc, log_nv_data);
	}

	return 0;
}

/*
 * Discription:maintenance command: adb shell ecall nve_write_test 8 10
 * Parameters:
 *          @ nv_item_num: the nv item number
 *          @ valid_size: the valid size of nv item
 * return value:
 *          0 - OK
 *          1 - ERR
 */
int nve_write_test(uint32_t nv_item_num, uint32_t valid_size)
{
	int ret = 0;
	unsigned char *data = (unsigned char *)NVE_TEST_WRITE_VALUE;

	memset(&nv_write_info, 0, sizeof(nv_write_info));
	strncpy(nv_write_info.nv_name, "NVTEST", sizeof("NVTEST"));
	nv_write_info.nv_number = nv_item_num;
	nv_write_info.valid_size = valid_size;
	nv_write_info.nv_operation = NV_WRITE;
	memset(nv_write_info.nv_data, 0x0, (size_t)NVE_NV_DATA_SIZE);
	memcpy(nv_write_info.nv_data, data, strlen(data) < NVE_NV_DATA_SIZE ?
						    strlen(data) :
						    NVE_NV_DATA_SIZE);
	nv_write_info.nv_data[NVE_NV_DATA_SIZE - 1] = '\0';
	ret = hisi_nve_direct_access(&nv_write_info);
	if (!ret) {
		pr_err("test nv write success!\n");
		pr_err("test nv write value:%s!\n", nv_write_info.nv_data);
		return NVE_TEST_OK;
	}

	pr_err("test nv write faild!\n");
	return NVE_TEST_ERR;
}

static int nve_read_init_value(uint32_t nv_item_num, uint32_t valid_size)
{
	int ret = 0;
	char log_nv_data[NVE_NV_DATA_SIZE + 1] = { 0 };

	memset(&nv_init_info, 0, sizeof(nv_init_info));
	strncpy(nv_init_info.nv_name, "NVTEST", sizeof("NVTEST"));
	nv_init_info.nv_number = nv_item_num;
	nv_init_info.valid_size = valid_size;
	nv_init_info.nv_operation = NV_READ;

	ret = hisi_nve_direct_access(&nv_init_info);
	if (!ret) {
		memcpy(log_nv_data, nv_init_info.nv_data,
		       nv_init_info.valid_size);
		pr_err("nve read init value success!\n");
		pr_err("read value:%s!\n", log_nv_data);
		return NVE_TEST_OK;
	}

	pr_err("nve read init value faild!\n");
	return NVE_TEST_ERR;
}

static int nve_write_init_value(void)
{
	int ret = 0;
	char log_nv_data[NVE_NV_DATA_SIZE + 1] = { 0 };

	nv_init_info.nv_operation = NV_WRITE;
	ret = hisi_nve_direct_access(&nv_init_info);

	if (!ret) {
		memcpy(log_nv_data, nv_init_info.nv_data,
		       nv_init_info.valid_size);
		pr_err("nve write init value success!\n");
		pr_err("write value:%s!\n", log_nv_data);
		return NVE_TEST_OK;
	}

	pr_err("nve write init value faild!\n");
	return NVE_TEST_ERR;
}

/*
 * Discription:maintenance command: adb shell ecall nve_read_test 8 10
 * Parameters:
 *          @ nv_item_num: the nv item number
 *          @ valid_size: the valid size of nv item
 * return value:
 *          0 - OK
 *          1 - ERR
 */
int nve_read_test(uint32_t nv_item_num, uint32_t valid_size)
{
	int ret = 0;
	char log_nv_data[NVE_NV_DATA_SIZE + 1] = { 0 };

	memset(&nv_read_info, 0, sizeof(nv_read_info));
	strncpy(nv_read_info.nv_name, "NVTEST", sizeof("NVTEST"));
	nv_read_info.nv_number = nv_item_num;
	nv_read_info.valid_size = valid_size;
	nv_read_info.nv_operation = NV_READ;

	ret = hisi_nve_direct_access(&nv_read_info);
	if (!ret) {
		memcpy(log_nv_data, nv_read_info.nv_data,
		       nv_read_info.valid_size);
		pr_err("test nv read success!\n");
		pr_err("test nv read value:%s!\n", log_nv_data);
		return NVE_TEST_OK;
	}

	pr_err("test nv read faild!\n");
	return NVE_TEST_ERR;
}

int nve_read_write_auto(uint32_t nv_item_num, uint32_t valid_size)
{
	int i = 0;
	int ret = 0;

	ret = nve_read_init_value(nv_item_num, valid_size);
	if (ret) {
		pr_err("nve_read_init_value test failed!\n");
		return ret;
	}

	for (i = 0; i < NVE_TEST_TIMES; i++) {
		ret = nve_write_test(nv_item_num, valid_size);
		if (ret) {
			pr_err("nve_write_test test failed!\n");
			return ret;
		}

		ret = nve_read_test(nv_item_num, valid_size);
		if (ret) {
			pr_err("nve_read_test test failed!\n");
			return ret;
		}

		if (strncmp((const char *)nv_read_info.nv_data,
			    (const char *)nv_write_info.nv_data,
			    (size_t)NVE_TEST_VALID_SIZE) == 0) {
			pr_err("test nve write and read value is same, test successed!\n");
		} else {
			pr_err("test nve write and read value is not same, test failed!\n");
			return NVE_TEST_ERR;
		}
	}

	ret = nve_write_init_value();
	if (ret) {
		pr_err("nve_write_init_value test failed!\n");
		return ret;
	}

	pr_err("test nve auto end!\n");
	return ret;
}

uint64_t nve_write_time_test(uint32_t nv_item_num, uint32_t valid_size)
{
	uint64_t total_nve_write_time = 0;
	uint64_t average_nve_write_time = 0;
	int i = 0;
	int ret = 0;

	ret = nve_read_init_value(nv_item_num, valid_size);
	if (ret) {
		pr_err("nve_read_init_value test failed!\n");
		return NVE_TEST_ERR;
	}

	for (i = 0; i < NVE_TEST_STRESS_TIMES; i++) {
		g_nve_write_start_time = hisi_getcurtime();
		ret = nve_write_test(nv_item_num, valid_size);
		g_nve_write_end_time = hisi_getcurtime();
		if (ret) {
			pr_err("nve_write_test test failed!\n");
			return NVE_TEST_ERR;
		}

		g_nve_cost_time = g_nve_write_end_time - g_nve_write_start_time;
		total_nve_write_time += g_nve_cost_time;
		pr_err("test nv cost time [%llu]ns,test_time = %d!\n",
		       g_nve_cost_time, i);
		msleep(20);
	}

	ret = nve_write_init_value();
	if (ret) {
		pr_err("nve_write_init_value test failed!\n");
		return NVE_TEST_ERR;
	}

	average_nve_write_time = total_nve_write_time / NVE_TEST_STRESS_TIMES;
	return average_nve_write_time;
}

uint64_t nve_read_time_test(uint32_t nv_item_num, uint32_t valid_size)
{
	uint64_t total_nve_read_time = 0;
	uint64_t average_nve_read_time = 0;
	int i = 0;
	int ret = 0;

	for (i = 0; i < NVE_TEST_STRESS_TIMES; i++) {
		g_nve_read_start_time = hisi_getcurtime();
		ret = nve_read_test(nv_item_num, valid_size);
		g_nve_read_end_time = hisi_getcurtime();
		if (ret) {
			pr_err("nve_write_test test failed!\n");
			return NVE_TEST_ERR;
		}

		g_nve_cost_time = g_nve_read_end_time - g_nve_read_start_time;
		total_nve_read_time += g_nve_cost_time;
		pr_err("test nv cost time [%llu]ns,test_time = %d!\n",
		       g_nve_cost_time, i);
		msleep(20);
	}

	average_nve_read_time = total_nve_read_time / NVE_TEST_STRESS_TIMES;
	return average_nve_read_time;
}

#ifdef CONFIG_CRC_SUPPORT
uint64_t nve_item_check_crc_test(unsigned int nv_number)
{
	int ret = 0;
	int i = 0;
	uint64_t total_check_crc_time = 0;
	uint64_t average_check_crc_time = 0;
	uint64_t check_crc_start_time = 0;
	uint64_t check_crc_end_time = 0;

	if (nve_data_struct->nve_current_ramdisk->header.nve_crc_support !=
	    NVE_CRC_SUPPORT_VERSION) {
		pr_err("current partition is not support CRC!\n");
		return NVE_TEST_ERR;
	}

	for (i = 0; i < NVE_TEST_STRESS_TIMES; i++) {
		check_crc_start_time = hisi_getcurtime();
		ret = check_crc_for_valid_items(
			nv_number, 1, nve_data_struct->nve_current_ramdisk);
		check_crc_end_time = hisi_getcurtime();
		if (!ret) {
			pr_err("NVE item CRC check success!cost time [%llu]ns,test_time = %d\n!",
			       (check_crc_end_time - check_crc_start_time), i);
		} else {
			pr_err("NVE item CRC check failed!\n");
			return NVE_TEST_ERR;
		}
		total_check_crc_time +=
			(check_crc_end_time - check_crc_start_time);
		msleep(20);
	}

	average_check_crc_time = total_check_crc_time / NVE_TEST_STRESS_TIMES;
	return average_check_crc_time;
}

uint64_t nve_current_partition_check_crc_test(void)
{
	int ret = 0;
	int i = 0;
	uint64_t total_check_crc_time = 0;
	uint64_t average_check_crc_time = 0;
	uint64_t check_crc_start_time = 0;
	uint64_t check_crc_end_time = 0;
	struct nve_partition_header_struct *nve_partition_header = NULL;

	nve_partition_header = &nve_data_struct->nve_current_ramdisk->header;

	if (nve_partition_header->nve_crc_support != NVE_CRC_SUPPORT_VERSION) {
		pr_err("current partition is not support CRC!");
		return NVE_TEST_ERR;
	}

	if ((!nve_data_struct) || (!nve_data_struct->nve_current_ramdisk)) {
		pr_err("nve_data_struct or nve_current_ramdisk is NULL !");
		return NVE_TEST_ERR;
	}

	for (i = 0; i < NVE_TEST_STRESS_TIMES; i++) {
		check_crc_start_time = hisi_getcurtime();
		ret = check_crc_for_valid_items(
			0, nve_partition_header->valid_items,
			nve_data_struct->nve_current_ramdisk);
		check_crc_end_time = hisi_getcurtime();
		if (!ret) {
			pr_err("NVE partition CRC check success!cost time [%llu]ns,test_time = %d!\n",
			       (check_crc_end_time - check_crc_start_time), i);
		} else {
			pr_err("NVE partition CRC check failed!\n");
			return NVE_TEST_ERR;
		}
		total_check_crc_time +=
			(check_crc_end_time - check_crc_start_time);
		msleep(20);
	}

	average_check_crc_time = total_check_crc_time / NVE_TEST_STRESS_TIMES;
	return average_check_crc_time;
}
#endif

void nve_all_test(void)
{
	nve_print_partition_test(nve_data_struct->nve_current_ramdisk);
}
#endif

/*
 * Function name:nve_open_ex.
 * Discription:open NV device.
 * return value:
 *          @ 0 - success.
 *          @ -1- failure.
 */
static int read_nve_to_ramdisk(void)
{
	int ret = 0;

	/* the driver is not initialized successfully, return error */
	if (!nve_data_struct) {
		ret = -ENOMEM;
		pr_err("[%s]:nve_data_struct has not been alloced.\n",
		       __func__);
		goto out;
	}

	/* Total avaliable NV partition size is 4M,but we only use 1M */
	nve_data_struct->nve_partition_count = NVE_PARTITION_COUNT;

	/*
	 * partition count must bigger than 3,
	 * one for partition 0,one for update, the other for runtime.
	 */
	if (nve_data_struct->nve_partition_count <= 3) {
		ret = -ENODEV;
		goto out;
	}

	nve_find_valid_partition(nve_data_struct);

	/* check partiton 0 is valid or not */
	ret = nve_check_partition(nve_data_struct->nve_store_ramdisk, 0);
	if (!ret) {
		/* partiton 0 is valid, restore it to current partition */
		pr_info("[NVE]partition0 is valid, restore it to current partition.\n");
		ret = nve_restore(nve_data_struct);
	}

	if (ret) {
		if (nve_data_struct->nve_current_id == NVE_INVALID_NVM) {
			pr_err("[%s]: no valid NVM.\n", __func__);
			ret = -ENODEV;
			goto out;
		} else {
			ret = 0;
		}
	}

	/* read the current NV partiton and store into Ramdisk */
	if (nve_read((unsigned long)nve_data_struct->nve_current_id *
			     NVE_PARTITION_SIZE,
		     (size_t)NVE_PARTITION_SIZE,
		     (unsigned char *)nve_data_struct->nve_current_ramdisk)) {
		pr_err("[%s]:nve_read error!\n", __func__);
		return -EIO;
	}

	ret = update_header_valid_nvme_items(nve_data_struct->nve_current_ramdisk);
	if (ret) {
		pr_err("[%s]line:%d valid_nv_items error!\n",
				__func__, __LINE__);
		return ret;
	}

	/* Drive initialization ends, record valid_nv_item_num */
	valid_nv_item_num =
		nve_data_struct->nve_current_ramdisk->header.valid_items;
	pr_err("[%s]line:%d init valid_nv_item_num = %u\n",
				__func__, __LINE__, valid_nv_item_num);
	nve_data_struct->initialized = 1;
out:
	return ret;
}

#ifdef CONFIG_CRC_SUPPORT
static int nve_check_crc_and_recover(unsigned int nv_item_start,
		unsigned int check_items, struct nve_ramdisk_struct *nvep)
{
	int ret = 0;

	if (!nvep) {
		pr_err("[%s]line:%d nvep is NULL.\n", __func__, __LINE__);
		return -EINVAL;
	}

	/* crc check for ram to one item */
	ret = check_crc_for_valid_items(nv_item_start, check_items,
					nvep->nve_current_ramdisk);
	/* retry to read to ramdisk */
	if (ret) {
		pr_err("[NVE], one item crc check failed, %d!\n", ret);
		/* this place can optmize */
		nve_find_valid_partition(nvep);
		if (nvep->nve_current_id == NVE_INVALID_NVM) {
			pr_err("[NVE]can't find the valid partition in 1-7!\n");
			return -EIO;
		}

		/* read the current NV partiton and store to Ramdisk */
		if (nve_read((unsigned long)nvep->nve_current_id *
				     NVE_PARTITION_SIZE,
			     NVE_PARTITION_SIZE,
			     (unsigned char *)
				     nvep->nve_current_ramdisk)) {
			pr_err("[NVE]nve_read error!\n");
			return -EIO;
		}

		ret = update_header_valid_nvme_items(
			nvep->nve_current_ramdisk);
		if (ret) {
			pr_err("[%s]line:%d  valid_nv_items error!\n",
				__func__, __LINE__);
			return ret;
		}
	}

	return 0;
}
#endif

#if CONFIG_HISI_NVE_WHITELIST
static bool nve_number_in_whitelist(uint32_t nv_number)
{
	unsigned int i = 0;

	for (i = 0; i < ARRAY_SIZE(nv_num_whitelist); i++) {
		if (nv_number == nv_num_whitelist[i])
			return true;
	}

	return false;
}

static bool nve_process_in_whitelist(void)
{
	unsigned int i = 0;

	for (i = 0; i < ARRAY_SIZE(nv_process_whitelist); i++) {
		if (!strncmp(current->comm, nv_process_whitelist[i],
			     strlen(nv_process_whitelist[i]) + 1)) {
			return true;
		}
	}

	return false;
}

static int hisi_nve_whitelist_check(struct hisi_nve_info_user *user_info)
{
	/* input parameter invalid, return.*/
	if (!user_info) {
		pr_err("[%s] input parameter is NULL.\n", __func__);
		return -EINVAL;
	}

	if (nve_whitelist_en && (user_info->nv_operation != NV_READ)) {
		if (nve_number_in_whitelist(user_info->nv_number) &&
		    (!nve_process_in_whitelist() || !get_userlock())) {
			pr_err("%s nv_number: %d process %s is not in whitelist, or phone was unlocked. Forbid write to NVE!\n",
			       __func__, user_info->nv_number, current->comm);

			return -EPERM;
		}
	}

	return 0;
}
#endif /* CONFIG_HISI_NVE_WHITELIST  */

static int hisi_nve_direct_access_for_ramdisk(
		struct hisi_nve_info_user *user_info)
{
	int ret = 0;
	struct nv_items_struct *nv_item = NULL;
	struct nv_items_struct nv_item_backup = { 0 };
	struct nve_partition_header_struct *nve_partition_header = NULL;

#if CONFIG_HISI_NVE_WHITELIST
	if (hisi_nve_whitelist_check(user_info)) {
		pr_err("[%s] hisi_nve_whitelist_check Failed!\n", __func__);
		return -EINVAL;
	}
#else
	/* input parameter invalid, return.*/
	if (!user_info) {
		pr_err("[%s] input parameter is NULL.\n", __func__);
		return -EINVAL;
	}
#endif /* CONFIG_HISI_NVE_WHITELIST */

	if (!nve_data_struct->nve_current_ramdisk) {
		pr_err("[%s] nve_data_struct->nve_current_ramdisk is NULL.\n",
			__func__);
		return -EINVAL;
	}

	/* get nve partition header to check nv number */
	nve_partition_header = &nve_data_struct->nve_current_ramdisk->header;
	if (user_info->nv_number >= nve_partition_header->valid_items) {
		pr_err("[%s] The input NV items[%d] is out of range,not exist.\n",
		       __func__, user_info->nv_number);
		ret = -EINVAL;
		goto out;
	}

	if (user_info->nv_number >= NV_ITEMS_MAX_NUMBER) {
		pr_err("[%s] nv_number is too big\n", __func__);
		return -EINVAL;
	}

	/* check nv valid size, it is same to old */
	nv_item = &nve_data_struct->nve_current_ramdisk
			   ->NV_items[user_info->nv_number];
	if (user_info->valid_size > nv_item->valid_size ||
	    user_info->valid_size == 0) {
		pr_err("[%s]info:Input valid_size = %d, original = %d!\n",
			__func__, user_info->valid_size, nv_item->valid_size);
		user_info->valid_size = nv_item->valid_size;
	}

	if (user_info->valid_size > NVE_NV_DATA_SIZE) {
		pr_err("[%s] user info valid size is error, %d.\n",
		       __func__, user_info->valid_size);
		ret = -EINVAL;
		goto out;
	}

	if (user_info->nv_operation == NV_READ) {
#ifdef CONFIG_CRC_SUPPORT
		if (nve_partition_header->nve_crc_support ==
		    NVE_CRC_SUPPORT_VERSION) {
			ret = nve_check_crc_and_recover(
				user_info->nv_number, 1, nve_data_struct);
			if (ret) {
				pr_err("[NVE]nve_check_crc_and_recover failed, ret = %d\n",
				       ret);
				return ret;
			}
		}
#endif
		/* read nv from ramdisk */
		memcpy(user_info->nv_data, nv_item->nv_data,
			(size_t)user_info->valid_size);
#ifdef CONFIG_HISI_DEBUG_FS
		nve_out_log(user_info, true);
#endif
	} else {
#ifdef CONFIG_CRC_SUPPORT
		if (nve_partition_header->nve_crc_support ==
		    NVE_CRC_SUPPORT_VERSION) {
			ret = nve_check_crc_and_recover(
				0, nve_partition_header->valid_items,
				nve_data_struct);
			if (ret) {
				pr_err("[NVE]nve_check_crc_and_recover failed, ret = %d\n",
				       ret);
				return ret;
			}
		}
#endif

		/* backup the original item,
		 * if write to device failed, we will recover the item
		 */
		memcpy(&nv_item_backup, nv_item,
			sizeof(struct nv_items_struct));
		/* write nv to ram */
		memset(nv_item->nv_data, 0x0, (unsigned long)NVE_NV_DATA_SIZE);
		memcpy(nv_item->nv_data, user_info->nv_data,
			(unsigned long)user_info->valid_size); /*[false alarm]:valid_size <= NVE_NV_DATA_SIZE */
#ifdef CONFIG_CRC_SUPPORT
		if (nve_partition_header->nve_crc_support ==
		    NVE_CRC_SUPPORT_VERSION)
			/* caculate crc to update */
			caculate_crc_for_valid_items(nv_item);
#endif
		/* update the current id */
		nve_increment(nve_data_struct);
		/* write the total ramdisk to device */
		ret = write_ramdisk_to_device(
			nve_data_struct->nve_current_id,
			nve_data_struct->nve_current_ramdisk);
		if (ret) {
			pr_err("[%s]write to device failed in line %d, and nv_number = %d!\n",
			       __func__, __LINE__, user_info->nv_number);
			/* if write to device failed, we will recover
			 * something recover the item
			 */
			memcpy(nv_item, &nv_item_backup,
				sizeof(struct nv_items_struct));
			/* recover the current id */
			nve_decrement(nve_data_struct);
			goto out;
		}

#ifdef CONFIG_HISI_DEBUG_FS
		nve_out_log(user_info, false);
#endif
	}
out:
	return ret;
}

/*
 * Function name:hisi_nve_direct_access.
 * Discription:read or write NV items interfaces that will be called by other
 * functions.
 * Parameters:
 *          @ user_info:struct hisi_nve_info_user pointer.
 * return value:
 *          @      0 - success.
 *          @ error - failure.
 * errors:
 *          @  ENOMEM - Out of memory
 *          @  ENODEV - No NVE device
 *          @  EBUSY  - Device or resource busy
 *          @  EFAULT - Bad address
 *          @  EINVAL - Invalid argument
 *          @  ENOTTY - Unknow command
 */
int hisi_nve_direct_access(struct hisi_nve_info_user *user_info)
{
	int ret = 0;

	if (!nve_data_struct) {
		pr_err("[%s] NVE struct not alloc.\n", __func__);
		return -ENOMEM;
	}
	/* the interface check the nv init */
	if (!nve_data_struct->initialized) {
		pr_err("[%s] NVE init is not done, please wait.\n",
		       __func__);
		return -ENODEV;
	}

	/* ensure only one process can visit NV at the same time in
	 * kernel
	 */
	if (down_interruptible(&nv_sem))
		return -EBUSY;

	ret = hisi_nve_direct_access_for_ramdisk(user_info);
	if (ret) {
		pr_err("[%s]access for nve according ramdisk failed in line %d!\n",
		       __func__, __LINE__);
		goto out;
	}

out:
	/* release the semaphore */
	up(&nv_sem);
	return ret;
}

/*
 * Function name:nve_open.
 * Discription:open NV device in terms of calling nve_open_ex().
 * return value:
 *          @      0 - success.
 *          @ error - failure.
 * errors:
 *          @  ENOMEM - Out of memory
 *          @  ENODEV - No NVE device
 */
static int nve_open(struct inode *inode, struct file *file)
{
	if (!nve_data_struct) {
		pr_err("[%s] NVE struct not alloc.\n", __func__);
		return -ENOMEM;
	}

	if (nve_data_struct->initialized)
		return 0;
	else
		return -ENODEV;
}

static int nve_close(struct inode *inode, struct file *file)
{
	return 0;
}

/*
 * Function name:nve_ioctl.
 * Discription:complement read or write NV by terms of sending command-words.
 * return value:
 *          @      0 - on success.
 *          @ -error - on error.
 * errors:
 *          @  ENOMEM - Out of memory
 *          @  ENODEV - No NVE device
 *          @  EBUSY  - Device or resource busy
 *          @  EFAULT - Bad address
 *          @  EINVAL - Invalid argument
 *          @  ENOTTY - Unknow command
 */
static long nve_ioctl(struct file *file, u_int cmd, u_long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	u_int size = 0;
	struct hisi_nve_info_user info = { 0 };

	/* make sure arg isnot NULL */
	if (!argp) {
		pr_err("[%s] The input arg is null.\n", __func__);
		return -ENOMEM;
	}

	/* make sure nve is init */
	if (!nve_data_struct) {
		pr_err("[%s] NVE struct not alloc.\n", __func__);
		return -ENOMEM;
	}

	/* the interface check the nv init */
	if (!nve_data_struct->initialized) {
		pr_err("[%s] NVE init is not done, please wait.\n",
		       __func__);
		return -ENODEV;
	}

	/* ensure only one process can visit NV device
	 * at the same time in API
	 */
	if (down_interruptible(&nv_sem))
		return -EBUSY;

	size = ((cmd & IOCSIZE_MASK) >> IOCSIZE_SHIFT);

	if (cmd & IOC_IN) {
		if (!access_ok(VERIFY_READ, arg, size)) {
			pr_err("[%s]access_in error!\n", __func__);
			up(&nv_sem);
			return -EFAULT;
		}
	}

	if (cmd & IOC_OUT) {
		if (!access_ok(VERIFY_WRITE, arg, size)) {
			pr_err("[%s]access_out error!\n", __func__);
			up(&nv_sem);
			return -EFAULT;
		}
	}

	switch (cmd) {
	case NVEACCESSDATA:
		if (copy_from_user(&info, argp,
				   sizeof(struct hisi_nve_info_user))) {
			up(&nv_sem);
			return -EFAULT;
		}

		ret = hisi_nve_direct_access_for_ramdisk(&info);
		if (ret) {
			pr_err("[%s]nve access failed in line %d!\n",
			       __func__, __LINE__);
			goto out;
		}

		if (info.nv_operation == NV_READ) {
			if (copy_to_user(argp, &info,
					 sizeof(struct hisi_nve_info_user))) {
				up(&nv_sem);
				return -EFAULT;
			}
		}
		break;
	default:
		pr_err("[%s] Unknow command!\n", __func__);
		ret = -ENOTTY;
		break;
	}
out:
	up(&nv_sem);
	return (long)ret;
}

#ifdef CONFIG_COMPAT
static long nve_compat_ioctl(struct file *file, u_int cmd, u_long arg)
{
	return nve_ioctl(file, cmd,
			 (unsigned long)compat_ptr((unsigned int)arg));
}
#endif

static const struct file_operations nve_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = nve_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = nve_compat_ioctl,
#endif
	.open = nve_open,
	.release = nve_close,
};

static int __init init_nve(void)
{
	int error = 0;

	/* semaphore initial */
	sema_init(&nv_sem, 1);

	/* alloc nve_ramdisk_struct */
	nve_data_struct = kzalloc(sizeof(struct nve_ramdisk_struct),
		GFP_KERNEL);
	if (!nve_data_struct) {
		pr_err("[%s] line %d failed kzalloc.\n",
		       __func__, __LINE__);
		return -NVE_ERROR_INIT;
	}

	/* alloc ramdisk */
	nve_data_struct->nve_current_ramdisk =
		(struct nv_partittion_struct *)kzalloc(
			(size_t)NVE_PARTITION_SIZE, GFP_KERNEL);
	if (!nve_data_struct->nve_current_ramdisk) {
		pr_err("[%s]failed to allocate current ramdisk buffer in line %d.\n",
		       __func__, __LINE__);
		error = -NVE_ERROR_INIT;
		goto failed_free_driver_data;
	}

	nve_data_struct->nve_update_ramdisk =
		(struct nv_partittion_struct *)kzalloc(
			(size_t)NVE_PARTITION_SIZE, GFP_KERNEL);
	if (!nve_data_struct->nve_update_ramdisk) {
		pr_err("[%s]failed to allocate update ramdisk buffer in line %d.\n",
		       __func__, __LINE__);
		error = -NVE_ERROR_INIT;
		goto failed_free_current_ramdisk;
	}

	nve_data_struct->nve_store_ramdisk =
		(struct nv_partittion_struct *)kzalloc(
			(size_t)NVE_PARTITION_SIZE, GFP_KERNEL);
	if (!nve_data_struct->nve_store_ramdisk) {
		pr_err("[%s]failed to allocate store ramdisk buffer in line %d.\n",
		       __func__, __LINE__);
		error = -NVE_ERROR_INIT;
		goto failed_free_update_ramdisk;
	}

	/* register a device in kernel, return the number of major device */
	nve_data_struct->nve_major_number = register_chrdev(0,
			"nve", &nve_fops);
	if (nve_data_struct->nve_major_number < 0) {
		pr_err("[NVE]Can't allocate major number for Non-Volatile memory Extension device.\n");
		error = -NVE_ERROR_INIT;
		goto failed_free_store_ramdisk;
	}

	/* register a class, make sure that mdev can create device node in
	 * "/dev"
	 */
	nve_class = class_create(THIS_MODULE, "nve");
	if (IS_ERR(nve_class)) {
		pr_err("[NVE]Error creating nve class.\n");
		unregister_chrdev(
		(unsigned int)nve_data_struct->nve_major_number, "nve");
		error = -NVE_ERROR_INIT;
		goto failed_free_store_ramdisk;
	}

	return 0;

failed_free_store_ramdisk:
	kfree(nve_data_struct->nve_store_ramdisk);
	nve_data_struct->nve_store_ramdisk = NULL;
failed_free_update_ramdisk:
	kfree(nve_data_struct->nve_update_ramdisk);
	nve_data_struct->nve_update_ramdisk = NULL;
failed_free_current_ramdisk:
	kfree(nve_data_struct->nve_current_ramdisk);
	nve_data_struct->nve_current_ramdisk = NULL;
failed_free_driver_data:
	kfree(nve_data_struct);
	nve_data_struct = NULL;
	return error;
}

static void __exit cleanup_nve(void)
{
	device_destroy(nve_class, MKDEV(dev_major_num, 0));
	class_destroy(nve_class);
	if (nve_data_struct) {
		unregister_chrdev(
		(unsigned int)nve_data_struct->nve_major_number, "nve");
		kfree(nve_data_struct->nve_store_ramdisk);
		nve_data_struct->nve_store_ramdisk = NULL;
		kfree(nve_data_struct->nve_update_ramdisk);
		nve_data_struct->nve_update_ramdisk = NULL;
		kfree(nve_data_struct->nve_current_ramdisk);
		nve_data_struct->nve_current_ramdisk = NULL;
		kfree(nve_data_struct);
		nve_data_struct = NULL;
		kfree(nve_block_device_name);
		nve_block_device_name = NULL;
	}
}

static int nve_setup(const char *val, const struct kernel_param *kp)
{
	int ret = 0;
	struct device *nve_dev = NULL;

	if (hisi_nv_setup_ok == 1) {
		pr_err("[%s]has been done.\n", __func__);
		return 0;
	}

	pr_info("[%s]nve setup begin.\n", __func__);
	/* get param by cmdline */
	if (!val)
		return -EINVAL;

	nve_block_device_name = kzalloc(strlen(val) + 1, GFP_KERNEL);
	if (!nve_block_device_name) {
		pr_err("[%s] line:%d failed to kzalloc.\n",
		       __func__, __LINE__);
		return -ENOMEM;
	}

	memcpy(nve_block_device_name, val, strlen(val) + 1);
	pr_info("[%s] device name = %s\n", __func__,
		nve_block_device_name);

	/* read nve partition to ramdisk */
	ret = read_nve_to_ramdisk();
	if (ret < 0) {
		pr_err("[%s] read nve to ramdisk failed!\n", __func__);
		return ret;
	}

	dev_major_num = (unsigned int)(nve_data_struct->nve_major_number);
	/* create a device node for application */
	nve_dev = device_create(nve_class, NULL, MKDEV(dev_major_num, 0), NULL,
				"nve0");
	if (IS_ERR(nve_dev)) {
		pr_err("[%s]failed to create nve device in line %d.\n",
		       __func__, __LINE__);
		return PTR_ERR(nve_dev);
	}

	hisi_nv_setup_ok = 1;
	pr_info("[%s]nve setup end.\n", __func__);

	return 0;
}

module_param_call(nve, nve_setup, NULL, NULL, 0660);

module_init(init_nve);
module_exit(cleanup_nve);

/* export hisi_nve_direct_access,so we can use it in other procedures */
EXPORT_SYMBOL(hisi_nve_direct_access);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hisi-nve");
MODULE_DESCRIPTION("Direct character-device access to NVE devices");
