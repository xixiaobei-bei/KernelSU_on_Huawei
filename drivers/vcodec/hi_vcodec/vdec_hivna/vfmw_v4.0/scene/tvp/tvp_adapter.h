#ifndef __TVP_ADAPTER_H__
#define __TVP_ADAPTER_H__

#ifdef ENV_ARMLINUX_KERNEL
HI_S32  TVP_VDEC_SecureInit(VOID);
HI_S32  TVP_VDEC_SecureExit(VOID);
HI_S32  TVP_VDEC_Suspend(VOID);
HI_S32  TVP_VDEC_Resume(VOID);
#endif

#endif //end of __TVP_ADAPTER_H__

