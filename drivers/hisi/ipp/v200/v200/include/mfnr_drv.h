

#ifndef _MFNR_DRV_H_
#define _MFNR_DRV_H_

#include "adapter_common.h"
#include "cfg_table_mfnr.h"

struct _mcf_ops_t;

typedef struct _mfnr_dev_t
{
    unsigned int        base_addr;
    struct _mfnr_ops_t * ops;
    struct _cmd_buf_t * cmd_buf;
} mfnr_dev_t;

typedef struct _mfnr_ops_t
{
    int (*prepare_cmd)(mfnr_dev_t * dev, cmd_buf_t * cmd_buf, mfnr_config_table_t * table);
} mfnr_ops_t;

int mfnr_prepare_cmd(mfnr_dev_t * dev, cmd_buf_t * cmd_buf, mfnr_config_table_t * table);


#endif /* _MFNR_DRV_H_ */

/*************************************** END *******************************************/


