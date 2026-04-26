/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: contexthub route source file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */
#ifndef __LINUX_INPUTHUB_ROUTE_PLU_H__
#define __LINUX_INPUTHUB_ROUTE_PLU_H__

#define DROP_NV_NUM 440
#define DROP_NV_SIZE 100
#define DROP_FASTBOOT_RECORD 30
#define DROP_FASTBOOT_NUM 8
#define SENSOR_DROP_IMONITOR_ID 936005000

#pragma pack(4)
struct pkt_drop_info_record_t {
	long time;
	int height;
};

struct pkt_drop_fastboot_record_t {
	struct pkt_drop_info_record_t drop_info[DROP_FASTBOOT_NUM];
	int index;
};
#pragma pack()

void drop_fastboot_record(const pkt_drop_data_req_t *head);
int process_drop_report(const pkt_drop_data_req_t *head);
void dmd_modem_record(pkt_mode_dmd_log_report_req_t* pkt);


#endif /* __LINUX_INPUTHUB_ROUTE_PLU_H__ */

