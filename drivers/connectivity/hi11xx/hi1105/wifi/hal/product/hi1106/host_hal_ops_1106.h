

#ifndef __HOST_HAL_OPS_HI1106_H__
#define __HOST_HAL_OPS_HI1106_H__

#include "host_hal_device.h"
#include "host_dscr_1106.h"
#include "host_ftm_1106.h"
#include "host_csi_1106.h"
#include "hal_common.h"
#include "mac_common.h"

extern const struct hal_common_ops_stru g_hal_common_ops_1106;


int32_t hi1106_rx_host_init_dscr_queue(uint8_t hal_dev_id);

uint32_t hi1106_host_rx_get_msdu_info_dscr(hal_host_device_stru *hal_dev,
    oal_netbuf_stru *netbuf, mac_rx_ctl_stru *rx_info);

void hi1106_host_rx_add_buff(hal_host_device_stru *hal_device, uint8_t en_queue_num);
void hi1106_host_rx_amsdu_list_build(hal_host_device_stru *hal_device, oal_netbuf_stru *netbuf);
uint8_t hi1106_host_get_rx_msdu_status(oal_netbuf_stru *netbuf);
uint32_t hi1106_host_rx_proc_msdu_dscr(hal_host_device_stru *hal_dev, oal_netbuf_stru *netbuf);
void hi1106_host_chip_irq_init(void);
void hi1106_rx_free_res(hal_host_device_stru *hal_device);
oal_netbuf_stru *hi1106_alloc_list_delete_netbuf(hal_host_device_stru *hal_dev,
    hal_host_rx_alloc_list_stru *alloc_list, dma_addr_t host_iova, uint32_t *pre_num);
void hi1106_rx_alloc_list_free(hal_host_device_stru *hal_dev, hal_host_rx_alloc_list_stru *alloc_list);
void hi1106_host_al_rx_fcs_info(hmac_vap_stru *hmac_vap);
void hi1106_host_get_rx_pckt_info(hmac_vap_stru *hmac_vap,
    dmac_atcmdsrv_atcmd_response_event *rx_pkcg_event_info);
int32_t hi1106_host_init_common_timer(hal_mac_common_timer *mac_timer);
void hi1106_host_set_mac_common_timer(hal_mac_common_timer *mac_common_timer);
void hi1106_host_disable_machw_phy_and_pa(void);
void hi1106_host_enable_machw_phy_and_pa(void);
#endif

