 /******************************************************************
 * Copyright    Copyright (c) 2015- Hisilicon Technologies CO., Ltd.
 * File name    config_table_cmdlst.h
 * Description:
 *
 * Version      1.0
 * Date         2015-05-17 16:54:26
 * Author       sunjikun_s00236180
 ******************************************************************/

#ifndef __CONFIG_TABLE_CMDLST_H_INCLUDED__
#define __CONFIG_TABLE_CMDLST_H_INCLUDED__

typedef struct _cmdlst_cfg_t
{
    unsigned char   to_use;
    unsigned int    slowdown_nrt_channel;
    unsigned int    prefetch;
} cmdlst_cfg_t;

typedef struct _cmdlst_ch_cfg_t
{
    unsigned char   to_use;
    unsigned int    nrt_channel;
    unsigned int    active_token_nbr_en;
    unsigned int    active_token_nbr;
} cmdlst_ch_cfg_t;

typedef struct _cmdlst_sw_ch_mngr_cfg_t
{
    unsigned char   to_use;
    unsigned int    sw_link_channel       ; 
    unsigned int    sw_link_token_nbr    ; 
    unsigned int    sw_ressource_request  ; 
    unsigned int    sw_priority           ; 
} cmdlst_sw_ch_mngr_cfg_t;

typedef struct _cmdlst_vp_rd_cfg_t
{
    unsigned char      to_use;
    unsigned int       vp_rd_id;
    unsigned int       rd_addr;
    unsigned int       rd_size;
} cmdlst_vp_rd_cfg_t;

typedef struct _cfg_tab_cmdlst_t
{
    cmdlst_cfg_t                 cfg;
    cmdlst_ch_cfg_t              ch_cfg;
    cmdlst_sw_ch_mngr_cfg_t      sw_ch_mngr_cfg; 
    cmdlst_vp_rd_cfg_t           vp_rd_cfg;
} cfg_tab_cmdlst_t;

typedef struct _cmdlst_state_t
{
    unsigned int ch_id;
    unsigned int ch_state;
    unsigned int last_exec;
} cmdlst_state_t;

#endif /*__CONFIG_TABLE_CMDLST_H_INCLUDED__*/

/********************************** END **********************************/
