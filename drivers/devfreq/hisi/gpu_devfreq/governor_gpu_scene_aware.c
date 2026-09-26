/*
 * governor_gpu_scene_aware.c
 *
 * gpu governor of devfreq framework
 *
 * Copyright (C) 2017-2020 Huawei Technologies Co., Ltd.
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

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/devfreq.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/of.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <governor.h>
#include <securec.h>
#include <linux/hisi/gpufreq.h>

#ifdef CONFIG_HUAWEI_DUBAI
#include <huawei_platform/power/dubai/dubai_gpu_freq_stats.h>
#endif
#if defined(CONFIG_GPU_VIRTUAL_DEVFREQ) || \
	defined(CONFIG_GPU_THROTTLE_DEVFREQ)
#include <linux/hisi/gpu_hook.h>
#endif

#define DEFAULT_GO_HISPEED_LOAD		90
#define DEFAULT_HISPEED_FREQ		533000000
#define DEFAULT_VSYNC_EQULALIZE		45
#define DEFAULT_LOADING_WINDOW		10
#define TARGET_LOAD			85
#define NTARGET_LOAD			1
#define MAX_LOADING_WINDOW		50
#define DFMO_MIN_OPENCL_BOOST_FREQ	332000000UL
#define DFMO_MAX_OPENCL_BOOST_FREQ	667000000UL
#define DFMO_DEFAULT_OPENCL_BOOST_FREQ	415000000UL
#define SURPORT_POLICY_NUM		20
#define SURPORT_NTARGET_LOAD_MAX	40
#define POLICY_BUF_MAX			1024
#define POLICY_ID_BUF_MAX		10

#define SCENE_AWARE_MAX_TOKEN		2

#define PERCENT_TRANSFER_DIV		100
#define PERCENT_UP_LIMIT		100
#define PERCENT_DOWN_LIMIT		0
#define DECIMAL			10

enum {
	INDEX,
	GO_HISPEED_LOAD,
	HISPEED_FREQ,
	VSYNC_EQUALIZE,
	LOADING_WINDOW,
	MAX_PARA
};

const char *policy_para_name[MAX_PARA] = {
	"policy   index",
	"hispeed   load",
	"hispeed   freq",
	"vsync equalize",
	"load    window"
};

struct scene_aware_policy {
	unsigned int para[MAX_PARA];
	unsigned int *target_load;
	unsigned int ntarget_load;
	struct list_head node;
};

struct devfreq_gpu_scene_aware_data {
	unsigned long buffer[MAX_LOADING_WINDOW];
	unsigned long util_sum;
	unsigned long utilisation;
	unsigned long normalized_util;
	unsigned long table_max_freq;
	unsigned long user_set_freq;
	unsigned long cl_boost_freq;
	unsigned int window_counter;
	unsigned int window_idx;
	int vsync;
	int cl_boost;
	struct list_head policy_list;
	struct scene_aware_policy *cur_policy;
};

static int devfreq_get_dev_status(struct devfreq *df,
				  struct devfreq_dev_status *stat)
{
	int err = df->profile->get_dev_status(df->dev.parent, stat);

#ifdef CONFIG_HUAWEI_DUBAI
	if (stat->busy_time && stat->current_frequency)
		dubai_update_gpu_info(stat->current_frequency, stat->busy_time,
				      stat->total_time, df->profile->polling_ms);
#endif

	return err;
}

static void devfreq_gpu_scene_aware_apply_limits(struct devfreq *df,
						 unsigned long *freq)
{
	struct devfreq_gpu_scene_aware_data *data = NULL;
#ifdef CONFIG_GPU_VIRTUAL_DEVFREQ
	unsigned long virtual_freq = 0;
	unsigned long devfreq_min = 0;
	bool virtual_freq_flag = false;
#endif

	data = df->data;
	if (data == NULL)
		return;

	/* Not less than cl_boost_freq, if necessary. */
	if (data->cl_boost != 0 && *freq < data->cl_boost_freq)
		*freq = data->cl_boost_freq;
	if (data->user_set_freq != 0)
		*freq = data->user_set_freq;
#ifdef CONFIG_GPU_THROTTLE_DEVFREQ
	gpu_devfreq_set_gpu_throttle(df->min_freq, df->max_freq, freq);
#endif
#ifdef CONFIG_GPU_VIRTUAL_DEVFREQ
	if (data->user_set_freq == 0) {
		gpu_virtual_devfreq_get_devfreq_min(&devfreq_min);
		if (*freq < devfreq_min && df->min_freq <= devfreq_min) {
			virtual_freq = *freq;
			virtual_freq_flag = true;
		}
	}
#endif

	if (df->min_freq != 0 && *freq < df->min_freq)
		*freq = df->min_freq;
	if (df->max_freq != 0 && *freq > df->max_freq)
		*freq = df->max_freq;
#ifdef CONFIG_GPU_VIRTUAL_DEVFREQ
	gpu_virtual_devfreq_set_core_mask(virtual_freq, virtual_freq_flag);
#endif
}

#ifdef CONFIG_GPU_VIRTUAL_DEVFREQ
static void virtual_devfreq_set_mode(unsigned long user_set_freq)
{
	if (user_set_freq != 0)
		gpu_virtual_devfreq_set_mode(false);
	else
		gpu_virtual_devfreq_set_mode(true);
}

static void virtual_devfreq_get_freq(unsigned long user_set_freq,
				     unsigned long *freq)
{
	unsigned long devfreq_min = 0;

	gpu_virtual_devfreq_get_devfreq_min(&devfreq_min);
	if (user_set_freq == 0 && devfreq_min == *freq)
		gpu_virtual_devfreq_get_freq(freq);
}
#endif

static int devfreq_gpu_scene_aware_func(struct devfreq *df,
					unsigned long *freq)
{
	unsigned int i;
	unsigned int targetload, vsync_equalize, util;
	unsigned long a;
	int err;
	struct devfreq_dev_status stat = { .busy_time = 0, .total_time = 100 };
	struct devfreq_gpu_scene_aware_data *data = NULL;
	struct devfreq_data *priv_data = NULL;

	data = df->data;
	if (data == NULL)
		return -EINVAL;
#ifdef CONFIG_GPU_VIRTUAL_DEVFREQ
	virtual_devfreq_set_mode(data->user_set_freq);
#endif

	err = devfreq_get_dev_status(df, &stat);
	if (err != 0)
		return err;

	priv_data = stat.private_data;

	if (priv_data == NULL)
		return -EINVAL;

	*freq = stat.current_frequency;
#ifdef CONFIG_GPU_VIRTUAL_DEVFREQ
	virtual_devfreq_get_freq(data->user_set_freq, freq);
#endif
	if (unlikely(stat.total_time == 0 || *freq == 0)) {
		*freq = data->cur_policy->para[HISPEED_FREQ];
		goto check_barrier;
	}

	util = stat.busy_time * PERCENT_TRANSFER_DIV / stat.total_time;
	data->util_sum += (*freq) * util;
	data->vsync = (stat.private_data != NULL) ? 1 : 0;
	data->cl_boost = (priv_data->cl_boost != 0) ? 1 : 0;
	vsync_equalize = (data->vsync != 0) ?
			 PERCENT_UP_LIMIT : data->cur_policy->para[VSYNC_EQUALIZE];

	if (data->window_counter >= data->cur_policy->para[LOADING_WINDOW])
		data->util_sum -= data->buffer[data->window_idx];
	else
		data->window_counter++;

	data->buffer[data->window_idx] = (*freq) * util;
	data->window_idx += 1;
	data->window_idx = data->window_idx %
			   data->cur_policy->para[LOADING_WINDOW];
	a = div_u64(data->util_sum, data->window_counter);
	data->utilisation = div64_u64(a, *freq);

	if (data->table_max_freq != 0)
		data->normalized_util = div64_u64(a, data->table_max_freq);

	if (data->user_set_freq > 0)
		goto check_barrier;

	if (data->window_counter >= data->cur_policy->para[LOADING_WINDOW]) {
		for (i = 0; i < data->cur_policy->ntarget_load - 1 && *freq >= data->cur_policy->target_load[i + 1]; i += 2)
			;

		targetload = data->cur_policy->target_load[i] *
			     vsync_equalize / PERCENT_TRANSFER_DIV;

		*freq = div_u64(a, targetload);
	}

	targetload = data->cur_policy->para[GO_HISPEED_LOAD] *
		     vsync_equalize / PERCENT_TRANSFER_DIV;
	if (util > targetload && data->cur_policy->para[HISPEED_FREQ] > *freq)
		*freq = data->cur_policy->para[HISPEED_FREQ];

check_barrier:
	devfreq_gpu_scene_aware_apply_limits(df, freq);

	return 0;
}

#define show_one(object)					\
static ssize_t show_##object					\
(struct device *dev, struct device_attribute *attr, char *buf)	\
{								\
	struct devfreq *devfreq = to_devfreq(dev);		\
	struct devfreq_gpu_scene_aware_data *data = NULL;	\
	int ret;						\
	mutex_lock(&devfreq->lock);				\
	data = devfreq->data;					\
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,	\
			 "%u\n", (unsigned int)data->object);	\
	mutex_unlock(&devfreq->lock);				\
	return ret;						\
}

show_one(vsync)
show_one(utilisation)
show_one(normalized_util)
show_one(cl_boost)
show_one(cl_boost_freq)
show_one(user_set_freq)

static void refresh_load_buffer(struct devfreq *devfreq,
				struct scene_aware_policy *new_policy)
{
	unsigned long util_avg;
	unsigned int i;
	struct devfreq_gpu_scene_aware_data *data = devfreq->data;

	if (data->cur_policy->para[LOADING_WINDOW] == new_policy->para[LOADING_WINDOW] ||
	    data->window_counter == 0)
		return;

	util_avg = div_u64(data->util_sum, data->window_counter);
	if (data->window_counter >= new_policy->para[LOADING_WINDOW]) {
		data->util_sum = util_avg * new_policy->para[LOADING_WINDOW];
		data->window_counter = new_policy->para[LOADING_WINDOW];
	} else {
		data->util_sum = util_avg * data->window_counter;
	}

	for (i = 0; i < new_policy->para[LOADING_WINDOW]; i++)
		data->buffer[i] = util_avg;

	data->window_idx = 0;
}

static ssize_t store_scene(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf,
			   size_t count)
{
	int ret;
	unsigned int input;
	struct devfreq *devfreq = to_devfreq(dev);
	struct devfreq_gpu_scene_aware_data *data = devfreq->data;
	struct scene_aware_policy *policy = NULL;
	char local_buf[POLICY_ID_BUF_MAX] = {0};

	if (count >= POLICY_ID_BUF_MAX)
		return -EINVAL;

	ret = strncpy_s(local_buf, POLICY_ID_BUF_MAX, buf, count);
	if (ret != EOK)
		return -EINVAL;

	ret = kstrtouint(local_buf, DECIMAL, &input);
	if (ret != 0)
		return -EINVAL;

	if (data->cur_policy->para[INDEX] == input)
		return count;

	mutex_lock(&devfreq->lock);
	list_for_each_entry(policy, &data->policy_list, node) {
		if (policy->para[INDEX] == input) {
			refresh_load_buffer(devfreq, policy);
			data->cur_policy = policy;
			break;
		}
	}
	mutex_unlock(&devfreq->lock);
	return count;
}

static ssize_t show_scene(struct device *dev,
			  struct device_attribute *attr,
			  char *buf)
{
	int ret;
	struct devfreq *devfreq = to_devfreq(dev);
	struct devfreq_gpu_scene_aware_data *data = NULL;

	mutex_lock(&devfreq->lock);
	data = devfreq->data;
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			 "%u\n", data->cur_policy->para[INDEX]);
	mutex_unlock(&devfreq->lock);

	return ret;
}

static int para_check(struct scene_aware_policy *new_policy)
{
	unsigned int i;

	if (new_policy->para[VSYNC_EQUALIZE] == PERCENT_DOWN_LIMIT ||
	    new_policy->para[VSYNC_EQUALIZE] > PERCENT_UP_LIMIT ||
	    new_policy->para[LOADING_WINDOW] == 0 ||
	    new_policy->para[LOADING_WINDOW] > MAX_LOADING_WINDOW ||
	    new_policy->para[GO_HISPEED_LOAD] == PERCENT_DOWN_LIMIT ||
	    new_policy->para[GO_HISPEED_LOAD] > PERCENT_UP_LIMIT ||
	    new_policy->para[INDEX] >= SURPORT_POLICY_NUM)
		return -1;

	for (i = 0; i < new_policy->ntarget_load; i += 2) {
		if (new_policy->target_load[i] > PERCENT_UP_LIMIT ||
		    new_policy->target_load[i] == PERCENT_DOWN_LIMIT)
			return -1;
	}

	return 0;
}

static int extract_sub_para(const char *buf,
			    unsigned int *para,
			    unsigned int ntokens)
{
	const char *cp = buf;
	unsigned int i = 0;

	while (i < ntokens) {
		if (sscanf_s(cp, "%u", &para[i++]) != 1)
			return -1;

		cp = strpbrk(cp, " :");
		if (cp == NULL)
			break;
		cp++;
	}

	return 0;
}

static inline bool check_tokens(const unsigned int *ntokens_sub)
{
	return ((ntokens_sub[1] & 0x1) == 0 ||
		ntokens_sub[1] > SURPORT_NTARGET_LOAD_MAX ||
		ntokens_sub[0] != MAX_PARA);
}

static struct scene_aware_policy *get_policy(const char *buf)
{
	int i;
	int ntokens = 1;
	int err = -EINVAL;
	const char *cp_sub[SCENE_AWARE_MAX_TOKEN] = {NULL};
	const char *cp = buf;
	unsigned int ntokens_sub[SCENE_AWARE_MAX_TOKEN] = { 1, 1 };
	struct scene_aware_policy *policy = NULL;

	cp_sub[0] = buf;
	while ((cp = strpbrk(cp, ","))) {
		cp_sub[ntokens] = ++cp;
		ntokens++;
		if (ntokens >= SCENE_AWARE_MAX_TOKEN)
			break;
	}

	if (ntokens == 1) {
		pr_err("ntokens not enough\n");
		goto err_parse;
	}

	for (i = 0; i < ntokens; i++) {
		cp = cp_sub[i];

		while ((cp = strpbrk(cp + 1, " :"))) {
			if (i != (ntokens - 1) && cp > cp_sub[i + 1])
				break;

			ntokens_sub[i]++;
		}
	}

	if (check_tokens(ntokens_sub)) {
		pr_err("sub ntokens err:%u,%u\n",
		       ntokens_sub[0], ntokens_sub[1]);
		goto err_parse;
	}

	policy = kzalloc(sizeof(struct scene_aware_policy), GFP_KERNEL);
	if (policy == NULL) {
		pr_err("alloc policy fail\n");
		err = -ENOMEM;
		goto err_parse;
	}

	if (extract_sub_para(cp_sub[0], policy->para, ntokens_sub[0]) != 0) {
		pr_err("%s extract policy para fail\n", __func__);
		goto err_policy;
	}

	policy->ntarget_load = ntokens_sub[1];
	policy->target_load =
		kzalloc(ntokens_sub[1] * sizeof(unsigned int), GFP_KERNEL);
	if (IS_ERR_OR_NULL(policy->target_load)) {
		pr_err("alloc target load fail\n");
		err = -ENOMEM;
		goto err_policy;
	}

	if (extract_sub_para(cp_sub[1], policy->target_load,
			     policy->ntarget_load) != 0) {
		pr_err("%s extract target load fail\n", __func__);
		goto err_target_load;
	}

	if (para_check(policy) != 0) {
		pr_err("%s para check error\n", __func__);
		goto err_target_load;
	}

	return policy;

err_target_load:
	kfree(policy->target_load);
err_policy:
	kfree(policy);
err_parse:
	return ERR_PTR(err);
}

static void release_policy(struct scene_aware_policy *policy)
{
	list_del(&policy->node);

	kfree(policy->target_load);

	kfree(policy);
}

static ssize_t show_scene_para(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	int ret;
	unsigned int i;
	ssize_t count = 0;
	char *index_name = NULL;
	struct devfreq *devfreq = to_devfreq(dev);
	struct devfreq_gpu_scene_aware_data *data = devfreq->data;
	struct scene_aware_policy *policy = NULL;

	mutex_lock(&devfreq->lock);

	list_for_each_entry(policy, &data->policy_list, node) {
		index_name = (policy == data->cur_policy) ? "->" : "  ";
		for (i = 0; i < MAX_PARA; i++) {
			ret = snprintf_s(buf + count, PAGE_SIZE - count,
					 PAGE_SIZE - count - 1, "  %s:  %d\n",
					 i == INDEX ? index_name : policy_para_name[i],
					 policy->para[i]);
			if (ret < 0)
				goto err_ret;

			count += ret;
			if ((unsigned int)count >= PAGE_SIZE)
				goto err_ret;
		}

		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1,
				 "  target load:   ");
		if (ret < 0)
			goto err_ret;

		count += ret;
		if ((unsigned int)count >= PAGE_SIZE)
			goto err_ret;

		if (IS_ERR_OR_NULL(policy->target_load))
			continue;

		for (i = 0; i < policy->ntarget_load - 1; i++) {
			ret = snprintf_s(buf + count, PAGE_SIZE - count,
					 PAGE_SIZE - count - 1, "%d:",
					 policy->target_load[i]);
			if (ret < 0)
				goto err_ret;

			count += ret;
			if ((unsigned int)count >= PAGE_SIZE)
				goto err_ret;
		}
		ret = snprintf_s(buf + count, PAGE_SIZE - count,
				 PAGE_SIZE - count - 1, "%d\n",
				 policy->target_load[i]);
		if (ret < 0)
			goto err_ret;

		count += ret;
		if ((unsigned int)count >= PAGE_SIZE)
			goto err_ret;
	}

err_ret:
	mutex_unlock(&devfreq->lock);
	return count;
}

static int policy_para_parse(struct devfreq *devfreq,
			     const char *buf,
			     size_t count)
{
	int ret;
	struct devfreq_gpu_scene_aware_data *data = NULL;
	struct scene_aware_policy *new_policy = NULL;
	struct scene_aware_policy *policy = NULL;
	char local_buf[POLICY_BUF_MAX] = {0};

	if (count >= POLICY_BUF_MAX)
		return -EINVAL;

	ret = strncpy_s(local_buf, POLICY_BUF_MAX, buf, count);
	if (ret != EOK)
		return -EINVAL;

	mutex_lock(&devfreq->lock);
	data = devfreq->data;
	new_policy = get_policy(local_buf);
	if (IS_ERR_OR_NULL(new_policy)) {
		pr_err("%s get policy fail\n", __func__);
		mutex_unlock(&devfreq->lock);
		return -EINVAL;
	}

	list_for_each_entry(policy, &data->policy_list, node) {
		if (policy->para[INDEX] == new_policy->para[INDEX]) {
			if (data->cur_policy == policy) {
				refresh_load_buffer(devfreq, new_policy);
				data->cur_policy = new_policy;
			}

			/* free old policy */
			release_policy(policy);
			break;
		}
	}

	list_add_tail(&new_policy->node, &data->policy_list);
	mutex_unlock(&devfreq->lock);

	return 0;
}

static ssize_t store_scene_para(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t count)
{
	int ret;
	struct devfreq *devfreq = to_devfreq(dev);

	ret = policy_para_parse(devfreq, buf, count);
	if (ret != 0)
		return -EINVAL;

	return count;
}

static ssize_t store_cl_boost(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf,
			      size_t count)
{
	int input;
	int ret;
	struct devfreq *devfreq = to_devfreq(dev);
	struct devfreq_gpu_scene_aware_data *data = NULL;

	ret = kstrtoint(buf, DECIMAL, &input);
	if (ret != 0 || input < 0)
		return -EINVAL;

	mutex_lock(&devfreq->lock);
	data = devfreq->data;
	data->cl_boost = input != 0 ? 1 : 0;
	ret = update_devfreq(devfreq);
	if (ret == 0)
		ret = count;
	mutex_unlock(&devfreq->lock);

	return ret;
}

static ssize_t store_cl_boost_freq(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf,
				   size_t count)
{
	unsigned long input;
	struct devfreq *devfreq = to_devfreq(dev);
	struct devfreq_gpu_scene_aware_data *data = NULL;
	int ret = kstrtoul(buf, DECIMAL, &input);
	if (ret != 0 ||
	    input > DFMO_MAX_OPENCL_BOOST_FREQ ||
	    input < DFMO_MIN_OPENCL_BOOST_FREQ)
		return -EINVAL;

	mutex_lock(&devfreq->lock);
	data = devfreq->data;
	data->cl_boost_freq = input;
	ret = update_devfreq(devfreq);
	if (ret == 0)
		ret = count;
	mutex_unlock(&devfreq->lock);

	return ret;
}

static ssize_t store_user_set_freq(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	unsigned long input;
	struct devfreq *devfreq = to_devfreq(dev);
	struct devfreq_gpu_scene_aware_data *data = NULL;
	int ret = kstrtoul(buf, DECIMAL, &input);
	if (ret != 0)
		return -EINVAL;

	mutex_lock(&devfreq->lock);
	data = devfreq->data;
	if (data->table_max_freq != 0)
		input = min(data->table_max_freq, input);

	data->user_set_freq = input;
	ret = update_devfreq(devfreq);
	if (ret == 0)
		ret = count;

	mutex_unlock(&devfreq->lock);

	return ret;
}

#define GPU_SCENE_AWARE_ATTR_RW(_name) \
	static DEVICE_ATTR(_name, 0644, show_##_name, store_##_name)

GPU_SCENE_AWARE_ATTR_RW(scene);
GPU_SCENE_AWARE_ATTR_RW(scene_para);
GPU_SCENE_AWARE_ATTR_RW(cl_boost);
GPU_SCENE_AWARE_ATTR_RW(cl_boost_freq);
GPU_SCENE_AWARE_ATTR_RW(user_set_freq);

#ifdef CONFIG_GPU_VIRTUAL_DEVFREQ
static ssize_t show_vfreq_enable(struct device *dev,
				 struct device_attribute *attr,
				 char * const buf)
{
	int ret;
	bool flag = false;
	struct devfreq *devfreq = to_devfreq(dev);

	mutex_lock(&devfreq->lock);
	gpu_virtual_devfreq_get_enable(&flag);
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			 "%d\n", flag);
	mutex_unlock(&devfreq->lock);

	return ret;
}

static ssize_t store_vfreq_enable(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned long input;
	bool flag = false;
	struct devfreq *devfreq = to_devfreq(dev);
	int ret = kstrtoul(buf, DECIMAL, &input);
	if (ret != 0)
		return -EINVAL;
	if (input == 0)
		flag = false;
	else
		flag = true;

	mutex_lock(&devfreq->lock);
	gpu_virtual_devfreq_set_enable(flag);
	mutex_unlock(&devfreq->lock);
	if (ret == 0)
		ret = count;

	return ret;
}

GPU_SCENE_AWARE_ATTR_RW(vfreq_enable);
#endif

#define GPU_SCENE_AWARE_ATTR_RO(_name) \
	static DEVICE_ATTR(_name, 0444, show_##_name, NULL)

GPU_SCENE_AWARE_ATTR_RO(vsync);
GPU_SCENE_AWARE_ATTR_RO(utilisation);
GPU_SCENE_AWARE_ATTR_RO(normalized_util);

static struct attribute *dev_entries[] = {
	&dev_attr_scene.attr,
	&dev_attr_scene_para.attr,
	&dev_attr_vsync.attr,
	&dev_attr_cl_boost.attr,
	&dev_attr_cl_boost_freq.attr,
	&dev_attr_user_set_freq.attr,
	&dev_attr_utilisation.attr,
	&dev_attr_normalized_util.attr,
#ifdef CONFIG_GPU_VIRTUAL_DEVFREQ
	&dev_attr_vfreq_enable.attr,
#endif
	NULL,
};

static struct attribute_group dev_attr_group = {
	.name = "gpu_scene_aware",
	.attrs = dev_entries,
};

static void gpufreq_scene_aware_policy_init(struct devfreq *devfreq)
{
#ifdef CONFIG_GPUFREQ_GOV_POLICY_USING_DTSI
	int ret, count, i;
	unsigned int cl_boost_freq;
	struct device_node *np = NULL;
	const char *scene_para = NULL;
	struct devfreq_gpu_scene_aware_data *data = devfreq->data;

	np = of_find_compatible_node(NULL, NULL, "hisi,scene-aware");
	if (np == NULL) {
		pr_err("%s: find hisi,scene-aware fail\n", __func__);
		return;
	}

	ret = of_property_read_u32(np, "cl-boost-freq", &cl_boost_freq);
	if (ret != 0)
		pr_err("%s: read cl-boost-freq fail\n", __func__);
	else
		data->cl_boost_freq = cl_boost_freq;

	count = of_property_count_strings(np, "scene-para");
	if (count <= 0) {
		pr_err("%s: count scene-para err %d\n", __func__, count);
		of_node_put(np);
		return;
	}

	count = min(count, SURPORT_POLICY_NUM);
	for (i = 0; i < count; i++) {
		ret = of_property_read_string_index(np, "scene-para", i, &scene_para);
		if (ret != 0)
			continue;

		ret = policy_para_parse(devfreq, scene_para, strlen(scene_para) + 1);
		if (ret != 0) {
			pr_err("%s: parse scene-para %s fail\n", __func__, scene_para);
			of_node_put(np);
			return;
		}
	}
	of_node_put(np);
#endif
}

static int gpu_scene_aware_init(struct devfreq *devfreq)
{
	int err = -ENOMEM;
	struct devfreq_gpu_scene_aware_data *data = NULL;
	struct devfreq_dev_profile *profile = NULL;

	if (devfreq->data != NULL)
		goto err_out;

	if (IS_ERR_OR_NULL(devfreq->profile))
		goto err_out;

	profile = devfreq->profile;

	data = kzalloc(sizeof(struct devfreq_gpu_scene_aware_data), GFP_KERNEL);
	if (data == NULL) {
		pr_err("%s: alloc data err\n", __func__);
		goto err_out;
	}

	data->cur_policy = kzalloc(sizeof(struct scene_aware_policy), GFP_KERNEL);
	if (data->cur_policy == NULL) {
		pr_err("%s: alloc cur policy err\n", __func__);
		goto err_sa_data;
	}

	data->cur_policy->target_load = kzalloc(sizeof(unsigned int), GFP_KERNEL);
	if (data->cur_policy->target_load == NULL) {
		pr_err("%s: alloc target load err\n", __func__);
		goto err_cur_policy;
	}

	data->cur_policy->para[INDEX] = 0;
	data->cur_policy->para[GO_HISPEED_LOAD] = DEFAULT_GO_HISPEED_LOAD;
	data->cur_policy->para[HISPEED_FREQ] = DEFAULT_HISPEED_FREQ;
	data->cur_policy->para[VSYNC_EQUALIZE] = DEFAULT_VSYNC_EQULALIZE;
	data->cur_policy->para[LOADING_WINDOW] = DEFAULT_LOADING_WINDOW;
	data->cur_policy->ntarget_load = NTARGET_LOAD;
	data->cl_boost_freq = DFMO_DEFAULT_OPENCL_BOOST_FREQ;

	if (profile->max_state > 0 && profile->freq_table != NULL)
		data->table_max_freq = profile->freq_table[profile->max_state - 1];

	*(data->cur_policy->target_load) = TARGET_LOAD;
	INIT_LIST_HEAD(&data->policy_list);
	list_add(&data->cur_policy->node, &data->policy_list);

	devfreq->data = data;
	gpufreq_scene_aware_policy_init(devfreq);

	err = sysfs_create_group(&devfreq->dev.kobj, &dev_attr_group);
	if (err != 0) {
		pr_err("%s: sysfs create err %d\n", __func__, err);
		goto err_target_load;
	}

#ifdef CONFIG_HUAWEI_DUBAI
	dubai_set_gpu_freq_num(profile->max_state);
#endif

	return 0;

err_target_load:
	kfree(data->cur_policy->target_load);
err_cur_policy:
	kfree(data->cur_policy);
err_sa_data:
	kfree(data);
	devfreq->data = NULL;
err_out:
	return err;
}

static void gpu_scene_aware_exit(struct devfreq *devfreq)
{
	struct devfreq_gpu_scene_aware_data *data = NULL;
	struct scene_aware_policy *policy = NULL;

	data = devfreq->data;
	if (IS_ERR_OR_NULL(data))
		return;

	sysfs_remove_group(&devfreq->dev.kobj, &dev_attr_group);

	while (!list_empty(&data->policy_list)) {
		policy = list_first_entry(&data->policy_list,
					  struct scene_aware_policy, node);

		release_policy(policy);
	}

	kfree(data);
	devfreq->data = NULL;
}

static void gov_suspend(struct devfreq *devfreq)
{
#ifdef CONFIG_GPU_RT_DEVFREQ
	int ret;

	mutex_lock(&devfreq->lock);
	ret = update_devfreq(devfreq);
	if (ret != 0)
		dev_err(&devfreq->dev, "%s update devfreq fail%d\n", __func__, ret);
	mutex_unlock(&devfreq->lock);
#else
#ifdef CONFIG_HUAWEI_DUBAI
	struct devfreq_dev_status stat;
	/*
	 * we have to get the busy/freq info,
	 * and report to dubai before devfreq suspend.
	 */
	devfreq_get_dev_status(devfreq, &stat);
#endif
#endif
	devfreq_monitor_suspend(devfreq);
}

static void gov_resume(struct devfreq *devfreq)
{
	devfreq_monitor_resume(devfreq);
}

static int devfreq_gpu_scene_aware_handler(struct devfreq *devfreq,
					   unsigned int event,
					   void *data)
{
	int ret = 0;

	switch (event) {
	case DEVFREQ_GOV_START:
		ret = gpu_scene_aware_init(devfreq);
		if (ret == 0) {
			/* only for init delay work */
			devfreq_monitor_start(devfreq);
			devfreq_monitor_suspend(devfreq);
		}
		break;
	case DEVFREQ_GOV_STOP:
		devfreq_monitor_stop(devfreq);
		gpu_scene_aware_exit(devfreq);
		break;
	case DEVFREQ_GOV_INTERVAL:
		devfreq_interval_update(devfreq, (unsigned int *)data);
		break;
	case DEVFREQ_GOV_SUSPEND:
		gov_suspend(devfreq);
		break;
	case DEVFREQ_GOV_RESUME:
		gov_resume(devfreq);
		break;
	case DEVFREQ_GOV_LIMITS:
		devfreq_gpu_scene_aware_apply_limits(devfreq, (unsigned long *)data);
		break;
	default:
		break;
	}

	return ret;
}

static struct devfreq_governor devfreq_gpu_scene_aware = {
	.name = "gpu_scene_aware",
	.immutable = 1,
	.get_target_freq = devfreq_gpu_scene_aware_func,
	.event_handler = devfreq_gpu_scene_aware_handler,
};

static int __init devfreq_gpu_scene_aware_init(void)
{
	return devfreq_add_governor(&devfreq_gpu_scene_aware);
}
subsys_initcall(devfreq_gpu_scene_aware_init);

static void __exit devfreq_gpu_scene_aware_exit(void)
{
	int ret;

	ret = devfreq_remove_governor(&devfreq_gpu_scene_aware);
	if (ret != 0)
		pr_err("%s: failed remove governor %d\n", __func__, ret);
}
module_exit(devfreq_gpu_scene_aware_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("gpu governor of devfreq framework");
