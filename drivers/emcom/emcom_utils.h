/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: utils of xengine module
 * Author: lianghenghui lianghenghui@huawei.com
 * Create: 2017-07-24
 */

#ifndef _EMCOM_UTILS_H
#define _EMCOM_UTILS_H
#include <huawei_platform/log/hw_log.h>

#undef HWLOG_TAG
#define HWLOG_TAG EMCOMK

HWLOG_REGIST();

#define EMCOM_DEBUG    1
#define EMCOM_INFO     1

/* network type */
typedef enum {
	NETWORK_TYPE_UNKNOWN = 0,
	NETWORK_TYPE_2G,
	NETWORK_TYPE_3G,
	NETWORK_TYPE_4G,
	NETWORK_TYPE_WIFI,
} network_type;

typedef enum {
	MODEM_NOT_SUPPORT_EMCOM,
	MODEM_SUPPORT_EMCOM
} emcom_support_enum;

#define emcom_logd(fmt, ...) \
	do { \
		if (EMCOM_DEBUG) { \
			hwlog_info("%s "fmt"\n", __func__, ##__VA_ARGS__); \
		} \
	} while (0)

#define emcom_logi(fmt, ...) \
	do { \
		if (EMCOM_INFO) { \
			hwlog_info("%s "fmt"\n", __func__, ##__VA_ARGS__); \
		} \
	} while (0)

#define emcom_loge(fmt, ...) \
	do \
		hwlog_err("%s "fmt"\n", __func__, ##__VA_ARGS__); \
	while (0)

bool emcom_is_modem_support(void);

#endif  /* _EMCOM_UTILS_H */
