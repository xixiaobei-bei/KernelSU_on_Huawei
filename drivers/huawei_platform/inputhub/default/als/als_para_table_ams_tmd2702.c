/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams tmd2702 source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_ams_tmd2702.h"

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

tcs3701_als_para_table tmd2702_als_para_diff_tp_color_table[] = {
	{ ANG, V3, VISI_TPLCD, OTHER,
	  { 50, 1423, -300, 0, 1423, -1713, -13324, 313, 135, 7041, 883, 969,
	    0, 1, 90, -1000, 0, 0, 2776, 12000, 0, 0, -317, 249, 4213, 0, 3948, 1427,
	    1916, 857, 6500, 100 }
	},

	{ ANG, V3, BOE_TPLCD, OTHER,
	  { 50, 2241, -4932, 0, 2241, 675, -216, -14363, 14855, 20137, 13703, 1209,
	    0, 1, 90, -1000, 0, 0, 2776, 12000, 0, 0, -317, 209, 3422, 0, 4425, 1798,
	    2031, 762, 6500, 100 }
	},

	{ ANG, V3, DEFAULT_TPLCD, OTHER,
	  { 0 }
	},

	{ BRQ, V3, BOE_TPLCD, OTHER,
	  { 50, 2241, -4932, 0, 2241, 675, -216, -14363, 14855, 20137, 13703, 1209,
	    0, 1, 90, -1000, 0, 0, 2776, 12000, 0, 0, -317, 209, 3422, 0, 4425, 1798,
	    2031, 762, 6500, 100 }
	},

	{ BRQ, V3, VISI_TPLCD, OTHER,
	  { 50, 1423, -300, 0, 1423, -1713, -13324, 313, 135, 7041, 883, 969,
	    0, 1, 90, -1000, 0, 0, 2776, 12000, 0, 0, -317, 249, 4213, 0, 3948, 1427,
	    1916, 857, 6500, 100 }
	},
	{ BRQ, V3, DEFAULT_TPLCD, OTHER,
	  { 0 }
	},
};

tcs3701_als_para_table *als_get_tmd2702_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(tmd2702_als_para_diff_tp_color_table))
		return NULL;
	return &(tmd2702_als_para_diff_tp_color_table[id]);
}

tcs3701_als_para_table *als_get_tmd2702_first_table(void)
{
	return &(tmd2702_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_tmd2702_table_count(void)
{
	return ARRAY_SIZE(tmd2702_als_para_diff_tp_color_table);
}
