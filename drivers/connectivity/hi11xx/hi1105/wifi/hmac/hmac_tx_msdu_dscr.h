

#ifndef __HMAC_TX_MSDU_DSCR_H__
#define __HMAC_TX_MSDU_DSCR_H__

/* 1 其他头文件包含 */
#include "hmac_user.h"
#include "hmac_vap.h"
#include "hmac_tx_data.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DRIVER_HMAC_TX_MSDU_DSCR_H
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define get_low_32_bits(a)  ((uint32_t)(((uint64_t)(a)) & 0x00000000FFFFFFFFUL))
#define get_high_32_bits(a) ((uint32_t)((((uint64_t)(a)) & 0xFFFFFFFF00000000UL) >> 32UL))
#define HOST_AL_TX_FLAG   0x5a


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_get_tx_ring_enable(hmac_user_stru *hmac_user, uint8_t tid)
{
    return oal_atomic_read(&hmac_user->tx_tid_info[tid].tx_ring.enabled);
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_tid_ring_full(hmac_msdu_info_ring_stru *tx_msdu_ring)
{
    un_rw_ptr write_ptr = { .rw_ptr = tx_msdu_ring->base_ring_info.write_index };
    un_rw_ptr release_ptr = { .rw_ptr = tx_msdu_ring->release_index };

    return IS_RW_RING_FULL(&release_ptr, &write_ptr);
}


OAL_STATIC OAL_INLINE ring_ptr_compare_enum hmac_tx_rw_ptr_compare(un_rw_ptr rw_ptr1, un_rw_ptr rw_ptr2)
{
    if (rw_ptr1.rw_ptr == rw_ptr2.rw_ptr) {
        return RING_PTR_EQUAL;
    }

    if (rw_ptr1.st_rw_ptr.bit_wrap_flag == rw_ptr2.st_rw_ptr.bit_wrap_flag) {
        return (rw_ptr1.st_rw_ptr.bit_rw_ptr > rw_ptr2.st_rw_ptr.bit_rw_ptr) ? RING_PTR_GREATER : RING_PTR_SMALLER;
    } else {
        return (rw_ptr1.st_rw_ptr.bit_rw_ptr < rw_ptr2.st_rw_ptr.bit_rw_ptr) ? RING_PTR_GREATER : RING_PTR_SMALLER;
    }
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_tx_ring_rw_ptr_check(hmac_msdu_info_ring_stru *tx_msdu_ring)
{
    un_rw_ptr release_ptr = { .rw_ptr = tx_msdu_ring->release_index };
    un_rw_ptr read_ptr = { .rw_ptr = tx_msdu_ring->base_ring_info.read_index };
    un_rw_ptr write_ptr = { .rw_ptr = tx_msdu_ring->base_ring_info.write_index };
    ring_ptr_compare_enum read_release_compare = hmac_tx_rw_ptr_compare(read_ptr, release_ptr);
    ring_ptr_compare_enum write_read_compare = hmac_tx_rw_ptr_compare(write_ptr, read_ptr);

    return (read_release_compare == RING_PTR_EQUAL || read_release_compare == RING_PTR_GREATER) &&
           (write_read_compare == RING_PTR_EQUAL || write_read_compare == RING_PTR_GREATER);
}

OAL_STATIC OAL_INLINE oal_netbuf_stru *hmac_tx_ring_get_netbuf(
    hmac_msdu_info_ring_stru *tx_ring, un_rw_ptr netbuf_ptr)
{
    uint16_t netbuf_index = netbuf_ptr.st_rw_ptr.bit_rw_ptr;

    if (tx_ring->netbuf_list == NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_ring_get_netbuf::netbuf list is NULL!}");
        return NULL;
    }

    if (tx_ring->netbuf_list[netbuf_index] == NULL) {
        oam_error_log1(0, OAM_SF_TX, "{hmac_tx_ring_get_netbuf::netbuf[%d] is NULL!}", netbuf_index);
    }

    return tx_ring->netbuf_list[netbuf_index];
}


OAL_STATIC OAL_INLINE msdu_info_stru *hmac_tx_get_ring_msdu_info(hmac_msdu_info_ring_stru *tx_ring, uint16_t index)
{
    return (msdu_info_stru *)(tx_ring->host_ring_buf + index * TX_RING_MSDU_INFO_LEN);
}


OAL_STATIC OAL_INLINE void hmac_tx_reset_msdu_info(hmac_msdu_info_ring_stru *tx_ring, un_rw_ptr reset_ptr)
{
    msdu_info_stru *msdu_info = hmac_tx_get_ring_msdu_info(tx_ring, reset_ptr.st_rw_ptr.bit_rw_ptr);
    if (tx_ring->host_tx_flag == HOST_AL_TX_FLAG) {
        return;
    }

    memset_s(msdu_info, sizeof(msdu_info_stru), 0, sizeof(msdu_info_stru));
}


OAL_STATIC OAL_INLINE void hmac_tx_rw_ptr_inc(un_rw_ptr *rw_ptr, uint16_t ring_size)
{
    if (++rw_ptr->st_rw_ptr.bit_rw_ptr >= ring_size) {
        rw_ptr->st_rw_ptr.bit_rw_ptr = 0;
        rw_ptr->st_rw_ptr.bit_wrap_flag = !rw_ptr->st_rw_ptr.bit_wrap_flag;
    }
}


OAL_STATIC OAL_INLINE void hmac_tx_msdu_ring_inc_release_ptr(hmac_msdu_info_ring_stru *tx_ring)
{
    un_rw_ptr release_ptr = { .rw_ptr = tx_ring->release_index };

    hmac_tx_rw_ptr_inc(&release_ptr, RING_DEPTH(tx_ring->base_ring_info.size));

    tx_ring->release_index = release_ptr.rw_ptr;
}

/* 10 函数声明 */
uint32_t hmac_tx_ring_push_msdu(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user,
    hmac_tid_info_stru *tid_info, hmac_msdu_info_ring_stru *tx_msdu_ring, oal_netbuf_stru *netbuf);
uint32_t hmac_tx_ring_init(hmac_user_stru *hmac_user, uint8_t tid, uint8_t ring_size);
void hmac_user_tx_ring_deinit(hmac_user_stru *hmac_user);
uint32_t hmac_set_tx_ring_device_base_addr(frw_event_mem_stru *frw_mem);
uint32_t hmac_tx_hostva_to_devva(uint8_t *hostva, uint32_t alloc_size, uint64_t *devva);
uint8_t *hmac_tx_netbuf_init_msdu_dscr(oal_netbuf_stru *netbuf);
uint32_t hmac_tx_alloc_ring(hmac_msdu_info_ring_stru *tx_ring);
void hmac_tx_host_ring_release(hmac_msdu_info_ring_stru *tx_ring);
void hmac_tx_reg_write_ring_info(hmac_user_stru *hmac_user, hmac_tid_info_stru *tx_tid_info, tid_cmd_enum_uint8 cmd);
uint32_t hmac_tx_sync_ring_info(mac_user_stru *mac_user, hmac_tid_info_stru *tx_tid_info, tid_cmd_enum_uint8 cmd);
void hmac_tx_ring_release_netbuf(hmac_msdu_info_ring_stru *tx_ring, oal_netbuf_stru *netbuf, uint16_t release_index);
uint32_t hmac_tx_unmap_msdu_dma_addr(hmac_msdu_info_ring_stru *tx_ring, msdu_info_stru *msdu_info, uint32_t netbuf_len);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
