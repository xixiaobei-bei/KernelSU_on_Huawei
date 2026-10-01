#ifndef __MDRV_MALLOC_H__
#define __MDRV_MALLOC_H__
#ifdef __cplusplus
extern "C"
{
#endif

enum MALLOC_RESERVED_REQ_ID
{
	MALLOC_SEC_SHM_REQ_MIN = 0,
	MALLOC_SEC_SHM_REQ_MAX,

	MALLOC_NONSEC_SHM_REQ_MIN = 0x100,
	MALLOC_NONSEC_SHM_REQ_MAX,

	MALLOC_TCM_REQ_MIN = 0x200,
	MALLOC_TCM_REQ_MAX,

	MALLOC_LLRAM_REQ_MIN = 0x300,
	MALLOC_LLRAM_REQ_MAX,

	MALLOC_AXIMEM_REQ_MIN = 0x400,
	MALLOC_AXIMEM_REQ_MAX,

	MALLOC_MEMBUF_REQ_MIN = 0x500,
	MALLOC_MEMBUF_REQ_MAX
};

void* mdrv_malloc_virt_to_phy(void *);

#include "mdrv_malloc_common.h"

#ifdef __cplusplus
}
#endif
#endif
