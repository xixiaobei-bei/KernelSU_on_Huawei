

#ifndef _HI1106_PCIE_CTRL_RB_REGS_H_
#define _HI1106_PCIE_CTRL_RB_REGS_H_

/* This file is generated automatically. Do not modify it */
#define PCIE_CTRL_RB_BASE    (0x40108000)

/* SYS CTL ID寄存器 0x000 */
typedef union {
    struct {
        unsigned int pcie_ctrl_sys_ctl_id : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctrl_sys_ctl_id;
#define PCIE_CTRL_RB_PCIE_CTRL_SYS_CTL_ID_OFF    0x000
#define PCIE_CTRL_RB_PCIE_CTRL_SYS_CTL_ID_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTRL_SYS_CTL_ID_OFF)

/* 通用寄存器 0x0010 */
typedef union {
    struct {
        unsigned int pcie_ctrl_gp_reg0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctrl_gp_reg0;
#define PCIE_CTRL_RB_PCIE_CTRL_GP_REG0_OFF    0x0010
#define PCIE_CTRL_RB_PCIE_CTRL_GP_REG0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTRL_GP_REG0_OFF)

/* 通用寄存器 0x0014 */
typedef union {
    struct {
        unsigned int pcie_ctrl_gp_reg1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctrl_gp_reg1;
#define PCIE_CTRL_RB_PCIE_CTRL_GP_REG1_OFF    0x0014
#define PCIE_CTRL_RB_PCIE_CTRL_GP_REG1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTRL_GP_REG1_OFF)

/* 测试寄存器 0x100 */
typedef union {
    struct {
        unsigned int pcie_ctrl_test_value : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctrl_test_value;
#define PCIE_CTRL_RB_PCIE_CTRL_TEST_VALUE_OFF    0x100
#define PCIE_CTRL_RB_PCIE_CTRL_TEST_VALUE_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTRL_TEST_VALUE_OFF)

/* 测试结果寄存器 0x104 */
typedef union {
    struct {
        unsigned int pcie_ctrl_test_result : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctrl_test_result;
#define PCIE_CTRL_RB_PCIE_CTRL_TEST_RESULT_OFF    0x104
#define PCIE_CTRL_RB_PCIE_CTRL_TEST_RESULT_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTRL_TEST_RESULT_OFF)

/* PCIE参数控制寄存器 0x108 */
typedef union {
    struct {
        unsigned int pcie_sp_ram_tmod : 7; /* 0:6 */
        unsigned int reserved0 : 25; /* 7:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_mem_sp_ram_tmode;
#define PCIE_CTRL_RB_PCIE_MEM_SP_RAM_TMODE_OFF    0x108
#define PCIE_CTRL_RB_PCIE_MEM_SP_RAM_TMODE_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MEM_SP_RAM_TMODE_OFF)

/* PCIE参数控制寄存器 0x10C */
typedef union {
    struct {
        unsigned int pcie_tp_ram_tmod : 8; /* 0:7 */
        unsigned int reserved0 : 24; /* 8:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_mem_tp_ram_tmode;
#define PCIE_CTRL_RB_PCIE_MEM_TP_RAM_TMODE_OFF    0x10C
#define PCIE_CTRL_RB_PCIE_MEM_TP_RAM_TMODE_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MEM_TP_RAM_TMODE_OFF)

/* PCIE参数控制寄存器 0x204 */
typedef union {
    struct {
        unsigned int reserved0 : 1; /* 0 */
        unsigned int pcie_para_efuse_sel_sel : 1; /* 1 */
        unsigned int pcie_para_efuse_sel_manu : 1; /* 2 */
        unsigned int soc_app_sris_mode : 1; /* 3 */
        unsigned int reserved1 : 28; /* 4:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_msg_mode;
#define PCIE_CTRL_RB_PCIE_MSG_MODE_OFF    0x204
#define PCIE_CTRL_RB_PCIE_MSG_MODE_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MSG_MODE_OFF)

/* Master总线边带信号配置寄存器 0x208 */
typedef union {
    struct {
        unsigned int soc_mstr_bmisc_info_cpl_stat : 2; /* 0:1 */
        unsigned int soc_mstr_rmisc_info : 13; /* 2:14 */
        unsigned int soc_mstr_rmisc_info_cpl_stat : 2; /* 15:16 */
        unsigned int reserved0 : 15; /* 17:31 */
    } bits;
    unsigned int as_dword;
} hreg_mstr_bus_cfg;
#define PCIE_CTRL_RB_MSTR_BUS_CFG_OFF    0x208
#define PCIE_CTRL_RB_MSTR_BUS_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MSTR_BUS_CFG_OFF)

/* Slave总线边带信号配置寄存器 0x20C */
typedef union {
    struct {
        unsigned int soc_slv_awmisc_info : 22; /* 0:21 */
        unsigned int soc_slv_awmisc_info_p_tag : 8; /* 22:29 */
        unsigned int soc_slv_awmisc_info_atu_bypass : 1; /* 30 */
        unsigned int reserved0 : 1; /* 31 */
    } bits;
    unsigned int as_dword;
} hreg_slv_bus_cfg0;
#define PCIE_CTRL_RB_SLV_BUS_CFG0_OFF    0x20C
#define PCIE_CTRL_RB_SLV_BUS_CFG0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_SLV_BUS_CFG0_OFF)

/* Slave总线边带信号配置寄存器 0x210 */
typedef union {
    struct {
        unsigned int soc_slv_awmisc_info_hdr_34dw0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_slv_bus_cfg1;
#define PCIE_CTRL_RB_SLV_BUS_CFG1_OFF    0x210
#define PCIE_CTRL_RB_SLV_BUS_CFG1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_SLV_BUS_CFG1_OFF)

/* Slave总线边带信号配置寄存器 0x214 */
typedef union {
    struct {
        unsigned int soc_slv_awmisc_info_hdr_34dw1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_slv_bus_cfg2;
#define PCIE_CTRL_RB_SLV_BUS_CFG2_OFF    0x214
#define PCIE_CTRL_RB_SLV_BUS_CFG2_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_SLV_BUS_CFG2_OFF)

/* Slave总线边带信号配置寄存器 0x218 */
typedef union {
    struct {
        unsigned int soc_slv_armisc_info : 22; /* 0:21 */
        unsigned int soc_slv_armisc_info_atu_bypass : 1; /* 22 */
        unsigned int soc_slv_wmisc_info_ep : 1; /* 23 */
        unsigned int soc_slv_wmisc_info_silentdrop : 1; /* 24 */
        unsigned int reserved0 : 7; /* 25:31 */
    } bits;
    unsigned int as_dword;
} hreg_slv_bus_cfg3;
#define PCIE_CTRL_RB_SLV_BUS_CFG3_OFF    0x218
#define PCIE_CTRL_RB_SLV_BUS_CFG3_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_SLV_BUS_CFG3_OFF)

/* MSI中断上报配置寄存器 0x21C */
typedef union {
    struct {
        unsigned int soc_ven_msi_func_num : 3; /* 0:2 */
        unsigned int soc_ven_msi_tc : 3; /* 3:5 */
        unsigned int reserved0 : 26; /* 6:31 */
    } bits;
    unsigned int as_dword;
} hreg_msi_cfg;
#define PCIE_CTRL_RB_MSI_CFG_OFF    0x21C
#define PCIE_CTRL_RB_MSI_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MSI_CFG_OFF)

/* 低功耗进出配置寄存器 0x220 */
typedef union {
    struct {
        unsigned int soc_outband_pwrup_cmd : 1; /* 0 */
        unsigned int soc_apps_pm_xmt_pme : 1; /* 1 */
        unsigned int soc_app_req_entr_l1 : 1; /* 2 */
        unsigned int soc_app_ready_entr_l23 : 1; /* 3 */
        unsigned int soc_app_req_exit_l1 : 1; /* 4 */
        unsigned int reserved0 : 1; /* 5 */
        unsigned int soc_sys_aux_pwr_det : 1; /* 6 */
        unsigned int soc_app_xfer_pending : 1; /* 7 */
        unsigned int reserved1 : 24; /* 8:31 */
    } bits;
    unsigned int as_dword;
} hreg_low_power_cfg;
#define PCIE_CTRL_RB_LOW_POWER_CFG_OFF    0x220
#define PCIE_CTRL_RB_LOW_POWER_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_LOW_POWER_CFG_OFF)

/* PCIE状态上报寄存器 0x224 */
typedef union {
    struct {
        unsigned int soc_pm_xtlh_block_tlp : 1; /* 0 */
        unsigned int soc_cfg_bus_master_en : 1; /* 1 */
        unsigned int soc_cfg_mem_space_en : 1; /* 2 */
        unsigned int reserved0 : 1; /* 3 */
        unsigned int soc_cfg_pm_no_soft_rst : 1; /* 4 */
        unsigned int soc_rdlh_link_up : 1; /* 5 */
        unsigned int soc_pm_curnt_state : 3; /* 6:8 */
        unsigned int soc_smlh_ltssm_state : 6; /* 9:14 */
        unsigned int soc_smlh_link_up : 1; /* 15 */
        unsigned int soc_brdg_slv_xfer_pending : 1; /* 16 */
        unsigned int soc_brdg_dbi_xfer_pending : 1; /* 17 */
        unsigned int reserved1 : 1; /* 18 */
        unsigned int soc_radm_xfer_pending : 1; /* 19 */
        unsigned int soc_pm_dstate : 3; /* 20:22 */
        unsigned int soc_radm_q_not_empty : 1; /* 23 */
        unsigned int soc_pm_linkst_in_l0s : 1; /* 24 */
        unsigned int soc_pm_linkst_in_l1 : 1; /* 25 */
        unsigned int soc_pm_linkst_in_l2 : 1; /* 26 */
        unsigned int soc_pm_linkst_l2_exit : 1; /* 27 */
        unsigned int soc_pm_linkst_in_l1sub : 1; /* 28 */
        unsigned int soc_mac_phy_pclk_rate : 3; /* 29:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_status0;
#define PCIE_CTRL_RB_PCIE_STATUS0_OFF    0x224
#define PCIE_CTRL_RB_PCIE_STATUS0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STATUS0_OFF)

/* APP消息配置寄存器 0x228 */
typedef union {
    struct {
        unsigned int soc_app_req_retry_en : 1; /* 0 */
        unsigned int soc_app_pf_req_retry_en : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_app_msg_cfg;
#define PCIE_CTRL_RB_APP_MSG_CFG_OFF    0x228
#define PCIE_CTRL_RB_APP_MSG_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_APP_MSG_CFG_OFF)

/* PCIE FC上报 0x230 */
typedef union {
    struct {
        unsigned int soc_rtlh_rfc_data : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_rtlh_rfc;
#define PCIE_CTRL_RB_PCIE_RTLH_RFC_OFF    0x230
#define PCIE_CTRL_RB_PCIE_RTLH_RFC_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_RTLH_RFC_OFF)

/* PCIE内部状态上报寄存器 0x23C */
typedef union {
    struct {
        unsigned int soc_cfg_relax_order_en : 1; /* 0 */
        unsigned int soc_cfg_no_snoop_en : 1; /* 1 */
        unsigned int soc_cfg_int_disable : 1; /* 2 */
        unsigned int soc_cfg_rcb : 1; /* 3 */
        unsigned int reserved0 : 28; /* 4:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_reg_status;
#define PCIE_CTRL_RB_PCIE_REG_STATUS_OFF    0x23C
#define PCIE_CTRL_RB_PCIE_REG_STATUS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_REG_STATUS_OFF)

/* PCIE状态上报寄存器 0x240 */
typedef union {
    struct {
        unsigned int soc_radm_cpl_timeout : 1; /* 0 */
        unsigned int reserved0 : 7; /* 1:7 */
        unsigned int soc_trgt_cpl_timeout : 1; /* 8 */
        unsigned int soc_radm_qoverflow : 1; /* 9 */
        unsigned int reserved1 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_status1;
#define PCIE_CTRL_RB_PCIE_STATUS1_OFF    0x240
#define PCIE_CTRL_RB_PCIE_STATUS1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STATUS1_OFF)

/* PCIE状态清除寄存器 0x244 */
typedef union {
    struct {
        unsigned int soc_radm_cpl_timeout_clr : 1; /* 0 */
        unsigned int reserved0 : 7; /* 1:7 */
        unsigned int soc_trgt_cpl_timeout_clr : 1; /* 8 */
        unsigned int soc_radm_qoverflow_clr : 1; /* 9 */
        unsigned int reserved1 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_status_clr;
#define PCIE_CTRL_RB_PCIE_STATUS_CLR_OFF    0x244
#define PCIE_CTRL_RB_PCIE_STATUS_CLR_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STATUS_CLR_OFF)

/* PCIE RADM CPL状态上报寄存器 0x248 */
typedef union {
    struct {
        unsigned int soc_radm_timeout_cpl_tag : 8; /* 0:7 */
        unsigned int soc_radm_timeout_cpl_len : 12; /* 8:19 */
        unsigned int soc_radm_timeout_cpl_attr : 2; /* 20:21 */
        unsigned int soc_radm_timeout_cpl_tc : 3; /* 22:24 */
        unsigned int soc_radm_timeout_cpl_func_num : 3; /* 25:27 */
        unsigned int reserved0 : 4; /* 28:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_radm_cpl_status;
#define PCIE_CTRL_RB_PCIE_RADM_CPL_STATUS_OFF    0x248
#define PCIE_CTRL_RB_PCIE_RADM_CPL_STATUS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_RADM_CPL_STATUS_OFF)

/* PCIE RADM VENDOR消息上报寄存器 0x24C */
typedef union {
    struct {
        unsigned int radm_vendor_msg_req_id : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_radm_vendor_msg;
#define PCIE_CTRL_RB_PCIE_RADM_VENDOR_MSG_OFF    0x24C
#define PCIE_CTRL_RB_PCIE_RADM_VENDOR_MSG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_RADM_VENDOR_MSG_OFF)

/* PCIE VENDOR MSG上报寄存器 0x250 */
typedef union {
    struct {
        unsigned int radm_vendor_msg_payload0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_radm_vendor_payload0;
#define PCIE_CTRL_RB_PCIE_RADM_VENDOR_PAYLOAD0_OFF    0x250
#define PCIE_CTRL_RB_PCIE_RADM_VENDOR_PAYLOAD0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_RADM_VENDOR_PAYLOAD0_OFF)

/* PCIE VENDOR MSG上报寄存器 0x254 */
typedef union {
    struct {
        unsigned int radm_vendor_msg_payload1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_radm_vendor_payload1;
#define PCIE_CTRL_RB_PCIE_RADM_VENDOR_PAYLOAD1_OFF    0x254
#define PCIE_CTRL_RB_PCIE_RADM_VENDOR_PAYLOAD1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_RADM_VENDOR_PAYLOAD1_OFF)

/* PCIE LTR ID上报寄存器 0x258 */
typedef union {
    struct {
        unsigned int radm_msg_ltr_req_id : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ltr_req_id;
#define PCIE_CTRL_RB_PCIE_LTR_REQ_ID_OFF    0x258
#define PCIE_CTRL_RB_PCIE_LTR_REQ_ID_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_LTR_REQ_ID_OFF)

/* PCIE LTR LATENCY上报寄存器 0x25C */
typedef union {
    struct {
        unsigned int radm_msg_ltr_latency : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ltr_latency;
#define PCIE_CTRL_RB_PCIE_LTR_LATENCY_OFF    0x25C
#define PCIE_CTRL_RB_PCIE_LTR_LATENCY_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_LTR_LATENCY_OFF)

/* PCIE LTR MSG上报寄存器 0x260 */
typedef union {
    struct {
        unsigned int radm_msg_ltr_payload0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ltr_payload0;
#define PCIE_CTRL_RB_PCIE_LTR_PAYLOAD0_OFF    0x260
#define PCIE_CTRL_RB_PCIE_LTR_PAYLOAD0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_LTR_PAYLOAD0_OFF)

/* PCIE LTR MSG上报寄存器 0x264 */
typedef union {
    struct {
        unsigned int radm_msg_ltr_payload1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ltr_payload1;
#define PCIE_CTRL_RB_PCIE_LTR_PAYLOAD1_OFF    0x264
#define PCIE_CTRL_RB_PCIE_LTR_PAYLOAD1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_LTR_PAYLOAD1_OFF)

/* PCIE UNLOCK ID上报寄存器 0x268 */
typedef union {
    struct {
        unsigned int radm_msg_unlock_req_id : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_unlock_req_id;
#define PCIE_CTRL_RB_PCIE_UNLOCK_REQ_ID_OFF    0x268
#define PCIE_CTRL_RB_PCIE_UNLOCK_REQ_ID_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_UNLOCK_REQ_ID_OFF)

/* PCIE UNLOCK MSG上报寄存器 0x26C */
typedef union {
    struct {
        unsigned int radm_msg_unlock_payload0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_unlock_payload0;
#define PCIE_CTRL_RB_PCIE_UNLOCK_PAYLOAD0_OFF    0x26C
#define PCIE_CTRL_RB_PCIE_UNLOCK_PAYLOAD0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_UNLOCK_PAYLOAD0_OFF)

/* PCIE UNLOCK MSG上报寄存器 0x270 */
typedef union {
    struct {
        unsigned int radm_msg_unlock_payload1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_unlock_payload1;
#define PCIE_CTRL_RB_PCIE_UNLOCK_PAYLOAD1_OFF    0x270
#define PCIE_CTRL_RB_PCIE_UNLOCK_PAYLOAD1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_UNLOCK_PAYLOAD1_OFF)

/* PCIE OBFF ID上报寄存器 0x274 */
typedef union {
    struct {
        unsigned int radm_msg_idle_req_id : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_idle_req_id;
#define PCIE_CTRL_RB_PCIE_IDLE_REQ_ID_OFF    0x274
#define PCIE_CTRL_RB_PCIE_IDLE_REQ_ID_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_IDLE_REQ_ID_OFF)

/* PCIE IDLE MSG上报寄存器 0x278 */
typedef union {
    struct {
        unsigned int radm_msg_idle_payload0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_idle_payload0;
#define PCIE_CTRL_RB_PCIE_IDLE_PAYLOAD0_OFF    0x278
#define PCIE_CTRL_RB_PCIE_IDLE_PAYLOAD0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_IDLE_PAYLOAD0_OFF)

/* PCIE IDLE MSG上报寄存器 0x27C */
typedef union {
    struct {
        unsigned int radm_msg_idle_payload1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_idle_payload1;
#define PCIE_CTRL_RB_PCIE_IDLE_PAYLOAD1_OFF    0x27C
#define PCIE_CTRL_RB_PCIE_IDLE_PAYLOAD1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_IDLE_PAYLOAD1_OFF)

/* PCIE OBFF ID上报寄存器 0x280 */
typedef union {
    struct {
        unsigned int radm_msg_obff_req_id : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_obff_req_id;
#define PCIE_CTRL_RB_PCIE_OBFF_REQ_ID_OFF    0x280
#define PCIE_CTRL_RB_PCIE_OBFF_REQ_ID_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_OBFF_REQ_ID_OFF)

/* PCIE OBFF MSG上报寄存器 0x284 */
typedef union {
    struct {
        unsigned int radm_msg_obff_payload0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_obff_payload0;
#define PCIE_CTRL_RB_PCIE_OBFF_PAYLOAD0_OFF    0x284
#define PCIE_CTRL_RB_PCIE_OBFF_PAYLOAD0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_OBFF_PAYLOAD0_OFF)

/* PCIE OBFF MSG上报寄存器 0x288 */
typedef union {
    struct {
        unsigned int radm_msg_obff_payload1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_obff_payload1;
#define PCIE_CTRL_RB_PCIE_OBFF_PAYLOAD1_OFF    0x288
#define PCIE_CTRL_RB_PCIE_OBFF_PAYLOAD1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_OBFF_PAYLOAD1_OFF)

/* PCIE CPU_ACT ID上报寄存器 0x28C */
typedef union {
    struct {
        unsigned int radm_msg_cpu_active_req_id : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_cpu_act_req_id;
#define PCIE_CTRL_RB_PCIE_CPU_ACT_REQ_ID_OFF    0x28C
#define PCIE_CTRL_RB_PCIE_CPU_ACT_REQ_ID_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CPU_ACT_REQ_ID_OFF)

/* PCIE CPU ACT MSG上报寄存器 0x290 */
typedef union {
    struct {
        unsigned int radm_msg_cpu_active_payload0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_cpu_act_payload0;
#define PCIE_CTRL_RB_PCIE_CPU_ACT_PAYLOAD0_OFF    0x290
#define PCIE_CTRL_RB_PCIE_CPU_ACT_PAYLOAD0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CPU_ACT_PAYLOAD0_OFF)

/* PCIE CPU ACT MSG上报寄存器 0x294 */
typedef union {
    struct {
        unsigned int radm_msg_cpu_active_payload1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_cpu_act_payload1;
#define PCIE_CTRL_RB_PCIE_CPU_ACT_PAYLOAD1_OFF    0x294
#define PCIE_CTRL_RB_PCIE_CPU_ACT_PAYLOAD1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CPU_ACT_PAYLOAD1_OFF)

/* PCIE PM TURNOFF ID上报寄存器 0x298 */
typedef union {
    struct {
        unsigned int radm_pm_turnoff_req_id : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_pm_turnoff_req_id;
#define PCIE_CTRL_RB_PCIE_PM_TURNOFF_REQ_ID_OFF    0x298
#define PCIE_CTRL_RB_PCIE_PM_TURNOFF_REQ_ID_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PM_TURNOFF_REQ_ID_OFF)

/* PCIE PM TURNOFF MSG上报寄存器 0x29C */
typedef union {
    struct {
        unsigned int radm_pm_turnoff_payload0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_pm_turnoff_payload0;
#define PCIE_CTRL_RB_PCIE_PM_TURNOFF_PAYLOAD0_OFF    0x29C
#define PCIE_CTRL_RB_PCIE_PM_TURNOFF_PAYLOAD0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PM_TURNOFF_PAYLOAD0_OFF)

/* PCIE PM TURNOFF MSG上报寄存器 0x2A0 */
typedef union {
    struct {
        unsigned int radm_pm_turnoff_payload1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_pm_turnoff_payload1;
#define PCIE_CTRL_RB_PCIE_PM_TURNOFF_PAYLOAD1_OFF    0x2A0
#define PCIE_CTRL_RB_PCIE_PM_TURNOFF_PAYLOAD1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PM_TURNOFF_PAYLOAD1_OFF)

/* PCIE TRGT超时状态上报寄存器 0x2A4 */
typedef union {
    struct {
        unsigned int soc_trgt_timeout_lookup_id : 8; /* 0:7 */
        unsigned int soc_trgt_timeout_cpl_len : 12; /* 8:19 */
        unsigned int soc_trgt_timeout_cpl_attr : 2; /* 20:21 */
        unsigned int soc_trgt_timeout_cpl_tc : 3; /* 22:24 */
        unsigned int soc_trgt_timeout_cpl_func_num : 3; /* 25:27 */
        unsigned int reserved0 : 4; /* 28:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_trgt_cpl_status;
#define PCIE_CTRL_RB_PCIE_TRGT_CPL_STATUS_OFF    0x2A4
#define PCIE_CTRL_RB_PCIE_TRGT_CPL_STATUS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_TRGT_CPL_STATUS_OFF)

/* PCIE MSG中断屏蔽前上报寄存器 0x2A8 */
typedef union {
    struct {
        unsigned int reserved0 : 4; /* 0:3 */
        unsigned int soc_pcie_send_f_err : 1; /* 4 */
        unsigned int soc_pcie_send_nf_err : 1; /* 5 */
        unsigned int soc_pcie_send_cor_err : 1; /* 6 */
        unsigned int soc_radm_msg_cpu_active : 1; /* 7 */
        unsigned int soc_radm_msg_idle : 1; /* 8 */
        unsigned int soc_radm_msg_obff : 1; /* 9 */
        unsigned int soc_radm_msg_turnoff : 1; /* 10 */
        unsigned int soc_radm_msg_unlock : 1; /* 11 */
        unsigned int soc_radm_msg_ltr : 1; /* 12 */
        unsigned int soc_radm_vendor_msg : 1; /* 13 */
        unsigned int reserved1 : 1; /* 14 */
        unsigned int soc_radm_pm_pme : 1; /* 15 */
        unsigned int reserved2 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_msg_intr;
#define PCIE_CTRL_RB_PCIE_MSG_INTR_OFF    0x2A8
#define PCIE_CTRL_RB_PCIE_MSG_INTR_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MSG_INTR_OFF)

/* PCIE MSG中断屏蔽后上报寄存器 0x2AC */
typedef union {
    struct {
        unsigned int reserved0 : 4; /* 0:3 */
        unsigned int soc_pcie_send_f_err_status : 1; /* 4 */
        unsigned int soc_pcie_send_nf_err_status : 1; /* 5 */
        unsigned int soc_pcie_send_cor_err_status : 1; /* 6 */
        unsigned int soc_radm_msg_cpu_active_status : 1; /* 7 */
        unsigned int soc_radm_msg_idle_status : 1; /* 8 */
        unsigned int soc_radm_msg_obff_status : 1; /* 9 */
        unsigned int soc_radm_msg_turnoff_status : 1; /* 10 */
        unsigned int soc_radm_msg_unlock_status : 1; /* 11 */
        unsigned int soc_radm_msg_ltr_status : 1; /* 12 */
        unsigned int soc_radm_vendor_msg_status : 1; /* 13 */
        unsigned int link_down_irq : 1; /* 14 */
        unsigned int soc_radm_pm_pme_st : 1; /* 15 */
        unsigned int reserved1 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_msg_intr_status;
#define PCIE_CTRL_RB_PCIE_MSG_INTR_STATUS_OFF    0x2AC
#define PCIE_CTRL_RB_PCIE_MSG_INTR_STATUS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MSG_INTR_STATUS_OFF)

/* PCIE MSG中断清除寄存器 0x2B0 */
typedef union {
    struct {
        unsigned int reserved0 : 4; /* 0:3 */
        unsigned int soc_pcie_send_f_err_clr : 1; /* 4 */
        unsigned int soc_pcie_send_nf_err_clr : 1; /* 5 */
        unsigned int soc_pcie_send_cor_err_clr : 1; /* 6 */
        unsigned int soc_radm_msg_cpu_active_clr : 1; /* 7 */
        unsigned int soc_radm_msg_idle_clr : 1; /* 8 */
        unsigned int soc_radm_msg_obff_clr : 1; /* 9 */
        unsigned int soc_radm_msg_turnoff_clr : 1; /* 10 */
        unsigned int soc_radm_msg_unlock_clr : 1; /* 11 */
        unsigned int soc_radm_msg_ltr_clr : 1; /* 12 */
        unsigned int soc_radm_vendor_msg_clr : 1; /* 13 */
        unsigned int link_down_irq_clr : 1; /* 14 */
        unsigned int soc_radm_pm_pme_clr : 1; /* 15 */
        unsigned int reserved1 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_msg_intr_clr;
#define PCIE_CTRL_RB_PCIE_MSG_INTR_CLR_OFF    0x2B0
#define PCIE_CTRL_RB_PCIE_MSG_INTR_CLR_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MSG_INTR_CLR_OFF)

/* PCIE MSG中断屏蔽寄存器 0x2B4 */
typedef union {
    struct {
        unsigned int reserved0 : 4; /* 0:3 */
        unsigned int soc_pcie_send_f_err_mask : 1; /* 4 */
        unsigned int soc_pcie_send_nf_err_mask : 1; /* 5 */
        unsigned int soc_pcie_send_cor_err_mask : 1; /* 6 */
        unsigned int soc_radm_msg_cpu_active_mask : 1; /* 7 */
        unsigned int soc_radm_msg_idle_mask : 1; /* 8 */
        unsigned int soc_radm_msg_obff_mask : 1; /* 9 */
        unsigned int soc_radm_msg_turnoff_mask : 1; /* 10 */
        unsigned int soc_radm_msg_unlock_mask : 1; /* 11 */
        unsigned int soc_radm_msg_ltr_mask : 1; /* 12 */
        unsigned int soc_radm_vendor_msg_mask : 1; /* 13 */
        unsigned int reserved1 : 1; /* 14 */
        unsigned int soc_radm_pm_pme_mask : 1; /* 15 */
        unsigned int reserved2 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_msg_intr_mask;
#define PCIE_CTRL_RB_PCIE_MSG_INTR_MASK_OFF    0x2B4
#define PCIE_CTRL_RB_PCIE_MSG_INTR_MASK_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MSG_INTR_MASK_OFF)

/* PCIE APP LTR配置寄存器 0x2B8 */
typedef union {
    struct {
        unsigned int soc_app_ltr_msg_latency : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_app_ltr_cfg0;
#define PCIE_CTRL_RB_PCIE_APP_LTR_CFG0_OFF    0x2B8
#define PCIE_CTRL_RB_PCIE_APP_LTR_CFG0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_APP_LTR_CFG0_OFF)

/* PCIE APP LTR配置寄存器 0x2BC */
typedef union {
    struct {
        unsigned int soc_app_ltr_msg_func_num : 3; /* 0:2 */
        unsigned int reserved0 : 29; /* 3:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_app_ltr_cfg1;
#define PCIE_CTRL_RB_PCIE_APP_LTR_CFG1_OFF    0x2BC
#define PCIE_CTRL_RB_PCIE_APP_LTR_CFG1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_APP_LTR_CFG1_OFF)

/* PCIE MAX LATENCY上报寄存器 0x2C0 */
typedef union {
    struct {
        unsigned int soc_cfg_ltr_max_latency : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_max_latency;
#define PCIE_CTRL_RB_PCIE_MAX_LATENCY_OFF    0x2C0
#define PCIE_CTRL_RB_PCIE_MAX_LATENCY_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MAX_LATENCY_OFF)

/* PCIE消息配置寄存器 0x2CC */
typedef union {
    struct {
        unsigned int soc_app_hdr_valid : 1; /* 0 */
        unsigned int soc_app_ltr_msg_req : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_msg_cfg;
#define PCIE_CTRL_RB_PCIE_MSG_CFG_OFF    0x2CC
#define PCIE_CTRL_RB_PCIE_MSG_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MSG_CFG_OFF)

/* PCIE配置寄存器 0x2D0 */
typedef union {
    struct {
        unsigned int pcie_app_ltssm_enable : 1; /* 0 */
        unsigned int soc_app_clk_req_n : 1; /* 1 */
        unsigned int pcie_linkdown_auto_rstn_disable : 1; /* 2 */
        unsigned int app_clk_pm_en : 1; /* 3 */
        unsigned int app_l1_pwr_off_en : 1; /* 4 */
        unsigned int device_type : 1; /* 5 */
        unsigned int phy_rxvalid_hand_disable : 1; /* 6 */
        unsigned int phy_ref_clk_out_en : 1; /* 7 */
        unsigned int soc_app_xfer_pending_sel : 1; /* 8 */
        unsigned int soc_apps_pm_xmt_turnoff : 1; /* 9 */
        unsigned int soc_app_unlock_msg : 1; /* 10 */
        unsigned int soc_app_l1sub_disable : 1; /* 11 */
        unsigned int soc_app_bus_num : 8; /* 12:19 */
        unsigned int soc_app_dev_num : 5; /* 20:24 */
        unsigned int reserved0 : 7; /* 25:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_cfg;
#define PCIE_CTRL_RB_PCIE_CFG_OFF    0x2D0
#define PCIE_CTRL_RB_PCIE_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CFG_OFF)

/* OBFF DEC配置寄存器 0x2D4 */
typedef union {
    struct {
        unsigned int host2device_tx_intr_set : 1; /* 0 */
        unsigned int host2device_rx_intr_set : 1; /* 1 */
        unsigned int device2host_tx_intr_set : 1; /* 2 */
        unsigned int device2host_rx_intr_set : 1; /* 3 */
        unsigned int reserved0 : 28; /* 4:31 */
    } bits;
    unsigned int as_dword;
} hreg_host2device_intr_set;
#define PCIE_CTRL_RB_HOST2DEVICE_INTR_SET_OFF    0x2D4
#define PCIE_CTRL_RB_HOST2DEVICE_INTR_SET_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_HOST2DEVICE_INTR_SET_OFF)

/* 核间通信寄存器 0x2D8 */
typedef union {
    struct {
        unsigned int host2device_reg0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_device_reg0;
#define PCIE_CTRL_RB_HOST_DEVICE_REG0_OFF    0x2D8
#define PCIE_CTRL_RB_HOST_DEVICE_REG0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_HOST_DEVICE_REG0_OFF)

/* 核间通信寄存器 0x2DC */
typedef union {
    struct {
        unsigned int host2device_reg1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_device_reg1;
#define PCIE_CTRL_RB_HOST_DEVICE_REG1_OFF    0x2DC
#define PCIE_CTRL_RB_HOST_DEVICE_REG1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_HOST_DEVICE_REG1_OFF)

/* PCIE屏蔽前中断上报 0x2E4 */
typedef union {
    struct {
        unsigned int host2device_tx_intr_raw_status : 1; /* 0 */
        unsigned int host2device_rx_intr_raw_status : 1; /* 1 */
        unsigned int device2host_tx_intr_raw_status : 1; /* 2 */
        unsigned int device2host_rx_intr_raw_status : 1; /* 3 */
        unsigned int device2host_intr_raw_status : 1; /* 4 */
        unsigned int pcie_msg_irq_raw_status : 1; /* 5 */
        unsigned int mac_n2_intr_raw_status : 1; /* 6 */
        unsigned int mac_n1_intr_raw_status : 1; /* 7 */
        unsigned int phy_n2_intr_raw_status : 1; /* 8 */
        unsigned int phy_n1_intr_raw_status : 1; /* 9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_intr_raw_status;
#define PCIE_CTRL_RB_HOST_INTR_RAW_STATUS_OFF    0x2E4
#define PCIE_CTRL_RB_HOST_INTR_RAW_STATUS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_HOST_INTR_RAW_STATUS_OFF)

/* PCIE中断上报 0x2E8 */
typedef union {
    struct {
        unsigned int host2device_tx_intr_mask : 1; /* 0 */
        unsigned int host2device_rx_intr_mask : 1; /* 1 */
        unsigned int device2host_tx_intr_mask : 1; /* 2 */
        unsigned int device2host_rx_intr_mask : 1; /* 3 */
        unsigned int device2host_intr_mask : 1; /* 4 */
        unsigned int pcie_msg_irq_mask : 1; /* 5 */
        unsigned int mac_n2_intr_mask : 1; /* 6 */
        unsigned int mac_n1_intr_mask : 1; /* 7 */
        unsigned int phy_n2_intr_mask : 1; /* 8 */
        unsigned int phy_n1_intr_mask : 1; /* 9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_intr_mask;
#define PCIE_CTRL_RB_HOST_INTR_MASK_OFF    0x2E8
#define PCIE_CTRL_RB_HOST_INTR_MASK_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_HOST_INTR_MASK_OFF)

/* PCIE屏蔽后中断上报 0x2EC */
typedef union {
    struct {
        unsigned int host2device_tx_intr_status : 1; /* 0 */
        unsigned int host2device_rx_intr_status : 1; /* 1 */
        unsigned int device2host_tx_intr_status : 1; /* 2 */
        unsigned int device2host_rx_intr_status : 1; /* 3 */
        unsigned int device2host_intr_status : 1; /* 4 */
        unsigned int pcie_msg_irq_status : 1; /* 5 */
        unsigned int mac_n2_intr_status : 1; /* 6 */
        unsigned int mac_n1_intr_status : 1; /* 7 */
        unsigned int phy_n2_intr_status : 1; /* 8 */
        unsigned int phy_n1_intr_status : 1; /* 9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_intr_status;
#define PCIE_CTRL_RB_HOST_INTR_STATUS_OFF    0x2EC
#define PCIE_CTRL_RB_HOST_INTR_STATUS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_HOST_INTR_STATUS_OFF)

/* PCIE中断上报 0x2F0 */
typedef union {
    struct {
        unsigned int host2device_tx_intr_clr : 1; /* 0 */
        unsigned int host2device_rx_intr_clr : 1; /* 1 */
        unsigned int device2host_tx_intr_clr : 1; /* 2 */
        unsigned int device2host_rx_intr_clr : 1; /* 3 */
        unsigned int reserved0 : 1; /* 4 */
        unsigned int reserved1 : 27; /* 5:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_intr_clr;
#define PCIE_CTRL_RB_HOST_INTR_CLR_OFF    0x2F0
#define PCIE_CTRL_RB_HOST_INTR_CLR_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_HOST_INTR_CLR_OFF)

/* 性能统计配置寄存器 0x2F4 */
typedef union {
    struct {
        unsigned int pfmc_test_enable : 1; /* 0 */
        unsigned int pfmc_test_finish : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_performance_test_cfg0;
#define PCIE_CTRL_RB_PERFORMANCE_TEST_CFG0_OFF    0x2F4
#define PCIE_CTRL_RB_PERFORMANCE_TEST_CFG0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PERFORMANCE_TEST_CFG0_OFF)

/* 性能统计时长配置寄存器 0x2F8 */
typedef union {
    struct {
        unsigned int test_delay_cnt : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_test_delay_cnt_low;
#define PCIE_CTRL_RB_TEST_DELAY_CNT_LOW_OFF    0x2F8
#define PCIE_CTRL_RB_TEST_DELAY_CNT_LOW_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_TEST_DELAY_CNT_LOW_OFF)

/* 性能统计上报寄存器 0x2FC */
typedef union {
    struct {
        unsigned int pcie_slv_write_performance0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_slv_write_performance0;
#define PCIE_CTRL_RB_SLV_WRITE_PERFORMANCE0_OFF    0x2FC
#define PCIE_CTRL_RB_SLV_WRITE_PERFORMANCE0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_SLV_WRITE_PERFORMANCE0_OFF)

/* 性能统计上报寄存器 0x300 */
typedef union {
    struct {
        unsigned int pcie_slv_write_performance1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_slv_write_performance1;
#define PCIE_CTRL_RB_SLV_WRITE_PERFORMANCE1_OFF    0x300
#define PCIE_CTRL_RB_SLV_WRITE_PERFORMANCE1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_SLV_WRITE_PERFORMANCE1_OFF)

/* 性能统计上报寄存器 0x304 */
typedef union {
    struct {
        unsigned int pcie_slv_read_performance0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_slv_read_performance0;
#define PCIE_CTRL_RB_SLV_READ_PERFORMANCE0_OFF    0x304
#define PCIE_CTRL_RB_SLV_READ_PERFORMANCE0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_SLV_READ_PERFORMANCE0_OFF)

/* 性能统计上报寄存器 0x308 */
typedef union {
    struct {
        unsigned int pcie_slv_read_performance1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_slv_read_performance1;
#define PCIE_CTRL_RB_SLV_READ_PERFORMANCE1_OFF    0x308
#define PCIE_CTRL_RB_SLV_READ_PERFORMANCE1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_SLV_READ_PERFORMANCE1_OFF)

/* 性能统计上报寄存器 0x30C */
typedef union {
    struct {
        unsigned int pcie_mst_write_performance0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_mst_write_performance0;
#define PCIE_CTRL_RB_MST_WRITE_PERFORMANCE0_OFF    0x30C
#define PCIE_CTRL_RB_MST_WRITE_PERFORMANCE0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MST_WRITE_PERFORMANCE0_OFF)

/* 性能统计上报寄存器 0x310 */
typedef union {
    struct {
        unsigned int pcie_mst_write_performance1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_mst_write_performance1;
#define PCIE_CTRL_RB_MST_WRITE_PERFORMANCE1_OFF    0x310
#define PCIE_CTRL_RB_MST_WRITE_PERFORMANCE1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MST_WRITE_PERFORMANCE1_OFF)

/* 性能统计上报寄存器 0x314 */
typedef union {
    struct {
        unsigned int pcie_mst_read_performance0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_mst_read_performance0;
#define PCIE_CTRL_RB_MST_READ_PERFORMANCE0_OFF    0x314
#define PCIE_CTRL_RB_MST_READ_PERFORMANCE0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MST_READ_PERFORMANCE0_OFF)

/* 性能统计上报寄存器 0x318 */
typedef union {
    struct {
        unsigned int pcie_mst_read_performance1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_mst_read_performance1;
#define PCIE_CTRL_RB_MST_READ_PERFORMANCE1_OFF    0x318
#define PCIE_CTRL_RB_MST_READ_PERFORMANCE1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MST_READ_PERFORMANCE1_OFF)

/* 操作采样上报寄存器 0x31C */
typedef union {
    struct {
        unsigned int pcie_slv_awaddr0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt0;
#define PCIE_CTRL_RB_MONITOR_RPT0_OFF    0x31C
#define PCIE_CTRL_RB_MONITOR_RPT0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT0_OFF)

/* 操作采样上报寄存器 0x320 */
typedef union {
    struct {
        unsigned int pcie_slv_awaddr1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt1;
#define PCIE_CTRL_RB_MONITOR_RPT1_OFF    0x320
#define PCIE_CTRL_RB_MONITOR_RPT1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT1_OFF)

/* 操作采样上报寄存器 0x324 */
typedef union {
    struct {
        unsigned int pcie_slv_wdata0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt2;
#define PCIE_CTRL_RB_MONITOR_RPT2_OFF    0x324
#define PCIE_CTRL_RB_MONITOR_RPT2_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT2_OFF)

/* 操作采样上报寄存器 0x328 */
typedef union {
    struct {
        unsigned int pcie_slv_wdata1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt3;
#define PCIE_CTRL_RB_MONITOR_RPT3_OFF    0x328
#define PCIE_CTRL_RB_MONITOR_RPT3_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT3_OFF)

/* 操作采样上报寄存器 0x32C */
typedef union {
    struct {
        unsigned int pcie_slv_araddr0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt6;
#define PCIE_CTRL_RB_MONITOR_RPT6_OFF    0x32C
#define PCIE_CTRL_RB_MONITOR_RPT6_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT6_OFF)

/* 操作采样上报寄存器 0x330 */
typedef union {
    struct {
        unsigned int pcie_slv_araddr1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt7;
#define PCIE_CTRL_RB_MONITOR_RPT7_OFF    0x330
#define PCIE_CTRL_RB_MONITOR_RPT7_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT7_OFF)

/* 操作采样上报寄存器 0x334 */
typedef union {
    struct {
        unsigned int pcie_slv_rdata0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt8;
#define PCIE_CTRL_RB_MONITOR_RPT8_OFF    0x334
#define PCIE_CTRL_RB_MONITOR_RPT8_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT8_OFF)

/* 操作采样上报寄存器 0x338 */
typedef union {
    struct {
        unsigned int pcie_slv_rdata1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt9;
#define PCIE_CTRL_RB_MONITOR_RPT9_OFF    0x338
#define PCIE_CTRL_RB_MONITOR_RPT9_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT9_OFF)

/* 操作采样上报寄存器 0x33C */
typedef union {
    struct {
        unsigned int pcie_mst_awaddr0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt10;
#define PCIE_CTRL_RB_MONITOR_RPT10_OFF    0x33C
#define PCIE_CTRL_RB_MONITOR_RPT10_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT10_OFF)

/* 操作采样上报寄存器 0x340 */
typedef union {
    struct {
        unsigned int pcie_mst_awaddr1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt12;
#define PCIE_CTRL_RB_MONITOR_RPT12_OFF    0x340
#define PCIE_CTRL_RB_MONITOR_RPT12_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT12_OFF)

/* 操作采样上报寄存器 0x344 */
typedef union {
    struct {
        unsigned int pcie_mst_wdata0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt14;
#define PCIE_CTRL_RB_MONITOR_RPT14_OFF    0x344
#define PCIE_CTRL_RB_MONITOR_RPT14_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT14_OFF)

/* 操作采样上报寄存器 0x348 */
typedef union {
    struct {
        unsigned int pcie_mst_wdata1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt15;
#define PCIE_CTRL_RB_MONITOR_RPT15_OFF    0x348
#define PCIE_CTRL_RB_MONITOR_RPT15_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT15_OFF)

/* 操作采样上报寄存器 0x34C */
typedef union {
    struct {
        unsigned int pcie_mst_araddr0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt17;
#define PCIE_CTRL_RB_MONITOR_RPT17_OFF    0x34C
#define PCIE_CTRL_RB_MONITOR_RPT17_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT17_OFF)

/* 操作采样上报寄存器 0x350 */
typedef union {
    struct {
        unsigned int pcie_mst_araddr1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt18;
#define PCIE_CTRL_RB_MONITOR_RPT18_OFF    0x350
#define PCIE_CTRL_RB_MONITOR_RPT18_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT18_OFF)

/* 操作采样上报寄存器 0x354 */
typedef union {
    struct {
        unsigned int pcie_mst_rdata0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt20;
#define PCIE_CTRL_RB_MONITOR_RPT20_OFF    0x354
#define PCIE_CTRL_RB_MONITOR_RPT20_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT20_OFF)

/* 操作采样上报寄存器 0x358 */
typedef union {
    struct {
        unsigned int pcie_mst_rdata1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt21;
#define PCIE_CTRL_RB_MONITOR_RPT21_OFF    0x358
#define PCIE_CTRL_RB_MONITOR_RPT21_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT21_OFF)

/* 操作采样上报寄存器 0x35C */
typedef union {
    struct {
        unsigned int pcie_slv_arid0 : 12; /* 0:11 */
        unsigned int reserved0 : 20; /* 12:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt22;
#define PCIE_CTRL_RB_MONITOR_RPT22_OFF    0x35C
#define PCIE_CTRL_RB_MONITOR_RPT22_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT22_OFF)

/* 操作采样上报寄存器 0x360 */
typedef union {
    struct {
        unsigned int pcie_slv_awid0 : 12; /* 0:11 */
        unsigned int reserved0 : 20; /* 12:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_rpt23;
#define PCIE_CTRL_RB_MONITOR_RPT23_OFF    0x360
#define PCIE_CTRL_RB_MONITOR_RPT23_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_RPT23_OFF)

/* 操作采样配置寄存器 0x364 */
typedef union {
    struct {
        unsigned int pcie_monitor_enable : 1; /* 0 */
        unsigned int pcie_monitor_mode : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_monitor_cfg0;
#define PCIE_CTRL_RB_MONITOR_CFG0_OFF    0x364
#define PCIE_CTRL_RB_MONITOR_CFG0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_MONITOR_CFG0_OFF)

/* 性能统计配置寄存器 0x368 */
typedef union {
    struct {
        unsigned int wr_slv_wdata_counter_en : 1; /* 0 */
        unsigned int wr_slv_rdata_counter_en : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_delay_monitor_cfg;
#define PCIE_CTRL_RB_DELAY_MONITOR_CFG_OFF    0x368
#define PCIE_CTRL_RB_DELAY_MONITOR_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_DELAY_MONITOR_CFG_OFF)

/* DELAY采样上报寄存器 0x36C */
typedef union {
    struct {
        unsigned int slv_wdata_latency : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_slv_wdata_latency;
#define PCIE_CTRL_RB_SLV_WDATA_LATENCY_OFF    0x36C
#define PCIE_CTRL_RB_SLV_WDATA_LATENCY_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_SLV_WDATA_LATENCY_OFF)

/* DELAY采样上报寄存器 0x370 */
typedef union {
    struct {
        unsigned int slv_rdata_latency : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_slv_rdata_latency;
#define PCIE_CTRL_RB_SLV_RDATA_LATENCY_OFF    0x370
#define PCIE_CTRL_RB_SLV_RDATA_LATENCY_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_SLV_RDATA_LATENCY_OFF)

/* PCIE LANE FLIP配置寄存器 0x374 */
typedef union {
    struct {
        unsigned int pcie_rx_lane_flip_en : 1; /* 0 */
        unsigned int pcie_tx_lane_flip_en : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_lane_flip_cfg;
#define PCIE_CTRL_RB_PCIE_LANE_FLIP_CFG_OFF    0x374
#define PCIE_CTRL_RB_PCIE_LANE_FLIP_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_LANE_FLIP_CFG_OFF)

/* PCIE错误上报寄存器 0x378 */
typedef union {
    struct {
        unsigned int cfg_rcvr_err_sts : 1; /* 0 */
        unsigned int cfg_bad_tlp_err_sts : 1; /* 1 */
        unsigned int cfg_bad_dllp_err_sts : 1; /* 2 */
        unsigned int cfg_replay_timer_timeout_err_sts : 1; /* 3 */
        unsigned int cfg_replay_number_rollover_err_sts : 1; /* 4 */
        unsigned int cfg_corrected_internal_err_sts : 1; /* 5 */
        unsigned int cfg_ecrc_err_sts : 1; /* 6 */
        unsigned int cfg_dl_protocol_err_sts : 1; /* 7 */
        unsigned int cfg_surprise_down_er_sts : 1; /* 8 */
        unsigned int cfg_mlf_tlp_err_sts : 1; /* 9 */
        unsigned int cfg_fc_protocol_err_sts : 1; /* 10 */
        unsigned int cfg_rcvr_overflow_err_sts : 1; /* 11 */
        unsigned int cfg_uncor_internal_err_sts : 1; /* 12 */
        unsigned int radm_fatal_err : 1; /* 13 */
        unsigned int radm_nonfatal_err : 1; /* 14 */
        unsigned int radm_correctable_err : 1; /* 15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_err_rpt_cfg;
#define PCIE_CTRL_RB_PCIE_ERR_RPT_CFG_OFF    0x378
#define PCIE_CTRL_RB_PCIE_ERR_RPT_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_ERR_RPT_CFG_OFF)

/* PCIE TRGT1 ATU上报寄存器1 0x37C */
typedef union {
    struct {
        unsigned int soc_radm_trgt1_atu_cbuf_err1 : 16; /* 0:15 */
        unsigned int soc_radm_trgt1_atu_sloc_match1 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_radm_trgt1_atu_rpt1;
#define PCIE_CTRL_RB_PCIE_RADM_TRGT1_ATU_RPT1_OFF    0x37C
#define PCIE_CTRL_RB_PCIE_RADM_TRGT1_ATU_RPT1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_RADM_TRGT1_ATU_RPT1_OFF)

/* PCIE TRGT1 ATU上报寄存器2 0x380 */
typedef union {
    struct {
        unsigned int soc_radm_trgt1_atu_cbuf_err2 : 16; /* 0:15 */
        unsigned int soc_radm_trgt1_atu_sloc_match2 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_radm_trgt1_atu_rpt2;
#define PCIE_CTRL_RB_PCIE_RADM_TRGT1_ATU_RPT2_OFF    0x380
#define PCIE_CTRL_RB_PCIE_RADM_TRGT1_ATU_RPT2_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_RADM_TRGT1_ATU_RPT2_OFF)

/*  0x3A0 */
typedef union {
    struct {
        unsigned int host_tx_ch_dr_empty_intr_mask : 4; /* 0:3 */
        unsigned int host_rx_ch_dr_empty_intr_mask : 6; /* 4:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctrl_ete_ch_dr_empty_intr_mask;
#define PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_MASK_OFF    0x3A0
#define PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_MASK_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_MASK_OFF)

/*  0x3A4 */
typedef union {
    struct {
        unsigned int host_tx_ch_dr_empty_intr_clr : 4; /* 0:3 */
        unsigned int host_rx_ch_dr_empty_intr_clr : 6; /* 4:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctrl_ete_ch_dr_empty_intr_clr;
#define PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_CLR_OFF    0x3A4
#define PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_CLR_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_CLR_OFF)

/*  0x3A8 */
typedef union {
    struct {
        unsigned int host_tx_ch_dr_empty_intr_status : 4; /* 0:3 */
        unsigned int host_rx_ch_dr_empty_intr_status : 6; /* 4:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctrl_ete_ch_dr_empty_intr_sts;
#define PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_STS_OFF    0x3A8
#define PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_STS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_STS_OFF)

/*  0x3AC */
typedef union {
    struct {
        unsigned int host_tx_ch_dr_empty_intr_raw_status : 4; /* 0:3 */
        unsigned int host_rx_ch_dr_empty_intr_raw_status : 6; /* 4:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctrl_ete_ch_dr_empty_intr_raw_sts;
#define PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_RAW_STS_OFF    0x3AC
#define PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_RAW_STS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_RAW_STS_OFF)

/*  0x3B0 */
typedef union {
    struct {
        unsigned int soc_stat_cnt_ltssm_enter_rcvry : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_stat_cnt_rcvry;
#define PCIE_CTRL_RB_PCIE_STAT_CNT_RCVRY_OFF    0x3B0
#define PCIE_CTRL_RB_PCIE_STAT_CNT_RCVRY_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STAT_CNT_RCVRY_OFF)

/*  0x3B4 */
typedef union {
    struct {
        unsigned int soc_stat_cnt_l1_enter_rcvry : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_stat_cnt_l1_rcvry;
#define PCIE_CTRL_RB_PCIE_STAT_CNT_L1_RCVRY_OFF    0x3B4
#define PCIE_CTRL_RB_PCIE_STAT_CNT_L1_RCVRY_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STAT_CNT_L1_RCVRY_OFF)

/*  0x3B8 */
typedef union {
    struct {
        unsigned int soc_pcie_stat_monitor_s1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_stat_monitor1;
#define PCIE_CTRL_RB_PCIE_STAT_MONITOR1_OFF    0x3B8
#define PCIE_CTRL_RB_PCIE_STAT_MONITOR1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STAT_MONITOR1_OFF)

/*  0x3BC */
typedef union {
    struct {
        unsigned int soc_pcie_stat_monitor_s2 : 4; /* 0:3 */
        unsigned int reserved0 : 8; /* 4:11 */
        unsigned int soc_ltssm_stat_samp_en : 1; /* 12 */
        unsigned int soc_ltssm_stat_flag_clr : 1; /* 13 */
        unsigned int reserved1 : 2; /* 14:15 */
        unsigned int soc_ltssm_stat_flag : 1; /* 16 */
        unsigned int reserved2 : 3; /* 17:19 */
        unsigned int soc_stat_cnt_ltssm_enter_rcvry_clr : 1; /* 20 */
        unsigned int soc_stat_cnt_l1_enter_rcvry_clr : 1; /* 21 */
        unsigned int reserved3 : 10; /* 22:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_stat_monitor2;
#define PCIE_CTRL_RB_PCIE_STAT_MONITOR2_OFF    0x3BC
#define PCIE_CTRL_RB_PCIE_STAT_MONITOR2_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STAT_MONITOR2_OFF)

/*  0x3C0 */
typedef union {
    struct {
        unsigned int soc_xfer_pending_timer : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_xfer_pending_timer;
#define PCIE_CTRL_RB_PCIE_XFER_PENDING_TIMER_OFF    0x3C0
#define PCIE_CTRL_RB_PCIE_XFER_PENDING_TIMER_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_XFER_PENDING_TIMER_OFF)

/*  0x3C4 */
typedef union {
    struct {
        unsigned int soc_xfer_pending_timer_en : 1; /* 0 */
        unsigned int reserved0 : 3; /* 1:3 */
        unsigned int soc_radm_cpl_timeout_en : 1; /* 4 */
        unsigned int reserved1 : 27; /* 5:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_xfer_pending_ctrl;
#define PCIE_CTRL_RB_PCIE_XFER_PENDING_CTRL_OFF    0x3C4
#define PCIE_CTRL_RB_PCIE_XFER_PENDING_CTRL_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_XFER_PENDING_CTRL_OFF)

/* PCIE PHY配置寄存器1 0x400 */
typedef union {
    struct {
        unsigned int pcie_pcs_tx_deemph_gen2_3p5db : 6; /* 0:5 */
        unsigned int reserved0 : 26; /* 6:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_config1;
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG1_OFF    0x400
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_CONFIG1_OFF)

/* PCIE PHY配置寄存器2 0x404 */
typedef union {
    struct {
        unsigned int pcie_pcs_tx_swing_low : 7; /* 0:6 */
        unsigned int reserved0 : 25; /* 7:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_config2;
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG2_OFF    0x404
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG2_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_CONFIG2_OFF)

/* PCIE PHY配置寄存器3 0x408 */
typedef union {
    struct {
        unsigned int pcie_pcs_tx_deemph_gen2_6db : 6; /* 0:5 */
        unsigned int reserved0 : 26; /* 6:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_config3;
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG3_OFF    0x408
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG3_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_CONFIG3_OFF)

/* PCIE PHY配置寄存器4 0x40C */
typedef union {
    struct {
        unsigned int pcie_pcs_tx_deemph_gen1 : 6; /* 0:5 */
        unsigned int reserved0 : 26; /* 6:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_config4;
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG4_OFF    0x40C
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG4_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_CONFIG4_OFF)

/* PCIE PHY配置寄存器5 0x410 */
typedef union {
    struct {
        unsigned int pcie_pcs_tx_swing_full : 7; /* 0:6 */
        unsigned int pcie_phy_ref_clkdiv2 : 1; /* 7 */
        unsigned int pcie_vreg_bypass : 1; /* 8 */
        unsigned int reserved0 : 23; /* 9:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_config5;
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG5_OFF    0x410
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG5_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_CONFIG5_OFF)

/* PCIE PHY配置寄存器6 0x414 */
typedef union {
    struct {
        unsigned int pcie_pcs_common_clocks : 1; /* 0 */
        unsigned int pcie_phy_tx0_term_offset : 5; /* 1:5 */
        unsigned int clkreq_oen : 1; /* 6 */
        unsigned int pcie_phy_mpll_multiplier : 7; /* 7:13 */
        unsigned int reserved0 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_config6;
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG6_OFF    0x414
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG6_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_CONFIG6_OFF)

/* PCIE PHY配置寄存器7 0x418 */
typedef union {
    struct {
        unsigned int pcie_phy_los_bias : 3; /* 0:2 */
        unsigned int pcie_phy_los_level : 5; /* 3:7 */
        unsigned int pcie_phy_rx0_eq : 3; /* 8:10 */
        unsigned int pcie_phy_tx_vboost_lvl : 3; /* 11:13 */
        unsigned int reserved0 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_config7;
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG7_OFF    0x418
#define PCIE_CTRL_RB_PCIE_PHY_CONFIG7_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_CONFIG7_OFF)

/* PCIE PHY配置地址寄存器 0x41C */
typedef union {
    struct {
        unsigned int phy_cr_addr : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_cfg_addr;
#define PCIE_CTRL_RB_PCIE_PHY_CFG_ADDR_OFF    0x41C
#define PCIE_CTRL_RB_PCIE_PHY_CFG_ADDR_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_CFG_ADDR_OFF)

/* PCIE PHY数据寄存器 0x420 */
typedef union {
    struct {
        unsigned int phy_cr_wr_data : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_wr_data;
#define PCIE_CTRL_RB_PCIE_PHY_WR_DATA_OFF    0x420
#define PCIE_CTRL_RB_PCIE_PHY_WR_DATA_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_WR_DATA_OFF)

/* PCIE PHY数据寄存器 0x424 */
typedef union {
    struct {
        unsigned int phy_cr_rd_data : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_rd_data;
#define PCIE_CTRL_RB_PCIE_PHY_RD_DATA_OFF    0x424
#define PCIE_CTRL_RB_PCIE_PHY_RD_DATA_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_RD_DATA_OFF)

/* PCIE PHY配置控制寄存器 0x428 */
typedef union {
    struct {
        unsigned int phy_config_en : 1; /* 0 */
        unsigned int reserved0 : 1; /* 1 */
        unsigned int phy_write : 1; /* 2 */
        unsigned int reserved1 : 12; /* 3:14 */
        unsigned int phy_config_rdy : 1; /* 15 */
        unsigned int reserved2 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_cfg1;
#define PCIE_CTRL_RB_PCIE_PHY_CFG1_OFF    0x428
#define PCIE_CTRL_RB_PCIE_PHY_CFG1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_CFG1_OFF)

/* PCIE PCS EN 配置 0x42C */
typedef union {
    struct {
        unsigned int soc_phy_txdetectrx_loopback_en0 : 1; /* 0 */
        unsigned int soc_phy_txdetectrx_loopback_en1 : 1; /* 1 */
        unsigned int reserved0 : 2; /* 2:3 */
        unsigned int soc_phy_txdetectrx_loopback_sel : 1; /* 4 */
        unsigned int reserved1 : 27; /* 5:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_en_cfg;
#define PCIE_CTRL_RB_PCIE_EN_CFG_OFF    0x42C
#define PCIE_CTRL_RB_PCIE_EN_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_EN_CFG_OFF)

/* PHY配置寄存器 0x430 */
typedef union {
    struct {
        unsigned int soc_mac_phy_txdeemph0 : 18; /* 0:17 */
        unsigned int reserved0 : 14; /* 18:31 */
    } bits;
    unsigned int as_dword;
} hreg_phy_cfg1;
#define PCIE_CTRL_RB_PHY_CFG1_OFF    0x430
#define PCIE_CTRL_RB_PHY_CFG1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PHY_CFG1_OFF)

/* PHY配置寄存器 0x434 */
typedef union {
    struct {
        unsigned int soc_mac_phy_txmargin : 3; /* 0:2 */
        unsigned int soc_mac_phy_txmargin_sel : 1; /* 3 */
        unsigned int soc_mac_phy_txswing : 1; /* 4 */
        unsigned int soc_mac_phy_txswing_sel : 1; /* 5 */
        unsigned int reserved0 : 2; /* 6:7 */
        unsigned int soc_mac_phy_txdeemph_sel : 1; /* 8 */
        unsigned int reserved1 : 3; /* 9:11 */
        unsigned int soc_mac_phy_txdeemph1 : 18; /* 12:29 */
        unsigned int reserved2 : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_phy_cfg2;
#define PCIE_CTRL_RB_PHY_CFG2_OFF    0x434
#define PCIE_CTRL_RB_PHY_CFG2_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PHY_CFG2_OFF)

/* pcie维测选择寄存器 0x438 */
typedef union {
    struct {
        unsigned int pcie_diag_sel : 4; /* 0:3 */
        unsigned int reserved0 : 28; /* 4:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_diag_sel;
#define PCIE_CTRL_RB_PCIE_DIAG_SEL_OFF    0x438
#define PCIE_CTRL_RB_PCIE_DIAG_SEL_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_DIAG_SEL_OFF)

/* pcie维测寄存器2 0x43C */
typedef union {
    struct {
        unsigned int pcie_mac_phy_powerdown : 4; /* 0:3 */
        unsigned int pcie_phy_mac_rxelecidle : 2; /* 4:5 */
        unsigned int reserved0 : 2; /* 6:7 */
        unsigned int pcie_phy_mac_rxstatus : 6; /* 8:13 */
        unsigned int reserved1 : 2; /* 14:15 */
        unsigned int pcie_mac_phy_txelecidle : 2; /* 16:17 */
        unsigned int reserved2 : 2; /* 18:19 */
        unsigned int pcie_mac_phy_rate : 2; /* 20:21 */
        unsigned int reserved3 : 2; /* 22:23 */
        unsigned int pcie_phy_mac_phystatus : 2; /* 24:25 */
        unsigned int reserved4 : 2; /* 26:27 */
        unsigned int pcie_mac_phy_clk_req_n : 1; /* 28 */
        unsigned int pcie_phy_mac_pclkack_n : 1; /* 29 */
        unsigned int reserved5 : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_status2;
#define PCIE_CTRL_RB_PCIE_STATUS2_OFF    0x43C
#define PCIE_CTRL_RB_PCIE_STATUS2_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STATUS2_OFF)

/* pcie维测寄存器3 0x440 */
typedef union {
    struct {
        unsigned int pcie_pipe0_rx_elecidle_disable : 1; /* 0 */
        unsigned int pcie_pipe0_tx_common_mode_disable : 1; /* 1 */
        unsigned int pcie_mac0_pclkreq_n : 2; /* 2:3 */
        unsigned int pcie_aux_clk_active : 1; /* 4 */
        unsigned int pcie_mstr_axi_clk_active : 1; /* 5 */
        unsigned int pcie_slv_axi_clk_active : 1; /* 6 */
        unsigned int reserved0 : 25; /* 7:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_status3;
#define PCIE_CTRL_RB_PCIE_STATUS3_OFF    0x440
#define PCIE_CTRL_RB_PCIE_STATUS3_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STATUS3_OFF)

/*  0x44C */
typedef union {
    struct {
        unsigned int soc_cfg_pcie_cap_int_msg_num : 5; /* 0:4 */
        unsigned int reserved0 : 3; /* 5:7 */
        unsigned int soc_smlh_ltssm_state_rcvry_eq : 1; /* 8 */
        unsigned int soc_pm_master_state : 5; /* 9:13 */
        unsigned int soc_pm_slave_state : 5; /* 14:18 */
        unsigned int soc_pm_l1sub_state : 3; /* 19:21 */
        unsigned int soc_rtlh_rfc_upd : 1; /* 22 */
        unsigned int soc_cfg_link_eq_req_int : 1; /* 23 */
        unsigned int soc_cfg_f_err_rpt_en : 1; /* 24 */
        unsigned int soc_cfg_nf_err_rpt_en : 1; /* 25 */
        unsigned int soc_cfg_cor_err_rpt_en : 1; /* 26 */
        unsigned int soc_cfg_reg_serren : 1; /* 27 */
        unsigned int soc_cfg_br_ctrl_serren : 1; /* 28 */
        unsigned int soc_pm_l1_entry_started : 1; /* 29 */
        unsigned int reserved1 : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_status4;
#define PCIE_CTRL_RB_PCIE_STATUS4_OFF    0x44C
#define PCIE_CTRL_RB_PCIE_STATUS4_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STATUS4_OFF)

/* pcie维测寄存器10 0x45C */
typedef union {
    struct {
        unsigned int pcie_slv_bmisc_info : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_status10;
#define PCIE_CTRL_RB_PCIE_STATUS10_OFF    0x45C
#define PCIE_CTRL_RB_PCIE_STATUS10_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STATUS10_OFF)

/* pcie维测寄存器11 0x460 */
typedef union {
    struct {
        unsigned int pcie_slv_rmisc_info : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_status11;
#define PCIE_CTRL_RB_PCIE_STATUS11_OFF    0x460
#define PCIE_CTRL_RB_PCIE_STATUS11_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STATUS11_OFF)

/* pcie维测寄存器12 0x464 */
typedef union {
    struct {
        unsigned int pcie_phy_mpll_en : 1; /* 0 */
        unsigned int pcie_phy_tx0_state : 1; /* 1 */
        unsigned int pcie_phy_ref_ssp_en : 1; /* 2 */
        unsigned int pcie_phy_rx0_pll_state : 1; /* 3 */
        unsigned int pcie_phy_mpll_state : 1; /* 4 */
        unsigned int pcie_phy_mpll_word_clk_en : 1; /* 5 */
        unsigned int pcie_phy_mpll_dword_clk_en : 1; /* 6 */
        unsigned int pcie_phy_mpll_qword_clk_en : 1; /* 7 */
        unsigned int pcie_phy_rx0_pll_en : 1; /* 8 */
        unsigned int reserved0 : 23; /* 9:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_status12;
#define PCIE_CTRL_RB_PCIE_STATUS12_OFF    0x464
#define PCIE_CTRL_RB_PCIE_STATUS12_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STATUS12_OFF)

/* pcie维测寄存器13 0x468 */
typedef union {
    struct {
        unsigned int cxpl_debug_info0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_status13;
#define PCIE_CTRL_RB_PCIE_STATUS13_OFF    0x468
#define PCIE_CTRL_RB_PCIE_STATUS13_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STATUS13_OFF)

/* pcie维测寄存器14 0x46C */
typedef union {
    struct {
        unsigned int cxpl_debug_info1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_status14;
#define PCIE_CTRL_RB_PCIE_STATUS14_OFF    0x46C
#define PCIE_CTRL_RB_PCIE_STATUS14_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STATUS14_OFF)

/* pcie维测寄存器15 0x470 */
typedef union {
    struct {
        unsigned int cxpl_debug_info_ei : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_status15;
#define PCIE_CTRL_RB_PCIE_STATUS15_OFF    0x470
#define PCIE_CTRL_RB_PCIE_STATUS15_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_STATUS15_OFF)

/* pcie tx abort消息上报 0x47C */
typedef union {
    struct {
        unsigned int tx_app_wr_err : 2; /* 0:1 */
        unsigned int reserved0 : 2; /* 2:3 */
        unsigned int tx_ll_fetch_err : 2; /* 4:5 */
        unsigned int reserved1 : 2; /* 6:7 */
        unsigned int tx_unsupport_err : 2; /* 8:9 */
        unsigned int reserved2 : 2; /* 10:11 */
        unsigned int tx_cpl_abort : 2; /* 12:13 */
        unsigned int reserved3 : 2; /* 14:15 */
        unsigned int tx_cpl_timeout : 2; /* 16:17 */
        unsigned int reserved4 : 2; /* 18:19 */
        unsigned int tx_data_poision : 2; /* 20:21 */
        unsigned int reserved5 : 2; /* 22:23 */
        unsigned int reserved6 : 8; /* 24:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_tx_abort_status;
#define PCIE_CTRL_RB_PCIE_TX_ABORT_STATUS_OFF    0x47C
#define PCIE_CTRL_RB_PCIE_TX_ABORT_STATUS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_TX_ABORT_STATUS_OFF)

/* pcie rx abort消息上报 0x480 */
typedef union {
    struct {
        unsigned int rx_app_wr_err : 2; /* 0:1 */
        unsigned int reserved0 : 2; /* 2:3 */
        unsigned int rx_ll_fetch_err : 2; /* 4:5 */
        unsigned int reserved1 : 2; /* 6:7 */
        unsigned int reserved2 : 24; /* 8:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_rx_abort_status;
#define PCIE_CTRL_RB_PCIE_RX_ABORT_STATUS_OFF    0x480
#define PCIE_CTRL_RB_PCIE_RX_ABORT_STATUS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_RX_ABORT_STATUS_OFF)

/*  0x484 */
typedef union {
    struct {
        unsigned int soc_phy_clk_req_n_sel : 1; /* 0 */
        unsigned int reserved0 : 3; /* 1:3 */
        unsigned int soc_pipe_lane_pclk_sel : 1; /* 4 */
        unsigned int reserved1 : 3; /* 5:7 */
        unsigned int soc_pipe_lane_tx2rx_loopbk : 2; /* 8:9 */
        unsigned int reserved2 : 2; /* 10:11 */
        unsigned int soc_pipe_rx_termination : 2; /* 12:13 */
        unsigned int reserved3 : 2; /* 14:15 */
        unsigned int soc_pipe_rx_cmn_refclk_mode : 2; /* 16:17 */
        unsigned int reserved4 : 14; /* 18:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_clk_sel_cfg;
#define PCIE_CTRL_RB_PCIE_PHY_CLK_SEL_CFG_OFF    0x484
#define PCIE_CTRL_RB_PCIE_PHY_CLK_SEL_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_CLK_SEL_CFG_OFF)

/* TX SWING手动使能 0x500 */
typedef union {
    struct {
        unsigned int pcs_tx_swing_sel_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcs_tx_swing_sel_en;
#define PCIE_CTRL_RB_PCS_TX_SWING_SEL_EN_OFF    0x500
#define PCIE_CTRL_RB_PCS_TX_SWING_SEL_EN_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCS_TX_SWING_SEL_EN_OFF)

/* DBI选择 0x504 */
typedef union {
    struct {
        unsigned int dbi_elbi_sel : 1; /* 0 */
        unsigned int dbi_ro_sel : 1; /* 1 */
        unsigned int dbi_dma_sel : 1; /* 2 */
        unsigned int dbi_ro_disable : 1; /* 3 */
        unsigned int reserved0 : 28; /* 4:31 */
    } bits;
    unsigned int as_dword;
} hreg_dbi_elbi_sel;
#define PCIE_CTRL_RB_DBI_ELBI_SEL_OFF    0x504
#define PCIE_CTRL_RB_DBI_ELBI_SEL_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_DBI_ELBI_SEL_OFF)

/*  0x508 */
typedef union {
    struct {
        unsigned int host_tx_err_intr_mask : 5; /* 0:4 */
        unsigned int reserved0 : 3; /* 5:7 */
        unsigned int host_tx_done_intr_mask : 5; /* 8:12 */
        unsigned int reserved1 : 3; /* 13:15 */
        unsigned int host_rx_err_intr_mask : 6; /* 16:21 */
        unsigned int reserved2 : 2; /* 22:23 */
        unsigned int host_rx_done_intr_mask : 6; /* 24:29 */
        unsigned int reserved3 : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctl_ete_intr_mask;
#define PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_MASK_OFF    0x508
#define PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_MASK_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_MASK_OFF)

/*  0x50C */
typedef union {
    struct {
        unsigned int host_tx_err_intr_clr : 5; /* 0:4 */
        unsigned int reserved0 : 3; /* 5:7 */
        unsigned int host_tx_done_intr_clr : 5; /* 8:12 */
        unsigned int reserved1 : 3; /* 13:15 */
        unsigned int host_rx_err_intr_clr : 6; /* 16:21 */
        unsigned int reserved2 : 2; /* 22:23 */
        unsigned int host_rx_done_intr_clr : 6; /* 24:29 */
        unsigned int reserved3 : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctl_ete_intr_clr;
#define PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_CLR_OFF    0x50C
#define PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_CLR_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_CLR_OFF)

/*  0x510 */
typedef union {
    struct {
        unsigned int host_tx_err_intr_status : 5; /* 0:4 */
        unsigned int reserved0 : 3; /* 5:7 */
        unsigned int host_tx_done_intr_status : 5; /* 8:12 */
        unsigned int reserved1 : 3; /* 13:15 */
        unsigned int host_rx_err_intr_status : 6; /* 16:21 */
        unsigned int reserved2 : 2; /* 22:23 */
        unsigned int host_rx_done_intr_status : 6; /* 24:29 */
        unsigned int reserved3 : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctl_ete_intr_sts;
#define PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_STS_OFF    0x510
#define PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_STS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_STS_OFF)

/*  0x514 */
typedef union {
    struct {
        unsigned int host_tx_err_intr_raw_status : 5; /* 0:4 */
        unsigned int reserved0 : 3; /* 5:7 */
        unsigned int host_tx_done_intr_raw_status : 5; /* 8:12 */
        unsigned int reserved1 : 3; /* 13:15 */
        unsigned int host_rx_err_intr_raw_status : 6; /* 16:21 */
        unsigned int reserved2 : 2; /* 22:23 */
        unsigned int host_rx_done_intr_raw_status : 6; /* 24:29 */
        unsigned int reserved3 : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_ctl_ete_intr_raw_sts;
#define PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_RAW_STS_OFF    0x514
#define PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_RAW_STS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CTL_ETE_INTR_RAW_STS_OFF)

/* TX中断上报延迟 0x528 */
typedef union {
    struct {
        unsigned int rx_done_intr_delay_cnt : 16; /* 0:15 */
        unsigned int rx_intr_delay_sel : 1; /* 16 */
        unsigned int reserved0 : 15; /* 17:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_host_int_delay;
#define PCIE_CTRL_RB_RX_HOST_INT_DELAY_OFF    0x528
#define PCIE_CTRL_RB_RX_HOST_INT_DELAY_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_RX_HOST_INT_DELAY_OFF)

/* TX中断上报延迟 0x52C */
typedef union {
    struct {
        unsigned int tx_done_intr_delay_cnt : 16; /* 0:15 */
        unsigned int tx_intr_delay_sel : 1; /* 16 */
        unsigned int reserved0 : 15; /* 17:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_host_int_delay;
#define PCIE_CTRL_RB_TX_HOST_INT_DELAY_OFF    0x52C
#define PCIE_CTRL_RB_TX_HOST_INT_DELAY_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_TX_HOST_INT_DELAY_OFF)

/* PCIE_CLK_GATE使能 0x530 */
typedef union {
    struct {
        unsigned int soc_pcie_clk_gate_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_clk_gate_en;
#define PCIE_CTRL_RB_PCIE_CLK_GATE_EN_OFF    0x530
#define PCIE_CTRL_RB_PCIE_CLK_GATE_EN_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CLK_GATE_EN_OFF)

/*  0x534 */
typedef union {
    struct {
        unsigned int pcie_max_awaddr0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_max_awaddr_cfg00;
#define PCIE_CTRL_RB_PCIE_MAX_AWADDR_CFG00_OFF    0x534
#define PCIE_CTRL_RB_PCIE_MAX_AWADDR_CFG00_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MAX_AWADDR_CFG00_OFF)

/*  0x538 */
typedef union {
    struct {
        unsigned int pcie_max_awaddr1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_max_awaddr_cfg01;
#define PCIE_CTRL_RB_PCIE_MAX_AWADDR_CFG01_OFF    0x538
#define PCIE_CTRL_RB_PCIE_MAX_AWADDR_CFG01_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MAX_AWADDR_CFG01_OFF)

/*  0x53C */
typedef union {
    struct {
        unsigned int pcie_min_awaddr0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_min_awaddr_cfg00;
#define PCIE_CTRL_RB_PCIE_MIN_AWADDR_CFG00_OFF    0x53C
#define PCIE_CTRL_RB_PCIE_MIN_AWADDR_CFG00_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MIN_AWADDR_CFG00_OFF)

/*  0x540 */
typedef union {
    struct {
        unsigned int pcie_min_awaddr1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_min_awaddr_cfg01;
#define PCIE_CTRL_RB_PCIE_MIN_AWADDR_CFG01_OFF    0x540
#define PCIE_CTRL_RB_PCIE_MIN_AWADDR_CFG01_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MIN_AWADDR_CFG01_OFF)

/*  0x544 */
typedef union {
    struct {
        unsigned int pcie_max_araddr0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_max_araddr_cfg00;
#define PCIE_CTRL_RB_PCIE_MAX_ARADDR_CFG00_OFF    0x544
#define PCIE_CTRL_RB_PCIE_MAX_ARADDR_CFG00_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MAX_ARADDR_CFG00_OFF)

/*  0x548 */
typedef union {
    struct {
        unsigned int pcie_max_araddr1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_max_araddr_cfg01;
#define PCIE_CTRL_RB_PCIE_MAX_ARADDR_CFG01_OFF    0x548
#define PCIE_CTRL_RB_PCIE_MAX_ARADDR_CFG01_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MAX_ARADDR_CFG01_OFF)

/*  0x54C */
typedef union {
    struct {
        unsigned int pcie_min_araddr0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_min_araddr_cfg00;
#define PCIE_CTRL_RB_PCIE_MIN_ARADDR_CFG00_OFF    0x54C
#define PCIE_CTRL_RB_PCIE_MIN_ARADDR_CFG00_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MIN_ARADDR_CFG00_OFF)

/*  0x550 */
typedef union {
    struct {
        unsigned int pcie_min_araddr1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_min_araddr_cfg01;
#define PCIE_CTRL_RB_PCIE_MIN_ARADDR_CFG01_OFF    0x550
#define PCIE_CTRL_RB_PCIE_MIN_ARADDR_CFG01_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_MIN_ARADDR_CFG01_OFF)

/*  0x554 */
typedef union {
    struct {
        unsigned int pcie_awaddr_sts0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_lmi_awaddr_sts0;
#define PCIE_CTRL_RB_PCIE_LMI_AWADDR_STS0_OFF    0x554
#define PCIE_CTRL_RB_PCIE_LMI_AWADDR_STS0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_LMI_AWADDR_STS0_OFF)

/*  0x558 */
typedef union {
    struct {
        unsigned int pcie_awaddr_sts1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_lmi_awaddr_sts1;
#define PCIE_CTRL_RB_PCIE_LMI_AWADDR_STS1_OFF    0x558
#define PCIE_CTRL_RB_PCIE_LMI_AWADDR_STS1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_LMI_AWADDR_STS1_OFF)

/*  0x55C */
typedef union {
    struct {
        unsigned int pcie_araddr_sts0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_lmi_araddr_sts0;
#define PCIE_CTRL_RB_PCIE_LMI_ARADDR_STS0_OFF    0x55C
#define PCIE_CTRL_RB_PCIE_LMI_ARADDR_STS0_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_LMI_ARADDR_STS0_OFF)

/*  0x560 */
typedef union {
    struct {
        unsigned int pcie_araddr_sts1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_lmi_araddr_sts1;
#define PCIE_CTRL_RB_PCIE_LMI_ARADDR_STS1_OFF    0x560
#define PCIE_CTRL_RB_PCIE_LMI_ARADDR_STS1_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_LMI_ARADDR_STS1_OFF)

/*  0x564 */
typedef union {
    struct {
        unsigned int pcie_awchan_valid : 1; /* 0 */
        unsigned int pcie_archan_valid : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_cfg_sts;
#define PCIE_CTRL_RB_PCIE_CFG_STS_OFF    0x564
#define PCIE_CTRL_RB_PCIE_CFG_STS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CFG_STS_OFF)

/*  0x568 */
typedef union {
    struct {
        unsigned int pcie_mstr_clr : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_cfg_clr;
#define PCIE_CTRL_RB_PCIE_CFG_CLR_OFF    0x568
#define PCIE_CTRL_RB_PCIE_CFG_CLR_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_CFG_CLR_OFF)

/*  0x56C */
typedef union {
    struct {
        unsigned int bus_monitor_enable : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_bus_monitor_enable;
#define PCIE_CTRL_RB_BUS_MONITOR_ENABLE_OFF    0x56C
#define PCIE_CTRL_RB_BUS_MONITOR_ENABLE_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_BUS_MONITOR_ENABLE_OFF)

/*  0x570 */
typedef union {
    struct {
        unsigned int wr_cnt : 6; /* 0:5 */
        unsigned int reserved0 : 2; /* 6:7 */
        unsigned int rd_cnt : 6; /* 8:13 */
        unsigned int reserved1 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_axi_cnt_rpt;
#define PCIE_CTRL_RB_HOST_AXI_CNT_RPT_OFF    0x570
#define PCIE_CTRL_RB_HOST_AXI_CNT_RPT_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_HOST_AXI_CNT_RPT_OFF)

/*  0x5A8 */
typedef union {
    struct {
        unsigned int soc_dbi_protect_en : 1; /* 0 */
        unsigned int reserved0 : 3; /* 1:3 */
        unsigned int soc_dbi_protect_status : 1; /* 4 */
        unsigned int reserved1 : 27; /* 5:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_refclk_dbi_prot;
#define PCIE_CTRL_RB_PCIE_REFCLK_DBI_PROT_OFF    0x5A8
#define PCIE_CTRL_RB_PCIE_REFCLK_DBI_PROT_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_REFCLK_DBI_PROT_OFF)

/*  0x600 */
typedef union {
    struct {
        unsigned int soc_phy_por_n : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_poweron_cfg;
#define PCIE_CTRL_RB_PCIE_PHY_POWERON_CFG_OFF    0x600
#define PCIE_CTRL_RB_PCIE_PHY_POWERON_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_POWERON_CFG_OFF)

/*  0x604 */
typedef union {
    struct {
        unsigned int soc_phy_reset_n : 1; /* 0 */
        unsigned int reserved0 : 3; /* 1:3 */
        unsigned int soc_phy_reset_sel : 1; /* 4 */
        unsigned int reserved1 : 27; /* 5:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_reset_cfg;
#define PCIE_CTRL_RB_PCIE_PHY_RESET_CFG_OFF    0x604
#define PCIE_CTRL_RB_PCIE_PHY_RESET_CFG_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_RESET_CFG_OFF)

/*  0x608 */
typedef union {
    struct {
        unsigned int soc_sym_align_en : 1; /* 0 */
        unsigned int soc_sym_align_en_sel : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_sym_align_en_ctl;
#define PCIE_CTRL_RB_PCIE_SYM_ALIGN_EN_CTL_OFF    0x608
#define PCIE_CTRL_RB_PCIE_SYM_ALIGN_EN_CTL_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_SYM_ALIGN_EN_CTL_OFF)

/*  0x610 */
typedef union {
    struct {
        unsigned int soc_jtag_protect : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_jtag_mode_protect;
#define PCIE_CTRL_RB_PCIE_JTAG_MODE_PROTECT_OFF    0x610
#define PCIE_CTRL_RB_PCIE_JTAG_MODE_PROTECT_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_JTAG_MODE_PROTECT_OFF)

/*  0x614 */
typedef union {
    struct {
        unsigned int soc_pcie_phy_reset_n : 1; /* 0 */
        unsigned int reserved0 : 3; /* 1:3 */
        unsigned int soc_pcie_phy_reset_bypass : 1; /* 4 */
        unsigned int reserved1 : 27; /* 5:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_reset_bypass;
#define PCIE_CTRL_RB_PCIE_PHY_RESET_BYPASS_OFF    0x614
#define PCIE_CTRL_RB_PCIE_PHY_RESET_BYPASS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_RESET_BYPASS_OFF)

/*  0x618 */
typedef union {
    struct {
        unsigned int soc_pcie_phy_apb_reset_n : 1; /* 0 */
        unsigned int reserved0 : 3; /* 1:3 */
        unsigned int soc_pcie_phy_apb_reset_bypass : 1; /* 4 */
        unsigned int reserved1 : 27; /* 5:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_apb_reset_bypass;
#define PCIE_CTRL_RB_PCIE_APB_RESET_BYPASS_OFF    0x618
#define PCIE_CTRL_RB_PCIE_APB_RESET_BYPASS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_APB_RESET_BYPASS_OFF)

/*  0x61C */
typedef union {
    struct {
        unsigned int pcie_phy_por_n_status : 1; /* 0 */
        unsigned int pcie_phy_prst_n_status : 1; /* 1 */
        unsigned int pcie_phy_reset_n_status : 1; /* 2 */
        unsigned int reserved0 : 29; /* 3:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_phy_rst_n_status;
#define PCIE_CTRL_RB_PCIE_PHY_RST_N_STATUS_OFF    0x61C
#define PCIE_CTRL_RB_PCIE_PHY_RST_N_STATUS_REG \
    (PCIE_CTRL_RB_BASE + PCIE_CTRL_RB_PCIE_PHY_RST_N_STATUS_OFF)

#endif
