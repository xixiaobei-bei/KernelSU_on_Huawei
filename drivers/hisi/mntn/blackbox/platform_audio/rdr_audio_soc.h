/*
 * rdr_audio_soc.h
 *
 * audio soc rdr
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __RDR_AUDIO_SOC_H__
#define __RDR_AUDIO_SOC_H__

#include <linux/hisi/rdr_pub.h>
#include <linux/types.h>

#ifndef RESET_CBFUNC_PRIO_LEVEL_LOWT
#define RESET_CBFUNC_PRIO_LEVEL_LOWT 0
#endif

#ifndef RESET_CBFUNC_PRIO_LEVEL_HIGH
#define RESET_CBFUNC_PRIO_LEVEL_HIGH 49
#endif

#define RESET_CBFUN_IGNORE_NAME "NONAME"
#define DRV_MODULE_NAME_LEN 9 /* exclude '\0' */
#define DSP_SEC_DDR_MAX_NUM 100
#define ICC_CHANNEL_FIFO_LEN 4
#define ICC_SHARE_FAMA_ADDR_OFFSET 0
#define OM_LOG_INNER_MAX_NUM 300
#define INNERLOG_ONE_INFO_LEN 124
#define UCOM_MEM_DYN_NODE_MAX_NUM 959
#define MEM_DYN_STATUS_ONE_INFO_LEN 307
#define MEM_DYN_NODE_ONE_INFO_LEN 167
#define COMPILE_TIME_BUFF_SIZE 24
#define RDR_FALG_OFFSET (DSP_FLAG_DATA_ADDR - DSP_DUMP_BIN_ADDR)
#define PARSE_INNERLOG_SIZE (OM_LOG_INNER_MAX_NUM * INNERLOG_ONE_INFO_LEN)
#define PARSE_MEM_DYN_LOG_SIZE (MEM_DYN_STATUS_ONE_INFO_LEN*UCOM_MEM_DYN_TYPE_BUTT + MEM_DYN_NODE_ONE_INFO_LEN*UCOM_MEM_DYN_NODE_MAX_NUM)
#define ICC_CHANNEL_NUM 1

struct drv_dsp_sec_info {
	unsigned int type;
	unsigned int src_addr;
	unsigned int des_addr;
	unsigned int size;
};

struct drv_dsp_sec_ddr_head {
	unsigned int sections_num;
	struct drv_dsp_sec_info sections[DSP_SEC_DDR_MAX_NUM];
};

/* for icc begin */
#define ICC_CHN_ID_MAX 16
#define ICC_STAT_MSG_NUM 50

struct icc_channel_fifo {
	unsigned int magic; /* fifo magic number, to identify the status of the channel fifo */
	unsigned int size; /* fifo size */
	unsigned int write; /* fifo read pointer */
	unsigned int read; /* fifo write pointer */
	unsigned char data[ICC_CHANNEL_FIFO_LEN];  /* fifo context */
};

struct icc_channel {
	unsigned int id; /* channel id */
	unsigned char name[ICC_CHANNEL_FIFO_LEN]; /* channel name */
	unsigned int send_seq_num; /* the serial number of the next sent packet */
	unsigned int recv_seq_num; /* the serial number of the last received packet */
	unsigned int ready_recv; /* receive fifo status: identifies whether the current fifo is available */
	unsigned int ipc_send_irq_id; /* the ipc interrupt used by the private channel to send data */
	unsigned int ipc_recv_irq_id; /* ipc interrupt for channel private receive data */
	unsigned int func_size; /* receive vector (subchannel) size */
	unsigned int fifo_recv; /* receive fifo pointer */
	unsigned int fifo_send; /* send fifo poniter */
};

struct icc_control {
	unsigned int cpu_id; /* curent nuclear cpu id */
	unsigned int channel_num; /* channel num */
	unsigned char channels[ICC_CHN_ID_MAX*4]; /* icc_channel structure poniter array */
};

struct icc_uni_msg_info {
	unsigned short channel_id;
	unsigned short packetlen;
	unsigned short msg_id;
	unsigned short drop;
	unsigned short fifo_read_pos;
	unsigned short fifo_write_pos;
	unsigned int time;
	unsigned int sender_pid;
	unsigned int receiver_pid;
};

struct icc_msg_fifo {
	unsigned int front;
	unsigned int rear;
	unsigned int size;
	struct icc_uni_msg_info msg[ICC_STAT_MSG_NUM];
};

struct icc_msg_info {
	struct icc_msg_fifo send;
	struct icc_msg_fifo recv;
};

struct icc_dbg {
	unsigned int protectword1;
	unsigned int total_length;
	unsigned int heap_base_offset;
	struct icc_control icc_ctrl;
	struct icc_channel icc_channels[ICC_CHANNEL_NUM];
	unsigned int state;
	unsigned int ipc_int_cnt;
	struct icc_msg_info msg_stat;
	unsigned int protectword2;
};
/* for icc end */

/* used to distinguish whether the callback function is called before modem/dsp reset or after reset */
enum DRV_RESET_CALLCBFUN_MOMENT {
	DRV_RESET_CALLCBFUN_RESET_BEFORE, /* modem/HIFI before reset */
	DRV_RESET_CALLCBFUN_RESET_AFTER, /* modem/HIFI after reset */
	DRV_RESET_CALLCBFUN_RESETING, /* modem/HIFI resetting */
	DRV_RESET_CALLCBFUN_MOEMENT_INVALID
};

/* record information of callback functions */
typedef int (*hifi_reset_cbfunc)(enum DRV_RESET_CALLCBFUN_MOMENT eparam, int userdata);
struct sreset_mgr_callbackfunc {
	char name[DRV_MODULE_NAME_LEN + 1];
	int priolevel;
	hifi_reset_cbfunc cbfun;
	int userdata;
};

/* link list unit of managing all callback functions */
struct sreset_mgr_lli {
	struct sreset_mgr_callbackfunc cbfuninfo;
	struct sreset_mgr_lli *pnext;
};

struct innerlog_record {
	unsigned int enlogid;
	unsigned int time_stamp;
	unsigned short fileid;
	unsigned short lineid;
	unsigned int value1;
	unsigned int value2;
	unsigned int value3;
};

struct innerlog_reset {
	unsigned int cnt;
	unsigned int time_stamp;
};

struct innerlog_obj {
	unsigned int curr_idx;
	struct innerlog_record records[OM_LOG_INNER_MAX_NUM];
	struct innerlog_reset wait_reset;
};

#define SOCDSP_ORIGINAL_INNERLOG_SIZE (sizeof(struct innerlog_obj))

/* mem_dyn */
enum UCOM_MEM_DYN_OM_ENABLE_ENUM {
	UCOM_MEM_DYN_OM_ENABLE_NO = 0,
	UCOM_MEM_DYN_OM_ENABLE_YES,
	UCOM_MEM_DYN_OM_ENABLE_BUTT
};

enum UCOM_MEM_DYN_TYPE_ENUM {
	UCOM_MEM_DYN_TYPE_DDR = 0,
	UCOM_MEM_DYN_TYPE_TCM,
	UCOM_MEM_DYN_TYPE_OCB, /* memory on ocb (on chip buffer, another name is sram) */
	UCOM_MEM_DYN_TYPE_USB_160K,
	UCOM_MEM_DYN_TYPE_USB_96K,
	UCOM_MEM_DYN_TYPE_BUTT
};

typedef struct {
	unsigned int addr;
	unsigned int size;
	unsigned short fileid;
	unsigned short lineid;
} mem_dyn_blk_stru;

typedef struct {
	unsigned int time_stamp;
	unsigned int curr_used_rate;
} mem_dyn_trace_stru;

struct mem_dyn_node {
	mem_dyn_blk_stru blk;
	unsigned int next; /* struct mem_dyn_node */
	unsigned int prev; /* struct mem_dyn_node */
};

struct mem_dyn_list {
	struct mem_dyn_node head;
	struct mem_dyn_node tail;
	unsigned int len;
};

struct mem_dyn_status {
	unsigned int enable;
	unsigned int total_size;
	unsigned int used_rate;
	mem_dyn_trace_stru mem_trace;
};

struct mem_dyn_ctrl {
	struct mem_dyn_status status[UCOM_MEM_DYN_TYPE_BUTT];
	struct mem_dyn_list freelist[UCOM_MEM_DYN_TYPE_BUTT];
	struct mem_dyn_list unfreelist[UCOM_MEM_DYN_TYPE_BUTT];
	struct mem_dyn_list idlelist;
	struct mem_dyn_node nodes[UCOM_MEM_DYN_NODE_MAX_NUM];
};

#define SOCDSP_ORIGINAL_DYNMEM_SIZE (sizeof(struct mem_dyn_ctrl))

int rdr_audio_soc_init(void);
void rdr_audio_soc_exit(void);
void rdr_audio_soc_reset(unsigned int modid, unsigned int etype, u64 coreid);

/* callback registration function before and after dsp reset */
int hifireset_regcbfunc(const char *pname, hifi_reset_cbfunc pcbfun, int userdata, int priolevel);
int hifireset_runcbfun(enum DRV_RESET_CALLCBFUN_MOMENT eparam);

#endif /* __RDR_AUDIO_SOC_H__ */

