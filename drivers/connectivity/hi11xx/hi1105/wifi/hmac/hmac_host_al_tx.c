

/* 1 头文件包含 */

#include "hmac_tx_amsdu.h"
#include "hmac_tx_msdu_dscr.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_AL_TX_C

#ifdef CONFIG_ARCH_KIRIN_PCIE
#define AL_TX_MSDU_NETBUF_MAX_NUM (4 * MAX_MAC_AGGR_MPDU_NUM)

OAL_STATIC oal_netbuf_stru *hmac_create_al_tx_packet(uint32_t size, uint8_t payload_flag)
{
    oal_netbuf_stru       *buf = NULL;
    uint32_t               loop;
    uint8_t               *data = NULL;
    buf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, (uint16_t)size + HAL_TX_MSDU_DSCR_LEN, OAL_NETBUF_PRIORITY_MID);
    if (buf == NULL) {
        oam_error_log0(0, OAM_SF_TX, "dmac_config_create_al_tx_packet::alloc Fail");
        return NULL;
    }
    oal_netbuf_put(buf, size);
    if (oal_netbuf_headroom(buf) < HAL_TX_MSDU_DSCR_LEN) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_netbuf_expand_headroom::netbuf headroom not enough, expanding...}");
        oal_netbuf_free(buf);
        return NULL;
    }
    data = oal_netbuf_data(buf);
    switch (payload_flag) {
        case RF_PAYLOAD_ALL_ZERO:
            memset_s(data, size, 0, size);
            break;
        case RF_PAYLOAD_ALL_ONE:
            memset_s(data, size, 0xFF, size);
            break;
        case RF_PAYLOAD_RAND:
            data[0] = oal_gen_random(18, 1); /* 18 随机数种子  */
            for (loop = 1; loop < size; loop++) {
                data[loop] = oal_gen_random(18, 0); /* 18 随机数种子  */
            }
            break;
        default:
            break;
    }
    oal_set_netbuf_next(buf, NULL);
    return buf;
}

OAL_STATIC void hmac_always_tx_set_tid_info(hmac_msdu_info_ring_stru *tx_ring)
{
    memset_s(tx_ring, sizeof(hmac_msdu_info_ring_stru), 0, sizeof(hmac_msdu_info_ring_stru));
    tx_ring->base_ring_info.size = HMAC_USER_TID_TX_RING_SIZE; /* 默认ring大小等级为6(缓存8192个msdu) */
    tx_ring->host_tx_flag = HOST_AL_TX_FLAG;
}

OAL_STATIC void hmac_always_tx_set_msdu_info(msdu_info_stru *msdu_info, oal_netbuf_stru *netbuf, uint64_t devva)
{
    memset_s(msdu_info, sizeof(msdu_info_stru), 0, sizeof(msdu_info_stru));
    msdu_info->msdu_addr_lsb = get_low_32_bits(devva);
    msdu_info->msdu_addr_msb = get_high_32_bits(devva);
    msdu_info->msdu_len = oal_netbuf_len(netbuf);
    msdu_info->data_type = DATA_TYPE_ETH;
    msdu_info->from_ds = 0;
    msdu_info->to_ds = 1;
}

OAL_STATIC void hmac_al_tx_set_ring_info(hmac_msdu_info_ring_stru *tx_ring, oal_netbuf_stru *netbuf, uint32_t index)
{
    tx_ring->netbuf_list[index] = netbuf;
    tx_ring->msdu_cnt++;
    tx_ring->msdu_len += netbuf->len;
    tx_ring->base_ring_info.write_index++;
}

OAL_STATIC void hmac_always_tx_set_update_info(mac_cfg_tx_comp_stru *event_set_al_tx,
    hmac_msdu_info_ring_stru *tx_ring, uint32_t net_buff_num, uint64_t devva)
{
    event_set_al_tx->base_addr_lsb = get_low_32_bits(devva);
    event_set_al_tx->base_addr_msb = get_high_32_bits(devva);
    event_set_al_tx->en_protocol_mode = tx_ring->base_ring_info.size;
    event_set_al_tx->en_ack_policy = net_buff_num / MAX_MAC_AGGR_MPDU_NUM;
    event_set_al_tx->write_index = net_buff_num;
    event_set_al_tx->uc_param = HAL_ALWAYS_TX_RF;
}
hmac_msdu_info_ring_stru g_always_tx_ring = {.msdu_cnt = 0};


uint32_t hmac_always_tx_get_netbuf_num(uint32_t payload_len)
{
    uint32_t net_buff_num;

    if (payload_len % AL_TX_MSDU_NETBUF_MAX_LEN == 0) {
        net_buff_num = payload_len / AL_TX_MSDU_NETBUF_MAX_LEN;
    } else {
        net_buff_num = payload_len / AL_TX_MSDU_NETBUF_MAX_LEN + 1;
    }
    net_buff_num = oal_min(net_buff_num, AL_TX_MSDU_NETBUF_MAX_NUM);
    return net_buff_num;
}


uint32_t hmac_always_tx_proc(uint8_t *param)
{
    mac_cfg_tx_comp_stru *event_set_al_tx = (mac_cfg_tx_comp_stru *)param;
    uint32_t ret;
    uint32_t net_buff_num;
    uint32_t index;
    uint64_t devva = 0;
    uint32_t packet_len;
    uint32_t alloc_size;
    hmac_msdu_info_ring_stru *tx_ring = &g_always_tx_ring;
    oal_netbuf_stru  *netbuf = NULL;
    uint8_t payload_flag = event_set_al_tx->en_payload_flag;
    uint32_t payload_len = event_set_al_tx->payload_len;
    hmac_always_tx_set_tid_info(tx_ring);
    ret = hmac_tx_alloc_ring(tx_ring);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_TX, "{hmac_always_tx_proc::hostva to devva failed[%d]}", ret);
        return ret;
    }

    net_buff_num = hmac_always_tx_get_netbuf_num(payload_len);
    for (index = 0; index < net_buff_num; index++) {
        packet_len = (index == (net_buff_num - 1)) ?
            (payload_len - (index * AL_TX_MSDU_NETBUF_MAX_LEN)) : AL_TX_MSDU_NETBUF_MAX_LEN;
        netbuf = hmac_create_al_tx_packet(packet_len, payload_flag);
        if (oal_unlikely(netbuf == NULL)) {
            hmac_tx_host_ring_release(tx_ring);
            oam_error_log0(0, OAM_SF_TX, "{hmac_always_tx_proc::netbuf is NULL}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        ret = hmac_tx_hostva_to_devva(hmac_tx_netbuf_init_msdu_dscr(netbuf),
            netbuf->len + HAL_TX_MSDU_DSCR_LEN, &devva);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_TX, "{hmac_always_tx_proc::hostva to devva failed[%d]}", ret);
            hmac_tx_host_ring_release(tx_ring);
            oal_netbuf_free(netbuf);
            return ret;
        }
        hmac_always_tx_set_msdu_info((msdu_info_stru *)(tx_ring->host_ring_buf + index * TX_RING_MSDU_INFO_LEN),
            netbuf, devva);
        hmac_al_tx_set_ring_info(tx_ring, netbuf, index);
    }
    alloc_size = RING_DEPTH(tx_ring->base_ring_info.size) * TX_RING_MSDU_INFO_LEN;
    if (pcie_if_hostca_to_devva(0, tx_ring->host_ring_dma_addr, &devva) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_always_tx_proc::hostva to devva failed");
        hmac_tx_host_ring_release(tx_ring);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    hmac_always_tx_set_update_info(event_set_al_tx, tx_ring, net_buff_num, devva);
    return ret;
}
#endif
