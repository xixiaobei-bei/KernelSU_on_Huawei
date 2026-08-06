/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hdcp_reg.h"
#include "esm/host_lib_driver_linux_if.h"


/* hdcp reg definition */
#define HDCP_REG_RMLCTL  0x3614
#define HDCP_REG_RMLSTS  0x3618
#define HDCP_REG_DPK_CRC 0x3630
#define HDCP_REG_DPK0    0x3620
#define HDCP_REG_DPK1    0x3624
#define HDCP_REG_SEED    0x361C
#define HDCP_API_INT_MSK 0xE10
#define HDCP_INT_STS     0xE0C
#define HDCP_INT_CLR     0xE08
#define HDCP_OBS         0xE04
#define HDCP_CFG         0xE00

#define HDCP_CFG_DPCD_PLUS_SHIFT  7
#define HDCP_CFG_DPCD_PLUS_MASK   BIT(7)
#define HDCP_KSV_ACCESS_INT       BIT(0)
#define HDCP_AUX_RESP_TIMEOUT     BIT(3)
#define HDCP_KSV_SHA1             BIT(5)
#define HDCP_FAILED               BIT(6)
#define HDCP_ENGAGED              BIT(7)
#define HDCP22_GPIOINT            BIT(8)
#define HDCP22_BOOTED             BIT(23)
#define HDCP22_STATE              (BIT(24)|BIT(25)|BIT(26))
#define HDCP22_CAP_CHECK_COMPLETE BIT(27)
#define HDCP22_CAPABLE_SINK       BIT(28)
#define HDCP22_AUTH_SUCCESS       BIT(29)
#define HDCP22_AUTH_FAILED        BIT(30)
#define HDCP22_RE_AUTH_REQ        BIT(31)
#define HDCP_CFG_EN_HDCP13        BIT(2)
#define HDCP_CFG_EN_HDCP          BIT(1)

/* enable hdcp interrupts */
static int hdcp_esm_intr_en(struct dp_ctrl *dptx)
{
	uint32_t reg = 0;

	if (!dptx || !dptx->dpufd) {
		DPU_FB_ERR("%s dptx or dpufd is null\n", HDCP_TAG);
		return RET_HDCP_ERROR;
	}

	if (dptx->dpufd->secure_ctrl.hdcp_dpc_sec_en)
		dptx->dpufd->secure_ctrl.hdcp_dpc_sec_en();
	else
		DPU_FB_ERR("%s ATF:hdcp_dpc_sec_en is NULL\n", HDCP_TAG);

	/* enable interrupt include HDCP22 GPIO output status */
	if(dptx->dpufd->secure_ctrl.hdcp_reg_get)
		reg = dptx->dpufd->secure_ctrl.hdcp_reg_get(HDCP_API_INT_MSK);
	else
		DPU_FB_ERR("%s ATF:hdcp_reg_get is NULL\n", HDCP_TAG);

	if(dptx->dpufd->secure_ctrl.hdcp_int_mask)
		dptx->dpufd->secure_ctrl.hdcp_int_mask(reg);
	else
		DPU_FB_ERR("%s ATF:hdcp_int_mask is NULL\n", HDCP_TAG);

	return RET_HDCP_SUCCESS;
}

/* disable hdcp interrupts */
static int hdcp_esm_intr_dis(struct dp_ctrl *dptx)
{
	uint32_t reg = 0;

	if (!dptx || !dptx->dpufd) {
		DPU_FB_ERR("%s dptx or dpufd is null\n", HDCP_TAG);
		return RET_HDCP_ERROR;
	}

	if(dptx->dpufd->secure_ctrl.hdcp_dpc_sec_en)
		dptx->dpufd->secure_ctrl.hdcp_dpc_sec_en();
	else
		DPU_FB_ERR("%s ATF:hdcp_dpc_sec_en is NULL\n", HDCP_TAG);

	/* Mask interrupt related to HDCP22 GPIO output status */
	if(dptx->dpufd->secure_ctrl.hdcp_reg_get)
		reg = dptx->dpufd->secure_ctrl.hdcp_reg_get(HDCP_API_INT_MSK);
	else
		DPU_FB_ERR("%s ATF:hdcp_reg_get is NULL\n", HDCP_TAG);

	reg &= ~HDCP22_GPIOINT;

	if (dptx->dpufd->secure_ctrl.hdcp_int_mask)
		dptx->dpufd->secure_ctrl.hdcp_int_mask(reg);
	else
		DPU_FB_ERR("%s ATF:hdcp_int_mask is NULL\n", HDCP_TAG);

	return RET_HDCP_SUCCESS;
}

static int hdcp_esm_handle_cp_irq(struct dp_ctrl *dptx)
{
	if (!dptx || !dptx->dpufd) {
		DPU_FB_ERR("%s dptx or dpufd is null\n", HDCP_TAG);
		return RET_HDCP_ERROR;
	}

	if (dptx->dpufd->secure_ctrl.hdcp_cp_irq)
		dptx->dpufd->secure_ctrl.hdcp_cp_irq();
	else
		DPU_FB_ERR("%s ATF:hdcp_cp_irq is NULL\n", HDCP_TAG);

	return RET_HDCP_SUCCESS;
}

static void hdcp22_gpio_intr_clear(bool keep_intr, struct dp_ctrl *dptx, uint32_t hdcpobs)
{
	uint32_t reg_mask = 0;

	if (keep_intr) { /* mask interrupt: DPTX_HDCP22_GPIOINT */
		DPU_FB_DEBUG("%s HDCP2.2 register mask\n", HDCP22_TAG);
		if(dptx->dpufd->secure_ctrl.hdcp_reg_get)
			reg_mask = dptx->dpufd->secure_ctrl.hdcp_reg_get(HDCP_API_INT_MSK);
		else
			DPU_FB_ERR("%s ATF:hdcp_reg_get is NULL\n", HDCP22_TAG);

		reg_mask |= HDCP22_GPIOINT;

		if (dptx->dpufd->secure_ctrl.hdcp_int_mask)
			dptx->dpufd->secure_ctrl.hdcp_int_mask(reg_mask);
		else
			DPU_FB_ERR("%s ATF:hdcp_int_mask is NULL\n", HDCP22_TAG);

		/* clear HDCP22 interrupt except authen_success/failed */
		hdcpobs &= (~(HDCP22_AUTH_SUCCESS | HDCP22_AUTH_FAILED));
	}
	DPU_FB_DEBUG("%s HDCP2.2 register clear, DPTX_HDCP_OBS=0x%x\n", HDCP22_TAG, hdcpobs);
	if (dptx->dpufd->secure_ctrl.hdcp_obs_set)
		dptx->dpufd->secure_ctrl.hdcp_obs_set(hdcpobs);
	else
		DPU_FB_ERR("%s ATF:hdcp_obs_set is NULL\n", HDCP22_TAG);
}

static void handle_hdcp22_gpio_intr(struct dp_ctrl *dptx)
{
	uint32_t hdcpobs = 0;
	bool keep_intr = false;

	if (dptx->dpufd->secure_ctrl.hdcp_reg_get)
		hdcpobs = dptx->dpufd->secure_ctrl.hdcp_reg_get(HDCP_OBS);
	else
		DPU_FB_ERR("%s ATF:hdcp_reg_get is NULL\n", HDCP22_TAG);

	DPU_FB_DEBUG("%s DPTX_HDCP_OBS = 0x%x\n", HDCP22_TAG, hdcpobs);
	if (hdcpobs & HDCP22_BOOTED)
		DPU_FB_DEBUG("%s ESM has booted\n", HDCP22_TAG);

	if (hdcpobs & HDCP22_CAP_CHECK_COMPLETE) {
		if (hdcpobs & HDCP22_CAPABLE_SINK)
			DPU_FB_DEBUG("%s sink is HDCP22 capable\n", HDCP22_TAG);
		else
			DPU_FB_NOTICE("%s sink is not HDCP22 capable\n", HDCP22_TAG);
	}

	if (hdcpobs & HDCP22_AUTH_SUCCESS) {
		keep_intr = true;
		DPU_FB_NOTICE("%s the authentication is succeded\n", HDCP22_TAG);
		dp_imonitor_set_param(DP_PARAM_HDCP_KEY_S, NULL);
	}

	if (hdcpobs & HDCP22_AUTH_FAILED) {
		keep_intr = true;
		DPU_FB_NOTICE("%s the authentication is failed\n", HDCP22_TAG);
		dp_imonitor_set_param(DP_PARAM_HDCP_KEY_F, NULL);
	}

	if (hdcpobs & HDCP22_RE_AUTH_REQ)
		DPU_FB_WARNING("%s the sink has requested a re-authentication\n", HDCP22_TAG);

	hdcp22_gpio_intr_clear(keep_intr, dptx, hdcpobs);
}

static void hdcp_intr_clear(bool keep_intr, struct dp_ctrl *dptx, uint32_t hdcpintsts)
{
	uint32_t reg_mask = 0;

	if (keep_intr) { /* mask interrupt: (DPTX_HDCP_ENGAGED | DPTX_HDCP_FAILED) */
		DPU_FB_DEBUG("%s HDCP1.3 register mask\n", HDCP13_TAG);
		if (dptx->dpufd->secure_ctrl.hdcp_reg_get)
			reg_mask = dptx->dpufd->secure_ctrl.hdcp_reg_get(HDCP_API_INT_MSK);
		else
			DPU_FB_ERR("%s ATF:hdcp_reg_get is NULL\n", HDCP13_TAG);

		reg_mask |= (HDCP_ENGAGED | HDCP_FAILED);
		if (dptx->dpufd->secure_ctrl.hdcp_int_mask)
			dptx->dpufd->secure_ctrl.hdcp_int_mask(reg_mask);
		else
			DPU_FB_ERR("%s ATF:hdcp_int_mask is NULL\n", HDCP13_TAG);

		/* clear HDCP interrupt except hdcp_engaged/failed */
		hdcpintsts &= (~(HDCP_ENGAGED | HDCP_FAILED));
	}
	DPU_FB_DEBUG("%s HDCP1.3 and 2.2 register clear. DPTX_HDCP_INT_STS=0x%x\n", HDCP13_TAG, hdcpintsts);
	if (dptx->dpufd->secure_ctrl.hdcp_int_clr)
		dptx->dpufd->secure_ctrl.hdcp_int_clr(hdcpintsts);
	else
		DPU_FB_ERR("%s ATF:hdcp_int_clr is NULL\n", HDCP13_TAG);
}

static int hdcp_esm_handle_intr(struct dp_ctrl *dptx, uint32_t *auth_fail_count)
{
	uint32_t hdcpintsts = 0;
	bool keep_intr = false;
	int ret = RET_HDCP_SUCCESS;

	if (!dptx || !auth_fail_count || !dptx->dpufd) {
		DPU_FB_ERR("%s dptx or dpufd or fail_count is NULL\n", HDCP13_TAG);
		return RET_HDCP_ERROR;
	}

	if(dptx->dpufd->secure_ctrl.hdcp_reg_get)
		hdcpintsts = dptx->dpufd->secure_ctrl.hdcp_reg_get(HDCP_INT_STS);
	else
		DPU_FB_ERR("%s ATF:hdcp_reg_get is NULL\n", HDCP13_TAG);

	DPU_FB_DEBUG("%s DPTX_HDCP_INT_STS=0x%08x\n", HDCP13_TAG, hdcpintsts);
	if (hdcpintsts & HDCP_KSV_ACCESS_INT)
		DPU_FB_DEBUG("%s KSV memory access guaranteed for read, write access\n", HDCP13_TAG);

	if (hdcpintsts & HDCP_KSV_SHA1)
		DPU_FB_DEBUG("%s SHA1 verification has been done\n", HDCP13_TAG);

	if (hdcpintsts & HDCP_AUX_RESP_TIMEOUT) {
		ret = RET_HDCP13_AUTH_TIMEOUT;
		keep_intr = true;
		DPU_FB_WARNING("%s DPTX_HDCP_AUX_RESP_TIMEOUT\n", HDCP13_TAG);
	}

	if (hdcpintsts & HDCP_FAILED) {
		(*auth_fail_count)++;
		if (*auth_fail_count > DPTX_HDCP_MAX_AUTH_RETRY) {
			ret = RET_HDCP13_AUTH_FAIL;
			keep_intr = true;
			DPU_FB_ERR("%s Reach max allowed retries count=%u\n", HDCP13_TAG, *auth_fail_count);
		} else {
			DPU_FB_INFO("%s HDCP13 authentication process was failed!\n", HDCP13_TAG);
			dp_imonitor_set_param(DP_PARAM_HDCP_KEY_F, NULL);
		}
	}

	if (hdcpintsts & HDCP_ENGAGED) {
		*auth_fail_count = 0;
		ret = RET_HDCP13_AUTH_SUCCESS;
		keep_intr = true;
		DPU_FB_NOTICE("%s HDCP13 authentication process was successful\n", HDCP13_TAG);
		dp_imonitor_set_param(DP_PARAM_HDCP_KEY_S, NULL);
	}

	if (hdcpintsts & HDCP22_GPIOINT) {
		DPU_FB_DEBUG("%s HDCP22_GPIOINT\n", HDCP22_TAG);
		handle_hdcp22_gpio_intr(dptx);
	}

	hdcp_intr_clear(keep_intr, dptx, hdcpintsts);
	return ret;
}

/* check hdcp enable or disable */
static int hdcp_check_status(struct dp_ctrl *dptx)
{
	uint32_t temp_value;

	if (!dptx || !dptx->dpufd) {
		DPU_FB_ERR("%s dptx or dpufd is null\n", HDCP_TAG);
		return RET_HDCP_ERROR;
	}

	if (dptx->dpufd->secure_ctrl.hdcp_reg_get == NULL) {
		DPU_FB_ERR("%s ATF:hdcp_reg_get is null\n", HDCP_TAG);
		return RET_HDCP_ERROR;
	}
	temp_value = dptx->dpufd->secure_ctrl.hdcp_reg_get(HDCP_CFG);
	DPU_FB_DEBUG("%s DPTX_HDCP_CFG is:0x%x!\n", HDCP_TAG, temp_value);
	if(temp_value & HDCP_CFG_EN_HDCP)
		return RET_HDCP_ENABLE;

	return RET_HDCP_DISABLE;
}

static void hdcp_esm_init(void)
{
	esm_driver_enable(1);
}

static void hdcp_esm_deinit(void)
{
	esm_driver_enable(0);
}

void hdcp_cb_fun_register(struct hdcp_cb_fun *hdcp_cb)
{
	hdcp_cb->intr_en = hdcp_esm_intr_en;
	hdcp_cb->intr_dis = hdcp_esm_intr_dis;
	hdcp_cb->handle_cp_irq = hdcp_esm_handle_cp_irq;
	hdcp_cb->handle_intr = hdcp_esm_handle_intr;
	hdcp_cb->init = hdcp_esm_init;
	hdcp_cb->deinit = hdcp_esm_deinit;
	hdcp_cb->check_status = hdcp_check_status;
}

