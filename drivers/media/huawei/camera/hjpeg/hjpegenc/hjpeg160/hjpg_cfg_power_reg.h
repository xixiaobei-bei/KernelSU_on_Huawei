/*
 * hjpg_cfg_power_reg.h
 *
 * provide interface for some platform, for power on by config all register .
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


#ifndef __INCLUDE_HJPG160_CFG_POWER_REG_H__
#define __INCLUDE_HJPG160_CFG_POWER_REG_H__

#define REG_BASE_PERI_CRG           ((unsigned int)0xFFF35000) //4k, ~0xFFF35FFF
#define REG_BASE_SCTR               ((unsigned int)0xFFF0A000) //4k, ~0xFFF0AFFF
#define REG_BASE_MEDIA_CRG          ((unsigned int)0xE87FF000) //4k, ~0xE87FFFFF
#define REG_BASE_PMCTRL             ((unsigned int)0xFFF31000) //4k, ~0xFFF31FFF

enum REG_OP_FLAG {
    OP_WRITE_ALL,
    OP_READ_BIT5,
    OP_READ_BIT15,
    OP_READ_STATUS,
};

struct reg_sets {
    u32 base;
    u32 offset;
    u32 val;
    u32 op_flag;
    u32 wait_us;
    char reg_name[64];
};

struct reg_sets isp_reg_sets_powerup[] = {
            //step 1, module mtcmos on
            { REG_BASE_PERI_CRG,    0x150,    0x00000001,     OP_WRITE_ALL,   100,    "CRG_PERIPHPERPWREN" },//Reg-PERPWREN, set BIT0(isppwren) to 1, enable isp power ctrl
            { REG_BASE_PERI_CRG,    0x158,    0x00000001,     OP_READ_STATUS,   1,    "PERPWRSTAT" },
            //step 1.1, module unrst
            { REG_BASE_MEDIA_CRG,    0x034,    0x04000000,      OP_WRITE_ALL,   0,      "MEDIA_CRGPERRSTDIS0" },//Reg-PERRSTDIS0, set BIT26(ip_rst_isp_subsys_cfg) to 1
            { REG_BASE_MEDIA_CRG,    0x038,    0x04000000,      OP_READ_STATUS,   0,      "PERRSTSTAT0" },
            //step 2, module clk enable
            { REG_BASE_MEDIA_CRG,   0x084,    0x30003000,     OP_WRITE_ALL,   0,       "MEDIA_CRGCLKDIV9" },//Reg-CLKDIV9, set BIT29(clkdivmasken[13]), BIT28(clkdivmasken[12]), BIT13(sc_gt_clk_fd_func), BIT12(sc_gt_clk_jpg_func) to 1
            { REG_BASE_MEDIA_CRG,   0x010,    0x1E198000,     OP_WRITE_ALL,   1,       "MEDIA_CRGPEREN1" },//Reg-PEREN1, set BIT28(gt_pclk_jpg), BIT27(gt_aclk_jpg), BIT26(gt_pclk_noc_isp), BIT25(gt_aclk_noc_isp), BIT20(gt_pclk_isp_noc_subsys), BIT19(gt_aclk_isp_noc_subsys), BIT16(gt_clk_fd_func), BIT15(gt_clk_jpg_func) to 1
            { REG_BASE_MEDIA_CRG,   0x018,    0x1E198000,     OP_READ_STATUS,   1,       "PERCLKEN1" },
            //step 3, module clk disable
            { REG_BASE_MEDIA_CRG,    0x014,    0x1E018000,     OP_WRITE_ALL,   1,       "MEDIA_CRGPERDIS1" },//Reg-PERDIS1, set BIT28(gt_pclk_jpg), BIT27(gt_aclk_jpg), BIT26(gt_pclk_noc_isp), BIT25(gt_aclk_noc_isp), BIT16(gt_clk_fd_func), BIT15(gt_clk_jpg_func) to 1
            { REG_BASE_MEDIA_CRG,    0x018,    0x1E018000,     OP_READ_STATUS,   0,       "PERCLKEN1" },
            //step 4, module iso disable
            { REG_BASE_PERI_CRG,   0x148,    0x00000001,     OP_WRITE_ALL,   0,       "CRG_PERIPHISODIS" },//Reg-ISODIS, set BIT0(isp_iso_en) to 1
            { REG_BASE_PERI_CRG,   0x14C,    0x00000001,     OP_READ_STATUS,   0,       "ISOSTAT" },
            //step 5, memory rempair
            //step 6, module unrst
            { REG_BASE_MEDIA_CRG,   0x034,    0x01E00000,     OP_WRITE_ALL,   0,       "MEDIA_CRGPERRSTDIS0" },//Reg-PERRSTDIS0, set BIT24(ip_rst_fd_func), BIT23(ip_rst_jpg_func), BIT22(ip_prst_jpg), BIT21(ip_arst_jpg) to 1
            { REG_BASE_MEDIA_CRG,    0x038,    0x01E00000,      OP_READ_STATUS,   0,      "PERRSTSTAT0" },
            { REG_BASE_MEDIA_CRG,   0x040,    0x0000000C,     OP_WRITE_ALL,   0,       "MEDIA_CRGPERRSTDIS1" },//Reg-PERRSTDIS1, set BIT3(ip_rst_noc_isp_cfg), BIT2(ip_rst_noc_isp) to 1
            { REG_BASE_MEDIA_CRG,   0x044,    0x0000000C,     OP_WRITE_ALL,   0,       "PERRSTSTAT1" },
            //step 7, module clk enable
            { REG_BASE_MEDIA_CRG,   0x010,    0x1E018000,     OP_WRITE_ALL,   0,       "MEDIA_CRGPEREN1" },//Reg-PEREN1, set BIT28(gt_pclk_jpg), BIT27(gt_aclk_jpg), BIT26(gt_pclk_noc_isp), BIT25(gt_aclk_noc_isp), BIT16(gt_clk_fd_func), BIT15(gt_clk_jpg_func) to 1
            { REG_BASE_MEDIA_CRG,   0x018,    0x1E018000,     OP_READ_STATUS,   1,       "PERCLKEN1" },
            //step 8, bus idle clear
            { REG_BASE_PMCTRL,      0x380,    0x00200000,     OP_WRITE_ALL,   0,       "PMCTRLNOC_POWER_IDLEREQ_0" },//Reg-NOC_POWER_IDLEREQ_0, set BIT21(biten[5]) to 1, request noc_isp_power_idlereq to 0
            { REG_BASE_PMCTRL,      0x384,    0x00000020,     OP_READ_STATUS,    0,       "PMCTRLNOC_POWER_IDLEACK_0" },//Reg-NOC_POWER_IDLEACK_0, BIT5, expect noc_isp_power_idleack is 0
            { REG_BASE_PMCTRL,      0x388,    0x00000020,     OP_READ_STATUS,    0,       "PMCTRLNOC_POWER_IDLE_0" },//Reg-NOC_POWER_IDLE_0, BIT5, expect noc_isp_power_idle is 0
};


struct reg_sets isp_reg_sets_powerdown[] = {
            //step 1, bus idle set
            { REG_BASE_PMCTRL,      0x380,    0x00200020,     OP_WRITE_ALL,   0,      "PMCTRLNOC_POWER_IDLEREQ_0" },//Reg-NOC_POWER_IDLEREQ_0, set BIT21(biten[5]), BIT5(noc_isp_power_idlereq) to 1, request noc_isp_power_idlereq to 1
            { REG_BASE_PMCTRL,      0x384,    0x00000020,     OP_READ_STATUS,    1,       "PMCTRLNOC_POWER_IDLEACK_0" },//Reg-NOC_POWER_IDLEACK_0, BIT5, expect noc_isp_power_idleack is 1
            { REG_BASE_PMCTRL,      0x388,    0x00000020,     OP_READ_STATUS,    1,       "PMCTRLNOC_POWER_IDLE_0" },//Reg-NOC_POWER_IDLE_0, BIT5, expect noc_isp_power_idle is 1
            //step 2, module rst
            { REG_BASE_MEDIA_CRG,   0x030,    0x01E00000,     OP_WRITE_ALL,   0,      "MEDIA_CRGPERRSTEN0" },//Reg-PERRSTEN0, set BIT24(ip_rst_fd_func), BIT23(ip_rst_jpg_func), BIT22(ip_prst_jpg), BIT21(ip_arst_jpg) to 1
            { REG_BASE_MEDIA_CRG,   0x038,    0x01E00000,     OP_READ_STATUS,   1,      "PERRSTSTAT0" },
            { REG_BASE_MEDIA_CRG,   0x03C,    0x0000000C,     OP_WRITE_ALL,   0,      "MEDIA_CRGPERRSTEN1" },//Reg-PERRSTEN1, set BIT3(ip_rst_noc_isp_cfg), BIT2(ip_rst_noc_isp) to 1
            { REG_BASE_MEDIA_CRG,   0x044,    0x0000000C,     OP_READ_STATUS,   1,      "PERRSTSTAT1" },
            //step 3, module clk disable
            { REG_BASE_MEDIA_CRG,       0x014,    0x1E018000,     OP_WRITE_ALL,   0,      "MEDIA_CRGPERDIS1" },//Reg-PERDIS1, set BIT28(gt_pclk_jpg), BIT27(gt_aclk_jpg), BIT26(gt_pclk_noc_isp), BIT25(gt_aclk_noc_isp), BIT16(gt_clk_fd_func), BIT15(gt_clk_jpg_func) to 1
            { REG_BASE_MEDIA_CRG,    0x018,    0x1E018000,     OP_READ_STATUS,   0,       "PERCLKEN1" },
            { REG_BASE_MEDIA_CRG,       0x014,    0x00180000,     OP_WRITE_ALL,   0,      "MEDIA_CRGPERDIS1" },//Reg-PERDIS1, set BIT20(gt_pclk_isp_noc_subsys), BIT19(gt_aclk_isp_noc_subsys) to 1
            { REG_BASE_MEDIA_CRG,    0x018,    0x00180000,     OP_READ_STATUS,   0,       "PERCLKEN1" },
            { REG_BASE_MEDIA_CRG,       0x084,    0x30003000,     OP_WRITE_ALL,   0,      "MEDIA_CRGCLKDIV9" },//Reg-CLKDIV9, set BIT29(clkdivmasken[13]), BIT28(clkdivmasken[12]), BIT13(sc_gt_clk_fd_func), BIT12(sc_gt_clk_jpg_func)  to 1
            //step 4, module iso
            { REG_BASE_PERI_CRG,       0x144,    0x00000001,     OP_WRITE_ALL,   0,      "CRG_PERIPHISOEN" },//Reg-ISOEN, set BIT0(isp_iso_en) to 1
            { REG_BASE_PERI_CRG,       0x14C,    0x00000001,     OP_READ_STATUS,   1,      "ISOSTAT" },
                  //step 4.2, module unrst
            { REG_BASE_MEDIA_CRG,   0x030,    0x04000000,     OP_WRITE_ALL,   0,      "MEDIA_CRGPERRSTEN0" },//Reg-PERRSTEN0, set BIT26(ip_rst_isp_subsys_cfg) to 1
            { REG_BASE_MEDIA_CRG,   0x038,    0x04000000,     OP_READ_STATUS,   1,      "PERRSTSTAT0" },
            //step 5, module mtcmos off
            { REG_BASE_PERI_CRG,        0x154,    0x00000001,     OP_WRITE_ALL,   0,      "CRG_PERIPHPERPWRDIS" },//Reg-PERPWRDIS, set BIT0(isppwrdis) to 1
            { REG_BASE_PERI_CRG,        0x158,    0x00000001,     OP_READ_STATUS,   0,      "PERPWRSTAT" },
};

struct reg_sets vivobus_reg_sets_powerup[] = {
            //step 1, module mtcmos on
            //step 2, module clk enable
            { REG_BASE_MEDIA_CRG,       0x084,    0x00080008,     OP_WRITE_ALL,   0,      "MEDIA_CRGCLKDIV9" },//Reg-CLKDIV9, set BIT19(reserved?), BIT3(sc_gt_clk_vivobus) to 1
            { REG_BASE_MEDIA_CRG,       0x000,    0x08040040,     OP_WRITE_ALL,   1,      "MEDIA_CRGPEREN0" },//Reg-PEREN0, set BIT27(gt_clk_vivobus2ddrc_core), BIT18(gt_clk_vivobus), BIT6(gt_clk_vivobus2ddrc)  to 1
            { REG_BASE_MEDIA_CRG,       0x008,    0x08040040,     OP_READ_STATUS,   1,      "PERCLKEN0" },
            //step 3, module clk disable
            { REG_BASE_MEDIA_CRG,       0x004,    0x00040040,     OP_WRITE_ALL,   1,      "MEDIA_CRGPERDIS0" },//Reg-PERDIS0, set BIT18(gt_clk_vivobus), BIT6(gt_clk_vivobus2ddrc) to 1, no need to disable gt_clk_vivobus2ddrc_core, which is set when power down
            { REG_BASE_MEDIA_CRG,       0x008,    0x00040040,     OP_READ_STATUS,   0,      "PERCLKEN0" },
            //step 4, module iso disable
            //step 5, memory rempair
            //step 6, module unrst
            //step 7, module clk enable
            { REG_BASE_MEDIA_CRG,       0x000,    0x00040040,     OP_WRITE_ALL,   0,      "MEDIA_CRGPEREN0" },//Reg-PEREN0, set BIT18(gt_clk_vivobus), BIT6(gt_clk_vivobus2ddrc)  to 1
            { REG_BASE_MEDIA_CRG,       0x008,    0x00040040,     OP_READ_STATUS,   1,      "PERCLKEN0" },
            //step 8, bus idle clear
            { REG_BASE_PMCTRL,            0x380,    0x80000000,     OP_WRITE_ALL,   0,      "PMCTRLNOC_POWER_IDLEREQ_0" },//Reg-NOC_POWER_IDLEREQ, set BIT31(biten[15]), request noc_dss_power_idlereq to 0
            { REG_BASE_PMCTRL,            0x384,    0x00008000,     OP_READ_STATUS,    0,      "PMCTRLNOC_POWER_IDLEACK_0" },//Reg-NOC_POWER_IDLEACK, BIT15, expect noc_dss_power_idleack is 0
            { REG_BASE_PMCTRL,            0x388,    0x00008000,     OP_READ_STATUS,    0,      "PMCTRLNOC_POWER_IDLE_0" },//Reg-NOC_POWER_IDLE, BIT15, expect noc_dss_power_idle is 0
};

struct reg_sets vivobus_reg_sets_powerdown[] = {
            //step 1, bus idle set
            { REG_BASE_PMCTRL,            0x380,    0x80008000,     OP_WRITE_ALL,   0,      "PMCTRLNOC_POWER_IDLEREQ_0" },//Reg-NOC_POWER_IDLEREQ, set BIT31(biten[15]), BIT15(noc_vivobus_power_idlereq), request noc_dss_power_idlereq to 1
            { REG_BASE_PMCTRL,            0x384,    0x00008000,     OP_READ_STATUS,    1,      "PMCTRLNOC_POWER_IDLEACK_0" },//Reg-NOC_POWER_IDLEACK, BIT13, expect noc_dss_power_idleack is 1
            { REG_BASE_PMCTRL,            0x388,    0x00008000,     OP_READ_STATUS,    1,      "PMCTRLNOC_POWER_IDLE_0" },//Reg-NOC_POWER_IDLE, BIT13, expect noc_dss_power_idle is 1
            //step 2, module rst
            //step 3, module clk disable
            { REG_BASE_MEDIA_CRG,       0x004,    0x08000000,     OP_WRITE_ALL,   0,      "MEDIA_CRGPERDIS0" },//Reg-PERDIS0, set BIT27(gt_clk_vivobus2ddrc_core) to 1, close gt_clk_vivobus2ddrc_core first
            { REG_BASE_MEDIA_CRG,       0x008,    0x08000000,     OP_READ_STATUS,   0,      "PERCLKEN0" },
            { REG_BASE_MEDIA_CRG,       0x004,    0x00040040,     OP_WRITE_ALL,   0,      "MEDIA_CRGPERDIS0" },//Reg-PERDIS0, set BIT18(gt_clk_vivobus), BIT6(gt_clk_vivobus2ddrc) to 1, close clks of media
            { REG_BASE_MEDIA_CRG,       0x008,    0x00040040,     OP_READ_STATUS,   0,      "PERCLKEN0" },
            { REG_BASE_MEDIA_CRG,       0x084,    0x00080000,     OP_WRITE_ALL,   0,      "MEDIA_CRGCLKDIV9" },//Reg-CLKDIV9, set BIT19(reserved?) to 1
};

struct reg_sets media_subsys_powerup[] = {
            //step 1, module mtcmos on
            { REG_BASE_PERI_CRG,        0x150,    0x00000020,     OP_WRITE_ALL,   100,      "CRG_PERIPHPERPWREN" },//Reg-PERPWREN, set BIT5(mediasubsyspwren) to 1, enable media subsys power ctrl
            { REG_BASE_PERI_CRG,    0x158,    0x00000020,     OP_READ_STATUS,   1,    "PERPWRSTAT" },
            //step 1.1, module unrst
            { REG_BASE_PERI_CRG,        0x0A0,    0x00040000,     OP_WRITE_ALL,   0,      "CRG_PERIPHPERRSTDIS5" },//Reg-PERRSTDIS5, set BIT18(ip_rst_media_crg) to 1
            { REG_BASE_PERI_CRG,        0x0A4,    0x00040000,     OP_READ_STATUS,   0,      "PERRSTSTAT5" },
            //step 2, module clk enable
            { REG_BASE_PERI_CRG,        0x410,    0x7C002028,     OP_WRITE_ALL,   0,      "CRG_PERIPHPEREN6" },//Reg-PEREN6, set BIT30(gt_clk_ppll6_media),BIT29(gt_clk_ppll7_media), BIT28(gt_clk_ppll3_media), BIT27(gt_clk_ppll2_media),BIT26(gt_clk_ppll4_media), BIT13(gt_clk_cfgbus_media), BIT5(gt_pclk_media_crg), and BIT3(gt_clk_media2cfgbus) to 1
            { REG_BASE_PERI_CRG,        0x418,    0x7C002028,     OP_READ_STATUS,   1,      "PERCLKEN6" },
            { REG_BASE_SCTR,              0x1B0,    0x00000040,     OP_WRITE_ALL,   0,      "SCTRLSCPEREN4" },//Reg-SCPEREN4, set BIT6(gt_clk_ppll0_media) to 1
            { REG_BASE_SCTR,              0x1B8,    0x00000040,     OP_READ_STATUS,   1,      "SCPERCLKEN4" },
            { REG_BASE_PERI_CRG,        0x040,    0x00000040,     OP_WRITE_ALL,   1,      "CRG_PERIPHPEREN4" },//Reg-PEREN4, set BIT6(gt_clk_media_tcxo) to 1
            { REG_BASE_PERI_CRG,        0x048,    0x00000040,     OP_READ_STATUS,   1,      "PERCLKEN4" },
            //step 3, module clk disable
            { REG_BASE_PERI_CRG,        0x414,   0x7C002028,     OP_WRITE_ALL,   0,      "CRG_PERIPHPERDIS6" },//Reg-PERDIS6, set BIT30(gt_clk_ppll6_media), BIT29(gt_clk_ppll7_media), BIT28(gt_clk_ppll3_media), BIT27(gt_clk_ppll2_media),BIT26(gt_clk_ppll4_media), BIT13(gt_clk_cfgbus_media), BIT5(gt_pclk_media_crg), and BIT3(gt_clk_media2cfgbus) to 1
            { REG_BASE_PERI_CRG,        0x418,   0x7C002028,     OP_READ_STATUS,   0,      "PERCLKEN6" },
            { REG_BASE_SCTR,              0x1B4,    0x00000040,     OP_WRITE_ALL,   0,      "SCTRLSCPERDIS4" },//Reg-SCPERDIS4, set BIT6(gt_clk_ppll0_media) to 1
            { REG_BASE_SCTR,              0x1B8,    0x00000040,     OP_READ_STATUS,   0,      "SCPERCLKEN4" },
            { REG_BASE_PERI_CRG,        0x044,    0x00000040,     OP_WRITE_ALL,   1,      "CRG_PERIPHPERDIS4" },//Reg-PERDIS4, set BIT6(gt_clk_media_tcxo) to 1
            { REG_BASE_PERI_CRG,        0x048,    0x00000040,     OP_READ_STATUS,   0,      "PERCLKEN4" },
            //step 4, module iso disable
            { REG_BASE_PERI_CRG,        0x148,    0x00000040,     OP_WRITE_ALL,   0,      "CRG_PERIPHISODIS" },//Reg-ISODIS, set BIT6(mediasubsys_iso_en) to 1
            { REG_BASE_PERI_CRG,   0x14C,    0x00000040,     OP_READ_STATUS,   0,       "ISOSTAT" },
            //step 5, memory rempair
            //step 6, module unrst
            { REG_BASE_PERI_CRG,        0x0A0,    0x00020000,     OP_WRITE_ALL,   0,      "CRG_PERIPHPERRSTDIS5" },//Reg-PERRSTDIS5, set BIT17(ip_rst_media) to 1
            { REG_BASE_PERI_CRG,        0x0A4,    0x00020000,     OP_READ_STATUS,   0,      "PERRSTSTAT5" },
            //step 7, module clk enable
            { REG_BASE_PERI_CRG,        0x410,     0x7C002028,     OP_WRITE_ALL,   0,      "CRG_PERIPHPEREN6" },//Reg-PEREN6, set BIT30(gt_clk_ppll6_media), BIT29(gt_clk_ppll7_media), BIT28(gt_clk_ppll3_media), BIT27(gt_clk_ppll2_media),BIT26(gt_clk_ppll4_media), BIT13(gt_clk_cfgbus_media), BIT5(gt_pclk_media_crg), and BIT3(gt_clk_media2cfgbus) to 1
            { REG_BASE_PERI_CRG,        0x418,    0x7C002028,     OP_READ_STATUS,   1,      "PERCLKEN6" },
            { REG_BASE_SCTR,              0x1B0,    0x00000040,     OP_WRITE_ALL,   0,      "SCTRLSCPEREN4" },//Reg-SCPEREN4, set BIT6(gt_clk_ppll0_media) to 1
            { REG_BASE_SCTR,              0x1B8,    0x00000040,     OP_READ_STATUS,   1,      "SCPERCLKEN4" },
            { REG_BASE_PERI_CRG,        0x040,    0x00000040,     OP_WRITE_ALL,   0,      "CRG_PERIPHPEREN4" },//Reg-PEREN4, set BIT6(gt_clk_media_tcxo) to 1
            { REG_BASE_PERI_CRG,        0x048,    0x00000040,     OP_READ_STATUS,   1,      "PERCLKEN4" },
};

struct reg_sets media_subsys_powerdown[] = {
            //step 1, module rst
            { REG_BASE_PERI_CRG,        0x09C,    0x00020000,     OP_WRITE_ALL,   0,      "CRG_PERIPHPERRSTEN5" },//Reg-PERRSTEN5, set BIT17(ip_rst_media) to 1
            { REG_BASE_PERI_CRG,        0x0A4,    0x00020000,     OP_READ_STATUS,   1,      "PERRSTSTAT5" },
            //step 3, module clk disable
            { REG_BASE_PERI_CRG,        0x414,    0x7C002028,     OP_WRITE_ALL,   0,      "CRG_PERIPHPERDIS6" },//Reg-PERDIS6, set BIT30(gt_clk_ppll6_media), BIT29(gt_clk_ppll7_media), BIT28(gt_clk_ppll3_media), BIT27(gt_clk_ppll2_media),BIT26(gt_clk_ppll4_media), BIT13(gt_clk_cfgbus_media), BIT5(gt_pclk_media_crg), and BIT3(gt_clk_media2cfgbus) to 1
            { REG_BASE_PERI_CRG,        0x418,   0x7C002028,     OP_READ_STATUS,   0,      "PERCLKEN6" },
            { REG_BASE_SCTR,              0x1B4,    0x00000040,     OP_WRITE_ALL,   0,      "SCTRLSCPERDIS4" },//Reg-SCPERDIS4, set BIT6(gt_clk_ppll0_media) to 1
            { REG_BASE_SCTR,              0x1B8,    0x00000040,     OP_READ_STATUS,   0,      "SCPERCLKEN4" },
            { REG_BASE_PERI_CRG,        0x044,    0x00000040,     OP_WRITE_ALL,   0,      "CRG_PERIPHPERDIS4" },//Reg-PERDIS4, set BIT6(gt_clk_media_tcxo) to 1
            { REG_BASE_PERI_CRG,        0x048,    0x00000040,     OP_READ_STATUS,   0,      "PERCLKEN4" },
            //step 4, module iso
            { REG_BASE_PERI_CRG,        0x144,    0x00000040,     OP_WRITE_ALL,   0,      "CRG_PERIPHISOEN" },//Reg-ISOEN, set BIT6(mediasubsys_iso_en) to 1
            { REG_BASE_PERI_CRG,       0x14C,    0x00000040,     OP_READ_STATUS,   1,      "ISOSTAT" },
            //step 4.2, module unrst
            { REG_BASE_PERI_CRG,        0x09C,    0x00040000,     OP_WRITE_ALL,   0,      "CRG_PERIPHPERRSTEN5" },//Reg-PERRSTEN5, set BIT18(ip_rst_media_crg) to 1
            { REG_BASE_PERI_CRG,        0x0A4,    0x00020000,     OP_READ_STATUS,   1,      "PERRSTSTAT5" },
            //step 5, module mtcmos off
            { REG_BASE_PERI_CRG,        0x154,    0x00000020,     OP_WRITE_ALL,   0,      "CRG_PERIPHPERPWRDIS" },//Reg-PERPWRDIS, set BIT5(mediasubsyspwrdis) to 1
            { REG_BASE_PERI_CRG,        0x158,    0x00000001,     OP_READ_STATUS,   0,      "PERPWRSTAT" },
};

#endif//__INCLUDE_HJPG160_CFG_POWER_REG_H__