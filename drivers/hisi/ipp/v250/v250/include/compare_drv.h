

#ifndef __COMPARE_DRV_CS_H_
#define __COMPARE_DRV_CS_H_

#include "adapter_common.h"
#include "cfg_table_compare.h"

struct _compare_ops_t;

typedef struct _compare_dev_t {
	unsigned int        base_addr;
	struct _compare_ops_t *ops;
	struct _cmd_buf_t *cmd_buf;
} compare_dev_t;

typedef struct _compare_ops_t {
	int (*prepare_cmd)(compare_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_compare_t *table);
} compare_ops_t;

int compare_prepare_cmd(compare_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_compare_t *table);

#endif /* __COMPARE_DRV_CS_H_ */

/*************************************** END *******************************************/


