

#ifndef __OAL_SDIO_HOST_IF_H__
#define __OAL_SDIO_HOST_IF_H__

#include "oal_util.h"
#include "oal_net.h"
#include "oal_sdio_comm.h"
#include "oal_bus_if.h"
#include "oal_hcc_bus.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef CONFIG_MMC
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio.h>
#endif
#endif

enum {
    SDIO_READ = 0,
    SDIO_WRITE,
    SDIO_OPT_BUTT
};

typedef int32_t (*hisdio_rx)(void *data);

/* 0x30~0x38, 0x3c~7B */
#define HISDIO_EXTEND_BASE_ADDR 0x30
#define HISDIO_EXTEND_REG_COUNT 64

#define hisdio_align_4_or_blk(len)     ((len) < HISDIO_BLOCK_SIZE ? ALIGN((len), 4) : ALIGN((len), HISDIO_BLOCK_SIZE))

#define HISDIO_WAKEUP_DEV_REG   0xf0
#define ALLOW_TO_SLEEP_VALUE    1
#define DISALLOW_TO_SLEEP_VALUE 0

#define SDIO_TC_BUF_TX_LEN (512 * 3)
#if (_PRE_OS_VERSION_LINUX != _PRE_OS_VERSION)
struct sdio_func {
    unsigned enable_timeout; /* max enable timeout in msec */
};
#endif

struct hisdio_extend_func {
    uint32_t int_stat;
    uint32_t msg_stat;
    uint32_t xfer_count;
    uint32_t credit_info;
    uint8_t comm_reg[HISDIO_EXTEND_REG_COUNT];
};

struct sdio_scatt {
    uint32_t max_scatt_num;
    struct scatterlist *sglist;
};

struct hsdio_func1_info {
    uint32_t func1_err_reg_info;
    uint32_t func1_err_int_count;
    uint32_t func1_ack_int_acount;
    uint32_t func1_msg_int_count;
    uint32_t func1_data_int_count;
    uint32_t func1_no_int_count;
};

struct hsdio_error_info {
    uint32_t rx_scatt_info_not_match;
};

typedef struct _hsdio_scatt_buff_ {
    /* record the tx scatt list assembled buffer */
    void *buff;
    uint32_t len;
} hsdio_scatt_buff;

struct hsdio_credit_info {
    uint8_t short_free_cnt;
    uint8_t large_free_cnt;
    oal_spin_lock_stru credit_lock;
};

struct oal_sdio {
    /* sdio work state, sleep , work or shutdown? */
    hcc_bus *pst_bus;

    uint32_t state; /* 总线当前状态 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct sdio_func *func;
#endif

    hisdio_rx credit_update_cb;

    /* used to sg list sdio block align */
    uint8_t *sdio_align_buff;

    uint64_t sdio_int_count;

    uint32_t sdio_suspend;
    uint32_t sdio_resume;

    struct sdio_scatt scatt_info[SDIO_OPT_BUTT];

    /* This is get from sdio , must alloc for dma, the extend area only can access by CMD53 */
    struct hisdio_extend_func *sdio_extend;
    struct hsdio_credit_info sdio_credit_info;
    uint32_t func1_int_mask;
    struct hsdio_func1_info func1_stat;
    struct hsdio_error_info error_stat;

    hsdio_scatt_buff tx_scatt_buff;
    hsdio_scatt_buff rx_scatt_buff;
    void *rx_reserved_buff; /* use the mem when rx alloc mem failed! */
    uint32_t rx_reserved_buff_len;

    oal_wakelock_stru st_sdio_rx_wakelock;

    oal_completion st_sdio_shutdown_response;
};

OAL_STATIC OAL_INLINE void oal_sdio_claim_host(struct oal_sdio *hi_sdio)
{
#ifdef CONFIG_MMC
    if (oal_warn_on(hi_sdio == NULL)) {
        return;
    }

    if (oal_warn_on(hi_sdio->func == NULL)) {
        return;
    }
    sdio_claim_host(hi_sdio->func);
#endif
}

OAL_STATIC OAL_INLINE void oal_sdio_release_host(struct oal_sdio *hi_sdio)
{
#ifdef CONFIG_MMC
    if (oal_warn_on(hi_sdio == NULL)) {
        return;
    }

    if (oal_warn_on(hi_sdio->func == NULL)) {
        return;
    }
    sdio_release_host(hi_sdio->func);
#endif
}

uint32_t oal_sdio_get_large_pkt_free_cnt(struct oal_sdio *hi_sdio);

void oal_netbuf_list_hex_dump(oal_netbuf_head_stru *head);
void oal_netbuf_hex_dump(oal_netbuf_stru *netbuf);
extern int32_t oal_wifi_platform_load_sdio(void);
extern void oal_wifi_platform_unload_sdio(void);

#ifdef CONFIG_MMC
void oal_sdio_info_show(struct oal_sdio *hi_sdio);
void oal_netbuf_list_hex_dump(oal_netbuf_head_stru *head);
void oal_netbuf_hex_dump(oal_netbuf_stru *netbuf);
int32_t oal_sdio_func_probe(struct oal_sdio *hi_sdio);
void oal_sdio_credit_update_cb_register(struct oal_sdio *hi_sdio, hisdio_rx cb);
extern int32_t oal_sdio_transfer_tx(struct oal_sdio *hi_sdio, oal_netbuf_stru *netbuf);

int32_t oal_sdio_transfer_netbuf_list(struct oal_sdio *hi_sdio,
                                      oal_netbuf_head_stru *head,
                                      int32_t rw);

extern struct oal_sdio *oal_sdio_init_module(void);
extern void oal_sdio_exit_module(struct oal_sdio *hi_sdio);

int32_t oal_sdio_send_msg(hcc_bus *pst_bus, uint32_t val);
extern uint32_t oal_sdio_func_max_req_size(struct oal_sdio *pst_hi_sdio);
extern int32_t oal_sdio_110x_working_check(void);
extern void oal_sdio_exit_module(struct oal_sdio *hi_sdio);

extern struct oal_sdio *g_hi110x_sdio;
OAL_STATIC OAL_INLINE struct oal_sdio *oal_get_sdio_default_handler(void)
{
    return g_hi110x_sdio;
}
#else
extern struct oal_sdio g_hi_sdio_ut;
OAL_STATIC OAL_INLINE struct oal_sdio *oal_get_sdio_default_handler(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    return NULL;
#else
    return &g_hi_sdio_ut;
#endif
}

OAL_STATIC OAL_INLINE uint32_t oal_sdio_func_max_req_size(struct oal_sdio *pst_hi_sdio)
{
    return 0;
}

OAL_STATIC OAL_INLINE void oal_sdio_info_show(struct oal_sdio *hi_sdio)
{
    return;
}

OAL_STATIC OAL_INLINE int32_t oal_sdio_send_msg(void *hi_sdio, uint32_t val)
{
    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE void oal_sdio_credit_update_cb_register(struct oal_sdio *hi_sdio, hisdio_rx cb)
{
    return;
}

OAL_STATIC OAL_INLINE int32_t oal_sdio_transfer_tx(struct oal_sdio *hi_sdio, oal_netbuf_stru *netbuf)
{
    return -OAL_FAIL;
}

OAL_STATIC OAL_INLINE int32_t oal_sdio_transfer_netbuf_list(struct oal_sdio *hi_sdio,
                                                            oal_netbuf_head_stru *head,
                                                            int32_t rw)
{
    return -OAL_FAIL;
}

OAL_STATIC OAL_INLINE struct oal_sdio *oal_sdio_init_module(void)
{
    return NULL;
}

OAL_STATIC OAL_INLINE void oal_sdio_exit_module(struct oal_sdio *hi_sdio)
{
}

OAL_STATIC OAL_INLINE int32_t oal_sdio_func_probe(struct oal_sdio *hi_sdio)
{
    return -1;
}
#endif

/* Kernel Functions */
#ifdef CONFIG_MMC
#define CONFIG_HISI_SDIO_TIME_DEBUG
/*
 * sdio_memcpy_fromio - read a chunk of memory from a SDIO function
 * @func: SDIO function to access
 * @dst: buffer to store the data
 * @addr: address to begin reading from
 * @count: number of bytes to read
 * Reads from the address space of a given SDIO function. Return
 * value indicates if the transfer succeeded or not.
 */
OAL_STATIC OAL_INLINE int32_t oal_sdio_memcpy_fromio(struct sdio_func *func, void *dst,
                                                     uint32_t addr, int32_t count)
{
    int32_t ret;
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    ktime_t time_start;
    time_start = ktime_get();
#endif
    ret = sdio_memcpy_fromio(func, dst, addr, count);
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    if (oal_unlikely(ret)) {
        /* If sdio transfer failed, dump the sdio info */
        uint64_t trans_us;
        ktime_t time_stop = ktime_get();
        trans_us = (uint64_t)ktime_to_us(ktime_sub(time_stop, time_start));
        printk(KERN_WARNING "[E]sdio_memcpy_fromio fail=%d, time cost:%llu us,[dst:%p,addr:%u,count:%d]\n",
               ret, trans_us, dst, addr, count);
    }
#endif
    return ret;
}

/*
 * oal_sdio_readsb - read from a FIFO on a SDIO function
 * @func: SDIO function to access
 * @dst: buffer to store the data
 * @addr: address of (single byte) FIFO
 * @count: number of bytes to read
 * Reads from the specified FIFO of a given SDIO function. Return
 * value indicates if the transfer succeeded or not.
 */
OAL_STATIC OAL_INLINE int32_t oal_sdio_readsb(struct sdio_func *func, void *dst, uint32_t addr,
                                              int32_t count)
{
    int32_t ret;
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    ktime_t time_start;
    time_start = ktime_get();
#endif
    ret = sdio_readsb(func, dst, addr, count);
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    if (oal_unlikely(ret)) {
        /* If sdio transfer failed, dump the sdio info */
        uint64_t trans_us;
        ktime_t time_stop = ktime_get();
        trans_us = (uint64_t)ktime_to_us(ktime_sub(time_stop, time_start));
        printk(KERN_WARNING "[E]sdio_readsb fail=%d, time cost:%llu us,[dst:%p,addr:%u,count:%d]\n",
               ret, trans_us, dst, addr, count);
    }
#endif
    return ret;
}

/*
 * oal_sdio_writesb - write to a FIFO of a SDIO function
 * @func: SDIO function to access
 * @addr: address of (single byte) FIFO
 * @src: buffer that contains the data to write
 * @count: number of bytes to write
 * Writes to the specified FIFO of a given SDIO function. Return
 * value indicates if the transfer succeeded or not.
 */
OAL_STATIC OAL_INLINE int32_t oal_sdio_writesb(struct sdio_func *func, uint32_t addr, void *src,
                                               int count)
{
    int32_t ret;
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    ktime_t time_start;
    time_start = ktime_get();
#endif
    ret = sdio_writesb(func, addr, src, count);
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    if (oal_unlikely(ret)) {
        /* If sdio transfer failed, dump the sdio info */
        uint64_t trans_us;
        ktime_t time_stop = ktime_get();
        trans_us = (uint64_t)ktime_to_us(ktime_sub(time_stop, time_start));
        printk(KERN_WARNING "[E]oal_sdio_writesb fail=%d, time cost:%llu us,[src:%p,addr:%u,count:%d]\n",
               ret, trans_us, src, addr, count);
    }
#endif
    return ret;
}

/*
 * oal_sdio_readb - read a single byte from a SDIO function
 * @func: SDIO function to access
 * @addr: address to read
 * @err_ret: optional status value from transfer
 * Reads a single byte from the address space of a given SDIO
 * function. If there is a problem reading the address, 0xff
 * is returned and @err_ret will contain the error code.
 */
OAL_STATIC OAL_INLINE uint8_t oal_sdio_readb(struct sdio_func *func, uint32_t addr, int32_t *err_ret)
{
    return sdio_readb(func, addr, err_ret);
}

/*
 * oal_sdio_writeb - write a single byte to a SDIO function
 * @func: SDIO function to access
 * @b: byte to write
 * @addr: address to write to
 * @err_ret: optional status value from transfer
 * Writes a single byte to the address space of a given SDIO
 * function. @err_ret will contain the status of the actual
 * transfer.
 */
OAL_STATIC OAL_INLINE void oal_sdio_writeb(struct sdio_func *func, uint8_t b, uint32_t addr, int32_t *err_ret)
{
    sdio_writeb(func, b, addr, err_ret);
}

/*
 * oal_sdio_readl - read a 32 bit integer from a SDIO function
 * @func: SDIO function to access
 * @addr: address to read
 * @err_ret: optional status value from transfer
 * Reads a 32 bit integer from the address space of a given SDIO
 * function. If there is a problem reading the address,
 * 0xffffffff is returned and @err_ret will contain the error
 * code.
 */
OAL_STATIC OAL_INLINE uint32_t oal_sdio_readl(struct sdio_func *func, uint32_t addr, int32_t *err_ret)
{
    return sdio_readl(func, addr, err_ret);
}

/*
 * oal_sdio_writel - write a 32 bit integer to a SDIO function
 * @func: SDIO function to access
 * @b: integer to write
 * @addr: address to write to
 * @err_ret: optional status value from transfer
 * Writes a 32 bit integer to the address space of a given SDIO
 * function. @err_ret will contain the status of the actual
 * transfer.
 */
OAL_STATIC OAL_INLINE void oal_sdio_writel(struct sdio_func *func, uint32_t b,
                                           uint32_t addr, int32_t *err_ret)
{
    sdio_writel(func, b, addr, err_ret);
}
#else
OAL_STATIC OAL_INLINE int32_t oal_sdio_memcpy_fromio(struct sdio_func *func, void *dst,
                                                     uint32_t addr, int32_t count)
{
    oal_reference(func);
    oal_reference(dst);
    oal_reference(addr);
    oal_reference(count);
    return -OAL_EBUSY;
}

OAL_STATIC OAL_INLINE int32_t oal_sdio_readsb(struct sdio_func *func, void *dst, uint32_t addr,
                                              int32_t count)
{
    oal_reference(func);
    oal_reference(dst);
    oal_reference(addr);
    oal_reference(count);
    return -OAL_EBUSY;
}

OAL_STATIC OAL_INLINE int32_t oal_sdio_writesb(struct sdio_func *func, uint32_t addr, void *src,
                                               int count)
{
    oal_reference(func);
    oal_reference(src);
    oal_reference(addr);
    oal_reference(count);
    return -OAL_EBUSY;
}

OAL_STATIC OAL_INLINE uint8_t oal_sdio_readb(struct sdio_func *func, uint32_t addr, int32_t *err_ret)
{
    oal_reference(func);
    oal_reference(addr);
    *err_ret = -OAL_EBUSY;
    ;
    return 0;
}

OAL_STATIC OAL_INLINE void oal_sdio_writeb(struct sdio_func *func, uint8_t b, uint32_t addr, int32_t *err_ret)
{
    oal_reference(func);
    oal_reference(addr);
    oal_reference(b);
    *err_ret = -OAL_EBUSY;
}

OAL_STATIC OAL_INLINE uint32_t oal_sdio_readl(struct sdio_func *func, uint32_t addr, int32_t *err_ret)
{
    oal_reference(func);
    oal_reference(addr);
    *err_ret = -OAL_EBUSY;
    ;
    return 0;
}

OAL_STATIC OAL_INLINE void oal_sdio_writel(struct sdio_func *func, uint32_t b,
                                           uint32_t addr, int32_t *err_ret)
{
    oal_reference(func);
    oal_reference(addr);
    oal_reference(b);
    *err_ret = -OAL_EBUSY;
}
#endif

#endif
