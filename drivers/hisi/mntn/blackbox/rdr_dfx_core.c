/*
 * rdr_dfx_core.c
 *
 * blackbox. (kernel run data recorder.)
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

#include <linux/fs.h>
#include <linux/io.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/of.h>

#include <linux/hisi/hisi_bootup_keypoint.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/hisi/rdr_dfx_core.h>
#include <linux/hisi/partition_ap_kernel.h>
#include <pr_log.h>

#include <securec.h>
#include <partition_macro.h>
#include "rdr_print.h"
#include "rdr_inner.h"

#define PR_LOG_TAG BLACKBOX_TAG

int dfx_open(void)
{
	void *buf = NULL;
	char p_name[RDR_PNAME_SIZE];
	int ret;
	int fd_dfx;

	BB_PRINT_START();

	buf = kzalloc(SZ_4K, GFP_KERNEL);
	if (buf == NULL) {
		BB_PRINT_ERR("[%s:%d] kzalloc buf1 fail\n", __func__, __LINE__);
		return -ENOMEM;
	}
	ret = flash_find_ptn_s(PART_DFX, buf, SZ_4K);
	if (ret != 0) {
		BB_PRINT_ERR("[%s:%d] flash_find_ptn_s fail\n", __func__, __LINE__);
		kfree(buf);
		return ret;
	}

	(void)memset_s(p_name, RDR_PNAME_SIZE, 0, RDR_PNAME_SIZE);

	ret = strncpy_s(p_name, RDR_PNAME_SIZE, buf, (RDR_PNAME_SIZE - 1));
	if (ret != EOK) {
		BB_PRINT_ERR("[%s:%d] strncpy_s err\n", __func__, __LINE__);
		kfree(buf);
		return ret;
	}
	p_name[RDR_PNAME_SIZE - 1] = '\0';
	kfree(buf);

	fd_dfx = SYS_OPEN(p_name, O_RDWR, FILE_LIMIT);

	return fd_dfx;
}

int dfx_read(u32 module, void *buffer, u32 size)
{
	int ret;
	int fd_dfx;
	int cnt = 0;
	u32 addr;

	mm_segment_t old_fs = get_fs();

	if (get_dfx_size(module) < size || !get_dfx_size(module))
		return cnt;
	if (buffer == NULL)
		return cnt;
	set_fs(KERNEL_DS);

	fd_dfx = dfx_open();
	addr = get_dfx_addr(module);
	if (addr == 0)
		goto close;
	ret = SYS_LSEEK(fd_dfx, addr, SEEK_SET);
	if (ret < 0) {
		BB_PRINT_ERR("%s():%d:lseek fail[%d]\n", __func__, __LINE__, ret);
		goto close;
	}
	cnt = SYS_READ(fd_dfx, buffer, size);
	if (cnt < 0) {
		BB_PRINT_ERR("%s():%d:read fail[%d]\n", __func__, __LINE__, cnt);
		goto close;
	}
close:
	SYS_CLOSE(fd_dfx);
	set_fs(old_fs);
	return cnt;
}

int dfx_write(u32 module, const void *buffer, u32 size)
{
	int ret;
	int fd_dfx;
	int cnt = 0;
	u32 addr;

	mm_segment_t old_fs = get_fs();

	if (get_dfx_size(module) < size || !get_dfx_size(module))
		return cnt;
	if (buffer == NULL)
		return cnt;
	set_fs(KERNEL_DS);

	fd_dfx = dfx_open();
	addr = get_dfx_addr(module);
	if (addr == 0)
		goto close;
	ret = SYS_LSEEK(fd_dfx, addr, SEEK_SET);
	if (ret < 0) {
		BB_PRINT_ERR("%s():%d:lseek fail[%d]\n", __func__, __LINE__, ret);
		goto close;
	}
	cnt = SYS_WRITE(fd_dfx, buffer, size);
	if (cnt < 0) {
		BB_PRINT_ERR("%s():%d:write fail[%d]\n", __func__, __LINE__, cnt);
		goto close;
	}
close:
	SYS_CLOSE(fd_dfx);
	set_fs(old_fs);
	return cnt;
}
