

/* 1 头文件包含 */
#include "wal_config_base.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "wal_main.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_CONFIG_BASE_C


int32_t wal_recv_config_cmd(uint8_t *puc_buf, uint16_t us_len)
{
    int8_t ac_vap_name[OAL_IF_NAME_SIZE];
    oal_net_device_stru *pst_net_dev = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_stru *pst_event = NULL;
    uint32_t ret;
    wal_msg_stru *pst_msg = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    uint16_t us_netbuf_len; /* 传给sdt的skb数据区不包括头尾空间的长度 */
    wal_msg_stru *pst_rsp_msg = NULL;
    wal_msg_rep_hdr *pst_rep_hdr = NULL;

    uint16_t us_msg_size = us_len;
    uint16_t us_need_response = OAL_FALSE;
    int32_t l_ret;

    DECLARE_WAL_MSG_REQ_STRU(st_msg_request);

    WAL_MSG_REQ_STRU_INIT(st_msg_request);

    l_ret = memcpy_s(ac_vap_name, OAL_IF_NAME_SIZE, puc_buf, OAL_IF_NAME_SIZE);
    ac_vap_name[OAL_IF_NAME_SIZE - 1] = '\0'; /* 防止字符串异常 */

    /* 根据dev_name找到dev */
    pst_net_dev = wal_config_get_netdev(ac_vap_name, OAL_STRLEN(ac_vap_name));
    if (pst_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_recv_config_cmd::wal_config_get_netdev return null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_dev_put(pst_net_dev); /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */

    pst_mac_vap = oal_net_dev_priv(pst_net_dev); /* 获取mac vap */
    if (oal_unlikely(pst_mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_recv_config_cmd::OAL_NET_DEV_PRIV(pst_net_dev) is null ptr.}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (us_msg_size < OAL_IF_NAME_SIZE) { // 确保大于0为合法值，防止减下溢，导致后续申请内存以及拷贝发生堆溢出
        oam_error_log1(0, OAM_SF_ANY, "{wal_recv_config_cmd::msg_size[%d] overrun!}", us_msg_size);
        return OAL_FAIL;
    }

    us_msg_size -= OAL_IF_NAME_SIZE;
    /* 申请内存 */
    pst_event_mem = frw_event_alloc_m(us_msg_size + sizeof(wal_msg_rep_hdr));
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_recv_config_cmd::request %d mem failed}\r\n", us_msg_size);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_HOST_CRX,
                       WAL_HOST_CRX_SUBTYPE_CFG, (uint16_t)(us_msg_size),
                       FRW_EVENT_PIPELINE_STAGE_0, pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    /* 填写事件payload */
    l_ret += memcpy_s(frw_get_event_payload(pst_event_mem) + sizeof(wal_msg_rep_hdr), us_msg_size,
                      puc_buf + OAL_IF_NAME_SIZE, us_msg_size);
    pst_msg = (wal_msg_stru *)(puc_buf + OAL_IF_NAME_SIZE);
    pst_rep_hdr = (wal_msg_rep_hdr *)pst_event->auc_event_data;

    WAL_RECV_CMD_NEED_RESP(pst_msg, us_need_response);

    if (us_need_response == OAL_TRUE) {
        pst_rep_hdr->request_address = (uintptr_t)&st_msg_request;
        wal_msg_request_add_queue(&st_msg_request);
    } else {
        pst_rep_hdr->request_address = 0;
    }

    ret = wal_config_process_pkt(pst_event_mem);

    if (us_need_response == OAL_TRUE) {
        wal_msg_request_remove_queue(&st_msg_request);
    }

    if (ret != OAL_SUCC) {
        frw_event_free_m(pst_event_mem);
        WAL_MSG_REQ_RESP_MEM_FREE(st_msg_request);
        return (int32_t)ret;
    }

    /* 释放内存 */
    frw_event_free_m(pst_event_mem);

    /* 如果是查询消息类型，结果上报 */
    if (us_need_response == OAL_TRUE) {
        if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
            WAL_MSG_REQ_RESP_MEM_FREE(st_msg_request);
            return OAL_ERR_CODE_PTR_NULL;
        }

        if (st_msg_request.pst_resp_mem == NULL) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                             "{wal_recv_config_cmd::get response ptr failed!}\r\n");
            return (int32_t)ret;
        }

        pst_rsp_msg = (wal_msg_stru *)st_msg_request.pst_resp_mem;

        us_netbuf_len = pst_rsp_msg->st_msg_hdr.us_msg_len + 1; /* +1是sdt工具的需要 */

        us_netbuf_len = (us_netbuf_len > WLAN_SDT_NETBUF_MAX_PAYLOAD) ? WLAN_SDT_NETBUF_MAX_PAYLOAD : us_netbuf_len;

        pst_netbuf = oam_alloc_data2sdt(us_netbuf_len);
        if (pst_netbuf == NULL) {
            WAL_MSG_REQ_RESP_MEM_FREE(st_msg_request);
            return OAL_ERR_CODE_PTR_NULL;
        }

        oal_netbuf_data(pst_netbuf)[0] = 'M'; /* sdt需要 */
        l_ret += memcpy_s(oal_netbuf_data(pst_netbuf) + 1, us_netbuf_len - 1,
                          (uint8_t *)pst_rsp_msg->auc_msg_data, us_netbuf_len - 1);
        if (l_ret != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "wal_recv_config_cmd::memcpy fail!");
        }

        oal_free(st_msg_request.pst_resp_mem);

        oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_CFG, OAM_PRIMID_TYPE_DEV_ACK);
    }

    return OAL_SUCC;
}

int32_t wal_recv_memory_cmd(uint8_t *puc_buf, uint16_t us_len)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    wal_sdt_mem_frame_stru *pst_mem_frame;
    uintptr_t mem_addr;           /* 读取内存地址 */
    uint16_t us_mem_len;          /* 需要读取的长度 */
    uint8_t uc_offload_core_mode; /* offload下，表示哪一个核 */
    int32_t l_ret = EOK;

    pst_mem_frame = (wal_sdt_mem_frame_stru *)puc_buf;
    mem_addr = pst_mem_frame->addr;
    us_mem_len = pst_mem_frame->us_len;
    uc_offload_core_mode = pst_mem_frame->en_offload_core_mode;

    if (uc_offload_core_mode == WAL_OFFLOAD_CORE_MODE_DMAC) {
        /* 如果是offload情形，并且要读取的内存是wifi芯片侧，需要抛事件，后续开发 */
        return OAL_SUCC;
    }

    if (mem_addr == 0) { /* 读写地址不合理 */
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (us_mem_len > WAL_SDT_MEM_MAX_LEN) {
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    if (pst_mem_frame->uc_mode == MAC_SDT_MODE_READ) {
        if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
            return OAL_ERR_CODE_PTR_NULL;
        }

        pst_netbuf = oam_alloc_data2sdt(us_mem_len);
        if (pst_netbuf == NULL) {
            return OAL_ERR_CODE_PTR_NULL;
        }
        l_ret += memcpy_s(oal_netbuf_data(pst_netbuf), us_mem_len, (void *)mem_addr, us_mem_len);
        oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_MEM_RW, OAM_PRIMID_TYPE_DEV_ACK);
    } else if (pst_mem_frame->uc_mode == MAC_SDT_MODE_WRITE) {
        l_ret += memcpy_s((void *)mem_addr, us_mem_len, pst_mem_frame->auc_data, us_mem_len);
    }

    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_recv_memory_cmd::memcpy fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

int32_t wal_parse_global_var_cmd(wal_sdt_global_var_stru *pst_global_frame, unsigned long global_var_addr)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    uint16_t us_skb_len;
    int32_t l_ret = EOK;

    if (pst_global_frame->uc_mode == MAC_SDT_MODE_WRITE) {
        l_ret += memcpy_s((void *)(uintptr_t)global_var_addr, pst_global_frame->us_len,
                          (void *)(pst_global_frame->auc_global_value), pst_global_frame->us_len);
    } else if (pst_global_frame->uc_mode == MAC_SDT_MODE_READ) {
        if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
            return OAL_ERR_CODE_PTR_NULL;
        }

        us_skb_len = pst_global_frame->us_len;

        us_skb_len = (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD) ? WLAN_SDT_NETBUF_MAX_PAYLOAD : us_skb_len;

        pst_netbuf = oam_alloc_data2sdt(us_skb_len);
        if (pst_netbuf == NULL) {
            return OAL_ERR_CODE_PTR_NULL;
        }

        l_ret += memcpy_s(oal_netbuf_data(pst_netbuf), us_skb_len,
                          (void *)(uintptr_t)global_var_addr, us_skb_len);
        oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_MEM_RW, OAM_PRIMID_TYPE_DEV_ACK);
    }

    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_parse_global_var_cmd::memcpy fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

int32_t wal_recv_global_var_cmd(uint8_t *puc_buf, uint16_t us_len)
{
    wal_sdt_global_var_stru *pst_global_frame = NULL;
    unsigned long global_var_addr;

    if (puc_buf == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_global_frame = (wal_sdt_global_var_stru *)puc_buf;

    if (pst_global_frame->en_offload_core_mode == WAL_OFFLOAD_CORE_MODE_DMAC) {
        /* offload情形，并且要读取的全局变量在wifi芯片侧，需要抛事件，后续开发 */
        return OAL_SUCC;
    }

    global_var_addr = oal_kallsyms_lookup_name(pst_global_frame->auc_global_value_name);
    if (global_var_addr == 0) { /* not found */
        oam_warning_log0(0, OAM_SF_ANY,
                         "{wal_recv_global_var_cmd::kernel lookup global var address returns 0!}\r\n");
        return OAL_FAIL;
    }

    return wal_parse_global_var_cmd(pst_global_frame, global_var_addr);
}


int32_t wal_recv_reg_cmd(uint8_t *puc_buf, uint16_t us_len)
{
    int8_t ac_vap_name[OAL_IF_NAME_SIZE];
    oal_net_device_stru *pst_net_dev = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    wal_sdt_reg_frame_stru *pst_reg_frame = NULL;
    int32_t l_ret;
    hmac_vap_cfg_priv_stru *pst_cfg_priv = NULL;
    oal_netbuf_stru *pst_net_buf = NULL;
    uint32_t ret;
    int32_t l_memcpy_ret;

    l_memcpy_ret = memcpy_s(ac_vap_name, OAL_IF_NAME_SIZE, puc_buf, OAL_IF_NAME_SIZE);
    ac_vap_name[OAL_IF_NAME_SIZE - 1] = '\0'; /* 防止字符串异常 */

    /* 根据dev_name找到dev */
    pst_net_dev = wal_config_get_netdev(ac_vap_name, OAL_STRLEN(ac_vap_name));
    if (pst_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_recv_reg_cmd::wal_config_get_netdev return null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_dev_put(pst_net_dev); /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */

    pst_mac_vap = oal_net_dev_priv(pst_net_dev); /* 获取mac vap */

    ret = hmac_vap_get_priv_cfg(pst_mac_vap, &pst_cfg_priv); /* 取配置私有结构体 */
    if (ret != OAL_SUCC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_recv_reg_cmd::hmac_vap_get_priv_cfg return null_ptr_err!}\r\n");
        return (int32_t)ret;
    }

    pst_cfg_priv->en_wait_ack_for_sdt_reg = OAL_FALSE;

    ret = hmac_sdt_recv_reg_cmd(pst_mac_vap, puc_buf, us_len);
    if (ret != OAL_SUCC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_recv_reg_cmd::hmac_sdt_recv_reg_cmd return error code!}\r\n");

        return (int32_t)ret;
    }

    pst_reg_frame = (wal_sdt_reg_frame_stru *)puc_buf;

    if (pst_reg_frame->uc_mode == MAC_SDT_MODE_READ || pst_reg_frame->uc_mode == MAC_SDT_MODE_READ16) {
        wal_wake_lock();
        /*lint -e730*/ /* info, boolean argument to function */
#ifndef _PRE_WINDOWS_SUPPORT
        l_ret = oal_wait_event_interruptible_timeout_m(pst_cfg_priv->st_wait_queue_for_sdt_reg,
            OAL_TRUE == pst_cfg_priv->en_wait_ack_for_sdt_reg, (2 * OAL_TIME_HZ)); /* 2 超时时间 */
#else
        oal_wait_event_interruptible_timeout_m(&pst_cfg_priv->st_wait_queue_for_sdt_reg,
            OAL_TRUE == pst_cfg_priv->en_wait_ack_for_sdt_reg, (2 * OAL_TIME_HZ), l_ret); /* 2 超时时间 */
#endif
        /*lint +e730*/
        if (l_ret == 0) {
            /* 超时 */
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                             "{wal_recv_reg_cmd::wal_netdev_open: wait queue timeout!}\r\n");
            wal_wake_unlock();
            return -OAL_EINVAL;
        } else if (l_ret < 0) {
            /* 异常 */
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                             "{wal_recv_reg_cmd::wal_netdev_open: wait queue error!}\r\n");
            wal_wake_unlock();
            return -OAL_EINVAL;
        }
        wal_wake_unlock();
        /*lint +e774*/
        /* 读取返回的寄存器值 */
        pst_reg_frame->reg_val = *((uint32_t *)(pst_cfg_priv->ac_rsp_msg));

        if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func != NULL)) {
            pst_net_buf = oam_alloc_data2sdt((uint16_t)sizeof(wal_sdt_reg_frame_stru));
            if (pst_net_buf == NULL) {
                return OAL_ERR_CODE_PTR_NULL;
            }

            l_memcpy_ret += memcpy_s(oal_netbuf_data(pst_net_buf), (uint16_t)sizeof(wal_sdt_reg_frame_stru),
                                     (uint8_t *)pst_reg_frame, (uint16_t)sizeof(wal_sdt_reg_frame_stru));

            oam_report_data2sdt(pst_net_buf, OAM_DATA_TYPE_REG_RW, OAM_PRIMID_TYPE_DEV_ACK);
        }
    }

    if (l_memcpy_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_recv_reg_cmd::memcpy fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)

int32_t wal_recv_sample_cmd(uint8_t *puc_buf, uint16_t us_len)
{
    int8_t ac_vap_name[OAL_IF_NAME_SIZE];
    oal_net_device_stru *pst_net_dev;
    mac_vap_stru *pst_mac_vap;

    if (memcpy_s(ac_vap_name, OAL_IF_NAME_SIZE, puc_buf, OAL_IF_NAME_SIZE) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_recv_sample_cmd::memcpy fail!");
        return OAL_FAIL;
    }
    ac_vap_name[OAL_IF_NAME_SIZE - 1] = '\0'; /* 防止字符串异常 */

    /* 根据dev_name找到dev */
    pst_net_dev = wal_config_get_netdev(ac_vap_name, OAL_STRLEN(ac_vap_name));
    if (pst_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_recv_sample_cmd::wal_config_get_netdev return null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_dev_put(pst_net_dev); /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev); /* 获取mac vap */

    hmac_sdt_recv_sample_cmd(pst_mac_vap, puc_buf, us_len);
    return OAL_SUCC;
}


uint32_t wal_sample_report2sdt(frw_event_mem_stru *pst_event_mem)
{
    uint16_t us_payload_len;
    oal_netbuf_stru *pst_net_buf;
    frw_event_stru *pst_event;

    if (pst_event_mem == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_sample_report2sdt::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    us_payload_len = pst_event->st_event_hdr.us_length - FRW_EVENT_HDR_LEN;

    pst_net_buf = oam_alloc_data2sdt(us_payload_len);
    if (pst_net_buf == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (memcpy_s(oal_netbuf_data(pst_net_buf), us_payload_len, pst_event->auc_event_data, us_payload_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_sample_report2sdt::memcpy fail!");
        oal_mem_sdt_netbuf_free(pst_net_buf, OAL_TRUE);
        return OAL_FAIL;
    }

    oam_report_data2sdt(pst_net_buf, OAM_DATA_TYPE_SAMPLE, OAM_PRIMID_TYPE_DEV_ACK);
    return OAL_SUCC;
}
#endif

uint32_t wal_dpd_report2sdt(frw_event_mem_stru *pst_event_mem)
{
    uint16_t us_payload_len;
    oal_netbuf_stru *pst_net_buf = NULL;
    frw_event_stru *pst_event = NULL;

    if (pst_event_mem == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_sample_report2sdt::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    us_payload_len = pst_event->st_event_hdr.us_length - FRW_EVENT_HDR_LEN;

    pst_net_buf = oam_alloc_data2sdt(us_payload_len);
    if (pst_net_buf == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (memcpy_s(oal_netbuf_data(pst_net_buf), us_payload_len, pst_event->auc_event_data, us_payload_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_dpd_report2sdt::memcpy fail!");
        oal_mem_sdt_netbuf_free(pst_net_buf, OAL_TRUE);
        return OAL_FAIL;
    }

    oam_report_data2sdt(pst_net_buf, OAM_DATA_TYPE_DPD, OAM_PRIMID_TYPE_DEV_ACK);
    return OAL_SUCC;
}

void wal_drv_cfg_func_hook_init(void)
{
    g_st_wal_drv_func_hook.p_wal_recv_cfg_data_func = wal_recv_config_cmd;
    g_st_wal_drv_func_hook.p_wal_recv_mem_data_func = wal_recv_memory_cmd;
    g_st_wal_drv_func_hook.p_wal_recv_reg_data_func = wal_recv_reg_cmd;
    g_st_wal_drv_func_hook.p_wal_recv_global_var_func = wal_recv_global_var_cmd;
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
    g_st_wal_drv_func_hook.p_wal_recv_sample_data_func = wal_recv_sample_cmd;
#endif
}
