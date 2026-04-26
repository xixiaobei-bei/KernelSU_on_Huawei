/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, 
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

#ifndef __LCS_COMM_INTERFACE_H__
#define __LCS_COMM_INTERFACE_H__

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

#define LCS_CLIENT_EXID_MAX_LEN (20)
#define LCS_CLIENT_NAME_MAX_LEN (160)
#define LCS_MOLR_TIMEOUT_DEFAULT (180)
#define LCS_MOLR_INTERVAL_DEFAULT (180)
#define LCS_HOR_ACC_DEFAULT_VALUE (60)
#define LCS_VER_ACC_DEFAULT_VALUE (60)


enum LCS_Plane {
    LCS_PLANE_CONTROL = 0,
    LCS_PLANE_SECURE_USER,

    LCS_PLANE_BUTT
};
typedef VOS_UINT8 LCS_PlaneUint8;


enum LCS_MolrMethod {
    LCS_MOLR_METHOD_GPS = 0,           /* Unassisted GPS */
    LCS_MOLR_METHOD_AGPS,              /* Assisted GPS */
    LCS_MOLR_METHOD_AGNSS,             /* Assisted GANSS */
    LCS_MOLR_METHOD_AGPS_AGNSS,        /* Assisted GPS and GANSS */
    LCS_MOLR_METHOD_BASIC_SELF,        /* Basic self location */
    LCS_MOLR_METHOD_TRANSFER_TP_ADDR,  /* Transfer to third party */
    LCS_MOLR_METHOD_RETRIEVAL_TP_ADDR, /* Retrieval from third party */

    LCS_MOLR_METHOD_BUTT
};
typedef VOS_UINT8 LCS_MolrMethodUint8;


enum LCS_HorAccSet {
    LCS_HOR_ACC_NOT_SET = 0,
    LCS_HOR_ACC_SET_PARAM,

    LCS_HOR_ACC_SET_BUTT
};
typedef VOS_UINT8 LCS_HorAccSetUint8;


enum LCS_VerReq {
    LCS_VER_NOT_REQUESTED = 0,
    LCS_VER_REQUESTED,

    LCS_VER_REQ_BUTT
};
typedef VOS_UINT8 LCS_VerReqUint8;


enum LCS_VerAccSet {
    LCS_VER_ACC_NOT_SET = 0,
    LCS_VER_ACC_SET_PARAM,

    LCS_VER_ACC_SET_BUTT
};
typedef VOS_UINT8 LCS_VerAccSetUint8;


enum LCS_VelReq {
    LCS_VEL_NOT_REQUESTED = 0,   /* Velocity not requested */
    LCS_VEL_REQ_HOR,             /* Horizontal velocity requested */
    LCS_VEL_REQ_HOR_VER,         /* Horizontal velocity and vertical velocity requested */
    LCS_VEL_REQ_HOR_ACC,         /* Horizontal velocity with uncertainty requested */
    LCS_VEL_REQ_HOR_ACC_VER_ACC, /* Horizontal velocity with uncertainty and vertical velocity with uncertainty
                                    requested */

    LCS_VEL_REQ_BUTT
};
typedef VOS_UINT8 LCS_VelReqUint8;


enum LCS_RepMode {
    LCS_REP_MODE_SINGLE_RPT = 0,
    LCS_REP_MODE_PERIODIC_RPT,

    LCS_REP_MODE_BUTT
};
typedef VOS_UINT8 LCS_RepModeUint8;


enum LCS_MtlraOp {
    LCS_MTLRA_ALLOW = 0,
    LCS_MTLRA_NOT_ALLOW,

    LCS_MTLRA_OP_BUTT
};
typedef VOS_UINT8 LCS_MtlraOpUint8;


enum LCS_NotificationType {
    LCS_NOTIFICATION_ALLOW_THIRD_PARTY = 0,
    LCS_NOTIFICATION_PERMIT_IF_USER_IGNORE,
    LCS_NOTIFICATION_FORBIDDEN_IF_USER_IGNORE,

    LCS_NOTIFICATION_TYPE_BUTT
};
typedef VOS_UINT8 LCS_NotificationTypeUint8;


enum LCS_LocationType {
    LCS_LOCATION_CURRENT = 0,
    LCS_LOCATION_CURR_OR_LAST,
    LCS_LOCATION_INITIAL,

    LCS_LOCATION_TYPE_BUTT
};
typedef VOS_UINT8 LCS_LocationTypeUint8;


typedef struct {
    VOS_UINT8 ellipPoint : 1;
    VOS_UINT8 ellipPointUncertCircle : 1;
    VOS_UINT8 ellipPointUncertEllip : 1;
    VOS_UINT8 polygon : 1;
    VOS_UINT8 ellipPointAlt : 1;
    VOS_UINT8 ellipPointAltUncertEllip : 1;
    VOS_UINT8 ellipArc : 1;
    VOS_UINT8 spare : 1;
} LCS_ShapeRep;


typedef struct {
    VOS_UINT8 length;
    VOS_UINT8 reserved[3]; /* 保留字段，长度为3,便于4字节对齐 */
    VOS_UINT8 value[LCS_CLIENT_EXID_MAX_LEN];
    /* _H2ASN_Array2String */
} LCS_ClientExternalId;


typedef struct {
    VOS_UINT8 length;
    VOS_UINT8 reserved[3]; /* 保留字段，长度为3,便于4字节对齐 */
    VOS_UINT8 value[LCS_CLIENT_NAME_MAX_LEN];
    /* _H2ASN_Array2String */
} LCS_ClientName;


typedef struct {
    LCS_MolrMethodUint8  method;
    LCS_HorAccSetUint8   horAccSet;
    VOS_UINT8            horAcc;     /* horizontal-accuracy */

    LCS_VerReqUint8      verReq;     /* verticalCoordinate Request */
    LCS_VerAccSetUint8   verAccSet;
    VOS_UINT8            verAcc;     /* vertical-accuracy */
    LCS_VelReqUint8      velReq;     /* velocityRequest */

    LCS_RepModeUint8     repMode;    /* MO-LR等待网侧回复定时器时长 */
    VOS_UINT16           timeOut;    /* 周期上报时的上报间隔时长 */
    VOS_UINT16           interval;

    union {
        VOS_UINT8        shapeRep;
        LCS_ShapeRep     lcsShapeRep;
    } u;

    VOS_UINT8            reserved[3]; /* 保留字段，长度为3,便于4字节对齐 */
    LCS_ClientExternalId tPAddr;     /* Third Party Address */
} LCS_MOLR_Para;


typedef struct {
    VOS_UINT32 opClientExId : 1;
    VOS_UINT32 opClientName : 1;
    VOS_UINT32 opPlane : 1;
    VOS_UINT32 opSpare : 29;

    VOS_UINT8                 handleId;
    LCS_NotificationTypeUint8 ntfType;
    LCS_LocationTypeUint8     locationType;
    LCS_PlaneUint8            plane;

    LCS_ClientExternalId      clientExId;
    LCS_ClientName            clientName;
} LCS_MTLR_Para;

#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of lcs_comm_interface.h */
