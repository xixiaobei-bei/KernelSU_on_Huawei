#include <asm/compiler.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/version.h>
#include <securec.h>
#include <linux/hisi/dpm_hwmon.h>

static LIST_HEAD(dpm_hwmon_ops_list);
static DEFINE_MUTEX(dpm_hwmon_ops_list_lock);

/* common module start */
int dpm_hwmon_register(struct dpm_hwmon_ops *dpm_ops)
{
	struct dpm_hwmon_ops *pos = NULL;
	int ret = -1;

	if (dpm_ops == NULL || dpm_ops->name == NULL) {
		pr_err("%s LINE[%d] dpm_ops is NULL\n", __func__, __LINE__);
		return -EINVAL;
	}

	mutex_lock(&dpm_hwmon_ops_list_lock);
	list_for_each_entry(pos, &dpm_hwmon_ops_list, ops_list) {
		if (strncasecmp(dpm_ops->name, pos->name, DPM_MODULE_NAME_LENGTH) == 0) {
			pr_err("%s LINE[%d] dpm module %s has registered\n", __func__, __LINE__, dpm_ops->name);
			mutex_unlock(&dpm_hwmon_ops_list_lock);
			return ret;
		}
	}
	list_add(&dpm_ops->ops_list, &dpm_hwmon_ops_list);
	mutex_unlock(&dpm_hwmon_ops_list_lock);

	if ((dpm_ops->dpm_cnt_tab_len > 0 && dpm_ops->dpm_cnt_tab_len < DPM_BUFFER_SIZE) &&
	   (dpm_ops->dpm_fit_tab_len > 0 && dpm_ops->dpm_fit_tab_len < DPM_BUFFER_SIZE)) {
		dpm_ops->dpm_counter_table = (unsigned long long *)kzalloc(sizeof(unsigned long long) * dpm_ops->dpm_cnt_tab_len, GFP_KERNEL);
		dpm_ops->dpm_fitting_table = (int *)kzalloc(sizeof(int) * dpm_ops->dpm_fit_tab_len, GFP_KERNEL);
		if (dpm_ops->dpm_counter_table == NULL || dpm_ops->dpm_counter_table == NULL) {
			ret = -ENOMEM;
			goto err_handler;
		}

	} else {
		return -EINVAL;
	}

	return 0;

err_handler:
	if (dpm_ops->dpm_counter_table != NULL) {
		kfree(dpm_ops->dpm_counter_table);
		dpm_ops->dpm_counter_table = NULL;
	}
	if (dpm_ops->dpm_fitting_table != NULL) {
		kfree(dpm_ops->dpm_fitting_table);
		dpm_ops->dpm_fitting_table = NULL;
	}

	return ret;
}
EXPORT_SYMBOL(dpm_hwmon_register);

int dpm_hwmon_unregister(struct dpm_hwmon_ops *dpm_ops)
{
	struct dpm_hwmon_ops *pos = NULL;
	struct dpm_hwmon_ops *tmp = NULL;

	if (dpm_ops == NULL) {
		pr_err("%s LINE[%d]dpm_ops is NULL\n", __func__, __LINE__);
		return -EINVAL;
	}

	mutex_lock(&dpm_hwmon_ops_list_lock);

	list_for_each_entry_safe(pos, tmp, &dpm_hwmon_ops_list, ops_list) {
		if (strncasecmp(dpm_ops->name, pos->name, DPM_MODULE_NAME_LENGTH) == 0) {
			if (pos->dpm_counter_table != NULL) {
				kfree(pos->dpm_counter_table);
				pos->dpm_counter_table = NULL;
			}
			if (pos->dpm_fitting_table != NULL) {
				kfree(pos->dpm_fitting_table);
				pos->dpm_fitting_table = NULL;
			}
			list_del_init(&pos->ops_list);
			break;
		}
	}

	mutex_unlock(&dpm_hwmon_ops_list_lock);

	return 0;
}
EXPORT_SYMBOL(dpm_hwmon_unregister);

#define	 DPMONITOR_SIGNAL_MODE_ADDR(base)		((base) + (0xE4))
#define	 DPMONITOR_CFG_TIME_ADDR(base)			((base) + (0x004))
#define	 DPMONITOR_CTRL_EN_ADDR(base)			((base) + (0x000))
#define	 DPMONITOR_SENSOR_EN_ADDR(base)			((base) + (0x008))
#define	 DPMONITOR_SOFT_SAMPLE_PULSE_ADDR(base)			((base) + (0x0E8))
#define	 DPMONITOR_SENSOR_DIS_ADDR(base)			((base) + (0x00C))
#define	 DPMONITOR_LOW_LOAD0_ADDR(base)			((base) + (0x014))

void dpm_monitor_enable(void __iomem *module_dpm_addr, unsigned int dpmonitor_signal_mode)
{
	/* step 5: module signal mode */
	writel(dpmonitor_signal_mode, DPMONITOR_SIGNAL_MODE_ADDR(module_dpm_addr));
	/* step 6: module cfg time */
	writel(0xffffffff, DPMONITOR_CFG_TIME_ADDR(module_dpm_addr));
	/* step 7: low power enable */
	writel(0x00080008, DPMONITOR_CTRL_EN_ADDR(module_dpm_addr));
	/* step 8: enable controller */
	writel(0x00010001, DPMONITOR_CTRL_EN_ADDR(module_dpm_addr));
	/* step 9: enable tsensor */
	writel(0x0000FFFF, DPMONITOR_SENSOR_EN_ADDR(module_dpm_addr));
}
EXPORT_SYMBOL(dpm_monitor_enable);

void dpm_monitor_disable(void __iomem *module_dpm_addr)
{
	/* step 1: disable tsensor */
	writel(0xFFFFFFFF, DPMONITOR_SENSOR_DIS_ADDR(module_dpm_addr));
	/* step 2: disable controller */
	writel(0x00010000, DPMONITOR_CTRL_EN_ADDR(module_dpm_addr));
}
EXPORT_SYMBOL(dpm_monitor_disable);

/* common module end */

long long int get_dpm_chdmod_power(const char *name)
{
	unsigned int  i;
	int ret;
	int flag = -1;
	long long int dpm_power = -1;
	struct dpm_hwmon_ops *pos = NULL;
	long long int counter_diff[DPM_BUFFER_SIZE];
	static unsigned int g_dpm_buffer[DPM_BUFFER_SIZE];

	pr_err("%s LINE[%d]\n", __func__, __LINE__);

	mutex_lock(&dpm_hwmon_ops_list_lock);
	list_for_each_entry(pos, &dpm_hwmon_ops_list, ops_list) {
		if (strncasecmp(name, pos->name, DPM_MODULE_NAME_LENGTH) == 0) {
			flag = 0;
			break;
		}
	}
	mutex_unlock(&dpm_hwmon_ops_list_lock);

	if (flag == 0) {
		ret = pos->hi_dpm_update_counter();
		if (ret != 0) {
			goto end;
		}
		for (i = 0; i < pos->dpm_cnt_tab_len; i++) {
			if (pos->dpm_counter_table[i] < g_dpm_buffer[i])
				counter_diff[i] = UINT64_MAX - g_dpm_buffer[i] + pos->dpm_counter_table[i];
			else
				counter_diff[i] = pos->dpm_counter_table[i] - g_dpm_buffer[i];
		}

		// calc power FIXME
		for (i = 0; i < pos->dpm_cnt_tab_len && i < pos->dpm_fit_tab_len; i++)
			dpm_power += counter_diff[i] * pos->dpm_fitting_table[i];

		for (i = 0; i < pos->dpm_cnt_tab_len; i++)
			g_dpm_buffer[i] = pos->dpm_counter_table[i];
	}
end:
	return dpm_power;
}

#ifdef CONFIG_HISI_DPM_HWMON_DEBUG

#define MAX_DPM_LOG_SIZE 1024
unsigned int g_dpm_buffer_for_fitting[DPM_BUFFER_SIZE];

static unsigned long long get_timer_value(void)
{
	unsigned long long total_us;
	struct timespec ts = (struct timespec){0, 0};

	getnstimeofday(&ts);
	total_us = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;

	return total_us;
}

static int select_which_dpm_update(const char *name)
{
	struct dpm_hwmon_ops *pos = NULL;
	int flag = -1;
	int count = -1;

	pr_debug("%s LINE[%d]\n", __func__, __LINE__);

	if (name == NULL)
		return count;

	mutex_lock(&dpm_hwmon_ops_list_lock);
	list_for_each_entry(pos, &dpm_hwmon_ops_list, ops_list) {
		if (strncasecmp(name, pos->name, DPM_MODULE_NAME_LENGTH) == 0) {
			flag = 0;
			break;
		}
	}
	mutex_unlock(&dpm_hwmon_ops_list_lock);

	if (flag == 0)
		count = pos->hi_dpm_get_counter_for_fitting();

	return count;
}

static const char * const dpm_module_table[] = {
	"dss_disp",
	"aicore",
	"vdec",
	"venc0",
	"dpm_ipp",
	"dpm_ivp",
};

static void print_dpm_register(const char *module_name)
{
	int bypes_num;
	char buf[1024] = {'\0'};
	unsigned long long total_us;
	unsigned int i;
	int offset = 0;
	int ret;

	pr_err("%s LINE[%d] module_name = %s\n", __func__, __LINE__, module_name);

	bypes_num = select_which_dpm_update(module_name);
	if (bypes_num == -1)
		return;

	total_us = get_timer_value();

	ret = sprintf_s(buf + offset, MAX_DPM_LOG_SIZE, "[DPM] time : %llu us reg: ", total_us);
	if (ret < 0)
		return;
	else
		offset += ret;

	for (i = 0; i < bypes_num; i++) {
		ret = sprintf_s(buf + offset, MAX_DPM_LOG_SIZE - offset, "%d,", g_dpm_buffer_for_fitting[i]);
		if (ret < 0)
			return;
		else
			offset += ret;
	}
	if (offset > 0) {
		buf[offset - 1] = '\n';
		pr_err("%s", buf);
	}

}

#ifdef CONFIG_HISI_DEBUG_FS
struct dentry *dpm_hwmon_debugfs_root = NULL;
static int dpm_debug_show(struct seq_file *m, void *v)
{
	int dpm_id;
	long long int dpm_power;

	for (dpm_id = 0; dpm_id < ARRAY_SIZE(dpm_module_table); dpm_id++) {
		dpm_power = get_dpm_chdmod_power(dpm_module_table[dpm_id]);
		seq_printf(m, "%s : ", dpm_module_table[dpm_id]);
		seq_printf(m, "%lld\n", dpm_power);
	}

	return 0;
}

static int dpm_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, dpm_debug_show, NULL);
}

static ssize_t dpm_debug_write(struct file *file, const char __user *user_buf, size_t count, loff_t *f_pos)
{
	char buf[32];
	unsigned int i;
	int dpm_id = 0;
	int timer_span_ms = 0;
	int total_count = 0;

	if (user_buf == NULL) {
		pr_err("%s:user_buf is NULL!\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(buf, user_buf, min_t(size_t, sizeof(buf) - 1, count))) {
		pr_err("[%s]copy error!\n", __func__);
		return -EFAULT;
	}
	pr_err("%s: buf = %s\n", __func__, buf);

	// dpm_id, timer_span_ms, total_count
	if (sscanf_s(buf, "%d %d %d", &dpm_id, &timer_span_ms, &total_count) != 3) {
		pr_err("[%s]the num of enter is wrong!\n", __func__);
		return -EFAULT;
	}

	pr_err("%s: dpm_id = %d, timer_span_ms = %d, total_count = %d!\n", __func__, dpm_id, timer_span_ms, total_count);

	if (dpm_id < ARRAY_SIZE(dpm_module_table) && timer_span_ms > 0 && total_count > 0) {
		for (i = 0; i < total_count; i++) {
			mdelay(timer_span_ms);
			print_dpm_register(dpm_module_table[dpm_id]);
		}
	}

	return count;
}

static const struct file_operations dpm_debug_fops = {
	.owner	= THIS_MODULE,
	.open	= dpm_debug_open,
	.read	= seq_read,
	.write	= dpm_debug_write,
	.llseek	= seq_lseek,
	.release	= single_release
};

#endif

static int __init dpm_hwmon_debugfs_init(void)
{
#ifdef CONFIG_HISI_DEBUG_FS
	struct dentry *dpm_hwmon_debug = NULL;

	pr_err("dpm %s\n", __func__);
	dpm_hwmon_debugfs_root = debugfs_create_dir("dpm_hwmon", NULL);
	if (dpm_hwmon_debugfs_root == NULL)
		goto Fail;

	dpm_hwmon_debug = debugfs_create_file("dpm_debug", 0770, dpm_hwmon_debugfs_root, NULL, &dpm_debug_fops);
	if (dpm_hwmon_debug == NULL)
		goto Fail;
#endif

	return 0;

#ifdef CONFIG_HISI_DEBUG_FS
Fail:
	debugfs_remove_recursive(dpm_hwmon_debugfs_root);
	pr_err("%s LINE[%d] fail!\n", __func__, __LINE__);

	return -ENOENT;
#endif

}

static void __exit dpm_hwmon_debugfs_exit(void)
{
	pr_err("dpm %s\n", __func__);
#ifdef CONFIG_HISI_DEBUG_FS
	debugfs_remove_recursive(dpm_hwmon_debugfs_root);
	pr_err("dpm_hwmon_debugfs removed!\n");
#endif
}

/*lint -e528 -esym(528,*)*/
module_init(dpm_hwmon_debugfs_init);
module_exit(dpm_hwmon_debugfs_exit);
/*lint -e528 +esym(528,*)*/

#endif
