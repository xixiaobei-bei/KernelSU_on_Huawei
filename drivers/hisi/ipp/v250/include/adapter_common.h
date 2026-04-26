/******************************************************************
 * Copyright    Copyright (c) 2015- Hisilicon Technologies CO., Ltd.
 * File name    adapter_common.h
 * Description:
 *
 * Version      1.0
 * Date         2015-07-07 14:23:09
 * Author       liujian_l00289972
 ********************************************************************/

#ifndef ADAPTER_COMMON_CS_H_INCLUDED
#define ADAPTER_COMMON_CS_H_INCLUDED

#include <linux/types.h>
#include <securec.h>
#include "ipp.h"
#include "include/config_table_cvdr.h"
#include "include/config_table_cmdlst.h"

#define CMDLST_BUFFER_SIZE         (0x1000)
#define CMDLST_HEADER_SIZE         (128)
#define CMDLST_CHANNEL_MAX         (8)
#define CMDLST_RD_CFG_MAX          (8)
#define CMDLST_32K_PAGE            (32768)

#define CMP_CHANNEL  7
#define RDR_CHANNEL  5
#define SLAM_CHANNEL 3
#define CPE_CHANNEL  1

#define STRIPE_NUM_EACH_CMP  (2)

#if 1
#define CMDLST_SET_REG(reg, val) cmdlst_set_reg(dev->cmd_buf, reg, val)
#else
#define CMDLST_SET_REG(reg, val) cmdlst_set_reg_by_cpu(reg, val)
#endif
#define CMDLST_SET_REG_INCR(reg, size, incr, is_read) cmdlst_set_reg_incr(dev->cmd_buf, reg, size, incr, is_read)
#define CMDLST_SET_REG_DATA(data) cmdlst_set_reg_data(dev->cmd_buf, data)

#define CMDLST_BURST_MAX_SIZE      256
#define CMDLST_PADDING_DATA        0xFF1FFFFD
#define CMDLST_STRIPE_MAX_NUM      (56)

#define UNSIGNED_INT_MAX      (0xffffffff)
#define LOG_NAME_LEN          (64)

typedef enum _cmdlst_eof_mode_e {
	CMD_EOF_CPE_MODE = 0,
	CMD_EOF_SLAM_MODE,
	CMD_EOF_REORDER_MODE,
	CMD_EOF_COMPARE_MODE,
} cmdlst_eof_mode_e;

typedef struct _cmd_buf_t {
	unsigned long long start_addr;
	unsigned int start_addr_isp_map;
	unsigned int buffer_size;
	unsigned int header_size;
	unsigned long long data_addr;
	unsigned int data_size;
	void *next_buffer;
} cmd_buf_t;

typedef struct _schedule_cmdlst_link_t {
	unsigned int stripe_cnt;
	unsigned int stripe_index;
	cmd_buf_t    cmd_buf;
	cfg_tab_cmdlst_t cmdlst_cfg_tab;
	void *data;
	struct list_head list_link;
} schedule_cmdlst_link_t;

typedef struct _cmdlst_rd_cfg_info_t {
	// read buffer address
	unsigned int fs;
	// read reg num in one stripe
	unsigned int rd_cfg_num;
	// read reg cfgs
	unsigned int rd_cfg[CMDLST_RD_CFG_MAX];
} cmdlst_rd_cfg_info_t;

typedef struct _cmdlst_stripe_info_t {
	unsigned int  is_first_stripe;
	unsigned int  is_last_stripe;
	unsigned int  is_need_set_sop;
	unsigned int  irq_mode;
	unsigned long long  irq_mode_sop;
	unsigned int  hw_priority;
	unsigned int  resource_share;
	unsigned int  en_link;
	unsigned int  ch_link;
	unsigned int  ch_link_act_nbr;
	cmdlst_rd_cfg_info_t rd_cfg_info;
} cmdlst_stripe_info_t;

typedef struct _cmdlst_para_t {
	unsigned int stripe_cnt;
	cmdlst_stripe_info_t cmd_stripe_info[CMDLST_STRIPE_MAX_NUM];
	void *cmd_entry;
	unsigned int channel_id;
} cmdlst_para_t;

typedef enum _cfg_irq_type_e {
	SET_IRQ = 0,
	CLR_IRQ,
} cfg_irq_type_e;

typedef struct _crop_region_info_t {
	unsigned int   x;
	unsigned int   y;
	unsigned int   width;
	unsigned int   height;
} crop_region_info_t;

typedef struct _isp_size_t {
	unsigned int   width;
	unsigned int   height;
} isp_size_t;

typedef struct _isp_stripe_info_t {
	crop_region_info_t  crop_region;
	isp_size_t          pipe_work_size;
	isp_size_t          full_size;

	unsigned int        stripe_cnt;
	unsigned int        overlap_left[MAX_CPE_STRIPE_NUM];
	unsigned int        overlap_right[MAX_CPE_STRIPE_NUM];
	unsigned int        stripe_width[MAX_CPE_STRIPE_NUM];
	unsigned int        stripe_start_point[MAX_CPE_STRIPE_NUM];
	unsigned int        stripe_end_point[MAX_CPE_STRIPE_NUM];
} isp_stripe_info_t;

typedef struct _df_size_constrain_t {
	unsigned int hinc;
	unsigned int pix_align;
	unsigned int out_width;
} df_size_constrain_t;

extern void df_size_dump_stripe_info(isp_stripe_info_t *p_stripe_info, char *s);
void df_size_split_stripe(unsigned int constrain_cnt, df_size_constrain_t *p_size_constrain,
						  isp_stripe_info_t *p_stripe_info, unsigned int overlap, unsigned int width);

int ipp_eop_handler(cmdlst_eof_mode_e mode);
int cmdlst_priv_prepare(void);

int df_sched_prepare(cmdlst_para_t *cmdlst_para);
int df_sched_start(cmdlst_para_t *cmdlst_para);

int cmdlst_set_buffer_padding(cmd_buf_t *cmd_buf);
int df_sched_set_buffer_header(cmdlst_para_t *cmdlst_para);
int cmdlst_read_buffer(unsigned int stripe_index, cmd_buf_t *cmd_buf, cmdlst_para_t *cmdlst_para);

void cmdlst_set_reg(cmd_buf_t *cmd_buf, unsigned int reg, unsigned int val);
void cmdlst_set_reg_incr(cmd_buf_t *cmd_buf, unsigned int reg, unsigned int size, unsigned int incr,
						 unsigned int is_read);
void cmdlst_set_reg_data(cmd_buf_t *cmd_buf, unsigned int data);
void cmdlst_do_flush(cmdlst_para_t *cmdlst_para);
void dump_addr(unsigned long long addr, int num, char *info);
void cmdlst_buff_dump(cmd_buf_t *cmd_buf);
void cmdlst_set_reg_by_cpu(unsigned int reg, unsigned int val);

extern int seg_src_set_cmdlst_wr_buf(cmd_buf_t *cmd_buf, unsigned int wr_addr, unsigned int rd_addr,
									 unsigned int data_size);
extern void ipp_update_cmdlst_cfg_tab(cmdlst_para_t *cmdlst_para);
extern void cpe_set_cmdlst_stripe(cmdlst_stripe_info_t *stripe, unsigned int stripe_cnt);


#endif /* ADAPTER_COMMON_CS_H_INCLUDED */



