

#ifdef _PRE_CONFIG_HISI_CONN_SOFTWDFT
#define HISI_LOG_TAG "[SOFT_WDT]"
#include "oal_util.h"
#include "oal_net.h"

#include "oal_schedule.h"
#include "oal_workqueue.h"
#include "securec.h"

/* 4s timeout,8s bug_on */
#define OAL_SOFTWDT_DEFAULT_TIMEOUT (4000) /* 4s */

typedef struct _hisi_conn_softwdt_ {
    oal_uint32 kick_time;
    oal_delayed_work wdt_delayed_work;
    oal_workqueue_stru *wdt_wq;
    oal_timer_list_stru wdt_timer;
    oal_uint32 wdt_timeout_count;
} hisi_conn_softwdt;

OAL_STATIC hisi_conn_softwdt hisi_softwdt;

#ifdef PLATFORM_SSI_FULL_LOG
OAL_STATIC oal_int32 disable_wdt_flag = 1;
#else
OAL_STATIC oal_int32 disable_wdt_flag = 0;
#endif
oal_debug_module_param(disable_wdt_flag, int, S_IRUGO | S_IWUSR);

OAL_STATIC oal_void oal_softwdt_kick(oal_void)
{
    oal_timer_start(&hisi_softwdt.wdt_timer, OAL_SOFTWDT_DEFAULT_TIMEOUT);
}

OAL_STATIC oal_void oal_softwdt_timeout(oal_ulong data)
{
    OAL_REFERENCE(data);
    hisi_softwdt.wdt_timeout_count++;

    if (hisi_softwdt.wdt_timeout_count == 1) {
        /* 第一次超时 */
#ifdef CONFIG_PRINTK
        printk(KERN_WARNING "hisi softwdt timeout first time,keep try...\n");
#else
        OAL_IO_PRINT("hisi softwdt timeout first time,keep try...\n");
#endif
        oal_softwdt_kick();
    }

    if (hisi_softwdt.wdt_timeout_count >= 2) {
        /* 第二次及以上次超时 */
#ifdef CONFIG_PRINTK
        printk(KERN_EMERG "[E]hisi softwdt timeout second time, dump system stack\n");
#else
        OAL_IO_PRINT("hisi softwdt timeout second time, dump system stack!\n");
#endif
        DECLARE_DFT_TRACE_KEY_INFO("oal_softwdt_timeout", OAL_DFT_TRACE_EXCEP);
        OAL_WARN_ON(1);
        hisi_softwdt.wdt_timeout_count = 0;
    }

    return;
}

OAL_STATIC oal_void oal_softwdt_feed_task(oal_work_stru *pst_work)
{
    oal_softwdt_kick();
    oal_queue_delayed_work_on(0, hisi_softwdt.wdt_wq, &hisi_softwdt.wdt_delayed_work,
                              OAL_MSECS_TO_JIFFIES(hisi_softwdt.kick_time));
    return;
}

oal_void oal_softwdt_abort_kick(oal_void)
{
    oal_cancel_delayed_work_sync(&hisi_softwdt.wdt_delayed_work);
    OAL_IO_PRINT("oal_softwdt_abort_kick done\n");
    return;
}

oal_int32 oal_softwdt_init(oal_void)
{
    if (disable_wdt_flag) {
        return OAL_SUCC;
    }

    memset_s((oal_void *)&hisi_softwdt, OAL_SIZEOF(hisi_softwdt), 0, OAL_SIZEOF(hisi_softwdt));
    oal_timer_init(&hisi_softwdt.wdt_timer, OAL_SOFTWDT_DEFAULT_TIMEOUT, oal_softwdt_timeout, 0);
    hisi_softwdt.kick_time = OAL_SOFTWDT_DEFAULT_TIMEOUT / 2; /* 4s/2 = 2s */
    OAL_INIT_DELAYED_WORK(&hisi_softwdt.wdt_delayed_work, oal_softwdt_feed_task);
    hisi_softwdt.wdt_wq = oal_create_workqueue("softwdt");
    if (OAL_UNLIKELY(hisi_softwdt.wdt_wq == NULL)) {
        OAL_IO_PRINT("hisi soft wdt create workqueue failed!\n");
        return -OAL_ENOMEM;
    }

    /* start wdt */
    oal_queue_delayed_work_on(0, hisi_softwdt.wdt_wq, &hisi_softwdt.wdt_delayed_work, 0);
    return OAL_SUCC;
}
oal_module_symbol(oal_softwdt_init);

oal_void oal_softwdt_exit(oal_void)
{
    if (disable_wdt_flag) {
        return;
    }

    oal_timer_delete_sync(&hisi_softwdt.wdt_timer);
    oal_cancel_delayed_work_sync(&hisi_softwdt.wdt_delayed_work);
    oal_destroy_workqueue(hisi_softwdt.wdt_wq);
}
oal_module_symbol(oal_softwdt_exit);
#endif

