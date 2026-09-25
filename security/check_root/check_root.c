/*
 * check_root.c
 *
 * check_root proc and funcitons
 *
 * Copyright (c) 2001-2019, Huawei Tech. Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <chipset_common/security/check_root.h>
#include <chipset_common/security/saudit.h>
#include <linux/fs.h>
#include <linux/cred.h>
#include <linux/statfs.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

#define ANDROID_THIRD_PART_APK_UID 10000
#define AID_SHELL 2000
#define HOSTS_PATH "/etc/hosts"
#define HOSTS_ENT_LOCAL "localhost"
#define HOSTS_ENT_LOCAL6 "ip6-localhost"
#define HOSTS_ENT_SIZE 128

static struct checkroot_ref_cnt checkroot_ref;

uint get_setids_state(void)
{
	unsigned int ids = 0;

	if (checkroot_ref.setuid)
		ids |= CHECKROOT_SETUID_FLAG;
	if (checkroot_ref.setgid)
		ids |= CHECKROOT_SETGID_FLAG;
	if (checkroot_ref.setresuid)
		ids |= CHECKROOT_SETRESUID_FLAG;
	if (checkroot_ref.setresgid)
		ids |= CHECKROOT_SETRESGID_FLAG;

	return ids;
}

static int checkroot_risk_id(int curr_id, unsigned int flag)
{
	const struct cred *now = current_cred();

	if ((curr_id < ANDROID_THIRD_PART_APK_UID) && (curr_id != AID_SHELL))
		return 0;
	if (now)
		pr_emerg("check_root:Uid %d,Gid %d,try to Privilege Escalate\n",
			now->uid.val, now->gid.val);

#ifdef CONFIG_CHECKROOT_FORCE_STOP
	if (curr_id >= ANDROID_THIRD_PART_APK_UID) {
		force_sig(SIGKILL, current);
		return -1;
	}
#endif
	if (flag & CHECKROOT_SETUID_FLAG)
		checkroot_ref.setuid++;
	if (flag & CHECKROOT_SETGID_FLAG)
		checkroot_ref.setgid++;
	if (flag & CHECKROOT_SETRESUID_FLAG)
		checkroot_ref.setresuid++;
	if (flag & CHECKROOT_SETRESGID_FLAG)
		checkroot_ref.setresgid++;

	return 0;
}

int checkroot_setuid(uid_t uid)
{
	return checkroot_risk_id((int)uid, CHECKROOT_SETUID_FLAG);
}

int checkroot_setgid(gid_t gid)
{
	return checkroot_risk_id((int)gid, CHECKROOT_SETGID_FLAG);
}

int checkroot_setresuid(uid_t uid)
{
	return checkroot_risk_id((int)uid, CHECKROOT_SETRESUID_FLAG);
}

int checkroot_setresgid(gid_t gid)
{
	return checkroot_risk_id((int)gid, CHECKROOT_SETRESGID_FLAG);
}

static int checkroot_readline(struct file *filp, char *buf, int len)
{
	int ret;
	int read_len = 0;
	mm_segment_t old_fs;

	if (!filp)
		return -EINVAL;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	for (; read_len < len; buf++, read_len++) {
		ret = vfs_read(filp, buf, 1, &filp->f_pos);
		if (ret <= 0)
			break;
		if (*buf == '\n') {
			*buf = 0;
			read_len++;
			break;
		}
	}
	set_fs(old_fs);
	return (ret < 0) ? ret : read_len;
}

static int checkroot_check_hosts(char *info, uint info_len, uint offset)
{
	struct file *filp = NULL;
	char line[HOSTS_ENT_SIZE] = {0};
	int status = STP_SAFE;

	filp = filp_open(HOSTS_PATH, O_RDONLY, 0);
	if (IS_ERR_OR_NULL(filp)) {
		pr_err("access %s failed with error %ld\n",
			HOSTS_PATH, PTR_ERR(filp));
		return status;
	}

	while (checkroot_readline(filp, line, sizeof(line)) > 0) {
		if (!strstr(line, HOSTS_ENT_LOCAL) &&
			!strstr(line, HOSTS_ENT_LOCAL6)) {
			pr_err("unknown host %s\n", line);
			status = STP_RISK;
			if (info &&
			    ((strlen(line) + strlen(info) + 1) < info_len)) {
				(void)strcat(info, line);
				(void)strcat(info, " ");
			}
		}
	}
	filp_close(filp, NULL);
	return status;
}

static int stp_keyfiles_trigger(void)
{
	int status;
	char *info = NULL;

	info = kzalloc(SAUDIT_BUFSIZ, GFP_KERNEL);
	if (!info)
		return -1;

	status = checkroot_check_hosts(info, SAUDIT_BUFSIZ, 0);
	saudit_log(KEY_FILES, status, 0, "hosts=%s", info);
	kfree(info);
	info = NULL;
	return 0;
}

static int __init checkroot_init(void)
{
	(void)memset(&checkroot_ref, 0, sizeof(checkroot_ref));
	(void)kernel_stp_scanner_register(stp_keyfiles_trigger);
	return 0;
}

static void __exit checkroot_exit(void)
{
}

module_init(checkroot_init);
module_exit(checkroot_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("provides check_root functions");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
