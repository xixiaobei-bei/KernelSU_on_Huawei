#include "public.h"
#include "vfmw.h"
#include "vfmw_dts.h"
#include "linux_kernel_osal.h"
#include "tvp_adapter.h"
#include <teek_client_api.h>

static UINT32 g_SecPrintEnable = 0x3;
static SINT32 g_SecInit = 0;

#define HI_VCODEC_INVOKE_CODE_A   (0x6728661c)
#define HI_VCODEC_INVOKE_CODE_B   (0x5b9c660c)
#define SET_INVOKE_CODE(value) {value.a = HI_VCODEC_INVOKE_CODE_A; value.b = HI_VCODEC_INVOKE_CODE_B;}

#ifdef HI_ADVCA_FUNCTION_RELEASE
#define SecPrint(type, fmt, arg...)                   \
    do{                                               \
    }while(0)
#else
#define SecPrint(type, fmt, arg...)                   \
    do{                                               \
        if (PRN_ALWS == type                          \
         || 0 != (g_SecPrintEnable & (1 << type)))    \
        {                                             \
            dprint(PRN_ALWS, fmt, ##arg);             \
        }                                             \
    }while(0)
#endif

typedef enum {
	HIVCODEC_CMD_ID_INVALID = 0x0,
	HIVCODEC_CMD_ID_INIT,
	HIVCODEC_CMD_ID_EXIT,
	HIVCODEC_CMD_ID_SUSPEND,
	HIVCODEC_CMD_ID_RESUME,
	HIVCODEC_CMD_ID_CONTROL,
	HIVCODEC_CMD_ID_RUN_PROCESS,
	HIVCODEC_CMD_ID_GET_IMAGE,
	HIVCODEC_CMD_ID_RELEASE_IMAGE,
	HIVCODEC_CMD_ID_READ_PROC,
	HIVCODEC_CMD_ID_WRITE_PROC,
} TEE_HIVCODEC_CMD_ID;

static SINT8 g_PacketID = 0;

static TEEC_Context g_TeeContext;
static TEEC_Session g_TeeSession;

SINT32 TVP_VDEC_Suspend(VOID)
{
	SINT32 ret = VDEC_OK;
	TEEC_Result result = TEEC_SUCCESS;
	TEEC_Operation operation;
	memset(&operation, 0, sizeof(TEEC_Operation));

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_VALUE_INPUT);
	operation.started = 1;
	operation.params[0].value.a = 0;
	operation.params[0].value.b = TEEC_VALUE_UNDEF;
	SET_INVOKE_CODE(operation.params[3].value);

	result = TEEK_InvokeCommand(&g_TeeSession, HIVCODEC_CMD_ID_SUSPEND, &operation, NULL);
	if (result != TEEC_SUCCESS) {
		SecPrint(PRN_FATAL, "InvokeCommand HIVCODEC_CMD_ID_SUSPEND Failed\n");
		ret = VDEC_ERR;
	} else {
		ret = operation.params[0].value.a;
	}

	return ret;
}

SINT32 TVP_VDEC_Resume(VOID)
{
	SINT32 ret = VDEC_OK;
	TEEC_Result result = TEEC_SUCCESS;
	TEEC_Operation operation;
	memset(&operation, 0, sizeof(TEEC_Operation));

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_VALUE_INPUT);
	operation.started = 1;
	operation.params[0].value.a = 0;
	operation.params[0].value.b = TEEC_VALUE_UNDEF;
	SET_INVOKE_CODE(operation.params[3].value);

	result = TEEK_InvokeCommand(&g_TeeSession, HIVCODEC_CMD_ID_RESUME, &operation, NULL);
	if (result != TEEC_SUCCESS) {
		SecPrint(PRN_FATAL, "InvokeCommand HIVCODEC_CMD_ID_RESUME Failed\n");
		ret = VDEC_ERR;
	} else {
		ret = operation.params[0].value.a;
	}

	return ret;
}

SINT32 TVP_VDEC_SecureInit(VOID)
{
	TEEC_Result result = TEEC_SUCCESS;
	TEEC_Operation operation;

	TEEC_UUID svc_id = { 0x528822b7, 0xfc78, 0x466b,
		{0xb5, 0x7e, 0x62, 0x09, 0x3d, 0x60, 0x34, 0xa7}
	};

	const UINT8 *package_name_0 = "/vendor/bin/hw/android.hardware.media.omx@1.0-service";
	const UINT8 *package_name_1 = "sample_omxvdec";
	UINT8 package_name[70] = { 0 };
	UINT32 uid = 0;

	if (g_SecInit != 0) {
		SecPrint(PRN_FATAL, "%s, TVP_VDEC_SecureInit init already\n", __func__);
		return VDEC_OK;
	}

	BUG_ON(strlen(package_name_0) >= sizeof(package_name));
	BUG_ON(strlen(package_name_1) >= sizeof(package_name));

	if (g_PacketID == 0) {
		strncpy(package_name, package_name_0, (sizeof(package_name) - 1));
	} else {
		strncpy(package_name, package_name_1, (sizeof(package_name) - 1));
	}

	result = TEEK_InitializeContext(NULL, &g_TeeContext);
	if (result != TEEC_SUCCESS) {
		SecPrint(PRN_FATAL, "TEEC_InitializeContext Failed\n");
		return VDEC_ERR;
	}

	memset(&operation, 0, sizeof(TEEC_Operation));
	operation.started = 1;
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);
	operation.params[2].tmpref.buffer = (VOID *) (&uid);
	operation.params[2].tmpref.size = sizeof(uid);
	operation.params[3].tmpref.buffer = (VOID *) (package_name);
	operation.params[3].tmpref.size = strlen(package_name) + 1;

	result = TEEK_OpenSession(&g_TeeContext, &g_TeeSession, &svc_id, TEEC_LOGIN_IDENTIFY, NULL, &operation, NULL);
	if (result != TEEC_SUCCESS) {
		SecPrint(PRN_FATAL, "TEEK_OpenSession Failed\n");
		goto InitWithfree_0;
	}

	g_SecInit = 1;
	return VDEC_OK;

InitWithfree_0:
	TEEK_FinalizeContext(&g_TeeContext);
	return VDEC_ERR;
}

SINT32 TVP_VDEC_SecureExit(VOID)
{
	SINT32 ret = VDEC_OK;

	SecPrint(PRN_ERROR, "%s Enter\n", __func__);
	if (!g_SecInit) {
		SecPrint(PRN_FATAL, "FATAL %s, init first\n", __func__);
		return VDEC_ERR;
	}

	TEEK_CloseSession(&g_TeeSession);
	TEEK_FinalizeContext(&g_TeeContext);

	SecPrint(PRN_FATAL, "%s Success\n", __func__);
	g_SecInit = 0;

	return ret;
}
