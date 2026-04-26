/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: mas block unistore partition debug
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

#define pr_fmt(fmt) "[BLK-IO]" fmt

#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/module.h>
#include <linux/bio.h>
#include <linux/delay.h>
#include <linux/gfp.h>
#include <linux/hisi/powerkey_event.h>
#include <trace/events/block.h>
#include <linux/types.h>
#include "blk.h"
#include "dsm_block.h"

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
ssize_t mas_part_stream_id_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct hd_struct *p = dev_to_part(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n", p->mas_hd.default_stream_id);
}

ssize_t mas_part_stream_id_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct hd_struct *p = dev_to_part(dev);
	struct blkdev_set_stream stream;
	unsigned int tmp;

	if (count <= 0)
		return count;

	if ((sscanf(buf, "%u", &tmp) <= 0))
		return count;

	if (tmp >= STREAM_NUM) {
		pr_err("%s id is <%u>\n", __func__, tmp);
	} else {
		stream.stream_id = (unsigned char)(tmp);
		p->mas_hd.default_stream_id = stream.stream_id;
	}

	return count;
}

ssize_t mas_part_stream_id_tst_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct hd_struct *p = dev_to_part(dev);

	return snprintf(buf, PAGE_SIZE, " %llu:%llu:%llu:%llu:%llu \n %llu:%llu:%llu:%llu:%llu \n",
				(p->mas_hd.stream_id_0_max_addr -
					(p->start_sect << SECTOR_BYTE)) >> (SECTION_SECTOR + SECTOR_BYTE),
				(p->mas_hd.stream_id_1_max_addr -
					(p->start_sect << SECTOR_BYTE)) >> (SECTION_SECTOR + SECTOR_BYTE),
				(p->mas_hd.stream_id_2_max_addr -
					(p->start_sect << SECTOR_BYTE)) >> (SECTION_SECTOR + SECTOR_BYTE),
				(p->mas_hd.stream_id_3_max_addr -
					(p->start_sect << SECTOR_BYTE)) >> (SECTION_SECTOR + SECTOR_BYTE),
				(p->mas_hd.stream_id_4_max_addr -
					(p->start_sect << SECTOR_BYTE)) >> (SECTION_SECTOR + SECTOR_BYTE),
				p->mas_hd.stream_id_0_max_addr, p->mas_hd.stream_id_1_max_addr,
				p->mas_hd.stream_id_2_max_addr, p->mas_hd.stream_id_3_max_addr,
				p->mas_hd.stream_id_4_max_addr);
}

static void mas_part_stream_id_tst_store_handle(struct hd_struct *p)
{
	p->mas_hd.stream_id_0_max_addr <<= (SECTION_SECTOR + SECTOR_BYTE);
	p->mas_hd.stream_id_1_max_addr <<= (SECTION_SECTOR + SECTOR_BYTE);
	p->mas_hd.stream_id_2_max_addr <<= (SECTION_SECTOR + SECTOR_BYTE);
	p->mas_hd.stream_id_3_max_addr <<= (SECTION_SECTOR + SECTOR_BYTE);
	p->mas_hd.stream_id_4_max_addr <<= (SECTION_SECTOR + SECTOR_BYTE);

	p->mas_hd.stream_id_0_max_addr += p->start_sect << SECTOR_BYTE;
	p->mas_hd.stream_id_1_max_addr += p->start_sect << SECTOR_BYTE;
	p->mas_hd.stream_id_2_max_addr += p->start_sect << SECTOR_BYTE;
	p->mas_hd.stream_id_3_max_addr += p->start_sect << SECTOR_BYTE;
	p->mas_hd.stream_id_4_max_addr += p->start_sect << SECTOR_BYTE;

	p->mas_hd.stream_id_tst_mode = 0x5A;
}

ssize_t mas_part_stream_id_tst_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hd_struct *p = dev_to_part(dev);
	if (sscanf(buf, "%llu:%llu:%llu:%llu:%llu", &p->mas_hd.stream_id_0_max_addr,
				&p->mas_hd.stream_id_1_max_addr, &p->mas_hd.stream_id_2_max_addr,
				&p->mas_hd.stream_id_3_max_addr, &p->mas_hd.stream_id_4_max_addr) <= 0) {
		pr_err("%s:sscanf failed\n", __func__);
		goto tst_out;
	}

	pr_err("input:%llu:%llu:%llu:%llu:%llu <%s>\n",
		p->mas_hd.stream_id_0_max_addr, p->mas_hd.stream_id_1_max_addr,
		p->mas_hd.stream_id_2_max_addr, p->mas_hd.stream_id_3_max_addr,
		p->mas_hd.stream_id_4_max_addr, buf);

	if (p->mas_hd.stream_id_0_max_addr > p->mas_hd.stream_id_1_max_addr) {
		pr_err("stream_id_0_max_addr > stream_id_1_max_addr\n");
		goto tst_out;
	}
	if (p->mas_hd.stream_id_1_max_addr > p->mas_hd.stream_id_2_max_addr) {
		pr_err("stream_id_1_max_addr > stream_id_2_max_addr\n");
		goto tst_out;
	}
	if (p->mas_hd.stream_id_2_max_addr > p->mas_hd.stream_id_3_max_addr) {
		pr_err("stream_id_2_max_addr > stream_id_3_max_addr\n");
		goto tst_out;
	}
	if (p->mas_hd.stream_id_3_max_addr > p->mas_hd.stream_id_4_max_addr) {
		pr_err("stream_id_3_max_addr > stream_id_4_max_addr\n");
		goto tst_out;
	}

	if (!p->mas_hd.stream_id_0_max_addr && !p->mas_hd.stream_id_1_max_addr &&
		!p->mas_hd.stream_id_2_max_addr && !p->mas_hd.stream_id_3_max_addr &&
		!p->mas_hd.stream_id_4_max_addr) {
		pr_err("stream id tst mode exit");
		goto tst_out;
	}

	mas_part_stream_id_tst_store_handle(p);

	pr_err("%llu:%llu:%llu:%llu:%llu offset: %llu stream_id_tst_mode = 0x%x\r\n",
		p->mas_hd.stream_id_0_max_addr, p->mas_hd.stream_id_1_max_addr,
		p->mas_hd.stream_id_2_max_addr, p->mas_hd.stream_id_3_max_addr,
		p->mas_hd.stream_id_4_max_addr, p->start_sect << SECTOR_BYTE,
		p->mas_hd.stream_id_tst_mode);

	return count;

tst_out:
	p->mas_hd.stream_id_tst_mode = 0;
	return count;
}

struct bdev_issue_unmap_para {
	struct hd_struct *part;
	unsigned long long lba;
	unsigned long long len;
};

static void bdev_issue_discard(struct block_device *bdev, void *arg)
{
	struct bdev_issue_unmap_para *para = (struct bdev_issue_unmap_para *)arg;
	if (bdev->bd_part != para->part)
		return;
	pr_err("find the partition block device\n");
	(void)blkdev_issue_discard(bdev, (para->lba >> SECTOR_BYTE),
		(para->len >> SECTOR_BYTE), GFP_NOFS, 0);
}

ssize_t mas_part_discard_simulate_store(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count)
{
	struct bdev_issue_unmap_para para;
	struct hd_struct *p = dev_to_part(dev);

	para.part = p;
	if (sscanf(buf, "%llu:%llu", &para.lba, &para.len) <= 0) {
		pr_err("%s:sscanf failed\n", __func__);
		return -EINVAL;
	}
	pr_err("discard lba = %llu, len = %llu\n", para.lba, para.len);
	iterate_bdevs(bdev_issue_discard, &para);
	return count;
}
#endif
