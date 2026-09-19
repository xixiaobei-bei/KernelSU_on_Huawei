

#ifndef __VDEC_MEMORY_H__
#define	__VDEC_MEMORY_H__

#include "hi_type.h"

typedef enum {
	SCENE_VIDEO,
	SCENE_HEIF,
	SCENE_BUTT
} VDEC_SCENE;

typedef struct {
	HI_U8      u8IsMapVirtual;
	HI_U8      u8IsMapped;
	HI_U32     u32ShareFd;
	UADDR      startPhyAddr;
	HI_U32     u32Size;
	VDEC_SCENE scene;
	HI_VOID    *pStartVirAddr;
} MEM_BUFFER_S;

HI_S32   VDEC_MEM_Probe(HI_VOID);
HI_S32   VDEC_MEM_Init(HI_VOID);
HI_S32   VDEC_MEM_Exit(HI_VOID);
HI_S32   VDEC_MEM_MapKernel(HI_S32 share_fd, MEM_BUFFER_S *psMBuf);
HI_S32   VDEC_MEM_UnmapKernel(MEM_BUFFER_S *psMBuf);

HI_S32   VDEC_MEM_GetMapInfo(HI_S32 share_fd, MEM_BUFFER_S *psMBuf);
HI_S32   VDEC_MEM_PutMapInfo(MEM_BUFFER_S *psMBuf);
HI_S32   VDEC_MEM_IommuMap(HI_S32 share_fd, HI_U32 size, UADDR *iova);
HI_S32   VDEC_MEM_IommuUnmap(HI_S32 share_fd, UADDR iova);

#endif

