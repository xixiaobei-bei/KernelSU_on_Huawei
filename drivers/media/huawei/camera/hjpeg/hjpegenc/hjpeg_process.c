/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: implement for processing jpeg power on /off/encode.
 * Author: lixiuhua
 * Create: 2017-02-28
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

#include <linux/of.h>
#include <linux/videodev2.h>
#include <linux/iommu.h>
#include <linux/platform_device.h>
#include <linux/of_irq.h>
#include <media/v4l2-fh.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <media/huawei/hjpeg_cfg.h>
#include <media/camera/jpeg/jpeg_base.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/rpmsg.h>
#include <linux/ioport.h>
#include <linux/hisi-iommu.h>
#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/pm_wakeup.h>
#include <linux/version.h>

#include "hicam_buf.h"
#include "smmu_cfg.h"
#include "cvdr_cfg.h"
#include "jpegenc_cfg.h"
#include "hjpg_reg_offset.h"
#include "hjpg_reg_offset_field.h"
#include "hjpg_table.h"
#include "hjpeg_common.h"
#include "hjpeg_intf.h"
#include "cam_log.h"
#include "hjpeg_debug.h"

#define i_2_hjpeg_enc(i) container_of(i, hjpeg_base_t, intf)

#define ENCODE_FINISH (1 << 4)
#define IRQ_MERGE_ENCODE_FINISH (1 << 1)
#define IRQ_INOUT_ENCODE_FINISH 1

#define IRQ_OUT_MASK_ENABLE   0x0000001E
#define IRQ_OUT_MASK_DISABLE  0x0000001F
#define IRQ_IN_MASK_ENABLE    0x30
#define IRQ_IN_MASK_DISABLE   0x0
#define IRQ_IN_CLR_VAL        0x30
#define IRQ_OUT_CLR_VAL       0x1F

typedef struct _tag_hjpeg_base {
	struct platform_device    *pdev;
	hjpeg_intf_t              intf;
	char const                *name;
	u32                       irq_no;
	struct semaphore          buff_done;
	hjpeg_hw_ctl_t            hw_ctl; /* for all phyaddr and viraddr */
	struct regulator          *jpeg_supply;
	struct regulator          *media_supply;
	struct clk                *jpegclk[JPEG_CLK_MAX];
	unsigned int              jpegclk_value[JPEG_CLK_MAX];
	unsigned int              jpegclk_low_frequency;
	unsigned int              power_off_frequency;
	atomic_t                  jpeg_on;
	struct iommu_domain       *domain;
	phy_pgd_t                 phy_pgd;
	struct wakeup_source      power_wakelock;
	struct mutex              wake_lock_mutex;
} hjpeg_base_t;

int is_hjpeg_qos_update(void);
int get_hjpeg_iova_update(void);
int get_hjpeg_wr_port_addr_update(void);
int is_pixel_fmt_update(void);
int is_cvdr_cfg_update(void);
int is_hjpeg_prefetch_bypass(void);

static void hjpeg_isr_do_tasklet(unsigned long data);
DECLARE_TASKLET(hjpeg_isr_tasklet, hjpeg_isr_do_tasklet, (unsigned long)0);

static void hjpeg_unmap_baseaddr(void);
static int hjpeg_map_baseaddr(void);

static int hjpeg_get_dts(struct platform_device *pDev);
static int get_phy_pgd_base(void);

static int hjpeg_poweron(hjpeg_base_t *pjpeg_dev);
static int hjpeg_poweroff(hjpeg_base_t *pjpeg_dev);
static int hjpeg_poweron_ippcomm(hjpeg_base_t *pjpeg_dev);
static int hjpeg_poweroff_ippcomm(hjpeg_base_t *pjpeg_dev);

static int hjpeg_setclk_enable(hjpeg_base_t *pjpeg_dev, int idx);
static void hjpeg_setclk_disable(hjpeg_base_t *pjpeg_dev, int idx);
static int hjpeg_clk_ctrl(void __iomem *subctrl1, bool enable);
static void hjpeg_apbbus_timeout_ctrl(void __iomem *subctrl1);
static int hjpeg_encode_process(hjpeg_intf_t *i, void *cfg);
static int hjpeg_power_on(hjpeg_intf_t *i);
static int hjpeg_power_off(hjpeg_intf_t *i);

static void hjpeg_irq_clr(void __iomem *subctrl1);
static void hjpeg_irq_inout_clr(void __iomem *jpegenc, void __iomem *subctl);
static void hjpeg_irq_mask(void __iomem *subctrl1, bool enable);
static void hjpeg_irq_inout_mask(bool enable);

static void hjpeg_encode_finish(void __iomem *jpegenc, void __iomem *subctl);

static int check_buffer_wide_height_stride(jpgenc_config_t *config);
static int check_input_buffer_yuv(jpgenc_config_t *config);
static int check_buffer_quality_outputsize(jpgenc_config_t *config);
static int get_dts_pc_drv_power_prop(struct device *pdev,
	struct device_node *np);
static int get_dts_pc_drv_power_regulator(struct device *pdev);
static int get_dts_pc_drv_power_frequency(struct device_node *np);
static int get_dts_pc_drv_power_clk_name(struct device *pdev,
	struct device_node *np);
static int get_dts_pc_ippcomm_power_prop(struct device_node *np);
static int hjpeg_get_dts_data(struct device *pdev, struct device_node *np);
static int hjpeg_get_dts_prop_data(struct device *pdev);
static int get_dts_cvdr_property_data(struct device_node *np);
static int get_dts_cvdr_stream_id(struct device_node *np);
static void get_dts_cvdr_qos_update(struct device_node *np);
static void get_dts_cvdr_iova_update_version(struct device_node *np);
static void get_dts_cvdr_wr_port_addr_update_version(struct device_node *np);
static void get_dts_cvdr_pixel_format_update(struct device_node *np);
static void get_dts_cvdr_cfg_update(struct device_node *np);
static void get_dts_cvdr_dis_timeout(struct device_node *np);
static void get_dts_cvdr_du_allocate_update(struct device_node *np);
static void get_dts_cvdr_limiter_du_prop(struct device_node *np);
#ifdef SOFT_RESET
static void hjpeg_soft_reset_cfg_reg_sets(void __iomem *temp_base);
#endif
static void set_input_buffer_address(jpgenc_config_t *config,
	unsigned int buf_format, void __iomem *base_addr);
static int get_dts_secadapt_prop_base(struct device_node *np);
static int get_dts_secadapt_prop_sid_ssid(struct device_node *np);
static int get_dts_secadapt_prop_jpgen_swid(struct device_node *np);
static int hjpeg_map_hjpeg_base(struct device_node *np, hjpeg_base_t *phjpeg);
static int hjpeg_map_smmu_base(struct device_node *np, hjpeg_base_t *phjpeg);
static int hjpeg_map_cvdr_base(struct device_node *np, hjpeg_base_t *phjpeg);
static int hjpeg_map_subctrl_baseaddr(struct device_node *np,
	hjpeg_base_t *phjpeg);
static int hjpeg_encode_process_config(jpgenc_config_t *pcfg);
#ifdef SOFT_RESET
static void hjpeg_encode_process_nop0(long *jiff);
static void hjpeg_encode_process_nop2(void);
#endif
static int hjpeg_encode_get_jpeg_size(jpgenc_config_t *pcfg);

static int hjpeg_poweron_enalbe(hjpeg_base_t *pjpeg_dev);
#if (POWER_CTRL_INTERFACE == POWER_CTRL_CFG_REGS)
static int hjpeg_poweron_enalbe_by_cfg_reg(hjpeg_base_t *pjpeg_dev);
#else
static int hjpeg_poweron_enalbe_by_regulator(hjpeg_base_t *pjpeg_dev);
#endif
static int hjpeg_ipp_comm_setclk_enable(hjpeg_base_t *pjpeg_dev);

static hjpeg_vtbl_t s_vtbl_hjpeg = {
	.encode_process = hjpeg_encode_process,
	.power_on = hjpeg_power_on,
	.power_down = hjpeg_power_off,
};

static hjpeg_base_t s_hjpeg = {
	.intf = { .vtbl = &s_vtbl_hjpeg, },
	.name = "hjpeg",
};

static const struct of_device_id s_hjpeg_dt_match[] = {
	{ .compatible = "huawei,hjpeg", .data = &s_hjpeg.intf, },
	{},
};

struct hipp_common_s *s_hjpeg_ipp_comm = NULL;

MODULE_DEVICE_TABLE(of, s_hjpeg_dt_match);

static struct timeval s_timeval_start;
static struct timeval s_timeval_end;
static int g_is_qos_update = 0;
static int g_iova_update_version = 0;
static int g_wr_port_addr_update_version = 0;
static int g_pixel_format_update = 0;
static int g_cvdr_cfg_update = 0;
static int g_cvdr_dis_timeout = 0;
static int g_is_irq_merge = 0;
static int g_clk_ctl_offset = 0;
/* default open prefetch mechanism */
static int g_is_prefetch_bypass = PREFETCH_BYPASS_NO;
static int g_is_du_allocate_update = DU_ALLOCATE_UPDATE_NO;
static int g_is_read_twice = 0;

extern int memset_s(void *dest, size_t destMax, int c, size_t count);

static void hjpeg_isr_do_tasklet(unsigned long data)
{
	up(&s_hjpeg.buff_done);
}

static irqreturn_t hjpeg_irq_handler(int irq, void *dev_id)
{
	(void)irq;
	(void)dev_id;
	hjpeg_encode_finish(s_hjpeg.hw_ctl.jpegenc_viraddr,
		s_hjpeg.hw_ctl.subctrl_viraddr);

	return IRQ_HANDLED;
}

static void calculate_encoding_time(void)
{
	u64 tm_used;

	tm_used = (s_timeval_end.tv_sec - s_timeval_start.tv_sec) *
		MICROSECOND_PER_SECOND + s_timeval_end.tv_usec -
		s_timeval_start.tv_usec;

	cam_debug("%s JPGENC encoding elapse %llu us", __FUNCTION__, tm_used);
}

static bool bypass_smmu(void)
{
	return (s_hjpeg.hw_ctl.smmu_bypass == BYPASS_YES);
}

static void set_rstmarker(void __iomem *base_addr, unsigned int rst)
{
	if (!base_addr)
		return;
	set_reg_val((void __iomem*)(
		(char*)base_addr + JPGENC_JPE_RESTART_INTERVAL_REG), rst);
}

static int __check_buffer_vaild(int share_fd, unsigned int req_addr,
	unsigned int req_size)
{
	int ret;
	struct iommu_format fmt = {0};
	struct device *pdev = NULL;

	if (share_fd < 0) {
		cam_err("invalid ion buffer: fd=%d", share_fd);
		return -1;
	}

	/* remap smmu v3 addr for diff domain */
	if (s_hjpeg.hw_ctl.smmu_type == ST_SMMUV3) {
		pdev = &(s_hjpeg.pdev->dev);
		if (!pdev) {
			cam_err("%s %d deivce is NULL",
				__FUNCTION__, __LINE__);
			return -ENOENT;
		}

		ret = hicam_buf_v3_map_iommu(pdev, share_fd, &fmt,
			s_hjpeg.hw_ctl.support_iova_padding);
		if (ret < 0) {
			cam_err("%s: fail to map iommu", __FUNCTION__);
			return ret;
		}
	} else {
		ret = hicam_buf_map_iommu(share_fd, &fmt);
	}

	if (ret < 0) {
		cam_err("%s: fail to map iommu", __FUNCTION__);
		return ret;
	}

	if (req_addr != fmt.iova || req_size > fmt.size) {
		cam_err("%s: fd:%d, iova:%#llx, size:%#llx",
			__FUNCTION__, share_fd, fmt.iova, fmt.size);
		ret = -ERANGE;
	}

	if (s_hjpeg.hw_ctl.smmu_type == ST_SMMUV3)
		hicam_buf_v3_unmap_iommu(pdev, share_fd, &fmt,
			s_hjpeg.hw_ctl.support_iova_padding);
	else
		hicam_buf_unmap_iommu(share_fd, &fmt);
	cam_debug("%s: fd:%d, iova:%#llx, size:%#llx",
		__FUNCTION__, share_fd, fmt.iova, fmt.size);
	return ret;
}

static int check_buffer_vaild(jpgenc_config_t *config)
{
	unsigned int vaild_input_size;
	unsigned int buf_format = (unsigned int)(config->buffer.format);

	if (__check_buffer_vaild(config->buffer.ion_fd,
		config->buffer.output_buffer,
		config->buffer.output_size) != 0) {
		cam_err("%s:check output buffer fail", __FUNCTION__);
		return -1;
	}
	if ((buf_format & JPGENC_FORMAT_BIT) == JPGENC_FORMAT_YUV422)
		vaild_input_size = config->buffer.width *
			config->buffer.height * 2; /* 2: ratio */
	else
		vaild_input_size = config->buffer.width *
			config->buffer.height * 3 / 2; /* 3,2: ratio */
	/* uv addr has been checked */
	if (__check_buffer_vaild(config->buffer.input_ion_fd,
		config->buffer.input_buffer_y, vaild_input_size) != 0) {
		cam_err("%s:check output buffer fail", __FUNCTION__);
		return -1;
	}

	return 0;
}

/* check the input parameter of hjpeg_encode_process from IOCTL
 * if invalid, return error.
 * called by hjpeg_encode_process
 */
static int check_config(jpgenc_config_t *config)
{
	int ret;
	cam_info("%s enter", __FUNCTION__);
	if (!config) {
		cam_err("%s: config is null %d", __FUNCTION__, __LINE__);
		return -EINVAL;
	}

	ret = check_buffer_wide_height_stride(config);
	if (ret != 0)
		return ret;

	ret = check_input_buffer_yuv(config);
	if (ret != 0)
		return ret;

	ret = check_buffer_quality_outputsize(config);
	if (ret != 0)
		return ret;

	return check_buffer_vaild(config);
}

static int check_buffer_wide_height_stride(jpgenc_config_t *config)
{
	unsigned int buf_format;
	buf_format = (unsigned int)(config->buffer.format);
	if (!CHECK_ALIGN(config->buffer.width, 2) || /* 2: offset */
		(config->buffer.width > 8192)) { /* 8192: the max value */
		cam_err(" width[%d] is invalid", config->buffer.width);
		return -1;
	}

	/* 8192: the max value */
	if (config->buffer.height == 0 || (config->buffer.height > 8192)) {
		cam_err(" height[%d] is invalid", config->buffer.height);
		return -1;
	}

	if ((config->buffer.stride == 0) ||
		!CHECK_ALIGN(config->buffer.stride, 16) || /* 16: offset */
		/* 16: ratio */
		(config->buffer.stride / 16 > (
		((buf_format & JPGENC_FORMAT_BIT) ==
		/* 1024,512: the value of jpgenc format bit */
		JPGENC_FORMAT_YUV422) ? 1024 : 512))) {
		cam_err(" stride[%d] is invalid", config->buffer.stride);
		return -1;
	}
	return 0;
}

static int check_input_buffer_yuv(jpgenc_config_t *config)
{
	unsigned int buf_format;
	buf_format = (unsigned int)(config->buffer.format);

	if ((config->buffer.input_buffer_y) == 0 ||
		/* 16: offset */
		!CHECK_ALIGN(config->buffer.input_buffer_y, 16)) {
		cam_err(" input buffer y[0x%x] is invalid",
			config->buffer.input_buffer_y);
		return -1;
	}

	if (((buf_format & JPGENC_FORMAT_BIT) == JPGENC_FORMAT_YUV420) &&
		((config->buffer.input_buffer_uv == 0) ||
		/* 16: ratio */
		!CHECK_ALIGN(config->buffer.input_buffer_uv, 16))) {
		cam_err(" input buffer uv[0x%x] is invalid ",
			config->buffer.input_buffer_uv);
		return -1;
	}

	if (((buf_format & JPGENC_FORMAT_BIT) == JPGENC_FORMAT_YUV420) &&
		(config->buffer.input_buffer_uv -
		config->buffer.input_buffer_y <
		config->buffer.stride * 8 * 16)) { /* 8,16:ratio */
		cam_err(" buffer format is invalid");
		return -1;
	}
	return 0;
}

static int check_buffer_quality_outputsize(jpgenc_config_t *config)
{
	if (config->buffer.quality > 100) { /* 100:the max value */
		cam_err(" quality[%d] is invalid, adjust to 100",
			config->buffer.quality);
		config->buffer.quality = 100; /* 100:the max value */
	}

	if (config->buffer.output_size <= JPGENC_HEAD_SIZE ||
		config->buffer.output_size > MAX_JPEG_BUFFER_SIZE) {
		cam_err(" output size[%u] is invalid",
			config->buffer.output_size);
		return -1;
	}
	return 0;
}

/*
 * set picture format
 * called by config_jpeg
 */
static void set_picture_format(void __iomem *base_addr, int fmt)
{
	unsigned int tmp = 0;

	if (((unsigned int)fmt & JPGENC_FORMAT_BIT) == JPGENC_FORMAT_YUV422)
		SET_FIELD_TO_REG((void __iomem*)(
			(char*)base_addr + JPGENC_JPE_PIC_FORMAT_REG),
			JPGENC_ENC_PIC_FORMAT, 0);
	else
		SET_FIELD_TO_REG((void __iomem*)(
			(char*)base_addr + JPGENC_JPE_PIC_FORMAT_REG),
			JPGENC_ENC_PIC_FORMAT, 1);

	if (fmt == JPGENC_FORMAT_VYUY || fmt == JPGENC_FORMAT_NV21) {
		REG_SET_FIELD(tmp, JPGENC_SWAPIN_Y_UV, 0);
		REG_SET_FIELD(tmp, JPGENC_SWAPIN_U_V, 1);
	} else if (fmt == JPGENC_FORMAT_YVYU) {
		REG_SET_FIELD(tmp, JPGENC_SWAPIN_Y_UV, 1);
		REG_SET_FIELD(tmp, JPGENC_SWAPIN_U_V, 1);
	} else if (fmt == JPGENC_FORMAT_YUYV) {
		REG_SET_FIELD(tmp, JPGENC_SWAPIN_Y_UV, 1);
		REG_SET_FIELD(tmp, JPGENC_SWAPIN_U_V, 0);
	} else { /* default formt JPGENC_FORMAT_UYVY|JPGENC_FORMAT_NV12 */
		REG_SET_FIELD(tmp, JPGENC_SWAPIN_Y_UV, 0);
		REG_SET_FIELD(tmp, JPGENC_SWAPIN_U_V, 0);
	}

	set_reg_val((void __iomem*)(
		(char*)base_addr + JPGENC_INPUT_SWAP_REG), tmp);
}

static void set_jpe_table_data(void __iomem *base_addr, const uint8_t *array,
	uint32_t size, uint32_t scaler)
{
	uint32_t temp;
	uint32_t tmpreg;
	uint32_t i;
	for (i = 1; i < size; i = i + 2) { /* 2:step */
		tmpreg = 0;
		temp = (array[i - 1] * scaler + 50U) / 100U;
		if (temp == 0U)
			temp = 1U;
		if (temp > 255U)
			temp = 255U;
		REG_SET_FIELD(tmpreg, JPGENC_TABLE_WDATA_H, temp);

		temp = (array[i] * scaler + 50U) / 100U;
		if (temp == 0U)
			temp = 1U;
		if (temp > 255U)
			temp = 255U;
		REG_SET_FIELD(tmpreg, JPGENC_TABLE_WDATA_L, temp);

		set_reg_val((void __iomem*)(
			(char*)base_addr + JPGENC_JPE_TABLE_DATA_REG), tmpreg);
	}
}

/*
 * set picture quality
 * called by config_jpeg
 */
static void set_picture_quality(void __iomem *base_addr, unsigned int quality)
{
	unsigned int scaler;
	unsigned int length;

	if (quality == 0) {
		SET_FIELD_TO_REG((void __iomem*)(
			(char*)base_addr + JPGENC_JPE_TQ_Y_SELECT_REG),
			JPGENC_TQ0_SELECT, 0);
		SET_FIELD_TO_REG((void __iomem*)(
			(char*)base_addr + JPGENC_JPE_TQ_U_SELECT_REG),
			JPGENC_TQ1_SELECT, 1);
		SET_FIELD_TO_REG((void __iomem*)(
			(char*)base_addr + JPGENC_JPE_TQ_V_SELECT_REG),
			JPGENC_TQ2_SELECT, 1);
	} else {
		if (quality < 50) /* 50:boundary value */
			scaler = 5000 / quality; /* 5000:ratio */
		else
			scaler = 200 - quality * 2; /* 200,2:ratio */

		/* q-table 2 */
		length = ARRAY_SIZE(luma_qtable2);
		SET_FIELD_TO_REG((void __iomem*)(
			(char*)base_addr + JPGENC_JPE_TABLE_ID_REG),
			JPGENC_TABLE_ID, 2); /* 2:length */
		set_jpe_table_data(base_addr, luma_qtable2, length, scaler);

		/* q-table 3 */
		length = ARRAY_SIZE(chroma_qtable2);
		SET_FIELD_TO_REG((void __iomem*)(
			(char*)base_addr + JPGENC_JPE_TABLE_ID_REG),
			JPGENC_TABLE_ID, 3); /* 3:length */
		set_jpe_table_data(base_addr, chroma_qtable2, length, scaler);

		SET_FIELD_TO_REG((void __iomem*)(
			(char*)base_addr + JPGENC_JPE_TQ_Y_SELECT_REG),
			JPGENC_TQ0_SELECT, 2); /* 2:length */
		SET_FIELD_TO_REG((void __iomem*)(
			(char*)base_addr + JPGENC_JPE_TQ_U_SELECT_REG),
			JPGENC_TQ1_SELECT, 3); /* 3:length */
		SET_FIELD_TO_REG((void __iomem*)(
			(char*)base_addr + JPGENC_JPE_TQ_V_SELECT_REG),
			JPGENC_TQ2_SELECT, 3); /* 3:length */
	}
}

static void set_picture_size(void __iomem *base_addr, jpgenc_config_t *config)
{
	uint32_t width_left;
	uint32_t width_right = 0;

	width_left = config->buffer.width;
	if (s_hjpeg.hw_ctl.power_controller == PC_HISP) {
		if (width_left >= 64) /* 64:boundary value */
			/* 2,16:ratio */
			width_left = ALIGN_DOWN((width_left / 2), 16);
		width_right = config->buffer.width - width_left;
	}

	SET_FIELD_TO_REG((void __iomem*)(
		(char*)base_addr + JPGENC_JPE_ENC_HRIGHT1_REG),
		JPGENC_ENC_HRIGHT1, width_left - 1);
	SET_FIELD_TO_REG((void __iomem*)(
		(char*)base_addr + JPGENC_JPE_ENC_VBOTTOM_REG),
		JPGENC_ENC_VBOTTOM, config->buffer.height - 1);
	SET_FIELD_TO_REG((void __iomem*)(
		(char*)base_addr + JPGENC_JPE_ENC_HRIGHT2_REG),
		JPGENC_ENC_HRIGHT2, width_right != 0 ? width_right -1 : 0);
}

int dump_jpeg_reg(void __iomem *viraddr)
{
	int i;
	if (!viraddr) {
		cam_err("%s: viraddr is NULL", __FUNCTION__);
		return -1;
	}
	for (i = 0; i <= JPGENC_FORCE_CLK_ON_CFG_REG;) {
		cam_debug("%s: jpeg reg 0x%x = 0x%x", __FUNCTION__, i,
			get_reg_val((void __iomem*)((char*)viraddr + i)));
		i += 4; /* 4:step */
	}
	for (i = JPGENC_DBG_0_REG; i <= JPGENC_DBG_13_REG;) {
		cam_debug("%s: dbg reg 0x%x = 0x%x", __FUNCTION__, i,
			get_reg_val((void __iomem*)((char*)viraddr + i)));
		i += 4; /* 4:step */
	}
	return 0;
}

void dump_reg(void)
{
	int i;
	void __iomem *viraddr = s_hjpeg.hw_ctl.jpegenc_viraddr;

	if (!viraddr) {
		cam_err("%s: jpegenc viraddr is NULL", __FUNCTION__);
		return;
	}

	for (i = 0; i <= JPGENC_FORCE_CLK_ON_CFG_REG;) {
		cam_info("%s: jpeg reg 0x%x = 0x%x", __FUNCTION__, i,
			get_reg_val((void __iomem*)((char*)viraddr + i)));
		i += 4; /* 4:step */
	}
	for (i = JPGENC_DBG_0_REG; i <= JPGENC_DBG_13_REG;) {
		cam_info("%s: jpeg reg 0x%x = 0x%x", __FUNCTION__, i,
			get_reg_val((void __iomem*)((char*)viraddr + i)));
		i += 4; /* 4:step */
	}

	/* ipp top */
	if (g_is_irq_merge != 0) {
		viraddr = s_hjpeg.hw_ctl.subctrl_viraddr;
		if (!viraddr) {
			cam_err("%s: jpegenc viraddr is NULL", __FUNCTION__);
			return;
		}
		cam_info("%s: top reg: 0x%08x=0x%08x", __FUNCTION__,
			JPGENC_IRQ_REG0,
			get_reg_val((void __iomem*)(
			(char*)viraddr + JPGENC_IRQ_REG0)));
		cam_info("%s: top reg: 0x%08x=0x%08x", __FUNCTION__,
			JPGENC_IRQ_REG1, get_reg_val((void __iomem*)(
			(char*)viraddr + JPGENC_IRQ_REG1)));
		cam_info("%s: top reg: 0x%08x=0x%08x", __FUNCTION__,
			JPGENC_IRQ_REG2, get_reg_val((void __iomem*)(
			(char*)viraddr + JPGENC_IRQ_REG2)));
		cam_info("%s: top reg: 0x%08x=0x%08x", __FUNCTION__,
			JPG_RO_STATE, get_reg_val((void __iomem*)(
			(char*)viraddr + JPG_RO_STATE)));
		cam_info("%s: top reg: 0x%08x=0x%08x", __FUNCTION__,
			JPGENC_CRG_CFG0, get_reg_val((void __iomem*)(
			(char*)viraddr + JPGENC_CRG_CFG0)));
		cam_info("%s: top reg: 0x%08x=0x%08x", __FUNCTION__,
			JPGENC_CRG_CFG1, get_reg_val((void __iomem*)(
			(char*)viraddr + JPGENC_CRG_CFG1)));
		cam_info("%s: top reg: 0x%08x=0x%08x", __FUNCTION__,
			JPGENC_MEM_CFG, get_reg_val((void __iomem*)(
			(char*)viraddr + JPGENC_MEM_CFG)));
	}

	dump_cvdr_debug_reg(&s_hjpeg.hw_ctl);
	dump_smmu_reg(&s_hjpeg.hw_ctl);
}

#ifdef SOFT_RESET
static int hjpeg_soft_reset(hjpeg_base_t *phjpeg)
{
	int i;
	void __iomem *jpeg_top_base = NULL;
	void __iomem *cvdr_axi_base = NULL;
	void __iomem *temp_base = NULL;

	jpeg_top_base = phjpeg->hw_ctl.subctrl_viraddr;
	cvdr_axi_base = phjpeg->hw_ctl.cvdr_viraddr;

	cam_info("%s, enter, subctrl=0x%x, cvdr=0x%x",
		__FUNCTION__, jpeg_top_base, cvdr_axi_base);
#if 1
	for (i = 0; i < ARRAY_SIZE(sr_cfg_reg_sets); ++i) {
		switch (sr_cfg_reg_sets[i].base) {
		case SR_JPEG_TOP_BASE:
			temp_base = jpeg_top_base;
			break;
		case SR_CVDR_AXI_BASE:
			temp_base = cvdr_axi_base;
			break;
		default:
			break;
		}

		hjpeg_soft_reset_cfg_reg_sets(temp_base);

		temp_base = NULL;
		msleep(100); /* 100: 100 ms */
	}
#endif
	return 0;
}

static void hjpeg_soft_reset_cfg_reg_sets(void __iomem *temp_base)
{
	u32 tempVal;

	if (!temp_base)
		return;

	switch (sr_cfg_reg_sets[i].op_flag) {
	case SR_OP_WRITE_ALL: {
		set_reg_val(temp_base + sr_cfg_reg_sets[i].offset,
			sr_cfg_reg_sets[i].value);
		cam_info("SR_OP_WRITE_ALL: write@0x%X@0x%X",
			sr_cfg_reg_sets[i].offset, sr_cfg_reg_sets[i].value);
		break;
	}
	case SR_OP_READ_ALL: {
		tempVal = get_reg_val(temp_base + sr_cfg_reg_sets[i].offset);
		cam_info("SR_OP_READ_ALL: read@0x%X@0x%X, expect@0x%X",
			sr_cfg_reg_sets[i].offset,
			sr_cfg_reg_sets[i].value, tempVal);
		break;
	}
	case SR_OP_WRITE_BIT: {
		tempVal = get_reg_val(temp_base + sr_cfg_reg_sets[i].offset);
		cam_info("SR_OP_WRITE_BIT: read@0x%X@0x%X",
			sr_cfg_reg_sets[i].offset, tempVal);
		if (sr_cfg_reg_sets[i].value == 1) {
			tempVal |= sr_cfg_reg_sets[i].bit_flag;
		} else {
			tempVal = ~tempVal;
			tempVal |= sr_cfg_reg_sets[i].bit_flag;
			tempVal = ~tempVal;
		}
		set_reg_val(temp_base + sr_cfg_reg_sets[i].offset, tempVal);
		cam_info("SR_OP_WRITE_BIT: write@0x%X@0x%X",
			sr_cfg_reg_sets[i].offset, tempVal);
		break;
	}
	case SR_OP_READ_BIT: {
		tempVal = get_reg_val(temp_base + sr_cfg_reg_sets[i].offset);
		cam_info("SR_OP_READ_BIT: read@0x%X = 0x%X",
			sr_cfg_reg_sets[i].offset, tempVal);
		cam_info("SR_OP_READ_BIT: value0x%X, expect@0x%X",
			(tempVal & sr_cfg_reg_sets[i].bit_flag),
			sr_cfg_reg_sets[i].value);
		break;
	}
	default:
		break;
	}
}
#endif

/*
 * configure JPEGENC register
 * called by hjpeg_encode_process
 */
static void hjpeg_config_jpeg(jpgenc_config_t *config)
{
	unsigned int buf_format;
	void __iomem *base_addr = s_hjpeg.hw_ctl.jpegenc_viraddr;

	cam_info("%s enter", __FUNCTION__);
	if (!config || !base_addr) {
		cam_err("%s: config is null! %d", __FUNCTION__, __LINE__);
		return;
	}
	buf_format = (unsigned int)(config->buffer.format);

	set_picture_format(base_addr, config->buffer.format);

	set_picture_size(base_addr, config);

	set_picture_quality(base_addr, config->buffer.quality);

	/* set input buffer address */
	set_input_buffer_address(config, buf_format, base_addr);

	/* set preread */
	if ((buf_format & JPGENC_FORMAT_BIT) == JPGENC_FORMAT_YUV420)
		SET_FIELD_TO_REG((void __iomem*)((char*)base_addr +
		JPGENC_PREREAD_REG), JPGENC_PREREAD, 4); /* 4:length */
	else
		SET_FIELD_TO_REG((void __iomem*)((char*)base_addr +
			JPGENC_PREREAD_REG), JPGENC_PREREAD, 0);

	SET_FIELD_TO_REG((void __iomem*)((char*)base_addr +
		/* 4:offset */
		JPGENC_STRIDE_REG), JPGENC_STRIDE, config->buffer.stride >> 4);
	SET_FIELD_TO_REG((void __iomem*)(
		(char*)base_addr + JPGENC_JPE_ENCODE_REG), JPGENC_ENCODE, 1);

	cam_info("%s activate JPGENC", __FUNCTION__);
	do_gettimeofday(&s_timeval_start);
	SET_FIELD_TO_REG((void __iomem*)(
		(char*)base_addr + JPGENC_JPE_INIT_REG), JPGENC_JP_INIT, 1);
}

static void set_input_buffer_address(jpgenc_config_t *config,
	unsigned int buf_format, void __iomem *base_addr)
{
	if (get_hjpeg_iova_update() == CVDR_IOVA_ADDR_34BITS) {
		u_jpegenc_address addr_y;
		/* 4:offset */
		addr_y.bits.address = config->buffer.input_buffer_y >> 4;
		addr_y.bits.reserved = 0;
		set_reg_val((void __iomem*)((char*)base_addr +
			JPGENC_ADDRESS_Y_REG), addr_y.reg32);
		if ((buf_format & JPGENC_FORMAT_BIT) == JPGENC_FORMAT_YUV420) {
			u_jpegenc_address addr_uv;
			addr_uv.bits.address =
				/* 4:offset */
				config->buffer.input_buffer_uv >> 4;
			addr_uv.bits.reserved = 0;
			set_reg_val((void __iomem*)((char*)base_addr +
				JPGENC_ADDRESS_UV_REG), addr_uv.reg32);
		}
	} else {
		SET_FIELD_TO_REG((void __iomem*)(
			(char*)base_addr + JPGENC_ADDRESS_Y_REG),
			/* 4:offset */
			JPGENC_ADDRESS_Y, config->buffer.input_buffer_y >> 4);
		if ((buf_format & JPGENC_FORMAT_BIT) == JPGENC_FORMAT_YUV420)
			SET_FIELD_TO_REG((void __iomem*)((char*)base_addr +
				JPGENC_ADDRESS_UV_REG), JPGENC_ADDRESS_UV,
				/* 4:offset */
				config->buffer.input_buffer_uv >> 4);
	}
}

static void hjpeg_enable_autogating(void)
{
	void __iomem *base_addr = s_hjpeg.hw_ctl.jpegenc_viraddr;
	set_reg_val((void __iomem*)(
		(char*)base_addr + JPGENC_FORCE_CLK_ON_CFG_REG), 0x0);
}

static void hjpeg_disable_autogating(void)
{
	void __iomem *base_addr = s_hjpeg.hw_ctl.jpegenc_viraddr;
	set_reg_val((void __iomem*)(
		(char*)base_addr + JPGENC_FORCE_CLK_ON_CFG_REG), 0x1);
}

static void hjpeg_disabe_irq(void)
{
	switch (g_is_irq_merge) {
	case JPEG_IRQ_IN_OUT:
		hjpeg_irq_inout_mask(false);
		break;
	case JPEG_IRQ_MERGE:
		hjpeg_irq_mask(s_hjpeg.hw_ctl.subctrl_viraddr, false);
		break;
	default:
		set_reg_val((void __iomem*)(
			(char*)s_hjpeg.hw_ctl.jpegenc_viraddr +
			JPGENC_JPE_STATUS_IMR_REG), 0x00);
		break;
	}
}

static void hjpeg_enable_irq(void)
{
	switch (g_is_irq_merge) {
	case JPEG_IRQ_IN_OUT:
		hjpeg_irq_inout_mask(true);
		break;
	case JPEG_IRQ_MERGE:
		hjpeg_irq_mask(s_hjpeg.hw_ctl.subctrl_viraddr, true);
		break;
	default:
		set_reg_val(s_hjpeg.hw_ctl.jpegenc_viraddr +
			JPGENC_JPE_STATUS_IMR_REG, 0x30);
		break;
	}
}

/* IOCTL HJPEG_ENCODE_PROCESS */
static int hjpeg_encode_process(hjpeg_intf_t *i, void *cfg)
{
	int ret;
	jpgenc_config_t *pcfg = (jpgenc_config_t *)cfg;
	(void)i;
	if (!pcfg) {
		cam_err("%s: cfg is null %d", __FUNCTION__, __LINE__);
		return -EINVAL;
	}

	cam_info("width:%d, height:%d, stride:%d, format:%#x, quality:%d, rst:%d, ion_fd:%d",
		pcfg->buffer.width, pcfg->buffer.height,
		pcfg->buffer.stride, pcfg->buffer.format,
		pcfg->buffer.quality, pcfg->buffer.rst, pcfg->buffer.ion_fd);
	cam_debug("input_buffer_y:%#x, input_buffer_uv:%#x, output_buffer:%#x, output_size:%u",
		pcfg->buffer.input_buffer_y, pcfg->buffer.input_buffer_uv,
		pcfg->buffer.output_buffer, pcfg->buffer.output_size);

	if (bypass_smmu() && (s_hjpeg.hw_ctl.smmu_type != ST_SMMUV3)) {
		if ((s_hjpeg.phy_pgd.phy_pgd_base == 0) &&
			(s_hjpeg.hw_ctl.power_controller == PC_DRV)) {
			cam_err("%s() %d phy_pgd_base is invalid, encode processing terminated",
				__FUNCTION__, __LINE__);
			return -EINVAL;
		}
	}

#ifndef MST_DEBUG
	ret = check_config(pcfg);
	if (ret != 0) {
		cam_err("%s() %d check_config failed, encode processing terminated",
			__FUNCTION__, __LINE__);
		return ret;
	}
#else
	ret = 0;
#endif

	return hjpeg_encode_process_config(pcfg);
}

static int hjpeg_encode_process_config(jpgenc_config_t *pcfg)
{
	int ret = 0;
#ifdef SOFT_RESET
	static int nop = 0;
#endif
	long jiff;

	set_rstmarker(s_hjpeg.hw_ctl.jpegenc_viraddr, pcfg->buffer.rst);
	hjpeg_disable_autogating();
	hjpeg_config_cvdr(&s_hjpeg.hw_ctl, pcfg);
	hjpeg_enable_irq();
	hjpeg_config_jpeg(pcfg);

#ifdef SOFT_RESET
	if (nop == 0) {
		hjpeg_encode_process_nop0(&jiff);
		nop = 1;
		return -1;
	}
#endif

	jiff = (long)msecs_to_jiffies(WAIT_ENCODE_TIMEOUT);
	if (down_timeout(&s_hjpeg.buff_done, jiff) != 0) {
		cam_err("[fail] time out wait for jpeg encode");
		ret = -1;
		dump_reg();
		hjpeg_irq_clr(s_hjpeg.hw_ctl.subctrl_viraddr);
#if defined(HISP120_CAMERA)
		hjpeg_120_dump_reg();
#endif
	}

	do_gettimeofday(&s_timeval_end);

	calculate_encoding_time();

	/* for debug */
	dump_jpeg_reg(s_hjpeg.hw_ctl.jpegenc_viraddr);

	hjpeg_disabe_irq();
	hjpeg_enable_autogating();

#ifdef SOFT_RESET
	if (nop == 2) { /* 2:the value of nOp */
		nop = 3; /* 3:the value of nOp */
		hjpeg_encode_process_nop2();
		return -1;
	}

	if (nop == 1) {
		nop = 2; /* 2:the value of nOp */
		cam_info("%s: op=1, just set to 2", __FUNCTION__);
	}
#endif

	if (hjpeg_encode_get_jpeg_size(pcfg) != 0)
		return -1;

	return ret;
}

#ifdef SOFT_RESET
static void hjpeg_encode_process_nop0(long *jiff)
{
	cam_info("%s: op=0, reset when encode", __FUNCTION__);
	hjpeg_soft_reset(&s_hjpeg);
	/* init qtable\hufftable etc. */
	hjpeg_init_hw_param(s_hjpeg.hw_ctl.jpegenc_viraddr,
		s_hjpeg.hw_ctl.power_controller, bypass_smmu());
	hjpeg_disabe_irq();
	hjpeg_enable_autogating();

	*jiff = (long)msecs_to_jiffies(10); /* 10: conversion ratio */
	down_timeout(&s_hjpeg.buff_done, *jiff);
}

static void hjpeg_encode_process_nop2(void)
{
	cam_info("%s: op=2, reset when encode done", __FUNCTION__);
	hjpeg_soft_reset(&s_hjpeg);
	/* init qtable\hufftable etc. */
	hjpeg_init_hw_param(s_hjpeg.hw_ctl.jpegenc_viraddr,
		s_hjpeg.hw_ctl.power_controller, bypass_smmu());
	hjpeg_disabe_irq();
	hjpeg_enable_autogating();
}
#endif

static int hjpeg_encode_get_jpeg_size(jpgenc_config_t *pcfg)
{
	u32 byte_cnt;
	byte_cnt = get_reg_val((void __iomem*)(
		(char*)s_hjpeg.hw_ctl.jpegenc_viraddr +
		JPGENC_JPG_BYTE_CNT_REG));
	if (byte_cnt == 0) {
		cam_err("%s encode fail", __FUNCTION__);
		pcfg->jpegSize = 0;
		return -1;
	}

	pcfg->jpegSize = byte_cnt;
	cam_info("%s jpeg encode process success.size=%u",
		__FUNCTION__, pcfg->jpegSize);
	return 0;
}

static int power_control(hjpeg_base_t *phjpeg, bool on)
{
	int ret;
	/* different platform operates differently. */
	switch (phjpeg->hw_ctl.power_controller) {
	case PC_DRV:
		if (on)
			ret = hjpeg_poweron(phjpeg);
		else
			ret = hjpeg_poweroff(phjpeg);
		break;
	case PC_HISP:
		if (on)
			ret = hisp_jpeg_powerup();
		else
			ret = hisp_jpeg_powerdn();
		break;
	case PC_IPPCOMM:
		if (on)
			ret = hjpeg_poweron_ippcomm(phjpeg);
		else
			ret = hjpeg_poweroff_ippcomm(phjpeg);
		break;
	default:
		cam_info("%s powerup/powerdown by self", __FUNCTION__);
		ret = -EINVAL;
		break;
	}

	return ret;
}

/* IOCTL HJPEG_POWERON */
static int hjpeg_power_on(hjpeg_intf_t *i)
{
	int ret;
	hjpeg_base_t *phjpeg = NULL;

	if (!i) {
		cam_err("%s jpeg intf is null", __FUNCTION__);
		return -EINVAL;
	}

	phjpeg = i_2_hjpeg_enc(i);

	cam_info("%s enter", __FUNCTION__);

	ret = power_control(phjpeg, true);
	if (ret != 0) {
		cam_err("%s() %d jpeg power up fail", __FUNCTION__, __LINE__);
		return ret;
	}

	/* config smmu */
	ret = hjpeg_smmu_config(&phjpeg->hw_ctl, &(phjpeg->phy_pgd));
	if (ret != 0) {
		cam_err("%s() %d failed to config smmu, prepare to power down",
			__FUNCTION__, __LINE__);
		goto powerup_error;
	}

	/* set jpeg clock ctrl */
	hjpeg_apbbus_timeout_ctrl(phjpeg->hw_ctl.subctrl_viraddr);
	ret = hjpeg_clk_ctrl(phjpeg->hw_ctl.subctrl_viraddr, true);
	if (ret != 0) {
		cam_err("%s %d failed to enable jpeg clock , prepare to power down",
			__FUNCTION__, __LINE__);
		goto powerup_error;
	}

	/* init qtable\hufftable etc. */
	hjpeg_init_hw_param(phjpeg->hw_ctl.jpegenc_viraddr,
		phjpeg->hw_ctl.power_controller, bypass_smmu());

	sema_init(&(phjpeg->buff_done), 0);

	if (phjpeg->irq_no) {
		/* request irq */
		ret = request_irq(phjpeg->irq_no, hjpeg_irq_handler, 0,
			"hjpeg_irq", 0);
		if (ret != 0) {
			cam_err("fail to request irq [%d], error: %d",
				phjpeg->irq_no, ret);
			goto powerup_error;
		}
	}
	cam_info("%s jpeg power on success", __FUNCTION__);
	return ret;

powerup_error:
	if (power_control(phjpeg, false) != 0)
		cam_err("%s() %d jpeg power down fail", __FUNCTION__, __LINE__);
	return ret;
}

/* IOCTL HJPEG_POWERDOWN */
static int hjpeg_power_off(hjpeg_intf_t *i)
{
	int ret;
	hjpeg_base_t *phjpeg;

	phjpeg = i_2_hjpeg_enc(i);

	cam_info("%s enter", __FUNCTION__);

	if (phjpeg->irq_no)
		free_irq(phjpeg->irq_no, 0);

	/* deconfig smmu: release v2 ipp smmu reference */
	ret = hjpeg_smmu_deconfig(&phjpeg->hw_ctl, &(phjpeg->phy_pgd));
	if (ret != 0)
		cam_err("%s() %d failed to deconfig smmu", __func__, __LINE__);

	ret = power_control(phjpeg, false);
	if (ret != 0)
		cam_err("%s jpeg power down fail", __FUNCTION__);
	else
		cam_info("%s jpeg power off success", __FUNCTION__);

	return ret;
}

static int hjpeg_poweron(hjpeg_base_t *pjpeg_dev)
{
	cam_info("%s enter", __FUNCTION__);

	if (atomic_read(&pjpeg_dev->jpeg_on) != 0) {
		atomic_inc(&pjpeg_dev->jpeg_on);
		cam_info("%s: jpeg power up, ref=%d",
			__FUNCTION__, atomic_read(&pjpeg_dev->jpeg_on));
		return 0;
	}

	mutex_lock(&pjpeg_dev->wake_lock_mutex);
	if (!pjpeg_dev->power_wakelock.active) {
		__pm_stay_awake(&pjpeg_dev->power_wakelock);
		cam_info("%s jpeg power on enter, wake lock", __FUNCTION__);
	}
	mutex_unlock(&pjpeg_dev->wake_lock_mutex);

	return hjpeg_poweron_enalbe(pjpeg_dev);
}

static int hjpeg_poweron_enalbe(hjpeg_base_t *pjpeg_dev)
{
#if (POWER_CTRL_INTERFACE == POWER_CTRL_CFG_REGS)
	return hjpeg_poweron_enalbe_by_cfg_reg(pjpeg_dev);
#else
	return hjpeg_poweron_enalbe_by_regulator(pjpeg_dev);
#endif
}

#if (POWER_CTRL_INTERFACE == POWER_CTRL_CFG_REGS)
static int hjpeg_poweron_enalbe_by_cfg_reg(hjpeg_base_t *pjpeg_dev)
{
	int ret;

	ret = cfg_powerup_regs();
	/* power up with config regs */
	if (ret != 0) {
		cam_err("Failed: cfg_powerup_regs %d", ret);
		goto failed_jpg_poweron;
	}

	ret = hjpeg_setclk_enable(pjpeg_dev, JPEG_FUNC_CLK);
	if (ret != 0) {
		cam_err("Failed: hjpeg_setclk_enable %d", ret);
		cfg_powerdn_regs();
		goto failed_jpg_poweron;
	}
	atomic_inc(&pjpeg_dev->jpeg_on);
	cam_info("%s: jpeg first power up, ref=%d",
		__FUNCTION__, atomic_read(&pjpeg_dev->jpeg_on));

	return ret;

failed_jpg_poweron:
	mutex_lock(&pjpeg_dev->wake_lock_mutex);
	if (pjpeg_dev->power_wakelock.active) {
		__pm_relax(&pjpeg_dev->power_wakelock);
		cam_err("%s jpeg power on failed, wake unlock", __FUNCTION__);
	}
	mutex_unlock(&pjpeg_dev->wake_lock_mutex);

	return ret;
}
#else

static int hjpeg_poweron_enalbe_by_regulator(hjpeg_base_t *pjpeg_dev)
{
	int ret;
	int ret2;
	ret = regulator_enable(pjpeg_dev->media_supply);
	/* power up with hardware interface */
	if (ret != 0) {
		cam_err("Failed: media regulator_enable %d", ret);
		goto failed_jpg_poweron;
	}
	ret = hjpeg_setclk_enable(pjpeg_dev, JPEG_FUNC_CLK);
	if (ret != 0) {
		cam_err("Failed: hjpeg_setclk_enable %d", ret);
		ret2 = regulator_disable(pjpeg_dev->media_supply);
		if (ret2 != 0)
			cam_err("Failed: media regulator_enable %d", ret2);
		goto failed_jpg_poweron;
	}
	ret = regulator_enable(pjpeg_dev->jpeg_supply);
	if (ret != 0) {
		cam_err("Failed: jpeg regulator_enable %d", ret);
		hjpeg_setclk_disable(pjpeg_dev, JPEG_FUNC_CLK);
		ret2 = regulator_disable(pjpeg_dev->media_supply);
		if (ret2 != 0)
			cam_err("Failed: media regulator_disable %d", ret2);
		goto failed_jpg_poweron;
	}
	atomic_inc(&pjpeg_dev->jpeg_on);
	cam_info("%s: jpeg first power up, ref=%d",
		__FUNCTION__, atomic_read(&pjpeg_dev->jpeg_on));

	return ret;

failed_jpg_poweron:
	mutex_lock(&pjpeg_dev->wake_lock_mutex);
	if (pjpeg_dev->power_wakelock.active) {
		__pm_relax(&pjpeg_dev->power_wakelock);
		cam_err("%s jpeg power on failed, wake unlock", __FUNCTION__);
	}
	mutex_unlock(&pjpeg_dev->wake_lock_mutex);

	return ret;
}
#endif

static int hjpeg_poweroff(hjpeg_base_t *pjpeg_dev)
{
	int ret;
	if (atomic_read(&pjpeg_dev->jpeg_on) == 0) {
		cam_info("%s: jpeg never power on, ref=%d",
			__FUNCTION__, atomic_read(&pjpeg_dev->jpeg_on));
		return 0;
	}

	atomic_dec(&pjpeg_dev->jpeg_on);
	if (atomic_read(&pjpeg_dev->jpeg_on) != 0) {
		cam_info("%s: jpeg power off, ref=%d", __FUNCTION__,
			atomic_read(&pjpeg_dev->jpeg_on));
		return 0;
	}

	/* close tbu and tcu connection */
	if (pjpeg_dev->hw_ctl.smmu_type == ST_SMMUV3) {
		ret = do_cfg_smmuv3_tbuclose(&pjpeg_dev->hw_ctl);
		if (ret != 0)
			cam_err("%s() %d failed to close connection of tbu and tcu",
				__FUNCTION__, __LINE__);
	}

	/* set jpeg clock ctrl */
	ret = hjpeg_clk_ctrl(pjpeg_dev->hw_ctl.subctrl_viraddr, false);
	if (ret != 0)
		cam_err("%s() %d failed to disable jpeg clock , prepare to power down",
			__FUNCTION__, __LINE__);

#if (POWER_CTRL_INTERFACE == POWER_CTRL_CFG_REGS)
	/* power down with config regs */
	hjpeg_setclk_disable(pjpeg_dev, JPEG_FUNC_CLK);
	ret = cfg_powerdn_regs();
	if (ret != 0)
		cam_err("Failed: cfg_powerdn_regs %d", ret);
#else
	ret = regulator_disable(pjpeg_dev->jpeg_supply);
	/* power down with hardware interface */
	if (ret != 0)
		cam_err("Failed: jpeg regulator_disable %d", ret);

	hjpeg_setclk_disable(pjpeg_dev, JPEG_FUNC_CLK);

	ret = regulator_disable(pjpeg_dev->media_supply);
	if (ret != 0)
		cam_err("Failed: media regulator_disable %d", ret);
#endif

	cam_info("%s: jpeg power down, ref=%d", __FUNCTION__,
		atomic_read(&pjpeg_dev->jpeg_on));

	mutex_lock(&pjpeg_dev->wake_lock_mutex);
	if (pjpeg_dev->power_wakelock.active) {
		__pm_relax(&pjpeg_dev->power_wakelock);
		cam_info("%s jpeg power off exit, wake unlock", __FUNCTION__);
	}
	mutex_unlock(&pjpeg_dev->wake_lock_mutex);

	return ret;
}

static int hjpeg_poweron_ippcomm(hjpeg_base_t *pjpeg_dev)
{
	int ret;
	cam_info("%s enter", __FUNCTION__);

	if (!s_hjpeg_ipp_comm) {
		cam_err("%s: s_hjpeg_ipp_comm is null", __FUNCTION__);
		return EINVAL;
	}

	if (atomic_read(&pjpeg_dev->jpeg_on) != 0) {
		atomic_inc(&pjpeg_dev->jpeg_on);
		cam_info("%s: jpeg power up, ref=%d", __FUNCTION__,
			atomic_read(&pjpeg_dev->jpeg_on));
		return 0;
	}

	mutex_lock(&pjpeg_dev->wake_lock_mutex);
	if (!pjpeg_dev->power_wakelock.active) {
		__pm_stay_awake(&pjpeg_dev->power_wakelock);
		cam_info("%s jpeg power on enter, wake lock", __FUNCTION__);
	}
	mutex_unlock(&pjpeg_dev->wake_lock_mutex);

	ret = s_hjpeg_ipp_comm->power_up(s_hjpeg_ipp_comm);
	if (ret != 0) {
		cam_err("%s: power failed %u", __FUNCTION__, ret);
		goto failed_jpg_poweron;
	}

	ret = hjpeg_setclk_enable(pjpeg_dev, JPEG_FUNC_CLK);
	if (ret != 0) {
		cam_err("Failed: hjpeg_setclk_enable.%d", ret);
		ret = s_hjpeg_ipp_comm->power_dn(s_hjpeg_ipp_comm);
		if (ret != 0)
			cam_err("Failed: ippcomm power dn.%d", ret);
		goto failed_jpg_poweron;
	}

	atomic_inc(&pjpeg_dev->jpeg_on);
	cam_info("%s: jpeg first power up, ref=%d", __FUNCTION__,
		atomic_read(&pjpeg_dev->jpeg_on));

	return ret;

failed_jpg_poweron:
	mutex_lock(&pjpeg_dev->wake_lock_mutex);
	if (pjpeg_dev->power_wakelock.active) {
		__pm_relax(&pjpeg_dev->power_wakelock);
		cam_err("%s jpeg power on failed, wake unlock", __FUNCTION__);
	}
	mutex_unlock(&pjpeg_dev->wake_lock_mutex);

	return ret;
}

static int hjpeg_poweroff_ippcomm(hjpeg_base_t *pjpeg_dev)
{
	int ret;
	if (atomic_read(&pjpeg_dev->jpeg_on) == 0) {
		cam_info("%s: jpeg never power on, ref=%d", __FUNCTION__,
			atomic_read(&pjpeg_dev->jpeg_on));
		return 0;
	}

	atomic_dec(&pjpeg_dev->jpeg_on);
	if (atomic_read(&pjpeg_dev->jpeg_on) != 0) {
		cam_info("%s: jpeg power off, ref=%d", __FUNCTION__,
			atomic_read(&pjpeg_dev->jpeg_on));
		return 0;
	}

	ret = s_hjpeg_ipp_comm->disable_smmu(s_hjpeg_ipp_comm);
	if (ret != 0)
		cam_err("%s: smmu disable failed %d", __FUNCTION__, ret);

	/* set jpeg clock ctrl */
	ret = hjpeg_clk_ctrl(pjpeg_dev->hw_ctl.subctrl_viraddr, false);
	if (ret != 0)
		cam_err("%s%d: failed to disable jpeg clock , prepare to power down",
			__FUNCTION__, __LINE__);

	hjpeg_setclk_disable(pjpeg_dev, JPEG_FUNC_CLK);

	ret = s_hjpeg_ipp_comm->power_dn(s_hjpeg_ipp_comm);
	if (ret != 0)
		cam_err("%s: failed to power dn jpeg %d", __FUNCTION__, ret);

	cam_info("%s: jpeg power down, ref=%d", __FUNCTION__,
		atomic_read(&pjpeg_dev->jpeg_on));

	mutex_lock(&pjpeg_dev->wake_lock_mutex);
	if (pjpeg_dev->power_wakelock.active) {
		__pm_relax(&pjpeg_dev->power_wakelock);
		cam_info("%s jpeg power off exit, wake unlock", __FUNCTION__);
	}
	mutex_unlock(&pjpeg_dev->wake_lock_mutex);

	return ret;
}

/* unmap all viraddr */
static void hjpeg_unmap_baseaddr(void)
{
	hjpeg_base_t *phjpeg = &s_hjpeg;

	if (IS_ERR_OR_NULL(phjpeg->hw_ctl.jpegenc_viraddr)) {
		iounmap((void*)phjpeg->hw_ctl.jpegenc_viraddr);
		phjpeg->hw_ctl.jpegenc_viraddr = NULL;
	}

	if (IS_ERR_OR_NULL(phjpeg->hw_ctl.subctrl_viraddr)) {
		iounmap((void*)phjpeg->hw_ctl.subctrl_viraddr);
		phjpeg->hw_ctl.subctrl_viraddr = NULL;
	}
	if (IS_ERR_OR_NULL(phjpeg->hw_ctl.smmu_viraddr)) {
		iounmap((void*)phjpeg->hw_ctl.smmu_viraddr);
		phjpeg->hw_ctl.smmu_viraddr = NULL;
	}

	if (IS_ERR_OR_NULL(phjpeg->hw_ctl.cvdr_viraddr)) {
		iounmap((void*)phjpeg->hw_ctl.cvdr_viraddr);
		phjpeg->hw_ctl.cvdr_viraddr = NULL;
	}

	if (IS_ERR_OR_NULL(phjpeg->hw_ctl.top_viraddr)) {
		iounmap((void*)phjpeg->hw_ctl.top_viraddr);
		phjpeg->hw_ctl.top_viraddr = NULL;
	}

	if (IS_ERR_OR_NULL(phjpeg->hw_ctl.secadapt_viraddr)) {
		iounmap((void*)phjpeg->hw_ctl.secadapt_viraddr);
		phjpeg->hw_ctl.secadapt_viraddr = NULL;
	}

	phjpeg->irq_no = 0;

/* debug */
#if (POWER_CTRL_INTERFACE == POWER_CTRL_CFG_REGS)
	cfg_unmap_reg_base();
#endif /* (POWER_CTRL_INTERFACE == POWER_CTRL_CFG_REGS) */
}

/* init resource for remap base addr and requst irq */
static int hjpeg_map_baseaddr(void)
{
	struct device_node *np = NULL;
	int ret;
	hjpeg_base_t *phjpeg = &s_hjpeg;

	np = phjpeg->pdev->dev.of_node;
	if (!np) {
		cam_err("%s: of node NULL", __FUNCTION__);
		return -ENXIO;
	}

	/* property(huawei,hjpeg-base) = <address, size>, so count is 2 */
	ret = hjpeg_map_hjpeg_base(np, phjpeg);
	if (ret != 0) {
		cam_err("%s hjpeg_map_hjpeg_base fail", __FUNCTION__);
		return -ENXIO;
	}

	/* property(huawei,smmu-base) = <address, size>, so count is 2 */
	ret = hjpeg_map_smmu_base(np, phjpeg);
	if (ret != 0) {
		cam_err("%s hjpeg_map_smmu_base fail", __FUNCTION__);
		goto fail;
	}

	/* property(huawei,cvdr-base) = <address, size>, so count is 2 */
	ret = hjpeg_map_cvdr_base(np, phjpeg);
	if (ret != 0) {
		cam_err("%s hjpeg_map_cvdr_base fail", __FUNCTION__);
		goto fail;
	}

	/* property(huawei,subctrl-base) = <address, size>, so count is 2 */
	ret = hjpeg_map_subctrl_baseaddr(np, phjpeg);
	if (ret != 0) {
		cam_err("%s hjpeg_map_subctrl_baseaddr fail", __FUNCTION__);
		goto fail;
	}

	/* interrupt from dts */
	phjpeg->irq_no = irq_of_parse_and_map(np, 0);
	if (phjpeg->irq_no == 0) {
		cam_err("%s failed get irq num %d", __FUNCTION__, __LINE__);
		goto fail;
	}
	cam_info("%s irq [%d]", __FUNCTION__, phjpeg->irq_no);

/* debug */
#if (POWER_CTRL_INTERFACE == POWER_CTRL_CFG_REGS)
	ret = cfg_map_reg_base();
	if (ret != 0) {
		cam_err("%s: cfg map failed", __FUNCTION__);
		ret = -ENXIO;
		goto fail;
	}
#endif
	return 0;

fail:
	hjpeg_unmap_baseaddr();

	return ret;
}

static int hjpeg_map_hjpeg_base(struct device_node *np, hjpeg_base_t *phjpeg)
{
	uint32_t base_array[2] = {0}; /* 2: array size */
	size_t count = 2;
	int ret;

	ret = of_property_read_u32_array(np, "huawei,hjpeg-base",
		base_array, count);
	if (ret < 0) {
		cam_err("%s failed get jpeg base %d", __FUNCTION__, __LINE__);
		return -ENXIO;
	}

	phjpeg->hw_ctl.jpegenc_phyaddr = base_array[0];
	phjpeg->hw_ctl.jpegenc_mem_size = base_array[1];

	phjpeg->hw_ctl.jpegenc_viraddr = ioremap(phjpeg->hw_ctl.jpegenc_phyaddr,
		phjpeg->hw_ctl.jpegenc_mem_size);
	if (IS_ERR_OR_NULL(phjpeg->hw_ctl.jpegenc_viraddr)) {
		cam_err("%s jpegenc phyaddr ioremap fail", __FUNCTION__);
		return -ENXIO;
	}

	cam_debug("%s hjpeg base address = 0x%x. hjpeg-base size = 0x%x",
		__FUNCTION__, base_array[0], base_array[1]);
	return 0;
}

static int hjpeg_map_smmu_base(struct device_node *np, hjpeg_base_t *phjpeg)
{
	uint32_t base_array[2] = {0}; /* 2: array size */
	size_t count = 2;
	int ret;

	ret = of_property_read_u32_array(np, "huawei,smmu-base",
		base_array, count);
	if (ret < 0) {
		cam_err("%s failed get smmu base %d", __FUNCTION__, __LINE__);
		return -ENXIO;
	}

	phjpeg->hw_ctl.smmu_phyaddr = base_array[0];
	phjpeg->hw_ctl.smmu_mem_size = base_array[1];
	phjpeg->hw_ctl.smmu_viraddr = ioremap(base_array[0], base_array[1]);
	if (IS_ERR_OR_NULL(phjpeg->hw_ctl.smmu_viraddr)) {
		cam_err("%s smmu phyaddr ioremap fail", __FUNCTION__);
		return -ENXIO;
	}

	cam_debug("%s smmu base address = 0x%x. smmu-base size = 0x%x",
		__FUNCTION__, base_array[0], base_array[1]);
	return 0;
}

static int hjpeg_map_cvdr_base(struct device_node *np, hjpeg_base_t *phjpeg)
{
	uint32_t base_array[2] = {0}; /* 2: array size */
	size_t count = 2;
	int ret;

	ret = of_property_read_u32_array(np, "huawei,cvdr-base",
		base_array, count);
	if (ret < 0) {
		cam_err("%s failed get cvdr base %d", __FUNCTION__, __LINE__);
		return -ENXIO;
	}

	phjpeg->hw_ctl.cvdr_phyaddr = base_array[0];
	phjpeg->hw_ctl.cvdr_mem_size = base_array[1];
	phjpeg->hw_ctl.cvdr_viraddr = ioremap(base_array[0], base_array[1]);
	if (IS_ERR_OR_NULL(phjpeg->hw_ctl.cvdr_viraddr)) {
		cam_err("%s cvdr phyaddr ioremap fail", __FUNCTION__);
		return -ENXIO;
	}

	cam_debug("%s cvdr base address = 0x%x. cvdr-base size = 0x%x",
		__FUNCTION__, base_array[0], base_array[1]);
	return 0;
}

static int hjpeg_map_subctrl_baseaddr(struct device_node *np,
	hjpeg_base_t *phjpeg)
{
	uint32_t base_array[2] = {0}; /* 2: array size */
	size_t count = 2;
	int ret;

	ret = of_property_read_u32_array(np, "huawei,subctrl-base",
		base_array, count);
	if (ret < 0) {
		cam_err("%s failed get subctrl base %d",
			__FUNCTION__, __LINE__);
		return -ENXIO;
	}
	phjpeg->hw_ctl.subctrl_phyaddr = base_array[0];
	phjpeg->hw_ctl.subctrl_mem_size = base_array[1];
	phjpeg->hw_ctl.subctrl_viraddr = ioremap(base_array[0], base_array[1]);
	if (IS_ERR_OR_NULL(phjpeg->hw_ctl.subctrl_viraddr)) {
		cam_err("%s subctrl phyaddr ioremap fail", __FUNCTION__);
		return -ENXIO;
	}

	cam_debug("%s subctrl base address = 0x%x. subctrl-base size = 0x%x",
		__FUNCTION__, base_array[0], base_array[1]);
	return 0;
}

/* get power controller */
static int get_dts_power_prop(struct device *pdev)
{
	struct device_node *np = NULL;
	u32 tmp = 0;
	int ret;

	if (!pdev) {
		cam_err("%s: pdev NULL", __FUNCTION__);
		return -1;
	}

	np = pdev->of_node;
	if (!np) {
		cam_err("%s: of node NULL", __FUNCTION__);
		return -1;
	}

	ret = of_property_read_u32(np, "huawei,power_control", &tmp);
	if (ret < 0) {
		cam_err("%s: getpower_control flag failed", __FUNCTION__);
		return -1;
	}
	s_hjpeg.hw_ctl.power_controller = tmp;
	cam_info("%s: power_controller=%d", __FUNCTION__, tmp);

	if (s_hjpeg.hw_ctl.power_controller == PC_DRV) {
		ret = get_dts_pc_drv_power_prop(pdev, np);
		if (ret != 0) {
			cam_err("%s: get_dts_pc_drv_power_prop failed",
				__FUNCTION__);
			return -1;
		}
	} else if (s_hjpeg.hw_ctl.power_controller == PC_IPPCOMM) {
		ret = get_dts_pc_ippcomm_power_prop(np);
		if (ret != 0) {
			cam_err("%s: get_dts_pc_ippcomm_power_prop failed",
				__FUNCTION__);
			return -1;
		}
	}
	return 0;
}

static int get_dts_pc_drv_power_prop(struct device *pdev,
	struct device_node *np)
{
	int ret;

	cam_info("[%s] drv regulator", __FUNCTION__);

	ret = get_dts_pc_drv_power_regulator(pdev);
	if (ret != 0) {
		cam_err("%s: get_dts_pc_drv_power_regulator failed",
			__FUNCTION__);
		return -1;
	}

	ret = get_dts_pc_drv_power_frequency(np);
	if (ret != 0) {
		cam_err("%s: get_dts_pc_drv_power_frequency failed",
			__FUNCTION__);
		return -1;
	}

	/* get clk parameters */
	ret = get_dts_pc_drv_power_clk_name(pdev, np);
	if (ret != 0) {
		cam_err("%s: get_dts_pc_drv_power_clk_name failed",
			__FUNCTION__);
		return -1;
	}

	return 0;
}

static int get_dts_pc_drv_power_regulator(struct device *pdev)
{
	/* get supply for jpeg */
	s_hjpeg.jpeg_supply = devm_regulator_get(pdev, "hjpeg-srt");
	if (IS_ERR(s_hjpeg.jpeg_supply)) {
		cam_err("[%s] Failed: ISPSRT devm_regulator_get", __FUNCTION__);
		return -1;
	}

	/* get supply for media */
	s_hjpeg.media_supply = devm_regulator_get(pdev, "hjpeg-media");
	if (IS_ERR(s_hjpeg.media_supply)) {
		cam_err("[%s] Failed: media devm_regulator_get", __FUNCTION__);
		return -1;
	}
	return 0;
}

static int get_dts_pc_drv_power_frequency(struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "clock-low-frequency",
		&(s_hjpeg.jpegclk_low_frequency));
	if (ret < 0) {
		cam_err("[%s] Failed: of_property_read_u32.%d",
			__FUNCTION__, ret);
		return -1;
	}

	ret = of_property_read_u32(np, "power-off-frequency",
		&(s_hjpeg.power_off_frequency));
	if (ret < 0) {
		cam_err("[%s] Failed: of_property_read_u32.%d",
			__FUNCTION__, ret);
		return -1;
	}
	return 0;
}

static int get_dts_pc_drv_power_clk_name(struct device *pdev,
	struct device_node *np)
{
	int i;
	int ret;
	const char *clk_name[JPEG_CLK_MAX] = {""};

	ret = of_property_read_string_array(np, "clock-names",
		clk_name, JPEG_CLK_MAX);
	if (ret < 0) {
		cam_err("[%s] Failed: of_property_read_string_array.%d",
			__FUNCTION__, ret);
		return -1;
	}

	for (i = 0; i < JPEG_CLK_MAX; ++i)
		cam_debug("[%s] clk_name[%d] = %s",
			__FUNCTION__, i, clk_name[i]);

	ret = of_property_read_u32_array(np, "clock-value",
		(unsigned int*)(&(s_hjpeg.jpegclk_value[0])), JPEG_CLK_MAX);
	if (ret < 0) {
		cam_err("[%s] Failed: of_property_read_u32_array.%d",
			__FUNCTION__, ret);
		return -1;
	}
	for (i = 0; i < JPEG_CLK_MAX; i++) {
		s_hjpeg.jpegclk[i] = devm_clk_get(pdev, clk_name[i]);
		if (IS_ERR_OR_NULL(s_hjpeg.jpegclk[i])) {
			cam_err("[%s] Failed : jpgclk.%s.%d.%li",
				__FUNCTION__, clk_name[i], i,
				PTR_ERR(s_hjpeg.jpegclk[i]));
			return -1;
		}
		cam_debug("[%s] Jpeg clock.%d.%s: %d Hz",
			__FUNCTION__, i, clk_name[i],
			s_hjpeg.jpegclk_value[i]);
	}
	return 0;
}

static int get_dts_pc_ippcomm_power_prop(struct device_node *np)
{
	int ret;
	ret = of_property_read_u32(np, "jpegclk-rate-mode",
		&(s_hjpeg.hw_ctl.jpgclk_prop.jpegclk_mode));
	if (ret != 0) {
		cam_info("[%s] Failded: of_property_read_u32 jpegclk-rate-mode",
			__FUNCTION__,
			s_hjpeg.hw_ctl.jpgclk_prop.jpegclk_mode);
		return -1;
	}

	ret = of_property_read_u32(np, "jpegclk-lowfreq-mode",
		&(s_hjpeg.hw_ctl.jpgclk_prop.jpegclk_lowfreq_mode));
	if (ret != 0) {
		cam_info("[%s] Failded: of_property_read_u32 jpegclk-rate-mode",
			__FUNCTION__,
			s_hjpeg.hw_ctl.jpgclk_prop.jpegclk_lowfreq_mode);
		return -1;
	}

	ret = of_property_read_u32(np, "jpegclk-offfreq-mode",
		&(s_hjpeg.hw_ctl.jpgclk_prop.jpegclk_offreq_mode));
	if (ret != 0) {
		cam_info("[%s] Failded: of_property_read_u32 jpegclk-rate-mode",
			__FUNCTION__,
			s_hjpeg.hw_ctl.jpgclk_prop.jpegclk_offreq_mode);
		return -1;
	}
	return 0;
}

static int get_dts_smmu_prop(struct device *pdev)
{
	struct device_node *np = NULL;
	u32 tmp;
	int ret;

	if (!pdev) {
		cam_err("%s: pdev NULL", __FUNCTION__);
		return -1;
	}

	np = pdev->of_node;
	if (!np) {
		cam_err("%s: of node NULL", __FUNCTION__);
		return -1;
	}

	/* get smmu_bypass */
	tmp = 0;
	ret = of_property_read_u32(np, "huawei,smmu_bypass", &tmp);
	if (ret < 0) {
		cam_err("%s: get smmu_bypass flag failed", __FUNCTION__);
		goto error;
	}
	s_hjpeg.hw_ctl.smmu_bypass = tmp;
	cam_info("%s: smmu_bypass=%d", __FUNCTION__, tmp);

	/* get smmu_type */
	tmp = 0;
	ret = of_property_read_u32(np, "huawei,smmu_type", &tmp);
	if (ret < 0) {
		cam_err("%s: get smmu_type flag failed", __FUNCTION__);
		goto error;
	}
	s_hjpeg.hw_ctl.smmu_type = tmp;
	cam_info("%s: smmu_type=%d", __FUNCTION__, tmp);
	return 0;

error:
	return -1;
}

static int get_dts_secadapt_prop(struct device *pdev)
{
	struct device_node *np = NULL;
	int ret;
	if (!pdev) {
		cam_err("%s: pdev NULL", __FUNCTION__);
		return -1;
	}

	np = pdev->of_node;
	if (!np) {
		cam_err("%s: of node NULL", __FUNCTION__);
		return -1;
	}

	/* property(huawei,secadapt-base) = <address, size>, so count is 2 */
	ret = get_dts_secadapt_prop_base(np);
	if (ret != 0) {
		cam_err("%s: get_dts_secadapt_prop_base fail", __FUNCTION__);
		return -1;
	}

	/* property(huawei,sid-ssid) = <sid ssid>, so count is 2 */
	ret = get_dts_secadapt_prop_sid_ssid(np);
	if (ret != 0) {
		cam_err("%s: get_dts_secadapt_prop_sid_ssid fail",
			__FUNCTION__);
		return -1;
	}

	/* property(huawei,jpgenc-swid) = <start_swid,swid_num>,so count is 2 */
	ret = get_dts_secadapt_prop_jpgen_swid(np);
	if (ret != 0) {
		cam_err("%s: get_dts_secadapt_prop_jpgen_swid fail",
			__FUNCTION__);
		return -1;
	}
	return 0;
}

static int get_dts_secadapt_prop_base(struct device_node *np)
{
	int ret;
	uint32_t base_array[2] = {0}; /* 2: array size */
	size_t count = 2;

	ret = of_property_read_u32_array(np, "huawei,secadapt-base",
		base_array, count);
	if (ret < 0) {
		cam_err("%s failed get subctrl base %d",
			__FUNCTION__, __LINE__);
		return -1;
	}
	s_hjpeg.hw_ctl.secadapt_phyaddr = base_array[0];
	s_hjpeg.hw_ctl.secadapt_mem_size = base_array[1];
	s_hjpeg.hw_ctl.secadapt_viraddr = ioremap(base_array[0], base_array[1]);
	if (IS_ERR_OR_NULL(s_hjpeg.hw_ctl.secadapt_viraddr)) {
		cam_err("%s secadapt phyaddr ioremap fail", __FUNCTION__);
		return -1;
	}

	cam_debug("%s secadapt base address = 0x%x. secadapt-base size = 0x%x. sec_viraddr: 0x%016lx",
		__FUNCTION__, base_array[0], base_array[1],
		s_hjpeg.hw_ctl.secadapt_viraddr);
	return 0;
}

static int get_dts_secadapt_prop_sid_ssid(struct device_node *np)
{
	int ret;
	uint32_t base_array[2] = {0}; /* 2: array size */
	size_t count = 2;

	ret = of_property_read_u32_array(np, "huawei,sid-ssid",
		base_array, count);
	if (ret < 0) {
		cam_err("%s failed get sid-ssid property %d",
			__FUNCTION__, __LINE__);
		return -1;
	}

	s_hjpeg.hw_ctl.secadapt_prop.sid = base_array[0];
	s_hjpeg.hw_ctl.secadapt_prop.ssid_ns = base_array[1];
	cam_debug("%s [%d] sid: %u, ssidv_ns: %u", __FUNCTION__,
		__LINE__, base_array[0], base_array[1]);
	return 0;
}

static int get_dts_secadapt_prop_jpgen_swid(struct device_node *np)
{
	int ret;
	uint32_t base_array[2] = {0}; /* 2: array size */
	size_t count = 2;

	ret = of_property_read_u32_array(np, "huawei,jpgen-swid",
		base_array, count);
	if (ret < 0) {
		cam_err("%s failed get sid-ssid property %d",
			__FUNCTION__, __LINE__);
		return -1;
	}

	s_hjpeg.hw_ctl.secadapt_prop.start_swid = base_array[0];
	s_hjpeg.hw_ctl.secadapt_prop.swid_num = base_array[1];

	cam_info("%s %d: start_swid: %u swid_num: %u", __FUNCTION__, __LINE__,
		base_array[0], base_array[1]);
	return 0;
}

static int get_dts_cvdr_prop(struct device *pdev)
{
	struct device_node *np = NULL;
	int ret;
	if (!pdev) {
		cam_err("%s: pdev NULL", __FUNCTION__);
		return -1;
	}

	np = pdev->of_node;
	if (!np) {
		cam_err("%s: of node NULL", __FUNCTION__);
		return -1;
	}

	ret = get_dts_cvdr_property_data(np);
	if (ret != 0) {
		cam_err("%s: get_dts_cvdr_property_data fail", __FUNCTION__);
		return -1;
	}

	ret = get_dts_cvdr_stream_id(np);
	if (ret != 0) {
		cam_err("%s: get_dts_cvdr_stream_id fail", __FUNCTION__);
		return -1;
	}

	get_dts_cvdr_qos_update(np);
	get_dts_cvdr_iova_update_version(np);
	get_dts_cvdr_wr_port_addr_update_version(np);
	get_dts_cvdr_pixel_format_update(np);
	get_dts_cvdr_cfg_update(np);
	get_dts_cvdr_dis_timeout(np);
	get_dts_cvdr_du_allocate_update(np);
	get_dts_cvdr_limiter_du_prop(np);
	return 0;
}

static int get_dts_cvdr_property_data(struct device_node *np)
{
	int ret;
	uint32_t base_array[3] = {0}; /* 3:array size */
	size_t count = 3;

	/* property(huawei,cvdr) = <type, rd-port wr-port>, so count is 3 */
	ret = of_property_read_u32_array(np, "huawei,cvdr", base_array, count);
	if (ret < 0) {
		cam_err("%s failed get cvdr property %d",
			__FUNCTION__, __LINE__);
		return -1;
	}

	s_hjpeg.hw_ctl.cvdr_prop.type = base_array[0];
	s_hjpeg.hw_ctl.cvdr_prop.rd_port = base_array[1];
	s_hjpeg.hw_ctl.cvdr_prop.wr_port = base_array[2]; /* 2:index */
	cam_info("%s: cvdr prop=<%d,%d,%d>", __FUNCTION__,
		base_array[0], base_array[1], base_array[2]); /* 2:index */

	return 0;
}

static int get_dts_cvdr_stream_id(struct device_node *np)
{
	int ret;
	uint32_t base_array[3] = {0}; /* 3:array size */
	size_t count = 3;

	/* property(huawei,cvdr) = <type, rd-port wr-port>, so count is 3 */
	ret = of_property_read_u32_array(np, "huawei,stream_id",
		base_array, count);
	if (ret < 0) {
		cam_err("%s failed get stream_id property %d",
			__FUNCTION__, __LINE__);
		return -1;
	}

	s_hjpeg.hw_ctl.stream_id[0] = base_array[0];
	s_hjpeg.hw_ctl.stream_id[1] = base_array[1];
	s_hjpeg.hw_ctl.stream_id[2] = base_array[2]; /* 2:index */
	cam_info("%s: stream id=<%d,%d,%d>", __FUNCTION__,
		base_array[0], base_array[1], base_array[2]); /* 2:index */

	return 0;
}

static void get_dts_cvdr_qos_update(struct device_node *np)
{
	int ret;

	/* g_is_qos_update */
	ret = of_property_read_u32(np, "huawei,qos_update", &g_is_qos_update);
	if (ret < 0)
		cam_info("%s: get qos_update flag failed", __FUNCTION__);
}

static void get_dts_cvdr_iova_update_version(struct device_node *np)
{
	int ret;

	/* g_iova_update_version */
	ret = of_property_read_u32(np, "huawei,iova_update",
		&g_iova_update_version);
	if (ret < 0)
		cam_info("%s: get iova_update flag failed", __FUNCTION__);
	cam_info("%s: iova_addr_update=<%d>", __FUNCTION__,
		g_iova_update_version);
}

static void get_dts_cvdr_wr_port_addr_update_version(struct device_node *np)
{
	int ret;

	/* is_wr_port_addr_update */
	ret = of_property_read_u32(np, "huawei,wr_port_addr_update",
		&g_wr_port_addr_update_version);
	if (ret < 0)
		cam_info("%s: get wr_port_addr_update flag failed",
			__FUNCTION__);
	cam_info("%s: wr_port_addr_update=<%d>", __FUNCTION__,
		g_wr_port_addr_update_version);
}

static void get_dts_cvdr_pixel_format_update(struct device_node *np)
{
	int ret;

	/* is_pixel_fmt_update */
	ret = of_property_read_u32(np, "huawei,pix_format_update",
		&g_pixel_format_update);
	if (ret < 0)
		cam_info("%s: get g_pixel_format_update flag failed",
			__FUNCTION__);
	cam_info("%s: g_pixel_format_update=<%d>", __FUNCTION__,
		g_pixel_format_update);
}

static void get_dts_cvdr_cfg_update(struct device_node *np)
{
	int ret;

	/* is_cvdr_cfg_update */
	ret = of_property_read_u32(np, "huawei,cvdr_cfg_update",
		&g_cvdr_cfg_update);
	if (ret < 0)
		cam_info("%s: get cvdr_cfg_update flag failed", __FUNCTION__);
	cam_info("%s: g_cvdr_cfg_update=<%d>", __FUNCTION__, g_cvdr_cfg_update);
}

static void get_dts_cvdr_dis_timeout(struct device_node *np)
{
	int ret;

	/* is_cvdr_dis_timeout */
	ret = of_property_read_u32(np, "huawei,cvdr_dis_timeout",
		&g_cvdr_dis_timeout);
	if (ret < 0)
		cam_info("%s: get g_cvdr_dis_timeout flag failed",
			__FUNCTION__);
	cam_info("%s: g_cvdr_dis_timeout=<%d>",
		__FUNCTION__, g_cvdr_dis_timeout);
}

static void get_dts_cvdr_du_allocate_update(struct device_node *np)
{
	int ret;

	/* g_is_du_allocate_update */
	ret = of_property_read_u32(np, "huawei,du_allocate_update",
		&g_is_du_allocate_update);
	if (ret < 0)
		cam_info("%s: get is_du_allocate_update failed",
			__FUNCTION__);
}

static void get_dts_cvdr_limiter_du_prop(struct device_node *np)
{
	int ret;
	uint32_t base_array[3] = {0}; /* 3:array size */
	size_t count = 3;

	s_hjpeg.hw_ctl.cvdr_prop.flag = 0;
	ret = of_property_read_u32_array(np, "huawei,cvdr_limiter_du",
		base_array, count);
	if (ret >= 0) {
		s_hjpeg.hw_ctl.cvdr_prop.flag = 1;
		s_hjpeg.hw_ctl.cvdr_prop.rd_limiter = base_array[0];
		s_hjpeg.hw_ctl.cvdr_prop.wr_limiter = base_array[1];
		/* 2, allocated du */
		s_hjpeg.hw_ctl.cvdr_prop.allocated_du = base_array[2];
		cam_info("%s: limiter_du=<%d,%d,%d>", __FUNCTION__,
			/* 2, allocated du */
			base_array[0], base_array[1], base_array[2]);
	}
}

static int hjpeg_get_dts(struct platform_device *p_devive)
{
	int ret;
	struct device *pdev = NULL;
	struct device_node *np = NULL;

	if (!p_devive) {
		cam_err("%s: p_devive NULL", __FUNCTION__);
		return -1;
	}

	pdev = &(p_devive->dev);

	np = pdev->of_node;
	if (!np) {
		cam_err("%s: of node NULL", __FUNCTION__);
		return -1;
	}

	ret = hjpeg_get_dts_data(pdev, np);
	if (ret != 0) {
		cam_err("%s: hjpeg_get_dts_data fail", __FUNCTION__);
		return -1;
	}

	return 0;
}

static int hjpeg_get_dts_data(struct device *pdev, struct device_node *np)
{
	int ret;
	u32 chip_type = CT_CS;
	/* get chip type */
	ret = of_property_read_u32(np, "huawei,chip_type", &chip_type);
	if (ret < 0)
		cam_err("%s: get chip_type flag failed", __FUNCTION__);
	s_hjpeg.hw_ctl.chip_type = chip_type;
	cam_info("%s: chip_type=%d", __FUNCTION__, chip_type);

	/* g_clk_ctl_offset */
	ret = of_property_read_u32(np, "huawei,clk_ctl_offset",
		&g_clk_ctl_offset);
	if (ret == 0)
		cam_info("%s: update clk_ctl_offset=0x%x", __FUNCTION__,
			g_clk_ctl_offset);

	/* hjpeg_irq_merge */
	ret = of_property_read_u32(np, "huawei,irq-merge", &g_is_irq_merge);
	if (ret == 0)
		cam_info("%s: update irq_merge=%u", __FUNCTION__,
			g_is_irq_merge);

	/* prefetch_bypass */
	ret = of_property_read_u32(np, "huawei,prefetch_bypass",
		&g_is_prefetch_bypass);
	if (ret == 0)
		cam_info("%s: update prefetch_bypass=%u", __FUNCTION__,
			g_is_prefetch_bypass);

	ret = hjpeg_get_dts_prop_data(pdev);
	if (ret != 0) {
		cam_err("%s: hjpeg_get_dts_prop_data fail", __FUNCTION__);
		return -1;
	}

	/* iova-padding-mode */
	ret = of_property_read_u32(np, "huawei,support-iova-padding",
		&(s_hjpeg.hw_ctl.support_iova_padding));
	if (ret == 0)
		cam_info("%s: support-iova-padding=%u", __FUNCTION__,
			s_hjpeg.hw_ctl.support_iova_padding);

	ret = of_property_read_u32(np, "huawei,twice_read_reg",
		&g_is_read_twice);
	if (ret == 0)
		cam_info("%s: g_is_read_twice=%u",
			__FUNCTION__, g_is_read_twice);
	return 0;
}

static int hjpeg_get_dts_prop_data(struct device *pdev)
{
	int ret;
	ret = get_dts_cvdr_prop(pdev);
	if (ret < 0) {
		cam_err("%s: get cvdr property failed", __FUNCTION__);
		return -1;
	}
	ret = get_dts_smmu_prop(pdev);
	if (ret < 0) {
		cam_err("%s: get smmu property failed", __FUNCTION__);
		return -1;
	}
	/* get power controller */
	ret = get_dts_power_prop(pdev);
	if (ret < 0) {
		cam_err("%s: get power property failed", __FUNCTION__);
		return -1;
	}

	if (s_hjpeg.hw_ctl.smmu_type == ST_SMMUV3) {
		ret = get_dts_secadapt_prop(pdev);
		if (ret < 0) {
			cam_err("%s: get secadapt property failed",
				__FUNCTION__);
			return -1;
		}
	} else if (s_hjpeg.hw_ctl.smmu_type != ST_SMMUV3) {
		ret = get_phy_pgd_base();
		if (ret < 0) {
			cam_err("%s: get phy pgd base failed", __FUNCTION__);
			return -1;
		}
	}
	return 0;
}

static int get_phy_pgd_base(void)
{
	phys_addr_t pgd_base = hicam_buf_get_pgd_base();
	if (!pgd_base)
		return -1;

	s_hjpeg.phy_pgd.phy_pgd_base = (uint32_t)pgd_base;
	/* ptw_msb = phy_pgd_base[38:32] */
	s_hjpeg.phy_pgd.phy_pgd_fama_ptw_msb =
		((uint32_t)(pgd_base >> 32)) & 0x0000007F; /* 32:offset */
	/* bps_msb_ns = phy_pgd_base[38:33] */
	s_hjpeg.phy_pgd.phy_pgd_fama_bps_msb_ns =
		((uint32_t)(pgd_base >> 32)) & 0x0000007E; /* 32:offset */
	return 0;
}

static int hjpeg_setclk_enable(hjpeg_base_t *pjpeg_dev, int idx)
{
	int ret;
	unsigned int jpeg_clk_value;
	cam_debug("%s enter (idx=%d) ", __FUNCTION__, idx);

	if (pjpeg_dev->hw_ctl.power_controller == PC_IPPCOMM)
		return hjpeg_ipp_comm_setclk_enable(pjpeg_dev);

	jpeg_clk_value = pjpeg_dev->jpegclk_value[idx];
#if defined(CONFIG_ES_LOW_FREQ)
	jpeg_clk_value = 415000000;
#endif
	ret = jpeg_enc_set_rate(pjpeg_dev->jpegclk[idx], jpeg_clk_value);
	if (ret != 0) {
		cam_err("[%s] Failed: jpeg_enc_set_rate[%d - %u].%d",
			__FUNCTION__, idx, jpeg_clk_value, ret);
		/* try to set low freq */
		if (jpeg_enc_set_rate(pjpeg_dev->jpegclk[idx],
			pjpeg_dev->jpegclk_low_frequency) != 0)
			cam_err("[%s] Failed to set low frequency 1: jpeg_enc_set_rate[%d - %d]",
				__FUNCTION__, idx,
				pjpeg_dev->jpegclk_low_frequency);
	}

	ret = jpeg_enc_clk_prepare_enable(pjpeg_dev->jpegclk[idx]);
	if (ret != 0) {
		cam_err("[%s] Failed: jpeg_enc_clk_prepare_enable.%d",
			__FUNCTION__, ret);
		/* try to set low freq */
		if (jpeg_enc_set_rate(pjpeg_dev->jpegclk[idx],
			pjpeg_dev->jpegclk_low_frequency) != 0)
			cam_err("[%s] Failed to set low frequency 2: jpeg_enc_set_rate[%d - %d]",
				__FUNCTION__, idx,
				pjpeg_dev->jpegclk_low_frequency);

		ret = jpeg_enc_clk_prepare_enable(pjpeg_dev->jpegclk[idx]);
		if (ret != 0)
			cam_err("[%s] Failed: jpeg_enc_clk_prepare_enable at low frequency.%d",
				__FUNCTION__, ret);
		return ret;
	}

	return ret;
}

static int hjpeg_ipp_comm_setclk_enable(hjpeg_base_t *pjpeg_dev)
{
	int ret;
	cam_debug("%s enter", __FUNCTION__);
	ret = s_hjpeg_ipp_comm->set_jpgclk_rate(s_hjpeg_ipp_comm,
		pjpeg_dev->hw_ctl.jpgclk_prop.jpegclk_mode);
	if (ret != 0) {
		cam_err("[%s] Failed: ipp_comm->set_jpgclk_rate %u .%d",
			__FUNCTION__,
			pjpeg_dev->hw_ctl.jpgclk_prop.jpegclk_mode, ret);
		ret = s_hjpeg_ipp_comm->set_jpgclk_rate(
			s_hjpeg_ipp_comm,
			pjpeg_dev->hw_ctl.jpgclk_prop.jpegclk_lowfreq_mode);
		if (ret != 0)
			cam_err("[%s] Failed: ipp_comm->set_jpgclk_rate %u .%d",
				__FUNCTION__,
				pjpeg_dev->hw_ctl.jpgclk_prop.jpegclk_mode,
				ret);
	}

	return ret;
}

static void hjpeg_setclk_disable(hjpeg_base_t *pjpeg_dev, int idx)
{
	int ret;

	cam_debug("%s enter ()idx=%d", __FUNCTION__, idx);

	if (pjpeg_dev->hw_ctl.power_controller == PC_IPPCOMM) {
		ret = s_hjpeg_ipp_comm->set_jpgclk_rate(s_hjpeg_ipp_comm,
			pjpeg_dev->hw_ctl.jpgclk_prop.jpegclk_offreq_mode);
		if (ret != 0)
			cam_err("[%s] Failed: ipp_comm->set_jpgclk_rate off: %d, ret: %d",
				__FUNCTION__,
				pjpeg_dev->hw_ctl.jpgclk_prop.jpegclk_offreq_mode,
				ret);
		return;
	}

	/* === this is new constraint for cs begin === */
	if (pjpeg_dev->hw_ctl.power_controller == PC_DRV) {
		ret = jpeg_enc_set_rate(pjpeg_dev->jpegclk[idx],
			pjpeg_dev->power_off_frequency);
		if (ret != 0)
			cam_err("[%s] Failed: jpeg_enc_set_rate.%d",
				__FUNCTION__, ret);
	}
	/* === this is new constraint for cs end === */
	jpeg_enc_clk_disable_unprepare(pjpeg_dev->jpegclk[idx]);
}

static int hjpeg_clk_ctrl(void __iomem *subctrl1, bool enable)
{
	u32 set_clk;
	u32 cur_clk;
	int ret = 0;
	u32 offset;
	void __iomem *subctrl_clk;

	if (g_clk_ctl_offset != 0)
		offset = JPGENC_CRG_CFG0;
	else
		offset = 0;
	subctrl_clk = subctrl1 + offset;

	cam_info("%s enter [0x%x]", __FUNCTION__, get_reg_val(subctrl_clk));

	if (enable)
		set_reg_val(subctrl_clk, get_reg_val(subctrl_clk) | 0x1);
	else
		set_reg_val(subctrl_clk, get_reg_val(subctrl_clk) & 0xFFFFFFFE);
	set_clk = enable ? 0x1 : 0x0;
	cur_clk = get_reg_val(subctrl_clk);
	if (g_is_read_twice != 0) {
		cur_clk = get_reg_val(subctrl_clk);
		cam_info("%s:%d read again status 0x%x",
			__FUNCTION__, __LINE__, cur_clk);
	}
	if (set_clk != cur_clk) {
		cam_err("%s() %d write fail jpeg clk status 0x%x",
			__FUNCTION__, __LINE__, cur_clk);
		ret = -EIO;
	}

	cam_info("%s jpeg clk status [0x%x]", __FUNCTION__, cur_clk);
	return ret;
}

static void hjpeg_apbbus_timeout_ctrl(void __iomem *subctrl1)
{
	void __iomem *top_cfg0_addr;
	top_cfg0_addr = subctrl1 + DMA_CRG_CFG0;
	if (g_cvdr_dis_timeout) {
		/*
		 * disable APB bus timeout,bit19/20,
		 * should be 0, default val is 0x00120001
		 */
		set_reg_val((void __iomem*)
			((char*)top_cfg0_addr), 0x00020001);
		cam_info("%s: IPP_TOP reg: 0x%08x=0x%08x", __FUNCTION__,
			DMA_CRG_CFG0,
			get_reg_val((void __iomem*)((char*)top_cfg0_addr)));
	}
}

static void hjpeg_irq_clr(void __iomem *subctrl1)
{
	u32 set_val;
	void __iomem *subctrl1_irq_clr = subctrl1 + JPGENC_IRQ_REG0;

	cam_info("%s enter", __FUNCTION__);

	set_val = 0x00000002;

	set_reg_val(subctrl1_irq_clr, set_val);
}

static void hjpeg_irq_inout_clr(void __iomem *jpegenc, void __iomem *subctl)
{
	/* 1. clear inner jpgen */
	set_reg_val((void __iomem*)(
		(char*)jpegenc + JPGENC_JPE_STATUS_ICR_REG),
		IRQ_IN_CLR_VAL);
	/* 2. clear ipp top register */
	set_reg_val((void __iomem*)((char*)(subctl + JPGENC_IRQ_REG0)),
		IRQ_OUT_CLR_VAL);
}

static void hjpeg_irq_mask(void __iomem *subctrl1, bool enable)
{
	u32 set_val;
	u32 cur_val;
	void __iomem *subctrl1_irq_mask = subctrl1 + JPGENC_IRQ_REG1;

	cam_info("%s enter", __FUNCTION__);

	set_val = enable ? 0x0000001D : 0x0000001F;

	set_reg_val(subctrl1_irq_mask, set_val);
	cur_val = get_reg_val(subctrl1_irq_mask);
	if (set_val != cur_val)
		cam_err("%s() %d isp jpeg irq mask status %u, mask write failed set_val=%u",
			__FUNCTION__, __LINE__, cur_val, set_val);

	cam_info("%s isp jpeg irq mask status %u", __FUNCTION__, cur_val);
}

static void hjpeg_irq_inout_mask(bool enable)
{
	u32 set_val;
	u32 cur_val;
	void __iomem *subctrl1_irq_mask = (void __iomem*)(
		(char*)s_hjpeg.hw_ctl.subctrl_viraddr + JPGENC_IRQ_REG1);
	void __iomem *jpg_irq_mask = (void __iomem*)(
		(char*)s_hjpeg.hw_ctl.jpegenc_viraddr +
		JPGENC_JPE_STATUS_IMR_REG);

	set_val = enable ? (IRQ_OUT_MASK_ENABLE) : (IRQ_OUT_MASK_DISABLE);
	set_reg_val(subctrl1_irq_mask, set_val);
	cur_val = get_reg_val(subctrl1_irq_mask);
	if (set_val != cur_val)
		cam_err("%s() %d isp subcrl irq mask status %x, mask write failed set_val=%x",
			__FUNCTION__, __LINE__, cur_val, set_val);
	cam_info("%s isp subcrl irq mask(0x%016lx) status %u enable(%u)",
		__FUNCTION__, subctrl1_irq_mask, cur_val, enable);

	set_val = enable ? (IRQ_IN_MASK_ENABLE) : (IRQ_IN_MASK_DISABLE);
	cur_val = get_reg_val(jpg_irq_mask);
	set_reg_val(jpg_irq_mask, set_val);
	cur_val = get_reg_val(jpg_irq_mask);
	if (set_val != cur_val)
		cam_err("%s() %d isp jpeg irq mask(0x%016lx) status %u, mask write failed set_val=%u",
			__FUNCTION__, __LINE__, jpg_irq_mask, cur_val, set_val);
	cam_info("%s isp jpeg irq mask(0x%016lx)  setval(0x%08x)status curval(0x%08x), enable(%u)",
		__FUNCTION__, jpg_irq_mask, set_val, cur_val, enable);
}

static void hjpeg_encode_finish(void __iomem *jpegenc, void __iomem *subctl)
{
	u32 value;
	u32 result;

	if (!jpegenc || !subctl) {
		cam_err("%s, input param is null", __FUNCTION__);
		return;
	}

	switch (g_is_irq_merge) {
	case JPEG_IRQ_IN_OUT:
		/* chip ask twice read for avoid some problem */
		(void)get_reg_val((void __iomem*)(
			(char*)subctl + JPGENC_IRQ_REG2));
		value = get_reg_val((void __iomem*)(
			(char*)subctl + JPGENC_IRQ_REG2));
		result = value & IRQ_INOUT_ENCODE_FINISH;
		break;
	case JPEG_IRQ_MERGE:
		value = get_reg_val((void __iomem*)(
			(char*)subctl + JPGENC_IRQ_REG2));
		result = value & IRQ_MERGE_ENCODE_FINISH;
		break;
	default:
		value = get_reg_val((void __iomem*)(
			(char*)jpegenc + JPGENC_JPE_STATUS_RIS_REG));
		result = value & ENCODE_FINISH;
		break;
	}

	cam_info("encode finish:0x%x, result: 0x%x", value, result);

	if (result) {
		tasklet_schedule(&hjpeg_isr_tasklet);
	} else {
		cam_err("err irq JPGENC_IRQ_REG2 status 0x%x", value);
		dump_reg();
#if defined(HISP120_CAMERA)
		hjpeg_120_dump_reg();
#endif
	}

	/* clr jpeg irq */
	switch (g_is_irq_merge) {
	case JPEG_IRQ_IN_OUT:
		hjpeg_irq_inout_clr(jpegenc, subctl);
		break;
	case JPEG_IRQ_MERGE:
		hjpeg_irq_clr(subctl);
		break;
	default:
		set_reg_val((void __iomem*)(
			(char*)jpegenc + JPGENC_JPE_STATUS_ICR_REG), 0x30);
		break;
	}
}

static struct platform_driver s_hjpeg_driver = {
	.driver = {
		.name = "huawei,hjpeg",
		.owner = THIS_MODULE,
		.of_match_table = s_hjpeg_dt_match,
	},
};

static int32_t hjpeg_platform_probe(struct platform_device *pdev)
{
	int32_t ret;
	cam_info("%s enter [%s]", __FUNCTION__, s_hjpeg.name);

	wakeup_source_init(&s_hjpeg.power_wakelock, "jpeg_power_wakelock");
	mutex_init(&s_hjpeg.wake_lock_mutex);

	hjpeg_register(pdev, &(s_hjpeg.intf));
	s_hjpeg.pdev = pdev;
	atomic_set(&s_hjpeg.jpeg_on, 0);
	ret = hjpeg_get_dts(pdev);
	if (ret < 0) {
		cam_err("[%s] Failed: hjpeg_get_dts.%d", __FUNCTION__, ret);
		return ret;
	}
	ret = hjpeg_map_baseaddr();
	if (ret < 0)
		cam_err("[%s] Failed: hjpeg_map_baseaddr.%d",
			__FUNCTION__, ret);

#ifdef CONFIG_SMMU_RWERRADDR
	s_hjpeg.hw_ctl.jpg_smmu_rwerraddr_virt =
		kmalloc(SMMU_RW_ERR_ADDR_SIZE, GFP_KERNEL | __GFP_DMA);
	if (s_hjpeg.hw_ctl.jpg_smmu_rwerraddr_virt)
		(void)memset_s(s_hjpeg.hw_ctl.jpg_smmu_rwerraddr_virt,
			SMMU_RW_ERR_ADDR_SIZE, 0, SMMU_RW_ERR_ADDR_SIZE);
	else
		cam_err("[%s] kmalloc g_jpg_smmu_rwerraddr_virt fail",
			__FUNCTION__);
#endif

	if (s_hjpeg.hw_ctl.power_controller == PC_IPPCOMM) {
		s_hjpeg_ipp_comm = hipp_common_register(
			JPEGE_UNIT, &(pdev->dev));
		if (!s_hjpeg_ipp_comm) {
			cam_err("[%s] ipp comm register failed",
				__FUNCTION__);
			return -1;
		}
	}

#if defined( HISP120_CAMERA )
	if (hjpeg_120_map_reg() != 0)
		cam_err("hjpeg_120_map_reg failed");
#endif

	return ret;
}

/* 0 - prefetch not bypass, 1 - prefetch bypass */
int is_hjpeg_prefetch_bypass(void)
{
	cam_debug("%s: is_prefetch_bypass=%d", __FUNCTION__,
		g_is_prefetch_bypass);
	return g_is_prefetch_bypass;
}

int is_hjpeg_du_allocate_update(void)
{
	cam_debug("%s: is_du_allocate_update=%d", __FUNCTION__,
		g_is_du_allocate_update);
	return g_is_du_allocate_update;
}

int is_hjpeg_qos_update(void)
{
	cam_debug("%s is_qos_update=%d", __FUNCTION__, g_is_qos_update);
	return g_is_qos_update;
}

int get_hjpeg_iova_update(void)
{
	cam_debug("%s iova_update_version=%d", __FUNCTION__,
		g_iova_update_version);
	return g_iova_update_version;
}

int get_hjpeg_wr_port_addr_update(void)
{
	cam_debug("%s wr_port_addr_update_version=%d", __FUNCTION__,
		g_wr_port_addr_update_version);
	return g_wr_port_addr_update_version;
}

int is_cvdr_cfg_update(void)
{
	return g_cvdr_cfg_update;
}

int is_pixel_fmt_update(void)
{
	return g_pixel_format_update;
}

static int __init hjpeg_init_module(void)
{
	cam_info("%s enter", __FUNCTION__);
	/* register driver for non-hotpluggable device */
	return platform_driver_probe(&s_hjpeg_driver, hjpeg_platform_probe);
}

static void __exit hjpeg_exit_module(void)
{
	cam_info("%s enter", __FUNCTION__);

	if (s_hjpeg.hw_ctl.power_controller == PC_IPPCOMM) {
		if (hipp_common_unregister(JPEGE_UNIT) != 0)
			cam_err("[%s] ipp comm unregister failed",
				__FUNCTION__);
	}

#ifdef CONFIG_SMMU_RWERRADDR
	if (s_hjpeg.hw_ctl.jpg_smmu_rwerraddr_virt) {
		kfree(s_hjpeg.hw_ctl.jpg_smmu_rwerraddr_virt);
		s_hjpeg.hw_ctl.jpg_smmu_rwerraddr_virt = NULL;
	}
#endif

#if defined(HISP120_CAMERA)
	hjpeg_120_unmap_reg();
#endif

	hjpeg_unmap_baseaddr();
	hjpeg_unregister(s_hjpeg.pdev);
	platform_driver_unregister(&s_hjpeg_driver);

	wakeup_source_trash(&s_hjpeg.power_wakelock);
	mutex_destroy(&s_hjpeg.wake_lock_mutex);
}

module_init(hjpeg_init_module);
module_exit(hjpeg_exit_module);

MODULE_DESCRIPTION("hjpeg driver");
MODULE_LICENSE("GPL v2");
