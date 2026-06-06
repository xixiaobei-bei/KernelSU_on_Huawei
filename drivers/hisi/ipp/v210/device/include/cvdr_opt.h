/******************************************************************
 * Copyright    Copyright (c) 2015- Hisilicon Technologies CO., Ltd.
 * File name    df_cvdr_opt.h
 * Description:
 *
 * Version      1.0
 * Date         2015-07-02 11:23:22
 * Author       mojing_mWX218223
 *******************************************************************/

#ifndef _CVDR_OPT_H
#define _CVDR_OPT_H

#include "cvdr_drv.h"
#include "cvdr.h"
#include "config_table_cvdr.h"

typedef struct _cvdr_opt_bw_t
{
	unsigned int       srt;
    unsigned int       pclk;
    unsigned int       throughput;
}cvdr_opt_bw_t;

typedef struct _cvdr_opt_fmt_t
{
    unsigned int       id;  
    unsigned int       width;
    unsigned int       height;
    unsigned int       line_size;
    unsigned int       addr;
    unsigned int       expand;
    unsigned int       full_width;
    cvdr_pix_fmt_e      pix_fmt;
}cvdr_opt_fmt_t;

typedef struct _pix_fmt_info_t
{
    char name[64];
    int  compact_size;
    int  expand_size;
    int  pix_num;
    int  pix_fmt;
}pix_fmt_info_t;

static const pix_fmt_info_t pix_fmt_info[19] =
{
    { "1PF8",   8,  8, 1,  8 },
    { "1PF10", 10, 16, 1, 10 },
    { "1PF12", 12, 16, 1, 12 },
    { "1PF14", 14, 16, 1, 14 },
    { "2PF8",  16, 16, 2,  8 },
    { "2PF10", 20, 32, 2, 10 },
    { "2PF12", 24, 32, 2, 12 },
    { "2PF14", 28, 32, 2, 14 },
    { "3PF8",  24, 32, 3,  8 },
    { "3PF10", 30, 32, 3, 10 },
    { "3PF12", 36, 64, 3, 12 },
    { "3PF14", 42, 64, 3, 14 },
    { "D32",   32, 32, 1, 32 },
    { "D48",   48, 64, 1, 48 },
    { "D64",   64, 64, 1, 64 },
    { "4PF8",  32, 32, 4, 8 },    
    { "4PF10", 40, 64, 4, 10 },
    { "4PF12", 48, 64, 4, 12 },
    { "4PF14", 56, 64, 4, 14 },
};

int dataflow_cvdr_wr_cfg_vp(cfg_tab_cvdr_t* p_cvdr_cfg, cvdr_opt_fmt_t* cfmt,
        unsigned int line_stride, unsigned int isp_clk, pix_format_e format);
int dataflow_cvdr_rd_cfg_vp(cfg_tab_cvdr_t* p_cvdr_cfg, cvdr_opt_fmt_t* cfmt, unsigned int line_stride, unsigned int isp_clk, pix_format_e format);
int dataflow_cvdr_cfg_d32(cfg_tab_cvdr_t* p_cvdr_cfg, cvdr_opt_fmt_t* cfmt,
    unsigned int total_bytes);
int dataflow_cvdr_wr_cfg_vp_fbcyuv(cfg_tab_cvdr_t* p_cvdr_cfg, cvdr_opt_fmt_t fmt,
        unsigned int current_stripe_num, unsigned int full_width);

int dataflow_cvdr_rd_cfg_vp_fbdyuv(cfg_tab_cvdr_t* p_cvdr_cfg, cvdr_opt_fmt_t fmt,
        unsigned int current_stripe_num, unsigned int full_width);

int dataflow_cvdr_cfg_cmdlst(void);

int dataflow_cvdr_calc_du_raw_readback(unsigned int id);
#endif /* _CVDR_OPT_H */

/********************************end************************************ */

