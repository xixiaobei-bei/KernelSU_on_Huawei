/*-
 * Copyright (c) 1996 - 2001 Brian Somers <brian@Awfulhak.org>
 *          based on work by Toshiharu OHNO <tony-o@iij.ad.jp>
 *                           Internet Initiative Japan, Inc (IIJ)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/usr.sbin/ppp/auth.c,v 1.56 2003/10/29 20:32:19 ru Exp $
 */


#include "PPP/Inc/ppp_public.h"
#include "PPP/Inc/layer.h"
#include "PPP/Inc/ppp_mbuf.h"
#include "PPP/Inc/hdlc.h"
#include "PPP/Inc/throughput.h"
#include "PPP/Inc/proto.h"
#include "PPP/Inc/ppp_fsm.h"
#include "PPP/Inc/lcp.h"
#include "PPP/Inc/async.h"
#include "PPP/Inc/auth.h"
#include "PPP/Inc/ipcp.h"
#include "PPP/Inc/pppid.h"
#include "PPP/Inc/link.h"

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AUTH_C


const VOS_CHAR *
Auth2Nam(VOS_UINT16 auth, VOS_CHAR type)
{
  static VOS_CHAR chap[10];

  switch (auth) {
  case PROTO_PAP:
    return "PAP";

  case PROTO_CHAP:
    /*VOS_nsprintf(chap, sizeof chap, "CHAP 0x%02x", type); */
    return chap;

  case 0:
    return "none";
  }
  return "unknown";
}




/*****************************************************************************
 函 数 名  : AuthTimeout
 功能描述  : 认证定时器超时处理
 输入参数  : l - PPP链接
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2008年10月27日
    作    者   : liukai
    修改内容   : porting from BSD

*****************************************************************************/
VOS_VOID AuthTimeout(struct link *l)
{
  VOS_UINT32 ulTimerValue;

  struct authinfo *authp = &(l->chap.auth);

  if (--authp->retry > 0) {    /* can still try */
      authp->id++;
      if (VOS_NULL_PTR != authp->fn.req)    /* add protection of function pointer */
      {
          (*authp->fn.req)(l);
          ulTimerValue = PPP_FSM_TIME_INTERVAL;
          if (authp->cfg.fsm.timeout > 0)
          {
              ulTimerValue = PS_MIN(authp->cfg.fsm.timeout*1000, PPP_FSM_TIME_INTERVAL);
          }
          VOS_StartRelTimer(&(authp->hAuthTimer), PS_PID_APP_PPP,
              ulTimerValue, (VOS_UINT32)(PPP_LINK_TO_ID(l)), PHASE_AUTHENTICATE,
              VOS_RELTIMER_NOLOOP, VOS_TIMER_PRECISION_0);
          /* timer_Start(&authp->authtimer); */
      }
  } else {
      PPP_MNTN_LOG(PS_PID_APP_PPP, 0, PS_PRINT_NORMAL,
          "Auth: No response from server try max times. Goto lcp stage!\r\n");
      chap_ReInit(&(l->chap));
      l->phase = PHASE_TERMINATE;
      fsm_Close(&(l->lcp.fsm));
  }
}    /* AuthTimeout */

void
auth_Init(struct authinfo *authp,/* struct physical *p, */auth_func req,
          auth_func success, auth_func failure)
{
  PS_MEM_SET(authp, '\0', sizeof(struct authinfo));
  authp->cfg.fsm.timeout = DEF_FSMRETRY;
  authp->cfg.fsm.maxreq = DEF_FSMAUTHTRIES;
  authp->cfg.fsm.maxtrm = 0;    /* not used */
  authp->fn.req = req;
  authp->fn.success = success;
  authp->fn.failure = failure;
  authp->hAuthTimer = VOS_NULL;
/*  authp->physical = p;*/
}

/*****************************************************************************
 函 数 名  : auth_StartReq
 功能描述  : 发起认证
 输入参数  : l - PPP链接
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2008年10月27日
    作    者   : liukai
    修改内容   : porting from BSD

*****************************************************************************/
VOS_VOID auth_StartReq(struct link *l)
{
  VOS_UINT32 ulTimerValue;

  struct authinfo *authp = &(l->chap.auth);
/*  timer_Stop(&authp->authtimer);*/
/*  authp->authtimer.func = AuthTimeout;
  authp->authtimer.name = "auth";
  authp->authtimer.load = authp->cfg.fsm.timeout * SECTICKS;
  authp->authtimer.arg = (void *)authp;*/
  authp->retry = (VOS_INT32)(authp->cfg.fsm.maxreq);
  authp->id = 1;
  if (VOS_NULL_PTR != authp->fn.req)    /* add protection of function pointer */
  {
    (*authp->fn.req)(l);

    /* 防止在上一次认证定时器没有超时前, 发生LCP Layer Up, 再次需要认证 */
    if (VOS_NULL != authp->hAuthTimer)
    {
        PS_STOP_REL_TIMER(&(authp->hAuthTimer));
    }
    /*  timer_Start(&authp->authtimer);*/
    ulTimerValue = PPP_FSM_TIME_INTERVAL;
    if (authp->cfg.fsm.timeout > 0)
    {
        ulTimerValue = PS_MIN(authp->cfg.fsm.timeout*1000, PPP_FSM_TIME_INTERVAL);
    }
    VOS_StartRelTimer(&(authp->hAuthTimer), PS_PID_APP_PPP,
            ulTimerValue, (VOS_UINT32)(PPP_LINK_TO_ID(l)), PHASE_AUTHENTICATE,
            VOS_RELTIMER_NOLOOP,VOS_TIMER_PRECISION_0);
  }
}    /* auth_StartReq */

/*****************************************************************************
 函 数 名  : auth_StopTimer
 功能描述  : 停止认证定时器
 输入参数  : authp - PPP链接中认证参数
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2008年10月27日
    作    者   : liukai
    修改内容   : porting from BSD

*****************************************************************************/
VOS_VOID auth_StopTimer(struct authinfo *authp)
{
/*  timer_Stop(&authp->authtimer);*/
    if (VOS_NULL_PTR != authp->hAuthTimer)
    {
        PS_STOP_REL_TIMER(&(authp->hAuthTimer));
    }
}    /* auth_StopTimer */

struct ppp_mbuf *
auth_ReadHeader(struct authinfo *authp, struct ppp_mbuf *bp)
{
  VOS_INT32 len;

  len = ppp_m_length(bp);
  if (len >= sizeof authp->in.hdr) {
    bp = ppp_mbuf_Read(bp, (VOS_CHAR *)&authp->in.hdr, sizeof authp->in.hdr);
    if (len >= VOS_NTOHS(authp->in.hdr.length))
      return bp;
    authp->in.hdr.length = VOS_HTONS(0);
    PPP_MNTN_LOG2(PS_PID_APP_PPP, 0, PS_PRINT_WARNING, "auth_ReadHeader: Short packet (%d > %d): !",
               VOS_NTOHS(authp->in.hdr.length), len);
  } else {
    authp->in.hdr.length = VOS_HTONS(0);
    PPP_MNTN_LOG2(PS_PID_APP_PPP, 0, PS_PRINT_WARNING, "auth_ReadHeader: Short packet header (%d > %d) !",
               (VOS_INT32)(sizeof authp->in.hdr), len);
  }

  ppp_m_freem(bp);
  return VOS_NULL_PTR;
}

struct ppp_mbuf *
auth_ReadName(struct authinfo *authp, struct ppp_mbuf *bp, VOS_INT32 len)
{
  if (len > sizeof authp->in.name - 1)
  {
    PPP_MNTN_LOG1(PS_PID_APP_PPP, 0, LOG_LEVEL_WARNING,
                  "auth_ReadName: Name too long (%d) !\r\n", len);
  }
  else {
    VOS_INT32 mlen = ppp_m_length(bp);

    if (len > mlen)
    {
      PPP_MNTN_LOG2(PS_PID_APP_PPP, 0, LOG_LEVEL_WARNING,
                    "auth_ReadName: Short packet (%d > %d) !\r\n",
                    len, mlen);
    }
    else {
      bp = ppp_mbuf_Read(bp, (VOS_CHAR *)authp->in.name, len);
      authp->in.name[len] = '\0';
      return bp;
    }
  }

  *authp->in.name = '\0';
  ppp_m_freem(bp);
  return VOS_NULL_PTR;
}


