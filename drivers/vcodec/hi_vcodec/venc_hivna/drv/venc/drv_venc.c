/*
 * drv_venc.c
 *
 * This is for venc drv.
 *
 * Copyright (c) 2009-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "drv_venc.h"
#include "hal_venc.h"
#include "drv_common.h"
#include "venc_regulator.h"
#include "drv_mem.h"

#ifdef SUPPORT_VENC_FREQ_CHANGE
static uint32_t g_current_venc_freq = 0;
#endif

static int32_t venc_init(void)
{
	if (drv_mem_init() != HI_SUCCESS) {
		HI_ERR_VENC("drv_mem_init failed");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

static void venc_deinit(void)
{
	drv_mem_exit();
}

static int32_t venc_update_hardware_status(struct encode_info *encode_info)
{
	int32_t ret = HI_SUCCESS;

	if (encode_info->is_reset == true) {
		ret = venc_regulator_reset();
		if (ret != HI_SUCCESS) {
			HI_ERR_VENC("reset failed");
			return ret;
		}
	}

#ifdef SUPPORT_VENC_FREQ_CHANGE
	mutex_lock(&g_venc_freq_mutex);
	if (g_current_venc_freq != g_venc_freq) {
		encode_info->clock_info.is_set_clock = true;
		g_current_venc_freq = g_venc_freq;
	}
	mutex_unlock(&g_venc_freq_mutex);
#endif

	if (encode_info->clock_info.is_set_clock == true) {
		ret = venc_regulator_update(&encode_info->clock_info);
		if (ret != HI_SUCCESS)
			HI_WARN_VENC("update clock info failed");
	}

	return ret;
}

static void venc_start_encode(struct encode_info *encode_info,
	struct venc_fifo_buffer *buffer, int32_t core_id)
{
	struct venc_context *ctx = NULL;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	ctx = &venc->ctx[core_id];
#if ((defined HIVCODECV500) && (defined VENC_DEBUG_ENABLE))
	venc_smmu_debug(ctx->reg_base, ctx->first_cfg_flag);
#endif

	if (ctx->is_protected != encode_info->is_protected)
		ctx->first_cfg_flag = true;

	if (encode_info->reg_cfg_mode == VENC_SET_CFGREGSIMPLE)
		vedu_hal_cfg_reg_simple(encode_info, core_id);
	else
		vedu_hal_cfg_reg(encode_info, core_id);

	ctx->status = VENC_BUSY;
	ctx->start_time = osal_get_sys_time_in_us();
	ctx->is_block = encode_info->is_block;
	ctx->buffer = buffer;
	ctx->is_protected = encode_info->is_protected;

	if (encode_info->is_block == false) {
		(void)memcpy_s(&ctx->channel, sizeof(struct channel_info), &encode_info->channel, sizeof(struct channel_info));
		osal_add_timer(&ctx->timer, INTERRUPT_TIMEOUT_MS);
	}

	venc_hal_start_encode((S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base)); /*lint !e826 */
}

static int32_t venc_encode(struct encode_info *encode_info, struct venc_fifo_buffer *buffer)
{
	int32_t ret;
	int32_t core_id;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	bool support_power_control_per_frame =
		is_support_power_control_per_frame() &&
		encode_info->enable_power_control_per_frame &&
		encode_info->is_block;

	ret = hi_venc_down_interruptible(&venc->hw_sem);
	if (ret != HI_SUCCESS) {
		HI_FATAL_VENC("down sem failed");
		return ret;
	}

	if (support_power_control_per_frame) {
		ret = venc_regulator_enable();
		if (ret != HI_SUCCESS) {
			HI_FATAL_VENC("power up fail, ret %d", ret);
			goto poweron_failed;
		}
	}

	ret = venc_update_hardware_status(encode_info);
	if (ret != HI_SUCCESS)
		goto exit;

	venc_set_clock_info(&encode_info->clock_info);

	core_id = venc_regulator_select_idle_core(&venc->event);
	ret = venc_check_coreid(core_id);
	if (ret != HI_SUCCESS) {
		HI_ERR_VENC("CORE_ERROR:invalid core ID is %d", core_id);
		goto exit;
	}

	pm_hardware_busy_enter(&venc->ctx[core_id].pm);

	venc_start_encode(encode_info, buffer, core_id);

	if (!encode_info->is_block)
		goto exit;

	/* Enter the synchronous mode, wait for the encoding to complete, and fill the queue. */
	ret = venc_drv_get_encode_done_info(buffer, &encode_info->encode_done_info);
	if (ret != HI_SUCCESS) {
		HI_FATAL_VENC("wait encode done failed");
		/* Marks the encoder as timeout and resets the encoder in the next frame. */
		venc->ctx[core_id].status = VENC_TIME_OUT;
	}

exit:
	if (support_power_control_per_frame) {
		ret = venc_regulator_disable();
		if (ret != HI_SUCCESS)
			HI_FATAL_VENC("power off fail, ret %d", ret);
	}
poweron_failed:
	hi_venc_up_interruptible(&venc->hw_sem);
	return ret;
}

static void venc_encode_timeout(unsigned long core_id)
{
	int32_t ret;
	struct encode_done_info info = {0};
	struct venc_context *ctx = NULL;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	unsigned long flag;

	ret = venc_check_coreid((int32_t)core_id);
	if (ret != HI_SUCCESS) {
		HI_ERR_VENC("CORE_ERROR:invalid core ID is %d", (int32_t)core_id);
		return;
	}

	ctx = &venc->ctx[core_id];

	spin_lock_irqsave(&ctx->lock, flag);
	if (ctx->status != VENC_BUSY) {
		spin_unlock_irqrestore(&ctx->lock, flag);
		HI_FATAL_VENC("core %d: current core status invalid, status: %d",
				(int32_t)core_id, ctx->status);
		return;
	}

	(void)memcpy_s(&info.channel_info, sizeof(struct channel_info), &ctx->channel, sizeof(struct channel_info));
	info.is_timeout = true;

	HI_WARN_VENC("coreid:%d encode timeout", (int32_t)core_id);
	ret = push(ctx->buffer, &info);
	if (ret != HI_SUCCESS)
		HI_FATAL_VENC("core %d: write buffer failed", (int32_t)core_id);

	pm_hardware_busy_exit(&ctx->pm);
	ctx->status = VENC_TIME_OUT;

	/* wake up preprocess and postprocess */
	venc_drv_osal_give_event(&venc->event);
	venc_drv_osal_give_event(&ctx->buffer->event);
	spin_unlock_irqrestore(&ctx->lock, flag);
}

static void venc_encode_done(struct venc_context *ctx)
{
	uint32_t ret;
	U_FUNC_VCPI_INTSTAT int_status;
	U_VEDU_VCPI_MULTISLC multi_slc;
	S_HEVC_AVC_REGS_TYPE *reg_base = NULL;
	struct encode_done_info info = {0};
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	unsigned long flag;

	reg_base = (S_HEVC_AVC_REGS_TYPE *)ctx->reg_base;
	int_status.u32 = reg_base->FUNC_VCPI_INTSTAT.u32;
	multi_slc.u32 = reg_base->VEDU_VCPI_MULTISLC.u32;

	if (!int_status.bits.vcpi_int_ve_eop &&
		!int_status.bits.vcpi_int_vedu_timeout &&
		!int_status.bits.vcpi_int_vedu_slice_end) {
		HI_WARN_VENC("not support isr %x", int_status.u32);
		reg_base->VEDU_VCPI_INTCLR.u32 = int_status.u32;
		return;
	}

#ifdef VENC_DEBUG_ENABLE
	if (venc->debug_flag & (1LL << PRINT_ENC_COST_TIME))
		HI_INFO_VENC("hardware cost time %llu us", osal_get_during_time(ctx->start_time));
#endif

	spin_lock_irqsave(&ctx->lock, flag);
	if (ctx->status != VENC_BUSY) {
		spin_unlock_irqrestore(&ctx->lock, flag);
		HI_FATAL_VENC("current core status invalid, core status: %d, isr status %x",
			ctx->status, int_status.u32);
		reg_base->VEDU_VCPI_INTCLR.u32 = int_status.u32;
		return;
	}

	// The interrupt handling function does not independently process the bufferfull interrupt status.
	// When the EOP or slice end interrupts are generated, the information is returned to the user mode by reading the raw interrupt register.
	info.is_timeout = int_status.bits.vcpi_int_vedu_timeout;
	if (int_status.bits.vcpi_int_ve_eop || int_status.bits.vcpi_int_vedu_timeout) {
		if (!ctx->is_block) {
			if (osal_del_timer(&ctx->timer, false) != HI_SUCCESS)
				HI_FATAL_VENC("delete timer fail");

			(void)memcpy_s(&info.channel_info, sizeof(struct channel_info), &ctx->channel, sizeof(struct channel_info));
		}

		venc_hal_get_reg_venc(&info.stream_info, ctx->reg_base);
		ret = push(ctx->buffer, &info);
		if (ret != HI_SUCCESS)
			HI_ERR_VENC("write buffer fail");
		venc_drv_osal_give_event(&ctx->buffer->event); /* wake up preprocess */

		pm_hardware_busy_exit(&ctx->pm);
		ctx->status = VENC_IDLE;
		venc_drv_osal_give_event(&venc->event); /* wake up postprocess */
		reg_base->VEDU_VCPI_INTCLR.u32 = 0xFFFFFFFF;
		spin_unlock_irqrestore(&ctx->lock, flag);

		return;
	}

#ifdef SLICE_INT_EN
	if (!ctx->is_block && multi_slc.bits.vcpi_multislc_en) {
		// slice end interrupt
		(void)memcpy_s(&info.channel_info, sizeof(struct channel_info), &ctx->channel, sizeof(struct channel_info));
		venc_hal_get_slice_reg(&info.stream_info, ctx->reg_base);
		ret = push(ctx->buffer, &info);
		if (ret != HI_SUCCESS)
			HI_ERR_VENC("write buffer fail");
		venc_drv_osal_give_event(&ctx->buffer->event);
	}
#endif
	reg_base->VEDU_VCPI_INTCLR.u32 = int_status.u32;
	spin_unlock_irqrestore(&ctx->lock, flag);
}

void venc_init_ops(void)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	venc->ops.init = venc_init;
	venc->ops.deinit = venc_deinit;
	venc->ops.encode = venc_encode;
	venc->ops.encode_done = venc_encode_done;
	venc->ops.encode_timeout = venc_encode_timeout;

	HI_INFO_VENC("config register by apb bus");
}
