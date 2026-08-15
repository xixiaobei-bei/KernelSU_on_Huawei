

#ifndef _MCF_DRV_H_INCLUDED__
#define _MCF_DRV_H_INCLUDED__

#include "adapter_common.h"
#include "cfg_table_mcf.h"

struct _mcf_ops_t;

typedef struct _mcf_dev_t
{
    unsigned int           base_addr;
    struct _mcf_ops_t * ops;
    struct _cmd_buf_t * cmd_buf;
} mcf_dev_t;

typedef struct _mcf_ops_t
{
    int (*prepare_cmd)(mcf_dev_t * dev, cmd_buf_t * cmd_buf, mcf_config_table_t * table);
} mcf_ops_t;

int mcf_prepare_cmd(mcf_dev_t * dev, cmd_buf_t * cmd_buf, mcf_config_table_t * table);

#endif /*_MCF_DRV_H_INCLUDED__*/

/***************************************end************************************/



