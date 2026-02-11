

#ifndef _VFMW_MEM_MANAGE_HEAD_
#define _VFMW_MEM_MANAGE_HEAD_

#include "vfmw.h"

#define MEM_MAN_ERR  (-1)
#define MEM_MAN_OK    0

typedef struct {
	UADDR phy_addr;
	UINT32 length;
	SINT32 is_sec_mem;
	UINT8 *vir_addr;
} mem_record_s;

VOID mem_init_mem_manager(void);

SINT32 mem_add_mem_record(UADDR phy_addr, VOID *vir_addr, UINT32 length);

SINT32 mem_del_mem_record(UADDR phy_addr, const VOID *vir_addr, UINT32 length);

VOID *mem_phy_2_vir(UADDR phy_addr);

UADDR mem_vir_2_phy(UINT8 *vir_addr);

VOID mem_write_phy_word(UADDR phy_addr, UINT32 data_32);

UINT32 mem_read_phy_word(UADDR phy_addr);

SINT32 mem_map_register_addr(
	UADDR reg_start_phy_addr, UINT32 reg_byte_len,
	mem_record_s *p_mem_record);

VOID mem_unmap_register_addr(UADDR phy_addr, UINT8 *vir_addr, UINT32 size);

#endif
