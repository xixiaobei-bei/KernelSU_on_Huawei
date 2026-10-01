#ifndef __VDM_DRV_HEADER__
#define __VDM_DRV_HEADER__
#include "vfmw.h"
#include "sysconfig.h"

#define VDMDRV_OK                 (0)
#define VDMDRV_ERR                (-1)

#define MSG_SLOT_SIZE             (256)

#define LUMA_HISTORGAM_NUM        (32)

#define HEVC_ONE_MSG_SLOT_LEN     (320)  // 64*5
#define MAX_FRAME_NUM             (32)
#define VDH_STREAM_BUF_CNT        (5)

#ifdef VFMW_HEVC_SUPPORT
#define USE_MSG_SLOT_SIZE         HEVC_ONE_MSG_SLOT_LEN
#else
#define USE_MSG_SLOT_SIZE         MSG_SLOT_SIZE
#endif
typedef enum {
	VDH_SHAREFD_MESSAGE_POOL = 0,
	VDH_SHAREFD_STREAM_BUF   = 1,
	VDH_SHAREFD_PMV_BUF      = (VDH_SHAREFD_STREAM_BUF + VDH_STREAM_BUF_CNT),
	VDH_SHAREFD_FRM_BUF,
	VDH_SHAREFD_MAX          = (VDH_SHAREFD_FRM_BUF + MAX_FRAME_NUM)
}VDH_SHAREFD;
typedef enum {
	VDH_STATE_REG   = 1,
	INT_STATE_REG   = 2,
	INT_MASK_REG    = 3,
	VCTRL_STATE_REG = 4,
} REG_ID_E;

typedef enum {
	VDM_IDLE_STATE     = 0,
	VDM_DECODE_STATE   = 1,
	VDM_REPAIR_STATE_0 = 2,
	VDM_REPAIR_STATE_1 = 3
} VDMDRV_STATEMACHINE_E;

typedef enum {
	VDMDRV_SLEEP_STAGE_NONE = 0,
	VDMDRV_SLEEP_STAGE_PREPARE,
	VDMDRV_SLEEP_STAGE_SLEEP
} VDMDRV_SLEEP_STAGE_E;

typedef enum {
	FIRST_REPAIR = 0,
	SECOND_REPAIR
} REPAIRTIME_S;

typedef enum hi_CONFIG_VDH_CMD {
	CONFIG_VDH_AfterDec_CMD = 200,
	CONFIG_VDH_ACTIVATEDEC_CMD
} CONFIG_VDH_CMD;

typedef struct {
	UINT32 vdh_reset_flag;
	UINT32 GlbResetFlag;
	SINT32 VdhStartRepairFlag;
	SINT32 VdhStartHwDecFlag;
	SINT32 VdhBasicCfg0;
	SINT32 VdhBasicCfg1;
	SINT32 VdhAvmAddr;
	SINT32 VdhVamAddr;
	SINT32 VdhStreamBaseAddr;
	SINT32 VdhEmarId;
	SINT32 VdhYstAddr;
	SINT32 VdhYstride;
	SINT32 VdhUvstride;//VREG_UVSTRIDE_1D
	SINT32 VdhCfgInfoAddr;//CFGINFO_ADDR
	SINT32 VdhUvoffset;
	SINT32 VdhRefPicType;
	SINT32 VdhFfAptEn;
	REPAIRTIME_S RepairTime;
	VID_STD_E VidStd;
	SINT32 ValidGroupNum0;
	SINT32 vdh_share_fd[VDH_SHAREFD_MAX];
	UINT32 vdhStreamBufNum;
	UINT32 vdhFrmBufNum;
	HI_BOOL IsFrmBufRemap;
	HI_BOOL IsPmvBufRemap;
	HI_BOOL IsAllBufRemap;
} OMXVDH_REG_CFG_S;

typedef struct {
	// vdm register base vir addr
	SINT32 *pVdmRegVirAddr;

	// vdm hal base addr
	UADDR HALMemBaseAddr;
	SINT32 HALMemSize;
	SINT32 VahbStride;

	/* message pool */
	UADDR MsgSlotAddr[256];
	SINT32 ValidMsgSlotNum;

	/* vlc code table */
	UADDR H264TabAddr;     /* 32 Kbyte */
	UADDR MPEG2TabAddr;    /* 32 Kbyte */
	UADDR MPEG4TabAddr;    /* 32 Kbyte */
	UADDR AVSTabAddr;      /* 32 Kbyte */
	UADDR VC1TabAddr;
	/* cabac table */
	UADDR H264MnAddr;
	/* nei info for vdh for hevc  */
	UADDR  sed_top_phy_addr;
	UADDR  pmv_top_phy_addr;
	UADDR  pmv_left_phy_addr;
	UADDR  rcn_top_phy_addr;
	UADDR  mn_phy_addr;
	UADDR  tile_segment_info_phy_addr;
	UADDR  dblk_left_phy_addr;
	UADDR  dblk_top_phy_addr;
	UADDR  sao_left_phy_addr;
	UADDR  sao_top_phy_addr;
	UADDR  ppfd_phy_addr;
	SINT32 ppfd_buf_len;

	/*nei info for vdh */
	UADDR SedTopAddr;    /* len = 64*4*x */
	UADDR PmvTopAddr;    /* len = 64*4*x */
	UADDR RcnTopAddr;    /* len = 64*4*x */
	UADDR ItransTopAddr;
	UADDR DblkTopAddr;
	UADDR PpfdBufAddr;
	UADDR PpfdBufLen;

	UADDR IntensityConvTabAddr;
	UADDR BitplaneInfoAddr;
	UADDR Vp6TabAddr;
	UADDR Vp8TabAddr;
	
	/* VP9 */
	UADDR DblkLeftAddr;
	UADDR Vp9ProbTabAddr;
	UADDR Vp9ProbCntAddr;

	UINT8 *luma_2d_vir_addr;
	UADDR luma_2d_phy_addr;
	UINT8 *chrom_2d_vir_addr;
	UADDR chrom_2d_phy_addr;
} VDMHAL_HWMEM_S;

typedef struct {
	UINT32 Int_State_Reg;

	UINT32 BasicCfg1;
	UINT32 VdmState;
	UINT32 Mb0QpInCurrPic;
	UINT32 SwitchRounding;
	UINT32 SedSta;
	UINT32 SedEnd0;

	UINT32 DecCyclePerPic;
	UINT32 RdBdwidthPerPic;
	UINT32 WrBdWidthPerPic;
	UINT32 RdReqPerPic;
	UINT32 WrReqPerPic;
	UINT32 LumaSumHigh;
	UINT32 LumaSumLow;
	UINT32 LumaHistorgam[LUMA_HISTORGAM_NUM];
} VDMHAL_BACKUP_S;

extern VDMHAL_HWMEM_S g_HwMem[MAX_VDH_NUM];
#endif
