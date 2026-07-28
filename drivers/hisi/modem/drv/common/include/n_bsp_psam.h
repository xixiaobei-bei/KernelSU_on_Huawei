/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _BSP_PSAM_H_
#define _BSP_PSAM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <bsp_shared_ddr.h>
#include "n_bsp_ipf.h"

#define BOSTON_BBIT_PSAM_INTR0	6
#define BOSTON_BBIT_PSAM_INTR1	7
#define BOSTON_BBIT_PSAM_INTR2	8


#define PSAM_BASE_CRG_BAK_ADDR              (unsigned long)((unsigned long)SHM_BASE_ADDR + SHM_OFFSET_PSAM)
#define PSAM_BASE_CRG_BAK_SIZE               256

#define PSAM_ADDR_LIMIT_BAK_ADDR            (PSAM_BASE_CRG_BAK_ADDR + PSAM_BASE_CRG_BAK_SIZE)
#define PSAM_ADDR_LIMIT_BAK_SIZE             64

#define PSAM_SHARE_DDR_END_ADDR				(PSAM_ADDR_LIMIT_BAK_ADDR + PSAM_ADDR_LIMIT_BAK_SIZE)

#define PSAM_DLBD_DESC_SIZE              	IPF_DLBD_DESC_SIZE
#define PSAM_DLBD_RES_NUM					1
#define PSAM_VER_100A	(0x31303061)
#define PSAM_VER_110A	(0x31313061)
#define PSAM_VER_120A	(0x31323061)
#define PSAM_VER_150A	(0x31353061)
#define PSAM_VER_160	(0x31363061)

#define PSAM_ADQ_SETTING_TABLE  	{HI_PSAM_ADQ_CTRL_OFFSET, 0xfffffff0},  \
	{HI_PSAM64_ADQ0_BASE_H_OFFSET, 0xffffffff}, \
	{HI_PSAM64_ADQ0_BASE_L_OFFSET, 0xffffffff}, \
	{HI_PSAM64_ADQ1_BASE_H_OFFSET, 0xffffffff}, \
	{HI_PSAM64_ADQ1_BASE_L_OFFSET, 0xffffffff}, \
	{HI_PSAM_ADQ0_WPTR_OFFSET, 0xffffffff},     \
	{HI_PSAM_ADQ1_WPTR_OFFSET, 0xffffffff}, \
	{HI_PSAM_ADQ0_RPTR_OFFSET, 0xffffffff},     \
	{HI_PSAM_ADQ1_RPTR_OFFSET, 0xffffffff}, \
	{HI_PSAM_ADQ_CTRL_OFFSET, 0xffffffff}, 

enum psam_version{
	PSAM_VERSION_0000  = 0x0,
	PSAM_VERSION_100a  = 0x1,		
	PSAM_VERSION_110a  = 0x2,
	PSAM_VERSION_120a  = 0x3,
	PSAM_VERSION_150a  = 0x4,
	PSAM_VERSION_160   = 0x5,
	PSAM_VERSTION_BOTTOM
};

struct psam_pm_cb
{
    int (*is_idle)(void);
    void (*save_reg)(unsigned int* base);
    void (*restore_reg)(unsigned int* base);
    void (*peer_restore_reg)(unsigned int* base);
	void (*peer_save_reg)(unsigned int* base);
};

/*psam ipf interface structure*/
typedef struct tagpsam_ipf_reg
{
    struct ipf_desc_handler_s * desc_hd;    
#ifndef __KERNEL__
    void* dlbdq_addr;
	unsigned int dlbdq_wptr_addr;
	unsigned int dlbdq_rptr_addr;
	void *dlcdq;
	unsigned int *dlcdq_wptr;
	unsigned int *dlcdq_rptr;
	int (*config_bd)(IPF_CONFIG_DLPARAM_S * dl_para,void * bd_base,unsigned int index, unsigned int wake);
#endif
}ipf_reg_s;

#ifdef __KERNEL__

typedef int (*adq_emtpy_cb_t)(unsigned int);

#ifdef CONFIG_PSAM_SUPPORT
struct psam_pm_cb* bsp_psam_set_ipf_para(struct tagpsam_ipf_reg *para);
int psam_config_dlad(unsigned int type, unsigned int num, IPF_AD_DESC_S *desc);
int psam_get_dlad_num(unsigned int* ad0_num, unsigned int* ad1_num);
int psam_register_adq_empty_dlcb(adq_emtpy_cb_t adq_emtpy_cb);
int bsp_psam_idle(void);
int psam_reinit_regs(void);
int bsp_psam_get_used_dlad(IPF_AD_TYPE_E ad_type, unsigned int * ad_num, IPF_AD_DESC_S * pst_ad_desc);
#else
struct psam_pm_cb* bsp_psam_set_ipf_para(struct tagpsam_ipf_reg *para)
{
	return 0;
}

static inline int psam_config_dlad(unsigned int type, unsigned int num, IPF_AD_DESC_S *desc)
{
	return 0;
}
static inline int psam_get_dlad_num(unsigned int* ad0_num, unsigned int* ad1_num)
{
	return 0;
}
static inline int psam_register_adq_empty_dlcb(adq_emtpy_cb_t adq_emtpy_cb)
{
	return 0;
}
static inline int bsp_psam_idle(void)
{
	return 0;
}
static inline int psam_reinit_regs(void)
{
	return 0;
}
#endif
	
#elif defined(__OS_VXWORKS__) || defined(__OS_RTOSCK__)||defined(__OS_RTOSCK_SMP__)

/*psam cipher interface structure tagPSAM_DL_S*/
typedef struct tagpsam_cipher_reg
{
    unsigned int ch_id;
	unsigned int cbdq_config;
	unsigned int cbdq_baddr_l;
	unsigned int cbdq_baddr_h;
	unsigned int cbdq_size;
	unsigned int cbdq_wptr_h_addr;
	unsigned int cbdq_wptr_l_addr;
	unsigned int crdq_rptr_h_addr;
	unsigned int crdq_rptr_l_addr;
	struct ipf_desc_handler_s* desc_hd;
}cipher_reg_s;

/*psam cipher interface structrue*/
typedef struct tagcipher_addr_reg
{
    unsigned int ps_id;
	void 			*direct_addr;
	unsigned int 	cnt;
}cipher_addr_s;



#ifdef CONFIG_PSAM
int bsp_psam_get_cipher_bd(struct tagpsam_cipher_reg * param);
int bsp_psam_config_cipher_rd(int psam_handle, cipher_addr_s *para);
//int psam_get_crdq_ptr(int psam_handle);
//unsigned int psam_get_cipher_busy(int psam_handle);
int bsp_psam_move_cbdq_ptr(int psam_handle, unsigned int bdq_ptr);
//int psam_reset_cbdq_crdq_ptr(int psam_handle);
int	bsp_psam_cipher_ch_srst(int psam_handle);
int bsp_psam_config_dlbd(unsigned int u32Num, IPF_CONFIG_PARAM_S* dl_para);
unsigned int bsp_psam_get_dlbd_num(unsigned int* cd_num);
int psam_init(void);
struct psam_pm_cb* bsp_psam_set_ipf_para(struct tagpsam_ipf_reg *para);
void bsp_psam_config_adthred(unsigned int u32DlADThr);
int bsp_psam_idle_status(void);
int bsp_psam_cbdq_idle(void);
void psam_clear_ptr(void);
void bsp_psam_stop(void);
#else
static inline int bsp_psam_get_cipher_bd(struct tagpsam_cipher_reg * param)
{
	return 0;
}

static inline int bsp_psam_config_cipher_rd(int psam_handle, cipher_addr_s *para)
{
	return 0;
}
/*
static inline unsigned int psam_get_crdq_ptr(void)
{
	return 0;
}

static inline unsigned int psam_get_cipher_busy(void)
{
	return 0;
}
*/
static inline int bsp_psam_move_cbdq_ptr(unsigned int chn, unsigned int bdq_ptr)
{
	return 0;
}

static inline void psam_reset_cbdq_crdq_ptr(void)
{

}

static inline int bsp_psam_cipher_ch_srst(int psam_handle)
{
	return 0;
}

static inline int bsp_psam_config_dlbd(unsigned int u32Num, IPF_CONFIG_PARAM_S* dl_para)
{
	return 0;
}

static inline unsigned int bsp_psam_get_dlbd_num(unsigned int* cd_num)
{
	return 0;
}

static inline int psam_init(void)
{
	return 0;
}

static inline struct psam_pm_cb* bsp_psam_set_ipf_para(struct tagpsam_ipf_reg *para)
{
	return 0;
}

static inline void bsp_psam_config_adthred(unsigned int u32DlADThr)
{
	
}
static inline int bsp_psam_idle_status(void)
{
	return 0;
}

static inline int bsp_psam_cbdq_idle(void)
{
    return 1;
}
#endif
#endif

#ifdef __CMSIS_RTOS
int	bsp_psam_init(void);
int psam_bakup_reg(void);
void psam_restore_reg(void);
#endif


#ifdef __cplusplus
} 
#endif

#endif
