/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *  Description: security_info ca, call secbootTA verify bypass connectnet cert.
 *  Create : 2021/07/06
 */
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hisi/partition_ap_kernel.h>
#include <linux/hisi/security_info.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/completion.h>
#include <teek_client_api.h>
#include <teek_client_id.h>
#include <linux/crypto.h>
#include <crypto/hash.h>
#include <crypto/sha.h>
#include <linux/mutex.h>

#define  TLV_TAG_BYTES     1
#define  TLV_LENGTH_BYTES  2
#define  CERT_HEADER_TAG   0xfd
#define  CERT_HASH_TAG     0x04
#define  BYPASS_QUEUE_NAME "bypass_net_cert_sig"

enum {
	BY_TAG_EXCEED_RANGE_ERR = 0xff0ee001,
	BY_LENGTH_EXCEED_RANGE_ERR = 0xff0ee002,
	BY_VALUE_EXCEED_RANGE_ERR = 0xff0ee003,
	BY_PARSER_CERT_HEADER_ERR = 0xff0ee004,
	BY_PARSER_CERT_HASH_ERR = 0xff0ee005,
	BY_CERT_BYTES_ERR = 0xff0ee006,
	BY_CRYPTO_ALLOC_SHASH_ERR = 0xff0ee007,
	BY_CRYPTO_DIGEST_LEN_ERR = 0xff0ee008,
	BY_CRYPTO_COMPUTE_SHA256_ERR = 0xff0ee009,
	BY_TLV_DIGEST_LEN_ERR = 0xff0ee00a,
	BY_COMPARE_TLV_DIGEST_ERR = 0xff0ee00b,
	BY_TEEK_INIT_ERR = 0xff0ee00c,
	BY_TEE_VERIFY_CERT_ERR = 0xff0ee00d,
	BY_ARGP_IS_NULL_ERR = 0xff0ee00e,
	BY_CMD_NOT_EQUAL_ERR = 0xff0ee00f,
	BY_KZALLOC_CERT_FAIL_ERR = 0xff0ee010,
	BY_COPY_FROM_USER_ERR = 0xff0ee011,
	BY_CREATE_WORK_QUEUE_ERR = 0xff0ee012,
};

struct asn1_buf {
	uint8_t *p;
	uint32_t len;
};

struct asn1_tlv {
	uint32_t tag;
	struct asn1_buf val;
	struct asn1_buf raw;
};

struct cert_tlv {
	struct asn1_tlv cert;
	struct asn1_tlv hash;
};

struct bypass_work_ctx {
	s32 result;
	struct completion completion;
	struct work_struct work;
	struct bypass_net_cert *cert_info;
	struct workqueue_struct *queue;
};

DEFINE_MUTEX(g_security_info_lock);

static s32 decode_tlv(struct asn1_tlv *tlv, u8 **p, u8 *end)
{
	if (*p + TLV_TAG_BYTES > end) {
		pr_err("tag exceed cert range\n");
		return BY_TAG_EXCEED_RANGE_ERR;
	}

	tlv->raw.p = *p;
	tlv->raw.len = 0;

	tlv->tag = **p;
	*p += TLV_TAG_BYTES;

	tlv->raw.len += TLV_TAG_BYTES;

	if (*p + TLV_LENGTH_BYTES > end) {
		pr_err("length exceed cert range\n");
		return BY_LENGTH_EXCEED_RANGE_ERR;
	}

	tlv->val.len = **p | (*(*p + 1) << 8);
	*p += TLV_LENGTH_BYTES;

	tlv->raw.len += TLV_LENGTH_BYTES;

	if (*p + tlv->val.len < *p || *p + tlv->val.len > end) {
		pr_err("val exceed cert range\n");
		return BY_VALUE_EXCEED_RANGE_ERR;
	}

	tlv->val.p = *p;
	*p += tlv->val.len;

	tlv->raw.len += tlv->val.len;

	return 0;
}

/*
 *  bypass_net_cert.cert format as below
 *
 *        +-------------+----------------+-------------------------+
 * cert   | type(1 Byte)| length(2 Bytes)|    value                |
 *        +-------------+----------------+-------------------------+
 *
 *        +-------------+----------------+-------------------------+
 * hash   | type(1 Byte)| length(2 Bytes)|    value                |
 *        +-------------+----------------+-------------------------+
 */
static s32 parser_bypass_net_cert(
	struct cert_tlv *cert_tlv, struct bypass_net_cert *cert_info)
{
	s32 ret;
	u8 *p = cert_info->cert;
	u8 *end = p + sizeof(cert_info->cert);

	ret = decode_tlv(&cert_tlv->cert, &p, end);
	if (ret != 0 || cert_tlv->cert.tag != CERT_HEADER_TAG) {
		pr_err("decode cert header error\n");
		return BY_PARSER_CERT_HEADER_ERR;
	}

	ret = decode_tlv(&cert_tlv->hash, &p, end);
	if (ret != 0 || cert_tlv->hash.tag != CERT_HASH_TAG) {
		pr_err("decode cert hash error\n");
		return BY_PARSER_CERT_HASH_ERR;
	}

	/* cert_info->cert_bytes include cert and hash */
	if (cert_info->cert_bytes != p - cert_info->cert) {
		pr_err("error,cert_bytes = %u\n", cert_info->cert_bytes);
		return BY_CERT_BYTES_ERR;
	}

	/* we should exclude the hash from cert_info->cert_bytes */
	cert_info->cert_bytes = cert_tlv->cert.raw.len;

	return 0;
}

static s32 do_compute_sha256(
	struct crypto_shash *tfm, u8 *msg, u32 msg_len, u8 *digst)
{
	SHASH_DESC_ON_STACK(shash, tfm);

	shash->tfm = tfm;
	shash->flags = 0x0;

	return crypto_shash_digest(shash, msg, msg_len, digst);
}

s32 security_info_compute_sha256(u8 *msg, u32 msg_len, u8 *digst, u32 digst_len)
{
	s32 ret;
	struct crypto_shash *tfm = NULL;

	if (!msg || !digst) {
		pr_err("error compute_sha256 input parameters\n");
		return BY_CRYPTO_ALLOC_SHASH_ERR;
	}

	tfm = crypto_alloc_shash("sha256", 0, 0);
	if (IS_ERR_OR_NULL(tfm)) {
		pr_err("error, sha256 tfm failed\n");
		return BY_CRYPTO_ALLOC_SHASH_ERR;
	}

	if (digst_len != crypto_shash_digestsize(tfm)) {
		pr_err("error digst_len %u\n", digst_len);
		ret = BY_CRYPTO_DIGEST_LEN_ERR;
		goto exit_free;
	}

	ret = do_compute_sha256(tfm, msg, msg_len, digst);
	if (ret != 0) {
		pr_err("error compute digst 0x%x\n", ret);
		ret = BY_CRYPTO_COMPUTE_SHA256_ERR;
		goto exit_free;
	}

exit_free:
	crypto_free_shash(tfm);

	return ret;
}

static s32 verify_bypass_net_cert_hash(struct bypass_net_cert *cert)
{
	s32 ret;
	u8 *msg = NULL;
	u32 msg_len;
	u8 digst[SHA256_BYTES] = {0};
	struct cert_tlv cert_tlv = { {0} };
	struct asn1_buf *hash_buf = NULL;

	ret = parser_bypass_net_cert(&cert_tlv, cert);
	if (ret != 0) {
		pr_err("error, parser cert\n");
		return ret;
	}

	hash_buf = &(cert_tlv.hash.val);
	if (hash_buf->len != sizeof(digst)) {
		pr_err("error, tlv hash len %u\n", hash_buf->len);
		return BY_TLV_DIGEST_LEN_ERR;
	}

	msg = cert_tlv.cert.raw.p;
	msg_len = cert_tlv.cert.raw.len;
	ret = security_info_compute_sha256(msg, msg_len, digst, sizeof(digst));
	if (ret != 0)
		return ret;

	if (memcmp(digst, hash_buf->p, sizeof(digst)) != 0) {
		pr_err("error, compare sha256\n");
		return BY_COMPARE_TLV_DIGEST_ERR;
	}

	security_info_err("ret=0x%x!\n", ret);

	return 0;
}

static void bypass_verify_sig_work(struct work_struct *work)
{
	s32 ret;
	u32 origin = 0;
	TEEC_Operation operation = {0};
	TEEC_Result result;
	TEEC_Session session;
	TEEC_Context context;
	struct bypass_work_ctx *ctx = NULL;

	ctx = container_of(work, struct bypass_work_ctx, work);

	ret = teek_init(&session, &context);
	if (ret != 0) {
		pr_err("error, teek_init 0x%x!\n", ret);
		ret = BY_TEEK_INIT_ERR;
		goto exit_err;
	}

	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
			TEEC_MEMREF_TEMP_INPUT,
			TEEC_NONE,
			TEEC_NONE,
			TEEC_NONE);
	operation.params[0].tmpref.buffer = ctx->cert_info;
	operation.params[0].tmpref.size = sizeof(*(ctx->cert_info));

	result = TEEK_InvokeCommand(
		&session,
		SECBOOT_CMD_VERIFY_BYPASS_NET_CERT,
		&operation,
		&origin);
	if (result != TEEC_SUCCESS) {
		pr_err("error, TEEK_InvokeCommand 0x%x!\n", result);
		ret = BY_TEE_VERIFY_CERT_ERR;
		goto exit_close;
	}
	ret = 0;
exit_close:
	TEEK_CloseSession(&session);
	TEEK_FinalizeContext(&context);

	security_info_err("ret=0x%x!\n", ret);

exit_err:
	ctx->result = ret;
	complete(&ctx->completion);
}

static void security_info_bypass_net_cert_deinit(struct bypass_work_ctx *ctx)
{
	kfree(ctx->cert_info);
	destroy_workqueue(ctx->queue);
}

static s32 security_info_bypass_net_cert_init(struct bypass_work_ctx *ctx)
{
	struct workqueue_struct *queue = NULL;
	struct bypass_net_cert *cert_info = NULL;

	cert_info = kzalloc(sizeof(*cert_info), GFP_KERNEL);
	if (cert_info == NULL) {
		pr_err("kzalloc cert failed\n");
		return BY_KZALLOC_CERT_FAIL_ERR;
	}

	queue = create_singlethread_workqueue(BYPASS_QUEUE_NAME);
	if (IS_ERR_OR_NULL(queue)) {
		pr_err("%s, create_workqueue error\n", __func__);
		kfree(cert_info);
		return BY_CREATE_WORK_QUEUE_ERR;
	}
	ctx->queue = queue;
	ctx->cert_info = cert_info;
	INIT_WORK(&ctx->work, bypass_verify_sig_work);
	init_completion(&ctx->completion);

	return 0;
}

s32 security_info_verify_bypass_net_cert(u32 cmd, uintptr_t arg)
{
	s32 ret;
	void __user *argp = (void __user *)arg;
	struct bypass_work_ctx ctx = {0};

	security_info_err("enter func");

	if (argp == NULL) {
		pr_err("argp is NULL\n");
		return BY_ARGP_IS_NULL_ERR;
	}

	if (cmd != SECURUTY_INFO_VERIFY_BYPASS_NET_CERT) {
		pr_err("cmd 0x%08x error\n", cmd);
		return BY_CMD_NOT_EQUAL_ERR;
	}

	mutex_lock(&g_security_info_lock);

	ret = security_info_bypass_net_cert_init(&ctx);
	if (ret != 0) {
		pr_err("error, init bypass cert work %d!\n", ret);
		mutex_unlock(&g_security_info_lock);
		return ret;
	}

	ret = copy_from_user(ctx.cert_info, argp, sizeof(*(ctx.cert_info)));
	if (ret != 0 || ctx.cert_info == NULL) {
		pr_err("copy cert from %d\n", ret);
		ret = BY_COPY_FROM_USER_ERR;
		goto exit_err;
	}

	ret = verify_bypass_net_cert_hash(ctx.cert_info);
	if (ret != 0) {
		pr_err("verify cert hash error 0x%x\n", ret);
		goto exit_err;
	}

	queue_work(ctx.queue, &ctx.work);

	wait_for_completion(&ctx.completion);

	ret = ctx.result;

exit_err:
	security_info_bypass_net_cert_deinit(&ctx);
	security_info_err("exit, ret=0x%x\n", ret);
	mutex_unlock(&g_security_info_lock);

	return ret;
}


