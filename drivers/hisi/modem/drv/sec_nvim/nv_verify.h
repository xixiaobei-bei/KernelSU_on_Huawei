
#ifndef _NVIM_VERIFIES_H
#define _NVIM_VERIFIES_H
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <bsp_nvim.h>

#define MAX_DIGEST_SIZE              32
#define SHA256_DIGEST_LENGTH         32

#define NVM_HASH_BLOCK_LEN  0x40000

u32 nv_verify_check_sec(s8 * path);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif

/* vim set ts=4 sw=4 */
