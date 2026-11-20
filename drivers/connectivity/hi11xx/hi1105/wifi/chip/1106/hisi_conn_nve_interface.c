
#ifdef HISI_CONN_NVE_SUPPORT
#include "hisi_conn_nve_interface.h"
#include "oal_kernel_file.h"
#include "oneimage.h"
#include "oal_ext_if.h"
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/random.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/compat.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#include "oal_util.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HISI_CONN_NVE_INTERFACE_C

/* 全局变量定义 */
static struct semaphore g_nv_sem;
static conn_nve_ramdisk_stru g_conn_nve_struct;
static int g_hisi_sync_ramdisk_done;
/*
 * Function name:read_nv_to_ramdisk.
 * Discription:read emmc hisi_conn_nve to ramdisk and check crc.
 * Parameters:None
 * return 0 for ok, 1 for ini fail, 3 for read fail, 4 for crc check fail.
 */
static int32_t read_nv_to_ramdisk(void)
{
    uint32_t tmp_crc;
    uint32_t age_max = 0;
    int up_to_date_idx = -1;
    int i;
    if (hisi_conn_nve_setup_done() == 0) {
        pr_err("[CONN_NVE]:%s faild due to ini failure!\n", __func__);
        return -CONN_NVE_ERROR_INIT;
    }
    // read hisi_conn_nve from emmc
    for (i = 0; i < CONN_NVE_IMG_NUM; i++) {
        if (hisi_conn_nve_read(HISI_CONN_WIFI_DATA, CONN_NVE_START_ADDR(i), sizeof(conn_nve_particion_stru),
            (u_char*)g_conn_nve_struct.conn_nve_ramdisk) != CONN_NVE_RET_OK) {
            continue;
        }
        // check data crc
        tmp_crc = crc32c_conn_nve(CRC32C_REV_SEED,
            (uint8_t*)g_conn_nve_struct.conn_nve_ramdisk, CONN_NVE_PARTICION_SIZE);
        if (tmp_crc != g_conn_nve_struct.conn_nve_ramdisk->crc) {
            pr_err("[CONN_NVE]:%s img[%d] crc check failure, new_crc is 0x%X, old_crc is 0x%X!\n", __func__, i,
                tmp_crc, g_conn_nve_struct.conn_nve_ramdisk->crc);
            continue;
        }
        // compare age, select newest img
        if (g_conn_nve_struct.conn_nve_ramdisk->age >= age_max) {
            age_max = g_conn_nve_struct.conn_nve_ramdisk->age;
            up_to_date_idx = i;
        }
    }
    // read newest data to ram
    if (up_to_date_idx == -1 || hisi_conn_nve_read(HISI_CONN_WIFI_DATA, CONN_NVE_START_ADDR(up_to_date_idx), \
        sizeof(conn_nve_particion_stru), (u_char*)g_conn_nve_struct.conn_nve_ramdisk) != CONN_NVE_RET_OK) {
        pr_err("[CONN_NVE]:%s faild due to read error!\n", __func__);
        return -CONN_NVE_ERROR_READ;
    }
    // check data crc
    tmp_crc = crc32c_conn_nve(CRC32C_REV_SEED, (uint8_t*)g_conn_nve_struct.conn_nve_ramdisk, CONN_NVE_PARTICION_SIZE);
    if (tmp_crc != g_conn_nve_struct.conn_nve_ramdisk->crc) {
        pr_err("[CONN_NVE]:%s faild due to crc check failure, new_crc is 0x%X, old_crc is 0x%X!\n", __func__,
            tmp_crc, g_conn_nve_struct.conn_nve_ramdisk->crc);
        return -CONN_NVE_ERROR_CRC;
    }
    g_hisi_sync_ramdisk_done = 1;
    g_conn_nve_struct.current_id = up_to_date_idx;
    return CONN_NVE_RET_OK;
}

static int32_t conn_nve_clear_all(void)
{
    int32_t error = CONN_NVE_RET_OK;
    unsigned int i;
    /* write init value of conn_nve */
    if (memset_s(g_conn_nve_struct.conn_nve_ramdisk, sizeof(conn_nve_particion_stru),
        0, sizeof(conn_nve_particion_stru)) != 0) {
        return -CONN_NVE_ERROR_CLEAR_ALL;
    }
    g_conn_nve_struct.conn_nve_ramdisk->age = 0;
    g_conn_nve_struct.conn_nve_ramdisk->crc = crc32c_conn_nve(CRC32C_REV_SEED,
        (const uint8_t*)g_conn_nve_struct.conn_nve_ramdisk, CONN_NVE_PARTICION_SIZE);
    for (i = 0; i < CONN_NVE_IMG_NUM; i++) {
        if (hisi_conn_nve_write(HISI_CONN_WIFI_DATA, CONN_NVE_START_ADDR(i), sizeof(conn_nve_particion_stru),
            (u_char*)g_conn_nve_struct.conn_nve_ramdisk)) {
            pr_err("%s clear img[%d] failed!\n", __func__, i);
            error = -CONN_NVE_ERROR_CLEAR_ALL;
        }
    }
    pr_err("[%s] succ, now crc code is 0x%X\n", __func__, g_conn_nve_struct.conn_nve_ramdisk->crc);
    return error;
}
/*
 * 写数据前检查ramdisk内容是否已与emmc同步
 * 每次写数据，整个ramdisk内容会被重新写回emmc
 * 如果ramdisk未同步emmc就被直接写入，会导致emmc数据被破坏
 */
static int32_t conn_nve_check_sync_before_write(void)
{
    int32_t ret = 0;
    /* ensure only one process can visit conn_nve at the same time in kernel */
    if (down_interruptible(&g_nv_sem)) {
        return -EBUSY;
    }
    if (g_hisi_sync_ramdisk_done == 0) {
        // 从emmc同步数据到ram
        ret = read_nv_to_ramdisk();
        // no ini data, try to set 0 all
        if (ret) {
            pr_err("[CONN_NVE]:%s no ini data, try to set 0 all!\n", __func__);
            if (conn_nve_clear_all() != 0) {
                pr_err("[CONN_NVE]:%s set 0 all fail!\n", __func__);
            } else {
                pr_err("[CONN_NVE]:%s set 0 all done!\n", __func__);
                ret = 0;
            }
            up(&g_nv_sem);
            return ret;
        }
        pr_err("[CONN_NVE]:%s sync succ!\n", __func__);
    }
    up(&g_nv_sem);
    return ret;
}


int32_t conn_nve_write_powcal_data(const nv_pow_stru *buf)
{
    if (hisi_conn_nve_setup_done() == 0) {
        pr_err("%s conn_nve ini failed!\n", __func__);
        return -CONN_NVE_ERROR_INIT;
    }
    if (conn_nve_check_sync_before_write()) {
        pr_err("%s check sync failed!\n", __func__);
        return -CONN_NVE_ERROR_WRITE;
    }
    /* ensure only one process can visit conn_nve at the same time in kernel */
    if (down_interruptible(&g_nv_sem)) {
        return -EBUSY;
    }
    // 首先拷贝数据至ram
    if (memcpy_s(&(CONN_NVE_PREFIX->pow_cal_data), sizeof(CONN_NVE_PREFIX->pow_cal_data), buf, sizeof(*buf))) {
        printk(KERN_ERR "conn_nve_write_powcal_data memcpy_s failed!\n");
        up(&g_nv_sem);
        return -CONN_NVE_ERROR_WRITE;
    }
    g_conn_nve_struct.conn_nve_ramdisk->age++;
    g_conn_nve_struct.conn_nve_ramdisk->crc = crc32c_conn_nve(CRC32C_REV_SEED,
        (uint8_t*)g_conn_nve_struct.conn_nve_ramdisk, CONN_NVE_PARTICION_SIZE);
    // 将数据同步到emmc
    ++g_conn_nve_struct.current_id;
    if (hisi_conn_nve_write(HISI_CONN_WIFI_DATA, CONN_NVE_START_ADDR(g_conn_nve_struct.current_id), \
        sizeof(conn_nve_particion_stru), (uint8_t*)g_conn_nve_struct.conn_nve_ramdisk)) {
        printk(KERN_ERR "hisi_conn_nve_write write data failed!\n");
        up(&g_nv_sem);
        return -CONN_NVE_ERROR_WRITE;
    }
    up(&g_nv_sem);
    return CONN_NVE_RET_OK;
}

int conn_nve_read_powcal_data(nv_pow_stru *buf)
{
    int32_t ret;
    if (hisi_conn_nve_setup_done() == 0) {
        pr_err("[CONN_NVE]:conn_nve_read faild due to ini failure!\n");
        return -CONN_NVE_ERROR_INIT;
    }
    /* ensure only one process can visit conn_nve at the same time in kernel */
    if (down_interruptible(&g_nv_sem)) {
        return -EBUSY;
    }
    if (g_hisi_sync_ramdisk_done == 0) {
        // 从emmc同步数据到ram
        ret = read_nv_to_ramdisk();
        if (ret) {
            pr_err("[CONN_NVE]:%s faild due to sync failure!\n", __func__);
            up(&g_nv_sem);
            return ret;
        }
        pr_err("[CONN_NVE]:%s sync succ!\n", __func__);
    }
    // 从ram取数据
    if (memcpy_s(buf, sizeof(*buf), &(CONN_NVE_PREFIX->pow_cal_data), sizeof(CONN_NVE_PREFIX->pow_cal_data))) {
        up(&g_nv_sem);
        return -CONN_NVE_ERROR_READ;
    }
    up(&g_nv_sem);
    return 0;
}

static void conn_nve_copy_pow_data_from_user_2g(const nv_pow_user_info_stru *info)
{
    switch (info->band_type) {
        case POW_CAL_BAND_2G_11B:
            POW_DATA_PREFIX.delta_gain_2g_11b[info->stream][info->band_idx].delta_gain[info->gain_lvl] = *info->buf;
            break;
        case POW_CAL_BAND_2G_OFDM_20:
            POW_DATA_PREFIX.delta_gain_2g_ofdm_20[info->stream][info->band_idx].delta_gain[info->gain_lvl] = \
                *info->buf;
            break;
        case POW_CAL_BAND_2G_OFDM_40:
            POW_DATA_PREFIX.delta_gain_2g_ofdm_40[info->stream][info->band_idx].delta_gain[info->gain_lvl] = \
                *info->buf;
            break;
        default:
            break;
    }
}

static void conn_nve_copy_pow_data_from_user_5g(const nv_pow_user_info_stru *info)
{
    switch (info->band_type) {
        case POW_CAL_BAND_5G_20:
            POW_DATA_PREFIX.delta_gain_5g_20[info->stream][info->band_idx].delta_gain[info->gain_lvl] = *info->buf;
            break;
        case POW_CAL_BAND_5G_40:
            POW_DATA_PREFIX.delta_gain_5g_40[info->stream][info->band_idx].delta_gain[info->gain_lvl] = *info->buf;
            break;
        case POW_CAL_BAND_5G_80:
            POW_DATA_PREFIX.delta_gain_5g_80[info->stream][info->band_idx].delta_gain[info->gain_lvl] = *info->buf;
            break;
        case POW_CAL_BAND_5G_160:
            POW_DATA_PREFIX.delta_gain_5g_160[info->stream][info->band_idx].delta_gain[info->gain_lvl] = *info->buf;
            break;
        default:
            break;
    }
}


int32_t conn_nve_write_powcal_data_pro(const nv_pow_user_info_stru *info)
{
    if (hisi_conn_nve_setup_done() == 0) {
        pr_err("%s :conn_nve ini failure found!\n", __func__);
        return -CONN_NVE_ERROR_INIT;
    }
    if (conn_nve_check_sync_before_write()) {
        pr_err("%s check sync failed!\n", __func__);
        return -CONN_NVE_ERROR_WRITE;
    }
    /* ensure only one process can visit conn_nve at the same time in kernel */
    if (down_interruptible(&g_nv_sem)) {
        return -EBUSY;
    }
    // 首先拷贝数据至ram
    if (info->band_type >= POW_CAL_BAND_2G_11B && info->band_type <= POW_CAL_BAND_5G_20) {
        conn_nve_copy_pow_data_from_user_2g(info);
    } else {
        conn_nve_copy_pow_data_from_user_5g(info);
    }
    g_conn_nve_struct.conn_nve_ramdisk->age++;
    g_conn_nve_struct.conn_nve_ramdisk->crc = crc32c_conn_nve(CRC32C_REV_SEED,
        (uint8_t*)g_conn_nve_struct.conn_nve_ramdisk, CONN_NVE_PARTICION_SIZE);
    // 将ram数据同步到emmc
    ++g_conn_nve_struct.current_id;
    if (hisi_conn_nve_write(HISI_CONN_WIFI_DATA, CONN_NVE_START_ADDR(g_conn_nve_struct.current_id), \
        sizeof(conn_nve_particion_stru), (uint8_t*)g_conn_nve_struct.conn_nve_ramdisk)) {
        printk(KERN_ERR "hisi_conn_nve_write write data failed!\n");
        up(&g_nv_sem);
        return -CONN_NVE_ERROR_WRITE;
    }
    up(&g_nv_sem);
    return CONN_NVE_RET_OK;
}

static void conn_nve_copy_dpd_data_to_user_5g(const nv_dpd_user_info_stru *info)
{
    if (info->bw == WLAN_BANDWIDTH_20) {
        memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_20), \
            sizeof(wlan_cali_dpd_lut_stru));
    } else if (info->lut_type == CONN_NVE_LUT_TYPE_NORMAL) {
        if (info->bw == WLAN_BANDWIDTH_80) {
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_80), \
                sizeof(wlan_cali_dpd_lut_stru));
        } else if (info->bw == WLAN_BANDWIDTH_160) {
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_160), \
                sizeof(wlan_cali_dpd_lut_stru));
        }
    } else if (info->lut_type == CONN_NVE_LUT_TYPE_SIDE_PLUS) {
        if (info->bw == WLAN_BANDWIDTH_80) {
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_side_plus_80), \
                sizeof(wlan_cali_dpd_lut_stru));
        } else if (info->bw == WLAN_BANDWIDTH_160) {
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_side_plus_160), \
                sizeof(wlan_cali_dpd_lut_stru));
        }
    } else if (info->lut_type == CONN_NVE_LUT_TYPE_SIDE_MINUS) {
        if (info->bw == WLAN_BANDWIDTH_80) {
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_side_minus_80), \
                sizeof(wlan_cali_dpd_lut_stru));
        } else if (info->bw == WLAN_BANDWIDTH_160) {
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_side_minus_160), \
                sizeof(wlan_cali_dpd_lut_stru));
        }
    }
}

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
static void conn_nve_copy_dpd_data_to_user_6g(const nv_dpd_user_info_stru *info)
{
    if (info->bw == WLAN_BANDWIDTH_20) {
        memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_20), \
            sizeof(wlan_cali_dpd_lut_stru));
    } else if (info->lut_type == CONN_NVE_LUT_TYPE_NORMAL) {
        if (info->bw == WLAN_BANDWIDTH_80) {
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_80), \
                sizeof(wlan_cali_dpd_lut_stru));
        } else if (info->bw == WLAN_BANDWIDTH_160) {
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_160), \
                sizeof(wlan_cali_dpd_lut_stru));
        }
    } else if (info->lut_type == CONN_NVE_LUT_TYPE_SIDE_PLUS) {
        if (info->bw == WLAN_BANDWIDTH_80) {
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_side_plus_80), \
                sizeof(wlan_cali_dpd_lut_stru));
        } else if (info->bw == WLAN_BANDWIDTH_160) {
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_side_plus_160), \
                sizeof(wlan_cali_dpd_lut_stru));
        }
    } else if (info->lut_type == CONN_NVE_LUT_TYPE_SIDE_MINUS) {
        if (info->bw == WLAN_BANDWIDTH_80) {
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_side_minus_80), \
                sizeof(wlan_cali_dpd_lut_stru));
        } else if (info->bw == WLAN_BANDWIDTH_160) {
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_side_minus_160), \
                sizeof(wlan_cali_dpd_lut_stru));
        }
    }
}

static void conn_nve_copy_dpd_data_from_user_6g(const nv_dpd_user_info_stru *info)
{
    if (info->bw == WLAN_BANDWIDTH_20) {
        memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_20), sizeof(*info->buf), info->buf, \
            sizeof(wlan_cali_dpd_lut_stru));
    } else if (info->lut_type == CONN_NVE_LUT_TYPE_NORMAL) {
        if (info->bw == WLAN_BANDWIDTH_80) {
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_80), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
        } else if (info->bw == WLAN_BANDWIDTH_160) {
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_160), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
        }
    } else if (info->lut_type == CONN_NVE_LUT_TYPE_SIDE_PLUS) {
        if (info->bw == WLAN_BANDWIDTH_80) {
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_side_plus_80), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
        } else if (info->bw == WLAN_BANDWIDTH_160) {
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_side_plus_160), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
        }
    } else if (info->lut_type == CONN_NVE_LUT_TYPE_SIDE_MINUS) {
        if (info->bw == WLAN_BANDWIDTH_80) {
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_side_minus_80), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
        } else if (info->bw == WLAN_BANDWIDTH_160) {
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_6g_side_minus_160), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
        }
    }
}
#endif

static void conn_nve_do_copy_dpd_data_to_user(const nv_dpd_user_info_stru *info)
{
    switch (info->band_type) {
        case WLAN_CALI_BAND_2G:
            memcpy_s(info->buf, sizeof(*info->buf), CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_2g), \
                sizeof(wlan_cali_dpd_lut_stru));
            break;
        case WLAN_CALI_BAND_5G:
            conn_nve_copy_dpd_data_to_user_5g(info);
            break;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        case WLAN_CALI_BAND_6G:
            conn_nve_copy_dpd_data_to_user_6g(info);
            break;
#endif
        default:
            break;
    }
}

static void conn_nve_copy_dpd_data_from_user_5g(const nv_dpd_user_info_stru *info)
{
    if (info->bw == WLAN_BANDWIDTH_20) {
        memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_20), sizeof(*info->buf), info->buf, \
            sizeof(wlan_cali_dpd_lut_stru));
    } else if (info->lut_type == CONN_NVE_LUT_TYPE_NORMAL) {
        if (info->bw == WLAN_BANDWIDTH_80) {
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_80), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
        } else if (info->bw == WLAN_BANDWIDTH_160) {
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_160), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
        }
    } else if (info->lut_type == CONN_NVE_LUT_TYPE_SIDE_PLUS) {
        if (info->bw == WLAN_BANDWIDTH_80) {
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_side_plus_80), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
        } else if (info->bw == WLAN_BANDWIDTH_160) {
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_side_plus_160), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
        }
    } else if (info->lut_type == CONN_NVE_LUT_TYPE_SIDE_MINUS) {
        if (info->bw == WLAN_BANDWIDTH_80) {
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_side_minus_80), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
        } else if (info->bw == WLAN_BANDWIDTH_160) {
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_5g_side_minus_160), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
        }
    }
}

static void conn_nve_do_copy_dpd_data_from_user(const nv_dpd_user_info_stru *info)
{
    switch (info->band_type) {
        case WLAN_CALI_BAND_2G:
            memcpy_s(CONN_NVE_DPD_UNIT(CONN_NVE_PREFIX, cali_data_2g), sizeof(*info->buf), info->buf, \
                sizeof(wlan_cali_dpd_lut_stru));
            break;
        case WLAN_CALI_BAND_5G:
            conn_nve_copy_dpd_data_from_user_5g(info);
            break;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        case WLAN_CALI_BAND_6G:
            conn_nve_copy_dpd_data_from_user_6g(info);
            break;
#endif
        default: break;
    }
}

int32_t conn_nve_read_dpd_data(const nv_dpd_user_info_stru *info)
{
    int32_t ret;
    if (hisi_conn_nve_setup_done() == 0) {
        pr_err("[CONN_NVE]:conn_nve_read faild due to ini failure!\n");
        return -CONN_NVE_ERROR_INIT;
    }
    /* ensure only one process can visit conn_nve at the same time in kernel */
    if (down_interruptible(&g_nv_sem)) {
        return -EBUSY;
    }
    if (g_hisi_sync_ramdisk_done == 0) {
        // 从emmc同步数据到ram
        ret = read_nv_to_ramdisk();
        if (ret) {
            pr_err("[CONN_NVE]:%s faild due to sync failure!\n", __func__);
            up(&g_nv_sem);
            return ret;
        }
        pr_err("[CONN_NVE]:%s sync succ!\n", __func__);
    }
    // 从ram取数据
    conn_nve_do_copy_dpd_data_to_user(info);
    up(&g_nv_sem);
    return 0;
}

int32_t conn_nve_write_dpd_data(const nv_dpd_user_info_stru *info)
{
    if (hisi_conn_nve_setup_done() == 0) {
        pr_err("%s conn_nve ini failed!\n", __func__);
        return -CONN_NVE_ERROR_INIT;
    }
    if (conn_nve_check_sync_before_write()) {
        pr_err("%s check sync failed!\n", __func__);
        return -CONN_NVE_ERROR_WRITE;
    }
    /* ensure only one process can visit conn_nve at the same time in kernel */
    if (down_interruptible(&g_nv_sem)) {
        return -EBUSY;
    }
    // 首先拷贝数据至ram
    conn_nve_do_copy_dpd_data_from_user(info);
    g_conn_nve_struct.conn_nve_ramdisk->age++;
    g_conn_nve_struct.conn_nve_ramdisk->crc = crc32c_conn_nve(CRC32C_REV_SEED,
        (uint8_t*)g_conn_nve_struct.conn_nve_ramdisk, CONN_NVE_PARTICION_SIZE);
    // 将ram数据同步到emmc
    ++g_conn_nve_struct.current_id;
    if (hisi_conn_nve_write(HISI_CONN_WIFI_DATA, CONN_NVE_START_ADDR(g_conn_nve_struct.current_id), \
        sizeof(conn_nve_particion_stru), (uint8_t*)g_conn_nve_struct.conn_nve_ramdisk)) {
        printk(KERN_ERR "hisi_conn_nve_write write data failed!\n");
        up(&g_nv_sem);
        return -CONN_NVE_ERROR_WRITE;
    }
    up(&g_nv_sem);
    return CONN_NVE_RET_OK;
}

int32_t conn_nve_write_rssi_data(const nv_rssi_user_info_stru *info)
{
    if (hisi_conn_nve_setup_done() == 0) {
        pr_err("%s conn_nve ini failed!\n", __func__);
        return -CONN_NVE_ERROR_INIT;
    }
    if (conn_nve_check_sync_before_write()) {
        pr_err("%s check sync failed!\n", __func__);
        return -CONN_NVE_ERROR_WRITE;
    }
    /* ensure only one process can visit conn_nve at the same time in kernel */
    if (down_interruptible(&g_nv_sem)) {
        return -EBUSY;
    }
    // 首先拷贝数据至ram
    if (info->band_type == WLAN_CALI_BAND_2G) {
        CONN_NVE_PREFIX->rssi_cal_data.delta_gain_2g[info->stream][info->band_idx][info->gain_lvl][info->fem_lvl] = \
            *info->buf;
    } else {
        CONN_NVE_PREFIX->rssi_cal_data.delta_gain_5g[info->stream][info->band_idx][info->gain_lvl][info->fem_lvl] = \
            *info->buf;
    }
    g_conn_nve_struct.conn_nve_ramdisk->age++;
    g_conn_nve_struct.conn_nve_ramdisk->crc = crc32c_conn_nve(CRC32C_REV_SEED,
        (uint8_t*)g_conn_nve_struct.conn_nve_ramdisk, CONN_NVE_PARTICION_SIZE);
    // 将ram数据同步到emmc
    ++g_conn_nve_struct.current_id;
    if (hisi_conn_nve_write(HISI_CONN_WIFI_DATA, CONN_NVE_START_ADDR(g_conn_nve_struct.current_id), \
        sizeof(conn_nve_particion_stru), (uint8_t*)g_conn_nve_struct.conn_nve_ramdisk)) {
        printk(KERN_ERR "hisi_conn_nve_write write data failed!\n");
        up(&g_nv_sem);
        return -CONN_NVE_ERROR_WRITE;
    }
    up(&g_nv_sem);
    return CONN_NVE_RET_OK;
}

int32_t conn_nve_read_rssi_data(const nv_rssi_user_info_stru *info)
{
    int32_t ret;
    if (hisi_conn_nve_setup_done() == 0) {
        pr_err("[CONN_NVE]:conn_nve_read faild due to ini failure!\n");
        return -CONN_NVE_ERROR_INIT;
    }
    /* ensure only one process can visit conn_nve at the same time in kernel */
    if (down_interruptible(&g_nv_sem)) {
        return -EBUSY;
    }
    if (g_hisi_sync_ramdisk_done == 0) {
        // 从emmc同步数据到ram
        ret = read_nv_to_ramdisk();
        if (ret) {
            pr_err("[CONN_NVE]:%s faild due to sync failure!\n", __func__);
            up(&g_nv_sem);
            return ret;
        }
        pr_err("[CONN_NVE]:%s sync succ!\n", __func__);
    }
    // 从ram取数据
    if (info->band_type == WLAN_CALI_BAND_2G) {
        *info->buf = \
            CONN_NVE_PREFIX->rssi_cal_data.delta_gain_2g[info->stream][info->band_idx][info->gain_lvl][info->fem_lvl];
    } else {
        *info->buf = \
            CONN_NVE_PREFIX->rssi_cal_data.delta_gain_5g[info->stream][info->band_idx][info->gain_lvl][info->fem_lvl];
    }
    up(&g_nv_sem);
    return CONN_NVE_RET_OK;
}

static void conn_nve_clear_pow_2g_by_cmd(uint8_t cmd)
{
    uint8_t core_idx = cmd - CONN_NVE_CLEAR_POW_C0;
    memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_11b[core_idx], \
        sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_11b[core_idx]),\
        0, sizeof(wlan_cali_pow_para_stru) * WICAL_2G_RF_BAND_NUM);
    memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_ofdm_20[core_idx], \
        sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_ofdm_20[core_idx]), \
        0, sizeof(wlan_cali_pow_para_stru) * WICAL_2G_RF_BAND_NUM);
    memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_ofdm_40[core_idx], \
        sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_ofdm_40[core_idx]), \
        0, sizeof(wlan_cali_pow_para_stru) * WICAL_2G_RF_BAND_NUM);
}

static void conn_nve_clear_pow_5g_by_cmd(uint8_t cmd)
{
    uint8_t core_idx = cmd - CONN_NVE_CLEAR_POW_C0;
    memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_20[core_idx], \
        sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_20[core_idx]), \
        0, sizeof(wlan_cali_pow_para_stru) * WICAL_5G_RF_BAND_NUM);
    memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_40[core_idx], \
        sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_40[core_idx]), \
        0, sizeof(wlan_cali_pow_para_stru) * WICAL_5G_RF_BAND_NUM);
    memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_80[core_idx], \
        sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_80[core_idx]), \
        0, sizeof(wlan_cali_pow_para_stru) * WICAL_5G_RF_BAND_NUM);
    memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_160[core_idx], \
        sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_160[core_idx]), \
        0, sizeof(wlan_cali_pow_para_stru) * WICAL_5G_RF_BAND_NUM);
}

static unsigned int conn_nve_clear_pow_by_cmd(uint8_t cmd)
{
    unsigned int ret;
    if (g_hisi_sync_ramdisk_done == 0) {
        // 从emmc同步数据到ram
        ret = read_nv_to_ramdisk();
        if (ret) {
            pr_err("[CONN_NVE]:%s faild due to sync failure!\n", __func__);
            return ret;
        }
        pr_err("[CONN_NVE]:%s sync succ!\n", __func__);
    }
    /* write init value of conn_nve */
    if (cmd == CONN_NVE_CLEAR_POW_ALL) {
        memset_s(&CONN_NVE_PREFIX->pow_cal_data, sizeof(CONN_NVE_PREFIX->pow_cal_data), 0, sizeof(nv_pow_stru));
    } else if (cmd >= CONN_NVE_CLEAR_POW_C0 && cmd <= CONN_NVE_CLEAR_POW_C3) {
        conn_nve_clear_pow_2g_by_cmd(cmd);
        conn_nve_clear_pow_5g_by_cmd(cmd);
    } else if (cmd == CONN_NVE_CLEAR_POW_2G) {
        memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_11b,
            sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_11b),
            0, sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_11b));
        memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_ofdm_20,
            sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_ofdm_20),
            0, sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_ofdm_20));
        memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_ofdm_40,
            sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_ofdm_40),
            0, sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_2g_ofdm_40));
    } else if (cmd == CONN_NVE_CLEAR_POW_5G) {
        memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_20,
            sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_20),
            0, sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_20));
        memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_40,
            sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_40),
            0, sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_40));
        memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_80,
            sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_80),
            0, sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_80));
        memset_s(&CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_160,
            sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_160),
            0, sizeof(CONN_NVE_PREFIX->pow_cal_data.delta_gain_5g_160));
    }

    return CONN_NVE_RET_OK;
}

static unsigned int conn_nve_clear_rssi_by_cmd(uint8_t cmd)
{
    unsigned int ret;
    uint8_t core_idx;
    if (g_hisi_sync_ramdisk_done == 0) {
        // 从emmc同步数据到ram
        ret = read_nv_to_ramdisk();
        if (ret) {
            pr_err("[CONN_NVE]:%s faild due to sync failure!\n", __func__);
            return ret;
        }
        pr_err("[CONN_NVE]:%s sync succ!\n", __func__);
    }
    /* write init value of conn_nve */
    if (cmd == CONN_NVE_CLEAR_RSSI_ALL) {
        memset_s(&CONN_NVE_PREFIX->rssi_cal_data, sizeof(CONN_NVE_PREFIX->rssi_cal_data), 0, sizeof(nv_rssi_stru));
    } else if (cmd >= CONN_NVE_CLEAR_RSSI_C0 && cmd <= CONN_NVE_CLEAR_RSSI_C3) {
        core_idx = cmd - CONN_NVE_CLEAR_RSSI_C0;
        memset_s(&CONN_NVE_PREFIX->rssi_cal_data.delta_gain_2g[core_idx], \
            sizeof(CONN_NVE_PREFIX->rssi_cal_data.delta_gain_2g[core_idx]), \
            0, RSSI_ONE_STREAM_SIZE_2G);
        memset_s(&CONN_NVE_PREFIX->rssi_cal_data.delta_gain_5g[core_idx], \
            sizeof(CONN_NVE_PREFIX->rssi_cal_data.delta_gain_5g[core_idx]), \
            0, RSSI_ONE_STREAM_SIZE_5G);
    } else if (cmd == CONN_NVE_CLEAR_RSSI_2G) {
        memset_s(&CONN_NVE_PREFIX->rssi_cal_data.delta_gain_2g, \
            sizeof(CONN_NVE_PREFIX->rssi_cal_data.delta_gain_2g), \
            0, sizeof(CONN_NVE_PREFIX->rssi_cal_data.delta_gain_2g));
    } else if (cmd == CONN_NVE_CLEAR_RSSI_5G) {
        memset_s(&CONN_NVE_PREFIX->rssi_cal_data.delta_gain_5g, \
            sizeof(CONN_NVE_PREFIX->rssi_cal_data.delta_gain_5g), \
            0, sizeof(CONN_NVE_PREFIX->rssi_cal_data.delta_gain_5g));
    }

    return CONN_NVE_RET_OK;
}

static void conn_nve_clear_dpd_2g_5g_by_cmd(uint8_t cmd)
{
    uint8_t core_idx = cmd - CONN_NVE_CLEAR_DPD_C0;
    memset_s(&DPD_DATA_PREFIX.cali_data_2g[core_idx], sizeof(DPD_DATA_PREFIX.cali_data_2g[core_idx]), \
        0, DPD_SIZE(WLAN_2G_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_20[core_idx], sizeof(DPD_DATA_PREFIX.cali_data_5g_20[core_idx]), \
        0, DPD_SIZE(WLAN_5G_20M_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_80[core_idx], sizeof(DPD_DATA_PREFIX.cali_data_5g_80[core_idx]), \
        0, DPD_SIZE(WLAN_5G_20M_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_160[core_idx], sizeof(DPD_DATA_PREFIX.cali_data_5g_160[core_idx]), \
        0, DPD_SIZE(WLAN_5G_160M_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_side_plus_80[core_idx], \
        sizeof(DPD_DATA_PREFIX.cali_data_5g_side_plus_80[core_idx]), 0, DPD_SIZE(WLAN_5G_20M_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_side_minus_80[core_idx], \
        sizeof(DPD_DATA_PREFIX.cali_data_5g_side_minus_80[core_idx]), 0, DPD_SIZE(WLAN_5G_20M_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_side_plus_160[core_idx], \
        sizeof(DPD_DATA_PREFIX.cali_data_5g_side_plus_160[core_idx]), 0, DPD_SIZE(WLAN_5G_160M_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_side_minus_160[core_idx], \
        sizeof(DPD_DATA_PREFIX.cali_data_5g_side_minus_160[core_idx]), 0, DPD_SIZE(WLAN_5G_160M_CALI_BAND_NUM));
}

static void conn_nve_clear_dpd_5g_all(void)
{
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_20, sizeof(DPD_DATA_PREFIX.cali_data_5g_20), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_5g_20));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_80, sizeof(DPD_DATA_PREFIX.cali_data_5g_80), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_5g_80));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_160, sizeof(DPD_DATA_PREFIX.cali_data_5g_160), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_5g_160));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_side_plus_80, sizeof(DPD_DATA_PREFIX.cali_data_5g_side_plus_80), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_5g_side_plus_80));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_side_minus_80, sizeof(DPD_DATA_PREFIX.cali_data_5g_side_minus_80), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_5g_side_minus_80));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_side_plus_160, sizeof(DPD_DATA_PREFIX.cali_data_5g_side_plus_160), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_5g_side_plus_160));
    memset_s(&DPD_DATA_PREFIX.cali_data_5g_side_minus_160, sizeof(DPD_DATA_PREFIX.cali_data_5g_side_minus_160), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_5g_side_minus_160));
}

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
static void conn_nve_clear_dpd_6g_by_cmd(uint8_t cmd)
{
    uint8_t core_idx = cmd - CONN_NVE_CLEAR_DPD_C0;
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_20[core_idx], \
        sizeof(DPD_DATA_PREFIX.cali_data_6g_20[core_idx]), 0, DPD_SIZE(WLAN_6G_20M_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_80[core_idx], \
        sizeof(DPD_DATA_PREFIX.cali_data_6g_80[core_idx]), 0, DPD_SIZE(WLAN_6G_20M_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_160[core_idx], \
        sizeof(DPD_DATA_PREFIX.cali_data_6g_160[core_idx]), 0, DPD_SIZE(WLAN_6G_160M_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_side_plus_80[core_idx], \
        sizeof(DPD_DATA_PREFIX.cali_data_6g_side_plus_80[core_idx]), 0, DPD_SIZE(WLAN_6G_20M_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_side_minus_80[core_idx], \
        sizeof(DPD_DATA_PREFIX.cali_data_6g_side_minus_80[core_idx]), 0, DPD_SIZE(WLAN_6G_20M_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_side_plus_160[core_idx], \
        sizeof(DPD_DATA_PREFIX.cali_data_6g_side_plus_160[core_idx]), 0, DPD_SIZE(WLAN_6G_160M_CALI_BAND_NUM));
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_side_minus_160[core_idx], \
        sizeof(DPD_DATA_PREFIX.cali_data_6g_side_minus_160[core_idx]), 0, DPD_SIZE(WLAN_6G_160M_CALI_BAND_NUM));
}

static void conn_nve_clear_dpd_6g_all(void)
{
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_20, sizeof(DPD_DATA_PREFIX.cali_data_6g_20), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_6g_20));
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_80, sizeof(DPD_DATA_PREFIX.cali_data_6g_80), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_6g_80));
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_160, sizeof(DPD_DATA_PREFIX.cali_data_6g_160), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_6g_160));
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_side_plus_80, sizeof(DPD_DATA_PREFIX.cali_data_6g_side_plus_80), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_6g_side_plus_80));
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_side_minus_80, sizeof(DPD_DATA_PREFIX.cali_data_6g_side_minus_80), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_6g_side_minus_80));
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_side_plus_160, sizeof(DPD_DATA_PREFIX.cali_data_6g_side_plus_160), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_6g_side_plus_160));
    memset_s(&DPD_DATA_PREFIX.cali_data_6g_side_minus_160, sizeof(DPD_DATA_PREFIX.cali_data_6g_side_minus_160), \
        0, sizeof(DPD_DATA_PREFIX.cali_data_6g_side_minus_160));
}

#endif

static unsigned int conn_nve_clear_dpd_by_cmd(uint8_t cmd)
{
    unsigned int ret;

    if (g_hisi_sync_ramdisk_done == 0) {
        // 从emmc同步数据到ram
        ret = read_nv_to_ramdisk();
        if (ret) {
            pr_err("[CONN_NVE]:%s faild due to sync failure!\n", __func__);
            return ret;
        }
        pr_err("[CONN_NVE]:%s sync succ!\n", __func__);
    }
    /* write init value of conn_nve */
    if (cmd == CONN_NVE_CLEAR_DPD_ALL) {
        memset_s(&CONN_NVE_PREFIX->dpd_cal_data, sizeof(CONN_NVE_PREFIX->dpd_cal_data), 0, sizeof(nv_dpd_stru));
    } else if (cmd >= CONN_NVE_CLEAR_DPD_C0 && cmd <= CONN_NVE_CLEAR_DPD_C3) {
        conn_nve_clear_dpd_2g_5g_by_cmd(cmd);
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        conn_nve_clear_dpd_6g_by_cmd(cmd);
#endif
    } else if (cmd == CONN_NVE_CLEAR_DPD_2G) {
        memset_s(&DPD_DATA_PREFIX.cali_data_2g, sizeof(DPD_DATA_PREFIX.cali_data_2g), \
            0, sizeof(DPD_DATA_PREFIX.cali_data_2g));
    } else if (cmd == CONN_NVE_CLEAR_DPD_5G) {
        conn_nve_clear_dpd_5g_all();
    }
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    if (cmd == CONN_NVE_CLEAR_DPD_6G) {
        conn_nve_clear_dpd_6g_all();
    }
#endif

    return CONN_NVE_RET_OK;
}

static int32_t conn_nve_do_clear(void)
{
    int32_t ret = CONN_NVE_RET_OK;
    g_conn_nve_struct.conn_nve_ramdisk->age++;
    g_conn_nve_struct.conn_nve_ramdisk->crc = crc32c_conn_nve(CRC32C_REV_SEED,
        (uint8_t*)g_conn_nve_struct.conn_nve_ramdisk, CONN_NVE_PARTICION_SIZE);
    // 将ram数据同步到emmc
    ++g_conn_nve_struct.current_id;
    if (hisi_conn_nve_write(HISI_CONN_WIFI_DATA, CONN_NVE_START_ADDR(g_conn_nve_struct.current_id), \
        sizeof(conn_nve_particion_stru), (uint8_t*)g_conn_nve_struct.conn_nve_ramdisk)) {
        printk(KERN_ERR "hisi_conn_nve_write write data failed!\n");
        ret = -CONN_NVE_ERROR_WRITE;
    }
    up(&g_nv_sem);
    pr_err("[%s] return %d, now crc code is 0x%X\n", __func__, ret, g_conn_nve_struct.conn_nve_ramdisk->crc);
    return ret;
}

int32_t conn_nve_clear_by_cmd(uint8_t cmd)
{
    int32_t ret = CONN_NVE_RET_OK;

    if (hisi_conn_nve_setup_done() == 0) {
        pr_err("[CONN_NVE]:%s faild due to ini failure!\n", __func__);
        return -CONN_NVE_ERROR_INIT;
    }
    /* ensure only one process can visit conn_nve at the same time in kernel */
    if (down_interruptible(&g_nv_sem)) {
        return -EBUSY;
    }
    if (cmd == CONN_NVE_CLEAR_ALL) {
        ret = conn_nve_clear_all();
    } else if (cmd >= CONN_NVE_CLEAR_POW_C0 && cmd <= CONN_NVE_CLEAR_POW_ALL) {
        ret = conn_nve_clear_pow_by_cmd(cmd);
    } else if (cmd >= CONN_NVE_CLEAR_RSSI_C0 && cmd <= CONN_NVE_CLEAR_RSSI_ALL) {
        ret = conn_nve_clear_rssi_by_cmd(cmd);
    } else if (cmd >= CONN_NVE_CLEAR_DPD_C0 && cmd <= CONN_NVE_CLEAR_DPD_ALL) {
        ret = conn_nve_clear_dpd_by_cmd(cmd);
    }
    // do actual clear operation
    ret = (cmd == CONN_NVE_CLEAR_ALL ? ret : \
        (ret == CONN_NVE_RET_OK ? conn_nve_do_clear() : CONN_NVE_ERROR_CLEAR_ALL));
    up(&g_nv_sem);
    return ret;
}

static int32_t conn_nve_get_wifi_cali_data_buf(void)
{
    /* get ram addr alloced in platform */
    g_conn_nve_struct.conn_nve_ramdisk = (conn_nve_particion_stru *)hisi_conn_nve_get_ramdisk(HISI_CONN_WIFI_DATA);
    if (g_conn_nve_struct.conn_nve_ramdisk == NULL) {
        pr_err("[%s] line %d failed kzalloc.\n", __func__, __LINE__);
        return -CONN_NVE_ERROR_INIT;
    }
    return CONN_NVE_RET_OK;
}

int32_t conn_nve_init_wifi_nvdata(void)
{
    /* semaphore initial */
    sema_init(&g_nv_sem, 1);
    pr_err("[%s]hisi_conn_nve init begin.\n", __func__);
    // send wifidata particion info first
    if (hisi_conn_nve_set_particion_size(HISI_CONN_WIFI_DATA, CONN_NVE_WIFIDATA_LEN) != CONN_NVE_RET_OK) {
        return -CONN_NVE_ERROR_INIT;
    }
    if (conn_nve_get_wifi_cali_data_buf() != CONN_NVE_RET_OK) {
        return -CONN_NVE_ERROR_INIT;
    }
    g_hisi_sync_ramdisk_done = 0;
    pr_err("[%s]hisi_conn_nve init end.\n", __func__);
    return CONN_NVE_RET_OK;
}
#endif
