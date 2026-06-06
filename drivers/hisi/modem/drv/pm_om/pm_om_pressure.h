#ifndef __PM_OM_PRESSURE_H__
#define __PM_OM_PRESSURE_H__
#include <bsp_pm.h>
void pm_press_test_start(unsigned int time);
void pm_press_test_stop(void);
extern int pm_wakeup_ccore(enum debug_wake_type type);
extern int pm_wakeup_nrcore(enum debug_wake_type type);
#endif