

#ifndef _VBK_DRV_CS_H_
#define _VBK_DRV_CS_H_

#include "adapter_common.h"
#include "cfg_table_vbk.h"

struct _mcf_ops_t;

typedef struct _vbk_dev_t {
	unsigned int        base_addr;
	struct _vbk_ops_t *ops;
	struct _cmd_buf_t *cmd_buf;
} vbk_dev_t;

typedef struct _vbk_ops_t {
	int (*prepare_cmd)(vbk_dev_t *dev, cmd_buf_t *cmd_buf, vbk_config_table_t *table);
} vbk_ops_t;

int vbk_prepare_cmd(vbk_dev_t *dev, cmd_buf_t *cmd_buf, vbk_config_table_t *table);


#endif /* _VBK_DRV_CS_H_ */

/*************************************** END *******************************************/


