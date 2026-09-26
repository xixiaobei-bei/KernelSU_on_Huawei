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
#include <soc_media1_crg_interface.h>
#include <soc_actrl_interface.h>
#include <soc_pmctrl_interface.h>
#include <soc_smmuv3_tbu_interface.h>
#include <linux/hisi/hisi_drmdriver.h>
#include <linux/hisi-iommu.h>

#include "hisi_drm_dpe_utils.h"
#include "hisi_drm_smmuv3.h"

static bool g_smmuv3_inited = false;
static int g_tbu_sr_refcount;
static int g_tbu0_cnt_refcount;
static bool offline_tbu1_conected = false;
static struct mutex g_tbu_sr_lock;
typedef bool (*smmu_event_cmp)(uint32_t value, uint32_t expect_value);

bool hisi_smmu_connect_event_cmp(u32 value, u32 expect_value)
{
	if (((value & 0x3) != 0x3) || (((value >> 8) & 0x7f) < expect_value))
		return false;

	return true;
}

bool hisi_smmu_disconnect_event_cmp(u32 value, u32 expect_value)
{
	UNUSED(expect_value);
	if ((value & 0x3) != 0x1)
		return false;

	return true;
}

void hisifb_atf_config_security(u32 master_op_type, u32 channel, u32 mode)
{

	configure_dss_service_security(master_op_type, channel, mode);

}
static void hisi_smmu_event_request(char __iomem *smmu_base,
	smmu_event_cmp cmp_func, enum smmu_event event, u32 check_value)
{
	u32 smmu_tbu_crack;
	u32 delay_count = 0;
	dump_stack();
	if (!smmu_base) {
		HISI_DRM_ERR("smmu_base is nullptr\n");
		return;
	}

	/* request event config */
	set_reg(SOC_SMMUv3_TBU_SMMU_TBU_CR_ADDR(smmu_base), event, 1, 0);

	/* check ack */
	do {
		smmu_tbu_crack = inp32(SOC_SMMUv3_TBU_SMMU_TBU_CRACK_ADDR(smmu_base));
		if (cmp_func(smmu_tbu_crack, check_value))
			break;

		udelay(1);
		delay_count++;
	} while (delay_count < SMMU_TIMEOUT);

	if (delay_count == SMMU_TIMEOUT)
		HISI_DRM_ERR("smmu=0x%x event=%d smmu_tbu_crack=0x%x check_value=0x%x!\n",
			smmu_base, event, smmu_tbu_crack, check_value);
}

void hisi_drm_smmuv3_on(struct hisi_drm_crtc *hisi_crtc)
{
	char __iomem *tbu_base = NULL;
	int ret;
	struct dss_hw_ctx *ctx = NULL;
	HISI_DRM_INFO("+\n");

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is nullptr\n");
		return;
	}

	if (!hisi_crtc->ctx) {
		HISI_DRM_ERR("hisi_crtc->ctx is nullptr\n");
		return;
	}

	ctx = hisi_crtc->ctx;

	mutex_lock(&g_tbu_sr_lock);
	if (g_tbu_sr_refcount == 0) {
		ret = regulator_enable(ctx->dpe_regulator[DPE_REGULATOR_SMMU].consumer);
		if (ret)
			HISI_DRM_ERR("fb%u smmu tcu regulator_enable failed, error=%d!\n", hisi_crtc->crtc_id, ret);
	}

	if (g_tbu0_cnt_refcount == 0) {
		hisifb_atf_config_security(DSS_SMMU_INIT, 0, ONLINE_COMPOSE_MODE);
		tbu_base = ctx->dss_base + DSS_SMMU_OFFSET;
		hisi_smmu_event_request(tbu_base, hisi_smmu_connect_event_cmp,
			TBU_CONNECT, DSS_TBU0_DTI_NUMS);

	}
	g_tbu0_cnt_refcount++;
	g_tbu_sr_refcount++;

	HISI_DRM_DEBUG("fb%u tbu_sr_refcount=%d, tbu0_cnt_refcount=%d, offline_tbu1_conected=%d",
		hisi_crtc->crtc_id, g_tbu_sr_refcount, g_tbu0_cnt_refcount, offline_tbu1_conected);

	mutex_unlock(&g_tbu_sr_lock);
	HISI_DRM_INFO("-\n");
}


int hisi_drm_smmuv3_off(struct hisi_drm_crtc *hisi_crtc)
{
	char __iomem *tbu_base = NULL;
	int ret = 0;
	struct dss_hw_ctx *ctx = NULL;
	HISI_DRM_INFO("+\n");

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is nullptr\n");
		return -EINVAL;
	}

	if (!hisi_crtc->ctx) {
		HISI_DRM_ERR("hisi_crtc->ctx is nullptr\n");
		return -EINVAL;
	}
	ctx = hisi_crtc->ctx;
	mutex_lock(&g_tbu_sr_lock);
	g_tbu_sr_refcount--;
	g_tbu0_cnt_refcount--;
	if (g_tbu0_cnt_refcount == 0) {
		/* keep the connection if tbu0 is used by fb2 cmdlist */
		tbu_base = ctx->dss_base + DSS_SMMU_OFFSET;
		hisi_smmu_event_request(tbu_base, hisi_smmu_disconnect_event_cmp, TBU_DISCONNECT, 0);
	}

	if (g_tbu_sr_refcount == 0) {
		ret = regulator_disable(ctx->dpe_regulator[DPE_REGULATOR_SMMU].consumer);
		if (ret)
			HISI_DRM_ERR("fb%u smmu tcu regulator_disable failed, error=%d!\n", hisi_crtc->crtc_id, ret);
	}
	HISI_DRM_DEBUG("fb%u tbu_sr_refcount=%d, tbu0_cnt_refcount=%d, offline_tbu1_conected=%d",
		hisi_crtc->crtc_id, g_tbu_sr_refcount, g_tbu0_cnt_refcount, offline_tbu1_conected);

	mutex_unlock(&g_tbu_sr_lock);
	HISI_DRM_INFO("-\n");

	return ret;
}

int hisi_drm_smmu_ch_config(struct dss_module_reg *dss_module, int chn_idx,
	bool mmu_enable, uint32_t frame_no)
{
	dss_smmu_t *smmu = NULL;
	u32 ssid;
	soc_dss_mmu_id_attr taget;

	if (!dss_module) {
		HISI_DRM_ERR("dss_module is nullptr\n");
		return -EINVAL;
	}

	smmu = &dss_module->smmu;
	dss_module->smmu_used = 1;

	taget.value = 0;
	taget.reg.ch_sid = mmu_enable ? 0x1 : 0x3F;
	taget.reg.ch_ssidv = 1;

	ssid = hisi_dss_get_ssid(frame_no);
	taget.reg.ch_ssid = ssid + HISI_PRIMARY_DSS_SSID_OFFSET;

	smmu->ch_mmu_attr[chn_idx].value = taget.value;

	HISI_DRM_INFO("config ch_mmu_attr[%d].value=0x%x, ssid=%u\n",
		chn_idx, smmu->ch_mmu_attr[chn_idx].value, taget.reg.ch_ssid);

	return 0;
}

void hisi_drm_smmu_ch_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *smmu_base, struct dss_smmu *s_smmu, int chn_idx)
{
	u32 idx = 0;
	u32 i = 0;

	char __iomem *addr = NULL;

	drm_check_and_void_return(hisi_crtc == NULL, "hisi_crtc is NULL");
	drm_check_and_void_return(smmu_base == NULL, "smmu_base is NULL");
	drm_check_and_void_return(s_smmu == NULL, "s_smmu is NULL");

	if (chn_idx == DSS_RCHN_V2)
		addr = hisi_crtc->ctx->dss_base + DSS_VBIF0_AIF + MMU_ID_ATTR_NS_11;
	else
		addr = hisi_crtc->ctx->dss_base + DSS_VBIF0_AIF + MMU_ID_ATTR_NS_0 + chn_idx * 0x4;

	hisi_set_reg(hisi_crtc, addr, s_smmu->ch_mmu_attr[chn_idx].value, 32, 0);

	HISI_DRM_INFO("config ch_mmu_attr[%d].value=0x%x, ssid=%u\n",
		chn_idx, s_smmu->ch_mmu_attr[chn_idx].value,
		s_smmu->ch_mmu_attr[chn_idx].reg.ch_ssid);

}

extern struct device *g_smmu_dev;

void hisi_dss_mmu_tlb_flush(struct hisi_drm_crtc *hisi_crtc, u32 frame_no)
{
	u32 ssid;

	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is nullptr\n");
		return;
	}

	ssid = hisi_dss_get_ssid(frame_no);
	if (hisi_crtc->crtc_id == PRIMARY_IDX)
		ssid = ssid + HISI_PRIMARY_DSS_SSID_OFFSET;
	else if (hisi_crtc->crtc_id == EXTERNAL_IDX)
		ssid = ssid + HISI_EXTERNAL_DSS_SSID_OFFSET;
	else
		ssid = HISI_OFFLINE_SSID;

	hisi_iommu_dev_flush_tlb(g_smmu_dev, ssid);
	HISI_DRM_DEBUG("hisi_iommu_dev_flush_tlb ssid=%u\n", ssid);
}

void hisi_dss_smmuv3_init(void)
{
	if (!g_smmuv3_inited) {
		mutex_init(&g_tbu_sr_lock);
		g_smmuv3_inited = true;
	}
}

