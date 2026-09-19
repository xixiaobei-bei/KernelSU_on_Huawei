#include <bsp_softtimer.h>
#include <bsp_pm.h>
#include <bsp_pm_om.h>

#undef THIS_MODU
#define THIS_MODU mod_pm_om
extern s32 pm_wakeup_ccore(enum debug_wake_type type);
void pm_press_timer_cb(void* p);
struct softtimer_list pm_press_timer =
{
	.name = "press_timer",
	.func = pm_press_timer_cb,
	.wake_type = SOFTTIMER_WAKE,
	.unit_type = TYPE_S,
};
void pm_press_timer_cb(void* p)
{
	pm_wakeup_ccore(PM_WAKEUP_THEN_SLEEP);
	bsp_softtimer_add(&pm_press_timer);
}

void pm_press_test_start(unsigned int time)
{
	int ret = 0;
	pm_press_timer.timeout = time;
	if(pm_press_timer.init_flags == TIMER_INIT_FLAG)
	{
		bsp_softtimer_delete(&pm_press_timer);
		bsp_softtimer_modify(&pm_press_timer,time);
		bsp_softtimer_add(&pm_press_timer);
	}
	else
	{
		ret = bsp_softtimer_create(&pm_press_timer);
		if(ret)
		{
			pmom_pr_err(KERN_ERR"pm press testtimer create failed\n");
			return;
		}
		else
			bsp_softtimer_add(&pm_press_timer);
	}
}
void pm_press_test_stop(void)
{
    if(pm_press_timer.init_flags == TIMER_INIT_FLAG)
	    bsp_softtimer_delete(&pm_press_timer);
}
