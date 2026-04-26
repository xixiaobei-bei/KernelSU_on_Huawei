

#include "host_hal_ops.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HOST_MAIN_BISHENG_C

const struct hal_common_ops_stru g_hal_common_ops_bisheng = {
    .host_chip_irq_init          = NULL,
    .host_rx_add_buff            = NULL,
    .host_get_rx_msdu_status     = NULL,
    .host_rx_amsdu_list_build    = NULL,
    .rx_get_next_sub_msdu        = NULL,
    .host_rx_get_msdu_info_dscr  = NULL,
    .rx_host_start_dscr_queue    = NULL,
    .rx_host_init_dscr_queue     = NULL,
    .rx_host_stop_dscr_queue     = NULL,
    .rx_free_res                 = NULL,
    .tx_ba_info_dscr_get         = NULL,
    .tx_msdu_dscr_info_get       = NULL,
    .host_tx_intr_init           = NULL,
    .host_rx_reset_smac_handler  = NULL,
    .rx_alloc_list_free          = NULL,
    .host_al_rx_fcs_info         = NULL,
    .host_rx_proc_msdu_dscr      = NULL,
#ifdef _PRE_WLAN_FEATURE_CSI
    .host_get_csi_info           = NULL,
    .host_csi_config             = NULL,
    .host_ftm_csi_init           = NULL,
#endif
};
