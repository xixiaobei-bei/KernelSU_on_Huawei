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
 * $FreeBSD: src/usr.sbin/ppp/lcp.c,v 1.105 2002/08/27 20:11:57 brian Exp $
 *  1. 2006-03-13 MODIFY BY F49086 FOR A32D02477
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
#include "PPP/Inc/pap.h"
#include "PPP/Inc/ppp_input.h"
#include "PPP/Inc/ppp_atcmd.h"

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_LCP_C


/* 保存从NV项中读取的MRU值*/
VOS_UINT16 g_usPppConfigMru = DEF_MRU;

/* for received LQRs */
struct lqrreq {
  struct fsm_opt_hdr hdr;
  VOS_UINT16 proto;        /* Quality protocol */
  VOS_UINT32 period;        /* Reporting interval */
};

VOS_INT32 LcpLayerUp(struct fsm *);
void LcpLayerDown(struct fsm *);
void LcpLayerStart(struct fsm *);
void LcpLayerFinish(struct fsm *);
void LcpInitRestartCounter(struct fsm *, VOS_INT32);
void LcpSendConfigReq(struct fsm *);
void LcpSentTerminateReq(struct fsm *);
void LcpSendTerminateAck(struct fsm *, VOS_CHAR);
void LcpDecodeConfig(struct fsm *, VOS_CHAR *, VOS_CHAR *, VOS_INT32,
                            struct fsm_decode *);

static struct fsm_callbacks lcp_Callbacks = {
  LcpLayerUp,
  LcpLayerDown,
  LcpLayerStart,
  LcpLayerFinish,
  LcpInitRestartCounter,
  LcpSendConfigReq,
  LcpSentTerminateReq,
  LcpSendTerminateAck,
  LcpDecodeConfig,
  fsm_NullRecvResetReq,
  fsm_NullRecvResetAck
};

static const VOS_CHAR * const lcp_TimerNames[] =
  {"LCP restart", "LCP openmode", "LCP stopped"};




VOS_INT32
lcp_ReportStatus(struct link *l/*struct cmdargs const *arg*/)
{
/*  struct link *l;*/
  struct lcp *lcp;

/*  l = command_ChooseLink(arg);*/
  lcp = &l->lcp;
  return 0;
}

VOS_UINT32
GenerateMagic(void)
{
  /* Generate random number which will be used as magic number */

  static VOS_UINT32 i = 1345;
  i++;
  return i;
}

void
lcp_SetupCallbacks(struct lcp *lcp)
{
  lcp->fsm.fn = &lcp_Callbacks;
}

void
lcp_Init(struct lcp *lcp, /*struct bundle *bundle, */struct link *l,
         const struct fsm_parent *parent)
{
  /* Initialise ourselves */
  VOS_INT32 mincode = parent ? 1 : LCP_MINMPCODE;

  fsm_Init(&lcp->fsm, "LCP", PROTO_LCP, mincode, LCP_MAXCODE,5 /*LogLCP,
           bundle*/, l, parent, &lcp_Callbacks, lcp_TimerNames);

  lcp->cfg.mru = g_usPppConfigMru;
  lcp->cfg.max_mru = MAX_MRU;
  lcp->cfg.mtu = 0;
  lcp->cfg.max_mtu = MAX_MTU;
  lcp->cfg.accmap = 0;
  lcp->cfg.openmode = 1;
  lcp->cfg.lqrperiod = DEF_LQRPERIOD;
  lcp->cfg.fsm.timeout = DEF_FSMRETRY;
  lcp->cfg.fsm.maxreq = DEF_FSMTRIES;
  lcp->cfg.fsm.maxtrm = DEF_FSMTRIES;

  lcp->cfg.acfcomp = NEG_ENABLED|NEG_ACCEPTED;

/* CDMA模式下只有PC拨号才会用到PPPA,此时只支持PAP不支持CHAP */
  lcp->cfg.chap05 = NEG_ENABLED|NEG_ACCEPTED; /* support CHAP */

  lcp->cfg.chap80nt = 0/*NEG_ACCEPTED*/;
  lcp->cfg.chap80lm = 0;
  lcp->cfg.chap81 = 0/*NEG_ACCEPTED*/;
  lcp->cfg.lqr = 0/*NEG_ACCEPTED*/;
  lcp->cfg.pap = NEG_ENABLED|NEG_ACCEPTED/*NEG_ACCEPTED*/;
  lcp->cfg.protocomp = NEG_ENABLED|NEG_ACCEPTED;
  *lcp->cfg.ident = '\0';
  lcp->hLcpCloseTimer = VOS_NULL_PTR;

  lcp_Setup(lcp, lcp->cfg.openmode);
}

/*****************************************************************************
 函 数 名  : lcp_Setup
 功能描述  :
 输入参数  :
 输出参数  :
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2008年8月1日
    作    者   : liukai
    修改内容   : 增加对控制Option MRRU和ShortSeq的全局变量的初始化

*****************************************************************************/
void
lcp_Setup(struct lcp *lcp, VOS_INT32 openmode)
{
/*  struct physical *p = link2physical(lcp->fsm.link);*/

  lcp->fsm.open_mode = openmode;

  lcp->his_mru = DEF_MRU;
  lcp->his_mrru = 0;
  lcp->his_magic = 0;
  lcp->his_lqrperiod = 0;
  lcp->his_acfcomp = 0;
  lcp->his_auth = 0;
  lcp->his_authtype = 0;
  lcp->his_callback.opmask = 0;
  lcp->his_shortseq = 0;
  lcp->mru_req = 0;

  if ((lcp->want_mru = lcp->cfg.mru) == 0){
    lcp->want_mru = DEF_MRU;
  }
  lcp->want_mrru = 0;
  lcp->want_shortseq = 0;
/*  lcp->want_mrru = lcp->fsm.bundle->ncp.mp.cfg.mrru;*/
/*  lcp->want_shortseq = IsEnabled(lcp->fsm.bundle->ncp.mp.cfg.shortseq) ? 1 : 0;*/
  lcp->want_acfcomp = IsEnabled(lcp->cfg.acfcomp) ? 1 : 0;

  if (lcp->fsm.parent) {
    lcp->his_accmap = 0xffffffff;
    lcp->want_accmap = lcp->cfg.accmap;
    lcp->his_protocomp = 0;
    lcp->want_protocomp = IsEnabled(lcp->cfg.protocomp) ? 1 : 0;
    lcp->want_magic = GenerateMagic();

    if (IsEnabled(lcp->cfg.chap05)) {
      lcp->want_auth = PROTO_CHAP;
      lcp->want_authtype = 0x05;
    } else if (IsEnabled(lcp->cfg.chap80nt) ||
               IsEnabled(lcp->cfg.chap80lm)) {
      lcp->want_auth = PROTO_CHAP;
      lcp->want_authtype = 0x80;
    } else if (IsEnabled(lcp->cfg.chap81)) {
      lcp->want_auth = PROTO_CHAP;
      lcp->want_authtype = 0x81;
    } else if (IsEnabled(lcp->cfg.pap)) {
      lcp->want_auth = PROTO_PAP;
      lcp->want_authtype = 0;
    } else {
      lcp->want_auth = 0;
      lcp->want_authtype = 0;
    }
/*
    if (p->type != PHYS_DIRECT)
      PS_MEM_CPY(&lcp->want_callback, &p->dl->cfg.callback,
             sizeof(struct callback));
    else*/
      lcp->want_callback.opmask = 0;
    lcp->want_lqrperiod = IsEnabled(lcp->cfg.lqr) ?
                          lcp->cfg.lqrperiod * 100 : 0;
  } else {
    lcp->his_accmap = lcp->want_accmap = 0;
    lcp->his_protocomp = lcp->want_protocomp = 1;
    lcp->want_magic = 0;
    lcp->want_auth = 0;
    lcp->want_authtype = 0;
    lcp->want_callback.opmask = 0;
    lcp->want_lqrperiod = 0;
  }

  lcp->his_reject = lcp->my_reject = 0;
  lcp->auth_iwait = lcp->auth_ineed = 0;
  lcp->LcpFailedMagic = 0;
}    /* lcp_Setup */

void
LcpInitRestartCounter(struct fsm *fp, VOS_INT32 what)
{
  /* Set fsm timer load */
  struct lcp *lcp = fsm2lcp(fp);
  PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "LcpInitRestartCounter");

/*  fp->FsmTimer.load = lcp->cfg.fsm.timeout * SECTICKS;*/
  switch (what) {
    case FSM_REQ_TIMER:
      fp->restart = lcp->cfg.fsm.maxreq; /* [false alarm]:移植开源代码 */
      break;
    case FSM_TRM_TIMER:
      fp->restart = lcp->cfg.fsm.maxtrm; /* [false alarm]:移植开源代码 */
      break;
    default:
      fp->restart = 1;
      break;
  }

/*fanzhibin f49086 add it begin*/

  /*========>A32D12744*/
  if( VOS_NULL_PTR !=(fp->timer) )
  {
      VOS_StopRelTimer(&(fp->timer));
  }
  /*A32D12744<========*/

  if (VOS_OK !=
      VOS_StartRelTimer(&(fp->timer),
        PS_PID_APP_PPP,
        PPP_FSM_TIME_INTERVAL,
        PPP_LINK_TO_ID(fp->link),
        fp->link->phase,
        VOS_RELTIMER_NOLOOP,
        VOS_TIMER_PRECISION_0))
  {
        PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL,"start reltimer error\r\n");
  }


/*fanzhibin f49086 add it end*/

}

void
LcpSendConfigReq(struct fsm *fp)
{
  /* Send config REQ please */
/*  struct physical *p = link2physical(fp->link);*/

  struct lcp *lcp = fsm2lcp(fp);
  VOS_CHAR buff[200];
  struct fsm_opt *o;
  VOS_UINT16 proto;
  VOS_UINT16 maxmru = 0;
/*
  if (!p) {
    log_Printf(LogERROR, "%s: LcpSendConfigReq: Not a physical link !\n",
              fp->link->name);
    return;
  }
*/
  PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "LcpSendConfigReq\r\n");
  o = (struct fsm_opt *)buff;
/*
  if (!physical_IsSync(p)) */{
    if (lcp->want_acfcomp && !REJECTED(lcp, TY_ACFCOMP)) /* [false alarm]:移植开源代码 */
      INC_FSM_OPT(TY_ACFCOMP, 2, o);

    if (lcp->want_protocomp && !REJECTED(lcp, TY_PROTOCOMP))
      INC_FSM_OPT(TY_PROTOCOMP, 2, o);

    if (!REJECTED(lcp, TY_ACCMAP)) {

      ua_htonl(&lcp->want_accmap, o->data);
      INC_FSM_OPT(TY_ACCMAP, 6, o);
    }
  }


/*  maxmru = p ? physical_DeviceMTU(p) : 0;*/
  if (lcp->cfg.max_mru && (!maxmru || maxmru > lcp->cfg.max_mru))
    maxmru = lcp->cfg.max_mru;
  if (maxmru && lcp->want_mru > maxmru) {
    PPP_MNTN_LOG2(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "Reducing configured MRU from <1> to <2>\r\n", lcp->want_mru, maxmru);
    lcp->want_mru = maxmru;
  }
  if (!REJECTED(lcp, TY_MRU)) {
    ua_htons(&lcp->want_mru, o->data);
    INC_FSM_OPT(TY_MRU, 4, o);
  }

  if (lcp->want_magic && !REJECTED(lcp, TY_MAGICNUM)) {
    ua_htonl(&lcp->want_magic, o->data);
    INC_FSM_OPT(TY_MAGICNUM, 6, o);
  }

  if (lcp->want_lqrperiod && !REJECTED(lcp, TY_QUALPROTO)) {
    proto = PROTO_LQR;
    ua_htons(&proto, o->data);
    ua_htonl(&lcp->want_lqrperiod, o->data + 2);
    INC_FSM_OPT(TY_QUALPROTO, 8, o);
  }

  switch (lcp->want_auth) {
  case PROTO_PAP:
    proto = PROTO_PAP;
    ua_htons(&proto, o->data);
    INC_FSM_OPT(TY_AUTHPROTO, 4, o);
    break;

  case PROTO_CHAP:
    proto = PROTO_CHAP;
    ua_htons(&proto, o->data);
    o->data[2] = lcp->want_authtype;
    INC_FSM_OPT(TY_AUTHPROTO, 5, o);
    break;
  }

  if (!REJECTED(lcp, TY_CALLBACK)) {
    if (lcp->want_callback.opmask & CALLBACK_BIT(CALLBACK_AUTH)) {
      *o->data = CALLBACK_AUTH;
      INC_FSM_OPT(TY_CALLBACK, 3, o);
    } else if (lcp->want_callback.opmask & CALLBACK_BIT(CALLBACK_CBCP)) {
      *o->data = CALLBACK_CBCP;
      INC_FSM_OPT(TY_CALLBACK, 3, o);
    } else if (lcp->want_callback.opmask & CALLBACK_BIT(CALLBACK_E164)) {
      VOS_INT32 sz = VOS_StrNLen(lcp->want_callback.msg,(SCRIPT_LEN-1));

      if (sz > sizeof o->data - 1) {
        sz = sizeof o->data - 1;
        PPP_MNTN_LOG1(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL,"Truncating E164 data to <1> octets (oops!)\n", sz);
      }
      *o->data = CALLBACK_E164;
      PS_MEM_CPY(o->data + 1, lcp->want_callback.msg, sz);
      INC_FSM_OPT((VOS_UINT8)TY_CALLBACK, (VOS_UINT8)(sz + 3), o);
    }
  }

  if (lcp->want_mrru && !REJECTED(lcp, TY_MRRU)) {
    ua_htons(&lcp->want_mrru, o->data);
    INC_FSM_OPT(TY_MRRU, 4, o);

    if (lcp->want_shortseq && !REJECTED(lcp, TY_SHORTSEQ))
      INC_FSM_OPT(TY_SHORTSEQ, 2, o);
  }
  fsm_Output(fp, CODE_CONFIGREQ, fp->reqid, buff, (VOS_CHAR *)o - buff,
             MB_LCPOUT);
}

void
lcp_SendProtoRej(struct lcp *lcp, VOS_CHAR *option, VOS_INT32 count)
{
  /* Don't understand `option' */
  PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "lcp_SendProtoRej\r\n");
  fsm_Output(&lcp->fsm, CODE_PROTOREJ, lcp->fsm.reqid, option, count,
             MB_LCPOUT);
}

/*lint -e578 by liukai*/
/*****************************************************************************
 函 数 名  : lcp_SendIdentification
 功能描述  : 向LCP包填写Identification
 输入参数  : lcp - LCP控制结构
 输出参数  : 无
 返 回 值  : 指示是否填写, 未填写 - 0, 填写 - 1
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2008年5月21日
    作    者   : liukai
    修改内容   : AT2D03379
*****************************************************************************/
VOS_INT32
lcp_SendIdentification(struct lcp *lcp)
{
  static VOS_CHAR id;        /* Use a private id */
  VOS_CHAR msg[DEF_MRU - 3];
  const VOS_CHAR *argv[2];
  VOS_CHAR Outchar = 1;
  VOS_CHAR *exp[2];

  PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "lcp_SendIdentification\r\n");
  if (*lcp->cfg.ident == '\0')
    return 0;

  exp[0]  = &Outchar;
  exp[1]  = &Outchar;
  argv[0] = lcp->cfg.ident;
  argv[1] = VOS_NULL_PTR;
  ua_htonl(&lcp->want_magic, msg);
  VOS_StrNCpy(msg + 4, exp[0], sizeof msg - 5);
  msg[sizeof msg - 1] = '\0';

  fsm_Output(&lcp->fsm, CODE_IDENT, id++, msg, 4 + VOS_StrNLen(msg + 4,DEF_MRU - 10), MB_LCPOUT);
  PPP_MNTN_LOG1(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, " MAGICNUM <1>\r\n", (VOS_INT32)lcp->want_magic);

  return 1;
}
/*lint +e578 by liukai*/

void
lcp_RecvIdentification(struct lcp *lcp, VOS_CHAR *data)
{
  PPP_MNTN_LOG1(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "lcp_RecvIdentification,MAGICNUM:<1>\r\n", (VOS_INT32)lcp->his_magic);
}

void
LcpSentTerminateReq(struct fsm *fp)
{
  PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "LcpSentTerminateReq\r\n");
  /* Term REQ just sent by FSM */
}

void
LcpSendTerminateAck(struct fsm *fp, VOS_CHAR id)
{
  PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "LcpSendTerminateAck\r\n");
  fsm_Output(fp, CODE_TERMACK, id, VOS_NULL_PTR, 0, MB_LCPOUT);
}

void
LcpLayerStart(struct fsm *fp)
{
  /* We're about to start up ! */
  struct lcp *lcp = fsm2lcp(fp);

  PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "LcpLayerStart");
  lcp->LcpFailedMagic = 0; /* [false alarm]:移植开源代码 */
  fp->more.reqs = fp->more.naks = fp->more.rejs = lcp->cfg.fsm.maxreq * 3;
  lcp->mru_req = 0;
}

void
LcpLayerFinish(struct fsm *fp)
{
  VOS_UINT16 usPppId = PPP_LINK_TO_ID(fp->link);

  /* We're now down */
  PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "LcpLayerFinish\r\n");

/*fanzhibin f49086 add it begin*/


  /*通知AT进行PDP去激活*/
  PPP_ProcPppRelEvent(usPppId);

  /* 可维可测信息上报*/
  Ppp_EventMntnInfo(usPppId, AT_PPP_RECV_RELEASE_IND);


  if (VOS_NULL_PTR != fp->link->lcp.hLcpCloseTimer)
  {
      PS_STOP_REL_TIMER(&(fp->link->lcp.hLcpCloseTimer));
      fp->link->lcp.hLcpCloseTimer = VOS_NULL_PTR;
  }

  /*LCP进入Finish,说明PPP和PC间断开已经完成
    需要通知拉管脚信号*/
  PPP_ProcPppDisconnEvent(usPppId);

  /*然后释放该PPP ID*/
  PppFreeId(usPppId);

  return;
/*fanzhibin f49086 add it end*/

}

VOS_INT32
LcpLayerUp(struct fsm *fp)
{


  PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "LcpLayerUp\r\n");
  fp->more.reqs = fp->more.naks = fp->more.rejs = fp->link->lcp.cfg.fsm.maxreq * 3;

  lcp_SendIdentification(&(fp->link->lcp));
/*fanzhibin f49086 add it begin*/
  if(fp->link->lcp.want_auth != 0)
  {
      ipcp_Init(&(fp->link->ipcp), fp->link, &parent);
      fp->link->lcp.auth_ineed = fp->link->lcp.want_auth;
      fp->link->lcp.auth_iwait = fp->link->lcp.his_auth;
      fp->link->phase = PHASE_AUTHENTICATE;
      PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "goto auth stage\r\n");

      /* added by liukai */
      if (fp->link->lcp.want_auth == PROTO_CHAP)    /* right now just CHAP needs UE to start authentication */
        auth_StartReq(fp->link);
  }
  else    /* added by liukai */
  {
      fp->link->phase = PHASE_NETWORK;
      fp->link->ipcp.fsm.state = ST_CLOSED;
      fsm_Open(&(fp->link->ipcp.fsm));
  }

/*fanzhibin f49086 add it end*/

  return 1;
}

void
LcpLayerDown(struct fsm *fp)
{


  PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "LcpLayerDown\r\n");
  lcp_Setup(fsm2lcp(fp), 0);
}
VOS_INT32
lcp_auth_nak(struct lcp *lcp, struct fsm_decode *dec)
{
  struct fsm_opt nak;

  nak.hdr.id = TY_AUTHPROTO;

  if (IsAccepted(lcp->cfg.pap)) {
    nak.hdr.len = 4;
    nak.data[0] = (VOS_UCHAR)(PROTO_PAP >> 8);
    nak.data[1] = (VOS_UCHAR)PROTO_PAP;
    fsm_nak(dec, &nak);
    return 1;
  }

  nak.hdr.len = 5;
  nak.data[0] = (VOS_UCHAR)(PROTO_CHAP >> 8);
  nak.data[1] = (VOS_UCHAR)PROTO_CHAP;

  if (IsAccepted(lcp->cfg.chap05)) {
    nak.data[2] = 0x05;
    fsm_nak(dec, &nak);
  } else if (IsAccepted(lcp->cfg.chap80nt) ||
             IsAccepted(lcp->cfg.chap80lm)) {
    nak.data[2] = 0x80;
    fsm_nak(dec, &nak);
  } else if (IsAccepted(lcp->cfg.chap81)) {
    nak.data[2] = 0x81;
    fsm_nak(dec, &nak);
  } else {
    return 0;
  }

  return 1;
}


void
LcpDecodeConfig(struct fsm *fp, VOS_CHAR *cp, VOS_CHAR *end, VOS_INT32 mode_type,
                struct fsm_decode *dec)
{
  /* Deal with incoming PROTO_LCP */

  struct lcp *lcp = fsm2lcp(fp);
  VOS_INT32 sz, pos, op, callback_req;
  VOS_UINT8 chap_type;
  VOS_UINT32 magic, accmap;
  VOS_UINT16 mru, maxmtu = 0, maxmru = 0, wantmtu, wantmru, proto;
  struct lqrreq *req;
  VOS_CHAR desc[22];


  struct fsm_opt *opt, nak;
  /*VOS_UINT32 auth_requested = 1;*/

  sz = op = callback_req = 0;

  while (end - cp >= sizeof(opt->hdr)) {
    if ((opt = fsm_readopt(&cp)) == VOS_NULL_PTR)
      break;

    /*VOS_nsprintf(request, sizeof request, " %s[%d]", protoname(opt->hdr.id),opt->hdr.len); */

    switch (opt->hdr.id) {
    case TY_MRU:
      lcp->mru_req = 1; /* [false alarm]:移植开源代码 */
      ua_ntohs(opt->data, &mru);
      PPP_MNTN_LOG1(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL,"mru= <1>\r\n", mru);

      switch (mode_type) {
      case MODE_REQ:
        if (lcp->cfg.max_mtu && (!maxmtu || maxmtu > lcp->cfg.max_mtu))
          maxmtu = lcp->cfg.max_mtu;
        wantmtu = lcp->cfg.mtu;
        if (maxmtu && wantmtu > maxmtu) {
          PPP_MNTN_LOG2(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL,
                        "Reducing configured MTU from <1> to <2>\r\n", wantmtu, maxmtu);
          wantmtu = maxmtu;
        }

        if (maxmtu && mru > maxmtu) {
          lcp->his_mru = maxmtu;
          nak.hdr.id = TY_MRU;
          nak.hdr.len = 4;
          ua_htons(&lcp->his_mru, nak.data);
          fsm_nak(dec, &nak);
        } else if (wantmtu && mru < wantmtu) {
          /* Push him up to MTU or MIN_MRU */
          lcp->his_mru = wantmtu;
          nak.hdr.id = TY_MRU;
          nak.hdr.len = 4;
          ua_htons(&lcp->his_mru, nak.data);
          fsm_nak(dec, &nak);
        } else {
          lcp->his_mru = mru;
          fsm_ack(dec, opt);
        }
        break;
      case MODE_NAK:

        if (lcp->cfg.max_mru && (!maxmru || maxmru > lcp->cfg.max_mru))
          maxmru = lcp->cfg.max_mru;
        wantmru = lcp->cfg.mru > maxmru ? maxmru : lcp->cfg.mru;

        if (wantmru && mru > wantmru)
          lcp->want_mru = wantmru;
        else if (mru > maxmru)
          lcp->want_mru = maxmru;
        else if (mru < MIN_MRU)
          lcp->want_mru = MIN_MRU;
        else
          lcp->want_mru = mru;
        break;
      case MODE_REJ:
        lcp->his_reject |= (1 << opt->hdr.id);
        break;
      }
      break;

    case TY_ACCMAP:
      ua_ntohl(opt->data, &accmap);
      PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "want accmap\r\n");

      switch (mode_type) {
      case MODE_REQ:
        lcp->his_accmap = accmap; /* [false alarm]:移植开源代码 */
        fsm_ack(dec, opt);
        break;
      case MODE_NAK:
        lcp->want_accmap = accmap; /* [false alarm]:移植开源代码 */
        break;
      case MODE_REJ:
        lcp->his_reject |= (1 << opt->hdr.id); /* [false alarm]:移植开源代码 */
        break;
      }
      break;

    case TY_AUTHPROTO:
      ua_ntohs(opt->data, &proto);
      chap_type = opt->hdr.len == 5 ? opt->data[2] : 0;

      PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "want auth\r\n");

      switch (mode_type) {
      case MODE_REQ:
        /*auth_requested = 0;*/
        switch (proto) {
        case PROTO_PAP:
          if (opt->hdr.len == 4 && IsAccepted(lcp->cfg.pap)) { /* [false alarm]:移植开源代码 */
            lcp->his_auth = proto;
            lcp->his_authtype = 0;
            fsm_ack(dec, opt);
          } else if (!lcp_auth_nak(lcp, dec)) {
            lcp->my_reject |= (1 << opt->hdr.id); /* [false alarm]:移植开源代码 */
            fsm_rej(dec, opt);
          }
          break;

        case PROTO_CHAP:
          if ((chap_type == 0x05 && IsAccepted(lcp->cfg.chap05))    /* chap_type: 0x05, use MD5 *//* [false alarm]:移植开源代码 */
              || (chap_type == 0x80 && (IsAccepted(lcp->cfg.chap80nt) || /* [false alarm]:移植开源代码 */
                                   (IsAccepted(lcp->cfg.chap80lm))))
              || (chap_type == 0x81 && IsAccepted(lcp->cfg.chap81)) /* [false alarm]:移植开源代码 */
             ) {
            lcp->his_auth = proto;
            lcp->his_authtype = chap_type;
            fsm_ack(dec, opt);
          } else {
            if (chap_type != 0x05)
              PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_WARNING, "%s not supported\r\n"/*,
                         Auth2Nam(PROTO_CHAP, chap_type)*/);

            if (!lcp_auth_nak(lcp, dec)) {
              lcp->my_reject |= (1 << opt->hdr.id); /* [false alarm]:移植开源代码 */
              fsm_rej(dec, opt);
            }
          }
          break;

        default:
          PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "not recognised\r\n");
          if (!lcp_auth_nak(lcp, dec)) {
            lcp->my_reject |= (1 << opt->hdr.id); /* [false alarm]:移植开源代码 */
            fsm_rej(dec, opt);
          }
          break;
        }
        break;

      case MODE_NAK:
        switch (proto) {
        case PROTO_PAP:
          if (IsEnabled(lcp->cfg.pap)) { /* [false alarm]:移植开源代码 */
            lcp->want_auth = PROTO_PAP;
            lcp->want_authtype = 0;
          } else {
            PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "Peer will only send PAP (not enabled)\r\n");
            lcp->his_reject |= (1 << opt->hdr.id);
          }
          break;
        case PROTO_CHAP:
          if (chap_type == 0x05 && IsEnabled(lcp->cfg.chap05)) { /* [false alarm]:移植开源代码 */
            lcp->want_auth = PROTO_CHAP;
            lcp->want_authtype = 0x05;
          } else if (chap_type == 0x80 && (IsEnabled(lcp->cfg.chap80nt) ||  /* [false alarm]:移植开源代码 */
                                           IsEnabled(lcp->cfg.chap80lm))) {
            lcp->want_auth = PROTO_CHAP;
            lcp->want_authtype = 0x80;
          } else if (chap_type == 0x81 && IsEnabled(lcp->cfg.chap81)) { /* [false alarm]:移植开源代码 */
            lcp->want_auth = PROTO_CHAP;
            lcp->want_authtype = 0x81;
          } else {
            PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "Peer will only send\r\n");
            lcp->his_reject |= (1 << opt->hdr.id); /* [false alarm]:移植开源代码 */
	     /*如果支持CHAP,但CHAP的算法UE不支持,则改为使用PAP验证*/
	     lcp->want_auth = PROTO_PAP;
            lcp->want_authtype = 0;

          }
          break;
        default:
          /* We've been NAK'd with something we don't understand :-( */
          lcp->his_reject |= (1 << opt->hdr.id); /* [false alarm]:移植开源代码 */
          break;
        }
        break;

      case MODE_REJ:
        lcp->his_reject |= (1 << opt->hdr.id); /* [false alarm]:移植开源代码 */
        PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "Peer will not auth by our way\r\n");
        lcp->want_auth = 0;    /* added by liukai, 2008-11-24 */
        break;
      }
      break;

    case TY_QUALPROTO:
      req = (struct lqrreq *)opt;
      PPP_MNTN_LOG2(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL,
                    "proto <1>;interval <2>\r\n",
                    (VOS_INT32)VOS_NTOHS(req->proto), (VOS_INT32)(VOS_NTOHL(req->period) * 10));
      switch (mode_type) {
      case MODE_REQ:
        if (VOS_NTOHS(req->proto) != PROTO_LQR || !IsAccepted(lcp->cfg.lqr)) { /* [false alarm]:移植开源代码 */
          fsm_rej(dec, opt);
          lcp->my_reject |= (1 << opt->hdr.id); /* [false alarm]:移植开源代码 */
        } else {
          lcp->his_lqrperiod = VOS_NTOHL(req->period);
          if (lcp->his_lqrperiod < MIN_LQRPERIOD * 100)
            lcp->his_lqrperiod = MIN_LQRPERIOD * 100;
          req->period = VOS_HTONL(lcp->his_lqrperiod);
          fsm_ack(dec, opt);
        }
        break;
      case MODE_NAK:
        lcp->want_lqrperiod = VOS_NTOHL(req->period); /* [false alarm]:移植开源代码 */
        break;
      case MODE_REJ:
        lcp->his_reject |= (1 << opt->hdr.id); /* [false alarm]:移植开源代码 */
        break;
      }
      break;

    case TY_MAGICNUM:
      ua_ntohl(opt->data, &magic);
      PPP_MNTN_LOG1(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "magic number <1>\r\n", (VOS_INT32)magic);

      switch (mode_type) {
      case MODE_REQ:
        if (lcp->want_magic) { /* [false alarm]:移植开源代码 */
          /* Validate magic number */
          if (magic == lcp->want_magic) {

            PPP_MNTN_LOG2(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL,"Magic is same <1> <2>\r\n", (VOS_INT32)magic, ++lcp->LcpFailedMagic);
            lcp->want_magic = GenerateMagic();
            fsm_nak(dec, opt);


          } else {
            lcp->his_magic = magic;
            lcp->LcpFailedMagic = 0;
            fsm_ack(dec, opt);
          }
        } else {
          lcp->my_reject |= (1 << opt->hdr.id);
          fsm_rej(dec, opt);
        }
        break;
      case MODE_NAK:
        PPP_MNTN_LOG1(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, " Magic <1> is NAKed!\r\n", (VOS_INT32)magic);
        lcp->want_magic = GenerateMagic(); /* [false alarm]:移植开源代码 */
        break;
      case MODE_REJ:
        PPP_MNTN_LOG1(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, " Magic <1> is REJected!\r\n", (VOS_INT32)magic);
        lcp->want_magic = 0; /* [false alarm]:移植开源代码 */
        lcp->his_reject |= (1 << opt->hdr.id);
        break;
      }
      break;

    case TY_PROTOCOMP:
      PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "proto compress\r\n");

      switch (mode_type) {
      case MODE_REQ:
        if (IsAccepted(lcp->cfg.protocomp)) { /* [false alarm]:移植开源代码 */
          lcp->his_protocomp = 1;
          fsm_ack(dec, opt);
        } else {
          fsm_rej(dec, opt);
          lcp->my_reject |= (1 << opt->hdr.id);
        }
        break;
      case MODE_NAK:
      case MODE_REJ:
        lcp->want_protocomp = 0; /* [false alarm]:移植开源代码 */
        lcp->his_reject |= (1 << opt->hdr.id);
        break;
      }
      break;

    case TY_ACFCOMP:
      PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "acf compress\r\n");
      switch (mode_type) {
      case MODE_REQ:
        if (IsAccepted(lcp->cfg.acfcomp)) { /* [false alarm]:移植开源代码 */
          lcp->his_acfcomp = 1;
          fsm_ack(dec, opt);
        } else {
          fsm_rej(dec, opt);
          lcp->my_reject |= (1 << opt->hdr.id);
        }
        break;
      case MODE_NAK:
      case MODE_REJ:
        lcp->want_acfcomp = 0; /* [false alarm]:移植开源代码 */
        lcp->his_reject |= (1 << opt->hdr.id);
        break;
      }
      break;

    case TY_SDP:
      PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "sdp\r\n");
      switch (mode_type) {
      case MODE_REQ:
      case MODE_NAK:
      case MODE_REJ:
        break;
      }
      break;
    default:
      sz = (sizeof desc - 2) / 2;
      if (sz > opt->hdr.len - 2)
        sz = opt->hdr.len - 2;
      pos = 0;
      desc[0] = sz ? ' ' : '\0';
      for (pos = 0; sz--; pos++)
        sprintf(desc+(pos<<1)+1, "%02x", opt->data[pos]);

      PPP_MNTN_LOG1(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "unknow option code %d\r\n",opt->hdr.id);

      if (mode_type == MODE_REQ) {
        fsm_rej(dec, opt);
        lcp->my_reject |= (1 << opt->hdr.id); /* [false alarm]:移植开源代码 */
      }
      break;
    }
  }

  if (mode_type != MODE_NOP) {

/*fanzhibin f49086 add it begin*/
/*fanzhibin f49086 add it end*/

    if (mode_type == MODE_REQ && !lcp->mru_req) { /* [false alarm]:移植开源代码 */
      mru = DEF_MRU;
      if (mru < DEF_MRU) {
        /* Don't let the peer use the default MRU */
        lcp->his_mru = lcp->cfg.mtu && lcp->cfg.mtu < mru ? lcp->cfg.mtu : mru;
        nak.hdr.id = TY_MRU;
        nak.hdr.len = 4;
        ua_htons(&lcp->his_mru, nak.data);
        fsm_nak(dec, &nak);
        lcp->mru_req = 1;    /* Don't keep NAK'ing this */
      }
    }
    fsm_opt_normalise(dec);
  }
}

extern PPP_ZC_STRU *
lcp_Input(/*struct bundle *bundle, */struct link *l, PPP_ZC_STRU *pstMem)
{
  struct ppp_mbuf *bp;


  bp = ppp_m_get_from_ttfmem(pstMem);

  PPP_MemFree(pstMem);

  if (VOS_NULL_PTR == bp)
  {
    return VOS_NULL_PTR;
  }

  /* Got PROTO_LCP from link */
  PPP_MNTN_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, PS_PRINT_NORMAL, "lcp_Input\r\n");
  fsm_Input(&l->lcp.fsm, bp);
  return VOS_NULL_PTR;
}


