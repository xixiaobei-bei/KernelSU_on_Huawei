

#ifndef __SLAM_DRV_H_
#define __SLAM_DRV_H_

#include "adapter_common.h"
#include "cfg_table_slam.h"

struct _slam_ops_t;

typedef struct _slam_dev_t
{
    unsigned int        base_addr;
    struct _slam_ops_t * ops;
    struct _cmd_buf_t * cmd_buf;
} slam_dev_t;

typedef struct _slam_ops_t
{
    int (*prepare_cmd)(slam_dev_t * dev, cmd_buf_t * cmd_buf, slam_config_table_t * table);
} slam_ops_t;

int slam_prepare_cmd(slam_dev_t * dev, cmd_buf_t * cmd_buf, slam_config_table_t * table);


#endif /* __SLAM_DRV_H_ */

/*************************************** END *******************************************/


