/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hisi_eis_data_archive.c
 *
 * eis core io interface header
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

#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <partition_macro.h>
#include <securec.h>
#include <uapi/linux/ioctl.h>
#include <linux/hisi/partition_ap_kernel.h>
#include <linux/power/hisi/coul/coul_event.h>
#include "hisi_eis_core_freq.h"
#include "pmic/hisi_pmic_eis.h"
#include "hisi_eis_data_archive.h"

static int g_freq_rolling_now;
static int g_full_freq_rolling;
#ifdef CONFIG_HISI_DEBUG_FS
static struct eis_freq_info g_single_inf_to_show[LEN_T_FREQ];
static struct eis_freq_info g_latest_inf_to_show[LEN_T_FREQ];
static int g_bat_temp = 25; /* 25: [10, 45] is valid range */
#endif /* CONFIG_HISI_DEBUG_FS */
static char g_buff[EIS_FREQ_PARAM_SIZE] = {0};

int eis_get_g_freq_rolling_now(void)
{
	return g_freq_rolling_now;
}

int eis_get_g_full_freq_rolling(void)
{
	return g_full_freq_rolling;
}

static void eis_set_g_freq_rolling_now(int val)
{
	if (val < 0 || val >= EIS_FREQ_ROLLING)
		val = 0;

	g_freq_rolling_now = val;

	hisi_eis_inf("g_freq_rolling_now set to %d\n", g_freq_rolling_now);
}

static void eis_set_g_full_freq_rolling(int val)
{
	if (val < 0)
		val = 0;

	g_full_freq_rolling = val;

	hisi_eis_inf("g_full_freq_rolling set to %d\n", g_full_freq_rolling);
}

static int hisi_eis_partition_ready(void)
{
	char p_name[EIS_BUF_PATH_LEN + 1] = {0};
	int ret;

	ret = flash_find_ptn_s(PART_BATT_TP_PARA, p_name, sizeof(p_name));
	if (ret != 0) {
		hisi_eis_err("flash_find_ptn_s failed!\n");
		return -1;
	}
	p_name[EIS_BUF_PATH_LEN] = '\0';
	if (sys_access(p_name, 0) != 0) {
		hisi_eis_err("name:%s\n", p_name);
		return -1;
	}
	return 0;
}

static int hisi_eis_flash_open(int flags)
{
	char p_name[EIS_BUF_PATH_LEN + 1] = {0};
	int ret, fd_dfx;

	ret = hisi_eis_partition_ready();
	if (ret != 0) {
		hisi_eis_err("ret %d\n", ret);
		return -1;
	}

	ret = flash_find_ptn_s(PART_BATT_TP_PARA, p_name, sizeof(p_name));
	if (ret != 0) {
		hisi_eis_err("find ptn err\n");
		return -1;
	}
	p_name[EIS_BUF_PATH_LEN] = '\0';
	fd_dfx = sys_open(p_name, flags, EIS_FILE_LIMIT);
	if (fd_dfx < 0)
		hisi_eis_err("sys open err\n");

	return fd_dfx;
}

int hisi_eis_read_flash_data(void *buf, u32 buf_size, u32 flash_offset)
{
	int ret, fd_flash;
	int cnt = 0;
	mm_segment_t old_fs;

	if (buf == NULL || buf_size == 0) {
		hisi_eis_err("null ptr err\n");
		return 0;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd_flash = hisi_eis_flash_open(O_RDONLY);
	if (fd_flash >= 0) {
		ret = sys_lseek(fd_flash, flash_offset, SEEK_SET);
		if (ret < 0) {
			hisi_eis_err("ret=%d, flash_offset=%x\n", ret,
				flash_offset);
			goto close;
		}
		cnt = sys_read(fd_flash, buf, buf_size);
		if (cnt != buf_size) {
			hisi_eis_err("cnt=%d\n", cnt);
			goto close;
		}
	} else {
		hisi_eis_err("fd_flash=%d\n", fd_flash);
		set_fs(old_fs);
		return fd_flash;
	}
close:
	sys_close(fd_flash);
	set_fs(old_fs);
	return cnt;
}

static void hisi_eis_write_flash_data(
	const void *p_buf, u32 buf_size, u32 flash_offset)
{
	int ret, fd_flash, cnt;
	mm_segment_t old_fs;

	if (p_buf == NULL) {
		hisi_eis_err("null ptr\n");
		return;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd_flash = hisi_eis_flash_open(O_RDWR | O_SYNC);
	if (fd_flash >= 0) {
		ret = sys_lseek(fd_flash, flash_offset, SEEK_SET);
		if (ret < 0) {
			hisi_eis_err("ret=%d\n", ret);
			goto close;
		}
		cnt = sys_write(fd_flash, p_buf, buf_size);
		if (cnt != buf_size) {
			hisi_eis_err("cnt=%d\n", cnt);
			goto close;
		}
	} else {
		hisi_eis_err("flash open failed\n");
		set_fs(old_fs);
		return;
	}
close:
	sys_close(fd_flash);
	set_fs(old_fs);
}

void hisi_eis_freq_save_index_to_flash(void)
{
	const size_t size = sizeof(int);
	int tmp_1 = -1;
	int tmp_2 = -1;
	int freq_inx_magic = FREQ_INDEX_MAGIC;

	hisi_eis_inf("g_freq_rolling_now = %d\n", g_freq_rolling_now);
	hisi_eis_inf("g_full_freq_rolling = %d\n", g_full_freq_rolling);
	hisi_eis_write_flash_data(&freq_inx_magic, size, FREQ_INDEX_OFFSET);
	hisi_eis_write_flash_data(&g_freq_rolling_now, size,
		FREQ_INDEX_OFFSET + size);
	hisi_eis_write_flash_data(&g_full_freq_rolling, size,
		FREQ_INDEX_OFFSET + 2 * size); /* 2: flash offset value of  full_freq_rolling */
	hisi_eis_read_flash_data(&tmp_1, size, FREQ_INDEX_OFFSET + size);
	hisi_eis_read_flash_data(&tmp_2, size, FREQ_INDEX_OFFSET + 2 * size); /* 2: flash offset value of  full_freq_rolling */
	hisi_eis_debug("tmp_1 = %d\n", tmp_1);
	hisi_eis_debug("tmp_2 = %d\n", tmp_2);
}

void hisi_eis_freq_read_index_from_flash(void)
{
	const size_t size = sizeof(int);
	int rolling_now = 0;
	int rolling_full = 0;
	int freq_inx_magic = 0;

	hisi_eis_read_flash_data(&freq_inx_magic, size, FREQ_INDEX_OFFSET);
	hisi_eis_read_flash_data(&rolling_now, size, FREQ_INDEX_OFFSET + size);
	hisi_eis_read_flash_data(&rolling_full, size,
		FREQ_INDEX_OFFSET + 2 * size); /* 2: flash offset value of  full_freq_rolling */
	hisi_eis_inf("magic num = %d\n", freq_inx_magic);
	hisi_eis_inf("rolling_now = %d\n", rolling_now);
	hisi_eis_inf("rolling_full = %d\n", rolling_full);
	if (freq_inx_magic != FREQ_INDEX_MAGIC) {
		hisi_eis_err("magic num = %d, not match, set rolling idx to 0\n",
			freq_inx_magic);
		rolling_now = 0;
		rolling_full = 0;
	}

	eis_set_g_freq_rolling_now(rolling_now);
	eis_set_g_full_freq_rolling(rolling_full);
}

void hisi_eis_freq_write_info_to_flash(struct eis_freq_info *freq_inf)
{
	int i;
	const size_t size = LEN_T_FREQ * sizeof(struct eis_freq_info);
	struct eis_freq_device *di = NULL;

	if (freq_inf == NULL) {
		hisi_eis_err("null ptr err\n");
		return;
	}
	di = container_of(freq_inf, struct eis_freq_device, freq_info[0]);
	if (di == NULL) {
		hisi_eis_err("null ptr err\n");
		return;
	}

	i = g_freq_rolling_now;
	hisi_eis_write_flash_data(freq_inf, size, EIS_FREQ_OFFSET + i * size);
	g_freq_rolling_now += 1;
	if (g_freq_rolling_now == EIS_FREQ_ROLLING) {
		g_freq_rolling_now = 0;
		g_full_freq_rolling = (g_full_freq_rolling + 1) % EIS_MAX_INT;
	}
	hisi_eis_freq_save_index_to_flash();
}

void hisi_eis_freq_info_printk(struct eis_freq_info *freq_info)
{
	int i;
	int offset = 0;
	int ret;
	const int str_len = EIS_FREQ_PARAM_SIZE - 1;

	if (freq_info == NULL) {
		hisi_eis_err("null ptr error\n");
		return;
	}
	hisi_eis_inf("\n%s %s %s %s %s %s %s %s\n", "eis_freq", "sample_valid",
		"eis_temp", "eis_cur", "eis_soc", "eis_cycle", "eis_real",
		"eis_imag");

	for (i = 0; i < EIS_FREQ_ROW; i++) {
		ret = snprintf_s(g_buff + offset, str_len - offset,
			EIS_FREQ_LINE_WIDTH, "\n%-5d %-d %-3d %-3d %-3d %-5d %-10ld %-10ld",
			freq_info[i].freq, freq_info[i].sample_valid,
			freq_info[i].temp, freq_info[i].cur,
			freq_info[i].soc, freq_info[i].cycle,
			freq_info[i].real, freq_info[i].imag);
		if (ret < 0) {
			hisi_eis_err("%dth ret = %d, %s\n", i, ret, g_buff);
			return;
		}
		offset += ret;
	}
	hisi_eis_inf("%s\n", g_buff);
}

void hisi_eis_freq_read_single_info_from_flash(
	struct eis_freq_info *freq_inf, int idex)
{
	const size_t size = LEN_T_FREQ * sizeof(struct eis_freq_info);

	if (freq_inf == NULL) {
		hisi_eis_err("null ptr err\n");
		return;
	}
	if (idex < 0 || idex > EIS_FREQ_ROLLING - 1 || freq_inf == NULL) {
		hisi_eis_err("index = %d overflow\n", idex);
		return;
	}
	hisi_eis_read_flash_data(freq_inf, size, EIS_FREQ_OFFSET + idex * size);
	hisi_eis_inf("begin to print\n");
	hisi_eis_freq_info_printk(freq_inf);
}

/* in this method, the buffer size must be at least EIS_FREQ_ROLLING * size */
void hisi_eis_freq_read_all_info_from_flash(
	struct eis_freq_info (*freq_inf)[LEN_T_FREQ])
{
	int i;
	int j = 0;
	const size_t size = LEN_T_FREQ * sizeof(struct eis_freq_info);

	if (freq_inf == NULL) {
		hisi_eis_err("null ptr err\n");
		return;
	}
	if (g_freq_rolling_now > 0 || g_full_freq_rolling > 0) {
		for (i = g_freq_rolling_now - 1; i == g_freq_rolling_now; i--) {
			/* when going before head, jump to end */
			if (i < 0)
				i = EIS_FREQ_ROLLING - 1;
			hisi_eis_read_flash_data(freq_inf[j], size,
				EIS_FREQ_OFFSET + i * size);
			hisi_eis_inf("begin to print %dth record\n", j);
			hisi_eis_freq_info_printk(freq_inf[j]);
			j++;
		}
	} else {
		for (i = 0; i <= g_freq_rolling_now; i++) {
			hisi_eis_read_flash_data(freq_inf[j], size,
				EIS_FREQ_OFFSET + i * size);
			hisi_eis_inf("begin to print %dth record\n", j);
			hisi_eis_freq_info_printk(freq_inf[j]);
			j++;
		}
	}
}

#ifdef CONFIG_HISI_DEBUG_FS
int show_numed_idx_info_and_cycle_to_shell(int idx)
{
	int index;
	/*
	 * the allowed appointed idx is [0, EIS_FREQ_ROLLING - 1],
	 * if not in this range, show newest one
	 */
	if (idx < 0 || idx > EIS_FREQ_ROLLING - 1) {
		if (g_freq_rolling_now > 0 || g_full_freq_rolling > 0) {
			index = g_freq_rolling_now - 1;
			if (index < 0)
				index = EIS_FREQ_ROLLING - 1;
		} else {
			index = g_freq_rolling_now;
		}
	} else {
		index = idx;
	}

	hisi_eis_freq_read_single_info_from_flash(g_single_inf_to_show, index);
	hisi_eis_inf("show_latest_single_info_from_flash_to_shell\n");
	hisi_eis_freq_info_printk(g_single_inf_to_show);
	return g_single_inf_to_show[LEN_T_FREQ - 1].cycle;
}

void set_latest_cycle_inc_to_flash_from_shell(int inc)
{
	int i;
	int index;

	if (g_freq_rolling_now > 0 || g_full_freq_rolling > 0) {
		index = g_freq_rolling_now - 1;
		if (index < 0)
			index = EIS_FREQ_ROLLING - 1;
	} else {
		index = g_freq_rolling_now;
	}

	hisi_eis_freq_read_single_info_from_flash(g_latest_inf_to_show, index);
	for (i = 0; i < LEN_T_FREQ; i++)
		g_latest_inf_to_show[i].cycle =
			(g_latest_inf_to_show[i].cycle + inc) % EIS_MAX_INT;
	hisi_eis_freq_write_info_to_flash(g_latest_inf_to_show);
	hisi_eis_inf("cycle increased %d, show latest info to shell\n", inc);
	hisi_eis_freq_info_printk(g_latest_inf_to_show);
}

void hisi_eis_freq_clear_flash_data(void)
{
	void *p_buf = NULL;
	void *p_buf_idx = NULL;
	/* 3 numbers: rolling magic, now and full */
	int size_of_idx = sizeof(int) * 3;

	p_buf = kzalloc(EIS_FREQ_SIZE, GFP_KERNEL);
	if (p_buf == NULL)
		return;

	p_buf_idx = kzalloc(size_of_idx, GFP_KERNEL);
	if (p_buf_idx == NULL) {
		kfree(p_buf);
		return;
	}
	hisi_eis_write_flash_data(p_buf, EIS_FREQ_SIZE, EIS_FREQ_OFFSET);
	hisi_eis_write_flash_data(p_buf_idx, size_of_idx, FREQ_INDEX_OFFSET);
	g_freq_rolling_now = 0;
	g_full_freq_rolling = 0;
	kfree(p_buf);
	kfree(p_buf_idx);
}

void test_freq_task_schedule(void)
{
	struct hisi_eis_device *di = get_g_eis_di();

	if (di == NULL) {
		hisi_eis_err("g_eis_di is null\n");
		return;
	}
	call_coul_blocking_notifiers(BATT_EIS_FREQ, NULL);
}

int test_eis_set_temp(int temp)
{
	g_bat_temp = temp;

	return g_bat_temp;
}

int get_test_bat_temp(void)
{
	return g_bat_temp;
}

#endif /* CONFIG_HISI_DEBUG_FS */

