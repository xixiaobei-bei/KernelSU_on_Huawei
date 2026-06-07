
#ifndef __VENC_REGULATOR_H__
#define __VENC_REGULATOR_H__

#include "hi_type.h"
#include "drv_venc_ioctl.h"
#include "drv_venc_osal.h"

#define MAX_NAME_LEN 20
#define VENC_CLK_RATE         "enc_clk_rate"
#define VENC_CLOCK_NAME       "clk_venc"
#define VENC_REGULATOR_NAME   "ldo_venc"
#define MEDIA_REGULATOR_NAME  "ldo_media"

typedef enum {
	VENC_CORE_0,
	VENC_CORE_1,
	MAX_SUPPORT_CORE_NUM,
} venc_core_id_t;

/* clock */
struct venc_clock {
	struct clk *venc_clk;
	venc_clk_t curr_clk_type;
};

/* regulator */
struct venc_regulator {
	struct regulator *media_regulator;
	struct regulator *venc_regulator[MAX_SUPPORT_CORE_NUM];
};

/* config */
struct venc_config_common {
	HI_U32 core_num;
	HI_U32 fpga_flag;
	HI_U32 qos_mode;
	HI_U32 clk_rate[VENC_CLK_BUTT];
};

struct venc_config_priv {
	HI_U32 reg_base_addr;
	HI_U32 reg_range;
};

struct venc_config {
	struct venc_config_common venc_conf_com;
	struct venc_config_priv venc_conf_priv[MAX_SUPPORT_CORE_NUM];
};

HI_S32 get_dts_config_info(struct platform_device *pdev);
HI_S32 get_regulator_info(struct platform_device *pdev);
HI_S32  venc_regulator_enable(void);
HI_S32  venc_regulator_disable(void);
HI_S32  venc_regulator_select_idle_core(vedu_osal_event_t *event);
HI_S32  venc_regulator_wait_hardware_idle(vedu_osal_event_t *event);
HI_S32  venc_regulator_update(const struct clock_info *clock_info);
HI_S32  venc_regulator_reset(void);
HI_BOOL venc_regulator_is_fpga(void);
HI_U64  venc_get_smmu_ttb(HI_VOID);
struct platform_device *venc_get_device(HI_VOID);
#endif

