/*
 * zrhung_wp_sochalt.c
 *
 * Reporting SR freeze screen events
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#include "zrhung_wp_sochalt.h"

#include <linux/sched.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/vmalloc.h>
#include <linux/aio.h>
#include <uapi/linux/uio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>

#include <chipset_common/hwzrhung/zrhung.h>
#include "../zrhung_common.h"
#include <chipset_common/hwerecovery/erecovery.h>

static int is_soc_halt;
static int is_longpress;
static int is_coldboot;

#define SOCHALT_INFO "BR_PRESS_10S"
#define ZRHUNG_POWERKEY_LONGPRESS_EVENT "AP_S_PRESS6S"
#define ZRHUNG_RECOVERY_COLDBOOT "COLDBOOT"
#define ZRHUNG_VMWTG_ERECOVERYID 401006000
#define ZRHUNG_VMWTG_FAULTID 901004000
#define SR_POSITION_KEYWORD "sr position:"
#define FASTBOOT_LOG_PATH "/proc/balong/log/fastboot_log"
#define BUFFER_SIZE_FASTBOOT (4 * 1024) /* 4KB buffer for reading fastboot log */

static int __init wp_reboot_reason_cmdline(char *reboot_reason_cmdline)
{
	if (!strcmp(reboot_reason_cmdline, SOCHALT_INFO)) {
		pr_err("%s %d: LONGPRESS10S_EVENT report zerohung\n",
		     __func__, __LINE__);
#ifndef CONFIG_MTK_PLATFORM
		pr_err("%s %d: LONGPRESS10S_EVENT report zerohung. not mtk\n",
		     __func__, __LINE__);
		is_soc_halt = 1;
#endif
		return 0;
	}
	if (!strcmp(reboot_reason_cmdline, ZRHUNG_POWERKEY_LONGPRESS_EVENT)) {
		pr_err("%s %d: LONGPRESS6S_EVENT report zerohung\n",
		     __func__, __LINE__);
		is_longpress = 1;
	} else if (!strcmp(reboot_reason_cmdline, ZRHUNG_RECOVERY_COLDBOOT)) {
		pr_err("%s %d: COLDBOOT, maybe VMWTG-surfaceflinger recovery\n",
		     __func__, __LINE__);
		is_coldboot = 1;
	}
	return 0;
}

early_param("reboot_reason", wp_reboot_reason_cmdline);

void zrhung_report_endrecovery(void)
{
#ifdef CONFIG_HW_ERECOVERY
	struct erecovery_eventobj rp;
	long ret;

	if (!is_coldboot)
		return;

	memset(&rp, 0, sizeof(rp));
	rp.erecovery_id = ZRHUNG_VMWTG_ERECOVERYID;
	rp.fault_id = ZRHUNG_VMWTG_FAULTID;
	rp.state = EVENT_END;
	ret = erecovery_report(&rp);
	if (ret)
		return;
	pr_err("%s %d: VMWTG-surfaceflinger recovery end report failed\n",
	       __func__, __LINE__);
#endif
}

void zrhung_get_longpress_event(void)
{
	if (is_soc_halt || is_longpress) {
		pr_err("%s %d: POWERKEY_LONGPRESS_EVENT send to zerohung\n",
		       __func__, __LINE__);
		zrhung_send_event(ZRHUNG_EVENT_LONGPRESS, NULL, NULL);
	}
}

int wp_get_sochalt(struct zrhung_write_event *we)
{
	if (!we) {
		pr_err("%s %d: param error\n", __func__, __LINE__);
		return -1;
	}

	memset(we, 0, sizeof(*we));

	if (is_soc_halt) {
		we->magic = MAGIC_NUM;
		we->len = sizeof(*we);
		we->wp_id = ZRHUNG_WP_SR;

		pr_err("%s %d: soc halt\n", __func__, __LINE__);
	}

	return 0;
}

void get_sr_position_from_fastboot(char *dst, unsigned int max_dst_size)
{
	mm_segment_t old_fs;
	char *reading_buf = NULL;
	char *plog = NULL;
	int fd = -1;
	unsigned int i;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd = sys_open(FASTBOOT_LOG_PATH, O_RDONLY, 0);
	if (fd < 0) {
		pr_err("%s %d: fail to open fastbootlog\n", __func__, __LINE__);
		goto out;
	}

	reading_buf = kzalloc(BUFFER_SIZE_FASTBOOT + 1, GFP_KERNEL);
	if (reading_buf == NULL)
		goto out;

	while (sys_read(fd, reading_buf, BUFFER_SIZE_FASTBOOT) > 0) {
		plog = strstr(reading_buf, SR_POSITION_KEYWORD);
		if (!plog)
			continue;

		if ((BUFFER_SIZE_FASTBOOT - (plog - reading_buf)) < max_dst_size) {
			strncpy(dst, plog, BUFFER_SIZE_FASTBOOT - (plog - reading_buf));
			break;
		}

		i = 0;
		while ((plog[i] != '\r') && (plog[i] != '\n') && (i < max_dst_size)) {
			dst[i] = plog[i];
			i++;
		}
		dst[max_dst_size - 1] = 0;
		break;
	}

out:
	if (fd >= 0)
		sys_close(fd);

	if (reading_buf != NULL)
		kfree(reading_buf);

	set_fs(old_fs);
}
