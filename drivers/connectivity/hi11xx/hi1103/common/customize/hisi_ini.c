

/* 头文件包含 */
#include <linux/version.h>
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
#define HISI_NVRAM_SUPPORT
#define HISI_DTS_SUPPORT
#endif

#include <linux/module.h>
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#include <linux/kernel.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#endif
#endif
#include <linux/time.h>
#include <linux/fs.h>

#ifdef HISI_NVRAM_SUPPORT
#include <linux/mtd/hisi_nve_interface.h>
#endif

#include "hisi_ini.h"
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#include "board.h"
#endif
#include "oal_schedule.h"
#ifdef HISI_DTS_SUPPORT
#include "board.h"
#endif

#include "oal_util.h"
#include "securec.h"

/* 全局变量定义 */
#define CUST_PATH_INI_CONN "/data/vendor/cust_conn/ini_cfg" /* 某运营商在不同产品的差异配置 */
/* mutex for open ini file */
struct mutex file_mutex_etc;
#if (_PRE_TARGET_PRODUCT_TYPE_E5 == _PRE_CONFIG_TARGET_PRODUCT)
int8 ini_file_name_etc[INI_FILE_PATH_LEN] = "/system/bin/wifi_hisi/cfg_e5_hisi.ini";
#elif (_PRE_TARGET_PRODUCT_TYPE_ONT == _PRE_CONFIG_TARGET_PRODUCT)
int8 ini_file_name_etc[INI_FILE_PATH_LEN] = "/var/cfg_ont_hisi.ini";
#elif (_PRE_TARGET_PRODUCT_TYPE_5630HERA == _PRE_CONFIG_TARGET_PRODUCT)
int8 ini_file_name_etc[INI_FILE_PATH_LEN] = "/etc/cfg_hera_hisi.ini";
#else
int8 ini_file_name_etc[INI_FILE_PATH_LEN] = "/system/bin/wifi_hisi/cfg_e5_hisi.ini";
#endif
int8 ini_conn_file_name_etc[INI_FILE_PATH_LEN] = {0};
#define INI_FILE_PATH (ini_file_name_etc)

INI_BOARD_VERSION_STRU board_version_etc = {{0}};
INI_PARAM_VERSION_STRU param_version_etc = {{0}};

int64 ini_file_time_sec = -1;

/*
 *  Prototype    : ko_read_line
 *  Description  : read one non-empty line .ini file
 */
static int32 ko_read_line(INI_FILE *fp, char *addr, int32 buf_len)
{
    int32 l_ret;
    int8 auc_tmp[MAX_READ_LINE_NUM] = {0};
    int32 cnt;
    int32 cnt_limit;

    l_ret = oal_file_read_ext(fp, fp->f_pos, auc_tmp, MAX_READ_LINE_NUM);

    if (l_ret < 0) {
        INI_ERROR("kernel_line read l_ret < 0");
        return INI_FAILED;
    } else if (l_ret == 0) {
        /* end of file */
        return 0;
    }

    cnt = 0;
    /* -2预留换行符和结束符 */
    cnt_limit = (buf_len - 2) > MAX_READ_LINE_NUM ? MAX_READ_LINE_NUM : (buf_len - 2);
    while ((cnt < cnt_limit) && (auc_tmp[cnt] != '\n')) {
        *addr++ = auc_tmp[cnt++];
    }
    *addr++ = '\n';
    *addr = '\0';

    /* change file pos to next line */
    fp->f_pos += (cnt + 1);

    return l_ret;
}

static INI_FILE *ini_file_open(int8 *filename, int8 *para)
{
    INI_FILE *fp = NULL;
    mm_segment_t fs;

    UNREF_PARAM(para);

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = (INI_FILE *)filp_open(filename, O_RDONLY, 0);
    set_fs(fs);
    if (OAL_IS_ERR_OR_NULL(fp)) {
        fp = NULL;
    }

    return fp;
}

static int32 ini_file_close(INI_FILE *fp)
{
    mm_segment_t fs;

    fs = get_fs();
    set_fs(KERNEL_DS);
    filp_close(fp, NULL);
    set_fs(fs);
    fp = NULL;
    return INI_SUCC;
}

static bool ini_file_exist(int8 *file_path)
{
    INI_FILE *fp = NULL;

    if (file_path == NULL) {
        INI_ERROR("para file_path is NULL\n");
        return false;
    }

    fp = ini_file_open(file_path, "rt");
    if (fp == NULL) {
        INI_DEBUG("%s not exist\n", file_path);
        return false;
    }

    ini_file_close(fp);

    INI_DEBUG("%s exist\n", file_path);

    return true;
}

/*
 *  Prototype    : ini_file_seek
 *  Description  : set f_pos in curr file ptr
 */
static int32 ini_file_seek(INI_FILE *fp, long fp_pos)
{
    fp->f_pos += fp_pos;
    return INI_SUCC;
}

static int32 ini_readline_func(INI_FILE *fp, int8 *rd_buf, uint32 buf_len)
{
    int8 auc_tmp[MAX_READ_LINE_NUM];
    int32 ret;

    memset_s(auc_tmp, sizeof(auc_tmp), 0, sizeof(auc_tmp));
    ret = ko_read_line(fp, auc_tmp, sizeof(auc_tmp));
    if (ret == INI_FAILED) {
        INI_ERROR("ko_read_line failed!!!");
        return INI_FAILED;
    } else if (ret == 0) {
        INI_ERROR("end of .ini file!!!");
        return INI_FAILED;
    }

    ret = strcpy_s(rd_buf, buf_len, auc_tmp);
    if (ret != EOK) {
        INI_ERROR("ini_readline_func: strcpy_s failed.");
        return INI_FAILED;
    }

    return INI_SUCC;
}

int32 ini_check_str_etc(INI_FILE *fp, int8 *auc_tmp, uint32 buf_len, const char *puc_var)
{
    uint16 auc_len;
    uint16 curr_var_len;
    uint16 search_var_len;

    if ((fp == NULL) || (puc_var == NULL) || (puc_var[0] == '\0')) {
        INI_ERROR("check if puc_var is NULL or blank");
        return INI_FAILED;
    }

    do {
        auc_len = (uint16)strlen(auc_tmp);
        curr_var_len = 0;

        while ((curr_var_len < buf_len) && (auc_tmp[curr_var_len] != '\r') &&
               (auc_tmp[curr_var_len] != '\n') && (auc_tmp[curr_var_len] != 0)) {
            curr_var_len++;
        }

        if ((auc_tmp[0] == '#') || (auc_tmp[0] == ' ') || (auc_tmp[0] == '\n') || (auc_tmp[0] == '\r')) {
            break;
        }
        search_var_len = (uint16)strlen(puc_var);
        if (search_var_len > curr_var_len) {
            break;
        }
        if (strncmp(auc_tmp, puc_var, search_var_len) == 0) {
            return INI_SUCC;
        } else {
            break;
        }
    } while (0);

    if (ini_file_seek(fp, -auc_len) == INI_FAILED) {
        INI_ERROR("file seek failed!!!");
        return INI_FAILED;
    }
    if (ini_file_seek(fp, curr_var_len + 1) == INI_FAILED) {
        INI_ERROR("file seek failed!!!");
        return INI_FAILED;
    }
    if (((curr_var_len + 1) < buf_len) && (auc_tmp[curr_var_len + 1] == '\n')) {
        if (ini_file_seek(fp, 1) == INI_FAILED) {
            INI_ERROR("file seek failed!!!");
            return INI_FAILED;
        }
    }

    return INI_FAILED;
}

/*
 *  Prototype    : ini_check_value
 *  Description  : check the value of config behind =
 */
static int32 ini_check_value(int8 *puc_value, uint32 value_len)
{
    uint32 cnt;
    const uint32 ul_value_min_len = 2;

    if (value_len < ul_value_min_len) {
        INI_ERROR("ini_check_value fail, puc_value length %u < 2(min len)\n", value_len);
        return INI_FAILED;
    }

    if (puc_value[0] == ' ' || puc_value[0] == '\r' || puc_value[0] == '\n' || puc_value[value_len - 1] == ' ') {
        puc_value[0] = '\0';
        INI_ERROR("::%s has blank space or is blank::", puc_value);
        return INI_FAILED;
    }

    /* 替换尾部空格和换行符为'\0' */
    cnt = value_len - 1;
    while ((cnt != 0) && (puc_value[cnt] == '\n' || puc_value[cnt] == '\r' || puc_value[cnt] == ' ')) {
        puc_value[cnt--] = '\0';
    }

    return INI_SUCC;
}

static int32 is_modu_exist(INI_FILE *fp, const char *modu_name)
{
    int32 ret = INI_FAILED;
    int8 auc_tmp[MAX_READ_LINE_NUM] = {0};
    /* find the value of mode var, such as ini_wifi_mode
     * every mode except PLAT mode has only one mode var */
    for (;;) {
        ret = ini_readline_func(fp, auc_tmp, sizeof(auc_tmp));
        if (ret == INI_FAILED) {
            INI_ERROR("have end of .ini file!!!");
            return INI_FAILED;
        }

        if (strstr(auc_tmp, INI_STR_DEVICE_BFG_PLAT) != NULL) {
            INI_ERROR("not find %s!!!", modu_name);
            return INI_FAILED;
        }

        ret = ini_check_str_etc(fp, auc_tmp, sizeof(auc_tmp), modu_name);
        if (ret == INI_SUCC) {
            INI_DEBUG("have found %s", modu_name);
            break;
        } else {
            continue;
        }
    }

    return ret;
}

/*
 *  Prototype    : ini_find_modu
 *  Description  : find moduler by mode value puc_value
 *
 */
static int32 ini_find_modu(INI_FILE *fp, int32 tag_index, int8 *puc_var, int8 *puc_value)
{
    int8 auc_modu[INI_STR_MODU_LEN] = {0};
    int32 ret = INI_FAILED;

    UNREF_PARAM(puc_var);
    UNREF_PARAM(puc_value);

    switch (tag_index) {
        case INI_MODU_WIFI:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_WIFI_NORMAL);
            break;
        case INI_MODU_POWER_FCC:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_POWER_FCC);
            break;
        case INI_MODU_POWER_ETSI:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_POWER_ETSI);
            break;
        case INI_MODU_POWER_JP:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_POWER_JP);
            break;
        case INI_MODU_GNSS:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_GNSS_NORMAL);
            break;
        case INI_MODU_BT:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_BT_NORMAL);
            break;
        case INI_MODU_FM:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_FM_NORMAL);
            break;
        case INI_MODU_PLAT:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_PLAT);
            break;
        case INI_MODU_HOST_VERSION:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INT_STR_HOST_VERSION);
            break;
        case INI_MODU_WIFI_MAC:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_WIFI_MAC);
            break;
        case INI_MODU_COEXIST:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_COEXIST);
            break;
        case INI_MODU_DEV_WIFI:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_DEVICE_WIFI);
            break;
        case INI_MODU_DEV_GNSS:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_DEVICE_GNSS);
            break;
        case INI_MODU_DEV_BT:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_DEVICE_BT);
            break;
        case INI_MODU_DEV_FM:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_DEVICE_FM);
            break;
        case INI_MODU_DEV_BFG_PLAT:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_DEVICE_BFG_PLAT);
            break;
        default:
            INI_ERROR("not suport tag type:%x!!!", tag_index);
            break;
    }
    if (ret != EOK) {
        INI_ERROR("find failed tag type:%x!!!", tag_index);
        return INI_FAILED;
    }

    return is_modu_exist(fp, auc_modu);
}

/*
 *  Prototype    : ini_find_var
 *  Description  : find difference mode variable value, and return puc_value
 */
static int32 ini_find_var(INI_FILE *fp, int32 tag_index, int8 *puc_var, int8 *puc_value, uint32 size)
{
    int32 ret;
    int8 auc_tmp[MAX_READ_LINE_NUM + 1] = {0};
    size_t search_var_len;

    /* find the modu of var, such as [HOST_WIFI_NORMAL] of wifi moduler */
    ret = ini_find_modu(fp, tag_index, puc_var, puc_value);

    if (ret == INI_FAILED) {
        return INI_FAILED;
    }

    /* find the var in modu, such as [HOST_WIFI_NORMAL] of wifi moduler */
    for (;;) {
        ret = ini_readline_func(fp, auc_tmp, sizeof(auc_tmp));
        if (ret == INI_FAILED) {
            INI_ERROR("have end of .ini file!!!");
            return INI_FAILED;
        }

        if (auc_tmp[0] == '[') {
#ifndef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
            INI_ERROR("not find %s!!!, check if var in correct mode", puc_var);
#endif
            return INI_FAILED;
        }

        search_var_len = strlen(puc_var);
        ret = ini_check_str_etc(fp, auc_tmp, sizeof(auc_tmp), puc_var);
        if ((ret == INI_SUCC) && (auc_tmp[search_var_len] == '=')) {
            if (strncpy_s(puc_value, size, &auc_tmp[search_var_len + 1], size - 1)) {
                INI_ERROR("space is not enough");
                continue;
            }
            break;
        } else {
            continue;
        }
    }

    return INI_SUCC;
}

int32 find_download_channel_etc(uint8 *buff, uint32 buf_len, int8 *puc_var)
{
    INI_FILE *fp = NULL;
    int8 version_buff[DOWNLOAD_CHANNEL_LEN] = {0};
    int32 l_ret;

    INI_MUTEX_LOCK(&file_mutex_etc);
    INI_INFO("ini file_name is %s", INI_FILE_PATH);
    fp = ini_file_open(INI_FILE_PATH, "rt");
    if (fp == 0) {
        fp = NULL;
        INI_ERROR("open %s failed!!!", INI_FILE_PATH);
        goto open_ini_file_fail;
    }

    /* find wlan download channel */
    l_ret = ini_find_var(fp, INI_MODU_PLAT, puc_var, version_buff, DOWNLOAD_CHANNEL_LEN);
    if (l_ret == INI_FAILED) {
        version_buff[0] = '\0';
        goto read_ini_var_fail;
    }
    if (ini_check_value(version_buff, strlen(version_buff)) == INI_FAILED) {
        goto read_ini_var_fail;
    }
    if (strcpy_s((int8 *)buff, buf_len, version_buff) != EOK) {
        goto read_ini_var_fail;
    }
    buff[buf_len - 1] = '\0';

    fp->f_pos = 0;
    ini_file_close(fp);
    INI_MUTEX_UNLOCK(&file_mutex_etc);
    return INI_SUCC;

read_ini_var_fail:
    fp->f_pos = 0;
    ini_file_close(fp);
open_ini_file_fail:
    INI_MUTEX_UNLOCK(&file_mutex_etc);
    return INI_FAILED;
}

int32 ini_find_var_value_by_path_etc(int8 *path, int32 tag_index, int8 *puc_var, int8 *puc_value, uint32 size)
{
    INI_FILE *fp = NULL;

#ifdef INI_TIME_TEST
    struct timeval tv0;
    struct timeval tv1;
#endif

    int32 l_ret;

    if (puc_var == NULL || puc_var[0] == '\0' || puc_value == NULL) {
        INI_ERROR("check if puc_var and puc_value is NULL or blank");
        return INI_FAILED;
    }

#ifdef INI_TIME_TEST
    do_gettimeofday(&tv0);
#endif

    INI_MUTEX_LOCK(&file_mutex_etc);

    fp = ini_file_open(path, "rt");
    if (fp == 0) {
        INI_ERROR("open %s failed!!!", path);
        INI_MUTEX_UNLOCK(&file_mutex_etc);
        return INI_FAILED;
    }

    /* find puc_var in .ini return puc_value */
    l_ret = ini_find_var(fp, tag_index, puc_var, puc_value, size);
    if (l_ret == INI_FAILED) {
        puc_value[0] = '\0';
        ini_file_close(fp);
        INI_MUTEX_UNLOCK(&file_mutex_etc);
        return INI_FAILED;
    }

#ifdef INI_TIME_TEST
    do_gettimeofday(&tv1);
    INI_DEBUG("time take = %ld", (tv1.tv_sec - tv0.tv_sec) * 1000 + (tv1.tv_usec - tv0.tv_usec) / 1000);
#endif

    ini_file_close(fp);
    INI_MUTEX_UNLOCK(&file_mutex_etc);

    /* check blank space of puc_value */
    if (ini_check_value(puc_value, strlen(puc_value)) == INI_SUCC) {
        INI_DEBUG("::return %s:%s::", puc_var, puc_value);
        return INI_SUCC;
    }

    return INI_FAILED;
}

/*
 *  Prototype    : ini_find_var_value_etc
 *  Description  : get var value from .ini file
 */
int32 ini_find_var_value_etc(int32 tag_index, int8 *puc_var, int8 *puc_value, uint32 size)
{
    /* read spec if exist */
    if (ini_file_exist(ini_conn_file_name_etc)) {
        if (ini_find_var_value_by_path_etc(ini_conn_file_name_etc, tag_index, puc_var, puc_value, size) == INI_SUCC) {
            return INI_SUCC;
        }
    }

    if (ini_file_exist(INI_FILE_PATH) == 0) {
        INI_ERROR(" %s not exist!!!", INI_FILE_PATH);
        return INI_FAILED;
    }

    return ini_find_var_value_by_path_etc(INI_FILE_PATH, tag_index, puc_var, puc_value, size);
}

#ifdef HISI_NVRAM_SUPPORT
/*
 *  Prototype    : read_conf_from_nvram_etc
 *  Description  : read nv buff from nvram
 *
 */
int32 read_conf_from_nvram_etc(uint8 *pc_out, uint32 size, uint32 nv_number, const char *nv_name)
{
    struct hisi_nve_info_user info;
    int32 ret;

    memset_s(&info, sizeof(info), 0, sizeof(info));
    memset_s(pc_out, size, 0, size);
    if (strcpy_s(info.nv_name, sizeof(info.nv_name), nv_name) != EOK) {
        INI_ERROR("read nvm failed nv_name size[%lu] less than input[%s]", sizeof(info.nv_name), nv_name);
        return INI_FAILED;
    }
    info.nv_name[strlen(HISI_CUST_NVRAM_NAME)] = '\0';
    info.nv_number = nv_number;
    info.valid_size = HISI_CUST_NVRAM_LEN;
    info.nv_operation = HISI_CUST_NVRAM_READ;

    ret = hisi_nve_direct_access(&info);
    if (size > sizeof(info.nv_data) || size <= OAL_STRLEN(info.nv_data)) {
        INI_ERROR("read nvm item[%s] fail, lenth[%d] longer than input[%d]", nv_name, (uint32)OAL_STRLEN(info.nv_data), size);
        return INI_FAILED;
    }
    if (ret == INI_SUCC) {
        if (memcpy_s(pc_out, size, info.nv_data, sizeof(info.nv_data)) != EOK) {
            INI_ERROR("read nvm{%s}lenth[%d] longer than input[%d]",
                      info.nv_data, (uint32)OAL_STRLEN(info.nv_data), size);
            return INI_FAILED;
        }
        OAL_IO_PRINT("read_conf_from_nvram_etc::nvram id[%d] nv name[%s] get data{%s}, size[%d]\r\n!",
                     nv_number, nv_name, info.nv_data, size);
    } else {
        INI_ERROR("read nvm [%d] %s failed", nv_number, nv_name);
        return INI_FAILED;
    }

    return INI_SUCC;
}

/*
 *  Prototype    : write_conf_to_nvram_etc
 *  Description  : change value and write to file
 */
int32 write_conf_to_nvram_etc(int8 *name, int8 *pc_arr)
{
    struct hisi_nve_info_user info;
    int32 ret;

    UNREF_PARAM(name);

    memset_s(&info, sizeof(info), 0, sizeof(info));
    if (strcpy_s(info.nv_name, sizeof(info.nv_name), HISI_CUST_NVRAM_NAME) != EOK) {
        INI_ERROR("write_conf_to_nvram failed nv_name size[%lu] less than input[%s]",
                  sizeof(info.nv_name), HISI_CUST_NVRAM_NAME);
        return INI_FAILED;
    }
    info.nv_name[sizeof(info.nv_name) - 1] = '\0';
    info.nv_number = HISI_CUST_NVRAM_NUM;
    info.valid_size = HISI_CUST_NVRAM_LEN;
    info.nv_operation = HISI_CUST_NVRAM_WRITE;
    if (memcpy_s(info.nv_data, sizeof(info.nv_data), pc_arr, HISI_CUST_NVRAM_LEN) != EOK) {
        INI_ERROR("write nvm[memcpy_s] failed");
        return INI_FAILED;
    }

    ret = hisi_nve_direct_access(&info);
    if (ret < -1) {
        INI_ERROR("write nvm failed");
        return INI_FAILED;
    }

    return INI_SUCC;
}
#endif

/*
 *  Prototype    : get_cust_conf_string_etc
 *  Description  : get config form *.ini file or dts(kernel)
 */
int32 get_cust_conf_string_etc(int32 tag_index, int8 *puc_var, int8 *puc_value, uint32 size)
{
    memset_s(puc_value, size, 0, size);
    return ini_find_var_value_etc(tag_index, puc_var, puc_value, size);
}

int32 set_cust_conf_string_etc(int32 tag_index, int8 *name, int8 *var)
{
    int32 ret = INI_FAILED;

    if (tag_index != CUST_MODU_NVRAM) {
        INI_ERROR("NOT SUPPORT MODU TO WRITE");
        return INI_FAILED;
    }
#ifdef HISI_NVRAM_SUPPORT
    ret = write_conf_to_nvram_etc(name, var);
#endif
    return ret;
}

/*
 *  Prototype    : get_cust_conf_int32_etc
 *  Description  : get config form *.ini file or dts(kernel)
 */

int32 get_cust_conf_int32_etc(int32 tag_index, int8 *puc_var, int32 *puc_value)
{
    int32 ret;
    int8 out_str[INI_READ_VALUE_LEN] = {0};

    ret = ini_find_var_value_etc(tag_index, puc_var, out_str, sizeof(out_str));

    if (ret < 0) {
        /* ini_find_var_value_etc has error log, delete this log */
        INI_DEBUG("cust modu didn't get var of %s.", puc_var);
        return INI_FAILED;
    }

    if (!strncmp(out_str, "0x", strlen("0x")) || !strncmp(out_str, "0X", strlen("0X"))) {
        INI_DEBUG("get hex of:%s.", puc_var);
        ret = sscanf(out_str, "%x", puc_value);
    } else {
        ret = sscanf(out_str, "%d", puc_value);
    }

    if (ret < 0) {
        INI_ERROR("%s trans to int failed", puc_var);
        return INI_FAILED;
    }

    INI_DEBUG("conf %s get vale:%d", puc_var, *puc_value);

    return INI_SUCC;
}

static int32 get_ini_file(int8 *file_path, INI_FILE **fp)
{
    if (file_path == NULL) {
        INI_INFO("para file_path is NULL\n");
        return INI_FAILED;
    }

    *fp = ini_file_open(file_path, "rt");
    if (*fp == NULL) {
        INI_INFO("inifile %s not exist\n", file_path);
        return INI_FAILED;
    }

    return INI_SUCC;
}

/*
 *  Prototype    : ini_file_check_timespec
 *  Description  : get *.ini file timespec
 */
static int32 ini_file_check_timespec(INI_FILE *fp)
{
    if (fp == NULL) {
        INI_ERROR("para file is NULL\n");
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (fp->f_path.dentry == NULL) {
        INI_ERROR("file dentry is NULL\n");
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (ini_file_time_sec != INF_FILE_GET_CTIME(fp->f_path.dentry)) {
        INI_INFO("ini_file time_secs changed from [%ld]to[%ld]\n",
                 ini_file_time_sec, INF_FILE_GET_CTIME(fp->f_path.dentry));
        ini_file_time_sec = INF_FILE_GET_CTIME(fp->f_path.dentry);

        return INI_FILE_TIMESPEC_RECONFIG;
    } else {
        INI_INFO("ini file is not upadted time_secs[%ld]\n", ini_file_time_sec);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }
}

/*
 *  Prototype    : ini_file_check_conf_update
 *  Description  : check *.ini file is updated or not
 */
int32 ini_file_check_conf_update(void)
{
    INI_FILE *fp = NULL;
    int32 ret;

    /* read spec if exist */
    if ((get_ini_file(ini_conn_file_name_etc, &fp) == INI_SUCC) &&
        (ini_file_check_timespec(fp) == INI_FILE_TIMESPEC_RECONFIG)) {
        INI_INFO("%s ini file is updated\n", ini_conn_file_name_etc);
        ret = INI_FILE_TIMESPEC_RECONFIG;
    } else if ((get_ini_file(INI_FILE_PATH, &fp) == INI_SUCC) &&
               (ini_file_check_timespec(fp) == INI_FILE_TIMESPEC_RECONFIG)) {
        INI_INFO("%s ini file is updated\n", INI_FILE_PATH);
        ret = INI_FILE_TIMESPEC_RECONFIG;
    } else {
        INI_INFO("no ini file is updated\n");
        ret = INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (fp != NULL) {
        ini_file_close(fp);
    }

    return ret;
}

#ifdef HISI_DTS_SUPPORT
int32 get_ini_file_name_from_dts_etc(int8 *dts_prop, int8 *prop_value, uint32 size)
{
#ifdef _PRE_CONFIG_USE_DTS
    return board_info_etc.bd_ops.get_ini_file_name_from_dts_etc(dts_prop, prop_value, size);
#endif
    return INI_SUCC;
}
#endif

/*
 * 函 数 名 : bin_mem_check
 * 功能描述  : 二进制内存检查
 * 输入参数  : int8 *pc_dest需要对比的内存地址
 *             int8 *pc_src比较的内存地址
 *             int16 us_lenth需要比较的内存长度
 */
STATIC int32 bin_mem_check(int8 *pc_dest, const int8 *pc_src, uint16 us_lenth)
{
    int16 loop;
    if (pc_dest == NULL || pc_src == NULL) {
        INI_ERROR("pointer is NULL!");
        return INI_FAILED;
    }

    if (us_lenth == 0) {
        return INI_SUCC;
    }

    for (loop = 0; loop < us_lenth; loop++) {
        if (pc_dest[loop] != pc_src[loop]) {
            return INI_FAILED;
        }
    }
    return INI_SUCC;
}

static int8 *search_target_str(int8* read_buf, uint32 buf_len, const int8 *target_str)
{
    int32 loop;
    uint16 target_len = OAL_STRLEN(target_str);

    for (loop = 0; loop < buf_len - target_len; loop++) {
        /* 判断首尾减少bin_mem_check调用次数 */
        if (!bin_mem_check(&read_buf[loop], target_str, target_len)) {
            return &read_buf[loop];
        }
    }
    return NULL;
}


/*
 * 函 数 名  : get_str_from_file
 * 功能描述  : 从文件中获取固定字符串，保存到另外的位置
 * 输入参数  : char *file_path 文件绝对路径
 *             const int8 *target_str 查找内容
 */
int8 *get_str_from_file_etc(int8 *pc_file_path, const int8 * target_str)
{
    INI_FILE *fp = NULL;
    int32 read_bytes, str_len;
    int8 read_buf[INI_KERNEL_READ_LEN + 1] = {0};
    int8 *version_str = NULL;
    uint8 target_len;
    INI_INFO("%s", __func__);

    if (unlikely(pc_file_path == NULL || target_str == NULL)) {
        INI_ERROR("arg is NULL!");
        return NULL;
    }

    target_len = OAL_STRLEN(target_str);
    if (target_len == 0) {
        INI_ERROR("target_str is empty!");
        return NULL;
    }

    fp = ini_file_open(pc_file_path, "rt");
    if (unlikely(fp == NULL)) {
        INI_ERROR("open file %s fail!", pc_file_path);
        return NULL;
    }
    INI_INFO("open file %s success to find str \"%s\"!", pc_file_path, target_str);

    /* 由于每次比较都会留uc_str_check_len不比较所以不是0 */
    do {
        read_bytes = oal_file_read_ext(fp, fp->f_pos, read_buf, INI_KERNEL_READ_LEN);
        if (read_bytes <= target_len) {
            ini_file_close(fp);
            INI_ERROR("cann't find device sw version string in %s!", pc_file_path);
            return NULL;
        }

        read_buf[read_bytes] = '\0';
        version_str = search_target_str(read_buf, read_bytes, target_str);
        if (version_str != NULL) {
            fp->f_pos += (version_str - read_buf);
            fp->f_pos += target_len;
            memset_s(read_buf, sizeof(read_buf), 0, sizeof(read_buf));
            /* 读取到‘\n’或者最大192B数据到ac_read_buf */
            ko_read_line(fp, read_buf, sizeof(read_buf));
            break;

        } else {
            fp->f_pos += (read_bytes - target_len);
        }
    } while (1);

    str_len = OAL_STRLEN(read_buf);
    version_str = (int8 *)kmalloc(str_len + 1, GFP_KERNEL);
    if (unlikely(version_str == NULL)) {
        INI_ERROR("find device sw version, but memory alloc fail!");
    }else {
        memcpy_s(version_str, str_len + 1, read_buf, str_len + 1);
        INI_INFO("find device sw version :%s", version_str);
    }

    ini_file_close(fp);
    return version_str;
}

int ini_cfg_init_etc(void)
{
#ifdef HISI_DTS_SUPPORT
    int32 ret;
    int8 auc_dts_ini_path[INI_FILE_PATH_LEN] = {0};
#endif

#ifndef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    INI_INFO("hi110x ini config search init!\n");
#endif

#ifdef HISI_DTS_SUPPORT
    ret = get_ini_file_name_from_dts_etc(PROC_NAME_INI_FILE_NAME, auc_dts_ini_path, sizeof(auc_dts_ini_path));
    if (ret < 0) {
        INI_ERROR("can't find dts proc %s\n", PROC_NAME_INI_FILE_NAME);
        return INI_FAILED;
    }
#endif
    INI_INIT_MUTEX(&file_mutex_etc);

#ifdef HISI_DTS_SUPPORT
    if (snprintf_s(ini_file_name_etc, sizeof(ini_file_name_etc),
                   sizeof(ini_file_name_etc) - 1, "%s", auc_dts_ini_path) < 0) {
        INI_ERROR("space is not enough\n");
        return INI_FAILED;
    }
    board_info_etc.ini_file_name = ini_file_name_etc;
    /* Note:"symbol snprintf()"has arg.count conflict(5 vs 4) */
    /*lint -e515*/
#endif
    if (snprintf_s(ini_conn_file_name_etc, sizeof(ini_conn_file_name_etc),
                   sizeof(ini_conn_file_name_etc) - 1, "%s", CUST_PATH_INI_CONN) < 0) {
        INI_ERROR("space is not enough\n");
        return INI_FAILED;
    }
    OAL_IO_PRINT("ini_file_name@%s\n", ini_file_name_etc);

#ifdef HISI_DTS_SUPPORT
    /*lint +e515*/
    INI_INFO("%s@%s\n", PROC_NAME_INI_FILE_NAME, ini_file_name_etc);
#else
    INI_INFO("ini_file_name@%s\n", ini_file_name_etc);
#endif
    return INI_SUCC;
}

void ini_cfg_exit_etc(void)
{
    INI_INFO("hi110x ini config search exit!\n");
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/*lint -e578*/ /*lint -e132*/ /*lint -e745*/ /*lint -e101*/ /*lint -e49*/ /*lint -e601*/
oal_debug_module_param_string(ini_file_name_etc, ini_file_name_etc, INI_FILE_PATH_LEN, OAL_S_IRUGO);
/*lint +e578*/ /*lint +e132*/ /*lint +e745*/ /*lint +e101*/ /*lint +e49*/ /*lint +e601*/
#endif
