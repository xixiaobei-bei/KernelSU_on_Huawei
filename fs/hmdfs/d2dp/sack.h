/* SPDX-License-Identifier: GPL-2.0 */
#ifndef D2D_SACK_H
#define D2D_SACK_H

#include "wrappers.h"

struct sack_pair {
	wrap_t l;
	wrap_t r;
} __packed;

#define SACK_PAIR_SIZE sizeof(struct sack_pair)

struct sack_pairs {
	struct sack_pair *pairs;
	unsigned int len;
};

#endif /* D2D_SACK_H */
