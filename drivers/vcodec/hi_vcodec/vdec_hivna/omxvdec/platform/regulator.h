

#ifndef __VDEC_REGULATOR_H__
#define __VDEC_REGULATOR_H__

#include "hi_type.h"
#include "vfmw_dts.h"

#define READ_CLOCK_RATE_INDEX_ONE    1
#define READ_CLOCK_RATE_INDEX_TWO    2
#define READ_CLOCK_RATE_INDEX_THREE  3

#define VDEC_TRQ_NUM_PROT            323
#define VDEC_TRQ_NUM_SAFE            324

typedef enum {
#ifdef LOWER_FREQUENCY_SUPPORT
	VDEC_CLK_RATE_LOWER = 0,
#endif
	VDEC_CLK_RATE_LOW,
	VDEC_CLK_RATE_NORMAL,
	VDEC_CLK_RATE_HIGH,
	VDEC_CLK_RATE_HEIF,
	VDEC_CLK_RATE_BUTT,
} clk_rate_e;

#ifdef PLATFORM_KIRIN970
static const struct of_device_id Hisi_Vdec_Match_Table[] = {
	{.compatible = "hisi,kirin970-vdec", },
	{ }
};
#endif

#ifdef PLATFORM_HIVCODECV210
static const struct of_device_id Hisi_Vdec_Match_Table[] = {
	{.compatible = "hisi,HiVCodecV210-vdec", },
	{ }
};
#endif

#ifdef PLATFORM_HIVCODECV300
static const struct of_device_id Hisi_Vdec_Match_Table[] = {
	{.compatible = "hisi,HiVCodecV300-vdec", },
	{ }
};
#endif

#ifdef PLATFORM_HIVCODECV310
static const struct of_device_id Hisi_Vdec_Match_Table[] = {
	{.compatible = "hisi,HiVCodecV310-vdec", },
	{}
};
#endif

#ifdef PLATFORM_HIVCODECV500
static const struct of_device_id Hisi_Vdec_Match_Table[] = {
	{.compatible = "hisi,HiVCodecV500-vdec", },
	{ }
};
#endif

hi_s32  vdec_regulator_probe(struct device *dev);
hi_s32  vdec_regulator_remove(struct device *dev);
hi_s32  vdec_regulator_enable(void);
hi_s32  vdec_regulator_disable(void);
hi_void vdec_regulator_get_clk_rate(clk_rate_e *p_clk_rate);
hi_s32  vdec_regulator_set_clk_rate(clk_rate_e e_clk_rate);

#endif
