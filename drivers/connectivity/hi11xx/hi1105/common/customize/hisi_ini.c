

/* 头文件包含 */
#include "hisi_ini.h"
#include "chr_user.h"
#include "oam_ext_if.h"

#ifdef HISI_DTS_SUPPORT
#include "board.h"
#endif

#ifdef HISI_NVRAM_SUPPORT
#ifdef CONFIG_ARCH_PLATFORM
#include <linux/mtd/nve_ap_kernel_interface.h>
#else
#include <linux/mtd/hisi_nve_interface.h>
#endif
#endif

#include "oal_util.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HISI_INI_C

/* 全局变量定义 */
#define CUST_PATH_INI_CONN "/data/vendor/cust_conn/ini_cfg" /* 某运营商在不同产品的差异配置 */
STATIC char g_ini_file_name[INI_FILE_PATH_LEN] = "/system/bin/wifi_hisi/cfg_e5_hisi.ini";
STATIC char g_ini_conn_file_name[INI_FILE_PATH_LEN] = {0};

#if defined(_PRE_PRODUCT_HI1620S_KUNPENG) || defined(_PRE_WINDOWS_SUPPORT)
#ifdef _PRE_WINDOWS_SUPPORT
#define INI_PATH_INI_CONN "cfg_udp_1103_mpw2.ini"
#else
#define INI_PATH_INI_CONN "/vendor/etc/cfg_udp_1103_mpw2.ini"
#endif
#endif

#ifdef _PRE_SUSPORT_OEMINFO
#define WIFI_CALI_OEMINFO_PATH  "/data/vendor/wifi/wifi_calibrate.conf"
#endif

STATIC long g_ini_file_time = -1;
STATIC fun_wifi_customize_t g_fun_wifi_customize = NULL;

#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
#define HI1103_TEL_HUAWEI_NV_WINVRAM_LENGTH 128
#define EEPROM_WIFI_CALI_PARTNAME    "WLAN"
#endif

STATIC inline bool is_line_character(char ch)
{
    return ((ch == '\n') || (ch == '\r'));
}

/*
 * 从当前文件指针位置开始，读一行(支持windows、linux、mac换行格式)
 * 一行最多读 buf_len - 1 个字节，大于此长度的会被截断
 * 末尾会添加字符串结束符，不返回任何换行符
 */
STATIC int32_t ko_read_line(oal_file_stru *fp, char *addr, uint32_t buf_len)
{
    uint32_t index;
    int32_t ret = oal_file_read_ext(fp, fp->f_pos, addr, buf_len);
    if (ret <= 0) {
        ini_debug("kernel_line read ret = %d", ret);
        return INI_FAILED; // 读结束
    }

    // find the line end
    index = 0;
    ret = (ret < buf_len) ? ret : (buf_len - 1); // 预留1个字节结束符
    while ((index < ret) && (!is_line_character(addr[index]))) {
        index++;
    }
    addr[index++] = '\0';

    // move file pos to next line
    fp->f_pos += index;
    while ((index < ret) && is_line_character(addr[index])) {
        index++;
        fp->f_pos++;
    }

    return ret;
}

STATIC bool ini_file_exist(char *file_path)
{
    oal_file_stru *fp = NULL;

    if (file_path == NULL) {
        ini_error("para file_path is NULL\n");
        return false;
    }

    fp = oal_file_open_readonly(file_path);
    if (fp == NULL) {
        ini_debug("%s not exist\n", file_path);
        return false;
    }

    oal_file_close(fp);

    ini_debug("%s exist\n", file_path);

    return true;
}

STATIC int32_t ini_str_trim_right(char *value, uint32_t value_len)
{
    uint32_t cnt;
    if (value_len < 1) {
        ini_error("ini_str_trim_right fail, value length is 0\n");
        return INI_FAILED;
    }

    /* 替换尾部空格为'\0' */
    cnt = value_len - 1;
    while ((cnt != 0) && (value[cnt] == ' ')) {
        value[cnt--] = '\0';
    }

    return INI_SUCC;
}

STATIC int32_t is_modu_exist(oal_file_stru *fp, const char *modu_name)
{
    int32_t ret = INI_FAILED;
    char tmp[MAX_READ_LINE_NUM] = {0};
    /* find the value of mode var, such as ini_wifi_mode
     * every mode except PLAT mode has only one mode var */
    for (;;) {
        ret = ko_read_line(fp, tmp, sizeof(tmp));
        if (ret == INI_FAILED) {
#ifdef _PRE_CONFIG_INI_PRINT_ERROR_TO_DEBUG
            ini_debug("not find %s!!!", modu_name);
#else
            ini_error("not find %s!!!", modu_name);
#endif
            break;
        }

        if (strncmp(tmp, modu_name, strlen(modu_name)) == 0) {
            ini_debug("have found %s", modu_name);
            return INI_SUCC;
        }
    }

    return INI_FAILED;
}

/*
 *  Prototype    : ini_find_var
 *  Description  : find difference mode variable value, and return value
 */
STATIC int32_t ini_find_var(oal_file_stru *fp, const char *tag, const char *var, char *value, uint32_t size)
{
    int32_t ret;
    char tmp[MAX_READ_LINE_NUM + 1] = {0};
    size_t search_var_len;

    /* find the modu of var, such as [HOST_WIFI_NORMAL] of wifi moduler */
    ret = is_modu_exist(fp, tag);
    if (ret != INI_SUCC) {
        return INI_FAILED;
    }

    /* find the var in modu, such as [HOST_WIFI_NORMAL] of wifi moduler */
    for (;;) {
        ret = ko_read_line(fp, tmp, sizeof(tmp));
        if (ret == INI_FAILED) {
            return INI_FAILED;
        }

        if (tmp[0] == '[') {
            ini_debug("not find %s!!!, check if var in correct mode", var);
            return INI_FAILED;
        }

        search_var_len = strlen(var);
        if ((strncmp(tmp, var, search_var_len) == 0) &&
            (tmp[search_var_len] == '=') && (tmp[search_var_len + 1] != ' ')) {  /* 等号左右两边不能有空格 */
            if (strncpy_s(value, size, &tmp[search_var_len + 1], size - 1)) {
                ini_error("ini_find_var:strncpy_s space is not enough");
                return INI_FAILED;
            }
            break;
        }
    }

    return INI_SUCC;
}

#ifdef _PRE_SUSPORT_OEMINFO
STATIC int32_t get_wifi_calibrate_param(uint32_t datalen, uint32_t offset, uint8_t *cali_param)
{
    int32_t ret;
    int wifi_cal_len = 0;
    uint32_t i;
    char data[HI1103_TEL_HUAWEI_NV_WINVRAM_LENGTH];
    oal_file_stru *file = NULL;

    ret = memset_s(data, HI1103_TEL_HUAWEI_NV_WINVRAM_LENGTH, 0, HI1103_TEL_HUAWEI_NV_WINVRAM_LENGTH);
    if (ret != EOK) {
        ini_error("memset wifi cali data failed.\n");
        return INI_FAILED;
    }

    file = oal_file_open_readonly(WIFI_CALI_OEMINFO_PATH);
    if (file == NULL) {
        return INI_FAILED;
    }
    oal_file_read_ext(file, (loff_t)offset, data, datalen);
    oal_file_close(file);

    for (i = 0; i < datalen; i++) {
        if (data[i] == '\0') {
            wifi_cal_len = i + 1;
            break;
        }
    }
    ret = memcpy_s(cali_param, wifi_cal_len, data, wifi_cal_len);
    if (ret != EOK) {
        ini_error("memcpy wifi cal data failed\n");
        return INI_FAILED;
    }
    return INI_SUCC;
}
#endif

STATIC int32_t ini_find_var_value_by_path(const char *path, const char *tag, const char *var,
                                          char *value, uint32_t size)
{
    oal_file_stru *fp = NULL;

#ifdef INI_TIME_TEST
    struct timeval tv0;
    struct timeval tv1;
#endif

    int32_t l_ret;

    if ((var == NULL) || (var[0] == '\0') || (value == NULL) || (size <= 1)) {
        ini_error("check if var and value is NULL or blank");
        return INI_FAILED;
    }
    if ((tag == NULL) || (tag[0] != '[') || (tag[strlen(tag) - 1] != ']')) {
        ini_error("tag invalid");
        return INI_FAILED;
    }

#ifdef INI_TIME_TEST
    oal_do_gettimeofday(&tv0);
#endif

    fp = oal_file_open_readonly(path);
    if (fp == NULL) {
        ini_error("open %s failed!!!", path);
        return INI_FAILED;
    }

    /* find var in .ini return value */
    l_ret = ini_find_var(fp, tag, var, value, size);
    if (l_ret == INI_FAILED) {
        value[0] = '\0';
        oal_file_close(fp);
        return INI_FAILED;
    }

#ifdef INI_TIME_TEST
    oal_do_gettimeofday(&tv1);
    ini_debug("time take = %ld", (tv1.tv_sec - tv0.tv_sec) * 1000 + (tv1.tv_usec - tv0.tv_usec) / 1000);
#endif

    oal_file_close(fp);

    /* check blank space of value */
    if (ini_str_trim_right(value, strlen(value)) == INI_SUCC) {
        ini_debug("::return %s:%s::", var, value);
        return INI_SUCC;
    }

    return INI_FAILED;
}

/*
 *  Prototype    : ini_find_var_value
 *  Description  : get var value from .ini file
 */
STATIC int32_t ini_find_var_value(const char *tag, const char *var, char *value, uint32_t size)
{
    /* read spec if exist */
    if (ini_file_exist(g_ini_conn_file_name)) {
        if (ini_find_var_value_by_path(g_ini_conn_file_name, tag, var, value, size) == INI_SUCC) {
            return INI_SUCC;
        }
    }

    if (ini_file_exist(g_ini_file_name) == 0) {
        ini_error(" %s not exist!!!", g_ini_file_name);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_INI, CHR_PLAT_DRV_ERROR_INI_MISS);
        return INI_FAILED;
    }

    return ini_find_var_value_by_path(g_ini_file_name, tag, var, value, size);
}

#ifdef HISI_NVRAM_SUPPORT
/*
 *  Prototype    : read_conf_from_nvram
 *  Description  : read nv buff from nvram
 *
 */
int32_t read_conf_from_nvram(uint8_t *pc_out, uint32_t size, uint32_t nv_number, const char *nv_name)
{
#ifdef CONFIG_ARCH_PLATFORM
    struct opt_nve_info_user info;
#else
    struct hisi_nve_info_user info;
#endif
    int32_t ret;

    memset_s(&info, sizeof(info), 0, sizeof(info));
    memset_s(pc_out, size, 0, size);
    if (strcpy_s(info.nv_name, sizeof(info.nv_name), nv_name) != EOK) {
        ini_error("read nvm failed nv_name size[%lu] less than input[%s]", sizeof(info.nv_name), nv_name);
        oam_error_log1(0, OAM_SF_INI, "read_conf_from_nvram:read nvm failed nv_name size[%lu] less than input",
                       sizeof(info.nv_name));
        return INI_FAILED;
    }
    info.nv_name[strlen(HISI_CUST_NVRAM_NAME)] = '\0';
    info.nv_number = nv_number;
    info.valid_size = HISI_CUST_NVRAM_LEN;
    info.nv_operation = HISI_CUST_NVRAM_READ;

#ifdef CONFIG_ARCH_PLATFORM
    ret = nve_direct_access_interface(&info);
#else
    ret = hisi_nve_direct_access(&info);
#endif
    if (size > sizeof(info.nv_data) || size <= OAL_STRLEN(info.nv_data)) {
        ini_error("read nvm item[%s] fail, lenth[%d] longer than input[%d]",
                  nv_name, (uint32_t)OAL_STRLEN(info.nv_data), size);
        return INI_FAILED;
    }
    if (ret == INI_SUCC) {
        if (memcpy_s(pc_out, size, info.nv_data, sizeof(info.nv_data)) != EOK) {
            ini_error("read nvm{%s}lenth[%d] longer than input[%d]",
                      info.nv_data, (uint32_t)OAL_STRLEN(info.nv_data), size);
            oam_error_log2(0, OAM_SF_INI, "read_conf_from_nvram:read nvm lenth[%d] longer than input[%d]",
                           (uint32_t)OAL_STRLEN(info.nv_data), size);
            return INI_FAILED;
        }
        oal_io_print("read_conf_from_nvram::nvram id[%d] nv name[%s] get data{%s}, size[%d]\r\n!",
                     nv_number, nv_name, info.nv_data, size);
    } else {
        ini_error("read nvm [%d] %s failed", nv_number, nv_name);
        return INI_FAILED;
    }

    return INI_SUCC;
}

/*
 *  Prototype    : write_conf_to_nvram
 *  Description  : change value and write to file
 */
int32_t write_conf_to_nvram(char *name, const char *pc_arr)
{
#ifdef CONFIG_ARCH_PLATFORM
    struct opt_nve_info_user info;
#else
    struct hisi_nve_info_user info;
#endif
    int32_t ret;

    unref_param(name);

    memset_s(&info, sizeof(info), 0, sizeof(info));
    if (strcpy_s(info.nv_name, sizeof(info.nv_name), HISI_CUST_NVRAM_NAME) != EOK) {
        oam_error_log1(0, OAM_SF_INI,
                       "write_conf_to_nvram:write_conf_to_nvram failed nv_name size[%lu] less than input[WINVRAM]",
                       sizeof(info.nv_name));
        return INI_FAILED;
    }
    info.nv_name[sizeof(info.nv_name) - 1] = '\0';
    info.nv_number = HISI_CUST_NVRAM_NUM;
    info.valid_size = HISI_CUST_NVRAM_LEN;
    info.nv_operation = HISI_CUST_NVRAM_WRITE;
    if (memcpy_s(info.nv_data, sizeof(info.nv_data), pc_arr, HISI_CUST_NVRAM_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_INI, "write_conf_to_nvram:write nvm[memcpy_s] failed");
        return INI_FAILED;
    }

#ifdef CONFIG_ARCH_PLATFORM
    ret = nve_direct_access_interface(&info);
#else
    ret = hisi_nve_direct_access(&info);
#endif
    if (ret < -1) {
        ini_error("write nvm failed");
        return INI_FAILED;
    }

    return INI_SUCC;
}
#else

#ifdef _PRE_SUSPORT_OEMINFO
int32_t read_conf_from_oeminfo(uint8_t *pc_out, uint32_t size, uint32_t offset)
{
    uint8_t eep_buff[HI1103_TEL_HUAWEI_NV_WINVRAM_LENGTH + 1] = {0};
    int32_t ret;
    uint32_t i = 0;

    ret = memset_s((void *)pc_out, size, 0, size);
    if (ret != EOK) {
        ini_error("memset_s failed %d", ret);
        return INI_FAILED;
    }

    ret = get_wifi_calibrate_param(HI1103_TEL_HUAWEI_NV_WINVRAM_LENGTH, offset, eep_buff);
    if (ret < 0) {
        chr_exception_report(CHR_READ_EEPROM_ERROR_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_INIT, CHR_PLAT_DRV_ERROR_EEPROM_READ_INIT);
        ini_error("oeminfo read failed %d", ret);
        return INI_FAILED;
    }

    for (i = 0; i < HI1103_TEL_HUAWEI_NV_WINVRAM_LENGTH; i++) {
        if (eep_buff[i] == ';') {
            eep_buff[i + 1] = 0;
            break;
        } else if (eep_buff[i] == 0xff) {
            eep_buff[i] = 0;
            break;
        }
    }
    ret = memcpy_s(pc_out, size, eep_buff, oal_min(size, i));
    if (ret != EOK) {
        ini_error("memcpy_s failed %d", ret);
        return INI_FAILED;
    }
    return INI_SUCC;
}
#endif

#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
int32_t read_conf_from_eeprom(uint8_t *pc_out, uint32_t size, uint32_t offset)
{
    uint8_t eep_buff[HI1103_TEL_HUAWEI_NV_WINVRAM_LENGTH + 1] = {0};
    int32_t ret;
    uint32_t i = 0;

    ret = memset_s((void *)pc_out, size, 0, size);
    if (ret != EOK) {
        ini_error("memset_s failed %d", ret);
        return INI_FAILED;
    }

#ifdef _PRE_WINDOWS_SUPPORT
    // no eeprom in windows project
#else
    ret = ini_eeprom_read(EEPROM_WIFI_CALI_PARTNAME, offset,  eep_buff,  HI1103_TEL_HUAWEI_NV_WINVRAM_LENGTH);
    if (ret < 0) {
        chr_exception_report(CHR_READ_EEPROM_ERROR_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_INIT, CHR_PLAT_DRV_ERROR_EEPROM_READ_INIT);
        ini_error("eeprom read failed %d", ret);
        return INI_FAILED;
    }
#endif

    for (i = 0; i < HI1103_TEL_HUAWEI_NV_WINVRAM_LENGTH; i++) {
        if (eep_buff[i] == ';') {
            eep_buff[i + 1] = 0;
            break;
        } else if (eep_buff[i] == 0xff) {
            eep_buff[i] = 0;
            break;
        }
    }
    ret = memcpy_s(pc_out, size, eep_buff, oal_min(size, i));
    if (ret != EOK) {
        ini_error("memcpy_s failed %d", ret);
        return INI_FAILED;
    }
    return INI_SUCC;
}
#else
int32_t read_conf_from_nvram(uint8_t *pc_out, uint32_t size, uint32_t nv_number,  const char *nv_name)
{
    char buf[128]; // nv name不超过128字节
    if (sprintf_s(buf, sizeof(buf), "%s=0", nv_name) < 0) {
        ini_error("sprintf_s failed\n");
        return INI_FAILED;
    }

    if (memcpy_s(pc_out, size, buf, size) != EOK) {
        ini_error("memcpy_s failed\n");
        return INI_FAILED;
    }
    return INI_SUCC;
}
#endif

int32_t write_conf_to_nvram(char *name, char *pc_arr)
{
    return INI_SUCC;
}

#endif

/*
 *  Prototype    : get_cust_conf_string
 *  Description  : get config form *.ini file or dts(kernel)
 */
int32_t get_cust_conf_string(const char *tag, const char *var, char *value, uint32_t size)
{
    memset_s(value, size, 0, size);
    return ini_find_var_value(tag, var, value, size);
}

/*
 *  Prototype    : get_cust_conf_int32
 *  Description  : get config form *.ini file or dts(kernel)
 */
int32_t get_cust_conf_int32(const char *tag, const char *var, int32_t *value)
{
    int32_t ret;
    char out_str[INI_READ_VALUE_LEN] = {0};

    ret = ini_find_var_value(tag, var, out_str, sizeof(out_str));
    if (ret < 0) {
        /* ini_find_var_value has error log, delete this log */
        ini_debug("cust modu didn't get var of %s.", var);
        return INI_FAILED;
    }

    if (!strncmp(out_str, "0x", strlen("0x")) || !strncmp(out_str, "0X", strlen("0X"))) {
        ini_debug("get hex of:%s.", var);
        ret = sscanf_s(out_str, "%x", value);
    } else {
        ret = sscanf_s(out_str, "%d", value);
    }

    if (ret <= 0) {
        ini_error("%s trans to int failed", var);
        return INI_FAILED;
    }

    ini_debug("conf %s get vale:%d", var, *value);

    return INI_SUCC;
}

STATIC int32_t get_ini_file(char *file_path, oal_file_stru **fp)
{
    if (file_path == NULL) {
        ini_info("para file_path is NULL\n");
        return INI_FAILED;
    }

    *fp = oal_file_open_readonly(file_path);
    if (*fp == NULL) {
        ini_info("inifile %s not exist\n", file_path);
        return INI_FAILED;
    }

    return INI_SUCC;
}

/*
 *  Prototype    : ini_file_check_timespec
 *  Description  : get *.ini file timespec
 */
STATIC int32_t ini_file_check_timespec(oal_file_stru *fp)
{
#ifndef _PRE_LINUX_TEST
#ifdef _PRE_WINDOWS_SUPPORT
    static int64 dentry_time = 0x04030201;
    if (g_ini_file_time != dentry_time) {
        g_ini_file_time = dentry_time;
        return INI_FILE_TIMESPEC_RECONFIG;
    } else {
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }
#else
    if (fp == NULL) {
        ini_error("para file is NULL\n");
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (fp->f_path.dentry == NULL) {
        ini_error("file dentry is NULL\n");
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (g_ini_file_time != inf_file_get_ctime(fp->f_path.dentry)) {
        ini_info("oal_file_stru time_secs changed from [%ld]to[%ld]\n",
                 g_ini_file_time, (long)inf_file_get_ctime(fp->f_path.dentry));
        g_ini_file_time = inf_file_get_ctime(fp->f_path.dentry);

        return INI_FILE_TIMESPEC_RECONFIG;
    } else {
        ini_info("ini file is not upadted time_secs[%ld]\n", g_ini_file_time);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }
#endif
#else
    return INI_FILE_TIMESPEC_RECONFIG;
#endif
}

/*
 *  Prototype    : ini_file_check_conf_update
 *  Description  : check *.ini file is updated or not
 */
int32_t ini_file_check_conf_update(void)
{
    oal_file_stru *fp = NULL;
    int32_t ret;

    /* read spec if exist */
    if ((get_ini_file(g_ini_conn_file_name, &fp) == INI_SUCC) &&
        (ini_file_check_timespec(fp) == INI_FILE_TIMESPEC_RECONFIG)) {
        ini_info("%s ini file is updated\n", g_ini_conn_file_name);
        ret = INI_FILE_TIMESPEC_RECONFIG;
    } else if ((get_ini_file(g_ini_file_name, &fp) == INI_SUCC) &&
               (ini_file_check_timespec(fp) == INI_FILE_TIMESPEC_RECONFIG)) {
        ini_info("%s ini file is updated\n", g_ini_file_name);
        ret = INI_FILE_TIMESPEC_RECONFIG;
    } else {
        ini_info("no ini file is updated\n");
        ret = INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (fp != NULL) {
        oal_file_close(fp);
    }

    return ret;
}

#ifdef HISI_DTS_SUPPORT
int32_t get_ini_file_name_from_dts(char *dts_prop, char *prop_value, uint32_t size)
{
#ifdef _PRE_CONFIG_USE_DTS
    return g_st_board_info.bd_ops.get_ini_file_name_from_dts(dts_prop, prop_value, size);
#endif
    return INI_SUCC;
}
#endif

char *get_ini_file_name(void)
{
    return g_ini_file_name;
}

#if defined(_PRE_PRODUCT_HI1620S_KUNPENG) || defined(_PRE_WINDOWS_SUPPORT)
int ini_cfg_init(void)
{
    ini_info("hi110x ini config search init!\n");

    snprintf_s(g_ini_file_name, sizeof(g_ini_file_name), sizeof(g_ini_file_name) - 1, "%s", INI_PATH_INI_CONN);
    oal_io_print("ini_file_name@ %s\n", g_ini_file_name);
    g_st_board_info.ini_file_name = g_ini_file_name;

    return INI_SUCC;
}
#else
int ini_cfg_init(void)
{
#ifdef HISI_DTS_SUPPORT
    int32_t ret;
    char auc_dts_ini_path[INI_FILE_PATH_LEN] = {0};
#endif

    ini_info("hi110x ini config search init!\n");

#ifdef HISI_DTS_SUPPORT
    ret = get_ini_file_name_from_dts(PROC_NAME_INI_FILE_NAME, auc_dts_ini_path, sizeof(auc_dts_ini_path));
    if (ret < 0) {
        ini_error("can't find dts proc %s\n", PROC_NAME_INI_FILE_NAME);
        return INI_FAILED;
    }
    if (snprintf_s(g_ini_file_name, sizeof(g_ini_file_name),
                   sizeof(g_ini_file_name) - 1, "%s", auc_dts_ini_path) < 0) {
        ini_error("space is not enough\n");
        return INI_FAILED;
    }
    g_st_board_info.ini_file_name = g_ini_file_name;
    /* Note:"symbol snprintf()"has arg.count conflict(5 vs 4) */
    /*lint -e515*/
#endif
    if (snprintf_s(g_ini_conn_file_name, sizeof(g_ini_conn_file_name),
                   sizeof(g_ini_conn_file_name) - 1, "%s", CUST_PATH_INI_CONN) < 0) {
        ini_error("space is not enough\n");
        return INI_FAILED;
    }
    oal_io_print("ini_file_name@%s\n", g_ini_file_name);

#ifdef HISI_DTS_SUPPORT
    /*lint +e515*/
    ini_info("%s@%s\n", PROC_NAME_INI_FILE_NAME, g_ini_file_name);
#else
    ini_info("ini_file_name@%s\n", g_ini_file_name);
#endif

    return INI_SUCC;
}
#endif

void ini_cfg_exit(void)
{
    ini_info("hi110x ini config search exit!\n");
}

void register_wifi_customize(fun_wifi_customize_t fun_wifi_customize)
{
    g_fun_wifi_customize = fun_wifi_customize;
}

void wifi_customize_h2d(void)
{
    if (g_fun_wifi_customize != NULL) {
        g_fun_wifi_customize();
    } else {
        ini_error("wifi_customize not register\n");
    }
}

oal_module_symbol(ini_file_check_conf_update);
oal_module_symbol(get_cust_conf_string);
oal_module_symbol(get_cust_conf_int32);
#ifdef HISI_NVRAM_SUPPORT
oal_module_symbol(read_conf_from_nvram);
#endif
oal_module_symbol(register_wifi_customize);

