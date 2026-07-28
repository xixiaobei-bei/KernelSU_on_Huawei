

#include "ipp.h"
#include "config_table_cvdr.h"
#include "cvdr_opt.h"
#include <linux/printk.h>

#define LOG_TAG LOG_MODULE_CVDR_OPT

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

typedef enum _cvdr_dev_e
{
    CVDR_RT = 0,
    CVDR_SRT = 1,
} cvdr_dev_e;

#define ISP_CLK   (600) //(32)
#define DERATE    (1.2)
#define ISP_BASE_ADDR (0xE8400000)
#define ISP_BASE_ADDR_CVDR_RT                  (ISP_BASE_ADDR + 0x00022000)
#define CVDR_RT_LIMITER_VP_RD_10_REG  0x8A8 /* Video port read Access limiter. */
#define CVDR_RT_LIMITER_VP_WR_26_REG  0x468 /* Video port write Access limiter. */
#define CVDR_VP_WR_NBR    (58)
#define ISP_BASE_ADDR_CVDR_SRT                 (ISP_BASE_ADDR + 0x0002E000)
#define CVDR_SRT_VP_WR_IF_CFG_0_REG    0x28
#define CVDR_VP_RD_NBR    (28)
#define CVDR_SRT_VP_RD_IF_CFG_0_REG    0x514 /* Video port read interface configuration: prefetch or reset or stall capability. */
#define CVDR_RT_CVDR_CFG_REG          0x0   /* CVDR config register. */

/*lint -e524*/
static cvdr_opt_bw_t cvdr_vp_wr_bw[WR_MAX] =
{
    [WR0_UV]  = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK*2}, //IOTAP_1_1
    [WR1_Y]  = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK*2}, //IOTAP_1_2
    [WR2_C]   = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK}, //SCALER
    [WR_CMDLST]   = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK}, //SCALER
    [WR0_SLAM]   = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK}, //SCALER
    [WR1_SLAM]   = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK}, //SCALER
    [WR2_SLAM]   = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK}, //SCALER
};

static cvdr_opt_bw_t cvdr_vp_rd_bw[RD_MAX] =
{
    [RD0_L0]   = {CVDR_SRT, ISP_CLK, (float)DERATE*720}, //FBDYUV_TNR
    [RD1_L1]  = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK}, //IOTAP
    [RD2_L2]  = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK}, //IOTAP
    [RD3_MONO_DS4]  = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK}, //IOTAP
    [RD4_MONO]  = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK}, //IOTAP
    [RD5_COL_Y_DS4]  = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK}, //BLC1
    [RD_CMDLST]  = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK}, //BLC1
    [RD0_SLAM]  = {CVDR_SRT, ISP_CLK, (float)DERATE*ISP_CLK}, //BLC1  
};
/*lint +e524*/

int calculate_cvdr_bw_limiter(cvdr_bw_cfg_t *bw, unsigned int throughput)
{
    int num_75mbytes;

    num_75mbytes          = (throughput + 74) / 75;

    bw->bw_limiter0       = num_75mbytes / 4;
    bw->bw_limiter1       = bw->bw_limiter0 + ((num_75mbytes % 4) / 2);
    bw->bw_limiter2       = bw->bw_limiter0 + ((num_75mbytes % 4) % 2);
    bw->bw_limiter3       = bw->bw_limiter1;
    bw->bw_limiter_reload = 0xF;
    pr_info("bw_limiter: %d, %d, %d, %d, reload: %x, throughput: %d",
            bw->bw_limiter0,
            bw->bw_limiter1,
            bw->bw_limiter2,
            bw->bw_limiter3,
            bw->bw_limiter_reload,
            throughput
        );

    return 0;
}

int calculate_cvdr_allocated_du(unsigned int pclk, unsigned int throughput, int is_dgen)
{
    unsigned int num_du;

    if (throughput > 0)
    {
//        num_du = ((int)(1.1*throughput) + 127) / 128 + 1;
    }
    else
    {
        num_du = 0;
    }
    return 6;
    //return num_du;
}

/* when in multple stripes mode, the width in cfmt is the full frame width*/
int dataflow_cvdr_wr_cfg_vp(cfg_tab_cvdr_t* p_cvdr_cfg, cvdr_opt_fmt_t* cfmt, unsigned int line_stride, unsigned int isp_clk, pix_format_e format)
{
    D("[%s] +\n", __func__);
    if(NULL == p_cvdr_cfg || NULL == cfmt)
    {
        pr_err("input param is NULL!");
        return CPE_FW_ERR;
    }
    pr_info("width = %d, height = %d, line_stride = %d, format = %d, pix_fmt = %d",
            cfmt->width, cfmt->height, line_stride, format, cfmt->pix_fmt);

    unsigned int i              = 0;
    unsigned int pix_size       = 0;
    unsigned int id             = cfmt->id;
    unsigned int config_num     = 0;
    unsigned int total_bytes[2] = {0};
    unsigned int config_width   = 0;
    unsigned int line_bytes     = 0;
    cvdr_pix_fmt_e pix_fmt = DF_1PF8;

    switch(format)
    {
        case PIXEL_FMT_CPE_Y8:
        case PIXEL_FMT_CPE_C8:
        case PIXEL_FMT_CPE_G8:
        case PIXEL_FMT_CPE_LF_HF:
            pix_fmt    = DF_1PF8;
            config_num = 1;
            break;
        case PIXEL_FMT_CPE_RM_PACKED_Y:
        case PIXEL_FMT_CPE_PACKED_UV:
        case PIXEL_FMT_CPE_A_B:
		case PIXEL_FMT_CPE_SLAM_OUT:
		case PIXEL_FMT_CPE_SLAM_SCORE:
            pix_fmt    = DF_2PF8;
            config_num = 1;
            break;
        case PIXEL_FMT_CPE_WARP_UVC:
        case PIXEL_FMT_CPE_PACKED_YUV:
            pix_fmt    = DF_3PF8;
            break;
        case PIXEL_FMT_CPE_SLAM_STAT:
            pix_fmt    = DF_D32;
			config_num = 1;
            break;
        case PIXEL_FMT_CPE_RM_PACKED_YUV:
            pix_fmt    = DF_D48;
            config_num = 1;
            break;
        default:
            pr_err("pix_fmt unsupported! format = %d", format);
            return CPE_FW_ERR;
    }

    cfmt->pix_fmt = pix_fmt;
    pix_size = (EXP_PIX == cfmt->expand) ?
                pix_fmt_info[pix_fmt].expand_size :
                pix_fmt_info[pix_fmt].compact_size;

    line_bytes   = cfmt->width * pix_size /8;
    config_width = ALIGN_UP(line_bytes, CVDR_ALIGN_BYTES);

    total_bytes[0] = ALIGN_UP(config_width * cfmt->height, CVDR_TOTAL_BYTES_ALIGN); // Num DUs per line
    total_bytes[1] = ALIGN_UP(config_width * (cfmt->height), CVDR_TOTAL_BYTES_ALIGN);

    for(i = id; i < (id + config_num); i++)
    {
        p_cvdr_cfg->vp_wr_cfg[i].to_use             = 1;
        p_cvdr_cfg->vp_wr_cfg[i].id                 = i;
        p_cvdr_cfg->vp_wr_cfg[i].fmt.fs_addr        = cfmt->addr + (i - id)*(config_width * cfmt->height);
        p_cvdr_cfg->vp_wr_cfg[i].fmt.last_page      = (p_cvdr_cfg->vp_wr_cfg[i].fmt.fs_addr + 
            ALIGN_UP(ALIGN_UP(cfmt->full_width * pix_size /8, CVDR_ALIGN_BYTES) * cfmt->height, CVDR_TOTAL_BYTES_ALIGN)) >> 15;

        p_cvdr_cfg->vp_wr_cfg[i].fmt.pix_fmt        = pix_fmt;
        p_cvdr_cfg->vp_wr_cfg[i].fmt.pix_expan      = cfmt->expand;
        p_cvdr_cfg->vp_wr_cfg[i].fmt.line_stride    = ALIGN_UP(cfmt->full_width * pix_size /8, CVDR_ALIGN_BYTES) / CVDR_ALIGN_BYTES - 1;
        p_cvdr_cfg->vp_wr_cfg[i].fmt.line_wrap      = DEFAULT_LINE_WRAP;
        calculate_cvdr_bw_limiter(&(p_cvdr_cfg->vp_wr_cfg[i].bw), cvdr_vp_wr_bw[i].throughput);

        D("fs_addr = 0x%x, id = %d, line_stride = %d, line_wrap = %d, config_width  = %d,"
              "last_page = 0x%x, total_byte = %d, pix_fmt = %d",
              p_cvdr_cfg->vp_wr_cfg[i].fmt.fs_addr, i, p_cvdr_cfg->vp_wr_cfg[i].fmt.line_stride,
              cfmt->height, config_width, p_cvdr_cfg->vp_wr_cfg[i].fmt.last_page,
              total_bytes[i-id], p_cvdr_cfg->vp_wr_cfg[i].fmt.pix_fmt);

    }
    D("[%s] -\n", __func__);

    return CPE_FW_OK;
}

/* by now we just read RAW data */
/* when in multple stripes mode, the width in cfmt is the full frame width*/
int dataflow_cvdr_rd_cfg_vp(cfg_tab_cvdr_t* p_cvdr_cfg, cvdr_opt_fmt_t* cfmt, unsigned int line_stride, unsigned int isp_clk, pix_format_e format)
{
    D("[%s] +\n", __func__);
    if(NULL == p_cvdr_cfg || NULL == cfmt)
    {
        pr_err("input param is NULL!");
        return CPE_FW_ERR;
    }

    unsigned int pix_size      = 0;
    unsigned int config_width  = 0;
    unsigned int id            = cfmt->id;
    unsigned int line_bytes    = 0;
    cvdr_pix_fmt_e pix_fmt     = DF_FMT_INVALID;

    pr_info("format=%d  id=%d\n", format, cfmt->id);

    switch(format)
    {
        case PIXEL_FMT_CPE_Y8:
        case PIXEL_FMT_CPE_C8:
        case PIXEL_FMT_CPE_G8:
        case PIXEL_FMT_CPE_LF_HF:
            pix_fmt    = DF_1PF8;
            break;
        case PIXEL_FMT_CPE_RM_PACKED_Y:
        case PIXEL_FMT_CPE_PACKED_UV:
        case PIXEL_FMT_CPE_A_B:
	    case PIXEL_FMT_CPE_SLAM_IN:
		case PIXEL_FMT_CPE_SLAM_OUT:
		case PIXEL_FMT_CPE_SLAM_SCORE:
            pix_fmt    = DF_2PF8;
            break;
        case PIXEL_FMT_CPE_WARP_UVC:
        case PIXEL_FMT_CPE_PACKED_YUV:
            pix_fmt    = DF_3PF8;
            break;
        case PIXEL_FMT_CPE_SLAM_STAT:
            pix_fmt    = DF_D32;
            break;
        case PIXEL_FMT_CPE_RM_PACKED_YUV:
            pix_fmt    = DF_D48;
            break;
        default:
            pr_err("pix_fmt unsupported! format = %d", format);
            return CPE_FW_ERR;

    }

    cfmt->pix_fmt = pix_fmt;

    pr_info("pix_fmt=%d   cfmt->pix_fmt=%d\n", pix_fmt, cfmt->pix_fmt);
    pix_size = (EXP_PIX == cfmt->expand) ?
                pix_fmt_info[pix_fmt].expand_size :
                pix_fmt_info[pix_fmt].compact_size;
                
    pr_info("pix_size=%d  cfmt->pix_fmt=%d\n", pix_size, cfmt->pix_fmt);

    line_bytes = cfmt->width * pix_size /8;
    config_width = ALIGN_UP(line_bytes, CVDR_ALIGN_BYTES);
    pr_info("line_bytes=%d  cfmt->pix_fmt=%d\n", line_bytes, cfmt->pix_fmt);

    unsigned int total_bytes  = ALIGN_UP(config_width * cfmt->height, CVDR_TOTAL_BYTES_ALIGN);
    unsigned int allocated_du = calculate_cvdr_allocated_du(cvdr_vp_rd_bw[id].pclk, cvdr_vp_rd_bw[id].throughput, 0);
    pr_info("total_bytes=%d  allocated_du=%d\n", total_bytes, allocated_du);

    p_cvdr_cfg->vp_rd_cfg[id].to_use                 = 1;
    p_cvdr_cfg->vp_rd_cfg[id].id                     = id;
    p_cvdr_cfg->vp_rd_cfg[id].fmt.fs_addr            = cfmt->addr;
    p_cvdr_cfg->vp_rd_cfg[id].fmt.last_page          = (cfmt->addr + 
            ALIGN_UP(ALIGN_UP(cfmt->full_width * pix_size /8, CVDR_ALIGN_BYTES) * cfmt->height, CVDR_TOTAL_BYTES_ALIGN)) >> 15;

    p_cvdr_cfg->vp_rd_cfg[id].fmt.pix_fmt            = pix_fmt;
    p_cvdr_cfg->vp_rd_cfg[id].fmt.pix_expan          = cfmt->expand;
    p_cvdr_cfg->vp_rd_cfg[id].fmt.allocated_du       = allocated_du;
    p_cvdr_cfg->vp_rd_cfg[id].fmt.line_size          = cfmt->line_size - 1;
    p_cvdr_cfg->vp_rd_cfg[id].fmt.hblank             = 0;
    p_cvdr_cfg->vp_rd_cfg[id].fmt.frame_size         = cfmt->height - 1;
    p_cvdr_cfg->vp_rd_cfg[id].fmt.vblank             = 0;
    p_cvdr_cfg->vp_rd_cfg[id].fmt.line_stride        = ALIGN_UP(cfmt->full_width * pix_size /8, CVDR_ALIGN_BYTES) / CVDR_ALIGN_BYTES - 1;
    p_cvdr_cfg->vp_rd_cfg[id].fmt.line_wrap          = DEFAULT_LINE_WRAP;

   if (0 != line_stride)
    {
        p_cvdr_cfg->vp_rd_cfg[id].fmt.line_stride  = ALIGN_UP(line_stride * pix_size / 8, CVDR_ALIGN_BYTES) / CVDR_ALIGN_BYTES - 1;
    }

    calculate_cvdr_bw_limiter(&(p_cvdr_cfg->vp_rd_cfg[id].bw), cvdr_vp_rd_bw[id].throughput);
    pr_info("allocated_du: %d, pclk: %d\n",
       p_cvdr_cfg->vp_rd_cfg[id].fmt.allocated_du, cvdr_vp_rd_bw[id].pclk);
   
    D("id = %d, addr = 0x%x, width = %d, height = %d, config_width = %d,\n"
          "line_stride = %d, line_size = %d, frame_size=%d\n",
          cfmt->id, cfmt->addr, cfmt->width, cfmt->height, config_width,
          p_cvdr_cfg->vp_rd_cfg[id].fmt.line_stride, cfmt->line_size - 1,p_cvdr_cfg->vp_rd_cfg[id].fmt.frame_size);

    D("[%s] -\n", __func__);

    return CPE_FW_OK;
}

int dataflow_cvdr_cfg_d32(cfg_tab_cvdr_t* p_cvdr_cfg, cvdr_opt_fmt_t* cfmt,
    unsigned int total_bytes)
{
    D("[%s] +\n", __func__);
    if(NULL == p_cvdr_cfg || NULL == cfmt)
    {
        pr_err("input param is NULL!");
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

    D("fs_addr = 0x%x, id = %d, last_page = 0x%x, total_byte = %d",
          p_cvdr_cfg->vp_wr_cfg[id].fmt.fs_addr, id,
          p_cvdr_cfg->vp_wr_cfg[id].fmt.last_page,
          total_bytes);

    D("[%s] -\n", __func__);
    return 0;
}

int dataflow_cvdr_cfg_cmdlst(void)
{
    return CPE_FW_OK;
}

int dataflow_cvdr_calc_du_raw_readback(unsigned int id)
{
//    return calculate_cvdr_allocated_du(cvdr_vp_rd_bw_raw_readback[id].pclk, cvdr_vp_rd_bw_raw_readback[id].throughput, 0);
    return CPE_FW_OK;
}

#pragma GCC diagnostic pop
/****************************end************************************** */

