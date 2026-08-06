/*
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
* Description: utility functions for network evaluation
* Author: dengyu davy.deng@huawei.com
* Create: 2018-02-13
*/

#include "emcom/evaluation_utils.h"
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "../emcom_utils.h"

#undef HWLOG_TAG
#define HWLOG_TAG emcom_nweval_utils
HWLOG_REGIST();

/* constant arrays */
static const int32_t
traffic_ss_segment_ends[TRAFFIC_SS_SEGMENT_ENDPOINTS] =
	{ -100, -60, -40, 0 };
static const int32_t traffic_rtt_segment_ends[RTT_POINT_ENDPOINTS] =
	{ 0, 80, 180, 10000 };
static const point_set
wifi_reference_probability_percent_sets[TRAFFIC_SS_SEGMENTS] =
	{{{ 100, 0, 0 }, { 50, 40, 10 }, { 0, 20, 80 }}, // good, normal, bad respectively
	{{ 100, 0, 0 }, { 80, 20, 0 }, { 30, 40, 30 }},
	{{ 100, 0, 0 }, { 90, 10, 0 }, { 50, 30, 20 }}};
static const point_set
lte_reference_probability_percent_sets[TRAFFIC_SS_SEGMENTS] =
	{{{ 100, 0, 0 }, { 50, 40, 10 }, { 0, 20, 80 }}, // good, normal, bad respectively
	{{ 100, 0, 0 }, { 80, 20, 0 }, { 30, 40, 30 }},
	{{ 100, 0, 0 }, { 90, 10, 0 }, { 50, 30, 20 }}};
static const point_set
others_reference_probability_percent_sets[TRAFFIC_SS_SEGMENTS] =
	{{{ 100, 0, 0 }, { 50, 40, 10 }, { 0, 20, 80 }}, // good, normal, bad respectively
	{{ 100, 0, 0 }, { 80, 20, 0 }, { 30, 40, 30 }},
	{{ 100, 0, 0 }, { 90, 10, 0 }, { 50, 30, 20 }}};
static const int32_t
stability_rtt_segment_ends[STABILITY_RTT_SEGMENT_ENDPOINTS] =
	{ 0, 80, 150, 200, 10000 }; // all network types share this rtt segment setting
static const int32_t
stability_rtt_fine_sqrdev_scals[STABILITY_RTT_FINE_STDEV_ENDPOINTS] =
	{ 160, 300, 500, 2500, 10000 }; // formally used stdev = {1, 10, 20, 50, 100}

/* function decarations */
static point_set convert_probability_to_count(point_set* probability_set,
														int8_t data_length);
static int32_t cal_euclidiean_distance_square(point point1, point point2);
static int8_t find_nearest_reference_point(int32_t* distance_squares,
													int8_t number);

/* function implementations */
/**
	 * @Function: cal_traffic_index
	 * @Description : calculate the traffic index (usually affected by resource competition)
*/
int8_t cal_traffic_index(int8_t network_type, int32_t signal_strength,
							int32_t* rtt_array, int8_t rtt_array_length)
{
	int32_t signal_strength_interval = INVALID_VALUE;
	int8_t result_index;
	int32_t distance_squares[INDEX_QUALITY_LEVELS] = {INVALID_VALUE};
	point_set* reference_probability_set = NULL;
	point_set reference_point_set;
	point current_point;
	int index;
	if (!validate_network_type(network_type) ||
		(signal_strength >= 0) || (rtt_array == NULL) || (rtt_array_length <= 0)) {
		emcom_loge(" : illegal parameter in cal_signal_index");
		return INVALID_VALUE;
	}

	/* pick reference points based on network type AND signal strength */
	for (index = 0; index < TRAFFIC_SS_SEGMENTS; index++) {
		if (signal_strength > traffic_ss_segment_ends[index] &&
			signal_strength <= traffic_ss_segment_ends[index + 1]) {
			signal_strength_interval = index;
			break;
		}
	}

	if (signal_strength_interval < 0) {
		emcom_loge(" : negative signal_strength_interval");
		return INVALID_VALUE;
	}

	switch (network_type) {
	case NETWORK_TYPE_WIFI:
		reference_probability_set = (point_set*)
		&wifi_reference_probability_percent_sets[signal_strength_interval];
		break;
	case NETWORK_TYPE_4G:
		reference_probability_set = (point_set*)
		&lte_reference_probability_percent_sets[signal_strength_interval];
		break;
	case NETWORK_TYPE_3G:
		reference_probability_set = (point_set*)
		&others_reference_probability_percent_sets[signal_strength_interval];
		break;
	case NETWORK_TYPE_2G:
		emcom_logd("2G network is not supported!");
		return INVALID_VALUE;
	default:
		emcom_loge(" : illegal network type in cal_traffic_index, %d", network_type);
		return INVALID_VALUE;
	}
	/* convert probability percent to count */
	reference_point_set = convert_probability_to_count(reference_probability_set,
												rtt_array_length);
	/* get the statistical characteristic of rtt_array, generate current point */
	current_point = get_space_point(rtt_array, rtt_array_length,
						traffic_rtt_segment_ends, RTT_POINT_ENDPOINTS);

	/* calculate Euclidean distance between current point and reference points */
	distance_squares[LEVEL_GOOD] = cal_euclidiean_distance_square(current_point,
									reference_point_set.good_point);
	distance_squares[LEVEL_NORMAL] = cal_euclidiean_distance_square(current_point,
									reference_point_set.normal_point);
	distance_squares[LEVEL_BAD] = cal_euclidiean_distance_square(current_point,
									reference_point_set.bad_point);
	/* find the point with shortest distance */
	result_index = find_nearest_reference_point(distance_squares, INDEX_QUALITY_LEVELS);
	if (result_index < 0) {
		emcom_loge(" : illegal result in cal_traffic_index");
		return INVALID_VALUE;
	}

	emcom_logd(" : traffic index %d calculated!", result_index);
	return result_index;
}
EXPORT_SYMBOL(cal_traffic_index); /*lint !e580*/

/**
	 * @Function: cal_rtt_stability_index
	 * @Description : calculate the stability index
*/
int8_t cal_rtt_stability_index(int8_t network_type, int32_t avg, int32_t sqrdev)
{
	int32_t value_rtt_sqrdev_bad = 0;
	int32_t value_rtt_sqrdev_normal = 0;
	int8_t stability_index = LEVEL_GOOD;

	if (!validate_network_type(network_type)) {
		emcom_loge(" : illegal parameter in cal_rtt_stability_index, network type %d",
						network_type);
		return INVALID_VALUE;
	}

	/* determine ss interval and rtt interval according to average value */
	if ((avg <= stability_rtt_segment_ends[RTT_PERFECT]) ||
		(avg > stability_rtt_segment_ends[RTT_VERY_BAD])) {
		emcom_logi(" : rtt_avg out of evaluation boundary");
		return INVALID_VALUE;
	} else if (avg <= stability_rtt_segment_ends[RTT_GOOD]) {
		value_rtt_sqrdev_bad = stability_rtt_fine_sqrdev_scals[RTT_GOOD];
		value_rtt_sqrdev_normal = stability_rtt_fine_sqrdev_scals[RTT_PERFECT];
	} else if (avg <= stability_rtt_segment_ends[RTT_NORMAL]) {
		value_rtt_sqrdev_bad = stability_rtt_fine_sqrdev_scals[RTT_NORMAL];
		value_rtt_sqrdev_normal = stability_rtt_fine_sqrdev_scals[RTT_GOOD];
	} else if (avg <= stability_rtt_segment_ends[RTT_BAD]) {
		value_rtt_sqrdev_bad = stability_rtt_fine_sqrdev_scals[RTT_BAD];
		value_rtt_sqrdev_normal = stability_rtt_fine_sqrdev_scals[RTT_NORMAL];
	} else {
		value_rtt_sqrdev_bad = stability_rtt_fine_sqrdev_scals[RTT_VERY_BAD];
		value_rtt_sqrdev_normal = stability_rtt_fine_sqrdev_scals[RTT_BAD];
	}

	emcom_logd(" : value_rtt_sqrdev_bad : %d, value_rtt_sqrdev_normal : %d",
					value_rtt_sqrdev_bad, value_rtt_sqrdev_normal);

	/* compare the input stdev values with the above reference ones */
	if (sqrdev > value_rtt_sqrdev_bad) {
		stability_index = LEVEL_BAD;
	} else if ((sqrdev > value_rtt_sqrdev_normal) && (sqrdev <= value_rtt_sqrdev_bad)) {
		stability_index = LEVEL_NORMAL;
	} else if ((sqrdev >= 0) && (sqrdev <= value_rtt_sqrdev_normal)) {
		stability_index = LEVEL_GOOD;
	} else {
		emcom_logi(" : unknown error in cal_stability_index");
		return INVALID_VALUE;
	}

	emcom_logd(" : stability index %d generated for network type %d",
					stability_index, network_type);
	return stability_index;
}
EXPORT_SYMBOL(cal_rtt_stability_index); /*lint !e580*/

/**
	 * @Function: get_space_point
	 * @Description : generate a point (x, y, z) in the 3-dimensional space,
	 *				given the data array and the segments
	 *				x, y, z are the statistical probabilities
	 *				that data fall in the first, second and third segment, respectively
	 * @Param : array is the input data array
	 *			length is the corresponding length
	 *			segment_ends are the boundaries of different segments
	 *			endpoints is the number of the above segments
	 * @Return : point representing the input array
*/
point get_space_point(int32_t* array, int8_t length,
							const int32_t* segment_ends, int8_t endpoints)
{
	point generated_point = { INVALID_VALUE, INVALID_VALUE, INVALID_VALUE };
	int8_t segments = endpoints - 1;
	int8_t count_in_dimensions[POINT_DIMENSIONS];
	int8_t* count_in_segments = NULL;
	int index1;
	int index2;
	if ((array == NULL) || (segment_ends == NULL) || (length <= 0) || (segments <= 0)) {
		emcom_loge(" : illegal parameter in get_probability_space_point");
		return generated_point;
	}

	count_in_segments = (int8_t*) kmalloc(segments * sizeof(int8_t), GFP_KERNEL);
	if (count_in_segments == NULL) {
		emcom_loge(" : memery allocation failed");
		return generated_point;
	}

	/* initialize the varibles */
	for (index1 = 0; index1 < segments; index1++)
		count_in_segments[index1] = 0;
	for (index1 = 0; index1 < POINT_DIMENSIONS; index1++)
		count_in_dimensions[index1] = 0;

	/* calculate counts in different segments */
	for (index1 = 0; index1 < length; index1++) {
		for (index2 = 0; index2 < segments; index2++) {
			/* note the use of index1 and index2, do not be confused */
			if (array[index1] > segment_ends[index2] && array[index1] <= segment_ends[index2 + 1]) {
				count_in_segments[index2]++;
				break; // break inner loop for segment
			}
		}
	}

	/* distinguish if this call is in "standard form" or not */
	if (POINT_DIMENSIONS == segments) {
		generated_point.x = count_in_segments[INDEX_X];
		generated_point.y = count_in_segments[INDEX_Y];
		generated_point.z = count_in_segments[INDEX_Z];
	} else if (segments > POINT_DIMENSIONS) {
		count_in_dimensions[INDEX_X] = count_in_segments[INDEX_X];
		count_in_dimensions[INDEX_Y] = count_in_segments[INDEX_Y];
		count_in_dimensions[INDEX_Z] = 0;
		/* fourth and latter segments will be merged into the third */
		for (index1 = INDEX_Z; index1 < segments; index1++)
			count_in_dimensions[INDEX_Z] += count_in_segments[index1];

		generated_point.x = count_in_dimensions[INDEX_X];
		generated_point.y = count_in_dimensions[INDEX_Y];
		generated_point.z = count_in_dimensions[INDEX_Z];
	} else {
		emcom_loge("unknown error, probably incorrect point dimension given");
	}

	kfree(count_in_segments);
	emcom_logd(" : obtained the current point %d, %d, %d in %d dimensional space",
					generated_point.x, generated_point.y,
					generated_point.z, POINT_DIMENSIONS);
	return generated_point;
}
EXPORT_SYMBOL(get_space_point); /*lint !e580*/

/**
	 * @Function: cal_euclidiean_distance
	 * @Description : calculate the euclidiean distance between given two points
	 * @Return : the distance of two points
*/
int32_t cal_euclidiean_distance_square(point point1, point point2)
{
	int32_t sum_square;

	if ((point1.x < 0) || (point1.y < 0) || (point1.z < 0) ||
		(point2.x < 0) || (point2.y < 0) || (point2.z < 0)) {
		emcom_loge(" : illegal point in cal_euclidiean_distance, point1.x = %d, \
point1.y = %d, point1.z = %d, point2.x = %d, point2.y = %d, point2.z = %d", \
						point1.x, point1.y, point1.z, point2.x, point2.y, point2.z);
		return INVALID_VALUE;
	}

	sum_square = (point1.x - point2.x) * (point1.x - point2.x) +
				(point1.y - point2.y) * (point1.y - point2.y) +
				(point1.z - point2.z) * (point1.z - point2.z);
	emcom_logd(" : calculated distance square between current point (%d, %d, %d) \
and reference point (%d, %d, %d) : %d", \
					point1.x, point1.y, point1.z, point2.x, point2.y, point2.z, sum_square);

	return sum_square;
}

/**
	 * @Function: find_nearest_reference_point
	 * @Description : find the reference point most adjacent to the current point, given distances
	 * @Return : index of nearest point
*/
int8_t find_nearest_reference_point(int32_t* distance_squares, int8_t number)
{
	int8_t nearest_reference_index = INVALID_VALUE;
	int32_t min = MAX_VALUE;
	int index;
	if ((distance_squares == NULL) || (number <= 0)) {
		emcom_loge(" : illegal point in find_nearest_reference_point");
		return INVALID_VALUE;
	}

	/* find the shortest temporary distance */
	for (index = 0; index < number; index++) {
		if (distance_squares[index] < 0) {
			emcom_loge(" : illegal (negative) distance in find_nearest_reference_point");
			return INVALID_VALUE;
		}
		if (distance_squares[index] < min)
			min = distance_squares[index];
	}
	/* a loop to find the index of nearest reference point */
	for (index = number - 1; index >= 0; index--) {
		if (distance_squares[index] == min) {
			nearest_reference_index = index;
			emcom_logd(" : found min distance square %d, nearest reference point %d",
							min, index);
			break; // in case where multiple nearest reference points exist, return the "worst condition"
		}
	}

	return nearest_reference_index;
}

/**
	 * @Function: cal_statistics
	 * @Description : calculate avg and stdev values
*/
void cal_statistics(const int32_t* array, const int8_t length, int32_t* avg, int32_t* sqrdev)
{
	/* the number of legal data may differ from the input parameter due to validation */
	int8_t actual_length = length;
	uint32_t sum = 0; // integer yields higher calculation efficiency
	int32_t average;
	int32_t square_deviation;
	int index;

	if ((array == NULL) || (length <= 0)) {
		emcom_loge(" : illegal parameter in cal_statistics, length : %d", length);
		return;
	}

	/* calculate average value */
	for (index = 0; index < length; index++) {
		if ((array[index] <= SIGNAL_ABNORMAL_THRESHOLD) || (array[index] >= RTT_ABNORMAL_THRESHOLD)) {
			emcom_logd(" : illegal data (index %d) %d in average value calculation", index, array[index]);
			actual_length--;
			continue;
		}
		sum += array[index];
	}
	if (actual_length == 0) {
		/* this means all RTTs are abnormal, most likely too high */
		/* return a high average value to ensure BAD is the evaluation result */
		*avg = RTT_ABNORMAL_THRESHOLD;
		*sqrdev = 0; // and sqrdev is irrelavent
		return;
	}
	if (actual_length == SS_COUNT)
		average = sum >> SS_SHIFT_BITS;
	else if (actual_length == RTT_COUNT)
		average = sum >> RTT_SHIFT_BITS;
	else
		average = sum / actual_length;

	/* calculate standard deviation */
	sum = 0;
	for (index = 0; index < length; index++) {
		if ((array[index] <= SIGNAL_ABNORMAL_THRESHOLD) ||
			(array[index] >= RTT_ABNORMAL_THRESHOLD)) {
			emcom_loge(" : illegal data (index %d) %d in deviation calculation",
							index, array[index]);
			/* do NOT change actual_length here again, otherwise it will be incorrect */
			continue;
		}
		sum += (average - array[index]) * (average - array[index]);
	}
	if (actual_length == SS_COUNT)
		square_deviation = sum >> SS_SHIFT_BITS;
	else if (actual_length == RTT_COUNT)
		square_deviation = sum >> RTT_SHIFT_BITS;
	else
		square_deviation = sum / actual_length;

	*avg = average;
	*sqrdev = square_deviation;
}
EXPORT_SYMBOL(cal_statistics); /*lint !e580*/

/**
	 * @Function: cal_statistics
	 * @Description : calculate avg and stdev values
	 * @Input : array, length
	 * @Output : avg, stdev
*/
point_set convert_probability_to_count(point_set* probability_set,
												int8_t data_length)
{
	point_set count_set = {INVALID_POINT, INVALID_POINT, INVALID_POINT};
	if (probability_set == NULL) {
		emcom_loge(" : null probability_set pointer");
		return count_set;
	}
	if ((!validate_point(probability_set->good_point)) ||
		(!validate_point(probability_set->normal_point)) ||
		(!validate_point(probability_set->bad_point))) {
		emcom_loge(" : illegal reference probability set");
		return count_set;
	}

	point_convert_percent_to_count(count_set.good_point, \
											probability_set->good_point, \
											data_length);
	point_convert_percent_to_count(count_set.normal_point, \
											probability_set->normal_point, \
											data_length);
	point_convert_percent_to_count(count_set.bad_point, \
											probability_set->bad_point, \
											data_length);

	emcom_logd(" : through conversion, the reference points are : good (%d, %d, %d), \
normal (%d, %d, %d), bad (%d, %d, %d)", \
				count_set.good_point.x, count_set.good_point.y, count_set.good_point.z, \
				count_set.normal_point.x, count_set.normal_point.y, count_set.normal_point.z, \
				count_set.bad_point.x, count_set.bad_point.y, count_set.bad_point.z);
	return count_set;
}
