/*
 * verity_handle_4.14.h
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

#ifndef VERITY_HANDLE_H
#define VERITY_HANDLE_H
#define FIRST_TIME_HASH_VERIFY 0
#define NO_FIRST_TIME_HASH_VERIFY 1
int oem_metadata_verity_fec_decode(struct dm_verity *v,
	struct dm_verity_io *io, sector_t block, u8 *metadata, u8 *want_digest);
int oem_data_verity_fec_decode(struct dm_verity *v, struct dm_verity_io *io,
	sector_t block, struct bvec_iter *iter, u8 *want_digest);
int oem_verity_handle_err(struct dm_verity *v);
void verity_dsm_init(void);
int verity_hash_sel_sha(struct dm_verity *v, struct ahash_request *req,
	const u8 *data, size_t len, u8 *digest);
#endif
