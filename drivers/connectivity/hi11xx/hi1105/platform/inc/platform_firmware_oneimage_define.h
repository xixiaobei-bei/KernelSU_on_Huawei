

#ifndef __PLATFORM_FIRMWARE_ONEIMAGE_DEFINE_H__
#define __PLATFORM_FIRMWARE_ONEIMAGE_DEFINE_H__

/* 1 其他头文件包含 */
/* 2 宏定义 */
#if !defined(WIN32) && !defined(_PRE_WINDOWS_SUPPORT)
#define firmware_oneimage_rename(NAME)  NAME##_hi1105

#define firmware_cfg_clear   firmware_oneimage_rename(firmware_cfg_clear)
#define firmware_cfg_init    firmware_oneimage_rename(firmware_cfg_init)
#define firmware_download    firmware_oneimage_rename(firmware_download)
#define firmware_download_function         firmware_oneimage_rename(firmware_download_function)
#define firmware_download_function_priv    firmware_oneimage_rename(firmware_download_function_priv)
#define firmware_get_cfg      firmware_oneimage_rename(firmware_get_cfg)
#define firmware_parse_cmd    firmware_oneimage_rename(firmware_parse_cmd)
#define firmware_read_cfg     firmware_oneimage_rename(firmware_read_cfg)
#define parse_file_cmd        firmware_oneimage_rename(parse_file_cmd)
#endif

#endif