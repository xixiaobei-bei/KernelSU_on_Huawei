

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_hardware.h"
#include "oal_schedule.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_HARDWARE_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_hi_timer_reg_stru *reg_timer;

oal_uint32 irq_save_time[MAX_NUM_CORES][OAL_TIMER_IRQ_TYPE_MAX_NUM] = {{0}, {0}};

oal_module_symbol(irq_save_time);
oal_module_symbol(reg_timer);

#endif

/*****************************************************************************
  3 函数实现
*****************************************************************************/






