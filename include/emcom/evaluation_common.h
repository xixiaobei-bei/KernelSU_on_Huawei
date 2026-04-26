/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: utility functions for network evaluation
 * Author: dengyu davy.deng@huawei.com
 * Create: 2018-09-10
 */

#ifndef _EVALUATION_COMMON_H_
#define _EVALUATION_COMMON_H_

#include <linux/types.h>

/* metric related */
#define SS_COUNT 8
#define SS_SHIFT_BITS 3 // "a / 32" is equivalent to "a >> 5"
#define RTT_COUNT 32 // the "ideal fixed RTT count we wish to be available"
#define RTT_SHIFT_BITS 5

/* evaluation results */
typedef enum {
	NETWORK_QUALITY_UNKNOWN = 10,
	NETWORK_QUALITY_GOOD = 11,
	NETWORK_QUALITY_NORMAL = 12,
	NETWORK_QUALITY_BAD = 13,
} network_quality;

typedef enum {
	NETWORK_BOTTLENECK_UNKNOWN = 20,
	NETWORK_BOTTLENECK_SIGNAL = 21,
	NETWORK_BOTTLENECK_TRAFFIC = 22,
	NETWORK_BOTTLENECK_STABILITY = 23,
	NETWORK_BOTTLENECK_NONE = 24,
} network_bottleneck;

/* misc values */
#ifndef NULL
#define NULL (void*)0
#endif // NULL
#define MAX_VALUE 1000000
#define INVALID_VALUE (-1)

#endif // EVALUATION_COMMON_H_INCLUDED