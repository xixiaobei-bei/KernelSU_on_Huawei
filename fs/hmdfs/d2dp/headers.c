// SPDX-License-Identifier: GPL-2.0
/*
 * D2D header implementation
 */

#include <linux/printk.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

#include "headers.h"
#include "params.h"
#include "wrappers.h"
#include "sack.h"

struct d2d_header_packed {
	__be16 flags;
	__be16 data_len;
	__be32 seq_id;
	__be64 packet_id;
} __packed;

#define D2D_HEADER_SIZE (sizeof(struct d2d_header_packed))
#define D2D_MAX_DATA_SIZE (UDP_DATAGRAM_SIZE - D2D_HEADER_SIZE)

size_t d2d_get_header_packed_size(void)
{
	return D2D_HEADER_SIZE;
}

static bool d2d_header_is_valid(const struct d2d_header *hdr, size_t len)
{
	size_t payload = len - D2D_HEADER_SIZE;

	if (len < D2D_HEADER_SIZE)
		return false;

	if (hdr->flags & D2D_PROTOCOL_VERSION_MASK)
		return false;

	/* Check corresponding limits for ACK or DATA packet */
	if (hdr->flags & D2D_FLAG_ACK) {
		if (hdr->data_len > SACK_PAIRS_MAX)
			return false;

		if (hdr->data_len * SACK_PAIR_SIZE != payload)
			return false;

		if (hdr->data_len) {
			size_t i = 0;
			wrap_t last_seq_id = hdr->seq_id - 1;

			for (i = 0; i < hdr->data_len; i++) {
				wrap_t l_sack = hdr->sack_pairs[i].l;
				wrap_t r_sack = hdr->sack_pairs[i].r;

				/* if SACK pair = [l, r], expect l <= r */
				if (d2d_wrap_gt(l_sack, r_sack))
					return false;

				/*
				 * if SACK pairs = {[l0, r0], [l1, r1]}, expect
				 * l1 > (r0 + 1)
				 */
				if (d2d_wrap_le(l_sack, last_seq_id + 1))
					return false;

				last_seq_id = r_sack;
			}
		}
	} else {
		if (!hdr->data_len ||
		    hdr->data_len > D2D_MAX_DATA_SIZE ||
		    hdr->data_len != payload)
			return false;
	}

	return true;
}

size_t d2d_header_read(const void *src, size_t len, struct d2d_header *hdr)
{
	size_t offset = 0;
	struct d2d_header_packed packed_hdr = { 0 };

	if (len < D2D_HEADER_SIZE)
		return 0;

	memcpy(&packed_hdr, src + offset, D2D_HEADER_SIZE);
	offset += D2D_HEADER_SIZE;

	hdr->flags = ntohs(packed_hdr.flags);
	hdr->data_len = ntohs(packed_hdr.data_len);
	hdr->seq_id = ntohl(packed_hdr.seq_id);
	hdr->packet_id = be64_to_cpu(packed_hdr.packet_id);

	/* Extract protocol version and discard it from flags */
	hdr->version = (hdr->flags & D2D_PROTOCOL_VERSION_MASK) >> 12;
	hdr->flags &= ~D2D_PROTOCOL_VERSION_MASK;

	if ((hdr->flags & D2D_FLAG_ACK) && hdr->data_len) {
		size_t i = 0;
		size_t sack_byte_len = SACK_PAIR_SIZE * hdr->data_len;

		if (len < D2D_HEADER_SIZE + sack_byte_len)
			return 0;

		/* check the upper boundary before copying data */
		if (hdr->data_len > SACK_PAIRS_MAX)
			return 0;

		for (i = 0; i < hdr->data_len; i++) {
			memcpy(hdr->sack_pairs + i, src + offset,
			       SACK_PAIR_SIZE);
			offset += SACK_PAIR_SIZE;
			hdr->sack_pairs[i].l = ntohl(hdr->sack_pairs[i].l);
			hdr->sack_pairs[i].r = ntohl(hdr->sack_pairs[i].r);
		}
	}

	if (!d2d_header_is_valid(hdr, len))
		return 0;

	return offset;
}

size_t d2d_header_write(void *dest, size_t len, const struct d2d_header *hdr)
{
	size_t i             = 0;
	size_t sack_byte_len = 0;
	size_t offset        = 0;
	struct d2d_header_packed packed_hdr = { 0 };
	bool is_ack = hdr->flags & D2D_FLAG_ACK;

	if (is_ack && hdr->data_len)
		sack_byte_len = SACK_PAIR_SIZE * hdr->data_len;

	if (len < D2D_HEADER_SIZE + sack_byte_len)
		return 0;

	/* Add protocol version to flags */
	packed_hdr.flags = htons(hdr->flags | (hdr->version << 12));
	packed_hdr.data_len = htons(hdr->data_len);
	packed_hdr.seq_id = htonl(hdr->seq_id);
	packed_hdr.packet_id = cpu_to_be64(hdr->packet_id);
	memcpy(dest, &packed_hdr, D2D_HEADER_SIZE);
	offset += D2D_HEADER_SIZE;

	if (!is_ack || !hdr->data_len)
		return offset;

	for (i = 0; i < hdr->data_len; i++) {
		struct sack_pair pair = {
			.l = htonl(hdr->sack_pairs[i].l),
			.r = htonl(hdr->sack_pairs[i].r),
		};
		memcpy(dest + offset, &pair, SACK_PAIR_SIZE);
		offset += SACK_PAIR_SIZE;
	}

	return offset;
}
