

#ifndef __PLAT_FIRMWARE_UTIL_H__
#define __PLAT_FIRMWARE_UTIL_H__
uint8_t *delete_space(uint8_t *string, int32_t *len);
int32_t num_to_string(uint8_t *string, uint32_t number);
int32_t string_to_num(const char *string, int32_t *number);
int32_t file_open_get_len(char *path, os_kernel_file **fp, uint32_t *file_len);
#endif