/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019. All rights reserved.
 * Description: device_channel.h.
 * Create: 2019/11/05
 */

#ifndef __DEVICE_INTERFACE_H
#define __DEVICE_INTERFACE_H
#include <linux/hisi/contexthub/protocol_as.h>


/*
 * ap device callback structure
 */
struct device_operation {
	int (*enable) (bool enable);
	int (*setdelay) (int ms);
};

/*
 * record device info for reume, recovery or test
 */
struct device_status {
	/*
	 * record whether device is in activate status,
	 * already opened and setdelay
	 */
	int status[TAG_DEVICE_END];
	/* record device delay time */
	int delay[TAG_DEVICE_END];
	/* record whether device was opened */
	int opened[TAG_DEVICE_END];
	int batch_cnt[TAG_DEVICE_END];
};

extern struct device_status g_device_status;

/*
 * ap device data struct
 */
struct ap_device_ops_record {
	struct device_operation ops;
	bool work_on_ap;
};

/*
 * Function    : inputhub_device_enable
 * Description : enable/disable device, use this interface will record
 *               device status for suspend, resume, recovery, reboot
 * Input       : [tag] key, identify an app of device
 *             : [enable] true for enable, false for disable
 * Output      : none
 * Return      : 0 is ok, otherwise failed
 */
int inputhub_device_enable(int tag, bool enable);

/*
 * Function    : inputhub_device_setdelay
 * Description : set delay for device, use this interface will record
 *               device status for suspend, resume, recovery, reboot
 * Input       : [tag] key, identify an app of device
 *             : [interval_param] interval param info
 * Output      : none
 * Return      : 0 is ok, otherwise failed
 */
int inputhub_device_setdelay(int tag, const interval_param_t *interval_param);

/*
 * Function    : register_ap_device_operations
 * Description : register ap device operation
 * Input       : [tag] key, identify an app of device
 *             : [ops] callback for ap device
 * Output      : none
 * Return      : 0 is ok, otherwise failed
 */
int register_ap_device_operations(int tag, struct device_operation *ops);

/*
 * Function    : unregister_ap_device_operations
 * Description : unregister ap device operation
 * Input       : [tag] key, identify an app of device
 * Output      : none
 * Return      : 0 is ok, otherwise failed
 */
int unregister_ap_device_operations(int tag);

/*
 * Function    : ap_device_enable
 * Description : ap device enable/disable
 * Input       : [tag] key, identify an app of device
 *               [enable] true is enable, false is disable
 * Output      : none
 * Return      : 0 is enable ok, otherwise is not
 */
int ap_device_enable(int tag, bool enable);

/*
 * Function    : ap_device_setdelay
 * Description : ap device setdelay
 * Input       : [tag] key, identify an app of device
 *               [ms] delay time, ms
 * Output      : none
 * Return      : 0 is set ok, otherwise is not
 */
int ap_device_setdelay(int tag, int ms);

/*
 * Function    : work_on_ap
 * Description : according to tag, retrun device work on ap or not
 * Input       : [tag] key, identify an app of device
 * Output      : none
 * Return      : true is work on ap, false is not
 */
bool work_on_ap(int tag);


#endif /* __DEVICE_INTERFACE_H */
