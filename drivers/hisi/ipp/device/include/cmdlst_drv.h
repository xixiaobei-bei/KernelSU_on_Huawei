/* *****************************************************************
 *   Copyright    Copyright (c) 2015- Hisilicon Technologies CO., Ltd.
 *   File name    cmdlst_drv.h
 *   Description:
 *
 *   Version      1.0
 *   Date         2015-05-17 16:28:35
 *   Author       sunjikun_s00236180
 *******************************************************************/


#ifndef __CMDLST_DRV_H_INCLUDED__
#define __CMDLST_DRV_H_INCLUDED__

#include "ipp.h"
#include "config_table_cmdlst.h"

struct _cmdlst_ops_t;

typedef struct _cmdlst_dev_t
{
    unsigned int base_addr;
    struct _cmdlst_ops_t * ops;
} cmdlst_dev_t;

typedef struct _cmdlst_ops_t
{
    int (*set_config)(cmdlst_dev_t * dev, cmdlst_cfg_t * cfg);
    int (*set_sw_ch)(cmdlst_dev_t * dev, cmdlst_ch_cfg_t * cfg,unsigned int channel_id);
    int (*set_sw_ch_mngr)(cmdlst_dev_t * dev, cmdlst_sw_ch_mngr_cfg_t * cfg,unsigned int channel_id);
    int (*set_vp_rd)(cmdlst_dev_t * dev, cmdlst_vp_rd_cfg_t * cfg);
    int (*do_config)(cmdlst_dev_t * dev, cfg_tab_cmdlst_t * table);
    int (*set_branch)(cmdlst_dev_t * dev, int ch_id);
    int (*get_state)(cmdlst_dev_t * dev, cmdlst_state_t *st);
} cmdlst_ops_t;

extern int cmdlst_get_state(cmdlst_dev_t * dev, cmdlst_state_t *st);
extern int cmdlst_do_config(cmdlst_dev_t * dev, cfg_tab_cmdlst_t * config_table);
int cmdlst_set_vp_rd(cmdlst_dev_t * dev, cmdlst_vp_rd_cfg_t * cfg);

#endif /*__CMDLST_DRV_H_INCLUDED__*/



