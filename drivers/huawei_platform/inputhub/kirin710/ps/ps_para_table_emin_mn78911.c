
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
#include "ps_para_table_emin_mn78911.h"

static mn78911_ps_para_table mn78911_ps_para_diff_tp_color_table[] = {
	/* tp_color reserved for future use */
	/*
	 *  EMIN MN78911: Extend-Data Format:
	 *  {
	 * mn_sensor.ps.integration_time
	 * mn_sensor.ps.ps_avg
	 * mn_sensor.ps.ps_pulse
	 * mn_sensor.ps.enh_mode
	 * mn_sensor.ps.ps_gain0
	 * mn_sensor.ps.ps_gain1
	 * mn_sensor.ps.ir_drive
	 * Pwave
	 * Pwindow
	 * }
	 */
	{PHONE_TYPE_MGA, V4, TS_PANEL_TIANMA, 0, {4, 6, 4, 0, 1, 1, 1, 170, 307}},
	{PHONE_TYPE_MGA, V4, TS_PANEL_SKYWORTH, 0, {4, 6, 4, 0, 1, 1, 1, 230, 544}},
};

mn78911_ps_para_table *ps_get_mn78911_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(mn78911_ps_para_diff_tp_color_table))
		return NULL;
	return &(mn78911_ps_para_diff_tp_color_table[id]);
}

mn78911_ps_para_table *ps_get_mn78911_first_table(void)
{
	return &(mn78911_ps_para_diff_tp_color_table[0]);
}

uint32_t ps_get_mn78911_table_count(void)
{
	return ARRAY_SIZE(mn78911_ps_para_diff_tp_color_table);
}
