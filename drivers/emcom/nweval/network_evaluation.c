/*
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
* Description: evaluate corresponding network by collected RTTs
* Author: dengyu davy.deng@huawei.com
* Create: 2018-02-13
*/

#include "emcom/network_evaluation.h"
#include <linux/string.h>
#include "../emcom_netlink.h"
#include "../emcom_utils.h"
#include "securec.h"
#include "emcom/evaluation_utils.h"

#undef HWLOG_TAG
#define HWLOG_TAG emcom_network_evaluation
HWLOG_REGIST();

/* calculate related */
int8_t g_network_type = NETWORK_TYPE_UNKNOWN;
/* storage related */
int32_t g_rtt_array[RTT_COUNT] = {0};
volatile uint8_t g_rtt_index = 0;
/* control related */
static volatile bool g_running_flag = false;

/* mutex */
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
	(void)memset_s(g_rtt_array, sizeof(g_rtt_array), 0, sizeof(g_rtt_array));
	g_rtt_index = 0;
	g_running_flag = false;
}
EXPORT_SYMBOL(nweval_init); /*lint !e580*/

/*
 * this function is called in tcp_input.c, to update the RTTs
 * it operates without lock, and updates a maximum of 32 RTTs
 */
void nweval_update_rtt(struct sock* sk, long rtt_us)
{
	int8_t rtt_index = g_rtt_index;
	if (!g_running_flag) /* fill the rtt array only when module is running */
		return;

	if (rtt_index >= RTT_COUNT)
		rtt_index = 0;

	/* update */
	g_rtt_array[rtt_index] = rtt_us;
	g_rtt_index = rtt_index + 1;
}
EXPORT_SYMBOL(nweval_update_rtt); /*lint !e580*/

/*
 * start rtt report, triggered by a message from DAEMON
 *
 */
void nweval_start_rtt_report(int8_t network_type)
{
	emcom_logd(" : nweval_start_rtt_report with network type %d", network_type);
	if (!validate_network_type(network_type)) {
		emcom_loge(" : illegal network type received in nweval_start_rtt_report");
		return;
	}

	g_network_type = network_type;
	(void)memset_s(g_rtt_array, sizeof(g_rtt_array), 0, sizeof(g_rtt_array));
	g_rtt_index = 0;
	g_running_flag = true;
}
EXPORT_SYMBOL(nweval_start_rtt_report); /*lint !e580*/

/*
 * stop rtt report, triggered by a message from DAEMON
 *
 */
void nweval_stop_rtt_report(void)
{
	emcom_logd(" : nweval_stop_rtt_report");
	g_running_flag = false;
	(void)memset_s(g_rtt_array, sizeof(g_rtt_array), 0, sizeof(g_rtt_array));
	g_rtt_index = 0;
	g_network_type = NETWORK_TYPE_UNKNOWN;
}
EXPORT_SYMBOL(nweval_stop_rtt_report); /*lint !e580*/

void nweval_trigger_rtt_report(int32_t signal_strength)
{
	sub_indices_report report;
	int8_t traffic_index;
	int8_t rtt_stability_index;

	if (signal_strength > 0) {
		emcom_loge(" : illegal (positive) signal strength %d received \
in nweval_trigger_rtt_report", signal_strength);
		return;
	}

	/* calculate the traffic sub-index and stability sub-index */
	emcom_logd(" : rtt report triggered with signal strength %d", signal_strength);
	cal_rtt_indices(signal_strength, &traffic_index, &rtt_stability_index);
	/* report to daemon */
	report.traffic_index = traffic_index;
	report.rtt_stability_index = rtt_stability_index;
	nweval_report_indices(&report);
}
EXPORT_SYMBOL(nweval_trigger_rtt_report); /*lint !e580*/

static void nweval_report_indices(sub_indices_report* report)
{
	int cmd = NETLINK_EMCOM_KD_NWEVAL_RTT_REPORT;
	if (report == NULL) {
		emcom_loge(" : null report pointer in nweval_report_indices! abort report");
		return;
	}

	emcom_send_msg2daemon(cmd, report, sizeof(sub_indices_report));
	/* after reporting, clear the stored array to gaurantee time effectiveness */
	(void)memset_s(g_rtt_array, sizeof(g_rtt_array), 0, sizeof(g_rtt_array));
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
		emcom_loge(" : NULL input %s pointer in cal_rtt_indices",
			(traffic_index == NULL) ? "traffic index" : "stability index");
		return;
	}
	/* make use of a copy so that the array won't be changed by other thread */
	if (memcpy_s(temp_rtt_array, sizeof(temp_rtt_array), g_rtt_array, sizeof(g_rtt_array)) != EOK) {
		emcom_loge("memcpy_s failed");
		return;
	}

	for (index = 0; index < RTT_COUNT; index++) {
		if ((temp_rtt_array[index] >= MIU_SECOND_TO_MILLI_SECOND)
			&& (temp_rtt_array[index] <= RTT_ILLEGAL_THRESHOLD)) {
			rtt_array[rtt_count] = temp_rtt_array[index] / MIU_SECOND_TO_MILLI_SECOND;
			rtt_count++;
		}
	}
	/* the RTTs are only available if sufficient cout is gathered */
	if (rtt_count < ADEQUATE_RTT) {
		emcom_logd(" : failed to obtain RTTs, default sub-index will be reported");
		*traffic_index = LEVEL_GOOD;
		*stability_index = LEVEL_GOOD;
		return;
	}

	emcom_logd(" : ready to cal_rtt_indices, rtt array length %d, first 3 RTTs %d, %d, %d,",
		rtt_count, rtt_array[0], rtt_array[1], rtt_array[2]);
	cal_statistics(rtt_array, rtt_count, &rtt_avg, &rtt_sqrdev);
	emcom_logd(" : statistics calculated, rtt avg %d, rtt sqrdev %d", rtt_avg, rtt_sqrdev);
	*traffic_index = cal_traffic_index(g_network_type, signal_strength, rtt_array, rtt_count);
	*stability_index = cal_rtt_stability_index(g_network_type, rtt_avg, rtt_sqrdev);
	emcom_logd(" : sub indices calculated, traffic index %d, rtt stability index %d",
		*traffic_index, *stability_index);
}

void nweval_event_process(int32_t event, uint8_t *pdata, uint16_t len)
{
	switch (event) {
	case NETLINK_EMCOM_DK_NWEVAL_START_RTT_REPORT:
		emcom_logd(" : received NE_START_RTT_REPORT");
		if ((pdata == NULL) || (len != sizeof(int32_t))) {
			emcom_loge(" : null pointer or illegal data length %u in \
				nweval_event_process", len);
			return;
		}
		nweval_start_rtt_report(*pdata);
		break;
	case NETLINK_EMCOM_DK_NWEVAL_STOP_RTT_REPORT:
		emcom_logd(" : received NE_STOP_RTT_REPORT");
		nweval_stop_rtt_report();
		break;
	case NETLINK_EMCOM_DK_NWEVAL_TRIGGER_EVALUATION:
		emcom_logd(" : received NE_TRIGGER_EVALUATION");
		if ((pdata == NULL) || (len != sizeof(int32_t))) {
			emcom_loge(" : null pointer or illegal data length in \
				nweval_event_process");
			return;
		}
		nweval_trigger_rtt_report(*((int32_t*)pdata));
		break;
	default:
		emcom_loge(" : received unsupported message");
		break;
	}
}
EXPORT_SYMBOL(nweval_event_process); /*lint !e580*/

void nweval_on_dk_connected(void)
{
	/* reply rtt ready message */
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_NWEVAL_RTT_READY, NULL, 0);
}
EXPORT_SYMBOL(nweval_on_dk_connected); /*lint !e580*/
