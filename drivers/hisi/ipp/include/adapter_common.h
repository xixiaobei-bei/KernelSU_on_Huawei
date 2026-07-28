/******************************************************************
 * Copyright    Copyright (c) 2015- Hisilicon Technologies CO., Ltd.
 * File name    adapter_common.h
 * Description:
 *
 * Version      1.0
 * Date         2015-07-07 14:23:09
 * Author       liujian_l00289972
 ********************************************************************/

#ifndef ADAPTER_COMMON_H_INCLUDED
#define ADAPTER_COMMON_H_INCLUDED

#include <linux/types.h>
#include "ipp.h"
#include "include/config_table_cvdr.h"
#include "include/config_table_cmdlst.h"

/*
#include "../device/include/config_table_cvdr.h"
#include "../device/include/config_table_cmdlst.h"
*/
#define CMDLST_BUFFER_SIZE         0x10000
#define CMDLST_HEADER_SIZE         128
#define CMDLST_CHANNEL_MAX                  8
#define CMDLST_RD_CFG_MAX                   8
#define CMDLST_32K_PAGE                     32768
#define MM_CHANNEL 0
#define SLAM_CHANNEL 1
#define SLAMTEST_CHANNEL 1

#define DEBUG_CMDLST

#if 1
#define CMDLST_SET_REG(reg, val) cmdlst_set_reg(dev->cmd_buf, reg, val)
#else
#define CMDLST_SET_REG(reg, val) cmdlst_set_reg_by_cpu(reg, val)
#endif
#define CMDLST_SET_REG_INCR(reg, size, incr) cmdlst_set_reg_incr(dev->cmd_buf, reg, size, incr)
#define CMDLST_SET_REG_DATA(data) cmdlst_set_reg_data(dev->cmd_buf, data)

#define CMDLST_BURST_MAX_SIZE 	256
#define CMDLST_PADDING_DATA        0xFF1FFFFD
#define CMDLST_BUF_SIZE            0x10000
#define CMDLST_BUF_ALIGN_SIZE      0x8000
#define CMDLST_STRIPE_MAX_NUM      16
#define CMDLST_BUF_MAX_NUM         96

#define UNSIGNED_INT_MAX      (0xffffffff)
#define LOG_NAME_LEN          (64)

typedef enum _cmdlst_eof_mode_e
{
    CMD_EOF_MCF_MODE = 0,
    CMD_EOF_MFNR_MODE,
    CMD_EOF_SLAM_MODE,
    CMD_EOF_SLAMTEST_MODE
}cmdlst_eof_mode_e;


typedef enum _cpe_cmdlst_irq_mode
{
    CMD_IRQ_MCF_MFNR_FULL_MODE  = 0x39FF,
    CMD_IRQ_MCF_FULL_MODE_TWO_INPUT   = 0x39FB,
    CMD_IRQ_MCF_FULL_MODE_ONE_INPUT   = 0x39F9,
    CMD_IRQ_MCF_PARTIAL_MODE                =0x18F9,
    CMD_IRQ_MCF_YGF_SINGLE_MODE         = 0x18D0,
    CMD_IRQ_MCF_YGF_SINGLE_MODE_LF_ONLY = 0x1090,
    CMD_IRQ_MCF_YGF_DUAL_MODE           = 0x18F0,
    CMD_IRQ_MCF_YGF_DUAL_MODE_LF_ONLY   = 0x10B0,
    CMD_IRQ_MFNR_FULL_COLOR_NO_MASK_MODE = 0x18FF,
    CMD_IRQ_MFNR_FULL_MONO_MODE         = 0x31AB,
    CMD_IRQ_MFNR_FULL_MONO_NO_MASK_MODE = 0x10AB,
    CMD_IRQ_MFNR_PARTIAL_COLOR_MODE     = 0x18FD,
    CMD_IRQ_MFNR_PARTIAL_MONO_MODE      = 0x10A9,
    CMD_IRQ_SLAM_SCOREMAP_MODE              =0x1C,
    CMD_IRQ_SLAM_PYRAMID_MODE               =0x0E,
    CMD_IRQ_SLAM_NORMAL_MODE                =0x0C,
    CMD_IRQ_MFNR_READ_STAT_MODE             =0x200,
    CMD_IRQ_SLAM_READ_STAT_MODE             =0x1,
    CMD_IRQ_MAX = 0x0,
}cpe_cmdlst_irq_mode;

typedef struct _cmd_buf_t
{
    unsigned long long start_addr;
    unsigned int start_addr_isp_map;
    unsigned int buffer_size;
    unsigned int header_size;
    unsigned long long data_addr;
    unsigned int data_size;
    void * next_buffer;
} cmd_buf_t;

typedef struct _schedule_cmdlst_link_t
{
    cmd_buf_t    cmd_buf;
    void* data;
    struct list_head list_link;
    cpe_cmdlst_irq_mode  irq_mode;
}schedule_cmdlst_link_t;

typedef struct _cmdlst_rd_cfg_info_t
{
    // read buffer address
    unsigned int fs;
    // read reg num in one stripe
    unsigned int rd_cfg_num;
    // read reg cfgs
    unsigned int rd_cfg[CMDLST_RD_CFG_MAX];
}cmdlst_rd_cfg_info_t;

typedef struct _cmdlst_stripe_info_t
{
    unsigned int  is_last_stripe;
    cmdlst_rd_cfg_info_t rd_cfg_info;
}cmdlst_stripe_info_t;

typedef struct _cmdlst_para_t
{
    unsigned int stripe_cnt;
    cmdlst_stripe_info_t cmd_stripe_info[CMDLST_STRIPE_MAX_NUM];
    void* cmd_entry;
    unsigned int channel_id;
}cmdlst_para_t; 

typedef enum _cfg_irq_type_e
{
    SET_IRQ = 0,
    CLR_IRQ,
}cfg_irq_type_e;

typedef struct _crop_region_info_t
{
    unsigned int   x;
    unsigned int   y;
    unsigned int   width;
    unsigned int   height;
} crop_region_info_t;

typedef struct _isp_size_t
{
    unsigned int   width;
    unsigned int   height;
} isp_size_t;

typedef struct _isp_stripe_info_t
{
    crop_region_info_t  crop_region;
    isp_size_t          pipe_work_size;
    isp_size_t          full_size;

    unsigned int        stripe_cnt;
    unsigned int        overlap_left[MAX_STRIPE_NUM];
    unsigned int        overlap_right[MAX_STRIPE_NUM];
    unsigned int        stripe_width[MAX_STRIPE_NUM];
    unsigned int        stripe_start_point[MAX_STRIPE_NUM];
    unsigned int        stripe_end_point[MAX_STRIPE_NUM];
} isp_stripe_info_t;

typedef struct _df_size_constrain_t
{
    unsigned int hinc;
    unsigned int pix_align;
    unsigned int out_width;
} df_size_constrain_t;

extern void df_size_dump_stripe_info(isp_stripe_info_t *p_stripe_info, char *s);
void df_size_split_stripe(unsigned int constrain_cnt, df_size_constrain_t* p_size_constrain, isp_stripe_info_t* p_stripe_info,unsigned int overlap, unsigned int width);

int cmdlst_eop_handler(cmdlst_eof_mode_e mode);
int df_sched_prepare(cmdlst_para_t * cmdlst_para);
int df_sched_start(cfg_tab_cmdlst_t* cmdlst_cfg, cmdlst_para_t * cmdlst_para);
int cmdlst_set_buffer_padding(cmd_buf_t *cmd_buf);
int df_sched_set_buffer_header(cmdlst_para_t * cmdlst_para);

void cmdlst_set_reg(cmd_buf_t *cmd_buf, unsigned int reg, unsigned int val);
void cmdlst_set_reg_incr(cmd_buf_t *cmd_buf, unsigned int reg, int size, int incr);
void cmdlst_set_reg_data(cmd_buf_t *cmd_buf, unsigned int data);
void cmdlst_do_flush(cmdlst_para_t * cmdlst_para);
void dump_addr(unsigned long long addr, int num, char *info);
void cmdlst_buff_dump(cmd_buf_t* cmd_buf);
void cmdlst_set_reg_by_cpu(unsigned int reg, unsigned int val);

#endif /* ADAPTER_COMMON_H_INCLUDED */



