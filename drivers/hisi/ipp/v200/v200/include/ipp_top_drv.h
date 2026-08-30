

#ifndef _IPP_TOP_DRV_H_
#define _IPP_TOP_DRV_H_

#include "cfg_table_cpe_top.h"
#include "adapter_common.h"
//#include "config_table.h"

typedef struct _cpe_top_dev_t
{
    unsigned int        base_addr;
    struct _mcf_ops_t * ops;
    struct _cmd_buf_t * cmd_buf;
} cpe_top_dev_t;

typedef struct _cpe_top_ops_t
{
    int (*prepare_cmd)(cpe_top_dev_t * dev, cmd_buf_t * cmd_buf, cpe_top_config_table_t * table);
} cpe_top_ops_t;

int cpe_top_prepare_cmd(cpe_top_dev_t * dev, cmd_buf_t * cmd_buf, cpe_top_config_table_t * table);
extern cpe_top_dev_t g_cpe_top_devs[];
#endif /* _IPP_TOP_DRV_H_ */

/*************************************** END *******************************************/


