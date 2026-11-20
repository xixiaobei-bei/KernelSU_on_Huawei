/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: implement the chipsets's common interface of HISI
 * Author: qidechun
 * Create: 2019-03-05
 */

/* ---- includes ---- */
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/statfs.h>
#include <linux/version.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/chipsets/common/bootfail_timer.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>
#include <hwbootfail/core/boot_detector.h>
#ifdef CONFIG_HUAWEI_DYNAMIC_BRD
#include <chipset_common/storage_rofa/dynbrd_public.h>
#endif
#ifdef CONFIG_HUAWEI_STORAGE_ROFA
#include <chipset_common/storage_rofa/storage_rofa.h>
#endif

/* ---- local macroes ---- */
#define BOOT_DETECTOR_TAG "boot_detector_enable"
#define BOOT_RECOVERY_TAG "boot_recovery_enable"
#define BOPD_SUPPORT_TAG "bopd_mem_sp"
#define BOOT_DETECTOR_DEV_NAME "hw_bfm"
#define boot_detector_llseek no_llseek
#define SHA256_DATA_LEN 32
#define BLOCK_CALLING_PROCESS_INTERVAL (0xffffffff)
#define SIG_TO_INIT 40
#define SIG_INT_VALUE 1234
#define BOOT_SLOWLY_TIME 300 /* Unit: second */

/* ---- local prototypes  ---- */
struct thread_param {
	struct bootfail_proc_param pparam;
	struct semaphore sem;
} __packed;

/* ---- local function prototypes ---- */
static long boot_detector_ioctl(struct file *file,
	unsigned int cmd,
	unsigned long arg);
static int boot_detector_open(struct inode *inode, struct file *file);
static ssize_t boot_detector_write(struct file *file,
	const char *data,
	size_t len,
	loff_t *ppos);
static ssize_t boot_detector_read(struct file *file,
	char __user *buf,
	size_t count,
	loff_t *pos);
static int boot_detector_release(struct inode *inode, struct file *file);
static int __process_upper_bootfail(void *pparam);
static int process_bootfail(struct bootfail_proc_param *pparam);

/* ---- local variables ---- */
static int boot_detector_enabled;
static int boot_recovery_enabled;
static int bopd_supported;
static bool enter_recovery;
static bool enter_erecovery;
static bool can_data_be_mounted = true;
static DEFINE_SEMAPHORE(boot_detector_sem);
static DEFINE_SEMAPHORE(detector_ctl_sem);
static const struct file_operations boot_detector_fops = {
	.owner	 = THIS_MODULE,
	.unlocked_ioctl = boot_detector_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = boot_detector_ioctl,
#endif
	.open = boot_detector_open,
	.read = boot_detector_read,
	.write = boot_detector_write,
	.release = boot_detector_release,
	.llseek = boot_detector_llseek,
};

static struct miscdevice boot_detector_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = BOOT_DETECTOR_DEV_NAME,
	.fops = &boot_detector_fops,
};

__weak long ksys_fsync(unsigned int fd)
{
	print_err("please implement ksys_fsync\n");
	return 0;
}

__weak long ksys_statfs(const char __user * path, struct statfs __user *buf)
{
	print_err("please implement ksys_statfs\n");
	return 0;
}

__weak long ksys_newlstat(const char __user *filename, struct stat __user *statbuf)
{
	print_err("please implement ksys_newlstat\n");
	return 0;
}

__weak long ksys_lstat64(const char __user *filename, struct stat64 __user *statbuf)
{
	print_err("please implement ksys_lstat64\n");
	return 0;
}

long bf_sys_read(unsigned int fd, char __user *buf, size_t count)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	return sys_read(fd, buf, count);
#else
#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	return sys_read(fd, buf, count);
#else
	return ksys_read(fd, buf, count);
#endif
#endif
}

long bf_sys_write(unsigned int fd, char __user *buf, size_t count)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	return sys_write(fd, buf, count);
#else
#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	return sys_write(fd, buf, count);
#else
	return ksys_write(fd, buf, count);
#endif
#endif
}

long bf_sys_open(const char __user *filename, int flags, umode_t mode)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	return sys_open(filename, flags, mode);
#else
#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	return sys_open(filename, flags, mode);
#else
	return ksys_open(filename, flags, mode);
#endif
#endif
}

long bf_sys_lseek(unsigned int fd, off_t offset, unsigned int whence)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	return sys_lseek(fd, offset, whence);
#else
#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	return sys_lseek(fd, offset, whence);
#else
	return ksys_lseek(fd, offset, whence);
#endif
#endif
}

long bf_sys_sync(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	return sys_sync();
#else
#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	return sys_sync();
#else
	ksys_sync();
	return 0;
#endif
#endif
}

long bf_sys_close(unsigned int fd)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	return sys_close(fd);
#else
#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	return sys_close(fd);
#else
	return ksys_close(fd);
#endif
#endif
}

long bf_sys_fsync(unsigned int fd)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	return sys_fsync(fd);
#else
#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	return sys_fsync(fd);
#else
	return ksys_fsync(fd);
#endif
#endif
}

long bf_sys_statfs(const char __user * path, struct statfs __user *buf)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	return sys_statfs(path, buf);
#else
#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	return sys_statfs(path, buf);
#else
	return ksys_statfs(path, buf);
#endif
#endif
}

long bf_sys_lstat64(const char __user *filename, struct stat64 __user *statbuf)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	return sys_lstat64(filename, statbuf);
#else
#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	return sys_lstat64(filename, statbuf);
#else
	return ksys_lstat64(filename, statbuf);
#endif
#endif
}

long bf_sys_newlstat(const char __user *filename, struct stat __user *statbuf)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	return sys_newlstat(filename, statbuf);
#else
#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	return sys_newlstat(filename, statbuf);
#else
	return ksys_newlstat(filename, statbuf);
#endif
#endif
}

long bf_lstat(const char __user *filename, void __user *statbuf)
{
#if BITS_PER_LONG == 32
	return bf_sys_lstat64(filename, (struct stat64 __user *)statbuf);
#else
	return bf_sys_newlstat(filename, (struct stat __user *)statbuf);
#endif
}
/* ---- function definitions ---- */
static int __init parse_boot_detector_enable_flag(char *p)
{
	if (p != NULL)  {
		boot_detector_enabled = (strncmp(p, "1", strlen("1")) == 0) ?
			1 : 0;
		print_info(BOOT_DETECTOR_TAG "=%s\n", p);
	}

	return 0;
} early_param(BOOT_DETECTOR_TAG, parse_boot_detector_enable_flag);

static int __init parse_boot_recovery_enable_flag(char *p)
{
	if (p != NULL) {
		boot_recovery_enabled = (strncmp(p, "1", strlen("1")) == 0) ?
			1 : 0;
		print_info(BOOT_RECOVERY_TAG "=%s\n", p);
	}

	return 0;
} early_param(BOOT_RECOVERY_TAG, parse_boot_recovery_enable_flag);

static int __init parse_bopd_support_flag(char *p)
{
	if (p != NULL) {
		if (strncmp(p, "on", strlen("on")) == 0)
			bopd_supported = 1;
	}

	return 0;
} early_param(BOPD_SUPPORT_TAG, parse_bopd_support_flag);

int is_boot_detector_enabled(void)
{
	int boot_detector_enable_flag;

	down_interruptible(&detector_ctl_sem);
	boot_detector_enable_flag = boot_detector_enabled;
	up(&detector_ctl_sem);

	return boot_detector_enable_flag;
}

int is_boot_recovery_enabled(void)
{
#ifndef CONFIG_BOOT_RECOVERY
	return 0;
#else
	return boot_recovery_enabled;
#endif
}

int is_bopd_supported(void)
{
	int bopd_supported_flag;

	down_interruptible(&detector_ctl_sem);
	bopd_supported_flag = bopd_supported;
	up(&detector_ctl_sem);

	return bopd_supported_flag;
}

static int __init early_parse_recovery_cmdline(char *p)
{
	if (p != NULL) {
		enter_recovery = (strncmp(p, "1",
			strlen("1")) == 0) ? true : false;
		print_err("%s mode!\n", enter_recovery ?
			"recovery/erecovery" : "normal");
	}

	return 0;
}
early_param("enter_recovery", early_parse_recovery_cmdline);

bool check_recovery_mode(void)
{
	return enter_recovery;
}

static int __init early_parse_erecovery_cmdline(char *p)
{
	if (p != NULL) {
		enter_erecovery = (strncmp(p, "1",
			strlen("1")) == 0) ? true : false;
		print_err("%s mode!\n", enter_recovery ?
			"recovery/erecovery" : "normal");
	}

	return 0;
}
early_param("enter_erecovery", early_parse_erecovery_cmdline);

bool is_erecovery_mode(void)
{
	return enter_erecovery;
}

static int __init early_parse_data_damaged_cmdline(char *p)
{
	if (p != NULL) {
		can_data_be_mounted = (strncmp(p, "1",
			strlen("1")) == 0) ? false : true;
		print_err("/data is: %s!\n", can_data_be_mounted ?
			"OK" : "damaged");
	}

	return 0;
}
early_param("data_damaged", early_parse_data_damaged_cmdline);

bool data_can_be_mounted(void)
{
	return enter_recovery && can_data_be_mounted;
}

void dump_init_log(void)
{
	int pid = 1;
	int ret;
	struct siginfo info;
	struct task_struct *t = NULL;

	info.si_signo = SIG_TO_INIT;
	info.si_code = SI_QUEUE;
	info.si_int = SIG_INT_VALUE;
	rcu_read_lock();
	t = find_task_by_vpid(pid);
	if (t == NULL) {
		print_err("Init dump: no such pid\n");
		rcu_read_unlock();
	} else {
		rcu_read_unlock();
		ret = send_sig_info(SIG_TO_INIT, &info, t);
	if (ret < 0)
		print_err("Init dump: error sending signal\n");
	else
		print_err("Init dump: sending signal success\n");
	}
}

static void boot_detector_ctl(int enable_flag)
{
	down_interruptible(&detector_ctl_sem);
	boot_detector_enabled = (enable_flag != 0) ? true : false;
	up(&detector_ctl_sem);
}

static bool is_cmd_of_stage(unsigned int cmd)
{
	return (cmd == GET_BOOT_STAGE || cmd == SET_BOOT_STAGE);
}

static long full_rw_file(int fd, char *buf, size_t buf_size, bool read)
{
	mm_segment_t old_fs;
	long bytes_total_to_rw = (long)buf_size;
	long bytes_total_rw = 0L;
	long bytes_this_time = 0L;
	char *ptemp = buf;

	old_fs = get_fs();
	set_fs(KERNEL_DS); //lint !e501
	while (bytes_total_to_rw > 0) {
		bytes_this_time = read ?
			bf_sys_read(fd, ptemp, bytes_total_to_rw) :
			bf_sys_write(fd, ptemp, bytes_total_to_rw);
		if (read ? (bytes_this_time <= 0) : (bytes_this_time < 0)) {
			print_err("bf_sys_read or bf_sys_write failed!\n");
			break;
		}
		ptemp += bytes_this_time;
		bytes_total_to_rw -= bytes_this_time;
		bytes_total_rw += bytes_this_time;
	}
	set_fs(old_fs);

	return bytes_total_rw;
}

static int rw_part(const char *dev_path,
	unsigned long long offset,
	char *buf,
	unsigned long long buf_size,
	bool read)
{
	int fd;
	int ret = -1;
	long bytes_total;
	long seek_result;
	mm_segment_t fs;

	fs = get_fs();
	set_fs(KERNEL_DS); //lint !e501
	fd = bf_sys_open(dev_path, read ? O_RDONLY : O_WRONLY, 0);
	if (fd < 0) {
		print_err("Open file [%s] failed!, fd = %d\n", dev_path, fd);
		goto __out;
	}

	seek_result = bf_sys_lseek(fd, (off_t)offset, SEEK_SET);
	if ((off_t)offset != seek_result) {
		print_err("lseek [%s] failed! result: %ld it should be: %ld\n",
			dev_path, (long)seek_result, (long)offset);
		goto __out;
	}

	bytes_total = full_rw_file(fd, buf, buf_size, read);
	if ((long long)buf_size != bytes_total) {
		print_err("rw [%s] failed!, result: %ld, it should be: %lld\n",
			dev_path, bytes_total, (long long)buf_size);
		goto __out;
	} else {
		ret = 0;
	}

__out:
	if (fd >= 0) {
		bf_sys_fsync(fd);
		bf_sys_close(fd);
	}

	set_fs(fs);

	return ret;
}

int read_part(const char *dev_path,
	unsigned long long offset,
	char *buf,
	unsigned long long buf_size)
{
	if (dev_path == NULL || buf == NULL)
		return -1;

	return rw_part(dev_path, offset, buf, buf_size, true);
}

int write_part(const char *dev_path,
	unsigned long long offset,
	const char *buf,
	unsigned long long buf_size)
{
	if (dev_path == NULL || buf == NULL)
		return -1;

	return rw_part(dev_path, offset, (char *)buf, buf_size, false);
}


static void set_start_mount_data_flag(void)
{
	int ret;
	union bfi_part_header hdr;

	(void)memset(&hdr, 0, sizeof(hdr));
	ret = read_part(get_bfi_dev_path(), 0, (char *)&hdr, sizeof(hdr));
	if (ret != 0) {
		print_err("read rrecord fail\n");
		return;
	}

	if (hdr.misc_msg.fmt_data_flag != 0)
		return;
	hdr.misc_msg.fmt_data_flag = START_MOUNT_DATA;
	ret = write_part(get_bfi_dev_path(), 0, (const char *)&hdr,
		sizeof(hdr));
	if (ret != 0)
		print_err("write rrecord fail\n");
}

int process_data_mount_in_erecovery(const char __user *dir_name)
{
	char tmp_path[128] = { 0 };

	if (!is_erecovery_mode())
		return 0;

	if (copy_from_user(tmp_path, dir_name, sizeof(tmp_path) - 1) != 0)
		return 0;

	if (strcmp(tmp_path, "/data") == 0) {
		set_start_mount_data_flag();
		if (!data_can_be_mounted()) {
			print_err("/data can't be mounted!\n");
			return -1;
		}
	}

	return 0;
}

static void process_boot_success(enum boot_stage stage)
{
	if (stage != BOOT_SUCC_STAGE)
		return;

	if (get_elapsed_time() > BOOT_SLOWLY_TIME) {
		struct bootfail_proc_param pparam;

		print_info("Save the bootup slowly log begin!\n");
		memset(&pparam, 0, sizeof(pparam));
		pparam.binfo.bootfail_errno = FRAMEWORK_SYSTEM_FREEZE;
		pparam.binfo.stage = FRAMEWORK_STAGE;
		pparam.binfo.suggest_recovery_method = METHOD_DO_NOTHING;
		scnprintf(pparam.detail_info, sizeof(pparam.detail_info) - 1,
			"bootup slowly, short timer elapsed: %us",
			get_elapsed_time());
		(void)process_bootfail(&pparam);
		msleep(500); /* sleep 500 ms */
	}

	print_info("Stop the boot timer now\n");
	stop_boot_timer();
}

static long ioctl_stage(struct file *file,
	unsigned int cmd,
	unsigned long arg)
{
	long ret = 0;
	enum boot_stage stage;
	enum bootfail_errorcode error_code;

	if (!is_boot_detector_enabled()) {
		print_info("BootDetector is disabled!\n");
		return -EPERM;
	}

	switch (cmd) {
	case GET_BOOT_STAGE:
		error_code = get_boot_stage(&stage);
		if (error_code != BF_OK) {
			print_err("get_boot_stage ret: %d!\n", error_code);
			ret = -EFAULT;
			break;
		}

		if (copy_to_user((enum boot_stage *)(uintptr_t)arg,
			&stage, sizeof(stage)) != 0) {
			print_err("copy_to_user failed!\n");
			ret = -EFAULT;
			break;
		}
		break;
	case SET_BOOT_STAGE:
		if (copy_from_user(&stage, (int *)(uintptr_t)arg,
			sizeof(stage)) != 0) {
			print_err("copy_from_user failed!\n");
			ret = -EFAULT;
			break;
		}
		process_boot_success(stage);
		error_code = set_boot_stage(stage);
		if (error_code != BF_OK) {
			print_err("set_boot_stage ret: %d!\n", error_code);
			ret = -EFAULT;
		}
		break;
	default:
		print_err("Invalid CMD: 0x%x\n", cmd);
		ret = -EFAULT;
		break;
	}

	return ret;
}

static bool is_cmd_of_boot_timer(unsigned int cmd)
{
	return (cmd == GET_TIMER_STATUS ||
		cmd == RESUME_TIMER ||
		cmd == SUSPEND_TIMER ||
		cmd == GET_TIMER_TIMEOUT_VALUE ||
		cmd == SET_TIMER_TIMEOUT_VALUE);
}

static long ioctl_boot_timer(struct file *file,
	unsigned int cmd,
	unsigned long arg)
{
	long ret = 0;
	uintptr_t args = arg;
	unsigned int timeout_value = 0;

	switch (cmd) {
	case GET_TIMER_STATUS: {
		int state;

		get_boot_timer_state(&state);
		if (copy_to_user((int *)args, &state, sizeof(state)) != 0) {
			print_err("copy_to_user failed!\n");
			ret = -EFAULT;
			break;
		}
		print_info("short timer stats is: %d\n", state);
		break;
	}
	case RESUME_TIMER:
		(void)resume_boot_timer();
		break;
	case SUSPEND_TIMER:
		(void)suspend_boot_timer();
		break;
	case GET_TIMER_TIMEOUT_VALUE:
		get_boot_timer_timeout_value(&timeout_value);
		if (copy_to_user((int *)args, &timeout_value,
			sizeof(timeout_value)) != 0) {
			print_err("copy_to_user failed!\n");
			ret = -EFAULT;
			break;
		}
		print_info("short timer timeout value is: %u\n",
			timeout_value);
		break;
	case SET_TIMER_TIMEOUT_VALUE:
		if (copy_from_user(&timeout_value, (int *)args,
			sizeof(timeout_value)) != 0) {
			print_err("copy_from_user failed!\n");
			ret = -EFAULT;
			break;
		}
		(void)set_boot_timer_timeout_value(timeout_value);
		print_info("set short timer timeout value to: %u\n",
			timeout_value);
		break;
	default:
		break;
	}

	return ret;
}

static bool is_cmd_of_action_timer(unsigned int cmd)
{
	return (cmd == ACTION_TIMER_CTL);
}

static long ioctl_action_timer(struct file *file,
	unsigned int cmd,
	unsigned long arg)
{
	long ret = 0;
	struct action_ioctl_data *pact_data;
	uintptr_t args = arg;

	pact_data = vmalloc(sizeof(*pact_data));
	if (pact_data == NULL) {
		print_err("vmalloc failed!\n");
		return -ENOMEM;
	}

	if (copy_from_user(pact_data, (struct action_ioctl_data *)args,
		sizeof(*pact_data)) != 0) {
		print_info("copy action ctl data from user failed!\n");
		vfree(pact_data);
		return -EFAULT;
	}

	pact_data->action_name[ACTION_NAME_LEN - 1] = '\0';
	print_info("set action timer: %x, %s, %d\n",
		pact_data->op,
		pact_data->action_name,
		pact_data->action_timer_timeout_value);
	switch (pact_data->op) {
	case ACT_TIMER_START:
		ret = action_timer_start(pact_data->action_name,
			pact_data->action_timer_timeout_value);
		break;
	case ACT_TIMER_STOP:
		ret = action_timer_stop(pact_data->action_name);
		break;
	case ACT_TIMER_PAUSE:
		ret = action_timer_pause(pact_data->action_name);
		break;
	case ACT_TIMER_RESUME:
		ret = action_timer_resume(pact_data->action_name);
		break;
	default:
		break;
	}
	vfree(pact_data);

	return ret;
}

static int __process_upper_bootfail(void *pparam)
{
	struct bootfail_proc_param *local_pparam = NULL;
	enum bootfail_errorcode errorcode;
	struct thread_param *tparam = (struct thread_param *)pparam;

	if (unlikely(tparam == NULL))
		return -EINVAL;

	/* sync logcat to storage */
	if (tparam->pparam.binfo.suggest_recovery_method != METHOD_DO_NOTHING) {
		bf_sys_sync();
		msleep(2000);
		bf_sys_sync();
	}

	down(&boot_detector_sem);
	local_pparam = vmalloc(sizeof(*local_pparam));
	if (local_pparam == NULL) {
		print_err("vmalloc failed!\n");
		up(&(tparam->sem));
		up(&boot_detector_sem);
		return -ENOMEM;
	}
	memcpy((void *)local_pparam, &(tparam->pparam), sizeof(*local_pparam));
	up(&(tparam->sem));
	errorcode = boot_fail_error(local_pparam);
	print_info("boot_fail_error ret: %d\n", errorcode);
	vfree(local_pparam);
	up(&boot_detector_sem);

	return 0;
}

static void wait_compeletion_of_processing_bootfail(void)
{
	while (1)
		msleep_interruptible(BLOCK_CALLING_PROCESS_INTERVAL);
}

unsigned long long get_sys_rtc_time(void)
{
	struct timeval tv = {0};

	do_gettimeofday(&tv);

	return (unsigned long long)tv.tv_sec;
}

static int get_fs_state(const char *pmount_point, struct statfs *pstat)
{
	mm_segment_t old_fs;
	int ret;

	old_fs = get_fs();
	set_fs(KERNEL_DS); //lint !e501
	memset((void *)pstat, 0, sizeof(*pstat));
	ret = bf_sys_statfs(pmount_point, pstat);
	set_fs(old_fs);

	return ret;
}

void get_data_part_info(struct bootfail_proc_param *pparam)
{
	struct statfs statbuf = { 0 };
	int ret = -1;

	if (unlikely(pparam == NULL)) {
		print_invalid_params("pparam: %p\n", pparam);
		return;
	}

	if (is_dir_existed("/data/lost+found")) {
		ret = get_fs_state("/data", &statbuf);
		if (ret == 0) {
			pparam->space_left = (unsigned int)((
				(unsigned long long)statbuf.f_bavail *
				statbuf.f_bsize) / BF_SIZE_1M); /* Unit: MB */
			pparam->inodes_left = (unsigned int)statbuf.f_ffree;
		}
	}
}

static void format_bootfail_params(struct bootfail_proc_param *pparam)
{
	get_data_part_info(pparam);
	if (pparam->magic == 0)
		pparam->magic = BF_SW_MAGIC_NUM;
	pparam->binfo.bootup_time = get_bootup_time();
	pparam->binfo.rtc_time = get_sys_rtc_time();
}

static int process_upper_bootfail(unsigned long arg)
{
	int ret = 0;
	struct bootfail_proc_param pparam;

	memset((void *)&pparam, 0, sizeof(pparam));
	if (copy_from_user(&pparam, ((struct bootfail_proc_param *)(uintptr_t)arg),
		sizeof(pparam)) != 0) {
		print_err("copy_from_user failed!\n");
		return -EFAULT;
	}
	ret = process_bootfail(&pparam);

	return ret;
}

static int process_bootfail(struct bootfail_proc_param *pparam)
{
	struct thread_param *param = NULL;
	struct task_struct *tsk = NULL;

	if (unlikely(pparam == NULL))
		return -EINVAL;

	param = vmalloc(sizeof(*param));
	if (param == NULL) {
		print_err("vmalloc failed!\n");
		return -ENOMEM;
	}
	memset((void *)param, 0, sizeof(*param));
	memcpy(&param->pparam, pparam, sizeof(*pparam));
	format_bootfail_params(&param->pparam);
	print_info("bootfail_errno: 0x%x, detail_info: %s, bootup_time: %ds\n",
		param->pparam.binfo.bootfail_errno, param->pparam.detail_info,
		param->pparam.binfo.bootup_time);
	sema_init(&param->sem, 0);
	tsk = kthread_run(__process_upper_bootfail,
		(void *)param, "proc_upper_bf");
	if (IS_ERR(tsk)) {
		print_err("kthread_run failed!\n");
		vfree(param);
		return -1;
	}
	down(&param->sem);
	if (param->pparam.binfo.suggest_recovery_method ==
		METHOD_NO_SUGGESTION)
		wait_compeletion_of_processing_bootfail();
	vfree(param);

	return 0;
}

#ifdef CONFIG_BOOT_DETECTOR_DBG
static long get_key_struct_size(unsigned long arg)
{
	struct key_struct_size size;

	size.binfo_size = sizeof(struct bootfail_basic_info);
	size.proc_param_size = sizeof(struct bootfail_proc_param);
	size.bfi_part_hdr_size = sizeof(union bfi_part_header);
	size.meta_size = sizeof(struct bootfail_meta_log);
	size.log_hdr_size = sizeof(struct bootfail_log_header);
	size.rrecord_hdr_size = sizeof(struct rrecord_header);
	size.rrecord_size = sizeof(struct rrecord);
	size.recovery_size = sizeof(struct recovery_info);
	size.bfi_part_size = get_bfi_part_size();
	if (copy_to_user((void *)(uintptr_t)arg, &size,
		sizeof(size)) != 0) {
		print_err("Failed to copy flag from user!\n");
		return -EFAULT;
	}
	return 0;
}
#else
static long get_key_struct_size(unsigned long arg)
{
	return -EFAULT;
}
#endif

static long ioctl_others(struct file *file,
	unsigned int cmd,
	unsigned long arg)
{
	long ret = 0;
	int enable_flag;
	struct bfi_dev_path *path = (struct bfi_dev_path *)(uintptr_t)arg;

	switch (cmd) {
	case PROCESS_BOOTFAIL:
		if (!is_boot_detector_enabled()) {
			print_info("BootDetector is disabled!\n");
			return -EPERM;
		}
		ret = process_upper_bootfail(arg);
		break;
	case BOOT_DETECTOR_ENABLE_CTRL:
		if (copy_from_user(&enable_flag, (int *)(uintptr_t)arg,
			sizeof(int)) != 0) {
			print_err("Failed to copy flag from user!\n");
			return -EFAULT;
		}
		print_info("set enable flag: %d\n", enable_flag);
		boot_detector_ctl(enable_flag);
		break;
	case GET_KEY_STRUCT_SIZE:
		return get_key_struct_size(arg);
	case SIMULATE_STORAGE_RDONLY:
		return simulate_storge_rdonly(arg);
	case GET_BFI_DEV_PATH:
		if (get_bfi_dev_path() == NULL)
			return -ENODEV;
		/*lint -e666*/
		if (copy_to_user(path->path, get_bfi_dev_path(),
			min(strlen(get_bfi_dev_path()),
			sizeof(path->path) - 1)) != 0) {
			print_err("Failed to copy bfi dev path to user!\n");
			return -EFAULT;
		}
		/*lint +e666*/
		break;
	default:
		print_err("Invalid CMD: 0x%x\n", cmd);
		ret = -EFAULT;
		break;
	}

	return ret;
}

static bool is_cmd_of_dynamic_brd(unsigned int cmd)
{
	return (cmd == IOC_CREATE_DYNAMIC_RAMDISK ||
		cmd ==  IOC_DELETE_DYNAMIC_RAMDISK);
}

static long ioctl_dynamic_brd(struct file *file,
	unsigned int cmd,
	unsigned long arg)
{
	long ret = 0;

#ifdef CONFIG_HUAWEI_DYNAMIC_BRD
	uintptr_t args = arg;

	switch (cmd) {
	case IOC_CREATE_DYNAMIC_RAMDISK:
		ret = create_dynamic_ramdisk(
			(struct dbrd_ioctl_block __user *)args);
		break;
	case IOC_DELETE_DYNAMIC_RAMDISK:
		ret = delete_dynamic_ramdisk(
			(struct dbrd_ioctl_block __user *)args);
		break;
	default:
		break;
	}
#endif

	return ret;
}

static bool is_cmd_of_storage_rofa(unsigned int cmd)
{
	return (cmd == IOC_CHECK_BOOTDISK_WP ||
		cmd == IOC_ENABLE_MONITOR ||
		cmd == IOC_DO_STORAGE_WRTRY ||
		cmd == IOC_GET_STORAGE_ROFA_INFO ||
		cmd == IOC_GET_BOOTDEVICE_DISK_COUNT ||
		cmd == IOC_GET_BOOTDEVICE_DISK_INFO ||
		cmd == IOC_GET_BOOTDEVICE_PROD_INFO);
}

static long ioctl_storage_rofa(struct file *file,
	unsigned int cmd,
	unsigned long arg)
{
	long ret = 0;

#ifdef CONFIG_HUAWEI_STORAGE_ROFA
	uintptr_t args = arg;

	switch (cmd) {
	case IOC_CHECK_BOOTDISK_WP:
		ret = storage_rochk_ioctl_check_bootdisk_wp(
			(struct bootdisk_wp_status_iocb __user *)args);
		break;
	case IOC_ENABLE_MONITOR:
		ret = storage_rochk_ioctl_enable_monitor(
			(struct storage_rochk_iocb __user *)args);
		break;
	case IOC_DO_STORAGE_WRTRY:
		ret = storage_rochk_ioctl_run_storage_wrtry_sync(
			(struct storage_rochk_iocb __user *)args);
		break;
	case IOC_GET_STORAGE_ROFA_INFO:
		ret = storage_rofa_ioctl_get_rofa_info(
			(struct storage_rofa_info_iocb __user *)args);
		break;
	case IOC_GET_BOOTDEVICE_DISK_COUNT:
		ret = storage_rochk_ioctl_get_bootdevice_disk_count(
			(struct storage_rochk_iocb __user *)args);
		break;
	case IOC_GET_BOOTDEVICE_DISK_INFO:
		ret = storage_rochk_ioctl_get_bootdevice_disk_info(
			(struct bootdevice_disk_info_iocb __user *)args);
		break;
	case IOC_GET_BOOTDEVICE_PROD_INFO:
		ret = storage_rochk_ioctl_get_bootdevice_prod_info(
			(struct bootdevice_prod_info_iocb __user *)args);
		break;
	default:
		break;
	}
#endif

	return ret;
}

static long boot_detector_ioctl(struct file *file,
	unsigned int cmd,
	unsigned long arg)
{
	long ret = 0;

	if ((void *)(uintptr_t)arg == NULL) {
		print_invalid_params("arg: %p\n", (void *)(uintptr_t)arg);
		return -EINVAL;
	}

	if (is_cmd_of_dynamic_brd(cmd))
		ret = ioctl_dynamic_brd(file, cmd, arg);
	else if (is_cmd_of_storage_rofa(cmd))
		ret = ioctl_storage_rofa(file, cmd, arg);
	else if (is_cmd_of_stage(cmd))
		ret = ioctl_stage(file, cmd, arg);
	else if (is_cmd_of_boot_timer(cmd))
		ret = ioctl_boot_timer(file, cmd, arg);
	else if (is_cmd_of_action_timer(cmd))
		ret = ioctl_action_timer(file, cmd, arg);
	else
		ret = ioctl_others(file, cmd, arg);

	return ret;
}

static int boot_detector_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

static ssize_t boot_detector_write(struct file *file,
	const char *data,
	size_t len,
	loff_t *ppos)
{
	return len;
}

static ssize_t boot_detector_read(struct file *file,
	char __user *buf,
	size_t count,
	loff_t *pos)
{
	return count;
}

static int boot_detector_release(struct inode *inode, struct file *file)
{
	return 0;
}

int read_from_phys_mem(unsigned long dst,
	unsigned long dst_max,
	void *phys_mem_addr,
	unsigned long data_len)
{
	char *pdst = NULL;
	char *paddr = NULL;
	unsigned long i;
	size_t bytes_to_read;

	bytes_to_read = min(dst_max, data_len);
	/*lint -e446*/
	paddr = (char *)ioremap_nocache((phys_addr_t)(uintptr_t)phys_mem_addr,
		bytes_to_read);
	/*lint +e446*/
	if (paddr == NULL)
		return -1;

	pdst = (char *)(uintptr_t)dst;
	for (i = 0; i < bytes_to_read; i++) {
		*pdst = readb(paddr);
		pdst++;
		paddr++;
	}
	iounmap(paddr);

	return 0;
}

int write_to_phys_mem(unsigned long dst,
	unsigned long dst_max,
	void *src,
	unsigned long src_len)
{
	char *psrc = NULL;
	char *paddr = NULL;
	unsigned long i;
	size_t bytes_to_write;

	bytes_to_write = min(dst_max, src_len);
	/*lint -e446*/
	paddr = (char *)ioremap_nocache((phys_addr_t)dst,
		bytes_to_write);
	/*lint +e446*/
	if (paddr == NULL)
		return -1;

	psrc = (char *)src;
	for (i = 0; i < bytes_to_write; i++) {
		writeb(*psrc, paddr);
		psrc++;
		paddr++;
	}
	iounmap(paddr);

	return 0;
}

static bool is_file_existed(const char *pfile_path)
{
	mm_segment_t old_fs;
	bootfail_stat_t st;
	int ret;

	if (unlikely(!pfile_path)) {
		print_invalid_params("pfile_path: %p\n", pfile_path);
		return false;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS); //lint !e501
	memset((void *)&st, 0, sizeof(st));
	ret = bf_lstat(pfile_path, &st);
	set_fs(old_fs);

	return (ret == 0) ? (true) : (false);
}

bool is_dir_existed(const char *pdir_path)
{
	return is_file_existed(pdir_path);
}

bool wait_file_node_ready(const char* file_node, int timeouts)
{
	if (unlikely(!file_node)) {
		print_err("invalid parameter: file_node\n");
		return false;
	}

	while (timeouts > 0) {
		if (is_file_existed(file_node)) {
			print_info("%s is ready!\n", file_node);
			return true;
		}

		msleep(1000); /* sleep 1 second */
		timeouts--;
	}

	print_err("%s is not ready until timeout!\n", file_node);
	return false;
}

int bootfail_common_init(struct common_init_params *pparam)
{
	int ret;

	if (pparam == NULL) {
		print_invalid_params("pparam: %p\n", pparam);
		return -1;
	}

	ret = misc_register(&boot_detector_miscdev);
	if (ret != 0) {
		print_err("misc_register failed, ret: %d.\n", ret);
		return ret;
	}

	if (!is_boot_detector_enabled()) {
		print_info("BootDetector is disabled!\n");
		return -1;
	}

	(void)start_boot_timer();

	return 0;
}
