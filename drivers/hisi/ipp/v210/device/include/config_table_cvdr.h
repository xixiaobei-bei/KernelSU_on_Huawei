 

#ifndef __CONFIG_TABLE_CVDR_H_INCLUDED__
#define __CONFIG_TABLE_CVDR_H_INCLUDED__

#include "cvdr.h"

typedef enum _cvdr_prefetch_mode_e
{
    OLD_CVDR_PREFETCH_EN,
    OLD_CVDR_PREFETCH_BYPASS,
} cvdr_prefetch_mode_e;

typedef enum _pix_type_e
{
    STD_PIX = 0,
    EXP_PIX = 1,
} pix_type_e; // expanded or not

typedef enum _cvdr_pix_fmt_e
{
    DF_1PF8  = 0,
    DF_1PF10 = 1,
    DF_1PF12 = 2,
    DF_1PF14 = 3,
    DF_2PF8  = 4,
    DF_2PF10 = 5,
    DF_2PF12 = 6,
    DF_2PF14 = 7,
    DF_3PF8  = 8,
    DF_3PF10 = 9,
    DF_3PF12 = 10,
    DF_3PF14 = 11,
    DF_D32   = 12,
    DF_D48   = 13,
    DF_D64   = 14,
    DF_4PF8  = 15,
    DF_FMT_INVALID,
} cvdr_pix_fmt_e;

typedef struct _cvdr_bw_cfg_t
{
    unsigned char       bw_limiter0;
    unsigned char       bw_limiter1;
    unsigned char       bw_limiter2;
    unsigned char       bw_limiter3;
    unsigned char       bw_limiter_reload;
} cvdr_bw_cfg_t;

typedef struct _cvdr_wr_fmt_desc_t
{
    unsigned int        fs_addr;
    unsigned int        last_page;

    cvdr_pix_fmt_e      pix_fmt;
    unsigned char       pix_expan;
    unsigned short      line_stride;
    unsigned short      line_wrap;
} cvdr_wr_fmt_desc_t;

typedef struct _cvdr_rd_fmt_desc_t
{
    unsigned int        fs_addr;
    unsigned int        last_page;
    cvdr_pix_fmt_e      pix_fmt;
    unsigned char       pix_expan;
    unsigned short      allocated_du;
    unsigned short      line_size;
    unsigned short      hblank;
    unsigned short      frame_size;
    unsigned short      vblank;
    unsigned short      line_stride;
    unsigned short      line_wrap;
} cvdr_rd_fmt_desc_t;

typedef struct _cvdr_vp_wr_cfg_t
{
    unsigned char           to_use;
    unsigned char           id;
    cvdr_wr_fmt_desc_t      fmt;
    cvdr_bw_cfg_t           bw;
} cvdr_vp_wr_cfg_t;

typedef struct _cvdr_vp_rd_cfg_t
{
    unsigned char           to_use;
    unsigned char           id;
    cvdr_rd_fmt_desc_t      fmt;
    cvdr_bw_cfg_t           bw;
} cvdr_vp_rd_cfg_t;

typedef struct _cvdr_global_config_t
{
    unsigned char   to_use;

    unsigned short   axi_wr_du_thr;
    unsigned short   du_thr_rch;
    unsigned short   max_axi_rd_id;
    unsigned short   max_axi_wr_id;
} cvdr_global_config_t;

typedef struct _cvdr_smmu_cfg_t
{
    unsigned char   to_use;

    unsigned int   num;
    unsigned int   smr_nscfg;
} cvdr_smmu_cfg_t;

typedef struct _cfg_tab_cvdr_t
{
    cvdr_vp_wr_cfg_t     vp_wr_cfg[WR_MAX];
    cvdr_vp_rd_cfg_t     vp_rd_cfg[RD_MAX];
} cfg_tab_cvdr_t;

#endif /*__CONFIG_TABLE_CVDR_H_INCLUDED__*/

/********************************** END **********************************/
