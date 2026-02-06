#include <linux/init.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include <chipset_common/dubai/dubai_ioctl.h>
#include <chipset_common/dubai/dubai_plat.h>

#include "dubai_battery_stats.h"
#include "dubai_cpu_stats.h"
#include "dubai_ddr_stats.h"
#include "dubai_display_stats.h"
#include "dubai_gpu_stats.h"
#include "dubai_peri_stats.h"
#include "dubai_misc_stats.h"
#include "dubai_sensorhub_stats.h"
#include "dubai_sr_stats.h"
#include "utils/buffered_log_sender.h"
#include "utils/dubai_utils.h"

#define DUBAI_MAGIC 'k'
#define DUBAID_NAME				"dubaid"

int dubai_register_module_ops(enum dubai_module_t module, void *mops)
{
	int ret = -EINVAL;

	switch (module) {
	case DUBAI_MODULE_BATTERY:
		ret = dubai_battery_register_ops(mops);
		break;
	case DUBAI_MODULE_DDR:
		ret = dubai_ddr_register_ops(mops);
		break;
	case DUBAI_MODULE_DISPLAY:
		ret = dubai_display_register_ops(mops);
		break;
	case DUBAI_MODULE_GPU:
		ret = dubai_gpu_register_ops(mops);
		break;
	case DUBAI_MODULE_PERI:
		ret = dubai_peri_register_ops(mops);
		break;
	case DUBAI_MODULE_WAKEUP:
		ret = dubai_wakeup_register_ops(mops);
		break;
	default:
		break;
	}

	return ret;
}

int dubai_unregister_module_ops(enum dubai_module_t module)
{
	int ret = -EINVAL;

	switch (module) {
	case DUBAI_MODULE_BATTERY:
		ret = dubai_battery_unregister_ops();
		break;
	case DUBAI_MODULE_DDR:
		ret = dubai_ddr_unregister_ops();
		break;
	case DUBAI_MODULE_DISPLAY:
		ret = dubai_display_unregister_ops();
		break;
	case DUBAI_MODULE_GPU:
		ret = dubai_gpu_unregister_ops();
		break;
	case DUBAI_MODULE_PERI:
		ret = dubai_peri_unregister_ops();
		break;
	case DUBAI_MODULE_WAKEUP:
		ret = dubai_wakeup_unregister_ops();
		break;
	default:
		break;
	}

	return ret;
}

static long dubai_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int rc;
	void __user *argp = (void __user *)arg;
	const char *comm = NULL;

	comm = current->group_leader ? current->group_leader->comm : current->comm;
	if (!strstr(comm, DUBAID_NAME))
		return -EPERM;

	rc = 0;
	switch (_IOC_TYPE(cmd)) {
	case DUBAI_IOC_DISPLAY:
		rc = dubai_ioctl_display(cmd, argp);
		break;
	case DUBAI_IOC_CPU:
		rc = dubai_ioctl_cpu(cmd, argp);
		break;
	case DUBAI_IOC_GPU:
		rc = dubai_ioctl_gpu(cmd, argp);
		break;
	case DUBAI_IOC_SENSORHUB:
		rc = dubai_ioctl_sensorhub(cmd, argp);
		break;
	case DUBAI_IOC_BATTERY:
		rc = dubai_ioctl_battery(cmd, argp);
		break;
	case DUBAI_IOC_UTILS:
		rc = dubai_ioctl_utils(cmd, argp);
		break;
	case DUBAI_IOC_SR:
		rc = dubai_ioctl_sr(cmd, argp);
		break;
	case DUBAI_IOC_MISC:
		rc = dubai_ioctl_misc(cmd, argp);
		break;
	case DUBAI_IOC_DDR:
		rc = dubai_ioctl_ddr(cmd, argp);
		break;
	case DUBAI_IOC_PERI:
		rc = dubai_ioctl_peri(cmd, argp);
		break;
	default:
		dubai_err("Unknown dubai ioctrl type");
		rc = -EINVAL;
		break;
	}

	return rc;
}

#ifdef CONFIG_COMPAT
static long dubai_compat_ioctl(struct file *filp,
			unsigned int cmd, unsigned long arg)
{
	return dubai_ioctl(filp, cmd, (unsigned long) compat_ptr(arg));
}
#endif

static int dubai_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int dubai_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations dubai_device_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = dubai_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= dubai_compat_ioctl,
#endif
	.open = dubai_open,
	.release = dubai_release,
};

static struct miscdevice dubai_device = {
	.name = "dubai",
	.fops = &dubai_device_fops,
	.minor = MISC_DYNAMIC_MINOR,
};

static int __init dubai_init(void)
{
	int ret = 0;

	dubai_display_stats_init();
	dubai_gpu_stats_init();
	dubai_proc_cputime_init();
	dubai_sr_stats_init();
	dubai_misc_stats_init();
	dubai_ddr_stats_init();
	dubai_peri_stats_init();

	ret = misc_register(&dubai_device);
	if (ret) {
		dubai_err("Failed to register dubai device");
		goto out;
	}
	dubai_info("Succeed to init dubai module");

out:
	return ret;
}

static void __exit dubai_exit(void)
{
	dubai_battery_stats_exit();
	dubai_gpu_stats_exit();
	dubai_proc_cputime_exit();
	dubai_sr_stats_exit();
	dubai_misc_stats_exit();
	dubai_ddr_stats_exit();
	dubai_peri_stats_exit();
	buffered_log_release();
}

late_initcall_sync(dubai_init);
module_exit(dubai_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yu Peng, <pengyu7@huawei.com>");
MODULE_DESCRIPTION("Huawei Device Usage Big-data Analytics Initiative Driver");
