/*
 * hw_kcollect.c
 *
 * This file use to collect kernel state and report it
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/power/hw_kcollect.h>
#include <securec.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/time.h>
#include <huawei_platform/power/hw_kstate.h>

struct kcollect_info {
	s32 mask;
	u32 tv_sec; /* seconds */
	u32 tv_usec; /* microseconds */
	u32 len;
	char buffer[KCOLLECT_BUFFER_SIZE];
};

static int switch_mask = 0;
static int killed_pid = -1;

/*
 * kstate call back
 * @return 0 for success
 */
static int kcollect_cb(channel_id src, packet_tag tag, const char *data, size_t len)
{
	int mask = 0;

	if (IS_ERR_OR_NULL(data) || (len != sizeof(mask))) {
		pr_err("hw_kcollect %s: invalid data or len:%d\n", __func__, (int)len);
		return -1;
	}

	if (memcpy_s(&mask, sizeof(mask), data, len) != EOK) {
		pr_err("hw_kcollect %s: failed to memcpy_s\n", __func__);
		return -1;
	}

	switch_mask = mask;

	pr_debug("hw_kcollect %s: src=%d tag=%d len=%d pg_switch_mask=%d\n", __func__, src, tag, (int)len, switch_mask);

	return 0;
}

static struct kstate_opt kcollect_opt = {
	.name = "kcollect",
	.tag = PACKET_TAG_KCOLLECT,
	.dst = CHANNEL_ID_KCOLLECT,
	.hook = kcollect_cb,
};

/*
 * packet and send data to userspace by kstate
 * @param mask: message mask
 * @return 0 for success
 */
static int report(int mask, va_list args, const char *fmt)
{
	int ret = -1;
	struct kcollect_info info;
	int length;
	size_t info_len;

	if (memset_s(&info, sizeof(info), 0, sizeof(info)) != EOK) {
		pr_err("hw_kcollect %s: failed to memset_s\n", __func__);
		return -1;
	}

	length = vscnprintf(info.buffer, KCOLLECT_BUFFER_SIZE - 1, fmt, args);
	if (length > 0) {
		info.mask = mask;
		info.len = length + 1;
		info.tv_sec = 0;
		info.tv_usec = 0;
		info_len = sizeof(info) - KCOLLECT_BUFFER_SIZE + length + 1;

		ret = kstate(CHANNEL_ID_KCOLLECT, PACKET_TAG_KCOLLECT, (char*)&info, info_len);
		if (ret < 0) {
			pr_err("hw_kcollect %s: kstate error\n", __func__);
			ret = -1;
		}
	}

	pr_debug("hw_kcollect %s: length=%d mask=%d\n", __func__, length, mask);

	return ret;
}

int hwbinderinfo(int callingpid, int calledpid)
{
	return kcollect(KCOLLECT_BINDERCALL_MASK, "[TPID %d CALLED][PID %d]", calledpid, callingpid);
}

int hwkillinfo(int pid, int seg)
{
	int ret = -1;

	if (killed_pid != pid) {
		ret = kcollect(KCOLLECT_FREEZER_MASK, "[PID %d KILLED][SIG %d]", pid, seg);
		killed_pid = pid;
	}

	return ret;
}

int hw_packet_cb(int uid, int pid)
{
	return kcollect(KCOLLECT_NETPACKET_MASK, "[PID %d NET][UID %d]", pid, uid);
}

/*
 * collect the data and report system
 * @param mask: message mask
 * @return 0 for success
 */
int kcollect(int mask, const char *fmt, ...)
{
	va_list args;
	int ret = -1;

	if (mask & switch_mask) {
		va_start(args, fmt);
		ret = report(mask, args, fmt);
		va_end(args);
	}

	return ret;
}

static int __init kcollect_init(void)
{
	return kstate_register_hook(&kcollect_opt);
}

static void __exit kcollect_exit(void)
{
	kstate_unregister_hook(&kcollect_opt);
}

late_initcall(kcollect_init);
module_exit(kcollect_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("collect kernel state and report");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
