

#include "ipp.h"
#include "config_table_cvdr.h"
#include "cvdr_opt.h"
#include <linux/printk.h>

#define LOG_TAG LOG_MODULE_CVDR_OPT

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

typedef enum _cvdr_dev_e {
	CVDR_RT = 0,
	CVDR_SRT = 1,
} cvdr_dev_e;

#define ISP_CLK   (600)
#define DERATE    (1.2)
#define ISP_BASE_ADDR (0xE8400000)
#define ISP_BASE_ADDR_CVDR_RT         (ISP_BASE_ADDR + 0x00022000)
#define CVDR_RT_LIMITER_VP_RD_10_REG  0x8A8 /* Video port read Access limiter. */
#define CVDR_RT_LIMITER_VP_WR_26_REG  0x468 /* Video port write Access limiter. */
#define CVDR_VP_WR_NBR                (58)
#define ISP_BASE_ADDR_CVDR_SRT        (ISP_BASE_ADDR + 0x0002E000)
#define CVDR_SRT_VP_WR_IF_CFG_0_REG   0x28
#define CVDR_VP_RD_NBR                (28)
#define CVDR_SRT_VP_RD_IF_CFG_0_REG   0x514 /* Video port read interface configuration: prefetch or reset or stall capability. */
#define CVDR_RT_CVDR_CFG_REG          0x0   /* CVDR config register. */

/*lint -e524*/
static cvdr_opt_bw_t cvdr_vp_wr_bw[VP_WR_MAX] = {
	[WR_CMDLST] = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[CPE_WR0]   = {CVDR_SRT, ISP_CLK, (float)DERATE *ISP_CLK * 2},
	[CPE_WR1]   = {CVDR_SRT, ISP_CLK, (float)DERATE *ISP_CLK * 2},
	[CPE_WR2]   = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[WR0_SLAM]  = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[WR1_SLAM]  = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[WR2_SLAM]  = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
};

static cvdr_opt_bw_t cvdr_vp_rd_bw[VP_RD_MAX] = {
	[RD_CMDLST] = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[CPE_RD0]   = {CVDR_SRT, ISP_CLK, (float)DERATE * 720},
	[CPE_RD1]   = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[CPE_RD2]   = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[CPE_RD3]   = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[CPE_RD4]   = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[CPE_RD5]   = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[RD0_SLAM]  = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[RD_RDR_DESC] = {CVDR_SRT, ISP_CLK, ISP_CLK}, //RDR
	[RD_CMP_REF]  = {CVDR_SRT, ISP_CLK, ISP_CLK}, //CMP
};

static cvdr_opt_bw_t cvdr_nr_wr_bw[NR_WR_MAX] = {
	[WR_RDR_DESC]    = {CVDR_SRT, ISP_CLK, ISP_CLK},
};

static cvdr_opt_bw_t cvdr_nr_rd_bw[NR_RD_MAX] = {
	[RD_CMP_CUR]    = {CVDR_SRT, ISP_CLK, ISP_CLK},
};


/*lint +e524*/
int calculate_cvdr_bw_limiter(cvdr_bw_cfg_t *bw, unsigned int throughput)
{
	bw->bw_limiter0       = 0xF;
	bw->bw_limiter1       = 0xF;
	bw->bw_limiter2       = 0xF;
	bw->bw_limiter3       = 0xF;
	bw->bw_limiter_reload = 0xF;

	return 0;
}

int calculate_cvdr_allocated_du(unsigned int pclk, unsigned int throughput, int is_dgen)
{
	return 6;
}

int cfg_tbl_cvdr_nr_wr_cfg(cfg_tab_cvdr_t *p_cvdr_cfg, int id)
{
	p_cvdr_cfg->nr_wr_cfg[id].to_use = 1;
	p_cvdr_cfg->nr_wr_cfg[id].en     = 1;
	calculate_cvdr_bw_limiter(&(p_cvdr_cfg->nr_wr_cfg[id].bw), cvdr_nr_wr_bw[id].throughput);
	return CPE_FW_OK;
}

int cfg_tbl_cvdr_nr_rd_cfg(cfg_tab_cvdr_t *p_cvdr_cfg, int id)
{
	p_cvdr_cfg->nr_rd_cfg[id].to_use = 1;
	p_cvdr_cfg->nr_rd_cfg[id].en     = 1;
	p_cvdr_cfg->nr_rd_cfg[id].allocated_du = 6;
	calculate_cvdr_bw_limiter(&(p_cvdr_cfg->nr_rd_cfg[id].bw), cvdr_nr_rd_bw[id].throughput);
	d("allocated_du: %d, pclk: %d", p_cvdr_cfg->nr_rd_cfg[id].allocated_du, cvdr_nr_rd_bw[id].pclk);
	return CPE_FW_OK;
}

static int dataflow_cvdr_wr_get_fmt_config_num(pix_format_e format, cvdr_pix_fmt_e *pix_fmt, unsigned int *config_num)
{
	switch (format) {
	case PIXEL_FMT_CPE_Y8:
	case PIXEL_FMT_CPE_1PF8:
		*pix_fmt    = DF_1PF8;
		*config_num = 1;
		break;

	case PIXEL_FMT_CPE_2PF8:
		*pix_fmt    = DF_2PF8;
		*config_num = 1;
		break;

	case PIXEL_FMT_CPE_3PF8:
		*pix_fmt    = DF_3PF8;
		break;

	case PIXEL_FMT_CPE_D32:
		*pix_fmt    = DF_D32;
		*config_num = 1;
		break;

	case PIXEL_FMT_CPE_D48:
		*pix_fmt    = DF_D48;
		*config_num = 1;
		break;

	case PIXEL_FMT_CPE_D64:
		*pix_fmt    = DF_D64;
		*config_num = 1;
		break;

	case PIXEL_FMT_CPE_D128:
		*pix_fmt    = DF_D128;
		*config_num = 1;
		break;

	default:
		e("pix_fmt unsupported! format = %d", format);
		return CPE_FW_ERR;
	}

	return CPE_FW_OK;
}

/* when in multple stripes mode, the width in cfmt is the full frame width*/
int dataflow_cvdr_wr_cfg_vp(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt, unsigned int line_stride,
							unsigned int isp_clk, pix_format_e format)
{
	if (NULL == p_cvdr_cfg || NULL == cfmt) {
		e("input param is NULL!");
		return CPE_FW_ERR;
	}

	d("width = %d, height = %d, line_stride = %d, format = %d, pix_fmt = %d\n",
	  cfmt->width, cfmt->height, line_stride, format, cfmt->pix_fmt);
	unsigned int i              = 0;
	unsigned int pix_size       = 0;
	unsigned int config_num     = 0;
	unsigned int total_bytes[2] = {0};
	unsigned int config_width   = 0;
	unsigned int line_bytes     = 0;
	cvdr_pix_fmt_e pix_fmt = DF_1PF8;

	if (dataflow_cvdr_wr_get_fmt_config_num(format, &pix_fmt, &config_num) != CPE_FW_OK)
		return CPE_FW_ERR;

	cfmt->pix_fmt = pix_fmt;
	pix_size = (EXP_PIX == cfmt->expand) ?
			   pix_fmt_info[pix_fmt].expand_size :
			   pix_fmt_info[pix_fmt].compact_size;
	line_bytes   = cfmt->width * pix_size / 8;
	config_width = ipp_align_up(line_bytes, CVDR_ALIGN_BYTES);
	total_bytes[0] = ipp_align_up(config_width * cfmt->height, CVDR_TOTAL_BYTES_ALIGN); // Num DUs per line
	total_bytes[1] = ipp_align_up(config_width * (cfmt->height), CVDR_TOTAL_BYTES_ALIGN);

	for (i = cfmt->id; i < (cfmt->id + config_num); i++) {
		p_cvdr_cfg->vp_wr_cfg[i].to_use             = 1;
		p_cvdr_cfg->vp_wr_cfg[i].id                 = i;
		p_cvdr_cfg->vp_wr_cfg[i].fmt.fs_addr        = cfmt->addr + (i - cfmt->id) * (config_width * cfmt->height);
		p_cvdr_cfg->vp_wr_cfg[i].fmt.last_page      = (p_cvdr_cfg->vp_wr_cfg[i].fmt.fs_addr +
				ipp_align_up(ipp_align_up(cfmt->full_width * pix_size / 8, CVDR_ALIGN_BYTES) * cfmt->height, CVDR_TOTAL_BYTES_ALIGN)) >> 15;
		p_cvdr_cfg->vp_wr_cfg[i].fmt.pix_fmt        = pix_fmt;
		p_cvdr_cfg->vp_wr_cfg[i].fmt.pix_expan      = cfmt->expand;
		p_cvdr_cfg->vp_wr_cfg[i].fmt.line_stride    = ipp_align_up(cfmt->full_width * pix_size / 8,
				CVDR_ALIGN_BYTES) / CVDR_ALIGN_BYTES - 1;
		p_cvdr_cfg->vp_wr_cfg[i].fmt.line_wrap      = DEFAULT_LINE_WRAP;
		calculate_cvdr_bw_limiter(&(p_cvdr_cfg->vp_wr_cfg[i].bw), cvdr_vp_wr_bw[i].throughput);

		if (0 != line_stride) {
			p_cvdr_cfg->vp_wr_cfg[i].fmt.line_stride  = ipp_align_up(line_stride, CVDR_ALIGN_BYTES) / CVDR_ALIGN_BYTES - 1;
			p_cvdr_cfg->vp_wr_cfg[i].fmt.last_page      = (p_cvdr_cfg->vp_wr_cfg[i].fmt.fs_addr +
					ipp_align_up(ipp_align_up(line_stride, CVDR_ALIGN_BYTES) * cfmt->height, CVDR_TOTAL_BYTES_ALIGN)) >> 15;
		}

	}

	return CPE_FW_OK;
}

static int dataflow_cvdr_rd_get_pix_fmt(pix_format_e format, cvdr_pix_fmt_e *pix_fmt)
{
	switch (format) {
	case PIXEL_FMT_CPE_Y8:
	case PIXEL_FMT_CPE_1PF8:
		*pix_fmt    = DF_1PF8;
		break;

	case PIXEL_FMT_CPE_2PF8:
		*pix_fmt    = DF_2PF8;
		break;

	case PIXEL_FMT_CPE_3PF8:
		*pix_fmt    = DF_3PF8;
		break;

	case PIXEL_FMT_CPE_D32:
		*pix_fmt    = DF_D32;
		break;

	case PIXEL_FMT_CPE_D48:
		*pix_fmt    = DF_D48;
		break;

	case PIXEL_FMT_CPE_D64:
		*pix_fmt    = DF_D64;
		break;

	case PIXEL_FMT_CPE_D128:
		*pix_fmt    = DF_D128;
		break;

	default:
		e("pix_fmt unsupported! format = %d", format);
		return CPE_FW_ERR;
	}
	return CPE_FW_OK;
}

/* by now we just read RAW data */
/* when in multple stripes mode, the width in cfmt is the full frame width*/
int dataflow_cvdr_rd_cfg_vp(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt, unsigned int line_stride,
							unsigned int isp_clk, pix_format_e format)
{
	if (NULL == p_cvdr_cfg || NULL == cfmt) {
		e("input param is NULL!");
		return CPE_FW_ERR;
	}

	unsigned int pix_size      = 0;
	unsigned int id            = cfmt->id;
	unsigned int line_bytes    = 0;
	cvdr_pix_fmt_e pix_fmt     = DF_FMT_INVALID;

	if (dataflow_cvdr_rd_get_pix_fmt(format, &pix_fmt) != CPE_FW_OK)
		return CPE_FW_ERR;

	cfmt->pix_fmt = pix_fmt;
	pix_size = (EXP_PIX == cfmt->expand) ?
			   pix_fmt_info[pix_fmt].expand_size :
			   pix_fmt_info[pix_fmt].compact_size;
	line_bytes = cfmt->width * pix_size / 8;
	unsigned int allocated_du = calculate_cvdr_allocated_du(cvdr_vp_rd_bw[id].pclk, cvdr_vp_rd_bw[id].throughput, 0);
	p_cvdr_cfg->vp_rd_cfg[id].to_use                 = 1;
	p_cvdr_cfg->vp_rd_cfg[id].id                     = id;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.fs_addr            = cfmt->addr;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.last_page          = (cfmt->addr +
			ipp_align_up(ipp_align_up(cfmt->full_width * pix_size / 8, CVDR_ALIGN_BYTES) * cfmt->height, CVDR_TOTAL_BYTES_ALIGN)) >> 15;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.pix_fmt            = pix_fmt;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.pix_expan          = cfmt->expand;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.allocated_du       = allocated_du;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.line_size          = cfmt->line_size - 1;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.hblank             = 0;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.frame_size         = cfmt->height - 1;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.vblank             = 0;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.line_stride        = ipp_align_up(cfmt->full_width * pix_size / 8,
			CVDR_ALIGN_BYTES) / CVDR_ALIGN_BYTES - 1;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.line_wrap          = DEFAULT_LINE_WRAP;

	if (0 != line_stride) {
		p_cvdr_cfg->vp_rd_cfg[id].fmt.line_stride  = ipp_align_up(line_stride, CVDR_ALIGN_BYTES) / CVDR_ALIGN_BYTES - 1;
		p_cvdr_cfg->vp_rd_cfg[id].fmt.last_page   = (cfmt->addr +
				ipp_align_up(ipp_align_up(line_stride, CVDR_ALIGN_BYTES) * cfmt->height, CVDR_TOTAL_BYTES_ALIGN)) >> 15;
	}

	calculate_cvdr_bw_limiter(&(p_cvdr_cfg->vp_rd_cfg[id].bw), cvdr_vp_rd_bw[id].throughput);
	return CPE_FW_OK;
}

int dataflow_cvdr_wr_cfg_d32(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt,
							 unsigned int total_bytes)
{
	if (NULL == p_cvdr_cfg || NULL == cfmt) {
		e("input param is NULL!");
		return CPE_FW_ERR;
	}

	unsigned int id = cfmt->id;
	p_cvdr_cfg->vp_wr_cfg[id].to_use             = 1;
	p_cvdr_cfg->vp_wr_cfg[id].id                 = id;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.fs_addr        = cfmt->addr;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.last_page      = (p_cvdr_cfg->vp_wr_cfg[id].fmt.fs_addr + total_bytes) >> 15;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.pix_fmt        = DF_D32;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.pix_expan      = 1;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.line_stride    = 0;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.line_wrap      = DEFAULT_LINE_WRAP;
	calculate_cvdr_bw_limiter(&(p_cvdr_cfg->vp_wr_cfg[id].bw), cvdr_vp_wr_bw[id].throughput);
	return 0;
}


int cfg_tbl_cvdr_rd_cfg_d64(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt, unsigned int total_bytes,
							unsigned int line_stride)
{
	if (NULL == p_cvdr_cfg || NULL == cfmt) {
		d("input param is NULL!");
		return -1;
	}

	unsigned int id = cfmt->id;
	unsigned int allocated_du = 2;
	p_cvdr_cfg->vp_rd_cfg[id].to_use                 = 1;
	p_cvdr_cfg->vp_rd_cfg[id].id                     = cfmt->id;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.fs_addr            = cfmt->addr;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.last_page          = (cfmt->addr + total_bytes) >> 15;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.pix_fmt            = DF_D64;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.pix_expan          = 1;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.allocated_du       = allocated_du;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.line_size          = cfmt->line_size - 1;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.hblank             = 0;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.frame_size         = cfmt->height - 1;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.vblank             = 0;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.line_stride        = (line_stride > 0) ? (line_stride - 1) : 0;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.line_wrap          = DEFAULT_LINE_WRAP;
	calculate_cvdr_bw_limiter(&(p_cvdr_cfg->vp_rd_cfg[id].bw), cvdr_vp_rd_bw[id].throughput);
	return 0;
}

int cfg_tbl_cvdr_wr_cfg_d64(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt, unsigned int total_bytes)
{
	if (NULL == p_cvdr_cfg || NULL == cfmt) {
		e("input param is NULL!");
		return -1;
	}

	unsigned int id = cfmt->id;
	p_cvdr_cfg->vp_wr_cfg[id].to_use             = 1;
	p_cvdr_cfg->vp_wr_cfg[id].id                 = id;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.fs_addr        = cfmt->addr;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.last_page      = (p_cvdr_cfg->vp_wr_cfg[id].fmt.fs_addr + total_bytes) >> 15;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.pix_fmt        = DF_D64;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.pix_expan      = 1;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.line_stride    = 0;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.line_wrap      = DEFAULT_LINE_WRAP;
	calculate_cvdr_bw_limiter(&(p_cvdr_cfg->vp_wr_cfg[id].bw), cvdr_vp_wr_bw[id].throughput);
	return 0;
}

#pragma GCC diagnostic pop
/****************************end************************************** */

