 

#ifndef __CFG_TABLE_CPE_TOP_H__
#define __CFG_TABLE_CPE_TOP_H__

typedef struct _cpe_top_mem_cfg0_t
{
    unsigned int  common_mem_ctrl_sp;
} cpe_top_mem_cfg0_t;

typedef struct _cpe_top_mem_cfg1_t
{
    unsigned int  mcf_pwr_mem_ctrl_sp;
} cpe_top_mem_cfg1_t;

typedef struct _cpe_top_mem_cfg2_t
{
    unsigned int  mfnr_pwr_mem_ctrl_sp;
} cpe_top_mem_cfg2_t;

typedef struct _crop_vpwr_t
{
    unsigned int  vpwr_ihleft;
    unsigned int  vpwr_ihright;
} crop_vpwr_t;

typedef struct _cpe_top_mode_cfg_t
{
    unsigned int  to_use;
    unsigned int  cpe_op_mode;
} cpe_top_mode_cfg_t;

typedef struct _cpe_top_mem_cfg_t
{
    unsigned int  to_use;
    cpe_top_mem_cfg0_t  mem_cfg0;
    cpe_top_mem_cfg1_t  mem_cfg1;
    cpe_top_mem_cfg2_t  mem_cfg2;
} cpe_top_mem_cfg_t;

typedef struct _cpe_top_crop_vpwr_t
{
    unsigned int  to_use;
    crop_vpwr_t  crop_vpwr_0;
    crop_vpwr_t  crop_vpwr_1;
    crop_vpwr_t  crop_vpwr_2;
} cpe_top_crop_vpwr_t;

typedef struct _cpe_top_config_table_t
{
    unsigned int        to_use;
    cpe_top_mode_cfg_t  mode_cfg; 
    cpe_top_mem_cfg_t   mem_cfg;
    cpe_top_crop_vpwr_t crop_vpwr;
} cpe_top_config_table_t;

#endif/*__CFG_TABLE_CPE_TOP_H__*/
