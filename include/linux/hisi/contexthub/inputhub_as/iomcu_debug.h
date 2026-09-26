/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019. All rights reserved.
 * Description: iomcu_debug.h.
 * Create: 2019/11/05
 */
#ifndef __IOMCU_DEBUG_H__
#define __IOMCU_DEBUG_H__

#include <linux/list.h>
#include <linux/mutex.h>

#define register_sensorhub_debug_operation(func) \
register_sensorhub_debug_op_and_opstr(#func, func);

#define unregister_sensorhub_debug_operation(func) \
unregister_sensorhub_debug_op_and_opstr(func);

typedef int (*sensor_debug_pfunc) (int tag, int argv[], int argc);

struct t_sensor_debug_operations_list {
	struct list_head head;
	struct mutex mlock;
};

/* to find operation by str */
struct sensor_debug_cmd {
	const char *str;
	sensor_debug_pfunc operation;
	struct list_head entry;
};

/* to find tag by str */
struct sensor_debug_tag_map {
	const char *str;
	int tag;
};

/* to search info by tag */
struct sensor_debug_search_info {
	const char *sensor;
	int tag;
	void *private_data;
};

struct power_dbg {
	const char *name;
	const struct attribute_group *attrs_group;
	struct device *dev;
};

#define MAX_CMD_BUF_ARGC 64

#define SINGLE_STR_LENGTH_MAX 30

#endif /* __IOMCU_DEBUG_H__ */
