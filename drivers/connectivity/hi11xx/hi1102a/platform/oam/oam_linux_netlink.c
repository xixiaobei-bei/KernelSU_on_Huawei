

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_linux_netlink.h"
#include "oam_ext_if.h"

#include "securec.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAM_LINUX_NETLINK_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
oam_netlink_stru netlink;

oam_netlink_proto_ops netlink_ops;

/* 数采上报app的结构体 */
typedef struct {
    oal_uint32 ul_daq_addr; /* 数采数据首地址 */
    oal_uint32 ul_data_len; /* 数采数据总的长度 */
    oal_uint32 ul_unit_len; /* 单元数据的最大长度:不包含(daq_unit_head)头长度 */
} oam_data_acq_info_stru;

/* 数采单元头结构体 */
typedef struct {
    oal_uint8 en_send_type; /* 数采单元数据序列号 */
    oal_uint8 uc_resv[3];
    oal_uint32 ul_msg_sn;   /* 数采单元数据序列号 */
    oal_uint32 ul_data_len; /* 当前单元长度 */
} oam_data_acq_data_head_stru;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*
 * 函 数 名  : oam_netlink_ops_register
 * 功能描述  : WAL模块向其它模块提供的注册netlink消息处理函数(接收方向)
 */
oal_void oam_netlink_ops_register(oam_nl_cmd_enum_uint8 en_type,
    oal_uint32 (*p_func)(oal_uint8 *puc_data, oal_uint32 ul_len))
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == p_func)) {
        OAL_IO_PRINT("oam_netlink_ops_register, p_func is null ptr.");
        return;
    }

    switch (en_type) {
        case OAM_NL_CMD_SDT:
            netlink_ops.p_oam_sdt_func = p_func;
            break;

        case OAM_NL_CMD_HUT:
            netlink_ops.p_oam_hut_func = p_func;
            break;

        case OAM_NL_CMD_ALG:
            netlink_ops.p_oam_alg_func = p_func;
            break;

        case OAM_NL_CMD_DAQ:
            netlink_ops.p_oam_daq_func = p_func;
            break;

        case OAM_NL_CMD_REG:
            netlink_ops.p_oam_reg_func = p_func;
            break;

        case OAM_NL_CMD_ACS:
            netlink_ops.p_oam_acs_func = p_func;
            break;

        case OAM_NL_CMD_PSTA:
            netlink_ops.p_oam_psta_func = p_func;
            break;

        default:
            OAL_IO_PRINT("oam_netlink_ops_register, err type = %d.", en_type);
            break;
    }
}

/*
 * 函 数 名  : oam_netlink_ops_unregister
 * 功能描述  : OAM模块向其它模块提供的卸载netlink消息处理函数(接收方向)
 */
oal_void oam_netlink_ops_unregister(oam_nl_cmd_enum_uint8 en_type)
{
    switch (en_type) {
        case OAM_NL_CMD_SDT:
            netlink_ops.p_oam_sdt_func = OAL_PTR_NULL;
            break;

        case OAM_NL_CMD_HUT:
            netlink_ops.p_oam_hut_func = OAL_PTR_NULL;
            break;

        case OAM_NL_CMD_ALG:
            netlink_ops.p_oam_alg_func = OAL_PTR_NULL;
            break;

        case OAM_NL_CMD_DAQ:
            netlink_ops.p_oam_daq_func = OAL_PTR_NULL;
            break;

        case OAM_NL_CMD_REG:
            netlink_ops.p_oam_reg_func = OAL_PTR_NULL;
            break;

        case OAM_NL_CMD_ACS:
            netlink_ops.p_oam_acs_func = OAL_PTR_NULL;
            break;

        case OAM_NL_CMD_PSTA:
            netlink_ops.p_oam_psta_func = OAL_PTR_NULL;
            break;

        default:
            OAL_IO_PRINT("oam_netlink_ops_unregister::err type = %d.", en_type);
            break;
    }
}

/*
 * 函 数 名  : oam_netlink_kernel_recv
 * 功能描述  : netlink消息接收函数(方向: host app -> 内核)
 */
oal_void oam_netlink_kernel_recv(oal_netbuf_stru *pst_buf)
{
    oal_netbuf_stru *pst_netbuf;
    oal_nlmsghdr_stru *pst_nlmsghdr;

    if (pst_buf == OAL_PTR_NULL) {
        OAL_IO_PRINT("oam_netlink_kernel_recv, pst_buf is null.");
        return;
    }

    pst_netbuf = pst_buf;

    while (OAL_NETBUF_LEN(pst_netbuf) >= OAL_NLMSG_SPACE(0)) {
        pst_nlmsghdr = oal_nlmsg_hdr(pst_netbuf);

        netlink.ul_pid = pst_nlmsghdr->nlmsg_pid;

        switch (pst_nlmsghdr->nlmsg_type) {
            case OAM_NL_CMD_SDT:
                if (netlink_ops.p_oam_sdt_func != OAL_PTR_NULL) {
                    netlink_ops.p_oam_sdt_func(OAL_NLMSG_DATA(pst_nlmsghdr), OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;

            case OAM_NL_CMD_HUT:
                if (netlink_ops.p_oam_hut_func != OAL_PTR_NULL) {
                    netlink_ops.p_oam_hut_func(OAL_NLMSG_DATA(pst_nlmsghdr), OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;

            case OAM_NL_CMD_ALG:
                if (netlink_ops.p_oam_alg_func != OAL_PTR_NULL) {
                    netlink_ops.p_oam_alg_func(OAL_NLMSG_DATA(pst_nlmsghdr), OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;
            case OAM_NL_CMD_DAQ:
                if (netlink_ops.p_oam_daq_func != OAL_PTR_NULL) {
                    netlink_ops.p_oam_daq_func(OAL_NLMSG_DATA(pst_nlmsghdr), OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;
            case OAM_NL_CMD_REG:
                if (netlink_ops.p_oam_reg_func != OAL_PTR_NULL) {
                    netlink_ops.p_oam_reg_func(OAL_NLMSG_DATA(pst_nlmsghdr), OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;
            case OAM_NL_CMD_ACS:
                if (netlink_ops.p_oam_acs_func != OAL_PTR_NULL) {
                    netlink_ops.p_oam_acs_func(OAL_NLMSG_DATA(pst_nlmsghdr), OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;
            case OAM_NL_CMD_PSTA:
                if (netlink_ops.p_oam_psta_func != OAL_PTR_NULL) {
                    netlink_ops.p_oam_psta_func(OAL_NLMSG_DATA(pst_nlmsghdr), OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;
            default:
                break;
        }

        oal_netbuf_pull(pst_netbuf, OAL_NLMSG_ALIGN(pst_nlmsghdr->nlmsg_len));
    }
}

/*
 * 函 数 名  : oam_netlink_kernel_send
 * 功能描述  : netlink消息发送函数(方向: 内核 -> host app)
 * 输入参数  : puc_data   : 输入数据
 *             ul_data_len: 数据长度
 *             en_type    : netlink msg类型
 * 输出参数  : 无
 * 返 回 值  : 成功: 发送的字节数(netlink头 + payload + padding)
 */
oal_int32 oam_netlink_kernel_send(oal_uint8 *puc_data, oal_uint32 ul_data_len, oam_nl_cmd_enum_uint8 en_type)
{
#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)

    return 0;
#else

#if (_PRE_TARGET_PRODUCT_TYPE_1102COMMON == _PRE_CONFIG_TARGET_PRODUCT)
    return 0;
#else
    oal_netbuf_stru *pst_netbuf;
    oal_nlmsghdr_stru *pst_nlmsghdr;
    oal_uint32 ul_size;
    oal_int32 l_ret;

    if (OAL_UNLIKELY(puc_data == NULL)) {
        OAL_WARN_ON(1);
        return -1;
    }

    // 若APP未注册，该值为0，会回发到驱动
    if (!netlink.ul_pid) {
        return -1;
    }

    ul_size = OAL_NLMSG_SPACE(ul_data_len);
    pst_netbuf = oal_netbuf_alloc(ul_size, 0, WLAN_MEM_NETBUF_ALIGN);
    if (pst_netbuf == OAL_PTR_NULL) {
        return -1;
    }

    /* 初始化netlink消息首部 */
    pst_nlmsghdr = oal_nlmsg_put(pst_netbuf, 0, 0, (oal_int32)en_type, (oal_int32)ul_data_len, 0);

    /* 设置控制字段 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    OAL_NETLINK_CB(pst_netbuf).portid = 0;
#else
    OAL_NETLINK_CB(pst_netbuf).pid = 0;
#endif
    OAL_NETLINK_CB(pst_netbuf).dst_group = 0;

    /* 填充数据区 */
    l_ret = memcpy_s(OAL_NLMSG_DATA(pst_nlmsghdr), ul_data_len, puc_data, ul_data_len);
    if (l_ret != EOK) {
        oal_netbuf_free(pst_netbuf);
        OAL_IO_PRINT("oam_netlink_kernel_send: memcpy_s failed, ret = %d", l_ret);
        return -1;
    }

    /* 发送数据 */
    l_ret = oal_netlink_unicast(netlink.pst_nlsk, pst_netbuf, netlink.ul_pid, OAL_MSG_DONTWAIT);

    return l_ret;

#endif /* _PRE_TARGET_PRODUCT_TYPE_1102COMMON == _PRE_CONFIG_TARGET_PRODUCT */
#endif /* _PRE_OS_VERSION_RAW == _PRE_OS_VERSION  */
}

/*
 * 函 数 名  : oam_netlink_kernel_send_ex
 * 功能描述  : netlink消息发送函数(方向: 内核 -> host app)
 * 输入参数  : puc_data_1st: 输入数据1
 *             puc_data_2nd: 输入数据2
 *             ul_len_1st  : 数据长度1
 *             ul_len_2nd  : 数据长度2
 *             en_type     : netlink msg类型
 * 输出参数  : 无
 * 返 回 值  : 成功: 发送的字节数(netlink头 + payload + padding)
 */
oal_int32 oam_netlink_kernel_send_ex(oal_uint8 *puc_data_1st, oal_uint8 *puc_data_2nd,
                                     oal_uint32 ul_len_1st, oal_uint32 ul_len_2nd,
                                     oam_nl_cmd_enum_uint8 en_type)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    return 0;
#else

    oal_netbuf_stru *pst_netbuf;
    oal_nlmsghdr_stru *pst_nlmsghdr;
    oal_uint32 ul_size;
    oal_int32 l_ret;

    if (OAL_UNLIKELY((puc_data_1st == NULL) || (puc_data_2nd == NULL))) {
        OAL_WARN_ON(1);
        return -1;
    }

    ul_size = OAL_NLMSG_SPACE(ul_len_1st + ul_len_2nd);
    pst_netbuf = oal_netbuf_alloc(ul_size, 0, WLAN_MEM_NETBUF_ALIGN);
    if (pst_netbuf == OAL_PTR_NULL) {
        return -1;
    }

    /* 初始化netlink消息首部 */
    pst_nlmsghdr = oal_nlmsg_put(pst_netbuf, 0, 0, (oal_int32)en_type, (oal_int32)(ul_len_1st + ul_len_2nd), 0);

    /* 设置控制字段 */
    OAL_NETLINK_CB(pst_netbuf).pid = 0;
    OAL_NETLINK_CB(pst_netbuf).dst_group = 0;

    /* 填充数据区 */
    if (memcpy_s(OAL_NLMSG_DATA(pst_nlmsghdr), (oal_uint32)(ul_len_1st + ul_len_2nd),
                 puc_data_1st, ul_len_1st) != EOK) {
        oal_netbuf_free(pst_netbuf);
        OAL_IO_PRINT ("memcpy_s error, destlen=%u, srclen=%u\n ", (oal_uint32)(ul_len_1st + ul_len_2nd), ul_len_1st);
        return -OAL_EFAIL;
    }

    memcpy_s((oal_uint8 *)OAL_NLMSG_DATA(pst_nlmsghdr) + ul_len_1st, ul_len_2nd, puc_data_2nd, ul_len_2nd);

    /* 发送数据 */
    l_ret = oal_netlink_unicast(netlink.pst_nlsk, pst_netbuf, netlink.ul_pid, OAL_MSG_DONTWAIT);

    return l_ret;
#endif
}

oal_uint32 oam_netlink_kernel_create(oal_void)
{
    netlink.pst_nlsk = oal_netlink_kernel_create(&OAL_INIT_NET, OAM_NETLINK_ID, 0,
                                                 oam_netlink_kernel_recv, OAL_PTR_NULL,
                                                 OAL_THIS_MODULE);
    if (netlink.pst_nlsk == OAL_PTR_NULL) {
        OAL_IO_PRINT("oam_netlink_kernel_create, can not create netlink.");

        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_IO_PRINT("netlink create succ.");

    return OAL_SUCC;
}

oal_void oam_netlink_kernel_release(oal_void)
{
    oal_netlink_kernel_release(netlink.pst_nlsk);

    netlink.ul_pid = 0;

    OAL_IO_PRINT("netlink release succ.");
}

/*lint -e578*/ /*lint -e19*/
oal_module_symbol(netlink_ops);
oal_module_symbol(oam_netlink_ops_register);
oal_module_symbol(oam_netlink_ops_unregister);
oal_module_symbol(oam_netlink_kernel_send);
oal_module_symbol(oam_netlink_kernel_send_ex);




