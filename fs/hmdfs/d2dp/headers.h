/* SPDX-License-Identifier: GPL-2.0 */
/*
 * D2D header declarations
 */

#ifndef D2D_HEADER_H
#define D2D_HEADER_H

#include <linux/types.h>
#include "params.h"
#include "sack.h"

enum {
	D2D_FLAG_NONE    = 0x0, /* No flags                                   */
	D2D_FLAG_ACK     = 0x1, /* This packet is an acknowledgement          */
	D2D_FLAG_SUSPEND = 0x2, /* Suspend sending new data (flow control)    */
};

/**
 * struct d2d_header - the D2D protocol header structure.
 *
 * This is the _unpacked_ header which is used inside the protocol. The real
 * _packed_ version (which is really being sent in the air) is hidden inside
 * headers.c translation unit.
 *
 * @flags: D2D flags
 * @data_len: data length / SACK pairs amount
 * @seq_id: sequence ID for data packet / ACK id
 * @packet_id: packet ID
 * @sack_pairs: the array of SACK pairs if present
 * @version: D2D protocol version
 */
struct d2d_header {
	u16 flags;
	u16 data_len;
	u32 seq_id;
	u64 packet_id;
	u8  version;
	struct sack_pair sack_pairs[SACK_PAIRS_MAX];
};

/* Despite it's isolation you can get packed header's size for length checks */
size_t d2d_get_header_packed_size(void);

/*
 * Reads packed header from @src and unpacks it in @hdr. Returns number of read
 * bytes on success, 0 on failure.
 *
 * WARNING: do not use @hdr if 0 was returned - it may be in incorrect state.
 */
size_t d2d_header_read(const void *src, size_t len, struct d2d_header *hdr);

/*
 * Packs @hdr and writes it to @dst. Returns number of written bytes on success,
 * 0 on failure.
 */
size_t d2d_header_write(void *dst, size_t len, const struct d2d_header *hdr);

#endif /* D2D_HEADER_H */
