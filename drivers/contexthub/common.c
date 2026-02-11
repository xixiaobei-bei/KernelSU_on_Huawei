/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub common driver.
 * Author: Huawei
 * Create: 2017-07-21
 */
#include <linux/of.h>
#include <linux/of_address.h>
#include "inputhub_api.h"

/*
 * 从DTS获取contexthub开关状态
 */
int get_contexthub_dts_status(void)
{
	int len = 0;
	struct device_node *node = NULL;
	const char *status = NULL;
	static int ret;
	static int once;

	if (is_sensorhub_disabled()) {
		pr_err("[%s]is_sensorhub_disabled \n", __func__);
		return -EINVAL;
	}

	if (once) {
		pr_info("[%s]status[%d]\n", __func__, ret);
		return ret;
	}

	node = of_find_compatible_node(NULL, NULL, "hisilicon,contexthub_status");
	if (node != NULL) {
		status = of_get_property(node, "status", &len);
		if (status == NULL) {
			pr_err("[%s]of_get_property status err\n", __func__);
			return -EINVAL;
		}

		if (strstr(status, "disabled")) {
			pr_info("[%s][disabled]\n", __func__);
			ret = -EINVAL;
		}
	}

	once = 1;
	pr_info("[%s][enabled]\n", __func__);
	return ret;
}
