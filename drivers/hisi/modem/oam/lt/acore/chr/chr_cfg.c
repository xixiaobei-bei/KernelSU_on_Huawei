
#include "vos.h"
#include "v_msg.h"
#include "chr_cfg.h"
#include "chr_om.h"
#include <mdrv_diag_system.h>
#include "msp_diag.h"
#define    THIS_FILE_ID        PS_FILE_ID_OM_ERRORLOG_C
/*保存黑名单的全局变量*/
CHR_ACPU_BLACK_SAVE_STRU g_stChrBlack;

/*保存优先级列表的全局变量*/
CHR_ACPU_PRIORITY_SAVE_STRU    *g_pstChrPriorityCfg;  //保存优先级0的配置表
OM_PRIORITY_LIST_STRU           g_stChrPrioritylist;    //临时记录AP下发的优先级多包数据

/*保存上报周期的全局变量*/
CHR_ACPU_PERIOD_SAVE_STRU      *g_pstChrPeriodCfg;     //保存上报周期的配置表
OM_PERIOD_LIST_STRU             g_stChrPeriodList;;     //临时记录AP下发的周期多包数据
OM_VCOM_DEBUG_INFO              g_stCfgVcomDebugInfo   = {0};
#define CHR_LogCfgReport(fmt, ...)  \
    DIAG_LogReport(DIAG_GEN_LOG_MODULE(0, 0, 1), MSP_PID_CHR, "chr_cfg", __LINE__, "%s:"fmt, __FUNCTION__, ##__VA_ARGS__)



VOS_UINT32 OM_AcpuBlackListSend(VOS_VOID)
{
    VOS_UINT32 ulBlackListLen;
    OM_ACPU_BLACK_LIST_STRU   *pstOmAcpuBlackList;

    /*A核申请黑名单列表消息发送给c核*/
    ulBlackListLen = g_stChrBlack.ulBlackListPacketLen 
                     + sizeof(OM_ACPU_BLACK_LIST_STRU) 
                     - VOS_MSG_HEAD_LENGTH;
    pstOmAcpuBlackList = (OM_ACPU_BLACK_LIST_STRU*)VOS_AllocMsg(MSP_PID_CHR,ulBlackListLen);
    if (VOS_NULL_PTR == pstOmAcpuBlackList)
    {
        chr_print("Alloc Req Msg failed!\n");
        return OM_APP_OMACPU_ALLOC_MSG_ERR;
    }
    
    pstOmAcpuBlackList->ulReceiverPid = MSP_PID_CHR_MDM;
    pstOmAcpuBlackList->ulPacketLen   = g_stChrBlack.ulBlackListPacketLen;
    pstOmAcpuBlackList->ulMsgName     = OM_ACPU_REPORT_BLACKLIST_NAME;

    VOS_MemCpy_s(&pstOmAcpuBlackList->stOmAcpuBlackList, 
                 g_stChrBlack.ulBlackListPacketLen,
                 g_stChrBlack.pstChrBlackList,
                 g_stChrBlack.ulBlackListPacketLen);
    
    /*a核发送黑名单给c核*/
    if(VOS_OK != VOS_SendMsg(MSP_PID_CHR, pstOmAcpuBlackList))
    {
        chr_print("send black list Msg failed!\n");
        return VOS_ERR;
    }
    chr_print("OM_AcpuBlackListSend success.\n");

    return VOS_OK;
}

VOS_UINT32 OM_AcpuBlackListProc(VOS_UINT8 *pucData, VOS_UINT32 ulLen)
{
    OM_APP_BLACK_LIST_STRU    *pstChrBlackList;
    VOS_UINT32 ulLength;
    VOS_UINT32 i;
    CHR_LIST_INFO_S *pstTemp = VOS_NULL;
    
    
    /*判断实际的消息长度和代码传入的是否一致*/
    pstChrBlackList =(OM_APP_BLACK_LIST_STRU *)pucData ;
    ulLength = pstChrBlackList->stOmHeader.ulMsgLen;
    CHR_LogCfgReport("OM_AcpuBlackListProc:MsgLen:0x%x,totallen:0x%x!\n",ulLength,ulLen);
    if(ulLen != (ulLength + sizeof(OM_ALARM_MSG_HEAD_STRU)))
    {
          chr_print("input length error !\n");
          return OM_APP_LENG_ERR;
    }
  
   
    /*检查黑名单的有效性*/
    for(i=0 ;i<(ulLength/sizeof(CHR_LIST_INFO_S)); i++)
    {
        if(pstChrBlackList->stBlackList[i].ulMsgModuleID > OM_ERR_LOG_MOUDLE_ID_BUTT ||
            pstChrBlackList->stBlackList[i].ulMsgModuleID < OM_ERR_LOG_MOUDLE_ID_GUNAS)
        {
            chr_print("param invalid !\n");
            /*返回错误结果*/
            return OM_APP_PARAM_INAVALID;          
        }
        CHR_LogCfgReport("blacklist moduleid:0x%x, alarmid:0x%x, AlarmType:%d!\n",
        pstChrBlackList->stBlackList[i].ulMsgModuleID,
        pstChrBlackList->stBlackList[i].usAlarmId,
        pstChrBlackList->stBlackList[i].usAlmType);
    }
   
    /*a核黑名单保存*/
    pstTemp = (CHR_LIST_INFO_S *)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT,ulLength);
    if(NULL == pstTemp)
    {
        chr_print("Alloc Req Msg failed!\n");
        return OM_APP_MEM_ALLOC_ERR;
    }
    VOS_MemCpy_s(pstTemp, ulLength, pstChrBlackList->stBlackList, ulLength);

    /*黑名单是否已经保存过*/
    if(NULL != g_stChrBlack.pstChrBlackList)
    {
        (VOS_VOID)VOS_MemFree(MSP_PID_CHR, g_stChrBlack.pstChrBlackList);
    }
    g_stChrBlack.pstChrBlackList    = pstTemp;
    g_stChrBlack.ulBlackListPacketLen = ulLength;
    /*先回复一个结果给apk，释放vcom口*/
    OM_AcpuSendAppcfgResult(OM_APP_MSG_OK);
    (VOS_VOID)OM_AcpuBlackListSend();
    
    return VOS_OK;
}


 
VOS_VOID OM_AcpuPriorityListInit(VOS_VOID)
{   
    g_stChrPrioritylist.ulSN = PRIORITY_PACKET_START;
    g_stChrPrioritylist.ulPrioPacketLen   = 0;
    blist_head_init(&g_stChrPrioritylist.List.PriorityList);

    return;
} 

VOS_VOID OM_AcpuPriorityListFree(VOS_VOID)
{   
    LIST_S* me = NULL;
    LIST_S* n = NULL;
    OM_PRIORITY_NODE_STRU *pChrSaveNode;
 
    blist_for_each_safe(me,n, &g_stChrPrioritylist.List.PriorityList)
    {
        pChrSaveNode = blist_entry(me, OM_PRIORITY_NODE_STRU, PriorityList);
        blist_del(&pChrSaveNode->PriorityList);
        VOS_MemFree(MSP_PID_CHR, pChrSaveNode);
    } 
    
    g_stChrPrioritylist.ulSN = PRIORITY_PACKET_START;
    g_stChrPrioritylist.ulPrioPacketLen   = 0;

    return;
} 

VOS_VOID OM_AcpuPriorityListAdd(OM_PRIORITY_NODE_STRU *pstPriorityNode)
{   
    blist_add_tail(&pstPriorityNode->PriorityList, &g_stChrPrioritylist.List.PriorityList);

    g_stChrPrioritylist.ulSN++;
    g_stChrPrioritylist.ulPrioPacketLen += pstPriorityNode->ulPacketLen;

    return;
} 

VOS_VOID OM_AcpuPriorityListEnd(VOS_VOID)
{   
    g_stChrPrioritylist.ulSN = PRIORITY_PACKET_END;
    return;
} 

 
VOS_UINT32 OM_AcpuCheckPriorityList(VOS_UINT8 *pucData)
{
    OM_APP_PRIORITY_CFG_STRU  *pstAppCheckPriorityCfg;
 
    
    pstAppCheckPriorityCfg =(OM_APP_PRIORITY_CFG_STRU *)pucData;
    /*判断传入长度和传入个数是否匹配*/
    if(pstAppCheckPriorityCfg->ucCount != (pstAppCheckPriorityCfg->stOmHeader.ulMsgLen-sizeof(VOS_UINT32))/(sizeof(CHR_PRIORITY_INFO_S)))
    {
        chr_print("input length not match count !\n");
        return PERIOD_CHK_FAIL;
    }
   
    
    /*检查序列号是否完整*/
    if((pstAppCheckPriorityCfg->ucPacketSN  != g_stChrPrioritylist.ulSN)&&
        (PRIORITY_PACKET_END   != pstAppCheckPriorityCfg->ucPacketSN) &&
        (PRIORITY_PACKET_START != pstAppCheckPriorityCfg->ucPacketSN))
    {
        chr_print("SN not match!\n");
        return PERIOD_CHK_FAIL;
    }

    if(PRIORITY_PACKET_START == pstAppCheckPriorityCfg->ucPacketSN)
    {
        chr_print("SN restart!\n");
        return PERIOD_CHK_RESTART;
    }
    
    if(PRIORITY_PACKET_END == pstAppCheckPriorityCfg->ucPacketSN)
    {
        chr_print("SN complete!\n");
        return PERIOD_CHK_COMPLETE;
    }
    
    return PERIOD_CHK_CONTINUE;
}


VOS_UINT32 OM_AcpuPriorityCfgSave(VOS_VOID)
{
    LIST_S* me = NULL;
    LIST_S* n = NULL;
    VOS_UINT8* pCurPrio;        
    VOS_UINT32 ulLen = 0;
    VOS_UINT32 ulTotalLen = 0;
    /*lint -save -e429*/ 
    OM_PRIORITY_NODE_STRU *pChrSaveNode;
    CHR_ACPU_PRIORITY_SAVE_STRU *pPrioTemp;
    ulTotalLen = g_stChrPrioritylist.ulPrioPacketLen+sizeof(CHR_ACPU_PRIORITY_SAVE_STRU);
      
    pPrioTemp = (CHR_ACPU_PRIORITY_SAVE_STRU*)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT, ulTotalLen);
    if(NULL == pPrioTemp)
    {
        return VOS_ERR;
    }
    VOS_MemSet_s(pPrioTemp, ulTotalLen, 0, ulTotalLen);
    
    pCurPrio = (VOS_UINT8*)pPrioTemp->PriorityCfg;
    blist_for_each_safe(me,n, &g_stChrPrioritylist.List.PriorityList)
    {
        pChrSaveNode = blist_entry(me, OM_PRIORITY_NODE_STRU, PriorityList);
        ulLen =  pChrSaveNode->ulPacketLen;
        
        VOS_MemCpy_s(pCurPrio, ulLen, pChrSaveNode->pstPriorityCfg, ulLen); 
        pCurPrio   += ulLen;
        pPrioTemp->ulPiorityLen += ulLen;

        if(pPrioTemp->ulPiorityLen > ulTotalLen)
        {
            VOS_MemFree(MSP_PID_CHR, pPrioTemp);
            return VOS_ERR;
        }
    }
    /*lint -restore +e429*/  
    OM_AcpuPriorityListFree();

    if(g_pstChrPriorityCfg)
    {
        VOS_MemFree(MSP_PID_CHR, g_pstChrPriorityCfg);
    }
    g_pstChrPriorityCfg = pPrioTemp;

    return VOS_OK;
}   

VOS_UINT32 OM_AcpuPriorityCfgSend(VOS_VOID)
{
    OM_ACPU_PRIORITY_CFG_STRU *pstOmAcpuPriorityCfg;
    
    /*A核申请优先级0列表消息发送给c核*/
    pstOmAcpuPriorityCfg  = (OM_ACPU_PRIORITY_CFG_STRU*)VOS_AllocMsg(MSP_PID_CHR,
               (g_pstChrPriorityCfg->ulPiorityLen +sizeof(OM_ACPU_PRIORITY_CFG_STRU) - VOS_MSG_HEAD_LENGTH));
    if (VOS_NULL_PTR == pstOmAcpuPriorityCfg)
    {
        chr_print("Alloc Req Msg failed!\n");
        return OM_APP_OMACPU_ALLOC_MSG_ERR;
    }
        
    pstOmAcpuPriorityCfg->ulReceiverPid = MSP_PID_CHR_MDM;
    pstOmAcpuPriorityCfg->ulMsgName     = OM_ACPU_REPORT_PRIORITY_NAME;
    pstOmAcpuPriorityCfg->ulPacketLen   = g_pstChrPriorityCfg->ulPiorityLen;  
    VOS_MemCpy_s(&pstOmAcpuPriorityCfg->PriorityCfg,
                  g_pstChrPriorityCfg->ulPiorityLen,
                  g_pstChrPriorityCfg->PriorityCfg,
                  g_pstChrPriorityCfg->ulPiorityLen);
        
    /*a核发送黑名单给c核*/
    if(VOS_OK != VOS_SendMsg(MSP_PID_CHR, pstOmAcpuPriorityCfg))
    {
        chr_print("send priority to ccpu fail !\n");
        return VOS_ERR;
    }

    chr_print("OM_AcpuPriorityCfgSend Send Priority Cfg success.\n");
    return VOS_OK;
}   


 VOS_UINT32 OM_AcpuPriorityCfgProc(VOS_UINT8 *pucData, VOS_UINT32 ulLen)
{
    OM_APP_PRIORITY_CFG_STRU  *pstAppOmPriorityCfg;
    OM_PRIORITY_NODE_STRU *pstPriorityNode;
    VOS_UINT32 ulLength;  
    VOS_UINT32 ulRet;
    VOS_UINT32 i;
    
    /*判断传入长度和实际的长度是否一致*/
    pstAppOmPriorityCfg =(OM_APP_PRIORITY_CFG_STRU *)pucData ;
    CHR_LogCfgReport("OM_AcpuPriorityCfgProc:MsgLen:0x%x,totallen:0x%x!\n",pstAppOmPriorityCfg->stOmHeader.ulMsgLen,ulLen);
    
    if(ulLen != (pstAppOmPriorityCfg->stOmHeader.ulMsgLen + sizeof(OM_ALARM_MSG_HEAD_STRU)))
    {
         chr_print("input length error !\n");
         return VOS_ERR;
    } 
    /*打印优先级*/
    for(i=0 ;i< pstAppOmPriorityCfg->ucCount; i++)
    {
       
        CHR_LogCfgReport("Priority moduleid:0x%x, alarmid:0x%x, AlarmType:0x%x,priority:0x%x!\n",
        pstAppOmPriorityCfg->alarmMap[i].ulModuleId,
        pstAppOmPriorityCfg->alarmMap[i].usAlarmId,
        pstAppOmPriorityCfg->alarmMap[i].usAlmType,
        pstAppOmPriorityCfg->alarmMap[i].priority );
    }
    /*检查优先级列表*/
    ulRet =OM_AcpuCheckPriorityList(pucData);
    if(PERIOD_CHK_FAIL == ulRet)
    {
        chr_print("param check err!\n");
        OM_AcpuPriorityListFree();
        return OM_APP_MEM_ALLOC_ERR ;
    }
    else if(PERIOD_CHK_RESTART == ulRet)
    {
        chr_print("Priority restart!\n");
        OM_AcpuPriorityListFree();
    }
    
    /* 创建节点，申请内存 */
    ulLength = pstAppOmPriorityCfg->stOmHeader.ulMsgLen-sizeof(VOS_UINT32);
    pstPriorityNode = (OM_PRIORITY_NODE_STRU *)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT,(ulLength +sizeof(OM_PRIORITY_NODE_STRU)));
    if(VOS_NULL_PTR == pstPriorityNode)
    {
        chr_print("list mem alloc error !\n");
        return OM_APP_MEM_ALLOC_ERR ;
    }
    VOS_MemCpy_s(pstPriorityNode->pstPriorityCfg, ulLength, pstAppOmPriorityCfg->alarmMap,ulLength);
    pstPriorityNode->ulPacketLen = ulLength;   
    /*先回复一个结果给apk，释放vcom口*/
    OM_AcpuSendAppcfgResult(OM_APP_MSG_OK);
    OM_AcpuPriorityListAdd(pstPriorityNode);
   
    if(PERIOD_CHK_COMPLETE == ulRet)
    {
        OM_AcpuPriorityListEnd();

        ulRet = OM_AcpuPriorityCfgSave();
        if(VOS_OK != ulRet)
        {
            chr_print("PriorityCfg Save error !\n");
            OM_AcpuPriorityListFree();
            return OM_APP_SAVE_LIST_FAIL;
        }
        
        ulRet = OM_AcpuPriorityCfgSend();
        if(VOS_OK != ulRet)
        {   
            chr_print("PriorityCfg Send error !\n");
            return OM_APP_SEND_CCPU_FAIL;

        }
    }
      
    return VOS_OK;
}   
 


VOS_VOID OM_AcpuPeriodListInit(VOS_VOID)
{   
    g_stChrPeriodList.ulSN = PRIORITY_PACKET_START;
    g_stChrPeriodList.ulPeriodPacketLen = 0;
    blist_head_init(&g_stChrPeriodList.List.PeriodList);

    return;
} 


VOS_VOID OM_AcpuPeriodListFree(VOS_VOID)
{   
    LIST_S* me = NULL;
    LIST_S* n = NULL;
    OM_PERIOD_NODE_STRU *pChrSaveNode;
 
    blist_for_each_safe(me, n,&g_stChrPeriodList.List.PeriodList)
    {
        pChrSaveNode = blist_entry(me, OM_PERIOD_NODE_STRU, PeriodList);
        blist_del(&pChrSaveNode->PeriodList);
        VOS_MemFree(MSP_PID_CHR, pChrSaveNode);
    } 
    
    g_stChrPeriodList.ulSN = PERIOD_PACKET_START;
    g_stChrPeriodList.ulPeriodPacketLen = 0;

    return;
} 


VOS_VOID OM_AcpuPeriodListAdd(OM_PERIOD_NODE_STRU *pstPeriodNode)
{   
    blist_add_tail(&pstPeriodNode->PeriodList, &g_stChrPeriodList.List.PeriodList);

    g_stChrPeriodList.ulSN++;
    g_stChrPeriodList.ulPeriodPacketLen += pstPeriodNode->ulPacketLen;

    return;
} 

VOS_VOID OM_AcpuPeriodListEnd(VOS_VOID)
{   
    g_stChrPeriodList.ulSN = PERIOD_PACKET_END;
    return;
} 

 
VOS_UINT32 OM_AcpuCheckPeriodList(VOS_UINT8 *pucData)
{
    OM_APP_PERIOD_CFG_STRU  *pstPeriodCfg;

    pstPeriodCfg =(OM_APP_PERIOD_CFG_STRU *)pucData;
    
   
    /*判断传入长度和传入个数是否匹配*/
    if(pstPeriodCfg->ucCount != (pstPeriodCfg->stOmHeader.ulMsgLen-sizeof(VOS_UINT32))/(sizeof(CHR_PERIOD_CFG_STRU)))
    {
        chr_print("input length not match count !\n");
        return PERIOD_CHK_FAIL;
    }
    
    /*检查序列号是否完整*/
    if((pstPeriodCfg->ucMsgSN != g_stChrPeriodList.ulSN)&&
        (PERIOD_PACKET_END   != pstPeriodCfg->ucMsgSN) &&
        (PERIOD_PACKET_START != pstPeriodCfg->ucMsgSN))
    {
        chr_print("SN not match %d!\n",PERIOD_PACKET_END);
        return PERIOD_CHK_FAIL;
    }

    if(PERIOD_PACKET_START == pstPeriodCfg->ucMsgSN)
    {
        chr_print("SN restart!\n");
        return PERIOD_CHK_RESTART;
    }
    
    if(PRIORITY_PACKET_END == pstPeriodCfg->ucMsgSN)
    {
        chr_print("SN complete!\n");
        return PERIOD_CHK_COMPLETE;
    }
    
    return PERIOD_CHK_CONTINUE;
}


VOS_UINT32 OM_AcpuPeriodCfgSave(VOS_VOID)
{
    LIST_S* me = NULL;
    LIST_S* n = NULL;
    VOS_UINT8* pCurPeriod;        
    VOS_UINT32 ulLen = 0;
    VOS_UINT32 ulTotalLen = 0;
    OM_PERIOD_NODE_STRU *pChrSaveNode;
    /*lint -save -e429*/ 
    CHR_ACPU_PERIOD_SAVE_STRU *pPeriodTemp;

    ulTotalLen = g_stChrPeriodList.ulPeriodPacketLen + sizeof(CHR_ACPU_PERIOD_SAVE_STRU);
    pPeriodTemp = (CHR_ACPU_PERIOD_SAVE_STRU*)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT, ulTotalLen);
    if(NULL == pPeriodTemp)
    {
        return VOS_ERR;
    }
    VOS_MemSet_s(pPeriodTemp, ulTotalLen, 0, ulTotalLen);
    
    pCurPeriod = (VOS_UINT8*)pPeriodTemp->PeriodCfg;
    blist_for_each_safe(me,n, &g_stChrPeriodList.List.PeriodList)
    {
        pChrSaveNode = blist_entry(me, OM_PERIOD_NODE_STRU, PeriodList);
        ulLen =  pChrSaveNode->ulPacketLen;
        
        VOS_MemCpy_s(pCurPeriod, ulLen, pChrSaveNode->pstPeriodCfg, ulLen); 
        pCurPeriod   += ulLen;
        pPeriodTemp->ulPacketLen += ulLen;
        pPeriodTemp->ulperiod  = pChrSaveNode->ulperiod;
        if(pPeriodTemp->ulPacketLen > ulTotalLen)
        {
            VOS_MemFree(MSP_PID_CHR,pPeriodTemp);
            return VOS_ERR;
        }
    }

    OM_AcpuPeriodListFree();

    if(g_pstChrPeriodCfg)
    {
        VOS_MemFree(MSP_PID_CHR, g_pstChrPeriodCfg);
    }
    g_pstChrPeriodCfg = pPeriodTemp;
    /*lint -restore +e429*/
    return VOS_OK;
}   

VOS_UINT32 OM_AcpuPeriodCfgSend(VOS_VOID)
{
    OM_ACPU_PERIOD_CFG_STRU *pstPeriodCfg;
    
    /*A核消息发送给c核*/
    pstPeriodCfg  = (OM_ACPU_PERIOD_CFG_STRU*)VOS_AllocMsg(MSP_PID_CHR,
               (g_pstChrPeriodCfg->ulPacketLen+sizeof(OM_ACPU_PERIOD_CFG_STRU) - VOS_MSG_HEAD_LENGTH));
    if (VOS_NULL_PTR == pstPeriodCfg)
    {
        chr_print("Alloc Req Msg failed!\n");
        return OM_APP_OMACPU_ALLOC_MSG_ERR;
    }
        
    pstPeriodCfg->ulReceiverPid = MSP_PID_CHR_MDM;
    pstPeriodCfg->ulMsgName     = OM_ACPU_REPORT_PERIOD_NAME;
    pstPeriodCfg->ulperiod      = g_pstChrPeriodCfg->ulperiod;
    pstPeriodCfg->ulPacketLen   = g_pstChrPeriodCfg->ulPacketLen;  
    VOS_MemCpy_s(&pstPeriodCfg->stOmAcpuPeriodCfg,
                  g_pstChrPeriodCfg->ulPacketLen,
                  g_pstChrPeriodCfg->PeriodCfg,
                  g_pstChrPeriodCfg->ulPacketLen);
        
    /*a核发送黑名单给c核*/
    if(VOS_OK != VOS_SendMsg(MSP_PID_CHR, pstPeriodCfg))
    {
        chr_print("send Period to ccpu fail !\n");
        return VOS_ERR;
    }
    chr_print("OM_AcpuPeriodCfgSend Send Period Cfg success.\n");

    return VOS_OK;
}   

VOS_UINT32 OM_AcpuPeriodCfgProc(VOS_UINT8 *pucData, VOS_UINT32 ulLen)
{
    OM_APP_PERIOD_CFG_STRU *pstAppPeriodCfg;
   
    OM_PERIOD_NODE_STRU  *pstPeriodNode;
    VOS_UINT32 ulLength;
    VOS_UINT32 ulRet;
    VOS_UINT32 i;
  
    /*判断传入长度和实际的长度是否一致*/
    pstAppPeriodCfg =(OM_APP_PERIOD_CFG_STRU *)pucData ;  
    
    CHR_LogCfgReport("OM_AcpuPeriodCfgProc:MsgLen:0x%x,totallen:0x%x!\n",pstAppPeriodCfg->stOmHeader.ulMsgLen,ulLen);
    if(ulLen != (pstAppPeriodCfg->stOmHeader.ulMsgLen + sizeof(OM_ALARM_MSG_HEAD_STRU)))
    {
         chr_print("input length error !\n");
         return VOS_ERR;
    }
   
    /*打印优先级*/
    for(i=0 ;i< pstAppPeriodCfg->ucCount; i++)
    {
       
        CHR_LogCfgReport("Priority moduleid:0x%x, count:0x%x!\n",
        pstAppPeriodCfg->alarmMap[i].ulModuleId,
        pstAppPeriodCfg->alarmMap[i].ulCount);
       
    }
    /*上报周期配置检查*/
    ulRet =OM_AcpuCheckPeriodList(pucData);
    if(PERIOD_CHK_FAIL == ulRet)
    {
        chr_print("param check err!\n");
        OM_AcpuPeriodListFree();
        return OM_APP_MEM_ALLOC_ERR ;
    }
    else if(PERIOD_CHK_RESTART == ulRet)
    {
        chr_print("Priority restart!\n");
        OM_AcpuPeriodListFree();
    }

    /* 创建节点，申请内存 */
    ulLength = pstAppPeriodCfg->stOmHeader.ulMsgLen-4*sizeof(VOS_UINT8);
    pstPeriodNode = (OM_PERIOD_NODE_STRU *)VOS_MemAlloc(MSP_PID_CHR, DYNAMIC_MEM_PT,(ulLength +sizeof(OM_PERIOD_NODE_STRU)));
    if(VOS_NULL_PTR == pstPeriodNode)
    {
        chr_print("list mem alloc error !\n");
        return OM_APP_MEM_ALLOC_ERR ;
    }
    VOS_MemCpy_s(pstPeriodNode->pstPeriodCfg, ulLength, pstAppPeriodCfg->alarmMap,ulLength);
    pstPeriodNode->ulPacketLen = ulLength;   
    pstPeriodNode->ulperiod    = (VOS_UINT32)pstAppPeriodCfg->ucperiod;
     /*先回复一个结果给apk，释放vcom口*/
    OM_AcpuSendAppcfgResult(OM_APP_MSG_OK);
    OM_AcpuPeriodListAdd(pstPeriodNode);

    if(PERIOD_CHK_COMPLETE == ulRet)
    {
        OM_AcpuPeriodListEnd();

        ulRet = OM_AcpuPeriodCfgSave();
        if(VOS_OK != ulRet)
        {
            chr_print("PeriodCfg Save error !\n");
            OM_AcpuPeriodListFree();
            return OM_APP_SAVE_LIST_FAIL;
        }
        
        ulRet = OM_AcpuPeriodCfgSend();
        if(VOS_OK != ulRet)
        {   
            chr_print("PeriodCfg Send error !\n");
            return OM_APP_SEND_CCPU_FAIL;

        }
    }
    
    return VOS_OK;
}   

VOS_VOID OM_AcpuSendAppcfgResult(VOS_UINT32 ulRest)
{
    OM_APP_RESULT_CFG_CNF_STRU               stOmAppCfgResultCnf={{0,},0};

   
    (VOS_VOID)VOS_MemSet_s(&stOmAppCfgResultCnf, sizeof(stOmAppCfgResultCnf), 0, (VOS_UINT32)sizeof(OM_APP_RESULT_CFG_CNF_STRU));
   
    stOmAppCfgResultCnf.stOmHeader.ulMsgType = OM_ERR_LOG_MSG_CFG_CNF;
    stOmAppCfgResultCnf.stOmHeader.ulMsgSN   = g_stCfgVcomDebugInfo.ulVCOMSendSn++;
    stOmAppCfgResultCnf.stOmHeader.ulMsgLen  = (sizeof(OM_APP_RESULT_CFG_CNF_STRU) - sizeof(OM_ALARM_MSG_HEAD_STRU));
    stOmAppCfgResultCnf.ulStatus             = ulRest;

    OM_AcpuSendVComData((VOS_UINT8 *)&stOmAppCfgResultCnf, (VOS_UINT32)sizeof(OM_APP_RESULT_CFG_CNF_STRU));

    return ;
}
/*****************************************************************************
 Function Name   : Chr_ResetCcoreCB
 Description     : 诊断modem单独复位回调函数
 Input           : enParam
 Output          : None
 Return          : VOS_VOID
*****************************************************************************/
VOS_INT chr_ResetCcoreCB(DRV_RESET_CB_MOMENT_E enParam, int userdata)
{
    
    CHR_APP_RESET_MSG_STRU *pstResetCcoreReq;
    
    if(enParam == MDRV_RESET_CB_AFTER) 
    {   
        chr_print("chr_ResetCcoreCB modem reset after!\n");
        pstResetCcoreReq  = (CHR_APP_RESET_MSG_STRU*)VOS_AllocMsg(MSP_PID_CHR,
                                 (sizeof(CHR_APP_RESET_MSG_STRU) - VOS_MSG_HEAD_LENGTH));
        if(NULL == pstResetCcoreReq)
        {
            return VOS_ERR;
        }
        pstResetCcoreReq->stMsgHeader.ulReceiverPid = MSP_PID_CHR;
        pstResetCcoreReq->stMsgHeader.ulMsgName     = CHR_ID_RESET_CCORE;
        if(VOS_OK != VOS_SendMsg(MSP_PID_CHR, pstResetCcoreReq))
        {
            chr_print("chr_ResetCcoreCB send msg fail !\n");
            return VOS_ERR;
        }
    }
    
    return VOS_OK;
}
/*****************************************************************************
 Function Name   : OM_AcpuResetProc
 Description     : 单独复位后a核处理函数
 Input           : enParam
 Output          : None
 Return          : VOS_VOID
*****************************************************************************/
VOS_UINT32 OM_AcpuResetProc(VOS_VOID)
{
   

    if((0!=g_stChrBlack.ulBlackListPacketLen) && (NULL!=g_stChrBlack.pstChrBlackList))
    {
        chr_print("OM_AcpuResetProc Send BlackList enter.\n");
        (VOS_VOID)OM_AcpuBlackListSend();
    }
    
    if(NULL != g_pstChrPriorityCfg)
    {        
        chr_print("OM_AcpuResetProc Send Priority Cfg enter.\n");
        (VOS_VOID)OM_AcpuPriorityCfgSend();
    }
    
    if(NULL != g_pstChrPeriodCfg)
    {
        chr_print("OM_AcpuResetProc Send Period Cfg enter.\n");
        (VOS_VOID)OM_AcpuPeriodCfgSend();
    }

    
    return VOS_OK;       
    
}

VOS_UINT32 CHR_Cfg_Init(VOS_VOID)
{  
    VOS_UINT32 ulRest;
    VOS_CHAR * resetName = "CHR";  /*C核单独复位的名字*/
 

    /*初始化优先级/上报周期节点*/
    OM_AcpuPriorityListInit();
    OM_AcpuPeriodListInit();
    
    ulRest = (VOS_UINT32)mdrv_sysboot_register_reset_notify(resetName, (pdrv_reset_cbfun)chr_ResetCcoreCB, 0, 49);
    if(ulRest)
    {
        chr_print("chr register ccore reset fail\n");
        return VOS_OK;
    }
    return VOS_OK;
}


VOS_VOID OM_AcpuBlackListShow(VOS_VOID)
{
    VOS_UINT32 i;
    if(NULL ==g_stChrBlack.pstChrBlackList)
    {
         chr_print("blacklist is null!\n");
         return ;
        
    }  
    for(i=0 ;i<(g_stChrBlack.ulBlackListPacketLen/sizeof(CHR_LIST_INFO_S)); i++)
    {
       
        chr_print("blacklist moudleid:0x%x,alarmid:0x%x,type:0x%x !\n",
            g_stChrBlack.pstChrBlackList[i].ulMsgModuleID,g_stChrBlack.pstChrBlackList[i].usAlarmId,g_stChrBlack.pstChrBlackList[i].usAlmType);
  
    } 
    return ;
}

VOS_VOID OM_AcpuPriority0Show(VOS_VOID)
{
    VOS_UINT32 i;
    if(NULL ==g_pstChrPriorityCfg)
    {
         chr_print("Priority0 is null!\n");
         return ;
        
    }  
    for(i=0 ;i<(g_pstChrPriorityCfg->ulPiorityLen/sizeof(CHR_PRIORITY_INFO_S)); i++)
    {
       
        chr_print("Priority0  moudleid:0x%x,alarmid:0x%x,priority:0x%x,type:0x%x !\n",
            g_pstChrPriorityCfg->PriorityCfg[i].ulModuleId, g_pstChrPriorityCfg->PriorityCfg[i].usAlarmId,
            g_pstChrPriorityCfg->PriorityCfg[i].priority,g_pstChrPriorityCfg->PriorityCfg[i].usAlmType);
    
    } 
    return ;
}



VOS_VOID OM_AcpuPeriodShow(VOS_VOID)
{
    VOS_UINT32 i;
    if(NULL ==g_pstChrPeriodCfg)
    {
         chr_print("PeriodCfg is null!\n");
         return ;
        
    }  
    for(i=0 ;i<(g_pstChrPeriodCfg->ulPacketLen/sizeof(CHR_PERIOD_CFG_STRU)); i++)
    {
        chr_print("period moudleid:0x%x,count:0x%x !\n",g_pstChrPeriodCfg->PeriodCfg[i].ulModuleId+0x20000, g_pstChrPeriodCfg->PeriodCfg[i].ulCount);
              
    } 
    return ;
}
