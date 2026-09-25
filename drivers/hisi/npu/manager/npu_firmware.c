/*
 * npu_firmware.c
 *
 * about npu firmware
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
#include "npu_firmware.h"

#include <securec.h>
#include <linux/io.h>
#include <linux/hisi/rdr_pub.h>

#include "npu_log.h"
#include "npu_common.h"
#include "npu_platform.h"
#include "bbox/npu_dfx_black_box.h"

#define NPU_FIRMWARE_GET_FILE_SIZE_FAIL  (-1)

u32 devdrv_get_firmware_remap_size(int type);

static struct devdrv_ts_bin_info dev_ts_bin_info = {0};
static struct devdrv_mem_desc* fw_mem_desc[DEVDRV_FW_TYPE_MAX] = {0};
static u8 g_bin_valid_buf[BIN_VALID_BUFF_LEN] = {0};

ssize_t devdrv_read_file(struct file *fp, void *firmware_dst_addr, loff_t fsize, loff_t pos, int buf_len)
{
	ssize_t ret;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs((mm_segment_t)KERNEL_DS);
	ret = vfs_read(fp, firmware_dst_addr, fsize, &pos);
	set_fs(old_fs);

	return ret;
}

loff_t devdrv_get_file_size(const char *path)
{
	int error;
	loff_t filesize = -1;
	struct kstat statbuff;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs((mm_segment_t)KERNEL_DS);

	error = vfs_stat(path, &statbuff);
	if (error < 0) {
		NPU_DRV_ERR("vfs_stat failed ret = %d\n ", error);
		set_fs(old_fs);
		return filesize;
	} else {
		filesize = statbuff.size;
	}
	set_fs(old_fs);
	return filesize;
}

static void devdrv_get_sha256_check(const u8 *buf, struct devdrv_check_sum *out)
{
	char str[128];
	int i;

	for (i = 0; i < DEVDRV_TS_BIN_CHEKC_LEN; i++) {
		out->check[i] = *(u8 *)buf;
		buf += sizeof(u8);
		snprintf(&str[i * 2], 3, "%02x", out->check[i]);
	}

	str[i * 2] = '\0';
	NPU_DRV_DEBUG("get sha256 from file: %s\n", str);
}

static void devdrv_get_segment_info(u8 *buf, u32 segment_num, u32 buff_len)
{
	u32 i;
	u32 offset = 0;

	for (i = 0; i < segment_num; i++) {
		if ((offset + sizeof(u32) + sizeof(u32) + sizeof(struct devdrv_check_sum)) > buff_len) {
			NPU_DRV_ERR("segment %d, offset: %u, len: %d\n", segment_num, offset, buff_len);
			return;
		}

		NPU_DRV_INFO("segment id: %d: \n", i);
		dev_ts_bin_info.segment[i].offset = *(u32 *)(buf + offset);
		offset += sizeof(u32);
		dev_ts_bin_info.segment[i].len = *(u32 *)(buf + offset);
		offset += sizeof(u32);
		devdrv_get_sha256_check((buf + offset), &dev_ts_bin_info.segment[i].segment_check);
		offset += sizeof(struct devdrv_check_sum);

		NPU_DRV_DEBUG("segment len: %d\n", dev_ts_bin_info.segment[i].len);
	}
}

static int devdrv_judge_segment_invalid(void)
{
	u32 segment_end;
	u32 i;

	for (i = 0; i < dev_ts_bin_info.segment_num; i++) {
		segment_end = dev_ts_bin_info.segment[i].offset + dev_ts_bin_info.segment[i].len;

		if (segment_end > dev_ts_bin_info.fw_data_len) {
			NPU_DRV_ERR("segment %d is invalid, "
				"offset: %d, len: %d, firmware len: %d\n",
				i, dev_ts_bin_info.segment[i].offset,
				dev_ts_bin_info.segment[i].len,
				dev_ts_bin_info.fw_data_len);
			return 1;
		}
	}

	return 0;
}

int devdrv_judge_bin_validity(struct file *fp, loff_t *fsize, loff_t *pos, u32 *ts_check_file)
{
	struct devdrv_check_sum check;
	loff_t read_pos = 0;
	ssize_t read_num;
	u32 fw_len;
	u32 fw_len_allign;
	u32 segment_num;
	int ret = memset_s(g_bin_valid_buf, BIN_VALID_BUFF_LEN, 0, BIN_VALID_BUFF_LEN);
	COND_RETURN_ERROR(ret != 0, -1, "memset_s failed. ret=%d\n", ret);

	read_num = devdrv_read_file(fp, g_bin_valid_buf, sizeof(u32) * 2, read_pos, BIN_VALID_BUFF_LEN);
	COND_RETURN_ERROR(read_num < (ssize_t)(sizeof(u32) * 2), -1, "vfs_read failed\n");

	NPU_DRV_DEBUG("firmware is a TS firmware\n");
	fw_len = *(u32 *)(g_bin_valid_buf + sizeof(u32));

	COND_RETURN_ERROR(fw_len == 0 || fw_len >= (*fsize - sizeof(u32) * 2), -1,
		"TS firmware length is invalid: %d\n", fw_len);

	NPU_DRV_DEBUG("TS firmware length: %d\n", fw_len);

	dev_ts_bin_info.ts_check_file = 1;
	*ts_check_file = 1;
	*fsize = fw_len;
	*pos = 8;

	fw_len_allign = 4 - (fw_len % 4);
	fw_len_allign = (fw_len % 4 > 0) ? (fw_len + fw_len_allign) : fw_len;

	NPU_DRV_DEBUG("fw_len_allign length: %d\n", fw_len_allign);

	/* get segment info */
	read_pos = 8 + fw_len_allign;

	read_num = devdrv_read_file(fp, g_bin_valid_buf, sizeof(struct devdrv_check_sum) +
		sizeof(u32) * 2, read_pos, BIN_VALID_BUFF_LEN);
	COND_RETURN_ERROR(read_num < (int)(sizeof(struct devdrv_check_sum) + sizeof(u32) * 2), -1, "vfs_read failed\n");

	segment_num = *(u32 *)g_bin_valid_buf;
	COND_RETURN_ERROR(segment_num > DEVDRV_TS_BIN_MAX_SEGMENT_NUM, -1, "too much segments: %d\n", segment_num);

	NPU_DRV_DEBUG("segment number: %d\n", segment_num);

	devdrv_get_sha256_check((g_bin_valid_buf + 2 * sizeof(u32)), &check);

	read_pos = 8 + fw_len_allign + 2 * sizeof(u32) + sizeof(struct devdrv_check_sum);
	read_num = devdrv_read_file(fp, g_bin_valid_buf,
		(long)(unsigned)sizeof(struct devdrv_ts_bin_segment) * segment_num,
		read_pos, BIN_VALID_BUFF_LEN);
	COND_RETURN_ERROR(read_num < (int)(sizeof(struct devdrv_ts_bin_segment) * segment_num), -1, "vfs_read failed\n");

	memcpy(&dev_ts_bin_info.fw_data_check, &check, sizeof(struct devdrv_check_sum));
	dev_ts_bin_info.fw_data_len = fw_len;
	dev_ts_bin_info.segment_num = segment_num;
	devdrv_get_segment_info(g_bin_valid_buf, segment_num, BIN_VALID_BUFF_LEN);

	COND_RETURN_ERROR(devdrv_judge_segment_invalid(), -1, "devdrv_is_segment_invalid return error\n");

	return 0;
}

bool devdrv_load_firmware(const struct file *fp, loff_t fsize, loff_t pos, u32 type)
{
	int ret;
	char *fw_load_addr = NULL;
	u64 max_size = (fsize > TS_FW_REMAP_SIZE) ? fsize : TS_FW_REMAP_SIZE;
	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info failed\r\n");
		return false;
	}

	NPU_DRV_DEBUG("devdrv_load_firmware\n ");
	if (type == DEVDRV_FW_TYPE_AICPU) {
		max_size = fsize;
		fw_load_addr = ioremap_nocache(devdrv_get_firmware_phy_addr(DEVDRV_FW_TYPE_AICPU), max_size);
	} else {
		if (fsize > TS_FW_MAX_SIZE * 4) {
			NPU_DRV_ERR("ts firmware size %d is too large\r\n", (u32)fsize);
			return false;
		}
		fw_load_addr = ioremap_nocache(devdrv_get_firmware_phy_addr(DEVDRV_FW_TYPE_TS), max_size);
	}

	if (fw_load_addr == NULL) {
		NPU_DRV_ERR("ioremap_nocache failed\n");
		return false;
	}
	NPU_DRV_DEBUG("max_size = 0x%llx ,fw_load_addr = %pK , fw_phy_addr = 0x%llx\n ",
		max_size, fw_load_addr, devdrv_get_firmware_phy_addr(DEVDRV_FW_TYPE_TS));
	NPU_DRV_DEBUG("pos %d type %d \n ", (u32)pos, type);

	ret = DEVDRV_PLAT_GET_RES_FW_PROC(plat_info)(fp, fsize, pos, (u64)(uintptr_t)fw_load_addr);
	if (ret != true) {
		NPU_DRV_ERR("npu_fw_proc failed, fsize is %d\r\n", (u32)fsize);
		iounmap(fw_load_addr);
		return false;
	}

	iounmap(fw_load_addr);
	return true;
}

int devdrv_load_cpu_fw_by_type(const char *firmware_path, u32 type)
{
	int ret = -1;
	loff_t fsize;
	struct file *fp = NULL;

	NPU_DRV_DEBUG("firmware_path: %p, type = %d \n", firmware_path, type);

	if (firmware_path == NULL) {
		NPU_DRV_ERR("firmware path is null\n");
		return -1;
	}

	if (type > DEVDRV_FW_TYPE_TS) {
		NPU_DRV_ERR("firmware type is invalid\n");
		return -1;
	}

	fsize = devdrv_get_file_size(firmware_path);
	NPU_DRV_DEBUG("firmware size is : %lld\n", fsize);
	if (fsize == NPU_FIRMWARE_GET_FILE_SIZE_FAIL) {
		NPU_DRV_ERR("get file size failed\n");
		return -1;
	}

	fp = filp_open(firmware_path, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		NPU_DRV_ERR("can't open firmware file\n");
		return -1;
	}
	// load img offset 0x80
	if (devdrv_load_firmware(fp, fsize, 0x80, type) == false) {
		NPU_DRV_ERR("can't load firmware file %d\n", type);
		filp_close(fp, NULL);
		return ret;
	}

	filp_close(fp, NULL);
	return 0;
}

int devdrv_load_cpu_fw(void)
{
	int ret;
	static u32 s_ts_fw_load_flag = 0;
	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_info failed\n");
		return -1;
	}

	if (DEVDRV_PLAT_GET_AICPU_MAX(plat_info) != 0) {
		ret = devdrv_load_cpu_fw_by_type(DEVDRV_AICPU_BINARY_PATH, DEVDRV_FW_TYPE_AICPU);
		if (ret != 0) {
			NPU_DRV_ERR("load ai cpu fw failed\n");
			return ret;
		}
	}

	if (s_ts_fw_load_flag == 0) {
		NPU_DRV_BOOT_TIME_TAG("start LOAD TS FW \n");
		if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info, DEVDRV_FEATURE_HWTS) == 1
			&& strstr(DEVDRV_PLAT_GET_CHIPTYPE(plat_info), "_es") != 0) {
			NPU_DRV_DEBUG("chiptype = %s\n", DEVDRV_PLAT_GET_CHIPTYPE(plat_info));
			ret = devdrv_load_cpu_fw_by_type(DEVDRV_NPU_FW_ES_IMA_PATH, DEVDRV_FW_TYPE_TS);
		} else {
			ret = devdrv_load_cpu_fw_by_type(DEVDRV_NPU_FW_IMA_PATH, DEVDRV_FW_TYPE_TS);
		}
		NPU_DRV_BOOT_TIME_TAG("END LOAD TS FW \n");
		if (ret != 0) {
			NPU_DRV_ERR("load ts cpu fw failed\n");
			/* bbox : load ts cpu fw failed */
			rdr_system_error((u32)RDR_EXC_TYPE_TS_INIT_EXCEPTION, 0, 0);
			return ret;
		}
	}

	return 0;
}

void devdrv_get_firmware_mem_desc(void)
{
	struct devdrv_platform_info* plat_info = NULL;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		NPU_DRV_ERR("devdrv_plat_get_irq failed\n");
		return;
	}

	fw_mem_desc[DEVDRV_FW_TYPE_AICPU] = DEVDRV_PLAT_GET_AIFW_BUF(plat_info);
	fw_mem_desc[DEVDRV_FW_TYPE_TS] = DEVDRV_PLAT_GET_TSFW_BUF(plat_info);
	return;
}

u32 devdrv_get_firmware_remap_size(int type)
{
	if (fw_mem_desc[type] == NULL)
		devdrv_get_firmware_mem_desc();

	return fw_mem_desc[type]->len;
}

u64 devdrv_get_firmware_phy_addr(int type)
{
	if (type >= DEVDRV_FW_TYPE_MAX) {
		NPU_DRV_ERR("devdrv_plat_get_irq failed\n");
		return 0;
	}

	if (fw_mem_desc[type] == NULL)
		devdrv_get_firmware_mem_desc();

	return (u64)fw_mem_desc[type]->base;
}
EXPORT_SYMBOL(devdrv_get_firmware_phy_addr);

