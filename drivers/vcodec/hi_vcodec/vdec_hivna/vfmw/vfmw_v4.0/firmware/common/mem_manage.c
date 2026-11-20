/*
 * mem_manage.c
 *
 * This is vdec mem_manager.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "mem_manage.h"
#include "vfmw_osal_ext.h"
#include "basedef.h"
#include "public.h"

#define    MAX_MEM_MAN_RECORD_NUM    (MAX_CHAN_NUM * 32)
mem_record_s  g_mem_record[MAX_MEM_MAN_RECORD_NUM];

void mem_init_mem_manager(void)
{
	SINT32 ret;
	VFMW_OSAL_SPIN_LOCK(G_SPINLOCK_RECORD);

	ret = memset_s(g_mem_record, sizeof(g_mem_record), 0, sizeof(g_mem_record));
	if (ret != EOK)
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);

	VFMW_OSAL_SPIN_UNLOCK(G_SPINLOCK_RECORD);
}

SINT32 mem_add_mem_record(UADDR phy_addr, void *vir_addr, UINT32 length)
{
	SINT32 i;
	SINT8 is_error_flag = 0;
	SINT32 target_pos  = -1;
	SINT32 ret        = MEM_MAN_ERR;

	if (vir_addr == NULL) {
		dprint(PRN_FATAL, "intput param is null\n");
		return ret;
	}

	VFMW_OSAL_SPIN_LOCK(G_SPINLOCK_RECORD);

	for (i = 0; i < MAX_MEM_MAN_RECORD_NUM; i++) {
		if ((g_mem_record[i].phy_addr <= phy_addr) &&
			(phy_addr < g_mem_record[i].phy_addr +
			g_mem_record[i].length)) {
			dprint(PRN_ERROR, "phy [%pK ~ %pK] overlap with Record[%d] [%pK ~ %pK]\n",
				   (void *)(uintptr_t)phy_addr,
				   (void *)(uintptr_t)(phy_addr + length), i,
				   (void *)(uintptr_t)(g_mem_record[i].phy_addr),
				   (void *)(uintptr_t)(g_mem_record[i].phy_addr +
				   g_mem_record[i].length));
			is_error_flag = 1;
			break;
		}

		if (g_mem_record[i].length == 0 && target_pos == -1)
			target_pos = i;
	}

	if (is_error_flag == 1) {
		dprint(PRN_FATAL, "%s conflict occured\n ", __func__);
		ret = MEM_MAN_ERR;
	} else if (target_pos == -1) {
		dprint(PRN_FATAL, "%s no free record slot\n ", __func__);
		ret = MEM_MAN_ERR;
	} else {
		g_mem_record[target_pos].phy_addr = phy_addr;
		g_mem_record[target_pos].vir_addr = vir_addr;
		g_mem_record[target_pos].length  = length;
		ret = MEM_MAN_OK;
	}

	VFMW_OSAL_SPIN_UNLOCK(G_SPINLOCK_RECORD);

	return ret;
}

SINT32 mem_del_mem_record(UADDR phy_addr, const void *vir_addr, UINT32 length)
{
	SINT32 i;

	if (vir_addr == NULL) {
		dprint(PRN_FATAL, "%s vir_addr is NULL", __func__);
		return MEM_MAN_ERR;
	}

	VFMW_OSAL_SPIN_LOCK(G_SPINLOCK_RECORD);
	for (i = 0; i < MAX_MEM_MAN_RECORD_NUM; i++) {
		if (g_mem_record[i].length == 0)
			continue;

		if (phy_addr == g_mem_record[i].phy_addr &&
			vir_addr == g_mem_record[i].vir_addr &&
			length == g_mem_record[i].length) {
			g_mem_record[i].length  = 0;
			g_mem_record[i].phy_addr = 0;
			g_mem_record[i].vir_addr = 0;
			VFMW_OSAL_SPIN_UNLOCK(G_SPINLOCK_RECORD);

			return MEM_MAN_OK;
		}
	}
	VFMW_OSAL_SPIN_UNLOCK(G_SPINLOCK_RECORD);

	return MEM_MAN_ERR;
}

void *mem_phy_2_vir(UADDR phy_addr)
{
	UINT32 i;
	UINT8 *vir_addr = NULL;

	for (i = 0; i < MAX_MEM_MAN_RECORD_NUM; i++) {
		if (g_mem_record[i].length == 0)
			continue;

		if ((phy_addr >= g_mem_record[i].phy_addr) &&
				(phy_addr < g_mem_record[i].phy_addr +
				g_mem_record[i].length)) {
			vir_addr = g_mem_record[i].vir_addr +
				(phy_addr - g_mem_record[i].phy_addr);
			break;
		}
	}

	return (void *) vir_addr;
}

UADDR mem_vir_2_phy(UINT8 *vir_addr)
{
	UINT32 i;

	UADDR phy_addr = 0;

	if (vir_addr == NULL) {
		dprint(PRN_FATAL, "%s vir_addr is NULL", __func__);
		return 0;
	}
	for (i = 0; i < MAX_MEM_MAN_RECORD_NUM; i++) {
		if (g_mem_record[i].length == 0)
			continue;

		if ((vir_addr >= g_mem_record[i].vir_addr) &&
			(vir_addr < g_mem_record[i].vir_addr +
			g_mem_record[i].length)) {
			phy_addr = g_mem_record[i].phy_addr +
				(vir_addr - g_mem_record[i].vir_addr);
			break;
		}
	}

	return phy_addr;
}

void mem_write_phy_word(UADDR phy_addr, UINT32 data_32)
{
	UINT32 *dst;

	dst = (UINT32 *) mem_phy_2_vir(phy_addr);
	if (dst != NULL)
		writel(data_32, dst);
}

UINT32 mem_read_phy_word(UADDR phy_addr)
{
	UINT32 *dst;
	UINT32 data_32;

	data_32 = 0;
	dst = (UINT32 *) mem_phy_2_vir(phy_addr);
	if (dst != NULL)
		data_32 = readl((volatile UINT32 *)dst);

	return data_32;
}

SINT32  mem_map_register_addr(
	UADDR reg_start_phy_addr, UINT32 reg_byte_len,
	mem_record_s *mem_record)
{
	UINT8 *ptr = NULL;
	UINT32 ret;

	if (mem_record == NULL || reg_start_phy_addr == 0 ||
		reg_byte_len == 0 || VFMW_OSAL_REGISTER_MAP == NULL)
		return MEM_MAN_ERR;

	ret = memset_s(mem_record, sizeof(*mem_record), 0, sizeof(*mem_record));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return MEM_MAN_ERR;
	}

	ptr = VFMW_OSAL_REGISTER_MAP(reg_start_phy_addr, reg_byte_len);
	if (ptr != NULL) {
		mem_record->phy_addr = reg_start_phy_addr;
		mem_record->vir_addr = ptr;
		mem_record->length = reg_byte_len;
		return MEM_MAN_OK;
	}

	return MEM_MAN_ERR;
}

void mem_unmap_register_addr(UADDR phy_addr, UINT8 *vir_addr, UINT32 size)
{
	if (phy_addr == 0 || vir_addr == 0 || VFMW_OSAL_REGISTER_UNMAP == NULL)
		return;

	VFMW_OSAL_REGISTER_UNMAP(vir_addr, size);
}
