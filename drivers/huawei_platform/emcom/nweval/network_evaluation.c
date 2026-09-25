/*
 * network_evaluation.c
 *
 * evaluate corresponding network by collected RTTs
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/emcom/network_evaluation.h>
#include <linux/string.h>
#include "../emcom_netlink.h"
#include "../emcom_utils.h"
#include <huawei_platform/emcom/evaluation_utils.h>

#undef HWLOG_TAG
#define HWLOG_TAG emcom_network_evaluation
HWLOG_REGIST();

/**** calculate related ****/
int8_t g_network_type = NETWORK_TYPE_UNKNOWN;
/**** storage related ****/
int32_t g_rtt_array[RTT_COUNT] = {0};
volatile uint8_t g_rtt_index = 0;
/**** control related ****/
static volatile bool g_running_flag = false;

/**** mutex ****/
/* we try to operate WITHOUT LOCKS */
 /* sometimes very large rtt values occur (very likely it is introduced by the new mechanism) */
#define RTT_ILLEGAL_THRESHOLD 1000000
#define MIU_SECOND_TO_MILLI_SECOND 1000

static void cal_rtt_indices(int32_t signal_strength,
				int8_t* traffic_index, int8_t* stability_index);
static void nweval_report_indices(sub_indices_report* pReport);

void nweval_init(void)
{
	g_network_type = NETWORK_TYPE_UNKNOWN;
	memset(g_rtt_array, 0, RTT_COUNT);
	g_rtt_index = 0;
	g_running_flag = false;
}
EXPORT_SYMBOL(nweval_init);

/*
 * this function is called in tcp_input.c, to update the RTTs
 * it operates without lock, and updates a maximum of 32 RTTs
 */
void nweval_update_rtt(struct sock* sk, long rtt_us)
{
	int8_t rtt_index = g_rtt_index;
	if (!g_running_flag)// fill the rtt array only when module is running
		return;

	if (rtt_index >= RTT_COUNT)
		rtt_index = 0;

	/* update */
	g_rtt_array[rtt_index] = rtt_us;
	g_rtt_index = rtt_index + 1;

}
EXPORT_SYMBOL(nweval_update_rtt);

/*
 * start rtt report, triggered by a message from DAEMON
 *
 */
void nweval_start_rtt_report(int8_t network_type)
{
	EMCOM_LOGD(" : nweval_start_rtt_report with network type %d", network_type);
	if (!VALIDATE_NETWORK_TYPE(network_type)) {
		EMCOM_LOGE(" : illegal network type received in nweval_start_rtt_report");
		return;
	}

	g_network_type = network_type;
	memset(g_rtt_array, 0, sizeof(int32_t)*RTT_COUNT);
	g_rtt_index = 0;
	g_running_flag = true;
}
EXPORT_SYMBOL(nweval_start_rtt_report);

/*
 * stop rtt report, triggered by a message from DAEMON
 *
 */
void nweval_stop_rtt_report(void)
{
	EMCOM_LOGD(" : nweval_stop_rtt_report");
	g_running_flag = false;
	memset(g_rtt_array, 0, sizeof(int32_t)*RTT_COUNT);
	g_rtt_index = 0;
	g_network_type = NETWORK_TYPE_UNKNOWN;
}
EXPORT_SYMBOL(nweval_stop_rtt_report);

void nweval_trigger_rtt_report(int32_t signal_strength)
{
	sub_indices_report report;
	int8_t traffic_index;
	int8_t rtt_stability_index;

	if (signal_strength > 0) {
		EMCOM_LOGE(" : illegal (positive) signal strength %d received \
in nweval_trigger_rtt_report", signal_strength);
		return;
	}

	/* calculate the traffic sub-index and stability sub-index */
	EMCOM_LOGD(" : rtt report triggered with signal strength %d", signal_strength);
	cal_rtt_indices(signal_strength, &traffic_index, &rtt_stability_index);
	/* report to daemon */
	report.traffic_index = traffic_index;
	report.rtt_stability_index = rtt_stability_index;
	nweval_report_indices(&report);
}
EXPORT_SYMBOL(nweval_trigger_rtt_report);

static void nweval_report_indices(sub_indices_report* report)
{
	int cmd = NETLINK_EMCOM_KD_NWEVAL_RTT_REPORT;
	if (report == NULL) {
		EMCOM_LOGE(" : null report pointer in nweval_report_indices! abort report");
		return;
	}

	emcom_send_msg2daemon(cmd, report, sizeof(sub_indices_report));
	/* after reporting, clear the stored array to gaurantee time effectiveness */
	memset(g_rtt_array, 0, sizeof(int32_t)*RTT_COUNT);
	g_rtt_index = 0;
}

static void cal_rtt_indices(int32_t signal_strength,
				int8_t* traffic_index, int8_t* stability_index)
{
	int32_t rtt_avg = INVALID_VALUE;
	int32_t rtt_sqrdev = INVALID_VALUE;
	uint8_t rtt_count = 0;
	int32_t rtt_array[RTT_COUNT];
	int32_t temp_rtt_array[RTT_COUNT];
	uint8_t index;

	if ((traffic_index == NULL) || (stability_index == NULL)) {
		EMCOM_LOGE(" : NULL input %s pointer in cal_rtt_indices",
			traffic_index == NULL ? "traffic index" : "stability index");
		return;
	}
	/* make use of a copy so that the array won't be changed by other thread */
	memcpy(temp_rtt_array, g_rtt_array, RTT_COUNT*sizeof(int32_t));
	for (index = 0; index < RTT_COUNT; index++) {
		if ((temp_rtt_array[index] >= MIU_SECOND_TO_MILLI_SECOND)
			&& (temp_rtt_array[index] <= RTT_ILLEGAL_THRESHOLD)) {
			rtt_array[rtt_count] = temp_rtt_array[index] / MIU_SECOND_TO_MILLI_SECOND;
			rtt_count++;
		}
	}
	/* the RTTs are only available if sufficient cout is gathered */
	if (rtt_count < ADEQUATE_RTT) {
		EMCOM_LOGD(" : failed to obtain RTTs, default sub-index will be reported");
		*traffic_index = LEVEL_GOOD;
		*stability_index = LEVEL_GOOD;
		return;
	}

	EMCOM_LOGD(" : ready to cal_rtt_indices, rtt array length %d, first 3 RTTs %d, %d, %d,",
		rtt_count, rtt_array[0], rtt_array[1], rtt_array[2]);
	cal_statistics(rtt_array, rtt_count, &rtt_avg, &rtt_sqrdev);
	EMCOM_LOGD(" : statistics calculated, rtt avg %d, rtt sqrdev %d", rtt_avg, rtt_sqrdev);
	*traffic_index = cal_traffic_index(g_network_type, signal_strength, rtt_array, rtt_count);
	*stability_index = cal_rtt_stability_index(g_network_type, rtt_avg, rtt_sqrdev);
	EMCOM_LOGD(" : sub indices calculated, traffic index %d, rtt stability index %d",
		*traffic_index, *stability_index);
}

void nweval_event_process(int32_t event, uint8_t *pdata, uint16_t len)
{
	switch (event) {
		case NETLINK_EMCOM_DK_NWEVAL_START_RTT_REPORT:
			EMCOM_LOGD(" : received NE_START_RTT_REPORT");
			if ((pdata == NULL) || (len != sizeof(int32_t))) {
				EMCOM_LOGE(" : null pointer or illegal data length %d in \
Emcom_Xengine_Nweval_start_report_rtt", len);
				return;
			}
			nweval_start_rtt_report(*pdata);
			break;
		case NETLINK_EMCOM_DK_NWEVAL_STOP_RTT_REPORT:
			EMCOM_LOGD(" : received NE_STOP_RTT_REPORT");
			nweval_stop_rtt_report();
			break;
		case NETLINK_EMCOM_DK_NWEVAL_TRIGGER_EVALUATION:
			EMCOM_LOGD(" : received NE_TRIGGER_EVALUATION");
			if ((pdata == NULL) || (len != sizeof(int32_t))) {
				EMCOM_LOGE(" : null pointer or illegal data length in \
Emcom_Xengine_Nweval_start_report_rtt");
				return;
			}
			nweval_trigger_rtt_report(*((int32_t*)pdata));
			break;
		default:
			EMCOM_LOGE(" : received unsupported message");
			break;
	}
}
EXPORT_SYMBOL(nweval_event_process);

void nweval_on_dk_connected(void)
{
	/* reply rtt ready message */
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_NWEVAL_RTT_READY, NULL, 0);
}
EXPORT_SYMBOL(nweval_on_dk_connected);