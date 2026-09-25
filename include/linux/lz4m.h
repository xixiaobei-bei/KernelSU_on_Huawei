#ifndef __LZ4M_H
#define __LZ4M_H
#include <stddef.h>
#include <linux/types.h>

#define memcpy __builtin_memcpy

//  Tunables
#define LZ4M_COMPRESS_HASH_BITS 10
#define LZ4M_COMPRESS_HASH_ENTRIES (1 << LZ4M_COMPRESS_HASH_BITS)
#define LZ4M_COMPRESS_HASH_MULTIPLY 2654435761U
#define LZ4M_COMPRESS_HASH_SHIFT (32 - LZ4M_COMPRESS_HASH_BITS)

//  Not tunables
#define LZ4M_GOFAST_SAFETY_MARGIN 32
#define LZ4M_LBIT 3
#define LZ4M_MBIT 3
#define LZ4M_D_BITS (16 - LZ4M_LBIT - LZ4M_MBIT)
#define LZ4M_LSHIFT (16 - LZ4M_LBIT)
#define LZ4M_MSHIFT (16 - LZ4M_LBIT - LZ4M_MBIT)
#define LZ4M_LMASK ((1 << LZ4M_LBIT) - 1)
#define LZ4M_MMASK ((1 << LZ4M_MBIT) - 1)
#define LZ4M_DMASK ((1 << LZ4M_D_BITS) - 1)

#define LZ4M_RUN_MASK ((1U << LZ4M_MBIT) - 1)
#define LZ4M_NEW_MASK (((1U << LZ4M_MBIT) - 1)<<2)

#define LZ4M_MEM_COMPRESS (16384)

//  Represents a position in the input stream
typedef struct { uint32_t offset; uint32_t word; } lz4m_hash_entry_t;

size_t lz4m_fast_encode(const unsigned char *src_buffer, size_t src_size,
		unsigned char *dst_buffer, size_t *dst_size,
		lz4m_hash_entry_t hash_table[LZ4M_COMPRESS_HASH_ENTRIES]);
size_t lz4m_decompress(const uint8_t *src_buffer, size_t src_size,
		uint8_t *dst_buffer, size_t *dst_size);
#endif
