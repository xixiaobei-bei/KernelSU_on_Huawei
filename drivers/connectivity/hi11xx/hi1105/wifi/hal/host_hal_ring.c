

#include "oal_util.h"
#include "oal_net.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "host_hal_ring.h"
#include "securec.h"
#include "host_hal_device.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID     OAM_FILE_ID_HAL_RING_C
#define HAL_WORD_TO_BYTE 4


uint32_t hal_ring_init(hal_host_ring_ctl_stru *ring_ctl)
{
    return OAL_SUCC;
}


void hal_ring_get_hw2sw(hal_host_ring_ctl_stru *ring_ctl)
{
    uint32_t reginfo;

    /* 入参判断 */
    if (oal_unlikely(ring_ctl == NULL)) {
        oam_error_log1(0, OAM_SF_RX, "{hal_ring_get_hw2sw::ring_ctl[%p].}", (uintptr_t)ring_ctl);
        return;
    }

    if (ring_ctl->ring_type == HAL_RING_TYPE_FREE_RING) {
        reginfo = oal_readl((void*)(uintptr_t)ring_ctl->read_ptr_reg);
        ring_ctl->un_read_ptr.read_ptr = (uint16_t)reginfo;
    } else if (ring_ctl->ring_type == HAL_RING_TYPE_COMPLETE_RING) {
        reginfo = oal_readl((void*)(uintptr_t)ring_ctl->write_ptr_reg);
        ring_ctl->un_write_ptr.write_ptr = (uint16_t)reginfo;
    } else {
        return;
    }

    return;
}


uint32_t hal_ring_set_sw2hw(hal_host_ring_ctl_stru *ring_ctl)
{
    if (ring_ctl == NULL) {
        return OAL_FAIL;
    }

    if (ring_ctl->ring_type == HAL_RING_TYPE_FREE_RING) {
        oal_writel(ring_ctl->un_write_ptr.write_ptr, (uintptr_t)ring_ctl->write_ptr_reg);
    } else if (ring_ctl->ring_type == HAL_RING_TYPE_COMPLETE_RING) {
        oal_writel(ring_ctl->un_read_ptr.read_ptr, (uintptr_t)ring_ctl->read_ptr_reg);
    } else {
        oam_error_log1(0, OAM_SF_RX, "{hal_ring_set_sw2hw::ring_type[%d] err.}", ring_ctl->ring_type);
    }

    return OAL_SUCC;
}


uint32_t hal_ring_get_entry_count(hal_host_ring_ctl_stru *ring_ctl, uint16_t *p_count)
{
    uint16_t count = 0;
    uint16_t read_idx;
    uint16_t write_idx;

    if (oal_unlikely(oal_any_null_ptr2(ring_ctl, p_count))) {
        oam_error_log0(0, OAM_SF_RX, "{hal_ring_get_entry_count::input para null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_ring_get_hw2sw(ring_ctl);
    if (ring_ctl->ring_type == HAL_RING_TYPE_FREE_RING) {
        if (hal_ring_is_full(ring_ctl)) {
            *p_count = 0;
            return OAL_SUCC;
        }
        write_idx = ring_ctl->un_write_ptr.st_write_ptr.bit_write_ptr;
        read_idx = ring_ctl->un_read_ptr.st_read_ptr.bit_read_ptr;

        if (hal_ring_wrap_around(ring_ctl)) {
            count = read_idx - write_idx;
        } else {
            count = ring_ctl->entries - write_idx;
            count += read_idx;
        }
    } else if (ring_ctl->ring_type == HAL_RING_TYPE_COMPLETE_RING) {
        if (hal_ring_is_empty(ring_ctl)) {
            *p_count = 0;
            return OAL_SUCC;
        }
        write_idx = ring_ctl->un_write_ptr.st_write_ptr.bit_write_ptr;
        read_idx = ring_ctl->un_read_ptr.st_read_ptr.bit_read_ptr;
        if (!hal_ring_wrap_around(ring_ctl)) {
            count = write_idx - read_idx;
        } else {
            count = ring_ctl->entries - read_idx;
            count += write_idx;
        }
    }
    *p_count = count;
    return OAL_SUCC;
}


OAL_STATIC void hal_ring_get_ring(uint8_t *entries, uint8_t *src_addr, uint32_t size)
{
    volatile uint32_t *src_addr_volatile = NULL;
    uint32_t i;

    if ((size % HAL_WORD_TO_BYTE) != 0) {
        oam_error_log1(0, OAM_SF_RX, "{hal_ring_get_ring::size = [%d].}", size);
        return;
    }

    src_addr_volatile = (volatile uint32_t *)src_addr;
    for (i = 0; i < size / HAL_WORD_TO_BYTE; i++) {
        *(uint32_t *)(entries + HAL_WORD_TO_BYTE * i) = *(src_addr_volatile + i);
    }
}

uint32_t hal_ring_get_entries(hal_host_ring_ctl_stru *ring_ctl,
    uint8_t *entries, uint16_t count)
{
    uint16_t entry_size;
    uint16_t remains;
    uint16_t read_idx;
    uint8_t *src_addr = NULL;
    /* 入参判断 */
    if (oal_unlikely(oal_any_null_ptr2(ring_ctl, entries))) {
        oam_error_log0(0, OAM_SF_RX, "{hal_ring_get_entries::input para null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (ring_ctl->ring_type != HAL_RING_TYPE_COMPLETE_RING) {
        oam_error_log0(0, OAM_SF_RX, "{hal_ring_get_entries::not complete ring!.}");
        return OAL_FAIL;  //lint !e527
    }

    if (hal_ring_is_empty(ring_ctl)) {
        oam_error_log0(0, OAM_SF_RX, "{hal_ring_get_entries::HAL_RING_IS_EMPTY!}");
        return OAL_FAIL;  //lint !e527
    }

    entry_size = ring_ctl->entry_size;

    read_idx = ring_ctl->un_read_ptr.st_read_ptr.bit_read_ptr;
    src_addr = (uint8_t *)(ring_ctl->p_entries) + entry_size * read_idx;
    if (count + read_idx >= ring_ctl->entries) {
        remains = (count + read_idx) % ring_ctl->entries;
        hal_ring_get_ring(entries, src_addr, entry_size * (ring_ctl->entries - read_idx));

        /* 内容读出后，清0 */
        memset_s(src_addr, entry_size * (ring_ctl->entries - read_idx),
            0, entry_size * (ring_ctl->entries - read_idx));
        if (remains != 0) {
            hal_ring_get_ring(entries + entry_size * (ring_ctl->entries - read_idx),
                (uint8_t *)(ring_ctl->p_entries), entry_size * remains);
            /* 内容读出后，清0 */
            memset_s(ring_ctl->p_entries, entry_size * remains, 0, entry_size * remains);
        }
        ring_ctl->un_read_ptr.st_read_ptr.bit_read_ptr = remains;
        ring_ctl->un_read_ptr.st_read_ptr.bit_wrap_flag =
            !ring_ctl->un_read_ptr.st_read_ptr.bit_wrap_flag;
    } else {
        hal_ring_get_ring(entries, src_addr, entry_size * count);
        /* 内容读出后，清0 */
        memset_s(src_addr, entry_size * count, 0, entry_size * count);
        ring_ctl->un_read_ptr.st_read_ptr.bit_read_ptr += count;
    }
    return OAL_SUCC;
}


uint32_t hal_ring_set_entries(hal_host_ring_ctl_stru *ring_ctl, uint64_t entry)
{
    uint16_t write_idx;

    if (oal_any_null_ptr1(ring_ctl) || (!entry)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (ring_ctl->ring_type != HAL_RING_TYPE_FREE_RING) {
        return OAL_FAIL;
    }

    if (hal_ring_is_full(ring_ctl)) {
        return OAL_FAIL;
    }
    write_idx = ring_ctl->un_write_ptr.st_write_ptr.bit_write_ptr;
    ring_ctl->p_entries[write_idx] = (uint64_t)entry;

    write_idx++;
    if (write_idx >= ring_ctl->entries) {
        /* 已到达ring的最后一个元素，index从0开始计数，并设置flag标志(flag翻转) */
        ring_ctl->un_write_ptr.st_write_ptr.bit_write_ptr = 0;
        ring_ctl->un_write_ptr.st_write_ptr.bit_wrap_flag = !ring_ctl->un_write_ptr.st_write_ptr.bit_wrap_flag;
    } else {
        /* 未到达ring的最后一个元素，index递增，flag标志保存不变 */
        ring_ctl->un_write_ptr.st_write_ptr.bit_write_ptr = write_idx;
    }

    return OAL_SUCC;
}


void hal_update_free_ring_wptr(hal_host_ring_ctl_stru *ring_ctl, uint16_t count)
{
    uint16_t remains;
    uint16_t write_idx;

    if (hal_ring_is_full(ring_ctl)) {
        return;  // lint !e527
    }

    write_idx = ring_ctl->un_write_ptr.st_write_ptr.bit_write_ptr;
    if (count + write_idx >= ring_ctl->entries) {
        remains = (count + write_idx) % ring_ctl->entries;
        ring_ctl->un_write_ptr.st_write_ptr.bit_write_ptr = remains;
        ring_ctl->un_write_ptr.st_write_ptr.bit_wrap_flag =
            !ring_ctl->un_write_ptr.st_write_ptr.bit_wrap_flag;
    } else {
        ring_ctl->un_write_ptr.st_write_ptr.bit_write_ptr += count;
    }
    return;
}
