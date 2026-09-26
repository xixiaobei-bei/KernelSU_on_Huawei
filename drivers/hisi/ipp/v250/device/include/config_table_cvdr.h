

#ifndef __CONFIG_TABLE_CVDR_CS_H_INCLUDED__
#define __CONFIG_TABLE_CVDR_CS_H_INCLUDED__

#include "cvdr.h"

typedef enum _cvdr_prefetch_mode_e {
	OLD_CVDR_PREFETCH_EN,
	OLD_CVDR_PREFETCH_BYPASS,
} cvdr_prefetch_mode_e;

typedef enum _pix_type_e {
	STD_PIX = 0,
	EXP_PIX = 1,
} pix_type_e; // expanded or not

typedef enum _cvdr_pix_fmt_e {
	DF_1PF8  = 0,
	DF_2PF8  = 1,
	DF_3PF8  = 2,
	DF_4PF8  = 3,
	DF_1PF10 = 4,
	DF_2PF10 = 5,
	DF_3PF10 = 6,
	DF_4PF10 = 7,
	DF_1PF12 = 8,
	DF_2PF12 = 9,
	DF_3PF12 = 10,
	DF_4PF12 = 11,
	DF_1PF14 = 12,
	DF_2PF14 = 13,
	DF_3PF14 = 14,
	DF_4PF14 = 15,
	DF_D32   = 28,
	DF_D48   = 29,
	DF_D64   = 30,
	DF_D128  = 31,
	DF_FMT_INVALID,
} cvdr_pix_fmt_e;

typedef struct _cvdr_bw_cfg_t {
	unsigned char       bw_limiter0;
	unsigned char       bw_limiter1;
	unsigned char       bw_limiter2;
	unsigned char       bw_limiter3;
	unsigned char       bw_limiter_reload;
} cvdr_bw_cfg_t;

typedef struct _cvdr_wr_fmt_desc_t {
	unsigned int        fs_addr;
	unsigned int        last_page;

	cvdr_pix_fmt_e      pix_fmt;
	unsigned char       pix_expan;
	unsigned short      line_stride;
	unsigned short      line_wrap;
	unsigned short      line_shart_wstrb;
	unsigned short      mono_mode;
} cvdr_wr_fmt_desc_t;

typedef struct _cvdr_rd_fmt_desc_t {
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

typedef struct _cvdr_vp_wr_cfg_t {
	unsigned char           to_use;
	unsigned char           id;
	cvdr_wr_fmt_desc_t      fmt;
	cvdr_bw_cfg_t           bw;
} cvdr_vp_wr_cfg_t;

typedef struct _cvdr_vp_rd_cfg_t {
	unsigned char           to_use;
	unsigned char           id;
	cvdr_rd_fmt_desc_t      fmt;
	cvdr_bw_cfg_t           bw;
} cvdr_vp_rd_cfg_t;

typedef struct _cvdr_nr_wr_cfg_t {
	unsigned char       to_use;
	unsigned char       nr_wr_stop_en_du_thr;
	unsigned char       nr_wr_stop_en_flux_ctrl;
	unsigned char       nr_wr_stop_en_pressure;
	unsigned char       nr_wr_stop_ok;
	unsigned char       nr_wr_stop;
	unsigned char       en;
	cvdr_bw_cfg_t       bw;
} cvdr_nr_wr_cfg_t;

typedef struct _cvdr_nr_rd_cfg_t {
	unsigned char       to_use;
	unsigned short      allocated_du;
	unsigned char       nr_rd_stop_en_du_thr;
	unsigned char       nr_rd_stop_en_flux_ctrl;
	unsigned char       nr_rd_stop_en_pressure;
	unsigned char       nr_rd_stop_ok;
	unsigned char       nr_rd_stop;
	unsigned char       en;
	cvdr_bw_cfg_t       bw;
} cvdr_nr_rd_cfg_t;

typedef struct _cvdr_global_config_t {
	unsigned char   to_use;

	unsigned short   axi_wr_du_thr;
	unsigned short   du_thr_rch;
	unsigned short   max_axi_rd_id;
	unsigned short   max_axi_wr_id;
} cvdr_global_config_t;

typedef struct _cvdr_smmu_cfg_t {
	unsigned char   to_use;

	unsigned int   num;
	unsigned int   smr_nscfg;
} cvdr_smmu_cfg_t;

typedef struct _cfg_tab_cvdr_t {
	cvdr_vp_wr_cfg_t     vp_wr_cfg[VP_WR_MAX];
	cvdr_vp_rd_cfg_t     vp_rd_cfg[VP_RD_MAX];
	cvdr_nr_wr_cfg_t     nr_wr_cfg[NR_WR_MAX];
	cvdr_nr_rd_cfg_t     nr_rd_cfg[NR_RD_MAX];
} cfg_tab_cvdr_t;

#endif /*__CONFIG_TABLE_CVDR_CS_H_INCLUDED__*/

/********************************** END **********************************/
