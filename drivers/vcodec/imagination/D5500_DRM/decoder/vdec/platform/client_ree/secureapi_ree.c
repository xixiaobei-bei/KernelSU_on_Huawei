
#pragma GCC diagnostic ignored "-Wstrict-prototypes"

#include "secureapi_ree.h"
#include "sysos_api_km.h"

#include "teek_client_api.h"
#include "teek_client_id.h"
#include "teek_client_type.h"
#include "teek_ns_client.h"
#include "teek_client_list.h"
#include "report_api.h"
#include <asm/memory.h>

#define SECURE_REE "SECURE_REE"
#include <ion_client.h>

static struct ion_client * ion_client(void)
{
    static struct ion_client * cl = NULL;
    if(cl == NULL)
        cl = get_ion_client();
    return cl;
}


extern void configure_master_security(unsigned int is_security, int master_id);

//#define DEBUG_TEE_LOCK
typedef struct
{
    TEEC_Context        sTEEContext;
    TEEC_Session        sTEESession;
    TEEC_SharedMemory   sTEESharedMemory;
    IMG_UINT32          ui32SecureId;
    struct ion_handle *  ion_handle;
    ion_phys_addr_t physaddr;
} sTEECommsContext;

static sTEECommsContext g_TEECommsContext = {0};

static IMG_HANDLE hMutex;

#define IMG_MAX_MESSAGE_SIZE (150*1024)
//#define MEDIA_UUID 1013
#define MEDIA_CODEC_UUID 1046
static TEEC_UUID uuidIMG_TA = {
            0x4a1a44e8, 0x3ed7, 0x4e00,
                        { 0x9c, 0x1e, 0x02,0xa2,0x57,0x7e,0xa2,0x4a }
};
//static char* pPackname =  "/system/bin/vdec";
//#define PACKNAME "/system/bin/vdec"
#define PACKNAME "/vendor/bin/hw/android.hardware.media.omx@1.0-service"

static IMG_CHAR * getEndPointName(MSG_ENDPOINT nEndpoint) {    
    switch (nEndpoint)
    {
    case ENDPOINT_VERIFY_FIRMWARE:
        return "ENDPOINT_VERIFY_FIRMWARE";
    case ENDPOINT_GET_PROPS:
        return "ENDPOINT_GET_PROPS";
    case ENDPOINT_POWER_ON:
        return "ENDPOINT_POWER_ON";
    case ENDPOINT_POWER_OFF:
        return "ENDPOINT_POWER_OFF";
     // TODO defualt
    }
    return "UNKNOWN";
}


static IMG_RESULT send_message_with_buffer(
    IMG_UINT32          ui32SecureId,
    IMG_BYTE          * pbyMsg,
    IMG_UINT16          ui16Size,
    MSG_ENDPOINT        nEndpoint,
    IMG_BYTE          * pbyBuf,
    IMG_UINT32          ui32BufSize)
{
    TEEC_Result         result    = 0;
    TEEC_Operation      operation = {0};
    uint32_t            origin    = 0;
    uint8_t            *pu8Buf    = NULL;
    uint8_t            *pu8Msg    = NULL;

    DEBUG_REPORT(SECURE_REE, "%s:%d ui16Size %d ui32BufSize %d\n", __func__, __LINE__, ui16Size, ui32BufSize);

    if (ui16Size + ui32BufSize > g_TEECommsContext.sTEESharedMemory.size)
    {
        REPORT(SECURE_REE, REPORT_ERR, "teec shared buffer too small. buffer %d ui16Size %d ui32BufSize %d\n",
            g_TEECommsContext.sTEESharedMemory.size, ui16Size, ui32BufSize);
        return IMG_ERROR_FATAL;
    }

    memset(g_TEECommsContext.sTEESharedMemory.buffer, 0, g_TEECommsContext.sTEESharedMemory.size);
    pu8Msg = g_TEECommsContext.sTEESharedMemory.buffer;
    if (NULL != pbyMsg)
    {
        IMG_MEMCPY(pu8Msg, pbyMsg, ui16Size);
    }

    if (pbyBuf)
    {
        pu8Buf = &pu8Msg[ui16Size];
        IMG_MEMCPY(pu8Buf, pbyBuf, ui32BufSize);
    }

    //Send Message to the TEE
    //ParamType 1,   TEEC_PARAM_TYPE_VALUE_INPUT,   IMG_RESULT, return code from SECURE_GetId()
    //ParamType 2,   TEEC_PARAM_TYPE_MEMREF_INOUT,  Message buffer (input/output)
    //ParamType 3,   TEEC_PARAM_TYPE_VALUE_OUTPUT,  IMG_RESULT, return code from SECURE_SendMessage()
    //[ParamType 4], Optional parameter must be TEEC_MEMREF_TEMP_OUTPUT/INPUT/INOUT
    memset(&operation, 0, sizeof(TEEC_Operation));
    operation.started    = 1;
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
                                            TEEC_VALUE_INOUT,
                                            TEEC_VALUE_OUTPUT,
                                            TEEC_NONE);
    operation.params[0].value.a = g_TEECommsContext.ui32SecureId;
    operation.params[1].value.a = g_TEECommsContext.physaddr;
    operation.params[1].value.b = ui16Size;
    if (pbyBuf)
    {
        DEBUG_REPORT(SECURE_REE, "%s:%d", __func__, __LINE__);
        //There is also a buffer to pass to the TEE along with the message,
        //it was already copied to a shared area
        operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
                                                TEEC_VALUE_INOUT,
                                                TEEC_VALUE_OUTPUT,
                                                TEEC_VALUE_INOUT);
        operation.params[3].value.a = g_TEECommsContext.physaddr + ui16Size;
        operation.params[3].value.b = ui32BufSize;
    }

    DEBUG_REPORT(SECURE_REE, "%s:%d session_id : %d", __func__, __LINE__, g_TEECommsContext.sTEESession.session_id);
    DEBUG_REPORT(SECURE_REE, "%s:%d nEndpoint : %d, operation.paramTypes : %d", __func__, __LINE__, nEndpoint, operation.paramTypes);
    result = TEEK_InvokeCommand(&(g_TEECommsContext.sTEESession), nEndpoint, &operation, &origin);
    if (result != TEEC_SUCCESS)
    {
        // TODO Something
        REPORT(SECURE_REE, REPORT_ERR, "HISI CA Invoke failed return : %d, origin : %d\n", result, origin);
        return IMG_ERROR_FATAL;
    }

    IMG_MEMCPY(pbyMsg, pu8Msg, ui16Size);
    if (pbyBuf)
    {
        IMG_MEMCPY(pbyBuf, pu8Buf, ui32BufSize);
    }

    return IMG_SUCCESS;
}


static IMG_RESULT call_send_message_with_buffer(
    IMG_UINT32          ui32SecureId,
    IMG_BYTE          * pbyMsg,
    IMG_UINT16          ui16Size,
    MSG_ENDPOINT        nEndpoint,
    IMG_BYTE          * pbyBuf,
    IMG_UINT32          ui32BufSize)
{
    IMG_RESULT rtn = IMG_SUCCESS;

#ifdef DEBUG_TEE_LOCK
    REPORT(SECURE_REE, REPORT_INFO, "%s pid %d nEndpoint %s TEELOCK WAIT", __func__, current->pid, getEndPointName(nEndpoint));
#endif
    SYSOSKM_LockMutex(hMutex);
#ifdef DEBUG_TEE_LOCK
    REPORT(SECURE_REE, REPORT_INFO, "%s pid %d nEndpoint %s TEELOCK ON", __func__, current->pid, getEndPointName(nEndpoint));
#endif

    rtn = send_message_with_buffer(ui32SecureId, pbyMsg, ui16Size, nEndpoint, pbyBuf, ui32BufSize);

#ifdef DEBUG_TEE_LOCK
    REPORT(SECURE_REE, REPORT_INFO, "%s pid %d nEndpoint %s TEELOCK RELEASE", __func__, current->pid, getEndPointName(nEndpoint));
#endif
    SYSOSKM_UnlockMutex(hMutex);
#ifdef DEBUG_TEE_LOCK
    REPORT(SECURE_REE, REPORT_INFO, "%s pid %d nEndpoint %s TEELOCK OFF", __func__, current->pid, getEndPointName(nEndpoint));
#endif

    return rtn;
}

#define PROBE_VALUE 1001
#define CMD_TEST_SESSION_CONNECTED 100
IMG_BOOL probe_session_is_exit()
{
    TEEC_Operation      operation = {0};
    TEEC_Result         result    = 0;
    uint32_t            origin    = 0;

    memset(&operation, 0, sizeof(TEEC_Operation));
    operation.started    = 1;
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT,
                                            TEEC_NONE,
                                            TEEC_NONE,
                                            TEEC_NONE);

    operation.params[0].value.a = PROBE_VALUE;
    result = TEEK_InvokeCommand(&(g_TEECommsContext.sTEESession), CMD_TEST_SESSION_CONNECTED, &operation, &origin);
    if (result != TEEC_SUCCESS)
    {
        REPORT(SECURE_REE, REPORT_ERR, "Probe session failed return : %d, origin : %d\n", result, origin);
        return IMG_FALSE;
    }

    return IMG_TRUE;
}

void close_ta_session()
{
    DEBUG_REPORT(SECURE_REE, "%s:%d", __func__, __LINE__);

    //The TA function to close session calls SECURE_TEE_ReleaseHandle() - it knows the handle
    TEEK_CloseSession(&g_TEECommsContext.sTEESession);

    g_TEECommsContext.ui32SecureId = 0;

    TEEK_FinalizeContext(&g_TEECommsContext.sTEEContext);
}

/*!
******************************************************************************

 @Function              SECURE_REE_GetId

 @Description

 This function establishes communications with a secure driver and returns a handle to allow commands to be sent to the driver

 @Input     nCore               : The core to establish secure communications with

 @Output    pui32SecureId          : This function returns a Secure ID if successful, or null otherwise.

 @Return    This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/

IMG_RESULT SECURE_REE_GetId(IMG_VIDEO_CORE nCore, IMG_UINT32 *pui32SecureId)
{
    IMG_RESULT     rtn       = IMG_SUCCESS;
    TEEC_Result    result    = 0;
    TEEC_Operation operation = {0};
    char packname[] = PACKNAME;
    //uint32_t       uuid = MEDIA_UUID;
    uint32_t       uuid = MEDIA_CODEC_UUID;

    DEBUG_REPORT(SECURE_REE, "%s called ,line = %d ,g_TEECommsContext.ui32SecureId = %x\n", __func__,__LINE__,g_TEECommsContext.ui32SecureId);

    if (g_TEECommsContext.ui32SecureId != 0)
    {
        IMG_BOOL isConnected = probe_session_is_exit();

        if (isConnected)
        {
            DEBUG_REPORT(SECURE_REE, "%s:%d Session have been init", __func__, __LINE__);
            *pui32SecureId = g_TEECommsContext.ui32SecureId;
            return IMG_SUCCESS;
        }
        else
        {
           // SECURE_REE_DeInit();
            close_ta_session();
            DEBUG_REPORT(SECURE_REE, "Session is aborted, we will connect again~");
        }
    }
    //SECURE_REE_Init();

    //InitializeContext
    result = TEEK_InitializeContext(NULL, &(g_TEECommsContext.sTEEContext));
    DEBUG_REPORT(SECURE_REE, "%s called ,line = %d ,result = %x\n", __func__,__LINE__,result);
    if (result != TEEC_SUCCESS)
    {
        REPORT(SECURE_REE, REPORT_ERR, "teec InitializeContext failed");
        return IMG_ERROR_FATAL;
    }
    else
    {
        // TODO success, need print something
        DEBUG_REPORT(SECURE_REE, "%s:%d, Context init success !", __func__, __LINE__);
    }

    memset(&operation, 0, sizeof(TEEC_Operation));
    operation.started    = 1;
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT,
                                            TEEC_NONE,
                                            TEEC_MEMREF_TEMP_INPUT,
                                            TEEC_MEMREF_TEMP_INPUT);

    operation.params[0].value.a = nCore;

    operation.params[2].tmpref.buffer = &uuid;
    operation.params[2].tmpref.size = sizeof(uuid);
    operation.params[3].tmpref.buffer = packname;
    operation.params[3].tmpref.size = strlen(packname) + 1;
    DEBUG_REPORT(SECURE_REE, "%s:%d, paramTypes : 0x%x", __func__, __LINE__, operation.paramTypes);
    result = TEEK_OpenSession(&g_TEECommsContext.sTEEContext,
                              &g_TEECommsContext.sTEESession, &uuidIMG_TA, TEEC_LOGIN_IDENTIFY, NULL, &operation, NULL);
    if (result != TEEC_SUCCESS)
    {
        REPORT(SECURE_REE, REPORT_ERR, "teec open session failed, result = %d\n", result);
        rtn = IMG_ERROR_FATAL;
        goto error_badSession;
    }
    else
    {
        DEBUG_REPORT(SECURE_REE, "%s:%d, session_id : %d", __func__, __LINE__, g_TEECommsContext.sTEESession.session_id);
    }

    g_TEECommsContext.ui32SecureId = operation.params[0].value.a;
    DEBUG_REPORT(SECURE_REE, "%s:%d ui32SecureId : %d", __func__, __LINE__, g_TEECommsContext.ui32SecureId);

    *pui32SecureId = g_TEECommsContext.ui32SecureId;

    return (rtn);

error_badSession:
    // TODO Something
error_badSharedMem:
    TEEK_FinalizeContext(&g_TEECommsContext.sTEEContext);
	//SECURE_REE_DeInit();

    return (rtn);
}

/*!
******************************************************************************

 @Function              SECURE_REE_ReleaseId

 @Description

 Releases a reference to a secure driver communications channel

******************************************************************************/

IMG_RESULT SECURE_REE_ReleaseId(IMG_UINT32 ui32SecureId)
{
    DEBUG_REPORT(SECURE_REE, "%s:%d", __func__, __LINE__);
    return (IMG_SUCCESS);
}

/*!
******************************************************************************

 @Function              SECURE_REE_ReleaseMsgBuffer

 @Description

 Provides shared memoryu

 @Input     ui32SecureId     : Secure ID

 @Input     pMsgBuff         : Buffer to release.

 @Return    This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/

IMG_RESULT SECURE_REE_ReleaseMsgBuffer(IMG_UINT32 ui32SecureId, IMG_VOID  *pMsgBuff)
{
#ifdef DEBUG_TEE_LOCK
    REPORT(SECURE_REE, REPORT_INFO, "%s pid %d TEELOCK RELEASE", __func__, current->pid);
#endif
    SYSOSKM_UnlockMutex(hMutex);
#ifdef DEBUG_TEE_LOCK
    REPORT(SECURE_REE, REPORT_INFO, "%s pid %d TEELOCK OFF", __func__, current->pid);
#endif
    return (IMG_SUCCESS);
}


/*!
******************************************************************************

 @Function              SECURE_REE_SendMessage

 @Description

******************************************************************************/

IMG_RESULT SECURE_REE_SendMessage(
    IMG_UINT32      ui32SecureId,
    IMG_BYTE*       pbyMsg,
    IMG_UINT16      ui16Size,
    MSG_ENDPOINT    nEndpoint)
{
    DEBUG_REPORT(SECURE_REE, "%s:%d ui32SecureId : %d, ui16Size : %d, nEndpoint [%d] : [%s]", __func__, __LINE__,
                 ui32SecureId, ui16Size, nEndpoint, getEndPointName(nEndpoint));
    return call_send_message_with_buffer(ui32SecureId, pbyMsg, ui16Size, nEndpoint, NULL, 0);
}


/*!
******************************************************************************

 @Function              SECURE_REE_SendMessageWithBuf

 @Description

******************************************************************************/

IMG_RESULT SECURE_REE_SendMessageWithBuf(
    IMG_UINT32          ui32SecureId,
    IMG_BYTE          * pbyMsg,
    IMG_UINT16          ui16Size,
    MSG_ENDPOINT        nEndpoint,
    IMG_BYTE          * pbyBuf,
    IMG_UINT32          ui32BufSize)
{
    DEBUG_REPORT(SECURE_REE,
                 "%s:%d ui32SecureId : %d, ui16Size : %d, nEndpoint [%d] : [%s], ui32BufSize[%d]", __func__, __LINE__,
                 ui32SecureId, ui16Size,  nEndpoint, getEndPointName(nEndpoint), ui32BufSize);
    return call_send_message_with_buffer(ui32SecureId, pbyMsg, ui16Size, nEndpoint, pbyBuf, ui32BufSize);
}

IMG_VOID SECURE_REE_Init(SYSDEVU_sInfo *psInfo)
{
    size_t         len = 0;
    IMG_UINT32     allocFlags = 0;
    IMG_INT32      ion_heap = 14;
    TEEC_Result    result    = 0;

    SYSOSKM_CreateMutex(&hMutex);

    g_TEECommsContext.sTEESharedMemory.size   = IMG_MAX_MESSAGE_SIZE;
    g_TEECommsContext.sTEESharedMemory.flags  = TEEC_MEM_INOUT;
    g_TEECommsContext.ion_handle = ion_alloc(ion_client(), g_TEECommsContext.sTEESharedMemory.size, PAGE_SIZE,
                           1 << ion_heap,
                           allocFlags);
    if (IS_ERR(g_TEECommsContext.ion_handle)) {
        IMG_ASSERT(!"ion_alloc failed!!!!");
        REPORT(SECURE_REE, REPORT_ERR, "%s alloc failed line = %d\n", __FUNCTION__, __LINE__);
        return;
    }

    result = img_vdec_get_ion_phys(ion_client(), g_TEECommsContext.ion_handle, &(g_TEECommsContext.physaddr), &len);
    if(result)
    {
        IMG_ASSERT(!"img_vdec_get_ion_phys failed");
        ion_free(ion_client(), g_TEECommsContext.ion_handle);
        return;
    }

    g_TEECommsContext.sTEESharedMemory.buffer = ion_map_kernel(ion_client(), g_TEECommsContext.ion_handle);
    if (NULL == g_TEECommsContext.sTEESharedMemory.buffer)
    {
        REPORT(SECURE_REE, REPORT_ERR, "sTEESharedMemory.buffer is null point\n");
        ion_free(ion_client(), g_TEECommsContext.ion_handle);
        return;
    }

    DEBUG_REPORT(SECURE_REE, "%s msg data %d bytes", __func__, g_TEECommsContext.sTEESharedMemory.size);

    g_TEECommsContext.ui32SecureId = 0;
}

IMG_VOID SECURE_REE_DeInit(IMG_VOID)
{
    if (g_TEECommsContext.ion_handle)
    {
        ion_unmap_kernel(ion_client(), g_TEECommsContext.ion_handle);
        g_TEECommsContext.sTEESharedMemory.buffer = NULL;
        ion_free(ion_client(), g_TEECommsContext.ion_handle);
        g_TEECommsContext.ion_handle = NULL;
    }
    g_TEECommsContext.sTEESharedMemory.size   = 0;
    SYSOSKM_DestroyMutex(hMutex); // TODO need to be protected
}

IMG_VOID SECURE_REE_StreamCreate(IMG_VOID)
{
    DEBUG_REPORT(SECURE_REE, "%s:%d", __func__, __LINE__);
    configure_master_security(1, 0);
}

IMG_VOID SECURE_REE_StreamDestroy(IMG_VOID)
{
    DEBUG_REPORT(SECURE_REE, "%s:%d", __func__, __LINE__);
    configure_master_security(0, 0);
}

