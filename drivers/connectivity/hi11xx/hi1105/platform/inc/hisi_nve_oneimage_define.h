

#ifndef __HISI_NVE_ONEIMAGE_DEFINE_H__
#define __HISI_NVE_ONEIMAGE_DEFINE_H__

/* 1 其他头文件包含 */
/* 2 宏定义 */
#if !defined(WIN32) && !defined(_PRE_WINDOWS_SUPPORT)
#define NVE_ONEIMAGE_RENAME(NAME)  NAME##_hi1105

#ifdef HISI_CONN_NVE_SUPPORT
#define hisi_conn_nve_setup_done            NVE_ONEIMAGE_RENAME(hisi_conn_nve_setup_done)
#define hisi_conn_nve_alloc_ramdisk         NVE_ONEIMAGE_RENAME(hisi_conn_nve_alloc_ramdisk)
#define hisi_conn_nve_free_ramdisk          NVE_ONEIMAGE_RENAME(hisi_conn_nve_free_ramdisk)
#define hisi_conn_nve_get_ramdisk           NVE_ONEIMAGE_RENAME(hisi_conn_nve_get_ramdisk)
#define crc32c_conn_nve                     NVE_ONEIMAGE_RENAME(crc32c_conn_nve)
#define hisi_conn_nve_set_particion_size    NVE_ONEIMAGE_RENAME(hisi_conn_nve_set_particion_size)
#define hisi_conn_nve_read                  NVE_ONEIMAGE_RENAME(hisi_conn_nve_read)
#define hisi_conn_nve_write                 NVE_ONEIMAGE_RENAME(hisi_conn_nve_write)
#endif

#endif

#endif