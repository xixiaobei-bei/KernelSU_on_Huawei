/*
 * hjpeg_process.c
 *
 * implement for processing jpeg power on /off/encode.
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * lixiuhua <aimee.li@hisilicon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
#include <linux/platform_data/remoteproc-hisi.h>
#include <linux/wakelock.h>
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
/*lint -save -e715 -e747 -e529 -e438 -e845 -e846 -e514 -e866 -e30 -e84*/
/*TODO:use kernel dump mem space do JPEG IP system verify*/

//#define MST_DEBUG 1
//#include "hjpeg250/hjpg_softreset_test.h"
//#define SOFT_RESET 1 
//#define IRQ_TEST 0

#define I2hjpegenc(i) container_of(i, hjpeg_base_t, intf)

#define ENCODE_FINISH (1<<4)
#define IRQ_MERGE_ENCODE_FINISH (1<<1)

typedef struct _tag_hjpeg_base
{
    struct platform_device*                     pdev;
    hjpeg_intf_t                                intf;
    char const*                                 name;
    u32                                         irq_no;
    struct semaphore                            buff_done;
    hjpeg_hw_ctl_t                              hw_ctl;               // for all phyaddr and viraddr
    struct regulator *                          jpeg_supply;
    struct regulator *                          media_supply;
    struct clk *                                jpegclk[JPEG_CLK_MAX];
    unsigned int                                jpegclk_value[JPEG_CLK_MAX];
    unsigned int                                jpegclk_low_frequency;
    unsigned int                                power_off_frequency;
    atomic_t                                    jpeg_on;
    struct iommu_domain *                       domain;
    phy_pgd_t                                   phy_pgd;
    struct wake_lock                            power_wakelock;
    struct mutex                                wake_lock_mutex;
} hjpeg_base_t;

int is_hjpeg_qos_update(void);
int get_hjpeg_iova_update(void);
int get_hjpeg_wr_port_addr_update(void);
int is_pixel_fmt_update(void);
int is_cvdr_cfg_update(void);

static void hjpeg_isr_do_tasklet(unsigned long data);
DECLARE_TASKLET(hjpeg_isr_tasklet, hjpeg_isr_do_tasklet, (unsigned long)0);

static void hjpeg_unmap_baseaddr(void);
static int hjpeg_map_baseaddr(void);

static int hjpeg_get_dts(struct platform_device* pDev);
static int get_phy_pgd_base(void);

static int hjpeg_poweron(hjpeg_base_t* pJpegDev);
static int hjpeg_poweroff(hjpeg_base_t* pJpegDev);

static int hjpeg_setclk_enable(hjpeg_base_t* pJpegDev, int idx);
static void hjpeg_setclk_disable(hjpeg_base_t* pJpegDev, int idx);
static int hjpeg_clk_ctrl(void __iomem* subctrl1, bool enable);
static int hjpeg_encode_process(hjpeg_intf_t *i, void *cfg);
static int hjpeg_power_on(hjpeg_intf_t *i);
static int hjpeg_power_off(hjpeg_intf_t *i);
static int hjpeg_get_reg(hjpeg_intf_t *i, void* cfg);
static int hjpeg_set_reg(hjpeg_intf_t *i, void* cfg);

static void hjpeg_irq_clr(void __iomem* subctrl1);
static void hjpeg_irq_mask(void __iomem* subctrl1, bool enable);
static void hjpeg_encode_finish(void __iomem* jpegenc, void __iomem* subctl);

static hjpeg_vtbl_t
s_vtbl_hjpeg =
{
    .encode_process = hjpeg_encode_process,
    .power_on = hjpeg_power_on,
    .power_down = hjpeg_power_off,
    .set_reg = hjpeg_set_reg,
    .get_reg = hjpeg_get_reg,
};

static hjpeg_base_t
s_hjpeg =
{
    .intf = { .vtbl = &s_vtbl_hjpeg, },
    .name = "hjpeg",
};/*lint !e785*/

static const struct of_device_id
s_hjpeg_dt_match[] =
{
    {
        .compatible = "huawei,hjpeg",
        .data = &s_hjpeg.intf,
    },
    {
    },/*lint !e785*/
};

MODULE_DEVICE_TABLE(of, s_hjpeg_dt_match);

static struct timeval s_timeval_start;
static struct timeval s_timeval_end;
static int is_qos_update = 0;
static int iova_update_version = 0;
static int wr_port_addr_update_version = 0;
static int pixel_format_update = 0;
static int cvdr_cfg_update = 0;
static int is_irq_merge = 0;
static int clk_ctl_offset = 0;

extern int memset_s(void *dest, size_t destMax, int c, size_t count);

static void hjpeg_isr_do_tasklet(unsigned long data)
{
    up(&s_hjpeg.buff_done);
}

static irqreturn_t hjpeg_irq_handler(int irq, void *dev_id)
{
    hjpeg_encode_finish(s_hjpeg.hw_ctl.jpegenc_viraddr
            , s_hjpeg.hw_ctl.subctrl_viraddr);

    return IRQ_HANDLED;
}

static void calculate_encoding_time(void)
{
    u64 tm_used;

    tm_used = (s_timeval_end.tv_sec - s_timeval_start.tv_sec) * MICROSECOND_PER_SECOND
        + s_timeval_end.tv_usec - s_timeval_start.tv_usec;

    cam_debug("%s JPGENC encoding elapse %llu us",__func__, tm_used);
}

static bool bypass_smmu(void)
{
    return (s_hjpeg.hw_ctl.smmu_bypass == BYPASS_YES);
}

static void set_rstmarker(void __iomem*  base_addr, unsigned int rst)
{
    set_reg_val((void __iomem*)((char*)base_addr + JPGENC_JPE_RESTART_INTERVAL_REG), rst);
}

static int __check_buffer_vaild(int share_fd, unsigned int req_addr, unsigned int req_size)
{
    int ret = 0;
    struct iommu_format fmt = { 0 };

    if (share_fd < 0) {
        cam_err("invalid ion buffer: fd=%d", share_fd);
        return -1;
    }

    ret = hicam_buf_map_iommu(share_fd, &fmt);
    if (ret < 0) {
        cam_err("%s: fail to map iommu.", __func__);
        return ret;
    }

    // FIXME: change test to req_addr < fmt.iova || req_addr + req_size > fmt.iova + fmt.size ?
    if (req_addr != fmt.iova || req_size > fmt.size) {
        ret = -ERANGE;
    }
    hicam_buf_unmap_iommu(share_fd, &fmt);
    cam_debug("%s: fd:%d, iova:%#llx, size:%#llx", __func__, share_fd, fmt.iova, fmt.size);
    return ret;
}

static int check_buffer_vaild(jpgenc_config_t* config)
{
    unsigned int vaild_input_size;

    if (__check_buffer_vaild(config->buffer.ion_fd, config->buffer.output_buffer, config->buffer.output_size)) {
        cam_err("%s:check output buffer fail", __func__);
        return -1;
    }

    if (JPGENC_FORMAT_YUV422 == ((unsigned int)(config->buffer.format) & JPGENC_FORMAT_BIT)) {
        vaild_input_size = config->buffer.width * config->buffer.height * 2;
    }
    else {
        vaild_input_size = config->buffer.width * config->buffer.height * 3 / 2;
    }

    // uv addr has been checked
    if (__check_buffer_vaild(config->buffer.input_ion_fd, config->buffer.input_buffer_y, vaild_input_size)) {
        cam_err("%s:check output buffer fail", __func__);
        return -1;
    }

    return 0;
}

/* check the input parameter of hjpeg_encode_process from IOCTL
  if invalid, return error.
  called by hjpeg_encode_process
*/
static int check_config(jpgenc_config_t* config)
{
    cam_info("%s enter ",__func__);
    if (config == NULL){
        cam_err("%s: config is null! (%d)",__func__, __LINE__);
        return -EINVAL;
    }

    if (!CHECK_ALIGN(config->buffer.width, 2) || (config->buffer.width > 8192)){
        cam_err(" width[%d] is invalid! ",config->buffer.width);
        return -1;
    }

    if (0 == config->buffer.height || (config->buffer.height > 8192)){
        cam_err(" height[%d] is invalid! ",config->buffer.height);
        return -1;
    }

    if ((0 == config->buffer.stride)
            || !CHECK_ALIGN(config->buffer.stride,16)
            || (config->buffer.stride / 16 > ((JPGENC_FORMAT_YUV422 == ((unsigned int)(config->buffer.format) &
            JPGENC_FORMAT_BIT)) ? 1024 : 512)))
    {
        cam_err(" stride[%d] is invalid! ",config->buffer.stride);
        return -1;
    }

    if ((0 == config->buffer.input_buffer_y) || !CHECK_ALIGN(config->buffer.input_buffer_y, 16)){
        cam_err(" input buffer y[0x%x] is invalid! ",config->buffer.input_buffer_y);
        return -1;
    }
    if ((JPGENC_FORMAT_YUV420 == ((unsigned int)(config->buffer.format) & JPGENC_FORMAT_BIT))
            && ((0== config->buffer.input_buffer_uv ) || !CHECK_ALIGN(config->buffer.input_buffer_uv, 16))) {
        cam_err(" input buffer uv[0x%x] is invalid! ",config->buffer.input_buffer_uv);
        return -1;
    }

    if ((JPGENC_FORMAT_YUV420 == ((unsigned int)(config->buffer.format) & JPGENC_FORMAT_BIT)) &&
        (config->buffer.input_buffer_uv - config->buffer.input_buffer_y < config->buffer.stride * 8 * 16)) {
        cam_err(" buffer format is invalid! ");
        return -1;
    }

    if (config->buffer.quality > 100){
        cam_err(" quality[%d] is invalid, adjust to 100",config->buffer.quality);
        config->buffer.quality = 100;
    }

    if(config->buffer.output_size <= JPGENC_HEAD_SIZE ||
       config->buffer.output_size > MAX_JPEG_BUFFER_SIZE){
        cam_err(" output size[%u] is invalid!",config->buffer.output_size);
        return -1;
    }
    return check_buffer_vaild(config);
}
/* set picture format
  called by config_jpeg
*/
static void set_picture_format(void __iomem* base_addr, int fmt)
{
    unsigned int tmp = 0;
    if (JPGENC_FORMAT_YUV422 == (fmt & JPGENC_FORMAT_BIT)) {
        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_PIC_FORMAT_REG),JPGENC_ENC_PIC_FORMAT, 0);/*lint !e845*/
    } else {
        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_PIC_FORMAT_REG),JPGENC_ENC_PIC_FORMAT, 1);
    }

    if (JPGENC_FORMAT_VYUY == fmt || JPGENC_FORMAT_NV21 == fmt) {
        REG_SET_FIELD(tmp,JPGENC_SWAPIN_Y_UV, 0);
        REG_SET_FIELD(tmp,JPGENC_SWAPIN_U_V, 1);
    }
    else if (JPGENC_FORMAT_YVYU == fmt) {
        REG_SET_FIELD(tmp,JPGENC_SWAPIN_Y_UV, 1);
        REG_SET_FIELD(tmp,JPGENC_SWAPIN_U_V, 1);
    }
    else if (JPGENC_FORMAT_YUYV == fmt) {
        REG_SET_FIELD(tmp,JPGENC_SWAPIN_Y_UV, 1);
        REG_SET_FIELD(tmp,JPGENC_SWAPIN_U_V, 0);
    }
    else {     //default formt  JPGENC_FORMAT_UYVY|JPGENC_FORMAT_NV12
        REG_SET_FIELD(tmp,JPGENC_SWAPIN_Y_UV, 0);
        REG_SET_FIELD(tmp,JPGENC_SWAPIN_U_V, 0);
    }

    set_reg_val((void __iomem*)((char*)base_addr + JPGENC_INPUT_SWAP_REG), tmp);
}

static void set_jpe_table_data(void __iomem* base_addr, const uint8_t *array, uint32_t size, uint32_t scaler)
{
    uint32_t temp;
    uint32_t tmpreg;
    uint32_t i;
    for(i = 1; i < size; i = i + 2)
    {
        tmpreg = 0;
        temp = (array[i - 1] * scaler + 50U) / 100U;
        if (temp == 0U)
            temp =  1U;
        if (temp > 255U)
            temp = 255U;
        REG_SET_FIELD(tmpreg, JPGENC_TABLE_WDATA_H, temp);

        temp = (array[i] * scaler + 50U) / 100U;
        if (temp == 0U)
            temp =  1U;
        if (temp > 255U)
            temp = 255U;
        REG_SET_FIELD(tmpreg, JPGENC_TABLE_WDATA_L, temp);

        set_reg_val((void __iomem*)((char*)base_addr + JPGENC_JPE_TABLE_DATA_REG), tmpreg);
    }
}

/* set picture quality
  called by config_jpeg
*/
static void set_picture_quality(void __iomem* base_addr, unsigned int quality)
{
    unsigned int scaler;
    unsigned int length;

    if(quality == 0)
    {
        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_TQ_Y_SELECT_REG),JPGENC_TQ0_SELECT,0);
        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_TQ_U_SELECT_REG),JPGENC_TQ1_SELECT,1);
        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_TQ_V_SELECT_REG),JPGENC_TQ2_SELECT,1);
    }
    else
    {
        if(quality < 50)
            scaler = 5000/quality;
        else
            scaler = 200 - quality * 2;

        /*q-table 2*/
        length = ARRAY_SIZE(luma_qtable2);
        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_TABLE_ID_REG),JPGENC_TABLE_ID,2);
        set_jpe_table_data(base_addr, luma_qtable2, length, scaler);

        /*q-table 3*/
        length = ARRAY_SIZE(chroma_qtable2);
        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_TABLE_ID_REG),JPGENC_TABLE_ID,3);
        set_jpe_table_data(base_addr, chroma_qtable2, length, scaler);

        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_TQ_Y_SELECT_REG),JPGENC_TQ0_SELECT,2);
        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_TQ_U_SELECT_REG),JPGENC_TQ1_SELECT,3);
        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_TQ_V_SELECT_REG),JPGENC_TQ2_SELECT,3);
    }
}

static void set_picture_size(void __iomem*  base_addr, jpgenc_config_t* config)
{
    uint32_t width_left;
    uint32_t width_right = 0;

    width_left = config->buffer.width;
    if (s_hjpeg.hw_ctl.power_controller == PC_HISP)
    {
        if(width_left >= 64)
        {
            width_left = ALIGN_DOWN((width_left/2), 16);
        }
        width_right = config->buffer.width - width_left;
    }

    SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_ENC_HRIGHT1_REG), JPGENC_ENC_HRIGHT1, width_left -1);
    SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_ENC_VBOTTOM_REG), JPGENC_ENC_VBOTTOM, config->buffer.height - 1);
    SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_ENC_HRIGHT2_REG), JPGENC_ENC_HRIGHT2, width_right != 0 ? width_right -1 : 0);
}

int dump_jpeg_reg(void __iomem * viraddr)
{
    int i;
    if (NULL == viraddr) {
        cam_err("%s: viraddr is NULL", __func__);
        return -1;
    }
    for (i = 0 ; i <= JPGENC_FORCE_CLK_ON_CFG_REG;) {
        cam_debug("%s: jpeg reg 0x%x = 0x%x", __func__, i, get_reg_val((void __iomem*)((char*)viraddr + i)));
        i += 4;
    }
    for (i = JPGENC_DBG_0_REG ; i <= JPGENC_DBG_13_REG;) {
        cam_debug("%s: dbg reg 0x%x = 0x%x", __func__, i, get_reg_val((void __iomem*)((char*)viraddr + i)));
        i += 4;
    }
    return 0;
}

#ifdef SOFT_RESET
static int hjpeg_soft_reset(hjpeg_base_t *phjpeg)
{
    int i = 0;
    void __iomem *   jpeg_top_base = NULL;
    void __iomem *   cvdr_axi_base = NULL;
    void __iomem *   temp_base = NULL;
    u32     tempVal;

    jpeg_top_base = phjpeg->hw_ctl.subctrl_viraddr;
    cvdr_axi_base = phjpeg->hw_ctl.cvdr_viraddr;

    cam_info("%s, enter, subctrl=0x%x, cvdr=0x%x\n", __func__, jpeg_top_base, cvdr_axi_base);
#if 1
    for (i = 0; i < ARRAY_SIZE(sr_cfg_reg_sets); ++i)
    {
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

        switch (sr_cfg_reg_sets[i].op_flag) {
            case SR_OP_WRITE_ALL: {
                set_reg_val(temp_base+sr_cfg_reg_sets[i].offset, sr_cfg_reg_sets[i].value);
                cam_info("SR_OP_WRITE_ALL: write@0x%X@0x%X", sr_cfg_reg_sets[i].offset, sr_cfg_reg_sets[i].value);
                break;
            }
            case SR_OP_READ_ALL: {
                tempVal = get_reg_val(temp_base+sr_cfg_reg_sets[i].offset);
                cam_info("SR_OP_READ_ALL: read@0x%X@0x%X, expect@0x%X", sr_cfg_reg_sets[i].offset, sr_cfg_reg_sets[i].value, tempVal);
                break;
            }
            case SR_OP_WRITE_BIT: {
                tempVal = get_reg_val(temp_base+sr_cfg_reg_sets[i].offset);
                cam_info("SR_OP_WRITE_BIT: read@0x%X@0x%X", sr_cfg_reg_sets[i].offset, tempVal);
                if(sr_cfg_reg_sets[i].value == 1) {
                    tempVal |= sr_cfg_reg_sets[i].bit_flag;
                }
                else {
                    tempVal = ~tempVal;
                    tempVal |= sr_cfg_reg_sets[i].bit_flag;
                    tempVal = ~tempVal;
                }
                set_reg_val(temp_base+sr_cfg_reg_sets[i].offset, tempVal);
                cam_info("SR_OP_WRITE_BIT: write@0x%X@0x%X", sr_cfg_reg_sets[i].offset, tempVal);
                break;
            }
            case SR_OP_READ_BIT: {
                tempVal = get_reg_val(temp_base+sr_cfg_reg_sets[i].offset);
                cam_info("SR_OP_READ_BIT: read@0x%X = 0x%X", sr_cfg_reg_sets[i].offset, tempVal);
                cam_info("SR_OP_READ_BIT: value0x%X, expect@0x%X", (tempVal & sr_cfg_reg_sets[i].bit_flag), sr_cfg_reg_sets[i].value);
                break;
            }
            default:
                break;
        }
        temp_base = NULL;
        msleep(100);
    }
#endif
    return 0;
}
#endif

/*
  configure JPEGENC register
  called by hjpeg_encode_process
*/
static void hjpeg_config_jpeg(jpgenc_config_t* config)
{
    void __iomem*  base_addr = s_hjpeg.hw_ctl.jpegenc_viraddr;

    cam_info("%s enter ",__func__);
    if (NULL == config) {
        cam_err("%s: config is null! (%d)",__func__, __LINE__);
        return;
    }

    set_picture_format(base_addr, config->buffer.format);

    set_picture_size(base_addr, config);

    set_picture_quality(base_addr, config->buffer.quality);

    //set input buffer address
    if (CVDR_IOVA_ADDR_34BITS == get_hjpeg_iova_update()) {
        U_JPEGENC_ADDRESS addr_y;
        addr_y.bits.address = config->buffer.input_buffer_y >> 4;
        addr_y.bits.reserved = 0;
        set_reg_val((void __iomem*)((char*)base_addr + JPGENC_ADDRESS_Y_REG), addr_y.reg32);
        if (JPGENC_FORMAT_YUV420 == ((unsigned int)(config->buffer.format) & JPGENC_FORMAT_BIT)) {
            U_JPEGENC_ADDRESS addr_uv;
            addr_uv.bits.address = config->buffer.input_buffer_uv >> 4;
            addr_uv.bits.reserved = 0;
            set_reg_val((void __iomem*)((char*)base_addr + JPGENC_ADDRESS_UV_REG),addr_uv.reg32 );
        }
    } else {
        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_ADDRESS_Y_REG),JPGENC_ADDRESS_Y,config->buffer.input_buffer_y >> 4);
        if (JPGENC_FORMAT_YUV420 == ((unsigned int)(config->buffer.format) & JPGENC_FORMAT_BIT)) {
            SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_ADDRESS_UV_REG),JPGENC_ADDRESS_UV,config->buffer.input_buffer_uv >> 4);
        }
    }

    //set preread
    if (JPGENC_FORMAT_YUV420 == ((unsigned int)(config->buffer.format) & JPGENC_FORMAT_BIT)) {
        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_PREREAD_REG),JPGENC_PREREAD,4);
    }
    else {
        SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_PREREAD_REG),JPGENC_PREREAD,0);
    }

    SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_STRIDE_REG),JPGENC_STRIDE,config->buffer.stride >> 4);
    SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_ENCODE_REG),JPGENC_ENCODE,1);

    cam_info("%s activate JPGENC",__func__);
    do_gettimeofday(&s_timeval_start);
    SET_FIELD_TO_REG((void __iomem*)((char*)base_addr + JPGENC_JPE_INIT_REG),JPGENC_JP_INIT,1);
}

static void hjpeg_enable_autogating(void)
{
    void __iomem *base_addr = s_hjpeg.hw_ctl.jpegenc_viraddr;
    set_reg_val((void __iomem*)((char*)base_addr + JPGENC_FORCE_CLK_ON_CFG_REG), 0x0);
}

static void hjpeg_disable_autogating(void)
{
    void __iomem *base_addr = s_hjpeg.hw_ctl.jpegenc_viraddr;
    set_reg_val((void __iomem*)((char*)base_addr + JPGENC_FORCE_CLK_ON_CFG_REG), 0x1);
}


static void hjpeg_disabe_irq(void)
{
    if (is_irq_merge) {
        hjpeg_irq_mask(s_hjpeg.hw_ctl.subctrl_viraddr, false);
    } else {
        void __iomem *base_addr = s_hjpeg.hw_ctl.jpegenc_viraddr;
        set_reg_val((void __iomem*)((char*)base_addr + JPGENC_JPE_STATUS_IMR_REG), 0x00);
    }
}

static void hjpeg_enable_irq(void)
{
    if (is_irq_merge) {
        hjpeg_irq_mask(s_hjpeg.hw_ctl.subctrl_viraddr, true);
    } else {
        void __iomem *base_addr = s_hjpeg.hw_ctl.jpegenc_viraddr;
        set_reg_val((void __iomem*)((char*)base_addr + JPGENC_JPE_STATUS_IMR_REG), 0x30);
    }
}

// IOCTL HJPEG_ENCODE_PROCESS
static int hjpeg_encode_process(hjpeg_intf_t *i, void *cfg)
{
#ifdef SOFT_RESET
    static int nOp = 0;
#endif
    long   jiff;
    int    ret;
    u32 byte_cnt;
    jpgenc_config_t* pcfg = (jpgenc_config_t *)cfg;

    if (NULL == pcfg) {
        cam_err("%s: cfg is null! (%d)",__func__, __LINE__);
        return -EINVAL;
    }

    cam_info("width:%d, height:%d, stride:%d, format:%#x, quality:%d, rst:%d, ion_fd:%d",
            pcfg->buffer.width,
            pcfg->buffer.height,
            pcfg->buffer.stride,
            pcfg->buffer.format,
            pcfg->buffer.quality,
            pcfg->buffer.rst,
            pcfg->buffer.ion_fd);
    cam_debug("input_buffer_y:%#x, input_buffer_uv:%#x, output_buffer:%#x, output_size:%u",
            pcfg->buffer.input_buffer_y,
            pcfg->buffer.input_buffer_uv,
            pcfg->buffer.output_buffer,
            pcfg->buffer.output_size);

    if (bypass_smmu()) {
        if ((0 == s_hjpeg.phy_pgd.phy_pgd_base) && (PC_DRV == s_hjpeg.hw_ctl.power_controller)) {
            cam_err("%s(%d)phy_pgd_base is invalid, encode processing terminated.",__func__, __LINE__);
            return -EINVAL;
        }
    }

#ifndef MST_DEBUG
    ret = check_config(pcfg);
    if (0 != ret){
        cam_err("%s(%d)check_config failed, encode processing terminated.",__func__, __LINE__);
        return ret;
    }
#else
    ret = 0;
#endif

    set_rstmarker(s_hjpeg.hw_ctl.jpegenc_viraddr, pcfg->buffer.rst);

    hjpeg_disable_autogating();

    hjpeg_config_cvdr(&s_hjpeg.hw_ctl, pcfg);

    hjpeg_enable_irq();
    hjpeg_config_jpeg(pcfg);
#ifdef SOFT_RESET
    if (nOp == 0) {
        cam_info("%s: op=0, reset when encode!", __func__);
        hjpeg_soft_reset(&s_hjpeg);
        // init qtable\hufftable etc.
        hjpeg_init_hw_param(s_hjpeg.hw_ctl.jpegenc_viraddr, s_hjpeg.hw_ctl.power_controller, bypass_smmu());
        nOp = 1;
        hjpeg_disabe_irq();
        hjpeg_enable_autogating();

        jiff = (long)msecs_to_jiffies(10);
        down_timeout(&s_hjpeg.buff_done, jiff);

        return -1;
    }
#endif

    jiff = (long)msecs_to_jiffies(WAIT_ENCODE_TIMEOUT);
    if (down_timeout(&s_hjpeg.buff_done, jiff)) {
        cam_err("time out wait for jpeg encode");
        ret = -1;
        hjpeg_irq_clr(s_hjpeg.hw_ctl.subctrl_viraddr);
        #if defined( HISP120_CAMERA )
            hjpeg_120_dump_reg();
        #endif

    }

    do_gettimeofday(&s_timeval_end);

    calculate_encoding_time();
    // for debug
    dump_jpeg_reg(s_hjpeg.hw_ctl.jpegenc_viraddr);

    hjpeg_disabe_irq();
    hjpeg_enable_autogating();

#ifdef SOFT_RESET
    if (nOp==2) {
        cam_info("%s: op=2, reset when encode done!", __func__);
        nOp = 3;
        hjpeg_soft_reset(&s_hjpeg);
        // init qtable\hufftable etc.
        hjpeg_init_hw_param(s_hjpeg.hw_ctl.jpegenc_viraddr, s_hjpeg.hw_ctl.power_controller, bypass_smmu());
        hjpeg_disabe_irq();
        hjpeg_enable_autogating();
        return -1;
    }

    if (nOp==1) {
        nOp = 2;
        cam_info("%s: op=1, just set to 2!", __func__);
    }
#endif

    byte_cnt = get_reg_val((void __iomem*)((char*)s_hjpeg.hw_ctl.jpegenc_viraddr + JPGENC_JPG_BYTE_CNT_REG));

    if (0 == byte_cnt) {
        cam_err("%s encode fail", __func__);
        pcfg->jpegSize = 0;
        return -1;
    }

    pcfg->jpegSize = byte_cnt;
    cam_info("%s jpeg encode process success.size=%u", __func__, pcfg->jpegSize);
    return ret;
}

static int power_control(hjpeg_base_t *phjpeg, bool on)
{
    int ret;
    // different platform operates differently.
    switch (phjpeg->hw_ctl.power_controller) {
        case PC_DRV:
            if (on) {
                ret = hjpeg_poweron(phjpeg);   /* [false alarm]:Value Never Read*/
            }else {
                ret = hjpeg_poweroff(phjpeg);  /* [false alarm]:Value Never Read*/
            }
            break;
        case PC_HISP:
            if (on) {
                ret = hisp_jpeg_powerup();
            } else {
                ret = hisp_jpeg_powerdn();
            }
            break;
        default:
            cam_info("%s powerup/powerdown by self\n",__func__);
            ret = -EINVAL;
            break;
    }

    return ret;
}

// IOCTL HJPEG_POWERON
static int hjpeg_power_on(hjpeg_intf_t *i)
{
    int ret;
    hjpeg_base_t* phjpeg;

    phjpeg = I2hjpegenc(i);/*lint !e826*/

    cam_info("%s enter\n",__func__);

    ret = power_control(phjpeg, true);
    if (0 != ret) {
        cam_err("%s(%d) jpeg power up fail",__func__, __LINE__);
        return ret;
    }

    // config smmu
    ret = hjpeg_smmu_config(&phjpeg->hw_ctl, &(phjpeg->phy_pgd));
    if (0 != ret) {
        cam_err("%s(%d) failed to config smmu, prepare to power down!",__func__, __LINE__);
        goto POWERUP_ERROR;
    }

    // set jpeg clock ctrl
    ret = hjpeg_clk_ctrl(phjpeg->hw_ctl.subctrl_viraddr, true);
    if (0 != ret) {
        cam_err("%s(%d) failed to enable jpeg clock , prepare to power down!",__func__, __LINE__);
        goto POWERUP_ERROR;
    }

    // init qtable\hufftable etc.
    hjpeg_init_hw_param(phjpeg->hw_ctl.jpegenc_viraddr, phjpeg->hw_ctl.power_controller, bypass_smmu());

    sema_init(&(phjpeg->buff_done), 0);

    if (phjpeg->irq_no)
    {
        /*request irq*/
        ret = request_irq(phjpeg->irq_no, hjpeg_irq_handler, 0, "hjpeg_irq", 0);
        if (ret != 0) {
            cam_err("fail to request irq [%d], error: %d", phjpeg->irq_no, ret);
            goto POWERUP_ERROR;
        }
    }
    cam_info("%s jpeg power on success",__func__);

    return ret;

POWERUP_ERROR:

    if ( 0 != power_control(phjpeg, false) )
        cam_err("%s(%d) jpeg power down fail",__func__, __LINE__);

    return ret;
}

// IOCTL HJPEG_POWERDOWN
static int hjpeg_power_off(hjpeg_intf_t *i)
{
    int ret;
    hjpeg_base_t* phjpeg;

    phjpeg = I2hjpegenc(i);/*lint !e826*/

    cam_info("%s enter\n",__func__);

    if (phjpeg->irq_no) {
        free_irq(phjpeg->irq_no, 0);
    }

    ret = power_control(phjpeg, false);

    if (ret != 0){
        cam_err("%s jpeg power down fail",__func__);
    }

    if (0 == ret) {
        cam_info("%s jpeg power off success",__func__);
    }


    return ret;
}

/*lint -save -e454 */
static int hjpeg_poweron(hjpeg_base_t* pJpegDev)
{
    int ret;
    int ret2;

    cam_info("%s enter\n",__func__);

    if (atomic_read(&pJpegDev->jpeg_on)) {
        atomic_inc(&pJpegDev->jpeg_on);
        cam_info("%s: jpeg power up, ref=%d\n", __func__, atomic_read(&pJpegDev->jpeg_on));
        return 0;
    }

    mutex_lock(&pJpegDev->wake_lock_mutex);
    if (!wake_lock_active(&pJpegDev->power_wakelock)) {
        wake_lock(&pJpegDev->power_wakelock);
        cam_info("%s jpeg power on enter, wake lock\n", __func__);
    }
    mutex_unlock(&pJpegDev->wake_lock_mutex); /*lint !e456 */

#if (POWER_CTRL_INTERFACE==POWER_CTRL_CFG_REGS)
    //power up with config regs
    if ((ret = cfg_powerup_regs())!=0) {
        cam_err("Failed: cfg_powerup_regs.%d\n", ret);
        goto FAILED_JPG_POWERON;
    }

    if ((ret = hjpeg_setclk_enable(pJpegDev, JPEG_FUNC_CLK)) != 0) {
        cam_err("Failed: hjpeg_setclk_enable.%d\n", ret);
        cfg_powerdn_regs();
        goto FAILED_JPG_POWERON;
    }
#else
    //power up with hardware interface
    if ((ret = regulator_enable(pJpegDev->media_supply)) != 0) {
        cam_err("Failed: media regulator_enable.%d\n", ret);
        goto FAILED_JPG_POWERON;
    }
    if ((ret = hjpeg_setclk_enable(pJpegDev, JPEG_FUNC_CLK)) != 0) {
        cam_err("Failed: hjpeg_setclk_enable.%d\n", ret);
        if ((ret2 = regulator_disable(pJpegDev->media_supply)) != 0) {
            cam_err("Failed: media regulator_enable.%d\n", ret2);
        }
        goto FAILED_JPG_POWERON;
    }
    if ((ret = regulator_enable(pJpegDev->jpeg_supply)) != 0) {
        cam_err("Failed: jpeg regulator_enable.%d\n", ret);
        hjpeg_setclk_disable(pJpegDev, JPEG_FUNC_CLK);
        if ((ret2 = regulator_disable(pJpegDev->media_supply)) != 0){
            cam_err("Failed: media regulator_disable.%d\n", ret2);
        }
        goto FAILED_JPG_POWERON;
    }
#endif

    atomic_inc(&pJpegDev->jpeg_on);
    cam_info("%s: jpeg first power up, ref=%d\n", __func__, atomic_read(&pJpegDev->jpeg_on));

    return ret;
FAILED_JPG_POWERON:
    mutex_lock(&pJpegDev->wake_lock_mutex);
    if (wake_lock_active(&pJpegDev->power_wakelock)) {
        wake_unlock(&pJpegDev->power_wakelock);
        cam_err("%s jpeg power on failed, wake unlock\n", __func__);
    }
    mutex_unlock(&pJpegDev->wake_lock_mutex); /*lint !e456 */

    return ret;
}

static int hjpeg_poweroff(hjpeg_base_t* pJpegDev)
{
    int ret;
    if (!atomic_read(&pJpegDev->jpeg_on)) {
        cam_info("%s: jpeg never power on, ref=%d\n", __func__, atomic_read(&pJpegDev->jpeg_on));
        return 0;
    }

    // set jpeg clock ctrl
    ret = hjpeg_clk_ctrl(pJpegDev->hw_ctl.subctrl_viraddr, false);
    if (0 != ret) {
        cam_err("%s(%d) failed to disable jpeg clock , prepare to power down!",__func__, __LINE__);
    }

    atomic_dec(&pJpegDev->jpeg_on);
    if (atomic_read(&pJpegDev->jpeg_on)) {
        cam_info("%s: jpeg power off, ref=%d\n", __func__, atomic_read(&pJpegDev->jpeg_on));
        return 0;
    }

#if (POWER_CTRL_INTERFACE==POWER_CTRL_CFG_REGS)
    //power down with config regs
    hjpeg_setclk_disable(pJpegDev, JPEG_FUNC_CLK);
    if ((ret = cfg_powerdn_regs())!=0) {
        cam_err("Failed: cfg_powerdn_regs.%d\n", ret);
    }
#else
    //power down with hardware interface
    if ((ret = regulator_disable(pJpegDev->jpeg_supply)) != 0) {
        cam_err("Failed: jpeg regulator_disable.%d\n", ret);
    }

    hjpeg_setclk_disable(pJpegDev, JPEG_FUNC_CLK);

    if ((ret = regulator_disable(pJpegDev->media_supply)) != 0){
        cam_err("Failed: media regulator_disable.%d\n", ret);
    }
#endif

    cam_info("%s: jpeg power down, ref=%d\n", __func__, atomic_read(&pJpegDev->jpeg_on));

    mutex_lock(&pJpegDev->wake_lock_mutex);
    if (wake_lock_active(&pJpegDev->power_wakelock)) {
        wake_unlock(&pJpegDev->power_wakelock);
        cam_info("%s jpeg power off exit, wake unlock\n", __func__);
    }
    mutex_unlock(&pJpegDev->wake_lock_mutex); /*lint !e456 */

    return ret;
}
/*lint -restore */

static int hjpeg_set_reg(hjpeg_intf_t* i, void* cfg)
{
    int ret = 0;

#ifdef MST_DEBUG
    void __iomem* base_addr = 0;
    jpgenc_config_t* pcfg;
    uint32_t addr;
    uint32_t value;

    cam_info("%s enter\n",__func__);


    pcfg = (jpgenc_config_t *)cfg;

    if (NULL == pcfg) {
        cam_err("%s: pcfg is null! (%d)",__func__, __LINE__);
        return -EINVAL;
    }

    addr = pcfg->reg.addr;
    value = pcfg->reg.value;

    if((addr < JPGENC_JPE_ENCODE_REG)
            ||(addr > JPGENC_FORCE_CLK_ON_CFG_REG )){
        cam_err("%s input addr is invaild 0x%x\n",__func__, addr);
        ret = -1;
        return ret;
    }

    base_addr = s_hjpeg.hw_ctl.jpegenc_viraddr;
    set_reg_val((void __iomem*)((char*)base_addr + addr), value);

    cam_debug("%s input addr is  0x%x input value is %d\n",__func__, addr, value);
#endif

    return ret;
}

static int hjpeg_get_reg(hjpeg_intf_t *i, void* cfg)
{
    int ret = 0;

#ifdef MST_DEBUG
    void __iomem* base_addr = 0;
    jpgenc_config_t* pcfg;
    uint32_t addr;

    cam_info("%s enter\n",__func__);

    pcfg = (jpgenc_config_t *)cfg;

    if (NULL == pcfg) {
        cam_err("%s: pcfg is null! (%d)",__func__, __LINE__);
        return -EINVAL;
    }

    addr = pcfg->reg.addr;

    if((addr < JPGENC_JPE_ENCODE_REG)
            ||(addr > JPGENC_FORCE_CLK_ON_CFG_REG )){
        cam_err("%s input addr is invaild 0x%x\n",__func__, addr);
        ret = -1;
        return ret;
    }

    base_addr = s_hjpeg.hw_ctl.jpegenc_viraddr;

    pcfg->reg.value = get_reg_val((void __iomem*)((char*)base_addr + addr));

    cam_info("%s input addr is  0x%x input value is %d\n",__func__, addr, pcfg->reg.value);
#endif

    return ret;
}

// unmap all viraddr
static void hjpeg_unmap_baseaddr(void)
{
    hjpeg_base_t * phjpeg = &s_hjpeg;

    if (IS_ERR_OR_NULL(phjpeg->hw_ctl.jpegenc_viraddr)){
        iounmap((void*)phjpeg->hw_ctl.jpegenc_viraddr);
        phjpeg->hw_ctl.jpegenc_viraddr = NULL;
    }

    if (IS_ERR_OR_NULL(phjpeg->hw_ctl.subctrl_viraddr)){
        iounmap((void*)phjpeg->hw_ctl.subctrl_viraddr);
        phjpeg->hw_ctl.subctrl_viraddr = NULL;
    }
    if (IS_ERR_OR_NULL(phjpeg->hw_ctl.smmu_viraddr)){
        iounmap((void*)phjpeg->hw_ctl.smmu_viraddr);
        phjpeg->hw_ctl.smmu_viraddr = NULL;
    }

    if (IS_ERR_OR_NULL(phjpeg->hw_ctl.cvdr_viraddr)) {
        iounmap((void *)phjpeg->hw_ctl.cvdr_viraddr);
        phjpeg->hw_ctl.cvdr_viraddr = NULL;
    }

    if (IS_ERR_OR_NULL(phjpeg->hw_ctl.top_viraddr)) {
        iounmap((void *)phjpeg->hw_ctl.top_viraddr);
        phjpeg->hw_ctl.top_viraddr = NULL;
    }

    phjpeg->irq_no = 0;

    // debug
#if (POWER_CTRL_INTERFACE==POWER_CTRL_CFG_REGS)
    cfg_unmap_reg_base();
#endif//(POWER_CTRL_INTERFACE==POWER_CTRL_CFG_REGS)
}

// init resource for remap base addr and requst irq
static int hjpeg_map_baseaddr(void)
{
    struct device_node *np;
    uint32_t base_array[2] = {0};
    size_t count ;
    int ret;
    hjpeg_base_t* phjpeg = &s_hjpeg;

    np = phjpeg->pdev->dev.of_node;
    if (NULL == np) {
        cam_err("%s: of node NULL.", __func__);
        return -ENXIO;
    }

    /* property(huawei,hjpeg-base) = <address, size>, so count is 2 */
    count = 2;
    ret = of_property_read_u32_array(np, "huawei,hjpeg-base", base_array, count);
    if (ret < 0) {
        cam_err("%s failed get jpeg base %d", __func__, __LINE__);
        return -ENXIO;
    }

    phjpeg->hw_ctl.jpegenc_phyaddr  = base_array[0];
    phjpeg->hw_ctl.jpegenc_mem_size = base_array[1];

    phjpeg->hw_ctl.jpegenc_viraddr = ioremap(phjpeg->hw_ctl.jpegenc_phyaddr, phjpeg->hw_ctl.jpegenc_mem_size);
    if (IS_ERR_OR_NULL(phjpeg->hw_ctl.jpegenc_viraddr)) {
        cam_err("%s jpegenc phyaddr ioremap fail", __func__);
        return -ENXIO;
    }

    cam_debug("%s hjpeg base address = 0x%x. hjpeg-base size = 0x%x.",
            __func__,base_array[0], base_array[1]);

    /* property(huawei,smmu-base) = <address, size>, so count is 2 */
    ret = of_property_read_u32_array(np, "huawei,smmu-base", base_array, count);
    if (ret < 0) {
        cam_err("%s failed get smmu base %d", __func__, __LINE__);
        ret = -ENXIO;
        goto fail;
    }


    phjpeg->hw_ctl.smmu_phyaddr  = base_array[0];
    phjpeg->hw_ctl.smmu_mem_size = base_array[1];
    phjpeg->hw_ctl.smmu_viraddr  = ioremap(base_array[0], base_array[1]);
    if (IS_ERR_OR_NULL(phjpeg->hw_ctl.smmu_viraddr)) {
        cam_err("%s smmu phyaddr ioremap fail", __func__);
        ret = -ENXIO;
        goto fail;
    }

    cam_debug("%s smmu base address = 0x%x. smmu-base size = 0x%x.",
            __func__,base_array[0], base_array[1]);

    /* property(huawei,cvdr-base) = <address, size>, so count is 2 */
    ret = of_property_read_u32_array(np, "huawei,cvdr-base", base_array, count);
    if (ret < 0) {
        cam_err("%s failed get cvdr base %d", __func__, __LINE__);
        ret = -ENXIO;
        goto fail;
    }

    phjpeg->hw_ctl.cvdr_phyaddr  = base_array[0];
    phjpeg->hw_ctl.cvdr_mem_size = base_array[1];
    phjpeg->hw_ctl.cvdr_viraddr  = ioremap(base_array[0], base_array[1]);
    if (IS_ERR_OR_NULL(phjpeg->hw_ctl.cvdr_viraddr)) {
        cam_err("%s cvdr phyaddr ioremap fail", __func__);
        ret = -ENXIO;
        goto fail;
    }

    cam_debug("%s cvdr base address = 0x%x. cvdr-base size = 0x%x.",
            __func__,base_array[0], base_array[1]);

    /* property(huawei,subctrl-base) = <address, size>, so count is 2 */
    ret = of_property_read_u32_array(np, "huawei,subctrl-base", base_array, count);
    if (ret < 0) {
        cam_err("%s failed get subctrl base %d", __func__, __LINE__);
        ret = -ENXIO;
        goto fail;
    }
    phjpeg->hw_ctl.subctrl_phyaddr  = base_array[0];
    phjpeg->hw_ctl.subctrl_mem_size = base_array[1];
    phjpeg->hw_ctl.subctrl_viraddr  = ioremap(base_array[0], base_array[1]);
    if (IS_ERR_OR_NULL(phjpeg->hw_ctl.subctrl_viraddr)) {
        cam_err("%s subctrl phyaddr ioremap fail", __func__);
        ret = -ENXIO;
        goto fail;
    }


    cam_debug("%s subctrl base address = 0x%x. subctrl-base size = 0x%x.",
            __func__,base_array[0], base_array[1]);

    // interrupt from dts
    phjpeg->irq_no = irq_of_parse_and_map(np, 0);
    if (phjpeg->irq_no == 0) {
        cam_err("%s failed get irq num (%d)\n", __func__, __LINE__);
        goto fail;
    }
    cam_info("%s irq [%d].", __func__, phjpeg->irq_no);

    // debug
#if (POWER_CTRL_INTERFACE==POWER_CTRL_CFG_REGS)
    if ((ret = cfg_map_reg_base())!=0) {
        cam_err("%s: cfg map failed.",__func__);
        ret = -ENXIO;
        goto fail;
    }
#endif

    return 0;

fail:
    hjpeg_unmap_baseaddr();

    return ret;
}
// get power controller
static int get_dts_power_prop(struct device *pdev)
{
    int i;
    struct device_node *np;
    const char* clk_name[JPEG_CLK_MAX] = {""};
    u32 tmp = 0;
    int ret;

    if (NULL == pdev) {
        cam_err("%s: pdev NULL.", __func__);
        return -1;
    }

    np = pdev->of_node;
    if (NULL == np) {
        cam_err("%s: of node NULL.", __func__);
        return -1;
    }

    ret = of_property_read_u32(np, "huawei,power_control", &tmp);
    if (ret < 0) {
        cam_err("%s: getpower_control flag failed.", __func__);
        goto error;
    }
    s_hjpeg.hw_ctl.power_controller = tmp;
    cam_info("%s: power_controller=%d", __func__, tmp);

    if (PC_DRV == s_hjpeg.hw_ctl.power_controller) {
        cam_info("[%s] drv regulator.\n", __func__);
        //get supply for jpeg
        s_hjpeg.jpeg_supply = devm_regulator_get(pdev, "hjpeg-srt");
        if (IS_ERR(s_hjpeg.jpeg_supply)) {
            cam_err("[%s] Failed : ISPSRT devm_regulator_get.\n", __func__);
            goto error;
        }

        //get supply for media
        s_hjpeg.media_supply = devm_regulator_get(pdev, "hjpeg-media");
        if (IS_ERR(s_hjpeg.media_supply)) {
            cam_err("[%s] Failed : media devm_regulator_get.\n", __func__);
            goto error;
        }

        if ((ret = of_property_read_u32(np, "clock-low-frequency", &(s_hjpeg.jpegclk_low_frequency))) < 0) {
            cam_err("[%s] Failed: of_property_read_u32.%d\n", __func__, ret);
            goto error;
        }

        if ((ret = of_property_read_u32(np, "power-off-frequency", &(s_hjpeg.power_off_frequency))) < 0) {
            cam_err("[%s] Failed: of_property_read_u32.%d\n", __func__, ret);
            goto error;
        }

        //get clk parameters
        if ((ret = of_property_read_string_array(np, "clock-names", clk_name, JPEG_CLK_MAX)) < 0) {
            cam_err("[%s] Failed : of_property_read_string_array.%d\n", __func__, ret);
            goto error;
        }

        for (i = 0; i < JPEG_CLK_MAX; ++i) {
            cam_debug("[%s] clk_name[%d] = %s\n", __func__, i, clk_name[i]);
        }

        if ((ret = of_property_read_u32_array(np, "clock-value", (unsigned int*)(&(s_hjpeg.jpegclk_value[0])), JPEG_CLK_MAX)) < 0) {
            cam_err("[%s] Failed: of_property_read_u32_array.%d\n", __func__, ret);
            goto error;
        }
        for (i = 0; i < JPEG_CLK_MAX; i++) {
            s_hjpeg.jpegclk[i] = devm_clk_get(pdev, clk_name[i]);
            if (IS_ERR_OR_NULL(s_hjpeg.jpegclk[i])) {
                cam_err("[%s] Failed : jpgclk.%s.%d.%li\n", __func__, clk_name[i], i, PTR_ERR(s_hjpeg.jpegclk[i]));
                goto error;
            }
            cam_debug("[%s] Jpeg clock.%d.%s: %d Hz\n", __func__, i, clk_name[i], s_hjpeg.jpegclk_value[i]);
        }
    }

    return 0;
error:
    return -1;

}

static int get_dts_smmu_prop(struct device *pdev)
{
    struct device_node *np;
    u32 tmp;
    int ret;

    if (NULL == pdev) {
        cam_err("%s: pdev NULL.", __func__);
        return -1;
    }

    np = pdev->of_node;
    if (NULL == np) {
        cam_err("%s: of node NULL.", __func__);
        return -1;
    }

    // get smmu_bypass
    tmp = 0;
    ret = of_property_read_u32(np, "huawei,smmu_bypass", &tmp);
    if (ret < 0) {
        cam_err("%s: get smmu_bypass flag failed.", __func__);
        goto error;
    }
    s_hjpeg.hw_ctl.smmu_bypass = tmp;
    cam_info("%s: smmu_bypass=%d", __func__, tmp);

    // get smmu_type
    tmp = 0;
    ret = of_property_read_u32(np, "huawei,smmu_type", &tmp);
    if (ret < 0) {
        cam_err("%s: get smmu_type flag failed.", __func__);
        goto error;
    }
    s_hjpeg.hw_ctl.smmu_type = tmp;
    cam_info("%s: smmu_type=%d", __func__, tmp);
    return 0;

error:
    return -1;
}

static int get_dts_cvdr_prop(struct device *pdev)
{
    struct device_node *np;
    int ret;
    uint32_t base_array[3] = {0};
    size_t count = 3;
    if (NULL == pdev) {
        cam_err("%s: pdev NULL.", __func__);
        return -1;
    }

    np = pdev->of_node;
    if (NULL == np) {
        cam_err("%s: of node NULL.", __func__);
        return -1;
    }

    /* property(huawei,cvdr) = <type, rd-port wr-port>, so count is 3 */
    ret = of_property_read_u32_array(np, "huawei,cvdr", base_array, count);
    if (ret < 0) {
        cam_err("%s failed get cvdr property %d", __func__, __LINE__);
        return -1;
    }

    s_hjpeg.hw_ctl.cvdr_prop.type    = base_array[0];
    s_hjpeg.hw_ctl.cvdr_prop.rd_port = base_array[1];
    s_hjpeg.hw_ctl.cvdr_prop.wr_port = base_array[2];
    cam_info("%s: cvdr prop=<%d,%d,%d>", __func__, base_array[0], base_array[1], base_array[2]);

    /* property(huawei,cvdr) = <type, rd-port wr-port>, so count is 3 */
    ret = of_property_read_u32_array(np, "huawei,stream_id", base_array, count);
    if (ret < 0) {
        cam_err("%s failed get stream_id property %d", __func__, __LINE__);
        return -1;
    }

    s_hjpeg.hw_ctl.stream_id[0]    = base_array[0];
    s_hjpeg.hw_ctl.stream_id[1]    = base_array[1];
    s_hjpeg.hw_ctl.stream_id[2]    = base_array[2];
    cam_info("%s: stream id=<%d,%d,%d>", __func__, base_array[0], base_array[1], base_array[2]);

    //is_qos_update
    ret = of_property_read_u32(np, "huawei,qos_update", &is_qos_update);
    if (ret < 0) {
        cam_info("%s: get qos_update flag failed.", __func__);
    }

    //iova_update_version
    ret = of_property_read_u32(np, "huawei,iova_update", &iova_update_version);
    if (ret < 0) {
        cam_info("%s: get iova_update flag failed.", __func__);
    }
    cam_info("%s: iova_addr_update=<%d>", __func__, iova_update_version);

    //is_wr_port_addr_update
    ret = of_property_read_u32(np, "huawei,wr_port_addr_update", &wr_port_addr_update_version);
    if (ret < 0) {
        cam_info("%s: get wr_port_addr_update flag failed.", __func__);
    }
    cam_info("%s: wr_port_addr_update=<%d>", __func__, wr_port_addr_update_version);
    //is_pixel_fmt_update
    ret = of_property_read_u32(np, "huawei,pix_format_update", &pixel_format_update);
    if (ret < 0) {
        cam_info("%s: get pixel_format_update flag failed.", __func__);
    }
    cam_info("%s: pixel_format_update=<%d>", __func__, pixel_format_update);
    //is_cvdr_cfg_update
    ret = of_property_read_u32(np, "huawei,cvdr_cfg_update", &cvdr_cfg_update);
    if (ret < 0) {
        cam_info("%s: get cvdr_cfg_update flag failed.", __func__);
    }
    cam_info("%s: cvdr_cfg_update=<%d>", __func__, cvdr_cfg_update);

    s_hjpeg.hw_ctl.cvdr_prop.flag    = 0;
    ret = of_property_read_u32_array(np, "huawei,cvdr_limiter_du", base_array, count);
    if (ret >= 0) {
        s_hjpeg.hw_ctl.cvdr_prop.flag    = 1;
        s_hjpeg.hw_ctl.cvdr_prop.rd_limiter    = base_array[0];
        s_hjpeg.hw_ctl.cvdr_prop.wr_limiter    = base_array[1];
        s_hjpeg.hw_ctl.cvdr_prop.allocated_du  = base_array[2];
        cam_info("%s: limiter_du=<%d,%d,%d>", __func__, base_array[0], base_array[1], base_array[2]);
    }

    return 0;
}

static int hjpeg_get_dts(struct platform_device* pDev)
{
    int ret;
    struct device *pdev;
    struct device_node *np;
    u32 chip_type;

    if (NULL == pDev) {
        cam_err("%s: pDev NULL.", __func__);
        return -1;
    }

    pdev = &(pDev->dev);

    np = pdev->of_node;

    if (NULL == np) {
        cam_err("%s: of node NULL.", __func__);
        return -1;
    }

    chip_type = CT_CS;
    // get chip type
    ret = of_property_read_u32(np, "huawei,chip_type", &chip_type);
    if (ret < 0) {
        cam_err("%s: get chip_type flag failed.", __func__);
    }
    s_hjpeg.hw_ctl.chip_type = chip_type;
    cam_info("%s: chip_type=%d", __func__, chip_type);

    // clk_ctl_offset
    ret = of_property_read_u32(np, "huawei,clk_ctl_offset", &clk_ctl_offset);
    if (ret == 0) {
        cam_info("%s: update clk_ctl_offset=0x%x.", __func__, clk_ctl_offset);
    }

    //hjpeg_irq_merge
    ret = of_property_read_u32(np, "huawei,irq-merge", &is_irq_merge);
    if (ret == 0) {
        cam_info("%s: update irq_merge=%u.", __func__, is_irq_merge);
    }

    ret = get_dts_cvdr_prop(pdev);
    if (ret < 0) {
        cam_err("%s: get cvdr property failed.", __func__);
        return -1;
    }
    ret = get_dts_smmu_prop(pdev);
    if (ret < 0) {
        cam_err("%s: get smmu property  failed.", __func__);
        return -1;
    }
    // get power controller
    ret = get_dts_power_prop(pdev);
    if (ret < 0) {
        cam_err("%s: get power property  failed.", __func__);
        return -1;
    }

    ret = get_phy_pgd_base();
    if (ret < 0) {
        cam_err("%s: get phy pgd base failed.", __func__);
        return -1;
    }

    return 0;
}

static int get_phy_pgd_base(void)
{
    phys_addr_t pgd_base = hicam_buf_get_pgd_base();
    if (!pgd_base) {
        return -1;
    }

    // TODO: need promote \<phy_pgd_base\> to 64 bit?
    s_hjpeg.phy_pgd.phy_pgd_base = (uint32_t)pgd_base;
    // ptw_msb = phy_pgd_base[38:32]
    s_hjpeg.phy_pgd.phy_pgd_fama_ptw_msb = ((uint32_t)(pgd_base >> 32)) & 0x0000007F;
    // bps_msb_ns = phy_pgd_base[38:33]
    s_hjpeg.phy_pgd.phy_pgd_fama_bps_msb_ns = ((uint32_t)(pgd_base >> 32)) & 0x0000007E;
    return 0;
}

static int hjpeg_setclk_enable(hjpeg_base_t* pJpegDev, int idx)
{
    int ret;

    cam_debug("%s enter (idx=%d) \n",__func__, idx);

#if defined( CONFIG_ES_LOW_FREQ  )
    if ((ret = jpeg_enc_set_rate(pJpegDev->jpegclk[idx], 415000000)) != 0) {
        cam_err("[%s] Failed: jpeg_enc_set_rate(%d - %d).%d\n",
                __func__, idx, pJpegDev->jpegclk_value[idx], ret);
        // try to set low freq
        if (0 != jpeg_enc_set_rate(pJpegDev->jpegclk[idx], pJpegDev->jpegclk_low_frequency)) {
            cam_err("[%s] Failed to set low frequency 1: jpeg_enc_set_rate(%d - %d).\n",
                __func__, idx, pJpegDev->jpegclk_low_frequency);
        }
    }
#else
    if ((ret = jpeg_enc_set_rate(pJpegDev->jpegclk[idx], pJpegDev->jpegclk_value[idx])) != 0) {
        cam_err("[%s] Failed: jpeg_enc_set_rate(%d - %d).%d\n",
                __func__, idx, pJpegDev->jpegclk_value[idx], ret);
        // try to set low freq
        if (0 != jpeg_enc_set_rate(pJpegDev->jpegclk[idx], pJpegDev->jpegclk_low_frequency)) {
            cam_err("[%s] Failed to set low frequency 1: jpeg_enc_set_rate(%d - %d).\n",
                __func__, idx, pJpegDev->jpegclk_low_frequency);
        }
    }
#endif

    if ((ret = jpeg_enc_clk_prepare_enable(pJpegDev->jpegclk[idx])) != 0) {
        cam_err("[%s] Failed: jpeg_enc_clk_prepare_enable.%d\n", __func__, ret);
        // try to set low freq
        if (0 != jpeg_enc_set_rate(pJpegDev->jpegclk[idx], pJpegDev->jpegclk_low_frequency)) {
            cam_err("[%s] Failed to set low frequency 2: jpeg_enc_set_rate(%d - %d).\n",
                __func__, idx, pJpegDev->jpegclk_low_frequency);
        }

        if ((ret = jpeg_enc_clk_prepare_enable(pJpegDev->jpegclk[idx])) != 0) {
            cam_err("[%s] Failed: jpeg_enc_clk_prepare_enable at low frequency.%d\n", __func__, ret);
        }
        return ret;
    }

    return ret;
}

static void hjpeg_setclk_disable(hjpeg_base_t* pJpegDev, int idx)
{
    int ret;

    cam_debug("%s enter (idx=%d) \n",__func__, idx);

    // === this is new constraint for cs begin===
    if (PC_DRV == pJpegDev->hw_ctl.power_controller) {
        if ((ret = jpeg_enc_set_rate(pJpegDev->jpegclk[idx], pJpegDev->power_off_frequency)) != 0) {
            cam_err("[%s] Failed: jpeg_enc_set_rate.%d\n", __func__, ret);
        }
    }
    // === this is new constraint for cs end ===

    jpeg_enc_clk_disable_unprepare(pJpegDev->jpegclk[idx]);
}

static int hjpeg_clk_ctrl(void __iomem* subctrl1, bool enable)
{
    u32 set_clk;
    u32 cur_clk;
    int ret = 0;
    u32 offset;

    if (clk_ctl_offset) {
        offset = JPGENC_CRG_CFG0;
    } else {
        offset = 0;
    }

    cam_info("%s enter\n",__func__);

    if (enable) {
        set_reg_val(subctrl1 + offset, get_reg_val(subctrl1)|0x1);
    } else {
        set_reg_val(subctrl1 + offset, get_reg_val(subctrl1)&0xFFFFFFFE);   /* [false alarm]:it is a dead code */
    }
    set_clk = enable ? 0x1 : 0x0;
    cur_clk = get_reg_val(subctrl1 + offset);
    if (set_clk != cur_clk) {
        cam_err("%s(%d) isp jpeg clk status %d, clk write failed",__func__, __LINE__, cur_clk);
        ret = -EIO;
    }

    cam_info("%s isp jpeg clk status %d",__func__, cur_clk);
    return ret;
}

static void hjpeg_irq_clr(void __iomem* subctrl1)
{
    u32 set_val;
    void __iomem* subctrl1_irq_clr = subctrl1 + JPGENC_IRQ_REG0;

    cam_info("%s enter\n", __func__);

    set_val = 0x00000002;

/* #ifdef IRQ_TEST */
/*     set_val = 0x6; */
/* #endif */

    set_reg_val(subctrl1_irq_clr, set_val);   /* [false alarm]:it is a dead code */
}

static void hjpeg_irq_mask(void __iomem* subctrl1, bool enable)
{
    u32 set_val;
    u32 cur_val;
    void __iomem* subctrl1_irq_mask = subctrl1 + JPGENC_IRQ_REG1;

    cam_info("%s enter\n", __func__);

    set_val = enable ? 0x0000001D : 0x0000001F;

/* #ifdef IRQ_TEST */
/*      if (enable) { */
/*          set_val = 0x19; */
/*      } */
/* #endif */
    set_reg_val(subctrl1_irq_mask,  set_val);
    cur_val = get_reg_val(subctrl1_irq_mask);
    if (set_val != cur_val) {
        cam_err("%s(%d) isp jpeg irq mask status %u, mask write failed set_val=%u", __func__, __LINE__, cur_val, set_val);
    }

    cam_info("%s isp jpeg irq mask status %u", __func__, cur_val);
}

static void hjpeg_encode_finish(void __iomem* jpegenc, void __iomem* subctl)
{
    u32 value;
    u32 result;
    if (is_irq_merge) {
        value = get_reg_val((void __iomem*)((char*)subctl + JPGENC_IRQ_REG2));
        result = value & IRQ_MERGE_ENCODE_FINISH;
    } else {
        value = get_reg_val((void __iomem*)((char*)jpegenc + JPGENC_JPE_STATUS_RIS_REG));
        result = value & ENCODE_FINISH;
    }

    if (result) {
        tasklet_schedule(&hjpeg_isr_tasklet);
    } else {
        cam_err("err irq JPGENC_IRQ_REG2 status 0x%x", value);

        #if defined( HISP120_CAMERA )
            hjpeg_120_dump_reg();
        #endif
    }

    /*clr jpeg irq*/
    if (is_irq_merge) {
        hjpeg_irq_clr(subctl);
    } else {
        set_reg_val((void __iomem*)((char*)jpegenc + JPGENC_JPE_STATUS_ICR_REG), 0x30);
    }
}

static struct platform_driver
s_hjpeg_driver =
{
    .driver =
    {
        .name = "huawei,hjpeg",
        .owner = THIS_MODULE,/*lint !e64*/
        .of_match_table = s_hjpeg_dt_match,
    },/*lint !e785*/
};/*lint !e785*/

static int32_t hjpeg_platform_probe(
        struct platform_device* pdev )
{
    int32_t ret;
    cam_info("%s enter [%s]\n", __func__, s_hjpeg.name);

    wake_lock_init(&s_hjpeg.power_wakelock, WAKE_LOCK_SUSPEND, "jpeg_power_wakelock");
    mutex_init(&s_hjpeg.wake_lock_mutex);

    hjpeg_register(pdev, &(s_hjpeg.intf));
    s_hjpeg.pdev = pdev;
    atomic_set(&s_hjpeg.jpeg_on, 0); /*lint !e1058 */
    ret = hjpeg_get_dts(pdev);
    if (ret < 0)
    {
        cam_err("[%s] Failed: hjpeg_get_dts.%d\n", __func__, ret);
        return ret;
    }
    ret = hjpeg_map_baseaddr();
    if (ret < 0)
    {
        cam_err("[%s] Failed: hjpeg_map_baseaddr.%d\n", __func__, ret);
    }

#ifdef CONFIG_SMMU_RWERRADDR
    s_hjpeg.hw_ctl.jpg_smmu_rwerraddr_virt = kmalloc(SMMU_RW_ERR_ADDR_SIZE, GFP_KERNEL|__GFP_DMA);
    if (s_hjpeg.hw_ctl.jpg_smmu_rwerraddr_virt) {
        memset_s(s_hjpeg.hw_ctl.jpg_smmu_rwerraddr_virt, SMMU_RW_ERR_ADDR_SIZE, 0, SMMU_RW_ERR_ADDR_SIZE); /*lint !e838 */
    } else {
        cam_err("[%s] kmalloc g_jpg_smmu_rwerraddr_virt fail.\n", __func__);
    }
#endif

#if defined( HISP120_CAMERA )
    if(hjpeg_120_map_reg()) {
        cam_err("hjpeg_120_map_reg failed");
    }
#endif

    return ret;
}

int is_hjpeg_qos_update(void)
{
    cam_debug("%s is_qos_update=%d.", __func__, is_qos_update);
    return is_qos_update;
}

int get_hjpeg_iova_update(void)
{
    cam_debug("%s iova_update_version=%d.", __func__, iova_update_version);
    return iova_update_version;
}

int get_hjpeg_wr_port_addr_update(void)
{
    cam_debug("%s wr_port_addr_update_version=%d.", __func__, wr_port_addr_update_version);
    return wr_port_addr_update_version;
}

int is_cvdr_cfg_update(void)
{
    return cvdr_cfg_update;
}
int is_pixel_fmt_update(void)
{
    return pixel_format_update;
}
static int __init
hjpeg_init_module(void)
{
    cam_info("%s enter\n", __func__);
    // register driver for non-hotpluggable device
    return platform_driver_probe(&s_hjpeg_driver,
            hjpeg_platform_probe);/*lint !e64*/
}

static void __exit
hjpeg_exit_module(void)
{
    cam_info("%s enter\n", __func__);

#ifdef CONFIG_SMMU_RWERRADDR
    if (s_hjpeg.hw_ctl.jpg_smmu_rwerraddr_virt) {
        kfree(s_hjpeg.hw_ctl.jpg_smmu_rwerraddr_virt);
        s_hjpeg.hw_ctl.jpg_smmu_rwerraddr_virt = NULL;
    }
#endif

#if defined( HISP120_CAMERA )
    hjpeg_120_unmap_reg();
#endif

    hjpeg_unmap_baseaddr();
    hjpeg_unregister(s_hjpeg.pdev);
    platform_driver_unregister(&s_hjpeg_driver);

    wake_lock_destroy(&s_hjpeg.power_wakelock);
    mutex_destroy(&s_hjpeg.wake_lock_mutex);
}
/*lint -restore*/

module_init(hjpeg_init_module);/*lint --e{528}*/
module_exit(hjpeg_exit_module);/*lint --e{528}*/

MODULE_DESCRIPTION("hjpeg driver");/*lint --e{753}*/
MODULE_LICENSE("GPL v2");/*lint --e{753}*/

