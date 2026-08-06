

#ifndef __PS_PARA_TABLE_EMIN_MN78911_H__
#define __PS_PARA_TABLE_EMIN_MN78911_H__

#include "sensor_config.h"

enum {
	MN78911_PS_PARA_PPULSES_IDX = 0,
	MN78911_PS_PARA_BINSRCH_TARGET_IDX,
	MN78911_PS_PARA_THRESHOLD_L_IDX,
	MN78911_PS_PARA_THRESHOLD_H_IDX,
    MN78911_PS_PARA_BUTT,
};

mn78911_ps_para_table *ps_get_mn78911_table_by_id(uint32_t id);
mn78911_ps_para_table *ps_get_mn78911_first_table(void);
uint32_t ps_get_mn78911_table_count(void);

#endif //__PS_PARA_TABLE_EMIN_MN78911_H__
