/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description:gpu efuse configuration
 * Author : gpu
 * Create : 2019/12/20
 */

#include <mali_kbase.h>
#include "mali_kbase_efuse.h"
#include "mali_kbase_defs.h"

#include <efuse_driver.h>
#include <string.h>

#include <linux/of.h>

/**
 * The function get_partial_pass_info returns 20 bits of information in the BUF array.
 * These information correspond to efUSE bits: 1624~1643, in which 1627~1642 indicate
 * whether the 16 GPU cores are normal or not.
 * Left -> Right, High -> Low.
 * src: buf[0]   buf[1]   buf[2]
 *      43210--- cba98765 -----fed
 * dst: u32
 *      ----------------fedcba9876543210
 */
#define get_gpu_failed_cores(buf) (((u32)buf[0] >> 3) | ((u32)buf[1] << 5) | (((u32)buf[2] & 0x7) << 13))
#define PARTIAL_PASS_INFO_SIZE (3)
enum SOC_SPEC_TYPE {
	PC_CHIP = 0,
	MODEM_LITE_CHIP,
	LITE_CHIP,
	NORMAL_CHIP,
	UNKNOWN_CHIP,
	SOC_SPEC_NUM
};

/**
 *  stack0 tile0 bit[0] ->1631
 *  stack0 tile1 bit[4] ->1637
 *  stack0 tile2 bit[8] ->1632
 *  stack0 tile3 bit[12]->1638
 *  stack1 tile0 bit[1] ->1633
 *  stack1 tile1 bit[5] ->1639
 *  stack1 tile2 bit[9] ->1634
 *  stack1 tile3 bit[13]->1640
 *  stack2 tile0 bit[2] ->1635
 *  stack2 tile1 bit[6] ->1641
 *  stack2 tile2 bit[10]->1636
 *  stack2 tile3 bit[14]->1642
 *  stack3 tile0 bit[3] ->1627
 *  stack3 tile1 bit[7] ->1628
 *  stack3 tile2 bit[11]->1629
 *  stack3 tile3 bit[15] ->1630
 */
static shader_present_struct g_clip_core[] = {
	{ EFUSE_BIT0_FAIL, CLIP_BIT3_CORE, CLIP_BIT3_CORE_PLUS },
	{ EFUSE_BIT1_FAIL, CLIP_BIT7_CORE, CLIP_BIT7_CORE_PLUS },
	{ EFUSE_BIT2_FAIL, CLIP_BIT11_CORE, CLIP_BIT11_CORE_PLUS },
	{ EFUSE_BIT3_FAIL, CLIP_BIT15_CORE, CLIP_BIT15_CORE_PLUS },
	{ EFUSE_BIT4_FAIL, CLIP_BIT0_CORE, CLIP_BIT0_CORE_PLUS },
	{ EFUSE_BIT5_FAIL, CLIP_BIT8_CORE, CLIP_BIT8_CORE_PLUS },
	{ EFUSE_BIT6_FAIL, CLIP_BIT1_CORE, CLIP_BIT1_CORE_PLUS },
	{ EFUSE_BIT7_FAIL, CLIP_BIT9_CORE, CLIP_BIT9_CORE_PLUS },
	{ EFUSE_BIT8_FAIL, CLIP_BIT2_CORE, CLIP_BIT2_CORE_PLUS },
	{ EFUSE_BIT9_FAIL, CLIP_BIT10_CORE, CLIP_BIT10_CORE_PLUS },
	{ EFUSE_BIT10_FAIL, CLIP_BIT4_CORE, CLIP_BIT4_CORE_PLUS },
	{ EFUSE_BIT11_FAIL, CLIP_BIT12_CORE, CLIP_BIT12_CORE_PLUS },
	{ EFUSE_BIT12_FAIL, CLIP_BIT5_CORE, CLIP_BIT5_CORE_PLUS },
	{ EFUSE_BIT13_FAIL, CLIP_BIT13_CORE, CLIP_BIT13_CORE_PLUS },
	{ EFUSE_BIT14_FAIL, CLIP_BIT6_CORE, CLIP_BIT6_CORE_PLUS },
	{ EFUSE_BIT15_FAIL, CLIP_BIT14_CORE, CLIP_BIT14_CORE_PLUS }
};

static struct device* s_kbdev = NULL;
u32 get_efuse_fail_core_num(u32 efuse_bit)
{
	u32 cnt = 0;
	while (efuse_bit) {
		cnt += efuse_bit & 1;
		efuse_bit >>= 1;
	}
	return cnt;
}

int clip_2_gpu_cores(u32 *core_mask, u32 abnormal_core_cnt, u32 efuse_abnormal_core_mask)
{
	int ret = OK;
	u32 i;
	/** 1. if more than 2 abnormal cores, return error;
	 *  2. if 2 abnormal cores, clip the 2 abnormal cores;
	 *  3. if 1 abnormal core, clip the 1 abnormal core and 1 normal core;
	 *  4. if 0 abnormal cores, clip two normal cores.
	 */
	if (abnormal_core_cnt > 2u) {
		dev_err(s_kbdev, "more than 2 abnormal shader cores, abnormal_core_cnt=[%d].\n",
			abnormal_core_cnt);
		ret = ERROR;
	} else if (abnormal_core_cnt == 2u) {
		for (i = 0u; i < sizeof(g_clip_core) / sizeof(shader_present_struct); ++i) {
			if (g_clip_core[i].efuse_fail_bit & efuse_abnormal_core_mask)
				*core_mask = *core_mask & g_clip_core[i].clip_core_mask;
		}
	} else if (abnormal_core_cnt == 1u) {
		for (i = 0u; i < sizeof(g_clip_core) / sizeof(shader_present_struct); ++i) {
			if (g_clip_core[i].efuse_fail_bit & efuse_abnormal_core_mask) {
				*core_mask = *core_mask & g_clip_core[i].clip_core_mask_plus;
				break;
			}
		}
	} else {
		*core_mask = LITE_CHIP_CORE_MASK;
	}

	return ret;
}

int get_shader_present_mask(u32* shader_present_mask, u32 efuse_abnormal_core_mask,
		enum SOC_SPEC_TYPE soc_spec_type)
{
	int ret = OK;
	u32 core_mask = 0xFFFF;
	u32 abnormal_core_cnt;

	abnormal_core_cnt = get_efuse_fail_core_num(efuse_abnormal_core_mask);
	switch (soc_spec_type) {
	case UNKNOWN_CHIP:
		ret = ERROR;
		break;
	case NORMAL_CHIP:
		if (abnormal_core_cnt != 0)
			ret = ERROR;
		break;
	case PC_CHIP:
		ret = clip_2_gpu_cores(&core_mask, abnormal_core_cnt, efuse_abnormal_core_mask);
		break;
	case MODEM_LITE_CHIP:
		if (abnormal_core_cnt != 0)
			ret = ERROR;
		break;
	case LITE_CHIP:
		ret = clip_2_gpu_cores(&core_mask, abnormal_core_cnt, efuse_abnormal_core_mask);
		break;
	default:
		ret = ERROR;
	}

	*shader_present_mask = core_mask;
	return ret;
}

enum SOC_SPEC_TYPE convert_to_enum(const char *soc_spec_str)
{
	if (strncmp(soc_spec_str, "normal", strlen("normal")) == 0) {
		return NORMAL_CHIP;
	} else if (strncmp(soc_spec_str, "lite", strlen("lite")) == 0) {
		return LITE_CHIP;
	} else if (strncmp(soc_spec_str, "wifi-only", strlen("wifi-only")) == 0) {
		return MODEM_LITE_CHIP;
	} else if (strncmp(soc_spec_str, "PC", strlen("PC")) == 0) {
		return PC_CHIP;
	} else if (strncmp(soc_spec_str, "unknown", strlen("unknown")) == 0) {
		return UNKNOWN_CHIP;
	} else {
		return UNKNOWN_CHIP; // for error soc_spec_str
	}
}

int get_gpu_efuse_cfg(struct kbase_device * const kbdev, u32 *shader_present_mask, u32 *lite_chip_mask)
{
	int ret;
	u32 efuse_abnormal_core_mask = 0xffff;
	enum SOC_SPEC_TYPE soc_spc_type;
	const char *soc_spec_str = NULL;
	struct device_node *np = NULL;
	u8 buf[PARTIAL_PASS_INFO_SIZE] = {0};

	s_kbdev = kbdev->dev;

	if (!shader_present_mask) {
		dev_err(s_kbdev, "%s, ptr is null\n", __func__);
		return ERROR;
	}

	ret = get_partial_pass_info(buf, PARTIAL_PASS_INFO_SIZE, EFUSE_TIMEOUT_SECOND);
	if (ret) {
		dev_err(s_kbdev, "%s, get_partial_pass_info failed\n", __func__);
		return ERROR;
	}
	efuse_abnormal_core_mask = get_gpu_failed_cores(buf);
	dev_info(s_kbdev, "gpu efuse_abnormal_core_mask=[0x%x], buf=0x%x,0x%x,0x%x!\n", efuse_abnormal_core_mask,
		buf[0], buf[1], buf[2]);

	/* read soc spec from dts*/
	np = of_find_compatible_node(NULL, NULL, "hisilicon, soc_spec");
	if (np) {
		ret = of_property_read_string(np, "soc_spec_set", &soc_spec_str);
		if (ret) {
			dev_err(s_kbdev, "Failed to get sco spec set!\n");
			return ERROR;
		}
	} else {
		dev_err(s_kbdev, "not find device node hisilicon, soc_spec!\n");
		return ERROR;
	}
	dev_info(s_kbdev, "gpu soc_spec_set=[%s]!\n", soc_spec_str);

	soc_spc_type = convert_to_enum(soc_spec_str);
	ret = get_shader_present_mask(shader_present_mask, efuse_abnormal_core_mask, soc_spc_type);
	if (ret) {
		dev_err(s_kbdev, "%s, get_shader_present_mask failed, return soc type=[%s], efuse_abnormal_core_mask=0x%x.\n",
			 __func__, soc_spec_str, efuse_abnormal_core_mask);
		return ERROR;
	}

	*lite_chip_mask = (soc_spc_type == LITE_CHIP || soc_spc_type == MODEM_LITE_CHIP) ? 1 : 0;

	dev_info(s_kbdev, "return soc type=[%s], gpu core_mask:0x%x, lite chip mask is %d\n",
		soc_spec_str, *shader_present_mask, *lite_chip_mask);

	return ret;
}
