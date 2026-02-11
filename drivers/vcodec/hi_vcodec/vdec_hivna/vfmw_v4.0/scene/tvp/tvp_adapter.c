
#include "public.h"
#include "tvp_adapter.h"

#include <teek_client_api.h>

#include "vfmw.h"
#include "vfmw_dts.h"
#include "linux_kernel_osal.h"

static UINT32 g_sec_pint_enable = 0x3;
static SINT32 g_sec_init;

#define HI_VCODEC_INVOKE_CODE_A    0x6728661c
#define HI_VCODEC_INVOKE_CODE_B    0x5b9c660c
#define set_invoke_code(value) \
	{value.a = HI_VCODEC_INVOKE_CODE_A; \
	 value.b = HI_VCODEC_INVOKE_CODE_B; }

#ifdef HI_ADVCA_FUNCTION_RELEASE
#define sec_print(type, fmt, arg...) \
	do \
	while (0)
#else
#define sec_print(type, fmt, arg...) \
	do { \
		if (type == PRN_ALWS || \
			(g_sec_pint_enable & (1 << type)) != 0) { \
			dprint(PRN_ALWS, fmt, ##arg); \
		} \
	} while (0)
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
} tee_hivcodec_cmd_id;

static SINT8 g_packet_id;

static TEEC_Context g_tee_context;
static TEEC_Session g_tee_session;

SINT32 tvp_vdec_suspend(void)
{
	SINT32 ret = VDEC_OK;
	TEEC_Result result;
	TEEC_Operation operation;

	ret = memset_s(&operation, sizeof(TEEC_Operation), 0, sizeof(TEEC_Operation));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return VDEC_ERR;
	}

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT,
		TEEC_NONE, TEEC_NONE, TEEC_VALUE_INPUT);
	operation.started = 1;
	operation.params[0].value.a = 0;
	operation.params[0].value.b = TEEC_VALUE_UNDEF;
	set_invoke_code(operation.params[3].value);

	result = TEEK_InvokeCommand(&g_tee_session,
		HIVCODEC_CMD_ID_SUSPEND, &operation, NULL);
	if (result != TEEC_SUCCESS) {
		sec_print(PRN_FATAL, "InvokeCommand HIVCODEC_CMD_ID_SUSPEND Failed\n");
		ret = VDEC_ERR;
	} else {
		ret = operation.params[0].value.a;
	}

	return ret;
}

SINT32 tvp_vdec_resume(void)
{
	SINT32 ret = VDEC_OK;
	TEEC_Result result;
	TEEC_Operation operation;

	ret = memset_s(&operation, sizeof(TEEC_Operation), 0, sizeof(TEEC_Operation));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return VDEC_ERR;
	}

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT,
		TEEC_NONE, TEEC_NONE, TEEC_VALUE_INPUT);
	operation.started = 1;
	operation.params[0].value.a = 0;
	operation.params[0].value.b = TEEC_VALUE_UNDEF;
	set_invoke_code(operation.params[3].value);

	result = TEEK_InvokeCommand(&g_tee_session,
		HIVCODEC_CMD_ID_RESUME, &operation, NULL);
	if (result != TEEC_SUCCESS) {
		sec_print(PRN_FATAL, "InvokeCommand HIVCODEC_CMD_ID_RESUME Failed\n");
		ret = VDEC_ERR;
	} else {
		ret = operation.params[0].value.a;
	}

	return ret;
}

SINT32 tvp_vdec_secure_init(void)
{
	TEEC_Result result;
	TEEC_Operation operation;
	hi_s32 ret;

	TEEC_UUID svc_id = { 0x528822b7, 0xfc78, 0x466b,
		{0xb5, 0x7e, 0x62, 0x09, 0x3d, 0x60, 0x34, 0xa7}
	};

	const UINT8 *package_name_0 =
		"/vendor/bin/hw/android.hardware.media.omx@1.0-service";
	const UINT8 *package_name_1 = "sample_omxvdec";
	UINT8 package_name[TVP_PACKAGE_NAME_MAX_LENGTH] = { 0 };
	UINT32 uid = 0;

	if (g_sec_init != 0) {
		sec_print(PRN_FATAL, "%s, tvp_vdec_secure_init init already\n",
			__func__);
		return VDEC_OK;
	}

	BUG_ON(strlen(package_name_0) >= sizeof(package_name));
	BUG_ON(strlen(package_name_1) >= sizeof(package_name));

	if (g_packet_id == 0) {
		ret = strncpy_s(package_name, TVP_PACKAGE_NAME_MAX_LENGTH, package_name_0,
			(sizeof(package_name) - 1));
		if (ret != EOK) {
			dprint(PRN_FATAL, " %s %d strncpy_s err in function\n", __func__, __LINE__);
			return VDEC_ERR;
		}
	} else {
		ret = strncpy_s(package_name, TVP_PACKAGE_NAME_MAX_LENGTH, package_name_1,
			(sizeof(package_name) - 1));
		if (ret != EOK) {
			dprint(PRN_FATAL, " %s %d strncpy_s err in function\n", __func__, __LINE__);
			return VDEC_ERR;
		}
	}

	result = TEEK_InitializeContext(NULL, &g_tee_context);
	if (result != TEEC_SUCCESS) {
		sec_print(PRN_FATAL, "TEEC_InitializeContext Failed\n");
		return VDEC_ERR;
	}

	ret = memset_s(&operation, sizeof(TEEC_Operation), 0,
		sizeof(TEEC_Operation));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return VDEC_ERR;
	}
	operation.started = 1;
	operation.paramTypes =
		TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
		TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);
	operation.params[TEEC_OPERATION_PARA_INDEX_SECOND].tmpref.buffer = (VOID *)(&uid);
	operation.params[TEEC_OPERATION_PARA_INDEX_SECOND].tmpref.size = sizeof(uid);
	operation.params[TEEC_OPERATION_PARA_INDEX_THIRD].tmpref.buffer = (VOID *)(package_name);
	operation.params[TEEC_OPERATION_PARA_INDEX_THIRD].tmpref.size = strlen(package_name) + 1;

	result = TEEK_OpenSession(&g_tee_context,
		&g_tee_session, &svc_id, TEEC_LOGIN_IDENTIFY,
		NULL, &operation, NULL);
	if (result != TEEC_SUCCESS) {
		sec_print(PRN_FATAL, "TEEK_OpenSession Failed\n");
		goto InitWithfree_0;
	}

	g_sec_init = 1;
	return VDEC_OK;

InitWithfree_0:
	TEEK_FinalizeContext(&g_tee_context);
	return VDEC_ERR;
}

SINT32 tvp_vdec_secure_exit(VOID)
{
	SINT32 ret = VDEC_OK;

	sec_print(PRN_ERROR, "%s Enter\n", __func__);
	if (!g_sec_init) {
		sec_print(PRN_FATAL, "FATAL %s, init first\n", __func__);
		return VDEC_ERR;
	}

	TEEK_CloseSession(&g_tee_session);
	TEEK_FinalizeContext(&g_tee_context);

	sec_print(PRN_FATAL, "%s Success\n", __func__);
	g_sec_init = 0;

	return ret;
}
