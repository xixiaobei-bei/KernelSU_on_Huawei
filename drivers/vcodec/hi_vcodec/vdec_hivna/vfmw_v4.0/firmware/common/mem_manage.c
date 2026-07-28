/*
 * vdec mem_manager
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */
#include "mem_manage.h"
#include "sysconfig.h"
#include "vfmw_osal_ext.h"
#include "basedef.h"
#include "public.h"

#define    MAX_MEM_MAN_RECORD_NUM    (MAX_CHAN_NUM*32)
MEM_RECORD_S  s_MemRecord[MAX_MEM_MAN_RECORD_NUM];

VOID MEM_InitMemManager(VOID)
{
	VFMW_OSAL_SpinLock(G_SPINLOCK_RECORD);

	memset(s_MemRecord, 0, sizeof(s_MemRecord));

	VFMW_OSAL_SpinUnLock(G_SPINLOCK_RECORD);

}

SINT32 MEM_AddMemRecord(UADDR PhyAddr, VOID *VirAddr, UINT32 Length)
{
	SINT32 i;
	SINT8 IsErrorFlag = 0;
	SINT32 TargetPos  = -1;
	SINT32 ret        = MEM_MAN_ERR;

	VFMW_OSAL_SpinLock(G_SPINLOCK_RECORD);

	for (i = 0; i < MAX_MEM_MAN_RECORD_NUM; i++) {
		if ((s_MemRecord[i].PhyAddr <= PhyAddr) && (PhyAddr < s_MemRecord[i].PhyAddr + s_MemRecord[i].Length)) {
			dprint(PRN_ERROR, "%s : Phy [%pK ~ %pK] overlap with Record[%d] [%pK ~ %pK]\n",
				__func__, (void *)(uintptr_t)PhyAddr, (void *)(uintptr_t)(PhyAddr + Length), i,
				(void *)(uintptr_t)(s_MemRecord[i].PhyAddr), (void *)(uintptr_t)(s_MemRecord[i].PhyAddr + s_MemRecord[i].Length));
			IsErrorFlag = 1;
			break;
		}

		if (s_MemRecord[i].Length == 0 && TargetPos == -1)
			TargetPos = i;
	}

	if (IsErrorFlag == 1) {
		dprint(PRN_FATAL, "%s conflict occured\n ", __func__);
		ret = MEM_MAN_ERR;
	} else if (TargetPos == -1) {
		dprint(PRN_FATAL, "%s no free record slot\n ", __func__);
		ret = MEM_MAN_ERR;
	} else {
		s_MemRecord[TargetPos].PhyAddr = PhyAddr;
		s_MemRecord[TargetPos].VirAddr = VirAddr;
		s_MemRecord[TargetPos].Length  = Length;
		ret = MEM_MAN_OK;
	}

	VFMW_OSAL_SpinUnLock(G_SPINLOCK_RECORD);

	return ret;
}

SINT32 MEM_DelMemRecord(UADDR PhyAddr, VOID *VirAddr, UINT32 Length)
{
	SINT32 i;

	VFMW_OSAL_SpinLock(G_SPINLOCK_RECORD);
	for (i = 0; i < MAX_MEM_MAN_RECORD_NUM; i++) {
		if (s_MemRecord[i].Length == 0)
			continue;

		if (PhyAddr == s_MemRecord[i].PhyAddr && VirAddr == s_MemRecord[i].VirAddr &&
		    Length == s_MemRecord[i].Length) {
			s_MemRecord[i].Length  = 0;
			s_MemRecord[i].PhyAddr = 0;
			s_MemRecord[i].VirAddr = 0;
			VFMW_OSAL_SpinUnLock(G_SPINLOCK_RECORD);

			return MEM_MAN_OK;
		}
	}
	VFMW_OSAL_SpinUnLock(G_SPINLOCK_RECORD);

	return MEM_MAN_ERR;
}

VOID *MEM_Phy2Vir(UADDR PhyAddr)
{
	UINT32 i;
	UINT8 *VirAddr = NULL;

	for (i = 0; i < MAX_MEM_MAN_RECORD_NUM; i++) {
		if (s_MemRecord[i].Length == 0)
			continue;

		if ((PhyAddr >= s_MemRecord[i].PhyAddr) && (PhyAddr < s_MemRecord[i].PhyAddr + s_MemRecord[i].Length)) {
			VirAddr = s_MemRecord[i].VirAddr + (PhyAddr - s_MemRecord[i].PhyAddr);
			break;
		}
	}

	return (VOID *) VirAddr;
}

UADDR MEM_Vir2Phy(UINT8 *VirAddr)
{
	UINT32 i;

	UADDR PhyAddr = 0;
	for (i = 0; i < MAX_MEM_MAN_RECORD_NUM; i++) {
		if (s_MemRecord[i].Length == 0)
			continue;

		if ((VirAddr >= s_MemRecord[i].VirAddr) && (VirAddr < s_MemRecord[i].VirAddr + s_MemRecord[i].Length)) {
			PhyAddr = s_MemRecord[i].PhyAddr + (VirAddr - s_MemRecord[i].VirAddr);
			break;
		}
	}

	return PhyAddr;
}

VOID MEM_WritePhyWord(UADDR PhyAddr, UINT32 Data32)
{
	UINT32 *pDst;

	pDst = (UINT32 *) MEM_Phy2Vir(PhyAddr);
	if (pDst != NULL)
		writel(Data32, pDst);
}

UINT32 MEM_ReadPhyWord(UADDR PhyAddr)
{
	UINT32 *pDst;
	UINT32 Data32;

	Data32 = 0;
	pDst = (UINT32 *) MEM_Phy2Vir(PhyAddr);
	if (pDst != NULL)
		Data32 = readl((volatile UINT32 *)pDst);

	return Data32;
}

SINT32  MEM_MapRegisterAddr(UADDR RegStartPhyAddr, UINT32 RegByteLen, MEM_RECORD_S *pMemRecord)
{
	UINT8 *ptr;

	if (pMemRecord == NULL || RegStartPhyAddr == 0 || RegByteLen == 0 || VFMW_OSAL_RegisterMap == NULL)
		return MEM_MAN_ERR;

	memset(pMemRecord, 0, sizeof(*pMemRecord));

	ptr = VFMW_OSAL_RegisterMap(RegStartPhyAddr, RegByteLen);

	if (ptr != NULL) {
		pMemRecord->PhyAddr = RegStartPhyAddr;
		pMemRecord->VirAddr = ptr;
		pMemRecord->Length = RegByteLen;
		return MEM_MAN_OK;
	}

	return MEM_MAN_ERR;
}

VOID MEM_UnmapRegisterAddr(UADDR PhyAddr, UINT8 *VirAddr, UINT32 Size)
{
	if (PhyAddr == 0 || VirAddr == 0 || VFMW_OSAL_RegisterUnMap == NULL)
		return;

	VFMW_OSAL_RegisterUnMap(VirAddr, Size);

	return;
}
