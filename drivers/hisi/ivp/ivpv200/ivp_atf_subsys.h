#ifndef _IVP_ATF_H_
#define _IVP_ATF_H_

#define IVP_SLV_SECMODE     (0xC500bb00)
#define IVP_MST_SECMODE     (0xC500bb01)
#define IVP_REMAP_SECMODE   (0xC500bb02)

enum secmode {
    IVP_SEC = 0,
    IVP_NONSEC
};

extern int ivpatf_change_slv_secmod(unsigned int mode);
extern int ivpatf_change_mst_secmod(unsigned int mode);
extern int ivpatf_poweron_remap_secmod(unsigned int ivp_addr, unsigned int len, unsigned long ddr_addr);


#endif
