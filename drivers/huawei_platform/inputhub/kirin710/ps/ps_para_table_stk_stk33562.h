
#ifndef __PS_PARA_TABLE_AMS_STK33562_H__
#define __PS_PARA_TABLE_AMS_STK33562_H__

#include "sensor_config.h"

enum {
	STK33562_PS_PARA_PPULSES_IDX = 0,
	STK33562_PS_PARA_BINSRCH_TARGET_IDX,
	STK33562_PS_PARA_THRESHOLD_L_IDX,
	STK33562_PS_PARA_THRESHOLD_H_IDX,
	STK33562_PS_PARA_BUTT,
};

stk33562_ps_para_table *ps_get_stk33562_table_by_id(uint32_t id);
stk33562_ps_para_table *ps_get_stk33562_first_table(void);
uint32_t ps_get_stk33562_table_count(void);

#endif
