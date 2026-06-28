/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

#include <linux/hisi/contexthub/tca.h>

#include "hisi_fb_def.h"
#include "hisi_fb.h"
#include "hisi_dp.h"

#include "dp/avgen.h"
#include "dp/intr.h"
#include "dp/core.h"
#include "dp/intr.h"
#include "dp/link.h"
#include "dp/dp_aux.h"
#include "hdcp22/host_lib_driver_linux_if.h"
#include "hdcp22/hdcp13.h"
#include <linux/printk.h>

#define DTS_COMP_SWING_VALUE "hisilicon,hisi_dp_swing"

struct platform_device *g_dp_pdev = NULL;
static bool bpress_powerkey = false;
static bool btrigger_timeout = false;

extern int get_current_dp_source_mode(void);
extern int update_external_display_timming_info(uint32_t width,uint32_t high,uint32_t fps);
extern int get_hdcp_state(uint32_t *state);

/*******************************************************************************
**
*/
int hisi_dptx_get_spec(void *data, unsigned int size, unsigned int *ext_acount)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct dp_ctrl *dptx = NULL;
	struct edid_audio *audio_info = NULL;

	if (data == NULL || ext_acount == NULL) {
		HISI_FB_ERR("[DP] parameter is NULL!\n");
		return -EINVAL;
	}

	if (g_dp_pdev == NULL) {
		HISI_FB_ERR("[DP] g_dp_pdev is NULL!\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(g_dp_pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return -EINVAL;
	}

	dptx = &hisifd->dp;
	audio_info = &dptx->edid_info.Audio;

	if (size < sizeof(struct edid_audio_info) * audio_info->extACount) {
		HISI_FB_ERR("[DP] size is error!size %d extACount %d\n", size, audio_info->extACount);
		return -EINVAL;
	}

	memcpy(data, audio_info->spec, sizeof(struct edid_audio_info) * audio_info->extACount);
	*ext_acount = audio_info->extACount;

	HISI_FB_INFO("[DP] get spec success\n");

	return 0;
}

int hisi_dptx_set_aparam(unsigned int channel_num, unsigned int data_width, unsigned int sample_rate)
{
	uint8_t orig_sample_freq = 0;
	uint8_t sample_freq = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct dp_ctrl *dptx = NULL;
	struct audio_params *aparams = NULL;

	if (channel_num > DPTX_CHANNEL_NUM_MAX || data_width > DPTX_DATA_WIDTH_MAX) {
		HISI_FB_ERR("[DP] input param is invalid. channel_num(%d) data_width(%d)\n", channel_num, data_width);
		return -EINVAL;
	}

	if (g_dp_pdev == NULL) {
		HISI_FB_ERR("[DP] g_dp_pdev is NULL!\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(g_dp_pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return -EINVAL;
	}

	dptx = &hisifd->dp;
	aparams = &dptx->aparams;

	HISI_FB_INFO("[DP] set aparam. channel_num(%d) data_width(%d) sample_rate(%d)\n",
		channel_num, data_width, sample_rate);

	switch (sample_rate) {
	case 32000:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_32K;
		sample_freq = IEC_SAMP_FREQ_32K;
		break;
	case 44100:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_44K;
		sample_freq = IEC_SAMP_FREQ_44K;
		break;
	case 48000:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_48K;
		sample_freq = IEC_SAMP_FREQ_48K;
		break;
	case 88200:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_88K;
		sample_freq = IEC_SAMP_FREQ_88K;
		break;
	case 96000:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_96K;
		sample_freq = IEC_SAMP_FREQ_96K;
		break;
	case 176400:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_176K;
		sample_freq = IEC_SAMP_FREQ_176K;
		break;
	case 192000:
		orig_sample_freq = IEC_ORIG_SAMP_FREQ_192K;
		sample_freq = IEC_SAMP_FREQ_192K;
		break;
	default:
		HISI_FB_INFO("[DP] invalid sample_rate\n");
		return -EINVAL;
	}

	aparams->iec_samp_freq = sample_freq;
	aparams->iec_orig_samp_freq = orig_sample_freq;
	aparams->num_channels = (uint8_t)channel_num;
	aparams->data_width = (uint8_t)data_width;

	dptx_audio_config(dptx);

	HISI_FB_INFO("[DP] set aparam success\n");

	return 0;
}

int dp_ceil(uint64_t a, uint64_t b)
{
	if (b == 0)
		return -1;

	if(a%b != 0) {
		return a/b + 1;
	} else {
		return a/b;
	}
}

int dp_pxl_ppll7_init(struct hisi_fb_data_type *hisifd, uint64_t pixel_clock)
{
	uint64_t refdiv,fbdiv,frac,postdiv1,postdiv2;
	uint64_t vco_min_freq_output = KIRIN_VCO_MIN_FREQ_OUPUT;
	uint64_t sys_clock_fref = KIRIN_SYS_FREQ;
	uint64_t ppll7_freq_divider;
	uint64_t vco_freq_output;
	uint64_t frac_range = 0x1000000;/*2^24*/
	uint64_t pixel_clock_ori;
	uint64_t pixel_clock_cur;
	uint32_t ppll7ctrl0;
	uint32_t ppll7ctrl1;
	uint32_t ppll7ctrl0_val;
	uint32_t ppll7ctrl1_val;
	int i, ret;
	int ceil_temp;
	int freq_divider_list[22]={1,2,3,4,5,6,7,8,9,10,
			12,14,15,16,20,21,24,
			25,30,36,42,49};

	int postdiv1_list[22]={1,2,3,4,5,6,7,4,3,5,
		   4,7,5,4,5,7,6,5,6,6,
			7,7};

	int postdiv2_list[22]={1,1,1,1,1,1,1,2,3,2,
		3,2,3,4,4,3,4,5,5,6,
			6,7};
	ret = 0;
	postdiv1 = 0;
	postdiv2 = 0;
	if (pixel_clock == 0)
		return -EINVAL;

	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	pixel_clock_ori = pixel_clock;

	if (pixel_clock_ori <= PERI_VOLTAGE_065V_CLK)
		pixel_clock_cur = pixel_clock * PPLL7_DIV_VOLTAGE_065V;
	else if (pixel_clock_ori <= PERI_VOLTAGE_070V_CLK)
		pixel_clock_cur = pixel_clock * PPLL7_DIV_VOLTAGE_070V;
	else if(pixel_clock_ori <= PERI_VOLTAGE_080V_CLK)
		pixel_clock_cur = pixel_clock * PPLL7_DIV_VOLTAGE_080V;
	else {
		HISI_FB_ERR("[DP] Clock don't support!!\n");
		return -EINVAL;
	}

	pixel_clock_cur = pixel_clock_cur / 1000;
	ceil_temp = dp_ceil(vco_min_freq_output, pixel_clock_cur);

	if (ceil_temp < 0)
		return -EINVAL;

	ppll7_freq_divider = (uint64_t)ceil_temp;/*lint !e571 */

	for (i = 0; i < 22; i++)
	{
		if(freq_divider_list[i] >= ppll7_freq_divider) {/*lint !e574 */
			ppll7_freq_divider = freq_divider_list[i];
			postdiv1 = postdiv1_list[i];
			postdiv2 = postdiv2_list[i];
			HISI_FB_INFO("[DP] postdiv1=0x%llx, POSTDIV2=0x%llx\n",postdiv1,postdiv2);/*lint !e559 */
			break;
		}
	}

	vco_freq_output = ppll7_freq_divider * pixel_clock_cur;
	if (vco_freq_output == 0)
		return -EINVAL;

	ceil_temp = dp_ceil(400000, vco_freq_output);

	if (ceil_temp < 0)
		return -EINVAL;

	refdiv=((vco_freq_output*ceil_temp) >= 494000) ? 1 : 2;
	HISI_FB_INFO("[DP] refdiv=0x%llx\n", refdiv);/*lint !e559 */

	fbdiv=(vco_freq_output * ceil_temp) * refdiv / sys_clock_fref;
	HISI_FB_INFO("[DP] fbdiv=0x%llx\n", fbdiv);/*lint !e559 */

	frac = (uint64_t)(ceil_temp * vco_freq_output - sys_clock_fref /refdiv*fbdiv) * refdiv * frac_range;
	frac = (uint64_t)frac / sys_clock_fref;
	HISI_FB_INFO("[DP] frac=0x%llx\n", frac);/*lint !e559 */

	ppll7ctrl0 = inp32(hisifd->pmctrl_base + MIDIA_PPLL7_CTRL0);
	ppll7ctrl0 &= ~MIDIA_PPLL7_FREQ_DEVIDER_MASK;/*lint !e648 */

	ppll7ctrl0_val = 0x0;
	ppll7ctrl0_val |= (uint32_t)(postdiv2 << 23 | postdiv1 << 20 | fbdiv << 8 | refdiv << 2);
	ppll7ctrl0_val &= MIDIA_PPLL7_FREQ_DEVIDER_MASK;/*lint !e648 */
	ppll7ctrl0 |= ppll7ctrl0_val;

	outp32(hisifd->pmctrl_base + MIDIA_PPLL7_CTRL0, ppll7ctrl0);

	ppll7ctrl1 = inp32(hisifd->pmctrl_base + MIDIA_PPLL7_CTRL1);
	ppll7ctrl1 &= ~MIDIA_PPLL7_FRAC_MODE_MASK;

	ppll7ctrl1_val = 0x0;
	ppll7ctrl1_val |= (uint32_t)(1 << 25 | 0 << 24 |frac);
	ppll7ctrl1_val &= MIDIA_PPLL7_FRAC_MODE_MASK;
	ppll7ctrl1 |= ppll7ctrl1_val;

	outp32(hisifd->pmctrl_base + MIDIA_PPLL7_CTRL1, ppll7ctrl1);

	/*comfirm ldi1 switch ppll7*/

	if (pixel_clock_ori <= PERI_VOLTAGE_065V_CLK)
		ret = clk_set_rate(hisifd->dss_pxl1_clk, DEFAULT_MIDIA_PPLL7_CLOCK_FREQ/PPLL7_DIV_VOLTAGE_065V);
	else if (pixel_clock_ori <= PERI_VOLTAGE_070V_CLK)
		ret = clk_set_rate(hisifd->dss_pxl1_clk, DEFAULT_MIDIA_PPLL7_CLOCK_FREQ/PPLL7_DIV_VOLTAGE_070V);
	else if(pixel_clock_ori <= PERI_VOLTAGE_080V_CLK)
		ret = clk_set_rate(hisifd->dss_pxl1_clk, DEFAULT_MIDIA_PPLL7_CLOCK_FREQ/PPLL7_DIV_VOLTAGE_080V);
	else {
		HISI_FB_ERR("[DP] Clock don't support!!\n");
		return -EINVAL;
	}

	if (ret < 0) {
		HISI_FB_ERR("[DP] fb%d dss_pxl1_clk clk_set_rate(%llu) failed, error=%d!\n",
			hisifd->index, pixel_clock_cur, ret);
	}
	return ret;
}

static int dp_clk_enable(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd;
	struct clk *clk_tmp = NULL;
	int ret = 0;

	if (pdev == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] fb%d, +.\n", hisifd->index);

	clk_tmp = hisifd->dss_auxclk_dpctrl_clk;
	if (clk_tmp) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_FB_ERR("[DP] fb%d dss_auxclk_dpctrl_clk clk_prepare failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_FB_ERR("[DP] fb%d dss_auxclk_dpctrl_clk clk_enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

	clk_tmp = hisifd->dss_pclk_dpctrl_clk;
	if (clk_tmp) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_FB_ERR("[DP] fb%d dss_pclk_dpctrl_clk clk_prepare failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_FB_ERR("[DP] fb%d dss_pclk_dpctrl_clk clk_enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

	clk_tmp = hisifd->dss_aclk_dpctrl_clk;
	if (clk_tmp) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_FB_ERR("[DP] fb%d dss_aclk_dpctrl_clk clk_prepare failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_FB_ERR("[DP] fb%d dss_aclk_dpctrl_clk clk_enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

	HISI_FB_INFO("[DP] fb%d, -.\n", hisifd->index);

	return 0;//lint !e438
}//lint !e550

static int dp_clk_disable(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd;
	struct clk *clk_tmp = NULL;

	if (pdev == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	clk_tmp = hisifd->dss_auxclk_dpctrl_clk;
	if (clk_tmp) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	clk_tmp = hisifd->dss_pclk_dpctrl_clk;
	if (clk_tmp) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	clk_tmp = hisifd->dss_aclk_dpctrl_clk;
	if (clk_tmp) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	return 0;
}

static int dp_clock_setup(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd;
	int ret;
	uint32_t default_aclk_dpctrl_rate;

	if (pdev == NULL) {
		HISI_FB_ERR("[DP] pdev NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	hisifd->dss_auxclk_dpctrl_clk = devm_clk_get(&pdev->dev, hisifd->dss_auxclk_dpctrl_name);
	if (IS_ERR(hisifd->dss_auxclk_dpctrl_clk)) {
		ret = PTR_ERR(hisifd->dss_auxclk_dpctrl_clk);//lint !e712
		HISI_FB_ERR("[DP] fb%d %s get fail ret = %d.\n",
			hisifd->index, hisifd->dss_auxclk_dpctrl_name, ret);
		return ret;
	} else {
		ret = clk_set_rate(hisifd->dss_auxclk_dpctrl_clk, DEFAULT_AUXCLK_DPCTRL_RATE);
		if (ret < 0) {
			HISI_FB_ERR("[DP] fb%d dss_auxclk_dpctrl_clk clk_set_rate(%lu) failed, error=%d!\n",
				hisifd->index, DEFAULT_AUXCLK_DPCTRL_RATE, ret);
			return -EINVAL;
		}

		HISI_FB_INFO("[DP] dss_auxclk_dpctrl_clk:[%lu]->[%lu].\n",
			DEFAULT_AUXCLK_DPCTRL_RATE, clk_get_rate(hisifd->dss_auxclk_dpctrl_clk));
	}

	default_aclk_dpctrl_rate = DEFAULT_ACLK_DPCTRL_RATE;

	hisifd->dss_aclk_dpctrl_clk = devm_clk_get(&pdev->dev, hisifd->dss_aclk_dpctrl_name);
	if (IS_ERR(hisifd->dss_aclk_dpctrl_clk)) {
		ret = PTR_ERR(hisifd->dss_aclk_dpctrl_clk);//lint !e712
		HISI_FB_ERR("[DP] fb%d dss_aclk_dpctrl_clk get fail ret = %d.\n",
			hisifd->index, ret);
		return ret;
	} else {
		ret = clk_set_rate(hisifd->dss_aclk_dpctrl_clk, default_aclk_dpctrl_rate);
		if (ret < 0) {
			HISI_FB_ERR("[DP] fb%d dss_aclk_dpctrl_clk clk_set_rate(%lu) failed, error=%d!\n",/*lint !e559 */
				hisifd->index, default_aclk_dpctrl_rate, ret);/*lint !e559 */
			return -EINVAL;
		}

		HISI_FB_INFO("[DP] dss_aclk_dpctrl_clk:[%lu]->[%lu].\n",/*lint !e559 */
			default_aclk_dpctrl_rate, clk_get_rate(hisifd->dss_aclk_dpctrl_clk));/*lint !e559 */
	}

	hisifd->dss_pclk_dpctrl_clk = devm_clk_get(&pdev->dev, hisifd->dss_pclk_dpctrl_name);
	if (IS_ERR(hisifd->dss_pclk_dpctrl_clk)) {
		ret = PTR_ERR(hisifd->dss_pclk_dpctrl_clk);//lint !e712
		HISI_FB_ERR("[DP] fb%d dss_pclk_dpctrl_clk get fail ret = %d.\n",
			hisifd->index, ret);
		return ret;
	}

	return 0;
}

static int dp_on(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;

	if (pdev == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] fb%d, +.\n", hisifd->index);

	dptx = &(hisifd->dp);

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	mutex_lock(&dptx->dptx_mutex);

	if (dptx->dptx_enable) {
		HISI_FB_ERR("[DP] dptx has already on.\n");
		mutex_unlock(&dptx->dptx_mutex);
		return 0;
	}

	/* dp dis reset */
	outp32(hisifd->peri_crg_base + PERRSTDIS0, 0x00000400);
	ret = dp_clk_enable(pdev);
	if (ret) {
		HISI_FB_ERR("[DP] DPTX dp clock enable failed !!!\n");
		ret = -ENODEV;
		goto err_out;
	}

	if (!dptx_check_dptx_id(dptx)) {
		HISI_FB_ERR("[DP] DPTX_ID not match to 0x%04x:0x%04x!\n",
			DPTX_ID_DEVICE_ID, DPTX_ID_VENDOR_ID);
		ret = -ENODEV;
		goto err_out;
	}

	ret = dptx_core_init(dptx);
	if (ret) {
		HISI_FB_ERR("[DP] DPTX core init failed!\n");
		ret = -ENODEV;
		goto err_out;
	}

	/* FIXME: clear intr */
	dptx_global_intr_dis(dptx);
	/*dptx_global_intr_clear(dptx);*/
	enable_irq(dptx->irq);
	/* Enable all top-level interrupts */
	dptx_global_intr_en(dptx);

	if(hisifd->secure_ctrl.hdcp_dpc_sec_en) {
		esm_driver_enable(1);
	}
	HDCP_DP_on(1);

	dptx->dptx_enable = true;
	dptx->detect_times = 0;
	dptx->current_link_rate = dptx->max_rate;
	dptx->current_link_lanes = dptx->max_lanes;
	bpress_powerkey = false;
	btrigger_timeout = false;

	ret = panel_next_on(pdev);
	if (ret) {
		HISI_FB_ERR("[DP] panel_next_on failed!\n");
	}

err_out:
	mutex_unlock(&dptx->dptx_mutex);

	HISI_FB_INFO("[DP] fb%d, -.\n", hisifd->index);

	return ret;
}

static int dp_off(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;
	uint32_t hdcp_state = 0;
	int i = 0;

	if (pdev == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] fb%d, +.\n", hisifd->index);

	dptx = &(hisifd->dp);

	mutex_lock(&dptx->dptx_mutex);

	if (!dptx->dptx_enable) {
		HISI_FB_INFO("[DP] dptx has already off.\n");
		mutex_unlock(&dptx->dptx_mutex);
		return 0;
	}

	if(hisifd->secure_ctrl.hdcp_dpc_sec_en) {
		esm_driver_enable(0);
	}
	HDCP_DP_on(0);

	/* FIXME: */
	if (dptx->video_transfer_enable) {
		handle_hotunplug(hisifd);
		mdelay(10);
	}

	get_hdcp_state(&hdcp_state);
	if(hdcp_state == 1) {
		HISI_FB_INFO("[DP] The hdcp is processing, need to wait\n");
		while(i < 100) {
			msleep(10);
			get_hdcp_state(&hdcp_state);
			if(hdcp_state == 0)
				break;
			i++;
		}
		HISI_FB_ERR("[DP] wait 1s,and go on\n");
	}
	HISI_FB_INFO("[DP] wait count = %d\n", i);

	/* FIXME: clear intr */
	dptx_global_intr_dis(dptx);
	disable_irq_nosync(dptx->irq);

	dptx_core_deinit(dptx);
	dptx_aux_disreset(dptx, false);
	dp_clk_disable(pdev);

	/* dp reset */
	outp32(hisifd->peri_crg_base + PERRSTEN0, 0x00000400);

	dptx->detect_times = 0;
	dptx->dptx_vr = false;
	dptx->dptx_enable = false;
	dptx->video_transfer_enable = false;
	dptx->dptx_plug_type = DP_PLUG_TYPE_NORMAL;
	bpress_powerkey = false;
	btrigger_timeout = false;

	ret = panel_next_off(pdev);
	if (ret) {
		HISI_FB_ERR("[DP] Panel DP next off error !!\n");
	}

	mutex_unlock(&dptx->dptx_mutex);

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);
	return ret;
}

static int dp_resume(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;

	if (pdev == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] fb%d, +.\n", hisifd->index);

	dptx = &(hisifd->dp);

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (dptx->dptx_enable) {
		dp_off(g_dp_pdev);
		msleep(10);
		dp_on(g_dp_pdev);
	}

	HISI_FB_INFO("[DP] fb%d, -.\n", hisifd->index);

	return 0;
}

void dp_send_cable_notification(struct dp_ctrl *dptx, int val)
{
	int state = 0;
	struct dtd *mdtd;
	struct video_params *vparams;

	if (!dptx) {
		HISI_FB_ERR("[DP] dptx is NULL!\n");
		return;
	}

	state = dptx->sdev.state;
	switch_set_state(&dptx->sdev, val);
	if (dptx->edid_info.Audio.basicAudio == 0x1) {
		if (val == Hot_Plug_OUT || val == Hot_Plug_OUT_VR) {
			switch_set_state(&dptx->dp_switch, 0);
		} else if (val == Hot_Plug_IN || val == Hot_Plug_IN_VR) {
			switch_set_state(&dptx->dp_switch, 1);
		}
	} else {
		HISI_FB_WARNING("[DP] basicAudio(%ud) no support!\n", dptx->edid_info.Audio.basicAudio);
	}

	vparams = &(dptx->vparams);
	mdtd = &(dptx->vparams.mdtd);

	update_external_display_timming_info(mdtd->h_active, mdtd->v_active, vparams->m_fps);

	HISI_FB_INFO("[DP] cable state %s %d\n",
		dptx->sdev.state == state ? "is same" : "switched to", dptx->sdev.state);
}

int dp_device_srs(struct hisi_fb_data_type *hisifd, bool ublank)
{
	struct dp_ctrl *dptx;
	struct hisi_panel_info *pinfo;

	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	dptx = &(hisifd->dp);
	pinfo = &(hisifd->panel_info);

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] dptx is NULL!\n");
		return -EINVAL;
	}
	HISI_FB_INFO("[DP] fb%d, +.\n", hisifd->index);

	mutex_lock(&dptx->dptx_mutex);

	if (dptx->dptx_enable && dptx->video_transfer_enable) {
		if (ublank) {
			if (bpress_powerkey) {
				int retval = 0;
				dptx_write_dpcd(dptx, DP_SET_POWER, DP_SET_POWER_D0);
				mdelay(10);
				retval = dptx_link_retraining(dptx, dptx->current_link_rate, dptx->current_link_lanes);
				if (retval < 0) {
					dp_imonitor_set_param(DP_PARAM_LINK_RETRAINING_FAILED, &retval);
				}
				dptx_enable_default_video_stream(dptx);
				bpress_powerkey = false;
				HISI_FB_INFO("[DP] Retraining when blank on. \n");
			}
		} else {
			dptx_write_dpcd(dptx, DP_SET_POWER, DP_SET_POWER_D3);
			dptx_disable_default_video_stream(dptx);
			bpress_powerkey = true;
			HISI_FB_INFO("[DP] Disable stream when blank off. \n");
		}
	} else {
		HISI_FB_INFO("[DP] SRS never excute! \n");
		mutex_unlock(&dptx->dptx_mutex);
		HISI_FB_INFO("[DP] fb%d, -.\n", hisifd->index);
		return 0;
	}

	mutex_unlock(&dptx->dptx_mutex);
	HISI_FB_INFO("[DP] fb%d, -.\n", hisifd->index);
	return 0;
}

int dp_get_color_bit_mode(struct hisi_fb_data_type *hisifd, void __user *argp)
{
	struct dp_ctrl *dptx;
	int ret;

	if (argp == NULL) {
		return -EINVAL;
	}

	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	dptx = &(hisifd->dp);

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] dptx is NULL!\n");
		return -EINVAL;
	}

	ret = (int)copy_to_user(argp, &(dptx->vparams.bpc), sizeof(dptx->vparams.bpc));
	if (ret) {
		HISI_FB_ERR("[DP]  copy_to_user failed! ret=%d.\n", ret);
		return ret;
	}

	return ret;
}

int dp_wakeup(struct hisi_fb_data_type *hisifd)
{
	struct dp_ctrl *dptx;

	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	dptx = &(hisifd->dp);

	if (!dptx) {
		HISI_FB_ERR("[DP] dptx is NULL!\n");
		return -EINVAL;
	}

	if (dptx->dptx_enable) {
		wake_up_interruptible(&dptx->dptxq);
	}

	return 0;
}
/*******************************************************************************
**
*/
int hisi_dptx_main_panel_blank(bool bblank)
{
	int ret;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;

	if (g_dp_pdev == NULL) {
		HISI_FB_ERR("[DP] g_dp_pdev is NULL!\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(g_dp_pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] fb%d, +.\n", hisifd->index);

	dptx = &(hisifd->dp);
	ret = 0;

	mutex_lock(&dptx->dptx_mutex);

	if (!dptx->dptx_enable) {
		HISI_FB_ERR("[DP] dptx has already off.\n");
		ret = -EINVAL;
		goto fail;
	}

	if (!dptx->video_transfer_enable) {
		HISI_FB_ERR("[DP] dptx never transfer video.\n");
		ret = -EINVAL;
		goto fail;
	}

	if(bblank) {
		if (btrigger_timeout) {
			dp_send_cable_notification(dptx, Hot_Plug_MAINPANEL_UP);
			btrigger_timeout = false;
		} else {
			HISI_FB_ERR("[DP] dptx receive repeat CMD to power on the main panel !!!\n");
			ret = -1;
		}
	} else {
		if (!btrigger_timeout) {
			dp_send_cable_notification(dptx, Hot_Plug_MAINPANEL_DOWN);
			btrigger_timeout = true;
		} else {
			HISI_FB_ERR("[DP] dptx receive repeat CMD to power down the main panel !!!\n");
			ret = -1;
		}
	}

fail:
	mutex_unlock(&dptx->dptx_mutex);
	HISI_FB_INFO("[DP] fb%d, -.\n", hisifd->index);

	return ret;
}

EXPORT_SYMBOL_GPL(hisi_dptx_main_panel_blank);

int hisi_dptx_switch_source(uint32_t user_mode, uint32_t user_format)
{
	int ret;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;
	struct hisi_panel_info *pinfo;

	if (g_dp_pdev == NULL) {
		HISI_FB_ERR("[DP] g_dp_pdev is NULL!\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(g_dp_pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] fb%d, +.\n", hisifd->index);

	dptx = &(hisifd->dp);
	ret = 0;

	mutex_lock(&dptx->dptx_mutex);

	if (!dptx->dptx_enable) {
		HISI_FB_ERR("[DP] dptx has already off.\n");
		ret = -EINVAL;
		goto fail;
	}

	if (!dptx->video_transfer_enable) {
		HISI_FB_ERR("[DP] dptx never transfer video.\n");
		ret = -EINVAL;
		goto fail;
	}

	if ((dptx->same_source == get_current_dp_source_mode()) && (!user_mode) && (!user_format)) {
		HISI_FB_ERR("[DP] dptx don't switch source when the dest mode is same as current!!!\n");
		ret = -EINVAL;
		goto fail;
	}

	dptx->user_mode = user_mode;
	dptx->user_mode_format = (enum video_format_type) user_format;
	dptx->same_source = get_current_dp_source_mode();

	HISI_FB_INFO("[DP] dptx user switch: mode (%d); format (%d); same_source (%d).\n",
		dptx->user_mode, dptx->user_mode_format, dptx->same_source);

	pinfo = &(hisifd->panel_info);

	/*PC mode change to 1080p*/
	if (((pinfo->xres >= MAX_DIFF_SOURCE_X_SIZE) && (dptx->max_edid_timing_hactive > MAX_DIFF_SOURCE_X_SIZE))
		|| (dptx->user_mode != 0)) {
		/* DP plug out */
		handle_hotunplug(hisifd);
		mutex_unlock(&dptx->dptx_mutex);

		msleep(10);

		mutex_lock(&dptx->dptx_mutex);
		ret = handle_hotplug(hisifd);
		dp_imonitor_set_param(DP_PARAM_HOTPLUG_RETVAL, &ret);
	}
fail:
	mutex_unlock(&dptx->dptx_mutex);

	HISI_FB_INFO("[DP] fb%d, -.\n", hisifd->index);

	return ret;
}

EXPORT_SYMBOL_GPL(hisi_dptx_switch_source);

int hisi_dptx_hpd_trigger(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode, TYPEC_PLUG_ORIEN_E typec_orien)
{
	int ret;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;
	uint8_t dp_lanes = 0;

	if (g_dp_pdev == NULL) {
		HISI_FB_ERR("[DP] g_dp_pdev is NULL!\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(g_dp_pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] fb%d, +.\n", hisifd->index);

	dptx = &(hisifd->dp);
	ret = 0;

	mutex_lock(&dptx->dptx_mutex);

	if (!dptx->dptx_enable) {
		HISI_FB_ERR("[DP] dptx has already off.\n");
		ret = -EINVAL;
		goto fail;
	}

	dptx->same_source = get_current_dp_source_mode();
	dptx->user_mode = 0;
	dptx->dptx_plug_type = typec_orien;
	dptx->user_mode_format = VCEA;
	HISI_FB_INFO("[DP] DP HPD Type:(%d), Mode:(%d), Gate:(%d)\n", irq_type, mode, dptx->dptx_gate);

	/* DP HPD event must be delayed when system is booting*/
	if(!dptx->dptx_gate) {
		wait_event_interruptible_timeout(dptx->dptxq, dptx->dptx_gate, msecs_to_jiffies(20000)); /*lint !e666 */
	}

	switch (mode) {
	case TCPC_DP:
		dp_lanes = 4;
		break;
	case TCPC_USB31_AND_DP_2LINE:
		dp_lanes = 2;
		break;
	default:
		HISI_FB_ERR("[DP] not supported tcpc_mux_ctrl_type=%d.\n", mode);
		ret = -EINVAL;
		goto fail;
	}

	switch (irq_type) {
	case TCA_IRQ_HPD_OUT:
		dptx_hpd_handler(dptx, false, dp_lanes);
		break;
	case TCA_IRQ_HPD_IN:
		dptx_hpd_handler(dptx, true, dp_lanes);
		break;
	case TCA_IRQ_SHORT:
		dptx_hpd_irq_handler(dptx);
		break;
	default:
		HISI_FB_ERR("[DP] not supported tca_irq_type=%d.\n", irq_type);
		ret = -EINVAL;
		goto fail;
	}

fail:
	mutex_unlock(&dptx->dptx_mutex);

	HISI_FB_INFO("[DP] fb%d, -.\n", hisifd->index);

	return ret;
}
EXPORT_SYMBOL_GPL(hisi_dptx_hpd_trigger);


int hisi_dptx_triger(bool enable)
{
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;
	int ret;

	if (g_dp_pdev == NULL) {
		HISI_FB_ERR("[DP] g_dp_pdev is NULL!\n");
		/* return 0 to ensure USB flow when DP is absent.*/
		return 0;
	}

	hisifd = platform_get_drvdata(g_dp_pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] fb%d, +. DP Device %s\n", hisifd->index, enable ? "ON" : "OFF");

	dptx = &(hisifd->dp);

	if (enable) {
		ret = dp_on(g_dp_pdev);
		if (ret) {
			HISI_FB_ERR("[DP] dp_on failed!\n");
		}
	} else {
		ret = dp_off(g_dp_pdev);
		if (ret) {
			HISI_FB_ERR("[DP] dp_off failed!\n");
		}
	}

	HISI_FB_INFO("[DP] fb%d, -.\n", hisifd->index);

	return ret;
}
EXPORT_SYMBOL_GPL(hisi_dptx_triger);

int hisi_dptx_notify_switch(void)
{
	int ret;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;
	bool lanes_status_change = false;

	if (g_dp_pdev == NULL) {
		HISI_FB_ERR("[DP] g_dp_pdev is NULL!\n");
		/* return 0 to ensure USB flow when DP is absent.*/
		return 0;
	}

	hisifd = platform_get_drvdata(g_dp_pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return -EINVAL;
	}

	dptx = &(hisifd->dp);
	HISI_FB_INFO("[DP] fb%d, + [DP] Status: %d.\n", hisifd->index, dptx->dptx_enable);
	mutex_lock(&dptx->dptx_mutex);

	if (dptx->dptx_enable) {
		lanes_status_change = true;
		dptx_disable_default_video_stream(dptx);
		dptx_typec_reset_ack(dptx);
		dptx_phy_set_lanes_status(dptx, false);
	}

	mutex_unlock(&dptx->dptx_mutex);

	if (!lanes_status_change) {
		ret = dp_on(g_dp_pdev);
		if (ret) {
			HISI_FB_ERR("[DP] DP on failed!\n");
			return -EINVAL;
		}
	}

	HISI_FB_INFO("[DP] fb%d, -.\n", hisifd->index);

	return 0;
}
EXPORT_SYMBOL_GPL(hisi_dptx_notify_switch);

bool hisi_dptx_ready(void)
{
	return (g_dp_pdev != NULL);
}
EXPORT_SYMBOL_GPL(hisi_dptx_ready);

/*******************************************************************************
**
*/
static int dp_device_init(struct platform_device *pdev)
{
	struct dp_ctrl *dptx;
	struct hisi_fb_data_type *hisifd;
	struct device_node *np = NULL;
	int i, ret;

	if (pdev == NULL) {
		HISI_FB_ERR("[DP] pdev NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] fb%d +.\n", hisifd->index);

	ret = dp_clock_setup(pdev);
	if (ret) {
		HISI_FB_ERR("[DP] DP clock setup failed!\n");
		return ret;
	}

	dptx = &(hisifd->dp);

	dptx->dev = &pdev->dev;
	dptx->base = hisifd->dp_base;
	dptx->hisifd = hisifd;
	if (IS_ERR(dptx->base)) {
		HISI_FB_ERR("[DP] dptx base is NULL!\n");
		return -EINVAL;
	}

	dptx->irq = hisifd->dp_irq;
	if (!dptx->irq) {
		HISI_FB_ERR("[DP] dptx irq is NULL!\n");
		return -EINVAL;
	}

	mutex_init(&dptx->dptx_mutex);
	init_waitqueue_head(&dptx->dptxq);
	init_waitqueue_head(&dptx->waitq);
	atomic_set(&(dptx->sink_request), 0);
	atomic_set(&(dptx->shutdown), 0);
	atomic_set(&(dptx->c_connect), 0);

	if (g_fpga_flag == 1) {
		dptx->max_rate = DPTX_PHYIF_CTRL_RATE_RBR;
	} else {
		dptx->max_rate = DPTX_PHYIF_CTRL_RATE_HBR2;
	}
	dptx->max_lanes = 4;
	dptx->dptx_vr = false;
	dptx->dptx_gate = false;
	dptx->dptx_enable = false;
	dptx->video_transfer_enable = false;
	dptx->dptx_detect_inited = false;
	dptx->user_mode = 0;
	dptx->detect_times = 0;
	dptx->dptx_plug_type = DP_PLUG_TYPE_NORMAL;
	dptx->user_mode_format = VCEA;
	dptx->same_source = get_current_dp_source_mode();
	dptx->max_edid_timing_hactive = 0;
	memset(&(dptx->edid_info), 0, sizeof(struct edid_information));

	dptx_video_params_reset(&dptx->vparams);
	dptx_audio_params_reset(&dptx->aparams);

	dptx->edid = kzalloc(DPTX_DEFAULT_EDID_BUFLEN, GFP_KERNEL);
	if (!dptx->edid) {
		HISI_FB_ERR("[DP] dptx base is NULL!\n");
		return -ENOMEM;
	}
	memset(dptx->edid, 0, DPTX_DEFAULT_EDID_BUFLEN);

	dptx->sdev.name = "hisi-dp";
	if (switch_dev_register(&dptx->sdev) < 0) {
		HISI_FB_ERR("[DP] dp switch registration failed!\n");
		ret = -ENODEV;
		goto err_edid_alloc;
	}

	dptx->dp_switch.name = "hdmi_audio";
	ret = switch_dev_register(&dptx->dp_switch);
	if (ret < 0) {
		HISI_FB_ERR("[DP] hdmi_audio switch device register error %d\n", ret);
		goto err_sdev_register;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_SWING_VALUE);
	if (!np) {
		HISI_FB_ERR("[DP] NOT FOUND device node %s!\n", DTS_COMP_SWING_VALUE);
		return -ENOMEM;
	}
	for(i = 0; i < DPTX_COMBOPHY_PARAM_NUM; i++)
	{
		ret = of_property_read_u32_index(np, "preemphasis_swing", i, &(dptx->combophy_pree_swing[i]));
		if(ret) {
			HISI_FB_ERR("[DP] preemphasis_swing[%d] is got fail\n", i);
			return -ENOMEM;
		}
	}

	ret = devm_request_threaded_irq(&(pdev->dev),
		dptx->irq, dptx_irq, dptx_threaded_irq,
		IRQF_SHARED | IRQ_LEVEL, "dwc_dptx", (void *)hisifd);//lint !e747
	if (ret) {
		HISI_FB_ERR("[DP] Request for irq %d failed!\n", dptx->irq);
		goto err_sdev_hdmi_register;
	} else {
		disable_irq(dptx->irq);
	}

	dp_debug_init_combophy_pree_swing(dptx->combophy_pree_swing, DPTX_COMBOPHY_PARAM_NUM);

	hisifd->dp_device_srs = dp_device_srs;
	hisifd->dp_get_color_bit_mode = dp_get_color_bit_mode;
	hisifd->dp_pxl_ppll7_init = dp_pxl_ppll7_init;
	hisifd->dp_wakeup = dp_wakeup;
	HISI_FB_INFO("[DP] fb%d -.\n", hisifd->index);

	return 0;
err_sdev_hdmi_register: /*lint !e563 */
	switch_dev_unregister(&dptx->dp_switch);

err_sdev_register: /*lint !e563 */
	switch_dev_unregister(&dptx->sdev);

err_edid_alloc: /*lint !e563 */
	if (dptx->edid != NULL) {
		kfree(dptx->edid);
		dptx->edid = NULL;
	}

	return ret;
}

static int dp_remove(struct platform_device *pdev)
{
	int ret;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;

	if (pdev == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] fb%d, +.\n", hisifd->index);

	dptx = &(hisifd->dp);
	switch_dev_unregister(&dptx->sdev);

	if (dptx->edid != NULL) {
		kfree(dptx->edid);
		dptx->edid = NULL;
	}

	dptx->dptx_vr = false;
	dptx->dptx_enable = false;
	dptx->video_transfer_enable = false;
	dptx_core_deinit(dptx);

	dptx_notify_shutdown(dptx);
	mdelay(15); //lint !e778 !e747 !e774 !e845

	ret = panel_next_remove(pdev);


	g_dp_pdev = NULL;

	HISI_FB_INFO("[DP] fb%d, -.\n", hisifd->index);

	return ret;
}

static int dp_probe(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd;
	struct platform_device *dpp_dev;
	struct hisi_fb_panel_data *pdata;
	int ret;

	if (pdev == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		dev_err(&pdev->dev, "NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] fb%d, +.\n", hisifd->index);

	ret = dp_device_init(pdev);
	if (ret) {
		dev_err(&pdev->dev, "fb%d mipi_dsi_irq_clk_setup failed, error=%d!\n", hisifd->index, ret);
		return -EINVAL;
	}


	/* alloc device */
	dpp_dev = platform_device_alloc(DEV_NAME_DSS_DPE, pdev->id);
	if (!dpp_dev) {
		dev_err(&pdev->dev, "fb%d platform_device_alloc failed, error=%d!\n", hisifd->index, ret);
		ret = -ENOMEM;
		goto err_device_alloc;
	}

	/* link to the latest pdev */
	hisifd->pdev = dpp_dev;

	/* alloc panel device data */
	ret = platform_device_add_data(dpp_dev, dev_get_platdata(&pdev->dev),
		sizeof(struct hisi_fb_panel_data));
	if (ret) {
		dev_err(&pdev->dev, "fb%d platform_device_add_data failed error=%d!\n", hisifd->index, ret);
		ret = -EINVAL;
		goto err_device_put;
	}

	/* data chain */
	pdata = dev_get_platdata(&dpp_dev->dev);
	pdata->on = NULL;
	pdata->off = NULL;
	pdata->remove = dp_remove;
	pdata->next = pdev;

	/* get/set panel info */
	memcpy(&hisifd->panel_info, pdata->panel_info, sizeof(struct hisi_panel_info));

	/* set driver data */
	platform_set_drvdata(dpp_dev, hisifd);
	/* device add */
	ret = platform_device_add(dpp_dev);
	if (ret) {
		dev_err(&pdev->dev, "fb%d platform_device_add failed, error=%d!\n", hisifd->index, ret);
		ret = -EINVAL;
		goto err_device_put;
	}

	g_dp_pdev = pdev;

	HISI_FB_INFO("[DP] fb%d, -.\n", hisifd->index);

	if (g_fpga_flag == 1) {
		dp_on(pdev);
	}
	return 0;

err_device_put:
	platform_device_put(dpp_dev);
err_device_alloc:
	return ret;
}

/*lint -save -e* */
static struct platform_driver this_driver = {
	.probe = dp_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = dp_resume,
	.shutdown = NULL,
	.driver = {
		.name = DEV_NAME_DP,
	},
};
/*lint -restore*/

static int __init dp_driver_init(void)
{
	int ret;

	ret = platform_driver_register(&this_driver);//lint !e64

	if (ret) {
		HISI_FB_ERR("[DP] platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

/*lint -e528 -esym(528,*)*/
module_init(dp_driver_init);
/*lint -e528 +esym(528,*)*/
