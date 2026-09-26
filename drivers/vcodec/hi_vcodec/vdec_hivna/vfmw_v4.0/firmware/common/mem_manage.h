#ifndef _VFMW_MEM_MANAGE_HEAD_
#define _VFMW_MEM_MANAGE_HEAD_

#include "vfmw.h"

#define MEM_MAN_ERR  -1
#define MEM_MAN_OK    0

typedef struct {
	UADDR PhyAddr;
	UINT32 Length;
	SINT32 IsSecMem;
	UINT8 *VirAddr;
} MEM_RECORD_S;

VOID MEM_InitMemManager(VOID);

SINT32 MEM_AddMemRecord(UADDR PhyAddr, VOID *VirAddr, UINT32 Length);

SINT32 MEM_DelMemRecord(UADDR PhyAddr, VOID *VirAddr, UINT32 Length);

VOID *MEM_Phy2Vir(UADDR PhyAddr);

UADDR MEM_Vir2Phy(UINT8 *VirAddr);

VOID MEM_WritePhyWord(UADDR PhyAddr, UINT32 Data32);

UINT32 MEM_ReadPhyWord(UADDR PhyAddr);

SINT32 MEM_MapRegisterAddr(UADDR RegStartPhyAddr, UINT32 RegByteLen, MEM_RECORD_S *pMemRecord);

VOID MEM_UnmapRegisterAddr(UADDR PhyAddr, UINT8 *VirAddr, UINT32 Size);

#endif
