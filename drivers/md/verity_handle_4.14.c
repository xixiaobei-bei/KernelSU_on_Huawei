/*
 * verity_handle_4.14.c
 *
 * Huawei verity fec handler for 4.14 kernel
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "dm-verity.h"
#include "dm-verity-fec.h"
#include "verity_handle_4.14.h"
#if defined(CONFIG_DM_VERITY_HW_RETRY)
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/mtd/hisi_nve_number.h>
#endif
#include <linux/delay.h>

#define DM_MSG_PREFIX "oem_verity"
#define PARAMETER_ERR (-1)

struct verity_fec_members {
	struct dm_verity *v;
	struct dm_verity_io *io;
	struct bvec_iter *iter;
	u8 *want_digest;
	enum verity_block_type type;
};

#if defined(CONFIG_HUAWEI_DSM)
#include <linux/jiffies.h>
#include <dsm/dsm_pub.h>
#include <linux/ctype.h>

#define BUFF_MAX 1024
static struct dsm_dev g_dm_dsm_dev = {
	.name        = "dsm_dm_verity",
	.device_name = NULL,
	.ic_name     = NULL,
	.module_name = NULL,
	.fops        = NULL,
	.buff_size   = BUFF_MAX,
};

static unsigned long g_timeout;
static struct dsm_client *g_dm_dsm_dclient;
static unsigned long g_err_count;
#define DSM_REPORT_INTERVAL (1)
#define DM_VERITY_MAX_PRINT_ERRS 20
#define HASH_ERR_VALUE 1
#define ROW_DATA_LENGTH 16
#define ROW_DATA_PER_HEX_LENGTH 3

enum info_type {
	CE_INFO = 0,
	FEC_INFO,
	REREAD_INFO,
	REREAD_ERR,
};

/* init dsm device  */
void verity_dsm_init(void)
{
	if (!g_dm_dsm_dclient) {
		g_dm_dsm_dclient = dsm_register_client(&g_dm_dsm_dev);
		if (!g_dm_dsm_dclient)
			pr_err("[%s]dsm_register_client register fail\n",
				__func__);
	}

	g_timeout = jiffies;
}

/* upload data info */
static void verity_dsm(struct dm_verity *v, enum verity_block_type type,
	unsigned long long block, int error_no, enum info_type sub_err)
{
	const char *type_str = NULL;
	const char *last_ops = NULL;
	const char *current_ops = NULL;
	char devname[BDEVNAME_SIZE + 1];

	memset(devname, 0x00, BDEVNAME_SIZE + 1);
	bdevname(v->data_dev->bdev, devname);

	if (type == DM_VERITY_BLOCK_TYPE_DATA)
		type_str = "data";
	else if (type == DM_VERITY_BLOCK_TYPE_METADATA)
		type_str = "metadata";

	if (time_after(jiffies, g_timeout)) {
		if (!dsm_client_ocuppy(g_dm_dsm_dclient)) {
			dsm_client_record(g_dm_dsm_dclient,
				"%s: %s block %llu is corrupted, dmd error num %d sub %d;version:1.0\n",
				devname, type_str, block,
				error_no, (int)sub_err);
			dsm_client_notify(g_dm_dsm_dclient, error_no);
		}

		g_timeout = jiffies + DSM_REPORT_INTERVAL * HZ;
	}

	if (sub_err == CE_INFO) {
		last_ops    = "CE hash fail,";
		current_ops = "soft hash ok,";
	} else if (sub_err == FEC_INFO) {
		last_ops    = "hash verify fail,";
		current_ops = "fec correct success,";
	} else if (sub_err == REREAD_INFO) {
		last_ops    = "hash verify fail before reread,";
		current_ops = "hash verify ok after reread,";
	} else if (sub_err == REREAD_ERR) {
		last_ops    = "hash verify fail before reread,";
		current_ops = "hash verify fail after reread,";
	}

	pr_err("[check image]:%s %s %s block= %llu,block name = %s\n",
		last_ops, current_ops, type_str, block, devname);
}

#if defined(CONFIG_DM_VERITY_HW_RETRY)
extern int get_dsm_notify_flag(void);

#define DM_MAX_ERR_COUNT 4
static int verity_read_write_nv(char *name,
	int nv_number, int opcode, char value)
{
	struct hisi_nve_info_user nve;
	int ret;

	if (!name || (opcode != NV_READ && opcode != NV_WRITE) ||
		strlen(name) >= NV_NAME_LENGTH)
		return 1;
	memset(&nve, 0, sizeof(nve));
	strncpy(nve.nv_name, name, strlen(name) + 1);
	nve.nv_number    = nv_number;
	nve.valid_size   = 1;
	nve.nv_operation = opcode;

	if (opcode == NV_WRITE)
		nve.nv_data[0] = value;
	ret = hisi_nve_direct_access(&nve);
	if (ret) {
		DMERR("nve ops fail! nv_name=%s,nv_number=%d,opcode=%d",
			name, nv_number, opcode);
		return 1;
	}
	if (opcode == NV_READ)
		return (int)nve.nv_data[0];
	return ret;
}

#endif
/* just make static checker happy */
static void hard_hash_fail_verity_dsm(struct dm_verity *v,
	enum verity_block_type type,
	unsigned long long block,
	int error_no, enum info_type sub_err)
{
#if defined(CONFIG_DM_VERITY_HW_RETRY)
	if (get_dsm_notify_flag())
		verity_dsm(v, type, block, error_no, sub_err);
	verity_read_write_nv("HWHASH",
		NVE_HW_HASH_ERR_NUM, NV_WRITE, HASH_ERR_VALUE);
#else
	verity_dsm(v, type, block, error_no, sub_err);
#endif
}

static void print_block_data(unsigned long long blocknr,
	unsigned char *data_to_dump,
	int start, int len)
{
	int i;
	int j;
	int bh_offset = (start / ROW_DATA_LENGTH) * ROW_DATA_LENGTH;
	char ch;

	if (g_err_count >= DM_VERITY_MAX_PRINT_ERRS || (!data_to_dump))
		return;

	g_err_count++;

	pr_err(" block error# : %llu, start offset(byte) : %d\n",
		blocknr, start);
	pr_err("printing Hash dump %dbyte\n", len);
	pr_err("--------------------------------------------------------\n");

	for (i = 0; i < (len + ROW_DATA_LENGTH - 1) / ROW_DATA_LENGTH; i++) {
		pr_err("0x%4.4x : ", bh_offset);
		for (j = 0; j < ROW_DATA_LENGTH; j++) {
			ch = *(data_to_dump + bh_offset + j);
			if (start <= bh_offset + j &&
				start + len > bh_offset + j)
				pr_err("%2.2x ", ch);
			else
				pr_err("-- ");
		}

		pr_err("\n");
		bh_offset += ROW_DATA_LENGTH;
	}
	pr_err("--------------------------------------------------------\n");
}

/*
 * OEM define Handle verification errors.
 */
#if defined(CONFIG_DM_VERITY_HW_RETRY)
int oem_verity_handle_err(struct dm_verity *v)
{
	int value = verity_read_write_nv("VMODE",
		NVE_VERIFY_MODE_NUM, NV_READ, (char)0);
	if (value != 0) {
		pr_err("read verify mode nve fail\n");
		/* we need pay attention on this case */
		return 0;
	} else if (value == DM_MAX_ERR_COUNT) {
		return 1;
	}

	if (!v) {
		pr_err("dm_verity pointer is null\n");
		return 0;
	}

	if (v->verify_failed_flag == 0) {
		if ((value++) >= DM_MAX_ERR_COUNT)
			value = DM_MAX_ERR_COUNT;
		if (verity_read_write_nv("VMODE",
			NVE_VERIFY_MODE_NUM, NV_WRITE, (char)value))
			pr_err("write verify mode nve fail\n");
		v->verify_failed_flag = 1;
	}
	return 0;
}
#endif
#endif

int verity_hash_sel_sha(struct dm_verity *v, struct ahash_request *req,
	const u8 *data, size_t len, u8 *digest)
{
	int r;
	struct verity_result res;

	if ((!v) || (!req) || (!data) || (!digest))
		return 1;

	r = verity_hash_init(v, req, &res, NO_FIRST_TIME_HASH_VERIFY);
	if (unlikely(r < 0))
		goto out;

	r = verity_hash_update(v, req, data, len, &res);
	if (unlikely(r < 0))
		goto out;

	r = verity_hash_final(v, req, digest, &res);

out:
	return r;
}
/*
 * compute digest for the DATA TYPE and the METADATA TYPE,
 * and verify
 */
static int verity_soft_hash(
	struct verity_fec_members verity_fec, const u8 *data)
{
	int r;
	struct verity_result res;
	struct ahash_request *req =
		verity_io_hash_req(verity_fec.v, verity_fec.io);

	if (!req)
		return 1;

	if (verity_fec.type == DM_VERITY_BLOCK_TYPE_DATA) {
		/* we get data from iter for 'DATA TYPE' */
		r = verity_hash_init(verity_fec.v, req,
			&res, NO_FIRST_TIME_HASH_VERIFY);
		if (unlikely(r < 0))
			return r;

		r = verity_for_io_block(verity_fec.v, verity_fec.io,
			verity_fec.iter, &res);
		if (unlikely(r < 0))
			return r;

		r = verity_hash_final(verity_fec.v, req,
			verity_io_real_digest(verity_fec.v, verity_fec.io),
			&res);
		if (unlikely(r < 0))
			return r;
	} else {
		/* we get data from vmalloc for 'METADATA TYPE' */
		/* And the default type is 'METADATA TYPE' */
		r = verity_hash_sel_sha(verity_fec.v, req,
			data, 1 << verity_fec.v->hash_dev_block_bits,
			verity_io_real_digest(verity_fec.v, verity_fec.io));
		if (unlikely(r < 0))
			return r;
	}
	if (likely(memcmp(verity_io_real_digest(verity_fec.v, verity_fec.io),
		verity_fec.want_digest, verity_fec.v->digest_size) == 0))
		return 0;
	return 1;
}

/*
 * for copy data to iter,
 * we must pay attention to that the iter will be changed.
 */
static int dataops_for_bv_block(const struct dm_verity *v,
	struct dm_verity_io *io, struct bvec_iter *iter, const u8 *data)
{
	unsigned int todo   = 1 << v->data_dev_block_bits;
	unsigned int offset = 0;
	struct bio *bio = dm_bio_from_per_bio_data(io, v->ti->per_io_data_size);

	if (!data)
		return PARAMETER_ERR;
	do {
		u8 *page = NULL;
		unsigned int len;
		struct bio_vec bv = bio_iter_iovec(bio, *iter);

		page = kmap_atomic(bv.bv_page);
		len  = bv.bv_len;

		if (likely(len >= todo))
			len = todo;

		(void)memcpy(page + bv.bv_offset, data + offset, len);
		kunmap_atomic(page);

		bio_advance_iter(bio, iter, len);

		todo -= len;
		offset += len;
	} while (todo);

	return 0;
}

static struct dm_bufio_client *read_data_from_ufs_emmc(
	const struct dm_verity *v, struct dm_buffer **bp,
	u8 **data, sector_t block, enum verity_block_type type)
{
	struct dm_bufio_client *bufio = NULL;
	unsigned int verity_block_size;
	struct block_device *bdev = NULL;
	/* we set param for different type. */
	/* And the default type is "METADATA TYPE" */
	if (type == DM_VERITY_BLOCK_TYPE_DATA) {
		verity_block_size = 1 << v->data_dev_block_bits;
		bdev = v->data_dev->bdev;
	} else {
		verity_block_size = 1 << v->hash_dev_block_bits;
		bdev = v->hash_dev->bdev;
	}

	bufio = dm_bufio_client_create(bdev,
		verity_block_size, 1, 0, NULL, NULL);
	if (IS_ERR(bufio))
		return NULL;
	*data = dm_bufio_read(bufio, block, bp);
	if (unlikely(IS_ERR(*data)))
		pr_err("read a null\n");
	return bufio;
}
/*
 * compute digest for the DATA TYPE and the METADATA TYPE,
 * and verify,
 * when the data verify seccussfuly,copy data to the memory.
 */
static int verify_hash(struct verity_fec_members verity_fec,
	const u8 *data, u8 *metadata)
{
	int r;
	unsigned int verity_block_size;
	/* we set param for different type */
	/* And the default type is "METADATA TYPE" */
	if (verity_fec.type == DM_VERITY_BLOCK_TYPE_DATA)
		verity_block_size = 1 << verity_fec.v->data_dev_block_bits;
	else
		verity_block_size = 1 << verity_fec.v->hash_dev_block_bits;

	r = verity_hash_sel_sha(verity_fec.v,
		verity_io_hash_req(verity_fec.v, verity_fec.io),
		data, verity_block_size,
		verity_io_real_digest(verity_fec.v, verity_fec.io));
	if (unlikely(r < 0))
		return r;

	r = memcmp(verity_io_real_digest(verity_fec.v, verity_fec.io),
		verity_fec.want_digest, verity_fec.v->digest_size);
	/* if the data is valid, copy the data */
	if (r == 0 && (verity_fec.type == DM_VERITY_BLOCK_TYPE_METADATA))
		memcpy(metadata, data, verity_block_size);
	else if (r == 0 && (verity_fec.type == DM_VERITY_BLOCK_TYPE_DATA))
		dataops_for_bv_block(verity_fec.v, verity_fec.io,
			verity_fec.iter, data);

	return r;
}

static int reread_data_for_verify(struct verity_fec_members verity_fec,
	sector_t block, u8 *metadata)
{
	int r;
	struct dm_buffer *buf = NULL;
	u8 *real_data = NULL;
	struct dm_bufio_client *bufio = read_data_from_ufs_emmc(verity_fec.v,
		&buf, &real_data, block, verity_fec.type);
	if (!bufio)
		return 1;

	if ((!real_data) || unlikely(IS_ERR(real_data))) {
		r = 1;
		goto release_ret;
	}

	r = verify_hash(verity_fec, real_data, metadata);
#if defined(CONFIG_HUAWEI_DSM)
	if (r == 0)
		verity_dsm(verity_fec.v, verity_fec.type, block,
			DSM_DM_VERITY_ERROR_NO, REREAD_INFO);
	else
		verity_dsm(verity_fec.v, verity_fec.type, block,
			DSM_DM_VERITY_ERROR_NO, REREAD_ERR);
#endif
	if (buf)
		dm_bufio_release(buf);

release_ret:
	if (bufio)
		dm_bufio_client_destroy(bufio);
	return r;
}

static int oem_verity_fec_decode(struct verity_fec_members verity_fec,
	sector_t block, u8 *metadata)
{
	int r;
	struct bvec_iter iter_val = {0};

	if (verity_fec.iter)
		iter_val = *(verity_fec.iter);
	if (verity_soft_hash(verity_fec, metadata) == 0) {
#if defined(CONFIG_HUAWEI_DSM)
		hard_hash_fail_verity_dsm(verity_fec.v, verity_fec.type, block,
			DSM_DM_VERITY_CE_ERROR_NO, CE_INFO);
#endif
		return 0;
	}
	if (verity_fec.iter)
		*(verity_fec.iter) = iter_val;
	if (verity_fec_decode(verity_fec.v, verity_fec.io, verity_fec.type,
		block, metadata, verity_fec.iter) == 0) {
#if defined(CONFIG_HUAWEI_DSM)
		verity_dsm(verity_fec.v, verity_fec.type, block,
			DSM_DM_VERITY_FEC_INFO_NO, FEC_INFO);
#endif
		return 0;
	}

	if (verity_fec.iter)
		*(verity_fec.iter) = iter_val;
	r = reread_data_for_verify(verity_fec, block, metadata);
#if defined(CONFIG_HUAWEI_DSM)
	if (r != 0) {
		print_block_data((unsigned long long)(block),
			(unsigned char *)verity_io_real_digest(verity_fec.v,
				verity_fec.io),
			0, verity_fec.v->digest_size);
		print_block_data((unsigned long long)(block),
			(unsigned char *)verity_fec.want_digest, 0,
			verity_fec.v->digest_size);
	}
#endif
	return r;
}

int oem_metadata_verity_fec_decode(struct dm_verity *v,
	struct dm_verity_io *io, sector_t block, u8 *metadata, u8 *want_digest)
{
	struct verity_fec_members verity_fec;

	if ((!v) || (!io) || (!metadata) || (!want_digest))
		return PARAMETER_ERR;

	verity_fec.v = v;
	verity_fec.io = io;
	verity_fec.type = DM_VERITY_BLOCK_TYPE_METADATA;
	verity_fec.iter = NULL;
	verity_fec.want_digest = want_digest;

	return oem_verity_fec_decode(verity_fec, block, metadata);
}

int oem_data_verity_fec_decode(struct dm_verity *v, struct dm_verity_io *io,
	sector_t block, struct bvec_iter *iter, u8 *want_digest)
{
	struct verity_fec_members verity_fec;

	if ((!v) || (!io) || (!iter) || (!want_digest))
		return PARAMETER_ERR;

	verity_fec.v = v;
	verity_fec.io = io;
	verity_fec.type = DM_VERITY_BLOCK_TYPE_DATA;
	verity_fec.iter = iter;
	verity_fec.want_digest = want_digest;

	return oem_verity_fec_decode(verity_fec, block, NULL);
}
