

/* 头文件包含 */
#include "plat_parse_changid.h"
#include "plat_debug.h"
#include "plat_firmware.h"
#include "board.h"
#include "securec.h"

#define CUST_README_BUFFER_SIZE 2048
#define HI110X_CUST_FIRMWARE_PATH "/vendor/firmware/"
#define HI110X_CUST_README_NAME "/readme.txt"
#define ID_KEY_INFO "Change-Id: "

STATIC uint8_t *g_changid_buffer;

STATIC uint8_t* get_readme_path(void)
{
    uint8_t *path = NULL;
    uint32_t path_len;
    const uint8_t *chip_name;

    chip_name = get_device_board_name();
    if (chip_name == NULL) {
        return NULL;
    }

    path_len = OAL_STRLEN(HI110X_CUST_FIRMWARE_PATH) + OAL_STRLEN(chip_name) +
               OAL_STRLEN(HI110X_CUST_README_NAME) + 1;

    path = (uint8_t *)os_kzalloc_gfp(path_len);
    if (path == NULL) {
        ps_print_err("alloc buffer fail\n");
        return NULL;
    }

    if (strcat_s(path, path_len, HI110X_CUST_FIRMWARE_PATH) != EOK) {
        ps_print_err("strcat [%s] fail, len %d\n", HI110X_CUST_FIRMWARE_PATH, path_len);
        os_mem_kfree(path);
        return NULL;
    }

    if (strcat_s(path, path_len, chip_name) != EOK) {
        ps_print_err("strcat[%s] [%s] fail, len %d\n", path, chip_name, path_len);
        os_mem_kfree(path);
        return NULL;
    }

    if (strcat_s(path, path_len, HI110X_CUST_README_NAME) != EOK) {
        ps_print_err("strcat [%s] fail, len %d\n", HI110X_CUST_README_NAME, path_len);
        os_mem_kfree(path);
        return NULL;
    }

    return path;
}

STATIC uint8_t *get_key_buffer(uint8_t *buf_addr, int32_t buf_len)
{
    uint8_t *begin = NULL;
    uint8_t *end = NULL;

    begin = os_str_str(buf_addr, ID_KEY_INFO);
    if (begin == NULL) {
        ps_print_info("not find key %s from readme\n", ID_KEY_INFO);
        return NULL;
    }

    end = os_str_chr(begin, '\n');
    if (end == NULL) {
        ps_print_info("changid is the last line\n");
        end = buf_addr + buf_len;
    }
    *end = '\0';

    end = os_str_chr(begin, '\r');
    if (end != NULL) {
        *end = '\0';
    }

    return begin;
}

STATIC int32_t plat_parse_changid(uint8_t *buf_addr, int32_t buf_len)
{
    int32_t len;
    uint8_t *buf = NULL;

    buf = get_key_buffer(buf_addr, buf_len);
    if (buf == NULL) {
        ps_print_err("get buffer fail\n");
        return -EINVAL;
    }

    ps_print_dbg("buffer %s \n", buf);
    if (OAL_STRNCMP(buf, ID_KEY_INFO, OAL_STRLEN(ID_KEY_INFO))) {
        ps_print_info("changid key %s not find\n", ID_KEY_INFO);
        return -EINVAL;
    }

    len = OAL_STRLEN(buf_addr) - OAL_STRLEN(ID_KEY_INFO) + 1;
    buf += OAL_STRLEN(ID_KEY_INFO);

    g_changid_buffer = (uint8_t *)os_kzalloc_gfp(len);
    if (oal_unlikely(g_changid_buffer == NULL)) {
        return -ENOMEM;
    }

    if (memcpy_s(g_changid_buffer, len, buf, len) != EOK) {
        os_mem_kfree(g_changid_buffer);
        g_changid_buffer = NULL;
        return -EINVAL;
    }

    return OAL_SUCC;
}

int32_t plat_parse_readme(uint8_t *path, uint8_t *buf)
{
    int32_t ret, size;

    size = firmware_read_cfg(path, buf, CUST_README_BUFFER_SIZE);
    if (oal_unlikely(size < 0)) {
        ps_print_err("read file fail\n");
        return -EFAIL;
    }

    ps_print_dbg("size is %d\n", size);
    if (size == CUST_README_BUFFER_SIZE) {
        ps_print_info("readme size can be oversize %d Byte\n", CUST_README_BUFFER_SIZE);
        return -EFAIL;
    }

    ret = plat_parse_changid(buf, size);
    if (oal_unlikely(ret < 0)) {
        ps_print_err("get changid fail\n");
        return -EFAIL;
    }

    return OAL_SUCC;
}

int32_t plat_read_changid(void)
{
    uint8_t *buf = NULL;
    uint8_t *path = NULL;
    int32_t ret;

    path = get_readme_path();
    if (path == NULL) {
        ps_print_err("get path fail\n");
        return -EFAIL;
    }

    buf = os_kzalloc_gfp(CUST_README_BUFFER_SIZE);
    if (buf == NULL) {
        os_mem_kfree(path);
        ps_print_err("malloc READ_CFG_BUF fail!\n");
        return -ENOMEM;
    }

    ret = plat_parse_readme(path, buf);
    if (ret < 0) {
        ps_print_err("plat cust get changid fail!\n");
    }

    os_mem_kfree(buf);
    os_mem_kfree(path);

    return ret;
}

void plat_changid_print(void)
{
    if (g_changid_buffer != NULL) {
        ps_print_info("changid: %s, chip nr %d\n",
                      g_changid_buffer, get_hi110x_subchip_type());
    }
}
void plat_free_changid_buffer(void)
{
    if (g_changid_buffer != NULL) {
        os_mem_kfree(g_changid_buffer);
        g_changid_buffer = NULL;
    }
}

