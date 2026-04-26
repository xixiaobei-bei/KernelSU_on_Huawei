/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019. All rights reserved.
 * Description: device_manager.h.
 * Create: 2019/11/05
 */

#ifndef __DEVICE_MANAGER_H
#define __DEVICE_MANAGER_H
#include <linux/of.h>
#include <linux/hisi/contexthub/protocol_as.h>

#define MAX_CHIP_INFO_LEN 50
#define MAX_NAME_LEN 50
#define PDC_SIZE 27

#define INVALID_VALUE	0xFFFFFFFF
#define INVALID_FILE_ID	0xFFFF
#define SENSOR_DETECT_RETRY 2
enum detect_mode {
	BOOT_DETECT,
	DETECT_RETRY,
	BOOT_DETECT_END = DETECT_RETRY + SENSOR_DETECT_RETRY - 1,
	REDETECT_LATER
};

#define MAX_REDETECT_NUM 100
struct sensor_redetect_state {
	u8 need_redetect_sensor;
	u8 need_recovery;
	u8 redetect_num;
	u32 detect_fail_num;
};

struct sensorlist_info {
	/*
	 * Name of this sensor.
	 * All sensors of the same "type" must have a different "name".
	 */
	char name[MAX_NAME_LEN];

	/* vendor of the hardware part */
	char vendor[MAX_CHIP_INFO_LEN];
	/*
	 * version of the hardware part + driver. The value of this field
	 * must increase when the driver is updated in a way that changes the
	 * output of this sensor. This is important for fused sensors when the
	 * fusion algorithm is updated.
	 */
	s32 version;

	/* maximum range of this sensor's value in SI units */
	s32 max_range;

	/* smallest difference between two values reported by this sensor */
	s32 resolution;

	/* rough estimate of this sensor's power consumption in mA */
	s32 power;

	/*
	 * this value depends on the reporting mode:
	 * continuous: minimum sample period allowed in microseconds
	 * on-change : 0
	 * one-shot  :-1
	 * special   : 0, unless otherwise noted
	 */
	s32 min_delay;

	/*
	 * number of events reserved for this sensor in the batch mode FIFO.
	 * If there is a dedicated FIFO for this sensor, then this is the
	 * size of this FIFO. If the FIFO is shared with other sensors,
	 * this is the size reserved for that sensor and it can be zero.
	 */
	u32 fifo_reserved_event_count;

	/*
	 * maximum number of events of this sensor that could be batched.
	 * This is especially relevant when the FIFO is shared between
	 * several sensors; this value is then set to the size of that FIFO.
	 */
	u32 fifo_max_event_count;
	/*
	 * This value is defined only for continuous mode and on-change sensors.
	 * It is the delay between two sensor events corresponding to the
	 * lowest frequency that this sensor supports.
	 * When lower frequencies are requested through batch()/setDelay()
	 * the events will be generated at this frequency instead.
	 * It can be used by the framework or applications to estimate
	 * when the batch FIFO may be full.
	 *
	 * @note
	 *   1) period_ns is in nanoseconds
	 *      where as maxDelay/minDelay are in microseconds.
	 *         continuous, on-change: maximum sampling period
	 *                                allowed in microseconds.
	 *         one-shot, special : 0
	 *   2) maxDelay should always fit within a 32 bit signed integer.
	 *      It is declared as 64 bit
	 *      on 64 bit architectures only for binary compatibility reasons.
	 * Availability: SENSORS_DEVICE_API_VERSION_1_3
	 */
	s32 max_delay;

	/*
	 * Flags for sensor. See SENSOR_FLAG_* above.
	 * Only the least significant 32 bits are used here.
	 * It is declared as 64 bit on 64 bit architectures
	 * only for binary compatibility reasons.
	 * Availability: SENSORS_DEVICE_API_VERSION_1_3
	 */
	u32 flags;
};

enum detect_state {
	DET_INIT = 0,
	DET_FAIL,
	DET_SUCC
};

#define MAX_CHIP_INFO_LEN 50
struct device_manager_node {
	char *device_name_str;
	u8 detect_result;
	int tag;
	const void *spara;
	int cfg_data_length;
	int (*detect)(struct device_node *dn);
	int (*cfg)(void);
	int (*send_calibrate_data)(bool);
	char device_chip_info[MAX_CHIP_INFO_LEN];
	struct sensorlist_info sensorlist_info;
	u16 dyn_file_id;
	u16 aux_file_id;
	struct list_head entry;
};

/*
 * Function    : register_contexthub_device
 * Description : register contexthub device, add node in list
 *               this func only can be called in device_initcall_sync,
 *               otherwise maybe have problem
 * Input       : [node] device_manager_node
 * Output      : none
 * Return      : 0 OK, other error
 */
int register_contexthub_device(struct device_manager_node *node);

/*
 * Function    : unregister_contexthub_device
 * Description : unregister contexthub device, delete node from list
 *               this func only can be called in device_initcall_sync,
 *               otherwise maybe have problem
 * Input       : [node] device_manager_node
 * Output      : none
 * Return      : 0 OK, other error
 */
int unregister_contexthub_device(struct device_manager_node *node);

/*
 * Function    : device_detect_init
 * Description : device manager init, called before contexthub boot,
 *               register msg process func for report mini ready and
 *               mcu ready package
 * Input       : none
 * Output      : none
 * Return      : none
 */
void device_detect_init(void);

/*
 * Function    : get_device_manager_node_by_tag
 * Description : according tag, get device_manager_node pointer
 * Input       : [tag] the key for device manager node
 * Output      : none
 * Return      : if node exist, return node, otherwise return NULL
 */
struct device_manager_node *get_device_manager_node_by_tag(int tag);

/*
 * Function    : device_set_cfg_data
 * Description : when boot on mcu ready stage, called
 * Input       : none
 * Output      : none
 * Return      : none
 */
void device_set_cfg_data(void);

/*
 * Function    : send_fileid_to_mcu
 * Description : when mcu ready stage of boot, recovery or resume, called.
 * Input       : none
 * Output      : none
 * Return      : 0 is ok, otherwise failed.
 */
int send_fileid_to_mcu(void);

/*
 * Function    : device_redetect_enter
 * Description : try to redect devices which detected fail before,
 *               called when screen off
 * Input       : none
 * Output      : none
 * Return      : none
 */
void device_redetect_enter(void);


/*
 * Function    : reset_calibrate_data_to_mcu
 * Description : reset all device calibrate data to mcu
 *               use no lock ipc send.Always called when recovery
 * Input       : none
 * Output      : none
 * Return      : none
 */
void reset_calibrate_data_to_mcu(void);

/*
 * Function    : send_calibrate_data_to_iomcu
 * Description : send calibrate data to iomcu(mostly for sensor)
 * Input       : [tag] key
 *             : [is_lock] use lock ipc or not
 * Output      : none
 * Return      : none
 */
void send_calibrate_data_to_iomcu(int tag, bool is_lock);

/*
 * Function    : device_manager_node_alloc
 * Description : alloc device manager node and init
 * Input       : none
 * Output      : none
 * Return      : pointer of alloc space
 */
struct device_manager_node *device_manager_node_alloc(void);

/*
 * Function    : device_manager_node_free
 * Description : free device_manager_node
 * Input       : none
 * Output      : none
 * Return      : none
 */
void device_manager_node_free(struct device_manager_node *device);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
/*
 * Function    : set_hw_dev_succ_flag
 * Description : set detect success flag
 * Input       : [tag] key
 * Output      : none
 * Return      : none
 */
void set_hw_dev_succ_flag(int tag);
#endif

#endif /* __DEVICE_MANAGER_H */
