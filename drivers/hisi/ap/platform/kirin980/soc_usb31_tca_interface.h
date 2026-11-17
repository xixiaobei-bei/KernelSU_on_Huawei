#ifndef __SOC_USB31_TCA_INTERFACE_H__
#define __SOC_USB31_TCA_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_USB31_TCA_TCA_CLK_RST_ADDR(base) ((base) + (0x0000))
#define SOC_USB31_TCA_TCA_INTR_EN_ADDR(base) ((base) + (0x0004))
#define SOC_USB31_TCA_TCA_INTR_STS_ADDR(base) ((base) + (0x0008))
#define SOC_USB31_TCA_TCA_GCFG_ADDR(base) ((base) + (0x0010))
#define SOC_USB31_TCA_TCA_TCPC_ADDR(base) ((base) + (0x0014))
#define SOC_USB31_TCA_TCA_SYSMODE_CFG_ADDR(base) ((base) + (0x0018))
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_ADDR(base) ((base) + (0x0020))
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG1_ADDR(base) ((base) + (0x0024))
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_ADDR(base) ((base) + (0x0028))
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG1_ADDR(base) ((base) + (0x002C))
#define SOC_USB31_TCA_TCA_PSTATE_ADDR(base) ((base) + (0x0030))
#define SOC_USB31_TCA_TCA_GEN_STATUS_ADDR(base) ((base) + (0x0034))
#define SOC_USB31_TCA_TCA_VBUS_CTRL_ADDR(base) ((base) + (0x0040))
#define SOC_USB31_TCA_TCA_VBUS_STATUS_ADDR(base) ((base) + (0x0044))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int suspend_clk_en : 1;
        unsigned int tca_ref_clk_en : 1;
        unsigned int reserved_0 : 6;
        unsigned int phy_rst_sw : 1;
        unsigned int xa_rst_sw : 1;
        unsigned int vba_rst_sw : 1;
        unsigned int reserved_1 : 21;
    } reg;
} SOC_USB31_TCA_TCA_CLK_RST_UNION;
#endif
#define SOC_USB31_TCA_TCA_CLK_RST_suspend_clk_en_START (0)
#define SOC_USB31_TCA_TCA_CLK_RST_suspend_clk_en_END (0)
#define SOC_USB31_TCA_TCA_CLK_RST_tca_ref_clk_en_START (1)
#define SOC_USB31_TCA_TCA_CLK_RST_tca_ref_clk_en_END (1)
#define SOC_USB31_TCA_TCA_CLK_RST_phy_rst_sw_START (8)
#define SOC_USB31_TCA_TCA_CLK_RST_phy_rst_sw_END (8)
#define SOC_USB31_TCA_TCA_CLK_RST_xa_rst_sw_START (9)
#define SOC_USB31_TCA_TCA_CLK_RST_xa_rst_sw_END (9)
#define SOC_USB31_TCA_TCA_CLK_RST_vba_rst_sw_START (10)
#define SOC_USB31_TCA_TCA_CLK_RST_vba_rst_sw_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xa_ack_evt_en : 1;
        unsigned int xa_timeout_evt_en : 1;
        unsigned int reserved_0 : 6;
        unsigned int sys_vbusvalid_evt_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int tca_vbusvalid_evt_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int tca_drv_host_vbus_evt_en : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 18;
    } reg;
} SOC_USB31_TCA_TCA_INTR_EN_UNION;
#endif
#define SOC_USB31_TCA_TCA_INTR_EN_xa_ack_evt_en_START (0)
#define SOC_USB31_TCA_TCA_INTR_EN_xa_ack_evt_en_END (0)
#define SOC_USB31_TCA_TCA_INTR_EN_xa_timeout_evt_en_START (1)
#define SOC_USB31_TCA_TCA_INTR_EN_xa_timeout_evt_en_END (1)
#define SOC_USB31_TCA_TCA_INTR_EN_sys_vbusvalid_evt_en_START (8)
#define SOC_USB31_TCA_TCA_INTR_EN_sys_vbusvalid_evt_en_END (8)
#define SOC_USB31_TCA_TCA_INTR_EN_tca_vbusvalid_evt_en_START (10)
#define SOC_USB31_TCA_TCA_INTR_EN_tca_vbusvalid_evt_en_END (10)
#define SOC_USB31_TCA_TCA_INTR_EN_tca_drv_host_vbus_evt_en_START (12)
#define SOC_USB31_TCA_TCA_INTR_EN_tca_drv_host_vbus_evt_en_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xa_ack_evt : 1;
        unsigned int xa_timeout_evt : 1;
        unsigned int reserved_0 : 6;
        unsigned int sys_vbusvalid_evt : 1;
        unsigned int sys_vbusvalid : 1;
        unsigned int tca_vbusvalid_evt : 1;
        unsigned int tca_vbusvalid : 1;
        unsigned int tca_drv_host_vbus_evt : 1;
        unsigned int tca_drv_host_vbus : 1;
        unsigned int reserved_1 : 18;
    } reg;
} SOC_USB31_TCA_TCA_INTR_STS_UNION;
#endif
#define SOC_USB31_TCA_TCA_INTR_STS_xa_ack_evt_START (0)
#define SOC_USB31_TCA_TCA_INTR_STS_xa_ack_evt_END (0)
#define SOC_USB31_TCA_TCA_INTR_STS_xa_timeout_evt_START (1)
#define SOC_USB31_TCA_TCA_INTR_STS_xa_timeout_evt_END (1)
#define SOC_USB31_TCA_TCA_INTR_STS_sys_vbusvalid_evt_START (8)
#define SOC_USB31_TCA_TCA_INTR_STS_sys_vbusvalid_evt_END (8)
#define SOC_USB31_TCA_TCA_INTR_STS_sys_vbusvalid_START (9)
#define SOC_USB31_TCA_TCA_INTR_STS_sys_vbusvalid_END (9)
#define SOC_USB31_TCA_TCA_INTR_STS_tca_vbusvalid_evt_START (10)
#define SOC_USB31_TCA_TCA_INTR_STS_tca_vbusvalid_evt_END (10)
#define SOC_USB31_TCA_TCA_INTR_STS_tca_vbusvalid_START (11)
#define SOC_USB31_TCA_TCA_INTR_STS_tca_vbusvalid_END (11)
#define SOC_USB31_TCA_TCA_INTR_STS_tca_drv_host_vbus_evt_START (12)
#define SOC_USB31_TCA_TCA_INTR_STS_tca_drv_host_vbus_evt_END (12)
#define SOC_USB31_TCA_TCA_INTR_STS_tca_drv_host_vbus_START (13)
#define SOC_USB31_TCA_TCA_INTR_STS_tca_drv_host_vbus_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int op_mode : 2;
        unsigned int reserved_0 : 2;
        unsigned int role_hstdev : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_USB31_TCA_TCA_GCFG_UNION;
#endif
#define SOC_USB31_TCA_TCA_GCFG_op_mode_START (0)
#define SOC_USB31_TCA_TCA_GCFG_op_mode_END (1)
#define SOC_USB31_TCA_TCA_GCFG_role_hstdev_START (4)
#define SOC_USB31_TCA_TCA_GCFG_role_hstdev_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tcpc_mux_control : 2;
        unsigned int tcpc_connector_orientation : 1;
        unsigned int tcpc_low_power_en : 1;
        unsigned int tcpc_valid : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_USB31_TCA_TCA_TCPC_UNION;
#endif
#define SOC_USB31_TCA_TCA_TCPC_tcpc_mux_control_START (0)
#define SOC_USB31_TCA_TCA_TCPC_tcpc_mux_control_END (1)
#define SOC_USB31_TCA_TCA_TCPC_tcpc_connector_orientation_START (2)
#define SOC_USB31_TCA_TCA_TCPC_tcpc_connector_orientation_END (2)
#define SOC_USB31_TCA_TCA_TCPC_tcpc_low_power_en_START (3)
#define SOC_USB31_TCA_TCA_TCPC_tcpc_low_power_en_END (3)
#define SOC_USB31_TCA_TCA_TCPC_tcpc_valid_START (4)
#define SOC_USB31_TCA_TCA_TCPC_tcpc_valid_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int typec_conn_mode : 2;
        unsigned int typec_flip : 1;
        unsigned int typec_disable : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_USB31_TCA_TCA_SYSMODE_CFG_UNION;
#endif
#define SOC_USB31_TCA_TCA_SYSMODE_CFG_typec_conn_mode_START (0)
#define SOC_USB31_TCA_TCA_SYSMODE_CFG_typec_conn_mode_END (1)
#define SOC_USB31_TCA_TCA_SYSMODE_CFG_typec_flip_START (2)
#define SOC_USB31_TCA_TCA_SYSMODE_CFG_typec_flip_END (2)
#define SOC_USB31_TCA_TCA_SYSMODE_CFG_typec_disable_START (3)
#define SOC_USB31_TCA_TCA_SYSMODE_CFG_typec_disable_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int block_ss_op : 1;
        unsigned int auto_block_ss_op_en : 2;
        unsigned int reserved_0 : 5;
        unsigned int ss_hdshk_req : 1;
        unsigned int dp_hdshk_req : 1;
        unsigned int reserved_1 : 6;
        unsigned int auto_safe_state : 1;
        unsigned int reserved_2 : 15;
    } reg;
} SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_UNION;
#endif
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_block_ss_op_START (0)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_block_ss_op_END (0)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_auto_block_ss_op_en_START (1)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_auto_block_ss_op_en_END (2)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_ss_hdshk_req_START (8)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_ss_hdshk_req_END (8)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_dp_hdshk_req_START (9)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_dp_hdshk_req_END (9)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_auto_safe_state_START (16)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG0_auto_safe_state_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xa_timeout_val : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG1_UNION;
#endif
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG1_xa_timeout_val_START (0)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_CFG1_xa_timeout_val_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpalt_dp4 : 1;
        unsigned int dpalt_disable : 1;
        unsigned int dpalt_disable_ack : 1;
        unsigned int reserved_0 : 1;
        unsigned int ss_rxdetect_disable : 1;
        unsigned int ss_rxdetect_disable_ack : 1;
        unsigned int reserved_1 : 2;
        unsigned int block_ss_op : 1;
        unsigned int xbar_ready : 1;
        unsigned int reserved_2 : 3;
        unsigned int pstate_synced : 1;
        unsigned int pstate_timeout_val : 1;
        unsigned int xa_req_compl_mult : 5;
        unsigned int ctrl_if_ovrrd_en : 5;
        unsigned int ctrl_if_ovrrd_val : 5;
        unsigned int reserved_3 : 2;
    } reg;
} SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_UNION;
#endif
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_dpalt_dp4_START (0)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_dpalt_dp4_END (0)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_dpalt_disable_START (1)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_dpalt_disable_END (1)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_dpalt_disable_ack_START (2)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_dpalt_disable_ack_END (2)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_ss_rxdetect_disable_START (4)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_ss_rxdetect_disable_END (4)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_ss_rxdetect_disable_ack_START (5)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_ss_rxdetect_disable_ack_END (5)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_block_ss_op_START (8)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_block_ss_op_END (8)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_xbar_ready_START (9)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_xbar_ready_END (9)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_pstate_synced_START (13)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_pstate_synced_END (13)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_pstate_timeout_val_START (14)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_pstate_timeout_val_END (14)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_xa_req_compl_mult_START (15)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_xa_req_compl_mult_END (19)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_ctrl_if_ovrrd_en_START (20)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_ctrl_if_ovrrd_en_END (24)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_ctrl_if_ovrrd_val_START (25)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG0_ctrl_if_ovrrd_val_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xa_req_compl_cycles : 16;
        unsigned int xa_req_compl_cycles_overrun : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG1_UNION;
#endif
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG1_xa_req_compl_cycles_START (0)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG1_xa_req_compl_cycles_END (15)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG1_xa_req_compl_cycles_overrun_START (16)
#define SOC_USB31_TCA_TCA_CTRLSYNCMODE_DBG1_xa_req_compl_cycles_overrun_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ssrx_pstate : 2;
        unsigned int ssrx_req : 1;
        unsigned int ssrx_ack : 1;
        unsigned int sstx_pstate : 2;
        unsigned int sstx_req : 1;
        unsigned int sstx_ack : 1;
        unsigned int dp_tx0_pstate : 2;
        unsigned int dp_tx0_req : 1;
        unsigned int dp_tx0_ack : 1;
        unsigned int dp_tx1_pstate : 2;
        unsigned int dp_tx1_req : 1;
        unsigned int dp_tx1_ack : 1;
        unsigned int dp_tx2_pstate : 2;
        unsigned int dp_tx2_req : 1;
        unsigned int dp_tx2_ack : 1;
        unsigned int dp_tx3_pstate : 2;
        unsigned int dp_tx3_req : 1;
        unsigned int dp_tx3_ack : 1;
        unsigned int reserved : 8;
    } reg;
} SOC_USB31_TCA_TCA_PSTATE_UNION;
#endif
#define SOC_USB31_TCA_TCA_PSTATE_ssrx_pstate_START (0)
#define SOC_USB31_TCA_TCA_PSTATE_ssrx_pstate_END (1)
#define SOC_USB31_TCA_TCA_PSTATE_ssrx_req_START (2)
#define SOC_USB31_TCA_TCA_PSTATE_ssrx_req_END (2)
#define SOC_USB31_TCA_TCA_PSTATE_ssrx_ack_START (3)
#define SOC_USB31_TCA_TCA_PSTATE_ssrx_ack_END (3)
#define SOC_USB31_TCA_TCA_PSTATE_sstx_pstate_START (4)
#define SOC_USB31_TCA_TCA_PSTATE_sstx_pstate_END (5)
#define SOC_USB31_TCA_TCA_PSTATE_sstx_req_START (6)
#define SOC_USB31_TCA_TCA_PSTATE_sstx_req_END (6)
#define SOC_USB31_TCA_TCA_PSTATE_sstx_ack_START (7)
#define SOC_USB31_TCA_TCA_PSTATE_sstx_ack_END (7)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx0_pstate_START (8)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx0_pstate_END (9)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx0_req_START (10)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx0_req_END (10)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx0_ack_START (11)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx0_ack_END (11)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx1_pstate_START (12)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx1_pstate_END (13)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx1_req_START (14)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx1_req_END (14)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx1_ack_START (15)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx1_ack_END (15)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx2_pstate_START (16)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx2_pstate_END (17)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx2_req_START (18)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx2_req_END (18)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx2_ack_START (19)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx2_ack_END (19)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx3_pstate_START (20)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx3_pstate_END (21)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx3_req_START (22)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx3_req_END (22)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx3_ack_START (23)
#define SOC_USB31_TCA_TCA_PSTATE_dp_tx3_ack_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int phy_typec_conn_mode : 2;
        unsigned int phy_typec_flip : 1;
        unsigned int phy_typec_disable : 1;
        unsigned int typec_flip_invert : 1;
        unsigned int reserved_0 : 3;
        unsigned int ref_clk_sel : 1;
        unsigned int reserved_1 : 3;
        unsigned int usb_dev_por : 1;
        unsigned int dp4_por : 1;
        unsigned int reserved_2 : 18;
    } reg;
} SOC_USB31_TCA_TCA_GEN_STATUS_UNION;
#endif
#define SOC_USB31_TCA_TCA_GEN_STATUS_phy_typec_conn_mode_START (0)
#define SOC_USB31_TCA_TCA_GEN_STATUS_phy_typec_conn_mode_END (1)
#define SOC_USB31_TCA_TCA_GEN_STATUS_phy_typec_flip_START (2)
#define SOC_USB31_TCA_TCA_GEN_STATUS_phy_typec_flip_END (2)
#define SOC_USB31_TCA_TCA_GEN_STATUS_phy_typec_disable_START (3)
#define SOC_USB31_TCA_TCA_GEN_STATUS_phy_typec_disable_END (3)
#define SOC_USB31_TCA_TCA_GEN_STATUS_typec_flip_invert_START (4)
#define SOC_USB31_TCA_TCA_GEN_STATUS_typec_flip_invert_END (4)
#define SOC_USB31_TCA_TCA_GEN_STATUS_ref_clk_sel_START (8)
#define SOC_USB31_TCA_TCA_GEN_STATUS_ref_clk_sel_END (8)
#define SOC_USB31_TCA_TCA_GEN_STATUS_usb_dev_por_START (12)
#define SOC_USB31_TCA_TCA_GEN_STATUS_usb_dev_por_END (12)
#define SOC_USB31_TCA_TCA_GEN_STATUS_dp4_por_START (13)
#define SOC_USB31_TCA_TCA_GEN_STATUS_dp4_por_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vbusvalid_overrd : 2;
        unsigned int powerpresent_overrd : 2;
        unsigned int drv_host_vbus_overrd : 2;
        unsigned int reserved_0 : 2;
        unsigned int tca_iddif : 1;
        unsigned int tca_misc_ctrl : 6;
        unsigned int reserved_1 : 17;
    } reg;
} SOC_USB31_TCA_TCA_VBUS_CTRL_UNION;
#endif
#define SOC_USB31_TCA_TCA_VBUS_CTRL_vbusvalid_overrd_START (0)
#define SOC_USB31_TCA_TCA_VBUS_CTRL_vbusvalid_overrd_END (1)
#define SOC_USB31_TCA_TCA_VBUS_CTRL_powerpresent_overrd_START (2)
#define SOC_USB31_TCA_TCA_VBUS_CTRL_powerpresent_overrd_END (3)
#define SOC_USB31_TCA_TCA_VBUS_CTRL_drv_host_vbus_overrd_START (4)
#define SOC_USB31_TCA_TCA_VBUS_CTRL_drv_host_vbus_overrd_END (5)
#define SOC_USB31_TCA_TCA_VBUS_CTRL_tca_iddif_START (8)
#define SOC_USB31_TCA_TCA_VBUS_CTRL_tca_iddif_END (8)
#define SOC_USB31_TCA_TCA_VBUS_CTRL_tca_misc_ctrl_START (9)
#define SOC_USB31_TCA_TCA_VBUS_CTRL_tca_misc_ctrl_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sys_vbusvalid : 1;
        unsigned int tca_vbusvalid : 1;
        unsigned int tca_powerpresent : 1;
        unsigned int tca_drv_host_vbus : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_USB31_TCA_TCA_VBUS_STATUS_UNION;
#endif
#define SOC_USB31_TCA_TCA_VBUS_STATUS_sys_vbusvalid_START (0)
#define SOC_USB31_TCA_TCA_VBUS_STATUS_sys_vbusvalid_END (0)
#define SOC_USB31_TCA_TCA_VBUS_STATUS_tca_vbusvalid_START (1)
#define SOC_USB31_TCA_TCA_VBUS_STATUS_tca_vbusvalid_END (1)
#define SOC_USB31_TCA_TCA_VBUS_STATUS_tca_powerpresent_START (2)
#define SOC_USB31_TCA_TCA_VBUS_STATUS_tca_powerpresent_END (2)
#define SOC_USB31_TCA_TCA_VBUS_STATUS_tca_drv_host_vbus_START (3)
#define SOC_USB31_TCA_TCA_VBUS_STATUS_tca_drv_host_vbus_END (3)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
