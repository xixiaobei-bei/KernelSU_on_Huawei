/*
 * jpeg jpu exec
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#include <linux/uaccess.h>
#include <linux/delay.h>
#include <securec.h>
#include "hisi_jpu_utils.h"
#include "hisi_jpu_iommu.h"
#include "hisi_jpu_def.h"
#include "jpgdec_platform.h"

uint32_t jpu_set_bits32(uint32_t old_val, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask = (uint32_t)((1UL << bw) - 1UL);
	uint32_t tmp = old_val;

	tmp &= ~(mask << bs);
	return (tmp | ((val & mask) << bs));
}

static void dump_top_reg(struct hisi_jpu_data_type *hisijd)
{
	hisi_jpu_info("dump top_reg: \n");
	hisi_jpu_info("JPGDEC_RO_STATE: 0x%x\n",
		inp32(hisijd->jpu_top_base + JPGDEC_RO_STATE));
	hisi_jpu_info("JPGDEC_CRG_CFG0: 0x%x\n",
		inp32(hisijd->jpu_top_base + JPGDEC_CRG_CFG0));
	hisi_jpu_info("JPGDEC_CRG_CFG1: 0x%x\n",
		inp32(hisijd->jpu_top_base + JPGDEC_CRG_CFG1));
	hisi_jpu_info("JPGDEC_MEM_CFG: 0x%x\n",
		inp32(hisijd->jpu_top_base + JPGDEC_MEM_CFG));
	hisi_jpu_info("JPGDEC_IRQ_REG0: 0x%x\n",
		inp32(hisijd->jpu_top_base + JPGDEC_IRQ_REG0));
	hisi_jpu_info("JPGDEC_IRQ_REG1: 0x%x\n",
		inp32(hisijd->jpu_top_base + JPGDEC_IRQ_REG1));
	hisi_jpu_info("JPGDEC_IRQ_REG2: 0x%x\n",
		inp32(hisijd->jpu_top_base + JPGDEC_IRQ_REG2));
}

static void hisi_jpu_dump_reg(struct hisi_jpu_data_type *hisijd)
{
	int i;

	if (hisijd == NULL) {
		hisi_jpu_err("hisijd is NULL\n");
		return;
	}

	hisi_jpu_info("dump debug_reg:\n");
	for (i = 0; i < JPGD_REG_DEBUG_RANGE; i++)
		hisi_jpu_info("JPEG_DEC_DEBUG_INFO offset @ %d, val:0x%x\n",
			NUM_COMPS_IN_SCAN * i, inp32(hisijd->jpu_dec_base +
			JPGD_REG_DEBUG_BASE + NUM_COMPS_IN_SCAN * i));

	hisi_jpu_info("dump cvdr_reg: \n");
	if ((hisijd->jpu_support_platform == HISI_DSS_V400) ||
		(hisijd->jpu_support_platform == HISI_DSS_V501))
		hisi_jpu_info("JPGDEC_CVDR_AXI_JPEG_CVDR_CFG: 0x%x\n",
			inp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_CVDR_CFG));

	hisi_jpu_info("JPGDEC_CVDR_AXI_WR_CFG1: 0x%x\n",
		inp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_WR_CFG1));
	hisi_jpu_info("JPGDEC_CVDR_AXI_WR_CFG2: 0x%x\n",
		inp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_WR_CFG2));
	hisi_jpu_info("JPGDEC_CVDR_AXI_LIMITER_RD_CFG1: 0x%x\n",
		inp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_LIMITER_RD_CFG1));
	hisi_jpu_info("JPGDEC_CVDR_AXI_RD_CFG1: 0x%x\n",
		inp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_RD_CFG1));
	hisi_jpu_info("JPGDEC_CVDR_AXI_LIMITER_RD_CFG2: 0x%x\n",
		inp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_LIMITER_RD_CFG2));
	hisi_jpu_info("JPGDEC_CVDR_AXI_RD_CFG2: 0x%x\n",
		inp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_RD_CFG2));

	hisi_jpu_info("JPGDEC_CVDR_AXI_JPEG_CVDR_WR_QOS_CFG: 0x%x\n",
		inp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_CVDR_WR_QOS_CFG));
	hisi_jpu_info("JPGDEC_CVDR_AXI_JPEG_CVDR_RD_QOS_CFG: 0x%x\n",
		inp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_CVDR_RD_QOS_CFG));

	dump_top_reg(hisijd);
}

static long jpu_timestamp_diff(const struct timeval *lasttime,
	const struct timeval *curtime)
{
	bool cond = ((lasttime == NULL) || (curtime == NULL));
	if (cond)
		return 0;

	return (curtime->tv_usec >= lasttime->tv_usec) ?
		(curtime->tv_usec - lasttime->tv_usec) :
		(1000000 - (lasttime->tv_usec - curtime->tv_usec)); /* clk cycle 1000000 us */
}

static void jpu_get_timestamp(struct timeval *tv)
{
	struct timespec ts;

	ktime_get_ts(&ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
}

static int hisi_jpu_dec_done(struct hisi_jpu_data_type *hisijd,
	struct jpu_data_t *jpu_req)
{
	struct timeval tv0 = {0};
	struct timeval tv1 = {0};
	long timediff;
	int ret;

	if (g_debug_jpu_dec_job_timediff != 0)
		jpu_get_timestamp(&tv0);

	ret = hisi_jpu_dec_done_config(hisijd, jpu_req);
	if (ret != 0) {
		hisi_jpu_err("hisi_jpu_dec_done_config failed ret = %d\n", ret);
		hisi_jpu_dump_reg(hisijd);
	}

	if (g_debug_jpu_dec_job_timediff != 0) {
		jpu_get_timestamp(&tv1);
		timediff = jpu_timestamp_diff(&tv0, &tv1);
		hisi_jpu_info("jpu job exec timediff is %ld us\n", timediff);
	}

	return ret;
}

static int hisijpu_set_crop(struct hisi_jpu_data_type *hisijd)
{
	struct jpu_data_t *jpu_req = NULL;
	jpu_dec_reg_t *pjpu_dec_reg = NULL;

	jpu_req = &(hisijd->jpu_req);
	pjpu_dec_reg = &(hisijd->jpu_dec_reg);

	/* for full decode */
	if (jpu_req->decode_mode <= HISI_JPEG_DECODE_MODE_FULL_SUB) {
		pjpu_dec_reg->crop_horizontal =
			jpu_set_bits32(pjpu_dec_reg->crop_horizontal,
			(jpu_req->inwidth - 1) << SHIFT_16_BIT, REG_SET_32_BIT, 0);

		pjpu_dec_reg->crop_vertical =
			jpu_set_bits32(pjpu_dec_reg->crop_vertical,
			(jpu_req->inheight - 1) << SHIFT_16_BIT, REG_SET_32_BIT, 0);
	} else {
		if ((jpu_req->region_info.right < MIN_INPUT_WIDTH) ||
			(jpu_req->region_info.bottom < MIN_INPUT_WIDTH))
			return -EINVAL;

		pjpu_dec_reg->crop_horizontal =
			jpu_set_bits32(pjpu_dec_reg->crop_horizontal,
			(((uint32_t)(jpu_req->region_info.right - 1)) << SHIFT_16_BIT) |
			(jpu_req->region_info.left), REG_SET_32_BIT, 0);

		pjpu_dec_reg->crop_vertical =
			jpu_set_bits32(pjpu_dec_reg->crop_vertical,
			(((uint32_t)(jpu_req->region_info.bottom - 1)) << SHIFT_16_BIT) |
			(jpu_req->region_info.top), REG_SET_32_BIT, 0);
	}

	return 0;
}

static void hisijpu_set_dqt(const struct jpu_data_t *jpu_req,
	char __iomem *jpu_dec_base)
{
	uint32_t i;

	for (i = 0; i < MAX_DCT_SIZE; i++)
		outp32(jpu_dec_base + JPGD_REG_QUANT + MAX_NUM_QUANT_TBLS * i,
			jpu_req->qvalue[i]);
}

static int hisijpu_set_dht(const struct jpu_data_t *jpu_req,
	char __iomem *jpu_dec_base)
{
	int i;

	for (i = 0; i < HDC_TABLE_NUM; i++) {
		outp32(jpu_dec_base + JPGD_REG_HDCTABLE + MAX_NUM_HUFF_TBLS * i,
			jpu_req->hdc_tab[i]);
		hisi_jpu_debug("hdc_tab[%d]%u\n", i, jpu_req->hdc_tab[i]);
	}

	for (i = 0; i < HAC_TABLE_NUM; i++) {
		outp32(jpu_dec_base + JPGD_REG_HACMINTABLE + MAX_NUM_HUFF_TBLS * i,
			jpu_req->hac_min_tab[i]);

		outp32(jpu_dec_base + JPGD_REG_HACBASETABLE + MAX_NUM_HUFF_TBLS * i,
			jpu_req->hac_base_tab[i]);

		hisi_jpu_debug("hac_min_tab[%d] %u\n", i, jpu_req->hac_min_tab[i]);
		hisi_jpu_debug("hac_base_tab[%d] %u\n", i, jpu_req->hac_base_tab[i]);
	}

	for (i = 0; i < HAC_SYMBOL_TABLE_NUM; i++) {
		outp32(jpu_dec_base + JPGD_REG_HACSYMTABLE + MAX_NUM_HUFF_TBLS * i,
			jpu_req->hac_symbol_tab[i]);

		hisi_jpu_debug("gs_hac_symbol_tab[%d] %u\n",
			i, jpu_req->hac_symbol_tab[i]);
	}

	return 0;
}

static bool is_original_format(enum jpu_raw_format in_format,
	enum jpu_color_space out_format)
{
	/* input format need be same as output format */
	bool cond = ((in_format == HISI_JPEG_DECODE_RAW_YUV444) &&
			(out_format == HISI_JPEG_DECODE_OUT_YUV444)) ||
		((in_format == HISI_JPEG_DECODE_RAW_YUV422_H2V1) &&
			(out_format == HISI_JPEG_DECODE_OUT_YUV422_H2V1)) ||
		((in_format == HISI_JPEG_DECODE_RAW_YUV422_H1V2) &&
			(out_format == HISI_JPEG_DECODE_OUT_YUV422_H1V2)) ||
		((in_format == HISI_JPEG_DECODE_RAW_YUV420) &&
			(out_format == HISI_JPEG_DECODE_OUT_YUV420)) ||
		((in_format == HISI_JPEG_DECODE_RAW_YUV400) &&
			(out_format == HISI_JPEG_DECODE_OUT_YUV400));

	return cond;
}

static jpu_output_format hisi_out_format_hal2jpu(struct hisi_jpu_data_type *hisijd)
{
	struct jpu_data_t *jpu_req = NULL;
	jpu_output_format format = JPU_OUTPUT_UNSUPPORT;
	bool cond = false;

	jpu_req = &(hisijd->jpu_req);
	if (is_original_format(jpu_req->in_img_format, jpu_req->out_color_format))
		return JPU_OUTPUT_YUV_ORIGINAL;

	/* YUV400 can't decode to yuv420 */
	cond = (jpu_req->in_img_format == HISI_JPEG_DECODE_RAW_YUV400) &&
		(jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV420);
	if (cond)
		return format;

	switch (jpu_req->out_color_format) {
	case HISI_JPEG_DECODE_OUT_YUV420:
		format = JPU_OUTPUT_YUV420;
		break;
	case HISI_JPEG_DECODE_OUT_RGBA4444:
		format = JPU_OUTPUT_RGBA4444;
		break;
	case HISI_JPEG_DECODE_OUT_BGRA4444:
		format = JPU_OUTPUT_BGRA4444;
		break;
	case HISI_JPEG_DECODE_OUT_RGB565:
		format = JPU_OUTPUT_RGB565;
		break;
	case HISI_JPEG_DECODE_OUT_BGR565:
		format = JPU_OUTPUT_BGR565;
		break;
	case HISI_JPEG_DECODE_OUT_RGBA8888:
		format = JPU_OUTPUT_RGBA8888;
		break;
	case HISI_JPEG_DECODE_OUT_BGRA8888:
		format = JPU_OUTPUT_BGRA8888;
		break;
	default:
		hisi_jpu_err("jpu_req out_color_format is %d not support\n",
			jpu_req->out_color_format);
		break;
	}

	return format;
}

static int hisi_sample_size_hal2jpu(int val)
{
	int ret;

	switch (val) {
	case HISI_JPEG_DECODE_SAMPLE_SIZE_1:
		ret = JPU_FREQ_SCALE_1;
		break;
	case HISI_JPEG_DECODE_SAMPLE_SIZE_2:
		ret = JPU_FREQ_SCALE_2;
		break;
	case HISI_JPEG_DECODE_SAMPLE_SIZE_4:
		ret = JPU_FREQ_SCALE_4;
		break;
	case HISI_JPEG_DECODE_SAMPLE_SIZE_8:
		ret = JPU_FREQ_SCALE_8;
		break;
	default:
		hisi_jpu_err("not support sample size %d\n", val);
		return -1;
	}

	hisi_jpu_info("sample size %d\n", val);
	return ret;
}

static int hisi_jpu_set_paramters(struct hisi_jpu_data_type *hisijd,
	struct jpu_data_t *jpu_req, jpu_dec_reg_t *pjpu_dec_reg)
{
	int out_format;
	int jpu_freq_scale;
	int ret;
	uint8_t yfac;
	uint8_t ufac;
	uint8_t vfac;

	/* output type, should compare with input format */
	out_format = hisi_out_format_hal2jpu(hisijd);
	if (out_format < 0) {
		hisi_jpu_err("hisi_out_format_hal2jpu failed\n");
		return -EINVAL;
	}

	/* set alpha value as 0xff */
	if (jpu_req->out_color_format >= HISI_JPEG_DECODE_OUT_RGBA4444)
		pjpu_dec_reg->output_type = jpu_set_bits32(pjpu_dec_reg->output_type,
			(uint32_t)out_format | (0xFF << SHIFT_8_BIT), REG_SET_16_BIT, 0);
	else
		pjpu_dec_reg->output_type = jpu_set_bits32(pjpu_dec_reg->output_type,
			(uint32_t)out_format, REG_SET_16_BIT, 0);

	/* frequence scale */
	jpu_freq_scale = hisi_sample_size_hal2jpu(jpu_req->sample_rate);
	if (jpu_freq_scale < 0) {
		hisi_jpu_err("hisi_sample_size_hal2jpu failed\n");
		return -EINVAL;
	}
	pjpu_dec_reg->freq_scale = jpu_set_bits32(pjpu_dec_reg->freq_scale,
		(uint32_t)jpu_freq_scale, REG_SET_2_BIT, 0);

	/* for decode region */
	ret = hisijpu_set_crop(hisijd);
	if (ret != 0) {
		hisi_jpu_err("hisijpu_set_crop ret = %d\n", ret);
		return -EINVAL;
	}

	/* MIDDLE FITER can use default value */
	/* sampling factor */
	yfac = (((jpu_req->component_info[0].hor_sample_fac << SHIFT_4_BIT) |
		jpu_req->component_info[0].ver_sample_fac) & 0xFF);
	ufac = (((jpu_req->component_info[1].hor_sample_fac << SHIFT_4_BIT) |
		jpu_req->component_info[1].ver_sample_fac) & 0xFF);
	vfac = (((jpu_req->component_info[2].hor_sample_fac << SHIFT_4_BIT) |
		jpu_req->component_info[2].ver_sample_fac) & 0xFF);
	pjpu_dec_reg->sampling_factor = jpu_set_bits32(pjpu_dec_reg->sampling_factor,
		(vfac | (ufac << SHIFT_8_BIT) | (yfac << SHIFT_16_BIT)),
		REG_SET_24_BIT, 0);

	/* set dqt table */
	hisijpu_set_dqt(jpu_req, hisijd->jpu_dec_base);

	/* set dht table */
	if (hisijpu_set_dht(jpu_req, hisijd->jpu_dec_base) != 0) {
		hisi_jpu_err("hisijpu_set_dht failed\n");
		return -EINVAL;
	}

	return 0;
}

static void hisi_jpu_start_addr_control(const struct hisi_jpu_data_type *hisijd,
	jpu_dec_reg_t *pjpu_dec_reg, const struct jpu_data_t *jpu_req)
{
	if (hisijd->jpu_support_platform == HISI_DSS_V600) {
		/* output buffer addr */
		pjpu_dec_reg->frame_start_y = jpu_set_bits32(pjpu_dec_reg->frame_start_y,
			(uint32_t)jpu_req->start_addr_y, REG_SET_29_BIT, 0);

		pjpu_dec_reg->frame_stride_y = jpu_set_bits32(pjpu_dec_reg->frame_stride_y,
			jpu_req->stride_y | ((uint32_t)(jpu_req->last_page_y << SHIFT_12_BIT)),
			REG_SET_30_BIT, 0);

		pjpu_dec_reg->frame_start_c = jpu_set_bits32(pjpu_dec_reg->frame_start_c,
			(uint32_t)jpu_req->start_addr_c, REG_SET_29_BIT, 0);

		pjpu_dec_reg->frame_stride_c = jpu_set_bits32(pjpu_dec_reg->frame_stride_c,
			jpu_req->stride_c | ((uint32_t)(jpu_req->last_page_c << SHIFT_12_BIT)),
			REG_SET_30_BIT, 0);

		/* start address for line buffer,unit is 16 byte,
		 * must align to 128 byte
		 */
		pjpu_dec_reg->lbuf_start_addr = jpu_set_bits32(pjpu_dec_reg->lbuf_start_addr,
			hisijd->lb_addr, REG_SET_29_BIT, 0);
	} else {
		/* output buffer addr */
		pjpu_dec_reg->frame_start_y = jpu_set_bits32(pjpu_dec_reg->frame_start_y,
			(uint32_t)jpu_req->start_addr_y, REG_SET_28_BIT, 0);

		pjpu_dec_reg->frame_stride_y = jpu_set_bits32(pjpu_dec_reg->frame_stride_y,
			jpu_req->stride_y | ((uint32_t)(jpu_req->last_page_y << SHIFT_12_BIT)),
			REG_SET_29_BIT, 0);

		pjpu_dec_reg->frame_start_c = jpu_set_bits32(pjpu_dec_reg->frame_start_c,
			(uint32_t)jpu_req->start_addr_c, REG_SET_28_BIT, 0);

		pjpu_dec_reg->frame_stride_c = jpu_set_bits32(pjpu_dec_reg->frame_stride_c,
			jpu_req->stride_c | ((uint32_t)(jpu_req->last_page_c << SHIFT_12_BIT)),
			REG_SET_29_BIT, 0);

		/*
		 * start address for line buffer,unit is 16 byte,
		 * must align to 128 byte
		 */
		pjpu_dec_reg->lbuf_start_addr = jpu_set_bits32(pjpu_dec_reg->lbuf_start_addr,
			hisijd->lb_addr, REG_SET_28_BIT, 0);
	}
}

static bool is_rgb_out(uint32_t format)
{
	bool cond = ((format == HISI_JPEG_DECODE_OUT_RGBA4444) ||
		(format == HISI_JPEG_DECODE_OUT_BGRA4444) ||
		(format == HISI_JPEG_DECODE_OUT_RGB565) ||
		(format == HISI_JPEG_DECODE_OUT_BGR565) ||
		(format == HISI_JPEG_DECODE_OUT_RGBA8888) ||
		(format == HISI_JPEG_DECODE_OUT_BGRA8888));

	return cond;
}

static int hisijpu_check_outbuffer_par(const struct jpu_data_t *jpu_req)
{
	uint32_t out_addr_align;
	uint32_t stride_align_mask;
	uint32_t out_format;

	bool cond = ((jpu_req->out_color_format >= HISI_JPEG_DECODE_OUT_FORMAT_MAX) ||
		(jpu_req->out_color_format <= HISI_JPEG_DECODE_OUT_UNKNOWN));
	if (cond) {
		hisi_jpu_err("out_color_format %d is out of range\n",
			jpu_req->out_color_format);
		return -EINVAL;
	}

	out_format = (uint32_t)jpu_req->out_color_format;
	if (is_rgb_out(out_format)) {
		out_addr_align = JPU_OUT_RGB_ADDR_ALIGN;
		stride_align_mask = JPU_OUT_STRIDE_ALIGN - 1;
	} else {
		out_addr_align = JPU_OUT_YUV_ADDR_ALIGN;
		stride_align_mask = JPU_OUT_STRIDE_ALIGN / JPGDEC_EVEN_NUM - 1;
	}

	/*
	 * start address for planar Y or RGB, unit is 16 byte,
	 * must align to 64 byte (YUV format) or 128 byte (RGB format)
	 * stride for planar Y or RGB, unit is 16 byte,
	 * must align to 64 byte (YUV format) or 128 byte (RGB format)
	 */
	cond = ((jpu_req->start_addr_y > JPU_MAX_ADDR) ||
		(jpu_req->start_addr_y & (out_addr_align - 1)));
	if (cond) {
		hisi_jpu_err("start_addr_y is not %u bytes aligned\n", out_addr_align - 1);
		return -EINVAL;
	}
	cond = ((jpu_req->stride_y < JPU_MIN_STRIDE) ||
		(jpu_req->stride_y > JPU_MAX_STRIDE));
	if (cond) {
		hisi_jpu_err("stride_y %u is not %u bytes aligned\n",
			jpu_req->stride_y, stride_align_mask);
		return -EINVAL;
	}

	/*
	 * start address for planar UV, unit is 16 byte, must align to 64 byte
	 * stride for planar UV, unit is 16 byte, must align to 64 byte
	 */
	cond = ((jpu_req->start_addr_c > JPU_MAX_ADDR) ||
		(jpu_req->start_addr_c & (out_addr_align - 1)));
	if (cond) {
		hisi_jpu_err("start_addr_c is not %u bytes aligned\n", out_addr_align - 1);
		return -EINVAL;
	}

	if ((jpu_req->stride_c > JPU_MAX_STRIDE / JPGDEC_EVEN_NUM)) {
		hisi_jpu_err("stride_c %u is not %u bytes aligned\n",
			jpu_req->stride_c, stride_align_mask);
		return -EINVAL;
	}

	return 0;
}

static int hisi_jpu_check_inbuff_par(const struct jpu_data_t *jpu_req)
{
	bool cond = ((jpu_req->in_img_format >= HISI_JPEG_DECODE_RAW_MAX) ||
		(jpu_req->in_img_format <= HISI_JPEG_DECODE_RAW_YUV_UNSUPPORT));
	if (cond) {
		hisi_jpu_err("in_img_format %d is out of range\n",
			jpu_req->in_img_format);
		return -EINVAL;
	}

	cond = ((jpu_req->sample_rate >= HISI_JPEG_DECODE_SAMPLE_SIZE_MAX) ||
		(jpu_req->sample_rate < HISI_JPEG_DECODE_SAMPLE_SIZE_1));
	if (cond) {
		hisi_jpu_err("sample_rate %d is out of range\n",
			jpu_req->sample_rate);
		return -EINVAL;
	}

	cond = ((jpu_req->inwidth / jpu_req->sample_rate) < MIN_INPUT_WIDTH) ||
		(jpu_req->inwidth > MAX_INPUT_WIDTH) ||
		((jpu_req->inheight / jpu_req->sample_rate) < MIN_INPUT_HEIGHT) ||
		(jpu_req->inheight > MAX_INPUT_HEIGHT);
	if (cond) {
		hisi_jpu_err("image inwidth = %u, inheight = %u, "
			"sample_rate %d is out of range\n",
			jpu_req->inwidth, jpu_req->inheight, jpu_req->sample_rate);
		return -EINVAL;
	}
	cond = ((jpu_req->pix_width / jpu_req->sample_rate) < MIN_INPUT_WIDTH) ||
		(jpu_req->pix_width > MAX_INPUT_WIDTH) ||
		((jpu_req->pix_height / jpu_req->sample_rate) < MIN_INPUT_HEIGHT) ||
		(jpu_req->pix_height > MAX_INPUT_HEIGHT);
	if (cond) {
		hisi_jpu_err("image inwidth = %u, inheight = %u, "
			"sample_rate %d is out of range\n",
			jpu_req->pix_width, jpu_req->pix_height, jpu_req->sample_rate);
		return -EINVAL;
	}

	return 0;
}

static int hisijpu_check_buffers(struct hisi_jpu_data_type *hisijd,
	struct jpu_data_t *jpu_req)
{
	if (hisi_jpu_check_inbuff_par(jpu_req) != 0) {
		hisi_jpu_err("check input buffer par error\n");
		return -EINVAL;
	}

	if (hisi_jpu_check_inbuff_addr(hisijd, jpu_req) != 0) {
		hisi_jpu_err("check inputbuff addr error\n");
		return -EINVAL;
	}

	if (hisijpu_check_outbuffer_par(jpu_req) != 0) {
		hisi_jpu_err("check outbuffer par error\n");
		return -EINVAL;
	}

	if (hisi_jpu_check_outbuff_addr(hisijd, jpu_req) != 0) {
		hisi_jpu_err("check outputbuff addr error\n");
		return -EINVAL;
	}
	return 0;
}

static int hisijpu_check_full_decode_info(const struct jpu_data_t *jpu_req)
{
	bool cond = false;

	if (jpu_req->decode_mode >= HISI_JPEG_DECODE_MODE_MAX) {
		hisi_jpu_err("decode_mode %d is out of range\n", jpu_req->decode_mode);
		return -EINVAL;
	}

	if (jpu_req->decode_mode > HISI_JPEG_DECODE_MODE_FULL_SUB) {
		hisi_jpu_info("in region decode, decode_mode %d\n",
			jpu_req->decode_mode);
		return 0;
	}

	cond = (jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV422_H2V1) &&
		((jpu_req->pix_width / jpu_req->sample_rate) % JPGDEC_EVEN_NUM);
	if (cond) {
		hisi_jpu_err("out_color_format %d, jpu_req->pix_width %u invalid\n",
			jpu_req->out_color_format, jpu_req->pix_width);
		return -EINVAL;
	}

	cond = (jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV420) &&
		(((jpu_req->pix_width / jpu_req->sample_rate) % JPGDEC_EVEN_NUM) ||
		((jpu_req->pix_height / jpu_req->sample_rate) % JPGDEC_EVEN_NUM));
	if (cond) {
		hisi_jpu_err("out_color_format %d, jpu_req->pix_width %u invalid\n",
			jpu_req->out_color_format, jpu_req->pix_width);
		return -EINVAL;
	}

	cond = (jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV422_H1V2) &&
		((jpu_req->pix_height / jpu_req->sample_rate) % JPGDEC_EVEN_NUM);
	if (cond) {
		hisi_jpu_err("out_color_format %d,jpu_req->pix_height %u invalid\n",
			jpu_req->out_color_format, jpu_req->pix_height);
		return -EINVAL;
	}

	return 0;
}

static int hisijpu_check_region_decode_info(const struct jpu_data_t *jpu_req)
{
	if (jpu_req->decode_mode < HISI_JPEG_DECODE_MODE_REGION) {
		hisi_jpu_info("in full decode, decode mode %d\n", jpu_req->decode_mode);
		return 0;
	}

	if ((jpu_req->region_info.left >= jpu_req->region_info.right) ||
		(jpu_req->region_info.top >= jpu_req->region_info.bottom) ||
		((((jpu_req->region_info.right - jpu_req->region_info.left) + 1) /
			jpu_req->sample_rate) < MIN_INPUT_WIDTH) ||
		((((jpu_req->region_info.bottom - jpu_req->region_info.top) +  1) /
			jpu_req->sample_rate) < MIN_INPUT_HEIGHT)) {
		hisi_jpu_err("region[%u %u %u %u] invalid sample_rate %d\n",
			jpu_req->region_info.left, jpu_req->region_info.right,
			jpu_req->region_info.top, jpu_req->region_info.bottom,
			jpu_req->sample_rate);
		return -EINVAL;
	}

	if ((jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV422_H2V1) &&
		(((((jpu_req->region_info.right - jpu_req->region_info.left) + 1) /
			jpu_req->sample_rate) % JPGDEC_EVEN_NUM) != 0)) {
		hisi_jpu_err("region[%u %u] width invalid\n",
			jpu_req->region_info.left, jpu_req->region_info.right);
		return -EINVAL;
	}

	if (((jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV420) ||
		(jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV444)) &&
		(((((jpu_req->region_info.right - jpu_req->region_info.left) + 1) /
			jpu_req->sample_rate) % JPGDEC_EVEN_NUM) ||
		((((jpu_req->region_info.bottom - jpu_req->region_info.top) + 1) /
			jpu_req->sample_rate) % JPGDEC_EVEN_NUM))) {
		hisi_jpu_err("region[%u %u %u %u] width or height invalid, "
			"jpu_req->sample_rate %d\n", jpu_req->region_info.left,
			jpu_req->region_info.right, jpu_req->region_info.top,
			jpu_req->region_info.bottom, jpu_req->sample_rate);
		return -EINVAL;
	}

	/* 440 height should be even num */
	if ((jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV422_H1V2) &&
		(((((jpu_req->region_info.bottom - jpu_req->region_info.top) + 1) /
			jpu_req->sample_rate) % JPGDEC_EVEN_NUM) != 0)) {
		hisi_jpu_err("region[%u %u %u %u] height invalid, sample_rate %d\n",
			jpu_req->region_info.left, jpu_req->region_info.right,
			jpu_req->region_info.top, jpu_req->region_info.bottom,
			jpu_req->sample_rate);
		return -EINVAL;
	}

	return 0;
}

static int hisijpu_check_format(const struct jpu_data_t *jpu_req)
{
	bool cond = (jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV420) &&
		((jpu_req->in_img_format == HISI_JPEG_DECODE_RAW_YUV422_H2V1) ||
		(jpu_req->in_img_format == HISI_JPEG_DECODE_RAW_YUV444)) &&
		(jpu_req->sample_rate == HISI_JPEG_DECODE_SAMPLE_SIZE_8);
	if (cond) {
		hisi_jpu_info("sample_rate %d, in_img_format %d\n",
			jpu_req->sample_rate, jpu_req->in_img_format);
		return -1;
	}

	return 0;
}

static void hisijpu_dump_info(const struct jpu_data_t *jpu_info)
{
	uint32_t i;

	hisi_jpu_info("informat %d, outformat %d\n",
		jpu_info->in_img_format, jpu_info->out_color_format);

	hisi_jpu_info("input buffer: w:%u h:%u align_w:%u align_h:%u\n"
		"num_compents:%u, sample_rate:%d\n",
		jpu_info->inwidth, jpu_info->inheight,
		jpu_info->pix_width, jpu_info->pix_height,
		jpu_info->num_components, jpu_info->sample_rate);

	hisi_jpu_info("stride_y: %u, stride_c:%u, restart_interval:%u\n",
		jpu_info->stride_y, jpu_info->stride_c, jpu_info->restart_interval);

	hisi_jpu_info("region[ %u %u %u %u ]\n",
		jpu_info->region_info.left, jpu_info->region_info.right,
		jpu_info->region_info.top, jpu_info->region_info.bottom);

	for (i = 0; i < jpu_info->num_components; i++)
		hisi_jpu_info("i = %u, component_id:%u, component_index:%u\n"
			"quant_tbl_num:%u, dc_tbl_num:%u, ac_tbl_num:%u\n"
			"hor_sample_fac:%u, ver_sample_fac:%u\n",
			i, jpu_info->component_info[i].component_id,
			jpu_info->component_info[i].component_index,
			jpu_info->component_info[i].quant_tbl_num,
			jpu_info->component_info[i].dc_tbl_num,
			jpu_info->component_info[i].ac_tbl_num,
			jpu_info->component_info[i].hor_sample_fac,
			jpu_info->component_info[i].ver_sample_fac);

	for (i = 0; i < HDC_TABLE_NUM; i++)
		hisi_jpu_debug("hdc_tab %u\n", jpu_info->hdc_tab[i]);

	for (i = 0; i < HAC_TABLE_NUM; i++) {
		hisi_jpu_debug("i:%d hac_min_tab:%u\n", i, jpu_info->hac_min_tab[i]);
		hisi_jpu_debug("i:%d hac_base_tab:%u\n", i, jpu_info->hac_base_tab[i]);
	}

	for (i = 0; i < HAC_SYMBOL_TABLE_NUM; i++)
		hisi_jpu_debug("hdc_tab %u\n", jpu_info->hac_symbol_tab[i]);

	for (i = 0; i < MAX_DCT_SIZE; i++)
		hisi_jpu_debug("qvalue %d\n", jpu_info->qvalue[i]);
}

static int hisijpu_check_userdata(struct hisi_jpu_data_type *hisijd,
	struct jpu_data_t *jpu_req)
{
	bool cond = (jpu_req->num_components == 0) ||
		(jpu_req->num_components > PIXEL_COMPONENT_NUM);
	if (cond) {
		hisi_jpu_err("the num_components %u is out of range\n", jpu_req->num_components);
		return -EINVAL;
	}

	if (jpu_req->decode_mode >= HISI_JPEG_DECODE_MODE_MAX) {
		hisi_jpu_err("the image decode_mode = %d is out of range\n", jpu_req->decode_mode);
		return -EINVAL;
	}

	/* equal to jpu_req->progressive_mode || jpu_req->arith_code */
	cond = (*((char *)(&(jpu_req->addr_offset)) + sizeof(jpu_req->addr_offset))) ||
		(*((char *)(&(jpu_req->addr_offset)) + sizeof(jpu_req->addr_offset) + sizeof(bool)));
	if (cond) {
		hisi_jpu_err("not support progressive mode and arith_code mode\n");
		return -EINVAL;
	}

	cond = (hisijd->jpu_support_platform == HISI_DSS_V400) ||
		(hisijd->jpu_support_platform == HISI_DSS_V500);
	if (cond) {
		/* is format can handle for chip limit */
		if (hisijpu_check_format(jpu_req) != 0) {
			hisi_jpu_err("this format not support 8 sample\n");
			return -EINVAL;
		}
	}

	if (hisijpu_check_buffers(hisijd, jpu_req) != 0) {
		hisi_jpu_err("hisijpu check_buffers fail\n");
		return -EINVAL;
	}

	if (hisijpu_check_region_decode_info(jpu_req) != 0) {
		hisi_jpu_err("check region decode info error\n");
		return -EINVAL;
	}

	if (hisijpu_check_full_decode_info(jpu_req) != 0) {
		hisi_jpu_err("check full decode info error\n");
		return -EINVAL;
	}

	if (g_debug_jpu_dec != 0)
		hisijpu_dump_info(jpu_req);

	hisi_jpu_info("data check ok, input format %d, output format %d, "
		"sample size %d\n", jpu_req->in_img_format, jpu_req->out_color_format,
		jpu_req->sample_rate);

	return 0;
}

static int hisi_jpu_dec_set_unreset(struct hisi_jpu_data_type *hisijd)
{
	jpu_check_null_return(hisijd->jpu_top_base, -EINVAL);

	/* module reset */
	outp32(hisijd->jpu_top_base + JPGDEC_CRG_CFG1, 0x1);
	outp32(hisijd->jpu_top_base + JPGDEC_CRG_CFG1, 0x0);

	return 0;
}

static int hisi_jpu_dec_set_cvdr(const struct hisi_jpu_data_type *hisijd)
{
	jpu_check_null_return(hisijd->jpu_cvdr_base, -EINVAL);

	if ((hisijd->jpu_support_platform == HISI_DSS_V400) ||
		(hisijd->jpu_support_platform == HISI_DSS_V501))
		outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_CVDR_CFG,
			JPGDEC_CVDR_AXI_JPEG_CVDR_CFG_VAL);

	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_WR_CFG1,
		AXI_JPEG_CVDR_NR_WR_CFG_VAL);
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_WR_CFG2,
		AXI_JPEG_CVDR_NR_WR_CFG_VAL);

	/* NRRD1 */
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_LIMITER_RD_CFG1,
		JPGDEC_NRRD_ACCESS_LIMITER1_VAL);
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_RD_CFG1,
		AXI_JPEG_CVDR_NR_RD_CFG_VAL);

	/* NRRD2 */
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_LIMITER_RD_CFG2,
		JPGDEC_NRRD_ACCESS_LIMITER2_VAL);
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_RD_CFG2,
		AXI_JPEG_CVDR_NR_RD_CFG_VAL);

	/*
	 * Wr_qos_max:0x1;wr_qos_threshold_01_start:0x1;
	 * wr_qos_threshold_01_stop:0x1,
	 * WR_QOS&RD_QOS encode will also set this
	 */
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_CVDR_WR_QOS_CFG,
		AXI_JPEG_CVDR_WR_QOS_CFG_VAL);
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_CVDR_RD_QOS_CFG,
		AXI_JPEG_CVDR_WR_QOS_CFG_VAL);
	return 0;
}

static int hisi_jpu_dec_reg_init(struct hisi_jpu_data_type *hisijd)
{
	errno_t ret;
	/* cppcheck-suppress */
	ret = memcpy_s(&(hisijd->jpu_dec_reg), sizeof(jpu_dec_reg_t),
		&(hisijd->jpu_dec_reg_default), sizeof(jpu_dec_reg_t));
	if (ret != EOK)
		return -EINVAL;
	return 0;
}

static void hisi_jpu_reg_bitstreams_default(
	const struct hisi_jpu_data_type *hisijd, const char __iomem *jpu_dec_base,
	jpu_dec_reg_t *jpu_dec_reg)
{
	bool cond = ((hisijd->jpu_support_platform == HISI_DSS_V501) ||
		(hisijd->jpu_support_platform == HISI_DSS_V510) ||
		(hisijd->jpu_support_platform == HISI_DSS_V510_CS) ||
		(hisijd->jpu_support_platform == HISI_DSS_V600));
	if (cond) {
		jpu_dec_reg->bitstreams_start_h =
			inp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_START_H);

		jpu_dec_reg->bitstreams_start =
			inp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_START_L);

		jpu_dec_reg->bitstreams_end_h =
			inp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_END_H);

		jpu_dec_reg->bitstreams_end =
			inp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_END_L);
	} else {
		jpu_dec_reg->bitstreams_start =
			inp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_START);

		jpu_dec_reg->bitstreams_end =
			inp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_END);
	}
}

static int hisi_jpu_dec_reg_default(struct hisi_jpu_data_type *hisijd)
{
	jpu_dec_reg_t *jpu_dec_reg = NULL;
	char __iomem *jpu_dec_base = NULL;

	jpu_check_null_return(hisijd->jpu_dec_base, -EINVAL);

	jpu_dec_base = hisijd->jpu_dec_base;
	jpu_dec_reg = &(hisijd->jpu_dec_reg_default);

	/* cppcheck-suppress * */
	(void)memset_s(jpu_dec_reg, sizeof(jpu_dec_reg_t), 0, sizeof(jpu_dec_reg_t));

	jpu_dec_reg->dec_start = inp32(jpu_dec_base + JPEG_DEC_START);
	jpu_dec_reg->preftch_ctrl = inp32(jpu_dec_base + JPEG_DEC_PREFTCH_CTRL);
	jpu_dec_reg->frame_size = inp32(jpu_dec_base + JPEG_DEC_FRAME_SIZE);
	jpu_dec_reg->crop_horizontal = inp32(jpu_dec_base + JPEG_DEC_CROP_HORIZONTAL);
	jpu_dec_reg->crop_vertical = inp32(jpu_dec_base + JPEG_DEC_CROP_VERTICAL);

	hisi_jpu_reg_bitstreams_default(hisijd, jpu_dec_base, jpu_dec_reg);

	jpu_dec_reg->frame_start_y = inp32(jpu_dec_base + JPEG_DEC_FRAME_START_Y);
	jpu_dec_reg->frame_stride_y = inp32(jpu_dec_base + JPEG_DEC_FRAME_STRIDE_Y);
	jpu_dec_reg->frame_start_c = inp32(jpu_dec_base + JPEG_DEC_FRAME_START_C);
	jpu_dec_reg->frame_stride_c = inp32(jpu_dec_base + JPEG_DEC_FRAME_STRIDE_C);
	jpu_dec_reg->lbuf_start_addr = inp32(jpu_dec_base + JPEG_DEC_LBUF_START_ADDR);
	jpu_dec_reg->output_type = inp32(jpu_dec_base + JPEG_DEC_OUTPUT_TYPE);
	jpu_dec_reg->freq_scale = inp32(jpu_dec_base + JPEG_DEC_FREQ_SCALE);
	jpu_dec_reg->middle_filter = inp32(jpu_dec_base + JPEG_DEC_MIDDLE_FILTER);
	jpu_dec_reg->sampling_factor = inp32(jpu_dec_base + JPEG_DEC_SAMPLING_FACTOR);
	jpu_dec_reg->dri = inp32(jpu_dec_base + JPEG_DEC_DRI);
	jpu_dec_reg->over_time_thd = inp32(jpu_dec_base + JPEG_DEC_OVER_TIME_THD);
	jpu_dec_reg->hor_phase0_coef01 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF01);
	jpu_dec_reg->hor_phase0_coef23 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF23);
	jpu_dec_reg->hor_phase0_coef45 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF45);
	jpu_dec_reg->hor_phase0_coef67 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF67);
	jpu_dec_reg->hor_phase2_coef01 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF01);
	jpu_dec_reg->hor_phase2_coef23 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF23);
	jpu_dec_reg->hor_phase2_coef45 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF45);
	jpu_dec_reg->hor_phase2_coef67 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF67);
	jpu_dec_reg->ver_phase0_coef01 = inp32(jpu_dec_base + JPEG_DEC_VER_PHASE0_COEF01);
	jpu_dec_reg->ver_phase0_coef23 = inp32(jpu_dec_base + JPEG_DEC_VER_PHASE0_COEF23);
	jpu_dec_reg->ver_phase2_coef01 = inp32(jpu_dec_base + JPEG_DEC_VER_PHASE2_COEF01);
	jpu_dec_reg->ver_phase2_coef23 = inp32(jpu_dec_base + JPEG_DEC_VER_PHASE2_COEF23);
	jpu_dec_reg->csc_in_dc_coef = inp32(jpu_dec_base + JPEG_DEC_CSC_IN_DC_COEF);
	jpu_dec_reg->csc_out_dc_coef = inp32(jpu_dec_base + JPEG_DEC_CSC_OUT_DC_COEF);
	jpu_dec_reg->csc_trans_coef0 = inp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF0);
	jpu_dec_reg->csc_trans_coef1 = inp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF1);
	jpu_dec_reg->csc_trans_coef2 = inp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF2);
	jpu_dec_reg->csc_trans_coef3 = inp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF3);
	jpu_dec_reg->csc_trans_coef4 = inp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF4);

	return 0;
}

static void hisi_jpu_addr_control(const struct hisi_jpu_data_type *hisijd,
	jpu_dec_reg_t *pjpu_dec_reg, const struct jpu_data_t *jpu_req)
{
	/* prefetch control */
	if (hisijd->jpu_support_platform == HISI_DSS_V600) {
		pjpu_dec_reg->preftch_ctrl = jpu_set_bits32(pjpu_dec_reg->preftch_ctrl,
			0x1, REG_SET_32_BIT, 0);
	} else {
		/* jpu_req->smmu_enable */
		pjpu_dec_reg->preftch_ctrl = jpu_set_bits32(pjpu_dec_reg->preftch_ctrl,
			(*((char *)(&(jpu_req->addr_offset)) + sizeof(jpu_req->addr_offset) +
			/* 2 is addr offset of code and mode */
		 	sizeof(bool) * 2)) ? 0x0 : 0x1, REG_SET_32_BIT, 0);
	}

	/* define reset interval */
	pjpu_dec_reg->dri = jpu_set_bits32(pjpu_dec_reg->dri,
		jpu_req->restart_interval, REG_SET_32_BIT, 0);

	/* frame size */
	pjpu_dec_reg->frame_size = jpu_set_bits32(pjpu_dec_reg->frame_size,
		((jpu_req->pix_width - 1) | ((jpu_req->pix_height - 1) <<
			SHIFT_16_BIT)), REG_SET_32_BIT, 0);

	/* input bitstreams addr */
	if (hisijd->jpu_support_platform == HISI_DSS_V501 ||
		hisijd->jpu_support_platform == HISI_DSS_V510 ||
		hisijd->jpu_support_platform == HISI_DSS_V510_CS ||
		hisijd->jpu_support_platform == HISI_DSS_V600)
		pjpu_dec_reg->bitstreams_start_h = (jpu_req->start_addr >>
			SHIFT_32_BIT) & 0x3;

	pjpu_dec_reg->bitstreams_start = jpu_set_bits32(pjpu_dec_reg->bitstreams_start,
		(uint32_t)jpu_req->start_addr, REG_SET_32_BIT, 0);

	if (hisijd->jpu_support_platform == HISI_DSS_V501 ||
		hisijd->jpu_support_platform == HISI_DSS_V510 ||
		hisijd->jpu_support_platform == HISI_DSS_V510_CS ||
		hisijd->jpu_support_platform == HISI_DSS_V600)
		pjpu_dec_reg->bitstreams_end_h = (jpu_req->end_addr >>
			REG_SET_32_BIT) & 0x3;

	pjpu_dec_reg->bitstreams_end = jpu_set_bits32(pjpu_dec_reg->bitstreams_end,
		(uint32_t)jpu_req->end_addr, REG_SET_32_BIT, 0);
	hisi_jpu_start_addr_control(hisijd, pjpu_dec_reg, jpu_req);
}

static int hisi_jpu_userdata_and_reg_init(struct hisi_jpu_data_type *hisijd,
	const jpu_dec_reg_t *pjpu_dec_reg, struct jpu_data_t *jpu_req,
	const void __user *argp)
{
	unsigned long retval;

	retval = copy_from_user(jpu_req, argp, sizeof(struct jpu_data_t));
	if (retval != 0) {
		hisi_jpu_err("copy_from_user failed\n");
		return -EINVAL;
	}

	if (hisijpu_check_userdata(hisijd, jpu_req) != 0) {
		hisi_jpu_err("hisijpu_check_userdata failed\n");
		return -EINVAL;
	}

	if (!hisijd->jpu_res_initialized) {
		if (hisi_jpu_dec_reg_default(hisijd) != 0) {
			hisi_jpu_err("hisi_jpu_dec_reg_default failed\n");
			return -EINVAL;
		}
		hisijd->jpu_res_initialized = true;
	}

	if (hisi_jpu_dec_reg_init(hisijd) != 0) {
		hisi_jpu_err("hisi_jpu_dec_reg_init failed\n");
		return -EINVAL;
	}

	if (hisi_jpu_dec_set_cvdr(hisijd) != 0) {
		hisi_jpu_err("hisi_jpu_dec_set_cvdr failed\n");
		return -EINVAL;
	}

	if (hisi_jpu_dec_set_unreset(hisijd) != 0) {
		hisi_jpu_err("hisi_jpu_dec_set_unreset failed\n");
		return -EINVAL;
	}
	return 0;
}

static void hisi_jpu_unlock_usecase(const struct hisi_jpu_data_type *hisijd)
{
#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510)
	int ret;

	if (hisijd->jpgd_drv && hisijd->jpgd_drv->unlock_usecase) {
		hisi_jpu_info("enter unlock usecase\n");

		ret = hisijd->jpgd_drv->unlock_usecase(hisijd->jpgd_drv);
		if (ret != 0)
			hisi_jpu_err("failed to unlock usecase ret is %d\n", ret);
	}
#endif
}

static int hisi_jpu_lock_usecase(const struct hisi_jpu_data_type *hisijd)
{
#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510)
	int ret;

	if (hisijd->jpgd_drv && hisijd->jpgd_drv->lock_usecase) {
		hisi_jpu_info("enter lock usecase\n");

		ret = hisijd->jpgd_drv->lock_usecase(hisijd->jpgd_drv);
		if (ret != 0) {
			hisi_jpu_err("failed to lock usecase ret is %d\n", ret);
			return ret;
		}
		return ret;
	}
#endif
	return 0;
}

static void hisi_jpu_dec_reg_val(const jpu_dec_reg_t *jpu_dec_reg)
{
	hisi_jpu_info("jpu reg val:dec_start = %u, preftch_ctrl = %u\n"
		"frame_size = %u, crop_horizontal = %u, crop_vertical = %u\n"
		"bitstreams_start_h = %u, bitstreams_start = %u, bitstreams_end_h = %u\n"
		"bitstreams_end = %u, frame_start_y = %u, frame_stride_y = %u\n"
		"frame_start_c = %u, frame_stride_c = %u, lbuf_start_addr = %u\n"
		"output_type = %u, freq_scale = %u, middle_filter = %u\n"
		"sampling_factor = %u, dri = %u, over_time_thd = %u\n"
		"hor_phase0_coef01 = %u, hor_phase0_coef23 = %u\n"
		"hor_phase0_coef45 = %u, hor_phase0_coef67 = %u\n"
		"hor_phase2_coef01 = %u, hor_phase2_coef23 = %u\n"
		"hor_phase2_coef45 = %u, hor_phase2_coef67 = %u\n"
		"ver_phase0_coef01 = %u, ver_phase0_coef23 = %u\n"
		"ver_phase2_coef01 = %u, ver_phase2_coef23 = %u\n"
		"csc_in_dc_coef = %u, csc_out_dc_coef = %u\n"
		"csc_trans_coef0 = %u, csc_trans_coef1 = %u\n"
		"csc_trans_coef2 = %u, csc_trans_coef3 = %u\n"
		"csc_trans_coef4 = %u\n", jpu_dec_reg->dec_start,
		jpu_dec_reg->preftch_ctrl, jpu_dec_reg->frame_size,
		jpu_dec_reg->crop_horizontal, jpu_dec_reg->crop_vertical,
		jpu_dec_reg->bitstreams_start_h, jpu_dec_reg->bitstreams_start,
		jpu_dec_reg->bitstreams_end_h, jpu_dec_reg->bitstreams_end,
		jpu_dec_reg->frame_start_y, jpu_dec_reg->frame_stride_y,
		jpu_dec_reg->frame_start_c, jpu_dec_reg->frame_stride_c,
		jpu_dec_reg->lbuf_start_addr, jpu_dec_reg->output_type,
		jpu_dec_reg->freq_scale, jpu_dec_reg->middle_filter,
		jpu_dec_reg->sampling_factor, jpu_dec_reg->dri,
		jpu_dec_reg->over_time_thd, jpu_dec_reg->hor_phase0_coef01,
		jpu_dec_reg->hor_phase0_coef23, jpu_dec_reg->hor_phase0_coef45,
		jpu_dec_reg->hor_phase0_coef67, jpu_dec_reg->hor_phase2_coef01,
		jpu_dec_reg->hor_phase2_coef23, jpu_dec_reg->hor_phase2_coef45,
		jpu_dec_reg->hor_phase2_coef67, jpu_dec_reg->ver_phase0_coef01,
		jpu_dec_reg->ver_phase0_coef23, jpu_dec_reg->ver_phase2_coef01,
		jpu_dec_reg->ver_phase2_coef23, jpu_dec_reg->csc_in_dc_coef,
		jpu_dec_reg->csc_out_dc_coef, jpu_dec_reg->csc_trans_coef0,
		jpu_dec_reg->csc_trans_coef1, jpu_dec_reg->csc_trans_coef2,
		jpu_dec_reg->csc_trans_coef3, jpu_dec_reg->csc_trans_coef4);
}

static void hisi_jpu_set_reg_bitstreams(const struct hisi_jpu_data_type *hisijd,
	char __iomem *jpu_dec_base, jpu_dec_reg_t *jpu_dec_reg)
{
	bool cond = ((hisijd->jpu_support_platform == HISI_DSS_V501) ||
		(hisijd->jpu_support_platform == HISI_DSS_V510) ||
		(hisijd->jpu_support_platform == HISI_DSS_V510_CS) ||
		(hisijd->jpu_support_platform == HISI_DSS_V600));
	if (cond) {
		outp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_START_H,
			jpu_dec_reg->bitstreams_start_h);
		outp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_START_L,
			jpu_dec_reg->bitstreams_start);
		outp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_END_H,
			jpu_dec_reg->bitstreams_end_h);
		outp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_END_L,
			jpu_dec_reg->bitstreams_end);
	} else {
		outp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_START,
			jpu_dec_reg->bitstreams_start);
		outp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_END,
			jpu_dec_reg->bitstreams_end);
	}
}

static int hisi_jpu_dec_set_reg(struct hisi_jpu_data_type *hisijd,
	jpu_dec_reg_t *jpu_dec_reg)
{
	char __iomem *jpu_dec_base = NULL;

	jpu_dec_base = hisijd->jpu_dec_base;
	jpu_check_null_return(hisijd->jpu_dec_base, -EINVAL);

	if (g_debug_jpu_dec != 0)
		hisi_jpu_dec_reg_val(jpu_dec_reg);
	outp32(jpu_dec_base + JPEG_DEC_PREFTCH_CTRL, jpu_dec_reg->preftch_ctrl);
	outp32(jpu_dec_base + JPEG_DEC_FRAME_SIZE, jpu_dec_reg->frame_size);
	outp32(jpu_dec_base + JPEG_DEC_CROP_HORIZONTAL, jpu_dec_reg->crop_horizontal);
	outp32(jpu_dec_base + JPEG_DEC_CROP_VERTICAL, jpu_dec_reg->crop_vertical);

	hisi_jpu_set_reg_bitstreams(hisijd, jpu_dec_base, jpu_dec_reg);
	outp32(jpu_dec_base + JPEG_DEC_FRAME_START_Y, jpu_dec_reg->frame_start_y);
	outp32(jpu_dec_base + JPEG_DEC_FRAME_STRIDE_Y, jpu_dec_reg->frame_stride_y);
	outp32(jpu_dec_base + JPEG_DEC_FRAME_START_C, jpu_dec_reg->frame_start_c);
	outp32(jpu_dec_base + JPEG_DEC_FRAME_STRIDE_C, jpu_dec_reg->frame_stride_c);
	outp32(jpu_dec_base + JPEG_DEC_LBUF_START_ADDR, jpu_dec_reg->lbuf_start_addr);
	outp32(jpu_dec_base + JPEG_DEC_OUTPUT_TYPE, jpu_dec_reg->output_type);
	outp32(jpu_dec_base + JPEG_DEC_FREQ_SCALE, jpu_dec_reg->freq_scale);
	outp32(jpu_dec_base + JPEG_DEC_MIDDLE_FILTER, jpu_dec_reg->middle_filter);
	outp32(jpu_dec_base + JPEG_DEC_SAMPLING_FACTOR, jpu_dec_reg->sampling_factor);
	outp32(jpu_dec_base + JPEG_DEC_DRI, jpu_dec_reg->dri);
	outp32(jpu_dec_base + JPEG_DEC_OVER_TIME_THD, jpu_dec_reg->over_time_thd);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF01, jpu_dec_reg->hor_phase0_coef01);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF23, jpu_dec_reg->hor_phase0_coef23);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF45, jpu_dec_reg->hor_phase0_coef45);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF67, jpu_dec_reg->hor_phase0_coef67);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF01, jpu_dec_reg->hor_phase2_coef01);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF23, jpu_dec_reg->hor_phase2_coef23);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF45, jpu_dec_reg->hor_phase2_coef45);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF67, jpu_dec_reg->hor_phase2_coef67);
	outp32(jpu_dec_base + JPEG_DEC_VER_PHASE0_COEF01, jpu_dec_reg->ver_phase0_coef01);
	outp32(jpu_dec_base + JPEG_DEC_VER_PHASE0_COEF23, jpu_dec_reg->ver_phase0_coef23);
	outp32(jpu_dec_base + JPEG_DEC_VER_PHASE2_COEF01, jpu_dec_reg->ver_phase2_coef01);
	outp32(jpu_dec_base + JPEG_DEC_VER_PHASE2_COEF23, jpu_dec_reg->ver_phase2_coef23);
	outp32(jpu_dec_base + JPEG_DEC_CSC_IN_DC_COEF, jpu_dec_reg->csc_in_dc_coef);
	outp32(jpu_dec_base + JPEG_DEC_CSC_OUT_DC_COEF, jpu_dec_reg->csc_out_dc_coef);
	outp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF0, jpu_dec_reg->csc_trans_coef0);
	outp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF1, jpu_dec_reg->csc_trans_coef1);
	outp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF2, jpu_dec_reg->csc_trans_coef2);
	outp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF3, jpu_dec_reg->csc_trans_coef3);
	outp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF4, jpu_dec_reg->csc_trans_coef4);

	/* to make sure start reg set at last */
	outp32(jpu_dec_base + JPEG_DEC_START, JPEGDEC_DECODE_START_DISABLE);

	return 0;
}

int hisijpu_job_exec(struct hisi_jpu_data_type *hisijd, const void __user *argp)
{
	int ret;
	struct jpu_data_t *jpu_req = NULL;
	jpu_dec_reg_t *pjpu_dec_reg = NULL;

	hisi_jpu_debug("+\n");
	jpu_check_null_return(hisijd, -EINVAL);
	jpu_check_null_return(argp, -EINVAL);

	jpu_req = &(hisijd->jpu_req);
	pjpu_dec_reg = &(hisijd->jpu_dec_reg);
	down(&hisijd->blank_sem);

	if (hisi_jpu_on(hisijd) != 0) {
		hisi_jpu_err("hisi_jpu_on failed\n");
		ret = -EINVAL;
		goto err_out;
	}

	if (hisi_jpu_lock_usecase(hisijd) != 0) {
		hisi_jpu_err("lock usecase failed\n");
		ret = -EINVAL;
		goto err_out;
	}

	/* input userdata and init reg */
	ret = hisi_jpu_userdata_and_reg_init(hisijd, pjpu_dec_reg, jpu_req, argp);
	if (ret != 0) {
		hisi_jpu_err("hisi_jpu_userdata_and_reg_init failed\n");
		goto err_out;
	}

	/* addr cotrol */
	hisi_jpu_addr_control(hisijd, pjpu_dec_reg, jpu_req);

	/* set paramters such as outtype, scale, dct, etc */
	if (hisi_jpu_set_paramters(hisijd, jpu_req, pjpu_dec_reg) != 0) {
		hisi_jpu_err("hisi_jpu_set_paramters failed\n");
		ret = -EINVAL;
		goto err_out;
	}

	/* start to work */
	pjpu_dec_reg->dec_start = jpu_set_bits32(pjpu_dec_reg->dec_start, 0x1, 1, 0);
	ret = hisi_jpu_dec_set_reg(hisijd, pjpu_dec_reg);
	if (ret != 0) {
		hisi_jpu_err("hisi_jpu_dec_set_reg ret = %d\n", ret);
		goto err_out;
	}

	/* jpu do the decode work */
	ret = hisi_jpu_dec_done(hisijd, jpu_req);
	if (ret != 0)
		hisi_jpu_err("hisi_jpu_dec_done failed ret = %d\n", ret);

err_out:
	hisi_jpu_unlock_usecase(hisijd);

	if (hisi_jpu_off(hisijd) != 0)
		hisi_jpu_err("hisi_jpu_off failed\n");

	up(&hisijd->blank_sem);
	hisi_jpu_debug("-\n");
	return ret;
}

#pragma GCC diagnostic pop