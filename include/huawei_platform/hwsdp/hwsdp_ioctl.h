
#ifndef HWSDP_IOCTL_H
#define HWSDP_IOCTL_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/ioctl.h>
#else
#include <stdint.h>
#include <sys/ioctl.h>
#endif

typedef enum {
	HWSDP_IOCTL_SUCCESS = 0,
	HWSDP_IOCTL_ERROR,
	HWSDP_IOCTL_ERR_UNINITIALIZED,
	HWSDP_IOCTL_ERR_INTERNAL,
	HWSDP_IOCTL_ERR_ALLOC_MEMORY,
	HWSDP_IOCTL_ERR_MEMCPY,
	HWSDP_IOCTL_ERR_NULL_PTR,
	HWSDP_IOCTL_ERR_BAD_PARAM,
	HWSDP_IOCTL_ERR_INIT_TEEC_CTX,
	HWSDP_IOCTL_ERR_OPEN_TEEC_SESSION,
	HWSDP_IOCTL_ERR_TEEC_OPERATION,
	HWSDP_IOCTL_ERR_TEEC_INVOKE_CMD,
	HWSDP_IOCTL_ERR_USER_DATA,
} hwsdp_ioctl_ret_t;

#define hwsdp_init_ioctl_data(io_data) do { \
	(io_data)->data_ptr = NULL; \
	(io_data)->data_len = 0u; \
	(io_data)->ret = HWSDP_IOCTL_SUCCESS; \
} while (0)

typedef struct {
#ifdef __KERNEL__
	u8 *data_ptr; /* in/out: data buffer pointer */
	u64 data_len; /* in: length of data */
#else
	uint8_t *data_ptr; /* in/out: data buffer pointer */
	uint64_t data_len; /* in: length of data */
#endif
	hwsdp_ioctl_ret_t ret; /* out: return code */
} hwsdp_ioctl_data_t;

/* magic number */
#define HWSDP_MAGIC 0x1fu

/* ioctl commands */
#define HWSDP_CMD_INIT_TEE _IOR(HWSDP_MAGIC, 1, hwsdp_ioctl_data_t *)
#define HWSDP_CMD_TEEC_OPS _IOWR(HWSDP_MAGIC, 2, hwsdp_ioctl_data_t *)

#endif

