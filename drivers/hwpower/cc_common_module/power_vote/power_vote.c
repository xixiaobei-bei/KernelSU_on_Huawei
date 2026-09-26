/*
 * power_vote.c
 *
 * vote for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/common_module/power_vote.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_vote
HWLOG_REGIST();

static LIST_HEAD(g_power_vote_list);
static DEFINE_SPINLOCK(g_power_vote_list_slock);

static int power_vote_check_type(int type)
{
	if ((type >= POWER_VOTE_TYPE_BEGIN) && (type < POWER_VOTE_TYPE_END))
		return 0;

	hwlog_err("invalid type=%d\n", type);
	return -1;
}

static void power_vote_lock(struct power_vote_object *obj)
{
	if (!obj)
		return;

	mutex_lock(&obj->lock);
}

static void power_vote_unlock(struct power_vote_object *obj)
{
	if (!obj)
		return;

	mutex_unlock(&obj->lock);
}

static int power_vote_get_client_id(struct power_vote_object *obj,
	const char *client_name)
{
	int i;

	for (i = 0; i < POWER_VOTE_MAX_CLIENTS; i++) {
		if (!obj->clients[i].name)
			continue;

		if (strcmp(obj->clients[i].name, client_name) == 0)
			return i;
	}

	/* new client */
	for (i = 0; i < POWER_VOTE_MAX_CLIENTS; i++) {
		if (obj->clients[i].name)
			continue;

		obj->clients[i].name = kstrdup(client_name, GFP_KERNEL);
		return obj->clients[i].name ? i : -EINVAL;
	}

	return -EINVAL;
}

static char *power_vote_get_client_name(struct power_vote_object *obj,
	int client_id)
{
	if (!obj || (client_id < 0))
		return NULL;

	return obj->clients[client_id].name;
}

static bool power_vote_get_client_enabled(struct power_vote_object *obj,
	int client_id)
{
	if (!obj || (client_id < 0))
		return false;

	return obj->clients[client_id].enabled;
}

static int power_vote_get_client_value(struct power_vote_object *obj,
	int client_id)
{
	if (!obj || (client_id < 0))
		return -EINVAL;

	if (obj->type == POWER_VOTE_SET_ANY)
		return obj->clients[client_id].value;

	if (obj->clients[client_id].enabled)
		return obj->clients[client_id].value;

	return -EINVAL;
}

static int power_vote_get_effective_result(struct power_vote_object *obj)
{
	if (!obj)
		return -EINVAL;

	if (obj->override_result != -EINVAL)
		return obj->override_result;

	return obj->eff_result;
}

static void power_vote_set_min(struct power_vote_object *obj,
	int client_id, int *eff_result, int *eff_id)
{
	int i;

	*eff_result = INT_MAX;
	*eff_id = -EINVAL;
	for (i = 0; i < POWER_VOTE_MAX_CLIENTS; i++) {
		if (!obj->clients[i].name)
			break;

		if (!obj->clients[i].enabled)
			continue;

		if (*eff_result <= obj->clients[i].value)
			continue;

		*eff_result = obj->clients[i].value;
		*eff_id = i;
	}

	if (*eff_id == -EINVAL)
		*eff_result = -EINVAL;
}

static void power_vote_set_max(struct power_vote_object *obj,
	int client_id, int *eff_result, int *eff_id)
{
	int i;

	*eff_result = INT_MIN;
	*eff_id = -EINVAL;
	for (i = 0; i < POWER_VOTE_MAX_CLIENTS; i++) {
		if (!obj->clients[i].name)
			break;

		if (!obj->clients[i].enabled)
			continue;

		if (*eff_result >= obj->clients[i].value)
			continue;

		*eff_result = obj->clients[i].value;
		*eff_id = i;
	}

	if (*eff_id == -EINVAL)
		*eff_result = -EINVAL;
}

static void power_vote_set_any(struct power_vote_object *obj,
	int client_id, int *eff_result, int *eff_id)
{
	int i;

	*eff_result = false;
	*eff_id = client_id;
	for (i = 0; i < POWER_VOTE_MAX_CLIENTS; i++) {
		if (!obj->clients[i].name)
			break;

		if (!obj->clients[i].enabled)
			continue;

		*eff_result += (int)obj->clients[i].enabled;
	}

	if (*eff_result)
		*eff_result = true;
}

static struct power_vote_object *power_vote_find_object(const char *name)
{
	unsigned long flags;
	struct power_vote_object *obj = NULL;
	bool found = false;

	if (!name)
		return NULL;

	spin_lock_irqsave(&g_power_vote_list_slock, flags);
	if (list_empty(&g_power_vote_list))
		goto out;

	list_for_each_entry(obj, &g_power_vote_list, list) {
		if (strcmp(obj->name, name) == 0) {
			found = true;
			break;
		}
	}

out:
	spin_unlock_irqrestore(&g_power_vote_list_slock, flags);
	return found ? obj : NULL;
}

int power_vote_create_object(const char *name,
	int vote_type, power_vote_cb cb, void *data)
{
	unsigned long flags;
	struct power_vote_object *obj = NULL;

	if (!name) {
		hwlog_err("name is null\n");
		return -EINVAL;
	}

	if (power_vote_check_type(vote_type))
		return -EINVAL;

	obj = power_vote_find_object(name);
	if (obj) {
		hwlog_err("vote object %s is exist\n", name);
		return -EEXIST;
	}

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj)
		return -ENOMEM;

	obj->name = kstrdup(name, GFP_KERNEL);
	if (!obj->name) {
		kfree(obj);
		return -ENOMEM;
	}

	obj->cb = cb;
	obj->type = vote_type;
	obj->voted_on = false;
	obj->data = data;
	mutex_init(&obj->lock);
	if (obj->type == POWER_VOTE_SET_ANY)
		obj->eff_result = 0;
	else
		obj->eff_result = -EINVAL;
	obj->eff_client_id = -EINVAL;
	obj->override_result = -EINVAL;
	spin_lock_irqsave(&g_power_vote_list_slock, flags);
	list_add(&obj->list, &g_power_vote_list);
	spin_unlock_irqrestore(&g_power_vote_list_slock, flags);

	hwlog_info("vote object %s create ok\n", name);
	return 0;
}

void power_vote_destroy_object(const char *name)
{
	int i;
	unsigned long flags;
	struct power_vote_object *obj = NULL;

	if (!name) {
		hwlog_err("name is null\n");
		return;
	}

	obj = power_vote_find_object(name);
	if (!obj) {
		hwlog_err("vote object %s is not exist\n", name);
		return;
	}

	spin_lock_irqsave(&g_power_vote_list_slock, flags);
	list_del(&obj->list);
	spin_unlock_irqrestore(&g_power_vote_list_slock, flags);

	for (i = 0; i < POWER_VOTE_MAX_CLIENTS; i++) {
		if (!obj->clients[i].name)
			break;
		kfree(obj->clients[i].name);
	}

	mutex_destroy(&obj->lock);
	kfree(obj->name);
	kfree(obj);
}

int power_vote_set(const char *name,
	const char *client_name, bool enabled, int value)
{
	int eff_id = -EINVAL;
	int eff_result = -EINVAL;
	char *eff_client_name = NULL;
	int ret = -EINVAL;
	bool similar_vote = false;
	int client_id;
	struct power_vote_object *obj = NULL;

	if (!name || !client_name) {
		hwlog_err("name or client_name is null\n");
		return ret;
	}

	obj = power_vote_find_object(name);
	if (!obj) {
		hwlog_err("vote object %s is not exist\n", name);
		return ret;
	}

	if (power_vote_check_type(obj->type))
		return ret;

	power_vote_lock(obj);

	client_id = power_vote_get_client_id(obj, client_name);
	if (client_id < 0)
		goto out;

	/*
	 * for POWER_VOTE_SET_ANY the value is to be ignored,
	 * set it to enabled so that the election still works based on
	 * value regardless of the type
	 */
	if (obj->type == POWER_VOTE_SET_ANY)
		value = enabled;

	if ((obj->clients[client_id].enabled == enabled) &&
		(obj->clients[client_id].value == value)) {
		hwlog_info("%s: %s,%d same vote %s of value=%d\n",
			obj->name, client_name, client_id,
			enabled ? "on" : "off", value);
		similar_vote = true;
	}

	obj->clients[client_id].enabled = enabled;
	obj->clients[client_id].value = value;

	if (similar_vote && obj->voted_on) {
		hwlog_info("%s: %s,%d ignore same vote %s of value=%d\n",
			obj->name, client_name, client_id,
			enabled ? "on" : "off", value);
		ret = 0;
		goto out;
	}

	switch (obj->type) {
	case POWER_VOTE_SET_MIN:
		power_vote_set_min(obj, client_id, &eff_result, &eff_id);
		break;
	case POWER_VOTE_SET_MAX:
		power_vote_set_max(obj, client_id, &eff_result, &eff_id);
		break;
	case POWER_VOTE_SET_ANY:
		power_vote_set_any(obj, client_id, &eff_result, &eff_id);
		break;
	default:
		goto out;
	}

	hwlog_info("%s: %s,%d vote %s of value=%d eff_result=%d,%d eff_id=%d\n",
		obj->name, client_name, client_id, enabled ? "on" : "off",
		value, eff_result, obj->eff_result, eff_id);
	ret = 0;
	if (!obj->voted_on || (eff_result != obj->eff_result)) {
		obj->eff_client_id = eff_id;
		obj->eff_result = eff_result;
		eff_client_name = power_vote_get_client_name(obj, eff_id);
		hwlog_info("%s: effective vote is now %d voted by %s,%d\n",
			obj->name, eff_result, eff_client_name, eff_id);
		if (obj->cb && (obj->override_result == -EINVAL))
			ret = obj->cb(obj, obj->data, eff_result, eff_client_name);
	}
	obj->voted_on = true;

out:
	power_vote_unlock(obj);
	return ret;
}

/*
 * the voting client that will override other client's votes.
 * the voting value will be effective only if enabled is set true.
 */
int power_vote_set_override(const char *name,
	const char *client_name, bool enabled, int value)
{
	int ret = -EINVAL;
	struct power_vote_object *obj = NULL;

	if (!name || !client_name) {
		hwlog_err("name or client_name is null\n");
		return ret;
	}

	obj = power_vote_find_object(name);
	if (!obj) {
		hwlog_err("vote object %s is not exist\n", name);
		return ret;
	}

	power_vote_lock(obj);

	if (enabled) {
		ret = obj->cb(obj, obj->data, value, client_name);
		if (!ret)
			obj->override_result = value;
	} else {
		ret = obj->cb(obj, obj->data, obj->eff_result,
			power_vote_get_client_name(obj, obj->eff_client_id));
		obj->override_result = -EINVAL;
	}

	power_vote_unlock(obj);
	return ret;
}

bool power_vote_is_client_enabled_locked(const char *name,
	const char *client_name, bool lock_flag)
{
	bool enabled = false;
	int client_id;
	struct power_vote_object *obj = NULL;

	if (!name || !client_name) {
		hwlog_err("name or client_name is null\n");
		return false;
	}

	obj = power_vote_find_object(name);
	if (!obj) {
		hwlog_err("vote object %s is not exist\n", name);
		return false;
	}

	if (lock_flag)
		power_vote_lock(obj);

	client_id = power_vote_get_client_id(obj, client_name);
	enabled = power_vote_get_client_enabled(obj, client_id);

	if (lock_flag)
		power_vote_unlock(obj);

	hwlog_info("%s: %s,%d get vote enabled=%d\n",
		obj->name, client_name, client_id, enabled);
	return enabled;
}

int power_vote_get_client_value_locked(const char *name,
	const char *client_name, bool lock_flag)
{
	int value;
	int client_id;
	struct power_vote_object *obj = NULL;

	if (!name || !client_name) {
		hwlog_err("name or client_name is null\n");
		return -EINVAL;
	}

	obj = power_vote_find_object(name);
	if (!obj) {
		hwlog_err("vote object %s is not exist\n", name);
		return -EINVAL;
	}

	if (lock_flag)
		power_vote_lock(obj);

	client_id = power_vote_get_client_id(obj, client_name);
	value = power_vote_get_client_value(obj, client_id);

	if (lock_flag)
		power_vote_unlock(obj);

	hwlog_info("%s: %s,%d get vote value=%d\n",
		obj->name, client_name, client_id, value);
	return value;
}

int power_vote_get_effective_result_locked(const char *name, bool lock_flag)
{
	int value;
	struct power_vote_object *obj = NULL;

	if (!name) {
		hwlog_err("name is null\n");
		return -EINVAL;
	}

	obj = power_vote_find_object(name);
	if (!obj) {
		hwlog_err("vote object %s is not exist\n", name);
		return -EINVAL;
	}

	if (lock_flag)
		power_vote_lock(obj);

	value = power_vote_get_effective_result(obj);

	if (lock_flag)
		power_vote_unlock(obj);

	hwlog_info("%s: get vote eff_result=%d\n", obj->name, value);
	return value;
}

static ssize_t power_vote_show_client_info(struct power_vote_object *obj,
	char *buf, size_t max_size, size_t cur_size)
{
	int i;
	char *type_str = NULL;
	char *eff_client_name = NULL;

	switch (obj->type) {
	case POWER_VOTE_SET_MIN:
		type_str = "Set_Min";
		break;
	case POWER_VOTE_SET_MAX:
		type_str = "Set_Max";
		break;
	case POWER_VOTE_SET_ANY:
		type_str = "Set_Any";
		break;
	default:
		type_str = "Unkonwn";
		break;
	}

	if (cur_size >= (max_size - POWER_VOTE_RD_BUF_SIZE))
		return cur_size;
	eff_client_name = power_vote_get_client_name(obj, obj->eff_client_id);
	cur_size += scnprintf(buf + cur_size, POWER_VOTE_RD_BUF_SIZE,
		"\n%s: type=%s eff_result=%d eff_client_name=%s eff_id=%d\n",
		obj->name, type_str,
		obj->eff_result,
		eff_client_name ? eff_client_name : "None",
		obj->eff_client_id);

	power_vote_lock(obj);

	for (i = 0; i < POWER_VOTE_MAX_CLIENTS; i++) {
		if (!obj->clients[i].name)
			continue;

		if (cur_size >= (max_size - POWER_VOTE_RD_BUF_SIZE))
			break;

		cur_size += scnprintf(buf + cur_size, POWER_VOTE_RD_BUF_SIZE,
			"\t%s: enabled=%d value=%d\n",
			obj->clients[i].name,
			obj->clients[i].enabled,
			obj->clients[i].value);
	}

	power_vote_unlock(obj);
	return cur_size;
}

static ssize_t power_vote_object_para_show(void *dev_data,
	char *buf, size_t size)
{
	ssize_t len = 0;
	unsigned long flags;
	struct power_vote_object *obj = NULL;

	spin_lock_irqsave(&g_power_vote_list_slock, flags);
	if (list_empty(&g_power_vote_list))
		goto out;

	list_for_each_entry(obj, &g_power_vote_list, list) {
		if (!obj)
			continue;
		len = power_vote_show_client_info(obj, buf, size, len);
	}

out:
	spin_unlock_irqrestore(&g_power_vote_list_slock, flags);
	return len;
}

static int __init power_vote_init(void)
{
	power_dbg_ops_register("vote_object", NULL,
		(power_dbg_show)power_vote_object_para_show, NULL);

	return 0;
}

static void __exit power_vote_exit(void)
{
}

subsys_initcall_sync(power_vote_init);
module_exit(power_vote_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("vote for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
