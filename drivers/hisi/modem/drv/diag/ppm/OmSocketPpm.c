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



/*****************************************************************************
  1 头文件包含
**************************************************************************** */
#include <product_config.h>
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <bsp_nvim.h>
#include <osl_thread.h>
#include <osl_types.h>
#include <nv_stru_drv.h>
#include <acore_nv_stru_drv.h>
#include <securec.h>
#include "hisocket.h"
#include "diag_port_manager.h"
#include "OmCommonPpm.h"
#include "OmSocketPpm.h"



/* ****************************************************************************
  2 全局变量定义
**************************************************************************** */
COMM_SOCKET_CTRL_INFO_STRU g_astSockInfo[SOCKET_BUTT] =
            {{SOCK_NULL, SOCK_NULL, {}, CPM_WIFI_OM_IND_PORT, SOCK_OM_IND_SRC_PORT_NUM,       {0,}},
             {SOCK_NULL, SOCK_NULL, {}, CPM_WIFI_OM_CFG_PORT, SOCK_OM_CFG_SRC_PORT_NUM,       {0,}},
             {SOCK_NULL, SOCK_NULL, {}, CPM_WIFI_AT_PORT,     SOCK_AT_SRC_PORT_NUM,           {0,}}
            };

/*****************************************************************************
  3 外部引用声明
*****************************************************************************/

#if (FEATURE_HISOCKET == FEATURE_ON)
/*****************************************************************************
  4 函数实现
*****************************************************************************/



/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*保存当前SOCKET的状态*/
u32          g_ulSockState = SOCK_OK;

/* 自旋锁，用来作自处理任务的临界资源保护 */
spinlock_t        g_stSockTaskSpinLock;

/*保存当前SOCKETOM的初始化状态*/
u32          g_ulSockOMInitState  = false;

/*保存当前SOCKETAT的初始化状态*/
u32          g_ulSockATInitState = false;

u32          g_ulOmIsReConn      = false;
u32          g_ulAtIsReConn      = false;


SOCKET_UDP_INFO_STRU g_stUdpInfo = {false, };


bool PPM_SockIsEnable(void)
{
    DIAG_CHANNLE_PORT_CFG_STRU    stPortCfg;

    /* 读取OM的物理输出通道 */
    if (NV_OK != bsp_nvm_read(NV_ID_DRV_DIAG_PORT, &stPortCfg, sizeof(DIAG_CHANNLE_PORT_CFG_STRU)))
    {
        return false;
    }

    //stPortCfg.enPortNum += CPM_APP_PORT;

    /* 检测参数*/
    if (CPM_OM_PORT_TYPE_WIFI != stPortCfg.enPortNum)
    {
        return false;
    }

    return true;
}



u32 PPM_SockUdpInit(void)
{
    COMM_SOCKET_CTRL_INFO_STRU *pstSockInfo = g_astSockInfo + SOCKET_OM_IND;
    pstSockInfo->socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (BSP_OK > pstSockInfo->socket)
    {
        return BSP_ERROR;
    }
    /* 创建SOCKET保护信号量 */
    if (CPM_WIFI_OM_IND_PORT == pstSockInfo->enPhyPort)
    {
        osl_sem_init(1, &pstSockInfo->SmClose);
    }
    else
    {
        return BSP_ERROR;
    }

    return BSP_OK;
}



bool PPM_SockTcpInit(COMM_SOCKET_CTRL_INFO_STRU *pstSockInfo)
{
    /* 创建SOCKET保护信号量 */
    if (CPM_WIFI_OM_CFG_PORT == pstSockInfo->enPhyPort)
    {
        osl_sem_init(1, &pstSockInfo->SmClose);
    }
    else if (CPM_WIFI_AT_PORT == pstSockInfo->enPhyPort)
    {
        osl_sem_init(1, &pstSockInfo->SmClose);
    }
    else
    {
        return false;
    }
    return true;
}


void PPM_SockBindListen(COMM_SOCKET_CTRL_INFO_STRU *pstSockInfo)
{
    struct sockaddr_in  sAddr;
    s32             lAddLen;

    if((SOCK_NULL != pstSockInfo->listener)||
       (SOCK_NULL != pstSockInfo->socket))
    {
        return;
    }

    pstSockInfo->listener = socket(AF_INET, SOCK_STREAM, 0);

    if (BSP_OK > pstSockInfo->listener)
    {
        ppm_printf("socket err !\n");
        pstSockInfo->listener = SOCK_NULL;
        msleep(100);/*delay 100 ms*/
        return ;
    }

    (void)memset_s(&sAddr, sizeof(sAddr), 0, sizeof(sAddr));
    sAddr.sin_family = AF_INET;

    sAddr.sin_addr.s_addr = 0;

    /* 监听的端口号 */
    sAddr.sin_port = htons(pstSockInfo->usPort);

    lAddLen = sizeof(struct sockaddr_in);

    /* 将监听Socket绑定到对应的端口上 */
    if (SOCKET_ERROR == bind(pstSockInfo->listener, (struct sockaddr *)&sAddr, lAddLen))
    {
        closesocket(pstSockInfo->listener);
        pstSockInfo->listener = SOCK_NULL;
        ppm_printf("bind err !\n");
        return ;
    }

    /* 设置服务器端监听的最大客户端数 */
    if (SOCKET_ERROR == listen(pstSockInfo->listener, SOCKET_NUM_MAX))
    {
        closesocket(pstSockInfo->listener);
        pstSockInfo->listener = SOCK_NULL;
        ppm_printf("listen err !\n");
        return ;
    }
}

void PPM_SockAcceptRecv(COMM_SOCKET_CTRL_INFO_STRU *pstSockInfo)
{
    Ip_fd_set                               listen1,listen2;
    s32                               ret;
    s32                               len;
    struct sockaddr_in                      from;
    unsigned long                               ulLockLevel;
    SOCKET                                  socket;
    SOCKET                                  maxSocket;

    if((SOCK_NULL == pstSockInfo->listener)&&(SOCK_NULL == pstSockInfo->socket))
    {
        ppm_printf("listener && socket err ! %d\n",pstSockInfo->usPort);
        return;
    }
    HI_FD_ZERO(&listen1);
    HI_FD_ZERO(&listen2);

    HI_FD_SET(pstSockInfo->listener, &listen1);

    if(SOCK_NULL != pstSockInfo->socket)
    {
        HI_FD_SET(pstSockInfo->socket, &listen1);
    }

    memcpy_s(&listen2, sizeof(listen2), &listen1,sizeof(listen1));

    maxSocket = pstSockInfo->socket > pstSockInfo->listener ? pstSockInfo->socket : pstSockInfo->listener;

    ret = select((int)maxSocket + 1,&listen1, NULL, &listen2, NULL);
    if(ret < 0)
    {
        ppm_printf("select err ! %d \n",pstSockInfo->usPort);
        return;
    }

    if((pstSockInfo->listener != SOCK_NULL) && (HI_FD_ISSET(pstSockInfo->listener, &listen1)))
    {
        len = sizeof(struct sockaddr_in);
        socket = accept(pstSockInfo->listener, (struct sockaddr *)&from, (int *)&len);
        if(socket < 0)
        {
            closesocket(pstSockInfo->listener);
            pstSockInfo->listener = SOCK_NULL;
            ppm_printf("accept err !%d \n",pstSockInfo->usPort);
        }
        else
        {
            if(SOCK_NULL != pstSockInfo->socket)
            {
                if(pstSockInfo->enPhyPort == CPM_WIFI_OM_CFG_PORT)
                {
                    g_ulOmIsReConn = true;
                }
                if(pstSockInfo->enPhyPort == CPM_WIFI_AT_PORT)
                {
                    g_ulAtIsReConn = true;
                }
                (void)osl_sem_down(pstSockInfo->SmClose, 0);
                closesocket(pstSockInfo->socket);
                (void)osl_sem_up(pstSockInfo->SmClose);
            }

            (void)osl_sem_down(pstSockInfo->SmClose, 0);
            pstSockInfo->socket = socket;
            (void)osl_sem_up(pstSockInfo->SmClose);

            if(CPM_WIFI_OM_CFG_PORT == pstSockInfo->enPhyPort)
            {
                if(0 > getpeername(pstSockInfo->socket, &g_stUdpInfo.stAddr, (int *)&len))
                {
                    ppm_printf("getpeername failed.\n");
                }
                else
                {
                    /* 获取对端地址后，修改端口号，用于UDP传输 */
                    g_stUdpInfo.stAddr.sin_port = htons(g_astSockInfo[SOCKET_OM_IND].usPort);

                    g_stUdpInfo.bStatus = true;
                }
            }
        }
    }

    if(SOCK_NULL == pstSockInfo->socket)
    {
        ppm_printf("SOCK_NULL == pstSockInfo->socket ,%d!\n",pstSockInfo->usPort);
        return;
    }

    ret = recv((int)pstSockInfo->socket, pstSockInfo->aucBuf, SOCK_OM_MSG_LEN, 0);
    if(ret <= 0)    /*客户端断开之后服务端会持续受到长度为0的数据包*/
    {
        (void)osl_sem_down(pstSockInfo->SmClose, 0);
        shutdown(pstSockInfo->socket,SHUT_RDWR);
        pstSockInfo->socket   = SOCK_NULL;
        (void)osl_sem_up(pstSockInfo->SmClose);
        if(pstSockInfo->enPhyPort == CPM_WIFI_OM_CFG_PORT)
        {
            PPM_DisconnectAllPort(OM_LOGIC_CHANNEL_CNF);

            g_stUdpInfo.bStatus = false;
        }
        ppm_printf("rcv err %d!\n",pstSockInfo->usPort);
        return;
    }

    if(pstSockInfo->enPhyPort == CPM_WIFI_OM_CFG_PORT)
    {
        OM_ACPU_DEBUG_CHANNEL_TRACE(pstSockInfo->enPhyPort, (u8*)pstSockInfo->aucBuf, ret, OM_ACPU_RECV_SOCKET, OM_ACPU_DATA);
    }

    scm_SpinLockIntLock(&g_stSockTaskSpinLock, ulLockLevel);
    /*将接收到的数据提交给上层处理*/
    ret = (s32)CPM_ComRcv(pstSockInfo->enPhyPort, pstSockInfo->aucBuf, ret);
    if(ret)
    {
        ppm_printf("CPM_ComRcv error, ret = 0x%x\n", ret);
    }

    scm_SpinUnlockIntUnlock(&g_stSockTaskSpinLock, ulLockLevel);
}


void PPM_SockServerProc(COMM_SOCKET_CTRL_INFO_STRU *pstSockInfo)
{
    /* coverity[no_escape] */
    for(;;)
    {
        PPM_SockBindListen(pstSockInfo);
        PPM_SockAcceptRecv(pstSockInfo);
    }

}


int PPM_SockOmServerTask(void* parm)
{
	UNUSED(parm);
    /* 初始化CFG口的SOCKET，采用TCP协议 */
    if (false == PPM_SockTcpInit(g_astSockInfo + SOCKET_OM_CFG))
    {
        return 0;
    }

    g_ulSockState = SOCK_START;
 
	g_ulSockOMInitState = true;
    /* CFG口自处理循环处理入口 */
    PPM_SockServerProc(g_astSockInfo + SOCKET_OM_CFG);

    return 0;
}



int PPM_SockAtServerTask(void* parm)
{
	UNUSED(parm);
    /* 初始化SOCKET */
    if (false == PPM_SockTcpInit(g_astSockInfo + SOCKET_AT))
    {
        return 0;
    }

    g_ulSockATInitState = true;

    PPM_SockServerProc(g_astSockInfo + SOCKET_AT);

    return 0;
}

/* UDP单包包长最大65535字节，减去UDP包头后最大65507个字节 */
#define UDP_MAX_LEN	    65507

/* 获取UDP端口发送的信息 */
u32 PPM_SocketGetUdpInfo(void)
{
    return g_stUdpInfo.ulTotalLen;
}



s32 PPM_SockOMIndComSend(u8 *pucVirAddr, u8 *pucPhyAddr, u16 uslength)
{
    SOCKET                              socket;
    s32                             nSndNum;
    u16  ustimes = 0, uslen = 0, i;

    (void)osl_sem_down(g_astSockInfo[SOCKET_OM_IND].SmClose, 0);

    socket = g_astSockInfo[SOCKET_OM_IND].socket;

    if ((SOCK_NULL == socket) || (false == g_stUdpInfo.bStatus))
    {
        (void)osl_sem_up(g_astSockInfo[SOCKET_OM_IND].SmClose);

        return BSP_ERROR;
    }

    if(uslength > UDP_MAX_LEN)
    {
        ustimes = uslength / UDP_MAX_LEN;
        uslen   = uslength % UDP_MAX_LEN;
    }
    else
    {
        ustimes = 0;
        uslen   = uslength;
    }

    /* 调用sendto将数据通过socket发送出去，走UDP */
    /* coverity[overrun-buffer-val] */
    for(i = 0; i < ustimes; i++)
    {
        diag_system_debug_socket_len(UDP_MAX_LEN);

        /* coverity[overrun-buffer-val] */
        nSndNum = sendto(socket, (pucVirAddr + (i*UDP_MAX_LEN)), UDP_MAX_LEN, 0, &g_stUdpInfo.stAddr, sizeof(g_stUdpInfo.stAddr));

        if (nSndNum != UDP_MAX_LEN)
        {
            diag_system_debug_socket_fail_len(UDP_MAX_LEN);

            (void)ppm_printf("uslength %d, nSndNum %d.\n", UDP_MAX_LEN, nSndNum);

            (void)osl_sem_up(g_astSockInfo[SOCKET_OM_IND].SmClose);

            return BSP_ERROR;
        }
        else
        {
            diag_system_debug_socket_sucess_len(UDP_MAX_LEN);
            g_stUdpInfo.ulTotalLen += nSndNum;
        }
    }

    if(0 != uslen)
    {
        diag_system_debug_socket_len(uslen);
        nSndNum = sendto(socket, (pucVirAddr + (ustimes*UDP_MAX_LEN)), uslen, 0, &g_stUdpInfo.stAddr, sizeof(g_stUdpInfo.stAddr));

        if (nSndNum != uslen)
        {
            diag_system_debug_socket_fail_len(uslen);

            (void)ppm_printf("uslength %d, nSndNum %d.\n", uslen, nSndNum);

            (void)osl_sem_up(g_astSockInfo[SOCKET_OM_IND].SmClose);

            return BSP_ERROR;
        }
        else
        {
            diag_system_debug_socket_sucess_len(uslen);

            g_stUdpInfo.ulTotalLen += nSndNum;
        }
    }
    diag_system_debug_send_data_end();

    (void)osl_sem_up(g_astSockInfo[SOCKET_OM_IND].SmClose);

    return BSP_OK;
}



s32 PPM_SockOMCfgComSend(u8* pucVirAddr, u8 *pucPhyAddr, u32 uslength)
{
    SOCKET socket;
    s32 nSndNum;

    if (false == g_ulSockOMInitState)
    {
        return BSP_ERROR;
    }

    (void)osl_sem_down(g_astSockInfo[SOCKET_OM_CFG].SmClose, 0);

    socket = g_astSockInfo[SOCKET_OM_CFG].socket;

    if (SOCK_NULL == socket)
    {
        (void)osl_sem_up(g_astSockInfo[SOCKET_OM_CFG].SmClose);
        (void)ppm_printf("socket err !\n");
        return BSP_ERROR;
    }
    OM_ACPU_DEBUG_CHANNEL_TRACE(CPM_WIFI_OM_CFG_PORT, (u8*)pucVirAddr, uslength, OM_ACPU_SEND_SOCKET, OM_ACPU_DATA);

    /* 调用send将数据通过socket发送出去，走TCP */
    nSndNum = send(socket, pucVirAddr, uslength, 0);

    (void)osl_sem_up(g_astSockInfo[SOCKET_OM_CFG].SmClose);

    if (nSndNum != uslength)
    {
        (void)ppm_printf("send err %d,%d!\n",nSndNum,uslength);
        if(g_ulOmIsReConn == true)
        {
            (void)ppm_printf("Re Conn %d,%d!\n",nSndNum,uslength);
            return BSP_ERROR;
        }
        PPM_DisconnectAllPort(OM_LOGIC_CHANNEL_CNF);

        g_stUdpInfo.bStatus = false;

        (void)osl_sem_down(g_astSockInfo[SOCKET_OM_CFG].SmClose, 0);

        closesocket(g_astSockInfo[SOCKET_OM_CFG].socket);

        g_astSockInfo[SOCKET_OM_CFG].socket = SOCK_NULL;

        (void)osl_sem_up(g_astSockInfo[SOCKET_OM_CFG].SmClose);

        return BSP_ERROR;
    }

    g_ulOmIsReConn = false;

    return BSP_OK;
}


s32 PPM_SockATComSend(u8* pucVirAddr, u8 *pucPhyAddr, u32 uslength)
{
    SOCKET socket;
    s32 nSndNum;

    if (false == g_ulSockATInitState)
    {
        return BSP_ERROR;
    }

    (void)osl_sem_down(g_astSockInfo[SOCKET_AT].SmClose, 0);

    socket = g_astSockInfo[SOCKET_AT].socket;

    if (SOCK_NULL == socket)
    {
        (void)osl_sem_up(g_astSockInfo[SOCKET_AT].SmClose);
        (void)ppm_printf("socket err !\n");
        return BSP_ERROR;
    }

    /* 调用send将数据通过socket发送出去，走TCP */
    nSndNum = send(socket, pucVirAddr, uslength, MSG_DONTWAIT);

    (void)osl_sem_up(g_astSockInfo[SOCKET_AT].SmClose);
    if (nSndNum != uslength)
    {
        ppm_printf("send err %d,%d!\n",nSndNum,uslength);
        if(g_ulAtIsReConn == true)
        {
            ppm_printf("Re Conn %d,%d!\n",nSndNum,uslength);
            return BSP_ERROR;
        }
        (void)osl_sem_down(g_astSockInfo[SOCKET_AT].SmClose, 0);
        closesocket(g_astSockInfo[SOCKET_AT].socket);
        g_astSockInfo[SOCKET_AT].socket =SOCK_NULL;
        (void)osl_sem_up(g_astSockInfo[SOCKET_AT].SmClose);
        return BSP_ERROR;
    }

    g_ulAtIsReConn = false;

    return BSP_OK;
}




u32 PPM_SockPortInit(void)
{
    if(PPM_SockInitTask())
    {
        return BSP_ERROR;
    }

    if ((BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
        && (true == PPM_SockIsEnable()))
    {
        /*CPM_PhySendReg(CPM_WIFI_OM_IND_PORT,    (CPM_SEND_FUNC)PPM_SockOMIndComSend);*/
        CPM_PhySendReg(CPM_WIFI_OM_IND_PORT,    (CPM_SEND_FUNC)PPM_SockOMIndComSend);
        CPM_PhySendReg(CPM_WIFI_OM_CFG_PORT,    (CPM_SEND_FUNC)PPM_SockOMCfgComSend);
        CPM_PhySendReg(CPM_WIFI_AT_PORT,        (CPM_SEND_FUNC)PPM_SockATComSend);
    }

    scm_SpinLockInit(&g_stSockTaskSpinLock);

    return BSP_OK;
}

u32 PPM_SockInitTask(void)
{
    
    u32 ulRelVal = 0;
    OSL_TASK_ID task_id = 0;

    if((BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
    && (true == PPM_SockIsEnable()))
    {
        /* 接收SOCKET数据的自处理任务 */
        ulRelVal = (u32)osl_task_init("om_server", 76, 8096,(OSL_TASK_FUNC)PPM_SockOmServerTask, NULL,(OSL_TASK_ID*)&task_id);
        if (ulRelVal )
        {
            ppm_printf("creta sock om server task fail\n");
            return BSP_ERROR;
        }

        /* 接收SOCKET数据的自处理任务 */
        ulRelVal = (u32)osl_task_init("om_server", 76, 8096,(OSL_TASK_FUNC)PPM_SockAtServerTask, NULL,(OSL_TASK_ID*)&task_id);
        if (ulRelVal )
        {
            ppm_printf("creta sock at server task fail\n");
            return BSP_ERROR;
        }

        PPM_SockUdpInit();

    }

}
#endif






