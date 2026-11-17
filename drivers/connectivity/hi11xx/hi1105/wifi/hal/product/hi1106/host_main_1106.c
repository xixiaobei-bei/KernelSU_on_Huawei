

#include "host_hal_ops.h"
#include "host_hal_ops_1106.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HOST_MAIN_1106_C

const struct hal_common_ops_stru g_hal_common_ops_1106 = {
    .host_chip_irq_init          = hi1106_host_chip_irq_init,
    .host_rx_add_buff            = hi1106_host_rx_add_buff,
    .host_get_rx_msdu_status     = hi1106_host_get_rx_msdu_status,
    .host_rx_amsdu_list_build    = hi1106_host_rx_amsdu_list_build,
    .rx_get_next_sub_msdu        = hi1106_rx_get_next_sub_msdu,
    .host_rx_get_msdu_info_dscr  = hi1106_host_rx_get_msdu_info_dscr,
    .rx_host_start_dscr_queue    = hi1106_rx_host_start_dscr_queue,
    .rx_host_init_dscr_queue     = hi1106_rx_host_init_dscr_queue,
    .rx_host_stop_dscr_queue     = hi1106_rx_host_stop_dscr_queue,
    .rx_free_res                 = hi1106_rx_free_res,
    .tx_ba_info_dscr_get         = hi1106_tx_ba_info_dscr_get,
    .tx_msdu_dscr_info_get       = hi1106_tx_msdu_dscr_info_get,
    .host_tx_intr_init           = hi1106_host_tx_intr_init,
    .host_rx_reset_smac_handler  = hi1106_host_rx_reset_smac_handler,
    .rx_alloc_list_free          = hi1106_rx_alloc_list_free,
    .host_al_rx_fcs_info         = hi1106_host_al_rx_fcs_info,
    .host_get_rx_pckt_info       = hi1106_host_get_rx_pckt_info,
    .host_rx_proc_msdu_dscr      = hi1106_host_rx_proc_msdu_dscr,
    .host_init_common_timer      = hi1106_host_init_common_timer,
    .host_set_mac_common_timer   = hi1106_host_set_mac_common_timer,
#ifdef _PRE_WLAN_FEATURE_CSI
    .host_get_csi_info           = hi1106_get_csi_info,
    .host_csi_config             = hi1106_csi_config,
    .host_ftm_csi_init           = hi1106_host_ftm_csi_init,
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    .host_vap_get_hal_vap_id     = hi1106_host_vap_get_by_hal_vap_id,
    .host_ftm_reg_init           = hi1106_host_ftm_reg_init,
    .host_ftm_get_info           = hi1106_host_ftm_get_info,
    .host_ftm_get_divider        = hi1106_host_ftm_get_divider,
    .host_ftm_set_sample         = hi1106_host_ftm_set_sample,
    .host_ftm_set_enable         = hi1106_host_ftm_set_enable,
    .host_ftm_set_m2s_phy        = hi1106_host_ftm_set_m2s_phy,
    .host_ftm_set_buf_base_addr  = hi1106_host_ftm_set_buf_base_addr,
    .host_ftm_set_buf_size       = hi1106_host_ftm_set_buf_size,
    .host_ftm_set_white_list     = hi1106_host_ftm_set_white_list,
#endif
};
