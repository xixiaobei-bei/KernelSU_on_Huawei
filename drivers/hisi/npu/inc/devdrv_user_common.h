/*
 * devdrv_user_common.h
 *
 * about devdrv user common
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef __DEVDRV_USER_COMMON_H
#define __DEVDRV_USER_COMMON_H

#include "devdrv_user_common_adapter.h"
#include "devdrv_user_common_model.h"

#define MAX_DAVINCI_NUM_OF_ONE_CHIP   4

#define DEVDRV_SHM_MAPS_SIZE   (4 * 1024 * 1024)
#define DEVDRV_SRAM_TS_SHM_SIZE 0x1000
#define DEVDRV_MAX_CQ_SLOT_SIZE 128

#define DEVDRV_CACHELINE_OFFSET 6
#define DEVDRV_CACHELINE_SIZE   64
#define DEVDRV_CACHELINE_MASK   (DEVDRV_CACHELINE_SIZE - 1)

#define DEVDRV_MAX_CQ_DEPTH  1024
#define DEVDRV_MAX_DAVINCI_NUM  1
#define DEVDRV_MAX_SQ_DEPTH	256

#define DEVDRV_FUNCTIONAL_SQ_FIRST_INDEX  112
#define DEVDRV_FUNCTIONAL_CQ_FIRST_INDEX  116

#define DEVDRV_MAX_FUNCTIONAL_SQ_NUM    4
#define DEVDRV_MAX_FUNCTIONAL_CQ_NUM    10

#define DEVDRV_MAILBOX_MAX_FEEDBACK  16
#define DEVDRV_MAILBOX_STOP_THREAD   0x0FFFFFFF
#define DEVDRV_BUS_DOWN     0x0F00FFFF

#define DEVDRV_HCCL_NAME_SIZE       64
#define DEVDRV_HCCL_MAX_NODE_NUM    128

#define DEVDRV_MAX_INTERRUPT_NUM    32
#define DEVDRV_MAX_MEMORY_DUMP_SIZE (4 * 1024 * 1024)

#define DEVDRV_USER_CONFIG_NAME_LEN 32
#define DEVDRV_USER_CONFIG_VALUE_LEN 128

#define DEVDRV_BB_DEVICE_ID_INFORM  0x66020004
#define CHIP_BASEADDR_PA_OFFSET 0x200000000000ULL

enum devdrv_ts_status {
	DEVDRV_TS_WORK = 0x0,
	DEVDRV_TS_SLEEP,
	DEVDRV_TS_DOWN,
	DEVDRV_TS_INITING,
	DEVDRV_TS_BOOTING,
	DEVDRV_TS_FAIL_TO_SUSPEND,
	DEVDRV_TS_SEC_WORK, // secure power up
	DEVDRV_TS_MAX_STATUS
};

struct devdrv_ts_sq_info {
	u32 head;
	u32 tail;
	u32 credit;
	u32 index;
	int uio_num;
	int uio_fd;
	int uio_map;
	u8 *uio_addr;
	int uio_size;
	u32 stream_num;
	u64 send_count;
	void *sq_sub;
};

struct devdrv_ts_cq_info {
	u32 head;
	u32 tail;
	volatile u32 count_report;
	u32 index;
	u32 phase;
	u32 int_flag;
	int uio_num;
	int uio_fd;
	int uio_map;
	u8 *uio_addr;
	int uio_size;
	u32 stream_num;
	u64 receive_count;
	void *cq_sub;
	u8 slot_size;
};

struct devdrv_stream_info {
	int id;
	u32 devid;
	// used for non-sink stream or sink stream
	u32 cq_index;
	u32 sq_index;
	void *stream_sub;
	int pid;
	u32 strategy;
	// used for sink stream
	u16 smmu_substream_id;
	u8 priority;
	u8 resv[5];
};

struct devdrv_mailbox_user_message {
	u8 message_payload[64];
	int message_length;
	int feedback_num;
	u8 *feedback_buffer;	/*
				 * if a sync message need feedback, must alloc buffer for feedback data.
				 * if a async message need feedback, set this to null,
				 * because driver will send a callback parameter to callback func,
				 * app has no need to free callback parameter in callback func.
				 */
	int sync_type;
	int cmd_type;
	int message_index;
	int message_pid;
};

struct devdrv_mailbox_feedback {
	void (*callback)(void *data);
	u8 *buffer;
	int feedback_num;
	int process_result;
};

struct devdrv_user_parameter {
	u32 devid;
	u32 cq_slot_size;
	u16 disable_wakelock;
};

struct devdrv_pcie_drv_info {
	u32 dev_id;
	u8 bus_number;
	u8 dev_number;
	u8 function_number;
};

struct devdrv_svm_to_devid {
	u32 src_devid;
	u32 dest_devid;
	unsigned long src_addr;
	unsigned long dest_addr;
};

struct devdrv_channel_info_devid {
	char name[DEVDRV_HCCL_NAME_SIZE];
	u32 handle;
	u32 event_id;
	u32 src_devid;
	u32 dest_devid;
	void *dest_doorbell_pa;
	void *ack_doorbell_pa;
	void *dest_mailbox_pa;
	/* for ipc event query */
	u32 status;
	u64 timestamp;
};

struct devdrv_trans_info {
	u32 src_devid;
	u32 dest_devid;
	u8 ways;
};

struct devdrv_notify_ioctl_info {
	u32 dev_id;
	u32 notify_id;
	u64 dev_addr;
	u64 host_addr;
	char name[DEVDRV_HCCL_NAME_SIZE];
};

struct devdrv_hardware_spec {
	u32 devid;
	u32 ai_core_num;
	u32 first_ai_core_id;
	u32 ai_cpu_num;
	u32 first_ai_cpu_id;
};

struct devdrv_hardware_inuse {
	u32 devid;
	u32 ai_core_num;
	u32 ai_core_error_bitmap;
	u32 ai_cpu_num;
	u32 ai_cpu_error_bitmap;
};

struct devdrv_manager_hccl_devinfo {
	u8 env_type;
	u32 dev_id;
	u32 ctrl_cpu_ip;
	u32 ctrl_cpu_id;
	u32 ctrl_cpu_core_num;
	u32 ctrl_cpu_endian_little;
	u32 ts_cpu_core_num;
	u32 ai_cpu_core_num;
	u32 ai_core_num;
	u32 ai_cpu_bitmap;
	u32 ai_core_id;
	u32 ai_cpu_core_id;
	u32 hardware_version;  /* mini, cloud, lite, etc. */
	u32 num_dev;
	u32 devids[DEVDRV_MAX_DAVINCI_NUM];
};

struct devdrv_sysrdy_info {
	u32 probe_dev_num;
	u32 rdy_dev_num;
};

enum devdrv_arch_type {
	ARCH_BEGIN = 0,
	ARCH_V100 = ARCH_BEGIN,
	ARCH_V200,
	ARCH_END
};

enum devdrv_chip_type {
	CHIP_BEGIN = 0,
	CHIP_MINI = CHIP_BEGIN,
	CHIP_CLOUD,
	CHIP_LITE_PHOENIX,
	CHIP_LITE_ORLANDO,
	CHIP_TINY_PHOENIX,
	CHIP_END
};

enum devdrv_version {
	VER_BEGIN = 0,
	VER_NA = VER_BEGIN,
	VER_ES,
	VER_CS,
	VER_CS2,
	VER_END
};

#define PLAT_COMBINE(arch, chip, ver) (((arch) << 16) | ((chip) << 8) | (ver))
#define PLAT_GET_ARCH(type) (((type) >> 16) & 0xffff)
#define PLAT_GET_CHIP(type) (((type) >> 8) & 0xff)
#define PLAT_GET_VER(type)  ((type) & 0xff)

enum devdrv_hardware_version {
	DEVDRV_PLATFORM_MINI_V1 = PLAT_COMBINE(ARCH_V100, CHIP_MINI, VER_NA),
	DEVDRV_PLATFORM_CLOUD_V1 = PLAT_COMBINE(ARCH_V100, CHIP_CLOUD, VER_NA),
	DEVDRV_PLATFORM_LITE_PHOENIX_ES = PLAT_COMBINE(ARCH_V100, CHIP_LITE_PHOENIX, VER_ES),
	DEVDRV_PLATFORM_LITE_PHOENIX_CS = PLAT_COMBINE(ARCH_V100, CHIP_LITE_PHOENIX, VER_CS),
	DEVDRV_PLATFORM_LITE_PHOENIX_CS2 = PLAT_COMBINE(ARCH_V100, CHIP_LITE_PHOENIX, VER_CS2),
	DEVDRV_PLATFORM_LITE_ORLANDO = PLAT_COMBINE(ARCH_V100, CHIP_LITE_ORLANDO, VER_NA),
	DEVDRV_PLATFORM_TINY_PHOENIX_ES = PLAT_COMBINE(ARCH_V100, CHIP_TINY_PHOENIX, VER_ES),
	DEVDRV_PLATFORM_TINY_PHOENIX_CS = PLAT_COMBINE(ARCH_V100, CHIP_TINY_PHOENIX, VER_CS),
	DEVDRV_PLATFORM_END
};

struct devdrv_occupy_stream_id {
	u16 count;
	u16 id[DEVDRV_MAX_STREAM_ID];
};

struct devdrv_black_box_devids {
	u32 dev_num;
	u32 devids[DEVDRV_MAX_DAVINCI_NUM];
};

struct devdrv_black_box_user {
	u32 devid;
	u32 size;
	u64 phy_addr;
	void *dst_buffer;
	u32 thread_should_stop;
	u32 exception_code;
	u64 tv_sec;
	u64 tv_nsec;
	union {
		struct devdrv_black_box_devids bbox_devids;
	} priv_data;
};

struct devdrv_module_status {
	u8 lpm3_start_fail;
	u8 lpm3_lost_heart_beat;
	u8 ts_start_fail;
	u8 ts_lost_heart_beat;
	u8 ts_sram_broken;
	u8 ts_sdma_broken;
	u8 ts_bs_broken;
	u8 ts_l2_buf0_broken;
	u8 ts_l2_buf1_broken;
	u8 ts_spcie_broken;
};

struct devdrv_dev_dma_addr {
	u32 devid;
	u32 size;
	u64 host_phy_addr;
	u64 device_phy_addr;
};

struct devdrv_dev_interrupt_info {
	u32 devid;
	u32 count;
	u32 reply_count;
	u32 interrupt_number[DEVDRV_MAX_INTERRUPT_NUM];
	u64 interrupt_phy_addr[DEVDRV_MAX_INTERRUPT_NUM];
};

struct devdrv_dev_interrupt_recv_signal {
	u32 interrupt_number;
	u32 timeout;
	u32 db_value;
};

struct devdrv_pcie_pre_reset {
	u32 dev_id;
};

struct devdrv_pcie_rescan {
	u32 dev_id;
};

struct devdrv_get_user_config_para {
	char config_name[DEVDRV_USER_CONFIG_NAME_LEN];
	char config_value[DEVDRV_USER_CONFIG_VALUE_LEN];
	u32 config_value_len;
};

#define DEVDRV_SQ_INFO_SIZE  sizeof(struct devdrv_ts_sq_info)
#define DEVDRV_CQ_INFO_SIZE  sizeof(struct devdrv_ts_cq_info)
#define DEVDRV_STREAM_INFO_SIZE  sizeof(struct devdrv_stream_info)

#define DEVDRV_SQ_INFO_OCCUPY_SIZE \
	(DEVDRV_SQ_INFO_SIZE * DEVDRV_MAX_SQ_NUM)
#define DEVDRV_CQ_INFO_OCCUPY_SIZE \
	(DEVDRV_CQ_INFO_SIZE * DEVDRV_MAX_CQ_NUM)
#define DEVDRV_STREAM_INFO_OCCUPY_SIZE \
	(DEVDRV_STREAM_INFO_SIZE * DEVDRV_MAX_STREAM_ID)
#define DEVDRV_MAX_INFO_SIZE \
	(DEVDRV_INFO_OCCUPY_SIZE + sizeof(u32))
#define DEVDRV_MAX_INFO_ORDER   (get_order(DEVDRV_MAX_INFO_SIZE))

#define PMU_EMMC_VCC_CHANNEL     7
#define PMU_EMMC_VCCQ_CHANNEL    14
#define ADCIN7_SLOT0  7
#define ADCIN8_SLOT1  8

struct devdrv_emmc_voltage_para {
	int emmc_vcc;  // should be 2950 mv
	int emmc_vccq;  // should be 1800 mv
};

#define DMANAGE_ERROR_ARRAY_NUM  128
struct devdrv_error_code_para {
	int error_code_count;
	unsigned int error_code[DMANAGE_ERROR_ARRAY_NUM];
};

struct tsensor_ioctl_arg {
	u32 coreid;
	u32 result_size;
	u32 result[4];
};

/* add necessary dfx function if you need */
enum devdrv_dfx_cmd {
	DEVDRV_DFX_QUERY_STATUS,
	DEVDRV_DFX_MAX_CMD
};

/*
 * DEVDRV_DFX_QUERY_STATUS
 * add necessary value info if you need, remember add both user code and kernel code
 */
struct devdrv_status_info {
	u16 sq_head[DEVDRV_MAX_SQ_NUM];
	u16 sq_tail[DEVDRV_MAX_SQ_NUM];
	u16 cq_head[DEVDRV_MAX_CQ_NUM];
	u16 cq_tail[DEVDRV_MAX_CQ_NUM];
	u16 func_sq_head[DEVDRV_MAX_DFX_SQ_NUM];
	u16 func_sq_tail[DEVDRV_MAX_DFX_SQ_NUM];
	u16 func_cq_head[DEVDRV_MAX_DFX_CQ_NUM];
	u16 func_cq_tail[DEVDRV_MAX_DFX_CQ_NUM];
	u64 sq_addr[DEVDRV_MAX_SQ_NUM];
	u64 cq_addr[DEVDRV_MAX_CQ_NUM];
	u64 func_sq_addr[DEVDRV_MAX_DFX_SQ_NUM];
	u64 func_cq_addr[DEVDRV_MAX_DFX_CQ_NUM];
	u16 stream_sq[DEVDRV_MAX_NON_SINK_STREAM_ID];
	u16 stream_cq[DEVDRV_MAX_NON_SINK_STREAM_ID];
	u32 ts_beat_count;
	u32 m3_beat_count;
	u32 ts_status;
	u8 ts_beat_en;
	u8 m3_beat_en;
	u8 cq_phase[DEVDRV_MAX_CQ_NUM];
	u8 func_cq_phase[DEVDRV_MAX_DFX_CQ_NUM];
};

/* ioctl parameter */
struct devdrv_dfx_para {
	u32 devid;
	u32 cmd;
	void *in;
	void *out;
};

enum devdrv_container_cmd {
	DEVDRV_CONTAINER_NOTIFY,
	DEVDRV_CONTAINER_ALLOCATE_TFLOPS,
	DEVDRV_CONTAINER_IS_CONTAINER,
	DEVDRV_CONTAINER_DOCKER_EXIT,
	DEVDRV_CONTAINER_DOCKER_CREATE,
	/* container tflops mode cmd end */
	/* container device assignment mode cmd begin */
	DEVDRV_CONTAINER_ASSIGN_NOTIFY,
	DEVDRV_CONTAINER_ASSIGN_ALLOCATE_DEVICES,
	DEVDRV_CONTAINER_ASSIGN_IS_ASSIGN_MODE,
	DEVDRV_CONTAINER_ASSIGN_SET_UUID,
	/* container device assignment mode cmd end */
	DEVDRV_CONTAINER_IS_IN_CONTAINER,
	DEVDRV_CONTAINER_MAX_CMD
};

struct devdrv_container_para {
	struct devdrv_dfx_para para;
};

#define DEVDRV_MINI_TOTAL_TFLOP 16
#define DEVDRV_MINI_FP16_UNIT   1
#define DEVDRV_MINI_INT8_UNIT   2

struct devdrv_container_alloc_para {
	u32 num;
	u32 npu_id[DEVDRV_MAX_DAVINCI_NUM];
};

struct devdrv_container_tflop_config {
	u32 tflop_mode;
	u32 total_tflop;
	u32 alloc_unit;
	u32 tflop_num;
};

enum devdrv_run_mode {
	DEVDRV_NORMAL_MODE = 0,
	DEVDRV_CONTAINER_MODE,
	DEVDRV_MAX_RUN_MODE
};

enum devdrv_container_tflop_mode {
	DEVDRV_FP16 = 0,
	DEVDRV_INT8,
	DEVDRV_MAX_TFLOP_MODE
};

struct devdrv_time_sync {
	int tz_minuteswest;
	int tz_dsttime;
	u8 thread_should_exit;
};

#define DEVDRV_MAX_LIB_LENGTH 128
struct devdrv_load_kernel {
	unsigned int devid;
	unsigned int share;
	char libname[DEVDRV_MAX_LIB_LENGTH];
	unsigned char sha256[32];
	int pid;
	void *binary;
	unsigned int size;
};

struct devdrv_load_kernel_serve {
	struct devdrv_load_kernel load_kernel;
	u8 thread_should_exit;
	u8 save_state;  // succ: 0, fail: 1
};

/*
|___SQ___|____INFO_____|__DOORBELL___|___CQ____|
*/
#define DEVDRV_VM_BLOCK_OFFSET  (32 * 1024 * 1024)
#define DEVDRV_VM_CQ_QUEUE_SIZE  (64 * 1024 * 1024)
#define DEVDRV_VM_CQ_SLOT_SIZE  (128 * 1024)

#define DEVDRV_VM_MEM_START  0xFFC0000000ULL
#define DEVDRV_VM_MEM_SIZE  (DEVDRV_VM_BLOCK_OFFSET * 3 + DEVDRV_VM_CQ_QUEUE_SIZE)

/* *custom ioctrl* */
typedef enum {
	DEVDRV_IOC_VA_TO_PA,             // current only use in lite
	DEVDRV_IOC_GET_SVM_SSID,         // current only use in lite
	DEVDRV_IOC_GET_CHIP_INFO,        // current only use in lite
	DEVDRV_IOC_ALLOC_CONTIGUOUS_MEM, // current only use in lite
	DEVDRV_IOC_FREE_CONTIGUOUS_MEM,  // current only use in lite
	DEVDRV_IOC_POWERUP,              // current only use in lite
	DEVDRV_IOC_POWERDOWN,            // current only use in lite
	DEVDRV_IOC_REBOOT,               // current only use in lite
	DEVDRV_IOC_LOAD_STREAM_BUFF,     // load stream buff of model
	DEVDRV_IOC_RESERVED
} devdrv_custom_ioc_t;

typedef struct {
	u32 version;
	u32 cmd;
	u32 result;
	u32 reserved;
	u64 arg;
} devdrv_custom_para_t;

typedef struct tag_stream_buffer_info {
	uint32_t stream_id;
	uint32_t mod_id;
	uint64_t len;
	uint64_t *addr;
	uint8_t  priority;
	uint8_t  reserve[7];
} devdrv_stream_buff_info_t; // total 32 Bytes

/* just for ai core bypass mode */
#define AI_PAGE_SIZE  4096
#define AI_PAGE_MASK  (~(AI_PAGE_SIZE - 1))

struct davinci_area_info {
	unsigned long va;
	unsigned long pa;
	unsigned long len;
};

#define UINT64  uint64_t
#define UINT32  uint32_t
#define UINT16  uint16_t
#define UINT8   uint8_t

struct process_info {
	pid_t vpid;
	UINT64 ttbr;
	UINT64 tcr;
	int pasid;
	UINT32 flags;
};

struct devdrv_chip_info {
	UINT32 l2_size;
	UINT32 reserved[3];
};

enum {
	STREAM_STRATEGY_NONSINK = 0,
	STREAM_STRATEGY_SINK = 1,
	STREAM_STRATEGY_MAX
};

struct devdrv_stream_strategy_ioctl_info {
	int stream_id;
	u32 strategy;
	u32 devid;
};

// for custom ioctl power up&down
typedef struct npu_secure_info {
	uint32_t secure_mode;
} npu_secure_info_t;

#endif
