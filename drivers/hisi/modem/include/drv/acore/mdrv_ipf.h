/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: eipf driver
 * Author: shennaiyu
 * Create: 2020-10-12
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

/**
 *  @brief   765ç­‰è€å¹³å°ipf acoreå¯¹å¤–å¤´æ–‡ä»¶ï¼Œ5010èåˆæ¶æ„ä¸å¯¹å¤–æä¾›ã€‚
 *  @file    mdrv_ipf.h
 *  @author  zhangsanmao
 *  @version v1.0
 *  @date    2019.12.17
 *  @note    ä¿®æ”¹è®°å½•(ç‰ˆæœ¬å·|ä¿®è®¢æ—¥æœŸ|è¯´æ˜)
 *  <ul><li>v1.0|2019.12.17|å¢åŠ è¯´æ˜ï¼šè¯¥å¤´æ–‡ä»¶5010èåˆæ¶æ„ä¸å¯¹å¤–æä¾›</li></ul>
 *  @since
 */

#ifndef __MDRV_ACORE_IPF_H__
#define __MDRV_ACORE_IPF_H__
#include "mdrv_ipf_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_NEW_PLATFORM
typedef void *modem_phy_addr;

typedef struct {
    unsigned short len;     /* ÉÏĞĞÊı¾İÔ´³¤¶È */
    unsigned short int_en;  /* ÖĞ¶ÏÊ¹ÄÜ */
    unsigned short fc_head; /* ÉÏĞĞIP¹ıÂËÆ÷Á´Í·ºÅ */
    unsigned short mode;
    modem_phy_addr data;       /* ÉÏĞĞÊı¾İÔ´µØÖ· */
    unsigned short usr_field1; /* Usr field Óò1 ,Í¸´« */
    unsigned int usr_field2;   /* Usr field Óò2 ,Í¸´« */
    unsigned int usr_field3;   /* Usr field Óò3 ,Í¸´« */
} ipf_config_param_s;

/* RDÃèÊö·û */
typedef struct {
    unsigned short attribute;
    unsigned short fc_head;
    unsigned short pkt_len;
    modem_phy_addr in_ptr;
    modem_phy_addr out_ptr;
    unsigned short result;
    unsigned short usr_field1;
    unsigned int usr_field2;
    unsigned int usr_field3;
} ipf_rd_desc_s;

/* ADÃèÊö·û */
typedef struct tagipf_ad_desc_s {
    modem_phy_addr out_ptr0;
    modem_phy_addr out_ptr1;
} ipf_ad_desc_s;

#endif

typedef ipf_config_param_s ipf_config_ulparam_s;
typedef ipf_config_param_s ipf_config_dlparam_s;

typedef int (*bsp_ipf_wakeup_dl_cb)(void);
typedef int (*bsp_ipf_adq_empty_dl_cb)(ipf_adq_empty_e adq_empty);

#ifdef CONFIG_IPF
/*****************************************************************************
 * º¯ Êı Ãû  : mdrv_ipf_config_ulbd
 *
 * ¹¦ÄÜÃèÊö  : ÅäÖÃÉÏĞĞÊı¾İ
 *
 * ÊäÈë²ÎÊı  : unsigned int u32Num     ĞèÒªÅäÖÃµÄBDÊıÄ¿
 *             ipf_config_ulparam_s* pstUlPara ÅäÖÃ²ÎÊı½á¹¹ÌåÊı×éÖ¸Õë
 * Êä³ö²ÎÊı  :
 *
 * ·µ »Ø Öµ  : IPF_SUCCESS ÅäÖÃ³É¹¦
 *             IPF_ERROR ÅäÖÃÊ§°Ü
 * ÆäËüËµÃ÷  :
 *
 *****************************************************************************/
int mdrv_ipf_config_ulbd(unsigned int num, ipf_config_ulparam_s *ul_para);

/*****************************************************************************
 * º¯ Êı Ãû  : mdrv_ipf_get_ulbd_num
 *
 * ¹¦ÄÜÃèÊö  : »ñÈ¡ÉÏĞĞBDQÖĞ¿ÕÏĞµÄBDÊı
 *
 * ÊäÈë²ÎÊı  :
 * Êä³ö²ÎÊı  :
 *
 * ·µ »Ø Öµ  : ¿ÕÏĞµÄBDÊıÄ¿£¬·¶Î§Îª0~BD_DESC_SIZE
 *
 * ÆäËüËµÃ÷  :
 *
 *****************************************************************************/
unsigned int mdrv_ipf_get_ulbd_num(void);

/*****************************************************************************
 * º¯ Êı Ãû  : mdrv_ipf_get_uldesc_num
 *
 * ¹¦ÄÜÃèÊö  : »ñÈ¡ÉÏĞĞ¿ÉÒÔÅäÖÃµÄ°üÊı
 *
 * ÊäÈë²ÎÊı  : void
 * Êä³ö²ÎÊı  :
 *
 * ·µ »Ø Öµ  : ²»»áÔì³É¶ÓÍ·¶ÂÈûµÄ¿ÉÉÏ´«µÄ°üÊı
 *
 * ÆäËüËµÃ÷  :
 *
 *****************************************************************************/
unsigned int mdrv_ipf_get_uldesc_num(void);

/*****************************************************************************
 * º¯ Êı Ãû  : mdrv_ipf_get_ulrd_num
 *
 * ¹¦ÄÜÃèÊö  : »ñÈ¡ÉÏĞĞRDÊıÄ¿
 *
 * ÊäÈë²ÎÊı  : void
 * Êä³ö²ÎÊı  :
 *
 * ·µ »Ø Öµ  : ÉÏĞĞRdÊıÄ¿
 *
 * ÆäËüËµÃ÷  :
 *
 *****************************************************************************/
unsigned int mdrv_ipf_get_ulrd_num(void);

/*****************************************************************************
 * º¯ Êı Ãû  : mdrv_ipf_get_dlrd
 *
 * ¹¦ÄÜÃèÊö  : »ñÈ¡ÏÂĞĞRD
 *
 * ÊäÈë²ÎÊı  : unsigned int* pu32Num  ¿ÉÒÔ·µ»ØµÄ×î¶àµÄRDÊıÄ¿
 *             ipf_rd_desc_s *pstRd  RDÃèÊö·û½á¹¹ÌåÊ×µØÖ·
 * Êä³ö²ÎÊı  :
 *
 * ·µ »Ø Öµ  : Êµ¼Ê·µ»ØµÄRDÊıÄ¿
 *
 * ÆäËüËµÃ÷  :
 *
 *****************************************************************************/
void mdrv_ipf_get_dlrd(unsigned int *num, ipf_rd_desc_s *rd);

/*****************************************************************************
 * º¯ Êı Ãû  : mdrv_ipf_get_dlrd_num
 *
 * ¹¦ÄÜÃèÊö  : »ñÈ¡ÏÂĞĞRDÊıÄ¿
 *
 * ÊäÈë²ÎÊı  : void
 * Êä³ö²ÎÊı  :
 *
 * ·µ »Ø Öµ  : ÏÂĞĞRdÊıÄ¿
 *
 * ÆäËüËµÃ÷  :
 *
 *****************************************************************************/
unsigned int mdrv_ipf_get_dlrd_num(void);

/*****************************************************************************
 * º¯ Êı Ãû  : mdrv_ipf_config_dlad
 *
 * ¹¦ÄÜÃèÊö  : ¸ø¿ÕÏĞµÄAD·ÖÅäĞÂµÄÄÚ´æ»º³åÇø
 *
 * ÊäÈë²ÎÊı  : ipf_ad_type_e eAdType   AD¶ÓÁĞºÅ£¬0£¨¶Ì°ü¶ÓÁĞ£©»ò1£¨³¤°ü¶ÔÁĞ£©
 *             unsigned int u32AdNum    ĞèÒªÅäÖÃµÄµÄADÊıÄ¿
 *             ipf_ad_desc_s *pstAdDesc »º´æ¿Õ¼ä¶ÔÓ¦µÄÊı¾İ½á¹¹Ê×µØÖ·
 * Êä³ö²ÎÊı  :
 *
 * ·µ »Ø Öµ  : IPF_SUCCESS ÅäÖÃ³É¹¦
 *             IPF_INVALID_PARA ÊäÈë²ÎÊıÎŞĞ§
 *
 * ÆäËüËµÃ÷  :
 *
 *****************************************************************************/
int mdrv_ipf_config_dlad(ipf_ad_type_e ad_type, unsigned int ad_num, ipf_ad_desc_s *ad_desc);

/*****************************************************************************
 * º¯ Êı Ãû  : mdrv_ipf_get_dlad_num
 *
 * ¹¦ÄÜÃèÊö  : »ñÈ¡ÏÂĞĞ£¨AºË£©¿ÕÏĞ£¨Ö¸ÏòµÄ»º³åÇøÒÑ¾­±»Ê¹ÓÃ£©ADÊıÄ¿
 *
 * ÊäÈë²ÎÊı  : unsigned int* pu32AD0Num  ·µ»Ø¿ÕÏĞµÄAD0µÄÊıÄ¿
 *             unsigned int* pu32AD1Num  ·µ»Ø¿ÕÏĞµÄAD1µÄÊıÄ¿
 * Êä³ö²ÎÊı  :
 *
 * ·µ »Ø Öµ  : IPF_SUCCESS ÅäÖÃ³É¹¦
 *             IPF_INVALID_PARA ÊäÈë²ÎÊıÎŞĞ§
 * ÆäËüËµÃ÷  :
 *
 *****************************************************************************/
int mdrv_ipf_get_dlad_num(unsigned int *ad0_num, unsigned int *ad1_num);

/*****************************************************************************
 * º¯ Êı Ãû  : mdrv_ipf_reinit_dlreg
 *
 * ¹¦ÄÜÃèÊö  : CCoreµ¥¶À¸´Î»»Ö¸´Ê±£¬ÅäÖÃIPF¼Ä´æÆ÷
 *
 * ÊäÈë²ÎÊı  : void
 * Êä³ö²ÎÊı  :
 *
 * ·µ »Ø Öµ  :
 *
 * ÆäËüËµÃ÷  :
 *
 *****************************************************************************/
void mdrv_ipf_reinit_dlreg(void);

/*****************************************************************************
 * º¯ Êı Ãû  : mdrv_ipf_get_used_dlad
 *
 * ¹¦ÄÜÃèÊö  : »ñÈ¡Î´±»Ê¹ÓÃµÄÏÂĞĞADÒÔ±ãADSÊÍ·Å
 *
 * ÊäÈë²ÎÊı  : ipf_ad_type_e eAdType  ADÀàĞÍ£º0£¨¶Ì°ü¶ÓÁĞ£©»ò1£¨³¤°ü¶ÔÁĞ£©
 *             unsigned int * pu32AdNum    ADÊıÄ¿
 *             ipf_ad_desc_s * pstAdDesc ĞèÒªÊÍ·ÅµÄADÊı×éÊ×µØÖ·
 * Êä³ö²ÎÊı  :
 *
 * ·µ »Ø Öµ  : IPF_SUCCESS£ºÕı³£·µ»Ø
 *             IPF_ERROR£ºÊ§°Ü·µ»Ø£¬µÈ´ı³¬Ê±IPFÍ¨µÀÊ¼ÖÕBUSY
 *             IPF_INVALID_PARA£ºÈë²Î³ö´í
 * ÆäËüËµÃ÷  : ADSÔÚccoreµ¥¶À¸´Î»Á÷³ÌÖĞĞèÒªµ÷ÓÃ¸Ãº¯Êı²¢ÊÍ·Å·µ»ØµÄAD£¬·ñÔò»áÔì³ÉÄÚ´æĞ¹Â¶
 *
 *****************************************************************************/
int mdrv_ipf_get_used_dlad(ipf_ad_type_e ad_type, unsigned int *ad_num, ipf_ad_desc_s *ad_esc);
#else
static inline int mdrv_ipf_config_ulbd(unsigned int num, ipf_config_ulparam_s *ul_para)
{
    return 0;
}

static inline unsigned int mdrv_ipf_get_ulbd_num(void)
{
    return 0;
}

static inline unsigned int mdrv_ipf_get_uldesc_num(void)
{
    return 0;
}

static inline unsigned int mdrv_ipf_get_ulrd_num(void)
{
    return 0;
}

static inline void mdrv_ipf_get_dlrd(unsigned int *num, ipf_rd_desc_s *rd)
{
    return;
}

static inline unsigned int mdrv_ipf_get_dlrd_num(void)
{
    return 0;
}

static inline int mdrv_ipf_config_dlad(ipf_ad_type_e ad_type, unsigned int ad_num, ipf_ad_desc_s *ad_desc)
{
    return 0;
}

static inline int mdrv_ipf_get_dlad_num(unsigned int *ad0_num, unsigned int *ad1_num)
{
    return 0;
}

static inline void mdrv_ipf_reinit_dlreg(void)
{
    return;
}

static inline int mdrv_ipf_get_used_dlad(ipf_ad_type_e ad_type, unsigned int *ad_num, ipf_ad_desc_s *ad_desc)
{
    return 0;
}
#endif
#ifdef __cplusplus
}
#endif

#endif
