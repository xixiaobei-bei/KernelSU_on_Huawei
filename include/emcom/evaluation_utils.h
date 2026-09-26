/*
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
* Description: utility functions for network evaluation
* Author: dengyu davy.deng@huawei.com
* Create: 2018-02-13
*/

#ifndef _EVALUATION_UTILS_H_
#define _EVALUATION_UTILS_H_

/* include files */
#include "evaluation_common.h"

/* validate parameters */
#define validate_network_type(parameter) \
	(NETWORK_TYPE_UNKNOWN == parameter || \
	NETWORK_TYPE_2G == parameter || \
	NETWORK_TYPE_3G == parameter || \
	NETWORK_TYPE_4G == parameter || \
	NETWORK_TYPE_WIFI == parameter)

/* misc values */
#define SIGNAL_ABNORMAL_THRESHOLD (-100)
#define RTT_ABNORMAL_THRESHOLD 400

/* index levels, IG means INDEX_GOOD, note it begins from 0 */
typedef enum {
	LEVEL_GOOD,
	LEVEL_NORMAL,
	LEVEL_BAD,
	LEVEL_MAX,
} index_quality;
#define INDEX_QUALITY_LEVELS    LEVEL_MAX

/* constants and structures used in signal strength evaluation */
/* number of signal strength interval endpoints, for instance, 3 levels require 4 endpoints */
#define SS_ENDPOINTS (INDEX_QUALITY_LEVELS + 1)

/* constants and structures used in traffic deduction */
#define TRAFFIC_SS_SEGMENTS 3
#define TRAFFIC_SS_SEGMENT_ENDPOINTS (TRAFFIC_SS_SEGMENTS + 1)

/* point related definitions */
#define POINT_DIMENSIONS 3
#define RTT_POINT_ENDPOINTS (POINT_DIMENSIONS + 1)

typedef struct { // 3-dimension point. coordinates are COUNTS IN DIFFERENT SEGMENTS
	int8_t x;
	int8_t y;
	int8_t z;
} point;

#define INVALID_POINT { INVALID_VALUE, INVALID_VALUE, INVALID_VALUE }
#define validate_point(pt) (pt.x >= 0 && pt.x <= 100 && pt.y >= 0 && \
							pt.y <= 100 && pt.z >= 0 && pt.z <= 100)
#define INDEX_X 0
#define INDEX_Y 1
#define INDEX_Z 2
typedef struct {
	point good_point;
	point normal_point;
	point bad_point;
} point_set;

#define point_convert_percent_to_count(p1, p2, len) \
	do { \
		p1.x = (p2.x * len) / 100; \
		p1.y = (p2.y * len) / 100; \
		p1.z = (p2.z * len) / 100; \
	} while (0)

/* constants and structures used in stability evaluation */
typedef enum {
	RTT_PERFECT,
	RTT_GOOD,
	RTT_NORMAL,
	RTT_BAD,
	RTT_VERY_BAD,
	RTT_LEVEL_MAX,
} rtt_level_quality;
#define STABILITY_RTT_SEGMENT_ENDPOINTS RTT_LEVEL_MAX
#define STABILITY_RTT_FINE_STDEV_ENDPOINTS RTT_LEVEL_MAX

/* function declarations */
void cal_statistics(const int32_t* array, const int8_t length, int32_t* avg, int32_t* stdev);
point get_space_point(int32_t* array, int8_t length,
							const int32_t* segment_ends, int8_t endpoints);
int8_t cal_traffic_index(int8_t network_type, int32_t signal_strength,
							int32_t* rtt_array, int8_t rtt_array_length);
int8_t cal_rtt_stability_index(int8_t network_type, int32_t avg, int32_t sqrdev);

#endif // _EVALUATION_UTILS_H_