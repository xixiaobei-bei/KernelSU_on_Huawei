

#define HISI_LOG_TAG               "[SDIO]"
#define HI11XX_LOG_MODULE_NAME     "[SDIO]"
#define HI11XX_LOG_MODULE_NAME_VAR sdio_loglevel
#include "oal_util.h"
#include "oal_sdio.h"
#include "oal_sdio_host_if.h"
#include "oal_net.h"
#include "oal_ext_if.h"

#ifdef CONFIG_MMC
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/host.h>
#include <linux/pm_runtime.h>
#include <linux/random.h>
#include "oal_hcc_host_if.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_exception_rst.h"
#endif
#include "plat_pm.h"
#include "securec.h"
#include "oam_ext_if.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_SDIO_HOST_C

#undef CONFIG_SDIO_MSG_ACK_HOST2ARM_DEBUG

OAL_STATIC struct oal_sdio *g_hi_sdio_debug = NULL;

/* 全局变量定义 */
OAL_STATIC struct completion g_sdio_driver_complete;
struct oal_sdio *g_hi110x_sdio;

OAL_STATIC uint8_t *g_sdio_enum_err_str = "probe timeout";
OAL_STATIC int32_t g_hisdio_intr_mode = 1; /* 0 -sdio 1-gpio */

#ifdef CONFIG_SDIO_FUNC_EXTEND
OAL_STATIC uint32_t g_sdio_extend_func = 1;
#else
OAL_STATIC uint32_t g_sdio_extend_func = 0;
#endif

#ifdef CONFIG_ARCH_HI1103_SDIO_DEBUG
extern int g_sdio_reset_ip;
oal_debug_module_param(g_sdio_reset_ip, int, S_IRUGO | S_IWUSR);
OAL_STATIC uint32_t g_wifi_sdio_fail_painc_limit = 4;
OAL_STATIC uint32_t g_wifi_sdio_fail_count = 0;
#endif

/* 函数定义 */
OAL_STATIC int32_t _oal_sdio_transfer_scatt(struct oal_sdio *hi_sdio, int32_t rw,
                                            uint32_t addr, struct scatterlist *sg,
                                            uint32_t sg_len,
                                            uint32_t rw_sz);
OAL_STATIC hcc_bus *oal_sdio_bus_init(struct oal_sdio *hi_sdio);
OAL_STATIC int32_t oal_sdio_get_state(hcc_bus *pst_bus, uint32_t mask);
OAL_STATIC void oal_enable_sdio_state(hcc_bus *pst_bus, uint32_t mask);
OAL_STATIC void oal_disable_sdio_state(hcc_bus *pst_bus, uint32_t mask);
OAL_STATIC void oal_sdio_bus_exit(struct oal_sdio *hi_sdio);
OAL_STATIC int32_t oal_sdio_wakeup_dev(struct oal_sdio *hi_sdio);

/* notify the mmc to probe sdio device. */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
extern void dw_mci_sdio_card_detect_change(void);
#endif

#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
OAL_STATIC void oal_sdio_detectcard_to_core(int32_t val)
{
    oal_print_hi11xx_log(HI11XX_LOG_DBG, "hi110x mmc detect, vendor id:0x%x  product id:0x%x",
                         HISDIO_VENDOR_ID_HI1103, HISDIO_PRODUCT_ID_HISI);
}
#else
OAL_STATIC void oal_sdio_detectcard_to_core(int32_t val)
{
    oal_print_hi11xx_log(HI11XX_LOG_DBG, "hi110x mmc detect, vendor id:0x%x  product id:0x%x",
                         HISDIO_VENDOR_ID_HI1103, HISDIO_PRODUCT_ID_HISI);
#ifndef _PRE_HI375X_SDIO
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
    dw_mci_sdio_card_detect_change();
#endif
#endif
}
#endif

int32_t oal_sdio_send_msg(hcc_bus *pst_bus, uint32_t val)
{
    int32_t ret = OAL_SUCC;
    struct oal_sdio *hi_sdio = (struct oal_sdio *)pst_bus->data;
    struct sdio_func *func = NULL;

    if (oal_unlikely(hi_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hi_sdio is null", __FUNCTION__);
        return -OAL_EINVAL;
    }

    if (oal_warn_on(hi_sdio->func == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: !hi_sdio->func is null", __FUNCTION__);
        return -OAL_EINVAL;
    }

    func = hi_sdio->func;
    oal_print_hi11xx_log(HI11XX_LOG_DBG, "send msg to notice device [0x%8x]", (uint32_t)val);
    if (val >= H2D_MSG_COUNT) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[Error]invaild param[%u]!", val);
        return -OAL_EINVAL;
    }
    hcc_bus_wake_lock(pst_bus);
    oal_sdio_claim_host(hi_sdio);
    /* sdio message can sent when wifi power on */
    if (hi110x_get_wifi_power_stat() == 0) {
        oam_error_log0(0, OAM_SF_ANY, "{oal_sdio_send_msg::wifi power off,can't send sdio msg!}");
        oal_sdio_release_host(hi_sdio);
        hcc_bus_wake_unlock(pst_bus);
        return -OAL_EBUSY;
    }

    oal_sdio_writel(func, (1 << val),
                    HISDIO_REG_FUNC1_WRITE_MSG, &ret);
    if (ret) {
        oam_error_log2(0, OAM_SF_ANY, "{oal_sdio_send_msg::failed to send sdio msg[%u]!ret=%d}", val, ret);
    }
    oal_sdio_release_host(hi_sdio);
    hcc_bus_wake_unlock(pst_bus);
    return ret;
}

OAL_STATIC int32_t oal_sdio_host_lock(hcc_bus *pst_bus)
{
    struct oal_sdio *hi_sdio = (struct oal_sdio *)pst_bus->data;
    oal_sdio_claim_host(hi_sdio);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_sdio_host_unlock(hcc_bus *pst_bus)
{
    struct oal_sdio *hi_sdio = (struct oal_sdio *)pst_bus->data;
    oal_sdio_release_host(hi_sdio);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_sdio_hcc_bus_power_down(hcc_bus *pst_bus, struct oal_sdio *hi_sdio)
{
    oal_wlan_gpio_intr_enable(hbus_to_dev(pst_bus), OAL_FALSE);
    hcc_disable(hbus_to_hcc(pst_bus), OAL_TRUE);
#ifdef CONFIG_MMC
    /* 下电之前关闭 SDIO HOST 控制器时钟 */
#if (defined _PRE_PLAT_FEATURE_SDIO_REPOWER) || (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
    mmc_power_save_host(hi_sdio->func->card->host);
#endif
#else
    oal_print_hi11xx_log(HI11XX_LOG_ERR, "no mmc_power_save_host after linux4.19");
    return -OAL_EFAIL;
#endif
#endif

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_sdio_hcc_bus_power_patch_lauch(hcc_bus *pst_bus)
{
    /* Patch下载完后 初始化通道资源，然后等待业务初始化完成 */
    oal_wlan_gpio_intr_enable(hbus_to_dev(pst_bus), OAL_TRUE);

    /* 第一个中断有可能在中断使能之前上报，强制调度一次RX Thread */
    up(&pst_bus->rx_sema);

    if (oal_wait_for_completion_timeout(&pst_bus->st_device_ready,
        (uint32_t)oal_msecs_to_jiffies(HOST_WAIT_BOTTOM_INIT_TIMEOUT)) == 0) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "wait first device ready timeout... %d ms ",
                             HOST_WAIT_BOTTOM_INIT_TIMEOUT);
        up(&pst_bus->rx_sema);

        if (oal_wait_for_completion_timeout(&pst_bus->st_device_ready,
            (uint32_t)oal_msecs_to_jiffies(15000)) == 0) { // 15000, 15S for device boot
            oal_print_hi11xx_log(HI11XX_LOG_WARN, "retry 5 second hold, still timeout");
            return -OAL_ETIMEDOUT;
        } else {
            /* 强制调度成功，说明有可能是GPIO中断未响应 */
            oal_print_hi11xx_log(HI11XX_LOG_WARN, KERN_WARNING "[E]retry succ, maybe gpio interrupt issue");
            declare_dft_trace_key_info("sdio gpio int issue", OAL_DFT_TRACE_FAIL);
        }
    }
    hcc_enable(hbus_to_hcc(pst_bus), OAL_TRUE);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_sdio_power_action(hcc_bus *pst_bus, hcc_bus_power_action_type action)
{
    struct oal_sdio *hi_sdio = NULL;
    if (oal_warn_on(pst_bus == NULL)) {
        return -OAL_EINVAL;
    }

    hi_sdio = (struct oal_sdio *)pst_bus->data;
    if (oal_warn_on(hi_sdio == NULL)) {
        return -OAL_ENODEV;
    }

    if (action == HCC_BUS_POWER_DOWN) {
        if (oal_sdio_hcc_bus_power_down(pst_bus, hi_sdio) != OAL_SUCC) {
            return -OAL_EFAIL;
        }
    }

    if (action == HCC_BUS_POWER_PATCH_LOAD_PREPARE) {
        /* close hcc */
        hcc_disable(hbus_to_hcc(pst_bus), OAL_TRUE);
        oal_init_completion(&pst_bus->st_device_ready);
        oal_wlan_gpio_intr_enable(hbus_to_dev(pst_bus), OAL_FALSE);
    }

    if (action == HCC_BUS_POWER_PATCH_LAUCH) {
        if (oal_sdio_hcc_bus_power_patch_lauch(pst_bus) != OAL_SUCC) {
            return -OAL_ETIMEDOUT;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t sdio_dev_init(struct oal_sdio *hi_sdio)
{
    int32_t ret;
    const uint32_t ul_timeout = 1000;
    struct sdio_func *func;

    func = hi_sdio->func;

    oal_sdio_claim_host(hi_sdio);

    func->enable_timeout = ul_timeout;

    ret = sdio_enable_func(func);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to enable sdio function! ret=%d", ret);
    }

    ret = sdio_set_block_size(func, HISDIO_BLOCK_SIZE);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to set sdio blk size! ret=%d", ret);
    }

    /* func 1 enable 之后, device 发的消息会被这里清掉 */
    /*
     * enable four interrupt sources in function 1:
     *      data ready for host to read
     *      read data error
     *      message from arm is available
     *      device has receive message from host
     *
     */
    oal_sdio_writeb(func, HISDIO_FUNC1_INT_MASK, HISDIO_REG_FUNC1_INT_ENABLE, &ret);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to enable sdio interrupt! ret=%d", ret);
    }

    oal_sdio_release_host(hi_sdio);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio function %d enabled.", func->num);

    return ret;
}

OAL_STATIC int32_t oal_sdio_bindcpu(hcc_bus *pst_bus, uint32_t chan, int32_t is_bind)
{
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_sdio_mmc_power_reinit(struct oal_sdio *hi_sdio)
{
    int32_t ret = OAL_SUCC;

#if (defined _PRE_PLAT_FEATURE_SDIO_REPOWER) || (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
    ret = mmc_power_save_host(hi_sdio->func->card->host);
    hi_sdio->func->card->host->pm_flags &= ~MMC_PM_KEEP_POWER;
    ret = mmc_power_restore_host(hi_sdio->func->card->host);
    hi_sdio->func->card->host->pm_flags |= MMC_PM_KEEP_POWER;
#endif
#else
    oal_print_hi11xx_log(HI11XX_LOG_ERR, "no mmc_power_save_host after linux4.19");
    return -OAL_EFAIL;
#endif

    if (ret < OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to mmc_power_restore_host ret=%d", ret);
        (void)ssi_dump_err_regs(SSI_ERR_SDIO_REENUM_FAIL);
        oal_sdio_release_host(hi_sdio);
        chr_exception(chr_wifi_drv(CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_SDIO_INIT));
#ifdef CONFIG_ARCH_HI1103_SDIO_DEBUG
        g_wifi_sdio_fail_count++;
        if (g_wifi_sdio_fail_count >= g_wifi_sdio_fail_painc_limit - 1) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio host ip reset flag=1");
            g_sdio_reset_ip = 1;
        } else if (g_wifi_sdio_fail_count >= g_wifi_sdio_fail_painc_limit) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio cause kernel panic");
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio dev init failed, fail count %u , limit %u",
                                 g_wifi_sdio_fail_count, g_wifi_sdio_fail_painc_limit);
        }
#endif

        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_sdio_dev_reinit(struct oal_sdio *hi_sdio)
{
    int32_t ret = sdio_dev_init(hi_sdio);
    if (ret) {
        (void)ssi_dump_err_regs(SSI_ERR_SDIO_REINIT_FAIL);
        oal_sdio_release_host(hi_sdio);
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio dev reinit failed ret =%d", ret);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_SDIO, CHR_PLAT_DRV_ERROR_SDIO_INIT);
#ifdef CONFIG_ARCH_HI1103_SDIO_DEBUG
        g_wifi_sdio_fail_count++;
        if (g_wifi_sdio_fail_count >= g_wifi_sdio_fail_painc_limit - 1) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio host ip reset flag=1");
            g_sdio_reset_ip = 1;
        }
        if (g_wifi_sdio_fail_count >= g_wifi_sdio_fail_painc_limit) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio cause kernel panic");
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio dev init failed, fail count %u , limit %u",
                                 g_wifi_sdio_fail_count, g_wifi_sdio_fail_painc_limit);
        }
#endif

        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_sdio_reinit(hcc_bus *pst_bus)
{
    int32_t ret;
    ktime_t time_start, time_stop;
    uint64_t trans_us;
    struct oal_sdio *hi_sdio = (struct oal_sdio *)pst_bus->data;

    oal_sdio_claim_host(hi_sdio);
    hcc_bus_disable_state(pst_bus, OAL_BUS_STATE_ALL);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "start to power restore sdio");
    time_start = ktime_get();

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wake_sema_count=%d", pst_bus->sr_wake_sema.count);
    sema_init(&pst_bus->sr_wake_sema, 1); /* S/R信号量 */

    ret = oal_sdio_mmc_power_reinit(hi_sdio);
    if (ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    ret = oal_sdio_dev_reinit(hi_sdio);
    if (ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    /* For sdio mem pg, sd clk 7 cycles mem pwrup cost */
    oal_sdio_wakeup_dev(hi_sdio);

    hcc_bus_enable_state(pst_bus, OAL_BUS_STATE_ALL);
    oal_sdio_release_host(hi_sdio);
    time_stop = ktime_get();
    trans_us = (uint64_t)ktime_to_us(ktime_sub(time_stop, time_start));
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio_dev_init ok cost %llu us", trans_us);

    return OAL_SUCC;
}

/*
 * Prototype    : oal_sdio_rw_buf
 * Description  : read or write buf
 */
OAL_STATIC int32_t oal_sdio_rw_buf(struct oal_sdio *hi_sdio, int32_t rw,
                                   uint32_t addr, uint8_t *buf, uint32_t rw_sz)
{
    struct sdio_func *func = hi_sdio->func;
    int32_t ret = OAL_SUCC;

    /* padding len of buf has been assure when alloc */
    rw_sz = hisdio_align_4_or_blk(rw_sz);
    if (WARN(rw_sz != hisdio_align_4_or_blk(rw_sz), "invaild len %u\n", rw_sz)) {
        /* just for debug, remove later */
        return -OAL_EINVAL;
    }

    oal_sdio_claim_host(hi_sdio);
    if (rw == SDIO_READ) {
        ret = oal_sdio_readsb(func, buf, addr, rw_sz);
    } else if (rw == SDIO_WRITE) {
        ret = oal_sdio_writesb(func, addr, buf, rw_sz);
    }

    oal_sdio_release_host(hi_sdio);

    return ret;
}

OAL_STATIC int32_t oal_sdio_check_rx_len(struct oal_sdio *hi_sdio, uint32_t count)
{
    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_sdio_xfercount_get(struct oal_sdio *hi_sdio, uint32_t *xfercount)
{
    int32_t ret = 0;
#ifdef CONFIG_SDIO_MSG_ACK_HOST2ARM_DEBUG
    /* read from 0x0c */
    *xfercount = oal_sdio_readl(hi_sdio->func, HISDIO_REG_FUNC1_XFER_COUNT, &ret);
    if (oal_unlikely(ret)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[ERROR]sdio read single package len failed ret=%d", ret);
        return ret;
    }
    hi_sdio->sdio_extend->xfer_count = *xfercount;
#else
    if (g_sdio_extend_func) {
        *xfercount = hi_sdio->sdio_extend->xfer_count;
        return OAL_SUCC;
    }

    /* read from 0x0c */
    *xfercount = oal_sdio_readl(hi_sdio->func, HISDIO_REG_FUNC1_XFER_COUNT, &ret);
    if (oal_unlikely(ret)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[E]sdio read xercount failed ret=%d", ret);
        declare_dft_trace_key_info("sdio readl 0x0c fail", OAL_DFT_TRACE_FAIL);
        return ret;
    }
    hi_sdio->sdio_extend->xfer_count = *xfercount;
#endif
    return OAL_SUCC;
}

/*
 * Prototype    : oal_sdio_data_sg_irq(struct oal_sdio *hi_sdio)
 * Description  : rx data from sdio, Just support single transfer
 */
OAL_STATIC int32_t oal_sdio_data_sg_irq(struct oal_sdio *hi_sdio)
{
    struct sdio_func *func = NULL;
    int32_t ret;
    uint32_t xfer_count;

    if (oal_unlikely(!hi_sdio)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hi_sdio is null", __FUNCTION__);
        return -OAL_EFAIL;
    };

    if (oal_unlikely(!hi_sdio->func)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: !hi_sdio->func is null", __FUNCTION__);
        return -OAL_EFAIL;
    };

    func = hi_sdio->func;

    ret = oal_sdio_xfercount_get(hi_sdio, &xfer_count);
    if (oal_unlikely(ret)) {
        return -OAL_EFAIL;
    }

    if (oal_unlikely(oal_sdio_check_rx_len(hi_sdio, xfer_count) != OAL_SUCC)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[Error]Sdio Rx Single Transfer len[%u] invalid", xfer_count);
    }

    /* beacuse get buf may cost lot of time, so release bus first */
    if (oal_unlikely(hi_sdio->pst_bus->bus_ops.rx == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: !hi_sdio->pst_bus->bus_ops.rx is null", __FUNCTION__);
        return -OAL_EFAIL;
    }

    oal_sdio_release_host(hi_sdio);
    hi_sdio->pst_bus->bus_ops.rx(hi_sdio->pst_bus->bus_ops_data);
    oal_sdio_claim_host(hi_sdio);

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_sdio_msg_stat(struct oal_sdio *hi_sdio, uint32_t *msg)
{
    int32_t ret = 0;
#ifdef CONFIG_SDIO_MSG_ACK_HOST2ARM_DEBUG
    /* read from old register */
#ifdef CONFIG_SDIO_D2H_MSG_ACK
    *msg = oal_sdio_readl(hi_sdio->func, HISDIO_REG_FUNC1_MSG_FROM_DEV, &ret);
#else
    *msg = oal_sdio_readb(hi_sdio->func, HISDIO_REG_FUNC1_MSG_FROM_DEV, &ret);
#endif

    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio readb error![ret=%d]", ret);
        return ret;
    }
    hi_sdio->sdio_extend->msg_stat = *msg;
#else
    if (g_sdio_extend_func) {
        *msg = hi_sdio->sdio_extend->msg_stat;
    }

    if (*msg == 0) {
        /* no sdio message! */
        return OAL_SUCC;
    }
#ifdef CONFIG_SDIO_D2H_MSG_ACK
    /* read from old register */
    /* 当使用0x30寄存器时需要下发CMD52读0x2B 才会产生HOST2ARM ACK中断 */
    (void)oal_sdio_readb(hi_sdio->func, HISDIO_REG_FUNC1_MSG_HIGH_FROM_DEV, &ret);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[E]sdio readb error![ret=%d]", ret);
        declare_dft_trace_key_info("sdio readl 0x2b fail", OAL_DFT_TRACE_FAIL);
    }
#endif
#endif
    return OAL_SUCC;
}

/*
 * Prototype    : oal_sdio_msg_irq
 * Description  : msg irq
 */
OAL_STATIC int32_t oal_sdio_msg_irq(struct oal_sdio *hi_sdio)
{
    int32_t bit = 0;
    struct sdio_func *func = NULL;
    uint32_t msg = 0;
    int32_t ret;
    unsigned long msg64;

    func = hi_sdio->func;

    /* reading interrupt form ARM Gerneral Purpose Register(0x28)  */
    ret = oal_sdio_msg_stat(hi_sdio, &msg);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "oal_sdio_msg_stat error![ret=%d]", ret);
        return ret;
    }
    msg64 = (unsigned long)msg;

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "sdio message:0x%8x ", msg);

    if (!msg) {
        return OAL_SUCC;
    }

#ifdef CONFIG_SDIO_D2H_MSG_ACK
    if (!!((1 << D2H_MSG_FLOWCTRL_OFF) & msg) & !!((1 << D2H_MSG_FLOWCTRL_ON) & msg)) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "[ERROR]on/off should come at the same time!");
        oal_warn_on(1);
    }
#endif
    if (test_bit(D2H_MSG_DEVICE_PANIC, &msg64)) {
        /* Close sdio. */
        hcc_bus_disable_state(hi_sdio->pst_bus, OAL_BUS_STATE_ALL);
    }

    oal_sdio_release_host(hi_sdio);
    hcc_bus_rx_transfer_unlock(hi_sdio->pst_bus);

    /* 优先处理Panic消息 */
    if (test_and_clear_bit(D2H_MSG_DEVICE_PANIC, &msg64)) {
        bit = D2H_MSG_DEVICE_PANIC;
        hi_sdio->pst_bus->msg[bit].count++;
        hi_sdio->pst_bus->last_msg = bit;
        hi_sdio->pst_bus->msg[bit].cpu_time = cpu_clock(UINT_MAX);
        if (hi_sdio->pst_bus->msg[bit].msg_rx) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "device panic msg come, 0x%8x", msg);
            hi_sdio->pst_bus->msg[bit].msg_rx(hi_sdio->pst_bus->msg[bit].data);
        }
    }

    bit = 0;
    /* 表示以第一个参数为迭代位下标，遍历第二个参数地址中低15bit中所有置1的位 */
    for_each_set_bit(bit, (const unsigned long *)&msg64, 15)
    {
        if (oal_unlikely(bit > 15)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: bit:%d >  5", __FUNCTION__, bit);
            continue;
        };
        hi_sdio->pst_bus->msg[bit].count++;
        hi_sdio->pst_bus->last_msg = bit;
        hi_sdio->pst_bus->msg[bit].cpu_time = cpu_clock(UINT_MAX);
        if (hi_sdio->pst_bus->msg[bit].msg_rx) {
            hi_sdio->pst_bus->msg[bit].msg_rx(hi_sdio->pst_bus->msg[bit].data);
        }
    }

    hcc_bus_rx_transfer_lock(hi_sdio->pst_bus);
    oal_sdio_claim_host(hi_sdio);

    return OAL_SUCC;
}

#ifdef CONFIG_SDIO_DEBUG
OAL_STATIC uint32_t oal_sdio_credit_info_update(struct oal_sdio *hi_sdio)
{
    uint8_t short_free_cnt, large_free_cnt;
    uint32_t ret = 0;
    oal_spin_lock(&hi_sdio->sdio_credit_info.credit_lock);

    short_free_cnt = hisdio_short_pkt_get(hi_sdio->sdio_extend->credit_info);
    large_free_cnt = hisdio_large_pkt_get(hi_sdio->sdio_extend->credit_info);

    if (hi_sdio->sdio_credit_info.short_free_cnt != short_free_cnt) {
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "short free cnt:%d ==> %d\r",
                             hi_sdio->sdio_credit_info.short_free_cnt, short_free_cnt);
        hi_sdio->sdio_credit_info.short_free_cnt = short_free_cnt;
        ret = 1;
    }

    if (hi_sdio->sdio_credit_info.large_free_cnt != large_free_cnt) {
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "large free cnt:%d ==> %d\r",
                             hi_sdio->sdio_credit_info.large_free_cnt, large_free_cnt);
        hi_sdio->sdio_credit_info.large_free_cnt = large_free_cnt;
        ret = 1;
    }

    oal_spin_unlock(&hi_sdio->sdio_credit_info.credit_lock);

    return ret;
}
#endif

void oal_sdio_credit_update_cb_register(struct oal_sdio *hi_sdio, hisdio_rx cb)
{
    if (oal_warn_on(hi_sdio->credit_update_cb != NULL)) {
        return;
    }
    hi_sdio->credit_update_cb = cb;
    return;
}

OAL_STATIC int32_t oal_sdio_extend_buf_get(struct oal_sdio *hi_sdio)
{
    int32_t ret = OAL_SUCC;
    if (g_sdio_extend_func) {
        ret = oal_sdio_memcpy_fromio(hi_sdio->func, (void *)hi_sdio->sdio_extend,
                                     HISDIO_EXTEND_BASE_ADDR, sizeof(struct hisdio_extend_func));
        if (oal_likely(ret == OAL_SUCC)) {
#ifdef CONFIG_SDIO_DEBUG
            printk(KERN_INFO "=========extend buff:%d=====\n",
                   hisdio_comm_reg_seq_get(hi_sdio->sdio_extend->credit_info));
            oal_print_hex_dump((void *)hi_sdio->sdio_extend, sizeof(struct hisdio_extend_func),
                               HEX_DUMP_GROUP_SIZE, "extend :");

            /* 此credit更新只在调试时使用 */
            if (oal_sdio_credit_info_update(hi_sdio)) {
                if (oal_likely(hi_sdio->credit_update_cb)) {
                    hi_sdio->credit_update_cb(hi_sdio->pst_bus->bus_ops_data);
                }
            }
#endif
        } else {
            declare_dft_trace_key_info("sdio read extend_buf fail", OAL_DFT_TRACE_FAIL);
        }
    }

    return ret;
}

OAL_STATIC int32_t oal_sdio_transfer_rx_reserved_buff(struct oal_sdio *hi_sdio)
{
    int32_t i;
    int32_t ret;
    int32_t left_size;
    uint32_t seg_nums, seg_size;
    struct scatterlist *sg = NULL;
    struct scatterlist *sg_t = NULL;
    uint32_t ul_extend_len = hi_sdio->sdio_extend->xfer_count;

    if (ul_extend_len == 0) {
        declare_dft_trace_key_info("extend_len is zero", OAL_DFT_TRACE_EXCEP);
        return -OAL_EINVAL;
    }

    seg_size = hi_sdio->rx_reserved_buff_len;

    seg_nums = ((ul_extend_len - 1) / seg_size) + 1;
    if (hi_sdio->scatt_info[SDIO_READ].max_scatt_num < seg_nums) {
        declare_dft_trace_key_info("rx_seserved_scatt_fail", OAL_DFT_TRACE_EXCEP);
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio seg nums :%u large than rx scatt num %u", seg_nums,
                             hi_sdio->scatt_info[SDIO_READ].max_scatt_num);
        return -OAL_EINVAL;
    }

    oal_print_hi11xx_log(HI11XX_LOG_WARN, "drop the rx buff length:%u", ul_extend_len);

    sg = hi_sdio->scatt_info[SDIO_READ].sglist;
    sg_init_table(sg, seg_nums);
    left_size = ul_extend_len;
    for_each_sg(sg, sg_t, seg_nums, i)
    {
        if (oal_unlikely(sg_t == NULL)) {
            oal_print_hi11xx_log(HI11XX_LOG_WARN, "sg_t==null");
            return -OAL_EINVAL;
        }
        sg_set_buf(sg_t, hi_sdio->rx_reserved_buff, oal_min(seg_size, left_size));
        left_size = left_size - seg_size;
    }
    ret = _oal_sdio_transfer_scatt(hi_sdio, SDIO_READ, HISDIO_REG_FUNC1_FIFO, sg, seg_nums, ul_extend_len);
    if (oal_unlikely(ret)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio trans revered mem failed! ret=%d", ret);
    }
    return ret;
}

#undef CONFIG_SDIO_RX_NETBUF_ALLOC_FAILED_DEBUG
#ifdef CONFIG_SDIO_RX_NETBUF_ALLOC_FAILED_DEBUG
    OAL_STATIC uint32_t g_rx_alloc_netbuf_debug = 0;
#endif

OAL_STATIC oal_netbuf_stru *oal_sdio_alloc_rx_netbuf(uint32_t ul_len)
{
#ifdef CONFIG_SDIO_RX_NETBUF_ALLOC_FAILED_DEBUG
    if (g_rx_alloc_netbuf_debug) {
        if (prandom_u32() % 256) { /* 获得256以内的一个随机值 */
            return NULL;
        }
    }
#endif
    return __netdev_alloc_skb(NULL, ul_len, GFP_KERNEL);
}

OAL_STATIC int32_t oal_sdio_build_rx_netbuf_list(struct oal_sdio *hi_sdio, oal_netbuf_head_stru *head)
{
#ifdef CONFIG_SDIO_FUNC_EXTEND
    int32_t i;
    uint8_t buff_len;
    uint16_t buff_len_t;
#endif
    int32_t ret = OAL_SUCC;
    uint32_t sum_len = 0;
    oal_netbuf_stru *netbuf = NULL;

    /* always should be empty */
    if (oal_unlikely(!oal_netbuf_list_empty(head))) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: head is not null", __FUNCTION__);
        return -OAL_EINVAL;
    }

#ifdef CONFIG_SDIO_FUNC_EXTEND
    for (i = 0; i < HISDIO_EXTEND_REG_COUNT; i++) {
        buff_len = hi_sdio->sdio_extend->comm_reg[i];
        if (buff_len == 0) {
            break;
        }

        buff_len_t = buff_len << HISDIO_D2H_SCATT_BUFFLEN_ALIGN_BITS;

        netbuf = oal_sdio_alloc_rx_netbuf(buff_len_t);
        if (netbuf == NULL) {
            declare_dft_trace_key_info("sdio_rx_no_mem", OAL_DFT_TRACE_OTHER);
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "rx no mem:%u, index:%d", buff_len, i);
            goto failed_netbuf_alloc;
        }

        oal_netbuf_put(netbuf, buff_len_t);
        sum_len += buff_len_t;
        if (oal_unlikely(!oal_netbuf_head_next(head))) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: head next is null", __FUNCTION__);
            return -OAL_EINVAL;
        }
        if (oal_unlikely(!oal_netbuf_head_prev(head))) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: head prev is null", __FUNCTION__);
            return -OAL_EINVAL;
        }
        __skb_queue_tail(head, netbuf);
    }

    if (oal_warn_on(hisdio_align_4_or_blk(sum_len) != hi_sdio->sdio_extend->xfer_count)) {
        declare_dft_trace_key_info("rx_scatt_len_not_match", OAL_DFT_TRACE_EXCEP);
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "scatt total len[%u] should = xfercount[%u],after pad len:%u",
                             sum_len,
                             hi_sdio->sdio_extend->xfer_count,
                             hisdio_align_4_or_blk(sum_len));

        hi_sdio->error_stat.rx_scatt_info_not_match++;
        goto failed_netbuf_alloc;
    }
#else
    netbuf = oal_sdio_alloc_rx_netbuf(hi_sdio->sdio_extend->xfer_count);
    if (netbuf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "rx no mem:%u", hi_sdio->sdio_extend->xfer_count);
        declare_dft_trace_key_info("sdio_rx_no_mem", OAL_DFT_TRACE_OTHER);
        goto failed_netbuf_alloc;
    }

    oal_netbuf_put(netbuf, hi_sdio->sdio_extend->xfer_count);
    sum_len += hi_sdio->sdio_extend->xfer_count;
    __skb_queue_tail(head, netbuf);
#endif

    if (oal_unlikely(oal_netbuf_list_empty(head))) {
#ifdef CONFIG_PRINTK
        printk("unvaild scatt info:xfercount:%u\n", hi_sdio->sdio_extend->xfer_count);
        print_hex_dump_bytes("scatt extend:", DUMP_PREFIX_ADDRESS,
                             hi_sdio->sdio_extend->comm_reg, HISDIO_EXTEND_REG_COUNT);
#endif
        return -OAL_EINVAL;
    }

    return ret;
failed_netbuf_alloc:
    skb_queue_purge(head);
    oal_sdio_transfer_rx_reserved_buff(hi_sdio);
    return -OAL_ENOMEM;
}

OAL_STATIC OAL_INLINE int32_t oal_sdio_get_func1_int_status(struct oal_sdio *hi_sdio, uint8_t *int_stat)
{
    int32_t ret = 0;
#ifdef CONFIG_SDIO_MSG_ACK_HOST2ARM_DEBUG
    /* read interrupt indicator register */
    *int_stat = oal_sdio_readb(hi_sdio->func, HISDIO_REG_FUNC1_INT_STATUS, &ret);
    if (oal_unlikely(ret)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to read sdio func1 interrupt status!ret=%d\n", ret);
        return ret;
    }
    hi_sdio->sdio_extend->int_stat &= hi_sdio->func1_int_mask;
    hi_sdio->sdio_extend->int_stat = *int_stat;
#else
    if (g_sdio_extend_func) {
        hi_sdio->sdio_extend->int_stat &= hi_sdio->func1_int_mask;
        *int_stat = (hi_sdio->sdio_extend->int_stat & 0xF);
        return OAL_SUCC;
    } else {
        /* read interrupt indicator register */
        *int_stat = oal_sdio_readb(hi_sdio->func, HISDIO_REG_FUNC1_INT_STATUS, &ret);
        if (oal_unlikely(ret)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to read sdio func1 interrupt status!ret=%d", ret);
            return ret;
        }
        *int_stat = (*int_stat) & hi_sdio->func1_int_mask;
    }
#endif
    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_sdio_clear_int_status(struct oal_sdio *hi_sdio, uint8_t int_stat)
{
    int32_t ret = 0;
#ifdef CONFIG_SDIO_MSG_ACK_HOST2ARM_DEBUG
    /* clear interrupt mask */
    oal_sdio_writeb(hi_sdio->func, int_stat, HISDIO_REG_FUNC1_INT_STATUS, &ret);
    if (oal_unlikely(ret)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "faild to clear sdio func1 interrupt!ret=%d", ret);
        return ret;
    }
#else

    if (g_sdio_extend_func) {
        return OAL_SUCC;
    }

    /* clear interrupt mask */
    oal_sdio_writeb(hi_sdio->func, int_stat, HISDIO_REG_FUNC1_INT_STATUS, &ret);
    if (oal_unlikely(ret)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "faild to clear sdio func1 interrupt!ret=%d", ret);
        return ret;
    }
#endif
    return OAL_SUCC;
}

/*
 * Prototype    : oal_sdio_do_isr
 * Description  : sdio rx data
 */
OAL_STATIC int32_t oal_sdio_do_isr(struct oal_sdio *hi_sdio)
{
    uint8_t int_mask;
    int32_t ret;
    struct sdio_func *func;

    func = hi_sdio->func;

    /* sdio bus state access lock by sdio bus claim locked. */
    if (oal_unlikely(OAL_TRUE != oal_sdio_get_state(hi_sdio->pst_bus, OAL_BUS_STATE_RX))) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio closed,state:%u",
                             oal_sdio_get_state(hi_sdio->pst_bus, OAL_BUS_STATE_RX));
        return OAL_SUCC;
    }

#ifndef CONFIG_SDIO_MSG_ACK_HOST2ARM_DEBUG
    ret = oal_sdio_extend_buf_get(hi_sdio);
    if (oal_unlikely(ret)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to read sdio extend area ret=%d", ret);
        return -OAL_EFAIL;
    }
#endif

    ret = oal_sdio_get_func1_int_status(hi_sdio, &int_mask);
    if (oal_unlikely(ret)) {
        return ret;
    }

    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "handle sdio interrupt mask:%d", int_mask);

    if (oal_unlikely((int_mask & HISDIO_FUNC1_INT_MASK) == 0)) {
        oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "no sdio interrupt occur[%u], unavailable %s interrupt",
                             int_mask,
                             g_hisdio_intr_mode ? "gpio" : "sdio");
        hi_sdio->func1_stat.func1_no_int_count++;
        return OAL_SUCC;
    }

    hi_sdio->sdio_int_count++;

    /* clear interrupt mask */
    ret = oal_sdio_clear_int_status(hi_sdio, int_mask);
    if (oal_unlikely(ret)) {
        return ret;
    }

    if (int_mask & HISDIO_FUNC1_INT_RERROR) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio func1 interrupt Error, try to read the data anyway");
        hi_sdio->func1_stat.func1_err_int_count++;
    }

    /* message interrupt, flow control */
    if (int_mask & HISDIO_FUNC1_INT_MFARM) {
        hi_sdio->func1_stat.func1_msg_int_count++;
        if (oal_sdio_msg_irq(hi_sdio) != OAL_SUCC) {
            return -OAL_EFAIL;
        }
    }

    if (int_mask & HISDIO_FUNC1_INT_DREADY) {
#ifdef CONFIG_SDIO_MSG_ACK_HOST2ARM_DEBUG
        ret = oal_sdio_extend_buf_get(hi_sdio);
        if (oal_unlikely(ret)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to read sdio extend area ret=%d", ret);
            return -OAL_EFAIL;
        }
#endif
        hi_sdio->func1_stat.func1_data_int_count++;
        return oal_sdio_data_sg_irq(hi_sdio);
    }

    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "succeed to handle sdio irq");
    return OAL_SUCC;
}

/*
 * Prototype    : oal_sdio_isr
 * Description  : sdio interrupt routine
 */
OAL_STATIC void oal_sdio_isr(struct sdio_func *func)
{
    struct oal_sdio *hi_sdio = NULL;
    int32_t ret;
    uint32_t weight = SDIO_MAX_CONTINUS_RX_COUNT;

    if (oal_unlikely(!func)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: func is null", __FUNCTION__);
        return;
    }

    hi_sdio = sdio_get_drvdata(func);
    if (hi_sdio == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "oal_sdio is NULL");
        return;
    }

    do {
        oal_sdio_release_host(hi_sdio);
        oal_sdio_claim_host(hi_sdio);
        ret = oal_sdio_do_isr(hi_sdio);
        if (oal_unlikely(ret)) {
            hcc_bus_exception_submit(hi_sdio->pst_bus, WIFI_TRANS_FAIL);

            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_SDIO, CHR_PLAT_DRV_ERROR_SDIO_INIT_ISR);
        }
        oal_sdio_release_host(hi_sdio);
        oal_sdio_claim_host(hi_sdio);
    } while ((ret == OAL_SUCC) && (--weight > 0));
}

OAL_STATIC int32_t oal_sdio_rxdata_proc(struct oal_sdio *hi_sdio)
{
    struct sdio_func *func = NULL;
    int32_t ret;

    if (oal_warn_on(hi_sdio == NULL)) {
        return -OAL_EINVAL;
    }

    if (oal_warn_on(!hi_sdio)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hi_sdio is null", __FUNCTION__);
        return -OAL_EINVAL;
    }

    func = hi_sdio->func;

    oal_sdio_claim_host(hi_sdio);
    oal_wake_lock(&hi_sdio->st_sdio_rx_wakelock);
    ret = oal_sdio_do_isr(hi_sdio);
    oal_wake_unlock(&hi_sdio->st_sdio_rx_wakelock);
    if (oal_unlikely(ret)) {
        hcc_bus_exception_submit(hi_sdio->pst_bus, WIFI_TRANS_FAIL);

        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_SDIO, CHR_PLAT_DRV_ERROR_SDIO_INIT_RX_DATA_PROC);
    }
    oal_sdio_release_host(hi_sdio);

    return OAL_SUCC;
}

/*
 * Prototype    : oal_sdio_interrupt_register
 * Description  : register interrupt
 */
OAL_STATIC int32_t oal_sdio_interrupt_register(struct oal_sdio *hi_sdio)
{
    int32_t ret;

    if (!g_hisdio_intr_mode) {
        oal_sdio_claim_host(hi_sdio);
        /* use sdio bus line data1 for sdio data interrupt */
        ret = sdio_claim_irq(hi_sdio->func, oal_sdio_isr);
        if (ret < 0) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to register sdio interrupt");
            oal_sdio_release_host(hi_sdio);
            return -OAL_EFAIL;
        }
        oal_sdio_release_host(hi_sdio);
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio interrupt register!");
        pm_runtime_get_sync(mmc_dev(hi_sdio->func->card->host)); /* 防止sd_clk动态开关 */
    }

    return OAL_SUCC;
}

/*
 * Prototype    : oal_sdio_interrupt_unregister
 * Description  : unregister interrupt
 */
OAL_STATIC void oal_sdio_interrupt_unregister(struct oal_sdio *hi_sdio)
{
    if (g_hisdio_intr_mode) {
        /* use GPIO interrupt for sdio data interrupt */
    } else {
        oal_sdio_claim_host(hi_sdio);
        /* use sdio bus line data1 for sdio data interrupt */
        sdio_release_irq(hi_sdio->func);
        oal_sdio_release_host(hi_sdio);
        pm_runtime_put_sync(mmc_dev(hi_sdio->func->card->host));
    }
}

OAL_STATIC int32_t oal_sdio_get_sleep_reg(struct oal_sdio *hi_sdio)
{
    int ret;
    int32_t ul_value;

    oal_sdio_claim_host(hi_sdio);
    ul_value = (int32_t)(uint32_t)sdio_f0_readb(hi_sdio->func, HISDIO_WAKEUP_DEV_REG, &ret);
    oal_sdio_release_host(hi_sdio);

    if (oal_unlikely(ret)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "read func0 reg 0x%x failed, ret=%d", HISDIO_WAKEUP_DEV_REG, ret);
    }

    return ul_value;
}

/*
 * Prototype    : oal_sdio_wakeup_dev
 * Description  : wakeup device
 */
OAL_STATIC int32_t oal_sdio_wakeup_dev(struct oal_sdio *hi_sdio)
{
    int ret;

    oal_sdio_claim_host(hi_sdio);
    sdio_f0_writeb(hi_sdio->func, DISALLOW_TO_SLEEP_VALUE, HISDIO_WAKEUP_DEV_REG, &ret);
    oal_sdio_release_host(hi_sdio);

    return ret;
}

/*
 * Prototype    : oal_sdio_sleep_dev
 * Description  : allow device to sleep
 */
OAL_STATIC int32_t oal_sdio_sleep_dev(struct oal_sdio *hi_sdio)
{
    int ret;

    oal_sdio_claim_host(hi_sdio);
    sdio_f0_writeb(hi_sdio->func, ALLOW_TO_SLEEP_VALUE, HISDIO_WAKEUP_DEV_REG, &ret);
    oal_sdio_release_host(hi_sdio);

    return ret;
}

OAL_STATIC int32_t oal_sdio_sleep_request(hcc_bus *pst_bus)
{
    struct oal_sdio *hi_sdio = (struct oal_sdio *)pst_bus->data;
    if (oal_warn_on(hi_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hi_sdio is null");
        return -OAL_EINVAL;
    }
#ifdef _PRE_PLAT_FEATURE_HI110X_SDIO_GPIO_WAKE
    hcc_bus_disable_state(pst_bus, OAL_BUS_STATE_ALL);
    board_host_wakeup_dev_set(0);
    oal_usleep(WLAN_WAKEUP_DEV_EVENT_DELAY_US);
#endif
    return oal_sdio_sleep_dev(hi_sdio);
}

OAL_STATIC int32_t oal_sdio_sleep_request_host(hcc_bus *pst_bus)
{
    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE void oal_sdio_func1_int_mask(struct oal_sdio *hi_sdio, uint32_t func1_int_mask)
{
    if (oal_warn_on(hi_sdio == NULL)) {
        return;
    }
    oal_sdio_claim_host(hi_sdio);
    hi_sdio->func1_int_mask &= ~func1_int_mask;
    oal_sdio_release_host(hi_sdio);
}

OAL_STATIC OAL_INLINE void oal_sdio_func1_int_unmask(struct oal_sdio *hi_sdio, uint32_t func1_int_mask)
{
    if (oal_warn_on(hi_sdio == NULL)) {
        return;
    }
    oal_sdio_claim_host(hi_sdio);
    hi_sdio->func1_int_mask |= func1_int_mask;
    oal_sdio_release_host(hi_sdio);
}

OAL_STATIC int32_t oal_sdio_wakeup_request(hcc_bus *pst_bus)
{
    struct oal_sdio *hi_sdio = (struct oal_sdio *)pst_bus->data;
    if (oal_warn_on(hi_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hi_sdio is null");
        return -OAL_EINVAL;
    }
    return oal_sdio_wakeup_dev(hi_sdio);
}

OAL_STATIC int32_t oal_sdio_get_sleep_state(hcc_bus *pst_bus)
{
    struct oal_sdio *hi_sdio = (struct oal_sdio *)pst_bus->data;
    if (oal_warn_on(hi_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hi_sdio is null");
        return -OAL_EINVAL;
    }

    return oal_sdio_get_sleep_reg(hi_sdio);
}

OAL_STATIC int32_t oal_sdio_rx_int_mask(hcc_bus *pst_bus, int32_t is_mask)
{
    struct oal_sdio *hi_sdio = (struct oal_sdio *)pst_bus->data;
    if (oal_warn_on(hi_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hi_sdio is null");
        return -OAL_EINVAL;
    }
    if (is_mask) {
        oal_sdio_func1_int_mask(hi_sdio, HISDIO_FUNC1_INT_DREADY);
    } else {
        oal_sdio_func1_int_unmask(hi_sdio, HISDIO_FUNC1_INT_DREADY);
    }
    return OAL_SUCC;
}

/*
 * Prototype    : oal_sdio_writesb_by_heap
 * Description  : before sdio send, copy buf to heap memory
 */
#ifdef CONFIG_VMAP_STACK
OAL_STATIC int32_t oal_sdio_writesb_by_heap(struct sdio_func *func, uint8_t *buf, uint32_t len)
{
    int32_t ret;
    uint8_t *send_buf = NULL;

    if (len <= 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "get invalid malloc len %d\n", len);
        return -OAL_EINVAL;
    }

    send_buf = (uint8_t *)kmalloc(len, GFP_KERNEL);
    if (send_buf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "malloc failed, len %d\n", len);
        return -OAL_ENOMEM;
    }

    if (memcpy_s(send_buf, len, buf, len) != EOK) {
        kfree(send_buf);
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "memcpy_s failed\n");
        return -OAL_EINVAL;
    }

    ret = oal_sdio_writesb(func, HISDIO_REG_FUNC1_FIFO, send_buf, len);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "write error:%d\n", ret);
    }

    kfree(send_buf);

    return ret;
}
#endif

/*
 * Prototype    : sdio_patch_writesb
 * Description  : provide interface for pm driver
 */
OAL_STATIC int32_t oal_sdio_patch_writesb(struct oal_sdio *pst_sdio, uint8_t *buf, uint32_t len)
{
#ifdef CONFIG_MMC
    int32_t ret;

    struct sdio_func *func = pst_sdio->func;

    if (func == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "Sdio device is NOT initialized");
        return -OAL_EIO;
    }

    if (buf == NULL || len == 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "Write buf is NULL");
        return -OAL_EINVAL;
    }

    len = hisdio_align_4_or_blk(len);

    oal_sdio_claim_host(pst_sdio);
#ifdef CONFIG_VMAP_STACK
    ret = oal_sdio_writesb_by_heap(func, buf, len);
#else
    ret = oal_sdio_writesb(func, HISDIO_REG_FUNC1_FIFO, buf, len);
#endif
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "oal_sdio_writesb error:%d", ret);
    }
    oal_sdio_release_host(pst_sdio);

    return ret;
#else
    return 0;
#endif
}

/*
 * Prototype    : sdio_patch_readsb
 * Description  : provide interface for pm driver
 */
OAL_STATIC int32_t oal_sdio_patch_readsb(struct oal_sdio *pst_sdio, uint8_t *buf,
                                         uint32_t len, uint32_t timeout)
{
#ifdef CONFIG_MMC
    uint8_t int_mask;
    uint8_t *ver_info = NULL;
    int32_t ret = 0;
    unsigned long timeout_jiffies;
    uint32_t xfer_count;
    int32_t i;

    struct sdio_func *func = pst_sdio->func;

    if (func == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio device is NOT initialized");

        return -OAL_EIO;
    }

    if (buf == NULL || len == 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "Invalid NULL read buf");

        return -OAL_EINVAL;
    }

    oal_sdio_claim_host(pst_sdio);
    timeout_jiffies = jiffies + msecs_to_jiffies(timeout);
    forever_loop() {
        int_mask = oal_sdio_readb(func, HISDIO_REG_FUNC1_INT_STATUS, &ret);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "read int mask fail, ret=%d", ret);
            oal_sdio_release_host(pst_sdio);
            return -OAL_EFAIL;
        }

        if (int_mask & HISDIO_FUNC1_INT_MASK) {
            /* sdio int came */
            break;
        }

        if (time_after(jiffies, timeout_jiffies)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "read int mask timeout, int_mask=%x", int_mask);
            oal_sdio_release_host(pst_sdio);
            return -OAL_ETIMEDOUT;
        }
        cpu_relax();
    }

    oal_sdio_writeb(func, int_mask, HISDIO_REG_FUNC1_INT_STATUS, &ret);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "clear int mask error:%d", ret);
        oal_sdio_release_host(pst_sdio);
        return -OAL_EFAIL;
    }

    xfer_count = oal_sdio_readl(func, HISDIO_REG_FUNC1_XFER_COUNT, &ret);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "read xfer_count err:%d", ret);
        oal_sdio_release_host(pst_sdio);
        return -OAL_EFAIL;
    }

    if (xfer_count < len) {
        len = xfer_count;
    }

    ver_info = kzalloc((xfer_count + 1), GFP_KERNEL);
    if (ver_info == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "oal_sdio_patch_readsb alloc %d failed", xfer_count + 1);
        oal_sdio_release_host(pst_sdio);
        return -OAL_ENOMEM;
    }

    /* kzalloc had memset */
    ret = oal_sdio_readsb(func, ver_info, HISDIO_REG_FUNC1_FIFO, xfer_count);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hsdio_readsb error:ret=%d", ret);
    } else {
        for (i = 0; i < len; i++) {
            buf[i] = ver_info[i];
        }
    }

    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "=====sdio read:[ret=%d]", ret);

    kfree(ver_info);

    oal_sdio_release_host(pst_sdio);

    return (ret ? -OAL_EFAIL : xfer_count);
#else
    return -1;
#endif
}

OAL_STATIC int32_t oal_sdio_patch_read(hcc_bus *pst_bus, uint8_t *buff, int32_t len, uint32_t timeout)
{
    struct oal_sdio *hi_sdio = (struct oal_sdio *)pst_bus->data;
    if (oal_warn_on(hi_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hi_sdio is null");
        return -OAL_EINVAL;
    }
    return oal_sdio_patch_readsb(hi_sdio, buff, len, timeout);
}

OAL_STATIC int32_t oal_sdio_patch_write(hcc_bus *pst_bus, uint8_t *buff, int32_t len)
{
    struct oal_sdio *hi_sdio = (struct oal_sdio *)pst_bus->data;
    if (oal_warn_on(hi_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hi_sdio is null");
        return -OAL_EINVAL;
    }
    return oal_sdio_patch_writesb(hi_sdio, buff, len);
}

/*
 * Prototype    : oal_sdio_dev_init
 * Description  : init sdio function
 * Input        : adapter   oal_sdio handler
 * Return Value : succ or fail
 */
OAL_STATIC int32_t oal_sdio_dev_init(struct oal_sdio *hi_sdio)
{
    struct sdio_func *func = NULL;
    int32_t ret;
    const uint32_t ul_timeout = 1000;

    if (oal_warn_on(hi_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hi_sdio is null", __FUNCTION__);
        return -OAL_EINVAL;
    }

    func = hi_sdio->func;

    oal_sdio_claim_host(hi_sdio);

    func->enable_timeout = ul_timeout;

    ret = sdio_enable_func(func);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to enable sdio function! ret=%d", ret);
        goto failed_enabe_func;
    }

    ret = sdio_set_block_size(func, HISDIO_BLOCK_SIZE);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to set sdio blk size! ret=%d", ret);
        goto failed_set_block_size;
    }

    /* before enable sdio function 1, clear its interrupt flag, no matter it exist or not */
    oal_sdio_writeb(func, HISDIO_FUNC1_INT_MASK, HISDIO_REG_FUNC1_INT_STATUS, &ret);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to clear sdio interrupt! ret=%d", ret);
        goto failed_clear_func1_int;
    }

    /*
     * enable four interrupt sources in function 1:
     *      data ready for host to read
     *      read data error
     *      message from arm is available
     *      device has receive message from host
     */
    oal_sdio_writeb(func, HISDIO_FUNC1_INT_MASK, HISDIO_REG_FUNC1_INT_ENABLE, &ret);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to enable sdio interrupt! ret=%d", ret);
        goto failed_enable_func1;
    }

    oal_sdio_release_host(hi_sdio);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio function %d enabled.", func->num);
#ifdef _PRE_COMMENT_CODE_
    if (!hi_sdio->func->card->cccr.multi_block) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "[Error]did't support muti block transfer! ");
    }
#endif

    return OAL_SUCC;

failed_enable_func1:
failed_clear_func1_int:
failed_set_block_size:
    sdio_disable_func(func);
failed_enabe_func:
    oal_sdio_release_host(hi_sdio);
    return ret;
}

/*
 * Prototype    : oal_sdio_deinit
 * Description  : deinit
 */
OAL_STATIC void oal_sdio_dev_deinit(struct oal_sdio *hi_sdio)
{
    struct sdio_func *func = NULL;
    int32_t ret = 0;

    if (oal_unlikely(!hi_sdio)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hi_sdio is null", __FUNCTION__);
        return;
    }

    if (oal_unlikely(!hi_sdio->func)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hi_sdio->func is null", __FUNCTION__);
        return;
    }

    func = hi_sdio->func;

    oal_sdio_claim_host(hi_sdio);
    oal_sdio_writeb(func, 0, HISDIO_REG_FUNC1_INT_ENABLE, &ret);
    oal_sdio_interrupt_unregister(hi_sdio);
    sdio_disable_func(func);
    hcc_bus_disable_state(hi_sdio->pst_bus, OAL_BUS_STATE_ALL);
    oal_sdio_release_host(hi_sdio);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "oal_sdio_dev_deinit! ");
}

OAL_STATIC OAL_INLINE void oal_sdio_print_state(uint32_t old_state, uint32_t new_state)
{
    if (old_state != new_state) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio state changed, tx[%s=>%s],rx[%s=>%s]",
                             (old_state & OAL_BUS_STATE_TX) ? "on " : "off",
                             (new_state & OAL_BUS_STATE_TX) ? "on " : "off",
                             (old_state & OAL_BUS_STATE_RX) ? "on " : "off",
                             (new_state & OAL_BUS_STATE_RX) ? "on " : "off");
    }
}

OAL_STATIC int32_t oal_sdio_get_state(hcc_bus *pst_bus, uint32_t mask)
{
    struct oal_sdio *hi_sdio = NULL;

    if (oal_warn_on(pst_bus == NULL)) {
        return OAL_FALSE;
    }

    if (oal_warn_on(pst_bus->data == NULL)) {
        return OAL_FALSE;
    }

    hi_sdio = (struct oal_sdio *)pst_bus->data;

    if ((hi_sdio->state & mask) == mask) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}

/*
 * Prototype    : oal_enable_sdio_state
 * Description  : set the sdio bus state
 * Input        : struct iodevice *io_dev, uint8_t state: TRUE/FALSE
 */
OAL_STATIC void oal_enable_sdio_state(hcc_bus *pst_bus, uint32_t mask)
{
    uint32_t old_state;
    struct oal_sdio *hi_sdio = NULL;

    if (oal_warn_on(pst_bus == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: pst_bus is null", __FUNCTION__);
        return;
    }

    hi_sdio = (struct oal_sdio *)pst_bus->data;

    oal_sdio_claim_host(hi_sdio);
    old_state = hi_sdio->state;
    hi_sdio->state |= mask;
    oal_sdio_print_state(old_state, hi_sdio->state);
    oal_sdio_release_host(hi_sdio);
}

OAL_STATIC void oal_disable_sdio_state(hcc_bus *pst_bus, uint32_t mask)
{
    uint32_t old_state;
    struct oal_sdio *hi_sdio = NULL;

    if (oal_warn_on(pst_bus == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: pst_bus is null", __FUNCTION__);
        return;
    }

    hi_sdio = (struct oal_sdio *)pst_bus->data;

    oal_sdio_claim_host(hi_sdio);
    old_state = hi_sdio->state;
    hi_sdio->state &= ~mask;
    oal_sdio_print_state(old_state, hi_sdio->state);
    oal_sdio_release_host(hi_sdio);
}

OAL_STATIC int32_t oal_sdio_rx_netbuf(hcc_bus *pst_bus, oal_netbuf_head_stru *pst_head)
{
    int32_t ret;
    struct oal_sdio *pst_sdio = (struct oal_sdio *)pst_bus->data;

    if (oal_warn_on(pst_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: pst_sdio is null", __FUNCTION__);
        return -OAL_EFAIL;
    }

    ret = oal_sdio_build_rx_netbuf_list(pst_sdio, pst_head);
    if (ret != OAL_SUCC) {
        return ret;
    }

    oal_sdio_claim_host(pst_sdio);

    ret = oal_sdio_transfer_netbuf_list(pst_sdio, pst_head, SDIO_READ);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "Failed to read scatt sdio![scatt len:%d]", oal_netbuf_list_len(pst_head));
        oal_sdio_release_host(pst_sdio);
        return -OAL_EFAIL;
    }

    oal_sdio_release_host(pst_sdio);

#ifdef CONFIG_SDIO_DEBUG
    oal_netbuf_list_hex_dump(pst_head);
#endif

    return ret;
}

OAL_STATIC int32_t oal_sdio_tx_netbuf(hcc_bus *pst_bus, oal_netbuf_head_stru *pst_head, hcc_netbuf_queue_type qtype)
{
    struct hcc_handler *pst_hcc;
    pst_hcc = hbus_to_hcc(pst_bus);
    if (pst_hcc == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio dev's hcc handler is null!");
        return 0;
    }
    if (oal_unlikely(oal_atomic_read(&pst_hcc->state) != HCC_ON)) {
        /* drop netbuf list, wlan close or exception */
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "drop sdio netbuflist %u", oal_netbuf_list_len(pst_head));
        return 0;
    }
    return oal_sdio_transfer_netbuf_list((struct oal_sdio *)pst_bus->data, pst_head, SDIO_WRITE);
}

OAL_STATIC struct oal_sdio *oal_sdio_alloc(struct sdio_func *func)
{
    struct oal_sdio *hi_sdio = NULL;

    if (oal_warn_on(func == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hi_sdio is null", __FUNCTION__);
        return NULL;
    }

    /* alloce sdio control struct */
    hi_sdio = oal_sdio_init_module();
    if (hi_sdio == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "Failed to alloc hi_sdio!");
        return NULL;
    }

    hi_sdio->func = func;

    /* func keep a pointer to oal_sdio */
    sdio_set_drvdata(func, hi_sdio);

    return hi_sdio;
}

/*
 * Prototype    : oal_sdio_free
 * Description  : free sdio dev
 */
OAL_STATIC void oal_sdio_free(struct oal_sdio *hi_sdio)
{
    if (hi_sdio == NULL) {
        return;
    }
    oal_sdio_exit_module(hi_sdio);
}

/*
 * Prototype    : oal_sdio_probe
 * Description  : initialize sdio interface
 */
OAL_STATIC int32_t oal_sdio_probe(struct sdio_func *func, const struct sdio_device_id *ids)
{
    struct oal_sdio *hi_sdio = NULL;
    int32_t ret;
    hcc_bus *pst_bus = NULL;

    if (oal_warn_on(!func)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: func is null", __FUNCTION__);
        return -OAL_EFAIL;
    };

    if (oal_warn_on(!ids)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: ids is null", __FUNCTION__);
        return -OAL_EFAIL;
    };

    /* alloce sdio control struct */
    hi_sdio = oal_sdio_alloc(func);
    if (hi_sdio == NULL) {
        g_sdio_enum_err_str = "failed to alloc hi_sdio!";
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s", g_sdio_enum_err_str);
        goto failed_sdio_alloc;
    }

    if (oal_sdio_dev_init(hi_sdio) != OAL_SUCC) {
        g_sdio_enum_err_str = "sdio dev init failed";
        (void)ssi_dump_err_regs(SSI_ERR_SDIO_PROBE_INIT_FAIL);
        goto failed_sdio_dev_init;
    }

    /* Print the sdio's cap */
    oal_print_hi11xx_log(HI11XX_LOG_INFO,
                         "max_segs:%u, max_blk_size:%u,max_blk_count:%u,,max_seg_size:%u,max_req_size:%u",
                         func->card->host->max_segs, func->card->host->max_blk_size,
                         func->card->host->max_blk_count, func->card->host->max_seg_size,
                         func->card->host->max_req_size);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "transer limit size:%u", oal_sdio_func_max_req_size(hi_sdio));

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "+++++++++++++func->enable_timeout= [%d]++++++++++++++++",
                         func->enable_timeout);

    /* register interrupt process function */
    ret = oal_sdio_interrupt_register(hi_sdio);
    if (ret < 0) {
        g_sdio_enum_err_str = "failed to register sdio interrupt";
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s", g_sdio_enum_err_str);
        goto failed_sdio_int_reg;
    }

    pst_bus = oal_sdio_bus_init(hi_sdio);
    if (pst_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio bus init failed");
        goto failed_sdio_bus_init;
    }

    oal_wake_lock_init(&hi_sdio->st_sdio_rx_wakelock, "wlan_sdio_rx_lock");
    oal_init_completion(&hi_sdio->st_sdio_shutdown_response);

    hi_sdio->pst_bus = pst_bus;
    pst_bus->data = (void *)hi_sdio;
    hcc_bus_enable_state(hi_sdio->pst_bus, OAL_BUS_STATE_ALL);
    complete(&g_sdio_driver_complete);
    return OAL_SUCC;

failed_sdio_bus_init:
    oal_sdio_interrupt_unregister(hi_sdio);
failed_sdio_int_reg:
failed_sdio_dev_init:
    oal_sdio_free(hi_sdio);
failed_sdio_alloc:

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_SDIO, CHR_PLAT_DRV_ERROR_SDIO_INIT_PROB_FAIL);

    return -OAL_EFAIL;
}

OAL_STATIC int32_t oal_sdio_single_transfer(struct oal_sdio *hi_sdio, int32_t rw,
                                            void *buf, uint32_t size)
{
    if (oal_warn_on(hi_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hi_sdio is null", __FUNCTION__);
        return -OAL_EFAIL;
    }
    if (oal_warn_on(hi_sdio->func == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hi_sdio->func is null", __FUNCTION__);
        return -OAL_EFAIL;
    }
    if (oal_warn_on(buf == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: buf is null", __FUNCTION__);
        return -OAL_EFAIL;
    }
    if (oal_warn_on((uintptr_t)buf & 0x3)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: buf :%ld", __FUNCTION__, (uintptr_t)buf);
        return -OAL_EFAIL;
    }

    return oal_sdio_rw_buf(hi_sdio, rw, HISDIO_REG_FUNC1_FIFO, buf, size);
}

int32_t oal_sdio_transfer_tx(struct oal_sdio *hi_sdio, oal_netbuf_stru *netbuf)
{
    int32_t ret = OAL_SUCC;
    int32_t tailroom, tailroom_add;

    tailroom = hisdio_align_4_or_blk(oal_netbuf_len(netbuf))
               - oal_netbuf_len(netbuf);
    if (tailroom > oal_netbuf_tailroom(netbuf)) {
        tailroom_add = tailroom - oal_netbuf_tailroom(netbuf);
        /* relloc the netbuf */
        ret = oal_netbuf_expand_head(netbuf, 0, tailroom_add, GFP_ATOMIC);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc tail room failed");
            return -OAL_EFAIL;
        }
    }

    oal_netbuf_put(netbuf, tailroom);

    return oal_sdio_single_transfer(hi_sdio, SDIO_WRITE,
                                    oal_netbuf_data(netbuf), oal_netbuf_len(netbuf));
}
oal_module_symbol(oal_sdio_transfer_tx);

OAL_STATIC void check_sg_format(struct scatterlist *sg, uint32_t sg_len)
{
    int32_t i;
    struct scatterlist *sg_t = NULL;
    for_each_sg(sg, sg_t, sg_len, i)
    {
        if (oal_unlikely(sg_t == NULL)) {
            continue;
        }
        if (oal_warn_on(((uintptr_t)sg_virt(sg_t) & 0x03) || (sg_t->length & 0x03))) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "check_sg_format:[i:%d][addr:%p][len:%u]",
                                 i, sg_virt(sg_t), sg_t->length);
        }
    }
}

static inline int32_t oal_mmc_io_rw_scat_extended_check(struct oal_sdio *hi_sdio,
                                                        struct scatterlist *sg,
                                                        uint32_t sg_len,
                                                        uint32_t fn,
                                                        uint32_t blksz,
                                                        uint32_t addr)
{
    if (oal_unlikely(hi_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hi_sdio is null", __FUNCTION__);
        return -EINVAL;
    }
    if (oal_unlikely(sg == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: sg is null", __FUNCTION__);
        return -EINVAL;
    }
    if (oal_unlikely(sg_len == 0)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: sg_len is %d", __FUNCTION__, sg_len);
        return -EINVAL;
    }
    if (oal_unlikely(fn > 7)) { /* 7 max funs, sdio协议固定的内容 */
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: fn is %d", __FUNCTION__, fn);
        return -EINVAL;
    }

    if (oal_warn_on(blksz == 0)) {
        return -EINVAL;
    }

    /* sanity check */
    if (oal_unlikely(addr & ~0x1FFFF)) {
        return -EINVAL;
    }

    return OAL_SUCC;
}

static inline void oal_mmc_io_rw_dump_mmc_data_info(struct mmc_data *data)
{
#ifdef CONFIG_SDIO_DEBUG
    int i;
    struct scatterlist *sg_t = NULL;
    for_each_sg(data->sg, sg_t, data->sg_len, i)
    {
        printk(KERN_INFO "======netbuf pkts %d, len:%d=========\n", i, sg_t->length);
        oal_print_hex_dump(sg_virt(sg_t), sg_t->length, HEX_DUMP_GROUP_SIZE, "sg buf  :");
    }
#endif
}

static inline int32_t oal_sdio_get_mmc_errno_by_resp(struct mmc_command *cmd)
{
    if (cmd->resp[0] & R5_ERROR) {
        return -EIO;
    }
    if (cmd->resp[0] & R5_FUNCTION_NUMBER) {
        return -EINVAL;
    }
    if (cmd->resp[0] & R5_OUT_OF_RANGE) {
        return -ERANGE;
    }
    return OAL_SUCC;
}

static inline void oal_mmc_io_rw_scat_extended_dump(struct mmc_card *card,
                                                    struct mmc_command *cmd,
                                                    int32_t write,
                                                    uint32_t sg_len,
                                                    uint32_t blocks,
                                                    uint32_t blksz)
{
    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "[blksz:%u][blocks:%u][sg_len:%u][mode:%s]",
                         blksz, blocks, sg_len, write ? "write" : "read");
    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "%s : [cmd opcode:%d][cmd arg:0x%8x][cmd flags: 0x%8x]",
                         mmc_hostname(card->host), cmd->opcode, cmd->arg, cmd->flags);
    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "sdio %s data transfer start", write ? "write" : "read");
}

OAL_STATIC int32_t oal_mmc_io_rw_scat_extended(struct oal_sdio *hi_sdio,
                                               int32_t write,
                                               uint32_t fn,
                                               uint32_t addr, int32_t incr_addr,
                                               struct scatterlist *sg, uint32_t sg_len,
                                               uint32_t blocks, uint32_t blksz)
{
    int32_t ret;
    struct mmc_request mrq = {0};
    struct mmc_command cmd = {0};
    struct mmc_data data = {0};
    struct mmc_card *card = NULL;

    ret = oal_mmc_io_rw_scat_extended_check(hi_sdio, sg, sg_len, fn, blksz, addr);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    card = hi_sdio->func->card;

    /* sg format */
    check_sg_format(sg, sg_len);

    mrq.cmd = &cmd;
    mrq.data = &data;

    /* 组合sdio命令参数 */
    cmd.opcode = SD_IO_RW_EXTENDED;
    cmd.arg = write ? 0x80000000 : 0x00000000;
    cmd.arg |= fn << 28; // bit28 func num
    cmd.arg |= incr_addr ? 0x04000000 : 0x00000000;
    cmd.arg |= addr << 9; // bit9 func address
    if (blocks == 1 && blksz <= HISDIO_BLOCK_SIZE) {
        cmd.arg |= (blksz == HISDIO_BLOCK_SIZE) ? 0 : blksz; /* byte mode */
    } else {
        cmd.arg |= 0x08000000 | blocks; /* block mode */
    }
    cmd.flags = MMC_RSP_SPI_R5 | MMC_RSP_R5 | MMC_CMD_ADTC;

    data.blksz = blksz;
    data.blocks = blocks;
    data.flags = write ? MMC_DATA_WRITE : MMC_DATA_READ;
    data.sg = sg;
    data.sg_len = sg_len;

    oal_mmc_io_rw_scat_extended_dump(card, &cmd, write, sg_len, blocks, blksz);

    mmc_set_data_timeout(&data, card);

    mmc_wait_for_req(card->host, &mrq);

    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "wait for %s tranfer over", write ? "write" : "read");

    if (cmd.error) {
        return cmd.error;
    }
    if (data.error) {
        return data.error;
    }
    if (oal_warn_on(mmc_host_is_spi(card->host))) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "HiSi WiFi  driver do not support spi sg transfer!");
        return -EIO;
    }

    ret = oal_sdio_get_mmc_errno_by_resp(&cmd);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    oal_mmc_io_rw_dump_mmc_data_info(&data);

    oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "Transfer done. %s sucuess!", write ? "write" : "read");
    return 0;
}

OAL_STATIC int32_t _oal_sdio_transfer_scatt(struct oal_sdio *hi_sdio, int32_t rw,
                                            uint32_t addr, struct scatterlist *sg,
                                            uint32_t sg_len,
                                            uint32_t rw_sz)
{
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    ktime_t time_start;
#endif
    int32_t ret;
    int32_t write = (rw == SDIO_READ) ? 0 : 1;

    oal_sdio_claim_host(hi_sdio);
    /* continue only when tx/rx all opened! */
    if (oal_unlikely(oal_sdio_get_state(hi_sdio->pst_bus, OAL_BUS_STATE_ALL) != OAL_TRUE)) {
        if (printk_ratelimit()) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio closed,state:%u, %s ignored",
                                 oal_sdio_get_state(hi_sdio->pst_bus, OAL_BUS_STATE_ALL),
                                 write ? "write" : "read");
        }
        schedule();
        oal_sdio_release_host(hi_sdio);
        return -OAL_EFAIL;
    }
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    time_start = ktime_get();
#endif
    ret = oal_mmc_io_rw_scat_extended(hi_sdio, write,
                                      hi_sdio->func->num, addr,
                                      0, sg,
                                      sg_len, (rw_sz / HISDIO_BLOCK_SIZE) ? : 1,
                                      min(rw_sz, (uint32_t)HISDIO_BLOCK_SIZE));
    if (oal_unlikely(ret)) {
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
        /* If sdio transfer failed, dump the sdio info */
        uint64_t trans_us;
        ktime_t time_stop = ktime_get();
        trans_us = (uint64_t)ktime_to_us(ktime_sub(time_stop, time_start));
        oal_print_hi11xx_log(HI11XX_LOG_ERR,
                             "sdio_transfer_scatt fail=%d, time cost:%llu us,[addr:%u, sg_len:%u,rw_sz:%u]",
                             ret, trans_us, addr, sg_len, rw_sz);

#endif
        if (write) {
            declare_dft_trace_key_info("sdio_write_fail", OAL_DFT_TRACE_FAIL);
            oam_error_log1(0, OAM_SF_ANY, "{oal_sdio_transfer_scatt::write failed=%d}", ret);
        } else {
            declare_dft_trace_key_info("sdio_read_fail", OAL_DFT_TRACE_FAIL);
            oam_error_log1(0, OAM_SF_ANY, "{oal_sdio_transfer_scatt::read failed=%d}", ret);
        }
        wlan_pm_dump_host_info();

        hcc_bus_exception_submit(hi_sdio->pst_bus, WIFI_TRANS_FAIL);

        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_SDIO, CHR_PLAT_DRV_ERROR_SDIO_WR_FAIL);
    }
    oal_sdio_release_host(hi_sdio);
    return ret;
}

/*
 * Prototype    : oal_sdio_transfer_scatt
 * Description  : sdio scatter data transfer
 */
OAL_STATIC int32_t oal_sdio_transfer_scatt(struct oal_sdio *hi_sdio, int32_t rw,
                                           uint32_t addr, struct scatterlist *sg,
                                           uint32_t sg_len, uint32_t sg_max_len,
                                           uint32_t rw_sz)
{
    int32_t ret;
    uint32_t align_len;
    uint32_t align_t;
    if (oal_warn_on(hi_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hi_sdio is null", __FUNCTION__);
        return -OAL_EINVAL;
    }
    if (oal_warn_on(!rw_sz)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: rw_sz is null", __FUNCTION__);
        return -OAL_EINVAL;
    }
    if (oal_warn_on(sg_max_len < sg_len)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: sg_max_len:%d <  sg_len:%d", __FUNCTION__, sg_max_len, sg_len);
        return -OAL_EINVAL;
    }

    if (oal_warn_on(!sg_len)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "Sdio %s Scatter list num should never be zero, total request len: %u",
                             rw == SDIO_READ ? "SDIO READ" : "SDIO WRITE",
                             rw_sz);
        return -OAL_EINVAL;
    }

    align_t = hisdio_align_4_or_blk(rw_sz);
    align_len = align_t - rw_sz;

    if (oal_likely(align_len)) {
        if (oal_unlikely(sg_len + 1 > sg_max_len)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "sg list over,sg_len:%u, sg_max_len:%u", sg_len, sg_max_len);
            return -OAL_ENOMEM;
        }
        sg_set_buf(&sg[sg_len], hi_sdio->sdio_align_buff, align_len);
        sg_len++;
    }
    sg_mark_end(&sg[sg_len - 1]);

    oal_print_hi11xx_log(HI11XX_LOG_DBG,
                         "sdio %s request %u bytes transfer, scatter list num %u, used %u bytes to align",
                         (rw == SDIO_READ) ? "read" : "write", rw_sz, sg_len, align_len);

    rw_sz = align_t;

    /* sdio scatter list driver ,when letter than 512 bytes bytes mode, other blockmode */
    oal_warn_on((rw_sz >= HISDIO_BLOCK_SIZE) && (rw_sz & (HISDIO_BLOCK_SIZE - 1)));
    /* 如果不是4字节对齐，则会dump堆栈信息 */
    oal_warn_on((rw_sz < HISDIO_BLOCK_SIZE) && (rw_sz & (sizeof(uint32_t) - 1)));

    if (oal_warn_on(align_len & 0x3)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "not 4 bytes align:%u", align_len);
    }

    ret = _oal_sdio_transfer_scatt(hi_sdio, rw, addr, sg, sg_len, rw_sz);

    return ret;
}

#if defined(CONFIG_HISDIO_H2D_SCATT_LIST_ASSEMBLE)
OAL_STATIC int32_t oal_sdio_transfer_merge_write_sglist(struct oal_sdio *hi_sdio,
                                                        oal_netbuf_head_stru *head,
                                                        struct scatterlist *sg,
                                                        uint32_t sg_max_len,
                                                        uint32_t *total_len,
                                                        uint32_t *sg_len,
                                                        int32_t rw)
{
    int32_t idx = 0;
    uint32_t offset = 0;
    oal_netbuf_stru *netbuf = NULL;
    oal_netbuf_stru *tmp = NULL;

    uint32_t align_len;
    uint32_t align_t;

    int32_t i;
    struct scatterlist *sg_t = NULL;
    uint32_t left_size, nents;
    uint32_t seg_size = hi_sdio->func->card->host->max_seg_size;

    /* 发送内存拷贝，合并成一块内存 */
    skb_queue_walk_safe(head, netbuf, tmp)
    {
        if (memcpy_s(hi_sdio->tx_scatt_buff.buff + offset, hi_sdio->tx_scatt_buff.len - offset,
            oal_netbuf_data(netbuf), oal_netbuf_len(netbuf)) != EOK) {
            oal_io_print("memcpy_s error, destlen=%u, srclen=%u\n ",
                         hi_sdio->tx_scatt_buff.len - offset, oal_netbuf_len(netbuf));
            return -OAL_EFAIL;
        }
        offset += oal_netbuf_len(netbuf);
    }

    align_t = hisdio_align_4_or_blk(offset);
    align_len = align_t - offset;
    offset = align_t; /* 对齐长度用内存填充 */

    /* build tx sg list */
    left_size = offset;
    nents = ((left_size - 1) / seg_size) + 1;
    if (oal_unlikely(nents > sg_max_len)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "tx merged scatt list num %d > sg_len:%u, max seg size:%u\n",
                             nents,
                             sg_max_len,
                             seg_size);
        return -OAL_ENOMEM;
    }

    for_each_sg(sg, sg_t, nents, i) {
        if (oal_unlikely(sg_t == NULL)) {
            return -OAL_ENOMEM;
        }
        sg_set_buf(sg_t, hi_sdio->tx_scatt_buff.buff + i * seg_size,
                   oal_min(seg_size, left_size));
        left_size = left_size - seg_size;
        idx++;
    }

    *sg_len = idx;
    *total_len = offset;

    return OAL_SUCC;
}
#endif

#if defined(CONFIG_HISDIO_D2H_SCATT_LIST_ASSEMBLE)
OAL_STATIC int32_t oal_sdio_transfer_merge_read_sglist(struct oal_sdio *hi_sdio,
                                                       oal_netbuf_head_stru *head,
                                                       struct scatterlist *sg,
                                                       uint32_t sg_max_len,
                                                       uint32_t *total_len,
                                                       uint32_t *sg_len,
                                                       int32_t rw)
{
    int32_t idx = 0;
    uint32_t offset = 0;
    oal_netbuf_stru *netbuf = NULL;
    oal_netbuf_stru *tmp = NULL;

    uint32_t align_len;
    uint32_t align_t;

    int32_t i;
    struct scatterlist *sg_t = NULL;
    uint32_t left_size, nents;
    uint32_t seg_size = hi_sdio->func->card->host->max_seg_size;

    skb_queue_walk_safe(head, netbuf, tmp)
    {
        offset += oal_netbuf_len(netbuf);
    }

    align_t = hisdio_align_4_or_blk(offset);
    align_len = align_t - offset;
    offset = align_t; /* 对齐长度用内存填充 */

    /* build rx sg list */
    left_size = offset;
    nents = ((left_size - 1) / seg_size) + 1;
    if (oal_unlikely(nents > sg_max_len)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "rx merged scatt list num %d > sg_len:%u, max seg size:%u\n",
                             nents,
                             sg_max_len,
                             seg_size);
        return -OAL_ENOMEM;
    }

    for_each_sg(sg, sg_t, nents, i) {
        if (oal_unlikely(sg_t == NULL)) {
            return -OAL_EINVAL;
        }
        sg_set_buf(sg_t, hi_sdio->rx_scatt_buff.buff + i * seg_size,
                   oal_min(seg_size, left_size));
        left_size = left_size - seg_size;
        idx++;
    }

    *sg_len = idx;
    *total_len = offset;

    return OAL_SUCC;
}
#endif

OAL_STATIC int32_t oal_sdio_transfer_rebuild_sglist(struct oal_sdio *hi_sdio,
                                                    oal_netbuf_head_stru* head,
                                                    struct scatterlist *sg,
                                                    uint32_t sg_max_len,
                                                    uint32_t *total_len,
                                                    uint32_t *sg_len,
                                                    int32_t rw)
{
    int32_t idx = 0;
    uint32_t sum_len = 0;
    oal_netbuf_stru *netbuf = NULL;
    oal_netbuf_stru *tmp = NULL;

    memset_s(sg, sizeof(struct scatterlist) * sg_max_len, 0, sizeof(struct scatterlist) * sg_max_len);

    /* sdio write/read, sdio use signle transfer when sg list item not align to block size(512B),
     * sdio used burst transfer if align to block size. burst fast than signle over then in sdio.
     * If master cpu more fast than sdio, we use master cpu memcpy sg to 4KB align mems to send when we write,
     * we recv sdio mem to sg 4KB align mems and memcpy to sg list when we read, refer to
     * feature macro CONFIG_HISDIO_H2D_SCATT_LIST_ASSEMBLE & CONFIG_HISDIO_D2H_SCATT_LIST_ASSEMBLE */
#if defined(CONFIG_HISDIO_H2D_SCATT_LIST_ASSEMBLE)
    if (rw == SDIO_WRITE) {
        /* 发送内存拷贝，合并成一块内存 */
        return oal_sdio_transfer_merge_write_sglist(hi_sdio, head, sg, sg_max_len, total_len, sg_len, rw);
    }
#endif

#if defined(CONFIG_HISDIO_D2H_SCATT_LIST_ASSEMBLE)
    if (rw == SDIO_READ) {
        /* 接收到连续内存，接收完成后靠背成离散内存 */
        return oal_sdio_transfer_merge_read_sglist(hi_sdio, head, sg, sg_max_len, total_len, sg_len, rw);
    }
#endif

    skb_queue_walk_safe(head, netbuf, tmp) {
        if (oal_warn_on(!oal_is_aligned((uintptr_t)oal_netbuf_data(netbuf), 4))) { /* 判断是不是4字节对齐 */
            /* This should never happned, debug */
            oal_netbuf_hex_dump(netbuf);
            return -OAL_EFAUL;
        }
        if (oal_warn_on(!oal_is_aligned(oal_netbuf_len(netbuf), HISDIO_H2D_SCATT_BUFFLEN_ALIGN))) {
            /* This should never happned, debug */
            oal_netbuf_hex_dump(netbuf);
            return -OAL_EFAUL;
        }
        sg_set_buf(&sg[idx], oal_netbuf_data(netbuf), oal_netbuf_len(netbuf));
        sum_len += oal_netbuf_len(netbuf);
        idx++;
    }

    *sg_len = idx;
    *total_len = sum_len;

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_sdio_transfer_restore_sglist(struct oal_sdio *hi_sdio,
                                                    oal_netbuf_head_stru *head,
                                                    struct scatterlist *sg,
                                                    uint32_t sg_len,
                                                    int32_t rw)
{
#if defined(CONFIG_HISDIO_D2H_SCATT_LIST_ASSEMBLE)
    int32_t offset = 0;
    oal_netbuf_stru *netbuf = NULL;
    oal_netbuf_stru *tmp = NULL;
    int32_t ret;

    if (rw == SDIO_READ) {
        /* 接收内存拷贝，分散成离散内存 */
        skb_queue_walk_safe(head, netbuf, tmp)
        {
            if ((hi_sdio->rx_scatt_buff.len - offset) > oal_netbuf_len(netbuf)) {
                ret = memcpy_s(oal_netbuf_data(netbuf), oal_netbuf_len(netbuf),
                               hi_sdio->rx_scatt_buff.buff + offset, oal_netbuf_len(netbuf));
                if (ret == EOK) {
                    offset += oal_netbuf_len(netbuf);
                }
            }
        }
    }
#endif

    return OAL_SUCC;
}

int32_t oal_sdio_transfer_netbuf_list(struct oal_sdio *hi_sdio,
                                      oal_netbuf_head_stru *head,
                                      int32_t rw)
{
    uint8_t sg_realloc = 0;
    int32_t ret;
    uint32_t sg_len = 0;
    uint32_t queue_len;
    uint32_t sum_len = 0;
    uint32_t request_sg_len;
    struct scatterlist *sg = NULL;
    struct sg_table sgtable;

    if (oal_warn_on(hi_sdio == NULL)) {
        return 0;
    }

    if (oal_warn_on(rw >= SDIO_OPT_BUTT)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "invaild rw:%d", rw);
        return 0;
    }

    if (oal_warn_on(oal_netbuf_list_empty(head))) {
        return -OAL_EINVAL;
    }

    if (rw == SDIO_WRITE) {
        if (oal_unlikely(hcc_bus_pm_wakeup_device(hi_sdio->pst_bus) != OAL_SUCC)) {
            oal_msleep(100); /* wait for a while retry */
            return -OAL_EBUSY;
        }
    }

    queue_len = oal_netbuf_list_len(head);
    /* must realloc the sg list mem, alloc more sg for the align buff */
    request_sg_len = queue_len + 1;
    if (oal_unlikely(request_sg_len > hi_sdio->scatt_info[rw].max_scatt_num)) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "transfer_netbuf_list realloc sg!, request:%d,max scatt num:%d",
                             request_sg_len, hi_sdio->scatt_info[rw].max_scatt_num);
        /* must realloc the sg list mem, alloc more sgs for the align buff */
        if (sg_alloc_table(&sgtable, request_sg_len, GFP_KERNEL)) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "transfer_netbuf_list alloc sg failed!");
            return -OAL_ENOMEM;
        }
        sg_realloc = 1;
        sg = sgtable.sgl;
    } else {
        sg = hi_sdio->scatt_info[rw].sglist;
    }

    /* merge sg list */
    ret = oal_sdio_transfer_rebuild_sglist(hi_sdio, head, sg, request_sg_len, &sum_len, &sg_len, rw);
    if (oal_unlikely(ret != OAL_SUCC)) {
        if (sg_realloc) {
            sg_free_table(&sgtable);
        }
        return -OAL_EFAIL;
    }

    ret = oal_sdio_transfer_scatt(hi_sdio, rw,
                                  HISDIO_REG_FUNC1_FIFO, sg,
                                  sg_len, request_sg_len,
                                  sum_len);
    if (oal_likely(ret == OAL_SUCC)) {
        oal_sdio_transfer_restore_sglist(hi_sdio, head, sg, request_sg_len, rw);
    }

    if (sg_realloc) {
        sg_free_table(&sgtable);
    }

    return ret;
}

/*
 * Prototype    : oal_sdio_remove
 * Description  : uninitialize sdio interface
 */
OAL_STATIC void oal_sdio_remove(struct sdio_func *func)
{
    struct oal_sdio *hi_sdio = NULL;

    if (oal_warn_on(func == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: func is null", __FUNCTION__);
        return;
    }

    hi_sdio = (struct oal_sdio *)sdio_get_drvdata(func);
    if (hi_sdio == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[Error]Invalid NULL hi_sdio!");
        return;
    }

    oal_wake_lock_exit(&hi_sdio->st_sdio_rx_wakelock);

    oal_sdio_dev_deinit(hi_sdio);
    oal_sdio_bus_exit(hi_sdio);
    oal_sdio_free(hi_sdio);
    sdio_set_drvdata(func, NULL);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "hisilicon connectivity sdio driver has been removed.");
}

/*
 * Prototype    : oal_sdio_suspend
 * Input        : struct device *dev
 */
OAL_STATIC int32_t oal_sdio_suspend(struct device *dev)
{
    /* to be implement */
    struct sdio_func *func = NULL;
    struct oal_sdio *hi_sdio = NULL;
    hcc_bus *pst_bus = NULL;

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "+++++++sdio suspend+++++++++++++");
    if (dev == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[WARN]dev is null");
        return OAL_SUCC;
    }
    func = dev_to_sdio_func(dev);
    hi_sdio = sdio_get_drvdata(func);
    if (hi_sdio == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hi_sdio is null");
        return OAL_SUCC;
    }

    if (oal_warn_on(hi_sdio->pst_bus == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[E]sdio's bus is null, fatal error");
        return -OAL_ENODEV;
    }

    if (hbus_to_dev(hi_sdio->pst_bus) == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio is not work...");
        return OAL_SUCC;
    }

    if (hi_sdio->pst_bus != hdev_to_hbus(hbus_to_dev(hi_sdio->pst_bus))) {
        /* sdio非当前接口 */
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio is not current bus, return");
        return OAL_SUCC;
    }

    pst_bus = hi_sdio->pst_bus;

    if (down_interruptible(&pst_bus->sr_wake_sema)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, KERN_ERR "sdio_wake_sema down failed.");
        return -OAL_EFAIL;
    }

    if (hcc_bus_wakelock_active(pst_bus)) {
        /*
         * has wake lock so stop controller's suspend,
         * otherwise controller maybe error while sdio reinit
         */
        oal_print_hi11xx_log(HI11XX_LOG_ERR, KERN_ERR "Already wake up");
        up(&pst_bus->sr_wake_sema);
        return -OAL_EFAIL;
    }

    wlan_pm_wkup_src_debug_set(OAL_TRUE);

    declare_dft_trace_key_info("sdio_os_suspend", OAL_DFT_TRACE_SUCC);
    hi_sdio->sdio_suspend++;
    return OAL_SUCC;
}

/*
 * Prototype    : oal_sdio_resume
 * Input        : struct device *dev
 */
OAL_STATIC int32_t oal_sdio_resume(struct device *dev)
{
    struct sdio_func *func = NULL;
    struct oal_sdio *hi_sdio = NULL;
    hcc_bus *pst_bus = NULL;

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "+++++++sdio resume+++++++++++++");
    if (dev == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "dev is null");
        return OAL_SUCC;
    }
    func = dev_to_sdio_func(dev);
    hi_sdio = sdio_get_drvdata(func);
    if (hi_sdio == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hi_sdio is null");
        return OAL_SUCC;
    }

    if (oal_warn_on(hi_sdio->pst_bus == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio's bus is null, fatal error");
        return OAL_SUCC;
    }

    if (hbus_to_dev(hi_sdio->pst_bus) == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio is not work...");
        return OAL_SUCC;
    }

    if (hi_sdio->pst_bus != hdev_to_hbus(hbus_to_dev(hi_sdio->pst_bus))) {
        /* sdio非当前接口 */
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio is not current bus, return");
        return OAL_SUCC;
    }

    pst_bus = hi_sdio->pst_bus;

    up(&pst_bus->sr_wake_sema);

    hi_sdio->sdio_resume++;
    declare_dft_trace_key_info("sdio_os_resume", OAL_DFT_TRACE_SUCC);

    return OAL_SUCC;
}

OAL_STATIC struct sdio_device_id const g_oal_sdio_ids[] = {
    { SDIO_DEVICE(HISDIO_VENDOR_ID_HI1102,  HISDIO_PRODUCT_ID_HISI) },
    { SDIO_DEVICE(HISDIO_VENDOR_ID_HI1103,  HISDIO_PRODUCT_ID_HISI) },
    { SDIO_DEVICE(HISDIO_VENDOR_ID_HI1102A, HISDIO_PRODUCT_ID_1102A_HISI) },
    { SDIO_DEVICE(HISDIO_VENDOR_ID_HI1103A, HISDIO_PRODUCT_ID_1103A_HISI) },
    { SDIO_DEVICE(HISDIO_VENDOR_ID_HISI,    HISDIO_PRODUCT_ID_1135_HISI) },
    { SDIO_DEVICE(HISDIO_VENDOR_ID_HISI,    HISDIO_PRODUCT_ID_1106_HISI) },
    {},
};
MODULE_DEVICE_TABLE(sdio, g_oal_sdio_ids);

OAL_STATIC const struct dev_pm_ops g_oal_sdio_pm_ops = {
    .suspend = oal_sdio_suspend,
    .resume = oal_sdio_resume,
};

OAL_STATIC void oal_sdio_dev_shutdown(struct device *dev)
{
    /* poweroff */
    struct oal_sdio *hi_sdio = oal_get_sdio_default_handler();
    if (hi_sdio == NULL) {
        return;
    }

    if (oal_sdio_get_state(hi_sdio->pst_bus, OAL_BUS_STATE_ALL) != OAL_TRUE) {
        /* wlan power off */
        return;
    }

    ssi_excetpion_dump_disable();
    /* system shutdown, should't write sdt file */
    oam_set_output_type(OAM_OUTPUT_TYPE_CONSOLE);

    /* disable sdio/gpio interrupt before os poweroff */
    if (g_hisdio_intr_mode) {
        /* gpio interrupt */
        oal_wlan_gpio_intr_enable(hbus_to_dev(hi_sdio->pst_bus), 0);
    } else {
        /* sdio interrupt */
        int32_t ret;
        oal_sdio_claim_host(hi_sdio);
        ret = sdio_disable_func(hi_sdio->func);
        oal_sdio_release_host(hi_sdio);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "wlan shutdown faile,ret=%d!", ret);
            return;
        }
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wlan shutdown sucuess!");
}

OAL_STATIC struct sdio_driver g_oal_sdio_driver = {
    .name = "oal_sdio",
    .id_table = g_oal_sdio_ids,
    .probe = oal_sdio_probe,
    .remove = oal_sdio_remove,
    .drv = {
        .owner = THIS_MODULE,
        .pm = &g_oal_sdio_pm_ops,
        .shutdown = oal_sdio_dev_shutdown,
    }
};

/* sdio first enum, wifi power on, must down later. */
OAL_STATIC int32_t oal_sdio_trigger_probe(void)
{
    int32_t ret;
    init_completion(&g_sdio_driver_complete);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "start to register sdio module");

    ret = sdio_register_driver(&g_oal_sdio_driver);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "register sdio driver Failed ret=%d", ret);
        goto failed_sdio_reg;
    }

    /* wifi chip power on */
    hi_wlan_power_set(1);

    /* notify mmc core to detect sdio device */
    oal_sdio_detectcard_to_core(1);

    if (wait_for_completion_timeout(&g_sdio_driver_complete, 10 * HZ)) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "hisi sdio load sucuess, sdio enum done.");
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio enum timeout, reason[%s]", g_sdio_enum_err_str);
        (void)ssi_dump_err_regs(SSI_ERR_SDIO_PROBE_FAIL);
        goto failed_sdio_enum;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "shutdown wifi after init sdio");
    oam_warning_log0(0, OAM_SF_ANY, "{oal_sdio_func_probe::shutdown wifi after init sdio.}");

    if (g_hi110x_sdio != NULL) {
        oal_sdio_claim_host(g_hi110x_sdio);
        hcc_bus_disable_state(g_hi110x_sdio->pst_bus, OAL_BUS_STATE_ALL);
        hi_wlan_power_set(0);
        oal_sdio_release_host(g_hi110x_sdio);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "g_hi110x_sdio is null");
        return -OAL_ENODEV;
    }

    return OAL_SUCC;

failed_sdio_enum:
    sdio_unregister_driver(&g_oal_sdio_driver);
failed_sdio_reg:
    hi_wlan_power_set(0);
    return -OAL_EFAIL;
}

OAL_STATIC void oal_sdio_func_remove(struct oal_sdio *hi_sdio)
{
    sdio_unregister_driver(&g_oal_sdio_driver);
    hi_wlan_power_set(0);
}

OAL_STATIC void oal_sdio_credit_info_init(struct oal_sdio *hi_sdio)
{
    hi_sdio->sdio_credit_info.large_free_cnt = 0;
    hi_sdio->sdio_credit_info.short_free_cnt = 0;
    oal_spin_lock_init(&hi_sdio->sdio_credit_info.credit_lock);
}

struct oal_sdio *oal_sdio_init_module()
{
#ifdef CONFIG_HISDIO_H2D_SCATT_LIST_ASSEMBLE
    uint32_t tx_scatt_buff_len;
#endif
#ifdef CONFIG_HISDIO_D2H_SCATT_LIST_ASSEMBLE
    uint32_t rx_scatt_buff_len;
#endif
    uint32_t ul_rx_seg_size;
    struct oal_sdio *hi_sdio = NULL;

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "hi110x sdio driver installing...");
    hi_sdio = (struct oal_sdio *)oal_memalloc(sizeof(struct oal_sdio));
    if (hi_sdio == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc oal_sdio failed [%d]", (int32_t)sizeof(struct oal_sdio));
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_SDIO, CHR_PLAT_DRV_ERROR_ALLOC_MEM);
        return NULL;
    }
    memset_s((void *)hi_sdio, sizeof(struct oal_sdio), 0, sizeof(struct oal_sdio));

#ifdef CONFIG_SDIO_FUNC_EXTEND
    g_sdio_extend_func = 1;
#else
    g_sdio_extend_func = 0;
#endif

    ul_rx_seg_size = ALIGN((HSDIO_HOST2DEV_PKTS_MAX_LEN), HISDIO_BLOCK_SIZE);
    /* alloc rx reserved mem */
    hi_sdio->rx_reserved_buff = (void *)oal_memalloc(ul_rx_seg_size);
    if (hi_sdio->rx_reserved_buff == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc rx_reserved_buff failed [%u]", ul_rx_seg_size);
        goto failed_rx_reserved_buff_alloc;
    }
    hi_sdio->rx_reserved_buff_len = ul_rx_seg_size;
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "alloc %u bytes rx_reserved_buff ", ul_rx_seg_size);

    hi_sdio->func1_int_mask = HISDIO_FUNC1_INT_MASK;

    oal_sdio_credit_info_init(hi_sdio);

    hi_sdio->sdio_extend = (struct hisdio_extend_func *)oal_memalloc(sizeof(struct hisdio_extend_func));
    if (hi_sdio->sdio_extend == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc sdio_extend failed [%d]",
                             (int32_t)sizeof(struct hisdio_extend_func));
        goto failed_sdio_extend_alloc;
    }
    memset_s(hi_sdio->sdio_extend, sizeof(struct hisdio_extend_func), 0, sizeof(struct hisdio_extend_func));

    g_hi110x_sdio = hi_sdio;

    g_hi_sdio_debug = hi_sdio;

    hi_sdio->scatt_info[SDIO_READ].max_scatt_num = HISDIO_DEV2HOST_SCATT_MAX + 1;
    hi_sdio->scatt_info[SDIO_READ].sglist = kzalloc(sizeof(struct scatterlist) * (HISDIO_DEV2HOST_SCATT_MAX + 1),
                                                    GFP_KERNEL);
    if (hi_sdio->scatt_info[SDIO_READ].sglist == NULL) {
        goto failed_sdio_read_sg_alloc;
    }

    /* 2 for algin buff and scatt info buff */
    hi_sdio->scatt_info[SDIO_WRITE].max_scatt_num = HISDIO_HOST2DEV_SCATT_MAX + 2;
    hi_sdio->scatt_info[SDIO_WRITE].sglist = kzalloc(sizeof(struct scatterlist) *
                                                     (hi_sdio->scatt_info[SDIO_WRITE].max_scatt_num),
                                                     GFP_KERNEL);
    if (hi_sdio->scatt_info[SDIO_WRITE].sglist == NULL) {
        goto failed_sdio_write_sg_alloc;
    }

    hi_sdio->sdio_align_buff = kzalloc(HISDIO_BLOCK_SIZE, GFP_KERNEL);
    if (hi_sdio->sdio_align_buff == NULL) {
        goto failed_sdio_align_buff_alloc;
    }
#ifdef CONFIG_HISDIO_H2D_SCATT_LIST_ASSEMBLE
    tx_scatt_buff_len = HISDIO_HOST2DEV_SCATT_SIZE + HISDIO_HOST2DEV_SCATT_MAX *
                        (HCC_HDR_TOTAL_LEN + OAL_ROUND_UP(HSDIO_HOST2DEV_PKTS_MAX_LEN, HISDIO_H2D_SCATT_BUFFLEN_ALIGN));
    tx_scatt_buff_len = hisdio_align_4_or_blk(tx_scatt_buff_len);
    hi_sdio->tx_scatt_buff.buff = oal_mem_dma_blockalloc(tx_scatt_buff_len, 5000); /* 5 seconds timeout */
    if (hi_sdio->tx_scatt_buff.buff == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc tx_scatt_buff failed,request %u bytes", tx_scatt_buff_len);
        goto failed_sdio_tx_scatt_buff_alloc;
    }
    hi_sdio->tx_scatt_buff.len = tx_scatt_buff_len;

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "alloc tx_scatt_buff ok,request %u bytes", tx_scatt_buff_len);
#endif

#ifdef CONFIG_HISDIO_D2H_SCATT_LIST_ASSEMBLE
    rx_scatt_buff_len = HISDIO_DEV2HOST_SCATT_MAX * (HCC_HDR_TOTAL_LEN + OAL_ROUND_UP(HSDIO_HOST2DEV_PKTS_MAX_LEN,
                                                                                      HISDIO_D2H_SCATT_BUFFLEN_ALIGN));
    rx_scatt_buff_len = hisdio_align_4_or_blk(rx_scatt_buff_len);
    hi_sdio->rx_scatt_buff.buff = oal_mem_dma_blockalloc(rx_scatt_buff_len, 5000); /* 5 seconds timeout */
    if (hi_sdio->rx_scatt_buff.buff == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc rx_scatt_buff failed,request %u bytes", rx_scatt_buff_len);
        goto failed_sdio_rx_scatt_buff_alloc;
    }
    hi_sdio->rx_scatt_buff.len = rx_scatt_buff_len;

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "alloc rx_scatt_buff ok,request %u bytes", rx_scatt_buff_len);
#endif

    return hi_sdio;
#ifdef CONFIG_HISDIO_D2H_SCATT_LIST_ASSEMBLE
failed_sdio_rx_scatt_buff_alloc:
#ifdef CONFIG_HISDIO_H2D_SCATT_LIST_ASSEMBLE
    oal_mem_dma_blockfree(hi_sdio->tx_scatt_buff.buff);
#endif
#endif
#ifdef CONFIG_HISDIO_H2D_SCATT_LIST_ASSEMBLE
failed_sdio_tx_scatt_buff_alloc:
    kfree(hi_sdio->sdio_align_buff);
#endif
failed_sdio_align_buff_alloc:
    kfree(hi_sdio->scatt_info[SDIO_WRITE].sglist);
failed_sdio_write_sg_alloc:
    kfree(hi_sdio->scatt_info[SDIO_READ].sglist);
failed_sdio_read_sg_alloc:
    kfree(hi_sdio->sdio_extend);
failed_sdio_extend_alloc:
    kfree(hi_sdio->rx_reserved_buff);
failed_rx_reserved_buff_alloc:
    kfree(hi_sdio);
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_SDIO, CHR_PLAT_DRV_ERROR_SDIO_MODULE);

    return NULL;
}
oal_module_symbol(oal_sdio_init_module);

void oal_sdio_exit_module(struct oal_sdio *hi_sdio)
{
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio module unregistered");
#ifdef CONFIG_HISDIO_D2H_SCATT_LIST_ASSEMBLE
    oal_mem_dma_blockfree(hi_sdio->rx_scatt_buff.buff);
#endif
#ifdef CONFIG_HISDIO_H2D_SCATT_LIST_ASSEMBLE
    oal_mem_dma_blockfree(hi_sdio->tx_scatt_buff.buff);
#endif
    kfree(hi_sdio->sdio_align_buff);
    kfree(hi_sdio->scatt_info[SDIO_WRITE].sglist);
    kfree(hi_sdio->scatt_info[SDIO_READ].sglist);
    kfree(hi_sdio->sdio_extend);
    kfree(hi_sdio->rx_reserved_buff);
    kfree(hi_sdio);
    g_hi110x_sdio = NULL;
#ifdef CONFIG_SDIO_DEBUG
    g_hi_sdio_debug = NULL;
#endif
}
oal_module_symbol(oal_sdio_exit_module);

/* ST */
void oal_sdio_tc_msg_001(int msg)
{
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "send msg 0x%8X", msg);
    wlan_pm_disable();
    oal_sdio_send_msg(g_hi_sdio_debug->pst_bus, msg);
    wlan_pm_enable();
}

void oal_sdio_tc_buf_tx_001(void)
{
    int32_t ret;
    void *buf = oal_memalloc(SDIO_TC_BUF_TX_LEN);
    if (buf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "oal_sdio_tc_buf_tx_001 memmalloc buf error");
        return;
    }
    memset_s(buf, SDIO_TC_BUF_TX_LEN, 0xff, SDIO_TC_BUF_TX_LEN);
    ret = oal_sdio_single_transfer(g_hi_sdio_debug, SDIO_WRITE, buf, SDIO_TC_BUF_TX_LEN);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "[WIFI] oal_sdio_tc_buf_tx_001 failed=%d", ret);
    }
}

void oal_sdio_tc_extend_001(void)
{
    int ret;
    char *buf = NULL;
    oal_netbuf_stru *netbuf = oal_netbuf_alloc(HISDIO_FUNC1_EXTEND_REG_LEN, 0, 0);
    if (netbuf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc netbuf fail");
        return;
    }
    oal_netbuf_put(netbuf, hisdio_align_4_or_blk(HISDIO_FUNC1_EXTEND_REG_LEN));
    buf = oal_netbuf_data(netbuf);
    memset_s(buf, oal_netbuf_len(netbuf), 0, oal_netbuf_len(netbuf));
    oal_sdio_claim_host(g_hi_sdio_debug);
    ret = oal_sdio_memcpy_fromio(g_hi_sdio_debug->func, buf, HISDIO_FUNC1_EXTEND_REG_BASE, HISDIO_FUNC1_EXTEND_REG_LEN);
    oal_sdio_release_host(g_hi_sdio_debug);
    if (ret) {
        printk("read failed ret=%d\n", ret);
    }
    print_hex_dump_bytes("extend :", DUMP_PREFIX_ADDRESS, buf, HISDIO_FUNC1_EXTEND_REG_LEN);
    oal_netbuf_free(netbuf);
}

void oal_sdio_tc_mem_cp_from(unsigned int offset, unsigned int len)
{
    int ret;
    char *buf = NULL;
    oal_netbuf_stru *netbuf = oal_netbuf_alloc(len, 0, 0);
    if (netbuf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "alloc fail");
        return;
    }
    oal_netbuf_put(netbuf, hisdio_align_4_or_blk(len));
    buf = oal_netbuf_data(netbuf);
    memset_s(buf, oal_netbuf_len(netbuf), 0, oal_netbuf_len(netbuf));
    oal_sdio_claim_host(g_hi_sdio_debug);
    ret = oal_sdio_memcpy_fromio(g_hi_sdio_debug->func, buf, offset, len);
    oal_sdio_release_host(g_hi_sdio_debug);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "read failed ret=%d", ret);
    }
    print_hex_dump_bytes("extend :", DUMP_PREFIX_ADDRESS, buf, len);
    oal_netbuf_free(netbuf);
}

void oal_sdio_read_func0(int offset, int len)
{
    int i;
    int j = 0;
    int ret = 0;
    char *buf = NULL;

    oal_netbuf_stru *netbuf = oal_netbuf_alloc(len, 0, 0);
    if (netbuf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "alloc fail");
        return;
    }

    if (g_hi_sdio_debug == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "g_hi_sdio_debug is null");
        oal_netbuf_free(netbuf);
        return;
    }

    oal_netbuf_put(netbuf, len);
    buf = oal_netbuf_data(netbuf);
    memset_s(buf, oal_netbuf_len(netbuf), 0, oal_netbuf_len(netbuf));
    for (i = 0; i < len; i++) {
        oal_sdio_claim_host(g_hi_sdio_debug);
        *(buf + i) = sdio_f0_readb(g_hi_sdio_debug->func, offset + i, &ret);
        oal_sdio_release_host(g_hi_sdio_debug);
        if (ret) {
            printk("read error ret=%d\n", ret);
            *(buf + i) = 0;
            break;
        }
        j++;
    }
    if (i != j) {
        printk("request read %d bytes, but only read %d bytes\n", len, j);
    }
    printk("##################%s##########[offset:0x%X, len:0x%X]\n", __FUNCTION__, offset, len);
    print_hex_dump_bytes("read_func0: ", DUMP_PREFIX_ADDRESS, buf, j);
    oal_netbuf_free(netbuf);
}

void oal_sdio_read_func1(int offset, int len)
{
    int i;
    int j = 0;
    int ret = 0;
    char *buf = NULL;
    oal_netbuf_stru *netbuf = oal_netbuf_alloc(len, 0, 0);
    if (netbuf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc fail");
        return;
    }
    oal_netbuf_put(netbuf, len);
    buf = oal_netbuf_data(netbuf);
    memset_s(buf, oal_netbuf_len(netbuf), 0, oal_netbuf_len(netbuf));
    for (i = 0; i < len; i++) {
        oal_sdio_claim_host(g_hi_sdio_debug);
        *(buf + i) = oal_sdio_readb(g_hi_sdio_debug->func, offset + i, &ret);
        oal_sdio_release_host(g_hi_sdio_debug);
        if (ret) {
            printk("read error ret=%d\n", ret);
            *(buf + i) = 0;
            break;
        }
        j++;
    }
    if (i != j) {
        printk("request read %d bytes, but only read %d bytes\n", len, j);
    }
    printk("##################%s##########[offset:0x%X, len:0x%X]\n", __FUNCTION__, offset, len);
    print_hex_dump_bytes("read_func0: ", DUMP_PREFIX_ADDRESS, buf, j);
    oal_netbuf_free(netbuf);
}

void oal_sdio_readsb_test(int offset, int len)
{
    int ret;
    char *buf = NULL;
    oal_netbuf_stru *netbuf = oal_netbuf_alloc(len, 0, 0);
    if (netbuf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc fail");
        return;
    }
    oal_netbuf_put(netbuf, len);
    buf = oal_netbuf_data(netbuf);
    memset_s(buf, oal_netbuf_len(netbuf), 0, oal_netbuf_len(netbuf));
    oal_sdio_claim_host(g_hi_sdio_debug);
    ret = oal_sdio_readsb(g_hi_sdio_debug->func, buf, offset, len);
    oal_sdio_release_host(g_hi_sdio_debug);
    if (ret) {
        printk("read failed ret=%d\n", ret);
    }
    printk("##################%s##########[offset:0x%X, len:0x%X]\n", __FUNCTION__, offset, len);
    print_hex_dump_bytes("oal readsb :", DUMP_PREFIX_ADDRESS, buf, len);
    oal_netbuf_free(netbuf);
}

#ifdef _PRE_COMMENT_CODE_
void oal_sdio_sched_001(void)
{
    if (g_hi_sdio_debug == NULL) {
        return;
    }
    printk("##################%s##########\n", __FUNCTION__);
    if (g_hisdio_intr_mode) {
        up(&g_hi_sdio_debug->gpio_rx_sema);
    } else {
        mmc_signal_sdio_irq(g_hi_sdio_debug->func->card->host);
    }
}
#endif

void oal_sdio_dump_extend_buf(void)
{
    if (g_hi_sdio_debug != NULL) {
        oal_sdio_claim_host(g_hi_sdio_debug);
        oal_sdio_extend_buf_get(g_hi_sdio_debug);
        oal_sdio_release_host(g_hi_sdio_debug);
#ifdef CONFIG_SDIO_DEBUG
        printk(KERN_INFO "=========extend buff:%d=====\n",
               hisdio_comm_reg_seq_get(g_hi_sdio_debug->sdio_extend->credit_info));
        print_hex_dump_bytes("extend :", DUMP_PREFIX_ADDRESS,
                             (void *)g_hi_sdio_debug->sdio_extend, sizeof(struct hisdio_extend_func));
#endif
    }
}

uint32_t oal_sdio_func_max_req_size(struct oal_sdio *pst_hi_sdio)
{
    uint32_t max_blocks;
    uint32_t size, size_device;

    if (oal_warn_on(pst_hi_sdio == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: pst_hi_sdio is null", __FUNCTION__);
        return 0;
    };
    /*
     * host transer limit
     * Blocks per command is limited by host count, host transfer
     * size and the maximum for IO_RW_EXTENDED of 511 blocks.
     */
    max_blocks = oal_min(pst_hi_sdio->func->card->host->max_blk_count, 511u);
    size = max_blocks * HISDIO_BLOCK_SIZE;

    size = oal_min(size, pst_hi_sdio->func->card->host->max_req_size);

    /*
     * device transer limit,per adma descr limit 32K in bootloader,
     * and total we have 20 descs
     */
    size_device = (32 * 1024) * 20;

    size = oal_min(size, size_device);
    return size;
}

/*
 * 函 数 名  : oal_sdio_gpio_flowctrl_irq
 * 功能描述  : oal层GPIO流控中断回调函数, 调用hcc bus提供的接口设置当前流控状态
 */
OAL_STATIC int32_t oal_sdio_gpio_flowctrl_irq(hcc_bus *pst_hi_bus, int32_t l_irq)
{
    int32_t l_gpio_val;
    struct hcc_handler *pst_hcc = NULL;

    if (oal_unlikely(pst_hi_bus == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, KERN_ERR"sdio bus is null, irq:%d", l_irq);
        return -OAL_EINVAL;
    }

    pst_hcc = pst_hi_bus->bus_dev->hcc;

    if (oal_unlikely(pst_hcc == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc handler is null!");
        return -OAL_EFAIL;
    }

    /* 获取流控GPIO管脚当前电平 */
    l_gpio_val = oal_gpio_get_value(g_st_board_info.flowctrl_gpio);
    if (l_gpio_val == 1) {
        /* 高电平, 流控开启, 无法发送 */
        hcc_dev_flowctrl_off(pst_hcc);
    } else if (l_gpio_val == 0) {
        /* 低电平, 流控关闭, 可以发送 */
        hcc_dev_flowctrl_on(pst_hcc, OAL_FALSE);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "invalid gpio value = %d!", l_gpio_val);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

/*
 * Prototype    : oal_sdio_gpio_irq
 * Description  : wlan irq process function
 */
OAL_STATIC int32_t oal_sdio_gpio_irq(hcc_bus *hi_bus, int32_t irq)
{
    oal_uint ul_state;
    if (oal_unlikely(hi_bus == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, KERN_ERR "sdio bus is null, irq:%d", irq);
        return -OAL_EINVAL;
    }

    if (!hi_bus->pst_pm_callback || !hi_bus->pst_pm_callback->pm_state_get) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "GPIO interrupt function param is NULL");
        return -OAL_EINVAL;
    }

    hi_bus->gpio_int_count++;

    if (oal_atomic_read(&g_wakeup_dev_wait_ack)) {
        hi_bus->pst_pm_callback->pm_wakeup_dev_ack();
    }

    ul_state = hi_bus->pst_pm_callback->pm_state_get();
    if (ul_state == 0) {
        /* 0==HOST_DISALLOW_TO_SLEEP表示不允许休眠 */
        hi_bus->data_int_count++;

        oal_print_hi11xx_log(HI11XX_LOG_DBG, "Gpio Rx Data Interrupt.");

        up(&hi_bus->rx_sema);
    } else {
        /* 1==HOST_ALLOW_TO_SLEEP表示当前是休眠，唤醒host */
        if (oal_warn_on(!hi_bus->pst_pm_callback->pm_wakeup_host)) {
            oal_print_hi11xx_log(HI11XX_LOG_DBG, "%s error:hi_bus->pst_pm_callback->pm_wakeup_host is null",
                                 __FUNCTION__);
            return -OAL_FAIL;
        }
        hi_bus->wakeup_int_count++;
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "Gpio Wakeup Interrrupt %llu,data intr %llu \r",
                             hi_bus->wakeup_int_count, hi_bus->gpio_int_count);
        hi_bus->pst_pm_callback->pm_wakeup_host();
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_sdio_gpio_rx_data(hcc_bus *hi_bus)
{
    return oal_sdio_rxdata_proc((struct oal_sdio *)hi_bus->data);
}

OAL_STATIC int32_t oal_sdio_wakeup_complete(hcc_bus *pst_bus)
{
#ifdef _PRE_PLAT_FEATURE_HI110X_SDIO_GPIO_WAKE
#endif
    return OAL_SUCC;
}

OAL_STATIC hcc_bus_opt_ops g_sdio_opt_ops = {
    .get_bus_state = oal_sdio_get_state,
    .disable_bus_state = oal_disable_sdio_state,
    .enable_bus_state = oal_enable_sdio_state,
    .rx_netbuf_list = oal_sdio_rx_netbuf,
    .tx_netbuf_list = oal_sdio_tx_netbuf,
    .send_msg = oal_sdio_send_msg,
    .lock = oal_sdio_host_lock,
    .unlock = oal_sdio_host_unlock,
    .sleep_request = oal_sdio_sleep_request,
    .sleep_request_host = oal_sdio_sleep_request_host,
    .wakeup_request = oal_sdio_wakeup_request,
    .get_sleep_state = oal_sdio_get_sleep_state,
    .wakeup_complete = oal_sdio_wakeup_complete,
    .rx_int_mask = oal_sdio_rx_int_mask,
    .power_action = oal_sdio_power_action,
    .reinit = oal_sdio_reinit,
    .deinit = NULL,
    .wlan_gpio_handler = oal_sdio_gpio_irq,
    .flowctrl_gpio_handler = oal_sdio_gpio_flowctrl_irq,
    .wlan_gpio_rxdata_proc = oal_sdio_gpio_rx_data,
    .patch_read = oal_sdio_patch_read,
    .patch_write = oal_sdio_patch_write,
    .bindcpu = oal_sdio_bindcpu,
    .chip_info = NULL,
    .print_trans_info = NULL,
    .reset_trans_info = NULL,
    .pending_signal_check = NULL,
    .pending_signal_process = NULL
};

/* add sdio to bus */
OAL_STATIC hcc_bus *oal_sdio_bus_init(struct oal_sdio *hi_sdio)
{
    int32_t ret;
    hcc_bus *pst_bus = NULL;

    pst_bus = hcc_alloc_bus();
    if (pst_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc sdio hcc bus failed, size:%u", (uint32_t)sizeof(hcc_bus));
        return NULL;
    }

    pst_bus->bus_type = HCC_BUS_SDIO;
    pst_bus->bus_id = 0x0;
    pst_bus->dev_id = HCC_CHIP_110X_DEV;

    pst_bus->opt_ops = &g_sdio_opt_ops;

    pst_bus->cap.align_size[HCC_TX] = HISDIO_H2D_SCATT_BUFFLEN_ALIGN;
    pst_bus->cap.align_size[HCC_RX] = HISDIO_D2H_SCATT_BUFFLEN_ALIGN;
    pst_bus->cap.max_trans_size = oal_sdio_func_max_req_size(hi_sdio);

    pst_bus->data = (void *)hi_sdio;

    ret = hcc_add_bus(pst_bus, "sdio");
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "add sdio bus failed, ret=%d", ret);
        hcc_free_bus(pst_bus);
        return NULL;
    }

    hi_sdio->pst_bus = pst_bus;

    return pst_bus;
}

OAL_STATIC void oal_sdio_bus_exit(struct oal_sdio *hi_sdio)
{
    if (hi_sdio->pst_bus == NULL) {
        return;
    }
    hcc_remove_bus(hi_sdio->pst_bus);
    hcc_free_bus(hi_sdio->pst_bus);
}

int32_t oal_sdio_110x_working_check(void)
{
    hcc_bus_dev *pst_bus_dev;
    pst_bus_dev = hcc_get_bus_dev(HCC_CHIP_110X_DEV);
    if (pst_bus_dev == NULL) {
        return OAL_FALSE;
    }

    if (pst_bus_dev->bus_cap & HCC_BUS_SDIO_CAP) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}

int32_t oal_wifi_platform_load_sdio(void)
{
    if (oal_sdio_110x_working_check() != OAL_TRUE) {
        /* sdio driver don't support */
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio driver don't support");
        return OAL_SUCC;
    }
#ifdef CONFIG_MMC
    if (oal_sdio_trigger_probe() != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "sdio first probe failed!");
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_SDIO, CHR_PLAT_DRV_ERROR_SDIO_ENUM);

        return -OAL_ENODEV;
    }
#endif
    return OAL_SUCC;
}

void oal_wifi_platform_unload_sdio(void)
{
    if (oal_sdio_110x_working_check() != OAL_TRUE) {
        return;
    }
#ifdef CONFIG_MMC
    if (g_hi110x_sdio != NULL) {
        oal_sdio_func_remove(g_hi110x_sdio);
    }
#endif
}

void oal_netbuf_list_hex_dump(oal_netbuf_head_stru *head)
{
#ifdef CONFIG_PRINTK
    int32_t index = 0;
    oal_netbuf_stru *netbuf = NULL;
    oal_netbuf_stru *tmp = NULL;
    if (!skb_queue_len(head)) {
        return;
    }
    printk(KERN_INFO "prepare to dump %d pkts=========\n", skb_queue_len(head));
    skb_queue_walk_safe(head, netbuf, tmp)
    {
        index++;
        printk(KERN_INFO "======netbuf pkts %d, len:%d=========\n", index, oal_netbuf_len(netbuf));
        print_hex_dump_bytes("netbuf  :", DUMP_PREFIX_ADDRESS, oal_netbuf_data(netbuf), oal_netbuf_len(netbuf));
    }
#else
    oal_reference(head);
#endif
}

void oal_netbuf_hex_dump(oal_netbuf_stru *netbuf)
{
#ifdef CONFIG_PRINTK
    printk(KERN_INFO "==prepare to netbuf,%p,len:%d=========\n",
           oal_netbuf_data(netbuf),
           oal_netbuf_len(netbuf));
    print_hex_dump_bytes("netbuf  :", DUMP_PREFIX_ADDRESS, oal_netbuf_data(netbuf), oal_netbuf_len(netbuf));
#else
    oal_reference(netbuf);
#endif
}

/*lint -e19*/
oal_module_symbol(oal_get_gpio_int_count_para);
/*lint +e19*/
uint32_t oal_sdio_get_large_pkt_free_cnt(struct oal_sdio *hi_sdio)
{
    uint32_t free_cnt;
    if (oal_warn_on(!hi_sdio)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: hi_sdio is null", __FUNCTION__);
        return 0;
    };
    oal_spin_lock(&hi_sdio->sdio_credit_info.credit_lock);
    free_cnt = (uint32_t)hi_sdio->sdio_credit_info.large_free_cnt;
    oal_spin_unlock(&hi_sdio->sdio_credit_info.credit_lock);
    return free_cnt;
}

#else
struct oal_sdio g_hi_sdio_ut;
int32_t oal_wifi_platform_load_sdio(void)
{
    return OAL_SUCC;
}

void oal_wifi_platform_unload_sdio(void)
{
    return;
}
#endif
