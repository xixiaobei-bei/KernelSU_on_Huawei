/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: contexthub route source file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#include <contexthub_route.h>
#include <contexthub_route_plu.h>
#include <linux/kernel.h>
#include <linux/rtc.h>
#include <linux/time64.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <huawei_platform/log/imonitor.h>
#include <securec.h>
#include "protocol.h"
#include "sensor_config.h"
#include "sensor_detect.h"
#include "sensor_info.h"
#include "sensor_feima.h"
#include "sensor_sysfs.h"

void drop_fastboot_record(const pkt_drop_data_req_t *head)
{
	static int drop_record_num;
	static struct pkt_drop_fastboot_record_t drop_record;
	struct timeval time = {0};

	if (head->data.material != 1 || head->data.height <= DROP_FASTBOOT_RECORD)
		return;

	read_calibrate_data_from_nv(DROP_NV_NUM, DROP_NV_SIZE, "DROP");
	if (memcpy_s(&drop_record, sizeof(struct pkt_drop_fastboot_record_t),
		user_info.nv_data, DROP_NV_SIZE) != EOK)
		hwlog_info("drop_fastboot_record copy nv fail\n");

	drop_record_num = drop_record.index;
	do_gettimeofday(&time);
	drop_record.drop_info[drop_record_num].time = time.tv_sec;
	drop_record.drop_info[drop_record_num].height = head->data.height;
	hwlog_info("drop_fastboot_record time: %ld, height: %d index: %d\n",
		drop_record.drop_info[drop_record_num].time,
		drop_record.drop_info[drop_record_num].height,
		drop_record.index);

	drop_record_num++;
	if (drop_record_num >= DROP_FASTBOOT_NUM)
		drop_record_num = 0;

	drop_record.index = drop_record_num;
	if (write_calibrate_data_to_nv(DROP_NV_NUM,
		DROP_NV_SIZE, "DROP", (char *)&drop_record))
		hwlog_info("drop_fastboot_record write nv fail\n");
}

int process_drop_report(const pkt_drop_data_req_t *head)
{
	struct imonitor_eventobj *obj = NULL;
	int ret = 0;
	int yaw = 0;
	int speed = 0;
	int shell = 0;
	int film = 0;

	if (!head) {
		hwlog_err("%s para error\n", __func__);
		return -1;
	}

	hwlog_info("Kernel get drop type %d, initial_speed %d, height %d, angle_pitch %d, angle_roll %d, impact %d\n",
		head->data.type, head->data.initial_speed,
		head->data.height, head->data.angle_pitch,
		head->data.angle_roll, head->data.material);

	obj = imonitor_create_eventobj(SENSOR_DROP_IMONITOR_ID);
	if (!obj) {
		hwlog_err("%s imonitor_create_eventobj failed\n", __func__);
		return -1;
	}

	ret += imonitor_set_param_integer_v2(obj, "Type",
		(long)(head->data.type));
	ret += imonitor_set_param_integer_v2(obj, "InitSpeed",
		(long)(head->data.initial_speed));
	ret += imonitor_set_param_integer_v2(obj, "Height",
		(long)(head->data.height));
	ret += imonitor_set_param_integer_v2(obj, "Pitch",
		(long)(head->data.angle_pitch));
	ret += imonitor_set_param_integer_v2(obj, "Roll",
		(long)(head->data.angle_roll));
	ret += imonitor_set_param_integer_v2(obj, "Material",
		(long)(head->data.material));

	ret += imonitor_set_param_integer_v2(obj, "Yaw", (long)(yaw));
	ret += imonitor_set_param_integer_v2(obj, "Speed", (long)(speed));
	ret += imonitor_set_param_integer_v2(obj, "Shell", (long)(shell));
	ret += imonitor_set_param_integer_v2(obj, "Film", (long)(film));
	if (ret) {
		imonitor_destroy_eventobj(obj);
		hwlog_err("%s imonitor_set_param failed, ret %d\n", __func__, ret);
		return ret;
	}

	ret = imonitor_send_event(obj);
	if (ret < 0)
		hwlog_err("%s imonitor_send_event failed, ret %d\n", __func__, ret);

	imonitor_destroy_eventobj(obj);
	return ret;
}

void dmd_modem_record(pkt_mode_dmd_log_report_req_t* pkt)
{
	hwlog_info("%s, dmd_case is %d dmd_id %d\n",
		__func__, pkt->dmd_case,pkt->dmd_id);

	dsm_client_record(shb_dclient, "modem_succ:%d,modem_valid:%d \n", pkt->resv1,pkt->resv2);
	dsm_client_notify(shb_dclient, pkt->dmd_id);

	dsm_client_record(shb_dclient,"m0_id:%d,m0_valid:%d,m0_channel:%d,m0_tas:%d,m0_mas:%d,m0_trx:%d,\
		m0_0_rat:%d,m0_0_band:%d,m0_0_power:%d,m0_0_rsv:%d,\
		m0_1_rat:%d,m0_1_band:%d,m0_1_power:%d,m0_1_rsv:%d,\
		m0_2_rat:%d,m0_2_band:%d,m0_2_power:%d,m0_2_rsv:%d,\
		m0_3_rat:%d,m0_3_band:%d,m0_3_power:%d,m0_3_rsv:%d\n",
		pkt->sensorhub_dmd_mode_info[0].modem_id,pkt->sensorhub_dmd_mode_info[0].validflag,pkt->sensorhub_dmd_mode_info[0].channel,pkt->sensorhub_dmd_mode_info[0].tas_staus,pkt->sensorhub_dmd_mode_info[0].mas_staus,pkt->sensorhub_dmd_mode_info[0].trx_tas_staus,
		pkt->sensorhub_dmd_mode_info[0].tx_info[0].rat,pkt->sensorhub_dmd_mode_info[0].tx_info[0].band,pkt->sensorhub_dmd_mode_info[0].tx_info[0].power,pkt->sensorhub_dmd_mode_info[0].tx_info[0].rsv,
		pkt->sensorhub_dmd_mode_info[0].tx_info[1].rat,pkt->sensorhub_dmd_mode_info[0].tx_info[1].band,pkt->sensorhub_dmd_mode_info[0].tx_info[1].power,pkt->sensorhub_dmd_mode_info[0].tx_info[1].rsv,
		pkt->sensorhub_dmd_mode_info[0].tx_info[2].rat,pkt->sensorhub_dmd_mode_info[0].tx_info[2].band,pkt->sensorhub_dmd_mode_info[0].tx_info[2].power,pkt->sensorhub_dmd_mode_info[0].tx_info[2].rsv,
		pkt->sensorhub_dmd_mode_info[0].tx_info[3].rat,pkt->sensorhub_dmd_mode_info[0].tx_info[3].band,pkt->sensorhub_dmd_mode_info[0].tx_info[3].power,pkt->sensorhub_dmd_mode_info[0].tx_info[3].rsv
	);
	dsm_client_notify(shb_dclient, pkt->dmd_id);

	dsm_client_record(shb_dclient,"m1_id:%d,m1_valid:%d,m1_channel:%d,m1_tas:%d,m1_mas:%d,m1_trx:%d,\
		m1_0_rat:%d,m1_0_band:%d,m1_0_power:%d,m1_0_rsv:%d,\
		m1_1_rat:%d,m1_1_band:%d,m1_1_power:%d,m1_1_rsv:%d,\
		m1_2_rat:%d,m1_2_band:%d,m1_2_power:%d,m1_2_rsv:%d,\
		m1_3_rat:%d,m1_3_band:%d,m1_3_power:%d,m1_3_rsv:%d\n",
		pkt->sensorhub_dmd_mode_info[1].modem_id,pkt->sensorhub_dmd_mode_info[1].validflag,pkt->sensorhub_dmd_mode_info[1].channel,pkt->sensorhub_dmd_mode_info[1].tas_staus,pkt->sensorhub_dmd_mode_info[1].mas_staus,pkt->sensorhub_dmd_mode_info[1].trx_tas_staus,
		pkt->sensorhub_dmd_mode_info[1].tx_info[0].rat,pkt->sensorhub_dmd_mode_info[1].tx_info[0].band,pkt->sensorhub_dmd_mode_info[1].tx_info[0].power,pkt->sensorhub_dmd_mode_info[1].tx_info[0].rsv,
		pkt->sensorhub_dmd_mode_info[1].tx_info[1].rat,pkt->sensorhub_dmd_mode_info[1].tx_info[1].band,pkt->sensorhub_dmd_mode_info[1].tx_info[1].power,pkt->sensorhub_dmd_mode_info[1].tx_info[1].rsv,
		pkt->sensorhub_dmd_mode_info[1].tx_info[2].rat,pkt->sensorhub_dmd_mode_info[1].tx_info[2].band,pkt->sensorhub_dmd_mode_info[1].tx_info[2].power,pkt->sensorhub_dmd_mode_info[1].tx_info[2].rsv,
		pkt->sensorhub_dmd_mode_info[1].tx_info[3].rat,pkt->sensorhub_dmd_mode_info[1].tx_info[3].band,pkt->sensorhub_dmd_mode_info[1].tx_info[3].power,pkt->sensorhub_dmd_mode_info[1].tx_info[3].rsv
	);
	dsm_client_notify(shb_dclient, pkt->dmd_id);

	dsm_client_record(shb_dclient,"m2_id:%d,m2_valid:%d,m2_channel:%d,m2_tas:%d,m2_mas:%d,m2_trx:%d,\
		m2_0_rat:%d,m2_0_band:%d,m2_0_power:%d,m2_0_rsv:%d,\
		m2_1_rat:%d,m2_1_band:%d,m2_1_power:%d,m2_1_rsv:%d,\
		m2_2_rat:%d,m2_2_band:%d,m2_2_power:%d,m2_2_rsv:%d,\
		m2_3_rat:%d,m2_3_band:%d,m2_3_power:%d,m2_3_rsv:%d\n",
		pkt->sensorhub_dmd_mode_info[2].modem_id,pkt->sensorhub_dmd_mode_info[2].validflag,pkt->sensorhub_dmd_mode_info[2].channel,pkt->sensorhub_dmd_mode_info[2].tas_staus,pkt->sensorhub_dmd_mode_info[2].mas_staus,pkt->sensorhub_dmd_mode_info[2].trx_tas_staus,
		pkt->sensorhub_dmd_mode_info[2].tx_info[0].rat,pkt->sensorhub_dmd_mode_info[2].tx_info[0].band,pkt->sensorhub_dmd_mode_info[2].tx_info[0].power,pkt->sensorhub_dmd_mode_info[2].tx_info[0].rsv,
		pkt->sensorhub_dmd_mode_info[2].tx_info[1].rat,pkt->sensorhub_dmd_mode_info[2].tx_info[1].band,pkt->sensorhub_dmd_mode_info[2].tx_info[1].power,pkt->sensorhub_dmd_mode_info[2].tx_info[1].rsv,
		pkt->sensorhub_dmd_mode_info[2].tx_info[2].rat,pkt->sensorhub_dmd_mode_info[2].tx_info[2].band,pkt->sensorhub_dmd_mode_info[2].tx_info[2].power,pkt->sensorhub_dmd_mode_info[2].tx_info[2].rsv,
		pkt->sensorhub_dmd_mode_info[2].tx_info[3].rat,pkt->sensorhub_dmd_mode_info[2].tx_info[3].band,pkt->sensorhub_dmd_mode_info[2].tx_info[3].power,pkt->sensorhub_dmd_mode_info[2].tx_info[3].rsv
	);
	dsm_client_notify(shb_dclient, pkt->dmd_id);
}


