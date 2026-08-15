#ifndef SOC_JPGDEC_INTERFACE_H
#define SOC_JPGDEC_INTERFACE_H 
#define MAX_INPUT_WIDTH (8192)
#define MAX_INPUT_HEIGHT (8192)
#define MIN_INPUT_WIDTH (16)
#define MIN_INPUT_HEIGHT (16)
#define JPU_OUT_RGB_ADDR_ALIGN (8)
#define JPU_OUT_YUV_ADDR_ALIGN (4)
#define JPU_OUT_STRIDE_ALIGN (8)
#define JPU_MIN_STRIDE (1)
#define JPU_MAX_STRIDE (2048)
#define JPU_MAX_ADDR (0xfffffff)
#define JPU_LB_ADDR_ALIGN (128)
#define JPGDEC_CVDR_AXI_JPEG_CVDR_CFG (0x0)
#define JPGDEC_CVDR_AXI_JPEG_CVDR_WR_QOS_CFG (0xc)
#define JPGDEC_CVDR_AXI_JPEG_CVDR_RD_QOS_CFG (0x10)
#define JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_8 (0x15B0)
#define JPGDEC_CVDR_AXI_JPEG_LIMITER_NR_RD_8 (0x15B8)
#define JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_9 (0x15C0)
#define JPGDEC_CVDR_AXI_JPEG_LIMITER_NR_RD_9 (0x15C8)
#define JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_8 (0x13B0)
#define JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_9 (0x13C0)
#define JPGDEC_CVDR_AXI_JPEG_LIMITER_NR_WR_8 (0x13B8)
#define JPGDEC_CVDR_AXI_JPEG_LIMITER_NR_WR_9 (0x13C8)
#define JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_1 (0xA10)
#define JPGDEC_CVDR_AXI_JPEG_LIMITER_NR_RD_1 (0xA18)
#define JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_2 (0xA20)
#define JPGDEC_CVDR_AXI_JPEG_LIMITER_NR_RD_2 (0xA28)
#define JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_0 (0x900)
#define JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_1 (0x910)
#define JPGDEC_CVDR_AXI_JPEG_LIMITER_NR_WR_0 (0x908)
#define JPGDEC_CVDR_AXI_JPEG_LIMITER_NR_WR_1 (0x918)
#define JPGDEC_RO_STATE (0xC)
#define JPGDEC_CRG_CFG0 (0x20)
#define JPGDEC_CRG_CFG1 (0x24)
#define JPGDEC_MEM_CFG (0x28)
#define JPGDEC_IRQ_REG0 (0x2C)
#define JPGDEC_IRQ_REG1 (0x30)
#define JPGDEC_IRQ_REG2 (0x34)
#define JPGDEC_PREF_STOP (0x18C)
#define BIT_JPGDEC_INT_OVER_TIME BIT(3)
#define BIT_JPGDEC_INT_DEC_ERR BIT(2)
#define BIT_JPGDEC_INT_BS_RES BIT(1)
#define BIT_JPGDEC_INT_DEC_FINISH BIT(0)
#define JPEG_DEC_START (0x0)
#define JPEG_DEC_PREFTCH_CTRL (0x4)
#define JPEG_DEC_FRAME_SIZE (0x10)
#define JPEG_DEC_CROP_HORIZONTAL (0x14)
#define JPEG_DEC_CROP_VERTICAL (0x18)
#define JPEG_DEC_BITSTREAMS_START_H (0x20)
#define JPEG_DEC_BITSTREAMS_START_L (0x24)
#define JPEG_DEC_BITSTREAMS_END_H (0x28)
#define JPEG_DEC_BITSTREAMS_END_L (0x2C)
#define JPEG_DEC_FRAME_START_Y (0x30)
#define JPEG_DEC_FRAME_STRIDE_Y (0x34)
#define JPEG_DEC_FRAME_START_C (0x38)
#define JPEG_DEC_FRAME_STRIDE_C (0x3C)
#define JPEG_DEC_LBUF_START_ADDR (0x40)
#define JPEG_DEC_OUTPUT_TYPE (0x50)
#define JPEG_DEC_FREQ_SCALE (0x54)
#define JPEG_DEC_MIDDLE_FILTER (0x58)
#define JPEG_DEC_SAMPLING_FACTOR (0x5C)
#define JPEG_DEC_DRI (0x60)
#define JPEG_DEC_OVER_TIME_THD (0x70)
#define JPEG_DEC_HOR_PHASE0_COEF01 (0x80)
#define JPEG_DEC_HOR_PHASE0_COEF23 (0x84)
#define JPEG_DEC_HOR_PHASE0_COEF45 (0x88)
#define JPEG_DEC_HOR_PHASE0_COEF67 (0x8C)
#define JPEG_DEC_HOR_PHASE2_COEF01 (0x90)
#define JPEG_DEC_HOR_PHASE2_COEF23 (0x94)
#define JPEG_DEC_HOR_PHASE2_COEF45 (0x98)
#define JPEG_DEC_HOR_PHASE2_COEF67 (0x9C)
#define JPEG_DEC_VER_PHASE0_COEF01 (0xA0)
#define JPEG_DEC_VER_PHASE0_COEF23 (0xA4)
#define JPEG_DEC_VER_PHASE2_COEF01 (0xA8)
#define JPEG_DEC_VER_PHASE2_COEF23 (0xAC)
#define JPEG_DEC_CSC_IN_DC_COEF (0xB0)
#define JPEG_DEC_CSC_OUT_DC_COEF (0xB4)
#define JPEG_DEC_CSC_TRANS_COEF0 (0xB8)
#define JPEG_DEC_CSC_TRANS_COEF1 (0xBC)
#define JPEG_DEC_CSC_TRANS_COEF2 (0xC0)
#define JPEG_DEC_CSC_TRANS_COEF3 (0xC4)
#define JPEG_DEC_CSC_TRANS_COEF4 (0xC8)
#define JPGD_REG_QUANT (0x200)
#define JPGD_REG_HDCTABLE (0x300)
#define JPGD_REG_HACMINTABLE (0x340)
#define JPGD_REG_HACBASETABLE (0x380)
#define JPGD_REG_HACSYMTABLE (0x400)
#define JPGD_REG_DEBUG_BASE (0x800)
#define JPGD_REG_DEBUG_RANGE (16)
#define JPEG_DEC_BITSTREAMS_START (0x14)
#define JPEG_DEC_BITSTREAMS_END (0x18)
#define JPU_LB_SIZE (128 * SZ_1K)
#define JPU_SMMU_OFFSET (0x1F000)
#define SMMU_SCR (0x0000)
#define SMMU_MEMCTRL (0x0004)
#define SMMU_LP_CTRL (0x0008)
#define SMMU_PRESS_REMAP (0x000C)
#define SMMU_INTMASK_NS (0x0010)
#define SMMU_INTRAW_NS (0x0014)
#define SMMU_INTSTAT_NS (0x0018)
#define SMMU_INTCLR_NS (0x001C)
#define SMMU_SMRx_NS (0x0020)
#define SMMU_RLD_EN0_NS (0x01F0)
#define SMMU_RLD_EN1_NS (0x01F4)
#define SMMU_RLD_EN2_NS (0x01F8)
#define SMMU_CB_SCTRL (0x0200)
#define SMMU_CB_TTBR0 (0x0204)
#define SMMU_CB_TTBR1 (0x0208)
#define SMMU_CB_TTBCR (0x020C)
#define SMMU_OFFSET_ADDR_NS (0x0210)
#define SMMU_SCACHEI_ALL (0x0214)
#define SMMU_SCACHEI_L1 (0x0218)
#define SMMU_SCACHEI_L2L3 (0x021C)
#define SMMU_FAMA_CTRL0 (0x0220)
#define SMMU_FAMA_CTRL1 (0x0224)
#define SMMU_ADDR_MSB (0x0300)
#define SMMU_ERR_RDADDR (0x0304)
#define SMMU_ERR_WRADDR (0x0308)
#define SMMU_FAULT_ADDR_TCU (0x0310)
#define SMMU_FAULT_ID_TCU (0x0314)
#define SMMU_FAULT_ADDR_TBUx (0x0320)
#define SMMU_FAULT_ID_TBUx (0x0324)
#define SMMU_FAULT_INFOx (0x0328)
#define SMMU_DBGRPTR_TLB (0x0380)
#define SMMU_DBGRDATA_TLB (0x0380)
#define SMMU_DBGRDATA0_CACHE (0x038C)
#define SMMU_DBGRDATA1_CACHE (0x0390)
#define SMMU_DBGAXI_CTRL (0x0394)
#define SMMU_OVA_ADDR (0x0398)
#define SMMU_OPA_ADDR (0x039C)
#define SMMU_OVA_CTRL (0x03A0)
#define SMMU_OPREF_ADDR (0x03A4)
#define SMMU_OPREF_CTRL (0x03A8)
#define SMMU_OPREF_CNT (0x03AC)
#define SMMU_SMRx_S (0x0500)
#define SMMU_RLD_EN0_S (0x06F0)
#define SMMU_RLD_EN1_S (0x06F4)
#define SMMU_RLD_EN2_S (0x06F8)
#define SMMU_INTMAS_S (0x0700)
#define SMMU_INTRAW_S (0x0704)
#define SMMU_INTSTAT_S (0x0708)
#define SMMU_INTCLR_S (0x070C)
#define SMMU_SCR_S (0x0710)
#define SMMU_SCB_SCTRL (0x0714)
#define SMMU_SCB_TTBR (0x0718)
#define SMMU_SCB_TTBCR (0x071C)
#define SMMU_OFFSET_ADDR_S (0x0720)
#define SMMU_SMRX_P (0x10000)
#define SMMU_SCR_P (0x10210)
enum jpu_freq_scale {
 JPU_FREQ_SCALE_1 = 0,
 JPU_FREQ_SCALE_2 = 1,
 JPU_FREQ_SCALE_4 = 2,
 JPU_FREQ_SCALE_8 = 3,
};
typedef enum {
 JPU_OUTPUT_UNSUPPORT = -1,
 JPU_OUTPUT_YUV420 = 0,
 JPU_OUTPUT_YUV_ORIGINAL = 1,
 JPU_OUTPUT_RGBA4444 = 4,
 JPU_OUTPUT_BGRA4444 = 5,
 JPU_OUTPUT_RGB565 = 6,
 JPU_OUTPUT_BGR565 = 7,
 JPU_OUTPUT_RGBA8888 = 8,
 JPU_OUTPUT_BGRA8888 = 9,
} jpu_output_format;
typedef enum {
 HISI_KIRIN_970 = 1,
 HISI_DSS_V500,
 HISI_DSS_V501,
 HISI_DSS_V510,
 HISI_DSS_V510_CS,
 UNSUPPORT_PLATFORM,
}jpeg_dec_platform;
typedef struct jpu_dec_reg {
 uint32_t dec_start;
 uint32_t preftch_ctrl;
 uint32_t frame_size;
 uint32_t crop_horizontal;
 uint32_t crop_vertical;
 uint32_t bitstreams_start_h;
 uint32_t bitstreams_start;
 uint32_t bitstreams_end_h;
 uint32_t bitstreams_end;
 uint32_t frame_start_y;
 uint32_t frame_stride_y;
 uint32_t frame_start_c;
 uint32_t frame_stride_c;
 uint32_t lbuf_start_addr;
 uint32_t output_type;
 uint32_t freq_scale;
 uint32_t middle_filter;
 uint32_t sampling_factor;
 uint32_t dri;
 uint32_t over_time_thd;
 uint32_t hor_phase0_coef01;
 uint32_t hor_phase0_coef23;
 uint32_t hor_phase0_coef45;
 uint32_t hor_phase0_coef67;
 uint32_t hor_phase2_coef01;
 uint32_t hor_phase2_coef23;
 uint32_t hor_phase2_coef45;
 uint32_t hor_phase2_coef67;
 uint32_t ver_phase0_coef01;
 uint32_t ver_phase0_coef23;
 uint32_t ver_phase2_coef01;
 uint32_t ver_phase2_coef23;
 uint32_t csc_in_dc_coef;
 uint32_t csc_out_dc_coef;
 uint32_t csc_trans_coef0;
 uint32_t csc_trans_coef1;
 uint32_t csc_trans_coef2;
 uint32_t csc_trans_coef3;
 uint32_t csc_trans_coef4;
} jpu_dec_reg_t;
#endif
