

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "plat_firmware.h"

#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <linux/rtc.h>

#include "plat_debug.h"
#include "plat_sdio.h"
#include "plat_uart.h"
#include "plat_cali.h"
#include "platform_common_clk.h"
#include "plat_pm.h"
#include "oal_sdio_host_if.h"
#include "oal_hcc_host_if.h"
#include "oam_ext_if.h"

#include "hw_bfg_ps.h"
#include "plat_efuse.h"
#include "bfgx_exception_rst.h"
#include "board.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLAT_FIRMWARE_C

#include "oal_util.h"
#include "securec.h"

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
/* hi1102 cfg文件路径 */
uint8 *cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_PATH,
    WIFI_CFG_PATH,
    BFGX_CFG_PATH,
    RAM_CHECK_CFG_PATH,
};

/* hi1103 mpw2 cfg文件路径 */
uint8 *mpw2_cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_HI1103_MPW2_PATH,
    WIFI_CFG_HI1103_MPW2_PATH,
    BFGX_CFG_HI1103_MPW2_PATH,
    RAM_CHECK_CFG_HI1103_MPW2_PATH,
};

/* hi1103 pilot cfg文件路径 */
uint8 *pilot_cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_HI1103_PILOT_PATH,
    WIFI_CFG_HI1103_PILOT_PATH,
    BFGX_CFG_HI1103_PILOT_PATH,
    RAM_CHECK_CFG_HI1103_PILOT_PATH,
};

/* hi1102a cfg文件 */
uint8 *auc_1102a_cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_HI1102A_PILOT_PATH,
    WIFI_CFG_HI1102A_PILOT_PATH,
    BFGX_CFG_HI1102A_PILOT_PATH,
    RAM_CHECK_CFG_HI1102A_PILOT_PATH,
    RAM_BCPU_CHECK_CFG_HI1102A_PILOT_PATH,
    BFGX_AND_HITALK_CFG_HI1102A_HITALK_PATH,
    HITALK_CFG_HI1102A_HITALK_PATH,
};

uint32 asic_type = HI1103_ASIC_MPW2;

uint8 **cfg_path = cfg_patch_in_vendor;

/* 存储cfg文件信息，解析cfg文件时赋值，加载的时候使用该变量 */
FIRMWARE_GLOBALS_STRUCT cfg_info;

/* 保存firmware file内容的buffer，先将文件读到这个buffer中，然后从这个向device buffer发送 */
static uint8 *firmware_down_buf = NULL;

/* g_pucDataBuf的长度 */
static uint32 firmware_down_buf_len = 0;

struct st_wifi_dump_mem_info nfc_buffer_data = { 0x30000000 + 0x000f9d00, OMLNFCDATABUFFLEN, "nfc_buffer_data" };
uint8 *pucNfcLog = NULL;
/*****************************************************************************
  4 函数实现
*****************************************************************************/

/*
 * 函 数 名  : set_hi1103_asic_type
 * 功能描述  : 设置hi1103 asic类型(MPW2/PILOT)
 * 输入参数  : ul_asic_type: 0-MPW2，1-PILOT
 */
void set_hi1103_asic_type(uint32 ul_asic_type)
{
    asic_type = ul_asic_type;
}

/*
 * 函 数 名  : get_hi1103_asic_type
 * 功能描述  : 获取hi1103 asic类型(MPW2/PILOT)
 * 返 回 值  : 0-MPW2，1-PILOT
 */
uint32 get_hi1103_asic_type(void)
{
    return asic_type;
}

/*
 * 函 数 名  : read_msg
 * 功能描述  : host接收device发来的消息
 * 输入参数  : data: 接收消息的buffer
 *             len : 接收buffer的长度
 * 返 回 值  : -1表示失败，否则返回实际接收的长度
 */
int32 read_msg(uint8 *data, int32 len)
{
    int32 l_len;
    hcc_bus *pst_bus = hcc_get_current_110x_bus();

    if (unlikely((data == NULL))) {
        PS_PRINT_ERR("data is NULL\n ");
        return -EFAIL;
    }

    if (unlikely((pst_bus == NULL))) {
        PS_PRINT_ERR("pst_bus is NULL\n ");
        return -EFAIL;
    }

    l_len = hcc_bus_patch_read(pst_bus, data, len, READ_MEG_TIMEOUT);
    PS_PRINT_DBG("Receive l_len=[%d]\n", l_len);

    return l_len;
}

int32 read_msg_timeout(uint8 *data, int32 len, uint32 timeout)
{
    int32 l_len;
    hcc_bus *pst_bus = hcc_get_current_110x_bus();

    if (unlikely((data == NULL))) {
        PS_PRINT_ERR("data is NULL\n ");
        return -EFAIL;
    }

    if (unlikely((pst_bus == NULL))) {
        PS_PRINT_ERR("pst_bus is NULL\n ");
        return -EFAIL;
    }

    l_len = hcc_bus_patch_read(pst_bus, data, len, timeout);
    PS_PRINT_DBG("Receive l_len=[%d], data = [%s]\n", l_len, data);

    return l_len;
}

/*
 * 函 数 名  : send_msg
 * 功能描述  : host往device发送消息
 * 输入参数  : data: 发送buffer
 *             len : 发送数据的长度
 * 返 回 值  : -1表示失败，否则返回实际发送的长度
 */
int32 send_msg(uint8 *data, int32 len)
{
    int32 l_ret;
    hcc_bus *pst_bus = hcc_get_current_110x_bus();

    if (unlikely((pst_bus == NULL))) {
        PS_PRINT_ERR("pst_bus is NULL\n ");
        return -EFAIL;
    }

    PS_PRINT_DBG("len = %d\n", len);
#ifdef HW_DEBUG
    const uint32 ul_max_print_len = 128;
    print_hex_dump_bytes("send_msg :", DUMP_PREFIX_ADDRESS, data,
                         (len < ul_max_print_len ? len : ul_max_print_len));
#endif
    l_ret = hcc_bus_patch_write(pst_bus, data, len);

    return l_ret;
}

/*
 * 函 数 名  : recv_expect_result
 * 功能描述  : 接收host期望device正确返回的内容
 * 输入参数  : expect: 期望device正确返回的内容
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32 recv_expect_result(const uint8 *expect)
{
    uint8 auc_buf[RECV_BUF_LEN];
    int32 l_len;
    int32 i;

    if (!OS_STR_LEN(expect)) {
        PS_PRINT_DBG("not wait device to respond!\n");
        return SUCC;
    }

    memset_s(auc_buf, RECV_BUF_LEN, 0, RECV_BUF_LEN);
    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_len = read_msg(auc_buf, RECV_BUF_LEN);
        if (l_len < 0) {
            PS_PRINT_ERR("recv result fail\n");
            continue;
        }

        if (!OS_MEM_CMP(auc_buf, expect, OS_STR_LEN(expect))) {
            PS_PRINT_DBG(" send SUCC, expect [%s] ok\n", expect);
            return SUCC;
        } else {
            PS_PRINT_WARNING(" error result[%s], expect [%s], read result again\n", auc_buf, expect);
        }
    }

    return -EFAIL;
}

int32 recv_expect_result_timeout(const uint8 *expect, uint32 timeout)
{
    uint8 auc_buf[RECV_BUF_LEN];
    int32 l_len;

    if (!OS_STR_LEN(expect)) {
        PS_PRINT_DBG("not wait device to respond!\n");
        return SUCC;
    }

    memset_s(auc_buf, RECV_BUF_LEN, 0, RECV_BUF_LEN);
    l_len = read_msg_timeout(auc_buf, RECV_BUF_LEN, timeout);
    if (l_len < 0) {
        PS_PRINT_ERR("recv result fail\n");
        return -EFAIL;
    }

    if (!OS_MEM_CMP(auc_buf, expect, OS_STR_LEN(expect))) {
        PS_PRINT_DBG(" send SUCC, expect [%s] ok\n", expect);
        return SUCC;
    } else {
        PS_PRINT_WARNING(" error result[%s], expect [%s], read result again\n", auc_buf, expect);
    }

    return -EFAIL;
}

/*
 * 函 数 名  : msg_send_and_recv_except
 * 功能描述  : host向device发送消息并等待device返回消息
 * 输入参数  : data  : 发送buffer
 *             len   : 发送内容的长度
 *             expect: 期望device回复的内容
 * 返 回 值  : -1表示失败，0表示成功
 */
int32 msg_send_and_recv_except(uint8 *data, int32 len, const uint8 *expect)
{
    int32 i;
    int32 l_ret;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_ret = send_msg(data, len);
        if (l_ret < 0) {
            continue;
        }

        l_ret = recv_expect_result(expect);
        if (l_ret == 0) {
            return SUCC;
        }
    }

    return -EFAIL;
}

/*
 * 函 数 名  : malloc_cmd_buf
 * 功能描述  : 解析cfg文件，将解析的结果保存在g_st_cfg_info全局变量中
 * 输入参数  : puc_cfg_info_buf: 保存有cfg文件内容的buffer
 *             ul_index        : 保存解析结果的数组索引值
 * 返 回 值  : NULL表示分配内存失败，否则返回指向保存解析cfg文件命令数组的首地址
 */
void *malloc_cmd_buf(uint8 *puc_cfg_info_buf, uint32 ul_index)
{
    int32 l_len;
    uint8 *flag;
    uint8 *p_buf;

    if (puc_cfg_info_buf == NULL) {
        PS_PRINT_ERR("malloc_cmd_buf: buf is NULL!\n");
        return NULL;
    }

    /* 统计命令个数 */
    flag = puc_cfg_info_buf;
    cfg_info.al_count[ul_index] = 0;
    while (flag != NULL) {
        /* 一个正确的命令行结束符为 ; */
        flag = OS_STR_CHR(flag, CMD_LINE_SIGN);
        if (flag == NULL) {
            break;
        }
        cfg_info.al_count[ul_index]++;
        flag++;
    }
    PS_PRINT_DBG("cfg file cmd count: al_count[%d] = %d\n", ul_index, cfg_info.al_count[ul_index]);

    /* 申请存储命令空间 */
    l_len = ((cfg_info.al_count[ul_index]) + CFG_INFO_RESERVE_LEN) * sizeof(struct cmd_type_st);
    p_buf = OS_KMALLOC_GFP(l_len);
    if (p_buf == NULL) {
        PS_PRINT_ERR("kmalloc cmd_type_st fail\n");
        return NULL;
    }
    memset_s((void *)p_buf, l_len, 0, l_len);

    return p_buf;
}

/*
 * 函 数 名  : delete_space
 * 功能描述  : 删除字符串两边多余的空格
 * 输入参数  : string: 原始字符串
 *             len   : 字符串的长度
 * 返 回 值  : 错误返回NULL，否则返回删除两边空格以后字符串的首地址
 */
uint8 *delete_space(uint8 *string, int32 *len)
{
    int i;

    if ((string == NULL) || (len == NULL)) {
        return NULL;
    }

    /* 删除尾部的空格 */
    for (i = *len - 1; i >= 0; i--) {
        if (string[i] != COMPART_KEYWORD) {
            break;
        }
        string[i] = '\0';
    }
    /* 出错 */
    if (i < 0) {
        PS_PRINT_ERR(" string is Space bar\n");
        return NULL;
    }
    /* 在for语句中减去1，这里加上1 */
    *len = i + 1;

    /* 删除头部的空格 */
    for (i = 0; i < *len; i++) {
        if (string[i] != COMPART_KEYWORD) {
            /* 减去空格的个数 */
            *len = *len - i;
            return &string[i];
        }
    }

    return NULL;
}

/*
 * 函 数 名  : string_to_num
 * 功能描述  : 将字符串转换成正整数
 * 输入参数  : string:输入的字符串
 * 输出参数  : number:字符串转换以后的正整数
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32 string_to_num(uint8 *string, int32 *number)
{
    int32 i;
    int32 l_num;

    if (string == NULL) {
        PS_PRINT_ERR("string is NULL!\n");
        return -EFAIL;
    }

    l_num = 0;
    for (i = 0; (string[i] >= '0') && (string[i] <= '9'); i++) {
        l_num = (l_num * 10) + (string[i] - '0');   /* 字符串转数字的逻辑需要 */
    }

    *number = l_num;

    return SUCC;
}

/*
 * 函 数 名  : num_to_string
 * 功能描述  : 将正整数转换成字符串
 * 输入参数  : string:输入的正整数
 * 输出参数  : number:正整数转换以后的字符串
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32 num_to_string(uint8 *string, uint32 number)
{
    int32 i = 0;
    int32 j = 0;
    int32 tmp[INT32_STR_LEN];
    uint32 num = number;

    if (string == NULL) {
        PS_PRINT_ERR("string is NULL!\n");
        return -EFAIL;
    }

    do {
        tmp[i] = num % 10;
        num = num / 10;   /* 将正整数转字符串逻辑需要 */
        i++;
    } while (num != 0);

    do {
        string[j] = tmp[i - 1 - j] + '0';
        j++;
    } while (j != i);

    string[j] = '\0';

    return SUCC;
}

/*
 * 函 数 名  : open_file_to_readm
 * 功能描述  : 打开文件，保存read mem读上来的内容
 * 返 回 值  : 返回打开文件的描述符
 */
OS_KERNEL_FILE_STRU *open_file_to_readm(uint8 *name)
{
    OS_KERNEL_FILE_STRU *fp = NULL;
    uint8 *file_name = NULL;

    if (name == NULL) {
        file_name = WIFI_DUMP_PATH "/readm_wifi";
    } else {
        file_name = name;
    }

    fp = filp_open(file_name, O_RDWR | O_CREAT, 0664);

    return fp;
}

/*
 * 函 数 名  : recv_device_mem
 * 功能描述  : 接收device发送上来的内存，保存到指定的文件中
 * 输入参数  : fp : 保存内存的文件指针
 *             len: 需要保存的内存的长度
 * 返 回 值  : -1表示失败，否则返回实际保存的内存的长度
 */
int32 recv_device_mem(OS_KERNEL_FILE_STRU *fp, uint8 *tmp_pucDataBuf, int32 len)
{
    int32 l_ret = -EFAIL;
    mm_segment_t fs;
    uint8 retry = 3;
    int32 lenbuf = 0;

    if (OAL_IS_ERR_OR_NULL(fp)) {
        PS_PRINT_ERR("fp is error,fp = 0x%p\n", fp);
        return -EFAIL;
    }

    if (tmp_pucDataBuf == NULL) {
        PS_PRINT_ERR("tmp_pucDataBuf is NULL\n");
        return -EFAIL;
    }

    PS_PRINT_DBG("expect recv len is [%d]\n", len);

    fs = get_fs();
    set_fs(KERNEL_DS);
    PS_PRINT_DBG("pos = %d\n", (int)fp->f_pos);
    while (len > lenbuf) {
        l_ret = read_msg(tmp_pucDataBuf + lenbuf, len - lenbuf);
        if (l_ret > 0) {
            lenbuf += l_ret;
        } else {
            retry--;
            lenbuf = 0;
            if (retry == 0) {
                l_ret = -EFAIL;
                PS_PRINT_ERR("time out\n");
                break;
            }
        }
    }

    if (len <= lenbuf) {
        vfs_write(fp, tmp_pucDataBuf, len, &fp->f_pos);
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    vfs_fsync(fp, 0);
#else
    vfs_fsync(fp, fp->f_path.dentry, 0);
#endif
    set_fs(fs);

    return l_ret;
}

/*
 * 函 数 名  : check_version
 * 功能描述  : 发送命令读device版本号，并检查device上报的版本号和host的版本号是否匹配
 * 返 回 值  : -1表示失败，0表示成功
 */
int32 check_version(void)
{
    int32 l_ret;
    int32 l_len;
    int32 i;
    uint8 rec_buf[VERSION_LEN];

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        memset_s(rec_buf, VERSION_LEN, 0, VERSION_LEN);

        l_ret = memcpy_s(rec_buf, sizeof(rec_buf), (uint8 *)VER_CMD_KEYWORD, OS_STR_LEN(VER_CMD_KEYWORD));
        if (l_ret != EOK) {
            PS_PRINT_ERR("rec_buf not enough\n");
            return -EFAIL;
        }
        l_len = OS_STR_LEN(VER_CMD_KEYWORD);

        rec_buf[l_len] = COMPART_KEYWORD;
        l_len++;

        l_ret = send_msg(rec_buf, l_len);
        if (l_ret < 0) {
            PS_PRINT_ERR("send version fail![%d]\n", i);
            continue;
        }

        memset_s(cfg_info.auc_DevVersion, VERSION_LEN, 0, VERSION_LEN);
        memset_s(rec_buf, VERSION_LEN, 0, VERSION_LEN);
        msleep(1);

        l_ret = read_msg(rec_buf, VERSION_LEN);
        if (l_ret < 0) {
            PS_PRINT_ERR("read version fail![%d]\n", i);
            continue;
        }

        memcpy_s(cfg_info.auc_DevVersion, VERSION_LEN, rec_buf, VERSION_LEN);

        if (!OS_MEM_CMP((int8 *)cfg_info.auc_DevVersion,
                        (int8 *)cfg_info.auc_CfgVersion,
                        OS_STR_LEN(cfg_info.auc_CfgVersion))) {
            PS_PRINT_INFO("Device Version = [%s], CfgVersion = [%s].\n",
                          cfg_info.auc_DevVersion, cfg_info.auc_CfgVersion);
            return SUCC;
        } else {
            PS_PRINT_ERR("ERROR version,Device Version = [%s], CfgVersion = [%s].\n",
                         cfg_info.auc_DevVersion, cfg_info.auc_CfgVersion);
        }
    }

    return -EFAIL;
}

/*
 * 函 数 名  : number_type_cmd_send
 * 功能描述  : 处理number类型的命令，并发送到device
 * 输入参数  : Key  : 命令的关键字
 *             Value: 命令的参数
 * 返 回 值  : -1表示失败，非零表示成功
 */
int32 number_type_cmd_send(uint8 *Key, uint8 *Value)
{
    int32 l_ret;
    int32 data_len;
    int32 Value_len;
    int32 i;
    int32 n;
    uint8 auc_num[INT32_STR_LEN];
    uint8 buff_tx[SEND_BUF_LEN];

    Value_len = OS_STR_LEN((int8 *)Value);

    memset_s(auc_num, INT32_STR_LEN, 0, INT32_STR_LEN);
    memset_s(buff_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

    data_len = 0;
    data_len = OS_STR_LEN(Key);
    l_ret = memcpy_s(buff_tx, sizeof(buff_tx), Key, data_len);
    if (l_ret != EOK) {
        PS_PRINT_ERR("buff_tx not enough\n");
        return -EFAIL;
    }

    buff_tx[data_len] = COMPART_KEYWORD;
    data_len = data_len + 1;

    for (i = 0, n = 0; (i <= Value_len) && (n < INT32_STR_LEN); i++) {
        if ((Value[i] == ',') || (Value_len == i)) {
            PS_PRINT_DBG("auc_num = %s, i = %d, n = %d\n", auc_num, i, n);
            if (n == 0) {
                continue;
            }
            l_ret = memcpy_s((uint8 *)&buff_tx[data_len], sizeof(buff_tx) - data_len, auc_num, n);
            if (l_ret != EOK) {
                PS_PRINT_ERR("buff_tx not enough\n");
                return -EFAIL;
            }
            data_len = data_len + n;

            buff_tx[data_len] = COMPART_KEYWORD;
            data_len = data_len + 1;

            memset_s(auc_num, INT32_STR_LEN, 0, INT32_STR_LEN);
            n = 0;
        } else if (Value[i] == COMPART_KEYWORD) {
            continue;
        } else {
            auc_num[n] = Value[i];
            n++;
        }
    }

    l_ret = send_msg(buff_tx, data_len);

    return l_ret;
}

/*
 * 函 数 名  : update_device_cali_count
 * 功能描述  : 使用WRITEM命令更新device的校准次数，首次上电时为全0
 * 返 回 值  : -1表示失败，0表示成功
 */
int32 update_device_cali_count(uint8 *Key, uint8 *Value)
{
    int32 l_ret;
    uint32 len, Value_len;
    uint32 number = 0;
    uint8 *addr;
    uint8 buff_tx[SEND_BUF_LEN];

    /* 重新组合Value字符串，入参Value只是一个地址，形式为"0xXXXXX" */
    /* 组合以后的形式为"数据宽度,要写的地址,要写的值"---"4,0xXXXX,value" */
    len = 0;
    memset_s(buff_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

    /* buff_tx="" */
    buff_tx[len] = '4';
    len++;
    buff_tx[len] = ',';
    len++;

    /* buff_tx="4," */
    Value_len = OS_STR_LEN(Value);
    addr = delete_space(Value, &Value_len);
    if (addr == NULL) {
        PS_PRINT_ERR("addr is NULL, Value[%s] Value_len[%d]", Value, Value_len);
        return -EFAIL;
    }
    l_ret = memcpy_s(&buff_tx[len], sizeof(buff_tx) - len, addr, Value_len);
    if (l_ret != EOK) {
        PS_PRINT_ERR("buff_tx not enough\n");
        return -EFAIL;
    }
    len += Value_len;
    buff_tx[len] = ',';
    len++;

    /* buff_tx="4,0xXXX," */
    l_ret = get_cali_count(&number);
    l_ret += num_to_string(&buff_tx[len], number);

    /* 此时buff_tx="4,0xXXX,value" */
    /* 使用WMEM_CMD_KEYWORD命令向device发送校准次数 */
    l_ret += number_type_cmd_send(WMEM_CMD_KEYWORD, buff_tx);
    if (l_ret < 0) {
        PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", Key, buff_tx);
        return l_ret;
    }

    l_ret = recv_expect_result(MSG_FROM_DEV_WRITEM_OK);
    if (l_ret < 0) {
        PS_PRINT_ERR("recv expect result fail!\n");
        return l_ret;
    }

    return SUCC;
}

/*
 * 函 数 名  : download_bfgx_cali_data
 * 功能描述  : 使用files命令加载bfgx的校准数据
 * 返 回 值  : -1表示失败，0表示成功
 */
int32 download_bfgx_cali_data(uint8 *Key, uint8 *Value)
{
    int32  l_ret;
    uint32 len = 0;
    uint32 Value_len;
    uint8 *addr;
    uint8 buff_tx[SEND_BUF_LEN];

    /* 重新组合Value字符串，入参Value只是一个地址，形式为"0xXXXXX" */
    /* 组合以后的形式为"FILES 文件个数 要写的地址"---"FILES 1 0xXXXX " */
    memset_s(buff_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

    /* buff_tx="" */
    len = OS_STR_LEN(Key);
    l_ret = memcpy_s(buff_tx, sizeof(buff_tx), Key, len);
    if (l_ret != EOK) {
        PS_PRINT_ERR("buff_tx not enough\n");
        return -EFAIL;
    }
    buff_tx[len] = COMPART_KEYWORD;
    len++;

    /* buff_tx="FILES " */
    buff_tx[len] = '1';
    len++;
    buff_tx[len] = COMPART_KEYWORD;
    len++;

    /* buff_tx="FILES 1 " */
    Value_len = OS_STR_LEN(Value);
    addr = delete_space(Value, &Value_len);
    if (addr == NULL) {
        PS_PRINT_ERR("addr is NULL, Value[%s] Value_len[%d]", Value, Value_len);
        return -EFAIL;
    }
    l_ret = memcpy_s(&buff_tx[len], sizeof(buff_tx) - len, addr, Value_len);
    if (l_ret != EOK) {
        PS_PRINT_ERR("buff_tx not enough\n");
        return -EFAIL;
    }
    len += Value_len;
    buff_tx[len] = COMPART_KEYWORD;
    len++;

    /* buff_tx="FILES 1 0xXXXX " */
    /* 发送地址 */
    l_ret = msg_send_and_recv_except(buff_tx, len, MSG_FROM_DEV_READY_OK);
    if (l_ret < 0) {
        PS_PRINT_ERR("SEND [%s] addr error\n", Key);
        return -EFAIL;
    }

    /* 获取bfgx校准数据 */
    l_ret = get_bfgx_cali_data(firmware_down_buf, &len, firmware_down_buf_len);
    if (l_ret < 0) {
        PS_PRINT_ERR("get bfgx cali data failed, len=%d\n", len);
        return -EFAIL;
    }

    /* Wait at least 5 ms */
    oal_usleep_range(FILE_CMD_WAIT_TIME_MIN, FILE_CMD_WAIT_TIME_MAX);

    /* 发送bfgx校准数据 */
    l_ret = msg_send_and_recv_except(firmware_down_buf, len, MSG_FROM_DEV_FILES_OK);
    if (l_ret < 0) {
        PS_PRINT_ERR("send bfgx cali data fail\n");
        return -EFAIL;
    }

    return SUCC;
}

/*
 * 函 数 名  : download_dcxo_cali_data
 * 功能描述  : 使用files命令加载dcxo的校准数据
 * 返 回 值  : -1表示失败，0表示成功
 */
int32 download_dcxo_cali_data(uint8 *Key, uint8 *Value)
{
    int32 l_ret;
    uint32 len;
    uint32 Value_len;
    uint8 *addr;
    uint8 buff_tx[SEND_BUF_LEN];

    /* 重新组合Value字符串，入参Value只是一个地址，形式为"0xXXXXX" */
    /* 组合以后的形式为"FILES 文件个数 要写的地址"---"FILES 1 0xXXXX " */
    memset_s(buff_tx, sizeof(buff_tx), 0, SEND_BUF_LEN);

    /* buff_tx="" */
    len = OS_STR_LEN(Key);
    l_ret = memcpy_s(buff_tx, sizeof(buff_tx), Key, len);
    if (l_ret != EOK) {
        PS_PRINT_ERR("memcpy_s faild, l_ret = %d", l_ret);
        return -EFAIL;
    }
    buff_tx[len] = COMPART_KEYWORD;
    len++;

    /* buff_tx="FILES " */
    buff_tx[len] = '1';
    len++;
    buff_tx[len] = COMPART_KEYWORD;
    len++;

    /* buff_tx="FILES 1 " */
    Value_len = OS_STR_LEN(Value);
    addr = delete_space(Value, &Value_len);
    if (addr == NULL) {
        PS_PRINT_ERR("addr is NULL, Value[%s] Value_len[%d]", Value, Value_len);
        return -EFAIL;
    }
    l_ret = memcpy_s(&buff_tx[len], sizeof(buff_tx) - len, addr, Value_len);
    if (l_ret != EOK) {
        PS_PRINT_ERR("memcpy_s faild, l_ret = %d", l_ret);
        return -EFAIL;
    }
    len += Value_len;
    buff_tx[len] = COMPART_KEYWORD;
    len++;

    /* buff_tx="FILES 1 0xXXXX " */
    /* 发送地址 */
    l_ret = msg_send_and_recv_except(buff_tx, len, MSG_FROM_DEV_READY_OK);
    if (l_ret < 0) {
        PS_PRINT_ERR("SEND [%s] addr error\n", Key);
        return -EFAIL;
    }

    oal_usleep_range(FILE_CMD_WAIT_TIME_MIN, FILE_CMD_WAIT_TIME_MAX);

    /* 发送dcxo校准数据 */
    l_ret = msg_send_and_recv_except(pucDcxoDataBuf, DCXO_CALI_DATA_BUF_LEN, MSG_FROM_DEV_FILES_OK);
    if (l_ret < 0) {
        PS_PRINT_ERR("send bfgx cali data fail\n");
        return -EFAIL;
    }

    return SUCC;
}

/*
 * 函 数 名  : parse_file_cmd
 * 功能描述  : 解析file命令参数
 * 输入参数  : string   : file命令的参数
 *             addr     : 发送的数据地址
 *             file_path: 发送文件的路径
 * 返 回 值  : -1表示失败，0表示成功
 */
int32 parse_file_cmd(uint8 *string, unsigned long *addr, int8 **file_path)
{
    uint8 *tmp;
    int32 count = 0;
    int8 *after;

    if (string == NULL || addr == NULL || file_path == NULL) {
        PS_PRINT_ERR("param is error!\n");
        return -EFAIL;
    }

    /* 获得发送的文件的个数，此处必须为1，string字符串的格式必须是"1,0xXXXXX,file_path" */
    tmp = string;
    while (*tmp == COMPART_KEYWORD) {
        tmp++;
    }
    string_to_num(tmp, &count);
    if (count != FILE_COUNT_PER_SEND) {
        PS_PRINT_ERR("the count of send file must be 1, count = [%d]\n", count);
        return -EFAIL;
    }

    /* 让tmp指向地址的首字母 */
    tmp = OS_STR_CHR(string, ',');
    if (tmp == NULL) {
        PS_PRINT_ERR("param string is err!\n");
        return -EFAIL;
    } else {
        tmp++;
        while (*tmp == COMPART_KEYWORD) {
            tmp++;
        }
    }

    *addr = simple_strtoul(tmp, &after, 16); /* 将字符串转换成16进制数 */

    PS_PRINT_DBG("file to send addr:[0x%lx]\n", *addr);

    /* "1,0xXXXX,file_path" */
    /*         ^          */
    /*       after        */
    while (COMPART_KEYWORD == *after) {
        after++;
    }
    /* 跳过','字符 */
    after++;
    while (COMPART_KEYWORD == *after) {
        after++;
    }

    PS_PRINT_DBG("after:[%s]\n", after);

    *file_path = after;

    return SUCC;
}

void oal_print_wcpu_reg(oal_uint32 *pst_buf, oal_uint32 ul_size)
{
    oal_int32 i;
    const oal_uint32 ul_print_reg_num = 4; /* 每次打印4字节寄存器的个数 */

    ul_size = OAL_ROUND_UP(ul_size, sizeof(oal_uint32)); /* 计算4字节对齐后的长度，默认进位 */
    if (ul_size) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "print wcpu registers:");
    }

    for (i = 0; i < ul_size; i += ul_print_reg_num) {
        OAM_ERROR_LOG4(0, OAM_SF_ANY, "wcpu_reg: %x %x %x %x",
                       *(pst_buf + i + 0), *(pst_buf + i + 1),
                       *(pst_buf + i + 2), *(pst_buf + i + 3));
    }
}

#define READ_DEVICE_MAX_BUF_SIZE 128
/* read device reg by bootloader */
int32 read_device_reg16(uint32 address, uint16 *value)
{
    int32 ret, buf_len;
    const uint32 ul_read_msg_len = 4;
    const uint32 ul_dump_len = 8;
    uint8 buf_tx[READ_DEVICE_MAX_BUF_SIZE];
    uint8 buf_result[READ_DEVICE_MAX_BUF_SIZE];

    memset_s(buf_tx, READ_DEVICE_MAX_BUF_SIZE, 0, READ_DEVICE_MAX_BUF_SIZE);
    memset_s(buf_result, READ_DEVICE_MAX_BUF_SIZE, 0, READ_DEVICE_MAX_BUF_SIZE);

    buf_len = snprintf_s(buf_tx, sizeof(buf_tx), sizeof(buf_tx) - 1, "%s%c0x%x%c%d%c",
                         RMEM_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         address,
                         COMPART_KEYWORD,
                         4,
                         COMPART_KEYWORD); /* 组成 READM 0x... 4 这样的命令，4表示长度 */
    if (buf_len < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "log str format err line[%d]\n", __LINE__);
        return buf_len;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "%s", buf_tx);

    ret = send_msg(buf_tx, buf_len);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "send msg [%s] failed, ret=%d", buf_tx, ret);
        return ret;
    }

    ret = read_msg(buf_result, ul_read_msg_len);
    if (ret > 0) {
        /* 解析回读的内存,都是小端直接转换 */
        *value = (uint16)oal_readl(buf_result);
        oal_print_hex_dump(buf_result, ul_dump_len, HEX_DUMP_GROUP_SIZE, "reg16: ");
        return 0;
    }

    oal_print_hi11xx_log(HI11XX_LOG_ERR, "read_device_reg16 failed, ret=%d", ret);

    return -1;
}

/* write device regs by bootloader */
int32 write_device_reg16(uint32 address, uint16 value)
{
    int32 ret, buf_len;
    uint8 buf_tx[READ_DEVICE_MAX_BUF_SIZE];

    memset_s(buf_tx, sizeof(buf_tx), 0, sizeof(buf_tx));

    buf_len = snprintf_s(buf_tx, sizeof(buf_tx), sizeof(buf_tx) - 1, "%s%c2%c0x%x%c%d%c",
                         WMEM_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         COMPART_KEYWORD,
                         address,
                         COMPART_KEYWORD,
                         value,
                         COMPART_KEYWORD);
    if (buf_len < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "log str format err line[%d]\n", __LINE__);
        return buf_len;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "%s", buf_tx);

    ret = send_msg(buf_tx, buf_len);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "send msg [%s] failed, ret=%d", buf_tx, ret);
        return ret;
    }

    ret = recv_expect_result(MSG_FROM_DEV_WRITEM_OK);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "send msg [%s] recv failed, ret=%d", buf_tx, ret);
        return ret;
    }

    return 0;
}
#ifdef HI110X_HAL_MEMDUMP_ENABLE
int32 recv_device_memdump(uint8 *pucDataBuf, int32 len)
{
    int32 l_ret = -EFAIL;
    uint8 retry = 3;
    int32 lenbuf = 0;

    if (pucDataBuf == NULL) {
        PS_PRINT_ERR("pucDataBuf is NULL\n");
        return -EFAIL;
    }

    PS_PRINT_DBG("expect recv len is [%d]\n", len);

    while (len > lenbuf) {
        l_ret = read_msg(pucDataBuf + lenbuf, len - lenbuf);
        if (l_ret > 0) {
            lenbuf += l_ret;
        } else {
            retry--;
            lenbuf = 0;
            if (retry == 0) {
                l_ret = -EFAIL;
                PS_PRINT_ERR("time out\n");
                break;
            }
        }
    }

    if (len <= lenbuf) {
        wifi_memdump_enquenue(pucDataBuf, len);
    }

    return l_ret;
}
int32 sdio_read_device_mem(struct st_wifi_dump_mem_info *pst_mem_dump_info,
                           uint8 *pucDataBuf,
                           uint32 ulDataBufLen)
{
    uint8 buf_tx[SEND_BUF_LEN];
    int32 ret = 0;
    uint32 size = 0;
    uint32 offset;
    uint32 remainder = pst_mem_dump_info->size;

    offset = 0;
    while (remainder > 0) {
        memset_s(buf_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

        size = min(remainder, ulDataBufLen);
        ret = snprintf_s(buf_tx, sizeof(buf_tx), sizeof(buf_tx) - 1, "%s%c0x%lx%c%d%c",
                         RMEM_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         pst_mem_dump_info->mem_addr + offset,
                         COMPART_KEYWORD,
                         size,
                         COMPART_KEYWORD);
        if (ret < 0) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "log str format err line[%d]\n", __LINE__);
            break;
        }
        PS_PRINT_DBG("read mem cmd:[%s]\n", buf_tx);
        send_msg(buf_tx, OS_STR_LEN(buf_tx));

        ret = recv_device_memdump(pucDataBuf, size);
        if (ret < 0) {
            PS_PRINT_ERR("wifi mem dump fail, filename is [%s],ret=%d\n", pst_mem_dump_info->file_name, ret);
            break;
        }

#ifdef CONFIG_PRINTK
        if (offset == 0) {
            oal_int8 *pst_file_name = (pst_mem_dump_info->file_name ?
                                       ((oal_int8 *)pst_mem_dump_info->file_name) : (oal_int8 *)"default: ");
            if (!oal_strcmp("wifi_device_panic_mem", pst_file_name)) {
                /* dump the device cpu reg mem when panic,
                  24B mem header + 24*4 reg info */
                if (size > CPU_PANIC_MEMDUMP_SIZE) {
                    oal_print_hex_dump(pucDataBuf + CPU_PANIC_MEMDUMP_HEAD_SIZE, CPU_PANIC_MEMDUMP_INFO_SIZE,
                                       HEX_DUMP_GROUP_SIZE, pst_file_name);
                    /* print sdt log */
#ifdef CONFIG_MMC
                    /* dump 24*4B */
                    oal_print_wcpu_reg ((oal_uint32 *)(pucDataBuf + CPU_PANIC_MEMDUMP_HEAD_SIZE),
                                        CPU_PANIC_MEMDUMP_INFO_SIZE / 4); /* 因寄存器宽度是4字节，所以这里入参要除4 */
#endif
                }
            }
        }
#endif

        offset += size;

        remainder -= size;
    }

    return ret;
}

/*
 * 函 数 名  : wifi_device_mem_dump
 * 功能描述  : firmware加载时读取wifi的内存
 * 输入参数  : pst_mem_dump_info  : 需要读取的内存信息
 *             count              : 需要读取的内存块个数
 * 返 回 值  : -1表示失败，0表示成功
 */
int32 wifi_device_mem_dump(struct st_wifi_dump_mem_info *pst_mem_dump_info, uint32 count)
{
    int32 ret = -EFAIL;
    uint32 i;
    uint8 *pucDataBuf = NULL;
    const uint32 ul_buff_size = 100;
    uint8 buff[ul_buff_size];
    uint32 *pcount = (uint32 *)&buff[0];
    uint32 sdio_transfer_limit = hcc_get_max_trans_size(hcc_get_110x_handler());

    /* μ??ú′??è????3é1|?ê,ò3′óD?????μ??ú′?èYò×éê??3é1|?￡ */
    sdio_transfer_limit = OAL_MIN(PAGE_SIZE, sdio_transfer_limit);

    if (pst_mem_dump_info == NULL) {
        PS_PRINT_ERR("pst_wifi_dump_info is NULL\n");
        return -EFAIL;
    }

    do {
        PS_PRINT_INFO("try to malloc mem dump buf len is [%d]\n", sdio_transfer_limit);
        pucDataBuf = (uint8 *)OS_KMALLOC_GFP(sdio_transfer_limit);
        if (pucDataBuf == NULL) {
            PS_PRINT_WARNING("malloc mem  len [%d] fail, continue to try in a smaller size\n", sdio_transfer_limit);
            sdio_transfer_limit = sdio_transfer_limit >> 1;
        }
    } while ((pucDataBuf == NULL) && (sdio_transfer_limit >= MIN_FIRMWARE_FILE_TX_BUF_LEN));

    if (pucDataBuf == NULL) {
        PS_PRINT_ERR("pucDataBuf KMALLOC failed\n");
        return -EFAIL;
    }

    PS_PRINT_INFO("mem dump data buf len is [%d]\n", sdio_transfer_limit);

    wifi_notice_hal_memdump();

    for (i = 0; i < count; i++) {
        *pcount = pst_mem_dump_info[i].size;
        PS_PRINT_INFO("mem dump data size [%d]==> [%d]\n", *pcount, pst_mem_dump_info[i].size);
        wifi_memdump_enquenue(buff, 4);  /* 后续申请sk_buff的大小 */
        ret = sdio_read_device_mem(&pst_mem_dump_info[i], pucDataBuf, sdio_transfer_limit);
        if (ret < 0) {
            break;
        }
    }
    wifi_memdump_finish();

    OS_MEM_KFREE(pucDataBuf);

    return ret;
}

#else

/*
 * 函 数 名  : sdio_read_device_mem
 * 功能描述  : 当device处于bootloader时从DEVICE读取内存
 * 返 回 值  : 小于0表示失败
 */
int32 sdio_read_device_mem(struct st_wifi_dump_mem_info *pst_mem_dump_info,
                           OS_KERNEL_FILE_STRU *fp,
                           uint8 *tmp_pucDataBuf,
                           uint32 tmp_ulDataBufLen)
{
    uint8 buf_tx[SEND_BUF_LEN];
    int32 ret = 0;
    uint32 size = 0;
    uint32 offset;
    uint32 remainder = pst_mem_dump_info->size;

    offset = 0;
    while (remainder > 0) {
        memset_s(buf_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

        size = min(remainder, tmp_ulDataBufLen);
        ret = snprintf_s(buf_tx, sizeof(buf_tx), sizeof(buf_tx) - 1, "%s%c0x%lx%c%d%c",
                         RMEM_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         pst_mem_dump_info->mem_addr + offset,
                         COMPART_KEYWORD,
                         size,
                         COMPART_KEYWORD);
        if (ret < 0) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "log str format err line[%d]\n", __LINE__);
            break;
        }
        PS_PRINT_DBG("read mem cmd:[%s]\n", buf_tx);
        send_msg(buf_tx, OS_STR_LEN(buf_tx));

        ret = recv_device_mem(fp, tmp_pucDataBuf, size);
        if (ret < 0) {
            PS_PRINT_ERR("wifi mem dump fail, filename is [%s],ret=%d\n", pst_mem_dump_info->file_name, ret);
            break;
        }

#ifdef CONFIG_PRINTK
        if (offset == 0) {
            oal_int8 *pst_file_name = (pst_mem_dump_info->file_name ?
                                       ((oal_int8 *)pst_mem_dump_info->file_name) : (oal_int8 *)"default: ");
            if (!oal_strcmp("wifi_device_panic_mem", pst_file_name)) {
                /* dump the device cpu reg mem when panic,
                  24B mem header + 24*4 reg info */
                if (size > CPU_PANIC_MEMDUMP_SIZE) {
                    oal_print_hex_dump(tmp_pucDataBuf + CPU_PANIC_MEMDUMP_HEAD_SIZE, CPU_PANIC_MEMDUMP_INFO_SIZE,
                                       HEX_DUMP_GROUP_SIZE, pst_file_name);
                    /* print sdt log */
#ifdef CONFIG_MMC
                    /* dump 24*4B */
                    oal_print_wcpu_reg ((oal_uint32 *)(tmp_pucDataBuf + CPU_PANIC_MEMDUMP_HEAD_SIZE),
                                        CPU_PANIC_MEMDUMP_INFO_SIZE / 4); /* 因寄存器宽度是4字节，所以这里入参要除4 */
#endif
                }
            }
        }
#endif

        offset += size;

        remainder -= size;
    }

    return ret;
}

/*
 * 函 数 名  : wifi_device_mem_dump
 * 功能描述  : firmware加载时读取wifi的内存
 * 输入参数  : pst_mem_dump_info  : 需要读取的内存信息
 *             count              : 需要读取的内存块个数
 * 返 回 值  : -1表示失败，0表示成功
 */
int32 wifi_device_mem_dump(struct st_wifi_dump_mem_info *pst_mem_dump_info, uint32 count)
{
    OS_KERNEL_FILE_STRU *fp = NULL;
    int32 ret = -EFAIL;
    uint32 i;
    const uint32 ul_filename_len = 100;
    char filename[ul_filename_len];

    ktime_t time_start, time_stop;
    oal_uint64 trans_us;
    uint8 *tmp_pucDataBuf = NULL;
    uint32 sdio_transfer_limit = hcc_get_max_trans_size(hcc_get_110x_handler());

    /* 导内存先考虑成功率,页大小对齐的内存容易申请成功。 */
    sdio_transfer_limit = OAL_MIN(PAGE_SIZE, sdio_transfer_limit);

    if (pst_mem_dump_info == NULL) {
        PS_PRINT_ERR("pst_wifi_dump_info is NULL\n");
        return -EFAIL;
    }

    do {
        PS_PRINT_INFO("try to malloc mem dump buf len is [%d]\n", sdio_transfer_limit);
        tmp_pucDataBuf = (uint8 *)OS_KMALLOC_GFP(sdio_transfer_limit);
        if (tmp_pucDataBuf == NULL) {
            PS_PRINT_WARNING("malloc mem  len [%d] fail, continue to try in a smaller size\n", sdio_transfer_limit);
            sdio_transfer_limit = sdio_transfer_limit >> 1;
        }
    } while ((tmp_pucDataBuf == NULL) && (sdio_transfer_limit >= MIN_FIRMWARE_FILE_TX_BUF_LEN));

    if (tmp_pucDataBuf == NULL) {
        PS_PRINT_ERR("tmp_pucDataBuf KMALLOC failed\n");
        return -EFAIL;
    }

    PS_PRINT_INFO("mem dump data buf len is [%d]\n", sdio_transfer_limit);

    plat_wait_last_rotate_finish();

    for (i = 0; i < count; i++) {
        time_start = ktime_get();
        /* 打开文件，准备接受wifi mem dump */
        memset_s(filename, sizeof(filename), 0, sizeof(filename));
        ret = snprintf_s(filename, sizeof(filename), sizeof(filename) - 1, WIFI_DUMP_PATH "/%s_%s.bin",
                         SDIO_STORE_WIFI_MEM, pst_mem_dump_info[i].file_name);
        if (ret < 0) {
            PS_PRINT_ERR("filename format str err\n");
            break;
        }
        PS_PRINT_INFO("readm %s\n", filename);

        fp = open_file_to_readm(filename);
        if (OAL_IS_ERR_OR_NULL(fp)) {
            PS_PRINT_ERR("create file error,fp = 0x%p, filename is [%s]\n", fp, pst_mem_dump_info[i].file_name);
            break;
        }

        ret = sdio_read_device_mem(&pst_mem_dump_info[i], fp, tmp_pucDataBuf, sdio_transfer_limit);
        if (ret < 0) {
            filp_close(fp, NULL);
            break;
        }
        filp_close(fp, NULL);
        time_stop = ktime_get();
        trans_us = (oal_uint64)ktime_to_us(ktime_sub(time_stop, time_start));
        OAL_IO_PRINT("device get mem %s cost %llu us\n", filename, trans_us);
    }

    /* send cmd to oam_hisi to rotate file */
    plat_send_rotate_cmd_2_app(CMD_READM_WIFI_SDIO);

    OS_MEM_KFREE(tmp_pucDataBuf);

    return ret;
}

#endif

/* 可以指定readm存储的文件路径，file_name传入前已判空 */
int32 sdio_read_path_mem(uint8 *Key, uint8 *Value, uint8 *file_name)
{
    int32 l_ret;
    uint32 size, readlen;
    int32 retry = 3;
    uint8 *flag;
    OS_KERNEL_FILE_STRU *fp = NULL;
    uint8 *tmp_pucDataBuf = NULL;
    uint32 sdio_transfer_limit = hcc_get_max_trans_size(hcc_get_110x_handler());

    /* 导内存先考虑成功率,页大小对齐的内存容易申请成功。 */
    sdio_transfer_limit = OAL_MIN(PAGE_SIZE, sdio_transfer_limit);

    flag = OS_STR_CHR(Value, ',');
    if (flag == NULL) {
        PS_PRINT_ERR("RECV LEN ERROR..\n");
        return -EFAIL;
    }
    flag++;
    PS_PRINT_DBG("recv len [%s]\n", flag);
    while (*flag == COMPART_KEYWORD) {
        flag++;
    }

    string_to_num(flag, &size);

    do {
        PS_PRINT_INFO("try to malloc sdio mem read buf len is [%d]\n", sdio_transfer_limit);
        tmp_pucDataBuf = (uint8 *)OS_KMALLOC_GFP(sdio_transfer_limit);
        if (tmp_pucDataBuf == NULL) {
            PS_PRINT_WARNING("malloc mem len [%d] fail, continue to try in a smaller size\n", sdio_transfer_limit);
            sdio_transfer_limit = sdio_transfer_limit >> 1;
        }
    } while ((tmp_pucDataBuf == NULL) && (sdio_transfer_limit >= MIN_FIRMWARE_FILE_TX_BUF_LEN));

    if (tmp_pucDataBuf == NULL) {
        PS_PRINT_ERR("tmp_pucDataBuf KMALLOC failed\n");
        return -EFAIL;
    }

    fp = open_file_to_readm(file_name);
    if (IS_ERR(fp)) {
        PS_PRINT_ERR("create file error,fp = 0x%p\n", fp);
        OS_MEM_KFREE(tmp_pucDataBuf);
        return SUCC;
    }

    l_ret = number_type_cmd_send(Key, Value);
    if (l_ret < 0) {
        PS_PRINT_ERR("send %s,%s fail \n", Key, Value);
        filp_close(fp, NULL);
        OS_MEM_KFREE(tmp_pucDataBuf);
        return l_ret;
    }

    PS_PRINT_DBG("recv len [%d]\n", size);
    while (size > 0) {
        readlen = min(size, sdio_transfer_limit);
        l_ret = recv_device_mem(fp, tmp_pucDataBuf, size);
        if (l_ret > 0) {
            size -= l_ret;
        } else {
            PS_PRINT_ERR("read error retry:%d\n", retry);
            --retry;
            if (!retry) {
                PS_PRINT_ERR("retry fail\n");
                break;
            }
        }
    }

    filp_close(fp, NULL);
    OS_MEM_KFREE(tmp_pucDataBuf);

    return l_ret;
}

int32 sdio_read_mem(uint8 *Key, uint8 *Value)
{
    int32 l_ret;
    uint32 size, readlen;
    int32 retry = 3;
    uint8 *flag;
    OS_KERNEL_FILE_STRU *fp = NULL;
    uint8 *tmp_pucDataBuf = NULL;
    uint32 sdio_transfer_limit = hcc_get_max_trans_size(hcc_get_110x_handler());

    /* 导内存先考虑成功率,页大小对齐的内存容易申请成功。 */
    sdio_transfer_limit = OAL_MIN(PAGE_SIZE, sdio_transfer_limit);

    flag = OS_STR_CHR(Value, ',');
    if (flag == NULL) {
        PS_PRINT_ERR("RECV LEN ERROR..\n");
        return -EFAIL;
    }
    flag++;
    PS_PRINT_DBG("recv len [%s]\n", flag);
    while (*flag == COMPART_KEYWORD) {
        flag++;
    }

    string_to_num(flag, &size);

    do {
        PS_PRINT_INFO("try to malloc sdio mem read buf len is [%d]\n", sdio_transfer_limit);
        tmp_pucDataBuf = (uint8 *)OS_KMALLOC_GFP(sdio_transfer_limit);
        if (tmp_pucDataBuf == NULL) {
            PS_PRINT_WARNING("malloc mem len [%d] fail, continue to try in a smaller size\n", sdio_transfer_limit);
            sdio_transfer_limit = sdio_transfer_limit >> 1;
        }
    } while ((tmp_pucDataBuf == NULL) && (sdio_transfer_limit >= MIN_FIRMWARE_FILE_TX_BUF_LEN));

    if (tmp_pucDataBuf == NULL) {
        PS_PRINT_ERR("tmp_pucDataBuf KMALLOC failed\n");
        return -EFAIL;
    }

    fp = open_file_to_readm(NULL);
    if (IS_ERR(fp)) {
        PS_PRINT_ERR("create file error,fp = 0x%p\n", fp);
        OS_MEM_KFREE(tmp_pucDataBuf);
        return SUCC;
    }

    l_ret = number_type_cmd_send(Key, Value);
    if (l_ret < 0) {
        PS_PRINT_ERR("send %s,%s fail \n", Key, Value);
        filp_close(fp, NULL);
        OS_MEM_KFREE(tmp_pucDataBuf);
        return l_ret;
    }

    PS_PRINT_DBG("recv len [%d]\n", size);
    while (size > 0) {
        readlen = min(size, sdio_transfer_limit);
        l_ret = recv_device_mem(fp, tmp_pucDataBuf, size);
        if (l_ret > 0) {
            size -= l_ret;
        } else {
            PS_PRINT_ERR("read error retry:%d\n", retry);
            --retry;
            if (!retry) {
                PS_PRINT_ERR("retry fail\n");
                break;
            }
        }
    }

    filp_close(fp, NULL);
    OS_MEM_KFREE(tmp_pucDataBuf);

    return l_ret;
}

/*
 * 函 数 名  : exec_number_type_cmd
 * 功能描述  : 执行number类型的命令
 * 输入参数  : Key  : 命令的关键字
 *             Value: 命令的参数
 * 返 回 值  : -1表示失败，0表示成功
 */
int32 exec_number_type_cmd(uint8 *Key, uint8 *Value)
{
    int32 l_ret = -EFAIL;
    BOARD_INFO *tmp_board_info = NULL;

    tmp_board_info = get_hi110x_board_info();
    if (tmp_board_info == NULL) {
        PS_PRINT_ERR("tmp_board_info is null!\n");
        return -EFAIL;
    }

    if (!OS_MEM_CMP(Key, VER_CMD_KEYWORD, OS_STR_LEN(VER_CMD_KEYWORD))) {
        l_ret = check_version();
        if (l_ret < 0) {
            PS_PRINT_ERR("check version FAIL [%d]\n", l_ret);
            return -EFAIL;
        }
    }

    if (!OS_STR_CMP((int8 *)Key, WMEM_CMD_KEYWORD)) {
        if (NULL != OS_STR_STR((int8 *)Value, (int8 *)STR_REG_NFC_EN_KEEP)) {
            if (V100 == get_ec_version()) {
                PS_PRINT_INFO("hi110x V100\n");
            } else {
                PS_PRINT_INFO("hi110x V120\n");
                return SUCC;
            }
        }

        l_ret = number_type_cmd_send(Key, Value);
        if (l_ret < 0) {
            PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", Key, Value);
            return l_ret;
        }

        l_ret = recv_expect_result(MSG_FROM_DEV_WRITEM_OK);
        if (l_ret < 0) {
            PS_PRINT_ERR("recv expect result fail!\n");
            return l_ret;
        }

    } else if (!OS_STR_CMP((int8 *)Key, CALI_COUNT_CMD_KEYWORD)) {
        /* 加载校准次数到device */
        l_ret = update_device_cali_count(Key, Value);
        if (l_ret < 0) {
            PS_PRINT_ERR("update device cali count fail\n");
            return l_ret;
        }
    } else if (!OS_STR_CMP((int8 *)Key, CALI_BFGX_DATA_CMD_KEYWORD)) {
        if (ir_only_mode) {
            PS_PRINT_INFO("ir only pass the download cali data cmd\n");
            return SUCC;
        }

        /* 加载BFGX的校准数据 */
        l_ret = download_bfgx_cali_data(FILES_CMD_KEYWORD, Value);
        if (l_ret < 0) {
            PS_PRINT_ERR("download bfgx cali data fail\n");
            return l_ret;
        }
    } else if (!OS_STR_CMP((int8 *)Key, CALI_DCXO_DATA_CMD_KEYWORD)) {
        PS_PRINT_DBG("download dcxo cali data begin\n");
        /* 发送TCXO 的校准参数到devcie */
        l_ret = download_dcxo_cali_data(FILES_CMD_KEYWORD, Value);
        if (l_ret < 0) {
            PS_PRINT_ERR("download dcxo cali data fail\n");
            return l_ret;
        }
    } else if (!OS_STR_CMP((int8 *)Key, JUMP_CMD_KEYWORD)) {
        l_ret = number_type_cmd_send(Key, Value);
        if (l_ret < 0) {
            PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", Key, Value);
            return l_ret;
        }

        /* 100000ms timeout */
        l_ret = recv_expect_result_timeout(MSG_FROM_DEV_JUMP_OK, READ_MEG_JUMP_TIMEOUT);
        if (l_ret >= 0) {
            PS_PRINT_INFO("JUMP success!\n");
            return l_ret;
        } else {
            PS_PRINT_ERR("CMD JUMP timeout! l_ret=%d\n", l_ret);
            return l_ret;
        }
    } else if (!OS_STR_CMP((int8 *)Key, SETPM_CMD_KEYWORD) || !OS_STR_CMP((int8 *)Key, SETBUCK_CMD_KEYWORD) ||
               !OS_STR_CMP((int8 *)Key, SETSYSLDO_CMD_KEYWORD) || !OS_STR_CMP((int8 *)Key, SETNFCRETLDO_CMD_KEYWORD) ||
               !OS_STR_CMP((int8 *)Key, SETPD_CMD_KEYWORD) || !OS_STR_CMP((int8 *)Key, SETNFCCRG_CMD_KEYWORD) ||
               !OS_STR_CMP((int8 *)Key, SETABB_CMD_KEYWORD) || !OS_STR_CMP((int8 *)Key, SETTCXODIV_CMD_KEYWORD)) {
        l_ret = number_type_cmd_send(Key, Value);
        if (l_ret < 0) {
            PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", Key, Value);
            return l_ret;
        }

        l_ret = recv_expect_result(MSG_FROM_DEV_SET_OK);
        if (l_ret < 0) {
            PS_PRINT_ERR("recv expect result fail!\n");
            return l_ret;
        }
    } else if (!OS_STR_CMP((int8 *)Key, RMEM_CMD_KEYWORD)) {
        l_ret = sdio_read_mem(Key, Value);
    }

    return l_ret;
}

/*
 * 函 数 名  : exec_quit_type_cmd
 * 功能描述  : 执行quit类型的命令
 * 返 回 值  : -1表示失败，0表示成功
 */
int32 exec_quit_type_cmd(void)
{
    int32 l_ret;
    int32 l_len;
    const uint32 ul_buf_len = 8;
    uint8 buf[ul_buf_len];
    BOARD_INFO *tmp_board_info = NULL;

    tmp_board_info = get_hi110x_board_info();
    if (tmp_board_info == NULL) {
        PS_PRINT_ERR("tmp_board_info is null!\n");
        return -EFAIL;
    }

    memset_s(buf, sizeof(buf), 0, sizeof(buf));

    l_ret = memcpy_s(buf, sizeof(buf), (uint8 *)QUIT_CMD_KEYWORD, OS_STR_LEN(QUIT_CMD_KEYWORD));
    if (l_ret != EOK) {
        PS_PRINT_ERR("memcpy_s faild, l_ret = %d", l_ret);
        return -EFAIL;
    }
    l_len = OS_STR_LEN(QUIT_CMD_KEYWORD);

    buf[l_len] = COMPART_KEYWORD;
    l_len++;

    l_ret = msg_send_and_recv_except(buf, l_len, MSG_FROM_DEV_QUIT_OK);

    return l_ret;
}

/*
 * 函 数 名  : exec_file_type_cmd
 * 功能描述  : 执行file类型的命令
 * 输入参数  : Key  : 命令的关键字
 *             Value: 命令的参数
 * 返 回 值  : -1表示失败，0表示成功
 */
int32 exec_file_type_cmd(uint8 *Key, uint8 *Value)
{
    unsigned long addr;
    unsigned long addr_send;
    int8 *path;
    int32 ret;
    uint32 file_len;
    uint32 transmit_limit;
    uint32 per_send_len;
    uint32 send_count;
    int32 rdlen;
    uint32 i;
    uint32 offset = 0;
    uint8 buff_tx[SEND_BUF_LEN] = {0};
    OS_KERNEL_FILE_STRU *fp = NULL;
    BOARD_INFO *tmp_board_info = NULL;

    tmp_board_info = get_hi110x_board_info();
    if (tmp_board_info == NULL) {
        PS_PRINT_ERR("tmp_board_info is null!\n");
        return -EFAIL;
    }

    if (ir_only_mode) {
        PS_PRINT_INFO("ir only pass the download file cmd\n");
        return SUCC;
    }

    ret = parse_file_cmd(Value, &addr, &path);
    if (ret < 0) {
        PS_PRINT_ERR("parse file cmd fail!\n");
        return ret;
    }

    PS_PRINT_INFO("download firmware:%s addr:0x%x\n", path, (uint32)addr);

    fp = filp_open(path, O_RDONLY, 0);
    if (OAL_IS_ERR_OR_NULL(fp)) {
        PS_PRINT_ERR("filp_open [%s] fail!!, fp=%p\n", path, fp);
        fp = NULL;
        return -EFAIL;
    }

    /* 获取file文件大小 */
    file_len = vfs_llseek(fp, 0, SEEK_END);
    if (file_len <= 0) {
        PS_PRINT_ERR("file size of %s is 0!!\n", path);
        filp_close(fp, NULL);
        return -EFAIL;
    }

    /* 恢复fp->f_pos到文件开头 */
    vfs_llseek(fp, 0, SEEK_SET);

    PS_PRINT_DBG("file len is [%d]\n", file_len);

    transmit_limit = firmware_down_buf_len;
    per_send_len = (transmit_limit > file_len) ? file_len : transmit_limit;
    send_count = (file_len + per_send_len - 1) / per_send_len;

    for (i = 0; i < send_count; i++) {
        rdlen = oal_file_read_ext(fp, fp->f_pos, firmware_down_buf, per_send_len);

        if (rdlen > 0) {
            PS_PRINT_DBG("len of kernel_read is [%d], i=%d\n", rdlen, i);
            fp->f_pos += rdlen;
        } else {
            PS_PRINT_ERR("len of kernel_read is error! ret=[%d], i=%d\n", rdlen, i);
            filp_close(fp, NULL);
            return rdlen;
        }

        addr_send = addr + offset;
        PS_PRINT_DBG("send addr is [0x%lx], i=%d\n", addr_send, i);
        ret = snprintf_s(buff_tx, SEND_BUF_LEN, SEND_BUF_LEN - 1, "%s%c%d%c0x%lx%c",
                         FILES_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         FILE_COUNT_PER_SEND,
                         COMPART_KEYWORD,
                         addr_send,
                         COMPART_KEYWORD);
        if (ret < 0) {
            PS_PRINT_ERR("send file addr cmd failed\n");
            filp_close(fp, NULL);
            return -EFAIL;
        }
        /* 发送地址 */
        PS_PRINT_DBG("send file addr cmd is [%s]\n", buff_tx);
        ret = msg_send_and_recv_except(buff_tx, OS_STR_LEN(buff_tx), MSG_FROM_DEV_READY_OK);
        if (ret < 0) {
            PS_PRINT_ERR("SEND [%s] error\n", buff_tx);
            filp_close(fp, NULL);
            return -EFAIL;
        }

        /* Wait at least 5 ms */
        oal_usleep_range(FILE_CMD_WAIT_TIME_MIN, FILE_CMD_WAIT_TIME_MAX);

        /* 发送文件内容 */
        ret = msg_send_and_recv_except(firmware_down_buf, rdlen, MSG_FROM_DEV_FILES_OK);
        if (ret < 0) {
            PS_PRINT_ERR(" sdio send data fail\n");
            filp_close(fp, NULL);
            return -EFAIL;
        }
        offset += rdlen;
    }

    filp_close(fp, NULL);

    /* 发送的长度要和文件的长度一致 */
    if (offset != file_len) {
        PS_PRINT_ERR("file send len is err! send len is [%d], file len is [%d]\n", offset, file_len);
        return -EFAIL;
    }

    return SUCC;
}

/*
 * 函 数 名  : exec_shutdown_type_cmd
 * 功能描述  : 执行shutdown cpu type的命令
 * 输入参数  : which_cpu: 要关闭的cpu
 * 返 回 值  : -1表示失败，非负数表示成功
 */
int32 exec_shutdown_type_cmd(uint32 which_cpu)
{
    int32 l_ret = -EFAIL;
    uint8 Value_SHUTDOWN[SHUTDOWN_TX_CMD_LEN];

    if (which_cpu == DEV_WCPU) {
        l_ret = snprintf_s(Value_SHUTDOWN, sizeof(Value_SHUTDOWN), sizeof(Value_SHUTDOWN) - 1, "%d,%s,%d",
                           HOST_TO_DEVICE_CMD_HEAD, SOFT_WCPU_EN_ADDR, 0);
        if (l_ret < 0) {
            PS_PRINT_ERR("SOFT_WCPU_EN_ADDR cmd format failed\n");
            return l_ret;
        }

        l_ret = number_type_cmd_send(WMEM_CMD_KEYWORD, Value_SHUTDOWN);
        if (l_ret < 0) {
            PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", SHUTDOWN_WIFI_CMD_KEYWORD, Value_SHUTDOWN);
            return l_ret;
        }
    } else if (which_cpu == DEV_BCPU) {
        l_ret = snprintf_s(Value_SHUTDOWN, sizeof(Value_SHUTDOWN), sizeof(Value_SHUTDOWN) - 1, "%d,%s,%d",
                           HOST_TO_DEVICE_CMD_HEAD, SOFT_BCPU_EN_ADDR, 0);
        if (l_ret < 0) {
            PS_PRINT_ERR("SOFT_BCPU_EN_ADDR cmd format failed\n");
            return l_ret;
        }

        l_ret = number_type_cmd_send(WMEM_CMD_KEYWORD, Value_SHUTDOWN);
        if (l_ret < 0) {
            PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", SHUTDOWN_BFGX_CMD_KEYWORD, Value_SHUTDOWN);
            return l_ret;
        }

        l_ret = recv_expect_result(MSG_FROM_DEV_WRITEM_OK);
        if (l_ret < 0) {
            PS_PRINT_ERR("recv expect result fail!\n");
            return l_ret;
        }

        l_ret = snprintf_s(Value_SHUTDOWN, sizeof(Value_SHUTDOWN), sizeof(Value_SHUTDOWN) - 1, "%d,%s,%d",
                           HOST_TO_DEVICE_CMD_HEAD, BCPU_DE_RESET_ADDR, 1);
        if (l_ret < 0) {
            PS_PRINT_ERR("BCPU_DE_RESET_ADDR cmd format failed\n");
            return l_ret;
        }

        l_ret = number_type_cmd_send(WMEM_CMD_KEYWORD, Value_SHUTDOWN);
        if (l_ret < 0) {
            PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", SHUTDOWN_BFGX_CMD_KEYWORD, Value_SHUTDOWN);
            return l_ret;
        }

        l_ret = recv_expect_result(MSG_FROM_DEV_WRITEM_OK);
        if (l_ret < 0) {
            PS_PRINT_ERR("recv expect result fail!\n");
            return l_ret;
        }
    } else {
        PS_PRINT_ERR("para is error, which_cpu=[%d]\n", which_cpu);
        return -EFAIL;
    }

    return SUCC;
}

/*
 * 函 数 名  : execute_download_cmd
 * 功能描述  : 执行firmware download的命令
 * 输入参数  : cmd_type: 加载命令的类型
 *             cmd_name: 命令的关键字
 *             cmd_para: 命令的参数
 * 返 回 值  : -1表示失败，非负数表示成功
 */
int32 execute_download_cmd(int32 cmd_type, uint8 *cmd_name, uint8 *cmd_para)
{
    int32 l_ret;

    switch (cmd_type) {
        case FILE_TYPE_CMD:
            PS_PRINT_DBG(" command type FILE_TYPE_CMD\n");
            l_ret = exec_file_type_cmd(cmd_name, cmd_para);
            break;
        case NUM_TYPE_CMD:
            PS_PRINT_DBG(" command type NUM_TYPE_CMD\n");
            l_ret = exec_number_type_cmd(cmd_name, cmd_para);
            break;
        case QUIT_TYPE_CMD:
            PS_PRINT_DBG(" command type QUIT_TYPE_CMD\n");
            l_ret = exec_quit_type_cmd();
            break;
        case SHUTDOWN_WIFI_TYPE_CMD:
            PS_PRINT_DBG(" command type SHUTDOWN_WIFI_TYPE_CMD\n");
            l_ret = exec_shutdown_type_cmd(DEV_WCPU);
            break;
        case SHUTDOWN_BFGX_TYPE_CMD:
            PS_PRINT_DBG(" command type SHUTDOWN_BFGX_TYPE_CMD\n");
            l_ret = exec_shutdown_type_cmd(DEV_BCPU);
            break;

        default:
            PS_PRINT_ERR("command type error[%d]\n", cmd_type);
            l_ret = -EFAIL;
            break;
    }

    return l_ret;
}

/*
 * 函 数 名  : firmware_read_cfg
 * 功能描述  : 读取cfg文件的内容，放到驱动动态分配的buffer中
 * 输入参数  : puc_CfgPatch    : cfg文件的路径
 *             puc_read_buffer : 保存cfg文件内容的buffer
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32 firmware_read_cfg(uint8 *puc_CfgPatch, uint8 *puc_read_buffer)
{
    OS_KERNEL_FILE_STRU *fp = NULL;
    int32 l_ret;

    if ((puc_CfgPatch == NULL) || (puc_read_buffer == NULL)) {
        PS_PRINT_ERR("para is NULL\n");
        return -EFAIL;
    }

    fp = filp_open(puc_CfgPatch, O_RDONLY, 0);
    if (OAL_IS_ERR_OR_NULL(fp)) {
        PS_PRINT_ERR("open file %s fail, fp=%p\n", puc_CfgPatch, fp);
        fp = NULL;
        return -EFAIL;
    }

    memset_s(puc_read_buffer, READ_CFG_BUF_LEN, 0, READ_CFG_BUF_LEN);

    l_ret = oal_file_read_ext(fp, fp->f_pos, puc_read_buffer, READ_CFG_BUF_LEN);

    filp_close(fp, NULL);
    fp = NULL;

    return l_ret;
}

/*
 * 函 数 名  : firmware_parse_cmd
 * 功能描述  : 解析cfg文件中的命令
 * 输入参数  : puc_cfg_buffer: 保存cfg文件内容的buffer
 *             puc_cmd_name  : 保存解析以后命令关键字的buffer
 *             puc_cmd_para  : 保存解析以后命令参数的buffer
 * 返 回 值  : 返回命令的类型
 */
int32 firmware_parse_cmd(uint8 *puc_cfg_buffer, uint8 *puc_cmd_name, uint32 cmd_name_len,
                         uint8 *puc_cmd_para, uint32 cmd_para_len)
{
    int32 l_ret;
    int32 cmd_type;
    int32 l_cmdlen;
    int32 l_paralen;
    uint8 *begin;
    uint8 *end;
    uint8 *link;
    uint8 *handle;
    uint8 *handle_temp;

    begin = puc_cfg_buffer;
    if ((puc_cfg_buffer == NULL) || (puc_cmd_name == NULL) || (puc_cmd_para == NULL)) {
        PS_PRINT_ERR("para is NULL\n");
        return ERROR_TYPE_CMD;
    }

    /* 注释行 */
    if (puc_cfg_buffer[0] == '@') {
        return ERROR_TYPE_CMD;
    }

    /* 错误行，或者退出命令行 */
    link = OS_STR_CHR((int8 *)begin, '=');
    if (link == NULL) {
        /* 退出命令行 */
        if (NULL != OS_STR_STR((int8 *)puc_cfg_buffer, QUIT_CMD_KEYWORD)) {
            return QUIT_TYPE_CMD;
        } else if (NULL != OS_STR_STR((int8 *)puc_cfg_buffer, SHUTDOWN_WIFI_CMD_KEYWORD)) {
            return SHUTDOWN_WIFI_TYPE_CMD;
        } else if (NULL != OS_STR_STR((int8 *)puc_cfg_buffer, SHUTDOWN_BFGX_CMD_KEYWORD)) {
            return SHUTDOWN_BFGX_TYPE_CMD;
        }

        return ERROR_TYPE_CMD;
    }

    /* 错误行，没有结束符 */
    end = OS_STR_CHR(link, ';');
    if (end == NULL) {
        return ERROR_TYPE_CMD;
    }

    l_cmdlen = link - begin;

    /* 删除关键字的两边空格 */
    handle = delete_space((uint8 *)begin, &l_cmdlen);
    if (handle == NULL) {
        return ERROR_TYPE_CMD;
    }

    /* 判断命令类型 */
    if (!OS_MEM_CMP(handle, (uint8 *)FILE_TYPE_CMD_KEY, OS_STR_LEN((uint8 *)FILE_TYPE_CMD_KEY))) {
        handle_temp = OS_STR_STR(handle, (uint8 *)FILE_TYPE_CMD_KEY);
        if (handle_temp == NULL) {
            PS_PRINT_ERR("'ADDR_FILE_'is not handle child string, handle=%s", handle);
            return ERROR_TYPE_CMD;
        }
        handle = handle_temp + OS_STR_LEN(FILE_TYPE_CMD_KEY);
        l_cmdlen = l_cmdlen - OS_STR_LEN(FILE_TYPE_CMD_KEY);
        cmd_type = FILE_TYPE_CMD;
    } else if (!OS_MEM_CMP(handle, (uint8 *)NUM_TYPE_CMD_KEY, OS_STR_LEN(NUM_TYPE_CMD_KEY))) {
        handle_temp = OS_STR_STR(handle, (uint8 *)NUM_TYPE_CMD_KEY);
        if (handle_temp == NULL) {
            PS_PRINT_ERR("'PARA_' is not handle child string, handle=%s", handle);
            return ERROR_TYPE_CMD;
        }
        handle = handle_temp + OS_STR_LEN(NUM_TYPE_CMD_KEY);
        l_cmdlen = l_cmdlen - OS_STR_LEN(NUM_TYPE_CMD_KEY);
        cmd_type = NUM_TYPE_CMD;
    } else {
        return ERROR_TYPE_CMD;
    }

    l_ret = memcpy_s(puc_cmd_name, cmd_name_len, handle, l_cmdlen);
    if (l_ret != EOK) {
        PS_PRINT_ERR("cmd len out of range! ret = %d\n", l_ret);
        return ERROR_TYPE_CMD;
    }

    /* 删除值两边空格 */
    begin = link + 1;
    l_paralen = end - begin;

    handle = delete_space((uint8 *)begin, &l_paralen);
    if (handle == NULL) {
        return ERROR_TYPE_CMD;
    }
    l_ret = memcpy_s(puc_cmd_para, cmd_para_len, handle, l_paralen);
    if (l_ret != EOK) {
        PS_PRINT_ERR("para len out of range!ret = %d\n", l_ret);
        return ERROR_TYPE_CMD;
    }

    return cmd_type;
}

/*
 * 函 数 名  : firmware_parse_cfg
 * 功能描述  : 解析cfg文件，将解析的结果保存在g_st_cfg_info全局变量中
 * 输入参数  : puc_cfg_info_buf: 保存了cfg文件内容的buffer
 *             l_buf_len       : puc_cfg_info_buf的长度
 *             ul_index        : 保存解析结果的数组索引值
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32 firmware_parse_cfg(uint8 *puc_cfg_info_buf, int32 l_buf_len, uint32 ul_index)
{
    int32 i;
    int32 l_len;
    int32 l_ret;
    uint8 *flag;
    uint8 *begin;
    uint8 *end;
    int32 cmd_type;
    uint8 cmd_name[DOWNLOAD_CMD_LEN];
    uint8 cmd_para[DOWNLOAD_CMD_PARA_LEN];
    uint32 cmd_para_len = 0;
    if (puc_cfg_info_buf == NULL) {
        PS_PRINT_ERR("puc_cfg_info_buf is NULL!\n");
        return -EFAIL;
    }

    cfg_info.apst_cmd[ul_index] = (struct cmd_type_st *)malloc_cmd_buf(puc_cfg_info_buf, ul_index);
    if (cfg_info.apst_cmd[ul_index] == NULL) {
        PS_PRINT_ERR(" malloc_cmd_buf fail!\n");
        return -EFAIL;
    }

    /* 解析CMD BUF */
    flag = puc_cfg_info_buf;
    l_len = l_buf_len;
    i = 0;
    while ((i < cfg_info.al_count[ul_index]) && (flag < &puc_cfg_info_buf[l_len])) {
        /*
         * 获取配置文件中的一行,配置文件必须是unix格式.
         * 配置文件中的某一行含有字符 @ 则认为该行为注释行
         */
        begin = flag;
        end = OS_STR_CHR(flag, '\n');
        if (end == NULL) { /* 文件的最后一行，没有换行符 */
            PS_PRINT_DBG("lost of new line!\n");
            end = &puc_cfg_info_buf[l_len];
        } else if (end == begin) { /* 该行只有一个换行符 */
            PS_PRINT_DBG("blank line\n");
            flag = end + 1;
            continue;
        }
        *end = '\0';

        PS_PRINT_DBG("operation string is [%s]\n", begin);

        memset_s(cmd_name, sizeof(cmd_name), 0, DOWNLOAD_CMD_LEN);
        memset_s(cmd_para, sizeof(cmd_para), 0, DOWNLOAD_CMD_PARA_LEN);

        cmd_type = firmware_parse_cmd(begin, cmd_name, sizeof(cmd_name), cmd_para, sizeof(cmd_para));

        PS_PRINT_DBG("cmd type=[%d],cmd_name=[%s],cmd_para=[%s]\n", cmd_type, cmd_name, cmd_para);

        if (cmd_type != ERROR_TYPE_CMD) { /* 正确的命令类型，增加 */
            cfg_info.apst_cmd[ul_index][i].cmd_type = cmd_type;
            memcpy_s(cfg_info.apst_cmd[ul_index][i].cmd_name, DOWNLOAD_CMD_LEN, cmd_name, DOWNLOAD_CMD_LEN);
            memcpy_s(cfg_info.apst_cmd[ul_index][i].cmd_para, DOWNLOAD_CMD_PARA_LEN,
                     cmd_para, DOWNLOAD_CMD_PARA_LEN);
            /* 获取配置版本号 */
            if (!OS_MEM_CMP(cfg_info.apst_cmd[ul_index][i].cmd_name,
                            VER_CMD_KEYWORD,
                            OS_STR_LEN(VER_CMD_KEYWORD))) {
                cmd_para_len = OS_STR_LEN(cfg_info.apst_cmd[ul_index][i].cmd_para);

                l_ret = memcpy_s(cfg_info.auc_CfgVersion, sizeof(cfg_info.auc_CfgVersion),
                                 cfg_info.apst_cmd[ul_index][i].cmd_para, cmd_para_len);
                if (l_ret != EOK) {
                    PS_PRINT_ERR("cmd_para_len = %d over auc_CfgVersion length", cmd_para_len);
                    return -EFAIL;
                }
                PS_PRINT_DBG("g_CfgVersion = [%s].\n", cfg_info.auc_CfgVersion);
            } else if (!OS_MEM_CMP(cfg_info.apst_cmd[ul_index][i].cmd_name,
                                   CALI_DCXO_DATA_CMD_KEYWORD,
                                   OS_STR_LEN(CALI_DCXO_DATA_CMD_KEYWORD))) {
                if (!test_bit(DCXO_PARA_READ_OK, &dcxo_info.nv_init_flag)) {
                    l_ret = read_dcxo_cali_data();
                    if (l_ret < 0) {
                        PS_PRINT_ERR("read dcxo para from nv failed !\n");
                        return -EFAIL;
                    }
                    set_bit(DCXO_PARA_READ_OK, &dcxo_info.nv_init_flag);
                } else {
                    PS_PRINT_DBG("dcxo para has already initialized, skip...\n");
                }
            }
            i++;
        }
        flag = end + 1;
    }

    /* 根据实际命令个数，修改最终的命令个数 */
    cfg_info.al_count[ul_index] = i;
    PS_PRINT_INFO("effective cmd count: al_count[%d] = %d\n", ul_index, cfg_info.al_count[ul_index]);

    return SUCC;
}

/*
 * 函 数 名  : firmware_get_cfg
 * 功能描述  : 读取cfg文件并解析，将解析的结果保存在g_st_cfg_info全局变量中
 * 输入参数  : puc_CfgPatch: cfg文件的路径
 *             ul_index     : 保存解析结果的数组索引值
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32 firmware_get_cfg(uint8 *puc_CfgPatch, uint32 ul_index)
{
    uint8 *puc_read_cfg_buf;
    int32 l_readlen;
    int32 l_ret;

    if (puc_CfgPatch == NULL) {
        PS_PRINT_ERR("cfg file path is null!\n");
        return -EFAIL;
    }

    /* cfg文件限定在小于2048,如果cfg文件的大小确实大于2048，可以修改READ_CFG_BUF_LEN的值 */
    puc_read_cfg_buf = OS_KMALLOC_GFP(READ_CFG_BUF_LEN);
    if (puc_read_cfg_buf == NULL) {
        PS_PRINT_ERR("kmalloc READ_CFG_BUF fail!\n");
        return -EFAIL;
    }

    l_readlen = firmware_read_cfg(puc_CfgPatch, puc_read_cfg_buf);
    if (l_readlen < 0) {
        PS_PRINT_ERR("read cfg error!\n");
        OS_MEM_KFREE(puc_read_cfg_buf);
        puc_read_cfg_buf = NULL;
        return -EFAIL;
    }
    /* 减1是为了确保cfg文件的长度不超过READ_CFG_BUF_LEN，因为firmware_read_cfg最多只会读取READ_CFG_BUF_LEN长度的内容 */
    else if (l_readlen > READ_CFG_BUF_LEN - 1) {
        PS_PRINT_ERR("cfg file [%s] larger than %d\n", puc_CfgPatch, READ_CFG_BUF_LEN);
        OS_MEM_KFREE(puc_read_cfg_buf);
        puc_read_cfg_buf = NULL;
        return -EFAIL;
    } else {
        PS_PRINT_DBG("read cfg file [%s] ok, size is [%d]\n", puc_CfgPatch, l_readlen);
    }

    l_ret = firmware_parse_cfg(puc_read_cfg_buf, l_readlen, ul_index);
    if (l_ret < 0) {
        PS_PRINT_ERR("parse cfg error!\n");
    }

    OS_MEM_KFREE(puc_read_cfg_buf);
    puc_read_cfg_buf = NULL;

    return l_ret;
}

/*
 * 函 数 名  : firmware_download
 * 功能描述  : firmware加载
 * 输入参数  : ul_index: 有效加载命令数组的索引
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32 firmware_download(uint32 ul_index)
{
    int32 l_ret;
    int32 i;
    int32 l_cmd_type;
    uint8 *puc_cmd_name;
    uint8 *puc_cmd_para;
    hcc_bus *pst_bus;

    if (ul_index >= CFG_FILE_TOTAL) {
        PS_PRINT_ERR("ul_index [%d] is error!\n", ul_index);
        return -EFAIL;
    }

    store_efuse_info();

    PS_PRINT_INFO("start download firmware, ul_index = [%d]\n", ul_index);

    if (cfg_info.al_count[ul_index] == 0) {
        PS_PRINT_ERR("firmware download cmd count is 0, ul_index = [%d]\n", ul_index);
        return -EFAIL;
    }

    pst_bus = hcc_get_current_110x_bus();
    if (pst_bus == NULL) {
        PS_PRINT_ERR("firmware curr bus is null, ul_index = [%d]\n", ul_index);
        return -EFAIL;
    }

    firmware_down_buf = (uint8 *)oal_memtry_alloc(OAL_MIN(pst_bus->cap.max_trans_size, MAX_FIRMWARE_FILE_TX_BUF_LEN),
                                           MIN_FIRMWARE_FILE_TX_BUF_LEN, &firmware_down_buf_len);
    firmware_down_buf_len = OAL_ROUND_DOWN(firmware_down_buf_len, 8);  /* 清除低3bit，保证8字节对齐 */

    if (firmware_down_buf == NULL || (firmware_down_buf_len == 0)) {
        PS_PRINT_ERR("firmware_down_buf KMALLOC failed, min request:%u\n", MIN_FIRMWARE_FILE_TX_BUF_LEN);
        return -EFAIL;
    }

    PS_PRINT_INFO("download firmware file buf len is [%d]\n", firmware_down_buf_len);

    for (i = 0; i < cfg_info.al_count[ul_index]; i++) {
        l_cmd_type = cfg_info.apst_cmd[ul_index][i].cmd_type;
        puc_cmd_name = cfg_info.apst_cmd[ul_index][i].cmd_name;
        puc_cmd_para = cfg_info.apst_cmd[ul_index][i].cmd_para;

        PS_PRINT_DBG("cmd[%d]:type[%d], name[%s], para[%s]\n", i, l_cmd_type, puc_cmd_name, puc_cmd_para);

        PS_PRINT_DBG("firmware down start cmd[%d]:type[%d], name[%s]\n", i, l_cmd_type, puc_cmd_name);

        l_ret = execute_download_cmd(l_cmd_type, puc_cmd_name, puc_cmd_para);
        if (l_ret < 0) {
            OS_MEM_KFREE(firmware_down_buf);
            firmware_down_buf = NULL;
            PS_PRINT_ERR("download firmware fail\n");

            return l_ret;
        }

        PS_PRINT_DBG("firmware down finish cmd[%d]:type[%d], name[%s]\n", i, l_cmd_type, puc_cmd_name);
    }

    OS_MEM_KFREE(firmware_down_buf);
    firmware_down_buf = NULL;

    PS_PRINT_INFO("finish download firmware\n");

    return SUCC;
}

int32 print_firmware_download_cmd(uint32 ul_index)
{
    int32 i;
    int32 l_cmd_type;
    uint8 *puc_cmd_name;
    uint8 *puc_cmd_para;
    uint32 count;

    count = cfg_info.al_count[ul_index];
    PS_PRINT_INFO("[%s] download cmd, total count is [%d]\n", cfg_path[ul_index], count);

    for (i = 0; i < count; i++) {
        l_cmd_type = cfg_info.apst_cmd[ul_index][i].cmd_type;
        puc_cmd_name = cfg_info.apst_cmd[ul_index][i].cmd_name;
        puc_cmd_para = cfg_info.apst_cmd[ul_index][i].cmd_para;

        PS_PRINT_INFO("cmd[%d]:type[%d], name[%s], para[%s]\n", i, l_cmd_type, puc_cmd_name, puc_cmd_para);
    }

    return 0;
}

int32 print_cfg_file_cmd(void)
{
    int32 i;

    for (i = 0; i < CFG_FILE_TOTAL; i++) {
        print_firmware_download_cmd(i);
    }

    return 0;
}

/*
 * 函 数 名  : firmware_cfg_path_init
 * 功能描述  : 获取firmware的cfg文件路径
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32 firmware_cfg_path_init(void)
{
    int32 l_ret;
    int32 l_len;
    uint8 rec_buf[VERSION_LEN];

    if (BOARD_VERSION_HI1103 == get_hi110x_subchip_type()) {
        memset_s(rec_buf, sizeof(rec_buf), 0, VERSION_LEN);

        l_ret = memcpy_s(rec_buf, sizeof(rec_buf), (uint8 *)VER_CMD_KEYWORD, OS_STR_LEN(VER_CMD_KEYWORD));
        if (l_ret != EOK) {
            PS_PRINT_ERR("memcpy_s failed,l_ret = %d", l_ret);
            return -EFAIL;
        }

        l_len = OS_STR_LEN(VER_CMD_KEYWORD);

        rec_buf[l_len] = COMPART_KEYWORD;
        l_len++;

        l_ret = send_msg(rec_buf, l_len);
        if (l_ret < 0) {
            PS_PRINT_ERR("Hi1103 send version cmd fail!\n");
            return -EFAIL;
        }

        msleep(1);

        l_ret = read_msg(rec_buf, VERSION_LEN);
        if (l_ret < 0) {
            PS_PRINT_ERR("Hi1103 read version fail!\n");
            return -EFAIL;
        }

        PS_PRINT_INFO("Hi1103 Device Version=[%s].\n", rec_buf);

        if (!OS_MEM_CMP((int8 *)rec_buf, (int8 *)HI1103_MPW2_BOOTLOADER_VERSION,
                        OS_STR_LEN(HI1103_MPW2_BOOTLOADER_VERSION))) {
            cfg_path = mpw2_cfg_patch_in_vendor;
            set_hi1103_asic_type(HI1103_ASIC_MPW2);
            return SUCC;
        } else if (!OS_MEM_CMP((int8 *)rec_buf, (int8 *)HI1103_PILOT_BOOTLOADER_VERSION,
                               OS_STR_LEN(HI1103_PILOT_BOOTLOADER_VERSION))) {
            cfg_path = pilot_cfg_patch_in_vendor;
            set_hi1103_asic_type(HI1103_ASIC_PILOT);
            return SUCC;
        } else {
            PS_PRINT_WARNING("Hi1103 Device Version Error!\n");
            cfg_path = pilot_cfg_patch_in_vendor;
            set_hi1103_asic_type(HI1103_ASIC_PILOT);
            return SUCC;
        }
    } else if (BOARD_VERSION_HI1102A == get_hi110x_subchip_type()) {
        PS_PRINT_INFO("subchip type is hi1102a.\n");
        cfg_path = auc_1102a_cfg_patch_in_vendor;
        set_hi1103_asic_type(HI1103_ASIC_PILOT);
        return SUCC;
    }

    return SUCC;
}

/*
 * 函 数 名  : firmware_cfg_init
 * 功能描述  : firmware加载的cfg文件初始化，读取并解析cfg文件，将解析的结果保存在
 *             g_st_cfg_info全局变量中
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32 firmware_cfg_init(void)
{
    int32 l_ret;
    uint32 i;

    l_ret = firmware_cfg_path_init();
    if (l_ret != SUCC) {
        PS_PRINT_ERR("firmware cfg path init fail!");
        return -EFAIL;
    }

    /* 申请由于保存共时钟校准数据的buffer */
    l_ret = dcxo_data_buf_malloc();
    if (l_ret < 0) {
        PS_PRINT_ERR("alloc dcxo data buf fail\n");
        goto alloc_dcxo_data_buf_fail;
    }

    /* 解析cfg文件 */
    for (i = 0; i < CFG_FILE_TOTAL; i++) {
        l_ret = firmware_get_cfg(cfg_path[i], i);
        if (l_ret < 0) {
            if ((i == RAM_REG_TEST_CFG) || (i == RAM_BCPU_REG_TEST_CFG) ||
                (i == BFGX_AND_HITALK_CFG) || (i == HITALK_CFG)) {
                PS_PRINT_WARNING("ram_reg_test_cfg or ram_bcpu_reg_test_cfg or hitalk maybe not exist, please check\n");
                continue;
            }

            PS_PRINT_ERR("get cfg file [%s] fail\n", cfg_path[i]);
            goto cfg_file_init_fail;
        }
    }

    return SUCC;

cfg_file_init_fail:
alloc_dcxo_data_buf_fail:

    firmware_cfg_clear();

    return -EFAIL;
}

/*
 * 函 数 名  : firmware_cfg_clear
 * 功能描述  : 释放firmware_cfg_init时申请的内存
 * 返 回 值  : 总是返回0，表示成功
 */
int32 firmware_cfg_clear(void)
{
    int32 i;

    for (i = 0; i < CFG_FILE_TOTAL; i++) {
        cfg_info.al_count[i] = 0;
        if (cfg_info.apst_cmd[i] != NULL) {
            OS_MEM_KFREE(cfg_info.apst_cmd[i]);
            cfg_info.apst_cmd[i] = NULL;
        }
    }

    dcxo_data_buf_free();

    return SUCC;
}

/*
 * 函 数 名  : nfc_buffer_data_recv
 * 功能描述  : 保存nfc buffer数据
 */
int32 nfc_buffer_data_recv(uint8 *tmp_pucDataBuf, int32 len)
{
    uint32 l_ret = 0;
    int32 lenbuf = 0;
    int32 retry = 3;

    if (tmp_pucDataBuf == NULL) {
        PS_PRINT_ERR("tmp_pucDataBuf is NULL\n");
        return -EFAIL;
    }

    // 接收数据
    while (len > lenbuf) {
        l_ret = read_msg(tmp_pucDataBuf + lenbuf, len - lenbuf);
        if (l_ret > 0) {
            lenbuf += l_ret;
        } else {
            retry--;
            lenbuf = 0;
            if (retry == 0) {
                PS_PRINT_ERR("time out\n");
                return -EFAIL;
            }
        }
    }
    return SUCC;
}

/*
 * 函 数 名  : save_nfc_lowpower_log_2_sdt
 * 功能描述  : 开机拷贝nfc buffer内存
 */
void save_nfc_lowpower_log_2_sdt(void)
{
    uint32 cp_len = 0;
    uint32 index = 0;
    uint8 buf_tx[SEND_BUF_LEN];
    uint32 total_len;
    int32 buf_tx_len = 0;
    hcc_bus *pst_bus;
    hcc_bus_dev *pst_bus_dev;
    uint8 uc_retry_times = 3;
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (NULL == pm_data) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return;
    }
    PS_PRINT_INFO("save nfc low-power log begin\n");
    total_len = nfc_buffer_data.size;

    pst_bus_dev = hcc_get_bus_dev(HCC_CHIP_110X_DEV);
    if (pst_bus_dev == NULL) {
        PS_PRINT_ERR("110x dev is null\n");
        return;
    }

    if (NULL == HDEV_TO_HBUS(pst_bus_dev)) {
        PS_PRINT_ERR("110x bus is null\n");
        return;
    }

    pst_bus = HDEV_TO_HBUS(pst_bus_dev);
    hcc_bus_lock(pst_bus);

    pucNfcLog = kmalloc(NFC_SEND_LEN_LIMIT, GFP_ATOMIC);
    if (pucNfcLog == NULL) {
        PS_PRINT_ERR("nfc log buffer malloc fail!\n");
        hcc_bus_unlock(pst_bus);
        return;
    }

    while (index < total_len) {
        cp_len = total_len - index;
        if (cp_len > NFC_SEND_LEN_LIMIT) {
            cp_len = NFC_SEND_LEN_LIMIT;
        }
        buf_tx_len = snprintf_s(buf_tx, sizeof(buf_tx), sizeof(buf_tx) - 1, "%s%c0x%lx%c%d%c",
                                RMEM_CMD_KEYWORD,
                                COMPART_KEYWORD,
                                nfc_buffer_data.mem_addr + index,
                                COMPART_KEYWORD,
                                cp_len,
                                COMPART_KEYWORD);
        if (buf_tx_len < 0) {
            PS_PRINT_ERR("NFC SEND CMD format failed\n");
            hcc_bus_unlock(pst_bus);
            kfree(pucNfcLog);
            pucNfcLog = NULL;
            return;
        }
        PS_PRINT_INFO("read nfc buffer cmd:[%s]\n", buf_tx);

        /* 需要增加delay时间，该时间和SDIO读取的长度相关 */
        send_msg(buf_tx, buf_tx_len);

        if (SUCC != nfc_buffer_data_recv(pucNfcLog, cp_len)) {
            PS_PRINT_ERR("recv data error!\n");
            hcc_bus_unlock(pst_bus);
            kfree(pucNfcLog);
            pucNfcLog = NULL;
            return;
        }

        /* 因为nfc属于bfgn，log走bfgn通道 */
        while (!wifi_choose_bfgn_channel_send_log2sdt(pucNfcLog, cp_len)) {
            uc_retry_times--;
            if (uc_retry_times == 0) {
                PS_PRINT_ERR("send log to bfgn log channels error\n");
                break;
            }
        }

        index += cp_len;
    }

    kfree(pucNfcLog);
    pucNfcLog = NULL;

    hcc_bus_unlock(pst_bus);
    PS_PRINT_INFO("save nfc low-power log sucess\n");
}

#define DEVICE_MEM_CHECK_SUCC 0x000f
#define GET_MEM_CHECK_FLAG    "0x50000018,4"
int32 is_device_mem_test_succ(void)
{
    int32 ret;
    int32 test_flag = 0;

    ret = number_type_cmd_send(RMEM_CMD_KEYWORD, GET_MEM_CHECK_FLAG);
    if (ret < 0) {
        PS_PRINT_WARNING("send cmd %s:%s fail,ret = %d\n", RMEM_CMD_KEYWORD, GET_MEM_CHECK_FLAG, ret);
        return -1;
    }

    ret = read_msg((uint8 *)&test_flag, sizeof(test_flag));
    if (ret < 0) {
        PS_PRINT_WARNING("read device test flag fail, read_len = %d, return = %d\n", (int32)sizeof(test_flag), ret);
        return -1;
    }
    PS_PRINT_WARNING("get device test flag:0x%x\n", test_flag);
    if (DEVICE_MEM_CHECK_SUCC == test_flag) {
        return 0;
    }
    return -1;
}
int32 get_device_test_mem(uint8 *file_name)
{
    wlan_memdump_t *wlan_memdump_s = NULL;
    const uint32 ul_buff_len = 100;
    uint8 buff[ul_buff_len];
    int32 ret;

    if (file_name == NULL) {
        PS_PRINT_ERR("reg mem test file_name is NULL!\n");
        return -EINVAL;
    }

    wlan_memdump_s = get_wlan_memdump_cfg();
    if (!wlan_memdump_s) {
        PS_PRINT_ERR("memdump cfg is NULL!\n");
        return -FAILURE;
    }
    ret = snprintf_s(buff, sizeof(buff), sizeof(buff) - 1, "0x%x,%d", wlan_memdump_s->addr, wlan_memdump_s->len);
    if (ret < 0) {
        PS_PRINT_WARNING("RMEM_CMD_KEYWORD format failed\n");
        return -FAILURE;
    }
    if (sdio_read_path_mem(RMEM_CMD_KEYWORD, buff, file_name) >= 0) {
        PS_PRINT_WARNING("read device mem succ\n");
    } else {
        PS_PRINT_WARNING("read device mem fail\n");
        return -FAILURE;
    }
    return 0;
}

