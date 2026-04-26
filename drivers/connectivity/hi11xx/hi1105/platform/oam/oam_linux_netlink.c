

/* 头文件包含 */
#include "oam_ext_if.h"
#include "oam_linux_netlink.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAM_LINUX_NETLINK_C

/* 全局变量定义 */
OAL_STATIC oam_netlink_stru g_st_netlink;
OAL_STATIC oam_netlink_proto_ops g_netlink_ops;

/* 数采上报app的结构体 */
typedef struct {
    uint32_t ul_daq_addr; /* 数采数据首地址 */
    uint32_t ul_data_len; /* 数采数据总的长度 */
    uint32_t ul_unit_len; /* 单元数据的最大长度:不包含(daq_unit_head)头长度 */
} oam_data_acq_info_stru;

/* 数采单元头结构体 */
typedef struct {
    uint8_t en_send_type; /* 数采单元数据序列号 */
    uint8_t uc_resv[3]; /* 定义oal_uint8类型的一维数组，元素个数为3 */
    uint32_t ul_msg_sn;   /* 数采单元数据序列号 */
    uint32_t ul_data_len; /* 当前单元长度 */
} oam_data_acq_data_head_stru;

/*
 * 函 数 名  : oam_netlink_ops_register
 * 功能描述  : WAL模块向其它模块提供的注册netlink消息处理函数(接收方向)
 */
void oam_netlink_ops_register(oam_nl_cmd_enum_uint8 en_type,
                              uint32_t (*p_func)(uint8_t *puc_data, uint32_t ul_len))
{
    if (oal_unlikely(p_func == NULL)) {
        oal_io_print("oam_netlink_ops_register, p_func is null ptr.");
        return;
    }

    switch (en_type) {
        case OAM_NL_CMD_SDT:
            g_netlink_ops.p_oam_sdt_func = p_func;
            break;

        case OAM_NL_CMD_HUT:
            g_netlink_ops.p_oam_hut_func = p_func;
            break;

        case OAM_NL_CMD_ALG:
            g_netlink_ops.p_oam_alg_func = p_func;
            break;

        case OAM_NL_CMD_DAQ:
            g_netlink_ops.p_oam_daq_func = p_func;
            break;

        case OAM_NL_CMD_REG:
            g_netlink_ops.p_oam_reg_func = p_func;
            break;

        case OAM_NL_CMD_ACS:
            g_netlink_ops.p_oam_acs_func = p_func;
            break;

        case OAM_NL_CMD_PSTA:
            g_netlink_ops.p_oam_psta_func = p_func;
            break;

        default:
            oal_io_print("oam_netlink_ops_register, err type = %d.", en_type);
            break;
    }
}

/*
 * 函 数 名  : oam_netlink_ops_unregister
 * 功能描述  : OAM模块向其它模块提供的卸载netlink消息处理函数(接收方向)
 */
void oam_netlink_ops_unregister(oam_nl_cmd_enum_uint8 en_type)
{
    switch (en_type) {
        case OAM_NL_CMD_SDT:
            g_netlink_ops.p_oam_sdt_func = NULL;
            break;

        case OAM_NL_CMD_HUT:
            g_netlink_ops.p_oam_hut_func = NULL;
            break;

        case OAM_NL_CMD_ALG:
            g_netlink_ops.p_oam_alg_func = NULL;
            break;

        case OAM_NL_CMD_DAQ:
            g_netlink_ops.p_oam_daq_func = NULL;
            break;

        case OAM_NL_CMD_REG:
            g_netlink_ops.p_oam_reg_func = NULL;
            break;

        case OAM_NL_CMD_ACS:
            g_netlink_ops.p_oam_acs_func = NULL;
            break;

        case OAM_NL_CMD_PSTA:
            g_netlink_ops.p_oam_psta_func = NULL;
            break;

        default:
            oal_io_print("oam_netlink_ops_unregister::err type = %d.", en_type);
            break;
    }
}

/*
 * 函 数 名  : oam_netlink_kernel_recv
 * 功能描述  : netlink消息接收函数(方向: host app -> 内核)
 */
OAL_STATIC void oam_netlink_kernel_recv(oal_netbuf_stru *pst_buf)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    oal_nlmsghdr_stru *pst_nlmsghdr = NULL;

    if (pst_buf == NULL) {
        oal_io_print("oam_netlink_kernel_recv, pst_buf is null.");
        return;
    }

    pst_netbuf = pst_buf;

    while (oal_netbuf_len(pst_netbuf) >= oal_nlmsg_space(0)) {
        pst_nlmsghdr = oal_nlmsg_hdr(pst_netbuf);

        g_st_netlink.ul_pid = pst_nlmsghdr->nlmsg_pid;

        switch (pst_nlmsghdr->nlmsg_type) {
            case OAM_NL_CMD_SDT:
                if (g_netlink_ops.p_oam_sdt_func != NULL) {
                    g_netlink_ops.p_oam_sdt_func(oal_nlmsg_data(pst_nlmsghdr),
                                                 oal_nlmsg_payload(pst_nlmsghdr, 0));
                }
                break;

            case OAM_NL_CMD_HUT:
                if (g_netlink_ops.p_oam_hut_func != NULL) {
                    g_netlink_ops.p_oam_hut_func(oal_nlmsg_data(pst_nlmsghdr),
                                                 oal_nlmsg_payload(pst_nlmsghdr, 0));
                }
                break;

            case OAM_NL_CMD_ALG:
                if (g_netlink_ops.p_oam_alg_func != NULL) {
                    g_netlink_ops.p_oam_alg_func(oal_nlmsg_data(pst_nlmsghdr),
                                                 oal_nlmsg_payload(pst_nlmsghdr, 0));
                }
                break;
            case OAM_NL_CMD_DAQ:
                if (g_netlink_ops.p_oam_daq_func != NULL) {
                    g_netlink_ops.p_oam_daq_func(oal_nlmsg_data(pst_nlmsghdr),
                                                 oal_nlmsg_payload(pst_nlmsghdr, 0));
                }
                break;
            case OAM_NL_CMD_REG:
                if (g_netlink_ops.p_oam_reg_func != NULL) {
                    g_netlink_ops.p_oam_reg_func(oal_nlmsg_data(pst_nlmsghdr),
                                                 oal_nlmsg_payload(pst_nlmsghdr, 0));
                }
                break;
            case OAM_NL_CMD_ACS:
                if (g_netlink_ops.p_oam_acs_func != NULL) {
                    g_netlink_ops.p_oam_acs_func(oal_nlmsg_data(pst_nlmsghdr),
                                                 oal_nlmsg_payload(pst_nlmsghdr, 0));
                }
                break;
            case OAM_NL_CMD_PSTA:
                if (g_netlink_ops.p_oam_psta_func != NULL) {
                    g_netlink_ops.p_oam_psta_func(oal_nlmsg_data(pst_nlmsghdr),
                                                  oal_nlmsg_payload(pst_nlmsghdr, 0));
                }
                break;
            default:
                break;
        }

        oal_netbuf_pull(pst_netbuf, oal_nlmsg_align(pst_nlmsghdr->nlmsg_len));
    }
}

/*
 * 函 数 名  : oam_netlink_kernel_send
 * 功能描述  : netlink消息发送函数(方向: 内核 -> host app)
 * 输入参数  : puc_data   : 输入数据
 *             ul_data_len: 数据长度
 *             en_type    : netlink msg类型
 * 输出参数  : 成功: 发送的字节数(netlink头 + payload + padding)
 */
int32_t oam_netlink_kernel_send(uint8_t *puc_data, uint32_t ul_data_len, oam_nl_cmd_enum_uint8 en_type)
{
#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
    return 0;
#else
#if (_PRE_TARGET_PRODUCT_TYPE_1102COMMON == _PRE_CONFIG_TARGET_PRODUCT)
    return 0;
#else
    oal_netbuf_stru *pst_netbuf;
    oal_nlmsghdr_stru *pst_nlmsghdr;
    uint32_t ul_size;
    int32_t l_ret;

    if (oal_unlikely(puc_data == NULL)) {
        oal_warn_on(1);
        return -1;
    }

    // 若APP未注册，该值为0，会回发到驱动
    if (!g_st_netlink.ul_pid) {
        return -1;
    }

    ul_size = oal_nlmsg_space(ul_data_len);
    pst_netbuf = oal_netbuf_alloc(ul_size, 0, WLAN_MEM_NETBUF_ALIGN);
    if (pst_netbuf == NULL) {
        return -1;
    }

    /* 初始化netlink消息首部 */
    pst_nlmsghdr = oal_nlmsg_put(pst_netbuf, 0, 0, (int32_t)en_type, (int32_t)ul_data_len, 0);

    /* 设置控制字段 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    oal_netlink_cb(pst_netbuf).portid = 0;
#else
    oal_netlink_cb(pst_netbuf).pid = 0;
#endif
    oal_netlink_cb(pst_netbuf).dst_group = 0;

    /* 填充数据区 */
    memcpy_s(oal_nlmsg_data(pst_nlmsghdr), ul_data_len, puc_data, ul_data_len);

    /* 发送数据 */
    l_ret = oal_netlink_unicast(g_st_netlink.pst_nlsk, pst_netbuf, g_st_netlink.ul_pid, OAL_MSG_DONTWAIT);

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
 * 输出参数  : 成功: 发送的字节数(netlink头 + payload + padding)
 */
int32_t oam_netlink_kernel_send_ex(uint8_t *puc_data_1st, uint8_t *puc_data_2nd,
                                   uint32_t ul_len_1st, uint32_t ul_len_2nd,
                                   oam_nl_cmd_enum_uint8 en_type)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    return 0;
#else

    oal_netbuf_stru *pst_netbuf = NULL;
    oal_nlmsghdr_stru *pst_nlmsghdr = NULL;
    uint32_t ul_size;
    int32_t l_ret;

    if (oal_unlikely((puc_data_1st == NULL) || (puc_data_2nd == NULL))) {
        oal_warn_on(1);
        return -1;
    }

    ul_size = oal_nlmsg_space(ul_len_1st + ul_len_2nd);
    pst_netbuf = oal_netbuf_alloc(ul_size, 0, WLAN_MEM_NETBUF_ALIGN);
    if (pst_netbuf == NULL) {
        return -1;
    }

    /* 初始化netlink消息首部 */
    pst_nlmsghdr = oal_nlmsg_put(pst_netbuf, 0, 0, (int32_t)en_type, (int32_t)(ul_len_1st + ul_len_2nd), 0);

    /* 设置控制字段 */
    oal_netlink_cb(pst_netbuf).pid = 0;
    oal_netlink_cb(pst_netbuf).dst_group = 0;

    /* 填充数据区 */
    if (memcpy_s(oal_nlmsg_data(pst_nlmsghdr), (uint32_t)(ul_len_1st + ul_len_2nd),
                 puc_data_1st, ul_len_1st) != EOK) {
        oal_netbuf_free(pst_netbuf);
        oal_io_print ("memcpy_s error, destlen=%u, srclen=%u\n ", (uint32_t)(ul_len_1st + ul_len_2nd), ul_len_1st);
        return -OAL_EFAIL;
    }

    memcpy_s((uint8_t *)oal_nlmsg_data(pst_nlmsghdr) + ul_len_1st, ul_len_2nd, puc_data_2nd, ul_len_2nd);

    /* 发送数据 */
    l_ret = oal_netlink_unicast(g_st_netlink.pst_nlsk, pst_netbuf, g_st_netlink.ul_pid, OAL_MSG_DONTWAIT);

    return l_ret;
#endif
}

uint32_t oam_netlink_kernel_create(void)
{
    g_st_netlink.pst_nlsk = oal_netlink_kernel_create(&OAL_INIT_NET, OAM_NETLINK_ID, 0,
                                                      oam_netlink_kernel_recv, NULL,
                                                      OAL_THIS_MODULE);
    if (g_st_netlink.pst_nlsk == NULL) {
        oal_io_print("oam_netlink_kernel_create, can not create netlink.");

        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_io_print("netlink create succ.");

    return OAL_SUCC;
}

void oam_netlink_kernel_release(void)
{
    oal_netlink_kernel_release(g_st_netlink.pst_nlsk);

    g_st_netlink.ul_pid = 0;

    oal_io_print("netlink release succ.");
}

/*lint -e578*/ /*lint -e19*/
oal_module_symbol(oam_netlink_ops_register);
oal_module_symbol(oam_netlink_ops_unregister);
oal_module_symbol(oam_netlink_kernel_send);
oal_module_symbol(oam_netlink_kernel_send_ex);
