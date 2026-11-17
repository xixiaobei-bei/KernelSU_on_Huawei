/*
 * ivp atf function header file
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _IVP_ATF_H_
#define _IVP_ATF_H_

#define IVP_SLV_SECMODE     0xC500BB00
#define IVP_MST_SECMODE     0xC500BB01
#define IVP_REMAP_SECMODE   0xC500BB02
#ifdef CONFIG_IVP_SMMU_V3
#define IVP_TBU_CFG_SECMODE 0xC500BB03
#endif

#define MEDIA2_IVP_SID_VALUE             8
#define MEDIA2_IVP0_SSID_VALUE           11
#define MEDIA2_IVP1_SSID_VALUE           12
#define MEDIA2_IVP_SEC_SID_VALUE         11
#define MEDIA2_IVP_SEC_SSID_VALUE        6

enum secmode {
	IVP_SEC = 0,
	IVP_NONSEC
};

int ivpatf_change_slv_secmod(unsigned int core_id, unsigned int mode);
int ivpatf_change_mst_secmod(unsigned int core_id, unsigned int mode);
int ivpatf_poweron_remap_secmod(unsigned int core_id, unsigned int ivp_addr,
	unsigned int len, unsigned long ddr_addr);

#ifdef CONFIG_IVP_SMMU_V3
int ivpatf_tbu_signal_cfg_secmod(unsigned int core_id, unsigned int sid,
	unsigned int ssid, unsigned int secmod);
#endif

#endif /* _IVP_ATF_H_ */
