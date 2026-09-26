/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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
 *  @brief   wifiç›¸å…³å¯¹å¤–å¤´æ–‡ä»¶ï¼Œ5010èåˆæ¶æ„ä¸å¯¹å¤–æä¾›ã€‚
 *  @file    mdrv_wifi.h
 *  @author  zhangsanmao
 *  @version v1.0
 *  @date    2019.12.17
 *  @note    ä¿®æ”¹è®°å½•(ç‰ˆæœ¬å·|ä¿®è®¢æ—¥æœŸ|è¯´æ˜)
 *  <ul><li>v1.0|2019.12.17|å¢åŠ è¯´æ˜ï¼šè¯¥å¤´æ–‡ä»¶5010èåˆæ¶æ„ä¸å¯¹å¤–æä¾›</li></ul>
 *  @since
 */


#ifndef __MDRV_ACORE_WIFI_H__
#define __MDRV_ACORE_WIFI_H__

/*************************WIFI START**********************************/

extern unsigned short BSP_WifiCalDataLen(unsigned short usLen);
#define DRV_WifiCalDataLen(usLen) BSP_WifiCalDataLen(usLen)

/*****************************************************************************
 º¯ Êı Ãû  : WIFI_TEST_CMD
 ¹¦ÄÜÃèÊö  : ²âÊÔÃüÁî
 ÊäÈë²ÎÊı  : cmdStr£»ÃüÁî×Ö·û´®
 Êä³ö²ÎÊı  : ÎŞ
 ·µ»ØÖµ    £ºÎŞ
*****************************************************************************/
extern void WIFI_TEST_CMD(char *cmdStr);

/*****************************************************************************
 º¯ Êı Ãû  : WIFI_GET_TCMD_MODE
 ¹¦ÄÜÃèÊö  : »ñÈ¡²âÊÔÃüÁîµÄÄ£Ê½
 ÊäÈë²ÎÊı  : ÎŞ
 Êä³ö²ÎÊı  : 16£ºĞ£×¼²âÊÔÄ£Ê½
             17£º±íÊ¾´¦ÓÚ·¢ÉäÄ£Ê½
             18£º±íÊ¾½ÓÊÕÄ£Ê½
 ·µ»ØÖµ    £ºº¯ÊıÖ´ĞĞµÄ×´Ì¬½á¹ûÖµ
*****************************************************************************/
extern int WIFI_GET_TCMD_MODE(void);

/*****************************************************************************
 º¯ Êı Ãû  : WIFI_POWER_START
 ¹¦ÄÜÃèÊö  : WIFIÉÏµç
 ÊäÈë²ÎÊı  : ÎŞ
 Êä³ö²ÎÊı  : ÎŞ
 ·µ»ØÖµ    £º 0: execute ok
              1: execute failed
*****************************************************************************/
extern int WIFI_POWER_START(void);

/*****************************************************************************
 º¯ Êı Ãû  : WIFI_POWER_SHUTDOWN
 ¹¦ÄÜÃèÊö  : WIFIÏÂµç
 ÊäÈë²ÎÊı  : ÎŞ
 Êä³ö²ÎÊı  : ÎŞ
 ·µ»ØÖµ    £º 0: execute ok
              1: execute failed
*****************************************************************************/
extern int WIFI_POWER_SHUTDOWN(void);

/*****************************************************************************
 º¯ Êı Ãû  : WIFI_GET_STATUS
 ¹¦ÄÜÃèÊö  : WIFI×´Ì¬»ñÈ¡
 ÊäÈë²ÎÊı  : ÎŞ
 Êä³ö²ÎÊı  : ÎŞ
 ·µ»ØÖµ    £º 0: wifi is off
              1: wifi is in normal mode
              2: wifi is in tcmd mode
*****************************************************************************/
extern int WIFI_GET_STATUS(void);

/*****************************************************************************
 º¯ Êı Ãû  : WIFI_GET_RX_DETAIL_REPORT
 ¹¦ÄÜÃèÊö  : get result of rx report: totalPkt, GoodPkt, ErrorPkt
 ÊäÈë²ÎÊı  : ÎŞ
 Êä³ö²ÎÊı  : totalPkt¡¢goodPkt¡¢badPkt
 ·µ»ØÖµ    £ºÎŞ
*****************************************************************************/
extern void WIFI_GET_RX_DETAIL_REPORT(int *totalPkt, int *goodPkt, int *badPkt);

/*****************************************************************************
 º¯ Êı Ãû  : WIFI_GET_RX_PACKET_REPORT
 ¹¦ÄÜÃèÊö  : get result of rx ucast&mcast packets
 ÊäÈë²ÎÊı  : ÎŞ
 Êä³ö²ÎÊı  : ucastPkts¡¢mcastPkts
 ·µ»ØÖµ    £ºÎŞ
*****************************************************************************/
extern void  WIFI_GET_RX_PACKET_REPORT(unsigned int *ucastPkts, unsigned int *mcastPkts);

/*****************************************************************************
 º¯ Êı Ãû  : WIFI_GET_PA_CUR_MODE
 ¹¦ÄÜÃèÊö  : get the currrent PA mode of the wifi chip
 ÊäÈë²ÎÊı  : ÎŞ
 Êä³ö²ÎÊı  : ÎŞ
 ·µ»ØÖµ    £º0:  (WIFI_ONLY_PA_MODE) WIFI chip is in PA mode
             1:  (WIFI_ONLY_NOPA_MODE) WIFI chip is in no PA mode
             -1: wifi chip is in abnormal mode
*****************************************************************************/
extern int WIFI_GET_PA_CUR_MODE(void);

/*****************************************************************************
 º¯ Êı Ãû  : WIFI_GET_PA_MODE
 ¹¦ÄÜÃèÊö  : get the support PA mode of wifi chip
 ÊäÈë²ÎÊı  : ÎŞ
 Êä³ö²ÎÊı  : ÎŞ
 ·µ»ØÖµ    £º0:  (WIFI_ONLY_PA_MODE) WIFI suppport only PA mode
             1:  (WIFI_ONLY_NOPA_MODE) WIFI suppport only no PA mode
             2:  (WIFI_PA_NOPA_MODE) WIFI suppport both PA &  no PA mode
             -1: failed
*****************************************************************************/
extern int WIFI_GET_PA_MODE(void);

/*****************************************************************************
 º¯ Êı Ãû  : WIFI_SET_PA_MODE
 ¹¦ÄÜÃèÊö  : set the PA mode of wifi chip
 ÊäÈë²ÎÊı  : 0:  (WIFI_ONLY_PA_MODE) WIFI suppport only PA mode
             1:  (WIFI_ONLY_NOPA_MODE) WIFI suppport only no PA mode
             2:  (WIFI_PA_NOPA_MODE) WIFI suppport both PA &  no PA mode
 Êä³ö²ÎÊı  : ÎŞ
 ·µ»ØÖµ    £º0: success
             -1: failed
*****************************************************************************/
extern int WIFI_SET_PA_MODE(int wifiPaMode);

/*****************************************************************************
 º¯ Êı Ãû  : DRV_WIFI_DATA_RESERVED_TAIL
 ¹¦ÄÜÃèÊö  : WIFI¼ÆËãĞèÒªÊı¾İ¿éÄÚ´æ
 ÊäÈë²ÎÊı  : usLen - ÓÃ»§ÉêÇëÊı¾İ³¤¶ÈLen
 Êä³ö²ÎÊı  : ÎŞ
 ·µ »Ø Öµ  : Êı¾İÇøµÄÎ²²¿Ô¤ÁôµÄ³¤¶È
*****************************************************************************/
extern unsigned int DRV_WIFI_DATA_RESERVED_TAIL(unsigned int len);

/*****************************************************************************
 º¯ Êı Ãû  : DRV_WIFI_SET_RX_FCTL
 ¹¦ÄÜÃèÊö  : ÉèÖÃWIFI½ÓÊÕÁ÷¿Ø±êÊ¶
 ÊäÈë²ÎÊı  : para1¡¢para2
 Êä³ö²ÎÊı  : ÎŞ
 ·µ »Ø Öµ  : BSP_OK/BSP_ERROR
*****************************************************************************/
extern unsigned int DRV_WIFI_SET_RX_FCTL(unsigned int para1, unsigned int para2);

/*****************************************************************************
 º¯ Êı Ãû  : DRV_WIFI_CLR_RX_FCTL
 ¹¦ÄÜÃèÊö  : Çå³ıWIFI½ÓÊÕÁ÷¿Ø±êÊ¶
 ÊäÈë²ÎÊı  : para1¡¢para2
 Êä³ö²ÎÊı  : ÎŞ
 ·µ »Ø Öµ  : 1 : ÓĞ
             0 : ÎŞ
*****************************************************************************/
extern unsigned int DRV_WIFI_CLR_RX_FCTL(unsigned int para1, unsigned int para2);

/*****************************************************************************
 º¯ Êı Ãû  : DRV_AT_GET_USER_EXIST_FLAG
 ¹¦ÄÜÃèÊö  : ·µ»Øµ±Ç°ÊÇ·ñÓĞUSBÁ¬½Ó»òÕßWIFIÓÃ»§Á¬½Ó(CºËµ÷ÓÃ)
 ÊäÈë²ÎÊı  : ÎŞ
 Êä³ö²ÎÊı  : ÎŞ
 ·µ »Ø Öµ  : 1 : ÓĞ
             0 : ÎŞ
*****************************************************************************/
extern unsigned int  DRV_AT_GET_USER_EXIST_FLAG(void);


/*************************WIFI END************************************/

#endif

