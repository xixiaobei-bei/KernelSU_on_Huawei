
#ifdef HISI_CONN_NVE_SUPPORT
#include "hisi_conn_nve.h"
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
#define THIS_FILE_ID OAM_FILE_ID_HISI_CONN_NVE_C

/* 全局变量定义 */
static uint8_t g_hisi_conn_nve_setup_ok;
static uint32_t g_memory_used = 0;
static uint8_t *g_hisi_conn_nve_ramdisk;
/* =================================================
 * 记录1M空间划分的分区表,目前只有WiFi使用，BFGX预留
 * 表项字段说明：
 * |-->分区标号
 * |-->分区首地址
 * |-->分区长度
 */
static hisi_conn_particion_stru g_hisi_conn_ptable[HISI_CONN_PTABLE_BUT + 1] = {
    {
        HISI_CONN_WIFI_DATA,
        0,
        0,
    },
    {
        HISI_CONN_BFGX_DATA,
        0,
        0,
    },
    {
        HISI_CONN_PTABLE_BUT,
        0,
        0,
    },
    /* 新增依次往后添加 */
};
/* ========================================================================
 * Table of CRC-32's of all single-byte values (made by make_crc_table)
 */
static const uint32_t crc_table[256] = {
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
    0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
    0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
    0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
    0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
    0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
    0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
    0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
    0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
    0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
    0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
    0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
    0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
    0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
    0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
    0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
    0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
    0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
    0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
    0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
    0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
    0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
    0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
    0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
    0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
    0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
    0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
    0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
    0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
    0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
    0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
    0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
    0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
    0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
    0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
    0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
    0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
    0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
    0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
    0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
    0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
    0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
    0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
    0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
    0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
    0x2d02ef8dL
};

/* ========================================================================= */
#define DO_CRC_1(buf) crc = crc_table[((int)crc ^ (*(buf)++)) & 0xff] ^ (crc >> 8);
#define DO_CRC_2(buf)  DO_CRC_1(buf); DO_CRC_1(buf);
#define DO_CRC_4(buf)  DO_CRC_2(buf); DO_CRC_2(buf);
#define DO_CRC_8(buf)  DO_CRC_4(buf); DO_CRC_4(buf);
#define CRC_TWO_BYTE_LEN    8
/* ========================================================================= */
/* 函数定义 */
uint8_t hisi_conn_nve_setup_done(void)
{
    return g_hisi_conn_nve_setup_ok;
}

int32_t hisi_conn_nve_alloc_ramdisk(void)
{
    g_hisi_conn_nve_ramdisk = (uint8_t*)kzalloc(CONN_NVE_SIZE, GFP_KERNEL);
    if (g_hisi_conn_nve_ramdisk == NULL) {
        pr_err("[%s] line %d failed kzalloc.\n", __func__, __LINE__);
        g_hisi_conn_nve_setup_ok = 0;
        return -CONN_NVE_ERROR_INIT;
    }
    g_hisi_conn_nve_setup_ok = 1;
    return CONN_NVE_RET_OK;
}

void hisi_conn_nve_free_ramdisk(void)
{
    kfree(g_hisi_conn_nve_ramdisk);
    g_hisi_conn_nve_ramdisk = NULL;
}

uint8_t* hisi_conn_nve_get_ramdisk(uint32_t particion_id)
{
    // 检查分区是否存在
    if (g_hisi_conn_nve_ramdisk == NULL || particion_id >= HISI_CONN_PTABLE_BUT) {
        return NULL;
    }
    return g_hisi_conn_nve_ramdisk + g_hisi_conn_ptable[particion_id].start;
}
uint32_t crc32c_conn_nve(uint32_t crc, const uint8_t *buf, unsigned long long len)
{
    crc = crc ^ 0xffffffffL;
    while (len >= CRC_TWO_BYTE_LEN) {
        DO_CRC_8(buf);
        len -= CRC_TWO_BYTE_LEN;
    }
    if (len) {
        do {
            DO_CRC_1(buf);
        } while (--len);
    }
    return crc ^ 0xffffffffL;
}
// 各分区实现自己的接口时，应在初始化代码中调用该函数，填充分区表信息
int32_t hisi_conn_nve_set_particion_size(uint32_t particion_id, uint32_t len)
{
    uint32_t total;
    uint32_t i;
    // 检查分区是否存在
    if (particion_id >= HISI_CONN_PTABLE_BUT) {
        return -CONN_NVE_ERROR_SET_SIZE;
    }
    // 检查空间是否足够
    if (g_memory_used - g_hisi_conn_ptable[particion_id].len + len > CONN_NVE_SIZE) {
        return -CONN_NVE_ERROR_SET_SIZE;
    }
    // 更新分区信息
    g_hisi_conn_ptable[particion_id].len = len;
    total = 0;
    for (i = HISI_CONN_WIFI_DATA; i < HISI_CONN_PTABLE_BUT; i++) {
        g_hisi_conn_ptable[i].start = total;
        total += g_hisi_conn_ptable[i].len;
    }
    g_memory_used = total;
    return CONN_NVE_RET_OK;
}
static int32_t hisi_conn_nve_check(uint32_t particion_id, size_t len)
{
    if (particion_id >= HISI_CONN_PTABLE_BUT) {
        printk(KERN_ERR "[CONN_NVE][%s]unknown particion id, id = %u!\n", __func__, particion_id);
        return -CONN_NVE_ERROR_NO_PART;
    }
    if (len > CONN_NVE_SIZE) {
        printk(KERN_ERR "[CONN_NVE][%s]read/write len over max_len, len = %u!\n", __func__, (uint32_t)len);
        return -CONN_NVE_ERROR_OVERSIZE;
    }
    return CONN_NVE_RET_OK;
}
/*
 * Function name:hisi_conn_nve_read.
 * Discription:write NV partition.
 * Parameters:
 *          @ particion_id:id of particion used to find start block number that will be read.
 *          @ offset:used to calculate start block number that will be read.
 *          @ len:total bytes that will be coppied from emmc.
 *          @ buf:given buffer whose bytes will be coppied from emmc.
 */
int32_t hisi_conn_nve_read(uint32_t particion_id, size_t offset, size_t len, uint8_t *buf)
{
    int ret;
    int fd;
    loff_t from;
    /*lint --e{501,48}*/
    mm_segment_t oldfs = get_fs();
    set_fs(get_ds());
    fd = (int)sys_open(CONN_NVE_DEVICE_NAME, O_RDONLY, (int)S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0) {
        printk(KERN_ERR "[CONN_NVE][%s]open nv block device failed, and fd = %x!\n", __func__, fd);
        ret = -ENODEV;
        goto out;
    }
    ret = hisi_conn_nve_check(particion_id, len);
    if (ret != CONN_NVE_RET_OK) {
        printk(KERN_ERR "[CONN_NVE][%s]check params fail!\n", __func__);
        goto out;
    }
    from = (loff_t)(g_hisi_conn_ptable[particion_id].start + offset);
    ret = (int)sys_lseek((unsigned int)fd, from, SEEK_SET);
    if (ret == -1) {
        printk(KERN_ERR "[CONN_NVE][%s] Fatal seek error, read flash from = ""0x%llx, len = 0x%zx, ret = 0x%x.\n",\
            __func__, from, len, ret);
        ret = -EIO;
        goto out;
    }

    ret = (int)sys_read((unsigned int)fd, (char *)buf, len);
    if (ret == -1) {
        printk(KERN_ERR "[CONN_NVE][%s] Fatal read error, read flash from = "
            "0x%llx, len = 0x%zx, ret = 0x%x.\n",
            __func__, from, len, ret);
        ret = -EIO;
        goto out;
    }

    sys_close((unsigned int)fd);
    set_fs(oldfs);
    return 0;

out:
    if (fd >= 0) {
        sys_close((unsigned int)fd);
    }
    set_fs(oldfs);
    return ret;
}
/*
 * Function name:hisi_conn_nve_write.
 * Discription:write NV partition.
 * Parameters:
 *          @ particion_id:id of particion used to find start block number that will be written.
 *          @ offset:used to calculate start block number that will be written.
 *          @ len:total bytes that will be written to emmc.
 *          @ buf:given buffer whose bytes will be written to emmc.
 */
int32_t hisi_conn_nve_write(uint32_t particion_id, size_t offset, size_t len, uint8_t *buf)
{
    int ret;
    int fd;
    loff_t from;
    /*lint --e{501,48}*/
    mm_segment_t oldfs = get_fs();
    set_fs(get_ds());
    fd = (int)sys_open(CONN_NVE_DEVICE_NAME, O_RDWR, (int)S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0) {
        printk(KERN_ERR "[CONN_NVE][%s]open nv block device failed, and fd = %x!\n", __func__, fd);
        ret = -ENODEV;
        goto out;
    }
    ret = hisi_conn_nve_check(particion_id, len);
    if (ret != CONN_NVE_RET_OK) {
        printk(KERN_ERR "[CONN_NVE][%s]check params fail!\n", __func__);
        goto out;
    }
    from = (loff_t)(g_hisi_conn_ptable[particion_id].start + offset);
    ret = (int)sys_lseek((unsigned int)fd, from, SEEK_SET);
    if (ret == -1) {
        printk(KERN_ERR "[CONN_NVE][%s] Fatal seek error, read flash from = "
            "0x%llx, len = 0x%zx, ret = 0x%x.\n", __func__, from, len, ret);
        ret = -EIO;
        goto out;
    }

    ret = (int)sys_write((unsigned int)fd, (char *)buf, len);
    if (ret == -1) {
        printk(KERN_ERR "[CONN_NVE][%s] Fatal write error, read flash from "
            "= 0x%llx, len = 0x%zx, ret = 0x%x.\n", __func__, from, len, ret);
        ret = -EIO;
        goto out;
    }

    ret = sys_fsync(fd);
    if (ret < 0) {
        printk(KERN_ERR "[CONN_NVE][%s] Fatal sync error, read flash from = "
            "0x%llx, len = 0x%zx, ret = 0x%x.\n", __func__, from, len, ret);
        ret = -EIO;
        goto out;
    }
    sys_close((unsigned int)fd);
    set_fs(oldfs);
    return 0;
out:
    if (fd >= 0) {
        sys_close((unsigned int)fd);
    }
    set_fs(oldfs);
    return ret;
}
EXPORT_SYMBOL(hisi_conn_nve_setup_done);
EXPORT_SYMBOL(hisi_conn_nve_get_ramdisk);
EXPORT_SYMBOL(crc32c_conn_nve);
EXPORT_SYMBOL(hisi_conn_nve_set_particion_size);
EXPORT_SYMBOL(hisi_conn_nve_read);
EXPORT_SYMBOL(hisi_conn_nve_write);
#endif
