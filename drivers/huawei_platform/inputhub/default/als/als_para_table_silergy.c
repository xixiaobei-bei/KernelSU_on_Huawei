/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table silergy source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_silergy.h"

#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "als_tp_color.h"
#include "contexthub_boot.h"
#include "contexthub_route.h"

als_para_normal_table syh399_als_para[] = {
	{ TEL, V3, TCL_TPLCD, SYH399_PARA_SIZE,
	  { 2055, 23010, 244, 120, 300, 7742, 10000, 2359, 4234, 9004, 804, 10000, 100, 600, 230, 25 }
	},

	{ TEL, V3, TM_TPLCD, SYH399_PARA_SIZE,
	  { 2189, 24089, 185, 120, 300, 7433, 10000, 3357, 3796, 8450, 768, 10000, 100, 600, 230, 25 }
	},

	{ TEL, V3, BOE_TPLCD, SYH399_PARA_SIZE,
	  { 2055, 23010, 244, 120, 300, 7742, 10000, 2359, 4234, 9004, 804, 10000, 100, 600, 230, 25 }
	},

	{ TEL, V3, INX_TPLCD, SYH399_PARA_SIZE,
	  { 2055, 23010, 244, 120, 300, 7742, 10000, 2359, 4234, 9004, 804, 10000, 100, 600, 230, 25 }
	},

	{ TEL, V3, BOE_TPLCD2, SYH399_PARA_SIZE,
	  { 2055, 23010, 244, 120, 300, 7742, 10000, 2359, 4234, 9004, 804, 10000, 100, 600, 230, 25 }
	},

	{ TEL, V3, TS_PANEL_UNKNOWN, SYH399_PARA_SIZE,
	  { 2055, 23010, 244, 120, 300, 7742, 10000, 2359, 4234, 9004, 804, 10000, 100, 600, 230, 25 }
	},

	{ JSC, V3, TS_PANEL_UNKNOWN, SYH399_PARA_SIZE,
	  { 2055, 23010, 244, 120, 300, 7742, 10000, 2359, 4234, 9004, 804, 10000, 100, 600, 230, 25 }
	},
};

als_para_normal_table *als_get_syh399_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(syh399_als_para))
		return NULL;
	return &(syh399_als_para[id]);
}

als_para_normal_table *als_get_syh399_first_table(void)
{
	return &(syh399_als_para[0]);
}

uint32_t als_get_syh399_table_count(void)
{
	return ARRAY_SIZE(syh399_als_para);
}
