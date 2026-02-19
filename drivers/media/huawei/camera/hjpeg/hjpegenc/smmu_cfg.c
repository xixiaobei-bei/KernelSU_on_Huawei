/*
 * smmu_cfg.c
 *
 * code for config jpeg qtable/hufftable etc.
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


#include <linux/iommu.h>
#include <asm/io.h>

#include "smmu_cfg.h"
#include "hjpeg_common.h"
#include "cam_log.h"

static int do_cfg_isp_smmu(hjpeg_hw_ctl_t *hw_ctl)
{
    u32 SMMU_SMRX_NS_offset;
    bool bypass;
    void __iomem* smmu_base_addr = hw_ctl->smmu_viraddr;

    cam_debug("%s enter ",__func__);

    if(IS_ERR_OR_NULL(smmu_base_addr)){
        cam_err("%s(%d) base addr error",__func__, __LINE__);
        return -1;
    }

    bypass = (hw_ctl->smmu_bypass == BYPASS_YES);
    if(bypass){
        /* disable SMMU only for JPGENC stream id */
        SMMU_SMRX_NS_offset = SMMU_SMRX_NS(hw_ctl->stream_id[0]);
        set_reg_val((void __iomem*)((char*)smmu_base_addr + SMMU_SMRX_NS_offset),
            get_reg_val((void __iomem*)((char*)smmu_base_addr + SMMU_SMRX_NS_offset)) | 0x1);

        SMMU_SMRX_NS_offset = SMMU_SMRX_NS(hw_ctl->stream_id[1]);
        set_reg_val((void __iomem*)((char*)smmu_base_addr +SMMU_SMRX_NS_offset),
            get_reg_val((void __iomem*)((char*)smmu_base_addr + SMMU_SMRX_NS_offset)) | 0x1);

        SMMU_SMRX_NS_offset = SMMU_SMRX_NS(hw_ctl->stream_id[2]);
        set_reg_val((void __iomem*)((char*)smmu_base_addr + SMMU_SMRX_NS_offset),
            get_reg_val((void __iomem*)((char*)smmu_base_addr + SMMU_SMRX_NS_offset)) | 0x1);

    }
    return 0;
}

static int do_cfg_smmu(hjpeg_hw_ctl_t *hw_ctl, void* pgd)
{
    //void __iomem* smmu_base_addr;
    uint64_t                smmu_rwerraddr_phys = 0;
    u32                     tmpVal;
    u32                     SMMU_SMRX_NS_offset;
    U_SMMU_SMMU_SCR         smmu_scr;
    U_SMMU_SMMU_ADDR_MSB    smmu_addr_msb;
    bool bypass;
    phy_pgd_t * phy_pgd = (phy_pgd_t*)pgd;
    void __iomem* smmu_base_addr = hw_ctl->smmu_viraddr;

    cam_info("%s enter ",__func__);

    if(IS_ERR_OR_NULL(smmu_base_addr)){
        cam_err("%s(%d) base addr error",__func__, __LINE__);
        return -1;
    }
    /* enable SMMU for global */
    smmu_scr.reg32 = get_reg_val((void __iomem*)((char*)smmu_base_addr + SMMU_GLOBAL_BYPASS));

    smmu_scr.bits.glb_bypass = 0x0;

    if (is_hjpeg_qos_update()) {
        smmu_scr.bits.ptw_pf     = 0x5;
    } else {
        smmu_scr.bits.ptw_pf     = 0x1;
    }

    smmu_scr.bits.ptw_mid    = 0x1d;
    cam_notice("%s: set reg 0x%x = 0x%x", __func__, SMMU_GLOBAL_BYPASS, smmu_scr.reg32);
    set_reg_val((void __iomem*)((char*)smmu_base_addr + SMMU_GLOBAL_BYPASS), smmu_scr.reg32);

    bypass = hw_ctl->smmu_bypass == BYPASS_YES;
    if (bypass) {
        /* disable SMMU only for JPGENC stream id */
        SMMU_SMRX_NS_offset = SMMU_SMRX_NS(hw_ctl->stream_id[0]);
        set_reg_val((void __iomem*)((char*)smmu_base_addr + SMMU_SMRX_NS_offset),
            get_reg_val((void __iomem*)((char*)smmu_base_addr + SMMU_SMRX_NS_offset)) | 0x1);

        SMMU_SMRX_NS_offset = SMMU_SMRX_NS(hw_ctl->stream_id[1]);
        set_reg_val((void __iomem*)((char*)smmu_base_addr +SMMU_SMRX_NS_offset),
            get_reg_val((void __iomem*)((char*)smmu_base_addr + SMMU_SMRX_NS_offset)) | 0x1);

        SMMU_SMRX_NS_offset = SMMU_SMRX_NS(hw_ctl->stream_id[2]);
        set_reg_val((void __iomem*)((char*)smmu_base_addr + SMMU_SMRX_NS_offset),
            get_reg_val((void __iomem*)((char*)smmu_base_addr + SMMU_SMRX_NS_offset)) | 0x1);

        tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_FAMA_CTRL1_NS));
        cam_notice("%s: SMMU_FAMA_CTRL1_NS = 0x%x, phy_pgd_fama_bps_msb_ns = 0x%x",
            __func__, tmpVal, phy_pgd->phy_pgd_fama_bps_msb_ns);
        REG_SET_FIELD(tmpVal, FAMA_CTRL0_NS_fama_bps_msb_ns,
            phy_pgd->phy_pgd_fama_bps_msb_ns);
        set_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_FAMA_CTRL1_NS), tmpVal);
        cam_notice("%s: set reg SMMU_FAMA_CTRL1_NS = 0x%x", __func__, tmpVal);

        return 0;
    }

    //config smmu
    tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_INTCLR_NS));
    cam_notice("%s: get reg 0x%x = 0x%x", __func__, SMMU_INTCLR_NS, tmpVal);
    tmpVal = 0xFF;
    set_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_INTCLR_NS), tmpVal);
    cam_notice("%s: set reg 0x%x = 0x%x", __func__, SMMU_INTCLR_NS, tmpVal);

    tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_INTMASK_NS));
    cam_notice("%s: get reg 0x%x = 0x%x", __func__, SMMU_INTMASK_NS, tmpVal);
    tmpVal = 0x0;
    set_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_INTMASK_NS), tmpVal);
    cam_notice("%s: set reg 0x%x = 0x%x", __func__, SMMU_INTMASK_NS, tmpVal);

    //RWERRADDR
    if (hw_ctl->jpg_smmu_rwerraddr_virt) {
        smmu_rwerraddr_phys = virt_to_phys(hw_ctl->jpg_smmu_rwerraddr_virt);
        tmpVal = smmu_rwerraddr_phys & 0xFFFFFFFF;
        cam_debug("%s: set smmu err low addr (0x%x)", __func__, tmpVal);
        set_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_ERR_RDADDR), tmpVal);
        set_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_ERR_WRADDR), tmpVal);
        smmu_addr_msb.reg32 = get_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_ADDR_MSB));
        tmpVal = (smmu_rwerraddr_phys >> 32) & 0x7F;
        cam_debug("%s: set smmu err high addr (0x%x)", __func__, tmpVal);
        smmu_addr_msb.bits.msb_errrd = tmpVal;
        smmu_addr_msb.bits.msb_errwr = tmpVal;
        set_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_ADDR_MSB), smmu_addr_msb.reg32);
    } else {
        cam_warn("%s: jpg_smmu_rwerraddr_virt is NULL!", __func__);
    }

    //SMMU Context Config
    tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_CB_TTBR0));
    cam_notice("%s: get reg 0x%x = 0x%x", __func__, SMMU_CB_TTBR0, tmpVal);
    tmpVal = phy_pgd->phy_pgd_base;
    set_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_CB_TTBR0), tmpVal);
    cam_notice("%s: set reg 0x%x = 0x%x", __func__, SMMU_CB_TTBR0, tmpVal);

    tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_CB_TTBCR));
    cam_notice("%s: get reg 0x%x = 0x%x", __func__, SMMU_CB_TTBCR, tmpVal);
    tmpVal |= 0x1;
    set_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_CB_TTBCR), tmpVal);
    cam_notice("%s: set reg 0x%x = 0x%x", __func__, SMMU_CB_TTBCR, tmpVal);

    // FAMA configuration
    tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_FAMA_CTRL1_NS));
    cam_notice("%s: SMMU_FAMA_CTRL1_NS = 0x%x, phy_pgd_fama_ptw_msb = 0x%x",
        __func__, tmpVal, phy_pgd->phy_pgd_fama_ptw_msb);
    REG_SET_FIELD(tmpVal, FAMA_CTRL1_NS_fama_ptw_msb_ns,
        phy_pgd->phy_pgd_fama_ptw_msb);
    set_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_FAMA_CTRL1_NS), tmpVal);
    cam_notice("%s: set reg SMMU_FAMA_CTRL1_NS = 0x%x", __func__, tmpVal);

    tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_FAMA_CTRL0_NS));
    cam_notice("%s: SMMU_FAMA_CTRL0_NS = 0x%x", __func__, tmpVal);
    REG_SET_FIELD(tmpVal, FAMA_CTRL0_NS_fama_chn_sel_ns, 1);
    set_reg_val((void __iomem*)((char*)smmu_base_addr+SMMU_FAMA_CTRL0_NS), tmpVal);
    cam_notice("%s: set reg SMMU_FAMA_CTRL0_NS = 0x%x", __func__, tmpVal);
    return 0;
}

int hjpeg_smmu_config(hjpeg_hw_ctl_t *hw_ctl, void* pgd)
{
    if(NULL == hw_ctl){
        cam_err("%s(%d) invalid parameter",__func__, __LINE__);
        return -1;
    }

    if (hw_ctl->smmu_type == ST_ISP_SMMU)
    {
        return do_cfg_isp_smmu(hw_ctl);
    }

    if (hw_ctl->smmu_type == ST_SMMU)
    {
        return do_cfg_smmu(hw_ctl, pgd);
    }
    cam_err("%s(%d) error smmu type(%d)",__func__, __LINE__, hw_ctl->smmu_type);
    return -1;
}





