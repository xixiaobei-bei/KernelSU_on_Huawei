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
static DEFINE_MUTEX(dpm_hwmon_ops_lock);

bool dpm_report_enabled = true;
EXPORT_SYMBOL(dpm_report_enabled);

/* common module start */
static void free_dpm_ops(struct dpm_hwmon_ops *dpm_ops)
{
	if (dpm_ops->dpm_counter_table != NULL) {
		kfree(dpm_ops->dpm_counter_table);
		dpm_ops->dpm_counter_table = NULL;
	}
	if (dpm_ops->last_dpm_counter != NULL) {
		kfree(dpm_ops->last_dpm_counter);
		dpm_ops->last_dpm_counter = NULL;
	}
	if (dpm_ops->dpm_fitting_table != NULL) {
		kfree(dpm_ops->dpm_fitting_table);
		dpm_ops->dpm_fitting_table = NULL;
	}
}

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

	if ((dpm_ops->dpm_cnt_len > 0 && dpm_ops->dpm_cnt_len < DPM_BUFFER_SIZE) &&
	   (dpm_ops->dpm_fit_len > 0 && dpm_ops->dpm_fit_len < DPM_BUFFER_SIZE)) {
		dpm_ops->dpm_counter_table = kzalloc(sizeof(unsigned long long) * dpm_ops->dpm_cnt_len, GFP_KERNEL);
		dpm_ops->last_dpm_counter = kzalloc(sizeof(unsigned long long) * dpm_ops->dpm_cnt_len, GFP_KERNEL);
		dpm_ops->dpm_fitting_table = kzalloc(sizeof(int) * dpm_ops->dpm_fit_len, GFP_KERNEL);
		if (dpm_ops->dpm_counter_table == NULL ||
		    dpm_ops->last_dpm_counter == NULL ||
		    dpm_ops->dpm_fitting_table == NULL) {
			ret = -ENOMEM;
			goto err_handler;
		}

	} else {
		return -EINVAL;
	}

	if (dpm_ops->hi_dpm_fitting_coff != NULL) {
		ret = dpm_ops->hi_dpm_fitting_coff();
		if (ret < 0)
			goto err_handler;
	}

	return 0;

err_handler:
	ret = dpm_hwmon_unregister(dpm_ops);
	if (ret < 0)
		pr_err("%s LINE[%d] register fail", __func__, __LINE__);
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
			free_dpm_ops(pos);
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

static long long get_first_level_power(struct dpm_hwmon_ops *pos)
{
	unsigned int  i;
	unsigned long long counter;
	unsigned long long counter_diff;
	long long dpm_power = 0;

	for (i = 0; (i < pos->dpm_cnt_len) && (i < pos->dpm_fit_len); i++) {
		counter = pos->dpm_counter_table[i];
		if (counter < pos->last_dpm_counter[i])
			counter_diff = UINT64_MAX - pos->last_dpm_counter[i] + counter;
		else
			counter_diff = counter - pos->last_dpm_counter[i];

		dpm_power += counter_diff * (long long)(pos->dpm_fitting_table[i]);
		pos->last_dpm_counter[i] = counter;
	}
	return dpm_power;
}

#if defined(CONFIG_HISI_DPM_TWO_LEVEL) || defined(CONFIG_HISI_DPM_HWMON_DEBUG)
static unsigned long long get_timer_value(void)
{
	unsigned long long total_us;
	struct timespec ts = (struct timespec){0, 0};

	getnstimeofday(&ts);
	total_us = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;

	return total_us;
}
#endif

#ifdef CONFIG_HISI_DPM_TWO_LEVEL
static long long get_compensate_power(struct dpm_hwmon_ops *pos)
{
	unsigned long long time;
	long long voltage;
	long long compensate;

	time = get_timer_value();
	voltage = (long long)(pos->hi_dpm_get_voltage());
	if (voltage > 0)
		compensate = (long long)(pos->compensate_coff) * voltage * voltage *
			     ((time - pos->last_time) / 1000);
	else
		compensate = 0;
	pos->last_time = time;
	return compensate;
}
#endif

static struct dpm_hwmon_ops *search_dpm_module(const char *name)
{
	struct dpm_hwmon_ops *pos = NULL;
	struct dpm_hwmon_ops *tmp = NULL;

	mutex_lock(&dpm_hwmon_ops_list_lock);
	list_for_each_entry(tmp, &dpm_hwmon_ops_list, ops_list) {
		if (strncasecmp(name, tmp->name, DPM_MODULE_NAME_LENGTH) == 0) {
			pos = tmp;
			break;
		}
	}
	mutex_unlock(&dpm_hwmon_ops_list_lock);
	return pos;
}

static long long get_dpm_power(struct dpm_hwmon_ops *pos)
{
	long long dpm_power = 0;

	if (pos != NULL) {
		mutex_lock(&dpm_hwmon_ops_lock);
#ifdef CONFIG_HISI_DPM_TWO_LEVEL
		dpm_power = get_first_level_power(pos) + get_compensate_power(pos);
#else
		dpm_power = get_first_level_power(pos);
#endif
		mutex_unlock(&dpm_hwmon_ops_lock);
	}
	return dpm_power;
}


long long get_dpm_chdmod_power(const char *name)
{
	struct dpm_hwmon_ops *pos = NULL;
	long long dpm_power = 0;

	if (name != NULL) {
		pos = search_dpm_module(name);
		dpm_power = get_dpm_power(pos);
	}
	return dpm_power;
}

#ifdef CONFIG_HISI_DPM_HWMON_DEBUG

#define MAX_DPM_LOG_SIZE 4096
unsigned long long g_dpm_buffer_for_fitting[DPM_BUFFER_SIZE];

static void print_time_array(unsigned long long *array, int len)
{
	char buf[MAX_DPM_LOG_SIZE] = {'\0'};
	unsigned long long total_us;
	unsigned int i;
	int offset = 0;
	int ret;

	total_us = get_timer_value();
	ret = sprintf_s(buf, MAX_DPM_LOG_SIZE, "[DPM] time : %llu us reg: ", total_us);
	if (ret < 0)
		return;
	else
		offset += ret;

	for (i = 0; i < len; i++) {
		ret = sprintf_s(buf + offset, MAX_DPM_LOG_SIZE - offset, "%llu,", array[i]);
		if (ret < 0)
			return;
		else
			offset += ret;
	}
	if (offset > 0)
		pr_err("%s\n", buf);
}

#ifdef CONFIG_HISI_DEBUG_FS
#define DPM_WRITE_PARANUM		4
#define DPM_MAX_COUNT		1000000

struct dentry *dpm_hwmon_debugfs_root;
static int dpm_debug_show(struct seq_file *m, void *v)
{
	int dpm_id;
	long long dpm_power;

	seq_printf(m, "dpm_report_enabled : %d\n", dpm_report_enabled);
	for (dpm_id = 0; dpm_id < ARRAY_SIZE(dpm_module_table); dpm_id++) {
		dpm_power = get_dpm_chdmod_power(dpm_module_table[dpm_id]);
		seq_printf(m, "%s : %lld\n", dpm_module_table[dpm_id], dpm_power);
	}

	return 0;
}

static int dpm_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, dpm_debug_show, NULL);
}

static ssize_t dpm_debug_write(struct file *file, const char __user *user_buf, size_t count, loff_t *f_pos)
{
	char buf[32] = {'\0'};
	unsigned int i;
	int dpm_id = 0;
	int timer_span_ms = 0;
	int total_count = 0;
	int mode = 0;
	struct dpm_hwmon_ops *pos = NULL;

	if (user_buf == NULL) {
		pr_err("%s:user_buf is NULL!\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(buf, user_buf, min_t(size_t, sizeof(buf) - 1, count))) {
		pr_err("[%s]copy error!\n", __func__);
		return -EFAULT;
	}
	pr_err("%s: buf = %s\n", __func__, buf);

	/* enter: dpm_id, timer_span_ms, total_count */
	if (sscanf_s(buf, "%d %d %d %d", &dpm_id, &timer_span_ms, &total_count, &mode) != DPM_WRITE_PARANUM) {
		pr_err("[%s]the num of enter is wrong!\n", __func__);
		return -EFAULT;
	}

	pr_err("%s: dpm_id = %d, timer_span_ms = %d, total_count = %d, mode = %d!\n",
	       __func__, dpm_id, timer_span_ms, total_count, mode);

	if (dpm_id >= ARRAY_SIZE(dpm_module_table) || dpm_id < 0 || timer_span_ms <= 0 ||
	    total_count < 0 || total_count > DPM_MAX_COUNT) {
		pr_err("illegal input!\n");
		return 0;
	}

	pos = search_dpm_module(dpm_module_table[dpm_id]);
	if (pos != NULL) {
		for (i = 0; i < total_count; i++) {
			mdelay(timer_span_ms);
#ifdef CONFIG_HISI_DPM_TWO_LEVEL
			pos->hi_dpm_get_counter_for_fitting(mode);
#else
			pos->hi_dpm_get_counter_for_fitting();
#endif
			print_time_array(g_dpm_buffer_for_fitting, pos->dpm_cnt_len);
			if (mode != 0)
				pr_err("%s : %lld\n", dpm_module_table[dpm_id], get_dpm_power(pos));
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
#endif
	pr_err("dpm %s\n", __func__);
	dpm_report_enabled = false;

#ifdef CONFIG_HISI_DEBUG_FS
	dpm_hwmon_debugfs_root = debugfs_create_dir("dpm_hwmon", NULL);
	if (dpm_hwmon_debugfs_root == NULL)
		return -ENOENT;

	dpm_hwmon_debug = debugfs_create_file("dpm_debug", 0660, dpm_hwmon_debugfs_root, NULL, &dpm_debug_fops);
	if (dpm_hwmon_debug == NULL)
		goto err_handler;
#endif

	return 0;

#ifdef CONFIG_HISI_DEBUG_FS
err_handler:
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

module_param_named(dpm_report_enable, dpm_report_enabled,
			bool, S_IRUGO | S_IWUSR);

/*lint -e528 -esym(528,*)*/
module_init(dpm_hwmon_debugfs_init);
module_exit(dpm_hwmon_debugfs_exit);
/*lint -e528 +esym(528,*)*/

#endif
