/*
 * Copyright (c) 2016 Linaro Limited.
 * Copyright (c) 2014-2016 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef HISI_DPE_H
#define HISI_DPE_H

#include <linux/delay.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/bug.h>
#include <linux/iommu.h>

#include <linux/spi/spi.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
#include <linux/ion.h>
#endif
#include <linux/hisi/hisi_ion.h>
#include <linux/gpio.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>

#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/pinctrl/consumer.h>
#include <linux/file.h>
#include <linux/dma-buf.h>
#include <linux/genalloc.h>
#include <linux/hisi-iommu.h>
#include <linux/clk.h>
#include <linux/atomic.h>

#include "hisi_defs.h"

#define CONFIG_DSS_LP_USED

enum dss_ovl_idx {
	DSS_OVL0 = 0,
	DSS_OVL1,
	DSS_OVL_IDX_MAX,
};

#define DSS_WCH_MAX  (2)

#include <soc_dss_interface.h>
#ifdef CONFIG_HISI_DSS_V600
#include <soc_media1_crg_interface.h>
#include <soc_actrl_interface.h>
#include <soc_pmctrl_interface.h>
#include <soc_smmuv3_tbu_interface.h>
#endif

enum PERI_VOLTAGE_LEVEL {
	PERI_VOLTAGE_LEVEL0 = 0x0,
	PERI_VOLTAGE_LEVEL1 = 0x1,
	PERI_VOLTAGE_LEVEL2 = 0x2,
	PERI_VOLTAGE_LEVEL3 = 0x3,
	PERI_VOLTAGE_LEVEL4 = 0x4,
	PERI_VOLTAGE_LEVEL5 = 0x5,
	PERI_VOLTAGE_MAXLEVEL = 0x7,
};

/*****************************/
enum dss_chn_idx {
	DSS_RCHN_NONE = -1,
	DSS_RCHN_D2 = 0,
	DSS_RCHN_D3,
	DSS_RCHN_V0,
	DSS_RCHN_G0,
	DSS_RCHN_V1,
	DSS_RCHN_G1,
	DSS_RCHN_D0,
	DSS_RCHN_D1,
	DSS_ONLINE_CHN_MAX,

	DSS_WCHN_W0 = DSS_ONLINE_CHN_MAX,
	DSS_WCHN_W1,

	DSS_CHN_MAX,

	DSS_RCHN_V2 = DSS_CHN_MAX,  /* for copybit, only supported in chicago */
	DSS_WCHN_W2,

	DSS_COPYBIT_MAX,
};

#define PRIMARY_CH DSS_RCHN_V0  /* primary plane for primary screen */
#define EXTERNAL_CH DSS_RCHN_G0 /* primary plane for external screen */

struct dss_rect {
	s32 x;
	s32 y;
	s32 w;
	s32 h;
};

struct dss_rect_ltrb {
	s32 left;
	s32 top;
	s32 right;
	s32 bottom;
};

struct dss_img {
	uint32_t format;
	uint32_t width;
	uint32_t height;
	uint32_t bpp; /* bytes per pixel */
	uint32_t buf_size;
	uint32_t stride;
	uint32_t stride_plane1;
	uint32_t stride_plane2;
	uint64_t phy_addr;
	uint64_t vir_addr;
	uint32_t offset_plane1;
	uint32_t offset_plane2;

	uint64_t afbc_header_addr;
	uint64_t afbc_payload_addr;
	uint32_t afbc_header_stride;
	uint32_t afbc_payload_stride;
	uint32_t afbc_scramble_mode;
	uint32_t mmbuf_base;
	uint32_t mmbuf_size;

	uint32_t mmu_enable;
	uint32_t csc_mode;
	uint32_t secure_mode;
	int32_t shared_fd;
	uint32_t reserved0;
};

struct drm_dss_layer {
	struct dss_img img;
	struct dss_rect src_rect;
	struct dss_rect src_rect_mask;
	struct dss_rect dst_rect;
	uint32_t transform;
	int32_t blending;
	uint32_t glb_alpha;
	uint32_t color; /* background color or dim color */
	int32_t layer_idx;
	int32_t chn_idx;
	uint32_t need_cap;
	int32_t acquire_fence;
};

enum hisi_dss_format {
	HISI_DSS_FORMAT_RGB565 = 0,
	HISI_DSS_FORMAT_RGBX4444,
	HISI_DSS_FORMAT_RGBA4444,
	HISI_DSS_FORMAT_RGBX5551,
	HISI_DSS_FORMAT_RGBA5551,
	HISI_DSS_FORMAT_RGBX8888,
	HISI_DSS_FORMAT_RGBA8888,

	HISI_DSS_FORMAT_BGR565,
	HISI_DSS_FORMAT_BGRX4444,
	HISI_DSS_FORMAT_BGRA4444,
	HISI_DSS_FORMAT_BGRX5551,
	HISI_DSS_FORMAT_BGRA5551,
	HISI_DSS_FORMAT_BGRX8888,
	HISI_DSS_FORMAT_BGRA8888,

	HISI_DSS_FORMAT_YUV422_I,

	/* YUV Semi-planar */
	HISI_DSS_FORMAT_YCBCR422_SP, /* NV16 */
	HISI_DSS_FORMAT_YCRCB422_SP,
	HISI_DSS_FORMAT_YCBCR420_SP,
	HISI_DSS_FORMAT_YCRCB420_SP, /* NV21*/

	/* YUV Planar */
	HISI_DSS_FORMAT_YCBCR422_P,
	HISI_DSS_FORMAT_YCRCB422_P,
	HISI_DSS_FORMAT_YCBCR420_P,
	HISI_DSS_FORMAT_YCRCB420_P, /* HISI_FORMAT_YV12 */

	/* YUV Package */
	HISI_DSS_FORMAT_YUYV422_PKG,
	HISI_DSS_FORMAT_UYVY422_PKG,
	HISI_DSS_FORMAT_YVYU422_PKG,
	HISI_DSS_FORMAT_VYUY422_PKG,

	/* 10bit */
	HISI_DSS_FORMAT_RGBA1010102,
	HISI_DSS_FORMAT_BGRA1010102,
	HISI_DSS_FORMAT_Y410_10BIT,
	HISI_DSS_FORMAT_YUV422_10BIT,

	HISI_DSS_FORMAT_YCRCB420_SP_10BIT,
	HISI_DSS_FORMAT_YCRCB422_SP_10BIT,
	HISI_DSS_FORMAT_YCRCB420_P_10BIT,
	HISI_DSS_FORMAT_YCRCB422_P_10BIT,

	HISI_DSS_FORMAT_YCBCR420_SP_10BIT,
	HISI_DSS_FORMAT_YCBCR422_SP_10BIT,
	HISI_DSS_FORMAT_YCBCR420_P_10BIT,
	HISI_DSS_FORMAT_YCBCR422_P_10BIT,

	HISI_DSS_FORMAT_MAX,

	HISI_DSS_FORMAT_UNSUPPORT = 800,
};

/* dss capability priority description */
#define CAP_HFBCD           BIT(15)
#define CAP_HFBCE           BIT(14)
#define CAP_1D_SHARPNESS    BIT(13)
#define CAP_2D_SHARPNESS    BIT(12)
#define CAP_TILE            BIT(11)
#define CAP_AFBCD           BIT(10)
#define CAP_AFBCE           BIT(9)
#define CAP_YUV_DEINTERLACE BIT(8)
#define CAP_YUV_PLANAR      BIT(7)
#define CAP_YUV_SEMI_PLANAR BIT(6)
#define CAP_YUV_PACKAGE     BIT(5)
#define CAP_SCL             BIT(4)
#define CAP_ROT             BIT(3)
#define CAP_PURE_COLOR      BIT(2)
#define CAP_DIM             BIT(1)
#define CAP_BASE            BIT(0)

#define REGULATOR_PDP_NAME    "regulator_dsssubsys"
#define REGULATOR_MMBUF       "regulator_mmbuf"
#define REGULATOR_MEDIA_NAME  "regulator_media_subsys"

#ifdef CONFIG_DRM_SMMU_V3
#define REGULATOR_SMMU_TCU_NAME "regulator_smmu_tcu"
#else /* mmbuf don't be used, stub smmuv3 regulator */
#define REGULATOR_SMMU_TCU_NAME "regulator_mmbuf"
#endif

enum DPE_REGULATER_TYPE {
	DPE_REGULATOR_PDP,
	DPE_REGULATOR_MMBUF,
	DPE_REGULATOR_MEDIA,
#ifdef CONFIG_HISI_DSS_V600
	DPE_REGULATOR_SMMU,
#endif
	DPE_REGULATER_NUM,
};

enum DPE_CLK_TYPE {
	DPE_AXI_CLK,
	DPE_PCLK_CLK,
	DPE_PRI_CLK,
	DPE_PXL0_CLK,
	DPE_PXL1_CLK,
	DPE_MMBUF_CLK,
	DPE_CLK_NUM,
};

#define META_DATA_SIZE 1024
struct lcd_dirty_region_info {
	int left_align;
	int right_align;
	int top_align;
	int bottom_align;

	int w_align;
	int h_align;
	int w_min;
	int h_min;

	int top_start;
	int bottom_start;
};

struct hiace_alg_parameter {
	/* paramters to avoid interference of black/white edge */
	int i_global_hist_black_pos;
	int i_global_hist_white_pos;
	int i_global_hist_black_weight;
	int i_global_hist_white_weight;
	int i_global_hist_zero_cut_ratio;
	int i_global_hist_slope_cut_ratio;

	/* Photo metadata */
	char classifieresult[META_DATA_SIZE];
	int i_result_len;

	/* function enable/disable switch */
	int i_do_lce;
	int i_do_sre;
	int i_do_aplc;

	/* minimum ambient light to enable SRE */
	int i_lasensor_sre_on_th;

	int iwidth;
	int iheight;
	int bitwidth;
	int imode; /* Image(1) or Video(0) mode */
	int ilevel; /* Video(0), gallery(1) ... */
	int ilhist_sft;

	int imaxlcdluminance;
	int iminlcdluminance;
	int imaxbacklight;
	int iminbacklight;
	int iambientlight;
	int ibacklight;
	long itimestamp; /* Timestamp of frame in millisecond */

	/* path of xml file */
	char chcfgname[512];
};

struct dss_vote_cmd {
	uint64_t dss_pri_clk_rate;
	uint64_t dss_axi_clk_rate;
	uint64_t dss_pclk_dss_rate;
	uint64_t dss_pclk_pctrl_rate;
	uint64_t dss_mmbuf_rate;
	uint32_t dss_voltage_level;
	uint32_t reserved;
};

struct hisi_vsync {
	ktime_t vsync_timestamp;
	ktime_t vsync_timestamp_prev;
	ktime_t vactive_timestamp;
};

struct ce_algorithm_parameter {
	int idiffmaxth;
	int idiffminth;
	int ialphaminth;
	int iflatdiffth;
	int ibindiffmaxth;

	int idarkpixelminth;
	int idarkpixelmaxth;
	int idarkavepixelminth;
	int idarkavepixelmaxth;
	int iwhitepixelth;
	int fweight;
	int fdarkratio;
	int fwhiteratio;

	int idarkpixelth;
	int fdarkslopeminth;
	int fdarkslopemaxth;
	int fdarkratiominth;
	int fdarkratiomaxth;

	int ibrightpixelth;
	int fbrightslopeminth;
	int fbrightslopemaxth;
	int fbrightratiominth;
	int fbrightratiomaxth;

	int izeropos0maxth;
	int izeropos1maxth;

	int idarkfmaxth;
	int idarkfminth;
	int ipos0maxth;
	int ipos0minth;

	int fkeepratio;
};

struct hiace_interface_set {
	int thminv;
	unsigned int *lut;
};

#endif /* __HISI_DPE_H__ */
