
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "tp_color.h"
#include "contexthub_boot.h"
#include "contexthub_route.h"
#include "ps_para_table_stk_stk33562.h"

static stk33562_ps_para_table stk33562_ps_para_diff_tp_color_table[] = {
	/* tp_color reserved for future use */
	/*
	 *  stk stk33562: Extend-Data Format:
	 *  {Pwave, Pwindow, oily_max_near_pdata, max_oily_add_pdata}
	 */
	{EVE, V4, TS_PANEL_TIANMA, 0, {103, 172, 4500, 750}},
	{EVE, V4, TS_PANEL_SKYWORTH, 0, {118, 191, 6000, 750}},
};

stk33562_ps_para_table *ps_get_stk33562_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(stk33562_ps_para_diff_tp_color_table))
		return NULL;
	return &(stk33562_ps_para_diff_tp_color_table[id]);
}

stk33562_ps_para_table *ps_get_stk33562_first_table(void)
{
	return &(stk33562_ps_para_diff_tp_color_table[0]);
}

uint32_t ps_get_stk33562_table_count(void)
{
	return ARRAY_SIZE(stk33562_ps_para_diff_tp_color_table);
}
