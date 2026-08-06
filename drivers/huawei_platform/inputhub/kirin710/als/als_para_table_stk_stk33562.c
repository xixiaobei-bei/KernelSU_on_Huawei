
#include "als_para_table_stk_stk33562.h"

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

static stk33562_als_para_table stk33562_als_para_diff_tp_color_table[] = {
	/* tp_color reserved for future use */
	/*
	 *    stk33562: Extend-Data Format:
	 *    { als_ratio, g_ratio, g_a_light_ratio, g_a_light_slope, g_day_light_ratio, g_day_light_slope,
	 *    g_cwf_light_slope,  middle_als_data, offset_max, offset_min, middle_g_data, offset_g_max, offset_g_min}
	 */
	{EVE, V4, TS_PANEL_TIANMA, 0,
		{1, 1, 14000, 0.454, 3000, 0.615, 0.12, 15277, 10000, 0, 1605, 10000, 0}
	},
	{EVE, V4, TS_PANEL_SKYWORTH, 0,
		{1, 1, 16000, 0.4026, 2000, 0.604, 0.1137, 17164, 10000, 0, 1578, 10000, 0}
	},
};

stk33562_als_para_table *als_get_stk33562_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(stk33562_als_para_diff_tp_color_table))
		return NULL;
	return &(stk33562_als_para_diff_tp_color_table[id]);
}

stk33562_als_para_table *als_get_stk33562_first_table(void)
{
	return &(stk33562_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_stk33562_table_count(void)
{
	return ARRAY_SIZE(stk33562_als_para_diff_tp_color_table);
}
