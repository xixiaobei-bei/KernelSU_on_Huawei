

#ifndef __REORDER_DRV_CS_H_
#define __REORDER_DRV_CS_H_

#include "adapter_common.h"
#include "cfg_table_reorder.h"

struct _reorder_ops_t;

typedef struct _reorder_dev_t {
	unsigned int        base_addr;
	struct _reorder_ops_t *ops;
	struct _cmd_buf_t *cmd_buf;
} reorder_dev_t;

typedef struct _reorder_ops_t {
	int (*prepare_cmd)(reorder_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_reorder_t *table);
} reorder_ops_t;

int reorder_prepare_cmd(reorder_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_reorder_t *table);

#endif /* __REORDER_DRV_CS_H_ */

/*************************************** END *******************************************/


