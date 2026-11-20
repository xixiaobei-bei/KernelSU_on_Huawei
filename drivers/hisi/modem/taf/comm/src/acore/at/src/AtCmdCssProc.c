

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "AtCmdCssProc.h"
#include "ATCmdProc.h"
#include "PsCommonDef.h"
#include "AtEventReport.h"


#define    THIS_FILE_ID                 PS_FILE_ID_AT_CMD_CSS_PROC_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*AT与CSS模块间消息处理函数指针*/
const AT_CSS_MSG_PRO_FUNC_STRU g_astAtCssMsgTab[] =
{
    /* 消息ID */                            /* 消息处理函数 */
    {ID_CSS_AT_MCC_INFO_SET_CNF,              AT_RcvCssMccInfoSetCnf},
    {ID_CSS_AT_MCC_VERSION_INFO_CNF,          AT_RcvCssMccVersionQryCnf},
    {ID_CSS_AT_QUERY_MCC_INFO_NOTIFY,         AT_RcvCssMccNotify},

    {ID_CSS_AT_BLACK_CELL_LIST_SET_CNF,       AT_RcvCssBlackCellListSetCnf},
    {ID_CSS_AT_BLACK_CELL_LIST_QUERY_CNF,     AT_RcvCssBlackCellListQryCnf},
    {ID_CSS_AT_BLACK_CELL_MCC_NOTIFY,         AT_RcvCssBlackCellMccNotify},

    {ID_CSS_AT_LINE_INDEX_LIST_SET_CNF,       AT_RcvCssLineIndexListSetCnf},
    {ID_CSS_AT_LINE_INDEX_LIST_QUERY_CNF,     AT_RcvCssLineIndexListQryCnf},
    {ID_CSS_AT_LINE_DETAIL_SET_CNF,           AT_RcvCssLineDetailSetCnf},
    {ID_CSS_AT_LINE_PLMN_NOTIFY,              AT_RcvCssLinePlmnNotify},
    {ID_CSS_AT_LINE_INDEX_NOTIFY,             AT_RcvCssLineIndexNotify},


};

/*****************************************************************************
  3 函数实现
*****************************************************************************/

VOS_VOID AT_ProcCssMsg(
    CssAtInterface_MSG                 *pstMsg
)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulMsgCnt;
    VOS_UINT32                          ulMsgId;
    VOS_UINT32                          ulRst;

    /*从g_astAtProcMsgFromImsaTab中获取消息个数*/
    ulMsgCnt = sizeof(g_astAtCssMsgTab)/sizeof(AT_CSS_MSG_PRO_FUNC_STRU);

    /*从消息包中获取MSG ID*/
    ulMsgId  = pstMsg->stMsgData.ulMsgId;

    /*g_astAtProcMsgFromCssTab查表，进行消息分发*/
    for (i = 0; i < ulMsgCnt; i++)
    {
        if (g_astAtCssMsgTab[i].ulMsgId == ulMsgId)
        {
            ulRst = g_astAtCssMsgTab[i].pProcMsgFunc(pstMsg);

            if (VOS_ERR == ulRst)
            {
                AT_ERR_LOG("AT_ProcCssMsg: Msg Proc Err!");
            }

            return;
        }
    }

    /*没有找到匹配的消息*/
    if (ulMsgCnt == i)
    {
        AT_ERR_LOG("AT_ProcCssMsg: Msg Id is invalid!");
    }

    return;
}


VOS_UINT32 AT_RcvCssMccInfoSetCnf(
    VOS_VOID                           *pstMsg
)
{
    /* 定义局部变量 */
    CSS_AT_MCC_INFO_SET_CNF_STRU       *pstMccInfoSetCnf    = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;
    VOS_UINT32                          ulRetVal;

    /* 初始化消息变量 */
    ucIndex          = 0;
    ulRetVal         = VOS_ERR;
    ulResult         = AT_ERROR;
    pstMccInfoSetCnf = (CSS_AT_MCC_INFO_SET_CNF_STRU *)pstMsg;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstMccInfoSetCnf->usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvCssMccInfoSetCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvCssMccInfoSetCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_MCCFREQ_SET */
    if (AT_CMD_MCCFREQ_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvCssMccInfoSetCnf: WARNING:Not AT_CMD_MCCFREQ_SET!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 判断查询操作是否成功 */
    if (VOS_OK == pstMccInfoSetCnf->ulResult)
    {
        ulResult    = AT_OK;
        ulRetVal    = VOS_OK;
    }

    gstAtSendData.usBufLen = 0;

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulResult);

    return ulRetVal;
}



VOS_UINT32 AT_RcvCssMccVersionQryCnf(
    VOS_VOID                           *pstMsg
)
{
    /* 定义局部变量 */
    CSS_AT_MCC_VERSION_INFO_CNF_STRU   *pstMccVersionInfoCnf    = VOS_NULL_PTR;
    VOS_UINT8                           aucVersionId[MCC_INFO_VERSION_LEN+1] = {0};
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* 初始化消息变量 */
    ucIndex     = 0;
    pstMccVersionInfoCnf = (CSS_AT_MCC_VERSION_INFO_CNF_STRU *)pstMsg;

    /* 通过ClientId获取ucIndex */
    if ( AT_FAILURE == At_ClientIdToUserId(pstMccVersionInfoCnf->usClientId, &ucIndex) )
    {
        AT_WARN_LOG("AT_RcvCssMccVersionQryCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvCssMccVersionQryCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_MCCFREQ_QRY */
    if ( AT_CMD_MCCFREQ_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt )
    {
        AT_WARN_LOG("AT_RcvCssMccVersionQryCnf: WARNING:Not AT_CMD_MCCFREQ_QRY!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 构造Version String */
    TAF_MEM_CPY_S(aucVersionId, sizeof(aucVersionId), pstMccVersionInfoCnf->aucVersionId, MCC_INFO_VERSION_LEN);

    /* 判断查询操作是否成功 */
    gstAtSendData.usBufLen= (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   "%s: %s",
                                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                   (VOS_CHAR *)aucVersionId);

    ulResult = AT_OK;

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}




VOS_UINT32 AT_RcvCssMccNotify(
    VOS_VOID                           *pstMsg
)
{
    /* 定义局部变量 */
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           i;
    VOS_UINT8                           aucMccStr[AT_CSS_MAX_MCC_ID_NUM*AT_MCC_PLUS_COMMA_LENGTH] = {0};
    VOS_UINT8                           aucVersionId[MCC_INFO_VERSION_LEN+1] = {0};
    CSS_AT_QUERY_MCC_INFO_NOTIFY_STRU  *pstCssMccNty    = VOS_NULL_PTR;

    /* 初始化消息变量 */
    ucIndex      = 0;
    pstCssMccNty = (CSS_AT_QUERY_MCC_INFO_NOTIFY_STRU *)pstMsg;

    /* 通过ClientId获取ucIndex */
    if ( AT_FAILURE == At_ClientIdToUserId(pstCssMccNty->usClientId, &ucIndex) )
    {
        AT_WARN_LOG("AT_RcvCssMccNotify: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* MCC个数不对 */
    if ((0 == pstCssMccNty->ulMccNum)
     || (AT_CSS_MAX_MCC_ID_NUM < pstCssMccNty->ulMccNum))
    {
        AT_WARN_LOG("AT_RcvCssMccNotify: WARNING:INVALID MCC NUM!");
        return VOS_ERR;
    }

    /* 构造上报给Ril的MCC字符串 */
    for (i = 0; i < pstCssMccNty->ulMccNum; i++)
    {
        aucMccStr[AT_MCC_PLUS_COMMA_LENGTH*i]   = (pstCssMccNty->astMccId[i].aucMcc[0] & 0x0f) + '0';
        aucMccStr[AT_MCC_PLUS_COMMA_LENGTH*i+1] = ((pstCssMccNty->astMccId[i].aucMcc[0] & 0xf0) >> 4) + '0';
        aucMccStr[AT_MCC_PLUS_COMMA_LENGTH*i+2] = (pstCssMccNty->astMccId[i].aucMcc[1] & 0x0f) + '0';
        aucMccStr[AT_MCC_PLUS_COMMA_LENGTH*i+3] = ',';
    }

    /* 字符串结束符 */
    aucMccStr[(VOS_UINT8)(pstCssMccNty->ulMccNum)*AT_MCC_PLUS_COMMA_LENGTH-1] = 0;

    /* 构造Version String */
    TAF_MEM_CPY_S(aucVersionId, sizeof(aucVersionId), pstCssMccNty->aucVersionId, MCC_INFO_VERSION_LEN);

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s %s,%s%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_MCC].pucText,
                                                    (VOS_CHAR *)aucVersionId,
                                                    (VOS_CHAR *)aucMccStr,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvCssBlackCellListSetCnf(
    VOS_VOID                           *pstMsg
)
{
    /* 定义局部变量 */
    CSS_AT_BLACK_CELL_LIST_SET_CNF_STRU                    *pstCssSetCnf    = VOS_NULL_PTR;
    VOS_UINT32                                              ulResult;
    VOS_UINT8                                               ucIndex;

    /* 初始化消息变量 */
    pstCssSetCnf = (CSS_AT_BLACK_CELL_LIST_SET_CNF_STRU *)pstMsg;

    ucIndex      = 0;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstCssSetCnf->usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvCssBlackCellListSetCnf: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvCssBlackCellListSetCnf: WARNING:AT_BROADCAST_INDEX!");

        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_CLOUDBLACKLIST_SET */
    if (AT_CMD_BLACKCELLLIST_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvCssBlackCellListSetCnf: WARNING:Not AT_CMD_BLACKCELLLIST_SET!");

        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    ulResult = AT_ERROR;

    if (VOS_OK == pstCssSetCnf->ulResult)
    {
        ulResult    = AT_OK;
    }

    gstAtSendData.usBufLen = 0;

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvCssBlackCellListQryCnf(
    VOS_VOID                           *pstMsg
)
{
    CSS_AT_BLACK_CELL_LIST_QUERY_CNF_STRU                  *pstCssQryCnf           = VOS_NULL_PTR;
    VOS_UINT8                                               aucVersionId[AT_CSS_BLACK_CELL_LIST_VERSION_LEN + 1];
    VOS_UINT32                                              ulMcc;
    VOS_UINT32                                              ulLoop;
    VOS_UINT16                                              usLength;
    VOS_UINT8                                               ucIndex;

    /* 初始化消息变量 */
    pstCssQryCnf = (CSS_AT_BLACK_CELL_LIST_QUERY_CNF_STRU *)pstMsg;

    ucIndex   = 0;

    /* 通过ClientId获取ucIndex */
    if ( AT_FAILURE == At_ClientIdToUserId(pstCssQryCnf->usClientId, &ucIndex) )
    {
        AT_WARN_LOG("AT_RcvCssBlackCellListQryCnf: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvCssBlackCellListQryCnf: WARNING:AT_BROADCAST_INDEX!");

        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_CLOUDBLACKLIST_QRY */
    if ( AT_CMD_BLACKCELLLIST_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt )
    {
        AT_WARN_LOG("AT_RcvCssBlackCellListQryCnf: WARNING:Not AT_CMD_BLACKCELLLIST_QRY!");

        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* MCC个数不对 */
    if (AT_CSS_MAX_MCC_ID_NUM < pstCssQryCnf->stMccInfo.ulMccNum)
    {
        AT_WARN_LOG("AT_RcvCssBlackCellListQryCnf: WARNING:INVALID MCC NUM!");

        At_FormatResultData(ucIndex, AT_ERROR);

        return VOS_ERR;
    }

    /* 构造上报给Ril的Version */
    TAF_MEM_SET_S(aucVersionId, sizeof(aucVersionId), 0, sizeof(aucVersionId));
    TAF_MEM_CPY_S(aucVersionId, sizeof(aucVersionId), pstCssQryCnf->aucVersionId, sizeof(pstCssQryCnf->aucVersionId));

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: \"%s\"",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       aucVersionId);

    /* 构造上报给Ril的MCC字符串 */
    for (ulLoop = 0; ulLoop < pstCssQryCnf->stMccInfo.ulMccNum; ulLoop++)
    {
        ulMcc = 0;

        AT_ConvertNasMccToBcdType(pstCssQryCnf->stMccInfo.aulMcc[ulLoop], &ulMcc);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%x%x%x",
                                           (ulMcc & 0x0f00)>>8,
                                           (ulMcc & 0xf0)>>4,
                                           (ulMcc & 0x0f));
    }

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_RcvCssBlackCellMccNotify(
    VOS_VOID                           *pstMsg
)
{
    /* 定义局部变量 */
    CSS_AT_BLACK_CELL_MCC_NOTIFY_STRU  *pstCssNty = VOS_NULL_PTR;
    VOS_UINT8                           aucVersionId[AT_CSS_BLACK_CELL_LIST_VERSION_LEN + 1];
    VOS_UINT32                          ulMcc;
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    /* 初始化消息变量 */
    pstCssNty = (CSS_AT_BLACK_CELL_MCC_NOTIFY_STRU *)pstMsg;

    ucIndex   = 0;

    /* 通过ClientId获取ucIndex */
    if ( AT_FAILURE == At_ClientIdToUserId(pstCssNty->usClientId, &ucIndex) )
    {
        AT_WARN_LOG("AT_RcvCssBlackCellMccNotify: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    /* MCC个数不对 */
    if ((0 == pstCssNty->stMccInfo.ulMccNum)
     || (AT_CSS_MAX_MCC_ID_NUM < pstCssNty->stMccInfo.ulMccNum))
    {
        AT_WARN_LOG("AT_RcvCssBlackCellMccNotify: WARNING:INVALID MCC NUM!");

        return VOS_ERR;
    }

    /* 构造上报给Ril的Version */
    TAF_MEM_SET_S(aucVersionId, sizeof(aucVersionId), 0, sizeof(aucVersionId));
    TAF_MEM_CPY_S(aucVersionId, sizeof(aucVersionId), pstCssNty->aucVersionId, sizeof(pstCssNty->aucVersionId));

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s\"%s\"",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_BLACK_CELL_MCC].pucText,
                                       aucVersionId);

    /* 构造上报给Ril的MCC字符串 */
    for (ulLoop = 0; ulLoop < pstCssNty->stMccInfo.ulMccNum; ulLoop++)
    {
        ulMcc = 0;

        AT_ConvertNasMccToBcdType(pstCssNty->stMccInfo.aulMcc[ulLoop], &ulMcc);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%x%x%x",
                                           (ulMcc & 0x0f00)>>8,
                                           (ulMcc & 0xf0)>>4,
                                           (ulMcc & 0x0f));
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (TAF_CHAR *)pgucAtSndCodeAddr,
                                      (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s",
                                      gaucAtCrLf);

    gstAtSendData.usBufLen = usLength;

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvCssLineIndexListSetCnf(
    VOS_VOID                           *pstMsg
)
{
    /* 定义局部变量 */
    CSS_AT_LINE_INDEX_LIST_SET_CNF_STRU                    *pstCssSetCnf    = VOS_NULL_PTR;
    VOS_UINT32                                              ulResult;
    VOS_UINT8                                               ucIndex;

    /* 初始化消息变量 */
    pstCssSetCnf = (CSS_AT_LINE_INDEX_LIST_SET_CNF_STRU *)pstMsg;

    ucIndex      = 0;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstCssSetCnf->usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvCssLineIndexListSetCnf: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvCssLineIndexListSetCnf: WARNING:AT_BROADCAST_INDEX!");

        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_LINEINDEXLIST_SET */
    if (AT_CMD_LINEINDEXLIST_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvCssLineIndexListSetCnf: WARNING:Not AT_CMD_LINEINDEXLIST_SET!");

        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    ulResult = AT_ERROR;

    if (VOS_OK == pstCssSetCnf->ulResult)
    {
        ulResult    = AT_OK;
    }

    gstAtSendData.usBufLen = 0;

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvCssLineIndexListQryCnf(
    VOS_VOID                           *pstMsg
)
{
    CSS_AT_LINE_INDEX_LIST_QUERY_CNF_STRU                  *pstCssQryCnf = VOS_NULL_PTR;
    VOS_UINT32                                              ulMcc;
    VOS_UINT32                                              ulMnc;
    VOS_UINT16                                              usLength;
    VOS_UINT8                                               aucVersionId[AT_CSS_CLOUD_LINE_VERSION_LEN + 1];
    VOS_UINT8                                               ucIndex;

    /* 初始化消息变量 */
    pstCssQryCnf = (CSS_AT_LINE_INDEX_LIST_QUERY_CNF_STRU *)pstMsg;

    ucIndex = 0;
    ulMcc   = 0xFFFFFFFF;
    ulMnc   = 0xFFFFFFFF;

    /* 通过ClientId获取ucIndex */
    if ( AT_FAILURE == At_ClientIdToUserId(pstCssQryCnf->usClientId, &ucIndex) )
    {
        AT_WARN_LOG("AT_RcvCssLineIndexListQryCnf: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvCssLineIndexListQryCnf: WARNING:AT_BROADCAST_INDEX!");

        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_CLOUDBLACKLIST_QRY */
    if ( AT_CMD_LINEINDEXLIST_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt )
    {
        AT_WARN_LOG("AT_RcvCssLineIndexListQryCnf: WARNING:Not AT_CMD_LINEINDEXLIST_QRY!");

        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    usLength = 0;
    /* 构造result  */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: %d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       pstCssQryCnf->ulResult);

    if (VOS_TRUE == pstCssQryCnf->ulResult)
    {
        AT_ConvertNasMccToBcdType(pstCssQryCnf->stRplmnInfo.ulMcc, &ulMcc);
        AT_ConvertNasMncToBcdType(pstCssQryCnf->stRplmnInfo.ulMnc, &ulMnc);

        /* 构造上报给Ril的RPLMN */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",\"%x%x%x",
                                           (ulMcc & 0x0f00)>>8,
                                           (ulMcc & 0xf0)>>4,
                                           (ulMcc & 0x0f));

        if (0x0f00 == (ulMnc & 0x0f00))
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%x%x\",",
                                               (ulMnc & 0xf0)>>4,
                                               (ulMnc & 0x0f));
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%x%x%x\",",
                                               (ulMnc & 0x0f00)>>8,
                                               (ulMnc & 0xf0)>>4,
                                               (ulMnc & 0x0f));
        }


        /* 构造上报给Ril的Version */
        TAF_MEM_SET_S(aucVersionId, sizeof(aucVersionId), 0, sizeof(aucVersionId));
        TAF_MEM_CPY_S(aucVersionId, sizeof(aucVersionId), pstCssQryCnf->aucVersionId, sizeof(pstCssQryCnf->aucVersionId));

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "\"%s\"",
                                           aucVersionId);
    }

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_RcvCssLineDetailSetCnf(
    VOS_VOID                           *pstMsg
)
{
    /* 定义局部变量 */
    CSS_AT_LINE_DETAIL_SET_CNF_STRU                        *pstCssSetCnf    = VOS_NULL_PTR;
    VOS_UINT32                                              ulResult;
    VOS_UINT8                                               ucIndex;

    /* 初始化消息变量 */
    pstCssSetCnf = (CSS_AT_LINE_DETAIL_SET_CNF_STRU *)pstMsg;

    ucIndex      = 0;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstCssSetCnf->usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvCssLineIndexListSetCnf: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvCssLineIndexListSetCnf: WARNING:AT_BROADCAST_INDEX!");

        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_LINEDETAIL_SET */
    if (AT_CMD_LINEDETAIL_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvCssLineIndexListSetCnf: WARNING:Not AT_CMD_LINEDETAIL_SET!");

        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    ulResult = AT_ERROR;

    if (VOS_OK == pstCssSetCnf->ulResult)
    {
        ulResult    = AT_OK;
    }

    gstAtSendData.usBufLen = 0;

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvCssLinePlmnNotify(
    VOS_VOID                           *pstMsg
)
{
    /* 定义局部变量 */
    CSS_AT_LINE_PLMN_NOTIFY_STRU       *pstCssNty = VOS_NULL_PTR;
    VOS_UINT32                          ulMcc     = 0xFFFFFFFF;
    VOS_UINT32                          ulMnc     = 0xFFFFFFFF;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    /* 初始化消息变量 */
    pstCssNty = (CSS_AT_LINE_PLMN_NOTIFY_STRU *)pstMsg;

    ucIndex   = 0;

    /* 通过ClientId获取ucIndex */
    if ( AT_FAILURE == At_ClientIdToUserId(pstCssNty->usClientId, &ucIndex) )
    {
        AT_WARN_LOG("AT_RcvCssLinePlmnNotify: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    AT_ConvertNasMccToBcdType(pstCssNty->stRplmnInfo.ulMcc, &ulMcc);
    AT_ConvertNasMncToBcdType(pstCssNty->stRplmnInfo.ulMnc, &ulMnc);

    usLength = 0;

    /* 构造上报给Ril的RPLMN */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s\"%x%x%x",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_LINE_RPLMN].pucText,
                                       (ulMcc & 0x0f00)>>8,
                                       (ulMcc & 0xf0)>>4,
                                       (ulMcc & 0x0f));

    if (0x0f00 == (ulMnc & 0x0f00))
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%x%x\"",
                                           (ulMnc & 0xf0)>>4,
                                           (ulMnc & 0x0f));
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%x%x%x\"",
                                           (ulMnc & 0x0f00)>>8,
                                           (ulMnc & 0xf0)>>4,
                                           (ulMnc & 0x0f));
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (TAF_CHAR *)pgucAtSndCodeAddr,
                                      (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s",
                                      gaucAtCrLf);

    gstAtSendData.usBufLen = usLength;

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvCssLineIndexNotify(
    VOS_VOID                           *pstMsg
)
{
    /* 定义局部变量 */
    CSS_AT_LINE_INDEX_NOTIFY_STRU      *pstCssNty = VOS_NULL_PTR;
    VOS_UINT32                          ulMcc     = 0xFFFFFFFF;
    VOS_UINT32                          ulMnc     = 0xFFFFFFFF;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLoop;

    /* 初始化消息变量 */
    pstCssNty = (CSS_AT_LINE_INDEX_NOTIFY_STRU *)pstMsg;

    ucIndex   = 0;

    /* 通过ClientId获取ucIndex */
    if ( AT_FAILURE == At_ClientIdToUserId(pstCssNty->usClientId, &ucIndex) )
    {
        AT_WARN_LOG("AT_RcvCssLineIndexNotify: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if ( 0 == pstCssNty->usLineNum)
    {
        AT_WARN_LOG("AT_RcvCssLineIndexNotify: WARNING:line num is 0!");

        return VOS_ERR;
    }

    AT_ConvertNasMccToBcdType(pstCssNty->stPlmnId.ulMcc, &ulMcc);
    AT_ConvertNasMncToBcdType(pstCssNty->stPlmnId.ulMnc, &ulMnc);

    usLength = 0;

    /* 构造上报给Ril的RPLMN */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s\"%x%x%x",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_LINE_INDEX].pucText,
                                       (ulMcc & 0x0f00)>>8,
                                       (ulMcc & 0xf0)>>4,
                                       (ulMcc & 0x0f));

    if (0x0f00 == (ulMnc & 0x0f00))
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%x%x\"",
                                           (ulMnc & 0xf0)>>4,
                                           (ulMnc & 0x0f));
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%x%x%x\"",
                                           (ulMnc & 0x0f00)>>8,
                                           (ulMnc & 0xf0)>>4,
                                           (ulMnc & 0x0f));
    }

    /* CSS带的usLineNum不会大于AT_CSS_PLMN_MAX_LINE_NUM，如果大于，就按AT_CSS_TACLAC_MAX_LINE_NUM个数处理 */
    for (usLoop = 0; ((usLoop < pstCssNty->usLineNum) && (usLoop < AT_CSS_TACLAC_MAX_LINE_NUM)); usLoop++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstCssNty->ausLineIndexList[usLoop]);
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (TAF_CHAR *)pgucAtSndCodeAddr,
                                      (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s",
                                      gaucAtCrLf);

    gstAtSendData.usBufLen = usLength;

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


