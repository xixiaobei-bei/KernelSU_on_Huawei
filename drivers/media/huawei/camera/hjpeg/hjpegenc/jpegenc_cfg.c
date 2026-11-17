/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: implement for config qtable, hufftable etc.
 * Author: lixiuhua
 * Create: 2012-12-22
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/iommu.h>
#include <asm/io.h>

#include "hjpeg_common.h"
#include "hjpg_reg_offset.h"
#include "hjpg_reg_offset_field.h"
#include "hjpg_table.h"
#include "cam_log.h"

extern int is_hjpeg_prefetch_bypass(void);

static void hjpeg_hufftable_init_luma_dc(void __iomem *base_addr);
static void hjpeg_hufftable_init_luma_ac(void __iomem *base_addr);
static void hjpeg_hufftable_init_chroma_dc(void __iomem *base_addr);
static void hjpeg_hufftable_init_chroma_ac(void __iomem *base_addr);

static void hjpeg_hufftable_init(void __iomem *base_addr)
{
	/* DC 0 */
	hjpeg_hufftable_init_luma_dc(base_addr);

	/* AC 0 */
	hjpeg_hufftable_init_luma_ac(base_addr);

	/* DC 1 */
	hjpeg_hufftable_init_chroma_dc(base_addr);

	/* AC 1 */
	hjpeg_hufftable_init_chroma_ac(base_addr);
}

static void hjpeg_hufftable_init_luma_dc(void __iomem *base_addr)
{
	unsigned int tmp;
	uint32_t i;
	uint32_t length_bit, length_value, length;

	length_bit = ARRAY_SIZE(luma_dc_bits);
	length_value = ARRAY_SIZE(luma_dc_value);
	length = length_bit + length_value;

	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_TDC0_LEN_REG), length);
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_TABLE_ID_REG), 4); /* 4: set data len */
	for (i = 1; i < length_bit; i = i + 2) { /* 2: bit interval */
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_H, luma_dc_bits[i - 1]);
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_L, luma_dc_bits[i]);
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_JPE_TABLE_DATA_REG), tmp);
	}
	for (i = 1; i < length_value; i += 2) { /* 2: bit interval */
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_H, luma_dc_value[i - 1]);
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_L, luma_dc_value[i]);
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_JPE_TABLE_DATA_REG), tmp);
	}
}

static void hjpeg_hufftable_init_luma_ac(void __iomem *base_addr)
{
	unsigned int tmp;
	uint32_t i;
	uint32_t length_bit, length_value, length;

	length_bit = ARRAY_SIZE(luma_ac_bits);
	length_value = ARRAY_SIZE(luma_ac_value);
	length = length_bit + length_value;
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_TAC0_LEN_REG), length);
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_TABLE_ID_REG), 5); /* 5: set data len */
	for (i = 1; i < length_bit; i = i + 2) { /* 2: bit interval */
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_H, luma_ac_bits[i - 1]);
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_L, luma_ac_bits[i]);
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_JPE_TABLE_DATA_REG), tmp);
	}
	for (i = 1; i < length_value; i = i + 2) { /* 2: bit interval */
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_H, luma_ac_value[i - 1]);
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_L, luma_ac_value[i]);
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_JPE_TABLE_DATA_REG), tmp);
	}
}

static void hjpeg_hufftable_init_chroma_dc(void __iomem *base_addr)
{
	unsigned int tmp;
	uint32_t i;
	uint32_t length_bit, length_value, length;

	length_bit = ARRAY_SIZE(chroma_dc_bits);
	length_value = ARRAY_SIZE(chroma_dc_value);
	length = length_bit + length_value;
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_TDC1_LEN_REG), length);
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_TABLE_ID_REG), 6); /* 6: set data len */
	for (i = 1; i < length_bit; i = i + 2) { /* 2: bit interval */
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_H, chroma_dc_bits[i - 1]);
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_L, chroma_dc_bits[i]);
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_JPE_TABLE_DATA_REG), tmp);
	}
	for (i = 1; i < length_value; i = i + 2) { /* 2: bit interval */
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_H,
			chroma_dc_value[i - 1]);
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_L, chroma_dc_value[i]);
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_JPE_TABLE_DATA_REG), tmp);
	}
}

static void hjpeg_hufftable_init_chroma_ac(void __iomem *base_addr)
{
	unsigned int tmp;
	uint32_t i;
	uint32_t length_bit, length_value, length;

	length_bit = ARRAY_SIZE(chroma_ac_bits);
	length_value = ARRAY_SIZE(chroma_ac_value);
	length = length_bit + length_value;
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_TAC1_LEN_REG), length);
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_TABLE_ID_REG), 7); /* 7: set data len */
	for (i = 1; i < length_bit; i = i + 2) { /* 2: bit interval */
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_H, chroma_ac_bits[i - 1]);
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_L, chroma_ac_bits[i]);
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_JPE_TABLE_DATA_REG), tmp);
	}
	for (i = 1; i < length_value; i = i + 2) { /* 2: bit interval */
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_H,
			chroma_ac_value[i - 1]);
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_L, chroma_ac_value[i]);
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_JPE_TABLE_DATA_REG), tmp);
	}
}

static void hjpeg_qtable_init(void __iomem *base_addr)
{
	int length;
	int i;
	unsigned int tmp;

	/* q-table 0 */
	length = ARRAY_SIZE(luma_qtable1);
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_TABLE_ID_REG), 0);
	for (i = 1; i < length; i = i + 2) { /* 2: bit interval */
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_H, luma_qtable1[i - 1]);
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_L, luma_qtable1[i]);
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_JPE_TABLE_DATA_REG), tmp);
	}

	/* q-table 1 */
	length = ARRAY_SIZE(chroma_qtable1);
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_TABLE_ID_REG), 1);
	for (i = 1; i < length; i = i + 2) { /* 2: bit interval */
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_H, chroma_qtable1[i - 1]);
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_L, chroma_qtable1[i]);
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_JPE_TABLE_DATA_REG), tmp);
	}

	/* q-table 2 */
	length = ARRAY_SIZE(luma_qtable2);
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_TABLE_ID_REG), 2); /* 2: set data len */
	for (i = 1; i < length; i = i + 2) { /* 2: bit interval */
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_H, luma_qtable2[i - 1]);
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_L, luma_qtable2[i]);
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_JPE_TABLE_DATA_REG), tmp);
	}

	/* q-table 3 */
	length = ARRAY_SIZE(chroma_qtable2);
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_TABLE_ID_REG), 3); /* 3: set data len */
	for (i = 1; i < length; i = i + 2) { /* 2: bit interval */
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_H, chroma_qtable2[i - 1]);
		REG_SET_FIELD(tmp, JPGENC_TABLE_WDATA_L, chroma_qtable2[i]);
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_JPE_TABLE_DATA_REG), tmp);
	}
	/* end */
}

static void hjpeg_prefetch_init(void __iomem *base_addr,
	uint32_t power_controller, bool smmu_bypass)
{
	uint32_t tmp = 0;
	if (is_hjpeg_prefetch_bypass() == PREFETCH_BYPASS_YES) {
		REG_SET_FIELD(tmp, JPGENC_PREFETCH_EN, 0);
		cam_info("%s prefetch bypass", __FUNCTION__);
		return;
	}

	if (!smmu_bypass)
		REG_SET_FIELD(tmp, JPGENC_PREFETCH_EN, 1);
	else
		REG_SET_FIELD(tmp, JPGENC_PREFETCH_EN, 0);

	/* 1210, set field val */
	REG_SET_FIELD(tmp, JPGENC_PREFETCH_DELAY, 1210);
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_PREFETCH_REG), tmp);

	if (power_controller == PC_HISP) {
		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_ID0_Y, 49); /* 49: set field val */
		REG_SET_FIELD(tmp, JPGENC_ID1_Y, 50); /* 50: set field val */
		REG_SET_FIELD(tmp, JPGENC_ID2_Y, 51); /* 51: set field val */
		REG_SET_FIELD(tmp, JPGENC_ID3_Y, 52); /* 52: set field val */
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_PREFETCH_IDY0_REG), tmp);

		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_ID4_Y, 53); /* 53: set field val */
		REG_SET_FIELD(tmp, JPGENC_ID5_Y, 54); /* 54: set field val */
		REG_SET_FIELD(tmp, JPGENC_ID6_Y, 55); /* 55: set field val */
		REG_SET_FIELD(tmp, JPGENC_ID7_Y, 56); /* 56: set field val */
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_PREFETCH_IDY1_REG), tmp);

		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_ID0_UV, 57); /* 57: set field val */
		REG_SET_FIELD(tmp, JPGENC_ID1_UV, 58); /* 58: set field val */
		REG_SET_FIELD(tmp, JPGENC_ID2_UV, 59); /* 59: set field val */
		REG_SET_FIELD(tmp, JPGENC_ID3_UV, 60); /* 60: set field val */
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_PREFETCH_IDUV0_REG), tmp);

		tmp = 0;
		REG_SET_FIELD(tmp, JPGENC_ID8_Y, 61); /* 61: set field val */
		REG_SET_FIELD(tmp, JPGENC_ID4_UV, 62); /* 62: set field val */
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_PREFETCH_IDUVY_REG), tmp);
	}
}

static void hjpeg_rstmarker_init(void __iomem *base_addr)
{
	set_reg_val((void __iomem*)((char*)base_addr +
		JPGENC_JPE_RESTART_INTERVAL_REG), JPGENC_RESTART_INTERVAL);
}

static void hjpeg_synccfg_init(void __iomem *base_addr)
{
	uint32_t tmp = 0;

	REG_SET_FIELD(tmp, JPGENC_SOURCE, 1);
	REG_SET_FIELD(tmp, JPGENC_SRAM_NOOPT, 1);
	set_reg_val((void __iomem*)((char*)base_addr + JPGENC_SYNCCFG_REG), tmp);
}

void hjpeg_init_hw_param(void __iomem *base_addr, uint32_t power_controller,
	bool smmu_bypass)
{
	cam_info("%s init jpeg table param", __FUNCTION__);
	if (!base_addr)
		return;
	hjpeg_hufftable_init(base_addr);

	hjpeg_qtable_init(base_addr);

	hjpeg_prefetch_init(base_addr, power_controller, smmu_bypass);

	hjpeg_rstmarker_init(base_addr);

	hjpeg_synccfg_init(base_addr);
}