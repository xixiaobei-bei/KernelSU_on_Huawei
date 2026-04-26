

#include "oam_rdr.h"

#include <linux/workqueue.h>
#include <linux/errno.h>
#include <linux/rtc.h>

#include "plat_exception_rst.h"
#include "securec.h"
#include "oal_util.h"
#if (defined CONFIG_HISI_BB) && (defined CONFIG_HI110X_PLAT_BB)
#include "mntn_subtype_exception.h"

OAL_STATIC struct rdr_exception_info_s g_hisi_conn_excetption_info[] = {
    {
        .e_modid = (u32)MODID_CONN_WIFI_EXEC,
        .e_modid_end = (u32)MODID_CONN_WIFI_EXEC,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_CONN,
        .e_reset_core_mask = RDR_CONN,
        .e_from_core = RDR_CONN,
        .e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type = WIFI_S_EXCEPTION,
        .e_exce_subtype = CONN_WIFI_EXEC,
        .e_upload_flag = (u32)RDR_UPLOAD_YES,
        .e_from_module = "CONN",
        .e_desc = "CONN_WIFI_EXEC",
    },
    {
        .e_modid = (u32)MODID_CONN_WIFI_CHAN_EXEC,
        .e_modid_end = (u32)MODID_CONN_WIFI_CHAN_EXEC,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_CONN,
        .e_reset_core_mask = RDR_CONN,
        .e_from_core = RDR_CONN,
        .e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type = WIFI_S_EXCEPTION,
        .e_exce_subtype = CONN_WIFI_CHAN_EXEC,
        .e_upload_flag = (u32)RDR_UPLOAD_YES,
        .e_from_module = "CONN",
        .e_desc = "CONN_WIFI_CHAN_EXEC",
    },
    {
        .e_modid = (u32)MODID_CONN_WIFI_WAKEUP_FAIL,
        .e_modid_end = (u32)MODID_CONN_WIFI_WAKEUP_FAIL,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_CONN,
        .e_reset_core_mask = RDR_CONN,
        .e_from_core = RDR_CONN,
        .e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type = WIFI_S_EXCEPTION,
        .e_exce_subtype = CONN_WIFI_WAKEUP_FAIL,
        .e_upload_flag = (u32)RDR_UPLOAD_YES,
        .e_from_module = "CONN",
        .e_desc = "CONN_WIFI_WAKEUP_FAIL",
    },
    {
        .e_modid = (u32)MODID_CONN_BFGX_EXEC,
        .e_modid_end = (u32)MODID_CONN_BFGX_EXEC,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_CONN,
        .e_reset_core_mask = RDR_CONN,
        .e_from_core = RDR_CONN,
        .e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type = BFGX_S_EXCEPTION,
        .e_exce_subtype = CONN_BFGX_EXEC,
        .e_upload_flag = (u32)RDR_UPLOAD_YES,
        .e_from_module = "CONN",
        .e_desc = "CONN_BFGX_EXEC",
    },
    {
        .e_modid = (u32)MODID_CONN_BFGX_BEAT_TIMEOUT,
        .e_modid_end = (u32)MODID_CONN_BFGX_BEAT_TIMEOUT,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_CONN,
        .e_reset_core_mask = RDR_CONN,
        .e_from_core = RDR_CONN,
        .e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type = BFGX_S_EXCEPTION,
        .e_exce_subtype = CONN_BFGX_BEAT_TIMEOUT,
        .e_upload_flag = (u32)RDR_UPLOAD_YES,
        .e_from_module = "CONN",
        .e_desc = "CONN_BFGX_BEAT_TIMEOUT",
    },
    {
        .e_modid = (u32)MODID_CONN_BFGX_WAKEUP_FAIL,
        .e_modid_end = (u32)MODID_CONN_BFGX_WAKEUP_FAIL,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_CONN,
        .e_reset_core_mask = RDR_CONN,
        .e_from_core = RDR_CONN,
        .e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type = BFGX_S_EXCEPTION,
        .e_exce_subtype = CONN_BFGX_WAKEUP_FAIL,
        .e_upload_flag = (u32)RDR_UPLOAD_YES,
        .e_from_module = "CONN",
        .e_desc = "CONN_BFGX_WAKEUP_FAIL",
    }
};
OAL_STATIC hisi_conn_ctl_t g_hisi_conn_modid_cfg[MODID_CONN_ARRAY_LEN] = {
    /* MODID_CONN_WIFI_EXEC */
    {
        .upload_en = UPLOAD_ALLOW,
        .interva_stime = HISI_CONN_INTERVAL_TIME,
    },
    /* MODID_CONN_WIFI_CHAN_EXEC */
    {
        .upload_en = UPLOAD_ALLOW,
        .interva_stime = HISI_CONN_INTERVAL_TIME,
    },
    /* MODID_CONN_WIFI_WAKEUP_FAIL */
    {
        .upload_en = UPLOAD_ALLOW,
        .interva_stime = HISI_CONN_INTERVAL_TIME,
    },
    /* MODID_CONN_BFGX_EXEC */
    {
        .upload_en = UPLOAD_ALLOW,
        .interva_stime = HISI_CONN_INTERVAL_TIME,
    },
    /* MODID_CONN_BFGX_BEAT_TIMEOUT */
    {
        .upload_en = UPLOAD_ALLOW,
        .interva_stime = HISI_CONN_INTERVAL_TIME,
    },
    /* MODID_CONN_BFGX_WAKEUP_FAIL */
    {
        .upload_en = UPLOAD_ALLOW,
        .interva_stime = HISI_CONN_INTERVAL_TIME,
    },
};
OAL_STATIC int32 g_initdone = 0;
OAL_STATIC int32 g_hisi_conn_initdone(void);
OAL_STATIC struct work_struct g_hisi_conn_dump_work;
OAL_STATIC struct workqueue_struct *g_hisi_conn_rdr_wq;
OAL_STATIC struct hisi_conn_info_s g_hisi_conn_info;

int32 hisi_conn_stat_init(void)
{
    int32 i;
    struct timex txc;

    do_gettimeofday(&(txc.time));
    txc.time.tv_sec -= sys_tz.tz_minuteswest * OAL_SEC_PER_MIN;
    for (i = 0; i < MODID_CONN_ARRAY_LEN; i++) {
        g_hisi_conn_modid_cfg[i].stat_info.lastuploadtime = txc.time.tv_sec;
        g_hisi_conn_modid_cfg[i].stat_info.happen_cnt = 0;
        g_hisi_conn_modid_cfg[i].stat_info.upload_cnt = 0;
    }
    return 0;
}
void _hisi_conn_rdr_system_error(uint32 modid, uint32 arg1, uint32 arg2)
{
    unsigned long long subtime;
    int32 modid_array;
    struct timex txc;
    ;
    modid_array = modid - MODID_CONN_WIFI_EXEC;

    if (g_hisi_conn_modid_cfg[modid_array].upload_en == UPLOAD_DISALLOW) {
        ps_print_info("[%s]:upload modid[0x%x] not allow ship,value:%d!\n",
                      __func__, modid, g_hisi_conn_modid_cfg[modid_array].upload_en);
        return;
    }

    do_gettimeofday(&(txc.time));
    txc.time.tv_sec -= sys_tz.tz_minuteswest * OAL_SEC_PER_MIN;
    subtime = txc.time.tv_sec - g_hisi_conn_modid_cfg[modid_array].stat_info.lastuploadtime;
    if (g_hisi_conn_modid_cfg[modid_array].interva_stime < subtime) {
        ps_print_info("[%s]:upload bbox[0x%x]!\n", __func__, modid);
        rdr_system_error(modid, arg1, arg2);
        g_hisi_conn_modid_cfg[modid_array].stat_info.lastuploadtime = txc.time.tv_sec;
        g_hisi_conn_modid_cfg[modid_array].stat_info.upload_cnt++;
    } else {
        ps_print_warning("[%s]:upload bbox error[0x%x],cur_interva_stime:%lld,set interva_stime:%lld!\n",
                         __func__, modid, subtime, g_hisi_conn_modid_cfg[modid_array].interva_stime);
    }
    g_hisi_conn_modid_cfg[modid_array].stat_info.happen_cnt++;
}
int32 hisi_conn_rdr_system_error(uint32 modid, uint32 arg1, uint32 arg2)
{
    if ((modid < MODID_CONN_WIFI_EXEC) || (modid >= MODID_CONN_BOTT)) {
        ps_print_err("[%s]:Input parameter is error[0x%x]!\n", __func__, modid);
        return -EINVAL;
    } else {
        if (!g_hisi_conn_initdone()) {
            _hisi_conn_rdr_system_error(modid, arg1, arg2);
        }
    }
    return 0;
}
void plat_bbox_msg_hander(int32 subsys_type, int32 exception_type)
{
    if (subsys_type == SUBSYS_BFGX) {
        switch (exception_type) {
            case BFGX_LASTWORD_PANIC:
                rdr_exception(MODID_CONN_BFGX_EXEC);
                break;
            default:
                break;
        }
    } else if (subsys_type == SUBSYS_WIFI) {
        switch (exception_type) {
            case WIFI_WATCHDOG_TIMEOUT:
            case BFGX_TIMER_TIMEOUT:
            case WIFI_DEVICE_PANIC:
                rdr_exception(MODID_CONN_WIFI_EXEC);
                break;
            default:
                break;
        }
    }
}
static int hisi_conn_copy_reg_to_bbox(char *src_addr, unsigned int len)
{
    unsigned int temp_offset;

    if ((src_addr == NULL) || (len == 0)) {
        ps_print_err("[%s]:HISI_CONN_ERROR:Input parameter is error!\n", __func__);
        return -EINVAL;
    }

    temp_offset = g_hisi_conn_info.bbox_addr_offset + len;

    // hisi_conn_bbox alloc size 8k
    if (memcpy_s(((char *)g_hisi_conn_info.rdr_addr + g_hisi_conn_info.bbox_addr_offset),
                 g_hisi_conn_info.hisi_conn_ret_info.log_len - g_hisi_conn_info.bbox_addr_offset,
                 src_addr, len) != EOK) {
        ps_print_err("[%s]:HISI_CONN_ERROR:Copy log to bbox size is error! temp_offset=%u\n, max len is %u",
                     __func__, temp_offset, g_hisi_conn_info.hisi_conn_ret_info.log_len);
        temp_offset = 0;
        g_hisi_conn_info.bbox_addr_offset = 0;
        return -ENOMEM;
    }
    g_hisi_conn_info.bbox_addr_offset = temp_offset;

    return 0;
}
int32 hisi_conn_save_stat_info(char *buf, int32 index, int32 limit)
{
    int32 i;
    int32 ret;
    struct rtc_time tm;
    ret = snprintf_s(buf + index, limit - index + 1, limit - index, "==========curr bbox info:\n");
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return index;
    }
    index += ret;
    for (i = 0; i < MODID_CONN_ARRAY_LEN; i++) {
        rtc_time_to_tm(g_hisi_conn_modid_cfg[i].stat_info.lastuploadtime, &tm);
        ret = snprintf_s(buf + index, limit - index + 1, limit - index,
                         "   id:%-10d  upload_en:%d    upload_cnt:%llu    happen_cnt:%llu    interva_stime:%llus   lastuploadtime:%4d-%02d-%02d  %02d:%02d:%02d\n",
                         i,
                         g_hisi_conn_modid_cfg[i].upload_en,
                         g_hisi_conn_modid_cfg[i].stat_info.upload_cnt,
                         g_hisi_conn_modid_cfg[i].stat_info.happen_cnt,
                         g_hisi_conn_modid_cfg[i].interva_stime,
                         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                         tm.tm_hour, tm.tm_min, tm.tm_sec); /* 转换成当前时间 */
        if (ret < 0) {
            OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
            return index;
        }
        index += ret;
    }
    return index;
}
void hisi_conn_save_bbox_info(void)
{
    char log_buf[HISI_CONN_BUF_LEN_MAX + 1] = {0};

    memset_s(g_hisi_conn_info.rdr_addr, g_hisi_conn_info.hisi_conn_ret_info.log_len,
             0, g_hisi_conn_info.hisi_conn_ret_info.log_len);

    hisi_conn_save_stat_info(log_buf, 0, HISI_CONN_BUF_LEN_MAX);

    hisi_conn_copy_reg_to_bbox(log_buf, strlen(log_buf));

    return;
}
static void hisi_conn_write_reg_log(void)
{
    switch (g_hisi_conn_info.dump_info.modid) {
        case MODID_CONN_WIFI_EXEC:
        case MODID_CONN_WIFI_CHAN_EXEC:
        case MODID_CONN_WIFI_WAKEUP_FAIL:
        case MODID_CONN_BFGX_EXEC:
        case MODID_CONN_BFGX_BEAT_TIMEOUT:
        case MODID_CONN_BFGX_WAKEUP_FAIL:
            break;
        default:
            break;
    }
    hisi_conn_save_bbox_info();
    return;
}
static void hisi_conn_rdr_dump(u32 modid, u32 etype, u64 coreid, char *pathname, pfn_cb_dump_done pfn_cb)
{
    if (pathname == NULL) {
        ps_print_err("[%s]:HISI_CONN_ERROR:pathname is empty\n", __func__);
        return;
    }
    if (!g_initdone) {
        ps_print_warning("[%s]:HISI_CONN_ERROR: rdr not init\n", __func__);
        return;
    }
    g_hisi_conn_info.dump_info.modid = modid;
    g_hisi_conn_info.dump_info.coreid = coreid;
    g_hisi_conn_info.dump_info.pathname = pathname;
    g_hisi_conn_info.dump_info.cb = pfn_cb;
    g_hisi_conn_info.bbox_addr_offset = 0;
    queue_work(g_hisi_conn_rdr_wq, &g_hisi_conn_dump_work);
    return;
}
static void hisi_conn_rdr_reset(u32 modid, u32 etype, u64 coreid)
{
    return;
}
static void hisi_conn_rdr_dump_work(struct work_struct *work)
{
    hisi_conn_write_reg_log();

    if (g_hisi_conn_info.dump_info.cb) {
        g_hisi_conn_info.dump_info.cb(g_hisi_conn_info.dump_info.modid, g_hisi_conn_info.dump_info.coreid);
    }

    return;
}
static int hisi_conn_register_exception(void)
{
    int ret;
    unsigned int size;
    unsigned long index;

    size = sizeof(g_hisi_conn_excetption_info) / sizeof(struct rdr_exception_info_s);
    for (index = 0; index < size; index++) {
        /* error return 0, ok return modid */
        ret = rdr_register_exception(&g_hisi_conn_excetption_info[index]);
        if (!ret) {
            ps_print_err("[%s]:HISI_CONN_ERROR:rdr_register_exception is failed! index=%ld ret=%d\n",
                         __func__, index, ret);
            return -EINTR;
        }
    }

    return 0;
}
static int hisi_conn_register_core(void)
{
    int ret;
    struct rdr_module_ops_pub s_soc_ops;

    s_soc_ops.ops_dump = hisi_conn_rdr_dump;
    s_soc_ops.ops_reset = hisi_conn_rdr_reset;
    /* register hisi_conn core dump and reset function */
    ret = rdr_register_module_ops((u64)RDR_CONN, &s_soc_ops, &g_hisi_conn_info.hisi_conn_ret_info);
    if (ret != 0) {
        ps_print_err("[%s]:HISI_CONN_ERROR:rdr_register_module_ops is failed! ret=0x%08x\n", __func__, ret);
    }

    return ret;
}
static int hisi_conn_addr_map(void)
{
#ifdef CONFIG_ARCH_PLATFORM
    g_hisi_conn_info.rdr_addr = dfx_bbox_map((phys_addr_t)g_hisi_conn_info.hisi_conn_ret_info.log_addr,
                                             g_hisi_conn_info.hisi_conn_ret_info.log_len);
#else
    g_hisi_conn_info.rdr_addr = hisi_bbox_map((phys_addr_t)g_hisi_conn_info.hisi_conn_ret_info.log_addr,
                                              g_hisi_conn_info.hisi_conn_ret_info.log_len);
#endif
    if (!g_hisi_conn_info.rdr_addr) {
        ps_print_err("[%s]:HISI_CONN_ERROR:hisi_bbox_map is failed!\n", __func__);
        return -EFAULT;
    }

    return 0;
}
static int hisi_conn_remove_map(void)
{
#ifdef CONFIG_ARCH_PLATFORM
    dfx_bbox_unmap ((char *)(g_hisi_conn_info.hisi_conn_ret_info.log_addr));
#else
    hisi_bbox_unmap ((char *)(g_hisi_conn_info.hisi_conn_ret_info.log_addr));
#endif
    return 0;
}
static int hisi_conn_rdr_resource_init(void)
{
    g_hisi_conn_rdr_wq = create_singlethread_workqueue("g_hisi_conn_rdr_wq");
    if (!g_hisi_conn_rdr_wq) {
        ps_print_err("[%s]:HISI_CONN_ERROR:Create_singlethread_workqueue is failed!\n", __func__);
        return -EINTR;
    }

    INIT_WORK(&g_hisi_conn_dump_work, hisi_conn_rdr_dump_work);

    return 0;
}
int hisi_conn_rdr_init(void)
{
    int ret;

    ret = hisi_conn_rdr_resource_init();
    if (ret != 0) {
        ps_print_err("[%s]:HISI_CONN_ERROR:hisi_conn_rdr_resource_init is faild!ret=%d\n", __func__, ret);
        return ret;
    }

    /* register ics exception */
    ret = hisi_conn_register_exception();
    if (ret != 0) {
        ps_print_err("[%s]:HISI_CONN_ERROR:hisi_conn_register_exception is faild!ret=%d\n", __func__, ret);
        goto exit_resource;
    }

    /* register ics dump and reset function */
    ret = hisi_conn_register_core();
    if (ret != 0) {
        ps_print_err("[%s]:HISI_CONN_ERROR:hisi_conn_register_core is failed!ret=%d\n", __func__, ret);
        goto exit_exception;
    }

    ret = hisi_conn_addr_map();
    if (ret != 0) {
        ps_print_err("[%s]:HISI_CONN_ERROR:hisi_conn_addr_map is failed!ret=%d\n", __func__, ret);
        goto exit_core;
    }

    ret = hisi_conn_stat_init();
    if (ret != 0) {
        ps_print_err("[%s]:HISI_CONN_ERROR:hisi_conn_stat_init is failed!ret=%d\n", __func__, ret);
        goto exit_core;
    }

    ps_print_info("hisi_conn_rdr_init all succ, buffer_len:0x%x,\n", g_hisi_conn_info.hisi_conn_ret_info.log_len);
    g_initdone = 1;
    return 0;

exit_core:
    rdr_unregister_module_ops((u64)RDR_CONN);
exit_exception:
    hisi_conn_remove_map();
exit_resource:
    destroy_workqueue(g_hisi_conn_rdr_wq);
    g_initdone = 0;
    return -EFAULT;
}
static int32 g_hisi_conn_initdone(void)
{
    if (!g_initdone) {
        if (hisi_conn_rdr_init()) {
            g_initdone = 0;
            return -1;
        }
    }
    return 0;
}

int hisi_conn_rdr_exit(void)
{
    if (g_initdone) {
        hisi_conn_remove_map();
        rdr_unregister_module_ops((u64)RDR_CONN);
        rdr_unregister_exception(RDR_CONN);
        destroy_workqueue(g_hisi_conn_rdr_wq);
        g_initdone = 0;
        ps_print_info("hisi_conn_rdr_init exit\n");
    } else {
        ps_print_info("hisi_conn_rdr_init not init no need exit\n");
    }

    return 0;
}
#endif
